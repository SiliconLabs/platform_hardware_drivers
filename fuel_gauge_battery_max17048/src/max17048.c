/***************************************************************************//**
* @file max17048.c
* @brief Driver for the MAX17048/9 Fuel Gauge
********************************************************************************
* # License
* <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
********************************************************************************
*
* SPDX-License-Identifier: Zlib
*
* The licensor of this software is Silicon Laboratories Inc.
*
* This software is provided \'as-is\', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
*******************************************************************************
* # Evaluation Quality
* This code has been minimally tested to ensure that it builds and is suitable
* as a demonstration for evaluation purposes only. This code will be maintained
* at the sole discretion of Silicon Labs.
*******************************************************************************/
#include "max17048.h"
#include "max17048_config.h"

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

// Global variables
static sl_i2cspm_t  *max17048_i2cspm_instance;
static bool         max17048_is_initialized = false;
// Tracking for the state of SLEEP and ALSC in addition to the ATHD field
static uint8_t      max17048_config_lower_tracking = 0x1C;
static uint8_t      max17048_rcomp_tracking = 0x97;
static uint8_t      max17048_valrt_max_tracking = 0xFF;
static uint8_t      max17048_valrt_min_tracking = 0x00;
static uint8_t      max17048_vreset_tracking = 0x96;
static uint8_t      max17048_hibthr_tracking = 0x80;
static uint8_t      max17048_actthr_tracking = 0x30;
static uint32_t     max17048_rcomp_update_interval = MAX17048_CONFIG_RCOMP_UPDATE_INTERVAL_MS;

static sl_sleeptimer_timer_handle_t   max17048_temp_timer_handle;
#ifdef MAX17048_CONFIG_ENABLE_HW_QSTRT
static sl_sleeptimer_timer_handle_t   max17048_quick_start_timer_handle;
#endif
max17048_temp_callback_t              max17048_temp_callback;
max17048_interrupt_callback_t         max17048_interrupt_callback[5];
void *max17048_callback_data[5];

// Function prototypes (private API)
static sl_status_t max17048_read_register(uint8_t reg_addr,
                                          uint8_t *data);
static sl_status_t max17048_write_register(uint8_t reg_addr,
                                           const uint8_t *data);
static void max17048_temp_timer_callback(sl_sleeptimer_timer_handle_t *handle,
                                         void *data);
#ifdef MAX17048_CONFIG_ENABLE_HW_QSTRT
static void max17048_quick_start_callback(sl_sleeptimer_timer_handle_t *handle,
                                          void *data);
#endif
static void max17048_alrt_pin_callback(uint8_t pin);
static sl_status_t max17048_set_rcomp(uint8_t rcomp);
static sl_status_t max17048_get_alert_condition(uint8_t *alert_condition);
static sl_status_t max17048_clear_alert_condition(uint8_t alert_condition,
                                                  uint8_t source);
static sl_status_t max17048_clear_alert_status_bit(void);
static sl_status_t max17048_clear_reset_indicator_bit(void);

/** @endcond */

/***************************************************************************//**
 * @brief
 *    Read a 16-bit word of data from the MAX17048.
 *
 * @param[in] reg_addr
 *    The first register to begin reading from
 *
 * @param[out] data
 *    The data to read
 *
 * @note
 *    All registers must be written and read as 16-bit words;
 *    8-bit writes cause no effect.
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_TRANSMIT I2C transmit failure
 ******************************************************************************/
static sl_status_t max17048_read_register(uint8_t reg_addr,
                                          uint8_t *data)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef result;

  uint8_t i2c_write_data[1];

  seq.addr = MAX17048_I2C_ADDRESS << 1;
  seq.flags = I2C_FLAG_WRITE_READ;

  i2c_write_data[0] = reg_addr;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len  = 1;

  seq.buf[1].data = data;
  seq.buf[1].len  = 2;

  /*
   * Invoke sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1)
   * to prevent the EFM32/EFR32 from entering energy mode EM2
   * or lower during I2C bus activity.
   */
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif
  result = I2CSPM_Transfer(max17048_i2cspm_instance, &seq);
  /*
   * Call sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1)
   * to remove the requirement to remain in EM1 or higher
   * after I2C bus activity is complete.
   */
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif

  if (result != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *    Write a 16-bit word of data to the MAX17048.
 *
 * @param[in] reg_addr
 *    The first register to begin writing to
 *
 * @param[in] length
 *    The number of bytes to write
 *
 * @param[in] data
 *    The data to write
 *
 * @note
 *    All registers must be written and read as 16-bit words; 
 *    8-bit writes cause no effect.
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_TRANSMIT I2C transmit failure
 ******************************************************************************/
static sl_status_t max17048_write_register(uint8_t reg_addr,
                                           const uint8_t *data)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef result;
  uint8_t i2c_write_data[3];
  uint8_t i2c_read_data[1];

  seq.addr = MAX17048_I2C_ADDRESS << 1;
  seq.flags = I2C_FLAG_WRITE;

  i2c_write_data[0] = reg_addr;
  i2c_write_data[1] = data[0];
  i2c_write_data[2] = data[1];

  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len  = 3;

  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len  = 0;

  /*
   * Invoke sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1)
   * to prevent the EFM32/EFR32 from entering energy mode EM2
   * or lower during I2C bus activity.
   */
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif
  result = I2CSPM_Transfer(max17048_i2cspm_instance, &seq);
  /*
   * Call sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1)
   * to remove the requirement to remain in EM1 or higher
   * after I2C bus activity is complete.
   */
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif
  if (result != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *    This function sets the MAX17048 RCOMP temperature compensation factor.
 *
 * @param[in] rcomp
 *    Temperature compensation factor
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
static sl_status_t max17048_set_rcomp(uint8_t rcomp)
{
  sl_status_t status;
  uint8_t buffer[2];

  // Update the private global variable to track
  max17048_rcomp_tracking = rcomp;
  buffer[0] = max17048_rcomp_tracking;
  buffer[1] = max17048_config_lower_tracking;
  status = max17048_write_register(MAX17048_CONFIG, buffer);

  return status;
}

/***************************************************************************//**
 *  Sleeptimer callback function to get the temperature,
 *  calculate the new RCOMP value, and write it to the MAX17048.
 ******************************************************************************/
static void max17048_temp_timer_callback(sl_sleeptimer_timer_handle_t *handle,
                                         void *data)
{
  (void)handle;
  (void)data;
  uint8_t temp;
  uint8_t rcomp;

  /*
   * Invoke sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1)
   * to prevent the EFM32/EFR32 from entering energy mode EM2
   * or lower during reading the temperature.
   */
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif
  temp = max17048_temp_callback();

  if (temp > 20) {
    rcomp = RCOMP0 + (temp - 20) * TEMP_CO_UP;
  } else {
    rcomp = RCOMP0 + (temp - 20) * TEMP_CO_DOWN;
  }

  max17048_set_rcomp(rcomp);
  /*
   * Call sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1)
   * to remove the requirement to remain in EM1 or higher
   * after writing rcomp to the MAX17048 is completed
   */
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif
}

/***************************************************************************//**
 *  Sleeptimer callback function to de-assert QSTRT pin after 1 ms.
 ******************************************************************************/
#ifdef MAX17048_CONFIG_ENABLE_HW_QSTRT
static void max17048_quick_start_callback(sl_sleeptimer_timer_handle_t *handle,
                                          void *data)
{
  (void)handle;
  (void)data;

  GPIO_PinOutClear(MAX17048_CONFIG_ENABLE_QSTRT_PORT,
                   MAX17048_CONFIG_ENABLE_QSTRT_PIN);
}
#endif

/***************************************************************************//**
 * This is a callback function that is invoked each time a GPIO interrupt
 * in one of the pushbutton inputs occurs. Pin number is passed as parameter.
 *
 * @param[in] pin  Pin number where interrupt occurs
 *
 * @note This function is called from ISR context and therefore it is
 *       not possible to call any API functions directly.
 ******************************************************************************/
static void max17048_alrt_pin_callback(uint8_t pin)
{
  (void)pin;
  max17048_interrupt_callback_t callback;
  uint8_t alert_condition = 0;

  // Check alert condition
  max17048_get_alert_condition(&alert_condition);

  if ((alert_condition & MAX17048_STATUS_VR) != 0) {
    /*
     * Process RESET alert callback because battery
     * has changed or there has been POR
     */
    callback = max17048_interrupt_callback[IRQ_RESET];
    callback(IRQ_RESET, max17048_callback_data[IRQ_RESET]);
    max17048_clear_alert_condition(alert_condition, MAX17048_STATUS_VR);
  } else if ((alert_condition & MAX17048_STATUS_HD) != 0) {
    // Cell nearing empty; may need to place system in ultra-low-power state
    callback = max17048_interrupt_callback[IRQ_EMPTY];
    callback(IRQ_EMPTY, max17048_callback_data[IRQ_EMPTY]);
    max17048_clear_alert_condition(alert_condition, MAX17048_STATUS_HD);
  } else if ((alert_condition & MAX17048_STATUS_VL) != 0) {
    /*
     * Voltage low alert; may need to set parameters for reduced
     * energy use before reaching empty threshold
     */
    callback = max17048_interrupt_callback[IRQ_VCELL_LOW];
    callback(IRQ_VCELL_LOW, max17048_callback_data[IRQ_VCELL_LOW]);
    max17048_clear_alert_condition(alert_condition, MAX17048_STATUS_VL);
  } else if ((alert_condition & MAX17048_STATUS_VH) != 0) {
    /*
     * Voltage high alert; may indicate battery is full charged
     * and need to place charging IC in maintenance/trickle charge state
     */
    callback = max17048_interrupt_callback[IRQ_VCELL_HIGH];
    callback(IRQ_VCELL_HIGH, max17048_callback_data[IRQ_VCELL_HIGH]);
    max17048_clear_alert_condition(alert_condition, MAX17048_STATUS_VH);
  } else if ((alert_condition & MAX17048_STATUS_SC) != 0) {
    // SOC changed by 1%; lowest priority interrupt
    callback = max17048_interrupt_callback[IRQ_SOC];
    callback(IRQ_SOC, max17048_callback_data[IRQ_SOC]);
    max17048_clear_alert_condition(alert_condition, MAX17048_STATUS_SC);
  }

  // Clears the ALRT status bit to release the ALRT pin.
  max17048_clear_alert_status_bit();
}

/***************************************************************************//**
 * @brief This function identifies which alert condition was met.
 *
 * @param[out] alert_condition
 *   These bits are set when they are cause an alert. 
 *   Values in the STATUS register:
 *   - <b>Bit 1:</b> (voltage high) is set when VCELL has been above
 *     ALRT.VALRTMAX.
 *   - <b>Bit 2:</b> (voltage low) is set when VCELL has been below
 *     ALRT.VALRTMIN.
 *   - <b>Bit 3:</b> (voltage reset) is set after the device has been reset if
 *     EnVr is set.
 *   - <b>Bit 4:</b> (SOC low) is set when SOC crosses the value in CONFIG.ATHD.
 *   - <b>Bit 5:</b> (1% SOC change) is set when SOC changes by at least 1% if
 *     CONFIG.ALSC is set.
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
static sl_status_t max17048_get_alert_condition(uint8_t *alert_condition)
{
  sl_status_t status;
  uint8_t buffer[2];

  status = max17048_read_register(MAX17048_STATUS, buffer);
  if (status == SL_STATUS_OK) {
    *alert_condition = buffer[0];
  }

  return status;
}

/***************************************************************************//**
 * @brief
 *    This function clears the alert status bit.
 *
 * @details
 *    When an alert is triggered, the IC drives the ALRT pin logic-low
 *    and sets CONFIG.ALRT = 1. The ALRT pin remains logic-low until the system
 *    software writes CONFIG.ALRT = 0 to clear the alert.
 *    The alert function is enabled by default, so any alert can
 *    occur immediately upon power-up. Entering sleep mode clears no alerts.
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
static sl_status_t max17048_clear_alert_status_bit(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  buffer[0] = max17048_rcomp_tracking;
  /*
   * Clear the ALRT bit by rewriting the lower byte of the CONFIG register.
   * The max17048_config_lower_tracking holds the state of SLEEP and ALSC
   * in addition to the ATHD field, so this will clear ALRT without modifying
   * the other bits in the lower byte of CONFIG.
   */
  buffer[1] = max17048_config_lower_tracking;
  status = max17048_write_register(MAX17048_CONFIG, buffer);

  return status;
}

/***************************************************************************//**
 * @brief
 *    This function clears the reset indicator bit.
 *
 * @details
 *     RI (reset indicator) is set when the device powers up.
 *     Any time this bit is set, the IC is not configured, so the
 *     model should be loaded and the bit should be cleared
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
static sl_status_t max17048_clear_reset_indicator_bit(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  status = max17048_read_register(MAX17048_STATUS, buffer);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Clear RI bit in the upper byte of the STATUS register if it is set
  if ((buffer[0] & MAX17048_STATUS_RI) != 0) {
    buffer[0] &= ~MAX17048_STATUS_RI;
    status = max17048_write_register(MAX17048_STATUS, buffer);
  }

  return status;
}

/**************************************************************************/ /**
 * @brief This function clears the correct flag for the specified alert.
 *
 * @param[in] alert_condition
 *    These bits are set when they are cause an alert.
 *
 * @param[in] source
 *    MAX17048 interrupt source.
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
static sl_status_t max17048_clear_alert_condition(uint8_t alert_condition,
                                                  uint8_t source)
{
  sl_status_t status;
  uint8_t buffer[2];

  /*
   * Clearing the flag for the specified alert also clears the RI bit.
   * RI bit, which is written to 0 to enable the loaded model, so there is
   * no issue with writing it to 0 again when the device is running normally.
   */
  buffer[0] = alert_condition & ~source;
  status = max17048_write_register(MAX17048_STATUS, buffer);

  return status;
}
/***************************************************************************//**
 *  Initialize the MAX17048.
 ******************************************************************************/
sl_status_t max17048_init(sl_i2cspm_t *i2cspm)
{
  sl_status_t status;

  // If already initialized, return status
  if (max17048_is_initialized == true) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }
  
  // Update i2cspm instance
  max17048_i2cspm_instance = i2cspm;

  GPIOINT_Init();
  GPIO_PinModeSet(MAX17048_CONFIG_ALRT_PORT,
                  MAX17048_CONFIG_ALRT_PIN,
                  gpioModeInputPullFilter,
                  1);
  GPIO_ExtIntConfig(MAX17048_CONFIG_ALRT_PORT,
                    MAX17048_CONFIG_ALRT_PIN,
                    MAX17048_CONFIG_ALRT_PIN,
                    false,
                    true,
                    true);
  // Register the callback function that is invoked when interrupt occurs
  GPIOINT_CallbackRegister(MAX17048_CONFIG_ALRT_PIN,
                           max17048_alrt_pin_callback);

#ifdef MAX17048_CONFIG_ENABLE_HW_QSTRT
  GPIO_PinModeSet(MAX17048_CONFIG_ENABLE_QSTRT_PORT,
                  MAX17048_CONFIG_ENABLE_QSTRT_PIN,
                  gpioModePushPull,
                  0);
#endif

  /* The driver calculates and updates the RCOMP factor at a rate of
   * 1000 ms <= MAX17048_CONFIG_RCOMP_UPDATE_INTERVAL_MS <= 60000 ms
   * and defaults to 1 minute (60000 ms = 1 minute).
   */
  if ((max17048_rcomp_update_interval < 1000) 
      || (max17048_rcomp_update_interval > 60000)) {
    return SL_STATUS_INVALID_RANGE;
  }

  TEMPDRV_Init();
  status = max17048_register_temperature_callback(((max17048_temp_callback_t)((int32_t)TEMPDRV_GetTemp)));
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Read and clear RI bit if it is set
  max17048_clear_reset_indicator_bit();

  // Change drive state to initialized
  max17048_is_initialized = true;

// User-specified stabilization delay
#if MAX17048_CONFIG_STABILIZATION_DELAY > 0
  sl_sleeptimer_delay_millisecond(MAX17048_CONFIG_STABILIZATION_DELAY);
#endif

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  De-initialize the MAX17048.
 ******************************************************************************/
sl_status_t max17048_deinit(void)
{
  sl_status_t status;

  if (max17048_is_initialized == false) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  // De-initialization tasks
  GPIO_PinModeSet(MAX17048_CONFIG_ALRT_PORT,
                  MAX17048_CONFIG_ALRT_PIN,
                  gpioModeDisabled,
                  1);

#ifdef MAX17048_CONFIG_ENABLE_HW_QSTRT
  GPIO_PinModeSet(MAX17048_CONFIG_ENABLE_QSTRT_PORT,
                  MAX17048_CONFIG_ENABLE_QSTRT_PIN,
                  gpioModeDisabled,
                  0);
#endif

  status = max17048_disable_soc_interrupt();
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = max17048_disable_empty_interrupt();
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = max17048_disable_vhigh_interrupt();
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = max17048_disable_vlow_interrupt();
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = max17048_disable_reset_interrupt();
  if (status != SL_STATUS_OK) {
    return status;
  }

  status = max17048_enter_sleep();
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Mark driver as not initialized
  max17048_is_initialized = false;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  This function returns the cell voltage in millivolts.
 ******************************************************************************/
sl_status_t max17048_get_vcell(uint32_t *vcell)
{
  sl_status_t status;
  uint8_t buffer[2];
  uint32_t vcell_reg_val;

  status = max17048_read_register(MAX17048_VCELL, buffer);
  vcell_reg_val = (buffer[0] << 8) | buffer[1];
  *vcell = (uint32_t)((uint64_t)(vcell_reg_val) * MAX17048_VCELL_RESOLUTION / 1000000);

  return status;
}

/***************************************************************************//**
 *  Read the SOC register and return the state-of-charge 
 *  as an integer (0 - 100%).
 ******************************************************************************/
sl_status_t max17048_get_soc(uint32_t *soc)
{
  sl_status_t status;
  uint8_t buffer[2];
  uint16_t soc_reg_val;

  status = max17048_read_register(MAX17048_SOC, buffer);
  soc_reg_val = (uint16_t)((buffer[0] << 8) | buffer[1]);
  *soc = (uint32_t)(soc_reg_val / MAX17048_SOC_RESOLUTION);

  return status;
}

/***************************************************************************//**
 *  This function gets an approximate value for the average SOC rate of change.
 ******************************************************************************/
sl_status_t max17048_get_crate(float *crate)
{
  sl_status_t status;
  uint8_t buffer[2];
  uint16_t crate_reg_val;

  status = max17048_read_register(MAX17048_CRATE, buffer);
  crate_reg_val = (buffer[0] << 8) | buffer[1];
  *crate = (float)(crate_reg_val * MAX17048_CRATE_RESOLUTION);

  return status;
}

/***************************************************************************//**
 *  Register the temperature update callback for the MAX17048 driver
 ******************************************************************************/
sl_status_t max17048_register_temperature_callback(max17048_temp_callback_t temp_cb)
{
  sl_status_t status;

  if (max17048_temp_callback == temp_cb) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  if (temp_cb == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  max17048_temp_callback = temp_cb;

  status = sl_sleeptimer_start_periodic_timer_ms(&max17048_temp_timer_handle,
                                                 max17048_rcomp_update_interval,
                                                 max17048_temp_timer_callback,
                                                 (void *)NULL,
                                                 0,
                                                 0);

  return status;
}

/***************************************************************************//**
 *  Unregister the temperature update callback for the MAX17048 driver
 ******************************************************************************/
sl_status_t max17048_unregister_temperature_callback(void)
{
  if (max17048_temp_callback == NULL) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  max17048_temp_callback = (max17048_temp_callback_t)((int32_t)TEMPDRV_GetTemp);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Set the RCOMP update interval.
 ******************************************************************************/
sl_status_t max17048_set_update_interval(uint32_t interval)
{
  sl_status_t status;

  if ((interval < 1000) || (interval > 60000)) {
    return SL_STATUS_INVALID_RANGE;
  }

  /*
   * Get the temperature, update the RCOMP value before restarting Sleeptimer
   * with the new update interval.
   */
  max17048_temp_timer_callback(NULL, NULL);

  max17048_rcomp_update_interval = interval;
  status = sl_sleeptimer_restart_periodic_timer_ms(&max17048_temp_timer_handle,
                                                   max17048_rcomp_update_interval,
                                                   max17048_temp_timer_callback,
                                                   (void *)NULL,
                                                   0,
                                                   0);

  return status;
}

/***************************************************************************//**
 *  Get the RCOMP update interval.
 ******************************************************************************/
uint32_t max17048_get_update_interval(void)
{
  return max17048_rcomp_update_interval;
}

/***************************************************************************//**
 *  Mask MAX17048 interrupts
 ******************************************************************************/
void max17048_mask_interrupts(void)
{
  GPIO_IntDisable(1 << MAX17048_CONFIG_ALRT_PIN);
}

/***************************************************************************//**
 *  Unmask MAX17048 interrupts
 ******************************************************************************/
void max17048_unmask_interrupts(void)
{
  /*
   * Clear ALRT pin interrupt flag in case an edge was detected while
   * the interrupt was disabled.
   */
  GPIO_IntClear(1 << MAX17048_CONFIG_ALRT_PIN);
  GPIO_IntEnable(1 << MAX17048_CONFIG_ALRT_PIN);
}

/***************************************************************************//**
 *  Enable alerting when SOC changes.
 ******************************************************************************/
sl_status_t max17048_enable_soc_interrupt(max17048_interrupt_callback_t irq_cb,
                                          void *cb_data)
{
  sl_status_t status;
  uint8_t buffer[2];

  if ((irq_cb == NULL) || (cb_data == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  if ((max17048_interrupt_callback[IRQ_SOC] != NULL)
      || (max17048_callback_data[IRQ_SOC] != NULL)) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  max17048_interrupt_callback[IRQ_SOC] = irq_cb;
  max17048_callback_data[IRQ_SOC] = cb_data;
  // Get the upper byte of the CONFIG register
  buffer[0] = max17048_rcomp_tracking;
  /*
   * Set ALSC bit in the lower byte of the CONFIG register
   * to enable alerting when SOC changes
   */
  max17048_config_lower_tracking |= MAX17048_CONFIG_ALSC;
  // Get the lower byte of the CONFIG register
  buffer[1] = max17048_config_lower_tracking;

  status = max17048_write_register(MAX17048_CONFIG, buffer);

  return status;
}

/***************************************************************************//**
 *  Disables the MAX17048 state-of-charge (SOC) interrupt and unregisters
 *  the user-provided callback function.
 ******************************************************************************/
sl_status_t max17048_disable_soc_interrupt(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  if ((max17048_interrupt_callback[IRQ_SOC] == NULL)
      || (max17048_callback_data[IRQ_SOC] == NULL)) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  max17048_interrupt_callback[IRQ_SOC] = NULL;
  max17048_callback_data[IRQ_SOC] = NULL;

  // Get the upper byte of the CONFIG register
  buffer[0] = max17048_rcomp_tracking;
  /*
   * Clear ALSC bit in the lower byte of the CONFIG register
   * to disable alerting when SOC changes
   */
  max17048_config_lower_tracking &= ~MAX17048_CONFIG_ALSC;
  // Get the lower byte of the CONFIG register
  buffer[1] = max17048_config_lower_tracking;
  status = max17048_write_register(MAX17048_CONFIG, buffer);

  return status;
}

/***************************************************************************//**
 * Enables the empty alert interrupt and set its threshold.
 ******************************************************************************/
sl_status_t max17048_enable_empty_interrupt(uint8_t athd,
                                            max17048_interrupt_callback_t irq_cb,
                                            void *cb_data)
{
  sl_status_t status;
  uint8_t buffer[2];

  if ((athd < 1) || (athd > 32)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if ((irq_cb == NULL) || (cb_data == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  if ((max17048_interrupt_callback[IRQ_EMPTY] != NULL)
      || (max17048_callback_data[IRQ_EMPTY] != NULL)) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  max17048_interrupt_callback[IRQ_EMPTY] = irq_cb;
  max17048_callback_data[IRQ_EMPTY] = cb_data;
  // Get the upper byte of the CONFIG register
  buffer[0] = max17048_rcomp_tracking;
  // Update the private global variable to track
  max17048_config_lower_tracking &= 0xE0;
  max17048_config_lower_tracking |= 32 - athd;
  // Update the lower byte of the CONFIG register
  buffer[1] = max17048_config_lower_tracking;

  status = max17048_write_register(MAX17048_CONFIG, buffer);

  return status;
}

/***************************************************************************//**
 * Disables the MAX17048 empty alert interrupt and unregisters the
 * the user-provided callback function.
 ******************************************************************************/
sl_status_t max17048_disable_empty_interrupt(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  if ((max17048_interrupt_callback[IRQ_EMPTY] == NULL)
      || (max17048_callback_data[IRQ_EMPTY] == NULL)) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  max17048_interrupt_callback[IRQ_EMPTY] = NULL;
  max17048_callback_data[IRQ_EMPTY] = NULL;

  // Get the upper byte of the CONFIG register
  buffer[0] = max17048_rcomp_tracking;
  /*
   * Update the private global variable to track.
   * Setting a threshold of 1% (ATHD = 31) should prevent the interrupt
   * from being requested and the driver will ignore it.
   */
  max17048_config_lower_tracking &= 0xE0;
  max17048_config_lower_tracking |= 0x1F;
  // Update the lower byte of the CONFIG register
  buffer[1] = max17048_config_lower_tracking;

  status = max17048_write_register(MAX17048_CONFIG, buffer);

  return status;
}

/***************************************************************************//**
 *  Set the empty alert threshold.
 ******************************************************************************/
sl_status_t max17048_set_empty_threshold(uint8_t athd)
{
  sl_status_t status;
  uint8_t buffer[2];

  if ((athd == 0) || (athd > 32)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Get the upper byte of the CONFIG register
  buffer[0] = max17048_rcomp_tracking;
  // Update the private global variable to track
  max17048_config_lower_tracking &= 0xE0;
  max17048_config_lower_tracking |= 32 - athd;
  // Update the lower byte of the CONFIG register
  buffer[1] = max17048_config_lower_tracking;

  status = max17048_write_register(MAX17048_CONFIG, buffer);

  return status;
}

/***************************************************************************//**
 *  Get the empty threshold.
 ******************************************************************************/
uint8_t max17048_get_empty_threshold(void)
{
  return (32 - (max17048_config_lower_tracking & 0x1F));
}

/***************************************************************************//**
 *  Enable the voltage high alert interrupt, sets its threshold, and
 *  register a user-provided callback function to respond to it.
 ******************************************************************************/
sl_status_t max17048_enable_vhigh_interrupt(uint32_t valrt_max_mv,
                                            max17048_interrupt_callback_t irq_cb,
                                            void *cb_data)
{
  sl_status_t status;
  uint8_t buffer[2];

  if ((irq_cb == NULL) || (cb_data == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  if ((max17048_interrupt_callback[IRQ_VCELL_HIGH] != NULL)
      || (max17048_callback_data[IRQ_VCELL_HIGH] != NULL)) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  max17048_interrupt_callback[IRQ_VCELL_HIGH] = irq_cb;
  max17048_callback_data[IRQ_VCELL_HIGH] = cb_data;

  if (valrt_max_mv > MAX17048_VALRT_MAX_MV) {
    valrt_max_mv = MAX17048_VALRT_MAX_MV;
  }

  buffer[0] = max17048_valrt_min_tracking; // Get the VALRT.MIN register
  // Update the private global variable to track
  max17048_valrt_max_tracking = (uint8_t)(valrt_max_mv / MAX17048_VALRT_RESOLUTION);
  buffer[1] = max17048_valrt_max_tracking; // Update the VALRT.MAX register
  status = max17048_write_register(MAX17048_VALRT, buffer);

  return status;
}

/***************************************************************************//**
 *  Disables the MAX17048 voltage high alert interrupt and
 *  unregisters the user-provided callback function.
 ******************************************************************************/
sl_status_t max17048_disable_vhigh_interrupt(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  if ((max17048_interrupt_callback[IRQ_VCELL_HIGH] == NULL)
      || (max17048_callback_data[IRQ_VCELL_HIGH] == NULL)) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  max17048_interrupt_callback[IRQ_VCELL_HIGH] = NULL;
  max17048_callback_data[IRQ_VCELL_HIGH] = NULL;

  // Get the VALRT.MIN register
  buffer[0] = max17048_valrt_min_tracking;
  // Update the private global variable to track
  max17048_valrt_max_tracking = 0xFF;
  // Update the VALRT.MAX register
  buffer[1] = max17048_valrt_max_tracking;
  status = max17048_write_register(MAX17048_VALRT, buffer);

  return status;
}

/***************************************************************************//**
 *  Set the voltage high alert interrupt threshold in millivolts.
 ******************************************************************************/
sl_status_t max17048_set_vhigh_threshold(uint32_t valrt_max_mv)
{
  sl_status_t status;
  uint8_t buffer[2];

  if (valrt_max_mv > MAX17048_VALRT_MAX_MV) {
    valrt_max_mv = MAX17048_VALRT_MAX_MV;
  }

  // Get the VALRT.MIN register
  buffer[0] = max17048_valrt_min_tracking;
  // Update the private global variable to track
  max17048_valrt_max_tracking = (uint8_t)(valrt_max_mv / MAX17048_VALRT_RESOLUTION);
  // Update the VALRT.MAX register
  buffer[1] = max17048_valrt_max_tracking;
  status = max17048_write_register(MAX17048_VALRT, buffer);

  return status;
}

/***************************************************************************//**
 *  Get the voltage high alert interrupt threshold in millivolts.
 ******************************************************************************/
uint32_t max17048_get_vhigh_threshold(void)
{
  return (uint32_t)(max17048_valrt_max_tracking * MAX17048_VALRT_RESOLUTION);
}

/***************************************************************************//**
 *  Enable the voltage low alert interrupt, sets its threshold, and
 *  register a user-provided callback function to respond to it.
 ******************************************************************************/
sl_status_t max17048_enable_vlow_interrupt(uint32_t valrt_min_mv,
                                           max17048_interrupt_callback_t irq_cb,
                                           void *cb_data)
{
  sl_status_t status;
  uint8_t buffer[2];

  if ((irq_cb == NULL) || (cb_data == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  if ((max17048_interrupt_callback[IRQ_VCELL_LOW] != NULL)
     || (max17048_callback_data[IRQ_VCELL_LOW] != NULL)) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  max17048_interrupt_callback[IRQ_VCELL_LOW] = irq_cb;
  max17048_callback_data[IRQ_VCELL_LOW] = cb_data;

  if (valrt_min_mv > MAX17048_VALRT_MIN_MV) {
    valrt_min_mv = MAX17048_VALRT_MIN_MV;
  }

  // Get the VALRT.MAX register
  buffer[1] = max17048_valrt_max_tracking;
  // Update the private global variable to track
  max17048_valrt_min_tracking = (uint8_t)(valrt_min_mv / MAX17048_VALRT_RESOLUTION);
  // Update the VALRT.MIN register
  buffer[0] = max17048_valrt_min_tracking;
  status = max17048_write_register(MAX17048_VALRT, buffer);

  return status;
}

/***************************************************************************//**
 *  Disables the MAX17048 voltage low alert interrupt and
 *  unregisters the user-provided callback function.
 ******************************************************************************/
sl_status_t max17048_disable_vlow_interrupt(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  if ((max17048_interrupt_callback[IRQ_VCELL_LOW] == NULL)
      || (max17048_callback_data[IRQ_VCELL_LOW] == NULL)) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  max17048_interrupt_callback[IRQ_VCELL_LOW] = NULL;
  max17048_callback_data[IRQ_VCELL_LOW] = NULL;

  // Get the VALRT.MAX register
  buffer[1] = max17048_valrt_max_tracking;
  // Update the private global variable to track
  max17048_valrt_min_tracking = 0x00;
  // Update the VALRT.MIN register
  buffer[0] = max17048_valrt_min_tracking;
  status = max17048_write_register(MAX17048_VALRT, buffer);

  return status;
}

/***************************************************************************//**
 *  Set the voltage low alert interrupt threshold in millivolts.
 ******************************************************************************/
sl_status_t max17048_set_vlow_threshold(uint32_t valrt_min_mv)
{
  sl_status_t status;
  uint8_t buffer[2];

  if (valrt_min_mv > MAX17048_VALRT_MIN_MV) {
    valrt_min_mv = MAX17048_VALRT_MIN_MV;
  }

  // Get the VALRT.MAX register
  buffer[1] = max17048_valrt_max_tracking;
  // Update the private global variable to track
  max17048_valrt_min_tracking = (uint8_t)(valrt_min_mv / MAX17048_VALRT_RESOLUTION);
  // Update the VALRT.MIN register
  buffer[0] = max17048_valrt_min_tracking;
  status = max17048_write_register(MAX17048_VALRT, buffer);

  return status;
}

/***************************************************************************//**
 *  Get the voltage low alert interrupt threshold in millivolts.
 ******************************************************************************/
uint32_t max17048_get_vlow_threshold(void)
{
  return (uint32_t)(max17048_valrt_min_tracking * MAX17048_VALRT_RESOLUTION);
}

/***************************************************************************//**
 *  Enable the voltage reset alert interrupt and set its threshold.
 ******************************************************************************/
sl_status_t max17048_enable_reset_interrupt(uint32_t vreset_mv,
                                            max17048_interrupt_callback_t irq_cb,
                                            void *cb_data)
{
  sl_status_t status;
  uint8_t buffer[2];
  uint8_t vreset_val;

  if ((irq_cb == NULL) || (cb_data == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  if ((max17048_interrupt_callback[IRQ_RESET] != NULL)
      || (max17048_callback_data[IRQ_RESET] != NULL)) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  if (vreset_mv > MAX17048_VRESET_MV) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  max17048_interrupt_callback[IRQ_RESET] = irq_cb;
  max17048_callback_data[IRQ_RESET] = cb_data;

  /*
   * Set EnVR bit in the upper of the STATUS register
   * to enable voltage reset alert.
   * By doing this, RI bit is clear.
   * RI bit, which is written to 0 to enable the loaded model, so there is
   * no issue with writing it to 0 again when the device is running normally.
   */
  buffer[0] = MAX17048_STATUS_ENVR;
  // Lower byte of STATUS is not implemented; writing to it has no effect
  buffer[1] = 0x00;
  status = max17048_write_register(MAX17048_STATUS, buffer);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Update the private global variable to track
  max17048_vreset_tracking &= 0x01;
  vreset_val = (uint8_t)(vreset_mv / MAX17048_VRESET_RESOLUTION);
  max17048_vreset_tracking |= (vreset_val << MAX17048_VRESET_SHIFT);
  // Update the VRESET register
  buffer[0] = max17048_vreset_tracking;
  // Writing to the ID register has no effect
  buffer[1] = 0x00;
  status = max17048_write_register(MAX17048_VRESET_ID, buffer);

  return status;
}

/***************************************************************************//**
 *  Disables the MAX17048 reset alert interrupt and unregisters
 *  the user-provided callback function.
 ******************************************************************************/
sl_status_t max17048_disable_reset_interrupt(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  if ((max17048_interrupt_callback[IRQ_RESET] == NULL)
      || (max17048_callback_data[IRQ_RESET] == NULL)) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  max17048_interrupt_callback[IRQ_RESET] = NULL;
  max17048_callback_data[IRQ_RESET] = NULL;

  /*
   * Clear EnVR bit in the upper byte of the STATUS register
   * to disable voltage reset alert
   */
  buffer[0] = 0x00;
  // Lower byte of STATUS is not implemented; writing to it has no effect
  buffer[1] = 0x00;
  status = max17048_write_register(MAX17048_STATUS, buffer);

  return status;
}

/***************************************************************************//**
 *  Set the reset alert interrupt threshold in millivolts.
 ******************************************************************************/
sl_status_t max17048_set_reset_threshold(uint32_t vreset_mv)
{
  sl_status_t status;
  uint8_t buffer[2];
  uint8_t vreset_val;

  if (vreset_mv > MAX17048_VRESET_MV) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Update the private global variable to track
  max17048_vreset_tracking &= MAX17048_VRESET_DIS;
  vreset_val = (uint8_t)(vreset_mv / MAX17048_VRESET_RESOLUTION);
  max17048_vreset_tracking |= (vreset_val << MAX17048_VRESET_SHIFT);
  // Update the VRESET register
  buffer[0] = max17048_vreset_tracking;
  // Writing to the ID register has no effect
  buffer[1] = 0x00;
  status = max17048_write_register(MAX17048_VRESET_ID, buffer);

  return status;
}

/***************************************************************************//**
 *  Get the reset alert interrupt threshold in millivolts.
 ******************************************************************************/
uint32_t max17048_get_reset_threshold(void)
{
  return (uint32_t)((max17048_vreset_tracking >> MAX17048_VRESET_SHIFT) * MAX17048_VRESET_RESOLUTION);
}

/***************************************************************************//**
 *  Read hibernate mode status.
 ******************************************************************************/
sl_status_t max17048_get_hibernate_state(max17048_hibstate_t *hibstat)
{
  sl_status_t status;
  uint8_t buffer[2];

  status = max17048_read_register(MAX17048_MODE, buffer);
  if (status == SL_STATUS_OK) {
    // Get HibStat bit in the upper byte of the MODE register
    *hibstat = (max17048_hibstate_t)((buffer[0] & MAX17048_MODE_HIBSTAT) != 0);
  }

  return status;
}

/***************************************************************************//**
 *  Set thresholds for entering and exiting hibernate mode.
 ******************************************************************************/
sl_status_t max17048_enable_auto_hibernate(float hib_thr, uint32_t act_thr)
{
  sl_status_t status;
  uint8_t buffer[2];

  if ((hib_thr == 0.0) || (act_thr == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (hib_thr > MAX17048_HIBTHR_PERCENT) {
    hib_thr = MAX17048_HIBTHR_PERCENT;
  }

  if (act_thr > MAX17048_ACTTHR_MV) {
    act_thr = MAX17048_ACTTHR_MV;
  }

  // Update the private global variables to track
  max17048_hibthr_tracking = (uint8_t)(hib_thr / MAX17048_HIBTHR_RESOLUTION);
  max17048_actthr_tracking = (uint8_t)(act_thr * 1000 / MAX17048_ACTTHR_RESOLUTION);

  buffer[0] = max17048_hibthr_tracking;
  buffer[1] = max17048_actthr_tracking;
  status = max17048_write_register(MAX17048_HIBRT, buffer);

  return status;
}

/***************************************************************************//**
 *  Disables automatic hibernation by setting the activity and
 *  CRATE thresholds to 0x0.
 ******************************************************************************/
sl_status_t max17048_disable_auto_hibernate(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  // Update the private global variables to track
  max17048_hibthr_tracking = 0x00;
  max17048_actthr_tracking = 0x00;

  buffer[0] = max17048_hibthr_tracking;
  buffer[1] = max17048_actthr_tracking;
  status = max17048_write_register(MAX17048_HIBRT, buffer);

  return status;
}

/***************************************************************************//**
 *  Set the hibernate threshold level in percent.
 ******************************************************************************/
sl_status_t max17048_set_hibernate_threshold(float hib_thr)
{
  sl_status_t status;
  uint8_t buffer[2];

  if (hib_thr > MAX17048_HIBTHR_PERCENT) {
    hib_thr = MAX17048_HIBTHR_PERCENT;
  }
  // Update the private global variable to track
  max17048_hibthr_tracking = (uint8_t)(hib_thr / MAX17048_HIBTHR_RESOLUTION);
  buffer[0] = max17048_hibthr_tracking;
  buffer[1] = max17048_actthr_tracking;
  status = max17048_write_register(MAX17048_HIBRT, buffer);

  return status;
}

/***************************************************************************//**
 *  Get the hibernate threshold level in percent.
 ******************************************************************************/
float max17048_get_hibernate_threshold(void)
{
  return (float)(max17048_hibthr_tracking * MAX17048_HIBTHR_RESOLUTION);
}

/***************************************************************************//**
 *  Set the hibernate threshold level in minivolts.
 ******************************************************************************/
sl_status_t max17048_set_activity_threshold(uint32_t act_thr)
{
  sl_status_t status;
  uint8_t buffer[2];

  if (act_thr > MAX17048_ACTTHR_MV) {
    act_thr = MAX17048_ACTTHR_MV;
  }

  // Update the private global variable to track
  max17048_actthr_tracking = (uint8_t)(act_thr * 1000 / MAX17048_ACTTHR_RESOLUTION);
  buffer[0] = max17048_hibthr_tracking;
  buffer[1] = max17048_actthr_tracking;
  status = max17048_write_register(MAX17048_HIBRT, buffer);

  return status;
}

/***************************************************************************//**
 *  Get the activity threshold level in minivolts.
 ******************************************************************************/
uint32_t max17048_get_activity_threshold(void)
{
  return (uint32_t)(max17048_actthr_tracking * MAX17048_ACTTHR_RESOLUTION / 1000);
}

/***************************************************************************//**
 *  Enables/disables the analog comparator in hibernate mode.
 ******************************************************************************/
sl_status_t max17048_enable_reset_comparator(bool enable)
{
  sl_status_t status;
  uint8_t buffer[2];

  if (enable) {
    // VRESET_DIS = 1 to disable the comparator in hibernate mode
    max17048_vreset_tracking |= MAX17048_VRESET_DIS;
  } else {
    // VRESET_DIS = 0 to keep the comparator enabled in hibernate mode
    max17048_vreset_tracking &= ~MAX17048_VRESET_DIS;
  }

  // Update the VRESET register
  buffer[0] = max17048_vreset_tracking;
  // Writing to the ID register has no effect
  buffer[1] = 0x00;
  status = max17048_write_register(MAX17048_VRESET_ID, buffer);

  return status;
}

/***************************************************************************//**
 *  Force the MAX17048 to enter sleep mode.
 ******************************************************************************/
sl_status_t max17048_enter_sleep(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  // Set EnSleep bit in the upper byte of the MODE register to enable sleep mode
  buffer[0] = MAX17048_MODE_ENSLEEP;
  // Lower byte of MODE is not implemented; writing to it has no effect
  buffer[1] = 0x00;
  status = max17048_write_register(MAX17048_MODE, buffer);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Get the upper byte of the CONFIG register
  buffer[0] = max17048_rcomp_tracking;
  max17048_config_lower_tracking |= MAX17048_CONFIG_SLEEP;
  // Get the lower byte of the CONFIG register
  buffer[1] = max17048_config_lower_tracking;
  status = max17048_write_register(MAX17048_CONFIG, buffer);

  return status;
}

/***************************************************************************//**
 *  Forces the MAX17048 to exit sleep mode.
 ******************************************************************************/
sl_status_t max17048_exit_sleep(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  // Get the upper byte of the CONFIG register
  buffer[0] = max17048_rcomp_tracking;
  // Clear SLEEP in the lower byte of the CONFIG register to exit sleep mode
  max17048_config_lower_tracking &= ~MAX17048_CONFIG_SLEEP;
  // Get the lower byte of the CONFIG register
  buffer[1] = max17048_config_lower_tracking;
  status = max17048_write_register(MAX17048_CONFIG, buffer);

  return status;
}

/***************************************************************************//**
 *  Force the MAX17048 to initiate a power-on reset (POR).
 ******************************************************************************/
sl_status_t max17048_force_reset(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  // Force all tracking variables to their default values
  max17048_config_lower_tracking = 0x1C;
  max17048_rcomp_tracking = 0x97;
  max17048_valrt_max_tracking = 0xFF;
  max17048_valrt_min_tracking = 0x00;
  max17048_vreset_tracking = 0x96;
  max17048_hibthr_tracking = 0x80;
  max17048_actthr_tracking = 0x30;

  buffer[0] = MAX17048_RESET_UPPER_BYTE;
  buffer[1] = MAX17048_RESET_LOWER_BYTE;
  status = max17048_write_register(MAX17048_CMD, buffer);

  return status;
}

/***************************************************************************//**
 *  Force the MAX17048 to initiate a battery quick start.
 ******************************************************************************/
sl_status_t max17048_force_quick_start(void)
{
  sl_status_t status;

#ifndef MAX17048_CONFIG_ENABLE_HW_QSTRT
  uint8_t buffer[2];

  // Lower byte of MODE is not implemented; writing to it has no effect.
  buffer[1] = 0;

  /*
   * Note that when forcing a quickstart, the device cannot be in sleep
   * mode because the system is assumed to have just powered and the OCV
   * is still settling.  It is thus permissible to write EnSleep = 0.
   */
  buffer[0] = MAX17048_MODE_QUICK_START;
  status = max17048_write_register(MAX17048_MODE, buffer);
#else
  // Assert GPIO pin connected to QSTRT and delay 1 ms.
  GPIO_PinOutSet(MAX17048_CONFIG_ENABLE_QSTRT_PORT,
                 MAX17048_CONFIG_ENABLE_QSTRT_PIN);

  status = sl_sleeptimer_start_timer_ms(&max17048_quick_start_timer_handle,
                                        1,
                                        max17048_quick_start_callback,
                                        (void *)NULL,
                                        0,
                                        0);
#endif /* MAX17048_CONFIG_ENABLE_HW_QSTRT */

  return status;
}

/***************************************************************************//**
 *  Load a custom model into the MAX17048 and enable it.
 ******************************************************************************/
sl_status_t max17048_load_model(const uint8_t *model)
{
  sl_status_t status;

  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef result;

  uint8_t buffer[2];
  uint8_t i2c_write_data[65];
  uint8_t i2c_read_data[1];
  uint8_t i;

  // Unlock the model
  buffer[0] = MAX17048_UNLOCK_UPPER_BYTE;
  buffer[1] = MAX17048_UNLOCK_LOWER_BYTE;

  status = max17048_write_register(MAX17048_LOCK_TABLE, buffer);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Write the model
  seq.addr = MAX17048_I2C_ADDRESS << 1;
  seq.flags = I2C_FLAG_WRITE;

  // Copy model and table start address into transfer buffer
  i2c_write_data[0] = MAX17048_TABLE;

  for (i = 0; i < 64; i++) {
    i2c_write_data[i + 1] = model[i];
  }

  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len  = 65;

  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len  = 0;

  /*
   * Invoke sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1)
   * to prevent the EFM32/EFR32 from entering energy mode EM2
   * or lower during I2C bus activity.
   */
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif
  result = I2CSPM_Transfer(max17048_i2cspm_instance, &seq);
  /*
   * Call sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1)
   * to remove the requirement to remain in EM1 or higher
   * after I2C bus activity is complete.
   */
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif

  if (result != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }

  // Lock and load the new model
  buffer[0] = 0x0;
  buffer[1] = 0x0;

  status = max17048_write_register(MAX17048_LOCK_TABLE, buffer);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Read and clear RI bit if it is set
  max17048_clear_reset_indicator_bit();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Read the MAX17048 device ID.
 ******************************************************************************/
sl_status_t max17048_get_id(uint8_t *id)
{
  sl_status_t status;
  uint8_t buffer[2];

  status = max17048_read_register(MAX17048_VRESET_ID, buffer);
  *id = buffer[1];

  return status;
}

/***************************************************************************//**
 *  Read the MAX17048 production version.
 ******************************************************************************/
sl_status_t max17048_get_production_version(uint16_t *ver)
{
  sl_status_t status;
  uint8_t buffer[2];

  status = max17048_read_register(MAX17048_VERSION, buffer);
  // Return VERSION bytes in the correct order.
  *ver = (buffer[0] << 8) | buffer[1];

  return status;
}

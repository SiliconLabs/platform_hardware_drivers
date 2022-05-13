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
#include "em_gpio.h"
#include "gpiointerrupt.h"
#include "sl_sleeptimer.h"

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

// Global variables
static sl_i2cspm_t  *max17048_i2cspm_instance;
static bool         max17048_is_initialized = false;
static uint8_t      max17048_athd_tracking = 0x1C;
static uint8_t      max17048_rcomp_tracking = 0x97;
static uint8_t      max17048_valrt_max_tracking = 0xFF;
static uint8_t      max17048_valrt_min_tracking = 0x00;
static uint8_t      max17048_vreset_tracking = 0x96;
static uint8_t      max17048_hibthr_tracking = 0x80;
static uint8_t      max17048_actthr_tracking = 0x30;
static uint32_t     max17048_rcomp_update_interval = MAX17048_CONFIG_RCOMP_UPDATE_INTERVAL_MS;

static sl_sleeptimer_timer_handle_t   max17048_temp_timer_handle;
static sl_sleeptimer_timer_handle_t   max17048_quick_start_timer_handle;
max17048_temp_callback_t              max17048_temp_callback;
max17048_interrupt_callback_t         max17048_interrupt_callback[5];
void *max17048_callback_data[5];

// Function prototypes (private API)
static sl_status_t max17048_read_register_block(uint8_t reg_addr,
                                                uint8_t *data,
                                                uint16_t length);
static sl_status_t max17048_write_register_block(uint8_t reg_addr,
                                                 const uint8_t *data,
                                                 uint16_t length);
static void max17048_temp_timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data);
static void max17048_alrt_pin_callback(uint8_t pin);
static void max17048_quick_start_callback(sl_sleeptimer_timer_handle_t *handle, void *data);
static sl_status_t max17048_set_rcomp(uint8_t rcomp);
static sl_status_t max17048_get_alert_condition(uint8_t *alert_condition);
static sl_status_t max17048_clear_alert_condition(uint8_t alert_condition,
                                           sl_max17048_irq_source_t source);
static sl_status_t max17048_clear_alert_status_bit(void);
static sl_status_t max17048_clear_reset_indicator_bit(void);

/** @endcond */

/***************************************************************************//**
 * @brief
 *    Read a block of data from the MAX17048.
 *
 * @param[in] reg_addr
 *    The first register to begin reading from
 *
 * @param[in] length
 *    The number of bytes to read
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
  static sl_status_t max17048_read_register_block(uint8_t reg_addr,
                                                uint8_t *data,
                                                uint16_t length)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef result;
  uint8_t i2c_write_data[1];

  seq.addr = MAX17048_I2C_ADDRESS << 1;
  seq.flags = I2C_FLAG_WRITE_READ;
  /* Select register to start reading from */
  i2c_write_data[0] = reg_addr;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len  = 1;
  /* Select length of data to be read */
  seq.buf[1].data = data;
  seq.buf[1].len  = length;

  result = I2CSPM_Transfer(max17048_i2cspm_instance, &seq);
  if (result != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *    Write a block of data to the MAX17048.
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
  static sl_status_t max17048_write_register_block(uint8_t reg_addr,
                                                 const uint8_t *data,
                                                 uint16_t length)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef result;
  uint8_t i2c_write_data[length + 1];
  uint8_t i2c_read_data[1];
  uint8_t i;

  seq.addr        = MAX17048_I2C_ADDRESS << 1;
  seq.flags       = I2C_FLAG_WRITE;
  /* Select register to start writing to */
  i2c_write_data[0] = reg_addr;
  for ( i = 0; i < length; i++ ) {
    i2c_write_data[i + 1] = data[i];
  }
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len  = length + 1;
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len  = 0;

  result = I2CSPM_Transfer(max17048_i2cspm_instance, &seq);
  if (result != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *    This function adjusts RCOMP to optimize IC performance for different
 *    lithium chemistries or different operating temperatures.
 *
 * @param[in] rcomp
 *    The compensation resistance.
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
  buffer[1] = max17048_athd_tracking;
  buffer[1] |= (1 << MAX17048_CONFIG_ALRT_BIT); // Writing 1 to ALRT bit does not actually set ALRT
  status = max17048_write_register_block(MAX17048_CONFIG, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Callback when sleep timer expires.
 ******************************************************************************/
static void max17048_temp_timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;
  uint8_t temp;
  uint8_t rcomp;

  temp = max17048_temp_callback();

  if (temp > 20) {
    rcomp = RCOMP0 + (temp - 20) * TEMP_CO_UP;
  } else {
    rcomp = RCOMP0 + (temp - 20) * TEMP_CO_DOWN;
  }

  max17048_set_rcomp(rcomp);
}

/***************************************************************************//**
 *  Callback when sleep timer expires.
 ******************************************************************************/
static void max17048_quick_start_callback(sl_sleeptimer_timer_handle_t *handle,
                                 void *data)
{
  (void)handle;
  (void)data;

  GPIO_PinOutClear(MAX17048_CONFIG_ENABLE_QSTRT_PORT,
                   MAX17048_CONFIG_ENABLE_QSTRT_PIN);
}

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
  max17048_interrupt_callback_t callback;
  uint8_t alert_condition = 0;

  if (pin == MAX17048_CONFIG_ALRT_PIN) {
    //check alert condition
    max17048_get_alert_condition(&alert_condition);

    if ((alert_condition & MAX17048_STATUS_VR) != 0) {
      // Process RESET alert callback because battery
      // has changed or there has been POR
      callback = max17048_interrupt_callback[IRQ_RESET];
      callback(IRQ_RESET, max17048_callback_data[IRQ_RESET]);
      max17048_clear_alert_condition(alert_condition, IRQ_RESET);
    }
    else if ((alert_condition & MAX17048_STATUS_HD) != 0) {
      // Cell nearing empty; may need to place system in ultra-low-power state
      callback = max17048_interrupt_callback[IRQ_EMPTY];
      callback(IRQ_EMPTY, max17048_callback_data[IRQ_EMPTY]);
      max17048_clear_alert_condition(alert_condition, IRQ_EMPTY);
    }
    else if ((alert_condition & MAX17048_STATUS_VL) != 0) {
      // Voltage low alert; may need to set parameters for reduced
      // energy use before reaching empty threshold
      callback = max17048_interrupt_callback[IRQ_VCELL_LOW];
      callback(IRQ_VCELL_LOW, max17048_callback_data[IRQ_VCELL_LOW]);
      max17048_clear_alert_condition(alert_condition, IRQ_VCELL_LOW);
    }
    else if ((alert_condition & MAX17048_STATUS_VH) != 0) {
      // Voltage high alert; may indicate battery is full charged
      // and need to place charging IC in maintenance/trickle charge state
      callback = max17048_interrupt_callback[IRQ_VCELL_HIGH];
      callback(IRQ_VCELL_HIGH, max17048_callback_data[IRQ_VCELL_HIGH]);
      max17048_clear_alert_condition(alert_condition, IRQ_VCELL_HIGH);
    }
    else if ((alert_condition & MAX17048_STATUS_SC) != 0) {
      // SOC changed by 1%; lowest priority interrupt
      // Clear
      callback = max17048_interrupt_callback[IRQ_SOC];
      callback(IRQ_SOC, max17048_callback_data[IRQ_SOC]);
      max17048_clear_alert_condition(alert_condition, IRQ_SOC);
    }

    // Clears the alert status bit to release the ALRT pin.
    max17048_clear_alert_status_bit();
  }
}

/***************************************************************************//**
 * @brief This function identifies which alert condition was met.
 *
 * @param[out] alert_condition
 *   These bits are set when they are cause an alert. Values:
 *   - <b>Bit 0:</b> (voltage high) is set when VCELL has been above
 *     ALRT.VALRTMAX.
 *   - <b>Bit 1:</b> (voltage low) is set when VCELL has been below
 *     ALRT.VALRTMIN.
 *   - <b>Bit 2:</b> (voltage reset) is set after the device has been reset if
 *     EnVr is set.
 *   - <b>Bit 3:</b> (SOC low) is set when SOC crosses the value in CONFIG.ATHD.
 *   - <b>Bit 4:</b> (1% SOC change) is set when SOC changes by at least 1% if
 *     CONFIG.ALSC is set.
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
static sl_status_t max17048_get_alert_condition(uint8_t *alert_condition)
{
  sl_status_t status;
  uint8_t buffer[2];

  status = max17048_read_register_block(MAX17048_STATUS, buffer, 2);
  if (status != SL_STATUS_OK) {
    return status;
  }

  *alert_condition = (buffer[0] >> 1) & 0x1F;

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
  // Clear ALRT bit in the LSB-CONFIG register to service and deassert the ALRT pin
  buffer[1] = max17048_athd_tracking & ~(1 << MAX17048_CONFIG_ALRT_BIT);
  status = max17048_write_register_block(MAX17048_CONFIG, (uint8_t*) buffer, 2);

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

  status = max17048_read_register_block(MAX17048_STATUS, buffer, 2);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Clear RI bit in the MSB-STATUS register if it is set
  if ((buffer[0] & 0x01) != 0) {
    buffer[0] &= ~(1 << 0);
    status = max17048_write_register_block(MAX17048_STATUS, (uint8_t*) buffer, 2);
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
                                           sl_max17048_irq_source_t source)
{
  sl_status_t status;
  uint8_t buffer[2];

  buffer[0] = alert_condition & ~(1 << source);
  buffer[0] <<= 1;
  status = max17048_write_register_block(MAX17048_STATUS, (uint8_t*) buffer, 2);

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
  /* register the callback function that is invoked when interrupt occurs */
  GPIOINT_CallbackRegister(MAX17048_CONFIG_ALRT_PIN, max17048_alrt_pin_callback);

#if (MAX17048_CONFIG_ENABLE_HW_QSTRT)
  if (MAX17048_CONFIG_ENABLE_HW_QSTRT) {
    GPIO_PinModeSet(MAX17048_CONFIG_ENABLE_QSTRT_PORT,
                    MAX17048_CONFIG_ENABLE_QSTRT_PIN,
                    gpioModePushPull,
                    0);
  }
#endif

  /* The driver calculates and updates the RCOMP factor at a rate of
  * 1000 ms <= MAX17048_CONFIG_RCOMP_UPDATE_INTERVAL_MS <= 60000 ms
  * and defaults to 1 minute (60000 ms = 1 minute).
  */
  if ((max17048_rcomp_update_interval < 1000) || (max17048_rcomp_update_interval > 60000)) {
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

  if (MAX17048_CONFIG_ENABLE_HW_QSTRT) {
    GPIO_PinModeSet(MAX17048_CONFIG_ENABLE_QSTRT_PORT,
                    MAX17048_CONFIG_ENABLE_QSTRT_PIN,
                    gpioModeDisabled ,
                    0);
  }

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

  status = max17048_read_register_block(MAX17048_VCELL, buffer, 2);
  vcell_reg_val = (buffer[0] << 8) | buffer[1];
  *vcell = (uint64_t) vcell_reg_val * MAX17048_VCELL_RESOLUTION / 1000000;

  return status;
}

/***************************************************************************//**
 *  Read the SOC register and return the state-of-charge as an integer (0 - 100%).
 ******************************************************************************/
sl_status_t max17048_get_soc(uint32_t *soc)
{
  sl_status_t status;
  uint8_t buffer[2];
  uint16_t soc_reg_val;

  status = max17048_read_register_block(MAX17048_SOC, buffer, 2);
  soc_reg_val = (buffer[0] << 8) | buffer[1];
  *soc = soc_reg_val / MAX17048_SOC_RESOLUTION;

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

  status = max17048_read_register_block(MAX17048_CRATE, buffer, 2);
  crate_reg_val = (buffer[0] << 8) | buffer[1];
  *crate = (float) crate_reg_val * MAX17048_CRATE_RESOLUTION;

  return status;
}

/***************************************************************************//**
 *  Register the temperature update callback for the MAX17048 driver
 ******************************************************************************/
sl_status_t max17048_register_temperature_callback(
    max17048_temp_callback_t temp_cb)
{
  sl_status_t status;
  uint32_t interval;

  if (max17048_temp_callback == temp_cb) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  if (temp_cb == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  max17048_temp_callback = temp_cb;

  interval = max17048_get_update_interval();
  status = sl_sleeptimer_start_periodic_timer_ms(&max17048_temp_timer_handle,
                                                 interval,
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

  max17048_rcomp_update_interval = sl_sleeptimer_ms_to_tick((uint16_t)interval);
  status = sl_sleeptimer_restart_periodic_timer(&max17048_temp_timer_handle,
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

  if (irq_cb == NULL || cb_data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (max17048_interrupt_callback[IRQ_SOC] != NULL
      || max17048_callback_data[IRQ_SOC] != NULL) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  max17048_interrupt_callback[IRQ_SOC] = irq_cb;
  max17048_callback_data[IRQ_SOC] = cb_data;
  
  buffer[0] = max17048_rcomp_tracking;  // Get MSB-CONFIG register
  // Set ALSC bit in the LSB-CONFIG register to enable alerting when SOC changes
  max17048_athd_tracking |= (1 << MAX17048_CONFIG_ALSC_BIT);
  buffer[1] = max17048_athd_tracking | (1 << MAX17048_CONFIG_ALRT_BIT); // Writing 1 to ALRT bit does not actually set ALRT

  status = max17048_write_register_block(MAX17048_CONFIG, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Disable alerting when SOC changes.
 ******************************************************************************/
sl_status_t max17048_disable_soc_interrupt(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  if (max17048_interrupt_callback[IRQ_SOC] == NULL
      || max17048_callback_data[IRQ_SOC] == NULL) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  buffer[0] = max17048_rcomp_tracking;  // Get MSB-CONFIG register
  // Clear ALSC bit in the LSB-CONFIG register to disable alerting when SOC changes
  max17048_athd_tracking &= ~(1 << MAX17048_CONFIG_ALSC_BIT);
  buffer[1] = max17048_athd_tracking | (1 << MAX17048_CONFIG_ALRT_BIT); // Writing 1 to ALRT bit does not actually set ALRT
  status = max17048_write_register_block(MAX17048_CONFIG, (uint8_t*) buffer, 2);

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

  if (max17048_interrupt_callback[IRQ_EMPTY] != NULL
      || max17048_callback_data[IRQ_EMPTY] != NULL) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  max17048_interrupt_callback[IRQ_EMPTY] = irq_cb;
  max17048_callback_data[IRQ_EMPTY] = cb_data;

  buffer[0] = max17048_rcomp_tracking; // Get the MSB-CONFIG register
  // Update the private global variable to track
  max17048_athd_tracking &= 0xE0;
  max17048_athd_tracking |= (32 - athd) & 0x1F;
  buffer[1] = max17048_athd_tracking; // Update the LSB-CONFIG register

  status = max17048_write_register_block(MAX17048_CONFIG, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 * Disable the empty alert interrupt by setting its threshold to 1%
 ******************************************************************************/
sl_status_t max17048_disable_empty_interrupt(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  buffer[0] = max17048_rcomp_tracking; // Get the MSB-CONFIG register
  // Update the private global variable to track
  max17048_athd_tracking &= 0xE0;
  max17048_athd_tracking |= 0x1F;
  buffer[1] = max17048_athd_tracking; // Update the LSB-CONFIG register

  status = max17048_write_register_block(MAX17048_CONFIG, (uint8_t*) buffer, 2);
  if (status != SL_STATUS_OK) {
    return status;
  }

  return status;
}

/***************************************************************************//**
 *  Set the empty alert threshold.
 ******************************************************************************/
sl_status_t max17048_set_empty_threshold(uint8_t athd)
{
  sl_status_t status;
  uint8_t buffer[2];

  if((athd == 0) || (athd > 32)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  buffer[0] = max17048_rcomp_tracking; // Get the MSB-CONFIG register
  // Update the private global variable to track
  max17048_athd_tracking &= 0xE0;
  max17048_athd_tracking |= (32 - athd) & 0x1F;
  buffer[1] = max17048_athd_tracking; // Update the LSB-CONFIG register

  status = max17048_write_register_block(MAX17048_CONFIG, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Get the empty threshold.
 ******************************************************************************/
uint8_t max17048_get_empty_threshold(void)
{
  return (32 - (max17048_athd_tracking & 0x1F));
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

  if((irq_cb == NULL) || (cb_data == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  if(max17048_interrupt_callback[IRQ_VCELL_HIGH] != NULL
      || max17048_callback_data[IRQ_VCELL_HIGH] != NULL) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  max17048_interrupt_callback[IRQ_VCELL_HIGH] = irq_cb;
  max17048_callback_data[IRQ_VCELL_HIGH] = cb_data;

  if (valrt_max_mv > MAX17048_VALRT_MAX_MV) {
    valrt_max_mv = MAX17048_VALRT_MAX_MV;
  }

  buffer[0] = max17048_valrt_min_tracking; // Get the VALRT.MIN register
  // Update the private global variable to track
  max17048_valrt_max_tracking = valrt_max_mv / MAX17048_VALRT_RESOLUTION;
  buffer[1] = max17048_valrt_max_tracking; // Update the VALRT.MAX register
  status = max17048_write_register_block(MAX17048_VALRT, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Disable the voltage high alert interrupt and unregister the
 *  user-provided callback function.
 ******************************************************************************/
sl_status_t max17048_disable_vhigh_interrupt(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  buffer[0] = max17048_valrt_min_tracking; // Get the VALRT.MIN register
  // Update the private global variable to track
  max17048_valrt_max_tracking = 0xFF;
  buffer[1] = max17048_valrt_max_tracking; // Update the VALRT.MAX register
  status = max17048_write_register_block(MAX17048_VALRT, (uint8_t*) buffer, 2);

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

  buffer[0] = max17048_valrt_min_tracking; // Get the VALRT.MIN register
  // Update the private global variable to track
  max17048_valrt_max_tracking = valrt_max_mv / MAX17048_VALRT_RESOLUTION;
  buffer[1] = max17048_valrt_max_tracking; // Update the VALRT.MAX register
  status = max17048_write_register_block(MAX17048_VALRT, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Get the voltage high alert interrupt threshold in millivolts.
 ******************************************************************************/
uint32_t max17048_get_vhigh_threshold(void)
{
  return (uint32_t) max17048_valrt_max_tracking * MAX17048_VALRT_RESOLUTION;
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

  if((irq_cb == NULL) || (cb_data == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  if(max17048_interrupt_callback[IRQ_VCELL_LOW] != NULL
     || max17048_callback_data[IRQ_VCELL_LOW] != NULL) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  max17048_interrupt_callback[IRQ_VCELL_LOW] = irq_cb;
  max17048_callback_data[IRQ_VCELL_LOW] = cb_data;

  if (valrt_min_mv > MAX17048_VALRT_MIN_MV) {
    valrt_min_mv = MAX17048_VALRT_MIN_MV;
  }

  buffer[1] = max17048_valrt_max_tracking; // Get the VALRT.MAX register
  // Update the private global variable to track
  max17048_valrt_min_tracking = valrt_min_mv / MAX17048_VALRT_RESOLUTION;
  buffer[0] = max17048_valrt_min_tracking; // Update the VALRT.MIN register
  status = max17048_write_register_block(MAX17048_VALRT, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Disable the voltage low alert interrupt and unregister the
 *  user-provided callback function.
 ******************************************************************************/
sl_status_t max17048_disable_vlow_interrupt(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  buffer[1] = max17048_valrt_max_tracking; // Get the VALRT.MAX register
  // Update the private global variable to track
  max17048_valrt_min_tracking = 0x00;
  buffer[0] = max17048_valrt_min_tracking; // Update the VALRT.MIN register
  status = max17048_write_register_block(MAX17048_VALRT, (uint8_t*) buffer, 2);

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

  buffer[1] = max17048_valrt_max_tracking; // Get the VALRT.MAX register
  // Update the private global variable to track
  max17048_valrt_min_tracking = valrt_min_mv / MAX17048_VALRT_RESOLUTION;
  buffer[0] = max17048_valrt_min_tracking; // Update the VALRT.MIN register
  status = max17048_write_register_block(MAX17048_VALRT, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Get the voltage low alert interrupt threshold in millivolts.
 ******************************************************************************/
uint32_t max17048_get_vlow_threshold(void)
{
  return (uint32_t) max17048_valrt_min_tracking * MAX17048_VALRT_RESOLUTION;
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

  if (max17048_interrupt_callback[IRQ_RESET] != NULL
      || max17048_callback_data[IRQ_RESET] != NULL) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  if (vreset_mv > MAX17048_VRESET_MV) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  max17048_interrupt_callback[IRQ_RESET] = irq_cb;
  max17048_callback_data[IRQ_RESET] = cb_data;

  status = max17048_read_register_block(MAX17048_STATUS, buffer, 2);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Set EnVR bit in the MSB-STATUS register to enable voltage reset alert
  buffer[0] |= (1 << MAX17048_STATUS_ENVR_BIT);
  status = max17048_write_register_block(MAX17048_STATUS, (uint8_t*) buffer, 2);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Update the private global variable to track
  max17048_vreset_tracking &= 0x01;
  vreset_val = vreset_mv / MAX17048_VRESET_RESOLUTION;
  max17048_vreset_tracking |= (vreset_val << 1);
  buffer[0] = max17048_vreset_tracking; // Update the VRESET register
  buffer[1] = 0x00; // Writing to ID register does not actually change ID
  status = max17048_write_register_block(MAX17048_VRESET_ID, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Disable voltage reset alert.
 ******************************************************************************/
sl_status_t max17048_disable_reset_interrupt(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  status = max17048_read_register_block(MAX17048_STATUS, buffer, 2);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Clear EnVR bit in the MSB-STATUS register to disable voltage reset alert
  buffer[0] &= ~(1 << MAX17048_STATUS_ENVR_BIT);
  status = max17048_write_register_block(MAX17048_STATUS, (uint8_t*) buffer, 2);

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
  max17048_vreset_tracking &= 0x01;
  vreset_val = vreset_mv / MAX17048_VRESET_RESOLUTION;
  max17048_vreset_tracking |= (vreset_val << 1);
  buffer[0] = max17048_vreset_tracking; // Update the VRESET register
  buffer[1] = 0x00; // Writing to ID register does not actually change ID

  status = max17048_write_register_block(MAX17048_VRESET_ID, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Get the reset alert interrupt threshold in millivolts.
 ******************************************************************************/
uint32_t max17048_get_reset_threshold(void)
{
  return (uint32_t) (max17048_vreset_tracking >> 1) * MAX17048_VRESET_RESOLUTION;
}

/***************************************************************************//**
 *  Read hibernate mode status.
 ******************************************************************************/
sl_status_t max17048_get_hibernate_state(max17048_hibstate_t *hibstat)
{
  sl_status_t status;
  uint8_t buffer[2];

  status = max17048_read_register_block(MAX17048_MODE, buffer, 2);
  // Get HibStat bit in the MSB-MODE register
  *hibstat = (max17048_hibstate_t)((buffer[0] >> MAX17048_MODE_HIBSTAT_BIT) & 0x01);

  return status;
}

/***************************************************************************//**
 *  Set thresholds for entering and exiting hibernate mode.
 ******************************************************************************/
sl_status_t max17048_enable_auto_hibernate(float hib_thr, uint32_t act_thr)
{
  sl_status_t status;
  uint8_t buffer[2];

  if ((hib_thr = 0x0) && (act_thr != 0x0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (hib_thr > MAX17048_HIBTHR_PERCENT) {
    hib_thr = MAX17048_HIBTHR_PERCENT;
  }

  if (act_thr > MAX17048_ACTTHR_MV) {
    act_thr = MAX17048_ACTTHR_MV;
  }

  // Update the private global variables to track
  max17048_hibthr_tracking = hib_thr / MAX17048_HIBTHR_RESOLUTION;
  max17048_actthr_tracking = act_thr * 1000 / MAX17048_ACTTHR_RESOLUTION;

  buffer[0] = max17048_hibthr_tracking;    // Update the HibThr register
  buffer[1] = max17048_actthr_tracking;    // Update the ActThr register
  status = max17048_write_register_block(MAX17048_HIBRT, (uint8_t*) buffer, 2);

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

  buffer[0] = 0x00; // Clear the HibThr register
  buffer[1] = 0x00; // Clear the ActThr register
  status = max17048_write_register_block(MAX17048_HIBRT, (uint8_t*) buffer, 2);

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
  max17048_hibthr_tracking = hib_thr / MAX17048_HIBTHR_RESOLUTION;
  buffer[0] = max17048_hibthr_tracking; // Update the HibThr register
  buffer[1] = max17048_actthr_tracking; // Get the ActThr register
  status = max17048_write_register_block(MAX17048_HIBRT, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Get the hibernate threshold level in percent.
 ******************************************************************************/
float max17048_get_hibernate_threshold(void)
{
  return (float) max17048_hibthr_tracking * MAX17048_HIBTHR_RESOLUTION;
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
  max17048_actthr_tracking = act_thr * 1000 / MAX17048_ACTTHR_RESOLUTION;
  buffer[0] = max17048_hibthr_tracking; // Get the HibThr register
  buffer[1] = max17048_actthr_tracking; // Update the ActThr register
  status = max17048_write_register_block(MAX17048_HIBRT, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Get the activity threshold level in minivolts.
 ******************************************************************************/
uint32_t max17048_get_activity_threshold(void)
{
  return (uint32_t) max17048_actthr_tracking * MAX17048_ACTTHR_RESOLUTION / 1000;
}

/***************************************************************************//**
 *  Enables/disables the analog comparator in hibernate mode.
 ******************************************************************************/
sl_status_t max17048_enable_reset_comparator(bool enable)
{
  sl_status_t status;
  uint8_t buffer[2];

  if (enable) {
    // Set DIS bit in the VRESET register to o disable 
    // the analog comparator in hibernate mode
    max17048_vreset_tracking |= (1 << MAX17048_VRESET_DIS_BIT);
  } else {
    // Clear DIS bit in the VRESET register to o enable 
    // the analog comparator in hibernate mode
    max17048_vreset_tracking  &= ~(1 << MAX17048_VRESET_DIS_BIT);
  }

  buffer[0] = max17048_vreset_tracking;
  status = max17048_write_register_block(MAX17048_VRESET_ID, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Forces the MAX17048 to enter sleep mode.
 ******************************************************************************/
sl_status_t max17048_enter_sleep(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  // Set EnSleep bit in the MSB-MODE register to enable sleep mode
  buffer[0] = 1 << MAX17048_MODE_ENSLEEP_BIT;
  status = max17048_write_register_block(MAX17048_MODE, (uint8_t*) buffer, 2);
  if (status != SL_STATUS_OK) {
    return status;
  }

  buffer[0] = max17048_rcomp_tracking;  // Get MSB-CONFIG register
  // Set SLEEP bit in the LSB-CONFIG register 
  // to force the IC to enter sleep mode
  max17048_athd_tracking |= (1 << MAX17048_CONFIG_SLEEP_BIT);
  buffer[1] = max17048_athd_tracking | (1 << MAX17048_CONFIG_ALRT_BIT); // Writing 1 to ALRT bit does not actually set ALRT
  status = max17048_write_register_block(MAX17048_CONFIG, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Forces the MAX17048 to exit sleep mode.
 ******************************************************************************/
sl_status_t max17048_exit_sleep(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  buffer[0] = max17048_rcomp_tracking;  // Get MSB-CONFIG register
  // Clear SLEEP bit in the LSB-CONFIG register 
  // to force the IC to exit sleep mode
  max17048_athd_tracking &= ~(1 << MAX17048_CONFIG_SLEEP_BIT);
  buffer[1] = max17048_athd_tracking | (1 << MAX17048_CONFIG_ALRT_BIT); // Writing 1 to ALRT bit does not actually set ALRT
  status = max17048_write_register_block(MAX17048_CONFIG, (uint8_t*) buffer, 2);

  return status;
}
/***************************************************************************//**
 *  Forces the MAX17048 to initiate a power-on reset (POR).
 ******************************************************************************/
sl_status_t max17048_force_reset(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  buffer[0] = MAX17048_RESET_MSB;
  buffer[1] = MAX17048_RESET_LSB;
  status = max17048_write_register_block(MAX17048_CMD, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Forces the MAX17048 to initiate a battery quick start.
 ******************************************************************************/
sl_status_t max17048_force_quick_start(void)
{
  sl_status_t status;
  uint8_t buffer[2];

  if (MAX17048_CONFIG_ENABLE_HW_QSTRT) {
    GPIO_PinOutSet(MAX17048_CONFIG_ENABLE_QSTRT_PORT, 
                   MAX17048_CONFIG_ENABLE_QSTRT_PIN);

    status = sl_sleeptimer_start_timer_ms(&max17048_quick_start_timer_handle,
                                          1,
                                          max17048_quick_start_callback,
                                          (void *)NULL,
                                          0,
                                          0);
    return status;
  }

  // Set EnSleep bit in the MSB-MODE register to enable sleep mode
  buffer[0] = 1 << MAX17048_MODE_QUICK_START_BIT;
  status = max17048_write_register_block(MAX17048_MODE, (uint8_t*) buffer, 2);

  return status;
}

/***************************************************************************//**
 *  Load a custom model into the MAX17048 and enable it.
 ******************************************************************************/
sl_status_t max17048_load_model(const uint8_t *model)
{
  sl_status_t status;
  uint8_t buffer[2];

  buffer[0] = MAX17048_UNLOCK_MSB;
  buffer[1] = MAX17048_UNLOCK_LSB;
  status = max17048_write_register_block(MAX17048_LOCK_TABLE, (uint8_t*) buffer, 2);
  if (status != SL_STATUS_OK) {
    return status;
  }

  for (uint8_t i = 0; i < 64; i += 2) {
    buffer[0] = model[i];
    buffer[1] = model[i + 1];
    max17048_write_register_block(MAX17048_TABLE + i, (uint8_t*) buffer, 2);
  }

  buffer[0] = 0x00;
  buffer[1] = 0x00;
  status = max17048_write_register_block(MAX17048_LOCK_TABLE, (uint8_t*) buffer, 2);
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

  status = max17048_read_register_block(MAX17048_VRESET_ID, buffer, 2);
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

  status = max17048_read_register_block(MAX17048_VERSION, buffer, 2);
  *ver =  (buffer[0] << 8) | buffer[1];

  return status;
}

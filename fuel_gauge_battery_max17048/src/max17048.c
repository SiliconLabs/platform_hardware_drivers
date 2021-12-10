/***************************************************************************//**
* @file max17048.c
* @brief Driver for the max17048/max17049 Fuel Gauge
********************************************************************************
* # License
* <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "stdio.h"
#include "max17048_config.h"
#include "app_log.h"
#include "gpiointerrupt.h"

max1704x_t *max17048;
static uint8_t empty_threshold = 32;
max17048_temp_callback_t max17048_temp_callback;
max17048_interrupt_callback_t interrupt_callback[5];
void *callback_data[5];
uint16_t rcomp_update_interval = MAX17048_CONFIG_RCOMP_UPDATE_INTERVAL_MS;
static sl_sleeptimer_timer_handle_t temp_timer;
static sl_sleeptimer_timer_handle_t quick_start_timer;
static void temp_callback(sl_sleeptimer_timer_handle_t *handle, void *data);
static void quick_start_callback(sl_sleeptimer_timer_handle_t *handle,
                                 void *data);
static void alrt_pin_callback(uint8_t pin);

/**************************************************************************/ /**
 * @brief This function read register
 *
 * @param[in] max17048 The instance to use.
 * @param[in] target_address Address register to read
 * @param[in] num_bytes Number of bytes to read
 * @param[out] rx_buff Data to read
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
static sl_status_t max17048_read_register(max1704x_t *max17048,
                                          uint8_t target_address,
                                          uint16_t *rx_buff,
                                          uint8_t num_bytes)
{
  // Transfer structure
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef result;
  uint8_t i2c_write_data[1];
  uint8_t value[2];

  if (max17048 == NULL) {
      return SL_STATUS_NULL_POINTER;
  }

  // Initializing I2C transfer
  seq.addr = max17048->I2C_address << 1;

  seq.flags = I2C_FLAG_WRITE_READ; // must write target address before reading
  i2c_write_data[0] = target_address;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len  = 1;
  seq.buf[1].data = value;
  seq.buf[1].len  = num_bytes;

  result = I2CSPM_Transfer(max17048->I2C_fuel_gauge, &seq);
  if (result != i2cTransferDone)
  {
      return SL_STATUS_TRANSMIT;
  }
  else
  {
      *rx_buff = ((uint16_t)value[0]<<8) | value[1];
  }

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 * @brief This function write register
 *
 * @param[in] max17048 The instance to use.
 * @param[in] target_address Address register to write
 * @param[in] data Data to write
 * @param[in] num_bytes Number of bytes to write
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
static sl_status_t max17048_write_register(max1704x_t *max17048,
                                           uint8_t target_address,
                                           uint16_t data,
                                           uint8_t num_bytes)
{
  // Transfer structure
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef result;
  uint8_t i2c_write_data[3];
  uint8_t i2c_read_data[1];

  if (max17048 == NULL) {
      return SL_STATUS_NULL_POINTER;
  }

  i2c_write_data[0] = target_address;
  i2c_write_data[1] = (uint8_t)(data >> 8);
  i2c_write_data[2] = (uint8_t)(data);

  // Initializing I2C transfer
  seq.addr        = max17048->I2C_address << 1;
  seq.flags       = I2C_FLAG_WRITE;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len  = num_bytes + 1;

  /* Select length of data to be read */
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len  = 0;

  result = I2CSPM_Transfer(max17048->I2C_fuel_gauge, &seq);
  if (result != i2cTransferDone)
  {
      return SL_STATUS_TRANSMIT;
  }

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 *  Initializes the max17048/max1749 - fuel gauge.
 ******************************************************************************/
sl_status_t max17048_init()
{
  sl_status_t status;
  static max1704x_t init = FUEL_GAUGE_DEFAULT;

  //  delay 170 milliseconds for stable
  sl_sleeptimer_delay_millisecond(170);

  if (max17048 != NULL) {
      return SL_STATUS_ALREADY_INITIALIZED;
  }

  max17048 = &init;
  GPIOINT_Init();
  GPIO_PinModeSet(MAX17048_CONFIG_ALRT_PORT,
                  MAX17048_CONFIG_ALRT_PIN,
                  gpioModeInputPullFilter,
                  1);

  if (MAX17048_CONFIG_ENABLE_HW_QSTRT) {
      GPIO_PinModeSet(MAX17048_CONFIG_ENABLE_QSTRT_PORT,
                      MAX17048_CONFIG_ENABLE_QSTRT_PIN,
                      gpioModePushPull,
                      0);
  }

  TEMPDRV_Init();
  max17048_register_temperature_callback(((max17048_temp_callback_t)((int32_t)TEMPDRV_GetTemp)));

  return status;
}

/***************************************************************************//**
 *  De-initialize the MAX17048 driver.
 ******************************************************************************/
sl_status_t max17048_deinit(void)
{
  if (max17048 == NULL) {
      return SL_STATUS_NOT_INITIALIZED;
  }

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

  max17048_disable_soc_interrupt();
  max17048_disable_empty_interrupt();
  max17048_disable_vhigh_interrupt();
  max17048_disable_vlow_interrupt();
  max17048_disable_reset_interrupt();
  max17048_enter_sleep();

  return SL_STATUS_OK;
}
/**************************************************************************/ /**
 *  Read the cell voltage.
 ******************************************************************************/
sl_status_t max17048_get_vcell(uint32_t *vcell)
{
  sl_status_t status;

  status = max17048_read_register(max17048,
                                  MAX17048_VCELL,
                                  (uint16_t*)vcell,
                                  2);

  return status;
}

/**************************************************************************/ /**
 *  Read the cell voltage.
 ******************************************************************************/
sl_status_t max17048_get_vcell_vol(float *vcell)
{
  sl_status_t status;
  uint16_t value;

  status = max17048_read_register(max17048, MAX17048_VCELL, &value, 2);

  *vcell = ((float)value / 1000000000) * MAX17048_VCELL_RESOLUTION;

  return status;
}

/**************************************************************************/ /**
 *  Read the SOC in integer.
 ******************************************************************************/
sl_status_t max17048_get_soc(uint32_t *soc)
{
  sl_status_t status;
  uint16_t value;

  status = max17048_read_register(max17048, MAX17048_SOC, &value, 2);

  *soc = value >> 8;

  return status;
}

/**************************************************************************/ /**
 *  Read the SOC rate.
 ******************************************************************************/
sl_status_t max17048_get_crate(float *crate)
{
  sl_status_t status;
  uint16_t value;

  status = max17048_read_register(max17048, MAX17048_CRATE, &value, 2);

  *crate = (((int16_t)value)*0.208)/100.0;

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
  status = sl_sleeptimer_start_periodic_timer_ms(&temp_timer,
                                                 interval,
                                                 temp_callback,
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
      return SL_STATUS_INVALID_PARAMETER;
  }

  rcomp_update_interval = sl_sleeptimer_ms_to_tick((uint16_t)interval);
  status = sl_sleeptimer_restart_timer(&temp_timer,
                                       rcomp_update_interval,
                                       temp_callback,
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
  return rcomp_update_interval;
}

static void alrt_pin_callback(uint8_t pin)
{
  max17048_interrupt_callback_t callback;
  uint8_t alert_condition;

  if (pin == MAX17048_CONFIG_ALRT_PIN) {
      //check alert condition
      max17048_get_alert_condition(&alert_condition);
      for (int i= 0; i < 5; i++) {
          if ((alert_condition >> i ) & 0x01) {
              callback = interrupt_callback[i];
              callback(i, callback_data[i]);
          }
      }
      max17048_clear_alert_condition();
      max1704x_clear_alert_status();
  }
}

/***************************************************************************//**
 *  Mask MAX17048 interrupts
 ******************************************************************************/
void max17048_mask_interrupts(void)
{
  GPIO_ExtIntConfig(MAX17048_CONFIG_ALRT_PORT,
                    MAX17048_CONFIG_ALRT_PIN,
                    MAX17048_CONFIG_ALRT_PIN,
                    false,
                    false,
                    false);
  GPIOINT_CallbackUnRegister(MAX17048_CONFIG_ALRT_PIN);
}

/***************************************************************************//**
 *  Unmask MAX17048 interrupts
 ******************************************************************************/
void max17048_unmask_interrupts(void)
{
  GPIO_ExtIntConfig(MAX17048_CONFIG_ALRT_PORT,
                    MAX17048_CONFIG_ALRT_PIN,
                    MAX17048_CONFIG_ALRT_PIN,
                    false,
                    true,
                    true);
  GPIOINT_CallbackRegister(MAX17048_CONFIG_ALRT_PIN, alrt_pin_callback);
}

/**************************************************************************/ /**
 *  Enable/disable alerting when SOC changes.
 ******************************************************************************/
sl_status_t max17048_enable_soc_interrupt(max17048_interrupt_callback_t irq_cb,
                                          void *cb_data)
{
  sl_status_t status;
  uint16_t value;

  if (irq_cb == NULL || cb_data == NULL) {
      return SL_STATUS_NULL_POINTER;
  }

  if (interrupt_callback[IRQ_SOC] != NULL || callback_data[IRQ_SOC] != NULL) {
      return SL_STATUS_ALREADY_INITIALIZED;
  }

  interrupt_callback[IRQ_SOC] = irq_cb;
  callback_data[IRQ_SOC] = cb_data;
  status = max17048_read_register(max17048, MAX17048_CONFIG, &value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  status = max17048_write_register(
      max17048,
      MAX17048_CONFIG,
      (value | (0x0001 << MAX17048_CONFIG_ALSC_BIT)),
      2);

  return status;
}

/**************************************************************************/ /**
 *  Disable alerting when SOC changes.
 ******************************************************************************/
sl_status_t max17048_disable_soc_interrupt(void)
{
  sl_status_t status;
  uint16_t value;

  if (interrupt_callback[IRQ_SOC] == NULL || callback_data[IRQ_SOC] == NULL) {
      return SL_STATUS_NOT_INITIALIZED;
  }
  status = max17048_read_register(max17048, MAX17048_CONFIG, &value, 2);

  if (status != SL_STATUS_OK) {
      return status;
  }

  status = max17048_write_register(
      max17048,
      MAX17048_CONFIG,
      (value & ~(0x0001 << MAX17048_CONFIG_ALSC_BIT)),
      2);

  return status;
}

/**************************************************************************/ /**
 *  Set the empty threshold.
 ******************************************************************************/
sl_status_t max17048_enable_empty_interrupt(
    uint8_t athd,
    max17048_interrupt_callback_t irq_cb,
    void *cb_data)
{
  sl_status_t status;
  uint16_t value;

  if ((athd < 1) || (athd>32)) {
      return SL_STATUS_INVALID_PARAMETER;
  }

  if ((irq_cb == NULL) || (cb_data == NULL)) {
      return SL_STATUS_NULL_POINTER;
  }

  if (interrupt_callback[IRQ_EMPTY] != NULL
      || callback_data[IRQ_EMPTY] != NULL) {
      return SL_STATUS_ALREADY_INITIALIZED;
  }

  interrupt_callback[IRQ_EMPTY] = irq_cb;
  callback_data[IRQ_EMPTY] = cb_data;

  status = max17048_read_register(max17048, MAX17048_CONFIG, &value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  value &= 0xFFE0;
  value |= athd & 0x1F;
  empty_threshold = 1 - athd;
  status = max17048_write_register(max17048, MAX17048_CONFIG, value, 2);

  return status;
}

/**************************************************************************/ /**
 *  Set the empty threshold.
 ******************************************************************************/
sl_status_t max17048_disable_empty_interrupt()
{
  sl_status_t status;
  uint16_t value;

  status = max17048_read_register(max17048, MAX17048_CONFIG, &value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  value &= 0xFFE0;
  value |= 32;
  empty_threshold = 32;
  status = max17048_write_register(max17048, MAX17048_CONFIG, value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  return status;
}

/**************************************************************************/ /**
 *  Set the empty threshold.
 ******************************************************************************/
sl_status_t max17048_set_empty_threshold( uint8_t athd)
{
  sl_status_t status;
  uint16_t value;

  if((athd == 0) || (athd>32)) {
      return SL_STATUS_INVALID_PARAMETER;
  }

  status = max17048_read_register(max17048, MAX17048_CONFIG, &value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  value &= 0xFFE0;
  value |= athd & 0x1F;
  empty_threshold = athd;
  status = max17048_write_register(max17048, MAX17048_CONFIG, value, 2);

  return status;
}

/***************************************************************************//**
 *  Get the empty threshold.
 ******************************************************************************/
uint8_t max17048_get_empty_threshold(void)
{
  return empty_threshold;
}

/**************************************************************************/ /**
 *  Set thresholds of the voltage alert.
 ******************************************************************************/
sl_status_t max17048_enable_vhigh_interrupt(uint8_t valrt_max,
                                            max17048_interrupt_callback_t irq_cb,
                                            void *cb_data)
{
    sl_status_t status;
    uint16_t value, valert;

    if((irq_cb == NULL) || (cb_data == NULL)) {
        return SL_STATUS_NULL_POINTER;
    }

    if(interrupt_callback[IRQ_VCELL_HIGH] != NULL
       || callback_data[IRQ_VCELL_HIGH] != NULL) {
        return SL_STATUS_ALREADY_INITIALIZED;
    }

    interrupt_callback[IRQ_VCELL_HIGH] = irq_cb;
    callback_data[IRQ_VCELL_HIGH] = cb_data;
    status = max17048_read_register(max17048, MAX17048_VALRT, &valert, 2);
    if (status != SL_STATUS_OK) {
        return status;
    }

    value = (valert & 0xFF00) | valrt_max;
    status = max17048_write_register(max17048, MAX17048_VALRT, value, 2);

    return status;
}

/***************************************************************************//**
 *  Disables the MAX17048 voltage high alert interrupt and
 *  unregisters the user-provided callback function.
 ******************************************************************************/
sl_status_t max17048_disable_vhigh_interrupt(void)
{
  sl_status_t status;
  uint16_t value, valert;

  status = max17048_read_register(max17048, MAX17048_VALRT, &valert, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  value = (valert & 0xFF00) | 0xFF;
  status = max17048_write_register(max17048, MAX17048_VALRT, value, 2);

  return status;
}

/***************************************************************************//**
 *  Set the voltage high alert interrupt threshold.
 ******************************************************************************/
sl_status_t max17048_set_vhigh_threshold(uint8_t valrt_max)
{
  sl_status_t status;
  uint16_t value, valert;

  status = max17048_read_register(max17048, MAX17048_VALRT, &valert, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  value = (valert & 0xFF00) | valrt_max;
  status = max17048_write_register(max17048, MAX17048_VALRT, value, 2);

  return status;
}

/***************************************************************************//**
 *  Get the voltage high alert interrupt threshold.
 ******************************************************************************/
uint8_t max17048_get_vhigh_threshold(void)
{
  uint16_t valert;

  max17048_read_register(max17048, MAX17048_VALRT, &valert, 2);

  return valert & 0xFF;
}

/***************************************************************************//**
 *  Enables the MAX17048 voltage low alert interrupt, sets its threshold,
 *  and registers a user-provided callback function to respond to it.
 ******************************************************************************/
sl_status_t max17048_enable_vlow_interrupt(uint8_t valrt_min,
                                           max17048_interrupt_callback_t irq_cb,
                                           void *cb_data)
{
  sl_status_t status;
  uint16_t value, valert;

  if((irq_cb == NULL) || (cb_data == NULL)) {
      return SL_STATUS_NULL_POINTER;
  }

  if(interrupt_callback[IRQ_VCELL_LOW] != NULL
     || callback_data[IRQ_VCELL_LOW] != NULL) {
      return SL_STATUS_ALREADY_INITIALIZED;
  }

  status = max17048_read_register(max17048, MAX17048_VALRT, &valert, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  interrupt_callback[IRQ_VCELL_LOW] = irq_cb;
  callback_data[IRQ_VCELL_LOW] = cb_data;
  value = (valrt_min << 8) | (valert & 0xFF);
  status = max17048_write_register(max17048, MAX17048_VALRT, value, 2);

  return status;
}

/***************************************************************************//**
 *  Disables the MAX17048 voltage low alert interrupt and
 *  unregisters the user-provided callback function.
 ******************************************************************************/
sl_status_t max17048_disable_vlow_interrupt(void)
{
  sl_status_t status;
  uint16_t value, valert;

  status = max17048_read_register(max17048, MAX17048_VALRT, &valert, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  value = valert & 0xFF;
  status = max17048_write_register(max17048, MAX17048_VALRT, value, 2);

  return status;
}

/***************************************************************************//**
 *  Set the voltage low alert interrupt threshold.
 ******************************************************************************/
sl_status_t max17048_set_vlow_threshold(uint8_t valrt_min)
{
  sl_status_t status;
  uint16_t value, valert;

  status = max17048_read_register(max17048, MAX17048_VALRT, &valert, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  value = (valrt_min << 8) | (valert & 0xFF);
  status = max17048_write_register(max17048, MAX17048_VALRT, value, 2);

  return status;
}

/***************************************************************************//**
 *  Get the voltage low alert interrupt threshold.
 ******************************************************************************/
uint8_t max17048_get_vlow_threshold(void)
{
  uint16_t valert;

  max17048_read_register(max17048, MAX17048_VALRT, &valert, 2);

  return valert >> 8;
}

/**************************************************************************/ /**
 *  Enable or disable voltage reset alert.
 ******************************************************************************/
sl_status_t max17048_enable_reset_interrupt(
    uint8_t vreset,
    max17048_interrupt_callback_t irq_cb,
    void *cb_data)
{
  uint16_t value;
  sl_status_t status;
  uint16_t res_vol;

  if ((irq_cb == NULL) || (cb_data == NULL)) {
      return SL_STATUS_NULL_POINTER;
  }

  if (interrupt_callback[IRQ_RESET] != NULL
      || callback_data[IRQ_RESET] != NULL) {
      return SL_STATUS_ALREADY_INITIALIZED;
  }

  if (vreset > 0x7F) {
      return SL_STATUS_INVALID_PARAMETER;
  }

  interrupt_callback[IRQ_RESET] = irq_cb;
  callback_data[IRQ_RESET] = cb_data;

  status = max17048_read_register(max17048, MAX17048_STATUS, &value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  status = max17048_write_register(
      max17048,
      MAX17048_STATUS,
      (value | (0x0001 << MAX17048_STATUS_ENVR_BIT)),
      2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  status = max17048_read_register(max17048, MAX17048_VRESET_ID, &value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  res_vol = (uint16_t)vreset << 9;
  res_vol |= value;
  status = max17048_write_register(max17048, MAX17048_VRESET_ID, res_vol, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  status = max17048_read_register(max17048, MAX17048_VRESET_ID, &value, 2);

  return status;
}

/**************************************************************************/ /**
 *  Disable voltage reset alert.
 ******************************************************************************/
sl_status_t max17048_disable_reset_interrupt()
{
  uint16_t value;
  sl_status_t status;

  status = max17048_read_register(max17048, MAX17048_STATUS, &value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  status = max17048_write_register(
      max17048,
      MAX17048_STATUS,
      (value & ~(0x0001 << MAX17048_STATUS_ENVR_BIT)),
      2);

  return status;
}

/***************************************************************************//**
 *  Set the reset alert interrupt threshold.
 ******************************************************************************/
sl_status_t max17048_set_reset_threshold(uint8_t vreset)
{
  sl_status_t status;
  uint16_t res_vol, value;

  if (vreset > 0x7F) {
      return SL_STATUS_INVALID_PARAMETER;
  }

  status = max17048_read_register(max17048, MAX17048_VRESET_ID, &value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  res_vol = vreset << 9;
  res_vol |= 0x1F;
  res_vol |= value;
  status = max17048_write_register(max17048, MAX17048_VRESET_ID, res_vol, 2);

  return status;
}

/***************************************************************************//**
 *  Get the reset alert interrupt threshold.
 ******************************************************************************/
uint8_t max17048_get_reset_threshold(void)
{
  uint16_t value;

  max17048_read_register(max17048, MAX17048_VRESET_ID, &value, 2);

  return value >> 9;
}

/**************************************************************************/ /**
 *  Read hibernate mode status.
 ******************************************************************************/
sl_status_t max17048_get_hibe_status(max17048_hibstate_t *hibstat)
{
  sl_status_t status;
  uint16_t value;

  status = max17048_read_register(max17048, MAX17048_MODE, &value, 2);

  *hibstat = (max17048_hibstate_t)((value >> 12) & 0x01);

  return status;
}

/**************************************************************************/ /**
 *  Set thresholds for entering and exiting hibernate mode.
 ******************************************************************************/
sl_status_t max17048_enable_auto_hibernate(uint8_t hib_thr, uint8_t act_thr)
{
    sl_status_t status;
    uint16_t value;

    if ((hib_thr = 0x0) && (act_thr != 0x0)) {
        return SL_STATUS_INVALID_PARAMETER;
    }

    value = ((uint16_t)hib_thr << 8) | act_thr;
    status = max17048_write_register(max17048, MAX17048_HIBRT, value, 2);

    return status;
}

/***************************************************************************//**
 *  Disables automatic hibernation by setting the activity and
 *  CRATE thresholds to 0x0.
 ******************************************************************************/
sl_status_t max17048_disable_auto_hibernate(void)
{
  sl_status_t status;
  uint16_t value;

  value = 0x00;
  status = max17048_write_register(max17048, MAX17048_HIBRT, value, 2);

  return status;
}

/**************************************************************************/ /**
 *  Set the hibernate threshold level.
 ******************************************************************************/
sl_status_t max17048_set_hibernate_threshold(uint8_t hib_thr)
{
  sl_status_t status;
  uint16_t value, thres_val;

  status = max17048_read_register(max17048, MAX17048_HIBRT, &thres_val, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  value = ((uint16_t)hib_thr << 8) | (thres_val & 0xFF);
  status = max17048_write_register(max17048, MAX17048_HIBRT, value, 2);

  return status;
}

/***************************************************************************//**
 *  Get the hibernate threshold level.
 ******************************************************************************/
uint8_t max17048_get_hibernate_threshold(void)
{
  uint16_t thres_val;

  max17048_read_register(max17048, MAX17048_HIBRT, &thres_val, 2);

  return thres_val >> 8;
}

/**************************************************************************/ /**
 *  Set the hibernate threshold level.
 ******************************************************************************/
sl_status_t max17048_set_activity_threshold(uint8_t act_thr)
{
  sl_status_t status;
  uint16_t value, thres_val;

  status = max17048_read_register(max17048, MAX17048_HIBRT, &thres_val, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  value = (thres_val & 0xFF00) | act_thr;

  status = max17048_write_register(max17048, MAX17048_HIBRT, value, 2);

  return status;
}

/***************************************************************************//**
 *  Get the activity threshold level.
 ******************************************************************************/
uint8_t max17048_get_activity_threshold(void)
{
  uint16_t thres_val;

  max17048_read_register(max17048, MAX17048_HIBRT, &thres_val, 2);

  return thres_val & 0xFF;
}

/**************************************************************************/ /**
 *  Adjust RCOMP register
 ******************************************************************************/
sl_status_t max17048_set_rcomp(uint8_t rcomp)
{
  sl_status_t status;
  uint16_t data;

  status = max17048_read_register(max17048, MAX17048_CONFIG, &data, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  data &= 0x00FF;
  data |= rcomp << 8;
  status = max17048_write_register(max17048, MAX17048_CONFIG, data, 2);

  return status;
}

/**************************************************************************/ /**
 *  Read alert status.
 ******************************************************************************/
sl_status_t max17048_get_alert_status( uint8_t *state)
{
  sl_status_t status;
  uint16_t value;

  status = max17048_read_register(max17048, MAX17048_CONFIG, &value, 2);

  *state = (value >> MAX17048_CONFIG_ALRT_BIT) & 0x0001;

  return status;
}

/**************************************************************************/ /**
 *  Clear alert status
 ******************************************************************************/
sl_status_t max1704x_clear_alert_status(void)
{
  sl_status_t status;
  uint16_t value;

  status = max17048_read_register(max17048, MAX17048_CONFIG, &value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  status = max17048_write_register(max17048, MAX17048_CONFIG,
                                   value & ~(1 << MAX17048_CONFIG_ALRT_BIT),
                                   2);

  return status;
}

/**************************************************************************/ /**
 *  Enables/disables the analog comparator in hibernate mode.
 ******************************************************************************/
sl_status_t max17048_enable_reset_comparator(bool enable)
{
    sl_status_t status;
    uint16_t value;

    status = max17048_read_register(max17048, MAX17048_VRESET_ID, &value, 2);
    if (status != SL_STATUS_OK) {
        return status;
    }

    if (enable) {
        status = max17048_write_register(max17048,
            MAX17048_VRESET_ID,
            (value | (0x0001 << MAX17048_VRESET_DIS_BIT)),
            2);
    } else {
        status = max17048_write_register(max17048,
            MAX17048_VRESET_ID,
            (value & ~(0x0001 << MAX17048_VRESET_DIS_BIT)),
            2);
    }

    return status;
}

/**************************************************************************/ /**
 *  Indicate which alert condition was met.
 ******************************************************************************/
sl_status_t max17048_get_alert_condition(uint8_t *alert_condition)
{
  sl_status_t status;
  uint16_t value;

  status = max17048_read_register(max17048, MAX17048_STATUS, &value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  *alert_condition = (value >> 9) & 0x1F;

  return status;
}

/**************************************************************************/ /**
 *  Clear which alert condition was met.
 ******************************************************************************/
sl_status_t max17048_clear_alert_condition(void)
{
  sl_status_t status;
  uint16_t value;

  status = max17048_read_register(max17048, MAX17048_STATUS, &value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  value &= 0x40FF;

  status = max17048_write_register(max17048, MAX17048_STATUS, value, 2);

  return status;
}


/**************************************************************************/ /**
 *  Enable sleep.
 ******************************************************************************/
sl_status_t max17048_enter_sleep()
{
  sl_status_t status;
  uint16_t value;

  status = max17048_read_register(max17048, MAX17048_MODE, &value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  status = max17048_write_register(max17048,
      MAX17048_MODE,
      (value | (0x0001 << MAX17048_MODE_EN_SLEEP_BIT)),
      2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  status = max17048_read_register(max17048, MAX17048_CONFIG, &value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  status = max17048_write_register(max17048,
      MAX17048_CONFIG,
      (value | (0x0001 << MAX17048_CONFIG_SLEEP_BIT)),
      2);

  return status;
}


/**************************************************************************/ /**
 *  Exit sleep.
 ******************************************************************************/
sl_status_t max17048_exit_sleep()
{
  sl_status_t status;
  uint16_t value;

  status = max17048_read_register(max17048, MAX17048_CONFIG, &value, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  status = max17048_write_register(max17048,
      MAX17048_CONFIG,
      (value & ~(0x0001 << MAX17048_CONFIG_SLEEP_BIT)),
      2);

  return status;
}
/**************************************************************************/ /**
 *  Reset the IC.
 ******************************************************************************/
sl_status_t max17048_force_reset()
{
  sl_status_t status;

  status = max17048_write_register(max17048, MAX17048_CMD, MAX17048_RESET, 2);

  return status;
}

/**************************************************************************/ /**
 *  Forces the MAX17048 to initiate a battery quick start.
 ******************************************************************************/
sl_status_t max17048_force_quick_start()
{
  sl_status_t status;
  uint16_t value;

  if (MAX17048_CONFIG_ENABLE_HW_QSTRT) {
      GPIO_PinOutSet(MAX17048_CONFIG_ENABLE_QSTRT_PORT,
                     MAX17048_CONFIG_ENABLE_QSTRT_PIN);

      status = sl_sleeptimer_start_timer_ms(&quick_start_timer,
                                              1,
                                              quick_start_callback,
                                              (void *)NULL,
                                              0,
                                              0);
      return status;
  } else {
      status = max17048_read_register(max17048, MAX17048_MODE, &value, 2);
      if (status != SL_STATUS_OK) {
          return status;
      }

      status = max17048_write_register(
          max17048,
          MAX17048_MODE,
          (value | (0x0001 << MAX17048_MODE_QUICK_START_BIT)),
          2);
  }

  return status;
}

/***************************************************************************//**
 *  Load a custom model into the MAX17048 and enable it.
 ******************************************************************************/
sl_status_t max17048_load_model(const uint8_t *model)
{
  sl_status_t status;
  uint16_t value;

  status = max17048_write_register(max17048, MAX17048_LOCK_TABLE, MAX17048_UNLOCK, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  for (int i = 0; i < 64; i += 2) {
      value = ((uint16_t)(*(model + i)) << 8) | (*(model + i + 1));
      max17048_write_register(max17048, MAX17048_TABLE + i, value, 2);
  }

  status = max17048_write_register(max17048, MAX17048_LOCK_TABLE, MAX17048_LOCK, 2);
  if (status != SL_STATUS_OK) {
      return status;
  }

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 *  Read the ID of th IC.
 ******************************************************************************/
sl_status_t max17048_get_id(uint8_t *id)
{
    sl_status_t status;
    uint16_t value;

    status = max17048_read_register(max17048, MAX17048_VRESET_ID, &value, 2);

    *id = (uint8_t)value;

    return status;
}

/**************************************************************************/ /**
 *  Read the product version of IC.
 ******************************************************************************/
sl_status_t max17048_get_production_version( uint16_t *ver)
{
    sl_status_t status;

    status = max17048_read_register(max17048, MAX17048_VERSION, ver, 2);

    return status;
}

/**************************************************************************/ /**
 *  Callback when sleep timer expires.
 ******************************************************************************/
static void temp_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;
  uint8_t temp;
  uint8_t rcomp;

  temp = max17048_temp_callback();

  if(temp > 20) {
      rcomp = RCOMP0 + (temp - 20) * TEMP_CO_UP;
  } else {
      rcomp = RCOMP0 + (temp - 20) * TEMP_CO_DOWN;
  }

  max17048_set_rcomp(rcomp);
}

/**************************************************************************/ /**
 *  Callback when sleep timer expires.
 ******************************************************************************/
static void quick_start_callback(sl_sleeptimer_timer_handle_t *handle,
                                 void *data)
{
  (void)handle;
  (void)data;

  GPIO_PinOutClear(MAX17048_CONFIG_ENABLE_QSTRT_PORT,
                   MAX17048_CONFIG_ENABLE_QSTRT_PIN);
}

/***************************************************************************//**
 * @file sen17731.c
 * @brief SEN17731 Soil moisture sensor
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
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
 ******************************************************************************/
// -----------------------------------------------------------------------------
//                               Includes
// -----------------------------------------------------------------------------

#include "sen17731.h"

// -----------------------------------------------------------------------------
//                       Local Variables
// -----------------------------------------------------------------------------

static sl_i2cspm_t *sen17731_i2cspm_instance = SEN17731_DEFAULT_I2CSPM_INSTANCE;
static uint16_t sen17731_i2c_addr = SEN17731_DEFAULT_ADDR;
static sen17731_calibration_t calib = {
  .dry_value = 0,
  .wet_value = 1023,
};

// -----------------------------------------------------------------------------
//                       Local Function
// -----------------------------------------------------------------------------

/**************************************************************************//**
 *  Writes block of data to sen17731 sensor.
 *****************************************************************************/
static sl_status_t sen17731_write_blocking(uint8_t *pdata, uint8_t len);

/**************************************************************************//**
 *  Reads block of data from sen17731 sensor.
 *****************************************************************************/
static sl_status_t sen17731_read_blocking(uint8_t *pdata, uint8_t len);

// -----------------------------------------------------------------------------
//                       Public Function
// -----------------------------------------------------------------------------

/**************************************************************************//**
 *  Sets I2CSPM instance for sen17731 soil moisture sensor.
 *****************************************************************************/
sl_status_t sen17731_set_i2cspm_instance(sl_i2cspm_t *i2cspm_instance)
{
  if (i2cspm_instance == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  sen17731_i2cspm_instance = i2cspm_instance;

  return SL_STATUS_OK;
}

/**************************************************************************//**
 *  Sets I2C address for sen17731 soil moisture sensor.
 *****************************************************************************/
sl_status_t sen17731_set_address(uint16_t address)
{
  uint8_t send_data[2];
  sl_status_t sc;

  if ((address < 0x07) | (address > 0x78)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  send_data[0] = CHANGE_ADDRESS;
  send_data[1] = (uint8_t)address;

  sc = sen17731_write_blocking(send_data, 2);
  if (sc == SL_STATUS_OK) {
    sen17731_i2c_addr = address;
  }
  return sc;
}

/**************************************************************************//**
 *  Scans I2C address of sen17731 soil moisture sensor.
 *****************************************************************************/
sl_status_t sen17731_scan_address(uint16_t *address, uint8_t *num_dev)
{
  sl_status_t sc;
  *num_dev = 0;

  if ((address == NULL) | (num_dev == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }
  for (uint16_t addr = 0x08; addr < 0x78; addr++) {
    sen17731_i2c_addr = addr;
    sc = sen17731_write_blocking(NULL, 0);
    if (sc == SL_STATUS_OK) {
      *(address + *num_dev) = addr;
      (*num_dev)++;
    }
  }
  return SL_STATUS_OK;
}

/**************************************************************************//**
 *  Selects device on the I2C bus.
 *****************************************************************************/
sl_status_t sen17731_select_device(uint16_t address)
{
  if ((address < 0x07) | (address > 0x78)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sen17731_i2c_addr = address;
  return SL_STATUS_OK;
}

/**************************************************************************//**
 *  Turns on the LED of sen17731 soil moisture sensor.
 *****************************************************************************/
sl_status_t sen17731_led_on()
{
  uint8_t cmd = LED_ON;

  return sen17731_write_blocking(&cmd, 1);
}

/**************************************************************************//**
 *  Turns off the LED of sen17731 soil moisture sensor.
 *****************************************************************************/
sl_status_t sen17731_led_off()
{
  uint8_t cmd = LED_OFF;

  return sen17731_write_blocking(&cmd, 1);
}

/**************************************************************************//**
 *  Sets dry value for calibration.
 *****************************************************************************/
sl_status_t sen17731_set_dry_value(uint16_t dry_value)
{
  // wet value have to be greater than the dry value.
  if ((dry_value > calib.wet_value) | (dry_value > 1023)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  calib.dry_value = dry_value;

  return SL_STATUS_OK;
}

/**************************************************************************//**
 *  Sets wet value for calibration.
 *****************************************************************************/
sl_status_t sen17731_set_wet_value(uint16_t wet_value)
{
  // wet value have to be greater than the dry value.
  if ((wet_value < calib.dry_value) | (wet_value > 1023)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  calib.wet_value = wet_value;

  return SL_STATUS_OK;
}

/**************************************************************************//**
 *  Gets the range of moisture raw value.
 *****************************************************************************/
sl_status_t sen17731_get_range_values(sen17731_calibration_t *range_values)
{
  if (range_values == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  range_values->dry_value = calib.dry_value;
  range_values->wet_value = calib.wet_value;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Gets the calibrated moisture value.
 ******************************************************************************/
sl_status_t sen17731_get_moisture(uint8_t *moisture)
{
  uint16_t value;
  sl_status_t sc;

  if (moisture == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  sc = sen17731_get_moisture_raw(&value);
  if (sc != SL_STATUS_OK) {
    return sc;
  }
  if (value < calib.dry_value) {
    *moisture = 0;
  } else {
    *moisture = (uint8_t)((value - calib.dry_value) * 100
                          / (calib.wet_value - calib.dry_value));
  }
  return SL_STATUS_OK;
}

/**************************************************************************//**
 *  Gets the moisture raw value from the sen17731 moisture sensor.
 *****************************************************************************/
sl_status_t sen17731_get_moisture_raw(uint16_t *value)
{
  sl_status_t sc;
  uint8_t recv_data[2];

  if (value == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  sc = sen17731_read_blocking(recv_data, 2);

  if (sc != SL_STATUS_OK) {
    return sc;
  }

  /* The ADC value increases from 0 -> 1023 according to the decreament of
   * the moisture value. */
  *value = 0x3ff - ((uint16_t)(recv_data[1] << 8) + (uint16_t)recv_data[0]);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *  Writes block of data to sen17731 sensor.
 *
 * @details
 *  This function writes block of data to sen17731 sensor.
 *
 * @param[in] pdata
 *  Block of data the wants to send to the sensor.
 * @param[in] len
 *  Length of data that sends to the sensor.
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_FAIL Failure
 ******************************************************************************/
static sl_status_t sen17731_write_blocking(uint8_t *pdata, uint8_t len)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  seq.addr = sen17731_i2c_addr << 1;
  seq.flags = I2C_FLAG_WRITE;

  seq.buf[0].data = pdata;
  seq.buf[0].len = len;

  ret = I2CSPM_Transfer(sen17731_i2cspm_instance, &seq);

  return ret;
}

/***************************************************************************//**
 * @brief
 *  Reads block of data from sen17731 sensor.
 *
 * @details
 *  This function writes block of data to sen17731 sensor.
 *
 * @param[in] pdata
 *  Block of data the wants to send to the sensor.
 * @param[in] len
 *  Length of data that sends to the sensor.
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_FAIL Failure
 ******************************************************************************/
static sl_status_t sen17731_read_blocking(uint8_t *pdata, uint8_t len)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t send_data = GET_VALUE;

  seq.addr = sen17731_i2c_addr << 1;
  seq.flags = I2C_FLAG_WRITE_READ;

  seq.buf[0].data = &send_data;
  seq.buf[0].len = 1;

  seq.buf[1].data = pdata;
  seq.buf[1].len = len;

  ret = I2CSPM_Transfer(sen17731_i2cspm_instance, &seq);

  return ret;
}

/***************************************************************************//**
 * @file ccs811.c
 * @brief CCS811 sensor IC driver source
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <ccs811.h>
#include "sl_sleeptimer.h"

// Create an I2C Instance for CCS811
static ccs811_i2c_t ccs811;

// register addresses
#define CCS_811_ID_ADDRESS              0x20
#define CCS_811_ID_ADDRESS_VALUE        0x81
#define CCS_811_SW_RESET_ADDR           0xFF
#define CCS_811_ADDR_ALG_RESULT_DATA    0x02
#define CCS_811_ADDR_APP_START          0xF4
#define CCS_811_ADDR_MEASURE_MODE       0x01
#define CCS_811_STATUS_REGISTER_ADDRESS 0x00
#define CCS_811_ERROR_REGISTER          0xE0

// measure mode setup
#define IDLE_NO_MEASURE 0  //Idle, low current mode
#define MEASURE_ONE_SEC 1  //Constant power mode, IAQ measurement every second
#define MEASURE_10_SEC  2  //Pulse heating mode IAQ measurement every 10 seconds
#define MEASURE_60_SEC  3  //Low power pulse heating mode IAQ measurement every 60 seconds

// important bits of status register
#define DATA_READY 3
#define APP_VALID  4

// -----------------------------------------------------------------------------
//                      Local function definitions
// -----------------------------------------------------------------------------

static ccs811_ret_st read_one_byte(uint8_t registeraddress,uint8_t *returnbyte);
static ccs811_ret_st reset(void);
static ccs811_ret_st set_app_start(void);
static ccs811_ret_st set_measure_mode(uint8_t measMode);

// -----------------------------------------------------------------------------
//                          Public functions
// -----------------------------------------------------------------------------

ccs811_ret_st ccs811_init(void)
{
  uint8_t checksum = 0;
  ccs811_ret_st ret_val = STATUS_FAIL;
  checksum += reset();
  sl_sleeptimer_delay_millisecond(10);
  uint8_t chip_id = 0;
  checksum += read_one_byte(CCS_811_ID_ADDRESS, &chip_id);

  if (chip_id != CCS_811_ID_ADDRESS_VALUE) {
    return ret_val;
  }
  checksum += set_app_start();
  sl_sleeptimer_delay_millisecond(10);
  checksum += set_measure_mode(MEASURE_ONE_SEC);
  sl_sleeptimer_delay_millisecond(10);

  if (checksum >= 4) {
    ret_val = STATUS_PASSED;
  }
  return ret_val;
}

ccs811_ret_st ccs811_measure(uint16_t *eco2, uint16_t *tvoc)
{
  ccs811_ret_st ret_val = STATUS_FAIL;
  uint8_t status_value = 0;
  read_one_byte(CCS_811_STATUS_REGISTER_ADDRESS, &status_value);

  // New data sample is ready and valid application firmware is loaded.
  if ((status_value & 1 << DATA_READY) && (status_value & 1 << APP_VALID)) {
    I2C_TransferSeq_TypeDef seq;
    I2C_TransferReturn_TypeDef ret;
    uint8_t i2c_read_data[4] = { 0, 0, 0, 0 };
    uint8_t i2c_write_data[1];

    uint16_t eco2_loc = 0;
    uint16_t tvoc_loc = 0;

    *eco2 = 0;
    *tvoc = 0;

    // Read four bytes from the ALG_RESULT_DATA mailbox register.
    i2c_write_data[0] = CCS_811_ADDR_ALG_RESULT_DATA;

    seq.addr = CCS_811_DEFAULT_I2C_ADDR << 1;
    seq.flags = I2C_FLAG_WRITE_READ;
    seq.buf[0].data = i2c_write_data;
    seq.buf[0].len = 1;
    seq.buf[1].data = i2c_read_data;
    seq.buf[1].len = 4;

    ret = I2CSPM_Transfer(I2C0, &seq);
    if (ret != i2cTransferDone) {
      return ret_val;
    } else {
      // Convert the read bytes to 16 bit values
      eco2_loc = ((uint16_t) i2c_read_data[0] << 8)
                 + (uint16_t) i2c_read_data[1];
      tvoc_loc = ((uint16_t) i2c_read_data[2] << 8)
                 + (uint16_t) i2c_read_data[3];
    }
    *eco2 = eco2_loc;
    *tvoc = tvoc_loc;
    ret_val = STATUS_PASSED;
  }
  return ret_val;
}

sl_status_t ccs811_i2c(ccs811_i2c_t *init)
{
  ccs811.i2c_sensor = init->i2c_sensor;
  ccs811.i2c_address = init->i2c_address;
  ccs811.oversample_rate = init->oversample_rate;
  sl_sleeptimer_delay_millisecond(10);
  return SL_STATUS_OK;
}

// -----------------------------------------------------------------------------
//                        Local functions
// -----------------------------------------------------------------------------

/***************************************************************************//**
* @brief
*  Setting the measure mode of the sensor.
*
* @param measMode
*   Setting the sensor sampling time.
*   Possible values:
*   0 Idle, low current mode
*   1 Constant power mode, IAQ measurement every second
*   2 Pulse heating mode IAQ measurement every 10 seconds
*   3 Low power pulse heating mode IAQ measurement every 60 seconds
*   mode 4 (force mode) is not supported, need direct ADC readout from sensor.
*
* @return
*   Return value is STATUS_FAIL or STATUS_PASSED.
******************************************************************************/
static ccs811_ret_st set_measure_mode(uint8_t measMode)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2c_read_data[1];
  uint8_t i2c_write_data[2];
  ccs811_ret_st ret_val = STATUS_FAIL;

  if (MEASURE_60_SEC < measMode) {
    measMode = MEASURE_ONE_SEC;
  }

  measMode = measMode << 4; //Bits 7,2,1 and 0 are reserved, clear them.

  // Write to the measurement mode register.
  i2c_write_data[0] = CCS_811_ADDR_MEASURE_MODE;
  i2c_write_data[1] = measMode;

  seq.addr = CCS_811_DEFAULT_I2C_ADDR << 1;
  seq.flags = I2C_FLAG_WRITE;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 2;
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len = 0;

  ret = I2CSPM_Transfer(I2C0, &seq);
  if (ret != i2cTransferDone) {
    return ret_val;
  }
  ret_val = STATUS_PASSED;
  return ret_val;
}

/***************************************************************************//**
* @brief
*  Reset the ccs811 sensor.
*
* @return
*   Return value is STATUS_FAIL or STATUS_PASSED.
*******************************************************************************/
static ccs811_ret_st reset(void)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2c_read_data[4];
  uint8_t i2c_write_data[5];
  ccs811_ret_st ret_val = STATUS_FAIL;
  // Write the 0x11 0xE5 0x72 0x8A key sequence to software reset register
  // The key sequence is used to prevent accidental reset
  i2c_write_data[0] = CCS_811_SW_RESET_ADDR;
  i2c_write_data[1] = 0x11;
  i2c_write_data[2] = 0xE5;
  i2c_write_data[3] = 0x72;
  i2c_write_data[4] = 0x8A;

  seq.addr = CCS_811_DEFAULT_I2C_ADDR << 1;
  seq.flags = I2C_FLAG_WRITE;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 5;
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len = 0;

  ret = I2CSPM_Transfer(I2C0, &seq);
  if (ret != i2cTransferDone) {
    return ret_val;
  }
  sl_sleeptimer_delay_millisecond(2); // Required to work properly after reset.
  ret_val = STATUS_PASSED;
  return ret_val;
}

/***************************************************************************//**
* @brief
*   Set CCS811 to application mode.
*
* @return
*   Return value is STATUS_FAIL or STATUS_PASSED.
******************************************************************************/
static ccs811_ret_st set_app_start(void)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2c_read_data[2];
  uint8_t i2c_write_data[1];
  ccs811_ret_st ret_val = STATUS_FAIL;

  // Perform a write with no data to the APP_START register to change the
  // state from boot mode to application mode.
  i2c_write_data[0] = CCS_811_ADDR_APP_START;

  seq.addr = CCS_811_DEFAULT_I2C_ADDR << 1;
  seq.flags = I2C_FLAG_WRITE;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 1;
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len = 0;

  ret = I2CSPM_Transfer(I2C0, &seq);
  if (ret != i2cTransferDone) {
    return ret_val;
  }
  sl_sleeptimer_delay_millisecond(10); // Needed for stable sensor work.

  ret_val = STATUS_PASSED;
  return ret_val;
}

/***************************************************************************//**
 * @brief
 *   I2C master read function for a single byte.
 *
 * @param [in] registeraddress
 *   The register address to read out.
 * @param [in] *returnbyte
 *   The value of this pointer will change to
 *   the read out value of the register.
 *
 * @return
 *    Return value is STATUS_FAIL or STATUS_PASSED.
 ******************************************************************************/
static ccs811_ret_st read_one_byte(uint8_t registeraddress,
                                   uint8_t *returnbyte)
{
  ccs811_ret_st ret_val = STATUS_FAIL;
  uint8_t data = 0;
  // Transfer structure
  I2C_TransferSeq_TypeDef i2c_transfer;

  // Initializing I2C transfer
  i2c_transfer.addr = CCS_811_DEFAULT_I2C_ADDR << 1;
  // Master write
  i2c_transfer.flags = I2C_FLAG_WRITE_READ;

  // Transmit buffer, no data to send
  i2c_transfer.buf[0].data = &registeraddress;
  i2c_transfer.buf[0].len = 1;

  // Receive buffer, two bytes to receive
  i2c_transfer.buf[1].data = &data;
  i2c_transfer.buf[1].len = 1;

  if (I2CSPM_Transfer(I2C0, &i2c_transfer) != i2cTransferDone) {
    return ret_val;
  }
  ret_val = STATUS_PASSED;
  *returnbyte = data;
  return ret_val;
}

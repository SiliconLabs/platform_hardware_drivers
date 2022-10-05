/***************************************************************************//**
 * @file mlx90640_i2c.c
 *******************************************************************************
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 *
 * EVALUATION QUALITY
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "mlx90640_i2c.h"
#include <stdio.h>

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static sl_i2cspm_t *i2cspm;
static uint8_t i2c_addr;

// -----------------------------------------------------------------------------
//                           Function definitions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * Assigns an I2CSPM instance and the default slave address for the driver to use
 ******************************************************************************/
sl_status_t mlx90640_I2C_Init(sl_i2cspm_t *i2cspm_instance, uint8_t new_i2c_addr)
{
  if(i2cspm_instance != NULL)
  {
    i2cspm = i2cspm_instance;
    i2c_addr = new_i2c_addr;
  }
  else
  {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Assigns an I2CSPM instance and a slave address for the driver to use
 ******************************************************************************/
sl_status_t mlx90640_I2C_change_bus_and_address(sl_i2cspm_t *i2cspm_instance, uint8_t new_i2c_addr)
{
  if((i2cspm_instance != NULL) && (i2c_addr != 0x00))
  {
    i2cspm = i2cspm_instance;
    i2c_addr = new_i2c_addr;
  }
  else
  {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Updates the I2C slave address of the device.
******************************************************************************/
sl_status_t mlx90640_I2C_set_own_address(uint8_t slave_addr)
{
  i2c_addr = slave_addr;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Retrieves the current I2C slave address used by the driver
******************************************************************************/
sl_status_t mlx90640_I2C_get_current_own_addr(uint8_t *current_address)
{
  *current_address = i2c_addr;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Issues an I2C general reset
 ******************************************************************************/
sl_status_t mlx90640_I2C_general_reset(void)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  uint8_t cmd[2] = {0x00,0x06};

  seq.addr = i2c_addr;
  seq.flags = I2C_FLAG_WRITE;
  seq.buf[0].len = 2;
  seq.buf[0].data = cmd;

  ret = I2CSPM_Transfer(i2cspm, &seq);

  if (ret != i2cTransferDone)
  {
      return -1;
  }
  return SL_STATUS_OK;
}


/***************************************************************************//**
 * Initiates an I2C read of the device
******************************************************************************/
sl_status_t mlx90640_I2C_read(uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
  uint8_t i2cData[1664] = {0};
  uint16_t counter = 0;
  uint16_t i = 0;
  uint16_t *p = data;
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  uint8_t cmd[2] = {0,0};
  cmd[0] = startAddress >> 8;
  cmd[1] = startAddress & 0x00FF;

  seq.addr = i2c_addr << 1;
  seq.flags = I2C_FLAG_WRITE_READ;
  seq.buf[0].len = 2;
  seq.buf[0].data = cmd;
  seq.buf[1].len = 2*nMemAddressRead;
  seq.buf[1].data = i2cData;
  ret = I2CSPM_Transfer(i2cspm, &seq);

  if (ret != i2cTransferDone)
  {
      return SL_STATUS_TIMEOUT;
  }

  for(counter = 0; counter < nMemAddressRead; counter++)
  {
      i = counter << 1;
      *p++ = (uint16_t)i2cData[i]*256 + (uint16_t)i2cData[i+1];
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Sets I2C base frequency to a given setting
******************************************************************************/
void mlx90640_I2C_freq_set(int freq)
{
  I2C_BusFreqSet(i2cspm, 0, freq, i2cClockHLRStandard);
}

/***************************************************************************//**
 * Initiates an I2C write to the device
******************************************************************************/
sl_status_t mlx90640_I2C_write(uint16_t writeAddress, uint16_t data)
{
  uint8_t cmd[4] = {0,0,0,0};
  static uint16_t dataCheck;

  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef i2c_ret;
  int ret;

  cmd[0] = writeAddress >> 8;
  cmd[1] = writeAddress & 0x00FF;
  cmd[2] = data >> 8;
  cmd[3] = data & 0x00FF;

  seq.addr = i2c_addr << 1;
  seq.flags = I2C_FLAG_WRITE;
  seq.buf[0].len = 4;
  seq.buf[0].data = cmd;

  i2c_ret = I2CSPM_Transfer(i2cspm, &seq);

  if (i2c_ret != i2cTransferDone)
  {
      return SL_STATUS_TIMEOUT;
  }

  ret = mlx90640_I2C_read(writeAddress, 1, &dataCheck);

  if (ret != 0)
  {
      return SL_STATUS_FAIL;
  }

  if (dataCheck != data)
  {
      return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}


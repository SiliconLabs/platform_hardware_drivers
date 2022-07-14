/***************************************************************************//**
 * @file as7265x_platform.c
 * @brief AS7265X Platform Source Code File.
 * @version 1.0.0
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
 *
 * EVALUATION QUALITY
 * This code has been minimally tested to ensure that it builds with the
 * specified dependency versions and is suitable as a demonstration for
 * evaluation purposes only.
 * This code will be maintained at the sole discretion of Silicon Labs.
 *
 ******************************************************************************/

#include <string.h>
#include "as7265x_platform.h"
#include "as7265x_config.h"

static sl_i2cspm_t *_as7265x_i2cspm_instance = AS7265X_CONFIG_I2C_INSTANCE;

/***************************************************************************//**
 * Set i2cspm instance
*******************************************************************************/
void as7265x_platform_set_i2cspm_instance(sl_i2cspm_t *i2cspm_instance)
{
  _as7265x_i2cspm_instance = i2cspm_instance;
}

/***************************************************************************//**
 * Read register value
*******************************************************************************/
sl_status_t as7265x_platform_read_register(uint8_t i2c_addr,
                                           uint8_t addr,
                                           uint8_t *pdata)
{
  I2C_TransferSeq_TypeDef seq;
  uint8_t i2c_write_data;

  if (pdata == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  } else {
    seq.addr = i2c_addr << 1;
    seq.flags = I2C_FLAG_WRITE_READ;

    i2c_write_data = addr;

    /*Write buffer*/
    seq.buf[0].data = &i2c_write_data;
    seq.buf[0].len = 1;

    /*Read buffer*/
    seq.buf[1].data = pdata;
    seq.buf[1].len = 1;

    if (I2CSPM_Transfer(_as7265x_i2cspm_instance, &seq) != i2cTransferDone) {
      return SL_STATUS_TRANSMIT;
    }
    return SL_STATUS_OK;
  }
}

/***************************************************************************//**
 * Write into register
*******************************************************************************/
sl_status_t as7265x_platform_write_register(uint8_t i2c_addr,
                                            uint8_t addr,
                                            uint8_t data)
{
  I2C_TransferSeq_TypeDef seq;
  uint8_t i2c_write_data[2];

  seq.addr = i2c_addr << 1;
  seq.flags = I2C_FLAG_WRITE;

  i2c_write_data[0] = addr;
  i2c_write_data[1] = data;

  /*Write buffer*/
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 2;

  if (I2CSPM_Transfer(_as7265x_i2cspm_instance, &seq) != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

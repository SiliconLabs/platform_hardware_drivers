/***************************************************************************//**
 * @file vcnl4040_platform.c
 * @brief platform for VCNL4040 Proximity sensor
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

#include "sl_status.h"
#include "vcnl4040_platform.h"
// -----------------------------------------------------------------------------
//                          Public functions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 *  Read two consecutive bytes from a given 'command code' location
 ******************************************************************************/
sl_status_t vcnl4040_i2c_read_command(sl_i2cspm_t *i2cspm,
                                      uint8_t address,
                                      uint8_t command,
                                      uint16_t *data)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t pdata[2];

  seq.addr = (uint16_t)(address << 1);
  seq.flags = I2C_FLAG_WRITE_READ;

  seq.buf[0].data = &command;
  seq.buf[0].len = 1;

  seq.buf[1].data = pdata;
  seq.buf[1].len = 2;

  ret = I2CSPM_Transfer(i2cspm, &seq);
  *data = (uint16_t)(pdata[1] << 8) + pdata[0];

  return ret;
}

/***************************************************************************//**
 *  Write two consecutive bytes to a given 'command code' location
 ******************************************************************************/
sl_status_t vcnl4040_i2c_write_command(sl_i2cspm_t *i2cspm,
                                       uint8_t address,
                                       uint8_t command,
                                       uint16_t data)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t send_data[3];

  send_data[0] = command;
  send_data[1] = (uint8_t)(data & 0xFF);
  send_data[2] = (uint8_t)(data >> 8);

  seq.addr = (uint16_t)(address << 1);
  seq.flags = I2C_FLAG_WRITE;

  seq.buf[0].data = send_data;
  seq.buf[0].len = 3;

  ret = I2CSPM_Transfer(i2cspm, &seq);

  return ret;
}

/***************************************************************************//**
 *  Read lower byte from a given 'command code' location
 ******************************************************************************/
sl_status_t vcnl4040_i2c_read_command_lower(sl_i2cspm_t *i2cspm,
                                            uint8_t address,
                                            uint8_t command,
                                            uint8_t *data)
{
  uint16_t pdata;
  sl_status_t ret;

  ret = vcnl4040_i2c_read_command(i2cspm, address, command, &pdata);
  *data = (uint8_t)(pdata & 0xFF);

  return ret;
}

/***************************************************************************//**
 *  Read upper byte from a given 'command code' location
 ******************************************************************************/
sl_status_t vcnl4040_i2c_read_command_upper(sl_i2cspm_t *i2cspm,
                                            uint8_t address,
                                            uint8_t command,
                                            uint8_t *data)
{
  uint16_t pdata;
  sl_status_t ret;

  ret = vcnl4040_i2c_read_command(i2cspm, address, command, &pdata);
  *data = (uint8_t)(pdata >> 8);

  return ret;
}

/***************************************************************************//**
 *  Write lower byte to a given 'command code' location
 ******************************************************************************/
sl_status_t vcnl4040_i2c_write_command_lower(sl_i2cspm_t *i2cspm,
                                             uint8_t address,
                                             uint8_t command,
                                             uint8_t data)
{
  uint16_t command_value;
  sl_status_t ret;

  ret = vcnl4040_i2c_read_command(i2cspm, address, command, &command_value);

  if (ret != SL_STATUS_OK) {
    return ret;
  }
  command_value &= 0xFF00;
  command_value |= (uint16_t)(data);

  return vcnl4040_i2c_write_command(i2cspm, address, command, command_value);
}

/***************************************************************************//**
 *  Write upper byte to a given 'command code' location
 ******************************************************************************/
sl_status_t vcnl4040_i2c_write_command_upper(sl_i2cspm_t *i2cspm,
                                             uint8_t address,
                                             uint8_t command,
                                             uint8_t data)
{
  uint16_t command_value;
  sl_status_t ret;

  ret = vcnl4040_i2c_read_command(i2cspm, address, command, &command_value);

  if (ret != SL_STATUS_OK) {
    return ret;
  }
  command_value &= 0x00FF;
  command_value |= (uint16_t)data << 8;

  return vcnl4040_i2c_write_command(i2cspm, address, command, command_value);
}

/***************************************************************************//**
 *  Masked write of upper/lower byte at a given 'command code' location
 ******************************************************************************/
sl_status_t vcnl4040_i2c_masked_write_command(sl_i2cspm_t *i2cspm,
                                              uint8_t address,
                                              uint8_t command,
                                              bool command_height,
                                              uint8_t mask,
                                              uint8_t data)
{
  uint8_t reg_value;
  sl_status_t ret;

  if (command_height == LOWER) {
    if (vcnl4040_i2c_read_command_lower(i2cspm, address, command,
                                        &reg_value) != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
    reg_value &= mask;
    reg_value |= data;
    ret = vcnl4040_i2c_write_command_lower(i2cspm, address, command, data);

    return ret;
  } else {
    if (vcnl4040_i2c_read_command_upper(i2cspm, address, command,
                                        &reg_value) != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
    reg_value &= mask;
    reg_value |= data;
    ret = vcnl4040_i2c_write_command_upper(i2cspm, address, command, data);

    return ret;
  }
}

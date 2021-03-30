/***************************************************************************//**
 * @file  mlx90632_i2c.c
 * @brief IrThermo 3 Click i2c communication.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include <mlx90632_i2c.h>
#include "sl_i2cspm.h"

// Implementation of I2C read for 16-bit values
int32_t mlx90632_i2c_read(int16_t register_address, uint16_t *value)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  uint8_t i2c_read_data[2];
  uint8_t i2c_write_data[2];

  seq.addr = SLAVE_ADDRESS;
  // Indicate combined write/read sequence:
  // S+ADDR(W)+DATA0+Sr+ADDR(R)+DATA1+P. */
  seq.flags = I2C_FLAG_WRITE_READ;
  // Select command to issue
  i2c_write_data[0] = register_address >> 8;
  i2c_write_data[1] = register_address;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 2;
  // Select location/length of data to be read
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len = 2;
  // Sending data
  ret = I2CSPM_Transfer (I2C1, &seq);

  *value = (i2c_read_data[0] << 8) | i2c_read_data[1];

  return ret;
}

// Implementation of I2C read for 32-bit values
int32_t mlx90632_i2c_read32(int16_t register_address, uint32_t *value)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  uint8_t i2c_read_data[4];
  uint8_t i2c_write_data[2];

  seq.addr = SLAVE_ADDRESS;
  // Indicate combined write/read sequence:
  // S+ADDR(W)+DATA0+Sr+ADDR(R)+DATA1+P.
  seq.flags = I2C_FLAG_WRITE_READ;
  // Select command to issue
  i2c_write_data[0] = register_address >> 8;
  i2c_write_data[1] = register_address;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 2;
  // Select location/length of data to be read
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len = 4;
  // Sending data
  ret = I2CSPM_Transfer (I2C1, &seq);

  *value = i2c_read_data[0] << 8 | i2c_read_data[1] | i2c_read_data[2] << 24
      | i2c_read_data[3] << 16;

  return ret;
}

// Implementation of I2C write for 16-bit values
int32_t mlx90632_i2c_write (int16_t register_address, uint16_t value)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  uint8_t i2c_value_data[2];
  uint8_t i2c_write_data[2];

  seq.addr = SLAVE_ADDRESS;
  // Indicate write sequence using two buffers:
  // S+ADDR(W)+DATA0+DATA1+P.
  seq.flags = I2C_FLAG_WRITE_WRITE;
  // Register address to be send
  i2c_write_data[0] = register_address >> 8;
  i2c_write_data[1] = register_address;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 2;
  // Data to be send
  i2c_value_data[0] = value >> 8;
  i2c_value_data[1] = value;
  seq.buf[1].data = i2c_value_data;
  seq.buf[1].len = 2;
  // Sending data
  ret = I2CSPM_Transfer (I2C1, &seq);

  value = (i2c_value_data[0] << 8) | i2c_value_data[1];

  return ret;
}


/***************************************************************************//**
 * @file
 * @brief I2C abstraction used by SSD1306
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <ssd1306_i2c.h>
#include "sl_i2cspm.h"
#include "sl_i2cspm_qwiic_config.h"

/***************************************************************************//**
 * @brief
 *   Initialize gpio used in the i2c interface.
 *
 * @detail
 *  The driver instances will be initialized automatically,
 *  during the sl_system_init() call in main.c.
 *****************************************************************************/
void ssd1306_i2c_init(void)
{
  return;
}

/***************************************************************************//**
 * @brief
 *    Start an blocking command transmit transfer.
 *
 * @note
 *    @n This function is blocking and returns when the transfer is complete.
 *
 * @param[in] command
 *    Transmit command buffer.
 *
 * @param[in] len
 *    Number of bytes in transfer.
 *
 * @return
 *    @ref SL_STATUS_OK on success or @ref SL_STATUS_FAIL on failure
 ******************************************************************************/
sl_status_t ssd1306_send_command(const void *cmd, uint8_t len)
{
  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2c_write_data[len];
  uint8_t i2c_read_data[1];
  const uint8_t *ptr = cmd;

  seq.addr  = SSD1306_SLAVE_ADDRESS << 1;
  seq.flags = I2C_FLAG_WRITE;
  /* Select register and data to write */
  i2c_write_data[0] = 0x00; // 0x00 for cmd, 0x40 for data
  for(int i = 0; i < len; i++) {
    i2c_write_data[i + 1] = ptr[i];
  }

  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len  = len + 1;
  /* Select location/length of data to be read */
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len  = 0;
  ret = I2CSPM_Transfer(SL_I2CSPM_QWIIC_PERIPHERAL, &seq);
  if (ret != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *    Start an blocking data transmit transfer.
 *
 * @note
 *    @n This function is blocking and returns when the transfer is complete.
 *
 * @param[in] data
 *    Transmit data buffer.
 *
 * @param[in] count
 *    Number of bytes in transfer.
 *
 * @return
 *    @ref SL_STATUS_OK on success or @ref SL_STATUS_FAIL on failure
 ******************************************************************************/
sl_status_t ssd1306_send_data(const void *data, uint8_t len)
{
  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2c_write_data[len];
  uint8_t i2c_read_data[1];
  const uint8_t *ptr = data;

  seq.addr  = SSD1306_SLAVE_ADDRESS << 1;
  seq.flags = I2C_FLAG_WRITE;
  /* Select register and data to write */
  i2c_write_data[0] = 0x40; // 0x00 for cmd, 0x40 for data
  for (int i = 0; i < len; i++)
  {
    i2c_write_data[i+1] = ptr[i];
  }
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len  = len + 1;
  /* Select location/length of data to be read */
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len  = 0;
  ret = I2CSPM_Transfer(SL_I2CSPM_QWIIC_PERIPHERAL, &seq);
  if (ret != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }

  return SL_STATUS_OK;
}


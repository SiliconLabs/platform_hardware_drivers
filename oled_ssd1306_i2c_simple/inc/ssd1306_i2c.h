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
#ifndef SSD1306_SPI_H
#define SSD1306_SPI_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SSD1306_SLAVE_ADDRESS 0x3D

void ssd1306_i2c_init(void);
sl_status_t ssd1306_send_command(const void *cmd, uint8_t len);
sl_status_t ssd1306_send_data(const void *data, uint8_t len);


/** @} */
#ifdef __cplusplus
}
#endif

#endif

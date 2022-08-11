/***************************************************************************//**
 * @file mma8452q_i2c.h
 * @brief MMA8452Q Platform file
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

#ifndef _MMA8452Q_PLATFORM_H_
#define _MMA8452Q_PLATFORM_H_

#include <stdint.h>
#include "sl_status.h"
#include "sl_i2cspm.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup mma8452q_i2c MMA8452Q I2C API Definitions */

/***************************************************************************//**
 * @addtogroup mma8452q_i2c
 * @brief  MMA8452Q Driver Platform
 * @details
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *    Read a byte from the MMA8452Q register.
 *
 * @param[in] addr
 *    Register address to read from sensor
 *
 * @param[out] data
 *    Data read from register
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_read_register(uint8_t addr, uint8_t *data);

/***************************************************************************//**
 * @brief
 *    Read a byte from the MMA8452Q register.
 *
 * @param[in] addr
 *    Register address to read from sensor
 *
 * @param[in] num_bytes
 *    The number of bytes to read
 *
 * @param[out] data
 *    Data read from register
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_read_block(uint8_t addr, uint8_t num_bytes, uint8_t *data);

/***************************************************************************//**
 * @brief
 *    Write to a MMA8452Q register.
 *
 * @param[in] addr
 *    Register address to write to sensor
 *
 * @param[in] data
 *    Data to write to register
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_write_register(uint8_t addr, uint8_t data);

/***************************************************************************//**
 * @brief
 *    Write a block of data to a MMA8452Q registers.
 *
 * @param[in] addr
 *    Register address to  write to sensor
 *
 * @param[in] num_bytes
 *    The number of bytes to write
 *
 * @param[in] data
 *    Data to write to register
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_write_block(uint8_t addr,
                                 uint8_t num_bytes,
                                 uint8_t *data);

/** @} (end addtogroup mma8452q_platform) */

#ifdef __cplusplus
}
#endif

#endif

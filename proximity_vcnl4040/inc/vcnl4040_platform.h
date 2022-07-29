/***************************************************************************//**
 * @file vcnl4040_platform.h
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
#ifndef VCNL4040_PLATFORM_H_
#define VCNL4040_PLATFORM_H_

// -----------------------------------------------------------------------------
//                               Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include "sl_status.h"
#include "sl_i2cspm_instances.h"

#define LOWER true
#define UPPER false

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup VCNL4040_Platform VCNL4040_Platform */

/***************************************************************************//**
 * @addtogroup VCNL4040_Platform
 * @brief  VCNL4040_Platform Driver API
 * @details
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                       Public Function Definitions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief
 *  Read two consecutive bytes from a given 'command code' location
 *
 * @param[in] command
 *  Command code location
 *
 * @param[out] data
 *  Data read from register
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_i2c_read_command(sl_i2cspm_t *i2cspm,
                                      uint8_t address,
                                      uint8_t command,
                                      uint16_t *data);

/***************************************************************************//**
 * @brief
 *  Write two consecutive bytes to a given 'command code' location
 *
 * @param[in] command
 *  Command code location
 *
 * @param[in] data
 *  Data to write to register
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_i2c_write_command(sl_i2cspm_t *i2cspm,
                                       uint8_t address,
                                       uint8_t command,
                                       uint16_t data);

/***************************************************************************//**
 * @brief
 *    Read lower byte from a given 'command code' location
 *
 * @param[in] command
 *    Command code location
 *
 * @param[out] data
 *    Data read from register
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_i2c_read_command_lower(sl_i2cspm_t *i2cspm,
                                            uint8_t address,
                                            uint8_t command,
                                            uint8_t *data);

/***************************************************************************//**
 * @brief
 *    Read upper byte from a given 'command code' location
 *
 * @param[in] command
 *    Command code location
 *
 * @param[out] data
 *    Data read from register
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_i2c_read_command_upper(sl_i2cspm_t *i2cspm,
                                            uint8_t address,
                                            uint8_t command,
                                            uint8_t *data);

/***************************************************************************//**
 * @brief
 *    Write lower byte to a given 'command code' location
 *
 * @param[in] command
 *    Command code location
 *
 * @param[in] data
 *    Data to write to register
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_i2c_write_command_lower(sl_i2cspm_t *i2cspm,
                                             uint8_t address,
                                             uint8_t command,
                                             uint8_t data);

/***************************************************************************//**
 * @brief
 *    Write upper byte to a given 'command code' location
 *
 * @param[in] command
 *    Command code location
 *
 * @param[in] data
 *    Data to write to register
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_i2c_write_command_upper(sl_i2cspm_t *i2cspm,
                                             uint8_t address,
                                             uint8_t command,
                                             uint8_t data);

/***************************************************************************//**
 * @brief
 *    Masked write of upper/lower byte at a given 'command code' location
 *
 * @param[in] command
 *    Command code location
 *
 * @param[in] commandHeight
 *    Command byte height - upper or lower byte
 *
 * @param[in] mask
 *    Masked bits to write
 *
 * @param[in] data
 *    Data to write to register
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_i2c_masked_write_command(sl_i2cspm_t *i2cspm,
                                              uint8_t address,
                                              uint8_t command,
                                              bool command_height,
                                              uint8_t mask,
                                              uint8_t data);

/** @} (end group VCNL4040_Platform) */

#ifdef __cplusplus
}
#endif

#endif /* VCNL4040_PLATFORM_H_ */

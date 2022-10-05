/***************************************************************************//**
 * @file mlx90640_i2c.h
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

#ifndef MLX90640_MLX90640_I2C_H_
#define MLX90640_MLX90640_I2C_H_

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include "sl_status.h"
#include "sl_i2cspm.h"

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief
 * Assigns an I2CSPM instance and the default slave address for the driver to use
 *
 * @param[in] i2cspm_instace - Pointer to the I2CSPM instance
 ******************************************************************************/
sl_status_t mlx90640_I2C_Init(sl_i2cspm_t *i2cspm_instance, uint8_t i2c_addr);

/***************************************************************************//**
 * Assigns a new I2CSPM instance and a slave address for the driver to use
 ******************************************************************************/
sl_status_t mlx90640_I2C_change_bus_and_address(sl_i2cspm_t *i2cspm_instance, uint8_t i2c_addr);

/***************************************************************************//**
 * @brief
 * Updates the I2C slave address of the device.
 * In case the address is already changed then it only stores the address to use
 *
 * @param[in] slave_addr - new I2C slave address to use
 ******************************************************************************/
sl_status_t mlx90640_I2C_set_own_address(uint8_t slave_addr);

/***************************************************************************//**
 * @brief
 * Retrieves the current I2C slave address used by the driver
 *
 * @param[out] current_address - Current I2C slave address used
 ******************************************************************************/
sl_status_t mlx90640_I2C_get_current_own_addr(uint8_t *current_address);

/***************************************************************************//**
 * @brief
 * Issues an I2C general reset
 ******************************************************************************/
sl_status_t mlx90640_I2C_general_reset(void);

/***************************************************************************//**
 * @brief
 * Initiates an I2C read of the device
 *
 * @param[in] startAddress - EEPROM memory address of the device to read out from
 * @param[in] nMemAddressRead - Length of the requested data
 * @param[out] data - pointer to an array where the received data will be stored
 ******************************************************************************/
sl_status_t mlx90640_I2C_read(uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data);

/***************************************************************************//**
 * @brief
 * Initiates an I2C write to the device
 *
 * @param[in] writeAddress - EEPROM memory address of the device to write to
 * @param[in] data - 16bit data to send to the device
 ******************************************************************************/
sl_status_t mlx90640_I2C_write(uint16_t writeAddress, uint16_t data);

/***************************************************************************//**
 * @brief
 * Sets I2C base frequency to a given setting
 *
 * @param[in] freq - new frequency in Hz to set for the I2C base frequency
 ******************************************************************************/
void mlx90640_I2C_freq_set(int freq);

#endif /* MLX90640_MLX90640_I2C_H_ */

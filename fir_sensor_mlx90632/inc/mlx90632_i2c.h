/***************************************************************************//**
 * @file  mlx90632_i2c.h
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
#ifndef MLX90632_I2C_H
#define MLX90632_I2C_H

#include <stdint.h>
#include <stdbool.h>

// Definition of I2C address of MLX90632
#define SLAVE_ADDRESS     0x3A << 1

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief Read the register_address 16 bit value from the mlx90632.
 *
 * @param[in]  register_address Address of the register to be read from.
 * @param[out] *value pointer to where read data can be written.
 *
 * @retval  0 for Success.
 * @retval <0 for Failure.
 *
 ******************************************************************************/
int32_t mlx90632_i2c_read(int16_t register_address, uint16_t *value);

/***************************************************************************//**
 * @brief Read the register_address 32 bit value from the mlx90632.
 *
 * @param[in]  register_address Address of the register to be read from.
 * @param[out] *value pointer to where read data can be written.
 *
 * @retval  0 for Success.
 * @retval <0 for Failure.
 *
 ******************************************************************************/
int32_t mlx90632_i2c_read32(int16_t register_address, uint32_t *value);

/***************************************************************************//**
 * @brief Write value to register_address of the mlx90632.
 *
 * @param[in] register_address Address of the register to be read from.
 * @param[in] value value to be written to register address of mlx90632.
 *
 * @retval  0 for Success.
 * @retval <0 for Failure.
 ******************************************************************************/
int32_t mlx90632_i2c_write(int16_t register_address, uint16_t value);

#endif // MLX90632_I2C_H

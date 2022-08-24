/***************************************************************************//**
 * @file sen17731.h
 * @brief SEN17731 Soil moisture sensor
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
#ifndef SEN17731_H_
#define SEN17731_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "sl_i2cspm.h"
#include "sl_status.h"

#include "sen17731_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup SEN17731 SEN17731 */

/***************************************************************************//**
 * @addtogroup SEN17731
 * @brief  SEN17731 Soil Moisture Hardware Driver
 * @details
 * @{
 *
 *
 *    @n @section sen17731_example SEN17731 example
 *
 *      Basic example for performing measurement: @n @n
 *      @code{.c}
 *
 * #include "sl_i2cspm_instances.h"
 * #include "sen17731.h"
 *    int main(void)
 *    {
 *      ...
 *      uint8_t moisture;
 *
 *      // Set I2CSPM instance for the sensor
 *      sen17731_set_i2cspm_instance(sl_i2cspm_qwiic);
 *
 *      // Calibrating the sensor
 *      sen17731_set_dry_value(90);
 *      sen17731_set_wet_value(1023);
 *
 *      // Read the soil moisture value
 *      sen17731_get_moisture(&moisture);
 *
 *
 *      ...
 *
 *    } @endcode
 *
 * @{
 ******************************************************************************/
// -----------------------------------------------------------------------------
//                               Macros
// -----------------------------------------------------------------------------

// sensor default i2c address
#define SEN17731_DEFAULT_ADDR               0x28

// sensor command codes
#define LED_OFF                             0x00
#define LED_ON                              0x01
#define CHANGE_ADDRESS                      0x03
#define GET_VALUE                           0x05

/***************************************************************************//**
 * @brief
 *  Struct to hold the wet and dry threshhold for calibration.
 ******************************************************************************/
typedef struct {
  uint16_t dry_value;       /*!< value in driest environment */
  uint16_t wet_value;       /*!< value in wetest environment */
} sen17731_calibration_t;

// -----------------------------------------------------------------------------
//                       Public Function Definitions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief
 *  Sets I2CSPM instance for sen17731 soil moisture sensor.
 *
 * @param[in] i2cspm
 *  The I2C peripheral to use.
 *
 * @details
 *  This function stores the I2C peripheral setting in static config, checks
 *  the connection by requesting/verifying chip ID, and loads default
 *  initialization settings.
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_INITIALIZATION Initialization Failure
 ******************************************************************************/
sl_status_t sen17731_set_i2cspm_instance(sl_i2cspm_t *i2cspm_instance);

/***************************************************************************//**
 * @brief
 *  Sets I2C address for sen17731 soil moisture sensor.
 *
 * @param[in] address
 *  The I2C address to use.
 *
 * @details
 *  This function sets the I2C address for sen17731 soil moisture sensor and
 *  stores the I2C address in static config.
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_FAIL Failure
 ******************************************************************************/
sl_status_t sen17731_set_address(uint16_t address);

/***************************************************************************//**
 * @brief
 *  Scans I2C address of all sensors that connected on the I2C bus.
 *
 * @param[out] address
 *  The pointer points to the memory area that holds the found addresses.
 * @param[out] num_dev
 *  The number of found device.
 *
 * @details
 *  This function scans the I2C address for all sensors on the I2C bus and
 *  stores the I2C address in static config.
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_FAIL Failure
 ******************************************************************************/
sl_status_t sen17731_scan_address(uint16_t *address, uint8_t *num_dev);

/***************************************************************************//**
 * @brief
 *  Selects device on the I2C bus.
 *
 * @param[in] address
 *  The I2C address of the device that wants to select.
 *
 * @details
 *  This function selects device on the I2C bus based on its I2C address.
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_FAIL Failure
 ******************************************************************************/
sl_status_t sen17731_select_device(uint16_t address);

/***************************************************************************//**
 * @brief
 *  Turns on the LED of sen17731 soil moisture sensor.
 *
 * @details
 *  This function turns on the LED on the sen17731 soil moisture sensor
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_FAIL Failure
 ******************************************************************************/
sl_status_t sen17731_led_on();

/***************************************************************************//**
 * @brief
 *  Turns off the LED of sen17731 soil moisture sensor.
 *
 * @details
 *  This function turns off the LED on the sen17731 soil moisture sensor
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_FAIL Failure
 ******************************************************************************/
sl_status_t sen17731_led_off();

/***************************************************************************//**
 * @brief
 *  Sets dry value for calibration.
 *
 * @param[in] dry_value
 *  The output of the sensor in dry environment.
 *
 * @details
 *  This function sets the dry value to calibrate for sen17731 soil moisture
 *  sensor.
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_FAIL Failure
 ******************************************************************************/
sl_status_t sen17731_set_dry_value(uint16_t dry_value);

/***************************************************************************//**
 * @brief
 *  Sets wet value for calibration.
 *
 * @param[in] wet_value
 *  The output of the sensor in wet environment.
 *
 * @details
 *  This function sets the wet value to calibrate for sen17731 soil moisture
 *  sensor.
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_FAIL Failure
 ******************************************************************************/
sl_status_t sen17731_set_wet_value(uint16_t wet_value);

/***************************************************************************//**
 * @brief
 *  Gets the range of moisture value.
 *
 * @param[out] range_values
 *  The range of the moisture value that read out of the sensor.
 *
 * @details
 *  This function gets the range of moisture values that read out from the
 *  sensor.
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_FAIL Failure
 ******************************************************************************/
sl_status_t sen17731_get_range_values(sen17731_calibration_t *range_values);

/***************************************************************************//**
 * @brief
 *  Gets the calibrated moisture value from the sen17731 moisture sensor.
 *
 * @details
 *  This function gets the moisture value which calibrated from the sen17731
 *  moisture sensor.
 *
 * @param[out] moisture
 *  The soil moisture value percentage which already calibrated.
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_FAIL Failure
 ******************************************************************************/
sl_status_t sen17731_get_moisture(uint8_t *moisture);

/***************************************************************************//**
 * @brief
 *  Gets the moisture raw value from the sen17731 moisture sensor.
 *
 * @details
 *  This function gets the raw value from the sen17731 moisture sensor. This
 *  value need to be calibrated to be appropriate with specified soil.
 *
 * @param[out] value
 *  The raw value from the sen17731 sensor.
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_FAIL Failure
 ******************************************************************************/
sl_status_t sen17731_get_moisture_raw(uint16_t *value);

#ifdef __cplusplus
}
#endif

#endif /* SEN17731_H_ */

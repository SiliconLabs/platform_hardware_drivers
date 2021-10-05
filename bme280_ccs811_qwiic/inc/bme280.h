/***************************************************************************//**
 * @file bme280.h
 * @brief header file for BME280 sensor driver
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef BME280_H_
#define BME280_H_

// -----------------------------------------------------------------------------
//                                 Includes
// -----------------------------------------------------------------------------

#include "em_device.h"
#include "em_gpio.h"
#include "sl_status.h"
#include "sl_i2cspm.h"
#include <bme280_config.h>

// -----------------------------------------------------------------------------
//                           Macros and Typedefs
// -----------------------------------------------------------------------------

// Return type for BME280 functions.
typedef enum {
  STATUS_FAILED,
  STATUS_OK
} bme280_ret_st;

// Structure to configure the BME280 sensor.
typedef struct {
  I2CSPM_Init_TypeDef  i2c_sensor;
  uint8_t              i2c_address;     // I2C address of the sensor
  uint8_t              oversample_rate; // Over-sample index
} bme280_i2c_t;

// Default initialization structure for BME280 I2C driver.
#define BME280_I2C_DEFAULT              \
  {                                     \
   {                                    \
    BME_280_DEFAULT_I2C_INSTANCE,       \
    BME_280_DEFAULT_SCL_PORT,           \
    BME_280_DEFAULT_SCL_PIN,            \
    BME_280_DEFAULT_SDA_PORT,           \
    BME_280_DEFAULT_SDA_PIN,            \
    0,                                  \
    I2C_FREQ_STANDARD_MAX,              \
    i2cClockHLRStandard,                \
   },                                   \
    BME_280_DEFAULT_I2C_ADDR,           \
    BME_280_DEFAULT_OVERSAMP_INDEX      \
  }

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                       Public Function Definitions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief
 *   Initialize the BME280 sensor.
 *
 * @return
 *   Return value is STATUS_FAILED or STATUS_OK.
 ******************************************************************************/
bme280_ret_st bme280_init(void);

/***************************************************************************//**
 * @brief
 *   After initialization read the temperature from BME280 sensor.
 *
 * @param [in] *temperature
 *   Value of this pointer will change to the measured value
 *   e.g. 2456 is equal to 24,56°C.
 *
 * @return
 *   Return value is STATUS_FAILED or STATUS_OK.
 ******************************************************************************/
bme280_ret_st bme280_read_temperature(int32_t *temperature);

/***************************************************************************//**
 * @brief
 *   After initialization read the pressure from BME280 sensor.
 *
 * @param [in] *pressure
 *   Value of this pointer will change to measured millibar value.
 *
 * @return
 *   Return value is STATUS_FAILED or STATUS_OK.
 ******************************************************************************/
bme280_ret_st bme280_read_pressure(uint32_t *pressure);

/***************************************************************************//**
 * @brief
 *   After initialization read the humidity from BME280 sensor.
 *
 * @param [in] *humidity
 *   Dividing with 1000 give the measured % humidity value
 *   e.g. 46067 is equal to 46,067%.
 *
 * @return
 *   Return value is STATUS_FAILED or STATUS_OK.
 ******************************************************************************/
bme280_ret_st bme280_read_humidity(uint32_t *humidity);

/***************************************************************************//**
 * @brief
 *   Send the BME280 force measure mode.
 *
 * @return
 *   Return value is STATUS_FAILED or STATUS_OK.
 ******************************************************************************/
bme280_ret_st bme280_ctrl_measure_set_to_work(void);

/***************************************************************************//**
 * @brief
 *   Send the BME280 force sleep mode.
 *
 * @return
 *   Return value is STATUS_FAILED or STATUS_OK.
 ******************************************************************************/
bme280_ret_st bme280_ctrl_measure_set_to_sleep(void);

/***************************************************************************//**
 * @brief
 *   Initializing the I2C for BME280.
 *
 * @param [in] *init
 *   Pass this pointer of the initializing structure.
 *
 * @return
 *   Return value is STATUS_FAILED or STATUS_OK.
 ******************************************************************************/
sl_status_t bme280_i2c(bme280_i2c_t *init);

#ifdef __cplusplus
}
#endif

#endif /* BME280_H_ */

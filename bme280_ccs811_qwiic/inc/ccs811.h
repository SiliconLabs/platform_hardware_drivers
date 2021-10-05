/***************************************************************************//**
 * @file ccs811.h
 * @brief header file for CCS811 sensor driver
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

#ifndef CCS811_H_
#define CCS811_H_

// -----------------------------------------------------------------------------
//                                Includes
// -----------------------------------------------------------------------------

#include "em_device.h"
#include "em_gpio.h"
#include "sl_status.h"
#include "sl_i2cspm.h"
#include <ccs811_config.h>

// -----------------------------------------------------------------------------
//                            Macros and Typedefs
// -----------------------------------------------------------------------------

// Structure to configure the CCS811 sensor
typedef struct {
  I2CSPM_Init_TypeDef  i2c_sensor;
  uint8_t              i2c_address;     // I2C address of the sensor
  uint8_t              oversample_rate; // Over-sample index
} ccs811_i2c_t;

// Return type for CCS811 functions
typedef enum {
  STATUS_FAIL,
  STATUS_PASSED
} ccs811_ret_st;

// Default initialization structure for CCS811 I2C driver.
#define CCS811_I2C_DEFAULT              \
  {                                     \
   {                                    \
    CCS_811_DEFAULT_I2C_INSTANCE,       \
    CCS_811_DEFAULT_SCL_PORT,           \
    CCS_811_DEFAULT_SCL_PIN,            \
    CCS_811_DEFAULT_SDA_PORT,           \
    CCS_811_DEFAULT_SDA_PIN,            \
    0,                                  \
    I2C_FREQ_STANDARD_MAX,              \
    i2cClockHLRStandard,                \
   },                                   \
    CCS_811_DEFAULT_I2C_ADDR,           \
    CCS_811_DEFAULT_OVERSAMP_INDEX      \
  }

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                      Public Function Definitions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief
 *   Initializing the I2C communication for CCS811.
 *
 * @return
 *   Return value is STATUS_FAIL or STATUS_PASSED.
 ******************************************************************************/
ccs811_ret_st ccs811_init(void);

/***************************************************************************//**
 * @brief
 *  Measure the CO2 and tVOC concentration of CCS811.
 *
 * @param *eco2
 *  The value of this pointer will change to the measured value of the CO2 ppm.
 * @param *tvoc
 *  The value of this pointer will change to the measured value of the tVOC ppb.
 *
 * @return
 *   Return value is STATUS_FAIL or STATUS_PASSED.
 ******************************************************************************/
ccs811_ret_st ccs811_measure(uint16_t *eco2, uint16_t *tvoc);

/***************************************************************************//**
 * @brief
 *   Initializing the I2C for CCS811.
 *
 * @param [in] *init
 *   Pass this pointer of the initializing structure.
 *
 * @return
 *   Return value is STATUS_FAIL or STATUS_PASSED.
 ******************************************************************************/
sl_status_t ccs811_i2c(ccs811_i2c_t* init);

#ifdef __cplusplus
}
#endif

#endif /* CCS811_H_ */

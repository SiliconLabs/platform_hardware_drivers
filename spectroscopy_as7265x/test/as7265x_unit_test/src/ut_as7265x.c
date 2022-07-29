/***************************************************************************//**
* @file ut_as7265x.c
* @brief unit test for the cap1166 Capacitive Touch Sensor driver
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "time_retarget.h"

#include "Basic.h"
#include "CUnit.h"
#include "ut_as7265x.h"

#include "as7265x.h"
#include "as7265x_config.h"

// static cap1166_handle_t my_cap1166_handle;
// static cap1166_cfg_t my_cap1166_config = CAP11666_DEFAULT_CONFIG;

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

static void test_as7265x_soft_reset(void);
static void test_as7265x_get_temperature(void);
static void test_as7265x_set_indicator_current(void);
static void test_as7265x_disable_indicator(void);
static void test_as7265x_enable_indicator(void);
static void test_as7265x_set_bulb_current(void);
static void test_as7265x_disable_bulb(void);
static void test_as7265x_enable_bulb(void);
static void test_as7265x_data_available(void);
static void test_as7265x_disable_interrupt(void);
static void test_as7265x_enable_interrupt(void);
static void test_as7265x_set_integration_cycles(void);
static void test_as7265x_set_gain(void);
static void test_as7265x_set_measurement_mode(void);
static void test_as7265x_get_calibrated_value(void);
static void test_as7265x_get_all_calibrated_value(void);
static void test_as7265x_get_channel(void);
static void test_as7265x_all_color_channel(void);
static void test_as7265x_take_measurements_with_bulb(void);
static void test_as7265x_take_measurements(void);
static void test_as7265x_get_firmware_version(void);
static void test_as7265x_get_hardware_version(void);
static void test_as7265x_get_device_type(void);
static void test_as7265x_init(void);
/*******************************************************************************
 *****************************   LOCAL VARS   **********************************
 ******************************************************************************/
static CU_TestInfo as7265x_unit_test[] = {
    { "test_as7265x_init",
        test_as7265x_init },
    { "test_as7265x_get_channel",
        test_as7265x_get_channel },
    { "test_as7265x_take_measurements_with_bulb",
        test_as7265x_take_measurements_with_bulb },
        { "test_as7265x_get_all_calibrated_value",
                test_as7265x_get_all_calibrated_value },
    { "test_as7265x_take_measurements",
        test_as7265x_take_measurements },
    { "test_as7265x_get_temperature",
        test_as7265x_get_temperature },
    { "test_as7265x_set_indicator_current",
        test_as7265x_set_indicator_current },
    { "test_as7265x_disable_indicator",
        test_as7265x_disable_indicator },
    { "test_as7265x_enable_indicator",
        test_as7265x_enable_indicator },
    { "test_as7265x_set_bulb_current",
        test_as7265x_set_bulb_current },
    { "test_as7265x_disable_bulb",
        test_as7265x_disable_bulb },
    { "test_as7265x_enable_bulb",
        test_as7265x_enable_bulb },
    { "test_as7265x_data_available",
        test_as7265x_data_available },
    { "test_as7265x_disable_interrupt",
        test_as7265x_disable_interrupt },
    { "test_as7265x_enable_interrupt",
        test_as7265x_enable_interrupt },
    { "test_as7265x_set_integration_cycles",
        test_as7265x_set_integration_cycles },
    { "test_as7265x_set_gain",
        test_as7265x_set_gain },
    { "test_as7265x_set_measurement_mode",
        test_as7265x_set_measurement_mode },
    { "test_as7265x_get_calibrated_value",
        test_as7265x_get_calibrated_value },
    { "test_as7265x_all_color_channel",
        test_as7265x_all_color_channel },
    { "test_as7265x_get_firmware_version",
        test_as7265x_get_firmware_version },
    { "test_as7265x_get_hardware_version",
        test_as7265x_get_hardware_version },
    { "test_as7265x_get_device_type",
        test_as7265x_get_device_type },
    { "test_as7265x_soft_reset",
        test_as7265x_soft_reset },
    CU_TEST_INFO_NULL,
};

static CU_SuiteInfo suites[] = {
    {"as7265_unit_test", NULL, NULL, NULL, NULL, as7265x_unit_test},
    CU_SUITE_INFO_NULL,
};

/*******************************************************************************
 *****************************      CODE      **********************************
 ******************************************************************************/

/**
 * @brief Test function for as7265x_soft_reset
 * 
 */
static void test_as7265x_soft_reset(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  // act
  status = as7265x_soft_reset();
  // assert
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_get_temperature
 * 
 */
static void test_as7265x_get_temperature(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  uint8_t temp_val;
  // act and assert
    // NULL pointer
  status = as7265x_get_temperature(AS72651_NIR, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_temperature(AS72652_VISIBLE, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_temperature(AS72653_UV, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

    // Normal case
  status = as7265x_get_temperature(AS72651_NIR, &temp_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_temperature(AS72652_VISIBLE, &temp_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_temperature(AS72653_UV, &temp_val);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_set_indicator_current
 * 
 */
static void test_as7265x_set_indicator_current(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  // act and assert
  // normal case
  status = as7265x_set_indicator_current(AS7265X_INDICATOR_CURRENT_LIMIT_1MA);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_indicator_current(AS7265X_INDICATOR_CURRENT_LIMIT_2MA);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_indicator_current(AS7265X_INDICATOR_CURRENT_LIMIT_4MA);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_indicator_current(AS7265X_INDICATOR_CURRENT_LIMIT_8MA);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_disable_indicator
 * 
 */
static void test_as7265x_disable_indicator(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  // act
  status = as7265x_disable_indicator();
  // assert
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_enable_indicator
 * 
 */
static void test_as7265x_enable_indicator(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  // act
  status = as7265x_enable_indicator();
  // assert
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_set_bulb_current
 * 
 */
static void test_as7265x_set_bulb_current(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  // act and assert
  // normal case
  status = as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_12_5MA,
                                    AS7265x_LED_WHITE);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_12_5MA,
                                    AS7265x_LED_IR);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_12_5MA,
                                    AS7265x_LED_UV);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_25MA,
                                    AS7265x_LED_WHITE);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_25MA,
                                    AS7265x_LED_IR);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_25MA,
                                    AS7265x_LED_UV);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_50MA,
                                    AS7265x_LED_WHITE);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_50MA,
                                    AS7265x_LED_IR);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_50MA,
                                    AS7265x_LED_UV);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_100MA,
                                    AS7265x_LED_WHITE);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_100MA,
                                    AS7265x_LED_IR);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_100MA,
                                    AS7265x_LED_UV);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_disable_bulb
 * 
 */
static void test_as7265x_disable_bulb(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  // act act and assert
  status = as7265x_disable_bulb(AS7265x_LED_WHITE);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_disable_bulb(AS7265x_LED_IR);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_disable_bulb(AS7265x_LED_UV);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_enable_bulb
 * 
 */
static void test_as7265x_enable_bulb(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  // act and assert
  status = as7265x_enable_bulb(AS7265x_LED_WHITE);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_enable_bulb(AS7265x_LED_IR);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_enable_bulb(AS7265x_LED_UV);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_data_available
 * 
 */
static void test_as7265x_data_available(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  bool is_data_ready;
  // act and assert
  // Null pointer
  status = as7265x_data_available(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
  // normal case
  status = as7265x_data_available(&is_data_ready);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_disable_interrupt
 * 
 */
static void test_as7265x_disable_interrupt(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  // act
  status = as7265x_disable_interrupt();
  // assert
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_enable_interrupt
 * 
 */
static void test_as7265x_enable_interrupt(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;

  // act
  status = as7265x_enable_interrupt();
  // assert
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_set_integration_cycles
 * 
 */
static void test_as7265x_set_integration_cycles(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;

  // act and assert
  status = as7265x_set_integration_cycles(10);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_integration_cycles(20);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_integration_cycles(50);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_set_gain
 * 
 */
static void test_as7265x_set_gain(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;

  // act and assert
  status = as7265x_set_gain(AS7265X_GAIN_1X);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_gain(AS7265X_GAIN_37X);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_gain(AS7265X_GAIN_16X);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_gain(AS7265X_GAIN_64X);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_set_gain
 * 
 */
static void test_as7265x_set_measurement_mode(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;

  // act and assert
  status = as7265x_set_measurement_mode(AS7265X_MEASUREMENT_MODE_4CHAN);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_measurement_mode(AS7265X_MEASUREMENT_MODE_4CHAN_2);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_measurement_mode(
    AS7265X_MEASUREMENT_MODE_6CHAN_CONTINUOUS);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_set_measurement_mode(
    AS7265X_MEASUREMENT_MODE_6CHAN_ONE_SHOT);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_get_calibrated_value
 * 
 */
static void test_as7265x_get_calibrated_value(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  float cal_val;

  // act and assert
  // null pointer
  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_A, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_B, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_C, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_D, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_E, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_F, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_G, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_H, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_I, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

    status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_J, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_K, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_L, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

    status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_R, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_S, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_T, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

    status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_U, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_V, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_W, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  // normal
  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_A, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_B, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_C, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_D, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_E, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_F, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_G, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_H, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_I, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

    status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_J, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_K, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_L, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

    status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_R, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_S, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_T, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

    status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_U, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_V, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_calibrated_value(AS7265x_COLOR_CHANNEL_W, &cal_val);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_get_all_calibrated_value
 * 
 */
static void test_as7265x_get_all_calibrated_value(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  as7265x_calibrated_data_t  cal_val;
  // act and assert

  status = as7265x_get_all_calibrated_value(&cal_val);
  CU_ASSERT(status == SL_STATUS_OK);
  status = as7265x_get_all_calibrated_value(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief Test function for as7265x_get_channel
 * 
 */
static void test_as7265x_get_channel(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  uint16_t color_data;
  // act and assert
  // Null pointer
  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_A, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_B, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_C, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_D, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_E, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_F, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_G, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_H, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_I, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

    status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_J, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_K, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_L, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

    status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_R, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_S, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_T, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

    status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_U, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_V, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_W, NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  // normal
  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_A, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_B, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_C, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_D, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_E, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_F, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_G, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_H, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_I, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

    status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_J, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_K, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_L, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

    status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_R, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_S, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_T, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

    status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_U, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_V, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_channel(AS7265x_COLOR_CHANNEL_W, &color_data);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_all_color_channel
 * 
 */
static void test_as7265x_all_color_channel(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  as7265x_color_data_t color_data;

  // act and assert
  status = as7265x_get_all_color_channel(&color_data);
  CU_ASSERT(status == SL_STATUS_OK);

  status = as7265x_get_all_color_channel(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief Test function for as7265x_take_measurements_with_bulb
 * 
 */
static void test_as7265x_take_measurements_with_bulb(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  // act
  status = as7265x_take_measurements_with_bulb();
  // assert
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_take_measurements
 * 
 */
static void test_as7265x_take_measurements(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  // act
  status = as7265x_take_measurements();
  // assert
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_get_firmware_version
 * 
 */
static void test_as7265x_get_firmware_version(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  as7265x_firmware_version_t firm_ver;
  // act and assert
  // Null pointer
  status = as7265x_get_firmware_version(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
  // normal case
  status = as7265x_get_firmware_version(&firm_ver);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_get_hardware_version
 * 
 */
static void test_as7265x_get_hardware_version(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  uint8_t hw_ver;
  // act and assert
  // Null pointer
  status = as7265x_get_hardware_version(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
  // normal case
  status = as7265x_get_hardware_version(&hw_ver);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_get_device_type
 * 
 */
static void test_as7265x_get_device_type(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  uint8_t dev_type;
  // act and assert
  // Null pointer
  status = as7265x_get_device_type(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
  // normal case
  status = as7265x_get_device_type(&dev_type);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test function for as7265x_init
 * 
 */
static void test_as7265x_init(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  // act
  status = as7265x_init();
  // assert
  CU_ASSERT(status == SL_STATUS_OK);
}

/*******************************************************************************
 *****************************      API       **********************************
 ******************************************************************************/
void ut_as7265x_run_test(void)
{
  // Init systick for clock() function
  time_retarget_init();
  printf("Start unit test.\n");
  printf("\nDo not touch the sensor during test!\n");

  // basic test
  CU_BasicRunMode mode = CU_BRM_VERBOSE;
  CU_ErrorAction error_action = CUEA_IGNORE;

  setvbuf(stdout, NULL, _IONBF, 0);
  error_action = CUEA_FAIL;

  /*
   * CU_BRM_VERBOSE
   * CU_BRM_NORMAL
   */
  mode = CU_BRM_VERBOSE;

  if (CU_initialize_registry()) {
    printf("\nInitialization of Test Registry failed.");
  } else {
    /* add test here */
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());

    /* Register suites. */
    if (CU_register_suites(suites) != CUE_SUCCESS) {
        fprintf(stderr, "suite registration failed - %s\n", CU_get_error_msg());
        exit(EXIT_FAILURE);
    }
    CU_basic_set_mode(mode);
    CU_set_error_action(error_action);

    printf("\nTests completed with return value %d.\n", CU_basic_run_tests());
    CU_cleanup_registry();
  }
}

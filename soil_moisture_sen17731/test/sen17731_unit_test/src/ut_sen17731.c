/***************************************************************************//**
 * @file ut_sen17731.c
 * @brief unit test for soi moisture sensor
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
#include "ut_common.h"
#include "sen17731.h"
#include "sl_i2cspm_instances.h"

// -----------------------------------------------------------------------------
//                       Local Functions
// -----------------------------------------------------------------------------

static void test_sen17731_set_i2cspm_instance(void);
static void test_sen17731_set_address(void);
static void test_sen17731_scan_address(void);
static void test_sen17731_select_device(void);
static void test_sen17731_led_on(void);
static void test_sen17731_led_off(void);
static void test_sen17731_set_dry_value(void);
static void test_sen17731_set_wet_value(void);
static void test_sen17731_get_range_values(void);
static void test_sen17731_get_moisture_raw(void);
static void test_sen17731_get_moisture(void);

// -----------------------------------------------------------------------------
//                       Local Variables
// -----------------------------------------------------------------------------

testCase_t testSuite[] = {
  { &test_sen17731_set_i2cspm_instance, "test_sen17731_set_i2cspm_instance" },
  { &test_sen17731_set_address, "test_sen17731_set_address" },
  { &test_sen17731_scan_address, "test_sen17731_scan_address" },
  { &test_sen17731_select_device, "test_sen17731_select_device" },
  { &test_sen17731_led_on, "test_sen17731_led_on" },
  { &test_sen17731_led_off, "test_sen17731_led_off" },
  { &test_sen17731_set_dry_value, "test_sen17731_set_dry_value" },
  { &test_sen17731_set_wet_value, "test_sen17731_set_wet_value" },
  { &test_sen17731_get_range_values, "test_sen17731_get_range_values" },
  { &test_sen17731_get_moisture_raw, "test_sen17731_get_moisture_raw" },
  { &test_sen17731_get_moisture, "test_sen17731_get_moisture" }
};

// -----------------------------------------------------------------------------
//                       Local Function Definition
// -----------------------------------------------------------------------------

/**
 * @brief Test fucntion for sen17731_set_i2cspm_instance
 */
static void test_sen17731_set_i2cspm_instance(void)
{
  sl_status_t status;

  status = sen17731_set_i2cspm_instance(NULL);
  TEST_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = sen17731_set_i2cspm_instance(sl_i2cspm_qwiic);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test fucntion for sen17731_set_address
 */
static void test_sen17731_set_address(void)
{
  sl_status_t status;
  uint16_t addr;

  addr = 0x06;
  status = sen17731_set_address(addr);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  addr = 0x28;
  status = sen17731_set_address(addr);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test fucntion for sen17731_scan_address
 */
static void test_sen17731_scan_address(void)
{
  sl_status_t status;
  uint16_t addr[255];
  uint8_t num_dev;

  status = sen17731_scan_address(NULL, NULL);
  TEST_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = sen17731_scan_address(addr, &num_dev);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test fucntion for sen17731_select_device
 */
static void test_sen17731_select_device(void)
{
  sl_status_t status;
  uint8_t addr = 0x28;

  status = sen17731_select_device(addr);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test fucntion for sen17731_led_on
 */
static void test_sen17731_led_on(void)
{
  sl_status_t status;

  status = sen17731_led_on();
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test fucntion for sen17731_led_off
 */
static void test_sen17731_led_off(void)
{
  sl_status_t status;

  status = sen17731_led_off();
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test fucntion for sen17731_set_dry_value
 */
static void test_sen17731_set_dry_value(void)
{
  uint16_t dry_value;
  sl_status_t status;

  dry_value = 1024;
  status = sen17731_set_dry_value(dry_value);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief Test fucntion for sen17731_set_wet_value
 */
static void test_sen17731_set_wet_value(void)
{
  uint16_t wet_value;
  sl_status_t status;

  wet_value = 1024;
  status = sen17731_set_wet_value(wet_value);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief Test fucntion for sen17731_get_range_values
 */
static void test_sen17731_get_range_values(void)
{
  sl_status_t status;
  sen17731_calibration_t range;

  status = sen17731_get_range_values(NULL);
  TEST_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = sen17731_get_range_values(&range);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test fucntion for sen17731_get_moisture
 */
static void test_sen17731_get_moisture_raw(void)
{
  sl_status_t status;
  uint16_t value;

  status = sen17731_get_moisture_raw(NULL);
  TEST_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = sen17731_get_moisture_raw(&value);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief Test fucntion for sen17731_get_moisture
 */
static void test_sen17731_get_moisture(void)
{
  sl_status_t status;
  uint8_t moisture;

  status = sen17731_get_moisture(NULL);
  TEST_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = sen17731_get_moisture(&moisture);
  TEST_ASSERT(status == SL_STATUS_OK);
}

// -----------------------------------------------------------------------------
//                       Public Function Definitions
// -----------------------------------------------------------------------------

void ut_sen17731_run_test(void)
{
  app_log("\n******************************************\n");
  app_log("Start unit test.\n");

  run_all(testSuite, GET_SIZE(testSuite));
  app_log("End testing\n");
}

/***************************************************************************//**
* @file ut_ak9753.c
* @brief unit test for the Human Presence driver
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
#include "ak9753.h"
#include "sl_i2cspm_instances.h"
#include "limits.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *****************************      Test cases      ***************************
 ******************************************************************************/

/**
 * @brief ak9753_tc_000: Testing for function ak9753_get_core_version
*/
static void ak9753_tc_000(void)
{
  ak9753_core_version_t core_version;

  ak9753_get_core_version(&core_version);

  TEST_ASSERT(core_version.major == AK9753_MAJOR_VERSION);
  TEST_ASSERT(core_version.build == AK9753_BUILD_VERSION);
  TEST_ASSERT(core_version.minor == AK9753_MINOR_VERSION);
  TEST_ASSERT(core_version.revision == AK9753_REVISION_VERSION);
}

/**
 * @brief ak9753_tc_001: Testing for function ak9753_init
*/
static void ak9753_tc_001(void)
{
  sl_status_t status;
  ak9753_config_t ak9753_config = {0};

  status = ak9753_init(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_init(&ak9753_config);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_002: Testing for function ak9753_get_mode
*/
static void ak9753_tc_002(void)
{
  sl_status_t status;
  ak9753_mode_t mode;

  status = ak9753_get_mode(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_mode(&mode);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_003: Testing for function ak9753_set_cutoff_freq
*/
static void ak9753_tc_003(void)
{
  sl_status_t status;
  ak9753_cutoff_freq_t fcList[] = {
    AK975X_FREQ_0_3HZ - 1,
    AK975X_FREQ_0_3HZ,
    AK975X_FREQ_0_6HZ,
    AK975X_FREQ_4_4HZ,
    AK975X_FREQ_8_8HZ,
    AK975X_FREQ_8_8HZ + 1
  };

  for (uint8_t i = 0; i < sizeof(fcList)/sizeof(fcList[0]); ++i)
  {
    status = ak9753_set_cutoff_freq(fcList[i]);
    TEST_ASSERT(status == SL_STATUS_OK);
  }
}

/**
 * @brief ak9753_tc_004: Testing for function ak9753_get_cutoff_freq
*/
static void ak9753_tc_004(void)
{
  sl_status_t status;
  uint8_t fc;

  status = ak9753_get_cutoff_freq(NULL);
	TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_cutoff_freq(&fc);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_005: Testing for function ak9753_is_data_ready
*/
static void ak9753_tc_005(void)
{
  sl_status_t status;
  bool data_ready;

  status = ak9753_is_data_ready(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_is_data_ready(&data_ready);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_006: Testing for function ak9753_set_mode
*/
static void ak9753_tc_006(void)
{
  sl_status_t status;
  ak9753_mode_t modeList[] = {
    AK975X_MODE_STANDBY - 1, 
    AK975X_MODE_STANDBY, 
    AK975X_MODE_EEPROM_ACCESS,
    AK975X_MODE_2,
    AK975X_MODE_3,
    AK975X_MODE_3 + 1
  };

  for (uint8_t i = 0; i < sizeof(modeList)/sizeof(modeList[0]); ++i)
  {
    status = ak9753_set_mode(modeList[i]);
    TEST_ASSERT(status == SL_STATUS_OK);
  }
}

/**
 * @brief ak9753_tc_007: Testing for function ak9753_get_dummy
*/
static void ak9753_tc_007(void)
{
  sl_status_t status;

  status = ak9753_get_dummy();
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_008: Testing for function ak9753_get_ir1_data
*/
static void ak9753_tc_008(void)
{
  sl_status_t status;
  int16_t measurement_data;

  status = ak9753_get_ir1_data(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_ir1_data(&measurement_data);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_009: Testing for function ak9753_get_ir2_data
*/
static void ak9753_tc_009(void)
{
  sl_status_t status;
  int16_t measurement_data;

  status = ak9753_get_ir2_data(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_ir2_data(&measurement_data);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_010: Testing for function ak9753_get_ir3_data
*/
static void ak9753_tc_010(void)
{
  sl_status_t status;
  int16_t measurement_data;

  status = ak9753_get_ir3_data(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_ir3_data(&measurement_data);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_011: Testing for function ak9753_get_ir4_data
*/
static void ak9753_tc_011(void)
{
  sl_status_t status;
  int16_t measurement_data;

  status = ak9753_get_ir4_data(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_ir4_data(&measurement_data);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_012: Testing for function ak9753_get_raw_temp
*/
static void ak9753_tc_012(void)
{
  sl_status_t status;
  int16_t measurement_data;

  status = ak9753_get_ir4_data(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_ir4_data(&measurement_data);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_013: Testing for function ak9753_get_tempC
*/
static void ak9753_tc_013(void)
{
  sl_status_t status;
  float tempC;

  status = ak9753_get_tempC(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_tempC(&tempC);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_014: Testing for function ak9753_get_tempF
*/
static void ak9753_tc_014(void)
{
  sl_status_t status;
  float tempF;

  status = ak9753_get_tempF(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_tempF(&tempF);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_015: Testing for function ak9753_set_threshold_ir24
*/
static void ak9753_tc_015(void)
{
  sl_status_t status;
  uint16_t threshold_value = 0;

  status = ak9753_set_threshold_ir24(true, threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);

  threshold_value = USHRT_MAX;
  status = ak9753_set_threshold_ir24(false, threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_016: Testing for function ak9753_get_threshold_ir24
*/
static void ak9753_tc_016(void)
{
  sl_status_t status;
  uint16_t threshold_value;

  status = ak9753_get_threshold_ir24(true, NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_threshold_ir24(true, &threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);

  status = ak9753_get_threshold_ir24(false, &threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_017: Testing for function ak9753_set_threshold_eeprom_ir24
*/
static void ak9753_tc_017(void)
{
  sl_status_t status;
  uint16_t threshold_value = 0;

  status = ak9753_set_threshold_eeprom_ir24(true, threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);

  threshold_value = USHRT_MAX;
  status = ak9753_set_threshold_eeprom_ir24(false, threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_018: Testing for function ak9753_get_eeprom_threshold_ir24
*/
static void ak9753_tc_018(void)
{
  sl_status_t status;
  uint16_t threshold_value;

  status = ak9753_get_eeprom_threshold_ir24(true, NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_eeprom_threshold_ir24(true, &threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);

  status = ak9753_get_eeprom_threshold_ir24(false, &threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_019: Testing for function ak9753_set_threshold_ir13
*/
static void ak9753_tc_019(void)
{
  sl_status_t status;
  uint16_t threshold_value = 0;

  status = ak9753_set_threshold_ir13(true, threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);

  threshold_value = USHRT_MAX;
  status = ak9753_set_threshold_ir13(false, threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_020: Testing for function ak9753_get_threshold_ir13
*/
static void ak9753_tc_020(void)
{
  sl_status_t status;
  uint16_t threshold_value;

  status = ak9753_get_threshold_ir13(true, NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_threshold_ir13(true, &threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);

  status = ak9753_get_threshold_ir13(false, &threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_021: Testing for function ak9753_set_threshold_eeprom_ir13
*/
static void ak9753_tc_021(void)
{
  sl_status_t status;
  uint16_t threshold_value = 0;

  status = ak9753_set_threshold_eeprom_ir13(true, threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);

  threshold_value = USHRT_MAX;
  status = ak9753_set_threshold_eeprom_ir13(false, threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_022: Testing for function ak9753_get_eeprom_threshold_ir13
*/
static void ak9753_tc_022(void)
{
  sl_status_t status;
  uint16_t threshold_value;

  status = ak9753_get_eeprom_threshold_ir13(true, NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_eeprom_threshold_ir13(true, &threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);

  status = ak9753_get_eeprom_threshold_ir13(false, &threshold_value);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_023: Testing for function ak9753_set_hysteresis_ir24
*/
static void ak9753_tc_023(void)
{
  sl_status_t status;
  int16_t testList[] = {-1, 0, UCHAR_MAX, UCHAR_MAX + 1};

  for (uint8_t i = 0; i < sizeof(testList)/sizeof(testList[0]); i++) {
    status = ak9753_set_hysteresis_ir24(testList[i]);
    TEST_ASSERT(status == SL_STATUS_OK);
  }
}

/**
 * @brief ak9753_tc_024: Testing for function ak9753_get_hysteresis_ir24
*/
static void ak9753_tc_024(void)
{
  sl_status_t status;
  uint8_t hysteresis_value;

  status = ak9753_get_hysteresis_ir24(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_hysteresis_ir24(&hysteresis_value);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_025: Testing for function ak9753_set_hysteresis_eeprom_ir24
*/
static void ak9753_tc_025(void)
{
  sl_status_t status;
  int16_t testList[] = {-1, 0, UCHAR_MAX, UCHAR_MAX + 1};

  for (uint8_t i = 0; i < sizeof(testList)/sizeof(testList[0]); i++) {
    status = ak9753_set_hysteresis_eeprom_ir24(testList[i]);
    TEST_ASSERT(status == SL_STATUS_OK);
  }
}

/**
 * @brief ak9753_tc_026: Testing for function ak9753_get_eeprom_hysteresis_ir24
*/
static void ak9753_tc_026(void)
{
  sl_status_t status;
  uint8_t hysteresis_value;

  status = ak9753_get_eeprom_hysteresis_ir24(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_eeprom_hysteresis_ir24(&hysteresis_value);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_027: Testing for function ak9753_get_hysteresis_ir24
*/
static void ak9753_tc_027(void)
{
  sl_status_t status;
  uint8_t hysteresis_value;

  status = ak9753_get_hysteresis_ir24(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_hysteresis_ir24(&hysteresis_value);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_028: Testing for function ak9753_set_hysteresis_ir13
*/
static void ak9753_tc_028(void)
{
  sl_status_t status;
  int16_t testList[] = {-1, 0, UCHAR_MAX, UCHAR_MAX + 1};

  for (uint8_t i = 0; i < sizeof(testList)/sizeof(testList[0]); i++) {
    status = ak9753_set_hysteresis_ir13(testList[i]);
    TEST_ASSERT(status == SL_STATUS_OK);
  }
}

/**
 * @brief ak9753_tc_029: Testing for function ak9753_get_hysteresis_ir13
*/
static void ak9753_tc_029(void)
{
  sl_status_t status;
  uint8_t hysteresis_value;

  status = ak9753_get_hysteresis_ir13(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_hysteresis_ir13(&hysteresis_value);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_030: Testing for function ak9753_set_hysteresis_eeprom_ir13
*/
static void ak9753_tc_030(void)
{
  sl_status_t status;
  int16_t testList[] = {-1, 0, UCHAR_MAX, UCHAR_MAX + 1};

  for (uint8_t i = 0; i < sizeof(testList)/sizeof(testList[0]); i++) {
    status = ak9753_set_hysteresis_eeprom_ir13(testList[i]);
    TEST_ASSERT(status == SL_STATUS_OK);
  }
}

/**
 * @brief ak9753_tc_031: Testing for function ak9753_get_eeprom_hysteresis_ir13
*/
static void ak9753_tc_031(void)
{
  sl_status_t status;
  uint8_t hysteresis_value;

  status = ak9753_get_eeprom_hysteresis_ir13(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_eeprom_hysteresis_ir13(&hysteresis_value);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_032: Testing for function ak9753_set_interrupts
*/
static void ak9753_tc_032(void)
{
  sl_status_t status;
  int16_t testList[] = {-1, 0, UCHAR_MAX, UCHAR_MAX + 1};

  for (uint8_t i = 0; i < sizeof(testList)/sizeof(testList[0]); i++) {
    status = ak9753_set_interrupts(testList[i]);
    TEST_ASSERT(status == SL_STATUS_OK);
  }
}

/**
 * @brief ak9753_tc_033: Testing for function ak9753_get_int_source
*/
static void ak9753_tc_033(void)
{
  sl_status_t status;
  uint8_t int_source;

  status = ak9753_get_int_source(NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_get_int_source(&int_source);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_034: Testing for function ak9753_is_data_overrun
*/
static void ak9753_tc_034(void)
{
  sl_status_t status;
  bool data_overrun;

  status = ak9753_is_data_overrun(NULL);
	TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_is_data_overrun(&data_overrun);
  TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_035: Testing for function ak9753_sw_reset
*/
static void ak9753_tc_035(void)
{
  sl_status_t status;
  status = ak9753_sw_reset();
	TEST_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief ak9753_tc_036: Testing for function ak9753_is_interrupt
*/
static void ak9753_tc_036(void)
{
  sl_status_t status;
  bool isIR13H;
  bool isIR13L;
  bool isIR24H;
  bool isIR24L;
  bool isDataReady;
  bool isDataOverrun;

  // valid test
  status = ak9753_is_interrupt( &isIR13H,
                                &isIR13L,
                                &isIR24H,
                                &isIR24L,
                                &isDataReady,
                                &isDataOverrun);
  TEST_ASSERT(status == SL_STATUS_OK);

  // NULL input test
  status = ak9753_is_interrupt( NULL, &isIR13L, &isIR24H, &isIR24L, &isDataReady, &isDataOverrun);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_is_interrupt( &isIR13H, NULL, &isIR24H, &isIR24L, &isDataReady, &isDataOverrun);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_is_interrupt( &isIR13H, &isIR13L, NULL, &isIR24L, &isDataReady, &isDataOverrun);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_is_interrupt( &isIR13H, &isIR13L, &isIR24H, NULL, &isDataReady, &isDataOverrun);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_is_interrupt( &isIR13H, &isIR13L, &isIR24H, &isIR24L, NULL, &isDataOverrun);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = ak9753_is_interrupt( &isIR13H, &isIR13L, &isIR24H, &isIR24L, &isDataReady, NULL);
  TEST_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief ak9753_tc_037: Testing for function ak9753_deinit
*/
static void ak9753_tc_037(void)
{
  sl_status_t status;

  status = ak9753_deinit();
  TEST_ASSERT(status == SL_STATUS_OK);

  status = ak9753_deinit();
  TEST_ASSERT(status == SL_STATUS_OK);
}

/*******************************************************************************
 *****************************      API       **********************************
 ******************************************************************************/

testCase_t testSuite[] = {
  {&ak9753_tc_000, "ak9753_tc_000"},
  {&ak9753_tc_001, "ak9753_tc_001"},
  {&ak9753_tc_002, "ak9753_tc_002"},
  {&ak9753_tc_003, "ak9753_tc_003"},
  {&ak9753_tc_004, "ak9753_tc_004"},
  {&ak9753_tc_005, "ak9753_tc_005"},
  {&ak9753_tc_006, "ak9753_tc_006"},
  {&ak9753_tc_007, "ak9753_tc_007"},
  {&ak9753_tc_008, "ak9753_tc_008"},
  {&ak9753_tc_009, "ak9753_tc_009"},
  {&ak9753_tc_010, "ak9753_tc_010"},
  {&ak9753_tc_011, "ak9753_tc_011"},
  {&ak9753_tc_012, "ak9753_tc_012"},
  {&ak9753_tc_013, "ak9753_tc_013"},
  {&ak9753_tc_014, "ak9753_tc_014"},
  {&ak9753_tc_015, "ak9753_tc_015"},
  {&ak9753_tc_016, "ak9753_tc_016"},
  {&ak9753_tc_017, "ak9753_tc_017"},
  {&ak9753_tc_018, "ak9753_tc_018"},
  {&ak9753_tc_019, "ak9753_tc_019"},
  {&ak9753_tc_020, "ak9753_tc_020"},
  {&ak9753_tc_021, "ak9753_tc_021"},
  {&ak9753_tc_022, "ak9753_tc_022"},
  {&ak9753_tc_023, "ak9753_tc_023"},
  {&ak9753_tc_024, "ak9753_tc_024"},
  {&ak9753_tc_025, "ak9753_tc_025"},
  {&ak9753_tc_026, "ak9753_tc_026"},
  {&ak9753_tc_027, "ak9753_tc_027"},
  {&ak9753_tc_028, "ak9753_tc_028"},
  {&ak9753_tc_029, "ak9753_tc_029"},
  {&ak9753_tc_030, "ak9753_tc_030"},
  {&ak9753_tc_031, "ak9753_tc_031"},
  {&ak9753_tc_032, "ak9753_tc_032"},
  {&ak9753_tc_033, "ak9753_tc_033"},
  {&ak9753_tc_034, "ak9753_tc_034"},
  {&ak9753_tc_035, "ak9753_tc_035"},
  {&ak9753_tc_036, "ak9753_tc_036"},
  {&ak9753_tc_037, "ak9753_tc_037"},
};

void ut_ak9753_run_test(void)
{
	app_log("\n******************************************\n");
  app_log("Start unit test.\n");

  run_all(testSuite, GET_SIZE(testSuite));
	app_log("End testing\n");
}

#ifdef __cplusplus
}
#endif
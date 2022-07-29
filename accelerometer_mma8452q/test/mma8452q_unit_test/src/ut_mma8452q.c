/***************************************************************************//**
* @file ut_mma8452q.c
* @brief unit test for the triple axis accelerometer mma8452q
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
#include "mma8452q_config.h"
#include "ut_mma8452q.h"
#include "sl_i2cspm_instances.h"

#include "time_retarget.h"

#include "Basic.h"
#include "CUnit.h"
#include "mma8452q.h"

// Local test functions.
static void test_mma8452q_init(void);
static void test_mma8452q_deinit(void);
static void test_mma8452q_set_address(void);
static void test_mma8452q_set_scale(void);
static void test_mma8452q_set_odr(void);
static void test_mma8452q_set_mods(void);
static void test_mma8452q_config_aslp(void);
static void test_mma8452q_config_trans(void);
static void test_mma8452q_config_orientation(void);
static void test_mma8452q_config_pulse(void);
static void test_mma8452q_config_ff_mt(void);
static void test_mma8452q_config_interrupt(void);
static void test_mma8452q_enable_low_noise(void);
static void test_mma8452q_enable_fast_read(void);
static void test_mma8452q_active(void);
static void test_mma8452q_auto_calibrate(void);
static void test_sl_mma8452q_getAcceleration(void);
static void test_sl_mma8452q_getCalculatedAcceleration(void);
static void test_mma8452q_getDataStatus(void);
static void test_mma8452q_check_for_data_ready(void);
static void test_mma8452q_getSysMode(void);
static void test_mma8452q_getIntSource(void);
static void test_mma8452q_getPLStatus(void);
static void test_mma8452q_get_ff_mt_status(void);
static void test_mma8452q_get_transient_status(void);
static void test_mma8452q_get_pulse_status(void);

// Test case variable.
static CU_TestInfo mma8452q_unit_test[] = {
    {"test_mma8452q_init", test_mma8452q_init},
    {"test_mma8452q_set_address",test_mma8452q_set_address},
    {"test_mma8452q_set_scale",test_mma8452q_set_scale},
    {"test_mma8452q_set_odr",test_mma8452q_set_odr},
    {"test_mma8452q_set_mods",test_mma8452q_set_mods},
    {"test_mma8452q_config_aslp",test_mma8452q_config_aslp},
    {"test_mma8452q_config_trans",test_mma8452q_config_trans},
    {"test_mma8452q_config_orientation",test_mma8452q_config_orientation},
    {"test_mma8452q_config_pulse",test_mma8452q_config_pulse},
    {"test_mma8452q_config_ff_mt",test_mma8452q_config_ff_mt},
    {"test_mma8452q_config_interrupt",test_mma8452q_config_interrupt},
    {"test_mma8452q_enable_low_noise",test_mma8452q_enable_low_noise},
    {"test_mma8452q_enable_fast_read",test_mma8452q_enable_fast_read},
    {"test_mma8452q_active",test_mma8452q_active},
    {"test_mma8452q_auto_calibrate",test_mma8452q_auto_calibrate},
    {"test_sl_mma8452q_get_acceleration",test_sl_mma8452q_getAcceleration},
    {"test_sl_mma8452q_get_calculated_acceleration",test_sl_mma8452q_getCalculatedAcceleration},
    {"test_mma8452q_get_data_status",test_mma8452q_getDataStatus},
    {"test_mma8452q_check_for_data_ready",test_mma8452q_check_for_data_ready},
    {"test_mma8452q_get_sys_mode",test_mma8452q_getSysMode},
    {"test_mma8452q_get_int_source",test_mma8452q_getIntSource},
    {"test_mma8452q_get_pl_status",test_mma8452q_getPLStatus},
    {"test_mma8452q_get_ff_mt_status",test_mma8452q_get_ff_mt_status},
    {"test_mma8452q_get_transient_status",test_mma8452q_get_transient_status},
    {"test_mma8452q_get_pulse_status",test_mma8452q_get_pulse_status},
    {"test_mma8452q_deinit",test_mma8452q_deinit},
    CU_TEST_INFO_NULL,
};

static CU_SuiteInfo suites[] = {
    {"mma8452q_unit_test", NULL, NULL, NULL, NULL, mma8452q_unit_test},
    CU_SUITE_INFO_NULL,
};

/**
 * @brief test_mma8452q_init: Test function for mma8452q_init
 *
 */
static void test_mma8452q_init(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;

  // invalid parameter case
  status = mma8452q_init(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  // normal case
  status = mma8452q_init(sl_i2cspm_qwiic);
  CU_ASSERT(status == SL_STATUS_OK);


  // already initialized case
  status = mma8452q_init(sl_i2cspm_qwiic);
  CU_ASSERT(status == SL_STATUS_ALREADY_INITIALIZED);

}

/**
 * @brief test_mma8452q_deinit: Test function for mma8452q_deinit
 *
 */
static void test_mma8452q_deinit(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;

  // normal case
  status = mma8452q_deinit();
  CU_ASSERT(status == SL_STATUS_OK);

  // not initialized case.
  status = mma8452q_deinit();
  CU_ASSERT(status == SL_STATUS_NOT_INITIALIZED);
}

/**
 * @brief test_mma8452q_set_address: Test function for mma8452q_set_address
 *
 */
static void test_mma8452q_set_address(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;

  // normal case
  status = mma8452q_set_address(SL_MMA8452Q_I2C_BUS_ADDRESS2);
  CU_ASSERT(status == SL_STATUS_OK);

  // invalid parameter case
  status = mma8452q_set_address(0x00);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief test_mma8452q_set_scale: Test function for mma8452q_set_scale
 * 
 */
static void test_mma8452q_set_scale(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  mma8452q_scale_t scale;

  // normal case
  scale = SCALE_2G;
  status = mma8452q_set_scale(scale);
  CU_ASSERT(status == SL_STATUS_OK);

  scale = SCALE_4G;
  status = mma8452q_set_scale(scale);
  CU_ASSERT(status == SL_STATUS_OK);

  scale = SCALE_8G;
  status = mma8452q_set_scale(scale);
  CU_ASSERT(status == SL_STATUS_OK);

  // invalid parameter case
  scale = SCALE_2G - 1;
  status = mma8452q_set_scale(scale);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
  scale = SCALE_8G + 1;
  status = mma8452q_set_scale(scale);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief test_mma8452q_set_odr: Test function for mma8452q_set_odr
 * 
 */
static void test_mma8452q_set_odr(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  mma8452q_odr_t odr;

  // normal case
  odr = ODR_1;
  status = mma8452q_set_odr(odr);
  CU_ASSERT(status == SL_STATUS_OK);

  odr = ODR_6;
  status = mma8452q_set_odr(odr);
  CU_ASSERT(status == SL_STATUS_OK);

  odr = ODR_400;
  status = mma8452q_set_odr(odr);
  CU_ASSERT(status == SL_STATUS_OK);

  odr = ODR_800;
  status = mma8452q_set_odr(odr);
  CU_ASSERT(status == SL_STATUS_OK);

  // invalid parameter case
  odr = ODR_800 - 1;
  status = mma8452q_set_odr(odr);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
  odr = ODR_1 + 1;
  status = mma8452q_set_odr(odr);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief test_mma8452q_set_mods: Test function for mma8452q_set_mods
 * 
 */
static void test_mma8452q_set_mods(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  mma8452q_mods_t mods;

  // normal case
  mods = NORMAL;
  status = mma8452q_set_mods(mods);
  CU_ASSERT(status == SL_STATUS_OK);

  mods = LNOISE_LPWR;
  status = mma8452q_set_mods(mods);
  CU_ASSERT(status == SL_STATUS_OK);

  mods = HI_RES;
  status = mma8452q_set_mods(mods);
  CU_ASSERT(status == SL_STATUS_OK);

  mods = LPWR;
  status = mma8452q_set_mods(mods);
  CU_ASSERT(status == SL_STATUS_OK);

  // invalid parameter case
  mods = NORMAL - 1;
  status = mma8452q_set_mods(mods);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
  mods = LPWR + 1;
  status = mma8452q_set_mods(mods);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief test_mma8452q_config_aslp: Test function for mma8452q_config_aslp
 * 
 */
static void test_mma8452q_config_aslp(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  mma8452q_aslp_odr_t alsp_rate;
  mma8452q_mods_t slp_mode_pwr;
  mma8452q_aslp_config_t aslp_cfg;

  // normal case
  alsp_rate = ASLP_ODR_50;
  slp_mode_pwr = NORMAL;
  aslp_cfg.alsp_count = 10;
  status = mma8452q_config_aslp(alsp_rate, slp_mode_pwr, aslp_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  alsp_rate = ASLP_ODR_12p5;
  slp_mode_pwr = LNOISE_LPWR;
  aslp_cfg.alsp_count = 10;
  status = mma8452q_config_aslp(alsp_rate, slp_mode_pwr, aslp_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  alsp_rate = ASLP_ODR_6p25;
  slp_mode_pwr = HI_RES;
  aslp_cfg.alsp_count = 10;
  status = mma8452q_config_aslp(alsp_rate, slp_mode_pwr, aslp_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  alsp_rate = ASLP_ODR_1p56;
  slp_mode_pwr = LPWR;
  aslp_cfg.alsp_count = 10;
  status = mma8452q_config_aslp(alsp_rate, slp_mode_pwr, aslp_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  // invalid parameter case
  alsp_rate = ASLP_ODR_50 - 1;
  slp_mode_pwr = NORMAL;
  aslp_cfg.alsp_count = 10;
  status = mma8452q_config_aslp(alsp_rate, slp_mode_pwr, aslp_cfg);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  alsp_rate = ASLP_ODR_50;
  slp_mode_pwr = NORMAL - 1;
  aslp_cfg.alsp_count = 10;
  status = mma8452q_config_aslp(alsp_rate, slp_mode_pwr, aslp_cfg);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

}

/**
 * @brief test_mma8452q_config_trans: Test function for mma8452q_config_trans
 * 
 */
static void test_mma8452q_config_trans(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  mma8452q_trans_config_t trans_cfg;

  // normal case
  trans_cfg.en_event_latch = true;
  trans_cfg.en_z_trans = true;
  trans_cfg.en_y_trans = true;
  trans_cfg.en_x_trans = true;
  trans_cfg.en_hpf_bypass = true;
  trans_cfg.db_cnt_mode = true;
  trans_cfg.threshold = 10;
  trans_cfg.debounce_cnt = 1;

  status = mma8452q_config_trans(trans_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  trans_cfg.en_event_latch = false;
  trans_cfg.en_z_trans = false;
  trans_cfg.en_y_trans = false;
  trans_cfg.en_x_trans = false;
  trans_cfg.en_hpf_bypass = false;
  trans_cfg.db_cnt_mode = false;
  trans_cfg.threshold = 10;
  trans_cfg.debounce_cnt = 1;

  status = mma8452q_config_trans(trans_cfg);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief test_mma8452q_config_orientation: Test function for
 * mma8452q_config_orientation
 * 
 */
static void test_mma8452q_config_orientation(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  mma8452q_orientation_config_t orient_cfg;

  // normal case
  orient_cfg.db_cnt_mode = true;
  orient_cfg.en_event_latch = true;
  orient_cfg.debounce_cnt = 1;

  status = mma8452q_config_orientation(orient_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  orient_cfg.db_cnt_mode = false;
  orient_cfg.en_event_latch = false;
  orient_cfg.debounce_cnt = 1;

  status = mma8452q_config_orientation(orient_cfg);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief test_mma8452q_config_pulse: Test function for mma8452q_config_pulse
 * 
 */
static void test_mma8452q_config_pulse(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  mma8452q_pulse_config_t pulse_cfg;

  // normal case
  pulse_cfg.double_abort = true;
  pulse_cfg.en_event_latch = true;
  pulse_cfg.en_z_double = true;
  pulse_cfg.en_z_single = true;
  pulse_cfg.en_y_double = true;
  pulse_cfg.en_y_single = true;
  pulse_cfg.en_x_double = true;
  pulse_cfg.en_x_single = true;
  pulse_cfg.pulse_thresh_z = 1;
  pulse_cfg.pulse_thresh_y = 1;
  pulse_cfg.pulse_thresh_X = 1;
  pulse_cfg.pulse_time_lmt = 1;
  pulse_cfg.pulse_latency = 1;
  pulse_cfg.pulse_window = 1;

  status = mma8452q_config_pulse(pulse_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  pulse_cfg.double_abort = false;
  pulse_cfg.en_event_latch = false;
  pulse_cfg.en_z_double = false;
  pulse_cfg.en_z_single = false;
  pulse_cfg.en_y_double = false;
  pulse_cfg.en_y_single = false;
  pulse_cfg.en_x_double = false;
  pulse_cfg.en_x_single = false;
  pulse_cfg.pulse_thresh_z = 1;
  pulse_cfg.pulse_thresh_y = 1;
  pulse_cfg.pulse_thresh_X = 1;
  pulse_cfg.pulse_time_lmt = 1;
  pulse_cfg.pulse_latency = 1;
  pulse_cfg.pulse_window = 1;

  status = mma8452q_config_pulse(pulse_cfg);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief test_mma8452q_config_ff_mt: Test function for mma8452q_config_ff_mt
 * 
 */
static void test_mma8452q_config_ff_mt(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  mma8452q_ff_mt_config_t ff_mt_cfg;

  // normal case.
  ff_mt_cfg.en_event_latch = true;
  ff_mt_cfg.ff_mt_sel = true;
  ff_mt_cfg.en_z_trans = true;
  ff_mt_cfg.en_y_trans = true;
  ff_mt_cfg.en_x_trans = true;
  ff_mt_cfg.db_cnt_mode = true;
  ff_mt_cfg.threshold = 1;
  ff_mt_cfg.debounce_cnt = 1;

  status = mma8452q_config_ff_mt(ff_mt_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  ff_mt_cfg.en_event_latch = false;
  ff_mt_cfg.ff_mt_sel = false;
  ff_mt_cfg.en_z_trans = false;
  ff_mt_cfg.en_y_trans = false;
  ff_mt_cfg.en_x_trans = false;
  ff_mt_cfg.db_cnt_mode = false;
  ff_mt_cfg.threshold = 1;
  ff_mt_cfg.debounce_cnt = 1;

  status = mma8452q_config_ff_mt(ff_mt_cfg);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief test_mma8452q_config_interrupt: Test function for
 * mma8452q_config_interrupt
 * 
 */
static void test_mma8452q_config_interrupt(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  mma8452q_interrupt_config_t interrupt_cfg;

  // normal case
  interrupt_cfg.en_trans_wake = true;
  interrupt_cfg.en_orientation_wake = true;
  interrupt_cfg.en_pulse_wake = true;
  interrupt_cfg.en_ff_mt_wake = true;
  interrupt_cfg.int_active_hi = true;
  interrupt_cfg.int_open_drain = true;
  interrupt_cfg.en_aslp_int = true;
  interrupt_cfg.en_trans_int = true;
  interrupt_cfg.en_orientation_int = true;
  interrupt_cfg.en_pulse_int = true;
  interrupt_cfg.en_ff_mt_int = true;
  interrupt_cfg.en_drdy_int = true;
  interrupt_cfg.cfg_aslp_int = true;
  interrupt_cfg.cfg_trans_int = true;
  interrupt_cfg.cfg_orientation_int = true;
  interrupt_cfg.cfg_pulse_int = true;
  interrupt_cfg.cfg_ff_mt_int = true;
  interrupt_cfg.cfg_drdy_int = true;

  status = mma8452q_config_interrupt(interrupt_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  interrupt_cfg.en_trans_wake = false;
  interrupt_cfg.en_orientation_wake = false;
  interrupt_cfg.en_pulse_wake = false;
  interrupt_cfg.en_ff_mt_wake = false;
  interrupt_cfg.int_active_hi = false;
  interrupt_cfg.int_open_drain = false;
  interrupt_cfg.en_aslp_int = false;
  interrupt_cfg.en_trans_int = false;
  interrupt_cfg.en_orientation_int = false;
  interrupt_cfg.en_pulse_int = false;
  interrupt_cfg.en_ff_mt_int = false;
  interrupt_cfg.en_drdy_int = false;
  interrupt_cfg.cfg_aslp_int = false;
  interrupt_cfg.cfg_trans_int = false;
  interrupt_cfg.cfg_orientation_int = false;
  interrupt_cfg.cfg_pulse_int = false;
  interrupt_cfg.cfg_ff_mt_int = false;
  interrupt_cfg.cfg_drdy_int = false;

  status = mma8452q_config_interrupt(interrupt_cfg);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief test_mma8452q_enable_low_noise: Test function for
 * mma8452q_enable_low_noise
 * 
 */
static void test_mma8452q_enable_low_noise(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;

  // normal case.
  status = mma8452q_enable_low_noise(true);
  CU_ASSERT(status == SL_STATUS_OK);

  status = mma8452q_enable_low_noise(false);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief test_mma8452q_enable_fast_read: Test function for
 * mma8452q_enable_fast_read
 * 
 */
static void test_mma8452q_enable_fast_read(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;

  // normal case.
  status = mma8452q_enable_fast_read(true);
  CU_ASSERT(status == SL_STATUS_OK);

  status = mma8452q_enable_fast_read(false);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief test_mma8452q_active: Test function for mma8452q_active
 * 
 */
static void test_mma8452q_active(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;

  // normal case.
  status = mma8452q_active(true);
  CU_ASSERT(status == SL_STATUS_OK);

  status = mma8452q_active(false);
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief test_mma8452q_auto_calibrate: Test function for mma8452q_auto_calibrate
 * 
 */
static void test_mma8452q_auto_calibrate(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;

  // normal case.
  status = mma8452q_auto_calibrate();
  CU_ASSERT(status == SL_STATUS_OK);
}

/**
 * @brief test_sl_mma8452q_getAcceleration: Test function for
 * sl_mma8452q_getAcceleration
 * 
 */
static void test_sl_mma8452q_getAcceleration(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  uint16_t avec[3];

  // normal case.
  status = sl_mma8452q_get_acceleration(avec);
  CU_ASSERT(status == SL_STATUS_OK);

}

/**
 * @brief test_sl_mma8452q_getCalculatedAcceleration: Test function for
 * sl_mma8452q_getCalculatedAcceleration
 *
 */
static void test_sl_mma8452q_getCalculatedAcceleration(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  float avec[3];

  // normal case.
  status = sl_mma8452q_get_calculated_acceleration(avec);
  CU_ASSERT(status == SL_STATUS_OK);

}

/**
 * @brief test_mma8452q_getDataStatus: Test function for mma8452q_getDataStatus
 *
 */
static void test_mma8452q_getDataStatus(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  uint8_t data_status;

  // normal case.
  status = mma8452q_get_data_status(&data_status);
  CU_ASSERT(status == SL_STATUS_OK);

  // invalid parameter case.
  status = mma8452q_get_data_status(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief test_mma8452q_check_for_data_ready: Test function for
 * mma8452q_check_for_data_ready
 * 
 */
static void test_mma8452q_check_for_data_ready(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  uint8_t is_data_ready;

  // normal case.
  status = mma8452q_check_for_data_ready(&is_data_ready);
  CU_ASSERT(status == SL_STATUS_OK);

  // invalid parameter case.
  status = mma8452q_check_for_data_ready(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief test_mma8452q_getSysMode: Test function for check_for_data_ready
 * 
 */
static void test_mma8452q_getSysMode(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  uint8_t reg_sysmode;

  // normal case.
  status = mma8452q_get_sysmode(&reg_sysmode);
  CU_ASSERT(status == SL_STATUS_OK);

  // invalid parameter case.
  status = mma8452q_get_sysmode(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief test_mma8452q_getIntSource: Test function for mma8452q_getIntSource
 * 
 */
static void test_mma8452q_getIntSource(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  uint8_t int_source;

  // normal case.
  status = mma8452q_get_int_source(&int_source);
  CU_ASSERT(status == SL_STATUS_OK);

  // invalid parameter case.
  status = mma8452q_get_int_source(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief test_mma8452q_getPLStatus: Test function for mma8452q_getPLStatus
 * 
 */
static void test_mma8452q_getPLStatus(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  uint8_t pl_status;

  // normal case.
  status = mma8452q_get_pl_status(&pl_status);
  CU_ASSERT(status == SL_STATUS_OK);

  // invalid parameter case.
  status = mma8452q_get_pl_status(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief test_mma8452q_get_ff_mt_status: Test function for mma8452q_get_ff_mt_status
 * 
 */
static void test_mma8452q_get_ff_mt_status(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  uint8_t ff_mt_status;

  // normal case.
  status = mma8452q_get_ff_mt_status(&ff_mt_status);
  CU_ASSERT(status == SL_STATUS_OK);

  // invalid parameter case.
  status = mma8452q_get_ff_mt_status(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief test_mma8452q_get_transient_status: Test function for
 * mma8452q_get_transient_status
 * 
 */
static void test_mma8452q_get_transient_status(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  uint8_t transient_status;

  // normal case.
  status = mma8452q_get_transient_status(&transient_status);
  CU_ASSERT(status == SL_STATUS_OK);

  // invalid parameter case.
  status = mma8452q_get_transient_status(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief test_mma8452q_get_pulse_status: Test function for
 * mma8452q_get_pulse_status
 * 
 */
static void test_mma8452q_get_pulse_status(void)
{
  // arrange
  sl_status_t status = SL_STATUS_OK;
  uint8_t pulse_status;

  // normal case.
  status = mma8452q_get_pulse_status(&pulse_status);
  CU_ASSERT(status == SL_STATUS_OK);

  // invalid parameter case.
  status = mma8452q_get_pulse_status(NULL);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);
}

void ut_mma8452q_run_test(void)
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

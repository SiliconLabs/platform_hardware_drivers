/***************************************************************************//**
* @file ut_cap1166.c
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
#include "ut_cap1166.h"

#include "spidrv.h"
#include "sl_spidrv_instances.h"

#include "cap1166.h"
#include "cap1166_config.h"

static cap1166_handle_t my_cap1166_handle;
static cap1166_cfg_t my_cap1166_config = CAP11666_DEFAULT_CONFIG;

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/
static void test_cap1166_sensor_enable(void);
static void test_cap1166_sensor_int_enable(void);
static void test_cap1166_sensor_repeat_rate_enable(void);
static void test_cap1166_sensor_threshold_config(void);
static void test_cap1166_sensor_recalib_config(void);
static void test_cap1166_sensor_noise_config(void);
static void test_cap1166_sensor_sensitivity_config(void);
static void test_cap1166_sensor_sensing_cycle_config(void);
static void test_cap1166_int_pin_config(void);
static void test_cap1166_multiple_touch_config(void);
static void test_cap1166_press_and_hold_config(void);
static void test_cap1166_init(void);
static void test_cap1166_config(void);
static void test_cap1166_proximity_detection_enable(void);
static void test_cap1166_touch_pattern_enable(void);
static void test_cap1166_detect_touch(void);
static void test_cap1166_identify(void);
static void test_cap1166_set_power_mode(void);
static void test_cap1166_check_interrupt_reason(void);
static void test_cap1166_reset(void);
static void test_cap1166_led_config(void);
static void test_cap1166_led_direct_set(void);

/*******************************************************************************
 *****************************   LOCAL VARS   **********************************
 ******************************************************************************/
static CU_TestInfo cap1166_unit_test[] = {
    { "test_cap1166_check_interrupt_reason",
        test_cap1166_check_interrupt_reason },
    { "test_cap1166_reset",
        test_cap1166_reset },
    { "test_cap1166_init",
        test_cap1166_init },
    { "test_cap1166_config",
        test_cap1166_config },
    { "test_cap1166_sensor_enable",
        test_cap1166_sensor_enable },
    { "test_cap1166_sensor_int_enable",
        test_cap1166_sensor_int_enable },
    { "test_cap1166_sensor_repeat_rate_enable",
        test_cap1166_sensor_repeat_rate_enable },
    { "test_cap1166_sensor_threshold_config",
        test_cap1166_sensor_threshold_config },
    { "test_cap1166_sensor_recalib_config",
        test_cap1166_sensor_recalib_config },
    { "test_cap1166_sensor_noise_config",
        test_cap1166_sensor_noise_config },
    { "test_cap1166_sensor_sensitivity_config",
        test_cap1166_sensor_sensitivity_config },
    { "test_cap1166_sensor_sensing_cycle_config",
        test_cap1166_sensor_sensing_cycle_config },
    { "test_cap1166_int_pin_config",
        test_cap1166_int_pin_config },
    { "test_cap1166_multiple_touch_config",
        test_cap1166_multiple_touch_config },
    { "test_cap1166_press_and_hold_config",
        test_cap1166_press_and_hold_config },
    { "test_cap1166_proximity_detection_enable",
        test_cap1166_proximity_detection_enable },
    { "test_cap1166_touch_pattern_enable",
        test_cap1166_touch_pattern_enable },
    { "test_cap1166_detect_touch",
        test_cap1166_detect_touch },
    { "test_cap1166_identify",
        test_cap1166_identify },
    { "test_cap1166_set_power_mode",
        test_cap1166_set_power_mode },
    { "test_cap1166_led_config",
        test_cap1166_led_config },
    { "test_cap1166_led_direct_set",
        test_cap1166_led_direct_set },
    CU_TEST_INFO_NULL,
};

static CU_SuiteInfo suites[] = {
    { "cap1166_unit_test", NULL, NULL, NULL, NULL, cap1166_unit_test},
    CU_SUITE_INFO_NULL,
};

/*******************************************************************************
 *****************************      CODE      **********************************
 ******************************************************************************/
/*
 * Test for cap1166_sensor_enable function
 */
static void test_cap1166_sensor_enable(void)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t sensor_input_en[6];

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  sensor_input_en[0] = CAP1166_SENSOR_INPUT_ENABLE;
  sensor_input_en[1] = CAP1166_SENSOR_INPUT_ENABLE;
  sensor_input_en[2] = CAP1166_SENSOR_INPUT_ENABLE;
  sensor_input_en[3] = CAP1166_SENSOR_INPUT_ENABLE;
  sensor_input_en[4] = CAP1166_SENSOR_INPUT_ENABLE;
  sensor_input_en[5] = CAP1166_SENSOR_INPUT_ENABLE;

  /* Null pointer */
  status = cap1166_sensor_enable(NULL,
                                 CAP1166_ACTIVE,
                                 sensor_input_en);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_sensor_enable(&my_cap1166_handle,
                                 CAP1166_ACTIVE,
                                 NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* sensor inputs enable in active mode */
  status = cap1166_sensor_enable(&my_cap1166_handle,
                                 CAP1166_ACTIVE,
                                 sensor_input_en);
  CU_ASSERT(status == SL_STATUS_OK);

  /* sensor inputs enable in standby mode */
  status = cap1166_sensor_enable(&my_cap1166_handle,
                                 CAP1166_STANDBY,
                                 sensor_input_en);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_sensor_enable(&my_cap1166_handle,
                                 CAP1166_ACTIVE,
                                 sensor_input_en);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_sensor_int_enable function
 */
static void test_cap1166_sensor_int_enable(void)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t sensor_input_int_en[6];

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  sensor_input_int_en[0] = CAP1166_SENSOR_INTERUPT_ENABLE;
  sensor_input_int_en[1] = CAP1166_SENSOR_INTERUPT_ENABLE;
  sensor_input_int_en[2] = CAP1166_SENSOR_INTERUPT_ENABLE;
  sensor_input_int_en[3] = CAP1166_SENSOR_INTERUPT_ENABLE;
  sensor_input_int_en[4] = CAP1166_SENSOR_INTERUPT_ENABLE;
  sensor_input_int_en[5] = CAP1166_SENSOR_INTERUPT_ENABLE;

  /* Null pointer */
  status = cap1166_sensor_int_enable(NULL,
                                     sensor_input_int_en);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_sensor_int_enable(&my_cap1166_handle,
                                     NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* sensor inputs interrupt enable */
  status = cap1166_sensor_int_enable(&my_cap1166_handle,
                                     sensor_input_int_en);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_sensor_int_enable(&my_cap1166_handle,
                                     sensor_input_int_en);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_sensor_repeat_rate_enable function
 */
static void test_cap1166_sensor_repeat_rate_enable(void)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t sensor_input_repeat_rate_en[6];

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  sensor_input_repeat_rate_en[0] = CAP1166_SENSOR_REPEAT_ENABLE;
  sensor_input_repeat_rate_en[1] = CAP1166_SENSOR_REPEAT_ENABLE;
  sensor_input_repeat_rate_en[2] = CAP1166_SENSOR_REPEAT_ENABLE;
  sensor_input_repeat_rate_en[3] = CAP1166_SENSOR_REPEAT_ENABLE;
  sensor_input_repeat_rate_en[4] = CAP1166_SENSOR_REPEAT_ENABLE;
  sensor_input_repeat_rate_en[5] = CAP1166_SENSOR_REPEAT_ENABLE;

  /* Null pointer */
  status = cap1166_sensor_repeat_rate_enable(NULL,
                                             sensor_input_repeat_rate_en);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_sensor_repeat_rate_enable(&my_cap1166_handle,
                                             NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* sensor inputs repeat rate enable */
  status = cap1166_sensor_repeat_rate_enable(&my_cap1166_handle,
                                             sensor_input_repeat_rate_en);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_sensor_repeat_rate_enable(&my_cap1166_handle,
                                             sensor_input_repeat_rate_en);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_sensor_threshold_config function
 */
static void test_cap1166_sensor_threshold_config(void)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t sensor_threshold_standby = CAP1166_STBY_THRESHOLD_64X;
  uint8_t sensor_threshold_active[6];

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  sensor_threshold_active[0] = CAP1166_SENS_IN_THRESHOLD_64X;
  sensor_threshold_active[1] = CAP1166_SENS_IN_THRESHOLD_64X;
  sensor_threshold_active[2] = CAP1166_SENS_IN_THRESHOLD_64X;
  sensor_threshold_active[3] = CAP1166_SENS_IN_THRESHOLD_64X;
  sensor_threshold_active[4] = CAP1166_SENS_IN_THRESHOLD_64X;
  sensor_threshold_active[5] = CAP1166_SENS_IN_THRESHOLD_64X;

  /* Null pointer */
  status = cap1166_sensor_threshold_config(NULL,
                                           CAP1166_ACTIVE,
                                           sensor_threshold_active);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_sensor_threshold_config(&my_cap1166_handle,
                                           CAP1166_ACTIVE,
                                           NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* sensor threshold in active mode */
  status = cap1166_sensor_threshold_config(&my_cap1166_handle,
                                           CAP1166_ACTIVE,
                                           sensor_threshold_active);
  CU_ASSERT(status == SL_STATUS_OK);

  /* sensor threshold in standby mode */
  status = cap1166_sensor_threshold_config(&my_cap1166_handle,
                                           CAP1166_STANDBY,
                                           &sensor_threshold_standby);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_sensor_threshold_config(&my_cap1166_handle,
                                           CAP1166_ACTIVE,
                                           sensor_threshold_active);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_sensor_recalib_config function
 */
static void test_cap1166_sensor_recalib_config(void)
{
  sl_status_t status = SL_STATUS_OK;
  recalib_cfg_t recalib_cfg;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* Null pointer */
  status = cap1166_sensor_recalib_config(NULL,
                                         &recalib_cfg);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_sensor_recalib_config(&my_cap1166_handle,
                                         NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* auto calibration */
  recalib_cfg.recalib_mode = CAP1166_AUTO_CALIB;
  recalib_cfg.rate_recalib = CAP1166_REC_SAMPLES_64_UPDT_TIME_64;

  status = cap1166_sensor_recalib_config(&my_cap1166_handle,
                                         &recalib_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* manual calibration */
  recalib_cfg.recalib_mode = CAP1166_MANUAL_CALIB;
  recalib_cfg.sensor_recalib[0] = CAP1166_SENSOR_RECALIB_ENABLE;
  recalib_cfg.sensor_recalib[1] = CAP1166_SENSOR_RECALIB_ENABLE;
  recalib_cfg.sensor_recalib[2] = CAP1166_SENSOR_RECALIB_ENABLE;
  recalib_cfg.sensor_recalib[3] = CAP1166_SENSOR_RECALIB_ENABLE;
  recalib_cfg.sensor_recalib[4] = CAP1166_SENSOR_RECALIB_ENABLE;
  recalib_cfg.sensor_recalib[5] = CAP1166_SENSOR_RECALIB_ENABLE;

  status = cap1166_sensor_recalib_config(&my_cap1166_handle,
                                         &recalib_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* delay calibration */
  recalib_cfg.recalib_mode = CAP1166_DELAY_CALIB;
  recalib_cfg.max_during_en = CAP1166_MAX_DUR_RECALIB_EN;
  recalib_cfg.delay_recalib = CAP1166_5600MILISEC_BEFORE_RECALIB;

  status = cap1166_sensor_recalib_config(&my_cap1166_handle,
                                         &recalib_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* negative delta count calibration */
  recalib_cfg.recalib_mode = CAP1166_NEG_DELTA_CALIB;
  recalib_cfg.neg_del_cnt = CAP1166_CONS_NEG_DELTA_CNT_16;

  status = cap1166_sensor_recalib_config(&my_cap1166_handle,
                                         &recalib_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_sensor_recalib_config(&my_cap1166_handle,
                                         &recalib_cfg);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_sensor_noise_config function
 */
static void test_cap1166_sensor_noise_config(void)
{
  sl_status_t status = SL_STATUS_OK;
  sensor_noise_cfg_t  sensor_noise_cfg;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* Null pointer */
  status = cap1166_sensor_noise_config(NULL,
                                       &sensor_noise_cfg);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_sensor_noise_config(&my_cap1166_handle,
                                       NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* configuration noise */
  sensor_noise_cfg.RF_noise_filter = CAP1166_RF_NOISE_FILTER_EN;
  sensor_noise_cfg.analog_noise_filter = CAP1166_AN_NOISE_FILTER_EN;
  sensor_noise_cfg.digital_noise_threshold = CAP1166_DIG_NOISE_THRESHOLD_EN;
  sensor_noise_cfg.noise_threshold = CAP1166_SENS_IN_THRESHOLD_50_PERCENT;
  sensor_noise_cfg.show_low_frequency_noise = CAP1166_SHOW_LOW_FREQ_NOISE;

  status = cap1166_sensor_noise_config(&my_cap1166_handle,
                                       &sensor_noise_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_sensor_noise_config(&my_cap1166_handle,
                                       &sensor_noise_cfg);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_sensor_sensitivity_config function
 */
static void test_cap1166_sensor_sensitivity_config(void)
{
  sl_status_t status = SL_STATUS_OK;
  sensitivity_control_cfg_t sensitivity_control_cfg;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* Null pointer */
  status = cap1166_sensor_sensitivity_config(NULL,
                                             CAP1166_ACTIVE,
                                             &sensitivity_control_cfg);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_sensor_sensitivity_config(&my_cap1166_handle,
                                             CAP1166_ACTIVE,
                                             NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* configure sensitivity in active mode */
  sensitivity_control_cfg.base_shift = CAP1166_SENSITIVITY_MULTIPLIER_32X;
  sensitivity_control_cfg.sens_active = CAP1166_DATA_SCALING_FACTOR_256X;

  status = cap1166_sensor_sensitivity_config(&my_cap1166_handle,
                                             CAP1166_ACTIVE,
                                             &sensitivity_control_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* configure sensitivity in standby mode */
  sensitivity_control_cfg.sens_stby = CAP1166_STBY_SENSE_MULTIPLIER_32X;

  status = cap1166_sensor_sensitivity_config(&my_cap1166_handle,
                                             CAP1166_STANDBY,
                                             &sensitivity_control_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_sensor_sensitivity_config(&my_cap1166_handle,
                                             CAP1166_ACTIVE,
                                             &sensitivity_control_cfg);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_sensor_sensing_cycle_config function
 */
static void test_cap1166_sensor_sensing_cycle_config(void)
{
  sl_status_t status = SL_STATUS_OK;
  sensing_cycle_cfg_t sensing_cycle_cfg;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* Null pointer */
  status = cap1166_sensor_sensing_cycle_config(NULL,
                                               CAP1166_ACTIVE,
                                               &sensing_cycle_cfg);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_sensor_sensing_cycle_config(&my_cap1166_handle,
                                               CAP1166_ACTIVE,
                                               NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* configure sensing cycle in active mode */
  sensing_cycle_cfg.number_samples_per_cycle = CAP1166_8_SAMPLES;
  sensing_cycle_cfg.overall_cycle_time = CAP1166_CYCLE_TIME_70MILISEC;
  sensing_cycle_cfg.sample_time = CAP1166_SAMPLE_TIME_1280MICROSEC;

  status = cap1166_sensor_sensing_cycle_config(&my_cap1166_handle,
                                               CAP1166_ACTIVE,
                                               &sensing_cycle_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* configure sensing cycle in standby mode */
  sensing_cycle_cfg.number_samples_per_cycle = CAP1166_STBY_AVERAGE_64;
  sensing_cycle_cfg.overall_cycle_time = CAP1166_STBY_CYCLE_TIME_35MS;
  sensing_cycle_cfg.sample_time = CAP1166_STBY_SAMPLE_TIME_320US;

  status = cap1166_sensor_sensing_cycle_config(&my_cap1166_handle,
                                               CAP1166_STANDBY,
                                               &sensing_cycle_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_sensor_sensing_cycle_config(&my_cap1166_handle,
                                               CAP1166_ACTIVE,
                                               &sensing_cycle_cfg);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_int_pin_config function
 */
static void test_cap1166_int_pin_config(void)
{
  sl_status_t status = SL_STATUS_OK;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* Null pointer */
  status = cap1166_int_pin_config(NULL,
                                  CAP1166_ALERT_ACTIVE_HIGH);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* Configures state of interrupt pin (ALERT# pin) */
  status = cap1166_int_pin_config(&my_cap1166_handle,
                                  CAP1166_ALERT_ACTIVE_HIGH);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_int_pin_config(&my_cap1166_handle,
                                  CAP1166_ALERT_ACTIVE_HIGH);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_multiple_touch_config function
 */
static void test_cap1166_multiple_touch_config(void)
{
  sl_status_t status = SL_STATUS_OK;
  multi_touch_cfg_t multi_touch_cfg;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* Null pointer */
  status = cap1166_multiple_touch_config(NULL,
                                         &multi_touch_cfg);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_multiple_touch_config(&my_cap1166_handle,
                                         NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* configure multiple touch */
  multi_touch_cfg.multi_block_en = CAP1166_MULTIPLE_BLOCK_EN;
  multi_touch_cfg.number_of_touch = CAP1166_SIMUL_TOUCH_NUM_3;

  status = cap1166_multiple_touch_config(&my_cap1166_handle,
                                         &multi_touch_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_multiple_touch_config(&my_cap1166_handle,
                                         &multi_touch_cfg);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_press_and_hold_config function
 */
static void test_cap1166_press_and_hold_config(void)
{
  sl_status_t status = SL_STATUS_OK;
  press_and_hold_cfg_t press_and_hold_cfg;
  uint8_t sensor_input_repeat_rate[6];

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  sensor_input_repeat_rate[0] = CAP1166_SENSOR_REPEAT_ENABLE;
  sensor_input_repeat_rate[1] = CAP1166_SENSOR_REPEAT_ENABLE;
  sensor_input_repeat_rate[2] = CAP1166_SENSOR_REPEAT_ENABLE;
  sensor_input_repeat_rate[3] = CAP1166_SENSOR_REPEAT_ENABLE;
  sensor_input_repeat_rate[4] = CAP1166_SENSOR_REPEAT_ENABLE;
  sensor_input_repeat_rate[5] = CAP1166_SENSOR_REPEAT_ENABLE;

  /* Null pointer */
  status = cap1166_press_and_hold_config(NULL,
                                         &press_and_hold_cfg,
                                         sensor_input_repeat_rate);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_press_and_hold_config(&my_cap1166_handle,
                                         NULL,
                                         sensor_input_repeat_rate);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_press_and_hold_config(&my_cap1166_handle,
                                         &press_and_hold_cfg,
                                         NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* configure press and hold detection */
  press_and_hold_cfg.hold_time = CAP1166_PRESS_AND_HOLD_EVENT_AFTER_280MILISEC;
  press_and_hold_cfg.release_detection_en = CAP1166_DETECT_RELEASE_EN;
  press_and_hold_cfg.set_repeat_rate = CAP1166_INTERR_REPEAT_RATE_175MILISEC;

  status = cap1166_press_and_hold_config(&my_cap1166_handle,
                                         &press_and_hold_cfg,
                                         sensor_input_repeat_rate);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_press_and_hold_config(&my_cap1166_handle,
                                         &press_and_hold_cfg,
                                         sensor_input_repeat_rate);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_init function
 */
static void test_cap1166_init(void)
{
  sl_status_t status = SL_STATUS_OK;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* Null pointer */
  status = cap1166_init(NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* initialize cap1166 */
  status = cap1166_init(&my_cap1166_handle);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_init(&my_cap1166_handle);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_config function
 */
static void test_cap1166_config(void)
{
  sl_status_t status = SL_STATUS_OK;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* configure cap1166 */
  status = cap1166_config(&my_cap1166_handle,
                          &my_cap1166_config);
  CU_ASSERT( status == SL_STATUS_OK);

  /* Null pointer */
  status = cap1166_config(NULL,
                          &my_cap1166_config);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_config(&my_cap1166_handle,
                          NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_config(&my_cap1166_handle,
                          &my_cap1166_config);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_proximity_detection_enable function
 */
static void test_cap1166_proximity_detection_enable(void)
{
  sl_status_t status = SL_STATUS_OK;
  proximity_cfg_t proximity_cfg;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* Null pointer */
  status = cap1166_proximity_detection_enable(NULL,
                                              &proximity_cfg);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_proximity_detection_enable(&my_cap1166_handle,
                                              NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* configure for proximity detection */
  proximity_cfg.sum_avr_mode =
      CAP1166_SUMMATION_BASED;
  proximity_cfg.standby_sensing_cycle.number_samples_per_cycle =
      CAP1166_STBY_AVERAGE_64;
  proximity_cfg.standby_sensing_cycle.overall_cycle_time =
      CAP1166_STBY_CYCLE_TIME_35MS;
  proximity_cfg.standby_sensing_cycle.sample_time =
      CAP1166_STBY_SAMPLE_TIME_320US;

  status = cap1166_proximity_detection_enable(&my_cap1166_handle,
                                              &proximity_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_proximity_detection_enable(&my_cap1166_handle,
                                              &proximity_cfg);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_touch_pattern_enable function
 */
static void test_cap1166_touch_pattern_enable(void)
{
  sl_status_t status = SL_STATUS_OK;
  pattern_cfg_t pattern_cfg;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* Null pointer */
  status = cap1166_touch_pattern_enable(NULL,
                                        &pattern_cfg);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_touch_pattern_enable(&my_cap1166_handle,
                                        NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* configure multiple touch pattern */
  pattern_cfg.MTP_alert = CAP1166_MLTP_EVENT_ALERT_EN;
  pattern_cfg.MTP_mode = CAP1166_MODE_PATTERN_RECOGNITION;
  pattern_cfg.threshold_percent = CAP1166_MULTIPLE_THRESHOLD_100_PERCENTS;

  pattern_cfg.MTP_sensor_inputs[0] = CAP1166_SENSOR_PATTERN_ENABLE;
  pattern_cfg.MTP_sensor_inputs[1] = CAP1166_SENSOR_PATTERN_ENABLE;
  pattern_cfg.MTP_sensor_inputs[2] = CAP1166_SENSOR_PATTERN_DISABLE;
  pattern_cfg.MTP_sensor_inputs[3] = CAP1166_SENSOR_PATTERN_DISABLE;
  pattern_cfg.MTP_sensor_inputs[4] = CAP1166_SENSOR_PATTERN_DISABLE;
  pattern_cfg.MTP_sensor_inputs[5] = CAP1166_SENSOR_PATTERN_DISABLE;

  status = cap1166_touch_pattern_enable(&my_cap1166_handle,
                                        &pattern_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_touch_pattern_enable(&my_cap1166_handle,
                                        &pattern_cfg);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_detect_touch function
 */
static void test_cap1166_detect_touch(void)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t in_sens[6];

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* Null pointer */
  status = cap1166_detect_touch(NULL,
                                in_sens);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_detect_touch(&my_cap1166_handle,
                                NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* normal case, read success but all button are released */
  status = cap1166_detect_touch(&my_cap1166_handle,
                                in_sens);

  CU_ASSERT(status == SL_STATUS_OK);
  CU_ASSERT(in_sens[0] == 0);
  CU_ASSERT(in_sens[1] == 0);
  CU_ASSERT(in_sens[2] == 0);
  CU_ASSERT(in_sens[3] == 0);
  CU_ASSERT(in_sens[4] == 0);
  CU_ASSERT(in_sens[5] == 0);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_detect_touch(&my_cap1166_handle,
                                in_sens);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_identify function
 */
static void test_cap1166_identify(void)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t partId;
  uint8_t partRev;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* NULL pointer */
  status = cap1166_identify(NULL,
                            &partId,
                            &partRev);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_identify(&my_cap1166_handle,
                            NULL,
                            &partRev);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_identify(&my_cap1166_handle,
                            &partId,
                            NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* normal cases */
  status = cap1166_identify(&my_cap1166_handle,
                            &partId,
                            &partRev);
  CU_ASSERT(status == SL_STATUS_OK);
  CU_ASSERT(partId == CAP1166_CHIP_ID);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_identify(&my_cap1166_handle,
                            &partId,
                            &partRev);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_set_power_mode function
 */
static void test_cap1166_set_power_mode(void)
{
  sl_status_t status = SL_STATUS_OK;
  sensor_inputs_cfg_t sensor_cfg;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* Null pointer */
  status = cap1166_set_power_mode(NULL,
                                  CAP1166_ACTIVE,
                                  CAP1166_GAIN_2,
                                  &sensor_cfg);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_set_power_mode(&my_cap1166_handle,
                                  CAP1166_ACTIVE,
                                  CAP1166_GAIN_2,
                                  NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* invalid analog gain */
  status = cap1166_set_power_mode(&my_cap1166_handle,
                                  CAP1166_ACTIVE,
                                  1,
                                  &sensor_cfg);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = cap1166_set_power_mode(&my_cap1166_handle,
                                  CAP1166_ACTIVE,
                                  3,
                                  &sensor_cfg);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  status = cap1166_set_power_mode(&my_cap1166_handle,
                                  CAP1166_ACTIVE,
                                  9,
                                  &sensor_cfg);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  /* normal cases */
  sensor_cfg.sensor_en[0] = CAP1166_SENSOR_INPUT_ENABLE;
  sensor_cfg.sensor_en[1] = CAP1166_SENSOR_INPUT_ENABLE;
  sensor_cfg.sensor_en[2] = CAP1166_SENSOR_INPUT_ENABLE;
  sensor_cfg.sensor_en[3] = CAP1166_SENSOR_INPUT_DISABLE;
  sensor_cfg.sensor_en[4] = CAP1166_SENSOR_INPUT_DISABLE;
  sensor_cfg.sensor_en[5] = CAP1166_SENSOR_INPUT_DISABLE;

  sensor_cfg.sensor_interrupt_en[0] = CAP1166_SENSOR_INTERUPT_ENABLE;
  sensor_cfg.sensor_interrupt_en[1] = CAP1166_SENSOR_INTERUPT_ENABLE;
  sensor_cfg.sensor_interrupt_en[2] = CAP1166_SENSOR_INTERUPT_ENABLE;
  sensor_cfg.sensor_interrupt_en[3] = CAP1166_SENSOR_INTERUPT_DISABLE;
  sensor_cfg.sensor_interrupt_en[4] = CAP1166_SENSOR_INTERUPT_DISABLE;
  sensor_cfg.sensor_interrupt_en[5] = CAP1166_SENSOR_INTERUPT_DISABLE;

  /* set active mode */
  status = cap1166_set_power_mode(&my_cap1166_handle,
                                  CAP1166_ACTIVE,
                                  CAP1166_GAIN_1,
                                  &sensor_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  status = cap1166_set_power_mode(&my_cap1166_handle,
                                  CAP1166_ACTIVE,
                                  CAP1166_GAIN_2,
                                  &sensor_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  status = cap1166_set_power_mode(&my_cap1166_handle,
                                  CAP1166_ACTIVE,
                                  CAP1166_GAIN_8,
                                  &sensor_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* set standby mode */
  status = cap1166_set_power_mode(&my_cap1166_handle,
                                  CAP1166_STANDBY,
                                  CAP1166_GAIN_1,
                                  &sensor_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  status = cap1166_set_power_mode(&my_cap1166_handle,
                                  CAP1166_STANDBY,
                                  CAP1166_GAIN_2,
                                  &sensor_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  status = cap1166_set_power_mode(&my_cap1166_handle,
                                  CAP1166_STANDBY,
                                  CAP1166_GAIN_8,
                                  &sensor_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* set deep sleep mode */
  status = cap1166_set_power_mode(&my_cap1166_handle,
                                  CAP1166_DEEP_SLEEP,
                                  CAP1166_GAIN_1,
                                  &sensor_cfg);
  CU_ASSERT(status == SL_STATUS_OK);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_set_power_mode(&my_cap1166_handle,
                                  CAP1166_ACTIVE,
                                  CAP1166_GAIN_2,
                                  &sensor_cfg);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_check_interrupt_reason function
 */
static void test_cap1166_check_interrupt_reason(void)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t interrupt_reason;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* Null pointer */
  status = cap1166_check_interrupt_reason(NULL,
                                          &interrupt_reason);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_check_interrupt_reason(&my_cap1166_handle,
                                          NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* normal case, there is no interrupt, function return SL_STATUS_FAIL */
  status = cap1166_check_interrupt_reason(&my_cap1166_handle,
                                          &interrupt_reason);
  CU_ASSERT(status == SL_STATUS_FAIL);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_check_interrupt_reason(&my_cap1166_handle,
                                          &interrupt_reason);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_reset function
 */
static void test_cap1166_reset(void)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t interrupt_reason;

  /* Normal case */
  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  my_cap1166_handle.sensor_rst_port = gpioPortC;
  my_cap1166_handle.sensor_rst_pin = 6;

  status = cap1166_reset(&my_cap1166_handle);
  CU_ASSERT(status == SL_STATUS_OK);

  cap1166_check_interrupt_reason(&my_cap1166_handle,
                                 &interrupt_reason);
  CU_ASSERT(interrupt_reason & CAP1166_RESET_INT_MASK);

  /* wrong reset pin will cause SL_STATUS_TIMEOUT */
  my_cap1166_handle.sensor_rst_port = gpioPortB;
  my_cap1166_handle.sensor_rst_pin = 4;

//  GPIO_PinModeSet(my_cap1166_handle.sensor_rst_port,
//                  my_cap1166_handle.sensor_rst_pin,
//                  gpioModePushPull,
//                  0);

  status = cap1166_reset(&my_cap1166_handle);
  CU_ASSERT(status == SL_STATUS_TIMEOUT);

  /* SPI transfer error */
  my_cap1166_handle.sensor_rst_port = gpioPortC;
  my_cap1166_handle.sensor_rst_pin = 6;
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_reset(&my_cap1166_handle);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*
 * Test for cap1166_led_config function
 */
static void test_cap1166_led_config(void)
{
  sl_status_t status = SL_STATUS_OK;
  led_cfg_t led_cfg = my_cap1166_config.led_cfg;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* Null pointer */
  status = cap1166_led_config(NULL,
                              &led_cfg);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  status = cap1166_led_config(&my_cap1166_handle,
                              NULL);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* normal case */
  status = cap1166_led_config(&my_cap1166_handle,
                              &led_cfg);
  CU_ASSERT(status == SL_STATUS_OK);
}

/*
 * Test for cap1166_led_direct_set function
 */
static void test_cap1166_led_direct_set(void)
{
  sl_status_t status = SL_STATUS_OK;

  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* Null pointer */
  status = cap1166_led_direct_set(NULL,
                                  0,
                                  CAP1166_LED_TURN_ON);
  CU_ASSERT(status == SL_STATUS_NULL_POINTER);

  /* normal cases */
  status = cap1166_led_direct_set(&my_cap1166_handle,
                                  0,
                                  CAP1166_LED_TURN_ON);
  CU_ASSERT(status == SL_STATUS_OK);

  status = cap1166_led_direct_set(&my_cap1166_handle,
                                  0,
                                  CAP1166_LED_TURN_OFF);
  CU_ASSERT(status == SL_STATUS_OK);

  status = cap1166_led_direct_set(&my_cap1166_handle,
                                  2,
                                  CAP1166_LED_TURN_ON);
  CU_ASSERT(status == SL_STATUS_OK);

  status = cap1166_led_direct_set(&my_cap1166_handle,
                                  2,
                                  CAP1166_LED_TURN_OFF);
  CU_ASSERT(status == SL_STATUS_OK);

  status = cap1166_led_direct_set(&my_cap1166_handle,
                                  5,
                                  CAP1166_LED_TURN_ON);
  CU_ASSERT(status == SL_STATUS_OK);

  status = cap1166_led_direct_set(&my_cap1166_handle,
                                  5,
                                  CAP1166_LED_TURN_OFF);
  CU_ASSERT(status == SL_STATUS_OK);

  /* invalid parameter case */
  status = cap1166_led_direct_set(&my_cap1166_handle,
                                  6,
                                  CAP1166_LED_TURN_ON);
  CU_ASSERT(status == SL_STATUS_INVALID_PARAMETER);

  /* SPI transfer error */
  my_cap1166_handle.spidrv_handle = NULL;

  status = cap1166_led_direct_set(&my_cap1166_handle,
                                  0,
                                  CAP1166_LED_TURN_ON);
  CU_ASSERT(status == SL_STATUS_TRANSMIT);
}

/*******************************************************************************
 *****************************      API       **********************************
 ******************************************************************************/
void ut_cap1166_run_test(void)
{
  /* Init systick for clock() function */
  time_retarget_init();

  uint8_t interrupt_reason;
  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* reset pin */
  my_cap1166_handle.sensor_rst_port = gpioPortC;
  my_cap1166_handle.sensor_rst_pin = 6;

  /* clear status register */
  cap1166_check_interrupt_reason(&my_cap1166_handle,
                                 &interrupt_reason);

  printf("Start unit test.\n");
  printf("\nDo not touch the sensor during test!\n");

  /* basic test */
  CU_BasicRunMode mode = CU_BRM_VERBOSE;
  CU_ErrorAction error_action = CUEA_IGNORE;

  setvbuf(stdout, NULL, _IONBF, 0);
  error_action = CUEA_FAIL;

  /*
   * CU_BRM_VERBOSE
   * CU_BRM_NORMAL
   */
  mode = CU_BRM_VERBOSE;

  if(CU_initialize_registry()){
    printf("\nInitialization of Test Registry failed.");
  }
  else{
    /* add test here */
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());

    /* Register suites. */
    if(CU_register_suites(suites) != CUE_SUCCESS){
        fprintf(stderr, "suite registration failed - %s\n", CU_get_error_msg());
        exit(EXIT_FAILURE);
    }
    CU_basic_set_mode(mode);
    CU_set_error_action(error_action);

    printf("\nTests completed with return value %d.\n", CU_basic_run_tests());
    CU_cleanup_registry();
  }
}

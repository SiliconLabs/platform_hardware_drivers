/**************************************************************************//**
 * @file app.c
 * @brief Application interface provided to main().
 * @version 1.0.0
*******************************************************************************
* # License
* <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
* # Experimental Quality
* This code has not been formally tested and is provided as-is. It is not
* suitable for production environments. In addition, this code will not be
* maintained and there may be no bug maintenance planned for these resources.
* Silicon Labs may update projects from time to time.
******************************************************************************/
#include "bma400.h"
#include "sl_status.h"
#include "printf.h"
#include "sl_simple_button_instances.h"

#define TEST_ACCELEROMETER      0
#define TEST_TAP_DETECTION      1
#define TEST_ACTIVITY_CHANGE    2
#define TEST_READ_FIFO_FULL     3

#define TEST_MODE TEST_ACCELEROMETER

/* Earth's gravity in m/s^2 */
#define GRAVITY_EARTH     (9.80665f)
/* 39.0625us per tick */
#define SENSOR_TICK_TO_S  (0.0000390625f)
/* Total number of frames */
#define N_FRAMES_FULL   1024
/* Add extra bytes to get complete fifo data */
#define FIFO_SIZE_FULL  (N_FRAMES_FULL + BMA400_FIFO_BYTES_OVERREAD)

static uint8_t app_btn0_pressed = false;

#if (TEST_MODE == TEST_ACCELEROMETER)
static sl_status_t bma400_conf_accelerometer(void);
static sl_status_t test_accelerometer(void);
static float lsb_to_ms2(int16_t accel_data, uint8_t g_range, uint8_t bit_width);

#elif (TEST_MODE == TEST_TAP_DETECTION)
static sl_status_t bma400_conf_tap_detection(void);
static sl_status_t test_tap_detection(void);

#elif (TEST_MODE == TEST_READ_FIFO_FULL)
bma400_fifo_data_t fifo_frame;
bma400_sensor_data_t accel_data[N_FRAMES_FULL] = { { 0 } };
static sl_status_t bma400_conf_read_fifo_full(void);
static sl_status_t test_read_fifo_full(void);
static float lsb_to_ms2(int16_t accel_data, uint8_t g_range, uint8_t bit_width);

#elif (TEST_MODE == TEST_ACTIVITY_CHANGE)
static sl_status_t bma400_conf_activity_change(void);
static sl_status_t test_activity_change(void);
#endif


/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  sl_status_t ret;

#if (TEST_MODE == TEST_ACCELEROMETER)
  ret = bma400_conf_accelerometer();
  if (ret != SL_STATUS_OK) {
    printf("Warning! Fail to configure accel 5 click, reason: 0x%04x\r\n", ret);
    while(1); // If configuration is not ok then forever loop here
  }
  else {
    printf("Configure accel 5 click successful.\r\n");
  }
  printf("Get accel data - BMA400_DATA_SENSOR_TIME\n");
  printf("Accel Gravity data in m/s^2\n");
  printf("\n --------- Press btn0 to start --------\n");
#elif (TEST_MODE == TEST_TAP_DETECTION)
  ret = bma400_conf_tap_detection();
  if (ret != SL_STATUS_OK) {
    printf("Warning! Fail to configure accel 5 click, reason: 0x%04x\r\n", ret);
    while(1); // If configuration is not ok then forever loop here
  }
  else {
    printf("Configure accel 5 click successful.\r\n");
  }
  printf("Single and Double Tap interrupts enabled\n");
  printf("Perform single or double tap on the board\n");
#elif (TEST_MODE == TEST_READ_FIFO_FULL)
  ret = bma400_conf_read_fifo_full();
  if (ret != SL_STATUS_OK) {
    printf("Warning! Fail to configure accel 5 click, reason: 0x%04x\r\n", ret);
    while(1); // If configuration is not ok then forever loop here
  }
  else {
    printf("Configure accel 5 click successful.\r\n");
  }

  printf("Read FIFO Full interrupt XYZ data\n");
  ret = test_read_fifo_full();
  if (ret != SL_STATUS_OK) {
    printf("Warning! Fail to test, reason: 0x%04x\r\n", ret);
    while(1); // If testing is not ok then forever loop here
  }
  printf("\n---------Testing finished------------- \n");
#elif (TEST_MODE == TEST_ACTIVITY_CHANGE)
  ret = bma400_conf_activity_change();
  if (ret != SL_STATUS_OK) {
    printf("Warning! Fail to configure accel 5 click, reason: 0x%04x\r\n", ret);
    while(1); // If configuration is not ok then forever loop here
  }
  else {
    printf("Configure accel 5 click successful.\r\n");
  }
  printf("Functionality test for Activity change interrupt\n");
  printf("Show activity on x y z axes of the board\n");
#endif
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  sl_status_t ret = SL_STATUS_OK;

  #if (TEST_MODE == TEST_ACCELEROMETER)
  if(app_btn0_pressed == true) {
    ret = test_accelerometer();
  }
  #elif (TEST_MODE == TEST_TAP_DETECTION)
    ret = test_tap_detection();
  #elif (TEST_MODE == TEST_ACTIVITY_CHANGE)
    test_activity_change();
  #endif
    if (ret != SL_STATUS_OK) {
      printf("Warning! Fail to test, reason: 0x%04x\r\n", ret);
      while(1); // If testing is not ok then forever loop here
    }
}

/**************************************************************************//**
 * Simple Button
 * Button state changed callback
 * @param[in] handle Button event handle
 *****************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  // Button pressed.
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (&sl_button_btn0 == handle) {
      if (app_btn0_pressed == true) {
        app_btn0_pressed = false;
      }
      else {
        app_btn0_pressed = true;
      }
    }
  }
}

#if (TEST_MODE == TEST_ACCELEROMETER)
/***************************************************************************//**
 * Function used in mode test accelerometer
 ******************************************************************************/

static sl_status_t bma400_conf_accelerometer(void)
{
  bma400_sensor_conf_t conf;
  bma400_int_enable_t int_en;
  sl_status_t ret;

  ret = bma400_init();
  if (ret != SL_STATUS_OK) {
    return ret;
  }
  /* Select the type of configuration to be modified */
  conf.type = BMA400_ACCEL;

  /* Get the accelerometer configurations which are set in the sensor */
  ret = bma400_get_sensor_conf(&conf, 1);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  /* Modify the desired configurations as per macros
   * available in bma400.h file */
  conf.param.accel.odr = BMA400_ODR_12_5HZ;
  conf.param.accel.range = BMA400_RANGE_2G;
  conf.param.accel.data_src = BMA400_DATA_SRC_ACCEL_FILT_1;

  /* Set the desired configurations to the sensor */
  ret = bma400_set_sensor_conf(&conf, 1);
  if (ret != SL_STATUS_OK) {
    return ret;
  }
  ret = bma400_set_power_mode(BMA400_MODE_NORMAL);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  int_en.type = BMA400_DRDY_INT_EN;
  int_en.conf = BMA400_ENABLE;
  ret = bma400_enable_interrupt(&int_en, 1);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  return SL_STATUS_OK;
}

static sl_status_t test_accelerometer(void)
{
  sl_status_t ret;
  bma400_sensor_data_t data;
  uint16_t int_status = 0;
  float t, x, y, z;

  ret = bma400_get_interrupt_status(&int_status);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  if (int_status & BMA400_ASSERTED_DRDY_INT) {
    ret = bma400_get_accel_data(BMA400_DATA_SENSOR_TIME, &data);
    if (ret != SL_STATUS_OK) {
      return ret;
    }

    /* 12-bit accelerometer at range 2G */
    x = lsb_to_ms2(data.x, 2, 12);
    y = lsb_to_ms2(data.y, 2, 12);
    z = lsb_to_ms2(data.z, 2, 12);
    t = (float)data.sensortime * SENSOR_TICK_TO_S;

    printf("Gravity-x = %.2f, Gravity-y = %.2f, Gravity-z =  %.2f, t(s) = %.4f\r\n",
           x, y, z, t);
  }

  return SL_STATUS_OK;
}

#elif (TEST_MODE == TEST_TAP_DETECTION)
/***************************************************************************//**
 * Functions used in mode test tap detection
 ******************************************************************************/
static sl_status_t bma400_conf_tap_detection(void)
{
  sl_status_t ret;
  bma400_sensor_conf_t conf[2] = { { 0 } };
  bma400_int_enable_t int_en[2];

  ret = bma400_init();
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  conf[0].type = BMA400_ACCEL;
  conf[1].type = BMA400_TAP_INT;

  ret = bma400_get_sensor_conf(conf, 2);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  conf[0].param.accel.odr = BMA400_ODR_200HZ;
  conf[0].param.accel.range = BMA400_RANGE_16G;
  conf[0].param.accel.data_src = BMA400_DATA_SRC_ACCEL_FILT_1;
  conf[0].param.accel.filt1_bw = BMA400_ACCEL_FILT1_BW_1;

  conf[1].param.tap.int_chan = BMA400_INT_CHANNEL_1;
  conf[1].param.tap.axes_sel = BMA400_X_AXIS_EN_TAP
                             | BMA400_Y_AXIS_EN_TAP
                             | BMA400_Z_AXIS_EN_TAP;
  conf[1].param.tap.sensitivity = BMA400_TAP_SENSITIVITY_0;
  conf[1].param.tap.tics_th = BMA400_TICS_TH_6_DATA_SAMPLES;
  conf[1].param.tap.quiet = BMA400_QUIET_60_DATA_SAMPLES;
  conf[1].param.tap.quiet_dt = BMA400_QUIET_DT_4_DATA_SAMPLES;

  ret = bma400_set_sensor_conf(conf, 2);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  ret = bma400_set_power_mode(BMA400_MODE_NORMAL);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  int_en[0].type = BMA400_SINGLE_TAP_INT_EN;
  int_en[0].conf = BMA400_ENABLE;
  int_en[1].type = BMA400_DOUBLE_TAP_INT_EN;
  int_en[1].conf = BMA400_ENABLE;
  ret = bma400_enable_interrupt(int_en, 2);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  return SL_STATUS_OK;
}

static sl_status_t test_tap_detection(void)
{
  sl_status_t ret;
  uint16_t int_status = 0;
  static int8_t count = 0;
  static int8_t dcount = 0;

  ret = bma400_get_interrupt_status(&int_status);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  if (int_status & BMA400_ASSERTED_S_TAP_INT) {
    printf("Single tap detected, count = %d\n", count);
    count++;
  }

  if (int_status & BMA400_ASSERTED_D_TAP_INT) {
    printf("Double tap detected, dcount = %d\n", dcount);
    dcount++;
  }

  return ret;
}

#elif (TEST_MODE == TEST_READ_FIFO_FULL)
/***************************************************************************//**
 * Functions used in mode test read fifo full
 ******************************************************************************/

static sl_status_t bma400_conf_read_fifo_full(void)
{
  sl_status_t ret;
  bma400_sensor_conf_t conf;
  bma400_device_conf_t fifo_conf;
  bma400_int_enable_t int_en;
  uint8_t fifo_buff[FIFO_SIZE_FULL] = { 0 };

  ret = bma400_init();
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  /* Select the type of configuration to be modified */
  conf.type = BMA400_ACCEL;
  /* Get the accelerometer configurations which are set in the sensor */
  ret = bma400_get_sensor_conf(&conf, 1);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  /* Modify the desired configurations as per macros
   * available in bma400.h file */
  conf.param.accel.odr = BMA400_ODR_100HZ;
  conf.param.accel.range = BMA400_RANGE_2G;
  conf.param.accel.data_src = BMA400_DATA_SRC_ACCEL_FILT_1;

  /* Set the desired configurations to the sensor */
  ret = bma400_set_sensor_conf(&conf, 1);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  fifo_conf.type = BMA400_FIFO_CONF;

  ret = bma400_get_device_conf(&fifo_conf, 1);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  fifo_conf.param.fifo_conf.conf_regs = BMA400_FIFO_X_EN
                                      | BMA400_FIFO_Y_EN
                                      | BMA400_FIFO_Z_EN;
  fifo_conf.param.fifo_conf.conf_status = BMA400_ENABLE;
  fifo_conf.param.fifo_conf.fifo_full_channel = BMA400_INT_CHANNEL_1;

  ret = bma400_set_device_conf(&fifo_conf, 1);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  ret = bma400_set_power_mode(BMA400_MODE_NORMAL);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  fifo_frame.data = fifo_buff;
  fifo_frame.length = FIFO_SIZE_FULL;

  int_en.type = BMA400_FIFO_FULL_INT_EN;
  int_en.conf = BMA400_ENABLE;

  ret = bma400_enable_interrupt(&int_en, 1);

  return ret;
}

static sl_status_t test_read_fifo_full(void)
{
  sl_status_t ret;
  float x, y, z;
  uint16_t int_status = 0;
  static uint8_t try = 1;
  uint16_t accel_frames_req = N_FRAMES_FULL;
  uint16_t idx;

  while (try <= 10) {
    ret = bma400_get_interrupt_status(&int_status);
    if (ret != SL_STATUS_OK) {
      printf("bma400_get_interrupt_status = %d\n", ret);
      return SL_STATUS_FAIL;
    }

    if (int_status & BMA400_ASSERTED_FIFO_FULL_INT) {
      printf("\n\nIteration : %d\n\n", try);
      printf("Requested FIFO length : %d\n", fifo_frame.length);

      ret = bma400_get_fifo_data(&fifo_frame);
      printf("bma400_get_fifo_data = %d\n", ret);

      printf("Available FIFO length : %d\n", fifo_frame.length);

      accel_frames_req = N_FRAMES_FULL;
      ret = bma400_extract_accel(&fifo_frame, accel_data, &accel_frames_req);
      printf("bma400_extract_accel = %d\n", ret);
      if (accel_frames_req) {
        printf("Extracted FIFO frames : %d\n", accel_frames_req);
        printf("Accel data in LSB units and Gravity data in m/s^2\n");
        for (idx = 0; idx < accel_frames_req; idx++) {
          /* 12-bit accelerometer at range 2G */
          printf("Accel[%d] X : %d raw LSB    Y : %d raw LSB    Z : %d raw LSB\n",
                 idx,
                 accel_data[idx].x,
                 accel_data[idx].y,
                 accel_data[idx].z);

          /* 12-bit accelerometer at range 2G */
          x = lsb_to_ms2(accel_data[idx].x, 2, 12);
          y = lsb_to_ms2(accel_data[idx].y, 2, 12);
          z = lsb_to_ms2(accel_data[idx].z, 2, 12);

          /* Print the data in m/s2. */
          printf("\t  Gravity-x = %4.2f, Gravity-y = %4.2f, Gravity-z = %4.2f\n", x, y, z);
        }
      }

      if (fifo_frame.conf_change) {
        printf("FIFO configuration change: 0x%X\n", fifo_frame.conf_change);

        if (fifo_frame.conf_change & BMA400_FIFO_CONF0_CHANGE) {
            printf("FIFO data source configuration changed\n");
        }
        if (fifo_frame.conf_change & BMA400_ACCEL_CONF0_CHANGE) {
            printf("Accel filt1_bw configuration changed\n");
        }
        if (fifo_frame.conf_change & BMA400_ACCEL_CONF1_CHANGE) {
            printf("Accel odr/osr/range configuration changed\n");
        }
      }

      try++;
    }
  }

  return SL_STATUS_OK;
}

#elif (TEST_MODE == TEST_ACTIVITY_CHANGE)
/***************************************************************************//**
 * Function used in mode test activity change
 ******************************************************************************/

static sl_status_t bma400_conf_activity_change(void)
{
  bma400_int_enable_t int_en;
  sl_status_t ret;
  bma400_sensor_conf_t accel_setting[2] = { { 0 } };

  ret = bma400_init();
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  accel_setting[0].type = BMA400_ACTIVITY_CHANGE_INT;
  accel_setting[1].type = BMA400_ACCEL;

  ret = bma400_get_sensor_conf(accel_setting, 2);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  accel_setting[0].param.act_ch.int_chan = BMA400_INT_CHANNEL_1;
  accel_setting[0].param.act_ch.axes_sel = BMA400_AXIS_XYZ_EN;
  accel_setting[0].param.act_ch.act_ch_ntps = BMA400_ACT_CH_SAMPLE_CNT_64;
  accel_setting[0].param.act_ch.data_source = BMA400_DATA_SRC_ACC_FILT1;
  accel_setting[0].param.act_ch.act_ch_thres = 10;

  accel_setting[1].param.accel.odr = BMA400_ODR_100HZ;
  accel_setting[1].param.accel.range = BMA400_RANGE_2G;
  accel_setting[1].param.accel.data_src = BMA400_DATA_SRC_ACCEL_FILT_1;

  /* Set the desired configurations to the sensor */
  ret = bma400_set_sensor_conf(accel_setting, 2);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  ret = bma400_set_power_mode(BMA400_MODE_NORMAL);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  int_en.type = BMA400_ACTIVITY_CHANGE_INT_EN;
  int_en.conf = BMA400_ENABLE;

  ret = bma400_enable_interrupt(&int_en, 1);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  return SL_STATUS_OK;
}

static sl_status_t test_activity_change(void)
{
  sl_status_t ret;
  uint16_t int_status = 0;
  bma400_sensor_data_t accel;

  ret = bma400_get_interrupt_status(&int_status);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  if (int_status & BMA400_ASSERTED_ACT_CH_X) {
    printf("Activity change interrupt asserted on X axis\n");

    ret = bma400_get_accel_data(BMA400_DATA_SENSOR_TIME, &accel);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
    printf("Accel Data :  X : %d    Y : %d    Z : %d    SENSOR_TIME : %d\n",
           accel.x,
           accel.y,
           accel.z,
           accel.sensortime);
  }

  if (int_status & BMA400_ASSERTED_ACT_CH_Y) {
    printf("Activity change interrupt asserted on Y axis\n");

    ret = bma400_get_accel_data(BMA400_DATA_SENSOR_TIME, &accel);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
    printf("Accel Data :  X : %d    Y : %d    Z : %d    SENSOR_TIME : %d\n",
           accel.x,
           accel.y,
           accel.z,
           accel.sensortime);
  }

  if (int_status & BMA400_ASSERTED_ACT_CH_Z) {
    printf("Activity change interrupt asserted on Z axis\n");

    ret = bma400_get_accel_data(BMA400_DATA_SENSOR_TIME, &accel);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
    printf("Accel Data :  X : %d    Y : %d    Z : %d    SENSOR_TIME : %d\n",
           accel.x,
           accel.y,
           accel.z,
           accel.sensortime);
  }

  return SL_STATUS_OK;
}

#endif

#if (TEST_MODE == TEST_ACCELEROMETER) || (TEST_MODE == TEST_READ_FIFO_FULL)
static float lsb_to_ms2(int16_t accel_data, uint8_t g_range, uint8_t bit_width)
{
  float accel_ms2;
  int16_t half_scale;

  half_scale = 1 << (bit_width - 1);
  accel_ms2 = (GRAVITY_EARTH * accel_data * g_range) / half_scale;

  return accel_ms2;
}

#endif

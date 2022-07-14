/***************************************************************************//**
 * @file app.c
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "app_assert.h"
#include "app_log.h"
#include "as7265x.h"
#include "sl_sleeptimer.h"

// Choose which type of test you want by uncommenting the #define

#define TEST_BASIC_READING
// #define TEST_BASIC_READING_WITH_LED
// #define TEST_READ_RAW_VALUE
// #define TEST_MAX_READ_RATE

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  sl_status_t status;
  uint8_t temperature = 0;
  uint8_t hardware_version_data = 0;
  as7265x_firmware_version_t firmware_version_data;

  status = as7265x_init();
  app_assert_status(status);
  app_log("\nInit as7265x sensor successfully\r\n");

  // Get sensor temperature.
  app_log("\n-------------Sensor temperature-------------\r\n");
  status = as7265x_get_temperature(AS72651_NIR, &temperature);
  app_assert_status(status);
  app_log("AS72651 Temperature = %d\r\n", temperature);

  status = as7265x_get_temperature(AS72652_VISIBLE, &temperature);
  app_assert_status(status);
  app_log("AS72652 Temperature = %d\r\n", temperature);

  status = as7265x_get_temperature(AS72653_UV, &temperature);
  app_assert_status(status);
  app_log("AS72653 Temperature = %d\r\n", temperature);

  // Get hardware and firmware version.
  app_log("\n---------Hardware and firmware version--------\r\n");
  status = as7265x_get_device_type(&hardware_version_data);
  app_assert_status(status);
  app_log("AMS Device Type: 0x%x\r\n", hardware_version_data);

  status = as7265x_get_hardware_version(&hardware_version_data);
  app_assert_status(status);
  app_log("AMS Hardware Version: 0x%x\r\n", hardware_version_data);

  status = as7265x_get_firmware_version(&firmware_version_data);
  app_assert_status(status);
  app_log("Major Firmware Version: 0x%x\r\n",
          firmware_version_data.major_firmware_version);
  app_log("Patch Firmware Version: 0x%x\r\n",
          firmware_version_data.patch_firmware_version);
  app_log("Build Firmware Version: 0x%x\r\n",
          firmware_version_data.build_firmware_version);

#if defined(TEST_BASIC_READING_WITH_LED) \
    || defined(TEST_READ_RAW_VALUE) \
    || defined(TEST_MAX_READ_RATE)
  status = as7265x_disable_indicator();
  app_assert_status(status);
#endif

#ifdef TEST_MAX_READ_RATE
  // Rather than toggle the LEDs with each measurement, turn on LEDs all the
  // time
  status = as7265x_enable_bulb(AS7265x_LED_WHITE);
  app_assert_status(status);
  status = as7265x_enable_bulb(AS7265x_LED_IR);
  app_assert_status(status);
  status = as7265x_enable_bulb(AS7265x_LED_UV);
  app_assert_status(status);

  // Integration time = 1 * 2.8(ms) = 2.8(ms)
  status = as7265x_set_integration_cycles(1);
  app_assert_status(status);
#endif


#if defined(TEST_BASIC_READING) \
    || defined(TEST_BASIC_READING_WITH_LED) \
    || defined(TEST_READ_RAW_VALUE) \
    || defined(TEST_MAX_READ_RATE)

  app_log("\n----Spectral data in counts/microwatt/cm2----\r\n");
  app_log(
    "|    A    |    B    |    C    |    D    |    E    |    F    |\
    G    |    H    |    R    |    I    |    S    |    J    |    T    |\
    U    |    V    |    W    |    K    |   L\r\n");

#endif
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  sl_status_t status;

#if defined(TEST_BASIC_READING) || defined(TEST_BASIC_READING_WITH_LED)
  as7265x_calibrated_data_t cal_value;
#ifdef TEST_BASIC_READING_WITH_LED
  status = as7265x_take_measurements_with_bulb();
#else
  status = as7265x_take_measurements();
#endif

  if (status == SL_STATUS_OK) {
    status = as7265x_get_all_calibrated_value(&cal_value);
    app_assert_status(status);

    app_log("|%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_A]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_B]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_C]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_D]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_E]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_F]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_G]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_H]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_R]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_I]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_S]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_J]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_T]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_U]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_V]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_W]);
    app_log("%9.3f|", cal_value.channel[AS7265x_COLOR_CHANNEL_K]);
    app_log("%9.3f\r\n", cal_value.channel[AS7265x_COLOR_CHANNEL_L]);
  }
#endif

#if defined(TEST_READ_RAW_VALUE) || defined(TEST_MAX_READ_RATE)

#ifdef TEST_MAX_READ_RATE
  uint32_t start_time = 0;
  uint32_t end_time = 0;

  start_time = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
#endif

  as7265x_color_data_t color_data;
  status = as7265x_take_measurements();

#ifdef TEST_MAX_READ_RATE
  end_time = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
  app_log("Data rate = %.2f\r\n", (float)1000 / (end_time - start_time));
#endif

  if (status == SL_STATUS_OK) {
      status = as7265x_get_all_color_channel(&color_data);
      app_assert_status(status);

      app_log("|%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_A]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_B]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_C]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_D]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_E]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_F]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_G]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_H]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_R]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_I]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_S]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_J]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_T]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_U]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_V]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_W]);
      app_log("%9d|", color_data.channel[AS7265x_COLOR_CHANNEL_K]);
      app_log("%9d\r\n", color_data.channel[AS7265x_COLOR_CHANNEL_L]);
  }
#endif
}

/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdint.h>
#include "app_log.h"
#include "app_assert.h"
#include "sl_i2cspm_instances.h"
#include "sl_udelay.h"
#include "sl_sleeptimer.h"
#include "shtc3.h"

#define TEST_BLOCKING                       // test for function blocking
// #define TEST_NON_BLOCKING                 // test for function non-blocking

static   shtc3_sensor_data_t   shtc3_data;  // Structure to hold SHTC3 data

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  uint16_t id;
  sl_status_t status;
  shtc3_core_version_t shtc3_core_version;

  get_version(&shtc3_core_version);
  app_log("Version of software: %d.%d.%d\n",
          shtc3_core_version.major,
          shtc3_core_version.minor,
          shtc3_core_version.build);

  status = shtc3_init(sl_i2cspm_qwiic);
  app_assert_status(status);
  app_log("Init successfully\n");

  // wake up command
  if (shtc3_wake() == SL_STATUS_OK) {
    app_log("SHTC3 wakeup\n");
  }
  // GetId command
  status = shtc3_get_id(&id);
  app_assert_status(status);
  app_log("SHTC3 sensor is detected with ID: 0x%X\n", id);

  status = shtc3_sleep();
  app_assert_status(status);
  app_log("SHTC3 sleep\n");
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
#ifdef TEST_BLOCKING
  if (shtc3_wake() == SL_STATUS_OK) {
    app_log("SHTC3 wakeup\n");
  }

  if (sl_shtc3_get_temp_rh(SHTC3_NORMAL_POWER_MODE,
                           SHTC3_RH_FIRST,
                           false,
                           true,
                           &shtc3_data) == SL_STATUS_OK) {
    app_log("Temp is: %0.2f Humidity is: %0.2f\n",
            shtc3_data.rh_t_data.calculated_temp,
            shtc3_data.rh_t_data.calculated_rh);
  }

  sl_sleeptimer_delay_millisecond(1500);

  if (sl_shtc3_get_temp(SHTC3_NORMAL_POWER_MODE,
                        false,
                        true,
                        &shtc3_data) == SL_STATUS_OK) {
    app_log("Temp is: %0.2f \n", shtc3_data.rh_t_data.calculated_temp);
  }

  sl_sleeptimer_delay_millisecond(1500);

  if (sl_shtc3_get_rh(SHTC3_NORMAL_POWER_MODE,
                      true,
                      true,
                      &shtc3_data) == SL_STATUS_OK) {
    app_log("humidity is: %0.2f \n", shtc3_data.rh_t_data.calculated_rh);
  }

  if (shtc3_sleep() == SL_STATUS_OK) {
    app_log("SHTC3 sleep\n");
  }

  sl_sleeptimer_delay_millisecond(1500);
#endif

#ifdef TEST_NON_BLOCKING
  if (shtc3_wake() == SL_STATUS_OK) {
    app_log("SHTC3 wakeup\n");
  }
  // none blocking
  while (sl_shtc3_get_temp_rh_non_blocking(SHTC3_NORMAL_POWER_MODE,
                                           SHTC3_RH_FIRST,
                                           true,
                                           &shtc3_data)
         != SL_STATUS_OK) {}
  app_log("Temp is: %0.2f Humidity is: %0.2f\n",
          shtc3_data.rh_t_data.calculated_temp,
          shtc3_data.rh_t_data.calculated_rh);

  if (shtc3_sleep() == SL_STATUS_OK) {
    app_log("SHTC3 sleep\n");
  }

  sl_sleeptimer_delay_millisecond(1500);
#endif
}

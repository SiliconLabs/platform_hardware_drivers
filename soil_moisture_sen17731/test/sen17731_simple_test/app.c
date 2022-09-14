/***************************************************************************//**
 * @file app.c
 * @brief Top level application functions
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
 * # Experimental Quality
 * This code has been minimally tested to ensure that it builds and is suitable
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
#include "sl_simple_timer.h"
#include "sl_i2cspm_instances.h"

#include "app_log.h"
#include "app_assert.h"

#include "sen17731.h"
#include "sen17731_config.h"

#define MOISTURE_THRESHOLD          70

// simple timer handle variable
static sl_simple_timer_t my_timer;

/**************************************************************************//**
 *  Simple timer callback function.
 *****************************************************************************/
static void timer_callback(sl_simple_timer_t *timer, void *data);

void app_init(void)
{
  sl_status_t sc;
  uint16_t addr[255];
  uint8_t num_dev;

  app_log("\r\t\tSEN-17731 Soil moisture sensor test program\n\n");

  app_log("\r\nScanning address of the sensor...\n");
  sc = sen17731_scan_address(addr, &num_dev);
  app_assert(sc == SL_STATUS_OK, "\rNo device connected\n");
  for (int i = 0; i < num_dev; i++) {
    app_log("\rDevice %d: address: 0x%x\n", i + 1, *addr + i);
  }
  sen17731_select_device(addr[0]);

  app_log("\r\nStart calibrating the sensor...\n");
  sen17731_set_dry_value(90);
  sen17731_set_wet_value(1023);
  app_log("\rCalibrating done...\n");

  sc = sl_simple_timer_start(&my_timer, 1000, timer_callback, NULL, true);

  if (sc != SL_STATUS_OK) {
    app_log("\r\n > Start periodic measuring soil moisture Fail\n");
  } else {
    app_log("\r\n > Start periodic measuring soil moisture...\n");
  }
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  /* This functin is empty because we use a simple timer to handle periodic
   *    action of the sensor */
}

static void timer_callback(sl_simple_timer_t *timer, void *data)
{
  uint8_t moisture;
  sl_status_t sc;

  (void)&timer;
  (void)&data;
  sc = sen17731_get_moisture(&moisture);

  if (sc != SL_STATUS_OK) {
    app_log("\r > Reading data failed\n");
  } else {
    app_log("\r > Soil moisture: %d%%\n", moisture);
    if (moisture > MOISTURE_THRESHOLD) {
      sen17731_led_on();
    } else {
      sen17731_led_off();
    }
  }
}

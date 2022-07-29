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

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
#include "app_log.h"
#include "sl_simple_timer.h"
#include "app_assert.h"
#include "vcnl4040.h"
#include "vcnl4040_config.h"

// simple timer handle variable
static sl_simple_timer_t my_timer;

/**************************************************************************//**
 *  Simple timer callback function.
 *****************************************************************************/
static void timer_callback(sl_simple_timer_t *timer, void *data);

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  sl_status_t sc;
  uint16_t id;

  app_log("\r======== VCNL4040 Proximity Sensor Driver ========\n");
  sc = vcnl4040_init(SL_VCNL4040_CONFIG_I2C);
  app_assert(sc == SL_STATUS_OK, "\rVCNL4040 Init fail...\n");
  app_log("\rVCNL4040 Init done...\n");

  vcnl4040_get_id(&id);
  app_log("\rDevice ID: 0x%4X\n", id);
  app_log("\r\t==================================\n");
//
  sc = sl_simple_timer_start(&my_timer, 1000, timer_callback, NULL, true);

  if (sc != SL_STATUS_OK) {
    app_log("\r > Start Periodic Measurement Fail\n");
  } else {
    app_log("\r > Start Periodic Measurement\n");
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
  sl_status_t sc = SL_STATUS_OK;
  (void)&timer;
  (void)&data;

  uint16_t prox;
  uint16_t ambient;
  uint16_t white;

  sc |= vcnl4040_get_proximity(&prox);
  sc |= vcnl4040_get_ambient(&ambient);
  sc |= vcnl4040_get_white(&white);

  if (sc != SL_STATUS_OK) {
    app_log("\r > Reading data failed\n");
  } else {
    app_log(
      "\r > Proximity value: % 4d\tAmbient light value: % 4d\tWhite light value: % 4d\n",
      prox,
      ambient,
      white);
  }
}

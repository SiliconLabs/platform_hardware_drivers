/***************************************************************************//**
 * @file app.c
 * @brief Top level application functions
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
*
* EXPERIMENTAL QUALITY
* This code has not been formally tested and is provided as-is.
* It is not suitable for production environments.
* This code will not be maintained.
*
******************************************************************************/
#include "joystick.h"
#include "sl_app_log.h"
#include "sl_sleeptimer.h"

#define READING_INTERVAL_MSEC 200

// Periodic timer handle.
static sl_sleeptimer_timer_handle_t app_periodic_timer;

static void app_periodic_timer_cb(sl_sleeptimer_timer_handle_t *timer, void *data);

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  sl_status_t sc;

  // Init joystick.
  sc = joystick_init();
  if (sc != SL_STATUS_OK) {
    sl_app_log("Warning! Failed to init Joystick\n");
  }
  else {
    sl_app_log("Joystick initialized\n");
  }

  // Start timer used for periodic indications.
  sc = sl_sleeptimer_start_periodic_timer_ms(&app_periodic_timer,
                                             READING_INTERVAL_MSEC,
                                             app_periodic_timer_cb,
                                             NULL,
                                             0,
                                             SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  if (sc != SL_STATUS_OK) {
    sl_app_log("Warning! Failed to start timer\n");
  }
  else {
    sl_app_log("Timer started\n");
  }
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{

}

/**************************************************************************//**
 * Timer callback
 * Called periodically to reading data from joystick.
 *****************************************************************************/
static void app_periodic_timer_cb(sl_sleeptimer_timer_handle_t *timer, void *data)
{
  (void)data;
  (void)timer;
  sl_status_t sc;
  uint16_t data_pos;

  // Reading current horizontal position
  sc = joystick_read_horizontal_position(&data_pos);
  if (sc != SL_STATUS_OK) {
    sl_app_log("Warning! Invalid Joystick reading\n");
  }
  else {
    sl_app_log("X = %d, ", data_pos);
  }

  // Reading current vertical position
  sc = joystick_read_vertical_position(&data_pos);
  if (sc != SL_STATUS_OK) {
    sl_app_log("Warning! Invalid Joystick reading\n");
  }
  else {
    sl_app_log("Y = %d\n", data_pos);
  }
}

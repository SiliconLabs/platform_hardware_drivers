/***************************************************************************//**
 * @file
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

#include "sl_pwm_instances.h"
#include "sl_simple_button_instances.h"
#include "stepper_a4988.h"

#define APP_STEPPER_RPM 120
#define APP_STEPPER_STEP_NUM 200

static a4988_stepper_inst_t app_stepper_instance = A4988_STEPPER_INST_DEFAULT;
static sl_sleeptimer_timer_handle_t app_stepper_sleeptimer_handle;

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  app_stepper_instance.pwm = &sl_pwm_stepper;
  a4988_stepper_init(&app_stepper_instance);

  a4988_stepper_set_speed(&app_stepper_instance, APP_STEPPER_RPM);
  a4988_stepper_step(&app_stepper_instance,
                     &app_stepper_sleeptimer_handle,
                     APP_STEPPER_STEP_NUM);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

// Emergency button: if the motor is enabled then a button press disables it.
// The next button press will enable and start it again.
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (a4988_stepper_get_state(&app_stepper_instance) == DISABLED) {
      a4988_stepper_enable(&app_stepper_instance, true);
      a4988_stepper_start(&app_stepper_instance);
    } else {
      a4988_stepper_enable(&app_stepper_instance, false);
    }
  }
}

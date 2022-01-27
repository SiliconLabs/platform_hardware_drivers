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

#include "sl_cli.h"
#include "sl_cli_handles.h"
#include "sl_iostream.h"
#include "sl_iostream_handles.h"

#include "sl_simple_button_instances.h"
#include "sl_pwm_instances.h"
#include "stepper_a4988.h"

static a4988_stepper_inst_t app_stepper_instance = A4988_STEPPER_INST_DEFAULT;
static sl_sleeptimer_timer_handle_t app_stepper_sleeptimer;

void app_cli_stepper_set_rpm(sl_cli_command_arg_t *arguments)
{
  sl_status_t sc;
  uint32_t argument_value;

  argument_value = sl_cli_get_argument_uint32(arguments, 0);
  sc = a4988_stepper_set_speed(&app_stepper_instance, argument_value);
  if (sc != SL_STATUS_OK) {
    sl_iostream_printf(sl_iostream_get_handle("vcom"),
                       "Error: %u\r\n",
                       (unsigned int)sc);
    return;
  }
  sl_iostream_printf(sl_iostream_get_handle("vcom"),
                     "Speed was set to: %u \r\n",
                     (unsigned int)argument_value);
}

void app_cli_stepper_set_dir(sl_cli_command_arg_t *arguments)
{
  sl_status_t sc;
  uint8_t argument_value;

  argument_value = sl_cli_get_argument_uint8(arguments, 0);
  sc = a4988_stepper_set_dir(&app_stepper_instance, argument_value);
  if (sc != SL_STATUS_OK) {
    sl_iostream_printf(sl_iostream_get_handle("vcom"),
                       "Error: %u\r\n",
                       (unsigned int)sc);
    return;
  }
  sl_iostream_printf(sl_iostream_get_handle("vcom"),
                     "Direction was set to: %u \r\n",
                     (unsigned int)argument_value);
}

void app_cli_stepper_step(sl_cli_command_arg_t *arguments)
{
  sl_status_t sc;
  uint32_t argument_value;

  argument_value = sl_cli_get_argument_uint32(arguments, 0);
  sc = a4988_stepper_step(&app_stepper_instance,
                          &app_stepper_sleeptimer,
                          argument_value);
  if (sc != SL_STATUS_OK) {
    sl_iostream_printf(sl_iostream_get_handle("vcom"),
                       "Error: %u\r\n",
                       (unsigned int)sc);
    return;
  }
  sl_iostream_printf(sl_iostream_get_handle("vcom"),
                     "Stepper is making %u steps\r\n",
                     (unsigned int)argument_value);
}

void app_cli_stepper_start(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t sc;

  sc = a4988_stepper_start(&app_stepper_instance);
  if (sc != SL_STATUS_OK) {
    sl_iostream_printf(sl_iostream_get_handle("vcom"),
                       "Error: %u\r\n",
                       (unsigned int)sc);
    return;
  }
  sl_iostream_printf(sl_iostream_get_handle("vcom"), "Start motor\r\n");
}

void app_cli_stepper_stop(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t sc;

  sc = a4988_stepper_stop(&app_stepper_instance);
  if (sc != SL_STATUS_OK) {
      sl_iostream_printf(sl_iostream_get_handle("vcom"),
                         "Error: %u\r\n",
                         (unsigned int)sc);
      return;
  }
  sl_iostream_printf(sl_iostream_get_handle("vcom"), "Stop motor\r\n");
}

void app_cli_stepper_enable(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t sc;

  sc = a4988_stepper_enable(&app_stepper_instance, true);
  if (sc != SL_STATUS_OK) {
    sl_iostream_printf(sl_iostream_get_handle("vcom"),
                       "Error: %u\r\n",
                       (unsigned int)sc);
    return;
  }
  sl_iostream_printf(sl_iostream_get_handle("vcom"),
                     "Enable motor drive\r\n");
}

void app_cli_stepper_disable(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t sc;

  sc = a4988_stepper_enable(&app_stepper_instance, false);
  if (sc != SL_STATUS_OK) {
    sl_iostream_printf(sl_iostream_get_handle("vcom"),
                       "Error: %u\r\n",
                       (unsigned int)sc);
    return;
  }
  sl_iostream_printf(sl_iostream_get_handle("vcom"), "Disable motor drive\r\n");
}

// Create command details for the commands. The macro SL_CLI_UNIT_SEPARATOR can be
// used to format the help text for multiple arguments.
static const sl_cli_command_info_t cmd__set_rpm = \
  SL_CLI_COMMAND(app_cli_stepper_set_rpm,
                 "Set speed (RPM)",
                 "Speed in rpm\r",
                 {sl_cli_arg_uint32, sl_cli_arg_end, });

static const sl_cli_command_info_t cmd__set_dir = \
  SL_CLI_COMMAND(app_cli_stepper_set_dir,
                 "Set direction",
                 "0: cw, 1: ccw\r",
                 {sl_cli_arg_uint8, sl_cli_arg_end, });

static const sl_cli_command_info_t cmd__step = \
  SL_CLI_COMMAND(app_cli_stepper_step,
                 "Make steps with the motor",
                 "Step count\r",
                 {sl_cli_arg_uint32, sl_cli_arg_end, });

static const sl_cli_command_info_t cmd__start = \
  SL_CLI_COMMAND(app_cli_stepper_start,
                 "Start stepper motor\r\n",
                 "",
                 {sl_cli_arg_end, });

static const sl_cli_command_info_t cmd__stop = \
  SL_CLI_COMMAND(app_cli_stepper_stop,
                 "Stop stepper motor\r\n",
                 "",
                 {sl_cli_arg_end, });

static const sl_cli_command_info_t cmd__enable = \
  SL_CLI_COMMAND(app_cli_stepper_enable,
                 "Enable stepper motor\r\n",
                 "",
                 {sl_cli_arg_end, });

static const sl_cli_command_info_t cmd__disable = \
  SL_CLI_COMMAND(app_cli_stepper_disable,
                 "Disable stepper motor\r\n",
                 "",
                 {sl_cli_arg_end, });

// Create the array of commands
static sl_cli_command_entry_t app_cli_stepper_command_table[] = {
  { "set_rpm", &cmd__set_rpm, false },
  { "set_dir", &cmd__set_dir, false },
  { "step", &cmd__step, false },
  { "start",  &cmd__start, false },
  { "stop",  &cmd__stop, false },
  { "enable",  &cmd__enable, false },
  { "disable",  &cmd__disable, false },
  { NULL, NULL, false },
};

// Create the command group at the top level
static sl_cli_command_group_t app_cli_stepper_command_group = {
  { NULL },
  false,
  app_cli_stepper_command_table
};

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  app_stepper_instance.pwm = &sl_pwm_stepper;
  a4988_stepper_init(&app_stepper_instance);
  sl_cli_command_add_command_group(sl_cli_inst_handle,
                                   &app_cli_stepper_command_group);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

// Emergency button: if the motor is enabled then a button press disables it.
// The next button press will enable it again.
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (a4988_stepper_get_state(&app_stepper_instance) == DISABLED) {
      a4988_stepper_enable(&app_stepper_instance, true);
    } else {
      a4988_stepper_enable(&app_stepper_instance, false);
    }
  }
}

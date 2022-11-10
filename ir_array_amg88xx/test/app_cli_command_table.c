/***************************************************************************//**
 * @file sl_cli_command_table.c
 * @brief Declarations of relevant command structs for cli framework.
 * @version x.y.z
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
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
 ******************************************************************************/

#include <stdlib.h>

#include "sl_cli_config.h"
#include "sl_cli_command.h"
#include "sl_cli_arguments.h"

#ifdef __cplusplus
extern "C" {
#endif

// Provide function declarations
void app_set_tempereture_scale(sl_cli_command_arg_t *arguments);
void app_get_thermistor_temperature(sl_cli_command_arg_t *arguments);
void app_get_thermistor_temperature_raw(sl_cli_command_arg_t *arguments);
void app_get_sensor_array_temperatures(sl_cli_command_arg_t *arguments);
void app_get_sensor_array_temperatures_raw(sl_cli_command_arg_t *arguments);
void app_sensor_wakeup(sl_cli_command_arg_t *arguments);
void app_sensor_sleep(sl_cli_command_arg_t *arguments);
void app_sensor_60_sec_standby(sl_cli_command_arg_t *arguments);
void app_sensor_10_sec_standby(sl_cli_command_arg_t *arguments);
void app_sensor_enable_moving_average(sl_cli_command_arg_t *arguments);
void app_sensor_disable_moving_average(sl_cli_command_arg_t *arguments);

// Command structs. Names are in the format : cli_cmd_{command group
//   name}_{command name}
// In order to support hyphen in command and group name, every occurence of it
//   while
// building struct names will be replaced by "_hyphen_"
static const sl_cli_command_info_t cli_cmd_set_temp_scale = \
  SL_CLI_COMMAND(app_set_tempereture_scale,
                 "Set sensor temperature scale",
                 "temperature unit (C or F)" SL_CLI_UNIT_SEPARATOR,
                 { SL_CLI_ARG_STRING, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_get_thermistor_temeprature = \
  SL_CLI_COMMAND(app_get_thermistor_temperature,
                 "Get thermistor temperature value",
                 "",
                 { SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_get_thermistor_temeprature_raw = \
  SL_CLI_COMMAND(app_get_thermistor_temperature_raw,
                 "Get thermistor temperature in raw conversion",
                 "",
                 { SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_get_sensor_array_temepratures = \
  SL_CLI_COMMAND(app_get_sensor_array_temperatures,
                 "Get sensor array temperature values",
                 "",
                 { SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_get_sensor_array_temepratures_raw = \
  SL_CLI_COMMAND(app_get_sensor_array_temperatures_raw,
                 "Get sensor array temperature values in raw conversion",
                 "",
                 { SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_sensor_wakeup = \
  SL_CLI_COMMAND(app_sensor_wakeup,
                 "Puts the device to normal mode from any other state.",
                 "",
                 { SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_sensor_sleep = \
  SL_CLI_COMMAND(app_sensor_sleep,
                 "Puts device into sleep mode.",
                 "",
                 { SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_sensor_60_sec_standby = \
  SL_CLI_COMMAND(app_sensor_60_sec_standby,
                 "Puts the device into 60 sec update interval mode.",
                 "",
                 { SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_sensor_10_sec_standby = \
  SL_CLI_COMMAND(app_sensor_10_sec_standby,
                 "Puts the device into 10 sec update interval mode.",
                 "",
                 { SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_sensor_enable_moving_average = \
  SL_CLI_COMMAND(app_sensor_enable_moving_average,
                 "Enable \"Twice Moving Average\" ",
                 "",
                 { SL_CLI_ARG_END, });
static const sl_cli_command_info_t cli_cmd_sensor_disable_moving_average = \
  SL_CLI_COMMAND(app_sensor_disable_moving_average,
                 "Disable \"Twice Moving Average\" ",
                 "",
                 { SL_CLI_ARG_END, });

// Create group command tables and structs if cli_groups given
// in template. Group name is suffixed with _group_table for tables
// and group commands are cli_cmd_( group name )_group
static const sl_cli_command_entry_t moving_average_group_table[] = {
  { "enable", &cli_cmd_sensor_enable_moving_average, false },
  { "disable", &cli_cmd_sensor_disable_moving_average, false },
  { NULL, NULL, false },
};

static const sl_cli_command_info_t cli_cmd_moving_average_group = \
  SL_CLI_COMMAND_GROUP(moving_average_group_table, "");

static const sl_cli_command_entry_t control_group_table[] = {
  { "wake", &cli_cmd_sensor_wakeup, false },
  { "sleep", &cli_cmd_sensor_sleep, false },
  { "60s", &cli_cmd_sensor_60_sec_standby, false },
  { "10s", &cli_cmd_sensor_10_sec_standby, false },
  { "moving_average", &cli_cmd_moving_average_group, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_control_group = \
  SL_CLI_COMMAND_GROUP(control_group_table, "");

static const sl_cli_command_entry_t temperature_group_table[] = {
  { "set_scale", &cli_cmd_set_temp_scale, false },
  { "scaled", &cli_cmd_get_thermistor_temeprature, false },
  { "raw", &cli_cmd_get_thermistor_temeprature_raw, false },
  { "array", &cli_cmd_get_sensor_array_temepratures, false },
  { "array_raw", &cli_cmd_get_sensor_array_temepratures_raw, false },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_temperature_group = \
  SL_CLI_COMMAND_GROUP(temperature_group_table, "");

// Create root command table
const sl_cli_command_entry_t app_cli_command_table[] = {
  { "control", &cli_cmd_control_group, false },
  { "temperature", &cli_cmd_temperature_group, false },
  { NULL, NULL, false },
};

#ifdef __cplusplus
}
#endif

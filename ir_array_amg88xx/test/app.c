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

#include "sl_cli_command.h"
#include "sl_cli_handles.h"
#include "ir_array_amg88xx_driver.h"
#include "sl_i2cspm_instances.h"

extern const sl_cli_command_entry_t app_cli_command_table[];

extern sl_cli_handle_t sl_cli_cli_vcom_handle;
extern sl_cli_command_group_t sl_cli_cli_vcom_command_group;

sl_cli_command_group_t sl_cli_app_command_group =
{
  { NULL },
  false,
  app_cli_command_table
};

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  amg88xx_init(sl_i2cspm_IR_sensor, AMG88XX_ADDRESS_OPEN, CELSIUS);
  // Remove unused autogen CLI command group
  sl_cli_command_remove_command_group(sl_cli_cli_vcom_handle,
                                      &sl_cli_cli_vcom_command_group);

  // Add application CLI commands
  sl_cli_command_add_command_group(sl_cli_cli_vcom_handle,
                                   &sl_cli_app_command_group);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  //
}

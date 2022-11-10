/***************************************************************************//**
 * @file
 * @brief app_adc_cli_cmds.c
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

#include <stdint.h>
#include "sl_cli.h"
#include "sl_cli_arguments.h"
#include "sl_cli_handles.h"
#include "ir_array_amg88xx_driver.h"

void app_set_tempereture_scale(sl_cli_command_arg_t *arguments)
{
  char *str = ((char *)arguments->argv[2]);
  if ((*str == 'f') || (*str == 'F')) {
    set_temperature_scale(FAHRENHEIT);
  } else {
    set_temperature_scale(CELSIUS);
  }
}

void app_get_thermistor_temperature(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  float temperature = 0;
  sl_status_t status_msg = SL_STATUS_OK;
  status_msg = amg88xx_get_thermistor_temperature(&temperature);
  if (status_msg == SL_STATUS_OK) {
    printf("Actual_Temperature: %0.3f \n", temperature);
  } else {
    printf("Error: %lu \n", status_msg);
  }
}

void app_get_thermistor_temperature_raw(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  int16_t temperature = 0;
  sl_status_t status_msg = SL_STATUS_OK;
  status_msg = amg88xx_get_thermistor_temperature_raw(&temperature);
  if (status_msg == SL_STATUS_OK) {
    printf("Actual_Temperature: 0x%x \n", temperature);
  } else {
    printf("Error: %lu \n", status_msg);
  }
}

void app_get_sensor_array_temperatures(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  float temperature_grid[8][8];
  int8_t i = 0;
  int8_t j = 0;
  I2C_TransferReturn_TypeDef read_result;
  read_result = amg88xx_get_sensor_array_temperatures(temperature_grid);
  if (read_result != SL_STATUS_OK) {
    printf("Error: i2c read failed");
  } else {
    while (i < 8)
    {
      j = 7;
      while (j >= 0)
      {
        printf("%3.2f, ", temperature_grid[i][j]);
        j--;
      }
      i++;
    }
    printf("\n");
  }
}

void app_get_sensor_array_temperatures_raw(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  uint16_t temperature_grid[8][8];
  int8_t i = 0;
  int8_t j = 0;
  I2C_TransferReturn_TypeDef read_result;
  read_result = amg88xx_get_sensor_array_temperatures_raw(temperature_grid);
  if (read_result != SL_STATUS_OK) {
    printf("Error: i2c read failed");
  } else {
    while (i < 8)
    {
      j = 7;
      while (j >= 0)
      {
        printf("0x%x, ", temperature_grid[i][j]);
        j--;
      }
      i++;
    }
    printf("\n");
  }
}

void app_sensor_wakeup(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t status_msg = SL_STATUS_OK;
  status_msg = amg88xx_wake();
  if (status_msg != SL_STATUS_OK) {
    printf("Error: %lu \n", status_msg);
  }
}

void app_sensor_sleep(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t status_msg = SL_STATUS_OK;
  status_msg = amg88xx_sleep();
  if (status_msg != SL_STATUS_OK) {
    printf("Error: %lu \n", status_msg);
  }
}

void app_sensor_60_sec_standby(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t status_msg = SL_STATUS_OK;
  status_msg = amg88xx_60_sec_standby();
  if (status_msg != SL_STATUS_OK) {
    printf("Error: %lu \n", status_msg);
  }
}

void app_sensor_10_sec_standby(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t status_msg = SL_STATUS_OK;
  status_msg = amg88xx_10_sec_standby();
  if (status_msg != SL_STATUS_OK) {
    printf("Error: %lu \n", status_msg);
  }
}

void app_sensor_enable_moving_average(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t status_msg = SL_STATUS_OK;
  status_msg = amg88xx_enable_moving_average();
  if (status_msg != SL_STATUS_OK) {
    printf("Error: %lu \n", status_msg);
  }
}

void app_sensor_disable_moving_average(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t status_msg = SL_STATUS_OK;
  status_msg = amg88xx_disable_moving_average();
  if (status_msg != SL_STATUS_OK) {
    printf("Error: %lu \n", status_msg);
  }
}

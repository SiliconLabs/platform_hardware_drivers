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

#include "barometer.h"
#include "sl_i2cspm_instances.h"
#include "app_log.h"

// Indicating an ongoing non-blocking pressure measurement
static uint8_t measurement_flag=0;

// User callback function for the nonblocking sensor read
void barometer_callback(float pressure);

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  app_log("\r\nInit\r\n");

  barometer_init_t init = BAROMETER_INIT_DEFAULT;
  // Using the QWIIC I2CSPM instance which was installed
  init.I2C_port = sl_i2cspm_qwiic;
  barometer_init(&init);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  if(!measurement_flag){
    barometer_get_pressure_non_blocking(barometer_callback);
    measurement_flag = 1;
  }
}

void barometer_callback(float pressure){
  app_log("Pressure: %f hPA\r\n", pressure);
  measurement_flag = 0;
}

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
#include <stdio.h>
#include "app_iostream_usart.h"
#include "barometer.h"

// Indicating an ongoing non-blocking pressure measurement
static uint8_t measurement_flag=0;

// User callback function for the nonblocking sensor read
void barometer_callback(float pressure);

void app_init(void)
{
  app_iostream_usart_init();
  printf("\r\nInit\r\n");

  barometer_init_t init = BAROMETER_INIT_DEFAULT;
  barometer_init(&init);
}

void app_process_action(void)
{
  if(!measurement_flag){
    barometer_get_pressure_non_blocking(barometer_callback);
    measurement_flag = 1;
  }
}

void barometer_callback(float pressure){
  printf("Pressure: %f hPA\r\n", pressure);
  measurement_flag = 0;
}

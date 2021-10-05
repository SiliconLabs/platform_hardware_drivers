/***************************************************************************//**
 * @file  app.c
 * @brief Top level application functions for Qwiic BME280 and CCS811 driver
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                              Includes
// -----------------------------------------------------------------------------

#include <bme280.h>
#include <ccs811.h>
#include <stdio.h>
#include "sl_i2cspm_instances.h"
#include "sl_sleeptimer.h"

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  printf("\r\nInit\r\n");
  // Initialize I2C for the sensor instances.
  bme280_i2c_t init_b = BME280_I2C_DEFAULT;
  bme280_i2c(&init_b);
  ccs811_i2c_t init_c = CCS811_I2C_DEFAULT;
  ccs811_i2c(&init_c);

  if (bme280_init() == STATUS_OK) {
    printf("\n\r BME280 on 0x%X I2C address found"
    "\n\r and initialized", BME_280_DEFAULT_I2C_ADDR);
  } else {
    printf("\n\r BME280 on 0x%X I2C address not found. Check cables. "
    "\n\r Try also alternative address", BME_280_DEFAULT_I2C_ADDR);

    if (BME_280_DEFAULT_I2C_ADDR == 0x77) {
      printf(" 0x76");
    } else {
      printf(" 0x77");
    }
  }

  if (ccs811_init () == STATUS_PASSED) {
    printf("\n\r CCS811 on 0x%X I2C address found"
    "\n\r and initialized", CCS_811_DEFAULT_I2C_ADDR);
  } else {
    printf("\n\r CCS811 on 0x%X I2C address not found. Check cables. "
    "\n\r Try also alternative address ", CCS_811_DEFAULT_I2C_ADDR);

    if (CCS_811_DEFAULT_I2C_ADDR == 0x5B) {
      printf(" 0x5A");
    } else {
      printf(" 0x5B");
    }
  }
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  bme280_ctrl_measure_set_to_work();

  int32_t temp = 0;
  if (bme280_read_temperature(&temp) == STATUS_OK) {
    printf("\n\r Temperature %ld,%ld %cCelsius", temp / 100,
    (temp % 100) / 10, 0XF8);
  }

  uint32_t hum = 0;
  if (bme280_read_humidity(&hum) == STATUS_OK) {
    printf("\r\n Humidity    %ld%%", hum / 1000);
  }

  uint32_t press = 0;
  if (bme280_read_pressure(&press) == STATUS_OK) {
    printf("\r\n Pressure    %ld mBar", press);
  }

  bme280_ctrl_measure_set_to_sleep();

  uint16_t co2 = 0;
  uint16_t tvoc = 0;
  if (ccs811_measure (&co2, &tvoc) == STATUS_PASSED) {
    printf("\n\r CO2         %d ppm", co2);
    printf("\n\r TVOC        %d ppb", tvoc);
  }

  sl_sleeptimer_delay_millisecond(5 * 1000);
}

/***************************************************************************//**
 * @file ir_array_amg88xx_tester.c
 * @brief amg88xx ir array tester.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
* # Evaluation Quality
* This code has been minimally tested to ensure that it builds and is suitable
* as a demonstration for evaluation purposes only. This code will be maintained
* at the sole discretion of Silicon Labs.
******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "sl_sleeptimer.h"
#include "ir_array_amg88xx_driver.h"
#include "ir_array_amg88xx_tester_graphics.h"

// -----------------------------------------------------------------------------
//                                   Defines
// -----------------------------------------------------------------------------

#define TEST_UPPER_INTERRUPT_VALUE_CELSUIS      50
#define TEST_UPPER_INTERRUPT_VALUE_FAHRENHEIT   150
#define TEST_LOWER_INTERRPUT_VALUE_CELSIUS      5
#define TEST_LOWER_INTERRUPT_VALUE_FAHRENHEIT   30
#define TEST_INTERRUPT_HYSTERESIS_CELSIUS       15
#define TEST_INTERRUPT_HYSTERESIS_FAHRENHEIT    40

// -----------------------------------------------------------------------------
//                                Local Variables
// -----------------------------------------------------------------------------

sl_sleeptimer_timer_handle_t cycle_timer;
bool do_cycle;

// -----------------------------------------------------------------------------
//                                Local Functions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * Callback on periodic timeout.
 ******************************************************************************/
static void cycle_timer_callback(sl_sleeptimer_timer_handle_t *handle,
                       void *data)
{
  (void)&handle;
  (void)&data;
  do_cycle = true;
}

static void check_temperature_valid_celsius(float temperature,
                                            int32_t error_code)
{
  if (temperature < 10 || temperature > 50) {
    print_error_msg("wrong temp", error_code);
  }
}

static void check_temperature_valid_fahrenheit(float temperature,
                                               int32_t error_code)
{
  if (temperature < 50 || temperature > 104) {
    print_error_msg("wrong temp", error_code);
  }
}

static void check_sl_status(sl_status_t sl_status, int32_t error_code)
{
  if (sl_status != SL_STATUS_OK) {
    print_error_msg("sl_status err", error_code);
  }
}

// -----------------------------------------------------------------------------
//                                Global Functions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * This function initializes the LCD screen and does initial tests for the
 * sensor.
 * In case something is wrong, the corresponding error message will be shown
 * on the LCD screen.
 ******************************************************************************/
void amg88xx_tester_start(void)
{
  init_graphics();

  // Tester begin.
  float temperature = 0;
  sl_status_t sl_status = SL_STATUS_OK;

  sl_status = amg88xx_get_thermistor_temperature(&temperature);
  check_sl_status(sl_status, 0);
  check_temperature_valid_celsius(temperature, 0);

  set_temperature_scale(FAHRENHEIT);
  sl_status = amg88xx_get_thermistor_temperature(&temperature);
  check_sl_status(sl_status, 1);
  check_temperature_valid_fahrenheit(temperature, 1);

  for (int i = 0; i<SENSOR_ARRAY_COLUMNS*SENSOR_ARRAY_ROWS; i++) {
    set_temperature_scale(CELSIUS);
    sl_status = amg88xx_get_pixel_temperature(i,
                                        &temperature);
    check_sl_status(sl_status, 2);
    check_temperature_valid_celsius(temperature, 2);

    set_temperature_scale(FAHRENHEIT);
    sl_status = amg88xx_get_pixel_temperature(i, &temperature);
    check_sl_status(sl_status, 3);
    check_temperature_valid_fahrenheit(temperature, 3);
  }

  float temperature_grid[SENSOR_ARRAY_COLUMNS][SENSOR_ARRAY_ROWS];
  sl_status = amg88xx_get_sensor_array_temperatures(temperature_grid);
  check_sl_status(sl_status, 4);
  for (int i = 0; i<SENSOR_ARRAY_COLUMNS; i++ ) {
    for (int j = 0; j<SENSOR_ARRAY_ROWS; j++) {
      check_temperature_valid_fahrenheit(temperature_grid[i][j], 4);
    }
  }

  bool is_enabled = false;

  sl_status = amg88xx_interrupt_pin_enable();
  check_sl_status(sl_status, 5);
  sl_status = amg88xx_is_interrupt_pin_enabled(&is_enabled);
  check_sl_status(sl_status, 6);
  if (!is_enabled) {
    print_error_msg("int pin err", 0);
  }

  sl_status = amg88xx_interrupt_pin_disable();
  check_sl_status(sl_status, 7);
  sl_status = amg88xx_is_interrupt_pin_enabled(&is_enabled);
  check_sl_status(sl_status, 8);
  if (is_enabled) {
    print_error_msg("int pin err", 1);
  }

  // Functions for manipulating Status/Clear Registers

  // Read Interrupt Table Register
  sl_status = amg88xx_enable_moving_average();
  check_sl_status(sl_status, 9);
  sl_status = amg88xx_is_moving_average_enabled(&is_enabled);
  check_sl_status(sl_status, 10);
  if (!is_enabled) {
    print_error_msg("mov avg err", 0);
  }

  sl_status = amg88xx_disable_moving_average();
  check_sl_status(sl_status, 11);
  sl_status = amg88xx_is_moving_average_enabled(&is_enabled);
  check_sl_status(sl_status, 12);
  if (is_enabled) {
    print_error_msg("mov avg err", 1);
  }

  set_temperature_scale(CELSIUS);
  sl_status = amg88xx_set_upper_interrupt_value(
                                            TEST_UPPER_INTERRUPT_VALUE_CELSUIS);
  check_sl_status(sl_status, 13);
  sl_status = amg88xx_get_upper_interrupt_value(&temperature);
  check_sl_status(sl_status, 14);
  if (temperature != TEST_UPPER_INTERRUPT_VALUE_CELSUIS) {
    print_error_msg("uiv err", 0);
  }

  set_temperature_scale(FAHRENHEIT);
  sl_status = amg88xx_set_upper_interrupt_value(
                                         TEST_UPPER_INTERRUPT_VALUE_FAHRENHEIT);
  check_sl_status(sl_status, 15);
  sl_status = amg88xx_get_upper_interrupt_value(&temperature);
  check_sl_status(sl_status, 16);
  if ((uint32_t)temperature != TEST_UPPER_INTERRUPT_VALUE_FAHRENHEIT-1) {
    print_error_msg("uiv err", 1);
  }

  set_temperature_scale(CELSIUS);
  sl_status = amg88xx_set_lower_interrupt_value(
                                            TEST_LOWER_INTERRPUT_VALUE_CELSIUS);
  check_sl_status(sl_status, 17);
  sl_status = amg88xx_get_lower_interrupt_value(&temperature);
  check_sl_status(sl_status, 18);
  if (temperature != TEST_LOWER_INTERRPUT_VALUE_CELSIUS) {
    print_error_msg("liv err", 0);
  }

  set_temperature_scale(FAHRENHEIT);
  sl_status = amg88xx_set_lower_interrupt_value(
                                         TEST_LOWER_INTERRUPT_VALUE_FAHRENHEIT);
  check_sl_status(sl_status, 19);
  sl_status = amg88xx_get_lower_interrupt_value(&temperature);
  check_sl_status(sl_status, 20);
  if ((uint32_t)temperature != TEST_LOWER_INTERRUPT_VALUE_FAHRENHEIT) {
    print_error_msg("liv err", 1);
  }

  set_temperature_scale(CELSIUS);
  sl_status = amg88xx_set_interrupt_hysteresis(
                                             TEST_INTERRUPT_HYSTERESIS_CELSIUS);
  check_sl_status(sl_status, 21);
  sl_status = amg88xx_get_interrupt_hysteresis(&temperature);
  check_sl_status(sl_status, 22);
  if (temperature != TEST_INTERRUPT_HYSTERESIS_CELSIUS) {
    print_error_msg("ih err", 0);
  }

  set_temperature_scale(FAHRENHEIT);
  sl_status = amg88xx_set_interrupt_hysteresis(
                                          TEST_INTERRUPT_HYSTERESIS_FAHRENHEIT);
  check_sl_status(sl_status, 23);
  sl_status = amg88xx_get_interrupt_hysteresis(&temperature);
  check_sl_status(sl_status, 24);
  if ((uint32_t)temperature != TEST_INTERRUPT_HYSTERESIS_FAHRENHEIT) {
    print_error_msg("ih err", 1);
  }

  set_temperature_scale(CELSIUS);

  // Tester end.

  sl_sleeptimer_start_periodic_timer_ms(&cycle_timer,
                                        100,
                                        cycle_timer_callback, NULL,
                                        0,
                       SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
}

/***************************************************************************//**
 * Main amg88xx tester logic.
 ******************************************************************************/
void amg88xx_tester_process_action(void)
{
  if (do_cycle) {
    do_cycle = false;
    float temperature_grid[8][8];
    I2C_TransferReturn_TypeDef read_result;
    read_result = amg88xx_get_sensor_array_temperatures(temperature_grid);
    if (read_result != SL_STATUS_OK) {
      print_error_msg("i2c read failed", (int)read_result);
    }

    print_temps(temperature_grid);
  }
}

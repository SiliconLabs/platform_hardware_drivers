/***************************************************************************//**
 * @file ir_array_amg88xx_driver.c
 * @version 1.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 *
 * EVALUATION QUALITY
 * This code has been minimally tested to ensure that it builds with the
 * specified dependency versions and is suitable as a demonstration for
 * evaluation purposes only.
 * This code will be maintained at the sole discretion of Silicon Labs.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "em_gpio.h"
#include "em_cmu.h"
#include "sl_status.h"
#include "ir_array_amg88xx_driver.h"

// -----------------------------------------------------------------------------
//                                Local Variables
// -----------------------------------------------------------------------------
static uint16_t amg88xx_I2C_address = AMG88XX_ADDRESS_OPEN;

static enum temperature_scale_t temperature_scale = CELSIUS;

static sl_i2cspm_t *amg88xx_I2C_instance = NULL;

// -----------------------------------------------------------------------------
//                                Local Functions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * Read numBytes from the amg88xx trough i2c starting at register_address.
 *
 * @param register_address The address of a register contained by the target
 * i2c device.
 * @param rx_buffer The data read from the i2c device will be stored here.
 * @param number_of_bytes Number of bytes will be read from the i2c device.
 *
 * @returns The result of the transaction.
 ******************************************************************************/
static sl_status_t amg88xx_i2c_read(uint8_t register_address,
                                    uint8_t *rx_buffer,
                                    uint8_t number_of_bytes)
{
  // Transfer structure.
  I2C_TransferSeq_TypeDef i2c_transfer;
  I2C_TransferReturn_TypeDef result;

  // Initializing I2C transfer.
  i2c_transfer.addr = (uint16_t) (amg88xx_I2C_address << 1);
  // Must write target address before reading.
  i2c_transfer.flags = I2C_FLAG_WRITE_READ;
  i2c_transfer.buf[0].data = &register_address;
  i2c_transfer.buf[0].len = 1;
  i2c_transfer.buf[1].data = rx_buffer;
  i2c_transfer.buf[1].len = number_of_bytes;

  result = I2CSPM_Transfer(amg88xx_I2C_instance, &i2c_transfer);

  if (result == i2cTransferDone) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FAIL;
  }
}

/***************************************************************************//**
 * I2C write numBytes to the amg88xx trough i2c starting at register_address.
 *
 * @param register_address is the address of a register contained by the target
 * i2c device
 * @param tx_buffer the data to be write to the i2c device
 * @param number_of_bytes number of bytes to be written to the i2c device
 *
 * @returns The result of the transaction.
 ******************************************************************************/
static sl_status_t amg88xx_i2c_write(uint8_t register_address,
                                     const uint8_t *tx_buffer,
                                     uint8_t number_of_bytes)
{
  // Transfer structure.
  I2C_TransferSeq_TypeDef i2c_transfer;
  I2C_TransferReturn_TypeDef result;
  uint8_t tx_buffer_with_address[I2C_BUFFER_SIZE + 1];

  // Register address to write to.
  tx_buffer_with_address[0] = register_address;
  for (uint8_t i = 0; i < number_of_bytes; i++) {
    tx_buffer_with_address[i + 1] = tx_buffer[i];
  }

  // Initializing I2C transfer.
  i2c_transfer.addr = (uint16_t) (amg88xx_I2C_address << 1);
  i2c_transfer.flags = I2C_FLAG_WRITE;
  i2c_transfer.buf[0].data = tx_buffer_with_address;
  i2c_transfer.buf[0].len = number_of_bytes + 1;
  i2c_transfer.buf[1].data = NULL;
  i2c_transfer.buf[1].len = 0;

  result = I2CSPM_Transfer(amg88xx_I2C_instance, &i2c_transfer);

  if (result == i2cTransferDone) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FAIL;
  }
}

/***************************************************************************//**
 * Converts raw temperature to celsius.
 *
 * @param temp Raw temperature.
 *
 * @returns The temperature in celsius.
 ******************************************************************************/
static float convert_raw_to_celsius(int16_t temperature)
{
  // Temperature is reported as 12-bit twos complement.
  // Check if temperature is negative.
  if (temperature & (1 << 11)) {
    // if temperature is negative, mask out the sign byte and
    // make the float negative
    temperature &= ~(1 << 11);
    temperature = temperature * -1;
  }

  return (float)(temperature * 0.25);
}

/***************************************************************************//**
 * Converts raw temperature to fahrenheit.
 *
 * @param Temp raw temperature
 *
 * @returns The temperature in fahrenheit.
 ******************************************************************************/
static float convert_raw_to_fahrenheit(int16_t temperature)
{
  // temperature is reported as 12-bit twos complement
  // check if temperature is negative
  if (temperature & (1 << 11)) {
    // if temperature is negative, mask out the sign byte and
    // make the float negative
    temperature &= ~(1 << 11);
    temperature = temperature * -1;
  }

  return (float)((float)(temperature * 0.25) * 1.8 + 32);
}

/***************************************************************************//**
 * Converts celsius to raw.
 *
 * @param degrees_C Temperature in celsius.
 *
 * @returns The temperature in raw format.
 ******************************************************************************/
static uint16_t convert_celsius_to_raw(float degrees_C)
{
  bool is_negative = false;

  if (degrees_C < 0) {
    degrees_C = (float)(degrees_C * -1.0);
    is_negative = true;
  }

  uint16_t temperature = 0;
  temperature = (uint16_t)roundf(degrees_C * 4);

  if (is_negative) {
    temperature |= (1 << 11);
  }

  return temperature;
}

/***************************************************************************//**
 * Converts fahrenheit to raw.
 *
 * @param degrees_F Temperature in fahrenheit.
 *
 * @returns The temperature in raw format.
 ******************************************************************************/
static uint16_t convert_fahrenheit_to_raw(float degrees_F)
{
  float degrees_C = (float) ((degrees_F - 32) / 1.8);
  return convert_celsius_to_raw(degrees_C);
}

// -----------------------------------------------------------------------------
//                                Global Functions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * Initialise the periphery
 ******************************************************************************/
void amg88xx_init(sl_i2cspm_t *amg88xx_i2c_instance_init,
                  int16_t I2C_address,
                  enum temperature_scale_t temp_scale)
{
  amg88xx_I2C_instance = amg88xx_i2c_instance_init;
  amg88xx_I2C_address = I2C_address;
  set_temperature_scale(temp_scale);
}

/***************************************************************************//**
 * Sets temperature scale for temperature related functions.
 ******************************************************************************/
void set_temperature_scale(enum temperature_scale_t temp_scale)
{
  temperature_scale = temp_scale;
}

/***************************************************************************//**
 * Get thermistor temperature.
 ******************************************************************************/
sl_status_t amg88xx_get_thermistor_temperature(float *thermistor_temperature)
{
  int16_t temperature = 0;
  uint32_t read_result;
  read_result = amg88xx_get_thermistor_temperature_raw(&temperature);
  if (read_result == SL_STATUS_OK) {
    // Temperature is reported as 12-bit twos complement.
    // Check if temperature is negative.
    if (temperature & (1 << 11)) {
      // If temperature is negative, mask out the sign byte and
      // make the float negative.
      temperature &= ~(1 << 11);
      temperature = temperature * -1;
    }
    *thermistor_temperature = (float) (temperature * 0.0625);
    if (temperature_scale == FAHRENHEIT) {
      *thermistor_temperature = (float) (*thermistor_temperature * 1.8 + 32);
    }
  }

  return read_result;
}

sl_status_t amg88xx_get_thermistor_temperature_raw(
  int16_t *raw_pixel_temperature)
{
  return amg88xx_i2c_read(THERMISTOR_REGISTER_LSB,
                          (void *)raw_pixel_temperature,
                          2);
}

/***************************************************************************//**
 * Get the temperature provided by a pixel.
 ******************************************************************************/
sl_status_t amg88xx_get_pixel_temperature(unsigned char pixel_number,
                                          float *pixel_temperature)
{
  int16_t temperature = 0;
  sl_status_t result = amg88xx_get_pixel_temperature_raw(pixel_number,
                                                         &temperature);
  if (temperature_scale == CELSIUS) {
    *pixel_temperature = convert_raw_to_celsius(temperature);
  } else {
    *pixel_temperature = convert_raw_to_fahrenheit(temperature);
  }
  return result;
}

sl_status_t amg88xx_get_pixel_temperature_raw(unsigned char pixel_number,
                                              int16_t *raw_pixel_temperature)
{
  return amg88xx_i2c_read(TEMPERATURE_REGISTER_START + (2 * pixel_number),
                          (void *)raw_pixel_temperature,
                          2);
}

/***************************************************************************//**
 * Get the temperatures of the IR sensor array.
 ******************************************************************************/
sl_status_t amg88xx_get_sensor_array_temperatures(
  float temperature_grid[SENSOR_ARRAY_COLUMNS][SENSOR_ARRAY_ROWS])
{
  uint8_t i = 0;
  uint16_t temperature_grid_raw[SENSOR_ARRAY_COLUMNS][SENSOR_ARRAY_ROWS];
  uint32_t read_result;
  read_result = amg88xx_i2c_read(TEMPERATURE_REGISTER_START,
                                 (void *)temperature_grid_raw,
                                 SENSOR_ARRAY_ROWS * SENSOR_ARRAY_COLUMNS * 2);
  while ((read_result == SL_STATUS_OK) && (i < SENSOR_ARRAY_COLUMNS)) {
    for (int j = 0; j < SENSOR_ARRAY_ROWS; j++) {
      if (temperature_scale == CELSIUS) {
        temperature_grid[i][j] = convert_raw_to_celsius(
          temperature_grid_raw[i][j]);
      } else {
        temperature_grid[i][j] = convert_raw_to_fahrenheit(
          temperature_grid_raw[i][j]);
      }
    }
    i++;
  }

  return read_result;
}

sl_status_t amg88xx_get_sensor_array_temperatures_raw(
  uint16_t temperature_grid[SENSOR_ARRAY_COLUMNS][SENSOR_ARRAY_ROWS])
{
  return amg88xx_i2c_read(TEMPERATURE_REGISTER_START,
                          (void *)temperature_grid,
                          SENSOR_ARRAY_ROWS * SENSOR_ARRAY_COLUMNS
                          * 2);
}

/***************************************************************************//**
 * Sets the I2C address of the amg88xx.
 ******************************************************************************/
void amg88xx_set_i2c_address(uint16_t address)
{
  amg88xx_I2C_address = address;
}

/***************************************************************************//**
 * Returns device to normal mode from any other state.
 ******************************************************************************/
sl_status_t amg88xx_wake(void)
{
  uint8_t register_value = 0x00;
  return amg88xx_i2c_write(POWER_CONTROL_REGISTER, &register_value, 1);
}

/***************************************************************************//**
 * Puts device into sleep mode.
 ******************************************************************************/
sl_status_t amg88xx_sleep(void)
{
  uint8_t register_value = 0x10;
  return amg88xx_i2c_write(POWER_CONTROL_REGISTER, &register_value, 1);
}

/***************************************************************************//**
 * Puts the device into 60 sec update interval mode.
 ******************************************************************************/
sl_status_t amg88xx_60_sec_standby(void)
{
  uint8_t register_value = 0x20;
  return amg88xx_i2c_write(POWER_CONTROL_REGISTER, &register_value, 1);
}

/***************************************************************************//**
 * Puts the device into 10 sec update interval mode.
 ******************************************************************************/
sl_status_t amg88xx_10_sec_standby(void)
{
  uint8_t register_value = 0x21;
  return amg88xx_i2c_write(POWER_CONTROL_REGISTER, &register_value, 1);
}

/***************************************************************************//**
 * Functions for manipulating Interrupt Control Register.
 ******************************************************************************/
sl_status_t amg88xx_interrupt_pin_enable(void)
{
  uint8_t ICR_value;
  sl_status_t read_result;
  read_result = amg88xx_i2c_read(INT_CONTROL_REGISTER, &ICR_value, 1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  ICR_value |= (1 << 0);
  ICR_value &= 0xFF;
  return amg88xx_i2c_write(INT_CONTROL_REGISTER, &ICR_value, 1);
}

sl_status_t amg88xx_interrupt_pin_disable(void)
{
  uint8_t ICR_value;
  sl_status_t read_result;
  read_result = amg88xx_i2c_read(INT_CONTROL_REGISTER, &ICR_value, 1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  ICR_value &= ~(1 << 0);
  ICR_value &= 0xFF;
  return amg88xx_i2c_write(INT_CONTROL_REGISTER, &ICR_value, 1);
}

sl_status_t amg88xx_set_interrupt_mode_absolute(void)
{
  uint8_t ICR_value;
  sl_status_t read_result;
  read_result = amg88xx_i2c_read(INT_CONTROL_REGISTER, &ICR_value, 1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  ICR_value |= (1 << 1);
  ICR_value &= 0xFF;
  return amg88xx_i2c_write(INT_CONTROL_REGISTER, &ICR_value, 1);
}

sl_status_t amg88xx_set_interrupt_mode_difference(void)
{
  uint8_t ICR_value;
  sl_status_t read_result;
  read_result = amg88xx_i2c_read(INT_CONTROL_REGISTER, &ICR_value, 1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  ICR_value &= ~(1 << 1);
  ICR_value &= 0xFF;
  return amg88xx_i2c_write(INT_CONTROL_REGISTER, &ICR_value, 1);
}

sl_status_t amg88xx_is_interrupt_pin_enabled(bool *is_enabled)
{
  uint8_t ICR_value;
  sl_status_t read_result;
  read_result = amg88xx_i2c_read(INT_CONTROL_REGISTER, &ICR_value, 1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  if (ICR_value & (1 << 0)) {
    *is_enabled = true;
  } else {
    *is_enabled = false;
  }

  return read_result;
}

/***************************************************************************//**
 * Functions for manipulating Status/Clear Registers.
 ******************************************************************************/
sl_status_t amg88xx_is_interrupt_flag_set(bool *is_set)
{
  uint8_t status_register_value;
  sl_status_t read_result;
  read_result = amg88xx_i2c_read(STATUS_REGISTER, &status_register_value, 1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  if (status_register_value & (1 << 1)) {
    *is_set = true;
  } else {
    *is_set = false;
  }

  return read_result;
}

sl_status_t amg88xx_is_temperature_output_overflow_flag_set(bool *is_set)
{
  uint8_t status_register_value;
  sl_status_t read_result;
  read_result = amg88xx_i2c_read(STATUS_REGISTER, &status_register_value, 1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  if (status_register_value & (1 << 2)) {
    *is_set = false;
  } else {
    *is_set = true;
  }

  return read_result;
}

sl_status_t amg88xx_is_thermistor_output_overflow_flag_set(bool *is_set)
{
  uint8_t status_register_value;
  sl_status_t read_result;
  read_result = amg88xx_i2c_read(STATUS_REGISTER, &status_register_value, 1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  if (status_register_value & (1 << 3)) {
    *is_set = false;
  } else {
    *is_set = true;
  }

  return read_result;
}

sl_status_t amg88xx_clear_interrupt_flag(void)
{
  uint8_t register_value = 0x02;
  return amg88xx_i2c_write(STATUS_CLEAR_REGISTER, &register_value, 1);
}

sl_status_t amg88xx_clear_temperature_output_overflow_flag(void)
{
  uint8_t register_value = 0x04;
  return amg88xx_i2c_write(STATUS_CLEAR_REGISTER, &register_value, 1);
}

sl_status_t amg88xx_clear_thermistor_output_overflow_flag(void)
{
  uint8_t register_value = 0x08;
  return amg88xx_i2c_write(STATUS_CLEAR_REGISTER, &register_value, 1);
}

sl_status_t amg88xx_clear_all_overflow(void)
{
  uint8_t register_value = 0x0C;
  return amg88xx_i2c_write(STATUS_CLEAR_REGISTER, &register_value, 1);
}

sl_status_t amg88xx_clear_all_status_flags(void)
{
  uint8_t register_value = 0x0E;
  return amg88xx_i2c_write(STATUS_CLEAR_REGISTER, &register_value, 1);
}

/***************************************************************************//**
 * Read Interrupt Table Register.
 ******************************************************************************/
sl_status_t amg88xx_is_pixel_interrupt_set(uint8_t pixel_number, bool *is_set)
{
  uint8_t interrupt_table_register = INT_TABLE_REGISTER_INT0
                                     + (pixel_number / 8);
  uint8_t pixel_position = (pixel_number % 8);

  uint8_t interrupt_table_row;
  sl_status_t read_result;
  read_result = amg88xx_i2c_read(interrupt_table_register,
                                 &interrupt_table_row,
                                 1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  if (interrupt_table_row & (1 << pixel_position)) {
    *is_set = true;
  } else {
    *is_set = false;
  }

  return read_result;
}

/***************************************************************************//**
 * Functions for manipulating Average Register.
 ******************************************************************************/
sl_status_t amg88xx_enable_moving_average(void)
{
  sl_status_t read_result;

  uint8_t register_value = 0x50;
  read_result = amg88xx_i2c_write(RESERVED_AVERAGE_REGISTER,
                                  &register_value,
                                  1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  register_value = 0x45;
  read_result = amg88xx_i2c_write(RESERVED_AVERAGE_REGISTER,
                                  &register_value,
                                  1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  register_value = 0x57;
  read_result = amg88xx_i2c_write(RESERVED_AVERAGE_REGISTER,
                                  &register_value,
                                  1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  register_value = 0x20;
  read_result = amg88xx_i2c_write(AVERAGE_REGISTER,
                                  &register_value,
                                  1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  register_value = 0x00;
  read_result = amg88xx_i2c_write(RESERVED_AVERAGE_REGISTER,
                                  &register_value,
                                  1);
  return read_result;
}

sl_status_t amg88xx_disable_moving_average(void)
{
  sl_status_t read_result;

  uint8_t register_value = 0x50;
  read_result = amg88xx_i2c_write(RESERVED_AVERAGE_REGISTER,
                                  &register_value,
                                  1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  register_value = 0x45;
  read_result = amg88xx_i2c_write(RESERVED_AVERAGE_REGISTER,
                                  &register_value,
                                  1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  register_value = 0x57;
  read_result = amg88xx_i2c_write(RESERVED_AVERAGE_REGISTER,
                                  &register_value,
                                  1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  register_value = 0x00;
  read_result = amg88xx_i2c_write(AVERAGE_REGISTER,
                                  &register_value,
                                  1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  register_value = 0x00;
  read_result = amg88xx_i2c_write(RESERVED_AVERAGE_REGISTER,
                                  &register_value,
                                  1);
  return read_result;
}

sl_status_t amg88xx_is_moving_average_enabled(bool *is_enabled)
{
  uint8_t average_register_value;
  sl_status_t read_result;
  read_result = amg88xx_i2c_read(AVERAGE_REGISTER,
                                 &average_register_value,
                                 1);
  if (read_result != SL_STATUS_OK) {
    return read_result;
  }

  if (average_register_value & (1 << 5)) {
    *is_enabled = true;
  } else {
    *is_enabled = false;
  }

  return read_result;
}

/***************************************************************************//**
 * Functions for manipulating Interrupt Level Register.
 ******************************************************************************/
sl_status_t amg88xx_set_upper_interrupt_value(float degrees)
{
  uint16_t temperature = 0;
  if (temperature_scale == CELSIUS) {
    temperature = convert_celsius_to_raw(degrees);
  } else {
    temperature = convert_fahrenheit_to_raw(degrees);
  }

  return amg88xx_i2c_write(INT_LEVEL_REGISTER_UPPER_LSB,
                           (void *)&temperature,
                           2);
}

sl_status_t amg88xx_set_upper_interrupt_value_raw(int16_t register_value)
{
  return amg88xx_i2c_write(INT_LEVEL_REGISTER_UPPER_LSB,
                           (void *)&register_value,
                           2);
}

sl_status_t amg88xx_set_lower_interrupt_value(float degrees)
{
  uint16_t temperature = 0;
  if (temperature_scale == CELSIUS) {
    temperature = convert_celsius_to_raw(degrees);
  } else {
    temperature = convert_fahrenheit_to_raw(degrees);
  }

  return amg88xx_i2c_write(INT_LEVEL_REGISTER_LOWER_LSB,
                           (void *)&temperature,
                           2);
}

sl_status_t amg88xx_set_lower_interrupt_value_raw(int16_t register_value)
{
  return amg88xx_i2c_write(INT_LEVEL_REGISTER_LOWER_LSB,
                           (void *)&register_value,
                           2);
}

sl_status_t amg88xx_set_interrupt_hysteresis(float degrees)
{
  uint16_t temperature = 0;
  if (temperature_scale == CELSIUS) {
    temperature = convert_celsius_to_raw(degrees);
  } else {
    temperature = convert_fahrenheit_to_raw(degrees);
  }

  return amg88xx_set_interrupt_hysteresis_raw(temperature);
}

sl_status_t amg88xx_set_interrupt_hysteresis_raw(int16_t register_value)
{
  return amg88xx_i2c_write(INT_LEVEL_REGISTER_HYST_LSB,
                           (void *)&register_value,
                           2);
}

sl_status_t amg88xx_get_upper_interrupt_value(float *degrees)
{
  int16_t temperature = 0;
  sl_status_t result = amg88xx_get_upper_interrupt_value_raw(&temperature);
  if (result != SL_STATUS_OK) {
    return result;
  }

  if (temperature_scale == CELSIUS) {
    *degrees = convert_raw_to_celsius(temperature);
  } else {
    *degrees = convert_raw_to_fahrenheit(temperature);
  }

  return result;
}

sl_status_t amg88xx_get_upper_interrupt_value_raw(int16_t *register_value)
{
  return amg88xx_i2c_read(INT_LEVEL_REGISTER_UPPER_LSB,
                          (void *)register_value,
                          2);
}

sl_status_t amg88xx_get_lower_interrupt_value(float *degrees)
{
  int16_t temperature = 0;
  sl_status_t result = amg88xx_get_lower_interrupt_value_raw(&temperature);
  if (result != SL_STATUS_OK) {
    return result;
  }

  if (temperature_scale == CELSIUS) {
    *degrees = convert_raw_to_celsius(temperature);
  } else {
    *degrees = convert_raw_to_fahrenheit(temperature);
  }

  return result;
}

sl_status_t amg88xx_get_lower_interrupt_value_raw(int16_t *register_value)
{
  return amg88xx_i2c_read(INT_LEVEL_REGISTER_LOWER_LSB,
                          (void *)register_value,
                          2);
}

sl_status_t amg88xx_get_interrupt_hysteresis(float *degrees)
{
  int16_t temperature = 0;
  sl_status_t result = amg88xx_get_interrupt_hysteresis_raw(&temperature);
  if (result != SL_STATUS_OK) {
    return result;
  }

  if (temperature_scale == CELSIUS) {
    *degrees = convert_raw_to_celsius(temperature);
  } else {
    *degrees = convert_raw_to_fahrenheit(temperature);
  }

  return result;
}

sl_status_t amg88xx_get_interrupt_hysteresis_raw(int16_t *register_value)
{
  return amg88xx_i2c_read(INT_LEVEL_REGISTER_HYST_LSB,
                          (void *)register_value,
                          2);
}

/***************************************************************************//**
 * @file ir_array_amg88xx_driver.h
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

#ifndef IR_ARRAY_AMG88XX_DRIVER_H_
#define IR_ARRAY_AMG88XX_DRIVER_H_

#include "sl_status.h"
#include "sl_i2cspm.h"

// -----------------------------------------------------------------------------
//                                   Defines
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * Dimensions of the sensor array.
 ******************************************************************************/
#define SENSOR_ARRAY_ROWS             8
#define SENSOR_ARRAY_COLUMNS          8

#define I2C_BUFFER_SIZE               10

/***************************************************************************//**
 * For detailed description for the following registers see the datasheet.
 ******************************************************************************/
#define AMG88XX_ADDRESS_OPEN          0x69
#define AMG88XX_ADDRESS_CLOSE         0x68

#define POWER_CONTROL_REGISTER        0x00
#define RESET_REGISTER                0x01
#define FRAMERATE_REGISTER            0x02
#define INT_CONTROL_REGISTER          0x03
#define STATUS_REGISTER               0x04
#define STATUS_CLEAR_REGISTER         0x05
#define AVERAGE_REGISTER              0x07
#define INT_LEVEL_REGISTER_UPPER_LSB  0x08
#define INT_LEVEL_REGISTER_UPPER_MSB  0x09
#define INT_LEVEL_REGISTER_LOWER_LSB  0x0A
#define INT_LEVEL_REGISTER_LOWER_MSB  0x0B
#define INT_LEVEL_REGISTER_HYST_LSB   0x0C
#define INT_LEVEL_REGISTER_HYST_MSB   0x0D
#define THERMISTOR_REGISTER_LSB       0x0E
#define THERMISTOR_REGISTER_MSB       0x0F
#define INT_TABLE_REGISTER_INT0       0x10
#define RESERVED_AVERAGE_REGISTER     0x1F
#define TEMPERATURE_REGISTER_START    0x80

// -----------------------------------------------------------------------------
//                            Variable declarations
// -----------------------------------------------------------------------------
enum temperature_scale_t{CELSIUS,
                         FAHRENHEIT};

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Initialise the periphery
 *
 * @param i2c         The used I2C instance.
 * @param I2C_address The used device address.
 * @param temp_scale  Temperature scale. Can be "celsius" or "fahrenheit".
 ******************************************************************************/
void amg88xx_init(sl_i2cspm_t *amg88xx_i2c_instance_init,
                  int16_t I2C_address,
                  enum temperature_scale_t temp_scale);

/***************************************************************************//**
 * Sets temperature scale for temperature related functions.
 * The scale is initialized to celsius by default.
 *
 * @param temp_scale Temperature scale. Can be "celsius" or "fahrenheit".
 ******************************************************************************/
void set_temperature_scale(enum temperature_scale_t temp_scale);

/***************************************************************************//**
 * Gets the thermistor temperature.
 * The temperature scale can be set globally with set_temperature_scale().
 *
 * @param thermistor_temperature Temperature of the thermistor.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_get_thermistor_temperature(float *thermistor_temperature);

/***************************************************************************//**
 * Gets the raw thermistor temperature.
 *
 * @param thermistor_temperature Raw temperature of the device.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_get_thermistor_temperature_raw(
  int16_t *thermistor_temperature);

/***************************************************************************//**
 * Gets the temperature of a single pixel.
 * The temperature scale can be set globally with set_temperature_scale().
 * For the pixel map check the amg88xx datasheet:
 * https://industrial.panasonic.com/cdbs/www-data/pdf/ADI8000/ADI8000C66.pdf
 *
 * @param pixel_number # of the pixel in order as in the registers. (0-63)
 * @param pixel_temperature Temperature of the pixel identified by pixel_number
 * in celsius.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_get_pixel_temperature(unsigned char pixelAddr,
                                          float *pixel_temperature);

/*******************************************************************************
 * Gets the raw temperature value of a single pixel.
 * For the pixel map check the amg88xx datasheet:
 * https://industrial.panasonic.com/cdbs/www-data/pdf/ADI8000/ADI8000C66.pdf
 *
 * @param pixel_number # of the pixel in order as in the registers. (0-63)
 * @param pixel_temperature Raw temperature of the pixel identified by
 * pixel_number.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_get_pixel_temperature_raw(unsigned char pixelAddr,
                                              int16_t *raw_pixel_temperature);

/***************************************************************************//**
 * Gets the temperatures of the IR sensor array.
 * The temperature scale can be set globally with set_temperature_scale().
 * For the pixel map check the amg88xx datasheet:
 * https://industrial.panasonic.com/cdbs/www-data/pdf/ADI8000/ADI8000C66.pdf
 *
 * @param temperature_grid Array of temperatures.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_get_sensor_array_temperatures(
  float temperature_grid[SENSOR_ARRAY_COLUMNS][SENSOR_ARRAY_ROWS]);

/***************************************************************************//**
 * Gets the raw temperatures of the IR sensor array.
 * For the pixel map check the amg88xx datasheet:
 * https://industrial.panasonic.com/cdbs/www-data/pdf/ADI8000/ADI8000C66.pdf
 *
 * @param temperature_grid Array of temperatures.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_get_sensor_array_temperatures_raw(
  uint16_t temperature_grid[SENSOR_ARRAY_COLUMNS][SENSOR_ARRAY_ROWS]);

/***************************************************************************//**
 * Sets the I2C address of the amg88xx.
 *
 * @param address i2c address of the amg88xx
 ******************************************************************************/
void amg88xx_set_i2c_address(uint16_t address);

/***************************************************************************//**
 * Puts the device to normal mode from any other state.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_wake(void);

/***************************************************************************//**
 * Puts device into sleep mode.
 * Temperature registers are not updated.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_sleep(void);

/***************************************************************************//**
 * Puts the device into 60 sec update interval mode.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_60_sec_standby(void);

/***************************************************************************//**
 * Puts the device into 10 sec update interval mode.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_10_sec_standby(void);

/***************************************************************************//**
 * Enable INT pin to pull low on interrupt flag.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_interrupt_pin_enable(void);

/***************************************************************************//**
 * Put INT pin into Hi-Z state.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_interrupt_pin_disable(void);

/***************************************************************************//**
 * Set interrupt mode to "Absolute Value" mode.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_set_interrupt_mode_absolute(void);

/***************************************************************************//**
 * Set interrupt mode to "Difference" mode.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_set_interrupt_mode_difference(void);

/***************************************************************************//**
 * is_enabled will be true if the INT pin is enabled.
 * False if INT pin is in Hi-Z.
 *
 * @param is_enabled True if the interrupt pin enabled, false otherwise.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_is_interrupt_pin_enabled(bool *is_enabled);

/***************************************************************************//**
 * is_set will be true,
 * if there is an interrupt flag in the status register.
 *
 * @param is_set True, if the interrupt flag is set, false otherwise.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_is_interrupt_flag_set(bool *is_set);

/***************************************************************************//**
 * is_set will be false, if temperature output overflow
 * flag is present in status register.
 *
 * @param is_set True if the temperature output overflow flag is set.
 * False otherwise.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_is_temperature_output_overflow_flag_set(bool *is_set);

/***************************************************************************//**
 * is_set will be false, if thermistor output overflow
 * flag is present in status register.
 *
 * @param is_set True if the thermistor output overflow flag is set.
 * False otherwise.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_is_thermistor_output_overflow_flag_set(bool *is_set);

/***************************************************************************//**
 * Clears interrupt flag in the status register.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_clear_interrupt_flag(void);

/***************************************************************************//**
 * Clears temperature output overflow flag
 * in the status register.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_clear_temperature_output_overflow_flag(void);

/***************************************************************************//**
 * Clears thermistor output overflow flag
 * in status register.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_clear_thermistor_output_overflow_flag(void);

/***************************************************************************//**
 * Clears both thermistor and
 * temperature overflow flags in the status register,
 * but leaves interrupt flag untouched.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_clear_all_overflow(void);

/***************************************************************************//**
 * Clears all flags in the status register.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_clear_all_status_flags(void);

/***************************************************************************//**
 * Reads the interrupt table register.
 * is_set will be true if interrupt flag is set
 * for the specified pixel
 * For the pixel map check the amg88xx datasheet:
 * https://industrial.panasonic.com/cdbs/www-data/pdf/ADI8000/ADI8000C66.pdf
 *
 * @param pixel_number # of the pixel in order as in the registers (0-63)
 * @param is_set True, if the interrupt is set for the pixel determined by
 * pixel_number. False otherwise.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_is_pixel_interrupt_set(uint8_t pixelAddr, bool *is_set);

/***************************************************************************//**
 * Enable "Twice Moving Average".
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_enable_moving_average(void);

/***************************************************************************//**
 * Disable "Twice Moving Average".
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_disable_moving_average(void);

/***************************************************************************//**
 * is_enabled will be true, if "Twice Moving Average"
 * is enabled.
 *
 * @param is_enabled True, if moving average enabled.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_is_moving_average_enabled(bool *is_enabled);

/***************************************************************************//**
 * Sets upper interrupt temperature.
 * The temperature scale can be set globally with set_temperature_scale().
 *
 * @param degrees Desired value.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_set_upper_interrupt_value(float degrees);

/***************************************************************************//**
 * Sets upper interrupt temperature.
 *
 * @param register_value Desired raw temperature value.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_set_upper_interrupt_value_raw(int16_t register_value);

/***************************************************************************//**
 * Sets lower interrupt temperature.
 * The temperature scale can be set globally with set_temperature_scale().
 *
 * @param degrees Desired value.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_set_lower_interrupt_value(float degrees);

/***************************************************************************//**
 * Sets lower interrupt temperature.
 *
 * @param register_value Desired raw value.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_set_lower_interrupt_value_raw(int16_t register_value);

/***************************************************************************//**
 * Sets interrupt hysteresis temperature.
 * The temperature scale can be set globally with set_temperature_scale().
 *
 * @param degrees Desired value.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_set_interrupt_hysteresis(float degrees);

/***************************************************************************//**
 * Sets interrupt hysteresis temperature.
 *
 * @param register_value Raw desired value.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_set_interrupt_hysteresis_raw(int16_t register_value);

/***************************************************************************//**
 * Sets upper interrupt temperature.
 * The temperature scale can be set globally with set_temperature_scale().
 *
 * @param degrees Temperature value.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_get_upper_interrupt_value(float *degrees);

/***************************************************************************//**
 * Gets upper interrupt temperature.
 *
 * @param register_value Raw temperature value.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_get_upper_interrupt_value_raw(int16_t *register_value);

/***************************************************************************//**
 * Sets lower interrupt temperature.
 * The temperature scale can be set globally with set_temperature_scale().
 *
 * @param degrees Temperature value.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_get_lower_interrupt_value(float *degrees);

/***************************************************************************//**
 * Gets lower interrupt temperature.
 *
 * @param register_value Raw temperature value.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_get_lower_interrupt_value_raw(int16_t *register_value);

/***************************************************************************//**
 * Gets interrupt hysteresis temperature.
 * The temperature scale can be set globally with set_temperature_scale().
 *
 * @param degrees Temperature value.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_get_interrupt_hysteresis(float *degrees);

/***************************************************************************//**
 * Gets interrupt hysteresis temperature.
 *
 * @param register_value Raw temperature value.
 *
 * @returns The result of the I2C transaction.
 ******************************************************************************/
sl_status_t amg88xx_get_interrupt_hysteresis_raw(int16_t *register_value);

#ifdef __cplusplus
}
#endif

#endif // IR_ARRAY_AMG88XX_DRIVER_H_

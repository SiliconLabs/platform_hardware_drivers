/***************************************************************************//**
 * @file as7265x.c
 * @brief AS7265X Source file
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
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
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

#include "string.h"
#include "as7265x.h"
#include "as7265x_config.h"
#include "as7265x_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2C physical registers.
#define STATUS_REG                          0x00
#define WRITE_REG                           0x01
#define READ_REG                            0x02

// Mask of I²C slave interface STATUS
#define RX_VALID                            0x01
#define TX_VALID                            0x02

// Register addresses
#define HW_VERSION_HIGH_REG                 0x00
#define HW_VERSION_LOW_REG                  0x01
#define FW_VERSION_HIGH_REG                 0x02
#define FW_VERSION_LOW_REG                  0x03
#define CONFIG_REG                          0x04
#define INTERGRATION_TIME_REG               0x05
#define DEVICE_TEMP_REG                     0x06
#define LED_CONFIG_REG                      0x07

// Raw channel registers
#define R_G_A_REG                           0x08
#define S_H_B_REG                           0x0A
#define T_I_C_REG                           0x0C
#define U_J_D_REG                           0x0E
#define V_K_E_REG                           0x10
#define W_L_F_REG                           0x12

// Calibrated channel registers
#define R_G_A_CAL_REG                       0x14
#define S_H_B_CAL_REG                       0x18
#define T_I_C_CAL_REG                       0x1C
#define U_J_D_CAL_REG                       0x20
#define V_K_E_CAL_REG                       0x24
#define W_L_F_CAL_REG                       0x28

// Device select register.
#define DEV_SELECT_CONTROL                  0x4F

// Coefficient register.
#define COEF_DATA_0_REG                     0x50
#define COEF_DATA_1_REG                     0x51
#define COEF_DATA_2_REG                     0x52
#define COEF_DATA_3_REG                     0x53
#define COEF_DATA_READ_REG                  0x54
#define COEF_DATA_WRITE_REG                 0x55

// Amount of ms to wait between checking for virtual register changes
#define POLLING_DELAY                       5

// Default integration cycles.
#define DEFAULT_INTEGRATION_CYCLE           20

// Raw data channel register array.
static const uint8_t channel_register[6] = {R_G_A_REG, S_H_B_REG, T_I_C_REG,
                                            U_J_D_REG, V_K_E_REG, W_L_F_REG};

// Calibrated data channel register array.
static const uint8_t cal_channel_register[6] = {R_G_A_CAL_REG, S_H_B_CAL_REG,
                                                T_I_C_CAL_REG, U_J_D_CAL_REG,
                                                V_K_E_CAL_REG, W_L_F_CAL_REG};

// Device type array.
static const uint8_t device_type[3] = {AS72653_UV,
                                       AS72652_VISIBLE,
                                       AS72651_NIR};

// Max wait time
static uint32_t max_wait_time;

// Virtual register function
static sl_status_t read_virtual_register(uint8_t virtual_addr, uint8_t *pdata);
static sl_status_t write_virtual_register(uint8_t virtual_addr, uint8_t data);

// Select device (AS72651, AS72652, AS72653)
static sl_status_t select_device(as7265x_device_type_t device);

// Get raw color data of a channel.
static sl_status_t get_channel(uint8_t channel_register,
                               as7265x_device_type_t device,
                               uint16_t *color_data);

// Get calibrated color data of a channel.
static sl_status_t get_calibrated_value(uint8_t cal_address,
                                        as7265x_device_type_t device,
                                        float *cal_val);

/***************************************************************************//**
 * Initialize as7265x sensor
*******************************************************************************/
sl_status_t as7265x_init(void)
{
  uint8_t value = 0;
  sl_status_t sc = SL_STATUS_OK;

  // Init default max wait time.
  max_wait_time = (uint32_t)(DEFAULT_INTEGRATION_CYCLE * 2 * 2.8 * 2);

  // Check to see if both slaves are detected
  sc = read_virtual_register(DEV_SELECT_CONTROL, &value);

  if (((value & 0b00110000) == 0) && (sc == SL_STATUS_OK)) {
    // Test if Slave1 and 2 are detected. If not, fail.
    return SL_STATUS_FAIL;
  }

  sc |= as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_12_5MA,
                                 AS7265x_LED_WHITE);
  sc |= as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_12_5MA,
                                 AS7265x_LED_IR);
  sc |= as7265x_set_bulb_current(AS7265X_LED_CURRENT_LIMIT_12_5MA,
                                 AS7265x_LED_UV);

  // Turn off bulb to avoid heating sensor
  sc |= as7265x_disable_bulb(AS7265x_LED_WHITE);
  sc |= as7265x_disable_bulb(AS7265x_LED_IR);
  sc |= as7265x_disable_bulb(AS7265x_LED_UV);

  // 50 * 2.8ms = 140ms. 0 to 255 is valid.
  sc |= as7265x_set_integration_cycles(49);

  // Set to 8mA (maximum)
  sc |= as7265x_set_indicator_current(AS7265X_INDICATOR_CURRENT_LIMIT_8MA);
  sc |= as7265x_enable_indicator();

  // Set gain to 64x
  sc |= as7265x_set_gain(AS7265X_GAIN_64X);

  /* One-shot reading of VBGYOR. If you use Mode 2 or 3 then integration time
   * is double between readings.
   */
  sc |= as7265x_set_measurement_mode(AS7265X_MEASUREMENT_MODE_6CHAN_CONTINUOUS);

  sc |= as7265x_enable_interrupt();

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Get device type.
*******************************************************************************/
sl_status_t as7265x_get_device_type(uint8_t *dev_type)
{
  uint8_t data = 0;
  sl_status_t sc = SL_STATUS_OK;

  if (dev_type == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  } else {
    sc = read_virtual_register(HW_VERSION_HIGH_REG, &data);

    if (sc != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }

    *dev_type = data;

    return SL_STATUS_OK;
  }
}

/***************************************************************************//**
 * Get hardware version.
*******************************************************************************/
sl_status_t as7265x_get_hardware_version(uint8_t *hw_ver)
{
  uint8_t data = 0;
  sl_status_t sc = SL_STATUS_OK;

  if (hw_ver == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  } else {
    sc = read_virtual_register(HW_VERSION_LOW_REG, &data);

    if (sc != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }

    *hw_ver = data;

    return SL_STATUS_OK;
  }
}

/***************************************************************************//**
 * Get firmware version.
*******************************************************************************/
sl_status_t as7265x_get_firmware_version(as7265x_firmware_version_t *version)
{
  uint8_t data = 0;
  sl_status_t sc = SL_STATUS_OK;

  if (version == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  } else {
    // Set to 0x01 for Major
    sc |= write_virtual_register(FW_VERSION_HIGH_REG, 0x01);

    // Set to 0x01 for Major
    sc |= write_virtual_register(FW_VERSION_LOW_REG, 0x01);

    sc |= read_virtual_register(FW_VERSION_LOW_REG, &data);

    version->major_firmware_version = data;

    // Set to 0x02 for Patch
    sc |= write_virtual_register(FW_VERSION_HIGH_REG, 0x02);

    // Set to 0x02 for Patch
    sc |= write_virtual_register(FW_VERSION_LOW_REG, 0x02);

    sc |= read_virtual_register(FW_VERSION_LOW_REG, &data);

    version->patch_firmware_version = data;

    // Set to 0x03 for Build
    sc |= write_virtual_register(FW_VERSION_HIGH_REG, 0x03);

    // Set to 0x03 for Build
    sc |= write_virtual_register(FW_VERSION_LOW_REG, 0x03);

    sc |= read_virtual_register(FW_VERSION_LOW_REG, &data);

    version->build_firmware_version = data;

    if (sc != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }

    return SL_STATUS_OK;
  }
}

/***************************************************************************//**
 * Take measurements.
*******************************************************************************/
sl_status_t as7265x_take_measurements(void)
{
  sl_status_t sc = SL_STATUS_OK;
  bool is_data_ready = false;
  uint32_t start_time = as7265x_get_ms();

  // Wait for data to be ready
  while (1) {
    sc = as7265x_data_available(&is_data_ready);
    if (sc != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }

    if (is_data_ready == true) {
      break;
    }

    if ((as7265x_get_ms() - start_time)
       > max_wait_time) {
      // Sensor failed to respond
      return SL_STATUS_TIMEOUT;
    }

    as7265x_delay_ms(POLLING_DELAY);
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Take measurements with bulbs.
*******************************************************************************/
sl_status_t as7265x_take_measurements_with_bulb(void)
{
  sl_status_t sc = SL_STATUS_OK;

  sc |= as7265x_enable_bulb(AS7265x_LED_WHITE);
  sc |= as7265x_enable_bulb(AS7265x_LED_IR);
  sc |= as7265x_enable_bulb(AS7265x_LED_UV);

  sc |= as7265x_take_measurements();

  // Turn off bulb to avoid heating sensor
  sc |= as7265x_disable_bulb(AS7265x_LED_WHITE);
  sc |= as7265x_disable_bulb(AS7265x_LED_IR);
  sc |= as7265x_disable_bulb(AS7265x_LED_UV);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Get all color readings.
*******************************************************************************/
sl_status_t as7265x_get_all_color_channel(as7265x_color_data_t *color_data)
{
  sl_status_t sc = SL_STATUS_OK;
  uint8_t i = 0;
  uint8_t j = 0;
  uint16_t* color_channel = &color_data->channel[AS7265x_COLOR_CHANNEL_A];

  if (color_data != NULL) {
    for (i = 0; i < 3; i++) {
      for (j = 0; j < 6; j++) {
        sc |= get_channel(channel_register[j],
                          device_type[i],
                          color_channel);
        color_channel++;
      }
    }

    if (sc != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }
}

/***************************************************************************//**
 * Get raw color data of a channel.
*******************************************************************************/
sl_status_t as7265x_get_channel(as7265x_color_channel_t channel,
                                uint16_t *color_data)
{
  if ((!color_data) || (channel >= AS7265x_COLOR_CHANNEL_MAX)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return get_channel(channel_register[channel % 6],
                     device_type[channel / 6],
                     color_data);
}

/***************************************************************************//**
 * Get all calibrated value.
*******************************************************************************/
sl_status_t as7265x_get_all_calibrated_value(as7265x_calibrated_data_t *cal_val)
{
  sl_status_t sc = SL_STATUS_OK;
  uint8_t i = 0;
  uint8_t j = 0;
  float* calib_data_channel = &cal_val->channel[AS7265x_COLOR_CHANNEL_A];

  if (cal_val != NULL) {
    for (i = 0; i < 3; i++) {
      for (j = 0; j < 6; j++) {
        sc |= get_calibrated_value(cal_channel_register[j],
                                   device_type[i],
                                   calib_data_channel);
        calib_data_channel++;
      }
    }

    if (sc != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }
}

/***************************************************************************//**
 * Get calibrated data of a channel.
*******************************************************************************/
sl_status_t as7265x_get_calibrated_value(as7265x_color_channel_t channel,
                                         float *cal_val)
{
  if ((!cal_val) || (channel >= AS7265x_COLOR_CHANNEL_MAX)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  return get_calibrated_value(cal_channel_register[channel % 6],
                              device_type[channel / 6],
                              cal_val);
}

/***************************************************************************//**
 * Set measurement mode.
*******************************************************************************/
sl_status_t as7265x_set_measurement_mode(as7265x_device_mode_t mode)
{
  uint8_t value = 0;
  sl_status_t sc = SL_STATUS_OK;

  if (mode > 0b11) {
    // Error check
    mode = 0b11;
  }

  // Read
  sc |= read_virtual_register(CONFIG_REG, &value);
  // Clear BANK bits
  value &= 0b11110011;
  // Set BANK bits with user's choice.
  value |= (mode << 2);
  // Write
  sc |= write_virtual_register(CONFIG_REG, value);

  sc |= read_virtual_register(INTERGRATION_TIME_REG, &value);

  if ((mode == AS7265X_MEASUREMENT_MODE_6CHAN_CONTINUOUS)
      || (mode == AS7265X_MEASUREMENT_MODE_6CHAN_ONE_SHOT)) {
    max_wait_time = value * 2.8 * 2 * 2;
  }

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Set gain value.
*******************************************************************************/
sl_status_t as7265x_set_gain(as7265x_gain_t gain)
{
  uint8_t value = 0;
  sl_status_t sc = SL_STATUS_OK;

  if (gain > 0b11) {
    gain = 0b11;
  }

  // Read
  sc |= read_virtual_register(CONFIG_REG, &value);

  // Clear GAIN bits
  value &= 0b11001111; 

  // Set GAIN bits with user's choice
  value |= (gain << 4);

  // Write
  sc |= write_virtual_register(CONFIG_REG, value);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Sets the integration cycle amount.
*******************************************************************************/
sl_status_t as7265x_set_integration_cycles(uint8_t cycle_value)
{
  sl_status_t sc = SL_STATUS_OK;
  uint8_t mode = 0;

  // Read measurement mode
  sc |= read_virtual_register(CONFIG_REG, &mode);
  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  mode &= 0b00001100;
  mode >>= 2;

  if ((mode == AS7265X_MEASUREMENT_MODE_6CHAN_CONTINUOUS)
      || (mode == AS7265X_MEASUREMENT_MODE_6CHAN_ONE_SHOT)) {
    // Wait for integration time * 2 + 100%
    max_wait_time = (uint32_t)(cycle_value * 2 * 2.8 * 2);
  } else {
    // Wait for integration time + 100%
    max_wait_time = (uint32_t)(cycle_value * 2.8 * 2);
  }

  // Write
  return write_virtual_register(INTERGRATION_TIME_REG, cycle_value);
}

/***************************************************************************//**
 * Enable interrupt pin.
*******************************************************************************/
sl_status_t as7265x_enable_interrupt(void)
{
  uint8_t value = 0;
  sl_status_t sc = SL_STATUS_OK;

  // Read
  sc |= read_virtual_register(CONFIG_REG, &value);

  // Set INT bit
  value |= (1 << 6);

  // Write
  sc |= write_virtual_register(CONFIG_REG, value);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Disable interrupt pin.
*******************************************************************************/
sl_status_t as7265x_disable_interrupt(void)
{
  uint8_t value = 0;
  sl_status_t sc = SL_STATUS_OK;

  // Read
  sc |= read_virtual_register(CONFIG_REG, &value);

  // Clear INT bit
  value &= ~(1 << 6);

  // Write
  sc |= write_virtual_register(CONFIG_REG, value);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Check data available.
*******************************************************************************/
sl_status_t as7265x_data_available(bool *is_data_ready)
{
  sl_status_t sc = SL_STATUS_OK;  
  uint8_t value = 0;

  if (is_data_ready == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  } else {
    sc = read_virtual_register(CONFIG_REG, &value);

    // Bit 1 is DATA_RDY
    *is_data_ready = value & (1 << 1);

    return sc;
  }
}

/***************************************************************************//**
 * Enable bulb of given device.
*******************************************************************************/
sl_status_t as7265x_enable_bulb(as7265x_led_type_t led_type)
{
  uint8_t value = 0;
  sl_status_t sc = SL_STATUS_OK;

  sc |= select_device((as7265x_device_type_t)led_type);

  // Read
  sc |= read_virtual_register(LED_CONFIG_REG, &value);

  // Set the bit
  value |= (1 << 3);

  sc |= write_virtual_register(LED_CONFIG_REG, value);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Disable bulb of given device.
*******************************************************************************/
sl_status_t as7265x_disable_bulb(as7265x_led_type_t led_type)
{
  uint8_t value = 0;
  sl_status_t sc = SL_STATUS_OK;

  sc |= select_device((as7265x_device_type_t)led_type);

  // Read
  sc |= read_virtual_register(LED_CONFIG_REG, &value);

  // Clear the bit
  value &= ~(1 << 3);

  sc |= write_virtual_register(LED_CONFIG_REG, value);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Set bulb current.
*******************************************************************************/
sl_status_t as7265x_set_bulb_current(as7265x_led_current_t current,
                                     as7265x_led_type_t led_type)
{
  uint8_t value = 0;
  sl_status_t sc = SL_STATUS_OK;

  sc |= select_device((as7265x_device_type_t)led_type);

  // Set the current
  if (current > 0b11) {
    // Limit to two bits
    current = 0b11;                                        
  }

  // Read
  sc |= read_virtual_register(LED_CONFIG_REG, &value);

  // Clear ICL_DRV bits
  value &= 0b11001111;

  // Set ICL_DRV bits with user's choice
  value |= (current << 4);

  // Write
  sc |= write_virtual_register(LED_CONFIG_REG, value);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Select device.
*******************************************************************************/
static sl_status_t select_device(as7265x_device_type_t device)
{
  // Set the bits 0:1. Just overwrite whatever is there because masking in
  // the correct value doesn't work.
  return write_virtual_register(DEV_SELECT_CONTROL, device);
}

/***************************************************************************//**
 * Enable the onboard indicator LED.
*******************************************************************************/
sl_status_t as7265x_enable_indicator(void)
{
  uint8_t value = 0;
  sl_status_t sc = SL_STATUS_OK;

  sc |= select_device(AS72651_NIR);

  // Read
  sc |= read_virtual_register(LED_CONFIG_REG, &value);

  // Set the bit
  value |= (1 << 0);

  sc |= write_virtual_register(LED_CONFIG_REG, value);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Disable the onboard indicator LED.
*******************************************************************************/
sl_status_t as7265x_disable_indicator(void)
{
  uint8_t value = 0;
  sl_status_t sc = SL_STATUS_OK;

  sc |= select_device(AS72651_NIR);

  // Read
  sc |= read_virtual_register(LED_CONFIG_REG, &value);

  // Clear the bit
  value &= ~(1 << 0);

  sc |= write_virtual_register(LED_CONFIG_REG, value);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Set indicator current.
*******************************************************************************/
sl_status_t as7265x_set_indicator_current(
    as7265x_indicator_led_current_t current)
{
  uint8_t value = 0;
  sl_status_t sc = SL_STATUS_OK;

  sc |= select_device(AS72651_NIR);

  if (current > 0b11) {
    current = 0b11;
  }

  // Read
  sc |= read_virtual_register(LED_CONFIG_REG, &value);

  // Clear ICL_IND bits
  value &= 0b11111001;     

  // Set ICL_IND bits with user's choice
  value |= (current << 1);

  // Write
  sc |= write_virtual_register(LED_CONFIG_REG, value);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Get temperature.
*******************************************************************************/
sl_status_t as7265x_get_temperature(as7265x_device_type_t device,
                                    uint8_t *temp_val)
{
  uint8_t data = 0;
  sl_status_t sc = SL_STATUS_OK;

  if (temp_val != NULL) {
    sc |= select_device(device);

    sc |= read_virtual_register(DEVICE_TEMP_REG, &data);

    if (sc != SL_STATUS_OK) {
        return SL_STATUS_FAIL;
    }

    *temp_val = data;

    return SL_STATUS_OK;
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }
}

/***************************************************************************//**
 * Soft reset device.
*******************************************************************************/
sl_status_t as7265x_soft_reset(void)
{
  uint8_t value = 0;
  sl_status_t sc = SL_STATUS_OK;

  // Read
  sc |= read_virtual_register(CONFIG_REG, &value);
  // Set RST bit, automatically cleared after reset
  value |= (1 << 7);

  // Write
  sc |= write_virtual_register(CONFIG_REG, value);

  if (sc != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Change the active I2CSPM instance.
*******************************************************************************/
sl_status_t as7265x_set_i2cspm_instance(sl_i2cspm_t *i2cspm_instance)
{
  if (i2cspm_instance != NULL) {
    as7265x_platform_set_i2cspm_instance(i2cspm_instance);
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FAIL;
  }
}

/***************************************************************************//**
 * Get calibrated color data of a channel.
*******************************************************************************/
static sl_status_t get_calibrated_value(uint8_t cal_address,
                                        as7265x_device_type_t device,
                                        float *cal_val)
{
  sl_status_t sc = SL_STATUS_OK;
  uint8_t b0, b1, b2, b3;
  uint32_t cal_bytes = 0;

  sc |= select_device(device);

  sc |= read_virtual_register(cal_address + 0, &b0);
  sc |= read_virtual_register(cal_address + 1, &b1);
  sc |= read_virtual_register(cal_address + 2, &b2);
  sc |= read_virtual_register(cal_address + 3, &b3);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  // Channel calibrated values are stored big-endian
  cal_bytes |= ((uint32_t)b0 << (8 * 3));
  cal_bytes |= ((uint32_t)b1 << (8 * 2));
  cal_bytes |= ((uint32_t)b2 << (8 * 1));
  cal_bytes |= ((uint32_t)b3 << (8 * 0));

  // Convert bytes to float
  memcpy(cal_val, &cal_bytes, sizeof(float));
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Get raw color data of a channel.
*******************************************************************************/
static sl_status_t get_channel(uint8_t channel_register,
                               as7265x_device_type_t device,
                               uint16_t *color_data)
{
  sl_status_t sc = SL_STATUS_OK;
  uint8_t data = 0;

  sc |= select_device(device);

  // High uint8_t
  sc |= read_virtual_register(channel_register, &data);

  *color_data = data << 8;

  sc |= read_virtual_register(channel_register + 1, &data);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  *color_data |= data;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Write into virtual register.
*******************************************************************************/
static sl_status_t write_virtual_register(uint8_t virtual_addr, uint8_t data)
{
  uint8_t status = 0;
  sl_status_t sc = SL_STATUS_OK;
  uint32_t start_time = as7265x_get_ms();

  // Wait for WRITE register to be empty
  while (1)
  {
    if ((as7265x_get_ms() - start_time)
        > max_wait_time) {
      // Sensor failed to respond
      return SL_STATUS_TIMEOUT;
    }
    sc = as7265x_platform_read_register(AS7265X_ADDR, STATUS_REG, &status);
    if (sc != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
    if ((status & TX_VALID) == 0) {
      // No inbound TX pending at slave. Okay to write now.
      break;
    }
    as7265x_delay_ms(POLLING_DELAY);
  }

  // Send the virtual register address (setting bit 7 to indicate we are writing
  // to a register).
  sc = as7265x_platform_write_register(AS7265X_ADDR,
                                       WRITE_REG,
                                       (virtual_addr | 1 << 7));

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  // Wait for WRITE register to be empty
  start_time = as7265x_get_ms();
  while (1)
  {
    if ((as7265x_get_ms() - start_time)
        > max_wait_time) {
      // Sensor failed to respond
      return SL_STATUS_TIMEOUT;
    }

    sc = as7265x_platform_read_register(AS7265X_ADDR, STATUS_REG, &status);

    if (sc != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }

    if ((status & TX_VALID) == 0) {
      // No inbound TX pending at slave. Okay to write now.
      break;
    }
    as7265x_delay_ms(POLLING_DELAY);
  }

  // Send the data to complete the operation.
  sc = as7265x_platform_write_register(AS7265X_ADDR, WRITE_REG, data);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Read virtual register.
*******************************************************************************/
static sl_status_t read_virtual_register(uint8_t virtual_addr, uint8_t *pdata)
{
  uint8_t status = 0;
  sl_status_t sc = SL_STATUS_OK;
  uint8_t data = 0;
  uint32_t start_time = 0;

  // Do a prelim check of the read register
  sc = as7265x_platform_read_register(AS7265X_ADDR, STATUS_REG, &status);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  if ((status & RX_VALID) != 0) {
    // There is data to be read. Read the byte but do nothing with it
    sc = as7265x_platform_read_register(AS7265X_ADDR, READ_REG, &data);
    if (sc != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
  }

  // Wait for WRITE flag to clear
  start_time = as7265x_get_ms();
  while (1)
  {
    if ((as7265x_get_ms() - start_time)
        > max_wait_time) {
      // Sensor failed to respond
      return SL_STATUS_TIMEOUT;
    }
    sc = as7265x_platform_read_register(AS7265X_ADDR, STATUS_REG, &status);
    if (sc != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
    if ((status & TX_VALID) == 0) {
      // If TX bit is clear, it is ok to write
      break;
    }
    as7265x_delay_ms(POLLING_DELAY);
  }

  // Send the virtual register address (bit 7 should be 0 to indicate we are
  // reading a register).
  sc = as7265x_platform_write_register(AS7265X_ADDR, WRITE_REG, virtual_addr);

  // Wait for READ flag to be set
  start_time = as7265x_get_ms();

  while (1)
  {
    if ((as7265x_get_ms() - start_time)
        > max_wait_time) {
      // Sensor failed to respond
      return SL_STATUS_TIMEOUT;
    }

    sc = as7265x_platform_read_register(AS7265X_ADDR, STATUS_REG, &status);

    if (sc != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }

    if ((status & RX_VALID) != 0) {
      // Read data is ready.
      break;
    }

    as7265x_delay_ms(POLLING_DELAY);
  }

  sc = as7265x_platform_read_register(AS7265X_ADDR, READ_REG, pdata);

  if (sc != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}
#ifdef __cplusplus
}
#endif

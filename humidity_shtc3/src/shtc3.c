/***************************************************************************//**
 * @file shtc3.c
 * @brief Platform independent driver for shtc3 humidity sensor
 * @version 0.1.0
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The license of this software is Silicon Laboratories Inc.
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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "shtc3.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

static sl_i2cspm_t *shtc3_i2cspm_instance;
static bool shtc3_is_initialized = false;
static bool shtc3_is_done = true;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Local prototypes
static sl_status_t shtc3_write_command(shtc3_commands_t cmd);
static sl_status_t shtc3_check_crc(uint8_t data[],
                                   uint8_t nbr_of_bytes,
                                   uint8_t checksum);
static float shtc3_calc_temperature(uint16_t raw_value);
static float shtc3_calc_humidity(uint16_t raw_value);
static sl_status_t shtc3_i2c_write_blocking(uint8_t  *src,
                                            uint32_t len);
static sl_status_t shtc3_i2c_read_blocking(uint8_t *src, uint32_t len);
static sl_status_t shtc3_i2c_write_read_blocking(uint8_t  *src,
                                                 uint32_t len_to_send,
                                                 uint8_t  *dest,
                                                 uint32_t len_to_receive);

/** @endcond */

/***************************************************************************//**
 *    get the software version of the core driver.
 ******************************************************************************/
void get_version(shtc3_core_version_t *core_version)
{
  core_version->major = SHTC3_MAJOR_VERION;
  core_version->minor = SHTC3_MINOR_VERION;
  core_version->build = SHTC3_BUILD_VERION;
  core_version->revision = SHTC3_REVISION;
}

/***************************************************************************//**
 *    Initialize the SHTC3
 ******************************************************************************/
sl_status_t shtc3_init(sl_i2cspm_t *i2cspm)
{
  sl_status_t ret;
  uint16_t id;

  if (i2cspm == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // If already initialized, return status
  if (shtc3_is_initialized == true) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  // wait for sensor to power-up
  sl_udelay_wait(240);

  // Update i2cspm instance
  shtc3_i2cspm_instance = i2cspm;

  shtc3_is_initialized = true;

  ret = shtc3_wake();
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  ret = shtc3_get_id(&id);
  if (ret != SL_STATUS_OK) {
    return ret;
  }
  if (id != SHTC3_ID) {
    return SL_STATUS_FAIL;
  }

  ret = shtc3_sleep();
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  return SL_STATUS_OK;
}

sl_status_t shtc3_deinit(void)
{
  sl_status_t ret;
  // If already not initialized, return status
  if (shtc3_is_initialized == false) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  shtc3_is_initialized = false;

  ret = shtc3_wake();
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  ret = shtc3_reset();
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  shtc3_i2cspm_instance = NULL;

  return SL_STATUS_OK;
}

sl_status_t shtc3_sleep(void)
{
  return shtc3_write_command(SHTC3_SLEEP);
}

sl_status_t shtc3_wake(void)
{
  sl_status_t ret;

  ret = shtc3_write_command(SHTC3_WAKEUP);

  if (ret == SL_STATUS_OK) {
    sl_udelay_wait(240);
  }

  return ret;
}

sl_status_t shtc3_reset(void)
{
  sl_status_t ret;

  // write reset command
  ret = shtc3_write_command(SHTC3_SOFT_RESET);

  // wait for sensor to reset
  sl_udelay_wait(240);

  return ret;
}

sl_status_t shtc3_get_id(uint16_t *shtc3_id)
{
  if (shtc3_id == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  sl_status_t ret;
  uint8_t data_to_send[2];
  uint8_t data_to_receive[3];
  shtc3_commands_t cmd = SHTC3_READ_ID;

  if (shtc3_id == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  data_to_send[0] = ((cmd) >> 8);
  data_to_send[1] = (cmd & 0xFF);

  ret = shtc3_i2c_write_read_blocking(data_to_send, 2, data_to_receive, 3);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  // verify checksum for id
  ret = shtc3_check_crc(data_to_receive, 2, data_to_receive[2]);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  // combine the two bytes to a 16-bit value
  *shtc3_id = (uint16_t)((data_to_receive[0] << 8) | data_to_receive[1]);

  return SL_STATUS_OK;
}

sl_status_t sl_shtc3_get_temp_rh(shtc3_power_mode_t pwr_mode,
                                 shtc3_data_order_t order,
                                 bool stretch,
                                 bool check_crc,
                                 shtc3_sensor_data_t *data)
{
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sl_status_t ret;
  shtc3_commands_t cmd;
  uint8_t data_to_send[2];
  uint8_t data_to_receive[6];
  uint8_t command = ((pwr_mode << 2) | (order << 1) | (stretch));

  switch (command) {
  case 0:
    // power = low, T first, stretch disable
    cmd = SHTC3_MEAS_T_RH_NOCLOCKSTR_LPM;
    break;

  case 1:
    // power = low, T first, stretch enable
    cmd = SHTC3_MEAS_T_RH_CLOCKSTR_LPM;
    break;

  case 2:
    // power = low, RH first, stretch disable
    cmd = SHTC3_MEAS_RH_T_NOCLOCKSTR_LPM;
    break;

  case 3:
    // power = low, RH first, stretch enable
    cmd = SHTC3_MEAS_RH_T_CLOCKSTR_LPM;
    break;

  case 4:
    // power = normal, T first, stretch disable
    cmd = SHTC3_MEAS_T_RH_NOCLOCKSTR_NPM;
    break;

  case 5:
    // power = normal, T first, stretch enable
    cmd = SHTC3_MEAS_T_RH_CLOCKSTR_NPM;
    break;

  case 6:
    // power = normal, RH first, stretch disable
    cmd = SHTC3_MEAS_RH_T_NOCLOCKSTR_NPM;
    break;
  default:
    // power = normal, RH first, stretch enable
    cmd = SHTC3_MEAS_RH_T_CLOCKSTR_NPM;
  }

  data_to_send[0] = ((cmd) >> 8);
  data_to_send[1] = (cmd & 0xFF);

  if (stretch == true) {
    ret = shtc3_i2c_write_read_blocking(data_to_send, 2, data_to_receive, 6);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
  } else {
    ret = shtc3_i2c_write_blocking(data_to_send, 2);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
    do {
      ret = shtc3_i2c_read_blocking(data_to_receive, 6);
    } while (ret != SL_STATUS_OK);
  }

  if (check_crc == true) {
    // verify checksum for temperature
    ret = shtc3_check_crc(data_to_receive, 2, data_to_receive[2]);
    if (ret != SL_STATUS_OK) {
      return ret;
    }

    // verify checksum for humidity
    ret = shtc3_check_crc(&data_to_receive[3], 2, data_to_receive[5]);
    if (ret != SL_STATUS_OK) {
      return ret;
    }

    if (order == SHTC3_TEMP_FIRST) {
      // combine the two bytes to a 16-bit value
      data->rh_t_data_raw.raw_temp = \
        ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];
      data->rh_t_data_raw.raw_rh = \
        ((uint16_t)data_to_receive[3] << 8) | (uint16_t)data_to_receive[4];
    } else {
      // combine the two bytes to a 16-bit value
      data->rh_t_data_raw.raw_rh = \
        ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];
      data->rh_t_data_raw.raw_temp = \
        ((uint16_t)data_to_receive[3] << 8) | (uint16_t)data_to_receive[4];
    }

    // if no error, calculate temperature in C degree and humidity in %RH
    data->rh_t_data.calculated_temp = shtc3_calc_temperature(
      data->rh_t_data_raw.raw_temp);
    data->rh_t_data.calculated_rh = shtc3_calc_humidity(
      data->rh_t_data_raw.raw_rh);
  } else {
    if (order == SHTC3_TEMP_FIRST) {
      // combine the two bytes to a 16-bit value
      data->rh_t_data_raw.raw_temp = \
        ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];
      data->rh_t_data_raw.raw_rh = \
        ((uint16_t)data_to_receive[3] << 8) | (uint16_t)data_to_receive[4];
    } else {
      // combine the two bytes to a 16-bit value
      data->rh_t_data_raw.raw_rh = \
        ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];
      data->rh_t_data_raw.raw_temp = \
        ((uint16_t)data_to_receive[3] << 8) | (uint16_t)data_to_receive[4];
    }
    // if no error, calculate temperature in C degree and humidity in %RH
    data->rh_t_data.calculated_temp = shtc3_calc_temperature(
      data->rh_t_data_raw.raw_temp);
    data->rh_t_data.calculated_rh = shtc3_calc_humidity(
      data->rh_t_data_raw.raw_rh);
  }

  shtc3_is_done = true;
  return SL_STATUS_OK;
}

sl_status_t sl_shtc3_get_temp_rh_non_blocking(shtc3_power_mode_t pwr_mode,
                                              shtc3_data_order_t order,
                                              bool check_crc,
                                              shtc3_sensor_data_t *data)
{
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sl_status_t ret;
  shtc3_commands_t cmd;
  uint8_t data_to_send[2];
  uint8_t data_to_receive[6];
  uint8_t command = ((pwr_mode << 2) | (order << 1));

  if (shtc3_is_done == true) {
    switch (command) {
    case 0:
      // power = low, T first, stretch disable
      cmd = SHTC3_MEAS_T_RH_NOCLOCKSTR_LPM;
      break;

    case 2:
      // power = low, RH first, stretch disable
      cmd = SHTC3_MEAS_RH_T_NOCLOCKSTR_LPM;
      break;

    case 4:
      // power = normal, T first, stretch disable
      cmd = SHTC3_MEAS_T_RH_NOCLOCKSTR_NPM;
      break;

    default:
      // power = normal, RH first, stretch disable
      cmd = SHTC3_MEAS_RH_T_NOCLOCKSTR_NPM;
    }

    data_to_send[0] = ((cmd) >> 8);
    data_to_send[1] = (cmd & 0xFF);

    ret = shtc3_i2c_write_blocking(data_to_send, 2);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
  }

  ret = shtc3_i2c_read_blocking(data_to_receive, 6);
  if (ret != SL_STATUS_OK) {
    shtc3_is_done = false;
    return SL_STATUS_BUSY;
  }

  if (check_crc == true) {
    // verify checksum for temperature
    ret = shtc3_check_crc(data_to_receive, 2, data_to_receive[2]);
    if (ret != SL_STATUS_OK) {
      return ret;
    }

    // verify checksum for humidity
    ret = shtc3_check_crc(&data_to_receive[3], 2, data_to_receive[5]);
    if (ret != SL_STATUS_OK) {
      return ret;
    }

    if (order == SHTC3_TEMP_FIRST) {
      // combine the two bytes to a 16-bit value
      data->rh_t_data_raw.raw_temp = \
        ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];
      data->rh_t_data_raw.raw_rh = \
        ((uint16_t)data_to_receive[3] << 8) | (uint16_t)data_to_receive[4];
    } else {
      // combine the two bytes to a 16-bit value
      data->rh_t_data_raw.raw_rh = \
        ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];
      data->rh_t_data_raw.raw_temp = \
        ((uint16_t)data_to_receive[3] << 8) | (uint16_t)data_to_receive[4];
    }

    // if no error, calculate temperature in C degree and humidity in %RH
    data->rh_t_data.calculated_temp = shtc3_calc_temperature(
      data->rh_t_data_raw.raw_temp);
    data->rh_t_data.calculated_rh = shtc3_calc_humidity(
      data->rh_t_data_raw.raw_rh);
  } else {
    if (order == SHTC3_TEMP_FIRST) {
      // combine the two bytes to a 16-bit value
      data->rh_t_data_raw.raw_temp = \
        ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];
      data->rh_t_data_raw.raw_rh = \
        ((uint16_t)data_to_receive[3] << 8) | (uint16_t)data_to_receive[4];
    } else {
      // combine the two bytes to a 16-bit value
      data->rh_t_data_raw.raw_rh = \
        ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];
      data->rh_t_data_raw.raw_temp = \
        ((uint16_t)data_to_receive[3] << 8) | (uint16_t)data_to_receive[4];
    }
    // if no error, calculate temperature in C degree and humidity in %RH
    data->rh_t_data.calculated_temp = shtc3_calc_temperature(
      data->rh_t_data_raw.raw_temp);
    data->rh_t_data.calculated_rh = shtc3_calc_humidity(
      data->rh_t_data_raw.raw_rh);
  }

  shtc3_is_done = true;

  return SL_STATUS_OK;
}

sl_status_t sl_shtc3_get_temp(shtc3_power_mode_t pwr_mode,
                              bool stretch,
                              bool check_crc,
                              shtc3_sensor_data_t *data)
{
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sl_status_t ret;
  shtc3_commands_t cmd;
  uint8_t data_to_send[2];
  uint8_t data_to_receive[3];
  if ((pwr_mode == true) && (stretch == true)) {
    cmd = SHTC3_MEAS_T_RH_CLOCKSTR_NPM;
  } else if ((pwr_mode == true) && (stretch == false)) {
    cmd = SHTC3_MEAS_T_RH_NOCLOCKSTR_NPM;
  } else if ((pwr_mode == false) && (stretch == true)) {
    cmd = SHTC3_MEAS_T_RH_CLOCKSTR_LPM;
  } else {
    cmd = SHTC3_MEAS_T_RH_NOCLOCKSTR_LPM;
  }

  data_to_send[0] = ((cmd) >> 8);
  data_to_send[1] = (cmd & 0xFF);

  if (stretch == true) {
    ret =
      shtc3_i2c_write_read_blocking(data_to_send, 2, data_to_receive,
                                    (2 + (uint8_t)check_crc));
    if (ret != SL_STATUS_OK) {
      return ret;
    }
  } else {
    ret = shtc3_i2c_write_blocking(data_to_send, 2);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
    do {
      ret = shtc3_i2c_read_blocking(data_to_receive, (2 + (uint8_t)check_crc));
    } while (ret != SL_STATUS_OK);
  }

  if (check_crc == true) {
    // verify checksum for temperature
    ret = shtc3_check_crc(data_to_receive, 2, data_to_receive[2]);
    if (ret != SL_STATUS_OK) {
      return ret;
    }

    // combine the two bytes to a 16-bit value
    data->rh_t_data_raw.raw_temp = \
      ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];

    // if no error, calculate temperature in C degree and humidity in %RH
    data->rh_t_data.calculated_temp = shtc3_calc_temperature(
      data->rh_t_data_raw.raw_temp);
  } else {
    data->rh_t_data_raw.raw_temp = \
      ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];

    // if no error, calculate temperature in C degree and humidity in %RH
    data->rh_t_data.calculated_temp = shtc3_calc_temperature(
      data->rh_t_data_raw.raw_temp);
  }

  return SL_STATUS_OK;
}

sl_status_t sl_shtc3_get_temp_non_blocking(shtc3_power_mode_t pwr_mode,
                                           bool check_crc,
                                           shtc3_sensor_data_t *data)
{
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sl_status_t ret;
  shtc3_commands_t cmd;
  uint8_t data_to_send[2];
  uint8_t data_to_receive[3];

  if (shtc3_is_done == true) {
    if (pwr_mode == SHTC3_NORMAL_POWER_MODE) {
      cmd = SHTC3_MEAS_T_RH_NOCLOCKSTR_NPM;
    } else {
      cmd = SHTC3_MEAS_T_RH_NOCLOCKSTR_LPM;
    }

    data_to_send[0] = ((cmd) >> 8);
    data_to_send[1] = (cmd & 0xFF);

    ret = shtc3_i2c_write_blocking(data_to_send, 2);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
  }

  ret = shtc3_i2c_read_blocking(data_to_receive, (2 + (uint8_t)check_crc));
  if (ret != SL_STATUS_OK) {
    shtc3_is_done = false;
    return SL_STATUS_BUSY;
  }

  if (check_crc == true) {
    // verify checksum for temperature
    ret = shtc3_check_crc(data_to_receive, 2, data_to_receive[2]);
    if (ret != SL_STATUS_OK) {
      return ret;
    }

    // combine the two bytes to a 16-bit value
    data->rh_t_data_raw.raw_temp = \
      ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];

    // if no error, calculate temperature in C degree and humidity in %RH
    data->rh_t_data.calculated_temp = shtc3_calc_temperature(
      data->rh_t_data_raw.raw_temp);
  } else {
    data->rh_t_data_raw.raw_temp = \
      ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];

    // if no error, calculate temperature in C degree and humidity in %RH
    data->rh_t_data.calculated_temp = shtc3_calc_temperature(
      data->rh_t_data_raw.raw_temp);
  }

  shtc3_is_done = true;
  return SL_STATUS_OK;
}

sl_status_t sl_shtc3_get_rh(shtc3_power_mode_t pwr_mode,
                            bool stretch,
                            bool check_crc,
                            shtc3_sensor_data_t *data)
{
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sl_status_t ret;
  shtc3_commands_t cmd;
  uint8_t data_to_send[2];
  uint8_t data_to_receive[3];
  if ((pwr_mode == true) && (stretch == true)) {
    cmd = SHTC3_MEAS_RH_T_CLOCKSTR_NPM;
  } else if ((pwr_mode == true) && (stretch == false)) {
    cmd = SHTC3_MEAS_RH_T_NOCLOCKSTR_NPM;
  } else if ((pwr_mode == false) && (stretch == true)) {
    cmd = SHTC3_MEAS_RH_T_CLOCKSTR_LPM;
  } else {
    cmd = SHTC3_MEAS_RH_T_NOCLOCKSTR_LPM;
  }

  data_to_send[0] = ((cmd) >> 8);
  data_to_send[1] = (cmd & 0xFF);

  if (stretch == true) {
    ret =
      shtc3_i2c_write_read_blocking(data_to_send, 2, data_to_receive,
                                    (2 + (uint8_t)check_crc));
    if (ret != SL_STATUS_OK) {
      return ret;
    }
  } else {
    ret = shtc3_i2c_write_blocking(data_to_send, 2);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
    do {
      ret = shtc3_i2c_read_blocking(data_to_receive, (2 + (uint8_t)check_crc));
    } while (ret != SL_STATUS_OK);
  }

  if (check_crc == true) {
    // verify checksum for temperature
    ret = shtc3_check_crc(data_to_receive, 2, data_to_receive[2]);
    if (ret != SL_STATUS_OK) {
      return ret;
    }

    // combine the two bytes to a 16-bit value
    data->rh_t_data_raw.raw_rh = \
      ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];

    // if no error, calculate temperature in C degree and humidity in %RH
    data->rh_t_data.calculated_rh = shtc3_calc_humidity(
      data->rh_t_data_raw.raw_rh);
  } else {
    data->rh_t_data_raw.raw_rh = \
      ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];

    // if no error, calculate temperature in C degree and humidity in %RH
    data->rh_t_data.calculated_rh = shtc3_calc_humidity(
      data->rh_t_data_raw.raw_rh);
  }

  shtc3_is_done = true;
  return SL_STATUS_OK;
}

sl_status_t sl_shtc3_get_rh_non_blocking(shtc3_power_mode_t pwr_mode,
                                         bool check_crc,
                                         shtc3_sensor_data_t *data)
{
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sl_status_t ret;
  shtc3_commands_t cmd;
  uint8_t data_to_send[2];
  uint8_t data_to_receive[3];

  if (shtc3_is_done == true) {
    if (pwr_mode == SHTC3_NORMAL_POWER_MODE) {
      cmd = SHTC3_MEAS_RH_T_NOCLOCKSTR_NPM;
    } else {
      cmd = SHTC3_MEAS_RH_T_NOCLOCKSTR_LPM;
    }

    data_to_send[0] = ((cmd) >> 8);
    data_to_send[1] = (cmd & 0xFF);

    ret = shtc3_i2c_write_blocking(data_to_send, 2);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
  }

  ret = shtc3_i2c_read_blocking(data_to_receive, (2 + (uint8_t)check_crc));
  if (ret != SL_STATUS_OK) {
    shtc3_is_done = false;
    return SL_STATUS_BUSY;
  }

  if (check_crc == true) {
    // verify checksum for temperature
    ret = shtc3_check_crc(data_to_receive, 2, data_to_receive[2]);
    if (ret != SL_STATUS_OK) {
      return ret;
    }

    // combine the two bytes to a 16-bit value
    data->rh_t_data_raw.raw_rh = \
      ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];

    // if no error, calculate temperature in C degree and humidity in %RH
    data->rh_t_data.calculated_rh = shtc3_calc_humidity(
      data->rh_t_data_raw.raw_rh);
  } else {
    data->rh_t_data_raw.raw_rh = \
      ((uint16_t)data_to_receive[0] << 8) | (uint16_t)data_to_receive[1];

    // if no error, calculate temperature in C degree and humidity in %RH
    data->rh_t_data.calculated_rh = shtc3_calc_humidity(
      data->rh_t_data_raw.raw_rh);
  }

  shtc3_is_done = true;

  return SL_STATUS_OK;
}

// Block write to shtc3
static sl_status_t shtc3_i2c_write_blocking(uint8_t *src, uint32_t len)
{
  I2C_TransferSeq_TypeDef seq;

  seq.addr = SHTC3_DEFAULT_ADDRESS << 1;
  seq.flags = I2C_FLAG_WRITE;

  // Write buffer
  seq.buf[0].data = src;
  seq.buf[0].len = len;

  if (I2CSPM_Transfer(shtc3_i2cspm_instance, &seq) != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }

  return SL_STATUS_OK;
}

static sl_status_t shtc3_i2c_read_blocking(uint8_t *src, uint32_t len)
{
  I2C_TransferSeq_TypeDef seq;

  seq.addr = SHTC3_DEFAULT_ADDRESS << 1;
  seq.flags = I2C_FLAG_READ;

  // read buffer
  seq.buf[0].data = src;
  seq.buf[0].len = len;

  if (I2CSPM_Transfer(shtc3_i2cspm_instance, &seq) != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }

  return SL_STATUS_OK;
}

// Block write and read to shtc3
static sl_status_t shtc3_i2c_write_read_blocking(uint8_t  *src,
                                                 uint32_t len_to_send,
                                                 uint8_t  *dest,
                                                 uint32_t len_to_receive)
{
  I2C_TransferSeq_TypeDef seq;

  seq.addr = SHTC3_DEFAULT_ADDRESS << 1;
  seq.flags = I2C_FLAG_WRITE_READ;

  // Write , read buffer
  seq.buf[0].data = src;
  seq.buf[0].len = len_to_send;

  seq.buf[1].data = dest;
  seq.buf[1].len = len_to_receive;

  if (I2CSPM_Transfer(shtc3_i2cspm_instance, &seq) != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/**************************************************************************//**
 * Write a command to the shtc3 sensor.
 *****************************************************************************/
static sl_status_t shtc3_write_command(shtc3_commands_t cmd)
{
  sl_status_t ret;

  uint8_t data_to_send[2];
  data_to_send[0] = (cmd >> 8);
  data_to_send[1] = (cmd & 0xFF);

  ret = shtc3_i2c_write_blocking(data_to_send, 2);

  return ret;
}

/**************************************************************************//**
*    check CRC function
******************************************************************************/
static sl_status_t shtc3_check_crc(uint8_t data[],
                                   uint8_t nbr_of_bytes,
                                   uint8_t checksum)
{
  uint8_t bit;        // bit mask
  uint8_t crc = 0xFF; // calculated checksum
  uint8_t byte_ctr;    // byte counter

  // calculates 8-Bit checksum with given polynomial
  for (byte_ctr = 0; byte_ctr < nbr_of_bytes; byte_ctr++) {
    crc ^= (data[byte_ctr]);
    for (bit = 8; bit > 0; --bit) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ CRC_POLYNOMIAL;
      } else {
        crc = (crc << 1);
      }
    }
  }
  // verify checksum
  if (crc != checksum) {
    return SL_STATUS_FAIL;
  } else {
    return SL_STATUS_OK;
  }
}

/**************************************************************************//**
 *   calculate the temperature from raw value
 *****************************************************************************/
static float shtc3_calc_temperature(uint16_t raw_value)
{
  // calculate temperature [�C]
  // T = -45 + 175 * rawValue / 2^16
  return (float)(175 * (float)raw_value / 65536.0f - 45.0f);
}

/**************************************************************************//**
 *   calculate humidity from the raw value
 *****************************************************************************/
static float shtc3_calc_humidity(uint16_t raw_value)
{
  // calculate relative humidity [%RH]
  // RH = rawValue / 2^16 * 100
  return (float)(100 * (float)raw_value / 65536.0f);
}

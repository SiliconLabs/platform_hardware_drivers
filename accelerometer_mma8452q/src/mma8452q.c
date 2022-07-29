/***************************************************************************//**
 * @file mma8452q.c
 * @brief MMA8452Q Source file
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

#include <mma8452q_config.h>
#include <mma8452q_i2c.h>
#include <mma8452q.h>
#include <string.h>
#include <sl_udelay.h>

/***************************************************************************//**
 * Definition
 ******************************************************************************/
// MMA8452Q Register Definitions //
#define  MMA8452Q_STATUS            0x00
#define  MMA8452Q_OUT_X_MSB         0x01
#define  MMA8452Q_OUT_X_LSB         0x02
#define  MMA8452Q_OUT_Y_MSB         0x03
#define  MMA8452Q_OUT_Y_LSB         0x04
#define  MMA8452Q_OUT_Z_MSB         0x05
#define  MMA8452Q_OUT_Z_LSB         0x06
#define  MMA8452Q_SYSMOD            0x0B
#define  MMA8452Q_INT_SOURCE        0x0C
#define  MMA8452Q_WHO_AM_I          0x0D
#define  MMA8452Q_XYZ_DATA_CFG      0x0E
#define  MMA8452Q_HP_FILTER_CUTOFF  0x0F
#define  MMA8452Q_PL_STATUS         0x10
#define  MMA8452Q_PL_CFG            0x11
#define  MMA8452Q_PL_COUNT          0x12
#define  MMA8452Q_PL_BF_ZCOMP       0x13
#define  MMA8452Q_P_L_THS_REG       0x14
#define  MMA8452Q_FF_MT_CFG         0x15
#define  MMA8452Q_FF_MT_SRC         0x16
#define  MMA8452Q_FF_MT_THS         0x17
#define  MMA8452Q_FF_MT_COUNT       0x18
#define  MMA8452Q_TRANSIENT_CFG     0x1D
#define  MMA8452Q_TRANSIENT_SRC     0x1E
#define  MMA8452Q_TRANSIENT_THS     0x1F
#define  MMA8452Q_TRANSIENT_COUNT   0x20
#define  MMA8452Q_PULSE_CFG         0x21
#define  MMA8452Q_PULSE_SRC         0x22
#define  MMA8452Q_PULSE_THSX        0x23
#define  MMA8452Q_PULSE_THSY        0x24
#define  MMA8452Q_PULSE_THSZ        0x25
#define  MMA8452Q_PULSE_TMLT        0x26
#define  MMA8452Q_PULSE_LTCY        0x27
#define  MMA8452Q_PULSE_WIND        0x28
#define  MMA8452Q_ASLP_COUNT        0x29
#define  MMA8452Q_CTRL_REG1         0x2A
#define  MMA8452Q_CTRL_REG2         0x2B
#define  MMA8452Q_CTRL_REG3         0x2C
#define  MMA8452Q_CTRL_REG4         0x2D
#define  MMA8452Q_CTRL_REG5         0x2E
#define  MMA8452Q_OFF_X             0x2F
#define  MMA8452Q_OFF_Y             0x30
#define  MMA8452Q_OFF_Z             0x31

/***************************************************************************//**
 * Local Variables
 ******************************************************************************/
static mma8452q_sensor_config_t mma8452q_cfg = {
  .dev_addr = SL_MMA8452Q_I2C_BUS_ADDRESS1,
  .scale = MMA8452Q_SCALE_2G,
  .odr = MMA8452Q_ODR_800,
  .alsp_rate = MMA8452Q_ASLP_ODR_50,
  .en_low_noise = false,
  .en_fast_read = false,
  .active_mode_pwr = MMA8452Q_NORMAL,
  .slp_mode_pwr = MMA8452Q_NORMAL,
  .aslp_cfg = {
    .alsp_count = 0x00
  },
  .trans_cfg = {
    .en_event_latch = false,
    .en_z_trans = false,
    .en_y_trans = false,
    .en_x_trans = false,
    .en_hpf_bypass = false,
    .db_cnt_mode = false,
    .threshold = 0x00,
    .debounce_cnt = 0x00
  },
  .orient_cfg = {
    .db_cnt_mode = true,
    .en_event_latch = false,
    .debounce_cnt = 0x00
  },
  .pulse_cfg = {
    .double_abort = false,
    .en_event_latch = false,
    .en_z_double = false,
    .en_z_single = false,
    .en_y_double = false,
    .en_y_single = false,
    .en_x_double = false,
    .en_x_single = false,
    .pulse_thresh_z = 0x00,
    .pulse_thresh_y = 0x00,
    .pulse_thresh_X = 0x00,
    .pulse_time_lmt = 0x00,
    .pulse_latency = 0x00,
    .pulse_window = 0x00
  },
  .ff_mt_cfg = {
    .en_event_latch = false,
    .ff_mt_sel = false,
    .en_z_trans = false,
    .en_y_trans = false,
    .en_x_trans = false,
    .db_cnt_mode = false,
    .threshold = 0x00,
    .debounce_cnt = 0x00
  },
  .interrupt_cfg = {
    .en_trans_wake = false,
    .en_orientation_wake = false,
    .en_pulse_wake = false,
    .en_ff_mt_wake = false,
    .int_active_hi = false,
    .int_open_drain = false,
    .en_aslp_int = false,
    .en_trans_int = false,
    .en_orientation_int = false,
    .en_pulse_int = false,
    .en_ff_mt_int = false,
    .en_drdy_int = false,
    .cfg_aslp_int = false,
    .cfg_trans_int = false,
    .cfg_orientation_int = false,
    .cfg_pulse_int = false,
    .cfg_ff_mt_int = false,
    .cfg_drdy_int = false
  },
  .enable = true
}; /* Structure to hold MMA8452Q driver config */
static sl_i2cspm_t *_mma8452q_i2cspm_instance = MMA8452Q_CONFIG_I2C;
static bool mma8452q_is_initialized = false;

/***************************************************************************//**
* Return the version information of the MMA8452Q.
*******************************************************************************/
void mma8452q_get_core_version(mma8542q_core_version_t *core_version)
{
  core_version->major = MMA8452Q_MAJOR_VERSION;
  core_version->build = MMA8452Q_BUILD_VERSION;
  core_version->minor = MMA8452Q_MINOR_VERSION;
  core_version->revision = MMA8452Q_REVISION_VERSION;
}

/***************************************************************************//**
* Initialize the MMA8452Q.
*******************************************************************************/
sl_status_t mma8452q_init(sl_i2cspm_t *i2cspm)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t temp = 0;
  // If already initialized, return status
  if (mma8452q_is_initialized == true) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  // Check for Null pointer
  if (i2cspm == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Update i2cspm instance
  _mma8452q_i2cspm_instance = i2cspm;

  // Check WHO_AM_I register
  status = mma8452q_read_register(MMA8452Q_WHO_AM_I, &temp);
  if (status == SL_STATUS_TRANSMIT) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (temp != SL_MMA8452Q_DEVICE_ID) {
    return SL_STATUS_NOT_FOUND;
  }

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  mma8452q_is_initialized = true;

  return SL_STATUS_OK;
}

/***************************************************************************//**
* De-initialize the MMAQ8452Q.
*******************************************************************************/
sl_status_t mma8452q_deinit(void)
{
  sl_status_t status = SL_STATUS_OK;

  if (mma8452q_is_initialized == false) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  // Reset device
  status = mma8452q_write_register(MMA8452Q_CTRL_REG2, 0x40);

  // Must to wait after reset device
  sl_udelay_wait(1000);

  status |= mma8452q_active(false);
  mma8452q_is_initialized = false;

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Set i2cspm instance
*******************************************************************************/
sl_status_t mma8452q_set_address(uint8_t i2c_address)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t temp = 0;
  uint8_t prev_addr;

  if ((i2c_address != SL_MMA8452Q_I2C_BUS_ADDRESS1)
      && (i2c_address != SL_MMA8452Q_I2C_BUS_ADDRESS2)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  prev_addr = mma8452q_cfg.dev_addr;
  mma8452q_cfg.dev_addr = i2c_address;
  status |= mma8452q_read_register(MMA8452Q_WHO_AM_I, &temp);

  if (status == SL_STATUS_TRANSMIT) {
    mma8452q_cfg.dev_addr = prev_addr;
    return status;
  }
  if (temp != SL_MMA8452Q_DEVICE_ID) {
    mma8452q_cfg.dev_addr = prev_addr;
    return SL_STATUS_NOT_FOUND;
  }

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Set active or standby mode
*******************************************************************************/
sl_status_t mma8452q_active(bool enable)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t temp;

  status |= mma8452q_read_register(MMA8452Q_CTRL_REG1, &temp);

  // active mode
  if (enable == true) {
    status |= mma8452q_write_register(MMA8452Q_CTRL_REG1, temp | (0x01));
    mma8452q_cfg.enable = true;
  }
  // standby mode
  else {
    status |= mma8452q_write_register(MMA8452Q_CTRL_REG1, temp & ~(0x01));
    mma8452q_cfg.enable = false;
  }

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Auto calibration method
*******************************************************************************/
sl_status_t mma8452q_auto_calibrate(void)
{
  sl_status_t status = SL_STATUS_OK;
  int16_t data[3];
  uint8_t offset[3];
  mma8452q_odr_t prev_odr = mma8452q_cfg.odr;
  mma8452q_scale_t prev_scale = mma8452q_cfg.scale;
  uint8_t is_data_ready = 0;

  status |= mma8452q_set_scale(MMA8452Q_SCALE_2G);
  status |= mma8452q_set_odr(MMA8452Q_ODR_200);

  if (SL_STATUS_OK != status) {
    return SL_STATUS_TRANSMIT;
  }

  // Wait for data status is ready
  do {
    status = mma8452q_check_for_data_ready(&is_data_ready);
    if (SL_STATUS_OK != status) {
      return status;
    }
  } while (!is_data_ready);

  status |= sl_mma8452q_get_acceleration(data);
  offset[0] = (uint8_t)((((int16_t)data[0] / 2) * (-1)) >> 4);
  offset[1] = (uint8_t)((((int16_t)data[1] / 2) * (-1)) >> 4);
  offset[2] = (uint8_t)(((65536 - (int16_t)data[1] / 2) * (-1)) >> 4);

  mma8452q_active(false);
  status |= mma8452q_write_register(MMA8452Q_OFF_X, offset[0]);
  status |= mma8452q_write_register(MMA8452Q_OFF_Y, offset[1]);
  status |= mma8452q_write_register(MMA8452Q_OFF_Z, offset[2]);
  mma8452q_active(true);

  status |= mma8452q_set_scale(prev_scale);
  status |= mma8452q_set_odr(prev_odr);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Return raw acceleration data computed using MMA8452Q OUT_X,Y,Z
*******************************************************************************/
sl_status_t sl_mma8452q_get_acceleration(int16_t avec[3])
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t raw_data[6]; // store x, y, z here

  if (mma8452q_cfg.en_fast_read == false) {
    status |= mma8452q_read_block(MMA8452Q_OUT_X_MSB, 6, raw_data);
    avec[0] = ((int16_t)(raw_data[0] << 8 | raw_data[1]));
    avec[1] = ((int16_t)(raw_data[2] << 8 | raw_data[3]));
    avec[2] = ((int16_t)(raw_data[4] << 8 | raw_data[5]));
  } else {
    status |= mma8452q_read_block(MMA8452Q_OUT_X_MSB, 3, raw_data);
    avec[0] = (int16_t)raw_data[0] << 8;
    avec[1] = (int16_t)raw_data[1] << 8;
    avec[2] = (int16_t)raw_data[2] << 8;
  }

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Return Calculated acceleration data computed using MMA8452Q OUT_X,Y,Z
*******************************************************************************/
sl_status_t sl_mma8452q_get_calculated_acceleration(float avec[3])
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t raw_data[6]; // store x, y, z here

  if (mma8452q_cfg.en_fast_read == false) {
    int16_t acceleration[3]; // store raw data of acceleration here
    status |= mma8452q_read_block(MMA8452Q_OUT_X_MSB, 6, raw_data);
    acceleration[0] = ((int16_t)(raw_data[0] << 8 | raw_data[1])) >> 4;
    acceleration[1] = ((int16_t)(raw_data[2] << 8 | raw_data[3])) >> 4;
    acceleration[2] = ((int16_t)(raw_data[4] << 8 | raw_data[5])) >> 4;
    avec[0] = (float)acceleration[0]
              / (float)(1 << 11) * (float)(mma8452q_cfg.scale);
    avec[1] = (float)acceleration[1]
              / (float)(1 << 11) * (float)(mma8452q_cfg.scale);
    avec[2] = (float)acceleration[2]
              / (float)(1 << 11) * (float)(mma8452q_cfg.scale);
  } else {
    int8_t acceleration[3]; // store raw data of acceleration here
    status |= mma8452q_read_block(MMA8452Q_OUT_X_MSB, 3, raw_data);
    acceleration[0] = (int8_t)raw_data[0];
    acceleration[1] = (int8_t)raw_data[1];
    acceleration[2] = (int8_t)raw_data[2];
    avec[0] = (float)acceleration[0]
              / (float)(1 << 7) * (float)(mma8452q_cfg.scale);
    avec[1] = (float)acceleration[1]
              / (float)(1 << 7) * (float)(mma8452q_cfg.scale);
    avec[2] = (float)acceleration[2]
              / (float)(1 << 7) * (float)(mma8452q_cfg.scale);
  }

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Get the data STATUS register
*******************************************************************************/
sl_status_t mma8452q_get_data_status(uint8_t *data_status)
{
  sl_status_t status = SL_STATUS_OK;

  // Check for Null pointer
  if (data_status == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = mma8452q_read_register(MMA8452Q_STATUS, data_status);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Check the available status of the data
*******************************************************************************/
sl_status_t mma8452q_check_for_data_ready(uint8_t *is_data_ready)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t temp;

  // Check for Null pointer
  if (is_data_ready == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = mma8452q_read_register(MMA8452Q_STATUS, &temp);
  *is_data_ready = (temp & 0x08) >> 3;

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Check the SYSMODE register
*******************************************************************************/
sl_status_t mma8452q_get_sysmode(uint8_t *reg_sysmode)
{
  sl_status_t status = SL_STATUS_OK;

  // Check for Null pointer
  if (reg_sysmode == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = mma8452q_read_register(MMA8452Q_SYSMOD, reg_sysmode);
  *reg_sysmode &= 0b00000011;

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Checks the interrupt source register
*******************************************************************************/
sl_status_t mma8452q_get_int_source(uint8_t *int_source)
{
  // Check for Null pointer
  if (int_source == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return mma8452q_read_register(MMA8452Q_INT_SOURCE, int_source);
}

/***************************************************************************//**
* Read the portrait/landscape status
*******************************************************************************/
sl_status_t mma8452q_get_pl_status(uint8_t *pl_status)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t plStat;

  // Check for Null pointer
  if (pl_status == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = mma8452q_read_register(MMA8452Q_PL_STATUS, &plStat);
  // Z-tilt lockout
  if (plStat & 0x40) {
    *pl_status = MMA8452Q_LOCKOUT;
  }
  // Otherwise return LAPO status
  else {
    *pl_status = (plStat & 0x6) >> 1;
  }

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Read the freefall/ motion status register
*******************************************************************************/
sl_status_t mma8452q_get_ff_mt_status(uint8_t *ff_mt_status)
{
  // Check for Null pointer
  if (ff_mt_status == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return mma8452q_read_register(MMA8452Q_FF_MT_SRC, ff_mt_status);
}

/***************************************************************************//**
* Read the transient status register
*******************************************************************************/
sl_status_t mma8452q_get_transient_status(uint8_t *transient_status)
{
  // Check for Null pointer
  if (transient_status == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return mma8452q_read_register(MMA8452Q_TRANSIENT_SRC, transient_status);
}

/***************************************************************************//**
* Read the pulse status register
*******************************************************************************/
sl_status_t mma8452q_get_pulse_status(uint8_t *pulse_status)
{
  // Check for Null pointer
  if (pulse_status == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return mma8452q_read_register(MMA8452Q_PULSE_SRC, pulse_status);
}

/***************************************************************************//**
* Set full scale range of x, y and z axis accelerometers
*******************************************************************************/
sl_status_t mma8452q_set_scale(mma8452q_scale_t scale)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t sysmode = 0;
  uint8_t cfg;
  if ((scale != MMA8452Q_SCALE_2G) && (scale != MMA8452Q_SCALE_4G)
      && (scale != MMA8452Q_SCALE_8G)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Change to standby if currently in other state
  status |= mma8452q_get_sysmode(&sysmode);
  if (sysmode != 0b00) {
    status |= mma8452q_active(false);
  }

  status |= mma8452q_read_register(MMA8452Q_XYZ_DATA_CFG, &cfg);
  // Mask out scale bits
  cfg &= 0xFC;
  cfg |= (scale >> 2);
  status |= mma8452q_write_register(MMA8452Q_XYZ_DATA_CFG, cfg);

  // Update local variables
  mma8452q_cfg.scale = scale;
  // Change to active mode
  mma8452q_active(true);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Set the output data rate
*******************************************************************************/
sl_status_t mma8452q_set_odr(mma8452q_odr_t odr)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t sysmode = 0;
  uint8_t ctrl;

  if (odr > MMA8452Q_ODR_1) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Change to standby if currently in other state
  status |= mma8452q_get_sysmode(&sysmode);
  if (sysmode != 0b00) {
    status |= mma8452q_active(false);
  }

  status |= mma8452q_read_register(MMA8452Q_CTRL_REG1, &ctrl);

  // Mask out data rate bits
  ctrl &= 0xC7;
  ctrl |= (odr << 3);
  status |= mma8452q_write_register(MMA8452Q_CTRL_REG1, ctrl);

  // Update local variables
  mma8452q_cfg.odr = odr;

  // Change to active mode
  mma8452q_active(true);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Set active power mode
*******************************************************************************/
sl_status_t mma8452q_set_mods(mma8452q_mods_t mods)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t sysmode = 0;
  uint8_t ctrl;

  if (mods > MMA8452Q_LPWR) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Change to standby if currently in other state
  status |= mma8452q_get_sysmode(&sysmode);
  if (sysmode != 0b00) {
    status |= mma8452q_active(false);
  }
  status |= mma8452q_read_register(MMA8452Q_CTRL_REG2, &ctrl);
  // Mask out data rate bits
  ctrl &= 0x03;
  ctrl |= mods;
  status |= mma8452q_write_register(MMA8452Q_CTRL_REG2, ctrl);
  // Update local variables
  mma8452q_cfg.active_mode_pwr = mods;

  // Change to active mode
  status |= mma8452q_active(true);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Config autosleep mode
*******************************************************************************/
sl_status_t mma8452q_config_aslp(mma8452q_aslp_odr_t alsp_rate,
                                 mma8452q_mods_t slp_mode_pwr,
                                 mma8452q_aslp_config_t aslp_cfg)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t sysmode = 0;
  uint8_t ctrl;

  if ((alsp_rate > MMA8452Q_ASLP_ODR_1p56) || (slp_mode_pwr > MMA8452Q_LPWR)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Change to standby if currently in other state
  status |= mma8452q_get_sysmode(&sysmode);
  if (sysmode != 0b00) {
    status |= mma8452q_active(false);
  }

  // Config sleep mode rate
  status |= mma8452q_read_register(MMA8452Q_CTRL_REG1, &ctrl);

  // Mask out data rate bits
  ctrl &= 0x3F;
  ctrl |= (alsp_rate << 5);
  status |= mma8452q_write_register(MMA8452Q_CTRL_REG1, ctrl);

  // Update local variables
  mma8452q_cfg.alsp_rate = alsp_rate;

  // Config sleep power mode
  status |= mma8452q_read_register(MMA8452Q_CTRL_REG2, &ctrl);
  // Mask out data rate bits
  ctrl &= 0xC7;
  ctrl |= (slp_mode_pwr << 3);
  status |= mma8452q_write_register(MMA8452Q_CTRL_REG2, ctrl);
  // Update local variables
  mma8452q_cfg.slp_mode_pwr = slp_mode_pwr;

  // Config counter autosleep
  status |= mma8452q_write_register(MMA8452Q_ASLP_COUNT, aslp_cfg.alsp_count);
  // Update local variables
  mma8452q_cfg.aslp_cfg = aslp_cfg;

  // Change to active mode
  status |= mma8452q_active(true);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Configure the transient function settings
*******************************************************************************/
sl_status_t mma8452q_config_trans(mma8452q_trans_config_t *trans_cfg)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t temp = 0;
  uint8_t sysmode = 0;

  // Change to standby if currently in other state
  status |= mma8452q_get_sysmode(&sysmode);
  if (sysmode != 0b00) {
    status |= mma8452q_active(false);
  }

  // Configure TRANSIENT_CFG register
  temp =
    (uint8_t)(((uint8_t)trans_cfg->en_event_latch << 4)
              | ((uint8_t)trans_cfg->en_z_trans << 3)
              | ((uint8_t)trans_cfg->en_y_trans << 2)
              | ((uint8_t)trans_cfg->en_x_trans << 1)
              | ((uint8_t)trans_cfg->en_hpf_bypass));
  status |= mma8452q_write_register(MMA8452Q_TRANSIENT_CFG, temp);

  // Configure TRANSIENT_THS register
  temp =
    (uint8_t)(((uint8_t)trans_cfg->db_cnt_mode << 7)
              | (trans_cfg->threshold & 0x7F));
  status |= mma8452q_write_register(MMA8452Q_TRANSIENT_THS, temp);

  // Configure TRANSIENT_COUNT register
  status |= mma8452q_write_register(MMA8452Q_TRANSIENT_COUNT,
                                    trans_cfg->debounce_cnt);

  // Update local variables
  mma8452q_cfg.trans_cfg = *trans_cfg;
  // Change to active mode
  status |= mma8452q_active(true);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Configure the orientation function settings
*******************************************************************************/
sl_status_t mma8452q_config_orientation(
  mma8452q_orientation_config_t orient_cfg)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t temp = 0;
  uint8_t sysmode = 0;

  // Change to standby if currently in other state
  status |= mma8452q_get_sysmode(&sysmode);
  if (sysmode != 0b00) {
    status |= mma8452q_active(false);
  }
  temp =
    (uint8_t)(((uint8_t)orient_cfg.db_cnt_mode << 7)
              | ((uint8_t)orient_cfg.en_event_latch << 6));
  status |= mma8452q_write_register(MMA8452Q_PL_CFG, temp);

  status |= mma8452q_write_register(MMA8452Q_PL_COUNT, orient_cfg.debounce_cnt);

  // Update local variables
  mma8452q_cfg.orient_cfg = orient_cfg;
  // Change to active mode
  status |= mma8452q_active(true);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Configure the pulse function settings
*******************************************************************************/
sl_status_t mma8452q_config_pulse(mma8452q_pulse_config_t *pulse_cfg)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t temp = 0;
  uint8_t sysmode = 0;

  // Change to standby if currently in other state
  status |= mma8452q_get_sysmode(&sysmode);
  if (sysmode != 0b00) {
    status |= mma8452q_active(false);
  }

  temp =
    (uint8_t)(((uint8_t)pulse_cfg->double_abort << 7)
              | ((uint8_t)pulse_cfg->en_event_latch << 6)
              | ((uint8_t)pulse_cfg->en_z_double << 5)
              | ((uint8_t)pulse_cfg->en_z_single << 4)
              | ((uint8_t)pulse_cfg->en_y_double << 3)
              | ((uint8_t)pulse_cfg->en_y_single << 2)
              | ((uint8_t)pulse_cfg->en_x_double << 1)
              | (uint8_t)pulse_cfg->en_z_single);
  status |= mma8452q_write_register(MMA8452Q_PULSE_CFG, temp);

  status |= mma8452q_write_register(MMA8452Q_PULSE_THSX,
                                    pulse_cfg->pulse_thresh_X);
  status |= mma8452q_write_register(MMA8452Q_PULSE_THSY,
                                    pulse_cfg->pulse_thresh_y);
  status |= mma8452q_write_register(MMA8452Q_PULSE_THSZ,
                                    pulse_cfg->pulse_thresh_z);
  status |= mma8452q_write_register(MMA8452Q_PULSE_TMLT,
                                    pulse_cfg->pulse_time_lmt);
  status |=
    mma8452q_write_register(MMA8452Q_PULSE_LTCY, pulse_cfg->pulse_latency);
  status |=
    mma8452q_write_register(MMA8452Q_PULSE_WIND, pulse_cfg->pulse_window);

  // Update local variables
  mma8452q_cfg.pulse_cfg = *pulse_cfg;
  // Change to active mode
  status |= mma8452q_active(true);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Configure the freefall/motion function settings
*******************************************************************************/
sl_status_t mma8452q_config_ff_mt(mma8452q_ff_mt_config_t *ff_mt_cfg)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t temp = 0;
  uint8_t sysmode = 0;

  // Change to standby if currently in other state
  status |= mma8452q_get_sysmode(&sysmode);
  if (sysmode != 0b00) {
    status |= mma8452q_active(false);
  }

  temp =
    (uint8_t)(((uint8_t)ff_mt_cfg->en_event_latch << 7)
              | ((uint8_t)ff_mt_cfg->ff_mt_sel << 6)
              | ((uint8_t)ff_mt_cfg->en_z_trans << 5)
              | ((uint8_t)ff_mt_cfg->en_y_trans << 4)
              | ((uint8_t)ff_mt_cfg->en_x_trans << 3));
  status |= mma8452q_write_register(MMA8452Q_FF_MT_CFG, temp);

  status |= mma8452q_write_register(MMA8452Q_FF_MT_THS, ff_mt_cfg->threshold);
  status |=
    mma8452q_write_register(MMA8452Q_FF_MT_COUNT, ff_mt_cfg->debounce_cnt);

  // Update local variables
  mma8452q_cfg.ff_mt_cfg = *ff_mt_cfg;
  // Change to active mode
  status |= mma8452q_active(true);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Configure the interrupt outputs
*******************************************************************************/
sl_status_t mma8452q_config_interrupt(
  mma8452q_interrupt_config_t *interrupt_cfg)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t temp = 0;
  uint8_t sysmode = 0;

  // Change to standby if currently in other state
  status |= mma8452q_get_sysmode(&sysmode);
  if (sysmode != 0b00) {
    status |= mma8452q_active(false);
  }

  temp =
    (uint8_t)(((uint8_t)interrupt_cfg->en_trans_wake << 6)
              | ((uint8_t)interrupt_cfg->en_orientation_wake << 5)
              | ((uint8_t)interrupt_cfg->en_pulse_wake << 4)
              | ((uint8_t)interrupt_cfg->en_ff_mt_wake << 3)
              | ((uint8_t)interrupt_cfg->int_active_hi << 1)
              | (uint8_t)interrupt_cfg->int_open_drain);
  status |= mma8452q_write_register(MMA8452Q_CTRL_REG3, temp);

  temp =
    (uint8_t)(((uint8_t)interrupt_cfg->en_aslp_int << 7)
              | ((uint8_t)interrupt_cfg->en_trans_int << 5)
              | ((uint8_t)interrupt_cfg->en_orientation_int << 4)
              | ((uint8_t)interrupt_cfg->en_pulse_int << 3)
              | ((uint8_t)interrupt_cfg->en_ff_mt_int << 2)
              | (uint8_t)interrupt_cfg->en_drdy_int);
  status |= mma8452q_write_register(MMA8452Q_CTRL_REG4, temp);

  temp =
    (uint8_t)(((uint8_t)interrupt_cfg->cfg_aslp_int << 7)
              | ((uint8_t)interrupt_cfg->cfg_trans_int << 5)
              | ((uint8_t)interrupt_cfg->cfg_orientation_int << 4)
              | ((uint8_t)interrupt_cfg->cfg_pulse_int << 3)
              | ((uint8_t)interrupt_cfg->cfg_ff_mt_int << 2)
              | (uint8_t)interrupt_cfg->cfg_drdy_int);
  status |= mma8452q_write_register(MMA8452Q_CTRL_REG5, temp);

  // Update local variables
  mma8452q_cfg.interrupt_cfg = *interrupt_cfg;
  // Change to active mode
  status |= mma8452q_active(true);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Configure the reduced noise mode setting
*******************************************************************************/
sl_status_t mma8452q_enable_low_noise(bool enable)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t sysmode = 0;
  uint8_t ctrl;

  // Change to standby if currently in other state
  status |= mma8452q_get_sysmode(&sysmode);
  if (sysmode != 0b00) {
    status |= mma8452q_active(false);
  }

  status |= mma8452q_read_register(MMA8452Q_CTRL_REG1, &ctrl);

  // Mask out data rate bits
  ctrl &= 0x7F;
  ctrl |= ((uint8_t)enable << 2);
  status |= mma8452q_write_register(MMA8452Q_CTRL_REG1, ctrl);

  // Update local variables
  mma8452q_cfg.en_low_noise = enable;

  // Change to active mode
  status |= mma8452q_active(true);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Configure the fast-read mode
*******************************************************************************/
sl_status_t mma8452q_enable_fast_read(bool enable)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t sysmode = 0;
  uint8_t ctrl;

  // Change to standby if currently in other state
  status |= mma8452q_get_sysmode(&sysmode);
  if (sysmode != 0b00) {
    status |= mma8452q_active(false);
  }

  status |= mma8452q_read_register(MMA8452Q_CTRL_REG1, &ctrl);

  // Mask out data rate bits
  ctrl &= 0xFD;
  ctrl |= ((uint8_t)enable << 1);
  status |= mma8452q_write_register(MMA8452Q_CTRL_REG1, ctrl);

  // Update local variables
  mma8452q_cfg.en_fast_read = enable;

  // Change to active mode
  status |= mma8452q_active(true);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Read register value
*******************************************************************************/
sl_status_t mma8452q_read_register(uint8_t addr, uint8_t *data)
{
  I2C_TransferSeq_TypeDef seq;
  uint8_t i2c_write_data;

  if (data == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  } else {
    seq.addr = mma8452q_cfg.dev_addr << 1;
    seq.flags = (uint16_t)I2C_FLAG_WRITE_READ;

    i2c_write_data = addr;

    /*Write buffer*/
    seq.buf[0].data = &i2c_write_data;
    seq.buf[0].len = 1;

    /*Read buffer*/
    seq.buf[1].data = data;
    seq.buf[1].len = 1;

    if (I2CSPM_Transfer(_mma8452q_i2cspm_instance, &seq) != i2cTransferDone) {
      return SL_STATUS_TRANSMIT;
    }
    return SL_STATUS_OK;
  }
}

/***************************************************************************//**
* Write into register
*******************************************************************************/
sl_status_t mma8452q_write_register(uint8_t addr, uint8_t data)
{
  I2C_TransferSeq_TypeDef seq;
  uint8_t i2c_write_data[2];

  seq.addr = mma8452q_cfg.dev_addr << 1;
  seq.flags = (uint16_t)I2C_FLAG_WRITE;

  i2c_write_data[0] = addr;
  i2c_write_data[1] = data;

  /*Write buffer*/
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 2;

  if (I2CSPM_Transfer(_mma8452q_i2cspm_instance, &seq) != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
* Read multiple bytes from sensor
*******************************************************************************/
sl_status_t mma8452q_read_block(uint8_t addr, uint8_t num_bytes, uint8_t *data)
{
  I2C_TransferSeq_TypeDef seq;
  uint8_t i2c_write_data;

  if (data == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  } else {
    seq.addr = mma8452q_cfg.dev_addr << 1;
    seq.flags = (uint16_t)I2C_FLAG_WRITE_READ;

    i2c_write_data = addr;

    /*Write buffer*/
    seq.buf[0].data = &i2c_write_data;
    seq.buf[0].len = 1;

    /*Read buffer*/
    seq.buf[1].data = data;
    seq.buf[1].len = num_bytes;

    if (I2CSPM_Transfer(_mma8452q_i2cspm_instance, &seq) != i2cTransferDone) {
      return SL_STATUS_TRANSMIT;
    }
    return SL_STATUS_OK;
  }
}

/***************************************************************************//**
* Write multiple bytes from sensor
*******************************************************************************/
sl_status_t mma8452q_write_block(uint8_t addr, uint8_t num_bytes, uint8_t *data)
{
  I2C_TransferSeq_TypeDef seq;
  uint8_t i2c_write_data[num_bytes + 1];

  seq.addr = mma8452q_cfg.dev_addr << 1;
  seq.flags = (uint16_t)I2C_FLAG_WRITE;

  i2c_write_data[0] = addr;

  memcpy(&i2c_write_data[1], data, num_bytes);

  /*Write buffer*/
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = num_bytes;

  if (I2CSPM_Transfer(_mma8452q_i2cspm_instance, &seq) != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ********************************************************************************
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
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/

#include "app_assert.h"
#include "app_log.h"
#include "mma8452q_config.h"
#include "mma8452q.h"

#define TEST_BASIC_READING
#define TEST_RAW_DATA_READING
#define TEST_ORIENTATION
#define TEST_READ_TAPS
#define TEST_FF_MT

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  mma8452q_init(MMA8452Q_CONFIG_I2C);
  mma8452q_set_address(SL_MMA8452Q_I2C_BUS_ADDRESS2);
#ifdef TEST_ORIENTATION
  mma8452q_orientation_config_t orient_cfg = {
    .db_cnt_mode = true,
    .en_event_latch = true,
    .debounce_cnt = 0x00
  };
  mma8452q_config_orientation(orient_cfg);
#endif
#ifdef TEST_READ_TAPS
  mma8452q_pulse_config_t pulse_cfg = {
    .double_abort = false,
    .en_event_latch = true,
    .en_z_double = true,
    .en_y_double = true,
    .en_y_single = true,
    .en_x_double = true,
    .en_x_single = true,
    .pulse_thresh_z = 0x80,
    .pulse_thresh_y = 0x80,
    .pulse_thresh_X = 0x80,
    .pulse_time_lmt = 0x30,
    .pulse_latency = 0xA0,
    .pulse_window = 0xFF
  };
  mma8452q_config_pulse(&pulse_cfg);
#endif
#ifdef TEST_FF_MT
  mma8452q_ff_mt_config_t ff_mt_cfg = {
    .en_event_latch = true,
    .ff_mt_sel = true,
    .en_z_trans = false,
    .en_y_trans = true,
    .en_x_trans = true,
    .db_cnt_mode = false,
    .threshold = 0x84,
    .debounce_cnt = 0x8
  };
  mma8452q_interrupt_config_t int_cfg = {
    .en_trans_wake = false,        // enable transient auto-sleep wake
    .en_orientation_wake = false,  // enable orientation auto-sleep wake
    .en_pulse_wake = false,        // enable pulse auto-sleep wake
    .en_ff_mt_wake = true,         // enable freefall/motion auto-sleep wake
    .int_active_hi = false,        // interrupt polarity [0:active_low,
                                   //   1:active_hi]
    .int_open_drain = false,       // interrupt pin cfg [0:push-pull,
                                   //   1:open-drain]
    .en_aslp_int = false,          // auto-sleep transition interrupt
    .en_trans_int = false,         // transient interrupt
    .en_orientation_int = false,   // orientation interrupt
    .en_pulse_int = false,         // pulse transition interrupt
    .en_ff_mt_int = true,          // freefall/motion interrupt
    .en_drdy_int = false,          // data ready interrupt
    .cfg_aslp_int = 0,             // interrupt routed to INT1 [0:INT2, 1:INT1]
    .cfg_trans_int = 0,            // interrupt routed to INT1 [0:INT2, 1:INT1]
    .cfg_orientation_int = 0,      // interrupt routed to INT1 [0:INT2, 1:INT1]
    .cfg_pulse_int = 0,            // interrupt routed to INT1 [0:INT2, 1:INT1]
    .cfg_ff_mt_int = 0,            // interrupt routed to INT1 [0:INT2, 1:INT1]
    .cfg_drdy_int = 0              // interrupt routed to INT1 [0:INT2, 1:INT1]
  };

  // Config interrupt for ff_mt
  mma8452q_config_interrupt(&int_cfg);
  mma8452q_config_ff_mt(&ff_mt_cfg);
#endif
  mma8452q_enable_fast_read(false);
  mma8452q_active(true);
  app_log(" Initialize MMA8452Q Successfully!!\r\n");
#ifdef TEST_BASIC_READING
  app_log(" Basic Reading Test\r\n");
  app_log("---------------------------------------\r\n");
#endif
#ifdef TEST_RAW_DATA_READING
  app_log(" Raw Data Reading Test\r\n");
  app_log("---------------------------------------\r\n");
#endif
#ifdef TEST_ORIENTATION
  app_log(" Orientation Reading Test\r\n");
  app_log("---------------------------------------\r\n");
#endif
#ifdef TEST_READ_TAPS
  app_log(" Tap Detection Test\r\n");
  app_log("---------------------------------------\r\n");
#endif
#ifdef TEST_FF_MT
  app_log(" Free Fall/ Motion Test\r\n");
  app_log("---------------------------------------\r\n");
#endif
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
#ifdef TEST_BASIC_READING
  float value[3];
  uint8_t is_ready = 0;

  mma8452q_check_for_data_ready(&is_ready);
  if (is_ready) {
    sl_mma8452q_get_calculated_acceleration(value);
    app_log(
      "acceleration x direction: %f, acceleration y direction: %f, acceleration z direction: %f\r\n",
      value[0],
      value[1],
      value[2]);
  }
#endif
#ifdef TEST_RAW_DATA_READING
  int16_t value_raw[3];
  uint8_t is_ready_raw = 0;

  mma8452q_check_for_data_ready(&is_ready_raw);
  if (is_ready_raw) {
    sl_mma8452q_get_acceleration(value_raw);
    app_log(
      "raw acceleration x direction: %d, raw acceleration y direction: %d, raw acceleration z direction: %d\r\n",
      value_raw[0],
      value_raw[1],
      value_raw[2]);
  }
#endif
#ifdef TEST_ORIENTATION
  uint8_t is_ready_orien = 0;
  uint8_t pl_status;

  mma8452q_check_for_data_ready(&is_ready_orien);
  if (is_ready_orien) {
    mma8452q_get_pl_status(&pl_status);
    if (pl_status == MMA8452Q_PORTRAIT_U) {
      app_log("Orientation status: up\r\n");
    }
    if (pl_status == MMA8452Q_PORTRAIT_D) {
      app_log("Orientation status: down\r\n");
    }
    if (pl_status == MMA8452Q_LANDSCAPE_R) {
      app_log("Orientation status: right\r\n");
    }
    if (pl_status == MMA8452Q_LANDSCAPE_L) {
      app_log("Orientation status: left\r\n");
    }
    if (pl_status == MMA8452Q_LOCKOUT) {
      app_log("Orientation status: flat\r\n");
    }
  }
#endif
#ifdef TEST_READ_TAPS
  uint8_t is_ready_tap = 0;
  mma8452q_check_for_data_ready(&is_ready_tap);
  if (is_ready_tap) {
    uint8_t tap = 0;
    uint8_t pulse_source = 0;

    mma8452q_get_pulse_status(&pulse_source);
    if (pulse_source & 0x80) {
      tap = pulse_source & 0x7F;
    }
    if (tap > 0) {
      app_log("TAP Detected\r\n");
    }
  }
#endif
#ifdef TEST_FF_MT
  uint8_t source_int = 0;
  uint8_t temp_ff_mt = 0;

  mma8452q_get_int_source(&source_int);
  mma8452q_get_ff_mt_status(&temp_ff_mt);
  if (source_int & MMA8452Q_MASK_IRQ_SRC_FF_MT) {
    if ((temp_ff_mt >> 7) == 1) {
      if ((temp_ff_mt & 0x02) == 0x02) {      // If XHE bit is set, 
                                              //x-motion detected
        if ((temp_ff_mt & 0x01) == 0x01) {    // If XHP is 1, x event was
                                              //   negative
          app_log(" x- \r\n");
        } else {
          app_log(" x+ \r\n");
        }
      }
      if ((temp_ff_mt & 0x08) == 0x08) {    // If YHE bit is set, y-motion
                                            //   detected
        if ((temp_ff_mt & 0x04) == 0x04) {  // If YHP is set, y event was
                                            //   negative
          app_log(" y- \r\n");
        } else {
          app_log(" y+ \r\n");
        }
      }
      if ((temp_ff_mt & 0x20) == 0x20) { // If ZHE bit is set, z-motion detected
        if ((temp_ff_mt & 0x10) == 0x10) {
          app_log(" z-(mg) \r\n");
        } else {
          app_log(" z+(mg) \r\n");
        }
      }
    }
  }
#endif
}

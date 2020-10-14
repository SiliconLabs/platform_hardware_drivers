/***************************************************************************//**
 * @file   nt3h2x11.h
 * @brief  APIs and types for high level NT3H2x11 functionalities.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
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
 ******************************************************************************/

/* ...
 *
 * EVALUATION QUALITY
 * This code has been minimally tested to ensure that it builds with the specified dependency versions and is suitable as a demonstration for evaluation purposes only.
 * This code will be maintained at the sole discretion of Silicon Labs.
 *
... */

#ifndef __NT3H2X11_H__

/***************************************************************************//**
 * @addtogroup NT3H2x11 Driver
 * @brief NT3H2x11 top level APIs and Types.
 * @{
 ******************************************************************************/

#define __NT3H2X11_H__

#include <stdbool.h>
#include "em_gpio.h"
#include "nt3h2x11_i2c.h"
#include "nt3h2x11_fd.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  nt3h2x11_i2c_init_t i2c_init;
  nt3h2x11_fd_init_t  fd_init;
} nt3h2x11_init_t;

// NT3H2x11 configuration register addresses within memory block.
typedef enum {
  config_reg_nc_reg               = 0,
  config_reg_last_ndef_block      = 1,
  config_reg_stam_mirror_block    = 2,
  config_reg_wdt_ls               = 3,
  config_reg_wdt_ms               = 4,
  config_reg_i2c_clock_str        = 5,
  config_reg_reg_lock             = 6
} nt3h2x11_config_reg_addr_t;

// NT3H2x11 session register addresses within memory block.
typedef enum {
  session_reg_nc_reg              = 0,
  session_reg_last_ndef_block     = 1,
  session_reg_stam_mirror_block   = 2,
  session_reg_wdt_ls              = 3,
  session_reg_wdt_ms              = 4,
  session_reg_i2c_clock_str       = 5,
  session_reg_ns_reg              = 6
} nt3h2x11_session_reg_addr_t;

/// NT3H2x11 error code.
typedef enum {
  nt3h2x11_err_none               = 0,
  nt3h2x11_err_i2c_error          = 1,
  nt3h2x11_err_invalid_reg_addr   = 2
} nt3h2x11_error_code_t;

/// NC_REG type. Details please refer to NT3H2111_2211 datasheet Table 13 and 14.
typedef struct {
  bool    nfcs_i2c_rst_on_off;
  bool    pthru_on_off;
  uint8_t fd_off;
  uint8_t fd_on;
  bool    sram_mirror_on_off;
  bool    transfer_dir;
} nt3h2x11_nc_reg_t;

/// I2C_CLOCK_STR type. Details please refer to NT3H2111_2211 datasheet Table 13 and 14.
typedef struct {
  /* Status bit to show the number of negative PWD_AUTH attempts reached
   *  0b: PWD_AUTH still possible
   *  1b: PWD_AUTH locked
   * Note: this field is only available in session register.
   */
  bool    nrg_auth_reached;
  /* Enables (1b) or disable (0b) the I2C clock stretching. */
  bool    i2c_clock_str;
} nt3h2x11_i2c_clock_str_t;

/// REG_LOCK type. Details please refer to NT3H2111_2211 datasheet Table 13 and 14.
typedef struct {
  bool    reg_lock_i2c;
  bool    reg_lock_nfc;
} nt3h2x11_reg_lock_t;

/// NS_REG type. Detials please refer to NRAG I2C plus datasheet Table 14.
typedef struct {
  /* 1b: all data bytes read from the address specified in LAST_NDEF_BLOCK.
   * Bit is reset to 0b when read. */
  bool    ndef_data_read;
  /* 1b: Memory access is locked to the I2C interface. */
  bool    i2c_locked;
  /* 1b: Memory access is locked to the NFC interface. */
  bool    rf_locked;
  /* 1b: data is ready in SRAM buffer to be read by I2C. */
  bool    sram_i2c_ready;
  /* 1b: data is ready in SRAM buffer to be read by NFC. */
  bool    sram_rf_ready;
  /* 1b: HV voltage error during EEPROM write or erase cycle
   * Needs to be written back via I2C to 0b to be cleared. */
  bool    eeprom_wr_err;
  /* 1b: EEPROM write cycle in progress - access to EEPROM disabled.
   * 0b: EEPROM access possible. */
  bool    eeprom_wr_busy;
  /* 1b: NFC field is detected. */
  bool    rf_firld_present;
} nt3h2x11_i2c_ns_reg_t;

/// Configuration register type. Details please refer to NT3H2111_2211 datasheet Table 13.
typedef struct {
  /* NC_REG. */
  nt3h2x11_nc_reg_t               nc_reg;
  /* LAST_NDEF_BLOCK: I2C block address of I2C block, which contains last byte(s) of stored NDEF message. */
  uint8_t                         last_ndef_block;
  /* SRAM_MIRROR_BLOCK: 2C block address of SRAM when mirrored into the User memory. */
  uint8_t                         sram_mirror_block;
  /* WDT_LS: Least Significant byte of watchdog time control register. */
  uint8_t                         wdt_ls;
  /* WDT_MS: Most Significant byte of watchdog time control register. When writing WDT_MS byte,
       the content of WDT_MS and WDT_LS gets active for the watchdog timer. */
  uint8_t                         wdt_ms;
  /* I2C_CLOCK_STR. */
  nt3h2x11_i2c_clock_str_t        i2c_clock_str;
  /* REG_LOCK. */
  nt3h2x11_reg_lock_t             reg_lock;

} nt3h2x11_config_reg_t;

/// Session register type. Details please refer to NT3H2111_2211 datasheet Table 14.
typedef struct {
  /* NC_REG. */
  nt3h2x11_nc_reg_t               nc_reg;
  /* LAST_NDEF_BLOCK: I2C block address of I2C block, which contains last byte(s) of stored NDEF message. */
  uint8_t                         last_ndef_block;
  /* SRAM_MIRROR_BLOCK: 2C block address of SRAM when mirrored into the User memory. */
  uint8_t                         sram_mirror_block;
  /* WDT_LS: Least Significant byte of watchdog time control register. */
  uint8_t                         wdt_ls;
  /* WDT_MS: Most Significant byte of watchdog time control register. When writing WDT_MS byte,
       the content of WDT_MS and WDT_LS gets active for the watchdog timer. */
  uint8_t                         wdt_ms;
  /* I2C_CLOCK_STR. */
  nt3h2x11_i2c_clock_str_t        i2c_clock_str;
  /* REG_LOCK. */
  nt3h2x11_i2c_ns_reg_t           ns_reg;

} nt3h2x11_session_reg_t;

/// NT3H2x11 register read result type.
typedef struct {
  nt3h2x11_error_code_t           err;
  uint8_t                         reg_value;
} nt3h2x11_reg_read_result_t;

void nt3h2x11_init (nt3h2x11_init_t init);

i2c_transfer_return_t nt3h2x11_read_block (uint8_t mema, uint8_t* data);

i2c_transfer_return_t nt3h2x11_write_block (uint8_t mema, uint8_t* data);

nt3h2x11_reg_read_result_t nt3h2x11_i2c_read_config_reg (nt3h2x11_config_reg_addr_t rega);

nt3h2x11_error_code_t nt3h2x11_i2c_write_config_reg (nt3h2x11_config_reg_addr_t rega, uint8_t regd);

nt3h2x11_reg_read_result_t nt3h2x11_i2c_read_session_reg (nt3h2x11_session_reg_addr_t rega);

nt3h2x11_error_code_t nt3h2x11_i2c_write_session_reg (nt3h2x11_session_reg_addr_t rega, uint8_t regd);

uint8_t nt3h2x11_encode_nc_reg (nt3h2x11_nc_reg_t nc_reg);

nt3h2x11_nc_reg_t nt3h2x11_decode_nc_reg (uint8_t reg_value);

uint8_t nt3h2x11_i2c_encode_clock_str (nt3h2x11_i2c_clock_str_t i2c_clock_str_reg);

nt3h2x11_i2c_clock_str_t nt3h2x11_i2c_decode_clock_str (uint8_t reg_value);

nt3h2x11_reg_lock_t nt3h2x11_decode_reg_lock (uint8_t reg_value);

#ifdef __cplusplus
}
#endif
/** @} (end addtogroup NT3H2x11 Driver) */
#endif

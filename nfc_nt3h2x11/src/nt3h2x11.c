/***************************************************************************//**
 * @file   nt3h2x11.c
 * @brief  Implementation for high level NT3H2x11 functionalities.
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

#include <stdint.h>
#include "em_cmu.h"
#include "../inc/nt3h2x11.h"
#include "../inc/nt3h2x11_i2c.h"
#include "../inc/nt3h2x11_fd.h"

#define ENABLED                                             (1)
#define DISABLED                                            (0)

#define ON                                                  (1)
#define OFF                                                 (0)

#define NT3H2X11_I2C_CONFIGURATION_REGS_MEM_ADDR            0x3A
#define NT3H2X11_I2C_SESSION_REGS_MEM_ADDR                  0xFE

#define BIT7_MASK                                           0x80
#define BIT6_MASK                                           0x40
#define BIT5_MASK                                           0x20
#define BIT4_MASK                                           0x10
#define BIT3_MASK                                           0x08
#define BIT2_MASK                                           0x04
#define BIT1_MASK                                           0x02
#define BIT0_MASK                                           0x01

#define NT3H2X11_REG_LOCKED                                 (1)
#define NT3H2X11_REG_UNLOCKED                               (0)

#define NT3H2X11_NC_REG_NFCS_I2C_RST_ON_OFF_M               BIT7_MASK
#define NT3H2X11_NC_REG_NFCS_I2C_RST_ON_OFF_SHIFT           (7)
#define NT3H2X11_NC_REG_PTHRU_ON_OFF_M                      BIT6_MASK
#define NT3H2X11_NC_REG_PTHRU_ON_OFF_SHIFT                  (6)
#define NT3H2X11_NC_REG_FD_OFF_M                            (BIT5_MASK | BIT4_MASK)
#define NT3H2X11_NC_REG_FD_OFF_SHIFT                        (4)
#define NT3H2X11_NC_REG_FD_ON_M                             (BIT3_MASK | BIT2_MASK)
#define NT3H2X11_NC_REG_FD_ON_SHIFT                         (2)
#define NT3H2X11_NC_REG_SRAM_MIRROR_ON_OFF_M                (BIT1_MASK)
#define NT3H2X11_NC_REG_SRAM_MIRROR_ON_OFF_SHIFT            (1)
#define NT3H2X11_NC_REG_TRANSFER_DIR_M                      (BIT0_MASK)
#define NT3H2X11_NC_REG_TRANSFER_DIR_SHIFT                  (0)

#define NT3H2X11_I2C_CLOCK_STR_M                            (BIT0_MASK)
#define NT3H2X11_I2C_CLOCK_STR_SHIFT                        (0)

#define TRANSFER_DIR_I2C_TO_NFC                             (0)
#define TRANSFER_DIR_NFC_TO_I2C                             (1)

extern void nt3h2x11_i2c_init (nt3h2x11_i2c_init_t i2c_init);

extern i2c_transfer_return_t nt3h2x11_i2c_read (uint8_t mema, uint8_t* data);

extern i2c_transfer_return_t nt3h2x11_i2c_write (uint8_t mema, uint8_t* data);

extern i2c_transfer_return_t nt3h2x11_i2c_read_reg (uint8_t mema, uint8_t rega, uint8_t* data);

extern i2c_transfer_return_t nt3h2x11_i2c_write_reg (uint8_t mema, uint8_t rega, uint8_t regdat);

extern void nt3h2x11_fd_init (nt3h2x11_fd_init_t fd_init);

/**************************************************************************//**
 * @brief
 *  Initialize NT3H2x11 peripherals.
 *
 * @param[in] init
 *  Initialization settings.
 *****************************************************************************/
void nt3h2x11_init (nt3h2x11_init_t init) {
  /* enable field detection GPIO interrupt */
  if(init.fd_init.enable) {
    nt3h2x11_fd_init(init.fd_init);
  }
  /* enable I2C communication */
  if(init.i2c_init.enable) {
    nt3h2x11_i2c_init(init.i2c_init);
  }
}

/**************************************************************************//**
 * @brief
 *  Read a memory block from NT3H2x11.
 *
 * @param[in] mema
 *  Memory address
 *
 * @param[out] data
 *  Data buffer to hold the result
 *
 * @returns
 *  I2C transfer status.
 *
 * @note
 *  Details for I2C READ operation, please refer to NT3H2111_2211 product
 *  data sheet section 9.7.
 *****************************************************************************/
i2c_transfer_return_t nt3h2x11_read_block (uint8_t mema, uint8_t* data) {
  return nt3h2x11_i2c_read(mema, data);
}

/**************************************************************************//**
 * @brief
 *   Write a memory block to NT3H2x11.
 *
 * @param[in] mema
 *  Memory address
 *
 * @param[in] data
 *  Data to be written
 *
 * @returns
 *  I2C transfer status.
 *
 * @note
 *  Details for I2C WRITE operation, please refer to NT3H2111_2211 product
 *  data sheet section 9.7.
 *****************************************************************************/
i2c_transfer_return_t nt3h2x11_write_block (uint8_t mema, uint8_t* data) {
  return nt3h2x11_i2c_write(mema, data);
}

/**************************************************************************//**
 * @brief
 *  Read a configuration register in NT3H2x11.
 *
 * @param[in] rega
 *  Register address within memory block
 *
 * @returns
 *  Read result (data and any error code)
 *
 * @note
 *  Details for I2C configuration registers. Please refer to NT3H2111_2211
 *  product data sheet section 8.3.12.
 *
 * @warning
 *  This function is provided as prototype and has not been tested.
 *****************************************************************************/
nt3h2x11_reg_read_result_t nt3h2x11_i2c_read_config_reg (nt3h2x11_config_reg_addr_t rega) {

  uint8_t byte_buff;

  nt3h2x11_reg_read_result_t result;

  if (rega > 6) {
    result.err = nt3h2x11_err_invalid_reg_addr;
    return result;
  }

  if (nt3h2x11_i2c_read_reg(NT3H2X11_I2C_CONFIGURATION_REGS_MEM_ADDR, rega, &byte_buff) != i2cTransferDone) {
    result.err = nt3h2x11_err_i2c_error;
  } else {
    result.err = nt3h2x11_err_none;
    result.reg_value = byte_buff;
  }

  return result;

}

/**************************************************************************//**
 * @brief
 *  Write to a configuration register in NT3H2x11.
 *
 * @param[in] rega
 *  Register address within memory block
 *
 * @param[in] regd
 *  Register data to be written
 *
 * @returns
 *  Any error code
 *
 * @note
 *  Details for I2C configuration registers. Please refer to NT3H2111_2211
 *  product data sheet section 8.3.12.
 *
 * @warning
 *  This function is provided as prototype and has not been tested.
 *****************************************************************************/
nt3h2x11_error_code_t nt3h2x11_i2c_write_config_reg (nt3h2x11_config_reg_addr_t rega, uint8_t regd) {

  if (rega > 6) {
    return nt3h2x11_err_invalid_reg_addr;
  }

  if (nt3h2x11_i2c_write_reg(NT3H2X11_I2C_CONFIGURATION_REGS_MEM_ADDR, rega, regd) != i2cTransferDone) {
    return nt3h2x11_err_i2c_error;
  }

  return nt3h2x11_err_none;
}

/**************************************************************************//**
 * @brief
 *  Read a session register in NT3H2x11.
 *
 * @param[in] rega
 *  Register address within memory block
 *
 * @returns
 *  Read result (data and any error code)
 *
 * @note
 *  Details for I2C session registers. Please refer to NT3H2111_2211
 *  product data sheet section 8.3.12.
 *
 * @warning
 *  This function is provided as prototype and has not been tested.
 *****************************************************************************/
nt3h2x11_reg_read_result_t nt3h2x11_i2c_read_session_reg (nt3h2x11_session_reg_addr_t rega) {

  uint8_t byte_buff;

  nt3h2x11_reg_read_result_t result;

  if (rega > 6) {
    result.err = nt3h2x11_err_invalid_reg_addr;
    return result;
  }

  if (nt3h2x11_i2c_read_reg(NT3H2X11_I2C_SESSION_REGS_MEM_ADDR, rega, &byte_buff) != i2cTransferDone) {
    result.err = nt3h2x11_err_i2c_error;
  } else {
    result.err = nt3h2x11_err_none;
    result.reg_value = byte_buff;
  }

  return result;

}

/**************************************************************************//**
 * @brief
 *  Write to a session register in NT3H2x11.
 *
 * @param[in] rega
 *  Register address within memory block
 *
 * @param[in] regd
 *  Register data to be written
 *
 * @returns
 *  Any error code
 *
 * @note
 *  Details for I2C session registers. Please refer to NT3H2111_2211
 *  product data sheet section 8.3.12.
 *
 * @warning
 *  This function is provided as prototype and has not been tested.
 *****************************************************************************/
nt3h2x11_error_code_t nt3h2x11_i2c_write_session_reg (nt3h2x11_session_reg_addr_t rega, uint8_t regd) {

  if (rega > 4) {
    return nt3h2x11_err_invalid_reg_addr;
  }

  if (nt3h2x11_i2c_write_reg(NT3H2X11_I2C_SESSION_REGS_MEM_ADDR, rega, regd) != i2cTransferDone) {
    return nt3h2x11_err_i2c_error;
  }

  return nt3h2x11_err_none;
}

/**************************************************************************//**
 * @brief
 *  Encode NC_REG content to one byte of data.
 *
 * @param[in] nc_reg
 *  Content of NC_REG to be encoded
 *
 * @returns
 *  Encoded data
 *
 * @note
 *  Details for NC_REG. Please refer to NT3H2111_2211 product data sheet
 *  section 8.3.12.
 *
 * @warning
 *  This function is provided as prototype and has not been tested.
 *****************************************************************************/
uint8_t nt3h2x11_encode_nc_reg (nt3h2x11_nc_reg_t nc_reg) {

  uint8_t result = 0;

  if (nc_reg.nfcs_i2c_rst_on_off) {
    result += ((ON << NT3H2X11_NC_REG_NFCS_I2C_RST_ON_OFF_SHIFT) & NT3H2X11_NC_REG_NFCS_I2C_RST_ON_OFF_M);
  }

  if (nc_reg.pthru_on_off) {
    result += ((ON << NT3H2X11_NC_REG_PTHRU_ON_OFF_SHIFT) & NT3H2X11_NC_REG_PTHRU_ON_OFF_M);
  }

  result += ((nc_reg.fd_off << NT3H2X11_NC_REG_FD_OFF_SHIFT) & NT3H2X11_NC_REG_FD_OFF_M);

  result += ((nc_reg.fd_on << NT3H2X11_NC_REG_FD_ON_SHIFT) & NT3H2X11_NC_REG_FD_ON_M);

  if (nc_reg.sram_mirror_on_off) {
    result += ((ON << NT3H2X11_NC_REG_SRAM_MIRROR_ON_OFF_SHIFT) & NT3H2X11_NC_REG_SRAM_MIRROR_ON_OFF_M);
  }

  if (nc_reg.transfer_dir) {
    result += ((TRANSFER_DIR_NFC_TO_I2C << NT3H2X11_NC_REG_TRANSFER_DIR_SHIFT) & NT3H2X11_NC_REG_TRANSFER_DIR_M);
  }

  return result;
}

/**************************************************************************//**
 * @brief
 *  Decode NC_REG content from one byte of raw data.
 *
 * @param[in] nc_reg
 *  One byte of raw NC_REG data to be decoded
 *
 * @returns
 *  Decoded NC_REG data
 *
 * @note
 *  Details for NC_REG. Please refer to NT3H2111_2211 product data sheet
 *  section 8.3.12.
 *
 * @warning
 *  This function is provided as prototype and has not been tested.
 *****************************************************************************/
nt3h2x11_nc_reg_t nt3h2x11_decode_nc_reg (uint8_t reg_value) {

  nt3h2x11_nc_reg_t nc_reg;

  /*  */
  if (reg_value & BIT7_MASK) {
    nc_reg.nfcs_i2c_rst_on_off = ON;
  } else {
    nc_reg.nfcs_i2c_rst_on_off = OFF;
  }
  /*  */
  if (reg_value & BIT6_MASK) {
    nc_reg.pthru_on_off = ON;
  } else {
    nc_reg.pthru_on_off = OFF;
  }
  /*  */
  nc_reg.fd_off = (reg_value & NT3H2X11_NC_REG_FD_OFF_M >> NT3H2X11_NC_REG_FD_OFF_SHIFT);
  /*  */
  nc_reg.fd_on  = (reg_value & NT3H2X11_NC_REG_FD_ON_M >> NT3H2X11_NC_REG_FD_ON_SHIFT);
  /*  */
  if (reg_value & BIT1_MASK) {
    nc_reg.sram_mirror_on_off = ON;
  } else {
    nc_reg.sram_mirror_on_off = OFF;
  }
  /*  */
  if (reg_value & BIT0_MASK) {
    nc_reg.transfer_dir = TRANSFER_DIR_NFC_TO_I2C;
  } else {
    nc_reg.transfer_dir = TRANSFER_DIR_I2C_TO_NFC;
  }

  return nc_reg;
}

/**************************************************************************//**
 * @brief
 *  Encode I2C_CLOCK_STR content to one byte of data.
 *
 * @param[in] i2c_clock_str_reg
 *  Content of I2C_CLOCK_STR to be encoded
 *
 * @returns
 *  Encoded data
 *
 * @note
 *  Details for I2C_CLOCK_STR. Please refer to NT3H2111_2211 product data sheet
 *  section 8.3.12.
 *
 * @warning
 *  This function is provided as prototype and has not been tested.
 *****************************************************************************/
uint8_t nt3h2x11_i2c_encode_clock_str (nt3h2x11_i2c_clock_str_t i2c_clock_str_reg) {

  uint8_t result = 0;

  if (i2c_clock_str_reg.i2c_clock_str) {
    result += ((ENABLED << NT3H2X11_I2C_CLOCK_STR_SHIFT) & NT3H2X11_I2C_CLOCK_STR_M);
  }

  return result;
}

/**************************************************************************//**
 * @brief
 *  Decode one byte of raw I2C_CLOCK_STR data.
 *
 * @param[in] i2c_clock_str_reg
 *  Raw I2C_CLOCK_STR data to be decoded
 *
 * @returns
 *  Decoded I2C_CLOCK_STR data
 *
 * @note
 *  Details for I2C_CLOCK_STR. Please refer to NT3H2111_2211 product data sheet
 *  section 8.3.12.
 *
 * @warning
 *  This function is provided as prototype and has not been tested.
 *****************************************************************************/
nt3h2x11_i2c_clock_str_t nt3h2x11_i2c_decode_clock_str (uint8_t reg_value) {

  nt3h2x11_i2c_clock_str_t i2c_clock_str_reg;

  if (reg_value & (BIT1_MASK)) {
    i2c_clock_str_reg.nrg_auth_reached = true;
  }

  if (reg_value & BIT0_MASK) {
    i2c_clock_str_reg.i2c_clock_str = ENABLED;
  } else {
    i2c_clock_str_reg.i2c_clock_str = DISABLED;
  }

  return i2c_clock_str_reg;
}

/**************************************************************************//**
 * @brief
 *  Decode one byte of raw REG_LOCK data.
 *
 * @param[in] i2c_clock_str_reg
 *  Raw REG_LOCK data to be decoded
 *
 * @returns
 *  Decoded REG_LOCK data
 *
 * @note
 *  Details for REG_LOCK. Please refer to NT3H2111_2211 product data sheet
 *  section 8.3.12.
 *
 * @warning
 *  This function is provided as prototype and has not been tested.
 *****************************************************************************/
nt3h2x11_reg_lock_t nt3h2x11_decode_reg_lock (uint8_t reg_value) {

  nt3h2x11_reg_lock_t reg_lock;

  if (reg_value & BIT1_MASK) {
    reg_lock.reg_lock_i2c = NT3H2X11_REG_LOCKED;
  } else {
    reg_lock.reg_lock_i2c = NT3H2X11_REG_UNLOCKED;
  }
  if (reg_value & BIT0_MASK) {
    reg_lock.reg_lock_nfc = NT3H2X11_REG_LOCKED;
  } else {
    reg_lock.reg_lock_nfc = NT3H2X11_REG_UNLOCKED;
  }

  return reg_lock;
}

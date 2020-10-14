/***************************************************************************//**
 * @file   nt3h2x11_i2c.c
 * @brief  Implementation for I2C interface for NT3H2x11.
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
#include <string.h>
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_i2c.h"
#include "../inc/nt3h2x11_i2c.h"

/// NT3H2x11 memory block size
#define NT3H2X11_BLOCK_SIZE                (16)

static I2C_TypeDef *nt3h2x11_i2c_port;

/**************************************************************************//**
 * @briefs
 *  Initialize NT3H2x11 I2C communication.
 *
 * @param[in] i2c_init
 *  I2C setting.
 *****************************************************************************/
void nt3h2x11_i2c_init (nt3h2x11_i2c_init_t i2c_init) {

  nt3h2x11_i2c_port = i2c_init.i2c_port;

  CMU_ClockEnable(cmuClock_GPIO, true);

#if defined(I2C0)
  if (i2c_init.i2c_port == I2C0) {
    CMU_ClockEnable(cmuClock_I2C0, true);
  }
#endif
#if defined(I2C1)
  else if (i2c_init.i2c_port == I2C1) {
    CMU_ClockEnable(cmuClock_I2C1, true);
  }
#endif
#if defined(I2C2)
  else if (i2c_init.i2c_port == I2C2) {
    CMU_ClockEnable(cmuClock_I2C2, true);
  }
#endif

  /* Configure GPIO mode */
  GPIO_PinModeSet(i2c_init.scl_port, i2c_init.scl_pin, gpioModeWiredAndPullUpFilter, 1);
  GPIO_PinModeSet(i2c_init.sda_port, i2c_init.sda_pin, gpioModeWiredAndPullUpFilter, 1);

  /* Using default settings */
  I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;
  /* Use 400khz SCK */
  i2cInit.freq = I2C_FREQ_FAST_MAX;

#if (_SILICON_LABS_32B_SERIES == 1)

  /* Enable pins at location 15 as specified in datasheet */
  nt3h2x11_i2c_port->ROUTEPEN = I2C_ROUTEPEN_SDAPEN | I2C_ROUTEPEN_SCLPEN;
  nt3h2x11_i2c_port->ROUTELOC0 = ((nt3h2x11_i2c_port->ROUTELOC0 & (~_I2C_ROUTELOC0_SDALOC_MASK)) | (i2c_init.sda_loc << _I2C_ROUTELOC0_SDALOC_SHIFT));
  nt3h2x11_i2c_port->ROUTELOC0 = ((nt3h2x11_i2c_port->ROUTELOC0 & (~_I2C_ROUTELOC0_SCLLOC_MASK)) | (i2c_init.scl_loc << _I2C_ROUTELOC0_SCLLOC_SHIFT));

#elif (_SILICON_LABS_32B_SERIES == 2)
  // Route GPIO pins to I2C module */
  GPIO->I2CROUTE[0].SDAROUTE = ((GPIO->I2CROUTE[0].SDAROUTE & ~_GPIO_I2C_SDAROUTE_MASK)
                              | (i2c_init.sda_port << _GPIO_I2C_SDAROUTE_PORT_SHIFT
                              | (i2c_init.sda_pin  << _GPIO_I2C_SDAROUTE_PIN_SHIFT)));
  GPIO->I2CROUTE[0].SCLROUTE = ((GPIO->I2CROUTE[0].SCLROUTE & ~_GPIO_I2C_SCLROUTE_MASK)
                              | (i2c_init.scl_port << _GPIO_I2C_SCLROUTE_PORT_SHIFT
                              | (i2c_init.scl_pin  << _GPIO_I2C_SCLROUTE_PIN_SHIFT)));
  GPIO->I2CROUTE[0].ROUTEEN = GPIO_I2C_ROUTEEN_SDAPEN | GPIO_I2C_ROUTEEN_SCLPEN;
#endif

/* Initializing the I2C */
I2C_Init(nt3h2x11_i2c_port, &i2cInit);
}

/**************************************************************************//**
 * @brief
 *  Read data through i2c.
 *
 * @param[in] address
 *  Data address to store read data
 *
 * @param[in] size
 *  Number of bytes to be read
 *
 * @param[out] pdata
 *  Buffer to store read data
 *
 * @returns
 *  I2C transfer result code.
 *****************************************************************************/
static i2c_transfer_return_t nt3h2x11_internal_i2c_read (uint8_t address,  uint32_t size, uint8_t* pdata) {

  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef result;

  seq.addr  = address;
  seq.flags = I2C_FLAG_READ;

  /* Select location/length of data to be read */
  seq.buf[0].data = pdata;
  seq.buf[0].len  = size;

  result = I2C_TransferInit(nt3h2x11_i2c_port, &seq);

  while (result == i2cTransferInProgress)
  {
    result = I2C_Transfer(nt3h2x11_i2c_port);
  }

  return result;

}

/**************************************************************************//**
 * @brief
 *  Write data through i2c.
 *
 * @param[in] address
 *  Data address to store read data
 *
 * @param[in] size
 *  Number of bytes to be written
 *
 * @param[out] pdata
 *  Buffer of data to be written
 *
 * @returns
 *  I2C transfer result code.
 *****************************************************************************/
static i2c_transfer_return_t nt3h2x11_internal_i2c_write (uint8_t address, uint32_t size, uint8_t* pdata) {

  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef result;

  seq.addr        = address;
  seq.flags       = I2C_FLAG_WRITE;
  seq.buf[0].data = pdata;
  seq.buf[0].len  = size;

  result = I2C_TransferInit(nt3h2x11_i2c_port, &seq);

  while (result == i2cTransferInProgress)
  {
    result = I2C_Transfer(nt3h2x11_i2c_port);
  }

  return result;
}

/**************************************************************************//**
 * @brief
 *  Read from NT3H2x11 via I2C.
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
i2c_transfer_return_t nt3h2x11_i2c_read (uint8_t mema, uint8_t* data) {
  /* Write memory address. */
  nt3h2x11_internal_i2c_write(NT3H2X11_DEFAULT_I2C_ADDR, 1, &mema);
  /* Write a block. */
  return nt3h2x11_internal_i2c_read(NT3H2X11_DEFAULT_I2C_ADDR, NT3H2X11_BLOCK_SIZE, data);
}

/**************************************************************************//**
 * @brief
 *   Write to NT3H2x11 via I2C.
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
i2c_transfer_return_t nt3h2x11_i2c_write (uint8_t mema, uint8_t* data) {
  /* Buffer to hold mema and data. */
  uint8_t buff[17];
  /* Assign mema. */
  buff[0] = mema;
  /* Assign data. */
  memcpy(&buff[1], data, NT3H2X11_BLOCK_SIZE);
  /* Write to NT3H2x11. */
  return nt3h2x11_internal_i2c_write(NT3H2X11_DEFAULT_I2C_ADDR, NT3H2X11_BLOCK_SIZE + 1, buff);
}

/**************************************************************************//**
 * @brief
 *  Read data from a register in NT3H2x11.
 *
 * @param[in] mema
 *  Memory address
 *
 * @param[in] rega
 *  Register address
 *
 * @param[out] regdat
 *  Buffer to hold register data
 *
 * @returns
 *  I2C transfer status.
 *
 * @note
 *  Details for I2C READ register operation, please refer to NT3H2111_2211
 *  product data sheet section 9.8.
 *****************************************************************************/
i2c_transfer_return_t nt3h2x11_i2c_read_reg (uint8_t mema, uint8_t rega, uint8_t* regdat) {
  /* Buffer to hold mema, rega and data. */
  uint8_t buff[2];
  /* Assign mema. */
  buff[0] = mema;
  /* Assign rega. */
  buff[0] = rega;
  /* Write addresses. */
  nt3h2x11_internal_i2c_write(NT3H2X11_DEFAULT_I2C_ADDR, 2, buff);
  /* Read regdat from NT3H2x11.  */
  return nt3h2x11_internal_i2c_read(NT3H2X11_DEFAULT_I2C_ADDR, 1, regdat);
}

/**************************************************************************//**
 * @brief
 *  Write data to a register in NT3H2x11.
 *
 * @param[in] mema
 *  Memory address
 *
 * @param[in] rega
 *  Register address
 *
 * @param[in] regdat
 *  Data to be written to targeted register
 *
 * @returns
 *  I2C transfer status.
 *
 * @note
 *  Details for I2C WRITE register operation, please refer to NT3H2111_2211
 *  product data sheet section 9.8.
 *****************************************************************************/
i2c_transfer_return_t nt3h2x11_i2c_write_reg (uint8_t mema, uint8_t rega, uint8_t regdat) {
  /* Buffer to hold mema, rega and data. */
  uint8_t buff[3];
  /* Assign mema. */
  buff[0] = mema;
  /* Assign rega. */
  buff[1] = rega;
  /* Assign regdat. */
  buff[2] = regdat;
  /* Write to NT3H2x11. */
  return nt3h2x11_internal_i2c_write(NT3H2X11_DEFAULT_I2C_ADDR, 3, buff);
}

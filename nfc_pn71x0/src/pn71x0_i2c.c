/***************************************************************************//**
 * @file   pn71x0_i2c.c
 * @brief  Implementation for I2C interface and NCI TML for PN71x0.
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
#include "em_i2c.h"
#include "em_gpio.h"
#include "../inc/pn71x0_i2c.h"
#include "nci.h"
#include "nci_tml.h"

/// PN71x0 default I2C address
#define PN71X0_DEFAULT_I2C_ADDR         (0x28 << 1)
/// PN71x0 I2C address used in this file
#define PN71X0_I2C_ADDR                 PN71X0_DEFAULT_I2C_ADDR
/// Number of times a I2C operation can be retried
#define PN71X0_I2C_MAX_RETRIES          (3)
/// I2C port to be used
static I2C_TypeDef *pn71x0_i2c_port;

/**************************************************************************//**
 * @brief
 *  I2C initialization for PN71x0 communication.
 *
 * @param[in] i2c_init
 *  I2C pin settings for PN71x0 communication.
 *****************************************************************************/
void pn71x0_i2c_init (pn71x0_i2c_init_t i2c_init) {

  pn71x0_i2c_port = i2c_init.i2c_port;

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
  pn71x0_i2c_port->ROUTEPEN = I2C_ROUTEPEN_SDAPEN | I2C_ROUTEPEN_SCLPEN;
  pn71x0_i2c_port->ROUTELOC0 = ((pn71x0_i2c_port->ROUTELOC0 & (~_I2C_ROUTELOC0_SDALOC_MASK)) | (i2c_init.sda_loc << _I2C_ROUTELOC0_SDALOC_SHIFT));
  pn71x0_i2c_port->ROUTELOC0 = ((pn71x0_i2c_port->ROUTELOC0 & (~_I2C_ROUTELOC0_SCLLOC_MASK)) | (i2c_init.scl_loc << _I2C_ROUTELOC0_SCLLOC_SHIFT));

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
  I2C_Init(pn71x0_i2c_port, &i2cInit);
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
static i2c_transfer_return_t pn71x0_i2c_read (uint8_t address,  uint32_t size, uint8_t* pdata) {

  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef result;

  seq.addr  = address;
  seq.flags = I2C_FLAG_READ;

  /* Select location/length of data to be read */
  seq.buf[0].data = pdata;
  seq.buf[0].len  = size;

  result = I2C_TransferInit(pn71x0_i2c_port, &seq);

  while (result == i2cTransferInProgress) {
      result = I2C_Transfer(pn71x0_i2c_port);
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
static i2c_transfer_return_t pn71x0_i2c_write (uint8_t address, uint32_t size, uint8_t* pdata) {

  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef result;

  seq.addr        = address;
  seq.flags       = I2C_FLAG_WRITE;
  seq.buf[0].data = pdata;
  seq.buf[0].len  = size;

  result = I2C_TransferInit(pn71x0_i2c_port, &seq);

  while (result == i2cTransferInProgress) {
      result = I2C_Transfer(pn71x0_i2c_port);
  }

  return result;
}

/**************************************************************************//**
 * @brief
 *  NCI TML receive function wrapper for PN71x0 I2C.
 *
 * @param[out] packet
 *  Packet buffer to hold the received packet.
 *
 * @returns
 *  Any error code.
 *****************************************************************************/
nci_tml_err_t nci_tml_receive (uint8_t* packet) {

  i2c_transfer_return_t i2c_ret;

  for (int i = 0; i < PN71X0_I2C_MAX_RETRIES; i++) {

      i2c_ret = pn71x0_i2c_read(PN71X0_I2C_ADDR, 3, packet);

      if (i2c_ret == i2cTransferDone) {
          break;
      }
  }

  if (i2c_ret != i2cTransferDone) {
      return nci_tml_err_comm_bus;
  }

  if (packet[2] != 0) {
      for (int i = 0; i < PN71X0_I2C_MAX_RETRIES; i++) {
          /* . */
          i2c_ret = pn71x0_i2c_read(PN71X0_I2C_ADDR, packet[2], &packet[3]);

          if (i2c_ret == i2cTransferDone) {
              break;
          }
      }
      /* . */
      if (i2c_ret != i2cTransferDone) {
          return nci_tml_err_comm_bus;
      }
  }

  nci_tml_log("NCI TML receive:    ");
  nci_tml_packet_log(packet, packet[2] + 3);
  nci_tml_log_ln(" ");

  return nci_tml_err_none;
}

/**************************************************************************//**
 * @brief
 *  NCI TML transceive function wrapper for PN71x0 I2C.
 *
 * @param[in] packet
 *  Packet buffer to hold the packet to be sent.
 *
 * @returns
 *  Any error code.
 *****************************************************************************/
nci_tml_err_t nci_tml_transceive (uint8_t* packet) {

  i2c_transfer_return_t i2c_ret;

  for (int i = 0; i < PN71X0_I2C_MAX_RETRIES; i++) {

      i2c_ret = pn71x0_i2c_write(PN71X0_I2C_ADDR, packet[2] + 3, packet);

      if (i2c_ret == i2cTransferDone) {
          break;
      }
  }

  if( i2c_ret != i2cTransferDone) {
      return nci_tml_err_comm_bus;
  }

  nci_tml_log("NCI TML transceive: ");
  nci_tml_packet_log(packet, packet[2] + 3);
  nci_tml_log_ln(" ");

  return nci_tml_err_none;
}

/***************************************************************************//**
 * @file rfid.c
 * @brief RFID driver
 * @version 1.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
 * This code has been minimally tested to ensure that it builds with
 * the specified dependency versions and is suitable as a demonstration for
 * evaluation purposes only.
 * This code will be maintained at the sole discretion of Silicon Labs.
 *
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "em_cmu.h"
#include "em_eusart.h"
#include "em_gpio.h"

#include "sl_status.h"

#include "rfid_config.h"
#include "rfid.h"


typedef struct {
  volatile char     *rx_buffer_ptr;     // Pointer to store the received data
  volatile uint16_t rx_buffer_idx;      // Index to place next received char
  uint16_t          rx_buffer_size;     // Max size of the receive buffer
  volatile bool     rx_is_complete;     // Indicator that a full packet has
                                        //    been received
  volatile bool     rx_is_processed;    // Indicator that the received packet
                                        //    has been processed by the
                                        //    application

  rfid_uid_t        latest_data;        // The UID of the last card scanned
} rfid_handle_t;

static rfid_handle_t handle;

/***************************************************************************//**
 * @brief
 *    Initialize the EUART peripheral and configure the RX pin
 ******************************************************************************/
static void euart_init(void)
{
  // Select LFXO for the EUART Clock
  CMU_LFXOInit_TypeDef lfxo_init = CMU_LFXOINIT_DEFAULT;
  CMU_LFXOInit(&lfxo_init);

  // set the EM23 Group A clock source to LFXO and enable it.
  CMU_ClockSelectSet(cmuClock_EM23GRPACLK, cmuSelect_LFXO);
  CMU_ClockEnable(cmuClock_EM23GRPACLK, true);

  // set EUART RX route to RX GPIO
  GPIO->EUARTROUTE->RXROUTE = (RFID_RX_PORT << _GPIO_EUART_RXROUTE_PORT_SHIFT)
                             | (RFID_RX_PIN << _GPIO_EUART_RXROUTE_PIN_SHIFT);

  // enable EUART clock
  CMU_ClockSelectSet(cmuClock_EUART0, cmuSelect_EM23GRPACLK);
  CMU_ClockEnable(cmuClock_EUART0, true);

  // EUART initialization
  EUSART_UartInit_TypeDef euart_init = EUSART_UART_INIT_DEFAULT_LF;
  euart_init.baudrate = 9600;

  EUSART_UartInitLf(EUART0, &euart_init);

  // enable interrupts for received data
  EUSART_IntEnable(EUART0, EUSART_IEN_RXFLIEN);

  // enable nvic
  NVIC_ClearPendingIRQ(EUART0_RX_IRQn);
  NVIC_EnableIRQ(EUART0_RX_IRQn);
}


/***************************************************************************//**
 * @brief
 *    Initialize the GPIO peripheral and configure the IO
 ******************************************************************************/
static void gpio_init(void)
{
  // enable the gpio clock
  CMU_ClockEnable(cmuClock_GPIO, true);

  // set RX pin as input
  GPIO_PinModeSet(RFID_RX_PORT, RFID_RX_PIN, gpioModeInput, 1);
}


/***************************************************************************//**
 * @brief
 *    Initialize the RFID handle
 *
 * @param[in] init
 *    RFID configuration struct
 *
 * @return
 *    sl_status_t
 ******************************************************************************/
sl_status_t rfid_init(rfid_init_t *init)
{
  // ensure that buffer size is correct and ptr is not NULL
  if(init->rx_buffer_size < RFID_PACKET_LEN || init->rx_buffer_ptr == NULL) {
      return SL_STATUS_INVALID_PARAMETER;
  }

  // initialize GPIO
  gpio_init();

  // initialize EUART
  euart_init();

  // setup handle
  handle.rx_buffer_ptr    = init->rx_buffer_ptr;
  handle.rx_buffer_size   = init->rx_buffer_size;
  handle.rx_buffer_idx    = 0;
  handle.rx_is_complete   = false;
  handle.rx_is_processed  = false;

  return SL_STATUS_OK;
}


/***************************************************************************//**
 * @brief
 *    Copy valid packet to RFID handle
 *
 * @param[in]  rfid_packet
 *    Valid received packet containing card UID
 *
 * @param[out] rfid
 *    RFID handle to store UID in.
 *
 * @return
 *    sl_status_t
 ******************************************************************************/
static sl_status_t rfid_format(char *rfid_packet, rfid_uid_t *rfid)
{
  // ensure parameters are not null
  if(rfid_packet == NULL || rfid == NULL) {
      return SL_STATUS_INVALID_PARAMETER;
  }

  // set data valid status
  rfid->valid = true;

  // copy card uid from packet to card_uid
  memcpy(rfid->card_uid, rfid_packet, RFID_UID_LEN);

  return SL_STATUS_OK;
}


/***************************************************************************//**
 * @brief
 *    Handle new UART char
 *
 * @param[in]  rfid_char
 *    Received UART char to process
 ******************************************************************************/
static void process_char(char rfid_char)
{
  // is_data_valid is set to true when a start of packet frame has been received
  //  and is set to false when the end of packet frame is received
  static bool is_data_valid = false;

  // a start of frame has been received, store incoming char until end of packet
  if(is_data_valid && !(rfid_char == PACKET_START)) {
      // check for overflow
      if(handle.rx_buffer_idx < handle.rx_buffer_size + 1) {
          // check if packet is complete
          if(rfid_char == PACKET_END) {
              handle.rx_is_processed  = false;
              handle.rx_is_complete   = true;
              handle.rx_buffer_idx    = 0;
              is_data_valid           = false;
          } else {
              // store char into buffer
              handle.rx_buffer_ptr[handle.rx_buffer_idx++] = rfid_char;
              handle.rx_is_complete = false;
          }
      }
  } else if(rfid_char == PACKET_START) {
      // set incoming data as valid
      is_data_valid = true;
      // reset rx buffer index
      handle.rx_buffer_idx = 0;
  }
}


/***************************************************************************//**
 * @brief
 *    Convert ASCII to decimal representation
 *
 *    Sourced from tinyprintf a2d()
 ******************************************************************************/
static int8_t ascii_to_int(char ch)
{
  if (ch >= '0' && ch <= '9')
      return ch - '0';
  else if (ch >= 'a' && ch <= 'f')
      return ch - 'a' + 10;
  else if (ch >= 'A' && ch <= 'F')
      return ch - 'A' + 10;
  else
      return -1;
}

/***************************************************************************//**
 * @brief
 *    Validate packet checksum
 *
 *    ID-12LA uses a binary exclusive or checksum
 *
 * @return
 *    true if checksum matches calculated
 ******************************************************************************/
static bool is_valid_checksum(volatile char* rfid_packet)
{
  // get checksum sent in packet
  uint16_t checksum = ascii_to_int(rfid_packet[10]) << 8 | ascii_to_int(rfid_packet[11]);

  // placeholder for calculated checksum
  uint16_t calc = 0;
  for(uint8_t index = 0; index < RFID_UID_LEN;) {
      // calculate the xor checksum from the card_uid received in packet
      calc ^= ascii_to_int(rfid_packet[index]) << 8 | ascii_to_int(rfid_packet[index+1]);

      index = index + 2;
  }

  // ensure calculated checksum matches checksum set in packet
  if(calc != checksum) {
      return false;
  }

  return true;
}


/***************************************************************************//**
 * @brief
 *    Process complete & valid packet reception
 *
 * @return
 *    sl_status_t
 ******************************************************************************/
sl_status_t rfid_process_action(void)
{
  // check to see if there is data to process
  if(!handle.rx_is_processed & handle.rx_is_complete) {
      handle.rx_is_processed = true;
      handle.rx_is_complete = false;

      // validate checksum
      if(!is_valid_checksum(handle.rx_buffer_ptr)) {
          handle.latest_data.valid = false;
          return SL_STATUS_INVALID_COUNT;
      }

      // copy card_uid from packet to handle card_uid
      return rfid_format((char *)handle.rx_buffer_ptr, &handle.latest_data);
  }

  return SL_STATUS_IS_WAITING;
}


/***************************************************************************//**
 * @brief
 *    Get the last read card UID
 *
 * @param[out] card_uid
 *    Buffer to store last read card UID
 *
 * @return
 *    sl_status_t
 ******************************************************************************/
sl_status_t rfid_get_data(rfid_uid_t *card_uid)
{
  // ensure parameter is not null
  if(card_uid == NULL) {
      return SL_STATUS_INVALID_PARAMETER;
  }

  // get the last card scanned
  memcpy(card_uid, &handle.latest_data, sizeof(rfid_uid_t));

  return SL_STATUS_OK;
}


/***************************************************************************//**
 * @brief
 *    EUART RX Interrupt Handler
 ******************************************************************************/
void EUART0_RX_IRQHandler(void)
{
  // get set flags
  uint32_t flags = EUSART_IntGet(EUART0);

  // process received char
  if(flags & EUSART_IF_RXFLIF) {
      process_char((char)(EUART0->RXDATA));
  }

  // clear flags
  EUSART_IntClear(EUART0, flags);
}

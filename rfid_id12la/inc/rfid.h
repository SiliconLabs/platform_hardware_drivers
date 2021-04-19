/***************************************************************************//**
 * @file rfid.h
 * @brief RFID driver header
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

#ifndef _RFID_H_
#define _RFID_H_

#include <stdint.h>
#include <stdbool.h>

#include "sl_status.h"

#define RFID_UID_LEN    10      // 10 ASCII defined by the datasheet
#define RFID_PACKET_LEN 14      // [DATA(10) + CHECKSUM(2) + CR(1) + LF(1)] = 14

#define PACKET_START    0x02
#define PACKET_END      0x03

typedef struct {
  char  card_uid[RFID_UID_LEN]; // buffer to store last card read
  bool  valid;                  // state of data in buffer
} rfid_uid_t;

typedef struct {
  char      *rx_buffer_ptr;     // pointer to static buffer
  uint16_t   rx_buffer_size;    // size of buffer
} rfid_init_t;

#define RFID_DECLARE_RX_BUFFER    static char rfid_rx_buffer[RFID_PACKET_LEN]

#define RFID_INIT_DEFAULT                                                     \
{                                                                             \
  rfid_rx_buffer,                                                             \
  RFID_PACKET_LEN                                                             \
}

sl_status_t rfid_init(rfid_init_t *init);
sl_status_t rfid_process_action(void);
sl_status_t rfid_get_data(rfid_uid_t *card_uid);

#endif /* _RFID_H_ */

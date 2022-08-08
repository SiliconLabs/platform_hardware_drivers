/***************************************************************************//**
 * @file rfid_id12la.h
 * @brief Header file of id12la rfid driver
 * @version 1.0
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

#ifndef RFID_ID12LA_H_
#define RFID_ID12LA_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include "stddef.h"
#include "sl_status.h"
#include "sl_i2cspm_instances.h"
#include "sl_sleeptimer.h"

/***************************************************************************//**
 * @addtogroup id12la - RFID
 * @brief Silicon Labs ID12LA RFID
 *
 * @n @section rfid_id12la ID12LA example code
 *
 * Basic example for RFID : @n @n
 * @code{.c}
 *
 * #include "sl_i2cspm_instances.h"
 * #include "rfid_id12la.h"
 *
 * static id12la_all_tag_t id12la_all_tag_data;
 * static uint8_t count_tag =0;
 * int main( void )
 * {
 *
 * ...
 *
 *   id12la_init(sl_i2cspm_qwiic);
 *
 *   while(1)
 *   {
 *
 *     id12la_get_all_tag(&id12la_all_tag_data, &count_tag);
 *
 * ...
 *
 *   }
 * } @endcode
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @name Sensor Defines
 * @{
 ******************************************************************************/

/* Communicate with the ID12LA over I2C */
#define ID12LA_DEFAULT_ADDRESS      0x7D
#define ID12LA_MAX_STORAGE_TAG      20
#define ID12LA_ADDRESS_LOCATION     0xC7

typedef struct {
  uint8_t id_tag[6];

  uint32_t time;

  /* true or false */
  bool checksum_valid;
} id12la_tag_t;

typedef struct {
  id12la_tag_t id12la_data[ID12LA_MAX_STORAGE_TAG];
} id12la_tag_list_t;

/***************************************************************************//**
 * @brief
 *    Initialize the id12la
 *
 * @param[in] i2cspm
 *    The I2C peripheral to use.
 *
 * @return
 *    sl_status_t error code
 ******************************************************************************/
sl_status_t id12la_init(sl_i2cspm_t *i2cspm);

/***************************************************************************//**
 * @brief
 *    This function gets all the available tags on Qwiic RFID Reader's buffer.
 *    The buffer on the Qwiic RFID can hold up to 20 tags and their scan times.
 *    This function can be called from the callback function of the
 *    corresponding interrupt pin.
 *
 * @param[in] tag_list
 *    pointer to the buffer where the all tag's information is stored
 *
 * @param[in] tag_count
 *    pointer to the value where tag count is stored
 *
 * @return
 *    sl_status_t error code
 ******************************************************************************/
sl_status_t id12la_get_all_tag(id12la_tag_list_t *tag_list, uint8_t *tag_count);

/***************************************************************************//**
 * @brief
 *    This function is used to change the i2c address of the rfid,
 *    this function is called only when the id12la_begin function is called
 *    before.
 *
 * @param[in] newAddress of rfid
 *
 * @return
 *    sl_status_t error code
 ******************************************************************************/
sl_status_t id12la_change_address_i2c(uint8_t new_address);

/***************************************************************************//**
 * @brief
 *    This function is used to scan and update the address of RFID
 *
 * @return
 *    sl_status_t error code
 ******************************************************************************/
sl_status_t id12la_scan_address(void);

/***************************************************************************//**
 * @brief
 *    This function is used to get the address of RFID
 *
 * @return
 *    Address of the RFID
 ******************************************************************************/
uint8_t id12la_get_i2c_address(void);

#ifdef __cplusplus
}
#endif
#endif /* RFID_ID12LA_H_ */

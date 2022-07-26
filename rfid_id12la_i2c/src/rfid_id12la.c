/***************************************************************************//**
 * @file rfid_id12la.c
 * @brief Platform independent driver for id12la RFID
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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rfid_id12la.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
static sl_i2cspm_t *id12la_i2cpsm_instance;
static bool id12la_is_initialized = false;
static uint8_t id12la_address_i2c = 0x7D;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// Local prototypes
static sl_status_t i2c_write_blocking(uint8_t *src, uint32_t len);
static sl_status_t i2c_read_blocking(uint8_t *dest, uint32_t len);
static bool compare_checksum(uint8_t *ptag);
static sl_status_t get_tag(uint8_t *i2c_rx_buffer);

/** @endcond */

/***************************************************************************//**
 *    Initialize the id12la
 ******************************************************************************/
sl_status_t id12la_init(sl_i2cspm_t *i2cspm)
{
  sl_status_t ret;

  if (i2cspm == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // If already initialized, return status
  if (id12la_is_initialized == true) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  // Update i2cspm instance
  id12la_i2cpsm_instance = i2cspm;

  ret = i2c_write_blocking(NULL, 0);
  if (ret != SL_STATUS_OK) {
    return ret;
  }

  id12la_is_initialized = true;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *    gets all the available tags on Qwiic RFID Reader's buffer
 ******************************************************************************/
sl_status_t id12la_get_all_tag(id12la_tag_list_t *tag_list, uint8_t *tag_count)
{
  sl_status_t ret;
  uint8_t i;
  uint8_t tag_infor[10] = { 0 };

  if (!id12la_i2cpsm_instance) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (tag_list == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (tag_count == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  for (i = 0; i < ID12LA_MAX_STORAGE_TAG; i++) {
    ret = get_tag(tag_infor);
    if (ret != SL_STATUS_OK) {
      break;
    }

    /*
     * if checksum = 0 => id of tag = 0 => The sensor's buffer has no data
     * left => return count
     */
    if (tag_infor[5] == 0) {
      break;
    }

    tag_list->id12la_data[i].checksum_valid = compare_checksum(tag_infor);

    for (uint8_t j = 0; j <= 5; j++) {
      tag_list->id12la_data[i].id_tag[j] = tag_infor[j];
    }

    tag_list->id12la_data[i].time = (uint32_t)(tag_infor[6] << 24)
                                    | (uint32_t)(tag_infor[7] << 16)
                                    | (uint32_t)(tag_infor[8] << 8)
                                    | (uint32_t)(tag_infor[9]);
  }
  // update tag count.
  *tag_count = i;

  return ret;
}

/***************************************************************************//**
 *    change the i2c address of the rfid
 ******************************************************************************/
sl_status_t id12la_change_address_i2c(uint8_t new_address)
{
  sl_status_t ret;

  if (!id12la_i2cpsm_instance) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Range of legal addresses
  if ((new_address < 0x07) || (new_address > 0x78)) {
    return SL_STATUS_FAIL;
  }

  // read 10 byte from I2C slave 5 byte ID+1 byte check sum + 4 byte time:

  uint8_t data_to_send[2];
  data_to_send[0] = ID12LA_ADDRESS_LOCATION;
  data_to_send[1] = new_address;

  ret = i2c_write_blocking(data_to_send, 2);

  if (ret == SL_STATUS_OK) {
    id12la_address_i2c = new_address;
  }
  return ret;
}

/***************************************************************************//**
 *    scan and update the address of RFID
 ******************************************************************************/
sl_status_t id12la_scan_address()
{
  if (!id12la_i2cpsm_instance) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  for (uint8_t address = 1; address < 127; address++) {
    id12la_address_i2c = address;
    if (i2c_write_blocking(NULL, 0) == SL_STATUS_OK) {
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_FAIL;
}

/***************************************************************************//**
 *    get the address of RFID
 ******************************************************************************/
uint8_t id12la_get_i2c_address(void)
{
  return id12la_address_i2c;
}

/***************************************************************************//**
 *    get the RFID tag's information from the Qwiic RFID Reader
 ******************************************************************************/
static sl_status_t get_tag(uint8_t *i2c_rx_buffer)
{
  sl_status_t ret;

  ret = i2c_read_blocking(i2c_rx_buffer, 10);
  sl_sleeptimer_delay_millisecond(20);
  return ret;
}

/***************************************************************************//**
 * @brief
 *    This function calculate checksum and compare it to checksum value of
 *    tag's information
 *
 * @param[in] pTag
 *    pointer to the buffer where the tag's information is stored
 * @return
 *    1: checksum correct
 *    0: checksum not correct
 ******************************************************************************/
static bool compare_checksum(uint8_t *ptag)
{
  uint8_t checksum = ptag[0] ^ ptag[1] ^ ptag[2] ^ ptag[3] ^ ptag[4];
  if (checksum == ptag[5]) {
    return true;
  } else {
    return false;
  }
}

/*Block write to RFID*/
static sl_status_t i2c_write_blocking(uint8_t *src, uint32_t len)
{
  I2C_TransferSeq_TypeDef seq;

  seq.addr = id12la_address_i2c << 1;
  seq.flags = I2C_FLAG_WRITE;

  /*Write buffer*/
  seq.buf[0].data = src;
  seq.buf[0].len = len;

  if (I2CSPM_Transfer(id12la_i2cpsm_instance, &seq) != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }

  return SL_STATUS_OK;
}

/* Block read from RFID */
static sl_status_t i2c_read_blocking(uint8_t *dest, uint32_t len)
{
  I2C_TransferSeq_TypeDef seq;

  seq.addr = id12la_address_i2c << 1;
  seq.flags = I2C_FLAG_READ;

  /*Read buffer*/
  seq.buf[0].data = dest;
  seq.buf[0].len = len;

  if (I2CSPM_Transfer(id12la_i2cpsm_instance, &seq) != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }

  return SL_STATUS_OK;
}

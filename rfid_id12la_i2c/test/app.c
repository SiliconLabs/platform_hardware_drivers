/***************************************************************************//**
 * @file app.c
 * @brief Top level application functions
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
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/
#include "app_log.h"
#include "app_assert.h"
#include "sl_i2cspm_instances.h"
#include "sl_sleeptimer.h"
#include "rfid_id12la.h"

static id12la_tag_list_t id12la_all_tag_data;
static uint8_t count_tag = 0;

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  sl_status_t ret;

  ret = id12la_init(sl_i2cspm_qwiic);

  if (ret != SL_STATUS_OK) {
    app_log("i2c address has been changed before\n");
    ret = id12la_scan_address();
    app_assert_status(ret);
    app_log("I2C address is: 0x%02X\n", id12la_get_i2c_address());
    app_log("rfid begins successfully, ready to scan some tags\n");
  } else {
    app_log("rfid inits successfully, ready scans some tags\n");
  }
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  if (id12la_get_all_tag(&id12la_all_tag_data, &count_tag) != SL_STATUS_OK) {
    app_log("error while scanning tags, check connection!!!\n");
  }

  if (count_tag > 0) {
    app_log("count tag: %d\n", count_tag);

    for (uint8_t i = 0; i < count_tag; i++) {
      if (id12la_all_tag_data.id12la_data[i].checksum_valid == true) {
        app_log("ID (last byte is checksum): 0x%02X 0x%02X 0x%02X 0x%02X \
                0x%02X 0x%02X\n",
                id12la_all_tag_data.id12la_data[i].id_tag[0], \
                id12la_all_tag_data.id12la_data[i].id_tag[1], \
                id12la_all_tag_data.id12la_data[i].id_tag[2], \
                id12la_all_tag_data.id12la_data[i].id_tag[3], \
                id12la_all_tag_data.id12la_data[i].id_tag[4], \
                id12la_all_tag_data.id12la_data[i].id_tag[5]);
        app_log("Scan time: %d\n\n", id12la_all_tag_data.id12la_data[i].time);
      } else {
        app_log("Tag %d : checksum error, please scan the tag again\n");
      }
    }
  }
}

/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "em_common.h"
#include "sl_status.h"

#include "rfid.h"

RFID_DECLARE_RX_BUFFER;

static rfid_uid_t rfid_uid;

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  // rfid default initialization
  rfid_init_t init = RFID_INIT_DEFAULT;
  rfid_init(&init);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  sl_status_t status;

  // check for new data
  status = rfid_process_action();

  if(status == SL_STATUS_OK) {
      // copy valid uid into rfid_uid
      if(rfid_uid.valid) {
          rfid_get_data(&rfid_uid);
          __BKPT();                     // use debugger to view rfid_uid
      }
  } else if(status == SL_STATUS_INVALID_COUNT){
      // uh oh checksum error
      __BKPT();
  }
}

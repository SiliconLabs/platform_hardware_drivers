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

#include "sl_status.h"
#include "gps.h"
#include "cmsis_gcc.h"

GPS_DECLARE_RX_BUFFER;
GPS_DECLARE_TX_BUFFER;

static gps_data_t gps_data;

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  gps_init_t init = GPS_INIT_DEFAULT;

  gps_init(&init);
  gps_enable(true);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  sl_status_t sc = gps_process_action();
  if(sc == SL_STATUS_OK) {
    gps_get_data(&gps_data);

    // Check if data is valid
    if(gps_data.status) {
      __BKPT();   // Using the debugger, check the gps_data contents
    }
  }
}

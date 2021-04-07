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

#include <sl_status.h>
#include "sl_i2cspm.h"
#include "sl_app_log.h"
#include <mlx90632.h>
#include <mlx90632_i2c.h>
#include "sl_udelay.h"

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  mlx90632_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  double  ambient, object;
  int i;
  sl_status_t sc;

  // Perform the measurement
  sc = mlx90632_measurment_cb(&ambient, &object);

  if (sc != SL_STATUS_OK) {
      sl_app_log("Measurement Failure!!!\n");
  }

  if (sc == SL_STATUS_OK) {
    sl_app_log("Ambient: %f C\n", ambient);
    sl_app_log("Object: %f C\n",object);
  }

  // Wait 1 sec
  for (i = 0; i<10; i++) {
    sl_udelay_wait(100000);
  }

  return;
}

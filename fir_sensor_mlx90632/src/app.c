/***************************************************************************//**
 * @file  app.c
 * @brief Top level application functions
 * @version 0.0.1
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
 *******************************************************************************
 * # Evaluation Quality
 * This code has been minimally tested to ensure that it builds and is suitable
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/

#include <sl_status.h>
#include "sl_i2cspm.h"
#include "app_log.h"
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
      app_log("Measurement Failure!!!\n");
  }

  if (sc == SL_STATUS_OK) {
    app_log("Ambient: %f C\n", ambient);
    app_log("Object: %f C\n",object);
  }

  // Wait 1 sec
  for (i = 0; i<10; i++) {
    sl_udelay_wait(100000);
  }

  return;
}

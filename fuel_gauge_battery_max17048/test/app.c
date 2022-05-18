/***************************************************************************//**
* @file app.c
* @brief MAX17048 driver demonstration.
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
* # Evaluation Quality
* This code has been minimally tested to ensure that it builds and is suitable
* as a demonstration for evaluation purposes only. This code will be maintained
* at the sole discretion of Silicon Labs.
*******************************************************************************/

#include "sl_i2cspm_instances.h"
#include "max17048.h"
#include "app_log.h"

static void max17048_soc_callback(sl_max17048_irq_source_t source,
                                  void *data);
/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  sl_status_t status;
  uint32_t vcell, cb_data, soc;
  float crate;
  uint8_t id;
  uint16_t version;

  // Initialize the max17048
  status = max17048_init(sl_i2cspm_inst);
  app_log_status(status);

  // Read and print the device ID
  status = max17048_get_id(&id);
  app_log_status(status);
  app_log("\nID: 0x%x\r\n", id);

  // Read and print the device version
  status = max17048_get_production_version(&version);
  app_log_status(status);
  app_log("Device version: 0x%x\r\n", version);

  // Read and print the battery voltage
  status = max17048_get_vcell(&vcell);
  app_log_status(status);
  app_log("vcell: %d mV\r\n", vcell);

  // Read and print the battery state of charge
  status = max17048_get_soc(&soc);
  app_log_status(status);
  app_log("soc: %d%%\r\n", soc);

  // Read and print the average SOC rate of change
  status = max17048_get_crate(&crate);
  app_log_status(status);
  app_log("crate: %f%%\r\n", crate);

  // Enables the MAX17048 state-of-charge (SOC) interrupt
  status = max17048_enable_soc_interrupt(max17048_soc_callback, &cb_data);
  app_log_status(status);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

static void max17048_soc_callback(sl_max17048_irq_source_t source,
                                  void *data)
{
  (void)source;
  (void)data;
  sl_status_t status;
  uint32_t soc, vcell;
  float crate;

  app_log("\nSOC interrupt >\r\n");
  // Read and print the battery state of charge
  status = max17048_get_soc(&soc);
  app_log_status(status);
  app_log("SOC is now: %d%%\r\n", soc);

  // Read and print the battery voltage
  status = max17048_get_vcell(&vcell);
  app_log_status(status);
  app_log("vcell is now: %d mV\r\n", vcell);

  // Read and print the average SOC rate of change
  status = max17048_get_crate(&crate);
  app_log_status(status);
  app_log("crate is now: %f%%\r\n", crate);
}

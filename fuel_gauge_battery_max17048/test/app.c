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
  app_log("\nid: 0x%x\r\n", id);

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

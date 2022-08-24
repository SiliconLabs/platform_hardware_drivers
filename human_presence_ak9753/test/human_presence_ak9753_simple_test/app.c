/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ********************************************************************************
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

#include <ak9753.h>
#include <ak9753_platform.h>
#include <ak9753_config.h>
#include "app_assert.h"
#include "app_log.h"

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  ak9753_config_t ak9753_cfg = {
    .I2C_address = AK9753_ADDR,
    .ak9753_i2cspm_instance = AK9753_CONFIG_I2C_INSTANCE,
    .cut_off_freq = AK975X_FREQ_8_8HZ,
    .mode = AK975X_MODE_0,
    .upper_threshold13 = 0x00,
    .lower_threshold13 = 0x00,
    .upper_threshold24 = 0x00,
    .lower_threshold24 = 0x00,
    .hysteresis_value24 = 0x00,
    .hysteresis_value13 = 0x00,
    .int_source_setting = 0x00,
    .int_present = false,
    .PDN_present = false
  };
  ak9753_init(&ak9753_cfg);
  ak9753_set_hysteresis_eeprom_ir13(10);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  bool is_avail = false;
  ak9753_is_data_ready(&is_avail);

  if (is_avail) {
    int16_t ret = 0;
    float tempC = 0;
    float tempF = 0;
    app_log("1:DWN[");
    ak9753_get_ir1_data(&ret);
    app_log("%d", ret);
    ak9753_get_ir2_data(&ret);
    app_log("]\t2:LFT[");
    app_log("%d", ret);
    ak9753_get_ir3_data(&ret);
    app_log("]\t3:UP[");
    app_log("%d", ret);
    ak9753_get_ir4_data(&ret);
    app_log("]\t4:RGH[");
    app_log("%d", ret);
    ak9753_get_tempC(&tempC);
    app_log("]\ttempC[");
    app_log("%f", tempC);
    ak9753_get_tempF(&tempF);
    app_log("]\ttempF[");
    app_log("%f", tempF);
    app_log("]\r\n");
    ak9753_get_dummy();
  }
}

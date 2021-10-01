/***************************************************************************//**
* @file app.c
* @brief Cap1166 example application
*******************************************************************************
* # License
* <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
* EXPERIMENTAL QUALITY
* This code has not been formally tested and is provided as-is. It is not
* suitable for production environments. This code will not be maintained.
******************************************************************************/

#include "app_log.h"

#include "spidrv.h"
#include "sl_spidrv_instances.h"

#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

#include "cap1166.h"
#include "cap1166_config.h"

cap1166_handle_t my_cap1166_handle;
cap1166_cfg_t my_cap1166_config = CAP11666_DEFAULT_CONFIG;

#define CAP1166_MTP_PATTERN
//#define CAP1166_PROXIMITY

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  sl_status_t ret;

  /* SPI driver handle */
  my_cap1166_handle.spidrv_handle = sl_spidrv_mikroe_handle;

  /* sensor reset pin */
  my_cap1166_handle.sensor_rst_port = gpioPortC;
  my_cap1166_handle.sensor_rst_pin = 6;

  /* change some configuration */
  my_cap1166_config.led_cfg.led_behavior[0] = CAP1166_OUTPUT_PULSE_1_MODE;
  my_cap1166_config.led_cfg.led_behavior[1] = CAP1166_OUTPUT_PULSE_2_MODE;
  my_cap1166_config.led_cfg.led_behavior[2] = CAP1166_OUTPUT_BREATH_MODE;

  my_cap1166_config.led_cfg.led_link_to_sensor[5] =
      CAP1166_OUTPUT_DO_NOT_LINK_TO_SENSOR;

#ifdef CAP1166_PROXIMITY
  my_cap1166_config.power_state = CAP1166_STANDBY;
#else
  my_cap1166_config.power_state = CAP1166_ACTIVE;
#endif

  /* initialize the sensor */
  ret = cap1166_init(&my_cap1166_handle);
  if(ret == SL_STATUS_OK){
      app_log_info("initialize successfully \r\n");
  }
  else{
      app_log_info("initialize return %02x \r\n", ret);
  }

  /* configure the sensor */
  ret = cap1166_config(&my_cap1166_handle,
                       &my_cap1166_config);
  if(ret == SL_STATUS_OK){
      app_log_info("configuration successfully \r\n");
  }
  else{
      app_log_info("configure return %04x \r\n", ret);
  }

#ifdef CAP1166_MTP_PATTERN
  cap1166_touch_pattern_enable(&my_cap1166_handle,
                               &(my_cap1166_config.pattern_cfg));
#endif

#ifdef CAP1166_PROXIMITY
  cap1166_proximity_detection_enable(&my_cap1166_handle,
                                     &(my_cap1166_config.proximity_cfg));
#endif

}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

/***************************************************************************//**
 * Interrupt handler
 ******************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  sl_status_t ret;
  uint8_t sensor_input[6];
  uint8_t int_reason = 0;

  if((handle == &sl_button_sensor_alt) &&
      (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED)) {

      ret = cap1166_check_interrupt_reason(&my_cap1166_handle,
                                           &int_reason);
      if(ret != SL_STATUS_OK) return;

      if(int_reason & CAP1166_TOUCH_DETECTED_INT_MASK){

          ret = cap1166_detect_touch(&my_cap1166_handle,
                                     sensor_input);

          app_log_info("status %02x touch : %d %d %d %d %d %d \r\n",
                       int_reason,
                       sensor_input[0],
                       sensor_input[1],
                       sensor_input[2],
                       sensor_input[3],
                       sensor_input[4],
                       sensor_input[5]);
      }
      if(int_reason & CAP1166_MULTI_TOUCH_PATTERN_INT_MASK){
          app_log_info("status %02x pattern detected. \r\n", int_reason);
      }
  }
}

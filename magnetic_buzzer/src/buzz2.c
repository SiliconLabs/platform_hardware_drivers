/***************************************************************************//**
* @file buzzer.c
* @brief driver for the buzzer
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
* # Evaluation Quality
* This code has been minimally tested to ensure that it builds and is suitable
* as a demonstration for evaluation purposes only. This code will be maintained
* at the sole discretion of Silicon Labs.
******************************************************************************/

#include "buzz2.h"

/***************************************************************************//**
 * @addtogroup buzz2 BUZZ 2 Click Driver
 * @brief  BUZZ 2 Click Driver.
 * @details
 * @{
 ******************************************************************************/

/***************************************************************************//**
 *  Configures the buzzer 2 click
 ******************************************************************************/
sl_status_t buzz2_cfg_setup(buzz2_t *buzz2,
                            sl_pwm_instance_t pwm,
                            sl_pwm_config_t pwm_config)
{
  buzz2->pwm = pwm;
  buzz2->config = pwm_config;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Initializes the buzzer 2 click
 ******************************************************************************/
sl_status_t  buzz2_init (buzz2_t *buzz2)
{
  sl_status_t retval = SL_STATUS_OK;

  retval = sl_pwm_init(&buzz2->pwm , &buzz2->config);

  return retval;
}

/***************************************************************************//**
 *  Sets the duty cycle which determines the amplitude (sound volume).
 ******************************************************************************/
sl_status_t  buzz2_set_duty_cycle (buzz2_t *buzz2, uint8_t duty_cycle)
{
  sl_pwm_set_duty_cycle(&buzz2->pwm, duty_cycle);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Disables PWM output
 ******************************************************************************/
sl_status_t  buzz2_pwm_stop (buzz2_t *buzz2)
{
  sl_pwm_stop(&buzz2->pwm);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Enables PWM output
 ******************************************************************************/
sl_status_t  buzz2_pwm_start (buzz2_t *buzz2)
{
  sl_pwm_start(&buzz2->pwm);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Creates different sound patterns
 ******************************************************************************/
sl_status_t  buzz2_play_sound (buzz2_t *buzz2,
                               uint16_t freq,
                               uint8_t volume,
                               uint16_t duration)
{
  sl_status_t retval = SL_STATUS_OK;

  buzz2->config.frequency = freq;

  retval = buzz2_init(buzz2);
  if(retval != SL_STATUS_OK){
      return retval;
  }

  buzz2_set_duty_cycle(buzz2, volume);
  buzz2_pwm_start(buzz2);

  while(duration--) {
      sl_sleeptimer_delay_millisecond(1);
  }

  buzz2_pwm_stop(buzz2);

  return retval;
}

/** @} (end group buzz2) */

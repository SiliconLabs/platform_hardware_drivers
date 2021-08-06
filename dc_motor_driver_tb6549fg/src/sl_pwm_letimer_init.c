/***************************************************************************//**
 * @file sl_pwm_letimer_init.c
 * @brief PWM DRIVER for LETIMER instance initialization
 * @version 1.0.0
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
 *
 * EVALUATION QUALITY
 * This code has been minimally tested to ensure that it builds with
 * the specified dependency versions and is suitable as a demonstration for
 * evaluation purposes only.
 * This code will be maintained at the sole discretion of Silicon Labs.
 *
 ******************************************************************************/
// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_pwm_letimer.h"
#include "sl_pwm_letimer_instances.h"
#include "em_gpio.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
sl_pwm_letimer_instance_t sl_pwm_letimer_motor0 = {
  .timer = SL_PWM_LETIMER_MOTOR0_PERIPHERAL,
  .channel = (uint8_t)(SL_PWM_LETIMER_MOTOR0_OUTPUT_CHANNEL),
  .port = (uint8_t)(SL_PWM_LETIMER_MOTOR0_OUTPUT_PORT),
  .pin = (uint8_t)(SL_PWM_LETIMER_MOTOR0_OUTPUT_PIN),
#if defined(SL_PWM_LETIMER_MOTOR0_OUTPUT_LOC)
  .location = (uint8_t)(SL_PWM_LETIMER_MOTOR0_OUTPUT_LOC),
#endif
};

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void sl_pwm_letimer_init_instances(void)
{

  sl_pwm_letimer_config_t pwm_letimer_motor0_config = {
    .frequency = SL_PWM_LETIMER_MOTOR0_FREQUENCY,
    .polarity = SL_PWM_LETIMER_MOTOR0_POLARITY,
  };

  sl_pwm_letimer_init(&sl_pwm_letimer_motor0, &pwm_letimer_motor0_config);

}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

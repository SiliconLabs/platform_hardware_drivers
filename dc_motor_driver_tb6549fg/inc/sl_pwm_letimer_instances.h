/***************************************************************************//**
 * @file sl_pwm_letimer_instance.h
 * @brief PWM DRIVER for LETIMER - instance
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

#ifndef SL_PWM_LETIMER_INSTANCES_H_
#define SL_PWM_LETIMER_INSTANCES_H_

#ifdef __cplusplus
extern "C"
{
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_pwm_letimer.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// For the BGM220 Explorer Kit (BRD4314A)
#define SL_PWM_LETIMER_MOTOR0_PERIPHERAL          LETIMER0
#define SL_PWM_LETIMER_MOTOR0_OUTPUT_CHANNEL      0
#define SL_PWM_LETIMER_MOTOR0_OUTPUT_PORT         gpioPortB
#define SL_PWM_LETIMER_MOTOR0_OUTPUT_PIN          4

#define SL_PWM_LETIMER_MOTOR0_FREQUENCY           1000
#define SL_PWM_LETIMER_MOTOR0_POLARITY            PWM_LETIMER_ACTIVE_HIGH

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
extern sl_pwm_letimer_instance_t sl_pwm_letimer_motor0;

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
void sl_pwm_letimer_init_instances(void);

#ifdef __cplusplus
}
#endif

#endif // SL_PWM_LETIMER_INSTANCES_H_

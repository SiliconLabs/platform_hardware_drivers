/***************************************************************************//**
 * @file sl_pwm_letimer.h
 * @brief PWM DRIVER for LETIMER
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
#ifndef SL_PWM_LETIMER_H_
#define SL_PWM_LETIMER_H_

/***************************************************************************//**
 * @addtogroup pwm PWM LETIMER Driver
 * @brief PWM LETIMER Driver
 * @{
 * The PWM LETIMER driver uses one or more LETIMER peripherals to generate one or more
 * PWM waveform, with configurable frequency, duty cycle, and polarity. Multiple
 * instances of the driver can be created and allocated to their own TIMER
 * channel.
 *
 * The duty cycle of the PWM waveform can be updated, while the PWM driver is
 * running by calling @ref sl_pwm_letimer_set_duty_cycle(), without causing
 * glitches in the output waveform.
 *
 * @note If several PWM driver instances are set up to use the same LETIMER
 * instance, the PWM frequency of these instances must be the same.
 *
 * ### PWM LETIMER Example Code {#pwm_example}
 *
 * Basic example for generating PWM waveform:
 *
 * ```c
 *
 * #include "sl_pwm.h"
 * #include "em_gpio.h"
 *
 * int main( void )
 * {
 *
 *   ...
 *
 *   sl_pwm_letimer_instance_t sl_pwm_letimer_led0 = {
 *     .timer    = LETIMER0,
 *     .channel  = 0,
 *     .port     = gpioPortA,
 *     .pin      = 0,
 *     .location = 0,
 *   };
 *
 *   sl_pwm_letimer_config_t pwm_letimer_led0_config = {
 *     .frequency = 10000,
 *     .polarity  = PWM_ACTIVE_HIGH,
 *   };
 *
 *   // Initialize PWM
 *   sl_pwm_letimer_init(&sl_pwm_letimer_led0, &pwm_letimer_led0_config);
 *
 *   // Set duty cycle to 40%
 *   sl_pwm_letimer_set_duty_cycle(&sl_pwm_letimer_led0, 40);
 *
 *   // Enable PWM output
 *   sl_pwm_letimer_start(&sl_pwm_letimer_led0);
 *
 *   ...
 *
 * }
 * ```
 *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "sl_status.h"
#include "sl_enum.h"
#include <stdint.h>

#include "em_letimer.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/** PWM polarity selection */
SL_ENUM(sl_pwm_letimer_polarity_t) {
  /** PWM polarity active high */
  PWM_LETIMER_ACTIVE_HIGH = 0,
  /** PWM polarity active low */
  PWM_LETIMER_ACTIVE_LOW = 1,
};

/**
 * PWM driver instance
 */
typedef struct sl_pwm_letimer_instance {
  LETIMER_TypeDef *timer; /**< LETIMER instance */
  uint8_t channel;      /**< LETIMER channel */
  uint8_t port;         /**< GPIO port */
  uint8_t pin;          /**< GPIO pin */
  uint8_t location;     /**< GPIO location */
} sl_pwm_letimer_instance_t;

/**
 * PWM driver configuration
 */
typedef struct sl_pwm_letimer_config {
  int frequency;              /**< PWM frequency */
  sl_pwm_letimer_polarity_t polarity; /**< PWM polarity */
} sl_pwm_letimer_config_t;

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
sl_status_t sl_pwm_letimer_init(sl_pwm_letimer_instance_t *pwm,
                                sl_pwm_letimer_config_t *config);
sl_status_t sl_pwm_letimer_deinit(sl_pwm_letimer_instance_t *pwm);
void sl_pwm_letimer_start(sl_pwm_letimer_instance_t *pwm);
void sl_pwm_letimer_stop(sl_pwm_letimer_instance_t *pwm);
void sl_pwm_letimer_set_duty_cycle(sl_pwm_letimer_instance_t *pwm,
                                   uint8_t percent);
uint8_t sl_pwm_letimer_get_duty_cycle(sl_pwm_letimer_instance_t *pwm);

/** @} (end addtogroup pwm) */

#ifdef __cplusplus
}
#endif

#endif // SL_PWM_LETIMER_H_

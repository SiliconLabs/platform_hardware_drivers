/***************************************************************************//**
 * @file dc_motor_TB6549FG.h
 * @brief TB6549FG driver
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

#ifndef DC_MOTOR_TB6549FG_H_
#define DC_MOTOR_TB6549FG_H_

/***************************************************************************//**
 * @addtogroup DC Motor TB6549FG driver
 * @{
 *
 * @brief
 *  The implementation of a DC motor driver using the Toshiba TB6549FG
 *  full-bridge driver. The IC allows to control a single DC brushed motor.
 *
 *  The following table shows the "truth table" for the different operation
 *  modes:
 *
 *  ----------------------------------------------------------------------------
 *  |Input                            |Output
 *  ----------------------------------------------------------------------------
 *  |IN1    | IN2    | SB    | PWM    |OUT1       |OUT2       |Mode            |
 *  ----------------------------------------------------------------------------
 *  |H      |H       |H      |H/L     |L          |L          |Short break     |
 *  |--------------------------------------------------------------------------|
 *  |       |        |       |H       |L          |H          |CW/CCW          |
 *  |L      |H       |H      |--------|----------------------------------------|
 *  |       |        |       |L       |L          |L          |Short break     |
 *  |--------------------------------------------------------------------------|
 *  |       |        |       |H       |H          |L          |CCW/CW          |
 *  |H      |L       |H      |--------|----------------------------------------|
 *  |       |        |       |L       |L          |L          |Short break     |
 *  |--------------------------------------------------------------------------|
 *  |L      |L       |H      |H/L     |OFF (High-Z)           |Stop            |
 *  |--------------------------------------------------------------------------|
 *  |H/L    |H/L     |L      |H/L     |OFF (High-Z)           |Standby         |
 *  ----------------------------------------------------------------------------
 *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdbool.h>
#include "sl_status.h"

#include "dc_motor_TB6549FG_config.h"

#include "sl_pwm_letimer.h"
#include "sl_pwm.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/***************************************************************************//**
 * @brief
 *    Typedef for the TB6549FG GPIO mapping
 ******************************************************************************/
typedef struct tb6549fg_gpio_instance {
  uint8_t IN1_port;   /**< IN1 GPIO port     */
  uint8_t IN1_pin;    /**< IN1 GPIO pin      */
  uint8_t IN2_port;   /**< IN2 GPIO port     */
  uint8_t IN2_pin;    /**< IN2 GPIO pin      */
  uint8_t SB_port;    /**< SLP GPIO port     */
  uint8_t SB_pin;     /**< SLP GPIO pin      */
  uint8_t PWM_port;   /**< PWM GPIO port     */
  uint8_t PWM_pin;    /**< PWM GPIO pin      */
  uint8_t PWM_loc;    /**< PWM GPIO location */
} tb6549fg_gpio_instance_t;

/***************************************************************************//**
 * @brief
 *    Default init struct for the tb6549fg_gpio_instance_t. Based on the
 *    explorer kit BGM220-EK4314A
 ******************************************************************************/
#define TB6549FG_GPIO_INSTANCE_DEFAULT                                         \
  {                                                                            \
    TB6549FG_IN1_PORT,    /* IN1 port */                                       \
    TB6549FG_IN1_PIN,     /* IN1 pin  */                                        \
    TB6549FG_IN2_PORT,    /* IN2 port */                                       \
    TB6549FG_IN2_PIN,     /* IN2 pin  */                                        \
    TB6549FG_SB_PORT,     /* SLP port */                                       \
    TB6549FG_SB_PIN,      /* SLP pin  */                                        \
    TB6549FG_PWM_PORT,    /* PWM port */                                       \
    TB6549FG_PWM_PIN,     /* PWM pin  */                                        \
    TB6549FG_PWM_LOC      /* PWM loc  */                                        \
  }

/***************************************************************************//**
 * @brief
 *    Typedef for the tb6549fg operation mode
 ******************************************************************************/
typedef enum {
  STANDBY_MODE,
  STOP_MODE,
  SHORT_BRAKE_MODE,
  CW_MODE,
  CCW_MODE,
} tb6549fg_mode_t;

/***************************************************************************//**
 * @brief
 *    Structure for the Click board and relevant configurations
 *    TODO: remove sl_pwm_letimer instance once LETIMER is integrated in the PWM
 *          driver. enable_sleep could be removed as well since there will be a
 *          single PWM instance
 ******************************************************************************/
typedef struct tb6549fg_inst {
  tb6549fg_gpio_instance_t gpio;             /**< GPIO config for TB6549FG */
  tb6549fg_mode_t mode;                      /**< Current operation mode   */
  uint32_t calc_pwm_frequency;               /**< Calculated PWM frequency */
  sl_pwm_letimer_instance_t *pwm_letimer;    /**< PWM LETIMER instance     */
  sl_pwm_instance_t *pwm;                    /**< PWM instance             */
  bool enable_sleep;                         /**< Use LETIMER for PWM      */
} tb6549fg_inst_t;

/***************************************************************************//**
 * @brief
 *    Default init struct for the tb6549fg_init function.
 ******************************************************************************/
#define TB6549FG_INST_DEFAULT                                                  \
  {                                                                            \
    TB6549FG_GPIO_INSTANCE_DEFAULT,         /* Default GPIO */                 \
    STANDBY_MODE,                           /* Standby mode as initial state */\
    0,                                      /* PWM frequency */                \
    NULL,                                   /* Pointer to pwm_letimer driver*/ \
    NULL,                                   /* Pointer to pwm driver */        \
    false                                   /* No sleep            */          \
  }

/***************************************************************************//**
 * @brief
 *    Symbol used to determine the delay length for transition between
 *    "stand-by" and "operation" modes (50ms according to TB6549FG datasheet)
 ******************************************************************************/
#define TRANSITION_DELAY_US 50000

/***************************************************************************//**
 * @brief
 *    Symbol used to determine the maximum PWM frequency
 *    (100 kHz according to TB6549FG datasheet)
 ******************************************************************************/
#define MAX_PWM_FREQUENCY 100000

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
sl_status_t tb6549fg_init(tb6549fg_inst_t *inst);
sl_status_t tb6549fg_cw_mode(tb6549fg_inst_t *inst,
                             uint8_t duty_cycle_percent);
sl_status_t tb6549fg_ccw_mode(tb6549fg_inst_t *inst,
                              uint8_t duty_cycle_percent);
sl_status_t tb6549fg_set_pwm_duty_cycle(tb6549fg_inst_t *inst,
                                        uint8_t duty_cycle_percent);
void tb6549fg_short_break_mode(tb6549fg_inst_t *inst);
void tb6549fg_stop_mode(tb6549fg_inst_t *inst);
void tb6549fg_standby_mode(tb6549fg_inst_t *inst);
uint8_t tb6549fg_get_pwm_duty_cycle(tb6549fg_inst_t *inst);

/** @} (end addtogroup DC Motor TB6549FG driver) */

#ifdef __cplusplus
}
#endif

#endif /* DC_MOTOR_TB6549FG_H_ */

/***************************************************************************//**
 * @file stepper_a4988.h
 * @brief Stepper motor driver.
 * @version 1.0.0
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
 *
 * EVALUATION QUALITY
 * This code has been minimally tested to ensure that it builds with
 * the specified dependency versions and is suitable as a demonstration for
 * evaluation purposes only.
 * This code will be maintained at the sole discretion of Silicon Labs.
 *
 ******************************************************************************/

#ifndef STEPPER_A4988_H_
#define STEPPER_A4988_H_

/***************************************************************************//**
 * @addtogroup Stepper motor driver
 * @{
 *
 * @brief
 *  The implementation of a stepper motor driver. It is primary made for the
 *  A4988 stepper motor driver to drive bipolar stepper motors.
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdbool.h>
#include <stepper_config_a4988.h>
#include "sl_status.h"
#include "sl_pwm.h"
#include "sl_sleeptimer.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief
 *    Typedef for specifying the direction of rotation.
 ******************************************************************************/
typedef enum {
  CW,
  CCW
} a4988_stepper_dir_t;

/***************************************************************************//**
 * @brief
 *    Typedef for the state of operation.
 ******************************************************************************/
typedef enum {
  DISABLED,
  IDLE,
  RUNNING
} a4988_stepper_state_t;

/***************************************************************************//**
 * @brief
 *    Typedef for the TB6549FG GPIO mapping
 ******************************************************************************/
typedef struct a4988_stepper_gpio_config {
  uint8_t step_port;
  uint8_t step_pin;
  uint8_t dir_port;
  uint8_t dir_pin;
  uint8_t enable_port;
  uint8_t enable_pin;
  uint8_t rst_port;
  uint8_t rst_pin;
} a4988_stepper_gpio_config_t;

/***************************************************************************//**
 * @brief
 *    Default init struct for the GPIO configuration in the stepper driver
 *    instance. The default values are based on the explorer kit BGM220-EK4314A.
 ******************************************************************************/
#define A4988_STEPPER_GPIO_CONFIG_DEFAULT                                      \
{                                                                              \
  A4988_STEPPER_STEP_PORT,                                                     \
  A4988_STEPPER_STEP_PIN,                                                      \
  A4988_STEPPER_DIR_PORT,                                                      \
  A4988_STEPPER_DIR_PIN,                                                       \
  A4988_STEPPER_ENABLE_PORT,                                                   \
  A4988_STEPPER_ENABLE_PIN,                                                    \
  A4988_STEPPER_RST_PORT,                                                      \
  A4988_STEPPER_RST_PIN                                                        \
}

/***************************************************************************//**
 * @brief
 *    Structure for the stepper driver configuration instance.
 ******************************************************************************/
typedef struct a4988_stepper_inst {
  a4988_stepper_gpio_config_t gpio_config;
  a4988_stepper_state_t state;
  a4988_stepper_dir_t dir;
  uint32_t rpm;
  uint32_t curr_step;
  uint32_t num_step;
  sl_pwm_instance_t *pwm;
  sl_sleeptimer_timer_handle_t *sleeptimer;
} a4988_stepper_inst_t;

/***************************************************************************//**
 * @brief
 *    Default init struct for the stepper driver instance.
 ******************************************************************************/
#define A4988_STEPPER_INST_DEFAULT                                             \
{                                                                              \
  A4988_STEPPER_GPIO_CONFIG_DEFAULT,                                           \
  DISABLED,                                                                    \
  CW,                                                                          \
  0,                                                                           \
  0,                                                                           \
  0,                                                                           \
  NULL,                                                                        \
  NULL                                                                         \
}

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief
 *    Initializes GPIOs needed to interface with the driver IC.
 *
 * @param[in] inst
 *    A4988 instance.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_INVALID_CONFIGURATION if RPM speed higher than the limit
 ******************************************************************************/
sl_status_t a4988_stepper_init(a4988_stepper_inst_t *inst);

/***************************************************************************//**
 * @brief
 *    Activates the RESET input on the driver which sets the translator
 *    to a predefined Home state and turns off all of the FET outputs.
 *    After the reset cycle, the driver is enabled.
 *
 * @param[in] inst
 *    A4988 instance.
 *
 * @return
 *    Error status
 ******************************************************************************/
sl_status_t a4988_stepper_reset(a4988_stepper_inst_t *inst);

/***************************************************************************//**
 * @brief
 *    Enables or disable the motor driver (turns on or off all of the
 *    FET outputs of the driver).
 *
 * @param[in] inst
 *    A4988 instance.
 *
 * @param[in] en
 *    True or false to enable or disable, respectively.
 *
 * @return
 *    Error status
 ******************************************************************************/
sl_status_t a4988_stepper_enable(a4988_stepper_inst_t *inst, bool en);

/***************************************************************************//**
 * @brief
 *    Returns with the inner state of the driver.
 *
 * @param[in] inst
 *    A4988 instance.
 *
 * @return
 *    Inner state of the driver.
 ******************************************************************************/
a4988_stepper_state_t a4988_stepper_get_state(a4988_stepper_inst_t *inst);

/***************************************************************************//**
 * @brief
 *    Sets the speed of the motor in rpm.
 *
 * @param[in] inst
 *    A4988 instance.
 *
 * @param[in] rpm
 *    Desired motor speed in rpm. This parameter has to be less than the
 *    STEPPER_MAX_RPM value.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_INVALID_PARAMETER if speed is out of valid range.
 ******************************************************************************/
sl_status_t a4988_stepper_set_speed(a4988_stepper_inst_t *inst, uint32_t rpm);

/***************************************************************************//**
 * @brief
 *    Sets the direction of rotation.
 *
 * @param[in] inst
 *    A4988 instance.
 *
 * @param[in] dir
 *    Direction of rotation.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_INVALID_PARAMETER if input is not CW or CCW.
 ******************************************************************************/
sl_status_t a4988_stepper_set_dir(a4988_stepper_inst_t *inst,
                                  a4988_stepper_dir_t dir);

/***************************************************************************//**
 * @brief
 *    Starts the motor with the configured speed and direction.
 *
 * @param[in] inst
 *    A4988 instance.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_INVALID_STATE if motor is not in idle state.
 *    SL_STATUS_INVALID_CONFIGURATION if RPM speed higher than the limit.
 ******************************************************************************/
sl_status_t a4988_stepper_start(a4988_stepper_inst_t *inst);

/***************************************************************************//**
 * @brief
 *    Stops the motor.
 *
 * @param[in] inst
 *    A4988 instance.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_INVALID_STATE if motor is not in running state.
 ******************************************************************************/
sl_status_t a4988_stepper_stop(a4988_stepper_inst_t *inst);

/***************************************************************************//**
 * @brief
 *    Move the motor by a given number of steps. This is a
 *    non-blocking function. The speed of the movement can be set with the
 *    a4988_stepper_start() function.
 *
 * @param[in] inst
 *    A4988 instance.
 *
 * @param[in] sleeptimer_handle
 *    Sleeptimer handle that the driver can use while omitting the
 *    given number of steps.
 *
 * @param[in] step_count
 *    Number of steps to move.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_INVALID_STATE if motor is not in idle state.
 *    SL_STATUS_INVALID_CONFIGURATION if RPM speed higher than the limit.
 *    SL_STATUS_INVALID_PARAMETER if sleeptimer handle parameter is NULL.
 ******************************************************************************/
sl_status_t a4988_stepper_step(a4988_stepper_inst_t *inst,
                               sl_sleeptimer_timer_handle_t *sleeptimer_handle,
                               uint32_t step_count);

/** @} (end addtogroup Stepper motor driver) */

#ifdef __cplusplus
}
#endif

#endif /* STEPPER_A4988_H_ */

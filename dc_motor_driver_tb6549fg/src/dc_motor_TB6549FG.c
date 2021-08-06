/***************************************************************************//**
 * @file dc_motor_TB6549FG.c
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
// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "dc_motor_TB6549FG.h"
#include "sl_udelay.h"

#include "em_gpio.h"
#include "em_cmu.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static CMU_Clock_TypeDef get_timer_clock(TIMER_TypeDef *timer);
static CMU_Clock_TypeDef get_letimer_clock(LETIMER_TypeDef *timer);
static void tb6549fg_standby_transition(tb6549fg_inst_t *inst);
static sl_status_t verify_pwm_frequency(tb6549fg_inst_t *inst);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/***************************************************************************//**
 * @brief
 *    Initializes GPIO and peripherals needed to interface with the TB6549FG IC.
 *
 * @param[in] inst
 *    TB6549FG instance.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_INVALID_CONFIGURATION if PWM frequency is higher than 100 kHz
 ******************************************************************************/
sl_status_t tb6549fg_init(tb6549fg_inst_t *inst){

  sl_status_t status = SL_STATUS_OK;

  CMU_ClockEnable(cmuClock_GPIO, true);

  //Configure GPIO pins, PWM pins are configured by PWM DRIVER and initialized
  //in sl_event_handler.c
  GPIO_PinModeSet(inst->gpio.IN1_port,
                  inst->gpio.IN1_pin,
                  gpioModePushPull, 0);

  GPIO_PinModeSet(inst->gpio.IN2_port,
                  inst->gpio.IN2_pin,
                  gpioModePushPull, 0);

  GPIO_PinModeSet(inst->gpio.SB_port,
                  inst->gpio.SB_pin,
                  gpioModePushPull, 0);

  //Verify the PWM instance frequency, it should be max 100 kHz
 status = verify_pwm_frequency(inst);

  //Configure for "standby" mode
  tb6549fg_standby_mode(inst);

 return status;
}

/***************************************************************************//**
 * @brief
 *    Operates the TB6549FG IC in CW direction.
 *
 * @param[in] inst
 *    TB6549FG instance.
 *
 * @param[in] duty_cycle_percent
 *    Up to 100, a value of 0 will bring no change to the duty cycle.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t tb6549fg_cw_mode(tb6549fg_inst_t *inst,
                             uint8_t duty_cycle_percent){

  sl_status_t status = SL_STATUS_OK;

  //Check if previous mode was STANDBY
  if (inst->mode == STANDBY_MODE){
    tb6549fg_standby_transition(inst);
  }

  //Configure control GPIO pins
  GPIO_PinOutClear(inst->gpio.IN1_port,
                   inst->gpio.IN1_pin);

  GPIO_PinOutSet(inst->gpio.IN2_port,
                 inst->gpio.IN2_pin);

  //Set new duty cycle. A value of 0 means that no update is required
  status = tb6549fg_set_pwm_duty_cycle(inst, duty_cycle_percent);

  //Enable PWM waveform
  if (inst->enable_sleep){
    sl_pwm_letimer_start(inst->pwm_letimer);
  } else {
    sl_pwm_start(inst->pwm);
  }

  //Update operation mode
  inst->mode = CW_MODE;

  return status;
}

/***************************************************************************//**
 * @brief
 *    Operates the TB6549FG IC in CCW direction.
 *
 * @param[in] inst
 *    TB6549FG instance.
 *
 * @param[in] duty_cycle_percent
 *    Up to 100, a value of 0 will bring no change to the duty cycle.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t tb6549fg_ccw_mode(tb6549fg_inst_t *inst,
                               uint8_t duty_cycle_percent){

  sl_status_t status = SL_STATUS_OK;

  //Check if previous mode was STANDBY
  if (inst->mode == STANDBY_MODE){
    tb6549fg_standby_transition(inst);
  }

  //Configure control GPIO pins
  GPIO_PinOutSet(inst->gpio.IN1_port,
                 inst->gpio.IN1_pin);

  GPIO_PinOutClear(inst->gpio.IN2_port,
                   inst->gpio.IN2_pin);

  //Set new duty cycle. A value of 0 means that no update is required
  status = tb6549fg_set_pwm_duty_cycle(inst, duty_cycle_percent);

  //Enable PWM waveform
  if (inst->enable_sleep){
    sl_pwm_letimer_start(inst->pwm_letimer);
  } else {
    sl_pwm_start(inst->pwm);
  }

  //Update operation mode
  inst->mode = CCW_MODE;

  return status;
}

/***************************************************************************//**
 * @brief
 *    Operates the TB6549FG IC in short break mode.
 *
 * @param[in] inst
 *    TB6549FG instance.
 ******************************************************************************/
void tb6549fg_short_break_mode(tb6549fg_inst_t *inst){

  //Check if previous mode was STANDBY
  if (inst->mode == STANDBY_MODE){
    tb6549fg_standby_transition (inst);
  }

  //Configure control GPIO pins
  GPIO_PinOutSet(inst->gpio.IN1_port,
                 inst->gpio.IN1_pin);

  GPIO_PinOutSet(inst->gpio.IN2_port,
                 inst->gpio.IN2_pin);

  //Update operation mode
  inst->mode = SHORT_BRAKE_MODE;
}

/**************************************************************************//**
 * @brief
 *    Operates the TB6549FG IC in stop mode.
 *
 * @param[in] inst
 *    TB6549FG instance.
 ******************************************************************************/
void tb6549fg_stop_mode(tb6549fg_inst_t *inst){

  //Check if previous mode was STANDBY
  if (inst->mode == STANDBY_MODE){
    tb6549fg_standby_transition (inst);
  }

  //Stop PWM. Allows lower consumption when using LETIMER for PWM
  if (inst->enable_sleep){
    sl_pwm_letimer_stop(inst->pwm_letimer);
  } else {
    sl_pwm_stop(inst->pwm);
  }

  //Configure control GPIO pins
  GPIO_PinOutClear(inst->gpio.IN1_port,
                   inst->gpio.IN1_pin);

  GPIO_PinOutClear(inst->gpio.IN2_port,
                   inst->gpio.IN2_pin);

  //Update operation mode
  inst->mode = STOP_MODE;
}

/**************************************************************************//**
 * @brief
 *    Operates the TB6549FG IC in standby mode.
 *
 * @param[in] inst
 *    TB6549FG instance.
 ******************************************************************************/
void tb6549fg_standby_mode(tb6549fg_inst_t *inst){

  //Stop PWM. Allows lower consumption when using LETIMER for PWM
  if (inst->enable_sleep){
    sl_pwm_letimer_stop(inst->pwm_letimer);
  } else {
    sl_pwm_stop(inst->pwm);
  }

  //Configure control GPIO pins
  GPIO_PinOutClear(inst->gpio.IN1_port,
                   inst->gpio.IN1_pin);

  GPIO_PinOutClear(inst->gpio.IN2_port,
                   inst->gpio.IN2_pin);

  GPIO_PinOutClear(inst->gpio.SB_port,
                   inst->gpio.SB_pin);

  //Update operation mode
  inst->mode = STANDBY_MODE;
}

/***************************************************************************//**
 * @brief
 *    Configure a new duty cycle for the PWM waveform.
 *
 * @param[in] inst
 *    TB6549FG instance.
 *
 * @param[in] duty_cycle_percent
 *    Up to 100, a value of 0 will bring no change to the duty cycle.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_INVALID_CONFIGURATION if desired duty cycle would have a high
 *                                    period lower than 2 us.
 ******************************************************************************/
sl_status_t tb6549fg_set_pwm_duty_cycle(tb6549fg_inst_t *inst,
                                        uint8_t duty_cycle_percent){

  sl_status_t status = SL_STATUS_OK;
  uint32_t period_us;

  if (duty_cycle_percent > 0){
    //Verify that desired duty cycle has a minimum high period os 2 us
    period_us = 1000000 / inst->calc_pwm_frequency;

    //Convert to ns to increase comparison resolution
    if ((period_us * duty_cycle_percent * 10) < 2000){
        return SL_STATUS_INVALID_CONFIGURATION;
    }

    //Set appropriate duty cycle
    if (inst->enable_sleep){
      sl_pwm_letimer_set_duty_cycle(inst->pwm_letimer, duty_cycle_percent);
    } else {
      sl_pwm_set_duty_cycle(inst->pwm, duty_cycle_percent);
    }
  }

  return status;
}

/**************************************************************************//**
 * @brief
 *   Get current duty cycle of PWM waveform.
 *
 * @param[in] inst
 *    TB6549FG instance.
 *
 * @return
 *   Percent of the currently configured PWM waveform.
 *****************************************************************************/
uint8_t tb6549fg_get_pwm_duty_cycle(tb6549fg_inst_t *inst){

  uint8_t duty_cycle;

  //Get configured duty cycle
  if (inst->enable_sleep){
    duty_cycle = sl_pwm_letimer_get_duty_cycle(inst->pwm_letimer);
  } else {
    duty_cycle = sl_pwm_get_duty_cycle(inst->pwm);
  }

  return duty_cycle;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief
 *    Utility function used to implemented the required delay when transitioning
 *    the TB6549FG from "standby" to "active" mode
 *
 * @param[in] inst
 *    TIMER instance.
 *
 ******************************************************************************/
static void tb6549fg_standby_transition(tb6549fg_inst_t *inst){

  //Configure control GPIO pins
  GPIO_PinOutSet(inst->gpio.SB_port,
                 inst->gpio.SB_pin);

  //Insert active delay to transition from standby to operation mode.
  //Ensure that charge pump circuit is ready before setting IN1 or IN2
  sl_udelay_wait(TRANSITION_DELAY_US);
}

/**************************************************************************//**
 * @brief
 *    Utility function used to verify the frequency of the PWM instance assigned
 *    to the driver. The TB6549FG establishes a maximum frequency of 100 kHz
 *
 * @param[in] inst
 *    TIMER instance.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_INVALID_PARAMETER if frequency is out of valid range
 *
 ******************************************************************************/
static sl_status_t verify_pwm_frequency(tb6549fg_inst_t *inst){

  uint32_t frequency = 0;

  //Verify actual configured frequency based on register values. May vary
  //from software component frequency depending on TIMER/LETIMER resolution
  if (inst->enable_sleep){
      frequency = (CMU_ClockFreqGet(get_letimer_clock(inst->pwm_letimer->timer)) /
                  (LETIMER_TopGet(inst->pwm_letimer->timer)+1));
  } else {
      frequency = (CMU_ClockFreqGet(get_timer_clock(inst->pwm->timer)) /
                  (TIMER_TopGet(inst->pwm->timer)+1));
  }

  //Store frequency in tb6549fg instance, this makes future duty cycle
  //calculations faster
  inst->calc_pwm_frequency = frequency;

  return (frequency > (uint32_t)MAX_PWM_FREQUENCY) ? SL_STATUS_INVALID_PARAMETER
                                                   : SL_STATUS_OK;
}

/**************************************************************************//**
 * @brief
 *    Gets the clock of a specific TIMER instance
 *
 * @param[in] inst
 *    TIMER instance.
 *
 * @return
 *   clock assigned to specific TIMER.
 *
 ******************************************************************************/
static CMU_Clock_TypeDef get_timer_clock(TIMER_TypeDef *timer){
#if defined(_CMU_HFCLKSEL_MASK) || defined(_CMU_CMD_HFCLKSEL_MASK)
  CMU_Clock_TypeDef timer_clock = cmuClock_HF;
#elif defined(_CMU_SYSCLKCTRL_MASK)
  CMU_Clock_TypeDef timer_clock = cmuClock_SYSCLK;
#else
#error "Unknown root of clock tree"
#endif

  switch ((uint32_t)timer) {
#if defined(TIMER0_BASE)
    case TIMER0_BASE:
      timer_clock = cmuClock_TIMER0;
      break;
#endif
#if defined(TIMER1_BASE)
    case TIMER1_BASE:
      timer_clock = cmuClock_TIMER1;
      break;
#endif
#if defined(TIMER2_BASE)
    case TIMER2_BASE:
      timer_clock = cmuClock_TIMER2;
      break;
#endif
#if defined(TIMER3_BASE)
    case TIMER3_BASE:
      timer_clock = cmuClock_TIMER3;
      break;
#endif
#if defined(TIMER4_BASE)
    case TIMER4_BASE:
      timer_clock = cmuClock_TIMER4;
      break;
#endif
#if defined(TIMER5_BASE)
    case TIMER5_BASE:
      timer_clock = cmuClock_TIMER5;
      break;
#endif
#if defined(TIMER6_BASE)
    case TIMER6_BASE:
      timer_clock = cmuClock_TIMER6;
      break;
#endif
    default:
      EFM_ASSERT(0);
      break;
  }
  return timer_clock;
}

/**************************************************************************//**
 * @brief
 *    Gets the clock of a specific LETIMER instance
 *
 * @param[in] inst
 *    LETIMER instance.
 *
 * @return
 *   clock assigned to specific LETIMER.
 *
 ******************************************************************************/
static CMU_Clock_TypeDef get_letimer_clock(LETIMER_TypeDef *timer)
{
#if defined(_CMU_HFCLKSEL_MASK) || defined(_CMU_CMD_HFCLKSEL_MASK)
  CMU_Clock_TypeDef timer_clock = cmuClock_HF;
#elif defined(CMU_EM23GRPACLK_BRANCH)
  CMU_Clock_TypeDef timer_clock = cmuClock_EM23GRPACLK;
#else
#error "Unknown root of clock tree"
#endif

  switch ((uint32_t)timer) {
#if defined(LETIMER0_BASE)
    case LETIMER0_BASE:
      timer_clock = cmuClock_LETIMER0;
      break;
#endif
#if defined(LETIMER1_BASE)
    case LETIMER1_BASE:
      timer_clock = cmuClock_LETIMER1;
      break;
#endif
#if defined(LETIMER2_BASE)
    case LETIMER2_BASE:
      timer_clock = cmuClock_LETIMER2;
      break;
#endif
#if defined(LETIMER3_BASE)
    case LETIMER3_BASE:
      timer_clock = cmuClock_LETIMER3;
      break;
#endif
#if defined(LETIMER4_BASE)
    case LETIMER4_BASE:
      timer_clock = cmuClock_LETIMER4;
      break;
#endif
    default:
      EFM_ASSERT(0);
      break;
  }
  return timer_clock;
}

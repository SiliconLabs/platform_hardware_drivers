/***************************************************************************//**
 * @file sl_pwm_letimer.c
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

/* This is a clone implementation of the PWM driver. The expectation is that
 *  LETIMER support is added but in the meantime a temporary driver is provided
 *  to enable EM2 PWM control for the DC MOTOR3 TB6549FG driver
 */
// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_pwm_letimer.h"
#include "em_gpio.h"
#include "em_bus.h"
#include "em_cmu.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static CMU_Clock_TypeDef get_letimer_clock(LETIMER_TypeDef *timer);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief
 *   Initialize PWM letimer driver.
 *
 * @param[in] pwm
 *   PWM letimer driver instance
 *
 * @param[in] config
 *   Driver configuration
 *
 * @return
 *   SL_STATUS_OK if there are no errors.
 *****************************************************************************/
sl_status_t sl_pwm_letimer_init(sl_pwm_letimer_instance_t *pwm,
                                sl_pwm_letimer_config_t *config)
{
  CMU_Clock_TypeDef timer_clock = get_letimer_clock(pwm->timer);
  CMU_ClockEnable(timer_clock, true);

  // Set PWM pin as output
  CMU_ClockEnable(cmuClock_GPIO, true);
  GPIO_PinModeSet((GPIO_Port_TypeDef)pwm->port,
                  pwm->pin,
                  gpioModePushPull,
                  config->polarity);

  // Configure LETIMER
  LETIMER_Init_TypeDef letimer_init = LETIMER_INIT_DEFAULT;

  // Reload top on underflow, PWM output, and run in free mode
  letimer_init.comp0Top = true;
  letimer_init.ufoa0 = letimerUFOAPwm;
  letimer_init.repMode = letimerRepeatFree;

  // Configure TIMER frequency
  uint32_t top = (CMU_ClockFreqGet(timer_clock) / (config->frequency)) - 1U;
  LETIMER_TopSet(pwm->timer, top);

  // Set initial duty cycle to 0%
  LETIMER_CompareSet(pwm->timer, pwm->channel, 0U);

  //Configure LETIMER output route pinout
#if defined(_LETIMER_ROUTE_MASK)
  BUS_RegMaskedWrite(&pwm->timer->ROUTE,
                     _LETIMER_ROUTE_LOCATION_MASK,
                     pwm->location << _LETIMER_ROUTE_LOCATION_SHIFT);
#elif defined(_LETIMER_ROUTELOC0_MASK)
  BUS_RegMaskedWrite(&pwm->timer->ROUTELOC0,
                     _LETIMER_ROUTELOC0_OUT0LOC_MASK  << (pwm->channel * 8U),
                     pwm->location << (pwm->channel * 8U));
#elif defined(_GPIO_LETIMER_ROUTEEN_MASK)
  //No current implementation of TIMER_NUM(ref) macro for LETIMER
  //ok because we only have 1 LETIMER instance in our devices
  volatile uint32_t * route_register = &GPIO->LETIMERROUTE[0].OUT0ROUTE;
  route_register += pwm->channel;
  *route_register = (pwm->port << _GPIO_LETIMER_OUT0ROUTE_PORT_SHIFT)
                    | (pwm->pin << _GPIO_LETIMER_OUT0ROUTE_PIN_SHIFT);

  GPIO->LETIMERROUTE[0].OUT0ROUTE = \
      (pwm->port << _GPIO_LETIMER_OUT0ROUTE_PORT_SHIFT) \
      | (pwm->pin << _GPIO_LETIMER_OUT0ROUTE_PIN_SHIFT);
#else
#error "Unknown route setting"
#endif

  // Initialize LETIMER
  LETIMER_Init(pwm->timer, &letimer_init);

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * @brief
 *   Deinitialize PWM letimer driver.
 *
 * @param[in] pwm
 *   PWM letimer driver instance
 *
 * @return
 *   SL_STATUS_OK if there are no errors.
 *****************************************************************************/
sl_status_t sl_pwm_letimer_deinit(sl_pwm_letimer_instance_t *pwm)
{
  // Reset TIMER routes
  sl_pwm_letimer_stop(pwm);

#if defined(_LETIMER_ROUTE_MASK)
  BUS_RegMaskedClear(&pwm->timer->ROUTE, _LETIMER_ROUTE_LOCATION_MASK);
#elif defined(_LETIMER_ROUTELOC0_MASK)
  BUS_RegMaskedClear(&pwm->timer->ROUTELOC0,
                     _LETIMER_ROUTELOC0_OUT0LOC_MASK << (pwm->channel * 8));
#elif defined(_GPIO_LETIMER_ROUTEEN_MASK)
  volatile uint32_t * route_register = &GPIO->LETIMERROUTE[0].OUT0ROUTE;
  route_register += pwm->channel;
  *route_register = 0;
#else
#error "Unknown route setting"
#endif

  // Reset LETIMER
  LETIMER_Reset(pwm->timer);

  // Reset GPIO
  GPIO_PinModeSet((GPIO_Port_TypeDef)pwm->port,
                  pwm->pin,
                  gpioModeDisabled,
                  0);

  CMU_Clock_TypeDef timer_clock = get_letimer_clock(pwm->timer);
  CMU_ClockEnable(timer_clock, false);

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * @brief
 *   Start generating PWM waveform
 *
 * @param[in] pwm
 *   PWM letimer driver instance
 *****************************************************************************/
void sl_pwm_letimer_start(sl_pwm_letimer_instance_t *pwm)
{
  // Enable PWM output
#if defined(_LETIMER_ROUTE_MASK)
  BUS_RegMaskedSet(&pwm->timer->ROUTE,
                   1 << (pwm->channel + _LETIMER_ROUTE_OUT0PEN_SHIFT));
#elif defined(_LETIMER_ROUTELOC0_MASK)
  BUS_RegMaskedSet(&pwm->timer->ROUTEPEN,
                   1 << (pwm->channel + _LETIMER_ROUTEPEN_OUT0PEN_SHIFT));
#elif defined(_GPIO_LETIMER_ROUTEEN_MASK)
  GPIO->LETIMERROUTE[0].ROUTEEN = 1 <<
                           (pwm->channel + _GPIO_LETIMER_ROUTEEN_OUT0PEN_SHIFT);
#else
#error "Unknown route setting"
#endif
}

/**************************************************************************//**
 * @brief
 *   Stop generating PWM waveform
 *
 * @param[in] pwm
 *   PWM letimer driver instance
 *****************************************************************************/
void sl_pwm_letimer_stop(sl_pwm_letimer_instance_t *pwm)
{
  // Disable PWM output
#if defined(_LETIMER_ROUTE_MASK)
  BUS_RegMaskedClear(&pwm->timer->ROUTE,
                     1 << (pwm->channel + _LETIMER_ROUTE_OUT0PEN_SHIFT));
#elif defined(_LETIMER_ROUTELOC0_MASK)
  BUS_RegMaskedClear(&pwm->timer->ROUTEPEN,
                     1 << (pwm->channel + _LETIMER_ROUTEPEN_OUT0PEN_SHIFT));
#elif defined(_GPIO_LETIMER_ROUTEEN_MASK)
  GPIO->LETIMERROUTE_CLR[0].ROUTEEN = 1 <<
                           (pwm->channel + _GPIO_LETIMER_ROUTEEN_OUT0PEN_SHIFT);
#else
#error "Unknown route setting"
#endif

  // Keep timer running in case other channels are in use
}

/**************************************************************************//**
 * @brief
 *   Set duty cycle for PWM waveform.
 *
 * @param[in] pwm
 *   PWM letimer driver instance
 *
 * @param[in] percent
 *   Percent of the PWM period waveform is in the state defined
 *   as the active polarity in the driver configuration
 *****************************************************************************/
void sl_pwm_letimer_set_duty_cycle(sl_pwm_letimer_instance_t *pwm,
                                   uint8_t percent)
{
  uint32_t top = LETIMER_TopGet(pwm->timer);

  // Set compare value
  LETIMER_CompareSet(pwm->timer, pwm->channel, (top * percent) / 100);
}

/**************************************************************************//**
 * @brief
 *   Set duty cycle for PWM waveform.
 *
 * @param[in] pwm
 *   PWM letimer driver instance
 *
 * @return
 *   Percent of the PWM period waveform is in the state defined
 *   as the active polarity in the driver configuration
 *****************************************************************************/
uint8_t sl_pwm_letimer_get_duty_cycle(sl_pwm_letimer_instance_t *pwm)
{
  uint32_t top = LETIMER_TopGet(pwm->timer);
  uint32_t compare = LETIMER_CompareGet(pwm->timer, pwm->channel);

  uint8_t percent = (uint8_t)((compare * 100) / top);

  return percent;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static CMU_Clock_TypeDef get_letimer_clock(LETIMER_TypeDef *timer)
{
#if defined(_CMU_HFCLKSEL_MASK) || defined(_CMU_CMD_HFCLKSEL_MASK)
  CMU_Clock_TypeDef timer_clock = cmuClock_HF;
#elif defined(_CMU_SYSCLKCTRL_MASK)
  CMU_Clock_TypeDef timer_clock = cmuClock_SYSCLK;
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

/***************************************************************************//**
 * @file DC_motor3_test_app.c
 * @brief DC Motor3 click board test app example using the TB6549FG driver
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
#include "em_gpio.h"

#include "dc_motor_TB6549FG.h"
#include "sl_pwm_letimer_instances.h"
#include "sl_pwm_instances.h"
#include "sl_sleeptimer.h"

#include "DC_motor3_test_app.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define DEBUG_PIN      (1)       //Enable PRS debugs for test state transitions
#define DEBUG_PIN_PORT gpioPortB //PRS debug port config
#define DEBUG_PIN_PIN  2         //PRS debug pin config

#define TEST_STATE_MS  500 //Transition time between test cases

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static void dc_motor3_app_tests_state_duration(uint16_t time_ms);
static void duty_cycle_variation(void);
static void test_duration_callback(sl_sleeptimer_timer_handle_t *handle,
                                   void *data);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
tb6549fg_inst_t motor3_click_inst = TB6549FG_INST_DEFAULT;//DC motor driver inst
test_states_TypeDef test_state = STANDBY;
volatile uint8_t i_cycle = 0;

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static sl_sleeptimer_timer_handle_t delay_timer; //Sleep timer instance
static const uint8_t dutyCyclePercentages[] =
    {20, 30, 40, 50, 60, 70, 80, 90, 100, 0};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/***************************************************************************//**
 * @brief
 *     Initialize DC Motor3 click board test app
 *
 * @param[in] enable_sleep
 *     Indicates if sleep mode (LETIMER PWM) will be tested
 ******************************************************************************/
void dc_motor3_app_init(bool enable_sleep){

  sl_status_t status = SL_STATUS_OK;

#if DEBUG_PIN
  //Configure debug pin (Used to check state transitions)
  GPIO_PinModeSet(DEBUG_PIN_PORT, DEBUG_PIN_PIN, gpioModePushPull, 0);
#endif

  //Verify if sleep mode (LETIMER PWM) is to be used
  if(enable_sleep){
      motor3_click_inst.enable_sleep = true;

      //Assign PWM LETIMER driver instance
      motor3_click_inst.pwm_letimer = &sl_pwm_letimer_motor0;

      //Initialize PWM LETIMER driver
      sl_pwm_letimer_init_instances();
  } else {
      //Assign PWM driver instance. Initialized in sl_event_handler
      motor3_click_inst.pwm = &sl_pwm_motor0;
  }

  //Initialize the TB6549FG driver
  status = tb6549fg_init(&motor3_click_inst);

  if(status != SL_STATUS_OK){
      //We should't reach this state, initialization issue, check PWM frequency
      // < 100 kHz
      __BKPT(0);
  }
}

/***************************************************************************//**
 * @brief
 *     DC Motor3 click board test app ticking function
 ******************************************************************************/
void dc_moto3_app_process_action(void){

  //Test state machine
  switch(test_state){
    case STANDBY:
      tb6549fg_standby_mode(&motor3_click_inst);
      test_state++;
      break;

    case STOP:
      tb6549fg_stop_mode(&motor3_click_inst);
      test_state++;
      break;

    case SHORTBREAK:
      tb6549fg_short_break_mode(&motor3_click_inst);
      test_state++;
      break;

    case CW:
      tb6549fg_cw_mode(&motor3_click_inst, 10);
      test_state++;
      break;

    case CW_cycle:
      duty_cycle_variation();
      break;

    case CCW:
      tb6549fg_ccw_mode(&motor3_click_inst, 10);
      test_state++;
      break;

    case CCW_cycle:
      duty_cycle_variation();
      break;

    default:
      //We should't reach this state
      __BKPT(0);
      break;
  }

  //Reset test state machine
  if (test_state == NUM_STATES){
    test_state = STANDBY;
  }

  //Start timer for test state duration
  dc_motor3_app_tests_state_duration(TEST_STATE_MS);
}

/*******************************************************************************
 * Redefinition of WEAK implementation.
 ******************************************************************************/
bool app_is_ok_to_sleep(void){

  //Allow app to sleep if enable_sleep is set
  if (motor3_click_inst.enable_sleep){
    return true;
  } else {
    return false;
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * Function to cycle across duty cycles.
 ******************************************************************************/
static void duty_cycle_variation(void){

  //Cycle through all the duty cycles from dutyCyclePercentages
  tb6549fg_set_pwm_duty_cycle(&motor3_click_inst,
                              dutyCyclePercentages[i_cycle++]);

  //Reset i_cycle and update tests state
  if (dutyCyclePercentages[i_cycle] == 0){
    i_cycle = 0;
    test_state++;
  }
}

/*******************************************************************************
 * Timer expiration callback for the test duration.
 *
 * @param[in] handle
 *     Pointer to sleeptimer handle.
 *
 * @param[in] data
 *     Pointer to callback data.
 ******************************************************************************/
static void test_duration_callback(sl_sleeptimer_timer_handle_t *handle,
                                   void *data)
{

  volatile bool *wait_flag = (bool *)data;

  (void)&handle;  // Unused parameter.

#if DEBUG_PIN
  //Toggle debug pin to indicate state transition
  GPIO_PinOutToggle(DEBUG_PIN_PORT, DEBUG_PIN_PIN);
#endif

  *wait_flag = false;
}

/***************************************************************************//**
 * @brief
 *     DC Motor3 click board test app state duration for each test function
 * @param[in] time_ms
 *     Time in miliseconds for each state transition
 ******************************************************************************/
static void dc_motor3_app_tests_state_duration(uint16_t time_ms){

  volatile bool wait = true;
  sl_status_t error_code;
  uint32_t delay = sl_sleeptimer_ms_to_tick(time_ms);

  error_code = sl_sleeptimer_start_timer(&delay_timer,
                                         delay,
                                         test_duration_callback,
                                         (void *)&wait,
                                         0,
                                         0);

  if (error_code != SL_STATUS_OK){
      __BKPT(0); //We shouldn't reach this state
  }

  //Do an active delay if sleep mode is not enabled
  if (!motor3_click_inst.enable_sleep){
      while (wait);
  }

}

/**************************************************************************//**
 * @file app.c
 * @brief Application interface provided to main().
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
* # Experimental Quality
* This code has not been formally tested and is provided as-is. It is not
* suitable for production environments. In addition, this code will not be
* maintained and there may be no bug maintenance planned for these resources.
* Silicon Labs may update projects from time to time.
******************************************************************************/

#include "buzz2.h"
#include "app_log.h"
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"
#include "sl_pwm_instances.h"
#include "sl_pwm_init_mikroe_config.h"

/*******************************************************************************
 *****************************    DEFINE     ***********************************
 ******************************************************************************/
#define W 4*Q // Whole 4/4 - 4 Beats
#define H 2*Q // Half 2/4 - 2 Beats
#define Q 250 // Quarter 1/4 - 1 Beat
#define E Q/2 // Eighth 1/8 - 1/2 Beat
#define S Q/4 // Sixteenth 1/16 - 1/4 Beat

// specific notes in the melody
const int melody[] = {
    BUZZ2_NOTE_A6, BUZZ2_NOTE_A6, BUZZ2_NOTE_A6, BUZZ2_NOTE_F6, BUZZ2_NOTE_C7,
    BUZZ2_NOTE_A6, BUZZ2_NOTE_F6, BUZZ2_NOTE_C7, BUZZ2_NOTE_A6, BUZZ2_NOTE_E7,
    BUZZ2_NOTE_E7, BUZZ2_NOTE_E7, BUZZ2_NOTE_F7, BUZZ2_NOTE_C7, BUZZ2_NOTE_Ab6,
    BUZZ2_NOTE_F6, BUZZ2_NOTE_C7, BUZZ2_NOTE_A6, BUZZ2_NOTE_A7, BUZZ2_NOTE_A6,
    BUZZ2_NOTE_A6, BUZZ2_NOTE_A7, BUZZ2_NOTE_Ab7, BUZZ2_NOTE_G7, BUZZ2_NOTE_Gb7,
    BUZZ2_NOTE_E7, BUZZ2_NOTE_F7, BUZZ2_NOTE_Bb6, BUZZ2_NOTE_Eb7, BUZZ2_NOTE_D7};

// note durations
const int note_durations[] = {
    Q, Q, Q, E + S, S, Q, E + S, S, H, Q, Q, Q, E + S, S, Q, E + S, S, H, Q,
    E + S, S, Q, E + S, S, S, Q, E, E, Q, E + S};

/*******************************************************************************
 *****************************   VARIABLE    **********************************
 ******************************************************************************/
volatile uint8_t volume = 5; // goes up to 9

static buzz2_t buzz2;

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  sl_status_t retval;

  // pwm configuration
  sl_pwm_config_t pwm_mikroe_config = {
    .frequency = BUZZ2_DEF_FREQ,
    .polarity = SL_PWM_MIKROE_POLARITY,
  };

  // buzzer2 configuration
  buzz2_cfg_setup(&buzz2,
                  sl_pwm_mikroe,
                  pwm_mikroe_config);

  // initialize the buzzer2
  retval = buzz2_init(&buzz2);
  if(retval == SL_STATUS_OK){
      app_log("configuration successfully \r\n");
  }
  else{
      app_log("Fail to configure buzz2, reason: %04x\r\n", retval);
  }
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  uint8_t i;

  for(i = 0; i < sizeof(melody)/sizeof(melody[0]); i++){
      buzz2_play_sound(&buzz2,
                       melody[i],
                       volume,
                       note_durations[i]);
  }
  sl_sleeptimer_delay_millisecond(2000);
}

/**************************************************************************//**
 * Handler for the button interrupt event
 *****************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  if((handle == &sl_button_btn0) &&
      (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED)) {
      volume++;
      if(volume > 9) volume = 0;

      app_log("The buzzer's volume is %d\n", volume);
  }
}

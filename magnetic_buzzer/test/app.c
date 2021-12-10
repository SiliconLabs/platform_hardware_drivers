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
#include "app_log.h"
#include "magnetic_buzzer.h"
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

/*******************************************************************************
 *****************************    DEFINE     ***********************************
 ******************************************************************************/
#define W 4*Q // Whole 4/4 - 4 Beats
#define H 2*Q // Half 2/4 - 2 Beats
#define Q 250 // Quarter 1/4 - 1 Beat
#define E Q/2 // Eighth 1/8 - 1/2 Beat
#define S Q/4 // Sixteenth 1/16 - 1/4 Beat

// a melody
const buzzer_note_t melody[] = {
    {BUZZER_NOTE_A6, Q},
    {BUZZER_NOTE_A6, Q},
    {BUZZER_NOTE_A6, Q},
    {BUZZER_NOTE_F6, E+S},
    {BUZZER_NOTE_C7, S},
    {BUZZER_NOTE_A6, Q},
    {BUZZER_NOTE_F6, E + S},
    {BUZZER_NOTE_C7, S},
    {BUZZER_NOTE_A6, H},
    {BUZZER_NOTE_E7, Q},
    {BUZZER_NOTE_E7, Q},
    {BUZZER_NOTE_E7, Q},
    {BUZZER_NOTE_F7, E + S},
    {BUZZER_NOTE_C7, S},
    {BUZZER_NOTE_Ab6, Q},
    {BUZZER_NOTE_F6, E + S},
    {BUZZER_NOTE_C7, S},
    {BUZZER_NOTE_A6, H},
    {BUZZER_NOTE_A7, Q},
    {BUZZER_NOTE_A6, E + S},
    {BUZZER_NOTE_A6, E + S},
    {BUZZER_NOTE_A7, S},
    {BUZZER_NOTE_Ab7, Q},
    {BUZZER_NOTE_G7, E + S},
    {BUZZER_NOTE_Gb7, S},
    {BUZZER_NOTE_E7, S},
    {BUZZER_NOTE_F7, Q},
    {BUZZER_NOTE_Bb6, E},
    {BUZZER_NOTE_Eb7, Q},
    {BUZZER_NOTE_D7, E + S},
    BUZZER_END_MELODY
};

/*******************************************************************************
 *****************************   VARIABLE    **********************************
 ******************************************************************************/
static buzzer_t buzzer = BUZZER_INIT_DEFAULT;

static buzzer_melody_t buzzer_melody = {
    .melody = melody,
    .buzzer = &buzzer,
    .len = 0
};

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  sl_status_t retval;

  // initialize the buzzer
  retval = buzzer_init(&buzzer);
  if(retval == SL_STATUS_OK){
      app_log("configuration successfully \r\n");
  }
  else{
      app_log("configure failed return %04x \r\n", retval);
  }
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  sl_status_t retval;

  retval = buzzer_play_melody(&buzzer_melody);
  if(retval != SL_STATUS_OK){
      app_log("Play melody failed return %04x \r\n", retval);
  }

  sl_sleeptimer_delay_millisecond(10000);
}

/**************************************************************************//**
 * Handler for the button interrupt event
 *****************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  buzzer_volume_t volume;
  if((handle == &sl_button_btn0) &&
      (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED)) {

      buzzer_get_volume(&buzzer, &volume);

      if(++volume > buzzer_VOL100){
          volume = buzzer_VOL0;
      }

      buzzer_set_volume(&buzzer, volume);

      app_log("The buzzer's volume is %d\n", volume);
  }
}

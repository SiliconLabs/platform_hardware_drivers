/***************************************************************************//**
 * @file app.c
 * @brief Top level application functions
*******************************************************************************
* # License
* <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
* EXPERIMENTAL QUALITY
* This code has not been formally tested and is provided as-is.
* It is not suitable for production environments.
* This code will not be maintained.
*
******************************************************************************/
#include "led_7seg.h"
#include "sl_sleeptimer.h"

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  led_7seg_init();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  led_7seg_display_number(88, LED_7SEG_LEFT_RIGHT_DOT); // test full segments
  
  /* Contrast increase */
  for (int i = 0; i < 20; i++) {
    led_7seg_set_contrast(5*i);
    sl_sleeptimer_delay_millisecond(150);
  }

  /* Contrast decrease */
  for (int i = 20; i > 0; i--) {
    led_7seg_set_contrast(5*i);
    sl_sleeptimer_delay_millisecond(150);
  }

  // Blink LED
  for (int i = 0; i < 7; i++) {
    led_7seg_set_contrast(i % 2 ? 0 : 100);
    sl_sleeptimer_delay_millisecond(500);
  }

  // Display decimal number from 0 to 9 on both 2 LEDs.
  for (int i = 0; i < 10; i++) {
    led_7seg_display_number(i*11, LED_7SEG_NO_DOT);
    sl_sleeptimer_delay_millisecond(1000);
  }
}

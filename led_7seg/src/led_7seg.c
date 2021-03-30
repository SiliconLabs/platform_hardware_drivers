/***************************************************************************//**
 * @file led_7seg.c
 * @brief 7SEG driver
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
* EVALUATION QUALITY
* This code has been minimally tested to ensure that it builds with
* the specified dependency versions and is suitable as a demonstration
* for evaluation purposes only.
* This code will be maintained at the sole discretion of Silicon Labs.
*
******************************************************************************/
#include <led_7seg.h>
#include "spidrv.h"
#include "sl_spidrv_instances.h"
#include "sl_pwm_instances.h"

#define spi_handle      sl_spidrv_mikroe_handle
#define LED_7SEG_DOT    0x80

static const uint8_t number_array[16] =
{
    0x3F,  // 0
    0x06,  // 1
    0x5B,  // 2
    0x4F,  // 3
    0x66,  // 4
    0x6D,  // 5
    0x7D,  // 6
    0x07,  // 7
    0x7F,  // 8
    0x6F,  // 9
    0x77,  // A
    0x7C,  // B
    0x39,  // C
    0x5E,  // D
    0x79,  // E
    0x71   // F
};

/***************************************************************************//**
 * @brief
 *   Initialize LED 7-Segment.
 *   This function should be called before the main loop
 *
 * @detail
 *  The SPI and PWM driver instances will be initialized automatically,
 *  during the sl_system_init() call in main.c.
 *****************************************************************************/
sl_status_t led_7seg_init(void)
{
  sl_status_t sc;

  /* Display number 0 */
  sc = led_7seg_display_number(0, LED_7SEG_NO_DOT);
  // Set the light intensity of LED 7-segment to 50%
  led_7seg_set_contrast(50);
  // Enable PWM output
  sl_pwm_start(&sl_pwm_mikroe);

  return sc;
}

/***************************************************************************//**
 * @brief
 *    Write a number on LED 7-segment display via SPI interface.
 *
 * @note
 *    The data received on the MISO wire is discarded.
 *    @n This function is blocking and returns when the transfer is complete.
 *
 * @param[in] number
 *    Transmit number that will display on the 7-segment.
 *
 * @param[in] dot
 *    Transmit dot that will display on the 7-segment.
 *
 * @return
 *    @ref SL_STATUS_OK on success or @ref SL_STATUS_FAIL on failure
 ******************************************************************************/
sl_status_t led_7seg_display_number(uint8_t number, dot_state_t dot)
{
  Ecode_t ret;

  uint8_t right_digit;
  uint8_t left_digit;
  uint8_t data_buf[2];

  number %= 100;

  left_digit = number / 10;
  right_digit = number % 10;

  if (dot == LED_7SEG_NO_DOT) {
    right_digit = number_array[right_digit];
    left_digit = number_array[left_digit];
  }
  else if (dot == LED_7SEG_LEFT_DOT) {
    right_digit = number_array[right_digit];
    left_digit = number_array[left_digit] | LED_7SEG_DOT;
  }
  else if (dot == LED_7SEG_RIGHT_DOT) {
    right_digit = number_array[right_digit] | LED_7SEG_DOT;
    left_digit = number_array[left_digit];
  }
  else if (dot == LED_7SEG_LEFT_RIGHT_DOT) {
    right_digit = number_array[right_digit] | LED_7SEG_DOT;
    left_digit = number_array[left_digit] | LED_7SEG_DOT;
  }

  if (left_digit == 0) {
    data_buf[0] = right_digit;
    data_buf[1] = 0;
  }
  else {
    data_buf[0] = right_digit;
    data_buf[1] = left_digit;
  }

  /* Send a block transfer to slave. */
  ret = SPIDRV_MTransmitB(spi_handle, data_buf, 2);
  if (ret != ECODE_EMDRV_SPIDRV_OK) {
    return SL_STATUS_FAIL;
  }

  /* Note that at this point all the data is loaded into the fifo, this does
   * not necessarily mean that the transfer is complete. */
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *    Set the light intensity of LED 7-segment.
 *
 * @param[in] percent
 *   Percent of the light intensity.
 *   0 <-> off
 *   100 <-> max of the light intensity
 *
 * @return
 *    @ref SL_STATUS_OK on success or @ref SL_STATUS_FAIL on failure
 ******************************************************************************/
void led_7seg_set_contrast(uint8_t percent)
{
   // Set duty cycle
   sl_pwm_set_duty_cycle(&sl_pwm_mikroe, percent);
}

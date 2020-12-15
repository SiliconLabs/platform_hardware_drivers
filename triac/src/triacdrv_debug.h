/***************************************************************************//**
* @file triacdrv_debug.h
* @brief ACMP/PRS/TIMER-driven automated triac driver debug
*        configuration to assist in porting.
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
* # Evaluation Quality
* This code has been minimally tested to ensure that it builds and is suitable
* as a demonstration for evaluation purposes only. This code will be maintained
* at the sole discretion of Silicon Labs.
******************************************************************************/

#ifndef TRIACDRV_DEBUG_H_
#define TRIACDRV_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NOTE: Depending on the device and its (W)STK, it may not be possible
 * to map all debug outputs to expansion header pins.  For example, the
 * pins shown below are for EFM32TG11.  While it would be desirable to
 * map TRIACDRV_DEBUG_PULSE_FALL_PRS_OUT to PC9 (PRS_CH5 location 0),
 * this is not possible, at least in the context of the test program
 * because PC9 is the GPIO for button 1.  Instead, it has to be mapped
 * to PD6 (location 2), which is also the GPIO specified for
 * TRIACDRV_DEBUG_ACMPOUT, so both cannot be driven at the same time.
 */
#ifdef TRIACDRV_DEBUG

// Enable ACMP output pin for observation
//#define TRIACDRV_DEBUG_ACMPOUT
#ifdef TRIACDRV_DEBUG_ACMPOUT
// Port and pin for debug output
#define TRIACDRV_DEBUG_ACMPOUT_PORT  gpioPortD
#define TRIACDRV_DEBUG_ACMPOUT_PIN   6
#define TRIACDRV_DEBUG_ACMPOUT_LOC   2
#endif /* DEBUG_ACMPOUT */

//#define TRIACDRV_DEBUG_PRSACMP
#ifdef TRIACDRV_DEBUG_PRSACMP
// Port and pin for debug output
#define TRIACDRV_DEBUG_PRSACMP_PORT  gpioPortC
#define TRIACDRV_DEBUG_PRSACMP_PIN   1
#define TRIACDRV_DEBUG_PRSACMP_LOC   0
#endif /* DEBUG_ACMPOUT */

//#define TRIACDRV_DEBUG_PULSE_RISE_TIMER_OUT
#ifdef TRIACDRV_DEBUG_PULSE_RISE_TIMER_OUT
// Port and pin for gate enable pulse leading edge output
#define TRIACDRV_DEBUG_PULSE_RISE_TIMER_OUT_PORT    gpioPortD   // TIM1_CC1
#define TRIACDRV_DEBUG_PULSE_RISE_TIMER_OUT_PIN     7
#define TRIACDRV_DEBUG_PULSE_RISE_TIMER_OUT_LOC     4
#endif /* TRIACDRV_DEBUG_PULSE_RISE_TIMER_OUT */

//#define TRIACDRV_DEBUG_PULSE_FALL_TIMER_OUT
#ifdef TRIACDRV_DEBUG_PULSE_FALL_TIMER_OUT
// Port and pin for gate enable pulse falling edge output
#define TRIACDRV_DEBUG_PULSE_FALL_TIMER_OUT_PORT    gpioPortC   // TIM1_CC2
#define TRIACDRV_DEBUG_PULSE_FALL_TIMER_OUT_PIN     13
#define TRIACDRV_DEBUG_PULSE_FALL_TIMER_OUT_LOC     4
#endif /* TRIACDRV_DEBUG_PULSE_FALL_TIMER_OUT */

/*
 * Note that TRIACDRV_DEBUG_PULSE_RISE_PRS_OUT is kind of redundant in
 * the scheme of things.  Because the PRS logic function is "AND with
 * next" (vs. "OR with previous"), the gate enable pulse rising edge
 * is, by definition, the gate enable pulse once ANDed with the next
 * PRS channel (the falling edge).
 */
//#define TRIACDRV_DEBUG_PULSE_RISE_PRS_OUT
#ifdef TRIACDRV_DEBUG_PULSE_RISE_PRS_OUT
// Port and pin for gate enable pulse leading edge output
#define TRIACDRV_DEBUG_PULSE_RISE_PRS_OUT_PORT      gpioPortC   // PRS_CH4
#define TRIACDRV_DEBUG_PULSE_RISE_PRS_OUT_PIN       8
#define TRIACDRV_DEBUG_PULSE_RISE_PRS_OUT_LOC       0
#endif /* TRIACDRV_DEBUG_PULSE_RISE_PRS_OUT */

//#define TRIACDRV_DEBUG_PULSE_FALL_PRS_OUT
#ifdef TRIACDRV_DEBUG_PULSE_FALL_PRS_OUT
// Port and pin for gate enable pulse falling edge output
#define TRIACDRV_DEBUG_PULSE_FALL_PRS_OUT_PORT      gpioPortD   // PRS_CH5
#define TRIACDRV_DEBUG_PULSE_FALL_PRS_OUT_PIN       6
#define TRIACDRV_DEBUG_PULSE_FALL_PRS_OUT_LOC       2
#endif /* TRIACDRV_DEBUG_PULSE_FALL_TIMER_OUT */

#endif /* TRIACDRV_DEBUG */

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* TRIACDRV_DEBUG_H_ */

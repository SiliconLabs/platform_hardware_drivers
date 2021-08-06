/***************************************************************************//**
 * @file dc_motor_TB6549FG_config.h
 * @brief TB6549FG driver configuration definitions
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

#ifndef DC_MOTOR_TB6549FG_CONFIG_H_
#define DC_MOTOR_TB6549FG_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// For the BGM220 Explorer Kit (BRD4314A)
#define TB6549FG_IN1_PORT   gpioPortB
#define TB6549FG_IN1_PIN    0

#define TB6549FG_IN2_PORT   gpioPortC
#define TB6549FG_IN2_PIN    6

#define TB6549FG_SB_PORT    gpioPortC
#define TB6549FG_SB_PIN     3

#define TB6549FG_PWM_PORT   gpioPortB
#define TB6549FG_PWM_PIN    4
#define TB6549FG_PWM_LOC    0         //0 for compatibility

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* DC_MOTOR_TB6549FG_CONFIG_H_ */

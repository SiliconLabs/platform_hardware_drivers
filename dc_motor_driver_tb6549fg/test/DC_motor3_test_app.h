/***************************************************************************//**
 * @file DC_motor3_test_app.h
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

#ifndef DC_MOTOR3_TEST_APP_H_
#define DC_MOTOR3_TEST_APP_H_

#ifdef __cplusplus
extern "C" {
#endif
// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/**
 * Typedef for the test app state-machine
 */
typedef enum{
  STANDBY    = 0,
  STOP       = 1,
  SHORTBREAK = 2,
  CW         = 3,
  CW_cycle   = 4,
  CCW        = 5,
  CCW_cycle  = 6,
  NUM_STATES
}test_states_TypeDef;

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
void dc_motor3_app_init(bool sleep_enable);
void dc_moto3_app_process_action(void);

#ifdef __cplusplus
}
#endif

#endif /* DC_MOTOR3_TEST_APP_H_ */

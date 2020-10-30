/***************************************************************************//**
 * @file   nt3h2x11_fd.h
 * @brief  Types for field detection interface for NT3H2x11.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
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
 ******************************************************************************/

/* ...
 *
 * EVALUATION QUALITY
 * This code has been minimally tested to ensure that it builds with the specified dependency versions and is suitable as a demonstration for evaluation purposes only.
 * This code will be maintained at the sole discretion of Silicon Labs.
 *
... */

#ifndef __NT3H2X11_FD_H__

/***************************************************************************//**
 * @addtogroup NT3H2x11 Driver
 * @brief NT3H2x11 Field Detection (FD) types.
 * @{
 ******************************************************************************/

#define __NT3H2X11_FD_H__

#include <stdint.h>
#include <stdbool.h>
#include "em_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  bool              enable;
  GPIO_Port_TypeDef fd_port;
  uint8_t           fd_pin;
} nt3h2x11_fd_init_t;

#ifdef __cplusplus
}
#endif
/** @} (end addtogroup NT3H2x11 Driver) */
#endif
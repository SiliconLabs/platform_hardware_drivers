/***************************************************************************//**
 * @file app_iostream_usart.h
 * @brief header file for simple CLI
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef APP_IOSTREAM_USART_H
#define APP_IOSTREAM_USART_H

#include <stdint.h>

/*******************************************************************************
 ********************************   MACROS   ***********************************
 ******************************************************************************/
#define CLI_CMD_LENGTH 10

/*******************************************************************************
 **************************   STRUCTURES   *******************************
 ******************************************************************************/
typedef struct {
  uint8_t cmd[CLI_CMD_LENGTH];
  void (*handler)();
} cli_cmd_t;

/***************************************************************************//**
 * @brief
 *    Initialize IOstream usart.
 ******************************************************************************/
void app_iostream_usart_init(void);

/***************************************************************************//**
 * @brief
 *    IOstream usart ticking function.
 ******************************************************************************/
void app_iostream_usart_process_action(void);

#endif  // APP_IOSTREAM_USART_H

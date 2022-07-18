/***************************************************************************//**
 * @file
 * @brief iostream usart examples functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
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

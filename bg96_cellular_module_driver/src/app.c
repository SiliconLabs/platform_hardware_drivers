/***************************************************************************//**
 * @file
 * @brief Top level application functions
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

#include "app_iostream_usart.h"
#include "bg96_driver.h"
#include "nb_iot.h"
#include "at_parser_core.h"
#include "at_parser_core.h"
#include "at_parser_events.h"
#include <stdio.h>

void app_init (void)
{
  bg96_nb_init ();
}

void app_process_action (void)
{
  app_iostream_usart_process_action ();
  at_parser_process ();
  at_event_process ();
  bg96_process ();
}

/*
 * at_parser_events.c
 *
 *  Created on: 2022. júl. 11.
 *      Author: sapolyak
 */
#include "at_parser_events.h"
#include <stdbool.h>
#include <stddef.h>

static void (*global_handle)();
static uint8_t global_ok_value;
static uint8_t *global_event_flag;
static bool event_handled = true;
static void *global_handler_data;

/**************************************************************************//**
 * @brief
 *    AT parser event listener listen function.
 *    This function can handle only one event at the same time.
 *
 * @param[in] event_flag
 *    Pointer to the flag to listen to.
 *
 * @param[in] event_flag_ok_value
 *    The decent flag value when the callback function should be called.
 *
 * @param[out] handle
 *    Pointer to a callback function.
 *
 * @param[in] handler_data
 *    Pointer to the data which will be given as callback parameter.
 *
 * @return
 *   SL_STATUS_OK if event listener has been set.
 *   SL_STATUS_ALLOCATION_FAILED if listener is already running.
 *
 *****************************************************************************/
sl_status_t at_listen_event(uint8_t *event_flag,
                            uint8_t event_ok_value,
                            void (*handle)(void*),
                            void *handler_data)
{
  global_event_flag = event_flag;
  global_ok_value = event_ok_value;
  global_handler_data = handler_data;
  if (handle != NULL && event_handled == true) {
    global_handle = handle;
    event_handled = false;
    return SL_STATUS_OK;
  }
  return SL_STATUS_ALLOCATION_FAILED;
}

/**************************************************************************//**
 * @brief
 *    AT parser event listener process function.
 *    This function SHALL be called periodically in the main loop.
 *
 *****************************************************************************/
void at_event_process(void)
{
  if (!event_handled
      && *global_event_flag == global_ok_value&& global_handle !=NULL) {
    global_handle(global_handler_data);
    event_handled = true;
  }
}


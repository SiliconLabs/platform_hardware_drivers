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
#include <stdio.h>
#include <string.h>
#include "em_chip.h"
#include "sl_iostream.h"
#include "sl_iostream_init_instances.h"
#include "sl_iostream_handles.h"
#include "at_parser_core.h"
#include "app_iostream_usart.h"
#include "at_parser_events.h"
#include "bg96_driver.h"
#include "nb_iot.h"
#include "nb_gnss.h"

/*******************************************************************************
 *******************************   MACROS   ************************************
 ******************************************************************************/
#ifndef BUFSIZE
#define BUFSIZE    80
#endif

/*******************************************************************************
 ***************   STATIC FUNCTION DECLARATIONS   ******************************
 ******************************************************************************/
static void app_parser(uint8_t *buf);
static void imei();
static void ip();
static void wakeup();
static void sleep();
static void net_reg();
static void open();
static void send();
static void close();
static void cops();
static void receive();
static void gps_start();
static void gps_location();
static void gps_stop();
static void open_server();
static void close_server();

static void imei_handler(void *handler_data);
static void ip_handler(void *handler_data);
static void wakeup_handler(void *handler_data);
static void sleep_handler(void *handler_data);
static void net_reg_handler(void *handler_data);
static void open_handler(void *handler_data);
static void close_handler(void *handler_data);
static void send_handler(void *handler_data);
static void recv_handler(void *handler_data);
static void cops_handler(void *handler_data);
static void stop_gnss_handler(void *handler_data);
static void get_position_handler(void *handler_data);
static void start_gnss_handler(void *handler_data);

/*******************************************************************************
 **************************  GLOBAL VAAIBLES   *********************************
 ******************************************************************************/
static uint8_t buffer[BUFSIZE];

/**************************************************************************//**
 * @brief
 *   Command and handler definitions for the CLI.
 *
 *****************************************************************************/
static cli_cmd_t cli_cmds[] = {
                                { "imei", imei },
                                { "ip", ip },
                                { "wakeup", wakeup },
                                { "sleep", sleep },
                                { "netreg", net_reg },
                                { "open", open },
                                { "send", send },
                                { "close", close },
                                { "cops", cops },
                                { "recv", receive },
                                { "gpsstart", gps_start },
                                { "location", gps_location },
                                { "gpsstop", gps_stop },
                                { "opens", open_server },
                                { "closes", close_server }, };
static uint8_t cli_cmd_size = sizeof(cli_cmds) / sizeof(cli_cmds[0]);

at_scheduler_status_t output_object = {
SL_STATUS_NOT_INITIALIZED,
                                        0, "" };

/**************************************************************************//**
 * @brief
 *   App layer process function.
 *   This function SHALL be called periodically in the main loop.
 *
 *****************************************************************************/
void app_iostream_usart_process_action(void)
{
  int8_t c = 0;
  static uint8_t index = 0;

  /* Retrieve characters, print local echo and full line back */
  c = getchar();
  if (c > 0) {
    if (c == '\r' || c == '\n') {
      putchar('\r');
      putchar('\n');
      buffer[index] = '\0';
      app_parser(buffer);
      index = 0;
    } else {
      if (index < BUFSIZE - 1) {
        buffer[index] = c;
        index++;
      }
      /* Local echo */
      putchar(c);
    }
  }
}

/**************************************************************************//**
 * @brief
 *   App lacer incoming command parser function.
 *
 * @param[in] buf
 *    Pointer to the buffer which contains the input command.
 *
 *****************************************************************************/
static void app_parser(uint8_t *buf)
{
  uint8_t i;

  for (i = 0; i < cli_cmd_size; i++) {
    if (!strcmp((const char*) cli_cmds[i].cmd, (const char*) buf)) {
      cli_cmds[i].handler();
      return;
    }
  }
  printf("Can't recognize command!\r\n");
}

/**************************************************************************//**
 *  Command handler functions
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *    Get IMEI function.
 *    Result will be available in the global output_object.
 *
 ******************************************************************************/
static void imei()
{
  at_parser_init_output_object(&output_object);
  read_imei(&output_object);
  at_listen_event((uint8_t*) &output_object.status, SL_STATUS_OK, imei_handler,
      (void*) &output_object);
  printf("Read IMEI command sent!\r\n");
}

/***************************************************************************//**
 * @brief
 *    Get IMEI handler function.
 *
 * @param[in] handler_data
 *    Data sent by the event handler.
 *    Currently  handler_data is a pointer to an at_scheduler_status_t.
 *
 ******************************************************************************/
static void imei_handler(void *handler_data)
{
  at_scheduler_status_t *l_output = (at_scheduler_status_t*) handler_data;

  if (l_output->error_code) {
    printf("Error while reading IMEI: %d\r\n", l_output->error_code);
  } else {
    printf("IMEI: %s\r\n", l_output->response_data);
  }
}

/***************************************************************************//**
 * @brief
 *    Get IO address function.
 *    Result will be available in the global output_object.
 *
 ******************************************************************************/
static void ip()
{
  at_parser_init_output_object(&output_object);
  read_ip(&output_object);
  at_listen_event((uint8_t*) &output_object.status, SL_STATUS_OK, ip_handler,
      (void*) &output_object);
  printf("Read IP command sent!\r\n");
}

/***************************************************************************//**
 * @brief
 *    Get IP address handler function.
 *
 * @param[in] handler_data
 *    Data sent by the event handler.
 *    Currently  handler_data is a pointer to an at_scheduler_status_t.
 *
 ******************************************************************************/
static void ip_handler(void *handler_data)
{
  at_scheduler_status_t *l_output = (at_scheduler_status_t*) handler_data;
  if (l_output->error_code) {
    printf("Error while reading IP: %d\r\n%s\r\n", l_output->error_code,
        l_output->response_data);
  } else {
    printf("IP address: %s\r\n", l_output->response_data);
  }
}

/***************************************************************************//**
 * @brief
 *    Wake up  function.
 *    Result will be available in the global output_object.
 *
 ******************************************************************************/
static void wakeup()
{
  at_parser_init_output_object(&output_object);
  bg96_wake_up(&output_object);
  at_listen_event((uint8_t*) &output_object.status, SL_STATUS_OK,
      wakeup_handler, (void*) &output_object);
  printf("Waking up device!\r\n");
}

/***************************************************************************//**
 * @brief
 *    Wake up handler function.
 *
 * @param[in] handler_data
 *    Data sent by the event handler.
 *    Currently  handler_data is a pointer to an at_scheduler_status_t.
 *
 ******************************************************************************/
static void wakeup_handler(void *handler_data)
{
  at_scheduler_status_t *l_output = (at_scheduler_status_t*) handler_data;
  if (l_output->error_code) {
    printf("Error while waking up: %d\r\n", l_output->error_code);
  } else {
    printf("Device is up!\r\n");
  }
}

/***************************************************************************//**
 * @brief
 *    Sleep function.
 *    Result will be available in the global output_object.
 *
 ******************************************************************************/
static void sleep()
{
  at_parser_init_output_object(&output_object);
  bg96_sleep(&output_object);
  at_listen_event((uint8_t*) &output_object.status, SL_STATUS_OK, sleep_handler,
      (void*) &output_object);
  printf("Put the device to sleep!\r\n");
}

/***************************************************************************//**
 * @brief
 *    Sleep handler function.
 *
 * @param[in] handler_data
 *    Data sent by the event handler.
 *    Currently  handler_data is a pointer to an at_scheduler_status_t.
 *
 ******************************************************************************/
static void sleep_handler(void *handler_data)
{
  at_scheduler_status_t *l_output = (at_scheduler_status_t*) handler_data;
  if (l_output->error_code) {
    printf("Error while putting to sleep: %d\r\n", l_output->error_code);
  } else {
    printf("Device went to sleep!\r\n");
  }
}

/***************************************************************************//**
 * @brief
 *    Network registration function.
 *    Result will be available in the global output_object.
 *
 ******************************************************************************/
static void net_reg()
{
  at_parser_init_output_object(&output_object);
  bg96_network_registration(&output_object);
  at_listen_event((uint8_t*) &output_object.status, SL_STATUS_OK,
      net_reg_handler, (void*) &output_object);
  printf("Network registration started!\r\n");
}

/***************************************************************************//**
 * @brief
 *    Network registration handler function.
 *
 * @param[in] handler_data
 *    Data sent by the event handler.
 *    Currently  handler_data is a pointer to an at_scheduler_status_t.
 *
 ******************************************************************************/
static void net_reg_handler(void *handler_data)
{
  at_scheduler_status_t *l_output = (at_scheduler_status_t*) handler_data;
  if (l_output->error_code) {
    printf("Network registration error: %d\r\n%s\r\n", l_output->error_code,
        l_output->response_data);
  } else {
    printf("Network registration done!\r\n");
  }
}

/***************************************************************************//**
 * @brief
 *    Open client connection function.
 *    Result will be available in the global output_object.
 *
 ******************************************************************************/
static void open()
{
  bg96_nb_connection_t connection = {
                                      0,
                                      9999,
                                      "TCP",
                                      (uint8_t*) "cloudsocket.hologram.io" };

  at_parser_init_output_object(&output_object);
  bg96_nb_open_connection(&connection, &output_object);
  at_listen_event((uint8_t*) &output_object.status, SL_STATUS_OK, open_handler,
      (void*) &output_object);
  printf("Open command sent!\r\n");
}

/***************************************************************************//**
 * @brief
 *    Open client connection handler function.
 *
 * @param[in] handler_data
 *    Data sent by the event handler.
 *    Currently  handler_data is a pointer to an at_scheduler_status_t.
 *
 ******************************************************************************/
static void open_handler(void *handler_data)
{
  at_scheduler_status_t *l_output = (at_scheduler_status_t*) handler_data;

  if (l_output->error_code) {
    printf("Network open error: %d\r\n", (int) l_output->error_code);
  } else {
    printf("Network opened.\r\n");
  }
}

/***************************************************************************//**
 * @brief
 *    Open server connection function.
 *    Result will be available in the global output_object.
 *
 ******************************************************************************/
static void open_server()
{
  bg96_nb_connection_t connection = {
                                      0,
                                      2020,
                                      "TCP LISTENER",
                                      (uint8_t*) "127.0.0.1" };

  at_parser_init_output_object(&output_object);
  bg96_nb_open_connection(&connection, &output_object);
  printf("Open server command sent!\r\n");
}

/***************************************************************************//**
 * @brief
 *    Close server connection function.
 *    Result will be available in the global output_object.
 ******************************************************************************/
static void close_server()
{
  bg96_nb_connection_t connection = {
                                      11,
                                      2020,
                                      "TCP LISTENER",
                                      (uint8_t*) "127.0.0.1" };

  at_parser_init_output_object(&output_object);
  bg96_nb_close_connection(&connection, &output_object);
  printf("Open server command sent!\r\n");
}

/***************************************************************************//**
 * @brief
 *    Send data on an opened channel.
 *    Result will be available in the global output_object.
 *
 ******************************************************************************/
static void send()
{
  bg96_nb_connection_t connection = {
                                      0,
                                      9999,
                                      "TCP",
                                      (uint8_t*) "cloudsocket.hologram.io" };
  uint8_t data_to_send[] =
      "{\"k\":\"Tm}hswZ8\",\"d\":\"Hello Silabs!\",\"t\":\"my_topic\"}";

  at_parser_init_output_object(&output_object);
  bg96_nb_send_data(&connection, data_to_send, &output_object);
  at_listen_event((uint8_t*) &output_object.status, SL_STATUS_OK, send_handler,
      (void*) &output_object);
  printf("Data has been sent!\r\n");
}

/***************************************************************************//**
 * @brief
 *    Send data handler function.
 *
 * @param[in] handler_data
 *    Data sent by the event handler.
 *    Currently  handler_data is a pointer to an at_scheduler_status_t.
 *
 ******************************************************************************/
static void send_handler(void *handler_data)
{
  at_scheduler_status_t *l_output = (at_scheduler_status_t*) handler_data;
  printf("Send response received! Error: %d Data: %s\r\n", l_output->error_code,
      l_output->response_data);
}


/***************************************************************************//**
 * @brief
 *    Close client connection function.
 *    Result will be available in the global output_object.
 *
 ******************************************************************************/
static void close()
{
  bg96_nb_connection_t connection = {
                                      0,
                                      9999,
                                      "TCP",
                                      (uint8_t*) "cloudsocket.hologram.io" };

  at_parser_init_output_object(&output_object);
  bg96_nb_close_connection(&connection, &output_object);
  at_listen_event((uint8_t*) &output_object.status, SL_STATUS_OK, close_handler,
      (void*) &output_object);
  printf("Close command sent!\r\n");
}

/***************************************************************************//**
 * @brief
 *    Close client connection handler function.
 *
 * @param[in] handler_data
 *    Data sent by the event handler.
 *    Currently  handler_data is a pointer to an at_scheduler_status_t.
 *
 ******************************************************************************/
static void close_handler(void *handler_data)
{
  at_scheduler_status_t *l_output = (at_scheduler_status_t*) handler_data;

  if (l_output->error_code) {
    printf("Network close error: %d\r\n %s\r\n", l_output->error_code,
        l_output->response_data);
  } else {
    printf("Network closed\r\n");
  }
}

/***************************************************************************//**
 * @brief
 *    Get actual operator function.
 *    Result will be available in the global output_object.
 *
 ******************************************************************************/
static void cops()
{
  at_parser_init_output_object(&output_object);
  bg96_get_operator(&output_object);
  at_listen_event((uint8_t*) &output_object.status, SL_STATUS_OK, cops_handler,
      (void*) &output_object);
  printf("Getting operator!\r\n");
}

/***************************************************************************//**
 * @brief
 *    Get actual operator handler function.
 *
 * @param[in] handler_data
 *    Data sent by the event handler.
 *    Currently  handler_data is a pointer to an at_scheduler_status_t.
 *
 ******************************************************************************/
static void cops_handler(void *handler_data)
{
  at_scheduler_status_t *l_output = (at_scheduler_status_t*) handler_data;

  if (l_output->error_code) {
    printf("Error while getting operator: %d\r\n %s\r\n", l_output->error_code,
        l_output->response_data);
  } else {
    printf("Actual operator: %s\r\n", l_output->response_data);
  }
}

/***************************************************************************//**
 * @brief
 *    Receive data function.
 *    Result will be available in the global output_object.
 *
 ******************************************************************************/
static void receive()
{
  at_parser_init_output_object(&output_object);
  bg96_nb_receive_data(&output_object);
  at_listen_event((uint8_t*) &output_object.status, SL_STATUS_OK, recv_handler,
      (void*) &output_object);
  printf("Receiving data!\r\n");
}

/***************************************************************************//**
 * @brief
 *    Receive data handler function.
 *
 * @param[in] handler_data
 *    Data sent by the event handler.
 *    Currently  handler_data is a pointer to an at_scheduler_status_t.
 *
 ******************************************************************************/
static void recv_handler(void *handler_data)
{
  at_scheduler_status_t *l_output = (at_scheduler_status_t*) handler_data;
  if (l_output->error_code) {
    printf("Error while receiving data: %d\r\n %s\r\n", l_output->error_code,
        l_output->response_data);
  } else {
    printf("Received data: %s\r\n", l_output->response_data);
  }
}

/***************************************************************************//**
 * @brief
 *    GNSS start function.
 *    Result will be available in the global output_object.
 *
 ******************************************************************************/
static void gps_start()
{
  at_parser_init_output_object(&output_object);
  gnss_start(&output_object);
  at_listen_event((uint8_t*) &output_object.status, SL_STATUS_OK,
      start_gnss_handler, (void*) &output_object);
  printf("GNSS start command sent.\r\n");
}

/***************************************************************************//**
 * @brief
 *    Start GNSS handler function.
 *
 * @param[in] handler_data
 *    Data sent by the event handler.
 *    Currently  handler_data is a pointer to an at_scheduler_status_t.
 *
 ******************************************************************************/
static void start_gnss_handler(void *handler_data)
{
  at_scheduler_status_t *l_output = (at_scheduler_status_t*) handler_data;

  if (l_output->error_code) {
    printf("Error while starting GNSS: %d\r\n", l_output->error_code);
  } else {
    printf("GNSS started.\r\n");
  }
}

/***************************************************************************//**
 * @brief
 *    GNSS get location function.
 *    Result will be available in the global output_object.
 *
 ******************************************************************************/
static void gps_location()
{
  at_parser_init_output_object(&output_object);
  gnss_get_position(&output_object);
  at_listen_event((uint8_t*) &output_object.status, SL_STATUS_OK,
      get_position_handler, (void*) &output_object);
  printf("GNSS position command sent.\r\n");
}

/***************************************************************************//**
 * @brief
 *    Get GNSS location handler function.
 *
 * @param[in] handler_data
 *    Data sent by the event handler.
 *    Currently  handler_data is a pointer to an at_scheduler_status_t.
 *
 ******************************************************************************/
static void get_position_handler(void *handler_data)
{
  at_scheduler_status_t *l_output = (at_scheduler_status_t*) handler_data;

  if (l_output->error_code) {
    printf("Error while getting position: %d\r\n %s\r\n", l_output->error_code,
        l_output->response_data);
  } else {
    printf("Actual position: %s\r\n", l_output->response_data);
  }
}

/***************************************************************************//**
 * @brief
 *    GNSS stop listening function.
 *    Result will be available in the global output_object.
 *
 ******************************************************************************/
static void gps_stop()
{
  at_parser_init_output_object(&output_object);
  at_listen_event((uint8_t*) &output_object.status, SL_STATUS_OK,
      stop_gnss_handler, (void*) &output_object);
  gnss_stop(&output_object);
  printf("GNSS stop command sent.\r\n");
}

/***************************************************************************//**
 * @brief
 *    Stop GNSS handler function.
 *
 * @param[in] handler_data
 *    Data sent by the event handler.
 *    Currently  handler_data is a pointer to an at_scheduler_status_t.
 *
 ******************************************************************************/
static void stop_gnss_handler(void *handler_data)
{
  at_scheduler_status_t *l_output = (at_scheduler_status_t*) handler_data;

  if (l_output->error_code) {
    printf("Error while stopping GNSS: %d\r\n", l_output->error_code);
  } else {
    printf("GNSS stopped.\r\n");
  }
}

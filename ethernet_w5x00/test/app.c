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
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "app_assert.h"
#include "w5x00_utils.h"
#include "ethernet.h"
#include "ethernet_client.h"
#include "ethernet_server.h"
#include "dns.h"
#include "sntp.h"
#include "http_server.h"
#include "w5x00.h"
#include "w5x00_utils.h"
#include "app_log.h"

#define USE_DHCP

#define app_log_print_ip(ip)                             \
  app_log("%d.%d.%d.%d", w5x00_ip4_addr_get_byte(ip, 0), \
          w5x00_ip4_addr_get_byte(ip, 1),                \
          w5x00_ip4_addr_get_byte(ip, 2),                \
                         w5x00_ip4_addr_get_byte(ip, 3))

uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
w5x00_ip4_addr_t ip = { WIZNET_IP4_DATA(127, 0, 0, 1) };
w5x00_ip4_addr_t dns_server1 = { WIZNET_IP4_DATA(1, 1, 1, 1) };
w5x00_ip4_addr_t dns_server2 = { WIZNET_IP4_DATA(8, 8, 8, 8) };

w5x00_ethernet_t eth;
w5x00_ethernet_client_t client;

w5x00_ip4_addr_t local_ip = { WIZNET_IP4_DATA(10, 42, 0, 242) };
w5x00_ip4_addr_t gateway_ip = { WIZNET_IP4_DATA(10, 42, 0, 1) };
w5x00_ip4_addr_t subnet_mask = { WIZNET_IP4_DATA(255, 255, 255, 0) };

static const char *get_content_body(const char *uri)
{
  const char *start, *end;

  start = end = uri;
  while (*uri) {
    if (((uri[0] == '\r') && (uri[1] == '\n'))
        || ((uri[0] == '\n') && (uri[1] == '\r'))) { // line ending with <cr><lf>
      if (end == start) { // checking for empty line
        if (uri[2]) {
          return &uri[2]; // body is the next line
        } else {
          break; // next line is not found
        }
      }
      end = start = &uri[2];
      uri += 2;
    } else if ((uri[0] == '\n')
               && (uri[1] != '\r')) { // line ending with <lf>
      if (end == start) {   // checking for empty line
        if (uri[1]) {
          return &uri[1];   // body is the next line
        } else {
          break;   // next line is not found
          }
        }
        end = start = &uri[1];
        uri += 1;
    } else {
      end = uri++;
    }
  }
  return NULL;
}

void http_client_get_device_public_ip(const char *host,
                                      uint16_t port,
                                      const char *path)
{
  static char message[1024];

  w5x00_ethernet_client_init(&client, &eth, 10000);
  sl_sleeptimer_delay_millisecond(1000);
  app_log("Connecting to: %s:%d\r\n", host, port);
  if (SL_STATUS_OK == w5x00_ethernet_client_connect_host(&client,
                                                         host,
                                                         port)) {
    app_log("\r\nConnected!\r\n\r\n");

    snprintf(message,
             sizeof(message),
             "GET http://%s%s HTTP/1.1\r\nHost: %s\r\nAccept: text/html\r\n\r\n",
             host,
             path,
             host);
    app_log("HTTP Request:\r\n\r\n%s\r\n\r\n", message);
    while (w5x00_ethernet_client_available_for_write(&client)
            < (int)strlen(message)) {}
    w5x00_ethernet_client_write(&client,
                                (uint8_t *)message,
                                strlen(message));
//    sl_sleeptimer_delay_millisecond(500);
    while (w5x00_ethernet_client_connected(&client)) {
      int length = w5x00_ethernet_client_read(&client,
                                              (uint8_t *)message,
                                              sizeof(message) - 1);
      if ((length > 0) && (length < (int)sizeof(message))) {
        message[length] = '\0';
        w5x00_ip4_addr_t ip;
        const char *body;

        app_log("HTTP Response:\r\n\r\n%s\r\n\r\n", message);

        body = get_content_body(message);
//        printf("%.*s", length, message);
        app_log("HTTP Response body (public ip address): %s\r\n", body);
        if (body
            && w5x00_ip4addr_aton(body, &ip)) {
          app_log("Public ip (parsed from the body): %d.%d.%d.%d\r\n\r\n",
              w5x00_ip4_addr_get_byte(&ip, 0),
              w5x00_ip4_addr_get_byte(&ip, 1),
              w5x00_ip4_addr_get_byte(&ip, 2),
              w5x00_ip4_addr_get_byte(&ip, 3));
        } else {
          app_log("Parse the ip from HTTP response body failed\r\n");
        }
      }
    }
    app_log("Connection remote closed!\r\n");
  } else {
    app_log("Connect eror!\r\n");
  }
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  sl_status_t status;

#ifdef USE_DHCP
  status = w5x00_ethernet_dhcp_init(&eth, mac, 30000, 10000);
  app_log("DHCP configuration: %s\r\n",
          SL_STATUS_OK == status ? "success":"failed");
#else
    status = w5x00_ethernet_static_init(&eth,
                                        mac,
                                        &local_ip,
                                        &gateway_ip,
                                        &subnet_mask,
                                        &dns_server1,
                                        3000);
    app_log("Static address configuration: %s\r\n",
              SL_STATUS_OK == status ? "success":"failed");
#endif

  if (SL_STATUS_OK != status) {
    enum EthernetLinkStatus link_status;

    link_status = w5x00_ethernet_link_status(&eth);
    if (EthernetLinkON == link_status) {
      app_log("Ethernet link status is on\r\n");
    } else if (EthernetLinkOFF == link_status) {
      app_log("Ethernet link status is off\r\n");
    }
  }

  w5x00_ethernet_set_dns_server(&eth, dns_server1);
//  w5x00_ethernet_set_dns_server(&eth, dns_server2);

  memset(&local_ip, 0, sizeof(local_ip));
  w5x00_ethernet_get_local_ip(&eth, &local_ip);
  app_log("local ip:    ");
  app_log_print_ip(&local_ip);
  app_log("\r\n");

  memset(&gateway_ip, 0, sizeof(gateway_ip));
  w5x00_ethernet_get_gateway_ip(&eth, &gateway_ip);
  app_log("gateway:     ");
  app_log_print_ip(&gateway_ip);
  app_log("\r\n");

  memset(&subnet_mask, 0, sizeof(subnet_mask));
  w5x00_ethernet_get_subnet_mask(&eth, &subnet_mask);
  app_log("subnet mask: ");
  app_log_print_ip(&subnet_mask);
  app_log("\r\n");

  app_log("dns:         ");
  app_log_print_ip(&eth.dns_server_address);
  app_log("\r\n");
  http_client_get_device_public_ip("checkip.amazonaws.com", 80, "/");
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

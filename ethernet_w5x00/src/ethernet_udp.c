/*
 *  Udp.cpp: Library to send/receive UDP packets with the Arduino ethernet shield.
 *  This version only offers minimal wrapping of socket.cpp
 *  Drop Udp.h/.cpp into the Ethernet library directory at hardware/libraries/Ethernet/
 *
 * MIT License:
 * Copyright (c) 2008 Bjoern Hartmann
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * bjoern@cs.stanford.edu 12/30/2008
 */
#include <string.h>

#include "ethernet.h"
#include "socket.h"
#include "w5x00.h"
#include "ethernet_udp.h"

/***************************************************************************//**
 * Ethernet UDP Protocol Init.
 ******************************************************************************/
sl_status_t ethernet_udp_init(w5x00_ethernet_udp_t *e_udp)
{
  if (e_udp == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  e_udp->sockindex = W5x00_MAX_SOCK_NUM;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Ethernet UDP Begin.
 ******************************************************************************/
sl_status_t w5x00_ethernet_udp_begin(w5x00_ethernet_udp_t *e_udp, uint16_t port)
{
  if (e_udp == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (e_udp->sockindex < W5x00_MAX_SOCK_NUM) {
    w5x00_socket_close(e_udp->sockindex);
  }
  e_udp->sockindex = w5x00_socket_begin(SnMR_UDP, port);
  if (e_udp->sockindex >= W5x00_MAX_SOCK_NUM) {
    return SL_STATUS_FAIL;
  }
  e_udp->port = port;
  e_udp->remaining = 0;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Ethernet UDP Available.
 ******************************************************************************/
int w5x00_ethernet_udp_available(w5x00_ethernet_udp_t *e_udp)
{
  if (e_udp == NULL) {
    return -1;
  }
  return e_udp->remaining;
}

/***************************************************************************//**
 * Ethernet UDP Stop.
 ******************************************************************************/
sl_status_t w5x00_ethernet_udp_stop(w5x00_ethernet_udp_t *e_udp)
{
  if (e_udp == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (e_udp->sockindex < W5x00_MAX_SOCK_NUM) {
    w5x00_socket_close(e_udp->sockindex);
    e_udp->sockindex = W5x00_MAX_SOCK_NUM;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Ethernet UDP Begin Packet.
 ******************************************************************************/
sl_status_t w5x00_ethernet_udp_begin_packet(  w5x00_ethernet_udp_t *e_udp,
                                              w5x00_ip4_addr_t *ip,
                                              uint16_t port)
{
  if (e_udp == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  e_udp->offset = 0;
  return w5x00_socket_begin_udp(e_udp->sockindex, ip, port);
}

/***************************************************************************//**
 * Ethernet UDP End Packet.
 ******************************************************************************/
sl_status_t w5x00_ethernet_udp_end_packet(w5x00_ethernet_udp_t *e_udp)
{
  if (e_udp == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  return w5x00_socket_send_udp(e_udp->sockindex);
}

/***************************************************************************//**
 * Ethernet UDP Write.
 ******************************************************************************/
int w5x00_ethernet_udp_write(w5x00_ethernet_udp_t *e_udp,
                             const uint8_t *buffer,
                             size_t size)
{
  if (e_udp == NULL) {
    return -1;
  }
  uint16_t bytes_written = w5x00_socket_buffer_data(  e_udp->sockindex,
                                                      e_udp->offset,
                                                      buffer,
                                                      size);
  e_udp->offset += bytes_written;
  return bytes_written;
}

/***************************************************************************//**
 * Ethernet UDP Parse Packet.
 ******************************************************************************/
int w5x00_ethernet_udp_parse_packet(w5x00_ethernet_udp_t *e_udp)
{
  if (e_udp == NULL) {
    return -1;
  }
  // discard any remaining bytes in the last packet
  while (e_udp->remaining) {
    // could this fail (loop endlessly) if remaining > 0 and recv in read fails?
    // should only occur if recv fails after telling us the data is there, lets
    // hope the w5100 always behaves :)
    w5x00_ethernet_udp_read(e_udp,
                            (uint8_t *)NULL,
                            e_udp->remaining);
  }

  if (w5x00_socket_recv_available(e_udp->sockindex) > 0) {
    //HACK - hand-parse the UDP packet using TCP recv method
    uint8_t tmpBuf[8];
    int ret=0;
    //read 8 header bytes and get IP and port from it
    ret = w5x00_socket_recv(e_udp->sockindex, tmpBuf, 8);
    if (ret > 0) {
      w5x00_ip4_addr_set_byte(&e_udp->remote_ip, 0, tmpBuf[0]);
      w5x00_ip4_addr_set_byte(&e_udp->remote_ip, 1, tmpBuf[1]);
      w5x00_ip4_addr_set_byte(&e_udp->remote_ip, 2, tmpBuf[2]);
      w5x00_ip4_addr_set_byte(&e_udp->remote_ip, 3, tmpBuf[3]);
      e_udp->remote_port = tmpBuf[4];
      e_udp->remote_port = (e_udp->remote_port << 8) + tmpBuf[5];
      e_udp->remaining = tmpBuf[6];
      e_udp->remaining = (e_udp->remaining << 8) + tmpBuf[7];

      // When we get here, any remaining bytes are the data
      ret = e_udp->remaining;
    }
    return ret;
  }
  // There aren't any packets available
  return 0;
}

/***************************************************************************//**
 * Ethernet UDP Read Byte.
 ******************************************************************************/
int w5x00_ethernet_udp_read_byte(w5x00_ethernet_udp_t *e_udp)
{
  uint8_t byte;

  if (e_udp == NULL) {
    return -1;
  }
  if ((e_udp->remaining > 0)
      && (w5x00_socket_recv(e_udp->sockindex, &byte, 1) > 0)) {
    // We read things without any problems
    e_udp->remaining--;
    return byte;
  }

  // If we get here, there's no data available
  return -1;
}

/***************************************************************************//**
 * Ethernet UDP Read To Buffer.
 ******************************************************************************/
int w5x00_ethernet_udp_read( w5x00_ethernet_udp_t *e_udp,
                             uint8_t *buffer,
                             size_t len)
{
  if (e_udp == NULL) {
    return -1;
  }
  if (e_udp->remaining > 0) {
    int got;
    if (e_udp->remaining <= len) {
      // data should fit in the buffer
      got = w5x00_socket_recv(e_udp->sockindex,
                                       buffer,
                                       e_udp->remaining);
    } else {
      // too much data for the buffer,
      // grab as much as will fit
      got = w5x00_socket_recv(e_udp->sockindex,
                                       buffer,
                                       len);
    }
    if (got > 0) {
      e_udp->remaining -= got;
      return got;
    }
  }
  // If we get here, there's no data available or recv failed
  return -1;
}

/***************************************************************************//**
 * Ethernet UDP Peek 1 byte.
 ******************************************************************************/
int w5x00_ethernet_udp_peek(w5x00_ethernet_udp_t *e_udp)
{
  if (e_udp == NULL) {
    return -1;
  }
  // Unlike recv, peek doesn't check to see if there's any data available, so we must.
  // If the user hasn't called parsePacket yet then return nothing otherwise they
  // may get the UDP header
  if (e_udp->sockindex >= W5x00_MAX_SOCK_NUM || e_udp->remaining == 0) {
    return -1;
  }
  return w5x00_socket_peek(e_udp->sockindex);
}

/***************************************************************************//**
 * Ethernet UDP Flush Tx Buffer.
 ******************************************************************************/
sl_status_t w5x00_ethernet_udp_flush(w5x00_ethernet_udp_t *e_udp)
{
  enum W5x00Chip chip;

  if (e_udp == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  chip = w5x00_get_chip();
  if (chip == W5x00_UNKNOWN || chip == W5x00_W5100) {
    return SL_STATUS_IO;
  }
  // We should wait for TX buffer to be emptied
  while(w5x00_socket_get_tx_free_size(e_udp->sockindex)
       != w5x00_get_socket_tx_max_size(e_udp->sockindex));
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Ethernet UDP Begin Multicast.
 ******************************************************************************/
sl_status_t w5x00_ethernet_udp_begin_multicast( w5x00_ethernet_udp_t *e_udp,
                                                w5x00_ip4_addr_t ip,
                                                uint16_t port)
{
  if (e_udp == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (e_udp->sockindex < W5x00_MAX_SOCK_NUM) w5x00_socket_close(e_udp->sockindex);
  e_udp->sockindex = w5x00_socket_begin_multicast(SnMR_UDP | SnMR_MULTI,
                                                  ip,
                                                  port);
  if (e_udp->sockindex >= W5x00_MAX_SOCK_NUM) {
    return 0;
  }
  e_udp->port = port;
  e_udp->remaining = 0;
  return SL_STATUS_OK;
}


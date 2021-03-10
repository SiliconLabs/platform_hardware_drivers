/***************************************************************************//**
 * @file gps.c
 * @brief GPS driver
 * @version 1.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "em_cmu.h"
#include "em_eusart.h"
#include "em_gpio.h"
#include "em_core.h"
#include "sl_status.h"

#include "gps.h"
#include "gps_config.h"

//#define GPS_NMEA_GNRMC_ID     "GNRMC"   // Only one supported
#define GPS_NMEA_START_TOK_LEN  7

typedef struct {
  volatile char *rx_buffer_ptr;       // Pointer to store rx data.
  volatile uint16_t rx_buffer_idx;    // Index to place next received character.
  uint16_t rx_buffer_size;            // Size of rx data buffer.
  volatile bool rx_is_complete;       // Indicator that a full NMEA sentence has
                                      //    been received through interrupt.
  volatile bool rx_processed;         // Indicator that a full NMEA sentence has
                                      //    been processed by application.

  volatile char *tx_buffer_ptr;       // Pointer to store tx data.
  volatile uint16_t tx_buffer_idx;    // Index of next character to transmit.
  uint16_t tx_buffer_max_size;        // Max size of the tx data buffer.
  uint16_t tx_buffer_out_size;        // Size of tx data to transmit. out <= max
  volatile bool tx_busy;              // Indicator that data is currently being
                                      //    transmitted.

  gps_data_t latest_data;             // Holds the latest gps data that was
                                      //    received and processed.
} gps_handle_t;

static gps_handle_t handle;


/***************************************************************************//**
 * @brief
 *    Initialize the EUART port
 ******************************************************************************/
static void init_euart(void)
{
  // Select LFXO for the EUART
  CMU_LFXOInit_TypeDef lfxoInit = CMU_LFXOINIT_DEFAULT;
  CMU_LFXOInit(&lfxoInit);
  CMU_ClockSelectSet(cmuClock_EM23GRPACLK, cmuSelect_LFXO);
  CMU_ClockEnable(cmuClock_EM23GRPACLK, true);

  // GPIO
  CMU_ClockEnable(cmuClock_GPIO, true);

  GPIO_PinModeSet(GPS_EUART_RX_PORT, GPS_EUART_RX_PIN, gpioModeInput, 1);
  GPIO_PinModeSet(GPS_EUART_TX_PORT, GPS_EUART_TX_PIN, gpioModePushPull, 1);

  GPIO->EUARTROUTE->TXROUTE = (GPS_EUART_TX_PORT << _GPIO_EUART_TXROUTE_PORT_SHIFT)
      | (GPS_EUART_TX_PIN << _GPIO_EUART_TXROUTE_PIN_SHIFT);
  GPIO->EUARTROUTE->RXROUTE = (GPS_EUART_RX_PORT << _GPIO_EUART_RXROUTE_PORT_SHIFT)
        | (GPS_EUART_RX_PIN << _GPIO_EUART_RXROUTE_PIN_SHIFT);

  GPIO->EUARTROUTE->ROUTEEN = GPIO_EUART_ROUTEEN_TXPEN;

  // EUART
  CMU_ClockSelectSet(cmuClock_EUART0, cmuSelect_EM23GRPACLK);
  CMU_ClockEnable(cmuClock_EUART0, true);

  EUSART_UartInit_TypeDef euartInit = EUSART_UART_INIT_DEFAULT_LF;
  euartInit.baudrate = 9600;  // <=9600 baud can only operate in EM2

  EUSART_UartInitLf(EUART0, &euartInit);

  EUSART_IntEnable(EUART0, EUSART_IEN_RXFLIEN);

  NVIC_ClearPendingIRQ(EUART0_RX_IRQn);
  NVIC_EnableIRQ(EUART0_RX_IRQn);

  NVIC_ClearPendingIRQ(EUART0_TX_IRQn);
  NVIC_EnableIRQ(EUART0_TX_IRQn);
}


/***************************************************************************//**
 * @brief
 *    Initialize the GPIO pins
 ******************************************************************************/
static void init_gpio(void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);

  GPIO_PinModeSet(GPS_ENABLE_PORT, GPS_ENABLE_PIN, gpioModePushPull, 1);
}


/***************************************************************************//**
 * @brief
 *    Formats the string into the gps data struct.
 *
 * @param[in] degmin
 *    Degrees minutes float value
 *
 * @param[in] cardinal
 *    NORTH, SOUTH, EAST, WEST
 *
 * @return
 *    Decimal degree float value
 ******************************************************************************/
static float convert_degmin_to_decdeg(float degmin, gps_cardinal_t cardinal)
{
  int32_t deg;
  float min;
  float sign = 1;

  deg = degmin / 100;   // dddmm.mmmm (extract and mask deg = ddd)
  min = (int32_t)degmin % (100);

  if(cardinal == 'S' || cardinal == 'W') {
      sign = -1;
  }

  // .d = M.m / 60
  // Decimal Degrees = Degrees + .d
  return sign * (deg + min / 60);
}


/***************************************************************************//**
 * @brief
 *    Checks if a nmea start key has been received. The first character of a
 *    nmea sentence is always '$'.
 *
 * @param[in] nmea_char
 *    Next received character
 *
 * @return
 *    True if '$', false if not
 ******************************************************************************/
static bool nmea_is_start(char nmea_char)
{
  if(nmea_char == '$') {
    return true;
  }
  return false;
}


/***************************************************************************//**
 * @brief
 *    Checks if the token in the nmea sentence is empty or not.
 *
 * @param[in] nmea_tok
 *    NMEA sentence token
 *
 * @return
 *    True if empty, false if not
 ******************************************************************************/
static bool nmea_is_empty(char *nmea_tok)
{
  if(nmea_tok == NULL) {
    return true;
  }

  if(*nmea_tok == ',') {
    return true;
  }

  return false;
}


/***************************************************************************//**
 * @brief
 *    Formats the string into the gps data struct based on the RMC NMEA ID.
 *
 * @param[in] nmea_str
 *    NMEA sentence string
 *
 * @param[out] gps
 *    Formatted data from string
 *
 * @return
 *    Error status
 ******************************************************************************/
static sl_status_t nmea_format(char *nmea_str, gps_data_t *gps)
{

  if(nmea_str == NULL || gps == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Helper variables that will be processed to convert to appropriate types.
  int32_t tmp_int = 0;
  float tmp_float = 0;
  gps_cardinal_t tmp_cardinal = NORTH;
  char *value_tok = nmea_str;

  if(strstr(value_tok, "RMC") != NULL) {
    // Grab individual variables from nmea string
    // RMC Recommended Minimum Navigation Information
    //                                                              12
    //        1         2 3       4 5        6  7   8   9      10 11|  13
    //        |         | |       | |        |  |   |   |      |  | |   |
    //$--RMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,xxxxxx,x.x,a,m,*hh<CR><LF>
    // 1) Time (UTC)
    // 2) Status, V = Navigation receiver warning
    // 3) Latitude
    // 4) N or S
    // 5) Longitude
    // 6) E or W
    // 7) Speed over ground, knots
    // 8) Track made good, degrees true
    // 9) Date, ddmmyy
    // 10) Magnetic Variation, degrees
    // 11) E or W
    // 12) Checksum

    // At this point, the RMC start id is already taken out so first token is
    // UTC Time (1)

    //--------
    // 1) UTC time token: hhmmss.sss
    value_tok = strchr(value_tok, ',') + 1; // Get next token location
    if(!nmea_is_empty(value_tok)) {
      tmp_float = atof(value_tok);

      // Change to fixed pt to avoid float err.
      tmp_int = tmp_float * 1000;

      // hhmmss[sss]
      gps->utctime.millisec = tmp_int % 1000;
      tmp_int /= 1000;    // Shift right by 3 (dec) digits

      // hhmmss
      gps->utctime.sec = tmp_int % 100;
      tmp_int /= 100;     // Shift right by 2 (dec) digits

      // hhmm
      gps->utctime.min = tmp_int % 100;
      tmp_int /= 100;     // Shift right by 2 (dec) digits

      // hh
      gps->utctime.hour = tmp_int;

    } else {
      // Epoch Time: 00:00:00 1 Jan 1970
      gps->utctime.hour = 0;
      gps->utctime.min = 0;
      gps->utctime.sec = 0;
      gps->utctime.millisec = 0;
    }

    //--------
    // 2) Status token: 'A' for data valid, 'V' for data invalid
    value_tok = strchr(value_tok, ',') + 1; // Get next token location
    gps->status = (*value_tok == 'A') ? true : false;

    //--------
    // 3,4) Latitude token: ddmm.mmmm [N or S]
    value_tok = strchr(value_tok, ',') + 1; // Get next token location
    if(!nmea_is_empty(value_tok)) {
      tmp_float = atof(value_tok);
    } else {
      tmp_float = 0;  // default
    }

    // [N or S]
    value_tok = strchr(value_tok, ',') + 1; // Get next token location
    if(!nmea_is_empty(value_tok)) {
      tmp_cardinal = (*value_tok == 'N') ? NORTH : SOUTH;
    } else {
      tmp_cardinal = NORTH; // default
    }

    gps->latitude_decdeg = convert_degmin_to_decdeg(tmp_float, tmp_cardinal);

    //--------
    // 5,6) Longitude token: ddmm.mmmm [E or W]
    value_tok = strchr(value_tok, ',') + 1; // Get next token location
    if(!nmea_is_empty(value_tok)) {
      tmp_float = atof(value_tok);
    } else {
      tmp_float = 0;  // default
    }

    // [W or W]
    value_tok = strchr(value_tok, ',') + 1; // Get next token location
    if(!nmea_is_empty(value_tok)) {
      tmp_cardinal = (*value_tok == 'E') ? EAST : WEST;
    } else {
      tmp_cardinal = EAST; // default
    }

    gps->longitude_decdeg = convert_degmin_to_decdeg(tmp_float, tmp_cardinal);

    //--------
    // 7) Speed token: float
    value_tok = strchr(value_tok, ',') + 1; // Get next token location
    if(!nmea_is_empty(value_tok)) {
      gps->speed_knots = atof(value_tok);
    } else {
      gps->speed_knots = 0;
    }

    //--------
    // 8) Course token: float
    value_tok = strchr(value_tok, ',') + 1; // Get next token location
    if(!nmea_is_empty(value_tok)) {
      gps->course_deg = atof(value_tok);
    } else {
      gps->course_deg = 0;
    }

    //--------
    // 9) UTC date token: ddmmyy
    value_tok = strchr(value_tok, ',') + 1; // Get next token location
    if(!nmea_is_empty(value_tok)) {
      tmp_int = atoi(value_tok);
      gps->utcdate.year = tmp_int % 100;
      tmp_int /= 100;
      gps->utcdate.month = tmp_int % 100;
      tmp_int /= 100;
      gps->utcdate.day = tmp_int;

    } else {
      // Epoch Time: 00:00:00 1 Jan 1970
      gps->utcdate.day = 1;
      gps->utcdate.month = 1;
      gps->utcdate.year = 70;
    }

    //--------
    // 10,11) Magnetic variation token: float
    value_tok = strchr(value_tok, ',') + 1; // Get next token location
    if(!nmea_is_empty(value_tok)) {
      gps->mag_var_deg = atof(value_tok);
    } else {
      gps->mag_var_deg = 0;
    }

    value_tok = strchr(value_tok, ',') + 1; // Get next token location
    if(!nmea_is_empty(value_tok)) {
      gps->mag_var_ew = (*value_tok == 'E') ? EAST : WEST;
    } else {
      gps->mag_var_ew = EAST; // default
    }
  }
  return SL_STATUS_OK;
}


/***************************************************************************//**
 * @brief
 *    Checks if a nmea start key has been received. The first character of a
 *    nmea sentence is always '$'.
 *
 * @param[in] nmea_char
 *    Next received character
 ******************************************************************************/
static void process_char(char nmea_char)
{
  static bool is_data_valid = false;

  if(is_data_valid && !nmea_is_start(nmea_char)) {

    // Check for overflow
    if(handle.rx_buffer_idx < handle.rx_buffer_size) {
      handle.rx_buffer_ptr[handle.rx_buffer_idx++] = nmea_char;
      handle.rx_is_complete = false;

      // Check if entire desired nmea sentence have been recorded.
      if(nmea_char == '\n') {
        handle.rx_processed = false;
        handle.rx_is_complete = true;
        handle.rx_buffer_idx = 0;
        is_data_valid = false;
      }
    }
  }

  // Each NMEA sentence starts with '$' so when that character is seen, the
  // following bytes should be recorded.
  else if(nmea_is_start(nmea_char)) {
    is_data_valid = true;
  }

}


/***************************************************************************//**
 * @brief
 *    Initializes all peripherals required to interface with GPS ORG1510 module.
 *
 * @param[in] init
 *    Initialize configuration. There are DECLARE_GPS_BUFFER and
 *    GPS_INIT_DEFAULT macro definitions that can be used.
 *
 * @return
 *    Error status
 ******************************************************************************/
sl_status_t gps_init(gps_init_t *init)
{
  // Check parameters
  if(init->rx_buffer_size < NMEA_MAX_LENGTH || init->rx_buffer_ptr == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  init_gpio();
  init_euart();

  handle.rx_buffer_ptr = init->rx_buffer_ptr;
  handle.rx_buffer_size = init->rx_buffer_size;
  handle.rx_buffer_idx = 0;
  handle.rx_is_complete = false;
  handle.rx_processed = false;

  handle.tx_buffer_ptr = init->tx_buffer_ptr;
  handle.tx_buffer_max_size = init->tx_buffer_size;
  handle.tx_buffer_out_size = 0;
  handle.tx_buffer_idx = 0;

  return SL_STATUS_OK;
}


/***************************************************************************//**
 * @brief
 *    Process the received characters when needed.
 *
 * @note
 *    Call this often in the while loop to check and process the received in the
 *    foreground. Do not call in an interrupt handler.
 *
 * @return
 *    SL_STATUS_OK                  If new data was processed.
 *    SL_STATUS_IS_WAITING          If there is no new data.
 ******************************************************************************/
sl_status_t gps_process_action(void)
{
  if(!handle.rx_processed & handle.rx_is_complete) {
    handle.rx_processed = true;
    return nmea_format((char *)handle.rx_buffer_ptr, &handle.latest_data);
  }

  return SL_STATUS_IS_WAITING;
}


/***************************************************************************//**
 * @brief
 *    Get the most recent GPS data.
 *
 * @note
 *    The GPS data is pulled from the Recommended Minimum Navigation Information
 *    (RMC) NMEA sentence.
 *
 * @param[out] data
 *    GPS struct data
 *
 * @return
 *    Error status
 ******************************************************************************/
sl_status_t gps_get_data(gps_data_t *data)
{
  if(data == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  memcpy(data, &handle.latest_data, sizeof(gps_data_t));
  return SL_STATUS_OK;
}


/***************************************************************************//**
 * @brief
 *    Send NMEA command to gps module.
 *
 * @note
 *    Input is not modified. The string is sent to the GPS module as is.
 *
 * @param[in] cmd
 *    String NMEA command.
 *    Can not be more than 80 characters long.
 *
 * @return
 *    SL_STATUS_OK      If string is accepted.
 *    SL_STATUS_BUSY    If string was rejected because previous data is still
 *                      being transmitted.
 ******************************************************************************/
sl_status_t gps_send_cmd(char *cmd)
{
  if(cmd == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // tx buffer was not initialized.
  if(handle.tx_buffer_ptr == NULL) {
    return SL_STATUS_INVALID_HANDLE;
  }


  if(!handle.tx_busy) {
    // Check if cmd is less than the max tx buffer size that was allocated
    // during initialization
    uint16_t out_size = handle.tx_buffer_max_size;
    uint16_t cmd_size = strlen(cmd);
    if(out_size > cmd_size) {
      out_size = cmd_size;
    }

    memcpy((char *)handle.tx_buffer_ptr, cmd, out_size);
    handle.tx_buffer_out_size = out_size;

    handle.tx_busy = true;

    EUSART_IntEnable(EUART0, EUSART_IEN_TXFLIEN);

    return SL_STATUS_OK;
  }

  return SL_STATUS_BUSY;
}


/***************************************************************************//**
 * @brief
 *    Enable or disable to LDO power. Useful to power cycle and reset the GPS
 *    module or save power when not in use.
 *
 * @param[in] en
 *    True or false to enable or disable, respectively.
 *
 * @return
 *    Error status
 ******************************************************************************/
sl_status_t gps_enable(bool en)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  if(en) {
    GPIO_PinOutSet(GPS_ENABLE_PORT, GPS_ENABLE_PIN);
  } else {
    GPIO_PinOutClear(GPS_ENABLE_PORT, GPS_ENABLE_PIN);
  }
  CORE_EXIT_CRITICAL();
  return SL_STATUS_OK;
}


/***************************************************************************//**
 * @brief
 *    EUART0 RX Interrupt Handler
 ******************************************************************************/
void EUART0_RX_IRQHandler(void)
{
  uint32_t flags = EUSART_IntGet(EUART0);
  if(flags & EUSART_IF_RXFLIF) {
      process_char((char)(EUART0->RXDATA));
  }

  EUSART_IntClear(EUART0, flags);
}


/**************************************************************************//**
 * @brief
 *    The EUART0 transmit interrupt outputs characters.
 *****************************************************************************/
void EUART0_TX_IRQHandler(void)
{
  // Send a previously received character
  if(handle.tx_buffer_idx < handle.tx_buffer_out_size) {
    EUART0->TXDATA = handle.tx_buffer_ptr[handle.tx_buffer_idx++];
  } else
  /*
   * Need to disable the transmit buffer level interrupt in this IRQ
   * handler when done or it will immediately trigger again upon exit
   * even though there is no data left to send.
   */
  {
    EUSART_IntDisable(EUART0, EUSART_IEN_TXFLIEN); // Disable TX FIFO Level Interrupt
    handle.tx_busy = false;
  }

  // Clear the requesting interrupt before exiting the handler
  EUSART_IntClear(EUART0, EUSART_IF_TXFLIF);
}


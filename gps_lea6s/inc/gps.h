/***************************************************************************//**
 * @file gps.h
 * @brief GPS driver prototypes.
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

#ifndef GPS_H_
#define GPS_H_

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup GPS
 * @{
 *
 * @brief
 *  The implementation of GPS receiver driver using the GPS LEA-6S MikroE Click
 *  Board.
 *
 *  The target application of the GPS module would be outdoor asset tracking.
 *  The location of assets can be tracked using the GPS module if the asset is
 *  misplaced or stolen.
 ******************************************************************************/

// Enumeration of basic 4-point cardinal directions
typedef enum {
  NORTH,
  SOUTH,
  EAST,
  WEST
} gps_cardinal_t;

// Container for the UTC time
typedef struct {
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint16_t millisec;
} gps_utctime_t;

// Container for the UTC data
typedef struct {
  uint8_t day;
  uint8_t month;
  uint8_t year;
} gps_utcdate_t;

// Container for all the gps data that is parsed from the RMC NMEA sentence.
typedef struct {
  gps_utctime_t utctime;      ///< UTC Time
  gps_utcdate_t utcdate;      ///< UTC Date
  bool status;                ///< Status of data (valid=1, warning=0)
  float latitude_decdeg;      ///< Latitude in Decimal Degrees
  float longitude_decdeg;     ///< Longitude in Decimal Degrees
  float speed_knots;          ///< Speed in knots
  float course_deg;           ///< Course in degrees
  float mag_var_deg;          ///< Magnetic variation in degrees
  gps_cardinal_t mag_var_ew;  ///< Magnetic variation East or West
} gps_data_t;

// GPS initialization data
typedef struct {
  char *rx_buffer_ptr;        ///< Pointer to RX buffer used by EUART Int.
  uint16_t rx_buffer_size;    ///< Size of RX buffer
  char *tx_buffer_ptr;        ///< Pointer to TX buffer used by EUART Int.
  uint16_t tx_buffer_size;    ///> Size of TX buffer
} gps_init_t;


/***************************************************************************//**
 * @brief
 *    Excluding the <CR><LF> characters, a single NMEA sentence will never be
 *    more than 80 characters long.
 ******************************************************************************/
#define NMEA_MAX_LENGTH       80


/***************************************************************************//**
 * @brief
 *    Declare a default global buffer for the GPS.
 *
 * @note
 *    This should be called as a global if GPS_INIT_DEFAULT is used as the
 *    default init struct data.
 *
 * @note
 *    Excluding the <CR><LF> characters, the buffer length should be at least
 *    80 characters long.
 ******************************************************************************/
#define GPS_DECLARE_RX_BUFFER   static char gps_rx_buffer[NMEA_MAX_LENGTH];
#define GPS_DECLARE_TX_BUFFER   static char gps_tx_buffer[NMEA_MAX_LENGTH];


/***************************************************************************//**
 * @brief
 *    Default init struct for the gps_init function.
 *
 * @note
 *    Excluding the <CR><LF> characters, the buffer length should be at least
 *    80 characters long.
 *
 * @pre
 *    DECLARE_GPS_BUFFER should already be allocated as a global variable.
 ******************************************************************************/
#define GPS_INIT_DEFAULT                                                      \
  {                                                                           \
    gps_rx_buffer,         /* buffer ptr for uart receive operations. */      \
    NMEA_MAX_LENGTH,       /* buffer size */                                  \
    gps_tx_buffer,         /* buffer ptr for uart transmit operations. */     \
    NMEA_MAX_LENGTH        /* buffer size */                                  \
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
sl_status_t gps_init(gps_init_t *init);


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
sl_status_t gps_process_action(void);


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
sl_status_t gps_get_data(gps_data_t *data);


/***************************************************************************//**
 * @brief
 *    Send NMEA command to gps module.
 *
 * @note
 *    Input is not modified. The string is sent to the GPS module as is.
 *
 * @param[in] cmd
 *    String NMEA command
 *
 * @return
 *    SL_STATUS_OK      If string is accepted.
 *    SL_STATUS_BUSY    If string was rejected because previous data is still
 *                      being transmitted.
 ******************************************************************************/
sl_status_t gps_send_cmd(char *cmd);


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
sl_status_t gps_enable(bool en);


/** @} (end addtogroup GPS) */

#ifdef __cplusplus
}
#endif

#endif /* GPS_H_ */

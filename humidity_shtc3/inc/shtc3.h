/***************************************************************************//**
 * @file shtc3.h
 * @brief Header file of shtc3 humidity driver
 * @version 0.1.0
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The license of this software is Silicon Laboratories Inc.
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

#ifndef SHTC3_H_
#define SHTC3_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include "sl_status.h"
#include "sl_udelay.h"
#include "sl_i2cspm.h"

/***************************************************************************//**
 * @addtogroup shtc3 - Humidity and Temperature Sensor
 * @brief Silicon Labs SHTC3 Humidity and Temperature Sensor
 * I2C driver.
 *
 *    @n @section shtc3_example SHTC3 example code
 *
 *    Basic example for humidity and temperature measurement using
 *    an SHTC3 sensor: @n @n
 *    @verbatim
 *
 #include "sl_i2cspm_instances.h"
 #include "shtc3.h"
 *
 *    static   shtc3_sensor_data_t   shtc3_data;
 *    int main( void )
 *    {
 *
 *    ...
 *
 *    shtc3_init(sl_i2cspm_qwiic);
 *    shtc3_wake();
 *    if (sl_shtc3_get_temp_rh(SHTC3_NORMAL_POWER_MODE,
 *                             SHTC3_RH_FIRST,
 *                             false,
 *                             true,
 *                             &shtc3_data)
 *     app_log("Temp is: %0.2f Humidity is: %0.2f\n",
 *              shtc3_data.rh_t_data.calculated_temp,
 *              shtc3_data.rh_t_data.calculated_rh);
 *
 *    ...
 *
 *    } @endverbatim
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @name Sensor Defines
 * @{
 ******************************************************************************/

#define SHTC3_MAJOR_VERION             0
#define SHTC3_MINOR_VERION             1
#define SHTC3_BUILD_VERION             0
#define SHTC3_REVISION                 1

/** Device for shtc3 */
#define SHTC3_DEFAULT_ADDRESS          0x70

/** Device for shtc3 */
#define SHTC3_ID                       0x887

/** P(x) = x^8 + x^5 + x^4 + 1 = 100110001 */
#define CRC_POLYNOMIAL                 0x131

/** @}  */

/// @brief SHTC3 commands
typedef enum {
  ///< command: read ID register
  SHTC3_READ_ID = 0xEFC8,
  ///< soft reset
  SHTC3_SOFT_RESET = 0x805D,
  ///< sleep
  SHTC3_SLEEP = 0xB098,
  ///< wakeup
  SHTC3_WAKEUP = 0x3517,
  ///< meas. read T first, clock stretching enabled, normal power mode
  SHTC3_MEAS_T_RH_CLOCKSTR_NPM = 0x7CA2,
  ///< meas. read RH first, clock stretching enabled, normal power mode
  SHTC3_MEAS_RH_T_CLOCKSTR_NPM = 0x5C24,
  ///< meas. read T first, clock stretching enabled, low power mode
  SHTC3_MEAS_T_RH_CLOCKSTR_LPM = 0x6458,
  ///< meas. read RH first, clock stretching enabled, low power mode
  SHTC3_MEAS_RH_T_CLOCKSTR_LPM = 0x44DE,

  ///< meas. read T first, clock stretching disabled, normal power mode
  SHTC3_MEAS_T_RH_NOCLOCKSTR_NPM = 0x7866,
  ///< meas. read RH first, clock stretching disabled, normal power mode
  SHTC3_MEAS_RH_T_NOCLOCKSTR_NPM = 0x58E0,
  ///< meas. read T first, clock stretching disabled, low power mode
  SHTC3_MEAS_T_RH_NOCLOCKSTR_LPM = 0x609C,
  ///< meas. read RH first, clock stretching disabled, low power mode
  SHTC3_MEAS_RH_T_NOCLOCKSTR_LPM = 0x401A
} shtc3_commands_t;

/// @brief power modes
typedef enum {
  SHTC3_LOW_POWER_MODE = 0,
  SHTC3_NORMAL_POWER_MODE
} shtc3_power_mode_t;

typedef enum {
  SHTC3_TEMP_FIRST,
  SHTC3_RH_FIRST,
} shtc3_data_order_t; // sensor data return order

/***************************************************************************//**
 * @brief
 *    Typedef for specifying the software version of the core driver.
 ******************************************************************************/
typedef struct
{
  uint8_t major;       /*!< major number */
  uint8_t minor;       /*!< minor number */
  uint8_t build;       /*!< build number */
  uint32_t revision;   /*!< revision number */
} shtc3_core_version_t;

/***************************************************************************//**
 * @brief
 *    Typedef for storing the SHTC3 sensor data.
 ******************************************************************************/
typedef struct {
  uint16_t raw_temp;         /*!< Temperature data */
  uint16_t raw_rh;           /*!< Relative humidity data */
} shtc3_raw_data_t;

/***************************************************************************//**
 * @brief
 *    Typedef for specifying the calculated result data.
 ******************************************************************************/
typedef struct
{
  float calculated_temp;         /*!< Temperature data */
  float calculated_rh;           /*!< Relative humidity data */
} shtc3_calc_data_t;

/***************************************************************************//**
 * @brief
 *    Structure to store the sensor data
 ******************************************************************************/
typedef struct shtc3_sensor_data{
  shtc3_raw_data_t  rh_t_data_raw;    /**< Relative huidity and temperature; raw
                                       *   data    */
  shtc3_calc_data_t rh_t_data;        /**< Relative huidity and temperature
                                       *                */
} shtc3_sensor_data_t;

/***************************************************************************//**
 * Local Variables
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Get the software version of the driver.
 *
 * @param[in] core_version
 *   Pointer to struct for specifying the software version of the core driver.
 *
 * @return
 *   none
 ******************************************************************************/
void get_version(shtc3_core_version_t *core_version);

/***************************************************************************//**
 * @brief
 *   Initialize the SHTC3 driver with the values provided in the
 *   shtc3_config.h file.
 *
 * @param[in] i2cspm
 *   The I2C peripheral to use.
 *
 * @details
 *   This function assigns the I2C instance used to communicate with the device
 *   and puts the SHTC3 device into sleep mode.
 *
 * @note
 *   The SHTC3 device is put in sleep mode initially, until a measurement is
 *   initialed, and after a measurement completes.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_ALREADY_INITIALIZED the function has previously
 *     been called.
 ******************************************************************************/
sl_status_t shtc3_init(sl_i2cspm_t *i2cspm);

/***************************************************************************//**
 * @brief
 *   De-initialize the SHTC3 driver.
 *
 * @param[in]
 *
 * @details
 *   De-initialization of the SHTC3 resets the device.
 *
 * @note
 *   The purpose of de-initialization is to shutdown the SHTC3 as part
 *   of a complete shutdown sequence for an EFM32/EFR32-based system.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_NOT_INITIALIZED if the driver is not initialized.
 ******************************************************************************/
sl_status_t shtc3_deinit(void);

/***************************************************************************//**
 * @brief
 *    Set power mode for SHTC3
 *
 * @param[in] _mode_power
 *    Mode power configuration structure
 *
 * @return
 *    None
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   This function puts the SHTC3 device in sleep mode.
 *
 * @param[in] none
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t shtc3_sleep(void);

/***************************************************************************//**
 * @brief
 *   This function wakes the SHTC3 device from sleep mode.
 *
 * @param[in] none
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t shtc3_wake(void);

/***************************************************************************//**
 * @brief
 *   This function initiates a software reset of the SHTC3 device.
 *
 * @param[in] none
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t shtc3_reset(void);

/***************************************************************************//**
 * @brief
 *   This function wakes the SHTC3 device from sleep mode.
 *
 * @param[in] shtc3_id
 *   Pointer to variable to hold 16-bit unique ID of SHTC3 device
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t shtc3_get_id(uint16_t *shtc3_id);

/***************************************************************************//**
 * @brief
 *   Initiates a measurement cycle and reads temp and RH data from the SHTC3.
 *   SHTC3 power mode, data return order, I2C clock stretching, and data CRC
 *   checking are all selectable.  This function receives raw data over I2C
 *   as well as calculates temperature (deg C) and RH (%) from the raw data.
 *   Raw and calculated values are stored in the data structure.
 *
 * @param[in] pwr_mode
 *   Power mode selection for SHTC3 device
 *
 * @param[in] order
 *   Return order of temp and RH data from the SHTC3
 *
 * @param[in] stretch
 *   Use I2C clock stretching (1) or not (0)
 *
 * @param[in] checkCRC
 *   Check temp and RH measurement value CRCs (1) or not (0)
 *
 * @param[out] data
 *   pointer to shtc3_sensor_data_t structure that holds raw and calcultaed temp
 *   and RH values
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t sl_shtc3_get_temp_rh(shtc3_power_mode_t pwr_mode,
                                 shtc3_data_order_t order,
                                 bool stretch,
                                 bool check_crc,
                                 shtc3_sensor_data_t *data);

/***************************************************************************//**
 * @brief
 *   Initiates a measurement cycle and reads temp and RH data from the SHTC3.
 *   SHTC3 power mode, data return order, I2C clock stretching, and data CRC
 *   checking are all selectable.  This function receives raw data over I2C
 *   as well as calculates temperature (deg C) and RH (%) from the raw data.
 *   Raw and calculated values are stored in the data structure.
 *   This function is a non-blocking function, so instead of waiting for the
 *   sensor to finish reading the temperature as well as humidity, this function
 *   will return SL_STATUS_BUSY when the sensor has not finished measuring.
 *
 * @param[in] pwr_mode
 *   Power mode selection for SHTC3 device
 *
 * @param[in] order
 *   Return order of temp and RH data from the SHTC3
 *
 * @param[in] checkCRC
 *   Check temp and RH measurement value CRCs (1) or not (0)
 *
 * @param[out] data
 *   pointer to shtc3_sensor_data_t structure that holds raw and calcultaed temp
 *   and RH values
 *
 * @return
 *   SL_STATUS_OK     if there are no errors
 *   SL_STATUS_BUSY   if the sensor is busy measuring temperature
 ******************************************************************************/
sl_status_t sl_shtc3_get_temp_rh_non_blocking(shtc3_power_mode_t pwr_mode,
                                              shtc3_data_order_t order,
                                              bool check_crc,
                                              shtc3_sensor_data_t *data);

/***************************************************************************//**
 * @brief
 *   Initiates a measurement cycle and reads only temp data from the SHTC3.
 *   SHTC3 power mode, I2C clock stretching, and data CRC
 *   checking are all selectable.  This function receives raw data over I2C
 *   as well as calculates temperature (deg C) from  the raw data.
 *   Raw and calculated values are stored in the data structure.
 *
 * @param[in] pwr_mode
 *   Power mode selection for SHTC3 device
 *
 * @param[in] stretch
 *   Use I2C clock stretching (1) or not (0)
 *
 * @param[in] checkCRC
 *   Check temp and RH measurement value CRCs (1) or not (0)
 *
 * @param[out] data
 *   pointer to shtc3_sensor_data_t stucture that holds raw and calcultaed temp
 *   and RH values
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t sl_shtc3_get_temp(shtc3_power_mode_t pwr_mode,
                              bool stretch,
                              bool check_crc,
                              shtc3_sensor_data_t *data);

/***************************************************************************//**
 * @brief
 *   Initiates a measurement cycle and reads only temp data from the SHTC3.
 *   SHTC3 power mode, I2C clock stretching, and data CRC
 *   checking are all selectable.  This function receives raw data over I2C
 *   as well as calculates temperature (deg C) from  the raw data.
 *   Raw and calculated values are stored in the data structure.
 *   This function is a non-blocking function, so instead of waiting for the
 *   sensor to finish reading the temperature , this function will return
 *   SL_STATUS_BUSY when the sensor has not finished measuring.
 *
 * @param[in] pwr_mode
 *   Power mode selection for SHTC3 device
 *
 * @param[in] stretch
 *   Use I2C clock stretching (1) or not (0)
 *
 * @param[in] checkCRC
 *   Check temp and RH measurement value CRCs (1) or not (0)
 *
 * @param[out] data
 *   pointer to shtc3_sensor_data_t stucture that holds raw and calcultaed temp
 *   and RH values
 *
 * @return
 *   SL_STATUS_OK     if there are no errors
 *   SL_STATUS_BUSY   if the sensor is busy measuring temperature
 ******************************************************************************/
sl_status_t sl_shtc3_get_temp_non_blocking(shtc3_power_mode_t pwr_mode,
                                           bool check_crc,
                                           shtc3_sensor_data_t *data);

/***************************************************************************//**
 * @brief
 *   Initiates a measurement cycle and reads only RH data from the SHTC3.
 *   SHTC3 power mode, I2C clock stretching, and data CRC
 *   checking are all selectable.  This function receives raw data over I2C
 *   as well as calculates RH (%) from the raw data.
 *   Raw and calculated values are stored in the data structure.
 *
 * @param[out] pwr_mode
 *   Power mode selection for SHTC3 device
 *
 * @param[out] stretch
 *   Use I2C clock stretching (1) or not (0)
 *
 * @param[out] checkCRC
 *   Check temp and RH measurement value CRCs (1) or not (0)
 *
 * @param[out] data
 *   pointer to shtc3_sensor_data_t stucture that holds raw and calcultaed temp
 *   and RH values
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t sl_shtc3_get_rh(shtc3_power_mode_t pwr_mode,
                            bool stretch,
                            bool check_crc,
                            shtc3_sensor_data_t *data);

/***************************************************************************//**
 * @brief
 *   Initiates a measurement cycle and reads only RH data from the SHTC3.
 *   SHTC3 power mode, and data CRC checking are all selectable. This function
 *   receives raw data over I2C as well as calculates RH (%) from the raw data.
 *   Raw and calculated values are stored in the data structure.
 *   This function is a non-blocking function, so instead of waiting for the
 *   sensor to finish reading the humidity, this function will return
 *   SL_STATUS_BUSY when the sensor has not finished measuring.
 *
 * @param[in] pwr_mode
 *   Power mode selection for SHTC3 device
 *
 * @param[in] checkCRC
 *   Check temp and RH measurement value CRCs (1) or not (0)
 *
 * @param[out] data
 *   pointer to shtc3_sensor_data_t stucture that holds raw and calcultaed temp
 *   and RH values
 *
 * @return
 *   SL_STATUS_OK     if there are no errors
 *   SL_STATUS_BUSY   if the sensor is busy measuring temperature
 ******************************************************************************/
sl_status_t sl_shtc3_get_rh_non_blocking(shtc3_power_mode_t pwr_mode,
                                         bool check_crc,
                                         shtc3_sensor_data_t *data);

/***************************************************************************//**
 * @brief
 *    Uses CRC-8 calculation to verify the 8-bit CRC value of a
 *    16-bit data value.
 *
 * @param[in] data
 *    Raw sensor data (RH or temp)
 *
 * @param[in] crc
 *    8-bit CRC value returned with data value
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *   @li @ref SL_STATUS_FAIL on failure.
 ******************************************************************************/
sl_status_t shtc3_verify_crc(uint16_t data, uint8_t crc);

/***************************************************************************//**
 * @brief
 *    Calculates temperature (deg C) from raw sensor value.
 *
 * @param[in] data
 *    Raw sensor data (temp)
 *
 * @return
 *   Float temperature value in degrees Celcius
 ******************************************************************************/
float shtc3_calculate_temp_from_raw_data(uint16_t data);

/***************************************************************************//**
 * @brief
 *    Calculates RH (%) from raw sensor value.
 *
 * @param[in] data
 *    Raw sensor data (RH)
 *
 * @return
 *   Float RH value in percent (%)
 ******************************************************************************/
float shtc3_calculate_rh_from_raw_data(uint16_t data);

#ifdef __cplusplus
}
#endif

/**@}* (shtc3) */

#endif /* SHTC3_H_ */

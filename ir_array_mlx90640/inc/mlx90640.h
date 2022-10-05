/***************************************************************************//**
 * @file mlx90640.h
 *******************************************************************************
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 *
 * EVALUATION QUALITY
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 *
 ******************************************************************************/

#ifndef MLX90640_MLX90640_H_
#define MLX90640_MLX90640_H_

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "sl_i2cspm.h"
#include "sl_status.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define MLX90640_NUM_OF_PIXELS                  768         // The MLX90640 has 32x24 pixels
#define MLX90640_DEFAULT_I2C_ADDR               0x33        // default I2C address
#define SCALEALPHA                              0.000001    // Default calibration value
#define MLX90640_CONFIG_EMISSIVITY              0.95        // Default calibration value
#define TA_SHIFT                                8           // Default shift for MLX90640 in open air
//#define MLX90640_DRIVER_DEBUG_PRINT_ENABLED     1           // Enables app_log() to print debug & status messages to terminal


/***************************************************************************//**
 * Typedef for the parameter structure of MLX90640
 ******************************************************************************/
typedef struct
{
  int16_t kVdd;
  int16_t vdd25;
  float KvPTAT;
  float KtPTAT;
  uint16_t vPTAT25;
  float alphaPTAT;
  int16_t gainEE;
  float tgc;
  float cpKv;
  float cpKta;
  uint8_t resolutionEE;
  uint8_t calibrationModeEE;
  float KsTa;
  float ksTo[5];
  int16_t ct[5];
  uint16_t alpha[768];
  uint8_t alphaScale;
  int16_t offset[768];
  int8_t kta[768];
  uint8_t ktaScale;
  int8_t kv[768];
  uint8_t kvScale;
  float cpAlpha[2];
  int16_t cpOffset[2];
  float ilChessC[3];
  uint16_t brokenPixels[5];
  uint16_t outlierPixels[5];
} paramsMLX90640;

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief
 * Initializes I2C, Reads out the EEPROM contents, Parses the parameters for further usage
 *
 * @param[in] i2cspm_instace - Pointer to the I2CSPM instance
 ******************************************************************************/
sl_status_t mlx90640_init(sl_i2cspm_t *i2cspm_instance, uint8_t i2c_addr);

/***************************************************************************//**
 * @brief
 * Requests and provides an array of temperatures for all 768 pixels as float values.
 *
 * @param[out] pixel_array - Pointer to an array to store the image as a temperature array of 768 pixels
 ******************************************************************************/
sl_status_t mlx90640_get_image_array(float *pixel_array);

/***************************************************************************//**
 * Change slave address to the value of "new_addr"
 ******************************************************************************/
sl_status_t mlx90640_set_slave_addr(sl_i2cspm_t *i2cspm_instance, uint8_t new_addr);

/***************************************************************************//**
 * Changes which I2C bus and slave address does the driver use
 ******************************************************************************/
sl_status_t mlx90640_change_devices(sl_i2cspm_t *i2cspm_instance, uint8_t new_addr);

/***************************************************************************//**
 * @brief
 * Requests and stores EEPROM content in the given array
 *
 * @param[out] eeData - array to store the EEPROM data in
 ******************************************************************************/
sl_status_t mlx90640_dump_EE(uint16_t *ee_data);

/***************************************************************************//**
 * @brief
 * Waits for a new data to be available with a given slave address
 ******************************************************************************/
sl_status_t mlx90640_synch_frame(void);

/***************************************************************************//**
 * @brief
 * Uses the global reset command described in the I2C standard to trigger a measurement.
 * !! Note that this function will reset all devices on the same I2C bus that support this command. !!
 ******************************************************************************/
sl_status_t mlx90640_trigger_measurement(void);

/***************************************************************************//**
 * @brief
 * Reads all the necessary frame data from the device.
 *
 * @param[out] frameData - array in which to store the frame data
******************************************************************************/
sl_status_t mlx90640_get_frame_data(uint16_t *frame_data);

/***************************************************************************//**
 * @brief
 * Extracts the parameters from a given EEPROM data array and stores values
 *
 * @param[in]  eeData   - dumped EEPROM data
 * @param[out] mlx90640 - parsed parameters will be stored in this array
 *
******************************************************************************/
sl_status_t mlx90640_extract_parameters(uint16_t *ee_data, paramsMLX90640 *mlx90640);

/***************************************************************************//**
 * @brief
 * Provides the current VDD of the device
 *
 * @param[in] frameData – pointer to the MLX90640 frame data that is already acquired
 * @param[in] params – pointer to the MCU memory location where the already extracted parameters are stored
 * @param[out] vdd - current VDD value of the device
******************************************************************************/
sl_status_t mlx90640_get_vdd(uint16_t *frameData, const paramsMLX90640 *params, float *vdd);

/***************************************************************************//**
 * @brief
 * Provides the current Ta of the device
 *
 * @param[in] frameData – pointer to the MLX90640 frame data that is already acquired
 * @param[in] params – pointer to the MCU memory location where the already extracted parameters are stored
 * @param[out] ta - current TA value of the device
******************************************************************************/
sl_status_t mlx90640_get_ta(uint16_t *frameData, const paramsMLX90640 *params, float *ta);

/***************************************************************************//**
 * @brief
 * Calculates values for all 768 pixels - not absolute temperature!
 *
 * @param[in] frameData – pointer to the MLX90640 frame data that is already acquired
 * @param[in] params – pointer to the MCU memory location where the already extracted parameters are stored
 * @param[out] result - pointer to the MCU memory location where the user wants the object temperatures data to be stored
******************************************************************************/
sl_status_t mlx90640_get_image(uint16_t *frameData, const paramsMLX90640 *params, float *result);

/***************************************************************************//**
 * @brief
 * Calculates the object temperatures for all 768 pixel.
 *
 * @param[in] frameData – pointer to the MLX90640 frame data that is already acquired
 * @param[in] params – pointer to the MCU memory location where the already extracted parameters are stored
 * @param[in] emissivity – emissivity defined by the user. The emissivity is a property of the measured object
 * @param[in] tr - reflected temperature defined by the user
 * @param[out] result – pointer to the MCU memory location where the user wants the object temperatures data to be stored
******************************************************************************/
sl_status_t mlx90640_calculate_to(uint16_t *frameData, const paramsMLX90640 *params, float emissivity, float tr, float *result);

/***************************************************************************//**
 * @brief
 * Writes the desired resolution value in order to change the current resolution
 *
 * @param[in] resolution - The resolution to be set
******************************************************************************/
sl_status_t mlx90640_set_resolution(uint8_t resolution);

/***************************************************************************//**
 * @brief
 * Provides the current resolution.
 *
 * @param[out] currentResolution – pointer to a memory location where the user wants to store the result
******************************************************************************/
sl_status_t mlx90640_get_cur_resolution(uint16_t *currentResolution);

/***************************************************************************//**
 * @brief
 * Writes the desired resolution value in order to change the current refresh rate
 *
 * @param[in] refreshRate - The refresh rate to be set
******************************************************************************/
sl_status_t mlx90640_SetRefreshRate(uint8_t refreshRate);

/***************************************************************************//**
 * @brief
 * Provides the current refresh rate.
 *
 * @param[out] current_rate – pointer to a memory location where the user wants to store the result
******************************************************************************/
sl_status_t mlx90640_get_refresh_rate(uint16_t *current_rate);

/***************************************************************************//**
 * @brief
 * Provides the sub-page for a selected frame data
 *
 * @param[in] frameData – pointer to an array of frame data
 * @param[out] sub_page_number – pointer to a memory location where the user wants to store the result
******************************************************************************/
sl_status_t mlx90640_get_sub_page_number(uint16_t *frameData, uint16_t *sub_page_number);

/***************************************************************************//**
 * @brief
 * Provides the current working mode.
 *
 * @param[out] cur_mode – pointer to a memory location where the user wants to store the result
******************************************************************************/
sl_status_t mlx90640_get_cur_mode(uint8_t *cur_mode);

/***************************************************************************//**
 * @brief
 * Sets device to interleaved mode
******************************************************************************/
sl_status_t mlx90640_set_interleaved_mode(void);

/***************************************************************************//**
 * @brief
 * Sets device to chess mode
******************************************************************************/
sl_status_t mlx90640_set_chess_mode(void);

/***************************************************************************//**
 * @brief
 * Corrects the values of the broken pixels and/or the outlier pixels.
 *
 * @param[in] pixels - pointer to the array that contains the pixels to be corrected.
 * @param[in] to – pointer to the object temperature values array.
 * @param[in] mode – the current working mode of the MLX90640 device
 * @param[in] params – pointer to the MCU memory location where the already extracted parameters are stored
******************************************************************************/
sl_status_t mlx90640_bad_pixels_correction(uint16_t *pixels, float *to, int mode, paramsMLX90640 *params);


#endif /* MLX90640_MLX90640_H_ */

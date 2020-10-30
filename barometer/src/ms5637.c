/***************************************************************************//**
* @file ms5637.c
* @brief MS5637 sensor IC driver source
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
* # Evaluation Quality
* This code has been minimally tested to ensure that it builds and is suitable
* as a demonstration for evaluation purposes only. This code will be maintained
* at the sole discretion of Silicon Labs.
******************************************************************************/
#include "barometer.h"
#include "sl_sleeptimer.h"

#define MS5637_ADDR                             0x76

#define MS5637_RESET_COMMAND                    0x1E
#define MS5637_READ_ADC_COMMAND                 0x00
#define MS5637_START_PRESSURE_CONVERSION        0x40
#define MS5637_START_TEMPERATURE_CONVERSION     0x50

#define MS5637_COEFF_BASE_ADDRESS               0xA0
#define MS5637_CRC_INDEX                        0
#define MS5637_PRESSURE_SENSITIVITY_INDEX       1
#define MS5637_PRESSURE_OFFSET_INDEX            2
#define MS5637_TEMP_COEFF_SENSITIVITY_INDEX     3
#define MS5637_TEMP_COEFF_OFFSET_INDEX          4
#define MS5637_REFERENCE_TEMPERATURE_INDEX      5
#define MS5637_TEMP_COEFF_OF_TEMPERATURE_INDEX  6
#define MS5637_COEFFICIENT_COUNT                7

#define MS5637_ADC_DATA_SIZE                    3

/// MS5637 Over-sample rate
typedef enum {
  MS5673_OSR_256,
  MS5673_OSR_512,
  MS5673_OSR_1024,
  MS5673_OSR_2048,
  MS5673_OSR_4096,
  MS5673_OSR_8192,
  MS5673_OSR_INVALID
} ms5637_osr_t;

// Measurement time depends on the selected over-sampling value
const uint8_t conversion_time[6] = {1, // OSR = 256
                  2, // OSR = 512
                  3, // OSR = 1024
                  5, // OSR = 2048
                  9, // OSR = 4096
                  17 // OSR = 8192
};

// Configuration structure for the ms5637 sensor
static barometer_init_t ms5637;

// Factory calibrated coefficients are saved for compensation
static uint16_t calibration_coeffs[MS5637_COEFFICIENT_COUNT + 1];

// Helper variables for the non-blocking sensor read
static uint32_t raw_temperature, raw_pressure;
static float temperature, pressure;
static barometer_states_t barometer_state = BAROMETER_STATE_UNINITIALIZED;
static sl_sleeptimer_timer_handle_t timer;

// The user callback which returns the sensor value in case of non-blocking sensor read
static void (*callback_user)(float);

// Local prototypes
static sl_status_t ms5637_sensor_read(uint8_t *rx_buff, uint8_t num_bytes);
static sl_status_t ms5637_sensor_write(uint8_t cmd);
static sl_status_t ms5637_read_coeffs();
static sl_status_t ms5637_crc4();
// Local callback function for the non-blocking sensor read function
void timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data);

/*******************************************************************************
 *   Initalize the MS5673 sensor
 ******************************************************************************/
sl_status_t barometer_init(barometer_init_t* init)
{
    if(barometer_state != BAROMETER_STATE_UNINITIALIZED){
      return SL_STATUS_INVALID_STATE;
    }

    ms5637.I2C_sensor = init->I2C_sensor;
    ms5637.I2C_address = init->I2C_address;
    ms5637.oversample_rate = init->oversample_rate;

    I2CSPM_Init(&init->I2C_sensor);

    barometer_reset();
    sl_sleeptimer_init();
    barometer_state = BAROMETER_STATE_STANDBY;

    return ms5637_read_coeffs();
}

/**************************************************************************//**
 *  Resets the MS5637 sensor.
 *****************************************************************************/
sl_status_t barometer_reset()
{
  if(ms5637_sensor_write(MS5637_RESET_COMMAND) == i2cTransferDone)
    return SL_STATUS_OK;

  return SL_STATUS_FAIL;
}

/**************************************************************************//**
 *  Configures the sensor with the given oversample rate.
 *****************************************************************************/
void barometer_config(ms5637_osr_t osr)
{
  ms5637.oversample_rate = osr;
}

/**************************************************************************//**
 *  Start the measurement process and waits for the results with sleeptimer.
 *****************************************************************************/
float barometer_get_pressure_blocking()
{
  uint32_t raw_temp, raw_press;
  float press, temp;

  barometer_start_conversion(BAROMETER_TEMPERATURE);
  sl_sleeptimer_delay_millisecond(barometer_get_conversion_time_in_millis());

  raw_temp = barometer_read_raw_conversion();
  barometer_start_conversion(BAROMETER_PRESSURE);
  sl_sleeptimer_delay_millisecond(barometer_get_conversion_time_in_millis());

  raw_press = barometer_read_raw_conversion();
  barometer_calculate(raw_temp, raw_press, &temp, &press);
  return press;
}

/**************************************************************************//**
 *  Start the measurement process. This function will not block the device.
 *****************************************************************************/
sl_status_t barometer_get_pressure_non_blocking(void (*user_cb)(float))
{
  uint32_t tick;

  if(barometer_state != BAROMETER_STATE_STANDBY){
    return SL_STATUS_INVALID_STATE;
  }

  callback_user = user_cb;
  barometer_start_conversion(BAROMETER_TEMPERATURE);

  tick = sl_sleeptimer_ms_to_tick(barometer_get_conversion_time_in_millis());

  return sl_sleeptimer_start_timer(&timer, tick, timer_callback, (void *)NULL, 0, 0);
}

/**************************************************************************//**
 *  Starts a temperature or pressure conversion process.
 *****************************************************************************/
sl_status_t barometer_start_conversion(barometer_measurement_t measurement_type)
{
  if(barometer_state != BAROMETER_STATE_STANDBY){
      return SL_STATUS_INVALID_STATE;
  }

  if(measurement_type == BAROMETER_TEMPERATURE){
      barometer_state = BAROMETER_STATE_TEMP_CONVERSION;
      return ms5637_sensor_write(MS5637_START_TEMPERATURE_CONVERSION + ms5637.oversample_rate * 2);
  }
  else if(measurement_type == BAROMETER_PRESSURE){
      barometer_state = BAROMETER_STATE_PRESS_CONVERSION;
      return ms5637_sensor_write(MS5637_START_PRESSURE_CONVERSION + ms5637.oversample_rate * 2);
  }

  return SL_STATUS_INVALID_PARAMETER;
}

/**************************************************************************//**
 *  Reads out the raw sensor data (temperature or pressure),
 *  if the conversion is ready.
 *****************************************************************************/
uint32_t barometer_read_raw_conversion()
{
  uint8_t sensorData[3];
  uint32_t adc_value;

  ms5637_sensor_write(MS5637_READ_ADC_COMMAND);

  ms5637_sensor_read(sensorData, MS5637_ADC_DATA_SIZE);

  adc_value = ((uint32_t)sensorData[0] << 16) | ((uint32_t)sensorData[1] << 8) | sensorData[2];

  barometer_state = BAROMETER_STATE_STANDBY;

  return adc_value;
}

/**************************************************************************//**
 *  This function is responsible for temperature compensation.
 *****************************************************************************/
void barometer_calculate(uint32_t raw_temp, uint32_t raw_press, float *temperature, float *pressure)
{
  int32_t dT, temp;
  int64_t OFF, SENS, P, T2, OFF2, SENS2;

  dT = (int32_t)raw_temp - ((int32_t)calibration_coeffs[MS5637_REFERENCE_TEMPERATURE_INDEX] << 8 );
  temp = 2000 + (((int64_t)dT * (int64_t)calibration_coeffs[MS5637_TEMP_COEFF_OF_TEMPERATURE_INDEX]) >> 23);

  // Second order temperature compensation
    if (temp < 2000) {
      T2 = (3 * ((int64_t)dT * (int64_t)dT)) >> 33;
      OFF2 = 61 * ((int64_t)temp - 2000) * ((int64_t)temp - 2000) / 16;
      SENS2 = 29 * ((int64_t)temp - 2000) * ((int64_t)temp - 2000) / 16;

      if (temp < -1500) {
        OFF2 += 17 * ((int64_t)temp + 1500) * ((int64_t)temp + 1500);
        SENS2 += 9 * ((int64_t)temp + 1500) * ((int64_t)temp + 1500);
      }
    } else {
      T2 = (5 * ((int64_t)dT * (int64_t)dT)) >> 38;
      OFF2 = 0;
      SENS2 = 0;
    }

    // OFF = OFF_T1 + TCO * dT
    OFF = ((int64_t)(calibration_coeffs[MS5637_PRESSURE_OFFSET_INDEX]) << 17) +
          (((int64_t)(calibration_coeffs[MS5637_TEMP_COEFF_OFFSET_INDEX]) *
            dT) >>
           6);
    OFF -= OFF2;

    // Sensitivity at actual temperature = SENS_T1 + TCS * dT
    SENS =
        ((int64_t)calibration_coeffs[MS5637_PRESSURE_SENSITIVITY_INDEX] << 16) +
        (((int64_t)calibration_coeffs[MS5637_TEMP_COEFF_SENSITIVITY_INDEX] *
          dT) >>
         7);
    SENS -= SENS2;

    // Temperature compensated pressure = D1 * SENS - OFF
    P = (((raw_press * SENS) >> 21) - OFF) >> 15;

    *temperature = ((float)temp - T2) / 100;
    *pressure = (float)P / 100;
}

/**************************************************************************//**
 *  Calculates the conversion time that a measurement takes.
 *****************************************************************************/
uint8_t barometer_get_conversion_time_in_millis()
{
  if(ms5637.oversample_rate >= MS5673_OSR_INVALID)
    return 0;
  return conversion_time[ms5637.oversample_rate];
}

/**************************************************************************//**
 *  Implements I2C master read function.
 *****************************************************************************/
static sl_status_t ms5637_sensor_read(uint8_t *rx_buff, uint8_t num_bytes)
{
  // Transfer structure
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef result;

  // Initializing I2C transfer
  seq.addr          = ms5637.I2C_address << 1;
  seq.flags         = I2C_FLAG_READ;
  seq.buf[0].data   = rx_buff;
  seq.buf[0].len    = num_bytes;

  result = I2CSPM_Transfer(ms5637.I2C_sensor.port, &seq);

  if(result != i2cTransferDone)
  {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/**************************************************************************//**
 *  Implements I2C master write function.
 *****************************************************************************/
static sl_status_t ms5637_sensor_write(uint8_t cmd)
{
  // Transfer structure
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef result;

  // Initializing I2C transfer
  seq.addr          = ms5637.I2C_address << 1;;
  seq.flags         = I2C_FLAG_WRITE;
  seq.buf[0].data   = &cmd;
  seq.buf[0].len    = 1;
  seq.buf[1].len    = 0;

  result = I2CSPM_Transfer(ms5637.I2C_sensor.port, &seq);

  if(result != i2cTransferDone)
   {
     return SL_STATUS_FAIL;
   }
   return SL_STATUS_OK;
}

/**************************************************************************//**
 *  Reads factory programmed coefficients from the sensor's PROM.
 *****************************************************************************/
sl_status_t ms5637_read_coeffs()
{
  uint8_t i;
  uint8_t temp_buff[2] = {0, 0};

  for(i=0; i<MS5637_COEFFICIENT_COUNT; i++)
  {
    ms5637_sensor_write(MS5637_COEFF_BASE_ADDRESS + (i*2));
    ms5637_sensor_read(temp_buff, 2);
    calibration_coeffs[i] = (temp_buff[0]<<8) | temp_buff[1];
  }

  return ms5637_crc4();
}

/**************************************************************************//**
 *  Verify coefficients by calculating CRC on the
 *  factory-calibrated values read from PROM.
 *****************************************************************************/
sl_status_t ms5637_crc4()
{
  uint8_t cnt; // simple counter
  uint8_t n_bit;

  uint16_t n_rem=0; // crc reminder
  uint16_t crc_factory_value = calibration_coeffs[0];
  uint16_t crc = (calibration_coeffs[0] & 0xF000) >> 12;

  calibration_coeffs[0]=((calibration_coeffs[0]) & 0x0FFF); // CRC byte is replaced by 0
  calibration_coeffs[7]=0; // Subsidiary value, set to 0

  for (cnt = 0; cnt < 16; cnt++) // operation is performed on bytes
  {
   if (cnt%2==1)
     n_rem ^= (calibration_coeffs[cnt>>1]) & 0x00FF;
   else
     n_rem ^= calibration_coeffs[cnt>>1]>>8;

   for (n_bit = 8; n_bit > 0; n_bit--)
   {
     if (n_rem & (0x8000))
       n_rem = (n_rem << 1) ^ 0x3000;
     else
       n_rem = (n_rem << 1);
   }
  }
  n_rem = ((n_rem >> 12) & 0x000F); // final 4-bit reminder is CRC code
  calibration_coeffs[0] = crc_factory_value;

  if (n_rem == crc)
    return SL_STATUS_OK;
  else
    return SL_STATUS_FAIL;
}

/**************************************************************************//**
 *  Callback function for the non-blocking sensor read function.
 *****************************************************************************/
void timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;
  uint32_t tick;

  if (barometer_state == BAROMETER_STATE_TEMP_CONVERSION){
      raw_temperature = barometer_read_raw_conversion();
      barometer_start_conversion(BAROMETER_PRESSURE);
      tick = sl_sleeptimer_ms_to_tick(barometer_get_conversion_time_in_millis());
      sl_sleeptimer_start_timer(&timer, tick, timer_callback, (void *)NULL, 0, 0);
  }
  else if (barometer_state == BAROMETER_STATE_PRESS_CONVERSION){
      raw_pressure = barometer_read_raw_conversion();
      barometer_calculate(raw_temperature, raw_pressure, &temperature, &pressure);
      barometer_state = BAROMETER_STATE_STANDBY;
      callback_user(pressure);
  }
}


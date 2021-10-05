/***************************************************************************//**
 * @file bme280.c
 * @brief BME280 sensor IC driver source
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <bme280.h>
#include "sl_sleeptimer.h"

// Create an I2C Instance for BME280
static bme280_i2c_t bme280;

// register addresses
#define  BME280_REGISTER_CONTROLHUMID     0xF2
#define  BME280_REGISTER_CONTROL          0xF4
#define  BME280_REGISTER_CONFIG           0xF5
#define  BME280_REGISTER_PRESSUREDATA     0xF7
#define  BME280_REGISTER_TEMPDATA         0xFA
#define  BME280_REGISTER_HUMIDDATA        0xFD

#define  BME280_REGISTER_STATUS           0xF3

#define  BME280_REGISTER_T1               0x88
#define  BME280_REGISTER_T2               0x8A
#define  BME280_REGISTER_T3               0x8C

#define  BME280_REGISTER_P1               0x8E
#define  BME280_REGISTER_P2               0x90
#define  BME280_REGISTER_P3               0x92
#define  BME280_REGISTER_P4               0x94
#define  BME280_REGISTER_P5               0x96
#define  BME280_REGISTER_P6               0x98
#define  BME280_REGISTER_P7               0x9A
#define  BME280_REGISTER_P8               0x9C
#define  BME280_REGISTER_P9               0x9E

#define  BME280_REGISTER_H1               0xA1
#define  BME280_REGISTER_H2               0xE1
#define  BME280_REGISTER_H3               0xE3
#define  BME280_REGISTER_H4               0xE4
#define  BME280_REGISTER_H5               0xE5
#define  BME280_REGISTER_H6               0xE7

#define  BME280_REGISTER_CHIP_ID          0xD0
#define  BME280_REGISTER_VERSION          0xD1
#define  BME280_REGISTER_RESET            0xE0

#define  BME280_REGISTER_CAL26            0xE1

// variables for calibration
uint16_t calib_t1 = 0;
int16_t calib_t2 = 0;
int16_t calib_t3 = 0;

uint8_t calib_h1 = 0;
int16_t calib_h2 = 0;
uint8_t calib_h3 = 0;
int16_t calib_h4 = 0;
int16_t calib_h5 = 0;
int8_t calib_h6 = 0;

uint16_t calib_p1 = 0;
int16_t calib_p2 = 0;
int16_t calib_p3 = 0;
int16_t calib_p4 = 0;
int16_t calib_p5 = 0;
int16_t calib_p6 = 0;
int16_t calib_p7 = 0;
int16_t calib_p8 = 0;
int16_t calib_p9 = 0;

int32_t t_fine = 0;

// -----------------------------------------------------------------------------
//                     Local Function Definitions
// -----------------------------------------------------------------------------

static bme280_ret_st read_one_byte(uint8_t registeraddress,
                                   uint8_t *returnbyte);
static bme280_ret_st write_one_byte(uint8_t registeraddress,
                                    uint8_t writevalue);
static bme280_ret_st read_two_bytes(uint8_t registeraddress,
                                    uint16_t *returnbyte);
static bme280_ret_st read_two_bytes_little_endian(uint8_t registeraddress,
                                                  uint16_t *readoutbytes);
static bme280_ret_st read_two_bytes_short16(uint8_t registeraddress,
                                            int16_t *readoutbytes);
static bme280_ret_st read_factory_compensation(void);

// -----------------------------------------------------------------------------
//                         Public Functions
// -----------------------------------------------------------------------------

sl_status_t bme280_i2c(bme280_i2c_t *init)
{
  bme280.i2c_sensor = init->i2c_sensor;
  bme280.i2c_address = init->i2c_address;
  bme280.oversample_rate = init->oversample_rate;
  return SL_STATUS_OK;
}

bme280_ret_st bme280_init(void)
{
  bme280_ret_st ret_val = STATUS_FAILED;
  uint8_t checksum = 0;
  t_fine = 0;
  // Read the ID register, check the stored value.
  uint8_t chip_id = 0;
  checksum += read_one_byte(BME280_REGISTER_CHIP_ID, &chip_id);

  if (chip_id != 0x60) {
    return ret_val;
  }
  // Reset the sensor.
  checksum += write_one_byte(BME280_REGISTER_RESET, 0xB6);
  sl_sleeptimer_delay_millisecond(2); // After reset recovery needed.
  checksum += read_factory_compensation();
  // Set the registers for sampling and filtering.
  checksum += write_one_byte(BME280_REGISTER_CONTROLHUMID, 0b01);
  checksum += write_one_byte(BME280_REGISTER_CONTROL, 0b100110);
  checksum += write_one_byte(BME280_REGISTER_CONFIG, 0b10000000);

  if (checksum >= 6) {
    ret_val = STATUS_OK;
  }
  return ret_val;
}

bme280_ret_st bme280_read_temperature(int32_t *temperature)
{
  // This function is based on the BME280 sample code from data sheet.
  bme280_ret_st ret_val = STATUS_FAILED;
  uint8_t checksum = 0;
  int32_t var1 = 0, var2 = 0, adc_t = 0;
  uint8_t temporary1 = 0;
  uint16_t temporary2 = 0;
  checksum += read_two_bytes_little_endian(BME280_REGISTER_TEMPDATA,
                                           &temporary2);
  adc_t = temporary2;
  adc_t <<= 8;
  checksum += read_one_byte(BME280_REGISTER_TEMPDATA + 2, &temporary1);
  adc_t |= temporary1;
  adc_t >>= 4;

  var1 = ((((adc_t >> 3) - ((int32_t) calib_t1 << 1))) * ((int32_t) calib_t2))
         >> 11;
  var2 = (((((adc_t >> 4) - ((int32_t) calib_t1))
         * ((adc_t >> 4) - ((int32_t) calib_t1))) >> 12) * ((int32_t) calib_t3))
         >> 14;
  t_fine = var1 + var2;
  *temperature = (t_fine * 5 + 128) >> 8;

  if (checksum >= 2) {
    ret_val = STATUS_OK;
  }
  return ret_val;
}

bme280_ret_st bme280_read_humidity(uint32_t *humidity)
{
  // This function is based on the BME280 sample code from data sheet.
  bme280_ret_st ret_val = STATUS_FAILED;
  uint8_t checksum = 0;
  int32_t adc_h = 0;
  uint16_t temporary = 0;
  checksum += read_two_bytes_little_endian(BME280_REGISTER_HUMIDDATA,
                                           &temporary);
  adc_h = temporary;
  int32_t v_x1_u32r;

  v_x1_u32r = (t_fine - ((int32_t) 76800));

  v_x1_u32r = (((((adc_h << 14) - (((int32_t) calib_h4) << 20)
      - (((int32_t) calib_h5) * v_x1_u32r)) + ((int32_t) 16384)) >> 15)
      * (((((((v_x1_u32r * ((int32_t) calib_h6)) >> 10)
      * (((v_x1_u32r * ((int32_t) calib_h3)) >> 11) + ((int32_t) 32768)))
      >> 10) + ((int32_t) 2097152)) * ((int32_t) calib_h2) + 8192) >> 14));

  v_x1_u32r = (v_x1_u32r
      - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t) calib_h1))
      >> 4));

  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;

  *humidity = (v_x1_u32r >> 12);

  if (checksum >= 1) {
    ret_val = STATUS_OK;
  }
  return ret_val;
}

bme280_ret_st bme280_read_pressure(uint32_t *pressure)
{
  // This function is based on the BME280 sample code from data sheet.
  bme280_ret_st ret_val = STATUS_FAILED;
  uint8_t checksum = 0;
  int64_t var1 = 0, var2 = 0, press = 0;
  uint16_t temporary1 = 0;
  uint8_t temporary2 = 0;
  int32_t adc_p = 0;

  checksum += read_two_bytes_little_endian(BME280_REGISTER_PRESSUREDATA,
                                           &temporary1);
  adc_p = temporary1;
  adc_p <<= 8;
  checksum += read_one_byte(BME280_REGISTER_PRESSUREDATA + 2,
                            &temporary2);
  adc_p |= temporary2;
  adc_p >>= 4;
  var1 = ((int64_t) t_fine) - 128000;
  var2 = var1 * var1 * (int64_t) calib_p6;
  var2 = var2 + ((var1 * (int64_t) calib_p5) << 17);
  var2 = var2 + (((int64_t) calib_p4) << 35);
  var1 = ((var1 * var1 * (int64_t) calib_p3) >> 8)
         + ((var1 * (int64_t) calib_p2) << 12);
  var1 = (((((int64_t) 1) << 47) + var1)) * ((int64_t) calib_p1) >> 33;

  if (var1 == 0) {
    return ret_val;  // Avoid exception caused by division by zero.
  }
  press = 1048576 - adc_p;
  press = (((press << 31) - var2) * 3125) / var1;
  var1 = (((int64_t) calib_p9) * (press >> 13) * (press >> 13)) >> 25;
  var2 = (((int64_t) calib_p8) * press) >> 19;

  press = ((press + var1 + var2) >> 8) + (((int64_t) calib_p7) << 4);
  *pressure = press / 25600;

  if (checksum >= 2) {
    ret_val = STATUS_OK;
  }
  return ret_val;
}

bme280_ret_st bme280_ctrl_measure_set_to_work(void)
{
  bme280_ret_st ret_val = STATUS_FAILED;
  // 0b 1 001 10  is pressure oversampling: x1, temp oversampling: x1
  // register setting mode: forced mode.
  ret_val = write_one_byte(BME280_REGISTER_CONTROL, 0b100110);
  return ret_val;
}

bme280_ret_st bme280_ctrl_measure_set_to_sleep(void)
{
  bme280_ret_st ret_val = STATUS_FAILED;
  // 0b 1 001 00  is pressure oversampling: x1, temp oversampling: x1
  // register setting mode: sleep mode.
  ret_val = write_one_byte(BME280_REGISTER_CONTROL, 0b100100);
  return ret_val;
}

// -----------------------------------------------------------------------------
//                          Local functions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief
 *   To read out the factory calibration compensating values from BME280.
 * @return
 *   Return value is STATUS_FAILED or STATUS_OK.
 ******************************************************************************/
static bme280_ret_st read_factory_compensation(void)
{
  // This code is based on the BME280 sample code from data sheet.
  uint8_t checksum = 0;
  bme280_ret_st ret_val = STATUS_FAILED;
  uint16_t temp1 = 0;
  uint8_t temp2 = 0;
  uint8_t temp3 = 0;
  checksum += read_two_bytes(BME280_REGISTER_T1, &calib_t1);
  checksum += read_two_bytes(BME280_REGISTER_T2, &temp1);
  calib_t2 = (int16_t) temp1;
  checksum += read_two_bytes(BME280_REGISTER_T3, &temp1);
  calib_t3 = (int16_t) temp1;
  checksum += read_one_byte(BME280_REGISTER_H1, &calib_h1);
  checksum += read_one_byte(0xE2, &temp2);
  temp2 <<= 8;
  checksum += read_one_byte(0xE1, &temp3);
  calib_h2 = (int16_t) (temp2 + temp3);
  checksum += read_one_byte(BME280_REGISTER_H3, &calib_h3);
  checksum += read_one_byte(BME280_REGISTER_H4, &temp2);
  temp2 <<= 4;
  checksum += read_one_byte(BME280_REGISTER_H4 + 1, &temp3);
  calib_h4 = temp2 | (temp3 & 0x0F);
  checksum += read_one_byte(BME280_REGISTER_H5 + 1, &temp2);
  temp2 <<= 4;
  checksum += read_one_byte(BME280_REGISTER_H5, &temp3);
  temp3 >>= 4;
  calib_h5 = temp2 | temp3;
  checksum += read_one_byte(BME280_REGISTER_H6, &temp2);
  calib_h6 = (int8_t) temp2;

  checksum += read_two_bytes(BME280_REGISTER_P1, &calib_p1);
  checksum += read_two_bytes_short16(BME280_REGISTER_P2, &calib_p2);
  checksum += read_two_bytes_short16(BME280_REGISTER_P3, &calib_p3);
  checksum += read_two_bytes_short16(BME280_REGISTER_P4, &calib_p4);
  checksum += read_two_bytes_short16(BME280_REGISTER_P5, &calib_p5);
  checksum += read_two_bytes_short16(BME280_REGISTER_P6, &calib_p6);
  checksum += read_two_bytes_short16(BME280_REGISTER_P7, &calib_p7);
  checksum += read_two_bytes_short16(BME280_REGISTER_P8, &calib_p8);
  checksum += read_two_bytes_short16(BME280_REGISTER_P9, &calib_p9);

  if (checksum >= 21) {
    ret_val = STATUS_OK;
  }
  return ret_val;
}

/***************************************************************************//**
 * @brief
 *   I2C master read function for a single byte.
 *
 * @param [in] registeraddress
 *   The register address to read out.
 * @param [in] *returnbyte
 *   The value of this pointer will change to
 *   the read out value of the register.
 *
 * @return
 *   Return value is STATUS_FAILED or STATUS_OK.
 ******************************************************************************/
static bme280_ret_st read_one_byte(uint8_t registeraddress, uint8_t *returnbyte)
{
  bme280_ret_st ret_val = STATUS_FAILED;
  uint8_t data = 0;
  // Transfer structure.
  I2C_TransferSeq_TypeDef i2c_transfer;

  // Initializing I2C transfer.
  i2c_transfer.addr = BME_280_DEFAULT_I2C_ADDR << 1;
  /// Master write
  i2c_transfer.flags = I2C_FLAG_WRITE_READ;

  // Transmit buffer, no data to send.
  i2c_transfer.buf[0].data = &registeraddress;
  i2c_transfer.buf[0].len = 1;

  // Receive buffer, two bytes to receive.
  i2c_transfer.buf[1].data = &data;
  i2c_transfer.buf[1].len = 1;

  if (I2CSPM_Transfer(I2C0, &i2c_transfer) == i2cTransferDone) {
    ret_val = STATUS_OK;
  }
  *returnbyte = data;
  return ret_val;
}

/***************************************************************************//**
 * @brief
 *   I2C master write read function for a single byte.
 *
 * @param [in] registeraddress
 *   The register address to write.
 * @param [in] writevalue
 *   The value to write to the given register.
 *
 * @return
 *   Return value is STATUS_FAILED or STATUS_OK.
 ******************************************************************************/
static bme280_ret_st write_one_byte(uint8_t registeraddress, uint8_t writevalue)
{
  bme280_ret_st ret_val = STATUS_FAILED;
  uint8_t data[2] = { registeraddress, writevalue };
  // Transfer structure.
  I2C_TransferSeq_TypeDef i2c_transfer;

  // Initializing I2C transfer.
  i2c_transfer.addr = BME_280_DEFAULT_I2C_ADDR << 1;
  // Master write
  i2c_transfer.flags = I2C_FLAG_WRITE;

  // Transmit buffer, 2 bytes to send.
  i2c_transfer.buf[0].data = data;
  i2c_transfer.buf[0].len = 2;

  i2c_transfer.buf[1].data = 0UL;
  i2c_transfer.buf[1].len = 0;

  if (I2CSPM_Transfer(I2C0, &i2c_transfer) == i2cTransferDone) {
    ret_val = STATUS_OK;
  }
  return ret_val;
}

/***************************************************************************//**
 * @brief
 *   I2C master read function for two bytes from consecutive registers.
 *
 *  @param [in] registeraddress
 *   The register address to read out.
 * @param [in] *returnbyte
 *   The value of this pointer will change to
 *   the read out value of the register.
 *
 * @return
 *   Return value is STATUS_FAILED or STATUS_OK.
 ******************************************************************************/
static bme280_ret_st read_two_bytes(uint8_t registeraddress,
                                    uint16_t *returnbyte)
{
  bme280_ret_st ret_val = STATUS_FAILED;
  uint16_t data = 0;

  // Transfer structure.
  I2C_TransferSeq_TypeDef i2c_transfer;

  // Initializing I2C transfer.
  i2c_transfer.addr = BME_280_DEFAULT_I2C_ADDR << 1;
  // Master write.
  i2c_transfer.flags = I2C_FLAG_WRITE_READ;
  // Transmit buffer, no data to send.
  i2c_transfer.buf[0].data = &registeraddress;
  i2c_transfer.buf[0].len = 1;

  // Receive buffer, two bytes to receive.
  i2c_transfer.buf[1].data = (uint8_t*)&data;
  i2c_transfer.buf[1].len = 2;

  if (I2CSPM_Transfer(I2C0, &i2c_transfer) == i2cTransferDone) {
    ret_val = STATUS_OK;
  }
  *returnbyte = data;
  return ret_val;
}

/***************************************************************************//**
 * @brief
 *   I2C master read function for two bytes from consecutive registers.
 *
 * @param [in] registeraddress
 *   The register address to read out.
 * @param [in] *returnbyte
 *   The value of this pointer will change to the read out value of the
 *   register in a little endian order.
 *
 * @return
 *   Return value is STATUS_FAILED or STATUS_OK.
 ******************************************************************************/
static bme280_ret_st read_two_bytes_little_endian(uint8_t registeraddress,
                                                  uint16_t *readoutbytes)
{
  bme280_ret_st ret_val = STATUS_FAILED;
  uint16_t temp = *readoutbytes;
  ret_val = read_two_bytes(registeraddress, &temp);
  *readoutbytes = (temp >> 8) | (temp << 8);

  return ret_val;
}

/***************************************************************************//**
 * @brief
 *   I2C master read function for two bytes from consecutive registers.
 *
 * @param [in] registeraddress
 *   The register address to read out.
 * @param [in] *returnbyte
 *   The value of this pointer will change to the read out value of
 *   the register in a short16 format.
 *
 * @return
 *   Return value is STATUS_FAILED or STATUS_OK.
 ******************************************************************************/
static bme280_ret_st read_two_bytes_short16(uint8_t registeraddress,
                                            int16_t *readoutbytes)
{
  bme280_ret_st ret_val = STATUS_FAILED;
  uint16_t temp = *readoutbytes;
  ret_val = read_two_bytes(registeraddress, &temp);
  *readoutbytes = temp;
  return ret_val;
}

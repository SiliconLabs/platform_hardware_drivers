/***************************************************************************//**
 * @file  mlx90632.c
 * @brief IrThremo 3 Click driver.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <mlx90632.h>
#include <mlx90632_i2c.h>
#include <math.h>

/// MLX90632 calibration variables
static int16_t ambient_new_raw;
static int16_t ambient_old_raw;
static int16_t object_new_raw;
static int16_t object_old_raw;

static int32_t PR;
static int32_t PG;
static int32_t PT;
static int32_t PO;
static int32_t Ea;
static int32_t Eb;
static int32_t Fa;
static int32_t Fb;
static int32_t Ga;

static int16_t Gb;
static int16_t Ka;
static int16_t Ha;
static int16_t Hb;

// Implementation of reading all calibration parameters
int32_t mlx90632_read_eeprom(int32_t *PR, int32_t *PG, int32_t *PO, int32_t *PT,
                             int32_t *Ea, int32_t *Eb, int32_t *Fa, int32_t *Fb,
                             int32_t *Ga, int16_t *Gb, int16_t *Ka, int16_t *Ha,
                             int16_t *Hb)
{
  int32_t ret;
  ret = mlx90632_i2c_read32(MLX90632_EE_P_R, (uint32_t *) PR);
  if (ret < 0) {
    return ret;
  }

  ret = mlx90632_i2c_read32(MLX90632_EE_P_G, (uint32_t *) PG);
  if (ret < 0) {
    return ret;
  }

  ret = mlx90632_i2c_read32(MLX90632_EE_P_O, (uint32_t *) PO);
  if (ret < 0) {
    return ret;
  }

  ret = mlx90632_i2c_read32(MLX90632_EE_P_T, (uint32_t *) PT);
  if (ret < 0) {
    return ret;
  }

  ret = mlx90632_i2c_read32(MLX90632_EE_Ea, (uint32_t *) Ea);
  if (ret < 0) {
    return ret;
  }

  ret = mlx90632_i2c_read32(MLX90632_EE_Eb, (uint32_t *) Eb);
  if (ret < 0) {
    return ret;
  }

  ret = mlx90632_i2c_read32(MLX90632_EE_Fa, (uint32_t *) Fa);
  if (ret < 0) {
    return ret;
  }

  ret = mlx90632_i2c_read32(MLX90632_EE_Fb, (uint32_t *) Fb);
  if (ret < 0) {
    return ret;
  }

  ret = mlx90632_i2c_read32(MLX90632_EE_Ga, (uint32_t *) Ga);
  if (ret < 0) {
    return ret;
  }

  ret = mlx90632_i2c_read(MLX90632_EE_Gb, (uint16_t *) Gb);
  if (ret < 0) {
    return ret;
  }

  ret = mlx90632_i2c_read(MLX90632_EE_Ha, (uint16_t *) Ha);
  if (ret < 0) {
    return ret;
  }

  ret = mlx90632_i2c_read(MLX90632_EE_Hb, (uint16_t *) Hb);
  if (ret < 0) {
    return ret;
  }

  ret = mlx90632_i2c_read(MLX90632_EE_Ka, (uint16_t *) Ka);
  if (ret < 0) {
    return ret;
  }
  // Set measuring mode to continuous
  setMode_Continuous();

  return ret;
}

// Initialize MLX90632 driver, confirm EEPROM version
int32_t mlx90632_init(void)
{
  int32_t ret;
  uint16_t eeprom_version, reg_status;

  ret = mlx90632_i2c_read(MLX90632_EE_VERSION, &eeprom_version);
  if (ret < 0){
    return ret;
  }

  if (eeprom_version != MLX90632_EEPROM_VERSION){
    return NOT_MATCHING_EEPROM_VERSION;
  }

  ret = mlx90632_i2c_read(MLX90632_REG_STATUS, &reg_status);
  if (ret < 0){
    return ret;
  }

  // Prepare a clean start with setting NEW_DATA to 0
  ret = mlx90632_i2c_write(MLX90632_REG_STATUS,
                           reg_status & ~(MLX90632_STAT_DATA_RDY));
  if (ret < 0) {
    return ret;
  }

  // Read the eeprom registers value from the mlx90632
  ret = mlx90632_read_eeprom(&PR, &PG, &PO, &PT,
                             &Ea, &Eb, &Fa, &Fb,
                             &Ga, &Gb, &Ka, &Ha, &Hb);

  return ret;
}

// Read ambient raw old and new values
int32_t mlx90632_read_temp_ambient_raw(int16_t *ambient_new_raw,
                                       int16_t *ambient_old_raw)
{
  int32_t ret;
  uint16_t read_tmp;

  ret = mlx90632_i2c_read(MLX90632_REG_RAM_6, &read_tmp);
  if (ret < 0) {
    return ret;
  }
  *ambient_new_raw = (int16_t)read_tmp;

  ret = mlx90632_i2c_read(MLX90632_REG_RAM_9, &read_tmp);
  if (ret < 0) {
    return ret;
  }
  *ambient_old_raw = (int16_t)read_tmp;

  return ret;
}

// Pre-calculations for ambient
double mlx90632_preprocess_temp_ambient(int16_t ambient_new_raw,
                                        int16_t ambient_old_raw, int16_t Gb)
{
  double VR_Ta, kGb;

  kGb = (double)Gb * 0.0009765625;
  VR_Ta = ambient_old_raw + kGb * (ambient_new_raw / 12.0);

  return ((ambient_new_raw / (12.0) ) / VR_Ta) * 524288.0;
}

// Ambient temperature calculations
double mlx90632_calc_temp_ambient(int16_t ambient_new_raw,
                                  int16_t ambient_old_raw, int32_t P_T,
                                  int32_t P_R, int32_t P_G,
                                  int32_t P_O, int16_t Gb)
{
  double AMB, ambientTemp;

  AMB = mlx90632_preprocess_temp_ambient(ambient_new_raw,
                                           ambient_old_raw, Gb);

  P_T = P_T * 5.6843418860808015E-14;
  P_R = P_R * 0.00390625;
  P_G = P_G * 0.000000953;
  P_O = P_O * 0.00390625;

  ambientTemp = P_O + (AMB - P_R) / P_G + P_T * (AMB - P_R)*(AMB - P_R);

  return ambientTemp;
}

// Clear REG_STATUS new_data bit
int32_t clearDataAvailable(void)
{
  int32_t ret;
  uint16_t reg_status;

  ret = mlx90632_i2c_read(MLX90632_REG_STATUS, &reg_status);
  if (ret < 0) {
    return ret;
  }

  ret = mlx90632_i2c_write(MLX90632_REG_STATUS,
                           reg_status & (~MLX90632_STAT_DATA_RDY));

  return ret;
}

// Give back the measurement mode
uint8_t getMode(void)
{
  int32_t ret;
  uint16_t mode;

  ret = mlx90632_i2c_read(MLX90632_REG_CONTROL, &mode);
  if (ret < 0) {
    return ret;
  }

  mode = (mode >> 1) & 0x0003;
  return (uint8_t)mode;
}

// Set the measurement mode to CONTINUOUS
int32_t setMode_Continuous(void)
{
  int32_t ret;
  uint16_t mode;

  ret = mlx90632_i2c_read(MLX90632_REG_CONTROL, &mode);
  if (ret < 0) {
    return ret;
  }

  mode |= 0x06;

  ret = mlx90632_i2c_write(MLX90632_REG_CONTROL, mode);

  return ret;
}

// Blocking delay
void sleep(int value)
{
  while(--value);
}

// Assert REG_STATUS and give back cyclic state.
int mlx90632_start_measurement(void)
{
  int ret, tries = 7500;
  uint16_t reg_status;

  ret = mlx90632_i2c_read(MLX90632_REG_STATUS, &reg_status);
  if (ret < 0) {
    return ret;
  }

  ret = mlx90632_i2c_write(MLX90632_REG_STATUS,
                           reg_status & (~MLX90632_STAT_DATA_RDY));
  if (ret < 0) {
    return ret;
  }

  while (tries-- > 0) {
    ret = mlx90632_i2c_read(MLX90632_REG_STATUS, &reg_status);
    if (ret < 0) {
      return ret;
    }

    if (reg_status & MLX90632_STAT_DATA_RDY) {
      break;
    }

    sleep(10000);
  }

  if (tries < 0) {
    return -1;
  }

  return (reg_status & MLX90632_REG_STATUS_BITMASK) >> 2;
}

// RAM select
int32_t mlx90632_channel_new_select(int32_t ret,
                                    uint8_t *channel_new,
                                    uint8_t *channel_old)
{
  switch (ret)
  {
    case 1:
      *channel_new = 1;
      *channel_old = 2;
      break;

    case 2:
      *channel_new = 2;
      *channel_old = 1;
      break;

    default:
      return -1;
  }
  return 0;
}

int32_t mlx90632_read_temp_object_raw(int32_t start_measurement_ret,
                                      int16_t *object_new_raw,
                                      int16_t *object_old_raw)
{
  int32_t ret;
  uint16_t read_tmp;
  int16_t read;
  uint8_t channel, channel_old;

  ret = mlx90632_channel_new_select(start_measurement_ret,
                                    &channel, &channel_old);
  if (ret != 0) {
    return ret;
  }

  ret = mlx90632_i2c_read(MLX90632_RAM_2(channel), &read_tmp);
  if (ret < 0) {
    return ret;
  }

  read = (int16_t)read_tmp;

  ret = mlx90632_i2c_read(MLX90632_RAM_1(channel), &read_tmp);
  if (ret < 0) {
    return ret;
  }
  *object_new_raw = (read + (int16_t)read_tmp) / 2;

  ret = mlx90632_i2c_read(MLX90632_RAM_2(channel_old), &read_tmp);
  if (ret < 0) {
    return ret;
  }
  read = (int16_t)read_tmp;

  ret = mlx90632_i2c_read(MLX90632_RAM_1(channel_old), &read_tmp);
  if (ret < 0) {
    return ret;
  }
  *object_old_raw = (read + (int16_t)read_tmp) / 2;

  return ret;
}

int32_t mlx90632_read_temp_raw(int16_t *ambient_new_raw,
                               int16_t *ambient_old_raw,
                               int16_t *object_new_raw,
                               int16_t *object_old_raw)
{
  int32_t ret, start_measurement_ret;

  // Read new and old **ambient** values from sensor
  ret = mlx90632_read_temp_ambient_raw(ambient_new_raw, ambient_old_raw);
  if (ret < 0) {
    return ret;
  }

  start_measurement_ret = mlx90632_start_measurement();
  if (start_measurement_ret < 0) {
    return start_measurement_ret;
  }

  // Read new and old **object** values from sensor
  ret = mlx90632_read_temp_object_raw(start_measurement_ret,
                                      object_new_raw, object_old_raw);

  return ret;
}

double mlx90632_preprocess_temp_object(int16_t object_new_raw,
                                       int16_t object_old_raw,
                                       int16_t ambient_new_raw,
                                       int16_t ambient_old_raw,
                                       int16_t Ka)
{
  double VR_IR, kKa;

  kKa = ((double)Ka) / 1024.0;
  VR_IR = ambient_old_raw + kKa * (ambient_new_raw / (MLX90632_REF_3));

  // Return STo=(S/12)/VRTO*2^19
  return ((((object_new_raw + object_old_raw) / 2) /
         (MLX90632_REF_12)) / VR_IR) * 524288.0;
}

double mlx90632_calc_temp_object_iteration(double prev_object_temp,
                                           int32_t object, double TAdut,
                                           int32_t Ga, int32_t Fa,
                                           int32_t Fb, int16_t Ha,
                                           int16_t Hb, double emissivity)
{
  double calcedGa, calcedGb, calcedFa, TAdut4, first_sqrt;
  // temp variables
  double KsTAtmp, Alpha_corr;
  double Ha_customer, Hb_customer;

  Ha_customer = Ha / ((double)16384.0);
  Hb_customer = Hb / ((double)16384.0);

  calcedGa = ((double)Ga * (prev_object_temp - 25)) / ((double)68719476736.0);
  KsTAtmp = (double)Fb * (TAdut - 25);
  calcedGb = KsTAtmp / ((double)68719476736.0);

  Alpha_corr = (((double)(Fa * POW10)) * Ha_customer *
               (double)(1 + calcedGa + calcedGb)) /
               ((double)70368744177664.0);

  calcedFa = object / (emissivity * (Alpha_corr / POW10));

  // TA^4
  TAdut4 = (TAdut + 273.15) * (TAdut + 273.15) *
           (TAdut + 273.15) * (TAdut + 273.15);

  first_sqrt = sqrt(calcedFa + TAdut4);

  return (sqrt(first_sqrt) - 273.15 - Hb_customer);
}

double mlx90632_calc_temp_object(int32_t object, int32_t ambient,
                                 int32_t Ea, int32_t Eb, int32_t Ga,
                                 int32_t Fa, int32_t Fb,
                                 int16_t Ha, int16_t Hb)
{
  double kEa, kEb, TAdut;
  double temp = 25.0;
  double tmp_emi = 1.0;
  int8_t i;

  kEa = ((double)Ea) / ((double)65536.0);
  kEb = ((double)Eb) / ((double)256.0);
  TAdut = (((double)ambient) - kEb) / kEa + 25;

  //iterate through calculations (minimum 3)
  for (i = 0; i < 5; ++i) {
    temp = mlx90632_calc_temp_object_iteration(temp, object, TAdut, Ga,
                                               Fa, Fb, Ha, Hb, tmp_emi);
  }
  return temp;
}

// Function for device reset
int32_t mlx90632_addressed_reset(void)
{
  return mlx90632_i2c_write(0x3005, MLX90632_RESET_CMD);
}

//  Function for unlock EEPROM
int32_t mlx90632_unlock_eeprom(void)
{
  return mlx90632_i2c_write(0x3005, MLX90632_EEPROM_UNLOCK_KEY);
}

// Function gives back both temperature values.
int32_t measurment_cb(double *ambient,double *object)
{
  int32_t ret;
  double pre_ambient, pre_object;

  ret = mlx90632_read_temp_raw(&ambient_new_raw, &ambient_old_raw,
                               &object_new_raw, &object_old_raw);

  pre_ambient = mlx90632_preprocess_temp_ambient(ambient_new_raw,
                                                 ambient_old_raw, Gb);

  *ambient = mlx90632_calc_temp_ambient(ambient_new_raw, ambient_old_raw, PT,
                                   PR, PG, PO, Gb);

  pre_object = mlx90632_preprocess_temp_object(object_new_raw, object_old_raw,
                                               ambient_new_raw, ambient_old_raw,
                                               Ka);

  *object = mlx90632_calc_temp_object(pre_object, pre_ambient, Ea,
                                     Eb, Ga, Fa, Fb, Ha, Hb);

  return ret;
}

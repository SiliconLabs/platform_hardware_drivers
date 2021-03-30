/***************************************************************************//**
 * @file  mlx90632.h
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

#ifndef MLX90632_H
#define MLX90632_H

#include <stdint.h>
#include <stdbool.h>
#include <sl_status.h>

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#ifndef BIT
#define BIT(x)(1UL << (x))
#endif

// 32 bit calibration parameters memory address
#define MLX90632_EE_P_R            (0x240c)
#define MLX90632_EE_P_G            (0x240e)
#define MLX90632_EE_P_T            (0x2410)
#define MLX90632_EE_P_O            (0x2412)
#define MLX90632_EE_Aa             (0x2414)
#define MLX90632_EE_Ab             (0x2416)
#define MLX90632_EE_Ba             (0x2418)
#define MLX90632_EE_Bb             (0x241a)
#define MLX90632_EE_Ca             (0x241c)
#define MLX90632_EE_Cb             (0x241e)
#define MLX90632_EE_Da             (0x2420)
#define MLX90632_EE_Db             (0x2422)
#define MLX90632_EE_Ea             (0x2424)
#define MLX90632_EE_Eb             (0x2426)
#define MLX90632_EE_Fa             (0x2428)
#define MLX90632_EE_Fb             (0x242a)
#define MLX90632_EE_Ga             (0x242c)

// 16 bit calibration parameters memory address
#define MLX90632_EE_Gb             (0x242e)
#define MLX90632_EE_Ka             (0x242f)
#define MLX90632_EE_Ha             (0x2481)
#define MLX90632_EE_Hb             (0x2482)

// Memory sections addresses
#define MLX90632_ADDR_RAM          (0x4000)
#define MLX90632_ADDR_EEPROM       (0x2480)
#define MLX90632_REG_CONTROL       (0x3001)

// EEPROM addresses - used at startup
#define MLX90632_EE_CTRL           (0x24d4)

#define MLX90632_EE_I2C_ADDRESS    (0x24d5)
#define MLX90632_EE_VERSION        (0x240b)

// RAM measure address-es
#define MLX90632_REG_RAM_1         (0x4000)
#define MLX90632_REG_RAM_2         (0x4001)
#define MLX90632_REG_RAM_3         (0x4002)
#define MLX90632_REG_RAM_4         (0x4003)
#define MLX90632_REG_RAM_5         (0x4004)
#define MLX90632_REG_RAM_6         (0x4005)
#define MLX90632_REG_RAM_7         (0x4006)
#define MLX90632_REG_RAM_8         (0x4007)
#define MLX90632_REG_RAM_9         (0x4008)

// RAM_MEAS address-es for each channel
#define MLX90632_RAM_1(meas_num)   (MLX90632_ADDR_RAM + 3 * meas_num)
#define MLX90632_RAM_2(meas_num)   (MLX90632_ADDR_RAM + 3 * meas_num + 1)
#define MLX90632_RAM_3(meas_num)   (MLX90632_ADDR_RAM + 3 * meas_num + 2)

// Device status register
#define   MLX90632_REG_STATUS       (0x3fff)
#define   MLX90632_STAT_DATA_RDY    BIT(0)

// Constants
#define MLX90632_EEPROM_VERSION     (0x8405)
#define NOT_MATCHING_EEPROM_VERSION (-6)
#define MLX90632_REF_12             (12.0)
#define MLX90632_REF_3              (12.0)
#define MLX90632_REG_STATUS_BITMASK (0x007C)
#define POW10                       (10000000000LL)
#define MLX90632_RESET_CMD          (0x0006)
#define MLX90632_EEPROM_UNLOCK_KEY  (0x554C)

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief Read the eeprom registers value from the mlx90632.
 *
 * @param[out]  P_R calibration constant
 * @param[out]  P_G calibration constant
 * @param[out]  P_O calibration constant
 * @param[out]  P_T calibration constant
 * @param[out]  Ea calibration constant
 * @param[out]  Eb calibration constant
 * @param[out]  Fa calibration constant
 * @param[out]  Fb calibration constant
 * @param[out]  Ga calibration constant
 * @param[out]  Gb calibration constant
 * @param[out]  Ka calibration constant
 * @param[out]  Ha Customer calibration constant
 * @param[out]  Hb Customer calibration constant
 *
 * @retval  0 Successfully read eeprom register.
 * @retval <0 Something went wrong.
 ******************************************************************************/
int32_t mlx90632_read_eeprom(int32_t *PR, int32_t *PG, int32_t *PO, int32_t *PT,
                             int32_t *Ea, int32_t *Eb, int32_t *Fa, int32_t *Fb,
                             int32_t *Ga, int16_t *Gb, int16_t *Ka, int16_t *Ha,
                             int16_t *Hb);

/***************************************************************************//**
 * @brief Initialize MLX90632 driver.
 *
 * EEPROM version is important to match sensor EEPROM content and calculations.
 * This is why this function checks for correct EEPROM version before it does
 * checksum validation of the EEPROM content.
 *
 * @retval  0 Successfully initialized MLX90632 driver.
 * @retval <0 Something went wrong.
 ******************************************************************************/
int32_t mlx90632_init(void);

/***************************************************************************//**
 * @brief Read ambient raw old and new values.
 *
 * @param[out] *ambient_new_raw Pointer
 * to memory location where new ambient value from sensor is stored.
 * @param[out] *ambient_old_raw Pointer
 * to memory location where old ambient value from sensor is stored.
 *
 * @retval  0 Successfully read both values.
 * @retval <0 Something went wrong.
 ******************************************************************************/
int32_t mlx90632_read_temp_ambient_raw(int16_t *ambient_new_raw,
                                       int16_t *ambient_old_raw);

/***************************************************************************//**
 * @brief Pre-calculations for ambient temperature.
 *
 * @param[in] *ambient_new_raw Pointer to
 *             memory location where new ambient value from sensor is stored.
 * @param[in] *ambient_old_raw  Pointer
 *             to memory location where old ambient value from sensor is stored.
 * @param[in] Gb calibration constant.
 *
 * @retval Calculated AMB value.
 ******************************************************************************/
double mlx90632_preprocess_temp_ambient(int16_t ambient_new_raw,
                                        int16_t ambient_old_raw, int16_t Gb);

/***************************************************************************//**
 * @brief Ambient temperature calculations.
 *
 * @param[in] ambient_new_raw value.
 * @param[in] ambient_old_raw value.
 * @param[in] P_T calibration constant.
 * @param[in] P_R calibration constant.
 * @param[in] P_G calibration constant.
 * @param[in] P_O calibration constant.
 * @param[in] Gb calibration constant.
 *
 * @retval Calculated ambient temperature value in C.
 ******************************************************************************/
double mlx90632_calc_temp_ambient(int16_t ambient_new_raw,
                                  int16_t ambient_old_raw, int32_t P_T,
                                  int32_t P_R, int32_t P_G, int32_t P_O,
                                  int16_t Gb);

/***************************************************************************//**
 * @brief Clear REG_STATUS new_data bit.
 *
 * @retval  0 Successfully read and write values.
 * @retval <0 Something went wrong.
 ******************************************************************************/
int32_t clearDataAvailable(void);

/***************************************************************************//**
 * @brief Get measurement modes.
 *
 * Sleeping step mode: In this mode the device will be by default in sleep.
 * On request (soc bit), the device will power-on, the state machine
 * will do one measurement, will go into sleep and will wait for next command.
 *
 * Step mode: In this mode the state machine will do one measurement upon
 * request (soc bit) and will wait for next command.
 * The device remains powered all time in this mode.
 *
 * Continuous mode: Measurements are executed continuously. The device
 * remains powered all time in this mode.
 *
 * @retval  1: Enables the sleeping step mode.
 * @retval  2: Enables the step mode.
 * @retval  3: Device is in continuous mode.
 * @retval <0 Something went wrong.
 ******************************************************************************/
uint8_t getMode(void);

/***************************************************************************//**
 * @brief Set CONTINUOUS measurement mode.
 *
 * Continuous mode: Measurements are executed continuously. The device
 * remains powered all time in this mode.
 *
 * @retval  0 Successfully read and write values.
 * @retval <0 Something went wrong.
 ******************************************************************************/
int32_t setMode_Continuous(void);

/***************************************************************************//**
 * @brief Delay, blocking.
 *
 * @param[in] value (0-32,767).
 ******************************************************************************/
void sleep(int value);

/***************************************************************************//**
 * @brief Assert REG_STATUS and give back
 *        the position of the measurement (Cycle_pos).
 *
 * @return  1: Cycle_pos = 1.
 * @return  2: Cycle_pos = 2.
 * @retval <0 Something went wrong.
 ******************************************************************************/
int mlx90632_start_measurement(void);

/***************************************************************************//**
 * @brief Set helper number for the RAM_MEAS address-es for each channel.
 *
 * @param[in]  ret should get the Cyclic_pos which can be 1 or 2.
 * @param[out] *channel_new Pointer to memory location
 *             where channel_new value is stored.
 * @param[out] *channel_old Pointer to memory location
 *             where channel_old value is stored.
 *
 * @retval  0 Successfully set.
 * @retval <0 Something went wrong.
 ******************************************************************************/
int32_t mlx90632_channel_new_select(int32_t ret, uint8_t *channel_new,
                                    uint8_t *channel_old);

/***************************************************************************//**
 * @brief Read object raw old and new values based on
 *        mlx90632_start_measurement retval value.
 *
 * @param[in]  start_measurement_ret should get the Cyclic_pos which can be 1/2.
 * @param[out] *object_new_raw Pointer
 *             to memory location where new ambient value from sensor is stored.
 * @param[out] *object_old_raw Pointer
 *             to memory location where old ambient value from sensor is stored.
 *
 * @retval  0 Successfully read both values.
 * @retval <0 Something went wrong.
 ******************************************************************************/
int32_t mlx90632_read_temp_object_raw(int32_t start_measurement_ret,
                                      int16_t *object_new_raw,
                                      int16_t *object_old_raw);

/***************************************************************************//**
 * @brief Read new and old  ambient and object values from sensor.
 *
 * @param[out] *ambient_new_raw Pointer
 *             to memory location where new ambient value from sensor is stored.
 * @param[out] *ambient_old_raw Pointer
 *             to memory location where old ambient value from sensor is stored.
 * @param[out] *object_new_raw Pointer
 *             to memory location where new ambient value from sensor is stored.
 * @param[out] *object_old_raw Pointer
 *             to memory location where old ambient value from sensor is stored.
 *
 * @retval  0 Successfully read the values.
 * @retval <0 Something went wrong.
 ******************************************************************************/
int32_t mlx90632_read_temp_raw(int16_t *ambient_new_raw,
                               int16_t *ambient_old_raw,
                               int16_t *object_new_raw,
                               int16_t *object_old_raw);
/***************************************************************************//**
 * @brief Pre-calculations for object temperature.
 *
 * @param[in] object_new_raw value.
 * @param[in] object_old_raw value.
 * @param[in] ambient_new_raw value.
 * @param[in] ambient_old_raw value.
 * @param[in] Ka calibration constant.
 *
 * @retval Calculated STO value.
 ******************************************************************************/
double mlx90632_preprocess_temp_object(int16_t object_new_raw,
                                       int16_t object_old_raw,
                                       int16_t ambient_new_raw,
                                       int16_t ambient_old_raw,
                                       int16_t Ka);

/***************************************************************************//**
 * @brief Calculations for object temperature.
 *
 * @param[in] prev_object_temp should be 25.0 C.
 * @param[in] object STO value.
 * @param[in] TAdut value.
 * @param[in] Ga calibration constant.
 * @param[in] Fa calibration constant.
 * @param[in] Fb calibration constant.
 * @param[in] Ha calibration constant.
 * @param[in] Hb calibration constant.
 * @param[in] emissivity, default 1.0.
 *
 * @retval Calculated object temperature value in Celsius.
 ******************************************************************************/
double mlx90632_calc_temp_object_iteration(double prev_object_temp,
                                           int32_t object, double TAdut,
                                           int32_t Ga, int32_t Fa,
                                           int32_t Fb, int16_t Ha,
                                           int16_t Hb, double emissivity);

/***************************************************************************//**
 * @brief Calculation of TAdut and iterate calculations.
 *
 * @param[in] object STO value.
 * @param[in] ambient AMB value.
 * @param[in] Ea calibration constant.
 * @param[in] Eb calibration constant.
 * @param[in] Ga calibration constant.
 * @param[in] Fa calibration constant.
 * @param[in] Fb calibration constant.
 * @param[in] Ha calibration constant.
 * @param[in] Hb calibration constant.
 *
 * @retval Calculated object temperature value in Celsius.
 ******************************************************************************/
double mlx90632_calc_temp_object(int32_t object, int32_t ambient,
                                 int32_t Ea, int32_t Eb, int32_t Ga,
                                 int32_t Fa, int32_t Fb,
                                 int16_t Ha, int16_t Hb);
/***************************************************************************//**
 * @brief Function for device reset.
 *
 * @retval  0 Successfully.
 * @retval <0 Something went wrong.
 ******************************************************************************/
int32_t mlx90632_addressed_reset(void);

/***************************************************************************//**
 * @brief  Function for unlock EEPROM.
 *
 * @retval  0 Successfully unlock the EEPROM.
 * @retval <0 Something went wrong.
 ******************************************************************************/
int32_t mlx90632_unlock_eeprom(void);

/***************************************************************************//**
 * @brief  Function gives back both temperature values. Contains all necessary
 *         functions for the measurement.
 *
 * @param[out] *ambient Pointer
 *             to memory location where ambient temperature value is stored.
 * @param[out] *object Pointer
 *             to memory location where object temperature value is stored.
 *
 * @retval  0 Successfully get temperature values.
 * @retval <0 Something went wrong.
 ******************************************************************************/
int32_t measurment_cb(double *ambient, double *object);

#endif // MLX90632_H

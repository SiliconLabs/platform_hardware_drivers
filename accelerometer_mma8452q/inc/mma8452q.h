/***************************************************************************//**
 * @file mma8452q.h
 * @brief MMA8452Q Prototypes
 * @version 1.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * This code has been minimally tested to ensure that it builds with the
 * specified dependency versions and is suitable as a demonstration for
 * evaluation purposes only.
 * This code will be maintained at the sole discretion of Silicon Labs.
 *
 ******************************************************************************/

#ifndef MMA8452Q_H_
#define MMA8452Q_H_
#include "sl_status.h"
#include "sl_i2cspm.h"

#include <stdint.h>
#include <stddef.h>
#include "gpiointerrupt.h"
#include "em_gpio.h"
#include "sl_status.h"
#include "sl_i2cspm.h"
#if (defined(SL_CATALOG_POWER_MANAGER_PRESENT))
#include "sl_power_manager.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup mma8452q MMA8452Q Driver API*/

/***************************************************************************//**
 * @addtogroup mma8452q
 * @brief  MMA8452Q Driver API
 * @details
 * @{
 ******************************************************************************/

/***************************************************************************//**
* @name    SOFTWARE VERSION DEFINITION
*******************************************************************************/
#define MMA8452Q_MAJOR_VERSION                0
#define MMA8452Q_MINOR_VERSION                1
#define MMA8452Q_BUILD_VERSION                0
#define MMA8452Q_REVISION_VERSION             0

/***************************************************************************//**
* @name    CHIP ID DEFINITION
*******************************************************************************/
#define SL_MMA8452Q_DEVICE_ID                 0x2A    /**< Device ID of the
                                                       *   MMA8452Q chip
                                                       *          */

/***************************************************************************//**
* @name    I2C ADDRESS DEFINITION
*******************************************************************************/
#define SL_MMA8452Q_I2C_BUS_ADDRESS1          0x1C    /**< I2C address of the
                                                       *   MMA8452Q chip: SA0
                                                       *   low-logic */
#define SL_MMA8452Q_I2C_BUS_ADDRESS2          0x1D    /**< I2C address of the
                                                       *   MMA8452Q chip: SA0
                                                       *   hi-logic  */

/***************************************************************************//**
 * @brief
 *    Typedef for full-scale settings.
 ******************************************************************************/
typedef enum mma8452q_scale
{
  MMA8452Q_SCALE_2G = 2,
  MMA8452Q_SCALE_4G = 4,
  MMA8452Q_SCALE_8G = 8
} mma8452q_scale_t;

/***************************************************************************//**
 * @brief
 *    Typedef for data rates settings.
 ******************************************************************************/
typedef enum mma8452q_odr
{
  MMA8452Q_ODR_800,
  MMA8452Q_ODR_400,
  MMA8452Q_ODR_200,
  MMA8452Q_ODR_100,
  MMA8452Q_ODR_50,
  MMA8452Q_ODR_12,
  MMA8452Q_ODR_6,
  MMA8452Q_ODR_1
} mma8452q_odr_t;

typedef enum
{
  MMA8452Q_ASLP_ODR_50,
  MMA8452Q_ASLP_ODR_12p5,
  MMA8452Q_ASLP_ODR_6p25,
  MMA8452Q_ASLP_ODR_1p56,
} mma8452q_aslp_odr_t;

typedef enum
{
  MMA8452Q_NORMAL,
  MMA8452Q_LNOISE_LPWR,
  MMA8452Q_HI_RES,
  MMA8452Q_LPWR,
} mma8452q_mods_t; // mode power scheme

/***************************************************************************//**
* @name    MMA8452Q portrait/landscape & back/front settings
*******************************************************************************/
#define MMA8452Q_BAFRO_FRONT 0
#define MMA8452Q_BAFRO_BACK  1
#define MMA8452Q_PORTRAIT_U  0
#define MMA8452Q_PORTRAIT_D  1
#define MMA8452Q_LANDSCAPE_R 2
#define MMA8452Q_LANDSCAPE_L 3
#define MMA8452Q_LOCKOUT     0x40

typedef enum
{
  MMA8452Q_PORTRAIT_UP,
  MMA8452Q_PORTRAIT_DOWN,
  MMA8452Q_LANDSCAPE_RIGHT,
  MMA8452Q_LANDSCAPE_LEFT,
  MMA8452Q_BACK,
  MMA8452Q_FRONT,
  MMA8452Q_ZLOCKOUT,
} mma8452q_orientation_t;

/***************************************************************************//**
 * @brief
 *    Typedef for specifying the software version of the core driver.
 ******************************************************************************/
typedef struct
{
  uint8_t major;  /*!< major number */
  uint8_t minor;  /*!< minor number */
  uint8_t build;  /*!< build number */
  uint32_t revision;  /*!< revision number */
} mma8542q_core_version_t;

/***************************************************************************//**
 * @brief
 *    Structure to store the sensor auto-sleep fxn configuration
 ******************************************************************************/
typedef struct {
  uint8_t alsp_count;       // min time period to transition to sleep
} mma8452q_aslp_config_t;

/***************************************************************************//**
 * @brief
 *    Structure to store the sensor transient fxn configuration
 ******************************************************************************/
typedef struct {
  bool en_event_latch;      // enable flag latch to TRANSIENT_SRC register
  bool en_z_trans;          // enable Z event when gt Z threshold
  bool en_y_trans;          // enable Y event when gt Y threshold
  bool en_x_trans;          // enable X event when gt X threshold
  bool en_hpf_bypass;       // bypass high pass filter
  bool db_cnt_mode;         // debounce counter mode [0:inc/dec, 1:inc/clr]
  uint8_t threshold;        // transient threshold (one threshold for all 3
                            //   axes)
  uint8_t debounce_cnt;     // min debounce counts
} mma8452q_trans_config_t;

/***************************************************************************//**
 * @brief
 *    Structure to store the sensor orientation fxn configuration
 ******************************************************************************/
typedef struct {
  bool db_cnt_mode;         // debounce counter mode [0:dec, 1:clr]
  bool en_event_latch;      // detection enable
  uint8_t debounce_cnt;     // min debounce counts
} mma8452q_orientation_config_t;

/***************************************************************************//**
 * @brief
 *    Structure to store the sensor pulse fxn configuration
 ******************************************************************************/
typedef struct {
  bool double_abort;        // abort double pulse detection after pulse latency
  bool en_event_latch;      // enable flag latch to PULSE_SRC register
  bool en_z_double;         // enable Z double pulse
  bool en_z_single;         // enable Z single pulse
  bool en_y_double;         // enable Y double pulse
  bool en_y_single;         // enable Y single pulse
  bool en_x_double;         // enable X double pulse
  bool en_x_single;         // enable X single pulse
  uint8_t pulse_thresh_z;   // pulse threshold Z
  uint8_t pulse_thresh_y;   // pulse threshold Y
  uint8_t pulse_thresh_X;   // pulse threshold X
  uint8_t pulse_time_lmt;   // maximum interval from pulse gt <-> lt threshold
  uint8_t pulse_latency;    // ignore interval btwn 1st and 2nd pulse
  uint8_t pulse_window;     // max interval after latency interval for 2nd pulse
} mma8452q_pulse_config_t;

/***************************************************************************//**
 * @brief
 *    Structure to store the sensor freefall/motion configuration
 ******************************************************************************/
typedef struct {
  bool en_event_latch;      // enable flag latch to FF_MT_SRC register
  bool ff_mt_sel;           // seletion btwn FF/MT [0:FF, 1:MT]
  bool en_z_trans;          // enable Z event when gt Z threshold
  bool en_y_trans;          // enable Y event when gt Y threshold
  bool en_x_trans;          // enable X event when gt X threshold
  bool db_cnt_mode;         // debounce counter mode [0:inc/dec, 1:inc/clr]
  uint8_t threshold;        // transient threshold (one threshold for all 3
                            //   axes)
  uint8_t debounce_cnt;     // min debounce counts
} mma8452q_ff_mt_config_t;

/***************************************************************************//**
 * @brief
 *    Structure to store the sensor interrupt(s) configuration
 ******************************************************************************/
typedef struct {
  bool en_trans_wake;          // enable transient auto-sleep wake
  bool en_orientation_wake;    // enable orientation auto-sleep wake
  bool en_pulse_wake;          // enable pulse auto-sleep wake
  bool en_ff_mt_wake;          // enable freefall/motion auto-sleep wake
  bool int_active_hi;          // interrupt polarity [0:active_low, 1:active_hi]
  bool int_open_drain;         // interrupt pin cfg [0:push-pull, 1:open-drain]
  bool en_aslp_int;            // auto-sleep transition interrupt
  bool en_trans_int;           // transient interrupt
  bool en_orientation_int;     // orientation interrupt
  bool en_pulse_int;           // pulse transition interrupt
  bool en_ff_mt_int;           // freefall/motion interrupt
  bool en_drdy_int;            // data ready interrupt
  bool cfg_aslp_int;           // interrupt routed to INT1 [0:INT2, 1:INT1]
  bool cfg_trans_int;          // interrupt routed to INT1 [0:INT2, 1:INT1]
  bool cfg_orientation_int;    // interrupt routed to INT1 [0:INT2, 1:INT1]
  bool cfg_pulse_int;          // interrupt routed to INT1 [0:INT2, 1:INT1]
  bool cfg_ff_mt_int;          // interrupt routed to INT1 [0:INT2, 1:INT1]
  bool cfg_drdy_int;           // interrupt routed to INT1 [0:INT2, 1:INT1]
} mma8452q_interrupt_config_t;

/***************************************************************************//**
 * @brief
 *    Structure to store the sensor configuration
 ******************************************************************************/
typedef struct {
  uint8_t                       dev_addr;           // configured device
                                                    //   I2C address
  mma8452q_scale_t              scale;              // accelerometer scale
  mma8452q_odr_t                odr;                // output data rate
                                                    //   when awake
  mma8452q_aslp_odr_t           alsp_rate;          // output data rate
                                                    //   when asleep
  bool                          en_low_noise;       // enable reduced
                                                    //   noise mode
  bool                          en_fast_read;       // enable fast-read
                                                    //   mode
  mma8452q_mods_t               active_mode_pwr;    // active mode power
                                                    //   scheme
  mma8452q_mods_t               slp_mode_pwr;       // sleep mode power
                                                    //   scheme
  mma8452q_aslp_config_t        aslp_cfg;
  mma8452q_trans_config_t       trans_cfg;
  mma8452q_orientation_config_t orient_cfg;
  mma8452q_pulse_config_t       pulse_cfg;
  mma8452q_ff_mt_config_t       ff_mt_cfg;
  mma8452q_interrupt_config_t   interrupt_cfg;
  bool                          enable;             // set to active mode
} mma8452q_sensor_config_t;

#define MMA8452Q_ENUM(name) typedef uint8_t name; enum name ## _enum

/***************************************************************************//**
 * @brief
 *    MMA8452Q interrupt source enum
 ******************************************************************************/
MMA8452Q_ENUM(sl_mma8452q_irq_source_t) {
  MMA8452Q_IRQ_SRC_DRDY = 0,
  MMA8452Q_IRQ_SRC_FF_MT = 1,
  MMA8452Q_IRQ_SRC_PULSE = 2,
  MMA8452Q_IRQ_SRC_LNDPRT = 3,
  MMA8452Q_IRQ_SRC_TRANS = 4,
  MMA8452Q_IRQ_SRC_ASLP = 5,
};

/***************************************************************************//**
 * @name  MMA8452Q interrupt source mask
 ******************************************************************************/
#define MMA8452Q_MASK_IRQ_SRC_DRDY       (1 << 0) //  Data Ready Interrupt bit
                                                  //   status
#define MMA8452Q_MASK_IRQ_SRC_FF_MT      (1 << 2) //  Freefall/Motion interrupt
                                                  //   status bit
#define MMA8452Q_MASK_IRQ_SRC_PULSE      (1 << 3) //  Pulse interrupt status bit
#define MMA8452Q_MASK_IRQ_SRC_LNDPRT     (1 << 4) //  Landscape/Portrait
                                                  //   Orientation interrupt
                                                  //   status bit
#define MMA8452Q_MASK_IRQ_SRC_TRANS      (1 << 5) //  Transient interrupt status
                                                  //   bit
#define MMA8452Q_MASK_IRQ_SRC_ASLP       (1 << 7) //  Auto-SLEEP/WAKE interrupt
                                                  //   status bit

/***************************************************************************//**
 * @name  MMA8452Q PL status mask
 ******************************************************************************/
#define MMA8452Q_MASK_PL_BAFRO           (1 << 0) //  Back or Front orientation
#define MMA8452Q_MASK_PL_LAPO_0          (1 << 2) //  Landscape/Portrait
                                                  //   orientation
#define MMA8452Q_MASK_PL_LAPO_1          (1 << 3) //  Landscape/Portrait
                                                  //   orientation
#define MMA8452Q_MASK_PL_LO              (1 << 6) //  Z-Tilt Angle Lockout
#define MMA8452Q_MASK_PL_NEWLP           (1 << 7) //  Landscape/Portrait status
                                                  //   change flag

/***************************************************************************//**
 * @name  MMA8452Q FF_MT source mask
 ******************************************************************************/
#define MMA8452Q_MASK_FF_MT_XHP          (1 << 0) //  X Motion Polarity Flag
#define MMA8452Q_MASK_FF_MT_XHE          (1 << 1) //  X Motion Flag
#define MMA8452Q_MASK_FF_MT_YHP          (1 << 2) // Y Motion Polarity Flag
#define MMA8452Q_MASK_FF_MT_YHE          (1 << 3) //  Y Motion Flag
#define MMA8452Q_MASK_FF_MT_ZHP          (1 << 4) //  Z Motion Polarity Flag
#define MMA8452Q_MASK_FF_MT_ZHE          (1 << 5) //  Z Motion Flag
#define MMA8452Q_MASK_FF_MT_EA           (1 << 7) //  Event Active Flag

/***************************************************************************//**
 * @name  MMA8452Q Trans source mask
 ******************************************************************************/
#define MMA8452Q_MASK_TRANS_X_TRANS_POL  (1 << 0) //  Polarity of X Transient
                                                  //   Event that triggered
                                                  //   interrupt
#define MMA8452Q_MASK_TRANS_XTRANSE      (1 << 1) //  X transient event
#define MMA8452Q_MASK_TRANS_Y_TRANS_POL  (1 << 2) //  Polarity of Y Transient
                                                  //   Event that triggered
                                                  //   interrupt
#define MMA8452Q_MASK_TRANS_YTRANSE      (1 << 3) //  Y transient event
#define MMA8452Q_MASK_TRANS_Z_TRANS_POL  (1 << 4) //  Polarity of Z Transient
                                                  //   Event that triggered
                                                  //   interrupt
#define MMA8452Q_MASK_TRANS_ZTRANSE      (1 << 5) //  Z transient event
#define MMA8452Q_MASK_TRANS_EA           (1 << 6) //  Event Active Flag

/***************************************************************************//**
 * @name  MMA8452Q Pulse source mask
 ******************************************************************************/
#define MMA8452Q_MASK_PULSE_POLX         (1 << 0) //  Pulse polarity of X-axis
                                                  //   Event
#define MMA8452Q_MASK_PULSE_POLY         (1 << 1) //  Pulse polarity of Y-axis
                                                  //   Event
#define MMA8452Q_MASK_PULSE_POLZ         (1 << 2) //  Pulse polarity of Z-axis
                                                  //   Event
#define MMA8452Q_MASK_PULSE_DPE          (1 << 3) //  Double pulse on first
                                                  //   event
#define MMA8452Q_MASK_PULSE_AXX          (1 << 4) //  X-axis even
#define MMA8452Q_MASK_PULSE_AXY          (1 << 5) //  Y-axis even
#define MMA8452Q_MASK_PULSE_AXZ          (1 << 6) //  Z-axis even
#define MMA8452Q_MASK_PULSE_EA           (1 << 6) //  Event Active Flag

/***************************************************************************//**
 * @brief
 *   Returns the information of the current software information
 *
 * @param[out] core_version
 *   The struct contain the data of the software version information
 ******************************************************************************/
void mma8452q_get_core_version(mma8542q_core_version_t *core_version);

/***************************************************************************//**
 * @brief
 *   Returns the raw acceleration data as read from MMA8452Q OUT_X,Y,Z
 *   registers. This function verifies the driver configuration fast-read mode
 *   to read data registers via block read, skipping LSBs when appropriate. See
 *   MMA8452Q datasheet for more details. Raw values do not take into account
 *   configured measurement range.
 *
 * @param[out] avec[3]
 *   2's complement 12-bit numbers representing measured acceleration OUT_X,Y,Z.
 *
 *  @note Assume that we store data in the 16-bit variables, if you want to use
 *  12-bit raw, please shift your variables to the right 4 value.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_TRANSMIT if I2C transmit failed.
 *
 *   @li @ref SL_INVALID_PARAMETER if invalid parameter.
 ******************************************************************************/
sl_status_t sl_mma8452q_get_acceleration(int16_t avec[3]);

/***************************************************************************//**
 * @brief
 *   Returns calculated acceleration data computed using MMA8452Q OUT_X,Y,Z
 *   register values with driver configured scale. This function verifies the
 *   driver configuration fast-read mode to read data registers via block read,
 *   skipping LSBs when appropriate. See MMA8452Q datasheet for more details.
 *
 * @param[out] avec[3]
 *   float numbers representing scaled acceleration OUT_X,Y,Z in units of g's.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_TRANSMIT if I2C transmit failed.
 *
 *   @li @ref SL_INVALID_PARAMETER if invalid parameter.
 ******************************************************************************/
sl_status_t sl_mma8452q_get_calculated_acceleration(float avec[3]);

/***************************************************************************//**
 * @brief
 *    This function reads the data STATUS register and returns raw 8-bit value.
 *
 * @param[out] data_status
 *    8-bit status register
 *
 * @return
 *    @li @ref SL_STATUS_OK if there are no errors.
 *
 *    @li @ref SL_STATUS_TRANSMIT if I2C transmit failed.
 *
 *    @li @ref SL_INVALID_PARAMETER if invalid parameter.
 ******************************************************************************/
sl_status_t mma8452q_get_data_status(uint8_t *data_status);

/***************************************************************************//**
 * @brief
 *    This function reads the data STATUS register and checks if the new
 *    data is available.
 *
 * @param[out] is_data_ready
 *    Data ready. Valid values: [0=No;1=Yes]
 *
 * @return
 *    @li @ref SL_STATUS_OK if there are no errors.
 *
 *    @li @ref SL_STATUS_TRANSMIT if I2C transmit failed.
 *
 *    @li @ref SL_INVALID_PARAMETER if invalid parameter.
 ******************************************************************************/
sl_status_t mma8452q_check_for_data_ready(uint8_t *is_data_ready);

/***************************************************************************//**
 * @brief
 *    This function checks the SYSMODE register, indicating the current device
 *    operating mode.
 *
 * @param[out] reg_sysmode
 *    SYSMODE. Valid values: [0x0=Standby mode;0x01=Wake mode;0x02=Sleep mode]
 *
 * @return
 *    @li @ref SL_STATUS_OK if there are no errors.
 *
 *    @li @ref SL_STATUS_TRANSMIT if I2C transmit failed.
 *
 *    @li @ref SL_INVALID_PARAMETER if invalid parameter.
 ******************************************************************************/
sl_status_t mma8452q_get_sysmode(uint8_t *reg_sysmode);

/***************************************************************************//**
 * @brief
 *    This function checks the interrupt source register. Bits are set
 *    indicating which function have asserted an interrupt. Interrupt flags are
 *    cleared by reading the appropriate interrupt source register.
 *
 * @param[out] int_source
 *    Interrupt source; used by interrupt service routine to read appropriate
 *    interrupt source register. Bits represent assertion of an interrupt fxn:
 *      Bit 7 - Auto-sleep/wake transition
 *      Bit 6 - DONT CARE; always 0
 *      Bit 5 - Acceleration transient greater than threshold
 *      Bit 4 - Change in orientation status
 *      Bit 3 - Single/double pulse event
 *      Bit 2 - Freefall/motion interrupt
 *      Bit 1 - DONT CARE; always 0
 *      Bit 0 - Presence of new data
 *
 * @return
 *    @li @ref SL_STATUS_OK if there are no errors.
 *
 *    @li @ref SL_STATUS_TRANSMIT if I2C transmit failed.
 *
 *    @li @ref SL_INVALID_PARAMETER if invalid parameter.
 ******************************************************************************/
sl_status_t mma8452q_get_int_source(uint8_t *int_source);

/***************************************************************************//**
 * @brief
 *    This function reads the portrait/landscape status register of the
 *   MMA8452Q.
 *    Will return either MMA8452Q_PORTRAIT_U, MMA8452Q_PORTRAIT_D,
 *    MMA8452Q_LANDSCAPE_R, MMA8452Q_LANDSCAPE_L, or MMA8452Q_LOCKOUT.
 *    MMA8452Q_LOCKOUT indicates that the sensor is in neither p or ls.
 *
 * @param[out] pl_status
 *    Portrait/Landscape orientation status
 *      Bit 7 - Landscape/Portrait status change flag
 *      Bit 6 - Z-Tilt Angle Lockout
 *      Bit 5 - DONT CARE; always 0
 *      Bit 4 - DONT CARE; always 0
 *      Bit 3 - DONT CARE; always 0
 *      Bit 2 - Landscape/Portrait orientation
 *      Bit 1 - Landscape/Portrait orientation
 *      Bit 0 - Back or Front orientation
 *
 * @return
 *    @li @ref SL_STATUS_OK if there are no errors.
 *
 *    @li @ref SL_STATUS_TRANSMIT if I2C transmit failed.
 *
 *    @li @ref SL_INVALID_PARAMETER if invalid parameter.
 ******************************************************************************/
sl_status_t mma8452q_get_pl_status(uint8_t *pl_status);

/***************************************************************************//**
 * @brief
 *    This function reads the freefall/motion status register of the MMA8452Q.
 *
 * @param[out] ff_mt_status
 *    Freefall/motion event/flag status
 *      Bit 7 - Event Active Flag
 *      Bit 6 - DONT CARE; always 0
 *      Bit 5 - Z Motion Flag
 *      Bit 4 - Z Motion Polarity Flag
 *      Bit 3 - Y Motion Flag
 *      Bit 2 - Y Motion Polarity Flag
 *      Bit 1 - X Motion Flag
 *      Bit 0 - X Motion Polarity Flag
 *
 * @return
 *    @li @ref SL_STATUS_OK if there are no errors.
 *
 *    @li @ref SL_STATUS_TRANSMIT if I2C transmit failed.
 *
 *    @li @ref SL_INVALID_PARAMETER if invalid parameter.
 ******************************************************************************/
sl_status_t mma8452q_get_ff_mt_status(uint8_t *ff_mt_status);

/***************************************************************************//**
 * @brief
 *    This function reads the transient status register of the MMA8452Q.
 *
 * @param[out] transient_status
 *    Transient event/flag status
 *      Bit 7 - DONT CARE; always 0
 *      Bit 6 - Event Active Flag
 *      Bit 5 - Z transient event
 *      Bit 4 - Polarity of Z Transient Event that triggered interrupt
 *      Bit 3 - Y transient event
 *      Bit 2 - Polarity of Y Transient Event that triggered interrupt
 *      Bit 1 - X transient event
 *      Bit 0 - Polarity of X Transient Event that triggered interrupt
 *
 * @return
 *    @li @ref SL_STATUS_OK if there are no errors.
 *
 *    @li @ref SL_STATUS_TRANSMIT if I2C transmit failed.
 *
 *    @li @ref SL_INVALID_PARAMETER if invalid parameter.
 ******************************************************************************/
sl_status_t mma8452q_get_transient_status(uint8_t *transient_status);

/***************************************************************************//**
 * @brief
 *    This function reads the pulse status register of the MMA8452Q.
 *
 * @param[out] pulse_status
 *    Pulse detection event/flag status
 *      Bit 7 - Event Active Flag
 *      Bit 6 - Z-axis event
 *      Bit 5 - Y-axis event
 *      Bit 4 - X-axis event
 *      Bit 3 - Double pulse on first event
 *      Bit 2 - Pulse polarity of Z-axis Event
 *      Bit 1 - Pulse polarity of Y-axis Event
 *      Bit 0 - Pulse polarity of X-axis Event
 *
 * @return
 *    @li @ref SL_STATUS_OK if there are no errors.
 *
 *    @li @ref SL_STATUS_TRANSMIT if I2C transmit failed.
 *
 *    @li @ref SL_INVALID_PARAMETER if invalid parameter.
 ******************************************************************************/
sl_status_t mma8452q_get_pulse_status(uint8_t *pulse_status);

/***************************************************************************//**
 * @brief
 *   Initialize the MMA8452Q driver with the values provided in the
 *   mma8452q_config.h file.
 *
 * @param[in] i2cspm
 *   The I2C peripheral to use.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_ALREADY_INITIALIZED the function has previously
 *     been called.
 *
 *   @li @ref SL_STATUS_INVALID_PARAMETER invalid parameters.
 *
 ******************************************************************************/
sl_status_t mma8452q_init(sl_i2cspm_t *i2cspm);

/***************************************************************************//**
 * @brief
 *   De-initalize the MMA8452Q driver.
 *
 * @param[in] i2cspm
 *   The I2C peripheral to use.
 *
 * @details
 *
 *   De-initialization of the MMA8452Q resets the device settings and places
 *   the device in standby mode, disabling the analog subsystem and internal
 *   clocks.
 *
 * @note
 *   The purpose of de-initialization is to shutdown the MMA8452Q as part
 *   of a complete shutdown sequence for an EFM32/EFR32-based system.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_NOT_INITIALIZED if the driver is not initialized.
 *
 *   @li @ref SL_STATUS_NOT_FOUND if the device is not found
 ******************************************************************************/
sl_status_t mma8452q_deinit(void);

/***************************************************************************//**
 * @brief
 *   Set MMA8452Q I2C address settings and confirm WHO_AM_I register. Check that
 *   provide parameter is one of the two valid I2C addresses.
 *
 * @param[in] i2c_address
 *    Device I2C address. (Default: 0x38[0x1C])
 *
 * @return
 *    @li @ref SL_STATUS_OK if there are no errors.
 *
 *    @li @ref SL_STATUS_TRANSMIT if I2C transmit failed.
 *
 *    @li @ref SL_INVALID_PARAMETER if invalid parameter.
 ******************************************************************************/
sl_status_t mma8452q_set_address(uint8_t i2c_address);

/***************************************************************************//**
 * @brief
 *   This function sets the full-scale range of the x, y, and z axis
 *   accelerometers.
 *
 * @param[in] scale
 *    Full-scale range; mma8452q_scale_t lists possible values
 *
 * @return
 *    @li @ref SL_STATUS_OK if there are no errors.
 *
 *    @li @ref SL_STATUS_INVALID_PARAMETER if scale is invalid.
 *
 *    @li @ref SL_STATUS_TRANSMIT if I2C transmit failed.
 *
 *    @li @ref SL_STATUS_NOT_FOUND if the device is not found
 ******************************************************************************/
sl_status_t mma8452q_set_scale(mma8452q_scale_t scale);

/***************************************************************************//**
 * @brief
 *   This function sets the output data rate of the MMA8452Q.
 *
 * @param[in] odr
 *    Output data rate; mma8452q_odr_t lists possible values
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *
 *    SL_STATUS_INVALID_PARAMETER if odr is invalid.
 *
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_set_odr(mma8452q_odr_t odr);

/***************************************************************************//**
 * @brief
 *   This function sets active power mode of the MMA8452Q.
 *
 * @param[in] mods
 *    Active power mode scheme
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *
 *    SL_STATUS_INVALID_PARAMETER if mods is invalid.
 *
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_set_mods(mma8452q_mods_t mods);

/***************************************************************************//**
 * @brief
 *   This function sets the auto-sleep output data rate of the MMA8452Q.
 *
 * @param[in] aslp_rate
 *    Sleep output data rate; mma8452q_aslp_odr_t lists possible values
 *
 * @param[in] slp_mode_pwr
 *    Sleep power mode scheme;  MMA8452Q_SMODS_TypeDef lists possible values
 *
 * @param[in] aslp_cfg
 *    A pointer to additional auto-sleep mode settings.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *
 *    SL_STATUS_INVALID_PARAMETER if aslp_rate, slp_mode_pwr are invalid.
 *
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_config_aslp(mma8452q_aslp_odr_t alsp_rate,
                                 mma8452q_mods_t slp_mode_pwr,
                                 mma8452q_aslp_config_t aslp_cfg);

/***************************************************************************//**
 * @brief
 *   This function configures the transient function settings of the MMA8452Q.
 *
 * @param[in] trans_cfg
 *    A pointer to transient function settings.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_config_trans(mma8452q_trans_config_t *trans_cfg);

/***************************************************************************//**
 * @brief
 *   This function configures the orientation function settings of the MMA8452Q.
 *
 * @param[in] orient_cfg
 *    A pointer to orientation function settings.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_config_orientation(
  mma8452q_orientation_config_t orient_cfg);

/***************************************************************************//**
 * @brief
 *   This function configures the pulse function settings of the MMA8452Q.
 *
 * @param[in] pulse_cfg
 *    A pointer to pulse function settings.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_config_pulse(mma8452q_pulse_config_t *pulse_cfg);

/***************************************************************************//**
 * @brief
 *   This function configures the freefall/motion function settings of the
 *   MMA8452Q.
 *
 * @param[in] ff_mt_cfg
 *    A pointer to freefall/motion function settings.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_config_ff_mt(mma8452q_ff_mt_config_t *ff_mt_cfg);

/***************************************************************************//**
 * @brief
 *   This function configures the interrupt outputs for the MMA8452Q.
 *
 * @param[in] interrupt_cfg
 *    A pointer to interrupt function settings.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_config_interrupt(
  mma8452q_interrupt_config_t *interrupt_cfg);

/***************************************************************************//**
 * @brief
 *   This function configures the reduced noise mode setting for the MMA8452Q.
 *
 * @param[in] enable
 *   If true, enables reduced noise mode, if false sets to normal operation mode
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_enable_low_noise(bool enable);

/***************************************************************************//**
 * @brief
 *   This function configures the fast-read mode for the MMA8452Q.
 *   Configure for 8-bit or 12-bit operation of the sensor.
 *
 * @param[in] enable
 *   If true, enables fast-read mode, if false sets to normal operation mode
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_enable_fast_read(bool enable);

/***************************************************************************//**
 * @brief
 *   This function puts the MMA8452Q device in active or standby mode.
 *
 * @param[in] enable
 *   If true, enables active mode, if false puts device in standby mode
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_active(bool enable);

/***************************************************************************//**
 * @brief
 *   This function performs calibration steps and stores new offset information
 *   internally on the MMA8452Q device. Caution should be used when performing
 *   this function as this should be done with device in specific (FRONT)
 *   orientation.
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 *
 *    SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t mma8452q_auto_calibrate(void);

/** @} (end addtogroup mma8452q) */
#ifdef __cplusplus
}
#endif
#endif /* MMA8452Q_H_ */

/***************************************************************************//**
 * @file vcnl4040.h
 * @brief VCNL4040 Proximity sensor
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
 * # Evaluation Quality
 * This code has been minimally tested to ensure that it builds and is suitable
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/
#ifndef VCNL4040_H_
#define VCNL4040_H_

#include <stdbool.h>
#include <stdint.h>
#include "sl_status.h"
#include "vcnl4040_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup VCNL4040 VCNL4040 */

/***************************************************************************//**
 * @addtogroup VCNL4040
 * @brief  VCNL4040 Proximity Hardware Driver
 * @details
 * @{
 *
 *
 *    @n @section vcnl4040_example VCNL4040 example
 *
 *      Basic example for performing measurement: @n @n
 *      @code{.c}
 *
 * #include "sl_i2cspm_instances.h"
 * #include "vcnl4040.h"
 *    int main(void)
 *    {
 *      ...
 *      uint16_t id;
 *      uint16_t prox;
 *      uint16_t ambient;
 *      uint16_t white;
 *
 *      // Initialize the vcnl4040
 *      vcnl4040_init(sl_i2cspm_qwiic);
 *
 *      // Read the ID
 *      vcnl4040_get_id(&id);
 *
 *      // Read the proximity value
 *      vcnl4040_get_proximity(&prox);
 *
 *      // Read the ambient light value
 *      vcnl4040_get_ambient(&ambient);
 *
 *      // Read the white light value
 *      vcnl4040_get_white(&white);
 *
 *
 *      ...
 *
 *    } @endcode
 *
 * @{
 ******************************************************************************/
// -----------------------------------------------------------------------------
//                               Macros
// -----------------------------------------------------------------------------

/************************************************/
/**\name    CHIP ID DEFINITION                  */
/************************************************/
#define SL_VCNL4040_DEVICE_ID                 0x0186   /**< Device ID of the VCNL4040 chip */

/************************************************/
/**\name    I2C ADDRESS DEFINITION       */
/***********************************************/
#define SL_VCNL4040_I2C_BUS_ADDRESS           0x60   /**< 7-bit unshifted I2C address of VCNL4040 */

/************************************************/
/**\name    VCNL4040 Command Codes       */
/***********************************************/
#define VCNL4040_ALS_CONF                     0x00
#define VCNL4040_ALS_THDH                     0x01
#define VCNL4040_ALS_THDL                     0x02
#define VCNL4040_PS_CONF1                     0x03 // Lower
#define VCNL4040_PS_CONF2                     0x03 // Upper
#define VCNL4040_PS_CONF3                     0x04 // Lower
#define VCNL4040_PS_MS                        0x04 // Upper
#define VCNL4040_PS_CANC                      0x05
#define VCNL4040_PS_THDL                      0x06
#define VCNL4040_PS_THDH                      0x07
#define VCNL4040_PS_DATA                      0x08
#define VCNL4040_ALS_DATA                     0x09
#define VCNL4040_WHITE_DATA                   0x0A
#define VCNL4040_INT_FLAG                     0x0B // Upper
#define VCNL4040_ID                           0x0C

#define VCNL4040_PS_DUTY_MASK                 ~((1 << 7) | (1 << 6))
#define VCNL4040_PS_DUTY_40                   0
#define VCNL4040_PS_DUTY_80                   (1 << 6)
#define VCNL4040_PS_DUTY_160                  (1 << 7)
#define VCNL4040_PS_DUTY_320                  ((1 << 7) | (1 << 6))

#define VCNL4040_PS_PERS_MASK                 ~((1 << 5) | (1 << 4))
#define VCNL4040_PS_PERS_1                    0
#define VCNL4040_PS_PERS_2                    (1 << 4)
#define VCNL4040_PS_PERS_3                    (1 << 5)
#define VCNL4040_PS_PERS_4                    ((1 << 5) | (1 << 4))

#define VCNL4040_ALS_PERS_MASK                ~((1 << 3) | (1 << 2))
#define VCNL4040_ALS_PERS_1                   0
#define VCNL4040_ALS_PERS_2                   (1 << 2)
#define VCNL4040_ALS_PERS_4                   (1 << 3)
#define VCNL4040_ALS_PERS_8                   ((1 << 3) | (1 << 2))

#define VCNL4040_ALS_INT_EN_MASK              ~(1 << 1)
#define VCNL4040_ALS_INT_DISABLE              0
#define VCNL4040_ALS_INT_ENABLE               (1 << 1)

#define VCNL4040_ALS_SD_MASK                  ~(1 << 0)
#define VCNL4040_ALS_SD_POWER_ON              0
#define VCNL4040_ALS_SD_POWER_OFF             (1 << 0)

#define VCNL4040_ALS_IT_MASK                  ~((1 << 7) | (1 << 6))
#define VCNL4040_ALS_IT_80MS                  0
#define VCNL4040_ALS_IT_160MS                 (1 << 6)
#define VCNL4040_ALS_IT_320MS                 (1 << 7)
#define VCNL4040_ALS_IT_640MS                 ((1 << 7) | (1 << 6))

#define VCNL4040_PS_IT_MASK                   ~((1 << 3) | (1 << 2) | (1 << 1))
#define VCNL4040_PS_IT_1T                     0
#define VCNL4040_PS_IT_15T                    (1 << 1)
#define VCNL4040_PS_IT_2T                     (1 << 2)
#define VCNL4040_PS_IT_25T                    ((1 << 2) | (1 << 1))
#define VCNL4040_PS_IT_3T                     (1 << 3)
#define VCNL4040_PS_IT_35T                    ((1 << 3) | (1 << 1))
#define VCNL4040_PS_IT_4T                     ((1 << 3) | (1 << 2))
#define VCNL4040_PS_IT_8T                     ((1 << 3) | (1 << 2) | (1 << 1))

#define VCNL4040_PS_SD_MASK                   ~(1 << 0)
#define VCNL4040_PS_SD_POWER_ON               0
#define VCNL4040_PS_SD_POWER_OFF              (1 << 0)

#define VCNL4040_PS_HD_MASK                   ~(1 << 3)
#define VCNL4040_PS_HD_12_BIT                 0
#define VCNL4040_PS_HD_16_BIT                 (1 << 3)

#define VCNL4040_PS_INT_MASK                  ~((1 << 1) | (1 << 0))
#define VCNL4040_PS_INT_DISABLE               0
#define VCNL4040_PS_INT_CLOSE                 (1 << 0)
#define VCNL4040_PS_INT_AWAY                  (1 << 1)
#define VCNL4040_PS_INT_BOTH                  ((1 << 1) | (1 << 0))

#define VCNL4040_PS_SMART_PERS_MASK           ~(1 << 4)
#define VCNL4040_PS_SMART_PERS_DISABLE        0
#define VCNL4040_PS_SMART_PERS_ENABLE         (1 << 1)

#define VCNL4040_PS_AF_MASK                   ~(1 << 3)
#define VCNL4040_PS_AF_DISABLE                0
#define VCNL4040_PS_AF_ENABLE                 (1 << 3)

#define VCNL4040_PS_TRIG_MASK                 ~(1 << 2)
#define VCNL4040_PS_TRIG_TRIGGER              (1 << 2)

#define VCNL4040_WHITE_EN_MASK                ~(1 << 7)
#define VCNL4040_WHITE_ENABLE                 0
#define VCNL4040_WHITE_DISABLE                (1 << 7)

#define VCNL4040_PS_MS_MASK                   ~(1 << 6)
#define VCNL4040_PS_MS_DISABLE                0
#define VCNL4040_PS_MS_ENABLE                 (1 << 6)

#define VCNL4040_LED_I_MASK                   ~((1 << 2) | (1 << 1) | (1 << 0))
#define VCNL4040_LED_50MA                     0
#define VCNL4040_LED_75MA                     (1 << 1)
#define VCNL4040_LED_100MA                    (1 << 1)
#define VCNL4040_LED_120MA                    ((1 << 1) | (1 << 0))
#define VCNL4040_LED_140MA                    (1 << 2)
#define VCNL4040_LED_160MA                    ((1 << 2) | (1 << 0))
#define VCNL4040_LED_180MA                    ((1 << 2) | (1 << 1))
#define VCNL4040_LED_200MA                    ((1 << 2) | (1 << 1) | (1 << 0))

#define VCNL4040_INT_FLAG_ALS_LOW             (1 << 5)
#define VCNL4040_INT_FLAG_ALS_HIGH            (1 << 4)
#define VCNL4040_INT_FLAG_CLOSE               (1 << 1)
#define VCNL4040_INT_FLAG_AWAY                (1 << 0)

#define VCNL4040_INT_AWAY_MASK                (1 << 0)
#define VCNL4040_INT_CLOSE_MASK               (1 << 1)
#define VCNL4040_INT_LIGHT_MASK               (1 << 4)
#define VCNL4040_INT_DARK_MASK                (1 << 5)

/***************************************************************************//**
 * @brief
 *  Typedef for specifying the software version of the core driver.
 ******************************************************************************/
typedef struct
{
  uint8_t major; /*!< major number */
  uint8_t minor; /*!< minor number */
  uint8_t build; /*!< build number */
  uint32_t revision; /*!< revision number */
} sl_vcnl4040_core_version_t;

/***************************************************************************//**
 * @brief
 *  Structure to store the sensor configuration
 ******************************************************************************/
typedef struct {
  uint8_t  PSDuty;               /*!< proximity sensor duty ratio */
  uint8_t  IRLEDCurrent;         /*!< IR led current */
  uint8_t  PSPersistence;        /*!< proximity sensor interrupt persistence */
  uint8_t  PSIntegrationTime;    /*!< proximity sensor integration time */
  uint8_t  PSResolution;         /*!< proximity sensor data resolution */
  uint8_t  PSInterruptType;      /*!< proximity sensor interrupt type */
  bool     PSEnabled;            /*!< proximity sensor enable/disable */
  bool     PSSmartPersEnabled;   /*!< proximity sensor smart persistence enable/disable */
  bool     PSActiveForceEnabled; /*!< proximity sensor avctive force mode enable/disable */
  bool     PSLogicEnabled;       /*!< proximity sensor detection logic output mode enable/disable */
  uint16_t PSCancelThresh;       /*!< proximity sensor cancellation level */
  uint16_t PSHighThreshold;      /*!< proximity sensor high interrupt threshold */
  uint16_t PSLowThreshold;       /*!< proximity sensor low interrupt threshold */

  uint8_t  ALSPersistence;       /*!< ambient light sensor interrupt persistence */
  uint8_t  ALSIntegrationTime;   /*!< ambient light sensor  integration time */
  uint16_t ALSHighThreshold;     /*!< ambient light sensor high interrupt threshold */
  uint16_t ALSLowThreshold;      /*!< ambient light sensor low interrupt threshold */
  bool     ALSEnabled;           /*!< ambient light sensor enable/disable */
  bool     ALSIntEnabled;        /*!< ambient light sensor interrupt enable/disable */
  bool     WhiteEnabled;         /*!< white channle enable/disable */
} SL_VCNL4040_Sensor_Config_TypeDef;

/***************************************************************************//**
 * @brief
 *  Type of interrupt source when using interrupt mode.
 ******************************************************************************/
typedef enum {
  INT_CLOSE = 0, /*!< interrupt close */
  INT_AWAY  = 1, /*!< interrupt away */
  INT_LIGHT = 2, /*!< interrupt light */
  INT_DARK  = 4, /*!< interrupt dark */
} vcnl4040_irq_source_t;

/***************************************************************************//**
 * @brief
 *   VCNL4040 interrupt callback function; normal operation
 *
 * @details
 *   This callback function is executed in the vcnl4040_is_interrupt function
 *   in the main loop. This callback function will take the interrupt source as
 *   input parameter. The vcnl4040_is_interrupt will be call in the main loop
 *   when the interrupt flag is raised. vcnl4040_is_interrupt will detect the
 *   source of the interrupt and then the corresponding callback function will
 *   be executed.
 *
 * @return
 *   TBD.
 ******************************************************************************/
typedef void (*vcnl4040_norm_interrupt_callback_t)(vcnl4040_irq_source_t irq);
// -----------------------------------------------------------------------------
//                       Public Function Definitions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief
 *  Initialize the VCNL4040 sensor.
 *
 * @param[in] i2cspm
 *  The I2C peripheral to use.
 *
 * @details
 *  This function stores the I2C peripheral setting in static config, checks
 *  the connection by requesting/verifying chip ID, and loads default
 *  initialization settings.
 *
 * @retval SL_STATUS_OK Success
 * @retval SL_STATUS_INITIALIZATION Initialization Failure
 ******************************************************************************/
sl_status_t vcnl4040_init(sl_i2cspm_t *i2cspm_instance);

/***************************************************************************//**
 * @brief
 *  De-initalize the VCNL4040 sensor.
 *
 * @param[in] i2cspm
 *  The I2C peripheral to use.
 *
 * @details
 *  This function returns the GPIO pin used for INT1 to disabled state, and
 *  powers down the device's ambient and proximity sensing.
 *
 * @note
 *  The purpose of de-initialization is to shutdown the VCNL4040 as part
 *  of a complete shutdown sequence for an EFM32/EFR32-based system.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *   @li @ref SL_STATUS_NOT_INITIALIZED if the driver was not initialized.
 ******************************************************************************/
sl_status_t vcnl4040_deinit(void);

/***************************************************************************//**
 * @brief
 *  This function reads the device ID.
 *
 * @param[out] idData
 *  ID MSB/LSB register data - 0x0186
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_get_id(uint16_t *id);

/***************************************************************************//**
 * @brief
 *  Set the duty cycle of the IR LED. The higher the duty ratio, the faster
 *  the response time achieved with higher power consumption. For example,
 *  PS_Duty = 1/320, peak IRED current = 100 mA, averaged current consumption
 *  is 100 mA/320 = 0.3125 mA.
 *
 * @param[in] duty_value
 *  PS IRED on/off duty ratio - [0x0:1/40, 0x1:1/80, 0x2:1/160, 0x3:1/320]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_set_ir_duty_cycle(uint8_t duty_value);

/***************************************************************************//**
 * @brief
 *  Get the duty cycle of the IR LED. The higher the duty ratio, the faster
 *  the response time achieved with higher power consumption. For example,
 *  PS_Duty = 1/320, peak IRED current = 100 mA, averaged current consumption
 *  is 100 mA/320 = 0.3125 mA.
 *
 * @param[out] duty_value
 *  PS IRED on/off duty ratio - [0x0:1/40, 0x1:1/80, 0x2:1/160, 0x3:1/320]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_get_ir_duty_cycle(uint8_t *duty_value);

/***************************************************************************//**
 * @brief
 *  Sets the IR LED sink current
 *
 * @param[in] current_value
 *  LED current selection setting - [
 *    0x0:50 mA,
 *    0x1:75 mA,
 *    0x2:100 mA,
 *    0x3:120 mA,
 *    0x4:140 mA,
 *    0x5:160 mA,
 *    0x6:180 mA,
 *    0x7:200 mA ]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_set_ir_led_sink_current(uint8_t current_value);

/***************************************************************************//**
 * @brief
 *  Gets the IR LED sink current
 *
 * @param[out] current_value
 *  LED current selection setting - [
 *    0x0:50 mA,
 *    0x1:75 mA,
 *    0x2:100 mA,
 *    0x3:120 mA,
 *    0x4:140 mA,
 *    0x5:160 mA,
 *    0x6:180 mA,
 *    0x7:200 mA ]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_get_ir_led_sink_current(uint8_t *current_value);

/***************************************************************************//**
 * @brief
 *  This function sets the proximity interrupt persistence value. The PS
 *  persistence function helps to avoid false trigger of the PS INT. It
 *  defines the amount of consecutive hits needed in order for a PS interrupt
 *  event to trigger.
 *
 * @param[in] pers_value
 *  PS interrupt persistence - [0x0:1, 0x1:2, 0x2:3, 0x3:4]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_set_proximity_interrupt_persistence(uint8_t pers_value);

/***************************************************************************//**
 * @brief
 *  This function returns the proximity interrupt persistence value.
 *
 * @param[out] pers_value
 *  PS interrupt persistence - [0x0:1, 0x1:2, 0x2:3, 0x3:4]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_proximity_interrupt_persistence(uint8_t *pers_value);

/***************************************************************************//**
 * @brief
 *  This function sets the integration time for the proximity sensor.
 *
 * @param[in] time_value
 *  PS integration time setting; represents the duration of energy received
 *  [0x0:1T, 0x1:1.5T, 0x2:2T, 0x3:2.5T, 0x4:3T, 0x5:3.5T, 0x6:4T, 0x7:8T]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_set_proximity_integration_time(uint8_t time_value);

/***************************************************************************//**
 * @brief
 *  This function returns the integration time for the proximity sensor.
 *
 * @param[out] time_value
 *  PS integration time setting; represents the duration of energy received
 *  [0x0:1T, 0x1:1.5T, 0x2:2T, 0x3:2.5T, 0x4:3T, 0x5:3.5T, 0x6:4T, 0x7:8T]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_proximity_integration_time(uint8_t *time_value);

/***************************************************************************//**
 * @brief
 *  This function powers on proximity detection.
 *
 * @param[in] enable
 *  If true, powers proximity detection, if false proximity detection off.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_power_on_proximity(bool enable);

/***************************************************************************//**
 * @brief
 *  This function returns the proximity detection powered status.
 *
 * @param[out] enabled
 *  If true, powers proximity detection, if false proximity detection off.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_proximity_powered(bool *enabled);

/***************************************************************************//**
 * @brief
 *  This function sets the proximity resolution.
 *
 * @param[in] resolution_value
 *  PS resolution - [0x0:12-bit, 0x1:16-bit]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_set_proximity_resolution(uint8_t resolution_value);

/***************************************************************************//**
 * @brief
 *  This function returns the proximity resolution.
 *
 * @param[out] resolution_value
 *  PS resolution - [0x0:12-bit, 0x1:16-bit]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_proximity_resolution(uint8_t *resolution_value);

/***************************************************************************//**
 * @brief
 *  This function sets the proximity interrupt type.
 *
 * @param[in] interruptValue
 *  PS interrupt configuraion - [
 *    0x0:disabled,
 *    0x1:trigger when close,
 *    0x2:trigger when away,
 *    0x3:trigger when close OR away]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_set_proximity_int_type(uint8_t interrupt_value);

/***************************************************************************//**
 * @brief
 *  This function returns the proximity interrupt type.
 *
 * @param[out] interruptValue
 *  PS interrupt configuraion - [
 *    0x0:disabled,
 *    0x1:trigger when close,
 *    0x2:trigger when away,
 *    0x3:trigger when close OR away]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_get_proximity_int_type(uint8_t *interrupt_value);

/***************************************************************************//**
 * @brief
 *  This function enables smart persistence. To accelerate the PS response
 *  time, smart persistence prevents the misjudgment of proximity sensing but
 *  also keeps a fast response time.
 *
 * @param[in] enable
 *  If true, enables smart persistence, if false disables smart persistence.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_enable_smart_persistence(bool enable);

/***************************************************************************//**
 * @brief
 *  This function returns the smart persistence enabled status.
 *
 * @param[out] enabled
 *  If true, smart persistence enabled, if false smart persistence disabled.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_get_smart_persistence_enabled(bool *enabled);

/***************************************************************************//**
 * @brief
 *  This function enables active force mode. Power saving method of using PS:
 *  VCNL4040 remains in standby mode; host requests single proximity
 *  measurement. VCNL4040 becomes active, takes one measurement, then returns
 *  to standby. PS measurement can then be read from PS result registers.
 *
 * @param[in] enable
 *  If true, enables active force mode, if false normal PS mode.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_enable_active_force_mode(bool enable);

/***************************************************************************//**
 * @brief
 *  This function returns active force mode enabled status.
 *
 * @param[in] enable
 *  If true, active force mode enabled, if false normal PS mode.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_active_force_mode_enabled(bool *enabled);

/***************************************************************************//**
 * @brief
 *  This function requests a single PS measurement (active force mode).
 *  Triggers a \single PS measurement.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_trigger_proximity_measurement(void);

/***************************************************************************//**
 * @brief
 *  This function enables the proximity detection logic output mode. When this
 *  mode is selected, the INT pin is pulled low when an object is close to the
 *  sensor (value is above high threshold) and is reset to high when the object
 *  moves away (value is below low threshold). Register: PS_THDH / PS_THDL
 *  define high and low thresholds.
 *
 * @param[in] enable
 *  If true, PS logic mode enabled, if false, normal PS interrupt operation.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_enable_proximity_logic_mode(bool enable);

/***************************************************************************//**
 * @brief
 *  This function returns the proximity detection logic output mode status.
 *
 * @param[out] enable
 *  If true, PS logic mode enabled, if false, normal PS interrupt operation.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_proximity_logic_mode_enabled(bool *enabled);

/***************************************************************************//**
 * @brief
 *  This function sets the proximity sensing cancelation value - helps reduce
 *  cross talk with ambient light.
 *
 * @param[in] cancelValue
 *  PS cancelation level setting - valid values: 0x0000 - 0xFFFF
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_set_proximity_cancelation(uint16_t cancel_value);

/***************************************************************************//**
 * @brief
 *    This function returns the proximity sensing cancelation value - helps
 *    reduce cross talk with ambient light.
 *
 * @param[out] cancelValue
 *    PS cancelation level setting - valid values: 0x0000 - 0xFFFF
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_proximity_cancelation(uint16_t *cancel_value);

/***************************************************************************//**
 * @brief
 *  This function sets the proximity sensing low threshold; value that
 *  proximity sensing must go below to trigger an interrupt.
 *
 * @param[in] thresholdValue
 *  PS interrupt low threshold setting - valid values: 0x0000 - 0xFFFF
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_set_proximity_low_threshold(uint16_t threshold_value);

/***************************************************************************//**
 * @brief
 *  This function returns the proximity sensing low threshold; value that
 *  proximity sensing must go below to trigger an interrupt.
 *
 * @param[out] thresholdValue
 *  PS interrupt low threshold setting - valid values: 0x0000 - 0xFFFF
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_proximity_low_threshold(uint16_t *threshold_value);

/***************************************************************************//**
 * @brief
 *  This function sets the proximity sensing high threshold; value that
 *  proximity sensing must go above to trigger an interrupt.
 *
 * @param[in] thresholdValue
 *  PS interrupt high threshold setting - valid values: 0x0000 - 0xFFFF
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_set_proximity_high_threshold(uint16_t threshold_value);

/***************************************************************************//**
 * @brief
 *  This function returns the proximity sensing high threshold; value that
 *  proximity sensing must go above to trigger an interrupt.
 *
 * @param[out] thresholdValue
 *  PS interrupt high threshold setting - valid values: 0x0000 - 0xFFFF
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_proximity_high_threshold(uint16_t *threshold_value);

/***************************************************************************//**
 * @brief
 *  This function reads the proximity sensor data value.
 *
 * @param[out] proximityData
 *  PS MSB/LSB register data - valid values: 0x0000 - 0xFFFF
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_get_proximity(uint16_t *proximity_data);

/***************************************************************************//**
 * @brief
 *  This function reads the interrupt flag and checks PS_IF_CLOSE interrupt
 *  status.
 *
 * @param[out] isClose
 *  Returns true if the prox value rises above the upper threshold
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_is_close(bool *is_close);

/***************************************************************************//**
 * @brief
 *  This function reads the interrupt flag and checks PS_IF_AWAY interrupt
 *  status.
 *
 * @param[out] isAway
 *  Returns true if the prox value drops below the lower threshold
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_is_away(bool *is_away);

/***************************************************************************//**
 * @brief
 *  This function sets the ambient interrupt persistence value. The ALS
 *  persistence function helps to avoid false trigger of the ALS INT. It
 *  defines the amount of consecutive hits needed in order for a ALS interrupt
 *  event to trigger.
 *
 * @param[in] persValue
 *  ALS interrupt persistence - [0x0:1, 0x1:2, 0x2:4, 0x3:8]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_set_ambient_interrupt_persistence(uint8_t pers_value);

/***************************************************************************//**
 * @brief
 *    This function returns the Ambient interrupt persistence value.
 *
 * @param[out] persValue
 *    ALS interrupt persistence - [0x0:1, 0x1:2, 0x2:4, 0x3:8]
 *
 *
 * @return
 *    SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_ambient_interrupt_persistance(uint8_t *pers_value);

/***************************************************************************//**
 * @brief
 *  This function enables ambient light detection interrupts.
 *
 * @param[in] enable
 *  If true, enables ambient interrupts, if false ambient interrupts disabled.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_enable_ambient_interrupts(bool enable);

/***************************************************************************//**
 * @brief
 *  This function returns the ambient light detection interrupt enabled status.
 *
 * @param[out] enable
 *  If true, ambient interrupts enabled, if false ambient interrupts disabled.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_ambient_interrupt_enabled(bool *enable);

/***************************************************************************//**
 * @brief
 *  This function powers on ambient light detection.
 *
 * @param[in] enable
 *  If true, powers ambient detection, if false ambient detection off.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_power_on_ambient(bool enable);

/***************************************************************************//**
 * @brief
 *  This function returns the ambient light detection power status.
 *
 * @param[out] enable
 *  If true, ambient detection powered on, if false ambient detection off.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_ambient_powered(bool *enable);

/***************************************************************************//**
 * @brief
 *  This function sets the integration time for the ambient light sensor
 *
 * @param[in] timeValue
 *  ALS integration time setting, longer integration time has higher
 *  sensitivity - [0x0:80 ms, 0x1:160 ms, 0x2:320 ms, 0x3:640 ms]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_set_ambient_integration_time(uint8_t time_value);

/***************************************************************************//**
 * @brief
 *  Get the integration time for the ambient light sensor
 *
 * @param[out] timeValue
 *  ALS integration time setting, longer integration time has higher
 *  sensitivity - [0x0:80 ms, 0x1:160 ms, 0x2:320 ms, 0x3:640 ms]
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_ambient_integration_time(uint8_t *time_value);

/***************************************************************************//**
 * @brief
 *  Set the ambient light sensing low threshold; value that ambient sensing
 *  must go below to trigger an interrupt.
 *
 * @param[in] thresholdValue
 *  ALS high interrupt threshold - valid values: 0x0000 - 0xFFFF
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_set_ambient_low_threshold(uint16_t threshold_value);

/***************************************************************************//**
 * @brief
 *  Get the ambient light sensing low threshold; value that ambient sensing
 *  must go below to trigger an interrupt.
 *
 * @param[out] thresholdValue
 *  ALS high interrupt threshold - valid values: 0x0000 - 0xFFFF
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_ambient_low_threshold(uint16_t *threshold_value);

/***************************************************************************//**
 * @brief
 *  Set the ambient light sensing high threshold; value that ambient sensing
 *  must go above to trigger an interrupt.
 *
 * @param[in] thresholdValue
 *  ALS high interrupt threshold - valid values: 0x0000 - 0xFFFF
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_set_ambient_high_threshold(uint16_t threshold_value);

/***************************************************************************//**
 * @brief
 *  Get the ambient light sensing high threshold; value that ambient sensing
 *  must go above to trigger an interrupt.
 *
 * @param[out] thresholdValue
 *  ALS high interrupt threshold - valid values: 0x0000 - 0xFFFF
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_ambient_high_threshold(uint16_t *threshold_value);

/***************************************************************************//**
 * @brief
 *  This function reads the ambient light sensor data value.
 *
 * @param[out] ambientData
 *  ALS MSB/LSB register data - valid values: 0x0000 - 0xFFFF
 *
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_get_ambient(uint16_t *ambient_data);

/***************************************************************************//**
 * @brief
 *  This function enables the white measurement channel.
 *
 * @param[in] enable
 *  If true, white channel enabled, if false, white channel disabled.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_enable_white_channel(bool enable);

/***************************************************************************//**
 * @brief
 *  This function returns the white measurement channel enabled status.
 *
 * @param[in] enable
 *  If true, white channel enabled, if false, white channel disabled.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 ******************************************************************************/
sl_status_t vcnl4040_get_white_channel_enabled(bool *enable);

/***************************************************************************//**
 * @brief
 *  This function reads the white light data value.
 *
 * @param[out] whiteData
 *  White channel MSB/LSB register data - valid values: 0x0000 - 0xFFFF
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_get_white(uint16_t *white_data);

/***************************************************************************//**
 * @brief
 *  This function reads the interrupt flag and checks PS_IF_H interrupt
 *  status.
 *
 * @param[out] isLight
 *  Returns true if the ALS value rises above the upper threshold
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_is_light(bool *is_light);

/***************************************************************************//**
 * @brief
 *  This function reads the interrupt flag and checks PS_IF_L interrupt
 *  status.
 *
 * @param[out] isDark
 *  Returns true if the ALS value drops below the lower threshold
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_is_dark(bool *is_dark);

/***************************************************************************//**
 * @brief
 *  This function resigters the callback function for interrupt events.
 *  status.
 *
 * @param[in] callback
 *  Function pointer that points to interrupt callback function.
 ******************************************************************************/
void vcnl4040_interrupt_callback_register(vcnl4040_norm_interrupt_callback_t callback);

/***************************************************************************//**
 * @brief
 *  This function is called in the main application process action sequence.
 *  The function checks the driver interrupt flag and if set, reads the sensor
 *  interrupt flag register and returns updated ALS and PS status. For
 *  proximity logic mode, status read is unnecessary and flag status used to
 *  set output parameters.
 *
 * @param[out] isLight
 *  Returns true if the ALS value rises above the upper threshold
 * @param[out] isDark
 *  Returns true if the ALS value drops below the lower threshold
 * @param[out] isClose
 *  Returns true if the prox value rises above the upper threshold
 * @param[out] isAway
 *  Returns true if the prox value drops below the lower threshold
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_is_interrupt(bool *isLight,
                                  bool *isDark,
                                  bool *isClose,
                                  bool *isAway);

/***************************************************************************//**
 * @brief
 *  This function returns the version code of the sensor.
 *
 * @param[out] version
 *  Returns the struct that holds the version code value.
 *
 * @return
 *  SL_STATUS_OK if there are no errors.
 *  SL_STATUS_TRANSMIT if I2C transmit failed.
 ******************************************************************************/
sl_status_t vcnl4040_get_core_version(sl_vcnl4040_core_version_t *version);

/** @} (end group VCNL4040) */

#ifdef __cplusplus
}
#endif

#endif /* VCNL4040_H_ */

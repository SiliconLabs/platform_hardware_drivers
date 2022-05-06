/***************************************************************************//**
 * @file max1704x.h
 * @brief define driver structures and APIs for the max17048/max17049 Fuel Gauge
 *******************************************************************************
 * # License
* <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
********************************************************************************
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
*******************************************************************************/
#ifndef __MAX17048_H_
#define __MAX17048_H_

#include <stdint.h>
#include <stddef.h>
#include "sl_status.h"
#include "sl_i2cspm.h"
#include "tempdrv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup max17048 - Fuel Gauge Sensor
 * @brief Driver for the max17048/max17049 Fuel Gauge


   @n @section max17048_example MAX17048 example

     Basic example for performing measurement: @n @n
     @verbatim

   #include "sl_i2cspm_instances.h"
   #include "max17048.h"

   int main( void )
   {

     ...

     uint32_t soc;
     uint32_t vcell;

     max17048_init(sl_i2cspm_sensor_env);
     // read the ID
     max17048_get_id(&id);
     max17048_get_vcell(&vcell);
     max17048_get_soc(&soc);

     ...

   } @endverbatim

 * @{
 ******************************************************************************/

#define MAX17048_VCELL                (0x02)   // Address of VCELL register
#define MAX17048_SOC                  (0x04)   // Address of SOC register
#define MAX17048_MODE                 (0x06)   // Address of MODE register
#define MAX17048_VERSION              (0x08)   // Address of VERSION register
#define MAX17048_HIBRT                (0x0A)   // Address of HIBRT register
#define MAX17048_CONFIG               (0x0C)   // Address of CONFIG register
#define MAX17048_VALRT                (0x14)   // Address of VALRT register
#define MAX17048_CRATE                (0x16)   // Address of CRATE register
#define MAX17048_VRESET_ID            (0x18)   // Address of VRESET/ID register
#define MAX17048_STATUS               (0x1A)   // Address of STATUS register
#define MAX17048_CMD                  (0xFE)   // Address of CMD register
#define MAX17048_LOCK_TABLE           (0x3E)   // Address of lock register
#define MAX17048_TABLE                (0x40)   // Address of TABLE register

#define MAX17048_RESET_MSB            (0x54)   // MSB - Reset value
#define MAX17048_RESET_LSB            (0x00)   // LSB - Reset value
#define MAX17048_RCOMP0               (0x97)   // POR value of RCOMP
#define MAX17048_UNLOCK_MSB           (0x4A)   // MSB - Unlock value
#define MAX17048_UNLOCK_LSB           (0x57)   // LSB - Unlock value
#define MAX17048_LOCK                 (0x00)   // Lock value

/* MODE Register */
#define MAX17048_MODE_HIBSTAT_BIT     (4)
#define MAX17048_MODE_ENSLEEP_BIT     (5)
#define MAX17048_MODE_QUICK_START_BIT (6)

/* CONFIG Register */
#define MAX17048_CONFIG_SLEEP_BIT     (7)
#define MAX17048_CONFIG_ALSC_BIT      (6)
#define MAX17048_CONFIG_ALRT_BIT      (5)
#define MAX17048_VRESET_DIS_BIT       (0)
#define MAX17048_STATUS_ENVR_BIT      (6)

/* VCELL resolution in nanoVolts (78.125 uV) */
#define MAX17048_VCELL_RESOLUTION     (78125)
/* SOC resolution (1%/256)*/
#define MAX17048_SOC_RESOLUTION       (256)
/* VALRT resolution in mV */
#define MAX17048_VALRT_RESOLUTION     (20)
/* VRESET resolution in mV */
#define MAX17048_VRESET_RESOLUTION    (40)
/* CRATE resolution in %/hr */
#define MAX17048_CRATE_RESOLUTION     (0.208)
/* Hibernate threshold resolution in %/hr */
#define MAX17048_HIBTHR_RESOLUTION    (0.208)
/* Active threshold resolution in microVolts (1.25mV) */
#define MAX17048_ACTTHR_RESOLUTION    (1250)

#define MAX17048_VALRT_MAX_MV         (MAX17048_VALRT_RESOLUTION * 255)
#define MAX17048_VALRT_MIN_MV         (MAX17048_VALRT_RESOLUTION * 255)
#define MAX17048_VRESET_MV            (MAX17048_VRESET_RESOLUTION * 127)
#define MAX17048_HIBTHR_PERCENT       (MAX17048_HIBTHR_RESOLUTION * 255)
#define MAX17048_ACTTHR_MV            (MAX17048_ACTTHR_RESOLUTION * 255)

#define MAX17048_STATE_ENABLE         0x01
#define MAX17048_STATE_DISABLE        0x00
#define MAX17048_VALUE_SET            0x01
#define MAX17048_VALUE_RESET          0x00

#define MAX17048_ALERT_VH_VAL         0xC8 // 4.00 V
#define MAX17048_ALERT_VL_VAL         0xAF // 3.50 V
#define MAX17048_VRESET_THRES         0x3F // 2.5 V

#define MAX17048_ALERT_PIN_SET        0x01
#define MAX17048_ALERT_PIN_RESET      0x00

#define MAX17048_I2C_ADDRESS          0x36 // I2C address

#define RCOMP0                        0x97
#define TEMP_CO_UP                    -0.5
#define TEMP_CO_DOWN                  5.0

#define MAX17048_STATUS_VH            (1 << 0)
#define MAX17048_STATUS_VL            (1 << 1)
#define MAX17048_STATUS_VR            (1 << 2)
#define MAX17048_STATUS_HD            (1 << 3)
#define MAX17048_STATUS_SC            (1 << 4)

#define MAX17048_ENUM(name) typedef uint8_t name; enum name##_enum

/// @brief MAX17048 interrupt source enum
MAX17048_ENUM(sl_max17048_irq_source_t) {
  IRQ_VCELL_HIGH  =  0,
  IRQ_VCELL_LOW   =  1,
  IRQ_RESET       =  2,
  IRQ_EMPTY       =  3,
  IRQ_SOC         =  4,
};

/// @brief MAX17048 hibernate state
typedef enum {
  activeMode    = 0,    ///< Device is in active mode
  hibernateMode = 1     ///< Device is in hibernate mode
} max17048_hibstate_t;

/***************************************************************************//**
 * @brief
 *   MAX17048 temperature external temperature measurement callback function.
 *
 * @details
 *   This callback function is executed from interrupt context when the user
 *   opts to provide the battery pack temperature through a mechanism other
 *   than the integrated EMU temperature sensor (e.g. an external temperature
 *   sensor or other means). The driver needs the battery pack temperature
 *   to periodically update the MAX17048 compensation factor (RCOMP).
 *
 * @return
 *   Temperature, as a signed integer in degrees C.
 ******************************************************************************/
typedef int32_t (*max17048_temp_callback_t)(void);

/***************************************************************************//**
 * @brief
 *   Initialize the MAX17048 driver with the values provided in the
 *   max17048_config.h file.
 *
 * @details
 *   This function does not write to any of the MAX17048 registers. It
 *   assigns the I2C used to communicate with the device, configures the
 *   GPIO(s) used for the ALRTn and optional QSTRT pin(s), and starts a
 *   software timer to trigger temperature (RCOMP) updates at the
 *   user-specified rate.
 *
 * @param[in] i2cspm
 *   The I2CSPM instance to use.
 *
 * @note
 *   Upon return, all interrupts from the MAX17048 are masked so that
 *   firmware can enable interrupts that are required, register their
 *   callback functions, and set relevant thresholds. Interrupts are
 *   globally unmasked by calling max17048_unmask_interrupts().
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_ALREADY_INITIALIZED the function has previously
 *     been called.
 ******************************************************************************/
sl_status_t max17048_init(sl_i2cspm_t *i2cspm);

/***************************************************************************//**
 * @brief
 *   De-initialize the MAX17048 driver.
 *
 * @details
 *   This function does not write to any of the MAX17048 registers. Its
 *   sole purpose is to return the GPIO pin(s) used for the ALRTn and
 *   optional QSTRT pins to the disabled state.
 *
 *   The proper way to de-initialize the MAX17048 would be to manually
 *   disable all of its interrupts (or set those with thresholds to
 *   min/max values that cannot be reached) and then place the device in
 *   sleep mode by calling max17048_enter_sleep(), which disables
 *   battery monitoring altogether.
 *
 * @note
 *   The purpose of de-initialization is to shutdown the MAX17048 as part
 *   of a complete shutdown sequence for an EFM32/EFR32-based system.
 *   Because driver de-initialization disables interrupt recognition, it
 *   is no longer possible to detect a battery swap, even though the
 *   MAX17048 will do so and assert ALRTn. Leave the driver initialized
 *   to handle ALRTn assertion on battery swap, even if the MAX17048 has
 *   been placed in sleep mode.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_NOT_INITIALIZED if the driver is not initialized.
 ******************************************************************************/
sl_status_t max17048_deinit(void);

/***************************************************************************//**
 * @brief This function returns the cell voltage in millivolts.
 *
 * @param[out] vcell The cell voltage expressed in millivolts as
 *   an integer. The function converts the raw output from the
 *   MAX17048 where 1 LSB = 78.125 uV/cell.
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
sl_status_t max17048_get_vcell(uint32_t *vcell);

/***************************************************************************//**
 * @brief This function returns SOC as an integer %.
 *
 * @param[out] soc The state of charge [0-100%]
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
sl_status_t max17048_get_soc(uint32_t *soc);

/***************************************************************************//**
 * @brief This function gets an approximate value for the average SOC rate of
 * change.
 *
 * @param[out] crate The average rate of change in SOC in % as a floating 
 *  point value. The CRATE register returns a value expressed at 1 LSB = 0.208%
 *  per hour. The value does not specifically reflect current consumption
 *  and cannot be converted to a current.
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
sl_status_t max17048_get_crate(float *crate);

/***************************************************************************//**
 * @brief
 *   Register the temperature update callback for the MAX17048 driver
 *
 * @details
 *   This function is used to register a callback that periodically updates
 *   the temperature of the battery monitored by the MAX17048. By default, the
 *   driver uses the EMU temperature sensor on Series 2 EFM32/EFR32 devices.
 *
 *   However, in order for this sensor to accurately reflect the battery
 *   temperature, the MCU must be underneath (e.g. on the side of the PCB
 *   opposite) or immediately adjacent to the battery.
 *
 *   In cases where this is not possible, the battery temperature can be
 *   provided to the driver from another source, such as an external
 *   temperature sensor that is integrated with the battery or mounted in
 *   close proximity to it. The driver imposes no particular requirements
 *   on the source of the temperature other than that it is provided as a
 *   signed integer in degrees Celsius.
 *
 * @note
 *   The driver permits only one temperature update callback function to be
 *   registered. Attempting to register another callback returns an error.
 *
 *   Firmware can unregister the current callback function, in which case
 *   the driver will revert to using the EMU temperature sensor. This
 *   can be useful to further reduce overall current draw in cases where the
 *   system enters a low-power, quiescent state such that the temperature
 *   reported by the EMU sensor is effectively the same as the battery
 *   temperature.
 *
 * @details
 *   There is no attempt to synchronize expiration of the software timer
 *   that updates the MAX17048's compensation factor (RCOMP) with
 *   registration of the user-provided callback function. If the software
 *   timer expires and no callback has been registered, the EMU sensor's
 *   output is used.
 *
 *   This can be a concern if the user callback function does not actually
 *   read the temperature from whatever alternate source is used but instead
 *   simply returns a value that is updated in some asynchronous fashion.
 *   If firmware does not seed the callback function with an accurate
 *   battery temperature, the MAX17408 could report an erroneous SOC.
 *
 * @param[in] temp_cb
 *   User-defined function to return the battery temperature to the driver.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_NULL_POINTER if the callback is NULL.
 *
 *   @li @ref SL_STATUS_ALREADY_INITIALIZED if a callback has already been
 *     initialized.
 ******************************************************************************/
sl_status_t max17048_register_temperature_callback(max17048_temp_callback_t temp_cb);

/***************************************************************************//**
 * @brief
 *   Unregister the temperature update callback for the MAX17048 driver
 *
 * @details
 *   This is the opposite of max17048_register_temperature_callback(). It
 *   unregisters the previously-registered temperature update callback
 *   function and causes the driver to revert to the output of the EMU
 *   temperature sensor when next updating the MAX17048's RCOMP value.
 *
 * @details
 *   There is no attempt to unregister the user-provided callback function
 *   before expiration of the software timer that updates RCOMP. If the
 *   timer expires and the callback is still registered, it must provide
 *   a valid temperature to the driver.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_NOT_INITIALIZED if a callback has not previously
 *     been registered.
 ******************************************************************************/
sl_status_t max17048_unregister_temperature_callback(void);

/***************************************************************************//**
 * @brief
 *   Set the RCOMP update interval.
 *
 * @details
 *   This function sets the temperature compensation factor (RCOMP) update
 *   interval. During driver initialization, the value specified by
 *   MAX17048_CONFIG_RCOMP_UPDATE_INTERVAL_MS is used and defaults to
 *   1 minute (60000 ms). Effectively, it changes the timeout rate of the
 *   Sleeptimer periodic software timer callback that recalculates RCOMP
 *   and sends the updated value to the MAX17048.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_INVALID_PARAMETER if interval < 1000 or
 *     interval > 60000.
 ******************************************************************************/
sl_status_t max17048_set_update_interval(uint32_t interval);

/***************************************************************************//**
 * @brief
 *   Get the RCOMP update interval.
 *
 * @details
 *   This function returns the interval at which the driver updates the
 *   MAX17048 temperature compensation factor (RCOMP). It is the
 *   timeout rate of the Sleeptimer periodic software timer callback that
 *   recalculates RCOMP and sends the updated value to the MAX17048. *
 *
 * @return
 *   @li Update rate in milliseconds.
 ******************************************************************************/
uint32_t max17048_get_update_interval(void);

/***************************************************************************//**
 * @brief
 *   MAX17048 interrupt callback function.
 *
 * @details
 *   This callback function is executed from interrupt context when the user
 *   has enabled one of the MAX17048 interrupt sources.
 ******************************************************************************/
typedef void (*max17048_interrupt_callback_t)(sl_max17048_irq_source_t irq, void *data);

/***************************************************************************//**
 * @brief
 *   Mask MAX17048 interrupts
 *
 * @details
 *   This disables the high-to-low edge detection on the GPIO pin
 *   connected to the MAX17048 ALRTn output. The MAX17048 itself
 *   does not have any way to globally enable/disable interrupts.
 ******************************************************************************/
void max17048_mask_interrupts(void);

/***************************************************************************//**
 * @brief
 *   Unmask MAX17048 interrupts
 *
 * @details
 *   This enables the high-to-low edge detection on the GPIO pin
 *   connected to the MAX17048 ALRTn output. The MAX17048 itself
 *   does not have any way to globally enable/disable interrupts.
 ******************************************************************************/
void max17048_unmask_interrupts(void);

/***************************************************************************//**
 * @brief
 *   Enables the MAX17048 state-of-charge (SOC) interrupt and registers a
 *   user-provided callback function to respond to it.
 *
 * @details
 *   This function enables the SOC interrupt by writing a 1 to the ALSC
 *   bit in the MAX17048 CONFIG register. The interrupt is requested
 *   whenever SOC changes by at least 1%.
 *
 * @param[in] irq_cb
 *   User-defined function to respond to the SOC interrupt.
 *
 * @param[in] cb_data
 *   Pointer to user data that will be passed to callback.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_NULL_POINTER if irq_cb or cb_data is NULL.
 *
 *   @li @ref SL_STATUS_ALREADY_INITIALIZED if a callback has already been
 *     initialized.
 ******************************************************************************/
sl_status_t max17048_enable_soc_interrupt(max17048_interrupt_callback_t irq_cb,
                                          void *cb_data);

/***************************************************************************//**
 * @brief
 *   Disables the MAX17048 state-of-charge (SOC) interrupt and unregisters
 *   the user-provided callback function.
 *
 * @details
 *   This function disables the SOC interrupt by writing a 0 to the ALSC
 *   bit in the MAX17048 CONFIG register.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_NOT_INITIALIZED if a callback has not previously
 *     been registered.
 ******************************************************************************/
sl_status_t max17048_disable_soc_interrupt(void);

/***************************************************************************//**
 * @brief
 *   Enables the MAX17048 empty alert interrupt, sets its threshold,
 *   and registers a user-provided callback function to respond to it.
 *
 * @details
 *   The empty threshold alert interrupt is technically always active
 *   because it does not have a dedicated enable bit.
 *
 *   However, the alert threshold is set by the ATHD field in the CONFIG
 *   register and is defined as 32 - ATHD. Because ATHD = 0x1C at POR, the
 *   empty threshold defaults to 4%.
 *
 * @param[in] athd
 *   Empty threshold specified as a percent between 1 and 32.
 *
 * @param[in] irq_cb
 *   User-defined function to respond to the SOC of charge falling
 *   below the specified empty threshold level.
 *
 * @param[in] cb_data
 *   Pointer to user data that will be passed to callback.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_INVALID_PARAMETER if athd < 1 or athd > 32.
 *
 *   @li @ref SL_STATUS_NULL_POINTER if irq_cb or cb_data is NULL.
 *
 *   @li @ref SL_STATUS_ALREADY_INITIALIZED if a callback has already been
 *     initialized.
 ******************************************************************************/
sl_status_t max17048_enable_empty_interrupt(
    uint8_t athd,
    max17048_interrupt_callback_t irq_cb,
    void *cb_data);

/***************************************************************************//**
 * @brief
 *   Disables the MAX17048 empty alert interrupt and unregisters the
 *   the user-provided callback function.
 *
 * @details
 *   The empty threshold alert interrupt is technically always active
 *   because it does not have a dedicated enable bit.
 *
 *   As explained for the max17048_enable_empty_interrupt() function
 *   the empty percent level is specified as 32 - ATHD, so setting a
 *   threshold of 1% (ATHD = 31) should prevent the interrupt from being
 *   requested and the driver will ignore it.
 *
 * @note
 *   Because the empty threshold interrupt cannot be disabled (e.g.
 *   there is a chance that in a very low-power system operation might
 *   be possible at 1% SOC), the user should consider disabling all
 *   driver interrupts and placing the MCU in a low-power mode and
 *   polling SOC at a very slow rate if battery swap is possible.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_disable_empty_interrupt(void);

/***************************************************************************//**
 * @brief
 *   Set the empty threshold.
 *
 * @details
 *   This function sets the level at which the empty threshold alert
 *   interrupt is requested.
 *
 * @note
 *   Setting the empty level to 1% effectively disables the threshold
 *   alert interrupt and will cause the driver to ignore it, which means
 *   that no callback function is executed.
 *
 *   There is no requirement for a callback to be registered before
 *   changing the empty threshold. If one has not previously been
 *   registered, the driver will ignore the interrupt.
 *
 * @param[in] athd
 *   Empty threshold specified as a percent between 1 and 32.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_INVALID_PARAMETER if athd = 0 or athd > 32.
 ******************************************************************************/
sl_status_t max17048_set_empty_threshold(uint8_t athd);

/***************************************************************************//**
 * @brief
 *   Get the empty threshold.
 *
 * @note
 *   The driver does not poll the MAX17048 to retrieve this value. At
 *   initialization it is assumed to be the default level of 4% and is
 *   tracked in a private variable should it otherwise be changed.
 *
 * @return
 *   @li Empty threshold between 1% and 32%.
 ******************************************************************************/
uint8_t max17048_get_empty_threshold(void);

/***************************************************************************//**
 * @brief
 *   Enables the MAX17048 voltage high alert interrupt, sets its threshold,
 *   and registers a user-provided callback function to respond to it.
 *
 * @details
 *   The voltage high alert interrupt is technically always active
 *   because it does not have a dedicated enable bit.
 *
 *   However, the high alert level is set by the VALRT register MAX field,
 *   and if it is set to a voltage higher than the maximum VCELL level,
 *   the interrupt will never be requested. Because each LSB of MAX
 *   corresponds to 20 mV, a value of 0xFF is 5.1V, which is greater than
 *   the maximum output voltage of a single Li+ cell.
 *
 * @note
 *   The driver does not sanity check valrt_max. If the user sets
 *   valrt_max to a nonsensical value, e.g. 0, the interrupt will
 *   trigger constantly. There is also no requirement that
 *   valrt_max > valrt_min.
 *
 * @param[in] valrt_max_mv
 *   High voltage alert threshold in millivolts.
 *
 * @param[in] irq_cb
 *   User-defined function to respond to VCELL > valrt_max.
 *
 * @param[in] cb_data
 *   Pointer to user data that will be passed to callback.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_NULL_POINTER if irq_cb or cb_data is NULL.
 *
 *   @li @ref SL_STATUS_ALREADY_INITIALIZED if a callback has already been
 *     initialized.
 ******************************************************************************/
sl_status_t max17048_enable_vhigh_interrupt(uint32_t valrt_max_mv,
                                            max17048_interrupt_callback_t irq_cb,
                                            void *cb_data);

/***************************************************************************//**
 * @brief
 *   Disables the MAX17048 voltage high alert interrupt and
 *   unregisters the user-provided callback function.
 *
 * @details
 *   The voltage high alert interrupt is technically always active
 *   because it does not have a dedicated enable bit.
 *
 *   However, the high alert level is set by the VALRT register MAX field,
 *   and if it is set to a voltage higher than the maximum VCELL level,
 *   the interrupt will never be requested. Because each LSB of MAX
 *   corresponds to 20 mV, a value of 0xFF is 5.1V, which is greater than
 *   the maximum output voltage of a single Li+ cell.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_disable_vhigh_interrupt(void);

/***************************************************************************//**
 * @brief
 *   Set the voltage high alert interrupt threshold.
 *
 * @details
 *   This function sets the level at which the voltage high alert
 *   interrupt is requested.
 *
 * @note
 *   The driver does not sanity check valrt_max. If the user sets
 *   valrt_max to a nonsensical value, e.g. 0, the interrupt will
 *   trigger constantly. There is also no requirement that
 *   valrt_max > valrt_min.
 *
 * @param[in] valrt_max_mv
 *   High voltage alert threshold in millivolts.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_set_vhigh_threshold(uint32_t valrt_max_mv);

/***************************************************************************//**
 * @brief
 *   Get the voltage high alert interrupt threshold.
 *
 * @note
 *   The driver does not poll the MAX17048 to retrieve this value. At
 *   initialization it is assumed to be the default level of 0xFF and
 *   is tracked in a private variable should it otherwise be changed.
 *
 * @return
 *   @li High voltage alert threshold in millivolts.
 ******************************************************************************/
uint32_t max17048_get_vhigh_threshold(void);

/***************************************************************************//**
 * @brief
 *   Enables the MAX17048 voltage low alert interrupt, sets its threshold,
 *   and registers a user-provided callback function to respond to it.
 *
 * @details
 *   The voltage low alert interrupt is technically always active
 *   because it does not have a dedicated enable bit.
 *
 *   However, the low alert level is set by the VALRT register MIN field,
 *   and if it is set to a voltage lower than the minimum VCELL level,
 *   the interrupt will never be requested. Because each LSB of MIN
 *   corresponds to 20 mV, a value of 0x0 is 0V, which is a voltage at
 *   which the system should not even be running.
 *
 * @note
 *   The driver does not sanity check valrt_min. If the user sets
 *   valrt_min to a nonsensical value, e.g. 0xFF, the interrupt will
 *   trigger constantly. There is also no requirement that
 *   valrt_max > valrt_min.
 *
 * @param[in] valrt_min_mv
 *   Low voltage alert threshold in millivolts.
 *
 * @param[in] irq_cb
 *   User-defined function to respond to VCELL > valrt_max.
 *
 * @param[in] cb_data
 *   Pointer to user data that will be passed to callback.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_NULL_POINTER if irq_cb or cb_data is NULL.
 *
 *   @li @ref SL_STATUS_ALREADY_INITIALIZED if a callback has already been
 *     initialized.
 ******************************************************************************/
sl_status_t max17048_enable_vlow_interrupt(uint32_t valrt_min_mv,
                                           max17048_interrupt_callback_t irq_cb,
                                           void *cb_data);

/***************************************************************************//**
 * @brief
 *   Disables the MAX17048 voltage low alert interrupt and
 *   unregisters the user-provided callback function.
 *
 * @details
 *   The voltage low alert interrupt is technically always active
 *   because it does not have a dedicated enable bit.
 *
 *   However, the low alert level is set by the VALRT register MIN field,
 *   and if it is set to a voltage lower than the minimum VCELL level,
 *   the interrupt will never be requested. Because each LSB of MAX
 *   corresponds to 20 mV, a value of 0x0 is 0V, which a voltage at
 *   which the system should not even be running.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_disable_vlow_interrupt(void);

/***************************************************************************//**
 * @brief
 *   Set the voltage low alert interrupt threshold.
 *
 * @details
 *   This function sets the level at which the voltage low alert
 *   interrupt is requested.
 *
 * @note
 *   The driver does not sanity check valrt_min. If the user sets
 *   valrt_min to a nonsensical value, e.g. 0xFF, the interrupt will
 *   trigger constantly. There is also no requirement that
 *   valrt_max > valrt_min.
 *
 * @param[in] valrt_min_mv
 *   Low voltage alert threshold in millivolts.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_set_vlow_threshold(uint32_t valrt_min_mv);

/***************************************************************************//**
 * @brief
 *   Get the voltage low alert interrupt threshold.
 *
 * @note
 *   The driver does not poll the MAX17048 to retrieve this value. At
 *   initialization it is assumed to be the default level of 0x0 and
 *   is tracked in a private variable should it otherwise be changed.
 *
 * @return
 *   @li Low voltage alert threshold in millivolts.
 ******************************************************************************/
uint32_t max17048_get_vlow_threshold(void);

/***************************************************************************//**
 * @brief
 *   Enables the MAX17048 reset alert interrupt, sets its threshold,
 *   and registers a user-provided callback function to respond to it.
 *
 * @details
 *   The reset alert interrupt is used to detect battery removal or a
 *   drop in the cell output voltage below which system functionality
 *   may be impaired. It is enabled by writing a 1 to the EnVr bit in
 *   the STATUS register.
 *
 *   The reset threshold is a 7-bit value specified in units of
 *   1 LSB = 40 mV.
 *
 * @note
 *   The driver does not fully sanity check vreset, and it is possible
 *   for the user to specify a nonsensical value that constantly
 *   triggers the interrupt. For example, vreset = 0x7F corresponds
 *   to a reset voltage of 5.08V, which is greater than the maximum
 *   output level of a single Li+ cell.
 *
 * @param[in] vreset_mv
 *   Low voltage alert threshold in millivolts.
 *
 * @param[in] irq_cb
 *   User-defined function to respond to VCELL > valrt_max.
 *
 * @param[in] cb_data
 *   Pointer to user data that will be passed to callback.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_NULL_POINTER if irq_cb or cb_data is NULL.
 *
 *   @li @ref SL_STATUS_ALREADY_INITIALIZED if a callback has already been
 *     initialized.
 *
 *   @li @ref SL_STATUS_INVALID_PARAMETER if vreset_mv > 5080.
 ******************************************************************************/
sl_status_t max17048_enable_reset_interrupt(uint32_t vreset_mv,
                                            max17048_interrupt_callback_t irq_cb,
                                            void *cb_data);

/***************************************************************************//**
 * @brief
 *   Disables the MAX17048 reset alert interrupt and unregisters
 *   the user-provided callback function.
 *
 * @details
 *   The reset alert interrupt is disabled by writing a 1 to the EnVr
 *   bit in the STATUS register.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_disable_reset_interrupt(void);

/***************************************************************************//**
 * @brief
 *   Set the reset alert interrupt threshold.
 *
 * @details
 *   This function sets the level at which the reset alert interrupt
 *   is requested.
 *
 * @note
 *   The driver does not fully sanity check vreset, and it is possible
 *   for the user to specify a nonsensical value that constantly
 *   triggers the interrupt. For example, vreset = 0x7F corresponds
 *   to a reset voltage of 5.08V, which is greater than the maximum
 *   output level of a single Li+ cell.
 *
 * @param[in] vreset_mv
 *   Low voltage alert threshold in millivolts.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_INVALID_PARAMETER if vreset_mv > 5080.
 ******************************************************************************/
sl_status_t max17048_set_reset_threshold(uint32_t vreset_mv);

/***************************************************************************//**
 * @brief
 *   Get the reset alert interrupt threshold.
 *
 * @note
 *   The driver does not poll the MAX17048 to retrieve this value. At
 *   initialization it is assumed to be the default level of 0x4B and
 *   is tracked in a private variable should it otherwise be changed.
 *
 * @return
 *   @li Reset alert threshold in millivolts.
 ******************************************************************************/
uint32_t max17048_get_reset_threshold(void);

/***************************************************************************//**
 * @brief
 *   Enables MAX17048 automatic hibernation by setting the activity
 *   and CRATE (rate of discharge/charge) thresholds to non-min/max
 *   values.
 *
 * @details
 *   Hibernate allows the MAX17048 to reduce its current draw from
 *   a typical of 23 uA in active mode to 4 uA or less by increasing
 *   the ADC sampling interval from 250 ms to 45 seconds.
 *
 *   Automatic hibernation happens when the CRATE is less than the
 *   user-specified threshold. Similarly, exit from hibernate is
 *   automatic if any single VCELL reading exceeds the user-specified
 *   activity threshold.
 *
 *   As the datasheet notes, writing 0xFFFF to HIBRT forces use of
 *   automatic hibernation; similarly, 0x0000 disables. It follows,
 *   then, that a hybrid use case is possible. Setting the HIBRT
 *   register HIBTHR and ACTTHR fields to 0xFF and 0x0, respectively,
 *   will keep the device in hibernate mode with no way to exit.
 *
 * @note
 *   The driver does not allow a use case where hib_thr = 0% and
 *   act_thr != 0x0 mV, in other words, hibernate is disabled but an
 *   activity level to cause exit from hibernate is specified. The
 *   datasheet does not specify what would happen in this case.
 *
 * @param[in] hib_thr
 *   CRATE threshold below which the MAX17048 enters hibernate
 *   mode as an percent.
 *
 * @param[in] act_thr
 *   Exit hibernate mode when VCELL changes by greater than the
 *   specified amount in millivolts.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 *
 *   @li @ref SL_STATUS_INVALID_PARAMETER if hib_thr = 0 and act_thr != 0.
 ******************************************************************************/
sl_status_t max17048_enable_auto_hibernate(float hib_thr, uint32_t act_thr);

/***************************************************************************//**
 * @brief
 *   Disables automatic hibernation by setting the activity and
 *   CRATE thresholds to 0x0.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_disable_auto_hibernate(void);

/***************************************************************************//**
 * @brief
 *   Set the hibernate threshold level.
 *
 * @details
 *   This function sets the CRATE threshold level evaluated my the
 *   MAX17048 to determine whether or not to enter hibernate mode.
 *
 * @param[in] hib_thr
 *   Hibernate threshold in charge/discharge in percent.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_set_hibernate_threshold(float hib_thr);

/***************************************************************************//**
 * @brief
 *   Get the hibernate threshold level.
 *
 * @note
 *   The driver does not poll the MAX17048 to retrieve this value. At
 *   initialization it is assumed to be the default level of 0x80 and
 *   is tracked in a private variable should it otherwise be changed.
 *
 * @return
 *   @li Hibernate threshold in charge/discharge in percent.
 ******************************************************************************/
float max17048_get_hibernate_threshold(void);

/***************************************************************************//**
 * @brief
 *   Set the activity threshold level.
 *
 * @details
 *   This function sets the VCELL threshold level evaluated my the
 *   MAX17048 to determine whether or not to exit hibernate mode.
 *
 * @param[in] act_thr
 *   Activity threshold (VCELL level change) in millivolts.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_set_activity_threshold(uint32_t act_thr);

/***************************************************************************//**
 * @brief
 *   Get the activity threshold level.
 *
 * @note
 *   The driver does not poll the MAX17048 to retrieve this value. At
 *   initialization it is assumed to be the default level of 0x30 and
 *   is tracked in a private variable should it otherwise be changed.
 *
 * @return
 *   @li Activity threshold (VCELL level) in millivolts.
 ******************************************************************************/
uint32_t max17048_get_activity_threshold(void);

/***************************************************************************//**
 * @brief
 *   Disabling the MAX17048 reset comparator in hibernate mode
 *   reduces current consumption by 0.5 uA.
 *
 * @details
 *   The reset comparator is disabled by writing a 1 to the Dis bit in the
 *   VRESET/ID register.
 *
 * @param[in] enable
 *   @li true - Enable the reset comparator in hibernate mode.
 *   @li false - Disable the reset comparator in hibernate mode.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_enable_reset_comparator(bool enable);

/***************************************************************************//**
 * @brief
 *   Check if the MAX17048 is in hibernate mode.
 *
 * @note
 *   The driver only polls the MAX17048 to determine if the device is
 *   in hibernate mode if the user has enabled it. By default, the
 *   driver manually enters/exits hibernate by writing the appropriate
 *   values to HIBRT upon EM2/3 entry/exit in response to Power Manager
 *   sl_power_manager_subscribe_em_transition_event() notifications.
 *
 * @param[out] hibstat
 *   @li activeMode - Device is in active mode
 *   @li hibernateMode - Device is in hibernate mode
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_get_hibernate_state(max17048_hibstate_t *hibstat);

/***************************************************************************//**
 * @brief
 *   Places the MAX17048 into sleep mode.
 *
 * @details
 *   Sleep mode offers the lowest current consumption, but it cannot
 *   be used as a substitute for hibernate mode. All monitoring is
 *   disabled in sleep mode. Any discharging (or charging) of the
 *   battery, which would happen even if the EFM32/EFR32 is in a low
 *   energy mode, would not be detected and would inject error into
 *   the SOC calculations that will accumulate over time.
 *
 *   The driver places the MAX17048 in sleep mode by first writing a
 *   1 to the EnSleep bit in the the MODE register and then by
 *   writing a 1 to the SLEEP bit in the CONFIG register.
 *
 * @note
 *   Although sleep mode is not suitable for normal operation of the
 *   MAX17048, it would be appropriate in EM4 when the only possible
 *   wake-up event is a power-on reset (POR) caused by battery
 *   replacement.
 *
 * @note
 *   It is possible to force exit from sleep mode by writing a 0 to
 *   the CONFIG register sleep bit. POR also exits sleep mode.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_enter_sleep(void);

/***************************************************************************//**
 * @brief
 *   Forces the MAX17048 to exit sleep mode.
 *
 * @details
 *   Although the proper exit mechanism from sleep mode would be a
 *   POR, as this causes the device to quick start its battery SOC
 *   calculations, it is possible to force exit from sleep mode by
 *   writing a 0 to the SLEEP bit in the CONFIG register.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_exit_sleep(void);

/***************************************************************************//**
 * @brief
 *   Forces the MAX17048 to initiate a power-on reset (POR).
 *
 * @details
 *   In general, POR should be detected using the MAX17048 reset
 *   comparator and its associated interrupt. However, it is possible
 *   to manually initiate a POR by writing 0x5400 to the CMD register.
 *
 * @note
 *   The MAX17048 will issue the reset immediately after the last bit
 *   of the command is clocked into the CMD register and will not ACK
 *   the transfer.
 *
 * @note
 *   The reset command forces all MAX17048 registers to their default
 *   values. The driver must update its private tracking variables to
 *   reflect this.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_force_reset(void);

/***************************************************************************//**
 * @brief
 *   Forces the MAX17048 to initiate a battery quick start.
 *
 * @details
 *   In general, POR should handle most battery swap events. However,
 *   if the battery cannot reach a fully relaxed state during the
 *   initial 17 ms POR window, it is possible to manually force the
 *   MAX17048 to perform initial VCELL measurements.
 *
 *   If the user has enabled hardware quick start by #defining
 *   MAX17048_CONFIG_ENABLE_HW_QSTRT and specifying the GPIO pin to
 *   which the QSTRT input is connected, the driver will use this
 *   option as it offers the lowest latency. Otherwise, the driver
 *   will initiate quick start by writing a 1 to the QuickStart bit
 *   in the MODE register.
 *
 * @note
 *   The rising edge on QSTRT initiates the quick start, however Maxim
 *   does not provide any timing specifications for the pin.
 *   Presumably, if QSTRT were to go low and then high again during
 *   the VCELL sampling window (which looks to be 17 ms based on the
 *   discussion of what happens after POR), another quick start would
 *   be initiated.
 *
 *   For this reason, it would seem that the driver ought to keep
 *   QSTRT high for some nominal amount of time, maybe 1 ms, before
 *   de-asserting it. This should be implemented with a Sleeptimer
 *   one-shot software timer whose callback de-asserts the pin.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_force_quick_start(void);

/***************************************************************************//**
 * @brief
 *   Load a custom model into the MAX17048 and enable it.
 *
 * @details
 *   This function unlocks the TABLE registers by writing 0x57 to address
 *   0x3F and 0x4A to address 0x3E. While the model is unlocked, battery
 *   status is not updated. The TABLE registers should be loaded and
 *   re-locked as quickly as possible by writing 0x00 to address 0x3F and
 *   0x00 to address 0x3E.
 *
 * @note
 *   By definition, a custom model consists of 64 bytes, written to the
 *   MAX17048 TABLE registers 16 bits at a time.
 *
 * @param[in] model
 *   A pointer to the 64-byte model array.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_load_model(const uint8_t *model);

/***************************************************************************//**
 * @brief
 *   Returns the one-time factory-programmable identifier value stored
 *   in the ID register.
 *
 * @param[out] id
 *   The value stored in the ID register.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_get_id(uint8_t *id);

/***************************************************************************//**
 * @brief
 *   Provides the production version of the MAX17048 by reading the
 *   contents of the VERSION register.
 *
 * @param[out] ver
 *   The value stored in the VERSION register.
 *
 * @return
 *   @li @ref SL_STATUS_OK on success.
 ******************************************************************************/
sl_status_t max17048_get_production_version(uint16_t *ver);

/** @} (end addtogroup max17048) */
#ifdef __cplusplus
}
#endif
#endif // MAX17048_H

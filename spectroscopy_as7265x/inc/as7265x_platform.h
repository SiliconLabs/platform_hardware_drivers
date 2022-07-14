/***************************************************************************//**
 * @file as7265x_platform.h
 * @brief AS7265X Platform Header File.
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
 *    misrepresented as being the original software.s
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

/**
 * @file  as7265x_platform.h
 * @brief Those platform functions are platform dependent and have
 *        to be implemented for the integrated platform.
 */

#ifndef _AS7265X_PLATFORM_H_
#define _AS7265X_PLATFORM_H_

#include <stdint.h>
#include "sl_status.h"
#include "sl_i2cspm.h"
#include "sl_sleeptimer.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup as7265x_platform AS7265X Driver Platform */

/***************************************************************************//**
 * @addtogroup as7265x_platform
 * @brief  AS7265X Driver Platform
 * @details
 * @{
 ******************************************************************************/

// Gets current tick count.
#define as7265x_get_tick_count() sl_sleeptimer_get_tick_count()

// Gets current millisecond count.
#define as7265x_get_ms() \
sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count())

// Delay 'ms' milliseconds
#define as7265x_delay_ms(ms) sl_sleeptimer_delay_millisecond(ms)

// Delay 'us' microseconds
#define as7265x_delay_us(us) sl_udelay_wait(us)

/***************************************************************************//**
 * @brief
 *    This function sets the IC2SPM instance used by platform functions.
 *
 * @param[in] i2cspm_instance
 *    I2CSPM instance, default: AS7265X_CONFIG_I2C_INSTANCE.
 *
 ******************************************************************************/
void as7265x_platform_set_i2cspm_instance(sl_i2cspm_t *i2cspm_instance);

/***************************************************************************//**
 * @brief
 *    This function is used to read a register.
 *
 * @param[in] dev
 *    I2C address, default: AS7265X_ADDR.
 * @param[in] addr
 *    Address of register.
 * @param[out] pdata
 *    Pointer of output value.
 * 
 * @return sl_status_t SL_STATUS_OK on success or SL_STATUS_TRANSMIT on failure.
 *
 ******************************************************************************/
sl_status_t as7265x_platform_read_register(uint8_t dev,
                                           uint8_t addr,
                                           uint8_t *pdata);

/***************************************************************************//**
 * @brief
 *    This function is used to read a register.
 *
 * @param[in] dev
 *    I2C address, default: AS7265X_ADDR.
 * @param[in] addr
 *    Address of register.
 * @param[out] pdata
 *    Pointer of output value.
 * 
 * @return sl_status_t SL_STATUS_OK on success or SL_STATUS_TRANSMIT on failure.
 *
 ******************************************************************************/
sl_status_t as7265x_platform_write_register(uint8_t dev,
                                            uint8_t addr,
                                            uint8_t data);

/** @} (end addtogroup as7265x_platform) */

#ifdef __cplusplus
}
#endif

#endif

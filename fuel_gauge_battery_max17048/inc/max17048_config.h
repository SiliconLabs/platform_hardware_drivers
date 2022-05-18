/***************************************************************************//**
* @file max17048_config.h
* @brief MAX17048 Driver Configuration
********************************************************************************
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
#ifndef MAX17048_CONFIG_H_
#define MAX17048_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

// MAX17048/9 default I2C address. Can be different on custom devices.
#define MAX17048_I2C_ADDRESS                0x36

// User-specified stabilization delay
#define MAX17048_CONFIG_STABILIZATION_DELAY 0

// I2C module used; note that I2CSPM handles the transfers
#define MAX17048_CONFIG_I2C                 I2C0

/*
 * GPIO port/pin used for the ALRTn interrupt pin.  This should almost
 * always be a port A/B pin so that it can detect a falling edge while
 * in EM2/3.  If the application firmware does not need to use EM2/3,
 * then a port C/D pin could be used.
 */
#define MAX17048_CONFIG_ALRT_PORT           gpioPortC
#define MAX17048_CONFIG_ALRT_PIN            1

/*
 * In order for the MAX17048 to accurately track state of charge
 * (SOC), it must periodically compensate for battery temperature
 * changes.  The datasheet specifies that this be done no less than
 * once per minute.
 *
 * The driver calculates and updates the RCOMP factor at a rate of
 * 1000 ms <= MAX17048_CONFIG_RCOMP_UPDATE_INTERVAL_MS <= 60000 ms
 * and defaults to 1 minute (60000 ms = 1 minute).
 */
#define MAX17048_CONFIG_RCOMP_UPDATE_INTERVAL_MS    60000

/*
 * The MAX17048/9 has a manual input (QSTRT) to force a battery quick
 * start, which is the initial measurement of cell voltage (VCELL) and
 * state of charge (SOC).  In certain circumstances, quick start is
 * necessary if the bouncing associated initial power-up or battery
 * swap results in an erroneous initial reading of SOC.
 *
 * If enabled, quick start support is conditionally compiled into the
 * driver. The user must also designate a GPIO pin to connect to the
 * QSTRT pin on the MAX17048/9 to manually initiate quick start.
 *
 * The QSTRT pin can be any MCU GPIO pin, including a port C/D pin,
 * because it is an output that is toggled under software control.
 */
#define MAX17048_CONFIG_ENABLE_HW_QSTRT     1

#ifdef MAX17048_CONFIG_ENABLE_HW_QSTRT

#define MAX17048_CONFIG_ENABLE_QSTRT_PORT   gpioPortC
#define MAX17048_CONFIG_ENABLE_QSTRT_PIN    2

#endif /* MAX17048_CONFIG_ENABLE_HW_QSTRT */

#ifdef __cplusplus
}
#endif

#endif /* MAX17048_CONFIG_H_ */

/***************************************************************************//**
 * @file ir_array_amg88xx_driver_config.h
 * @brief SparkFun Grid-EYE Infrared Array Breakout (AMG8833) config file.
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

#ifndef IR_ARRAY_AMG88XX_DRIVER_CONFIG_H_
#define IR_ARRAY_AMG88XX_DRIVER_CONFIG_H_

#include "em_gpio.h"
#include "ir_array_amg88xx_driver.h"

#define AMG88XX_DEFAULT_I2C_INSTANCE      I2C0
#define AMG88XX_DEFAULT_I2C_ADDRESS       AMG88XX_ADDRESS_OPEN

#endif // IR_ARRAY_AMG88XX_DRIVER_CONFIG_H_

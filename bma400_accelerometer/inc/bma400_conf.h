/***************************************************************************//**
 * @file bma400._conf.h
 * @brief Silicon Labs Empty Example Project
 *
 * Configuration filr for the BMA accelerometer driver
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef BMA400_CONFIG_H
#define BMA400_CONFIG_H

/* ########################################################################## */
/*                           System includes                                  */
/* ########################################################################## */

/* ########################################################################## */
/*                        Non system includes                                 */
/* ########################################################################## */
#include "em_gpio.h"

/* ########################################################################## */
/*                             Mecros Defn                                    */
/* ########################################################################## */
#define BMA400_DEBUG_MODE (0) /*Flag used for develpment */

/* BMA400 I2C address macros */
#define BMA400_I2C_ADDRESS_SDO_LOW  (0x14<<1)
#define BMA400_I2C_ADDRESS_SDO_HIGH (0x15<<1)

/* I2C HW defs */
#define BMA400_I2C0_PORT    gpioPortC
#define BMA400_I2C0_PIN_SDA (0)
#define BMA400_I2C0_PIN_SCL (1)

/* ########################################################################## */
/*                            Enum & and Typedefs                             */
/* ########################################################################## */

/* Interface selection enums */
typedef enum bma400_intf {
  eBMA400_Intfc_I2C0=0, /* I2C0 interface */
  eBMA400_Intfc_I2C1,   /* I2C1 interface */
  eBMA400_Intfc_Total   /* I2C1 interface */
} eBMA400_Intfc;

#endif

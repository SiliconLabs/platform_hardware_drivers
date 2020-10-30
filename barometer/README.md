# Barometer driver #

## Overview ##

This project shows the implementation of an I2C pressure sensor driver. A target application of a barometer sensor could be a weather station or an altitude sensor.

This driver contains two different sensor IC drivers: the Infineon **DPS310** and the **MS5637** from TE Connectivity. They have the same public facing header and separate implementations so the user need to decide which one is used in compile time.

## Gecko SDK version ##

GSDK v3.0.0

## Hardware Required ##

- WSTK Mainboard (BRD4001A)
- EFR32xG22 Radio Board (BRD4182A)

Supported barometer boards:
- [Adafruit DPS310 board](https://www.adafruit.com/product/3678](https://www.adafruit.com/product/4494))
- [Sparkfun MS5637 board](https://www.sparkfun.com/products/14688)
- [MikroE Pressure 3 click board](https://www.mikroe.com/pressure-3-click)

## Connections Required ##

A barometer sensor board can be easily connected up with two I2C wires (SDA and SCL) along with 3v3 and GND. For the designated boards, SparkFun qwiic compatible STEMMA QT connectors can be used.

## How It Works ##

These sensors are factory calibrated and contain calibration coefficients that are used in the application to compensate the measurement results for sensor non-linearities. They also have a temperature sensor built in, therefore temperature drift compensation is included in the masurements. 

For the lowest noise readings, it is recommended to take multiple measurements and combine the readings into one result. This increases the current consumption and also the measurement time,
reducing the maximum possible measurement rate. It is done internally in the sensor and the user can specify the oversampling rate (OSR) by configuring the sensor. It is necessary to balance the accuracy and data rate
required for each application with the allowable current consumption. 

![API](doc/barometer_API.png)

The init function resets and configures the sensor, in this way the user only needs to call one init function during startup. The factory-programmed calibration values are also read from the sensor in the init function. After initialization the sensor is in Standby mode. A typical temperature compensated pressure measurement is shown in the following figure.

![Workflow](doc/sensor_workflow.png)

In case of using the high-level measurement API functions (blocking or non-blocking) this whole measurement and compensation process is done internally by the driver. Using the non-blocking function, the user can register a callback function which is called after the measurement and the compensated sensor value calculation is ready. In this case sleeptimer is used for correct timings. 

## Setup ##

To test the barometer application, you need to connect the barometer sensor board to the WSTK's designated I2C EXP header pins. 

Add the barometer folder to your project. Choose which barometer sensor IC you want to use and delete the other one's source file from the src directory.

The driver uses the sleeptimer for timings. Also a higher level kit driver I2CSPM (I2C simple poll-based master mode driver) is used for initializing the I2C peripheral as master mode and performing the I2C transfer. These modules are need to be added to the include paths and its sources need to be copied to the project

1. Copy the barometer directory to your project. Add the inc directory to the project's include paths.
2. Add Sleeptimer source and header files to the project (platform/service/sleeptimer)
3. Add I2CSPM files (platform/driver/i2cspm)
4. Add I2C emlib source and header files to the project (platform/emlib/)
5. Choose the actual sensor IC and delete other's source file from your project
6. Modify the default I2C resources in the barometer_config.h file

## .sls Projects Used ##

barometer_tester.sls


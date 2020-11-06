# Barometer driver #

## Overview ##

This project shows the implementation of **BMA400** accelerometer driver. This driver is primarily design for the I2C bus but will be extended to SPI very shortly. 

## Gecko SDK version ##

GSDK v3.0.0 - GSDK v2.7

## Hardware Required ##

- BGM220 Explorer kit

Supported barometer boards:](https://www.sparkfun.com/products/14688)
- [MikroE Accel 5 click board](https://www.mikroe.com/accel-5-click)

## Connections Required ##

An Accel 5 click accelerometer board can be easily connected up with two I2C wires (SDA and SCL) along with 3v3 and GND. 

## How It Works ##

This sensor is factory calibrated and contain calibration coefficients that are used in the application to compensate the measurement results for sensor non-linearities. They also have a temperature sensor built in, therefore temperature drift compensation is included in the measurements. 

The BMA400 sensor has low energy levels and can also read accelerometer data in different modes. The current driver only uses the high energy mode for reading the data so far. This will be extended subsequently.

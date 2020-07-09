# Z-Wave Motion Sensor PIR Example #

## Summary ##

This project shows the implementation of PIR sensor with Z-Wave.

## Gecko SDK version ##

v2.7.3

## Hardware Required ##

- BRD4001A WSTK board
- BRD4202A Radio board with ZGM130S
- BRD8030A Occupancy sensor EXP board
- UZB7 Controller USB Dongle

## Setup ##

To test the PIR sensor, you need to connect the occupancy sensor EXP board to the WSTK board through the expansion header. Then, you should program the ZGM130S with the MotionSensorPIR.sls project.

To add the node to Z-Wave network, you need to plug the UZB7 Controller USB Dongle into the PC and run the Z-Wave PC controller software.

## How It Works ##

1. Push PB1 on WSTK to enter learn mode to add/remove the device to the network
2. Push PB2 on WSTK to start/stop PIR motion sensor measurements
3. RGB LED on the radio board will turn green if motion is detected

## .sls Projects Used ##

- MotionSensorPIR.sls

## Special Notes ##

Since the BRD8029A (Buttons and LEDs EXP Board) is replaced with occupancy sensor EXP board, buttons and LEDs are limited and moved to WSTK main board. It's required to tie pin P4 and P12 together at the Breakout Pads to allow EM4 wake-up using PB2.

# Human Presence AK9753 Driver #

![Type badge](https://img.shields.io/badge/dynamic/json?url=https://raw.githubusercontent.com/SiliconLabs/application_examples_ci/master/hardware_drivers/human_presence_ak9753_common.json&label=Type&query=type&color=green)
![Technology badge](https://img.shields.io/badge/dynamic/json?url=https://raw.githubusercontent.com/SiliconLabs/application_examples_ci/master/hardware_drivers/human_presence_ak9753_common.json&label=Technology&query=technology&color=green)
![License badge](https://img.shields.io/badge/dynamic/json?url=https://raw.githubusercontent.com/SiliconLabs/application_examples_ci/master/hardware_drivers/human_presence_ak9753_common.json&label=License&query=license&color=green)
![SDK badge](https://img.shields.io/badge/dynamic/json?url=https://raw.githubusercontent.com/SiliconLabs/application_examples_ci/master/hardware_drivers/human_presence_ak9753_common.json&label=SDK&query=sdk&color=green)
[![Required board](https://img.shields.io/badge/Sparkfun-Human%20Presence%20AK9753-green)](https://www.sparkfun.com/products/14349)
![Build badge](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/SiliconLabs/application_examples_ci/master/hardware_drivers/human_presence_ak9753_build_status.json)
![Flash badge](https://img.shields.io/badge/dynamic/json?url=https://raw.githubusercontent.com/SiliconLabs/application_examples_ci/master/hardware_drivers/human_presence_ak9753_common.json&label=Flash&query=flash&color=blue)
![RAM badge](https://img.shields.io/badge/dynamic/json?url=https://raw.githubusercontent.com/SiliconLabs/application_examples_ci/master/hardware_drivers/human_presence_ak9753_common.json&label=RAM&query=ram&color=blue)

## Description ##

The AK9753 is a low power and compact infrared-ray (IR) sensor module. It is composed of four
quantum IR sensors and an integrated circuit (IC) for characteristic compensation. The four IR sensors’
offset and gain variations are calibrated at shipment. An integral analog-to-digital converter provides
16-bits data outputs. The AK9753 is suitable for several feet human detector by using external lens.


The goal is to provide a hardware driver that supports the basic IR measurement readout, along with configuration for the various embedded functionality and interrupt generation. Interfaces should be intuitive and easy to integrate with our wireless stacks, focusing on BLE.

## Gecko SDK version ##

GSDK v4.1.0

## Hardware Required ##

- [A BGM220P Explorer Kit board.](https://www.silabs.com/development-tools/wireless/bluetooth/bgm220-explorer-kit)

- [SparkFun Human Presence AK9753 Board (Qwiic)](https://cdn.sparkfun.com/assets/6/7/9/8/e/AK9753_DS.pdf)

## Connections Required ##

An AK9753 sensor board can be easily connected up with two I2C wires (SDA and SCL) along with 3v3 and GND. For the designated boards, SparkFun Qwiic compatible STEMMA QT connectors can be used.

![hardware_connection](docs/img/hardware_connection.PNG)

**Note:** Normal Mode / Switch Mode selection is controlled by the CAD1 pin and CAD0 pin.
When CAD1 pin and CAD0 pin are set as CAD1 pin= CAD0 pin= “H”, the digital output can be used
through the I2C interface. When CAD1 pin and CAD0 pin are set as CAD1 pin= CAD0 pin= “H”, Switch Mode is selected. When Switch Mode is selected, SCL pin and SDA pin should be tied to “H”. (Do not access the AK9753 through the I2C interface in Switch Mode.)
| CAD1      | CAD0 | I2C output | Slave address |Mode        |
| --------- | ---- | ---------- | ------------- | ---------- |
| L         | L    | Enable     | 64H           | Normal Mode|
| L         | H    | Enable     | 65H           | Normal Mode|
| H         | L    | Enable     | 66H           | Normal Mode|
| H         | H    | Disable    | Prohibited    | Switch Mode|

The second of which is the I2C pull-up jumper. If multiple boards are connected to the I2C bus, the equivalent resistance goes down, increasing your pull-up strength. If multiple boards are connected on the same bus, make sure only one board has the pull-up resistors connected.

![hardware_jumper](docs/img/hardware_jumper.jpg)

## Setup ##

To test this application, you should connect the BMG220 Explorer Kit Board to the PC using a micro-USB cable.

You can either import the provided **human_presence_ak9753_simple_test.sls** project file or start with an empty example project as the basis:

1. Create a "Platform - Empty C Project" project for the "BGM220 Explorer Kit Board" using Simplicity Studio v5. Use the default project settings. Be sure to connect and select the BGM220 Explorer Kit Board from the "Debug Adapters" on the left before creating a project.

2. Copy the files */test/human_presence_ak9753_simple_test/app.c*, */inc/ak9753.h*, */src/ak9753.c*, */src/ak9753_platform.c*, */inc/ak9753.h*, */inc/ak9753_config.h*, */inc/ak9753_platform.h* into the project root folder (overwriting existing app.c).

3. Set the test mode in the *app.c* file.

4. Install the software components:

    - Open the .slcp file in the project.

    - Select the SOFTWARE COMPONENTS tab.

    - Install **[Platform] > [Driver] > [I2CSPM] > [Qwiic]** component with the default instance name: Qwiic.

    - Install the **[Services] > [IO Stream] > [IO Stream: USART]** component with the default instance name: vcom.

    - Install the **[Services] > [IO Stream] > [IO Stream: USART] > [IO Stream: Retarget STDIO]** component.

    - Install the **[Application] > [Utility] > [Log]** component.

    - Install the **[Platform] > [Driver] > [GPIOINT]** component.

5. Build and flash the project to your device.

## How It Works ##

### Normal Mode ###

 There are the eight Modes in Normal Mode.(CAD0 pin= “L” or CAD1 pin= “L”)

1. Power down Mode

2. Stand-by Mode

3. Single shot Mode

4. Continuous Mode 0

5. Continuous Mode 1

6. Continuous Mode 2

7. Continuous Mode 3

8. EEPROM access Mode

![normal_mode](docs/img/normal_mode.JPG)

### Switch Mode ###

 There are two Modes in Switch Mode. (CAD0 pin= CAD1 pin= “H”)

1. Power down Mode
2. Measurement Mode

![switch_mode](docs/img/switch_mode.JPG)

Some functionality of MMA8452Q includes the following:

- Power Down: If present, set the PDN pin to logic low to power down the AK9753

  ```c
  sl_status_t ak9753_power_down(void);
  ```

- Power Up: If present, set the PDN pin to logic high to power up the AK9753

  ```c
  sl_status_t ak9753_power_up(void);
  ```

- Set Mode: Set the AK9753 mode of operation.

  ```c
  sl_status_t ak9753_set_mode(uint8_t mode);
  ```

- Threshold: Set the threshold level for differential output IR2-IR4. Performs the appropriate bit-shift for register settings. 16-bit (pre-shift) value alsostored in local configuration.

  ```c
  sl_status_t ak9753_set_threshold_ir24(bool height, uint16_t thresholdValue);
  ```

- Threshold in EEPROM:  Set the threshold level for differential output IR2-IR4 stored in sensor EEPROM. Performs the appropriate bit-shift for register settings.

  ```c
  sl_status_t ak9753_set_eeprom_threshold_ir24(bool height, uint16_t thresholdValue);
  ```

- Hysteresis threshold: Set the hysteresis of threshold level for differential output IR2-IR4.  Masks only the lower 5 bits. Value also stored in local configuration.

  ```c
  sl_status_t ak9753_set_hysteresis_ir24(uint8_t hysteresisValue);
  ```

- Hysteresis threshold in EEPROM: Set the hysteresis of threshold level for differential output IR2-IR4 stored in sensor EEPROM.

  ```c
  sl_status_t ak9753_set_eeprom_hysteresis_ir24(uint8_t hysteresisValue);
  ```

### API Overview ###

![api_overview](docs/img/api_overview.png)

[ak9753.c](src/ak9753.c): implements APIs for application.

- Initialization and configuration API: specific register read/write to get and set settings for AK9753.

- Read Sensor Data/Status: specific register read to get acceleration data and status.

[ak9753.c](src/ak9753_platform.c): implements APIs for low level.

- Low-level functions: initialize I2C communication, read/write a memory block via I2C, given memory address, and read/write a register via I2C, given register address.

### Testing ###

This simple test application demonstrates the main of the available features of the human presence AK9753 sensor , after initialization, the human presence AK9753 sensor measures the value and return on the serial communication interface.

Please follow the below step to test the example:

- Open a terminal program on your PC, such as the Console that is integrated into Simplicity Studio or a third-party tool terminal like TeraTerm to receive the logs from the virtual COM port.

![simple test result](docs/img/simple_test_result.JPG "simple test result")

Our unit test for the application run with the flow chart below.

![unit test flowchart](docs/img/unit_test_flowchart.PNG "unit test flowchart")

The unit test for all the APIs also was provided with the result below.

![unit test result](docs/img/unit_test_result.PNG "unit test result")

## .sls Projects Used ##

- [**human_presence_ak9753_simple_test.sls**](SimplicityStudio/human_presence_ak9753_simple_test.sls)
- [**human_presence_ak9753_unit_test.sls**](SimplicityStudio/ak9753_unit_test.sls)
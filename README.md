# Hardware Drivers

## Overview

The projects in this repository are drivers for external hardware such as sensors, displays, or transmitters that would commonly be used with Silicon Labs products. These drivers are intended to demonstrate the use of external hardware with Silicon Labs devices and to allow customers to use that external hardware in their wireless projects. Compatibility with wireless stacks is a major goal of this project.

#### Driver Naming Conventions

Driver directories are named first with the peripheral device type, followed by the name of the specific device name interfaced by the driver e.g. ./audio_dac_UDA1334/

Option A:
Header files, source files, and function prefixes will then reference the device name, in order to emphasize the specific compatibility with that driver, e.g. uda1334.h, uda1334.c, uda1334_init()

Option B:
Header files, source files, and function prefixes will then reference the driver type, in order to emphasize general compatibility with that type of device, e.g. audio_dac.h, audio_dac.c, audio_dac_init()

#### Directory Structure

driver/  <br/><br/>
├── doc/  <br/><br/>
│   ├── optional_doxygen  <br/><br/>
│   └── project.xml   <br/><br/>
├── SimplicityStudio/  <br/><br/>
│    ├── driver_simple_efx32xx.slsproj   <br/><br/>
│    └── driver_test_efx32xx.slsproj   <br/><br/>
├── IAR/  <br/><br/>
│    ├── driver_app_efx32xx.ewp  <br/><br/>
│    └── driver_app_efx32xx.eww  <br/><br/>
├── src/  <br/><br/> 
│    └── driver.c  <br/><br/>
├── inc/  <br/><br/>
│    ├── driver_config.h <br/><br/>
│    └── driver.h <br/><br/> 
├── test/ <br/><br/>
│    ├── driver_simple.c <br/><br/>
│    └── driver_test.c <br/><br/>
├── README.md <br/><br/>

doc/ contains the build system file, as well as generated doxygen

SimplicityStudio/ contains studio project files for sample app and unit tests

IAR/ contains IAR project files for sample app

src/ contains core driver src files. Should be copied/linked to include driver in a target project

inc/ contains core driver inc files. Should be copied/linked to include driver in a target project

test/ contains source files for test programs. \_simple.c is a simple use case for the driver, \_test.c is the unit test runner.

README.md contains driver description, setup instructions, and links to high level documentation (docs.silabs.com or wiki)



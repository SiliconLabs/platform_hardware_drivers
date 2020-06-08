## Device Drivers

#### Driver Naming Conventions

Driver directories are named first with the peripheral device type, followed by the name of the specific device name interfaced by the driver e.g. ./audio_dac_UDA1334/

Option A:
Header files, source files, and function prefixes will then reference the device name, in order to emphasize the specific compatibility with that driver, e.g. uda1334.h, uda1334.c, uda1334_init()

Option B:
Header files, source files, and function prefixes will then reference the driver type, in order to emphasize general compatibility with that type of device, e.g. audio_dac.h, audio_dac.c, audio_dac_init()

#### Directory Structure

driver/
├── doc/
│   ├── optional_doxygen
│   └── project.xml
├── SimplicityStudio/
│    ├── driver_simple_efx32xx.slsproj
│    └── driver_test_efx32xx.slsproj
├── IAR/
│    ├── driver_app_efx32xx.ewp
│    └── driver_app_efx32xx.eww
├── src/
│    └── driver.c
├── inc/
│    ├── driver_config.h
│    └── driver.h 
├── test/
│    ├── driver_simple.c
│    └── driver_test.c
├── README.md

doc/ contains the build system file, as well as generated doxygen

SimplicityStudio/ contains studio project files for sample app and unit tests

IAR/ contains IAR project files for sample app

src/ contains core driver src files. Should be copied/linked to include driver in a target project

inc/ contains core driver inc files. Should be copied/linked to include driver in a target project

test/ contains source files for test programs. \_simple.c is a simple use case for the driver, \_test.c is the unit test runner.

README.md contains driver description, setup instructions, and links to high level documentation (docs.silabs.com or wiki)



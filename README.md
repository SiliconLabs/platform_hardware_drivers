<table border="0">
  <tr>
    <td align="left" valign="middle">
    <h1>EFM32 and EFR32<br/>Hardware Drivers</h1>
  </td>
  <td align="left" valign="middle">
    <a href="https://www.silabs.com/products/wireless">
      <img src="http://pages.silabs.com/rs/634-SLU-379/images/WGX-transparent.png"  title="Silicon Labs Gecko and Wireless Gecko MCUs" alt="EFM32 32-bit Microcontrollers" width="250"/>
    </a>
  </td>
  </tr>
</table>

# Silicon Labs Hardware Drivers #

This repo contains example drivers developed by Silicon Labs engineers for EFM32 and EFR32 devices. These drivers interface with different types of external hardware and are tested against a specific device listed in the driver name. These drivers are intended to be fully compatible with Silicon Labs' wireless stacks. All drivers provide a minimal Simplicity Studio project for testing and to show an example usage of the driver. The driver source is provided for modification and extension as needed. All drivers in this repository are considered to be EVALUATION QUALITY which implies that the code provided in the repos is subjected to limited testing and is provided provided as-is. It is not suitable for production environments.

## Requirements ##

1. Gecko SDK Suite, available via Simplicity Studio
2. Simplicity Studio IDE
3. Driver specific test hardware

## Using Hardware Drivers

All drivers consist of a single .h and .c file, with an optional user defined \*\_config.h file defining board and device specific dependencies. To include a driver in a Simplicity Studio project, paste or link the .c and .h files into the target project and add all required Gecko SDK dependencies. Additionally if needed, define a \*\_config.h file with parameters suited for the target design. 

## Documentation

Driver documentation is contained in each driver specific readme file, as well as Doxygen available in the driver header file.

## Reporting Bugs/Issues and Posting Questions and Comments ##

To report bugs, please create a new "Issue" in the "Issues" section of this repo.  Please reference the board, project, and source files associated with the bug, and reference line numbers.  If you are proposing a fix, also include information on the proposed fix in the Issue description. Currently this repository does not accept pull requests.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of this repo.

## Disclaimer ##

The Gecko SDK suite supports development with Silicon Labs IoT SoC and module devices. All drivers in this repository are considered to be EVALUATION QUALITY which implies that the code provided in the repos is subjected to limited testing and is provided provided as-is. It is not suitable for production environments.
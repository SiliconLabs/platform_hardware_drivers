/***************************************************************************//**
* @file triacdrv.h
* @brief ACMP/PRS/TIMER-driven automated triac driver
*******************************************************************************
* # License
* <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef TRIACDRV_H
#define TRIACDRV_H

#include <stdbool.h>
#include <stdint.h>

// Device header
#include "em_device.h"

// emlib
#include "em_acmp.h"
#include "em_cmu.h"
#include "em_prs.h"
#include "em_timer.h"

// Platform status
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

  /***************************************************************************//**
   * @addtogroup triac Triac Driver
   * @brief Triac Driver
   *
   * ## Overview
   *
   *  TRIACDRV is a hardware device driver for Series 1 EFM32 and EFR32
   *  devices that uses an appropriate voltage level representation of
   *  the AC line voltage in order to fire a timed pulse to drive the
   *  gate on a triac.
   *
   *  By using the PRS to connect one of the device's analog
   *  comparators (ACMP) with one of its timers (TIMERs), the triac
   *  gate enable pulse can be fired automatically and with zero CPU
   *  intervention. In addition to initializing the necessary hardware,
   *  the driver includes functions to get and set the duty cycle
   *  (between 0 and 100% in 1% increments) and to monitor the half-wave
   *  duty cycle periods as a kind of calibration to account for some
   *  level of potential variation in the incoming DC waveform due to
   *  poor line conditions.
   *
   *  The driver is written for Gecko SDK v2.7.x. While it has not been
   *  tested, nothing in the driver or the example demonstration
   *  project should preclude it from being built against Gecko SDK
   *  v3.x.
   *
   *  As mentioned above, the driver runs on Series 1 EFM32 and EFR32
   *  devices.  The analog comparator, peripheral reflex system (PRS),
   *  and timer on Series 2 devices have all of the necessary
   *  functionality to implement the technique used in this driver.
   *  However, substantial additions of conditionally compiled code
   *  would be required to handle the differences in analog and digital
   *  pin routing (ABUS and DBUS) and module initialization between
   *  Series 1 and 2 devices.
   *
   * ## Initialization
   *
   *  TRIACDRV uses a structure of type TRIACDRV_Init_TypeDef for
   *  initialization. It holds all of the chip hardware resources
   *  needed for operation, as well as parameters that specify the
   *  characteristics of the AC line waveform input to the analog
   *  comparator and the output characteristics of the triac gate
   *  enable pulse.
   *
   *  All initialization is performed by triacdrv_init() using this
   *  structure. No other hardware initialization is required.
   *  Furthermore, TRIACDRV prescales the TIMER clock based on the
   *  frequency of the HFPERCLK that is in effect when triacdrv_init()
   *  is called. Therefore, the HFPERCLK frequency cannot be changed
   *  (e.g. by changing the HFCLK frequency) once TRIACDRV is
   *  initialized.
   *
   * ## Hardware Configuration
   *
   *  As noted above, triacdrv_init() performs all hardware
   *  setup with the data passed in the structure of type
   *  TRIACDRV_Init_TypeDef.
   *
   *  On the hardware front, TRIACDRV requires:
   *
   *  - One analog comparator and one analog input pin
   *
   *  - One 16-bit TIMER, of which 3 channels are used. Unused
   *    channels cannot be used elsewhere in the end user
   *    application.
   *
   *  - Three PRS channels, allocated as follows:
   *
   *    + One channel that connects the ACMP output to the TIMER input
   *
   *    + Two adjacent channels for the gate enable output pulse. In
   *      this sense, adjacent means the channels are numbered N and
   *      N + 1, such that N + 1 < PRS_CHAN_COUNT (defined in the
   *      device-specific header file). Furthermore, neither of these
   *      can be the same PRS channel allocated to the ACMP output.
   *
   *  - One digital output pin corresponding to the base PRS channel
   *    allocated to the chosen TIMER, e.g. for channels 8 and 9, the
   *    digital PRS_CH8 pin is needed and must be available.
   *
   *  The default provided by the TRIAC_INIT_DEFAULT initializer does
   *  not apply to any specific EFM32 or EFR32 board but is useful to
   *  see how the individual parameters for any given implementation
   *  need to be configured
   *
   * ## Operation
   *
   *  After calling triacdrv_init(), the driver is fully operational
   *  and running with either 0% or 100% duty cycle, depending on the
   *  setting of the initOn member of the initialization structure.
   *
   *  While the output duty cycle can be changed immediately by calling
   *  triacdrv_set_duty_cycle() with an integer parameter between 0 and
   *  100%, it is recommended to first call triacdrv_calibrate() for
   *  some number of 60 Hz cycles in order to tune the detected maximum
   *  and minimum half-wave lengths.
   *
   * ## Energy Modes
   *
   *  Because the TIMER counter must run in order to output gate enable
   *  pulses in response to zero-crossing events, TRIACDRV is restricted
   *  to running in EM0 or EM1. To enter EM2 (or lower), application
   *  software should first set the output duty cycle to 0% by calling
   *  triacdrv_set_duty_cycle(). Upon returning to EM0, there is no
   *  need to re-initialize the driver; the output can be re-enabled
   *  simply by setting the duty cycle to a non-zero value.
   *
   * ## Usage
   *
   *  See test/main.c for a simple example that shows how to use the
   *  driver.
   *
   *  A working project is provided in the SimplicityStudio directory
   *  that can be imported, built, and downloaded to the EFM32TG11
   *  Starter Kit. The LETIMER is initialized to provide a 60 Hz
   *  stimulus output on the PC14 pin (EXP header pin 12) that should
   *  be connected to the PC0 pin (EXP header pin 3) with a jumper
   *  wire. The triac gate enable pulse is driven on PC8 (EXP header
   *  pin 10) and can be observed along with the 60 Hz stimulus square
   *  wave on an oscilloscope.
   *
   * ## Additional Details
   *
   *  See the README.md file for a comprehensive explanation of how
   *  the driver functions, input requirements, implementation notes,
   *  and porting guidance.
   *
   * @{
   ******************************************************************************/

// TRIAC drive input waveform type
typedef enum
{
  triacInputRectifiedSine,   // Rectified sine wave (no negative half wave) input
  triacInputOffsetSine,      // Offset sine wave with zero-crossing at 50%
} TRIACDRV_InputWave_Typedef;

// TRIACDRV initialization structure
typedef struct
{
  // ACMP used for zero crossing detection
  ACMP_TypeDef                *acmp;

  // APORT ACMP input channel
  ACMP_Channel_TypeDef        acmpInput;

  // PRS channel used to link ACMP and TIMER
  uint32_t                    acmpPrsChannel;

  // TIMER used for half-wave measurement and output generation
  TIMER_TypeDef               *timer;

  /*
   * Base PRS channel used for gate enable output pulse.  Two
   * adjacent channels (N and N + 1) are ANDed together to form
   * the output pulse.
   */
  uint32_t                    timerPrsBaseChannel;

  /*
   * Output port, pin, and location for timerPrsBaseChannel.  This
   * must be one of the PRS_CH pin options for the PRS channel
   * selected, e.g. if PRS channel 4 is selected as the
   * timerPrsBaseChannel on EFM32TG11, then the pin options are PC8
   * and PF1.
   */
  GPIO_Port_TypeDef           timerPrsOutputPort;
  uint32_t                    timerPrsOutputPin;
  uint32_t                    timerPrsOutputLoc;

  // Input waveform type
  TRIACDRV_InputWave_Typedef  inputWave;

  // Threshold voltage for zero-crossing in mV
  uint32_t                    zeroThreshold;

  // AVDD supply voltage in mV
  uint32_t                    avdd;

  // Gate enable pulse width in microseconds
  uint32_t                    pulseWidth;

  // Triac initially on?
  bool                        initOn;
} TRIACDRV_Init_TypeDef;

// Maximum allowable gate enable pulse width in microseconds
#define TRIACDRV_MAX_ENABLE_PULSE_WIDTH   200

// Minimum/maximum allowable zero-crossing threshold in millivolts
#define TRIACDRV_MIN_ZERO_THRESHOLD       5
#define TRIACDRV_MAX_ZERO_THRESHOLD       500

// Minimum allowable TIMER clock frequency
#define TRIACDRV_MIN_TIMER_FREQUENCY      1000000

/*
 * Minimum and maximum allowable AVDD voltages.  AVDD can be as low as
 * 1.8V on most Series 1 devices but is limited to 1.85V on EFM32xG1
 * and EFR32xG1.
 */
#define TRIACDRV_MIN_AVDD                 1800
#define TRIACDRV_MAX_AVDD                 3800

// Default triac initialization structure
#define TRIAC_INIT_DEFAULT                                                              \
{                                                                                       \
  ACMP0,                    /* ACMP0 for zero crossing detection */                     \
  acmpInputAPORT2XCH27,     /* ACMP0 input pin */                                       \
  0,                        /* PRS channel 0 to link ACMP with TIMERn */                \
  TIMER1,                   /* TIMER1 for frequency input and gate enable output */     \
  1,                        /* PRS channel 1/2 for output pulse generation */           \
  gpioPortF,                /* PRS CH1 on port F */                                     \
  7,                        /* PRS_CH1 on pin PF7 */                                    \
  6,                        /* PRS_CH1 on PF7 is location 6 */                          \
  triacInputRectifiedSine,  /* Rectified sine wave input */                             \
  200,                      /* 200 mV zero crossing threshold */                        \
  3300,                     /* AVDD = 3.3V */                                           \
  20,                       /* gate enable pulse width */                               \
  false,                    /* Initially off (duty cycle = 0%) */                       \
}

// Maximum number of half-waves that can be counted during calibration
#define TRIACDRV_MAX_CAL_COUNT   60

// Function prototypes (user API)
sl_status_t triacdrv_calibrate(uint32_t count);
uint32_t triacdrv_get_duty_cycle(void);
sl_status_t triacdrv_set_duty_cycle(uint32_t duty);
sl_status_t triacdrv_init(const TRIACDRV_Init_TypeDef *init);

/** @} (end addtogroup triac) */

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* TRIACDRV_H */

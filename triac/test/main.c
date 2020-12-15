/***************************************************************************//**
* @file main.c
* @brief ACMP/PRS/TIMER-driven automated triac driver demonstration.
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

// Device header
#include "em_device.h"

// emlib
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_letimer.h"

// TRIACDRV
#include "triacdrv.h"

#include "bsp.h"

/*
 * Using the LFXO for the LETIMER clock, 32768/120 = 273.066666.  Note
 * that the output low/high time has to be half of one 60 Hz period,
 * thus the 120 Hz output overflow frequency for the LETIMER.  Use 273
 * for the counter top value.  Use 328 to get a 50 Hz output.
 */
#define LETIMER_TOP 273

// Duty cycle value
uint32_t triacDuty;   // initially off

/**************************************************************************//**
 * @brief
 *   Initialize the LETIMER to output a 50/60 Hz square wave for testing.
 *****************************************************************************/
void letimerLineHz(void)
{
  LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;

  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

  CMU_ClockEnable(cmuClock_LETIMER0, true);

  // Enable LETIMER0 output0 on PC14 (EXP header pin 12)
  LETIMER0->ROUTEPEN |=  LETIMER_ROUTEPEN_OUT0PEN;
  LETIMER0->ROUTELOC0 = LETIMER_ROUTELOC0_OUT0LOC_LOC5;

  // Reload COMP0 on underflow, pulse output, and run in repeat mode
  letimerInit.comp0Top = true;
  letimerInit.ufoa0 = letimerUFOAToggle;
  letimerInit.topValue = LETIMER_TOP;

  // Need REP0 != 0 to pulse on underflow
  LETIMER_RepeatSet(LETIMER0, 0, 1);

  // Initialize and enable LETIMER
  LETIMER_Init(LETIMER0, &letimerInit);
}

void initCMU(void)
{
  CMU_LFXOInit_TypeDef lfxoinit = CMU_LFXOINIT_DEFAULT;

  CMU_ClockEnable(cmuClock_HFLE, true);

  CMU_LFXOInit(&lfxoinit);

  CMU_HFXOInit_TypeDef hfxoinit = CMU_HFXOINIT_DEFAULT;
  CMU_HFXOInit(&hfxoinit);
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
}

void initGPIO(void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);

  /*
   * Use minimum slew rate alternate setting for port C pins.  This
   * results in a 100 ns rise/fall time when viewed on a scope.  The
   * default primary slew rate of 5 is retained for other puns.
   */
  GPIO_SlewrateSet(gpioPortC, 5, 0);
  GPIO_PinModeSet(gpioPortC, 14, gpioModePushPullAlternate, 0);

  // Enable PB0 and PB1 pins as input with filter enabled
  GPIO_PinModeSet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, gpioModeInput, 1);
  GPIO_PinModeSet(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN, gpioModeInput, 1);

  // Clear GPIO interrupt flags
  GPIO_IntClear(1 << BSP_GPIO_PB0_PIN);
  GPIO_IntClear(1 << BSP_GPIO_PB1_PIN);

  // Enable GPIO_NVIC source; both buttons on the TG11 board are on odd pins
  NVIC_EnableIRQ(GPIO_ODD_IRQn);

  // Enable pin interrupts interrupt
  GPIO_ExtIntConfig(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, BSP_GPIO_PB0_PIN, false, true, true);
  GPIO_ExtIntConfig(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN, BSP_GPIO_PB1_PIN, false, true, true);
}

int main(void)
{
  TRIACDRV_Init_TypeDef triacInit = TRIAC_INIT_DEFAULT;

  CHIP_Init();

  // Enable HFLE clock and LFXO
  initCMU();

  initGPIO();

  letimerLineHz();

  triacInit.acmpInput = acmpInputAPORT0YCH0;  // PC0, EXP header pin 3
  triacInit.acmpPrsChannel = 3;
  triacInit.timerPrsBaseChannel = 4;
  triacInit.timerPrsOutputPort = gpioPortC;
  triacInit.timerPrsOutputPin = 8;
  triacInit.timerPrsOutputLoc = 0;            // PC8, EXP header pin 10
  triacInit.pulseWidth = TRIACDRV_MAX_ENABLE_PULSE_WIDTH;

  // Halt if initialization fails
  if (triacdrv_init(&triacInit) != SL_STATUS_OK)
    __BKPT(0);

  // Set initial duty cycle
  triacDuty = 25;
  triacdrv_set_duty_cycle(triacDuty);

  // Calibrate
  triacdrv_calibrate(60);

  // Sit in EM1 and wait for button presses
  while (1)
    EMU_EnterEM1();
}

void GPIO_ODD_IRQHandler(void)
{
  uint32_t flags= GPIO_IntGet();

  /*
   * Need to see which button was actually pressed.  Buttons 0 and 1
   * are connected to PD5 and PC9, respectively, so they both trigger
   * the GPIO_ODD interrupt.  However, because they use different pin
   * numbers, it's the button pressed can be determined simply by
   * checking which interrupt flag is set (PB1 is given priority).
   */
  if ((flags & (1 << BSP_GPIO_PB1_PIN)))
    // PB0 pressed; reduce duty cycle by 5%
    if (triacDuty > 0)
      triacDuty -= 5;
    else
      triacDuty = 0;
  // PB1 pressed; increase duty cycle by 5%
  else
    if (triacDuty < 100)
      triacDuty += 5;
    else
      triacDuty = 100;

  triacdrv_set_duty_cycle(triacDuty);

  GPIO_IntClear(flags);
}

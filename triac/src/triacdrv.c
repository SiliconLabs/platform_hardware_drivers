/***************************************************************************//**
* @file triacdrv.c
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

#include "triacdrv.h"

#ifdef TRIACDRV_DEBUG
#include "triacdrv_debug.h"
#endif

// Function prototypes (private API)
#ifndef TRIACDRV_DISABLE_HW_RESOURCE_CHECKING
sl_status_t triacdrv_initCheckValues(uint32_t avdd, uint32_t threshold, uint32_t width, uint32_t acmp_ch, uint32_t timer_ch);
#else
sl_status_t triacdrv_initCheckValues(uint32_t avdd, uint32_t threshold, uint32_t width);
#endif
#ifndef TRIACDRV_DISABLE_HW_RESOURCE_CHECKING
sl_status_t triacdrv_initCheckPrsGPIO(uint32_t channel, GPIO_Port_TypeDef port, uint32_t pin, uint32_t location);
#endif /* TRIACDRV_DISABLE_HW_RESOURCE_CHECKING */
sl_status_t triacdrv_initPRS(ACMP_TypeDef *acmp, uint32_t acmpChannel, TIMER_TypeDef *tmr, uint32_t tmrChannel);
void triacdrv_initTIMER(TIMER_TypeDef *tmr, uint32_t prsChannel, uint32_t pwidth, bool initOn);
void triacdrv_initACMP(ACMP_TypeDef *acmp, ACMP_Channel_TypeDef acmpInput, TRIACDRV_InputWave_Typedef waveType, uint32_t avdd, uint32_t threshold);

// Global variables
static bool               triacdrvIsInitialized = false;
static uint32_t           triacdrvTimerFreq;
static TIMER_TypeDef      *triacdrvTimer;
static uint32_t           triacdrvPulseWidthTicks;
static volatile uint32_t  triacdrvDutyPercent = 0;
static volatile uint32_t  triacdrvPulseRiseTime;
static volatile uint32_t  triacdrvPulseFallTime;

/*
 * Tracking for the maximum and minimum measured half-cycle time.
 * These are set to the maximum possible value.  Because the initial
 * duty cycle can only be 0% or 100%, this has no impact on how the
 * system behaves.  When the first call to triacdrv_set_duty_cycle() is
 * made, these are sanity checked and adjusted.
 */
static volatile uint32_t  triacdrvHalfWaveMax = _TIMER_CNT_MASK;
static volatile uint32_t  triacdrvHalfWaveMin = _TIMER_CNT_MASK;

/***************************************************************************//**
 * @brief
 *   Measure a user-specified number of half-cycles to determine the
 *   minimum and maximum lengths.
 *
 * @details
 *
 * @note
 *
 * @param[in] count
 *   An integer cycle count less than 60.
 ******************************************************************************/
sl_status_t triacdrv_calibrate(uint32_t count)
{
  uint32_t length;

  if (triacdrvIsInitialized == false)
    return SL_STATUS_NOT_INITIALIZED;

  if (count > TRIACDRV_MAX_CAL_COUNT)
    return SL_STATUS_INVALID_RANGE;

  while (count > 0)
  {
    // Wait for a half-cycle to be measured
    while ((TIMER_IntGet(triacdrvTimer) & TIMER_IF_CC0) == 0);

    /*
     * See if most recently captured half-cycle count is shorter than
     * the last one saved.  This code works on the premise that the
     * gate enable is fired some amount of time AFTER the last
     * zero-crossing, so it tries to keep the duty cycle adjusted
     * relative to the smallest measured half-cycle time.
     */
    length = TIMER_CaptureGet(triacdrvTimer, 0);

    if (length < triacdrvHalfWaveMin)
      triacdrvHalfWaveMin = length;

    if (length > triacdrvHalfWaveMax)
      triacdrvHalfWaveMax = length;

    count--;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *   Set the triac gate enable duty cycle.
 *
 * @details
 *
 * @note
 *
 * @param[in] duty
 *   An integer duty cycle (PWM output duty cycle) between 0 and 100%.
 ******************************************************************************/
sl_status_t triacdrv_set_duty_cycle(uint32_t duty)
{
  uint32_t length;

  if (triacdrvIsInitialized == false)
    return SL_STATUS_NOT_INITIALIZED;

  // Make sure duty cycle is not more than 100%
  if (duty > 100)
    return SL_STATUS_INVALID_RANGE;

  /*
   * Make sure the half cycle times are sane values.  If not, measure
   * an incoming half-cycle.
   */
  if (triacdrvHalfWaveMin == _TIMER_CNT_MASK)
  {
    while ((TIMER_IntGet(triacdrvTimer) & TIMER_IF_CC0) == 0);

    triacdrvHalfWaveMin = TIMER_CaptureGet(triacdrvTimer, 0);
    triacdrvHalfWaveMax = triacdrvHalfWaveMin;
  }
  /*
   * Already have a measured half-cycle.  See if most recently
   * captured half-cycle count is shorter than the last one saved.
   * This code works on the premise that the gate enable is fired some
   * amount of time AFTER the last zero-crossing, so it tries to
   * keep the duty cycle adjusted relative to the smallest measured
   * half-cycle time.
   */
  else
  {
    length = TIMER_CaptureGet(triacdrvTimer, 0);

    if (length < triacdrvHalfWaveMin)
      triacdrvHalfWaveMin = length;

    if (length > triacdrvHalfWaveMax)
      triacdrvHalfWaveMax = length;
  }

  // Calculate the new match times from the wavelength
  if (duty > 0)
  {
    triacdrvPulseRiseTime = (100 - duty) * (triacdrvHalfWaveMin / 100);
    triacdrvPulseFallTime = triacdrvPulseRiseTime + triacdrvPulseWidthTicks;
  }
  else
  {
    triacdrvPulseRiseTime = triacdrvHalfWaveMax;
    triacdrvPulseFallTime = triacdrvHalfWaveMax;
  }

  // Set the new duty cycle and firing time
  triacdrvDutyPercent = duty;
  TIMER_CompareSet(triacdrvTimer, 1, triacdrvPulseRiseTime);
  TIMER_CompareSet(triacdrvTimer, 2, triacdrvPulseFallTime);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *   Return the triac gate enable duty cycle.
 *
 * @details
 *
 * @note
 *
 * @param
 *   None.
 ******************************************************************************/
uint32_t triacdrv_get_duty_cycle(void)
{
  return triacdrvDutyPercent;
}

/***************************************************************************//**
 * @brief
 *   Initialize TRIACDRV.
 *
 * @details
 *
 * @note
 *
 * @param[in] init
 *   A pointer to the TRIACDRV initialization structure.
 ******************************************************************************/
sl_status_t triacdrv_init(const TRIACDRV_Init_TypeDef *init)
{
  sl_status_t rc = SL_STATUS_OK;

  // If already initialized, return OK
  if (triacdrvIsInitialized == true)
    return rc;

  /*
   * Check the user-specified integer values in the initialization
   * structure to make sure they are valid (PRS channel numbers,
   * zero-crossing threshold voltage, and gate enable pulse width).
   */
#ifndef TRIACDRV_DISABLE_HW_RESOURCE_CHECKING
  rc = triacdrv_initCheckValues(init->avdd,
                                init->zeroThreshold,
                                init->pulseWidth,
                                init->acmpPrsChannel,
                                init->timerPrsBaseChannel);
#else
  rc = triacdrv_initCheckValues(init->avdd,
                                init->zeroThreshold,
                                init->pulseWidth);
#endif

  if (rc != SL_STATUS_OK)
    return rc;

  // Verify that the PRS_CHn output port, pin, and location are valid.
#ifndef TRIACDRV_DISABLE_HW_RESOURCE_CHECKING
  rc = triacdrv_initCheckPrsGPIO(init->timerPrsBaseChannel,
                                 init->timerPrsOutputPort,
                                 init->timerPrsOutputPin,
                                 init->timerPrsOutputLoc);

  if (rc != SL_STATUS_OK)
    return rc;
#endif

  /*
   * Initialize the PRS and clocks next.  This function also verifies
   * that the selected ACMP and TIMER are actually available on the
   * chip, and, assuming they are, enables their clocks.
   *
   * The intent here is that if the selected ACMP and/or TIMER is/are
   * not available, no hardware initialization is performed, and
   * triacdrv_init() can return an error code to the main program.
   *
   * While a somewhat arbitrary value, the TIMER clock frequency is
   * also checked to make sure it is at least 1 MHz so that the duty
   * cycle calculations have sufficient resolution.  In theory, a lower
   * limit could be chosen, but this might require the prescaler
   * calculation in triacdrv_initTIMER() to return an error, and the
   * whole purpose of handling clock and PRS initialization together is
   * to avoid having to undo any hardware configuration later.
   */
  rc = triacdrv_initPRS(init->acmp,
                        init->acmpPrsChannel,
                        init->timer,
                        init->timerPrsBaseChannel);

  if (rc != SL_STATUS_OK)
    return rc;

  /*
   * All resouces and values check out; proceed with initialization by
   * setting up the PRS_CHn GPIO pin and assigning it to the specified
   * channel.
   */
  GPIO_PinModeSet(init->timerPrsOutputPort, init->timerPrsOutputPin, gpioModePushPull, 0);
  PRS_GpioOutputLocation(init->timerPrsBaseChannel, init->timerPrsOutputLoc);

  // Initialize the selected ACMP with the specified input and threshold
  triacdrv_initACMP(init->acmp, init->acmpInput, init->inputWave, init->avdd, init->zeroThreshold);

  // Initialize the specified TIMER
  triacdrv_initTIMER(init->timer,
                     init->acmpPrsChannel,
                     init->pulseWidth,
                     init->initOn);

  // Change drive state to initialized
  triacdrvIsInitialized = true;

  return rc;
}

/***************************************************************************//**
 * Private TIMER initialization function called by triacdrv_init().
 ******************************************************************************/
void triacdrv_initTIMER(TIMER_TypeDef *tmr,
                        uint32_t prsChannel,
                        uint32_t pwidth,
                        bool initOn)
{
  uint32_t tmrpresc, inputCount;
  bool prescFound = false;
  uint64_t preclk, pwprod;

  /*
   * Find the smallest possible TIMER prescaler that permits counting
   * the 50/60 Hz input without overflowing.  The selected TIMER
   * frequency has previously been determined to be at least 1 MHz.
   */
  tmrpresc = timerPrescale1;

  while (!prescFound)
  {
    /*
     * Divide the TIMER frequency by the prescaler, then divide this
     * by 50 Hz (which requires more TIMER counts than does 60 Hz).
     * This needs to be less than the maximum possible timer count
     * (e.g. 0xFFFF for a 16-bit TIMER).
     */
    inputCount = (triacdrvTimerFreq >> tmrpresc) / 50;

    if (inputCount < _TIMER_CNT_MASK)
      prescFound = true;
    else
      tmrpresc++;
  }

  /*
   * Determine if the gate enable pulse width is valid and convert it
   * to a number of prescaled TIMER clock ticks per:
   *
   * (TIMER clock frequency / TIMER prescaler) * (gate enable pulse width in microseconds)
   * -------------------------------------------------------------------------------------
   *                                       1,000,000
   *
   * This is done using 64-bit math and cast to a 32-bit result in
   * the end as the timer registers are 32 bits wide.
   */
  // Calculate the prescaled TIMER clock
  preclk = (uint64_t)triacdrvTimerFreq / (uint64_t)(1 << tmrpresc);

  // Calculate the top half of the tick equation:
  pwprod = preclk * (uint64_t)pwidth;

  // Pulse width tick count
  triacdrvPulseWidthTicks = (uint32_t)(pwprod / (uint64_t)1000000);

  /*
   * Initialize timer but do not enable it yet.  The counter will start
   * COUNTING when CC0 is initialized and the first edge is detected,
   * so this has to be done first.
   */
  TIMER_Init_TypeDef timerinit = TIMER_INIT_DEFAULT;
  timerinit.fallAction = timerInputActionReloadStart;
  timerinit.riseAction = timerInputActionReloadStart;
  timerinit.prescale = tmrpresc;
  timerinit.enable = false;
  timerinit.dmaClrAct = true;

  TIMER_Init(tmr, &timerinit);

  /*
   * Note that counter reload-start is not an event like overflow
   * and underflow.  This means that (a) buffered register values
   * (CCVB and TOPB) are not transfered to the working registers
   * (CCV and TOP), and the TIMER_CC_CTRL_COIST bit does not take
   * effect when the counter restarts.
   *
   * Because the zero-crossing event reloads and restarts the
   * counter, the output pin that drives the triac gate enable must
   * reset back to its inactive state.  This is done by setting
   * TIMER_CTRL_RSSCOIST, which must be done manually because the
   * TIMER_Init_TypeDef does not include a setting for it.
   */
  tmr->CTRL |= TIMER_CTRL_RSSCOIST;

  /*
   * Each ACMP zero-crossing rising/falling edge detection results in
   * a PRS pulse.  Setup CC0 to detect the rising edge of this pulse
   * only.  These pulses are one HFCLK wide, so there is no need/reason
   * to detect the falling edge, too.  CC0 captures this edge (the
   * count is the duration of the AC input high/low time) and causes a
   * TIMER reload-start.
   *
   * However, initializing CC0 must be the last step in the TIMER setup
   * process because the next zero-crossing, whenever it happens,
   * starts the counter.  That first zero-crossing occurs at CNT = 0
   * and must be ignored.  Furthermore, the CC channel controlling the
   * triac gate enable must also be setup so that the triac can be
   * immediately placed in the on or off state depending on the setting
   * of the initializer initOn member.
   */
  TIMER_InitCC_TypeDef cc0init = TIMER_INITCC_DEFAULT;
  cc0init.edge = timerEdgeRising;
  cc0init.mode = timerCCModeCapture;
  cc0init.prsInput = true;
  cc0init.prsSel = prsChannel;

  /*
   * CC1 output compare match is the rising edge of the gate enable
   * pulse.  The signal is initially low and goes high upon
   * match.  Because CC2, which determines the falling edge of the
   * pulse, is initially high, the two signals are ANDed by the PRS
   * logic to drive create the pulse.  Except for the debug code
   * below, CC1 is not actually driven to a pin.
   */
  TIMER_InitCC_TypeDef cc1init = TIMER_INITCC_DEFAULT;
  cc1init.mode = timerCCModeCompare;
  cc1init.cmoa = timerOutputActionSet;
  cc1init.prsOutput = timerPrsOutputLevel;

#ifdef TRIACDRV_DEBUG_PULSE_RISE_TIMER_OUT
  GPIO_PinModeSet(TRIACDRV_DEBUG_PULSE_RISE_TIMER_OUT_PORT, TRIACDRV_DEBUG_PULSE_RISE_TIMER_OUT_PIN, gpioModePushPull, 0);

  // Drive CC1 output to pin for observation
  tmr->ROUTELOC0 |= (TRIACDRV_DEBUG_PULSE_RISE_TIMER_OUT_LOC << _TIMER_ROUTELOC0_CC1LOC_SHIFT);
  tmr->ROUTEPEN |= TIMER_ROUTEPEN_CC1PEN;
#endif

  /*
   * This is the complementary part of the gate enable pulse code
   * above.  CC2 output compare match defines the pulse falling edge.
   * The signal is initially high and goes low upon match, which is
   * triacdrvPulseWidthTicks later.  ANDing CC1 with CC2 (via the PRS)
   * creates the gate enable pulse.
   */
  TIMER_InitCC_TypeDef cc2init = TIMER_INITCC_DEFAULT;
  cc2init.mode = timerCCModeCompare;
  cc2init.cmoa = timerOutputActionClear;
  cc2init.coist = true;
  cc2init.prsOutput = timerPrsOutputLevel;

#ifdef TRIACDRV_DEBUG_PULSE_FALL_TIMER_OUT
  GPIO_PinModeSet(TRIACDRV_DEBUG_PULSE_FALL_TIMER_OUT_PORT, TRIACDRV_DEBUG_PULSE_FALL_TIMER_OUT_PIN, gpioModePushPull, 1);

  // Drive CC2 output to pin for observation
  tmr->ROUTELOC0 |= (TRIACDRV_DEBUG_PULSE_FALL_TIMER_OUT_LOC << _TIMER_ROUTELOC0_CC2LOC_SHIFT);
  tmr->ROUTEPEN |= TIMER_ROUTEPEN_CC2PEN;
#endif

  /*
   * Setup the CCV registers for the initial on or off state.  If
   * the triac is to initially be on, then the compare match needs
   * to occur at CCV = 0, which is the default out of reset.
   *
   * If the triac should be off, then set the compare match at the
   * maximum timer count.  Because a zero-crossing at 50 or 60 Hz
   * will restart the counter before the maximum count value is
   * reached, a match will never occur, and the gate enable will
   * remain low.
   *
   * Set the global duty cycle and firing time variables to align
   * with the on/off determination.
   */
  if (!initOn)
  {
    triacdrvPulseRiseTime = _TIMER_CNT_MASK;
    triacdrvPulseFallTime = _TIMER_CNT_MASK;
    TIMER_CompareSet(tmr, 1, triacdrvPulseRiseTime);
    TIMER_CompareSet(tmr, 2, triacdrvPulseFallTime);
    triacdrvDutyPercent = 0;
  }
  else
  {
    triacdrvDutyPercent = 100;
    triacdrvPulseRiseTime = 0;
    triacdrvPulseFallTime = triacdrvPulseRiseTime + triacdrvPulseWidthTicks;
  }

  // Enable CC1 and CC2 for gate enable rising and falling edges
  TIMER_InitCC(tmr, 1, &cc1init);
  TIMER_InitCC(tmr, 2, &cc2init);

  /*
   * Initialize CC0 and wait for first capture, which occurs at
   * CNT = 0.  All subsequent CC0 captures will be the CNT value
   * where the zero-crossing rising or falling edge occurs.
   */
  TIMER_InitCC(tmr, 0, &cc0init);

  while ((TIMER_IntGet(tmr) & TIMER_IF_CC0) == 0);

  /*
   * First edge detected CNT = 0; read CCV to throw it away,
   * then clear the CC0 flag.
   */
  (void)tmr->CC[0].CCV;
  TIMER_IntClear(tmr, TIMER_IF_CC0);
}

/***************************************************************************//**
 * Private PRS initialization function called by triacdrv_init().
 ******************************************************************************/

sl_status_t triacdrv_initPRS(ACMP_TypeDef *acmp, uint32_t acmpChannel, TIMER_TypeDef *tmr, uint32_t tmrChannel)
{
  sl_status_t rc = SL_STATUS_OK;
  uint32_t acmpProducer, tmrProducer, tmrSignalCC1, tmrSignalCC2, tmpFreq;
  CMU_Clock_TypeDef acmpClock, timerClock;

  // Figure out which ACMP to use, but don't do anything right now.
  if (false)
    { }
#if defined(ACMP0)
  else if (acmp == ACMP0)
  {
    acmpProducer = PRS_CH_CTRL_SOURCESEL_ACMP0;
    acmpClock = cmuClock_ACMP0;
  }
#endif
#if defined(ACMP1)
  else if (acmp == ACMP1)
  {
    acmpProducer = PRS_CH_CTRL_SOURCESEL_ACMP1;
    acmpClock = cmuClock_ACMP1;
  }
#endif
#if defined(ACMP2)
  else if (acmp == ACMP2)
  {
    acmpProducer = PRS_CH_CTRL_SOURCESEL_ACMP2;
    acmpClock = cmuClock_ACMP2;
  }
#endif
#if defined(ACMP3)
  else if (acmp == ACMP3)
  {
    acmpProducer = PRS_CH_CTRL_SOURCESEL_ACMP3;
    acmpClock = cmuClock_ACMP1;
  }
#endif
#ifndef TRIACDRV_DISABLE_HW_RESOURCE_CHECKING
  else
    return SL_STATUS_INVALID_CONFIGURATION;
#endif /* TRIACDRV_DISABLE_HW_RESOURCE_CHECKING */

  // Figure out which TIMER to use, but don't do anything right now.
  if (false)
    { }
#if defined(TIMER0)
  else if (tmr == TIMER0)
  {
    tmrProducer = PRS_CH_CTRL_SOURCESEL_TIMER0;
    tmrSignalCC1 = PRS_TIMER0_CC1;
    tmrSignalCC2 = PRS_TIMER0_CC2;
    timerClock = cmuClock_TIMER0;
  }
#endif
#if defined(TIMER1)
  else if (tmr == TIMER1)
  {
    tmrProducer = PRS_CH_CTRL_SOURCESEL_TIMER1;
    tmrSignalCC1 = PRS_TIMER1_CC1;
    tmrSignalCC2 = PRS_TIMER1_CC2;
    timerClock = cmuClock_TIMER1;
  }
#endif
#if defined(TIMER2)
  else if (tmr == TIMER2)
  {
    tmrProducer = PRS_CH_CTRL_SOURCESEL_TIMER2;
    tmrSignalCC1 = PRS_TIMER2_CC1;
    tmrSignalCC2 = PRS_TIMER2_CC2;
    timerClock = cmuClock_TIMER2;
  }
#endif
#if defined(TIMER3)
  else if (tmr == TIMER3)
  {
    tmrProducer = PRS_CH_CTRL_SOURCESEL_TIMER3;
    tmrSignalCC1 = PRS_TIMER3_CC1;
    tmrSignalCC2 = PRS_TIMER3_CC2;
    timerClock = cmuClock_TIMER3;
  }
#endif
#if defined(TIMER4)
  else if (tmr == TIMER4)
  {
    tmrProducer = PRS_CH_CTRL_SOURCESEL_TIMER4;
    tmrSignalCC1 = PRS_TIMER4_CC1;
    tmrSignalCC2 = PRS_TIMER4_CC2;
    timerClock = cmuClock_TIMER4;
  }
#endif
#if defined(TIMER5)
  else if (tmr == TIMER5)
  {
    tmrProducer = PRS_CH_CTRL_SOURCESEL_TIMER5;
    tmrSignalCC1 = PRS_TIMER5_CC1;
    tmrSignalCC2 = PRS_TIMER5_CC2;
    timerClock = cmuClock_TIMER5;
  }
#endif
#if defined(TIMER6)
  else if (tmr == TIMER6)
  {
    tmrProducer = PRS_CH_CTRL_SOURCESEL_TIMER6;
    tmrSignalCC1 = PRS_TIMER6_CC1;
    tmrSignalCC2 = PRS_TIMER6_CC2;
    timerClock = cmuClock_TIMER6;
  }
#endif
#ifndef TRIACDRV_DISABLE_HW_RESOURCE_CHECKING
  else
    return SL_STATUS_INVALID_CONFIGURATION;
#endif /* TRIACDRV_DISABLE_HW_RESOURCE_CHECKING */

  /*
   * Make sure the selected TIMER clock is at least 1 MHz.  Assuming
   * it is, save this frequency and the timer for use elsewhere.
   */
  tmpFreq = CMU_ClockFreqGet(timerClock);
  if (tmpFreq >= TRIACDRV_MIN_TIMER_FREQUENCY)
  {
    triacdrvTimerFreq = tmpFreq;
    triacdrvTimer = tmr;
  }
  else
    return SL_STATUS_INVALID_RANGE;

  /*
   * ACMP and TIMER selections are valid.  Rnable their respective
   * clocks, as well as the GPIO and PRS clocks.
   */
  CMU_ClockEnable(cmuClock_PRS, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(acmpClock, true);
  CMU_ClockEnable(timerClock, true);

  // Make ACMP output available on PRS channel for TIMER
  PRS_SourceSignalSet(acmpChannel,
                      acmpProducer,
                      PRS_CH_CTRL_SIGSEL_ACMP0OUT,
                      prsEdgeBoth);

#ifdef TRIACDRV_DEBUG_PRSACMP
  GPIO_PinModeSet(TRIACDRV_DEBUG_PRSACMP_PORT, TRIACDRV_DEBUG_PRSACMP_PIN, gpioModePushPull, 0);

  // Route ACMP PRS output to debug pin
  PRS_GpioOutputLocation(acmpChannel, TRIACDRV_DEBUG_PRSACMP_LOC);
#endif

  // Assign TIMER CC1 to the selected PRS channel
  PRS_SourceSignalSet (tmrChannel, tmrProducer, tmrSignalCC1, prsEdgeOff);

#ifdef TRIACDRV_DEBUG_PULSE_RISE_PRS_OUT
  GPIO_PinModeSet(TRIACDRV_DEBUG_PULSE_RISE_PRS_OUT_PORT, TRIACDRV_DEBUG_PULSE_RISE_PRS_OUT_PIN, gpioModePushPull, 0);

  // Route CC1 PRS output to debug pin
  PRS_GpioOutputLocation(tmrChannel, TRIACDRV_DEBUG_PULSE_RISE_PRS_OUT_LOC);
#endif

  // Assign TIMER CC2 to the paired PRS channel
  PRS_SourceSignalSet (tmrChannel + 1, tmrProducer, tmrSignalCC2, prsEdgeOff);

#ifdef TRIACDRV_DEBUG_PULSE_FALL_PRS_OUT
  GPIO_PinModeSet(TRIACDRV_DEBUG_PULSE_FALL_PRS_OUT_PORT, TRIACDRV_DEBUG_PULSE_FALL_PRS_OUT_PIN, gpioModePushPull, 1);

  // Route CC2 PRS output to debug pin
  PRS_GpioOutputLocation(tmrChannel + 1, TRIACDRV_DEBUG_PULSE_FALL_PRS_OUT_LOC);
#endif

  // AND the PRS channel for CC1 with the channel for CC2
  PRS->CH[tmrChannel].CTRL |= PRS_CH_CTRL_ANDNEXT;

  return rc;
}

/***************************************************************************//**
 * Private ACMP initialization function called by triacdrv_init().
 ******************************************************************************/
void triacdrv_initACMP(ACMP_TypeDef *acmp,
                       ACMP_Channel_TypeDef acmpInput,
                       TRIACDRV_InputWave_Typedef waveType,
                       uint32_t avdd,
                       uint32_t threshold)
{
  uint32_t divvb;

  /*
   * Initialize the selected ACMP but do not enable.  Operate in high
   * accuracy mode with increased bias currents to properly detect
   * zero-crossings when the input is an offset sine wave.
   */
  ACMP_Init_TypeDef acmpinit = ACMP_INIT_DEFAULT;
  acmpinit.accuracy = acmpAccuracyHigh;
  acmpinit.fullBias = true;
  acmpinit.biasProg = 0x10;
  acmpinit.enable = false;

  ACMP_Init(acmp, &acmpinit);

  // Use the 2.5V source for the VB divider
  ACMP_VBConfig_TypeDef vb_config = ACMP_VBCONFIG_DEFAULT;
  vb_config.input = acmpVBInput2V5;

  // Is the input waveform a rectified sine wave?
  if (waveType == triacInputRectifiedSine)
  {
    /*
     * Calculate the divider needed to derive the requested zero-crossing
     * threshold voltage.
     *
     * The VBDIV divided voltage is calculated using:
     *
     * VBDIV = VB × ((DIVVB + 1) / 64)
     *
     * The divider, then, for VBDIV in mV is...
     *
     * DIVVB = ((VBDIV × 64) / (VB × 1000) - 1
     *
     * ...where VB = 2.55V (ACMP VB 2V5 input)
     */
    divvb = ((threshold << 6) / 2500) - 1;

    vb_config.div0 = divvb;
    vb_config.div1 = divvb;
  }
  /*
   * Calculate the threshold voltages for when the input is falling
   * (will be less than the AVDD midpoint so the comparator output goes
   * to 0) or rising (will be greater than the AVDD midpoint so the
   * comparator output goes to 1). The user-specified zeroThreshold
   * is here treated as a hysteresis value.
   */
  else
  {
    // Input is less than the AVDD midpoint voltage plus the threshold
    vb_config.div0 = ((((avdd / 2) + threshold) << 6) / 2500) - 1;

    // Input is more than the AVDD midpoint voltage minus the threshold
    vb_config.div1 = ((((avdd / 2) - threshold) << 6) / 2500) - 1;
  }
  ACMP_VBSetup(acmp, &vb_config);

#ifdef TRIACDRV_DEBUG_ACMPOUT
  GPIO_PinModeSet(TRIACDRV_DEBUG_ACMPOUT_PORT, TRIACDRV_DEBUG_ACMPOUT_PIN, gpioModePushPull, 0);

  // Drive ACMP output state to a pin for debug
  ACMP_GPIOSetup(acmp, TRIACDRV_DEBUG_ACMPOUT_LOC, true, false);
#endif

  // Compare the threshold voltage (NEG) against the input (POS)
  ACMP_ChannelSet(acmp, acmpInputVBDIV, acmpInput);

  // Everything is setup, so now enable
  ACMP_Enable(acmp);

  // Wait for warm-up
  while (!(acmp->STATUS & _ACMP_STATUS_ACMPACT_MASK)) ;
}

/***************************************************************************//**
 * Private GPIO checking functions called by triacdrv_init().
 *
 * Note that this function only determines if the PRS_CHn GPIO location
 * is out of range for the CHnLOC bit field.  Not all CHnLOC bit field
 * values are encoded, e.g. a given PRS_CHn might only encode locations
 * 0 and 2, and the mask value for that bit field would be 3.  The user
 * could specify a location of 1 or 3, and this function cannot determine
 * that they are invalid.
 ******************************************************************************/
#ifndef TRIACDRV_DISABLE_HW_RESOURCE_CHECKING
sl_status_t triacdrv_initCheckValues(uint32_t avdd,
                                     uint32_t threshold,
                                     uint32_t width,
                                     uint32_t acmp_ch,
                                     uint32_t timer_ch)
#else
sl_status_t triacdrv_initCheckValues(uint32_t avdd,
                                     uint32_t threshold,
                                     uint32_t width)
#endif /* TRIACDRV_DISABLE_HW_RESOURCE_CHECKING */
{
  sl_status_t rc = SL_STATUS_OK;

  /*
   * Before doing any hardware initialization, make sure that the
   * specified AVDD supply voltage, zero-crossing threshold voltage
   * and the gate enable pulse width are within the allowable ranges
   * for the driver.
   */
  if ((avdd > TRIACDRV_MAX_AVDD) ||
      (avdd < TRIACDRV_MIN_AVDD))
    return SL_STATUS_INVALID_RANGE;

  if ((threshold < TRIACDRV_MIN_ZERO_THRESHOLD) ||
      (threshold > TRIACDRV_MAX_ZERO_THRESHOLD))
    return SL_STATUS_INVALID_RANGE;

  if ((width == 0) || (width > TRIACDRV_MAX_ENABLE_PULSE_WIDTH))
    return SL_STATUS_INVALID_RANGE;

#ifndef TRIACDRV_DISABLE_HW_RESOURCE_CHECKING
  /*
   * Make sure the same PRS channels are not selected for the ACMP and
   * TIMER (actually two channels, N and N + 1).
   */
  if ((acmp_ch == timer_ch) || (acmp_ch == (timer_ch + 1)))
    return SL_STATUS_INVALID_PARAMETER;

  // Check if selected PRS channel for the ACMP is valid
  if (acmp_ch >= PRS_CHAN_COUNT)
    return SL_STATUS_INVALID_CONFIGURATION;

  /*
   * Check if selected TIMER base PRS channel is valid.  It must be
   * one less then PRS_CHAN_COUNT because channels N and N + 1 are
   * used.
   */
  if (timer_ch >= (PRS_CHAN_COUNT - 1))
    return SL_STATUS_INVALID_CONFIGURATION;
#endif /* TRIACDRV_DISABLE_HW_RESOURCE_CHECKING */

  return rc;
}

#ifndef TRIACDRV_DISABLE_HW_RESOURCE_CHECKING
sl_status_t triacdrv_initCheckPrsGPIO(uint32_t channel,
                                      GPIO_Port_TypeDef port,
                                      uint32_t pin,
                                      uint32_t location)
{
  sl_status_t rc = SL_STATUS_OK;
  uint32_t shift, mask, group;

  // If selected GPIO port and pin are not valid...
  if (!(GPIO_PORT_PIN_VALID(port, pin)))
    return SL_STATUS_INVALID_CONFIGURATION;

  // Find the PRS_ROUTELOCn shift offset for the selected PRS channel
  shift = (channel % 4) * 8;

  /*
   * All ROUTELOCn registers are in consecutive memory locations, so
   * they can be treated as an array starting at ROUTELOC0.  Find the
   * index into this array to determine which _PRS_ROUTELOCn_MASK
   * value to use.
   */
  group = channel / 4;

  switch (group)
  {
    case 0 :
      mask = _PRS_ROUTELOC0_MASK;
      break;
    case 1 :
      mask = _PRS_ROUTELOC1_MASK;
      break;
#ifdef _PRS_ROUTELOC2_MASK
    case 2 :
      mask = _PRS_ROUTELOC2_MASK;
      break;
#endif
#ifdef _PRS_ROUTELOC3_MASK
    case 3 :
      mask = _PRS_ROUTELOC3_MASK;
      break;
#endif
#ifdef _PRS_ROUTELOC4_MASK
    case 4 :
      mask = _PRS_ROUTELOC4_MASK;
      break;
#endif
#ifdef _PRS_ROUTELOC5_MASK
    case 5 :
      mask = _PRS_ROUTELOC5_MASK;
      break;
#endif
    // Should never get here as the PRS channel has already been validated
    default :
      return SL_STATUS_INVALID_CONFIGURATION;
      break;
  }

  // Shift the CHnLOC mask value to the lower 8 bits
  mask = ((mask >> shift) & 0x000000ff);

  // Check if the specified PRS_CHn location is valid
  if (location > mask)
    return SL_STATUS_INVALID_CONFIGURATION;

  return rc;
}
#endif /* TRIACDRV_DISABLE_HW_RESOURCE_CHECKING */

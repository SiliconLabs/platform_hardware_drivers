/***************************************************************************//**
 * @file audio_dac_simple.c
 * @brief Simple test program using the audio dac driver
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
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
#include "em_device.h"
#include "em_chip.h"
#include "audio_dac.h"
#include "test_A.h"
#include "em_gpio.h"



// Globally declared AUDIO_DAC_DATA
// Data cannot go out of scope while playback is active
// So the safest option is to declare it statically
AUDIO_DAC_DATA(testData, (int16_t *) test_A, TEST_A_LEN, resolution16);

int main(void)
{
  /* Chip errata */
  CHIP_Init();
  audio_dac_init_t init = AUDIO_DAC_INIT_DEFAULT;
  init.stereo = true;

  // initialize DAC using default parameters and no callbacks
  audio_dac_init(&init, false);

  /// Start test playback, repeat the sine wave
  /// Resulting audio should be a constant A note (440 Hz)
  audio_dac_start(&testData, true);

  /// Loop indefinitely
  while (1){};
}

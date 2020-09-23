/***************************************************************************//**
 * @file audio_dac_config.h
 * @brief application defined configuration for audio_dac.h driver. This
 * implementation is designed for the SLSTK3701A Starter Kit and the Adafruit
 * Stereo I2S Decoder Board.
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


#ifndef AUDIO_DAC_CONFIG_H_
#define AUDIO_DAC_CONFIG_H_

#include "sl_status.h"

// LDMA Channel used for audio output
#define AUDIO_DAC_CONFIG_LDMA_CHANNEL 0

// USART used for audio output, must support I2S
#define AUDIO_DAC_CONFIG_USART      USART1

// Clock enable for selected USART
#define AUDIO_DAC_CONFIG_USART_CLOCK  cmuClock_USART1

// GPIO initialization and routing for selected USART
// TX - PC1
// CS - PC0
// CLK - PB11
#define AUDIO_DAC_CONFIG_GPIO_INIT                              \
{                                                               \
  GPIO_PinModeSet(gpioPortC, 1, gpioModePushPull, 1);           \
  GPIO_PinModeSet(gpioPortC, 0, gpioModePushPull, 1);           \
  GPIO_PinModeSet(gpioPortB, 11, gpioModePushPull, 1);          \
}
#define AUDIO_DAC_CONFIG_USART_ROUTE  USART_ROUTELOC0_TXLOC_LOC4      \
                                      | USART_ROUTELOC0_CSLOC_LOC4    \
                                      | USART_ROUTELOC0_CLKLOC_LOC5   \

// DMA channel configuration used for all LDMA->I2S transmit operations
#define AUDIO_DAC_CONFIG_LDMA_TRANSFER_CFG                            \
  LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_USART1_TXBL);     \

#endif /* AUDIO_DAC_CONFIG_H_ */

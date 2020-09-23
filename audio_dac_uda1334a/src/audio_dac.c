/***************************************************************************//**
 * @file audio_dac.c
 * @brief core implementation of audio dac driver
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
#include <stdint.h>
#include <stddef.h>
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_usart.h"
#include "em_ldma.h"
#include "audio_dac.h"
#include "audio_dac_config.h"
#include "sl_status.h"

#define LDMA_MAX_TRANSFER_SIZE 2000

// Status flag for audio play back
// Stores whether play back is in progress or stopped
static bool playback_in_progress = false;

// Required LDMA transfer configuration
// This configuration is defined by the user customizable 
// audio_dac_config.h file.
static  LDMA_TransferCfg_t txCfg = AUDIO_DAC_CONFIG_LDMA_TRANSFER_CFG;

// Status variable for whether to use callback functions
// If this variable is true, assume valid function pointers are stored
// below
static bool callbacks_valid;

// Callback Function Pointers
// Initialized to null to provide extra error checking
// User callback for initialization
sl_status_t (*init_callback)(void) = NULL;
// User callback for volume control
sl_status_t (*volume_callback)(int volume) = NULL;
// User callback for mute control
sl_status_t (*mute_callback)(void) = NULL;

// Custom descriptor type for linked audio transfers.
// This descriptor transfers one 16 bit sample per I2S request,
// does not interrupt when the transfer completes, and will
// link when the transfer completes.
#define AUDIO_DAC_DESCRIPTOR_LINK(src, dest, count, width, linkjmp)   \
  {                                                                   \
    .xfer =                                                           \
    {                                                                 \
      .structType   = ldmaCtrlStructTypeXfer,                         \
      .structReq    = 0,                                              \
      .xferCnt      = (count) - 1,                                    \
      .byteSwap     = 0,                                              \
      .blockSize    = ldmaCtrlBlockSizeUnit1,                         \
      .doneIfs      = 0,                                              \
      .reqMode      = ldmaCtrlReqModeBlock,                           \
      .decLoopCnt   = 0,                                              \
      .ignoreSrec   = 0,                                              \
      .srcInc       = ldmaCtrlSrcIncOne,                              \
      .size         = width,                                          \
      .dstInc       = ldmaCtrlDstIncNone,                             \
      .srcAddrMode  = ldmaCtrlSrcAddrModeAbs,                         \
      .dstAddrMode  = ldmaCtrlDstAddrModeAbs,                         \
      .srcAddr      = (uint32_t)(src),                                \
      .dstAddr      = (uint32_t)(dest),                               \
      .linkMode     = ldmaLinkModeRel,                                \
      .link         = 1,                                              \
      .linkAddr     = (linkjmp) * 4                                   \
    }                                                                 \
  }

// Custom descriptor type for terminating (single) audio transfers.
// This descriptor transfers one 16 bit sample per I2S request,
// does not interrupt when the transfer completes, and will
// terminate when the transfer completes.
#define AUDIO_DAC_DESCRIPTOR_SINGLE(src, dest, width, count)          \
  {                                                                   \
    .xfer =                                                           \
    {                                                                 \
      .structType  = ldmaCtrlStructTypeXfer,                          \
      .structReq    = 0,                                              \
      .xferCnt      = (count) - 1,                                    \
      .byteSwap    = 0,                                               \
      .blockSize    = ldmaCtrlBlockSizeUnit1,                         \
      .doneIfs      = 0,                                              \
      .reqMode      = ldmaCtrlReqModeBlock,                           \
      .decLoopCnt  = 0,                                               \
      .ignoreSrec  = 0,                                               \
      .srcInc      = ldmaCtrlSrcIncOne,                               \
      .size        = width,                                           \
      .dstInc      = ldmaCtrlDstIncNone,                              \
      .srcAddrMode  = ldmaCtrlSrcAddrModeAbs,                         \
      .dstAddrMode  = ldmaCtrlDstAddrModeAbs,                         \
      .srcAddr      = (uint32_t)(src),                                \
      .dstAddr      = (uint32_t)(dest),                               \
      .linkMode    = 0,                                               \
      .link        = 0,                                               \
      .linkAddr    = 0                                                \
    }                                                                 \
  }



/**************************************************************************//**
 * Set up LDMA
 *****************************************************************************/
static sl_status_t init_ldma(void)
{
  CMU_ClockEnable(cmuClock_LDMA, true);
  // Default LDMA init
  LDMA_Init_t init = LDMA_INIT_DEFAULT;
  LDMA_Init(&init);

  return SL_STATUS_OK;
}

/**************************************************************************//**
 * I2S initialization function
 *****************************************************************************/
static sl_status_t init_i2s(audio_dac_init_t *init)
{
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Enable clock for USART
  CMU_ClockEnable(AUDIO_DAC_CONFIG_USART_CLOCK, true);

  // Enable GPIO clock and I2S pins
  AUDIO_DAC_CONFIG_GPIO_INIT;

  // Initialize USART to output I2S stereo data
  USART_InitI2s_TypeDef def = USART_INITI2S_DEFAULT;

  switch (init->res) {
    case resolution8:
      def.sync.databits = usartDatabits8;
      def.format = usartI2sFormatW8D8;
      break;
    case resolution16:
      def.sync.databits = usartDatabits16;
      def.format = usartI2sFormatW16D16;
      break;
    default:
      return SL_STATUS_FAIL;
  }

  def.sync.enable = usartDisable;
  def.sync.autoTx = true;
  def.justify = usartI2sJustifyRight;

  // define mono channel operation
  def.mono = !(init->stereo);
  def.dmaSplit = false;

  // Set baud rate to achieve desired sample frequency
  // Note to multiply this value by the word size to determine baudrate
  def.sync.baudrate = init->frequency * init->res;

  USART_InitI2s(AUDIO_DAC_CONFIG_USART, &def);

  // Enable route to GPIO pins for I2S transfer on EXP header
  AUDIO_DAC_CONFIG_USART->ROUTEPEN =  USART_ROUTEPEN_TXPEN
                                      | USART_ROUTEPEN_CSPEN
                                      | USART_ROUTEPEN_CLKPEN;

  AUDIO_DAC_CONFIG_USART->ROUTELOC0 = AUDIO_DAC_CONFIG_USART_ROUTE;

  // Enable USART
  USART_Enable(AUDIO_DAC_CONFIG_USART, usartEnableTx);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Initializes all peripherals needed for audio_dac transmit
 *
 * @param init    Pointer to the audio DAC initialization struct
 * @param callbacks Determines if user defined callback functions are enabled.
 *          Only set this to true if valid function pointers have been
 *          defined in the initialization struct for init_cb, volume_cb,
 *          and mute_cb
 ******************************************************************************/
sl_status_t audio_dac_init(audio_dac_init_t *init, bool callbacks)
{
  sl_status_t status = SL_STATUS_OK;
  status |= init_ldma();
  status |= init_i2s(init);

  // Initialize state and callbacks
  playback_in_progress = false;
  callbacks_valid = false;
  if (callbacks) {
  // Check that function pointers are not null
    if (init->init_cb && init->volume_cb && init->mute_cb) {
      init_callback = init->init_cb;
      volume_callback = init->volume_cb;
      mute_callback = init->mute_cb;

      callbacks_valid = true;
    } else {
      // Mismatch between callback parameter and function pointers
      return SL_STATUS_NULL_POINTER;
    }
  }

  if (callbacks_valid) {
    // Double check for null function pointer
    if (init_callback) {
      status |= init_callback();
    } else {
      return SL_STATUS_NULL_POINTER;
    }
  }
  return status;
}

/***************************************************************************//**
 * Start a transmit to audio_dac
 *
 * @param data    Pointer to the audio data struct
 * @param repeat  Determines if the audio play back should loop continuously
 * 
 * @note  The audio data struct must not go out of scope while a transfer is 
*         in progress. Ensure that the data struct is statically allocated.
 ******************************************************************************/
sl_status_t audio_dac_start(audio_dac_data_t *data, bool repeat)
{
  // Get LDMA descriptor width based on resolution
  LDMA_CtrlSize_t transferWidth;
  switch (data->res) {
  case resolution8:
    transferWidth = ldmaCtrlSizeByte;
    break;
  case resolution16:
    transferWidth = ldmaCtrlSizeHalf;
    break;
  default:
    return SL_STATUS_FAIL;
  }

  // Loop through each descriptor and set up the descriptor to transfer samples
  // Loop count is equal to the count / LDMA_MAX_TRANSFER_SIZE,
  // the maximum samples per descriptor
  uint32_t remainingCount = data->count;
  uint32_t currentSample = 0;
  uint32_t descriptorCount = data->count / LDMA_MAX_TRANSFER_SIZE;
  for(uint8_t i = 0; i < descriptorCount; i++) {
    if (remainingCount > LDMA_MAX_TRANSFER_SIZE) {
    data->descriptors[i] = (LDMA_Descriptor_t) AUDIO_DAC_DESCRIPTOR_LINK(
        &(data->samples[currentSample]),
        &(AUDIO_DAC_CONFIG_USART->TXDOUBLE),
        LDMA_MAX_TRANSFER_SIZE,
        transferWidth,
        1);
      remainingCount = remainingCount - LDMA_MAX_TRANSFER_SIZE;
      currentSample += LDMA_MAX_TRANSFER_SIZE;
    } else {
      // Fail if still in the loop while remainingCount < LDMA_MAX_TRANSFER_SIZE
      return SL_STATUS_FAIL;
    }

  }
  // Set the final descriptor to loop if repeat is set
  if (repeat) {
    data->descriptors[descriptorCount] =
        (LDMA_Descriptor_t) AUDIO_DAC_DESCRIPTOR_LINK(
            &(data->samples[currentSample]),
            &(AUDIO_DAC_CONFIG_USART->TXDOUBLE),
            remainingCount,
            transferWidth,
            -(descriptorCount));
  } else {
    data->descriptors[descriptorCount] =
        (LDMA_Descriptor_t) AUDIO_DAC_DESCRIPTOR_SINGLE(
            &(data->samples[currentSample]),
            &(AUDIO_DAC_CONFIG_USART->TXDOUBLE),
            remainingCount,
            transferWidth);
  }

  LDMA_StartTransfer(
      AUDIO_DAC_CONFIG_LDMA_CHANNEL,
      &txCfg,
      (const LDMA_Descriptor_t *) data->descriptors);

  // Mask LDMA chdone interrupt as this has no handler
  LDMA->IFC = 1UL << (uint8_t)AUDIO_DAC_CONFIG_LDMA_CHANNEL;
  LDMA->IEN &= ~(1UL << (uint8_t)AUDIO_DAC_CONFIG_LDMA_CHANNEL);

  playback_in_progress = true;
  return SL_STATUS_OK;
}


/***************************************************************************//**
 * Mute audio DAC play back
 * 
 * @note  This method executes the user configurable mute function in
 *        audio_dac_config.h
 ******************************************************************************/
sl_status_t audio_dac_mute(void)
{
  if(audio_dac_status() && callbacks_valid) {
  // Double check for null function pointer
  if (volume_callback) {
    return mute_callback();
  } else {
    return SL_STATUS_NULL_POINTER;
  }
  } else {
  return SL_STATUS_FAIL;
  }
}

/***************************************************************************//**
 * Change volume during audio DAC play back
 * 
 * @param vol  unsigned integer volume level. Accepts values 0 to 100
 *
 * @note  This method executes the user configurable volume function in
 *        audio_dac_config.h
 ******************************************************************************/
sl_status_t audio_dac_volume(int vol)
{
  if(audio_dac_status() && callbacks_valid) {
  // Double check for null function pointer
  if (volume_callback) {
    return volume_callback(vol);
  } else {
    return SL_STATUS_NULL_POINTER;
  }
  } else {
    return SL_STATUS_FAIL;
  }
}

/***************************************************************************//**
 * Stop audio DAC play back
 ******************************************************************************/
sl_status_t audio_dac_stop(void)
{
  if(audio_dac_status()) {
    LDMA_StopTransfer(AUDIO_DAC_CONFIG_LDMA_CHANNEL);
    playback_in_progress = false;
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FAIL;
  }
}

/***************************************************************************//**
 * Get the status of the audio DAC driver. Returns true if play back is in
 * progress.
 ******************************************************************************/
bool audio_dac_status(void)
{
  return playback_in_progress;
}

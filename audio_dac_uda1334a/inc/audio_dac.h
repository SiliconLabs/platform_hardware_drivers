/***************************************************************************//**
 * @file audio_dac.h
 * @brief public header for audio dac driver
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

#ifndef AUDIO_DAC_H
#define AUDIO_DAC_H

#include <stddef.h>
#include <stdint.h>
#include "em_ldma.h"
#include "em_usart.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup audio_dac Audio DAC Driver
 * @brief Audio DAC Driver
 * ## Overview
 *  The audio DAC driver plays signed 8 bit or 16 bit PCM audio trough an i2s
 *  DAC. The driver is designed for compatability with the UDA1334A i2s DAC,
 *  but can be ported to other compatible hardware. The driver can play arrays
 *  of PCM samples (such as wav files) for both stereo and mono formats, and
 *  can optionally loop continuously to play sine wave tables. The audio DAC
 *  driver uses DMA transfers to transmit audio samples without any processor
 *  intervention or overhead.
 *
 *  The audio DAC driver is designed to run on Silicon Labs series 1 & 2
 *  devices. This driver requires a single I2S enabled USART, and an LDMA
 *  channel; both of these are defined in audio_dac_config.h.

 * ## Initialization
 *  First, an audio_dac_data_t struct contains the data and metadata required
 *  for playback. This struct must be initialized and statically allocated via
 *  the AUDIO_DAC_DATA() macro. Ensure that the pointer to the PCM samples is 
 *  signed and the width matches the declared resolution
 *  (e.g. int16_t for resolution16) 
 *  
 *  Next, the audio DAC should be initialized by calling audio_dac_init() with
 *  an audio_dac_init_t struct. This struct can be initialized with default 
 *  parameters via the AUDIO_DAC_INIT_DEFAULT macro.

 * ## Audio Playback
 *  To play audio, call audio_dac_start() with the desired audio data struct.
 *  One started, use the audio_dac_mute() and audio_dac_volume() to control
 *  playback.
 *  
 *  audio_dac_stop() will stop playback and reset the driver, it is required to
 *  stop playback prior to restarting.
 *  
 *  audio_dac_status() will return whether audio playback is in progress.
 *  
 * ## Hardware Configuration
 *  The hardware specific configuration, including GPIO pins and peripheral
 *  usage are defined in the audio_dac_config.h file. This file is intended to
 *  be modified by the user in order port to different hardware.
 *  
 *  The initialization struct also allows the user to supply three callback
 *  pointers. These callbacks allow for easy integration with external
 *  control interfaces, like i2c, which are commonly used by audio devices.
 *
 *  init_cb is run after initialization and can be used to additionally
 *  configure the DAC as needed. For example, many audio codecs with i2c control
 *  interfaces require a sequence of i2c writes to initialize the codec.
 *
 *  volume_cb is run during playback through the audio_dac_volume function. It
 *  should be used to send control signals to the DAC that change the output
 *  volume.
 *
 *  mute_cb is run during playback through the audio_dac_mute_function. It
 *  should be used to send control signals to the DAC that mutes or unmutes
 *  audio output.
 *
 *  Stub implementations for all of these callbacks are located in the
 *  audio_dac_config.h file for the UDA1334A. The UDA1334A only has available
 *  control signals to implement mute functionality via a GPIO pin.
 *
 * ## Energy Modes
 *  Audio playback is supported down to EM1. Audio playback should be stopped
 *  prior to EM2/EM3 entry.
 *  
 * ## Usage
 *  See test/audio_dac_simple.c for a simple example usage of this driver.
 *  Project templates for the example are included in the SimplicityStudio/ and
 *  IAR/ directories. These projects are designed to run on a EFM32GG11 Starter
 *  Kit.
 * 
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Data types

// Audio resolution
// driver supports 8-bit and 16-bit audio
typedef enum {
  resolution8 = 8,      ///< 8 bit audio samples
  resolution16 = 16,    ///< 16 bit audio samples
} resolution_t;

// Audio DAC initialization data
typedef struct {
  uint32_t frequency;             ///< Sample frequency
  bool stereo;                    ///< Stereo or mono audio data
  resolution_t res;               ///< Audio sample resolution
  sl_status_t (*init_cb)(void);       ///< User callback for initialization
  sl_status_t (*volume_cb)(int volume);   ///< User callback for volume control
  sl_status_t (*mute_cb)(void);       ///< User callback for mute control
} audio_dac_init_t;

// Default initialization parameters for audio DAC
// Uses callback function pointers defined in audio_dac_config.h
#define AUDIO_DAC_INIT_DEFAULT                        \
{                                                     \
  44100,          /* 44.1kHz sample frequency */      \
  true,           /* stereo mode */                   \
  resolution16,   /* 16 bit audio */                  \
  NULL,           /* No defined init callback */      \
  NULL,           /* No defined volume callback */    \
  NULL,           /* No defined mute callback */      \
}

// Container for all audio data required to play a sound sample
// Since the maximum single transfer length is 2048 bytes, we must use N
// descriptors where N = int_ceiling(Array Length / 2048)
typedef struct {
  void *samples;                    ///< Pointer to audio samples
  uint32_t count;                   ///< number of audio samples
  resolution_t res;                 ///< Width of audio samples
  LDMA_Descriptor_t *descriptors;   ///< Pointer to LDMA descriptor list
} audio_dac_data_t;


/***************************************************************************//**
 * Helper macro for intializing audio data and metadata required for playback.
 * 
 * Audio data and metadata must not go out of scope while playback is active
 * and should not be allocated locally.It is recommended to define this macro
 * globally in a source file in order to ensure data is statically allocated.
 * 
 * @param name        Name of the audio_dac_data_t struct that the macro will
  *                   create
 * @param samples     Pointer to audio samples
 * @param count       Number of audio samples
 * @param resolution  Audio resolution, supported formats are 8-bit & 16-bit
 ******************************************************************************/
#define AUDIO_DAC_DATA(name, samples, count, resolution)    \
  /* Define descriptor list metadata */                     \
  /* Named after audio data name */                         \
  LDMA_Descriptor_t name##_desc[count / 2048 + 1];          \
  /* Create audio data struct with  descriptor list  */     \
  audio_dac_data_t name = {                                 \
    samples,                                                \
    count,                                                  \
    resolution,                                             \
    name##_desc,                                            \
  }

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************//**
 * Initializes all peripherals needed for audio_dac transmit
 *
 * @param init  Pointer to the audio dac initialization struct
 * @param callbacks Determines if user defined callback functions are enabled.
 *          Only set this to true if valid function pointers have been
 *          defined in the initialization struct for init_cb, volume_cb,
 *          and mute_cb
 ******************************************************************************/
sl_status_t audio_dac_init(audio_dac_init_t *init, bool callbacks);

/***************************************************************************//**
 * Start a transmit to audio_dac
 *
 * @param data    Pointer to the audio data struct
 * @param repeat  Determines if the audio playback should loop continuously
 * 
 * @note  The audio data struct must not go out of scope while a transfer is 
*         in progress. Ensure that the data struct is statically allocated.
 ******************************************************************************/
sl_status_t audio_dac_start(audio_dac_data_t *data, bool repeat);

/***************************************************************************//**
 * Mute audio dac playback
 * 
 * @note  This method executes the user configurable mute function in
 *        audio_dac_config.h
 ******************************************************************************/
sl_status_t audio_dac_mute(void);

/***************************************************************************//**
 * Change volume during audio dac playback
 * 
 * @param vol  integer volume value to be passed to user volume callback
 * @note  This method executes the user configurable volume function in
 *        audio_dac_config.h
 ******************************************************************************/
sl_status_t audio_dac_volume(int vol);

/***************************************************************************//**
 * Stop audio dac playback
 ******************************************************************************/
sl_status_t audio_dac_stop(void);

/***************************************************************************//**
 * Get the status of the audio dac driver. Returns true if playback is in
 * progress.
 ******************************************************************************/
bool audio_dac_status(void);

/** @} (end addtogroup audio_dac) */

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AUDIO_DAC_H


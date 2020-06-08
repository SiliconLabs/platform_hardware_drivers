#ifndef TFA9896_H
#define TFA9896_H
// Resources used:
// USART1
// I2C0
// GPIO PORT C pins 6, 8, 9, 10, & 11
// GPIO PORT C configured for high drive strength
// LDMA Channel 0
// LDMA Configured for fixed priority



#include "em_ldma.h"
#include "wav_files.h"
#include "em_usart.h"
#include "em_i2c.h"

/**************************************************************************//**
 * TFA9896 Driver defines
 *****************************************************************************/
#define TFA9896_LDMA_CH 0 // Driver uses LDMA channel 0 (highest priority by default)
#define TFA9896_SAMPLE_FREQUENCY 22050

///Function Declarations

/**************************************************************************//**
 * @brief Initializes all peripherals needed for TFA9896 transmit
 *****************************************************************************/
void TFA9896_init(void);

/**************************************************************************//**
 * @brief  sends I2C control command to TFA9896 and blocks until transfer completes
 *****************************************************************************/
I2C_TransferReturn_TypeDef TFA9896_controlBlocking(uint16_t addr, uint8_t* txBuffer,uint8_t txLen);

/**************************************************************************//**
 * @brief sends I2C control command to TFA9896 and returns immediately
 *****************************************************************************/
void TFA9896_controlNonBlocking(uint16_t addr, uint8_t* txBuffer,uint8_t txLen);

/**************************************************************************//**
 * @brief Starts a transmit to TFA9896
 *****************************************************************************/
void TFA9896_start(LDMA_Descriptor_t  *descriptor);

/**************************************************************************//**
 * @brief Stops active LDMA transfer
 *****************************************************************************/
void TFA9896_stop(void);

/**************************************************************************//**
 * Custom descriptor types for audio file transfers
 *****************************************************************************/
/*
 * CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF
 * This descriptor transfers one 16 bit sample per I2S req
 * and does not interrupt when the transfer completes, and will link
 * when the tranfser completes
 */
#define CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(src, dest, count, linkjmp) \
  {                                                                 \
    .xfer =                                                         \
    {                                                               \
      .structType   = ldmaCtrlStructTypeXfer,                       \
      .structReq    = 0,                                            \
      .xferCnt      = (count) - 1,                                  \
      .byteSwap     = 0,                                            \
      .blockSize    = ldmaCtrlBlockSizeUnit1,                       \
      .doneIfs      = 0,                                            \
      .reqMode      = ldmaCtrlReqModeBlock,                         \
      .decLoopCnt   = 0,                                            \
      .ignoreSrec   = 0,                                            \
      .srcInc       = ldmaCtrlSrcIncOne,                            \
      .size         = ldmaCtrlSizeHalf,                             \
      .dstInc       = ldmaCtrlDstIncNone,                           \
      .srcAddrMode  = ldmaCtrlSrcAddrModeAbs,                       \
      .dstAddrMode  = ldmaCtrlDstAddrModeAbs,                       \
      .srcAddr      = (uint32_t)(src),                              \
      .dstAddr      = (uint32_t)(dest),                             \
      .linkMode     = ldmaLinkModeRel,                              \
      .link         = 1,                                            \
      .linkAddr     = (linkjmp) * 4                                 \
    }                                                               \
  }

/*
 * CUSTOM_DESCRIPTOR_SINGLE_M2P_HALF
 * This descriptor transfers one 16 bit sample per I2S req
 * and does not interrupt when the transfer completes, and will
 * end the LDMA channel when the transfer completes
 */
#define CUSTOM_DESCRIPTOR_SINGLE_M2P_HALF(src, dest, count) \
  {                                                                 \
    .xfer =                                                         \
    {                                                               \
      .structType   = ldmaCtrlStructTypeXfer,                       \
      .structReq    = 0,                                            \
      .xferCnt      = (count) - 1,                                  \
      .byteSwap     = 0,                                            \
      .blockSize    = ldmaCtrlBlockSizeUnit1,                       \
      .doneIfs      = 0,                                            \
      .reqMode      = ldmaCtrlReqModeBlock,                         \
      .decLoopCnt   = 0,                                            \
      .ignoreSrec   = 0,                                            \
      .srcInc       = ldmaCtrlSrcIncOne,                            \
      .size         = ldmaCtrlSizeHalf,                             \
      .dstInc       = ldmaCtrlDstIncNone,                           \
      .srcAddrMode  = ldmaCtrlSrcAddrModeAbs,                       \
      .dstAddrMode  = ldmaCtrlDstAddrModeAbs,                       \
      .srcAddr      = (uint32_t)(src),                              \
      .dstAddr      = (uint32_t)(dest),                             \
      .linkMode     = 0,                                  	    \
      .link         = 0,                                  	    \
      .linkAddr     = 0                                  	    \
    }                                                               \
  }


/**************************************************************************//**
 * Descriptor lists for each audio file
 *
 * Note that since the maximum single transfer length is 2048 bytes, we must link N transfers where
 * N = int_ceiling(Array Length / 2048)
 *
 * If the Audio transfer is meant to play once, the final descriptor should be a *_SINGLE() type that
 * Terminates the LDMA transfer. If the audio file is meant to be looped until stopped, the final descriptor
 * should be a *_LINKED() type that requires an LDMA_StopTransfer() call to stop.
 *****************************************************************************/

/*
 * Descriptor list for WAV_Clear
 * length is 13922
*/
static LDMA_Descriptor_t WAV_Clear_Desc[] = {
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Clear[0], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Clear[2048], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Clear[4096], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Clear[6144], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Clear[8192], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Clear[10240], &USART1->TXDATA, 2048, 1),
	CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Clear[12280], &USART1->TXDATA, 2048, 1),
	CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Clear[14328], &USART1->TXDATA, 2048, 1),
	CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Clear[16376], &USART1->TXDATA, 2048, 1),
	CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Clear[18424], &USART1->TXDATA, 2048, 1),
	CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Clear[20472], &USART1->TXDATA, 2048, -10),
    CUSTOM_DESCRIPTOR_SINGLE_M2P_HALF(&WAV_Clear[22520], &USART1->TXDATA, 1104), // Final transfer does not link
};

/*
 * Descriptor list for WAV_Intrusion
 * length is 39862
*/
static LDMA_Descriptor_t WAV_Intrusion_Desc[] = {
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[0], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[2048], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[4096], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[6144], &USART1->TXDATA, 2048, 1),

    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[8192], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[10240], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[12288], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[14336], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[16384], &USART1->TXDATA, 2048, 1),

    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[18432], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[20480], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[22528], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[24576], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[26624], &USART1->TXDATA, 2048, 1),

    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[28672], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[30720], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[32768], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[34816], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[36864], &USART1->TXDATA, 2048, 1),
    CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Intrusion[38912], &USART1->TXDATA, 950, -19) // Final transfer loops back to top
};

static LDMA_Descriptor_t WAV_A_Desc[] = {
	CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_A[0], &USART1->TXDOUBLE, WAV_A_LENGTH, 0),
};

static LDMA_Descriptor_t WAV_Cs_Desc[] = {
	CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_Cs[0], &USART1->TXDOUBLE, WAV_Cs_LENGTH, 0),
};

static LDMA_Descriptor_t WAV_E_Desc[] = {
	CUSTOM_DESCRIPTOR_LINKEREL_M2P_HALF(&WAV_E[0], &USART1->TXDOUBLE, WAV_E_LENGTH, 0),
};


#endif //TFA9896_H

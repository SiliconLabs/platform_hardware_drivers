#include <stdio.h>
#include <stdint.h>
#include <TFA9896.h>

#include "em_gpio.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_usart.h"
#include "em_ldma.h"
#include "em_i2c.h"

// Globally stored LDMA channel config used for all LDMA->I2S transmit operations
static  LDMA_TransferCfg_t txCfg = LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_USART1_TXBL);

/**************************************************************************//**
 * @brief sets up LDMA
 *****************************************************************************/
static void initLdma(void)
{
  CMU_ClockEnable(cmuClock_LDMA, true);
  // Default LDMA init
  LDMA_Init_t init = LDMA_INIT_DEFAULT;
  LDMA_Init(&init);
}

/**************************************************************************//**
 * @brief I2S initialization function, sampeFrequency in Hz
 *****************************************************************************/
static void initI2s(uint32_t sampleFrequency)
{
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Enable clock for USART1
  CMU_ClockEnable(cmuClock_USART1, true);

  // Enable GPIO clock and I2S pins
  GPIO_PinModeSet(gpioPortC, 6, gpioModePushPull, 1);
  GPIO_PinModeSet(gpioPortC, 8, gpioModePushPull, 1);
  GPIO_PinModeSet(gpioPortC, 9, gpioModePushPull, 1);

  // Initialize USART1 to receive data from microphones synchronously
  USART_InitI2s_TypeDef def = USART_INITI2S_DEFAULT;
  def.sync.databits = usartDatabits16;
  def.format = usartI2sFormatW16D16;
  def.sync.enable = usartDisable;
  def.sync.autoTx = true;
  def.justify = usartI2sJustifyLeft;

  // define mono channel operation
  def.mono = true;
  def.dmaSplit = false;

  // Set baud rate to achieve desired sample frequency
  // Note to multiple this value by the word size to determine baudrate
  def.sync.baudrate = sampleFrequency * 16;

  USART_InitI2s(USART1, &def);

  // Enable route to GPIO pins for I2S transfer on route #5
  USART1->ROUTEPEN =  USART_ROUTEPEN_TXPEN
                      | USART_ROUTEPEN_CSPEN
                      | USART_ROUTEPEN_CLKPEN;

  USART1->ROUTELOC0 = USART_ROUTELOC0_TXLOC_LOC11
                      | USART_ROUTELOC0_CSLOC_LOC11
                      | USART_ROUTELOC0_CLKLOC_LOC11;

  // Enable USART1
  USART_Enable(USART1, usartEnableTx);
}

/**************************************************************************//**
 * @brief  Setup I2C
 *****************************************************************************/
static void initI2c(void)
{
  CMU_ClockEnable(cmuClock_I2C0, true);
  // Using default settings
  I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;
  i2cInit.master = true;
  // Use ~400khz SCK
  i2cInit.freq = I2C_FREQ_FAST_MAX;

  // Using PC11(SDA) and PC10(SCL)
  GPIO_PinModeSet(gpioPortC, 11, gpioModeWiredAndPullUpFilter, 1);
  GPIO_PinModeSet(gpioPortC, 10, gpioModeWiredAndPullUpFilter, 1);
  GPIO_DriveStrengthSet(gpioPortC, gpioDriveStrengthStrongAlternateStrong);

  I2C0->ROUTEPEN = I2C_ROUTEPEN_SDAPEN | I2C_ROUTEPEN_SCLPEN;
  I2C0->ROUTELOC0 = (I2C0->ROUTELOC0 & (~_I2C_ROUTELOC0_SDALOC_MASK)) | I2C_ROUTELOC0_SDALOC_LOC16;
  I2C0->ROUTELOC0 = (I2C0->ROUTELOC0 & (~_I2C_ROUTELOC0_SCLLOC_MASK)) | I2C_ROUTELOC0_SCLLOC_LOC14;

  // Initializing the I2C
  I2C_Init(I2C0, &i2cInit);
}

/**************************************************************************//**
 * @brief Initializes all peripherals needed for TFA9896 transmit
 *****************************************************************************/
void TFA9896_init(void)
{
  initLdma();
  initI2s(TFA9896_SAMPLE_FREQUENCY);
  initI2c();
}

/**************************************************************************//**
 * @brief  sends I2C control command to TFA9896 and blocks until transfer completes
 *****************************************************************************/
I2C_TransferReturn_TypeDef TFA9896_controlBlocking(uint16_t addr, uint8_t* txBuffer,uint8_t txLen)
{
  I2C_TransferReturn_TypeDef result;
  // Transfer structure
  I2C_TransferSeq_TypeDef i2cTransfer;

  // Initializing I2C transfer
  i2cTransfer.addr          = addr;
  i2cTransfer.flags         = I2C_FLAG_WRITE;
  i2cTransfer.buf[0].data   = txBuffer;
  i2cTransfer.buf[0].len    = txLen;
  I2C_TransferInit(I2C0, &i2cTransfer);

  // Sending data
  while (result == i2cTransferInProgress)
  {
    result = I2C_Transfer(I2C0);
  }
  return result;
}

/**************************************************************************//**
 * @brief sends I2C control command to TFA9896 and returns immediately
 *****************************************************************************/
void TFA9896_controlNonBlocking(uint16_t addr, uint8_t* txBuffer,uint8_t txLen)
{
  // Transfer structure
  I2C_TransferSeq_TypeDef i2cTransfer;

  // Initializing I2C transfer
  i2cTransfer.addr          = addr;
  i2cTransfer.flags         = I2C_FLAG_WRITE;
  i2cTransfer.buf[0].data   = txBuffer;
  i2cTransfer.buf[0].len    = txLen;
  I2C_TransferInit(I2C0, &i2cTransfer);

  I2C_Transfer(I2C0);
}

/**************************************************************************//**
 * @brief Starts a transmit to TFA9896
 *****************************************************************************/
void TFA9896_start(LDMA_Descriptor_t  *descriptor)
{
  LDMA_StartTransfer(TFA9896_LDMA_CH, &txCfg, (const LDMA_Descriptor_t *) descriptor);
}

/**************************************************************************//**
 * @brief Stops active LDMA transfer
 *****************************************************************************/
void TFA9896_stop(void)
{
  LDMA_StopTransfer(TFA9896_LDMA_CH);
}

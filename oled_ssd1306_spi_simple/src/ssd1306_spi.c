/***************************************************************************//**
 * @file
 * @brief SPI abstraction used by SSD1306
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "em_cmu.h"
#include "em_gpio.h"
#include "spidrv.h"
#include "sl_spidrv_instances.h"
#include "ssd1306_spi.h"

#define spi_handle    sl_spidrv_mikroe_handle

/***************************************************************************//**
 * @brief
 *   Initialize gpio used in the SPI interface.
 *
 * @detail
 *  The driver instances will be initialized automatically,
 *  during the sl_system_init() call in main.c.
 *****************************************************************************/
void ssd1306_spi_init(void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Setup GPIOs */

  /* Configure RS pin as an output and drive inactive high */
  GPIO_PinModeSet(SSD1306_RS_GPIO_PORT, SSD1306_RS_GPIO_PIN, gpioModePushPull, 1);

  /* Configure DC pin as an output and drive inactive high */
  GPIO_PinModeSet(SSD1306_SPI_DC_PORT, SSD1306_SPI_DC_PIN, gpioModePushPull, 1);
}

/***************************************************************************//**
 * @brief
 *    Send blocking command over SPI interface.
 *
 * @note
 *    The data received on the MISO wire is discarded.
 *    @n This function is blocking and returns when the transfer is complete.
 *
 * @param[in] command
 *    Transmit command buffer.
 *
 * @param[in] len
 *    Number of bytes in transfer.
 *
 * @return
 *    @ref SL_STATUS_OK on success or @ref SL_STATUS_FAIL on failure
 ******************************************************************************/
sl_status_t ssd1306_send_command(const void *cmd, int len)
{
  Ecode_t ret;

  /* Clear DC pin to send command */
  GPIO_PinOutClear(SSD1306_SPI_DC_PORT, SSD1306_SPI_DC_PIN);

  /* Blocking data transfer to slave. */
  ret = SPIDRV_MTransmitB(spi_handle, cmd, len);
  if(ret != ECODE_EMDRV_SPIDRV_OK)
  {
    return SL_STATUS_FAIL;
  }

  /* Note that at this point all the data is loaded into the fifo, this does
   * not necessarily mean that the transfer is complete. */
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *    Send blocking data over SPI interface.
 *
 * @note
 *    The data received on the MISO wire is discarded.
 *    @n This function is blocking and returns when the transfer is complete.
 *
 * @param[in] data
 *    Transmit data buffer.
 *
 * @param[in] len
 *    Number of bytes in transfer.
 *
 * @return
 *    @ref SL_STATUS_OK on success or @ref SL_STATUS_FAIL on failure
 ******************************************************************************/
sl_status_t ssd1306_send_data(const void *data, int len)
{
  Ecode_t ret;

  /* Set DC pin to send data */
  GPIO_PinOutSet(SSD1306_SPI_DC_PORT, SSD1306_SPI_DC_PIN);

  /* Blocking data transfer to slave. */
  ret = SPIDRV_MTransmitB(spi_handle, data, len);
  if(ret != ECODE_EMDRV_SPIDRV_OK)
  {
    return SL_STATUS_FAIL;
  }

  /* Note that at this point all the data is loaded into the fifo, this does
   * not necessarily mean that the transfer is complete. */
  return SL_STATUS_OK;
}


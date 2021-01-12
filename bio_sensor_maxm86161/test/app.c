/***************************************************************************//**
 * @file app.c
 * @brief A simple example of how to use the maxm86161 driver
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
#include "sl_i2cspm.h"
#include "sl_udelay.h"
#include "em_cmu.h"
#include "em_emu.h"

#include "maxm86161.h"
#include "maxm86161_i2c.h"
#include "maxm86161_config.h"
#include <stdio.h>
#include <string.h>

static void gpio_setup(void);
static void init_mikroe_i2c(void);
static bool init_maxm86161(void);
static void maxm86161_run(void);
static void maxm86161_pause(void);
static int32_t maxm86161_hrm_identify_part(int16_t *part_id);


/**************************************************************************//**
 * Global Variables and Constants
 *****************************************************************************/
static maxm86161_device_config_t default_maxim_config = {
    3,//interrupt level
    {
#if (PROX_SELECTION & PROX_USE_IR)
        0x02,//LED2 - IR
        0x01,//LED1 - green
        0x03,//LED3 - RED
#elif (PROX_SELECTION & PROX_USE_RED)
        0x03,//LED3 - RED
        0x02,//LED2 - IR
        0x01,//LED1 - green
#else // default use GREEN
        0x01,//LED1 - green
        0x02,//LED2 - IR
        0x03,//LED3 - RED
#endif
        0x00,
        0x00,
        0x00,
    },
    {
        0x20,// green
        0x15,// IR
        0x10,// LED
    },
    {
        MAXM86161_PPG_CFG_ALC_DS,
        MAXM86161_PPG_CFG_OFFSET_NO,
        MAXM86161_PPG_CFG_TINT_117p3_US,
        MAXM86161_PPG_CFG_LED_RANGE_16k,
        MAXM86161_PPG_CFG_SMP_RATE_P1_24sps,
        MAXM86161_PPG_CFG_SMP_AVG_1
    },
    {
        MAXM86161_INT_ENABLE,//full_fifo
        MAXM86161_INT_DISABLE,//data_rdy
        MAXM86161_INT_DISABLE,//alc_ovf
#ifdef PROXIMITY
        MAXM86161_INT_ENABLE,//proximity
#else
        MAXM86161_INT_DISABLE,
#endif
        MAXM86161_INT_DISABLE,//led_compliant
        MAXM86161_INT_DISABLE,//die_temp
        MAXM86161_INT_DISABLE,//pwr_rdy
        MAXM86161_INT_DISABLE//sha
    }
};

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  init_mikroe_i2c();

  gpio_setup();

  init_maxm86161();
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

static void init_mikroe_i2c(void)
{
  I2CSPM_Init_TypeDef mikroe = MIKROE_I2C_INIT_DEFAULT;

  I2CSPM_Init(&mikroe);
}


/**************************************************************************//**
* @brief Setup GPIO, enable sensor isolation switch
*****************************************************************************/
static void gpio_setup(void)
{
  /* Enable GPIO clock */
  CMU_ClockEnable(cmuClock_GPIO, true);

  GPIO_PinModeSet(MAXM86161_BTN0_GPIO_PORT, MAXM86161_BTN0_GPIO_PIN, gpioModeInputPull, 1);  // PC7 is button
  GPIO_ExtIntConfig(MAXM86161_BTN0_GPIO_PORT, MAXM86161_BTN0_GPIO_PIN, MAXM86161_BTN0_GPIO_PIN, false, true, true);

  GPIO_PinModeSet(MAXM86161_EN_GPIO_PORT, MAXM86161_EN_GPIO_PIN, gpioModePushPull, 1);
  // need delay to wait the Maxim ready before we can read and write to register
  sl_udelay_wait(5000);
  /* Configure PB0 as input and enable interrupt  */
  GPIO_PinModeSet(MAXM86161_INT_GPIO_PORT, MAXM86161_INT_GPIO_PIN, gpioModeInput, 1);
  GPIO_IntConfig(MAXM86161_INT_GPIO_PORT, MAXM86161_INT_GPIO_PIN, true, true, true);
  if(GPIO_PinInGet(MAXM86161_INT_GPIO_PORT , MAXM86161_INT_GPIO_PIN) == 0)
    GPIO_IntSet(1 << MAXM86161_INT_GPIO_PIN);

  //GPIO_PinModeSet(gpioPortA, 4, gpioModePushPull, 1); // PA4 is LED

  /* Enable ODD interrupt to catch button press that changes slew rate */
  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

static bool init_maxm86161(void)
{
  int16_t part_id;
  maxm86161_device_config_t device_config = default_maxim_config;

  if (!maxm86161_hrm_identify_part(&part_id))
  {
    return 1;
  }

  maxm86161_init_device(device_config);
  maxm86161_shutdown_device(true);

  return 0;
}

/**************************************************************************//*
 * @brief
 *  Start the device's autonomous measurement operation.
 *  The device must be configured before calling this function.
 *
 * @return
 *  Returns error status.
 *****************************************************************************/
static void maxm86161_run(void)
{
  maxm86161_shutdown_device(false);
}

/**************************************************************************//**
 * @brief
 *  Pause the device's autonomous measurement operation.
 *  HRM must be running before calling this function.
 *
 * @return
 *  Returns error status.
 *****************************************************************************/
static void maxm86161_pause(void)
{
  maxm86161_shutdown_device(true);
}

/**************************************************************************//**
 * @brief Identify maxm86161 parts
 *****************************************************************************/
static int32_t maxm86161_hrm_identify_part(int16_t *part_id)
{
  int32_t valid_part = 0;

  *part_id = maxm86161_i2c_read_from_register(MAXM86161_REG_PART_ID);

  switch(*part_id)
  { // Static HRM/SpO2 supports all maxm86161 parts
    case 0x36:
      valid_part = 1;
      break;
    default:
      valid_part = 0;
      break;
  }
  return valid_part;
}

/**************************************************************************//**
 * @brief GPIO Interrupt handler for even pins.
 *****************************************************************************/

/*state of maxm86161 device*/
bool turn_on = false;

void GPIO_ODD_IRQHandler(void)
{
  uint32_t flags;
  uint8_t int_status;
  bool data_ready;
  maxm86161_ppg_sample_t sample;
  flags = GPIO_IntGet();
  GPIO_IntClear(flags);

  if(flags & (1 << MAXM86161_INT_GPIO_PIN))
  {
    int_status =  maxm86161_i2c_read_from_register(MAXM86161_REG_IRQ_STATUS1);
    if(int_status & MAXM86161_INT_1_FULL)
    {
      data_ready = maxm86161_read_samples_in_fifo(&sample);
      if(data_ready)
      {
        printf("\n%lu,%lu,%lu,", sample.ppg1, sample.ppg2, sample.ppg3);
      }
    }
  }

  if(flags & (1 << MAXM86161_BTN0_GPIO_PIN))
  {
    turn_on = !turn_on;
    if (turn_on)
    {
      maxm86161_run();
    }
    else
    {
      maxm86161_pause();
    }
  }
}

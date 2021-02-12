/***************************************************************************//**
 * @file   pn71x0_gpio.c
 * @brief  PN71x0 reset and interrupt GPIO interfaces.
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
 ******************************************************************************/

/* ...
 *
 * EVALUATION QUALITY
 * This code has been minimally tested to ensure that it builds with the specified dependency versions and is suitable as a demonstration for evaluation purposes only.
 * This code will be maintained at the sole discretion of Silicon Labs.
 *
... */

#include <stdint.h>
#include <stdbool.h>
#include "em_cmu.h"
#include "em_gpio.h"
#include "../inc/pn71x0_gpio.h"

static GPIO_Port_TypeDef   ven_port;
static uint8_t             ven_pin;

/**************************************************************************//**
 * @brief
 *  GPIO initialization for PN71x0 reset and interrupt.
 *
 * @param[in] gpio_init
 *  GPIO pin settings for PN71x0 rst and int pins.
 *****************************************************************************/
void pn71x0_gpio_init (pn71x0_gpio_init_t gpio_init) {
    /* Activate GPIO clock. */
    CMU_ClockEnable(cmuClock_GPIO, true);
    /* Set mode for IRQ pin */
    GPIO_PinModeSet(gpio_init.irq_port, gpio_init.irq_pin, gpioModeInput, 1);
    /* Enable raising-edge interrupts for IRQ pin. */
    GPIO_ExtIntConfig(gpio_init.irq_port, gpio_init.irq_pin, gpio_init.irq_pin, 1, 0, true);
    /* Enable GPIO Interrupt. */
    if ( (gpio_init.irq_pin % 2) == 1 ) {
      /* Enable IRQ for odd numbered GPIO pins. */
      NVIC_EnableIRQ(GPIO_ODD_IRQn);
    } else {
      /* Enable IRQ for even numbered GPIO pins. */
      NVIC_EnableIRQ(GPIO_EVEN_IRQn);
    }
    /* Set mode for VEN pin */
    GPIO_PinModeSet(gpio_init.ven_port, gpio_init.ven_pin, gpioModePushPull, 1);
    /* Store VEN ports and pins for reset use. */
    ven_port = gpio_init.ven_port;
    ven_pin  = gpio_init.ven_pin;
}

/**************************************************************************//**
 * @brief
 *  Reset PN71x0.
 *****************************************************************************/
void pn71x0_gpio_reset(void) {
    /* Apply VEN reset */
    GPIO_PinOutSet(ven_port, ven_pin);
    for (volatile int i = 0; i < 1000000; i++);
    GPIO_PinOutClear(ven_port, ven_pin);
    for (volatile int i = 0; i < 1000000; i++);
    GPIO_PinOutSet(ven_port, ven_pin);
}


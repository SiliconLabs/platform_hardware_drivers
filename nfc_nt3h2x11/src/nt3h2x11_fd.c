/***************************************************************************//**
 * @file   nt3h2x11_fd.c
 * @brief  Field detection interface for NT3H2x11.
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

#include "em_cmu.h"
#include "em_gpio.h"
#include "../inc/nt3h2x11_fd.h"

/**************************************************************************//**
 * @brief
 *  Initialize GPIO interrupt for field detection pin for NT3H2x11.
 *****************************************************************************/
void nt3h2x11_fd_init (nt3h2x11_fd_init_t fd_init) {
  /* Turn on GPIO clock. */
  CMU_ClockEnable(cmuClock_GPIO, true);
  /* Configure GPIO pins. */
  GPIO_PinModeSet(fd_init.fd_port, fd_init.fd_pin, gpioModeInputPullFilter, 1);
  /* Enable interrupts for FD pin. */
  GPIO_ExtIntConfig(fd_init.fd_port, fd_init.fd_pin, fd_init.fd_pin, 1, 1, true);
  /* Enable FD GPIO Interrupt. */
  if ( (fd_init.fd_pin % 2) == 1 ) {
    /* Enable IRQ for odd numbered GPIO pins. */
    NVIC_EnableIRQ(GPIO_ODD_IRQn);
  } else {
    /* Enable IRQ for even numbered GPIO pins. */
    NVIC_EnableIRQ(GPIO_EVEN_IRQn);
  }
}

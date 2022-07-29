/*
 * sl_vcnl4040_config.h
 *
 *  Created on: Jul 22, 2022
 *      Author: TungDT53
 */

#ifndef VCNL4040_CONFIG_H_
#define VCNL4040_CONFIG_H_

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2C module used; note that I2CSPM handles the transfers
#define SL_VCNL4040_CONFIG_I2C          I2C1

/*
 * GPIO port/pin used for INT interrupt pin. This should typically
 * be a port A/B pin so that it can detect a falling edge while in
 * EM2/3. If the application firmware does not need to use EM2/3,
 * then a port C/D pin can be used.
 */
#define SL_VCNL4040_CONFIG_INT_PORT     gpioPortB
#define SL_VCNL4040_CONFIG_INT_PIN      0

#ifdef __cplusplus
}
#endif

#endif /* SL_VCNL4040_CONFIG_H_ */

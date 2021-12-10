/*
 * Filename: max17048_config.h
 */
#ifndef MAX17048_CONFIG_H_
#define MAX17048_CONFIG_H_

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2C module used; note that I2CSPM handles the transfers
#define MAX17048_CONFIG_I2C      I2C0

/*
 * GPIO port/pin used for the ALRTn interrupt pin.  This should almost
 * always be a port A/B pin so that it can detect a falling edge while
 * in EM2/3.  If the application firmware does not need to use EM2/3,
 * then a port C/D pin could be used.
 */
#define MAX17048_CONFIG_ALRT_PORT   gpioPortC
#define MAX17048_CONFIG_ALRT_PIN    1

/*
 * In order for the MAX17048 to accurately track state of charge
 * (SOC), it must periodically compensate for battery temperature
 * changes.  The datasheet specifies that this be done no less than
 * once per minute.
 *
 * The driver calculates and updates the RCOMP factor at a rate of
 * 1000 ms <= MAX17048_CONFIG_RCOMP_UPDATE_INTERVAL_MS <= 60000 ms
 * and defaults to 1 minute (60000 ms = 1 minute).
 */
#define MAX17048_CONFIG_RCOMP_UPDATE_INTERVAL_MS    60000

/*
 * The MAX17048/9 has a manual input (QSTRT) to force a battery quick
 * start, which is the initial measurement of cell voltage (VCELL) and
 * state of charge (SOC).  In certain circumstances, quick start is
 * necessary if the bouncing associated initial power-up or battery
 * swap results in an erroneous initial reading of SOC.
 *
 * If enabled, quick start support is conditionally compiled into the
 * driver. The user must also designate a GPIO pin to connect to the
 * QSTRT pin on the MAX17048/9 to manually initiate quick start.
 *
 * The QSTRT pin can be any MCU GPIO pin, including a port C/D pin,
 * because it is an output that is toggled under software control.
 */
#define MAX17048_CONFIG_ENABLE_HW_QSTRT     0

#ifdef MAX17048_CONFIG_ENABLE_HW_QSTRT

#define MAX17048_CONFIG_ENABLE_QSTRT_PORT   gpioPortC
#define MAX17048_CONFIG_ENABLE_QSTRT_PIN    2

#endif /* MAX17048_CONFIG_ENABLE_HW_QSTRT */

#ifdef __cplusplus
}
#endif

#endif /* MAX17048_CONFIG_H_ */

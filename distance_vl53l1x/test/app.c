/***************************************************************************//**
 * @file
 * @brief VL53L1X Test application.
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "em_common.h"
#include "app.h"
#include "sl_sleeptimer.h"
#include "app_log.h"

#include "vl53l1x.h"
#include "vl53l1x_config.h"

// This define enables the periodic distance printing via UART
#define VL53_TEST_APP_PERIODIC_DISTANCE

// VL53 Timer callback function
void vl53_read_distance_periodic(sl_sleeptimer_timer_handle_t *handle,
		void *data);

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
void app_init(void) {
	static sl_sleeptimer_timer_handle_t handle_vl53;
	uint8_t sensor_state = 0;
	sl_status_t vl53_status = SL_STATUS_OK;

	/////////////////////////////////////////////////////////////////////////////
	// Put your additional application init code here!                         //
	// This is called once during start-up.                                    //
	/////////////////////////////////////////////////////////////////////////////

	app_log("==== VL53L1X Test application ====\n");
	app_log("app_init function called...\n");

	// Waiting for device to boot up...
	while (0 == sensor_state) {
		// Read sensor's state (0 = boot state, 1 = device is booted )
		vl53_status = vl53l1x_get_boot_state(VL53L1X_ADDR, &sensor_state);

		if (SL_STATUS_OK != vl53_status) {
			break;
		}

		// Wait for 2 ms
		sl_sleeptimer_delay_millisecond(2);
	}

	if (SL_STATUS_OK == vl53_status) {
		app_log("Platform I2C communication is OK.\n");
	} else {
		app_log("Platform I2C communication test has been failed.\n"
				"Please check the I2C bus connection and "
				"the I2C (I2CSPM) configuration.\n");
		return;
	}

	app_log("VL53L1X booted\n");

	// Initialize the sensor with the default settings
	vl53_status = vl53l1x_init(VL53L1X_ADDR);

	// Optional sensor configuration example function calls, see API documentation for options
	{
		vl53_status = vl53l1x_set_distance_mode(VL53L1X_ADDR,
		VL53L1X_DISTANCE_MODE_LONG); // Select distance mode

		vl53_status = vl53l1x_set_timing_budget_in_ms(VL53L1X_ADDR, 100); // in ms possible values [20, 50, 100, 200, 500]
		vl53_status = vl53l1x_set_inter_measurement_in_ms(VL53L1X_ADDR, 200); // in ms, IM must be > = TB

		vl53_status = vl53l1x_set_roi_xy(VL53L1X_ADDR, 16, 16); // min. ROI is 4,4
	}

	// Check return codes of the optional configuration function calls
	if (SL_STATUS_OK == vl53_status) {
		app_log("Sensor initialization and configuration are done.\n");
	} else {
		app_log("Sensor initialization and configuration has been failed.\n");
		return;
	}

	// Start ranging
	vl53_status = vl53l1x_start_ranging(VL53L1X_ADDR);

	// Check ranging status
	if (SL_STATUS_OK == vl53_status) {
		app_log("VL53L1X ranging has been started ...\n");

		// Get measurement result
		vl53_read_distance_periodic(NULL, NULL);
	} else {
		app_log("Start ranging has been failed.\n");
		return;
	}

#if defined(VL53_TEST_APP_PERIODIC_DISTANCE)
	// Setup a periodic sleep timer with 1000 ms time period
	sl_sleeptimer_start_periodic_timer_ms(&handle_vl53, 1000,
			vl53_read_distance_periodic, NULL, 0, 0);
	app_log("Periodic timer is configured to send periodic distance data.\n");
#endif

	app_log("==================================\n");

#if defined(VL53_TEST_APP_PERIODIC_DISTANCE)
	app_log("> Periodic measurement:\n");
#endif

}

void vl53_read_distance_periodic(sl_sleeptimer_timer_handle_t *handle,
		void *data) {
	sl_status_t vl53_status = SL_STATUS_OK;
	uint8_t is_data_ready = 0;
	vl53l1x_result_t result;
	(void) handle;
	(void) data;

	// Check measurement data status
	vl53_status = vl53l1x_check_for_data_ready(VL53L1X_ADDR, &is_data_ready);

	if (0 != is_data_ready) {
		// Measurement data is ready to read from the sensor
		vl53_status = vl53l1x_get_result(VL53L1X_ADDR, &result);

		// Print result
		app_log(" > Distance: %4u mm, Range status: %1u, "
				"Signal rate: %5u, Ambient rate: %5u, "
				"Number of SPADS: %4u\n", result.distance, result.status,
				result.signal_per_spad, result.ambient, result.signal_per_spad);

		// Clear sensor's interrupt status
		vl53_status = vl53l1x_clear_interrupt(VL53L1X_ADDR);
	}

	if (SL_STATUS_OK != vl53_status) {
		app_log("VL53L1X sensor operation has been failed "
				"during the periodic distancing.\n");
	}
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void) {

}

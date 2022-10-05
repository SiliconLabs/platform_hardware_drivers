/***************************************************************************//**
 * @file
 * @brief Top level application functions
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

#include "mlx90640/mlx90640.h"
#include "mlx90640/mlx90640_i2c.h"
#include "sl_i2cspm_instances.h"
#include "app_log.h"
#include "sl_sleeptimer.h"
#include "app_assert.h"

static float mlx90640_image[MLX90640_NUM_OF_PIXELS];
static sl_sleeptimer_timer_handle_t mlx90640_get_image_timer;
static bool print_image = false;

void print_mlx90640_temperature_array_cb()
{
  print_image = true;
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  sl_status_t sc;
  uint16_t refrate = 0;
  sc = mlx90640_init(sl_i2cspm_mlx90640, MLX90640_DEFAULT_I2C_ADDR);

  if(sc == SL_STATUS_OK)
  {
    app_log("\nMLX90640 initialized successfully\n");
    sc = mlx90640_SetRefreshRate(0x03);
    app_assert_status(sc);

    sc = mlx90640_GetRefreshRate(&refrate);
    app_assert_status(sc);

    app_log("RefreshRate: %x\n", refrate);

    sc = sl_sleeptimer_start_periodic_timer_ms(&mlx90640_get_image_timer,
                                      250,
                                      print_mlx90640_temperature_array_cb,
                                      (void*) NULL,
                                      0,
                                      0);
    app_assert_status(sc);
  }
  else
  {
    app_log("\nMLX90640 initialization failed!\n");
  }
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  if(print_image)
  {
    print_image = false;
    mlx90640_get_image_array(mlx90640_image);

    for(int i = 0; i < MLX90640_NUM_OF_PIXELS; i++)
    {
      app_log("%3.2f,", mlx90640_image[i]);
    }
    app_log("\n");
  }
}

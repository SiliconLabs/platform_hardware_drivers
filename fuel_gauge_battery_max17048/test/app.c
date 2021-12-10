/***************************************************************************//**
 * @file
 * @brief Core application logic.
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
 ********************************************************************************
 * # Experience Quality
 * This code has been minimally tested to ensure that it builds and is suitable
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/
#include "stdio.h"
#include "em_common.h"
#include "app.h"
#include "app_log.h"
#include "app_assert.h"
#include "gatt_db.h"
#include "sl_bluetooth.h"
#include "sl_i2cspm_instances.h"
#include "sl_sleeptimer.h"

#include "sl_board_control.h"
#include "dmd.h"
#include "glib.h"

#include "max17048.h"

typedef struct{
  uint8_t alert[5];
  uint32_t soc;
  float vcell;
  float crate;
  max17048_hibstate_t hibstate;
}battery_irq_t;

battery_irq_t battery_info;
// Configuration defines
#define X_BORDER 5
#define Y_BORDER 2

static GLIB_Context_t glibContext;
// Globals
static uint32_t xOffset, yOffset;
char display_string[20];
uint8_t id;
uint16_t version;
float vcell;
uint32_t soc;
uint8_t alert_condition;
uint8_t val;
const char* alert_string[5] = {"vhigh", "vlow", "reset", "empty", "soc"};
static sl_sleeptimer_timer_handle_t max17048_timer;

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;
static void soc_callback(sl_max17048_irq_source_t irq, battery_irq_t *data);
static void empty_callback(sl_max17048_irq_source_t irq, battery_irq_t *data);
static void vhigh_callback(sl_max17048_irq_source_t irq, battery_irq_t *data);
static void vlow_callback(sl_max17048_irq_source_t irq, battery_irq_t *data);
static void reset_callback(sl_max17048_irq_source_t irq, battery_irq_t *data);
void graphics_init(void);
static void refresh_display(void);
void graphics_clear(void);
static void max17048_timer_callback(sl_sleeptimer_timer_handle_t *handle,
                                    void *data);

/**************************************************************************//**
 * @brief This function draws the initial display screen
 *****************************************************************************/
void GRAPHICS_Update(void)
{
  DMD_updateDisplay();
}

void graphics_clear(void)
{
  GLIB_clear(&glibContext);

  // Reset the offset values to their defaults
  xOffset = X_BORDER;
  yOffset = Y_BORDER;
}

void GRAPHICS_AppendString(char *str)
{
  GLIB_drawString(&glibContext,
                  str,
                  strlen(str),
                  xOffset,
                  yOffset,
                  0);
  yOffset += glibContext.font.fontHeight + glibContext.font.lineSpacing;
}
/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  sl_status_t sc;

  // Initialize display
  graphics_init();
  // Initializes the max17048 - fuel gauge
  sc = max17048_init();
  app_log_status(sc);

  // read the ID
  sc = max17048_get_id(&id);
  app_log_status(sc);
  app_log("id: %d\r\n", id);

  // read the version of IC
  sc = max17048_get_production_version(&version);
  app_log_status(sc);
  app_log("version: %x\r\n", version);

  sc = max17048_get_vcell_vol(&battery_info.vcell);
  app_log_status(sc);
  app_log("vcell: %f V/cell\r\n", battery_info.vcell);

  // get SOC
  sc = max17048_get_soc(&battery_info.soc);
  app_log_status(sc);
  app_log("soc: %d%%\r\n", battery_info.soc);

  sc = max17048_get_crate(&battery_info.crate);
  app_log_status(sc);
  app_log("soc rate: %f C\r\n", battery_info.crate);

  sc = max17048_disable_auto_hibernate();
  app_log_status(sc);

  sc = max17048_enable_soc_interrupt(
    (max17048_interrupt_callback_t)soc_callback,
    &battery_info);
  app_log_status(sc);

  sc = max17048_enable_empty_interrupt(
    10,
    (max17048_interrupt_callback_t)empty_callback,
    &battery_info);
  app_log_status(sc);
  app_log("set soc empty: 10 %%\r\n");

  sc = max17048_enable_vhigh_interrupt(
    MAX17048_ALERT_VH_VAL,
    (max17048_interrupt_callback_t)vhigh_callback,
    &battery_info);
  app_log_status(sc);
  app_log("set vhigh alert: %.2f \r\n", MAX17048_ALERT_VH_VAL*0.02);

  sc = max17048_enable_vlow_interrupt(
    MAX17048_ALERT_VL_VAL,
    (max17048_interrupt_callback_t)vlow_callback,
    &battery_info);
  app_log_status(sc);
  app_log("set vlow alert: %.2f \r\n", MAX17048_ALERT_VL_VAL*0.02);

  sc = max17048_enable_reset_interrupt(
    MAX17048_VRESET_THRES,
    (max17048_interrupt_callback_t)reset_callback,
    &battery_info);
  app_log_status(sc);
  app_log("set vreset: %.2f vol\r\n", MAX17048_VRESET_THRES*0.04);

  max17048_unmask_interrupts();
  sc = max17048_clear_alert_condition();
  app_log_status(sc);

  sc = max1704x_clear_alert_status();
  app_log_status(sc);

  sc = sl_sleeptimer_start_timer(&max17048_timer,
                                 5000,
                                 max17048_timer_callback,
                                 (void *)NULL,
                                 0,
                                 0);
  app_log_status(sc);

  refresh_display();
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;
  uint8_t system_id[8];

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      app_log("Stack version: %u.%u.%u\r\r\n",
               evt->data.evt_system_boot.major,
               evt->data.evt_system_boot.minor,
               evt->data.evt_system_boot.patch);
      // Extract unique ID from BT Address.
      sc = sl_bt_system_get_identity_address(&address, &address_type);
      app_log_status(sc);
      //app_log("local BT device address: ");
      for (int i = 0; i < 5; i++)
      {
        app_log("%2.2x:", address.addr[5 - i]);
      }
      app_log("%2.2x\r\r\n", address.addr[0]);

      // Pad and reverse unique ID to get System ID.
      system_id[0] = address.addr[5];
      system_id[1] = address.addr[4];
      system_id[2] = address.addr[3];
      system_id[3] = 0xFF;
      system_id[4] = 0xFE;
      system_id[5] = address.addr[2];
      system_id[6] = address.addr[1];
      system_id[7] = address.addr[0];

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id,
                                                   0,
                                                   sizeof(system_id),
                                                   system_id);
      app_log_status(sc);

      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_log_status(sc);

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160, // min. adv. interval (milliseconds * 1.6)
        160, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
      app_log_status(sc);
      // Start general advertising and enable connections.
      sc = sl_bt_advertiser_start(advertising_set_handle,
        sl_bt_advertiser_general_discoverable,
        sl_bt_advertiser_connectable_scannable);
      app_log_status(sc);

//      sc = sl_bt_system_set_soft_timer(32768 *5 , 0, 0);
//      app_log_status(sc);
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      // Restart advertising after client has disconnected.
      sc = sl_bt_advertiser_start(
        advertising_set_handle,
        sl_bt_advertiser_general_discoverable,
        sl_bt_advertiser_connectable_scannable);
      app_log_status(sc);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

static void soc_callback(sl_max17048_irq_source_t irq, battery_irq_t *data)
{
  sl_status_t status;
  data->alert[irq] = 1;
  status = max17048_get_soc(&data->soc);
  app_log_status(status);
  app_log("interrupt soc: %d%%\r\n", data->soc);
  refresh_display();
}

static void empty_callback(sl_max17048_irq_source_t irq, battery_irq_t *data)
{
  sl_status_t status;
  data->alert[irq] = 1;
  status = max17048_get_soc(&data->soc);
  app_log_status(status);
  app_log("interrupt soc empty: %d%%\r\n", data->soc);
  refresh_display();
}

static void vhigh_callback(sl_max17048_irq_source_t irq, battery_irq_t *data)
{
  sl_status_t status;
  data->alert[irq] = 1;
  status = max17048_get_vcell_vol(&data->vcell);
  app_log_status(status);
  app_log("interrupt vhigh: %f vol\r\n", data->vcell);
  refresh_display();
}

static void vlow_callback(sl_max17048_irq_source_t irq, battery_irq_t *data)
{
  sl_status_t status;
  data->alert[irq] = 1;
  status = max17048_get_vcell_vol(&data->vcell);
  app_log_status(status);
  app_log("interrupt vlow: %f vol\r\n", data->vcell);
  refresh_display();
}

static void reset_callback(sl_max17048_irq_source_t irq, battery_irq_t *data)
{
  sl_status_t status;
  data->alert[irq] = 1;
  status = max17048_get_vcell_vol(&data->vcell);
  app_log_status(status);
  app_log("interrupt vreset: %f vol\r\n", data->vcell);
  refresh_display();
}

void graphics_init(void)
{
  EMSTATUS status;

  sl_board_enable_display();
  /* Initialize the DMD module for the DISPLAY device driver. */
  status = DMD_init(0);
  if (DMD_OK != status) {
    while (1) ;
  }

  status = GLIB_contextInit(&glibContext);
  if (GLIB_OK != status) {
    while (1) ;
  }

  glibContext.backgroundColor = White;
  glibContext.foregroundColor = Black;

  /* Use Normal font */
  GLIB_setFont(&glibContext, (GLIB_Font_t *)&GLIB_FontNormal8x8);
}

static void refresh_display(void)
{
  sl_status_t sc;
  // Effective size about 12 lines by 15 chars
  graphics_clear();

  sprintf(display_string, "ID %d", id);
  GRAPHICS_AppendString(display_string);

  sprintf(display_string, "Version %d", version);
  GRAPHICS_AppendString(display_string);

  sprintf(display_string, "SOC %ld%%", battery_info.soc);
  GRAPHICS_AppendString(display_string);

  sprintf(display_string, "VCELL %.2f V", battery_info.vcell);
  GRAPHICS_AppendString(display_string);

  sprintf(display_string, "CRATE %.2f C", battery_info.crate);
  GRAPHICS_AppendString(display_string);

  max17048_get_hibe_status(&battery_info.hibstate);
  sprintf(display_string, "Hibernating %d", battery_info.hibstate);
  GRAPHICS_AppendString(display_string);

  for(int i= 0; i< 5; i++) {
      if(battery_info.alert[i] == 0x01) {
          sprintf(display_string, "Alert %s", alert_string[i]);
          GRAPHICS_AppendString(display_string);
          battery_info.alert[i] = 0;
          if(i== IRQ_VCELL_HIGH) {
              app_log("max17048_disable_vhigh_interrupt\r\n");
              sc = max17048_disable_vhigh_interrupt();
              app_log_status(sc);
              sc = max17048_enable_vlow_interrupt(
                MAX17048_ALERT_VL_VAL,
                (max17048_interrupt_callback_t)vlow_callback,
                &battery_info);
              app_log_status(sc);
          } else if (i == IRQ_VCELL_LOW) {
              app_log("max17048_disable_vlow_interrupt\r\n");
              sc = max17048_disable_vlow_interrupt();
              app_log_status(sc);
              sc = max17048_enable_vhigh_interrupt(
                MAX17048_ALERT_VH_VAL,
                (max17048_interrupt_callback_t)vhigh_callback,
                &battery_info);
              app_log_status(sc);
          }
        }
  }

  DMD_updateDisplay();
}

static void max17048_timer_callback(sl_sleeptimer_timer_handle_t *handle,
                                    void *data) {
  (void)handle;
  (void)data;
  uint32_t tick;
  sl_status_t sc;

  // get VCELL
  sc = max17048_get_vcell_vol(&battery_info.vcell);
  app_log_status(sc);
  app_log("vcell: %f V/cell\r\n", battery_info.vcell);

  // get SOC
  sc = max17048_get_soc(&battery_info.soc);
  app_log_status(sc);
  app_log("soc: %d%%\r\n", battery_info.soc);

  // get CRATE
  sc = max17048_get_crate(&battery_info.crate);
  app_log_status(sc);
  app_log("soc rate: %f C\r\n", battery_info.crate);
  refresh_display();

  tick = sl_sleeptimer_ms_to_tick(5000);
  sc = sl_sleeptimer_start_timer(&max17048_timer,
                                 tick,
                                 max17048_timer_callback,
                                 (void *)NULL,
                                 0,
                                 0);
}

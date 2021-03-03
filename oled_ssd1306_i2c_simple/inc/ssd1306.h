/***************************************************************************//**
 * @file
 * @brief SSD1306 interface
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

#ifndef SSD1306_H
#define SSD1306_H

#include "ssd1306_config.h"
#include "sl_status.h"

#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

/* Fundamental Command */
#define SSD1306_SETCONTRAST                           0x81
#define SSD1306_DISPLAYALLON_RESUME                   0xA4
#define SSD1306_DISPLAYALLON                          0xA5
#define SSD1306_NORMALDISPLAY                         0xA6
#define SSD1306_INVERTDISPLAY                         0xA7
#define SSD1306_DISPLAYOFF                            0xAE
#define SSD1306_DISPLAYON                             0xAF

/* Scrolling Command */
#define SSD1306_RIGHT_HORIZONTAL_SCROLL               0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL                0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL  0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL   0x2A
#define SSD1306_DEACTIVATE_SCROLL                     0x2E
#define SSD1306_ACTIVATE_SCROLL                       0x2F
#define SSD1306_SET_VERTICAL_SCROLL_AREA              0xA3

/* Addressing Setting Command */
#define SSD1306_SETLOWCOLUMN                          0x00
#define SSD1306_SETHIGHCOLUMN                         0x10
#define SSD1306_MEMORYMODE                            0x20
#define SSD1306_COLUMNADDR                            0x21
#define SSD1306_PAGEADDR                              0x22

/* Hardware Configuration Command */
#define SSD1306_SETSTARTLINE                          0x40
#define SSD1306_SEGREMAP                              0xA0
#define SSD1306_SETSEGMENTREMAP                       0xA1
#define SSD1306_SETMULTIPLEX                          0xA8
#define SSD1306_COMSCANINC                            0xC0
#define SSD1306_COMSCANDEC                            0xC8
#define SSD1306_SETDISPLAYOFFSET                      0xD3
#define SSD1306_SETDISPLAYCLOCKDIV                    0xD5
#define SSD1306_SETCOMPINS                            0xDA
#define SSD1306_SETPRECHARGE                          0xD9
#define SSD1306_SETVCOMDETECT                         0xDB

/* Charge Pump Command */
#define SSD1306_CHARGEPUMP                            0x8D

/***************************************************************************//**
 * @addtogroup ssd1306
 * @brief ssd1306 interface.
 * @{
 ******************************************************************************/

/**
 * General ssd1306 data structure.
 */
typedef struct ssd1306_t {
  unsigned short width;       ///< Display pixel width
  unsigned short height;      ///< Display pixel height
} ssd1306_t;


sl_status_t ssd1306_init(void);
sl_status_t ssd1306_draw(const void *data);
const ssd1306_t *ssd1306_get(void);

sl_status_t ssd1306_invert_color(void);
sl_status_t ssd1306_normal_color(void);
sl_status_t ssd1306_set_contrast(uint8_t value);
sl_status_t ssd1306_scroll_right(uint8_t start_page_addr, uint8_t end_page_addr);
sl_status_t ssd1306_scroll_left(uint8_t start_page_addr, uint8_t end_page_addr);
sl_status_t ssd1306_scroll_diag_right(uint8_t start_page_addr, uint8_t end_page_addr);
sl_status_t ssd1306_scroll_diag_left(uint8_t start_page_addr, uint8_t end_page_addr);
sl_status_t ssd1306_stop_scroll(void);
sl_status_t ssd1306_display_on(bool on);

#ifdef __cplusplus
}
#endif

/** @} */

#endif

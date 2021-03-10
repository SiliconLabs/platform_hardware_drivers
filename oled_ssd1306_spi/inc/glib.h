/***************************************************************************//**
 * @file glib.h
 * @brief Silicon Labs Graphics Library
 *******************************************************************************
* # License
* <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* SPDX-License-Identifier: Zlib
*
* The licensor of this software is Silicon Laboratories Inc.
*
* This software is provided \'as-is\', without any express or implied
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
*******************************************************************************
*
* EVALUATION QUALITY
* This code has been minimally tested to ensure that it builds with the specified
* dependency versions and is suitable as a demonstration for evaluation purposes only.
* This code will be maintained at the sole discretion of Silicon Labs.
*
******************************************************************************/

#ifndef GLIB_H
#define GLIB_H

/* C-header files */
#include <sl_status.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <glib_font.h>

/* GLIB header files */

#ifdef __cplusplus
extern "C" {
#endif

/** GLIB Base error code */
#define ECODE_GLIB_BASE                         0x00000000

/* Error codes */
/** Successful call */
#define GLIB_OK                                 0x00000000
/** Function did not draw */
#define GLIB_ERROR_NOTHING_TO_DRAW              (ECODE_GLIB_BASE | 0x0001)
/** Invalid char */
#define GLIB_ERROR_INVALID_CHAR                 (ECODE_GLIB_BASE | 0x0002)
/** Coordinates out of bounds */
#define GLIB_OUT_OF_BOUNDS                      (ECODE_GLIB_BASE | 0x0003)
/** Invalid coordinates (ex. xMin > xMax) */
#define GLIB_ERROR_INVALID_REGION               (ECODE_GLIB_BASE | 0x0004)
/** Invalid argument */
#define GLIB_ERROR_INVALID_ARGUMENT             (ECODE_GLIB_BASE | 0x0005)
/** Out of memory */
#define GLIB_ERROR_OUT_OF_MEMORY                (ECODE_GLIB_BASE | 0x0006)
/** File not supported */
#define GLIB_ERROR_FILE_NOT_SUPPORTED           (ECODE_GLIB_BASE | 0x0007)
/** General IO Error */
#define GLIB_ERROR_IO                           (ECODE_GLIB_BASE | 0x0008)
/** Invalid file */
#define GLIB_ERROR_INVALID_FILE                 (ECODE_GLIB_BASE | 0x0009)
/** Driver not initialized correctly */
#define GLIB_ERROR_DRIVER_NOT_INITIALIZED       (ECODE_GLIB_BASE | 0x000A)

#define Black      0x00  // Black color, no pixel
#define White      0x01  // Pixel is set. Color depends on OLED

typedef uint32_t   glib_status_t;

/** @struct glib_display_geometry
 *  @brief Dimensions of the display
 */
typedef struct __glib_display_geometry_t{
  /** Horizontal size of the display, in pixels */
  uint16_t xSize;
  /** Vertical size of the display, in pixels */
  uint16_t ySize;
} glib_display_geometry_t; /**< Typedef for display dimensions */

/** @brief Font definition structure
 */
typedef struct __glib_font_t{
  /** Width in pixels of each character. */
  uint8_t width;

  /** Height in pixels of each character. */
  uint8_t height;

  /** Number of pixels between each character in this font. */
  uint8_t spacing;

  /** Pointer to the pixel map for the font. */
  const uint16_t *data;
} glib_font_t;


/** @brief GLIB Drawing Context
 *  (Multiple instances of glib_context_t can exist)
 */
typedef struct __glib_context_t{
  /** Background color */
  uint32_t backgroundColor;

  /** Foreground color */
  uint32_t foregroundColor;

  /** Font definition */
  glib_font_t font;
} glib_context_t;

/* Fonts included in the library */
#ifdef GLIB_INCLUDE_FONT_6x8
extern const glib_font_t glib_font_6x8; /* Default */
#endif
#ifdef GLIB_INCLUDE_FONT_7x10
extern const glib_font_t glib_font_7x10;
#endif
#ifdef GLIB_INCLUDE_FONT_11x18
extern const glib_font_t glib_font_11x18;
#endif
#ifdef GLIB_INCLUDE_FONT_16x26
extern const glib_font_t glib_font_16x26;
#endif

glib_status_t glib_init(void);
glib_status_t glib_update_display(void);
glib_status_t glib_clear(glib_context_t *pContext);
glib_status_t glib_set_font(glib_context_t *pContext, glib_font_t *pFont);
glib_status_t glib_draw_pixel(glib_context_t *pContext, int32_t x, int32_t y);
glib_status_t glib_draw_char(glib_context_t *pContext, char my_char, int32_t x, int32_t y);
glib_status_t glib_draw_string(glib_context_t *pContext, const char* str,
                         int32_t x0, int32_t y0);
glib_status_t glib_draw_bmp(glib_context_t *pContext, const uint8_t *data);
glib_status_t glib_draw_line(glib_context_t *pContext, uint8_t x1, uint8_t y1,
                                                     uint8_t x2, uint8_t y2);
glib_status_t glib_draw_rectangle(glib_context_t *pContext, uint8_t x1, uint8_t y1,
                                                          uint8_t x2, uint8_t y2);
glib_status_t glib_draw_circle(glib_context_t *pContext, uint8_t par_x, uint8_t par_y,
                                                uint8_t par_r);

glib_status_t glib_set_invert_color(void);
glib_status_t glib_set_normal_color(void);
glib_status_t glib_set_contrast(uint8_t value);
glib_status_t glib_scroll_right(uint8_t start_page_addr, uint8_t end_page_addr);
glib_status_t glib_scroll_left(uint8_t start_page_addr, uint8_t end_page_addr);
glib_status_t glib_scroll_diag_right(uint8_t start_page_addr, uint8_t end_page_addr);
glib_status_t glib_scroll_diag_left(uint8_t start_page_addr, uint8_t end_page_addr);
glib_status_t glib_stop_scroll(void);
glib_status_t glib_enable_display(bool on);

#ifdef __cplusplus
}
#endif

#endif

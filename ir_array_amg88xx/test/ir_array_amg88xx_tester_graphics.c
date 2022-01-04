/***************************************************************************//**
 * @file ir_array_amg88xx_tester_graphics.c
 * @brief amg88xx graphics functions.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
* # Evaluation Quality
* This code has been minimally tested to ensure that it builds and is suitable
* as a demonstration for evaluation purposes only. This code will be maintained
* at the sole discretion of Silicon Labs.
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "sl_board_control.h"
#include "glib.h"
#include "dmd.h"
#include "em_assert.h"
#include "sl_memlcd_display.h"
#include "ir_array_amg88xx_driver.h"

// -----------------------------------------------------------------------------
//                                   Defines
// -----------------------------------------------------------------------------

#define MAX_STRING_LENGTH       (SL_MEMLCD_DISPLAY_WIDTH / 8)
#define TEXT_DISPLAY_X_OFFSET   2
#define TEXT_DISPLAY_Y_OFFSET   4
#define FIELD_SIZE              16

// -----------------------------------------------------------------------------
//                                Local Variables
// -----------------------------------------------------------------------------
static GLIB_Context_t glibContext;

// -----------------------------------------------------------------------------
//                                Global Functions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * Init graphics.
 ******************************************************************************/
void init_graphics(void)
{
  uint32_t status;

  // Enable the memory lcd
  status = sl_board_enable_display();
  EFM_ASSERT(status == SL_STATUS_OK);

  // Initialize the DMD support for memory lcd display
  status = DMD_init(0);
  EFM_ASSERT(status == DMD_OK);

  // Initialize the glib context
  status = GLIB_contextInit(&glibContext);
  EFM_ASSERT(status == GLIB_OK);

  glibContext.backgroundColor = White;
  glibContext.foregroundColor = Black;

  // Fill lcd with background color
  GLIB_clear(&glibContext);
}

/***************************************************************************//**
 * Print error message.
 *
 * The msg length should be less than the MAX_STRING_LENGTH of the screen.
 *
 ******************************************************************************/
void print_error_msg(const char *msg, const int error_code)
{
  glibContext.backgroundColor = Black;
  glibContext.foregroundColor = White;

  GLIB_clear(&glibContext);

  GLIB_setFont(&glibContext, (GLIB_Font_t *) &GLIB_FontNormal8x8);
  if (strlen(msg) > MAX_STRING_LENGTH) {
    GLIB_drawStringOnLine(&glibContext,
                          "Too long err msg",
                          5,
                          GLIB_ALIGN_CENTER,
                          0,
                          0,
                          true);
  } else {
    GLIB_drawStringOnLine(&glibContext,
                          msg,
                          5,
                          GLIB_ALIGN_CENTER,
                          0,
                          0,
                          true);
    char return_code_text[MAX_STRING_LENGTH + 1];
    sprintf(return_code_text, "Error code: %d", error_code);
    GLIB_drawStringOnLine(&glibContext,
                          return_code_text,
                          6,
                          GLIB_ALIGN_CENTER,
                          0,
                          0,
                          true);
  }

  DMD_updateDisplay();

  while (1);
}

/***************************************************************************//**
 * Prints the temperature grid to the screen.
 ******************************************************************************/
void print_temps(
                float temperature_grid[SENSOR_ARRAY_COLUMNS][SENSOR_ARRAY_ROWS])
{

  GLIB_clear(&glibContext);

  // Draw background, and temperature values.
  GLIB_setFont(&glibContext, (GLIB_Font_t *) &GLIB_FontNarrow6x8);
  for (int i = 0; i < SENSOR_ARRAY_COLUMNS; i++) {
    for (int j = 0; j < SENSOR_ARRAY_ROWS; j++) {
      if (temperature_grid[i][j] >= 40) {
        glibContext.backgroundColor = Red;
        glibContext.foregroundColor = Red;
      } else if (temperature_grid[i][j] >= 30) {
        glibContext.backgroundColor = Yellow;
        glibContext.foregroundColor = Yellow;
      } else if (temperature_grid[i][j] >= 26) {
        glibContext.backgroundColor = Green;
        glibContext.foregroundColor = Green;
      } else {
        glibContext.backgroundColor = Blue;
        glibContext.foregroundColor = Blue;
      }

      GLIB_Rectangle_t rectangle = {i*FIELD_SIZE,
                                    j*FIELD_SIZE,
                                    i*FIELD_SIZE + FIELD_SIZE,
                                    j*FIELD_SIZE + FIELD_SIZE};
      GLIB_drawRectFilled(&glibContext, &rectangle);

      glibContext.foregroundColor = Black;
      char temp_string[3];
      sprintf(temp_string, "%d", (int16_t)temperature_grid[i][j]);
      GLIB_drawString(&glibContext, temp_string, 3,
                      TEXT_DISPLAY_X_OFFSET + 16*i,
                      TEXT_DISPLAY_Y_OFFSET + 16*j,
                      true);
    }
  }

  // Draw grid.
  for (int i = 0; i < 7; i++) {
    GLIB_drawLineH(&glibContext,
                   0,
                   FIELD_SIZE + i * FIELD_SIZE - 1,
                   SL_MEMLCD_DISPLAY_WIDTH - 1);
    GLIB_drawLineV(&glibContext,
                   FIELD_SIZE + i * FIELD_SIZE - 1,
                   0,
                   SL_MEMLCD_DISPLAY_HEIGHT - 1);
  }

  DMD_updateDisplay();
}

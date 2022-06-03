/***************************************************************************//**
 * @file app.c
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ********************************************************************************
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
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/
#include "string.h"

#include "diskio.h"
#include "ff.h"

#include "app_log.h"
#include "app_assert.h"

#if !FF_FS_NORTC && !FF_FS_READONLY
#include "sl_sleeptimer.h"
#endif

static const char str[] = "Silabs SD Card I/O Example via SPI!\r\n";
static const char* fst[] = {"", "FAT12", "FAT16", "FAT32", "exFAT"};

BYTE f_work[FF_MAX_SS]; // Work area (larger is better for processing time)

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  FATFS fs, *pfs;
  FIL fil;
  DWORD fre_clust;
  FRESULT ret_code;
  UINT bw, br;
#ifdef _ENABLE_FORMAT_SD
  MKFS_PARM fmt_opt = {FM_FAT32, 0, 0, 0, 0};
#endif

#if !FF_FS_NORTC && !FF_FS_READONLY
  DWORD time_data;
  sl_status_t sc;
  sl_sleeptimer_date_t date = {
      .year = 122,
      .month = 6,
      .month_day = 2,
      .hour = 10,
      .min = 30,
      .sec = 0,
   };

  sc = sl_sleeptimer_set_datetime(&date);
  app_assert_status(sc);

  time_data = get_fattime();
  app_log("\nCurrent time is %u/%u/%u %2u:%02u:%02u.\n\n",
          (time_data >> 25) + 1980,
          (time_data >> 21) & 0x0f,
          (time_data >> 16) & 0x1f,
          (time_data >> 11) & 0x1f,
          (time_data >> 5) & 0x3f,
          (time_data << 1) & 0x1f);
#endif

  // Give a work area to the default drive
  ret_code = f_mount(&fs, "", 0);
  app_assert_status(ret_code);

  // Show logical drive status
  ret_code = f_getfree("", &fre_clust, &pfs);
  app_assert_status(ret_code);
  app_log("-------------- Volume status --------------\n\r");
  app_log(("FAT type = %s\nBytes/Cluster = %lu\nNumber of FATs = %u\n"
           "Root DIR entries = %u\nSectors/FAT = %lu\n"
           "Number of clusters = %lu\nVolume start (lba) = %lu\n"
           "FAT start (lba) = %lu\nDIR start (lba,clustor) = %lu\n"
           "Data start (lba) = %lu\n%lu KiB total disk space.\n"
           "%lu KiB available.\n\n"),
            fst[pfs->fs_type],
            (DWORD)pfs->csize * 512,
            pfs->n_fats,
            pfs->n_rootdir,
            pfs->fsize,
            pfs->n_fatent - 2,
            (DWORD)pfs->volbase,
            (DWORD)pfs->fatbase,
            (DWORD)pfs->dirbase,
            (DWORD)pfs->database,
            (pfs->n_fatent - 2) * (pfs->csize / 2),
            fre_clust * (pfs->csize / 2));

  app_log("-------- Open file to write and read again ---------\n\r");
  // Open file to write
  ret_code = f_open(&fil, "hello.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
  app_assert_status(ret_code);

  // Write a message
  ret_code = f_write(&fil, str, sizeof(str), &bw);
  app_assert_status(ret_code);
  app_log("Write a message to SD card success! Byte writen = %d\n\r", bw);

  // Close file
  ret_code = f_close(&fil);
  app_assert_status(ret_code);

  // Open file to read
  ret_code = f_open(&fil, "hello.txt", FA_OPEN_EXISTING | FA_READ);
  app_assert_status(ret_code);

  // Read back the content and print on the console
  ret_code = f_read(&fil, f_work, sizeof(f_work), &br);
  app_assert_status(ret_code);
  app_log("Read a message from SD card success! Byte read = %d\n\r", br);
  app_log("Content: %s", f_work);

  // Close file
  ret_code = f_close(&fil);
  app_assert_status(ret_code);

  // Unmount SDCARD
  (void)f_mount(NULL, "", 1);
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
}

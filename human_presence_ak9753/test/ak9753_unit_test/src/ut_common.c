/***************************************************************************//**
* @file ut_common.c
* @brief unit test common file
*******************************************************************************
* # License
* <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "ut_common.h"

#ifdef __cplusplus
extern "C" {
#endif

gStatus_t gTestStatus;

void run_all(testCase_t *pTest, const uint8_t testCaseNum)
{
  for (uint8_t i = 0; i < testCaseNum; i++) {
    //execute test
    gTestStatus = PASSED;
    
    pTest[i].func();
    app_log("Test: %s ", pTest[i].testName);

    //print test log
    if (gTestStatus == PASSED) {
      app_log("...passed\n");
    }
    else {
      app_log("...failed\n");
    }
  }
}

#ifdef __cplusplus
}
#endif
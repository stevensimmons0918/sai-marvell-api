/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssAppPlatformLogLib.h
*
* @brief CPSS Application Platform Logging Facility Implementation
*
* @version   1
********************************************************************************
*/
#ifndef __CPSS_APP_PLATFORM_LOGLIB_H
#define __CPSS_APP_PLATFORM_LOGLIB_H

typedef enum _CPSS_APP_PLATFORM_LOG_LEVEL_ENT
{
  CPSS_APP_PLATFORM_LOG_LEVEL_NONE_E         = 0x00,
  CPSS_APP_PLATFORM_LOG_LEVEL_ERR_E          = 0x01,
  CPSS_APP_PLATFORM_LOG_LEVEL_INFO_E         = 0x02,
  CPSS_APP_PLATFORM_LOG_LEVEL_DBG_E          = 0x04,
  CPSS_APP_PLATFORM_LOG_LEVEL_ENTRY_E        = 0x08,
  CPSS_APP_PLATFORM_LOG_LEVEL_EXIT_E         = 0x10,

  /* Must be print. Eg. cmd output */
  CPSS_APP_PLATFORM_LOG_LEVEL_PRINT_E        = 0x20,

  /* All = all except _PRINT_ */
  CPSS_APP_PLATFORM_LOG_LEVEL_ALL_E          = 0xFF | CPSS_APP_PLATFORM_LOG_LEVEL_PRINT_E,
  CPSS_APP_PLATFORM_LOG_LEVEL_LAST_E         = 0xFF
} CPSS_APP_PLATFORM_LOG_LEVEL_ENT;

typedef enum _CPSS_APP_PLATFORM_LOG_MODULE_ENT
{
  CPSS_APP_PLATFORM_LOG_MODULE_APPPLAT_E,
  CPSS_APP_PLATFORM_LOG_MODULE_APPREF_E,
  CPSS_APP_PLATFORM_LOG_MODULE_CUSTOM_E,

  CPSS_APP_PLATFORM_LOG_MODULE_LAST_E
} CPSS_APP_PLATFORM_LOG_MODULE_ENT;

extern CPSS_APP_PLATFORM_LOG_LEVEL_ENT capLogDb [CPSS_APP_PLATFORM_LOG_MODULE_LAST_E];

#ifndef CAP_LOG_MODULE
#define CAP_LOG_MODULE CPSS_APP_PLATFORM_LOG_MODULE_APPPLAT_E
#endif

#define CPSS_APP_PLAT_PRINTF cpssOsPrintf

#define CPSS_APP_PLATFORM_LOG_ERR_MAC(...) \
do {                                                                           \
 if (capLogDb[CAP_LOG_MODULE] & CPSS_APP_PLATFORM_LOG_LEVEL_ERR_E)             \
 {                                                                             \
     CPSS_APP_PLAT_PRINTF("<ERR> in %s at %s:%d :: ", __func__, __FILE__, __LINE__);  \
     CPSS_APP_PLAT_PRINTF(__VA_ARGS__);                                        \
 }                                                                             \
} while (0)

#define CPSS_APP_PLATFORM_LOG_INFO_MAC(...) \
 if (capLogDb[CAP_LOG_MODULE] & CPSS_APP_PLATFORM_LOG_LEVEL_INFO_E) \
     CPSS_APP_PLAT_PRINTF("<INFO> "__VA_ARGS__)

#define CPSS_APP_PLATFORM_LOG_DBG_MAC(...) \
 if (capLogDb[CAP_LOG_MODULE] & CPSS_APP_PLATFORM_LOG_LEVEL_DBG_E) \
     CPSS_APP_PLAT_PRINTF("\n<DBG> "__VA_ARGS__)

#define CPSS_APP_PLATFORM_LOG_ENTRY_MAC() \
 if (capLogDb[CAP_LOG_MODULE] & CPSS_APP_PLATFORM_LOG_LEVEL_ENTRY_E) \
     CPSS_APP_PLAT_PRINTF("\n[IN] %s\n", __func__)

#define CPSS_APP_PLATFORM_LOG_EXIT_MAC(_rc) \
 if (capLogDb[CAP_LOG_MODULE] & CPSS_APP_PLATFORM_LOG_LEVEL_EXIT_E) \
     CPSS_APP_PLAT_PRINTF("\n[OUT] %s RET=0x%x\n", __func__, _rc)

#define CPSS_APP_PLATFORM_LOG_PRINT_MAC(...) \
 if (capLogDb[CAP_LOG_MODULE] & CPSS_APP_PLATFORM_LOG_LEVEL_PRINT_E) \
     CPSS_APP_PLAT_PRINTF(__VA_ARGS__)

#define CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(_rc, _fnName)                                                   \
    do {                                                                                                      \
          if (_rc != GT_OK)                                                                                   \
          {                                                                                                   \
              CPSS_APP_PLATFORM_LOG_ERR_MAC("%s ret=0x%X \n", #_fnName, _rc);                                 \
              CPSS_APP_PLATFORM_LOG_EXIT_MAC(_rc);                                                            \
              return _rc;                                                                                     \
          }                                                                                                   \
       } while(0)

#endif /* __CPSS_APP_PLATFORM_LOGLIB_H */

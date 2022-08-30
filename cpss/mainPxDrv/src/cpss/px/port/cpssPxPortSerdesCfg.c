/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file cpssPxPortSerdesCfg.c
*
* @brief CPSS implementation for SerDes configuration and control facility.
*
* @version   88
********************************************************************************
*/
#define PIPE_MISSING 0
/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/port/private/prvCpssPxPortLog.h>

#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH/mvComPhyEomIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nm/mvComPhyH28nmEomIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* #define SERDES_CFG_DBG */
#ifdef SERDES_CFG_DBG
#define SERDES_DBG_DUMP_MAC(x)     cpssOsPrintf x
#define SERDES_TUNE_DBG
#else
#define SERDES_DBG_DUMP_MAC(x)
#endif

/* #define SERDES_TUNE_DBG */
#ifdef SERDES_TUNE_DBG
#define SERDES_TUNE_DUMP_MAC(x)     cpssOsPrintf x
#else
#define SERDES_TUNE_DUMP_MAC(x)
#endif

#define PRV_CPSS_PX_PORT_SERDES_SPEED_HW_TO_SW_CONVERT_MAC(swSerdesSpeed, hwSerdesSpeed)\
    switch(hwSerdesSpeed)                                                                 \
    {                                                                                     \
        case SPEED_NA:                                                                    \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_NA_E;                             \
            break;                                                                        \
        case _1_25G:                                                                      \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_1_25_E;                           \
            break;                                                                        \
        case _3_125G:                                                                     \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_3_125_E;                          \
            break;                                                                        \
        case _3_33G:                                                                      \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_3_333_E;                          \
            break;                                                                        \
        case _3_75G:                                                                      \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_3_75_E;                           \
            break;                                                                        \
        case _4_25G:                                                                      \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_4_25_E;                           \
            break;                                                                        \
        case _5G:                                                                         \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_5_E;                              \
            break;                                                                        \
        case _6_25G:                                                                      \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_6_25_E;                           \
            break;                                                                        \
        case _7_5G:                                                                       \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_7_5_E;                            \
            break;                                                                        \
        case _10_3125G:                                                                   \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_10_3125_E;                        \
            break;                                                                        \
        case _11_25G:                                                                     \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_11_25_E;                          \
            break;                                                                        \
        case _11_5625G:                                                                   \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_11_5625_E;                        \
            break;                                                                        \
        case _12_5G:                                                                      \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_12_5_E;                           \
            break;                                                                        \
        case _10_9375G:                                                                   \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_10_9375_E;                        \
            break;                                                                        \
        case _12_1875G:                                                                   \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_12_1875_E;                        \
            break;                                                                        \
        case _5_625G:                                                                     \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_5_625_E;                          \
            break;                                                                        \
        case _5_15625G:                                                                   \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_5_156_E;                          \
            break;                                                                        \
        case _12_8906G:                                                                   \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_12_8906_E;                        \
            break;                                                                        \
        case _20_625G:                                                                    \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_20_625_E;                         \
            break;                                                                        \
        case _25_78125G:                                                                  \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_25_78125_E;                       \
            break;                                                                        \
        case _27_5G:                                                                      \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_27_5_E;                             \
            break;                                                                        \
        case _28_05G:                                                                     \
            swSerdesSpeed = CPSS_PORT_SERDES_SPEED_28_05_E;                            \
            break;                                                                        \
        default:                                                                          \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                \
    }

#if 0
/* array defining serdes speed used in given interface mode for given port data speed */

CPSS_PORT_SERDES_SPEED_ENT serdesPxFrequency[CPSS_PORT_INTERFACE_MODE_NA_E][CPSS_PORT_SPEED_NA_E] =
{                                               /* 10M                                      100M                                  1G                                        10G                                   12G                               2.5G                                  5G                                     13.6G                           20G                                   40G                                   16G                            15G                             75G                             100G                    */
/* CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E */  {CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   ,  CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E  */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   ,  CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_MII_E           */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   ,  CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_SGMII_E         */ ,{CPSS_PORT_SERDES_SPEED_1_25_E , CPSS_PORT_SERDES_SPEED_1_25_E, CPSS_PORT_SERDES_SPEED_1_25_E, CPSS_PORT_SERDES_SPEED_NA_E   ,  CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_3_125_E, CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_XGMII_E         */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_3_125_E,  CPSS_PORT_SERDES_SPEED_3_75_E, CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_4_25_E, CPSS_PORT_SERDES_SPEED_6_25_E, CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_5_E , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_MGMII_E         */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   ,  CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,   */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_1_25_E, CPSS_PORT_SERDES_SPEED_NA_E   ,  CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_GMII_E,         */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   ,  CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_MII_PHY_E,      */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   ,  CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_QX_E,           */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   ,  CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_3_125_E, CPSS_PORT_SERDES_SPEED_6_25_E , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_HX_E,           */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_6_25_E ,  CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_3_125_E, CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_RXAUI_E,        */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_6_25_E ,  CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_100BASE_FX_E,   */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_1_25_E, CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   ,  CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_QSGMII_E,       */ ,{CPSS_PORT_SERDES_SPEED_5_E    , CPSS_PORT_SERDES_SPEED_5_E   , CPSS_PORT_SERDES_SPEED_5_E   , CPSS_PORT_SERDES_SPEED_NA_E   ,  CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_XLG_E,          */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   ,  CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_5_156_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E   */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_3_125_E,  CPSS_PORT_SERDES_SPEED_3_75_E, CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_4_25_E, CPSS_PORT_SERDES_SPEED_6_25_E, CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_KR_E            */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_10_3125_E,  CPSS_PORT_SERDES_SPEED_12_5_E, CPSS_PORT_SERDES_SPEED_NA_E , CPSS_PORT_SERDES_SPEED_5_156_E, CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_10_3125_E, CPSS_PORT_SERDES_SPEED_10_3125_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_10_3125_E}
/* CPSS_PORT_INTERFACE_MODE_HGL_E           */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_3_125_E,  CPSS_PORT_SERDES_SPEED_3_75_E, CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_4_25_E, CPSS_PORT_SERDES_SPEED_6_25_E, CPSS_PORT_SERDES_SPEED_12_5_E , CPSS_PORT_SERDES_SPEED_3_333_E, CPSS_PORT_SERDES_SPEED_3_125_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_12_5_E}
/* CPSS_PORT_INTERFACE_MODE_CHGL_12_E       */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_3_125_E,  CPSS_PORT_SERDES_SPEED_3_75_E, CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_4_25_E, CPSS_PORT_SERDES_SPEED_6_25_E, CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_ILKN12_E        */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_3_125_E,  CPSS_PORT_SERDES_SPEED_3_75_E, CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_4_25_E, CPSS_PORT_SERDES_SPEED_6_25_E, CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_SR_LR_E         */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_10_3125_E,  CPSS_PORT_SERDES_SPEED_12_5_E, CPSS_PORT_SERDES_SPEED_NA_E , CPSS_PORT_SERDES_SPEED_5_156_E, CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_10_3125_E, CPSS_PORT_SERDES_SPEED_10_3125_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_10_3125_E}
/* CPSS_PORT_INTERFACE_MODE_ILKN16_E        */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_3_125_E,  CPSS_PORT_SERDES_SPEED_3_75_E, CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_4_25_E, CPSS_PORT_SERDES_SPEED_6_25_E, CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
/* CPSS_PORT_INTERFACE_MODE_ILKN24_E        */ ,{CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_NA_E  , CPSS_PORT_SERDES_SPEED_3_125_E,  CPSS_PORT_SERDES_SPEED_3_75_E, CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_4_25_E, CPSS_PORT_SERDES_SPEED_6_25_E, CPSS_PORT_SERDES_SPEED_NA_E   , CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E, CPSS_PORT_SERDES_SPEED_NA_E}
};
#endif

/**
* @internal prvCpssPxPortIfModeSerdesNumGet function
* @endinternal
*
* @brief   Get number of first serdes and quantity of serdeses occupied by given
*         port in given interface mode. Extended function used directly only in
*         special cases when naturally error would be returned.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port media interface mode
*
* @param[out] startSerdesPtr           - first used serdes number
* @param[out] numOfSerdesLanesPtr      - quantity of serdeses occupied
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if port doesn't support given interface mode
* @retval GT_NOT_SUPPORTED         - wrong device family
*/
GT_STATUS prvCpssPxPortIfModeSerdesNumGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    OUT GT_U32                          *startSerdesPtr,
    OUT GT_U32                          *numOfSerdesLanesPtr
)
{
    GT_STATUS       rc;             /* return code */
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U8           sdVecSize;      /* size of serdes redundancy array */
    GT_U16          *sdVectorPtr;   /* serdes redundancy array */
    GT_PHYSICAL_PORT_NUM localPort; /* number of port in its port group */
    GT_U32          portMacMap;     /* MAC number */
    GT_U32          portGroup;      /* local core number */


    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    *startSerdesPtr = portMacMap;
    *numOfSerdesLanesPtr = 0 ;
#if PIPE_MISSING
    if(prvCpssPxPortRemotePortCheck(devNum,portNum))
    {
        return GT_OK;
    }
#endif

    localPort = portMacMap;

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

    if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
    {
            *numOfSerdesLanesPtr = 0;
            *startSerdesPtr = 0;
    }
    else
    {
        MV_HWS_PORT_STANDARD portMode;  /* port i/f mode and speed translated to BlackBox enum */
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode,
                            PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap),
                            &portMode);
        if(rc != GT_OK)
        {
            return rc;
        }
        if((ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E) ||
           (ifMode == CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E))
        {
            localPort = localPort&0xFFFFFFFC;
        }

        rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup, localPort, portMode, &curPortParams);
        if (GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        sdVecSize   = curPortParams.numOfActLanes;
        sdVectorPtr = curPortParams.activeLanesList;

        *startSerdesPtr = sdVectorPtr[0];
        if(CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E == ifMode)
        {
            *numOfSerdesLanesPtr = 0;
        }
        else
        {
            *numOfSerdesLanesPtr = sdVecSize;
        }
    }

    if(((*startSerdesPtr) + (*numOfSerdesLanesPtr)) > (GT_U32)prvCpssPxHwInitNumOfSerdesGet(devNum))
    {
        /* this check to avoid ACCESS VIOLATION on next DB:
            PRV_CPSS_PX_DEV_REGS_MAC(devNum)->serdesConfig[serdes]
        */

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "DevNum[%d] hold [%d] SERDESes but calculated SERDES[%d..%d] for portMacNum[%d] \n",
            devNum ,
            prvCpssPxHwInitNumOfSerdesGet(devNum) ,
            (*startSerdesPtr) ,
            (*startSerdesPtr) + (*numOfSerdesLanesPtr) - 1,
            portMacMap
            );
    }

    return GT_OK;

}


/**
* @internal prvCpssPxPortIfModeCheckAndSerdesNumGet function
* @endinternal
*
* @brief   Get number of first serdes and quantity of serdeses occupied by given
*         port in given interface mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port media interface mode
*
* @param[out] startSerdesPtr           - first used serdes number
* @param[out] numOfSerdesLanesPtr      - quantity of serdeses occupied
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if port doesn't support given interface mode
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS prvCpssPxPortIfModeCheckAndSerdesNumGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    OUT GT_U32                          *startSerdesPtr,
    OUT GT_U32                          *numOfSerdesLanesPtr
)
{
    return prvCpssPxPortIfModeSerdesNumGet(devNum,portNum,ifMode,startSerdesPtr,numOfSerdesLanesPtr);
}

/**
* @internal prvCpssPxPortNumberOfSerdesLanesGet function
* @endinternal
*
* @brief   Get number of first SERDES and quantity of SERDESes occupied by given
*         port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] firstSerdesPtr           - (pointer to) first used SERDES number
* @param[out] numOfSerdesLanesPtr      - (pointer to) quantity of SERDESes occupied
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - wrong device family
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortNumberOfSerdesLanesGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *firstSerdesPtr,
    OUT GT_U32                          *numOfSerdesLanesPtr
)
{
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* port media interface mode */
    GT_STATUS                       rc ;    /* return code */
    /* get interface type */
    rc = cpssPxPortInterfaceModeGet(devNum,portNum,&ifMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get first SERDES and number of SERDES lanes */
    rc = prvCpssPxPortIfModeCheckAndSerdesNumGet(devNum, portNum, ifMode,
                                                    firstSerdesPtr,
                                                    numOfSerdesLanesPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}



/***********internal functions*************************************************/

/**
* @internal prvCpssPxPortSerdesTuning function
* @endinternal
*
* @brief   Configure serdes tuning values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - core/port group number
*                                      sdVectorPtr - array of numbers of serdeses occupied by port
*                                      sdVecSize   - size of serdeses array
*                                      ifMode      - interface to configure on port
*                                      speed       - speed to configure on port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortSerdesTuning
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  portGroup,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  MV_HWS_PORT_STANDARD    portMode
)
{
    GT_STATUS                       rc;             /* return code */
    CPSS_PORT_SERDES_SPEED_ENT serdesSpeed;    /* required serdes frequency */
    CPSS_PORT_SERDES_TUNE_STC *tuneValuesPtr;  /* ptr to specific entry in tuning array */
    GT_U32                          i;              /* iterator */
    GT_U16                          *sdVectorPtr;
    GT_U8                           sdVecSize;
    GT_U32                          portMacMap;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT    rxConfig;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT    txConfig;

    if(PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr != NULL)
    {/* if serdes fine tuning values DB initialized */

        PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

        if(GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacMap, portMode, &curPortParams))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
        }
        sdVecSize   = curPortParams.numOfActLanes;
        sdVectorPtr = curPortParams.activeLanesList;

        /* convert HWS serdes speed to CPSS serdes speed enumerator */
        PRV_CPSS_PX_PORT_SERDES_SPEED_HW_TO_SW_CONVERT_MAC(serdesSpeed, curPortParams.serdesSpeed);

        for(i = 0; i < sdVecSize; i++)
        {
            if(NULL == PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr[sdVectorPtr[i]])
            {/* if fine tuning values not defined for current serdes skip */
                continue;
            }

            tuneValuesPtr = &PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr[sdVectorPtr[i]][serdesSpeed];

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsAvagoSerdesManualCtleConfig(devNum[%d], portGroup[%d], serdesNum[%d],"
                                     "dcGain[%d], lowFrequency[%d], highFrequency[%d], bandWidth[%d], squelch[%d])",
                                     devNum, portGroup, sdVectorPtr[i],
                                     tuneValuesPtr->rxTune.avago.DC,
                                     tuneValuesPtr->rxTune.avago.LF,
                                     tuneValuesPtr->rxTune.avago.HF,
                                     tuneValuesPtr->rxTune.avago.BW,
                                     tuneValuesPtr->rxTune.avago.sqlch);

            rxConfig.rxAvago.bandWidth      = tuneValuesPtr->rxTune.avago.BW;
            rxConfig.rxAvago.dcGain         = tuneValuesPtr->rxTune.avago.DC;
            rxConfig.rxAvago.highFrequency  = tuneValuesPtr->rxTune.avago.HF;
            rxConfig.rxAvago.lowFrequency   = tuneValuesPtr->rxTune.avago.LF;
            rxConfig.rxAvago.squelch        = tuneValuesPtr->rxTune.avago.sqlch;
            rxConfig.rxAvago.gainshape1 = rxConfig.rxAvago.gainshape2 = rxConfig.rxAvago.dfeGAIN =
            rxConfig.rxAvago.dfeGAIN2   = rxConfig.rxAvago.shortChannelEn = 0;

            rc = mvHwsAvagoSerdesManualCtleConfig(CAST_SW_DEVNUM(devNum),  portGroup, sdVectorPtr[i], &rxConfig);
            if(rc != GT_OK)
            {
                return rc;
            }

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsAvagoSerdesManualTxConfig(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d],"
                                     "TxAtten[%d], TxPost-Cursor[%d], TxPre-Cursor[%d])",
                                     devNum, portGroup, sdVectorPtr[i], HWS_DEV_SERDES_TYPE(devNum, sdVectorPtr[i]),
                                     tuneValuesPtr->txTune.avago.atten,
                                     tuneValuesPtr->txTune.avago.post,
                                     tuneValuesPtr->txTune.avago.pre);

            txConfig.txAvago.atten = tuneValuesPtr->txTune.avago.atten;
            txConfig.txAvago.post  = tuneValuesPtr->txTune.avago.post;
            txConfig.txAvago.pre   = tuneValuesPtr->txTune.avago.pre;
            txConfig.txAvago.pre2  = txConfig.txAvago.pre3 = 0;
            rc = mvHwsAvagoSerdesManualTxConfig(CAST_SW_DEVNUM(devNum), portGroup, sdVectorPtr[i], &txConfig);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortSerdesPolaritySet function
* @endinternal
*
* @brief   Configure the Polarity values on Serdeses if SW DB values initialized.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - core/port group number
* @param[in] sdVectorPtr              - array of numbers of serdeses occupied by port
* @param[in] sdVecSize                - size of serdeses array
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortSerdesPolaritySet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              portGroup,
    IN  GT_U16              *sdVectorPtr,
    IN  GT_U8               sdVecSize
)
{
    GT_STATUS   rc;
    GT_U32      i;
    PRV_CPSS_PX_PORT_SERDES_POLARITY_CONFIG_STC   polarityValues;

    CPSS_NULL_PTR_CHECK_MAC(sdVectorPtr);

    /* if serdes Polarity SW DB values initialized */
    if(PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr != NULL)
    {
        for(i=0; i < sdVecSize; i++)
        {
            polarityValues = PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr[sdVectorPtr[i]];

            /* skip serdes if both Tx & Rx polarity values not defined */
            if ((polarityValues.txPolarity == GT_FALSE) && (polarityValues.rxPolarity == GT_FALSE))
            {
                continue;
            }

            CPSS_LOG_INFORMATION_MAC("Calling: cpssPxPortSerdesLanePolaritySet(devNum[%d], portGroup[%d], serdesNum[%d], invertTx[%d], invertRx[%d])",
                                     devNum, portGroup, sdVectorPtr[i], polarityValues.txPolarity, polarityValues.rxPolarity);

            rc = mvHwsSerdesPolarityConfig(CAST_SW_DEVNUM(devNum), portGroup, (GT_U32)sdVectorPtr[i], HWS_DEV_SERDES_TYPE(devNum, sdVectorPtr[i]), polarityValues.txPolarity, polarityValues.rxPolarity);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPxPortSerdesPolaritySet: error in mvHwsSerdesPolarityConfig\n");
            }
        }
    }

    return GT_OK;
}



/**
* @internal internal_cpssPxPortSerdesPowerStatusSet function
* @endinternal
*
* @brief   Sets power state of SERDES port lanes according to port capabilities.
*         XG / XAUI ports: All 4 (TX or RX) lanes are set simultanuously.
*         HX / QX ports: lanes 0,1 or TX and RX may be set separately.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] powerUp                  - GT_TRUE  = power up, GT_FALSE = power down
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortSerdesPowerStatusSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 powerUp
)
{
    GT_STATUS rc;   /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    rc = prvCpssPxPortSerdesPartialPowerDownSet(devNum, portNum, !powerUp, !powerUp);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortSerdesPowerStatusSet function
* @endinternal
*
* @brief   Sets power state of SERDES port lanes according to port capabilities.
*         XG / XAUI ports: All 4 (TX or RX) lanes are set simultanuously.
*         HX / QX ports: lanes 0,1 or TX and RX may be set separately.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] powerUp                  - GT_TRUE  = power up, GT_FALSE = power down
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortSerdesPowerStatusSet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 powerUp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesPowerStatusSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, powerUp));

    rc = internal_cpssPxPortSerdesPowerStatusSet(devNum, portNum,powerUp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, powerUp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesTuningSet function
* @endinternal
*
* @brief   SerDes fine tuning values set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) where values
*                                      in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
* @param[in] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set same tuning parameters for all lanes defined in laneBmp in SW DB,
*       then cpssPxPortSerdesPowerStatusSet will write them to HW.
*       WARNING: This API requires interface to be already configured on port,
*       kept for backward compatibility and highly advised not to use it.
*       The proper way is to use cpssPxPortSerdesLaneTuningSet
*
*/
static GT_STATUS internal_cpssPxPortSerdesTuningSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneBmp,
    IN  CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    IN  CPSS_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_STATUS rc;   /* return code */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode; /* interface mode of port */
    GT_U32  startSerdes,    /* first serdes occupied by port */
            serdesNumPerPort; /* number of serdeses occupied by port in current
                                interface mode */
    GT_U32  i;      /* iterator */
    GT_U32  lanesNumInDev; /* number of serdes lanes in current device or single
                            core for multi-core devices */
    GT_U32  serdesNumInDevice;    /* global number of serdes in multi-core device */
    GT_U32  maxSerdesNum;   /* maximum serdeses in device */
    GT_U32  portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(tuneValuesPtr);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(serdesFrequency >= CPSS_PORT_SERDES_SPEED_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxPortInterfaceModeGet(devNum, portNum, &ifMode);
    if((rc != GT_OK) && (rc != GT_BAD_STATE))
        return rc;

    if((rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum, ifMode,
                                               &startSerdes,
                                               &serdesNumPerPort)) != GT_OK)
    {
        return rc;
    }
    if(serdesNumPerPort == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "internal_cpssPxPortSerdesTuningSet return code is [%d], serdesNumPerPort is 0", rc);
    }

    if (tuneValuesPtr->type != CPSS_PORT_SERDES_AVAGO_E ||
        tuneValuesPtr->txTune.avago.post     > 32  ||
        tuneValuesPtr->txTune.avago.pre      > 32  ||
        tuneValuesPtr->txTune.avago.post     < -32 ||
        tuneValuesPtr->txTune.avago.pre      < -32 ||
        tuneValuesPtr->txTune.avago.atten    > 31  ||
        tuneValuesPtr->rxTune.avago.sqlch    > 310 ||
        tuneValuesPtr->rxTune.avago.DC       > 255 ||
        tuneValuesPtr->rxTune.avago.LF       > 15  ||
        tuneValuesPtr->rxTune.avago.HF       > 15  ||
        tuneValuesPtr->rxTune.avago.BW       > 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }


    lanesNumInDev = prvCpssPxHwInitNumOfSerdesGet(devNum);

        maxSerdesNum = lanesNumInDev;


    if(NULL == PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr)
    {
        GT_U32 memSize;

        memSize = sizeof(CPSS_PORT_SERDES_TUNE_STC_PTR) * maxSerdesNum;

        PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr =
                        (CPSS_PORT_SERDES_TUNE_STC_PTR*)cpssOsMalloc(memSize);
        if(NULL == PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        cpssOsMemSet(PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr, 0, memSize);
    }

    serdesNumInDevice = startSerdes;

    for(i = 0; (i < lanesNumInDev) && (serdesNumInDevice+i < maxSerdesNum); i++)
    {
        if((laneBmp & (1<<i)) == 0)
            continue;

        if(NULL == PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i])
        {
            PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i] =
                (CPSS_PORT_SERDES_TUNE_STC_PTR)cpssOsMalloc(
                    sizeof(CPSS_PORT_SERDES_TUNE_STC)*CPSS_PORT_SERDES_SPEED_NA_E);
            if(NULL == PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i])
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            cpssOsMemSet(PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i], 0,
                        sizeof(CPSS_PORT_SERDES_TUNE_STC)*CPSS_PORT_SERDES_SPEED_NA_E);
        }

        PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice+i]
                                                [serdesFrequency] = *tuneValuesPtr;

    }

    return GT_OK;
}

/**
* @internal cpssPxPortSerdesTuningSet function
* @endinternal
*
* @brief   SerDes fine tuning values set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) where values
*                                      in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
* @param[in] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set same tuning parameters for all lanes defined in laneBmp in SW DB,
*       then cpssPxPortSerdesPowerStatusSet will write them to HW.
*
*/
GT_STATUS cpssPxPortSerdesTuningSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneBmp,
    IN  CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    IN  CPSS_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesTuningSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneBmp, serdesFrequency, tuneValuesPtr));

    rc = internal_cpssPxPortSerdesTuningSet(devNum, portNum, laneBmp, serdesFrequency, tuneValuesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneBmp, serdesFrequency, tuneValuesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesTuningGet function
* @endinternal
*
* @brief   SerDes fine tuning values get.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
*
* @param[out] tuneValuesPtr            - (ptr to) structure to put tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Reads values saved in SW DB by cpssPxPortSerdesTuningSet or
*       if it was not called, from default matrix (...SerdesPowerUpSequence).
*
*/
static GT_STATUS internal_cpssPxPortSerdesTuningGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneNum,
    IN  CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT CPSS_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_STATUS rc;   /* return code */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode; /* interface mode of port */
    GT_U32  startSerdes,    /* first serdes occupied by port */
            serdesNumPerPort; /* number of serdeses occupied by port in current
                                interface mode */
    GT_U32  portGroupId;    /* the port group Id - support multi-port-groups device */
    GT_U32  lanesNumInDev; /* number of serdes lanes in current device */
    GT_U32  portMacMap; /* number of mac mapped to this physical port */
    GT_U32  serdesNumInDevice;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT results;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    CPSS_NULL_PTR_CHECK_MAC(tuneValuesPtr);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(serdesFrequency >= CPSS_PORT_SERDES_SPEED_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxPortInterfaceModeGet(devNum, portNum, &ifMode);
    if((rc != GT_OK) && (rc != GT_BAD_STATE))
        return rc;

    if((rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum, ifMode, &startSerdes,
                                                &serdesNumPerPort)) != GT_OK)
    {
        return rc;
    }
    if(serdesNumPerPort == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "internal_cpssPxPortSerdesTuningSet return code is [%d], serdesNumPerPort is 0", rc);
    }
    lanesNumInDev = prvCpssPxHwInitNumOfSerdesGet(devNum);

    if(startSerdes+laneNum >= lanesNumInDev)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);
    if(NULL == PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr)
        goto getDefaults;

    serdesNumInDevice = startSerdes + laneNum;

    if(NULL == PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr[serdesNumInDevice])
        goto getDefaults;

    *tuneValuesPtr = PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr
                                        [serdesNumInDevice][serdesFrequency];

    return GT_OK;

getDefaults:

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesAutoTuneResult(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *results)", devNum, portGroupId, startSerdes+laneNum, HWS_DEV_SERDES_TYPE(devNum, startSerdes+laneNum));
        rc = mvHwsSerdesAutoTuneResult(CAST_SW_DEVNUM(devNum), portGroupId, startSerdes+laneNum,
                                        HWS_DEV_SERDES_TYPE(devNum, startSerdes+laneNum),
                                        &results);
        if(rc != GT_OK)
                {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
                }

        tuneValuesPtr->type = CPSS_PORT_SERDES_AVAGO_E;
        tuneValuesPtr->rxTune.avago.sqlch   = results.avagoResults.sqleuch;
        tuneValuesPtr->rxTune.avago.LF      = results.avagoResults.LF;
        tuneValuesPtr->rxTune.avago.HF      = results.avagoResults.HF;
        tuneValuesPtr->rxTune.avago.EO      = results.avagoResults.EO;
        tuneValuesPtr->txTune.avago.post    = results.avagoResults.avagoStc.post;
        tuneValuesPtr->txTune.avago.atten   = results.avagoResults.avagoStc.atten;
        tuneValuesPtr->txTune.avago.pre     = results.avagoResults.avagoStc.pre;
        cpssOsMemCpy(tuneValuesPtr->rxTune.avago.DFE, results.avagoResults.DFE,
                     sizeof(GT_32)*CPSS_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS);

    return GT_OK;
}

/**
* @internal cpssPxPortSerdesTuningGet function
* @endinternal
*
* @brief   SerDes fine tuning values get.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
*
* @param[out] tuneValuesPtr            - (ptr to) structure to put tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Reads values saved in SW DB by cpssPxPortSerdesTuningSet or
*       if it was not called, from default matrix (...SerdesPowerUpSequence).
*
*/
GT_STATUS cpssPxPortSerdesTuningGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneNum,
    IN  CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT CPSS_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesTuningGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, serdesFrequency, tuneValuesPtr));

    rc = internal_cpssPxPortSerdesTuningGet(devNum, portNum, laneNum, serdesFrequency, tuneValuesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, serdesFrequency, tuneValuesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesLaneTuningSet function
* @endinternal
*
* @brief   SerDes lane fine tuning values set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
* @param[in] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set tuning parameters for lane in SW DB, then
*       cpssPxPortSerdesPowerStatusSet or cpssPxPortModeSpeedSet will write
*       them to HW.
*
*/
static GT_STATUS internal_cpssPxPortSerdesLaneTuningSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  portGroupNum,
    IN  GT_U32                  laneNum,
    IN  CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    IN  CPSS_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_U32      lanesNumInDev; /* number of serdes lanes in current device */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(tuneValuesPtr);

    if(0 == (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp
             & (1<<(portGroupNum))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    lanesNumInDev = prvCpssPxHwInitNumOfSerdesGet(devNum);

    if(laneNum >= lanesNumInDev)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(serdesFrequency >= CPSS_PORT_SERDES_SPEED_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (tuneValuesPtr->type != CPSS_PORT_SERDES_AVAGO_E ||
        tuneValuesPtr->txTune.avago.post     > 32  ||
        tuneValuesPtr->txTune.avago.pre      > 32  ||
        tuneValuesPtr->txTune.avago.post     < -32 ||
        tuneValuesPtr->txTune.avago.pre      < -32 ||
        tuneValuesPtr->txTune.avago.atten    > 31  ||
        tuneValuesPtr->rxTune.avago.sqlch    > 310 ||
        tuneValuesPtr->rxTune.avago.DC       > 255 ||
        tuneValuesPtr->rxTune.avago.LF       > 15  ||
        tuneValuesPtr->rxTune.avago.HF       > 15  ||
        tuneValuesPtr->rxTune.avago.BW       > 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }



    /* if DB not allocated yet let's allocate it */
    if(NULL == PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr)
    {
        GT_U32 memSize;

        memSize = sizeof(CPSS_PORT_SERDES_TUNE_STC_PTR) * lanesNumInDev;
        PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr =
                        (CPSS_PORT_SERDES_TUNE_STC_PTR*)cpssOsMalloc(memSize);
        if(NULL == PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        cpssOsMemSet(PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr, 0, memSize);
    }

    /* if entry for required lane not allocated yet, lets allocate it */
    if(NULL == PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNum])
    {
        PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNum] =
            (CPSS_PORT_SERDES_TUNE_STC_PTR)cpssOsMalloc(
                sizeof(CPSS_PORT_SERDES_TUNE_STC)*CPSS_PORT_SERDES_SPEED_NA_E);
        if(NULL == PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNum])
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        cpssOsMemSet(PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNum], 0,
                    sizeof(CPSS_PORT_SERDES_TUNE_STC)*CPSS_PORT_SERDES_SPEED_NA_E);
    }

    PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNum]
                                            [serdesFrequency] = *tuneValuesPtr;

    return GT_OK;
}

/**
* @internal cpssPxPortSerdesLaneTuningSet function
* @endinternal
*
* @brief   SerDes lane fine tuning values set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
* @param[in] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set tuning parameters for lane in SW DB, then
*       cpssPxPortSerdesPowerStatusSet or cpssPxPortModeSpeedSet will write
*       them to HW.
*
*/
GT_STATUS cpssPxPortSerdesLaneTuningSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  portGroupNum,
    IN  GT_U32                  laneNum,
    IN  CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    IN  CPSS_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesLaneTuningSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupNum, laneNum, serdesFrequency, tuneValuesPtr));

    rc = internal_cpssPxPortSerdesLaneTuningSet(devNum, portGroupNum, laneNum, serdesFrequency, tuneValuesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupNum, laneNum, serdesFrequency, tuneValuesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesLaneTuningGet function
* @endinternal
*
* @brief   Get SerDes lane fine tuning values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
*
* @param[out] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get tuning parameters for lane from SW DB.
*
*/
static GT_STATUS internal_cpssPxPortSerdesLaneTuningGet
(
    IN   GT_SW_DEV_NUM           devNum,
    IN   GT_U32                  portGroupNum,
    IN   GT_U32                  laneNum,
    IN   CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT  CPSS_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_U32      lanesNumInDev; /* number of serdes lanes in current device */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(tuneValuesPtr);

    if(0 == (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp
             & (1<<(portGroupNum))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    lanesNumInDev = prvCpssPxHwInitNumOfSerdesGet(devNum);

    if(laneNum >= lanesNumInDev)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(serdesFrequency >= CPSS_PORT_SERDES_SPEED_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(NULL == PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr)
        goto getDefaults;

    if(NULL == PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr[laneNum])
        goto getDefaults;

    *tuneValuesPtr = PRV_CPSS_PX_PP_MAC(devNum)->serdesCfgDbArrPtr
                                        [laneNum][serdesFrequency];

    return GT_OK;

getDefaults:
    /* Defaults parameters get from current configuration - not according to serdesFrequency
    TBD - Defaults parameters should be get according to serdesFrequency */


        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
/* TBD - sfould be implemented code to get default serdes configuration parametwers */
#ifdef DEF_SERDES_PARAMS
        GT_U32  regAddr;
        PRV_CPSS_PX_PORT_SERDES_POWER_UP_ARRAY *serdesPowerUpSequencePtr;
                                        /* pointer to serdes power up sequence */
        PRV_CPSS_PX_PORT_SERDES_EXT_CFG_ARRAY  *serdesExtCfgPtr; /* pointer to
            external registers serdes power up configuration (just dummy here) */
        GT_U32  valueIndex;

        rc = serDesConfig(devNum,&serdesPowerUpSequencePtr,&serdesExtCfgPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(0 == PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            portGroupNum = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        serdesConfig[laneNum].receiverReg0;
        if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupNum, regAddr,
                                                        0, 4, &tuneValuesPtr->sqlch)) != GT_OK)
            return rc;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        serdesConfig[laneNum].dfeF0F1CoefCtrl;
        if((rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupNum, regAddr,
                                                        0, 5, &tuneValuesPtr->dfe)) != GT_OK)
            return rc;

        tuneValuesPtr->ffeC = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_INDEX_E]
                                        [serdesFrequency],0,4);

        tuneValuesPtr->ffeR = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_INDEX_E]
                                        [serdesFrequency],4,3);

        tuneValuesPtr->ffeS = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_INDEX_E]
                                        [serdesFrequency],8,2);

        valueIndex = PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG7_XCAT_INDEX_E;
        tuneValuesPtr->sampler = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr[valueIndex]
                                        [serdesFrequency],8,7);

        tuneValuesPtr->txEmphAmp    = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_INDEX_E]
                                        [serdesFrequency],0,4);
        tuneValuesPtr->txAmp        = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_INDEX_E]
                                        [serdesFrequency],8,5);
        tuneValuesPtr->txAmpAdj     = U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_INDEX_E]
                                        [serdesFrequency],13,1);

        tuneValuesPtr->txEmphEn     = BIT2BOOL_MAC(U32_GET_FIELD_MAC(serdesPowerUpSequencePtr
                                        [PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG0_INDEX_E]
                                        [serdesFrequency],15,1));


    return GT_OK;
#endif
}

/**
* @internal cpssPxPortSerdesLaneTuningGet function
* @endinternal
*
* @brief   Get SerDes lane fine tuning values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane where values in tuneValuesPtr appliable
* @param[in] serdesFrequency          - serdes speed for which to save tuning values
*
* @param[out] tuneValuesPtr            - (ptr to) structure with tuned values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get tuning parameters for lane from SW DB.
*
*/
GT_STATUS cpssPxPortSerdesLaneTuningGet
(
    IN   GT_SW_DEV_NUM           devNum,
    IN   GT_U32                  portGroupNum,
    IN   GT_U32                  laneNum,
    IN   CPSS_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT  CPSS_PORT_SERDES_TUNE_STC *tuneValuesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesLaneTuningGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupNum, laneNum, serdesFrequency, tuneValuesPtr));

    rc = internal_cpssPxPortSerdesLaneTuningGet(devNum, portGroupNum, laneNum, serdesFrequency, tuneValuesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupNum, laneNum, serdesFrequency, tuneValuesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssPxPortSerdesResetStateSet function
* @endinternal
*
* @brief   Set SERDES Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] startSerdes              - first SERDES number
* @param[in] serdesesNum              - number of SERDESes
* @param[in] state                    - Reset state
*                                      GT_TRUE  - Port SERDES is under Reset
*                                      GT_FALSE - Port SERDES is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortSerdesResetStateSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                startSerdes,
    IN  GT_U32                serdesesNum,
    IN  GT_BOOL               state
)
{
    GT_U32    i;              /* iterator */
    GT_U32    portGroupId;    /*the port group Id - support multi-port-groups device */
    GT_U32    portMacMap;
    GT_STATUS rc;


    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

        for(i = startSerdes; i < startSerdes+serdesesNum; i++)
        {
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesReset(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], analogReset[%d], digitalReset[%d], syncEReset[%d])", devNum,portGroupId, i, HWS_DEV_SERDES_TYPE(devNum, i)/*Lion2/3 serdes type*/, state/*analogReset*/, state/*digitalReset*/, state/*syncEReset*/);
            rc = mvHwsSerdesReset(CAST_SW_DEVNUM(devNum),portGroupId, i,
                                    HWS_DEV_SERDES_TYPE(devNum, i),
                                    state/*analogReset*/,
                                    state/*digitalReset*/,
                                    state/*syncEReset*/);

            if(rc != GT_OK)
                        {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
                        }
        }

        return GT_OK;
}

/**
* @internal internal_cpssPxPortSerdesResetStateSet function
* @endinternal
*
* @brief   Set SERDES Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port SERDES is under Reset
*                                      GT_FALSE - Port SERDES is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortSerdesResetStateSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_BOOL               state
)
{
    GT_U32    startSerdes = 0, serdesesNum = 0;
    GT_STATUS rc;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    GT_U32    portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((rc = cpssPxPortInterfaceModeGet(devNum,portNum,&ifMode)) != GT_OK)
        return rc;

    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((rc = prvCpssPxPortIfModeCheckAndSerdesNumGet(devNum,portNum,ifMode,&startSerdes,&serdesesNum)) != GT_OK)
        return rc;

    return prvCpssPxPortSerdesResetStateSet(devNum,portMacMap,startSerdes,serdesesNum,state);
}

/**
* @internal cpssPxPortSerdesResetStateSet function
* @endinternal
*
* @brief   Set SERDES Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port SERDES is under Reset
*                                      GT_FALSE - Port SERDES is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortSerdesResetStateSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_BOOL               state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesResetStateSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssPxPortSerdesResetStateSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesAutoTune function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
*
* @retval GT_OK                    - on success, for TX_TRAINING_STATUS means training
*                                       succeeded
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error, for TX_TRAINING_STATUS means training
*                                       failed
*
* @note Directions for TX training execution:
*       1. As a pre-condition, ports on both sides of the link must be UP.
*       2. For 2 sides of the link call execute TX_TRAINING_CFG;
*       No need to maintain special timing sequence between them.
*       The CFG phase sets some parameters at the SerDes as a preparation
*       to the training phase.
*       3. After CFG is done, for both sides of the link, call TX_TRAINING_START;
*       No need to maintain special timing sequence between them.
*       4. Wait at least 0.5 Sec. (done by API inside).
*       5. Verify Training status by calling TX_TRAINING_STATUS.
*       No need to maintain special timing sequence between them.
*       This call provides the training status (OK/Failed) and terminates it.
*
*/
static GT_STATUS internal_cpssPxPortSerdesAutoTune
(
    IN  GT_SW_DEV_NUM                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup;  /* local core number */
    MV_HWS_PORT_STANDARD    hwsPortMode;/* current ifMode of port in HWS format */
    GT_U32                                        optAlgoMask;/* bitmap of optimization algorithms */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    CPSS_PORT_INTERFACE_MODE_ENT cpssIfMode;    /* current interface of port */
    MV_HWS_AUTO_TUNE_STATUS_RES tuneRes; /*status parmeter needed for some portTuningMode*/

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    cpssIfMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

    hwsPortMode = NON_SUP_MODE;

    if(NON_SUP_MODE == hwsPortMode)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,cpssIfMode,
                                    PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap),
                                    &hwsPortMode);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("portMacMap=%d,ifMode=%d,speed=%d\n", portMacMap, cpssIfMode, PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap));
            return rc;
        }
    }

    optAlgoMask = PRV_CPSS_PX_PORT_SD_OPT_ALG_BMP_MAC(devNum, portMacMap);
    if(CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALL_E == optAlgoMask)
    {
        optAlgoMask = (CPSS_PORT_SERDES_TRAINING_OPTIMISATION_DFE_E
                                  | CPSS_PORT_SERDES_TRAINING_OPTIMISATION_FFE_E
                                  | CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALIGN90_E);
    }

    switch(portTuningMode)
    {
        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, TRxTuneCfg, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                            TRxTuneCfg, optAlgoMask, &tuneRes);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("portMacMap=%d,hwsPortMode=%d,speed=%d\n", portMacMap, hwsPortMode);
            }
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, TRxTuneStart, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                            TRxTuneStart, optAlgoMask, &tuneRes);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, TRxTuneStatus, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                            TRxTuneStatus, optAlgoMask, &tuneRes);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, RxTrainingOnly, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                            RxTrainingOnly, optAlgoMask, &tuneRes);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_TRAINING_STOP_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneStop(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], stopRx[%d], stopTx[%d])", devNum,portGroup,portMacMap,hwsPortMode,GT_TRUE,GT_TRUE);
            rc = mvHwsPortAutoTuneStop(CAST_SW_DEVNUM(devNum),portGroup,portMacMap,hwsPortMode,GT_TRUE,GT_TRUE);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STOP_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneStop(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], stopRx[%d], stopTx[%d])", devNum,portGroup,portMacMap,hwsPortMode,GT_FALSE,GT_TRUE);
            rc = mvHwsPortAutoTuneStop(CAST_SW_DEVNUM(devNum),portGroup,portMacMap,hwsPortMode,GT_FALSE,GT_TRUE);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_STOP_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneStop(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], stopRx[%d], stopTx[%d])", devNum,portGroup,portMacMap,hwsPortMode,GT_TRUE,GT_FALSE);
            rc = mvHwsPortAutoTuneStop(CAST_SW_DEVNUM(devNum),portGroup,portMacMap,hwsPortMode,GT_TRUE,GT_FALSE);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E:
            {
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, RxTrainingAdative, optAlgoMask);
                rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                                RxTrainingAdative, optAlgoMask, &tuneRes);
            }
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E:
            {
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, RxTrainingStopAdative, optAlgoMask);
                rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                                RxTrainingStopAdative, optAlgoMask, &tuneRes);
            }
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_VSR_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, RxTrainingVsr, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                            RxTrainingVsr, optAlgoMask, &tuneRes);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_FIXED_CTLE_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, RxTrainingVsr, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                            FixedCtleTraining, optAlgoMask, &tuneRes);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal cpssPxPortSerdesAutoTune function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
*
* @retval GT_OK                    - on success, for TX_TRAINING_STATUS means training
*                                       succeeded
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error, for TX_TRAINING_STATUS means training
*                                       failed
*
* @note Directions for TX training execution:
*       1. As a pre-condition, ports on both sides of the link must be UP.
*       2. For 2 sides of the link call execute TX_TRAINING_CFG;
*       No need to maintain special timing sequence between them.
*       The CFG phase sets some parameters at the SerDes as a preparation
*       to the training phase.
*       3. After CFG is done, for both sides of the link, call TX_TRAINING_START;
*       No need to maintain special timing sequence between them.
*       4. Wait at least 0.5 Sec. (done by API inside).
*       5. Verify Training status by calling TX_TRAINING_STATUS.
*       No need to maintain special timing sequence between them.
*       This call provides the training status (OK/Failed) and terminates it.
*
*/
GT_STATUS cpssPxPortSerdesAutoTune
(
    IN  GT_SW_DEV_NUM                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT   portTuningMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesAutoTune);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portTuningMode));

    rc = internal_cpssPxPortSerdesAutoTune(devNum, portNum, portTuningMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portTuningMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesAutoTuneExt function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes including required
*         optimizations.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success, for TX_TRAINING_STATUS means training
*                                       succeeded
* @retval GT_BAD_PARAM             - on wrong port number, device, serdesOptAlgBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error, for TX_TRAINING_STATUS means training
*                                       failed
*
* @note See cpssPxPortSerdesAutoTune.
*
*/
static GT_STATUS internal_cpssPxPortSerdesAutoTuneExt
(
    IN  GT_SW_DEV_NUM                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode,
    IN  GT_U32                                   serdesOptAlgBmp
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup;  /* local core number */
    MV_HWS_PORT_STANDARD    hwsPortMode;/* current ifMode of port in HWS format */
    GT_U32                                        optAlgoMask;/* bitmap of optimization algorithms */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    MV_HWS_AUTO_TUNE_STATUS_RES tuneRes; /*status parmeter needed for some portTuningMode*/

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap),
                                PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap),
                                &hwsPortMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

    if(CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALL_E == serdesOptAlgBmp)
    {
        optAlgoMask = (CPSS_PORT_SERDES_TRAINING_OPTIMISATION_DFE_E
                              | CPSS_PORT_SERDES_TRAINING_OPTIMISATION_FFE_E
                              | CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALIGN90_E);
    }
    else
    {
        optAlgoMask = serdesOptAlgBmp;
    }

    switch(portTuningMode)
    {
        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, TRxTuneCfg, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                TRxTuneCfg, optAlgoMask, &tuneRes);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)" ,devNum, portGroup, portMacMap, hwsPortMode, TRxTuneStart, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                TRxTuneStart, optAlgoMask, &tuneRes);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, TRxTuneStatus, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                TRxTuneStatus, optAlgoMask, &tuneRes);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, RxTrainingOnly, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                            RxTrainingOnly, optAlgoMask, &tuneRes);
        break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_TRAINING_STOP_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneStop(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], stopRx[%d], stopTx[%d])", devNum, portGroup, portMacMap, hwsPortMode, GT_TRUE, GT_TRUE);
            rc = mvHwsPortAutoTuneStop(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                       GT_TRUE, GT_TRUE);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STOP_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneStop(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], stopRx[%d], stopTx[%d])", devNum, portGroup, portMacMap, hwsPortMode, GT_FALSE, GT_TRUE);
            rc = mvHwsPortAutoTuneStop(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                       GT_FALSE, GT_TRUE);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_STOP_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneStop(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], stopRx[%d], stopTx[%d])", devNum, portGroup, portMacMap, hwsPortMode, GT_TRUE, GT_FALSE);
            rc = mvHwsPortAutoTuneStop(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                       GT_TRUE, GT_FALSE);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E:
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, RxTrainingAdative, optAlgoMask);
                rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                                RxTrainingAdative, optAlgoMask, &tuneRes);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E:
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, RxTrainingStopAdative, optAlgoMask);
                rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                                RxTrainingStopAdative, optAlgoMask, &tuneRes);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_VSR_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, RxTrainingVsr, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                            RxTrainingVsr, optAlgoMask, &tuneRes);
            break;

        case CPSS_PORT_SERDES_AUTO_TUNE_MODE_FIXED_CTLE_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneSetExt(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portTuningMode[%d], optAlgoMask[%d], *results)", devNum, portGroup, portMacMap, hwsPortMode, RxTrainingVsr, optAlgoMask);
            rc = mvHwsPortAutoTuneSetExt(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode,
                                                            FixedCtleTraining, optAlgoMask, &tuneRes);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
        if(rc != GT_OK)
        {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
    return rc;
}

/**
* @internal cpssPxPortSerdesAutoTuneExt function
* @endinternal
*
* @brief   Run auto tune algorithm on given port's serdes including required
*         optimizations.
*         Set the port Tx and Rx parameters according to different working
*         modes/topologies.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portTuningMode           - port tuning mode
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success, for TX_TRAINING_STATUS means training
*                                       succeeded
* @retval GT_BAD_PARAM             - on wrong port number, device, serdesOptAlgBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error, for TX_TRAINING_STATUS means training
*                                       failed
*
* @note See cpssPxPortSerdesAutoTune.
*
*/
GT_STATUS cpssPxPortSerdesAutoTuneExt
(
    IN  GT_SW_DEV_NUM                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT   portTuningMode,
    IN  GT_U32                                   serdesOptAlgBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesAutoTuneExt);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portTuningMode, serdesOptAlgBmp));

    rc = internal_cpssPxPortSerdesAutoTuneExt(devNum, portNum, portTuningMode, serdesOptAlgBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portTuningMode, serdesOptAlgBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX data, TX data or both
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - number of port group (local core), not used for non-multi-core
* @param[in] serdesNum                - serdes number
* @param[in] numOfBits                - number of bits to inject to serdes data
* @param[in] direction                - whether  is TX, RX or both
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_INIT_ERROR            - on Hws initialization failre
*
* @note Debug function.
*
*/
static GT_STATUS internal_cpssPxPortSerdesErrorInject
(
    IN GT_U8    devNum,
    IN GT_U32   portGroup,
    IN GT_U32   serdesNum,
    IN GT_U32   numOfBits,
    IN CPSS_PORT_DIRECTION_ENT  direction
)
{
    GT_STATUS   rc;         /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    switch (direction)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            rc = mvHwsAvagoSerdesErrorInject(devNum, portGroup, serdesNum, numOfBits, RX_DIRECTION);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Hws return code is %d", rc);
            }
            break;
        case CPSS_PORT_DIRECTION_TX_E:
            rc = mvHwsAvagoSerdesErrorInject(devNum, portGroup, serdesNum, numOfBits, TX_DIRECTION);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Hws return code is %d", rc);
            }
            break;
        case CPSS_PORT_DIRECTION_BOTH_E:
            rc = mvHwsAvagoSerdesErrorInject(devNum, portGroup, serdesNum, numOfBits, RX_DIRECTION);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Hws return code is %d", rc);
            }
            rc = mvHwsAvagoSerdesErrorInject(devNum, portGroup, serdesNum, numOfBits, TX_DIRECTION);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Hws return code is %d", rc);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong direction=%d", direction);
    }

    return rc;
}

/**
* @internal cpssPxPortSerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX data, TX data or both
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - number of port group (local core), not used for non-multi-core
* @param[in] serdesNum                - serdes number
* @param[in] numOfBits                - number of bits to inject to serdes data
* @param[in] direction                - whether  is TX, RX or both
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_INIT_ERROR            - on Hws initialization failre
*
* @note Debug function.
*
*/
GT_STATUS cpssPxPortSerdesErrorInject
(
    IN GT_U8    devNum,
    IN GT_U32   portGroup,
    IN GT_U32   serdesNum,
    IN GT_U32   numOfBits,
    IN CPSS_PORT_DIRECTION_ENT  direction
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesErrorInject);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroup, serdesNum, numOfBits, direction));

    rc = internal_cpssPxPortSerdesErrorInject(devNum, portGroup, serdesNum, numOfBits, direction);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroup, serdesNum, numOfBits, direction));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesAutoTuneStatusGet function
* @endinternal
*
* @brief   Get current status of RX and TX serdes auto-tuning on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] rxTuneStatusPtr          - RX tuning status
* @param[out] txTuneStatusPtr          - TX tuning status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssPxPortSerdesAutoTuneStatusGet
(
    IN  GT_SW_DEV_NUM                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *rxTuneStatusPtr,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *txTuneStatusPtr
)
{
    GT_STATUS               rc;         /* return code */
    MV_HWS_PORT_STANDARD    hwsPortMode;/* current ifMode of port in HWS format */
    GT_U32                  portGroupId;/* core number  */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    CPSS_NULL_PTR_CHECK_MAC(rxTuneStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(txTuneStatusPtr);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap),
                                PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap),
                                &hwsPortMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortAutoTuneStateCheck(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], *rxTune, *txTune)", devNum, portGroupId, portMacMap, hwsPortMode);
    rc = mvHwsPortAutoTuneStateCheck(CAST_SW_DEVNUM(devNum), portGroupId, portMacMap, hwsPortMode,
                                    (MV_HWS_AUTO_TUNE_STATUS*)rxTuneStatusPtr,
                                    (MV_HWS_AUTO_TUNE_STATUS*)txTuneStatusPtr);

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal cpssPxPortSerdesAutoTuneStatusGet function
* @endinternal
*
* @brief   Get current status of RX and TX serdes auto-tuning on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] rxTuneStatusPtr          - RX tuning status
* @param[out] txTuneStatusPtr          - TX tuning status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxPortSerdesAutoTuneStatusGet
(
    IN  GT_SW_DEV_NUM                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *rxTuneStatusPtr,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *txTuneStatusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesAutoTuneStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, rxTuneStatusPtr, txTuneStatusPtr));

    rc = internal_cpssPxPortSerdesAutoTuneStatusGet(devNum, portNum, rxTuneStatusPtr, txTuneStatusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, rxTuneStatusPtr, txTuneStatusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesLanePolarityGet function
* @endinternal
*
* @brief   Get the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane
*
* @param[out] invertTx                 - GT_TRUE  - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[out] invertRx                 - GT_TRUE  - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get the Tx/Rx polarity parameters for lane from HW if serdes initialized
*       or from SW DB if serdes was not initialized
*
*/
static GT_STATUS internal_cpssPxPortSerdesLanePolarityGet
(
    IN   GT_SW_DEV_NUM      devNum,
    IN   GT_U32             portGroupNum,
    IN   GT_U32             laneNum,
    OUT  GT_BOOL            *invertTx,
    OUT  GT_BOOL            *invertRx
)
{
    GT_STATUS   rc;
    GT_U32      lanesNumInDev;      /* number of serdes lanes in current device */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(invertTx);
    CPSS_NULL_PTR_CHECK_MAC(invertRx);

    if(0 == (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp & (1<<(portGroupNum))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    lanesNumInDev = prvCpssPxHwInitNumOfSerdesGet(devNum);

    if(laneNum >= lanesNumInDev)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    rc = mvHwsSerdesPolarityConfigGet(CAST_SW_DEVNUM(devNum), portGroupNum, laneNum, HWS_DEV_SERDES_TYPE(devNum, laneNum), invertTx, invertRx);
    if((rc == GT_NOT_INITIALIZED) && (PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr != NULL))
    {
        *invertTx = PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr[laneNum].txPolarity;
        *invertRx = PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr[laneNum].rxPolarity;
    }
    else if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "internal_cpssPxPortSerdesLanePolarityGet: error in mvHwsSerdesPolarityConfig, laneNum = %d\n", laneNum);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortSerdesLanePolarityGet function
* @endinternal
*
* @brief   Get the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane
*
* @param[out] invertTx                 - GT_TRUE  - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[out] invertRx                 - GT_TRUE  - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get the Tx/Rx polarity parameters for lane from HW if serdes initialized
*       or from SW DB if serdes was not initialized
*
*/
GT_STATUS cpssPxPortSerdesLanePolarityGet
(
    IN   GT_SW_DEV_NUM  devNum,
    IN   GT_U32         portGroupNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL        *invertTx,
    OUT  GT_BOOL        *invertRx
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesLanePolarityGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupNum, laneNum, invertTx, invertRx));

    rc = internal_cpssPxPortSerdesLanePolarityGet(devNum, portGroupNum, laneNum, invertTx, invertRx);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupNum, laneNum, invertTx, invertRx));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesLanePolaritySet function
* @endinternal
*
* @brief   Set the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane
* @param[in] invertTx                 - GT_TRUE  - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[in] invertRx                 - GT_TRUE  - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - If serdes initialized: Set the Tx/Rx polarity parameters for lane
*       in both HW and SW DB
*       - If serdes was not initialized: Set the Tx/Rx polarity parameters
*       for lane only in SW DB
*
*/
static GT_STATUS internal_cpssPxPortSerdesLanePolaritySet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              portGroupNum,
    IN  GT_U32              laneNum,
    IN  GT_BOOL             invertTx,
    IN  GT_BOOL             invertRx
)
{
    GT_STATUS   rc;
    GT_U32      memSize;
    GT_U32      lanesNumInDev;      /* number of serdes lanes in current device */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if(0 == (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp & (1<<(portGroupNum))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    lanesNumInDev = prvCpssPxHwInitNumOfSerdesGet(devNum);

    if ((laneNum >= lanesNumInDev) || (invertTx > 1) || (invertRx > 1))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* if SW DB was not allocated yet, allocate it here */
    if (PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr == NULL)
    {
        memSize = sizeof(PRV_CPSS_PX_PORT_SERDES_POLARITY_CONFIG_STC) * lanesNumInDev;

        PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr = (PRV_CPSS_PX_PORT_SERDES_POLARITY_CONFIG_STC *)cpssOsMalloc(memSize);

        if (PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        /* Zero the Tx/Rx polarity SW DB for all lanes */
        cpssOsMemSet(PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr, 0, memSize);
    }

    /* per Serdes number: save the Tx/Rx polarity values in SW DB */
    PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr[laneNum].txPolarity = invertTx;
    PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr[laneNum].rxPolarity = invertRx;

    SERDES_DBG_DUMP_MAC(("invertTx=%d, invertRx=%d\n",
                         PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr[laneNum].txPolarity,
                         PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr[laneNum].rxPolarity));

    rc = mvHwsSerdesPolarityConfig(CAST_SW_DEVNUM(devNum), portGroupNum, laneNum, HWS_DEV_SERDES_TYPE(devNum, laneNum), invertTx, invertRx);
    if ((rc != GT_OK) && (rc != GT_NOT_INITIALIZED))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "internal_cpssPxPortSerdesLanePolaritySet: error in mvHwsSerdesPolarityConfig, laneNum = %d\n", laneNum);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortSerdesLanePolaritySet function
* @endinternal
*
* @brief   Set the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane
* @param[in] invertTx                 - GT_TRUE  - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[in] invertRx                 - GT_TRUE  - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - If serdes initialized: Set the Tx/Rx polarity parameters for lane
*       in both HW and SW DB
*       - If serdes was not initialized: Set the Tx/Rx polarity parameters
*       for lane only in SW DB
*
*/
GT_STATUS cpssPxPortSerdesLanePolaritySet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          portGroupNum,
    IN  GT_U32          laneNum,
    IN  GT_BOOL         invertTx,
    IN  GT_BOOL         invertRx
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesLanePolaritySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupNum, laneNum, invertTx, invertRx));

    rc = internal_cpssPxPortSerdesLanePolaritySet(devNum, portGroupNum, laneNum, invertTx, invertRx);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupNum, laneNum, invertTx, invertRx));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesPolaritySet function
* @endinternal
*
* @brief   Invert the Tx or Rx serdes polarity.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) to define
*                                      polarity on.
* @param[in] invertTx                 -  GT_TRUE - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[in] invertRx                 -  GT_TRUE - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note This API should be egaged by application after serdes power up. Important to
*       wrap both steps i.e. "serdes power up" and "serdes polarity set" by port
*       disable and link_change interrupt lock and restore port enable and
*       reenable link change interrupt only after "serdes polarity set" to
*       prevent interrupt toggling during the process.
*
*/
static GT_STATUS internal_cpssPxPortSerdesPolaritySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               laneBmp,
    IN GT_BOOL              invertTx,
    IN GT_BOOL              invertRx
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      portGroupId;    /* port group */
    GT_U32      startSerdes;    /* first serdes of port */
    GT_U32      numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32      i;              /* iterator */
    GT_U32      lanesNumInDev;  /* number of serdes in device or single core */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                            CAST_SW_DEVNUM(devNum), portMacMap);

    rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum,
                            PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap),
                            &startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK)
        return rc;

    lanesNumInDev = prvCpssPxHwInitNumOfSerdesGet(devNum);

    if ((0 == numOfSerdesLanes) || ((startSerdes + numOfSerdesLanes) > lanesNumInDev))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    for (i = 0; (i < numOfSerdesLanes) && ((startSerdes + i) < lanesNumInDev); i++)
    {
        if(laneBmp & (1<<i))
        {
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesPolarityConfig(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], invertTx[%d], invertRx[%d])", devNum, portGroupId, startSerdes+i, HWS_DEV_SERDES_TYPE(devNum, startSerdes+i), invertTx, invertRx);
            rc = mvHwsSerdesPolarityConfig(CAST_SW_DEVNUM(devNum), portGroupId, startSerdes+i,
                                           HWS_DEV_SERDES_TYPE(devNum, startSerdes+i),
                                           invertTx, invertRx);

            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }

       }

    }

    return GT_OK;
}

/**
* @internal cpssPxPortSerdesPolaritySet function
* @endinternal
*
* @brief   Invert the Tx or Rx serdes polarity.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) to define
*                                      polarity on.
* @param[in] invertTx                 -  GT_TRUE - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[in] invertRx                 -  GT_TRUE - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note This API should be egaged by application after serdes power up. Important to
*       wrap both steps i.e. "serdes power up" and "serdes polarity set" by port
*       disable and link_change interrupt lock and restore port enable and
*       reenable link change interrupt only after "serdes polarity set" to
*       prevent interrupt toggling during the process.
*
*/
GT_STATUS cpssPxPortSerdesPolaritySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               laneBmp,
    IN GT_BOOL              invertTx,
    IN GT_BOOL              invertRx
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesPolaritySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneBmp, invertTx, invertRx));

    rc = internal_cpssPxPortSerdesPolaritySet(devNum, portNum, laneBmp, invertTx, invertRx);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneBmp, invertTx, invertRx));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesPolarityGet function
* @endinternal
*
* @brief   Get status of the Tx or Rx serdes polarity invert.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.) to
*                                      define polarity on.
*
* @param[out] invertTxPtr              - (ptr to) GT_TRUE - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[out] invertRxPtr              - (ptr to) GT_TRUE - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssPxPortSerdesPolarityGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32               laneNum,
    OUT GT_BOOL              *invertTxPtr,
    OUT GT_BOOL              *invertRxPtr
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      portGroupId;    /* port group */
    GT_U32      startSerdes;    /* first serdes of port */
    GT_U32      numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(invertTxPtr);
    CPSS_NULL_PTR_CHECK_MAC(invertRxPtr);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum,
                                         PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap),
                                         &startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK)
        return rc;

    if(laneNum >= numOfSerdesLanes)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);


    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                            CAST_SW_DEVNUM(devNum), portMacMap);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesPolarityConfigGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *invertTx, *invertRx)", devNum, portGroupId, startSerdes+laneNum, HWS_DEV_SERDES_TYPE(devNum, startSerdes+laneNum));
    rc = mvHwsSerdesPolarityConfigGet(CAST_SW_DEVNUM(devNum), portGroupId, startSerdes+laneNum,
                                      HWS_DEV_SERDES_TYPE(devNum, startSerdes+laneNum), invertTxPtr,
                                      invertRxPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }


    return GT_OK;
}

/**
* @internal cpssPxPortSerdesPolarityGet function
* @endinternal
*
* @brief   Get status of the Tx or Rx serdes polarity invert.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.) to
*                                      define polarity on.
*
* @param[out] invertTxPtr              - (ptr to) GT_TRUE - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[out] invertRxPtr              - (ptr to) GT_TRUE - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssPxPortSerdesPolarityGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32               laneNum,
    OUT GT_BOOL              *invertTxPtr,
    OUT GT_BOOL              *invertRxPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesPolarityGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, invertTxPtr, invertRxPtr));

    rc = internal_cpssPxPortSerdesPolarityGet(devNum, portNum, laneNum, invertTxPtr, invertRxPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, invertTxPtr, invertRxPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortSerdesLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on SerDes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum  - device number
* @param[in] portNum - physical port number
* @param[in] mode    - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note  For Serdes Loopback modes:
*        - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
*          configurations should be disabled, for getting LinkUp
*          and Traffic on port.
*        - After return to 'Normal' mode, the Rx & Tx Serdes
*          Polarity configuration should be restored, for
*          re-synch again the PCS and MAC units and getting
*          LinkUp and Traffic on port.
*        - Switching between 'AN_TX_RX'/'DIG_RX_TX' modes
*          requires returning to the 'Normal' mode before.
*
*/
GT_STATUS prvCpssPxPortSerdesLoopbackModeSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT mode
)
{
    GT_STATUS               rc;          /* return code */
    GT_U32                  portGroupId; /* core number of port */
    MV_HWS_PORT_STANDARD    portMode;    /* port interface in HWS format */
    MV_HWS_PORT_LB_TYPE     lbType;      /* loopback type in HWS format */
    GT_U32                  portMacMap;  /* number of mac mapped to this physical port */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode; /* current interface of port */
    CPSS_PORT_SPEED_ENT         speed;   /* current speed of port */
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U8                       sdVecSize = 0;
    GT_U16                      *sdVectorPtr = NULL;

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap);
    speed =  PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap);
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
        return rc;

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portMacMap, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "hwsPortModeParamsGet returned null ");
    }

    sdVecSize   = curPortParams.numOfActLanes;
    sdVectorPtr = curPortParams.activeLanesList;

    switch(mode)
    {
        case CPSS_PX_PORT_SERDES_LOOPBACK_DISABLE_E:
            lbType = DISABLE_LB;
            rc = prvCpssPxPortSerdesPolaritySet(devNum, 0, sdVectorPtr, sdVecSize);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssPxPortSerdesPolaritySet, portNum = %d\n", portMacMap);
            }
            break;
        case CPSS_PX_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E:
            lbType = TX_2_RX_LB;
            break;
        case CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E:
            lbType = TX_2_RX_DIGITAL_LB;
            break;
        case CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E:
            lbType = RX_2_TX_LB;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackSet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], lbType[%d])",
                             devNum, portGroupId, portMacMap, portMode, HWS_PMA, lbType);

    rc = mvHwsPortLoopbackSet(CAST_SW_DEVNUM(devNum), portGroupId,
                              (portMode != QSGMII) ? portMacMap : (portMacMap & 0xFFFFFFFC),
                              portMode, HWS_PMA, lbType);
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    return GT_OK;
}


/**
* @internal internal_cpssPxPortSerdesLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on SerDes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum  - device number
* @param[in] portNum - physical port number
* @param[in] mode    - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note  For Serdes Loopback modes:
*        - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
*          configurations should be disabled, for getting LinkUp
*          and Traffic on port.
*        - After return to 'Normal' mode, the Rx & Tx Serdes
*          Polarity configuration should be restored, for
*          re-synch again the PCS and MAC units and getting
*          LinkUp and Traffic on port.
*        - Switching between 'AN_TX_RX'/'DIG_RX_TX' modes
*          requires returning to the 'Normal' mode before.
*
*/
static GT_STATUS internal_cpssPxPortSerdesLoopbackModeSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT   mode
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      startSerdes;    /* first serdes of port */
    GT_U32      numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(mode >= CPSS_PX_PORT_SERDES_LOOPBACK_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum,
                            PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap),
                            &startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK)
        return rc;

    if(0 == numOfSerdesLanes)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    {
        return prvCpssPxPortSerdesLoopbackModeSet(devNum,portNum,mode);
    }

}

/**
* @internal cpssPxPortSerdesLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on SerDes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum  - device number
* @param[in] portNum - physical port number
* @param[in] mode    - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
*
* @note  For Serdes Loopback modes:
*        - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
*          configurations should be disabled, for getting LinkUp
*          and Traffic on port.
*        - After return to 'Normal' mode, the Rx & Tx Serdes
*          Polarity configuration should be restored, for
*          re-synch again the PCS and MAC units and getting
*          LinkUp and Traffic on port.
*        - Switching between 'AN_TX_RX'/'DIG_RX_TX' modes
*          requires returning to the 'Normal' mode before.
*
*/
GT_STATUS cpssPxPortSerdesLoopbackModeSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT   mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesLoopbackModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mode));

    rc = internal_cpssPxPortSerdesLoopbackModeSet(devNum, portNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesLoopbackModeGet function
* @endinternal
*
* @brief   Get current mode of loopback on SerDes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*                                      to read loopback status
*
* @param[out] modePtr                  - current loopback mode or none
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_SUPPORTED         - on not expected mode value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssPxPortSerdesLoopbackModeGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  laneNum,
    OUT CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT *modePtr
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      portGroupId;    /* port group */
    GT_U32      startSerdes;    /* first serdes of port */
    GT_U32      numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */
    MV_HWS_SERDES_LB  lbMode;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum,
                            PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap),
                            &startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK)
        return rc;

    if(laneNum >= numOfSerdesLanes)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                            CAST_SW_DEVNUM(devNum), portMacMap);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesLoopbackGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *lbModePtr)", devNum, portGroupId, startSerdes+laneNum, HWS_DEV_SERDES_TYPE(devNum, startSerdes+laneNum));
    rc = mvHwsSerdesLoopbackGet(CAST_SW_DEVNUM(devNum), portGroupId, startSerdes+laneNum,
                                    HWS_DEV_SERDES_TYPE(devNum, startSerdes+laneNum), &lbMode);

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    switch(lbMode)
    {
        case SERDES_LP_DISABLE:
            *modePtr = CPSS_PX_PORT_SERDES_LOOPBACK_DISABLE_E;
            break;
        case SERDES_LP_AN_TX_RX:
            *modePtr = CPSS_PX_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E;
            break;
        case SERDES_LP_DIG_TX_RX:
            *modePtr = CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E;
            break;
        case SERDES_LP_DIG_RX_TX:
            *modePtr = CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortSerdesLoopbackModeGet function
* @endinternal
*
* @brief   Get current mode of loopback on SerDes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*                                      to read loopback status
*
* @param[out] modePtr                  - current loopback mode or none
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - NULL pointer
* @retval GT_NOT_SUPPORTED         - on not expected mode value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssPxPortSerdesLoopbackModeGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  laneNum,
    OUT CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT   *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesLoopbackModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, modePtr));

    rc = internal_cpssPxPortSerdesLoopbackModeGet(devNum, portNum, laneNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssPxPortEomDfeResGet function
* @endinternal
*
* @brief   Returns the current DFE parameters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
*
* @param[out] dfeResPtr                - current DFE V in millivolts
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*
* @note for sip 5 dfeResPtr size must be CPSS_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS
*
*/
static GT_STATUS internal_cpssPxPortEomDfeResGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  serdesNum,
    OUT GT_U32                 *dfeResPtr
)
{
    GT_STATUS rc;
    GT_U32 portGroupId;    /* port group */
    GT_U32 startSerdes;
    GT_U32 numOfSerdesLanes;
    GT_U32 globalSerdesNum;
    GT_U32 portMacMap;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT results;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(dfeResPtr);

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    if(portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMapShadowPtr->portMap.macNum);


    if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum, ifMode,
                                           &startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK) return rc;

    globalSerdesNum = startSerdes + serdesNum;

    if(serdesNum >= numOfSerdesLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsAvagoSerdesAutoTuneResult(devNum[%d], portGroup[%d], serdesNum[%d])", devNum, portGroupId, globalSerdesNum);
        rc = mvHwsAvagoSerdesAutoTuneResult(CAST_SW_DEVNUM(devNum), portGroupId, globalSerdesNum, &results);
        if(rc == GT_OK)
        {
            cpssOsMemCpy(dfeResPtr, results.avagoResults.DFE, sizeof(GT_U32) * CPSS_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS);
        }

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }
    return rc;
}

/**
* @internal cpssPxPortEomDfeResGet function
* @endinternal
*
* @brief   Returns the current DFE parameters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
*
* @param[out] dfeResPtr                - current DFE V in millivolts
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*/
GT_STATUS cpssPxPortEomDfeResGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  serdesNum,
    OUT GT_U32                 *dfeResPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortEomDfeResGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesNum, dfeResPtr));

    rc = internal_cpssPxPortEomDfeResGet(devNum, portNum, serdesNum, dfeResPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesNum, dfeResPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


#define DBG_PRINT_MATRIX  0

/**
* @internal internal_cpssPxPortSerdesEyeMatrixGet function
* @endinternal
*
* @brief   Returns the eye mapping matrix for SERDES.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - SERDES number
* @param[in] eye_inputPtr             - pointer to the serdes Eye monitoring input parameters structure
*
* @param[out] eye_resultsPtr           - pointer to the serdes Eye monitoring results structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - if not supported
* @retval else                     - on error
*
* @note The function allocated memory and gets the SERDES EOM matrix, vbtc and
*       hbtc calculation in text format.
*       After using this function application needs to free allocated memory, for example:
*       cpssOsFree(eye_resultsPtr->matrixPtr);
*       cpssOsFree(eye_resultsPtr->vbtcPtr);
*       cpssOsFree(eye_resultsPtr->hbtcPtr);
*
*/
static GT_STATUS internal_cpssPxPortSerdesEyeMatrixGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  GT_U32                                serdesNum,
    IN CPSS_PX_PORT_SERDES_EYE_INPUT_STC   *eye_inputPtr,
    OUT CPSS_PX_PORT_SERDES_EYE_RESULT_STC *eye_resultsPtr
)
{
    GT_STATUS rc;
    GT_U32    startSerdes;
    GT_U32    numOfSerdesLanes;
    GT_U32    portMacNum;
    GT_U32    dwell_bits_low_limit = 100000;
    GT_U32    dwell_bits_up_limit = 100000000;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(eye_inputPtr);
    CPSS_NULL_PTR_CHECK_MAC(eye_resultsPtr);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if((eye_inputPtr->min_dwell_bits > eye_inputPtr->max_dwell_bits)
        || (eye_inputPtr->min_dwell_bits < dwell_bits_low_limit)
        || (eye_inputPtr->max_dwell_bits > dwell_bits_up_limit))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

   /* Convert local serdes number to global*/
    rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum,
                            PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacNum),
                            /*OUT*/&startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK)
    {
        return rc;
    }
    if(serdesNum >= numOfSerdesLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
#if !defined(ASIC_SIMULATION) && !defined(CPSS_BLOB)
{
    MV_HWS_AVAGO_SERDES_EYE_GET_INPUT  eye_input;
    MV_HWS_AVAGO_SERDES_EYE_GET_RESULT eye_results;

    eye_resultsPtr->globalSerdesNum = startSerdes + serdesNum;
    eye_input.min_dwell_bits = eye_inputPtr->min_dwell_bits;
    eye_input.max_dwell_bits = eye_inputPtr->max_dwell_bits;

    rc = mvHwsAvagoSerdesEyeGet(CAST_SW_DEVNUM(devNum), eye_resultsPtr->globalSerdesNum, &eye_input, &eye_results);
    if(rc == GT_OK)
    {
        eye_resultsPtr->matrixPtr = eye_results.matrixPtr;
        eye_resultsPtr->x_points = eye_results.x_points;
        eye_resultsPtr->y_points = eye_results.y_points;
        eye_resultsPtr->vbtcPtr = eye_results.vbtcPtr;
        eye_resultsPtr->hbtcPtr = eye_results.hbtcPtr;
        eye_resultsPtr->height_mv = eye_results.height_mv;
        eye_resultsPtr->width_mui = eye_results.width_mui;
    }
}
#else
    rc = GT_NOT_SUPPORTED;
#endif /*ASIC_SIMULATION*/
    return rc;
}

/**
* @internal cpssPxPortSerdesEyeMatrixGet function
* @endinternal
*
* @brief   Returns the eye mapping matrix for SERDES.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - SERDES number
* @param[in] eye_inputPtr             - pointer to the serdes Eye monitoring input parameters structure
*
* @param[out] eye_resultsPtr           - pointer to the serdes Eye monitoring results structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_SUPPORTED         - if not supported
* @retval else                     - on error
*
* @note The function allocated memory and gets the SERDES EOM matrix, vbtc and
*       hbtc calculation in text format.
*       After using this function application needs to free allocated memory, for example:
*       cpssOsFree(eye_resultsPtr->matrixPtr);
*       cpssOsFree(eye_resultsPtr->vbtcPtr);
*       cpssOsFree(eye_resultsPtr->hbtcPtr);
*
*/
GT_STATUS cpssPxPortSerdesEyeMatrixGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  GT_U32                                serdesNum,
    IN  CPSS_PX_PORT_SERDES_EYE_INPUT_STC  *eye_inputPtr,
    OUT CPSS_PX_PORT_SERDES_EYE_RESULT_STC *eye_resultsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesEyeMatrixGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesNum, eye_inputPtr, eye_resultsPtr));

    rc = internal_cpssPxPortSerdesEyeMatrixGet(devNum, portNum, serdesNum, eye_inputPtr, eye_resultsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesNum, eye_inputPtr, eye_resultsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortEomBaudRateGet function
* @endinternal
*
* @brief   Returns the current system baud rate.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
*
* @param[out] baudRatePtr              - current system baud rate in pico-seconds.
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*/
static GT_STATUS internal_cpssPxPortEomBaudRateGet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  GT_U32                    serdesNum,
    OUT GT_U32                   *baudRatePtr
)
{
    GT_U32 portGroupId;
    GT_U32 startSerdes;
    GT_U32 numOfSerdesLanes;
    GT_U32 globalSerdesNum;
    GT_U32 portMacMap;
    GT_U32 rc;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(baudRatePtr);

    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    if(portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);
    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMapShadowPtr->portMap.macNum);

    if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum, ifMode,
                                           &startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK)
        return rc;


    globalSerdesNum = startSerdes + serdesNum;

    if(serdesNum >= numOfSerdesLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsAvagoSerdeEomUiGet(devNum[%d], portGroup[%d], serdesNum[%d])", devNum, portGroupId, globalSerdesNum);
    rc = mvHwsAvagoSerdesEomUiGet(CAST_SW_DEVNUM(devNum), portGroupId, globalSerdesNum, baudRatePtr);

    if(rc != GT_OK){
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }
    return rc;
}

/**
* @internal cpssPxPortEomBaudRateGet function
* @endinternal
*
* @brief   Returns the current system baud rate.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port  number
* @param[in] serdesNum                - local serdes number
*
* @param[out] baudRatePtr              - current system baud rate in pico-seconds.
*
* @retval GT_OK                    - on success
* @retval else                     - on error
*/
GT_STATUS cpssPxPortEomBaudRateGet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  GT_U32                    serdesNum,
    OUT GT_U32                   *baudRatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortEomBaudRateGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesNum, baudRatePtr));

    rc = internal_cpssPxPortEomBaudRateGet(devNum, portNum, serdesNum, baudRatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesNum, baudRatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssPxPortSerdesTxEnableSet function
* @endinternal
*
* @brief   Enable / Disable transmission of packets in SERDES layer of a port.
*         Use this API to disable Tx for loopback ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port
*                                      - GT_FALSE - Disable transmission of packets in
*                                      SERDES layer of a port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Disabling transmission of packets in SERDES layer of a port causes
*       to link down of devices that are connected to the port.
*
*/
static GT_STATUS internal_cpssPxPortSerdesTxEnableSet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL enable
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      portGroupId;     /*the port group Id - support multi-port-groups device */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode; /* current interface of port */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */
    MV_HWS_PORT_STANDARD    portMode;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    /*need cpssPxPortInterfaceModeGet API*/
    rc = cpssPxPortInterfaceModeGet(devNum,portNum,&ifMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode,
                                PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap),
                                &portMode);
    if(rc != GT_OK)
        return rc;


    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortTxEnable(devNum[%d], portGroup[%d], portMacMap[%d], portMode[%d], enable[%d])",devNum, portGroupId, portMacMap, portMode, enable);
    rc = mvHwsPortTxEnable(CAST_SW_DEVNUM(devNum), portGroupId, portMacMap, portMode, enable);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortSerdesTxEnableSet function
* @endinternal
*
* @brief   Enable / Disable transmission of packets in SERDES layer of a port.
*         Use this API to disable Tx for loopback ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port
*                                      - GT_FALSE - Disable transmission of packets in
*                                      SERDES layer of a port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Disabling transmission of packets in SERDES layer of a port causes
*       to link down of devices that are connected to the port.
*
*/
GT_STATUS cpssPxPortSerdesTxEnableSet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesTxEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortSerdesTxEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortSerdesTxEnableGet function
* @endinternal
*
* @brief   Get Enable / Disable transmission of packets in SERDES layer of a port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - Pointer to transmission of packets in SERDES
*                                      layer of a port.
*                                      - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port.
*                                      - GT_FALSE - Enable transmission of packets in
*                                      SERDES layer of a port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortSerdesTxEnableGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      startSerdes = 0, serdesesNum = 0;
    GT_U32      portGroupId;     /*the port group Id - support multi-port-groups device */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode; /* current port interface mode */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacMap) == PRV_CPSS_PORT_FE_E)
    {
        /* no serdes support for FE ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /*need cpssPxPortInterfaceModeGet API*/
    rc = cpssPxPortInterfaceModeGet(devNum,portNum,&ifMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortIfModeCheckAndSerdesNumGet(devNum,portNum,ifMode,
                                                   &startSerdes,&serdesesNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);
    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesTxEnableGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *enablePtr)", devNum, portGroupId, startSerdes,HWS_DEV_SERDES_TYPE(devNum, startSerdes));
    rc = mvHwsSerdesTxEnableGet(CAST_SW_DEVNUM(devNum), portGroupId, startSerdes,HWS_DEV_SERDES_TYPE(devNum, startSerdes), enablePtr);

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortSerdesTxEnableGet function
* @endinternal
*
* @brief   Get Enable / Disable transmission of packets in SERDES layer of a port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - Pointer to transmission of packets in SERDES
*                                      layer of a port.
*                                      - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port.
*                                      - GT_FALSE - Enable transmission of packets in
*                                      SERDES layer of a port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortSerdesTxEnableGet
(
    IN GT_SW_DEV_NUM   devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesTxEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortSerdesTxEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxSerdesRefClockTranslateCpss2Hws function
* @endinternal
*
* @brief   Get serdes referense clock from CPSS DB and translate it to HWS format
*
* @param[in] devNum                   - device number
*
* @param[out] refClockPtr              - (ptr to) serdes referense clock in HWS format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - if value from CPSS DB not supported
*/
GT_STATUS prvCpssPxSerdesRefClockTranslateCpss2Hws
(
    IN  GT_SW_DEV_NUM devNum,
    OUT MV_HWS_REF_CLOCK_SUP_VAL *refClockPtr
)
{
    switch(PRV_CPSS_PX_PP_MAC(devNum)->port.serdesRefClock)
    {
        case CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E:
            *refClockPtr = MHz_25;
            break;

        case CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_125_SINGLE_ENDED_E:
        case CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_125_DIFF_E:
        case CPSS_PX_PP_SERDES_REF_CLOCK_INTERNAL_125_E:
            *refClockPtr = MHz_125;
            break;

        case CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_SINGLE_ENDED_E:
        case CPSS_PX_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E:
            *refClockPtr = MHz_156;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal prvCpssPxPortSerdesManualConfig function
* @endinternal
*
* @brief   Service function for cpssPxPortSerdesManualConfig API's - make common tasks:
*         check parameters and obtain common data
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesCfgPtr             - serdes configuration parameters
*
* @param[out] portGroupPtr             - core number of port
* @param[out] startSerdesPtr           - number of first serdes used by port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS prvCpssPxPortSerdesManualConfig
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneNum,
    IN  GT_VOID                 *serdesCfgPtr,
    OUT GT_U32                  *portGroupPtr,
    OUT GT_U32                  *startSerdesPtr
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32      numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32      portMacMap; /* number of mac mapped to this physical port */
        CPSS_PORT_INTERFACE_MODE_ENT    ifMode;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(serdesCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(portGroupPtr);
    CPSS_NULL_PTR_CHECK_MAC(startSerdesPtr);

    *portGroupPtr = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

        ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap);
        if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
        {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

    rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum,
                            ifMode,
                            startSerdesPtr, &numOfSerdesLanes);
    if(rc != GT_OK)
        return rc;

    /* > and not >= to allow redundant serdes configuration */
    if(laneNum > numOfSerdesLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxPortSerdesManualTxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes TX in HW.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesTxCfgPtr           - serdes Tx parameters:
*                                      txAmp - Tx Driver output Amplitude/Attenuator: [0...31]
*                                      txAmpAdjEn - not rellevant for Pipe
*                                      emph0 - Post-Cursor: Controls the Emphasis Amplitude for Gen0 bit rates
*                                      for ports  #0-#11: [-31...31]
*                                      for ports #12-#15: [0...31]
*                                      emph1 - Pre-Cursor: Controls the emphasis amplitude for Gen1 bit rates
*                                      for ports  #0-#11: [-31...31]
*                                      for ports #12-#15: [0...31]
*                                      txAmpShft - not rellevant for Pipe
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssPxPortSerdesTuningSet.
*
*/
static GT_STATUS internal_cpssPxPortSerdesManualTxConfigSet
(
    IN GT_SW_DEV_NUM                        devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
)
{
    GT_STATUS   rc;
    GT_U32      portGroup;
    GT_U32      startSerdes;    /* first serdes of port */
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT    txParams;

    rc = prvCpssPxPortSerdesManualConfig(devNum, portNum, laneNum, serdesTxCfgPtr,
                                           &portGroup, &startSerdes);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(serdesTxCfgPtr->type != CPSS_PORT_SERDES_AVAGO_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((serdesTxCfgPtr->txTune.avago.atten & 0xFFFFFFE0) != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (portNum <= 11)
    {
        if (serdesTxCfgPtr->txTune.avago.post < -31 ||
            serdesTxCfgPtr->txTune.avago.post >  31 ||
            serdesTxCfgPtr->txTune.avago.pre < -31 ||
            serdesTxCfgPtr->txTune.avago.pre >  31)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    else
    { /* for ports #12-#15 */
        if (serdesTxCfgPtr->txTune.avago.post <  0 ||
            serdesTxCfgPtr->txTune.avago.post > 31 ||
            serdesTxCfgPtr->txTune.avago.pre <  0 ||
            serdesTxCfgPtr->txTune.avago.pre > 31)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsAvagoSerdesManualTxConfig(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d],"
                                     "TxAtten[%d], TxPost-Cursor[%d], TxPre-Cursor[%d])",
                                     devNum, portGroup, startSerdes+laneNum, HWS_DEV_SERDES_TYPE(devNum, startSerdes+laneNum),
                                     serdesTxCfgPtr->txTune.avago.atten, serdesTxCfgPtr->txTune.avago.post, serdesTxCfgPtr->txTune.avago.pre);

    txParams.txAvago.atten = serdesTxCfgPtr->txTune.avago.atten;
    txParams.txAvago.post  = serdesTxCfgPtr->txTune.avago.post;
    txParams.txAvago.pre   = serdesTxCfgPtr->txTune.avago.pre;
    txParams.txAvago.pre2  = txParams.txAvago.pre3 = 0;
    rc = mvHwsAvagoSerdesManualTxConfig(CAST_SW_DEVNUM(devNum), portGroup, startSerdes+laneNum, &txParams);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Hws return code is [%d]", rc);
    }

    return rc;
}

/**
* @internal cpssPxPortSerdesManualTxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes TX in HW.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesTxCfgPtr           - serdes Tx parameters
* @param[in] serdesTxCfgPtr           - serdes Tx parameters:
*                                      txAmp - Tx Driver output Amplitude/Attenuator: [0...31]
*                                      txAmpAdjEn - not rellevant for Pipe
*                                      emph0 - Post-Cursor: Controls the Emphasis Amplitude for Gen0 bit rates
*                                      for ports  #0-#11: [-31...31]
*                                      for ports #12-#15: [0...31]
*                                      emph1 - Pre-Cursor: Controls the emphasis amplitude for Gen1 bit rates
*                                      for ports  #0-#11: [-31...31]
*                                      for ports #12-#15: [0...31]
*                                      txAmpShft - not rellevant for Pipe
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssPxPortSerdesTuningSet.
*
*/
GT_STATUS cpssPxPortSerdesManualTxConfigSet
(
    IN GT_SW_DEV_NUM                        devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesManualTxConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, serdesTxCfgPtr));

    rc = internal_cpssPxPortSerdesManualTxConfigSet(devNum, portNum, laneNum, serdesTxCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, serdesTxCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Read specific parameters of serdes TX.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesTxCfgPtr           - serdes Tx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTxCfgPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssPxPortSerdesManualTxConfigGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup; /* local core number */
    GT_U32                  startSerdes;    /* first serdes of port */
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT    serdesTxTuneValues;  /* current serdes tuning values in HWS format */

    rc = prvCpssPxPortSerdesManualConfig(devNum, portNum, laneNum,
                                           serdesTxCfgPtr, &portGroup,
                                           &startSerdes);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clean buffer to avoid random trash in LOG */
    cpssOsMemSet(&serdesTxTuneValues, 0, sizeof(serdesTxTuneValues));

    rc = mvHwsSerdesManualTxConfigGet(CAST_SW_DEVNUM(devNum), portGroup, startSerdes+laneNum, HWS_DEV_SERDES_TYPE(devNum, startSerdes+laneNum), &serdesTxTuneValues);

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Hws return code is [%d]", rc);
    }
    serdesTxCfgPtr->type = CPSS_PORT_SERDES_AVAGO_E;
    serdesTxCfgPtr->txTune.avago.atten = serdesTxTuneValues.txAvago.atten;
    serdesTxCfgPtr->txTune.avago.post = serdesTxTuneValues.txAvago.post;
    serdesTxCfgPtr->txTune.avago.pre = serdesTxTuneValues.txAvago.pre;
    return rc;
}

/**
* @internal cpssPxPortSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Read specific parameters of serdes TX.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesTxCfgPtr           - serdes Tx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTxCfgPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssPxPortSerdesManualTxConfigGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesManualTxConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, serdesTxCfgPtr));

    rc = internal_cpssPxPortSerdesManualTxConfigGet(devNum, portNum, laneNum, serdesTxCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, serdesTxCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesManualRxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes RX in HW.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesRxCfgPtr           - serdes Rx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssPxPortSerdesTuningSet.
*
*/
static GT_STATUS internal_cpssPxPortSerdesManualRxConfigSet
(
    IN GT_SW_DEV_NUM                        devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup; /* local core number */
    GT_U32                  startSerdes;    /* first serdes of port */
    GT_U32                  portMacNum;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    configParams;
    MV_HWS_PORT_STANDARD                portMode;

    rc = prvCpssPxPortSerdesManualConfig(devNum, portNum, laneNum, serdesRxCfgPtr, &portGroup, &startSerdes);
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                    PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portMacNum),
                    PRV_CPSS_PX_PORT_SPEED_MAC(devNum,portMacNum),
                    &portMode);
        if(rc != GT_OK)
        {
            return rc;
        }

    if (serdesRxCfgPtr->type != CPSS_PORT_SERDES_AVAGO_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((serdesRxCfgPtr->rxTune.avago.DC > 255) ||
       ((serdesRxCfgPtr->rxTune.avago.LF > 15) && (serdesRxCfgPtr->rxTune.avago.LF != 0xFFFF)) ||
       ((serdesRxCfgPtr->rxTune.avago.HF > 15) && (serdesRxCfgPtr->rxTune.avago.HF != 0xFFFF)) ||
       ((serdesRxCfgPtr->rxTune.avago.BW > 15) && (serdesRxCfgPtr->rxTune.avago.BW != 0xFFFF)) ||
       (serdesRxCfgPtr->rxTune.avago.sqlch > 308))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(&configParams, 0, sizeof(configParams));

    configParams.lowFrequency  = serdesRxCfgPtr->rxTune.avago.LF;
    configParams.highFrequency = serdesRxCfgPtr->rxTune.avago.HF;
    configParams.bandWidth     = serdesRxCfgPtr->rxTune.avago.BW;
    configParams.dcGain        = serdesRxCfgPtr->rxTune.avago.DC;
    configParams.squelch       = serdesRxCfgPtr->rxTune.avago.sqlch;
    configParams.gainshape1 = configParams.gainshape2 = configParams.dfeGAIN =
            configParams.dfeGAIN2   = configParams.shortChannelEn = 0;

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortManualCtleConfig(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], laneNum[%d], configParams{dcGain[%d],"
                             "lowFrequency[%d], highFrequency[%d], bandWidth[%d], squelch[%d]})",
                             devNum, portGroup, portMacNum, portMode, (GT_U8)(laneNum),
                             configParams.dcGain,
                             configParams.lowFrequency,
                             configParams.highFrequency,
                             configParams.bandWidth,
                             configParams.squelch);

    portMacNum = (QSGMII == portMode) ? (portMacNum & 0xFFFC) : portMacNum;
    rc = mvHwsPortManualCtleConfig(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, (GT_U8)(laneNum), &configParams);
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal cpssPxPortSerdesManualRxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes RX in HW.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesRxCfgPtr           - serdes Rx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssPxPortSerdesTuningSet.
*
*/
GT_STATUS cpssPxPortSerdesManualRxConfigSet
(
    IN GT_SW_DEV_NUM                        devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesManualRxConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, serdesRxCfgPtr));

    rc = internal_cpssPxPortSerdesManualRxConfigSet(devNum, portNum, laneNum, serdesRxCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, serdesRxCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesManualRxConfigGet function
* @endinternal
*
* @brief   Read specific parameters of serdes RX.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesRxCfgPtr           - serdes Rx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTxCfgPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note The squelch transforms from units 0 - 15 to
*       units 0 - 308 by formula (x 16) + 68. So it's value can be not the same as it was set.
*       For example, was set the squelch = 110. It's transforms and safes inside as
*       integer part of (squelch - 68) / 16 = (integer part)(110 - 68) / 16 = 2
*       And we obtain after getting (2 16) + 68 = 100
*
*/
static GT_STATUS internal_cpssPxPortSerdesManualRxConfigGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup; /* local core number */
    GT_U32                  startSerdes;    /* first serdes of port */
    GT_U32                  portMacNum;
    MV_HWS_PORT_STANDARD    portMode;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    configParams;

    rc = prvCpssPxPortSerdesManualConfig(devNum, portNum, laneNum, serdesRxCfgPtr,
                                           &portGroup, &startSerdes);
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portMacNum),
                PRV_CPSS_PX_PORT_SPEED_MAC(devNum,portMacNum),
                &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = mvHwsAvagoSerdesManualCtleConfigGet(CAST_SW_DEVNUM(devNum), 0, startSerdes + laneNum, &configParams);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mvHwsAvagoSerdesManualCtleConfigGet return code is [%d]", rc);
    }
    serdesRxCfgPtr->type = CPSS_PORT_SERDES_AVAGO_E;
    serdesRxCfgPtr->rxTune.avago.BW     = configParams.bandWidth;
    serdesRxCfgPtr->rxTune.avago.DC     = configParams.dcGain;
    serdesRxCfgPtr->rxTune.avago.HF     = configParams.highFrequency;
    serdesRxCfgPtr->rxTune.avago.LF     = configParams.lowFrequency;
    serdesRxCfgPtr->rxTune.avago.sqlch  = configParams.squelch;

    return rc;
}

/**
* @internal cpssPxPortSerdesManualRxConfigGet function
* @endinternal
*
* @brief   Read specific parameters of serdes RX.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesRxCfgPtr           - serdes Rx parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTxCfgPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note The squelch transforms from units 0 - 15 to
*       units 0 - 308 by formula (x 16) + 68. So it's value can be not the same as it was set.
*       For example, was set the squelch = 110. It's transforms and safes inside as
*       integer part of (squelch - 68) / 16 = (integer part)(110 - 68) / 16 = 2
*       And we obtain after getting (2 16) + 68 = 100
*
*/
GT_STATUS cpssPxPortSerdesManualRxConfigGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              laneNum,
    OUT CPSS_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesManualRxConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, serdesRxCfgPtr));

    rc = internal_cpssPxPortSerdesManualRxConfigGet(devNum, portNum, laneNum, serdesRxCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, serdesRxCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes (true/false).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_SUPPORTED         - if interface not supported
*/
static GT_STATUS internal_cpssPxPortSerdesSignalDetectGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
)
{
    GT_STATUS                        rc;                               /* return code */
    GT_U32                           portGroupId;                      /* number of core in multi-port-group devices */
    GT_U32                           i;                                /* iterator */
    GT_U32                           portMacMap;                       /* number of mac mapped to this physical port */
    GT_U32                           portGroup;                        /* local core number */
    MV_HWS_PORT_STANDARD             hwsPortMode;                      /* current ifMode of port in HWS format */
    CPSS_PORT_INTERFACE_MODE_ENT     cpssIfMode;                       /* current interface of port */
    MV_HWS_PORT_INIT_PARAMS          curPortParams;                    /* current port parameters */
    GT_U32                           curLanesList[HWS_MAX_SERDES_NUM]; /* current lanes list */


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(signalStatePtr);
    *signalStatePtr = GT_FALSE; /* Init the value to avoid "non initialized" problem */

    if(CPSS_PORT_INTERFACE_MODE_NA_E == PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portMacMap))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    cpssIfMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

    hwsPortMode = NON_SUP_MODE;

    if(NON_SUP_MODE == hwsPortMode)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,cpssIfMode,
                                    PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap),
                                    &hwsPortMode);
        if(rc != GT_OK)
            return rc;
    }


    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

    if(GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    rc = mvHwsRebuildActiveLaneList(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode, curLanesList);

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesSignalDetectGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *signalDet)",
            devNum, portGroupId, MV_HWS_SERDES_NUM(curLanesList[i]), HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])));
        rc = mvHwsSerdesSignalDetectGet(CAST_SW_DEVNUM(devNum), portGroupId,
                                        MV_HWS_SERDES_NUM(curLanesList[i]),
                                        HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])),
                                        signalStatePtr);
        if((rc != GT_OK) || (GT_FALSE == *signalStatePtr))
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            break;
        }
    }

    return rc;
}


GT_STATUS prvPxPortSerdesSignalDetectLiveGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
)
{
    GT_STATUS                        rc;                               /* return code */
    GT_U32                           portGroupId;                      /* number of core in multi-port-group devices */
    GT_U32                           i;                                /* iterator */
    GT_U32                           portMacMap;                       /* number of mac mapped to this physical port */
    GT_U32                           portGroup;                        /* local core number */
    MV_HWS_PORT_STANDARD             hwsPortMode;                      /* current ifMode of port in HWS format */
    CPSS_PORT_INTERFACE_MODE_ENT     cpssIfMode;                       /* current interface of port */
    MV_HWS_PORT_INIT_PARAMS          curPortParams;                    /* current port parameters */
    GT_U32                           curLanesList[HWS_MAX_SERDES_NUM]; /* current lanes list */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(signalStatePtr);

    if(CPSS_PORT_INTERFACE_MODE_NA_E == PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portMacMap))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    cpssIfMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

    hwsPortMode = NON_SUP_MODE;

    if(NON_SUP_MODE == hwsPortMode)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,cpssIfMode,
                                    PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap),
                                    &hwsPortMode);
        if(rc != GT_OK)
            return rc;
    }


    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

    if(GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    rc = mvHwsRebuildActiveLaneList(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode, curLanesList);

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesSignalDetectGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *signalDet)",
            devNum, portGroupId, MV_HWS_SERDES_NUM(curLanesList[i]), HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])));

        rc = mvHwsAvagoSerdesSignalLiveDetectGet(CAST_SW_DEVNUM(devNum), portGroupId, MV_HWS_SERDES_NUM(curLanesList[i]), signalStatePtr);

        if((rc != GT_OK) || (GT_FALSE == *signalStatePtr))
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            break;
        }
    }

    return rc;
}


/**
* @internal prvCpssPxPortSerdesNumberGet function
* @endinternal
*
* @brief   Return number of active serdeses and array of active serdeses numbers that port uses
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] numOfSerdesesPtr         - number of active serdeses that port uses
* @param[out] serdesesNumberPtr        - array of active serdeses numbers that port uses
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_SUPPORTED         - if interface not supported
*/
GT_STATUS prvCpssPxPortSerdesNumberGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                 *numOfSerdesesPtr,
    OUT GT_U32                 *serdesesNumberPtr
)
{
    GT_STATUS                        rc;                               /* return code */
    GT_U32                           i;                                /* iterator */
    GT_U32                           portMacMap;                       /* number of mac mapped to this physical port */
    GT_U32                           portGroup;                        /* local core number */
    MV_HWS_PORT_STANDARD             hwsPortMode;                      /* current ifMode of port in HWS format */
    CPSS_PORT_INTERFACE_MODE_ENT     cpssIfMode;                       /* current interface of port */
    MV_HWS_PORT_INIT_PARAMS          curPortParams;                    /* current port parameters */
    GT_U32                           curLanesList[HWS_MAX_SERDES_NUM]; /* current lanes list */

    *numOfSerdesesPtr = 0;
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(numOfSerdesesPtr);
    CPSS_NULL_PTR_CHECK_MAC(serdesesNumberPtr);

    if(CPSS_PORT_INTERFACE_MODE_NA_E == PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portMacMap))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    cpssIfMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

    hwsPortMode = NON_SUP_MODE;

    if(NON_SUP_MODE == hwsPortMode)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,cpssIfMode,
                                    PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap),
                                    &hwsPortMode);
        if(rc != GT_OK)
            return rc;
    }

    if(GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    rc = mvHwsRebuildActiveLaneList(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, hwsPortMode, curLanesList);

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
    {

        serdesesNumberPtr[*numOfSerdesesPtr] = MV_HWS_SERDES_NUM(curLanesList[i]);
        (*numOfSerdesesPtr)++;
    }
    return rc;
}


/**
* @internal internal_cpssPxPortSerdesSquelchSet function
* @endinternal
*
* @brief   Set For port Threshold (Squelch) for signal OK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] squelch                  - threshold for signal OK (0-15)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_SUPPORTED         - if interface not supported
*/
GT_STATUS internal_cpssPxPortSerdesSquelchSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  squelch
)
{
    GT_STATUS rc;
    GT_U32 numOfSerdeses;
    GT_U32 serdesesNumber[HWS_MAX_SERDES_NUM];
#ifndef ASIC_SIMULATION
    GT_U32 i;
    GT_U32 rc1;
#endif
    GT_U32 portMacMap;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    if(CPSS_PORT_INTERFACE_MODE_NA_E == PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portMacMap))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    if(squelch > 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortSerdesNumberGet( devNum, portNum, &numOfSerdeses, serdesesNumber);
    if(GT_OK != rc)
    {
        return rc;
    }
#ifndef ASIC_SIMULATION
    for(i = 0; i < numOfSerdeses; i++)
    {
        rc1 = mvHwsAvagoSerdesSignalOkCfg(CAST_SW_DEVNUM(devNum),
                                          PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap),
                                          serdesesNumber[i],
                                          squelch);
        if(0 != rc1)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
#endif
    return GT_OK;
}

/**
* @internal cpssPxPortSerdesSquelchSet function
* @endinternal
*
* @brief   Set For port Threshold (Squelch) for signal OK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] squelch                  - threshold for signal OK (0-15)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_SUPPORTED         - if interface not supported
*/
GT_STATUS cpssPxPortSerdesSquelchSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  squelch
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesSquelchSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, squelch));

    rc = internal_cpssPxPortSerdesSquelchSet(devNum, portNum, squelch);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, squelch));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssPxPortSerdesSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes (true/false).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_NOT_SUPPORTED         - if interface not supported
*/
GT_STATUS cpssPxPortSerdesSignalDetectGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesSignalDetectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, signalStatePtr));

    rc = internal_cpssPxPortSerdesSignalDetectGet(devNum, portNum, signalStatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, signalStatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesStableSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes when it became be stable(true/false).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - the stable signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on signal is not stable during 1000ms.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Using algorithm to detect Serdes Signal to be stable.
*
*/
GT_STATUS internal_cpssPxPortSerdesStableSignalDetectGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32 interval = 100;
    GT_U32 window = 5;          /* 5 * 10msec = 50msec continuous time for stable signal indication */
    GT_U32 window_nosignal = 2; /* 2 * 10msec = 20msec continuous time for stable no signal indication */
    GT_U32 delayMS = 10;
    GT_BOOL signalStateFirst;
    GT_BOOL signalStateNext;
    GT_U32 stableInterval;
    GT_U32 portMacMap;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    if(NULL == signalStatePtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    *signalStatePtr = 0;

    rc = cpssPxPortSerdesSignalDetectGet(devNum, portNum, &signalStateFirst);
    if(GT_OK != rc)
    {
        return rc;
    }
    stableInterval = 0;
    for(i = 0; i < interval; i++)
    {
        rc = cpssPxPortSerdesSignalDetectGet(devNum, portNum, &signalStateNext);
        if(GT_OK != rc)
        {
            return rc;
        }
        if(signalStateNext == signalStateFirst)
        {
            stableInterval++;
            if((GT_TRUE == signalStateFirst) && (stableInterval >= window))
            {
                *signalStatePtr = signalStateFirst;
                return GT_OK;
            }
            else if((GT_FALSE == signalStateFirst) && (stableInterval >= window_nosignal))
            {
                *signalStatePtr = signalStateFirst;
                return GT_OK;
            }
        }
        else
        {
            signalStateFirst = signalStateNext;
            stableInterval = 0;
        }
        cpssOsTimerWkAfter(delayMS);
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssPxPortSerdesStableSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes when it became be stable(true/false).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - the stable signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on signal is not stable during 1000ms.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Using algorithm to detect Serdes Signal to be stable.
*
*/
GT_STATUS cpssPxPortSerdesStableSignalDetectGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesStableSignalDetectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, signalStatePtr));

    rc = internal_cpssPxPortSerdesStableSignalDetectGet(devNum, portNum, signalStatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, signalStatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesLaneSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on specific SerDes lane (true/false).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - number of local core
* @param[in] laneNum                  - number of required serdes lane
*
* @param[out] signalStatePtr           - signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssPxPortSerdesLaneSignalDetectGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  laneNum,
    OUT GT_BOOL *signalStatePtr
)
{
        GT_STATUS rc;
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(signalStatePtr);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesSignalDetectGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *signalDet)", devNum, portGroupId, laneNum, HWS_DEV_SERDES_TYPE(devNum, laneNum));
    rc = mvHwsSerdesSignalDetectGet(CAST_SW_DEVNUM(devNum), portGroupId, laneNum,
                                        HWS_DEV_SERDES_TYPE(devNum, laneNum),
                                        signalStatePtr);
        if(rc != GT_OK)
        {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
        return rc;
}

/**
* @internal cpssPxPortSerdesLaneSignalDetectGet function
* @endinternal
*
* @brief   Return Signal Detect state on specific SerDes lane (true/false).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - number of local core
* @param[in] laneNum                  - number of required serdes lane
*
* @param[out] signalStatePtr           - signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssPxPortSerdesLaneSignalDetectGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          laneNum,
    OUT GT_BOOL         *signalStatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesLaneSignalDetectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, laneNum, signalStatePtr));

    rc = internal_cpssPxPortSerdesLaneSignalDetectGet(devNum, portGroupId, laneNum, signalStatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, laneNum, signalStatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesCDRLockStatusGet function
* @endinternal
*
* @brief   Return SERDES CDR lock status (true - locked /false - not locked).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] cdrLockPtr               - CRD lock state on serdes:
*                                      GT_TRUE  - CDR locked;
*                                      GT_FALSE - CDR not locked.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - cdrLockPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssPxPortSerdesCDRLockStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *cdrLockPtr
)
{
    GT_STATUS   rc;
    GT_U32      portGroupId;
    GT_U32      startSerdes,
                numOfSerdesLanes;
    GT_U32      i;
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(cdrLockPtr);

    if(CPSS_PORT_INTERFACE_MODE_NA_E == PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portMacMap))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum,
                            PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap),
                            &startSerdes, &numOfSerdesLanes);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);
    for(i = startSerdes; i < startSerdes+numOfSerdesLanes; i++)
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesCdrLockStatusGet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *cdrLock)", devNum, portGroupId, i, HWS_DEV_SERDES_TYPE(devNum, i));
        rc = mvHwsSerdesCdrLockStatusGet(CAST_SW_DEVNUM(devNum), portGroupId, i,
                                        HWS_DEV_SERDES_TYPE(devNum, i), cdrLockPtr);

        if((rc != GT_OK) || (GT_FALSE == *cdrLockPtr))
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            break;
        }
    }

    return rc;
}

/**
* @internal cpssPxPortSerdesCDRLockStatusGet function
* @endinternal
*
* @brief   Return SERDES CDR lock status (true - locked /false - not locked).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] cdrLockPtr               - CRD lock state on serdes:
*                                      GT_TRUE  - CDR locked;
*                                      GT_FALSE - CDR not locked.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - cdrLockPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssPxPortSerdesCDRLockStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *cdrLockPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesCDRLockStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cdrLockPtr));

    rc = internal_cpssPxPortSerdesCDRLockStatusGet(devNum, portNum, cdrLockPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cdrLockPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesAutoTuneOptAlgSet function
* @endinternal
*
* @brief   Configure bitmap of training/auto-tuning optimisation algorithms which
*         will run on serdeses of port in addition to usual training.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API configures field SW DB which will be used by
*       cpssPxPortSerdesAutoTune.
*
*/
static GT_STATUS internal_cpssPxPortSerdesAutoTuneOptAlgSet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serdesOptAlgBmp
)
{
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    PRV_CPSS_PX_PORT_SD_OPT_ALG_BMP_MAC(devNum, portMacMap) = serdesOptAlgBmp;

    return GT_OK;
}

/**
* @internal cpssPxPortSerdesAutoTuneOptAlgSet function
* @endinternal
*
* @brief   Configure bitmap of training/auto-tuning optimisation algorithms which
*         will run on serdeses of port in addition to usual training.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] serdesOptAlgBmp          - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API configures field SW DB which will be used by
*       cpssPxPortSerdesAutoTune.
*
*/
GT_STATUS cpssPxPortSerdesAutoTuneOptAlgSet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serdesOptAlgBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesAutoTuneOptAlgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesOptAlgBmp));

    rc = internal_cpssPxPortSerdesAutoTuneOptAlgSet(devNum, portNum, serdesOptAlgBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesOptAlgBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesAutoTuneOptAlgGet function
* @endinternal
*
* @brief   Get bitmap of training/auto-tuning optimisation algorithms which
*         will run on serdeses of port in addition to usual training.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] serdesOptAlgBmpPtr       - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortSerdesAutoTuneOptAlgGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               *serdesOptAlgBmpPtr
)
{
    GT_U32      portMacMap; /* number of mac mapped to this physical port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(serdesOptAlgBmpPtr);

    *serdesOptAlgBmpPtr = PRV_CPSS_PX_PORT_SD_OPT_ALG_BMP_MAC(devNum, portMacMap);

    return GT_OK;
}

/**
* @internal cpssPxPortSerdesAutoTuneOptAlgGet function
* @endinternal
*
* @brief   Get bitmap of training/auto-tuning optimisation algorithms which
*         will run on serdeses of port in addition to usual training.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] serdesOptAlgBmpPtr       - bitmap of optimisation algorithms that should run on
*                                      serdeses of port (see
*                                      CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortSerdesAutoTuneOptAlgGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               *serdesOptAlgBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesAutoTuneOptAlgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesOptAlgBmpPtr));

    rc = internal_cpssPxPortSerdesAutoTuneOptAlgGet(devNum, portNum, serdesOptAlgBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesOptAlgBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesAutoTuneResultsGet function
* @endinternal
*
* @brief   Read the results of SERDES auto tuning.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesTunePtr            - serdes Tune parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssPxPortSerdesAutoTuneResultsGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              laneNum,
    OUT CPSS_PORT_SERDES_TUNE_STC       *serdesTunePtr
)
{
    GT_STATUS                           rc;           /* return code */
    GT_U32                              portGroup;    /* local core number */
    GT_U32                              startSerdes;  /* first serdes of port */
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT results;      /* current serdes tuning values in HWS
                                                         format */
    GT_U32                              portMacMap;   /* number of mac mapped to this physical port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(serdesTunePtr);

    rc = prvCpssPxPortSerdesManualConfig(devNum, portNum, laneNum,
                                           serdesTunePtr, &portGroup,
                                           &startSerdes);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Initialize local structure */
    cpssOsMemSet(&results, 0, sizeof(results));

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesAutoTuneResult(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], *results)", devNum, portGroup, startSerdes+laneNum, HWS_DEV_SERDES_TYPE(devNum, startSerdes+laneNum));
    rc = mvHwsSerdesAutoTuneResult(CAST_SW_DEVNUM(devNum), portGroup, startSerdes+laneNum,
                                   HWS_DEV_SERDES_TYPE(devNum, startSerdes+laneNum), &results);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }
    serdesTunePtr->type                 = CPSS_PORT_SERDES_AVAGO_E;
    serdesTunePtr->rxTune.avago.sqlch   = results.avagoResults.sqleuch;
    serdesTunePtr->txTune.avago.atten   = results.avagoResults.avagoStc.atten;
    serdesTunePtr->txTune.avago.post    = results.avagoResults.avagoStc.post;
    serdesTunePtr->txTune.avago.pre     = results.avagoResults.avagoStc.pre;
    serdesTunePtr->txTune.avago.pre2    = results.avagoResults.avagoStc.pre2;
    serdesTunePtr->txTune.avago.pre3    = results.avagoResults.avagoStc.pre3;
    serdesTunePtr->rxTune.avago.dfeGAIN = 0;
    serdesTunePtr->rxTune.avago.DC      = (GT_U32 )results.avagoResults.DC;
    serdesTunePtr->rxTune.avago.LF      = (GT_U32 )results.avagoResults.LF;
    serdesTunePtr->rxTune.avago.HF      = (GT_U32 )results.avagoResults.HF;
    serdesTunePtr->rxTune.avago.BW      = (GT_U32 )results.avagoResults.BW;
    serdesTunePtr->rxTune.avago.EO      = (GT_U32 )results.avagoResults.EO;
    cpssOsMemCpy( serdesTunePtr->rxTune.avago.DFE, results.avagoResults.DFE, sizeof(serdesTunePtr->rxTune.avago.DFE));

    return rc;

}

/**
* @internal cpssPxPortSerdesAutoTuneResultsGet function
* @endinternal
*
* @brief   Read the results of SERDES auto tuning.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* @param[out] serdesTunePtr            - serdes Tune parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssPxPortSerdesAutoTuneResultsGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_PORT_SERDES_TUNE_STC       *serdesTunePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesAutoTuneResultsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, serdesTunePtr));

    rc = internal_cpssPxPortSerdesAutoTuneResultsGet(devNum, portNum, laneNum, serdesTunePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, serdesTunePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSerdesEnhancedAutoTune function
* @endinternal
*
* @brief   Set Rx training process using the enhance tuning and starts the
*         auto tune process.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
static GT_STATUS internal_cpssPxPortSerdesEnhancedAutoTune
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   min_LF,
    IN  GT_U8                   max_LF
)
{
    GT_STATUS               rc;          /* return code */
    GT_U32                  portGroup;   /* local core number */
    MV_HWS_PORT_STANDARD    hwsPortMode; /* current ifMode of port in HWS format */
    GT_U32                  portMacMap;  /* number of mac mapped to this physical port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap),
                                PRV_CPSS_PX_PORT_SPEED_MAC(devNum,  portMacMap),
                                &hwsPortMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroup  = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortEnhanceTuneSet(devNum[%d], portGroup[%d], serdesNum[%d], serdesType[%d], min_LF[%d], max_LF[%d])",
                             (GT_U8)devNum, (GT_U32)portGroup, (GT_U32)portMacMap, (MV_HWS_PORT_STANDARD)hwsPortMode, (GT_U8)min_LF, (GT_U8)max_LF);
    rc = mvHwsPortEnhanceTuneSet(
        (GT_U8)                   devNum,
        (GT_U32)                  portGroup,
        (GT_U32)                  portMacMap,
        (MV_HWS_PORT_STANDARD)    hwsPortMode,
        (GT_U8)                   min_LF,
        (GT_U8)                   max_LF);
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }
    return rc;
}

/**
* @internal cpssPxPortSerdesEnhancedAutoTune function
* @endinternal
*
* @brief   Set Rx training process using the enhance tuning and starts the
*         auto tune process.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS cpssPxPortSerdesEnhancedAutoTune
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U8                  min_LF,
    IN  GT_U8                  max_LF
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSerdesAutoTuneResultsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, min_LF, max_LF));

    rc = internal_cpssPxPortSerdesEnhancedAutoTune(devNum, portNum, min_LF, max_LF);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, min_LF, max_LF));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortSerdesAutoTuneStatusGetWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function prvCpssPxPortSerdesAutoTuneStatusGet
*         in order to use in common code.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] rxTuneStatusPtr          - (pointer to) rx tune status parameter
* @param[out] rxTuneStatusPtr          - (pointer to) tx tune status parameter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssPxPortSerdesAutoTuneStatusGetWrapper
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *rxTuneStatusPtr,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *txTuneStatusPtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum;
    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatusTemp;
    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatusTemp;

    CPSS_LOG_INFORMATION_MAC("inside PortSerdesAutoTuneStatusGetWrapper function wrapper");

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(rxTuneStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(txTuneStatusPtr);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = cpssPxPortSerdesAutoTuneStatusGet(devNum, portNum, &rxTuneStatusTemp, &txTuneStatusTemp);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "AutoTuneStatusGetWrapper failed=%d", rc);
    }

    *rxTuneStatusPtr = rxTuneStatusTemp;
    *txTuneStatusPtr = txTuneStatusTemp;

    return GT_OK;
}


/**
* @internal prvCpssPxPortSerdesFunctionsObjInit function
* @endinternal
*
* @brief   Init and bind common function pointers to Px port serdes functions.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssPxPortSerdesFunctionsObjInit
(
    IN GT_SW_DEV_NUM devNum
)
{
    CPSS_LOG_INFORMATION_MAC("binding px port serdes functions");

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /* common functions bind - (currently for Port Manager use) */
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppTuneStatusGetFunc = prvCpssPxPortSerdesAutoTuneStatusGetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSigDetGetFunc = cpssPxPortSerdesSignalDetectGet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppTuneExtSetFunc = cpssPxPortSerdesAutoTune;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppLaneTuneSetFunc = cpssPxPortSerdesLaneTuningSet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppLaneTuneGetFunc = cpssPxPortSerdesLaneTuningGet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSerdesNumGetFunc = prvCpssPxPortIfModeSerdesNumGet;

    return GT_OK;
}

/**
* @internal prvCpssPxPortSerdesLoopbackModeSetWrapper function
* @endinternal
*
* @brief   Wrapper function for configuring loopback of specific type on SerDes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneBmp                  - bitmap of SERDES lanes (bit 0-> lane 0, etc.) where to
*                                      set loopback (not used for Lion2)
* @param[in] mode                     - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Pay attention - when new loopback mode enabled on serdes lane,
*       previous mode disabled
*
*/
GT_STATUS prvCpssPxPortSerdesLoopbackModeSetWrapper
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneBmp,
    IN  CPSS_PORT_SERDES_LOOPBACK_MODE_ENT   mode
)
{
    GT_STATUS rc;

    laneBmp = laneBmp;

    rc = cpssPxPortSerdesLoopbackModeSet(devNum,portNum,(CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT)mode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling cpssPxPortSerdesLoopbackModeSet from wrapper failed=%d");
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortSerdesLanePolaritySetWrapper function
* @endinternal
*
* @brief   Wrapper function Set the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupNum             - number of port group (local core), not used for non-multi-core
* @param[in] laneNum                  - number of SERDES lane
* @param[in] invertTx                 - GT_TRUE  - Transmit Polarity Invert.
*                                      GT_FALSE - no invert
* @param[in] invertRx                 - GT_TRUE  - Receive Polarity Invert.
*                                      GT_FALSE - no invert
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - If serdes initialized: Set the Tx/Rx polarity parameters for lane
*       in both HW and SW DB
*       - If serdes was not initialized: Set the Tx/Rx polarity parameters
*       for lane only in SW DB
*
*/
GT_STATUS prvCpssPxPortSerdesLanePolaritySetWrapper
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32          portGroupNum,
    IN  GT_U32          laneNum,
    IN  GT_BOOL         invertTx,
    IN  GT_BOOL         invertRx
)
{
    GT_STATUS rc;

    rc = cpssPxPortSerdesLanePolaritySet(devNum, portGroupNum, laneNum, invertTx, invertRx);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling cpssPxPortSerdesLanePolaritySet from wrapper failed=%d");
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortSerdesAutoTuneResultsGetWrapper
*           function
* @endinternal
*
* @brief   Wrapper function Get the SerDes Tune Result.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of port
* @param[in] laneNum                  - number of SERDES lane
* @param[out] serdesTunePtr           - Pointer to Tuning
*       result.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssPxPortSerdesAutoTuneResultsGetWrapper
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 laneNum,
    OUT CPSS_PORT_SERDES_TUNE_STC       *serdesTunePtr
)
{
    GT_STATUS rc;

    rc = cpssPxPortSerdesAutoTuneResultsGet(devNum, portNum, laneNum, serdesTunePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling cpssDxChPortSerdesAutoTuneResultsGet from wrapper failed=%d",rc);
    }

    return rc;
}

/**
* @internal prvCpssPxAutoNeg1GSgmiiWrapper function
* @endinternal
*
* @brief   Auto-Negotiation sequence for 1G QSGMII/SGMII
*
* @note    APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] ifMode                - port ifMode
* @param[in] autoNegotiationPtr    - auto nego params str
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssPxAutoNeg1GSgmiiWrapper
(
    IN  GT_SW_DEV_NUM                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                         portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT                 ifMode,
    IN  CPSS_PORT_MANAGER_SGMII_AUTO_NEGOTIATION_STC *autoNegotiationPtr
)
{
    GT_STATUS rc = GT_OK;

    CPSS_NULL_PTR_CHECK_MAC(autoNegotiationPtr);
    GT_UNUSED_PARAM(ifMode);

    rc = cpssPxPortInbandAutoNegEnableSet(devNum, portNum, autoNegotiationPtr->inbandEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssPxPortDuplexAutoNegEnableSet(devNum, portNum, autoNegotiationPtr->duplexEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssPxPortSpeedAutoNegEnableSet(devNum, portNum, autoNegotiationPtr->speedEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssPxPortInBandAutoNegBypassEnableSet(CAST_SW_DEVNUM(devNum), portNum, autoNegotiationPtr->byPassEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssPxPortFlowCntrlAutoNegEnableSet(CAST_SW_DEVNUM(devNum), portNum, autoNegotiationPtr->flowCtrlEnable, autoNegotiationPtr->flowCtrlPauseAdvertiseEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    return rc;
}

/**
* @internal prvCpssPxHwCoreClockGetWrapper function
* @endinternal
*
* @brief   get the current core clock
*
* @note    APPLICABLE DEVICES:      Pipe.
*
* @param[in]  devNum                   - physical device number
* @param[out] coreClkDbPtr             - core clock Db
* @param[out] coreClkHWPtr             - core clock HW
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* note: in px we only get one clock (Db), but because the
* function in dx return 2 clocks, we return here 2 clocks aswell
* (which are the same clock)
*/
GT_STATUS prvCpssPxHwCoreClockGetWrapper
(
    IN  GT_U8   devNum,
    OUT GT_U32  *coreClkDbPtr,
    OUT GT_U32  *coreClkHwPtr
)
{
    GT_STATUS rc;
    CPSS_NULL_PTR_CHECK_MAC(coreClkDbPtr);
    CPSS_NULL_PTR_CHECK_MAC(coreClkHwPtr);

    rc = cpssPxHwCoreClockGet(devNum, coreClkDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    *coreClkHwPtr = *coreClkDbPtr; /*read function note */

    return rc;
}

/* --------------------- debug functions --------------------------------- */

/**
* @internal prvDebugCpssPxPortHwModeGet function
* @endinternal
*
* @brief   Read from HW current interface and speed of port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvDebugCpssPxPortHwModeGet
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum
)
{
    GT_STATUS                   rc;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;

    rc = prvCpssPxPortInterfaceModeHwGet(devNum, portNum, &ifMode);
    if(rc != GT_OK)
    {
        cpssOsPrintf("prvCpssPxPortInterfaceModeHwGet fail:rc=%d\n", rc);
        return rc;
    }

    cpssOsPrintf("ifMode=%d,", ifMode);

    rc = prvCpssPxPortSpeedHwGet(devNum, portNum, &speed);
    if(rc != GT_OK)
    {
        cpssOsPrintf("prvCpssPxPortSpeedHwGet fail:rc=%d\n", rc);
        return rc;
    }

    cpssOsPrintf("speed=%d\n", speed);

    return GT_OK;
}

/**
* @internal prvDebugCpssPxPortTrxTune function
* @endinternal
*
* @brief   Run TRX training on pair of ports - needed at this moment for HW testing
*         because TRX training must be started on both connected ports more or less
*         simultaneously and automatic TRX training mechanism too complicated for that.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum0                 - first physical port number
* @param[in] portNum1                 - second physical port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - serdesTunePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*/
GT_STATUS prvDebugCpssPxPortTrxTune
(
    IN  GT_SW_DEV_NUM                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum0,
    IN  GT_PHYSICAL_PORT_NUM                     portNum1
)
{
    GT_STATUS rc;

    rc = cpssPxPortSerdesAutoTune(devNum, portNum0,
                        CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssPxPortSerdesAutoTune port %d failed\n", portNum0);
        return rc;
    }

    rc = cpssPxPortSerdesAutoTune(devNum, portNum1,
                        CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssPxPortSerdesAutoTune port %d failed\n", portNum1);
        return rc;
    }

    return GT_OK;
}

/* ----- end of debug section; please don't place below real API's ------ */




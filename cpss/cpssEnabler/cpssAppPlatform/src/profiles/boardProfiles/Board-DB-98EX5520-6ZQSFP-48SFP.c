/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file Board-DB-98EX5520-6ZQSFP-48SFP.c
*
* @brief Aldrin 2 DB Board related information.
*
* @version   1
********************************************************************************/

#include <profiles/cpssAppPlatformProfile.h>

CPSS_DXCH_PORT_MAP_STC aldrinB2B_0 [] =
{ /* Port,            mappingType                              portGroup,  intefaceNum,    txQPort,       TM Enable ,     tmPortInd*/
    {  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,              9,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,              8,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,             11,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,             10,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,             13,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,             12,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,             15,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,             14,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,             29,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,             28,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       31,             31,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       30,             30,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,             25,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,             24,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,             27,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,             26,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,             21,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,             20,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,             23,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,             22,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,             17,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,             16,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,             19,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,             18,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,              0,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,              1,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,              2,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,              4,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,              6,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,      127,             63,           GT_FALSE,          GT_NA,           GT_FALSE}
};

CPSS_DXCH_PORT_MAP_STC aldrinB2B_1 [] =
{ /* Port,            mappingType                              portGroup,  intefaceNum,    txQPort,       TM Enable ,     tmPortInd*/
    {  1,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,             29,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  2,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,             28,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  3,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       31,             31,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  4,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       30,             30,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  5,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,             25,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  6,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,             24,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  7,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,             27,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  8,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,             26,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  9,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,             21,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,             20,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,             23,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,             22,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,             17,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,             16,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,             19,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,             18,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,             13,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,             12,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,             15,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,             14,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,              9,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,              8,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,             11,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,             10,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,              0,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,              1,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,              2,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,              4,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,              6,           GT_FALSE,          GT_NA,           GT_FALSE}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,      127,             63,           GT_FALSE,          GT_NA,           GT_FALSE}
};

CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC DB_98EX5520_6ZQSFP_48SFP =
{
    _SM_(boardName             ) "DB-98EX5520-6ZQSFP-48SFP",
    _SM_(osCallType            ) CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E,
    _SM_(extDrvCallType        ) CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC DB_98EX5520_6ZQSFP_48SFP_BOARD_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E,
    {
        _SM_(boardPtr              ) &DB_98EX5520_6ZQSFP_48SFP
    }
};

CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC PP_98EX5520_PP_MAP_INFO =
{
    _SM_(mapType               ) CPSS_APP_PLATFORM_PP_MAP_TYPE_FIXED_E,
    _SM_(mngInterfaceType      ) CPSS_CHANNEL_PEX_MBUS_E,
    _SM_(devNum                ) 0,
    _SM_(portMap               ) NULL,
    _SM_(portMapSize           ) 0,
    {{
        _SM_(busId             ) 0x01,
        _SM_(deviceId          ) 0x00,
        _SM_(functionId        ) 0x00,
    }}
};
CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC PP_98EX5520_PP_MAP_INFO_0 =
{
    _SM_(mapType               ) CPSS_APP_PLATFORM_PP_MAP_TYPE_FIXED_E,
    _SM_(mngInterfaceType      ) CPSS_CHANNEL_PEX_MBUS_E,
    _SM_(devNum                ) 0,
    _SM_(portMap               ) aldrinB2B_0,
    _SM_(portMapSize           ) 30,
    {{
        _SM_(busId     ) 0x01,
#ifdef ASIC_SIMULATION
        _SM_(deviceId  ) 0x00,
#else
        _SM_(deviceId  ) 0x00,
#endif
        _SM_(functionId) 0x00,

    }}
};
CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC PP_98EX5520_PP_MAP_INFO_1 =
{
    _SM_(mapType               ) CPSS_APP_PLATFORM_PP_MAP_TYPE_FIXED_E,
    _SM_(mngInterfaceType      ) CPSS_CHANNEL_PEX_MBUS_E,
    _SM_(devNum                ) 1,
    _SM_(portMap               ) aldrinB2B_1,
    _SM_(portMapSize           ) 30,
    {{
        _SM_(busId     ) 0x02,
#ifdef ASIC_SIMULATION
        _SM_(deviceId  ) 0x01,
#else
        _SM_(deviceId  ) 0x00,
#endif
        _SM_(functionId) 0x00,
    }}
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC DB_98EX5520_6ZQSFP_48SFP_PP_MAP_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_E,
    {
#ifdef ANSI_PROFILES
        _SM_(ppMapPtr   ) (CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC*)&PP_98EX5520_PP_MAP_INFO
#else
        _SM_(ppMapPtr   ) &PP_98EX5520_PP_MAP_INFO
#endif
    }
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC DB_98EX5520_6ZQSFP_48SFP_PP_MAP_INFO_0 =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_E,
    {
#ifdef ANSI_PROFILES
        _SM_(ppMapPtr   ) (CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC*)&PP_98EX5520_PP_MAP_INFO_0
#else
        _SM_(ppMapPtr   ) &PP_98EX5520_PP_MAP_INFO_0
#endif
    }
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC DB_98EX5520_6ZQSFP_48SFP_PP_MAP_INFO_1 =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_E,
    {
#ifdef ANSI_PROFILES
        _SM_(ppMapPtr   ) (CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC*)&PP_98EX5520_PP_MAP_INFO_1
#else
        _SM_(ppMapPtr   ) &PP_98EX5520_PP_MAP_INFO_1
#endif
    }
};
CPSS_APP_PLATFORM_BOARD_PARAM_LED_STC DB_98EX5520_6ZQSFP_48SFP_LED_TABLE =
{
    _SM_(devNum                ) 0,
    _SM_(ledConf               ) { CPSS_LED_ORDER_MODE_BY_PORT_E, GT_FALSE, CPSS_LED_BLINK_DUTY_CYCLE_2_E, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {0, } },
    _SM_(ledClass              ) { GT_FALSE, GT_TRUE, CPSS_LED_BLINK_SELECT_1_E, GT_TRUE, 10, GT_FALSE, GT_FALSE },
    _SM_(ledInterfaceNum       ) 3
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC DB_98EX5520_6ZQSFP_48SFP_LED_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_LED_E,
    {
#ifdef ANSI_PROFILES
        _SM_(ledInfoPtr ) (CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC*)&DB_98EX5520_6ZQSFP_48SFP_LED_TABLE
#else
        _SM_(ledInfoPtr ) &DB_98EX5520_6ZQSFP_48SFP_LED_TABLE
#endif
    }
};

#ifdef CPSS_APP_PLATFORM_PHASE_2
CPSS_APP_PLATFORM_BOARD_PROFILE_STC DB_98EX5520_6ZQSFP_48SFP_CASCADE_PROFILE =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_CASCADE_E,
    {
        _SM_(cascadePtr            ) NULL
    }
};
#endif

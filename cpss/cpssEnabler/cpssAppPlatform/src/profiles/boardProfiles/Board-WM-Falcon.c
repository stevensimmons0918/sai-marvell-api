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

CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC WM_FALCON_12_8_Z2 =
{
    _SM_(boardName             ) "WM-FALCON-12.8-Z2",
    _SM_(osCallType            ) CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E,
    _SM_(extDrvCallType        ) CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC WM_FALCON_12_8_Z2_BOARD_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E,
    {
        _SM_(boardPtr          ) &WM_FALCON_12_8_Z2
    }
};

CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC PP_FALCON_12_8_Z2_PP_MAP_TABLE =
{
    _SM_(mapType               ) CPSS_APP_PLATFORM_PP_MAP_TYPE_FIXED_E,
    _SM_(mngInterfaceType      ) CPSS_CHANNEL_PEX_FALCON_Z_E,
    _SM_(devNum                ) 0,
    _SM_(portMap               ) NULL,
    _SM_(portMapSize           ) 0,
    {{
        _SM_(busId     ) 0x01,
        _SM_(deviceId  ) 0x00,
        _SM_(functionId) 0x00,
    }}
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC WM_FALCON_12_8_Z2_PP_MAP_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_E,
    {
#ifdef ANSI_PROFILES
        _SM_(ppMapPtr   ) (CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC*)&PP_FALCON_12_8_Z2_PP_MAP_TABLE
#else
        _SM_(ppMapPtr   ) &PP_FALCON_12_8_Z2_PP_MAP_TABLE
#endif
    }
};

/****************************************************************************/
/****************************************************************************/

CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC WM_FALCON_12_8_R0_1 =
{
    _SM_(boardName             ) "WM-FALCON-12.8-Rev0.1",
    _SM_(osCallType            ) CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E,
    _SM_(extDrvCallType        ) CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E
};

CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC FALCON_12_8_Belly2Belly_1 =
{
    _SM_(boardName             ) "FALCON-Belly-To-Belly",
    _SM_(osCallType            ) CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E,
    _SM_(extDrvCallType        ) CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC WM_FALCON_12_8_R0_1_BOARD_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E,
    {
        _SM_(boardPtr          ) &WM_FALCON_12_8_R0_1
    }
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC WM_FALCON_12_8_Belly2Belly_1_BOARD_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E,
    {
        _SM_(boardPtr          ) &FALCON_12_8_Belly2Belly_1
    }
};

CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC PP_FALCON_12_8_R0_1_PP_MAP_TABLE =
{
    _SM_(mapType               ) CPSS_APP_PLATFORM_PP_MAP_TYPE_FIXED_E,
    _SM_(mngInterfaceType      ) CPSS_CHANNEL_PEX_EAGLE_E,
    _SM_(devNum                ) 0,
    _SM_(portMap               ) NULL,
    _SM_(portMapSize           ) 0,
    {{
        _SM_(busId     ) 0x01,
        _SM_(deviceId  ) 0x00,
        _SM_(functionId) 0x00,
    }}
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC WM_FALCON_12_8_R0_1_PP_MAP_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_E,
    {
#ifdef ANSI_PROFILES
        _SM_(ppMapPtr   ) (CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC*)&PP_FALCON_12_8_R0_1_PP_MAP_TABLE
#else
        _SM_(ppMapPtr   ) &PP_FALCON_12_8_R0_1_PP_MAP_TABLE
#endif
    }
};

/****************************************************************************/
/****************************************************************************/

CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC WM_FALCON_6_4 =
{
    _SM_(boardName             ) "FALCON-6.4",
    _SM_(osCallType            ) CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E,
    _SM_(extDrvCallType        ) CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC WM_FALCON_6_4_BOARD_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E,
    {
        _SM_(boardPtr          ) &WM_FALCON_6_4
    }
};

#if 0
CPSS_APP_PLATFORM_BOARD_PARAM_LED_STC DB_98EX5520_6ZQSFP_48SFP_LED_TABLE =
{
    _SM_(devNum                ) 0,
    _SM_(ledConf               ) { CPSS_LED_ORDER_MODE_BY_PORT_E, GT_FALSE, CPSS_LED_BLINK_DUTY_CYCLE_2_E },
    _SM_(ledClass              ) { GT_FALSE, GT_TRUE, CPSS_LED_BLINK_SELECT_1_E, GT_TRUE, 10, GT_FALSE, GT_FALSE },
    _SM_(ledInterfaceNum       ) 3
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC DB_98EX5520_6ZQSFP_48SFP_LED_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_LED_E,
    {
        _SM_(ledInfoPtr            ) &DB_98EX5520_6ZQSFP_48SFP_LED_TABLE
    }
};
#endif

#ifdef CPSS_APP_PLATFORM_PHASE_2
CPSS_APP_PLATFORM_BOARD_PROFILE_STC DB_98EX5520_6ZQSFP_48SFP_CASCADE_PROFILE =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_CASCADE_E,
    {
        _SM_(cascadePtr            ) NULL
    }
};
#endif

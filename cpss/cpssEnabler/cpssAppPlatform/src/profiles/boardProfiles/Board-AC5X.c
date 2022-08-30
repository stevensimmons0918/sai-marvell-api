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
* @file Board-WM-AC5X.c
*
* @brief AC5X Board profiles
*
* @version   1
********************************************************************************/

#include <profiles/cpssAppPlatformProfile.h>

CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC WM_AC5X =
{
    _SM_(boardName             ) "WM-AC5X",
    _SM_(osCallType            ) CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E,
    _SM_(extDrvCallType        ) CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E
};

CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC RD_AC5X_32SFP =
{
    _SM_(boardName             ) "RD-AC5X-32SFP",
    _SM_(osCallType            ) CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E,
    _SM_(extDrvCallType        ) CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E
};


CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC DB_AC5X_8SFP =
{
    _SM_(boardName             ) "DB-AC5X-8SFP",
    _SM_(osCallType            ) CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E,
    _SM_(extDrvCallType        ) CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC WM_AC5X_BOARD_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E,
    {
        _SM_(boardPtr          ) &WM_AC5X
    }
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC DB_AC5X_8SFP_BOARD_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E,
    {
        _SM_(boardPtr          ) &DB_AC5X_8SFP
    }
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC RD_AC5X_32SFP_BOARD_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E,
    {
        _SM_(boardPtr          ) &RD_AC5X_32SFP
    }
};


CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC PP_AC5X_PP_MAP_TABLE =
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

CPSS_APP_PLATFORM_BOARD_PROFILE_STC WM_AC5X_PP_MAP_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_E,
    {
#ifdef ANSI_PROFILES
        _SM_(ppMapPtr   ) (CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC*)&PP_AC5X_PP_MAP_TABLE
#else
        _SM_(ppMapPtr   ) &PP_AC5X_PP_MAP_TABLE
#endif
    }
};

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* cpssHalConfig.c
*
* DESCRIPTION:
*       define system configuration paramaters, that cannot be read from the device
*       example for configuration:
*           Pci scan device Id - current CPSS_PP_FAMILY_DXCH_BOBCAT3_E = CPSS_98DX42FF_CNS only
*           mngInterfaceType - current is CPSS_CHANNEL_PEX_E only
*           size of cpss memory - 2048*1024
*           serdesRefClock - APPDEMO_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E
*           ...
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include "xpsCommon.h"
#include <cpssHalSys.h>

inline CPSS_PP_INTERFACE_CHANNEL_ENT cpssHalSys_param_mngInterfaceType(
    XP_DEV_TYPE_T xpDevType)
{
    switch (xpDevType)
    {
        case ALDB2B:
        case ALD2:
        case ALD:
        case ALDDB:
        case CYGNUS:
        case ALDRIN2XL:
        case TG48M_P:
            return CPSS_CHANNEL_PEX_MBUS_E;
        default: //FALCON, AC5X
            return CPSS_CHANNEL_PEX_EAGLE_E;
    }
}

inline GT_U32  cpssHalSys_param_default_mem_init_size(void)
{
    return (2048*1024);
}

inline GT_U32 cpssHalSys_param_rxDescNum(XP_DEV_TYPE_T xpDevType)
{
    switch (xpDevType)
    {
        case ALDB2B:
        case ALD2:
        case ALD:
        case ALDDB:
        case ALDRIN2XL:
        case TG48M_P:
            return 256;
        default:
            return 160;
    }
}

inline GT_U32 cpssHalSys_param_txDescNum(XP_DEV_TYPE_T xpDevType)
{
    return 160;
}

inline GT_U32 cpssHalSys_param_rxBuffSize(XP_DEV_TYPE_T xpDevType)
{
    return (2*1024);
}

inline GT_U32 cpssHalSys_param_rxBuffNum(XP_DEV_TYPE_T xpDevType)
{
    switch (xpDevType)
    {
        case ALDB2B:
        case ALD2:
        case ALD:
        case ALDDB:
        case ALDRIN2XL:
        case TG48M_P:
            return 256;
        default:
            return 160;
    }
}

inline GT_U32 cpssHalSys_param_rxBufferPercentage_0(XP_DEV_TYPE_T xpDevType)
{
    return 10; /* Best Effort */
}

inline GT_U32 cpssHalSys_param_rxBufferPercentage_1(XP_DEV_TYPE_T xpDevType)
{
    return 10;  /* Background */
}

inline GT_U32 cpssHalSys_param_rxBufferPercentage_2(XP_DEV_TYPE_T xpDevType)
{
    return 10; /* Excellent Effort */
}

inline GT_U32 cpssHalSys_param_rxBufferPercentage_3(XP_DEV_TYPE_T xpDevType)
{
    return 10; /* Critical Applications */
}

inline GT_U32 cpssHalSys_param_rxBufferPercentage_4(XP_DEV_TYPE_T xpDevType)
{
    return 10; /* Video, < 100 ms latency and jitter */
}

inline GT_U32 cpssHalSys_param_rxBufferPercentage_5(XP_DEV_TYPE_T xpDevType)
{
    return 10;  /* Voice, < 10 ms latency and jitter */
}

inline GT_U32 cpssHalSys_param_rxBufferPercentage_6(XP_DEV_TYPE_T xpDevType)
{
    return 10;  /* Internetwork Control */
}

inline GT_U32 cpssHalSys_param_rxBufferPercentage_7(XP_DEV_TYPE_T xpDevType)
{
    return 10; /* Network Control */
}

inline GT_U32 cpssHalSys_param_auqDescNum(XP_DEV_TYPE_T xpDevType)
{
    return 4096;
}

inline GT_BOOL cpssHalSys_param_fuqUseSeparate(XP_DEV_TYPE_T xpDevType)
{
    return GT_FALSE;
}

inline GT_U32 cpssHalSys_param_fuqDescBlockSize(XP_DEV_TYPE_T xpDevType)
{
    return 0;
}

inline CPSS_DXCH_CFG_ROUTING_MODE_ENT cpssHalSys_param_ppRoutingMode(
    XP_DEV_TYPE_T xpDevType)
{
    return CPSS_DXCH_TCAM_ROUTER_BASED_E;
}

inline GT_U32 cpssHalSys_param_maxNumOfPbrEntries(XP_DEV_TYPE_T xpDevType)
{
    return 8192;
}

inline CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT cpssHalSys_param_trunkMembersMode(
    XP_DEV_TYPE_T xpDevType)
{
    return CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E;//CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E
}

inline GT_U32 cpssHalSys_param_mru(XP_DEV_TYPE_T xpDevType)
{
    return 10240;
}

inline GT_U32 cpssHalSys_param_maxNumOfQueues(XP_DEV_TYPE_T xpDevType)
{
    return 8;
}


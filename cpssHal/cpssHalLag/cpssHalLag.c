/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 ********************************************************************************
 * @file cpssHalLag.c
 *
 * @brief Private API implementation for CPSS Lag functions that can be used in XPS layer
 *
 * @version   01
 ********************************************************************************
 */

#include "cpssHalLag.h"
#include "cpssHalUtil.h"
#include "xpsCommon.h"
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include "cpssHalDevice.h"

GT_STATUS cpssHalSetTrunkHashCrcParameters
(
    int                                  devId,
    CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT     crcMode,
    GT_U32                               crcSeed
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTrunkHashCrcParametersSet(devNum, crcMode, crcSeed, 0);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set Trunk Hash CRC parameters on DevId (%d)", devNum);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetRandomHashSeedParameters
(
    int                                  devId,
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    GT_U32                               seed
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTrunkHashPrngSeedSet(devNum, hashClient, seed);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set Trunk Hash Random parameters on DevId (%d)", devNum);
            return status;
        }
    }
    return status;
}
GT_STATUS cpssHalSetTrunkHashGlobalMode
(
    int                                  devId,
    CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  hashMode
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTrunkHashGlobalModeSet(devNum, hashMode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set Trunk Hash Mode on device (%d)", devNum);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalGetTrunkHashGlobalMode
(
    int                                   devId,
    CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  *hashModePtr
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTrunkHashGlobalModeGet(devNum, hashModePtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get Trunk Hash Mode on device (%d)", devNum);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalAddTrunkMember
(
    int                      devId,
    GT_TRUNK_ID              trunkId,
    CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTrunkMemberAdd(devNum, trunkId, memberPtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Adding Port to trunk failed :%d ", status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalEnableTrunkMember
(
    int                      devId,
    GT_TRUNK_ID              trunkId,
    CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTrunkMemberEnable(devNum, trunkId, memberPtr);
        if (status !=GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Enabling Port to trunk failed :%d ", status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalDisableTrunkMember
(
    int                      devId,
    GT_TRUNK_ID              trunkId,
    CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS status = GT_OK;
    int cpssDevNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        status = cpssDxChTrunkMemberDisable(cpssDevNum, trunkId, memberPtr);
        if (status !=GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Disabling Port to trunk failed :%d ", status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalRemoveTrunkMember
(
    int                      devId,
    GT_TRUNK_ID              trunkId,
    CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTrunkMemberRemove(devNum, trunkId, memberPtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Removing Port from trunk failed :%d ", status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetTrunkHashIpAddMacMode
(
    int      devId,
    GT_BOOL  enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTrunkHashIpAddMacModeSet(devNum, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting MAC Hash Mode failed");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetTrunkHashIpMode
(
    int     devId,
    GT_BOOL enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTrunkHashIpModeSet(devNum, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Setting Ip Hash Mode failed");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetTrunkHashL4Mode
(
    int                             devId,
    CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT hashMode
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTrunkHashL4ModeSet(devNum, hashMode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Setting L4 Hash Mode failed");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetTrunkHashIpv6Mode
(
    int                                  devId,
    CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT   hashMode
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTrunkHashIpv6ModeSet(devNum, hashMode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting Ipv6 Hash Mode failed");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetTrunkHashMplsModeEnable
(
    int     devId,
    GT_BOOL enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTrunkHashMplsModeEnableSet(devNum, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting Mpls Hash Mode failed");
            return status;
        }
    }
    return status;
}


GT_STATUS cpssHalSetHashNumBits
(
    int devId,
    GT_U32 startBit,
    GT_U32 numOfBits,
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTrunkHashNumBitsSet(devNum, hashClient, startBit, numOfBits);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting TrunkHashNumBits failed");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalTrunkEcmpLttTableRandomEnableSet
(
    int devId,
    GT_TRUNK_ID trunkId,
    GT_BOOL enable
)
{
    GT_STATUS status = GT_OK;
    CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC ecmpLttInfo;

    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTrunkEcmpLttTableGet(devNum, trunkId, &ecmpLttInfo);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Getting TrunkEcmpLttTable failed");
            return status;
        }

        ecmpLttInfo.ecmpRandomPathEnable = enable;

        status = cpssDxChTrunkEcmpLttTableSet(devNum, trunkId, &ecmpLttInfo);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting TrunkEcmpLttTable failed");
            return status;
        }
    }
    return status;
}

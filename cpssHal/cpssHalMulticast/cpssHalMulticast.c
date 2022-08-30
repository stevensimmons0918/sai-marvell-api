/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

#include "cpssHalMulticast.h"
#include "cpssHalDevice.h"
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

GT_STATUS cpssHalIpMllBridgeEnable(int devId,
                                   GT_BOOL enable)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChIpMllBridgeEnable(devNum, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChIpMllBridgeEnable Failed :%d ", status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalBrgMcMemberAdd(GT_U8 cpssDevId,
                                GT_U16 vidx,
                                GT_PHYSICAL_PORT_NUM cpssPortNum)
{
    GT_STATUS   status = GT_OK;
    status = cpssDxChBrgMcMemberAdd(cpssDevId, vidx, cpssPortNum);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChBrgMcMemberAdd Failed %d", status);
        return status;
    }
    return status;
}

GT_STATUS cpssHalBrgMcMemberDelete(GT_U8 cpssDevId,
                                   GT_U16 vidx,
                                   GT_PHYSICAL_PORT_NUM cpssPortNum)
{
    GT_STATUS   status = GT_OK;
    status = cpssDxChBrgMcMemberDelete(cpssDevId, vidx, cpssPortNum);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChBrgMcMemberDelete Failed %d", status);
        return status;
    }
    return status;
}

GT_STATUS cpssHalBrgMcGroupDelete(int devId,
                                  GT_U16 vidx)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgMcGroupDelete(devNum, vidx);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgMcGroupDelete Failed vidx :%d rc :%d ", vidx, status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalL2MLLPairRead(int devId,
                               GT_U32 mllPairHwIdx,
                               CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT mllPairReadForm,
                               CPSS_DXCH_L2_MLL_PAIR_STC *mllPairEntryPtr)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChL2MllPairRead(devNum, mllPairHwIdx, mllPairReadForm,
                                       mllPairEntryPtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "mllPairReadForm Failed :%d ", status);
            return status;
        }
    }
    return status;
}
GT_STATUS cpssHalIpMLLPairRead(int devId,
                               GT_U32 mllPairHwIdx,
                               CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT mllPairReadForm,
                               CPSS_DXCH_IP_MLL_PAIR_STC *mllPairEntryPtr)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChIpMLLPairRead(devNum, mllPairHwIdx, mllPairReadForm,
                                       mllPairEntryPtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "mllPairReadForm Failed :%d ", status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalIpMLLPairWrite(int devId,
                                GT_U32 mllPairHwIdx,
                                CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT mllPairWriteForm,
                                CPSS_DXCH_IP_MLL_PAIR_STC *mllPairEntryPtr)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChIpMLLPairWrite(devNum, mllPairHwIdx, mllPairWriteForm,
                                        mllPairEntryPtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChIpMLLPairWrite Failed :%d ", status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalIpMcRouteEntriesWrite(int devId,
                                       GT_U32 routeEntryIndex,
                                       CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChIpMcRouteEntriesWrite(devNum, routeEntryIndex, routeEntryPtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChIpMcRouteEntriesWrite Failed :%d ", status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalIpMcRouteEntriesRead(int devId,
                                      GT_U32 routeEntryIndex,
                                      CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChIpMcRouteEntriesRead(devNum, routeEntryIndex, routeEntryPtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChIpMcRouteEntriesRead Failed :%d ", status);
            return status;
        }
    }
    return status;
}
GT_STATUS cpssHalIpLpmIpv4McEntryAdd
(
    GT_U32                       lpmDBId,
    GT_U32                       vrId,
    GT_IPADDR                    *ipGroupPtr,
    GT_U32                       ipGroupPrefixLen,
    GT_IPADDR                    *ipSrcPtr,
    GT_U32                       ipSrcPrefixLen,
    CPSS_DXCH_IP_LTT_ENTRY_STC   *mcRouteLttEntryPtr,
    GT_BOOL                      override,
    GT_BOOL                      defragmentationEnable
)

{
    GT_STATUS   status = GT_OK;
    status = cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, ipGroupPtr,
                                         ipGroupPrefixLen,
                                         ipSrcPtr, ipSrcPrefixLen, mcRouteLttEntryPtr,
                                         override, defragmentationEnable);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChIpLpmIpv4McEntryAdd Failed :%d ", status);
        return status;
    }
    return status;
}

GT_STATUS cpssHalIpLpmIpv4McEntryDel
(
    GT_U32                       lpmDBId,
    GT_U32                       vrId,
    GT_IPADDR                    *ipGroupPtr,
    GT_U32                       ipGroupPrefixLen,
    GT_IPADDR                    *ipSrcPtr,
    GT_U32                       ipSrcPrefixLen
)

{
    GT_STATUS   status = GT_OK;
    status = cpssDxChIpLpmIpv4McEntryDel(lpmDBId, vrId, ipGroupPtr,
                                         ipGroupPrefixLen,
                                         ipSrcPtr, ipSrcPrefixLen);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChIpLpmIpv4McEntryDel Failed :%d ", status);
        return status;
    }
    return status;
}

GT_STATUS cpssHalIpLpmIpv4McEntrySearch
(
    GT_U32                       lpmDBId,
    GT_U32                       vrId,
    GT_IPADDR                    *ipGroupPtr,
    GT_U32                       ipGroupPrefixLen,
    GT_IPADDR                    *ipSrcPtr,
    GT_U32                       ipSrcPrefixLen
)

{
    GT_STATUS   status = GT_OK;
    CPSS_DXCH_IP_LTT_ENTRY_STC routeLttEntryPtr;
    GT_U32 tcamGroupRowIndexPtr = 0;
    GT_U32 tcamGroupColumnIndexPtr = 0;
    GT_U32 tcamSrcRowIndexPtr = 0;
    GT_U32 tcamSrcColumnIndexPtr = 0;
    cpssOsMemSet(&routeLttEntryPtr, 0, sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));

    status = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId, ipGroupPtr,
                                            ipGroupPrefixLen,
                                            ipSrcPtr, ipSrcPrefixLen, &routeLttEntryPtr,
                                            &tcamGroupRowIndexPtr, &tcamGroupColumnIndexPtr,
                                            &tcamSrcRowIndexPtr, &tcamSrcColumnIndexPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "cpssDxChIpLpmIpv4McEntrySearch Failed :%d ", status);
        return status;
    }
    return status;
}

GT_STATUS cpssHalUpdtIntfIpMcRoutingEn(GT_U8 cpssDevId, GT_U32 cpssPortNum,
                                       CPSS_IP_PROTOCOL_STACK_ENT protocol, GT_BOOL enable)
{
    GT_STATUS cpssStatus = GT_OK;

    /* Enable Multicast IPv4 Routing on port */
    cpssStatus = cpssDxChIpPortRoutingEnable(cpssDevId, cpssPortNum,
                                             CPSS_IP_MULTICAST_E, protocol, enable);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to enable MC routing on port %d\n", cpssPortNum);
        return (cpssStatus);
    }
    return cpssStatus;
}

GT_STATUS cpssHalL2MLLPairWrite(int devId,
                                GT_U32 mllPairHwIdx,
                                CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT mllPairWriteForm,
                                CPSS_DXCH_L2_MLL_PAIR_STC *mllPairEntryPtr)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChL2MllPairWrite(devNum, mllPairHwIdx, mllPairWriteForm,
                                        mllPairEntryPtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChL2MllPairWrite  Failed :%d ", status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalL2MllLttEntrySet(int devId,
                                  GT_U32 mllPairHwIdx,
                                  CPSS_DXCH_L2_MLL_LTT_ENTRY_STC *lttEntryPtr)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChL2MllLttEntrySet(devNum, mllPairHwIdx, lttEntryPtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChL2MllLttEntrySet Failed :%d ", status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalL2MllLookupMaxVidxIndexGet
(
    GT_U8    devNum,
    GT_U32   *maxVidxIndexPtr
)
{
    GT_STATUS   status = GT_OK;
    status = cpssDxChL2MllLookupMaxVidxIndexGet(devNum, maxVidxIndexPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChL2MllLookupMaxVidxIndexGet Failed :%d ", status);
        return status;
    }
    return status;
}

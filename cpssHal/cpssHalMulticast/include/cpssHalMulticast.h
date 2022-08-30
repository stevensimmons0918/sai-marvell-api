/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* cpssHalMulticast.h
*
* DESCRIPTION:
*       initialize system
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __cpssHalMulticast__
#define __cpssHalMulticast__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/l2mll/cpssDxChL2Mll.h>

GT_STATUS cpssHalIpMllBridgeEnable(int devId,
                                   GT_BOOL enable);


GT_STATUS cpssHalBrgMcMemberAdd
(
    GT_U8 cpssDevId,
    GT_U16 vidx,
    GT_PHYSICAL_PORT_NUM cpssPortNum
);

GT_STATUS cpssHalBrgMcMemberDelete
(
    GT_U8 cpssDevId,
    GT_U16 vidx,
    GT_PHYSICAL_PORT_NUM cpssPortNum
);

GT_STATUS cpssHalIpMLLPairWrite
(
    int devId,
    GT_U32 mllPairHwIdx,
    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT mllPairWriteForm,
    CPSS_DXCH_IP_MLL_PAIR_STC
    *mllPairEntryPtr
);

GT_STATUS cpssHalIpMLLPairRead
(
    int devId,
    GT_U32 mllPairHwIdx,
    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT mllPairWriteForm,
    CPSS_DXCH_IP_MLL_PAIR_STC
    *mllPairEntryPtr
);

GT_STATUS cpssHalIpMcRouteEntriesRead
(
    int devId,
    GT_U32 routeEntryIndex,
    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr
);
GT_STATUS cpssHalIpMcRouteEntriesWrite
(
    int devId,
    GT_U32 routeEntryIndex,
    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr
);

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
);

GT_STATUS cpssHalIpLpmIpv4McEntryDel
(
    GT_U32                       lpmDBId,
    GT_U32                       vrId,
    GT_IPADDR                    *ipGroupPtr,
    GT_U32                       ipGroupPrefixLen,
    GT_IPADDR                    *ipSrcPtr,
    GT_U32                       ipSrcPrefixLen
);

GT_STATUS cpssHalBrgMcGroupDelete(int devId,
                                  GT_U16 vidx);

GT_STATUS cpssHalIpLpmIpv4McEntrySearch
(
    GT_U32                       lpmDBId,
    GT_U32                       vrId,
    GT_IPADDR                    *ipGroupPtr,
    GT_U32                       ipGroupPrefixLen,
    GT_IPADDR                    *ipSrcPtr,
    GT_U32                       ipSrcPrefixLen
);

GT_STATUS cpssHalUpdtIntfIpMcRoutingEn(GT_U8 cpssDevId,
                                       GT_U32 cpssPortNum,
                                       CPSS_IP_PROTOCOL_STACK_ENT protocol,
                                       GT_BOOL enable);

GT_STATUS cpssHalL2MLLPairWrite(int devId,
                                GT_U32 mllPairHwIdx,
                                CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT mllPairWriteForm,
                                CPSS_DXCH_L2_MLL_PAIR_STC *mllPairEntryPtr);

GT_STATUS cpssHalL2MllLttEntrySet(int devId,
                                  GT_U32 mllPairHwIdx,
                                  CPSS_DXCH_L2_MLL_LTT_ENTRY_STC *lttEntryPtr);

GT_STATUS cpssHalL2MllLookupMaxVidxIndexGet
(
    GT_U8    devNum,
    GT_U32   *maxVidxIndexPtr
);

GT_STATUS cpssHalL2MLLPairRead(int devId,
                               GT_U32 mllPairHwIdx,
                               CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT mllPairReadForm,
                               CPSS_DXCH_L2_MLL_PAIR_STC *mllPairEntryPtr);

#ifdef __cplusplus
}
#endif

#endif

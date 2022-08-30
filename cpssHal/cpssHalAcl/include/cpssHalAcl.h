/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/*******************************************************************************
* @file cpssHalAcl.h
*
* @brief Internal header which defines API for helpers functions, which are
*        specific for XPS ACL.
*
* @version   01
*******************************************************************************/

#ifndef _cpssHalAcl_h_
#define _cpssHalAcl_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <gtOs/gtGenTypes.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include "cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h"

GT_STATUS cpssHalAclSetEgressAclPacketType
(
    IN GT_U32  devId,
    IN GT_U32  portNum,
    CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT pktType,
    IN GT_BOOL enable
);

GT_STATUS cpssHalAclEnablePortIngressAcl
(
    IN GT_U32  devId,
    IN GT_U32  portNum,
    IN GT_BOOL enable
);

GT_STATUS cpssHalAclSetPortLookupCfgTabAccessMode
(
    IN GT_U32                                         devNum,
    IN GT_U32                                         portNum,
    IN CPSS_PCL_DIRECTION_ENT                         direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT                     lookupNum,
    IN GT_U32                                         subLookupNum,
    IN CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT mode
);

GT_STATUS cpssHalAclSetPclCfgTblEntry
(
    IN GT_U32                       devNum,
    IN CPSS_INTERFACE_INFO_STC      *interfaceInfoPtr,
    IN CPSS_PCL_DIRECTION_ENT       direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT   lookupNum,
    IN CPSS_DXCH_PCL_LOOKUP_CFG_STC *lookupCfgPtr
);

GT_STATUS cpssHalAclInit
(
    IN GT_U32               devId
);

GT_STATUS cpssHalAclIngressPolicyEnable
(
    IN GT_U32               devId,
    IN GT_BOOL              enable
);

GT_STATUS cpssHalAclEgressPolicyEnable
(
    IN GT_U32               devId,
    IN GT_BOOL              enable
);

GT_STATUS cpssHalAclPortListEnable
(
    IN GT_U32               devId,
    IN GT_BOOL              enable
);

GT_STATUS cpssHalAclPortListPortMappingSet(

    IN  GT_U32                  devId,
    IN  CPSS_PCL_DIRECTION_ENT  direction,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN  GT_U32                  group,
    IN  GT_U32                  offset
);

GT_STATUS cpssHalPclLookupCfgPortListEnableSet
(
    IN  GT_U32                        devId,
    IN  CPSS_PCL_DIRECTION_ENT        direction,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN  GT_U32                        subLookupNum,
    IN  GT_BOOL                       enable
);


GT_STATUS cpssHalPclUserDefinedByteSet
(
    IN GT_U32                               devId,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN CPSS_PCL_DIRECTION_ENT               direction,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
);


GT_STATUS cpssHalPclUserDefinedBytesSelectSet
(
    IN  GT_U32                               devId,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT           lookupNum,
    IN  CPSS_DXCH_PCL_UDB_SELECT_STC         *udbSelectPtr
);

GT_STATUS cpssHalPclUdeEtherTypeSet
(
    IN  GT_U32          devId,
    IN  GT_U32          index,
    IN  GT_U32          ethType
);

GT_STATUS cpssHalPclCounterEnable
(
    IN  GT_U32          devId,
    IN  GT_BOOL         enable
);

GT_STATUS cpssHalPclCounterBlockConfigure
(
    IN  GT_U32                      devId,
    IN  GT_U32                      cncPacketCntrBlock,
    IN  GT_U32                      cncByteCntrBlock,
    IN  CPSS_DXCH_CNC_CLIENT_ENT    cncClient,
    IN  GT_BOOL                     enable
);

GT_STATUS cpssHalPclTunnelTermForceVlanModeEnableSet
(
    IN GT_U32               devId,
    IN GT_BOOL              enable
);

GT_STATUS cpssHalPclPortIngressPolicyEnable
(
    IN GT_U32               devId,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL              enable
);

#ifdef __cplusplus
}
#endif

#endif //_cpssHalAcl_h_

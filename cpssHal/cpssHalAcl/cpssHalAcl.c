/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/*******************************************************************************
* @file cpssHalAcl.c
*
* @brief Private API implementation for CPSS ACL feature which can be used in
*        XPS layer.
*
* @version   01
*******************************************************************************/

#include "cpssHalAcl.h"
#include "xpsCommon.h"
#include "cpssHalDevice.h"
#include "cpssHalCounter.h"

GT_STATUS cpssHalAclInit
(
    IN GT_U32               devId
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    cpssDevId = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        if ((rc = cpssDxChPclInit(cpssDevId)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChPclInit failed with err : %d on device :%d\n ", rc, cpssDevId);
            return rc;
        }
    }
    return GT_OK;
}

GT_STATUS cpssHalAclSetEgressAclPacketType
(
    IN GT_U32  devId,
    IN GT_U32  portNum,
    CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT pktType,
    IN GT_BOOL enable
)
{
    GT_STATUS status = GT_OK;
    GT_U32    cpssDevId = 0;
    GT_U32    cpssPortNum = 0;

    /* Convert global port number into local (dev, port) */
    cpssDevId   = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    /*SetEgress packetType*/
    status = cpssDxCh2EgressPclPacketTypesSet(cpssDevId, cpssPortNum, pktType,
                                              enable);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set packet type for EPCL, "
              "rc:%d, dev:%d, enable:%d\n", status, cpssDevId, enable);
        return status;
    }

    return GT_OK;
}

GT_STATUS cpssHalAclIngressPolicyEnable
(
    IN GT_U32               devId,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    cpssDevId = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        /* Enable Ingress PCL */
        if ((rc = cpssDxChPclIngressPolicyEnable(cpssDevId, GT_TRUE)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChPclIngressPolicyEnable failed with err : %d on device :%d\n ", rc,
                  cpssDevId);
            return rc;
        }
    }

    return GT_OK;
}


GT_STATUS cpssHalAclEgressPolicyEnable
(
    IN GT_U32               devId,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    cpssDevId = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        /* Enable Egress PCL */
        if ((rc = cpssDxCh2PclEgressPolicyEnable(cpssDevId, enable)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxCh2PclEgressPolicyEnable failed with err : %d on device :%d\n ", rc,
                  cpssDevId);
            return rc;
        }
    }

    return GT_OK;
}


GT_STATUS cpssHalAclPortListEnable
(
    IN GT_U32               devId,
    IN GT_BOOL              enable
)
{

    GT_STATUS rc = GT_OK;
    GT_U32    cpssDevId = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        /* Enable PortList Grouping for ingress  */
        if ((rc = cpssDxChPclPortListGroupingEnableSet(cpssDevId,
                                                       CPSS_PCL_DIRECTION_INGRESS_E, enable)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChPclPortListGroupingEnableSet for ingress failed with err : %d on device :%d\n ",
                  rc, cpssDevId);
            return rc;
        }

        // enabling port list in look ip cfg table for ingress

        if ((rc = cpssDxChPclLookupCfgPortListEnableSet(cpssDevId,
                                                        CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_NUMBER_0_E, 0, enable)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChPclLookupCfgPortListEnableSet for ingress failed with err : %d on device :%d\n ",
                  rc, cpssDevId);
            return rc;
        }

        if ((rc = cpssDxChPclLookupCfgPortListEnableSet(cpssDevId,
                                                        CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_NUMBER_1_E, 0, enable)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChPclLookupCfgPortListEnableSet for ingress failed with err : %d on device :%d\n ",
                  rc, cpssDevId);
            return rc;
        }

        /* Enable PortList Grouping for egress  */
        if ((rc = cpssDxChPclPortListGroupingEnableSet(cpssDevId,
                                                       CPSS_PCL_DIRECTION_EGRESS_E, enable)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChPclPortListGroupingEnableSet for egress failed with err : %d on device :%d\n ",
                  rc, cpssDevId);
            return rc;
        }

        // enabling port list in look ip cfg table for ingress
        if ((rc = cpssDxChPclLookupCfgPortListEnableSet(cpssDevId,
                                                        CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0, enable)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChPclLookupCfgPortListEnableSet for egress failed with err : %d on device :%d\n ",
                  rc, cpssDevId);
            return rc;
        }
    }

    return GT_OK;

}

GT_STATUS cpssHalAclPortListPortMappingSet(

    IN  GT_U32                  devId,
    IN  CPSS_PCL_DIRECTION_ENT  direction,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN  GT_U32                  group,
    IN  GT_U32                  offset
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    cpssDevId = 0;


    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        /* Set port list - port mapping */
        if ((rc = cpssDxChPclPortListPortMappingSet(cpssDevId, direction, portNum,
                                                    enable, group, offset)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChPclPortListPortMappingSet failed with err : %d on device :%d\n ", rc,
                  cpssDevId);
            return rc;
        }
    }

    return GT_OK;

}

GT_STATUS cpssHalAclEnablePortIngressAcl
(
    IN GT_U32  devId,
    IN GT_U32  portNum,
    IN GT_BOOL enable
)
{
    GT_STATUS status = GT_OK;
    GT_U32    cpssDevId = 0;
    GT_U32    cpssPortNum = 0;

    /* Convert global port number into local (dev, port) */
    cpssDevId   = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    /* Enables/disables ingress policy per port */
    status = cpssDxChPclPortIngressPolicyEnable(cpssDevId, cpssPortNum, enable);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to enable port ingress policy, "
              "rc:%d, dev:%d, enable:%d\n", status, cpssDevId, enable);
        return status;
    }

    return GT_OK;
}

GT_STATUS cpssHalAclSetPortLookupCfgTabAccessMode
(
    IN GT_U32                                         devId,
    IN GT_U32                                         portNum,
    IN CPSS_PCL_DIRECTION_ENT                         direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT                     lookupNum,
    IN GT_U32                                         subLookupNum,
    IN CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT mode
)
{
    GT_STATUS status = GT_OK;
    GT_U32    cpssDevId = 0;
    GT_U32    cpssPortNum = 0;

    /* Convert global port number into local (dev, port) */
    cpssDevId   = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    status = cpssDxChPclPortLookupCfgTabAccessModeSet(cpssDevId, cpssPortNum,
                                                      direction, lookupNum, subLookupNum, mode);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set port lookup cfg table access mode, "
              "rc:%d, dev:%d, port:%d, direction:%d, lookupNum:%d, subLookupNum:%d, mode:%d\n",
              status, cpssDevId, cpssPortNum, direction, lookupNum, subLookupNum, mode);
        return status;
    }

    return GT_OK;
}

GT_STATUS cpssHalAclSetPclCfgTblEntry
(
    IN GT_U32                          devId,
    IN CPSS_INTERFACE_INFO_STC         *interfaceInfoPtr,
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_DXCH_PCL_LOOKUP_CFG_STC    *lookupCfgPtr
)
{
    GT_STATUS status = GT_OK;
    GT_U32    cpssDevId = 0;

    if (interfaceInfoPtr->type == CPSS_INTERFACE_PORT_E)
    {
        cpssDevId = interfaceInfoPtr->devPort.hwDevNum;
        status = cpssDxChPclCfgTblSet(cpssDevId, interfaceInfoPtr, direction, lookupNum,
                                      lookupCfgPtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set pcl cfg table entry, "
                  "rc:%d, dev:%d\n", status, cpssDevId);
            return status;
        }
    }
    else
    {
        XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
        {
            status = cpssDxChPclCfgTblSet(cpssDevId, interfaceInfoPtr, direction, lookupNum,
                                          lookupCfgPtr);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set pcl cfg table entry, "
                      "rc:%d, dev:%d\n", status, cpssDevId);
                return status;
            }
        }
    }

    return GT_OK;
}

GT_STATUS cpssHalPclUserDefinedByteSet
(
    IN GT_U32                               devId,
    IN CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN CPSS_PCL_DIRECTION_ENT               direction,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    cpssDevId = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        if ((rc = cpssDxChPclUserDefinedByteSet(cpssDevId, ruleFormat, packetType,
                                                direction, udbIndex, offsetType, offset)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChPclUserDefinedByteSet"
                  " failed with err : %d on device :%d\n ", rc, cpssDevId);
            return rc;
        }
    }

    return GT_OK;
}

GT_STATUS cpssHalPclUserDefinedBytesSelectSet
(
    IN  GT_U32                               devId,
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT           lookupNum,
    IN  CPSS_DXCH_PCL_UDB_SELECT_STC         *udbSelectPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    cpssDevId = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        if ((rc = cpssDxChPclUserDefinedBytesSelectSet(cpssDevId, ruleFormat,
                                                       packetType,
                                                       lookupNum, udbSelectPtr)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChPclUserDefinedBytesSelectSet"
                  " failed with err : %d on device :%d\n ", rc, cpssDevId);
            return rc;
        }
    }

    return GT_OK;
}

GT_STATUS cpssHalPclUdeEtherTypeSet
(
    IN  GT_U32          devId,
    IN  GT_U32          index,
    IN  GT_U32          ethType
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    cpssDevId = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        if ((rc = cpssDxChPclUdeEtherTypeSet(cpssDevId, index, ethType)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " cpssHalPclUdeEtherTypeSet"
                  " failed with err : %d on device :%d\n ", rc, cpssDevId);
            return rc;
        }
    }

    return GT_OK;
}

GT_STATUS cpssHalPclCounterEnable
(
    IN  GT_U32          devId,
    IN  GT_BOOL         enable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    cpssDevId = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        if ((rc = cpssDxChCncCountingEnableSet(cpssDevId,
                                               CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PCL_E, GT_TRUE)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChCncCountingEnableSet failed with err : %d on device :%d\n ", rc,
                  cpssDevId);
            return rc;
        }

        if ((rc = cpssDxChCncCounterClearByReadEnableSet(cpssDevId, GT_TRUE)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChCncCounterClearByReadEnableSet failed with err : %d on device :%d\n ",
                  rc, cpssDevId);
            return rc;
        }
    }

    return GT_OK;
}
GT_STATUS cpssHalPclCounterBlockConfigure
(
    IN  GT_U32                      devId,
    IN  GT_U32                      cncPacketCntrBlock,
    IN  GT_U32                      cncByteCntrBlock,
    IN  CPSS_DXCH_CNC_CLIENT_ENT    cncClient,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    index = cncPacketCntrBlock - XPS_ACL_CNC_PKT_COUNTER_BLK_START;

    if ((rc = cpssHalCncBlockClientEnableAndBindSet(devId, cncPacketCntrBlock,
                                                    cncClient, enable, index,
                                                    CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalCncBlockClientEnableAndBindSet failed with err : %d on device :%d\n ",
              rc, devId);
        return rc;
    }

    if ((rc = cpssHalCncBlockClientEnableAndBindSet(devId, cncByteCntrBlock,
                                                    cncClient, enable, index,
                                                    CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalCncBlockClientEnableAndBindSet failed with err : %d on device :%d\n ",
              rc, devId);
        return rc;
    }

    return rc;
}

GT_STATUS cpssHalPclTunnelTermForceVlanModeEnableSet
(
    IN GT_U32               devId,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    cpssDevId = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        /* Enable Ingress PCL */
        if ((rc = cpssDxCh3PclTunnelTermForceVlanModeEnableSet(cpssDevId,
                                                               enable)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " PclTunnelTermForceVlanMode failed with err : %d on device :%d\n ", rc,
                  cpssDevId);
            return rc;
        }
    }

    return GT_OK;
}


GT_STATUS cpssHalPclPortIngressPolicyEnable
(
    IN GT_U32               devId,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    cpssDevId = 0;

    /*NOTE: This can be called with ePort. */
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        /* Enable Ingress PCL */
        if ((rc = cpssDxChPclPortIngressPolicyEnable(cpssDevId, portNum,
                                                     enable)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChPclPortIngressPolicyEnable failed : %d on device :%d\n ", rc,
                  cpssDevId);
            return rc;
        }
    }

    return GT_OK;
}

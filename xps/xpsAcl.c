// file xpsAcl.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsAcl.h"
#include "xpsLock.h"
#include "xpsScope.h"
#include "xpsAllocator.h"
#include "xpsGlobalSwitchControl.h"
#include "xpsLag.h"
#include "xpsPort.h"
#include "openXpsLag.h"

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>
#include "cpssHalDevice.h"
#include "cpssHalCounter.h"
#include "cpssHalAcl.h"
#include "cpssHalTcam.h"
#include "cpssHalUtil.h"
#include "cpssHalCtrlPkt.h"
#include "cpssHalMirror.h"
#include "cpssHalPha.h"
#include "cpssHalPort.h"
#include "cpssHalQos.h"
#include "cpssHalVlan.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

static uint8_t cncBlockListBound[XPS_ACL_CNC_BLOCK_COUNT];

static uint16_t cncPerBlockCount[XPS_ACL_CNC_BLOCK_COUNT];

static xpsDbHandle_t aclTableIdDbHndl = XPS_STATE_INVALID_DB_HANDLE;

static xpsDbHandle_t aclTableEntryDbHndl = XPS_STATE_INVALID_DB_HANDLE;

static xpsDbHandle_t aclTableVtcamMapDbHndl = XPS_STATE_INVALID_DB_HANDLE;

static xpsDbHandle_t aclCounterIdMapDbHndl = XPS_STATE_INVALID_DB_HANDLE;

static bool xpsAclDropEgressPacketsInfo[MAX_PORTNUM];

XP_STATUS xpsAclUdbKeysInitScope(xpsScope_t scopeId);

XP_STATUS xpsAclUdbKeysForNonIpInit(xpsDevice_t devNum, uint32_t direction,
                                    uint32_t lookupNum);
/*********************************************** NON-IP KEY FORMAT *************************************************************
* ------------------------------------------------------------------------------------------------------------------------------
* Key Index   | 0...3     |    4...5    |6...7|   8...9   |      10           |   11      | 12  |  13...18   |   19...24       |
* ------------------------------------------------------------------------------------------------------------------------------
* Key Details | Port List | Source ePort| VID | Ethertype |  Applicable Flow  | Reserved  | UP0 | MAC Source | MAC Destination |
*             |           |             |     |           |    Sub-Template   |           |     |   Address  |    Address      |
* ------------------------------------------------------------------------------------------------------------------------------
********************************************************************************************************************************/

XP_STATUS xpsAclUdbKeysForIpv4Init(xpsDevice_t devNum, uint32_t direction,
                                   uint32_t lookupNum, uint32_t packetType);
/**************************************************** IPv4 KEY FORMAT *****************************************************************
* -------------------------------------------------------------------------------------------------------------------------------------
* Key    | 0...3 |4...5 |6...7| 8...9 |      10        |    11      | 12|13...16|17...20| 21 |   22   |23...24|25...26|  27 | 28 | 29 |
* Index  |       |      |     |       |                |            |   |       |       |    |        |       |       |     |    |    |
* -------------------------------------------------------------------------------------------------------------------------------------
* Key    | Port  |Src   | VID |QoS    | Applicable Flow|IP Protocol |UP0| SIPv4 | DIPv4 |DSCP|L4 Valid|L4 Src |L4 Dst | TCP |ICMP|ICMP|
* Details| List  |ePort |     |Profile|  Sub-Template  |   Number   |   |       |       |    |        |  Port |  Port |Flags|type|code|
* -------------------------------------------------------------------------------------------------------------------------------------
***************************************************************************************************************************************/

XP_STATUS xpsAclUdbKeysForIpv6Init(xpsDevice_t devNum, uint32_t direction,
                                   uint32_t lookupNum, uint32_t packetType);
/************************************************ IPv6 KEY FORMAT ************************************************************
* ----------------------------------------------------------------------------------------------------------------------------
* Key    | 0...3 |   4   |      5        |   6  | 7 | 8...23|24...39|   40   |41...42|43...44|  45 | 46 | 47 |UDB60_FIXED_STC|
* Index  |       |       |               |      |   |       |       |        |       |       |     |    |    |               |
* ----------------------------------------------------------------------------------------------------------------------------
* Key    | Port  |Trg Phy|Applicable Flow|Next  |UP0| SIPv6 | DIPv4 |L4 Valid|L4 Src |L4 Dst | TCP |ICMP|ICMP|Src Port, vid  |
* Details| List  | Port  | Sub-Template  |Header|   |       |       |        |  Port |  Port |Flags|type|code|               |
* ----------------------------------------------------------------------------------------------------------------------------
******************************************************************************************************************************/

XP_STATUS xpsAclUdbKeysForArpInit(xpsDevice_t devNum, uint32_t direction,
                                  uint32_t lookupNum);
/*********************************************** ARP KEY FORMAT ********************************************************
* ----------------------------------------------------------------------------------------------------------------------
* Key Index   | 0...3     |    4...5    | 6...7 |   8...9   |      10          |11...12 |  13...18   |   19...24       |
* ----------------------------------------------------------------------------------------------------------------------
* Key Details | Port List | Source ePort|  VID  | Reserved  |  Applicable Flow | Opcode | MAC Source | MAC Destination |
*             |           |             |       |           |   Sub-Template   |        |   Address  |    Address      |
* ----------------------------------------------------------------------------------------------------------------------
*************************************************************************************************************************/

XP_STATUS xpsAclUdbKeysForVxlanIpv6Init(xpsDevice_t devNum);
/*********************************************** Vxlan Ipv6 KEY FORMAT ********************************************************
* ------------------------------------------------
* Key Index   | 0...1     |    2...4    | 5...20 |
* ------------------------------------------------
* Key Details | PCL ID    |    VNI ID   |  SIP   |
*             |           |             |        |
* ------------------------------------------------
*************************************************************************************************************************/

static XP_STATUS xpsAclCounterBlockClientSet(xpsDevice_t devId,
                                             uint32_t parallelLookupNum,
                                             uint32_t packetCntrBlockId, uint32_t byteCntrBlockId, bool enable);
/* compare function for table id DB
*/
static int32_t aclTableIdCompare(void *key, void *data)
{
    uint32_t value1 = *((uint32_t *)key);
    xpsAclTableContextDbEntry* value2 = (xpsAclTableContextDbEntry*)data;
    return ((value1) - (value2->tableId));
}


/* compare function for table Entry DB
*/
static int32_t aclTableEntryCompare(void *key, void *data)
{
    uint32_t value1 = *((uint32_t *)key);
    xpsAclEntryContextDbEntry* value2 = (xpsAclEntryContextDbEntry*)data;
    return ((value1) - (value2-> key));
}


/* compare function for table id DB
*/
static int32_t aclVtcamMappingCompare(void *key, void *data)
{
    uint32_t value1 = *((uint32_t *)key);
    xpsAclTableVtcamMapContextDbEntry* value2 = (xpsAclTableVtcamMapContextDbEntry*)
                                                data;
    return ((value1) - (value2->groupId));
}

/* compare function for counter id DB
*/
static int32_t aclCounterIdMappingCompare(void *key, void *data)
{
    uint32_t value1 = *((uint32_t *)key);
    xpsAclCounterIdMappingContextDbEntry* value2 =
        (xpsAclCounterIdMappingContextDbEntry*)data;
    return ((value1) - (value2->globalCounterId));
}

void xpsAclDropEgressPacketsParamSet(xpsDevice_t devId, uint32_t portNum,
                                     bool enabled)
{
    xpsAclDropEgressPacketsInfo[portNum] = enabled;
}

XP_STATUS xpsAclInit(void)
{
    return xpsAclInitScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsAclDeInit(void)
{

    return xpsAclDeInitScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsAclAddCnpRateLimitRule(xpsDevice_t devId, uint32_t portNum)
{
    if (cpssHalIsEbofDevice(devId))
    {
        GT_STATUS rc4, rc6;
        rc4 = cpssHalCtrlPktCnpOnPort(devId, portNum, CPSS_HAL_IP_IPV4);
        rc6 = cpssHalCtrlPktCnpOnPort(devId, portNum, CPSS_HAL_IP_IPV6);
        if (rc4 != GT_OK || rc6 != GT_OK)
        {
            return XP_ERR_FAILED;
        }
    }
    return XP_NO_ERR;
}

extern uint32_t ctrlPacketEgressIpv4AclTableId;
extern uint32_t ctrlPacketEgressIpv6AclTableId;
extern uint32_t ctrlPacketIpv4AclTableId;
extern uint32_t ctrlPacketIpv6AclTableId;

static GT_STATUS cpssHalCtrlPktFabricoErrorPacketDrop
(
    uint32_t devId,
    xpsAclErrorEgressDropMatch dropType
)
{
    XP_STATUS ret =XP_NO_ERR;
    bool            is_control_acl = false, isIpv4Type = false;
    uint16_t        value = 0;
    uint16_t        mask  = 0;
    uint8_t         i     = 0;
    GT_U32 maxKeyFlds;
    GT_U32 tableId;
    xpsAclkeyFieldList_t       aclFieldData;
    xpsPclAction_t             aclEntryData;
    uint32_t                     vtcamId;
    xpsAclTableContextDbEntry_t  tableIdDbKey;
    xpsAclTableContextDbEntry_t *tableIdDbEntry = NULL;
    xpsAclKeyFormat              keyFormat;
    uint32_t  portGroup = 0;
    int32_t   portNum  = 0;
    uint8_t   maxPorts = 0;
    static bool portListUpdated = false;
    static xpsAclTableContextDbEntry_t  tmpTableInfo;
    /* add 1 for port group bit map */
    maxKeyFlds = 6+1+1;
    uint32_t offset = 0, portOffset = 0;
    uint32_t ruleId = 0;
    GT_U32 cpssPortNum;
    xpsScope_t scopeId;

    if ((ret = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return ret;
    }
    xpsAclkeyField_t           aclFieldList[maxKeyFlds];
    uint8_t                    iacl_value[maxKeyFlds];
    uint8_t                    iacl_mask[maxKeyFlds];

    if ((ret = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XPS_ALLOCATOR_ACL_TABLE_ID,
                                      &ruleId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsAllocatorAllocateId failed with err : %d\n ", ret);
        return GT_FAIL;
    }

    switch (dropType)
    {
        case XPS_FABRICO_ERROR_EGRESS_DROP_61_IPV4_MATCH_E:
            isIpv4Type = TRUE;
        /* fallthrough */
        case XPS_FABRICO_ERROR_EGRESS_DROP_61_IPV6_MATCH_E:
            offset = 0;
            break;
        case XPS_FABRICO_ERROR_EGRESS_DROP_62_IPV4_MATCH_E:
            isIpv4Type = TRUE;
        /* fallthrough */
        case XPS_FABRICO_ERROR_EGRESS_DROP_62_IPV6_MATCH_E:
            offset = 1;
            break;
        case XPS_FABRICO_ERROR_EGRESS_DROP_63_IPV4_MATCH_E:
            isIpv4Type = TRUE;
        /* fallthrough */
        case XPS_FABRICO_ERROR_EGRESS_DROP_63_IPV6_MATCH_E:
            offset = 2;
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: unknown dropType %d\n", dropType);
    }

    if (isIpv4Type)
    {
        tableId = ctrlPacketEgressIpv4AclTableId;
        vtcamId = XPS_L3_EGR_STG_TABLE_ID_0;
        keyFormat = XPS_PCL_IPV4_L4_KEY;
    }
    else
    {
        tableId = ctrlPacketEgressIpv6AclTableId;
        vtcamId = XPS_L3V6_EGR_STG_TABLE_ID_0;
        keyFormat = XPS_PCL_IPV6_L4_KEY;
    }
    tableIdDbKey.tableId = tableId;
    if (((ret = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                   (xpsDbKey_t)&tableIdDbKey,
                                   (void**)&tableIdDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", ret);
        return GT_FAIL;
    }

    if (tableIdDbEntry == NULL)
    {
        ret = XP_ERR_NULL_POINTER;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", ret);
        return GT_FAIL;
    }

    tableIdDbEntry->vtcamId[0] = vtcamId;
    tableIdDbEntry->numOfDuplicates = 1;

    if (!portListUpdated)
    {
        memset(&tmpTableInfo, 0x00, sizeof(tmpTableInfo));
        ret = xpsGlobalSwitchControlGetMaxPorts(devId, &maxPorts);
        if (ret != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get max physical port number. error:%d\n", ret);
            return GT_FAIL;
        }
        if (maxPorts > MAX_PORTNUM)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "maxPorts > MAX_PORTNUM. maxPorts:%d\n", maxPorts);
            return GT_FAIL;
        }
        XPS_GLOBAL_PORT_ITER(portNum, maxPorts)
        {
            cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

            portGroup = (cpssPortNum)/XPS_PORT_LIST_OFFSET;
            portOffset = (0x1)<<((cpssPortNum)%(XPS_PORT_LIST_OFFSET));

            if (portGroup >= XPS_PORT_LIST_PORT_GROUP_MAX)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Invalid port Num : %d \n ", portNum);
                return GT_FAIL;
            }

            if (xpsAclDropEgressPacketsInfo[portNum])
            {
                tmpTableInfo.portGroupBmp[portGroup]++;
                tmpTableInfo.portListBmp[portGroup] |= ((portGroup << XPS_PORT_LIST_OFFSET)|
                                                        portOffset);
                tmpTableInfo.bindCount++;
            }
        }
        portListUpdated = true;
    }

    if (tmpTableInfo.bindCount == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "skip adding rule, no ports to apply the egress drop rule\n");
        return GT_OK;
    }

    for (portGroup = 0; portGroup < XPS_PORT_LIST_PORT_GROUP_MAX; portGroup++)
    {
        if (tmpTableInfo.portGroupBmp[portGroup] != 0)
        {
            tableIdDbEntry->portGroupBmp[portGroup] = tmpTableInfo.portGroupBmp[portGroup];
            tableIdDbEntry->portListBmp[portGroup] = tmpTableInfo.portListBmp[portGroup];
        }
    }
    tableIdDbEntry->bindCount = tmpTableInfo.bindCount;

    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));
    memset(&aclFieldList, 0x00, sizeof(aclFieldList));
    memset(iacl_value, 0x00, sizeof(iacl_value));
    memset(iacl_mask, 0x00, sizeof(iacl_mask));
    /* check if needed hard drop */
    aclEntryData.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    aclEntryData.actionStop = GT_FALSE;
    aclEntryData.mirror.cpuCode = XPS_FABRICO_ERROR_EGRESS_DROP_REASON_CODE;

    for (i =0; i < maxKeyFlds; i++)
    {
        aclFieldList[i].value = &iacl_value[i];
        aclFieldList[i].mask  = &iacl_mask[i];
    }
    /* needed for place holder to update ports bit map */
    maxKeyFlds--;

    aclFieldData.fldList = aclFieldList;
    aclFieldData.numFlds = maxKeyFlds;
    aclFieldData.isValid = 1;

    value = CPSS_HAL_CNP_ROCEV2_UDP_DESTINATION_PORT_CNS;    //UDP destination port 4791 (ROCEv2)
    mask  = 0xFFFF;
    uint16_t valuebyte0 = 0;
    uint16_t maskbyte0 = 0;
    valuebyte0 = ((value & 0xFF00) >> 8);
    maskbyte0  = ((mask & 0xFF00) >> 8);
    i = 0;
    aclFieldData.fldList[i].keyFlds = XPS_PCL_L4_BYTE2;
    memcpy(aclFieldData.fldList[i].value, &valuebyte0, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[i].mask, &maskbyte0, sizeof(uint8_t));

    uint8_t valuebyte1 = 0;
    uint8_t maskbyte1 = 0;
    valuebyte1 = (value & 0x00FF);
    maskbyte1  = (mask & 0x00FF);
    i++;
    aclFieldData.fldList[i].keyFlds = XPS_PCL_L4_BYTE3;
    memcpy(aclFieldData.fldList[i].value, &valuebyte1, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[i].mask, &maskbyte1, sizeof(uint8_t));

    uint8_t isL4Value = 1;
    uint8_t isL4Mask = 0x1;
    i++;
    aclFieldData.fldList[i].keyFlds = XPS_PCL_IS_L4_VALID;
    memcpy(aclFieldData.fldList[i].value, &isL4Value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[i].mask, &isL4Mask, sizeof(uint8_t));

    i++;
    if (isIpv4Type)
    {
        value = 1;
        mask = 0x1;
        aclFieldData.fldList[i].keyFlds = XPS_PCL_IS_IPV4_UDP;
        memcpy(aclFieldData.fldList[i].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[i].mask, &mask, sizeof(uint8_t));

        value = 0x11;
        mask = 0xFF;
        i++;
        aclFieldData.fldList[i].keyFlds = XPS_PCL_ICMP_CODE;
        memcpy(aclFieldData.fldList[i].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[i].mask, &mask, sizeof(uint8_t));

        value = 0x61 + offset;
        mask = 0xFF;
        i++;
        aclFieldData.fldList[i].keyFlds = XPS_PCL_ICMP_MSG_TYPE;
        memcpy(aclFieldData.fldList[i].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[i].mask, &mask, sizeof(uint8_t));

    }
    else
    {
        value = 1;
        mask = 0x1;
        aclFieldData.fldList[i].keyFlds = XPS_PCL_IS_IPV6_UDP;
        memcpy(aclFieldData.fldList[i].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[i].mask, &mask, sizeof(uint8_t));

        value = 0x11;
        mask = 0xFF;
        i++;
        aclFieldData.fldList[i].keyFlds = XPS_PCL_ICMPV6_MSG_TYPE;
        memcpy(aclFieldData.fldList[i].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[i].mask, &mask, sizeof(uint8_t));

        value = 0x61 + offset;
        mask = 0xFF;
        i++;
        aclFieldData.fldList[i].keyFlds = XPS_PCL_ICMPV6_CODE;
        memcpy(aclFieldData.fldList[i].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[i].mask, &mask, sizeof(uint8_t));
    }

    ret =  xpsAclWriteEntry(devId, tableId,
                            0,
                            ruleId, &aclFieldData, aclEntryData,
                            keyFormat, is_control_acl);

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
          "INFO Rule xpsAclWriteEntry %s ruleId %d dropType %d ret %d\n",
          (keyFormat == XPS_PCL_IPV4_L4_KEY) ? "IPV4" : "IPV6", ruleId, dropType, ret);

    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsAclWriteEntry %s failed ruleId %d dropType %d ret %d\n",
              (keyFormat == XPS_PCL_IPV4_L4_KEY) ? "IPV4" : "IPV6", ruleId, dropType,
              ret);
        return GT_FAIL;
    }

    return GT_OK;
}


extern uint32_t ctrlPacketL3Ipv4CommonDropAclTableId;
extern uint32_t ctrlPacketL3Ipv6CommonDropAclTableId;

XP_STATUS xpsAclRouterCommonDropUpdate(xpsDevice_t devId,
                                       xpsInterfaceId_t  l3IntfId, bool enable)
{
    XP_STATUS retVal                 = XP_NO_ERR;
    xpsInterfaceId_t portInterfaceId = XPS_INTF_INVALID_ID;
    xpsDevice_t portDevId;
    xpPort_t portId;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum, i;
    xpsLagPortIntfList_t lagPortList;
    GT_STATUS cpssStatus = GT_OK;
    xpsVlan_t vlanId = 0;
    int port = 0;
    int maxTotalPorts = 0;
    CPSS_PORTS_BMP_STC                   globalPortMembers;
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_BOOL                              isValid = GT_FALSE;
    xpsScope_t scopeId;
    xpsInterfaceType_e intfType;

    memset(&portsMembers, 0, sizeof(portsMembers));
    memset(&portsTagging, 0, sizeof(portsTagging));
    memset(&vlanInfo, 0, sizeof(vlanInfo));
    memset(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));
    memset(&lagPortList, 0, sizeof(lagPortList));

    /* Get Scope Id from devId */
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return retVal;
    }

    retVal = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);

    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    if ((intfType != XPS_PORT_ROUTER) && (intfType != XPS_VLAN_ROUTER))
    {
        return XP_ERR_INVALID_PARAMS;
    }


    if (intfType == XPS_VLAN_ROUTER)
    {
        vlanId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Vlan Id : %d\n", vlanId);

        if ((vlanId <= XPS_VLANID_MIN) && (vlanId > XPS_VLANID_MAX))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Invalid vlan id %d maxVlan %d .", vlanId, XPS_VLANID_MAX);
            return XP_ERR_INVALID_VLAN_ID;
        }

        cpssHalGetMaxGlobalPorts(&maxTotalPorts);

        memset(&globalPortMembers, 0, sizeof(globalPortMembers));
        cpssStatus = cpssHalReadBrgVlanEntry(devId, (GT_U16)vlanId, &portsMembers,
                                             &portsTagging, &vlanInfo, &isValid,
                                             &portsTaggingCmd, &globalPortMembers);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Read vlan %d failed .",
                  vlanId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        /* Enabling UC routing on all member ports of the vlan*/
        XPS_GLOBAL_PORT_ITER(port, maxTotalPorts)
        {
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&globalPortMembers, port))
            {
                cpssDevId = xpsGlobalIdToDevId(devId, port);
                cpssPortNum = xpsGlobalPortToPortnum(devId, port);

                retVal = xpsAclCommonDropRuleUpdate(cpssDevId, cpssPortNum, enable);
                if (retVal != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed xpsAclCommonDropRuleUpdate for port intf : %d \n", cpssPortNum);
                    return xpsConvertCpssStatusToXPStatus(cpssStatus);
                }
            }
        }
    }
    else
    {
        retVal = xpsL3RetrieveInterfaceMapping(scopeId, l3IntfId, &portInterfaceId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port interface id for L3 interface: %d\n", l3IntfId);
            return retVal;
        }
        retVal = xpsInterfaceGetTypeScope(scopeId, portInterfaceId, &intfType);

        if (retVal != XP_NO_ERR)
        {
            return retVal;
        }
        if (intfType == XPS_PORT)
        {

            // Get the port interface id from L3 interface id
            lagPortList.portIntf[0] = portInterfaceId;
            lagPortList.size = 1;
        }
        else if (intfType == XPS_LAG)
        {
            if ((retVal = xpsLagGetPortIntfListScope(scopeId, portInterfaceId,
                                                     &lagPortList)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ports failed, lag interface(%d)", portInterfaceId);
                return retVal;
            }
        }
        for (i = 0; i < lagPortList.size; i++)
        {
            portInterfaceId = lagPortList.portIntf[i];

            // Get the port id and dev id from the port interface id
            // TODO: is this call required? Or XPS port db to be obsoleted?
            retVal = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portInterfaceId,
                                                          &portDevId, &portId);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get physical port id and device from the port interface %d \n",
                      portInterfaceId);
                return retVal;
            }

            cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
            cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

            retVal = xpsAclCommonDropRuleUpdate(cpssDevId, cpssPortNum, enable);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed xpsAclCommonDropRuleUpdate for port intf : %d \n", cpssPortNum);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }

    return retVal;
}

XP_STATUS xpsAclCommonDropRuleUpdate(xpsDevice_t devId, uint32_t portNum,
                                     bool enable)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsAclTableContextDbEntry_t             tableIdDbKey;
    xpsAclTableContextDbEntry_t             *tableIdDbEntry = NULL;
    xpsAclTableContextDbEntry_t             *tableIdV6DbEntry = NULL;
    uint32_t                                portGroup;
    uint32_t                                offset;
    GT_STATUS cpssRet = GT_OK;
    xpsScope_t                      scopeId=0;

    tableIdDbKey.tableId = ctrlPacketL3Ipv4CommonDropAclTableId;
    if (((retVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                      (xpsDbKey_t)&tableIdDbKey,
                                      (void**)&tableIdDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", retVal);
        return retVal;
    }

    if (tableIdDbEntry == NULL)
    {
        retVal = XP_ERR_NULL_POINTER;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", retVal);
        return retVal;
    }

    tableIdDbEntry->vtcamId[0] = XPS_L3_ING_STG_TABLE_ID_0;
    tableIdDbEntry->numOfDuplicates = 1;

    portGroup = (portNum)/XPS_PORT_LIST_OFFSET;
    offset = (0x1)<<((portNum)%(XPS_PORT_LIST_OFFSET));
    if (portGroup > (XPS_PORT_LIST_PORT_GROUP_MAX -1))
    {
        retVal = XP_ERR_INVALID_VALUE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "portGroup obtained from cpssPortNum is Invalid, err : %d portGroup: %d\n ",
              retVal, portGroup);
        return retVal;
    }

    //if adding a port to a portlist
    if (enable)
    {
        tableIdDbEntry->portGroupBmp[portGroup]++;
        tableIdDbEntry->portListBmp[portGroup] |= ((portGroup << XPS_PORT_LIST_OFFSET)|
                                                   offset);
        tableIdDbEntry->bindCount++;
    }
    //if removing a port from port list
    else
    {
        tableIdDbEntry->portListBmp[portGroup] &= (~offset);
    }

    tableIdDbKey.tableId = ctrlPacketL3Ipv6CommonDropAclTableId;
    if (((retVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                      (xpsDbKey_t)&tableIdDbKey,
                                      (void**)&tableIdV6DbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", retVal);
        return retVal;
    }

    if (tableIdV6DbEntry == NULL)
    {
        retVal = XP_ERR_NULL_POINTER;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", retVal);
        return retVal;
    }

    tableIdV6DbEntry->vtcamId[0] = XPS_L3V6_ING_STG_TABLE_ID_0;
    tableIdV6DbEntry->numOfDuplicates = 1;

    portGroup = (portNum)/XPS_PORT_LIST_OFFSET;
    offset = (0x1)<<((portNum)%(XPS_PORT_LIST_OFFSET));
    //if adding a port to a portlist
    if (enable)
    {
        tableIdV6DbEntry->portGroupBmp[portGroup]++;
        tableIdV6DbEntry->portListBmp[portGroup] |= ((portGroup << XPS_PORT_LIST_OFFSET)
                                                     |
                                                     offset);
        tableIdV6DbEntry->bindCount++;
    }
    //if removing a port from port list
    else
    {
        tableIdV6DbEntry->portListBmp[portGroup] &= (~offset);
    }

    cpssRet = xpsAclAddIngressCommonDropRules(devId);

    if (cpssRet != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAclAddIngressCommonDropRules failed %d\n", cpssRet);
        return xpsConvertCpssStatusToXPStatus(cpssRet);
    }

    /* Ensure that clean-up of last port happens in Acl Write Entry */
    if (enable == false)
    {
        if (tableIdDbEntry)
        {
            tableIdDbEntry->portGroupBmp[portGroup]--;
            tableIdDbEntry->bindCount--;
        }

        if (tableIdV6DbEntry)
        {
            tableIdV6DbEntry->portGroupBmp[portGroup]--;
            tableIdV6DbEntry->bindCount--;
        }
    }

    return retVal;
}


XP_STATUS xpsAclAddIngressCommonDropRules(xpsDevice_t devId)
{
    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;;
    ipv4Addr_t ipv4AddressPattern, ipv4AddressMask;
    ipv6Addr_t ipv6AddressPattern, ipv6AddressMask;
    static uint32_t sipRuleId = 0;
    static uint32_t dipRuleId = 0;

    memset(ipv4AddressMask, 0, sizeof(ipv4AddressMask));
    memset(ipv4AddressPattern, 0, sizeof(ipv4AddressPattern));
    memset(ipv6AddressPattern, 0, sizeof(ipv6AddressPattern));
    memset(ipv6AddressMask, 0, sizeof(ipv6AddressMask));

#if 1
    /* Do we need this rule */
    /* Install Sip Link-local ipv4 rule */
    memset(ipv4AddressMask, 0, sizeof(ipv4AddressMask));
    ipv4AddressPattern[0] = 169;
    ipv4AddressPattern[1] = 254;
    ipv4AddressMask[0] = 0xff;
    ipv4AddressMask[1] = 0xff;

    if (!sipRuleId)
    {
        retVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_ENTRY,
                                        &sipRuleId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Could not allocate ACL ruleId id, error %d\n", retVal);
            return retVal;
        }
    }

    rc = cpssHalCtrlCommonPacketDrop(devId, true, true, ipv4AddressPattern,
                                     ipv4AddressMask, NULL, NULL,
                                     sipRuleId);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed cpssHalCtrlCommonPacketDrop error:%d\n", rc);
        return XP_ERR_FAILED;
    }

    /* Do we need this rule */
    /* Install Dip Link-local ipv4 rule */
    memset(ipv4AddressMask, 0, sizeof(ipv4AddressMask));
    ipv4AddressPattern[0] = 169;
    ipv4AddressPattern[1] = 254;
    ipv4AddressMask[0] = 0xff;
    ipv4AddressMask[1] = 0xff;

    if (!dipRuleId)
    {
        retVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_ENTRY,
                                        &dipRuleId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Could not allocate ACL ruleId id, error %d\n", retVal);
            return retVal;
        }
    }
    rc = cpssHalCtrlCommonPacketDrop(devId, true, false, ipv4AddressPattern,
                                     ipv4AddressMask, NULL, NULL,
                                     dipRuleId);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed cpssHalCtrlCommonPacketDrop error:%d\n", rc);
        return XP_ERR_FAILED;
    }
#endif

#if 0 /* Not a part of TBD */
    /* Install Sip Link-local ipv6 rule */
    memset(ipv6AddressMask, 0, sizeof(ipv6AddressMask));
    ipv6AddressPattern[0] = 0xfe;
    ipv6AddressPattern[1] = 0x80;
    ipv6AddressPattern[2] = 0x0;
    ipv6AddressPattern[3] = 0x0;
    ipv6AddressPattern[4] = 0x0;
    ipv6AddressPattern[5] = 0x0;
    ipv6AddressPattern[6] = 0x0;
    ipv6AddressPattern[7] = 0x0;

    memset(ipv6AddressMask, 0xff, 8);

    rc = cpssHalCtrlCommonPacketDrop(devId, false, true, NULL, NULL,
                                     ipv6AddressPattern, ipv6AddressMask,
                                     CPSS_HAL_CTRLPKT_ACL_PRIO_IPv6_LINK_LOCAL_SRC_IP_MATCH);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed cpssHalCtrlCommonPacketDrop error:%d\n", rc);
        return XP_ERR_FAILED;
    }


    /* Install Dip Link-local ipv6 rule */
    memset(ipv6AddressMask, 0, sizeof(ipv6AddressMask));
    ipv6AddressPattern[0] = 0xfe;
    ipv6AddressPattern[1] = 0x80;
    ipv6AddressPattern[2] = 0x0;
    ipv6AddressPattern[3] = 0x0;
    ipv6AddressPattern[4] = 0x0;
    ipv6AddressPattern[5] = 0x0;
    ipv6AddressPattern[6] = 0x0;
    ipv6AddressPattern[7] = 0x0;

    memset(ipv6AddressMask, 0xff, 8);

    rc = cpssHalCtrlCommonPacketDrop(devId, false, false, NULL, NULL,
                                     ipv6AddressPattern, ipv6AddressMask,
                                     CPSS_HAL_CTRLPKT_ACL_PRIO_IPv6_LINK_LOCAL_DST_IP_MATCH);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed cpssHalCtrlCommonPacketDrop error:%d\n", rc);
        return XP_ERR_FAILED;
    }
#endif
    /* TODO Case only for ip packets: dest mac is broadcast but DIP is wrong */
    // /* Install drop dest mac broadcast and not arp */
    // rc = cpssHalCtrlPktDstBCMacNotArpDrop(devId, CPSS_HAL_CTRLPKT_ACL_PRIO_BC_DST_NOT_ARP_MATCH);
    // if (rc != GT_OK)
    // {
    //     LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
    //           "Failed cpssHalCtrlPktDstBCMacNotArpDrop error:%d\n", rc);
    //     return XP_ERR_FAILED;
    // }

    return XP_NO_ERR;
}

XP_STATUS xpsAclAddEgressDropPacketsRule(xpsDevice_t devId)
{
    GT_STATUS rc;

    if (cpssHalIsEbofDevice(devId))
    {
        rc = cpssHalCtrlPktFabricoErrorPacketDrop(devId,
                                                  XPS_FABRICO_ERROR_EGRESS_DROP_61_IPV4_MATCH_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssHalCtrlPktFabricoErrorPacketDrop 61. error:%d\n", rc);
            return XP_ERR_FAILED;
        }
        rc = cpssHalCtrlPktFabricoErrorPacketDrop(devId,
                                                  XPS_FABRICO_ERROR_EGRESS_DROP_62_IPV4_MATCH_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssHalCtrlPktFabricoErrorPacketDrop 62. error:%d\n", rc);
            return XP_ERR_FAILED;
        }
        rc = cpssHalCtrlPktFabricoErrorPacketDrop(devId,
                                                  XPS_FABRICO_ERROR_EGRESS_DROP_63_IPV4_MATCH_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssHalCtrlPktFabricoErrorPacketDrop 63. error:%d\n", rc);
            return XP_ERR_FAILED;
        }
        rc = cpssHalCtrlPktFabricoErrorPacketDrop(devId,
                                                  XPS_FABRICO_ERROR_EGRESS_DROP_61_IPV6_MATCH_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssHalCtrlPktFabricoErrorPacketDrop IPv6 61. error:%d\n", rc);
            return XP_ERR_FAILED;
        }
        rc = cpssHalCtrlPktFabricoErrorPacketDrop(devId,
                                                  XPS_FABRICO_ERROR_EGRESS_DROP_62_IPV6_MATCH_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssHalCtrlPktFabricoErrorPacketDrop IPv6 62. error:%d\n", rc);
            return XP_ERR_FAILED;
        }
        rc = cpssHalCtrlPktFabricoErrorPacketDrop(devId,
                                                  XPS_FABRICO_ERROR_EGRESS_DROP_63_IPV6_MATCH_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssHalCtrlPktFabricoErrorPacketDrop IPv6 63. error:%d\n", rc);
            return XP_ERR_FAILED;
        }
    }
    return XP_NO_ERR;
}

XP_STATUS xpsAclAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    GT_STATUS status   = GT_OK;
    XP_STATUS xpStatus = XP_NO_ERR;
    CPSS_INTERFACE_INFO_STC  interfaceInfo;
    CPSS_PCL_DIRECTION_ENT          direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC lookupCfg;
    int32_t   portNum  = 0;
    uint8_t   maxPorts = 0;
    uint32_t  cpuPortNum;
    GT_U8     cpssDevId = 0;
    GT_U32    cpssPortNum = 0;

    memset(&cncBlockListBound, XPS_ACL_PARALLEL_LOOKUP_INVALID,
           sizeof(uint8_t)*(XPS_ACL_CNC_BLOCK_COUNT));
    memset(&cncPerBlockCount, 0, sizeof(uint16_t)*(XPS_ACL_CNC_BLOCK_COUNT));

    xpStatus = xpsGlobalSwitchControlGetMaxPorts(devId, &maxPorts);
    if (xpStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get max physical port number. error:%d\n", xpStatus);
        return xpStatus;
    }

    XPS_GLOBAL_PORT_ITER(portNum, maxPorts)
    {
        status = cpssHalAclEnablePortIngressAcl(devId, portNum, GT_TRUE);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to pcl port ingress policy enable, device:%d, port:%d, error:%d\n ",
                  devId, portNum, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }

        status = cpssHalAclSetEgressAclPacketType(devId, portNum,
                                                  CPSS_DXCH_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set packet type for egress policy, device:%d, port:%d, error:%d\n ",
                  devId, portNum, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }

        /* set the access mode */

        /* for ingress acl*/
        status = cpssHalAclSetPortLookupCfgTabAccessMode(devId, portNum,
                                                         CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_NUMBER_0_E, 0,
                                                         CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set port lookup cfg table access mode, device:%d, port:%d, error:%d\n ",
                  devId, portNum, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }

        status = cpssHalAclSetPortLookupCfgTabAccessMode(devId, portNum,
                                                         CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_NUMBER_1_E, 0,
                                                         CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set port lookup cfg table access mode, device:%d, port:%d, error:%d\n ",
                  devId, portNum, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }

        /* for egress acl*/
        status = cpssHalAclSetPortLookupCfgTabAccessMode(devId, portNum,
                                                         CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_NUMBER_0_E, 0,
                                                         CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set port lookup cfg table access mode, device:%d, port:%d, error:%d\n ",
                  devId, portNum, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }

        memset(&interfaceInfo, 0, sizeof(interfaceInfo));
        memset(&lookupCfg, 0, sizeof(lookupCfg));

        /* Convert global port number into local (dev, port) */
        cpssDevId   = xpsGlobalIdToDevId(devId, portNum);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

        interfaceInfo.type = CPSS_INTERFACE_PORT_E;
        interfaceInfo.devPort.portNum = cpssPortNum;
        interfaceInfo.devPort.hwDevNum = cpssDevId;

        lookupCfg.pclId = 0;
        lookupCfg.enableLookup = GT_TRUE;
        lookupCfg.dualLookup = GT_FALSE;
        lookupCfg.groupKeyTypes.nonIpKey = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
        lookupCfg.groupKeyTypes.ipv4Key = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
        lookupCfg.groupKeyTypes.ipv6Key = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
        lookupCfg.tcamSegmentMode = CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E;
        lookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
        direction = CPSS_PCL_DIRECTION_INGRESS_E;
        if ((status = cpssHalAclSetPclCfgTblEntry(devId, &interfaceInfo, direction,
                                                  lookupNum, &lookupCfg)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalAclSetPclCfgTblEntry failed with err : %d on device :%d\n ", status,
                  devId);
            return xpsConvertCpssStatusToXPStatus(status);
        }
        lookupNum = CPSS_PCL_LOOKUP_NUMBER_1_E;
        lookupCfg.dualLookup = GT_TRUE;
        if ((status = cpssHalAclSetPclCfgTblEntry(devId, &interfaceInfo, direction,
                                                  lookupNum, &lookupCfg)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalAclSetPclCfgTblEntry failed with err : %d on device :%d\n ", status,
                  devId);
            return xpsConvertCpssStatusToXPStatus(status);
        }

        lookupCfg.dualLookup = GT_FALSE;
        lookupCfg.groupKeyTypes.nonIpKey = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E;
        lookupCfg.groupKeyTypes.ipv4Key = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E;
        lookupCfg.groupKeyTypes.ipv6Key = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E;
        direction = CPSS_PCL_DIRECTION_EGRESS_E;
        lookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
        if ((status = cpssHalAclSetPclCfgTblEntry(devId, &interfaceInfo, direction,
                                                  lookupNum, &lookupCfg)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalAclSetPclCfgTblEntry failed with err : %d on device :%d\n ", status,
                  devId);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* Vxlan IPCL config set */
    memset(&interfaceInfo, 0, sizeof(interfaceInfo));
    memset(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type = CPSS_INTERFACE_INDEX_E;
    interfaceInfo.index = XPS_VXLAN_IPV6_PCL_CFG_TABLE_INDEX_CNS;

    lookupCfg.pclId = XPS_VXLAN_IPV6_PCL_ID_CNS;
    lookupCfg.enableLookup = GT_TRUE;
    lookupCfg.dualLookup = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
    lookupCfg.groupKeyTypes.ipv4Key = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
    lookupCfg.groupKeyTypes.ipv6Key = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
    if ((status = cpssHalAclSetPclCfgTblEntry(devId, &interfaceInfo,
                                              CPSS_PCL_DIRECTION_INGRESS_E,
                                              CPSS_PCL_LOOKUP_NUMBER_0_E, &lookupCfg)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalAclSetPclCfgTblEntry failed with err : %d on device :%d\n ", status,
              devId);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    /* Set egress ACL packet type for CPU port */
    status = cpssHalGlobalSwitchControlGetCpuPhysicalPortNum(devId, &cpuPortNum);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalGlobalSwitchControlGetCpuPhysicalPortNum failed %d\n", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }
    status = cpssHalAclSetEgressAclPacketType(devId, cpuPortNum,
                                              CPSS_DXCH_PCL_EGRESS_PKT_TO_CPU_E, GT_TRUE);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set packet type for egress policy, device:%d, port:%d, error:%d\n ",
              devId, cpuPortNum, status);
        return xpsConvertCpssStatusToXPStatus(status);
    }
    status = cpssHalAclSetEgressAclPacketType(devId, cpuPortNum,
                                              CPSS_DXCH_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set packet type for egress policy, device:%d, port:%d, error:%d\n ",
              devId, cpuPortNum, status);
        return xpsConvertCpssStatusToXPStatus(status);
    }
    /* EPCL config set */
    memset(&interfaceInfo, 0, sizeof(interfaceInfo));
    memset(&lookupCfg, 0, sizeof(lookupCfg));

    /* Convert global port number into local (dev, port) */
    cpssDevId   = xpsGlobalIdToDevId(devId, cpuPortNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, cpuPortNum);

    interfaceInfo.type = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.portNum = cpssPortNum;
    interfaceInfo.devPort.hwDevNum = cpssDevId;

    lookupCfg.pclId = 0;
    lookupCfg.enableLookup = GT_TRUE;
    lookupCfg.dualLookup = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E;
    lookupCfg.groupKeyTypes.ipv4Key = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E;
    lookupCfg.groupKeyTypes.ipv6Key = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E;
    direction = CPSS_PCL_DIRECTION_EGRESS_E;
    lookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
    if ((status = cpssHalAclSetPclCfgTblEntry(cpssDevId, &interfaceInfo, direction,
                                              lookupNum, &lookupCfg)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalAclSetPclCfgTblEntry failed with err : %d on device :%d\n ", status,
              cpssDevId);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    /* Enabling counters */
    status = cpssHalPclCounterEnable(devId, (GT_BOOL)true);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to enable counters for ACL. error:%d\n", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    xpStatus = xpsAclCounterAllocatorsInit(devId);
    if (xpStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to init the allocators for counter block . error:%d\n", xpStatus);
        return xpStatus;
    }

    xpStatus = xpsAclCounterConfigure(devId);
    if (xpStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to Confiure CNC counters for ACL. error:%d\n", xpStatus);
        return xpStatus;
    }

    return XP_NO_ERR;

}

XP_STATUS xpsAclCounterAllocatorsInit(xpsDevice_t devId)
{
    xpsScope_t                      scopeId=0;
    XP_STATUS                       ret = XP_NO_ERR;

    /* initialize allocator for counter Id for Ingress parallel lookup 0*/

    ret = xpsAllocatorInitIdAllocator(scopeId, XPS_ALLOCATOR_ACL_COUNTER_BLOCK_1,
                                      XPS_CPSS_CNC_COUNTER_PER_BLK, XPS_CNC_BLOCK_0_RANGE_START);

    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to init the allocators for allocatorId : %d. error:%d\n",
              XPS_ALLOCATOR_ACL_COUNTER_BLOCK_2, ret);
        return ret;
    }

    ret = xpsAllocatorInitIdAllocator(scopeId, XPS_ALLOCATOR_ACL_COUNTER_BLOCK_2,
                                      XPS_CPSS_CNC_COUNTER_PER_BLK, XPS_CNC_BLOCK_1_RANGE_START);

    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to init the allocators for allocatorId : %d. error:%d\n",
              XPS_ALLOCATOR_ACL_COUNTER_BLOCK_2, ret);
        return ret;
    }

    ret = xpsAllocatorInitIdAllocator(scopeId, XPS_ALLOCATOR_ACL_COUNTER_BLOCK_3,
                                      XPS_CPSS_CNC_COUNTER_PER_BLK, XPS_CNC_BLOCK_2_RANGE_START);

    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to init the allocators for allocatorId : %d. error:%d\n",
              XPS_ALLOCATOR_ACL_COUNTER_BLOCK_3, ret);
        return ret;
    }

    ret = xpsAllocatorInitIdAllocator(scopeId, XPS_ALLOCATOR_ACL_COUNTER_BLOCK_4,
                                      XPS_CPSS_CNC_COUNTER_PER_BLK, XPS_CNC_BLOCK_3_RANGE_START);

    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to init the allocators for allocatorId : %d. error:%d\n",
              XPS_ALLOCATOR_ACL_COUNTER_BLOCK_4, ret);
        return ret;
    }

    ret = xpsAllocatorInitIdAllocator(scopeId, XPS_ALLOCATOR_ACL_COUNTER_BLOCK_5,
                                      XPS_CPSS_CNC_COUNTER_PER_BLK, XPS_CNC_BLOCK_4_RANGE_START);

    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to init the allocators for allocatorId : %d. error:%d\n",
              XPS_ALLOCATOR_ACL_COUNTER_BLOCK_5, ret);
        return ret;
    }

    ret = xpsAllocatorInitIdAllocator(scopeId, XPS_ALLOCATOR_ACL_COUNTER_BLOCK_6,
                                      XPS_CPSS_CNC_COUNTER_PER_BLK, XPS_CNC_BLOCK_5_RANGE_START);

    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to init the allocators for allocatorId : %d. error:%d\n",
              XPS_ALLOCATOR_ACL_COUNTER_BLOCK_6, ret);
        return ret;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclCounterConfigure(xpsDevice_t devId)
{
    GT_STATUS               rc = GT_OK;
    uint32_t                client;
    CPSS_PP_FAMILY_TYPE_ENT devType = cpssHalDevPPFamilyGet(devId);

    for (client = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E;
         client <= CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E; client++)
    {
        if ((client >= CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E) &&
            (client <= CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E))
        {
            continue;
        }

        if ((devType == CPSS_PP_FAMILY_DXCH_AC3X_E) ||
            (devType == CPSS_PP_FAMILY_DXCH_ALDRIN_E))
        {
            /* CNC block-0 is allocated for ACL counters for AC3X */
            rc = cpssHalCncBlockClientEnableAndBindSet(devId, XPS_ACL_CNC_BLK_NUM_AC3X,
                                                       (CPSS_DXCH_CNC_CLIENT_ENT) client, GT_TRUE, 0,
                                                       CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E);
        }
        else if (devType == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            /* CNC block-0 & 1 are allocated for ACL pkt & byte counters for Aldrin2 */
            rc = cpssHalPclCounterBlockConfigure(devId, XPS_ACL_CNC_BLK_NUM_ALDRIN2,
                                                 XPS_ACL_CNC_BLK_NUM_ALDRIN2+1,
                                                 (CPSS_DXCH_CNC_CLIENT_ENT) client, GT_TRUE);
        }

        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " Acl cnc counter configure failed with err : %d on device :%d\n ",
                  rc,
                  devId);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        /*Note: In falcon, CNC counters are allocated dynamically, nothing to done during init */
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclInitScope(xpsScope_t scopeId)
{
    XP_STATUS                       ret = XP_NO_ERR;
    GT_STATUS                       rc = GT_OK;
    uint32_t                        devNum = 0;
    aclTableIdDbHndl = XPS_IACL_TABLE_ID_DB_HNDL;

    ret = xpsStateRegisterDb(scopeId, "Acl table ID Db", XPS_GLOBAL,
                             &aclTableIdCompare, aclTableIdDbHndl);
    if (ret != XP_NO_ERR)
    {
        return ret;
    }

    aclTableEntryDbHndl = XPS_ACL_TABLE_ENTRY_DB_HNDL;

    ret = xpsStateRegisterDb(scopeId, "Acl table Entry Db", XPS_GLOBAL,
                             &aclTableEntryCompare, aclTableEntryDbHndl);
    if (ret != XP_NO_ERR)
    {
        return ret;
    }

    aclTableVtcamMapDbHndl = XPS_ACL_TABLE_VTCAM_MAP_DB_HNDL;

    ret = xpsStateRegisterDb(scopeId, "Acl table Vtcam Mapping Db", XPS_GLOBAL,
                             &aclVtcamMappingCompare, aclTableVtcamMapDbHndl);
    if (ret != XP_NO_ERR)
    {
        return ret;
    }

    aclCounterIdMapDbHndl = XPS_ACL_COUNTER_ID_MAPPING_DB_HNDL;

    ret = xpsStateRegisterDb(scopeId, "Acl table Vtcam Mapping Db", XPS_GLOBAL,
                             &aclCounterIdMappingCompare, aclCounterIdMapDbHndl);
    if (ret != XP_NO_ERR)
    {
        return ret;
    }

    /* initialize the Acl Table Id Allocator*/
    ret = xpsAllocatorInitIdAllocator(scopeId, XPS_ALLOCATOR_ACL_TABLE_ID,
                                      (XP_ACL_TABLE_MAX_IDS + ACL_TABLE_BASE),
                                      (XP_ACL_TABLE_RANGE_START + ACL_TABLE_BASE));

    if (ret != XP_NO_ERR)
    {
        return ret;
    }

    if ((rc = cpssHalAclInit(devNum)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalAclInit failed with err : %d on device :%d\n ", rc, devNum);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    if ((rc = cpssHalTcamAclClientGroupSet(devNum)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalTcamAclClientGroupSet failed with err : %d on device :%d\n ", rc,
              devNum);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalVtcamMgrCreate(devNum, XPS_GLOBAL_TACM_MGR);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to create Vtcam Mgr ",
              rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    ret = xpsAclTableInit(devNum);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsAclTableInit failed with err : %d ", ret);
        return ret;
    }

    rc = cpssHalAclIngressPolicyEnable(devNum, (GT_BOOL)true);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to enable ingress policy ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalAclEgressPolicyEnable(devNum, (GT_BOOL)true);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to enable egress policy ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalAclPortListEnable(devNum, (GT_BOOL)true);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to enable port list  ",
              rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    ret = xpsAclPortListPortMappingSet(devNum);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the port-portgroup mapping for portlist. error:%d\n", ret);
        return ret;
    }

    /*Below setting is to carry PCL-Id info from TTI to IPCL*/
    rc = cpssHalPclTunnelTermForceVlanModeEnableSet(devNum, (GT_BOOL)false);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to disable PclTunnelTermForceVlanMode ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    ret = xpsAclUdbKeysInitScope(devNum);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsAclUdbKeysInitScope failed with err : %d ", ret);
        return ret;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclTableInit(xpsDevice_t devId)
{
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC     vTcamInfo;
    GT_STATUS                           rc = GT_OK;
    GT_U32    ipcl0ClientGrpId = 0xffffffff;
    GT_U32    ipcl1ClientGrpId = 0xffffffff;
    GT_U32    epclClientGrpId = 0xffffffff;
    uint32_t hitNum = 0;

    memset(&vTcamInfo, 0x00, sizeof(CPSS_DXCH_VIRTUAL_TCAM_INFO_STC));
    rc = cpssHalTcamAclClientGroupIdGet(devId, CPSS_DXCH_TCAM_IPCL_0_E,
                                        &ipcl0ClientGrpId);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to IPCL_0 groupId : %d  ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalTcamAclClientGroupIdGet(devId, CPSS_DXCH_TCAM_IPCL_1_E,
                                        &ipcl1ClientGrpId);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to IPCL_1 groupId : %d  ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalTcamAclClientGroupIdGet(devId, CPSS_DXCH_TCAM_EPCL_E,
                                        &epclClientGrpId);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to IPCL_1 groupId : %d  ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    vTcamInfo.hitNumber = 0;
    vTcamInfo.ruleSize = (CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT)
                         XPS_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    vTcamInfo.autoResize = (GT_BOOL)true;
    vTcamInfo.ruleAdditionMethod = (CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_ENT)
                                   XPS_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;

    /* Creating tcam regions for mirror acls */
    vTcamInfo.clientGroup = ipcl1ClientGrpId;

    //IPCL_1
    if ((rc = cpssHalVtcamCreate(XPS_GLOBAL_TACM_MGR,
                                 XPS_L3_MIRROR_ING_STG_TABLE_ID_0,
                                 &vTcamInfo)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to create tcam region : %d  ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    vTcamInfo.ruleSize = (CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT)
                         XPS_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
    //IPCL_1
    if ((rc = cpssHalVtcamCreate(XPS_GLOBAL_TACM_MGR,
                                 XPS_L3V6_MIRROR_ING_STG_TABLE_ID_0,
                                 &vTcamInfo)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to create tcam region : %d  ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    //IPCL_0 : ING
    vTcamInfo.clientGroup = ipcl0ClientGrpId;
    vTcamInfo.ruleSize = (CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT)
                         XPS_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    /* Creating tcam regions for control acl rules */

    if ((rc = cpssHalVtcamCreate(XPS_GLOBAL_TACM_MGR, XPS_L3_CTRL_TABLE_ID,
                                 &vTcamInfo)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to create tcam region : %d  ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    vTcamInfo.ruleSize = (CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT)
                         XPS_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
    if ((rc = cpssHalVtcamCreate(XPS_GLOBAL_TACM_MGR, XPS_L3V6_CTRL_TABLE_ID,
                                 &vTcamInfo)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to create tcam region : %d  ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Creating tcam regions for 4 parallel lookups for 30B  rules */

    vTcamInfo.ruleSize = (CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT)
                         XPS_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    for (hitNum = 0; hitNum < 2; hitNum++)
    {
        vTcamInfo.hitNumber = hitNum;
        /* Ingress stage vTcam */
        vTcamInfo.clientGroup = ipcl0ClientGrpId;
        if ((rc = cpssHalVtcamCreate(XPS_GLOBAL_TACM_MGR,
                                     (XPS_L3_ING_STG_TABLE_ID_0+hitNum),
                                     &vTcamInfo)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "failed to create ing tcam region. hitNum :%d rc  %d  ", hitNum, rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        /* Egress stage vTcam */
        vTcamInfo.clientGroup = epclClientGrpId;
        if ((rc = cpssHalVtcamCreate(XPS_GLOBAL_TACM_MGR,
                                     (XPS_L3_EGR_STG_TABLE_ID_0+hitNum),
                                     &vTcamInfo)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "failed to create egr tcam region. hitNum :%d rc  %d  ", hitNum, rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }

    /* Creating tcam regions for 4 parallel lookups for 60B  rules */
    vTcamInfo.ruleSize = (CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT)
                         XPS_VIRTUAL_TCAM_RULE_SIZE_60_B_E;

    for (hitNum = 0; hitNum < 4; hitNum++)
    {
        vTcamInfo.hitNumber = hitNum;
        /* Ingress stage vTcam */
        vTcamInfo.clientGroup = ipcl0ClientGrpId;
        if ((rc = cpssHalVtcamCreate(XPS_GLOBAL_TACM_MGR,
                                     (XPS_L3V6_ING_STG_TABLE_ID_0+hitNum),
                                     &vTcamInfo)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "failed to create V6 Ing tcam region. hitNum :%d rc  %d  ", hitNum, rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        /* Egress stage vTcam */
        vTcamInfo.clientGroup = epclClientGrpId;
        if ((rc = cpssHalVtcamCreate(XPS_GLOBAL_TACM_MGR,
                                     (XPS_L3V6_EGR_STG_TABLE_ID_0+hitNum),
                                     &vTcamInfo)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "failed to create V6 Egrtcam region. hitNum :%d rc  %d  ", hitNum, rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }

    // Creating tcam regions for port v4/v6 statistcs
    //IPCL v4
    vTcamInfo.clientGroup = ipcl1ClientGrpId;
    vTcamInfo.hitNumber = 3;
    vTcamInfo.ruleSize = (CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT)
                         XPS_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    if ((rc = cpssHalVtcamCreate(XPS_GLOBAL_TACM_MGR,
                                 XPS_PORT_V4_ING_STG_TABLE_ID_0,
                                 &vTcamInfo)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to create tcam region : %d  ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    //EPCL v4
    vTcamInfo.clientGroup = epclClientGrpId;
    vTcamInfo.hitNumber = 3;
    vTcamInfo.ruleSize = (CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT)
                         XPS_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    if ((rc = cpssHalVtcamCreate(XPS_GLOBAL_TACM_MGR,
                                 XPS_PORT_V4_ERG_STG_TABLE_ID_0,
                                 &vTcamInfo)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to create tcam region : %d  ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    //IPCL v6
    vTcamInfo.clientGroup = ipcl1ClientGrpId;
    vTcamInfo.hitNumber = 3;
    vTcamInfo.ruleSize = (CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT)
                         XPS_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
    if ((rc = cpssHalVtcamCreate(XPS_GLOBAL_TACM_MGR,
                                 XPS_PORT_V6_ING_STG_TABLE_ID_0,
                                 &vTcamInfo)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to create tcam region : %d  ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    //EPCL v6
    vTcamInfo.clientGroup = epclClientGrpId;
    vTcamInfo.hitNumber = 3;
    vTcamInfo.ruleSize = (CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT)
                         XPS_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
    if ((rc = cpssHalVtcamCreate(XPS_GLOBAL_TACM_MGR,
                                 XPS_PORT_V6_ERG_STG_TABLE_ID_0,
                                 &vTcamInfo)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to create tcam region : %d  ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclPortListPortMappingSet(xpsDevice_t devId)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    GT_STATUS rc        = GT_OK;
    int32_t   portNum   = 0;
    uint8_t   maxPorts  = 0;
    uint32_t  portGroup = 0;
    uint32_t  offset    = 0;
    uint32_t  cpssPortNum = 0;
    uint32_t  cpssDevNum = 0;

    xpStatus = xpsGlobalSwitchControlGetMaxPorts(devId, &maxPorts);
    if (xpStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get max physical port number. error:%d\n", xpStatus);
        return xpStatus;
    }

    XPS_GLOBAL_PORT_ITER(portNum, maxPorts)
    {
        cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

        portGroup = (cpssPortNum)/(XPS_PORT_LIST_OFFSET);

        offset = (cpssPortNum)%(XPS_PORT_LIST_OFFSET);
        if ((rc = cpssHalAclPortListPortMappingSet(cpssDevNum,
                                                   (CPSS_PCL_DIRECTION_ENT)XPS_ACL_DIRECTION_INGRESS, cpssPortNum, (GT_BOOL)true,
                                                   portGroup, offset)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "failed to set port list mapping for portnum : %d error: %d  ", cpssPortNum,
                  rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        if ((rc = cpssHalAclPortListPortMappingSet(cpssDevNum,
                                                   (CPSS_PCL_DIRECTION_ENT)XPS_ACL_DIRECTION_EGRESS, cpssPortNum, (GT_BOOL)true,
                                                   portGroup, offset)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "failed to set port list mapping for portnum : %d error: %d  ", cpssPortNum,
                  rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        offset++;
    }

    /* Init cpu port map */
    portGroup = (CPSS_CPU_PORT_NUM_CNS)/(XPS_PORT_LIST_OFFSET);
    offset = (CPSS_CPU_PORT_NUM_CNS)%(XPS_PORT_LIST_OFFSET);
    if ((rc = cpssHalAclPortListPortMappingSet(devId,
                                               (CPSS_PCL_DIRECTION_ENT)XPS_ACL_DIRECTION_EGRESS, CPSS_CPU_PORT_NUM_CNS,
                                               (GT_BOOL)true,
                                               portGroup, offset)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to set port list mapping for portnum : %d error: %d  ",
              CPSS_CPU_PORT_NUM_CNS, rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    return XP_NO_ERR;
}


XP_STATUS xpsAclTableDeInitScope(xpsScope_t scopeId)
{
    GT_STATUS                           rc = GT_OK;

    /* Creating tcam regions for 4 parallel lookups for 30B  rules */

    for (uint32_t vTcamId = XPS_MIN_VTCAM_IDS; vTcamId <= XPS_MAX_VTCAM_IDS;
         vTcamId++)
    {
        if ((rc = cpssHalVtcamRemove(XPS_GLOBAL_TACM_MGR, vTcamId)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "failed to delete tcam region : vTcamId :%d rc %d  ", vTcamId, rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }

    /* Deleting the vTcam Manager */
    if ((rc = cpssHalVtcamMgrDelete(scopeId, XPS_GLOBAL_TACM_MGR)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to delete vTcam Manager : %d  ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    return XP_NO_ERR;

}

XP_STATUS xpsAclCreateTable(xpsDevice_t devId, xpAclTableInfo_t tableInfo,
                            uint32_t *tableIdNum)
{
    XPS_LOCK(xpsAclCreateTable);

    XP_STATUS                        ret = XP_NO_ERR;
    uint32_t                         tableId = 0;
    xpsScope_t                       scopeId = 0;
    xpsAclTableContextDbEntry_t      *aclTableIdDbKey = NULL;

    /* Get scope from Device ID */
    if ((ret = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        return ret;
    }

    if ((ret = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_ACL_TABLE_ID,
                                      &tableId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsAllocatorAllocateId failed with err : %d\n ", ret);
        return ret;
    }

    if ((ret = xpsStateHeapMalloc(sizeof(xpsAclTableContextDbEntry_t),
                                  (void**)&aclTableIdDbKey)) == XP_NO_ERR)
    {
        memset(aclTableIdDbKey, 0, sizeof(xpsAclTableContextDbEntry_t));
        aclTableIdDbKey->tableId = tableId;
        aclTableIdDbKey->tcamMgrId = XPS_GLOBAL_TACM_MGR;
        aclTableIdDbKey->stage = tableInfo.stage;
        aclTableIdDbKey->tableType = tableInfo.tableType;
        aclTableIdDbKey->isMirror = tableInfo.isMirror;

        if ((ret = xpsStateInsertData(scopeId, aclTableIdDbHndl,
                                      (void*)aclTableIdDbKey)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Insert IACL keyformat failed, devId(%d)", devId);
            /* Free Allocated memory */
            xpsStateHeapFree((void*)aclTableIdDbKey);
            return ret;
        }
    }

    /* This Id need to store for futher operation */
    *tableIdNum = tableId;

    return ret;

}

XP_STATUS xpsAclUpdateTableVtcamMapping(xpsDevice_t devId, uint32_t tableId,
                                        uint32_t groupId)
{

    XP_STATUS                               xpsRetVal = XP_NO_ERR;
    xpsAclTableVtcamMapContextDbEntry_t     tableVtcamMapDbKey;
    xpsAclTableVtcamMapContextDbEntry_t     *tableVtcamMapDbEntry = NULL;
    xpsAclTableContextDbEntry_t             tableIdDbKey;
    xpsAclTableContextDbEntry_t             *tableIdDbEntry = NULL;
    xpsScope_t                              scopeId = 0;
    bool                                    newVtcamRegion = true;


    /* Get scope from Device ID */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        return xpsRetVal;
    }

    tableVtcamMapDbKey.groupId = groupId;
    if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableVtcamMapDbHndl,
                                         (xpsDbKey_t)&tableVtcamMapDbKey,
                                         (void**)&tableVtcamMapDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    if (tableVtcamMapDbEntry == NULL)
    {
        if ((xpsRetVal = xpsStateHeapMalloc(sizeof(xpsAclTableVtcamMapContextDbEntry_t),
                                            (void**)&tableVtcamMapDbEntry)) == XP_NO_ERR)
        {
            memset(tableVtcamMapDbEntry, 0, sizeof(xpsAclTableVtcamMapContextDbEntry_t));
            tableVtcamMapDbEntry->groupId = groupId;
            /* Start with XP_ACL_TABLE_MAX_IDS and dynamically grow. */
            tableVtcamMapDbEntry->tableMax = XP_ACL_TABLE_MAX_IDS;
            if ((xpsRetVal = xpsStateHeapMalloc((sizeof(uint32_t)*
                                                 (tableVtcamMapDbEntry->tableMax)),
                                                (void**)&(tableVtcamMapDbEntry->tableId))) == XP_NO_ERR)
            {
                memset(tableVtcamMapDbEntry->tableId, 0,
                       (sizeof(uint32_t)*(tableVtcamMapDbEntry->tableMax)));
            }
            else
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Memory allocation for ACL table entry failed, devId(%d)", devId);
                xpsStateHeapFree((void*)tableVtcamMapDbEntry);
                return xpsRetVal;
            }

            if ((xpsRetVal = xpsStateInsertData(scopeId, aclTableVtcamMapDbHndl,
                                                (void*)tableVtcamMapDbEntry)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Insert ACL table entry failed, devId(%d)", devId);
                /* Free Allocated memory */
                xpsStateHeapFree((void*)tableVtcamMapDbEntry->tableId);
                xpsStateHeapFree((void*)tableVtcamMapDbEntry);
                return xpsRetVal;
            }
        }
    }
    if (tableVtcamMapDbEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    /*Dynamically grow the number of tables per group.*/
    if (tableVtcamMapDbEntry->tableCount >= tableVtcamMapDbEntry->tableMax)
    {
        uint32_t oldMaxCnt = tableVtcamMapDbEntry->tableMax;
        uint32_t *oldCtx = tableVtcamMapDbEntry->tableId;
        tableVtcamMapDbEntry->tableMax += XP_ACL_TABLE_MAX_IDS;
        if ((xpsRetVal = xpsStateHeapMalloc((sizeof(uint32_t)*
                                             (tableVtcamMapDbEntry->tableMax)),
                                            (void**)&(tableVtcamMapDbEntry->tableId))) == XP_NO_ERR)
        {
            memset(tableVtcamMapDbEntry->tableId, 0,
                   (sizeof(uint32_t)*(tableVtcamMapDbEntry->tableMax)));
            memcpy(tableVtcamMapDbEntry->tableId, oldCtx, (sizeof(uint32_t)*oldMaxCnt));
            /* Free Old Ctx memory */
            xpsStateHeapFree((void*)oldCtx);
        }
        else
        {
            /* What to do with oldCtx */
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Memory allocation for ACL table entry failed, devId(%d)", devId);
            return xpsRetVal;
        }
    }

    tableVtcamMapDbEntry->tableId[ tableVtcamMapDbEntry->tableCount ] = tableId;
    tableVtcamMapDbEntry->tableCount += 1;

    /* Retrieving table info */
    tableIdDbKey.tableId = tableId ;
    if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                         (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }
    if (tableIdDbEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    if (tableIdDbEntry->bindCount)
    {
        if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
        {
            if (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)
            {
                for (uint8_t i=0; i< tableIdDbEntry->numOfDuplicates; i++)
                {
                    if (!(tableVtcamMapDbEntry->vTcamRegionIngressL3V6[ tableIdDbEntry->vtcamId[i] -
                                                                                                   XPS_ING_STG_V6_USER_ACL_TABLE_START ]))
                    {
                        newVtcamRegion = false;
                        tableIdDbEntry->currentVtcamId = tableIdDbEntry->vtcamId[i];
                        tableVtcamMapDbEntry -> vTcamRegionIngressL3V6[tableIdDbEntry->vtcamId[i] -
                                                                                                  XPS_ING_STG_V6_USER_ACL_TABLE_START]+= 1;
                        break;
                    }
                }
            }
            else
            {
                for (uint8_t i=0; i< tableIdDbEntry->numOfDuplicates; i++)
                {
                    if (!(tableVtcamMapDbEntry->vTcamRegionEgressL3V6[ tableIdDbEntry->vtcamId[i] -
                                                                                                  XPS_EGR_STG_V6_USER_ACL_TABLE_START ]))
                    {
                        newVtcamRegion = false;
                        tableIdDbEntry->currentVtcamId = tableIdDbEntry->vtcamId[i];
                        tableVtcamMapDbEntry -> vTcamRegionEgressL3V6[tableIdDbEntry->vtcamId[i] -
                                                                                                 XPS_EGR_STG_V6_USER_ACL_TABLE_START]+= 1;
                        break;
                    }
                }
            }
        }
        else
        {
            if (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)
            {
                for (uint8_t i=0; i< tableIdDbEntry->numOfDuplicates; i++)
                {
                    if (!(tableVtcamMapDbEntry->vTcamRegionIngressL3[ tableIdDbEntry->vtcamId[i] -
                                                                                                 XPS_ING_STG_V4_USER_ACL_TABLE_START ]))
                    {
                        newVtcamRegion = false;
                        tableIdDbEntry->currentVtcamId = tableIdDbEntry->vtcamId[i];
                        tableVtcamMapDbEntry -> vTcamRegionIngressL3[ tableIdDbEntry->vtcamId[i] -
                                                                                                 XPS_ING_STG_V4_USER_ACL_TABLE_START ]+= 1;
                        break;
                    }
                }
            }
            else
            {
                for (uint8_t i=0; i< tableIdDbEntry->numOfDuplicates; i++)
                {
                    if (!(tableVtcamMapDbEntry->vTcamRegionEgressL3[ tableIdDbEntry->vtcamId[i] -
                                                                                                XPS_EGR_STG_V4_USER_ACL_TABLE_START ]))
                    {
                        newVtcamRegion = false;
                        tableIdDbEntry->currentVtcamId = tableIdDbEntry->vtcamId[i];
                        tableVtcamMapDbEntry -> vTcamRegionEgressL3[ tableIdDbEntry->vtcamId[i] -
                                                                                                XPS_EGR_STG_V4_USER_ACL_TABLE_START ]+= 1;
                        break;
                    }
                }
            }
        }
    }

    if (newVtcamRegion == true)
    {
        uint8_t i = 0;

        if (tableIdDbEntry->numOfDuplicates < XPS_ACL_MAX_TBLS_IN_GROUP)
        {
            if (tableIdDbEntry->stage ==
                XPS_PCL_STAGE_INGRESS) /*Finding a space in vtcam region mapping for ingress stage for parallel lookup*/
            {
                if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
                {
                    for (i =0; i< XPS_ACL_MAX_TBLS_IN_GROUP; i++)
                    {
                        if (!(tableVtcamMapDbEntry->vTcamRegionIngressL3V6[i]))
                        {
                            tableIdDbEntry->vtcamId[tableIdDbEntry->numOfDuplicates] = i +
                                                                                       XPS_ING_STG_V6_USER_ACL_TABLE_START;
                            tableIdDbEntry->currentVtcamId = i + XPS_ING_STG_V6_USER_ACL_TABLE_START;
                            tableVtcamMapDbEntry->vTcamRegionIngressL3V6[i] += 1;
                            break;
                        }
                    }
                    if (i >= XPS_ACL_MAX_TBLS_IN_GROUP) /*if all vtcams for parallel lookup are occupied by tables of same group : new tables added to 1st vtcam region*/
                    {
                        tableIdDbEntry->vtcamId[tableIdDbEntry->numOfDuplicates] =
                            XPS_ING_STG_V6_USER_ACL_TABLE_START;
                        tableIdDbEntry->currentVtcamId = XPS_ING_STG_V6_USER_ACL_TABLE_START;
                        tableVtcamMapDbEntry->vTcamRegionIngressL3V6[tableIdDbEntry->currentVtcamId -
                                                                                                    XPS_ING_STG_V6_USER_ACL_TABLE_START] += 1;
                    }
                }
                else
                {
                    for (i =0; i< XPS_ACL_MAX_TBLS_IN_GROUP; i++)
                    {
                        if (!(tableVtcamMapDbEntry -> vTcamRegionIngressL3[i]))
                        {
                            tableIdDbEntry->vtcamId[tableIdDbEntry->numOfDuplicates] = i +
                                                                                       XPS_ING_STG_V4_USER_ACL_TABLE_START;
                            tableIdDbEntry->currentVtcamId = i + XPS_ING_STG_V4_USER_ACL_TABLE_START;
                            tableVtcamMapDbEntry -> vTcamRegionIngressL3[i]+= 1;
                            break;
                        }
                    }
                    /*TODO:For 5th table, when part of 4th group, because of current design
                      where numOfDuplicates++, tablIdDb->vtcamId array becomes {2,2,2,2} with
                      numOfDuplicates=4. Because of this, the L3Vtcam region ref_cnt decrement
                      in UnBind APIs are not proper, this needs to be fixed.Array-out-of-bounds are
                      avoided by defensive check in these APIs(to not decrement when value is already 0).
                      Also, because of this 5th table cannot be part of more than 4 groups, which
                      also has to be fixed.
                      Check why numOfDuplicates has to be incremented in this scenario, without this,
                      the issues can be fixed.
                    */
                    if (i >= XPS_ACL_MAX_TBLS_IN_GROUP) /*if all vtcams for parallel lookup are occupied by tables of same group : new tables added to 1st vtcam region*/
                    {
                        tableIdDbEntry->vtcamId[tableIdDbEntry->numOfDuplicates] =
                            XPS_ING_STG_V4_USER_ACL_TABLE_START;
                        tableIdDbEntry->currentVtcamId = XPS_ING_STG_V4_USER_ACL_TABLE_START;
                        tableVtcamMapDbEntry -> vTcamRegionIngressL3[ tableIdDbEntry->currentVtcamId -
                                                                                                     XPS_ING_STG_V4_USER_ACL_TABLE_START ]+= 1;
                    }
                }
                tableIdDbEntry->numOfDuplicates++;
            }
            else       /*Finding a space in vtcam region mapping for Egress stage for parallel lookup*/
            {
                if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
                {
                    for (i =0; i< XPS_ACL_MAX_TBLS_IN_GROUP; i++)
                    {
                        if (!(tableVtcamMapDbEntry->vTcamRegionEgressL3V6[i]))
                        {
                            tableIdDbEntry->vtcamId[tableIdDbEntry->numOfDuplicates] = i +
                                                                                       XPS_EGR_STG_V6_USER_ACL_TABLE_START;
                            tableIdDbEntry->currentVtcamId = i + XPS_EGR_STG_V6_USER_ACL_TABLE_START;
                            tableVtcamMapDbEntry->vTcamRegionEgressL3V6[i] += 1;
                            break;
                        }
                    }
                    if (i >= XPS_ACL_MAX_TBLS_IN_GROUP) /*if all vtcams for parallel lookup are occupied by tables of same group : new tables added to 1st vtcam region*/
                    {
                        tableIdDbEntry->vtcamId[tableIdDbEntry->numOfDuplicates] =
                            XPS_EGR_STG_V6_USER_ACL_TABLE_START;
                        tableIdDbEntry->currentVtcamId = XPS_EGR_STG_V6_USER_ACL_TABLE_START;
                        tableVtcamMapDbEntry->vTcamRegionEgressL3V6[tableIdDbEntry->currentVtcamId -
                                                                                                   XPS_EGR_STG_V6_USER_ACL_TABLE_START] += 1;
                    }
                }
                else
                {
                    for (i =0; i< XPS_ACL_MAX_TBLS_IN_GROUP; i++)
                    {
                        if (!(tableVtcamMapDbEntry -> vTcamRegionEgressL3[i]))
                        {
                            tableIdDbEntry->vtcamId[tableIdDbEntry->numOfDuplicates] = i +
                                                                                       XPS_EGR_STG_V4_USER_ACL_TABLE_START;
                            tableIdDbEntry->currentVtcamId = i + XPS_EGR_STG_V4_USER_ACL_TABLE_START;
                            tableVtcamMapDbEntry -> vTcamRegionEgressL3[i]+= 1;
                            break;
                        }
                    }
                    if (i >= XPS_ACL_MAX_TBLS_IN_GROUP) /*if all vtcams for parallel lookup are occupied by tables of same group : new tables added to 1st vtcam region*/
                    {
                        tableIdDbEntry->vtcamId[tableIdDbEntry->numOfDuplicates] =
                            XPS_EGR_STG_V4_USER_ACL_TABLE_START;
                        tableIdDbEntry->currentVtcamId = XPS_EGR_STG_V4_USER_ACL_TABLE_START;
                        tableVtcamMapDbEntry -> vTcamRegionEgressL3[ tableIdDbEntry->currentVtcamId -
                                                                                                    XPS_EGR_STG_V4_USER_ACL_TABLE_START ]+= 1;
                    }
                }
                tableIdDbEntry->numOfDuplicates++;
            }
        }
    }

    tableIdDbEntry->bindCount++;

    return XP_NO_ERR;

}

XP_STATUS xpsAclUdbKeysInitScope(xpsScope_t scopeId)
{
    XP_STATUS                        ret = XP_NO_ERR;
    uint32_t                         devNum = 0;
    GT_U32                           directionId, lookupId;

    for (directionId = CPSS_PCL_DIRECTION_INGRESS_E;
         directionId <= CPSS_PCL_DIRECTION_EGRESS_E; directionId++)
    {
        for (lookupId = CPSS_PCL_LOOKUP_NUMBER_0_E;
             lookupId <= CPSS_PCL_LOOKUP_NUMBER_1_E; lookupId++)
        {
            if ((directionId == CPSS_PCL_DIRECTION_EGRESS_E) &&
                (lookupId == CPSS_PCL_LOOKUP_NUMBER_1_E))
            {
                break;
            }
            else
            {
                ret = xpsAclUdbKeysForNonIpInit(devNum, directionId, lookupId);
                if (ret != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          " xpsAclUdbKeysForNonIpInit failed with err : %d ", ret);
                    return ret;
                }

                ret = xpsAclUdbKeysForIpv4Init(devNum, directionId, lookupId,
                                               XPS_PCL_PACKET_TYPE_IPV4_TCP);
                if (ret != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          " xpsAclUdbKeysForIpv4Init failed with err : %d ", ret);
                    return ret;
                }

                ret = xpsAclUdbKeysForIpv4Init(devNum, directionId, lookupId,
                                               XPS_PCL_PACKET_TYPE_IPV4_UDP);
                if (ret != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          " xpsAclUdbKeysForIpv4Init failed with err : %d ", ret);
                    return ret;
                }

                ret = xpsAclUdbKeysForIpv4Init(devNum, directionId, lookupId,
                                               XPS_PCL_PACKET_TYPE_IPV4_OTHER);
                if (ret != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          " xpsAclUdbKeysForIpv4Init failed with err : %d ", ret);
                    return ret;
                }

                ret = xpsAclUdbKeysForIpv6Init(devNum, directionId, lookupId,
                                               XPS_PCL_PACKET_TYPE_IPV6_TCP);
                if (ret != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          " xpsAclUdbKeysForIpv6Init failed with err : %d ", ret);
                    return ret;
                }

                ret = xpsAclUdbKeysForIpv6Init(devNum, directionId, lookupId,
                                               XPS_PCL_PACKET_TYPE_IPV6_UDP);
                if (ret != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          " xpsAclUdbKeysForIpv6Init failed with err : %d ", ret);
                    return ret;
                }

                ret = xpsAclUdbKeysForIpv6Init(devNum, directionId, lookupId,
                                               XPS_PCL_PACKET_TYPE_IPV6_OTHER);
                if (ret != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          " xpsAclUdbKeysForIpv6Init failed with err : %d ", ret);
                    return ret;
                }

                ret = xpsAclUdbKeysForArpInit(devNum, directionId, lookupId);
                if (ret != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          " xpsAclUdbKeysForArpInit failed with err : %d ", ret);
                    return ret;
                }

            }
        }
    }

    ret = xpsAclUdbKeysForVxlanIpv6Init(devNum);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsAclUdbKeysForVxlanIpv6Init failed with err : %d ", ret);
        return ret;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclUdbKeysForNonIpInit(xpsDevice_t devNum, uint32_t direction,
                                    uint32_t lookupNum)
{
    GT_STATUS                      status = GT_OK;
    GT_U8                          offset = 0; /* UDB offset from anchor */
    GT_U32                         udbIndex = 0;
    GT_U32                         index = 0;
    CPSS_DXCH_PCL_UDB_SELECT_STC   udbSelectStruct;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormatType;
    CPSS_PP_FAMILY_TYPE_ENT devType = cpssHalDevPPFamilyGet(devNum);

    if (direction == XPS_ACL_DIRECTION_INGRESS)
    {
        ruleFormatType = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
    }
    else
    {
        ruleFormatType = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E;
    }
    cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

    /* Key format for NON-IP flows */

    /* UDB[40-43] offset for PortList Metadata */
    for (udbIndex = 40, offset = 0 ; udbIndex < 44; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset /* PortList */);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    offset = (direction == CPSS_PCL_DIRECTION_INGRESS_E) ? 99 : 26;
    /* UDB[38-39] offset for VLAN-ID */
    for (udbIndex = 38; udbIndex < 40; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* vlan-id */
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[45] offset for Applicable Flow Sub-Template Metadata/ Is IP */
    offset = (direction == CPSS_PCL_DIRECTION_INGRESS_E) ? 91 : 40;
    for (udbIndex = 45 ; udbIndex < 46; udbIndex++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[46-47] offset for EtherType Metadata */
    offset = (direction == CPSS_PCL_DIRECTION_INGRESS_E) ? 24 : 22;
    for (udbIndex = 46; udbIndex < 48; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset /* Ether type */);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /*TODO: PCL UP0 is un-used now. This udb index is free to use */
    /* UDB[48] offset for UP0 Metadata */
    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        for (udbIndex = 48, offset = 105 ; udbIndex < 49; udbIndex++, offset++)
        {
            status = cpssHalPclUserDefinedByteSet(devNum,
                                                  ruleFormatType,
                                                  CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                                  (CPSS_PCL_DIRECTION_ENT)direction,
                                                  udbIndex,
                                                  CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                                  offset /* UP0 */);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set user defined byte,"
                      " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                      direction, udbIndex, offset, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
        }
    }
    /* PCL UP0 is used for CPU Code metadata field for Egress */
    /* UDB[48] offset for UP0 Metadata */
    else if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        for (udbIndex = 48, offset = 87 ; udbIndex < 49; udbIndex++, offset++)
        {
            status = cpssHalPclUserDefinedByteSet(devNum,
                                                  ruleFormatType,
                                                  CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                                  (CPSS_PCL_DIRECTION_ENT)direction,
                                                  udbIndex,
                                                  CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                                  offset /* CPU-CODE */);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set user defined byte,"
                      " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                      direction, udbIndex, offset, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
        }
    }

    /* UDB[49] offset for Reserved Metadata */
    if ((direction == CPSS_PCL_DIRECTION_EGRESS_E) &&
        (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
         devType == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        udbIndex = 49;
        offset=53;
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* */
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }

        udbIndex = 37;
        offset=64;
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* DSA-Tag cmd*/
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[2-7] offset for Source Mac address */
    for (udbIndex = 2, offset = 6 ; udbIndex < 8; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* SMAC */
                                              CPSS_DXCH_PCL_OFFSET_L2_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[8-13] offset for Destination Mac address */
    for (udbIndex = 8, offset = 0 ; udbIndex < 14; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* DMAC */
                                              CPSS_DXCH_PCL_OFFSET_L2_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[0-1] offset for pfc opcode */
    for (udbIndex = 0, offset = 15 ; udbIndex < 2; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* opCode */
                                              CPSS_DXCH_PCL_OFFSET_L2_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* it's a must to enable UDB's */
    for (index = 0, udbIndex = 40; index < 4; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex;      /* PortList */
    }
    udbSelectStruct.ingrUdbReplaceArr[4]    = GT_TRUE; /* Source ePort */
    udbSelectStruct.ingrUdbReplaceArr[5]    = GT_TRUE; /* Source ePort */
    udbSelectStruct.udbSelectArr[6]         = 38;       /* VLAN-ID */
    udbSelectStruct.udbSelectArr[7]         = 39;       /* VLAN-ID */
    udbSelectStruct.udbSelectArr[8]         = 46;      /* Ethertype */
    udbSelectStruct.udbSelectArr[9]         = 47;      /* Ethertype */
    udbSelectStruct.udbSelectArr[10]        =
        45;      /* Applicable Flow Sub-Template or Is IP */

    if ((direction == CPSS_PCL_DIRECTION_EGRESS_E) &&
        (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
         devType == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        udbSelectStruct.udbSelectArr[11]        = 49;      /* EPORT[7:1] */
    }
    udbSelectStruct.udbSelectArr[12]        = 48;      /* UP0 */
    for (index = 13, udbIndex = 2; index < 19; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex;/* SMAC */
    }
    for (index = 19, udbIndex = 8; index < 25; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex;/* DMAC */
    }
    if ((direction == CPSS_PCL_DIRECTION_EGRESS_E) &&
        (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
         devType == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        udbSelectStruct.udbSelectArr[25]         = 37;       /* DSA-TAG Cmd */
    }

    udbSelectStruct.udbSelectArr[26]         = 0;       /* opCode */
    udbSelectStruct.udbSelectArr[27]         = 1;       /* opCode */

    status = cpssHalPclUserDefinedBytesSelectSet(devNum,
                                                 ruleFormatType,
                                                 CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                                 (CPSS_PCL_LOOKUP_NUMBER_ENT)lookupNum,
                                                 &udbSelectStruct);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set user defined byte Select Arr,"
              " device:%d, direction:%d, lookupNum:%d, error:%d\n ", devNum, direction,
              lookupNum,
              status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclUpdatePortIngressAcl(xpsDevice_t devId, uint32_t portNum,
                                     uint32_t tableId, uint32_t groupId, bool enable)
{
    XP_STATUS                               xpsRetVal = XP_NO_ERR;
    uint32_t                                portGroup;
    uint32_t                                offset;
    xpsAclTableContextDbEntry_t             tableIdDbKey;
    xpsAclTableContextDbEntry_t             *tableIdDbEntry;
    xpsAclTableVtcamMapContextDbEntry_t     tableVtcamMapDbKey;
    xpsAclTableVtcamMapContextDbEntry_t     *tableVtcamMapDbEntry;
    xpsAclEntryContextDbEntry_t             tableEntryDbKey;
    xpsAclEntryContextDbEntry_t             *tableEntryDbEntry = NULL;
    xpsAclKeyFormat                         keyFormat;
    uint32_t                                ruleId, priority;
    bool                                    is_control_acl = false;
    xpsPclAction_t                          actionPtr;
    xpsScope_t                              scopeId = 0;
    uint32_t                                cpssPortNum;

    /* Get scope from Device ID */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsScopeGetScopeId failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    tableVtcamMapDbKey.groupId = groupId;
    if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableVtcamMapDbHndl,
                                         (xpsDbKey_t)&tableVtcamMapDbKey, (void**)&tableVtcamMapDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }
    if (tableVtcamMapDbEntry == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Db does not exist for groupId : %d error : %d  \n ", groupId, xpsRetVal);
        return XP_ERR_KEY_NOT_FOUND;
    }

    for (uint8_t i=0; i< tableVtcamMapDbEntry->tableCount; i++)
    {
        /* All table bind to port will goto group 0.
           Hence skip, other tables */
        if (tableId != tableVtcamMapDbEntry->tableId[i])
        {
            continue;
        }

        tableIdDbKey.tableId = tableVtcamMapDbEntry->tableId[i];
        if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                             (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
            return xpsRetVal;
        }

        if (tableIdDbEntry != NULL)
        {
            if (tableIdDbEntry->tableId != 0)
            {
                cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

                portGroup = (cpssPortNum)/XPS_PORT_LIST_OFFSET;
                offset = (0x1)<<((cpssPortNum)%(XPS_PORT_LIST_OFFSET));
                //if adding a port to a portlist
                if (enable)
                {
                    tableIdDbEntry->portGroupBmp[portGroup]++;
                    tableIdDbEntry->portListBmp[portGroup] |= ((portGroup << XPS_PORT_LIST_OFFSET)|
                                                               offset);
                }
                //if removing a port from port list
                else
                {
                    tableIdDbEntry->portListBmp[portGroup] &= (~offset);
                }
                /* Update hw entries using numEntries and rule Id from xpsAclTableContextDbEntry_t Db */
                for (uint32_t j =0; j< tableIdDbEntry->numEntries; j++)
                {
                    ruleId = tableIdDbEntry->ruleId[j];

                    /* Retrieving key fields and action info */
                    tableEntryDbKey.key = XPS_PCL_TABLE_ID_KEY(tableIdDbKey.tableId, ruleId);

                    if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableEntryDbHndl,
                                                         (xpsDbKey_t)&tableEntryDbKey, (void**)&tableEntryDbEntry)) != XP_NO_ERR))
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
                        return xpsRetVal;
                    }

                    if (tableEntryDbEntry == NULL)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Retrieving ACL table entry failed, devId(%d)", devId);
                        return XP_ERR_KEY_NOT_FOUND;
                    }

                    actionPtr = tableEntryDbEntry -> action;
                    priority = tableEntryDbEntry -> priority;
                    keyFormat = tableEntryDbEntry -> keyFormat;

                    /* Call writeEntryKey and write as new entry for all rule ids passed. */
                    if ((xpsRetVal = xpsAclWriteEntry(devId, tableIdDbKey.tableId, priority, ruleId,
                                                      &tableEntryDbEntry -> rule, actionPtr, keyFormat, is_control_acl)) != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "failed to write Entry in tableId :%d\n", tableIdDbKey.tableId);
                        return xpsRetVal;
                    }

                    /* If Egress and Mirror session is set, add the port to EPCL MTAG Rule */
                    if (actionPtr.mirror.mirrorToTxAnalyzerPortEn == GT_TRUE)
                    {
                        xpsRetVal = xpsAclErspanRuleUpdate(devId, portNum, enable,
                                                           actionPtr.mirror.egressMirrorToAnalyzerIndex);
                        if (xpsRetVal != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Failed to enable mirror on port %d with error code %d\n", portNum, xpsRetVal);
                            return xpsRetVal;
                        }
                    }
                }
                if (enable == false)
                {
                    tableIdDbEntry->portGroupBmp[portGroup]--;
                }
            }

            if (tableIdDbEntry->isMirror)
            {
                xpsRetVal = xpsMirrorUpdateErSpan2Index(devId, portNum,
                                                        (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)?true:false, enable);
                if (xpsRetVal != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "xpsMirrorUpdateErSpan2Index Failed %d \n", xpsRetVal);
                    return (xpsRetVal);
                }
            }
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclUnbindIntfObjAcl(xpsDevice_t devId, uint32_t intfId,
                                 uint32_t tableId, uint32_t groupId, bool isVlan,
                                 xpsAclTableContextDbEntry_t *tableIdDbEntry,
                                 xpsLagPortIntfList_t *portList)
{
    XP_STATUS                               xpsRetVal = XP_NO_ERR;
    GT_STATUS                               rc;
    xpsAclTableVtcamMapContextDbEntry_t     tableVtcamMapDbKey;
    xpsAclTableVtcamMapContextDbEntry_t     *tableVtcamMapDbEntry = NULL;
    xpsAclEntryContextDbEntry_t              tableEntryDbKey;
    xpsAclEntryContextDbEntry_t             *tableEntryDbEntry = NULL;
    xpsAclKeyFormat                         keyFormat;
    uint32_t                                ruleId, priority, portGroupIndex;
    bool                                    is_control_acl = false;
    xpsPclAction_t                          actionPtr;
    uint32_t                                portNum;
    uint32_t                                vtcamId;
    xpsScope_t                              scopeId = 0;

    /* Get scope from Device ID */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsScopeGetScopeId failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    tableVtcamMapDbKey.groupId = groupId;
    if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableVtcamMapDbHndl,
                                         (xpsDbKey_t)&tableVtcamMapDbKey, (void**)&tableVtcamMapDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    if (tableVtcamMapDbEntry == NULL)
    {
        xpsRetVal = XP_ERR_NULL_POINTER;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Table Vtcam Mapping db already not found with err: %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    //if the table is no longer bound to any interface , remove the rules from hardware
    if (!tableIdDbEntry->bindCount)
    {
        for (uint32_t i =0; i< tableIdDbEntry->numEntries; i++)
        {
            /* Delete the entry from HW */
            /* Free table entry data from the table db */
            /* Freeing duplication of rules for all the ports of different port groups sharing the same rule */
            for (portGroupIndex = 0; portGroupIndex < XPS_PORT_LIST_PORT_GROUP_MAX;
                 portGroupIndex++)
            {
                if (tableIdDbEntry->portGroupBmp[portGroupIndex])
                {

                    /* Each newRuleId will contain PortGroupId as 4MSBs followed by ruleId in the rest 28 LSBs. */
                    ruleId = ((portGroupIndex << 28) | (tableIdDbEntry->ruleId[i] &
                                                        0xFFFFFFF)); /* copying portGroupId and ruleId */

                    if ((tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)&&
                        (tableIdDbEntry->isMirror))
                    {
                        if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
                        {
                            vtcamId = XPS_L3V6_MIRROR_ING_STG_TABLE_ID_0;
                        }
                        else
                        {
                            vtcamId = XPS_L3_MIRROR_ING_STG_TABLE_ID_0;
                        }

                        /* Deleting rule from hardware */
                        if ((rc = cpssDxChVirtualTcamRuleDelete(tableIdDbEntry->tcamMgrId, vtcamId,
                                                                ruleId)) != GT_OK)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  " cpssDxChVirtualTcamRuleDelete  failed with err : %d n ", rc);
                            return xpsConvertCpssStatusToXPStatus(rc);
                        }
                    }
                    else
                    {
                        for (uint32_t j=0; j<tableIdDbEntry->numOfDuplicates; j++)
                        {

                            vtcamId = tableIdDbEntry->vtcamId[j];
                            /* Deleting rule from hardware */
                            if ((rc = cpssDxChVirtualTcamRuleDelete(tableIdDbEntry->tcamMgrId,
                                                                    tableIdDbEntry->vtcamId[j], ruleId)) != GT_OK)
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                      " cpssDxChVirtualTcamRuleDelete  failed with err : %d n ", rc);
                                return xpsConvertCpssStatusToXPStatus(rc);
                            }
                        }
                    }
                }
            }
            ruleId = tableIdDbEntry->ruleId[i];

            /* Retrieving key fields and action info */

            tableEntryDbKey.key = XPS_PCL_TABLE_ID_KEY(tableId, ruleId);

            if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableEntryDbHndl,
                                                 (xpsDbKey_t)&tableEntryDbKey, (void**)&tableEntryDbEntry)) != XP_NO_ERR))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
                return xpsRetVal;
            }

            if (tableEntryDbEntry == NULL)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Retrieving ACL table entry failed, devId(%d)", devId);
                return XP_ERR_KEY_NOT_FOUND;
            }

            actionPtr = tableEntryDbEntry -> action;
            /* If Egress and Mirror session is set, add the port to EPCL MTAG Rule */
            if (actionPtr.mirror.mirrorToTxAnalyzerPortEn == GT_TRUE)
            {
                for (uint32_t index = 0; index < portList->size; index++)
                {
                    portNum = portList->portIntf[index];
                    xpsRetVal = xpsAclErspanRuleUpdate(devId, portNum, false,
                                                       actionPtr.mirror.egressMirrorToAnalyzerIndex);
                    if (xpsRetVal != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Failed to enable mirror on port %d with error code %d\n", intfId, xpsRetVal);
                        return xpsRetVal;
                    }
                }
            }
        }
    }
    else
    {
        for (uint32_t i =0; i< tableIdDbEntry->numEntries; i++)
        {
            ruleId = tableIdDbEntry->ruleId[i];

            /* Retrieving key fields and action info */

            tableEntryDbKey.key = XPS_PCL_TABLE_ID_KEY(tableId, ruleId);

            if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableEntryDbHndl,
                                                 (xpsDbKey_t)&tableEntryDbKey, (void**)&tableEntryDbEntry)) != XP_NO_ERR))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
                return xpsRetVal;
            }

            if (tableEntryDbEntry == NULL)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Retrieving ACL table entry failed, devId(%d)", devId);
                return XP_ERR_KEY_NOT_FOUND;
            }

            actionPtr = tableEntryDbEntry -> action;
            priority = tableEntryDbEntry -> priority;
            keyFormat = tableEntryDbEntry -> keyFormat;

            /* Call writeEntryKey and write as new entry for all rule ids passed. */
            if ((xpsRetVal = xpsAclWriteEntry(devId, tableId, priority, ruleId,
                                              &(tableEntryDbEntry -> rule), actionPtr, keyFormat,
                                              is_control_acl)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "failed to write Entry in tableId :%d\n", tableId);
                return xpsRetVal;
            }

            /* If Egress and Mirror session is set, add the port to EPCL MTAG Rule */
            if (actionPtr.mirror.mirrorToTxAnalyzerPortEn == GT_TRUE)
            {
                for (uint32_t index = 0; index < portList->size; index++)
                {
                    portNum = portList->portIntf[index];
                    xpsRetVal = xpsAclErspanRuleUpdate(devId, portNum, false,
                                                       actionPtr.mirror.egressMirrorToAnalyzerIndex);
                    if (xpsRetVal != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Failed to enable mirror on port %d with error code %d\n", intfId, xpsRetVal);
                        return xpsRetVal;
                    }
                }
            }
        }
    }

    tableVtcamMapDbEntry->tableCount
    -=1; /* Decreasing the table count in the table group*/

    //Changing the corresponding vtcam region mapping for the unbound table
    for (uint32_t i=0; i< tableIdDbEntry->numOfDuplicates; i++)
    {
        /* In mirror case, vtcam region mapping already updated in API,
           xpsAclUpdateMirrorTableVtcamMap. Hence skip here to avoid
           redundant decrement.
         */
        if ((!tableIdDbEntry->isMirror) &&
            (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS))
        {
            if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
            {
                if (tableVtcamMapDbEntry->vTcamRegionIngressL3V6[tableIdDbEntry->vtcamId[i] -
                                                                                            XPS_ING_STG_V6_USER_ACL_TABLE_START] > 0)
                {
                    tableVtcamMapDbEntry->vTcamRegionIngressL3V6[tableIdDbEntry->vtcamId[i] -
                                                                                            XPS_ING_STG_V6_USER_ACL_TABLE_START]--;
                }
            }
            else if ((tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV4) ||
                     (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_NON_IP))
            {
                if (tableVtcamMapDbEntry->vTcamRegionIngressL3[tableIdDbEntry->vtcamId[i] -
                                                                                          XPS_ING_STG_V4_USER_ACL_TABLE_START] > 0)
                {
                    tableVtcamMapDbEntry->vTcamRegionIngressL3[tableIdDbEntry->vtcamId[i] -
                                                                                          XPS_ING_STG_V4_USER_ACL_TABLE_START]--;
                }
            }
        }
        else if (tableIdDbEntry->stage == XPS_PCL_STAGE_EGRESS)
        {
            if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
            {
                if (tableVtcamMapDbEntry->vTcamRegionEgressL3V6[tableIdDbEntry->vtcamId[i] -
                                                                                           XPS_EGR_STG_V6_USER_ACL_TABLE_START] > 0)
                {
                    tableVtcamMapDbEntry->vTcamRegionEgressL3V6[tableIdDbEntry->vtcamId[i] -
                                                                                           XPS_EGR_STG_V6_USER_ACL_TABLE_START]--;
                }

            }
            else if ((tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV4) ||
                     (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_NON_IP))
            {
                if (tableVtcamMapDbEntry->vTcamRegionEgressL3[tableIdDbEntry->vtcamId[i] -
                                                                                         XPS_EGR_STG_V4_USER_ACL_TABLE_START] > 0)
                {
                    tableVtcamMapDbEntry->vTcamRegionEgressL3[tableIdDbEntry->vtcamId[i] -
                                                                                         XPS_EGR_STG_V4_USER_ACL_TABLE_START]--;
                }
            }
        }
    }
    if (!(tableVtcamMapDbEntry->tableCount))
    {

        /*Deleting Vtcam Id mapping for per group Db*/
        if (((xpsRetVal = xpsStateDeleteData(scopeId, aclTableVtcamMapDbHndl,
                                             (xpsDbKey_t)&tableVtcamMapDbKey, (void**)&tableVtcamMapDbEntry)) != XP_NO_ERR))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsStateDeleteData failed with err : %d  for group id : %d\n ", xpsRetVal,
                  groupId);
            return xpsRetVal;
        }

        if (tableVtcamMapDbEntry)
        {
            if (tableVtcamMapDbEntry->tableId)
            {
                if ((xpsRetVal = xpsStateHeapFree((void*) tableVtcamMapDbEntry->tableId)) !=
                    XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "XPS state Heap Free failed for table with err :%d \n", xpsRetVal);
                    xpsStateHeapFree((void*)tableVtcamMapDbEntry);
                    return xpsRetVal;
                }
            }

            if ((xpsRetVal = xpsStateHeapFree((void*) tableVtcamMapDbEntry)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "XPS state Heap Free failed with err :%d \n", xpsRetVal);
                return xpsRetVal;
            }
        }
    }

    if (tableIdDbEntry->isMirror)
    {
        if (isVlan)
        {
            xpsRetVal = xpsMirrorVlanUpdateErSpan2Index(devId, intfId,
                                                        (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)?true:false, false);
        }
        else
        {
            xpsRetVal = xpsMirrorUpdateErSpan2Index(devId, intfId,
                                                    (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)?true:false, false);
        }
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsMirrorUpdateErSpan2Index Failed %d \n", xpsRetVal);
            return (xpsRetVal);
        }
    }

    /* Reset numOfDuplicates. */
    if (!tableIdDbEntry->bindCount)
    {
        tableIdDbEntry->numOfDuplicates = 0;
    }

    return (xpsRetVal);
}

XP_STATUS xpsAclSetIntfObjAcl(xpsDevice_t devId, uint32_t intfId,
                              uint32_t tableId, bool isVlan,
                              xpsAclTableContextDbEntry_t *tableIdDbEntry,
                              xpsLagPortIntfList_t *portList)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsAclKeyFormat                         keyFormat;
    uint32_t                                ruleId, priority, index=0, portNum=0;
    bool                                    is_control_acl = false;
    xpsPclAction_t                          actionPtr;
    xpsAclEntryContextDbEntry_t              tableEntryDbKey;
    xpsAclEntryContextDbEntry_t             *tableEntryDbEntry = NULL;
    xpsScope_t                              scopeId = 0;

    /* Get scope from Device ID */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsScopeGetScopeId failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    for (uint32_t i = 0; i< tableIdDbEntry->numEntries; i++)
    {
        ruleId = tableIdDbEntry->ruleId[i];

        /* Retrieving key fields and action info */

        tableEntryDbKey.key = XPS_PCL_TABLE_ID_KEY(tableId, ruleId);

        if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableEntryDbHndl,
                                             (xpsDbKey_t)&tableEntryDbKey, (void**)&tableEntryDbEntry)) != XP_NO_ERR))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
            return xpsRetVal;
        }

        if (tableEntryDbEntry == NULL)
        {
            xpsRetVal = XP_ERR_NULL_POINTER;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Retrieving ACL table entry failed, devId(%d) err(%d)", devId, xpsRetVal);
            return xpsRetVal;
        }

        actionPtr = tableEntryDbEntry -> action;
        priority = tableEntryDbEntry -> priority;
        keyFormat = tableEntryDbEntry -> keyFormat;


        /* Call writeEntryKey and write as new entry for all rule ids passed. */
        if ((xpsRetVal = xpsAclWriteEntry(devId, tableId, priority, ruleId,
                                          &tableEntryDbEntry -> rule, actionPtr, keyFormat, is_control_acl)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "failed to write Entry in tableId :%d\n", tableId);
            return xpsRetVal;
        }

        /* If Egress and Mirror session is set, add the port to EPCL MTAG Rule */
        if (actionPtr.mirror.mirrorToTxAnalyzerPortEn == GT_TRUE)
        {
            for (index = 0; index < portList->size; index++)
            {
                portNum = portList->portIntf[index];
                xpsRetVal = xpsAclErspanRuleUpdate(devId, portNum, true,
                                                   actionPtr.mirror.egressMirrorToAnalyzerIndex);
                if (xpsRetVal != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to enable mirror on port %d with error code %d\n", portNum, xpsRetVal);
                    return xpsRetVal;
                }
            }
        }
    }

    if (tableIdDbEntry->isMirror)
    {
        if (isVlan)
        {
            xpsRetVal = xpsMirrorVlanUpdateErSpan2Index(devId, intfId,
                                                        (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)?true:false, true);
        }
        else
        {
            xpsRetVal = xpsMirrorUpdateErSpan2Index(devId, intfId,
                                                    (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)?true:false, true);
        }

        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsMirrorUpdateErSpan2Index Failed %d \n", xpsRetVal);
            return (xpsRetVal);
        }
    }

    return (xpsRetVal);
}

XP_STATUS xpsAclLagUnbind(xpsDevice_t devId, uint32_t lagId,
                          uint32_t groupId, uint32_t tableId)
{
    XP_STATUS                               xpStatus = XP_NO_ERR;
    xpsLagPortIntfList_t                    portList;
    XP_STATUS                               xpsRetVal = XP_NO_ERR;
    uint32_t                                portGroup;
    uint32_t                                offset;
    xpsAclTableContextDbEntry_t             tableIdDbKey;
    xpsAclTableContextDbEntry_t             *tableIdDbEntry = NULL;
    xpsScope_t                              scopeId = 0;
    uint32_t                                portNum;
    uint32_t                                cpssPortNum;

    memset(&portList, 0, sizeof(portList));

    /* Get the Lag member port list */
    xpStatus = xpsLagGetPortIntfList(lagId, &portList);
    if (xpStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to Lag port members, "
              "device:%d, lag:%d, pcl:%d, error:%d\n ", devId, lagId, xpStatus);
        return xpStatus;

    }

    if (!portList.size)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "no lag member for lagId ::%d",
              lagId);
    }
    /* Get scope from Device ID */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        return xpsRetVal;
    }

    /* Retrieving table info */
    tableIdDbKey.tableId = tableId ;
    if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                         (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    if (tableIdDbEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    //decrease the bindcount per table
    if (tableIdDbEntry->bindCount)
    {
        tableIdDbEntry->bindCount--;
    }

    //remove the unbound port from the portlist and update the rules in hw
    for (uint32_t i = 0; i < portList.size; i++)
    {
        portNum = portList.portIntf[i];
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

        portGroup = (cpssPortNum)/XPS_PORT_LIST_OFFSET;
        offset = (0x1)<<((cpssPortNum)%(XPS_PORT_LIST_OFFSET));
        if (tableIdDbEntry->portGroupBmp[portGroup])
        {
            tableIdDbEntry->portListBmp[portGroup] &= (~offset);
        }
    }

    xpsRetVal = xpsAclUnbindIntfObjAcl(devId, lagId, tableId, groupId, false,
                                       tableIdDbEntry, &portList);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Lag xpsAclUnbindIntfObjAcl error:%d\n", xpsRetVal);
        /*Revert bind configuration*/
        for (uint32_t i = 0; i < portList.size; i++)
        {
            portNum = portList.portIntf[i];
            cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

            portGroup = (cpssPortNum)/XPS_PORT_LIST_OFFSET;
            offset = (0x1)<<((cpssPortNum)%(XPS_PORT_LIST_OFFSET));
            if (tableIdDbEntry->portGroupBmp[portGroup])
            {
                tableIdDbEntry->portListBmp[portGroup] |= ((portGroup << XPS_PORT_LIST_OFFSET)|
                                                           offset);
            }
        }
        tableIdDbEntry->bindCount++;

        return xpsRetVal;
    }

    for (uint32_t i = 0; i < portList.size; i++)
    {
        portNum = portList.portIntf[i];
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
        portGroup = (cpssPortNum)/XPS_PORT_LIST_OFFSET;
        if (tableIdDbEntry->portGroupBmp[portGroup])
        {
            tableIdDbEntry->portGroupBmp[portGroup]--;
        }
    }
    return XP_NO_ERR;
}
XP_STATUS xpsAclSetLagAcl(xpsDevice_t devId, uint32_t lagId,
                          uint32_t tableId, uint32_t groupId)
{
    xpsLagPortIntfList_t                    portList;
    XP_STATUS                               xpsRetVal = XP_NO_ERR;
    uint32_t                                portGroup;
    uint32_t                                offset;
    xpsAclTableContextDbEntry_t             tableIdDbKey;
    xpsAclTableContextDbEntry_t             *tableIdDbEntry = NULL;
    uint32_t portNum;
    uint32_t                                cpssPortNum;
    xpsScope_t                              scopeId = 0;

    /* Get scope from Device ID */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsScopeGetScopeId failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    /* Get the Lag member port list */
    memset(&portList, 0, sizeof(portList));
    xpsRetVal = xpsLagGetPortIntfList(lagId, &portList);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to Lag port members, "
              "device:%d, lag:%d, pcl:%d, error:%d\n ", devId, lagId, xpsRetVal);
        return xpsRetVal;

    }

    if (!portList.size)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "no lag member for lagId ::%d",
              lagId);
    }

    xpsRetVal = xpsAclUpdateTableVtcamMapping(devId, tableId, groupId);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the port-portgroup mapping for portlist. error:%d\n", xpsRetVal);
        return xpsRetVal;
    }

    /* Retrieving table info */
    tableIdDbKey.tableId = tableId ;
    if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                         (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    if (tableIdDbEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    for (uint32_t index = 0; index < portList.size; index++)
    {
        portNum = portList.portIntf[index];
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

        portGroup = (cpssPortNum)/XPS_PORT_LIST_OFFSET;
        offset = (0x1)<<((cpssPortNum)%(XPS_PORT_LIST_OFFSET));
        tableIdDbEntry->portGroupBmp[portGroup]++;
        tableIdDbEntry->portListBmp[portGroup] |= ((portGroup << XPS_PORT_LIST_OFFSET)|
                                                   offset);
    }

    xpsRetVal = xpsAclSetIntfObjAcl(devId, lagId, tableId, false, tableIdDbEntry,
                                    &portList);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Lag xpsAclSetIntfObjAcl error:%d\n", xpsRetVal);
        for (uint32_t index = 0; index < portList.size; index++)
        {
            portNum = portList.portIntf[index];
            cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

            portGroup = (cpssPortNum)/XPS_PORT_LIST_OFFSET;
            offset = (0x1)<<((cpssPortNum)%(XPS_PORT_LIST_OFFSET));
            tableIdDbEntry->portListBmp[portGroup] &= (~offset);
            tableIdDbEntry->portGroupBmp[portGroup]--;
        }
        tableIdDbEntry->bindCount--;
        return xpsRetVal;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclUdbKeysForIpv4Init(xpsDevice_t devNum, uint32_t direction,
                                   uint32_t lookupNum, uint32_t packetType)
{
    GT_STATUS                      status = GT_OK;
    GT_U8                          offset = 0; /* UDB offset from anchor */
    GT_U32                         udbIndex = 0;
    GT_U32                         index = 0;
    GT_U32                         offsetIndex = 0;
    CPSS_DXCH_PCL_UDB_SELECT_STC   udbSelectStruct;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT  offsetType;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    CPSS_PP_FAMILY_TYPE_ENT devType = cpssHalDevPPFamilyGet(devNum);

    cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

    /* Key format for IPv4 flows */

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
    }
    else
    {
        ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E;
    }

    /* UDB[31-34] offset for PortList Metadata */
    for (udbIndex = 31, offset = 0 ; udbIndex < 35; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset /* PortList */);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[48-49] offset for VLAN-ID */
    offset = (direction == CPSS_PCL_DIRECTION_INGRESS_E) ? 99 : 26;
    for (udbIndex = 48; udbIndex < 50; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* vlan-id */
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[36] offset for Applicable Flow Sub-Template Metadata/ Is IPv4 */
    offset = (direction == CPSS_PCL_DIRECTION_INGRESS_E) ? 91 : 40;
    for (udbIndex = 36 ; udbIndex < 37; udbIndex++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[16] offset for IP protocol number Metadata */
    for (udbIndex = 16, offset = 11 ; udbIndex < 17; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                              offset /* IP protocol number */);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /*TODO: PCL UP0 is un-used now. This udb index is free to use */
    /* UDB[37] offset for UP0 Metadata - Ingress */
    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        offset = 105; /* UP */
    }
    /* UDB[37] offset for Marvell DSA tag cmd - Egress */
    else if ((direction == CPSS_PCL_DIRECTION_EGRESS_E) &&
             (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
              devType == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        offset = 64; /* DSA tag Cmd */
    }

    for (udbIndex = 37; udbIndex < 38; udbIndex++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[3-6] offset for Source IPv4 address */
    for (udbIndex = 3, offset = 14 ; udbIndex < 7; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* SIPv4 */
                                              CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[7-10] offset for Destination IPv4 address */
    for (udbIndex = 7, offset = 18 ; udbIndex < 11; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* DIPv4 */
                                              CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[11] offset for DSCP */
    for (udbIndex = 11, offset = 3 ; udbIndex < 12; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* DSCP */
                                              CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[12/38] offset for Flags */
    offsetIndex = (packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E) ? 13 : 51;
    offsetType = (packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E) ?
                 CPSS_DXCH_PCL_OFFSET_L4_E : CPSS_DXCH_PCL_OFFSET_METADATA_E;
    udbIndex = (packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E) ? 12 : 38;
    offset = offsetIndex;
    status = cpssHalPclUserDefinedByteSet(devNum,
                                          ruleFormat,
                                          (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                          (CPSS_PCL_DIRECTION_ENT)direction,
                                          udbIndex,/* Flags */
                                          offsetType,
                                          offset);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set user defined byte,"
              " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
              direction, udbIndex, offset, status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    /* UDB[39] offset for L4 Valid Metadata */
    offset = (direction == CPSS_PCL_DIRECTION_INGRESS_E) ? 51 : 41;
    for (udbIndex = 39; udbIndex < 40; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset /* L4 Valid */);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB{[13-14]/[40-41]} offset for TCP/UDP Src Port */
    offsetIndex = (packetType != CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E) ? 0 : 48;
    offsetType = (packetType != CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E) ?
                 CPSS_DXCH_PCL_OFFSET_L4_E:CPSS_DXCH_PCL_OFFSET_METADATA_E;
    udbIndex = (packetType != CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E) ? 13 : 40;
    for (index = 0, offset = offsetIndex ; index < 2 ;
         udbIndex++, offset++, index++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              offsetType,
                                              offset);
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB{[19-20]/[42-43]} offset for TCP/UDP Dst Port */
    offsetIndex = (packetType != CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E) ? 2 : 48;
    offsetType = (packetType != CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E) ?
                 CPSS_DXCH_PCL_OFFSET_L4_E:CPSS_DXCH_PCL_OFFSET_METADATA_E;
    udbIndex = (packetType != CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E) ? 19 : 42;
    for (index = 0, offset = offsetIndex ; index < 2; udbIndex++, offset++, index++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              offsetType,
                                              offset);
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB{[17-18]/[44-45]} offset for ICMP type and code*/
    if (!cpssHalIsEbofDevice(devNum))
    {
        offsetIndex = (packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E) ? 0 : 44;
        offsetType = (packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E) ?
                     CPSS_DXCH_PCL_OFFSET_L4_E:CPSS_DXCH_PCL_OFFSET_METADATA_E;
        udbIndex = (packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E) ? 17 : 44;
    }
    else
    {
        switch (packetType)
        {
            case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E:
                offsetIndex = 0;
                offsetType = CPSS_DXCH_PCL_OFFSET_L4_E;
                udbIndex = 17;
                break;
            /* CNP*/
            case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E:
                offsetIndex = 44;
                offsetType = CPSS_DXCH_PCL_OFFSET_METADATA_E;
                udbIndex = 44;
                break;
            case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E:
            default:
                offsetIndex = 8;
                offsetType = CPSS_DXCH_PCL_OFFSET_L4_E;
                udbIndex = 45;
        }
    }

    for (index = 0, offset = offsetIndex ; index < 2; udbIndex++, offset++, index++)
    {

        if (cpssHalIsEbofDevice(devNum))
        {
            /* patch to add UDB for offset byte 12 in payload of UDP packet */
            if ((packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E) &&
                (direction == CPSS_PCL_DIRECTION_EGRESS_E))
            {
                if (offset == 9)
                {
                    /* udbSelectArr[44] = UDB[28] = offset 20 from L4 offset packet start, mapped to XPS_PCL_ICMP_MSG_TYPE
                       udbSelectArr[45] = UDB[29] = offset 8 from L4 offset packet start, mapped to XPS_PCL_ICMP_CODE */
                    offset = 20;
                    udbIndex = 44;
                }
            }
        }

        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* ICMP type and code */
                                              offsetType,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[46-47] offset for EtherType Metadata */
    offset = (direction == CPSS_PCL_DIRECTION_INGRESS_E) ? 24 : 22;
    for (udbIndex = 46; udbIndex < 48; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset /* Ether type */);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[35] offset for CPU Code metadata field for Egress */
    if ((direction == CPSS_PCL_DIRECTION_EGRESS_E) &&
        (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
         devType == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        for (udbIndex = 35, offset = 87 ; udbIndex < 36; udbIndex++, offset++)
        {
            status = cpssHalPclUserDefinedByteSet(devNum,
                                                  ruleFormat,
                                                  (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                                  (CPSS_PCL_DIRECTION_ENT)direction,
                                                  udbIndex,/* CPU Code */
                                                  CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                                  offset);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set user defined byte,"
                      " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                      direction, udbIndex, offset, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
        }
        udbIndex = 30;
        offset=53;
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* CPU Code */
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* it's a must to enable UDB's */
    for (index = 0, udbIndex = 31; index < 4; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex; /* PortList */
    }
    if ((direction == CPSS_PCL_DIRECTION_EGRESS_E) &&
        (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
         devType == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        udbSelectStruct.udbSelectArr[4]         = 30;      /* EPORT [7:1] */
    }
    udbSelectStruct.udbSelectArr[5]         = 35;      /* CPU Code */
    udbSelectStruct.udbSelectArr[6]         = 48;       /* VLAN-ID */
    udbSelectStruct.udbSelectArr[7]         = 49;       /* VLAN-ID */
    udbSelectStruct.udbSelectArr[8]         = 46;      /* Ethertype */
    udbSelectStruct.udbSelectArr[9]         = 47;      /* Ethertype */
    udbSelectStruct.udbSelectArr[10]        =
        36;      /* Applicable Flow Sub-Template or Is IPv4 */
    udbSelectStruct.udbSelectArr[11]        = 16;       /* IP protocol number */
    udbSelectStruct.udbSelectArr[12]        = 37;      /* UP0 */
    for (index = 13, udbIndex = 3; index < 17; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex; /* SIPv4 */
    }
    for (index = 17, udbIndex = 7; index < 21; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex; /* DIPv4 */
    }
    udbSelectStruct.udbSelectArr[21]        = 11;      /* DSCP */
    udbSelectStruct.udbSelectArr[22]        = 39;      /* L4 Valid */
    udbIndex = (packetType != CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E) ? 13 : 40;
    for (index = 23; index < 25; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex; /* TCP Src Port */
    }
    udbIndex = (packetType != CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E) ? 19 : 42;
    for (index = 25; index < 27; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex; /* TCP Dst Port */
    }
    udbSelectStruct.udbSelectArr[27]        = (packetType ==
                                               CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E) ? 12 : 38; /* Flags */
    udbIndex = (packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E) ? 17 : 44;
    for (index = 28; index < 30; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex; /* ICMP Msg & code */
    }

    status = cpssHalPclUserDefinedBytesSelectSet(devNum,
                                                 ruleFormat,
                                                 (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                                 (CPSS_PCL_LOOKUP_NUMBER_ENT)lookupNum,
                                                 &udbSelectStruct);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set user defined byte Select Arr,"
              " device:%d, direction:%d, lookupNum:%d, error:%d\n ", devNum, direction,
              lookupNum,
              status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    return XP_NO_ERR;

}

XP_STATUS xpsAclUdbKeysForIpv6Init(xpsDevice_t devNum, uint32_t direction,
                                   uint32_t lookupNum, uint32_t packetType)
{
    GT_STATUS                      status = GT_OK;
    GT_U8                          offset = 0; /* UDB offset from anchor */
    GT_U32                         udbIndex = 0;
    GT_U32                         index = 0;
    GT_U32                         offsetIndex = 0;
    CPSS_DXCH_PCL_UDB_SELECT_STC   udbSelectStruct;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT  offsetType;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    CPSS_PP_FAMILY_TYPE_ENT devType = cpssHalDevPPFamilyGet(devNum);

    cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

    /* Key format for IPv6 flows */
    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E;

    }
    else
    {
        ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E;
    }

    /* UDB[33-36] offset for PortList Metadata */
    for (udbIndex = 33, offset = 0 ; udbIndex < 37; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset /* PortList */);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[37] offset for Trg Phy Port Metadata */
    for (udbIndex = 37, offset = 17 ; udbIndex < 38; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset /* Trg Phy Port */);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[38] offset for Applicable Flow Sub-Template Metadata/ Is IPv6 */
    offset = (direction == CPSS_PCL_DIRECTION_INGRESS_E) ? 91 : 40;
    for (udbIndex = 38; udbIndex < 39; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[16] offset for Next Header */
    for (udbIndex = 16, offset = 8 ; udbIndex < 17; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* Next Header */
                                              CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /*TODO: PCL UP0 is un-used now. This udb index is free to use */
    /* UDB[39] offset for UP0 Metadata */
    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        offset = 105; /* UP0 */
    }
    /* UDB[39] offset for Marvell DSA tag cmd - Egress */
    else if ((direction == CPSS_PCL_DIRECTION_EGRESS_E) &&
             (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
              devType == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        offset = 64; /* DSA tag Cmd */
    }
    for (udbIndex = 39; udbIndex < 40; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[0-15] offset for Source IPv6 address */
    for (udbIndex = 0, offset = 10 ; udbIndex < 16; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* SIPv6 */
                                              CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[17-32] offset for Destination IPv6 address */
    for (udbIndex = 17, offset = 26 ; udbIndex < 33; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* DIPv6 */
                                              CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[40] offset for L4 Valid Metadata */
    offset = (direction == CPSS_PCL_DIRECTION_INGRESS_E) ? 51 : 41;
    for (udbIndex = 40; udbIndex < 41; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset /* L4 Valid */);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[41-42] offset for TCP/UDP Src Port */
    offsetIndex = (packetType != CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E) ? 0 : 48;
    offsetType  = (packetType != CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E) ?
                  CPSS_DXCH_PCL_OFFSET_L4_E:CPSS_DXCH_PCL_OFFSET_METADATA_E;
    for (udbIndex = 41, offset = offsetIndex ; udbIndex < 43 ; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              offsetType,
                                              offset /* L4 Src Port */);
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[43-44] offset for TCP/UDP Dst Port */
    offsetIndex = (packetType != CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E) ? 2 : 48;
    offsetType  = (packetType != CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E) ?
                  CPSS_DXCH_PCL_OFFSET_L4_E:CPSS_DXCH_PCL_OFFSET_METADATA_E;
    for (udbIndex = 43, offset = offsetIndex ; udbIndex < 45; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              offsetType,
                                              offset /* L4 Dst Port */);
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[45] offset for Flags */
    offsetIndex = (packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E) ? 13 : 51;
    offsetType  = (packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E) ?
                  CPSS_DXCH_PCL_OFFSET_L4_E : CPSS_DXCH_PCL_OFFSET_METADATA_E;
    for (udbIndex = 45, offset = offsetIndex ; udbIndex < 46; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* Flags */
                                              offsetType,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[46-47] offset for ICMP type and code*/
    if (!cpssHalIsEbofDevice(devNum))
    {
        offsetIndex = (packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E) ? 0 : 44;
        offsetType  = (packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E) ?
                      CPSS_DXCH_PCL_OFFSET_L4_E:CPSS_DXCH_PCL_OFFSET_METADATA_E;
    }
    else
    {
        switch (packetType)
        {
            case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E:
                offsetIndex = 0;
                offsetType = CPSS_DXCH_PCL_OFFSET_L4_E;
                break;
            case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E:
                offsetIndex = 44;
                offsetType = CPSS_DXCH_PCL_OFFSET_METADATA_E;
                break;
            /* CNP*/
            case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E:
            default:
                offsetIndex = 8;
                offsetType = CPSS_DXCH_PCL_OFFSET_L4_E;
        }
    }

    for (udbIndex = 46, offset = offsetIndex ; udbIndex < 48; udbIndex++, offset++)
    {

        if (cpssHalIsEbofDevice(devNum))
        {
            /* patch to add UDB for offset byte 12 in payload of UDP packet */
            if ((packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E) &&
                (direction == CPSS_PCL_DIRECTION_EGRESS_E))
            {
                if (offset == 9)
                {
                    /*  udbSelectArr[47] = UDB[46] = offset 20 from L4 offset packet start, mapped to XPS_PCL_ICMPV6_CODE
                        udbSelectArr[46] = UDB[45] = offset 8 from L4 offset packet start, mapped to XPS_PCL_ICMPV6_TYPE*/
                    offset = 20;
                }
            }
        }

        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* DSCP */
                                              offsetType,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[48] offset for CPU Code metadata field for Egress */
    if ((direction == CPSS_PCL_DIRECTION_EGRESS_E) &&
        (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
         devType == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        for (udbIndex = 48, offset = 87 ; udbIndex < 49; udbIndex++, offset++)
        {
            status = cpssHalPclUserDefinedByteSet(devNum,
                                                  ruleFormat,
                                                  (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                                  (CPSS_PCL_DIRECTION_ENT)direction,
                                                  udbIndex,/* CPU Code */
                                                  CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                                  offset);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set user defined byte,"
                      " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                      direction, udbIndex, offset, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
        }
        udbIndex = 49;
        offset=53;
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormat,
                                              (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* CPU Code */
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* it's a must to enable UDB's */
    for (index = 0, udbIndex = 33; index < 4; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex; /* PortList */
    }
    udbSelectStruct.udbSelectArr[4]        = 37;      /* Trg Phy Port */
    udbSelectStruct.udbSelectArr[5]        =
        38;      /* Applicable Flow Sub-Template or Is IPv6 */
    udbSelectStruct.udbSelectArr[6]        = 16;      /* Next Header */
    udbSelectStruct.udbSelectArr[7]        = 39;      /* UP0 / DSA-Tag cmd */

    for (index = 8, udbIndex = 0; index < 24; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex; /* SIPv6 */
    }

    for (index = 24, udbIndex = 17; index < 40; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex; /* DIPv6 */
    }
    udbSelectStruct.udbSelectArr[40]     = 40;      /* L4 Valid */

    for (index = 41, udbIndex = 41; index < 43; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex; /* TCP/UDP Src Port */
    }

    for (index = 43, udbIndex = 43; index < 45; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex; /* TCP/UDP Dst Port */
    }

    udbSelectStruct.udbSelectArr[45]        = 45;       /* Flags */

    for (index = 46, udbIndex = 46; index < 48; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex;      /* ICMP type & code */
    }

    if ((direction == CPSS_PCL_DIRECTION_EGRESS_E) &&
        (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
         devType == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        udbSelectStruct.udbSelectArr[48]        = 48;      /* CPU Code */
        udbSelectStruct.udbSelectArr[49]        = 49;      /* EPORT [7:1] */
    }

    status = cpssHalPclUserDefinedBytesSelectSet(devNum,
                                                 ruleFormat,
                                                 (CPSS_DXCH_PCL_PACKET_TYPE_ENT)packetType,
                                                 (CPSS_PCL_LOOKUP_NUMBER_ENT)lookupNum,
                                                 &udbSelectStruct);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set user defined byte Select Arr,"
              " device:%d, direction:%d, lookupNum:%d, error:%d\n ", devNum, direction,
              lookupNum,
              status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclUdbKeysForArpInit(xpsDevice_t devNum, uint32_t direction,
                                  uint32_t lookupNum)
{
    GT_STATUS                      status = GT_OK;
    GT_U8                          offset = 0; /* UDB offset from anchor */
    GT_U32                         udbIndex = 0;
    GT_U32                         arpEthertype = 0x0806;
    int                            index = 0;
    CPSS_DXCH_PCL_UDB_SELECT_STC   udbSelectStruct;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormatType;
    CPSS_PP_FAMILY_TYPE_ENT devType = cpssHalDevPPFamilyGet(devNum);

    cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

    /* Key format for ARP flows */
    if (direction == XPS_ACL_DIRECTION_INGRESS)
    {
        ruleFormatType = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
    }
    else
    {
        ruleFormatType = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E;
    }

    /* UDB[40-43] offset for PortList Metadata */
    for (udbIndex = 40, offset = 0 ; udbIndex < 44; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset /* PortList */);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    offset = (direction == CPSS_PCL_DIRECTION_INGRESS_E) ? 99 : 26;
    /* UDB[48-49] offset for VLAN-ID */
    for (udbIndex = 48; udbIndex < 50; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* vlan-id */
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[45] offset for CPU Code Metadata */
    offset = (direction == CPSS_PCL_DIRECTION_INGRESS_E) ? 96 : 87;
    for (udbIndex = 45; udbIndex < 46; udbIndex++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset /* Reserved */);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[46] offset for Reserved Metadata */
    offset = (direction == CPSS_PCL_DIRECTION_INGRESS_E) ? 96 : 89;
    for (udbIndex = 46; udbIndex < 47; udbIndex++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset /* Reserved */);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[44] offset for Applicable Flow Sub-Template Metadata/ Egress UDB Packet Type */
    offset = (direction == CPSS_PCL_DIRECTION_INGRESS_E) ? 91 : 19;
    for (udbIndex = 44, offset = offset ; udbIndex < 45; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[2-3] offset for opCode */
    for (udbIndex = 2, offset = 9 ; udbIndex < 4; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* opCode */
                                              CPSS_DXCH_PCL_OFFSET_L2_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[4-9] offset for Source Mac address */
    for (udbIndex = 4, offset = 6 ; udbIndex < 10; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* SMAC */
                                              CPSS_DXCH_PCL_OFFSET_L2_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[14-19] offset for Destination Mac address */
    for (udbIndex = 14, offset = 0 ; udbIndex < 19; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* DMAC */
                                              CPSS_DXCH_PCL_OFFSET_L2_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }
    if ((direction == CPSS_PCL_DIRECTION_EGRESS_E) &&
        (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
         devType == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        udbIndex = 39;
        offset=53;
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* */
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }

        udbIndex = 38;
        offset=64;
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              ruleFormatType,
                                              CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E,
                                              (CPSS_PCL_DIRECTION_ENT)direction,
                                              udbIndex,/* DSA-Tag cmd*/
                                              CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, direction:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  direction, udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }
    /* it's a must to enable UDB's */
    for (index = 0, udbIndex = 40; index < 4; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex;      /* PortList */
    }
    udbSelectStruct.ingrUdbReplaceArr[4]    = GT_TRUE; /* Source ePort */
    udbSelectStruct.ingrUdbReplaceArr[5]    = GT_TRUE; /* Source ePort */
    udbSelectStruct.udbSelectArr[6]         = 48;       /* VLAN-ID */
    udbSelectStruct.udbSelectArr[7]         = 49;       /* VLAN-ID */
    udbSelectStruct.udbSelectArr[8]         = 45;      /* CPU Code for Egress */
    udbSelectStruct.udbSelectArr[9]         = 46;      /* Reserved */
    udbSelectStruct.udbSelectArr[10]        =
        44;      /* Applicable Flow Sub-Template or Egress UDB Packet Type */
    udbSelectStruct.udbSelectArr[11]        = 2;       /* opCode */
    udbSelectStruct.udbSelectArr[12]        = 3;       /* opCode */
    for (index = 13, udbIndex = 4; index < 19; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex;/* SMAC */
    }
    for (index = 19, udbIndex = 14; index < 25; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex;/* DMAC */
    }

    if ((direction == CPSS_PCL_DIRECTION_EGRESS_E) &&
        (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
         devType == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        udbSelectStruct.udbSelectArr[25]        = 39;      /* EPORT[7:1] */
    }

    if ((direction == CPSS_PCL_DIRECTION_EGRESS_E) &&
        (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
         devType == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        udbSelectStruct.udbSelectArr[26]         = 38;       /* DSA-TAG Cmd */
    }

    status = cpssHalPclUserDefinedBytesSelectSet(devNum,
                                                 ruleFormatType,
                                                 CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E,
                                                 (CPSS_PCL_LOOKUP_NUMBER_ENT)lookupNum,
                                                 &udbSelectStruct);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set user defined byte Select Arr,"
              " device:%d, direction:%d, lookupNum:%d, error:%d\n ", devNum, direction,
              lookupNum,
              status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    /* Set UDE 1 for ARP ether type*/
    status = cpssHalPclUdeEtherTypeSet(devNum, 1, arpEthertype);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set user defined Ethertype "
              " device:%d, ethertype:%d, error:%d\n ", devNum, arpEthertype, status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclUdbKeysForVxlanIpv6Init(xpsDevice_t devNum)
{
    GT_STATUS                      status = GT_OK;
    GT_U8                          offset = 0; /* UDB offset from anchor */
    GT_U32                         udbIndex = 0;
    int                            index = 0;
    CPSS_DXCH_PCL_UDB_SELECT_STC   udbSelectStruct;

    cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

    /* UDB[2-4] VNI ID */
    for (udbIndex = 2, offset = 54 ; udbIndex < 5; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                              XPS_VXLAN_IPV6_PCL_UDB_PACKET_TYPE_CNS,
                                              CPSS_PCL_DIRECTION_INGRESS_E,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* UDB[5-20] offset for SIP */
    for (udbIndex = 5, offset = 10 ; udbIndex < 21; udbIndex++, offset++)
    {
        status = cpssHalPclUserDefinedByteSet(devNum,
                                              CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                              XPS_VXLAN_IPV6_PCL_UDB_PACKET_TYPE_CNS,
                                              CPSS_PCL_DIRECTION_INGRESS_E,
                                              udbIndex,
                                              CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E,
                                              offset);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,"
                  " device:%d, udbIndex:%d, offset:%d, error:%d\n ", devNum,
                  udbIndex, offset, status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    for (index = 2, udbIndex = 2; index < 21; index++, udbIndex++)
    {
        udbSelectStruct.udbSelectArr[index] = udbIndex;
    }
    udbSelectStruct.ingrUdbReplaceArr[0]    = GT_TRUE; /* PCL-ID[7:0] */
    udbSelectStruct.ingrUdbReplaceArr[1]    = GT_TRUE; /* PCL-ID[9:8] */

    status = cpssHalPclUserDefinedBytesSelectSet(devNum,
                                                 CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                                 XPS_VXLAN_IPV6_PCL_UDB_PACKET_TYPE_CNS,
                                                 CPSS_PCL_LOOKUP_NUMBER_0_E,
                                                 &udbSelectStruct);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set user defined byte Select Arr,"
              " device:%d, error:%d\n ", devNum, status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    return XP_NO_ERR;
}

static XP_STATUS xpsUdbNonIpKeySetIngress(xpsAclkeyFieldList_t *field,
                                          xpsPclRuleFormatIngrUdbOnly_t *value, xpsPclRuleFormatIngrUdbOnly_t *mask)
{
    uint32_t index = 0;

    /* Reserved field */
    value->udb[11] = 0;
    mask->udb[11] = 0;

    for (index=0; index < (field->numFlds); index++)
    {
        switch (field->fldList[index].keyFlds)
        {
            case XP_PCL_PORT_BIT_MAP: /* UDB[0-3] */
                memcpy((uint8_t*)&value->udb[0], field->fldList[index].value,
                       (sizeof(uint32_t)));
                memcpy((uint8_t*)&mask->udb[0], field->fldList[index].mask, (sizeof(uint32_t)));
                break;

            case XP_PCL_SRC_PORT:
                memcpy((uint8_t*)&value->replacedFld.srcPort, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->replacedFld.srcPort, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;

            case XP_PCL_VID: /* UDB[6-7] */
            case XPS_PCL_VID1: /* UDB[6-7] */
                memcpy((uint8_t*)&value->udb[6], field->fldList[index].value, sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb[6], field->fldList[index].mask, sizeof(uint16_t));
                break;

            case XPS_PCL_IS_NON_IPV6:
            case XPS_PCL_IS_NON_IPV4:
            case XPS_PCL_IS_NON_IP:
                /* Applicable Flow Sub-Template */
                value->udb[10] = XPS_PCL_APP_FLOW_TEMP_MASK_ALL;
                mask->udb[10] =
                    XPS_PCL_APP_FLOW_ALL_NON_IP_PACKETS_MASK; /* EthernetOther and Arp */
                break;

            case XPS_PCL_ETHER_TYPE:
                memcpy((uint8_t*)&value->udb[8], field->fldList[index].value, sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb[8], field->fldList[index].mask, sizeof(uint16_t));
                break;

            case XP_PCL_UP:
                memcpy((uint8_t*)&value->udb[12], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[12], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_MAC_SA: /* UDB[13-18] */
                memcpy((uint8_t*)&value->udb[13], field->fldList[index].value,
                       (sizeof(uint8_t)*6));
                memcpy((uint8_t*)&mask->udb[13], field->fldList[index].mask,
                       (sizeof(uint8_t)*6));
                break;

            case XPS_PCL_MAC_DA: /* UDB[19-24] */
                memcpy((uint8_t*)&value->udb[19], field->fldList[index].value,
                       (sizeof(uint8_t)*6));
                memcpy((uint8_t*)&mask->udb[19], field->fldList[index].mask,
                       (sizeof(uint8_t)*6));
                break;

            case XPS_PCL_FC_OPCODE: /* UDB[26-27] */
                memcpy((uint8_t*)&value->udb[26], field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb[26], field->fldList[index].mask, sizeof(uint16_t));
                break;

            default :
                break;

        }
    }
    return XP_NO_ERR;
}

static XP_STATUS xpsUdbNonIpKeySetEgress(xpsAclkeyFieldList_t *field,
                                         xpsPclRuleFormatEgrUdbOnly_t *value, xpsPclRuleFormatEgrUdbOnly_t *mask)
{
    uint32_t index = 0, i =0 ;

    for (i = 4; i < 6; i++)
    {
        value->udb[i] =
            0; /* Masking XP_PCL_SRC_PORT as it is not applicable for EACL */
        mask->udb[i] = 0;
    }

    for (index=0; index < (field->numFlds); index++)
    {
        switch (field->fldList[index].keyFlds)
        {
            case XP_PCL_PORT_BIT_MAP: /* UDB[0-3] */
                memcpy((uint8_t*)&value->udb[0], field->fldList[index].value,
                       (sizeof(uint32_t)));
                memcpy((uint8_t*)&mask->udb[0], field->fldList[index].mask, (sizeof(uint32_t)));
                break;
#if 0
            case XP_PCL_SRC_PORT:
                memcpy((uint8_t*)&value->replacedFld.srcPort, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->replacedFld.srcPort, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;
#endif
            case XP_PCL_VID: /* UDB[6-7] */
            case XPS_PCL_VID1: /* UDB[6-7] */
                memcpy((uint8_t*)&value->udb[6], field->fldList[index].value, sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb[6], field->fldList[index].mask, sizeof(uint16_t));
                break;

            case XPS_PCL_ETHER_TYPE:
                memcpy((uint8_t*)&value->udb[8], field->fldList[index].value, sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb[8], field->fldList[index].mask, sizeof(uint16_t));
                break;

            case XPS_PCL_IS_NON_IPV6:
            case XPS_PCL_IS_NON_IPV4:
            case XPS_PCL_IS_NON_IP:
                /* Is IP */
                value->udb[10] = XPS_PCL_EGRESS_PKT_TYPE_NON_IP;
                mask->udb[10] = XPS_PCL_EGRESS_PKT_TYPE_NON_IP;
                break;

            case XPS_PCL_CPU_CODE: /* UDB[12] */
                memcpy((uint8_t*)&value->udb[12], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[12], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_EPORT: /* UDB[11] */
                memcpy((uint8_t*)&value->udb[11], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[11], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_MAC_SA: /* UDB[13-18] */
                memcpy((uint8_t*)&value->udb[13], field->fldList[index].value,
                       (sizeof(uint8_t)*6));
                memcpy((uint8_t*)&mask->udb[13], field->fldList[index].mask,
                       (sizeof(uint8_t)*6));
                break;

            case XPS_PCL_MAC_DA: /* UDB[19-24] */
                memcpy((uint8_t*)&value->udb[19], field->fldList[index].value,
                       (sizeof(uint8_t)*6));
                memcpy((uint8_t*)&mask->udb[19], field->fldList[index].mask,
                       (sizeof(uint8_t)*6));
                break;

            case XPS_PCL_DSA_CMD: /* UDB[25] */
                memcpy((uint8_t*)&value->udb[25], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[25], field->fldList[index].mask, sizeof(uint8_t));
                break;

            default :
                break;

        }
    }
    return XP_NO_ERR;
}
static XP_STATUS xpsIngressUdbIpv4KeySet(xpsAclkeyFieldList_t *field,
                                         xpsPclRuleFormatIngrUdbOnly_t *value, xpsPclRuleFormatIngrUdbOnly_t *mask)
{
    uint32_t index = 0;

    /* Applicable Flow Sub-Template */
    value->udb[10] =
        XPS_PCL_APP_FLOW_TEMP_MASK_ALL; /* Masking to match all IPv4 flows */
    mask->udb[10] = XPS_PCL_APP_FLOW_TEMP_MASK_ALL;

    for (index=0; index < (field->numFlds); index++)
    {
        switch (field->fldList[index].keyFlds)
        {
            case XP_PCL_PORT_BIT_MAP: /* UDB[0-3] */
                memcpy((uint8_t*)&value->udb[0], field->fldList[index].value,
                       (sizeof(uint32_t)));
                memcpy((uint8_t*)&mask->udb[0], field->fldList[index].mask, (sizeof(uint32_t)));
                break;

            case XP_PCL_SRC_PORT:
                memcpy((uint8_t*)&value->replacedFld.srcPort, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->replacedFld.srcPort, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;

            case XPS_PCL_IS_IPV4_TCP:
                /* Applicable Flow Sub-Template */
                value->udb[10] = XPS_PCL_APP_FLOW_TEMP_IPV4_TCP; /* IPV4-TCP */
                mask->udb[10] = XPS_PCL_APP_FLOW_TEMP_IPV4_TCP;
                break;

            case XPS_PCL_IS_IPV4_UDP:
                /* Applicable Flow Sub-Template */
                value->udb[10] = XPS_PCL_APP_FLOW_TEMP_IPV4_UDP; /* IPV4-UDP */
                mask->udb[10] = XPS_PCL_APP_FLOW_TEMP_IPV4_UDP;
                break;

            case XPS_PCL_IS_IPV4_OTHER:
                /* Applicable Flow Sub-Template */
                value->udb[10] = XPS_PCL_APP_FLOW_TEMP_IPV4_OTHER; /* IPV4-OTHER */
                mask->udb[10] = XPS_PCL_APP_FLOW_TEMP_IPV4_OTHER;
                break;

            case XPS_PCL_IS_NON_IPV6:
            case XPS_PCL_IS_IP:
            case XPS_PCL_IS_IPV4:
                /* Applicable Flow Sub-Template */
                /*masking for ipv4-other,ipv4-tcp and ipv4-udp packets*/
                value->udb[10] = XPS_PCL_APP_FLOW_TEMP_MASK_ALL; /* IPV4 */
                mask->udb[10] = XPS_PCL_APP_FLOW_ALL_IPV4_PACKETS_MASK;
                break;

            case XP_PCL_VID: /* UDB[6-7] */
            case XPS_PCL_VID1: /* UDB[6-7] */
                memcpy((uint8_t*)&value->udb[6], field->fldList[index].value, sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb[6], field->fldList[index].mask, sizeof(uint16_t));
                break;

            case XP_PCL_QOS_PROFILE:
                memcpy((uint8_t*)&value->replacedFld.qosProfile, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->replacedFld.qosProfile, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;

            case XPS_PCL_IP_PROTOCOL: /* UDB[11] */
                memcpy((uint8_t*)&value->udb[11], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[11], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XP_PCL_UP:
                memcpy((uint8_t*)&value->udb[12], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[12], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_ETHER_TYPE:
                memcpy((uint8_t*)&value->udb[8], field->fldList[index].value, sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb[8], field->fldList[index].mask, sizeof(uint16_t));
                break;

            case XPS_PCL_SIP: /* UDB[13-16] */
                memcpy((uint8_t*)&value->udb[13], field->fldList[index].value,
                       (sizeof(uint32_t)));
                memcpy((uint8_t*)&mask->udb[13], field->fldList[index].mask,
                       (sizeof(uint32_t)));
                break;

            case XPS_PCL_DIP: /* UDB[17-20] */
                memcpy((uint8_t*)&value->udb[17], field->fldList[index].value,
                       (sizeof(uint32_t)));
                memcpy((uint8_t*)&mask->udb[17], field->fldList[index].mask,
                       (sizeof(uint32_t)));
                break;

            case XPS_PCL_DSCP:
                memcpy((uint8_t*)&value->udb[21], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[21], field->fldList[index].mask, sizeof(uint8_t));
                /*DSCP value passed will be 6 bits. But udb[21] covers DSCP +ECN*/
                value->udb[21] = value->udb[21] <<2;
                mask->udb[21] = mask->udb[21] <<2;
                break;

            case XP_PCL_TCP_FLAGS:
                memcpy((uint8_t*)&value->udb[27], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[27], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_IS_L4_VALID:
                *(field->fldList[index].value) = XPS_PCL_IS_L4_VALID_IPCL_METADATA_VAL;
                *(field->fldList[index].mask) = XPS_PCL_IS_L4_VALID_IPCL_METADATA_VAL;
                memcpy((uint8_t*)&value->udb[22], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[22], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_ICMP_MSG_TYPE:
                memcpy((uint8_t*)&value->udb[28], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[28], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_ICMP_CODE:
                memcpy((uint8_t*)&value->udb[29], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[29], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE0:
                memcpy((uint8_t*)&value->udb[23], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[23], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE1:
                memcpy((uint8_t*)&value->udb[24], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[24], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE2:
                memcpy((uint8_t*)&value->udb[25], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[25], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE3:
                memcpy((uint8_t*)&value->udb[26], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[26], field->fldList[index].mask, sizeof(uint8_t));
                break;

            default :
                break;
        }
    }

    return XP_NO_ERR;
}

static XP_STATUS xpsEgressUdbIpv4KeySet(xpsAclkeyFieldList_t *field,
                                        xpsPclRuleFormatEgrUdbOnly_t *value, xpsPclRuleFormatEgrUdbOnly_t *mask)
{
    uint32_t index = 0, i =0;

    for (i = 4; i < 6; i++)
    {
        value->udb[i] =
            0; /* Masking QOS_PROFILE as it is not applicable for EACL */
        mask->udb[i] = 0;
    }
#if 0
    /* Is IPv4 */
    value->udb[10] = XPS_PCL_EGRESS_PKT_TYPE_IPV4; /* IPV4 */
    mask->udb[10] = XPS_PCL_EGRESS_PKT_TYPE_IPV4;
#endif

    for (index=0; index < (field->numFlds); index++)
    {
        switch (field->fldList[index].keyFlds)
        {
            case XP_PCL_PORT_BIT_MAP: /* UDB[0-3] */
                memcpy((uint8_t*)&value->udb[0], field->fldList[index].value,
                       (sizeof(uint32_t)));
                memcpy((uint8_t*)&mask->udb[0], field->fldList[index].mask, (sizeof(uint32_t)));
                break;
#if 0
            case XP_PCL_SRC_PORT:
                memcpy((uint8_t*)&value->replacedFld.srcPort, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->replacedFld.srcPort, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;
#endif
            case XPS_PCL_IS_IPV4_TCP:
            case XPS_PCL_IS_IPV4_UDP:
            case XPS_PCL_IS_IPV4_OTHER:
            case XPS_PCL_IS_IPV4:
            case XPS_PCL_IS_NON_IPV6:
            case XPS_PCL_IS_IP:
                /* Is IPv4 */
                value->udb[10] = XPS_PCL_EGRESS_PKT_TYPE_IPV4; /* IPV4 */
                mask->udb[10] = XPS_PCL_EGRESS_PKT_TYPE_IPV4;
                break;

            case XP_PCL_VID: /* UDB[6-7] */
            case XPS_PCL_VID1: /* UDB[6-7] */
                memcpy((uint8_t*)&value->udb[6], field->fldList[index].value, sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb[6], field->fldList[index].mask, sizeof(uint16_t));
                break;

#if 0
            case XP_PCL_QOS_PROFILE:
                memcpy((uint8_t*)&value->replacedFld.qosProfile, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->replacedFld.qosProfile, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;

#endif
            case XPS_PCL_IP_PROTOCOL: /* UDB[11] */
                memcpy((uint8_t*)&value->udb[11], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[11], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_DSA_CMD: /* UDB[12] */
                memcpy((uint8_t*)&value->udb[12], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[12], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_CPU_CODE: /* UDB[5] */
                memcpy((uint8_t*)&value->udb[5], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[5], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_EPORT: /* UDB[4] */
                memcpy((uint8_t*)&value->udb[4], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[4], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_SIP: /* UDB[13-16] */
                memcpy((uint8_t*)&value->udb[13], field->fldList[index].value,
                       (sizeof(uint32_t)));
                memcpy((uint8_t*)&mask->udb[13], field->fldList[index].mask,
                       (sizeof(uint32_t)));
                break;

            case XPS_PCL_DIP: /* UDB[17-20] */
                memcpy((uint8_t*)&value->udb[17], field->fldList[index].value,
                       (sizeof(uint32_t)));
                memcpy((uint8_t*)&mask->udb[17], field->fldList[index].mask,
                       (sizeof(uint32_t)));
                break;

            case XPS_PCL_ETHER_TYPE:
                memcpy((uint8_t*)&value->udb[8], field->fldList[index].value, sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb[8], field->fldList[index].mask, sizeof(uint16_t));
                break;

            case XPS_PCL_DSCP:
                memcpy((uint8_t*)&value->udb[21], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[21], field->fldList[index].mask, sizeof(uint8_t));
                /*DSCP value passed will be 6 bits. But udb[21] covers DSCP +ECN*/
                value->udb[21] = value->udb[21] <<2;
                mask->udb[21] = mask->udb[21] <<2;
                break;

            case XP_PCL_TCP_FLAGS:
                memcpy((uint8_t*)&value->udb[27], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[27], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_IS_L4_VALID:
                *(field->fldList[index].value) = XPS_PCL_IS_L4_VALID_EPCL_METADATA_VAL;
                *(field->fldList[index].mask) = XPS_PCL_IS_L4_VALID_EPCL_METADATA_VAL;
                memcpy((uint8_t*)&value->udb[22], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[22], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_ICMP_MSG_TYPE:
                memcpy((uint8_t*)&value->udb[28], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[28], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_ICMP_CODE:
                memcpy((uint8_t*)&value->udb[29], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[29], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE0:
                memcpy((uint8_t*)&value->udb[23], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[23], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE1:
                memcpy((uint8_t*)&value->udb[24], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[24], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE2:
                memcpy((uint8_t*)&value->udb[25], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[25], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE3:
                memcpy((uint8_t*)&value->udb[26], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[26], field->fldList[index].mask, sizeof(uint8_t));
                break;

            default :
                break;
        }
    }

    return XP_NO_ERR;
}


static XP_STATUS xpsIngressUdbIpv6KeySet(xpsAclkeyFieldList_t *field,
                                         xpsPclRuleFormatIngrUdbOnly_t *value, xpsPclRuleFormatIngrUdbOnly_t *mask)
{
    uint32_t index = 0;

    /* Applicable Flow Sub-Template */
    value->udb[5] = XPS_PCL_APP_FLOW_TEMP_IPV6;
    mask->udb[5] = XPS_PCL_APP_FLOW_TEMP_IPV6;

    for (index=0; index < (field->numFlds); index++)
    {
        switch (field->fldList[index].keyFlds)
        {
            case XP_PCL_PORT_BIT_MAP: /* UDB[0-3] */
                memcpy((uint8_t*)&value->udb[0], field->fldList[index].value,
                       (sizeof(uint32_t)));
                memcpy((uint8_t*)&mask->udb[0], field->fldList[index].mask, (sizeof(uint32_t)));
                break;

            case XP_PCL_SRC_PORT:
                memcpy((uint8_t*)&value->udb60FixedFld.srcPort, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb60FixedFld.srcPort, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;

            case XP_PCL_VID:
            case XPS_PCL_VID1:
            case XPS_PCL_FC_OPCODE:
                memcpy((uint8_t*)&value->udb60FixedFld.vid, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb60FixedFld.vid, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;

            case XP_PCL_QOS_PROFILE:
                memcpy((uint8_t*)&value->udb60FixedFld.qosProfile, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb60FixedFld.qosProfile, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;

            case XPS_PCL_DEST_PORT: /* UDB[4] */
                memcpy((uint8_t*)&value->udb[4], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[4], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_IS_IPV6_TCP: /* UDB[5-6] */
                /* Applicable Flow Sub-Template */
                value->udb[5] = XPS_PCL_APP_FLOW_TEMP_IPV6 /* IPv6 */;
                mask->udb[5] = XPS_PCL_APP_FLOW_TEMP_IPV6 /* IPv6 */;
                /* Next Header */
                value->udb[6] = XPS_PCL_NXT_HDR_IPV6_TCP /* TCP */;
                mask->udb[6] = XPS_PCL_NXT_HDR_IPV6_TCP /* TCP */;
                break;

            case XPS_PCL_IS_IPV6_UDP: /* UDB[5-6] */
                /* Applicable Flow Sub-Template */
                value->udb[5] = XPS_PCL_APP_FLOW_TEMP_IPV6 /* IPv6 */;
                mask->udb[5] = XPS_PCL_APP_FLOW_TEMP_IPV6 /* IPv6 */;
                /* Next Header */
                value->udb[6] = XPS_PCL_NXT_HDR_IPV6_UDP /* UDP */;
                mask->udb[6] = XPS_PCL_NXT_HDR_IPV6_UDP /* UDP */;
                break;

            case XPS_PCL_IP_PROTOCOL:
                memcpy((uint8_t*)&value->udb[6], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[6], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_IS_NON_IPV4:
            case XPS_PCL_IS_IP:
            case XPS_PCL_IS_IPV6:
            case XPS_PCL_IS_IPV6_OTHER: /* UDB[5] */
                /* Applicable Flow Sub-Template */
                value->udb[5] = XPS_PCL_APP_FLOW_TEMP_IPV6 /* IPv6 */;
                mask->udb[5] = XPS_PCL_APP_FLOW_TEMP_IPV6 /* IPv6 */;
                break;

            case XP_PCL_UP: /* UDB[7] */
                memcpy((uint8_t*)&value->udb[7], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[7], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_IPV6_SIP: /* UDB[8-23] */
                memcpy((uint8_t*)&value->udb[8], field->fldList[index].value,
                       (sizeof(uint8_t)*16));
                memcpy((uint8_t*)&mask->udb[8], field->fldList[index].mask,
                       (sizeof(uint8_t)*16));
                break;

            case XPS_PCL_IPV6_DIP: /* UDB[24-39] */
                memcpy((uint8_t*)&value->udb[24], field->fldList[index].value,
                       (sizeof(uint8_t)*16));
                memcpy((uint8_t*)&mask->udb[24], field->fldList[index].mask,
                       (sizeof(uint8_t)*16));
                break;

            case XPS_PCL_IS_L4_VALID: /* UDB[40] */
                *(field->fldList[index].value) = XPS_PCL_IS_L4_VALID_IPCL_METADATA_VAL;
                *(field->fldList[index].mask) = XPS_PCL_IS_L4_VALID_IPCL_METADATA_VAL;
                memcpy((uint8_t*)&value->udb[40], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[40], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE0: /* UDB[41] */
                memcpy((uint8_t*)&value->udb[41], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[41], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE1: /* UDB[42] */
                memcpy((uint8_t*)&value->udb[42], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[42], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE2: /* UDB[43] */
                memcpy((uint8_t*)&value->udb[43], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[43], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE3: /* UDB[44] */
                memcpy((uint8_t*)&value->udb[44], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[44], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XP_PCL_TCP_FLAGS: /* UDB[45] */
                memcpy((uint8_t*)&value->udb[45], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[45], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_ICMPV6_MSG_TYPE: /* UDB[46] */
                memcpy((uint8_t*)&value->udb[46], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[46], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_ICMPV6_CODE: /* UDB[47] */
                memcpy((uint8_t*)&value->udb[47], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[47], field->fldList[index].mask, sizeof(uint8_t));
                break;

            default :
                break;
        }
    }

    return XP_NO_ERR;
}

static XP_STATUS xpsEgressUdbIpv6KeySet(xpsAclkeyFieldList_t *field,
                                        xpsPclRuleFormatEgrUdbOnly_t *value, xpsPclRuleFormatEgrUdbOnly_t *mask)
{
    uint32_t index, i = 0;

    value->udb[4] =
        0; /* Masking XPS_PCL_DEST_PORT as it is not applicable for EACL */
    mask->udb[4] = 0;

    for (i = 4; i < 6; i++)
    {
        value->udb[i] =
            0; /* Masking XP_PCL_SRC_PORT as it is not applicable for EACL */
        mask->udb[i] = 0;
    }
    /* Is IPv6 */
    value->udb[5] = XPS_PCL_EGRESS_PKT_TYPE_IPV6;
    mask->udb[5] = XPS_PCL_EGRESS_PKT_TYPE_IPV6;

    for (index=0; index < (field->numFlds); index++)
    {
        switch (field->fldList[index].keyFlds)
        {
            case XP_PCL_PORT_BIT_MAP: /* UDB[0-3] */
                memcpy((uint8_t*)&value->udb[0], field->fldList[index].value,
                       (sizeof(uint32_t)));
                memcpy((uint8_t*)&mask->udb[0], field->fldList[index].mask, (sizeof(uint32_t)));
                break;
#if 0
            case XP_PCL_SRC_PORT:
                memcpy((uint8_t*)&value->udb60FixedFld.srcPort, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb60FixedFld.srcPort, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;
#endif
            case XP_PCL_VID:
            case XPS_PCL_VID1: /* UDB[6-7] */
                memcpy((uint8_t*)&value->udb60FixedFld.vid, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb60FixedFld.vid, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;
#if 0
            case XP_PCL_QOS_PROFILE:
                memcpy((uint8_t*)&value->udb60FixedFld.qosProfile, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb60FixedFld.qosProfile, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;

            case XPS_PCL_DEST_PORT: /* UDB[4] */
                memcpy((uint8_t*)&value->udb[4], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[4], field->fldList[index].mask, sizeof(uint8_t));
                break;
#endif

            case XPS_PCL_IS_IPV6_TCP: /* UDB[5-6] */
                /* Is IPv6 */
                value->udb[5] = XPS_PCL_EGRESS_PKT_TYPE_IPV6 /* IPv6 */;
                mask->udb[5] = XPS_PCL_EGRESS_PKT_TYPE_IPV6 /* IPv6 */;
                /* Next Header */
                value->udb[6] = XPS_PCL_NXT_HDR_IPV6_TCP /* TCP */;
                mask->udb[6] = XPS_PCL_NXT_HDR_IPV6_TCP /* TCP */;
                break;

            case XPS_PCL_IS_IPV6_UDP: /* UDB[5-6] */
                /* Is IPv6 */
                value->udb[5] = XPS_PCL_EGRESS_PKT_TYPE_IPV6 /* IPv6 */;
                mask->udb[5] = XPS_PCL_EGRESS_PKT_TYPE_IPV6 /* IPv6 */;
                /* Next Header */
                value->udb[6] = XPS_PCL_NXT_HDR_IPV6_UDP /* UDP */;
                mask->udb[6] = XPS_PCL_NXT_HDR_IPV6_UDP /* UDP */;
                break;

            case XPS_PCL_IS_IPV6_OTHER: /* UDB[5] */
            case XPS_PCL_IS_IPV6:
            case XPS_PCL_IS_IP: /* UDB[5] */
            case XPS_PCL_IS_NON_IPV4:
                /* Is IPv6 */
                value->udb[5] = XPS_PCL_EGRESS_PKT_TYPE_IPV6 /* IPv6 */;
                mask->udb[5] = XPS_PCL_EGRESS_PKT_TYPE_IPV6 /* IPv6 */;
                break;

            case XPS_PCL_IP_PROTOCOL:
                memcpy((uint8_t*)&value->udb[6], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[6], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_DSA_CMD:
                memcpy((uint8_t*)&value->udb[7], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[7], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_IPV6_SIP: /* UDB[8-23] */
                memcpy((uint8_t*)&value->udb[8], field->fldList[index].value,
                       (sizeof(uint8_t)*16));
                memcpy((uint8_t*)&mask->udb[8], field->fldList[index].mask,
                       (sizeof(uint8_t)*16));
                break;

            case XPS_PCL_IPV6_DIP: /* UDB[24-39] */
                memcpy((uint8_t*)&value->udb[24], field->fldList[index].value,
                       (sizeof(uint8_t)*16));
                memcpy((uint8_t*)&mask->udb[24], field->fldList[index].mask,
                       (sizeof(uint8_t)*16));
                break;

            case XPS_PCL_IS_L4_VALID: /* UDB[40] */
                *(field->fldList[index].value) = XPS_PCL_IS_L4_VALID_EPCL_METADATA_VAL;
                *(field->fldList[index].mask) = XPS_PCL_IS_L4_VALID_EPCL_METADATA_VAL;
                memcpy((uint8_t*)&value->udb[40], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[40], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE0: /* UDB[41] */
                memcpy((uint8_t*)&value->udb[41], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[41], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE1: /* UDB[42] */
                memcpy((uint8_t*)&value->udb[42], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[42], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE2: /* UDB[43] */
                memcpy((uint8_t*)&value->udb[43], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[43], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_L4_BYTE3: /* UDB[44] */
                memcpy((uint8_t*)&value->udb[44], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[44], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XP_PCL_TCP_FLAGS: /* UDB[45] */
                memcpy((uint8_t*)&value->udb[45], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[45], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_ICMPV6_MSG_TYPE: /* UDB[46] */
                memcpy((uint8_t*)&value->udb[46], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[46], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_ICMPV6_CODE: /* UDB[47] */
                memcpy((uint8_t*)&value->udb[47], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[47], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_CPU_CODE: /* UDB[48] */
                memcpy((uint8_t*)&value->udb[48], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[48], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_EPORT: /* UDB[4] */
                memcpy((uint8_t*)&value->udb[49], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[49], field->fldList[index].mask, sizeof(uint8_t));
                break;

            default :
                break;
        }
    }

    return XP_NO_ERR;
}

static XP_STATUS xpsIngressUdbArpKeySet(xpsAclkeyFieldList_t *field,
                                        xpsPclRuleFormatIngrUdbOnly_t *value, xpsPclRuleFormatIngrUdbOnly_t *mask)
{
    uint32_t index = 0, i =0;

    for (i = 8; i < 10; i++)
    {
        value->udb[i] = 0; /* Reserved */
        mask->udb[i] = 0;
    }

    for (index=0; index < (field->numFlds); index++)
    {
        switch (field->fldList[index].keyFlds)
        {
            case XP_PCL_PORT_BIT_MAP:  /* UDB[0-3] */
                memcpy((uint8_t*)&value->udb[0], field->fldList[index].value,
                       (sizeof(uint32_t)));
                memcpy((uint8_t*)&mask->udb[0], field->fldList[index].mask, (sizeof(uint32_t)));
                break;

            case XP_PCL_SRC_PORT:
                memcpy((uint8_t*)&value->replacedFld.srcPort, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->replacedFld.srcPort, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;

            case XP_PCL_VID: /* UDB[6-7] */
            case XPS_PCL_VID1: /* UDB[6-7] */
                memcpy((uint8_t*)&value->udb[6], field->fldList[index].value, sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb[6], field->fldList[index].mask, sizeof(uint16_t));
                break;

            case XPS_PCL_IS_ARP_REQUEST:
                value->udb[10] = XPS_PCL_APP_FLOW_TEMP_UDE1 /* ARP */;
                mask->udb[10] = XPS_PCL_APP_FLOW_TEMP_MASK_UDE1;
                value->udb[11] = XPS_PCL_ARP_REQUEST_OP_CODE;
                mask->udb[11] = XPS_PCL_ARP_REQUEST_OP_CODE;
                break;

            case XPS_PCL_IS_ARP_REPLY:
                value->udb[10] = XPS_PCL_APP_FLOW_TEMP_UDE1 /* ARP */;
                mask->udb[10] = XPS_PCL_APP_FLOW_TEMP_MASK_UDE1;
                value->udb[11] = XPS_PCL_ARP_REPLY_OP_CODE;
                mask->udb[11] = XPS_PCL_ARP_REPLY_OP_CODE;
                break;

            case XPS_PCL_IS_NON_IPV6:
            case XPS_PCL_IS_NON_IPV4:
            case XPS_PCL_IS_ARP:
                *(field->fldList[index].value) = XPS_PCL_APP_FLOW_TEMP_UDE1;
                *(field->fldList[index].mask) = XPS_PCL_APP_FLOW_TEMP_MASK_UDE1;
                memcpy((uint8_t*)&value->udb[10], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[10], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_ARP_OPCODE: /* UDB[11-12] */
                memcpy((uint8_t*)&value->udb[11], field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb[11], field->fldList[index].mask, sizeof(uint16_t));
                break;

            case XPS_PCL_MAC_SA: /* UDB[13-18]] */
                memcpy((uint8_t*)&value->udb[13], field->fldList[index].value,
                       (sizeof(uint8_t)*6));
                memcpy((uint8_t*)&mask->udb[13], field->fldList[index].mask,
                       (sizeof(uint8_t)*6));
                break;

            case XPS_PCL_MAC_DA: /* UDB[19-24] */
                memcpy((uint8_t*)&value->udb[19], field->fldList[index].value,
                       (sizeof(uint8_t)*6));
                memcpy((uint8_t*)&mask->udb[19], field->fldList[index].mask,
                       (sizeof(uint8_t)*6));
                break;

            default :
                break;
        }
    }

    return XP_NO_ERR;

}

static XP_STATUS xpsEgressUdbArpKeySet(xpsAclkeyFieldList_t *field,
                                       xpsPclRuleFormatEgrUdbOnly_t *value, xpsPclRuleFormatEgrUdbOnly_t *mask)
{
    uint32_t index = 0, i =0;

    for (i = 8; i < 10; i++)
    {
        value->udb[i] = 0; /* Reserved */
        mask->udb[i] = 0;
    }

    for (i = 4; i < 6; i++)
    {
        value->udb[i] =
            0; /* Masking XP_PCL_SRC_PORT as it is not applicable for EACL */
        mask->udb[i] = 0;
    }

    for (index=0; index < (field->numFlds); index++)
    {
        switch (field->fldList[index].keyFlds)
        {
            case XP_PCL_PORT_BIT_MAP:  /* UDB[0-3] */
                memcpy((uint8_t*)&value->udb[0], field->fldList[index].value,
                       (sizeof(uint32_t)));
                memcpy((uint8_t*)&mask->udb[0], field->fldList[index].mask, (sizeof(uint32_t)));
                break;

#if 0
            case XP_PCL_SRC_PORT:
                memcpy((uint8_t*)&value->replacedFld.srcPort, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->replacedFld.srcPort, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;
#endif

            case XP_PCL_VID: /* UDB[6-7] */
            case XPS_PCL_VID1: /* UDB[6-7] */
                memcpy((uint8_t*)&value->udb[6], field->fldList[index].value, sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb[6], field->fldList[index].mask, sizeof(uint16_t));
                break;

            case XPS_PCL_IS_ARP_REQUEST:
                value->udb[10] = XPS_PCL_EGRESS_PKT_TYPE_UDE1 /* ARP */;
                mask->udb[10] = XPS_PCL_EGRESS_PKT_TYPE_MASK;
                value->udb[11] = XPS_PCL_ARP_REQUEST_OP_CODE;
                mask->udb[11] = XPS_PCL_ARP_REQUEST_OP_CODE;
                break;

            case XPS_PCL_IS_ARP_REPLY:
                value->udb[10] = XPS_PCL_EGRESS_PKT_TYPE_UDE1 /* ARP */;
                mask->udb[10] = XPS_PCL_EGRESS_PKT_TYPE_MASK;
                value->udb[11] = XPS_PCL_ARP_REPLY_OP_CODE;
                mask->udb[11] = XPS_PCL_ARP_REPLY_OP_CODE;
                break;

            case XPS_PCL_IS_NON_IPV6:
            case XPS_PCL_IS_NON_IPV4:
            case XPS_PCL_IS_ARP:
                *(field->fldList[index].value) = ((XPS_PCL_EGRESS_PKT_TYPE_UDE1 << 1) & 0x1F);
                *(field->fldList[index].mask) = 0x1F;
                memcpy((uint8_t*)&value->udb[10], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[10], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_ARP_OPCODE: /* UDB[11-12] */
                memcpy((uint8_t*)&value->udb[11], field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->udb[11], field->fldList[index].mask, sizeof(uint16_t));
                break;

            case XPS_PCL_MAC_SA: /* UDB[13-18]] */
                memcpy((uint8_t*)&value->udb[13], field->fldList[index].value,
                       (sizeof(uint8_t)*6));
                memcpy((uint8_t*)&mask->udb[13], field->fldList[index].mask,
                       (sizeof(uint8_t)*6));
                break;

            case XPS_PCL_MAC_DA: /* UDB[19-24] */
                memcpy((uint8_t*)&value->udb[19], field->fldList[index].value,
                       (sizeof(uint8_t)*6));
                memcpy((uint8_t*)&mask->udb[19], field->fldList[index].mask,
                       (sizeof(uint8_t)*6));
                break;

            case XPS_PCL_CPU_CODE: /* UDB[8] */
                memcpy((uint8_t*)&value->udb[8], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[8], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_EPORT: /* UDB[25] */
                memcpy((uint8_t*)&value->udb[25], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[25], field->fldList[index].mask, sizeof(uint8_t));
                break;

            case XPS_PCL_DSA_CMD:
                memcpy((uint8_t*)&value->udb[26], field->fldList[index].value, sizeof(uint8_t));
                memcpy((uint8_t*)&mask->udb[26], field->fldList[index].mask, sizeof(uint8_t));
                break;

            default :
                break;
        }
    }

    return XP_NO_ERR;

}

static XP_STATUS xpsIngressUdbVxlanIpv6KeySet(xpsAclkeyFieldList_t *field,
                                              xpsPclRuleFormatIngrUdbOnly_t *value, xpsPclRuleFormatIngrUdbOnly_t *mask)
{
    uint32_t index = 0;

    for (index=0; index < (field->numFlds); index++)
    {
        switch (field->fldList[index].keyFlds)
        {
            case XPS_PCL_PCLID:  /* UDB[0-1] */
                memcpy((uint8_t*)&value->replacedFld.pclId, field->fldList[index].value,
                       sizeof(uint16_t));
                memcpy((uint8_t*)&mask->replacedFld.pclId, field->fldList[index].mask,
                       sizeof(uint16_t));
                break;

            case XPS_PCL_VNI: /* UDB[2-4] */
                memcpy((uint8_t*)&value->udb[2], field->fldList[index].value,
                       (sizeof(uint8_t)*3));
                memcpy((uint8_t*)&mask->udb[2], field->fldList[index].mask,
                       (sizeof(uint8_t)*3));
                break;

            case XPS_PCL_IPV6_SIP: /* UDB[5-20] */
                memcpy((uint8_t*)&value->udb[5], field->fldList[index].value,
                       (sizeof(uint8_t)*16));
                memcpy((uint8_t*)&mask->udb[5], field->fldList[index].mask,
                       (sizeof(uint8_t)*16));
                break;

            default :
                break;
        }
    }

    return XP_NO_ERR;

}

XP_STATUS xpsUdbKeySet(xpsAclkeyFieldList_t *fieldList,
                       xpsPclRuleFormat_t *pattern, xpsPclRuleFormat_t *mask,
                       xpsAclKeyFormat keyFormat, xpsAclStage_e stage)
{
    XP_STATUS                           xpsRetVal = XP_NO_ERR;

    if (keyFormat == XPS_PCL_NON_IP_KEY)
    {
        if (stage == XPS_PCL_STAGE_INGRESS)
        {
            if ((xpsRetVal = xpsUdbNonIpKeySetIngress(fieldList,
                                                      &(pattern->ruleIngrUdbOnly), &(mask->ruleIngrUdbOnly))) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " xpsUdbNonIpKeySetIngress failed with err : %d ", xpsRetVal);
                return xpsRetVal;

            }
        }
        else
        {
            if ((xpsRetVal = xpsUdbNonIpKeySetEgress(fieldList, &(pattern->ruleEgrUdbOnly),
                                                     &(mask->ruleEgrUdbOnly))) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " xpsUdbNonIpKeySetEgress failed with err : %d ", xpsRetVal);
                return xpsRetVal;

            }
        }
    }
    else if (keyFormat == XPS_PCL_IPV4_L4_KEY)
    {
        if (stage == XPS_PCL_STAGE_INGRESS)
        {
            if ((xpsRetVal = xpsIngressUdbIpv4KeySet(fieldList, &(pattern->ruleIngrUdbOnly),
                                                     &(mask->ruleIngrUdbOnly))) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " xpsUdbIngressIpv4TcpKeySet failed with err : %d ", xpsRetVal);
                return xpsRetVal;

            }
        }
        else
        {
            if ((xpsRetVal = xpsEgressUdbIpv4KeySet(fieldList, &(pattern->ruleEgrUdbOnly),
                                                    &(mask->ruleEgrUdbOnly))) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " xpsUdbEgressIpv4TcpKeySet failed with err : %d ", xpsRetVal);
                return xpsRetVal;

            }
        }

    }
    else if (keyFormat == XPS_PCL_IPV6_L4_KEY)
    {
        if (stage == XPS_PCL_STAGE_INGRESS)
        {
            if ((xpsRetVal = xpsIngressUdbIpv6KeySet(fieldList, &(pattern->ruleIngrUdbOnly),
                                                     &(mask->ruleIngrUdbOnly))) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " xpsIngressUdbIpv4TcpKeySet failed with err : %d ", xpsRetVal);
                return xpsRetVal;

            }
        }
        else
        {
            if ((xpsRetVal = xpsEgressUdbIpv6KeySet(fieldList, &(pattern->ruleEgrUdbOnly),
                                                    &(mask->ruleEgrUdbOnly))) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " xpsEgressUdbIpv4TcpKeySet failed with err : %d ", xpsRetVal);
                return xpsRetVal;

            }
        }
    }
    else if (keyFormat == XPS_PCL_ARP_KEY)
    {
        if (stage == XPS_PCL_STAGE_INGRESS)
        {
            if ((xpsRetVal = xpsIngressUdbArpKeySet(fieldList, &(pattern->ruleIngrUdbOnly),
                                                    &(mask->ruleIngrUdbOnly))) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " xpsIngressUdpArpKeySet failed with err : %d ", xpsRetVal);
                return xpsRetVal;

            }
        }
        else
        {
            if ((xpsRetVal = xpsEgressUdbArpKeySet(fieldList, &(pattern->ruleEgrUdbOnly),
                                                   &(mask->ruleEgrUdbOnly))) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " xpsEgressUdpArpKeySet failed with err : %d ", xpsRetVal);
                return xpsRetVal;

            }
        }
    }
    else if (keyFormat == XPS_PCL_VXLAN_IPV6_KEY)
    {
        if (stage == XPS_PCL_STAGE_INGRESS)
        {
            if ((xpsRetVal = xpsIngressUdbVxlanIpv6KeySet(fieldList,
                                                          &(pattern->ruleIngrUdbOnly),
                                                          &(mask->ruleIngrUdbOnly))) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " xpsIngressUdpArpKeySet failed with err : %d ", xpsRetVal);
                return xpsRetVal;

            }
        }
    }
    else
    {
        return XP_ERR_INVALID_VALUE;
    }

    return XP_NO_ERR;

}

XP_STATUS xpsGetAclTableType(uint32_t tableIdNum,
                             xpsAclTableType_t *tableType)
{
    xpsAclTableContextDbEntry_t         *tableIdDbEntry = NULL;
    xpsAclTableContextDbEntry_t         tableIdDbKey;
    XP_STATUS                           xpsRetVal = XP_NO_ERR;
    xpsScope_t                          scopeId = 0;


    tableIdDbKey.tableId = tableIdNum;

    if ((xpsRetVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                        (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    if (tableIdDbEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    *tableType = tableIdDbEntry->tableType;

    return XP_NO_ERR;
}

XP_STATUS xpsGetAclKeyFormat(xpsAclKeyFlds_t entryType,
                             xpsAclTableType_t tableType, xpsAclKeyFormat *keyFormat)
{
    switch (entryType)
    {
        case XPS_PCL_IS_NON_IP:
            if (tableType == XPS_ACL_TABLE_TYPE_IPV6)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " skip] Unsupported attribute SAI_ACL_IP_TYPE_NON_IP in L3v6 Table \n ");
                return XP_NO_ERR;
            }
            *keyFormat = XPS_PCL_NON_IP_KEY;
            break;
        case XPS_PCL_IS_ARP:
        case XPS_PCL_IS_ARP_REQUEST:
        case XPS_PCL_IS_ARP_REPLY:
        case XPS_PCL_IS_NON_IPV6:
        case XPS_PCL_IS_IPV4:
            if (tableType == XPS_ACL_TABLE_TYPE_IPV6)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " [skip] Unsupported attribute in L3v6 Table \n \n ");
                return XP_NO_ERR;
            }
            *keyFormat = XPS_PCL_IPV4_L4_KEY;
            break;
        case XPS_PCL_IS_NON_IPV4:
        case XPS_PCL_IS_IPV6:
            if ((tableType == XPS_ACL_TABLE_TYPE_IPV4) ||
                (tableType == XPS_ACL_TABLE_TYPE_NON_IP)||
                (tableType == XPS_ACL_TABLE_TYPE_MIRROR))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " [skip] Unsupported attribute in L3v6 Table \n ");
                return XP_NO_ERR;
            }
            *keyFormat = XPS_PCL_IPV6_L4_KEY;
            break;

        case XPS_PCL_IS_IP:
            if (tableType == XPS_ACL_TABLE_TYPE_NON_IP)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " [skip] Unsupported attribute SAI_ACL_IP_TYPE_IP in Non-Ip Table \n ");
                return XP_NO_ERR;
            }
        case XPS_PCL_IS_ANY:
            if (tableType == XPS_ACL_TABLE_TYPE_IPV4_IPV6)
            {
                *keyFormat = XPS_PCL_NON_IP_KEY;
            }
            else
            {
                *keyFormat = (tableType == XPS_ACL_TABLE_TYPE_IPV4) ? XPS_PCL_IPV4_L4_KEY :
                             XPS_PCL_IPV6_L4_KEY;
            }
            break;
        default:
            return XP_ERR_INVALID_ARG;
    }
    return XP_NO_ERR;
}

/* Allocated new counterId for AclEntry
*/
XP_STATUS xpsAclCreateNewCounter(xpsDevice_t devId, xpsScope_t scopeId,
                                 CPSS_PP_FAMILY_TYPE_ENT devType, xpsAclTableContextDbEntry_t *tableIdDbEntry,
                                 uint32_t parallelLookupNum, uint32_t *localCounterId)
{
    XP_STATUS      xpsRetVal = XP_NO_ERR;
    GT_STATUS      rc;
    bool           newBlockBind = false;
    uint32_t       count = 0;
    uint32_t       blockId = 0;
    uint16_t       allocatorId = 0;
    uint32_t       packetCntrBlockNum = 0,
                   byteCntrBlockNum;

    if (devType == CPSS_PP_FAMILY_DXCH_AC3X_E ||
        devType == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
    {
        blockId = XPS_ACL_CNC_BLK_NUM_AC3X;
        newBlockBind = false;
    }
    else if (devType == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        blockId = XPS_ACL_CNC_BLK_NUM_ALDRIN2;
        newBlockBind = false;
    }
    else
    {
        for (count =0; count< XPS_ACL_CNC_BLOCK_COUNT; count++)
        {
            if (cncBlockListBound[count] == parallelLookupNum)
            {
                if (cncPerBlockCount[count] < XPS_CPSS_CNC_COUNTER_PER_BLK)
                {
                    blockId = count + XPS_ACL_CNC_PKT_COUNTER_BLK_START;
                    newBlockBind = false;
                    break;
                }
            }
            if (!newBlockBind)
            {
                if (cncBlockListBound[count] == XPS_ACL_PARALLEL_LOOKUP_INVALID)
                {

                    blockId = count + XPS_ACL_CNC_PKT_COUNTER_BLK_START;
                    newBlockBind = true;

                }
            }

        }
    }

    switch (blockId)
    {
        case XPS_ACL_CNC_PKT_COUNTER_BLK_START:
            allocatorId = XPS_ALLOCATOR_ACL_COUNTER_BLOCK_1;
            break;
        case XPS_ACL_CNC_PKT_COUNTER_BLK_START+1:
            allocatorId = XPS_ALLOCATOR_ACL_COUNTER_BLOCK_2;
            break;
        case XPS_ACL_CNC_PKT_COUNTER_BLK_START+2:
            allocatorId = XPS_ALLOCATOR_ACL_COUNTER_BLOCK_3;
            break;
        case XPS_ACL_CNC_PKT_COUNTER_BLK_START+3:
            allocatorId = XPS_ALLOCATOR_ACL_COUNTER_BLOCK_4;
            break;
        case XPS_ACL_CNC_PKT_COUNTER_BLK_START+4:
            allocatorId = XPS_ALLOCATOR_ACL_COUNTER_BLOCK_5;
            break;
        case XPS_ACL_CNC_PKT_COUNTER_BLK_START+5:
            allocatorId = XPS_ALLOCATOR_ACL_COUNTER_BLOCK_6;
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " Invalid blockId");

    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "OFFSET %d blockId : %d blockBound :%d %d %d %d %d \n",
          XPS_ACL_CNC_PKT_COUNTER_BLK_START,
          blockId, cncBlockListBound[0], cncBlockListBound[1], cncBlockListBound[2],
          cncBlockListBound[3], cncBlockListBound[4]);

    if ((xpsRetVal = xpsAllocatorAllocateId(scopeId,
                                            allocatorId, localCounterId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsAllocatorAllocateId failed with err : %d\n ", xpsRetVal);
        return xpsRetVal;
    }
    if (newBlockBind)
    {
        packetCntrBlockNum = blockId;
        byteCntrBlockNum = blockId + XPS_ACL_CNC_BLOCK_COUNT;

        if ((tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)&&
            (tableIdDbEntry->isMirror))
        {
            if ((rc = cpssHalPclCounterBlockConfigure(scopeId,
                                                      packetCntrBlockNum, byteCntrBlockNum,
                                                      CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E,
                                                      (GT_BOOL)true)) != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " xpsAllocatorAllocateId failed with err : %d\n ", rc);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }
        else
        {
            if ((xpsRetVal = xpsAclCounterBlockClientSet(devId, parallelLookupNum,
                                                         packetCntrBlockNum, byteCntrBlockNum, true)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Unbinding clients from blocks failed, devId(%d)", devId);
                return xpsRetVal;
            }
        }
        cncBlockListBound[blockId - XPS_ACL_CNC_PKT_COUNTER_BLK_START] =
            parallelLookupNum;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          " creating  counter EXISTING BLOCK: %d counterIndex : %d\n ",
          packetCntrBlockNum, *localCounterId);
    cncPerBlockCount[blockId - XPS_ACL_CNC_PKT_COUNTER_BLK_START] += 1;

    return xpsRetVal;
}

/* Release counterId for AclEntry
*/
XP_STATUS xpsAclRemoveCounter(xpsDevice_t devId, xpsScope_t scopeId,
                              CPSS_PP_FAMILY_TYPE_ENT devType, xpsAclTableContextDbEntry_t *tableIdDbEntry,
                              uint32_t localCounterId)
{

    XP_STATUS      xpsRetVal = XP_NO_ERR;
    GT_STATUS      rc;
    uint32_t       blockId = 0;
    uint16_t       allocatorId = 0;
    uint32_t       packetCntrBlockNum = 0,
                   byteCntrBlockNum = 0;
    uint32_t       parallelLookupNum = 0;

    blockId = (localCounterId/XPS_CPSS_CNC_COUNTER_PER_BLK) +
              XPS_ACL_CNC_PKT_COUNTER_BLK_START;

    if (devType == CPSS_PP_FAMILY_DXCH_AC3X_E ||
        devType == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
    {
        blockId = XPS_ACL_CNC_BLK_NUM_AC3X;
    }
    else if (devType == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        blockId = XPS_ACL_CNC_BLK_NUM_ALDRIN2;
    }

    switch (blockId)
    {
        case XPS_ACL_CNC_PKT_COUNTER_BLK_START:
            allocatorId = XPS_ALLOCATOR_ACL_COUNTER_BLOCK_1;
            break;
        case XPS_ACL_CNC_PKT_COUNTER_BLK_START+1:
            allocatorId = XPS_ALLOCATOR_ACL_COUNTER_BLOCK_2;
            break;
        case XPS_ACL_CNC_PKT_COUNTER_BLK_START+2:
            allocatorId = XPS_ALLOCATOR_ACL_COUNTER_BLOCK_3;
            break;
        case XPS_ACL_CNC_PKT_COUNTER_BLK_START+3:
            allocatorId = XPS_ALLOCATOR_ACL_COUNTER_BLOCK_4;
            break;
        case XPS_ACL_CNC_PKT_COUNTER_BLK_START+4:
            allocatorId = XPS_ALLOCATOR_ACL_COUNTER_BLOCK_5;
            break;
        case XPS_ACL_CNC_PKT_COUNTER_BLK_START+5:
            allocatorId = XPS_ALLOCATOR_ACL_COUNTER_BLOCK_6;
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " Invalid blockId");

    }
    if ((xpsRetVal = xpsAllocatorReleaseId(scopeId, allocatorId,
                                           localCounterId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsAllocatorReleaseId failed with err : %d\n ", xpsRetVal);
        return xpsRetVal;
    }
    cncPerBlockCount[ blockId - XPS_ACL_CNC_PKT_COUNTER_BLK_START] --;
    if ((cncPerBlockCount[ blockId - XPS_ACL_CNC_PKT_COUNTER_BLK_START] == 0) &&
        ((devType == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
         (devType == CPSS_PP_FAMILY_DXCH_AC5X_E)))
    {
        parallelLookupNum = cncBlockListBound[ blockId -
                                                       XPS_ACL_CNC_PKT_COUNTER_BLK_START];
        packetCntrBlockNum = blockId;
        byteCntrBlockNum = blockId + XPS_ACL_CNC_BLOCK_COUNT;

        if ((tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)&&
            (tableIdDbEntry->isMirror))
        {
            if ((rc = cpssHalPclCounterBlockConfigure(scopeId,
                                                      packetCntrBlockNum, byteCntrBlockNum,
                                                      CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E,
                                                      (GT_BOOL)false)) != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " xpsAllocatorAllocateId failed with err : %d\n ", rc);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }
        else
        {
            if ((xpsRetVal = xpsAclCounterBlockClientSet(devId, parallelLookupNum,
                                                         packetCntrBlockNum, byteCntrBlockNum, false)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Unbinding clients from blocks failed, devId(%d)", devId);
                return xpsRetVal;
            }
        }
        cncBlockListBound[ blockId - XPS_ACL_CNC_PKT_COUNTER_BLK_START] =
            XPS_ACL_PARALLEL_LOOKUP_INVALID;
    }
    return xpsRetVal;
}

XP_STATUS xpsPortsAclEntryKeySet(xpsDevice_t devId, uint32_t tableId,
                                 uint32_t ruleId,
                                 uint32_t *ports, uint32_t ports_count)
{

    XP_STATUS                                   xpsRetVal = XP_NO_ERR;
    xpsAclEntryContextDbEntry_t                 tableEntryDbKey;
    xpsAclEntryContextDbEntry_t                 *tableEntryDbEntry = NULL;
    uint32_t                                    portGroupId = 0;
    uint32_t                                    portNum = 0;
    xpsScope_t                              scopeId = 0;


    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        return xpsRetVal;
    }

    if (!ports && ports_count)
    {
        return XP_ERR_INVALID_DATA;
    }
    /* Retrieving key fields and action info */
    tableEntryDbKey.key = XPS_PCL_TABLE_ID_KEY(tableId, ruleId);

    if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableEntryDbHndl,
                                         (xpsDbKey_t)&tableEntryDbKey, (void**)&tableEntryDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }
    memset(tableEntryDbEntry->portGroupBmp, 0,
           sizeof(tableEntryDbEntry->portGroupBmp));
    memset(tableEntryDbEntry->portListBmp, 0,
           sizeof(tableEntryDbEntry->portListBmp));
    for (uint16_t i = 0; i < ports_count; i++)
    {
        portNum = ports[i];
        portGroupId = portNum/24;
        if (portGroupId < XPS_PORT_LIST_PORT_GROUP_MAX)
        {
            tableEntryDbEntry->portGroupBmp[portGroupId]++;
            tableEntryDbEntry->portListBmp[portGroupId] |= (1 << (portNum % 24) |
                                                            portGroupId << 24);
        }
    }

    return xpsRetVal;
}

/* get corresponding vtcamId by keyFormat, tableType and stage*/
static XP_STATUS xpsAclGetVtcamIdByKeyFormat(xpsAclTableContextDbEntry_t
                                             *tableIdDbEntry,
                                             xpsAclKeyFormat keyFormat,
                                             uint32_t *vtcamId)
{
    XP_STATUS  xpsRetVal = XP_NO_ERR;

    if (!tableIdDbEntry || !vtcamId)
    {
        return XP_ERR_INVALID_DATA;
    }

    if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV4_IPV6)
    {
        if ((tableIdDbEntry->isMirror) &&
            (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS))
        {
            if (keyFormat == XPS_PCL_IPV6_L4_KEY)
            {
                *vtcamId = XPS_L3V6_MIRROR_ING_STG_TABLE_ID_0;
            }
            else
            {
                *vtcamId = XPS_L3_MIRROR_ING_STG_TABLE_ID_0;
            }
        }
        else if (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)
        {
            if (keyFormat == XPS_PCL_IPV6_L4_KEY)
            {
                *vtcamId = XPS_ING_STG_V6_USER_ACL_TABLE_START;
            }
            else
            {
                *vtcamId = XPS_ING_STG_V4_USER_ACL_TABLE_START;
            }
        }
        else
        {
            if (keyFormat == XPS_PCL_IPV6_L4_KEY)
            {
                *vtcamId = XPS_EGR_STG_V6_USER_ACL_TABLE_START;
            }
            else
            {
                *vtcamId = XPS_EGR_STG_V4_USER_ACL_TABLE_START;
            }
        }
    }
    else if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
    {
        if ((tableIdDbEntry->isMirror) &&
            (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS))
        {
            *vtcamId = XPS_L3V6_MIRROR_ING_STG_TABLE_ID_0;
        }
        else if (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)
        {
            *vtcamId = XPS_ING_STG_V6_USER_ACL_TABLE_START;
        }
        else
        {
            *vtcamId = XPS_EGR_STG_V6_USER_ACL_TABLE_START;
        }
    }
    else
    {
        if ((tableIdDbEntry->isMirror) &&
            (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS))
        {
            *vtcamId = XPS_L3_MIRROR_ING_STG_TABLE_ID_0;
        }
        else if (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)
        {
            *vtcamId = XPS_ING_STG_V4_USER_ACL_TABLE_START;
        }
        else
        {
            *vtcamId = XPS_EGR_STG_V4_USER_ACL_TABLE_START;
        }
    }
    return xpsRetVal;
}

static XP_STATUS xpsAclWriteEntryKeySet(xpsDevice_t devId, uint32_t tableId,
                                        uint32_t priority, uint32_t ruleId, xpsAclkeyFieldList_t *fieldList,
                                        xpsPclAction_t actionPtr, bool is_update, xpsAclKeyFormat keyFormat,
                                        bool is_control_acl)
{

    XPS_LOCK(xpsAclWriteEntryKeySet);

    XP_STATUS                                   xpsRetVal = XP_NO_ERR;
    xpsAclTableContextDbEntry_t                 *tableIdDbEntry = NULL;
    xpsAclTableContextDbEntry_t                 tableIdDbKey;
    xpsAclEntryContextDbEntry_t                 tableEntryDbKey;
    xpsAclEntryContextDbEntry_t                 *tableEntryDbEntry = NULL;
    xpsAclCounterIdMappingContextDbEntry_t      counterIdMapDbKey;
    xpsAclCounterIdMappingContextDbEntry_t      *counterIdMapDbEntry = NULL;
    uint32_t                                    parallelLookupNum;
    uint32_t                                    localCounterId=0;
    bool                                        newLocalCounterId = false;
    uint32_t                                    vtcamId;
    xpsScope_t                                  scopeId = 0;
    xpsVirtualTcamRuleType_t                    tcamRuleType;
    xpsVirtualTcamRuleData_t                    ruleData;
    xpsPclRuleFormat_t                          pattern, mask;
    uint32_t                                    portNum;
    uint32_t                                    cpssPortNum;
    uint32_t                                    cpssDevNum;
    GT_STATUS                                   rc;
    bool                                        priorityUpdate = false;
    xpsPclAction_t                              dummyActionPtr;
    uint32_t                                    currentPriority;
    uint32_t                                    portGroupIndex = 0;
    uint32_t                                    keyFldsIndex = 0;
    uint32_t                                    portGrpkeyFldsIndex = 0;
    uint32_t                                    vlanKeyFldsIndex = 0;
    uint32_t                                    newRuleId = 0, newPriority=0;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  ruleAttributes;
    CPSS_PP_FAMILY_TYPE_ENT                     devType = cpssHalDevPPFamilyGet(
                                                              devId);
    bool                                        hasInPortsFld = false;
    /* when in_ports/out_ports/in_port/out_port/outer_vlan_id in acl entry, should always deploy to hw*/
    bool                                        needDeploy = false;
    bool                                        keyFormatHasChange = false;
    xpsAclKeyFormat                             oldkeyFormat = XPS_PCL_NON_IP_KEY;
    uint32_t origNumFlds = fieldList->numFlds;

    memset(&pattern, 0, sizeof(xpsPclRuleFormat_t));
    memset(&mask, 0, sizeof(xpsPclRuleFormat_t));

    tableIdDbKey.tableId = tableId;

    if ((xpsRetVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                        (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }
    if (tableIdDbEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    tableEntryDbKey.key = XPS_PCL_TABLE_ID_KEY(tableId, ruleId);

    if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableEntryDbHndl,
                                         (xpsDbKey_t)&tableEntryDbKey, (void**)&tableEntryDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    if (tableEntryDbEntry == NULL)
    {
        if ((xpsRetVal = xpsStateHeapMalloc(sizeof(xpsAclEntryContextDbEntry_t),
                                            (void**)&tableEntryDbEntry)) == XP_NO_ERR)
        {
            memset(tableEntryDbEntry, 0, sizeof(xpsAclEntryContextDbEntry_t));
            tableEntryDbEntry -> key = XPS_PCL_TABLE_ID_KEY(tableId, ruleId);


            if ((xpsRetVal = xpsStateInsertData(scopeId, aclTableEntryDbHndl,
                                                (void*)tableEntryDbEntry)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Insert ACL table entry failed, devId(%d)", devId);
                /* Free Allocated memory */
                xpsStateHeapFree((void*)tableEntryDbEntry);
                return xpsRetVal;
            }
            tableIdDbEntry ->ruleId[tableIdDbEntry ->numEntries]=ruleId;
            tableIdDbEntry ->numEntries++;
            tableEntryDbEntry ->isDirty = true;
            for (uint16_t i=0; i<fieldList->numFlds; i++)
            {
                /* Update portListBmp when create acl entry*/
                if (fieldList->fldList[i].keyFlds == XP_PCL_PORT_BIT_MAP)
                {
                    memset(tableEntryDbEntry->portGroupBmp, 0,
                           sizeof(tableEntryDbEntry->portGroupBmp));
                    memset(tableEntryDbEntry->portListBmp, 0,
                           sizeof(tableEntryDbEntry->portListBmp));
                    /* Each array member represents 8 port
                       e.g value[0]=00000011 represents port[0-1] */
                    for (uint8_t index = 0; index < (SYSTEM_MAX_PORT/8); index++)
                    {
                        if (fieldList->fldList[i].value[index] > 0)
                        {
                            /* every portGroup has 24 ports */
                            tableEntryDbEntry->portGroupBmp[index/3]++;
                            /* portListBmp |= portBitOffset | portGroupId */
                            tableEntryDbEntry->portListBmp[index/3] |= (fieldList->fldList[i].value[index]
                                                                        << (index%3)*8 | index/3 << 24);
                            hasInPortsFld = true;
                        }
                    }
                }
            }
        }
    }

    if (tableEntryDbEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    tableEntryDbEntry -> action = actionPtr;
    tableEntryDbEntry -> priority = priority;

    oldkeyFormat = tableEntryDbEntry->keyFormat;
    for (uint8_t i=0; i<fieldList->numFlds; i++)
    {
        if (fieldList->fldList[i].keyFlds == XPS_PCL_IS_ARP ||
            fieldList->fldList[i].keyFlds == XPS_PCL_IS_ARP_REQUEST ||
            fieldList->fldList[i].keyFlds == XPS_PCL_IS_ARP_REPLY)
        {
            keyFormat = XPS_PCL_ARP_KEY;
        }
        if ((fieldList->fldList[i].keyFlds == XPS_PCL_VID1 ||
             fieldList->fldList[i].keyFlds == XP_PCL_PORT_BIT_MAP))
        {
            needDeploy = true;
        }
        if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV4_IPV6)
        {
            if (fieldList->fldList[i].keyFlds == XPS_PCL_IS_IPV4)
            {
                keyFormat = XPS_PCL_IPV4_L4_KEY;
            }
            if (fieldList->fldList[i].keyFlds == XPS_PCL_IS_IPV6)
            {
                keyFormat = XPS_PCL_IPV6_L4_KEY;
            }
            if (fieldList->fldList[i].keyFlds == XPS_PCL_ETHER_TYPE)
            {
                keyFormat = XPS_PCL_NON_IP_KEY;
            }
        }

    }
    tableEntryDbEntry->keyFormat = keyFormat;
    for (portGroupIndex = 0; portGroupIndex < XPS_PORT_LIST_PORT_GROUP_MAX;
         portGroupIndex++)
    {
        if (tableEntryDbEntry->portGroupBmp[portGroupIndex])
        {
            hasInPortsFld = true;
            break;
        }
    }

    tcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
    switch (tableIdDbEntry->stage)
    {
        case XPS_PCL_STAGE_INGRESS:

            switch (keyFormat)
            {
                case XPS_PCL_NON_IP_KEY:
                case XPS_PCL_IPV4_L4_KEY:
                case XPS_PCL_ARP_KEY:
                case XPS_PCL_VXLAN_IPV6_KEY:
                    tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
                    break;
                case XPS_PCL_IPV6_L4_KEY:
                    tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
                    break;
                default:
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "xpsAclWriteEntryKey:rule format is not supported\n");
                    return XP_ERR_INVALID_VALUE ;
            }
            break;
        case XPS_PCL_STAGE_EGRESS:
            switch (keyFormat)
            {
                case XPS_PCL_NON_IP_KEY:
                case XPS_PCL_IPV4_L4_KEY:
                case XPS_PCL_ARP_KEY:
                    tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_EGRESS_UDB_30_E;
                    break;
                case XPS_PCL_IPV6_L4_KEY:
                    tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_EGRESS_UDB_60_E;
                    break;
                default:
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "xpsAclWriteEntryKey:rule format is not supported\n");
                    return XP_ERR_INVALID_VALUE ;
            }

            actionPtr.egressPolicy = GT_TRUE;

            //EPCL supports only FORWARD or HARDDROP
            switch (actionPtr.pktCmd)
            {
                case CPSS_PACKET_CMD_DROP_SOFT_E:
                    actionPtr.mirror.cpuCode = XPS_HARD_DROP_REASON_CODE;
                    actionPtr.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
                case CPSS_PACKET_CMD_FORWARD_E:
                case CPSS_PACKET_CMD_DROP_HARD_E:
                    break;
                case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
                    if (devType == CPSS_PP_FAMILY_DXCH_AC3X_E ||
                        devType == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "xpsAclWriteEntryKey:Cmd TRAP not supported in Egress \n");
                    }
                    else
                    {
                        break;
                    }
                default:
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "xpsAclWriteEntryKey:packet command is not supported %d \n", actionPtr.pktCmd);
                    return XP_NO_ERR;
            }

            break;

        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsAclWriteEntryKey: Invalid table stage\n");
            return XP_ERR_INVALID_VALUE ;
    }
    if (actionPtr.redirect.redirectCmd ==
        CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
    {
        portNum = actionPtr.redirect.data.outIf.outInterface.devPort.portNum;
        cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
        actionPtr.redirect.data.outIf.outInterface.devPort.hwDevNum = cpssDevNum;
        actionPtr.redirect.data.outIf.outInterface.devPort.portNum  = cpssPortNum;
        actionPtr.redirect.data.outIf.outInterface.type = CPSS_INTERFACE_PORT_E;
        actionPtr.bypassBridge = GT_TRUE;
        actionPtr.bypassIngressPipe = GT_TRUE;
        if ((rc = cpssDxChBrgEgrFltVlanPortFilteringEnableSet(cpssDevNum, cpssPortNum,
                                                              GT_FALSE) != GT_OK))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChBrgEgrFltVlanPortFilteringEnableSet  failed with err : %d n ", rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }
    else if (actionPtr.redirect.redirectCmd ==
             CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E)
    {
        actionPtr.bypassBridge = GT_TRUE;
    }

    if ((actionPtr.mirror.mirrorToRxAnalyzerPort) &&
        (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS))
    {
        if (!tableIdDbEntry->isMirror)
        {
            tableIdDbEntry->isMirror = true;
            if ((xpsRetVal =  xpsAclUpdateMirrorTableVtcamMap(devId, tableId)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "  xpsAclUpdateMirrorTableVtcamMap failed with err : %d ", xpsRetVal);
                return xpsRetVal;
            }
        }

    }
    else if ((actionPtr.mirror.mirrorToTxAnalyzerPortEn) &&
             (tableIdDbEntry->stage == XPS_PCL_STAGE_EGRESS))
    {
        tableIdDbEntry->isMirror = true;
    }
    else
    {
        tableIdDbEntry->isMirror = false;
    }

    /* Create new counterIdmapEntry for acl_entry*/
    if ((is_control_acl || tableIdDbEntry->bindCount || needDeploy) &&
        actionPtr.matchCounter.enableMatchCount)
    {
        tableEntryDbEntry->counterEnable = true;
        tableEntryDbEntry->counterId =  actionPtr.matchCounter.matchCounterIndex;
        counterIdMapDbKey.globalCounterId = actionPtr.matchCounter.matchCounterIndex;

        if (((xpsRetVal = xpsStateSearchData(scopeId, aclCounterIdMapDbHndl,
                                             (xpsDbKey_t)&counterIdMapDbKey, (void**)&counterIdMapDbEntry)) != XP_NO_ERR))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
            return xpsRetVal;
        }

        if (counterIdMapDbEntry == NULL)
        {
            if ((xpsRetVal = xpsStateHeapMalloc(sizeof(
                                                    xpsAclCounterIdMappingContextDbEntry_t),
                                                (void**)&counterIdMapDbEntry)) == XP_NO_ERR)
            {
                memset(counterIdMapDbEntry, 0, sizeof(xpsAclCounterIdMappingContextDbEntry_t));
                counterIdMapDbEntry -> globalCounterId =
                    actionPtr.matchCounter.matchCounterIndex;
                counterIdMapDbEntry -> numOfEntries +=1;

                if ((xpsRetVal = xpsStateInsertData(scopeId, aclCounterIdMapDbHndl,
                                                    (void*)counterIdMapDbEntry)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Insert ACL table entry failed, devId(%d)", devId);
                    /* Free Allocated memory */
                    xpsStateHeapFree((void*)counterIdMapDbEntry);
                    return xpsRetVal;
                }
                newLocalCounterId = true;
            }
            else
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "xpsStateHeapMalloc failed, retVal(%d)", xpsRetVal);
                return xpsRetVal;
            }

        }
        else
        {
            if ((tableEntryDbEntry -> isDirty) && (!is_update))
            {
                counterIdMapDbEntry -> numOfEntries +=1;
                newLocalCounterId = false;
            }
        }
    }

    /* Control ACL */
    if (is_control_acl)
    {
        /* Duplication of rules for all the ports of different port groups sharing the same rule */
        for (portGroupIndex = 0; portGroupIndex < XPS_PORT_LIST_PORT_GROUP_MAX;
             portGroupIndex++)
        {
            uint32_t portListMask = 0;
            bool rule_valid = true;
            GT_BOOL valid = GT_FALSE;

            if (hasInPortsFld)
            {
                if (tableEntryDbEntry->portGroupBmp[portGroupIndex])
                {
                    portListMask = ~(tableEntryDbEntry->portListBmp[portGroupIndex]);
                }
                else
                {
                    /* Set acl entry invalid, if no port in this group*/
                    portListMask = 0;
                    rule_valid = false;
                }
                portListMask&=0x00FFFFFF;
                /* no inversion should be done on the masking of the port group id (bits 24-27)*/
                portListMask|=0x0F000000;
                /* NOTE: Offset to Key fields should not be changed
                   and should be same for all port groups. */
                if (keyFldsIndex == 0)
                {
                    portGrpkeyFldsIndex = fieldList->numFlds;
                    fieldList->numFlds++;
                }
                if ((fieldList->fldList[portGrpkeyFldsIndex].value == NULL) ||
                    (fieldList->fldList[portGrpkeyFldsIndex].mask == NULL))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                          "Got null value at %d \n ", portGrpkeyFldsIndex);
                    continue;
                }
                memset(fieldList->fldList[portGrpkeyFldsIndex].value, 0x00, sizeof(uint32_t));
                memset(fieldList->fldList[portGrpkeyFldsIndex].mask, 0x00, sizeof(uint32_t));

                fieldList->fldList[portGrpkeyFldsIndex].keyFlds = XP_PCL_PORT_BIT_MAP;
                memcpy(fieldList->fldList[portGrpkeyFldsIndex].value,
                       &(tableEntryDbEntry->portListBmp[portGroupIndex]), sizeof(uint32_t));
                memcpy(fieldList->fldList[portGrpkeyFldsIndex].mask, &(portListMask),
                       sizeof(uint32_t));

                /* Each newPriority will contain TableId as 16MSBs followed by priority in the rest 16 LSBs. */
                newPriority = (tableId << 16) | (priority & 0xFFFF);

                /* Each newRuleId will contain PortGroupId as 4MSBs followed by ruleId in the rest 28 LSBs. */
                newRuleId = ((portGroupIndex << 28) | (ruleId &
                                                       0xFFFFFFF)); /* copying portGroupId and ruleId */

            }
            else
            {
                /* Set all groups acl entry to invalid execpt group 0 */
                newPriority = (tableId << 16) | (priority & 0xFFFF);
                newRuleId = ((portGroupIndex << 28) | (ruleId & 0xFFFFFFF));
                if (portGroupIndex > 0)
                {
                    rule_valid = false;
                }
            }
            if (!(fieldList->isValid))
            {
                if ((rc = cpssDxChVirtualTcamRuleValidStatusSet(tableIdDbEntry->tcamMgrId,
                                                                tableIdDbEntry->currentVtcamId,
                                                                newRuleId, GT_FALSE) != GT_OK))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          " cpssDxChVirtualTcamRuleValidStatusSet  failed with err : %d n ", rc);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            ruleData.valid               = rule_valid == false ? GT_FALSE :
                                           (GT_BOOL)fieldList->isValid;
            ruleData.rule.pcl.patternPtr = (CPSS_DXCH_PCL_RULE_FORMAT_UNT*)&pattern;
            ruleData.rule.pcl.maskPtr    = (CPSS_DXCH_PCL_RULE_FORMAT_UNT*)&mask;
            ruleData.rule.pcl.actionPtr  = &actionPtr;
            ruleAttributes.priority      = priority;
            if ((xpsRetVal = xpsUdbKeySet(fieldList, &pattern, &mask, keyFormat,
                                          tableIdDbEntry-> stage)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " xpsUdbKeySet failed with err : %d ", xpsRetVal);
                return xpsRetVal;
            }

            if (actionPtr.matchCounter.enableMatchCount)
            {
                parallelLookupNum =
                    XPS_ACL_PARALLEL_LOOKUP_0; //As currently all mirror rules are in parallel lookup 0
                if (newLocalCounterId)
                {
                    xpsRetVal = xpsAclCreateNewCounter(devId, scopeId, devType, tableIdDbEntry,
                                                       parallelLookupNum, &localCounterId);
                    if (xpsRetVal != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              " xpsAclCreateNewCounter failed with err : %d\n ", xpsRetVal);
                        return xpsRetVal;
                    }
                    ruleData.rule.pcl.actionPtr->matchCounter.matchCounterIndex = localCounterId;
                    counterIdMapDbEntry->localCounterId[counterIdMapDbEntry->numOfDuplicates] =
                        localCounterId;
                    counterIdMapDbEntry->parallelLookupNum[counterIdMapDbEntry->numOfDuplicates] =
                        parallelLookupNum;
                    counterIdMapDbEntry->numOfDuplicates++;
                    newLocalCounterId = false;
                }
                else
                {
                    ruleData.rule.pcl.actionPtr->matchCounter.matchCounterIndex =
                        counterIdMapDbEntry->localCounterId[0];
                }
            }
            if (keyFormat != XPS_PCL_IPV6_L4_KEY)
            {
                tableIdDbEntry->currentVtcamId = XPS_L3_CTRL_TABLE_ID;
            }
            else
            {
                tableIdDbEntry->currentVtcamId = XPS_L3V6_CTRL_TABLE_ID;
            }
            /* writing rule to hardware */
            if (!rule_valid)
            {
                valid = GT_FALSE;
                cpssDxChVirtualTcamRuleValidStatusGet(tableIdDbEntry->tcamMgrId,
                                                      tableIdDbEntry->currentVtcamId,
                                                      newRuleId, &valid);
                if (valid &&
                    (rc = cpssDxChVirtualTcamRuleDelete(tableIdDbEntry->tcamMgrId,
                                                        tableIdDbEntry->currentVtcamId,
                                                        newRuleId)) != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          " cpssDxChVirtualTcamRuleDelete  failed with err : %d n ", rc);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            else
            {
                if ((rc = cpssDxChVirtualTcamRuleWrite(tableIdDbEntry->tcamMgrId,
                                                       tableIdDbEntry->currentVtcamId,
                                                       newRuleId, &ruleAttributes,
                                                       &tcamRuleType, &ruleData)) != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          " cpssDxChVirtualTcamRuleWrite  failed with err : %d n ", rc);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            tableEntryDbEntry -> isDirty = false;
        }

    }
    /* User ACL */
    else
    {
        if (tableIdDbEntry->bindCount || needDeploy)
        {
            /* Duplication of rules for all the ports of different port groups sharing the same rule */
            for (portGroupIndex = 0; portGroupIndex < XPS_PORT_LIST_PORT_GROUP_MAX;
                 portGroupIndex++)
            {
                uint32_t portListMask = 0;
                uint32_t portListBmp = 0;
                bool rule_valid = true;
                if (tableEntryDbEntry->portGroupBmp[portGroupIndex])
                {
                    portListMask = ~(tableEntryDbEntry->portListBmp[portGroupIndex]);
                    portListBmp = tableEntryDbEntry->portListBmp[portGroupIndex];
                }
                else if (!hasInPortsFld &&
                         tableIdDbEntry->portGroupBmp[portGroupIndex]) /*If not in_ports in acl field, use aclTable bind Ports*/
                {
                    portListMask = ~(tableIdDbEntry->portListBmp[portGroupIndex]);
                    portListBmp = tableIdDbEntry->portListBmp[portGroupIndex];
                }
                else /* If portGroup is not in the range of in_ports, set this entry to invalid */
                {
                    portListBmp = 0x0;
                    rule_valid = false;
                }
                portListMask&=0x00FFFFFF;
                /* no inversion should be done on the masking of the port group id (bits 24-27)*/
                portListMask|=0x0F000000;

                /* if only outer_vlan_id in acl entry, write to hw and set portListMask to 0*/
                if (!hasInPortsFld && needDeploy)
                {
                    portListMask = 0x0;
                    rule_valid = true;
                    needDeploy = false;
                }
                /* NOTE: Offset to Key fields should not be changed
                   and should be same for all port groups. */
                if (keyFldsIndex == 0)
                {
                    portGrpkeyFldsIndex = fieldList->numFlds;
                    fieldList->numFlds++;
                }
                if ((fieldList->fldList[portGrpkeyFldsIndex].value == NULL) ||
                    (fieldList->fldList[portGrpkeyFldsIndex].mask == NULL))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                          "Got null value at %d \n ", portGrpkeyFldsIndex);
                    continue;
                }
                memset(fieldList->fldList[portGrpkeyFldsIndex].value, 0x00, sizeof(uint32_t));
                memset(fieldList->fldList[portGrpkeyFldsIndex].mask, 0x00, sizeof(uint32_t));

                fieldList->fldList[portGrpkeyFldsIndex].keyFlds = XP_PCL_PORT_BIT_MAP;
                memcpy(fieldList->fldList[portGrpkeyFldsIndex].value,
                       &(portListBmp), sizeof(uint32_t));
                memcpy(fieldList->fldList[portGrpkeyFldsIndex].mask, &(portListMask),
                       sizeof(uint32_t));
                /* Each newPriority will contain TableId as 16MSBs followed by priority in the rest 16 LSBs. */
                newPriority = (tableId << 16) | (priority & 0xFFFF);

                /* Each newRuleId will contain PortGroupId as 4MSBs followed by ruleId in the rest 28 LSBs. */
                newRuleId = ((portGroupIndex << 28) | (ruleId &
                                                       0xFFFFFFF)); /* copying portGroupId and ruleId */

                if (tableIdDbEntry->isVlan == true)
                {
                    if (keyFldsIndex == 0)
                    {
                        vlanKeyFldsIndex = fieldList->numFlds;
                        fieldList->numFlds++;
                    }
                    memset(fieldList->fldList[vlanKeyFldsIndex].value, 0x00, sizeof(uint16_t));
                    memset(fieldList->fldList[vlanKeyFldsIndex].mask, 0x00, sizeof(uint16_t));

                    uint16_t vlanId = (tableIdDbEntry->vlanId) & 0xFFF; /* vlan id is 12 bits only*/
                    uint16_t mask = 0x0FFF;
                    fieldList->fldList[vlanKeyFldsIndex].keyFlds = XP_PCL_VID;
                    memcpy(fieldList->fldList[vlanKeyFldsIndex].value, &vlanId, sizeof(uint16_t));
                    memcpy(fieldList->fldList[vlanKeyFldsIndex].mask, &mask, sizeof(uint16_t));
                    fieldList->numFlds++;
                }

                keyFldsIndex = 1;
                GT_BOOL valid = GT_FALSE;
                cpssDxChVirtualTcamRuleValidStatusGet(tableIdDbEntry->tcamMgrId,
                                                      tableIdDbEntry->currentVtcamId,
                                                      newRuleId, &valid);
                if (is_update)
                {
                    ruleData.rule.pcl.patternPtr = (CPSS_DXCH_PCL_RULE_FORMAT_UNT*)&pattern;
                    ruleData.rule.pcl.maskPtr    = (CPSS_DXCH_PCL_RULE_FORMAT_UNT*)&mask;
                    ruleData.rule.pcl.actionPtr  = &dummyActionPtr;

                    if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV4_IPV6)
                    {
                        xpsRetVal = xpsAclGetVtcamIdByKeyFormat(tableIdDbEntry, oldkeyFormat,
                                                                &tableIdDbEntry->currentVtcamId);
                        if (xpsRetVal != XP_NO_ERR)
                        {
                            return xpsRetVal;
                        }
                        /*when keyformat has changed , we should remove the old entry with corresponding vtcamid, ruleFormat */
                        if (keyFormat != oldkeyFormat)
                        {
                            keyFormatHasChange = true;
                            if (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)
                            {
                                if ((oldkeyFormat == XPS_PCL_NON_IP_KEY) ||
                                    (oldkeyFormat == XPS_PCL_IPV4_L4_KEY) ||
                                    (oldkeyFormat == XPS_PCL_ARP_KEY))
                                {
                                    tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
                                }
                                else if (oldkeyFormat == XPS_PCL_IPV6_L4_KEY)
                                {
                                    tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
                                }
                            }
                            else if (tableIdDbEntry->stage == XPS_PCL_STAGE_EGRESS)
                            {
                                if ((oldkeyFormat == XPS_PCL_NON_IP_KEY) ||
                                    (oldkeyFormat == XPS_PCL_IPV4_L4_KEY) ||
                                    (oldkeyFormat == XPS_PCL_ARP_KEY))
                                {
                                    tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_EGRESS_UDB_30_E;
                                }
                                else if (oldkeyFormat == XPS_PCL_IPV6_L4_KEY)
                                {
                                    tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_EGRESS_UDB_60_E;
                                }
                            }
                        }
                    }

                    if (rule_valid && valid)
                    {
                        if ((rc = cpssDxChVirtualTcamRuleRead(tableIdDbEntry->tcamMgrId,
                                                              tableIdDbEntry->currentVtcamId,
                                                              newRuleId, &tcamRuleType, &ruleData)) != GT_OK)
                        {

                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  " cpssDxChVirtualTcamRuleRead  failed with err : %d \n ", rc);
                            return xpsConvertCpssStatusToXPStatus(rc);
                        }
                        /* reset replaceFld to 0 */
                        memset(&ruleData.rule.pcl.maskPtr->ruleIngrUdbOnly.replacedFld, 0,
                               sizeof(ruleData.rule.pcl.maskPtr->ruleIngrUdbOnly.replacedFld));
                        memset(&ruleData.rule.pcl.patternPtr->ruleIngrUdbOnly.replacedFld, 0,
                               sizeof(ruleData.rule.pcl.patternPtr->ruleIngrUdbOnly.replacedFld));
                        if ((rc = cpssDxChVirtualTcamRulePriorityGet(tableIdDbEntry->tcamMgrId,
                                                                     tableIdDbEntry->currentVtcamId,
                                                                     newRuleId, &currentPriority)) != GT_OK)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  " cpssDxChVirtualTcamRulePriorityGet  failed with err : %d \n ", rc);
                            return xpsConvertCpssStatusToXPStatus(rc);

                        }
                        if (newPriority != currentPriority)
                        {
                            priorityUpdate = true;
                        }

                        /* TODO Check. below code is not needed
                         * Action cannot be changed to mirror directly. Mirror rules will be part of different ACL table type */

                    }
                    if ((actionPtr.mirror.mirrorToRxAnalyzerPort == true) &&
                        (dummyActionPtr.mirror.mirrorToRxAnalyzerPort == false))
                    {
                        //Trying to update some packet action to MIRROR: Current rule needs to be deleted and moved to second lookup

                        for (uint32_t j=0; j<tableIdDbEntry->numOfDuplicates; j++)
                        {
                            valid = GT_FALSE;
                            /* Deleting rule from hardware */
                            cpssDxChVirtualTcamRuleValidStatusGet(tableIdDbEntry->tcamMgrId,
                                                                  tableIdDbEntry->vtcamId[j],
                                                                  newRuleId, &valid);
                            if (valid && (rc = cpssDxChVirtualTcamRuleDelete(tableIdDbEntry->tcamMgrId,
                                                                             tableIdDbEntry->vtcamId[j], newRuleId)) != GT_OK)
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                      " cpssDxChVirtualTcamRuleDelete  failed with err : %d n ", rc);
                                return xpsConvertCpssStatusToXPStatus(rc);
                            }
                        }
                        if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
                        {
                            tableIdDbEntry->currentVtcamId = XPS_L3V6_MIRROR_ING_STG_TABLE_ID_0;
                        }
                        else
                        {
                            tableIdDbEntry->currentVtcamId = XPS_L3_MIRROR_ING_STG_TABLE_ID_0;
                        }

                    }
                    if ((actionPtr.mirror.mirrorToRxAnalyzerPort == false) &&
                        (dummyActionPtr.mirror.mirrorToRxAnalyzerPort == true))
                    {
                        //Trying to change MIRROR action to some other action: Rules need to be moved back to previous position

                        if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
                        {
                            vtcamId = XPS_L3V6_MIRROR_ING_STG_TABLE_ID_0;
                        }
                        else
                        {
                            vtcamId = XPS_L3_MIRROR_ING_STG_TABLE_ID_0;
                        }
                        valid = GT_FALSE;
                        /* Deleting rule from hardware */
                        cpssDxChVirtualTcamRuleValidStatusGet(tableIdDbEntry->tcamMgrId,
                                                              vtcamId,
                                                              newRuleId, &valid);
                        if (valid &&
                            (rc = cpssDxChVirtualTcamRuleDelete(tableIdDbEntry->tcamMgrId, vtcamId,
                                                                newRuleId)) != GT_OK)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  " cpssDxChVirtualTcamRuleDelete  failed with err : %d n ", rc);
                            return xpsConvertCpssStatusToXPStatus(rc);
                        }

                        tableIdDbEntry->currentVtcamId = tableIdDbEntry->vtcamId[0];

                        if ((xpsRetVal =  xpsAclUpdateMirrorTableVtcamMap(devId, tableId)) != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "  xpsAclUpdateMirrorTableVtcamMap failed with err : %d ", xpsRetVal);
                            return xpsRetVal;
                        }

                    }

                    if (keyFormatHasChange)
                    {
                        /* Deleting rule from hardware */
                        cpssDxChVirtualTcamRuleValidStatusGet(tableIdDbEntry->tcamMgrId,
                                                              tableIdDbEntry->currentVtcamId,
                                                              newRuleId, &valid);
                        if (valid && (rc = cpssDxChVirtualTcamRuleDelete(tableIdDbEntry->tcamMgrId,
                                                                         tableIdDbEntry->currentVtcamId,
                                                                         newRuleId)) != GT_OK)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  " cpssDxChVirtualTcamRuleDelete  failed with err : %d n ", rc);
                            return xpsConvertCpssStatusToXPStatus(rc);
                        }
                        /*when old entry has been removed, we should update ruleFormat to keep it consistent with keyformat*/
                        if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV4_IPV6)
                        {
                            if (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)
                            {
                                if (keyFormat == XPS_PCL_IPV6_L4_KEY)
                                {
                                    tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
                                }
                                else
                                {
                                    tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
                                }
                            }
                            else if (tableIdDbEntry->stage == XPS_PCL_STAGE_EGRESS)
                            {
                                if (keyFormat == XPS_PCL_IPV6_L4_KEY)
                                {
                                    tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_EGRESS_UDB_60_E;
                                }
                                else
                                {
                                    tcamRuleType.rule.pcl.ruleFormat = XPS_PCL_RULE_FORMAT_EGRESS_UDB_30_E;
                                }
                            }

                            memset(ruleData.rule.pcl.patternPtr->ruleIngrUdbOnly.udb, 0,
                                   sizeof(ruleData.rule.pcl.patternPtr->ruleIngrUdbOnly.udb));
                            memset(ruleData.rule.pcl.maskPtr->ruleIngrUdbOnly.udb, 0,
                                   sizeof(ruleData.rule.pcl.maskPtr->ruleIngrUdbOnly.udb));
                        }

                    }
                    memcpy(&pattern, ruleData.rule.pcl.patternPtr, sizeof(xpsPclRuleFormat_t));
                    memcpy(&mask, ruleData.rule.pcl.maskPtr, sizeof(xpsPclRuleFormat_t));
                }

                if ((xpsRetVal = xpsUdbKeySet(fieldList, &pattern, &mask, keyFormat,
                                              tableIdDbEntry-> stage)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          " xpsUdbKeySet failed with err : %d ", xpsRetVal);
                    return xpsRetVal;
                }

                if (!(fieldList->isValid))
                {
                    if (valid &&
                        ((rc = cpssDxChVirtualTcamRuleValidStatusSet(tableIdDbEntry->tcamMgrId,
                                                                     tableIdDbEntry->currentVtcamId,
                                                                     newRuleId, GT_FALSE) != GT_OK)))
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              " cpssDxChVirtualTcamRuleValidStatusSet  failed with err : %d n ", rc);
                        return xpsConvertCpssStatusToXPStatus(rc);
                    }
                }

                if (priorityUpdate == true)
                {
                    for (uint32_t i = 0; i< tableIdDbEntry->numOfDuplicates; i++)
                    {
                        if ((tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)&&
                            (tableIdDbEntry->isMirror))
                        {
                            if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
                            {
                                vtcamId = XPS_L3V6_MIRROR_ING_STG_TABLE_ID_0;
                            }
                            else
                            {
                                vtcamId = XPS_L3_MIRROR_ING_STG_TABLE_ID_0;
                            }
                        }
                        else
                        {
                            vtcamId = tableIdDbEntry->vtcamId[i];
                        }

                        if ((rc = cpssDxChVirtualTcamRulePriorityUpdate(tableIdDbEntry->tcamMgrId,
                                                                        vtcamId,
                                                                        newRuleId, newPriority, XPS_EQUAL_PRIORITY_RULE_POSITION_LAST_E)) != GT_OK)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  " cpssDxChVirtualTcamRulePriorityUpdate  failed with err : %d n ", rc);
                            return xpsConvertCpssStatusToXPStatus(rc);
                        }
                    }
                    if (!tableIdDbEntry->numOfDuplicates && rule_valid)
                    {
                        xpsRetVal = xpsAclGetVtcamIdByKeyFormat(tableIdDbEntry, keyFormat, &vtcamId);
                        if (xpsRetVal != XP_NO_ERR)
                        {
                            return xpsRetVal;
                        }
                        if ((rc = cpssDxChVirtualTcamRulePriorityUpdate(tableIdDbEntry->tcamMgrId,
                                                                        vtcamId,
                                                                        newRuleId, newPriority, XPS_EQUAL_PRIORITY_RULE_POSITION_LAST_E)) != GT_OK)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  " cpssDxChVirtualTcamRulePriorityUpdate  failed with err : %d n ", rc);
                            return xpsConvertCpssStatusToXPStatus(rc);
                        }
                    }
                }
                else
                {
                    ruleData.valid               = rule_valid == false ? GT_FALSE :
                                                   (GT_BOOL)fieldList->isValid;
                    ruleData.rule.pcl.patternPtr = (CPSS_DXCH_PCL_RULE_FORMAT_UNT*)&pattern;
                    ruleData.rule.pcl.maskPtr    = (CPSS_DXCH_PCL_RULE_FORMAT_UNT*)&mask;
                    ruleData.rule.pcl.actionPtr  = &actionPtr;
                    ruleAttributes.priority   = newPriority;

                    for (uint32_t i = 0; i< tableIdDbEntry->numOfDuplicates; i++)
                    {
                        if (tableIdDbEntry->isMirror)
                        {
                            parallelLookupNum =
                                XPS_ACL_PARALLEL_LOOKUP_0; //As currently all mirror rules are in parallel lookup 0
                        }
                        else if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
                        {
                            if (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)
                            {
                                parallelLookupNum = tableIdDbEntry->vtcamId[i] -
                                                    XPS_ING_STG_V6_USER_ACL_TABLE_START;
                            }
                            else
                            {
                                parallelLookupNum = tableIdDbEntry->vtcamId[i] -
                                                    XPS_EGR_STG_V6_USER_ACL_TABLE_START;
                            }
                        }
                        else
                        {
                            if (tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)
                            {
                                parallelLookupNum = tableIdDbEntry->vtcamId[i] -
                                                    XPS_ING_STG_V4_USER_ACL_TABLE_START;
                            }
                            else
                            {
                                parallelLookupNum = tableIdDbEntry->vtcamId[i] -
                                                    XPS_EGR_STG_V4_USER_ACL_TABLE_START;
                            }
                        }

                        if (actionPtr.matchCounter.enableMatchCount)
                        {
                            /*numOfDuplicates can not more that XPS_ACL_MAX_TBLS_IN_GROUP*/
                            if ((i<counterIdMapDbEntry->numOfDuplicates)&&
                                (counterIdMapDbEntry->parallelLookupNum[i] == parallelLookupNum))
                            {
                                newLocalCounterId = false;
                            }

                            if (newLocalCounterId)
                            {
                                xpsRetVal = xpsAclCreateNewCounter(devId, scopeId, devType, tableIdDbEntry,
                                                                   parallelLookupNum, &localCounterId);
                                if (xpsRetVal != XP_NO_ERR)
                                {
                                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                          " xpsAclCreateNewCounter failed with err : %d\n ", xpsRetVal);
                                    return xpsRetVal;
                                }
                                ruleData.rule.pcl.actionPtr->matchCounter.matchCounterIndex = localCounterId;
                                counterIdMapDbEntry->localCounterId[counterIdMapDbEntry->numOfDuplicates] =
                                    localCounterId;
                                counterIdMapDbEntry->parallelLookupNum[counterIdMapDbEntry->numOfDuplicates] =
                                    parallelLookupNum;
                                counterIdMapDbEntry->numOfDuplicates++;
                                newLocalCounterId = false;
                            }
                            else
                            {
                                ruleData.rule.pcl.actionPtr->matchCounter.matchCounterIndex =
                                    counterIdMapDbEntry->localCounterId[i];
                            }
                        }
                        if ((tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)&&
                            (tableIdDbEntry->isMirror))
                        {
                            if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
                            {
                                vtcamId = XPS_L3V6_MIRROR_ING_STG_TABLE_ID_0;
                            }
                            else
                            {
                                vtcamId = XPS_L3_MIRROR_ING_STG_TABLE_ID_0;
                            }
                            tableIdDbEntry->currentVtcamId = vtcamId;
                        }
                        else
                        {
                            vtcamId = tableIdDbEntry->vtcamId[i];
                        }
                        /* writing rule to hardware */
                        if (!rule_valid)
                        {
                            valid = GT_FALSE;
                            cpssDxChVirtualTcamRuleValidStatusGet(tableIdDbEntry->tcamMgrId,
                                                                  vtcamId, newRuleId, &valid);
                            if (valid &&
                                (rc = cpssDxChVirtualTcamRuleDelete(tableIdDbEntry->tcamMgrId, vtcamId,
                                                                    newRuleId)) != GT_OK)
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                      " cpssDxChVirtualTcamRuleDelete  failed with err : %d n ", rc);
                                return xpsConvertCpssStatusToXPStatus(rc);
                            }
                        }
                        else
                        {
                            if ((rc = cpssDxChVirtualTcamRuleWrite(tableIdDbEntry->tcamMgrId, vtcamId,
                                                                   newRuleId, &ruleAttributes,
                                                                   &tcamRuleType, &ruleData)) != GT_OK)
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                      "ACL write failed rId %u newRid %u prio %u newPri %d err %d \n ",
                                      ruleId, newRuleId, priority, newPriority, rc);
                                return xpsConvertCpssStatusToXPStatus(rc);
                            }
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                                  " Rule Created Successfully with ruleId : %d  vtcamId : %d \n ", newRuleId,
                                  vtcamId);
                        }
                        if (tableIdDbEntry->isMirror)
                        {
                            break; // mirror rules will never have duplicates as every rule is in 2nd lookup in parrallel lookup 0
                        }
                    }
                    /* write to hw when acl table don`t bound to others*/
                    if (!tableIdDbEntry->numOfDuplicates)
                    {
                        parallelLookupNum = XPS_ACL_PARALLEL_LOOKUP_0;

                        xpsRetVal = xpsAclGetVtcamIdByKeyFormat(tableIdDbEntry, keyFormat, &vtcamId);
                        if (xpsRetVal != XP_NO_ERR)
                        {
                            return xpsRetVal;
                        }
                        tableIdDbEntry->currentVtcamId = vtcamId;
                        if (actionPtr.matchCounter.enableMatchCount)
                        {
                            if (newLocalCounterId)
                            {
                                xpsRetVal = xpsAclCreateNewCounter(devId, scopeId, devType, tableIdDbEntry,
                                                                   parallelLookupNum, &localCounterId);
                                if (xpsRetVal != XP_NO_ERR)
                                {
                                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                          " xpsAclCreateNewCounter failed with err : %d\n ", xpsRetVal);
                                    return xpsRetVal;
                                }
                                ruleData.rule.pcl.actionPtr->matchCounter.matchCounterIndex = localCounterId;
                                counterIdMapDbEntry->localCounterId[counterIdMapDbEntry->numOfDuplicates] =
                                    localCounterId;
                                counterIdMapDbEntry->parallelLookupNum[counterIdMapDbEntry->numOfDuplicates] =
                                    parallelLookupNum;
                                counterIdMapDbEntry->numOfDuplicates++;
                                newLocalCounterId = false;
                            }
                            else
                            {
                                ruleData.rule.pcl.actionPtr->matchCounter.matchCounterIndex =
                                    counterIdMapDbEntry->localCounterId[0];
                            }
                        }

                        /* writing rule to hardware */
                        if (!rule_valid)
                        {
                            valid = GT_FALSE;
                            cpssDxChVirtualTcamRuleValidStatusGet(tableIdDbEntry->tcamMgrId,
                                                                  vtcamId, newRuleId, &valid);
                            if (valid &&
                                (rc = cpssDxChVirtualTcamRuleDelete(tableIdDbEntry->tcamMgrId, vtcamId,
                                                                    newRuleId)) != GT_OK)
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                      " cpssDxChVirtualTcamRuleDelete  failed with err : %d n ", rc);
                                return xpsConvertCpssStatusToXPStatus(rc);
                            }
                        }
                        else
                        {
                            if ((rc = cpssDxChVirtualTcamRuleWrite(tableIdDbEntry->tcamMgrId, vtcamId,
                                                                   newRuleId, &ruleAttributes,
                                                                   &tcamRuleType, &ruleData)) != GT_OK)
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                      "ACL write failed rId %u newRid %u prio %u newPri %d err %d , ruleFormat:%d, vtcamId:%d\n ",
                                      ruleId, newRuleId, priority, newPriority, rc, tcamRuleType.rule.pcl.ruleFormat,
                                      vtcamId);
                                return xpsConvertCpssStatusToXPStatus(rc);
                            }
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                                  "Rule Created Successfully with ruleId : %d  vtcamId : %d  keyFormat:%d, ruleFormat:%d\n ",
                                  newRuleId,
                                  vtcamId, keyFormat, tcamRuleType.rule.pcl.ruleFormat);
                        }
                    }
                }
            }
            tableEntryDbEntry ->isDirty = false; //indicating rule is written into hw
        }
    }
    /* If portbitmap is non-zero, this is incremented to hold
      XPS_PORT_BITMAP. Reset this to original value before saving to DB. */
    if ((origNumFlds) && (origNumFlds != fieldList->numFlds))
    {
        fieldList->numFlds = origNumFlds;
    }

    tableEntryDbEntry->rule.numFlds = fieldList->numFlds;
    tableEntryDbEntry->rule.isValid = fieldList->isValid;
    tableEntryDbEntry->rule.fldList = fieldList->fldList;

    return XP_NO_ERR;
}


XP_STATUS xpsAclWriteEntry(xpsDevice_t devId, uint32_t tableId,
                           uint32_t priority, uint32_t ruleId, xpsAclkeyFieldList_t *fieldList,
                           xpsPclAction_t actionPtr, xpsAclKeyFormat keyFormat, bool is_control_acl)
{

    XPS_LOCK(xpsAclWriteEntry);

    XP_STATUS                           xpsRetVal = XP_NO_ERR;

    if ((xpsRetVal = xpsAclWriteEntryKeySet(devId, tableId, priority, ruleId,
                                            fieldList, actionPtr, false, keyFormat, is_control_acl)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to write Entry in tableId :%d\n", tableId);
        return xpsRetVal;
    }

    return XP_NO_ERR;
}


XP_STATUS xpsUpdateAclEntry(xpsDevice_t devId, uint32_t tableId,
                            uint32_t priority, uint32_t ruleId, xpsAclkeyFieldList_t *fieldList,
                            xpsPclAction_t actionPtr, xpsAclKeyFormat keyFormat, bool is_control_acl)
{

    XPS_LOCK(xpsUpdateAclEntry);

    XP_STATUS                           xpsRetVal = XP_NO_ERR;

    if ((xpsRetVal = xpsAclWriteEntryKeySet(devId, tableId, priority, ruleId,
                                            fieldList, actionPtr, true, keyFormat, is_control_acl)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to write Entry in tableId :%d\n", tableId);
        return xpsRetVal;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclReadData(xpsDevice_t devId, uint32_t tableId,
                         uint32_t ruleId, xpsPclAction_t *actionPtr, xpsAclKeyFormat keyFormat)
{
    XPS_LOCK(xpsAclTypeReadData);

    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsAclTableContextDbEntry_t         *tableIdDbEntry = NULL;
    xpsAclTableContextDbEntry_t         tableIdDbKey;
    xpsAclEntryContextDbEntry_t              tableEntryDbKey;
    xpsAclEntryContextDbEntry_t             *tableEntryDbEntry = NULL;
    xpsScope_t                 scopeId;

    /* Get scope from Device ID */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        return xpsRetVal;
    }

    tableIdDbKey.tableId = tableId;

    if ((xpsRetVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                        (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    /* retrieving key fields and action info */

    tableEntryDbKey.key = XPS_PCL_TABLE_ID_KEY(tableId, ruleId);

    if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableEntryDbHndl,
                                         (xpsDbKey_t)&tableEntryDbKey, (void**)&tableEntryDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    if (tableEntryDbEntry == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Retrieving ACL table entry failed, devId(%d)", devId);
        return XP_ERR_KEY_NOT_FOUND;
    }

    actionPtr = &(tableEntryDbEntry->action);

    return XP_NO_ERR;
}

XP_STATUS xpsAclCounterBlockClientSet(xpsDevice_t devId,
                                      uint32_t parallelLookupNum, uint32_t packetCntrBlockNum,
                                      uint32_t byteCntrBlockNum, bool enable)
{
    GT_STATUS       rc  =  GT_OK;
    CPSS_DXCH_CNC_CLIENT_ENT    cncClientIng;
    CPSS_DXCH_CNC_CLIENT_ENT    cncClientIng1;
    CPSS_DXCH_CNC_CLIENT_ENT    cncClientEgr;

    switch (parallelLookupNum)
    {
        case XPS_ACL_PARALLEL_LOOKUP_0:
            cncClientIng = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E;
            cncClientIng1 = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E;
            cncClientEgr = CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E;
            break;
        case XPS_ACL_PARALLEL_LOOKUP_1:
            cncClientIng = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E;
            cncClientIng1 = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E;
            cncClientEgr = CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E;
            break;
        case XPS_ACL_PARALLEL_LOOKUP_2:
            cncClientIng = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E;
            cncClientIng1 = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E;
            cncClientEgr = CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E;
            break;
        case XPS_ACL_PARALLEL_LOOKUP_3:
            cncClientIng = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E;
            cncClientIng1 = CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E;
            cncClientEgr = CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E;
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Invalid parallelLookupNum : %d\n ", parallelLookupNum);
            return XP_ERR_INVALID_DATA;
    }

    if ((rc = cpssHalPclCounterBlockConfigure(devId,
                                              packetCntrBlockNum, byteCntrBlockNum,
                                              cncClientIng,
                                              (GT_BOOL)enable)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsAllocatorAllocateId failed with err : %d\n ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    if ((rc = cpssHalPclCounterBlockConfigure(devId,
                                              packetCntrBlockNum, byteCntrBlockNum,
                                              cncClientIng1,
                                              (GT_BOOL)enable)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsAllocatorAllocateId failed with err : %d\n ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    if ((rc = cpssHalPclCounterBlockConfigure(devId,
                                              packetCntrBlockNum, byteCntrBlockNum,
                                              cncClientEgr, (GT_BOOL)enable)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsAllocatorAllocateId failed with err : %d\n ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }
    return XP_NO_ERR;
}

XP_STATUS xpsAclSetPortAcl(xpsDevice_t devId, uint32_t portId,
                           uint32_t tableId, uint32_t groupId)
{
    XP_STATUS                               xpsRetVal = XP_NO_ERR;
    uint32_t                                portGroup;
    uint32_t                                offset;
    xpsAclTableContextDbEntry_t             tableIdDbKey;
    xpsAclTableContextDbEntry_t             *tableIdDbEntry = NULL;
    xpsScope_t                              scopeId = 0;
    uint32_t                                portNum = 0;
    xpsLagPortIntfList_t                    portList;
    memset(&portList, 0, sizeof(portList));

    portNum = xpsGlobalPortToPortnum(devId, portId);
    if (portNum == 0xFFFF)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid port Num : %d \n ", portId);
        return XP_ERR_INVALID_VALUE;
    }

    /* Get scope from Device ID */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        return xpsRetVal;
    }

    /* Retrieving table info */
    tableIdDbKey.tableId = tableId ;
    if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                         (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    if (tableIdDbEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    xpsRetVal = xpsAclUpdateTableVtcamMapping(devId, tableId, groupId);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the port-portgroup mapping for portlist. error:%d\n", xpsRetVal);
        return xpsRetVal;
    }

    portGroup = (portNum)/XPS_PORT_LIST_OFFSET;
    offset = (0x1)<<((portNum)%(XPS_PORT_LIST_OFFSET));

    if (portGroup >= XPS_PORT_LIST_PORT_GROUP_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid port Num : %d \n ", portNum);
        return XP_ERR_INVALID_VALUE;
    }

    tableIdDbEntry->portGroupBmp[portGroup]++;
    tableIdDbEntry->portListBmp[portGroup] |= ((portGroup << XPS_PORT_LIST_OFFSET)|
                                               offset);
    portList.size = 1;
    portList.portIntf[0] = portId;

    xpsRetVal = xpsAclSetIntfObjAcl(devId, portId, tableId, false, tableIdDbEntry,
                                    &portList);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Port xpsAclSetIntfObjAcl error:%d\n", xpsRetVal);
        if (tableIdDbEntry->portGroupBmp[portGroup])
        {
            tableIdDbEntry->portListBmp[portGroup] &= (~offset);
        }
        tableIdDbEntry->portGroupBmp[portGroup]--;
        tableIdDbEntry->bindCount--;
        return xpsRetVal;
    }

    return XP_NO_ERR;
}

/*TODO Vlan ACL is not supported. Revisit this code and align with port/lag*/
XP_STATUS xpsAclSetVlanAcl(xpsDevice_t devId, uint32_t vlanId,
                           uint32_t tableId, uint32_t groupId)
{

    XP_STATUS                               xpStatus = XP_NO_ERR;
    xpsInterfaceId_t                        *intfList  = NULL;
    xpsInterfaceId_t                        intfId;
    uint16_t                                numOfIntfs = 0;
    xpsInterfaceType_e                      intfType;
    xpsAclTableContextDbEntry_t             tableIdDbKey;
    xpsAclTableContextDbEntry_t             *tableIdDbEntry = NULL;
    xpsLagPortIntfList_t                    portList;
    uint32_t                                portGroup;
    uint32_t                                offset;
    uint32_t                                portNum;
    uint32_t                                cpssPortNum;
    xpsScope_t                              scopeId = 0;

    /* Get scope from Device ID */
    if ((xpStatus = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsScopeGetScopeId failed with err : %d \n ", xpStatus);
        return xpStatus;
    }

    memset(&portList, 0, sizeof(xpsLagPortIntfList_t));

    xpStatus = xpsVlanGetInterfaceList(devId, vlanId, &intfList, &numOfIntfs);
    if (xpStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to get interface list of vlan :%d error:%d \n", vlanId, xpStatus);
        return xpStatus;
    }

    if (!numOfIntfs)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "no vlan member for vlanId ::%d", vlanId);
    }

    xpStatus = xpsAclUpdateTableVtcamMapping(devId, tableId, groupId);
    if (xpStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the port-portgroup mapping for portlist. error:%d\n", xpStatus);
        return xpStatus;
    }

    /* Retrieving table info */
    tableIdDbKey.tableId = tableId ;
    if (((xpStatus = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                        (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpStatus);
        return xpStatus;
    }
    if (tableIdDbEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    tableIdDbEntry->isVlan = true;
    tableIdDbEntry->vlanId = vlanId;

    for (int i = 0; i < numOfIntfs; i++)
    {
        //Get interfacetype
        intfId = intfList[i];
        xpStatus = xpsInterfaceGetTypeScope(devId, intfId, &intfType);
        if (xpStatus != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Interface Get type for interface(%d) failed", intfId);
            return xpStatus;
        }

        if (intfType == XPS_LAG)
        {
            /* Get the Lag member port list */
            xpStatus = xpsLagGetPortIntfList(intfId, &portList);
            if (xpStatus != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to Lag port members, "
                      "device:%d, lag:%d, pcl:%d, error:%d\n ", devId, intfId, xpStatus);
                return xpStatus;
            }
        }
        else if (intfType == XPS_PORT)
        {
            portList.portIntf[portList.size] = intfId;
            (portList.size)++;
        }
    }

    for (uint32_t index = 0; index < portList.size; index++)
    {
        portNum = portList.portIntf[index];
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

        portGroup = (cpssPortNum)/XPS_PORT_LIST_OFFSET;
        offset = (0x1)<<((cpssPortNum)%(XPS_PORT_LIST_OFFSET));
        tableIdDbEntry->portGroupBmp[portGroup]++;
        tableIdDbEntry->portListBmp[portGroup] |= ((portGroup << XPS_PORT_LIST_OFFSET)|
                                                   offset);
    }

    xpStatus = xpsAclSetIntfObjAcl(devId, vlanId, tableId, true, tableIdDbEntry,
                                   &portList);
    if (xpStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vlan xpsAclSetIntfObjAcl error:%d\n", xpStatus);
        for (uint32_t index = 0; index < portList.size; index++)
        {
            portNum = portList.portIntf[index];
            cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

            portGroup = (cpssPortNum)/XPS_PORT_LIST_OFFSET;
            offset = (0x1)<<((cpssPortNum)%(XPS_PORT_LIST_OFFSET));
            tableIdDbEntry->portListBmp[portGroup] &= (~offset);
            tableIdDbEntry->portGroupBmp[portGroup]--;
        }
        tableIdDbEntry->bindCount--;
        return xpStatus;
    }

    return XP_NO_ERR;
}

/*TODO Vlan ACL is not supported. Revisit this code and align with port/lag*/
XP_STATUS xpsAclVlanUnbind(xpsDevice_t devId, uint32_t vlanId,
                           uint32_t groupId, uint32_t tableId)
{
    XP_STATUS                               xpsRetVal = XP_NO_ERR;
    xpsAclTableContextDbEntry_t             tableIdDbKey;
    xpsAclTableContextDbEntry_t             *tableIdDbEntry = NULL;
    xpsInterfaceId_t                        *intfList  = NULL;
    xpsInterfaceId_t                        intfId;
    uint16_t                                numOfIntfs = 0;
    xpsInterfaceType_e                      intfType;
    xpsScope_t                              scopeId = 0;
    xpsLagPortIntfList_t                    portList;
    uint32_t                                portGroup;
    uint32_t                                offset;
    uint32_t                                portNum;
    uint32_t                                cpssPortNum;

    /* Get scope from Device ID */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        return xpsRetVal;
    }

    /* Retrieving table info */
    tableIdDbKey.tableId = tableId ;
    if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                         (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }
    if (tableIdDbEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    xpsRetVal = xpsVlanGetInterfaceList(devId, vlanId, &intfList, &numOfIntfs);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to get interface list of vlan :%d error:%d \n", vlanId, xpsRetVal);
        return xpsRetVal;
    }

    if (!numOfIntfs)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "no vlan member for vlanId ::%d", vlanId);
    }

    for (int i =0; i<numOfIntfs; i++)
    {
        //Get interfacetype
        intfId = intfList[i];
        xpsRetVal = xpsInterfaceGetTypeScope(devId, intfId, &intfType);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Interface Get type for interface(%d) failed", intfId);
            return xpsRetVal;
        }

        if (intfType == XPS_LAG)
        {
            /* Get the Lag member port list */
            xpsRetVal = xpsLagGetPortIntfList(intfId, &portList);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to Lag port members, "
                      "device:%d, lag:%d, pcl:%d, error:%d\n ", devId, intfId, xpsRetVal);
                return xpsRetVal;
            }
        }
        else if (intfType == XPS_PORT)
        {
            portList.portIntf[portList.size] = intfId;
            (portList.size)++;
        }
    }

    //decrease the bindcount per table
    if (tableIdDbEntry->bindCount)
    {
        tableIdDbEntry->bindCount--;
    }

    //remove the unbound port from the portlist and update the rules in hw
    for (uint32_t i = 0; i < portList.size; i++)
    {
        portNum = portList.portIntf[i];
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

        portGroup = (cpssPortNum)/XPS_PORT_LIST_OFFSET;
        offset = (0x1)<<((cpssPortNum)%(XPS_PORT_LIST_OFFSET));
        if (tableIdDbEntry->portGroupBmp[portGroup])
        {
            tableIdDbEntry->portListBmp[portGroup] &= (~offset);
        }
    }
    tableIdDbEntry->isVlan = false;

    xpsRetVal = xpsAclUnbindIntfObjAcl(devId, vlanId, tableId, groupId, true,
                                       tableIdDbEntry, &portList);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vlan xpsAclUnbindIntfObjAcl error:%d\n", xpsRetVal);
        /*Revert bind configuration*/
        for (uint32_t i = 0; i < portList.size; i++)
        {
            portNum = portList.portIntf[i];
            cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

            portGroup = (cpssPortNum)/XPS_PORT_LIST_OFFSET;
            offset = (0x1)<<((cpssPortNum)%(XPS_PORT_LIST_OFFSET));
            if (tableIdDbEntry->portGroupBmp[portGroup])
            {
                tableIdDbEntry->portListBmp[portGroup] |= ((portGroup << XPS_PORT_LIST_OFFSET)|
                                                           offset);
            }
        }
        tableIdDbEntry->bindCount++;

        return xpsRetVal;
    }

    for (uint32_t i = 0; i < portList.size; i++)
    {
        portNum = portList.portIntf[i];
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
        portGroup = (cpssPortNum)/XPS_PORT_LIST_OFFSET;
        if (tableIdDbEntry->portGroupBmp[portGroup])
        {
            tableIdDbEntry->portGroupBmp[portGroup]--;
        }
    }
    return XP_NO_ERR;
}

XP_STATUS xpsAclGetCounterValueExt(xpDevice_t devId, uint32_t counterId,
                                   uint64_t *pktCnt, uint64_t *byteCnt)
{
    uint32_t                   packetCntrIndex, byteCntrIndex =0;
    uint32_t                   packetCntrBlockNum, byteCntrBlockNum = 0;
    GT_STATUS                  rc = GT_OK;
    CPSS_PP_FAMILY_TYPE_ENT    devType = cpssHalDevPPFamilyGet(devId);


    if (!pktCnt || !byteCnt)
    {
        return XP_ERR_INVALID_DATA;
    }

    if (devType == CPSS_PP_FAMILY_DXCH_AC3X_E ||
        devType == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
    {
        if ((rc = cpssHalCncCounterGet(devId, XPS_ACL_CNC_BLK_NUM_AC3X,
                                       counterId,
                                       CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E, pktCnt, byteCnt, NULL)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalCncCounterGet failed with err : %d\n ", rc);
            return xpsConvertCpssStatusToXPStatus(rc);

        }
    }
    else
    {
        packetCntrIndex = counterId;
        packetCntrIndex  %= XPS_CPSS_CNC_COUNTER_PER_BLK;

        byteCntrIndex = counterId;
        byteCntrIndex  %= XPS_CPSS_CNC_COUNTER_PER_BLK;

        packetCntrBlockNum =
            counterId/XPS_CPSS_CNC_COUNTER_PER_BLK;
        if (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
            devType == CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
            byteCntrBlockNum = packetCntrBlockNum + XPS_ACL_CNC_BLOCK_COUNT;
        }
        else if (devType == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            byteCntrBlockNum = packetCntrBlockNum + 1;
        }
        if ((rc = cpssHalCncCounterGet(devId, packetCntrBlockNum, packetCntrIndex,
                                       CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E, pktCnt, NULL, NULL)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalCncCounterGet failed with err : %d\n ", rc);
            return xpsConvertCpssStatusToXPStatus(rc);

        }

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              " fetching packet counter with counterId %d blockId: %d counterIndex : %d Cnt :%lu \n ",
              counterId, packetCntrBlockNum, packetCntrIndex,
              *pktCnt);

        if ((rc = cpssHalCncCounterGet(devId, byteCntrBlockNum, byteCntrIndex,
                                       CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E, NULL, byteCnt, NULL)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalCncCounterGet failed with err : %d\n ", rc);
            return xpsConvertCpssStatusToXPStatus(rc);

        }
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              " fetching BYTE counter with counterId %d blockId: %d counterIndex : %d Cnt :%lu \n",
              counterId, byteCntrBlockNum, byteCntrIndex,
              *byteCnt);
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclGetCounterValue(xpDevice_t devId, uint32_t counterIdx,
                                uint64_t *byteCount, uint64_t *packCount)
{
    XPS_LOCK(xpsAclGetCounterValue);
    XP_STATUS                  xpsRetVal = XP_NO_ERR;
    uint64_t                   pktCnt = 0, byteCnt = 0;
    xpsAclCounterIdMappingContextDbEntry_t  counterIdMapDbKey;
    xpsAclCounterIdMappingContextDbEntry_t  *counterIdMapDbEntry = NULL;

    counterIdMapDbKey.globalCounterId = counterIdx;

    if (((xpsRetVal = xpsStateSearchData(devId, aclCounterIdMapDbHndl,
                                         (xpsDbKey_t)&counterIdMapDbKey, (void**)&counterIdMapDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed for %d DB HNDL with err : %d \n ",
              aclCounterIdMapDbHndl, xpsRetVal);
        return xpsRetVal;
    }
    /* Counter DB is allocated only when ACL is deployed.*/
    if (counterIdMapDbEntry == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_WARNING,
              "No counter DB found. Check ACL is binded. err(%d)", xpsRetVal);
        *packCount = pktCnt;
        *byteCount = byteCnt;
        return XP_NO_ERR;
    }

    for (uint32_t i=0; i < counterIdMapDbEntry->numOfDuplicates; i++)
    {
        pktCnt = 0;
        byteCnt = 0;

        xpsRetVal = xpsAclGetCounterValueExt(devId,
                                             counterIdMapDbEntry->localCounterId[i], &pktCnt, &byteCnt);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " xpsAclGetCounterValueExt failed with err : %d\n ", xpsRetVal);
            return xpsRetVal;
        }

        *packCount += pktCnt;
        *byteCount += byteCnt;
    }

    return XP_NO_ERR;

}



XP_STATUS xpsAclDeleteTable(xpsDevice_t devId, uint32_t tableId)
{
    XPS_LOCK(xpsAclDeleteTable);
    XP_STATUS               ret = XP_NO_ERR;
    xpsAclTableContextDbEntry_t      tableIdDbKey;
    xpsAclTableContextDbEntry_t      *tableIdDbEntry=NULL;
    xpsScope_t              scopeId = 0;

    //get scope from Device ID
    if ((ret = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        return ret;
    }

    /* Retrieving table info */
    tableIdDbKey.tableId = tableId ;
    if (((ret = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                   (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) == XP_NO_ERR))
    {
        if (tableIdDbEntry == NULL)
        {
            return XP_ERR_KEY_NOT_FOUND;
        }
        if (tableIdDbEntry ->numEntries == 0)
        {
            ret = xpsStateDeleteData(scopeId, aclTableIdDbHndl, (xpsDbKey_t) &tableIdDbKey,
                                     (void **) &tableIdDbEntry);
            if (ret != XP_NO_ERR)
            {
                return ret;
            }

            if ((ret = xpsStateHeapFree((void*) tableIdDbEntry)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "XPS state Heap Free failed with err :%d \n", ret);
                return ret;
            }

            if ((ret = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_ACL_TABLE_ID,
                                             tableId)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " xpsAllocatorReleaseId failed with err : %d\n ", ret);
                return ret;
            }
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " Table is not empty. Failed to delete tableId: %d\n ", tableId);
            return ret;
        }
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Entry not found for tableId :%d\n", tableId);
        return XP_ERR_KEY_NOT_FOUND;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclDeleteEntry(xpsDevice_t devId, uint32_t tableId,
                            uint32_t ruleId, bool is_control_acl)
{

    XPS_LOCK(xpsAclDeleteEntry);

    GT_STATUS                  rc  = GT_OK;
    XP_STATUS                  xpsRetVal = XP_NO_ERR;
    xpsAclTableContextDbEntry_t      tableIdDbKey;
    xpsAclTableContextDbEntry_t      *tableIdDbEntry=NULL;
    xpsAclEntryContextDbEntry_t              tableEntryDbKey;
    xpsAclEntryContextDbEntry_t             *tableEntryDbEntry = NULL;
    xpsAclCounterIdMappingContextDbEntry_t  counterIdMapDbKey;
    xpsAclCounterIdMappingContextDbEntry_t  *counterIdMapDbEntry = NULL;
    xpsScope_t              scopeId = 0;
    uint32_t                            portGroupIndex;
    uint32_t                            newRuleId = 0;
    uint32_t                            vtcamId =0;
    CPSS_PP_FAMILY_TYPE_ENT    devType = cpssHalDevPPFamilyGet(devId);
    GT_BOOL valid = GT_FALSE;

    tableIdDbKey.tableId = tableId;
    if ((xpsRetVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                        (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    if (tableIdDbEntry != NULL)
    {
        if (tableIdDbEntry->tableId != 0)
        {
            /* Update hw entries using numEntries and rule Id from xpsAclTableContextDbEntry_t Db */
            for (uint32_t i =0; i< tableIdDbEntry->numEntries; i++)
            {
                if (tableIdDbEntry ->ruleId[i] == ruleId)
                {
                    /* Retrieving key fields and action info */
                    tableEntryDbKey.key = XPS_PCL_TABLE_ID_KEY(tableIdDbKey.tableId, ruleId);

                    if (is_control_acl)
                    {
                        if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableEntryDbHndl,
                                                             (xpsDbKey_t)&tableEntryDbKey, (void**)&tableEntryDbEntry)) != XP_NO_ERR))
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
                            return xpsRetVal;
                        }

                        if (tableEntryDbEntry == NULL)
                        {
                            xpsRetVal = XP_ERR_NULL_POINTER;
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Retrieving ACL table entry failed, devId(%d) err(%d)\n", devId, xpsRetVal);
                            return xpsRetVal;
                        }
                        if (tableEntryDbEntry->counterEnable)
                        {
                            /* Get counter Id from DB to free the entry */
                            /* Free counter DB */
                            counterIdMapDbKey.globalCounterId = tableEntryDbEntry->counterId;

                            if (((xpsRetVal = xpsStateSearchData(scopeId, aclCounterIdMapDbHndl,
                                                                 (xpsDbKey_t)&counterIdMapDbKey, (void**)&counterIdMapDbEntry)) != XP_NO_ERR))
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                      "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
                                return xpsRetVal;
                            }
                            if (counterIdMapDbEntry == NULL)
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_WARNING,
                                      "counter Db not found or Db already deleted with err : %d \n ", xpsRetVal);
                            }
                            else
                            {
                                counterIdMapDbEntry->numOfEntries -=1;
                            }
                        }
                        /* Delete the entry from HW */
                        /* Free table entry data from the table db */
                        /* Freeing duplication of rules for all the ports of different port groups sharing the same rule */
                        for (portGroupIndex = 0; portGroupIndex < XPS_PORT_LIST_PORT_GROUP_MAX;
                             portGroupIndex++)
                        {
                            /* Each newRuleId will contain PortGroupId as 4MSBs followed by ruleId in the rest 28 LSBs. */
                            newRuleId = ((portGroupIndex << 28) | (ruleId &
                                                                   0xFFFFFFF)); /* copying portGroupId and ruleId */

                            /* Deleting rule from hardware */
                            valid = GT_FALSE;
                            cpssDxChVirtualTcamRuleValidStatusGet(tableIdDbEntry->tcamMgrId,
                                                                  tableIdDbEntry->currentVtcamId,
                                                                  newRuleId, &valid);
                            if (valid &&
                                ((rc = cpssDxChVirtualTcamRuleDelete(tableIdDbEntry->tcamMgrId,
                                                                     tableIdDbEntry->currentVtcamId,
                                                                     newRuleId)) != GT_OK))
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                      " cpssDxChVirtualTcamRuleDelete  failed with err : %d n ", rc);
                                return xpsConvertCpssStatusToXPStatus(rc);
                            }

                        }

                        if (tableEntryDbEntry->counterEnable)
                        {
                            if (counterIdMapDbEntry != NULL)
                            {

                                if (!counterIdMapDbEntry->numOfEntries)
                                {
                                    /* Clear On Read is set for CNC.
                                       Read the existing cnc counter to clear it
                                       before freeing the resoure. */
                                    uint64_t byteCounterValue = 0;
                                    uint64_t pktCounterValue = 0;
                                    xpsRetVal = xpsAclGetCounterValue(devId,
                                                                      counterIdMapDbEntry->globalCounterId,
                                                                      &byteCounterValue,
                                                                      &pktCounterValue);
                                    if (xpsRetVal != XP_NO_ERR)
                                    {
                                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                              "xpsAclGetCounterValue:error %d\n", xpsRetVal);
                                        return (xpsRetVal);
                                    }

                                    //release the local counter Ids for parrallel lookup
                                    //remove the db maintained for storing local counter Ids
                                    for (uint32_t j=0; j< counterIdMapDbEntry->numOfDuplicates; j++)
                                    {

                                        uint32_t localCounterId = counterIdMapDbEntry->localCounterId[j];

                                        xpsRetVal = xpsAclRemoveCounter(devId, scopeId, devType,
                                                                        tableIdDbEntry, localCounterId);
                                        if (xpsRetVal != XP_NO_ERR)
                                        {
                                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                                  " xpsAclRemoveCounter failed with err : %d\n ", xpsRetVal);
                                            return xpsRetVal;
                                        }
                                        counterIdMapDbEntry->localCounterId[j] = DEFAULT_NULL_VALUE;
                                        counterIdMapDbEntry->parallelLookupNum[j] = DEFAULT_NULL_VALUE;
                                    }
                                }

                                /* Deleting the Acl Counter Db */
                                if ((xpsRetVal = xpsStateDeleteData(scopeId, aclCounterIdMapDbHndl,
                                                                    (xpsDbKey_t)&counterIdMapDbKey, (void**)&counterIdMapDbEntry)) != XP_NO_ERR)
                                {
                                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                          "Delete ACL counter entry failed, devId(%d)", devId);
                                    return xpsRetVal;
                                }

                                if ((xpsRetVal = xpsStateHeapFree((void*) counterIdMapDbEntry)) != XP_NO_ERR)
                                {
                                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                          "XPS state Heap Free failed with err :%d \n", xpsRetVal);
                                    return xpsRetVal;
                                }

                            }
                        }
                        uint32_t j =0;
                        for (j=i; j<tableIdDbEntry->numEntries-1; j++)
                        {
                            tableIdDbEntry->ruleId[j]=tableIdDbEntry->ruleId[j+1];
                        }

                        tableIdDbEntry->ruleId[j] = DEFAULT_NULL_VALUE;
                        tableIdDbEntry->numEntries--;

                        if (((xpsRetVal = xpsStateDeleteData(scopeId, aclTableEntryDbHndl,
                                                             (xpsDbKey_t)&tableEntryDbKey, (void**)&tableEntryDbEntry)) != XP_NO_ERR))
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
                            return xpsRetVal;
                        }

                        if ((xpsRetVal = xpsStateHeapFree((void*) tableEntryDbEntry)) != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "XPS state Heap Free failed with err :%d \n", xpsRetVal);
                            return xpsRetVal;
                        }

                        break;
                    }
                    else /* User ACL */
                    {
                        bool hasInPortsFld = false;
                        if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableEntryDbHndl,
                                                             (xpsDbKey_t)&tableEntryDbKey, (void**)&tableEntryDbEntry)) != XP_NO_ERR))
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
                            return xpsRetVal;
                        }

                        if (tableEntryDbEntry == NULL)
                        {
                            xpsRetVal = XP_ERR_NULL_POINTER;
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Retrieving ACL table entry failed, devId(%d) err(%d)\n", devId, xpsRetVal);
                            return xpsRetVal;
                        }


                        if (tableEntryDbEntry->counterEnable)
                        {
                            /* Get counter Id from DB to free the entry */
                            /* Free counter DB */
                            counterIdMapDbKey.globalCounterId = tableEntryDbEntry->counterId;

                            if (((xpsRetVal = xpsStateSearchData(scopeId, aclCounterIdMapDbHndl,
                                                                 (xpsDbKey_t)&counterIdMapDbKey, (void**)&counterIdMapDbEntry)) != XP_NO_ERR))
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                      "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
                                return xpsRetVal;
                            }
                            if (counterIdMapDbEntry  == NULL)
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_WARNING,
                                      "counter Db not found or Db already deleted with err : %d \n ", xpsRetVal);
                            }
                            else
                            {
                                counterIdMapDbEntry->numOfEntries -=1;
                            }
                        }
                        /* Delete the entry from HW */
                        /* Free table entry data from the table db */
                        /* Freeing duplication of rules for all the ports of different port groups sharing the same rule */
                        for (portGroupIndex = 0; portGroupIndex < XPS_PORT_LIST_PORT_GROUP_MAX;
                             portGroupIndex++)
                        {
                            if (tableIdDbEntry->portGroupBmp[portGroupIndex] ||
                                tableEntryDbEntry->portGroupBmp[portGroupIndex])
                            {
                                hasInPortsFld = true;
                                /* Each newRuleId will contain PortGroupId as 4MSBs followed by ruleId in the rest 28 LSBs. */
                                newRuleId = ((portGroupIndex << 28) | (ruleId &
                                                                       0xFFFFFFF)); /* copying portGroupId and ruleId */


                                for (uint32_t j=0; j<tableIdDbEntry->numOfDuplicates; j++)
                                {
                                    if ((tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)&&
                                        (tableIdDbEntry->isMirror))
                                    {
                                        if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
                                        {
                                            vtcamId = XPS_L3V6_MIRROR_ING_STG_TABLE_ID_0;
                                        }
                                        else
                                        {
                                            vtcamId = XPS_L3_MIRROR_ING_STG_TABLE_ID_0;
                                        }

                                    }
                                    else
                                    {
                                        vtcamId = tableIdDbEntry->vtcamId[j];
                                    }
                                    /* Deleting rule from hardware */
                                    valid = GT_FALSE;
                                    cpssDxChVirtualTcamRuleValidStatusGet(tableIdDbEntry->tcamMgrId, vtcamId,
                                                                          newRuleId, &valid);
                                    if (valid &&
                                        ((rc = cpssDxChVirtualTcamRuleDelete(tableIdDbEntry->tcamMgrId, vtcamId,
                                                                             newRuleId)) != GT_OK))
                                    {
                                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                              " cpssDxChVirtualTcamRuleDelete  failed with err : %d n ", rc);
                                        return xpsConvertCpssStatusToXPStatus(rc);
                                    }
                                    if ((tableIdDbEntry->stage == XPS_PCL_STAGE_INGRESS)&&
                                        (tableIdDbEntry->isMirror))
                                    {
                                        break;
                                    }
                                }
                                /* acl entries which don`t bound to others and in_ports/out_ports/in_port/out_port in match fileds*/
                                if (!tableIdDbEntry->numOfDuplicates)
                                {
                                    xpsRetVal = xpsAclGetVtcamIdByKeyFormat(tableIdDbEntry,
                                                                            tableEntryDbEntry->keyFormat, &vtcamId);
                                    if (xpsRetVal != XP_NO_ERR)
                                    {
                                        return xpsRetVal;
                                    }
                                    valid = GT_FALSE;
                                    cpssDxChVirtualTcamRuleValidStatusGet(tableIdDbEntry->tcamMgrId, vtcamId,
                                                                          newRuleId, &valid);
                                    if (valid &&
                                        ((rc = cpssDxChVirtualTcamRuleDelete(tableIdDbEntry->tcamMgrId, vtcamId,
                                                                             newRuleId)) != GT_OK))
                                    {
                                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                              " cpssDxChVirtualTcamRuleDelete  failed with err : %d n ", rc);
                                        return xpsConvertCpssStatusToXPStatus(rc);
                                    }
                                }
                            }
                        }
                        /* acl entries which don`t bound to others and only outer_vlan_id in match fileds*/
                        if (!hasInPortsFld)
                        {
                            newRuleId = ruleId & 0xFFFFFFF;
                            xpsRetVal = xpsAclGetVtcamIdByKeyFormat(tableIdDbEntry,
                                                                    tableEntryDbEntry->keyFormat, &vtcamId);
                            if (xpsRetVal != XP_NO_ERR)
                            {
                                return xpsRetVal;
                            }
                            valid = GT_FALSE;
                            cpssDxChVirtualTcamRuleValidStatusGet(tableIdDbEntry->tcamMgrId, vtcamId,
                                                                  newRuleId, &valid);
                            if (valid &&
                                ((rc = cpssDxChVirtualTcamRuleDelete(tableIdDbEntry->tcamMgrId, vtcamId,
                                                                     newRuleId)) != GT_OK))
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                      " cpssDxChVirtualTcamRuleDelete  failed with err : %d n ", rc);
                                return xpsConvertCpssStatusToXPStatus(rc);
                            }
                        }
                        if (tableEntryDbEntry->counterEnable)
                        {
                            if (counterIdMapDbEntry != NULL)
                            {
                                if (!counterIdMapDbEntry->numOfEntries)
                                {
                                    /* Clear On Read is set for CNC.
                                       Read the existing cnc counter to clear it
                                       before freeing the resoure. */
                                    uint64_t byteCounterValue = 0;
                                    uint64_t pktCounterValue = 0;

                                    xpsRetVal = xpsAclGetCounterValue(devId,
                                                                      counterIdMapDbEntry->globalCounterId,
                                                                      &byteCounterValue,
                                                                      &pktCounterValue);
                                    if (xpsRetVal != XP_NO_ERR)
                                    {
                                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                              "xpsAclGetCounterValue:error %d\n", xpsRetVal);
                                        return (xpsRetVal);
                                    }

                                    //release the local counter Ids for parrallel lookup
                                    //remove the db maintained for storing local counter Ids
                                    for (uint32_t j=0; j< counterIdMapDbEntry->numOfDuplicates; j++)
                                    {

                                        uint32_t localCounterId = counterIdMapDbEntry->localCounterId[j];
                                        xpsRetVal = xpsAclRemoveCounter(devId, scopeId, devType,
                                                                        tableIdDbEntry, localCounterId);
                                        if (xpsRetVal != XP_NO_ERR)
                                        {
                                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                                  " xpsAclRemoveCounter failed with err : %d\n ", xpsRetVal);
                                            return xpsRetVal;
                                        }
                                        counterIdMapDbEntry->localCounterId[j] = DEFAULT_NULL_VALUE;
                                        counterIdMapDbEntry->parallelLookupNum[j] = DEFAULT_NULL_VALUE;
                                    }
                                }

                                /* Deleting the Acl Counter Db */
                                if ((xpsRetVal = xpsStateDeleteData(scopeId, aclCounterIdMapDbHndl,
                                                                    (xpsDbKey_t)&counterIdMapDbKey, (void**)&counterIdMapDbEntry)) != XP_NO_ERR)
                                {
                                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                          "Delete ACL counter entry failed, devId(%d)", devId);
                                    return xpsRetVal;
                                }

                                if ((xpsRetVal = xpsStateHeapFree((void*) counterIdMapDbEntry)) != XP_NO_ERR)
                                {
                                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                          "XPS state Heap Free failed with err :%d \n", xpsRetVal);
                                    return xpsRetVal;
                                }

                            }
                        }
                        uint32_t j =0;
                        for (j=i; j<tableIdDbEntry ->numEntries-1; j++)
                        {
                            tableIdDbEntry ->ruleId[j]=tableIdDbEntry ->ruleId[j+1];
                        }

                        tableIdDbEntry ->ruleId[j] = DEFAULT_NULL_VALUE;
                        tableIdDbEntry ->numEntries--;
                        /* TBD - Freeing is causing issue in testbed for egress acl testsuite */
#if 0
                        xpFree(tableEntryDbEntry->rule.fldList->value);
                        xpFree(tableEntryDbEntry->rule.fldList->mask);
                        xpFree(tableEntryDbEntry->rule.fldList);
#endif
                        /* Deleting the Acl Entry Db */
                        if (((xpsRetVal = xpsStateDeleteData(scopeId, aclTableEntryDbHndl,
                                                             (xpsDbKey_t)&tableEntryDbKey, (void**)&tableEntryDbEntry)) != XP_NO_ERR))
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "xpsStateDeleteData failed with err : %d for rule Id: %d \n ", xpsRetVal,
                                  ruleId);
                            return xpsRetVal;
                        }

                        if (tableEntryDbEntry != NULL)
                        {
                            if ((xpsRetVal = xpsStateHeapFree((void*) tableEntryDbEntry)) != XP_NO_ERR)
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                      "XPS state Heap Free failed with err :%d \n", xpsRetVal);
                                return xpsRetVal;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    return XP_NO_ERR;

}

XP_STATUS xpsAclPortUnbind(xpsDevice_t devId, uint32_t portId,
                           uint32_t groupId, uint32_t tableId)
{
    XP_STATUS                               xpsRetVal = XP_NO_ERR;
    uint32_t                                portGroup;
    uint32_t                                offset;
    xpsAclTableContextDbEntry_t             tableIdDbKey;
    xpsAclTableContextDbEntry_t             *tableIdDbEntry = NULL;
    xpsScope_t                              scopeId = 0;
    uint32_t                                portNum = 0;
    xpsLagPortIntfList_t                    portList;
    memset(&portList, 0, sizeof(xpsLagPortIntfList_t));

    portNum = xpsGlobalPortToPortnum(devId, portId);
    if (portNum == 0xFFFF)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid port Num : %d \n ", portId);
        return XP_ERR_INVALID_VALUE;
    }

    /* Get scope from Device ID */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        return xpsRetVal;
    }

    /* Retrieving table info */
    tableIdDbKey.tableId = tableId ;
    if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                         (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }
    if (tableIdDbEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    //decrease the bindcount per table
    if (tableIdDbEntry->bindCount)
    {
        tableIdDbEntry->bindCount--;
    }
    //remove the unbound port from the portlist and update the rules in hw
    portGroup = (portNum)/XPS_PORT_LIST_OFFSET;
    offset = (0x1)<<((portNum)%(XPS_PORT_LIST_OFFSET));

    if (portGroup >= XPS_PORT_LIST_PORT_GROUP_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid port Num : %d \n ", portNum);
        return XP_ERR_INVALID_VALUE;
    }

    if (tableIdDbEntry->portGroupBmp[portGroup])
    {
        tableIdDbEntry->portListBmp[portGroup] &= (~offset);
    }
    portList.size = 1;
    portList.portIntf[0] = portId;

    xpsRetVal = xpsAclUnbindIntfObjAcl(devId, portId, tableId, groupId, false,
                                       tableIdDbEntry, &portList);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Port xpsAclUnbindIntfObjAcl error:%d\n", xpsRetVal);
        /*Revert bind configuration*/
        if (tableIdDbEntry->portGroupBmp[portGroup])
        {
            tableIdDbEntry->portListBmp[portGroup] |= ((portGroup << XPS_PORT_LIST_OFFSET)|
                                                       offset);
        }
        tableIdDbEntry->bindCount++;
        return xpsRetVal;
    }

    if (tableIdDbEntry->portGroupBmp[portGroup])
    {
        tableIdDbEntry->portGroupBmp[portGroup]--;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclUpdateMirrorTableVtcamMap(xpsDevice_t devId, uint32_t tableId)
{
    XP_STATUS                               xpsRetVal = XP_NO_ERR;
    xpsAclTableVtcamMapContextDbEntry_t     tableVtcamMapDbKey;
    xpsAclTableVtcamMapContextDbEntry_t     *tableVtcamMapDbEntry = NULL;
    xpsAclTableContextDbEntry_t             tableIdDbKey;
    xpsAclTableContextDbEntry_t             *tableIdDbEntry = NULL;
    xpsScope_t                              scopeId = 0;

    /* Get scope from Device ID */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        return xpsRetVal;
    }

    tableIdDbKey.tableId = tableId ;
    if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                         (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    if (tableIdDbEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }
    /*Note this flow is valid for MIRROR and INGRESS stage*/
    if (tableIdDbEntry->stage != XPS_PCL_STAGE_INGRESS)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid Stage to handle : %d \n ", tableIdDbEntry->stage);
        return XP_ERR_INVALID_FIELD_REQ;
    }

    if ((xpsRetVal = xpsStateGetNextData(scopeId, aclTableVtcamMapDbHndl, NULL,
                                         (void**)&tableVtcamMapDbEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Getting first node from db failed  err: %d \n", xpsRetVal);
        return xpsRetVal;
    }
    while (tableVtcamMapDbEntry != NULL)
    {
        for (uint32_t count =0; count< tableVtcamMapDbEntry->tableCount; count++)
        {
            if (tableVtcamMapDbEntry->tableId[count] == tableId)
            {
                //Removing from corresponding vtcam region mapping for ingress: mirror rules to 2nd lookup only for IACL
                for (uint32_t i=0; i< tableIdDbEntry->numOfDuplicates; i++)
                {
                    if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
                    {
                        if (!tableIdDbEntry->isMirror)
                        {
                            tableVtcamMapDbEntry->vTcamRegionIngressL3V6[tableIdDbEntry->vtcamId[i] -
                                                                                                    XPS_ING_STG_V6_USER_ACL_TABLE_START]++;
                        }
                        else
                        {
                            if ((tableVtcamMapDbEntry->vTcamRegionIngressL3V6[tableIdDbEntry->vtcamId[i] -
                                                                                                         XPS_ING_STG_V6_USER_ACL_TABLE_START]) > 0)
                            {
                                tableVtcamMapDbEntry->vTcamRegionIngressL3V6[tableIdDbEntry->vtcamId[i] -
                                                                                                        XPS_ING_STG_V6_USER_ACL_TABLE_START]--;
                            }

                        }
                    }
                    else if ((tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV4) ||
                             (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_NON_IP))
                    {
                        if (!tableIdDbEntry->isMirror)
                        {
                            tableVtcamMapDbEntry->vTcamRegionIngressL3[tableIdDbEntry->vtcamId[i] -
                                                                                                  XPS_ING_STG_V4_USER_ACL_TABLE_START]++;
                        }
                        else
                        {
                            if ((tableVtcamMapDbEntry->vTcamRegionIngressL3[tableIdDbEntry->vtcamId[i] -
                                                                                                       XPS_ING_STG_V4_USER_ACL_TABLE_START]) > 0)
                            {
                                tableVtcamMapDbEntry->vTcamRegionIngressL3[tableIdDbEntry->vtcamId[i] -
                                                                                                      XPS_ING_STG_V4_USER_ACL_TABLE_START]--;
                            }
                        }
                    }
                }
            }
        }

        tableVtcamMapDbKey.groupId = tableVtcamMapDbEntry->groupId;

        if ((xpsRetVal = xpsStateGetNextData(scopeId, aclTableVtcamMapDbHndl,
                                             (xpsDbKey_t)&tableVtcamMapDbKey, (void**)&tableVtcamMapDbEntry)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get next vtcamMap Entry from list failed  err: %d \n", xpsRetVal);
            return xpsRetVal;
        }

    }
    return XP_NO_ERR;
}

XP_STATUS xpsAclDeleteAclEntries(xpsScope_t scopeId)
{

    XP_STATUS xpsRetVal = XP_NO_ERR;
    bool is_control_acl = false;
    uint32_t                        devId = 0;
    uint32_t                        tableId = 0, ruleId = 0;
    xpsAclTableContextDbEntry_t             tableIdDbKey;
    xpsAclTableContextDbEntry_t             *tableIdDbEntry = NULL;
    xpsAclTableVtcamMapContextDbEntry_t     tableVtcamMapDbKey;
    xpsAclTableVtcamMapContextDbEntry_t     *tableVtcamMapDbEntry = NULL;
    xpsAclEntryContextDbEntry_t              tableEntryDbKey;
    xpsAclEntryContextDbEntry_t             *tableEntryDbEntry = NULL;
    xpsAclCounterIdMappingContextDbEntry_t  counterIdMapDbKey;
    xpsAclCounterIdMappingContextDbEntry_t  *counterIdMapDbEntry = NULL;

    memset(&tableIdDbKey, 0x0, sizeof(xpsAclTableContextDbEntry_t));
    memset(&tableVtcamMapDbKey, 0x0, sizeof(xpsAclTableVtcamMapContextDbEntry_t));
    memset(&tableEntryDbKey, 0x0, sizeof(xpsAclEntryContextDbEntry_t));
    memset(&counterIdMapDbKey, 0x0, sizeof(xpsAclCounterIdMappingContextDbEntry_t));

    /* Deleting all from hw */

    /* Loop through all the xpsAclTableContextDbEntry_t in RB tree */

    //Get the first tree node for aclTableIdDbHndlin-order traversal with NULL key
    xpsRetVal=  xpsStateGetNextData(scopeId, aclTableIdDbHndl, NULL,
                                    (void * *)&tableIdDbEntry);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get Data failed");
        return xpsRetVal;
    }

    while (tableIdDbEntry != NULL)
    {
        is_control_acl = (tableIdDbEntry->tableId <= XPS_MAX_CTRL_TBL_IDS) ? true :
                         false;

        tableId = tableIdDbEntry->tableId;

        /* Delete all the entries of this table */
        for (uint32_t i =0; i< tableIdDbEntry->numEntries; i++)
        {

            ruleId = tableIdDbEntry->ruleId[i];

            /* Retrieving key fields and action info */

            tableEntryDbKey.key = XPS_PCL_TABLE_ID_KEY(tableId, ruleId);

            if (((xpsRetVal = xpsStateSearchData(scopeId, aclTableEntryDbHndl,
                                                 (xpsDbKey_t)&tableEntryDbKey, (void**)&tableEntryDbEntry)) != XP_NO_ERR))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
                return xpsRetVal;
            }

            if (tableEntryDbEntry == NULL)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Retrieving ACL table entry failed, devId(%d)", devId);
                return XP_ERR_KEY_NOT_FOUND;
            }

            if ((xpsRetVal = xpsAclDeleteEntry(devId, tableId, ruleId,
                                               is_control_acl)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to delete acl entry with devId : %d, tableId : %d, ruleId : %d, err: %d \n",
                      devId, tableId, ruleId, xpsRetVal);
                return xpsRetVal;
            }
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "ACL entry successfully deleted with tableId : %d, ruleId : %d \n", tableId,
                  ruleId);
            if (&(tableIdDbEntry->ruleId[i]) != NULL)
            {
                if ((xpsRetVal = xpsStateHeapFree((void*) &(tableIdDbEntry->ruleId[i]))) !=
                    XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "XPS state Heap Free failed with err :%d \n", xpsRetVal);
                    return xpsRetVal;
                }
            }
        }

        tableIdDbKey.tableId = tableIdDbEntry->tableId;

        /* Deleting the table */
        if ((xpsRetVal = xpsAclDeleteTable(devId, tableId)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to delete table with devId : %d, tableId : %d err: %d \n",
                  devId, tableId, xpsRetVal);
            return xpsRetVal;
        }
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "ACL table successfully deleted with tableId : %d \n", tableId);

        //get-next node
        xpsRetVal=  xpsStateGetNextData(scopeId, aclTableIdDbHndl,
                                        (xpsDbKey_t)&tableIdDbKey, (void  **)&tableIdDbEntry);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get next node failed");
            return xpsRetVal;
        }
    }

    /* Deleting group info of the table */
    if ((xpsRetVal = xpsStateGetNextData(scopeId, aclTableVtcamMapDbHndl, NULL,
                                         (void**)&tableVtcamMapDbEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Getting first node from db failed  err: %d \n", xpsRetVal);
        return xpsRetVal;
    }

    while (tableVtcamMapDbEntry != NULL)
    {
        tableVtcamMapDbKey.groupId = tableVtcamMapDbEntry->groupId;

        /*Deleting Vtcam Id mapping for per group Db*/
        if (((xpsRetVal = xpsStateDeleteData(scopeId, aclTableVtcamMapDbHndl,
                                             (xpsDbKey_t)&tableVtcamMapDbKey, (void**)&tableVtcamMapDbEntry)) != XP_NO_ERR))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsStateDeleteData failed with err : %d  for group id : %d\n ", xpsRetVal,
                  tableVtcamMapDbEntry->groupId);
            return xpsRetVal;
        }

        if ((xpsRetVal = xpsStateHeapFree((void*) tableVtcamMapDbEntry)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "XPS state Heap Free failed with err :%d \n", xpsRetVal);
            return xpsRetVal;
        }
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "vtcam map successfully deleted with groupId : %d \n",
              tableVtcamMapDbKey.groupId);

        if ((xpsRetVal = xpsStateGetNextData(scopeId, aclTableVtcamMapDbHndl,
                                             (xpsDbKey_t)&tableVtcamMapDbKey, (void**)&tableVtcamMapDbEntry)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get next vtcamMap Entry from list failed  err: %d \n", xpsRetVal);
            return xpsRetVal;
        }

    }

    /* Deleting counter info of the tables */
    if ((xpsRetVal = xpsStateGetNextData(scopeId, aclCounterIdMapDbHndl, NULL,
                                         (void**)&counterIdMapDbEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Getting first node from db failed  err: %d \n", xpsRetVal);
        return xpsRetVal;
    }
    while (counterIdMapDbEntry != NULL)
    {
        counterIdMapDbKey.globalCounterId = counterIdMapDbEntry->globalCounterId;

        /*Deleting Vtcam Id mapping for per group Db*/
        if (((xpsRetVal = xpsStateDeleteData(scopeId, aclCounterIdMapDbHndl,
                                             (xpsDbKey_t)&counterIdMapDbKey, (void**)&counterIdMapDbEntry)) != XP_NO_ERR))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsStateDeleteData failed with err : %d  for counter id : %d\n ", xpsRetVal,
                  counterIdMapDbEntry->globalCounterId);
            return xpsRetVal;
        }

        if ((xpsRetVal = xpsStateHeapFree((void*) counterIdMapDbEntry)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "XPS state Heap Free failed with err :%d \n", xpsRetVal);
            return xpsRetVal;
        }

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "counter DB successfully deleted with globalCounterId : %d \n",
              counterIdMapDbKey.globalCounterId);

        if ((xpsRetVal = xpsStateGetNextData(scopeId, aclCounterIdMapDbHndl,
                                             (xpsDbKey_t)&counterIdMapDbKey, (void**)&counterIdMapDbEntry)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get next counterIdMap Entry from list failed  err: %d \n", xpsRetVal);
            return xpsRetVal;
        }

    }

    return xpsRetVal;
}

XP_STATUS xpsAclDeInitScope(xpsScope_t scopeId)
{

    XP_STATUS ret = XP_NO_ERR;

    ret = xpsAclDeleteAclEntries(scopeId);
    if (ret)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Failed to delete acl entries from hw with err : %d\n ", ret);
        return ret;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
          "ACL entries successfully deleted \n");

    ret = xpsStateDeRegisterDb(scopeId, &aclTableIdDbHndl);
    if (ret)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Failed to deregister aclTableIdDbHndl with err : %d\n ", ret);
        return ret;
    }

    ret = xpsStateDeRegisterDb(scopeId, &aclTableEntryDbHndl);
    if (ret)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Failed to deregister aclTableEntryDbHndl with err : %d\n ", ret);
        return ret;
    }

    ret = xpsStateDeRegisterDb(scopeId, &aclTableVtcamMapDbHndl);
    if (ret)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Failed to deregister aclTableVtcamMapDbHndl with err : %d\n ", ret);
        return ret;
    }

    ret = xpsStateDeRegisterDb(scopeId, &aclCounterIdMapDbHndl);
    if (ret)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Failed to deregister aclCounterIdMapDbHndl with err : %d\n ", ret);
        return ret;
    }

    ret = xpsAclTableDeInitScope(scopeId);
    if (ret)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Failed to deinit scope with err : %d\n ", ret);
        return ret;
    }

    xpsLockDestroy(XP_LOCKINDEX_XPS_IACL);

    return XP_NO_ERR;

}

extern uint32_t ctrlPacketPFCAclTableId;  /* falcon */

XP_STATUS xpsAclPFCRuleUpdate(xpsDevice_t devId, uint32_t portNum, bool enable)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsAclTableContextDbEntry_t             tableIdDbKey;
    xpsAclTableContextDbEntry_t             *tableIdDbEntry = NULL;
    uint32_t                                portGroup;
    uint32_t                                offset;
    GT_STATUS cpssRet = GT_OK;
    static uint32_t ruleId = 0;
    uint32_t     cpssPortNum;
    xpsScope_t                              scopeId = 0;

    if ((retVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        return retVal;
    }
    if (cpssHalDevPPFamilyGet(devId) != CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        return XP_NO_ERR;
    }

    tableIdDbKey.tableId = ctrlPacketPFCAclTableId;
    if (((retVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                      (xpsDbKey_t)&tableIdDbKey,
                                      (void**)&tableIdDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", retVal);
        return retVal;
    }

    if (tableIdDbEntry == NULL)
    {
        retVal = XP_ERR_NULL_POINTER;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", retVal);
        return retVal;
    }

    tableIdDbEntry->vtcamId[0] = XPS_L3_ING_STG_TABLE_ID_0;
    tableIdDbEntry->numOfDuplicates = 1;

    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    portGroup = (cpssPortNum)/XPS_PORT_LIST_OFFSET;
    offset = (0x1)<<((cpssPortNum)%(XPS_PORT_LIST_OFFSET));
    //if adding a port to a portlist
    if (enable)
    {
        tableIdDbEntry->portGroupBmp[portGroup]++;
        tableIdDbEntry->portListBmp[portGroup] |= ((portGroup << XPS_PORT_LIST_OFFSET)|
                                                   offset);
        tableIdDbEntry->bindCount++;
    }
    //if removing a port from port list
    else
    {
        tableIdDbEntry->portListBmp[portGroup] &= (~offset);
    }

    /* Even though this rule is for PFC ctrl packet, we need to manage the rule per
       port. ACL portbitmap logic is modeled under User ACL. Hence allocate ruleId
       from User Range..*/
    if (!ruleId)
    {
        retVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_ENTRY,
                                        &ruleId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Could not allocate ACL ruleId id, error %d\n", retVal);
            return retVal;
        }
    }

    uint16_t opCode = 0x0101;
    cpssRet = cpssHalCtrlPktFlowControl(devId, ctrlPacketPFCAclTableId, opCode,
                                        CPSS_PACKET_CMD_DROP_HARD_E,
                                        (cpssHalCtrlPktAclPriority_e)ruleId,
                                        false);

    if (cpssRet != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalCtrlPktFlowControl failed %d\n", cpssRet);
        return xpsConvertCpssStatusToXPStatus(cpssRet);
    }
    if (enable == false)
    {
        tableIdDbEntry->portGroupBmp[portGroup]--;
        tableIdDbEntry->bindCount--;
    }

    return retVal;
}


/*This function is used to make rule valid/Invalid*/
XP_STATUS xpsAclSetRuleValid(xpsDevice_t devId, uint32_t tableId,
                             uint32_t index, uint8_t valid)
{
    return XP_NO_ERR;
}

XP_STATUS xpsAclDeleteL4PortRange(xpsDevice_t devId)
{
    return XP_NO_ERR;
}

XP_STATUS xpsAclGetAvailableEntries(xpsDevice_t devId,
                                    uint32_t tableIdNum, uint32_t *availableEntries)
{

    GT_STATUS cpssRet                           = GT_OK;
    XP_STATUS ret                               = XP_NO_ERR;
    uint32_t usedEntries                        = 0;
    uint32_t maxAclEntries                      = 0;
    uint32_t ruleSize                           = 0;
    GT_U32 tcamMgrId                            = 0;
    GT_U32 vTcamId                              = 0;
    xpsAclTableContextDbEntry_t tableIdDbKey;
    xpsAclTableContextDbEntry_t *tableIdDbEntry = NULL;
    xpCfgTblEntryCountStruct    tblEntryCount;
    xpsScope_t                              scopeId = 0;

    if ((ret = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        return ret;
    }

    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC tcamUsage;

    for (vTcamId = XPS_MIN_VTCAM_IDS; vTcamId <= XPS_MAX_VTCAM_IDS; vTcamId++)
    {
        if ((cpssRet = cpssDxChVirtualTcamUsageGet(tcamMgrId, vTcamId,
                                                   &tcamUsage)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChVirtualTcamUsageGet  failed with err : \n ", cpssRet);
            return xpsConvertCpssStatusToXPStatus(cpssRet);
        }
        if ((vTcamId == XPS_L3_CTRL_TABLE_ID) ||
            (vTcamId >= XPS_L3_ING_STG_TABLE_ID_0 &&
             vTcamId <= XPS_L3_ING_STG_TABLE_ID_3) ||
            (vTcamId >= XPS_L3_EGR_STG_TABLE_ID_0 &&
             vTcamId <= XPS_L3_EGR_STG_TABLE_ID_3) ||
            (vTcamId == XPS_L3_MIRROR_ING_STG_TABLE_ID_0))
        {
            usedEntries += tcamUsage.rulesUsed;
        }
        else
        {
            usedEntries += (tcamUsage.rulesUsed*2);
        }
    }
    /* Retrieving table info */
    tableIdDbKey.tableId = tableIdNum ;
    if (((ret = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                   (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Table %d not found in search data.\n", tableIdNum);
        *availableEntries =  0;
        return ret;
    }
    if (tableIdDbEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    /* Get max ACL count */
    memset(&tblEntryCount, 0, sizeof(xpCfgTblEntryCountStruct));
    if ((ret = xpsCpssCfgTableNumEntriesGet(devId, &tblEntryCount)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Cpss Table Entries Count Get failed %d\n", ret);
        *availableEntries =  0;
        return ret;
    }

    ruleSize = XPS_VIRTUAL_TCAM_RULE_SIZE_30_B_E
               +1; //Currently 30 byte rule size is used for each entry.

    if (usedEntries <= tblEntryCount.aclEntries)
    {
        if (tableIdDbEntry->tableType == XPS_ACL_TABLE_TYPE_IPV6)
        {
            usedEntries = usedEntries/2;
            maxAclEntries = tblEntryCount.aclEntries/
                            (ruleSize*2); //For Ipv6, 1 entry size is twice of Ipv4
            *availableEntries = maxAclEntries - usedEntries;
        }
        else
        {
            maxAclEntries = tblEntryCount.aclEntries/ruleSize;
            *availableEntries = maxAclEntries - usedEntries;
        }
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Used count more than maximum count.\n");
        *availableEntries = 0;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAclReadEntry(xpsDevice_t devId, uint32_t tableId,
                          uint32_t indexOrPri, xpsVirtualTcamRuleData_t *ruleData,
                          xpsAclKeyFormat keyFormat)
{
    return XP_NO_ERR;
}

XP_STATUS xpsAclCopyEntry(xpDevice_t devId, uint32_t tableId,
                          uint32_t indexSrc, uint32_t indexDst)
{
    return XP_NO_ERR;
}

XP_STATUS xpsAclSetPolicingEnable(xpsDevice_t devId, uint32_t enable)
{
    return XP_NO_ERR;
}

XP_STATUS xpsAclEnableLookup(xpsDevice_t devId, bool ingressEn,
                             bool aclEnLookup)
{
    return XP_NO_ERR;
}

XP_STATUS xpsAclSetRuleInbandMgmt
(
    uint32_t devId, uint32_t port
)
{
    return cpssHalCtrlPktInbandMgmt(devId, port);
}

extern uint32_t egressErspanIpv4AclTableId;  /* falcon */
extern uint32_t egressErspanIpv6AclTableId;  /* falcon */
extern uint32_t egressErspanNonIpAclTableId;  /* falcon */

/* In pipelin, for Ingress Mirror, below rule will have port-bitmap
  key set for mirror port and for egress mirror it will be actual
  target phy port. Hence for egress mirror, add the corresponding
  egress target ports to this rule. */

XP_STATUS xpsAclErspanRuleUpdate(xpsDevice_t devId, uint32_t portId,
                                 bool enable, uint32_t analyzerIndex)
{
    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsAclTableContextDbEntry_t  tableIdDbKey;
    xpsAclTableContextDbEntry_t *tableIdDbEntry = NULL;
    uint32_t                     portGroup;
    uint32_t                     offset;
    uint32_t                     portNum;
    uint32_t                     tableId;
    uint32_t                     vtcamId;
    bool                         isIpv4;
    xpsAclKeyFormat              keyFormat;
    uint8_t                      type = XPS_ACL_TABLE_TYPE_IPV4;
    xpsMirrorType_e              mirrorType;
    uint32_t                     ePort = 0;
    CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC policerTrig;
    GT_BOOL valid = GT_TRUE;
    xpsScope_t                              scopeId = 0;

    memset(&policerTrig, 0, sizeof(policerTrig));

    if ((retVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        return retVal;
    }
    if ((cpssHalDevPPFamilyGet(devId) != CPSS_PP_FAMILY_DXCH_FALCON_E) &&
        (cpssHalDevPPFamilyGet(devId) != CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        return XP_NO_ERR;
    }

    portNum = xpsGlobalPortToPortnum(devId, portId);
    if (portNum == 0xFFFF)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid port Num : %d \n ", portId);
        return XP_ERR_INVALID_VALUE;
    }

    retVal = xpsMirrorErspanSessionTypeGet(devId, analyzerIndex, &isIpv4,
                                           &mirrorType, &ePort);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsMirrorErspanSessionTypeGet failed: %d \n", retVal);
        return retVal;
    }

    /*This API can be called for non-ERSPAN flow. check and return */
    if (mirrorType != XP_MIRROR_ERSPAN2)
    {
        return XP_NO_ERR;
    }

    rc = cpssHalPolicerTriggerEntryGet(devId, CPSS_DXCH_POLICER_STAGE_EGRESS_E,
                                       ePort, &policerTrig);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalPolicerTriggerEntryGet  failed with err : %d n ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    for (type = XPS_ACL_TABLE_TYPE_ARP; type <= XPS_ACL_TABLE_TYPE_IPV6; type++)
    {
        if (type == XPS_ACL_TABLE_TYPE_IPV4)
        {
            tableId = egressErspanIpv4AclTableId;
            vtcamId = XPS_L3_EGR_STG_TABLE_ID_0;
            keyFormat = XPS_PCL_IPV4_L4_KEY;
        }
        else if (type == XPS_ACL_TABLE_TYPE_IPV6)
        {
            tableId = egressErspanIpv6AclTableId;
            vtcamId = XPS_L3V6_EGR_STG_TABLE_ID_0;
            keyFormat = XPS_PCL_IPV6_L4_KEY;
        }
        else if (type == XPS_ACL_TABLE_TYPE_NON_IP)
        {
            tableId = egressErspanNonIpAclTableId;
            vtcamId = XPS_L3_EGR_STG_TABLE_ID_0; //NON-IP is 30B UDB
            keyFormat = XPS_PCL_NON_IP_KEY;
        }
        else
        {
            tableId = egressErspanNonIpAclTableId;
            vtcamId = XPS_L3_EGR_STG_TABLE_ID_0; /* Add the rule to Non-IP table*/
            keyFormat = XPS_PCL_ARP_KEY;
        }

        tableIdDbKey.tableId = tableId;
        if (((retVal = xpsStateSearchData(scopeId, aclTableIdDbHndl,
                                          (xpsDbKey_t)&tableIdDbKey,
                                          (void**)&tableIdDbEntry)) != XP_NO_ERR))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsStateSearchData failed with err : %d \n ", retVal);
            return retVal;
        }

        if (tableIdDbEntry == NULL)
        {
            retVal = XP_ERR_NULL_POINTER;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsStateSearchData failed with err : %d \n ", retVal);
            return retVal;
        }

        tableIdDbEntry->vtcamId[0] = vtcamId;
        tableIdDbEntry->numOfDuplicates = 1;

        portGroup = (portNum)/XPS_PORT_LIST_OFFSET;
        offset = (0x1)<<((portNum)%(XPS_PORT_LIST_OFFSET));

        if (portGroup >= XPS_PORT_LIST_PORT_GROUP_MAX)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Invalid port Num : %d \n ", portNum);
            return XP_ERR_INVALID_VALUE;
        }

        //if adding a port to a portlist
        if (enable)
        {
            tableIdDbEntry->portGroupBmp[portGroup]++;
            tableIdDbEntry->portListBmp[portGroup] |= ((portGroup << XPS_PORT_LIST_OFFSET)|
                                                       offset);
            tableIdDbEntry->bindCount++;
        }
        //if removing a port from port list
        else
        {
            tableIdDbEntry->bindCount--;
            tableIdDbEntry->portListBmp[portGroup] &= (~offset);
        }

        GT_U32 maxKeyFlds = 2;
        xpsAclkeyFieldList_t       aclFieldData;
        xpsAclkeyField_t           aclFieldList[maxKeyFlds + 1];
        uint8_t                    iacl_value[maxKeyFlds];
        uint8_t                    iacl_mask[maxKeyFlds];
        uint32_t                   acl_value_for_port_bitmap;
        uint32_t                   acl_mask_for_port_bitmap;
        xpsPclAction_t             aclEntryData;
        uint32_t i;
        uint32_t ruleId = ((analyzerIndex*(XPS_ACL_TABLE_TYPE_IPV6+1)) + type);
        uint32_t newRuleId = 0;

        memset(&aclFieldData, 0x00, sizeof(aclFieldData));
        memset(&aclEntryData, 0x00, sizeof(aclEntryData));
        memset(&aclFieldList, 0x00, sizeof(aclFieldList));
        memset(iacl_value, 0x00, sizeof(iacl_value));
        memset(iacl_mask, 0x00, sizeof(iacl_mask));
        memset(&acl_value_for_port_bitmap, 0x00, sizeof(acl_value_for_port_bitmap));
        memset(&acl_mask_for_port_bitmap, 0x00, sizeof(acl_mask_for_port_bitmap));

        aclEntryData.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
        aclEntryData.oam.oamProcessEnable                  = GT_TRUE;
        aclEntryData.oam.oamProfile                        =
            CPSSHAL_ERSPAN_OAM_PROFILE_INDEX;
        aclEntryData.flowId                                =
            CPSSHAL_ERSPAN_EPCL_FLOW_ID_START + analyzerIndex;
        if (policerTrig.meteringEnable == GT_TRUE)
        {
            aclEntryData.policer.policerEnable                 =
                CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
            aclEntryData.policer.policerId                     = policerTrig.policerIndex;
        }
        else
        {
            aclEntryData.policer.policerEnable                 =
                CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E;
            aclEntryData.policer.policerId                     = 0;
        }
        aclEntryData.egressPolicy                          = GT_TRUE;
        aclEntryData.epclPhaInfo.phaThreadIdAssignmentMode =
            CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
        aclEntryData.epclPhaInfo.phaThreadId               =
            (isIpv4) ? CPSSHAL_ERSPAN_IPV4_PHA_THREAD_ID :
            CPSSHAL_ERSPAN_IPV6_PHA_THREAD_ID;
        aclEntryData.epclPhaInfo.phaThreadType             =
            CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ERSPAN_TYPE_II_MIRROR_E;
        aclEntryData.epclPhaInfo.phaThreadUnion.epclErspanTypeIImirror.erspanAnalyzerIndex
            = analyzerIndex;

        for (i =0; i < maxKeyFlds; i++)
        {
            aclFieldList[i].value = (uint8_t*)&iacl_value[i];
            aclFieldList[i].mask  = (uint8_t*)&iacl_mask[i];
        }

        /* Field for port bitmap */
        aclFieldList[i].value = (uint8_t*)&acl_value_for_port_bitmap;
        aclFieldList[i].mask  = (uint8_t*)&acl_mask_for_port_bitmap;

        aclFieldData.fldList = aclFieldList;
        aclFieldData.numFlds = maxKeyFlds;
        aclFieldData.isValid = 1;

        uint8_t value = 0x02; /* Egress eDSA tag CMD - TO_Analyzer */
        uint8_t keyMask = 0x03;
        aclFieldData.fldList[0].keyFlds = XPS_PCL_DSA_CMD;
        memcpy(aclFieldData.fldList[0].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[0].mask, &keyMask, sizeof(uint8_t));

        /* Qualify ePort associated with analyzer index to differentiat
          each mirror session. */
        value = (ePort& 0x7F) << 1; /* Analyzer EPORT */
        keyMask = 0xFE;
        aclFieldData.fldList[1].keyFlds = XPS_PCL_EPORT;
        memcpy(aclFieldData.fldList[1].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[1].mask, &keyMask, sizeof(uint8_t));

        retVal =  xpsAclWriteEntry(devId, tableId, 0, ruleId,
                                   &aclFieldData,
                                   aclEntryData, keyFormat, false);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: xpsAclWriteEntry failed for ruleId: %d \n", ruleId);
            return retVal;
        }

        if (enable == false)
        {
            tableIdDbEntry->portGroupBmp[portGroup]--;
        }

        if (tableIdDbEntry->bindCount == 0)
        {
            for (uint32_t i =0; i< tableIdDbEntry->numEntries; i++)
            {
                for (uint32_t portGroupIndex = 0; portGroupIndex < XPS_PORT_LIST_PORT_GROUP_MAX;
                     portGroupIndex++)
                {
                    newRuleId = ((portGroupIndex << 28) | (tableIdDbEntry->ruleId[i] &
                                                           0xFFFFFFF)); /* copying portGroupId and ruleId */

                    for (uint32_t j=0; j<tableIdDbEntry->numOfDuplicates; j++)
                    {
                        valid = GT_FALSE;
                        /* Deleting rule from hardware */
                        cpssDxChVirtualTcamRuleValidStatusGet(tableIdDbEntry->tcamMgrId,
                                                              tableIdDbEntry->vtcamId[j],
                                                              newRuleId, &valid);

                        /* Deleting rule from hardware */
                        if (valid)
                        {
                            rc = cpssDxChVirtualTcamRuleDelete(tableIdDbEntry->tcamMgrId,
                                                               tableIdDbEntry->vtcamId[j], newRuleId);
                            if (rc != GT_OK)
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                      " cpssDxChVirtualTcamRuleDelete  failed with err : %d n ", rc);
                                return xpsConvertCpssStatusToXPStatus(rc);
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}


XP_STATUS xpsAclGetPortMapFromTableId(xpsDevice_t devId, uint32_t tableId,
                                      uint32_t **pPortListBmp)
{
    XP_STATUS                       xpsRetVal = XP_NO_ERR;
    xpsAclTableContextDbEntry_t     tableIdDbKey;
    xpsAclTableContextDbEntry_t     *tableIdDbEntry = NULL;

    if (pPortListBmp == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "pPortListBmp is NULL.\n ");
        return XP_ERR_NULL_POINTER;
    }

    /* Retrieving table info */
    tableIdDbKey.tableId = tableId ;
    if (((xpsRetVal = xpsStateSearchData(devId, aclTableIdDbHndl,
                                         (xpsDbKey_t)&tableIdDbKey, (void**)&tableIdDbEntry)) != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
        return xpsRetVal;
    }

    if (tableIdDbEntry == NULL)
    {
        return xpsRetVal;
    }

    *pPortListBmp = tableIdDbEntry->portListBmp;

    return xpsRetVal;
}


#ifdef __cplusplus
}
#endif

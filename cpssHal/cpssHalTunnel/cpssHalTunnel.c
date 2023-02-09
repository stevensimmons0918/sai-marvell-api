/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/


/*******************************************************************************
* @file cpssHalTunnel.c
*
* @brief Internal header which defines API for helpers functions, which are
*        specific for XPS Tunnel.
*
* @version   01
*******************************************************************************/

#include "cpssHalTunnel.h"
#include "cpssHalUtil.h"
#include "cpssHalL3.h"
#include "cpssHalDevice.h"
#include "xpsCommon.h"
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnel.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>

/**
* @internal cpssHalTtiPortLookupEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TTI lookup for the specified key
*         type at the port.
*
* @param[in] cpssDevNum               - device number
* @param[in] cpssPortNum              - port number
* @param[in] keyType                  - TTI key type
* @param[in] enable                   - GT_TRUE:  TTI lookup
*                                      GT_FALSE: disable TTI lookup
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
#define TTI_OVERRIDE_SRCID_MASK 7

GT_STATUS cpssHalTtiPortLookupEnableSet(GT_U8 cpssDevNum,
                                        GT_U32 cpssPortNum,
                                        CPSS_DXCH_TTI_KEY_TYPE_ENT keyType,
                                        GT_BOOL enable)
{
    GT_STATUS status = GT_OK;

    status = cpssDxChTtiPortLookupEnableSet(cpssDevNum, (GT_PORT_NUM)cpssPortNum,
                                            keyType, enable);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set TTI Port Lookup, "
              "status:%d, dev:%d, portNum:%d, enable:%d\n", status, cpssDevNum, cpssPortNum,
              enable);
        return status;
    }
    return status;
}


/**
* @internal cpssHalTtiPortIpv4OnlyTunneledEnableSet function
* @endinternal
*
* @brief   This function enables/disables the IPv4 TTI lookup for only tunneled
*         packets received on port.
*
* @param[in] devId                   - device number
* @param[in] cpssPortNum             - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                  - GT_TRUE:  IPv4 TTI lookup only for tunneled packets
*                                      GT_FALSE: disable IPv4 TTI lookup only for tunneled packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS cpssHalTtiPortIpv4OnlyTunneledEnableSet(GT_U8 cpssDevNum,
                                                  GT_U32 cpssPortNum, GT_BOOL enable)
{
    GT_STATUS status = GT_OK;

    status = cpssDxChTtiPortIpv4OnlyTunneledEnableSet(cpssDevNum,
                                                      (GT_PORT_NUM)cpssPortNum, enable);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set IPV4 Tunnel Enable, "
              "status:%d, dev:%d, portNum:%d, enable:%d\n", status, cpssDevNum, cpssPortNum,
              enable);
        return status;
    }
    return status;
}

/**
* @internal cpssHalTtiPortIpv4OnlyMacToMeEnableSet function
* @endinternal
*
* @brief   This function enables/disables the IPv4 TTI lookup for only mac to me
*         packets received on port.
*
* @param[in] devId                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  IPv4 TTI lookup only for mac to me packets
*                                      GT_FALSE: disable IPv4 TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalTtiPortIpv4OnlyMacToMeEnableSet(GT_U8 cpssDevNum,
                                                 GT_U32 cpssPortNum, GT_BOOL enable)
{
    GT_STATUS status = GT_OK;

    status = cpssDxChTtiPortIpv4OnlyMacToMeEnableSet(cpssDevNum,
                                                     (GT_PORT_NUM)cpssPortNum, enable);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set IPV4 Mac2Me Tunnel Enable, "
              "status:%d, dev:%d, portNum:%d, enable:%d\n", status, cpssDevNum, cpssPortNum,
              enable);
        return status;
    }
    return status;
}

/**
* @internal cpssHalTtiMacModeGet function
* @endinternal
*
* @brief   This function gets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @param[in] devId                   - device number
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E
*
* @param[out] macModePtr               - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or key type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssHalTtiMacModeGet(GT_U32 devId,
                               CPSS_DXCH_TTI_KEY_TYPE_ENT keyType,
                               CPSS_DXCH_TTI_MAC_MODE_ENT *macModePtr)
{
    GT_STATUS status = GT_OK;

    if ((uint8_t)devId == 0xff)
    {
        return GT_FAIL;
    }

    status = cpssDxChTtiMacModeGet(devId, keyType, macModePtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get TTI Mac Mode, "
              "status:%d, dev:%d \n", status, devId);
        return status;
    }
    return status;
}

/**
* @internal cpssHalTtiMacModeSet function
* @endinternal
* @brief   This function sets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @param[in] devId                   - device number
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E
* @param[in] macMode                  - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not needed for TRILL key
*
*/
GT_STATUS cpssHalTtiMacModeSet(GT_U32 devId,
                               CPSS_DXCH_TTI_KEY_TYPE_ENT keyType,
                               CPSS_DXCH_TTI_MAC_MODE_ENT macMode)
{
    GT_STATUS status = GT_OK;

    if ((uint8_t)devId == 0xff)
    {
        return GT_FAIL;
    }

    status = cpssDxChTtiMacModeSet(devId, keyType, macMode);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to set TTI Mac Mode, "
              "status:%d, dev:%d \n", status, devId);
        return status;
    }
    return status;
}

/*
* @internal cpssHalTunnelTerminationEntryAdd function
* @endinternal
*
* @brief   Write Rule to Tunnel Table.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*                                      if rule with given rule Id already exists it overridden only
*                                      when the same priotity specified, otherwize GT_BAD_PARAM returned.
* @param[in] ruleAttributesPtr        - (pointer to)rule attributes (for priority driven vTCAM - priority)
* @param[in] ruleTypePtr              -     (pointer to)type of contents of rule
* @param[in] ruleDataPtr              -     (pointer to)data of contents of rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/

GT_STATUS cpssHalTunnelTerminationEntryAdd(GT_U32 vTcamMngId, GT_U32 vTcamId,
                                           CPSS_DXCH_VIRTUAL_TCAM_RULE_ID ruleId,
                                           CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC *ruleAttributesPtr,
                                           CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC *ruleTypePtr,
                                           CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC *ruleDataPtr)
{
    GT_STATUS status = GT_OK;

    status = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, ruleId,
                                          ruleAttributesPtr, ruleTypePtr, ruleDataPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to Create TCAM Table Entry, "
              "status:%d, MgrId:%d TableId:%d\n", status, vTcamMngId, vTcamId);
        return status;
    }
    return status;
}

/**
* @internal cpssHalTunnelTerminationActionUpdate function
* @endinternal
*
* @brief   Update Rule Action for Tunnel
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] actionTypePtr            - (pointer to)type of contents of action
* @param[in] actionDataPtr            - (pointer to)data of contents of action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
GT_STATUS cpssHalTunnelTerminationActionUpdate
(
    GT_U32                                        vTcamMngId,
    GT_U32                                        vTcamId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                ruleId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   *actionTypePtr,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   *actionDataPtr
)
{
    GT_STATUS status = GT_OK;

    status = cpssDxChVirtualTcamRuleActionUpdate(
                 vTcamMngId, vTcamId, (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)ruleId,
                 actionTypePtr, actionDataPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to Update TCAM Table Entry, "
              "status:%d, MgrId:%d TableId:%d RuleId :%d \n", status, vTcamMngId, vTcamId,
              ruleId);
        return status;
    }
    return status;
}

/**
* @internal cpssHalTunnelTerminationActionGet function
* @endinternal
*
* @brief   Get Rule Action.
*
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] actionTypePtr            - (pointer to)type of contents of action
*
* @param[out] actionDataPtr            - (pointer to)data of contents of action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
GT_STATUS cpssHalTunnelTerminationActionGet
(
    GT_U32                                        vTcamMngId,
    GT_U32                                        vTcamId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                ruleId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   *actionTypePtr,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   *actionDataPtr
)
{
    GT_STATUS status = GT_OK;
    status = cpssDxChVirtualTcamRuleActionGet(
                 vTcamMngId, vTcamId, (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)ruleId,
                 actionTypePtr, actionDataPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to Get TCAM Table Entry, "
              "status:%d, MgrId:%d TableId:%d RuleId :%d \n", status, vTcamMngId, vTcamId,
              ruleId);
        return status;
    }
    return status;
}


GT_STATUS cpssHalTunnelTerminationRuleGet
(
    GT_U32                                      vTcamMngId,
    GT_U32                                      vTcamId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        *ruleTypePtr,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        *ruleDataPtr
)
{
    GT_STATUS status = GT_OK;
    status = cpssDxChVirtualTcamRuleRead(
                 vTcamMngId, vTcamId, (CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)ruleId,
                 ruleTypePtr, ruleDataPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to Get TCAM Table Entry, "
              "status:%d, MgrId:%d TableId:%d RuleId :%d \n", status, vTcamMngId, vTcamId,
              ruleId);
        return status;
    }
    return status;
}

/**
* @internal cpssHalTunnelTerminationEntryDelete function
* @endinternal
*
* @brief   Delete Rule From internal Tunnel Table
*
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
GT_STATUS cpssHalTunnelTerminationEntryDelete(GT_U32 vTcamMngId, GT_U32 vTcamId,
                                              CPSS_DXCH_VIRTUAL_TCAM_RULE_ID ruleId)
{
    GT_STATUS status = GT_OK;

    status = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, ruleId);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to Delete TCAM Table Entry, "
              "status:%d, MgrId:%d TableId:%d\n", status, vTcamMngId, vTcamId);
        return status;
    }
    return status;
}

/**
* @internal cpssHalTunnelTerminationIPv6KeySet function
* @endinternal
*
* @brief   Set Tunnel TTI params for UDB IPV6 Key
*
*
* @param[in] devId                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/


GT_STATUS cpssHalTunnelTerminationIPv6KeySet(GT_U32 devId)
{
    GT_STATUS status = GT_OK;
    int i = -1;
    int  pclId = CPSS_HAL_TTI_UDB_IPV6_PCL_ID;

    status = cpssDxChTtiPacketTypeKeySizeSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV6_E,
                                             CPSS_DXCH_TTI_KEY_SIZE_30_B_E);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChTtiPacketTypeKeySizeSet failed with err : %d\n for Device :%d ",
              status, devId);
        return (status);
    }

    status = cpssDxChTtiPclIdSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV6_E, pclId);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChTtiPclIdSet failed with err : %d\n for Device :%d PclId :%d \n",
              status, devId, pclId);
        return (status);
    }

    /* User Defined Byte offset to PCL-ID*/
    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV6_E, 0,
                                           CPSS_DXCH_TTI_OFFSET_METADATA_E, CPSS_HAL_TTI_META_PCL_ID_OFFSET);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }

    /* User Defined Byte offset to TTI META IPv6 Tunneling Protocol */
    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV6_E, 1,
                                           CPSS_DXCH_TTI_OFFSET_METADATA_E,
                                           CPSS_HAL_TTI_META_IPV6_TNL_PROT_ID_OFFSET);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }
    /* User Defined Byte offset to IPv6 DstIp*/
    for (i = 0; i < CPSS_HAL_IPV6_ADDR_LEN; i++)
    {
        status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV6_E,
                                               (2 + i),
                                               CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E,
                                               CPSS_HAL_TTI_IPV6_DIP_OFFSET+i);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChTtiUserDefinedByteSet, "
                  "status:%d, devId:%d \n", status, devId);
            return (status);
        }
    }
    return status;
}

GT_STATUS cpssHalTunnelTunnelStartEntryWrite
(
    int                                   devId,
    GT_U32                                entryId,
    CPSS_TUNNEL_TYPE_ENT                  tunnelType,
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT    *entry_PTR
)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTunnelStartEntrySet(devNum, entryId, tunnelType, entry_PTR);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TS entry write failed :%d ",
                  status);
            return status;
        }

    }
    return status;
}

GT_STATUS cpssHalTunnelTunnelStartEntryRead
(
    int                                   devId,
    GT_U32                                entryId,
    CPSS_TUNNEL_TYPE_ENT                 *tunnelType_PTR,
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT    *entry_PTR
)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTunnelStartEntryGet(devNum, entryId, tunnelType_PTR,
                                             entry_PTR);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TS entry read failed");
            return status;
        }
        /* read is needed only from first device */
        break;
    }
    return status;
}

GT_STATUS cpssHalTunnelStartEcnModeSet(int devId,
                                       CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT ecnMode)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTunnelStartEcnModeSet(devNum, ecnMode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TS entry write failed");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalTunnelStartEcnModeGet(int devId,
                                       CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT *ecnMode)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTunnelStartEcnModeGet(devNum, ecnMode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TS entry write failed");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalTunnelVlanToVniMapSet
(
    int    devId,
    GT_U32 vlan,
    GT_U32 vni
)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTunnelStartEgessVlanTableServiceIdSet(devNum, vlan, vni);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChTunnelStartEgessVlanTableServiceIdSet failed:%d", status);
            return status;
        }

    }
    return status;
}

GT_STATUS cpssHalTunnelVxlanInit
(
    int    devId
)
{
    GT_STATUS   status = GT_OK;
    GT_U8       devNum;
    GT_U8       i = 0;
    CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC profile;
    memset(&profile, 0, sizeof(profile));

    profile.templateDataSize =
        CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E; /* 8 byte Vlan header */
    /* Configure template for Vxlan header - 24bit VNI is dervied from eVlan <service-Id> */
    for (i=0; i<64; i++)
    {
        profile.templateDataBitsCfg[i] =
            CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_0_E;
        if (i>=48 && i<=55)
        {
            profile.templateDataBitsCfg[i] =
                CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_0_E;
        }
        if (i>=40 && i<=47)
        {
            profile.templateDataBitsCfg[i] =
                CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_1_E;
        }
        if (i>=32 && i<=39)
        {
            profile.templateDataBitsCfg[i] =
                CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_2_E;
        }
        if (i==3)
        {
            profile.templateDataBitsCfg[i] =
                CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
        }
    }

    profile.udpSrcPortMode =
        CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTunnelStartGenProfileTableEntrySet(devNum,
                                                            CPSSHAL_GEN_TUNNEL_PROFILE_VXLAN_UDP_PORT_FROM_HASH, &profile);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChTunnelStartGenProfileTableEntrySet failed:%d", status);
            return status;
        }

        profile.udpSrcPortMode =
            CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E;
        status = cpssDxChTunnelStartGenProfileTableEntrySet(devNum,
                                                            CPSSHAL_GEN_TUNNEL_PROFILE_VXLAN_UDP_PORT_FROM_TS, &profile);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChTunnelStartGenProfileTableEntrySet failed:%d", status);
            return status;
        }
        status = cpssDxChTunnelStartPassengerVlanTranslationEnableSet(devNum, GT_TRUE);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "TS VlanTranslation failed:%d", status);
            return status;
        }

        status = cpssDxChBrgL2EcmpEnableSet(devNum, GT_TRUE);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Enable L2Ecmp failed:%d", status);
            return status;
        }

    }

    return status;
}

GT_STATUS cpssHalTunnelToEPortMapSet
(
    IN GT_U8              devId,
    IN GT_PORT_NUM        ePort,
    IN GT_U32             tunnelStartIndex
)
{
    GT_STATUS                           status = GT_OK;
    int                                 devNum;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  egressInfo;

    memset(&egressInfo, 0, sizeof(egressInfo));
    egressInfo.tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
    egressInfo.forceArpTsPortDecision = GT_TRUE;
    egressInfo.tunnelStart = GT_TRUE;
    egressInfo.tunnelStartPtr = tunnelStartIndex;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, ePort,
                                                                 &egressInfo);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting tunnel start entry to eport failed devNum=%d port=%d tunnel=%d",
                  devNum, ePort, tunnelStartIndex);
            return status;
        }
    }

    return status;
}

GT_STATUS cpssHalTunnelToEPortMapClear
(
    IN GT_U8              devId,
    IN GT_PORT_NUM        ePort
)
{
    GT_STATUS                           status = GT_OK;
    int                                 devNum;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  egressInfo;

    memset(&egressInfo, 0, sizeof(egressInfo));
    egressInfo.tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
    egressInfo.forceArpTsPortDecision = GT_TRUE;
    egressInfo.tunnelStart = GT_FALSE;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, ePort,
                                                                 &egressInfo);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting tunnel start entry to eport failed devNum=%d port=%d",
                  devNum, ePort);
            return status;
        }
    }

    return status;
}

GT_STATUS cpssHalTunnelEPortToPhyPortMapSet
(
    IN GT_U8              devId,
    IN GT_PORT_NUM        ePort,
    IN CPSS_INTERFACE_TYPE_ENT type,
    IN GT_U32             intfNum
)
{
    GT_STATUS                                 status = GT_OK;
    int                                       devNum;
    CPSS_INTERFACE_INFO_STC                   physicalInfo;
    GT_U32                                    cpssDevNum;
    GT_U32                                    cpssPortNum;

    memset(&physicalInfo, 0, sizeof(physicalInfo));

    physicalInfo.type = type;
    if (type == CPSS_INTERFACE_PORT_E)
    {
        cpssHalSetDeviceSwitchId(devId);
        cpssDevNum = xpsGlobalIdToDevId(devId, intfNum);
        cpssPortNum = xpsGlobalPortToPortnum(devId, intfNum);
        physicalInfo.devPort.hwDevNum = cpssDevNum;
        physicalInfo.devPort.portNum = cpssPortNum;
    }
    else if (type == CPSS_INTERFACE_TRUNK_E)
    {
        physicalInfo.trunkId = intfNum;
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     ePort,
                                                                     &physicalInfo);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to clear eport to physical port mapping with error %d", status);
            return status;
        }
    }

    return status;
}

GT_STATUS cpssHalTunnelEPorTMtuProfileSet
(
    IN GT_U8  devId,
    IN GT_U32 portNum,
    IN GT_U32 ePort
)
{
    GT_STATUS status = GT_OK;
    GT_U32    mtuProfileIdx;
    GT_U32    cpssDevNum;
    GT_U32    cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    status = cpssDxChBrgGenMtuPortProfileIdxGet(cpssDevNum, cpssPortNum,
                                                &mtuProfileIdx);
    if (status!= GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get MTU profile Index for analyzer port %d", cpssPortNum);
        return status;
    }
    status = cpssDxChBrgGenMtuPortProfileIdxSet(cpssDevNum, ePort, mtuProfileIdx);
    if (status!= GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set MTU profile Index for ePort %d", ePort);
        return status;
    }

    return status;
}

GT_STATUS cpssHalTunnelEPorTSrcMacClear
(
    IN GT_U8  devId,
    IN GT_U32 ePort
)
{
    GT_U32    cpssDevNum;
    GT_STATUS cpssStatus = GT_OK;
    uint32_t macSaTableIndex = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        cpssStatus = cpssDxChIpRouterPortGlobalMacSaIndexGet(cpssDevNum, ePort,
                                                             &macSaTableIndex);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get MAC SA table index to port id %d\n", ePort);
            return (cpssStatus);
        }

        /* Free mac sa table index */
        cpssStatus = cpssHalL3FreeMacSaTableIndex(macSaTableIndex);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set free MAC SA table index %d\n", macSaTableIndex);
            return (cpssStatus);
        }

        macSaTableIndex = DEFAULT_MAC_SA_TABLE_INDEX;

        // Set the allocated index to the egress port
        cpssStatus = cpssDxChIpRouterPortGlobalMacSaIndexSet(cpssDevNum, ePort,
                                                             macSaTableIndex);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set MAC SA table index to port id %d\n", ePort);
            return (cpssStatus);
        }
    }
    return (cpssStatus);
}

GT_STATUS cpssHalTunnelEPorTSrcMacSet
(
    IN GT_U8  devId,
    IN GT_U32 macSaTableIndex,
    IN GT_U32 ePort
)
{
    GT_STATUS status = GT_OK;
    GT_U32    cpssDevNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        status = cpssDxChIpRouterPortGlobalMacSaIndexSet(cpssDevNum, ePort,
                                                         macSaTableIndex);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set MAC SA table index to port id %d\n", ePort);
            return (status);
        }
    }

    return status;
}

GT_STATUS cpssHalTunnelEPortToPhyPortMapClear
(
    IN GT_U8              devId,
    IN GT_PORT_NUM        ePort
)
{
    GT_STATUS                                 status = GT_OK;
    int                                       devNum;
    CPSS_INTERFACE_INFO_STC                   physicalInfo;

    memset(&physicalInfo, 0, sizeof(physicalInfo));

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                     ePort,
                                                                     &physicalInfo);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to clear eport to physical port mapping with error %d", status);
            return status;
        }
    }

    return status;
}

/**
* @internal cpssHalTunnelVxlanIPv4TTIKeySet function
* @endinternal
*
* @brief   Set Tunnel TTI params for VXLAN IPV4 UDB Key
*
*
* @param[in] devId                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/

GT_STATUS cpssHalTunnelVxlanIPv4TTIKeySet(GT_U32 devId)
{
    GT_STATUS status = GT_OK;
    int i = -1;

    status = cpssDxChTtiPacketTypeKeySizeSet(devId,
                                             CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E,
                                             CPSS_DXCH_TTI_KEY_SIZE_30_B_E);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChTtiPacketTypeKeySizeSet failed with err : %d\n for Device :%d ",
              status, devId);
        return (status);
    }

    status = cpssDxChTtiPclIdSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E,
                                 CPSS_HAL_TTI_UDB_IPV4_VXLAN_PCL_ID);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChTtiPclIdSet failed with err : %d\n for Device :%d\n",
              status, devId);
        return (status);
    }

    /* User Defined Byte offset to PCL-ID*/
    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E,
                                           0,
                                           CPSS_DXCH_TTI_OFFSET_METADATA_E, CPSS_HAL_TTI_META_PCL_ID_OFFSET);
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }

    /*Source ePort/Trunk info*/
    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E,
                                           1,
                                           CPSS_DXCH_TTI_OFFSET_METADATA_E,
                                           26 /*offset*/);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }
    /* User Defined Byte offset to IP header Protocol */
    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E,
                                           2,
                                           CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E,
                                           11 /*offset*/);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }

    /* User Defined Byte offset to UDP dst port */
    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E,
                                           3,
                                           CPSS_DXCH_TTI_OFFSET_L4_E,
                                           2 /*offset*/);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }

    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E,
                                           4,
                                           CPSS_DXCH_TTI_OFFSET_L4_E,
                                           3 /*offset*/);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }

    /* User Defined Byte offset to VNI id */
    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E,
                                           5,
                                           CPSS_DXCH_TTI_OFFSET_L4_E,
                                           12 /*offset*/);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }

    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E,
                                           6,
                                           CPSS_DXCH_TTI_OFFSET_L4_E,
                                           13 /*offset*/);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }

    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E,
                                           7,
                                           CPSS_DXCH_TTI_OFFSET_L4_E,
                                           14 /*offset*/);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }

    /* User Defined Byte offset to DIP*/
    for (i = 0; i < CPSS_HAL_IPV4_ADDR_LEN; i++)
    {
        status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E,
                                               (8 + i),
                                               CPSS_DXCH_TTI_OFFSET_METADATA_E,
                                               (4 - i) /*offset*/);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChTtiUserDefinedByteSet, "
                  "status:%d, devId:%d \n", status, devId);
            return (status);
        }
    }

    /* User Defined Byte offset to SIP*/
    for (i = 0; i < CPSS_HAL_IPV4_ADDR_LEN; i++)
    {
        status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E,
                                               (12 + i),
                                               CPSS_DXCH_TTI_OFFSET_METADATA_E,
                                               (8 - i) /*offset*/);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChTtiUserDefinedByteSet, "
                  "status:%d, devId:%d \n", status, devId);
            return (status);
        }
    }

    return status;
}

/**
* @internal cpssHalTunnelVxlanIPv6TTIKeySet function
* @endinternal
*
* @brief   Set Tunnel TTI params for VXLAN IPV6 UDB Key
*
*
* @param[in] devId                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/

GT_STATUS cpssHalTunnelVxlanIPv6TTIKeySet(GT_U32 devId)
{
    GT_STATUS status = GT_OK;
    int i = -1;

    status = cpssDxChTtiPacketTypeKeySizeSet(devId,
                                             CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E,
                                             CPSS_DXCH_TTI_KEY_SIZE_30_B_E);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChTtiPacketTypeKeySizeSet failed with err : %d\n for Device :%d ",
              status, devId);
        return (status);
    }

    status = cpssDxChTtiPclIdSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E,
                                 CPSS_HAL_TTI_UDB_IPV6_VXLAN_PCL_ID);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChTtiPclIdSet failed with err : %d\n for Device :%d \n",
              status, devId);
        return (status);
    }

    /* User Defined Byte offset to PCL-ID*/
    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E,
                                           0,
                                           CPSS_DXCH_TTI_OFFSET_METADATA_E, CPSS_HAL_TTI_META_PCL_ID_OFFSET);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }

    /*Source ePort/Trunk info*/
    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E,
                                           1,
                                           CPSS_DXCH_TTI_OFFSET_METADATA_E,
                                           26 /*offset*/);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }
    /* User Defined Byte offset to IP header Protocol */
    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E,
                                           2,
                                           CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E,
                                           8 /*offset*/);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }

    /* User Defined Byte offset to UDP dst port */
    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E,
                                           3,
                                           CPSS_DXCH_TTI_OFFSET_L4_E,
                                           2 /*offset*/);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }

    status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E,
                                           4,
                                           CPSS_DXCH_TTI_OFFSET_L4_E,
                                           3 /*offset*/);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChTtiUserDefinedByteSet, "
              "status:%d, devId:%d \n", status, devId);
        return (status);
    }

    /* User Defined Byte offset to DIP*/
    for (i = 0; i < CPSS_HAL_IPV6_ADDR_LEN; i++)
    {
        status = cpssDxChTtiUserDefinedByteSet(devId, CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E,
                                               (5 + i),
                                               CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E,
                                               (26 + i) /*offset*/);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChTtiUserDefinedByteSet, "
                  "status:%d, devId:%d \n", status, devId);
            return (status);
        }
    }

    return status;
}

GT_STATUS cpssHalEportToPhysicalPortTargetMappingTableGet
(
    uint32_t  devId,
    uint32_t  portNum,
    CPSS_INTERFACE_INFO_STC *physicalInfoPtr
)
{
    GT_STATUS status = GT_OK;
    status = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devId, portNum,
                                                                 physicalInfoPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Eport get failed :%d ",
              status);
        return status;
    }

    return status;
}

GT_STATUS cpssHalTunnelL2EcmpLttTableSet
(
    IN GT_U8              devId,
    IN GT_PORT_NUM        ePort,
    IN CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC *info
)
{
    GT_STATUS status = GT_OK;
    int       devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgL2EcmpLttTableSet(devNum, ePort, info);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "L2EcmpLttTableSet Failed  %d", status);
            return status;
        }
    }

    return status;
}

GT_STATUS cpssHalTunnelL2EcmpTableSet
(
    IN GT_U8              devId,
    IN GT_U32             index,
    IN CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC     *ecmpEntryPtr
)
{
    GT_STATUS status = GT_OK;
    int       devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if (ecmpEntryPtr)
        {
            ecmpEntryPtr->targetHwDevice = devNum;
        }

        status = cpssDxChBrgL2EcmpTableSet(devNum, index, ecmpEntryPtr);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "L2EcmpTableSet Failed  %d", status);
            return status;
        }
    }

    return status;
}

GT_STATUS cpssHalGlobalEportRangeUpdate(uint32_t devId,
                                        CPSS_HAL_EPORT_TYPE type,
                                        uint32_t minVal, uint32_t maxVal)
{
    GT_STATUS status = GT_OK;
    int       devNum;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  global;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  l2Ecmp;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  l2Dlb;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        memset(&global, 0, sizeof(global));
        memset(&l2Ecmp, 0, sizeof(l2Ecmp));
        memset(&l2Dlb, 0, sizeof(l2Dlb));

        status = cpssDxChCfgGlobalEportGet(devNum, &global, &l2Ecmp, &l2Dlb);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCfgGlobalEportGet Failed  %d", status);
            return status;
        }

        if (type  == CPSS_HAL_EPORT_TYPE_GLOBAL)
        {
            global.enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E;
            global.minValue = minVal;
            global.maxValue = maxVal;
        }
        else if (type == CPSS_HAL_EPORT_TYPE_L2ECMP)
        {
            l2Ecmp.enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E;
            l2Ecmp.minValue = minVal;
            l2Ecmp.maxValue = maxVal;
        }
        else
        {
            return GT_NOT_SUPPORTED;
        }

        status = cpssDxChCfgGlobalEportSet(devNum, &global, &l2Ecmp, &l2Dlb);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCfgGlobalEportSet Failed  %d type %d ", status, type);
            return status;
        }
        if (type == CPSS_HAL_EPORT_TYPE_L2ECMP)
        {
            status = cpssDxChBrgL2EcmpIndexBaseEportSet(devNum, minVal);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "L2Ecmp Base Eport Set failed:%d", status);
                return status;
            }
        }
    }

    return status;
}

GT_STATUS cpssHalTunnelEnableMeshIdFiltering(uint32_t devId,
                                             GT_BOOL enable)
{
    GT_STATUS status = GT_OK;
    int       devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        /* Enable split horizon filtering on Eports */
        status =  cpssDxChBrgEgrMeshIdConfigurationSet(devNum, enable, 0,
                                                       1); /* Offset = 0, bits = 1 */
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Egress MeshId configuration Set failed:%d", status);
            return status;
        }
        /* Enable split horizon filtering on MLL */
        status =  cpssDxChL2MllSourceBasedFilteringConfigurationSet(devNum, enable, 1,
                                                                    1); /* Offset = 1, bits = 1 */
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "L2MLL source based filtering configuration Set failed:%d", status);
            return status;
        }
        status = cpssDxChTtiSourceIdBitsOverrideSet(devId, 0, TTI_OVERRIDE_SRCID_MASK);
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Tti source Id bits override configuration Set failed:%d", status);
            return status;
        }

    }
    return status;
}

GT_STATUS cpssHalTunnelSetEPortMeshId(uint32_t devId, uint32_t ePort,
                                      uint32_t meshId)
{
    GT_STATUS status = GT_OK;
    int       devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status =  cpssDxChBrgEgrPortMeshIdSet(devNum, ePort, meshId);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set the MeshId for primary eport %d\n",
                  ePort);
            return status;
        }
    }
    return status;
}
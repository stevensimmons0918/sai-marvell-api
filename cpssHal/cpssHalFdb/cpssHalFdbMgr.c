/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalFdbMgr.c                                                        *
*                                                                              *
* @brief Private API implementation using CPSS FDB MANAGER which can be used   *
*        in XPS layer.                                                         *
*                                                                              *
* @version   01                                                                *
********************************************************************************
*/


#include "cpssHalFdbMgr.h"
#include "cpssHalDevice.h"
#include "cpssDxChBrgFdbManagerTypes.h"
#include "cpssDxChBrgFdbManager.h"
#include "xpsFdb.h"
#include "xpsFdbMgr.h"
#include "xpsCommon.h"
#include "xpsLock.h"
#include "xpsInternal.h"
#include "xpsScope.h"
#include "xpsMtuProfile.h"
#include "xpsVxlan.h"

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#define FDB_MAX_TOTAL_ENTRIES       _128K
#define NUM_OF_ENTRIES_PER_SCAN      256

/**
* @internal cpssHalFdbMgrConvertDxChTablePktCmdToGenric
* @endinternal
*
* @brief   Convert generic pkt command to device specific packet command
*
* @param[in] pktCmd                     - generic acket command
* @param[out] *cpssTableCmd          - (pointer to) DxCh MAC table command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertDxChTablePktCmdToGenric
(
    CPSS_PACKET_CMD_ENT             cpssTableCmd,
    xpPktCmd_e                      *pktCmd
)
{
    switch (cpssTableCmd)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            *pktCmd= XP_PKTCMD_FWD;
            break;

        case CPSS_PACKET_CMD_DROP_HARD_E:
        case CPSS_PACKET_CMD_DROP_SOFT_E:/*was CPSS_MAC_TABLE_SOFT_DROP_E */
            *pktCmd = XP_PKTCMD_DROP;
            break;

        case CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E:
            *pktCmd = XP_PKTCMD_FWD_MIRROR;
            break;

        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            *pktCmd = XP_PKTCMD_TRAP;
            break;
        case CPSS_PACKET_CMD_ROUTE_E :
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalFdbMgrConvertDxChTablePktCmdToGenric : CPSS_PACKET_CMD_ROUTE_E not supporterd \n")
            ;
            return GT_BAD_PARAM;
        case CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E :
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalFdbMgrConvertDxChTablePktCmdToGenric : CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E not supporterd \n")
            ;
            return GT_BAD_PARAM;
        case CPSS_PACKET_CMD_BRIDGE_E:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalFdbMgrConvertDxChTablePktCmdToGenric : CPSS_PACKET_CMD_BRIDGE_E not supporterd \n")
            ;
            return GT_BAD_PARAM;
        case CPSS_PACKET_CMD_NONE_E:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalFdbMgrConvertDxChTablePktCmdToGenric : CPSS_PACKET_CMD_NONE_E not supporterd \n")
            ;
            return GT_BAD_PARAM;
        case CPSS_PACKET_CMD_LOOPBACK_E:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalFdbMgrConvertDxChTablePktCmdToGenric : CPSS_PACKET_CMD_LOOPBACK_E not supporterd \n")
            ;
            return GT_BAD_PARAM;
        case CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalFdbMgrConvertDxChTablePktCmdToGenric : CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E not supporterd \n")
            ;
            return GT_BAD_PARAM;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalFdbMgrConvertDxChTablePktCmdToGenric : [%d] not supporterd \n",
                  cpssTableCmd) ;
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

GT_STATUS cpssHalFdbMgrConvertFromHwTunnelEntry(xpsFdbEntry_t *fdbEntry)
{
    XP_STATUS   result = XP_NO_ERR;
    xpsDevice_t devId  = 0;
    xpsVlan_t   vlanId = 0;
    xpsInterfaceId_t tnlIntfId = 0;

    if (fdbEntry->vlanId)
    {
        if ((result = xpsVxlanGetTunnelVlanByHwEVlanId(devId, fdbEntry->vlanId,
                                                       &vlanId)) == XP_NO_ERR)
        {
            if (!vlanId)
            {
                return GT_FAIL;
            }
            if ((result = xpsVxlanGetTunnelIdbyEPort(devId, vlanId, fdbEntry->intfId,
                                                     &tnlIntfId)) == XP_NO_ERR)
            {
                fdbEntry->intfId = tnlIntfId;
                fdbEntry->vlanId = vlanId;

                return GT_OK;
            }
        }
    }
    return GT_FAIL;
}

/**
* @internal cpssHalFdbMgrConvertCpssToGenricL2MactEntry function
* @endinternal
*
* @brief   Convert device specific MAC entry into generic MAC entry
*
* @param[in] dxChMacEntryPtr          - (pointer to) cpss MAC entry parameters
*
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[out] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/

GT_STATUS cpssHalFdbMgrConvertCpssToGenricL2MactEntry
(
    int                                                  devId,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                  *dxChMacEntryPtr,
    XPS_FDB_ENTRY_TYPE_ENT                               fdbEntryType,
    void                                                 *macEntryPtr
)
{
    GT_STATUS                   status = GT_OK;
    xpsFdbEntry_t               *fdbEntry  = (xpsFdbEntry_t *)macEntryPtr;
    int                         frontPanelNum = 0;
    XP_STATUS                   xpsStatus = XP_NO_ERR;

    cpssOsMemSet(fdbEntry, 0, sizeof(xpsFdbEntry_t));
    if (XPS_FDB_ENTRY_TYPE_MAC_ADDR_E == fdbEntryType)
    {
        cpssOsMemCpy(&fdbEntry->macAddr,
                     &dxChMacEntryPtr->format.fdbEntryMacAddrFormat.macAddr,
                     sizeof(dxChMacEntryPtr->format.fdbEntryMacAddrFormat.macAddr));
        fdbEntry->vlanId = dxChMacEntryPtr->format.fdbEntryMacAddrFormat.fid;
    }

    if (dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.type ==
        CPSS_INTERFACE_PORT_E)
    {
        fdbEntry->intfId =
            dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.portNum;
        if (!IS_EPORT(fdbEntry->intfId))
        {
            xpsStatus = cpssHalL2GPortNum(devId, fdbEntry->intfId, &frontPanelNum);
            if (xpsStatus != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Interface Id to frontPanel port Number conversion failed");
            }
            fdbEntry->intfId = frontPanelNum;
        }
    }
    else if (dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.type ==
             CPSS_INTERFACE_TRUNK_E)
    {
        fdbEntry->intfId = xpsUtilCpssToXpsInterfaceConvert(
                               dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.trunkId,
                               XPS_LAG);
    }

    if (dxChMacEntryPtr->format.fdbEntryMacAddrFormat.isStatic)
    {
        fdbEntry->isStatic = GT_TRUE;
    }

    if (dxChMacEntryPtr->format.fdbEntryMacAddrFormat.daRoute)
    {
        fdbEntry->isRouter = GT_TRUE;
    }

    status = cpssHalFdbMgrConvertDxChTablePktCmdToGenric(
                 dxChMacEntryPtr->format.fdbEntryMacAddrFormat.daCommand, &(fdbEntry->pktCmd));
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Conversion of packet cmd failed with cpss error code %d cmd :%d", status,
              dxChMacEntryPtr->format.fdbEntryMacAddrFormat.daCommand);
        return status;
    }

    /* check if entry is tunnel entry and convert to generic L2 params */
    status = cpssHalFdbMgrConvertFromHwTunnelEntry(fdbEntry);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "No convertion from Tunnel entry");
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Converted from Tunnel entry");
    }

    return GT_OK;
}



/**
 * @internal cpssHalFdbMgrConvertCpssToGenericMacEntry function
 * @endinternal
 *
 * @brief   Convert device specific MAC entry into generic MAC entry
 *
 * @param[in] devId                    - device Id
 * @param[in] dxChMacEntryPtr          - (pointer to) cpss MAC entry parameters
 *
 * @param[in] fdbEntryType      - FdbEntry type such as IPv host
 *       unicast entry, or layer 2 MAC Entry.
 * @param[out] macEntryPtr              - (pointer to) MAC entry parameters
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong parameters
 */

GT_STATUS cpssHalFdbMgrConvertCpssToGenericMacEntry
(
    int                                                  devId,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC        *dxChMacEntryPtr,
    XPS_FDB_ENTRY_TYPE_ENT                               fdbEntryType,
    void                                                 *macEntryPtr
)
{
    GT_STATUS   status = GT_NOT_SUPPORTED;
    /* convert entry type from device specific format */
    switch (fdbEntryType)
    {
        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_E:
            status = cpssHalFdbMgrConvertCpssToGenricL2MactEntry(devId, dxChMacEntryPtr,
                                                                 fdbEntryType, macEntryPtr);
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_UC_E:
        case XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E:
        case XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E:
            status = cpssHalFdbMgrConvertCpssToL3HostMacEntry(devId, dxChMacEntryPtr,
                                                              fdbEntryType, macEntryPtr);
            break;

        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_FID_VID1_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_FID_VID1_E:
            break;

        default:
            return GT_BAD_PARAM;
    }

    return status;
}



/*
* @internal cpssHalFdbMgrLearningScan function
* @endinternal
*
* @brief   The function gets the AUQ messages according to the input attributes.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] paramsPtr             - the learning scan attributes.
* @param[out] entriesLearningArray - (pointer to) Update event array.
* @param[out] entriesLearningNumPtr- (pointer to) Number of Update events filled in array.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalFdbMgrLearningScan
(
    GT_U32                                             fdbManagerId,
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC learningParams,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC   entriesLearningArray[],
    GT_U32                                             *entriesLearningNumPtr
)
{
    GT_STATUS status = GT_OK;

    learningParams.addNewMacUcEntries = GT_TRUE;
    learningParams.addWithRehashEnable = GT_TRUE;
    learningParams.updateMovedMacUcEntries = GT_TRUE;

    status = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &learningParams,
                                               entriesLearningArray, entriesLearningNumPtr);
    if (status != GT_NO_MORE && status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Getting entries from HW failed with cpss error code %d", status);
        return status;
    }

    return status;
}


/**
* @internal cpssHalFdbMgrConvertGenericToDxChMacEntryKey function
* @endinternal
*
* @brief   Convert generic MAC entry key into device specific MAC entry key
*
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryKeyPtr           - (pointer to) MAC entry key parameters
* @param[in] macEntryKeyPtr           - (pointer to) MAC entry
*        Type
*
* @param[out] dxChMacEntryKeyPtr       - (pointer to) DxCh MAC entry key parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertGenericToDxChMacEntryKey
(
    int                                 devId,
    XPS_FDB_ENTRY_TYPE_ENT              fdbEntryType,
    void                                *macEntryKeyPtr,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC *dxChMacEntryKeyPtr
)
{
    cpssOsMemSet(dxChMacEntryKeyPtr, 0, sizeof(*dxChMacEntryKeyPtr));

    /* convert entry type into device specific format */
    switch (fdbEntryType)
    {
        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_E:
            dxChMacEntryKeyPtr->fdbEntryType =
                CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            dxChMacEntryKeyPtr->fdbEntryType =
                CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            dxChMacEntryKeyPtr->fdbEntryType =
                CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E;
            break;
        case XPS_FDB_ENTRY_TYPE_IPV4_UC_E:
            dxChMacEntryKeyPtr->fdbEntryType =
                CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E:
        case XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E:
            dxChMacEntryKeyPtr->fdbEntryType =
                CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert key data into device specific format */
    if (XPS_FDB_ENTRY_TYPE_MAC_ADDR_E == fdbEntryType ||
        XPS_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E == fdbEntryType)
    {
        xpsFdbEntry_t *fdbEntryPtr = (xpsFdbEntry_t *)macEntryKeyPtr;

        cpssOsMemCpy(dxChMacEntryKeyPtr->format.fdbEntryMacAddrFormat.macAddr.arEther,
                     &(fdbEntryPtr->macAddr),
                     sizeof(dxChMacEntryKeyPtr->format.fdbEntryMacAddrFormat.macAddr));

        dxChMacEntryKeyPtr->format.fdbEntryMacAddrFormat.fid = fdbEntryPtr->vlanId;
    }

    else
    {
        if (XPS_FDB_ENTRY_TYPE_IPV4_UC_E == fdbEntryType)
        {
            xpsL3HostEntry_t *l3HostEntryPtr = (xpsL3HostEntry_t *)macEntryKeyPtr;

            cpssOsMemCpy(&dxChMacEntryKeyPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP,
                         &l3HostEntryPtr->ipv4Addr,
                         sizeof(dxChMacEntryKeyPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP));

            dxChMacEntryKeyPtr->format.fdbEntryIpv4UcFormat.vrfId = l3HostEntryPtr->vrfId;

        }
        else
        {
            if ((XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E == fdbEntryType) ||
                (XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E == fdbEntryType))
            {
                xpsL3HostEntry_t *l3HostEntryPtr = (xpsL3HostEntry_t *)macEntryKeyPtr;

                cpssOsMemCpy(&dxChMacEntryKeyPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP,
                             &l3HostEntryPtr->ipv6Addr,
                             sizeof(dxChMacEntryKeyPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP));

                dxChMacEntryKeyPtr->format.fdbEntryIpv6UcFormat.vrfId = l3HostEntryPtr->vrfId;
            }

            else
            {
                // MULTICAST STUFF
            }
        }
    }

    return GT_OK;
}


/**
* @internal cpssHalFdbMgrConvertXpsFdbEntryTypeToDxChMacEntryType function
* @endinternal
*
* @brief   return the FDB Mac
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] fdbEntryType           - XPS FDB Entry Type
*
* @param[out] dxChMacEntryType          - (pointer to)
*       dxChMacEntry Type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertXpsFdbEntryTypeToDxChMacEntryType
(
    GT_U8 fdbEntryType,
    GT_U8 *dxChMacEntryType
)
{
    if (dxChMacEntryType == NULL)
    {
        return GT_BAD_PARAM;
    }

    switch (fdbEntryType)
    {
        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_E:
            *dxChMacEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            *dxChMacEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            *dxChMacEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_UC_E:
            *dxChMacEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E:
        case XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E:
            *dxChMacEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E;
            break;

        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E:
        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_FID_VID1_E:
        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_FID_VID1_E:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "This type is not mapped to cpss type");
            break;

        default:
            return GT_BAD_PARAM;

    }

    return GT_OK;
}


/**
* @internal cpssHalFdbMgrConvertPktCmdToDxChFdbTableDaSaCmd
* @endinternal
*
* @brief   Convert generic pkt command to device specific packet command
*
* @param[in] pktCmd                     - generic acket command
* @param[out] *cpssMacTableCmd          - (pointer to) DxCh MAC table command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertPktCmdToDxChFdbTableDaSaCmd
(
    xpPktCmd_e                        pktCmd,
    CPSS_PACKET_CMD_ENT               *cpssMacTableCmd
)
{
    switch (pktCmd)
    {
        case XP_PKTCMD_FWD:
            *cpssMacTableCmd = CPSS_PACKET_CMD_FORWARD_E;
            break;

        case XP_PKTCMD_DROP:
            *cpssMacTableCmd = CPSS_PACKET_CMD_DROP_HARD_E;
            break;

        case XP_PKTCMD_TRAP:
            *cpssMacTableCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;

        case XP_PKTCMD_FWD_MIRROR:
            *cpssMacTableCmd = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}



GT_STATUS cpssHalFdbMgrConvertToHwTunnelEntry(xpsFdbEntry_t *fdbEntry)
{
    XP_STATUS   result    = XP_NO_ERR;
    xpsDevice_t devId     = 0;
    xpsVlan_t   hwEVlanId = 0;
    uint32_t    ePort     = 0;

    if (fdbEntry->vlanId)
    {
        if ((result = xpsVxlanGetTunnelHwEVlanId(devId, fdbEntry->vlanId,
                                                 &hwEVlanId)) == XP_NO_ERR)
        {
            if (!hwEVlanId)
            {
                return GT_FAIL;
            }
            if (fdbEntry->intfId)
            {
                if ((result = xpsVxlanGetTunnelEPort(devId, fdbEntry->intfId,
                                                     &ePort)) == XP_NO_ERR)
                {
                    fdbEntry->vlanId = hwEVlanId;
                    fdbEntry->intfId = ePort;
                    return GT_OK;
                }
            }
        }
    }

    return GT_FAIL;
}

/**
* @internal cpssHalFdbMgrConvertL2MactEntryToDxChMacEntry function
* @endinternal
*
* @brief   Convert generic MAC entry into device specific MAC entry
*
* @param[in] devId                   - device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @param[out] dxChMacEntryPtr          - (pointer to) DxCh MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertL2MactEntryToDxChMacEntry
(
    int                                      devId,
    XPS_FDB_ENTRY_TYPE_ENT                   fdbEntryType,
    void                                     *macEntryPtr,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC      *dxChMacEntryPtr
)
{
    GT_STATUS status = GT_OK;
    xpsFdbEntry_t *fdbEntry  = (xpsFdbEntry_t *)macEntryPtr;
    XP_STATUS xpsStatus = XP_NO_ERR;
    xpsInterfaceType_e type;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;

    cpssOsMemSet(dxChMacEntryPtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));

    /* convert entry type into device specific format */
    status = cpssHalFdbMgrConvertXpsFdbEntryTypeToDxChMacEntryType(fdbEntryType,
                                                                   (GT_U8 *) &dxChMacEntryPtr->fdbEntryType);

    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Conversion to dxCh structure failed with cpss error code %d", status);
        return status;
    }

    if (XPS_FDB_ENTRY_TYPE_MAC_ADDR_E == fdbEntryType)
    {
        cpssOsMemCpy(dxChMacEntryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther,
                     fdbEntry->macAddr,
                     sizeof(dxChMacEntryPtr->format.fdbEntryMacAddrFormat.macAddr));

        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.fid = fdbEntry->vlanId;
    }

    xpsStatus = xpsInterfaceGetType(fdbEntry->intfId, &type);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Getting interface type failed");
        return GT_FAIL;
    }

    /*if it is  physical port*/
    if (type == XPS_PORT)
    {
        cpssDevId = xpsGlobalIdToDevId(devId, fdbEntry->intfId);
        cpssPortNum = xpsGlobalPortToPortnum(devId, fdbEntry->intfId);
        if ((cpssDevId == 0xff) || (cpssPortNum == 0xffff))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Invalid Device/Port Id :%d : %d  \n", devId, fdbEntry->intfId);
            return XP_ERR_INVALID_ARG;
        }
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.portNum
            = cpssPortNum;
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.hwDevNum
            = cpssDevId;
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.type =
            CPSS_INTERFACE_PORT_E;
    }

    else if (type == XPS_LAG) /*if it is lag*/
    {
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.trunkId
            =
                (GT_TRUNK_ID)xpsUtilXpstoCpssInterfaceConvert(fdbEntry->intfId, type);
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.type =
            CPSS_INTERFACE_TRUNK_E;
    }

    else if (type == XPS_VLAN_ROUTER)
    {
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.fid = fdbEntry->vlanId;
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.type =
            CPSS_INTERFACE_VID_E;
    }

    else if (type == XPS_TUNNEL_VXLAN)
    {
        /* check if entry is tunnel entry and convert to generic L2 params */
        status = cpssHalFdbMgrConvertToHwTunnelEntry(fdbEntry);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "No convertion to Tunnel entry");
            return GT_FAIL;
        }

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Converted to Tunnel entry");

        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.fid = fdbEntry->vlanId;
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.portNum
            = fdbEntry->intfId;
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.hwDevNum
            = devId;
    }

    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Unsupported type");
        return GT_BAD_VALUE;
    }

    if (fdbEntry->isStatic)
    {
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.isStatic = GT_TRUE;
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.age = GT_FALSE;
    }

    /*to mark router mac entry*/
    if (fdbEntry->isRouter)
    {
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.daRoute = GT_TRUE;
    }
    if (fdbEntry->isControl)
    {
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.appSpecificCpuCode = GT_TRUE;
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.portNum
            = CPSS_CPU_PORT_NUM_CNS;
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.hwDevNum
            = 0;
        dxChMacEntryPtr->format.fdbEntryMacAddrFormat.dstInterface.type =
            CPSS_INTERFACE_PORT_E;
    }
    /*copy DA command*/
    /* convert daCommand info into device specific format */
    status = cpssHalFdbMgrConvertPktCmdToDxChFdbTableDaSaCmd(fdbEntry->pktCmd,
                                                             &(dxChMacEntryPtr->format.fdbEntryMacAddrFormat.daCommand));
    if (status != GT_OK)
    {
        return status;
    }

    return status;
}


/**
* @internal cpssHalFdbMgrConvertL3HostEntryToDxChMacEntry function
* @endinternal
*
* @brief   Convert generic MAC entry into device specific MAC entry
*
* @param[in] devId                   - device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @param[out] dxChMacEntryPtr          - (pointer to) DxCh MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertL3HostEntryToDxChMacEntry
(
    int                                      devId,
    XPS_FDB_ENTRY_TYPE_ENT                   fdbEntryType,
    void                                     *macEntryPtr,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC      *dxChMacEntryPtr
)
{
    GT_STATUS status = GT_OK;
    XP_STATUS xpsStatus = XP_NO_ERR;
    xpsArpPointer_t arpPointer;
    xpsInterfaceType_e l3IntfType;
    xpsInterfaceType_e egressIntfType;
    GT_U8 cpssDevId = 0xff;
    GT_U32 cpssPortNum = 0xffffffff;
    xpsInterfaceInfo_t *intfInfo = NULL;

    xpsL3HostEntry_t *l3hostEntry = (xpsL3HostEntry_t *)macEntryPtr;
    cpssOsMemSet(dxChMacEntryPtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));
    /*Get arp Pointer from SW DB*/
    xpsStatus = xpsArpGetArpPointer(devId, l3hostEntry->nhEntry.nextHop.macDa,
                                    &arpPointer);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed in xpsArpGetArpPointer: %d\n", xpsStatus);
        return GT_FAIL;
    }

    /* Get the type of the router interface */
    xpsStatus = xpsInterfaceGetType(l3hostEntry->nhEntry.nextHop.l3InterfaceId,
                                    &l3IntfType);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed in xpsInterfaceGetType: %d\n", xpsStatus);
        return GT_FAIL;
    }

    /* Get the type of the router interface */
    xpsStatus = xpsInterfaceGetInfo(l3hostEntry->nhEntry.nextHop.l3InterfaceId,
                                    &intfInfo);
    //If interface is not found, then intfInfo can be NULL
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed in xpsInterfaceGetInfo: %d\n", xpsStatus);
        return GT_FAIL;
    }

    if (NULL == intfInfo)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid xpsInterfaceGetInfo: %d\n", xpsStatus);
        return GT_FAIL;
    }

    l3IntfType = intfInfo->type;

    /* This Neighbor is received but arp is not learnt yet for ROUTER VLAN. Program Neighbor entry with drop interface */
    if (l3hostEntry->nhEntry.nextHop.egressIntfId != XPS_INTF_INVALID_ID)
    {
        /* Get the type of the egress interface */
        xpsStatus = xpsInterfaceGetType(l3hostEntry->nhEntry.nextHop.egressIntfId,
                                        &egressIntfType);
        if (xpsStatus != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed in xpsInterfaceGetType for egress Interface(%d) err %d\n",
                  l3hostEntry->nhEntry.nextHop.egressIntfId, xpsStatus);
            return GT_FAIL;
        }

        if (egressIntfType == XPS_PORT)
        {
            cpssDevId = xpsGlobalIdToDevId(devId,
                                           l3hostEntry->nhEntry.nextHop.egressIntfId);
            cpssPortNum = xpsGlobalPortToPortnum(devId,
                                                 l3hostEntry->nhEntry.nextHop.egressIntfId);

            if ((cpssDevId == 0xff) || (cpssPortNum == 0xffff))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Invalid Device/Port Id :%d : %d  \n", devId,
                      l3hostEntry->nhEntry.nextHop.egressIntfId);
                return XP_ERR_INVALID_ARG;
            }
        }
    }

    /* convert entry type into device specific format */
    status = cpssHalFdbMgrConvertXpsFdbEntryTypeToDxChMacEntryType(fdbEntryType,
                                                                   (GT_U8 *) &dxChMacEntryPtr->fdbEntryType);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed in cpssHalConvertXpsFdbEntryTypeToDxChMacEntryType: %d\n", status);
        return status;
    }

    if (XPS_FDB_ENTRY_TYPE_IPV4_UC_E == fdbEntryType)
    {
        cpssOsMemCpy(&dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP,
                     &l3hostEntry->ipv4Addr,
                     sizeof(dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP));

        dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.vrfId = l3hostEntry->vrfId;

        if ((l3hostEntry->nhEntry.pktCmd != XP_PKTCMD_DROP) &&
            (l3hostEntry->nhEntry.nextHop.egressIntfId != XPS_INTF_INVALID_ID))
        {
            /* Populate CPSS parameters according to the egress L3 interface type */
            switch (l3IntfType)
            {
                case XPS_PORT_ROUTER:
                case XPS_SUBINTERFACE_ROUTER:
                case XPS_VLAN_ROUTER:
                    {
                        if (egressIntfType == XPS_PORT)
                        {
                            dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.type
                                = CPSS_INTERFACE_PORT_E;
                            dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum
                                = (GT_PORT_NUM)cpssPortNum;
                            dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.devPort.hwDevNum
                                = (GT_HW_DEV_NUM)cpssDevId;
                        }
                        else if (egressIntfType == XPS_LAG)
                        {
                            dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.type
                                = CPSS_INTERFACE_TRUNK_E;
                            dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.trunkId
                                = (GT_TRUNK_ID)xpsUtilXpstoCpssInterfaceConvert(
                                      l3hostEntry->nhEntry.nextHop.egressIntfId, egressIntfType);
                        }
                        else if (egressIntfType == XPS_VLAN_ROUTER)
                        {
                            dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.type
                                = CPSS_INTERFACE_VIDX_E;
                            dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.vidx
                                = 0xfff;
                        }
                        else
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Invalid egress interface type : Egress Interface: \n");
                            return XP_ERR_INVALID_ARG;
                        }
                        break;
                    }
                default:
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Unsupported L3 interface type received \n");
                        return XP_ERR_INVALID_ARG;
                    }
            }
        }

        /*Workaround: To support pkt command per Fdb-host based IPv4/6 Unicast,
          TODO - - USE APPROPRIATE MACRO FOR NULL PORT FOR FALCON*/

        if (l3hostEntry->nhEntry.pktCmd == XP_PKTCMD_DROP)
        {
            /* assign to the dropped port*/
            dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum
                = CPSS_NULL_PORT_NUM_CNS;
        }
        else if (l3hostEntry->nhEntry.pktCmd != XP_PKTCMD_FWD)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "L3 Host Entry Next Hop entry pktCmd is not FWD return Invalid\n");
            return XP_ERR_INVALID_ARG;
        }

        dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.ttlHopLimDecOptionsExtChkByPass
            = GT_FALSE;
        dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.ttlHopLimitDecEnable
            = GT_TRUE;
        dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.countSet =
            CPSS_IP_CNT_SET0_E;
        dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.ICMPRedirectEnable
            = GT_FALSE;

        /* Falcon FDB-based host entry accepts only profile index 0 or 1 (single bit). If interface has MTU with other profiles,
         * * we can throw an error or just reset the MTU to profile 0*/
        if (intfInfo->mtuPorfileIndex > 1)
        {
            //TODO: Throw warning error.
            dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.mtuProfileIndex
                = MTU_L3_PROFILE_INDEX0;
        }
        else
        {
            dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.mtuProfileIndex
                =  intfInfo->mtuPorfileIndex;
        }
        dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteType =
            CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
        dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.pointer.arpPointer
            = arpPointer;
        dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.age = GT_FALSE;

        if (l3IntfType == XPS_VLAN_ROUTER)
        {
            dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.nextHopVlanId
                = XPS_INTF_MAP_INTF_TO_BD(l3hostEntry->nhEntry.nextHop.l3InterfaceId);
        }
        else if (l3IntfType == XPS_PORT_ROUTER)
        {
            dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.nextHopVlanId
                = intfInfo->bdHWId;
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Not supported L3 Type :%d \n", l3IntfType);
            return XP_ERR_NOT_SUPPORTED;
        }
    }
    else if ((XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E == fdbEntryType) ||
             (XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E == fdbEntryType))
    {
        cpssOsMemCpy(&dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP,
                     &l3hostEntry->ipv6Addr,
                     sizeof(dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP));

        dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.vrfId = l3hostEntry->vrfId;

        if ((l3hostEntry->nhEntry.pktCmd != XP_PKTCMD_DROP) &&
            (l3hostEntry->nhEntry.nextHop.egressIntfId != XPS_INTF_INVALID_ID))
        {
            /* Populate CPSS parameters according to the egress L3 interface type */
            switch (l3IntfType)
            {
                case XPS_PORT_ROUTER:
                case XPS_SUBINTERFACE_ROUTER:
                case XPS_VLAN_ROUTER:
                    {
                        if (egressIntfType == XPS_PORT)
                        {
                            dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.type
                                = CPSS_INTERFACE_PORT_E;
                            dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum
                                = (GT_PORT_NUM)cpssPortNum;
                            dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.devPort.hwDevNum
                                = (GT_HW_DEV_NUM)cpssDevId;
                        }
                        else if (egressIntfType == XPS_LAG)
                        {
                            dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.type
                                = CPSS_INTERFACE_TRUNK_E;

                            dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.trunkId
                                = (GT_TRUNK_ID)xpsUtilXpstoCpssInterfaceConvert(
                                      l3hostEntry->nhEntry.nextHop.egressIntfId, egressIntfType);

                        }
                        else if (egressIntfType == XPS_VLAN_ROUTER)
                        {
                            dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.type
                                = CPSS_INTERFACE_VIDX_E;
                            dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.vidx
                                = 0xfff;
                        }
                        else
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Invalid egress interface type : Egress Interface: \n");
                            return XP_ERR_INVALID_ARG;
                        }
                        break;
                    }
                default:
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Unsupported L3 interface type received \n");
                        return XP_ERR_INVALID_ARG;
                    }
            }
        }

        /*Workaround: To support pkt command per Fdb-host based IPv4/6 Unicast,
          TODO - - USE APPROPRIATE MACRO FOR NULL PORT FOR FALCON*/

        if (l3hostEntry->nhEntry.pktCmd == XP_PKTCMD_DROP)
        {
            /* assign to the dropped port*/
            dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum
                = CPSS_NULL_PORT_NUM_CNS;
        }
        else if (l3hostEntry->nhEntry.pktCmd != XP_PKTCMD_FWD)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "L3 Host Entry Next Hop entry pktCmd is not FWD return Invalid\n");
            return XP_ERR_INVALID_ARG;
        }

        /* Falcon FDB-based host entry accepts only profile index 0 or 1 (single bit). If interface has MTU with other profiles
         *  we can throw an error or just reset the MTU to profile 0
         *  */

        if (intfInfo->mtuPorfileIndex > 1)
        {
            //TODO: Throw warning error.
            dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.mtuProfileIndex
                = MTU_L3_PROFILE_INDEX0;
        }
        else
        {
            dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.mtuProfileIndex
                = intfInfo->mtuPorfileIndex;
        }

        dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.ttlHopLimDecOptionsExtChkByPass
            = GT_FALSE;
        dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.ttlHopLimitDecEnable
            = GT_TRUE;
        dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.countSet =
            CPSS_IP_CNT_SET0_E;
        dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.ICMPRedirectEnable
            = GT_FALSE;
        dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteType =
            CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
        dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.pointer.arpPointer
            = arpPointer;
        dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.age = GT_FALSE;
        dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.ipv6ExtInfo.siteId
            = CPSS_IP_SITE_ID_INTERNAL_E;

        if (l3IntfType == XPS_VLAN_ROUTER)
        {
            dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.nextHopVlanId
                = XPS_INTF_MAP_INTF_TO_BD(l3hostEntry->nhEntry.nextHop.l3InterfaceId);
        }
        else if (l3IntfType == XPS_PORT_ROUTER)
        {
            dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.nextHopVlanId
                = intfInfo->bdHWId;
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Not supported L3 Type :%d \n", l3IntfType);
            return XP_ERR_NOT_SUPPORTED;
        }
    }

    return GT_OK;
}


/**
* @internal cpssHalFdbMgrConvertGenericToDxChMacEntry function
* @endinternal
*
* @brief   Convert generic MAC entry into device specific MAC entry
*
* @param[in] devId                   - device Id
* @param[in] fdbEntryType            - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @param[out] dxChMacEntryPtr          - (pointer to) DxCh MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertGenericToDxChMacEntry
(
    int                                  devId,
    XPS_FDB_ENTRY_TYPE_ENT               fdbEntryType,
    void                                 *macEntryPtr,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  *dxChMacEntryPtr
)
{
    GT_STATUS status = GT_OK;

    /* call specific format function to convert to device specific format */
    switch (fdbEntryType)
    {
        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_E:
            status = cpssHalFdbMgrConvertL2MactEntryToDxChMacEntry(devId, fdbEntryType,
                                                                   macEntryPtr, dxChMacEntryPtr);
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_UC_E:
        case XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E:
        case XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E:
            status = cpssHalFdbMgrConvertL3HostEntryToDxChMacEntry(devId, fdbEntryType,
                                                                   macEntryPtr, dxChMacEntryPtr);
            break;
        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_FID_VID1_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_FID_VID1_E:
            break;

        default:
            return GT_BAD_PARAM;
    }

    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Conversion from L2 Mac to cpss mac entry failed with cpss error code %d",
              status);
        return status;
    }

    return status;
}


/**
* @internal cpssHalFdbMgrMacEntryFind function
* @endinternal
*
* @brief   function to find an existing entry that match the key.
*
* @param devId                       - Device Id
* @param[in] fdbEntryType            - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr           - (pointer to) mac entry key
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found, but bucket is not FULL
* @retval GT_FULL                  - entry not found, but bucket is FULL
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssHalFdbMgrMacEntryFind
(
    int                     devId,
    GT_U32                  fdbManagerId,
    XPS_FDB_ENTRY_TYPE_ENT  fdbEntryType,
    void                    *macEntryKeyPtr
)
{
    GT_STATUS status = GT_OK;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC entryGet;

    status = cpssHalFdbMgrConvertGenericToDxChMacEntryKey(devId, fdbEntryType,
                                                          macEntryKeyPtr, &entryGet);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Conversion to key format failed with cpss error code %d", status);
        return status;
    }

    status = cpssDxChBrgFdbManagerEntryGet(fdbManagerId, &entryGet);
    if (status != GT_OK && status!= GT_NOT_FOUND)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalFdbMgrMacEntryFind failed with cpss error code %d", status);
        return status;
    }

    if (status == GT_NOT_FOUND)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "cpssHalFdbMgrMacEntryFind -- Entry not found");
        return status;
    }

    return status;
}


/**
* @internal cpssHalFdbMgrMacEntryWrite function
* @endinternal
*
* @brief   Write the new entry in Hardware MAC address table in
*          specified index
*
* @param[in] devId                    - device Id
* @param[in] fdbEntryType             - fdbEntryType
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,saCommand,daCommand
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalFdbMgrMacEntryWrite
(
    int                           devId,
    GT_U32                        fdbManagerId,
    XPS_FDB_ENTRY_TYPE_ENT        fdbEntryType,
    void                          *macEntryPtr
)
{
    xpsLockTake(XP_LOCKINDEX_XPS_FDB_LOCK);

    GT_STATUS                             status = GT_OK;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC   dxChMacEntryPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   paramsInfo;

    cpssOsMemSet(&paramsInfo, 0, sizeof(paramsInfo));
    cpssOsMemSet(&dxChMacEntryPtr, 0, sizeof(dxChMacEntryPtr));

    status = cpssHalFdbMgrConvertGenericToDxChMacEntry(devId, fdbEntryType,
                                                       macEntryPtr, &dxChMacEntryPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Conversion from L2 Mac to cpss mac entry failed with cpss error code %d",
              status);
        xpsLockRelease(XP_LOCKINDEX_XPS_FDB_LOCK);
        return status;
    }

    /* Set the flag to support Application Specific CPU CODE Assignment
     * appSpecificCpuCode is valid  for MAC/IP-MC Entry format.
     * TODO: When IP local host entries moved to FDB, revisit this part on
     * how to update appSpecificCpuCode.
     */
    if (fdbEntryType == XPS_FDB_ENTRY_TYPE_MAC_ADDR_E)
    {
        dxChMacEntryPtr.format.fdbEntryMacAddrFormat.appSpecificCpuCode = GT_TRUE;
    }

    paramsInfo.rehashEnable = GT_TRUE;

    status = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId, &dxChMacEntryPtr,
                                           &paramsInfo);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Entry Add failed with cpss error code %d", status);
        xpsLockRelease(XP_LOCKINDEX_XPS_FDB_LOCK);
        return status;
    }

    xpsLockRelease(XP_LOCKINDEX_XPS_FDB_LOCK);
    return status;
}



/*
* @internal cpssHalFdbMgrMacEntryDelete function
* @endinternal
*
* @brief   This function delete entry from the CPSS FDB Manager's database and HW.
*          All types of entries can be deleted using this API.
*          NOTE: the FDB manager will remove the info from all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format to be deleted.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - the entry was not found.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS cpssHalFdbMgrMacEntryDelete
(
    int                     devId,
    GT_U32                  fdbManagerId,
    XPS_FDB_ENTRY_TYPE_ENT  fdbEntryType,
    void                    *macEntryKeyPtr
)
{
    GT_STATUS status = GT_OK;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC dxChMacEntry;

    status = cpssHalFdbMgrConvertGenericToDxChMacEntryKey(devId, fdbEntryType,
                                                          macEntryKeyPtr, &dxChMacEntry);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Conversion to key format failed with cpss error code %d", status);
        return status;
    }
#if 0
    status = cpssDxChBrgFdbManagerEntryGet(fdbManagerId, &dxChMacEntry);
    if (status != GT_OK && status!= GT_NOT_FOUND)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalFdbMgrMacEntryFind failed with cpss error code %d", status);
        return status;
    }

    if (status == GT_NOT_FOUND)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "cpssHalFdbMgrMacEntryFind -- Entry not found");
        return status;
    }
#endif
    status = cpssDxChBrgFdbManagerEntryDelete(fdbManagerId, &dxChMacEntry);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Delete entry failed with cpss error code %d", status);
        return status;
    }

    return status;
}



/**
* @internal cpssHalFdbMgrConvertCpssToL3HostMacEntry function
* @endinternal
*
* @brief   Convert device specific MAC entry into generic MAC entry
*
* @param[in] devId                    - device Id
* @param[in] dxChMacEntryPtr          - (pointer to) cpss MAC entry parameters
*
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[out] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertCpssToL3HostMacEntry
(
    int                                  devId,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  *dxChMacEntryPtr,
    XPS_FDB_ENTRY_TYPE_ENT               fdbEntryType,
    void                                 *macEntryPtr
)
{
    XP_STATUS xpsStatus = XP_NO_ERR;
    GT_U32 arpPointer;
    xpsScope_t scopeId;
    xpsInterfaceId_t trunkId;
    xpsInterfaceInfo_t *intfInfo;

    xpsL3HostEntry_t *l3HostEntry = (xpsL3HostEntry_t *) macEntryPtr;
    cpssOsMemSet(l3HostEntry, 0, sizeof(xpsL3HostEntry_t));

    if (fdbEntryType == XPS_FDB_ENTRY_TYPE_IPV4_UC_E)
    {
        arpPointer =
            dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.pointer.arpPointer;
    }
    else
    {
        arpPointer =
            dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.pointer.arpPointer;
    }
    /*Get arp Pointer from SW DB*/
    xpsStatus = xpsIpRouterArpAddRead(devId, arpPointer,
                                      &l3HostEntry->nhEntry.nextHop.macDa);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Arp address read failed with xps error code %d", xpsStatus);
        return GT_FAIL;
    }

    /* TODO -- Packet cmd is not supported in fdb manager - if packet action is forward, it forwards with valid egress intf*/
    /* If it is TRAP, we need to assign appropraite CPU port? -- doubt*/

    if (CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E ==
        dxChMacEntryPtr->fdbEntryType)
    {
        cpssOsMemCpy(&l3HostEntry->ipv4Addr,
                     &dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP,
                     sizeof(dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP));
        l3HostEntry->vrfId = dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.vrfId;
        l3HostEntry->type = XP_PREFIX_TYPE_IPV4;

        if (dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum
            != CPSS_NULL_PORT_NUM_CNS)
        {
            switch (dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.type)
            {
                case CPSS_INTERFACE_PORT_E:
                    l3HostEntry->nhEntry.nextHop.egressIntfId =
                        dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum;
                    break;
                case CPSS_INTERFACE_TRUNK_E:
                    trunkId = (xpsInterfaceId_t)(
                                  dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.trunkId);

                    l3HostEntry->nhEntry.nextHop.egressIntfId = xpsUtilCpssToXpsInterfaceConvert(
                                                                    trunkId, XPS_LAG);
                    break;

                /* This case can hit when Dst Intf is not resolved and
                   FDB Unicast miss action set to FWD
                 */

                case CPSS_INTERFACE_VID_E:
                case CPSS_INTERFACE_VIDX_E:

                    l3HostEntry->nhEntry.nextHop.egressIntfId = XPS_INTF_MAP_BD_TO_INTF(
                                                                    dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.nextHopVlanId);
                    break;


                default:
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Unsupported L3 interface type received");
                        return XP_ERR_INVALID_ARG;
                    }
            }

            xpsStatus = xpsScopeGetScopeId(devId, &scopeId);
            if (xpsStatus != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get scope for devId : %d\n", devId);
                return xpsStatus;
            }

            xpsStatus = xpsInterfaceGetInfoScope(scopeId,
                                                 l3HostEntry->nhEntry.nextHop.egressIntfId, &intfInfo);
            if (xpsStatus != XP_NO_ERR || (intfInfo == NULL))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get mapped L3 interface with port inteface %d\n",
                      l3HostEntry->nhEntry.nextHop.egressIntfId);
                return xpsStatus;
            }

            if (dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.nextHopVlanId
                < XPS_L3_RESERVED_ROUTER_VLAN(devId))
            {
                l3HostEntry->nhEntry.nextHop.l3InterfaceId = XPS_INTF_MAP_BD_TO_INTF(
                                                                 dxChMacEntryPtr->format.fdbEntryIpv4UcFormat.ucRouteInfo.fullFdbInfo.nextHopVlanId);
            }
            else
            {
                l3HostEntry->nhEntry.nextHop.l3InterfaceId =
                    intfInfo->intfMappingInfo.keyIntfId;
            }
        }
        else
        {
            l3HostEntry->nhEntry.nextHop.egressIntfId = XPS_INTF_INVALID_ID;
            l3HostEntry->nhEntry.nextHop.l3InterfaceId = XPS_INTF_INVALID_ID;
            l3HostEntry->nhEntry.pktCmd = XP_PKTCMD_DROP;
        }

    }

    else if ((CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E ==
              dxChMacEntryPtr->fdbEntryType))
    {
        cpssOsMemCpy(&l3HostEntry->ipv6Addr,
                     &dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP,
                     sizeof(dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP));
        l3HostEntry->vrfId = dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.vrfId;
        l3HostEntry->type = XP_PREFIX_TYPE_IPV6;

        if (dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum
            != CPSS_NULL_PORT_NUM_CNS)
        {
            switch (dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.type)
            {
                case CPSS_INTERFACE_PORT_E:
                    l3HostEntry->nhEntry.nextHop.egressIntfId =
                        dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum;
                    break;
                case CPSS_INTERFACE_TRUNK_E:
                    trunkId = (xpsInterfaceId_t)(
                                  dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.trunkId);

                    l3HostEntry->nhEntry.nextHop.egressIntfId = xpsUtilCpssToXpsInterfaceConvert(
                                                                    trunkId, XPS_LAG);
                    break;

                /* This case can hit when Dst Intf is not resolved and
                   FDB Unicast miss action set to FWD
                 */

                case CPSS_INTERFACE_VID_E:
                case CPSS_INTERFACE_VIDX_E:

                    l3HostEntry->nhEntry.nextHop.egressIntfId = XPS_INTF_MAP_BD_TO_INTF(
                                                                    dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.nextHopVlanId);
                    break;

                default:
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Unsupported L3 interface type recieved");
                        return XP_ERR_INVALID_ARG;
                    }
            }

            xpsStatus = xpsScopeGetScopeId(devId, &scopeId);
            if (xpsStatus != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get scope for devId : %d\n", devId);
                return xpsStatus;
            }

            xpsStatus = xpsInterfaceGetInfoScope(scopeId,
                                                 l3HostEntry->nhEntry.nextHop.egressIntfId, &intfInfo);
            if (xpsStatus != XP_NO_ERR || (intfInfo == NULL))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get mapped L3 interface with port inteface %d\n",
                      l3HostEntry->nhEntry.nextHop.egressIntfId);
                return xpsStatus;
            }

            if (dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.nextHopVlanId
                < XPS_L3_RESERVED_ROUTER_VLAN(devId))
            {
                l3HostEntry->nhEntry.nextHop.l3InterfaceId = XPS_INTF_MAP_BD_TO_INTF(
                                                                 dxChMacEntryPtr->format.fdbEntryIpv6UcFormat.ucRouteInfo.fullFdbInfo.nextHopVlanId);
            }

            else
            {
                l3HostEntry->nhEntry.nextHop.l3InterfaceId =
                    intfInfo->intfMappingInfo.keyIntfId;
            }
        }

        else
        {
            l3HostEntry->nhEntry.nextHop.egressIntfId = XPS_INTF_INVALID_ID;
            l3HostEntry->nhEntry.nextHop.l3InterfaceId = XPS_INTF_INVALID_ID;
            l3HostEntry->nhEntry.pktCmd = XP_PKTCMD_DROP;
        }

    }

    return GT_OK;
}



/**
* @internal cpssHalFdbMgrMacEntryGet function
* @endinternal
*
* @brief   function to find an existing entry and convert to xps structure.
*
* @param devId                       - Device Id
* @param[in] fdbEntryType            - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr           - (pointer to) mac entry key
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found, but bucket is not FULL
* @retval GT_FULL                  - entry not found, but bucket is FULL
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssHalFdbMgrMacEntryGet
(
    int                     devId,
    GT_U32                  fdbManagerId,
    XPS_FDB_ENTRY_TYPE_ENT  fdbEntryType,
    void                    *macEntryKeyPtr
)
{
    GT_STATUS status = GT_OK;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC macEntry;

    status = cpssHalFdbMgrConvertGenericToDxChMacEntryKey(devId, fdbEntryType,
                                                          macEntryKeyPtr, &macEntry);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Conversion to key format failed with cpss error code %d", status);
        return status;
    }

    status = cpssDxChBrgFdbManagerEntryGet(fdbManagerId, &macEntry);
    if (status != GT_OK && status!= GT_NOT_FOUND)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalFdbMgrMacEntryFind failed with cpss error code %d", status);
        return status;
    }

    if (status == GT_NOT_FOUND)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "cpssHalFdbMgrMacEntryFind -- Entry not found");
        return status;
    }

    status = cpssHalFdbMgrConvertCpssToGenericMacEntry(devId, &macEntry,
                                                       fdbEntryType, (void*)macEntryKeyPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to convert cpss to xps mac entry type");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    return status;

}


/**
* @internal cpssHalFdbMgrMacEntryUpdate function
* @endinternal
*
* @brief   Updates the entry in Hardware MAC address table in
*          specified index
*
* @param[in] devId                    - device Id
* @param[in] fdbEntryType             - fdbEntryType
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,saCommand,daCommand
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalFdbMgrMacEntryUpdate
(
    int                           devId,
    GT_U32                        fdbManagerId,
    XPS_FDB_ENTRY_TYPE_ENT        fdbEntryType,
    void                          *macEntryPtr
)
{
    xpsLockTake(XP_LOCKINDEX_XPS_FDB_LOCK);

    GT_STATUS                             status = GT_OK;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC   dxChMacEntryPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC   paramsInfo;

    cpssOsMemSet(&paramsInfo, 0, sizeof(paramsInfo));
    cpssOsMemSet(&dxChMacEntryPtr, 0, sizeof(dxChMacEntryPtr));

    status = cpssHalFdbMgrConvertGenericToDxChMacEntry(devId, fdbEntryType,
                                                       macEntryPtr, &dxChMacEntryPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Conversion from L2 Mac to cpss mac entry failed with cpss error code %d",
              status);
        xpsLockRelease(XP_LOCKINDEX_XPS_FDB_LOCK);
        return status;
    }

    paramsInfo.updateOnlySrcInterface = GT_FALSE;

    status = cpssDxChBrgFdbManagerEntryUpdate(fdbManagerId, &dxChMacEntryPtr,
                                              &paramsInfo);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Entry Add failed with cpss error code %d", status);
        xpsLockRelease(XP_LOCKINDEX_XPS_FDB_LOCK);
        return status;
    }

    xpsLockRelease(XP_LOCKINDEX_XPS_FDB_LOCK);
    return status;
}

/**
* @internal cpssHalFdbMgrMacCountersGet function
* @endinternal
*
* @brief API for fetching FDB Manager Mac counters.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] *macCount            - FDB Manager total used mac Counts.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssHalFdbMgrMacCountersGet
(
    int                           devId,
    GT_U32                        fdbManagerId,
    GT_U32                        *macCount
)
{
    GT_STATUS                              status = GT_OK;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC countersPtr;

    cpssOsMemSet(&countersPtr, 0, sizeof(countersPtr));

    status = cpssDxChBrgFdbManagerCountersGet(fdbManagerId, &countersPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "FDB manager counters get failed with cpss error code %d", status);
        return status;
    }

    *macCount = countersPtr.macUnicastDynamic + countersPtr.macUnicastStatic +
                countersPtr.macMulticastDynamic + countersPtr.macMulticastStatic;

    return status;
}

/**
* @internal cpssHalFdbMgrBrgSecurBreachCommandSet function
* @endinternal
*
* @brief API for setting Bridge secure breach command set.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] eventType             - event type.
* @param[in] command               - packet command.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssHalFdbMgrBrgSecurBreachCommandSet
(
    int                                 devId,
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType,
    xpsPktCmd_e                         command
)
{
    GT_STATUS                          status = GT_OK;
    CPSS_PACKET_CMD_ENT                cpssCommand;

    status = cpssHalFdbMgrConvertPktCmdToDxChFdbTableDaSaCmd(command, &cpssCommand);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "FDB manager secure breach convert failed %d", status);
        return status;
    }

    status = cpssDxChBrgSecurBreachEventPacketCommandSet(devId,
                                                         eventType, cpssCommand);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "FDB manager secure breach set cmd failed with cpss error code %d", status);
        return status;
    }

    return status;
}

/**
* @internal cpssHalFdbMgrBrgSecurBreachCommandGet function
* @endinternal
*
* @brief API for setting Bridge secure breach command set.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] eventType             - event type.
* @param[in] command               - packet command.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssHalFdbMgrBrgSecurBreachCommandGet
(
    int                                 devId,
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType,
    xpsPktCmd_e                         *command
)
{
    GT_STATUS                          status = GT_OK;
    CPSS_PACKET_CMD_ENT                cpssCommand;

    status = cpssDxChBrgSecurBreachEventPacketCommandGet(devId,
                                                         eventType, &cpssCommand);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "FDB manager secure breach get cmd failed with cpss error code %d", status);
        return status;
    }

    status = cpssHalFdbMgrConvertDxChTablePktCmdToGenric(cpssCommand, command);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "FDB manager secure breach get convert failed %d", status);
        return status;
    }

    return status;
}

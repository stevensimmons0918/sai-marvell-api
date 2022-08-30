// xpsCopp.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsCopp.h
 * \brief This file contains API prototypes and type definitions
 *        for CoPP
 * \Copyright (c) Marvell [2000-2020]
 */

#ifndef _xpsCopp_h_
#define _xpsCopp_h_

#include "xpsEnums.h"
#include "xpsInit.h"
#include "cpssDxChNetIf.h"
//#include "xpControlPlanePolicingMgr.h"
#include "openXpsCopp.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * typedef: enum XPS_HOSTIF_TRAP_TYPE
 * Description: Enumeration of XPS Control Packets type.
 * Enumerations:
 * Comments:
 */
typedef enum
{
    XPS_COPP_CTRL_PKT_ARP_REQUEST = 0,
    XPS_COPP_CTRL_PKT_ARP_RESPONSE = 1,
    XPS_COPP_CTRL_PKT_BGP = 2,
    XPS_COPP_CTRL_PKT_BGPV6 = 3,
    XPS_COPP_CTRL_PKT_DHCP = 4,
    XPS_COPP_CTRL_PKT_DHCPV6 = 5,
    XPS_COPP_CTRL_PKT_EAPOL = 6,
    XPS_COPP_CTRL_PKT_IGMP_TYPE_LEAVE = 7,
    XPS_COPP_CTRL_PKT_IGMP_TYPE_QUERY = 8,
    XPS_COPP_CTRL_PKT_IGMP_TYPE_V1_REPORT = 9,
    XPS_COPP_CTRL_PKT_IGMP_TYPE_V2_REPORT = 10,
    XPS_COPP_CTRL_PKT_IGMP_TYPE_V3_REPORT = 11,
    XPS_COPP_CTRL_PKT_IP2ME = 12,
    XPS_COPP_CTRL_PKT_IPV6_MLD_V1_DONE = 13,
    XPS_COPP_CTRL_PKT_IPV6_MLD_V1_REPORT = 14,
    XPS_COPP_CTRL_PKT_IPV6_MLD_V1_V2 = 15,
    XPS_COPP_CTRL_PKT_IPV6_NEIGHBOR_DISCOVERY = 16,
    XPS_COPP_CTRL_PKT_L3_MTU_ERROR = 17,
    XPS_COPP_CTRL_PKT_LACP = 18,
    XPS_COPP_CTRL_PKT_LLDP = 19,
    XPS_COPP_CTRL_PKT_MLD_V2_REPORT = 20,
    XPS_COPP_CTRL_PKT_OSPF = 21,
    XPS_COPP_CTRL_PKT_OSPFV6 = 22,
    XPS_COPP_CTRL_PKT_PIM = 23,
    XPS_COPP_CTRL_PKT_PVRST = 24,
    XPS_COPP_CTRL_PKT_SAMPLEPACKET = 25,
    XPS_COPP_CTRL_PKT_SNMP = 26,
    XPS_COPP_CTRL_PKT_SSH = 27,
    XPS_COPP_CTRL_PKT_STP = 28,
    XPS_COPP_CTRL_PKT_TTL_ERROR = 29,
    XPS_COPP_CTRL_PKT_UDLD = 30,
    XPS_COPP_CTRL_PKT_VRRP = 31,
    XPS_COPP_CTRL_PKT_VRRPV6 = 32,
    XPS_COPP_CTRL_PKT_STATIC_FDB_MOVE = 33,
    XPS_COPP_CTRL_PKT_DHCP_L2 = 34,
    XPS_COPP_CTRL_PKT_DHCPV6_L2 = 35,
    XPS_COPP_CTRL_PKT_MAX,
} xpsCoppCtrlPkt_e;


#define XPS_COPP_RULE_ID_MAX 4

typedef struct xpsCoppRuleInfo
{
    uint32_t ruleId;
    uint32_t tableId;
} xpsCoppRuleInfo_t;

typedef struct xpsCpuCodeTableContextDbEntry
{
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;
    xpsCoppRuleInfo_t        ruleInfo[XPS_COPP_RULE_ID_MAX];
    uint32_t                 ruleNum;
    uint32_t                 policerId;
    uint32_t                 priority;
    xpsPktCmd_e              pktCmd;
} xpsCpuCodeTableContextDbEntry_t;


/**
 * \brief API that initializes the XPS COPP Mgr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppInit(void);

/**
 * \brief API that initializes the XPS COPP Mgr for a scope
 *
 *\param [in] scopeId
 * \return XP_STATUS
 */
XP_STATUS xpsCoppInitScope(xpsScope_t scopeId);

/**
 * \brief API to De-Init the XPS Copp Mgr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppDeInit(void);

/**
 * \brief API to De-Init the XPS COPP Mgr
 *
 *\param [in] scopeId
 * \return XP_STATUS
 */
XP_STATUS xpsCoppDeInitScope(xpsScope_t scopeId);

/**
 * \brief API to perform Add Device operations for COPP
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief API to perform Remove Device operations for COPP
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppRemoveDevice(xpsDevice_t devId);

/**
 * \brief This method adds a COPP entry with key {sourcePort,
 *        reasonCode}
 *
 * \param [in] devId
 * \param [in] sourcePort
 * \param [in] reasonCode
 * \param [in] entry
 * \param [out] indexList
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppAddEntry(xpDevice_t devId, uint32_t sourcePort,
                          uint32_t reasonCode, xpCoppEntryData_t entry, xpsHashIndexList_t *indexList);

/**
 * \brief This method updates a COPP entry using index
 *
 * \param [in] devId
 * \param [in] index
 * \param [in] entry
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppUpdateEntry(xpDevice_t devId, uint32_t index,
                             xpCoppEntryData_t entry);

/**
 * \brief This method gets COPP entry index with key
 *        {sourcePort, reasonCode}
 *
 * \param [in] devId
 * \param [in] sourcePort
 * \param [in] reasonCode
 * \param [out] index
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppGetIndex(xpDevice_t devId, uint32_t sourcePort,
                          uint32_t reasonCode, int *index);

/**
 * \public
 * \brief This method gets COPP entry with key {sourcePort,
 *        reasonCode}
 *
 * \param [in] devId
 * \param [in] sourcePort
 * \param [in] reasonCode
 * \param [out] entry
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppGetEntry(xpDevice_t devId, uint32_t sourcePort,
                          uint32_t reasonCode, xpCoppEntryData_t* entry);

/**
 * \brief This method gets COPP entry by index
 *
 * \param [in] devId
 * \param [in] index
 * \param [out] entry
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppGetEntryByIndex(xpDevice_t devId, uint32_t index,
                                 xpCoppEntryData_t* entry);

/**
 * \brief This method removes COPP entry by index
 *
 * \param [in] devId
 * \param [in] index
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppRemoveEntryByIndex(xpDevice_t devId, uint32_t index);

/**
 * \brief This method sets rehash level for COPP table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppSetRehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for COPP table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppGetRehashLevel(xpDevice_t devId, uint8_t* numOfRehashLevels);

/**
 * \brief This method sets rehash level for COPP table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppSetRehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for COPP table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppGetRehashLevel(xpDevice_t devId, uint8_t* numOfRehashLevels);

/**
 * \brief This method displays all the entries in COPP table
 *
 * \param [in] devId
 * \param [in] startIndex
 * \param [in] endIndex
 * \param [in] detailFormat
 * \param [in] silentMode
 * \param [in] tblecpyIndx
 * \param [out] numOfValidEntries
 * \param [out] logFile
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppDisplayTable(xpsDevice_t devId, uint32_t *numOfValidEntries,
                              uint32_t startIndex, uint32_t endIndex, char * logFile, uint32_t detailFormat,
                              uint32_t silentMode, uint32_t tblecpyIndx);

/**
 * \brief This method enables Ctrl Pkt trap at port level
 *
 * \param [in] devId
 * \param [in] type
 * \param [in] portId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppEnableCtrlPktTrapOnPort(xpDevice_t devId,
                                         xpsCoppCtrlPkt_e type, xpsPort_t portId, uint8_t enable);

/**
 * \brief This method enables Ctrl Pkt trap at vlan level
 *
 * \param [in] devId
 * \param [in] type
 * \param [in] vlanId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppEnableCtrlPktTrapOnVlan(xpDevice_t devId,
                                         xpsCoppCtrlPkt_e type,
                                         xpsVlan_t vlanId, macAddr_t mac,
                                         xpsPktCmd_e pktCmd, uint8_t enable);

/**
 * \brief This method enables Ctrl Pkt trap at switch level
 *
 * \param [in] devId
 * \param [in] type
 * \param [in] pktCmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppEnableCtrlPktTrapOnSwitch(xpDevice_t devId,
                                           xpsCoppCtrlPkt_e type, xpsPktCmd_e pktCmd);

/**
 * \brief This method displays Ctrl Pkt trap information for a trap type
 *
 * \param [in] devId
 * \param [in] type
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppDisplayCtrlPktTraps(xpDevice_t devId, xpsCoppCtrlPkt_e type);

XP_STATUS xpsCoppInbandMgmtRatelimitSet(xpDevice_t devId, uint32_t rate);

#ifdef __cplusplus
}
#endif

#endif

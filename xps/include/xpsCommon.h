// xpsCommon.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsCommon.h
 * \brief This file contains API prototypes and type definitions
 *  for use to other layer by exposing from the XPS layer
 */

#ifndef _xpsCommon_h_
#define _xpsCommon_h_



#ifdef __cplusplus
#include <iostream>
#include <exception>
#endif

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>         //To support directory related operations (i.e. openDir,clodeDir) - Currently used in serdesInit
#include "xpEnums.h"
#include "xpsEnums.h"
#include "xpTypes.h"
#include "xpsMirror.h"
#include "xpsInterface.h"
#include "xpsFdb.h"
//#include "xpsTunnel.h"
#include "xpsMpls.h"
#include "xpsXpImports.h"
#include <fcntl.h>
#include "prvCpssBindFunc.h"
#include "xpsUtil.h"
#ifdef __cplusplus
extern "C" {
#endif


/*NOTE: Trunk-id 0 is reserved by CPSS
        Trunk-id 1 is reserved for cascade trunk_id in cpssHalCascadeMgr.c
*/
#define CPSS_CSCD_TRUNK_ID 1


#define XPS_GLOBAL_TACM_MGR  0

/*
 * Update the below macros for vTCAMs ID's
 * Reserving 1 to 9 can be used for future entities, if any
 */

/*
* ACL Table prev start id was 0. Moved to 10 to accomodate all the tables under same
* vTCAM manager.
*/

#define ACL_TABLE_BASE 20

#define XPS_MAX_VTCAM_IDS   19
#define XPS_MIN_VTCAM_IDS   0
#define XPS_MAX_CTRL_TBL_IDS 1

#define XPS_L3_CTRL_TABLE_ID XPS_MIN_VTCAM_IDS
#define XPS_L3V6_CTRL_TABLE_ID XPS_MAX_CTRL_TBL_IDS

/* NOTE: order of below allocation is important and MUST not be modified.
   If changed, will to re-look at the region mapping.*/
#define XPS_L3_ING_STG_TABLE_ID_0   2
#define XPS_L3_ING_STG_TABLE_ID_1   3
#define XPS_L3_ING_STG_TABLE_ID_2   4
#define XPS_L3_ING_STG_TABLE_ID_3   5

#define XPS_L3V6_ING_STG_TABLE_ID_0 6
#define XPS_L3V6_ING_STG_TABLE_ID_1 7
#define XPS_L3V6_ING_STG_TABLE_ID_2 8
#define XPS_L3V6_ING_STG_TABLE_ID_3 9


#define XPS_L3_EGR_STG_TABLE_ID_0   10
#define XPS_L3_EGR_STG_TABLE_ID_1   11
#define XPS_L3_EGR_STG_TABLE_ID_2   12
#define XPS_L3_EGR_STG_TABLE_ID_3   13

#define XPS_L3V6_EGR_STG_TABLE_ID_0 14
#define XPS_L3V6_EGR_STG_TABLE_ID_1 15
#define XPS_L3V6_EGR_STG_TABLE_ID_2 16
#define XPS_L3V6_EGR_STG_TABLE_ID_3 17

#define XPS_L3_MIRROR_ING_STG_TABLE_ID_0     18
#define XPS_L3V6_MIRROR_ING_STG_TABLE_ID_0   19
#define XPS_L3_MIRROR_EGR_STG_TABLE_ID_0     20 /* Reserved */
#define XPS_L3V6_MIRROR_EGR_STG_TABLE_ID_0   21 /* Reserved */

#define XPS_ING_STG_V4_USER_ACL_TABLE_START XPS_L3_ING_STG_TABLE_ID_0
#define XPS_ING_STG_V6_USER_ACL_TABLE_START 6

#define XPS_EGR_STG_V4_USER_ACL_TABLE_START 10
#define XPS_EGR_STG_V6_USER_ACL_TABLE_START 14

#define TUNNEL_TERM_TABLE_ID 25
/*TODO Allocating new Vtcam for UNI fails PCL egress TCs for
       m0 devices. Need to check this.
       For now add the rules on Tunnel Term VTcam.*/
#define UNI_TERM_TABLE_ID TUNNEL_TERM_TABLE_ID

#define XPS_PORT_V4_ING_STG_TABLE_ID_0     32
#define XPS_PORT_V4_ERG_STG_TABLE_ID_0     33
#define XPS_PORT_V6_ING_STG_TABLE_ID_0     34
#define XPS_PORT_V6_ERG_STG_TABLE_ID_0     35


/* M0 Macros used in CNC */

#define CPSS_CNC_BLOCK_0    0 // ACL Counters
#define CPSS_CNC_BLOCK_1    1 // RIF Ingress Pass
#define CPSS_CNC_BLOCK_2    2 // RIF Ingress drop
#define CPSS_CNC_BLOCK_3    3 // RIF Egress Pass
#define CPSS_CNC_BLOCK_4    4 // RIF Egress drop
#define CPSS_CNC_BLOCK_5    5 // Not used
#define CPSS_CNC_BLOCK_6    6 // Not used
#define CPSS_CNC_BLOCK_7    7 // Not used

#define XPS_CPSS_CNC_COUNTER_PER_BLK 1024

/* Macros for ACL CNC counters */
/* AC3x - Block-0
 * Aldrin2 - Block-0,1
 * Falcon - Block-0-11
 * AC5X - Block-0-6
 */
#define XPS_ACL_CNC_BLK_NUM_AC3X            CPSS_CNC_BLOCK_0
#define XPS_ACL_CNC_BLK_NUM_ALDRIN2         CPSS_CNC_BLOCK_0
#define XPS_ACL_CNC_BLOCK_COUNT             6
#define XPS_ACL_CNC_PKT_COUNTER_BLK_START   0
#define XPS_ACL_CNC_BYTE_COUNTER_BLK_START  XPS_ACL_CNC_PKT_COUNTER_BLK_START + XPS_ACL_CNC_BLOCK_COUNT

#define XPS_CNC_BLOCK_0_RANGE_START (XPS_CPSS_CNC_COUNTER_PER_BLK*0)
#define XPS_CNC_BLOCK_1_RANGE_START (XPS_CPSS_CNC_COUNTER_PER_BLK*1)
#define XPS_CNC_BLOCK_2_RANGE_START (XPS_CPSS_CNC_COUNTER_PER_BLK*2)
#define XPS_CNC_BLOCK_3_RANGE_START (XPS_CPSS_CNC_COUNTER_PER_BLK*3)
#define XPS_CNC_BLOCK_4_RANGE_START (XPS_CPSS_CNC_COUNTER_PER_BLK*4)
#define XPS_CNC_BLOCK_5_RANGE_START (XPS_CPSS_CNC_COUNTER_PER_BLK*5)

/* Macros for Queue Counters */
#define QUEUE_STAT_WATERMARK_CNC_BLOCK_START   22
#define QUEUE_STAT_CNC_BLOCK_FIRST             16
#define QUEUE_STAT_CLIENT                      CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E

/* Note that block allocation is dynamic based on profile value.
   Future unsed blocks may vary based on allocation.
   TODO: Need to have common allocator for CNC blocks to accomodate the design.*/
#define FALCON_RIF_CNC_BLOCK_START 24
#define ALDRIN2_RIF_CNC_BLOCK_START 2
#define AC5X_RIF_CNC_BLOCK_START 7

#define MAX_RIF_CLINETS_NON_64_BIT_MODE  4

#define CNC_CLIENT_L2L3_INGRESS_VLAN_BLK_OFFSET   0
#define CNC_CLIENT_INGRESS_VLAN_DROP_BLK_OFFSET   1
#define CNC_CLIENT_EGRESS_VLAN_PASS_BLK_OFFSET    2
#define CNC_CLIENT_EGRESS_VLAN_DROP_BLK_OFFSET    3

#define MAX_RIF_CLINETS_64_BIT_MODE      8

/* For same Client, group Pass,Drop in sequence.
   This helps to re-use when pass/drop bit is 0th bit.
*/
#define CNC_CLIENT_L2L3_INGRESS_VLAN_PKT_BLK_OFFSET    0
#define CNC_CLIENT_L2L3_INGRESS_VLAN_BYTE_BLK_OFFSET   1
#define CNC_CLIENT_INGRESS_VLAN_DROP_PKT_BLK_OFFSET    2
#define CNC_CLIENT_INGRESS_VLAN_DROP_BYTE_BLK_OFFSET   3
#define CNC_CLIENT_EGRESS_VLAN_PASS_PKT_BLK_OFFSET     4
#define CNC_CLIENT_EGRESS_VLAN_DROP_PKT_BLK_OFFSET     5
#define CNC_CLIENT_EGRESS_VLAN_PASS_BYTE_BLK_OFFSET    6
#define CNC_CLIENT_EGRESS_VLAN_DROP_BYTE_BLK_OFFSET    7

/**
 *
 * \brief get endpoint Info of vlan
 *
 * \param [in] vlanId
 * \param [in] intfId
 * \param [in] *tagType
 * \param [in] *data
 * \returns int
 *
 */
/*  Vlan utility APIs  */
XP_STATUS xpsVlanSetIngStpState(xpsDevice_t devId, xpsVlan_t vlanId,
                                xpsInterfaceId_t intfId, xpsStpState_e stpState);
XP_STATUS xpsVlanSetRouterAclEnable(xpsDevice_t devId, xpsVlan_t vlanId,
                                    uint8_t enable);
XP_STATUS xpsVlanSetRouterAclId(xpsDevice_t devId, xpsVlan_t vlanId,
                                uint32_t aclId);
XP_STATUS xpsVlanGetRouterAclEnable(xpsDevice_t devId, xpsVlan_t vlanId,
                                    uint8_t *enable);
XP_STATUS xpsVlanGetRouterAclId(xpsDevice_t devId, xpsVlan_t vlanId,
                                uint32_t *aclId);
XP_STATUS xpsVlanIncrementFdbCount(xpsDevice_t devId, xpsVlan_t vlanId);
XP_STATUS xpsVlanDecrementFdbCount(xpsDevice_t devId, xpsVlan_t vlanId);
/**
 * \brief Clear fdbCount of single Vlan
 *
 * \param [in] devId
 * \param [in] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanClearFdbCountByVlan(xpsDevice_t devId, xpsVlan_t vlanId);

/**
 * \brief Clear fdbCount of all Vlans
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanClearFdbCountForAllVlan(xpsDevice_t devId);

/* Private vlan utility APIs */
XP_STATUS xpsPVlanAddFdb(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                         xpsPrivateVlanType_e vlanType, xpsHashIndexList_t *indexList);

/* These APIs Required for Vlan manager. Not to be directly used by User */
XP_STATUS xpsStgAddVlanDb(xpsScope_t scopeId, xpsStp_t stgId, xpsVlan_t vlanId);
XP_STATUS xpsStgRemoveVlanDb(xpsScope_t scopeId, xpsStp_t stgId,
                             xpsVlan_t vlanId);
XP_STATUS xpsStpUpdateIntfStateDb(xpsDevice_t devId, xpsStp_t stpId,
                                  xpsInterfaceId_t intfId, xpsStpState_e stpState);
XP_STATUS xpsVlanAddPortToLag(xpsDevice_t devId, xpsInterfaceId_t lagIntfId,
                              xpsInterfaceId_t portIntfId);
XP_STATUS xpsVlanRemovePortFromLag(xpsDevice_t devId,
                                   xpsInterfaceId_t lagIntfId, xpsInterfaceId_t portIntfId);

/**************************************** MPLS Tunnel ***************************************************/
/**
 * \brief This method deletes an MPLS tunnel origination entry.
 *
 * \param [in] devId        Device Id of device
 * \param [in] mplsTnlId    MPLS tunnel interface Id
 *
 * \return XP_STATUS
*/
XP_STATUS xpsMplsRemoveTunnelOrigination(xpsDevice_t devId,
                                         xpsInterfaceId_t mplsTnlId);

/**************************************** MPLS VPN ***************************************************/
/**
 * \brief This method adds a MPLS VPN entry on a device
 *
 * \param [in] devId        Device Id of device
 * \param [in] vpnLabel     VPN label
 * \param [in] l3IntfId     Id of the L3 interface
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMplsAddVpnEntry(xpsDevice_t devId, uint32_t vpnLabel,
                             uint32_t l3IntfId);

/**
 * \brief This method removes a MPLS VPN entry on a device
 *
 * \param [in] devId        Device Id of device
 * \param [in] vpnLabel     VPN label
 *
 * \return XP_STATUS
 */

XP_STATUS xpsMplsRemoveVpnEntry(xpsDevice_t devId, uint32_t vpnLabel);


/**
 * \brief API to insert a new device database for an existing
 *        per device database
 *
 * This API will create a new device db associated with an
 * existing per device database handle
 *
 * \param [in] devId
 *
 * \return int
 */
XP_STATUS xpsPortCreateDbForDevice(xpsDevice_t devId);

/**
 * \brief API to delete an existing device database associated
 *        with a per device database handle
 *
 * This API should be invoked when a device is reset or removed
 * from the system. This API will purge a per device database
 * handle's device database
 *
 * \param [in] devId
 *
 * \return int
 */
XP_STATUS xpsPortDeleteDbForDevice(xpsDevice_t devId);

/**
 * \brief xpVprintf
 * \param [in] fmt
 * \param [in] ap
 */
#if 0
int xpVprintf(const char* fmt, va_list ap);
return vprintf(fmt, ap);
return 0;
}
#endif



#ifdef DEBUG_FLAG
#define XPS_FUNC_ENTRY_LOG() \
        cpssOsPrintf("ENTRY:\t  %s:%d:%s \n ", __FILE__, __LINE__,__func__);

#define XPS_FUNC_EXIT_LOG() \
        cpssOsPrintf("EXIT:\t  %s:%d:%s \n ", __FILE__, __LINE__,__func__);

#else
#define XPS_FUNC_ENTRY_LOG() \
        do{} while(0)

#define XPS_FUNC_EXIT_LOG() \
        do{} while(0)
#endif


#define LOGFN(MOD,SUB,LVL,MSG, ...) writeLog((char*)__FILE__, __LINE__, LVL, MSG, ##__VA_ARGS__)

#define LOGENTRY(MOD,LVL,MSG, ...) writeLog((char*)__FILE__, 0, LVL, MSG, ##__VA_ARGS__)

#define IS_DEVICE_VALID(a) 1

#ifdef __cplusplus
}
#endif

#endif  //_xpsCommon_h_



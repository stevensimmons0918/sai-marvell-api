// xpSaiApp.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpSaiApp_h_
#define _xpSaiApp_h_

/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
 xpSaiApp - sample code and examples
 --------------------------------
 (1) sample code for chip initialization
 (2) sample code for xdk initialization -- feature manager init or 
     cInterface init.
 (3) configuration file (parsing infastructure)
 (4) configuration code (examples) to program all tables
 ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/

#ifdef __cplusplus
extern "C" {
#endif
#include "sai.h"
#ifdef __cplusplus
}
#endif

//Config file 
#include "xpAppConfig.h"
#include "xpConfigFile.h"

//Init manager
#include "xpsInit.h"

//Interface manager
#include "xpsInterface.h"
#include "xpsPort.h"

//Layer 2 managers
#include "xpsFdb.h"
#include "xpsStp.h"
#include "xpsVlan.h"
#include "xpsLag.h"

//Layer 3 managers
#include "xpsL3.h"

//Tunnel managers
#include "xpsTunnel.h"
#include "xpsVxlan.h"
#include "xpsGeneve.h"
#include "xpsNvgre.h"
#include "xpsIpGre.h"
#include "xpsIpinIp.h"
#include "xpsMpls.h"

//Multicast managers
#include "xpsMulticast.h"

//Aging manager
#include "xpsAging.h"

//Counting/Policing/Sampling manager (infrastructure)
#include "xpsAcm.h"
#include "xpsSflow.h"

//NAT manager
#include "xpsNat.h"

//IACL and EACL manager
#include "xpsAcl.h"
//Packet driver
#include "xpsPacketDrv.h"

// Qos Manager
#include "xpsQos.h"

// 802.1BR Manager
#include "xps8021Br.h"


//Mirror manager
#include "xpsMirror.h"

//Egress filter manager
#include "xpsEgressFilter.h"

//Tree managment utility (standard linux library)
#include <search.h>
#include "xpAppUtil.h"

#define XP_SAI_APP_DEFAULT_VLAN 1
#define MAX_VRF_IDX		4096
#define MAX_ACL_TABLE_ATTR      (SAI_ACL_TABLE_ATTR_FIELD_END - SAI_ACL_TABLE_ATTR_FIELD_START)
#define MAX_NEXT_HOP_GROUP	64

//typedef l2MdtEntry l2MdtNodeList[512];
//typedef l3MdtEntry l3MdtNodeList[512];
//
//typedef enum prgType
//{
//    XP_FL_PRG = 0,
//    XP_XPS_PRG,
//}prgType;
//
typedef struct tnlVlanContext
{
    uint16_t vlanId:12;      // Maximum 4k.       
    uint32_t stgIdx;
    xpL2DomainCtx_t l2DomainContext;
} tnlVlanContext;

// instance  @ main.c
extern sai_switch_api_t* xpSaiSwitchApi; 
extern sai_vlan_api_t* xpSaiVlanApi;
extern sai_bridge_api_t* xpSaiBridgeApi;
extern sai_mirror_api_t* xpSaiMirrorApi;
extern sai_stp_api_t* xpSaiStpApi;
extern sai_lag_api_t* xpSaiLagApi;
extern sai_router_interface_api_t* xpSaiRouterInterfaceApi;
extern sai_route_api_t* xpSaiRouteApi;
extern sai_fdb_api_t* xpSaiFdbApi;
extern sai_acl_api_t* xpSaiAclApi;
extern sai_virtual_router_api_t* xpSaiVrfApi;
extern sai_neighbor_api_t* xpSaiNeighborApi;
extern sai_next_hop_api_t* xpSaiNexthopApi;
extern sai_next_hop_group_api_t* xpSaiNextHopGroupApi;
extern sai_acl_api_t* xpSaiAclApi;
extern sai_port_api_t* xpSaiPortApi;
extern sai_queue_api_t* xpSaiQueueApi;
extern sai_scheduler_api_t* xpSaiSchedulerApi;
extern sai_wred_api_t* xpSaiWredApi;
extern sai_qos_map_api_t* xpSaiQosMapApi;
extern sai_policer_api_t* xpSaiPolicerApi;

XP_STATUS saiStatus2XpsStatus(sai_status_t status);

//
//
//
//typedef struct oifIndexToRootMdtIdxMap
//{
//    uint32_t oifIdx;
//    uint32_t rootMdtIdx; 
//} oifIndexToRootMdtIdxMap;
//
//typedef struct mvifIndexToAllocatedMvifIdxMap
//{
//    uint32_t mvifIdx;
//    uint32_t allocatedMvifIdx; 
//} mvifIndexToAllocatedMvifIdxMap;
//
//typedef struct mdtToAllocatedMdtmap
//{
//    uint32_t idx;
//    uint32_t mdtIdx; 
//} mdtToAllocatedMdtmap;
//
////compare function for oifIndexToRootMdtIdxMap database
//int compareOifIndexToRootMdtIdxMap(const void *l, const void *r);
////get mdt idx from oifIndexToRootMdtIdxMap
//void* getOifIndexToRootMdtIdxMap(uint32_t oifIdx);
//
////compare function for mvifIndexToAllocatedMvifIdxMap database
//int compareMvifIndexToAllocatedMvifIdxMap(const void *l, const void *r);
////get allocated mvif idx from mvifIndexToAllocatedMvifIdxMap
//void* getMvifIndexToAllocatedMvifIdxMap(uint32_t mvifIdx);
//
////compare function for mdtToAllocatedMdtmap database
//int compareMdtToAllocatedMdtMap(const void *l, const void *r);
////get mdt idx from mdtToAllocatedMdtmap
//void* getMdtToAllocatedMdtMap(uint32_t idx, void * mvifToMdtIdxRoot);
#ifdef __cplusplus
extern "C" {
#endif
XP_STATUS xpAppAddDevice(xpsDevice_t deviceId, xpsInitType_t initType, xpDevConfigStruct* config);

XP_STATUS xpAppDeInitDevice(xpsDevice_t devId);

XP_STATUS xpAppSendPacketToXp(xpDevice_t deviceId, sai_object_id_t switchOid);
#ifdef __cplusplus
}
#endif
////xpAppVlanContext database management APIs.
//#if 0
//XP_STATUS xpAppL2LearnProcess(xpDevice_t devId, xphRxHdr *xphHdr, void *buf, uint16_t bufSize);
//#endif

//XP_STATUS xpSaiAppDeInit(void);
//XP_STATUS xpSaiAppLoadProfiles(xpDevice_t devId);

//xpApp functions - table population.
XP_STATUS xpSaiAppPopulateTables(xpDevice_t devNum, void *arg);

XP_STATUS xpSaiAppWriteControlMacEntry(xpDevice_t devId, sai_object_id_t switchOid, ControlMacEntry *controlMacData);

XP_STATUS xpSaiAppWritePortIvifEntry(xpDevice_t devId, sai_object_id_t switchOid, PortConfigEntry *portConfigData);
XP_STATUS xpSaiAppConfigureMvifEntry(xpDevice_t devId, MvifEntry *mvifData, xpsMcL3InterfaceListId_t l3IfListId, xpsMcL2InterfaceListId_t ifListId, bool isL2);
XP_STATUS xpSaiAppConfigureIpv4BridgeMcEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv4BridgeMcEntry *ipv4BridgeMcData, uint32_t numOfMvifEntry, uint32_t *index);
XP_STATUS xpSaiAppConfigureIpv6BridgeMcEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv6BridgeMcEntry *ipv6BridgeMcData, uint32_t numOfMvifEntry, uint32_t *index);
XP_STATUS xpSaiAppConfigureIpv4RouteMcEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv4RouteMcEntry *ipv4RouteMcData, uint32_t numOfMvifEntry, uint32_t *index);
XP_STATUS xpSaiAppConfigureIpv6RouteMcEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv6RouteMcEntry *ipv6RouteMcData, uint32_t numOfMvifEntry, uint32_t *index);
XP_STATUS xpSaiAppConfigureIpv4PimBiDirEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv4PimBiDirEntry *ipv4PimBiDirData, uint32_t *index);
XP_STATUS xpSaiAppConfigureMacSAMSBsEntry(xpDevice_t devId, sai_object_id_t switchOid, uint8_t *macSAMSBs);
XP_STATUS xpSaiAppWriteNatData(xpDevice_t devId, sai_object_id_t switchOid);

XP_STATUS xpSaiAppConfigureIacl(xpDevice_t devNum, sai_object_id_t switchOid);
XP_STATUS xpSaiAppConfigurecustIacl(xpDevice_t devNum, sai_object_id_t switchOid);
XP_STATUS xpSaiAppConfigureEacl(xpDevice_t devNum, sai_object_id_t switchOid);

extern XP_STATUS xpAppCleanup(xpsDevice_t devId);

int testMplsTunnel(xpDevice_t deviceId, XP_PIPE_MODE mode);
extern XP_STATUS xpPyInit(void);

int recvTrapPacket(xpDevice_t devNum, uint16_t NumOfPkt);
void getPortStat(xpDevice_t devId, xpPort_t* devPortList, uint32_t numOfPort);
// deviceId
XP_STATUS xpSaiAppConfigureMirrorData(xpDevice_t devId, sai_object_id_t switchOid);
XP_STATUS xpSaiAppConfigureVrf(xpDevice_t devId, sai_object_id_t switchOid, VrfEntry* vrfEntry);
//XP_STATUS xpsProfileLoadLde(xpDevice_t devId, uint32_t sdeId, uint32_t ldeId, uint8_t *profile, uint8_t *seProfile);

#endif // _xpApp_h_

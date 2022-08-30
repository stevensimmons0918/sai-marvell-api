//  xpApp.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpApp_h_
#define _xpApp_h_

/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
 xpApp - sample code and examples
 --------------------------------
 (1) sample code for chip initialization
 (2) sample code for xdk initialization -- feature manager init or 
     cInterface init.
 (3) configuration file (parsing infastructure)
 (4) configuration code (examples) to program all tables
 ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/

//Config file 
#include "xpAppConfig.h"
//#include "xpConfigFile.h"

//Init manager
#include "xpsInit.h"

//Interface manager
#include "xpsInterface.h"
#include "xpsPort.h"

//Sal

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
#include "xpsPbb.h"

//Multicast managers
#include "xpsMulticast.h"

//Aging manager
#include "xpsAging.h"

//Counting/Policing/Sampling manager (infrastructure)
#include "xpsAcm.h"
#include "xpsSflow.h"

//NAT manager
#include "xpsNat.h"
#if 0
#include "xpsAcl.h"
#endif
//Packet driver
#include "xpsPacketDrv.h"

// Qos Manager
#include "xpsQos.h"

// Copp Manager
#include "xpsCopp.h"

// 802.1BR Manager
#include "xps8021Br.h"


//Mirror manager
#include "xpsMirror.h"

#include "xpAppUtil.h"

#ifdef __cplusplus
extern "C" {
#endif

//typedef l2MdtEntry l2MdtNodeList[512];
//typedef l3MdtEntry l3MdtNodeList[512];
//
//typedef enum prgType
//{
//    XP_FL_PRG = 0,
//    XP_XPS_PRG,
//}prgType;
//
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

XP_STATUS xpAppAddDevice(xpsDevice_t deviceId, xpsInitType_t initType, xpDevConfigStruct* config);

////xpAppVlanContext database management APIs.
//#if 0
//XP_STATUS xpAppL2LearnProcess(xpDevice_t devId, xphRxHdr *xphHdr, void *buf, uint16_t bufSize);
//#endif

//xpsApp functions - init examples.
XP_STATUS xpsAppInit(XP_DEV_TYPE_T devType, xpsRangeProfileType_t rpType, xpsInitType_t initType);
//XP_STATUS xpsAppDeInit(void);
XP_STATUS xpsAppInitDevice(xpsDevice_t devId, xpsInitType_t initType, xpDevConfigStruct *devConfig);
XP_STATUS xpAppDeInitDevice(xpsDevice_t devId);
//XP_STATUS xpsAppLoadProfiles(xpDevice_t devId);

//xpApp functions - table population.
XP_STATUS xpsAppPopulateTables(xpDevice_t devNum, xpsInitType_t initType, xpDevConfigStruct* config);
//PureOf - table population.
XP_STATUS xpsAppPureOfPopulateTable(xpDevice_t devNum);

XP_STATUS xpsAppWriteControlMacEntry(xpDevice_t devId, ControlMacEntry *controlMacData);

XP_STATUS xpsAppWritePortIvifEntry(xpDevice_t devId, PortConfigEntry *portConfigData);
XP_STATUS xpsAppWritePortIvifEntry(xpDevice_t devId, PortConfigEntry *portConfigData);
XP_STATUS xpsAppConfigureMvifEntry(xpDevice_t devId, MvifEntry *mvifData, xpsMcL3InterfaceListId_t l3IfListId, xpsMcL2InterfaceListId_t ifListId, bool isL2);
XP_STATUS xpsAppConfigureIpv4BridgeMcEntry(xpDevice_t devId, Ipv4BridgeMcEntry *ipv4BridgeMcData, uint32_t numOfMvifEntry, uint32_t *index);
XP_STATUS xpsAppConfigureIpv6BridgeMcEntry(xpDevice_t devId, Ipv6BridgeMcEntry *ipv6BridgeMcData, uint32_t numOfMvifEntry, uint32_t *index);
XP_STATUS xpsAppConfigureIpv4RouteMcEntry(xpDevice_t devId, Ipv4RouteMcEntry *ipv4RouteMcData, uint32_t numOfMvifEntry, uint32_t *index);
XP_STATUS xpsAppConfigureIpv4RouteMcControlEntry(xpsDevice_t devId, Ipv4RouteMcEntry *ipv4RouteMcData, uint32_t *index);
XP_STATUS xpsAppConfigureIpv6RouteMcControlEntry(xpsDevice_t devId, Ipv6RouteMcEntry *ipv6RouteMcData, uint32_t *index);
XP_STATUS xpsAppConfigureIpv6RouteMcEntry(xpDevice_t devId, Ipv6RouteMcEntry *ipv6RouteMcData, uint32_t numOfMvifEntry, uint32_t *index);
XP_STATUS xpsAppConfigureIpv4PimBiDirEntry(xpDevice_t devId, Ipv4PimBiDirEntry *ipv4PimBiDirData, uint32_t *index);
XP_STATUS xpsAppConfigureMacSAMSBsEntry(xpDevice_t devId, uint8_t *macSAMSBs);
XP_STATUS xpsAppWriteNatData(xpDevice_t devId);
#if 0
XP_STATUS xpsAppConfigurePbr(xpDevice_t devId, xpIaclkeyFieldList_t *fldData, bool *kyFldLst, uint32_t pbrBaseIndex);
XP_STATUS xpsAppConfigureWcmIacl(xpsDevice_t devNum);
XP_STATUS xpsAppConfigureIacl(xpDevice_t devNum, xpDevConfigStruct* config);
XP_STATUS xpsAppConfigureEacl(xpDevice_t devNum);
#endif
XP_STATUS xpAppSendPacketToXp(xpDevice_t deviceId);
extern XP_STATUS xpAppCleanup();
XP_STATUS xpsAppConfigurecustAcl(xpDevice_t devNum, xpDevConfigStruct* config);

int testMplsTunnel(xpDevice_t deviceId, XP_PIPE_MODE mode);
void getPortStat(xpDevice_t devId, xpPort_t* devPortList, uint32_t numOfPort);
// deviceId
XP_STATUS xpsAppConfigureMirrorData(xpDevice_t devId);
XP_STATUS xpsAppWriteQosMap(xpDevice_t devId);
XP_STATUS xpsAppConfigureOpenFlow(xpDevice_t devId);
XP_STATUS xpsAppConfigurePureOpenFlow(xpDevice_t devNum);
XP_STATUS xpsAppPrivateWireEnable(xpDevice_t devId);
XP_STATUS xpsAppPortVlanRedirect(xpDevice_t devId);
XP_STATUS xpsAppProgarmTrustBr(xpsDevice_t devId, TrustBrEntry *trustBrEntry);
XP_STATUS xpsAppProgarmVif(xpsDevice_t devId, VifEntry *vifDataEntry);
//XP_STATUS xpsProfileLoadLde(xpDevice_t devId, uint32_t sdeId, uint32_t ldeId, uint8_t *profile, uint8_t *seProfile);
#ifdef  __cplusplus
}
#endif
#endif // _xpApp_h_

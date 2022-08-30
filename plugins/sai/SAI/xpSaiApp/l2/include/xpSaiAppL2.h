// xpSaiApp.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpSaiAppL2_h_
#define _xpSaiAppL2_h_

/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
 xpSaiApp - sample code and examples
 --------------------------------
 (1) sample code for chip initialization
 (2) sample code for xdk initialization -- feature manager init or 
     cInterface init.
 (3) configuration file (parsing infastructure)
 (4) configuration code (examples) to program all tables
 ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/

#include "xpSaiApp.h"
#include "xpAppUtil.h"
#include "xpsInternal.h"

//Tree managment utility (standard linux library)
#include <search.h>


#define MAX_NO_LAGS (256)
#define MAX_VLAN_MEMBER_OID (256)

//xpSaiAppVlanContext database defined for the application code.
typedef struct xpSaiAppVlanParams
{
    uint8_t     macSAmissCmd;
    uint8_t     broadcastCmd;
    uint8_t     ipv4ARPBCCmd;
    uint8_t     ipv4RouteEn;
    uint8_t     ipv6RouteEn;
    uint8_t     ipv4BridgeMcMode;
    uint8_t     ipv6BridgeMcMode;
    uint8_t     ipv4RouteMcEn;
    uint8_t     ipv6RouteMcEn;
    uint8_t     mplsRouteEn;
    uint8_t     ipv4RouteMcMode;
    uint8_t     ipv6RouteMcMode;
    uint8_t     vrfId;
    uint8_t     unregMcCmd;
    uint8_t     enL3Inf;
    uint8_t     macSALSBs;
    uint8_t     natEn;
    uint8_t     natMode;
    uint8_t     natScope;	
    uint8_t     ipv4UrpfEn; 
    uint8_t     ipv4UrpfMode; 
    uint8_t     ipv6UrpfEn; 
    uint8_t     ipv6UrpfMode;
} xpSaiAppVlanParams;

typedef struct xpSaiAppVlanContext
{
    uint16_t    vlanId:12;      // Maximum 4k
    uint8_t     devBitMap[MAX_SYS_DEVICES/8 + 1];
    uint32_t    stgIdx;
    uint8_t     numPorts;
    XP_PVLAN_TYPE vlanType;
    uint16_t    parentVlan:12;    
    xpSaiAppVlanParams *vlanParams;
    xpL2DomainCtx_t l2DomainContext;
} xpSaiAppVlanContext;


typedef struct xpSaiAppPortVlanParams
{
    uint8_t     tagType;
    uint8_t     enOpenFlow;
    uint8_t     stpState;
    uint8_t     setIngressVif;
    uint8_t     disableTunnelVif;
    uint8_t     mirrorToAnalyzerMask;
    uint8_t     setBd;
    uint8_t     enVlanHairPining;
    uint8_t     enPbb;
    uint8_t     natMode;
    uint8_t     natScope;
    uint8_t     enRouterACL;
    uint32_t    routeAclId;
    uint8_t     enBridgeACL;
    uint32_t    bridgeAclId;
    uint8_t     enIpv4Tunnel;
    uint8_t     enMplsTunnel;
    uint8_t     countMode;
    uint8_t     setEgressVif;
    uint16_t    evif;
    uint8_t     encapType;
}xpSaiAppPortVlanParams;


//encapNode and encapList defined for the application code.

typedef struct
{
    uint8_t     encapType;
    uint32_t    encapId;
} encapEntry;

typedef struct _extPortGroup
{
    uint32_t key;
    xpsInterfaceId_t groupId;
    xpsInterfaceId_t *intfId;
    uint16_t numOfExtendedPorts;
}extPortGroup;

//xpSaiApp functions - table population.
XP_STATUS xpSaiAppConfigureVlan(xpDevice_t devNum, sai_object_id_t switchOid, VlanEntry *vlanEntry);
XP_STATUS xpSaiAppSetVlanParams(xpDevice_t devNum, sai_object_id_t switchOid, xpVlan_t vlanId, xpSaiAppVlanParams * vlanParams);
XP_STATUS xpSaiAppSetPortVlanParams(xpDevice_t devId, xpVlan_t vlanId, xpsInterfaceId_t intfId, xpSaiAppPortVlanParams * portVlan);
XP_STATUS xpSaiAppConfigureLag(xpDevice_t devNum, sai_object_id_t switchOid, PortLagEntry *portLagEntry);
XP_STATUS xpSaiAppGetFdbEncap(xpDevice_t devNum, MacEntry *xpsFdbData, encapEntry *encapEntryData);
XP_STATUS xpSaiAppAddFdb(xpDevice_t devId, sai_object_id_t switchOid, MacEntry *xpsFdbData, uint32_t numOfMvifEntry);
XP_STATUS xpSaiAppConfigureTenant(xpDevice_t devId, sai_object_id_t switchOid, VlanTenantIdEntry * tenantEntry);
void* xpSaiAppGetExtPortsGroup(uint32_t port);
void xpSaiAppL2CleanUp(xpsDevice_t devId);

#endif // _xpSaiAppL2_h_


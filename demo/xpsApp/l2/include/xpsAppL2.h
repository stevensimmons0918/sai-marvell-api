//  xpsApp.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpsAppL2_h_
#define _xpsAppL2_h_

/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****
 xpsApp - sample code and examples
 --------------------------------
 (1) sample code for chip initialization
 (2) sample code for xdk initialization -- feature manager init or 
     cInterface init.
 (3) configuration file (parsing infastructure)
 (4) configuration code (examples) to program all tables
 ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/

#include "xpsApp.h"
#include "xpAppUtil.h"
#include "xpsInternal.h"

//Tree managment utility (standard linux library)
#include <search.h>


#define MAX_NO_LAGS 256

//xpsAppVlanContext database defined for the application code.
typedef struct xpsAppVlanParams
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
    uint32_t    ipv4MtuLen;
    uint32_t    ipv6MtuLen;
    uint8_t     mtuPktCmd;
    uint32_t    mvifIdx[10];
    uint32_t    mcVifIdx;
    uint32_t    mcastIdx;
} xpsAppVlanParams;

typedef struct xpsAppVlanContext
{
    uint16_t    vlanId:12;      // Maximum 4k
} xpsAppVlanContext;


typedef struct xpsAppPortVlanParams
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
}xpsAppPortVlanParams;



//encapNode and encapList defined for the application code.

typedef struct
{
    uint8_t     encapType;
    uint32_t    encapId;
} encapEntry;

typedef struct _extPortGroup
{
    int key;
    xpsInterfaceId_t groupId;
    xpsInterfaceId_t *intfId;
    uint16_t numOfExtendedPorts;
}extPortGroup;


//xpsApp functions - table population.
XP_STATUS xpsAppCreateDefaultVlan(xpDevice_t devId);
XP_STATUS xpsAppConfigureVlan(xpDevice_t devNum, VlanEntry *vlanEntry, uint32_t numOfMvifEntry);
uint8_t xpsAppSetVlanParams(xpDevice_t devNum, xpVlan_t vlanId, xpsAppVlanParams * vlanParams, uint32_t numOfMvifEntry);
uint8_t xpsAppSetPortVlanParams(xpDevice_t devId, xpVlan_t vlanId, xpsInterfaceId_t intfId, xpsAppPortVlanParams * portVlan);
XP_STATUS xpsAppConfigureLag(xpDevice_t devNum, PortLagEntry *portLagEntry);
XP_STATUS xpsAppGetFdbEncap(xpDevice_t devId, MacEntry *xpsFdbData, encapEntry *encapEntryData);
uint8_t xpsAppAddFdb(xpDevice_t devId, MacEntry *xpsFdbData, uint32_t numOfMvifEntry);
XP_STATUS xpsAppConfigureTenant(xpDevice_t devId, VlanTenantIdEntry * tenantEntry);
void* xpsAppGetExtPortsGroup(xpDevice_t devNum, uint32_t port);
int compareExtPortsMap(const void *l, const void *r);
XP_STATUS xpsAppConfigureVlanScope(xpDevice_t devNum, VlanScopeEntry *vlanScopeEntry, uint32_t numOfMvifEntry);
uint8_t xpsAppAddFdbScope(xpDevice_t devId, MacScopeEntry *xpsFdbScopeData, uint32_t numOfMvifEntry);
uint8_t xpsAppAddFdbControlMac(xpDevice_t devId, FdbControlMacEntry *xpsFdbControlMacData);
XP_STATUS xpsAppProgarmTrustBr(xpDevice_t devId, TrustBrEntry *trustBrEntry);


#endif // _xpsAppL2_h_


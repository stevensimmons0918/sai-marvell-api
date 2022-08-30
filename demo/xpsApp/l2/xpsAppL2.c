// xpsApp.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsAppL2.h"
#include "assert.h"
#include "xpsMac.h"
#include "xpsScope.h"
#include "xpsL3.h"
#include "xpsGlobalSwitchControl.h"
#include "xpsLag.h"

xpsInterfaceId_t gLagIntfId[XP_MAX_DEVICES][MAX_NO_LAGS];

#define XPSAPP_DEFAULT_VLAN 100

extern uint32_t tnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t vlanToL3Intf[4096];
extern uint32_t l3TnlIntf[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlType[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlVirtualId[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlIsid[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t
multicastVif[XP_MAX_DEVICES][XP_MULTICAST_TYPE_MAX][MAX_MULTICAST_ENTRIES];

extern int numOfEntry(entryList *list);
//global definition for extendedPort database's "extPortsRoot"
static void *extPortsRoot[XP_MAX_DEVICES] = { NULL };

//compare function for extPorts database
int compareExtPortsMap(const void *l, const void *r)
{
    const extPortGroup *lm = (extPortGroup *)l;
    const extPortGroup *lr = (extPortGroup *)r;
    return lm->key - lr->key;
}

//get extPorts database
void* xpsAppGetExtPortsGroup(xpDevice_t devId, uint32_t port)
{
    extPortGroup tmpCtx;

    tmpCtx.key = port;
    void *r = tfind(&tmpCtx, &extPortsRoot[devId], compareExtPortsMap);
    if (r)
    {
        //LOG HERE.
    }
    return r;
}

XP_STATUS xpsAppCreateDefaultVlan(xpDevice_t devId)
{
    xpsPort_t portNum = 0;
    XP_STATUS retVal = XP_NO_ERR;
    xpsInterfaceId_t intfId;
    uint8_t isPortValid=0;
    uint16_t maxTotalPorts =0;
    if ((retVal = xpsGlobalSwitchControlGetMaxNumPhysicalPorts(devId,
                                                               &maxTotalPorts)) != XP_NO_ERR)
    {
        printf("%s:Error: Can't get max physical port number. error code:%d\n",
               __FUNCTION__, retVal);
    }

    retVal = xpsVlanCreate(devId, XPSAPP_DEFAULT_VLAN);
    if ((retVal != XP_NO_ERR) && (retVal != XP_ERR_KEY_EXISTS))
    {
        printf("%s:Error: Can't create default vlan. error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }
    for (portNum = 0; portNum < maxTotalPorts; portNum++)
    {
        retVal = xpsMacIsPortNumValid(devId, portNum, &isPortValid);
        if (retVal != XP_NO_ERR && retVal != XP_PORT_NOT_INITED)
        {
            printf("%s:Error: Port validity check failed for port- %d with %d\n",
                   __FUNCTION__, portNum, retVal);
            return retVal;
        }

        if (0 == isPortValid)
        {
            continue;
        }

        if ((retVal = xpsPortGetPortIntfId(devId, portNum, &intfId)) != XP_NO_ERR)
        {
            printf("%s:Error: Can't get interface Id. error code: %d\n", __FUNCTION__,
                   retVal);
            return retVal;
        }

        retVal = xpsVlanAddInterface(devId, XPSAPP_DEFAULT_VLAN, intfId,
                                     XP_L2_ENCAP_DOT1Q_UNTAGGED);
        if ((retVal != XP_NO_ERR) && (retVal != XP_ERR_KEY_EXISTS))
        {
            printf("%s:Error: Can't add interface to default vlan. error code: %d\n",
                   __FUNCTION__, retVal);
            return retVal;
        }
    }

    retVal = xpsVlanSetDefault(devId, XPSAPP_DEFAULT_VLAN);
    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error: Can't set default pvid. error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    return XP_NO_ERR;

}
uint8_t xpsAppSetVlanParams(xpDevice_t devId, xpsVlan_t vlanId,
                            xpsAppVlanParams * vlanParams, uint32_t numOfMvifEntry)
{

    xpsPktCmd_e pktCmd;
    int macSALSBs;
    uint32_t vrfId = 0xFF;
    XP_STATUS retVal = XP_NO_ERR;
    xpVlanBridgeMcMode_e bridgeMode;
    xpVlanRouteMcMode_t routeMode;
    uint8_t mplsRouteEn, natMode, natScope;
    xpsInterfaceId_t l3intfId;
    xpsScope_t scopeId;

    retVal = xpsScopeGetScopeId(devId, &scopeId);

    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in xpsScopeGetScopeId : error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }
    retVal = xpsL3CreateVlanIntfScope(scopeId, vlanId, &l3intfId);
    vlanToL3Intf[vlanId] = (uint32_t)l3intfId;

    if ((retVal != XP_NO_ERR) && (retVal != XP_ERR_KEY_EXISTS))
    {
        return retVal;
    }
    //program ipv4RouteEn
    pktCmd = (xpsPktCmd_e)(vlanParams->ipv4RouteEn);
    printf("xpsApp: ipv4RouteEn = %d\n", pktCmd);
    if ((retVal = xpsL3SetIntfIpv4UcRoutingEn(devId, l3intfId,
                                              pktCmd)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv4UcRoutingEn() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }
    //verify ipv4RouteEn
    uint32_t routeEn = 0;
    if ((retVal = xpsL3GetIntfIpv4UcRoutingEn(devId, l3intfId,
                                              &routeEn)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3IntGetIpv4UcRoutingEn() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    if (vlanParams->ipv4RouteEn)
    {
        //If ipv4 routing in enabled then program ipv4 urpf configurations
        //Set ipv4 urpf configurations
        if ((retVal = xpsL3SetIpv4UrpfEnable(devId, l3intfId,
                                             (uint32_t)(vlanParams->ipv4UrpfEn))) != XP_NO_ERR)
        {
            //In case of error print error message, destroy pointer and return from function
            printf("%s:Error: xpsL3SetIpv4UrpfEnable() for vlan %d failed with error code: %d\n",
                   __FUNCTION__, vlanId, retVal);
            return retVal;
        }
        if ((retVal = xpsL3SetIpv4UrpfMode(devId, l3intfId,
                                           (xpsUrpfMode_e)(vlanParams->ipv4UrpfMode))) != XP_NO_ERR)
        {
            //In case of error print error message, destroy pointer and return from function
            printf("%s:Error: xpsL3SetIpv4UrpfMode() for vlan %d failed with error code: %d\n",
                   __FUNCTION__, vlanId, retVal);
            return retVal;
        }
    }


    //program ipv6RouteEn
    pktCmd = (xpsPktCmd_e)(vlanParams->ipv6RouteEn);
    printf("xpsApp: ipv6RouteEn = %d\n", pktCmd);
    if ((retVal = xpsL3SetIntfIpv6UcRoutingEn(devId, l3intfId,
                                              pktCmd)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv6UcRoutingEn() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    //verify ipv6RouteEn
    routeEn = 0;
    if ((retVal = xpsL3GetIntfIpv6UcRoutingEn(devId, l3intfId,
                                              &routeEn)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3GetIntfIpv6UcRoutingEn() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    if (vlanParams->ipv6RouteEn)
    {
        //Set ipv6 urpf configurations
        if ((retVal = xpsL3SetIpv6UrpfEnable(devId, l3intfId,
                                             (uint32_t)(vlanParams->ipv6UrpfEn))) != XP_NO_ERR)
        {
            //In case of error print error message, destroy pointer and return from function
            printf("%s:Error: xpsL3SetIpv6UrpfEnable() for vlan %d failed with error code: %d\n",
                   __FUNCTION__, vlanId, retVal);
            return retVal;
        }
        if ((retVal = xpsL3SetIpv6UrpfMode(devId, l3intfId,
                                           (xpsUrpfMode_e)(vlanParams->ipv6UrpfMode))) != XP_NO_ERR)
        {
            //In case of error print error message, destroy pointer and return from function
            printf("%s:Error: xpsL3SetIpv6UrpfMode() for vlan %d failed with error code: %d\n",
                   __FUNCTION__, vlanId, retVal);
            return retVal;
        }
    }


    //program mpslRouteEn. Enable always for now
    mplsRouteEn=(uint32_t)(vlanParams->mplsRouteEn);
    if ((retVal = xpsL3SetIntfMplsRoutingEn(devId, l3intfId,
                                            mplsRouteEn)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv6UcRoutingEn() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }
    printf("xpsApp: mplsRouteEn = %d on vlan: %d\n", mplsRouteEn, vlanId);

    // program nat enable

    //program nat mode
    natMode = (uint32_t)(vlanParams->natMode);
    natScope = (uint32_t)(vlanParams->natScope);

    //if (xpAppConf.profileNum != XP_DEFAULT_TWO_PIPE_PROFILE)
    {
        printf("xpsL3SetIntfNatMode = natMode = %d\n", natMode);
        if ((retVal = xpsL3SetIntfNatMode(devId, l3intfId,
                                          (xpsVlanNatMode_e) natMode)) != XP_NO_ERR)
        {
            printf("%s:Error: xpsVlanSetNatMode() for vlan %d failed with error code: %d\n",
                   __FUNCTION__, vlanId, retVal);
            return retVal;
        }


        //program nat scope
        printf("xpsL3SetIntfNatScope = natScope = %d\n", natScope);
        if ((retVal = xpsL3SetIntfNatScope(devId, l3intfId, natScope)) != XP_NO_ERR)
        {
            printf("%s:Error: xpsVlanSetNatScope() for vlan %d failed with error code: %d\n",
                   __FUNCTION__, vlanId, retVal);
            return retVal;
        }
    }

    //program vrfId
    vrfId = (xpsPktCmd_e)(vlanParams->vrfId);
    printf("xpsAppSetVlanParams = vrfId = %d\n", vrfId);
    if ((retVal = xpsL3SetIntfVrf(devId, l3intfId, vrfId)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfVrf() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    //verify vrfId
    vrfId = 0;
    if ((retVal = xpsL3GetIntfVrf(devId, l3intfId, &vrfId)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3GetIntfVrf() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }
    printf("getVrf() returns %d\n", vrfId);

    //program ipv4BridgeMcMode
    bridgeMode = (xpsVlanBridgeMcMode_e)(vlanParams->ipv4BridgeMcMode);
    printf("xpsApp: ipv4BridgeMcMode = %d\n", bridgeMode);
    if ((retVal = xpsVlanSetIpv4McBridgeMode(devId, vlanId,
                                             bridgeMode)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsVlanSetIpv4McBridgeMode() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    //program ipv6BridgeMcMode
    bridgeMode = (xpsVlanBridgeMcMode_e)(vlanParams->ipv6BridgeMcMode);
    printf("xpsApp: ipv6BridgeMcMode = %d\n", bridgeMode);
    if ((retVal = xpsVlanSetIpv6McBridgeMode(devId, vlanId,
                                             bridgeMode)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsVlanSetIpv6McBridgeMode() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    //program ipv4RouteMcEn
    pktCmd = (xpsPktCmd_e)(vlanParams->ipv4RouteMcEn);
    printf("xpsApp: ipv4RouteMcEn = %d\n", pktCmd);
    if ((retVal = xpsL3SetIntfIpv4McRoutingEn(devId, l3intfId,
                                              pktCmd)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv4McRoutingEn() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    //program ipv6RouteMcEn
    pktCmd = (xpsPktCmd_e)(vlanParams->ipv6RouteMcEn);
    printf("xpsApp: ipv6RouteMcEn = %d\n", pktCmd);

    // For Now Make Ipv6 Routing Enable. Later this will come from config.txt
    if ((retVal = xpsL3SetIntfIpv6McRoutingEn(devId, l3intfId,
                                              pktCmd)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv6McRoutingEn() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    //program ipv4RouteMcMode
    routeMode = (xpVlanRouteMcMode_t)(vlanParams->ipv4RouteMcMode);
    printf("xpsApp: ipv4RouteMcMode = %d\n", routeMode);
    if ((retVal = xpsL3SetIntfIpv4McRouteMode(devId, l3intfId,
                                              routeMode)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv4McRouteMode() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    //program ipv6RouteMcMode
    routeMode = (xpVlanRouteMcMode_t)(vlanParams->ipv6RouteMcMode);
    printf("xpsApp: ipv6RouteMcMode = %d\n", routeMode);
    if ((retVal = xpsL3SetIntfIpv6McRouteMode(devId, l3intfId,
                                              routeMode)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsVlanSetIpv6McRouteMode() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }
    //program Unregister Mc Cmd
    pktCmd = (xpsPktCmd_e)(vlanParams->unregMcCmd);
    printf("xpsAppSetVlanParams : vlanId = %d unregMcCmd = %d\n", vlanId, pktCmd);
    if ((retVal = xpsVlanSetUnregMcCmd(devId, vlanId, pktCmd)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsVlanSetUnregMcCmd() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }
    pktCmd = (xpsPktCmd_e)(vlanParams->enL3Inf);
    macSALSBs = vlanParams->macSALSBs ;
    printf("xpsAppSetVlanParams : vlanId = %d enL3Inf = %d macSALSBs = %x \n",
           vlanId, pktCmd, macSALSBs);
    if (pktCmd == 1)
    {
        if ((retVal = xpsL3SetIntfEgressRouterMacLSB(devId, l3intfId,
                                                     macSALSBs)) != XP_NO_ERR)
        {
            printf("%s:Error: xpsL3SetIntfEgressRouterMacLSB() for vlan %d failed with error code: %d\n",
                   __FUNCTION__, vlanId, retVal);
            return retVal;
        }
    }

    //program ipv4MtuLen
    printf("xpsApp: ipv4MtuLen = %d\n", vlanParams->ipv4MtuLen);
    if ((retVal = xpsL3SetIpv4MtuLenForInterface(devId, l3intfId,
                                                 vlanParams->ipv4MtuLen)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIpv4MtuLenForInterface() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    //program ipv6MtuLen
    printf("xpsApp: ipv6MtuLen = %d\n", vlanParams->ipv6MtuLen);
    if ((retVal = xpsL3SetIpv6MtuLenForInterface(devId, l3intfId,
                                                 vlanParams->ipv6MtuLen)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIpv6MtuLenForInterface() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    //program mtuPktCmd
    printf("xpsApp: mtuPktCmd = %d\n", vlanParams->mtuPktCmd);
    if ((retVal = xpsL3SetMtuPktCmdForInterface(devId, l3intfId,
                                                vlanParams->mtuPktCmd)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetMtuPktCmdForInterface() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    // Program mvif and unreg MC Flood List
    if (vlanParams->mcVifIdx != -1)
    {
        uint32_t i, entryNum;
        xpsMcL2InterfaceListId_t ifListId;

        if ((retVal = xpsMulticastCreateL2InterfaceListScope(scopeId, vlanId,
                                                             &ifListId)) != XP_NO_ERR)
        {
            printf("%s:Error:xpsMulticastCreateL2InterfaceListScope for vlan %d failed with error code: %d\n",
                   __FUNCTION__, vlanId, retVal);
            return retVal;
        }
        if ((retVal = xpsMulticastAddL2InterfaceListToDevice(devId,
                                                             ifListId)) != XP_NO_ERR)
        {
            printf("%s:Error:xpsMulticastAddL2InterfaceListToDevice for vlan %d failed with error code: %d\n",
                   __FUNCTION__, vlanId, retVal);
            return retVal;
        }

        /* Populate mvif entries */
        for (i = 0; i < 10; i++)
        {
            if (vlanParams->mvifIdx[i] == 0)
            {
                break;
            }

            for (entryNum = 0; entryNum < numOfMvifEntry; entryNum++)
            {
                if (((MvifEntry *)getEntry(&configFile.mvifData,
                                           entryNum))->index == vlanParams->mvifIdx[i])
                {
                    xpsAppConfigureMvifEntry(devId, (MvifEntry *)getEntry(&configFile.mvifData,
                                                                          entryNum), 0, ifListId, 1);
                    break;
                }
            }
        }

        // Store the multicast Vif in global db
        multicastVif[devId][XP_MULTICAST_TYPE_VLAN][vlanParams->mcastIdx] = ifListId;

        if ((retVal = xpsVlanSetIPv4UnregMcastL2InterfaceList(devId, vlanId,
                                                              (xpsMcL2InterfaceListId_t)ifListId)) != XP_NO_ERR)
        {
            printf("%s:Error:xpsVlanSetIPv4UnregMcastL2InterfaceList for vlan %d failed with error code: %d\n",
                   __FUNCTION__, vlanId, retVal);
            return retVal;
        }
        if ((retVal = xpsVlanSetIPv6UnregMcastL2InterfaceList(devId, vlanId,
                                                              (xpsMcL2InterfaceListId_t)ifListId)) != XP_NO_ERR)
        {
            printf("%s:Error:xpsVlanSetIPv6UnregMcastL2InterfaceList for vlan %d failed with error code: %d\n",
                   __FUNCTION__, vlanId, retVal);
            return retVal;
        }
    }

    return XP_NO_ERR;
}


uint8_t xpsAppSetPortVlanParams(xpDevice_t devId, xpVlan_t vlanId,
                                xpsInterfaceId_t intfId, xpsAppPortVlanParams * portVlan)
{

    XP_STATUS retVal = XP_NO_ERR;

    if ((retVal = xpsVlanSetOpenFlowEnable(devId, vlanId, intfId,
                                           portVlan->enOpenFlow)))
    {
        printf("%s:Error: xpsVlanSetHairpin() failed with error code: %d\n",
               __FUNCTION__, retVal);
        return retVal;
    }

    if (portVlan->enVlanHairPining == 1)
    {
        if ((retVal = xpsVlanSetHairpin(devId, vlanId, intfId, 1)))
        {
            printf("%s:Error: xpsVlanSetHairpin() failed with error code: %d\n",
                   __FUNCTION__, retVal);
            return retVal;
        }
    }
    return XP_NO_ERR;

}

XP_STATUS xpsAppProgarmTrustBr(xpsDevice_t devId, TrustBrEntry *trustBrEntry)
{
    XP_STATUS retVal = XP_NO_ERR;
    uint16_t i=0, j=0;
    xpsScope_t scopeId;
    xpsInterfaceId_t stripEtagIntfId;
    xpLag_t lagId;
    xpsLagPortIntfList_t portIntfList;

    retVal = xpsScopeGetScopeId(devId, &scopeId);

    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in xpsScopeGetScopeId : error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    for (i=0; i < trustBrEntry->numOflongEtagModePort; i++)
    {
        retVal = xpsPortEnableLongEtagMode(devId, trustBrEntry->longEtagModePort[i], 1);
        if (retVal != XP_NO_ERR)
        {
            printf("Error : xpInterfaceEnableLongEtagMode() Failed for interface type :%d devId %d\n",
                   XPS_PORT, devId);
            return retVal;
        }
    }

    if (trustBrEntry->numOfAddEtagPorts)
    {
        for (i=0; i < trustBrEntry->numOfAddEtagPorts; i++)
        {
            // Give urw intruction to add etag in the egress packet
            retVal = xpsPortAddEtag(devId, trustBrEntry->addEtagPorts[i], 1);
            if (retVal != XP_NO_ERR)
            {
                printf("Error : xpsPortAddEtag failed for portVif :%d devId %d\n",
                       trustBrEntry->addEtagPorts[i], devId);
                return retVal;
            }
        }
    }

    if (trustBrEntry->numOfKeepEtagPorts)
    {
        for (i=0; i < trustBrEntry->numOfKeepEtagPorts; i++)
        {
            retVal = xpsPortKeepEtag(devId, trustBrEntry->keepEtagPorts[i], 1);
            if (retVal != XP_NO_ERR)
            {
                printf("Error : xpsPortKeepEtag() Failed for portVif :%d devId %d\n",
                       trustBrEntry->keepEtagPorts[i], devId);
                return retVal;
            }
        }
    }

    if (trustBrEntry->numOfAddEtagLags)
    {
        for (i=0; i < trustBrEntry->numOfAddEtagLags; i++)
        {
            lagId = gLagIntfId[devId][trustBrEntry->addEtagLags[i]];
            retVal = xpsLagGetIngressPortIntfList(scopeId, lagId, &portIntfList);
            if (retVal != XP_NO_ERR)
            {
                printf("Error : xpsLagGetIngressPortIntfList failed for lagId :%d devId %d\n",
                       lagId, devId);
                return retVal;
            }

            for (j=0; j < portIntfList.size; j++)
            {
                stripEtagIntfId = portIntfList.portIntf[j];

                retVal = xpsPortAddEtag(devId, stripEtagIntfId, 1);
                if (retVal != XP_NO_ERR)
                {
                    printf("Error : xpsPortAddEtag failed for portVif :%d devId %d\n",
                           stripEtagIntfId, devId);
                    return retVal;
                }
            }
        }
    }

    if (trustBrEntry->numOfKeepEtagLags)
    {
        for (i=0; i < trustBrEntry->numOfKeepEtagLags; i++)
        {
            lagId = gLagIntfId[devId][trustBrEntry->keepEtagLags[i]];
            retVal = xpsLagGetIngressPortIntfList(scopeId, lagId, &portIntfList);
            if (retVal != XP_NO_ERR)
            {
                printf("Error : xpsLagGetIngressPortIntfList failed for lagId :%d devId %d\n",
                       lagId, devId);
                return retVal;
            }

            for (j=0; j < portIntfList.size; j++)
            {
                stripEtagIntfId = portIntfList.portIntf[j];

                retVal = xpsPortKeepEtag(devId, stripEtagIntfId, 1);
                if (retVal != XP_NO_ERR)
                {
                    printf("Error : xpsPortKeepEtag() Failed for portVif :%d devId %d\n",
                           stripEtagIntfId, devId);
                    return retVal;
                }
            }
        }
    }

    if (trustBrEntry->numOfStripEtagLagVif)
    {
        xpLag_t lagId;

        for (i=0; i < trustBrEntry->numOfStripEtagLagVif; i++)
        {
            lagId = gLagIntfId[devId][trustBrEntry->stripEtagLagVif[i]];
            // Enable etag stripping for egress interface.
            //retVal = xpsInterfaceStripIncomingEtag(devId, lagId, 1);
            if (retVal != XP_NO_ERR)
            {
                printf("Error : xpsInterfaceStripIncomingEtag failed for lag Id : %d devId %d\n",
                       lagId, devId);
                return retVal;
            }
        }
    }

    if (trustBrEntry->numOfStripEtagVif)
    {
        for (i=0; i < trustBrEntry->numOfStripEtagVif; i++)
        {
            // Enable etag stripping for egress interface.
            //retVal = xpsInterfaceStripIncomingEtag(devId, trustBrEntry->stripEtagVif[i], 1);
            if (retVal != XP_NO_ERR)
            {
                printf("Error : xpsInterfaceStripIncomingEtag failed for vif : %d devId %d\n",
                       trustBrEntry->stripEtagVif[i], devId);
                return retVal;
            }
        }
    }

    return retVal;
}

XP_STATUS xpsAppConfigureVlan(xpDevice_t devId, VlanEntry *vlanEntry,
                              uint32_t numOfMvifEntry)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpVlan_t vlanId = 0;
    uint32_t numOfPortEntry = 0, portEntryNum = 0, numOfLogicalPorts, logicalPort,
             createExtendedPorts =0;
    xpPort_t devPort;
    xpsInterfaceId_t intfId;
    xpsPeg_t groupId;
    vlanId = vlanEntry->vlan;
    int index=0;

    /*create vlan for this vlanId*/
    retVal = xpsVlanCreate(devId, vlanId);
    if ((retVal != XP_NO_ERR) && (retVal != XP_ERR_KEY_EXISTS))
    {
        printf("%s:Error:Filed to create vlan for vlanId %d | retVal : %d \n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }
    printf("%s:Info: allocated Vlan %d successfully in device %d\n", __FUNCTION__,
           vlanId, devId);

    if (vlanEntry->parentVlan != 0)
    {
        if (vlanEntry->vlanType == 0)
        {
            retVal = xpsPVlanCreatePrimary(devId, vlanId);
            if (retVal != XP_NO_ERR)
            {
                printf("Error : xpsPVlanCreatePrimary() Failed Error : %d for devId %d\n",
                       retVal, devId);
                return retVal;
            }
        }
        else if (vlanEntry->vlanType == 1)
        {
            retVal = xpsPVlanCreateSecondary(devId, vlanId, vlanEntry->parentVlan,
                                             (xpsPrivateVlanType_e)(vlanEntry->vlanType+1));
            if (retVal != XP_NO_ERR)
            {
                printf("Error : xpsPVlanCreateSecondary() Failed Error : %d for devId %d\n",
                       retVal, devId);
                return retVal;
            }
        }
        else if (vlanEntry->vlanType == 2)
        {
            retVal = xpsPVlanCreateSecondary(devId, vlanId, vlanEntry->parentVlan,
                                             (xpsPrivateVlanType_e)(vlanEntry->vlanType+1));
            if (retVal != XP_NO_ERR)
            {
                printf("Error : xpsPVlanCreateSecondary - community() Failed Error : %d for devId %d\n",
                       retVal, devId);
                return retVal;
            }
        }
    }

    xpsAppVlanParams *vlanParams = (xpsAppVlanParams *)malloc(sizeof(
                                                                  xpsAppVlanParams));
    memset(vlanParams, 0x0, sizeof(xpsAppVlanParams));
    vlanParams->ipv4RouteEn =  vlanEntry->ipv4RouteEn;
    vlanParams->ipv6RouteEn =  vlanEntry->ipv6RouteEn;
    vlanParams->ipv4BridgeMcMode =  vlanEntry->ipv4BridgeMcMode;
    vlanParams->ipv6BridgeMcMode =  vlanEntry->ipv6BridgeMcMode;
    vlanParams->ipv4RouteMcEn =  vlanEntry->ipv4RouteMcEn;
    vlanParams->ipv6RouteMcEn =  vlanEntry->ipv6RouteMcEn;
    vlanParams->mplsRouteEn =  vlanEntry->mplsRouteEn;
    vlanParams->ipv4RouteMcMode =  vlanEntry->ipv4RouteMcMode;
    vlanParams->ipv6RouteMcMode =  vlanEntry->ipv6RouteMcMode;
    vlanParams->vrfId =  vlanEntry->vrfId;
    vlanParams->unregMcCmd =  vlanEntry->unregMcCmd;
    vlanParams->enL3Inf =  vlanEntry->enL3Inf;
    vlanParams->macSALSBs =  vlanEntry->macSALSBs;
    vlanParams->natEn =  vlanEntry->natEn;
    vlanParams->natMode =  vlanEntry->natMode;
    vlanParams->natScope =  vlanEntry->natScope;
    vlanParams->ipv4UrpfEn = vlanEntry->ipv4UrpfEn;
    vlanParams->ipv4UrpfMode = vlanEntry->ipv4UrpfMode;
    vlanParams->ipv6UrpfEn = vlanEntry->ipv6UrpfEn;
    vlanParams->ipv6UrpfMode = vlanEntry->ipv6UrpfMode;
    vlanParams->ipv4MtuLen = vlanEntry->ipv4MtuLen;
    vlanParams->ipv6MtuLen = vlanEntry->ipv6MtuLen;
    vlanParams->mtuPktCmd = vlanEntry->mtuPktCmd;
    for (index=0; index<10; index++)
    {
        vlanParams->mvifIdx[index] = vlanEntry->mvifIdx[index];
    }
    vlanParams->mcVifIdx = vlanEntry->mcVifIdx;
    vlanParams->mcastIdx = vlanEntry->mcastIdx;
    xpsStp_t stpId = 0;

    //In a scope one vlan for all devices belong to same STP.
    //Get the stpId for the first device, same vlan, considering it belongs to same scope
    xpsScope_t scopeId;
    xpsDevice_t firstDevId;
    retVal = xpsScopeGetScopeId(devId, &scopeId);

    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in xpsScopeGetScopeId : error code: %d\n", __FUNCTION__,
               retVal);
        free(vlanParams);
        return retVal;
    }

    retVal = xpsScopeGetFirstDevice(scopeId, &firstDevId);
    if (retVal != XP_NO_ERR)
    {
        printf("Error : xpsScopeGetFirstDevice() Failed %d\n", retVal);
        free(vlanParams);
        return retVal;
    }

    //If a previous device was found in this scope then get stp for the same vlan
    if (firstDevId != XP_MAX_DEVICES)
    {
        retVal = xpsVlanGetStp(firstDevId, vlanId, &stpId);
        if (retVal != XP_NO_ERR)
        {
            printf("Error : xpsVlanGetStp() Failed for devId %d vlanId %d\n", firstDevId,
                   vlanId);
            free(vlanParams);
            return retVal;
        }

        //Check stp existence in Scope.
        retVal = xpsStpIsExist(scopeId, stpId);
        if ((retVal != XP_NO_ERR) && (retVal != XP_ERR_RESOURCE_NOT_AVAILABLE)
            &&(vlanEntry->vlanType != 2))
        {
            printf("Error : xpsStpIsExist() Failed for devId %d vlanId %d\n", devId,
                   vlanId);
            free(vlanParams);
            return retVal;
        }

        //Skip stp allocation for isolated vlan or if already allocated for a vlan in another device
        if ((vlanEntry->vlanType != 2)
            && ((retVal == XP_ERR_RESOURCE_NOT_AVAILABLE) || (stpId == 0)))
        {
            retVal = xpsStpCreateScope(scopeId, &stpId);
            if (retVal != XP_NO_ERR)
            {
                printf("Error : xpsStpCreate() Failed for devId %d\n", devId);
                free(vlanParams);
                return retVal;
            }
        }
    }
    else
    {
        //Skip stp allocation for isolated vlan or if already allocated for a vlan in another device
        if ((vlanEntry->vlanType != 2) && (stpId == 0))
        {
            retVal = xpsStpCreateScope(scopeId, &stpId);
            if (retVal != XP_NO_ERR)
            {
                printf("Error : xpsStpCreate() Failed for devId %d\n", devId);
                free(vlanParams);
                return retVal;
            }
        }
    }

    xpsAppPortVlanParams *portVlan = (xpsAppPortVlanParams *)malloc(sizeof(
                                                                        xpsAppPortVlanParams));
    memset(portVlan, 0x0, sizeof(xpsAppPortVlanParams));

    numOfPortEntry = vlanEntry->numPorts;
    for (portEntryNum = 0; portEntryNum < numOfPortEntry; portEntryNum++)
    {
        void *r = NULL;
        devPort = vlanEntry->portData[portEntryNum].port;
        portVlan->tagType = vlanEntry->portData[portEntryNum].tagType;
        portVlan->stpState = (xpVlanStgState_e)
                             vlanEntry->portData[portEntryNum].stpState;
        portVlan->setIngressVif = vlanEntry->portData[portEntryNum].setIngressVif;
        portVlan->disableTunnelVif = vlanEntry->portData[portEntryNum].disableTunnelVif;
        portVlan->mirrorToAnalyzerMask =
            vlanEntry->portData[portEntryNum].mirrorToAnalyzerMask;
        portVlan->setBd = vlanEntry->portData[portEntryNum].setBd;
        portVlan->enVlanHairPining = vlanEntry->portData[portEntryNum].enVlanHairPining;
        portVlan->enPbb = vlanEntry->portData[portEntryNum].enPbb;
        portVlan->natMode = vlanEntry->portData[portEntryNum].natMode;
        portVlan->natScope = vlanEntry->portData[portEntryNum].natScope;
        portVlan->enRouterACL = vlanEntry->portData[portEntryNum].enRouterACL;
        portVlan->routeAclId = vlanEntry->portData[portEntryNum].routeAclId;
        portVlan->enBridgeACL = vlanEntry->portData[portEntryNum].enBridgeACL;
        portVlan->bridgeAclId = vlanEntry->portData[portEntryNum].bridgeAclId;
        portVlan->enIpv4Tunnel = vlanEntry->portData[portEntryNum].enIpv4Tunnel;
        portVlan->enMplsTunnel = vlanEntry->portData[portEntryNum].enMplsTunnel;
        portVlan->enOpenFlow = vlanEntry->portData[portEntryNum].enOpenFlow;
        portVlan->setEgressVif = vlanEntry->portData[portEntryNum].setEgressVif;
        portVlan->evif = vlanEntry->portData[portEntryNum].evif;
        portVlan->encapType = vlanEntry->portData[portEntryNum].encapType;
        /* if numOfExtPorts = 0 then no port extender is connected to physical port */
        numOfLogicalPorts = vlanEntry->portData[portEntryNum].numOfExtPorts;

        intfId = 0xFFFFFFFF;
        if (!numOfLogicalPorts)
        {
            retVal = xpsPortGetPortIntfId(devId, devPort, &intfId);
            if (retVal)
            {
                printf("Error : xpsPortGetPortIntfId() Failed for interfac type :%d devId %d\n",
                       XPS_PORT, devId);
                free(vlanParams);
                free(portVlan);
                return retVal;
            }
            numOfLogicalPorts = 1;
        }
        /* Extended port is connected to physical port*/
        else
        {
            r = xpsAppGetExtPortsGroup(devId, devPort);
            if (r == NULL)
            {
                /* Create extended port group */
                retVal = xps8021BrCreatePortExtenderGroupScope(scopeId, XP_8021BR_MAX_PER_GROUP,
                                                               &groupId);
                if (retVal)
                {
                    printf("Error : xps8021BrCreatePortExtenderGroup() Failed for devPort :%d devId %d\n",
                           devPort, devId);
                    free(vlanParams);
                    free(portVlan);
                    return retVal;
                }

                extPortGroup *tmpCtx = (extPortGroup *)malloc(sizeof(extPortGroup));
                if (!tmpCtx)
                {
                    printf("%s:Error: Can't allocate extPortGroup context for port Index: %d\n",
                           __FUNCTION__, devPort);
                    free(vlanParams);
                    free(portVlan);
                    return XP_ERR_INIT;
                }
                tmpCtx->groupId = groupId;
                tmpCtx->key = devPort;
                tmpCtx->intfId = (xpsInterfaceId_t *)malloc(numOfLogicalPorts * sizeof(
                                                                xpsInterfaceId_t));
                tmpCtx->numOfExtendedPorts = numOfLogicalPorts;
                r = tsearch(tmpCtx, &extPortsRoot[devId], compareExtPortsMap);
                if (r == NULL)
                {
                    printf("%s:Error: Can't insert extPortGroup ctx in tree for port %d\n",
                           __FUNCTION__, tmpCtx->key);
                    free(tmpCtx->intfId);
                    free(tmpCtx);
                    free(vlanParams);
                    free(portVlan);
                    return XP_ERR_INIT;
                }
                createExtendedPorts = 1;
            }
        }

        for (logicalPort = 0; logicalPort < numOfLogicalPorts; logicalPort++)
        {
            if (vlanEntry->portData[portEntryNum].numOfExtPorts > 0)
            {
                if (createExtendedPorts)
                {
                    uint32_t maxCascadePorts = 1;
                    retVal = xps8021BrCreateExtendedPortScope(scopeId, groupId, maxCascadePorts,
                                                              &intfId);
                    if (retVal)
                    {
                        printf("Error : xps8021BrCreateExtendedPort() Failed for group :%d devId %d\n",
                               groupId, devId);
                        free(vlanParams);
                        free(portVlan);
                        return retVal;
                    }

                    retVal = xps8021BrInitExtendedPort(devId, groupId, intfId);
                    if (retVal)
                    {
                        printf("Error : xps8021BrBindExtendedPort() Failed for group :%d devId %d intfId : %d\n",
                               groupId, devId, intfId);
                        free(vlanParams);
                        free(portVlan);
                        return retVal;
                    }

                    xpsInterfaceId_t cascadeIntfId;
                    retVal = xpsPortGetPortIntfId(devId, devPort, &cascadeIntfId);
                    if (retVal)
                    {
                        printf("Error : xpsPortGetPortIntfId() Failed for devId %d portId : %d\n",
                               devId, devPort);
                        free(vlanParams);
                        free(portVlan);
                        return retVal;
                    }

                    retVal = xps8021BrBindExtendedPortScope(scopeId, groupId, intfId,
                                                            cascadeIntfId);
                    if (retVal)
                    {
                        printf("Error : xps8021BrBindExtendedPort() Failed for group :%d devId %d\n",
                               groupId, devId);
                        free(vlanParams);
                        free(portVlan);
                        return retVal;
                    }
                    (*(extPortGroup **)r)->intfId[logicalPort] = intfId;
                }
                else
                {
                    intfId = (*(extPortGroup **)r)->intfId[logicalPort];
                }
            }

            if (vlanEntry->parentVlan == 0)
            {
                retVal = xpsVlanAddInterface(devId, vlanId, intfId,
                                             (xpsL2EncapType_e)portVlan->tagType);
                //To take care of tagged interfaces in vlan 100
                if ((retVal == XP_ERR_KEY_EXISTS) && (vlanId == XPSAPP_DEFAULT_VLAN))
                {
                    retVal = xpsVlanRemoveInterface(devId, vlanId, intfId);
                    if (retVal != XP_NO_ERR)
                    {
                        printf("Error : xpsVlanRemoveInterface() Failed Error : %d for devId %d vlanId %d intfId %d\n",
                               retVal, devId, vlanId, intfId);
                        free(vlanParams);
                        free(portVlan);
                        return retVal;
                    }

                    retVal = xpsVlanAddInterface(devId, vlanId, intfId,
                                                 (xpsL2EncapType_e)portVlan->tagType);
                    if (retVal != XP_NO_ERR)
                    {
                        printf("Error : xpsVlanAddInterface() Failed Error : %d for devId %d vlanId %d intfId %d tagType %d\n",
                               retVal, devId, vlanId, intfId, portVlan->tagType);
                        free(vlanParams);
                        free(portVlan);
                        return retVal;
                    }
                }
                if (retVal != XP_NO_ERR)
                {
                    printf("Error : xpsVlanAddInterface() Failed Error : %d for devId %d vlanId %d intfId %d tagType %d\n",
                           retVal, devId, vlanId, intfId, portVlan->tagType);
                    free(vlanParams);
                    free(portVlan);
                    return retVal;
                }
            }
            else if (vlanEntry->parentVlan != 0) //Private Vlan Interface
            {
                if (vlanEntry->vlanType == 0)
                {
                    retVal = xpsPVlanAddInterfacePrimary(devId, vlanId, intfId,
                                                         (xpsL2EncapType_e) portVlan->tagType);
                    if (retVal != XP_NO_ERR)
                    {
                        printf("Error : xpsPVlanAddInterfacePrimary() Failed Error : %d for devId %d\n",
                               retVal, devId);
                        free(vlanParams);
                        free(portVlan);
                        return retVal;
                    }
                }
                else if (vlanEntry->vlanType == 1)
                {
                    retVal = xpsPVlanAddInterfaceCommunity(devId, vlanEntry->parentVlan, vlanId,
                                                           intfId, (xpsL2EncapType_e) portVlan->tagType);
                    if (retVal != XP_NO_ERR)
                    {
                        printf("Error : xpsPVlanRemoveInterfaceCommunity() Failed Error : %d for devId %d\n",
                               retVal, devId);
                        free(vlanParams);
                        free(portVlan);
                        return retVal;
                    }
                }
                else if (vlanEntry->vlanType == 2)
                {
                    retVal = xpsPVlanAddInterfaceIsolated(devId, vlanEntry->parentVlan, vlanId,
                                                          intfId, (xpsL2EncapType_e) portVlan->tagType);
                    if (retVal != XP_NO_ERR)
                    {
                        printf("Error : xpsVlanAddInterface() Failed Error : %d for devId %d\n", retVal,
                               devId);
                        free(vlanParams);
                        free(portVlan);
                        return retVal;
                    }
                }
            }

            xpsAppSetPortVlanParams(devId, vlanId, intfId, portVlan);

            //Filter the isolated vlan port
            if (vlanEntry->vlanType == 2)
            {
                //1. Put all ports in blocking
                //2. Also have the facility to set ingress stp state only
                //xpsVlanSetStpState(devId, intfId, pVlanIvifIdx, vlanCtx->stgIdx, SPAN_STATE_DISABLE, 0);
            }
            else
            {
                //set the port VLAN state to FORWARDING (by default it would be BLOCKING);
                //egressFilterID's bitmask to set the port's bit if STG is DISABLED or FORWARD, and reset if LEARN or BLOCK
                if ((retVal = xpsStpSetState(devId, stpId, intfId,
                                             (xpsStpState_e) portVlan->stpState)) != XP_NO_ERR)
                {
                    printf("%s:Error: xpsStpSetState() for port %d, vlan %d failed with error code: %d\n",
                           __FUNCTION__, intfId, vlanId, retVal);
                    free(vlanParams);
                    free(portVlan);
                    return retVal;
                }
            }
            if (portVlan->setEgressVif)
            {
                uint32_t enRedirect =1;
                xpsInterfaceId_t sIfId;
                xpsInterfaceId_t dIfId;

                retVal = xpsPortGetPortIntfId(devId, devPort, &sIfId);
                if (retVal)
                {
                    printf("Error : xpsPortGetPortIntfId() Failed for interfac type :%d devId %d\n",
                           XPS_PORT, devId);
                    free(vlanParams);
                    free(portVlan);
                    return retVal;
                }

                retVal = xpsPortGetPortIntfId(devId, portVlan->evif, &dIfId);
                if (retVal)
                {
                    printf("Error : xpsPortGetPortIntfId() Failed for interfac type :%d devId %d\n",
                           XPS_PORT, devId);
                    free(vlanParams);
                    free(portVlan);
                    return retVal;
                }

                retVal = xpsVlanSetRedirectToInterface(devId, vlanId, sIfId, enRedirect, dIfId,
                                                       (xpVlanEncap_t) portVlan->encapType);
                if (retVal != XP_NO_ERR)
                {
                    printf("Error : xpsVlanSetRedirectToInterface() Failed Error : %d for devId %d\n",
                           retVal, devId);
                    free(vlanParams);
                    free(portVlan);
                    return retVal;
                }
            }
            printf("Port : %d added to Vlan : %d\n", intfId, vlanId);
        }// for loop

    }//for loop

    xpsVlanConfig_t *vlanConfig = (xpsVlanConfig_t *)malloc(sizeof(
                                                                xpsVlanConfig_t));
    memset(vlanConfig, 0x0, sizeof(xpsVlanConfig_t));
    retVal = xpsVlanGetConfig(devId, vlanId, vlanConfig);
    if (retVal != XP_NO_ERR)
    {
        printf("Error : xpsVlanGetConfig() Failed for devId %d and vlanId %d \n", devId,
               vlanId);
        free(vlanParams);
        free(portVlan);
        free(vlanConfig);
        return retVal;
    }
    vlanConfig->stpId = stpId;
    vlanConfig->saMissCmd = (xpsPktCmd_e)vlanEntry->macSAmissCmd;
    vlanConfig->bcCmd = (xpsPktCmd_e)vlanEntry->broadcastCmd;
    vlanConfig->unknownUcCmd = (xpsPktCmd_e)vlanEntry->broadcastCmd;
    vlanConfig->arpBcCmd = (xpsPktCmd_e)vlanEntry->ipv4ARPBCCmd;
    retVal = xpsVlanSetConfig(devId, vlanId, vlanConfig);
    if (retVal != XP_NO_ERR)
    {
        printf("Error : xpsVlanSetConfig() Failed for devId %d and vlanId %d \n", devId,
               vlanId);
        free(vlanParams);
        free(portVlan);
        free(vlanConfig);
        return retVal;
    }

    retVal = (XP_STATUS)xpsAppSetVlanParams(devId, vlanId, vlanParams,
                                            numOfMvifEntry);
    if (retVal != XP_NO_ERR)
    {
        printf("Error : xpsAppSetVlanParams() Failed for devId %d\n", devId);
        free(vlanParams);
        free(portVlan);
        free(vlanConfig);
        return retVal;
    }

    printf("%s:Info: Vlan entries programmed successfully\n", __FUNCTION__);
    free(vlanParams);
    free(vlanConfig);
    free(portVlan);

    return retVal;
}

XP_STATUS xpsAppConfigureVlanScope(xpDevice_t devId,
                                   VlanScopeEntry *vlanScopeEntry, uint32_t numOfMvifEntry)
{
    XP_STATUS retVal = XP_NO_ERR;
    VlanEntry *vlanEntry = NULL;
    xpsScope_t scopeId;
    uint32_t numOfPortEntry = 0, portEntryNum = 0;

    vlanEntry = (VlanEntry *)malloc(sizeof(VlanEntry));
    if (vlanEntry == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    memset(vlanEntry, 0, sizeof(VlanEntry));

    vlanEntry->vlan = vlanScopeEntry->vlan;
    vlanEntry->macSAmissCmd = vlanScopeEntry->macSAmissCmd;
    vlanEntry->broadcastCmd = vlanScopeEntry->broadcastCmd;
    vlanEntry->ipv4ARPBCCmd = vlanScopeEntry->ipv4ARPBCCmd;
    vlanEntry->ipv4RouteEn = vlanScopeEntry->ipv4RouteEn;
    vlanEntry->ipv6RouteEn = vlanScopeEntry->ipv6RouteEn;
    vlanEntry->ipv4BridgeMcMode = vlanScopeEntry->ipv4BridgeMcMode;
    vlanEntry->ipv6BridgeMcMode = vlanScopeEntry->ipv6BridgeMcMode;
    vlanEntry->ipv4RouteMcEn = vlanScopeEntry->ipv4RouteMcEn;
    vlanEntry->ipv6RouteMcEn = vlanScopeEntry->ipv6RouteMcEn;
    vlanEntry->mplsRouteEn = vlanScopeEntry->mplsRouteEn;
    vlanEntry->ipv4RouteMcMode = vlanScopeEntry->ipv4RouteMcMode;
    vlanEntry->ipv6RouteMcMode = vlanScopeEntry->ipv6RouteMcMode;
    vlanEntry->vrfId = vlanScopeEntry->vrfId;
    vlanEntry->unregMcCmd = vlanScopeEntry->unregMcCmd;
    vlanEntry->numPorts = vlanScopeEntry->numPorts;
    vlanEntry->vlanType = vlanScopeEntry->vlanType;
    vlanEntry->parentVlan = vlanScopeEntry->parentVlan;
    vlanEntry->enL3Inf = vlanScopeEntry->enL3Inf;
    vlanEntry->macSALSBs = vlanScopeEntry->macSALSBs;
    vlanEntry->natMode = vlanScopeEntry->natMode;
    vlanEntry->natScope = vlanScopeEntry->natScope;
    vlanEntry->natEn = vlanScopeEntry->natEn;
    vlanEntry->ipv4UrpfEn = vlanScopeEntry->ipv4UrpfEn;
    vlanEntry->ipv4UrpfMode = vlanScopeEntry->ipv4UrpfMode;
    vlanEntry->ipv6UrpfEn = vlanScopeEntry->ipv6UrpfEn;
    vlanEntry->ipv6UrpfMode = vlanScopeEntry->ipv6UrpfMode;
    vlanEntry->ipv4MtuLen = vlanScopeEntry->ipv4MtuLen;
    vlanEntry->ipv6MtuLen = vlanScopeEntry->ipv6MtuLen;
    vlanEntry->mtuPktCmd = vlanScopeEntry->mtuPktCmd;

    numOfPortEntry = vlanScopeEntry->numPorts;
    for (portEntryNum = 0; portEntryNum < numOfPortEntry; portEntryNum++)
    {
        vlanEntry->portData[portEntryNum].port =
            vlanScopeEntry->portData[portEntryNum].port;
        vlanEntry->portData[portEntryNum].tagType =
            vlanScopeEntry->portData[portEntryNum].tagType;
        vlanEntry->portData[portEntryNum].numOfExtPorts =
            vlanScopeEntry->portData[portEntryNum].numOfExtPorts;
        vlanEntry->portData[portEntryNum].stpState =
            vlanScopeEntry->portData[portEntryNum].stpState;
        vlanEntry->portData[portEntryNum].setIngressVif =
            vlanScopeEntry->portData[portEntryNum].setIngressVif;
        vlanEntry->portData[portEntryNum].disableTunnelVif =
            vlanScopeEntry->portData[portEntryNum].disableTunnelVif;
        vlanEntry->portData[portEntryNum].mirrorToAnalyzerMask =
            vlanScopeEntry->portData[portEntryNum].mirrorToAnalyzerMask;
        vlanEntry->portData[portEntryNum].setBd =
            vlanScopeEntry->portData[portEntryNum].setBd;
        vlanEntry->portData[portEntryNum].enVlanHairPining =
            vlanScopeEntry->portData[portEntryNum].enVlanHairPining;
        vlanEntry->portData[portEntryNum].enPbb =
            vlanScopeEntry->portData[portEntryNum].enPbb;
        vlanEntry->portData[portEntryNum].natMode =
            vlanScopeEntry->portData[portEntryNum].natMode;
        vlanEntry->portData[portEntryNum].natScope =
            vlanScopeEntry->portData[portEntryNum].natScope;
        vlanEntry->portData[portEntryNum].enRouterACL =
            vlanScopeEntry->portData[portEntryNum].enRouterACL;
        vlanEntry->portData[portEntryNum].routeAclId =
            vlanScopeEntry->portData[portEntryNum].routeAclId;
        vlanEntry->portData[portEntryNum].enBridgeACL =
            vlanScopeEntry->portData[portEntryNum].enBridgeACL;
        vlanEntry->portData[portEntryNum].bridgeAclId =
            vlanScopeEntry->portData[portEntryNum].bridgeAclId;
        vlanEntry->portData[portEntryNum].enIpv4Tunnel =
            vlanScopeEntry->portData[portEntryNum].enIpv4Tunnel;
        vlanEntry->portData[portEntryNum].enMplsTunnel =
            vlanScopeEntry->portData[portEntryNum].enMplsTunnel;
        vlanEntry->portData[portEntryNum].enOpenFlow =
            vlanScopeEntry->portData[portEntryNum].enOpenFlow;
        vlanEntry->portData[portEntryNum].setEgressVif =
            vlanScopeEntry->portData[portEntryNum].setEgressVif;
        vlanEntry->portData[portEntryNum].evif =
            vlanScopeEntry->portData[portEntryNum].evif;
        vlanEntry->portData[portEntryNum].encapType =
            vlanScopeEntry->portData[portEntryNum].encapType;
    }

    retVal = xpsScopeGetScopeId(devId, &scopeId);

    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in xpsScopeGetScopeId : error code: %d\n", __FUNCTION__,
               retVal);
        free(vlanEntry);
        return retVal;
    }

    if (scopeId == vlanScopeEntry->scopeId)
    {
        retVal = xpsAppConfigureVlan(devId, vlanEntry, numOfMvifEntry);
    }

    free(vlanEntry);
    return retVal;
}

XP_STATUS xpsAppConfigureLag(xpDevice_t devId, PortLagEntry *portLagEntry)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpLag_t lagId;
    uint32_t i = 0;
    xpsScope_t scopeId;

    retVal = xpsScopeGetScopeId(devId, &scopeId);

    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in xpsScopeGetScopeId : error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    // Create a lag
    if ((retVal = xpsLagCreateScope(scopeId, &lagId)) != XP_NO_ERR)
    {
        printf("%s:Error: couldn't create lag, rc: %d\n", __FUNCTION__, retVal);
        return retVal;
    }

    // Store the lagId in global db
    gLagIntfId[devId][portLagEntry->lagId] = lagId;

    // Add each port into the lag
    for (i = 0; i < portLagEntry->numPorts; i++)
    {
        // Add it to the Lag
        if ((retVal = xpsLagAddPortScope(scopeId, lagId,
                                         portLagEntry->ports[i])) != XP_NO_ERR)
        {
            printf("%s:Error: couldn't add portIntf: %d, to lagId: %d\n", __FUNCTION__,
                   portLagEntry->ports[i], lagId);
            return retVal;
        }
    }

    // Add the lag to the corresponding vlan
    if ((retVal = xpsVlanAddInterface(devId, portLagEntry->vlan, lagId,
                                      (xpsL2EncapType_e)portLagEntry->tagType)) != XP_NO_ERR)
    {
        printf("%s:Error: couldn't add lag id: %d, to vlan: %d, with tag type: %d\n",
               __FUNCTION__, lagId, portLagEntry->vlan, portLagEntry->tagType);
        return retVal;
    }

    // Deploy the lag
    if ((retVal = xpsLagDeploy(devId, lagId, AUTO_DIST_ENABLE)) != XP_NO_ERR)
    {
        printf("%s:Error: couldn't deploy lag on device: %d, lagId: %d\n", __FUNCTION__,
               devId, lagId);
        return retVal;
    }



    printf("%s:Info: Lag entries programmed successfully\n", __FUNCTION__);
    return XP_NO_ERR;
}

XP_STATUS xpsAppGetFdbEncap(xpDevice_t devId, MacEntry *xpsFdbData,
                            encapEntry *encapEntryData)
{

    uint32_t numOfVlanEntry = 0, entryNum = 0, numOfPortEntry = 0, portEntryNum = 0,
             numOfLagEntry = 0;// numOfencapEntry = 0, encapEntryNum = 0;
    bool isFound = 0;
    VlanEntry *vlanEntry;
    PortLagEntry *portLagEntry;

    //For non-tunnel entries
    if (xpsFdbData->tunnelIdx == -1)
    {
        //Loop through the vlan table and match port to get encapType
        numOfVlanEntry = numOfEntry(&configFile.vlanData);
        for (entryNum = 0; entryNum < numOfVlanEntry; entryNum++)
        {
            vlanEntry = (VlanEntry *)getEntry(&configFile.vlanData, entryNum);
            if (vlanEntry->vlan == xpsFdbData->vlan)
            {
                numOfPortEntry = vlanEntry->numPorts;
                for (portEntryNum = 0; portEntryNum < numOfPortEntry; portEntryNum++)
                {
                    if (xpsFdbData->port == vlanEntry->portData[portEntryNum].port)
                    {
                        encapEntryData->encapType = vlanEntry->portData[portEntryNum].tagType;
                        encapEntryData->encapId = vlanEntry->vlan;
                        return XP_NO_ERR;
                    }
                }
            }
        }

        //Loop through the lag table and find out encaptype
        numOfLagEntry = numOfEntry(&configFile.portLagData);
        for (entryNum = 0; entryNum < numOfLagEntry; entryNum++)
        {
            printf("in for loop\n");
            portLagEntry = (PortLagEntry *)getEntry(&configFile.portLagData, entryNum);
            if (portLagEntry->vlan == xpsFdbData->vlan)
            {
                printf("vlan matched\n");
                numOfPortEntry = portLagEntry->numPorts;
                for (portEntryNum = 0; portEntryNum < numOfPortEntry; portEntryNum++)
                {
                    printf("in port for loop\n");
                    if ((uint32_t)xpsFdbData->port == portLagEntry->ports[portEntryNum])
                    {
                        printf("port matched\n");
                        encapEntryData->encapType = portLagEntry->tagType;
                        encapEntryData->encapId = portLagEntry->vlan;
                        return XP_NO_ERR;
                    }
                }
            }
        }
    }

    /*tunnelIdx is not -1; its a tunnel.  0 is VXLAN and 1 is NVGRE
        tunnelId is VNI/TNI and tunnel type is XP_L2_ENCAP_VXLAN or XP_L2_ENCAP_NVGRE */
    else
    {
        switch (tnlType[devId][xpsFdbData->tunnelIdx])
        {
            case 0: //vxlan
                encapEntryData->encapType = XP_L2_ENCAP_VXLAN;
                break;

            case 1: //nvgre
                encapEntryData->encapType = XP_L2_ENCAP_NVGRE;
                break;

            case 4: //geneve
                encapEntryData->encapType = XP_L2_ENCAP_GENEVE;
                break;

            default:
                printf("%s:Invalid tunnel type in config data", __FUNCTION__);
                //return XP_ERR_INVALID_ARG;
        }
        encapEntryData->encapId = tnlVirtualId[devId][xpsFdbData->tunnelIdx];
        isFound = 1;
        if (xpsFdbData->encapType == 8)
        {
            encapEntryData->encapType = XP_L2_ENCAP_PBB;
        }
    }

    //There are some invalid entries in the config.txt; which are not present in vlan table. For those entries the below code is present.
    if (!isFound)
    {
        encapEntryData->encapType = 0;
        encapEntryData->encapId = 0;
    }

    return XP_NO_ERR;

}


uint8_t xpsAppAddFdb(xpDevice_t devId, MacEntry *xpsFdbData,
                     uint32_t numOfMvifEntry)
{
    XP_STATUS retVal;
    xpsFdbEntry_t fdbEntry, getFdbEntry;
    uint32_t gTnlVif = 0;
    uint32_t index = 0;
    xpsHashIndexList_t indexList;
    encapEntry encapEntryData;

    memset(&fdbEntry, 0, sizeof(fdbEntry));

    fdbEntry.macAddr[0] = xpsFdbData->macAddr[0];
    fdbEntry.macAddr[1] = xpsFdbData->macAddr[1];
    fdbEntry.macAddr[2] = xpsFdbData->macAddr[2];
    fdbEntry.macAddr[3] = xpsFdbData->macAddr[3];
    fdbEntry.macAddr[4] = xpsFdbData->macAddr[4];
    fdbEntry.macAddr[5] = xpsFdbData->macAddr[5];

    fdbEntry.vlanId = xpsFdbData->vlan;
    fdbEntry.pktCmd = (xpPktCmd_e)xpsFdbData->pktCmd;
    fdbEntry.isControl = xpsFdbData->controlMac;
    fdbEntry.isRouter = xpsFdbData->routerMac;
    fdbEntry.isStatic = 0;
    fdbEntry.serviceInstId = 0;

    xpsScope_t scopeId;
    retVal = xpsScopeGetScopeId(devId, &scopeId);

    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in xpsScopeGetScopeId : error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    if ((retVal = xpsAppGetFdbEncap(devId, xpsFdbData,
                                    &encapEntryData)) != XP_NO_ERR)
    {
        printf("%s:Error in getting EncapNode: error code: %d\n", __FUNCTION__, retVal);
        return retVal;
    }

    printf("Mac: %x:%x:%x:%x:%x:%x \n", fdbEntry.macAddr[0], fdbEntry.macAddr[1],
           fdbEntry.macAddr[2], fdbEntry.macAddr[3], fdbEntry.macAddr[4],
           fdbEntry.macAddr[5]);

    if (xpsFdbData->tunnelIdx != -1)
    {
        switch (encapEntryData.encapType)
        {
            case 6: //XP_L2_ENCAP_VXLAN:
                fdbEntry.intfId = tnlVif[devId][xpsFdbData->tunnelIdx];
                fdbEntry.serviceInstId = tnlVirtualId[devId][xpsFdbData->tunnelIdx];
                break;
            case 7: //XP_L2_ENCAP_NVGRE:
                fdbEntry.intfId = tnlVif[devId][xpsFdbData->tunnelIdx];
                fdbEntry.serviceInstId = tnlVirtualId[devId][xpsFdbData->tunnelIdx];
                break;
            case 8: //XP_L2_ENCAP_PBB:
                printf("FDB: PBB\n");
                fdbEntry.intfId = pbbTnlVif[devId][xpsFdbData->tunnelIdx];
                fdbEntry.serviceInstId = pbbTnlIsid[devId][xpsFdbData->tunnelIdx ];
                break;
            case 9: //XP_L2_ENCAP_GENEVE:
                gTnlVif= xpsFdbData->tunnelIdx + xpsFdbData->geneveTunnelType;
                printf("FDB: Geneve Tunnel Vif : %d | eVif: %d \n", gTnlVif,
                       tnlVif[devId][gTnlVif]);
                fdbEntry.intfId = tnlVif[devId][gTnlVif];
                fdbEntry.serviceInstId = tnlVirtualId[devId][xpsFdbData->tunnelIdx +
                                                             xpsFdbData->geneveTunnelType];
                break;
            default:
                printf("Error: invalid tunnel encaptype: %d\n", encapEntryData.encapType);
                break;
        }
    }
    else if (xpsFdbData->isLag == 1)
    {
        // This port is part of a lag, need the lag intf
        fdbEntry.intfId = gLagIntfId[devId][xpsFdbData->lagId];
        fdbEntry.serviceInstId = encapEntryData.encapId;
    }
    else if (xpsFdbData->isMcast == 1)
    {
        uint32_t i, entryNum;
        xpsMcL2InterfaceListId_t ifListId;

        if ((retVal = xpsMulticastCreateL2InterfaceListScope(scopeId, xpsFdbData->vlan,
                                                             &ifListId)) != XP_NO_ERR)
        {
            printf("%s: Error in Creating Interface List for vlan: %d: error code: %d\n",
                   __FUNCTION__, xpsFdbData->vlan, retVal);
            return retVal;
        }
        if ((retVal = xpsMulticastAddL2InterfaceListToDevice(devId,
                                                             ifListId)) != XP_NO_ERR)
        {
            printf("%s: Error in Adding Interface List %d to device %d: error code: %d\n",
                   __FUNCTION__, devId, ifListId, retVal);
            return retVal;
        }

        /* Populate mvif entries */
        for (i = 0; i < 10; i++)
        {
            if (xpsFdbData->mvifIdx[i] == 0)
            {
                break;
            }

            for (entryNum = 0; entryNum < numOfMvifEntry; entryNum++)
            {
                if (((MvifEntry *)getEntry(&configFile.mvifData,
                                           entryNum))->index == xpsFdbData->mvifIdx[i])
                {
                    xpsAppConfigureMvifEntry(devId, (MvifEntry *)getEntry(&configFile.mvifData,
                                                                          entryNum), 0, ifListId, 1);
                    break;
                }
            }
        }

        // Store the multicast Vif in global db
        multicastVif[devId][XP_MULTICAST_TYPE_FDB][xpsFdbData->mcastIdx] = ifListId;

        fdbEntry.intfId = ifListId;
        fdbEntry.serviceInstId = encapEntryData.encapId;
    }
    else
    {
        xpsInterfaceId_t intfId;
        /* If port is connected to port extender */
        if (xpsFdbData->extendedPortIdx != -1)
        {
            /* Search for extended group context for perticular port*/
            void *r = xpsAppGetExtPortsGroup(devId, xpsFdbData->port);
            if (r == NULL)
            {
                printf("%s:Error: Can't found extPortGroup ctx in tree for port %d\n",
                       __FUNCTION__, xpsFdbData->port);
                return XP_ERR_INVALID_ARG;
            }

            /* Check extended port count for group*/
            if (xpsFdbData->extendedPortIdx >= (*(extPortGroup **)r)->numOfExtendedPorts)
            {
                printf("%s:Error: invalid extendedPortIdx : %d for port %d\n", __FUNCTION__,
                       xpsFdbData->extendedPortIdx, xpsFdbData->port);
                return XP_ERR_INVALID_ARG;
            }
            intfId = (*(extPortGroup **)r)->intfId[xpsFdbData->extendedPortIdx];
        }
        /* Port is normal port */
        else
        {
            if ((retVal = xpsPortGetPortIntfId(devId, xpsFdbData->port,
                                               &intfId)) != XP_NO_ERR)
            {
                printf("%s:Error: failed to get the interface ID for device :%d, port : %d\n",
                       __FUNCTION__, devId, xpsFdbData->port);
                return retVal;
            }
        }
        fdbEntry.intfId = intfId;
        fdbEntry.serviceInstId = encapEntryData.encapId;
    }

    if ((retVal = xpsFdbAddEntry(devId, &fdbEntry, &indexList)) != XP_NO_ERR)
    {
        printf("%s:Error in inserting an entry for vlan %d: error code: %d\n",
               __FUNCTION__, fdbEntry.vlanId, retVal);
        return retVal;
    }

    index = indexList.index[0];

    printf("xpsAppAddFdb: Pass index:%d \n", index);

    if (xpsFdbData->agingFlag)
    {
        //Enable aging for table
        xpsFdbConfigureEntryAging(devId, 1, index);
        //Trigger two times to make aging-time out
        xpsFdbTriggerAging(devId);
        xpsFdbTriggerAging(devId);
        xpsFdbTriggerAging(devId);

        //Process FIFO to delete the aged out entry with default handler
        xpsAgeFifoHandler(devId);
    }
    else
    {
        //Disable aging for table
        xpsFdbConfigureEntryAging(devId, 0, index);
    }

    printf("Perform lookup for the same entry\n");
    memset(&getFdbEntry, 0, sizeof(getFdbEntry));

    //Perform the lookup on added entry just before
    getFdbEntry.macAddr[0] = xpsFdbData->macAddr[0];
    getFdbEntry.macAddr[1] = xpsFdbData->macAddr[1];
    getFdbEntry.macAddr[2] = xpsFdbData->macAddr[2];
    getFdbEntry.macAddr[3] = xpsFdbData->macAddr[3];
    getFdbEntry.macAddr[4] = xpsFdbData->macAddr[4];
    getFdbEntry.macAddr[5] = xpsFdbData->macAddr[5];

    getFdbEntry.vlanId = xpsFdbData->vlan;

    xpsPrivateVlanType_e vlanType;

    xpsPVlanGetType(devId, getFdbEntry.vlanId, &vlanType);

    if (vlanType != VLAN_ISOLATED)
    {
        if ((retVal = xpsFdbGetEntry(devId, &getFdbEntry)) != XP_NO_ERR)
        {
            printf("Error in Fdb table lookup: error code: %d\n", retVal);
            return retVal;
        }
    }
    printf("xpsFdbEntry:: macAddr: %x:%x:%x:%x:%x:%x, vlan: %d, pktCmd: %d, instId: %d\n",
           getFdbEntry.macAddr[0], getFdbEntry.macAddr[1], getFdbEntry.macAddr[2],
           getFdbEntry.macAddr[3], getFdbEntry.macAddr[4], getFdbEntry.macAddr[5],
           getFdbEntry.vlanId, getFdbEntry.pktCmd, getFdbEntry.intfId);

    return XP_NO_ERR;
}

uint8_t xpsAppAddFdbScope(xpDevice_t devId, MacScopeEntry *xpsFdbScopeData,
                          uint32_t numOfMvifEntry)
{
    XP_STATUS retVal = XP_NO_ERR;
    MacEntry xpsFdbData;
    xpsScope_t scopeId;
    uint8_t index;

    xpsFdbData.macAddr[0] = xpsFdbScopeData->macAddr[0];
    xpsFdbData.macAddr[1] = xpsFdbScopeData->macAddr[1];
    xpsFdbData.macAddr[2] = xpsFdbScopeData->macAddr[2];
    xpsFdbData.macAddr[3] = xpsFdbScopeData->macAddr[3];
    xpsFdbData.macAddr[4] = xpsFdbScopeData->macAddr[4];
    xpsFdbData.macAddr[5] = xpsFdbScopeData->macAddr[5];
    xpsFdbData.vlan = xpsFdbScopeData->vlan;
    xpsFdbData.port = xpsFdbScopeData->port;
    xpsFdbData.encapType = xpsFdbScopeData->encapType;
    xpsFdbData.controlMac = xpsFdbScopeData->controlMac;
    xpsFdbData.routerMac = xpsFdbScopeData->routerMac;
    xpsFdbData.pktCmd = xpsFdbScopeData->pktCmd;
    xpsFdbData.tunnelIdx = xpsFdbScopeData->tunnelIdx;
    xpsFdbData.geneveTunnelType = xpsFdbScopeData->geneveTunnelType;
    xpsFdbData.isLag = xpsFdbScopeData->isLag;
    xpsFdbData.lagId = xpsFdbScopeData->lagId;
    xpsFdbData.extendedPortIdx = xpsFdbScopeData->extendedPortIdx;
    xpsFdbData.isMcast = xpsFdbScopeData->isMcast;
    for (index = 0; index < 10; index++)
    {
        xpsFdbData.mvifIdx[index] = xpsFdbScopeData->mvifIdx[index];
    }
    xpsFdbData.vifMcastIdx = xpsFdbScopeData->vifMcastIdx;
    xpsFdbData.agingFlag = xpsFdbScopeData->agingFlag;

    retVal = xpsScopeGetScopeId(devId, &scopeId);

    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in xpsScopeGetScopeId : error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    if (scopeId == xpsFdbScopeData->scopeId)
    {
        retVal = (XP_STATUS)xpsAppAddFdb(devId, &xpsFdbData, numOfMvifEntry);
    }
    return retVal;
}

uint8_t xpsAppAddFdbControlMac(xpDevice_t devId,
                               FdbControlMacEntry *xpsFdbControlMacData)
{
    XP_STATUS retVal;
    uint32_t index = 0;
    xpsHashIndexList_t indexList;
    uint32_t reasonCode;

    if ((retVal = xpsFdbAddControlMacEntry(devId, xpsFdbControlMacData->vlanId,
                                           xpsFdbControlMacData->macAddr, xpsFdbControlMacData->reasonCode,
                                           &indexList)) != XP_NO_ERR)
    {
        printf("%s:Error in inserting an entry for vlan %d: error code: %d\n",
               __FUNCTION__, xpsFdbControlMacData->vlanId, retVal);
        return retVal;
    }

    index = indexList.index[0];

    printf("xpsFdbAddControlMacEntry: Pass index:%d \n", index);

    printf("Perform lookup for the same entry\n");

    if ((retVal = xpsFdbGetControlMacEntryReasonCode(devId,
                                                     xpsFdbControlMacData->vlanId, xpsFdbControlMacData->macAddr,
                                                     &reasonCode)) != XP_NO_ERR)
    {
        printf("Error in Fdb Control Mac table lookup: error code: %d\n", retVal);
        return retVal;
    }

    printf("xpsFdbAddControlMacEntry:: macAddr: %x:%x:%x:%x:%x:%x, vlan: %d, reasoncode: %d\n",
           xpsFdbControlMacData->macAddr[0], xpsFdbControlMacData->macAddr[1],
           xpsFdbControlMacData->macAddr[2], xpsFdbControlMacData->macAddr[3],
           xpsFdbControlMacData->macAddr[4], xpsFdbControlMacData->macAddr[5],
           xpsFdbControlMacData->vlanId, reasonCode);

    return XP_NO_ERR;
}

XP_STATUS xpsAppConfigureTenant(xpDevice_t devId,
                                VlanTenantIdEntry * tenantEntry)
{

    xpsServiceIdData_t data;
    XP_STATUS ret;
    memset(&data, 0, sizeof(xpsServiceIdData_t));
    data.vlanId = tenantEntry->vlanId; //Vlan Id used after tunnel termination

    if ((ret = xpsVlanServiceInstanceCreate(devId, tenantEntry->tenantId,
                                            (xpsServiceInstanceType_e)tenantEntry->tunnelType, &data)) != XP_NO_ERR)
    {
        printf("ERROR: xpsVlanServiceInstanceCreate returns %d\n", ret);
        return ret;
    }

    return XP_NO_ERR;
}

// xpSaiApp.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiAppL2.h"
#include "assert.h"

#ifndef TBD  /*This is exposing SAI internal logic to XPS APP.Should be removed after redesign XPS APP to USE saiports.h */
#include "xpSai.h"
#endif  /*TBD*/
#include "xpSaiAppSaiUtils.h"
#include "xpSaiFdb.h"
#include "xpSaiBridge.h"


XP_SAI_LOG_REGISTER_API(SAI_API_UNSPECIFIED);

#define XPSAIAPP_MAX_LEARNED_ADDRESSES (16*1024)
#define XPSAIAPP_MAX_PORT_NUM 128
// Macro to omit warning "Unused variable"
#define UNUSED(x) if (0) (void) (x)
xpsInterfaceId_t gLagIntfId[MAX_NO_LAGS];

extern int numOfEntry(entryList *list);
sai_object_id_t bridgeDot1Q = SAI_NULL_OBJECT_ID;
extern uint32_t vrfIdxToVrf[MAX_VRF_IDX];

extern uint32_t tnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t vlanToL3Intf[4096];
extern uint32_t l3TnlIntf[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlType[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlVirtualId[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlIsid[XP_MAX_DEVICES][NUM_OF_TUNNELS];

extern uint64_t vlanToRif[XP_MAX_DOT1Q_VLAN];

//global definition for extendedPort database's "extPortsRoot"
static void *extPortsRoot = NULL;

//compare function for extPorts database
static inline int compareExtPortsMap(const void *l, const void *r)
{
    if (((const extPortGroup *)l)->key > ((const extPortGroup *)r)->key)
    {
        return 1;
    }
    else if (((const extPortGroup *)l)->key < ((const extPortGroup *)r)->key)
    {
        return -1;
    }
    return 0;
}

//get extPorts database
void* xpSaiAppGetExtPortsGroup(uint32_t port)
{
    extPortGroup tmpCtx;

    tmpCtx.key = port;
    void *r = tfind(&tmpCtx, &extPortsRoot, compareExtPortsMap);
    if (r)
    {
        //LOG HERE.
    }
    return r;
}


XP_STATUS xpSaiAppSetVlanParams(xpDevice_t devId, sai_object_id_t switchOid,
                                xpsVlan_t vlanId, xpSaiAppVlanParams * vlanParams)
{
    xpsPktCmd_e             pktCmd;
    int                     macSALSBs;
    XP_STATUS               retVal      = XP_NO_ERR;
    xpVlanBridgeMcMode_e    bridgeMode;
    xpVlanRouteMcMode_t     routeMode;
    uint8_t                 mplsRouteEn, natMode, natScope;
    sai_status_t            saiRetVal   = SAI_STATUS_SUCCESS;
    sai_attribute_t         sai_attr[5];
    sai_attribute_t         sw_attr[1];
    sai_object_id_t         rifOid      = 0;
    sai_object_id_t         vlanOid     = 0;
    xpsInterfaceId_t        l3intfId    = 0;

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, devId, vlanId, &vlanOid);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Error : SAI VLAN object can not be created.\n");
        return saiStatus2XpsStatus(saiRetVal);
    }
    sai_attr[0].id = SAI_ROUTER_INTERFACE_ATTR_TYPE;
    sai_attr[0].value.s32 = SAI_ROUTER_INTERFACE_TYPE_VLAN;

    sai_attr[1].id = SAI_ROUTER_INTERFACE_ATTR_VLAN_ID;
    sai_attr[1].value.oid = vlanOid;

    sai_attr[2].id = SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID;
    sai_attr[2].value.oid = xpSaiAppSaiGetiVrfOid(vrfIdxToVrf[vlanParams->vrfId]);

    sai_attr[3].id = SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE;
    sai_attr[3].value.u32 = vlanParams->ipv4RouteEn;

    sai_attr[4].id = SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE;
    sai_attr[4].value.u32 = vlanParams->ipv6RouteEn;

    saiRetVal = xpSaiRouterInterfaceApi->create_router_interface(&rifOid, switchOid,
                                                                 sizeof(sai_attr)/sizeof(sai_attribute_t), sai_attr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI create_router_interface() Failed Error : %d for devId %d\n",
               saiRetVal, devId);
        return saiStatus2XpsStatus(saiRetVal);
    }

    vlanToRif[vlanId] = rifOid;

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(rifOid, &l3intfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertRouterInterfaceOid() failed with error code: %d\n",
                       saiRetVal);
        return saiStatus2XpsStatus(saiRetVal);
    }

    vlanToL3Intf[vlanId] = l3intfId;

    sw_attr[0].id = SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID;

    saiRetVal = xpSaiSwitchApi->get_switch_attribute(switchOid,
                                                     sizeof(sw_attr)/sizeof(sai_attribute_t), sw_attr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : Failed to get default .1Q bridge OID. Failed Error : %d for devId %d\n",
               saiRetVal, devId);
        return saiStatus2XpsStatus(saiRetVal);
    }

    bridgeDot1Q = sw_attr[0].value.oid;

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
    printf("xpSaiApp: mplsRouteEn = %d on vlan: %d\n", mplsRouteEn, vlanId);

    // program nat enable

    //program nat mode
    natMode = (uint32_t)(vlanParams->natMode);
    natScope = (uint32_t)(vlanParams->natScope);

    {
        printf("xpsL3SetIntfNatMode = natMode = %d\n", natMode);
        if ((retVal = xpsL3SetIntfNatMode(devId, l3intfId,
                                          (xpsVlanNatMode_e)natMode)) != XP_NO_ERR)
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

    //program ipv4BridgeMcMode
    bridgeMode = (xpsVlanBridgeMcMode_e)(vlanParams->ipv4BridgeMcMode);
    printf("xpSaiApp: ipv4BridgeMcMode = %d\n", bridgeMode);
    if ((retVal = xpsVlanSetIpv4McBridgeMode(devId, vlanId,
                                             bridgeMode)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsVlanSetIpv4McBridgeMode() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    //program ipv6BridgeMcMode
    bridgeMode = (xpsVlanBridgeMcMode_e)(vlanParams->ipv6BridgeMcMode);
    printf("xpSaiApp: ipv6BridgeMcMode = %d\n", bridgeMode);
    if ((retVal = xpsVlanSetIpv6McBridgeMode(devId, vlanId,
                                             bridgeMode)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsVlanSetIpv6McBridgeMode() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    //program ipv4RouteMcEn
    pktCmd = (xpsPktCmd_e)(vlanParams->ipv4RouteMcEn);
    printf("xpSaiApp: ipv4RouteMcEn = %d\n", pktCmd);
    if ((retVal = xpsL3SetIntfIpv4McRoutingEn(devId, l3intfId,
                                              pktCmd)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv4McRoutingEn() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    //program ipv6RouteMcEn
    pktCmd = (xpsPktCmd_e)(vlanParams->ipv6RouteMcEn);
    printf("xpSaiApp: ipv6RouteMcEn = %d\n", pktCmd);

    // For Now Make Ipv4 Routing Enable. Later this will come from config.txt
    if ((retVal = xpsL3SetIntfIpv6McRoutingEn(devId, l3intfId,
                                              pktCmd)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv6McRoutingEn() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    //program ipv4RouteMcMode
    routeMode = (xpVlanRouteMcMode_t)(vlanParams->ipv4RouteMcMode);
    printf("xpSaiApp: ipv4RouteMcMode = %d\n", routeMode);
    if ((retVal = xpsL3SetIntfIpv4McRouteMode(devId, l3intfId,
                                              routeMode)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv4McRouteMode() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }

    //program ipv6RouteMcMode
    routeMode = (xpVlanRouteMcMode_t)(vlanParams->ipv6RouteMcMode);
    printf("xpSaiApp: ipv6RouteMcMode = %d\n", routeMode);
    if ((retVal = xpsL3SetIntfIpv6McRouteMode(devId, l3intfId,
                                              routeMode)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsVlanSetIpv6McRouteMode() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }
    //program Unregister Mc Cmd
    pktCmd = (xpsPktCmd_e)(vlanParams->unregMcCmd);
    printf("xpSaiAppSetVlanParams : vlanId = %d unregMcCmd = %d\n", vlanId, pktCmd);
    if ((retVal = xpsVlanSetUnregMcCmd(devId, vlanId, pktCmd)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsVlanSetUnregMcCmd() for vlan %d failed with error code: %d\n",
               __FUNCTION__, vlanId, retVal);
        return retVal;
    }
    pktCmd = (xpsPktCmd_e)(vlanParams->enL3Inf);
    macSALSBs = vlanParams->macSALSBs ;
    printf("xpSaiAppSetVlanParams : vlanId = %d enL3Inf = %d macSALSBs = %x \n",
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
    return XP_NO_ERR;

}

XP_STATUS xpSaiAppSetPortVlanParams(xpDevice_t devId, xpVlan_t vlanId,
                                    xpsInterfaceId_t intfId, xpSaiAppPortVlanParams * portVlan)
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


XP_STATUS xpSaiAppConfigureVlan(xpDevice_t devId, sai_object_id_t switchOid,
                                VlanEntry *vlanEntry)
{
    XP_STATUS           retVal          = XP_NO_ERR;
    xpVlan_t            vlanId          = vlanEntry->vlan;
    uint32_t            numOfPortEntry  = 0, portEntryNum = 0, numOfLogicalPorts,
                        logicalPort, createExtendedPorts =0;
    xpPort_t            devPort;
    xpsInterfaceId_t    intfId;
    xpsPeg_t            groupId;
    sai_object_id_t     stpOid, saiPortOid;
    sai_attribute_t     vlanAttrList;
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    xpsStp_t            xpsStp          = 0;
    sai_object_id_t     *vlanMemberOidList;/*SAIAPP_MAX_PORT_COUNT*/
    sai_object_id_t     vlanOid;
    sai_object_id_t     brPortOid;
    sai_object_id_t     stpPortOid;
    sai_attribute_t     vlan_attr[3];
    sai_attribute_t     vlan_stp_attr[1];
    uint8_t             isVlanExist     = 0;
    uint16_t            portIdx         = 0;
    uint32_t maxPortNum = 0;
    sai_vlan_tagging_mode_t vlanMemberTaggingMode;
    xpsVlanConfig_t *vlanConfig = NULL;
    xpSaiAppPortVlanParams *portVlan = NULL;
    vlanMemberOidList = (sai_object_id_t *)malloc(MAX_VLAN_MEMBER_OID * sizeof(
                                                      sai_object_id_t));
    if (vlanMemberOidList == NULL)
    {
        printf("Error allocating memory for vlanMemberOidList\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    memset(vlanMemberOidList, 0x0, MAX_VLAN_MEMBER_OID * sizeof(sai_object_id_t));
    memset(&vlan_attr, 0, sizeof(vlan_attr));
    memset(&vlan_stp_attr, 0, sizeof(vlan_stp_attr));
    memset(&stpOid, 0, sizeof(stpOid));

    vlan_attr[0].id = SAI_VLAN_ATTR_VLAN_ID;
    vlan_attr[0].value.u16 = vlanId;
    vlan_attr[1].id = SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES;
    vlan_attr[1].value.u32 = XPSAIAPP_MAX_LEARNED_ADDRESSES;

    /*create vlan for this vlanId*/
    saiRetVal = xpSaiVlanApi->create_vlan(&vlanOid, switchOid, 2, vlan_attr);
    if ((saiRetVal != SAI_STATUS_SUCCESS) &&
        (saiRetVal != SAI_STATUS_ITEM_ALREADY_EXISTS))
    {
        printf("%s:Error:Failed to create vlan for vlanId %d | retVal : %d \n",
               __FUNCTION__, vlanId, saiRetVal);
        free(vlanMemberOidList);
        return saiStatus2XpsStatus(saiRetVal);
    }
    if (saiRetVal == SAI_STATUS_ITEM_ALREADY_EXISTS)
    {
        printf("%s:Error:Vlan %d already exists\n", __FUNCTION__, vlanId);
        if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, devId, vlanId,
                                          &vlanOid)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI vlan object could not be created\n");
            free(vlanMemberOidList);
            return saiStatus2XpsStatus(saiRetVal);
        }
        /*Use to get stp id using Vlan*/
        vlan_stp_attr[0].id = SAI_VLAN_ATTR_STP_INSTANCE;

        if ((saiRetVal = xpSaiVlanApi->get_vlan_attribute(vlanOid, 1,
                                                          vlan_stp_attr)) != SAI_STATUS_SUCCESS)
        {
            printf("%s:Error:Failed to get_stp for Vlan %d | retVal : %d \n", __FUNCTION__,
                   vlanId, saiRetVal);
            free(vlanMemberOidList);
            return saiStatus2XpsStatus(saiRetVal);
        }

        stpOid = (sai_object_id_t)vlan_stp_attr[0].value.oid;
        isVlanExist = 1;
    }
    else
    {
        printf("%s:Info: Sai allocated Vlan %d successfully in device %d\n",
               __FUNCTION__, vlanId, devId);
    }

    if (vlanEntry->parentVlan != 0)
    {
        if (vlanEntry->vlanType == 0)
        {
            retVal = xpsPVlanCreatePrimary(devId, vlanId);
            if (retVal != XP_NO_ERR)
            {
                printf("Error : xpsPVlanCreatePrimary() Failed Error : %d for devId %d\n",
                       retVal, devId);
                free(vlanMemberOidList);
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
                free(vlanMemberOidList);
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
                free(vlanMemberOidList);
                return retVal;
            }
        }
    }

    xpSaiAppVlanParams *vlanParams = (xpSaiAppVlanParams *)malloc(sizeof(
                                                                      xpSaiAppVlanParams));
    if (vlanParams == NULL)
    {
        printf("Error allocating memory for vlanParams\n");
        free(vlanMemberOidList);
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(vlanParams, 0x0, sizeof(xpSaiAppVlanParams));
    vlanParams->ipv4RouteEn =  vlanEntry->ipv4RouteEn;
    vlanParams->ipv6RouteEn =  vlanEntry->ipv6RouteEn;
    vlanParams->ipv4BridgeMcMode =  vlanEntry->ipv4BridgeMcMode;
    vlanParams->ipv6BridgeMcMode =  vlanEntry->ipv6BridgeMcMode;
    vlanParams->ipv4RouteMcEn =  vlanEntry->ipv4RouteMcEn;
    vlanParams->ipv6RouteMcEn =  vlanEntry->ipv6RouteMcEn;
    vlanParams->mplsRouteEn =  vlanEntry->mplsRouteEn;
    vlanParams->ipv4RouteMcMode =  vlanEntry->ipv4RouteMcMode;
    vlanParams->ipv6RouteMcMode =  vlanEntry->ipv6RouteMcMode;
    vlanParams->vrfId = vlanEntry->vrfId;
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

    /*Use to get stp id using Vlan*/
    memset(&vlan_stp_attr, 0, sizeof(vlan_stp_attr));
    vlan_stp_attr[0].id = SAI_VLAN_ATTR_STP_INSTANCE;

    if ((saiRetVal = xpSaiVlanApi->get_vlan_attribute(vlanOid, 1,
                                                      vlan_stp_attr)) != SAI_STATUS_SUCCESS)
    {
        printf("%s:Error:Failed to get_stp for Vlan %d | retVal : %d \n", __FUNCTION__,
               vlanId, saiRetVal);
        retVal = saiStatus2XpsStatus(saiRetVal);
        goto exit;
    }

    xpsStp = (xpsStp_t)xpSaiObjIdValueGet(vlan_stp_attr[0].value.oid);
    //Check stp existence in Scope.
    retVal = xpsStpIsExist(XP_SCOPE_DEFAULT, xpsStp);
    if ((retVal != XP_NO_ERR) && (retVal != XP_ERR_RESOURCE_NOT_AVAILABLE)
        &&(vlanEntry->vlanType != 2))
    {
        printf("Error : xpsStpIsExist() Failed for devId %d vlanId %d\n", devId,
               vlanId);
        goto exit;
    }

    //Skip stp allocation for isolated vlan or if already allocated for a vlan in another device
    if ((vlanEntry->vlanType != 2) && (!isVlanExist)
        && ((retVal == XP_ERR_RESOURCE_NOT_AVAILABLE) || (xpsStp == 0)))
    {
        //Get max port num
        retVal = xpsPortGetMaxNum(devId, &maxPortNum);
        if (retVal != XP_NO_ERR)
        {
            printf("%s:Error:Failed to get max port number retVal : %d \n", __FUNCTION__,
                   saiRetVal);
            goto exit;
        }

        //attribute_count = 1 for attribute VLAN
        if ((saiRetVal = xpSaiStpApi->create_stp(&stpOid, switchOid, 0,
                                                 NULL)) != SAI_STATUS_SUCCESS)
        {
            printf("%s:Error:Failed to create_stp for Vlan %d | retVal : %d \n",
                   __FUNCTION__, vlanId, saiRetVal);
            goto exit;
        }

        /* SAI creates STP with BLOCK state as default. Application needs to set the STP to FORWARD state for the apptest */
        XPS_GLOBAL_PORT_ITER(portIdx, maxPortNum)
        {
            if ((saiRetVal = xpSaiBridgePortObjIdCreate(devId, portIdx,
                                                        SAI_BRIDGE_PORT_TYPE_PORT, &saiPortOid)) != SAI_STATUS_SUCCESS)
            {
                printf("Error : SAI object can not be created.\n");
                goto exit;
            }

            if ((retVal = xpSaiAppStpSetState(&stpPortOid, stpOid, saiPortOid,
                                              SAI_STP_PORT_STATE_FORWARDING)) != XP_NO_ERR)
            {
                printf("%s:Error: Setting the stp state to FORWARD retVal : %d \n",
                       __FUNCTION__, retVal);
                goto exit;
            }
        }

        //VLAN attributes to bind with STP-ID
        vlanAttrList.id = SAI_VLAN_ATTR_STP_INSTANCE;
        vlanAttrList.value.oid = stpOid;

        if ((saiRetVal = xpSaiVlanApi->set_vlan_attribute(vlanOid,
                                                          &vlanAttrList)) != SAI_STATUS_SUCCESS)
        {
            printf("%s:Error:Failed to bind Vlan for given stpOid %d | retVal : %d \n",
                   __FUNCTION__, vlanId, saiRetVal);
            goto exit;
        }
    }

    memset(&vlanAttrList, 0, sizeof(sai_attribute_t));

    //VLAN attributes to set unknown SA cmd
    vlanAttrList.id = SAI_VLAN_ATTR_LEARN_DISABLE;
    vlanAttrList.value.booldata = (bool)((vlanEntry->macSAmissCmd == XP_PKTCMD_FWD)
                                         ?1:0);

    if ((saiRetVal = xpSaiVlanApi->set_vlan_attribute(vlanOid,
                                                      &vlanAttrList)) != SAI_STATUS_SUCCESS)
    {
        printf("%s:Error:Failed to set unknown SA miss command for vlanId %d | retVal : %d \n",
               __FUNCTION__, vlanId, saiRetVal);
        goto exit;
    }

    //Convert sai object to uint32_t object
    xpsStp = (uint32_t)xpSaiObjIdValueGet(stpOid);

    vlanConfig = (xpsVlanConfig_t *)malloc(sizeof(xpsVlanConfig_t));
    if (vlanConfig == NULL)
    {
        printf("Error allocating memory for vlanConfig\n");
        free(vlanParams);
        free(vlanMemberOidList);
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(vlanConfig, 0x0, sizeof(xpsVlanConfig_t));

    retVal = xpsVlanGetConfig(devId, vlanId, vlanConfig);
    if (retVal != XP_NO_ERR)
    {
        printf("Error : xpsVlanGetConfig() Failed for devId %d and vlanId %d \n", devId,
               vlanId);
        goto exit;
    }

    vlanConfig->saMissCmd = (xpsPktCmd_e)(vlanEntry->macSAmissCmd);
    vlanConfig->bcCmd = (xpsPktCmd_e)(vlanEntry->broadcastCmd);
    vlanConfig->unknownUcCmd = (xpsPktCmd_e)(vlanEntry->broadcastCmd);
    vlanConfig->arpBcCmd = (xpsPktCmd_e)(vlanEntry->ipv4ARPBCCmd);
    retVal = xpsVlanSetConfig(devId, vlanId, vlanConfig);
    if (retVal != XP_NO_ERR)
    {
        printf("Error : xpsVlanSetConfig() Failed for devId %d and vlanId %d \n", devId,
               vlanId);
        goto exit;
    }
    printf("%s:Info: Sai allocated stpId %u successfully in device %d for Vlan %d\n",
           __FUNCTION__, vlanConfig->stpId, devId, vlanId);

    retVal = xpSaiAppSetVlanParams(devId, switchOid, vlanId, vlanParams);
    if (retVal != XP_NO_ERR)
    {
        printf("Error : xpSaiAppSetVlanParams() Failed for devId %d\n", devId);
        goto exit;
    }

    portVlan = (xpSaiAppPortVlanParams *)malloc(sizeof(xpSaiAppPortVlanParams));
    if (portVlan == NULL)
    {
        printf("Error allocating memory for portVlan\n");
        free(vlanConfig);
        free(vlanParams);
        free(vlanMemberOidList);
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(portVlan, 0x0, sizeof(xpSaiAppPortVlanParams));

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
                goto exit;
            }
            numOfLogicalPorts = 1;
        }
        /* Extended port is connected to physical port*/
        else
        {
            r = xpSaiAppGetExtPortsGroup(devPort);
            if (r == NULL)
            {
                /* Create extended port group */
                memset(&groupId, 0, sizeof(groupId));
                retVal = xps8021BrCreatePortExtenderGroup(XP_8021BR_MAX_PER_GROUP, &groupId);
                if (retVal)
                {
                    printf("Error : xps8021BrCreatePortExtenderGroup() Failed for devPort :%d devId %d\n",
                           devPort, devId);
                    goto exit;
                }

                extPortGroup *tmpCtx = (extPortGroup *)malloc(sizeof(extPortGroup));
                if (!tmpCtx)
                {
                    printf("%s:Error: Can't allocate extPortGroup context for port Index: %d\n",
                           __FUNCTION__, devPort);
                    retVal = XP_ERR_INIT;
                    goto exit;
                }
                tmpCtx->groupId = groupId;
                tmpCtx->key = devPort;
                tmpCtx->intfId = (xpsInterfaceId_t *)malloc(numOfLogicalPorts * sizeof(
                                                                xpsInterfaceId_t));
                if (tmpCtx->intfId == NULL)
                {
                    printf("Error allocating memory for Interface Id for port %d\n", devPort);
                    free(vlanParams);
                    free(tmpCtx);
                    free(portVlan);
                    free(vlanMemberOidList);
                    free(vlanConfig);
                    return XP_ERR_MEM_ALLOC_ERROR;
                }
                tmpCtx->numOfExtendedPorts = numOfLogicalPorts;
                r = tsearch(tmpCtx, &extPortsRoot, compareExtPortsMap);
                if (r == NULL)
                {
                    printf("%s:Error: Can't insert extPortGroup ctx in tree for port %d\n",
                           __FUNCTION__, tmpCtx->key);
                    free(tmpCtx->intfId);
                    free(tmpCtx);
                    retVal = XP_ERR_INIT;
                    goto exit;
                }
                createExtendedPorts = 1;
                free(tmpCtx->intfId);
                free(tmpCtx);
            }

        }

        for (logicalPort = 0; logicalPort < numOfLogicalPorts; logicalPort++)
        {
            if (vlanEntry->portData[portEntryNum].numOfExtPorts > 0)
            {
                if (createExtendedPorts)
                {
                    retVal = xps8021BrCreateExtendedPort(groupId, 1, &intfId);
                    if (retVal)
                    {
                        printf("Error : xps8021BrCreateExtendedPort() Failed for group :%d devId %d\n",
                               groupId, devId);
                        goto exit;
                    }

                    retVal = xps8021BrInitExtendedPort(devId, groupId, intfId);
                    if (retVal)
                    {
                        printf("Error : xps8021BrBindExtendedPort() Failed for group :%d devId %d intfId : %d\n",
                               groupId, devId, intfId);
                        goto exit;
                    }

                    xpsInterfaceId_t cascadeIntfId;
                    retVal = xpsPortGetPortIntfId(devId, devPort, &cascadeIntfId);
                    if (retVal)
                    {
                        printf("Error : xpsPortGetPortIntfId() Failed for devId %d portId : %d\n",
                               devId, devPort);
                        goto exit;
                    }

                    retVal = xps8021BrBindExtendedPort(groupId, intfId, cascadeIntfId);
                    if (retVal)
                    {
                        printf("Error : xps8021BrBindExtendedPort() Failed for group :%d devId %d\n",
                               groupId, devId);
                        free(vlanConfig);
                        free(vlanParams);
                        free(portVlan);
                        return retVal;
                    }
                    if (r != NULL)
                    {
                        (*(extPortGroup **)r)->intfId[logicalPort] = intfId;
                    }
                }
                else
                {
                    if (r != NULL)
                    {
                        intfId = (*(extPortGroup **)r)->intfId[logicalPort];
                    }
                }
            }

            if (vlanEntry->parentVlan == 0)
            {
                retVal = xpSaiAppSaiConvertTaggingMode((xpL2EncapType_e)portVlan->tagType,
                                                       &vlanMemberTaggingMode);
                if (retVal == XP_NO_ERR)
                {
                    saiRetVal = xpSaiBridgePortObjIdCreate(devId, (sai_uint64_t) intfId,
                                                           SAI_BRIDGE_PORT_TYPE_PORT, &brPortOid);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        printf("Error : SAI create_bridge_port() Failed Error : %d for devId %d\n",
                               saiRetVal, devId);
                        retVal = saiStatus2XpsStatus(saiRetVal);
                        goto exit;
                    }

                    memset(&vlan_attr, 0, sizeof(vlan_attr));

                    vlan_attr[0].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
                    /*This is exposing SAI internal logic to XPS APP.Should be removed after redesign XPS APP to USE saiports.h */
                    if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, devId, vlanId,
                                                      &vlan_attr[0].value.oid)) != SAI_STATUS_SUCCESS)
                    {
                        printf("Error : SAI object can not be created.\n");
                        retVal = saiStatus2XpsStatus(saiRetVal);
                        goto exit;
                    }

                    vlan_attr[1].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
                    vlan_attr[1].value.oid = brPortOid;

                    vlan_attr[2].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
                    vlan_attr[2].value.u32 = vlanMemberTaggingMode;

                    // Add ports to vlan
                    saiRetVal = xpSaiVlanApi->create_vlan_member(&vlanMemberOidList[logicalPort],
                                                                 switchOid, 3, vlan_attr);
                    if (saiRetVal == SAI_STATUS_ITEM_ALREADY_EXISTS &&
                        (vlanId == XP_SAI_APP_DEFAULT_VLAN))
                    {
                        printf("Could not add ports to the vlan %u, intfId %d\n", vlanId, intfId);
                        saiRetVal = xpSaiVlanApi->remove_vlan_member(vlanMemberOidList[logicalPort]);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            printf("Error : SAI remove_vlan_member() Failed Error : %d for devId %d vlanId %d intfId %d\n",
                                   saiRetVal, devId, vlanId, intfId);
                            retVal = saiStatus2XpsStatus(saiRetVal);
                            goto exit;
                        }
                        saiRetVal = xpSaiVlanApi->create_vlan_member(&vlanMemberOidList[logicalPort],
                                                                     switchOid, 3, vlan_attr);
                    }
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        printf("Error : SAI create_vlan_member() Failed Error : %d for devId %d vlanId %d intfId %d bridgePortId: %llu\n",
                               saiRetVal, devId, vlanId, intfId, (long long unsigned int)brPortOid);
                        retVal = saiStatus2XpsStatus(saiRetVal);
                        goto exit;
                    }
                }
                else
                {
                    // TODO : As all Tagging mode right now not supported in Sai,
                    // we handle this here. Latter on changes will be incorporated in
                    // Sai call
                    retVal = xpsVlanAddInterface(devId, vlanId, intfId,
                                                 (xpsL2EncapType_e)(portVlan->tagType));
                    //To take care of tagged interfaces in vlan 100
                    if ((retVal == XP_ERR_KEY_EXISTS) && (vlanId == XP_SAI_APP_DEFAULT_VLAN))
                    {
                        retVal = xpsVlanRemoveInterface(devId, vlanId, intfId);
                        if (retVal != XP_NO_ERR)
                        {
                            printf("Error : xpsVlanRemoveInterface() Failed Error : %d for devId %d vlanId %d intfId %d\n",
                                   retVal, devId, vlanId, intfId);
                            goto exit;
                        }

                        retVal = xpsVlanAddInterface(devId, vlanId, intfId,
                                                     (xpsL2EncapType_e)(portVlan->tagType));
                    }
                    if (retVal != XP_NO_ERR)
                    {
                        printf("Error : xpsVlanAddInterface() Failed Error : %d for devId %d vlanId %d intfId %d tagType %d\n",
                               retVal, devId, vlanId, intfId, portVlan->tagType);
                        goto exit;
                    }
                }
            }
            else if (vlanEntry->parentVlan != 0) //Private Vlan Interface
            {
                if (vlanEntry->vlanType == 0)
                {
                    retVal = xpsPVlanAddInterfacePrimary(devId, vlanId, intfId,
                                                         (xpsL2EncapType_e)(portVlan->tagType));
                    if (retVal != XP_NO_ERR)
                    {
                        printf("Error : xpsPVlanAddInterfacePrimary() Failed Error : %d for devId %d\n",
                               retVal, devId);
                        goto exit;
                    }
                }
                else if (vlanEntry->vlanType == 1)
                {
                    retVal = xpsPVlanAddInterfaceCommunity(devId, vlanEntry->parentVlan, vlanId,
                                                           intfId, (xpsL2EncapType_e)(portVlan->tagType));
                    if (retVal != XP_NO_ERR)
                    {
                        printf("Error : xpsPVlanRemoveInterfaceCommunity() Failed Error : %d for devId %d\n",
                               retVal, devId);
                        goto exit;
                    }
                }
                else if (vlanEntry->vlanType == 2)
                {
                    retVal = xpsPVlanAddInterfaceIsolated(devId, vlanEntry->parentVlan, vlanId,
                                                          intfId, (xpsL2EncapType_e)(portVlan->tagType));
                    if (retVal != XP_NO_ERR)
                    {
                        printf("Error : xpsVlanAddInterface() Failed Error : %d for devId %d\n", retVal,
                               devId);
                        goto exit;
                    }
                }
            }

            xpSaiAppSetPortVlanParams(devId, vlanId, intfId, portVlan);

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
                sai_stp_port_state_t saiStpState;
                //Convert xps Stp state to SAI stp state
                //Convert intfId to sai Bridge Port Object
                if ((saiRetVal = xpSaiBridgePortObjIdCreate(devId, intfId,
                                                            SAI_BRIDGE_PORT_TYPE_PORT, &saiPortOid)) != SAI_STATUS_SUCCESS)
                {
                    printf("Error : SAI object can not be created.\n");
                    retVal = saiStatus2XpsStatus(saiRetVal);
                    goto exit;
                }

                if ((retVal = xpSaiAppSaiConvertStpState((xpVlanStgState_e)(portVlan->stpState),
                                                         &saiStpState)) != XP_NO_ERR)
                {
                    printf("%s:Error:Unknown stp port state %d for stpId %u, port %d | retVal : %d \n",
                           __FUNCTION__, saiStpState, vlanConfig->stpId, devPort, saiRetVal);
                    retVal = XP_STATUS_NUM_OF_ENTRIES;
                    goto exit;
                }

                if ((retVal = xpSaiAppStpSetState(&stpPortOid, stpOid, saiPortOid,
                                                  saiStpState)) != XP_NO_ERR)
                {
                    printf("%s:Error: Setting the stp state to FORWARD retVal : %d \n",
                           __FUNCTION__, retVal);
                    goto exit;
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
                    goto exit;
                }

                retVal = xpsPortGetPortIntfId(devId, portVlan->evif, &dIfId);
                if (retVal)
                {
                    printf("Error : xpsPortGetPortIntfId() Failed for interfac type :%d devId %d\n",
                           XPS_PORT, devId);
                    goto exit;
                }

                retVal = xpsVlanSetRedirectToInterface(devId, vlanId, sIfId, enRedirect, dIfId,
                                                       (xpVlanEncap_t)(portVlan->encapType));
                if (retVal != XP_NO_ERR)
                {
                    printf("Error : xpsVlanSetRedirectToInterface() Failed Error : %d for devId %d\n",
                           retVal, devId);
                    goto exit;
                }
            }


        }//for loop


        retVal = xpsPortGetMaxNum(devId, &maxPortNum);
        if (retVal != XP_NO_ERR)
        {
            printf("%s:Error:Failed to get max port number retVal : %d \n", __FUNCTION__,
                   saiRetVal);
            goto exit;
        }
#if 0
TODO:
        Move this to the bridge port usage when bridge port learning mode is supported
        in SAI
        XPS_GLOBAL_PORT_ITER(portIdx, maxPortNum)
        {

            if ((retVal = xpSaiAppPortSetAttr(portIdx, devId)) != XP_NO_ERR)
            {
                printf("%s:Error: Setting the Port Attribute to FORWARD retVal : %d \n",
                       __FUNCTION__, retVal);
                goto exit;
            }
        }
#endif
    }

    printf("%s:Info: Vlan entries programmed successfully\n", __FUNCTION__);

exit:
    free(vlanParams);
    free(vlanMemberOidList);
    if (vlanConfig)
    {
        free(vlanConfig);
    }
    if (portVlan)
    {
        free(portVlan);
    }

    return retVal;
}

XP_STATUS xpSaiAppConfigureLag(xpDevice_t devId, sai_object_id_t switchOid,
                               PortLagEntry *portLagEntry)
{
    XP_STATUS retVal = XP_NO_ERR;
    uint32_t i;
    sai_object_id_t lagOid, brPortOid, saiPortOId, vlanMemberLagOid, lagMemberOId;
    sai_attribute_t lagMemberAttrList[2];
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t lagIntfId;
    sai_attribute_t vlan_attr[3];
    sai_attribute_t sw_attr[1];
    sai_attribute_t brport_attr[3];
    sai_vlan_tagging_mode_t vlanMemberTaggingMode;

    memset(&vlan_attr, 0, sizeof(vlan_attr));

    if ((saiRetVal = xpSaiLagApi->create_lag(&lagOid, switchOid, 0,
                                             NULL)) != SAI_STATUS_SUCCESS)
    {
        printf("%s:Error: couldn't create lag | retVal: %d\n", __FUNCTION__, saiRetVal);
        return saiStatus2XpsStatus(saiRetVal);
    }

    for (i = 0; i < portLagEntry->numPorts; i++)
    {
        //Convert intfId to sai Port Object
        if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                          (sai_uint64_t)portLagEntry->ports[i], &saiPortOId)) != SAI_STATUS_SUCCESS)
        {
            printf("Error : SAI object can not be created.\n");
            return saiStatus2XpsStatus(saiRetVal);
        }

        lagMemberAttrList[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
        lagMemberAttrList[0].value.oid = lagOid;
        lagMemberAttrList[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
        lagMemberAttrList[1].value.oid = saiPortOId;

        // add port in Lag
        if ((saiRetVal = xpSaiLagApi->create_lag_member(&lagMemberOId, switchOid, 2,
                                                        lagMemberAttrList)) != SAI_STATUS_SUCCESS)
        {
            printf("%s:Error: couldn't create lag | retVal: %d\n", __FUNCTION__, saiRetVal);
            return saiStatus2XpsStatus(saiRetVal);
        }

    }

    //Convert Sai Object to xps Lag Id
    lagIntfId = (xpsInterfaceId_t)xpSaiObjIdValueGet(lagOid);
    // Store the lagOid in global db
    gLagIntfId[portLagEntry->lagId] = lagIntfId;

    if (SAI_NULL_OBJECT_ID == bridgeDot1Q)
    {
        sw_attr[0].id = SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID;

        saiRetVal = xpSaiSwitchApi->get_switch_attribute(switchOid,
                                                         sizeof(sw_attr)/sizeof(sai_attribute_t), sw_attr);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            printf("Error : Failed to get default .1Q bridge OID. Failed Error : %d for devId %d\n",
                   saiRetVal, devId);
            return saiStatus2XpsStatus(saiRetVal);
        }

        bridgeDot1Q = sw_attr[0].value.oid;
    }

    retVal = xpSaiAppSaiConvertTaggingMode((xpL2EncapType_e)portLagEntry->tagType,
                                           &vlanMemberTaggingMode);
    if (retVal == XP_NO_ERR)
    {
        memset(&brport_attr, 0, sizeof(brport_attr));

        brport_attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
        brport_attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_PORT;

        brport_attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
        brport_attr[1].value.oid = lagOid;

        brport_attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
        brport_attr[2].value.oid = bridgeDot1Q;

        saiRetVal = xpSaiBridgeApi->create_bridge_port(&brPortOid, switchOid, 3,
                                                       brport_attr);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            printf("Error : SAI create_bridge_port() Failed Error : %d for devId %d\n",
                   saiRetVal, devId);
            return saiStatus2XpsStatus(saiRetVal);
        }

        vlan_attr[0].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
        /*This is exposing SAI internal logic to XPS APP.Should be removed after redesign XPS APP to USE saiports.h */
        if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, devId,
                                          portLagEntry->vlan, &vlan_attr[0].value.oid)) != SAI_STATUS_SUCCESS)
        {
            printf("Error : SAI object can not be created.\n");
            return saiStatus2XpsStatus(saiRetVal);
        }

        vlan_attr[1].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
        vlan_attr[1].value.oid = brPortOid;

        vlan_attr[2].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
        vlan_attr[2].value.u32 = vlanMemberTaggingMode;

        saiRetVal = xpSaiVlanApi->create_vlan_member(&vlanMemberLagOid, switchOid, 3,
                                                     vlan_attr);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            printf("Error : SAI add_ports_to_vlan() Failed Error : %d for devId %d\n",
                   saiRetVal, devId);
            return saiStatus2XpsStatus(saiRetVal);
        }
    }

    printf("%s:Info: Lag entries programmed successfully for lagId: %u\n",
           __FUNCTION__, lagIntfId);

    return XP_NO_ERR;
}

XP_STATUS xpSaiAppGetFdbEncap(xpDevice_t devId, MacEntry *xpsFdbData,
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

    /*tunnelIdx is not -1;its a tunnel.  0 is VXLAN and 1 is NVGRE
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

    //There are some invalid entries in the config.txt;which are not present in vlan table. For those entries the below code is present.
    if (!isFound)
    {
        encapEntryData->encapType = 0;
        encapEntryData->encapId = 0;
    }

    return XP_NO_ERR;

}

XP_STATUS xpSaiAppAddFdb(xpDevice_t devId, sai_object_id_t switchOid,
                         MacEntry *xpsFdbData, uint32_t numOfMvifEntry)
{
    XP_STATUS retVal;
    xpsFdbEntry_t fdbEntry, getFdbEntry;
    uint32_t gTnlVif = 0;
    xpsHashIndexList_t indexList;
    bool saiVlanBridgeMode = false;
    encapEntry encapEntryData;

    memset(&fdbEntry, 0, sizeof(fdbEntry));

    fdbEntry.macAddr[0] = xpsFdbData->macAddr[0];
    fdbEntry.macAddr[1] = xpsFdbData->macAddr[1];
    fdbEntry.macAddr[2] = xpsFdbData->macAddr[2];
    fdbEntry.macAddr[3] = xpsFdbData->macAddr[3];
    fdbEntry.macAddr[4] = xpsFdbData->macAddr[4];
    fdbEntry.macAddr[5] = xpsFdbData->macAddr[5];

    fdbEntry.vlanId = xpsFdbData->vlan;
    fdbEntry.pktCmd = (xpPktCmd_e)(xpsFdbData->pktCmd);
    fdbEntry.isControl = xpsFdbData->controlMac;
    fdbEntry.isRouter = xpsFdbData->routerMac;
    fdbEntry.isStatic = 0;
    fdbEntry.serviceInstId = 0;

    if ((retVal = xpSaiAppGetFdbEncap(devId, xpsFdbData,
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
                // TODO: Currently PBB is not supported in xpSai.
                // We will enable below code once PBB support is available in xpSai
                return XP_NO_ERR;

            // coverity[UNREACHABLE]
            // printf("FDB: PBB\n");
            // fdbEntry.intfId = pbbTnlVif[devId][xpsFdbData->tunnelIdx];
            // fdbEntry.serviceInstId = pbbTnlIsid[devId][xpsFdbData->tunnelIdx ];
            // break;
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
        fdbEntry.intfId = gLagIntfId[xpsFdbData->lagId];
        fdbEntry.serviceInstId = encapEntryData.encapId;

        if (!fdbEntry.isControl && !fdbEntry.isRouter)
        {
            saiVlanBridgeMode = true;
        }
    }
    else if (xpsFdbData->isMcast == 1)
    {
        uint32_t i, entryNum;
        xpsMcL2InterfaceListId_t ifListId;
        memset(&ifListId, 0, sizeof(ifListId));
        if ((retVal = xpsMulticastCreateL2InterfaceList(xpsFdbData->vlan,
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
                    xpSaiAppConfigureMvifEntry(devId, (MvifEntry *)getEntry(&configFile.mvifData,
                                                                            entryNum), 0, ifListId, 1);
                    break;
                }
            }
        }
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
            void *r = xpSaiAppGetExtPortsGroup(xpsFdbData->port);
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
                printf("%s: Error: Failed to get the interface ID for device : %d, port : %d\n",
                       __FUNCTION__, devId, xpsFdbData->port);
                return retVal;
            }
        }
        fdbEntry.intfId = intfId;
        fdbEntry.serviceInstId = encapEntryData.encapId;

        if (!fdbEntry.isControl && !fdbEntry.isRouter)
        {
            saiVlanBridgeMode = true;
        }
    }

    if (!saiVlanBridgeMode)
    {
        if ((retVal = xpsFdbAddEntry(devId, &fdbEntry, &indexList)) != XP_NO_ERR)
        {
            printf("%s:Error in inserting an entry for vlan %d: error code: %d\n",
                   __FUNCTION__, fdbEntry.vlanId, retVal);
            return retVal;
        }

        printf("xpSaiAppAddFdb: Pass index:%d \n", indexList.index[0]);

        memset(&getFdbEntry, 0, sizeof(getFdbEntry));

        //Perform the lookup on added entry just before
        getFdbEntry.macAddr[0] = xpsFdbData->macAddr[0];
        getFdbEntry.macAddr[1] = xpsFdbData->macAddr[1];
        getFdbEntry.macAddr[2] = xpsFdbData->macAddr[2];
        getFdbEntry.macAddr[3] = xpsFdbData->macAddr[3];
        getFdbEntry.macAddr[4] = xpsFdbData->macAddr[4];
        getFdbEntry.macAddr[5] = xpsFdbData->macAddr[5];

        getFdbEntry.vlanId = xpsFdbData->vlan;

        xpsPrivateVlanType_e vlanType =  VLAN_NONE;
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
               getFdbEntry.macAddr[0],
               getFdbEntry.macAddr[1],
               getFdbEntry.macAddr[2],
               getFdbEntry.macAddr[3],
               getFdbEntry.macAddr[4],
               getFdbEntry.macAddr[5],
               getFdbEntry.vlanId,
               getFdbEntry.pktCmd,
               getFdbEntry.intfId);
    }

    if (saiVlanBridgeMode)
    {
        sai_status_t saiRetVal;
        sai_fdb_entry_t fdb_entry;
        xpsPrivateVlanType_e vlanType = VLAN_NONE;

        memset(&fdb_entry, 0, sizeof(fdb_entry));
        fdb_entry.switch_id = switchOid;

        /*This is exposing SAI internal logic to XPS APP.Should be removed after redesign XPS APP to USE saibridge.h */
        if ((saiRetVal = xpSaiBridgeVlanObjIdCreate(devId, fdbEntry.vlanId,
                                                    &fdb_entry.bv_id)) != SAI_STATUS_SUCCESS)
        {
            printf("Error : SAI object can not be created.\n");
            return saiStatus2XpsStatus(saiRetVal);
        }

        memcpy(fdb_entry.mac_address, fdbEntry.macAddr, sizeof(fdb_entry.mac_address));
        xpSaiAppMacReverse(fdb_entry.mac_address);

        /* Note: This is specifically added for aging
         * We are keeping all the fdb entries belong to PVLAN as static so age out won't happen */
        xpsPVlanGetType(devId, fdbEntry.vlanId, &vlanType);
        if (vlanType != VLAN_NONE)
        {
            fdbEntry.isStatic = 1;
            printf("Static for Vlan-Id %d\n", fdbEntry.vlanId);
        }

        sai_attribute_t attr_list[3];

        attr_list[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
        attr_list[0].value.s32 = fdbEntry.isStatic ? SAI_FDB_ENTRY_TYPE_STATIC :
                                 SAI_FDB_ENTRY_TYPE_DYNAMIC;

        attr_list[1].id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID;

        /*This is exposing SAI internal logic to XPS APP.Should be removed after redesign XPS APP to USE saiports.h */
        if ((saiRetVal = xpSaiBridgePortObjIdCreate(devId,
                                                    (sai_uint64_t) fdbEntry.intfId, SAI_BRIDGE_PORT_TYPE_PORT,
                                                    &attr_list[1].value.oid)) != SAI_STATUS_SUCCESS)
        {
            printf("Error : SAI object can not be created.\n");
            return saiStatus2XpsStatus(saiRetVal);
        }

        attr_list[2].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION;

        saiRetVal = xpSaiConvertXpsPacketAction2Sai(fdbEntry.pktCmd,
                                                    (sai_packet_action_t*)&attr_list[2].value.s32);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            printf("Error : xpSaiConvertXpsPacketAction2Sai() failed : retVal %d | devId %d \n",
                   saiRetVal, devId);
            return saiStatus2XpsStatus(saiRetVal);
        }

        saiRetVal = xpSaiFdbApi->create_fdb_entry(&fdb_entry,
                                                  sizeof(attr_list)/sizeof(sai_attribute_t), attr_list);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            printf("Error : SAI create_fdb_entry() failed : Failed Error : %d for devId %d intfId %d bridgePortId: %llu\n",
                   saiRetVal, devId, fdbEntry.intfId,
                   (long long unsigned int)attr_list[1].value.oid);
            return saiStatus2XpsStatus(saiRetVal);
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiAppConfigureTenant(xpDevice_t devId, sai_object_id_t switchOid,
                                  VlanTenantIdEntry * tenantEntry)
{

    xpsServiceIdData_t data;
    XP_STATUS ret = XP_NO_ERR;
    memset(&data, 0, sizeof(xpsServiceIdData_t));
    data.vlanId = tenantEntry->vlanId;//Vlan Id used after tunnel termination

    if ((ret = xpsVlanServiceInstanceCreate(devId, tenantEntry->tenantId,
                                            (xpsServiceInstanceType_e)(tenantEntry->tunnelType), &data)) != XP_NO_ERR)
    {
        printf("ERROR: xpsVlanServiceInstanceCreate returns %d\n", ret);
        return ret;
    }

    return ret;
}

void xpSaiAppL2CleanUp(xpsDevice_t devId)
{
    UNUSED(devId);

    printf("Calling %s\n", __FUNCTION__);
    extPortGroup *item;
    while (extPortsRoot != NULL)
    {
        item = *(extPortGroup **) extPortsRoot;
        tdelete(item, &extPortsRoot, compareExtPortsMap);
        free(item->intfId);
        free(item);
    }
}

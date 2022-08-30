// xpsAppTnl.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

//#include "xpsApp.h"
#include "xpsAppTnl.h"
#include "xpsScope.h"

extern uint32_t vlanToL3Intf[4096];
extern uint32_t tnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t l3TnlIntf[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlType[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlVirtualId[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlIsid[XP_MAX_DEVICES][NUM_OF_TUNNELS];

uint8_t xpsAppConfigureIpTnnlData(xpDevice_t devId, TunnelEntry *tunnelEntry)
{
    xpsAppIpTunnelData tunnelData;
    xpsIpTunnelData_t ipData;
    int ret;
    uint32_t tnlIdx, serviceId = 0;
    uint32_t nhIdx, index;
    xpsInterfaceId_t tnlIntfId = 0, portIntfId;
    xpsInterfaceId_t lstTnlId[MAX_GENEVE_SUPPORTED_OPTIONS];
    xpsVxlanTunnelConfig_t vxlanTcfg;
    xpsNvgreTunnelConfig_t nvgreTcfg;
    xpsGeneveTunnelConfig_t geneveTcfg;
    xpsIpinIpTunnelConfig_t ipInIpTcfg;
    xpsIpGreTunnelConfig_t greTcfg;
    xpsVpnGreTunnelConfig_t vpnGreTcfg;
    ipv4Addr_t lclEpIpAddr;
    ipv4Addr_t rmtEpIpAddr;
    xpsGeneveFormatType_t optionFormat;
    inetAddr_t lclEpInetAddr;
    inetAddr_t rmtEpInetAddr;

    memset(&tunnelData, 0x0, sizeof(xpsAppIpTunnelData));
    memset(&vxlanTcfg, 0x0, sizeof(xpsVxlanTunnelConfig_t));
    memset(&nvgreTcfg, 0x0, sizeof(xpsNvgreTunnelConfig_t));
    memset(&geneveTcfg, 0x0, sizeof(xpsGeneveTunnelConfig_t));
    memset(&ipInIpTcfg, 0x0, sizeof(xpsIpinIpTunnelConfig_t));
    memset(&greTcfg, 0x0, sizeof(xpsIpGreTunnelConfig_t));
    memset(&vpnGreTcfg, 0x0, sizeof(xpsVpnGreTunnelConfig_t));

    tnlIdx = tunnelEntry->tnlIdx;
    l3TnlIntf[devId][tnlIdx] = 0;
    memset(&tunnelData, 0x0, sizeof(xpsAppIpTunnelData));

    tunnelData.dmac[0] = tunnelEntry->dmac[0];
    tunnelData.dmac[1] = tunnelEntry->dmac[1];
    tunnelData.dmac[2] = tunnelEntry->dmac[2];
    tunnelData.dmac[3] = tunnelEntry->dmac[3];
    tunnelData.dmac[4] = tunnelEntry->dmac[4];
    tunnelData.dmac[5] = tunnelEntry->dmac[5];
    tunnelData.smac = tunnelEntry->smac;
    tunnelData.tagType = tunnelEntry->tagType;
    tunnelData.dip[0] = tunnelEntry->dip[0];
    tunnelData.dip[1] = tunnelEntry->dip[1];
    tunnelData.dip[2] = tunnelEntry->dip[2];
    tunnelData.dip[3] = tunnelEntry->dip[3];
    tunnelData.myVtepIp[0] = tunnelEntry->myVtepIp[0];
    tunnelData.myVtepIp[1] = tunnelEntry->myVtepIp[1];
    tunnelData.myVtepIp[2] = tunnelEntry->myVtepIp[2];
    tunnelData.myVtepIp[3] = tunnelEntry->myVtepIp[3];
    tunnelData.vni = tunnelEntry->vni;
    tunnelData.vlan = tunnelEntry->vlan;
    tunnelData.udpPort = tunnelEntry->udpPort;

    tunnelData.tunnelId = tunnelEntry->tunnelVif;
    tunnelData.portId = tunnelEntry->portId;
    tunnelData.bdId = tunnelEntry->bdId;
    tunnelData.isMcast = tunnelEntry->isMcast;
    tunnelData.setBd = tunnelEntry->setBd;
    tunnelData.greProtocolId = tunnelEntry->greProtocolId;
    tunnelData.tnlVlan = tunnelEntry->tnlVlan;
    tunnelData.ipv4ARPBCCmd = tunnelEntry->ipv4ARPBCCmd;
    tunnelData.ipv4RouteEn = tunnelEntry->ipv4RouteEn;
    tunnelData.ipv6RouteEn = tunnelEntry->ipv6RouteEn;
    tunnelData.ipv4BridgeMcMode = tunnelEntry->ipv4BridgeMcMode;
    tunnelData.ipv6BridgeMcMode = tunnelEntry->ipv6BridgeMcMode;
    tunnelData.ipv4RouteMcEn = tunnelEntry->ipv4RouteMcEn;
    tunnelData.ipv6RouteMcEn = tunnelEntry->ipv6RouteMcEn;
    tunnelData.mplsRouteEn = tunnelEntry->mplsRouteEn;
    tunnelData.ipv4RouteMcMode = tunnelEntry->ipv4RouteMcMode;
    tunnelData.ipv6RouteMcMode = tunnelEntry->ipv6RouteMcMode;
    tunnelData.vrfId = tunnelEntry->vrfId;
    tunnelData.unregMcCmd = tunnelEntry->unregMcCmd;
    tunnelData.bind2Vlan = tunnelEntry->bind2Vlan;
    tunnelData.bindVlan = tunnelEntry->bindVlan;

    void *r  = xpsAppGetNhContext(devId, tunnelEntry->nhId);

    if (r)
    {
        nhIdx = (*(nhContextMap **)r)->nhIdx;
    }
    else
    {
        printf("ERROR: Nh not available for tnl nh Id %d\n", tunnelEntry->nhId);
        return XP_ERR_INVALID_DATA;
    }
    printf("tnlIdx: %d | NhId:%d | nhIdx: %d \n ", tnlIdx, tunnelEntry->nhId,
           nhIdx);

    //Write Tunnel entries in HW table

    serviceId = tunnelEntry->vni;

    //data.vlanId = tunnelData.bdId; //Vlan Id used after tunnel termination

    lclEpIpAddr[0] = tunnelEntry->myVtepIp[0];
    lclEpIpAddr[1] = tunnelEntry->myVtepIp[1];
    lclEpIpAddr[2] = tunnelEntry->myVtepIp[2];
    lclEpIpAddr[3] = tunnelEntry->myVtepIp[3];
    rmtEpIpAddr[0] = tunnelEntry->dip[0];
    rmtEpIpAddr[1] = tunnelEntry->dip[1];
    rmtEpIpAddr[2] = tunnelEntry->dip[2];
    rmtEpIpAddr[3] = tunnelEntry->dip[3];

    xpsScope_t scopeId;
    ret = xpsScopeGetScopeId(devId, &scopeId);

    if (ret != XP_NO_ERR)
    {
        printf("%s:Error in xpsScopeGetScopeId : error code: %d\n", __FUNCTION__, ret);
        return ret;
    }

    switch (tunnelEntry->tnlType)
    {
        case XP_CONFIGFILE_IPTNL_TYPE_VXLAN:
            printf("Tunnel Type: Vxlan\n");
            printf("VNI: %d \n", serviceId);
            printf("tunnel idx: %d \n", tunnelEntry->tnlIdx);
            tunnelData.tnlType = XPAPP_IPTNL_TYPE_VXLAN;
            tnlType[devId][tnlIdx] = tunnelEntry->tnlType;
            tnlVirtualId[devId][tnlIdx] = tunnelData.vni;

            ret = xpsVxlanAddLocalVtep(devId, tunnelEntry->myVtepIp);
            if (ret != XP_NO_ERR && ret != XP_ERR_KEY_EXISTS)
            {
                printf("ERROR: xpsVxlanAddLocalVtep returns %d\n", ret);
                return ret;
            }

            if ((ret = xpsVxlanCreateTunnelInterfaceScope(scopeId, lclEpIpAddr, rmtEpIpAddr,
                                                          &tnlIntfId)) != XP_NO_ERR)
            {
                printf("ERROR: xpsVxlanCreateTunnelInterface returns %d\n", ret);
                return ret;
            }

            tnlVif[devId][tnlIdx] = tnlIntfId;
            ret = xpsVlanAddEndpoint(devId, tunnelEntry->vlan, tnlIntfId, XP_L2_ENCAP_VXLAN,
                                     tunnelData.vni);
            if (ret != XP_NO_ERR && ret != XP_ERR_KEY_EXISTS)
            {
                printf("ERROR: xpsVlanAddEndpoint returns %d for vlan %d tnlIntfId %d encap %d tnlData %d\n",
                       ret, tunnelEntry->vlan, tnlIntfId, XP_L2_ENCAP_VXLAN, tunnelData.vni);
                return ret;
            }

            if (tunnelData.bind2Vlan)
            {
                ret = xpsVlanAddInterface(devId, tunnelData.bindVlan, tnlIntfId,
                                          XP_L2_ENCAP_VXLAN);
                if (ret != XP_NO_ERR && ret != XP_ERR_KEY_EXISTS)
                {
                    printf("ERROR: xpsVlanAddInterface returns %d\n", ret);
                    return ret;
                }
            }

            if (!tunnelData.isMcast)
            {
                if ((ret = xpsVxlanAddTunnelEntry(devId, tnlIntfId)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsVxlanAddTunnelEntry returns %d\n", ret);
                    return ret;
                }

                if ((ret = xpsVxlanSetTunnelNextHopData(devId, tnlIntfId, nhIdx)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsVxlanSetTunnelNextHopData returns %d\n", ret);
                    return ret;
                }

                /*TO_DO : populate xpsVxlanTunnelConfig_t *vxlanTcfgif required*/
                if ((ret = xpsVxlanSetTunnelConfig(devId, tnlIntfId, &vxlanTcfg)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsVxlanSetTunnelConfig returns %d\n", ret);
                    return ret;
                }
            }
            /* Consider isMcast */
            else
            {
                if ((ret = xpsPortGetPortIntfId(devId, tunnelData.portId,
                                                &portIntfId)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsVxlanAddMcTunnelEntry returns %d\n", ret);
                    return ret;
                }

                if ((ret = xpsVxlanAddMcTunnelEntry(devId, tnlIntfId, lclEpIpAddr, rmtEpIpAddr,
                                                    vlanToL3Intf[tunnelEntry->vlan], portIntfId)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsVxlanAddMcTunnelEntry returns %d\n", ret);
                    return ret;
                }

                ret = xpsVxlanAddLocalVtep(devId, rmtEpIpAddr);
                if (ret != XP_NO_ERR && ret != XP_ERR_KEY_EXISTS)
                {
                    printf("ERROR: xpsVxlanAddLocalVtep returns %d\n", ret);
                    return ret;
                }
            }

            break;

        case XP_CONFIGFILE_IPTNL_TYPE_NVGRE:
            tunnelData.tnlType = XPAPP_IPTNL_TYPE_NVGRE;
            tnlVirtualId[devId][tnlIdx] = tunnelData.vni;
            tnlType[devId][tnlIdx] = tunnelEntry->tnlType;

            printf("TNI: %d\n", serviceId);
            printf("Tunnel Type: NVGRE\n");
            // xpsAppConfigureNvgreTunnelStart(devId, tunnelData, &tunnelId);
            if ((ret = xpsNvgreCreateTunnelInterfaceScope(scopeId, lclEpIpAddr, rmtEpIpAddr,
                                                          &tnlIntfId)) != XP_NO_ERR)
            {
                printf("ERROR: xpsNvgreCreateTunnelInterface returns %d\n", ret);
                return ret;
            }

            ret = xpsNvgreAddLocalNve(devId, tunnelEntry->myVtepIp);
            if (ret != XP_NO_ERR && ret != XP_ERR_KEY_EXISTS)
            {
                printf("ERROR: xpsNvgreAddLocalNve returns %d\n", ret);
                return ret;
            }
            tnlVif[devId][tnlIdx] = tnlIntfId;

            ret = xpsVlanAddEndpoint(devId, tunnelEntry->vlan, tnlIntfId, XP_L2_ENCAP_NVGRE,
                                     tunnelData.vni);
            if (ret != XP_NO_ERR && ret != XP_ERR_KEY_EXISTS)
            {
                printf("ERROR: xpsVlanAddEndpoint returns %d for vlan %d tnlIntfId %d encap %d tnlData %d\n",
                       ret, tunnelEntry->vlan, tnlIntfId, XP_L2_ENCAP_NVGRE, tunnelData.vni);
                return ret;
            }

            if (tunnelData.bind2Vlan)
            {
                ret = xpsVlanAddInterface(devId, tunnelData.bindVlan, tnlIntfId,
                                          XP_L2_ENCAP_NVGRE);
                if (ret != XP_NO_ERR && ret != XP_ERR_KEY_EXISTS)
                {
                    printf("ERROR: xpsVlanAddInterface returns %d\n", ret);
                    return ret;
                }
            }

            if (!tunnelData.isMcast)
            {
                if ((ret = xpsNvgreAddTunnelEntry(devId, tnlIntfId)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsNvgreAddTunnelEntry returns %d\n", ret);
                    return ret;
                }

                if ((ret = xpsNvgreSetTunnelNextHopData(devId, tnlIntfId, nhIdx)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsNvgreSetTunnelNextHopData returns %d\n", ret);
                    return ret;
                }

                if ((ret = xpsNvgreSetTunnelConfig(devId, tnlIntfId, &nvgreTcfg)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsNvgreSetTunnelConfig returns %d\n", ret);
                    return ret;
                }
            }
            /* Consider isMcast */
            else
            {
                if ((ret = xpsPortGetPortIntfId(devId, tunnelData.portId,
                                                &portIntfId)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsVxlanAddMcTunnelEntry returns %d\n", ret);
                    return ret;
                }

                if ((ret = xpsNvgreAddMcTunnelEntry(devId, tnlIntfId, lclEpIpAddr, rmtEpIpAddr,
                                                    vlanToL3Intf[tunnelEntry->vlan], portIntfId)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsVxlanAddMcTunnelEntry returns %d\n", ret);
                    return ret;
                }

                ret = xpsNvgreAddLocalNve(devId, rmtEpIpAddr);
                if (ret != XP_NO_ERR && ret != XP_ERR_KEY_EXISTS)
                {
                    printf("ERROR: xpsNvgreAddLocalNve returns %d\n", ret);
                    return ret;
                }

            }

            //if(tunnelEntry->bind2Vlan == 1)
            //{
            //    xpsAppNvgreTunnelBindToVlan(devId, tunnelId, tunnelData.bindVlan);
            //}

            break;

        case XP_CONFIGFILE_IPTNL_TYPE_GENEVE:

            printf("Tunnel Type: GENEVE\n");
            tunnelData.tnlType = XPAPP_IPTNL_TYPE_GENEVE;


            // Create the base geneve interface with only fixed header
            index = 0;

            optionFormat = (xpsGeneveFormatType_t)index;
            tnlVirtualId[devId][tnlIdx + index] = tunnelData.vni;
            tnlType[devId][tnlIdx] = tunnelEntry->tnlType;
            printf("Geneve Type: %d\n", index);
            if ((ret = xpsGeneveCreateTunnelInterfaceScope(scopeId, lclEpIpAddr,
                                                           rmtEpIpAddr, optionFormat, &lstTnlId[index])) != XP_NO_ERR)
            {
                printf("ERROR: xpsGeneveCreateTunnelInterface returns %d\n", ret);
                return ret;
            }

            tnlVif[devId][tnlIdx + index] = lstTnlId[index];
            printf("tnlIdx : %d\n", tnlIdx);
            printf("tnlVif : %d\n", tnlVif[devId][tnlIdx + index]);
            if (tunnelData.bind2Vlan)
            {
                ret = xpsVlanAddInterface(devId, tunnelData.bindVlan,
                                          tnlVif[devId][tnlIdx + index], XP_L2_ENCAP_GENEVE);
                if (ret != XP_NO_ERR && ret != XP_ERR_KEY_EXISTS)
                {
                    printf("ERROR: xpsVlanAddInterface returns %d\n", ret);
                    return ret;
                }
            }

            ret = xpsGeneveAddLocalEndpoint(devId, tunnelEntry->myVtepIp);
            if (ret != XP_NO_ERR && ret != XP_ERR_KEY_EXISTS)
            {
                printf("ERROR: xpsGeneveAddLocalEndpoint returns %d\n", ret);
                return ret;
            }

            if ((ret = xpsGeneveAddTunnelEntry(devId, lstTnlId[index])) != XP_NO_ERR)
            {
                printf("ERROR: xpsGeneveAddTunnelEntry returns %d\n", ret);
                return ret;
            }


            if ((ret = xpsGeneveSetTunnelConfig(devId, lstTnlId[index],
                                                &geneveTcfg)) != XP_NO_ERR)
            {
                printf("ERROR: xpsGeneveSetTunnelConfig returns %d\n", ret);
                return ret;
            }

            if ((ret = xpsGeneveSetTunnelNextHopData(devId, lstTnlId[index],
                                                     nhIdx)) != XP_NO_ERR)
            {
                printf("ERROR: xpsGeneveSetTunnelNextHopData returns %d\n", ret);
                return ret;
            }

            // Create additional geneve options

            for (index = 1; index < MAX_GENEVE_SUPPORTED_OPTIONS; index++)
            {
                optionFormat = (xpsGeneveFormatType_t)index;
                tnlVirtualId[devId][tnlIdx + index] = tunnelData.vni;
                printf("Geneve Type: %d\n", index);

                if ((ret = xpsGeneveCreateTunnelInterfaceScope(scopeId, lclEpIpAddr,
                                                               rmtEpIpAddr, optionFormat, &lstTnlId[index])) != XP_NO_ERR)
                {
                    printf("ERROR: xpsGeneveCreateTunnelInterface returns %d\n", ret);
                    return ret;
                }

                if ((ret = xpsGeneveTunnelBindOption(devId, tnlVif[devId][tnlIdx],
                                                     lstTnlId[index])) != XP_NO_ERR)
                {
                    printf("ERROR: xpsGeneveTunnelBindOption returns %d\n", ret);
                    return ret;
                }

                tnlVif[devId][tnlIdx + index] = lstTnlId[index];
                if ((ret = xpsGeneveSetTunnelNextHopData(devId, lstTnlId[index],
                                                         nhIdx)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsGeneveSetTunnelNextHopData returns %d\n", ret);
                    return ret;
                }
            }

            break;

        case XP_CONFIGFILE_IPTNL_TYPE_IP_OVER_IP:
            printf("Tunnel Type: IPinIP\n");
            tunnelData.tnlType = XPAPP_IPTNL_TYPE_IP_OVER_IP;
            tnlType[devId][tnlIdx] = tunnelEntry->tnlType;

            memset(&ipData, 0, sizeof(xpsIpTunnelData_t));
            ipData.type = XP_IP_OVER_IP_TUNNEL;

            COPY_IPV4_ADDR_T(ipData.lclEpIpAddr.addr.ipv4Addr, lclEpIpAddr);
            COPY_IPV4_ADDR_T(ipData.rmtEpIpAddr.addr.ipv4Addr, rmtEpIpAddr);


            tnlVirtualId[devId][tnlIdx] = tunnelData.vni;

            if ((ret = xpsIpinIpCreateTunnelInterfaceScope(scopeId, &ipData,/*&tunnelData*/
                                                           &tnlIntfId)) != XP_NO_ERR)
            {
                printf("ERROR: xpsIpinIpCreateTunnelInterface returns %d\n", ret);
                return ret;
            }
            tnlVif[devId][tnlIdx] = tnlIntfId;

            if ((ret = xpsIpinIpAddTunnelEntry(devId, tnlIntfId)) != XP_NO_ERR)
            {
                printf("ERROR: xpsIpinIpAddTunnelEntry returns %d\n", ret);
                return ret;
            }

            if ((ret = xpsIpinIpSetTunnelConfig(devId, tnlIntfId,
                                                &ipInIpTcfg)) != XP_NO_ERR)
            {
                printf("ERROR: xpsIpinIpSetTunnelConfig returns %d\n", ret);
                return ret;
            }

            if ((ret = xpsIpinIpSetTunnelNextHopData(devId, tnlIntfId, nhIdx)) != XP_NO_ERR)
            {
                printf("ERROR: xpsIpinIpSetTunnelNextHopData returns %d\n", ret);
                return ret;
            }

            if (tunnelData.setBd)
            {
                xpsAppTnlInfParams *tnlInfParams = (xpsAppTnlInfParams *)malloc(sizeof(
                                                                                    xpsAppTnlInfParams));
                memset(tnlInfParams, 0x0, sizeof(xpsAppTnlInfParams));
                tnlInfParams->ipv4RouteEn =  tunnelEntry->ipv4RouteEn;
                tnlInfParams->ipv6RouteEn =  tunnelEntry->ipv6RouteEn;
                tnlInfParams->ipv4BridgeMcMode =  tunnelEntry->ipv4BridgeMcMode;
                tnlInfParams->ipv6BridgeMcMode =  tunnelEntry->ipv6BridgeMcMode;
                tnlInfParams->ipv4RouteMcEn =  tunnelEntry->ipv4RouteMcEn;
                tnlInfParams->ipv6RouteMcEn =  tunnelEntry->ipv6RouteMcEn;
                tnlInfParams->mplsRouteEn =  tunnelEntry->mplsRouteEn;
                tnlInfParams->ipv4RouteMcMode =  tunnelEntry->ipv4RouteMcMode;
                tnlInfParams->ipv6RouteMcMode =  tunnelEntry->ipv6RouteMcMode;
                tnlInfParams->vrfId =  tunnelEntry->vrfId;
                tnlInfParams->unregMcCmd =  tunnelEntry->unregMcCmd;

                xpsInterfaceId_t intfId;
                ret = xpsL3CreateTunnelIntfScope(scopeId, &intfId);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsL3CreateTunnelIntf : error code: %d\n", __FUNCTION__,
                           ret);
                    free(tnlInfParams);
                    return ret;
                }

                ret = xpsL3InitTunnelIntf(devId, intfId);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsL3InitTunnelIntf : error code: %d\n", __FUNCTION__, ret);
                    free(tnlInfParams);
                    return ret;
                }

                ret = xpsAppSetTnlInfParams(devId, intfId, tnlInfParams);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsAppSetTnlInfParams : error code: %d\n", __FUNCTION__,
                           ret);
                    return ret;
                }

                if ((ret = xpsL3BindTunnelIntf(devId, tnlIntfId, intfId)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsL3BindTunnelIntf returns %d\n", ret);
                    return ret;
                }
                l3TnlIntf[devId][tnlIdx] = intfId;
                printf("l3If for IPoverIP = %d\n", l3TnlIntf[devId][tnlIdx]);
                free(tnlInfParams);

            }

            break;

        case XP_CONFIGFILE_IPTNL_TYPE_GRE:
            printf("Tunnel Type: GRE\n");
            tunnelData.tnlType = XPAPP_IPTNL_TYPE_GRE;

            tnlVirtualId[devId][tnlIdx] = tunnelData.vni;
            tnlType[devId][tnlIdx] = tunnelEntry->tnlType;

            if ((ret = xpsIpGreCreateTunnelInterfaceScope(scopeId, lclEpIpAddr, rmtEpIpAddr,
                                                          &tnlIntfId)) != XP_NO_ERR)
            {
                printf("ERROR: xpsIpGreCreateTunnelInterface returns %d\n", ret);
                return ret;
            }
            tnlVif[devId][tnlIdx] = tnlIntfId;

            if ((ret = xpsIpGreAddTunnelEntry(devId, tnlIntfId)) != XP_NO_ERR)
            {
                printf("ERROR: xpsIpGreAddTunnelEntry returns %d\n", ret);
                return ret;
            }

            if ((ret = xpsIpGreSetTunnelConfig(devId, tnlIntfId, &greTcfg)) != XP_NO_ERR)
            {
                printf("ERROR: xpsIpGreSetTunnelConfig returns %d\n", ret);
                return ret;
            }

            if ((ret = xpsIpGreSetTunnelNextHopData(devId, tnlIntfId, nhIdx)) != XP_NO_ERR)
            {
                printf("ERROR: xpsIpGreSetTunnelNextHopData returns %d\n", ret);
                return ret;
            }

            if (tunnelData.setBd)
            {
                xpsAppTnlInfParams *tnlInfParams = (xpsAppTnlInfParams *)malloc(sizeof(
                                                                                    xpsAppTnlInfParams));
                memset(tnlInfParams, 0x0, sizeof(xpsAppTnlInfParams));
                tnlInfParams->ipv4RouteEn =  tunnelEntry->ipv4RouteEn;
                tnlInfParams->ipv6RouteEn =  tunnelEntry->ipv6RouteEn;
                tnlInfParams->ipv4BridgeMcMode =  tunnelEntry->ipv4BridgeMcMode;
                tnlInfParams->ipv6BridgeMcMode =  tunnelEntry->ipv6BridgeMcMode;
                tnlInfParams->ipv4RouteMcEn =  tunnelEntry->ipv4RouteMcEn;
                tnlInfParams->ipv6RouteMcEn =  tunnelEntry->ipv6RouteMcEn;
                tnlInfParams->mplsRouteEn =  tunnelEntry->mplsRouteEn;
                tnlInfParams->ipv4RouteMcMode =  tunnelEntry->ipv4RouteMcMode;
                tnlInfParams->ipv6RouteMcMode =  tunnelEntry->ipv6RouteMcMode;
                tnlInfParams->vrfId =  tunnelEntry->vrfId;
                tnlInfParams->unregMcCmd =  tunnelEntry->unregMcCmd;

                xpsInterfaceId_t intfId;
                ret = xpsL3CreateTunnelIntfScope(scopeId, &intfId);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsL3CreateTunnelIntf: error code: %d\n", __FUNCTION__,
                           ret);
                    free(tnlInfParams);
                    return ret;
                }

                ret = xpsL3InitTunnelIntf(devId, intfId);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsL3InitTunnelIntf : error code: %d\n", __FUNCTION__, ret);
                    free(tnlInfParams);
                    return ret;
                }

                ret = xpsAppSetTnlInfParams(devId, intfId, tnlInfParams);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsAppSetTnlInfParams : error code: %d\n", __FUNCTION__,
                           ret);
                    return ret;
                }


                if ((ret = xpsL3BindTunnelIntf(devId, tnlIntfId, intfId)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsL3TunnelIntfBind returns %d\n", ret);
                    return ret;
                }

                l3TnlIntf[devId][tnlIdx] = intfId;
                printf("l3If for GRE = %d\n", l3TnlIntf[devId][tnlIdx]);
                free(tnlInfParams);
            }
            break;

        case XP_CONFIGFILE_IPTNL_TYPE_VPN_GRE:
            printf("Tunnel Type: VPN GRE\n");
            tunnelData.tnlType = XPAPP_IPTNL_TYPE_VPN_GRE;

            tnlVirtualId[devId][tnlIdx] = tunnelData.vni;
            tnlType[devId][tnlIdx] = tunnelEntry->tnlType;

            if ((ret = xpsVpnGreCreateStrictModeTunnelInterfaceScope(scopeId, lclEpIpAddr,
                                                                     rmtEpIpAddr, tunnelData.vni, &tnlIntfId)) != XP_NO_ERR)
            {
                printf("ERROR: xpsVpnGreCreateTunnelInterface returns %d\n", ret);
                return ret;
            }
            tnlVif[devId][tnlIdx] = tnlIntfId;

            if ((ret = xpsVpnGreAddTunnelEntry(devId, tnlIntfId)) != XP_NO_ERR)
            {
                printf("ERROR: xpsVpnGreAddTunnelEntry returns %d\n", ret);
                return ret;
            }

            if ((ret = xpsVpnGreSetTunnelConfig(devId, tnlIntfId,
                                                &vpnGreTcfg)) != XP_NO_ERR)
            {
                printf("ERROR: xpsVpnGreSetTunnelConfig returns %d\n", ret);
                return ret;
            }

            if ((ret = xpsVpnGreSetTunnelNextHopData(devId, tnlIntfId, nhIdx)) != XP_NO_ERR)
            {
                printf("ERROR: xpsVpnGreSetTunnelNextHopData returns %d\n", ret);
                return ret;
            }

            if (tunnelData.setBd)
            {
                xpsAppTnlInfParams *tnlInfParams = (xpsAppTnlInfParams *)malloc(sizeof(
                                                                                    xpsAppTnlInfParams));
                memset(tnlInfParams, 0x0, sizeof(xpsAppTnlInfParams));
                tnlInfParams->ipv4RouteEn =  tunnelEntry->ipv4RouteEn;
                tnlInfParams->ipv6RouteEn =  tunnelEntry->ipv6RouteEn;
                tnlInfParams->ipv4RouteMcEn =  tunnelEntry->ipv4RouteMcEn;
                tnlInfParams->ipv6RouteMcEn =  tunnelEntry->ipv6RouteMcEn;
                tnlInfParams->mplsRouteEn =  tunnelEntry->mplsRouteEn;
                tnlInfParams->ipv4RouteMcMode =  tunnelEntry->ipv4RouteMcMode;
                tnlInfParams->ipv6RouteMcMode =  tunnelEntry->ipv6RouteMcMode;
                tnlInfParams->vrfId =  tunnelEntry->vrfId;
                tnlInfParams->unregMcCmd =  tunnelEntry->unregMcCmd;

                xpsInterfaceId_t intfId;
                ret = xpsL3CreateTunnelIntfScope(scopeId, &intfId);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsL3CreateTunnelIntf: error code: %d\n", __FUNCTION__,
                           ret);
                    free(tnlInfParams);
                    return ret;
                }

                ret = xpsL3InitTunnelIntf(devId, intfId);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsL3InitTunnelIntf : error code: %d\n", __FUNCTION__, ret);
                    free(tnlInfParams);
                    return ret;
                }

                ret = xpsAppSetTnlInfParams(devId, intfId, tnlInfParams);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsAppSetTnlInfParams : error code: %d\n", __FUNCTION__,
                           ret);
                    return ret;
                }


                if ((ret = xpsL3BindTunnelIntf(devId, tnlIntfId, intfId)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsL3TunnelIntfBind returns %d\n", ret);
                    return ret;
                }

                l3TnlIntf[devId][tnlIdx] = intfId;
                printf("l3If for GRE = %d\n", l3TnlIntf[devId][tnlIdx]);
                free(tnlInfParams);
            }
            break;
        case XP_CONFIGFILE_IPTNL_TYPE_VPN_GRE_LOOSE:
            printf("Tunnel Type: VPN GRE LOOSE\n");
            tunnelData.tnlType = XPAPP_IPTNL_TYPE_VPN_GRE_LOOSE;

            tnlVirtualId[devId][tnlIdx] = tunnelData.vni;
            tnlType[devId][tnlIdx] = tunnelEntry->tnlType;

            if ((ret = xpsVpnGreCreateLooseModeIpTunnelInterfaceScope(scopeId, lclEpIpAddr,
                                                                      rmtEpIpAddr, &tnlIntfId)) != XP_NO_ERR)
            {
                printf("ERROR: xpsVpnGreCreateTunnelInterface returns %d\n", ret);
                return ret;
            }
            tnlVif[devId][tnlIdx] = tnlIntfId;

            if ((ret = xpsVpnGreAddTunnelEntry(devId, tnlIntfId)) != XP_NO_ERR)
            {
                printf("ERROR: xpsVpnGreAddTunnelEntry returns %d\n", ret);
                return ret;
            }

            if ((ret = xpsVpnGreSetTunnelConfig(devId, tnlIntfId,
                                                &vpnGreTcfg)) != XP_NO_ERR)
            {
                printf("ERROR: xpsVpnGreSetTunnelConfig returns %d\n", ret);
                return ret;
            }

            if ((ret = xpsVpnGreSetTunnelNextHopData(devId, tnlIntfId, nhIdx)) != XP_NO_ERR)
            {
                printf("ERROR: xpsVpnGreSetTunnelNextHopData returns %d\n", ret);
                return ret;
            }

            if (tunnelData.setBd)
            {
                uint32_t vpnLabel = tunnelData.vni;
                xpsMplsVpnGreLooseModeParams_t vpnParams;
                memset(&vpnParams, 0x0, sizeof(xpsMplsVpnGreLooseModeParams_t));
                xpsAppTnlInfParams *tnlInfParams = (xpsAppTnlInfParams *)malloc(sizeof(
                                                                                    xpsAppTnlInfParams));
                memset(tnlInfParams, 0x0, sizeof(xpsAppTnlInfParams));
                tnlInfParams->ipv4RouteEn =  tunnelEntry->ipv4RouteEn;
                tnlInfParams->ipv6RouteEn =  tunnelEntry->ipv6RouteEn;
                tnlInfParams->ipv4BridgeMcMode =  tunnelEntry->ipv4BridgeMcMode;
                tnlInfParams->ipv6BridgeMcMode =  tunnelEntry->ipv6BridgeMcMode;
                tnlInfParams->ipv4RouteMcEn =  tunnelEntry->ipv4RouteMcEn;
                tnlInfParams->ipv6RouteMcEn =  tunnelEntry->ipv6RouteMcEn;
                tnlInfParams->mplsRouteEn =  tunnelEntry->mplsRouteEn;
                tnlInfParams->ipv4RouteMcMode =  tunnelEntry->ipv4RouteMcMode;
                tnlInfParams->ipv6RouteMcMode =  tunnelEntry->ipv6RouteMcMode;
                tnlInfParams->vrfId =  tunnelEntry->vrfId;
                tnlInfParams->unregMcCmd =  tunnelEntry->unregMcCmd;

                xpsInterfaceId_t intfId;
                ret = xpsL3CreateTunnelIntfScope(scopeId, &intfId);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsL3CreateTunnelIntf: error code: %d\n", __FUNCTION__,
                           ret);
                    free(tnlInfParams);
                    return ret;
                }

                ret = xpsL3InitTunnelIntf(devId, intfId);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsL3InitTunnelIntf : error code: %d\n", __FUNCTION__, ret);
                    free(tnlInfParams);
                    return ret;
                }

                ret = xpsAppSetTnlInfParams(devId, intfId, tnlInfParams);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsAppSetTnlInfParams : error code: %d\n", __FUNCTION__,
                           ret);
                    return ret;
                }

                vpnParams.bdId = XPS_INTF_MAP_INTF_TO_BD(intfId);

                if ((ret = xpsMplsAddVpnGreLooseModeEntry(devId, vpnLabel,
                                                          &vpnParams)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsMplsAddVpnGreLooseModeEntry returns %d\n", ret);
                    return ret;
                }

                l3TnlIntf[devId][tnlIdx] = intfId;
                printf("l3If for VPN GRE = %d\n", l3TnlIntf[devId][tnlIdx]);
                free(tnlInfParams);
            }
            break;
        case XP_CONFIGFILE_IPTNL_TYPE_GRE_ERSPAN2:
            {
                printf("Tunnel Type: GRE-ERSPAN2\n");
                tnlVirtualId[devId][tnlIdx] = tunnelData.vni;
                tnlType[devId][tnlIdx] = tunnelEntry->tnlType;

                lclEpInetAddr.type = XP_PREFIX_TYPE_IPV4;
                COPY_IPV4_ADDR_T(&lclEpInetAddr.addr.ipv4Addr, lclEpIpAddr);
                rmtEpInetAddr.type = XP_PREFIX_TYPE_IPV4;
                COPY_IPV4_ADDR_T(&rmtEpInetAddr.addr.ipv4Addr, rmtEpIpAddr);
                ret = xpsErspanGreCreateTunnelInterfaceScope(scopeId, &lclEpInetAddr,
                                                             &rmtEpInetAddr, &tnlIntfId);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsErspanGreCreateTunnelInterface : error code: %d\n",
                           __FUNCTION__, ret);
                    return ret;
                }

                l3TnlIntf[devId][tnlIdx] = tnlIntfId;

                ret = xpsErspanGreAddTunnelEntry(devId, tnlIntfId);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsErspanGreAddTunnelEntry : error code: %d\n",
                           __FUNCTION__, ret);
                    return ret;
                }
            }
            break;

        default:
            return XP_NO_ERR;
    }

    printf("%s:Info: Succesfully programmed Ip Tunnel entries in HW, tnlIntfId %d \n",
           __FUNCTION__, tnlIntfId);
    return XP_NO_ERR;
}

uint8_t xpsAppSetTnlInfParams(xpDevice_t devId, xpsInterfaceId_t intfId,
                              xpsAppTnlInfParams * tnlInfParams)
{

    xpsPktCmd_e pktCmd;
    uint32_t vrfId = 0xFF;
    XP_STATUS retVal = XP_NO_ERR;
    xpVlanRouteMcMode_t routeMode;
    uint8_t mplsRouteEn ;

    //program ipv4RouteEn
    pktCmd = (xpsPktCmd_e)(tnlInfParams->ipv4RouteEn);
    printf("xpsApp: ipv4RouteEn = %d\n", pktCmd);
    if ((retVal = xpsL3SetIntfIpv4UcRoutingEn(devId, intfId, pktCmd)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv4UcRoutingEn() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }
    //verify ipv4RouteEn
    uint32_t routeEn = 0;
    if ((retVal = xpsL3GetIntfIpv4UcRoutingEn(devId, intfId,
                                              &routeEn)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3IntGetIpv4UcRoutingEn() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }


    //program ipv6RouteEn
    pktCmd = (xpsPktCmd_e)(tnlInfParams->ipv6RouteEn);
    printf("xpsApp: ipv6RouteEn = %d\n", pktCmd);
    if ((retVal = xpsL3SetIntfIpv6UcRoutingEn(devId, intfId, pktCmd)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv6UcRoutingEn() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }

    //verify ipv6RouteEn
    routeEn = 0;
    if ((retVal = xpsL3GetIntfIpv6UcRoutingEn(devId, intfId,
                                              &routeEn)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3GetIntfIpv6UcRoutingEn() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }

    //program mpslRouteEn. Enable always for now
    mplsRouteEn=(xpsPktCmd_e)(tnlInfParams->mplsRouteEn);
    if ((retVal = xpsL3SetIntfMplsRoutingEn(devId, intfId,
                                            mplsRouteEn)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv6UcRoutingEn() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }
    printf("xpsApp: mplsRouteEn = %d on intfId: %d\n", mplsRouteEn, intfId);

    //program vrfId
    vrfId = (xpsPktCmd_e)(tnlInfParams->vrfId);
    printf("xpsAppSettnlInfParams = vrfId = %d\n", vrfId);
    if ((retVal = xpsL3SetIntfVrf(devId, intfId, vrfId)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfVrf() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }

    //verify vrfId
    vrfId = 0;
    if ((retVal = xpsL3GetIntfVrf(devId, intfId, &vrfId)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3GetIntfVrf() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }
    printf("getVrf() returns %d\n", vrfId);

    //program ipv4RouteMcEn
    pktCmd = (xpsPktCmd_e)(tnlInfParams->ipv4RouteMcEn);
    printf("xpsApp: ipv4RouteMcEn = %d\n", pktCmd);
    if ((retVal = xpsL3SetIntfIpv4McRoutingEn(devId, intfId, pktCmd)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv4McRoutingEn() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }

    //program ipv6RouteMcEn
    pktCmd = (xpsPktCmd_e)(tnlInfParams->ipv6RouteMcEn);
    printf("xpsApp: ipv4RouteMcEn = %d\n", pktCmd);

    // For Now Make Ipv4 Routing Enable. Later this will come from config.txt
    if ((retVal = xpsL3SetIntfIpv6McRoutingEn(devId, intfId, 0x1)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv6McRoutingEn() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }

    //program ipv4RouteMcMode
    routeMode = (xpVlanRouteMcMode_t)(tnlInfParams->ipv4RouteMcMode);
    printf("xpsApp: ipv4RouteMcMode = %d\n", routeMode);
    if ((retVal = xpsL3SetIntfIpv4McRouteMode(devId, intfId,
                                              routeMode)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv4McRouteMode() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }

    //program ipv6RouteMcMode
    routeMode = (xpVlanRouteMcMode_t)(tnlInfParams->ipv6RouteMcMode);
    printf("xpsApp: ipv6RouteMcMode = %d\n", routeMode);
    if ((retVal = xpsL3SetIntfIpv6McRouteMode(devId, intfId,
                                              routeMode)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsVlanSetIpv6McRouteMode() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }

    return XP_NO_ERR;

}

uint8_t xpsAppConfigurePbbTunnelData(xpsDevice_t devId,
                                     TunnelPBBEntry *tunnelEntry, uint32_t *tunnelId)
{
    xpsAppPbbTunnelData tunnelData;
    int ret;
    //uint32_t serviceId = 0;

    memset(&tunnelData, 0x0, sizeof(xpsAppPbbTunnelData));

    tunnelData.bDa[0] = tunnelEntry->bDa[0];
    tunnelData.bDa[1] = tunnelEntry->bDa[1];
    tunnelData.bDa[2] = tunnelEntry->bDa[2];
    tunnelData.bDa[3] = tunnelEntry->bDa[3];
    tunnelData.bDa[4] = tunnelEntry->bDa[4];
    tunnelData.bDa[5] = tunnelEntry->bDa[5];
    tunnelData.bSa[0] = tunnelEntry->bSa[0];
    tunnelData.bSa[1] = tunnelEntry->bSa[1];
    tunnelData.bSa[2] = tunnelEntry->bSa[2];
    tunnelData.bSa[3] = tunnelEntry->bSa[3];
    tunnelData.bSa[4] = tunnelEntry->bSa[4];
    tunnelData.bSa[5] = tunnelEntry->bSa[5];

    tunnelData.iSID = tunnelEntry->iSID;
    tunnelData.portId = tunnelEntry->portId;
    tunnelData.bTag = tunnelEntry->bTag;
    tunnelData.setBd = 1;
    tunnelData.bdId = tunnelEntry->bdId;

    //Write Tunnel entries in HW table

    //serviceId = tunnelEntry->iSID;

    xpsScope_t scopeId;
    if ((ret = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return ret;
    }

    if ((ret = xpsPbbCreateTunnelInterfaceScope(scopeId, tunnelData.bSa,
                                                tunnelData.bDa, tunnelData.bTag, tunnelId)) != XP_NO_ERR)
    {
        printf("ERROR: xpsPbbCreateTunnelInterfaceScope returns %d\n", ret);
        return ret;
    }

    ret = xpsVlanAddEndpoint(devId, tunnelData.bdId, *tunnelId, XP_L2_ENCAP_PBB,
                             tunnelData.iSID);
    if (ret != XP_NO_ERR && ret != XP_ERR_KEY_EXISTS)
    {
        printf("ERROR: xpsVlanAddEndpoint returns %d for vlan %d tnlIntfId %d encap %d tnlData %d\n",
               ret, tunnelData.bdId, *tunnelId, XP_L2_ENCAP_PBB, tunnelData.iSID);
        return ret;
    }

    if ((ret = xpsPbbAddTunnelEntry(devId, *tunnelId)) != XP_NO_ERR)
    {
        printf("ERROR: xpsPbbAddTunnelEntry returns %d\n", ret);
        return ret;
    }

    if ((ret = xpsPbbBindPortToTunnel(devId, *tunnelId,
                                      (xpsPort_t)tunnelData.portId)) != XP_NO_ERR)
    {
        printf("ERROR: xpsPbbBindPortToTunnel returns %d\n", ret);
        return ret;
    }

    if ((ret = xpsPbbAddIsid(devId, tunnelData.iSID,
                             (xpsVlan_t)tunnelData.bdId)) != XP_NO_ERR)
    {
        printf("ERROR: xpsPbbAddTunnelEntry returns %d\n", ret);
        return ret;
    }

    if (tunnelData.bind2Vlan)
    {
        ret = xpsVlanAddInterface(devId, tunnelData.bindVlan, *tunnelId,
                                  XP_L2_ENCAP_PBB);
        if (ret != XP_NO_ERR && ret != XP_ERR_KEY_EXISTS)
        {
            printf("ERROR: xpsVlanAddInterface returns %d\n", ret);
            return ret;
        }
    }


    printf("%s:Info: Succesfully programmed Pbb Tunnel entries in HW, tnlIntfId %d \n",
           __FUNCTION__, *tunnelId);
    return XP_NO_ERR;
}

XP_STATUS xpsAppConfigSrInterface(xpsDevice_t devId, SrhEntry *srhEntry,
                                  xpsInterfaceId_t *srIntf)
{
    XP_STATUS ret = XP_NO_ERR;
    xpsSrhData_t srhData;

    if (!srhEntry)
    {
        return XP_ERR_NULL_POINTER;
    }

    srhData.numOfSegments = srhEntry->numSegment;
    memcpy(srhData.segment0, srhEntry->ip0, sizeof(ipv6Addr_t));
    memcpy(srhData.segment1, srhEntry->ip1, sizeof(ipv6Addr_t));
    memcpy(srhData.segment2, srhEntry->ip2, sizeof(ipv6Addr_t));

    ret = xpsSrCreateInterface(devId, srIntf, srhData);

    printf("%s:srIntf = %d.\n", __FUNCTION__, *srIntf);
    if (ret != XP_NO_ERR)
    {

        return ret;
    }

    tnlVif[devId][srhEntry->srIdx] = *srIntf;
    return ret;
}

XP_STATUS xpsAppProgramSrhData(xpsDevice_t devId, uint8_t numSegment,
                               xpsInterfaceId_t srIntf, uint32_t nhId)
{
    XP_STATUS ret = XP_NO_ERR;

    ret = xpsSrSetNextHopData(devId, srIntf, nhId);

    if (ret != XP_NO_ERR)
    {
        return ret;
    }

    if (numSegment != 0)
    {
        ret = xpsSrOriginationEntry(devId, srIntf);
    }
    else
    {
        ret = xpsSrTerminationEntry(devId, srIntf);
    }
    return ret;
}
XP_STATUS xpsAppConfigureLocalSidData(xpDevice_t devId, localSidEntry *sidEntry)
{
    XP_STATUS ret = XP_NO_ERR;
    xpSidEntry localSidEntry;

    localSidEntry.localSidData.func = (xpSrEndFuncType_t)sidEntry->func;
    localSidEntry.localSidData.args = sidEntry->args;
    localSidEntry.localSidData.VRFId = sidEntry->VRFId;
    localSidEntry.localSidData.nhId = sidEntry->nhId;
    localSidEntry.localSidData.ecmpEnable = sidEntry->ecmpEnable;
    localSidEntry.localSidData.flag = sidEntry->flag;
    COPY_IPV6_ADDR_T(localSidEntry.localSid, sidEntry->localSid);
    //Set SID entry
    ret = xpsSrSetLocalSid(devId, localSidEntry);
    if (ret)
    {
        printf("%s:Error: Couldn't set SID entry, errorCode = %d.\n", __FUNCTION__,
               ret);
        return ret;
    }
    printf("SID entry set successfully.\n");

    return ret;
}

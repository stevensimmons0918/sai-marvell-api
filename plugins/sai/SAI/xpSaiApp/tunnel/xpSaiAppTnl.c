// xpSaiAppTnl.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiAppTnl.h"

extern uint32_t vrfIdxToVrf[MAX_VRF_IDX];

extern uint32_t tnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t vlanToL3Intf[4096];
extern uint32_t l3TnlIntf[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlType[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlVirtualId[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlIsid[XP_MAX_DEVICES][NUM_OF_TUNNELS];

XP_STATUS xpSaiAppConfigureIpTnnlData(xpDevice_t devId,
                                      sai_object_id_t switchOid, TunnelEntry *tunnelEntry)
{
    XP_STATUS ret = XP_NO_ERR;
    xpSaiAppIpTunnelData tunnelData;
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

    xpsIpTunnelData_t ipData;

    memset(&tunnelData, 0x0, sizeof(xpSaiAppIpTunnelData));
    memset(&vxlanTcfg, 0x0, sizeof(xpsVxlanTunnelConfig_t));
    memset(&nvgreTcfg, 0x0, sizeof(xpsNvgreTunnelConfig_t));
    memset(&geneveTcfg, 0x0, sizeof(xpsGeneveTunnelConfig_t));
    memset(&ipInIpTcfg, 0x0, sizeof(xpsIpinIpTunnelConfig_t));
    memset(&greTcfg, 0x0, sizeof(xpsIpGreTunnelConfig_t));
    memset(&lstTnlId, 0, sizeof(lstTnlId));
    memset(&lclEpInetAddr, 0, sizeof(inetAddr_t));
    memset(&rmtEpInetAddr, 0, sizeof(inetAddr_t));

    tnlIdx = tunnelEntry->tnlIdx;
    l3TnlIntf[devId][tnlIdx] = 0;

    memset(&tunnelData, 0x0, sizeof(xpSaiAppIpTunnelData));

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

    void *r  = xpSaiAppGetNhContext(devId, tunnelEntry->nhId);

    if (r)
    {
        nhIdx = (*(xpSaiAppNhContextMap **)r)->nhIdx;
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
        printf("ERROR: xpsScopeGetScopeId returns %d\n", ret);
        return ret;
    }


    switch (tunnelEntry->tnlType)
    {
        case XP_CONFIGFILE_IPTNL_TYPE_VXLAN:
            printf("Tunnel Type: Vxlan\n");
            printf("VNI: %d \n", serviceId);
            tunnelData.tnlType = XPAPP_IPTNL_TYPE_VXLAN;
            tnlType[devId][tnlIdx] = tunnelEntry->tnlType;
            tnlVirtualId[devId][tnlIdx] = tunnelData.vni;

            ret = xpsVxlanAddLocalVtep(devId, tunnelEntry->myVtepIp);
            if (ret != XP_NO_ERR && ret != XP_ERR_KEY_EXISTS)
            {
                printf("ERROR: xpsVxlanAddLocalVtep returns %d\n", ret);
                return ret;
            }

            if ((ret = xpsVxlanCreateTunnelInterfaceScope(scopeId, &lclEpInetAddr,
                                                          &rmtEpInetAddr,
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
                if ((ret = xpsVxlanAddTunnelEntry(devId, tnlIntfId, 0)) != XP_NO_ERR)
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

            tnlVirtualId[devId][tnlIdx] = tunnelData.vni;
            memset(&ipData, 0, sizeof(xpsIpTunnelData_t));
            ipData.type = XP_IP_OVER_IP_TUNNEL;

            COPY_IPV4_ADDR_T(ipData.lclEpIpAddr.addr.ipv4Addr, lclEpIpAddr);
            COPY_IPV4_ADDR_T(ipData.rmtEpIpAddr.addr.ipv4Addr, rmtEpIpAddr);


            if ((ret = xpsIpinIpCreateTunnelInterfaceScope(scopeId, &ipData,
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
                xpSaiAppTnlInfParams *tnlInfParams = (xpSaiAppTnlInfParams *)malloc(sizeof(
                                                                                        xpSaiAppTnlInfParams));
                if (tnlInfParams == NULL)
                {
                    printf("Error  allocating memory for tnlInfParams\n");
                    return XP_ERR_MEM_ALLOC_ERROR;
                }

                memset(tnlInfParams, 0x0, sizeof(xpSaiAppTnlInfParams));
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
                memset(&intfId, 0, sizeof(intfId));
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

                ret = xpSaiAppSetTnlInfParams(devId, intfId, tnlInfParams);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpSaiAppSetTnlInfParams : error code: %d\n", __FUNCTION__,
                           ret);
                    free(tnlInfParams);
                    return ret;
                }

                if ((ret = xpsL3BindTunnelIntf(devId, tnlIntfId, intfId)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsL3BindTunnelIntf returns %d\n", ret);
                    free(tnlInfParams);
                    return ret;
                }

                l3TnlIntf[devId][tnlIdx] = intfId;
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
                xpSaiAppTnlInfParams *tnlInfParams = (xpSaiAppTnlInfParams *)malloc(sizeof(
                                                                                        xpSaiAppTnlInfParams));
                if (tnlInfParams == NULL)
                {
                    printf("Error  allocating memory for tnlInfParams\n");
                    return XP_ERR_MEM_ALLOC_ERROR;
                }
                memset(tnlInfParams, 0x0, sizeof(xpSaiAppTnlInfParams));
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
                memset(&intfId, 0x0, sizeof(intfId));
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

                ret = xpSaiAppSetTnlInfParams(devId, intfId, tnlInfParams);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsAppSetTnlInfParams : error code: %d\n", __FUNCTION__,
                           ret);
                    free(tnlInfParams);
                    return ret;
                }

                if ((ret = xpsL3BindTunnelIntf(devId, tnlIntfId, intfId)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsL3TunnelIntfBind returns %d\n", ret);
                    free(tnlInfParams);
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
                xpSaiAppTnlInfParams *tnlInfParams = (xpSaiAppTnlInfParams *)malloc(sizeof(
                                                                                        xpSaiAppTnlInfParams));
                if (tnlInfParams == NULL)
                {
                    printf("Error  allocating memory for tnlInfParams\n");
                    return XP_ERR_MEM_ALLOC_ERROR;
                }
                memset(tnlInfParams, 0x0, sizeof(xpSaiAppTnlInfParams));
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
                memset(&intfId, 0x0, sizeof(intfId));
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

                ret = xpSaiAppSetTnlInfParams(devId, intfId, tnlInfParams);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsAppSetTnlInfParams : error code: %d\n", __FUNCTION__,
                           ret);
                    free(tnlInfParams);
                    return ret;
                }

                if ((ret = xpsL3BindTunnelIntf(devId, tnlIntfId, intfId)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsL3TunnelIntfBind returns %d\n", ret);
                    free(tnlInfParams);
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
                xpSaiAppTnlInfParams *tnlInfParams = (xpSaiAppTnlInfParams *)malloc(sizeof(
                                                                                        xpSaiAppTnlInfParams));
                if (tnlInfParams == NULL)
                {
                    printf("Error  allocating memory for tnlInfParams\n");
                    return XP_ERR_MEM_ALLOC_ERROR;
                }
                memset(tnlInfParams, 0x0, sizeof(xpSaiAppTnlInfParams));
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
                memset(&intfId, 0x0, sizeof(intfId));
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

                ret = xpSaiAppSetTnlInfParams(devId, intfId, tnlInfParams);
                if (ret != XP_NO_ERR)
                {
                    printf("%s:Error in xpsAppSetTnlInfParams : error code: %d\n", __FUNCTION__,
                           ret);
                    free(tnlInfParams);
                    return ret;
                }

                vpnParams.bdId = XPS_INTF_MAP_INTF_TO_BD(intfId);

                if ((ret = xpsMplsAddVpnGreLooseModeEntry(devId, vpnLabel,
                                                          &vpnParams)) != XP_NO_ERR)
                {
                    printf("ERROR: xpsMplsAddVpnGreLooseModeEntry returns %d\n", ret);
                    free(tnlInfParams);
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

XP_STATUS xpSaiAppSetTnlInfParams(xpDevice_t devId, xpsInterfaceId_t intfId,
                                  xpSaiAppTnlInfParams * tnlInfParams)
{

    xpsPktCmd_e pktCmd;
    uint32_t vrfId = 0xFF;
    XP_STATUS retVal = XP_NO_ERR;
    xpVlanRouteMcMode_t routeMode;
    uint8_t mplsRouteEn ;

    //program ipv4RouteEn
    pktCmd = (xpsPktCmd_e)(tnlInfParams->ipv4RouteEn);
    printf("xpSaiApp: ipv4RouteEn = %d\n", pktCmd);
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
    printf("xpSaiApp: ipv6RouteEn = %d\n", pktCmd);
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
    mplsRouteEn=(uint32_t)(tnlInfParams->mplsRouteEn);
    if ((retVal = xpsL3SetIntfMplsRoutingEn(devId, intfId,
                                            mplsRouteEn)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv6UcRoutingEn() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }
    printf("xpSaiApp: mplsRouteEn = %d on intfId: %d\n", mplsRouteEn, intfId);

    //program vrfId
    vrfId = vrfIdxToVrf[tnlInfParams->vrfId];

    printf("xpSaiAppSettnlInfParams = vrfId = %d\n", vrfId);
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
    printf("xpSaiApp: ipv4RouteMcEn = %d\n", pktCmd);
    if ((retVal = xpsL3SetIntfIpv4McRoutingEn(devId, intfId, pktCmd)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv4McRoutingEn() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }

    //program ipv6RouteMcEn
    pktCmd = (xpsPktCmd_e)(tnlInfParams->ipv6RouteMcEn);
    printf("xpSaiApp: ipv4RouteMcEn = %d\n", pktCmd);

    // For Now Make Ipv4 Routing Enable. Later this will come from config.txt
    if ((retVal = xpsL3SetIntfIpv6McRoutingEn(devId, intfId, 0x1)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv6McRoutingEn() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }

    //program ipv4RouteMcMode
    routeMode = (xpVlanRouteMcMode_t)(tnlInfParams->ipv4RouteMcMode);
    printf("xpSaiApp: ipv4RouteMcMode = %d\n", routeMode);
    if ((retVal = xpsL3SetIntfIpv4McRouteMode(devId, intfId,
                                              routeMode)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsL3SetIntfIpv4McRouteMode() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }

    //program ipv6RouteMcMode
    routeMode = (xpVlanRouteMcMode_t)(tnlInfParams->ipv6RouteMcMode);
    printf("xpSaiApp: ipv6RouteMcMode = %d\n", routeMode);
    if ((retVal = xpsL3SetIntfIpv6McRouteMode(devId, intfId,
                                              routeMode)) != XP_NO_ERR)
    {
        printf("%s:Error: xpsVlanSetIpv6McRouteMode() for intfId %d failed with error code: %d\n",
               __FUNCTION__, intfId, retVal);
        return retVal;
    }

    return XP_NO_ERR;

}

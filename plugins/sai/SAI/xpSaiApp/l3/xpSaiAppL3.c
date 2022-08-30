// xpSaiAppL3.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiAppL3.h"
#include "xpSai.h"
#include "xpSaiRoute.h"
#include "xpSaiAppSaiUtils.h"

XP_SAI_LOG_REGISTER_API(SAI_API_UNSPECIFIED);

extern int numOfEntry(entryList *list);

extern uint32_t vrfIdxToVrf[MAX_VRF_IDX];
extern uint32_t tnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t vlanToL3Intf[4096];
extern xpsInterfaceId_t gLagIntfId[MAX_NO_LAGS];
extern uint32_t l3TnlIntf[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlType[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlVirtualId[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlIsid[XP_MAX_DEVICES][NUM_OF_TUNNELS];


//global definition for nh database's "nhContextRoot"
static void *nhContextRoot[XP_MAX_DEVICES] = { NULL };

//compare function for nh database
int xpSaiAppCompareNhCtxMap(const void *l, const void *r)
{
    const xpSaiAppNhContextMap *lm = (xpSaiAppNhContextMap *)l;
    const xpSaiAppNhContextMap *lr = (xpSaiAppNhContextMap *)r;
    return lm->key - lr->key;
}

//get nexthop database
void* xpSaiAppGetNhContext(xpDevice_t devId, uint32_t nhIndex)
{
    xpSaiAppNhContextMap tmpNhCtx;

    tmpNhCtx.key = nhIndex;
    void *r = tfind(&tmpNhCtx, &nhContextRoot[devId], xpSaiAppCompareNhCtxMap);
    if (r)
    {
        //LOG HERE.
        printf("NhCtx: nhIndex: 0x%x\n", nhIndex);
    }
    return r;
}

//
// XpsApp Utility function to write Ipvx NH Table entries
//
XP_STATUS xpSaiAppPopulateNh(xpDevice_t devId, sai_object_id_t switchOid,
                             IpxNhEntry* ipvxNhData)
{
    XP_STATUS retVal = XP_NO_ERR;
    void *r = NULL;
    uint32_t intfId;
    int gTnlVif = 0;
    xpsScope_t scopeId;

    //get scope from Device ID
    retVal = xpsScopeGetScopeId(devId, &scopeId);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    if (xpSaiAppGetNhContext(devId, ipvxNhData->nhId) == NULL)
    {
        xpSaiAppNhContextMap *nhCtx = (xpSaiAppNhContextMap *)malloc(sizeof(
                                                                         xpSaiAppNhContextMap));
        if (!nhCtx)
        {
            printf("%s:Error: Can't allocate nh context for nh Index: %d\n", __FUNCTION__,
                   ipvxNhData->nhId);
            return XP_ERR_INIT;
        }

        if ((retVal = xpsL3CreateRouteNextHopScope(scopeId, 1,
                                                   &nhCtx->nhIdx)) != XP_NO_ERR)
        {
            free(nhCtx);
            printf("%s:Error xpsL3RouteNextHopCreate() error code: %d\n", __FUNCTION__,
                   retVal);
            return retVal;
        }
        printf("Created : NH %d for NH-Index : %d", nhCtx->nhIdx, ipvxNhData->nhId);

        memset(&nhCtx->nh, 0, sizeof(xpsL3NextHopEntry_t));
        nhCtx->key = ipvxNhData->nhId;
        nhCtx->nh.pktCmd = (xpPktCmd_e)(ipvxNhData->pktCmd);
        nhCtx->nh.propTTL = ipvxNhData->procTTL;
        nhCtx->nh.reasonCode = ipvxNhData->reasonCode;

        nhCtx->nh.nextHop.macDa[0] = ipvxNhData->macAddr[0];
        nhCtx->nh.nextHop.macDa[1] = ipvxNhData->macAddr[1];
        nhCtx->nh.nextHop.macDa[2] = ipvxNhData->macAddr[2];
        nhCtx->nh.nextHop.macDa[3] = ipvxNhData->macAddr[3];
        nhCtx->nh.nextHop.macDa[4] = ipvxNhData->macAddr[4];
        nhCtx->nh.nextHop.macDa[5] = ipvxNhData->macAddr[5];
        nhCtx->nh.nextHop.l3InterfaceId = vlanToL3Intf[ipvxNhData->cTagVid];

        if (ipvxNhData->isTagged == 0x0 || ipvxNhData->isTagged == 0x1) //Untagged
        {
            if (ipvxNhData->tnlIdx == -2) // Lag Port
            {
                nhCtx->nh.nextHop.egressIntfId = gLagIntfId[ipvxNhData->lagId];
            }
            else
            {
                /* If port is connected to port extender */
                if (ipvxNhData->extendedPortIdx != -1)
                {
                    /* Search for extended group context for perticular port*/
                    void *r = xpSaiAppGetExtPortsGroup(ipvxNhData->destPort);
                    if (r == NULL)
                    {
                        free(nhCtx);
                        printf("%s:Error: Can't found extPortGroup ctx in tree for port %d\n",
                               __FUNCTION__, ipvxNhData->destPort);
                        return XP_ERR_INVALID_ARG;
                    }

                    /* Check extended port count for group*/
                    if (ipvxNhData->extendedPortIdx >= (*(extPortGroup **)r)->numOfExtendedPorts)
                    {
                        free(nhCtx);
                        printf("%s:Error: invalid extendedPortIdx : %d for port %d\n", __FUNCTION__,
                               ipvxNhData->extendedPortIdx, ipvxNhData->destPort);
                        return XP_ERR_INVALID_ARG;
                    }
                    intfId = (*(extPortGroup **)r)->intfId[ipvxNhData->extendedPortIdx];
                }
                /* Port is normal port */
                else
                {
                    retVal = xpsPortGetPortIntfId(devId, ipvxNhData->destPort, &intfId);
                    if (retVal)
                    {
                        free(nhCtx);
                        printf("Error : xpsPortGetPortIntfId() Failed for interfac type :%d devId %d\n",
                               XPS_PORT, devId);
                        return retVal;
                    }
                }
                nhCtx->nh.nextHop.egressIntfId = intfId;
            }
        }
        else if (ipvxNhData->isTagged == 0x3) //MPLS tunnel
        {
            void *r  = getAllocatedMplsTnlVif(devId, ipvxNhData->mplsIdx);
            if (r == NULL)
            {
                free(nhCtx);
                printf("Error: mplsIdx : %d in not found in allocatedMplsTnlIvif \n",
                       ipvxNhData->mplsIdx);
                return XP_ERR_INIT;
            }
            nhCtx->nh.nextHop.egressIntfId  = (*(allocatedMplsTnlVifMap **)r)->tnlId;
        }
        else if (ipvxNhData->isTagged == 0x4) //MPLS L3 vpn
        {
            void *r  = getAllocatedMplsTnlVif(devId, ipvxNhData->mplsIdx);
            if (r == NULL)
            {
                free(nhCtx);
                printf("Error: mplsIdx : %d in not found in allocatedMplsTnlIvif \n",
                       ipvxNhData->mplsIdx);
                return XP_ERR_INIT;
            }
            nhCtx->nh.nextHop.egressIntfId  = (*(allocatedMplsTnlVifMap **)r)->tnlId;
            nhCtx->nh.vpnLabel  = ipvxNhData->virtualId;
        }
        else if (ipvxNhData->isTagged == 0x5) // gre
        {
            nhCtx->nh.nextHop.l3InterfaceId = l3TnlIntf[devId][ipvxNhData->tnlIdx];
            nhCtx->nh.nextHop.egressIntfId = tnlVif[devId][ipvxNhData->tnlIdx ];
        }
        else if (ipvxNhData->isTagged == 0x6) // Vxlan, nvgre
        {
            nhCtx->nh.nextHop.egressIntfId = tnlVif[devId][ipvxNhData->tnlIdx ];

            // Same Code for vxlan and nvgre Tunnel Next Hop
            nhCtx->nh.serviceInstId  = tnlVirtualId[devId][ipvxNhData->tnlIdx];
        }
        else if (ipvxNhData->isTagged == 0x7) //ipinip
        {
            nhCtx->nh.nextHop.l3InterfaceId = l3TnlIntf[devId][ipvxNhData->tnlIdx];
            nhCtx->nh.nextHop.egressIntfId = tnlVif[devId][ipvxNhData->tnlIdx ];
        }
        // Geneve tunnel
        else if (ipvxNhData->isTagged == 0x8) //geneve
        {
            gTnlVif = ipvxNhData->tnlIdx + ipvxNhData->geneveTunnelType;
            printf("IPv4 Host: Geneve Tunnel Vif : %d", gTnlVif);
            nhCtx->nh.nextHop.egressIntfId = tnlVif[devId][gTnlVif];

            // Same Code for vxlan ,nvgre and geneve Tunnel Next Hop
            nhCtx->nh.serviceInstId = tnlVirtualId[devId][ipvxNhData->tnlIdx];
        }
        else if (ipvxNhData->isTagged == 0xb) //Vpn Gre
        {
            nhCtx->nh.nextHop.egressIntfId = tnlVif[devId][ipvxNhData->tnlIdx];
            nhCtx->nh.vpnLabel = tnlVirtualId[devId][ipvxNhData->tnlIdx];
        }

        printf("pktCmd %d \n", nhCtx->nh.pktCmd);
        printf("reasonCode %d \n", nhCtx->nh.reasonCode);
        printf("MAC Addr 0x%2x:0x%2x:0x%2x:0x%2x:0x%2x:0x%2x \n",
               nhCtx->nh.nextHop.macDa[5], nhCtx->nh.nextHop.macDa[4],
               nhCtx->nh.nextHop.macDa[3], nhCtx->nh.nextHop.macDa[2],
               nhCtx->nh.nextHop.macDa[1], nhCtx->nh.nextHop.macDa[0]);
        printf("egressVif %d \n", nhCtx->nh.nextHop.egressIntfId);

        r = tsearch(nhCtx, &nhContextRoot[devId], xpSaiAppCompareNhCtxMap);
        if (r == NULL)
        {
            printf("%s:Error: Can't insert NhCtx in tree for vlanId %d\n", __FUNCTION__,
                   nhCtx->key);
            free(nhCtx);
            return XP_ERR_INIT;
        }

        printf("%s:Info: allocated Nh for nhId %d successfully\n", __FUNCTION__,
               ipvxNhData->nhId);

        if (ipvxNhData->isTagged == 0x4)
        {
            if ((retVal = xpsL3SetRouteVpnNextHop(devId, nhCtx->nhIdx,
                                                  &nhCtx->nh)) != XP_NO_ERR)
            {
                tdelete(nhCtx, &nhContextRoot[devId], xpSaiAppCompareNhCtxMap);
                free(nhCtx);
                printf("%s:Error in xpsL3SetRouteVpnNextHop an entry: error code: %d\n",
                       __FUNCTION__, retVal);
                return retVal;
            }
        }
        else
        {
            if ((retVal = xpsL3SetRouteNextHop(devId, nhCtx->nhIdx,
                                               &nhCtx->nh)) != XP_NO_ERR)
            {
                tdelete(nhCtx, &nhContextRoot[devId], xpSaiAppCompareNhCtxMap);
                free(nhCtx);
                printf("%s:Error in xpsL3SetRouteNextHop an entry: error code: %d\n",
                       __FUNCTION__, retVal);
                return retVal;
            }
        }
        free(nhCtx);
    }
    else
    {
        printf("%s:Info: NH context for nhId %d is already allocated\n", __FUNCTION__,
               ipvxNhData->nhId);
    }

    return retVal;
}

//IPv6 Host Table Configuration

XP_STATUS xpSaiAppWriteIpv6HostEntry(xpDevice_t devId,
                                     sai_object_id_t switchOid, Ipv6HostEntry* ipv6HostData)
{
    XP_STATUS retVal;
    xpsL3HostEntry_t pL3HostEntry;
    xpsL3HostEntry_t pL3HostEntryLkp;
    xpsHashIndexList_t indexList;

    if (!ipv6HostData)
    {
        printf("Null pointer received while adding Ipv6Host Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    memset(&pL3HostEntry, 0, sizeof(xpsL3HostEntry_t));
    memset(&pL3HostEntryLkp.nhEntry, 0, sizeof(xpsL3NextHopEntry_t));

    pL3HostEntry.type =(xpIpPrefixType_t) XP_PREFIX_TYPE_IPV6;
    pL3HostEntry.ipv6Addr[0]  = ipv6HostData->ipv6Addr [0];
    pL3HostEntry.ipv6Addr[1]  = ipv6HostData->ipv6Addr [1];
    pL3HostEntry.ipv6Addr[2]  = ipv6HostData->ipv6Addr [2];
    pL3HostEntry.ipv6Addr[3]  = ipv6HostData->ipv6Addr [3];
    pL3HostEntry.ipv6Addr[4]  = ipv6HostData->ipv6Addr [4];
    pL3HostEntry.ipv6Addr[5]  = ipv6HostData->ipv6Addr [5];
    pL3HostEntry.ipv6Addr[6]  = ipv6HostData->ipv6Addr [6];
    pL3HostEntry.ipv6Addr[7]  = ipv6HostData->ipv6Addr [7];
    pL3HostEntry.ipv6Addr[8]  = ipv6HostData->ipv6Addr [8];
    pL3HostEntry.ipv6Addr[9]  = ipv6HostData->ipv6Addr [9];
    pL3HostEntry.ipv6Addr[10] = ipv6HostData->ipv6Addr[10];
    pL3HostEntry.ipv6Addr[11] = ipv6HostData->ipv6Addr[11];
    pL3HostEntry.ipv6Addr[12] = ipv6HostData->ipv6Addr[12];
    pL3HostEntry.ipv6Addr[13] = ipv6HostData->ipv6Addr[13];
    pL3HostEntry.ipv6Addr[14] = ipv6HostData->ipv6Addr[14];
    pL3HostEntry.ipv6Addr[15] = ipv6HostData->ipv6Addr[15];

    pL3HostEntry.vrfId = vrfIdxToVrf[ipv6HostData->vrf];

    printf("IpAddress %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x, VRF %d \n",
           pL3HostEntry.ipv6Addr[15], pL3HostEntry.ipv6Addr[14], pL3HostEntry.ipv6Addr[13],
           pL3HostEntry.ipv6Addr[12], pL3HostEntry.ipv6Addr[11], pL3HostEntry.ipv6Addr[10],
           pL3HostEntry.ipv6Addr[9], pL3HostEntry.ipv6Addr[8], pL3HostEntry.ipv6Addr[7],
           pL3HostEntry.ipv6Addr[6], pL3HostEntry.ipv6Addr[5], pL3HostEntry.ipv6Addr[4],
           pL3HostEntry.ipv6Addr[3], pL3HostEntry.ipv6Addr[2], pL3HostEntry.ipv6Addr[1],
           pL3HostEntry.ipv6Addr[0], pL3HostEntry.vrfId);

    void *r  = xpSaiAppGetNhContext(devId, ipv6HostData->nhId);
    if (r == NULL)
    {
        printf("Error: nhId : %d in not found in nhContextMap \n", ipv6HostData->nhId);
        return XP_ERR_INIT;
    }
    pL3HostEntry.nhEntry = (*(xpSaiAppNhContextMap **)r)->nh;

    if (pL3HostEntry.nhEntry.pktCmd == XP_PKTCMD_TRAP)
    {
        if ((retVal = xpsL3AddIpHostControlEntry(devId, &pL3HostEntry,
                                                 &indexList)) != XP_NO_ERR)
        {
            printf("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
                   retVal);
            return retVal;
        }
    }
    else
    {
        if ((retVal = xpsL3AddIpHostEntry(devId, &pL3HostEntry,
                                          &indexList)) != XP_NO_ERR)
        {
            printf("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
                   retVal);
            return retVal;
        }
    }

    printf("writeIpv6HostEntry: Pass\n");

    printf("Perform Ipv6HostEntry  lookup for the same entry\n");

    pL3HostEntryLkp.type =(xpIpPrefixType_t) XP_PREFIX_TYPE_IPV6;
    pL3HostEntryLkp.ipv6Addr[0]  = ipv6HostData->ipv6Addr [0];
    pL3HostEntryLkp.ipv6Addr[1]  = ipv6HostData->ipv6Addr [1];
    pL3HostEntryLkp.ipv6Addr[2]  = ipv6HostData->ipv6Addr [2];
    pL3HostEntryLkp.ipv6Addr[3]  = ipv6HostData->ipv6Addr [3];
    pL3HostEntryLkp.ipv6Addr[4]  = ipv6HostData->ipv6Addr [4];
    pL3HostEntryLkp.ipv6Addr[5]  = ipv6HostData->ipv6Addr [5];
    pL3HostEntryLkp.ipv6Addr[6]  = ipv6HostData->ipv6Addr [6];
    pL3HostEntryLkp.ipv6Addr[7]  = ipv6HostData->ipv6Addr [7];
    pL3HostEntryLkp.ipv6Addr[8]  = ipv6HostData->ipv6Addr [8];
    pL3HostEntryLkp.ipv6Addr[9]  = ipv6HostData->ipv6Addr [9];
    pL3HostEntryLkp.ipv6Addr[10] = ipv6HostData->ipv6Addr[10];
    pL3HostEntryLkp.ipv6Addr[11] = ipv6HostData->ipv6Addr[11];
    pL3HostEntryLkp.ipv6Addr[12] = ipv6HostData->ipv6Addr[12];
    pL3HostEntryLkp.ipv6Addr[13] = ipv6HostData->ipv6Addr[13];
    pL3HostEntryLkp.ipv6Addr[14] = ipv6HostData->ipv6Addr[14];
    pL3HostEntryLkp.ipv6Addr[15] = ipv6HostData->ipv6Addr[15];
    pL3HostEntryLkp.vrfId = vrfIdxToVrf[ipv6HostData->vrf];

    if ((retVal = xpsL3GetIpHostEntry(devId, &pL3HostEntryLkp)) != XP_NO_ERR)
    {
        printf("Error in ipv6Host table lookup: error code: %d\n", retVal);
        return retVal;
    }
    printf("Lookup: ipv6Host lookup: pktCmd = %d  egressVif = %d \n",
           pL3HostEntryLkp.nhEntry.pktCmd, pL3HostEntryLkp.nhEntry.nextHop.egressIntfId);
    return XP_NO_ERR;
}


///IPv6 Route

XP_STATUS xpSaiAppWriteIpv6RouteEntry(xpDevice_t devId,
                                      sai_object_id_t switchOid, Ipv6RouteEntry* ipv6RouteData)
{
    XP_STATUS retVal;
    uint32_t ecmpSize, lkpecmpSize=0;
    uint32_t nhIdx;
    xpsL3RouteEntry_t pL3RouteEntry;
    xpsL3RouteEntry_t pL3RouteEntryLkp;
    xpsL3NextHopEntry_t pL3NextHopEntryLkp;
    uint32_t prefixIndex = 0xFFFFFFFF;

    memset(&pL3RouteEntry, 0, sizeof(xpsL3RouteEntry_t));
    memset(&pL3RouteEntryLkp, 0, sizeof(xpsL3RouteEntry_t));
    memset(&pL3NextHopEntryLkp, 0, sizeof(xpsL3NextHopEntry_t));

    pL3RouteEntry.type = (xpIpPrefixType_t) XP_PREFIX_TYPE_IPV6;

    if (!ipv6RouteData)
    {
        printf("Null pointer received while adding ipv6Route Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    pL3RouteEntry.ipv6Addr[0]  = ipv6RouteData->ipv6Addr[0];
    pL3RouteEntry.ipv6Addr[1]  = ipv6RouteData->ipv6Addr[1];
    pL3RouteEntry.ipv6Addr[2]  = ipv6RouteData->ipv6Addr[2];
    pL3RouteEntry.ipv6Addr[3]  = ipv6RouteData->ipv6Addr[3];
    pL3RouteEntry.ipv6Addr[4]  = ipv6RouteData->ipv6Addr[4];
    pL3RouteEntry.ipv6Addr[5]  = ipv6RouteData->ipv6Addr[5];
    pL3RouteEntry.ipv6Addr[6]  = ipv6RouteData->ipv6Addr[6];
    pL3RouteEntry.ipv6Addr[7]  = ipv6RouteData->ipv6Addr[7];
    pL3RouteEntry.ipv6Addr[8]  = ipv6RouteData->ipv6Addr[8];
    pL3RouteEntry.ipv6Addr[9]  = ipv6RouteData->ipv6Addr[9];
    pL3RouteEntry.ipv6Addr[10] = ipv6RouteData->ipv6Addr[10];
    pL3RouteEntry.ipv6Addr[11] = ipv6RouteData->ipv6Addr[11];
    pL3RouteEntry.ipv6Addr[12] = ipv6RouteData->ipv6Addr[12];
    pL3RouteEntry.ipv6Addr[13] = ipv6RouteData->ipv6Addr[13];
    pL3RouteEntry.ipv6Addr[14] = ipv6RouteData->ipv6Addr[14];
    pL3RouteEntry.ipv6Addr[15] = ipv6RouteData->ipv6Addr[15];
    pL3RouteEntry.vrfId = vrfIdxToVrf[ipv6RouteData->vrf];
    pL3RouteEntry.nhEcmpSize = ipv6RouteData->ecmpSize;
    ecmpSize = pL3RouteEntry.nhEcmpSize;
    pL3RouteEntry.ipMaskLen =ipv6RouteData->netMask;

    printf("pL3RouteEntry.ipv6Addr %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x, VRF %d \n",
           pL3RouteEntry.ipv6Addr[15], pL3RouteEntry.ipv6Addr[14],
           pL3RouteEntry.ipv6Addr[13], pL3RouteEntry.ipv6Addr[12],
           pL3RouteEntry.ipv6Addr[11], pL3RouteEntry.ipv6Addr[10],
           pL3RouteEntry.ipv6Addr[9], pL3RouteEntry.ipv6Addr[8], pL3RouteEntry.ipv6Addr[7],
           pL3RouteEntry.ipv6Addr[6], pL3RouteEntry.ipv6Addr[5], pL3RouteEntry.ipv6Addr[4],
           pL3RouteEntry.ipv6Addr[3], pL3RouteEntry.ipv6Addr[2], pL3RouteEntry.ipv6Addr[1],
           pL3RouteEntry.ipv6Addr[0], pL3RouteEntry.vrfId);

    uint32_t i;
    for (i = 0; i < ecmpSize; i++) // Add muliple Nh for IP Prefix
    {
        nhIdx=*((uint16_t *)getEntry(&(ipv6RouteData->nhId), i));
        void *r  = xpSaiAppGetNhContext(devId, nhIdx);
        if (r == NULL)
        {
            printf("Error: nhId : %d in not found in nhContextMap \n", nhIdx);
            return XP_ERR_INIT;
        }

        printf("NetMask: %d | EcmpSize: %d \n", pL3RouteEntry.ipMaskLen,
               pL3RouteEntry.nhEcmpSize);

        if (i == 0)
        {
            pL3RouteEntry.nhId = (*(xpSaiAppNhContextMap **)r)->nhIdx;
        }
    }

    if ((retVal = xpsL3AddIpRouteEntry(devId, &pL3RouteEntry,
                                       &prefixIndex, NULL)) != XP_NO_ERR)
    {
        printf("%s:Error in inserting an entry: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    printf("writeIpv6RouteEntry: Pass\n");

    printf("Perform Ipv6Route lookup for the same entry\n");


    pL3RouteEntryLkp.ipv6Addr[0]  = ipv6RouteData->ipv6Addr[0];
    pL3RouteEntryLkp.ipv6Addr[1]  = ipv6RouteData->ipv6Addr[1];
    pL3RouteEntryLkp.ipv6Addr[2]  = ipv6RouteData->ipv6Addr[2];
    pL3RouteEntryLkp.ipv6Addr[3]  = ipv6RouteData->ipv6Addr[3];
    pL3RouteEntryLkp.ipv6Addr[4]  = ipv6RouteData->ipv6Addr[4];
    pL3RouteEntryLkp.ipv6Addr[5]  = ipv6RouteData->ipv6Addr[5];
    pL3RouteEntryLkp.ipv6Addr[6]  = ipv6RouteData->ipv6Addr[6];
    pL3RouteEntryLkp.ipv6Addr[7]  = ipv6RouteData->ipv6Addr[7];
    pL3RouteEntryLkp.ipv6Addr[8]  = ipv6RouteData->ipv6Addr[8];
    pL3RouteEntryLkp.ipv6Addr[9]  = ipv6RouteData->ipv6Addr[9];
    pL3RouteEntryLkp.ipv6Addr[10] = ipv6RouteData->ipv6Addr[10];
    pL3RouteEntryLkp.ipv6Addr[11] = ipv6RouteData->ipv6Addr[11];
    pL3RouteEntryLkp.ipv6Addr[12] = ipv6RouteData->ipv6Addr[12];
    pL3RouteEntryLkp.ipv6Addr[13] = ipv6RouteData->ipv6Addr[13];
    pL3RouteEntryLkp.ipv6Addr[14] = ipv6RouteData->ipv6Addr[14];
    pL3RouteEntryLkp.ipv6Addr[15] = ipv6RouteData->ipv6Addr[15];
    pL3RouteEntryLkp.vrfId = vrfIdxToVrf[ipv6RouteData->vrf];
    pL3RouteEntryLkp.ipMaskLen=ipv6RouteData->netMask;
    pL3RouteEntryLkp.type = (xpIpPrefixType_t) XP_PREFIX_TYPE_IPV6;

    printf("Prefix %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x, VRF %d \n",
           pL3RouteEntryLkp.ipv6Addr[15], pL3RouteEntryLkp.ipv6Addr[14],
           pL3RouteEntryLkp.ipv6Addr[13], pL3RouteEntryLkp.ipv6Addr[12],
           pL3RouteEntryLkp.ipv6Addr[11], pL3RouteEntryLkp.ipv6Addr[10],
           pL3RouteEntryLkp.ipv6Addr[9], pL3RouteEntryLkp.ipv6Addr[8],
           pL3RouteEntryLkp.ipv6Addr[7], pL3RouteEntryLkp.ipv6Addr[6],
           pL3RouteEntryLkp.ipv6Addr[5], pL3RouteEntryLkp.ipv6Addr[4],
           pL3RouteEntryLkp.ipv6Addr[3], pL3RouteEntryLkp.ipv6Addr[2],
           pL3RouteEntryLkp.ipv6Addr[1], pL3RouteEntryLkp.ipv6Addr[0],
           pL3RouteEntryLkp.vrfId);

    if ((retVal = xpsL3FindIpRouteEntry(devId, &pL3RouteEntryLkp,
                                        &prefixIndex)) != XP_NO_ERR)
    {
        printf("Error in ipv4Route table lookup: error code: %d\n", retVal);
        return retVal;
    }

    lkpecmpSize = pL3RouteEntry.nhEcmpSize;
    printf("Lookup: NetMask: %d | EcmpSize: %d | LookupNhId: %d \n",
           pL3RouteEntryLkp.ipMaskLen, pL3RouteEntry.nhEcmpSize, pL3RouteEntry.nhId);

    for (i=0; i < lkpecmpSize; i++)
    {
        if ((retVal = xpsL3GetRouteNextHop(devId, pL3RouteEntry.nhId,
                                           &pL3NextHopEntryLkp)) != XP_NO_ERR)
        {
            printf("Error in Nh lookup: error code: %d\n", retVal);
            return retVal;
        }
        printf("Lookup: ipv4Host lookup: pktCmd = %d  egressVif = %d \n",
               pL3NextHopEntryLkp.pktCmd, pL3NextHopEntryLkp.nextHop.egressIntfId);
        pL3RouteEntry.nhId++;
    }

    return XP_NO_ERR;
}

// XpApp Utility function to write IPv4 Host Table entries

XP_STATUS xpSaiAppWriteIpv4HostEntry(xpDevice_t devId,
                                     sai_object_id_t switchOid, Ipv4HostEntry* ipv4HostData)
{
    XP_STATUS retVal;
    xpsL3HostEntry_t pL3HostEntry;
    xpsL3HostEntry_t pL3HostEntryLkp;
    xpsHashIndexList_t indexList;
    sai_neighbor_entry_t l3HostEntry;
    sai_attribute_t saiAttr[2];
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t ipV4Addr = 0;
    uint32_t pktCmd = 0;

    memset(&l3HostEntry, 0, sizeof(l3HostEntry));

    if (!ipv4HostData)
    {
        printf("Null pointer received while adding Ipv4Host Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    memset(&pL3HostEntry, 0, sizeof(xpsL3HostEntry_t));
    memset(&pL3HostEntryLkp, 0, sizeof(xpsL3HostEntry_t));

    pL3HostEntry.type =(xpIpPrefixType_t) XP_PREFIX_TYPE_IPV4;

    pL3HostEntry.ipv4Addr[0]  = ipv4HostData->ipv4Addr[0];
    pL3HostEntry.ipv4Addr[1]  = ipv4HostData->ipv4Addr[1];
    pL3HostEntry.ipv4Addr[2]  = ipv4HostData->ipv4Addr[2];
    pL3HostEntry.ipv4Addr[3]  = ipv4HostData->ipv4Addr[3];
    pL3HostEntry.vrfId = vrfIdxToVrf[ipv4HostData->vrf];

    printf("IpAddress %d.%d.%d.%d, VRF %d \n", pL3HostEntry.ipv4Addr[3],
           pL3HostEntry.ipv4Addr[2], pL3HostEntry.ipv4Addr[1], pL3HostEntry.ipv4Addr[0],
           pL3HostEntry.vrfId);

    void *r  = xpSaiAppGetNhContext(devId, ipv4HostData->nhId);
    if (r == NULL)
    {
        printf("Error: nhId : %d in not found in nhContextMap \n", ipv4HostData->nhId);
        return XP_ERR_INIT;
    }
    pL3HostEntry.nhEntry = (*(xpSaiAppNhContextMap **)r)->nh;

    if ((retVal = xpsL3AddIpHostEntry(devId, &pL3HostEntry,
                                      &indexList)) != XP_NO_ERR)
    {
        printf("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    printf("writeIpv4HostEntry: Pass\n");

    printf("Perform Ipv4HostEntry  lookup for the same entry using sai\n");

    pL3HostEntryLkp.ipv4Addr[0] = ipv4HostData->ipv4Addr[0];
    pL3HostEntryLkp.ipv4Addr[1] = ipv4HostData->ipv4Addr[1];
    pL3HostEntryLkp.ipv4Addr[2] = ipv4HostData->ipv4Addr[2];
    pL3HostEntryLkp.ipv4Addr[3] = ipv4HostData->ipv4Addr[3];
    pL3HostEntryLkp.vrfId = vrfIdxToVrf[ipv4HostData->vrf];

    l3HostEntry.switch_id = switchOid;
    l3HostEntry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    memcpy(&ipV4Addr, pL3HostEntryLkp.ipv4Addr, sizeof(ipV4Addr));
    l3HostEntry.ip_address.addr.ip4 =  htonl(ipV4Addr);

    l3HostEntry.rif_id = xpSaiAppSaiGetVlanRifOid(
                             pL3HostEntry.nhEntry.nextHop.l3InterfaceId);

    saiAttr[0].id = SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION;
    saiAttr[1].id = SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS;

    saiRetVal = xpSaiNeighborApi->get_neighbor_entry_attribute(&l3HostEntry, 2,
                                                               saiAttr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error in ipv4Host table lookup: error code: %d\n", saiRetVal);
        return saiStatus2XpsStatus(saiRetVal);
    }

    pktCmd = xpSaiObjIdValueGet(saiAttr[0].value.s32);
    printf("Lookup: ipv4Host lookup: pktCmd = %d and mac = %x:%x:%x:%x:%x:%x\n",
           pktCmd, saiAttr[1].value.mac[0], saiAttr[1].value.mac[1],
           saiAttr[1].value.mac[2], saiAttr[1].value.mac[3], saiAttr[1].value.mac[4],
           saiAttr[1].value.mac[5]);

    return XP_NO_ERR;
}

XP_STATUS xpSaiAppWriteIpv4RouteEntry(xpDevice_t devId,
                                      sai_object_id_t switchOid, Ipv4RouteEntry* ipv4RouteData)
{
    XP_STATUS retVal;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t ecmpSize, lkpecmpSize=0;
    uint32_t nhIdx;
    xpsL3RouteEntry_t pL3RouteEntry;
    xpsL3RouteEntry_t pL3RouteEntryLkp;
    xpsL3NextHopEntry_t pL3NextHopEntryLkp;
    uint32_t prefixIndex = 0xFFFFFFFF;
    uint32_t  ipV4Addr  = 0;
    uint32_t  ipV4Mask  = 0;
    uint32_t pktCmd = 0;
    sai_route_entry_t routeEntry;
    sai_attribute_t saiAttr[2];

    memset(&routeEntry, 0, sizeof(routeEntry));
    memset(saiAttr, 0, sizeof(saiAttr));
    memset(&pL3RouteEntry, 0, sizeof(xpsL3RouteEntry_t));
    memset(&pL3RouteEntryLkp, 0, sizeof(xpsL3RouteEntry_t));
    memset(&pL3NextHopEntryLkp, 0, sizeof(xpsL3NextHopEntry_t));

    pL3RouteEntry.type = XP_PREFIX_TYPE_IPV4;

    if (!ipv4RouteData)
    {
        printf("Null pointer received while adding ipv4Route Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    pL3RouteEntry.ipv4Addr[0] = ipv4RouteData->ipv4Addr[0];
    pL3RouteEntry.ipv4Addr[1] = ipv4RouteData->ipv4Addr[1];
    pL3RouteEntry.ipv4Addr[2] = ipv4RouteData->ipv4Addr[2];
    pL3RouteEntry.ipv4Addr[3] = ipv4RouteData->ipv4Addr[3];
    pL3RouteEntry.vrfId = vrfIdxToVrf[ipv4RouteData->vrf];
    pL3RouteEntry.nhEcmpSize = ipv4RouteData->ecmpSize;
    ecmpSize = pL3RouteEntry.nhEcmpSize;
    pL3RouteEntry.ipMaskLen =ipv4RouteData->netMask;
    printf("IpAddress %d.%d.%d.%d, VRF %d \n", pL3RouteEntry.ipv4Addr[3],
           pL3RouteEntry.ipv4Addr[2], pL3RouteEntry.ipv4Addr[1], pL3RouteEntry.ipv4Addr[0],
           pL3RouteEntry.vrfId);

    uint32_t i;
    for (i = 0; i < ecmpSize; i++) // Add muliple Nh for IP Prefix
    {
        nhIdx=*((uint16_t *)getEntry(&(ipv4RouteData->nhId), i));
        void *r  = xpSaiAppGetNhContext(devId, nhIdx);
        if (r == NULL)
        {
            printf("Error: nhId : %d in not found in nhContextMap \n", nhIdx);
            return XP_ERR_INIT;
        }

        printf("NetMask: %d | EcmpSize: %d \n", pL3RouteEntry.ipMaskLen,
               pL3RouteEntry.nhEcmpSize);

        if (i == 0)
        {
            pL3RouteEntry.nhId = (*(xpSaiAppNhContextMap **)r)->nhIdx;
        }
    }

    if ((retVal = xpsL3AddIpRouteEntry(devId, &pL3RouteEntry,
                                       &prefixIndex, NULL)) != XP_NO_ERR)
    {
        printf("%s:Error in inserting an entry: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    printf("Perform Ipv4Route lookup for the same entry using sai\n");

    pL3RouteEntryLkp.ipv4Addr[0] = ipv4RouteData->ipv4Addr[0];
    pL3RouteEntryLkp.ipv4Addr[1] = ipv4RouteData->ipv4Addr[1];
    pL3RouteEntryLkp.ipv4Addr[2] = ipv4RouteData->ipv4Addr[2];
    pL3RouteEntryLkp.ipv4Addr[3] = ipv4RouteData->ipv4Addr[3];
    pL3RouteEntryLkp.vrfId = vrfIdxToVrf[ipv4RouteData->vrf];
    pL3RouteEntryLkp.ipMaskLen =ipv4RouteData->netMask;

    lkpecmpSize = pL3RouteEntry.nhEcmpSize;

    routeEntry.switch_id = switchOid;
    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, devId,
                                 (sai_uint64_t) pL3RouteEntryLkp.vrfId, &routeEntry.vr_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI virtual router object could not be created\n");
        return saiStatus2XpsStatus(saiRetVal);
    }

    routeEntry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    memcpy(&ipV4Addr, pL3RouteEntryLkp.ipv4Addr, sizeof(ipV4Addr));
    routeEntry.destination.addr.ip4 = htonl(ipV4Addr);
    saiRetVal = xpSaiConvertPrefix2IPv4Mask(pL3RouteEntryLkp.ipMaskLen, &ipV4Mask);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI xpSaiConvertPrefix2IPv4Mask() failed : retVal %d | devId %d \n",
               saiRetVal, devId);
        return saiStatus2XpsStatus(saiRetVal);
    }

    routeEntry.destination.mask.ip4 = htonl(ipV4Mask);
    saiAttr[0].id= SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION;
    saiAttr[1].id= SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID;

    saiRetVal = xpSaiRouteApi->get_route_entry_attribute(&routeEntry, 2, saiAttr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error in ipv4Route table lookup: error code : retVal %d | devId %d \n",
               saiRetVal, devId);
        return saiStatus2XpsStatus(saiRetVal);
    }

    pktCmd = xpSaiObjIdValueGet(saiAttr[0].value.s32);
    printf("Lookup: NetMask: %d | EcmpSize: %d | LookupNhId: %" PRIu64
           " | pktcmd: %d \n", pL3RouteEntryLkp.ipMaskLen, pL3RouteEntry.nhEcmpSize,
           saiAttr[1].value.oid, pktCmd);

    for (i=0; i < lkpecmpSize; i++)
    {
        if ((retVal = xpsL3GetRouteNextHop(devId, pL3RouteEntry.nhId,
                                           &pL3NextHopEntryLkp)) != XP_NO_ERR)
        {
            printf("Error in Nh lookup: error code: %d\n", retVal);
            return retVal;
        }
        printf("Lookup: ipv4Host lookup: pktCmd = %d  reasonCode = %d egressVif = %d \n",
               pL3NextHopEntryLkp.pktCmd, pL3NextHopEntryLkp.reasonCode,
               pL3NextHopEntryLkp.nextHop.egressIntfId);
        pL3RouteEntry.nhId++;
    }
    return XP_NO_ERR;
}

void xpSaiAppL3CleanUp(xpsDevice_t devId)
{
    printf("Calling %s\n", __FUNCTION__);
    xpSaiAppNhContextMap *item;
    while (nhContextRoot[devId] != NULL)
    {
        item = *(xpSaiAppNhContextMap **) nhContextRoot[devId];
        tdelete(item, &nhContextRoot[devId], xpSaiAppCompareNhCtxMap);
        free(item);
    }
}

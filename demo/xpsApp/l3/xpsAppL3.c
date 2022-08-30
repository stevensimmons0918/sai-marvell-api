// xpsAppL3.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

//#include "xpApp.h"
#include "xpsAppL3.h"
#include "xpsScope.h"

extern int numOfEntry(entryList *list);
//extern uint32_t tnlVirtualId[devId][NUM_OF_TUNNELS];
extern uint32_t tnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t vlanToL3Intf[4096];
extern xpsInterfaceId_t gLagIntfId[XP_MAX_DEVICES][MAX_NO_LAGS];
extern uint32_t l3TnlIntf[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlType[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlVirtualId[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlIsid[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t
multicastVif[XP_MAX_DEVICES][XP_MULTICAST_TYPE_MAX][MAX_MULTICAST_ENTRIES];


//global definition for nh database's "nhContextRoot"
static void *nhContextRoot[XP_MAX_DEVICES] = { NULL };

//compare function for nh database
int compareNhCtxMap(const void *l, const void *r)
{
    const nhContextMap *lm = (nhContextMap *)l;
    const nhContextMap *lr = (nhContextMap *)r;
    return lm->key - lr->key;
}

//get nexthop database
void* xpsAppGetNhContext(xpDevice_t devId, uint32_t nhIndex)
{
    nhContextMap tmpNhCtx;

    tmpNhCtx.key = nhIndex;
    void *r = tfind(&tmpNhCtx, &nhContextRoot[devId], compareNhCtxMap);
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
XP_STATUS xpsAppPopulateNh(xpDevice_t devId, IpxNhEntry* ipvxNhData)
{
    XP_STATUS retVal = XP_NO_ERR;
    void *r = NULL;
    uint32_t intfId;
    int gTnlVif = 0;
    xpsScope_t scopeId;
    retVal = xpsScopeGetScopeId(devId, &scopeId);

    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in xpsScopeGetScopeId : error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    if (xpsAppGetNhContext(devId, ipvxNhData->nhId) == NULL)
    {
        nhContextMap *nhCtx = (nhContextMap *)malloc(sizeof(nhContextMap));
        if (!nhCtx)
        {
            printf("%s:Error: Can't allocate nh context for nh Index: %d\n", __FUNCTION__,
                   ipvxNhData->nhId);
            return XP_ERR_INIT;
        }

        if (ipvxNhData->allocateNhWithId)
        {
            if ((retVal = xpsL3CreateRouteNextHopWithIdScope(scopeId,
                                                             ipvxNhData->nhId)) != XP_NO_ERR)
            {
                printf("%s:Error xpsL3RouteNextHopCreate() error code: %d\n", __FUNCTION__,
                       retVal);
                free(nhCtx);
                return retVal;
            }
            nhCtx->nhIdx = ipvxNhData->nhId;
        }
        else
        {
            if ((retVal = xpsL3CreateRouteNextHopScope(scopeId, 1,
                                                       &nhCtx->nhIdx)) != XP_NO_ERR)
            {
                printf("%s:Error xpsL3RouteNextHopCreate() error code: %d\n", __FUNCTION__,
                       retVal);
                free(nhCtx);
                return retVal;
            }
        }
        printf("Created : NH %d for NH-Index : %d\n", nhCtx->nhIdx, ipvxNhData->nhId);

        memset(&nhCtx->nh, 0, sizeof(xpsL3NextHopEntry_t));
        nhCtx->key = ipvxNhData->nhId;
        nhCtx->nh.pktCmd = (xpPktCmd_e) ipvxNhData->pktCmd;
        nhCtx->nh.propTTL = ipvxNhData->procTTL;
        nhCtx->nh.reasonCode = ipvxNhData->reasonCode;

        nhCtx->nh.nextHop.macDa[0] = ipvxNhData->macAddr[0];
        nhCtx->nh.nextHop.macDa[1] = ipvxNhData->macAddr[1];
        nhCtx->nh.nextHop.macDa[2] = ipvxNhData->macAddr[2];
        nhCtx->nh.nextHop.macDa[3] = ipvxNhData->macAddr[3];
        nhCtx->nh.nextHop.macDa[4] = ipvxNhData->macAddr[4];
        nhCtx->nh.nextHop.macDa[5] = ipvxNhData->macAddr[5];
        nhCtx->nh.nextHop.l3InterfaceId = vlanToL3Intf[ipvxNhData->cTagVid];
        if (nhCtx->nh.pktCmd== XP_PKTCMD_DROP ||
            nhCtx->nh.pktCmd== XP_PKTCMD_TRAP)//pktCmd = Trap to CPU or Drop
        {
            nhCtx->nh.nextHop.egressIntfId = XPS_INTF_INVALID_ID;

        }
        else if (ipvxNhData->isTagged == 0x0 || ipvxNhData->isTagged == 0x1) //Untagged
        {
            if (ipvxNhData->tnlIdx == -2) // Lag Port
            {
                nhCtx->nh.nextHop.egressIntfId = gLagIntfId[devId][ipvxNhData->lagId];
            }
            else
            {
                /* If port is connected to port extender */
                if (ipvxNhData->extendedPortIdx != -1)
                {
                    /* Search for extended group context for perticular port*/
                    void *r = xpsAppGetExtPortsGroup(devId, ipvxNhData->destPort);
                    if (r == NULL)
                    {
                        printf("%s:Error: Can't found extPortGroup ctx in tree for port %d\n",
                               __FUNCTION__, ipvxNhData->destPort);
                        free(nhCtx);
                        return XP_ERR_INVALID_ARG;
                    }

                    /* Check extended port count for group*/
                    if (ipvxNhData->extendedPortIdx >= (*(extPortGroup **)r)->numOfExtendedPorts)
                    {
                        printf("%s:Error: invalid extendedPortIdx : %d for port %d\n", __FUNCTION__,
                               ipvxNhData->extendedPortIdx, ipvxNhData->destPort);
                        free(nhCtx);
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
                        printf("Error : xpsPortGetPortIntfId() Failed for interfac type :%d devId %d\n",
                               XPS_PORT, devId);
                        free(nhCtx);
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
                printf("Error: mplsIdx : %d in not found in allocatedMplsTnlIvif \n",
                       ipvxNhData->mplsIdx);
                free(nhCtx);
                return XP_ERR_INIT;
            }
            nhCtx->nh.nextHop.egressIntfId  = (*(allocatedMplsTnlVifMap **)r)->tnlId;
        }
        else if (ipvxNhData->isTagged == 0x4) //MPLS L3 vpn
        {
            void *r  = getAllocatedMplsTnlVif(devId, ipvxNhData->mplsIdx);
            if (r == NULL)
            {
                printf("Error: mplsIdx : %d in not found in allocatedMplsTnlIvif \n",
                       ipvxNhData->mplsIdx);
                free(nhCtx);
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


        /*isTagged ==> encapType ==> nhType -- translation from config to parsing to fl*/
        else if (ipvxNhData->isTagged == XP_L3_ENCAP_SRV6_TUNNEL)
        {

            //uint32_t srEntryNum = ipvxNhData->tnlIdx - 71; //#define SR_START_INDEX 71
            //SrhEntry *srhEntry = (SrhEntry *)getEntry(&configFile.srhData, srEntryNum);

            nhCtx->nh.nextHop.egressIntfId = tnlVif[devId][ipvxNhData->tnlIdx ];
            //nhCtx->nh.nextHop.l3InterfaceId = ipvxNhData->cTagVid;
            //nhCtx->encapType = XP_L3_ENCAP_SRV6_TUNNEL;
#if 0
            nhCtx->nh.nextHop.srV6Tnnl.ins3Ptr = 0; //SR TODO

            if (srhEntry->numSegment != 0)
            {
                /*uint32_t ins3Id = 28950;
                retVal = xpAllocateInsPtrWithId(XP_INSERTION3, ins3Id);
                if(retVal)
                {
                    printf("%s:Error: ins3 allocation for srId failed for SRH.\n", __FUNCTION__);
                    return retVal;
                }
                printf("ins3Id for SRH = %u.\n", ins3Id);
                nhCtx->nh.nextHopData.srV6Tnnl.ins3Ptr = ins3Id; *///SR TODO
                nhCtx->nh.nextHopData.srV6Tnnl.ins3Ptr = srhEntry->ins3Id;

            }


            memcpy(&(nhCtx->nh.nextHopData.srV6Tnnl.seg3Msbs), srhEntry->ip2,
                   sizeof(uint16_t));
#endif


        }


        printf("pktCmd %d \n", nhCtx->nh.pktCmd);
        printf("reasonCode %d \n", nhCtx->nh.reasonCode);
        printf("MAC Addr 0x%2x:0x%2x:0x%2x:0x%2x:0x%2x:0x%2x \n",
               nhCtx->nh.nextHop.macDa[5], nhCtx->nh.nextHop.macDa[4],
               nhCtx->nh.nextHop.macDa[3], nhCtx->nh.nextHop.macDa[2],
               nhCtx->nh.nextHop.macDa[1], nhCtx->nh.nextHop.macDa[0]);
        printf("egressVif %d \n", nhCtx->nh.nextHop.egressIntfId);

        r = tsearch(nhCtx, &nhContextRoot[devId], compareNhCtxMap);
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
                printf("%s:Error in xpsL3SetRouteVpnNextHop an entry: error code: %d\n",
                       __FUNCTION__, retVal);
                tdelete(nhCtx, &nhContextRoot[devId], compareNhCtxMap);
                free(nhCtx);
                return retVal;
            }
        }
        else
        {
            if ((retVal = xpsL3SetRouteNextHop(devId, nhCtx->nhIdx,
                                               &nhCtx->nh)) != XP_NO_ERR)
            {
                printf("%s:Error in xpsL3SetRouteNextHop an entry: error code: %d\n",
                       __FUNCTION__, retVal);
                tdelete(nhCtx, &nhContextRoot[devId], compareNhCtxMap);
                free(nhCtx);
                return retVal;
            }
        }
    }
    else
    {
        printf("%s:Info: NH context for nhId %d is already allocated\n", __FUNCTION__,
               ipvxNhData->nhId);
    }
    return retVal;
}

//
// XpsApp Utility function to write Ipvx NH Scope Table entries
//
XP_STATUS xpsAppPopulateNhScope(xpDevice_t devId,
                                IpxNhScopeEntry* ipvxNhScopeData)
{
    XP_STATUS retVal = XP_NO_ERR;
    IpxNhEntry ipvxNhData;
    xpsScope_t scopeId;

    ipvxNhData.macAddr[0] = ipvxNhScopeData->macAddr[0];
    ipvxNhData.macAddr[1] = ipvxNhScopeData->macAddr[1];
    ipvxNhData.macAddr[2] = ipvxNhScopeData->macAddr[2];
    ipvxNhData.macAddr[3] = ipvxNhScopeData->macAddr[3];
    ipvxNhData.macAddr[4] = ipvxNhScopeData->macAddr[4];
    ipvxNhData.macAddr[5] = ipvxNhScopeData->macAddr[5];
    ipvxNhData.destPort = ipvxNhScopeData->destPort;
    ipvxNhData.nhId = ipvxNhScopeData->nhId;
    ipvxNhData.cTagVid = ipvxNhScopeData->cTagVid;
    ipvxNhData.virtualId = ipvxNhScopeData->virtualId;
    ipvxNhData.isTagged = ipvxNhScopeData->isTagged;
    ipvxNhData.tnlIdx = ipvxNhScopeData->tnlIdx;
    ipvxNhData.mplsIdx = ipvxNhScopeData->mplsIdx;
    ipvxNhData.geneveTunnelType = ipvxNhScopeData->geneveTunnelType;
    ipvxNhData.pktCmd = ipvxNhScopeData->pktCmd;
    ipvxNhData.procTTL = ipvxNhScopeData->procTTL;
    ipvxNhData.nhType = ipvxNhScopeData->nhType;
    ipvxNhData.lagId = ipvxNhScopeData->lagId;
    ipvxNhData.extendedPortIdx = ipvxNhScopeData->extendedPortIdx;
    ipvxNhData.reasonCode = ipvxNhScopeData->reasonCode;

    retVal = xpsScopeGetScopeId(devId, &scopeId);

    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in xpsScopeGetScopeId : error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    if (scopeId == ipvxNhScopeData->scopeId)
    {
        retVal = xpsAppPopulateNh(devId, &ipvxNhData);
    }
    return retVal;
}

//IPv6 Host Table Configuration

XP_STATUS xpsAppWriteIpv6HostEntry(xpDevice_t devId,
                                   Ipv6HostEntry* ipv6HostData)
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

    pL3HostEntry.vrfId = ipv6HostData->vrf;

    printf("IpAddress %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x, VRF %d \n",
           pL3HostEntry.ipv6Addr[15], pL3HostEntry.ipv6Addr[14], pL3HostEntry.ipv6Addr[13],
           pL3HostEntry.ipv6Addr[12], pL3HostEntry.ipv6Addr[11], pL3HostEntry.ipv6Addr[10],
           pL3HostEntry.ipv6Addr[9], pL3HostEntry.ipv6Addr[8], pL3HostEntry.ipv6Addr[7],
           pL3HostEntry.ipv6Addr[6], pL3HostEntry.ipv6Addr[5], pL3HostEntry.ipv6Addr[4],
           pL3HostEntry.ipv6Addr[3], pL3HostEntry.ipv6Addr[2], pL3HostEntry.ipv6Addr[1],
           pL3HostEntry.ipv6Addr[0], pL3HostEntry.vrfId);

    void *r  = xpsAppGetNhContext(devId, ipv6HostData->nhId);
    if (r == NULL)
    {
        printf("Error: nhId : %d in not found in nhContextMap \n", ipv6HostData->nhId);
        return XP_ERR_INIT;
    }
    pL3HostEntry.nhEntry = (*(nhContextMap **)r)->nh;

    if ((retVal = xpsL3AddIpHostEntry(devId, &pL3HostEntry,
                                      &indexList)) != XP_NO_ERR)
    {
        printf("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
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
    pL3HostEntryLkp.vrfId = ipv6HostData->vrf;

    if ((retVal = xpsL3GetIpHostEntry(devId, &pL3HostEntryLkp)) != XP_NO_ERR)
    {
        printf("Error in ipv6Host table lookup: error code: %d\n", retVal);
        return retVal;
    }
    printf("Lookup: ipv6Host lookup: pktCmd = %d  egressVif = %d \n",
           pL3HostEntryLkp.nhEntry.pktCmd, pL3HostEntryLkp.nhEntry.nextHop.egressIntfId);
    return XP_NO_ERR;
}

XP_STATUS xpsAppWriteIpv6HostControl(xpDevice_t devId,
                                     Ipv6HostControlEntry *ipv6HostControlData)
{
    XP_STATUS retVal;
    uint32_t index = 0;
    xpsHashIndexList_t indexList;
    uint32_t reasonCode;
    xpsL3HostEntry_t pL3HostEntry;

    if (!ipv6HostControlData)
    {
        printf("Null pointer received while adding Ipv6 Host Control Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    memset(&pL3HostEntry, 0, sizeof(xpsL3HostEntry_t));
    pL3HostEntry.vrfId = ipv6HostControlData->vrf;
    COPY_IPV6_ADDR_T(pL3HostEntry.ipv6Addr, ipv6HostControlData->ipv6Addr);
    pL3HostEntry.type = XP_PREFIX_TYPE_IPV6;
    pL3HostEntry.nhEntry.reasonCode = ipv6HostControlData->reasonCode;

    if ((retVal = xpsL3AddIpHostControlEntry(devId, &pL3HostEntry,
                                             &indexList))!= XP_NO_ERR)
    {
        printf("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    index = indexList.index[0];

    printf("IpAddress %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x, VRF : %d, ReasonCode : %d\n",
           ipv6HostControlData->ipv6Addr[15], ipv6HostControlData->ipv6Addr[14],
           ipv6HostControlData->ipv6Addr[13], ipv6HostControlData->ipv6Addr[12],
           ipv6HostControlData->ipv6Addr[11], ipv6HostControlData->ipv6Addr[10],
           ipv6HostControlData->ipv6Addr[9], ipv6HostControlData->ipv6Addr[8],
           ipv6HostControlData->ipv6Addr[7], ipv6HostControlData->ipv6Addr[6],
           ipv6HostControlData->ipv6Addr[5], ipv6HostControlData->ipv6Addr[4],
           ipv6HostControlData->ipv6Addr[3], ipv6HostControlData->ipv6Addr[2],
           ipv6HostControlData->ipv6Addr[1], ipv6HostControlData->ipv6Addr[0],
           ipv6HostControlData->vrf, ipv6HostControlData->reasonCode);
    printf("xpsAppWriteIpv6HostControl : Pass index:%d \n", index);

    printf("Perform Ipv6HostControlEntry  lookup for the same entry\n");

    if ((retVal = xpsL3GetIpHostControlEntryReasonCode(devId, &pL3HostEntry,
                                                       &reasonCode)) != XP_NO_ERR)
    {
        printf("Error in ipv6HostControl table lookup: error code: %d\n", retVal);
        return retVal;
    }
    printf("Lookup: ipv6HostControl lookup: IpAddress %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x, VRF = %d, reasonCode = %d \n",
           ipv6HostControlData->ipv6Addr[15], ipv6HostControlData->ipv6Addr[14],
           ipv6HostControlData->ipv6Addr[13], ipv6HostControlData->ipv6Addr[12],
           ipv6HostControlData->ipv6Addr[11], ipv6HostControlData->ipv6Addr[10],
           ipv6HostControlData->ipv6Addr[9], ipv6HostControlData->ipv6Addr[8],
           ipv6HostControlData->ipv6Addr[7], ipv6HostControlData->ipv6Addr[6],
           ipv6HostControlData->ipv6Addr[5], ipv6HostControlData->ipv6Addr[4],
           ipv6HostControlData->ipv6Addr[3], ipv6HostControlData->ipv6Addr[2],
           ipv6HostControlData->ipv6Addr[1], ipv6HostControlData->ipv6Addr[0],
           ipv6HostControlData->vrf, reasonCode);

    return XP_NO_ERR;
}

///IPv6 Route

XP_STATUS xpsAppWriteIpv6RouteEntry(xpDevice_t devId,
                                    Ipv6RouteEntry* ipv6RouteData)
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
    pL3RouteEntry.vrfId = ipv6RouteData->vrf;
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
        void *r  = xpsAppGetNhContext(devId, nhIdx);
        if (r == NULL)
        {
            printf("Error: nhId : %d in not found in nhContextMap \n", nhIdx);
            return XP_ERR_INIT;
        }

        printf("NetMask: %d | EcmpSize: %d \n", pL3RouteEntry.ipMaskLen,
               pL3RouteEntry.nhEcmpSize);

        if (i == 0)
        {
            pL3RouteEntry.nhId = (*(nhContextMap **)r)->nhIdx;
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
    pL3RouteEntryLkp.vrfId = ipv6RouteData->vrf;
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

XP_STATUS xpsAppWriteIpv4HostEntry(xpDevice_t devId,
                                   Ipv4HostEntry* ipv4HostData)
{
    XP_STATUS retVal;
    xpsL3HostEntry_t pL3HostEntry;
    xpsL3HostEntry_t pL3HostEntryLkp;
    xpsHashIndexList_t indexList;

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
    pL3HostEntry.vrfId = ipv4HostData->vrf;

    printf("IpAddress %d.%d.%d.%d, VRF %d \n", pL3HostEntry.ipv4Addr[3],
           pL3HostEntry.ipv4Addr[2], pL3HostEntry.ipv4Addr[1], pL3HostEntry.ipv4Addr[0],
           pL3HostEntry.vrfId);

    void *r  = xpsAppGetNhContext(devId, ipv4HostData->nhId);
    if (r == NULL)
    {
        printf("Error: nhId : %d in not found in nhContextMap \n", ipv4HostData->nhId);
        return XP_ERR_INIT;
    }
    pL3HostEntry.nhEntry = (*(nhContextMap **)r)->nh;

    if ((retVal = xpsL3AddIpHostEntry(devId, &pL3HostEntry,
                                      &indexList)) != XP_NO_ERR)
    {
        printf("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    printf("writeIpv4HostEntry: Pass\n");

    printf("Perform Ipv4HostEntry  lookup for the same entry\n");

    pL3HostEntryLkp.ipv4Addr[0] = ipv4HostData->ipv4Addr[0];
    pL3HostEntryLkp.ipv4Addr[1] = ipv4HostData->ipv4Addr[1];
    pL3HostEntryLkp.ipv4Addr[2] = ipv4HostData->ipv4Addr[2];
    pL3HostEntryLkp.ipv4Addr[3] = ipv4HostData->ipv4Addr[3];
    pL3HostEntryLkp.vrfId = ipv4HostData->vrf;

    if ((retVal = xpsL3GetIpHostEntry(devId, &pL3HostEntryLkp)) != XP_NO_ERR)
    {
        printf("Error in ipv4Host table lookup: error code: %d\n", retVal);
        return retVal;
    }
    printf("Lookup: ipv4Host lookup: pktCmd = %d  egressVif = %d \n",
           pL3HostEntryLkp.nhEntry.pktCmd, pL3HostEntryLkp.nhEntry.nextHop.egressIntfId);

    return XP_NO_ERR;
}

// XpApp Utility function to write IPv4 Host Scope Table entries

XP_STATUS xpsAppWriteIpv4HostScopeEntry(xpDevice_t devId,
                                        Ipv4HostScopeEntry* ipv4HostScopeData)
{
    XP_STATUS retVal = XP_NO_ERR;
    Ipv4HostEntry ipv4HostData;
    xpsScope_t scopeId;

    ipv4HostData.ipv4Addr[0] = ipv4HostScopeData->ipv4Addr[0];
    ipv4HostData.ipv4Addr[1] = ipv4HostScopeData->ipv4Addr[1];
    ipv4HostData.ipv4Addr[2] = ipv4HostScopeData->ipv4Addr[2];
    ipv4HostData.ipv4Addr[3] = ipv4HostScopeData->ipv4Addr[3];
    ipv4HostData.vrf = ipv4HostScopeData->vrf;
    ipv4HostData.nhId = ipv4HostScopeData->nhId;

    retVal = xpsScopeGetScopeId(devId, &scopeId);

    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in xpsScopeGetScopeId : error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    if (scopeId == ipv4HostScopeData->scopeId)
    {
        retVal = xpsAppWriteIpv4HostEntry(devId, &ipv4HostData);
    }
    return retVal;
}

XP_STATUS xpsAppWriteIpv4HostControl(xpDevice_t devId,
                                     Ipv4HostControlEntry *ipv4HostControlData)
{
    XP_STATUS retVal;
    uint32_t index = 0;
    xpsHashIndexList_t indexList;
    uint32_t reasonCode;
    xpsL3HostEntry_t pL3HostEntry;

    if (!ipv4HostControlData)
    {
        printf("Null pointer received while adding Ipv4 Host Control Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    memset(&pL3HostEntry, 0, sizeof(xpsL3HostEntry_t));
    pL3HostEntry.vrfId = ipv4HostControlData->vrf;
    COPY_IPV4_ADDR_T(pL3HostEntry.ipv4Addr, ipv4HostControlData->ipv4Addr);
    pL3HostEntry.type = XP_PREFIX_TYPE_IPV4;
    pL3HostEntry.nhEntry.reasonCode = ipv4HostControlData->reasonCode;

    if ((retVal = xpsL3AddIpHostControlEntry(devId, &pL3HostEntry,
                                             &indexList))!= XP_NO_ERR)
    {
        printf("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    index = indexList.index[0];

    printf("IpAddress %d.%d.%d.%d, VRF : %d, ReasonCode : %d\n",
           ipv4HostControlData->ipv4Addr[3], ipv4HostControlData->ipv4Addr[2],
           ipv4HostControlData->ipv4Addr[1], ipv4HostControlData->ipv4Addr[0],
           ipv4HostControlData->vrf, ipv4HostControlData->reasonCode);
    printf("xpsAppWriteIpv4HostControl : Pass index:%d \n", index);

    printf("Perform Ipv4HostControlEntry  lookup for the same entry\n");

    if ((retVal = xpsL3GetIpHostControlEntryReasonCode(devId, &pL3HostEntry,
                                                       &reasonCode)) != XP_NO_ERR)
    {
        printf("Error in ipv4HostControl table lookup: error code: %d\n", retVal);
        return retVal;
    }
    printf("Lookup: ipv4HostControl lookup: IpAddress %d.%d.%d.%d, VRF = %d, reasonCode = %d \n",
           ipv4HostControlData->ipv4Addr[3], ipv4HostControlData->ipv4Addr[2],
           ipv4HostControlData->ipv4Addr[1], ipv4HostControlData->ipv4Addr[0],
           ipv4HostControlData->vrf, reasonCode);

    return XP_NO_ERR;
}

XP_STATUS xpsAppWriteIpv4RouteEntry(xpDevice_t devId,
                                    Ipv4RouteEntry* ipv4RouteData)
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
    pL3RouteEntry.vrfId = ipv4RouteData->vrf;
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
        void *r  = xpsAppGetNhContext(devId, nhIdx);
        if (r == NULL)
        {
            printf("Error: nhId : %d in not found in nhContextMap \n", nhIdx);
            return XP_ERR_INIT;
        }

        printf("NetMask: %d | EcmpSize: %d \n", pL3RouteEntry.ipMaskLen,
               pL3RouteEntry.nhEcmpSize);

        if (i == 0)
        {
            pL3RouteEntry.nhId = (*(nhContextMap **)r)->nhIdx;
        }
    }

    if ((retVal = xpsL3AddIpRouteEntry(devId, &pL3RouteEntry,
                                       &prefixIndex, NULL)) != XP_NO_ERR)
    {
        printf("%s:Error in inserting an entry: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    printf("Perform Ipv4Route lookup for the same entry\n");

    pL3RouteEntryLkp.ipv4Addr[0] = ipv4RouteData->ipv4Addr[0];
    pL3RouteEntryLkp.ipv4Addr[1] = ipv4RouteData->ipv4Addr[1];
    pL3RouteEntryLkp.ipv4Addr[2] = ipv4RouteData->ipv4Addr[2];
    pL3RouteEntryLkp.ipv4Addr[3] = ipv4RouteData->ipv4Addr[3];
    pL3RouteEntryLkp.vrfId = ipv4RouteData->vrf;
    pL3RouteEntryLkp.ipMaskLen =ipv4RouteData->netMask;

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
        printf("Lookup: ipv4Host lookup: pktCmd = %d  reasonCode = %d egressVif = %d \n",
               pL3NextHopEntryLkp.pktCmd, pL3NextHopEntryLkp.reasonCode,
               pL3NextHopEntryLkp.nextHop.egressIntfId);
        pL3RouteEntry.nhId++;
    }
    return XP_NO_ERR;
}

XP_STATUS xpsAppWriteIpv4RouteScopeEntry(xpDevice_t devId,
                                         Ipv4RouteScopeEntry* ipv4RouteScopeData)
{
    XP_STATUS retVal = XP_NO_ERR;
    Ipv4RouteEntry ipv4RouteData;
    xpsScope_t scopeId;

    ipv4RouteData.ipv4Addr[0] = ipv4RouteScopeData->ipv4Addr[0];
    ipv4RouteData.ipv4Addr[1] = ipv4RouteScopeData->ipv4Addr[1];
    ipv4RouteData.ipv4Addr[2] = ipv4RouteScopeData->ipv4Addr[2];
    ipv4RouteData.ipv4Addr[3] = ipv4RouteScopeData->ipv4Addr[3];
    ipv4RouteData.vrf = ipv4RouteScopeData->vrf;
    ipv4RouteData.nhId = ipv4RouteScopeData->nhId;
    ipv4RouteData.netMask = ipv4RouteScopeData->netMask;
    ipv4RouteData.ecmpSize = ipv4RouteScopeData->ecmpSize;

    retVal = xpsScopeGetScopeId(devId, &scopeId);

    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in xpsScopeGetScopeId : error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    if (scopeId == ipv4RouteScopeData->scopeId)
    {
        retVal = xpsAppWriteIpv4RouteEntry(devId, &ipv4RouteData);
    }
    return retVal;
}


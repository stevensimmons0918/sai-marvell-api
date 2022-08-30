// xpSaiApp.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

//xpSaiApp and IPC infrastructure
#include "xpSaiApp.h"
#include "xpSaiUtil.h"

#include "xpSaiDev.h"

#include "xpsGlobalSwitchControl.h"
//#include "xpCtrlMac.h"
#include "xpSaiAppL2.h"
#include "xpSaiAppL3.h"
#include "xpSaiAppTnl.h"
#include "xpSaiAppMpls.h"
#include "xpSaiAppSaiUtils.h"
#include "xpSaiAppQos.h"

#include "assert.h"

#ifndef TBD  /*This is exposing SAI internal logic to XPS APP.Should be removed after redesign XPS APP to USE saiports.h */
#include "xpSai.h"
#endif  /*TBD*/

#include "xpAppConfig.h"
#include "xpPyInc.h"


XP_SAI_LOG_REGISTER_API(SAI_API_UNSPECIFIED);

#define GET_INDEX_FOR_KEY_ENTRY_FORMAT(enumVal, keyFldLst, keyEntryFormatIndex) \
{ \
    keyEntryFormatIndex = 0; \
    int loopIdx; \
    for(loopIdx = 0; loopIdx <= enumVal; loopIdx++) \
        if(keyFldLst[loopIdx] == 1) \
            keyEntryFormatIndex++; \
    keyEntryFormatIndex--; \
}

#define XP_SAI_VRF_DB "./vrfDb.txt"

//TODO: Need to cleanthis once sal is in place
#define xpFreeBuffer(buf) free(buf)
#define xpGetBuffer malloc(XP_MAX_PACKET_SIZE);
extern xpsInterfaceId_t gLagIntfId[MAX_NO_LAGS];

extern uint32_t tnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t vlanToL3Intf[4096];
extern uint32_t l3TnlIntf[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlType[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlVirtualId[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlIsid[XP_MAX_DEVICES][NUM_OF_TUNNELS];

uint32_t vrfIdxToVrf[MAX_VRF_IDX] = {0};
int32_t mvifToIfListIdMap[128];
static uint64_t
nhIdIp4[256]; //array to store nhIds created by NextHop Call, latter will use in SaiRoute call for reference
static uint64_t nhGrpIdIp4[MAX_NEXT_HOP_GROUP];

XP_STATUS xpPortSetPvidModeAllPkts(sai_object_id_t switchOid, uint32_t portId,
                                   uint8_t set);
XP_STATUS saiStatus2XpsStatus(sai_status_t status)
{
    if (status != SAI_STATUS_SUCCESS)
    {
        //TODO: Place holder.
        return XP_STATUS_NUM_OF_ENTRIES;
    }
    return XP_NO_ERR;
}

XP_STATUS xpAppDeInit(void)
{
    return xpsSdkDeInit();
}

XP_STATUS xpAppDeInitDevice(xpsDevice_t devId)
{
    XP_STATUS status = XP_NO_ERR;
    printf("xpAppDeInitDevice(devId = 0x%x)\n", devId);

    //xps specific removal of a device

    // TODO: What is the need of this function to be implemented in xpSaiApp
    //xpSaiSwitchApi->remove_switch(switchOid);

    printf("xpAppDeInitDevice(devId = 0x%x) done.\n", devId);

    status = xpAppCleanup(devId);
    if (status)
    {
        printf("Error: xpInitMgr removeDevice failed.\n");
    }

    return status;
}

XP_STATUS xpSaiAppConfigureVrf(xpsDevice_t devId, sai_object_id_t switchOid,
                               VrfEntry* vrfEntry)
{
    sai_attribute_t vrfV4AttrList, vrfV6AttrList;
    uint32_t vrfIdx = vrfEntry->vrfIdx;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    sai_object_id_t vrOid = 0;
    uint32_t        vrfId = 0;

    vrfV4AttrList.id = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE;
    memcpy(&(vrfV4AttrList.value), &(vrfEntry->v4RouteEn),
           sizeof(vrfEntry->v4RouteEn));
    vrfV6AttrList.id = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE;
    memcpy(&(vrfV6AttrList.value), &(vrfEntry->v6RouteEn),
           sizeof(vrfEntry->v6RouteEn));

    saiRetVal = xpSaiVrfApi->create_virtual_router(&vrOid, switchOid, 1,
                                                   &vrfV4AttrList);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI create_virtual_router() Failed Error : %d for devId %d vrfIdx %d \n",
               saiRetVal, devId, vrfIdx);
        return saiStatus2XpsStatus(saiRetVal);
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(vrOid, SAI_OBJECT_TYPE_VIRTUAL_ROUTER))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(vrOid));
        return XP_STATUS_NUM_OF_ENTRIES;
    }

    vrfId = (uint32_t)xpSaiObjIdValueGet(vrOid);

    vrfIdxToVrf[vrfIdx] = vrfId;

    printf("SAI Success : create_virtual_router() devId %d vrfIdx %d allocatedVrf %d\n",
           devId, vrfIdx, vrfId);

    saiRetVal = xpSaiVrfApi->set_virtual_router_attribute(vrOid, &vrfV4AttrList);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI set_virtual_router_attribute() Failed for V4 Error : %d for devId %d vrfId %d \n",
               saiRetVal, devId, vrfId);
        return saiStatus2XpsStatus(saiRetVal);
    }
    printf("SAI Success : set_virtual_router_attribute() for V4 devId %d vrfId %d \n",
           devId, vrfId);

    saiRetVal = xpSaiVrfApi->set_virtual_router_attribute(vrOid, &vrfV6AttrList);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI set_virtual_router_attribute() Failed for V6 Error : %d for devId %d vrfId %d \n",
               saiRetVal, devId, vrfId);
        return saiStatus2XpsStatus(saiRetVal);
    }
    printf("SAI Success : set_virtual_router_attribute() for V6 devId %d vrfId %d \n",
           devId, vrfId);

    return XP_NO_ERR;
}

// Populate Feature Tables for various features
XP_STATUS xpSaiAppPopulateTables(xpsDevice_t devId, void *arg)
{
    XP_STATUS err = XP_NO_ERR;
    uint32_t ipv4BridgeMcHandle = 0;
    uint32_t ipv6BridgeMcHandle = 0;
    uint32_t ipv4RouteMcHandle = 0;
    uint32_t ipv6RouteMcHandle = 0;
    uint32_t ipv4PimBiDirHandle = 0;
    uint32_t entryNum = 0;
    uint32_t numOfFdbEntry = 0;
    uint32_t numOfNhEntry = 0;
    uint32_t numOfVlanEntry = 0;
    uint32_t numOfPortConfigEntry = 0;
    uint32_t numOfLagEntry = 0;
    uint32_t numOfPBBTnlEntry = 0;
    uint32_t numOfMplsTnlEntry = 0;
    uint32_t numOfMplsLabelEntry = 0;
    uint32_t numOfIpv4HostEntry = 0;
    uint32_t numOfIpv4RouteEntry = 0;
    uint32_t numOfControlMacEntry = 0;
    uint32_t numOfIpv6HostEntry = 0;
    uint32_t numOfIpv6RouteEntry = 0;
    uint32_t numOfTnlEntry = 0;
    uint32_t numOfIpv4BridgeMcEntry = 0;
    uint32_t numOfIpv6BridgeMcEntry = 0;
    uint32_t numOfIpv4RouteMcEntry = 0;
    uint32_t numOfIpv6RouteMcEntry = 0;
    uint32_t numOfIpv4PimBiDirEntry = 0;
    uint32_t numOfL3OIFEntry = 0;
    uint32_t numOfMvifEntry = 0;
    uint32_t numOfTenantEntry = 0;
    uint32_t numOfVrfEntry = 0;
    uint32_t numOfSaiNeighborEntry = 0;
    uint32_t numOfSaiNhEntry = 0;
    uint32_t numOfSaiRouterEntry = 0;
    uint32_t numOfSaiIpv6RouterEntry = 0;
    uint32_t numOfSaiIpv6NeighborEntry = 0;
    uint32_t numOfSaiIpv6NhEntry = 0;
    uint32_t numOfSchedulerQueueEntry = 0;
    uint32_t numOfShaperPortEntry = 0;
    uint32_t numOfWredEntry = 0;
    uint32_t numOfIngressQosMapEntry = 0;
    uint32_t numOfEgressQosMapEntry = 0;
    uint32_t numOfPolicerEntry = 0;
    uint32_t numOfSaiNhGroupEntry = 0;
    xpInitType_t initType = INIT_COLD;
    FILE *fPtr = NULL;
    FILE *fCheck = NULL;
    int retCheck = 0;
    uint32_t configVrfId = 0;
    uint32_t allocatedVrfId = 0;
    char buf[20] = {0};
    uint32_t index = 0;
    sai_object_id_t switchOid = 0;

    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_SWITCH, 0, 0,
                         &switchOid) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI switch object could not be created.\n");
        return saiStatus2XpsStatus(SAI_STATUS_FAILURE);
    }

    if (arg)
    {
        xpSaiWmIpcParam_t *param = (xpSaiWmIpcParam_t *)arg;
        initType = param->initType;
    }

    printf("Initializing LPM Route Manager Infrastructure \n");

    numOfFdbEntry = numOfEntry(&configFile.macData);
    numOfNhEntry = numOfEntry(&configFile.ipvxNhData);
    numOfVlanEntry = numOfEntry(&configFile.vlanData);
    numOfTenantEntry = numOfEntry(&configFile.vlanTenantIdData);
    numOfPortConfigEntry = numOfEntry(&configFile.portConfigData);
    numOfLagEntry = numOfEntry(&configFile.portLagData);
    numOfTnlEntry = numOfEntry(&configFile.tunnelData);
    numOfPBBTnlEntry = numOfEntry(&configFile.tunnelPBBData);
    numOfMplsTnlEntry = numOfEntry(&configFile.tunnelMplsData);
    numOfMplsLabelEntry = numOfEntry(&configFile.mplsLabelData);
    numOfIpv4HostEntry = numOfEntry(&configFile.ipv4HostData);
    numOfIpv4RouteEntry = numOfEntry(&configFile.ipv4RouteData);
    numOfControlMacEntry = numOfEntry(&configFile.controlMacData);
    numOfIpv6HostEntry = numOfEntry(&configFile.ipv6HostData);
    numOfIpv6RouteEntry = numOfEntry(&configFile.ipv6RouteData);
    numOfIpv4BridgeMcEntry = numOfEntry(&configFile.ipv4BridgeMcData);
    numOfIpv6BridgeMcEntry = numOfEntry(&configFile.ipv6BridgeMcData);
    numOfIpv4RouteMcEntry = numOfEntry(&configFile.ipv4RouteMcData);
    numOfIpv6RouteMcEntry = numOfEntry(&configFile.ipv6RouteMcData);
    numOfIpv4PimBiDirEntry = numOfEntry(&configFile.ipv4PimBiDirData);
    numOfL3OIFEntry = numOfEntry(&configFile.l3OIFEntry);
    numOfMvifEntry = numOfEntry(&configFile.mvifData);
    numOfVrfEntry = numOfEntry(&configFile.vrfData);
    numOfSaiNeighborEntry = numOfEntry(&configFile.ipv4SaiNeighborData);
    numOfSaiNhEntry =  numOfEntry(&configFile.ipv4SaiNhData);
    numOfSaiRouterEntry = numOfEntry(&configFile.ipv4SaiRouteData);
    numOfSaiIpv6NeighborEntry = numOfEntry(&configFile.ipv6SaiNeighborData);
    numOfSaiIpv6NhEntry =  numOfEntry(&configFile.ipv6SaiNhData);
    numOfSaiIpv6RouterEntry = numOfEntry(&configFile.ipv6SaiRouteData);
    numOfSchedulerQueueEntry = numOfEntry(&configFile.saiSchedulerData);
    numOfShaperPortEntry = numOfEntry(&configFile.qosShaperPortData);
    numOfWredEntry = numOfEntry(&configFile.qosAqmQProfileData);
    numOfIngressQosMapEntry = numOfEntry(&configFile.qosIngressMapData);
    numOfEgressQosMapEntry = numOfEntry(&configFile.qosEgressMapData);
    numOfPolicerEntry = numOfEntry(&configFile.policerData);
    numOfSaiNhGroupEntry = numOfEntry(&configFile.ipSaiNhGroupData);

    printf("numOfPortConfigEntry = %d \n", numOfPortConfigEntry);
    printf("numOfIpv4HostEntry = %d \n", numOfIpv4HostEntry);
    printf("numOfIpv4RouteEntry = %d \n", numOfIpv4RouteEntry);
    printf("numOfControlMacEntry = %d \n", numOfControlMacEntry);
    printf("numFdbEntry = %d \n", numOfFdbEntry);
    printf("numOfVlanEntry = %d \n", numOfVlanEntry);
    printf("numOfTenantEntry = %d \n", numOfTenantEntry);
    printf("numOfLagEntry = %d \n", numOfLagEntry);
    printf("numOfPBBTnlEntry = %d \n", numOfPBBTnlEntry);
    printf("numOfMplsTnlEntry = %d \n", numOfMplsTnlEntry);
    printf("numOfMplsLabelEntry = %d \n", numOfMplsLabelEntry);
    printf("numOfIpv6HostEntry = %d \n", numOfIpv6HostEntry);
    printf("numOfIpv6RouteEntry = %d \n", numOfIpv6RouteEntry);
    printf("numOfIpv4BridgeMcEntry = %d \n", numOfIpv4BridgeMcEntry);
    printf("numOfIpv6BridgeMcEntry = %d \n", numOfIpv6BridgeMcEntry);
    printf("numOfMvifEntry = %d \n", numOfMvifEntry);
    printf("numOfIpv4RouteMcEntry = %d \n", numOfIpv4RouteMcEntry);
    printf("numOfIpv6RouteMcEntry = %d \n", numOfIpv6RouteMcEntry);
    printf("numOfIpv4PimBiDirEntry = %d \n", numOfIpv4PimBiDirEntry);
    printf("numOfL3OIFEntry = %d \n", numOfL3OIFEntry);
    printf("numOfMvifEntry = %d \n", numOfMvifEntry);
    printf("numOfSaiNeighborEntry = %d \n", numOfSaiNeighborEntry);
    printf("numOfSaiNhEntry = %d \n", numOfSaiNhEntry);
    printf("numOfSaiRouterEntry = %d \n", numOfSaiRouterEntry);
    printf("numOfSaiIpv6NeighborEntry= %d \n", numOfSaiIpv6NeighborEntry);
    printf("numOfSaiIpv6NhEntry = %d \n", numOfSaiIpv6NhEntry);
    printf("numOfSaiIpv6RouterEntry = %d \n", numOfSaiIpv6RouterEntry);
    printf("numOfVrfEntry = %d \n", numOfVrfEntry);
    printf("numOfSchedulerQueueEntry = %d \n", numOfSchedulerQueueEntry);
    printf("numOfShaperPortEntry = %d \n", numOfShaperPortEntry);
    printf("numOfWredEntry = %d \n", numOfWredEntry);
    printf("numOfIngressQosMapEntry = %d \n", numOfIngressQosMapEntry);
    printf("numOfEgressQosMapEntry = %d \n", numOfEgressQosMapEntry);
    printf("numOfPolicerEntry = %d \n", numOfPolicerEntry);
    printf("numOfSaiNhGroupEntry = %d \n", numOfSaiNhGroupEntry);

    if (arg)
    {
        xpSaiWmIpcParam_t *param = (xpSaiWmIpcParam_t *)arg;
        switchOid = param->xpSaiSwitchObjId;
    }


    /* if no config file, then return */
    if (configFile.filename == NULL)
    {
        return XP_NO_ERR;
    }
    //
    // Populate VRF Table
    //
    for (entryNum = 0; entryNum < numOfVrfEntry; entryNum++)
    {
        VrfEntry* vrfEntry = (VrfEntry*)getEntry(&configFile.vrfData, entryNum);
        err = xpSaiAppConfigureVrf(devId, switchOid, vrfEntry);
        if (err < 0)
        {
            printf("Error: programming Vrf data through xpAppConfigureVrf, error code = %d\n",
                   err);
            return err;
        }
    }
    if (numOfVrfEntry > 0)
    {
        if (initType == INIT_COLD)
        {
            /*Check File is available or not*/
            fCheck = fopen(XP_SAI_VRF_DB, "r");
            if (fCheck)
            {
                fclose(fCheck);
                retCheck = remove(XP_SAI_VRF_DB);
                if (retCheck != 0)
                {
                    return XP_ERR_NOT_SUPPORTED;
                }
            }
        }
        fPtr = fopen(XP_SAI_VRF_DB, "w+");
        if (!fPtr)
        {
            printf("Error while accessing Vrf DB\n");
            return XP_ERR_FILE_OPEN;
        }

        fseek(fPtr, 0, SEEK_END);
        for (index = 0; index < MAX_VRF_IDX; index++)
        {
            if (vrfIdxToVrf[index] > 0)
            {
                fprintf(fPtr, "%d-%d\n", index, vrfIdxToVrf[index]);
            }
        }
        if (fPtr)
        {
            fclose(fPtr);
        }
    }
    if (initType == INIT_WARM)
    {
        fPtr = fopen(XP_SAI_VRF_DB, "r");
        if (!fPtr)
        {
            printf("Error while accessing Vrf DB\n");
            return XP_ERR_FILE_OPEN;
        }

        for (index = 0; index < MAX_VRF_IDX; index++)
        {
            char *result = fgets(buf, sizeof(buf), fPtr);
            if (!result)
            {
                printf("Error while reading Vrf DB\n");
            }
            sscanf(buf, "%u-%u\n", &configVrfId, &allocatedVrfId);
            if (configVrfId >= MAX_VRF_IDX)
            {
                printf("Invalid Vrf Id %d. Should be less than %d\n", configVrfId, MAX_VRF_IDX);
            }
            else
            {
                vrfIdxToVrf[configVrfId] = allocatedVrfId;
            }
        }
        if (fPtr)
        {
            fclose(fPtr);
        }
    }
    printf("xpApp populate VRF done.\n");

    //
    // Populate VLAN and portVlan Tables
    //
    for (entryNum = 0; entryNum < numOfVlanEntry; entryNum++)
    {
        VlanEntry * vlanEntry = (VlanEntry *)getEntry(&configFile.vlanData, entryNum);
        err = xpSaiAppConfigureVlan(devId, switchOid, vlanEntry);
        if (err < 0)
        {
            printf("Error: programming Vlan data through xpAppConfigureVlan, error code = %d\n",
                   err);
            return err;
        }
    }
    printf("xpApp populate VLAN done.\n");

    for (entryNum = 0; entryNum < numOfTenantEntry; entryNum++)
    {
        VlanTenantIdEntry * tenantEntry = (VlanTenantIdEntry *)getEntry(
                                              &configFile.vlanTenantIdData, entryNum);
        err = xpSaiAppConfigureTenant(devId, switchOid, tenantEntry);
        if (err < 0)
        {
            printf("Error: programming Tenant data through xpSaiAppConfigureTenant, error code = %d\n",
                   err);
            return err;
        }
    }
    printf("xpApp populate Tenants done.\n");

    //Configure the portDefaultVlaId
    for (entryNum = 0; entryNum < numOfPortConfigEntry; entryNum++)
    {
        printf("\nIn PortConfig Programming\n");
        xpSaiAppWritePortIvifEntry(devId, switchOid,
                                   (PortConfigEntry *)getEntry(&configFile.portConfigData, entryNum));
    }
    printf("xpSaiApp populate port configuration done.\n");

    //Configure the Nh for Tunnel Entries
    for (entryNum = 0; entryNum < numOfNhEntry; entryNum++)
    {
        printf("\nIn tnlNh Programming\n");
        IpxNhEntry *tnlNhData = (IpxNhEntry *)getEntry(&configFile.ipvxNhData,
                                                       entryNum);
        if (tnlNhData->nhType == 1)
        {
            xpSaiAppPopulateNh(devId, switchOid, tnlNhData);
        }
    }


    //
    // Populate vxLan/Nvgre/gre/ipinip/geneve Tables
    //
    for (entryNum = 0; entryNum < numOfTnlEntry; entryNum++)
    {
        if (!entryNum)
        {
            xpsTunnelEnableTermination(devId);
        }
        TunnelEntry *tunnelEntry = (TunnelEntry *)getEntry(&configFile.tunnelData,
                                                           entryNum);

        err = xpSaiAppConfigureIpTnnlData(devId, switchOid, tunnelEntry);
        if (err)
        {
            printf("Error: programming Tunnel data through xpsAppConfigureIpTnnlData, error code = %d\n",
                   err);
            //return err;
        }
    }

    printf("xpApp populate Tunnel  done.\n");

    //
    // Populate the LAG entries
    //
    for (entryNum = 0; entryNum < numOfLagEntry; entryNum++)
    {
        err = xpSaiAppConfigureLag(devId, switchOid,
                                   (PortLagEntry *)getEntry(&configFile.portLagData, entryNum));
        if (err)
        {
            printf("Error: programming Lag data through xpSaiAppConfigureLagData, error code = %d\n",
                   err);
            //return err;
        }
    }

    // Populate Tunnel Table with MPLS entries
    for (entryNum = 0; entryNum < numOfMplsTnlEntry; entryNum++)
    {
        if (!entryNum)
        {
            xpsTunnelEnableTermination(devId);
        }
        TunnelMplsEntry* tunnelMplsEntry = (TunnelMplsEntry *)getEntry(
                                               &configFile.tunnelMplsData, entryNum);
        xpSaiAppConfigureMplsTunnelEntry(devId, switchOid, tunnelMplsEntry);
    }
    printf("xpApp populate MPLS tunnels done.\n");

    //
    // Populate the FDB entries
    //
    for (entryNum = 0; entryNum < numOfFdbEntry; entryNum++)
    {
        MacEntry * xpsFdbData = (MacEntry *)getEntry(&configFile.macData, entryNum);
        xpSaiAppAddFdb(devId, switchOid, xpsFdbData, numOfMvifEntry);
    }
    printf("xpApp populate FDB done.\n");

    //
    // Populate NH Table
    //
    for (entryNum = 0; entryNum < numOfNhEntry; entryNum++)
    {
        printf("\nIn ipvxNh Programming\n");
        IpxNhEntry *ipvxNhData = (IpxNhEntry *)getEntry(&configFile.ipvxNhData,
                                                        entryNum);
        if (ipvxNhData->nhType == 0)
        {
            xpSaiAppPopulateNh(devId, switchOid, ipvxNhData);
        }
    }

    //Configure the Mpls Label Table
    for (entryNum = 0; entryNum < numOfMplsLabelEntry; entryNum++)
    {
        xpSaiAppConfigureMplsLabelEntry(devId, switchOid,
                                        (MplsLabelEntry *)getEntry(&configFile.mplsLabelData, entryNum));
    }
    printf("xpApp populate MPLS label done.\n");

    //
    // Populate Ipv4Host Table
    //
    for (entryNum = 0; entryNum < numOfIpv4HostEntry; entryNum++)
    {
        //  uint32_t nhIndex = 0;
        printf("\nIn Ipv4Host Programming\n");
        Ipv4HostEntry* ipv4HostData = (Ipv4HostEntry *)getEntry(
                                          &configFile.ipv4HostData, entryNum);

        printf("============ Ipv4HostData->nhId = %d\n", ipv4HostData->nhId);
        IpxNhEntry *ipvxNhData = (IpxNhEntry *)getEntry(&configFile.ipvxNhData,
                                                        ipv4HostData->nhId);
        if (ipvxNhData)
        {
            /*Since normal ipv4 hosting entries are to be written via SAI adapter APIs,
              need to write only entries for tunnel and lag test cases via XPS APIs*/

            if ((ipvxNhData->isTagged > 0x1) || (ipvxNhData->tnlIdx != -1))
            {
                xpSaiAppWriteIpv4HostEntry(devId, switchOid, ipv4HostData);
            }
        }
    }

    //
    // Populate Ipv4Route Table
    //
    //

#if 1
    for (entryNum = 0; entryNum < numOfIpv4RouteEntry; entryNum++)
    {
        printf("\nIn Ipv4Route Programming\n");

        Ipv4RouteEntry* ipv4RouteData = (Ipv4RouteEntry *)getEntry(
                                            &configFile.ipv4RouteData, entryNum);

        uint16_t nhIdx = *((uint16_t *)getEntry(&(ipv4RouteData->nhId), 0));
        IpxNhEntry *ipvxNhData = (IpxNhEntry *)getEntry(&configFile.ipvxNhData, nhIdx);
        if (ipvxNhData)
        {
            /* Since normal ipv4 hosting entries are to be written via SAI adapter APIs,
               need to write only entries for tunnel and lag test cases via XPS APIs*/
            if ((ipvxNhData->isTagged > 0x1) || (ipvxNhData->tnlIdx != -1))
            {
                xpSaiAppWriteIpv4RouteEntry(devId, switchOid, ipv4RouteData);
            }
        }
    }
    //xpSaiAppL3UcDisplayPrefixes(devId);
#else
    //xpSaiAppL3UcAddIpv6Prefix(devId);
    //xpSaiAppL3UcAddPrefix(devId);
#endif

    //
    // Populate ControlMac Table
    //
    for (entryNum = 0; entryNum < numOfControlMacEntry; entryNum++)
    {
        printf("\nIn ControlMac Programming\n");
        xpSaiAppWriteControlMacEntry(devId, switchOid,
                                     (ControlMacEntry *)getEntry(&configFile.controlMacData, entryNum));
    }

    //
    // Populate Ipv6Host Table
    //
    static uint64_t
    nhIdIp6[256]; //array to store nhIds created by NextHop Call, latter will use in SaiRoute call for reference
    for (entryNum = 0; entryNum < numOfIpv6HostEntry; entryNum++)
    {
        //        uint32_t nhIndex = 0;
        printf("\nIn Ipv6Host Programming\n");
        Ipv6HostEntry* ipv6HostData = (Ipv6HostEntry *)getEntry(
                                          &configFile.ipv6HostData, entryNum);

        printf("============ Ipv6HostData->nhId = %d\n", ipv6HostData->nhId);
        IpxNhEntry *ipvxNhData = (IpxNhEntry *)getEntry(&configFile.ipvxNhData,
                                                        ipv6HostData->nhId);
        if (ipvxNhData)
        {

            /*Since normal ipv4 hosting entries are to be written via SAI adapter APIs,
                need to write only entries for tunnel and lag test cases via XPS APIs*/

            if ((ipvxNhData->isTagged > 0x1) || (ipvxNhData->tnlIdx != -1))
            {
                xpSaiAppWriteIpv6HostEntry(devId, switchOid, ipv6HostData);
            }
        }
    }
    //
    //     Populate  Ipv6Route Table
    //
    for (entryNum = 0; entryNum < numOfIpv6RouteEntry; entryNum++)
    {
        printf("\nIn Ipv6Route Programming\n");

        Ipv6RouteEntry* ipv6RouteData = (Ipv6RouteEntry *)getEntry(
                                            &configFile.ipv6RouteData, entryNum);

        if (ipv6RouteData->ecmpSize > 1) //currently sai supports ecmpSize == 1
        {
            xpSaiAppWriteIpv6RouteEntry(devId, switchOid, ipv6RouteData);
        }
        else
        {
            uint16_t nhIdx = *((uint16_t *)getEntry(&(ipv6RouteData->nhId), 0));
            IpxNhEntry *ipvxNhData = (IpxNhEntry *)getEntry(&configFile.ipvxNhData, nhIdx);
            if (ipvxNhData)
            {

                /*Since normal ipv4 hosting entries are to be written via SAI adapter APIs,
                    need to write only entries for tunnel and lag test cases via XPS APIs*/

                if ((ipvxNhData->isTagged > 0x1) || (ipvxNhData->tnlIdx != -1))
                {
                    xpSaiAppWriteIpv6RouteEntry(devId, switchOid, ipv6RouteData);
                }
            }
        }
    }

    //
    // Populate ipv4BridgeMc entries
    //
    printf("\nIpv4 Bridge Mc Programming:\n");
    for (entryNum = 0; entryNum < numOfIpv4BridgeMcEntry; entryNum++)
    {
        xpSaiAppConfigureIpv4BridgeMcEntry(devId, switchOid,
                                           (Ipv4BridgeMcEntry *)getEntry(&configFile.ipv4BridgeMcData, entryNum),
                                           numOfMvifEntry, &ipv4BridgeMcHandle);
    }

    //
    // Populate ipv6BridgeMc entries
    //
    printf("\nIpv6 Bridge Mc Programming:\n");
    for (entryNum = 0; entryNum < numOfIpv6BridgeMcEntry; entryNum++)
    {
        xpSaiAppConfigureIpv6BridgeMcEntry(devId, switchOid,
                                           (Ipv6BridgeMcEntry *)getEntry(&configFile.ipv6BridgeMcData, entryNum),
                                           numOfMvifEntry, &ipv6BridgeMcHandle);
    }

    //
    // prepare mdt node entry from OIF
    //
    //for(entryNum = 0; entryNum < numOfL3OIFEntry; entryNum++)
    //{
    //xpSaiAppPreapareL3MdtNodeList((l3MdtEntry *)getEntry(&configFile.l3OIFEntry, entryNum), &mdtL3NodeListData, entryNum);
    //}

    //
    // Populate ipv4RouteMc entries
    //
    printf("\nIpv4 Route Mc Programming:\n");
    for (entryNum = 0; entryNum < numOfIpv4RouteMcEntry; entryNum++)
    {
        xpSaiAppConfigureIpv4RouteMcEntry(devId, switchOid,
                                          (Ipv4RouteMcEntry *)getEntry(&configFile.ipv4RouteMcData, entryNum),
                                          numOfMvifEntry, &ipv4RouteMcHandle);
    }

    //
    // Populate ipv6RouteMc entries
    //
    printf("\nIpv6 Route Mc Programming:\n");
    for (entryNum = 0; entryNum < numOfIpv6RouteMcEntry; entryNum++)
    {
        xpSaiAppConfigureIpv6RouteMcEntry(devId, switchOid,
                                          (Ipv6RouteMcEntry *)getEntry(&configFile.ipv6RouteMcData, entryNum),
                                          numOfMvifEntry, &ipv6RouteMcHandle);
    }

    //
    // Populate ipv4PimBiDir entries
    //
    printf("\nIpv4 PimBiDir Programming:\n");
    for (entryNum = 0; entryNum < numOfIpv4PimBiDirEntry; entryNum++)
    {
        xpSaiAppConfigureIpv4PimBiDirEntry(devId, switchOid,
                                           (Ipv4PimBiDirEntry *)getEntry(&configFile.ipv4PimBiDirData, entryNum),
                                           &ipv4PimBiDirHandle);
    }

    printf("\nRouter MAC MSBs programming:\n");
    if ((err = xpSaiAppConfigureMacSAMSBsEntry(devId, switchOid,
                                               configFile.macSAMSBs)) != XP_NO_ERR)
    {
        printf("Failed to configure Router MAC MSBs. Error code %d\n", err);
    }

    testMplsTunnel(devId, XP_SINGLE_PIPE);

    if ((err = xpSaiAppConfigureMirrorData(devId, switchOid)) != XP_NO_ERR)
    {
        printf("Failed to configure Mirror. Error code %d\n", err);
        return err;
    }

    //
    // Populate Ipv4 Sai neighbor Table
    //
    for (entryNum = 0; entryNum < numOfSaiNeighborEntry; entryNum++)
    {
        printf("\nIn Ipv4 Sai neighbor Programming\n");
        Ipv4SaiNeighborEntry* ipv4SaiNeighborData = (Ipv4SaiNeighborEntry *)getEntry(
                                                        &configFile.ipv4SaiNeighborData, entryNum);

        ipv4SaiNeighborData->l3IntfId = vlanToL3Intf[(xpVlan_t)
                                                     ipv4SaiNeighborData->l3IntfId];

        xpSaiAppSaiWriteIpv4HostEntry(devId, switchOid, ipv4SaiNeighborData);
    }

    //
    // Populate Ipv4 Sai nexthop Table
    //
    for (entryNum = 0; entryNum < numOfSaiNhEntry; entryNum++)
    {
        uint64_t nhIndex;
        printf("\nIn Ipv4 Sai nexthop Programming\n");
        Ipv4SaiNhEntry* ipv4SaiNhData = (Ipv4SaiNhEntry *)getEntry(
                                            &configFile.ipv4SaiNhData, entryNum);

        ipv4SaiNhData->l3IntfId = vlanToL3Intf[(xpVlan_t)ipv4SaiNhData->l3IntfId];

        xpSaiAppSaiWriteIpv4NextHopEntry(devId, switchOid, ipv4SaiNhData, &nhIndex);
        nhIdIp4[ipv4SaiNhData->nhId] = nhIndex;
    }

    //
    // Populate Ipv4 Sai next hop group Table
    //

    for (entryNum = 0; entryNum < numOfSaiNhGroupEntry; entryNum++)
    {
        uint64_t nhGrpId;
        IpvxSaiNhGroupEntry* ipvxSaiNhGroupData = (IpvxSaiNhGroupEntry *)getEntry(
                                                      &configFile.ipSaiNhGroupData, entryNum);

        xpSaiAppConfigureIpv4NextHopGroupEntry(devId, switchOid, ipvxSaiNhGroupData,
                                               nhIdIp4, &nhGrpId);
        nhGrpIdIp4[ipvxSaiNhGroupData->nhGroupId] = nhGrpId;
    }

    //
    // Populate Ipv4 Sai router Table
    //

    uint64_t nhOrnhGrpId;
    for (entryNum = 0; entryNum < numOfSaiRouterEntry; entryNum++)
    {
        printf("\nIn Ipv4 Sai Ipv4 route Programming\n");
        nhOrnhGrpId = 0;
        Ipv4SaiRouteEntry* ipv4SaiRouteData = (Ipv4SaiRouteEntry *)getEntry(
                                                  &configFile.ipv4SaiRouteData, entryNum);
        if (ipv4SaiRouteData->nhGroupId == -1)
        {
            nhOrnhGrpId = nhIdIp4[ipv4SaiRouteData->nhId];
        }
        else
        {
            nhOrnhGrpId = nhGrpIdIp4[ipv4SaiRouteData->nhGroupId];
        }
        xpSaiAppSaiWriteIpv4RouteEntry(devId, switchOid, ipv4SaiRouteData, nhOrnhGrpId);
    }

    //
    // Populate Ipv6 Sai neighbor Table
    //
    for (entryNum = 0; entryNum < numOfSaiIpv6NeighborEntry; entryNum++)
    {
        printf("\nIn Ipv6 Sai neighbor Programming\n");
        Ipv6SaiNeighborEntry* ipv6SaiNeighborData = (Ipv6SaiNeighborEntry *)getEntry(
                                                        &configFile.ipv6SaiNeighborData, entryNum);

        ipv6SaiNeighborData->l3IntfId = vlanToL3Intf[(xpVlan_t)
                                                     ipv6SaiNeighborData->l3IntfId];

        xpSaiAppSaiWriteIpv6HostEntry(devId, switchOid, ipv6SaiNeighborData);
    }

    //
    // Populate Ipv6 Sai nexthop Table
    //
    for (entryNum = 0; entryNum < numOfSaiIpv6NhEntry; entryNum++)
    {
        uint64_t nhIndex;
        printf("\nIn Ipv6 Sai nexthop Programming\n");
        Ipv6SaiNhEntry* ipv6SaiNhData = (Ipv6SaiNhEntry *)getEntry(
                                            &configFile.ipv6SaiNhData, entryNum);

        ipv6SaiNhData->l3IntfId = vlanToL3Intf[(xpVlan_t)ipv6SaiNhData->l3IntfId];

        xpSaiAppSaiWriteIpv6NextHopEntry(devId, switchOid, ipv6SaiNhData, &nhIndex);
        nhIdIp6[ipv6SaiNhData->nhId] = nhIndex;

    }

    //
    // Populate Ipv6 Sai router Table
    //
    for (entryNum = 0; entryNum < numOfSaiIpv6RouterEntry; entryNum++)
    {
        printf("\nIn Ipv6 Sai nexthop Programming\n");
        Ipv6SaiRouteEntry* ipv6SaiRouteData = (Ipv6SaiRouteEntry *)getEntry(
                                                  &configFile.ipv6SaiRouteData, entryNum);

        xpSaiAppSaiWriteIpv6RouteEntry(devId, switchOid, ipv6SaiRouteData,
                                       nhIdIp6[ipv6SaiRouteData->nhId]);

    }

    //
    // Populate QoS Scheduler entries
    //
    for (entryNum = 0; entryNum < numOfSchedulerQueueEntry; entryNum++)
    {
        xpSaiAppConfigureQueueSchedulerEntry(devId, switchOid,
                                             (saiSchedulerDataEntry *)getEntry(&configFile.saiSchedulerData, entryNum));
    }

    //
    // Populate QoS Shaper port entries
    //
    for (entryNum = 0; entryNum < numOfShaperPortEntry; entryNum++)
    {
        xpSaiAppConfigureShaperPortEntry(devId, switchOid,
                                         (qosShaperPortDataEntry *)getEntry(&configFile.qosShaperPortData, entryNum));
    }

    //
    // Populate QoS AQM Queue Profile entries
    //
    for (entryNum = 0; entryNum < numOfWredEntry; entryNum++)
    {
        xpSaiAppConfigureWredEntry(devId, switchOid,
                                   (qosAqmQProfileDataEntry *)getEntry(&configFile.qosAqmQProfileData, entryNum));
    }

    //
    // Populate QoS Ingress QOS Map entries
    //
    for (entryNum = 0; entryNum < numOfIngressQosMapEntry; entryNum++)
    {
        xpSaiAppConfigureIngressQosMapEntry(devId, switchOid,
                                            (qosIngressMapDataEntry *)getEntry(&configFile.qosIngressMapData, entryNum));
    }

    //
    // Populate QoS Egress QOS Map entries
    //
    for (entryNum = 0; entryNum < numOfEgressQosMapEntry; entryNum++)
    {
        //TODO:For now not using SAI app support for egress QOS map. As this is giving more apptest failures.
        //Enable once issue is fixed.
        //xpSaiAppConfigureEgressQosMapEntry(devId, switchOid, (qosEgressMapDataEntry *)getEntry(&configFile.qosEgressMapData, entryNum));
    }

    //
    // Populate policer entries
    //
    for (entryNum = 0; entryNum < numOfPolicerEntry; entryNum++)
    {
        xpSaiAppConfigurePolicerEntry(devId, switchOid,
                                      (policerDataEntry *)getEntry(&configFile.policerData, entryNum));
    }

    // Enabling admin state for all ports
    sai_attribute_t sai_port_attr;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    uint16_t maxPorts = 0, port_id = 0;;
    sai_object_id_t portOid = 0;

    sai_port_attr.id = SAI_PORT_ATTR_ADMIN_STATE;
    sai_port_attr.value.u8 = 1;

    xpsGlobalSwitchControlGetMaxNumPhysicalPorts(devId, &maxPorts);
    for (port_id = 0; port_id < maxPorts; port_id++)
    {
        if ((saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                          (sai_uint64_t)port_id, &portOid)) != SAI_STATUS_SUCCESS)
        {
            printf("Error : SAI object can not be created.\n");
        }

        if ((saiStatus = xpSaiPortApi->set_port_attribute(portOid,
                                                          &sai_port_attr)) != SAI_STATUS_SUCCESS)
        {
            printf("%s set_port_attribute() fail |retVal: %d\n", __FUNCTION__, saiStatus);
        }
    }

    return err;
}

XP_STATUS xpSaiAppWritePortIvifEntry(xpsDevice_t devId,
                                     sai_object_id_t switchOid, PortConfigEntry *portConfigData)
{
    XP_STATUS retVal;
    xpsPortConfig_t portConfigEntryT;
    xpsInterfaceId_t ingressVif, egressVif;
    memset(&portConfigEntryT, 0x0, sizeof(xpsPortConfig_t));

    retVal = xpsIsPortInited(devId, portConfigData->port);
    if (retVal != XP_NO_ERR)
    {
        printf("xpSaiApp unable to retrieve MAC number for Port #%d.",
               portConfigData->port);
    }

    retVal = xpsPortGetPortIntfId(devId, portConfigData->port, &ingressVif);
    if (retVal)
    {
        printf("Error : xpsPortGetPortIntfId() Failed for devId %d portId : %d\n",
               devId, portConfigData->port);
        return retVal;
    }

    retVal = xpsPortGetPortIntfId(devId, portConfigData->evif, &egressVif);
    if (retVal)
    {
        printf("Error : xpsPortGetPortIntfId() Failed for devId %d portId : %d\n",
               devId, portConfigData->port);
        return retVal;
    }

    retVal = xpsPortGetConfig(devId, ingressVif, &portConfigEntryT);
    if (retVal)
    {
        return retVal;
    }

    portConfigEntryT.ingressVif = ingressVif;
    portConfigEntryT.portState = portConfigData->portState;
    portConfigEntryT.acceptedFrameType = portConfigData->acceptedFrameType;
    portConfigEntryT.bypassTunnelVif = portConfigData->disableTunnelVif;
    portConfigEntryT.bypassACLsPBR = portConfigData->disableAclPbr;
    portConfigEntryT.setBridgeDomain = portConfigData->setBd;
    portConfigEntryT.bridgeDomain = portConfigData->defaultVlan;
    portConfigEntryT.pvid = portConfigData->defaultVlan;
    portConfigEntryT.pvidModeAllPkt = 0;
    portConfigEntryT.pvid = portConfigData->defaultVlan;
    portConfigEntryT.privateWireEn = portConfigData->privateWireEn;
    portConfigEntryT.evif = egressVif;
    portConfigEntryT.addPortBasedTag = portConfigData->addPortBasedTag;
    portConfigEntryT.macSAmissCmd = portConfigData->macSaMissCmd;

    retVal = xpsPortSetConfig(devId, ingressVif, &portConfigEntryT);
    if (retVal)
    {
        return retVal;
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiAppWriteControlMacEntry(xpsDevice_t devId,
                                       sai_object_id_t switchOid, ControlMacEntry* controlMacData)
{
    XP_STATUS retVal = XP_NO_ERR;

    if (!controlMacData)
    {
        printf("Null pointer received while adding ControlMac Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    printf("controlMacData->routerMac: %d \n", controlMacData->routerMac);

    if (controlMacData->routerMac == 1)
    {
        sai_attribute_t switchAttr;
        uint8_t* mac = controlMacData->keyMacAddr;
        uint8_t multicastMac[3] = {0x01, 0x00, 0x5E};

        printf("MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
               mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);

        xpSaiAppMacReverse(mac);
        /* Check whether MAC is multicast MAC */
        if (!memcmp(mac, multicastMac, 3))
        {
            xpSaiAppMacReverse(mac);
            /*All validation checks supposed to be done on XPS layer.*/
            retVal = xpsL3AddIngressRouterMac(devId, mac);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Switch MAC address could not be set!\n");
                return retVal;
            }

            return retVal;
        }

        switchAttr.id = SAI_SWITCH_ATTR_SRC_MAC_ADDRESS;
        memcpy(switchAttr.value.mac, mac, sizeof(switchAttr.value.mac));
        if ((retVal = (XP_STATUS)xpSaiSwitchApi->set_switch_attribute(switchOid,
                                                                      &switchAttr)))
        {
            printf("Error in setting xpsL3AddIngressRouterMac \n");
        }
    }

    return retVal;
}

XP_STATUS xpSaiAppConfigureMvifEntry(xpsDevice_t devId, MvifEntry *mvifData,
                                     xpsMcL3InterfaceListId_t l3IfListId, xpsMcL2InterfaceListId_t ifListId,
                                     bool isL2)
{
    XP_STATUS retVal;
    uint32_t intfId, i, greIntfId;

    if (!mvifData)
    {
        printf("Error: Null pointer received for mvifData Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    switch (mvifData->tnlType)
    {
        //For non-tunnel type
        case -1:
            if (mvifData->numPorts > 0)
            {
                for (i = 0 ; i < mvifData->numPorts; i++)
                {
                    if ((retVal = xpsPortGetPortIntfId(devId, mvifData->portList[i],
                                                       &intfId)) != XP_NO_ERR)
                    {
                        printf("%s: Error in getting intfId for portNum %d: error code: %d\n",
                               __FUNCTION__, mvifData->portList[i], retVal);
                        return retVal;
                    }
                    if ((retVal = xpsMulticastAddInterfaceToL2InterfaceList(devId, ifListId,
                                                                            intfId)) != XP_NO_ERR)
                    {
                        printf("%s: Error in adding %d intfId of Port %d in ifListId %d: error code: %d\n",
                               __FUNCTION__, intfId, mvifData->portList[i], ifListId, retVal);
                        return retVal;
                    }
                    printf("%s: intfId %d of Port %d added successfully in ifList %d\n",
                           __FUNCTION__, intfId, mvifData->portList[i], ifListId);
                }
            }
            break;
        case 5:
            if (mvifData->numPorts > 0)
            {
                for (i = 0 ; i < mvifData->numPorts; i++)
                {
                    if ((retVal = xpsPortGetPortIntfId(devId, mvifData->portList[i],
                                                       &intfId)) != XP_NO_ERR)
                    {
                        printf("%s: Error in getting intfId for portNum %d: error code: %d\n",
                               __FUNCTION__, mvifData->portList[i], retVal);
                        return retVal;
                    }
                    if ((retVal = xpsMulticastAddInterfaceToL2InterfaceList(devId, ifListId,
                                                                            intfId)) != XP_NO_ERR)
                    {
                        printf("%s: Error in adding %d intfId of Port %d in ifListId %d: error code: %d\n",
                               __FUNCTION__, intfId, mvifData->portList[i], ifListId, retVal);
                        return retVal;
                    }
                    printf("%s: intfId %d of Port %d added successfully in ifList %d\n",
                           __FUNCTION__, intfId, mvifData->portList[i], ifListId);
                }
            }
            break;
        //For Vxlan
        case 6:
            intfId = tnlVif[devId][mvifData->tnlIdx];
            if ((retVal = xpsMulticastAddInterfaceToL2InterfaceList(devId, ifListId,
                                                                    intfId)) != XP_NO_ERR)
            {
                printf("%s: Error in adding %d ifList for %d intfId for tnlIdx %d: error code: %d\n",
                       __FUNCTION__, ifListId, intfId, mvifData->tnlIdx, retVal);
                return retVal;
            }
            printf("%s: intfId %d added successfully in ifList %d for tnlIdx %d\n",
                   __FUNCTION__, intfId, ifListId, mvifData->tnlIdx);
            break;
        case 7:
            intfId = tnlVif[devId][mvifData->tnlIdx];
            if ((retVal = xpsMulticastAddInterfaceToL2InterfaceList(devId, ifListId,
                                                                    intfId)) != XP_NO_ERR)
            {
                printf("%s: Error in adding %d ifList for %d intfId for tnlIdx %d: error code: %d\n",
                       __FUNCTION__, ifListId, intfId, mvifData->tnlIdx, retVal);
                return retVal;
            }
            printf("%s: intfId %d added successfully in ifList %d for tnlIdx %d\n",
                   __FUNCTION__, intfId, ifListId, mvifData->tnlIdx);
            break;
        case 8:
            // TODO: Currently PBB is not supported in xpSai.
            // We will enable below code once PBB support is available in xpSai
            break;
        case 9:
            intfId = l3TnlIntf[devId][mvifData->tnlIdx];
            greIntfId = tnlVif[devId][mvifData->tnlIdx];
            //For GRE
            if ((retVal = xpsMulticastAddInterfaceToL3InterfaceList(devId, l3IfListId,
                                                                    intfId)) != XP_NO_ERR)
            {
                printf("%s: Error in adding intfId to %d l3IfList for %d intfId for tnlIdx %d: error code: %d\n",
                       __FUNCTION__, l3IfListId, intfId, mvifData->tnlIdx, retVal);
                return retVal;
            }
            printf("%s: greIntfId %d: intfId %d added successfully in l3IfList %d for tnlIdx %d\n",
                   __FUNCTION__, greIntfId, intfId, l3IfListId, mvifData->tnlIdx);
            break;

        default:
            printf("Invalid Encap for type = %d\n", mvifData->tnlType);
            return XP_ERR_INVALID_PARAMS;
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiAppConfigureIpv4BridgeMcEntry(xpsDevice_t devId,
                                             sai_object_id_t switchOid, Ipv4BridgeMcEntry *ipv4BridgeMcData,
                                             uint32_t numOfMvifEntry, uint32_t *index)
{
    XP_STATUS retVal;
    uint32_t i, entryNum;
    xpsMcL2InterfaceListId_t ifListId;
    xpsMulticastIPv4BridgeEntry_t ipv4BridgeMcEntry;// getIpv4BridgeMcEntry;

    if (!ipv4BridgeMcData || !index)
    {
        printf("Error: Null pointer received for ipv4BridgeMcData Entry \n");
        return XP_ERR_NULL_POINTER;
    }
    memset(&ifListId, 0, sizeof(ifListId));
    if ((retVal = xpsMulticastCreateL2InterfaceList(ipv4BridgeMcData->vlan,
                                                    &ifListId)) != XP_NO_ERR)
    {
        printf("%s: Error in Creating Interface List for vlan: %d: error code: %d\n",
               __FUNCTION__, ipv4BridgeMcData->vlan, retVal);
        return retVal;
    }
    if ((retVal = xpsMulticastAddL2InterfaceListToDevice(devId,
                                                         ifListId)) != XP_NO_ERR)
    {
        printf("%s: Error in Adding Interface List %d to device %d: error code: %d\n",
               __FUNCTION__, devId, ifListId, retVal);
        return retVal;
    }

    memset(&ipv4BridgeMcEntry, 0, sizeof(xpIPv4MulticastBridgeEntry_t));
    ipv4BridgeMcEntry.pktCmd = (xpMulticastPktCmdType_e) ipv4BridgeMcData->pktCmd;
    ipv4BridgeMcEntry.bdId = ipv4BridgeMcData->vlan;
    COPY_IPV4_ADDR_T(ipv4BridgeMcEntry.sourceAddress, ipv4BridgeMcData->sip);
    COPY_IPV4_ADDR_T(ipv4BridgeMcEntry.groupAddress, ipv4BridgeMcData->grpAddr);
    ipv4BridgeMcEntry.multicastVifIdx = ifListId;
    if ((retVal = xpsMulticastAddIPv4BridgeEntry(devId,
                                                 &ipv4BridgeMcEntry, 0)) != XP_NO_ERR)
    {
        printf("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    printf("\naddIPv4MulticastBridgeEntry -> sip : %d.%d.%d.%d grpAddr : %d.%d.%d.%d pktcmd : %d, bdId: %d, mvif: %d\n",
           ipv4BridgeMcEntry.sourceAddress[3], ipv4BridgeMcEntry.sourceAddress[2],
           ipv4BridgeMcEntry.sourceAddress[1], ipv4BridgeMcEntry.sourceAddress[0],
           ipv4BridgeMcEntry.groupAddress[3], ipv4BridgeMcEntry.groupAddress[2],
           ipv4BridgeMcEntry.groupAddress[1], ipv4BridgeMcEntry.groupAddress[0],
           ipv4BridgeMcEntry.pktCmd, ipv4BridgeMcEntry.bdId,
           ipv4BridgeMcEntry.multicastVifIdx);

    *index = 0;
    /*    if ((retVal = xpGetIPv4MulticastBridgeEntry(devId, *index, &getIpv4BridgeMcEntry)) != XP_NO_ERR)
        {
            printf("%s: Error in reading an entry: error code: %d\n", __FUNCTION__, retVal);
            return retVal;
        }
        printf("getIPv4MulticastBridgeEntry -> sip : %d.%d.%d.%d grpAddr : %d.%d.%d.%d pktcmd : %d, bdId: %d, mvif: %d\n", getIpv4BridgeMcEntry.sourceAddress[3], getIpv4BridgeMcEntry.sourceAddress[2], getIpv4BridgeMcEntry.sourceAddress[1], getIpv4BridgeMcEntry.sourceAddress[0], getIpv4BridgeMcEntry.groupAddress[3], getIpv4BridgeMcEntry.groupAddress[2], getIpv4BridgeMcEntry.groupAddress[1], getIpv4BridgeMcEntry.groupAddress[0], getIpv4BridgeMcEntry.pktCmd, getIpv4BridgeMcEntry.bdId, getIpv4BridgeMcEntry.multicastVifIdx);
    */

    if (ipv4BridgeMcData->agingFlag)
    {
        //Enable aging for an entry
        xpsMulticastConfigureIpv4BridgeTableAging(devId, true);
        xpsMulticastConfigureIpv4BridgeEntryAging(devId, 1, *index);

        //Configure two times to make aging-time out
        xpsMulticastTriggerIpv4BridgeTableAging(devId);
        xpsMulticastTriggerIpv4BridgeTableAging(devId);

        //Process FIFO to delete the aged out entry with default handler
        xpsAgeFifoHandler(devId);
    }

    /* Populate mvif entries */
    for (i = 0; i < 10; i++)
    {
        if (ipv4BridgeMcData->nodeIdx[i] == 0)
        {
            break;
        }

        for (entryNum = 0; entryNum < numOfMvifEntry; entryNum++)
        {
            if (((MvifEntry *)getEntry(&configFile.mvifData,
                                       entryNum))->index == ipv4BridgeMcData->nodeIdx[i])
            {
                xpSaiAppConfigureMvifEntry(devId, (MvifEntry *)getEntry(&configFile.mvifData,
                                                                        entryNum), 0, ifListId, 1);
                break;
            }
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiAppConfigureIpv6BridgeMcEntry(xpsDevice_t devId,
                                             sai_object_id_t switchOid, Ipv6BridgeMcEntry *ipv6BridgeMcData,
                                             uint32_t numOfMvifEntry, uint32_t *index)
{
    XP_STATUS retVal;
    uint32_t  i, entryNum;
    xpsHashIndexList_t indexList;
    xpsMcL2InterfaceListId_t ifListId;
    xpsMulticastIPv6BridgeEntry_t ipv6BridgeMcEntry, getIpv6BridgeMcEntry;

    if (!ipv6BridgeMcData || !index)
    {
        printf("Error: Null pointer received for ipv6BridgeMcData Entry \n");
        return XP_ERR_NULL_POINTER;
    }
    memset(&ifListId, 0, sizeof(ifListId));
    if ((retVal = xpsMulticastCreateL2InterfaceList(ipv6BridgeMcData->vlan,
                                                    &ifListId)) != XP_NO_ERR)
    {
        printf("%s: Error in Creating Interface List for vlan: %d: error code: %d\n",
               __FUNCTION__, ipv6BridgeMcData->vlan, retVal);
        return retVal;
    }
    if ((retVal = xpsMulticastAddL2InterfaceListToDevice(devId,
                                                         ifListId)) != XP_NO_ERR)
    {
        printf("%s: Error in Adding Interface List %d to device %d: error code: %d\n",
               __FUNCTION__, devId, ifListId, retVal);
        return retVal;
    }

    memset(&ipv6BridgeMcEntry, 0, sizeof(xpIPv6MulticastBridgeEntry_t));
    ipv6BridgeMcEntry.pktCmd = (xpMulticastPktCmdType_e) ipv6BridgeMcData->pktCmd;
    ipv6BridgeMcEntry.bdId = ipv6BridgeMcData->vlan;
    COPY_IPV6_ADDR_T(ipv6BridgeMcEntry.sourceAddress, ipv6BridgeMcData->sip);
    COPY_IPV6_ADDR_T(ipv6BridgeMcEntry.groupAddress, ipv6BridgeMcData->grpAddr);
    ipv6BridgeMcEntry.multicastVifIdx = ifListId;
    memset(&indexList, 0, sizeof(indexList));
    if ((retVal = xpsMulticastAddIPv6BridgeEntry(devId, &ipv6BridgeMcEntry,
                                                 &indexList)) != XP_NO_ERR)
    {
        printf("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    printf("\naddIPv6MulticastBridgeEntry -> sip : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d grpAddr : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d pktcmd : %d, bdId: %d, mvif: %d\n",
           ipv6BridgeMcEntry.sourceAddress[0], ipv6BridgeMcEntry.sourceAddress[1],
           ipv6BridgeMcEntry.sourceAddress[2], ipv6BridgeMcEntry.sourceAddress[3],
           ipv6BridgeMcEntry.sourceAddress[4], ipv6BridgeMcEntry.sourceAddress[5],
           ipv6BridgeMcEntry.sourceAddress[6], ipv6BridgeMcEntry.sourceAddress[7],
           ipv6BridgeMcEntry.sourceAddress[8], ipv6BridgeMcEntry.sourceAddress[9],
           ipv6BridgeMcEntry.sourceAddress[10], ipv6BridgeMcEntry.sourceAddress[11],
           ipv6BridgeMcEntry.sourceAddress[12], ipv6BridgeMcEntry.sourceAddress[13],
           ipv6BridgeMcEntry.sourceAddress[14], ipv6BridgeMcEntry.sourceAddress[15],
           ipv6BridgeMcEntry.groupAddress[0], ipv6BridgeMcEntry.groupAddress[1],
           ipv6BridgeMcEntry.groupAddress[2], ipv6BridgeMcEntry.groupAddress[3],
           ipv6BridgeMcEntry.groupAddress[4], ipv6BridgeMcEntry.groupAddress[5],
           ipv6BridgeMcEntry.groupAddress[6], ipv6BridgeMcEntry.groupAddress[7],
           ipv6BridgeMcEntry.groupAddress[8], ipv6BridgeMcEntry.groupAddress[9],
           ipv6BridgeMcEntry.groupAddress[10], ipv6BridgeMcEntry.groupAddress[11],
           ipv6BridgeMcEntry.groupAddress[12], ipv6BridgeMcEntry.groupAddress[13],
           ipv6BridgeMcEntry.groupAddress[14], ipv6BridgeMcEntry.groupAddress[15],
           ipv6BridgeMcEntry.pktCmd, ipv6BridgeMcEntry.bdId,
           ipv6BridgeMcEntry.multicastVifIdx);

    *index = indexList.index[0];

    if (indexList.size > 1)
    {
        printf("!!! Rehash occured!!! NOT TAKING CARE AS OF NOW.\n");
        return XP_NO_ERR;
    }
    memset(&getIpv6BridgeMcEntry, 0, sizeof(getIpv6BridgeMcEntry));
    if ((retVal = xpsMulticastGetIPv6BridgeEntry(devId, *index,
                                                 &getIpv6BridgeMcEntry)) != XP_NO_ERR)
    {
        printf("%s: Error in reading an entry: error code: %d\n", __FUNCTION__, retVal);
        return retVal;
    }

    printf("xpsMulticastGetIPv6BridgeEntry-> sip : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d grpAddr : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d pktcmd : %d, bdId: %d, mvif: %d, l2DomainId: %d\n",
           getIpv6BridgeMcEntry.sourceAddress[0], getIpv6BridgeMcEntry.sourceAddress[1],
           getIpv6BridgeMcEntry.sourceAddress[2], getIpv6BridgeMcEntry.sourceAddress[3],
           getIpv6BridgeMcEntry.sourceAddress[4], getIpv6BridgeMcEntry.sourceAddress[5],
           getIpv6BridgeMcEntry.sourceAddress[6], getIpv6BridgeMcEntry.sourceAddress[7],
           getIpv6BridgeMcEntry.sourceAddress[8], getIpv6BridgeMcEntry.sourceAddress[9],
           getIpv6BridgeMcEntry.sourceAddress[10], getIpv6BridgeMcEntry.sourceAddress[11],
           getIpv6BridgeMcEntry.sourceAddress[12], getIpv6BridgeMcEntry.sourceAddress[13],
           getIpv6BridgeMcEntry.sourceAddress[14], getIpv6BridgeMcEntry.sourceAddress[15],
           getIpv6BridgeMcEntry.groupAddress[0], getIpv6BridgeMcEntry.groupAddress[1],
           getIpv6BridgeMcEntry.groupAddress[2], getIpv6BridgeMcEntry.groupAddress[3],
           getIpv6BridgeMcEntry.groupAddress[4], getIpv6BridgeMcEntry.groupAddress[5],
           getIpv6BridgeMcEntry.groupAddress[6], getIpv6BridgeMcEntry.groupAddress[7],
           getIpv6BridgeMcEntry.groupAddress[8], getIpv6BridgeMcEntry.groupAddress[9],
           getIpv6BridgeMcEntry.groupAddress[10], getIpv6BridgeMcEntry.groupAddress[11],
           getIpv6BridgeMcEntry.groupAddress[12], getIpv6BridgeMcEntry.groupAddress[13],
           getIpv6BridgeMcEntry.groupAddress[14], getIpv6BridgeMcEntry.groupAddress[15],
           getIpv6BridgeMcEntry.pktCmd, getIpv6BridgeMcEntry.bdId,
           getIpv6BridgeMcEntry.multicastVifIdx, getIpv6BridgeMcEntry.ipv6McL2DomainId);

    /* Populate mvif entries */
    for (i = 0; i < 10; i++)
    {
        if (ipv6BridgeMcData->nodeIdx[i] == 0)
        {
            break;
        }

        for (entryNum = 0; entryNum < numOfMvifEntry; entryNum++)
        {
            if (((MvifEntry *)getEntry(&configFile.mvifData,
                                       entryNum))->index == ipv6BridgeMcData->nodeIdx[i])
            {
                xpSaiAppConfigureMvifEntry(devId, (MvifEntry *)getEntry(&configFile.mvifData,
                                                                        entryNum), 0, ifListId, 1);
                break;
            }
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiAppConfigureIpv4RouteMcEntry(xpsDevice_t devId,
                                            sai_object_id_t switchOid, Ipv4RouteMcEntry *ipv4RouteMcData,
                                            uint32_t numOfMvifEntry, uint32_t *index)
{
    XP_STATUS retVal;
    uint32_t  i, j, entryNum = 0, intfId;
    xpsHashIndexList_t indexList;
    xpsMcL2InterfaceListId_t l2IfListId;
    xpsMcL3InterfaceListId_t l3IfListId;
    xpsMulticastIPv4RouteEntry_t ipv4RouteMcEntry, getIpv4RouteMcEntry;

    if (!ipv4RouteMcData || !index)
    {
        printf("Error: Null pointer received for ipv4RouteMcData Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    //Create L3 IntfList Id
    memset(&l3IfListId, 0, sizeof(l3IfListId));
    if ((retVal = xpsMulticastCreateL3InterfaceList(&l3IfListId)) != XP_NO_ERR)
    {
        printf("%s: Error in Creating L3Interface List: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }
    // Add L3 IntfList to Dev
    if ((retVal = xpsMulticastAddL3InterfaceListToDevice(devId,
                                                         l3IfListId)) != XP_NO_ERR)
    {
        printf("%s: Error in Adding L3Interface List %d to device %d: error code: %d\n",
               __FUNCTION__, devId, l3IfListId, retVal);
        return retVal;
    }

    memset(&ipv4RouteMcEntry, 0, sizeof(xpsMulticastIPv4RouteEntry_t));
    ipv4RouteMcEntry.pktCmd = (xpMulticastPktCmdType_e) ipv4RouteMcData->pktCmd;
    ipv4RouteMcEntry.rpfType = (xpMulticastRpfCheckType_e) ipv4RouteMcData->rpfType;
    ipv4RouteMcEntry.rpfFailCmd = (xpMulticastRpfFailCmd_e)
                                  ipv4RouteMcData->rpfFailCmd;
    ipv4RouteMcEntry.multicastVifIdx = l3IfListId;
    ipv4RouteMcEntry.vrfIdx = vrfIdxToVrf[ipv4RouteMcData->vrfId];
    COPY_IPV4_ADDR_T(ipv4RouteMcEntry.sourceAddress, ipv4RouteMcData->sip);
    COPY_IPV4_ADDR_T(ipv4RouteMcEntry.groupAddress, ipv4RouteMcData->grpAddr);

    if ((ipv4RouteMcEntry.rpfType == XP_MC_RPF_CHECK_TYPE_PORT) ||
        (ipv4RouteMcEntry.rpfType == XP_MC_RPF_CHECK_TYPE_PIMBIDIR ||
         (ipv4RouteMcEntry.rpfType == XP_MC_RPF_CHECK_TYPE_BD)))
    {
        ipv4RouteMcEntry.rpfValue = ipv4RouteMcData->rpfValue;
    }
    else if (ipv4RouteMcEntry.rpfType == XP_MC_RPF_CHECK_TYPE_VIF)
    {
        if ((retVal = xpsPortGetPortIntfId(devId, ipv4RouteMcData->rpfValue,
                                           &intfId)) != XP_NO_ERR)
        {
            printf("%s: Error in getting intfId for rpfVal(portNum) %d: error code: %d\n",
                   __FUNCTION__, ipv4RouteMcData->rpfValue, retVal);
            return retVal;
        }
        ipv4RouteMcEntry.rpfValue = intfId;
    }

    // Populate v4RouteMc Entry
    memset(&indexList, 0, sizeof(indexList));
    if ((retVal = xpsMulticastAddIPv4RouteEntry(devId, &ipv4RouteMcEntry,
                                                &indexList)) != XP_NO_ERR)
    {
        printf("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    printf("\naddIPv4MulticastRouteEntry -> sip : %d.%d.%d.%d grpAddr : %d.%d.%d.%d pktcmd : %d, vrfId: %d, mvif: %d, rpfType : %d, rpfValue : %d, rpfFailCmd : %d\n",
           ipv4RouteMcEntry.sourceAddress[3], ipv4RouteMcEntry.sourceAddress[2],
           ipv4RouteMcEntry.sourceAddress[1], ipv4RouteMcEntry.sourceAddress[0],
           ipv4RouteMcEntry.groupAddress[3], ipv4RouteMcEntry.groupAddress[2],
           ipv4RouteMcEntry.groupAddress[1], ipv4RouteMcEntry.groupAddress[0],
           ipv4RouteMcEntry.pktCmd, ipv4RouteMcEntry.vrfIdx,
           ipv4RouteMcEntry.multicastVifIdx, ipv4RouteMcEntry.rpfType,
           ipv4RouteMcEntry.rpfValue, ipv4RouteMcEntry.rpfFailCmd);

    *index = indexList.index[0];

    if (indexList.size > 1)
    {
        printf("!!! Rehash occured!!! NOT TAKING CARE AS OF NOW.\n");
        return XP_NO_ERR;
    }
    memset(&getIpv4RouteMcEntry, 0, sizeof(getIpv4RouteMcEntry));
    if ((retVal = xpsMulticastGetIPv4RouteEntry(devId, *index,
                                                &getIpv4RouteMcEntry)) != XP_NO_ERR)
    {
        printf("%s: Error in reading an entry: error code: %d\n", __FUNCTION__, retVal);
        return retVal;
    }
    printf("xpsMulticastGetIPv4RouteEntry-> sip : %d.%d.%d.%d grpAddr : %d.%d.%d.%d pktcmd : %d, vrfId: %d, mvif: %d, rpfType : %d, rpfVal : %d, rpfFailCmd : %d\n\n\n",
           getIpv4RouteMcEntry.sourceAddress[3], getIpv4RouteMcEntry.sourceAddress[2],
           getIpv4RouteMcEntry.sourceAddress[1], getIpv4RouteMcEntry.sourceAddress[0],
           getIpv4RouteMcEntry.groupAddress[3], getIpv4RouteMcEntry.groupAddress[2],
           getIpv4RouteMcEntry.groupAddress[1], getIpv4RouteMcEntry.groupAddress[0],
           getIpv4RouteMcEntry.pktCmd, getIpv4RouteMcEntry.vrfIdx,
           getIpv4RouteMcEntry.multicastVifIdx, getIpv4RouteMcEntry.rpfType,
           getIpv4RouteMcEntry.rpfValue, getIpv4RouteMcEntry.rpfFailCmd);


    if (ipv4RouteMcData->agingFlag)
    {
        xpsMulticastConfigureIpv4RouteTableAging(devId, true);
        xpsMulticastConfigureIpv4RouteEntryAging(devId, 1, *index);

        //Trigger two times to make aging-time out
        xpsMulticastTriggerIpv4RouteTableAging(devId);
        xpsMulticastTriggerIpv4RouteTableAging(devId);

        //Process FIFO to delete the aged out entry with default handler
        xpsAgeFifoHandler(devId);
    }

    /* Iterate through all mvifs for all vlans in one L3 Intf*/
    for (i = 0; i < 10; i++)
    {
        if (ipv4RouteMcData->vlanNode[i].vlanId == 0)
        {
            break;
        }
        //Create L2 IntfList for Given Vlan
        memset(&l2IfListId, 0, sizeof(l2IfListId));
        if ((retVal = xpsMulticastCreateL2InterfaceList(
                          ipv4RouteMcData->vlanNode[i].vlanId, &l2IfListId)) != XP_NO_ERR)
        {
            printf("%s: Error in Creating Interface List for vlan: %d: error code: %d\n",
                   __FUNCTION__, ipv4RouteMcData->vlanNode[i].vlanId, retVal);
            return retVal;
        }
        printf("%s: Created Interface List %d for vlan %d\n", __FUNCTION__, l2IfListId,
               ipv4RouteMcData->vlanNode[i].vlanId);
        //Add L2 IntfList to Dev
        if ((retVal = xpsMulticastAddL2InterfaceListToDevice(devId,
                                                             l2IfListId)) != XP_NO_ERR)
        {
            printf("%s: Error in Adding Interface List %d to device %d: error code: %d\n",
                   __FUNCTION__, devId, l2IfListId, retVal);
            return retVal;
        }
        /* Populate mvif entries */
        for (j = 0; j < 10; j++)
        {
            if (ipv4RouteMcData->vlanNode[i].nodeIdx[j] == 0)
            {
                break;
            }
            for (entryNum = 0; entryNum < numOfMvifEntry; entryNum++)
            {
                if (((MvifEntry *)getEntry(&configFile.mvifData,
                                           entryNum))->index == ipv4RouteMcData->vlanNode[i].nodeIdx[j])
                {
                    //add Intf to L2 IntfList
                    xpSaiAppConfigureMvifEntry(devId, (MvifEntry *)getEntry(&configFile.mvifData,
                                                                            entryNum), l3IfListId, l2IfListId, 0);
                    break;
                }
            }
        }
        if (((MvifEntry *)getEntry(&configFile.mvifData, entryNum))->tnlType!= 9)
        {
            //Add L2 IntfList to L3 IntfList
            if ((retVal = xpsMulticastAddBridgingInterfaceToL3InterfaceList(devId,
                                                                            l3IfListId, l2IfListId)) != XP_NO_ERR)
            {
                printf("%s: Error in Adding Bridge Interface List %d to L3 List %d: error code: %d\n",
                       __FUNCTION__, l2IfListId, l3IfListId, retVal);
                return retVal;
            }
            printf("%s: Added l2IfListId %d in l3IfListId %d\n", __FUNCTION__, l2IfListId,
                   l3IfListId);
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiAppConfigureIpv6RouteMcEntry(xpsDevice_t devId,
                                            sai_object_id_t switchOid, Ipv6RouteMcEntry *ipv6RouteMcData,
                                            uint32_t numOfMvifEntry, uint32_t *index)
{
    XP_STATUS retVal;
    uint32_t  i, j, entryNum = 0, intfId, l3IntfId;
    xpsHashIndexList_t indexList;
    xpsMcL2InterfaceListId_t l2IfListId;
    xpsMcL3InterfaceListId_t l3IfListId;
    xpsMulticastIPv6RouteEntry_t ipv6RouteMcEntry, getIpv6RouteMcEntry;
    xpMcastDomainId_t mcL3DomainId;

    if (!ipv6RouteMcData || !index)
    {
        printf("Error: Null pointer received for ipv6RouteMcData Entry \n");
        return XP_ERR_NULL_POINTER;
    }
    memset(&l3IfListId, 0, sizeof(l3IfListId));
    //Create L3 IntfList Id
    if ((retVal = xpsMulticastCreateL3InterfaceList(&l3IfListId)) != XP_NO_ERR)
    {
        printf("%s: Error in Creating L3Interface List: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }
    // Add L3 IntfList to Dev
    if ((retVal = xpsMulticastAddL3InterfaceListToDevice(devId,
                                                         l3IfListId)) != XP_NO_ERR)
    {
        printf("%s: Error in Adding L3Interface List %d to device %d: error code: %d\n",
               __FUNCTION__, devId, l3IfListId, retVal);
        return retVal;
    }

    l3IntfId = XPS_INTF_MAP_BD_TO_INTF(ipv6RouteMcData->rpfValue);
    memset(&mcL3DomainId, 0, sizeof(mcL3DomainId));
    if ((retVal = xpsL3GetV6McL3DomainIdForInterface(devId, l3IntfId,
                                                     &mcL3DomainId)) != XP_NO_ERR)
    {
        printf("%s: Error in Adding L3Interface List %d to device %d: error code: %d\n",
               __FUNCTION__, devId, l3IfListId, retVal);
        return retVal;
    }

    memset(&ipv6RouteMcEntry, 0, sizeof(xpsMulticastIPv6RouteEntry_t));
    ipv6RouteMcEntry.pktCmd = (xpMulticastPktCmdType_e) ipv6RouteMcData->pktCmd;
    ipv6RouteMcEntry.rpfType = (xpMulticastRpfCheckType_e) ipv6RouteMcData->rpfType;
    ipv6RouteMcEntry.rpfFailCmd = (xpMulticastRpfFailCmd_e)
                                  ipv6RouteMcData->rpfFailCmd;
    ipv6RouteMcEntry.multicastVifIdx = l3IfListId;
    ipv6RouteMcEntry.vrfIdx = vrfIdxToVrf[ipv6RouteMcData->vrfId];
    ipv6RouteMcEntry.ipv6McL3DomainId = mcL3DomainId;
    COPY_IPV6_ADDR_T(ipv6RouteMcEntry.sourceAddress, ipv6RouteMcData->sip);
    COPY_IPV6_ADDR_T(ipv6RouteMcEntry.groupAddress, ipv6RouteMcData->grpAddr);

#if 1
    if ((ipv6RouteMcEntry.rpfType == XP_MC_RPF_CHECK_TYPE_PORT) ||
        (ipv6RouteMcEntry.rpfType == XP_MC_RPF_CHECK_TYPE_PIMBIDIR ||
         (ipv6RouteMcEntry.rpfType == XP_MC_RPF_CHECK_TYPE_BD)))
    {
        ipv6RouteMcEntry.rpfValue = ipv6RouteMcData->rpfValue;
    }
    else if (ipv6RouteMcEntry.rpfType == XP_MC_RPF_CHECK_TYPE_VIF)
    {
        if ((retVal = xpsPortGetPortIntfId(devId, ipv6RouteMcData->rpfValue,
                                           &intfId)) != XP_NO_ERR)
        {
            printf("%s: Error in getting intfId for rpfVal(portNum) %d: error code: %d\n",
                   __FUNCTION__, ipv6RouteMcData->rpfValue, retVal);
            return retVal;
        }
        ipv6RouteMcEntry.rpfValue = intfId;
    }
#endif
    memset(&indexList, 0, sizeof(indexList));

    // Populate v6RouteMc Entry
    if ((retVal = xpsMulticastAddIPv6RouteEntry(devId, &ipv6RouteMcEntry,
                                                &indexList)) != XP_NO_ERR)
    {
        printf("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    printf("\naddIPv6MulticastRouteEntry -> sip : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d grpAddr : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d pktcmd : %d, vrfId: %d, mvif: %d\n",
           ipv6RouteMcEntry.sourceAddress[0], ipv6RouteMcEntry.sourceAddress[1],
           ipv6RouteMcEntry.sourceAddress[2], ipv6RouteMcEntry.sourceAddress[3],
           ipv6RouteMcEntry.sourceAddress[4], ipv6RouteMcEntry.sourceAddress[5],
           ipv6RouteMcEntry.sourceAddress[6], ipv6RouteMcEntry.sourceAddress[7],
           ipv6RouteMcEntry.sourceAddress[8], ipv6RouteMcEntry.sourceAddress[9],
           ipv6RouteMcEntry.sourceAddress[10], ipv6RouteMcEntry.sourceAddress[11],
           ipv6RouteMcEntry.sourceAddress[12], ipv6RouteMcEntry.sourceAddress[13],
           ipv6RouteMcEntry.sourceAddress[14], ipv6RouteMcEntry.sourceAddress[15],
           ipv6RouteMcEntry.groupAddress[0], ipv6RouteMcEntry.groupAddress[1],
           ipv6RouteMcEntry.groupAddress[2], ipv6RouteMcEntry.groupAddress[3],
           ipv6RouteMcEntry.groupAddress[4], ipv6RouteMcEntry.groupAddress[5],
           ipv6RouteMcEntry.groupAddress[6], ipv6RouteMcEntry.groupAddress[7],
           ipv6RouteMcEntry.groupAddress[8], ipv6RouteMcEntry.groupAddress[9],
           ipv6RouteMcEntry.groupAddress[10], ipv6RouteMcEntry.groupAddress[11],
           ipv6RouteMcEntry.groupAddress[12], ipv6RouteMcEntry.groupAddress[13],
           ipv6RouteMcEntry.groupAddress[14], ipv6RouteMcEntry.groupAddress[15],
           ipv6RouteMcEntry.pktCmd, ipv6RouteMcEntry.vrfIdx,
           ipv6RouteMcEntry.multicastVifIdx);

    *index = indexList.index[0];

    if (indexList.size > 1)
    {
        printf("!!! Rehash occured!!! NOT TAKING CARE AS OF NOW.\n");
        return XP_NO_ERR;
    }
    memset(&getIpv6RouteMcEntry, 0, sizeof(getIpv6RouteMcEntry));
    if ((retVal = xpsMulticastGetIPv6RouteEntry(devId, *index,
                                                &getIpv6RouteMcEntry)) != XP_NO_ERR)
    {
        printf("%s: Error in reading an entry: error code: %d\n", __FUNCTION__, retVal);
        return retVal;
    }

    printf("xpsMulticastGetIPv6RouteEntry-> sip : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d grpAddr : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d pktcmd : %d, mvif: %d, l3DomainId: %d\n",
           getIpv6RouteMcEntry.sourceAddress[0], getIpv6RouteMcEntry.sourceAddress[1],
           getIpv6RouteMcEntry.sourceAddress[2], getIpv6RouteMcEntry.sourceAddress[3],
           getIpv6RouteMcEntry.sourceAddress[4], getIpv6RouteMcEntry.sourceAddress[5],
           getIpv6RouteMcEntry.sourceAddress[6], getIpv6RouteMcEntry.sourceAddress[7],
           getIpv6RouteMcEntry.sourceAddress[8], getIpv6RouteMcEntry.sourceAddress[9],
           getIpv6RouteMcEntry.sourceAddress[10], getIpv6RouteMcEntry.sourceAddress[11],
           getIpv6RouteMcEntry.sourceAddress[12], getIpv6RouteMcEntry.sourceAddress[13],
           getIpv6RouteMcEntry.sourceAddress[14], getIpv6RouteMcEntry.sourceAddress[15],
           getIpv6RouteMcEntry.groupAddress[0], getIpv6RouteMcEntry.groupAddress[1],
           getIpv6RouteMcEntry.groupAddress[2], getIpv6RouteMcEntry.groupAddress[3],
           getIpv6RouteMcEntry.groupAddress[4], getIpv6RouteMcEntry.groupAddress[5],
           getIpv6RouteMcEntry.groupAddress[6], getIpv6RouteMcEntry.groupAddress[7],
           getIpv6RouteMcEntry.groupAddress[8], getIpv6RouteMcEntry.groupAddress[9],
           getIpv6RouteMcEntry.groupAddress[10], getIpv6RouteMcEntry.groupAddress[11],
           getIpv6RouteMcEntry.groupAddress[12], getIpv6RouteMcEntry.groupAddress[13],
           getIpv6RouteMcEntry.groupAddress[14], getIpv6RouteMcEntry.groupAddress[15],
           getIpv6RouteMcEntry.pktCmd, getIpv6RouteMcEntry.multicastVifIdx,
           getIpv6RouteMcEntry.ipv6McL3DomainId);

    /* Iterate through all mvifs for all vlans in one L3 Intf*/
    for (i = 0; i < 10; i++)
    {
        if (ipv6RouteMcData->vlanNode[i].vlanId == 0)
        {
            break;
        }
        //Create L2 IntfList for Given Vlan
        memset(&l2IfListId, 0, sizeof(l2IfListId));
        if ((retVal = xpsMulticastCreateL2InterfaceList(
                          ipv6RouteMcData->vlanNode[i].vlanId, &l2IfListId)) != XP_NO_ERR)
        {
            printf("%s: Error in Creating Interface List for vlan: %d: error code: %d\n",
                   __FUNCTION__, ipv6RouteMcData->vlanNode[i].vlanId, retVal);
            return retVal;
        }
        printf("%s: Created Interface List %d for vlan %d\n", __FUNCTION__, l2IfListId,
               ipv6RouteMcData->vlanNode[i].vlanId);
        //Add L2 IntfList to Dev
        if ((retVal = xpsMulticastAddL2InterfaceListToDevice(devId,
                                                             l2IfListId)) != XP_NO_ERR)
        {
            printf("%s: Error in Adding Interface List %d to device %d: error code: %d\n",
                   __FUNCTION__, devId, l2IfListId, retVal);
            return retVal;
        }
        /* Populate mvif entries */
        for (j = 0; j < 10; j++)
        {
            if (ipv6RouteMcData->vlanNode[i].nodeIdx[j] == 0)
            {
                break;
            }
            for (entryNum = 0; entryNum < numOfMvifEntry; entryNum++)
            {
                if (((MvifEntry *)getEntry(&configFile.mvifData,
                                           entryNum))->index == ipv6RouteMcData->vlanNode[i].nodeIdx[j])
                {
                    //add Intf to L2 IntfList
                    xpSaiAppConfigureMvifEntry(devId, (MvifEntry *)getEntry(&configFile.mvifData,
                                                                            entryNum), l3IfListId, l2IfListId, 0);
                    break;
                }
            }
        }
        if (((MvifEntry *)getEntry(&configFile.mvifData, entryNum))->tnlType!= 9)
        {
            //Add L2 IntfList to L3 IntfList
            if ((retVal = xpsMulticastAddBridgingInterfaceToL3InterfaceList(devId,
                                                                            l3IfListId, l2IfListId)) != XP_NO_ERR)
            {
                printf("%s: Error in Adding Bridge Interface List %d to L3 List %d: error code: %d\n",
                       __FUNCTION__, l2IfListId, l3IfListId, retVal);
                return retVal;
            }
            printf("%s: Added l2IfListId %d in l3IfListId %d\n", __FUNCTION__, l2IfListId,
                   l3IfListId);
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiAppConfigureIpv4PimBiDirEntry(xpsDevice_t devId,
                                             sai_object_id_t switchOid, Ipv4PimBiDirEntry *ipv4PimBiDirData, uint32_t *index)
{
    XP_STATUS retVal;
    xpsHashIndexList_t indexList;
    xpIPv4MulticastPimBidirRpfEntry_t ipv4PimBiDirEntry;// getIpv4PimBiDirEntry;

    if (!ipv4PimBiDirData || !index)
    {
        printf("Error: Null pointer received for ipv4PimBiDirData Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    memset(&ipv4PimBiDirEntry, 0, sizeof(xpIPv4MulticastPimBidirRpfEntry_t));
    ipv4PimBiDirEntry.bdId = ipv4PimBiDirData->bdId;
    ipv4PimBiDirEntry.vrfIdx = vrfIdxToVrf[ipv4PimBiDirData->vrfId];
    ipv4PimBiDirEntry.rpfValue = ipv4PimBiDirData->rpfValue;
    COPY_IPV4_ADDR_T(ipv4PimBiDirEntry.groupAddress, ipv4PimBiDirData->grpAddr);

    memset(&indexList, 0, sizeof(indexList));
    if ((retVal = xpsMulticastAddIPv4PimBiDirRPFEntry(devId, &ipv4PimBiDirEntry,
                                                      &indexList)) != XP_NO_ERR)
    {
        printf("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    printf("\naddIPv4PimBiDirEntry -> grpAddr : %d.%d.%d.%d, vrfId: %d, bdId: %d, rpfValue : %d\n",
           ipv4PimBiDirEntry.groupAddress[3], ipv4PimBiDirEntry.groupAddress[2],
           ipv4PimBiDirEntry.groupAddress[1], ipv4PimBiDirEntry.groupAddress[0],
           ipv4PimBiDirEntry.vrfIdx, ipv4PimBiDirEntry.bdId, ipv4PimBiDirEntry.rpfValue);

    *index = indexList.index[0];

    if (indexList.size > 1)
    {
        printf("!!! Rehash occured!!! NOT TAKING CARE AS OF NOW.\n");
        return XP_NO_ERR;
    }
    /*
        if ((retVal = xpGetIPv4MulticastPimBidirRpfEntry(devId, *index, &getIpv4PimBiDirEntry)) != XP_NO_ERR)
        {
            printf("%s: Error in reading an entry: error code: %d\n", __FUNCTION__, retVal);
            return retVal;
        }
        printf("getIPv4PimBiDirEntry -> grpAddr : %d.%d.%d.%d, vrfId: %d, bdId: %d, rpfVal : %d\n\n\n", getIpv4PimBiDirEntry.groupAddress[3], getIpv4PimBiDirEntry.groupAddress[2], getIpv4PimBiDirEntry.groupAddress[1], getIpv4PimBiDirEntry.groupAddress[0], getIpv4PimBiDirEntry.vrfIdx, getIpv4PimBiDirEntry.bdId, getIpv4PimBiDirEntry.rpfValue);
    */
    return XP_NO_ERR;
}

XP_STATUS xpSaiAppConfigureMacSAMSBsEntry(xpsDevice_t devId,
                                          sai_object_id_t switchOid, uint8_t *macSAMSBs)
{
    /* Program the High32 bits of RouterMAC */
    macAddrHigh_t macHigh;
    XP_STATUS retVal = XP_NO_ERR;
    sai_attribute_t attr;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    memset(attr.value.mac, 0, sizeof(sai_mac_t));
    macHigh[0] = macSAMSBs[0];
    macHigh[1] = macSAMSBs[1];
    macHigh[2] = macSAMSBs[2];
    macHigh[3] = macSAMSBs[3];
    macHigh[4] = macSAMSBs[4];

    attr.id = SAI_SWITCH_ATTR_SRC_MAC_ADDRESS;
    memcpy(attr.value.mac, macHigh, sizeof(macAddrHigh_t));
    xpSaiAppMacReverse(attr.value.mac);

    if ((saiRetVal = xpSaiSwitchApi->set_switch_attribute(switchOid,
                                                          &attr)) != SAI_STATUS_SUCCESS)
    {
        printf("Error : set_router_switch_attribute failed for sw : %d\n",
               (uint32_t)xpSaiObjIdValueGet(switchOid));
        return saiStatus2XpsStatus(saiRetVal);
    }

    return retVal;
}
//TODO
#if 0
XP_STATUS xpSaiAppConfigureIacl(xpsDevice_t devId, sai_object_id_t switchOid)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t count = 0;
    uint32_t tmpCount = 0;
    uint32_t /*intfIdInPort = 0,*/ intfIdOutPort = 0;
    uint32_t numOfIaclDataEntries = 0;
    uint32_t i = 0, j = 0, vlan = 0;
    uint32_t tnlIdx = 0;

    bool etherType = false;
    bool cTagVid = false;
    bool cTagDei = false;
    bool cTagPcp = false;
    bool sTagVid = false;
    bool sTagDei = false;
    bool sTagPcp = false;
    bool L4DestPort = false;
    bool L4SourcePort = false;
    bool protocol = false;
    bool isDIPSet = false;
    bool isSIPSet = false;
    bool isDmacSet = false;
    bool isSmacSet = false;

    sai_object_id_t acl_tid[5]= { 0, 0, 0, 0, 0};
    sai_object_id_t pAcl_entry = 0ULL, bAcl_entry = 0ULL, rAcl_entry = 0ULL;
    sai_attribute_t *aclAttrList = (sai_attribute_t*)malloc(sizeof(
                                                                sai_attribute_t)*MAX_ACL_TABLE_ATTR);
    if (NULL == aclAttrList)
    {
        printf("%s:, ERROR: Failed to allocate memory for aclAttrList \n",
               __FUNCTION__);
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    memset(aclAttrList, 0, sizeof(sai_attribute_t)*MAX_ACL_TABLE_ATTR);

    xpsIaclData_t *iaclDataEntry;
    xpIaclTableProfile_t tblProfile;
    xpIaclkeyFieldList_t fldData, ruleData;

    printf("iACL : devId:%d \n", devId);

    IaclL2Ipv4KeyAttribute *iaclKey;
    IaclL2Ipv4MaskAttribute *iaclMask;
    IaclData *iaclData;

    memset(&tblProfile, 0x00, sizeof(xpIaclTableProfile_t));
    memset(&fldData, 0x00, sizeof(xpIaclkeyFieldList_t));
    memset(&ruleData, 0x00, sizeof(xpIaclkeyFieldList_t));

    numOfIaclDataEntries = numOfEntry(&configFile.iaclData);

    if (numOfIaclDataEntries == 0)
    {
        free(aclAttrList);
        return XP_NO_ERR;
    }

    if (numOfIaclDataEntries > MAX_ACL_TABLE_ATTR)
    {
        /* more entries than SAI suppoorts */
        printf("iACL ERROR, attempt to configure %d entries, max supported are %d\n",
               numOfIaclDataEntries, MAX_ACL_TABLE_ATTR);
        free(aclAttrList);
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    /* create acl table with Ingress stage as it is iacl table creation */
    aclAttrList[count].id = SAI_ACL_TABLE_ATTR_ACL_STAGE;
    aclAttrList[count++].value.s32 = SAI_ACL_STAGE_INGRESS;

    for (i = 0; i < numOfIaclDataEntries; i++)
    {
        iaclKey = (IaclL2Ipv4KeyAttribute *)getEntry(&configFile.iaclKey, i);

        if (!isDmacSet && iaclKey->isDmacSet)
        {
            isDmacSet = true;
            // dmac attribute for pAcl create table
            aclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
            aclAttrList[count++].value.booldata = true;
        }

        if (!isSmacSet && iaclKey->isSmacSet)
        {
            isSmacSet = true;
            aclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
            aclAttrList[count++].value.booldata = true;
        }

        if (!etherType  && iaclKey->etherType != -1)
        {
            etherType = true;
            aclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
            aclAttrList[count++].value.booldata = true;
        }

        if (!cTagVid  && iaclKey->cTagVid != -1)
        {
            cTagVid = true;
            aclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID;
            aclAttrList[count++].value.booldata = true;
        }

        if (!cTagDei  && iaclKey->cTagDei != -1)
        {
            cTagDei = true;
            aclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI;
            aclAttrList[count++].value.booldata = true;
        }

        if (!cTagPcp  && iaclKey->cTagPcp != -1)
        {
            cTagPcp = true;
            aclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI;
            aclAttrList[count++].value.booldata = true;
        }

        if (!sTagVid  && iaclKey->sTagVid != -1)
        {
            sTagVid = true;
            aclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID;
            aclAttrList[count++].value.booldata = true;
        }

        if (!sTagDei  && iaclKey->sTagDei != -1)
        {
            sTagDei = true;
            aclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI;
            aclAttrList[count++].value.booldata = true;
        }

        if (!sTagPcp  && iaclKey->sTagPcp != -1)
        {
            sTagPcp = true;
            aclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI;
            aclAttrList[count++].value.booldata = true;
        }

        if (!isDIPSet && iaclKey->isDIPSet)
        {
            isDIPSet = true;
            aclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_DST_IP;
            aclAttrList[count++].value.booldata = true;
        }

        if (!isSIPSet && iaclKey->isSIPSet)
        {
            isSIPSet = true;
            aclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_IP;
            aclAttrList[count++].value.booldata = true;
        }

        if (!protocol  && iaclKey->protocol != -1)
        {
            protocol = true;
            aclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
            aclAttrList[count++].value.booldata = true;
        }

        if (!L4SourcePort  && iaclKey->L4SourcePort != -1)
        {
            L4SourcePort = true;
            aclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT;
            aclAttrList[count++].value.booldata = true;
        }

        if (!L4DestPort  && iaclKey->L4DestPort != -1)
        {
            L4DestPort = true;
            aclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT;
            aclAttrList[count++].value.booldata = true;
        }
    }

    aclAttrList[count].id = SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST;
    aclAttrList[count].value.s32list.count = 1;
    aclAttrList[count].value.s32list.list = (int32_t *)malloc(sizeof(sai_int32_t));
    aclAttrList[count].value.s32list.list[0] = SAI_ACL_BIND_POINT_TYPE_PORT;
    tmpCount =
        count; //count value incremented later in code. Need to store allocated count value to free it later.

    if ((saiRetVal = xpSaiAclApi->create_acl_table(&acl_tid[0], switchOid, count+1,
                                                   aclAttrList)) != SAI_STATUS_SUCCESS)
    {
        printf(" create Table returns %d :\n", saiRetVal);
        printf("%s:, ERROR: Failed to createTable \n", __FUNCTION__);
        free(aclAttrList[tmpCount].value.s32list.list);
        free(aclAttrList);
        return saiStatus2XpsStatus(saiRetVal);
    }

    aclAttrList[count].value.s32list.list[0] = SAI_ACL_BIND_POINT_TYPE_VLAN;
    if ((saiRetVal = xpSaiAclApi->create_acl_table(&acl_tid[1], switchOid, count+1,
                                                   aclAttrList)) != SAI_STATUS_SUCCESS)
    {
        printf(" create Table returns %d :\n", saiRetVal);
        printf("%s:, ERROR: Failed to createTable \n", __FUNCTION__);
        free(aclAttrList[tmpCount].value.s32list.list);
        free(aclAttrList);
        return saiStatus2XpsStatus(saiRetVal);
    }

    aclAttrList[count].value.s32list.list[0] = SAI_ACL_BIND_POINT_TYPE_ROUTER_INTF;
    if ((saiRetVal = xpSaiAclApi->create_acl_table(&acl_tid[2], switchOid, count+1,
                                                   aclAttrList)) != SAI_STATUS_SUCCESS)
    {
        printf(" create Table returns %d :\n", saiRetVal);
        printf("%s:, ERROR: Failed to createTable \n", __FUNCTION__);
        free(aclAttrList[tmpCount].value.s32list.list);
        free(aclAttrList);
        return saiStatus2XpsStatus(saiRetVal);
    }

    aclAttrList[count].value.s32list.list[0] = SAI_ACL_BIND_POINT_TYPE_PORT;
    if ((saiRetVal = xpSaiAclApi->create_acl_table(&acl_tid[3], switchOid, count+1,
                                                   aclAttrList)) != SAI_STATUS_SUCCESS)
    {
        printf(" create Table returns %d :\n", saiRetVal);
        printf("%s:, ERROR: Failed to createTable \n", __FUNCTION__);
        free(aclAttrList[tmpCount].value.s32list.list);
        free(aclAttrList);
        return saiStatus2XpsStatus(saiRetVal);
    }

    aclAttrList[count].value.s32list.list[0] = SAI_ACL_BIND_POINT_TYPE_VLAN;
    if ((saiRetVal = xpSaiAclApi->create_acl_table(&acl_tid[4], switchOid, count+1,
                                                   aclAttrList)) != SAI_STATUS_SUCCESS)
    {
        printf(" create Table returns %d :\n", saiRetVal);
        printf("%s:, ERROR: Failed to createTable \n", __FUNCTION__);
        free(aclAttrList[tmpCount].value.s32list.list);
        free(aclAttrList);
        return saiStatus2XpsStatus(saiRetVal);
    }
    free(aclAttrList[tmpCount].value.s32list.list);

    for (i = 0; i < numOfIaclDataEntries; i++)
    {
        iaclDataEntry = (xpIaclData_t *)malloc(sizeof(xpIaclData_t));
        memset(iaclDataEntry, 0, sizeof(xpIaclData_t));

        /* Copy field data from reference key details struct and fill up values */
        memcpy(&ruleData, &fldData, sizeof(xpIaclkeyFieldList_t));

        iaclData = (IaclData *)getEntry(&configFile.iaclData, i);
        iaclKey = (IaclL2Ipv4KeyAttribute *)getEntry(&configFile.iaclKey, i);
        iaclMask = (IaclL2Ipv4MaskAttribute *)getEntry(&configFile.iaclMask, i);
        vlan = iaclData->vlan;

        printf("----------------Port Num : %d | iAclId: %d ----------------------\n",
               iaclData->portNum, iaclKey->iAclId);
        //
        //Set IACL entry
        //

        // creating port object for portNum and eVifId
        sai_object_id_t /*iPort = 0,*/ ePort = 0;
        count = 0;

        if (iaclKey->priority != -1)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
            aclAttrList[count].value.u32 = iaclKey->priority;
            count++;
        }

        if (iaclKey->isDmacSet)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
            aclAttrList[count].value.aclfield.enable =true;
            memcpy(aclAttrList[count].value.aclfield.data.mac, iaclKey->macDA,
                   sizeof(sai_mac_t));
            memcpy(aclAttrList[count].value.aclfield.mask.mac, iaclMask->macDA,
                   sizeof(sai_mac_t));
            // reversing the mac order as config file is parsed by reverse mac addr
            // User suppose to give mac addr in straight order
            xpSaiAppMacReverse(aclAttrList[count].value.aclfield.data.mac);
            xpSaiAppMacReverse(aclAttrList[count].value.aclfield.mask.mac);
            xpSaiNegMask((sai_uint8_t*)&aclAttrList[count].value.aclfield.mask.mac,
                         sizeof(sai_mac_t));
            count++;
        }

        if (iaclKey->isSmacSet)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC;
            aclAttrList[count].value.aclfield.enable =true;
            memcpy(aclAttrList[count].value.aclfield.data.mac, iaclKey->macSA,
                   sizeof(sai_mac_t));
            memcpy(aclAttrList[count].value.aclfield.mask.mac, iaclMask->macSA,
                   sizeof(sai_mac_t));
            // reversing the mac order as config file is parsed by reverse mac addr
            // User suppose to give mac addr in straight order
            xpSaiAppMacReverse(aclAttrList[count].value.aclfield.data.mac);
            xpSaiAppMacReverse(aclAttrList[count].value.aclfield.mask.mac);
            xpSaiNegMask((sai_uint8_t*)&aclAttrList[count].value.aclfield.mask.mac,
                         sizeof(sai_mac_t));
            count++;
        }

        if (iaclKey->etherType != -1)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE;
            aclAttrList[count].value.aclfield.enable =true;
            aclAttrList[count].value.aclfield.data.u16 = iaclKey->etherType;
            aclAttrList[count].value.aclfield.mask.u16 = iaclMask->etherType;
            xpSaiNegMask((sai_uint8_t*)&aclAttrList[count].value.aclfield.mask.u16,
                         sizeof(sai_uint16_t));
            count++;
        }

        if (iaclKey->cTagVid != -1)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID;
            aclAttrList[count].value.aclfield.enable =true;
            aclAttrList[count].value.aclfield.data.u16 = iaclKey->cTagVid;
            aclAttrList[count].value.aclfield.mask.u16 = iaclMask->cTagVid;
            xpSaiNegMask((sai_uint8_t*)&aclAttrList[count].value.aclfield.mask.u16,
                         sizeof(sai_uint16_t));
            count++;
        }

        if (iaclKey->cTagPcp != -1)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI;
            aclAttrList[count].value.aclfield.enable =true;
            aclAttrList[count].value.aclfield.data.u8 = iaclKey->cTagPcp;
            aclAttrList[count].value.aclfield.mask.u8 = iaclMask->cTagPcp;
            xpSaiNegMask(&aclAttrList[count].value.aclfield.mask.u8, sizeof(sai_uint8_t));
            count++;
        }

        if (iaclKey->cTagDei != -1)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI;
            aclAttrList[count].value.aclfield.enable =true;
            aclAttrList[count].value.aclfield.data.u8 = iaclKey->cTagDei;
            aclAttrList[count].value.aclfield.mask.u8 = iaclMask->cTagDei;
            xpSaiNegMask((sai_uint8_t*)&aclAttrList[count].value.aclfield.mask.u8,
                         sizeof(sai_uint8_t));
            count++;
        }

        if (iaclKey->sTagVid != -1)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID;
            aclAttrList[count].value.aclfield.data.u16 = iaclKey->sTagVid;
            aclAttrList[count].value.aclfield.mask.u16 = iaclMask->sTagVid;
            xpSaiNegMask((sai_uint8_t*)&aclAttrList[count].value.aclfield.mask.u16,
                         sizeof(sai_uint16_t));
            count++;
        }

        if (iaclKey->sTagPcp != -1)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI;
            aclAttrList[count].value.aclfield.enable =true;
            aclAttrList[count].value.aclfield.data.u8 = iaclKey->sTagPcp;
            aclAttrList[count].value.aclfield.mask.u8 = iaclMask->sTagPcp;
            xpSaiNegMask((sai_uint8_t*)&aclAttrList[count].value.aclfield.mask.u8,
                         sizeof(sai_uint8_t));
            count++;
        }

        if (iaclKey->sTagDei != -1)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI;
            aclAttrList[count].value.aclfield.enable =true;
            aclAttrList[count].value.aclfield.data.u8 = iaclKey->sTagDei;
            aclAttrList[count].value.aclfield.mask.u8 = iaclMask->sTagDei;
            xpSaiNegMask(&aclAttrList[count].value.aclfield.mask.u8, sizeof(sai_uint8_t));
            count++;
        }

        sai_ip4_t ipv4AddrKey, ipv4AddrMask;
        uint32_t len = sizeof(sai_ip4_t);
        if (iaclKey->isDIPSet)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IP;

            for (j = 0; j < len; ++j)
            {
                ((uint8_t*)&ipv4AddrKey)[len-1-j] = iaclKey->DIP[j];
                ((uint8_t*)&ipv4AddrMask)[len-1-j] = iaclMask->DIP[j];
            }

            aclAttrList[count].value.aclfield.enable =true;
            aclAttrList[count].value.aclfield.data.ip4 = ipv4AddrKey;
            aclAttrList[count].value.aclfield.mask.ip4 = ipv4AddrMask;
            xpSaiNegMask((sai_uint8_t*)&aclAttrList[count].value.aclfield.mask.ip4,
                         sizeof(sai_ip4_t));
            count++;
        }

        if (iaclKey->isSIPSet)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP;

            for (j = 0; j < len; ++j)
            {
                ((uint8_t*)&ipv4AddrKey)[len-1-j] = iaclKey->SIP[j];
                ((uint8_t*)&ipv4AddrMask)[len-1-j] = iaclMask->SIP[j];
            }

            aclAttrList[count].value.aclfield.enable =true;
            aclAttrList[count].value.aclfield.data.ip4 = ipv4AddrKey;
            aclAttrList[count].value.aclfield.mask.ip4 = ipv4AddrMask;
            xpSaiNegMask((sai_uint8_t*)&aclAttrList[count].value.aclfield.mask.ip4,
                         sizeof(sai_ip4_t));
            count++;
        }

        if (iaclKey->L4DestPort != -1)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
            aclAttrList[count].value.aclfield.enable =true;
            aclAttrList[count].value.aclfield.data.u16 = htole32(iaclKey->L4DestPort);
            aclAttrList[count].value.aclfield.mask.u16 = htole32(iaclMask->L4DestPort);
            xpSaiNegMask((sai_uint8_t*)&aclAttrList[count].value.aclfield.mask.ip4,
                         sizeof(sai_ip4_t));
            count++;
        }

        if (iaclKey->L4SourcePort != -1)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT;
            aclAttrList[count].value.aclfield.enable =true;
            aclAttrList[count].value.aclfield.data.u32 = iaclKey->L4SourcePort;
            aclAttrList[count].value.aclfield.mask.u32 = iaclMask->L4SourcePort;
            xpSaiNegMask((sai_uint8_t*)&aclAttrList[count].value.aclfield.mask.ip4,
                         sizeof(sai_ip4_t));
            count++;
        }

        if (iaclKey->icmpMessageType != -1)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE;
            aclAttrList[count].value.aclfield.enable =true;
            aclAttrList[count].value.aclfield.data.u8 = iaclKey->icmpMessageType;
            aclAttrList[count].value.aclfield.mask.u8 = iaclMask->icmpMessageType;
            xpSaiNegMask(&aclAttrList[count].value.aclfield.mask.u8, sizeof(sai_uint8_t));
            count++;
        }

        if (iaclKey->protocol != -1)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
            aclAttrList[count].value.aclfield.enable =true;
            aclAttrList[count].value.aclfield.data.u8 = iaclKey->protocol;
            aclAttrList[count].value.aclfield.mask.u8 = iaclMask->protocol;
            xpSaiNegMask(&aclAttrList[count].value.aclfield.mask.u8, sizeof(sai_uint8_t));
            count++;
        }

        //
        //Create IACL data in form of SAI attrubutes
        //
        iaclDataEntry->iAclDataType.aclData.enRedirectToEvif =
            iaclData->enRedirectToEvif;
        iaclDataEntry->iAclDataType.aclData.encapType = iaclData->encapType;
        tnlIdx = iaclData->eVifId;

        retVal = xpsPortGetPortIntfId(devId, iaclData->eVifId, &intfIdOutPort);
        if (retVal)
        {
            printf("Error : xpsPortGetPortIntfId() Failed for eVifId :%d devId %d\n",
                   iaclData->eVifId, devId);
            free(iaclDataEntry);
            free(aclAttrList);
            return retVal;
        }

        if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                          (sai_uint64_t)intfIdOutPort, &ePort)) != SAI_STATUS_SUCCESS)
        {
            printf("Error : SAI object can not be created.\n");
            free(iaclDataEntry);
            free(aclAttrList);
            return saiStatus2XpsStatus(saiRetVal);
        }

        if (iaclData->enRedirectToEvif == 1)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
            aclAttrList[count].value.aclaction.enable =true;
            if (iaclData->encapType == XP_IACL_ENCAP_TAGGED ||
                iaclData->encapType == XP_IACL_ENCAP_UNTAGGED) // Non Tunnel Port
            {
                aclAttrList[count++].value.aclaction.parameter.oid = ePort;
                printf("\niaclDataEntry->egrVifId : %d\n",
                       iaclDataEntry->iAclDataType.aclData.egrVifIdOrRsnCode.eVifId);
            }
            else if (iaclData->encapType == XP_IACL_ENCAP_IP_TUNNEL) // IP Tunnel Port
            {
                intfIdOutPort = tnlVif[devId][tnlIdx];
                if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                                  (sai_uint64_t)intfIdOutPort, &ePort)) != SAI_STATUS_SUCCESS)
                {
                    printf("Error : SAI object can not be created.\n");
                    free(iaclDataEntry);
                    free(aclAttrList);
                    return saiStatus2XpsStatus(saiRetVal);
                }
                aclAttrList[count++].value.aclaction.parameter.oid = ePort;
            }
            else
            {
                aclAttrList[count++].value.aclaction.parameter.oid = ePort;
            }
        }

        if (iaclData->remarkDscp)
        {
            // putting dscp remark value which should assign to enDscpRemarkOrVrfUpd in adapter
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP;
            aclAttrList[count].value.aclaction.enable =true;
            aclAttrList[count++].value.aclaction.parameter.u8 = iaclData->remarkDscp;
            //iaclDataEntry->iAclDataType.aclData.enDscpRemarkOrVrfUpd = iaclData->remarkDscp;
        }

        if (iaclData->encapType != -1)
        {
            // putting encaptype will be used in REDIRECT and using as metadeta
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_ACL_META_DATA;
            aclAttrList[count].value.aclaction.enable =true;
            aclAttrList[count++].value.aclaction.parameter.u32 = iaclData->encapType;

        }
        iaclDataEntry->iAclDataType.aclData.enPcpRemarkOrStagRw = iaclData->remarkPcp;
        iaclDataEntry->iAclDataType.aclData.enDpRemark = iaclData->remarkDp;
        iaclDataEntry->iAclDataType.aclData.dp = iaclData->DP;

        if (iaclData->remarkTc)
        {
            // enable the remarkTc in sai adapter when tc field is provided
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TC;
            aclAttrList[count].value.aclaction.enable =true;
            aclAttrList[count++].value.aclaction.parameter.u8 = iaclData->TC;
        }
        if (iaclData->rsnCode)
        {
            // enable the remarkTc in sai adapter when reason code field is provided
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_USER_TRAP_ID;
            aclAttrList[count].value.aclaction.enable =true;
            aclAttrList[count++].value.aclaction.parameter.u32 = iaclData->rsnCode;
        }
        iaclDataEntry->iAclDataType.aclData.enMirrorSsnUpd = iaclData->enMirrorSsnUpd;

        //TODO: aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS;
        iaclDataEntry->iAclDataType.aclData.mirrorSessionId = iaclData->mirrorSessionId;

        printf("iaclDataEntry->rsnCode : %d \n",
               iaclDataEntry->iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode);
        printf("iaclDataEntry->egrVifIdOrRsnCode.eVifId : %d \n",
               iaclDataEntry->iAclDataType.aclData.egrVifIdOrRsnCode.eVifId);
        printf("iaclDataEntry->enPktCmdUpd : %d \n",
               iaclDataEntry->iAclDataType.aclData.enPktCmdUpd);
        printf("iaclDataEntry->pktCmd : %d \n",
               iaclDataEntry->iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command);
        printf("iaclDataEntry->isTerminal : %d \n",
               iaclDataEntry->iAclDataType.aclData.isTerminal);
        printf("iaclDataEntry->qosOrValnRw : %d \n",
               iaclDataEntry->iAclDataType.aclData.qosOrVlanRw);
        printf("iaclDataEntry->enDscpRemarkOrVrfUpd : %d \n",
               iaclDataEntry->iAclDataType.aclData.enDscpRemarkOrVrfUpd);
        printf("iaclDataEntry->enPcpRemarkOrStagRw : %d \n",
               iaclDataEntry->iAclDataType.aclData.enPcpRemarkOrStagRw);
        printf("iaclDataEntry-> enDpRemark : %d \n",
               iaclDataEntry->iAclDataType.aclData.enDpRemark);
        printf("iaclDataEntry->DP : %d \n", iaclDataEntry->iAclDataType.aclData.dp);
        printf("iaclDataEntry->enTcRemark : %d \n",
               iaclDataEntry->iAclDataType.aclData.enTcRemark);
        printf("iaclDataEntry->tc : %d \n", iaclDataEntry->iAclDataType.aclData.tc);
        printf("iaclDataEntry->enMirrorSsnUpd : %d \n",
               iaclDataEntry->iAclDataType.aclData.enMirrorSsnUpd);
        printf("iaclDataEntry->mirrorSessionId : %d \n",
               iaclDataEntry->iAclDataType.aclData.mirrorSessionId);

        if (iaclData->pktCmdOrEcmpSize != -1)
        {
            aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
            aclAttrList[count].value.aclaction.enable =true;
            switch (iaclData->pktCmdOrEcmpSize)
            {
                case 0://DROP
                    aclAttrList[count++].value.aclaction.parameter.s32 = SAI_PACKET_ACTION_DROP;
                    break;
                case 1://FORWARD
                    aclAttrList[count++].value.aclaction.parameter.s32 = SAI_PACKET_ACTION_FORWARD;
                    break;
                case 2://TRAP_TO_CPU
                    aclAttrList[count++].value.aclaction.parameter.s32 = SAI_PACKET_ACTION_COPY;
                    break;
                case 3://FORWARD_AND_MIRROR_TO_CPU
                    aclAttrList[count++].value.aclaction.parameter.s32 = SAI_PACKET_ACTION_LOG;
                    break;
                default:
                    printf("%s:, ERROR: Invalid acl action \n", __FUNCTION__);
                    break;
            }
        }

        switch (iaclData->type)
        {
            case 0: /* PACL */
                {
                    printf("iaclOnPort\n");
                    uint32_t intfId;
                    sai_attribute_t aclAttrPort;
                    sai_object_id_t portOid;

                    aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
                    aclAttrList[count++].value.oid = acl_tid[iaclKey->tableId];
                    xpSaiAclApi->create_acl_entry(&pAcl_entry, switchOid, count, aclAttrList);

                    retVal = xpsPortGetPortIntfId(devId, iaclData->portNum, &intfId);
                    if (retVal)
                    {
                        printf("Error : xpsPortGetPortIntfId() Failed for interfac type :%d devId %d\n",
                               XPS_PORT, devId);
                        free(iaclDataEntry);
                        free(aclAttrList);
                        return retVal;
                    }
                    if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                                      (sai_uint64_t)intfId, &portOid)) != SAI_STATUS_SUCCESS)
                    {
                        printf("Error : SAI object can not be created.\n");
                        free(iaclDataEntry);
                        free(aclAttrList);
                        return saiStatus2XpsStatus(saiRetVal);
                    }
                    aclAttrPort.id = SAI_PORT_ATTR_INGRESS_ACL;
                    aclAttrPort.value.oid = acl_tid[iaclKey->tableId];
                    if ((saiRetVal = xpSaiPortApi->set_port_attribute(portOid,
                                                                      &aclAttrPort)) != SAI_STATUS_SUCCESS)
                    {
                        printf("Error : set_port_attribute failed for port : %d\n",
                               (uint32_t)xpSaiObjIdValueGet(portOid));
                        free(iaclDataEntry);
                        free(aclAttrList);
                        return saiStatus2XpsStatus(saiRetVal);
                    }
                    break;
                }

            case 1: /* BACL */
                {
                    printf("iaclOnPort-Vlan\n");
                    sai_attribute_t aclAttrVlan;
                    sai_object_id_t vlanOid;

                    aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
                    aclAttrList[count++].value.oid = acl_tid[iaclKey->tableId];
                    xpSaiAclApi->create_acl_entry(&bAcl_entry, switchOid, count, aclAttrList);

                    if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, devId,
                                                      (sai_uint64_t)vlan, &vlanOid)) != SAI_STATUS_SUCCESS)
                    {
                        printf("Error : SAI object can not be created.\n");
                        free(iaclDataEntry);
                        free(aclAttrList);
                        return saiStatus2XpsStatus(saiRetVal);
                    }
                    aclAttrVlan.id = SAI_VLAN_ATTR_INGRESS_ACL;
                    aclAttrVlan.value.oid = acl_tid[iaclKey->tableId];
                    if ((saiRetVal = xpSaiVlanApi->set_vlan_attribute(vlanOid,
                                                                      &aclAttrVlan)) != SAI_STATUS_SUCCESS)
                    {
                        printf("Error : set_vlan_attribute failed for vlan : %d\n",
                               (uint32_t)xpSaiObjIdValueGet(vlanOid));
                        free(iaclDataEntry);
                        free(aclAttrList);
                        return saiStatus2XpsStatus(saiRetVal);
                    }

                    break;
                }
            case 2: /* RACL */
                {
                    printf("iaclOnRoute\n");
                    uint32_t l3IntfId = XPS_INTF_MAP_BD_TO_INTF(vlan);
                    sai_attribute_t aclAttrRif;
                    sai_object_id_t rifOid;

                    aclAttrList[count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
                    aclAttrList[count++].value.oid = acl_tid[iaclKey->tableId];
                    xpSaiAclApi->create_acl_entry(&rAcl_entry, switchOid, count, aclAttrList);

                    rifOid = xpSaiAppSaiGetVlanRifOid(l3IntfId);

                    aclAttrRif.id = SAI_ROUTER_INTERFACE_ATTR_INGRESS_ACL;
                    aclAttrRif.value.oid = acl_tid[iaclKey->tableId];
                    if ((saiRetVal = xpSaiRouterInterfaceApi->set_router_interface_attribute(rifOid,
                                                                                             &aclAttrRif)) != SAI_STATUS_SUCCESS)
                    {
                        printf("Error : set_router_interface_attribute failed for rif : %d\n",
                               (uint32_t)xpSaiObjIdValueGet(rifOid));
                        free(iaclDataEntry);
                        free(aclAttrList);
                        return saiStatus2XpsStatus(saiRetVal);
                    }

                    break;
                }
            default:
                {
                    printf(", ERROR: Invalid : ACL Type\n");
                    break;
                }
        }
        free(iaclDataEntry);
    }
    return retVal;
}

XP_STATUS xpSaiAppConfigureEacl(xpDevice_t devId, sai_object_id_t switchOid)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t numOfEaclDataEntries = 0;
    uint32_t i= 0;
    uint32_t tmpV6Index = 0;
    uint8_t kefIndex = 0;
    uint8_t tmpTypeMask;
    uint8_t tmpPcpMask, tmpPcpData;
    uint8_t count = 0;
    uint8_t tmpCount = 0;
    int32_t *tmpPtr;
    xpsEaclData_t *eaclDataEntry;
    xpEaclkeyFieldList_t v6fldData, v6ruleData;
    uint32_t tmpUintArray[30];

    bool L4DestPort = false;
    bool L4SourcePort = false;
    bool icmpMessageType = false;
    bool protocol = false;
    bool eVlan = false;
    bool etherType = false;
    bool tcpFlags = false;
    bool dscp = false;
    bool pcp = false;
    bool reasonCode = false;
    bool isDmacSet = false;
    bool isSmacSet = false;
    bool isDIPSet = false;
    bool isSIPSet = false;

    uint8_t numV6Fileds = 0;
    bool v6KyFldLst[XP_EACL_IPV6_MAX_FLDS] = {0,};

    sai_object_id_t acl_tid_port = 0ULL, acl_tid_vlan = 0ULL, acl_tid_rif = 0ULL,
                    acl_tid_sw = 0ULL;
    sai_object_id_t acl_entry_port = 0ULL, acl_entry_vlan = 0ULL,
                    acl_entry_route = 0ULL, acl_entry_switch = 0ULL;
    sai_attribute_t *eAclAttrList = (sai_attribute_t*)malloc(sizeof(
                                                                 sai_attribute_t)*MAX_ACL_TABLE_ATTR);
    memset(eAclAttrList, 0, sizeof(sai_attribute_t)*MAX_ACL_TABLE_ATTR);

    printf("eACL : devId:%d \n", devId);
    EaclL2KeyAttribute *eaclKey;
    EaclL2MaskAttribute *eaclMask;
    EaclData *eaclData;
    memset(&v6fldData, 0x00, sizeof(xpEaclkeyFieldList_t));
    memset(&v6ruleData, 0x00, sizeof(xpEaclkeyFieldList_t));

    numOfEaclDataEntries = numOfEntry(&configFile.eaclData);

    if (numOfEaclDataEntries == 0)
    {
        free(eAclAttrList);
        return XP_NO_ERR;
    }

    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_ACL_STAGE;
    eAclAttrList[count++].value.s32 = SAI_ACL_STAGE_EGRESS;

    /*Identify unique entries for preparing a new key to write in SE reg */
    for (i = 0; i < numOfEaclDataEntries; i++)
    {
        eaclKey = (EaclL2KeyAttribute *)getEntry(&configFile.eaclKey, i);

        printf("EACL TYPE : %d\n", eaclKey->type);
        switch (eaclKey->type)
        {
            case XP_EACL_V4_TYPE:

                if (!isDmacSet && eaclKey->isDmacSet)
                {
                    isDmacSet = true;
                    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
                    eAclAttrList[count++].value.booldata = true;
                }

                if (!isSmacSet && eaclKey->isSmacSet)
                {
                    isSmacSet = true;
                    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
                    eAclAttrList[count++].value.booldata = true;
                }

                if (!isDIPSet && eaclKey->isDIPSet)
                {
                    isDIPSet = true;
                    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_DST_IP;
                    eAclAttrList[count++].value.booldata = true;
                }

                if (!isSIPSet && eaclKey->isSIPSet)
                {
                    isSIPSet = true;
                    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_IP;
                    eAclAttrList[count++].value.booldata = true;
                }

                if (!L4DestPort && eaclKey->L4DestPort != -1)
                {
                    L4DestPort = true;
                    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT;
                    eAclAttrList[count++].value.booldata = true;
                }

                if (!L4SourcePort && eaclKey->L4SourcePort != -1)
                {
                    L4SourcePort = true;
                    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT;
                    eAclAttrList[count++].value.booldata = true;
                }

                if (!eVlan && eaclKey->eVlan != -1)
                {
                    eVlan = true;
                    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID;
                    eAclAttrList[count++].value.booldata = true;
                }

                if (!etherType && eaclKey->etherType != -1)
                {
                    etherType = true;
                    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
                    eAclAttrList[count++].value.booldata = true;
                }

                if (!icmpMessageType && eaclKey->icmpMessageType != -1)
                {
                    icmpMessageType= true;
                    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE;
                    eAclAttrList[count++].value.booldata = true;
                }

                if (!protocol && eaclKey->protocol != -1)
                {
                    protocol = true;
                    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
                    eAclAttrList[count++].value.booldata = true;
                }

                if (!dscp && eaclKey->dscp != -1)
                {
                    dscp = true;
                    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_DSCP;
                    eAclAttrList[count++].value.booldata = true;
                }

                if (!pcp && eaclKey->pcp != -1)
                {
                    pcp = true;
                    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI;
                    eAclAttrList[count++].value.booldata = true;
                }

                if (!tcpFlags && eaclKey->tcpFlags != -1)
                {
                    tcpFlags = true;
                    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS;
                    eAclAttrList[count++].value.booldata = true;
                }

                if (!reasonCode && eaclKey->reasonCode != -1)
                {
                    reasonCode = true;
                    eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
                    eAclAttrList[count++].value.booldata = true;
                }


                break;

            case XP_EACL_V6_TYPE:

                if (eaclKey->type != -1 && v6KyFldLst[XP_EACL_KEY_TYPE_V6] == 0)
                {
                    v6KyFldLst[XP_EACL_KEY_TYPE_V6] = 1;
                    numV6Fileds++;
                }

                if (eaclKey->isDIP6Set && v6KyFldLst[XP_EACL_DIP_V6] == 0)
                {
                    v6KyFldLst[XP_EACL_DIP_V6] = 1;
                    numV6Fileds++;
                }

                if (eaclKey->isSIP6Set && v6KyFldLst[XP_EACL_SIP_V6] == 0)
                {
                    v6KyFldLst[XP_EACL_SIP_V6] = 1;
                    numV6Fileds++;
                }

                if (eaclKey->L4DestPort != -1 && v6KyFldLst[XP_EACL_L4_V6_DEST_PORT] == 0)
                {
                    v6KyFldLst[XP_EACL_L4_V6_DEST_PORT] = 1;
                    numV6Fileds++;
                }

                if (eaclKey->L4SourcePort != -1 && v6KyFldLst[XP_EACL_L4_V6_SRC_PORT] == 0)
                {
                    v6KyFldLst[XP_EACL_L4_V6_SRC_PORT] = 1;
                    numV6Fileds++;
                }

                if (eaclKey->egressVif != -1 && v6KyFldLst[XP_EACL_V6_EGRESS_VIF] == 0)
                {
                    v6KyFldLst[XP_EACL_V6_EGRESS_VIF] = 1;
                    numV6Fileds++;
                }

                if (eaclKey->eVlan != -1 && v6KyFldLst[XP_EACL_V6_EGRESS_BD] == 0)
                {
                    v6KyFldLst[XP_EACL_V6_EGRESS_BD] = 1;
                    numV6Fileds++;
                }

                if (eaclKey->etherType != -1 &&
                    v6KyFldLst[XP_EACL_V6_ETHER_TYPE] == 0) //TODO: issue with this field
                {
                    v6KyFldLst[XP_EACL_V6_ETHER_TYPE] = 1;
                    numV6Fileds++;
                }

                if (eaclKey->icmpMessageType != -1 && v6KyFldLst[XP_EACL_ICMP_V6_MSG_TYPE] == 0)
                {
                    v6KyFldLst[XP_EACL_ICMP_V6_MSG_TYPE] = 1;
                    numV6Fileds++;
                }

                /* Not sure about the XP_EACL_V6_EGRESS_BD, so skipping it */

                if (((eaclKey->pcp != -1) || (eaclKey->exp != -1)) &&
                    v6KyFldLst[XP_EACL_V6_RSNCODE_TC_DP] == 0)
                {
                    v6KyFldLst[XP_EACL_V6_RSNCODE_TC_DP] = 1;
                    numV6Fileds++;
                }

                break;

            default:
                printf("ERROR: invalid EACL type\n");
                break;
        }
    }

    eAclAttrList[count].id = SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST;
    eAclAttrList[count].value.s32list.count = 1;
    eAclAttrList[count].value.s32list.list = (int32_t *)malloc(sizeof(sai_int32_t));
    eAclAttrList[count].value.s32list.list[0] = SAI_ACL_BIND_POINT_TYPE_PORT;
    tmpCount =
        count; //count value incremented later in code. Need to store allocated count value to free it later.
    tmpPtr = eAclAttrList[tmpCount].value.s32list.list;

    if ((saiRetVal = xpSaiAclApi->create_acl_table(&acl_tid_port, switchOid,
                                                   count+1, eAclAttrList)) != SAI_STATUS_SUCCESS)
    {
        printf(" create Table returns %d :\n", saiRetVal);
        printf("%s:, ERROR: Failed to createTable \n", __FUNCTION__);
        free(tmpPtr);
        free(eAclAttrList);
        return saiStatus2XpsStatus(saiRetVal);
    }

    eAclAttrList[count].value.s32list.list[0] = SAI_ACL_BIND_POINT_TYPE_VLAN;
    if ((saiRetVal = xpSaiAclApi->create_acl_table(&acl_tid_vlan, switchOid,
                                                   count+1, eAclAttrList)) != SAI_STATUS_SUCCESS)
    {
        printf(" create Table returns %d :\n", saiRetVal);
        printf("%s:, ERROR: Failed to createTable \n", __FUNCTION__);
        free(tmpPtr);
        free(eAclAttrList);
        return saiStatus2XpsStatus(saiRetVal);
    }

    eAclAttrList[count].value.s32list.list[0] = SAI_ACL_BIND_POINT_TYPE_ROUTER_INTF;
    if ((saiRetVal = xpSaiAclApi->create_acl_table(&acl_tid_rif, switchOid, count+1,
                                                   eAclAttrList)) != SAI_STATUS_SUCCESS)
    {
        printf(" create Table returns %d :\n", saiRetVal);
        printf("%s:, ERROR: Failed to createTable \n", __FUNCTION__);
        free(tmpPtr);
        free(eAclAttrList);
        return saiStatus2XpsStatus(saiRetVal);
    }

    eAclAttrList[count].value.s32list.list[0] = SAI_ACL_BIND_POINT_TYPE_SWITCH;
    if ((saiRetVal = xpSaiAclApi->create_acl_table(&acl_tid_sw, switchOid, count+1,
                                                   eAclAttrList)) != SAI_STATUS_SUCCESS)
    {
        printf(" create Table returns %d :\n", saiRetVal);
        printf("%s:, ERROR: Failed to createTable \n", __FUNCTION__);
        free(tmpPtr);
        free(eAclAttrList);
        return saiStatus2XpsStatus(saiRetVal);
    }

    /* For eacl ipv6 type*/
    tmpV6Index = 0;
    v6fldData.numFlds = numV6Fileds;
    v6fldData.type = XP_EACL_V6_TYPE;
    v6fldData.fldList = (xpEaclkeyField_t *)malloc(sizeof(xpEaclkeyField_t) *
                                                   numV6Fileds);
    memset(v6fldData.fldList, 0, sizeof(xpEaclkeyField_t) * numV6Fileds);
    v6fldData.isValid = 1;
    /* Initialize the field data and calculate key size from byte mask */
    for (i =0; i< XP_EACL_IPV6_MAX_FLDS; i++)
    {
        if (v6KyFldLst[i] == 1)
        {
            v6fldData.fldList[tmpV6Index].fld.v6Fld = (xpEaclV6KeyFlds)i;
            tmpV6Index++;
        }
    }
    printf("eACL : Table Created SucessFully\n");

    /*For ipv6 eacl */
    if ((retVal = xpsEaclDefineKey(devId, &v6fldData)) != XP_NO_ERR)
    {
        printf("%s:, ERROR: xpsEaclDefineKey() failed for ipv6 type \n", __FUNCTION__);
        free(v6fldData.fldList);
        free(tmpPtr);
        free(eAclAttrList);
        return retVal;
    }
    printf("eACL : xpsEaclDefineKey() for ipv6 type Done ! \n");

    for (i = 0; i < numOfEaclDataEntries; i++)
    {
        eaclDataEntry = (xpsEaclData_t *)malloc(sizeof(xpsEaclData_t));
        memset(eaclDataEntry, 0, sizeof(xpsEaclData_t));

        eaclData = (EaclData *)getEntry(&configFile.eaclData, i);
        eaclKey = (EaclL2KeyAttribute *)getEntry(&configFile.eaclKey, i);
        eaclMask = (EaclL2MaskAttribute *)getEntry(&configFile.eaclMask, i);

        memcpy(&v6ruleData, &v6fldData, sizeof(xpEaclkeyFieldList_t));

        count = 0;

        if (eaclKey->priority != -1)
        {
            eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
            eAclAttrList[count++].value.u32 = eaclKey->priority;
        }

        if (eaclKey->type == 0)
        {
            if (eaclKey->isSmacSet)
            {
                eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC;
                eAclAttrList[count].value.aclfield.enable =true;
                memcpy(eAclAttrList[count].value.aclfield.data.mac, eaclKey->macSa,
                       sizeof(sai_mac_t));
                memcpy(eAclAttrList[count].value.aclfield.mask.mac, eaclMask->macSa,
                       sizeof(sai_mac_t));

                // reversing the mac order as config file is parsed by reverse mac addr
                // User suppose to give mac addr in straight order
                xpSaiAppMacReverse(eAclAttrList[count].value.aclfield.data.mac);
                xpSaiAppMacReverse(eAclAttrList[count].value.aclfield.mask.mac);
                xpSaiNegMask((sai_uint8_t*)&eAclAttrList[count].value.aclfield.mask.mac,
                             sizeof(sai_mac_t));
                count++;
            }

            if (eaclKey->isDmacSet)
            {
                eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
                eAclAttrList[count].value.aclfield.enable =true;
                memcpy(eAclAttrList[count].value.aclfield.data.mac, eaclKey->macDa,
                       sizeof(sai_mac_t));
                memcpy(eAclAttrList[count].value.aclfield.mask.mac, eaclMask->macDa,
                       sizeof(sai_mac_t));

                // reversing the mac order as config file is parsed by reverse mac addr
                // User suppose to give mac addr in straight order
                xpSaiAppMacReverse(eAclAttrList[count].value.aclfield.data.mac);
                xpSaiAppMacReverse(eAclAttrList[count].value.aclfield.mask.mac);
                xpSaiNegMask((sai_uint8_t*)&eAclAttrList[count].value.aclfield.mask.mac,
                             sizeof(sai_mac_t));
                count++;
            }

            sai_ip4_t ipv4AddrKey, ipv4AddrMask;
            uint32_t len = sizeof(sai_ip4_t);
            uint8_t j = 0;
            if (eaclKey->isDIPSet)
            {
                eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IP;

                for (j = 0; j < len; ++j)
                {
                    ((uint8_t*)&ipv4AddrKey)[len-1-j] = eaclKey->dip[j];
                    ((uint8_t*)&ipv4AddrMask)[len-1-j] = eaclMask->dip[j];
                }

                eAclAttrList[count].value.aclfield.enable =true;
                eAclAttrList[count].value.aclfield.data.ip4 = ipv4AddrKey;
                eAclAttrList[count].value.aclfield.mask.ip4 = ipv4AddrMask;
                xpSaiNegMask((sai_uint8_t*)&eAclAttrList[count].value.aclfield.mask.ip4,
                             sizeof(sai_ip4_t));
                count++;
            }

            if (eaclKey->isSIPSet)
            {
                eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP;

                for (j = 0; j < len; ++j)
                {
                    ((uint8_t*)&ipv4AddrKey)[len-1-j] = eaclKey->sip[j];
                    ((uint8_t*)&ipv4AddrMask)[len-1-j] = eaclMask->sip[j];
                }

                eAclAttrList[count].value.aclfield.enable =true;
                eAclAttrList[count].value.aclfield.data.ip4 = ipv4AddrKey;
                eAclAttrList[count].value.aclfield.mask.ip4 = ipv4AddrMask;
                xpSaiNegMask((sai_uint8_t*)&eAclAttrList[count].value.aclfield.mask.ip4,
                             sizeof(sai_ip4_t));
                count++;
            }

            if (eaclKey->L4DestPort != -1)
            {
                tmpUintArray[0] = htole32(eaclKey->L4DestPort);
                tmpUintArray[1] = htole32(eaclMask->L4DestPort);
                eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
                eAclAttrList[count].value.aclfield.enable =true;
                eAclAttrList[count].value.aclfield.data.u16 = tmpUintArray[0];
                eAclAttrList[count].value.aclfield.mask.u16 = tmpUintArray[1];
                xpSaiNegMask((sai_uint8_t*)&eAclAttrList[count].value.aclfield.mask.u16,
                             sizeof(sai_uint16_t));
                count++;
            }

            if (eaclKey->L4SourcePort != -1)
            {
                tmpUintArray[0] = htole32(eaclKey->L4SourcePort);
                tmpUintArray[1] = htole32(eaclMask->L4SourcePort);
                eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT;
                eAclAttrList[count].value.aclfield.enable =true;
                eAclAttrList[count].value.aclfield.data.u16 = tmpUintArray[0];
                eAclAttrList[count].value.aclfield.mask.u16 = tmpUintArray[1];
                xpSaiNegMask((sai_uint8_t*)&eAclAttrList[count].value.aclfield.mask.u16,
                             sizeof(sai_uint16_t));
                count++;
            }

            if (eaclKey->etherType != -1)
            {
                tmpUintArray[0] = htole32(eaclKey->etherType);
                tmpUintArray[1] = htole32(eaclMask->etherType);
                eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE;
                eAclAttrList[count].value.aclfield.enable =true;
                eAclAttrList[count].value.aclfield.data.u16 = tmpUintArray[0];
                eAclAttrList[count].value.aclfield.mask.u16 = tmpUintArray[1];
                xpSaiNegMask((sai_uint8_t*)&eAclAttrList[count].value.aclfield.mask.u16,
                             sizeof(sai_uint16_t));
                count++;
            }

            if (eaclKey->icmpMessageType != -1)
            {
                tmpUintArray[0] = htole32(eaclKey->icmpMessageType);
                tmpUintArray[1] = htole32(eaclMask->icmpMessageType);
                eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE;
                eAclAttrList[count].value.aclfield.enable =true;
                eAclAttrList[count].value.aclfield.data.u8 = tmpUintArray[0];
                eAclAttrList[count].value.aclfield.mask.u8 = tmpUintArray[1];
                xpSaiNegMask((sai_uint8_t*)&eAclAttrList[count].value.aclfield.mask.u8,
                             sizeof(sai_uint8_t));
                count++;
            }

            if (eaclKey->protocol != -1)
            {
                tmpUintArray[0] = htole32(eaclKey->protocol);
                tmpUintArray[1] = htole32(eaclMask->protocol);
                eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
                eAclAttrList[count].value.aclfield.enable =true;
                eAclAttrList[count].value.aclfield.data.u8 = tmpUintArray[0];
                eAclAttrList[count].value.aclfield.mask.u8 = tmpUintArray[1];
                xpSaiNegMask((sai_uint8_t*)&eAclAttrList[count].value.aclfield.mask.u8,
                             sizeof(sai_uint8_t));
                count++;
            }

            if (eaclKey->dscp != -1)
            {
                tmpUintArray[0] = htole32(eaclKey->dscp);
                tmpUintArray[1] = htole32(eaclMask->dscp);
                eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_DSCP;
                eAclAttrList[count].value.aclfield.enable =true;
                eAclAttrList[count].value.aclfield.data.u8 = tmpUintArray[0];
                eAclAttrList[count].value.aclfield.mask.u8 = tmpUintArray[1];
                xpSaiNegMask((sai_uint8_t*)&eAclAttrList[count].value.aclfield.mask.u8,
                             sizeof(sai_uint8_t));
                count++;
            }

            if (eaclKey->pcp != -1)
            {
                tmpUintArray[0] = htole32(eaclKey->pcp);
                tmpUintArray[1] = htole32(eaclMask->pcp);
                eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI;
                eAclAttrList[count].value.aclfield.enable =true;
                eAclAttrList[count].value.aclfield.data.u8 = tmpUintArray[0];
                eAclAttrList[count].value.aclfield.mask.u8 = tmpUintArray[1];
                xpSaiNegMask((sai_uint8_t*)&eAclAttrList[count].value.aclfield.mask.u8,
                             sizeof(sai_uint8_t));
                count++;
            }

            if (eaclKey->tcpFlags != -1)
            {
                tmpUintArray[0] = htole32(eaclKey->tcpFlags);
                tmpUintArray[1] = htole32(eaclMask->tcpFlags);
                eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS;
                eAclAttrList[count].value.aclfield.enable =true;
                eAclAttrList[count].value.aclfield.data.u8 = tmpUintArray[0];
                eAclAttrList[count].value.aclfield.mask.u8 = tmpUintArray[1];
                xpSaiNegMask((sai_uint8_t*)&eAclAttrList[count].value.aclfield.mask.u8,
                             sizeof(sai_uint8_t));
                count++;
            }
        }
        else
        {
            if (v6KyFldLst[XP_EACL_KEY_TYPE_V6])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_KEY_TYPE_V6, v6KyFldLst, kefIndex);
                v6ruleData.fldList[kefIndex].value = (uint8_t *)&eaclKey->type;
                v6ruleData.fldList[kefIndex].mask = &tmpTypeMask;
            }

            if (v6KyFldLst[XP_EACL_DIP_V6])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_DIP_V6, v6KyFldLst, kefIndex);
                v6ruleData.fldList[kefIndex].value = eaclKey->dip6;
                v6ruleData.fldList[kefIndex].mask = eaclMask->dip6;
            }

            if (v6KyFldLst[XP_EACL_SIP_V6])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_SIP_V6, v6KyFldLst, kefIndex);
                v6ruleData.fldList[kefIndex].value = eaclKey->sip6;
                v6ruleData.fldList[kefIndex].mask = eaclMask->sip6;
            }

            if (v6KyFldLst[XP_EACL_L4_V6_DEST_PORT])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_L4_V6_DEST_PORT, v6KyFldLst, kefIndex);
                tmpUintArray[16] = htole32(eaclKey->L4DestPort);
                tmpUintArray[17] = htole32(eaclMask->L4DestPort);
                v6ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[16];
                v6ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[17];
            }

            if (v6KyFldLst[XP_EACL_L4_V6_SRC_PORT])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_L4_V6_SRC_PORT, v6KyFldLst, kefIndex);
                tmpUintArray[18] = htole32(eaclKey->L4SourcePort);
                tmpUintArray[19] = htole32(eaclMask->L4SourcePort);
                v6ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[18];
                v6ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[19];
            }

            if (v6KyFldLst[XP_EACL_V6_ETHER_TYPE])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_V6_ETHER_TYPE, v6KyFldLst, kefIndex);
                tmpUintArray[20] = htole32(eaclKey->etherType);
                tmpUintArray[21] = htole32(eaclMask->etherType);
                v6ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[20];
                v6ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[21];
            }

            if (v6KyFldLst[XP_EACL_V6_EGRESS_VIF])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_V6_EGRESS_VIF, v6KyFldLst, kefIndex);
                tmpUintArray[22] = htole32(eaclKey->egressVif);
                tmpUintArray[23] = htole32(eaclMask->egressVif);
                v6ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[22];
                v6ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[23];
            }

            if (v6KyFldLst[XP_EACL_V6_EGRESS_BD])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_V6_EGRESS_BD, v6KyFldLst, kefIndex);
                tmpUintArray[24] = htole32(eaclKey->eVlan);
                tmpUintArray[25] = htole32(eaclMask->eVlan);
                v6ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[24];
                v6ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[25];
            }

            if (v6KyFldLst[XP_EACL_ICMP_V6_MSG_TYPE])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_ICMP_V6_MSG_TYPE, v6KyFldLst, kefIndex);
                tmpUintArray[26] = htole32(eaclKey->icmpMessageType);
                tmpUintArray[27] = htole32(eaclMask->icmpMessageType);
                v6ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[26];
                v6ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[27];
            }

            tmpPcpData = 0x00;
            if (v6KyFldLst[XP_EACL_V6_RSNCODE_TC_DP])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_V6_RSNCODE_TC_DP, v6KyFldLst, kefIndex);
                tmpPcpData = (eaclKey->pcp & 0x07) | ((eaclKey->exp & 0x07) << 3);
                v6ruleData.fldList[kefIndex].value = &tmpPcpData;
                tmpPcpMask = (eaclMask->pcp & 0x07) | ((eaclMask->exp & 0x07) << 3);
                v6ruleData.fldList[kefIndex].mask = &tmpPcpMask;
            }
        }

        eaclDataEntry->enPktCmdUpd = eaclData->enPktCmdUpd;
        eaclDataEntry->enRsnCodeUpd = eaclData->enRsnCodeUpd;
        eaclDataEntry->pktCmd = eaclData->pktCmd;
        eaclDataEntry->rsnCode = eaclData->rsnCode;

        printf("pkt cmd en %d \n", eaclData->enPktCmdUpd);
        printf("rsn code update %d \n", eaclData->enRsnCodeUpd);
        printf("pktCmd %d \n", eaclData->pktCmd);
        printf("rsn code %d \n", eaclData->rsnCode);

        if (eaclDataEntry->enRsnCodeUpd)
        {
            eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_USER_TRAP_ID;
            eAclAttrList[count].value.aclaction.enable =true;
            eAclAttrList[count++].value.aclaction.parameter.u32 = eaclDataEntry->rsnCode;
        }

        if (eaclData->pktCmd != -1)
        {
            eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
            eAclAttrList[count].value.aclaction.enable =true;
            switch (eaclData->pktCmd)
            {
                case 0://DROP
                    eAclAttrList[count++].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_DROP;
                    break;
                case 1://FORWARD
                    eAclAttrList[count++].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_FORWARD;
                    break;
                case 2://TRAP_TO_CPU
                    eAclAttrList[count++].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_COPY;
                    break;
                case 3://FORWARD_AND_MIRROR_TO_CPU
                    eAclAttrList[count++].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_LOG;
                    break;
                default:
                    printf("%s:, ERROR: Invalid acl action \n", __FUNCTION__);
                    break;
            }
        }

        //
        // Create entry for v4 eacl egress ports
        //

        if (eaclKey->type  == 0 && eaclKey->egressVif != -1)
        {
            printf("eaclOnPort\n");
            uint32_t intfId;
            sai_attribute_t aclAttrPort;
            sai_object_id_t portOid;

            eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
            eAclAttrList[count++].value.oid = acl_tid_port;
            xpSaiAclApi->create_acl_entry(&acl_entry_port, switchOid, count, eAclAttrList);

            retVal = xpsPortGetPortIntfId(devId, eaclKey->egressVif, &intfId);
            if (retVal)
            {
                printf("Error : xpsPortGetPortIntfId() Failed for interfac type :%d devId %d\n",
                       XPS_PORT, devId);
                free(eaclDataEntry);
                free(v6fldData.fldList);
                free(tmpPtr);
                free(eAclAttrList);
                return retVal;
            }
            if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                              (sai_uint64_t)intfId, &portOid)) != SAI_STATUS_SUCCESS)
            {
                printf("Error : SAI object can not be created.\n");
                free(eaclDataEntry);
                free(v6fldData.fldList);
                free(tmpPtr);
                free(eAclAttrList);
                return saiStatus2XpsStatus(saiRetVal);
            }
            aclAttrPort.id = SAI_PORT_ATTR_EGRESS_ACL;
            aclAttrPort.value.oid = acl_tid_port;
            if ((saiRetVal = xpSaiPortApi->set_port_attribute(portOid,
                                                              &aclAttrPort)) != SAI_STATUS_SUCCESS)
            {
                printf("Error : set_port_attribute failed for port : %d\n",
                       (uint32_t)xpSaiObjIdValueGet(portOid));
                free(eaclDataEntry);
                free(v6fldData.fldList);
                free(tmpPtr);
                free(eAclAttrList);
                return saiStatus2XpsStatus(saiRetVal);
            }
        }

        //
        // Create entry for v4 eacl vlan
        //
        else if (eaclKey->type == 0 && eaclKey->eVlan != -1)
        {
            printf("eaclOnVlan\n");
            sai_attribute_t aclAttrVlan;
            sai_object_id_t vlanOid;

            eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
            eAclAttrList[count++].value.oid = acl_tid_vlan;
            xpSaiAclApi->create_acl_entry(&acl_entry_vlan, switchOid, count, eAclAttrList);

            if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, devId,
                                              (sai_uint64_t)eaclKey->eVlan, &vlanOid)) != SAI_STATUS_SUCCESS)
            {
                printf("Error : SAI object can not be created.\n");
                free(eaclDataEntry);
                free(v6fldData.fldList);
                free(tmpPtr);
                free(eAclAttrList);
                return saiStatus2XpsStatus(saiRetVal);
            }
            aclAttrVlan.id = SAI_VLAN_ATTR_EGRESS_ACL;
            aclAttrVlan.value.oid = acl_tid_vlan;
            if ((saiRetVal = xpSaiVlanApi->set_vlan_attribute(vlanOid,
                                                              &aclAttrVlan)) != SAI_STATUS_SUCCESS)
            {
                printf("Error : set_vlan_attribute failed for vlan : %d\n",
                       (uint32_t)xpSaiObjIdValueGet(vlanOid));
                free(eaclDataEntry);
                free(v6fldData.fldList);
                free(tmpPtr);
                free(eAclAttrList);
                return saiStatus2XpsStatus(saiRetVal);
            }
        }

        //
        // Create entry for v4 eacl router interface
        //
        else if (eaclKey->type == 0 && eaclKey->eRif != -1)
        {
            printf("eaclOnRoute\n");
            uint32_t l3IntfId = XPS_INTF_MAP_BD_TO_INTF(eaclKey->eRif);
            sai_attribute_t aclAttrRif;
            sai_object_id_t rifOid;

            eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
            eAclAttrList[count++].value.oid = acl_tid_rif;
            xpSaiAclApi->create_acl_entry(&acl_entry_route, switchOid, count, eAclAttrList);

            rifOid = xpSaiAppSaiGetVlanRifOid(l3IntfId);

            aclAttrRif.id = SAI_ROUTER_INTERFACE_ATTR_EGRESS_ACL;
            aclAttrRif.value.oid = acl_tid_rif;
            if ((saiRetVal = xpSaiRouterInterfaceApi->set_router_interface_attribute(rifOid,
                                                                                     &aclAttrRif)) != SAI_STATUS_SUCCESS)
            {
                printf("Error : set_router_interface_attribute failed for rif : %d\n",
                       (uint32_t)xpSaiObjIdValueGet(rifOid));
                free(eaclDataEntry);
                free(v6fldData.fldList);
                free(tmpPtr);
                free(eAclAttrList);
                return saiStatus2XpsStatus(saiRetVal);
            }
        }
        else if ((eaclKey->type == 0) && (eaclKey->eVlan == -1) &&
                 (eaclKey->egressVif == -1) &&
                 (eaclKey->eRif == -1))
        {
            sai_attribute_t aclAttrSw;
            sai_object_id_t swOid;

            printf("eaclOnSwitch\n");
            eAclAttrList[count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
            eAclAttrList[count++].value.oid = acl_tid_sw;
            xpSaiAclApi->create_acl_entry(&acl_entry_switch, switchOid, count,
                                          eAclAttrList);

            if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_SWITCH, devId,
                                              (sai_uint64_t)0, &swOid)) != SAI_STATUS_SUCCESS)
            {
                printf("Error : SAI object can not be created.\n");
                free(eaclDataEntry);
                free(v6fldData.fldList);
                free(tmpPtr);
                free(eAclAttrList);
                return saiStatus2XpsStatus(saiRetVal);
            }
            aclAttrSw.id = SAI_SWITCH_ATTR_EGRESS_ACL;
            aclAttrSw.value.oid = acl_tid_sw;
            if ((saiRetVal = xpSaiSwitchApi->set_switch_attribute(swOid,
                                                                  &aclAttrSw)) != SAI_STATUS_SUCCESS)
            {
                printf("Error : set_router_switch_attribute failed for sw : %d\n",
                       (uint32_t)xpSaiObjIdValueGet(swOid));
                free(eaclDataEntry);
                free(v6fldData.fldList);
                free(tmpPtr);
                free(eAclAttrList);
                return saiStatus2XpsStatus(saiRetVal);
            }
        }

        //
        // Write Key and Data for v6
        //
        else if (eaclKey->type == 1)
        {
            if ((retVal = xpsEaclWriteKey(devId, eaclKey->eAclId,
                                          &v6ruleData)) != XP_NO_ERR)
            {
                printf("%s:, ERROR:Failed to set eacl v6 rule for index %d\n", __FUNCTION__,
                       eaclKey->eAclId);
                free(eaclDataEntry);
                free(v6fldData.fldList);
                free(tmpPtr);
                free(eAclAttrList);
                return retVal;
            }

            if ((retVal = xpsEaclWriteData(devId, eaclKey->eAclId,
                                           eaclDataEntry)) != XP_NO_ERR)
            {
                printf("%s:, ERROR:Failed to set eacl data for eacl index %d\n", __FUNCTION__,
                       eaclKey->eAclId);
                free(eaclDataEntry);
                free(v6fldData.fldList);
                free(tmpPtr);
                free(eAclAttrList);
                return retVal;
            }
        }
        free(eaclDataEntry);
    }
    free(v6fldData.fldList);
    free(tmpPtr);
    free(eAclAttrList);

    printf("xpSaiAppConfigureEacl done successfully\n");
    return retVal;
}
#endif

#if 0
XP_STATUS xpSaiAppAddSetPortPrivateVlan(xpsDevice_t devId, xpVlan_t vlanId,
                                        xpPort_t devPort, xpSaiAppPortVlanParams * portVlan)
{
    uint8_t isVlanFound = 0;
    uint8_t cidx = 0, iidx = 0, index = 0;
    uint8_t numOfPvlanEntry = gPrivateVlanArray.count;
    xpSaiAppVlanContext *vlanCtx = gVlanCtxArray[vlanId];

    if (vlanCtx->vlanType == VLANTYPE_PRIMARY)
    {
        xpSaiAppAddPortToVlan(devId, vlanId, devPort, portVlan->tagType);
        xpSaiAppSetPortVlanParams(devId, vlanId, devPort, portVlan);
    }

    else if (vlanCtx->vlanType == VLANTYPE_COMMUNITY)
    {
        // In this case, add community ports to primary vlan, and also add primary vlan's ports in the community vlan
        for (index=0; index < numOfPvlanEntry; index++)
        {
            xpSaiAppPrivatevlanCtx *pVlanCtx = (xpSaiAppPrivatevlanCtx *)getEntry(
                                                   &gPrivateVlanArray, index);
            if ((pVlanCtx->priVlanCtx) &&
                (pVlanCtx->priVlanCtx->vlanId == vlanCtx->parentVlan))
            {
                for (cidx = 0; cidx < pVlanCtx->cidx; cidx++)
                {
                    if (pVlanCtx->comVlanCtx[cidx]->vlanId == vlanId)
                    {
                        isVlanFound = 1;
                        break;
                    }
                }
                if (isVlanFound)
                {
                    xpSaiAppVlanContext *comVlanCtx = pVlanCtx->comVlanCtx[cidx];
                    // Add community port and portvlan to community vlan
                    xpSaiAppAddPortToVlan(devId, vlanId, devPort, portVlan->tagType);
                    xpSaiAppSetPortVlanParams(devId, vlanId, devPort, portVlan);

                    // Add community port and portVlan to Primary vlan
                    xpSaiAppAddPortToPVlan(devId, comVlanCtx->parentVlan, devPort,
                                           portVlan->tagType, vlanId);
                }
            }
            if (index == 10)
            {
                printf("Error: Please configure primary Vlan first\n");
            }
        }
    }
    else if (vlanCtx->vlanType == VLANTYPE_ISOLATED)

    {
        // if its an isolated vlan then the packet should never be flooded to any of the isolated ports
        // not even unicasted. Such a packet can only get out through a primary port.
        for (index=0; index < numOfPvlanEntry; index++)
        {
            xpSaiAppPrivatevlanCtx *pVlanCtx = (xpSaiAppPrivatevlanCtx *)getEntry(
                                                   &gPrivateVlanArray, index);
            if ((pVlanCtx->priVlanCtx) &&
                (pVlanCtx->priVlanCtx->vlanId == vlanCtx->parentVlan))
            {
                for (iidx = 0; iidx < pVlanCtx->iidx; iidx++)
                {
                    if (pVlanCtx->isoVlanCtx[iidx]->vlanId == vlanId)
                    {
                        isVlanFound = 1;
                        break;
                    }
                }
                if (isVlanFound)
                {
                    xpSaiAppVlanContext *isoVlanCtx = pVlanCtx->isoVlanCtx[iidx];
                    // Add isolated port and portvlan to isolated vlan  //Need to create a filter
                    xpSaiAppAddPortToVlan(devId, vlanId, devPort, portVlan->tagType);
                    xpSaiAppSetPortVlanParams(devId, vlanId, devPort, portVlan);

                    //Filter this port;
                    xpL2DomainSetStgState(devId, devPort, vlanCtx->pVlanVifIdx[devId][devPort],
                                          vlanCtx->stgIdx, SPAN_STATE_DISABLE, 0);

                    // Add isolated port and portVlan to Primary vlan
                    xpSaiAppAddPortToPVlan(devId, isoVlanCtx->parentVlan, devPort,
                                           portVlan->tagType, vlanId);
                }
            }
        }
        if (index == 10)
        {
            printf("Error: Please configure primary Vlan first\n");
        }

    }

    return XP_NO_ERR;
}

#endif


#if 0 // commented to compile it with GCC

XP_STATUS xpSaiAppAddPortsToVlan(xpsDevice_t devId, xpVlan_t vlanId,
                                 VlanEntry * vlanEntry)
{
    uint32_t numOfPortEntry = 0, portEntryNum = 0;
    XP_STATUS retVal = XP_NO_ERR;
    xpPort_t devPort;
    xpSaiAppPortVlanParams *portVlan = (xpSaiAppPortVlanParams *)malloc(sizeof(
                                                                            xpSaiAppPortVlanParams));
    memset(portVlan, 0x0, sizeof(xpSaiAppPortVlanParams));
    numOfPortEntry = vlanEntry->numPorts;


    for (portEntryNum = 0; portEntryNum < numOfPortEntry; portEntryNum++)
    {
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
        portVlan->countMode = vlanEntry->portData[portEntryNum].countMode;

        xpSaiAppAddPortToVlan(devId, vlanId, devPort, portVlan->tagType);
        xpSaiAppSetPortVlanParams(devId, vlanId, devPort, portVlan);

    }
    printf("%s:Info: Ports added to Vlan successfully\n", __FUNCTION__);
    return retVal;
}

XP_STATUS xpSaiAppAgeHandler(xpsDevice_t devId)
{
    /*
     * Sample Code to invoke IPC Receive Hander to get Age
     * Fifo Message from SCP . Once the message is received
     * call the the AgeTableManager Hadler to Process FIO
     * Message
    */
    XP_STATUS status = XP_NO_ERR;
#ifndef MAC
    xpAgeFifoIpc fifoIpc;
    uint16_t msgSize = 0;
    xpAgeFifoData fifoData;

    memset(&fifoIpc, 0, sizeof(xpAgeFifoIpc));
    memset(&fifoData, 0, sizeof(xpAgeFifoData));

    status = xpIpcRecvAgeFifoIPC(devId, &fifoIpc, &msgSize);

    if (status)
    {
        return status;
    }

    if (!msgSize)
    {
        return XP_NO_ERR;
    }

    fifoData.ageMemRowAddr = fifoIpc.fifoMemAddr;
    fifoData.ageMemRowData[0] = fifoIpc.fifoRowData[0];
    fifoData.ageMemRowData[1] = fifoIpc.fifoRowData[1];

    xpTableManager *tblMgr = xpDeviceMgr::instance()->getTableMgr(devId);

    if (!tblMgr)
    {
        return XP_ERR_NULL_POINTER;
    }

    status = tblMgr->processAgeFifoMessage(&fifoData);
#endif
    return status;
}
#endif

#if 0
XP_STATUS xpSaiAppL2LearnProcess(xpsDevice_t devId, xphRxHdr *xphHdr, void *buf,
                                 uint16_t bufSize)
{
    XP_STATUS status = XP_NO_ERR;
    xphRxHdrMetaData *rxData;
    xpFdbEntry fdbEntry, lookupFdbEntryT;
    uint32_t index, reHashIndex;
    uint8_t srcMacAddr[XP_MAC_ADDR_LEN];
    uint8_t destMacAddr[XP_MAC_ADDR_LEN];
    int32_t lookupIndex;
    eVlanType vlanType;

    /*

    if primary vlan
        1. add the fdb entry in primary
        2. get all the secondary vlans
        3. add the fdb entry in all of them
    if community
        1. add the fdb entry in community
        2. get the primary and add fdb in it
    if isolated
        1. get the primary and add fdb in it

    */

    xpSaiAppAddFdb(devId, MacEntry *xpFdbData, xpSaiAppVlanContext *vlanCtx,
                   xpTableEntryHandle)

    LOG(XP_SUBMOD_L2_LEARN_MGR, XP_LOG_TRACE,
        "Inside xpL2LearnMgr::processL2LearnPacket");

    if (!xphHdr)
    {
        status = XP_ERR_NULL_POINTER;
        LOG(XP_LOG_ERROR, "No XPH header present in the packet, status = %d", status);
        return status;
    }

    LOG(XP_LOG_TRACE, "Reason code %d Vif %d", xphHdr->reasonCode,
        xphHdr->ingressVif);
    uint64_t mData = xphHdr->metadata;
    rxData = (xphRxHdrMetaData *)&mData;

    //Check vlanType
    uint32_t vlanId = rxData->vlanId;
    void *vlanCtx = xpSaiAppGetVlanContext(
                        vlanId); //API misleading. it gets the vlan context.
    (*(xpSaiAppVlanContext **)vlanCtx)->l2DomainContext;
    //
    //(*(xpSaiAppVlanContext **)vlanCtx)->l2DomainContext;
    // based on vlanType only the bdId is different.

    //below is the way of getting the vlanType from vlanId as of now.
    // Maybe in shim we might come up with a better maintenence.
    for (index=0; index < 10; index++)
    {
        if (gPrivateVlanArray[index].primaryVlan == vlanId)
        {
            vlanType = VLANTYPE_PRIMARY;
            break;
        }
        for (cidx = 0; cidx < gPrivateVlanArray[index].cidx; cidx++)
        {
            if (gPrivateVlanArray[index].communityVlan[cidx] == vlanId)
            {
                vlanType = VLANTYPE_COMMUNITY;
                break;
            }
        }
        for (iidx = 0; iidx < gPrivateVlanArray[index].iidx; iidx++)
        {
            if (gPrivateVlanArray[index].isolatedVlan[iidx] == vlanId)
            {
                vlanType = VLANTYPE_ISOLATED;
                break;
            }
        }
    }



    memcpy(&destMacAddr, buf, XP_MAC_ADDR_LEN);

    buf = (uint8_t *)buf + XP_MAC_ADDR_LEN;
    memcpy(&srcMacAddr, buf, XP_MAC_ADDR_LEN);

    /*fdbEntry.keyMACAddress[5] = srcMacAddr[0];
    fdbEntry.keyMACAddress[4] = srcMacAddr[1];
    fdbEntry.keyMACAddress[3] = srcMacAddr[2];
    fdbEntry.keyMACAddress[2] = srcMacAddr[3];
    fdbEntry.keyMACAddress[1] = srcMacAddr[4];
    fdbEntry.keyMACAddress[0] = srcMacAddr[5];

    fdbEntry.keyBridgeDomain = rxData->bdId;
    fdbEntry.pktCmd = XP_PKTCMD_FWD;
    fdbEntry.mirrorMask = 0;
    fdbEntry.macDAIsControl = 0;
    fdbEntry.countMode = 0;
    fdbEntry.ecmpSize = 1;
    fdbEntry.ingressEgressVif = xphHdr->ingressVif;*/

    fdbEntry.macAddr[5] = xpFdbData->macAddr[0];
    fdbEntry.macAddr[4] = xpFdbData->macAddr[1];
    fdbEntry.macAddr[3] = xpFdbData->macAddr[2];
    fdbEntry.macAddr[2] = xpFdbData->macAddr[3];
    fdbEntry.macAddr[1] = xpFdbData->macAddr[4];
    fdbEntry.macAddr[0] = xpFdbData->macAddr[5];

    fdbEntry.bdId = vlanCtx->l2DomainContext.bdId;
    fdbEntry.pktCmd = XP_PKTCMD_FWD;
    fdbEntry.mirrorMask = 0;
    fdbEntry.macDAIsControl = 0;
    fdbEntry.routerMAC = xpFdbData->routerMac;;
    fdbEntry.countMode = 0;
    fdbEntry.ecmpSize = 1;
    fdbEntry.vif = xphHdr->ingressVif;

    LOG(XP_LOG_TRACE, "Incoming BD %d Vlan %d", (uint32_t)rxData->bdId,
        (uint32_t)rxData->vlanId);


    if ((retVal = xpL2LearnAddFdbEntry(devId, &fdbEntry, &index,
                                       &reHashIndex)) != XP_NO_ERR)
    {
        printf(" %d %d \n", fdbEntry.encapType, encapEntryData.encapId);
        printf("%s:Error in inserting an entry: error code: %d\n", __FUNCTION__,
               retVal);
        return retVal;
    }

    // Depending on reason code handle code differently

    switch (xphHdr->reasonCode)
    {

        case XP_BRIDGE_MAC_SA_NEW:
        case XP_BRIDGE_RC_IVIF_SA_MISS:

            if ((status = fdbMgr->addEntry(devId, (void *)&fdbEntry, index,
                                           reHashIndex)) != XP_NO_ERR)
            {
                LOG(XP_LOG_ERROR, "Error in inserting an FDB entry: error code: %d\n", status);
                return status;
            }

            LOG(XP_LOG_TRACE, "Added fdb entry at index %d ", index);
            break;

        case XP_BRIDGE_MAC_SA_MOVE:

            // Lookup the original entry

            if ((status = fdbMgr->lookup(devId, (void *)&fdbEntry,
                                         lookupIndex)) != XP_NO_ERR)
            {
                LOG(XP_LOG_ERROR, "Mac Move: Fdb table lookup failed: error code: %d", status);
                return status;
            }

            LOG(XP_LOG_TRACE,
                "Mac Move: Fdb table lookup index: %d pktCmd = %d  ingressEgressVif = %d",
                lookupIndex, fdbEntry.pktCmd, fdbEntry.ingressEgressVif);

            // Update the entry

            fdbEntry.ingressEgressVif = xphHdr->ingressVif;

            if ((status = fdbMgr->writeEntry(devId, lookupIndex,
                                             (void *)&fdbEntry)) != XP_NO_ERR)
            {
                LOG(XP_LOG_ERROR, "Error in updating an FDB entry: error code: %d", status);
                return status;
            }

            LOG(XP_LOG_TRACE, "Updated fdb entry after mac move at index %d", index);
            break;

        default:
            break;
    }

    //Perform the lookup on added entry just before
    lookupIndex = 0;
    lookupFdbEntryT.keyMACAddress[5] = srcMacAddr[0];
    lookupFdbEntryT.keyMACAddress[4] = srcMacAddr[1];
    lookupFdbEntryT.keyMACAddress[3] = srcMacAddr[2];
    lookupFdbEntryT.keyMACAddress[2] = srcMacAddr[3];
    lookupFdbEntryT.keyMACAddress[1] = srcMacAddr[4];
    lookupFdbEntryT.keyMACAddress[0] = srcMacAddr[5];

    lookupFdbEntryT.keyBridgeDomain = rxData->bdId;

    if ((status = fdbMgr->lookup(devId, (void *)&lookupFdbEntryT,
                                 lookupIndex)) != XP_NO_ERR)
    {
        LOG(XP_LOG_ERROR, "Error in Fdb table lookup: error code: %d", status);
        return status;
    }

    LOG(XP_LOG_TRACE,
        "Lookup newly added fdb entry at index = %d pktCmd = %d  ingressEgressVif = %d",
        lookupIndex, lookupFdbEntryT.pktCmd, lookupFdbEntryT.ingressEgressVif);

    return status;
}
#endif

int testMplsTunnel(xpsDevice_t deviceId, XP_PIPE_MODE mode)
{
    ////TODO: do we need this. if we do, please use FL apis.
    //xpMplsTunnelEntry tblEntry;
    //xpMplsTunnelEntry getTblEntry;
    //uint32_t index = 0, reHashIndex = 0;
    //int32_t lookupIdx = 0;

    //printf("dbg: Inside %s func\n", __FUNCTION__);


    // //
    //// Dev Init for the MPLS  Manager
    ////
    //mplsTunnelMgr = (xpMplsTunnelMgr *)xpTunnelPl::instance()->getMgr(XP_MPLS_TUNNEL_MGR);

    //if(mplsTunnelMgr == NULL)
    //{
    //    printf("ERROR <dev = %d> :mplsTunnelMgr is NULL. Exiting...\n",0);
    //    return XP_ERR_INIT;
    //}
    //uint8_t tblCopyNum = tableProfile->getXpIsmeTblCopyNum(MPLS_TNL_TBL);
    //xpIsmeTableContext* mplsTunnelTblCtx[tblCopyNum];
    //for(uint8_t i = 0; i < tblCopyNum; i++)
    //{
    //    mplsTunnelTblCtx[i] = tableProfile->getXpIsmeCfg(MPLS_TNL_TBL, i);
    //}


    //xpMplsTunnelMgr *xpPmMgr = static_cast<xpMplsTunnelMgr *>(xpTunnelPl::instance()->getMgr(XP_MPLS_TUNNEL_MGR));


    //memset((void *)&tblEntry, 0 , sizeof(xpMplsTunnelEntry));
    //for (int i = 0; i < 5; i++)
    //{
    //    tblEntry.KeyOuterVlanId = 0;
    //    tblEntry.KeyMplsLabel = i + 29;
    //    tblEntry.isP2MPBudNode = 1U;
    //    if(xpPmMgr->addEntry(deviceId, (void *)&tblEntry, index, reHashIndex) != XP_NO_ERR)
    //    {
    //        printf("ERROR in inserting an entry. Exiting...\n");
    //        //return -1;
    //    }
    //    printf("index of %dth entry : %d \n",i+1, index);
    //}

    //tblEntry.KeyOuterVlanId = 25U;
    //tblEntry.KeyMplsLabel = 40U;
    //tblEntry.isP2MPBudNode = 1U;
    //tblEntry.innerTerminateLabelPos = 2U;
    //tblEntry.countMode = 5U;
    //tblEntry.copyTTLfromTunnelHeader = 1U;
    //tblEntry.floodVif = 3215U;
    //tblEntry.counterIndex = 2567U;

    ///* Add an entry */
    //if(xpPmMgr->addEntry(deviceId, (void *)&tblEntry, index, reHashIndex) != XP_NO_ERR)
    //{
    //    printf("ERROR in inserting an entry. Exiting...\n");
    //    return -1;
    //}

    //getTblEntry.KeyOuterVlanId = 25U;
    //getTblEntry.KeyMplsLabel = 40U;

    //xpPmMgr->lookup(deviceId, (void *)&getTblEntry, lookupIdx);
    //if(lookupIdx != (int32_t)index)
    //{
    //    printf("Lookup Mismatch:: Expected Value %d Actual Value %d \n", index, lookupIdx);
    //}
    //else
    //{
    //    printf("Lookup PASS:: Index %d Actual Value %d \n", lookupIdx, index);
    //    //compareMplsTunnelEntry(&tblEntry, &getTblEntry);
    //}

    //tblEntry.isP2MPBudNode = 1U;
    //tblEntry.innerTerminateLabelPos = 2U;
    //tblEntry.countMode = 3U;
    //tblEntry.copyTTLfromTunnelHeader = 1U;
    //tblEntry.floodVif = 1234U;
    //tblEntry.counterIndex = 23567U;

    ///* Set whole entry */
    //if(xpPmMgr->writeEntry(deviceId, index, (void *)&tblEntry) != XP_NO_ERR)
    //{
    //    printf("ERROR in updating an entry. Exiting...\n");
    //    return -1;
    //}

    ///* Read whole Entry */
    //if(xpPmMgr->readEntry(deviceId, index, (void *)&getTblEntry) != XP_NO_ERR)
    //{
    //    printf("ERROR in reading an entry. Exiting...\n");
    //    return -1;
    //}
    ////compareMplsTunnelEntry(&tblEntry, &getTblEntry);

    //uint32_t floodVif = 123U, getFloodVif = 0;
    //xpPmMgr->setField(deviceId, index, MPLSTUNNEL_FLOOD_VIF, (void *)&floodVif);
    //xpPmMgr->getField(deviceId, index, MPLSTUNNEL_FLOOD_VIF, (void *)&getFloodVif);
    //if(floodVif != getFloodVif)
    //{
    //    printf("Compare floodVif :: FAIL\n");
    //} else
    //{
    //    printf("Compare floodVif :: PASS\n");
    //}

    return 0;
}


////
//// Device Level sdk APP deinit
////
//XP_STATUS xdkDeInitDevice(xpsDevice_t devId)
//{
//    XP_STATUS status = XP_NO_ERR;
//#if 0
//    //
//    // Deinitialize the Device Manager Layer
//    // Deinit all Blocks on the chip
//    //
//    xpInitMgr  *initMgr;
//    initMgr = new xpInitMgr(devId);
//    status = initMgr->removeDevice(devId,devType,devConfig);
//    if(status)
//    {
//        printf("Device Manager deInit failed. Error #%1d", status);
//        return status;
//    }
//#endif
//    //status = xpInit xpSaiAppPLRemoveDevice(devId);
//    status = xpAppDeInitDevice(devId);
//    if (status)
//    if(status != 0)
//    {
//        printf("Removing device failed. Error #%1d", status);
//        return status;
//    }
XP_STATUS natPacketHandler(xpsDevice_t devId, xphRxHdr *xphHdr, void *buf,
                           uint16_t bufSize)
{
    //TODO ::  Verify from Ashwini weather this is stale then remove
    XP_STATUS status = XP_NO_ERR;
#if 0
    printf("nat packet handler \n");
    int SIP_OFFSET = 26;
    ipv4Addr_t sip;
    ipv4Addr_t dip;

    buf = (uint8_t*)buf + SIP_OFFSET;
    memcpy(&sip, buf, sizeof(ipv4Addr_t));
    printf(" SIP Address :: 0x%x:0x%x:0x%x:0x%x\n", sip[0], sip[1], sip[2], sip[3]);
    buf = (uint8_t*)buf + XP_IPV4_ADDR_LEN;
    memcpy(&dip, buf, sizeof(ipv4Addr_t));
    printf(" DIP Address :: 0x%x:0x%x:0x%x:0x%x\n", dip[0], dip[1], dip[2], dip[3]);

    xpNATIpv4Entry tblEntry;
    tblEntry.key.IsValid = 1;
    tblEntry.mask.IsValid = 0x1;
    tblEntry.mask.DestAddress[0] = 0x0;
    tblEntry.mask.DestAddress[1] = 0x0;
    tblEntry.mask.DestAddress[2] = 0x0;
    tblEntry.mask.DestAddress[3] = 0x0;
    tblEntry.mask.SrcAddress[0] = 0xff;
    tblEntry.mask.SrcAddress[1] = 0x0ff;
    tblEntry.mask.SrcAddress[2] = 0xff;
    tblEntry.mask.SrcAddress[3] = 0xff;
    memcpy(tblEntry.key.SrcAddress, sip, sizeof(ipv4Addr_t));
    memcpy(tblEntry.key.DestAddress, dip, sizeof(ipv4Addr_t));
    if (natipv4Mgr->writeEntry(devId, 5, (void *)&tblEntry) != XP_NO_ERR)
    {

    }
    xpNATIpv4Entry getTblEntry;
    natipv4Mgr->readEntry(devId, 5, &getTblEntry);
    getTblEntry.printEntry();

    printf("Inside natPacketHandler \n");
#endif
    return status;
}

//TODO
XP_STATUS xpSaiAppWriteNatData(xpsDevice_t devId, sai_object_id_t switchOid)
{
    XP_STATUS retVal = XP_NO_ERR;
    int numOfNatDataEntries = 0;
    uint32_t j = 0;
    xpsNatEntryKey_t xpsKey;
    xpsNatEntryMask_t xpsMask;
    xpsNatEntryData_t xpsData;
    NatEntry *natEntry;

    printf("\nIn XPS NAT Programming\n");
    numOfNatDataEntries = numOfEntry(&configFile.natData);
    int i;
    for (i = 0; i < numOfNatDataEntries; i++)
    {
        natEntry = (NatEntry *)getEntry(&configFile.natData, i);
        xpSaiAppVlanContext *vlanCtx = (xpSaiAppVlanContext *)malloc(sizeof(
                                                                         xpSaiAppVlanContext));
        //initVlanContext(vlanCtx);
        if (!vlanCtx)
        {
            printf("%s:Error: Can't allocate vlan context for vlan Id\n", __FUNCTION__);
            return XP_ERR_INIT;
        }
        memset(&xpsKey, 0, sizeof(xpsNatEntryKey_t));
        memset(&xpsMask, 0, sizeof(xpsNatEntryMask_t));
        memset(&xpsData, 0, sizeof(xpsNatEntryData_t));

        /* Set key */
        xpsKey.SrcAddress[0] = natEntry->keySrcAddress[3];
        xpsKey.SrcAddress[1] = natEntry->keySrcAddress[2];
        xpsKey.SrcAddress[2] = natEntry->keySrcAddress[1];
        xpsKey.SrcAddress[3] = natEntry->keySrcAddress[0];
        xpsKey.SrcPort = natEntry->keySrcPort;
        xpsKey.DestAddress[0] = natEntry->keyDestAddress[3];
        xpsKey.DestAddress[1] = natEntry->keyDestAddress[2];
        xpsKey.DestAddress[2] = natEntry->keyDestAddress[1];
        xpsKey.DestAddress[3] = natEntry->keyDestAddress[0];
        xpsKey.DestPort = natEntry->keyDestPort;
        vlanCtx->vlanId = natEntry->bd;
#if 0
        xpSaiAppVlanContext *ptr = xpSaiAppGetVlanContext(natEntry->bd);
        if (ptr == NULL)
        {
            printf("%s:Error: Can't get vlanCtx for bdId %d\n", __FUNCTION__, natEntry->bd);
            return XP_ERR_INIT;
        }

        if ((retVal = xpL2DomainGetBdId(&ptr->l2DomainContext, &bdId)) != XP_NO_ERR)
        {
            printf("%s:Error: xpL2DomainMgr::getBdId() failed with error %d\n",
                   __FUNCTION__, retVal);
            return retVal;
        }
        xpsKey.Bd = natEntry->bd; //TODO
        printf("bd %d mode %d scope %d \n", natEntry->bd, natEntry->natMode,
               natEntry->natScope);
        // Program nat mode
        if ((retVal = xpsSetNatModeVlan(devId, natEntry->bd,
                                        natEntry->natMode)) != XP_NO_ERR)
        {
            printf("%s:Error: xpsSetNatModeOnPortVlan() failed with error %d\n",
                   __FUNCTION__, retVal);
            return retVal;
        }
        // Program nat scope
        if ((retVal = xpsSetNatScopeVlan(devId, natEntry->bd,
                                         natEntry->natScope)) != XP_NO_ERR)
        {
            printf("%s:Error: xpsSetNatScopeOnPortVlan() failed with error %d\n",
                   __FUNCTION__, retVal);
            return retVal;
        }
#endif
        xpsKey.Bd = natEntry->bd;

        if (natEntry->flag)
        {
            xpsKey.Flag = natEntry->flag;
        }
        xpsKey.Protocol = natEntry->protocol;

        /* Set mask */
        for (j = 0; j < sizeof(ipv4Addr_t); j++)
        {
            if (natEntry->keyDestAddress[j] == natEntry->dipAddress[j])
            {
                continue;
            }
            else
            {
                break;
            }
        }
        if (j == 4)
        {
            memset(xpsMask.DestAddress, 0xff, sizeof(ipv4Addr_t));
        }
        for (j = 0; j < sizeof(ipv4Addr_t); j++)
        {
            if (natEntry->keySrcAddress[j] == natEntry->sipAddress[j])
            {
                continue;
            }
            else
            {
                break;
            }
        }
        if (j == 4)
        {
            memset(xpsMask.SrcAddress, 0xff, sizeof(ipv4Addr_t));
        }
        if (natEntry->keySrcPort == natEntry->srcPort)
        {
            xpsMask.SrcPort = 0xffff;
        }
        if (natEntry->keyDestPort == natEntry->destPort)
        {
            xpsMask.DestPort = 0xffff;
        }
        xpsMask.Bd = 0xffff;
        if (!xpsKey.Flag)
        {
            xpsMask.Flag = 0x1ff;
        }
        xpsMask.Protocol = 0xff;
        //xpsMask.type = 0x1;

        /* Set data */
        xpsData.SIPAddress[0] = natEntry->sipAddress[3];
        xpsData.SIPAddress[1] = natEntry->sipAddress[2];
        xpsData.SIPAddress[2] = natEntry->sipAddress[1];
        xpsData.SIPAddress[3] = natEntry->sipAddress[0];
        xpsData.srcPort = natEntry->srcPort;
        xpsData.DIPAddress[0] = natEntry->dipAddress[3];
        xpsData.DIPAddress[1] = natEntry->dipAddress[2];
        xpsData.DIPAddress[2] = natEntry->dipAddress[1];
        xpsData.DIPAddress[3] = natEntry->dipAddress[0];
        xpsData.destPort = natEntry->destPort;
        xpsData.vif = 100;


        //xpsL3IntfSetNatEn(devId,XPS_INTF_MAP_BD_TO_INTF(100),0x1);

        if (natEntry->natMode == 0x2 && natEntry->pktCmd != 0x1)
        {
            printf("calling external entry \n");
            if ((retVal = xpsNatAddExternalEntry(devId, i, &xpsKey, &xpsMask,
                                                 &xpsData)) != XP_NO_ERR)
            {
                printf("%s:Error: Writing external NAT IPv4 entry at index %d failed. error-code = %d\n",
                       __FUNCTION__, i, retVal);
                free(vlanCtx);
                return retVal;
            }
        }
        else if (natEntry->natMode == 0x3 && natEntry->pktCmd != 0x1)
        {
            printf("calling Add entry \n");
            if ((retVal = xpsNatAddDoubleEntry(devId, i, &xpsKey, &xpsMask,
                                               &xpsData)) != XP_NO_ERR)
            {
                printf("%s:Error: Writing NAT entry at index %d failed error-code = %d \n",
                       __FUNCTION__, i, retVal);
                free(vlanCtx);
                return retVal;
            }
        }
        else if (natEntry->pktCmd != 0x1)
        {
            printf("calling internal entry \n");
            if ((retVal = xpsNatAddInternalEntry(devId, i, &xpsKey, &xpsMask,
                                                 &xpsData)) != XP_NO_ERR)
            {
                printf("%s:Error: Writing internal NAT IPv4 entry at index %d failed. error-code = %d\n",
                       __FUNCTION__, i, retVal);
                free(vlanCtx);
                return retVal;
            }
        }
        else
        {
            printf("calling filter rule \n");

            if ((retVal = xpsNatAddFilterRule(devId, i, &xpsKey, &xpsMask,
                                              &xpsData)) != XP_NO_ERR)
            {
                printf("%s:Error: Writing Fileter Rule at index %d failed. error-code = %d\n",
                       __FUNCTION__, i, retVal);
                free(vlanCtx);
                return retVal;
            }
        }
        /* Lookup */
        free(vlanCtx);
    }
    ////uint32_t index = 88,value = 1;
    ////retVal = xpSetMdtNatConfig(devId,index,value);
    //printf("Exiting on success\n");
    return XP_NO_ERR; //TODO
}

//TODO
#ifdef __cplusplus
extern "C" {
#endif
XP_STATUS xpAppSendPacketToXp(xpsDevice_t deviceId, sai_object_id_t switchOid)
{
    XP_STATUS retVal = XP_NO_ERR;
    struct xpPacketInfo *pktInfo = NULL;
    int queueNumArr[65] = {0, }, i = 0;
    uint32_t numOfPcapEntry = numOfEntry(&configFile.controlPcapList);
    xpInputControlPktEntry *pcapEntry = NULL;
    int pktHexFile = 0;
    uint32_t pktLen = 0;
    char *pktBufOffset = NULL;
    uint32_t entryNum = 0, offset = 0;
    uint32_t ingressVif;
    uint32_t egressVif;
    uint32_t mcpuPort = 0, mcpuSdePortNum = 0;
    xphTxHdr txHeader;
    sai_attribute_t switchAttrList[1];
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpPort_t cpuPort = 0;

    //Check for the list of packets to send are available or not
    if (!strcmp(XP_CONFIG.pcapConfigFile, ""))
    {
        return XP_NO_ERR;
    }

    for (entryNum = 0; entryNum < numOfPcapEntry; entryNum++)
    {
        pcapEntry = (xpInputControlPktEntry *)getEntry(&configFile.controlPcapList,
                                                       entryNum);
        memset(queueNumArr, 0, sizeof(queueNumArr));
        if (!strncmp(pcapEntry->queueNum, "all", 3))
        {
            for (i = 0; i <= 40; i++)
            {
                queueNumArr[i] = i;
            }
            queueNumArr[i] = -1;
        }
        else
        {
            queueNumArr[0] = atoi(pcapEntry->queueNum);
            queueNumArr[1] = -1;
        }

        //Clear txHeader
        memset(&txHeader, 0, sizeof(xphTxHdr));
        txHeader.metadata.setBridgeDomain = pcapEntry->setBd;
        txHeader.metadata.bridgeDomain[0] = (pcapEntry->bridgeDomain >> 8) & 0xFF;
        txHeader.metadata.bridgeDomain[1] = (pcapEntry->bridgeDomain) & 0xFF;
        txHeader.metadata.macSAmissCmd = pcapEntry->macSAMissCmd;
        txHeader.metadata.acceptedFrameType = pcapEntry->acceptedFrameType;
        txHeader.metadata.setIngressVif = 1;

        offset = 0;
        pktInfo = (struct xpPacketInfo*)malloc(sizeof(struct xpPacketInfo));
        if (!pktInfo)
        {
            printf("%s:Error: malloc for pktInfo failed\n", __FUNCTION__);
            return XP_ERR_MEM_ALLOC_ERROR;
        }

        i = 0;
        while (queueNumArr[i] != -1)
        {
            memset(pktInfo, 0, sizeof(struct xpPacketInfo));

            pktInfo->buf = xpGetBuffer;
            if (!pktInfo->buf)
            {
                i++;
                printf("%s: ERR: Could not get buffer\n", __FUNCTION__);
                continue;
            }
            pktInfo->priority = queueNumArr[i];
            pktHexFile = open(pcapEntry->pcapName, O_RDONLY, (int)0600);
            if (pktHexFile == -1)
            {
                xpFreeBuffer(pktInfo->buf);
                free(pktInfo);
                printf("%s: Error: could not get packet hex file: %s\n", __FUNCTION__,
                       strerror(errno));
                return XP_ERR_FILE_OPEN;
            }

            // Retrive the pvif for the mcpu sde port
            // if ingress port is cpu port then get vif for cpu port from PortConfig
            switchAttrList[0].id = SAI_SWITCH_ATTR_CPU_PORT;

            saiRetVal = xpSaiSwitchApi->get_switch_attribute(switchOid, 1, switchAttrList);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                printf("Error %d: SAI get_switch_attribute failed for SAI_SWITCH_ATTR_CPU_PORT ",
                       saiRetVal);
                xpFreeBuffer(pktInfo->buf);
                free(pktInfo);
                close(pktHexFile);
                return saiStatus2XpsStatus(saiRetVal);
            }

            cpuPort = (xpPort_t)xpSaiObjIdValueGet(switchAttrList[0].value.oid);

            printf("%s: get_switch_attribute cpu port number %d ",
                   __FUNCTION__, cpuPort);

            if ((retVal = xpsGlobalSwitchControlGetMcpuPortNumber(deviceId,
                                                                  &mcpuPort)) != XP_NO_ERR)
            {
                printf("Error %d: Unable to get MCPU port number\n", retVal);
            }

            if (pcapEntry->ingressPort == mcpuPort)
            {
                if ((retVal = xpsGlobalSwitchControlGetMcpuSdePortNumber(deviceId,
                                                                         &mcpuSdePortNum)) != XP_NO_ERR)
                {
                    close(pktHexFile);
                    xpFreeBuffer(pktInfo->buf);
                    free(pktInfo);
                    return retVal;
                }
                // Retrive the pvif for the mcpu sde port
                pcapEntry->ingressPort = mcpuSdePortNum;
            }
            if ((retVal = xpsPortGetPortIntfId(deviceId, pcapEntry->ingressPort,
                                               &ingressVif)) != XP_NO_ERR)
            {
                printf("Error: Get Port IntfId for Ingress Port Failed with Error Code: %d\n",
                       retVal);
                close(pktHexFile);
                xpFreeBuffer(pktInfo->buf);
                free(pktInfo);
                return retVal;
            }
            if ((retVal = xpsPortGetPortIntfId(deviceId, pcapEntry->egressPort,
                                               &egressVif)) != XP_NO_ERR)
            {
                printf("Error: Get Port IntfId for Egress Port Failed with Error Code: %d\n",
                       retVal);
                close(pktHexFile);
                xpFreeBuffer(pktInfo->buf);
                free(pktInfo);
                return retVal;
            }

            txHeader.ingressVifLsbByte2 = (ingressVif >> 12) & 0xFF;
            txHeader.ingressVifLsbByte1 = (ingressVif >> 4) & 0xFF;
            txHeader.ingressVifLsbByte0 = ingressVif & 0xF;

            txHeader.egressVifLsbByte2 = (egressVif >> 16) & 0xF;
            txHeader.egressVifLsbByte1 = (egressVif >> 8) & 0xFF;
            txHeader.egressVifLsbByte0 = egressVif & 0xFF;

            if (strcmp(pcapEntry->nextEngine, "IVIF") == 0)
            {
                txHeader.nextEngine = 3;
                offset = sizeof(xphTxHdr);
            }
            else if (strcmp(pcapEntry->nextEngine, "URW") == 0)
            {
                txHeader.nextEngine = 16;
                offset = sizeof(xphTxHdr);
            }

            if (offset)
            {
                //xpInterfaceParserMgrSetIktXpHeaderExists(deviceId, CPU_PORT, 1);
            }

            lseek(pktHexFile, 0, SEEK_SET);
            pktBufOffset = ((char *)pktInfo->buf + offset);
            /* Copy packet data*/
            if ((retVal = xpGetPktDataAndPktLength(pktHexFile, &pktBufOffset,
                                                   &pktLen)) != XP_NO_ERR)
            {
                close(pktHexFile);
                xpFreeBuffer(pktInfo->buf);
                free(pktInfo);
                PRINT_LOG("%s:Error: xpGetPktDataAndPktLength failed with error code = %d\n",
                          __FUNCTION__, retVal);
                return retVal;
            }
            pktInfo->bufSize = pktLen + sizeof(xphTxHdr);

            close(pktHexFile);
            xpFreeBuffer(pktInfo->buf);
            i++;
        }
        free(pktInfo);
    }
    return XP_NO_ERR;
}

XP_STATUS xpAppAddDevice(xpsDevice_t deviceId, xpsInitType_t initType,
                         xpDevConfigStruct* config)
{
    XP_STATUS retVal;
    //
    // Demo : Program the HW Tables using the config data input
    //
    if ((retVal = xpSaiAppPopulateTables(deviceId, NULL)) != XP_NO_ERR)
    {
        printf("%s:Error: xpSaiAppPopulateTables Failed with Error code: %d\n",
               __FUNCTION__, retVal);
        return XP_ERR_INIT_FAILED;
    }

    printf("xpSaiApp config.txt loaded for device 0x%x\n", deviceId);

    xpAppSchedPolicyInit(deviceId);
    return XP_NO_ERR;
}
#ifdef __cplusplus
}
#endif

XP_STATUS xpAppCleanup(xpsDevice_t devId)
{
    XP_STATUS status = XP_NO_ERR;

    if (xpSaiGetExitStatus() <= 0)
    {
        xpSaiSetExitStatus();
    }

    sleep(1);
    printf("Inside %s\n", __FUNCTION__);

    if ((status = xpsProcessTerminationHandler()) != XP_NO_ERR)
    {
        printf("xpsProcessTerminationHandler failed. RC: %u\n", status);
    }
    if (XP_CONFIG.userCli)
    {
        pyDalDeInit();
    }

    deInitConfigEntryList();
    xpSaiAppL2CleanUp(devId);
    xpSaiAppL3CleanUp(devId);
    return status;
}

void getPortStat(xpsDevice_t devId, xpsPort_t* devPortList, uint32_t numOfPort)
{
    uint8_t statCounterStart = 0;
    uint8_t statCounterEnd = 31;
    uint32_t portIndex = 0;
    struct xp_Statistics *stat = NULL;
    XP_STATUS ret = XP_NO_ERR;

    stat = (struct xp_Statistics *)malloc(sizeof(struct xp_Statistics));
    if (!stat)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for stat\n");
        return;
    }

    for (portIndex = 0; portIndex < numOfPort; portIndex++)
    {
        memset(stat, 0, sizeof(struct xp_Statistics));
        printf("-------------\n");
        printf("Port#:    %3d\n", devPortList[portIndex]);
        printf("-------------\n");
        ret = xpLinkManagerCounterStatsDirectGet(devId, devPortList[portIndex],
                                                 statCounterStart, statCounterEnd, stat);

        if (ret == XP_NO_ERR)
        {
            printf("RxOk        0x%-16" PRIx64 "   Rx65-127    0x%-16" PRIx64 "\n",
                   stat->frameRxOk, stat->frameRxLength65To127);
            printf("RxAll       0x%-16" PRIx64 "   Rx128-255   0x%-16" PRIx64 "\n",
                   stat->frameRxAll, stat->frameRxLength128To255);
            printf("RxFCSEr     0x%-16" PRIx64 "   Rx256-511   0x%-16" PRIx64 "\n",
                   stat->frameRxFcsErr, stat->frameRxLength256To511);
            printf("FrameAnyEr  0x%-16" PRIx64 "   Rx512-1023  0x%-16" PRIx64 "\n",
                   stat->frameRxAnyErr, stat->frameRxLength512To1023);
            printf("RxOctGood   0x%-16" PRIx64 "   Rx1024-1518     0x%-16" PRIx64 "\n",
                   stat->octetsRxGoodFrame, stat->frameRxLength1024To1518);
            printf("RxOct       0x%-16" PRIx64 "   Rx1519-UP   0x%-16" PRIx64 "\n",
                   stat->octetsRx, stat->frameRxLength1519AndUp);
            printf("RxUC        0x%-16" PRIx64 "   RxFifoFull  0x%-16" PRIx64 "\n",
                   stat->frameRxUnicastAddr, stat->frameDroppedFromRxFIFOFullCondition);
            printf("RxMC        0x%-16" PRIx64 "   TxOK        0x%-16" PRIx64 "\n",
                   stat->frameRxMulticastAddr, stat->frameTransmittedOk);
            printf("RxBC        0x%-16" PRIx64 "   TxAll       0x%-16" PRIx64 "\n",
                   stat->frameRxBroadcastAddr, stat->frameTransmittedAll);
            printf("RxPause     0x%-16" PRIx64 "   TxErr       0x%-16" PRIx64 "\n",
                   stat->frameRxTypePause, stat->frameTransmittedWithErr);
            printf("RxLenEr     0x%-16" PRIx64 "   TxWithOErr  0x%-16" PRIx64 "\n",
                   stat->frameRxLengthErr, stat->octetsTransmittedWithoutErr);
            printf("RxUndSize   0x%-16" PRIx64 "   TxTotal     0x%-16" PRIx64 "\n",
                   stat->frameRxUndersize, stat->octetsTransmittedTotal);
            printf("RxOverSize  0x%-16" PRIx64 "   TxUC        0x%-16" PRIx64 "\n",
                   stat->frameRxOversize, stat->framesTransmittedUnicast);
            printf("RxFragments 0x%-16" PRIx64 "   TxMC        0x%-16" PRIx64 "\n",
                   stat->fragmentsRx, stat->framesTransmittedMulticast);
            printf("RxJabFrag   0x%-16" PRIx64 "   TxBC        0x%-16" PRIx64 "\n",
                   stat->jabberFrameRx, stat->framesTransmittedBroadcast0);
            printf("RxLen64     0x%-16" PRIx64 "   TxPause     0x%-16" PRIx64 "\n",
                   stat->frameRxLength64, stat->framesTransmittedPause);
        }

    }
    free(stat);
}

XP_STATUS xpSaiAppConfigureMirrorData(xpDevice_t devId,
                                      sai_object_id_t switchOid)
{
    XP_STATUS status = XP_NO_ERR;
    static MirrorEntry mirrorEntry;
    int entry= 0;
    int analyzer= 0;
    int src= 0;
    uint32_t tnlIdx = 0;
    uint32_t analyzerIntfId  =0;
    uint32_t analyzerId  =0;
    sai_object_id_t portOid = 0, mirrorOid = 0;
    sai_attribute_t port_attr;
    sai_attribute_t *mirror_attr = (sai_attribute_t *)malloc(sizeof(
                                                                 sai_attribute_t)*9);
    if (mirror_attr == NULL)
    {
        printf("Error allocating memory for mirror attribute\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    memset(mirror_attr, 0, sizeof(sai_attribute_t)*9);
    sai_mac_t macSa;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    memset(&macSa, 0, sizeof(macSa));
    printf("In mirror/ERSPAN2 programming ...\n");
    for (entry= 0; entry < numOfEntry(&configFile.mirrorData); ++entry)
    {
        mirrorEntry = *((MirrorEntry *)getEntry(&configFile.mirrorData, entry));
        for (analyzer= 0; analyzer < (int) mirrorEntry.numAnalyzers; ++analyzer)
        {
            if (mirrorEntry.mirrorType == 0)
            {
                // Fetch port interface id from port db
                if ((status = xpsPortGetPortIntfId(devId,
                                                   mirrorEntry.data.mirrorData.analyzers[analyzer], &analyzerIntfId)) != XP_NO_ERR)
                {
                    printf("%s:Error: couldn't get portIntf id for device: %d, port: %d\n",
                           __FUNCTION__, devId, analyzer);
                    free(mirror_attr);
                    return status;
                }
                if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                                  (sai_uint64_t)analyzerIntfId, &portOid)) != SAI_STATUS_SUCCESS)
                {
                    printf("Error : SAI object can not be created.\n");
                    free(mirror_attr);
                    return saiStatus2XpsStatus(saiRetVal);
                }

                mirror_attr[0].id = SAI_MIRROR_SESSION_ATTR_TYPE;
                mirror_attr[0].value.s32 = SAI_MIRROR_SESSION_TYPE_LOCAL;
                mirror_attr[1].id = SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
                mirror_attr[1].value.oid = portOid;

                /*create mirror session Id */
                saiRetVal = xpSaiMirrorApi->create_mirror_session(&mirrorOid, switchOid, 2,
                                                                  mirror_attr);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    printf("%s:Error:Failed to create mirror session | retVal : %d \n",
                           __FUNCTION__, saiRetVal);
                    free(mirror_attr);
                    return saiStatus2XpsStatus(saiRetVal);
                }
            }
            else
            {
                int i = 0;
                TunnelEntry tunnelEntry;
                tnlIdx = mirrorEntry.data.erspanData.tunnelTableIndex;

                for (i = 0; i < NUM_OF_TUNNELS; i++)
                {
                    tunnelEntry = *((TunnelEntry *)getEntry(&configFile.tunnelData, i));
                    if (tnlIdx == tunnelEntry.tnlIdx)
                    {
                        break;
                    }
                }

                if ((status = xpsPortGetPortIntfId(devId, tunnelEntry.portId,
                                                   &analyzerIntfId)) != XP_NO_ERR)
                {
                    printf("%s:Error: couldn't get portIntf id for device: %d, port: %d\n",
                           __FUNCTION__, devId, tunnelEntry.portId);
                    free(mirror_attr);
                    return status;
                }
                if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                                  (sai_uint64_t)analyzerIntfId, &portOid)) != SAI_STATUS_SUCCESS)
                {
                    printf("Error : SAI object can not be created.\n");
                    free(mirror_attr);
                    return saiStatus2XpsStatus(saiRetVal);
                }

                mirror_attr[0].id = SAI_MIRROR_SESSION_ATTR_TYPE;
                mirror_attr[0].value.s32 = SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE;
                mirror_attr[1].id = SAI_MIRROR_SESSION_ATTR_VLAN_ID;
                mirror_attr[1].value.u16 = tunnelEntry.bdId;
                mirror_attr[2].id = SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
                mirror_attr[2].value.oid = portOid;
                mirror_attr[3].id = SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS;
                memcpy(mirror_attr[3].value.mac, tunnelEntry.dmac, sizeof(sai_mac_t));

                sai_ip4_t ipv4AddrSrc, ipv4AddrDst;
                uint32_t len = sizeof(sai_ip4_t);
                uint32_t j = 0;

                for (j = 0; j < len; ++j)
                {
                    ((uint8_t*)&ipv4AddrSrc)[len-1-j] = tunnelEntry.myVtepIp[j];
                    ((uint8_t*)&ipv4AddrDst)[len-1-j] = tunnelEntry.dip[j];
                }
                mirror_attr[4].id = SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS;
                mirror_attr[4].value.ipaddr.addr.ip4 = ipv4AddrSrc;
                mirror_attr[4].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
                mirror_attr[5].id = SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS;
                mirror_attr[5].value.ipaddr.addr.ip4 = ipv4AddrDst;
                mirror_attr[5].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

                uint32_t bdId = (uint32_t) tunnelEntry.bdId;
                xpsL3GetIntfEgressRouterMacLSB(devId, XPS_INTF_MAP_BD_TO_INTF(bdId), &macSa[0]);
                mirror_attr[6].id = SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS;
                memcpy(mirror_attr[6].value.mac, macSa, sizeof(sai_mac_t));

                mirror_attr[7].id = SAI_MIRROR_SESSION_ATTR_ERSPAN_ENCAPSULATION_TYPE;
                mirror_attr[7].value.s32 = SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL;

                mirror_attr[8].id = SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION;
                mirror_attr[8].value.s32 = SAI_IP_ADDR_FAMILY_IPV4;

                /*create mirror session Id */
                saiRetVal = xpSaiMirrorApi->create_mirror_session(&mirrorOid, switchOid, 9,
                                                                  mirror_attr);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    printf("%s:Error:Failed to create mirror session | retVal : %d \n",
                           __FUNCTION__, saiRetVal);
                    free(mirror_attr);
                    return saiStatus2XpsStatus(saiRetVal);
                }
            }

            for (src= 0; src < (int) mirrorEntry.numSources; ++src)
            {
                switch (mirrorEntry.sources[src].type)
                {
                    case TYPE_PORT:
                        if ((status = xpsPortGetPortIntfId(devId, mirrorEntry.sources[src].id.port,
                                                           &analyzerIntfId)) != XP_NO_ERR)
                        {
                            printf("%s: Error: Failed to get the interface ID for device : %d, port : %d\n",
                                   __FUNCTION__, devId, mirrorEntry.sources[src].id.port);
                            free(mirror_attr);
                            return status;
                        }
                        if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                                          (sai_uint64_t)analyzerIntfId, &portOid)) != SAI_STATUS_SUCCESS)
                        {
                            printf("Error : SAI object can not be created.\n");
                            free(mirror_attr);
                            return saiStatus2XpsStatus(saiRetVal);
                        }

                        port_attr.value.objlist.count = 1;
                        port_attr.value.objlist.list = (sai_object_id_t *)calloc(
                                                           port_attr.value.objlist.count, sizeof(sai_object_id_t));
                        memset(port_attr.value.objlist.list, 0x0,
                               port_attr.value.objlist.count * sizeof(sai_object_id_t));
                        port_attr.value.objlist.list[0] = mirrorOid;
                        if (mirrorEntry.dir == XP_INGRESS)
                        {
                            port_attr.id = SAI_PORT_ATTR_INGRESS_MIRROR_SESSION;

                            saiRetVal = xpSaiPortApi->set_port_attribute(portOid, &port_attr);
                            if (saiRetVal != SAI_STATUS_SUCCESS)
                            {
                                printf("%s:Error:Failed to set attribute for port %d | retVal : %d \n",
                                       __FUNCTION__, analyzerIntfId, saiStatus2XpsStatus(saiRetVal));
                                free(mirror_attr);
                                return saiStatus2XpsStatus(saiRetVal);
                            }
                        }
                        else if (mirrorEntry.dir == XP_EGRESS)
                        {
                            port_attr.id = SAI_PORT_ATTR_EGRESS_MIRROR_SESSION;

                            saiRetVal = xpSaiPortApi->set_port_attribute(portOid, &port_attr);
                            if (saiRetVal != SAI_STATUS_SUCCESS)
                            {
                                printf("%s:Error:Failed to set attribute for port %d | retVal : %d \n",
                                       __FUNCTION__, analyzerIntfId, saiRetVal);
                                free(mirror_attr);
                                return saiStatus2XpsStatus(saiRetVal);
                            }
                        }
                        free(port_attr.value.objlist.list);
                        break;

                    case TYPE_VLAN:
                        analyzerId = (uint32_t)xpSaiObjIdValueGet(mirrorOid);
                        if ((status = xpsVlanSetMirrorToAnalyzer(devId,
                                                                 mirrorEntry.sources[src].id.vlan, true, analyzerId)) != XP_NO_ERR)
                        {
                            free(mirror_attr);
                            return status;
                        }
                        break;

                    case TYPE_LAG:
                        analyzerId = (uint32_t)xpSaiObjIdValueGet(mirrorOid);
                        analyzerIntfId = gLagIntfId[mirrorEntry.sources[src].id.lag];
                        if ((status = xpsLagEnableMirroring(devId, analyzerIntfId,
                                                            analyzerId)) != XP_NO_ERR)
                        {
                            printf("xpSaiAppConfigureMirrorData: Error returned when adding source %1d in table entry #%1d.  Error code = %1d.\n",
                                   src, entry, (int) status);
                            free(mirror_attr);
                            return status;
                        }
                        break;
                    default:
                        printf("xpAppConfigureMirrorData: Unknown source type %1d in table entry #%1d.\n",
                               (int)mirrorEntry.sources[src].type, entry);
                        free(mirror_attr);
                        return XP_ERR_KEY_NOT_FOUND;
                }
            }
        }
    }

    printf("xpSaiAppConfigureMirrorData: Mirror entries programmed successfully.\n");
    free(mirror_attr);
    return status;
}


void xpSaiAppIaclRemarkEn(xpsDevice_t devId)
{
    int i, profileId;
    uint16_t maxNetworkPorts = 0;

    // By default configure the trust profile

    // Start with trust l2 (4 bits pcp/dei input, profile 0, same values, tc is enumerated, dp is not set)
    for (i = 0; i < (1 << 4); i++)
    {
        uint32_t pcp = (i >> 1) & (0x7);
        uint32_t dei = (i & 0x1);
        uint32_t tc = pcp;

        xpsQosPortIngressSetTrafficClassForL2QosProfile(devId, QOSMAP_PROFILE_TRUST,
                                                        pcp, dei, tc);
        xpsQosPortIngressRemapPriorityForL2QosProfile(devId, QOSMAP_PROFILE_TRUST, pcp,
                                                      dei, pcp, dei, 0, 0);

        // By default, traffic class for other L2Qos profiles should be 0
        for (profileId = 1; profileId < QOSMAP_PROFILE_MAX; profileId++)
        {
            xpsQosPortIngressSetTrafficClassForL2QosProfile(devId, (xpQosMapPfl_t)profileId,
                                                            pcp, dei, 0);
        }
    }

    // Now set the trust l3 (6 bits dscp input, profile 0, same values, tc is enumerated and repeated, dp is not set)
    for (i = 0; i < (1 << 6); i++)
    {
        uint32_t pcp = (i >> 1) & (0x7);
        uint32_t dei = (i & 0x1);
        uint32_t dscp = i;
        uint32_t tc  = (i >> 3);   // TC is unique for each DSCP group (8 in a group)

        xpsQosPortIngressSetTrafficClassForL3QosProfile(devId, QOSMAP_PROFILE_TRUST,
                                                        dscp, tc);
        xpsQosPortIngressRemapPriorityForL3QosProfile(devId, QOSMAP_PROFILE_TRUST, dscp,
                                                      pcp, dei, dscp, 0);
    }

    // Now set the trust mpls (3 bits exp input, profile 0, save vames, tc is enumerated and repeated, dp is not set)
    for (i = 0; i < (1 << 3); i++)
    {
        uint32_t pcp = i;
        uint32_t dei = 0;
        uint32_t dscp = i;

        xpsQosPortIngressSetTrafficClassForMplsQosProfile(devId, QOSMAP_PROFILE_TRUST,
                                                          i, i);
        xpsQosPortIngressRemapPriorityForMplsQosProfile(devId, QOSMAP_PROFILE_TRUST, i,
                                                        pcp, dei, dscp, i);
    }
    // By default we will only use the Trust L2 profile for a port, we will explicitly enable it for every port in the system
    xpsGlobalSwitchControlGetMaxNumPhysicalPorts(devId, &maxNetworkPorts);

    for (i = 0; i < maxNetworkPorts; i++)
    {
        xpsQosPortIngressSetTrustL2ForPort(devId, i);
    }
    xpsQosSetPcpDeiRemarkEn(devId, 0x1);
    xpsQosSetDscpRemarkEn(devId, 0x1);

}

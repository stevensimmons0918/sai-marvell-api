// xpsApp.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

//xpsApp and IPC infrastructure
#include "xpsApp.h"
#include "xpsGlobalSwitchControl.h"


//#include "xpCtrlMac.h"
#include "xpsAppL2.h"
#include "xpsAppL3.h"
#include "xpsAppTnl.h"
#include "xpsAppMpls.h"
#include "xpsAppQos.h"
#include "xpsAppCopp.h"
#include "xpsScope.h"

#include "xpsOpenflow.h"
#include "xpsMac.h"
#include "assert.h"
#include "xpAppConfig.h"
#include "xpPyInc.h"
extern XP_STATUS xpAppTestIaclAddDevice(xpDevice_t devNum);
extern uint32_t tnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t vlanToL3Intf[4096];
extern uint32_t l3TnlIntf[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlType[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t tnlVirtualId[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern uint32_t pbbTnlIsid[XP_MAX_DEVICES][NUM_OF_TUNNELS];
extern xpsInterfaceId_t gLagIntfId[XP_MAX_DEVICES][MAX_NO_LAGS];
extern uint32_t
multicastVif[XP_MAX_DEVICES][XP_MULTICAST_TYPE_MAX][MAX_MULTICAST_ENTRIES];
int32_t mvifToIfListIdMap[128];
extern xpSem_t *rxSemaphore[MAX_RX_QUEUES];

uint32_t xpEaclV4KeyByteMask[] =
{
    0x1,
    0x3f,
    0x3f,
    0xf,
    0xf,
    0x3,
    0x3,
    0x1,
    0x1,
    0x3,
    0x3,
    0x7,
    0x3,
    0x7,
    0x1,
};

uint32_t xpEaclV6KeyByteMask[] =
{
    0x1,
    0xffff,
    0xffff,
    0x3,
    0x3,
    0x3,
    0x1,
    0x3,
    0x3,
    0x3,
};
#define GET_INDEX_FOR_KEY_ENTRY_FORMAT(enumVal, keyFldLst, keyEntryFormatIndex) \
{ \
    keyEntryFormatIndex = 0; \
    int loopIdx; \
    for(loopIdx = 0; loopIdx <= enumVal; loopIdx++) \
        if(keyFldLst[loopIdx] == 1) \
            keyEntryFormatIndex++; \
    keyEntryFormatIndex--; \
}

#define XP_MAX_CUST_ACL_KEY_TYPE 2

//device agnostic initialization of sdk which requires system specific range profile
XP_STATUS xpsAppInit(XP_DEV_TYPE_T devType, xpsRangeProfileType_t rpType,
                     xpsInitType_t initType)
{
    XP_STATUS status = XP_NO_ERR;

    //SDK api to print/log version information
    status = xpsSdkLogVersion();
    if (status)
    {
        return status;
    }

    if ((status = xpsSdkInit(devType, rpType, initType)) != XP_NO_ERR)
    {
        return status;
    }

    return XP_NO_ERR;
}

XP_STATUS xpAppDeInit(void)
{
    XP_STATUS status = XP_NO_ERR;

    status = xpsSdkDeInit();
    return status;
}
XP_STATUS xpsAppInitDevice(xpsDevice_t devId, xpsInitType_t initType,
                           xpDevConfigStruct *devConfig)
{
    XP_STATUS status = XP_NO_ERR;
    xpDmaConfigInfo dmaConfig;
    struct timeval currTime;
    unsigned long int tmpTime = 0, totalTime = 0;

    dmaConfig.pktBufSize = xpAppConf.pktBufSize;
    dmaConfig.txDescDepth = xpAppConf.txDescDepth;
    dmaConfig.numOfTxQueue = xpAppConf.numOfTxQueue;
    dmaConfig.rxDescDepth = xpAppConf.rxDescDepth;
    dmaConfig.numOfRxQueue = xpAppConf.numOfRxQueue;
    dmaConfig.completionBoundary = xpAppConf.completionBoundary;
    dmaConfig.enablePrefetch = xpAppConf.enablePrefetch;
    dmaConfig.enableDescRing = xpAppConf.enableDescRing;
    dmaConfig.fetchRetryCount = xpAppConf.fetchRetryCount;
    devConfig->dmaConfig = &dmaConfig;

    PRINT_LOG("xpsAppInitDevice(devId = 0x%x)\n", devId);
    gettimeofday(&currTime, NULL);
    tmpTime = currTime.tv_sec * (uint32_t)1000;
    PRINT_LOG("ChipInit start timeStamp: %ld\n", tmpTime);

    status = xpsSdkDevInit(devId, initType, devConfig);
    if (status)
    {
        return status;
    }
    PRINT_LOG("xpsAppInitDevice(devId = 0x%x) done.\n", devId);

    status = xpsScopeAddDevice(devId, XP_SCOPE_DEFAULT);
    PRINT_LOG("xpsScopeAddDevice(devId = 0x%x) rc %d.\n", devId, status);
    if (status)
    {
        return status;
    }

    gettimeofday(&currTime, NULL);
    PRINT_LOG("ChipInit end timeStamp: %ld\n", (currTime.tv_sec * (uint32_t)1000));
    totalTime = (currTime.tv_sec * (uint32_t)1000) - tmpTime;
    PRINT_LOG("Total execution time for chipInit : %ld msec\n", totalTime);
    PRINT_LOG("xpsAppInitDevice(devId = 0x%x) done.\n", devId);

    return XP_NO_ERR;
}

XP_STATUS xpAppDeInitDevice(xpsDevice_t devId)
{
    XP_STATUS status = XP_NO_ERR;
    PRINT_LOG("xpAppDeInitDevice(devId = 0x%x)\n", devId);

    //xps specific removal of a device
    if ((status = xpsSdkDevDeInit(devId)) != XP_NO_ERR)
    {
        return status;
    }

    PRINT_LOG("xpAppDeInitDevice(devId = 0x%x) done.\n", devId);
    return status;
}

// Populate Feature Tables for various features
XP_STATUS xpsAppPopulateTables(xpsDevice_t devId, xpsInitType_t initType,
                               xpDevConfigStruct* config)
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
    //uint32_t numOfL3OIFEntry = 0;
    uint32_t numOfMvifEntry = 0;
    uint32_t numOfTenantEntry = 0;
    uint32_t numOfQosSchedulerEntry = 0;
    uint32_t numOfQosShaperPortEntry = 0;
    uint32_t numOfQosShaperQueueEntry = 0;
    uint32_t numOfQosAqmQProfileEntry = 0;
    uint32_t numOfQosEgressMapEntry = 0;
    uint32_t numOfVlanScopeEntry = 0;
    uint32_t numOfFdbScopeEntry = 0;
    uint32_t numOfNhScopeEntry = 0;
    uint32_t numOfIpv4HostScopeEntry = 0;
    uint32_t numOfIpv4RouteScopeEntry = 0;
    uint32_t numOfFdbControlMacEntry = 0;
    uint32_t numOfIpv4HostControlEntry = 0;
    uint32_t numOfIpv6HostControlEntry = 0;
    uint32_t numOfCoppEntry = 0;
    uint32_t numOfTrustBrEntry = 0;
    uint32_t numOfVifEntry = 0;
    uint32_t pbbTunnelId = 0;
    uint32_t numOfSrhEntry = 0;
    uint32_t numOfLocalSidEntry = 0;

    PRINT_LOG("Initializing LPM Route Manager Infrastructure \n");

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
    //numOfL3OIFEntry = numOfEntry(&configFile.l3OIFEntry);
    numOfMvifEntry = numOfEntry(&configFile.mvifData);
    numOfQosSchedulerEntry = numOfEntry(&configFile.qosSchedulerData);
    numOfQosShaperPortEntry = numOfEntry(&configFile.qosShaperPortData);
    numOfQosShaperQueueEntry = numOfEntry(&configFile.qosShaperQueueData);
    numOfQosAqmQProfileEntry = numOfEntry(&configFile.qosAqmQProfileData);
    numOfQosEgressMapEntry = numOfEntry(&configFile.qosEgressMapData);
    numOfVlanScopeEntry = numOfEntry(&configFile.vlanScopeData);
    numOfFdbScopeEntry = numOfEntry(&configFile.macScopeData);
    numOfNhScopeEntry = numOfEntry(&configFile.ipvxNhScopeData);
    numOfIpv4HostScopeEntry = numOfEntry(&configFile.ipv4HostScopeData);
    numOfIpv4RouteScopeEntry = numOfEntry(&configFile.ipv4RouteScopeData);
    numOfFdbControlMacEntry = numOfEntry(&configFile.fdbControlMacData);
    numOfIpv4HostControlEntry = numOfEntry(&configFile.ipv4HostControlData);
    numOfIpv6HostControlEntry = numOfEntry(&configFile.ipv6HostControlData);
    numOfCoppEntry = numOfEntry(&configFile.coppData);
    numOfTrustBrEntry = numOfEntry(&configFile.trustBrData);
    numOfVifEntry = numOfEntry(&configFile.vifData);
    numOfSrhEntry = numOfEntry(&configFile.srhData);
    numOfLocalSidEntry = numOfEntry(&configFile.mySidData);

    PRINT_LOG("numOfPortConfigEntry = %d \n", numOfPortConfigEntry);
    PRINT_LOG("numOfIpv4HostEntry = %d \n", numOfIpv4HostEntry);
    PRINT_LOG("numOfIpv4RouteEntry = %d \n", numOfIpv4RouteEntry);
    PRINT_LOG("numOfControlMacEntry = %d \n", numOfControlMacEntry);
    PRINT_LOG("numFdbEntry = %d \n", numOfFdbEntry);
    PRINT_LOG("numOfVlanEntry = %d \n", numOfVlanEntry);
    PRINT_LOG("numOfTenantEntry = %d \n", numOfTenantEntry);
    PRINT_LOG("numOfLagEntry = %d \n", numOfLagEntry);
    //PRINT_LOG("numOfPBBTnlEntry = %d \n", numOfPBBTnlEntry);
    PRINT_LOG("numOfMplsTnlEntry = %d \n", numOfMplsTnlEntry);
    PRINT_LOG("numOfMplsLabelEntry = %d \n", numOfMplsLabelEntry);
    PRINT_LOG("numOfIpv6HostEntry = %d \n", numOfIpv6HostEntry);
    PRINT_LOG("numOfIpv6RouteEntry = %d \n", numOfIpv6RouteEntry);
    PRINT_LOG("numOfIpv4BridgeMcEntry = %d \n", numOfIpv4BridgeMcEntry);
    PRINT_LOG("numOfMvifEntry = %d \n", numOfMvifEntry);
    PRINT_LOG("numOfIpv4RouteMcEntry = %d \n", numOfIpv4RouteMcEntry);
    PRINT_LOG("numOfIpv4PimBiDirEntry = %d \n", numOfIpv4PimBiDirEntry);
    //PRINT_LOG("numOfL3OIFEntry = %d \n", numOfL3OIFEntry);
    PRINT_LOG("numOfMvifEntry = %d \n", numOfMvifEntry);
    PRINT_LOG("numOfVlanScopeEntry = %d \n", numOfVlanScopeEntry);
    PRINT_LOG("numOfFdbScopeEntry = %d \n", numOfFdbScopeEntry);
    PRINT_LOG("numOfNhScopeEntry = %d \n", numOfNhScopeEntry);
    PRINT_LOG("numOfIpv4HostScopeEntry = %d \n", numOfIpv4HostScopeEntry);
    PRINT_LOG("numOfIpv4RouteScopeEntry = %d \n", numOfIpv4RouteScopeEntry);
    PRINT_LOG("numOfVifEntry = %d \n", numOfVifEntry);
    PRINT_LOG("numOfSrhEntry = %d \n", numOfSrhEntry);
    PRINT_LOG("numOfLocalSidEntry = %d \n", numOfLocalSidEntry);

    //
    // Populate VLAN and portVlan Tables
    //

    for (entryNum = 0; entryNum < numOfVlanEntry; entryNum++)
    {
        VlanEntry * vlanEntry = (VlanEntry *)getEntry(&configFile.vlanData, entryNum);

        err = xpsAppConfigureVlan(devId, vlanEntry, numOfMvifEntry);
        if (err != XP_NO_ERR)
        {
            PRINT_LOG("Error: programming Vlan data through xpAppConfigureVlan, error code = %d\n",
                      err);
            return err;
        }
    }
    PRINT_LOG("xpApp populate VLAN done.\n");

    for (entryNum = 0; entryNum < numOfTenantEntry; entryNum++)
    {
        VlanTenantIdEntry * tenantEntry = (VlanTenantIdEntry *)getEntry(
                                              &configFile.vlanTenantIdData, entryNum);
        err = xpsAppConfigureTenant(devId, tenantEntry);
        if (err != XP_NO_ERR)
        {
            PRINT_LOG("Error: programming Tenant data through xpsAppConfigureTenant, error code = %d\n",
                      err);
        }
    }
    PRINT_LOG("xpApp populate Tenants done.\n");

    //Configure the portDefaultVlaId
    for (entryNum = 0; entryNum < numOfPortConfigEntry; entryNum++)
    {
        //PRINT_LOG("\nIn PortConfig Progarmming\n");
        xpsAppWritePortIvifEntry(devId,
                                 (PortConfigEntry *)getEntry(&configFile.portConfigData, entryNum));
    }
    PRINT_LOG("xpApp populate port configuration done.\n");
    //
    // Populate SRH entries
    //
    uint32_t srIntList[numOfSrhEntry];
    for (entryNum = 0; entryNum < numOfSrhEntry; entryNum++)
    {
        SrhEntry *srhEntry = (SrhEntry *)getEntry(&configFile.srhData, entryNum);
        xpsInterfaceId_t srIntf = 0;
        if (!srhEntry)
        {
            continue;
        }
        err = xpsAppConfigSrInterface(devId, srhEntry, &srIntf);
        if (err != XP_NO_ERR)
        {
            PRINT_LOG("Error: Configure SR interface, error code = %d\n", err);
        }
        srIntList[entryNum] = srIntf;
    }
    //Configure the Nh for Tunnel Entries
    for (entryNum = 0; entryNum < numOfNhEntry; entryNum++)
    {
        PRINT_LOG("\nIn tnlNh Programming\n");
        IpxNhEntry *tnlNhData = (IpxNhEntry *)getEntry(&configFile.ipvxNhData,
                                                       entryNum);
        if (tnlNhData->nhType == 1)
        {
            xpsAppPopulateNh(devId, tnlNhData);
        }
    }

    for (entryNum = 0; entryNum < numOfSrhEntry; entryNum++)
    {
        SrhEntry *srhEntry = (SrhEntry *)getEntry(&configFile.srhData, entryNum);
        if (!srhEntry)
        {
            continue;
        }
        err = xpsAppProgramSrhData(devId, srhEntry->numSegment,
                                   (xpsInterfaceId_t)srIntList[entryNum], srhEntry->nhId);
        if (err != XP_NO_ERR)
        {
            PRINT_LOG("Error: programming SRH data, error code = %d\n", err);
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

        err = (XP_STATUS)xpsAppConfigureIpTnnlData(devId, tunnelEntry);
        if (err)
        {
            PRINT_LOG("Error: programming Tunnel data through xpsAppConfigureIpTnnlData, error code = %d\n",
                      err);
            //return err;
        }
    }
    PRINT_LOG("xpApp populate Tunnel  done.\n");

    //PRINT_LOG("xpApp populate Tunnel  done.\n");

    //
    // Populate PBB tables
    //
    for (entryNum = 0; entryNum < numOfPBBTnlEntry; entryNum++)
    {
        if (!entryNum)
        {
            xpsTunnelEnableTermination(devId);
        }
        TunnelPBBEntry *tunnelPBBEntry = (TunnelPBBEntry *)getEntry(
                                             &configFile.tunnelPBBData, entryNum);
        pbbTnlIsid[devId][entryNum] = tunnelPBBEntry->iSID;
        err = (XP_STATUS) xpsAppConfigurePbbTunnelData(devId, tunnelPBBEntry,
                                                       &pbbTunnelId);
        if (err)
        {
            PRINT_LOG("Error: programming Pbbdata through xpsAppConfigurePbbTunnelData, error code = %d\n",
                      err);
            //return err;
        }
        pbbTnlVif[devId][entryNum] = pbbTunnelId;
        PRINT_LOG("entryNum : %d, pbbTunnelId : %d, pbbTnlVif[devId][entryNum]: %d\n",
                  entryNum, pbbTunnelId, pbbTnlVif[devId][entryNum]);
    }
    //
    // Populate the LAG entries
    //
    for (entryNum = 0; entryNum < numOfLagEntry; entryNum++)
    {
        err = xpsAppConfigureLag(devId,
                                 (PortLagEntry *)getEntry(&configFile.portLagData, entryNum));
        if (err)
        {
            PRINT_LOG("Error: programming Lag data through xpsAppConfigureLagData, error code = %d\n",
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

        TunnelMplsEntry *tunnelEntry = (TunnelMplsEntry *)getEntry(
                                           &configFile.tunnelMplsData, entryNum);

        err = (XP_STATUS)xpsAppConfigureMplsTunnelEntry(devId, tunnelEntry);

    }
    PRINT_LOG("xpApp populate MPLS tunnels done.\n");

    //
    // Populate the FDB entries
    //
    for (entryNum = 0; entryNum < numOfFdbEntry; entryNum++)
    {
        MacEntry * xpsFdbData = (MacEntry *)getEntry(&configFile.macData, entryNum);
        xpsAppAddFdb(devId, xpsFdbData, numOfMvifEntry);
    }
    PRINT_LOG("xpApp populate FDB done.\n");

    //
    // Populate NH Table
    //
    for (entryNum = 0; entryNum < numOfNhEntry; entryNum++)
    {
        PRINT_LOG("\nIn ipvxNh Programming\n");
        IpxNhEntry *ipvxNhData = (IpxNhEntry *)getEntry(&configFile.ipvxNhData,
                                                        entryNum);
        if (ipvxNhData->nhType == 0)
        {
            xpsAppPopulateNh(devId, ipvxNhData);
        }
    }


    //Configure the Mpls Label Table
    for (entryNum = 0; entryNum < numOfMplsLabelEntry; entryNum++)
    {
        xpsAppConfigureMplsLabelEntry(devId,
                                      (MplsLabelEntry *)getEntry(&configFile.mplsLabelData, entryNum));
    }
    PRINT_LOG("xpApp populate MPLS label done.\n");

    //
    // Populate Ipv4Host Table
    //
    for (entryNum = 0; entryNum < numOfIpv4HostEntry; entryNum++)
    {
        PRINT_LOG("\nIn Ipv4Host Programming\n");

        Ipv4HostEntry *ipv4HostEntry = (Ipv4HostEntry *)getEntry(
                                           &configFile.ipv4HostData, entryNum);

        err = xpsAppWriteIpv4HostEntry(devId, ipv4HostEntry);

    }

    //
    // Populate Ipv4Route Table
    //
    //

#if 1
    for (entryNum = 0; entryNum < numOfIpv4RouteEntry; entryNum++)
    {
        PRINT_LOG("\nIn Ipv4Route Programming\n");

        Ipv4RouteEntry *ipv4RouteEntry = (Ipv4RouteEntry *)getEntry(
                                             &configFile.ipv4RouteData, entryNum);

        err = xpsAppWriteIpv4RouteEntry(devId, ipv4RouteEntry);
    }
    //xpsAppL3UcDisplayPrefixes(devId);
#else
    //xpsAppL3UcAddIpv6Prefix(devId);
    //xpsAppL3UcAddPrefix(devId);
#endif

    //
    // Populate ControlMac Table
    //
    for (entryNum = 0; entryNum < numOfControlMacEntry; entryNum++)
    {
        PRINT_LOG("\nIn ControlMac Programming\n");
        xpsAppWriteControlMacEntry(devId,
                                   (ControlMacEntry *)getEntry(&configFile.controlMacData, entryNum));
    }

    //
    // Populate Ipv6Host Table
    //
    for (entryNum = 0; entryNum < numOfIpv6HostEntry; entryNum++)
    {
        PRINT_LOG("\nIn Ipv6Host Programming\n");

        Ipv6HostEntry *ipv6HostEntry = (Ipv6HostEntry *)getEntry(
                                           &configFile.ipv6HostData, entryNum);

        err = xpsAppWriteIpv6HostEntry(devId, ipv6HostEntry);

    }
    //
    //     Populate  Ipv6Route Table
    //
    for (entryNum = 0; entryNum < numOfIpv6RouteEntry; entryNum++)
    {
        PRINT_LOG("\nIn Ipv6Route Programming\n");
        Ipv6RouteEntry *ipv6RouteEntry = (Ipv6RouteEntry *)getEntry(
                                             &configFile.ipv6RouteData, entryNum);

        err = xpsAppWriteIpv6RouteEntry(devId, ipv6RouteEntry);
    }

    //
    // Populate ipv4BridgeMc entries
    //
    PRINT_LOG("\nIpv4 Bridge Mc Programming:\n");
    for (entryNum = 0; entryNum < numOfIpv4BridgeMcEntry; entryNum++)
    {
        xpsAppConfigureIpv4BridgeMcEntry(devId,
                                         (Ipv4BridgeMcEntry *)getEntry(&configFile.ipv4BridgeMcData, entryNum),
                                         numOfMvifEntry, &ipv4BridgeMcHandle);
    }

    //
    // Populate ipv6BridgeMc entries
    //
    PRINT_LOG("\nIpv6 Bridge Mc Programming:\n");
    for (entryNum = 0; entryNum < numOfIpv6BridgeMcEntry; entryNum++)
    {
        xpsAppConfigureIpv6BridgeMcEntry(devId,
                                         (Ipv6BridgeMcEntry *)getEntry(&configFile.ipv6BridgeMcData, entryNum),
                                         numOfMvifEntry, &ipv6BridgeMcHandle);
    }

    //
    // prepare mdt node entry from OIF
    //
    //for(entryNum = 0; entryNum < numOfL3OIFEntry; entryNum++)
    //{
    //xpsAppPreapareL3MdtNodeList((l3MdtEntry *)getEntry(&configFile.l3OIFEntry, entryNum), &mdtL3NodeListData, entryNum);
    //}

    //
    // Populate ipv4RouteMc entries
    //
    PRINT_LOG("\nIpv4 Route Mc Programming:\n");
    for (entryNum = 0; entryNum < numOfIpv4RouteMcEntry; entryNum++)
    {
        Ipv4RouteMcEntry *tempEntry = (Ipv4RouteMcEntry *)getEntry(
                                          &configFile.ipv4RouteMcData, entryNum);
        if ((tempEntry->isMartialIp) > 0)
        {
            xpsAppConfigureIpv4RouteMcControlEntry(devId, tempEntry, &ipv4RouteMcHandle);
        }
        else
        {
            xpsAppConfigureIpv4RouteMcEntry(devId, tempEntry, numOfMvifEntry,
                                            &ipv4RouteMcHandle);
        }
    }

    //
    // Populate ipv6RouteMc entries
    //
    PRINT_LOG("\nIpv6 Route Mc Programming:\n");
    for (entryNum = 0; entryNum < numOfIpv6RouteMcEntry; entryNum++)
    {
        Ipv6RouteMcEntry *tempEntry = (Ipv6RouteMcEntry *)getEntry(
                                          &configFile.ipv6RouteMcData, entryNum);
        if ((tempEntry->isMartialIp) > 0)
        {
            xpsAppConfigureIpv6RouteMcControlEntry(devId, tempEntry, &ipv6RouteMcHandle);
        }
        else
        {
            xpsAppConfigureIpv6RouteMcEntry(devId, tempEntry, numOfMvifEntry,
                                            &ipv6RouteMcHandle);
        }
    }

    //
    // Populate ipv4PimBiDir entries
    //
    PRINT_LOG("\nIpv4 PimBiDir Programming:\n");
    for (entryNum = 0; entryNum < numOfIpv4PimBiDirEntry; entryNum++)
    {
        xpsAppConfigureIpv4PimBiDirEntry(devId,
                                         (Ipv4PimBiDirEntry *)getEntry(&configFile.ipv4PimBiDirData, entryNum),
                                         &ipv4PimBiDirHandle);
    }

    //
    // Populate QoS Scheduler entries
    //
    for (entryNum = 0; entryNum < numOfQosSchedulerEntry; entryNum++)
    {
        xpsAppConfigureQosSchedulerEntry(devId,
                                         (qosSchedulerDataEntry *)getEntry(&configFile.qosSchedulerData, entryNum));
    }

    //
    // Populate QoS Shaper port entries
    //
    for (entryNum = 0; entryNum < numOfQosShaperPortEntry; entryNum++)
    {
        xpsAppConfigureQosShaperPortEntry(devId,
                                          (qosShaperPortDataEntry *)getEntry(&configFile.qosShaperPortData, entryNum));
    }

    //
    // Populate QoS Shaper queue entries
    //
    for (entryNum = 0; entryNum < numOfQosShaperQueueEntry; entryNum++)
    {
        xpsAppConfigureQosShaperQueueEntry(devId,
                                           (qosShaperQueueDataEntry *)getEntry(&configFile.qosShaperQueueData, entryNum));
    }

    //
    // Populate QoS AQM Queue Profile entries
    //
    for (entryNum = 0; entryNum < numOfQosAqmQProfileEntry; entryNum++)
    {
        xpsAppConfigureQosAqmQProfileEntry(devId,
                                           (qosAqmQProfileDataEntry *)getEntry(&configFile.qosAqmQProfileData, entryNum));
    }

    //
    // Populate QoS Egress Map entries
    //
    for (entryNum = 0; entryNum < numOfQosEgressMapEntry; entryNum++)
    {
        xpsAppConfigureEgressQosMapEntry(devId,
                                         (qosEgressMapDataEntry *)getEntry(&configFile.qosEgressMapData, entryNum));
    }

    xpsAppConfigureMacSAMSBsEntry(devId, configFile.macSAMSBs);

    //
    // Populate mySid table entries
    //
    for (entryNum = 0; entryNum < numOfLocalSidEntry; entryNum++)
    {
        localSidEntry *sidEntry = (localSidEntry *)getEntry(&configFile.mySidData,
                                                            entryNum);
        if (!sidEntry)
        {
            continue;
        }
        err = xpsAppConfigureLocalSidData(devId, sidEntry);
        if (err != XP_NO_ERR)
        {
            PRINT_LOG("Error: programming localSid data, error code = %d\n", err);
        }
    }

    if (XP_CONFIG.numScope != 0)
    {
        //
        // Populate Scope VLAN and portVlan Tables
        //
        for (entryNum = 0; entryNum < numOfVlanScopeEntry; entryNum++)
        {
            VlanScopeEntry * vlanScopeEntry = (VlanScopeEntry *)getEntry(
                                                  &configFile.vlanScopeData, entryNum);

            err = xpsAppConfigureVlanScope(devId, vlanScopeEntry, numOfMvifEntry);
            if (err != XP_NO_ERR)
            {
                PRINT_LOG("Error: programming Vlan data through xpAppConfigureVlan, error code = %d\n",
                          err);
                return err;
            }
        }

        //
        // Populate the FDB Scope entries
        //
        for (entryNum = 0; entryNum < numOfFdbScopeEntry; entryNum++)
        {
            MacScopeEntry * xpsFdbScopeData = (MacScopeEntry *)getEntry(
                                                  &configFile.macScopeData, entryNum);
            err = (XP_STATUS)xpsAppAddFdbScope(devId, xpsFdbScopeData, numOfMvifEntry);
            if (err != XP_NO_ERR)
            {
                PRINT_LOG("Error: programming fdb scope entries through xpsAppAddFdbScope, error code = %d\n",
                          err);
                return err;
            }
        }

        //
        // Configure the Nh Scope  Entries
        //
        for (entryNum = 0; entryNum < numOfNhScopeEntry; entryNum++)
        {
            IpxNhScopeEntry *nhScopeData = (IpxNhScopeEntry *)getEntry(
                                               &configFile.ipvxNhScopeData, entryNum);
            if (nhScopeData->nhType == 0)
            {
                err = xpsAppPopulateNhScope(devId, nhScopeData);
                if (err != XP_NO_ERR)
                {
                    PRINT_LOG("Error: programming NH scope entry through xpsAppPopulateNhScope, error code = %d\n",
                              err);
                    return err;
                }
            }
        }

        //
        // Populate Ipv4HostScope Table
        //
        for (entryNum = 0; entryNum < numOfIpv4HostScopeEntry; entryNum++)
        {
            PRINT_LOG("\nIn Ipv4HostScope Programming\n");

            Ipv4HostScopeEntry *ipv4HostScopeEntry = (Ipv4HostScopeEntry *)getEntry(
                                                         &configFile.ipv4HostScopeData, entryNum);

            err = xpsAppWriteIpv4HostScopeEntry(devId, ipv4HostScopeEntry);
            if (err != XP_NO_ERR)
            {
                PRINT_LOG("Error: programming ipv4 host scope entry through xpsAppWriteIpv4HostScopeEntry, error code = %d\n",
                          err);
                return err;
            }

        }

        //
        // Populate Ipv4RouteScope Table
        //
        for (entryNum = 0; entryNum < numOfIpv4RouteScopeEntry; entryNum++)
        {
            PRINT_LOG("\nIn Ipv4RouteScope Programming\n");

            Ipv4RouteScopeEntry *ipv4RouteScopeEntry = (Ipv4RouteScopeEntry *)getEntry(
                                                           &configFile.ipv4RouteScopeData, entryNum);

            err = xpsAppWriteIpv4RouteScopeEntry(devId, ipv4RouteScopeEntry);
            if (err != XP_NO_ERR)
            {
                PRINT_LOG("Error: programming ipv4 route scope entry using xpsAppWriteIpv4RouteScopeEntry, error code = %d\n",
                          err);
                return err;
            }
        }
    }

    //
    // Populate Fdb ControlMac Table
    //
    for (entryNum = 0; entryNum < numOfFdbControlMacEntry; entryNum++)
    {
        FdbControlMacEntry * fdbControlMacEntry = (FdbControlMacEntry *)getEntry(
                                                      &configFile.fdbControlMacData, entryNum);

        err = (XP_STATUS)xpsAppAddFdbControlMac(devId, fdbControlMacEntry);
        if (err != XP_NO_ERR)
        {
            PRINT_LOG("Error: Programming Fdb ControlMac data through xpAppConfigureFdbControlMac, error code = %d\n",
                      err);
            return err;
        }
    }

    //
    // Populate Ipv4 Host Control Table
    //
    for (entryNum = 0; entryNum < numOfIpv4HostControlEntry; entryNum++)
    {
        Ipv4HostControlEntry * ipv4HostControlEntry = (Ipv4HostControlEntry *)getEntry(
                                                          &configFile.ipv4HostControlData, entryNum);

        err = xpsAppWriteIpv4HostControl(devId, ipv4HostControlEntry);
        if (err != XP_NO_ERR)
        {
            PRINT_LOG("Error: Programming Ipv4 Host Control data through xpsAppWriteIpv4HostControl, error code = %d\n",
                      err);
            return err;
        }
    }

    //
    // Populate Ipv6 Host Control Table
    //
    for (entryNum = 0; entryNum < numOfIpv6HostControlEntry; entryNum++)
    {
        Ipv6HostControlEntry * ipv6HostControlEntry = (Ipv6HostControlEntry *)getEntry(
                                                          &configFile.ipv6HostControlData, entryNum);

        err = xpsAppWriteIpv6HostControl(devId, ipv6HostControlEntry);
        if (err != XP_NO_ERR)
        {
            PRINT_LOG("Error: Programming Ipv6 Host Control data through xpsAppWriteIpv6HostControl, error code = %d\n",
                      err);
            return err;
        }
    }

    //
    // Populate trust BR
    //
    for (entryNum = 0; entryNum < numOfTrustBrEntry; entryNum++)
    {
        TrustBrEntry * trustBrEntry = (TrustBrEntry *)getEntry(&configFile.trustBrData,
                                                               entryNum);

        err = xpsAppProgarmTrustBr(devId, trustBrEntry);
        if (err != XP_NO_ERR)
        {
            PRINT_LOG("Error: Programming trust BR data through xpsAppProgarmTrustBr, error code = %d\n",
                      err);
            return err;
        }
    }

    //
    // Populate CoPP Table
    //
    for (entryNum = 0; entryNum < numOfCoppEntry; entryNum++)
    {
        CoppEntry * coppEntry = (CoppEntry *)getEntry(&configFile.coppData, entryNum);

        err = xpsAppWriteCopp(devId, coppEntry);
        if (err != XP_NO_ERR)
        {
            PRINT_LOG("Error: Programming Copp data through xpsAppWriteCopp, error code = %d\n",
                      err);
            return err;
        }
    }

    testMplsTunnel(devId, XP_SINGLE_PIPE);
    testMplsTunnel(devId, XP_SINGLE_PIPE);

    if ((err = xpsAppWriteNatData(devId)) != XP_NO_ERR)
    {
        PRINT_LOG("Failed to configure NAT. Error code %d\n", err);
        return err;
    }

    if ((err = xpsAppConfigureMirrorData(devId)) != XP_NO_ERR)
    {
        PRINT_LOG("Failed to configure Mirror. Error code %d\n", err);
        //TODO: Need to uncomment below lin eonce we resolve the bug(XDK-956)
        return err;
    }


    PRINT_LOG("QOS Remap config\n");
    if ((err = xpsAppWriteQosMap(devId)) != XP_NO_ERR)
    {
        PRINT_LOG("Failed to configure QOS Remap. Error code %d\n", err);
        return err;
    }
    PRINT_LOG("QOS Remap config done \n");

    return err;
}


XP_STATUS xpsAppWritePortIvifEntry(xpsDevice_t devId,
                                   PortConfigEntry *portConfigData)
{
    XP_STATUS retVal;
    xpsPortConfig_t portConfigEntryT;
    memset(&portConfigEntryT, 0x0, sizeof(xpsPortConfig_t));

    xpsInterfaceId_t egressVif;
    retVal = xpsPortGetPortIntfId(devId, portConfigData->evif, &egressVif);
    if (retVal)
    {
        PRINT_LOG("Error : xpsPortGetPortIntfId() Failed for devId %d portId : %d\n",
                  devId, portConfigData->port);
        return retVal;
    }

    retVal = xpsPortGetConfig(devId, portConfigData->port, &portConfigEntryT);
    if (retVal)
    {
        return retVal;
    }

    portConfigEntryT.ingressVif = portConfigData->port;
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
    portConfigEntryT.bumPolicerEn = portConfigData->bumPolicerEn;
    portConfigEntryT.portDefaultDEI = portConfigData->portDefaultDEI;
    portConfigEntryT.portDefaultPCP = portConfigData->portDefaultPCP;
    portConfigEntryT.etagExists = portConfigData->etagExists;

    return xpsPortSetConfig(devId, portConfigData->port, &portConfigEntryT);
}

XP_STATUS xpsAppWriteControlMacEntry(xpsDevice_t devId,
                                     ControlMacEntry* controlMacData)
{
    XP_STATUS retVal;
    macAddr_t keyMAC;
    uint32_t  keyVlanId;

    if (!controlMacData)
    {
        PRINT_LOG("Null pointer received while adding ControlMac Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    keyVlanId = controlMacData->keyVlanId;

    keyMAC[0] = controlMacData->keyMacAddr[0];
    keyMAC[1] = controlMacData->keyMacAddr[1];
    keyMAC[2] = controlMacData->keyMacAddr[2];
    keyMAC[3] = controlMacData->keyMacAddr[3];
    keyMAC[4] = controlMacData->keyMacAddr[4];
    keyMAC[5] = controlMacData->keyMacAddr[5];

    PRINT_LOG("ContromMac Using FL API Entry\n");
    PRINT_LOG("MAC Addr %2x:%2x:%2x:%2x:%2x:%2x \n", keyMAC[5], keyMAC[4],
              keyMAC[3], keyMAC[2], keyMAC[1], keyMAC[0]);
    PRINT_LOG("keyVlanID: %d \n", keyVlanId);

    PRINT_LOG("controlMacData->routerMac: %d \n", controlMacData->routerMac);
    PRINT_LOG("controlMacData->controlActionEn: %d \n",
              controlMacData->controlActionEn);

    if (controlMacData->routerMac == 1)
    {
        retVal =  xpsL3AddIngressRouterMac(devId, keyMAC);
        PRINT_LOG("xpsL3AddIngressRouterMac");
        if (retVal)
        {
            PRINT_LOG("Error in setting xpsL3AddIngressRouterMac \n");
            return retVal;
        }
    }
    else if (controlMacData->routerMac == 0)
    {
        if (controlMacData->controlActionEn == 1)
        {
            retVal =  xpsVlanSetGlobalControlMacWithAction(devId, keyMAC);
            PRINT_LOG("xpsVlanSetGlobalControlMacWithAction");
            if (retVal)
            {
                PRINT_LOG("Error in setting xpsVlanSetGlobalControlMacWithAction \n");
                return retVal;
            }
        }
        else
        {
            retVal =  xpsVlanSetGlobalControlMac(devId, keyMAC);
            PRINT_LOG("xpsVlanSetGlobalControlMac");
            if (retVal)
            {
                PRINT_LOG("Error in setting xpsVlanSetGlobalControlMac \n");
                return retVal;
            }
        }
    }

    //controlMac configuration using PL Manager

    PRINT_LOG("\n\ncontrolMacData->controlMac: %d \n", controlMacData->controlMac);
#if 0 //TODO:: Stale code should be removed after successful execution of apptest
    if (controlMacData->controlMac==1)
    {
        xpControlMacEntry controlMacEntryT, lookupControlMacEntryT;
        int32_t lookupIndex;

        controlMacEntryT.key.IsValid = 1;
        controlMacEntryT.mask.IsValid = 1;

        controlMacEntryT.key.VlanID = controlMacData->keyVlanId;
        controlMacEntryT.mask.VlanID = controlMacData->maskVlanId;
        controlMacEntryT.data.routerMac = controlMacData->routerMac;
        controlMacEntryT.data.controlMac = controlMacData->controlMac;

        controlMacEntryT.key.mac[0] = controlMacData->keyMacAddr[0];
        controlMacEntryT.key.mac[1] = controlMacData->keyMacAddr[1];
        controlMacEntryT.key.mac[2] = controlMacData->keyMacAddr[2];
        controlMacEntryT.key.mac[3] = controlMacData->keyMacAddr[3];
        controlMacEntryT.key.mac[4] = controlMacData->keyMacAddr[4];
        controlMacEntryT.key.mac[5] = controlMacData->keyMacAddr[5];
        controlMacEntryT.mask.mac[0] = controlMacData->maskMacAddr[0];
        controlMacEntryT.mask.mac[1] = controlMacData->maskMacAddr[1];
        controlMacEntryT.mask.mac[2] = controlMacData->maskMacAddr[2];
        controlMacEntryT.mask.mac[3] = controlMacData->maskMacAddr[3];
        controlMacEntryT.mask.mac[4] = controlMacData->maskMacAddr[4];
        controlMacEntryT.mask.mac[5] = controlMacData->maskMacAddr[5];

        //printControlMacEntry(&controlMacEntryT);

        /* Write Entry */
        uint32_t rMac=0x0;
        retVal = xpGetMac(devId, index, controlMacEntryT.key.mac,
                          controlMacEntryT.mask.mac, &rMac);
        if (retVal)
        {
            PRINT_LOG("%s:Error in inserting an entry: error code: %d\n", __FUNCTION__,
                      retVal);
            return retVal;
        }
        controlMacEntryT.data.routerMac = rMac;

        //if((retVal = controlMacMgr->writeEntry(devId, index, (void *)&controlMacEntryT)) != XP_NO_ERR)
        //{
        //    PRINT_LOG("%s:Error in inserting an entry: error code: %d\n", __FUNCTION__, retVal);
        //    return retVal;
        //}

        PRINT_LOG("index = %d\n", index);

        lookupIndex=index;
        PRINT_LOG("Lookup index = %d\n", lookupIndex);
        index++;

        /* Lookup */
        lookupControlMacEntryT.key.VlanID = controlMacData->keyVlanId;
        memcpy(lookupControlMacEntryT.key.mac, controlMacEntryT.key.mac,
               sizeof(macAddr_t));

        //if((retVal = controlMacMgr->lookup(devId, (void*)&lookupControlMacEntryT, lookupIndex)) != XP_NO_ERR)
        //{
        //    PRINT_LOG("%s:Error in entry Lookup: error code: %d\n", __FUNCTION__, retVal);
        //    return retVal;
        //}

        //printControlMacEntry(&lookupControlMacEntryT);

    }
#endif
    return XP_NO_ERR;
}

XP_STATUS xpsAppConfigureMvifEntry(xpsDevice_t devId, MvifEntry *mvifData,
                                   xpsMcL3InterfaceListId_t l3IfListId, xpsMcL2InterfaceListId_t ifListId,
                                   bool isL2)
{
    XP_STATUS retVal;
    uint32_t intfId, i, greIntfId;

    if (!mvifData)
    {
        PRINT_LOG("Error: Null pointer received for mvifData Entry \n");
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
                        PRINT_LOG("%s: Error in getting intfId for portNum %d: error code: %d\n",
                                  __FUNCTION__, mvifData->portList[i], retVal);
                        return retVal;
                    }
                    if ((retVal = xpsMulticastAddInterfaceToL2InterfaceList(devId, ifListId,
                                                                            intfId)) != XP_NO_ERR)
                    {
                        PRINT_LOG("%s: Error in adding %d intfId of Port %d in ifListId %d: error code: %d\n",
                                  __FUNCTION__, intfId, mvifData->portList[i], ifListId, retVal);
                        return retVal;
                    }
                    PRINT_LOG("%s: intfId %d of Port %d added successfully in ifList %d\n",
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
                        PRINT_LOG("%s: Error in getting intfId for portNum %d: error code: %d\n",
                                  __FUNCTION__, mvifData->portList[i], retVal);
                        return retVal;
                    }
                    if ((retVal = xpsMulticastAddInterfaceToL2InterfaceList(devId, ifListId,
                                                                            intfId)) != XP_NO_ERR)
                    {
                        PRINT_LOG("%s: Error in adding %d intfId of Port %d in ifListId %d: error code: %d\n",
                                  __FUNCTION__, intfId, mvifData->portList[i], ifListId, retVal);
                        return retVal;
                    }
                    PRINT_LOG("%s: intfId %d of Port %d added successfully in ifList %d\n",
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
                PRINT_LOG("%s: Error in adding %d ifList for %d intfId for tnlIdx %d: error code: %d\n",
                          __FUNCTION__, ifListId, intfId, mvifData->tnlIdx, retVal);
                return retVal;
            }
            PRINT_LOG("%s: intfId %d added successfully in ifList %d for tnlIdx %d\n",
                      __FUNCTION__, intfId, ifListId, mvifData->tnlIdx);
            break;
        case 7:
            intfId = tnlVif[devId][mvifData->tnlIdx];
            if ((retVal = xpsMulticastAddInterfaceToL2InterfaceList(devId, ifListId,
                                                                    intfId)) != XP_NO_ERR)
            {
                PRINT_LOG("%s: Error in adding %d ifList for %d intfId for tnlIdx %d: error code: %d\n",
                          __FUNCTION__, ifListId, intfId, mvifData->tnlIdx, retVal);
                return retVal;
            }
            PRINT_LOG("%s: intfId %d added successfully in ifList %d for tnlIdx %d\n",
                      __FUNCTION__, intfId, ifListId, mvifData->tnlIdx);
            break;
        case 8:
            intfId = pbbTnlVif[devId][mvifData->tnlIdx];
            if ((retVal = xpsMulticastAddInterfaceToL2InterfaceList(devId, ifListId,
                                                                    intfId)) != XP_NO_ERR)
            {
                PRINT_LOG("%s: Error in adding %d ifList for %d intfId for tnlIdx %d: error code: %d\n",
                          __FUNCTION__, ifListId, intfId, mvifData->tnlIdx, retVal);
                return retVal;
            }
            PRINT_LOG("%s: intfId %d added successfully in ifList %d for tnlIdx %d\n",
                      __FUNCTION__, intfId, ifListId, mvifData->tnlIdx);
            break;
        case 9:
            intfId = l3TnlIntf[devId][mvifData->tnlIdx];
            greIntfId = tnlVif[devId][mvifData->tnlIdx];
            //For GRE
            if ((retVal = xpsMulticastAddInterfaceToL3InterfaceList(devId, l3IfListId,
                                                                    intfId)) != XP_NO_ERR)
            {
                PRINT_LOG("%s: Error in adding intfId to %d l3IfList for %d intfId for tnlIdx %d: error code: %d\n",
                          __FUNCTION__, l3IfListId, intfId, mvifData->tnlIdx, retVal);
                return retVal;
            }
            PRINT_LOG("%s: greIntfId %d: intfId %d added successfully in l3IfList %d for tnlIdx %d\n",
                      __FUNCTION__, greIntfId, intfId, l3IfListId, mvifData->tnlIdx);
            break;

        default:
            PRINT_LOG("Invalid Encap for type = %d\n", mvifData->tnlType);
            return XP_ERR_INVALID_PARAMS;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAppConfigureIpv4BridgeMcEntry(xpsDevice_t devId,
                                           Ipv4BridgeMcEntry *ipv4BridgeMcData, uint32_t numOfMvifEntry, uint32_t *index)
{
    XP_STATUS retVal;
    xpsMcL2InterfaceListId_t ifListId;
    uint32_t  i, entryNum;
    xpsMulticastIPv4BridgeEntry_t ipv4BridgeMcEntry;// getIpv4BridgeMcEntry;

    if (!ipv4BridgeMcData || !index)
    {
        PRINT_LOG("Error: Null pointer received for ipv4BridgeMcData Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    xpsScope_t scopeId;
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        printf("%s:Error: couldn't get ScopeId for device: %d",  __FUNCTION__, devId);
        return retVal;
    }

    if ((retVal = xpsMulticastCreateL2InterfaceListScope(scopeId,
                                                         ipv4BridgeMcData->vlan, &ifListId)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in Creating Interface List for vlan: %d: error code: %d\n",
                  __FUNCTION__, ipv4BridgeMcData->vlan, retVal);
        return retVal;
    }

    // Store the multicast Vif in global db
    multicastVif[devId][XP_MULTICAST_TYPE_IPV4_BRIDGE][ipv4BridgeMcData->idx] =
        ifListId;

    if ((retVal = xpsMulticastAddL2InterfaceListToDevice(devId,
                                                         ifListId)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in Adding Interface List %d to device %d: error code: %d\n",
                  __FUNCTION__, devId, ifListId, retVal);
        return retVal;
    }

    memset(&ipv4BridgeMcEntry, 0, sizeof(xpIPv4MulticastBridgeEntry_t));
    ipv4BridgeMcEntry.pktCmd = (xpMulticastPktCmdType_e) ipv4BridgeMcData->pktCmd;
    ipv4BridgeMcEntry.bdId = ipv4BridgeMcData->vlan;
    ipv4BridgeMcEntry.vlanId = ipv4BridgeMcData->vlan;
    COPY_IPV4_ADDR_T(ipv4BridgeMcEntry.sourceAddress, ipv4BridgeMcData->sip);
    COPY_IPV4_ADDR_T(ipv4BridgeMcEntry.groupAddress, ipv4BridgeMcData->grpAddr);
    ipv4BridgeMcEntry.multicastVifIdx = ifListId;

    if ((retVal = xpsMulticastAddIPv4BridgeEntry(devId,
                                                 &ipv4BridgeMcEntry, 0)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
                  retVal);
        return retVal;
    }

    PRINT_LOG("\naddIPv4MulticastBridgeEntry -> sip : %d.%d.%d.%d grpAddr : %d.%d.%d.%d pktcmd : %d, bdId: %d, mvif: %d\n",
              ipv4BridgeMcEntry.sourceAddress[3], ipv4BridgeMcEntry.sourceAddress[2],
              ipv4BridgeMcEntry.sourceAddress[1], ipv4BridgeMcEntry.sourceAddress[0],
              ipv4BridgeMcEntry.groupAddress[3], ipv4BridgeMcEntry.groupAddress[2],
              ipv4BridgeMcEntry.groupAddress[1], ipv4BridgeMcEntry.groupAddress[0],
              ipv4BridgeMcEntry.pktCmd, ipv4BridgeMcEntry.bdId,
              ipv4BridgeMcEntry.multicastVifIdx);

    *index = 0;

    /*    if ((retVal = xpGetIPv4MulticastBridgeEntry(devId, *index, &getIpv4BridgeMcEntry)) != XP_NO_ERR)
        {
            PRINT_LOG("%s: Error in reading an entry: error code: %d\n", __FUNCTION__, retVal);
            return retVal;
        }
        PRINT_LOG("getIPv4MulticastBridgeEntry -> sip : %d.%d.%d.%d grpAddr : %d.%d.%d.%d pktcmd : %d, bdId: %d, mvif: %d\n", getIpv4BridgeMcEntry.sourceAddress[3], getIpv4BridgeMcEntry.sourceAddress[2], getIpv4BridgeMcEntry.sourceAddress[1], getIpv4BridgeMcEntry.sourceAddress[0], getIpv4BridgeMcEntry.groupAddress[3], getIpv4BridgeMcEntry.groupAddress[2], getIpv4BridgeMcEntry.groupAddress[1], getIpv4BridgeMcEntry.groupAddress[0], getIpv4BridgeMcEntry.pktCmd, getIpv4BridgeMcEntry.bdId, getIpv4BridgeMcEntry.multicastVifIdx);
    */
    if (ipv4BridgeMcData->agingFlag)
    {
        //Enable aging for an entry
        xpsMulticastConfigureIpv4BridgeEntryAging(devId, 1, *index);

        //Trigger two times to make aging-time out
        xpsMulticastTriggerIpv4BridgeTableAging(devId);
        xpsMulticastTriggerIpv4BridgeTableAging(devId);
        xpsMulticastTriggerIpv4BridgeTableAging(devId);

        //Process FIFO to delete the aged out entry with default handler
        xpsAgeFifoHandler(devId);
    }
    else
    {
        //To avoid the cases when default value is non zero for an entry
        xpsMulticastConfigureIpv4BridgeEntryAging(devId, 0, *index);
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
                xpsAppConfigureMvifEntry(devId, (MvifEntry *)getEntry(&configFile.mvifData,
                                                                      entryNum), 0, ifListId, 1);
                break;
            }
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAppConfigureIpv6BridgeMcEntry(xpsDevice_t devId,
                                           Ipv6BridgeMcEntry *ipv6BridgeMcData, uint32_t numOfMvifEntry, uint32_t *index)
{
    XP_STATUS retVal;
    xpsMcL2InterfaceListId_t ifListId;
    uint32_t  i, entryNum;
    xpsHashIndexList_t indexList;

    xpsMulticastIPv6BridgeEntry_t ipv6BridgeMcEntry, getIpv6BridgeMcEntry;

    if (!ipv6BridgeMcData || !index)
    {
        PRINT_LOG("Error: Null pointer received for ipv6BridgeMcData Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    xpsScope_t scopeId;
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        printf("%s:Error: couldn't get ScopeId for device: %d",  __FUNCTION__, devId);
        return retVal;
    }

    if ((retVal = xpsMulticastCreateL2InterfaceListScope(scopeId,
                                                         ipv6BridgeMcData->vlan, &ifListId)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in Creating Interface List for vlan: %d: error code: %d\n",
                  __FUNCTION__, ipv6BridgeMcData->vlan, retVal);
        return retVal;
    }

    // Store the multicast Vif in global db
    multicastVif[devId][XP_MULTICAST_TYPE_IPV6_BRIDGE][ipv6BridgeMcData->idx] =
        ifListId;

    if ((retVal = xpsMulticastAddL2InterfaceListToDevice(devId,
                                                         ifListId)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in Adding Interface List %d to device %d: error code: %d\n",
                  __FUNCTION__, devId, ifListId, retVal);
        return retVal;
    }

    memset(&ipv6BridgeMcEntry, 0, sizeof(xpIPv6MulticastBridgeEntry_t));
    ipv6BridgeMcEntry.pktCmd = (xpMulticastPktCmdType_e) ipv6BridgeMcData->pktCmd;
    ipv6BridgeMcEntry.bdId = ipv6BridgeMcData->vlan;
    ipv6BridgeMcEntry.vlanId = ipv6BridgeMcData->vlan;
    COPY_IPV6_ADDR_T(ipv6BridgeMcEntry.sourceAddress, ipv6BridgeMcData->sip);
    COPY_IPV6_ADDR_T(ipv6BridgeMcEntry.groupAddress, ipv6BridgeMcData->grpAddr);
    ipv6BridgeMcEntry.multicastVifIdx = ifListId;

    if ((retVal = xpsMulticastAddIPv6BridgeEntry(devId, &ipv6BridgeMcEntry,
                                                 &indexList)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
                  retVal);
        return retVal;
    }

    PRINT_LOG("\naddIPv6MulticastBridgeEntry -> sip : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d grpAddr : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d pktcmd : %d, bdId: %d, mvif: %d\n",
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
        PRINT_LOG("!!! Rehash occured!!! NOT TAKING CARE AS OF NOW.\n");
        return XP_NO_ERR;
    }
    if ((retVal = xpsMulticastGetIPv6BridgeEntry(devId, *index,
                                                 &getIpv6BridgeMcEntry)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in reading an entry: error code: %d\n", __FUNCTION__,
                  retVal);
        return retVal;
    }
    PRINT_LOG("xpsMulticastGetIPv6BridgeEntry-> sip : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d grpAddr : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d pktcmd : %d, bdId: %d, mvif: %d, l2DomainId: %d\n",
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
                xpsAppConfigureMvifEntry(devId, (MvifEntry *)getEntry(&configFile.mvifData,
                                                                      entryNum), 0, ifListId, 1);
                break;
            }
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAppConfigureIpv4RouteMcControlEntry(xpsDevice_t devId,
                                                 Ipv4RouteMcEntry *ipv4RouteMcData, uint32_t *index)
{
    XP_STATUS retVal;
    xpsHashIndexList_t indexList;
    ipv4Addr_t grpAddress;

    COPY_IPV4_ADDR_T(grpAddress, ipv4RouteMcData->grpAddr);
    if ((retVal = xpsMulticastAddIpv4RouteControlEntry(devId,
                                                       ipv4RouteMcData->vrfId, grpAddress, ipv4RouteMcData->reasonCode,
                                                       (xpsMulticastPktCmdType_e)ipv4RouteMcData->pktCmd, &indexList)) != XP_NO_ERR)
    {
        PRINT_LOG("%s:Error while programming ipv4 route control entry. Error code: %d\n",
                  __FUNCTION__, retVal);
        return retVal;
    }

    *index = indexList.index[0];

    if (indexList.size > 1)
    {
        PRINT_LOG("Rehash occured.\n");
    }

    return XP_NO_ERR;

}


XP_STATUS xpsAppConfigureIpv6RouteMcControlEntry(xpsDevice_t devId,
                                                 Ipv6RouteMcEntry *ipv6RouteMcData, uint32_t *index)
{
    XP_STATUS retVal;
    xpsHashIndexList_t indexList;
    ipv6Addr_t grpAddress;
    COPY_IPV6_ADDR_T(grpAddress, ipv6RouteMcData->grpAddr);
    if ((retVal = xpsMulticastAddIpv6RouteControlEntry(devId,
                                                       ipv6RouteMcData->vrfId, grpAddress, ipv6RouteMcData->reasonCode,
                                                       (xpsMulticastPktCmdType_e)ipv6RouteMcData->pktCmd, &indexList)) != XP_NO_ERR)
    {
        PRINT_LOG("%s:Error while programming ipv4 route control entry. Error code: %d\n",
                  __FUNCTION__, retVal);
        return retVal;
    }

    *index = indexList.index[0];

    if (indexList.size > 1)
    {
        PRINT_LOG("Rehash occured.\n");
    }

    return XP_NO_ERR;

}

XP_STATUS xpsAppConfigureIpv4RouteMcEntry(xpsDevice_t devId,
                                          Ipv4RouteMcEntry *ipv4RouteMcData, uint32_t numOfMvifEntry, uint32_t *index)
{
    XP_STATUS retVal;
    uint32_t i, j, entryNum = 0, intfId;
    xpsHashIndexList_t indexList;
    xpsMcL2InterfaceListId_t l2IfListId;
    xpsMcL3InterfaceListId_t l3IfListId;
    xpsMulticastIPv4RouteEntry_t ipv4RouteMcEntry, getIpv4RouteMcEntry;

    if (!ipv4RouteMcData || !index)
    {
        PRINT_LOG("Error: Null pointer received for ipv4RouteMcData Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    xpsScope_t scopeId;
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        printf("%s:Error: couldn't get ScopeId for device: %d",  __FUNCTION__, devId);
        return retVal;
    }

    //Create L3 IntfList Id
    if ((retVal = xpsMulticastCreateL3InterfaceListScope(scopeId,
                                                         &l3IfListId)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in Creating L3Interface List: error code: %d\n",
                  __FUNCTION__, retVal);
        return retVal;
    }

    // Store the multicast Vif in global db
    multicastVif[devId][XP_MULTICAST_TYPE_IPV4_ROUTE][ipv4RouteMcData->idx] =
        l3IfListId;

    // Add L3 IntfList to Dev
    if ((retVal = xpsMulticastAddL3InterfaceListToDevice(devId,
                                                         l3IfListId)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in Adding L3Interface List %d to device %d: error code: %d\n",
                  __FUNCTION__, devId, l3IfListId, retVal);
        return retVal;
    }

    memset(&ipv4RouteMcEntry, 0, sizeof(xpsMulticastIPv4RouteEntry_t));
    ipv4RouteMcEntry.pktCmd = (xpMulticastPktCmdType_e) ipv4RouteMcData->pktCmd;
    ipv4RouteMcEntry.rpfType = (xpMulticastRpfCheckType_e) ipv4RouteMcData->rpfType;
    ipv4RouteMcEntry.rpfFailCmd = (xpMulticastRpfFailCmd_e)
                                  ipv4RouteMcData->rpfFailCmd;
    ipv4RouteMcEntry.multicastVifIdx = l3IfListId;
    ipv4RouteMcEntry.vrfIdx = ipv4RouteMcData->vrfId;
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
            PRINT_LOG("%s: Error in getting intfId for rpfVal(portNum) %d: error code: %d\n",
                      __FUNCTION__, ipv4RouteMcData->rpfValue, retVal);
            return retVal;
        }
        ipv4RouteMcEntry.rpfValue = intfId;
    }


    // Populate v4RouteMc Entry
    if ((retVal = xpsMulticastAddIPv4RouteEntry(devId, &ipv4RouteMcEntry,
                                                &indexList)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
                  retVal);
        return retVal;
    }

    PRINT_LOG("\naddIPv4MulticastRouteEntry -> sip : %d.%d.%d.%d grpAddr : %d.%d.%d.%d pktcmd : %d, vrfId: %d, mvif: %d, rpfType : %d, rpfValue : %d, rpfFailCmd : %d\n",
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
        PRINT_LOG("!!! Rehash occured!!! NOT TAKING CARE AS OF NOW.\n");
        return XP_NO_ERR;
    }

    if ((retVal = xpsMulticastGetIPv4RouteEntry(devId, *index,
                                                &getIpv4RouteMcEntry)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in reading an entry: error code: %d\n", __FUNCTION__,
                  retVal);
        return retVal;
    }
    PRINT_LOG("xpsMulticastGetIPv4RouteEntry-> sip : %d.%d.%d.%d grpAddr : %d.%d.%d.%d pktcmd : %d, vrfId: %d, mvif: %d, rpfType : %d, rpfVal : %d, rpfFailCmd : %d\n\n\n",
              getIpv4RouteMcEntry.sourceAddress[3], getIpv4RouteMcEntry.sourceAddress[2],
              getIpv4RouteMcEntry.sourceAddress[1], getIpv4RouteMcEntry.sourceAddress[0],
              getIpv4RouteMcEntry.groupAddress[3], getIpv4RouteMcEntry.groupAddress[2],
              getIpv4RouteMcEntry.groupAddress[1], getIpv4RouteMcEntry.groupAddress[0],
              getIpv4RouteMcEntry.pktCmd, getIpv4RouteMcEntry.vrfIdx,
              getIpv4RouteMcEntry.multicastVifIdx, getIpv4RouteMcEntry.rpfType,
              getIpv4RouteMcEntry.rpfValue, getIpv4RouteMcEntry.rpfFailCmd);

    if (ipv4RouteMcData->agingFlag)
    {
        xpsMulticastConfigureIpv4RouteEntryAging(devId, 1, *index);

        //Trigger two times to make aging-time out
        xpsMulticastTriggerIpv4RouteTableAging(devId);
        xpsMulticastTriggerIpv4RouteTableAging(devId);
        xpsMulticastTriggerIpv4RouteTableAging(devId);

        //Process FIFO to delete the aged out entry with default handler
        xpsAgeFifoHandler(devId);
    }
    else
    {
        //To avoid the cases when default value is non zero for an entry
        xpsMulticastConfigureIpv4RouteEntryAging(devId, 0, *index);
    }


    /* Iterate through all mvifs for all vlans in one L3 Intf*/
    for (i = 0; i < 10; i++)
    {
        if (ipv4RouteMcData->vlanNode[i].vlanId == 0)
        {
            break;
        }
        //Create L2 IntfList for Given Vlan
        if ((retVal = xpsMulticastCreateL2InterfaceListScope(scopeId,
                                                             ipv4RouteMcData->vlanNode[i].vlanId, &l2IfListId)) != XP_NO_ERR)
        {
            PRINT_LOG("%s: Error in Creating Interface List for vlan: %d: error code: %d\n",
                      __FUNCTION__, ipv4RouteMcData->vlanNode[i].vlanId, retVal);
            return retVal;
        }
        PRINT_LOG("%s: Created Interface List %d for vlan %d\n", __FUNCTION__,
                  l2IfListId, ipv4RouteMcData->vlanNode[i].vlanId);
        //Add L2 IntfList to Dev
        if ((retVal = xpsMulticastAddL2InterfaceListToDevice(devId,
                                                             l2IfListId)) != XP_NO_ERR)
        {
            PRINT_LOG("%s: Error in Adding Interface List %d to device %d: error code: %d\n",
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
                    xpsAppConfigureMvifEntry(devId, (MvifEntry *)getEntry(&configFile.mvifData,
                                                                          entryNum), l3IfListId, l2IfListId, 0);
                    break;
                }
            }
        }
        if (((MvifEntry *)getEntry(&configFile.mvifData, entryNum) != NULL) &&
            ((MvifEntry *)getEntry(&configFile.mvifData, entryNum))->tnlType!= 9)
        {
            //Add L2 IntfList to L3 IntfList
            if ((retVal = xpsMulticastAddBridgingInterfaceToL3InterfaceList(devId,
                                                                            l3IfListId, l2IfListId)) != XP_NO_ERR)
            {
                PRINT_LOG("%s: Error in Adding Bridge Interface List %d to L3 List %d: error code: %d\n",
                          __FUNCTION__, l2IfListId, l3IfListId, retVal);
                return retVal;
            }
            PRINT_LOG("%s: Added l2IfListId %d in l3IfListId %d\n", __FUNCTION__,
                      l2IfListId, l3IfListId);
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAppConfigureIpv6RouteMcEntry(xpsDevice_t devId,
                                          Ipv6RouteMcEntry *ipv6RouteMcData, uint32_t numOfMvifEntry, uint32_t *index)
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
        PRINT_LOG("Error: Null pointer received for ipv6RouteMcData Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    xpsScope_t scopeId;
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        printf("%s:Error: couldn't get ScopeId for device: %d",  __FUNCTION__, devId);
        return retVal;
    }
    //Create L3 IntfList Id
    if ((retVal = xpsMulticastCreateL3InterfaceListScope(scopeId,
                                                         &l3IfListId)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in Creating L3Interface List: error code: %d\n",
                  __FUNCTION__, retVal);
        return retVal;
    }

    // Store the multicast Vif in global db
    multicastVif[devId][XP_MULTICAST_TYPE_IPV6_ROUTE][ipv6RouteMcData->idx] =
        l3IfListId;

    // Add L3 IntfList to Dev
    if ((retVal = xpsMulticastAddL3InterfaceListToDevice(devId,
                                                         l3IfListId)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in Adding L3Interface List %d to device %d: error code: %d\n",
                  __FUNCTION__, devId, l3IfListId, retVal);
        return retVal;
    }

    l3IntfId = XPS_INTF_MAP_BD_TO_INTF(ipv6RouteMcData->rpfValue);

    if ((retVal = xpsL3GetV6McL3DomainIdForInterface(devId, l3IntfId,
                                                     &mcL3DomainId)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in Adding L3Interface List %d to device %d: error code: %d\n",
                  __FUNCTION__, devId, l3IfListId, retVal);
        return retVal;
    }

    memset(&ipv6RouteMcEntry, 0, sizeof(xpsMulticastIPv6RouteEntry_t));
    ipv6RouteMcEntry.pktCmd = (xpMulticastPktCmdType_e) ipv6RouteMcData->pktCmd;
    ipv6RouteMcEntry.rpfType = (xpMulticastRpfCheckType_e) ipv6RouteMcData->rpfType;
    ipv6RouteMcEntry.rpfFailCmd = (xpMulticastRpfFailCmd_e)
                                  ipv6RouteMcData->rpfFailCmd;
    ipv6RouteMcEntry.multicastVifIdx = l3IfListId;
    ipv6RouteMcEntry.vrfIdx = ipv6RouteMcData->vrfId;
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
            PRINT_LOG("%s: Error in getting intfId for rpfVal(portNum) %d: error code: %d\n",
                      __FUNCTION__, ipv6RouteMcData->rpfValue, retVal);
            return retVal;
        }
        ipv6RouteMcEntry.rpfValue = intfId;
    }
#endif

    // Populate v6RouteMc Entry
    if ((retVal = xpsMulticastAddIPv6RouteEntry(devId, &ipv6RouteMcEntry,
                                                &indexList)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
                  retVal);
        return retVal;
    }

    PRINT_LOG("\naddIPv6MulticastRouteEntry -> sip : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d grpAddr : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d pktcmd : %d, vrfId: %d, mvif: %d\n",
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
        PRINT_LOG("!!! Rehash occured!!! NOT TAKING CARE AS OF NOW.\n");
        return XP_NO_ERR;
    }

    if ((retVal = xpsMulticastGetIPv6RouteEntry(devId, *index,
                                                &getIpv6RouteMcEntry)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in reading an entry: error code: %d\n", __FUNCTION__,
                  retVal);
        return retVal;
    }

    PRINT_LOG("xpsMulticastGetIPv6RouteEntry-> sip : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d grpAddr : %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d pktcmd : %d, mvif: %d, l3DomainId: %d\n",
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
        if ((retVal = xpsMulticastCreateL2InterfaceListScope(scopeId,
                                                             ipv6RouteMcData->vlanNode[i].vlanId, &l2IfListId)) != XP_NO_ERR)
        {
            PRINT_LOG("%s: Error in Creating Interface List for vlan: %d: error code: %d\n",
                      __FUNCTION__, ipv6RouteMcData->vlanNode[i].vlanId, retVal);
            return retVal;
        }
        PRINT_LOG("%s: Created Interface List %d for vlan %d\n", __FUNCTION__,
                  l2IfListId, ipv6RouteMcData->vlanNode[i].vlanId);
        //Add L2 IntfList to Dev
        if ((retVal = xpsMulticastAddL2InterfaceListToDevice(devId,
                                                             l2IfListId)) != XP_NO_ERR)
        {
            PRINT_LOG("%s: Error in Adding Interface List %d to device %d: error code: %d\n",
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
                    xpsAppConfigureMvifEntry(devId, (MvifEntry *)getEntry(&configFile.mvifData,
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
                PRINT_LOG("%s: Error in Adding Bridge Interface List %d to L3 List %d: error code: %d\n",
                          __FUNCTION__, l2IfListId, l3IfListId, retVal);
                return retVal;
            }
            PRINT_LOG("%s: Added l2IfListId %d in l3IfListId %d\n", __FUNCTION__,
                      l2IfListId, l3IfListId);
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpsAppConfigureIpv4PimBiDirEntry(xpsDevice_t devId,
                                           Ipv4PimBiDirEntry *ipv4PimBiDirData, uint32_t *index)
{
    XP_STATUS retVal;
    xpsHashIndexList_t indexList;

    xpIPv4MulticastPimBidirRpfEntry_t ipv4PimBiDirEntry;// getIpv4PimBiDirEntry;

    if (!ipv4PimBiDirData || !index)
    {
        PRINT_LOG("Error: Null pointer received for ipv4PimBiDirData Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    memset(&ipv4PimBiDirEntry, 0, sizeof(xpIPv4MulticastPimBidirRpfEntry_t));
    ipv4PimBiDirEntry.bdId = ipv4PimBiDirData->bdId;
    ipv4PimBiDirEntry.vrfIdx = ipv4PimBiDirData->vrfId;
    ipv4PimBiDirEntry.rpfValue = ipv4PimBiDirData->rpfValue;
    COPY_IPV4_ADDR_T(ipv4PimBiDirEntry.groupAddress, ipv4PimBiDirData->grpAddr);

    if ((retVal = xpsMulticastAddIPv4PimBiDirRPFEntry(devId, &ipv4PimBiDirEntry,
                                                      &indexList)) != XP_NO_ERR)
    {
        PRINT_LOG("%s: Error in inserting an entry: error code: %d\n", __FUNCTION__,
                  retVal);
        return retVal;
    }

    PRINT_LOG("\naddIPv4PimBiDirEntry -> grpAddr : %d.%d.%d.%d, vrfId: %d, bdId: %d, rpfValue : %d\n",
              ipv4PimBiDirEntry.groupAddress[3], ipv4PimBiDirEntry.groupAddress[2],
              ipv4PimBiDirEntry.groupAddress[1], ipv4PimBiDirEntry.groupAddress[0],
              ipv4PimBiDirEntry.vrfIdx, ipv4PimBiDirEntry.bdId, ipv4PimBiDirEntry.rpfValue);

    *index = indexList.index[0];

    if (indexList.size > 1)
    {
        PRINT_LOG("!!! Rehash occured!!! NOT TAKING CARE AS OF NOW.\n");
        return XP_NO_ERR;
    }
    /*
        if ((retVal = xpGetIPv4MulticastPimBidirRpfEntry(devId, *index, &getIpv4PimBiDirEntry)) != XP_NO_ERR)
        {
            PRINT_LOG("%s: Error in reading an entry: error code: %d\n", __FUNCTION__, retVal);
            return retVal;
        }
        PRINT_LOG("getIPv4PimBiDirEntry -> grpAddr : %d.%d.%d.%d, vrfId: %d, bdId: %d, rpfVal : %d\n\n\n", getIpv4PimBiDirEntry.groupAddress[3], getIpv4PimBiDirEntry.groupAddress[2], getIpv4PimBiDirEntry.groupAddress[1], getIpv4PimBiDirEntry.groupAddress[0], getIpv4PimBiDirEntry.vrfIdx, getIpv4PimBiDirEntry.bdId, getIpv4PimBiDirEntry.rpfValue);
    */
    return XP_NO_ERR;
}

XP_STATUS xpsAppConfigureMacSAMSBsEntry(xpsDevice_t devId, uint8_t *macSAMSBs)
{
    /* Program the High32 bits of RouterMAC */
    macAddrHigh_t macHigh;
    XP_STATUS retVal = XP_NO_ERR;

    macHigh[0] = macSAMSBs[0];
    macHigh[1] = macSAMSBs[1];
    macHigh[2] = macSAMSBs[2];
    macHigh[3] = macSAMSBs[3];
    macHigh[4] = macSAMSBs[4];

    if ((retVal = xpsL3SetEgressRouterMacMSbs(devId, macHigh)) != XP_NO_ERR)
    {
        PRINT_LOG("Error in ipv4Host table Higher Mac Programming: error code: %d\n",
                  retVal);
        return retVal;
    }
    return retVal;
    return XP_NO_ERR;
}

#if 0
XP_STATUS xpsAppConfigureIacl(xpsDevice_t devId, xpDevConfigStruct* config)
{
    XP_STATUS retVal = XP_NO_ERR;
    uint8_t lkpEnable = 1;
    uint8_t kefIndex = 0;
    uint32_t intfId = 0;
    uint8_t tmpCtagData[2], tmpCtagMask[2], tmpStagData[2], tmpStagMask[2], tmpType,
            tmpTypeMask, tmpPktData, tmpPktMask;
    uint32_t tmpUintArray[18];
    uint32_t lkpIaclId = 1;
    uint32_t index = 0;
    uint32_t portNum =0;
    uint32_t numOfIaclDataEntries = 0;
    uint32_t i, j, vlan;
    uint32_t tnlIdx;
    xpsIaclData_t *iaclDataEntry;
    xpsIaclKeyType_e type = XP_IACL_V4_TYPE;
    xpIaclTableProfile_t tblProfile;
    xpIaclkeyFieldList_t fldData, ruleData;

    uint32_t keySize = 0;

    uint8_t numFileds = 0;
    bool kyFldLst[XP_IACL_COMMON_MAX_FLDS] = {0,};

    //PRINT_LOG("iACL : devId:%d \n",devId);
    IaclL2Ipv4KeyAttribute *iaclKey;
    IaclL2Ipv4MaskAttribute *iaclMask;
    IaclData *iaclData;

    memset(&tblProfile, 0x00, sizeof(xpIaclTableProfile_t));
    memset(&fldData, 0x00, sizeof(xpIaclkeyFieldList_t));
    memset(&ruleData, 0x00, sizeof(xpIaclkeyFieldList_t));

    numOfIaclDataEntries = numOfEntry(&configFile.iaclData);
    IaclL2Ipv4KeyAttribute *pbrKey;
    uint32_t numOfPbrDataEntries = 0;
    numOfPbrDataEntries = numOfEntry(&configFile.pbrData);


    /* Code flow for flex key implementation
     *  *  1. Identify unique entries which we will need for preparing a new key to write it in SE reg
     *  *  2. Compute the key size required for new Create table(may be we need to add the mapping of keyfield Vs length some where in our code)
     *  *  3. Call deleteTable/Create table with required profile(xpIaclTableProfile_t   -> keySize, numDb)
     *  *  4. Program the SE command with appropriate keyFieldData_t and xpIaclLookupType
     *  *         We need to call this API for Ipv4 IACL type only as we dont have any TC for Ipv6 type.
     *  *         We need to call this API for each IACL lookup type(PACL, VACL, RACL).
     *  *  5. Fetch the data fields info from config struct and call xpAclMgr::setIaclData()
     *  *  6. Call setIaclEntry() for each test case in appTest.
     *  *          Which will have the key and mask values for required key fields.
     *  *  */

    /*Identify unique entries for preparing a new key to write in SE reg */

    kyFldLst[XP_IACL_COMMON_KEY_TYPE] = 1;
    numFileds++;

    kyFldLst[XP_IACL_COMMON_ID] = 1;
    numFileds++;

    for (i = 0; i < numOfIaclDataEntries; i++)
    {
        //memset(iaclKey,0,sizeof(IaclL2Ipv4KeyAttribute));
        iaclKey = (IaclL2Ipv4KeyAttribute *)getEntry(&configFile.iaclKey, i);

        if (iaclKey->isDmacSet && kyFldLst[XP_IACL_COMMON_MAC_DA] == 0)
        {
            kyFldLst[XP_IACL_COMMON_MAC_DA] = 1;
            numFileds++;
        }

        if (iaclKey->isSmacSet && kyFldLst[XP_IACL_COMMON_MAC_SA] == 0)
        {
            kyFldLst[XP_IACL_COMMON_MAC_SA] = 1;
            numFileds++;
        }

        if (iaclKey->etherType != -1 &&
            kyFldLst[XP_IACL_COMMON_ETHER_TYPE] == 0) //TODO: issue with this field
        {
            kyFldLst[XP_IACL_COMMON_ETHER_TYPE] = 1;
            numFileds++;
        }

        if (((iaclKey->cTagVid != -1) | (iaclKey->cTagDei != -1) |
             (iaclKey->cTagPcp != -1)) && kyFldLst[XP_IACL_COMMON_CTAG_VID_DEI_PCP] == 0)
        {
            kyFldLst[XP_IACL_COMMON_CTAG_VID_DEI_PCP] = 1;
            numFileds++;
        }

        if (((iaclKey->sTagVid != -1) | (iaclKey->sTagDei != -1) |
             (iaclKey->sTagPcp != -1)) && kyFldLst[XP_IACL_COMMON_STAG_VID_DEI_PCP] == 0)
        {
            kyFldLst[XP_IACL_COMMON_STAG_VID_DEI_PCP] = 1;
            numFileds++;
        }

        if (iaclKey->isDIPSet && kyFldLst[XP_IACL_COMMON_DIP] == 0)
        {
            kyFldLst[XP_IACL_COMMON_DIP] = 1;
            numFileds++;
        }

        if (iaclKey->isSIPSet && kyFldLst[XP_IACL_COMMON_SIP] == 0)
        {
            kyFldLst[XP_IACL_COMMON_SIP] = 1;
            numFileds++;
        }

        if (iaclKey->L4DestPort != -1 &&
            kyFldLst[XP_IACL_COMMON_L4_DEST_PORT] == 0) //TODO: issue with this field
        {
            kyFldLst[XP_IACL_COMMON_L4_DEST_PORT] = 1;
            numFileds++;
        }

        if (iaclKey->L4SourcePort != -1 &&
            kyFldLst[XP_IACL_COMMON_L4_SRC_PORT] == 0) //TODO: issue with this field
        {
            kyFldLst[XP_IACL_COMMON_L4_SRC_PORT] = 1;
            numFileds++;
        }

        if (iaclKey->iVif != -1 && kyFldLst[XP_IACL_COMMON_IVIF] == 0)
        {
            kyFldLst[XP_IACL_COMMON_IVIF] = 1;
            numFileds++;
        }

        if (iaclKey->icmpMessageType != -1 &&
            kyFldLst[XP_IACL_COMMON_ICMP_MSG_TYPE] == 0)
        {
            kyFldLst[XP_IACL_COMMON_ICMP_MSG_TYPE] = 1;
            numFileds++;
        }

        if (iaclKey->protocol != -1 && kyFldLst[XP_IACL_COMMON_PROTOCOL] == 0)
        {
            kyFldLst[XP_IACL_COMMON_PROTOCOL] = 1;
            numFileds++;
        }

        if (((iaclKey->pktHasCtag != -1) | (iaclKey->pktHasStag != -1)) &&
            kyFldLst[XP_IACL_COMMON_TAG_FRAGMENT_INFO] == 0)
        {
            kyFldLst[XP_IACL_COMMON_TAG_FRAGMENT_INFO] = 1;
            numFileds++;
        }

        if (iaclKey->BD != -1 && kyFldLst[XP_IACL_COMMON_BD] == 0)
        {
            kyFldLst[XP_IACL_COMMON_BD] = 1;
            numFileds++;
        }

        if (iaclKey->pktLen && kyFldLst[XP_IACL_COMMON_PKTLEN] == 0)
        {
            kyFldLst[XP_IACL_COMMON_PKTLEN] = 1;
            numFileds++;
        }

    }

    for (i = 0; i < numOfPbrDataEntries; i++)
    {
        //memset(pbrKey,0,sizeof(IaclL2Ipv4KeyAttribute));
        pbrKey = (IaclL2Ipv4KeyAttribute *)getEntry(&configFile.pbrKey, i);

        if (pbrKey->isDmacSet && kyFldLst[XP_IACL_COMMON_MAC_DA] == 0)
        {
            kyFldLst[XP_IACL_COMMON_MAC_DA] = 1;
            numFileds++;
        }

        if (pbrKey->isSmacSet && kyFldLst[XP_IACL_COMMON_MAC_SA] == 0)
        {
            kyFldLst[XP_IACL_COMMON_MAC_SA] = 1;
            numFileds++;
        }

        if (pbrKey->etherType != -1 &&
            kyFldLst[XP_IACL_COMMON_ETHER_TYPE] == 0) //TODO: issue with this field
        {
            kyFldLst[XP_IACL_COMMON_ETHER_TYPE] = 1;
            numFileds++;
        }

        if (((pbrKey->cTagVid != -1) || (pbrKey->cTagDei != -1) ||
             (pbrKey->cTagPcp != -1)) && kyFldLst[XP_IACL_COMMON_CTAG_VID_DEI_PCP] == 0)
        {
            kyFldLst[XP_IACL_COMMON_CTAG_VID_DEI_PCP] = 1;
            numFileds++;
        }

        if (((pbrKey->sTagVid != -1) || (pbrKey->sTagDei != -1) ||
             (pbrKey->sTagPcp != -1)) && kyFldLst[XP_IACL_COMMON_STAG_VID_DEI_PCP] == 0)
        {
            kyFldLst[XP_IACL_COMMON_STAG_VID_DEI_PCP] = 1;
            numFileds++;
        }

        if (pbrKey->isDIPSet && kyFldLst[XP_IACL_COMMON_DIP] == 0)
        {
            kyFldLst[XP_IACL_COMMON_DIP] = 1;
            numFileds++;
        }

        if (pbrKey->isSIPSet && kyFldLst[XP_IACL_COMMON_SIP] == 0)
        {
            kyFldLst[XP_IACL_COMMON_SIP] = 1;
            numFileds++;
        }

        if (pbrKey->L4DestPort != -1 && kyFldLst[XP_IACL_COMMON_L4_DEST_PORT] == 0)
        {
            kyFldLst[XP_IACL_COMMON_L4_DEST_PORT] = 1;
            numFileds++;
        }

        if (pbrKey->L4SourcePort != -1 && kyFldLst[XP_IACL_COMMON_L4_SRC_PORT] == 0)
        {
            kyFldLst[XP_IACL_COMMON_L4_SRC_PORT] = 1;
            numFileds++;
        }

        if (pbrKey->vrfId != -1 && kyFldLst[XP_IACL_COMMON_VRF_ID] == 0)
        {
            kyFldLst[XP_IACL_COMMON_VRF_ID] = 1;
            numFileds++;
        }

        if (pbrKey->pktLen && kyFldLst[XP_IACL_COMMON_PKTLEN] == 0)
        {
            kyFldLst[XP_IACL_COMMON_PKTLEN] = 1;
            numFileds++;
        }

    }

    j=0;
    fldData.numFlds = numFileds;
    fldData.type = type;
    fldData.fldList = (xpIaclkeyField_t *)malloc(sizeof(xpIaclkeyField_t) *
                                                 numFileds);
    memset(fldData.fldList, 0, sizeof(xpIaclkeyField_t) * numFileds);
    fldData.isValid = 1;
    /* Initialize the field data and calculate key size from byte mask */
    for (i =1; i< XP_IACL_COMMON_MAX_FLDS; i++)
    {
        if (kyFldLst[i] == 1)
        {
            fldData.fldList[j].keyFlds = (xpIaclKeyFlds_t)i ;
            j++;
        }
    }

    keySize = WIDTH_384;

    tblProfile.numTables = 3;
    tblProfile.tableProfile[0].tblType = (xpIaclType_e)0;
    tblProfile.tableProfile[1].tblType = (xpIaclType_e)1;
    tblProfile.tableProfile[2].tblType = (xpIaclType_e)2;
    tblProfile.tableProfile[0].keySize = keySize;
    tblProfile.tableProfile[1].keySize = keySize;
    tblProfile.tableProfile[2].keySize = keySize;
    tblProfile.tableProfile[0].numDb = 1;
    tblProfile.tableProfile[1].numDb = 1;
    tblProfile.tableProfile[2].numDb = 1;

    if ((retVal = xpsIaclCreateTable(devId, tblProfile)) != XP_NO_ERR)
    {
        PRINT_LOG(" create Table returns %d :\n", retVal);
        PRINT_LOG("%s:, ERROR: Failed to createTable \n", __FUNCTION__);
        free(fldData.fldList);
        return retVal;
    }

    PRINT_LOG("iACL : Table Created SucessFully\n");

    if ((retVal = xpsIaclDefinePaclKey(devId, type, &fldData)) != XP_NO_ERR)
    {
        PRINT_LOG("%s:, ERROR %d: xpsIaclDefinePaclKey() failed  \n", __FUNCTION__,
                  retVal);
        free(fldData.fldList);
        return retVal;
    }
    PRINT_LOG("iACL : xpsIaclDefinePaclKey() done ! \n");


    if ((retVal = xpsIaclDefineBaclKey(devId, type, &fldData)) != XP_NO_ERR)
    {
        PRINT_LOG("%s:, ERROR %d: xpsIaclDefineBaclKey() failed  \n", __FUNCTION__,
                  retVal);
        free(fldData.fldList);
        return retVal;
    }
    PRINT_LOG("iACL : xpsIaclDefineBaclKey() done ! \n");

    if ((retVal = xpsIaclDefineRaclKey(devId, type, &fldData)) != XP_NO_ERR)
    {
        PRINT_LOG("%s:, ERROR %d: xpsIaclDefineRaclKey() failed  \n", __FUNCTION__,
                  retVal);
        free(fldData.fldList);
        return retVal;
    }
    PRINT_LOG("iACL : xpsIaclDefineRaclKey() done ! \n");

    /*Old implementation starts from here */
    for (i = 0; i < numOfIaclDataEntries; i++)
    {
        iaclDataEntry = (xpIaclData_t *)malloc(sizeof(xpIaclData_t));
        memset(iaclDataEntry, 0, sizeof(xpIaclData_t));

        /* Copy field data from reference key details struct and fill up values */
        memcpy(&ruleData, &fldData, sizeof(xpIaclkeyFieldList_t));

        iaclData = (IaclData *)getEntry(&configFile.iaclData, i);
        iaclKey = (IaclL2Ipv4KeyAttribute *)getEntry(&configFile.iaclKey, i);
        iaclMask = (IaclL2Ipv4MaskAttribute *)getEntry(&configFile.iaclMask, i);
        portNum = iaclData->portNum;
        vlan = iaclData->vlan;
        lkpIaclId = iaclKey->iAclId;
        index = i;

        xpPortConfig_t portConfigEntryT;
        memset(&portConfigEntryT, 0x0, sizeof(xpPortConfig_t));



        //key.isValid = 1;
        //mask.isValid = 1;
        PRINT_LOG("----------------Port Num : %d | iAclId: %d ----------------------\n",
                  portNum, iaclKey->iAclId);
        switch (iaclData->type)
        {
            case 0: //ACL for Port
                {
                    PRINT_LOG("iaclOnPort\n");
                    //
                    // Enable IACL lookup and set IACL ID on port
                    //
                    retVal = xpsPortGetPortIntfId(devId, portNum, &intfId);
                    if (retVal)
                    {
                        PRINT_LOG("Error : xpsPortGetPortIntfId() Failed for interfac type :%d devId %d\n",
                                  XPS_PORT, devId);
                        free(iaclDataEntry);
                        free(fldData.fldList);
                        return retVal;
                    }
                    if ((retVal = xpsPortSetField(devId, intfId, XPS_PORT_ACL_EN,
                                                  lkpEnable)) != XP_NO_ERR)
                    {
                        PRINT_LOG("%s:, ERROR: failed to eanble iacl on port intf id :%d ,error code: %d\n",
                                  __FUNCTION__, intfId, retVal);
                        free(iaclDataEntry);
                        free(fldData.fldList);
                        return retVal;
                    }
                    if ((retVal = xpsPortSetField(devId, intfId, XPS_PORT_ACL_ID,
                                                  lkpIaclId)) != XP_NO_ERR)
                    {
                        PRINT_LOG("%s:, ERROR: failed to set iacl id on port intf id :%d ,lookup id : %d,error code: %d\n",
                                  __FUNCTION__, intfId, lkpIaclId, retVal);
                        free(iaclDataEntry);
                        free(fldData.fldList);
                        return retVal;
                    }
                    break;
                }

            case 1: // ACL for Port-Vlan
                {
                    PRINT_LOG("iaclOnPort-Vlan\n");
                    //
                    // Enable IACL lookup and set IACL ID on port-vlan
                    //
                    if ((retVal = xpsVlanSetBridgeAclEnable(devId, vlan, lkpEnable)) != XP_NO_ERR)
                    {
                        PRINT_LOG("%s:, ERROR: failed to eanble iacl on vlan : %d,error code: %d\n",
                                  __FUNCTION__, vlan, retVal);
                        free(iaclDataEntry);
                        free(fldData.fldList);
                        return retVal;
                    }

                    if ((retVal = xpsVlanSetBridgeAclId(devId, vlan, lkpIaclId)) != XP_NO_ERR)
                    {
                        PRINT_LOG("%s:, ERROR: Failed to set BACL id : %d for vlan %d\n", __FUNCTION__,
                                  lkpIaclId, vlan);
                        free(iaclDataEntry);
                        free(fldData.fldList);
                        return retVal;
                    }
                    break;
                }

            case 2: // ACL for Route
                {
                    PRINT_LOG("iaclOnRoute\n");

                    //
                    // Enable IACL lookup and set IACL ID on route
                    //
                    uint32_t l3IntfId = XPS_INTF_MAP_BD_TO_INTF(vlan);
                    if ((retVal = xpsL3SetRouterAclEnable(devId, l3IntfId, lkpEnable)) != XP_NO_ERR)
                    {
                        PRINT_LOG("%s:, ERROR: Failed to enable RACL on vlan : %d\n", __FUNCTION__,
                                  vlan);
                        free(iaclDataEntry);
                        free(fldData.fldList);
                        return retVal;
                    }

                    if ((retVal = xpsL3SetRouterAclId(devId, l3IntfId, lkpIaclId)) != XP_NO_ERR)
                    {
                        PRINT_LOG("%s:, ERROR:Failed to set RACL id for vlan : %d\n", __FUNCTION__,
                                  vlan);
                        free(iaclDataEntry);
                        free(fldData.fldList);
                        return retVal;
                    }
                    break;
                }
            default:
                {
                    PRINT_LOG(", ERROR: Invalid : ACL Type\n");
                    break;
                }
        }

        tmpType = 0x00;
        tmpTypeMask = 0x00;
        GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_KEY_TYPE, kyFldLst, kefIndex);
        ruleData.fldList[kefIndex].value = &tmpType;
        ruleData.fldList[kefIndex].mask = &tmpTypeMask;

        if (kyFldLst[XP_IACL_COMMON_ID])
        {
            //ruleData.fldList[XP_IACL_COMMON_ID].fld.v4Fld = XP_IACL_COMMON_ID;
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_ID, kyFldLst, kefIndex);

            tmpUintArray[0] = htole32(iaclMask->iAclId);
            tmpUintArray[1] = htole32(iaclKey->iAclId);
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[0];
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[1];
        }

        if (kyFldLst[XP_IACL_COMMON_MAC_SA])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_MAC_SA, kyFldLst, kefIndex);
            ruleData.fldList[kefIndex].value = iaclKey->macSA;
            ruleData.fldList[kefIndex].mask = iaclMask->macSA;
        }
        if (kyFldLst[XP_IACL_COMMON_MAC_DA])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_MAC_DA, kyFldLst, kefIndex);
            ruleData.fldList[kefIndex].value = iaclKey->macDA;
            ruleData.fldList[kefIndex].mask = iaclMask->macDA;
        }

        if (kyFldLst[XP_IACL_COMMON_ETHER_TYPE])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_ETHER_TYPE, kyFldLst, kefIndex);

            tmpUintArray[2] = htole32(iaclKey->etherType);
            tmpUintArray[3] = htole32(iaclMask->etherType);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[2];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[3];
        }

        if (kyFldLst[XP_IACL_COMMON_CTAG_VID_DEI_PCP])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_CTAG_VID_DEI_PCP, kyFldLst,
                                           kefIndex);

            tmpCtagData[0] = iaclKey->cTagVid & 0xFF;
            tmpCtagData[1] = ((iaclKey->cTagPcp & 0x07) << 5) | ((iaclKey->cTagDei & 0x01)
                                                                 << 4) | ((iaclKey->cTagVid >> 8) & 0x0F);

            ruleData.fldList[kefIndex].value = tmpCtagData;
            tmpCtagMask[0] = iaclMask->cTagVid & 0xFF;
            tmpCtagMask[1] = ((iaclMask->cTagPcp & 0x07) << 5) | ((iaclMask->cTagDei & 0x01)
                                                                  << 4) | ((iaclMask->cTagVid >> 8) & 0x0F);
            ruleData.fldList[kefIndex].mask = tmpCtagMask;
        }

        if (kyFldLst[XP_IACL_COMMON_STAG_VID_DEI_PCP])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_STAG_VID_DEI_PCP, kyFldLst,
                                           kefIndex);


            tmpStagData[0] = iaclKey->sTagVid & 0xFF;
            tmpStagData[1] = ((iaclKey->sTagPcp & 0x07) << 5) | ((iaclKey->sTagDei & 0x01)
                                                                 << 4) | ((iaclKey->sTagVid >> 8) & 0x0F);
            ruleData.fldList[kefIndex].value = tmpStagData;
            tmpStagMask[0] = iaclMask->sTagVid & 0xFF;
            tmpStagMask[1] = ((iaclMask->sTagPcp & 0x07) << 5) | ((iaclMask->sTagDei & 0x01)
                                                                  << 4) | ((iaclMask->sTagVid >> 8) & 0x0F);
            ruleData.fldList[kefIndex].mask = tmpStagMask;
        }

        if (kyFldLst[XP_IACL_COMMON_DIP])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_DIP, kyFldLst, kefIndex);
            ruleData.fldList[kefIndex].value = iaclKey->DIP;
            ruleData.fldList[kefIndex].mask = iaclMask->DIP;
        }

        j=3;
        if (kyFldLst[XP_IACL_COMMON_SIP])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_SIP, kyFldLst, kefIndex);
            ruleData.fldList[kefIndex].value = iaclKey->SIP;
            ruleData.fldList[kefIndex].mask = iaclMask->SIP;
        }

        if (kyFldLst[XP_IACL_COMMON_PKTLEN])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_PKTLEN, kyFldLst, kefIndex);
            tmpUintArray[16] = htole32(iaclKey->pktLen);
            tmpUintArray[17] = htole32(iaclMask->pktLen);

            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[16];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[17];
        }

        if (kyFldLst[XP_IACL_COMMON_L4_DEST_PORT])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_L4_DEST_PORT, kyFldLst, kefIndex);

            tmpUintArray[4] = htole32(iaclKey->L4DestPort);
            tmpUintArray[5] = htole32(iaclMask->L4DestPort);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[4];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[5];
        }

        if (kyFldLst[XP_IACL_COMMON_L4_SRC_PORT])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_L4_SRC_PORT, kyFldLst, kefIndex);

            tmpUintArray[6] = htole32(iaclKey->L4SourcePort);
            tmpUintArray[7] = htole32(iaclMask->L4SourcePort);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[6];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[7];
        }

        if (kyFldLst[XP_IACL_COMMON_ICMP_MSG_TYPE])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_ICMP_MSG_TYPE, kyFldLst,
                                           kefIndex);

            tmpUintArray[8] = htole32(iaclKey->icmpMessageType);
            tmpUintArray[9] = htole32(iaclMask->icmpMessageType);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[8];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[9];
        }
        if (kyFldLst[XP_IACL_COMMON_PROTOCOL])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_PROTOCOL, kyFldLst, kefIndex);

            tmpUintArray[10] = htole32(iaclKey->protocol);
            tmpUintArray[11] = htole32(iaclMask->protocol);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[10];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[11];
        }

        if (kyFldLst[XP_IACL_COMMON_BD])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_BD, kyFldLst, kefIndex);

            tmpUintArray[12] = htole32(iaclKey->BD);
            tmpUintArray[13] = htole32(iaclMask->BD);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[12];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[13];
        }
        if (kyFldLst[XP_IACL_COMMON_IVIF])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_IVIF, kyFldLst, kefIndex);

            tmpUintArray[14] = htole32(iaclKey->iVif);
            tmpUintArray[15] = htole32(iaclMask->iVif);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[14];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[15];
        }
        if (kyFldLst[XP_IACL_COMMON_TAG_FRAGMENT_INFO])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_TAG_FRAGMENT_INFO, kyFldLst,
                                           kefIndex);
            tmpPktData = ((iaclKey->pktHasCtag & 0x01) << 1) | ((iaclKey->pktHasStag &
                                                                 0x01));
            ruleData.fldList[kefIndex].value = &tmpPktData;
            tmpPktMask = ((iaclMask->pktHasCtag & 0x01) << 1) | ((iaclMask->pktHasStag &
                                                                  0x01)) | 0xFC;
            ruleData.fldList[kefIndex].mask = &tmpPktMask;
        }

        //
        //Create IACL data
        //

        iaclDataEntry->iAclDataType.aclData.enPktCmdUpd = iaclData->enPktCmdUpd;
        iaclDataEntry->iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command =
            iaclData->pktCmdOrEcmpSize;
        iaclDataEntry->iAclDataType.aclData.qosOrVlanRw = iaclData->qosOrVlanRw;
        iaclDataEntry->iAclDataType.aclData.enRedirectToEvif =
            iaclData->enRedirectToEvif;
        iaclDataEntry->iAclDataType.aclData.encapType = iaclData->encapType;
        iaclDataEntry->iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode =
            iaclData->rsnCode;
        PRINT_LOG("iaclDataEntry->rsnCode : %d \n",
                  iaclDataEntry->iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode);

        iaclDataEntry->iAclDataType.aclData.enPktCmdUpd = iaclData->enPktCmdUpd;
        iaclDataEntry->iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command =
            iaclData->pktCmdOrEcmpSize;
        iaclDataEntry->iAclDataType.aclData.qosOrVlanRw = iaclData->qosOrVlanRw;
        if (iaclDataEntry->iAclDataType.aclData.qosOrVlanRw == 1)
        {
            iaclDataEntry->iAclDataType.aclData.qosOrVlanRwData.vlan = iaclData->vlan;
        }
        else
        {
            iaclDataEntry->iAclDataType.aclData.qosOrVlanRwData.qosData.dscp =
                iaclData->DSCP;
            iaclDataEntry->iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = iaclData->PCP;
        }
        iaclDataEntry->iAclDataType.aclData.enRedirectToEvif =
            iaclData->enRedirectToEvif;
        iaclDataEntry->iAclDataType.aclData.encapType = iaclData->encapType;

        tnlIdx = iaclData->eVifId;
        if (iaclData->enRedirectToEvif == 1)
        {
            // taking pktCmd value as ecmp size since control bit enRedirectToEvif is set
            if (iaclData->encapType == XP_IACL_ENCAP_TAGGED ||
                iaclData->encapType == XP_IACL_ENCAP_UNTAGGED) // Non Tunnel Port
            {
                //evif change for N devices
                retVal = xpsPortGetPortIntfId(devId, iaclData->eVifId, &tnlIdx);
                if (retVal)
                {
                    PRINT_LOG("Error : xpsPortGetPortIntfId() Failed for devId %d portId : %d\n",
                              devId, iaclData->eVifId);
                    free(iaclDataEntry);
                    free(fldData.fldList);
                    return retVal;
                }
                iaclDataEntry->iAclDataType.aclData.egrVifIdOrRsnCode.eVifId = tnlIdx;
                PRINT_LOG("\niaclDataEntry->egrVifIdOrRsnCode.eVifId : %d\n",
                          iaclDataEntry->iAclDataType.aclData.egrVifIdOrRsnCode.eVifId);
            }
            else if (iaclData->encapType == XP_IACL_ENCAP_IP_TUNNEL) //IP Tunnel Port
            {
                iaclDataEntry->iAclDataType.aclData.egrVifIdOrRsnCode.eVifId =
                    tnlVif[devId][tnlIdx];
            }
            else
            {
                iaclDataEntry->iAclDataType.aclData.egrVifIdOrRsnCode.eVifId = iaclData->eVifId;
            }
        }
        else
        {
            iaclDataEntry->iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode =
                iaclData->rsnCode;
        }

        iaclDataEntry->iAclDataType.aclData.isTerminal = iaclData->isTerminal;
        iaclDataEntry->iAclDataType.aclData.enDscpRemarkOrVrfUpd = iaclData->remarkDscp;
        iaclDataEntry->iAclDataType.aclData.enPcpRemarkOrStagRw = iaclData->remarkPcp;
        iaclDataEntry->iAclDataType.aclData.enDpRemark = iaclData->remarkDp;
        iaclDataEntry->iAclDataType.aclData.dp = iaclData->DP;
        iaclDataEntry->iAclDataType.aclData.enTcRemark = iaclData->remarkTc;
        iaclDataEntry->iAclDataType.aclData.tc = iaclData->TC;
        iaclDataEntry->iAclDataType.aclData.enMirrorSsnUpd = iaclData->enMirrorSsnUpd;
        iaclDataEntry->iAclDataType.aclData.mirrorSessionId = iaclData->mirrorSessionId;

        PRINT_LOG("iaclDataEntry->enPktCmdUpd : %d \n",
                  iaclDataEntry->iAclDataType.aclData.enPktCmdUpd);
        PRINT_LOG("iaclDataEntry->pktCmd : %d \n",
                  iaclDataEntry->iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command);
        PRINT_LOG("iaclDataEntry->isTerminal : %d \n",
                  iaclDataEntry->iAclDataType.aclData.isTerminal);
        PRINT_LOG("iaclDataEntry->qosOrValnRw : %d \n",
                  iaclDataEntry->iAclDataType.aclData.qosOrVlanRw);
        if (iaclDataEntry->iAclDataType.aclData.qosOrVlanRw == 1)
        {
            PRINT_LOG("iaclDataEntry->qosOrVlanRwData.vlan : %d \n",
                      iaclDataEntry->iAclDataType.aclData.qosOrVlanRwData.vlan);
        }
        else
        {
            PRINT_LOG("iaclDataEntry->qosOrVlanRwData.qosData.dscp : %d \n",
                      iaclDataEntry->iAclDataType.aclData.qosOrVlanRwData.qosData.dscp);
            PRINT_LOG("iaclDataEntry->qosOrVlanRwData.qosData.pcp : %d \n",
                      iaclDataEntry->iAclDataType.aclData.qosOrVlanRwData.qosData.pcp);
        }
        PRINT_LOG("iaclDataEntry->enRedirectToEvif : %d \n",
                  iaclDataEntry->iAclDataType.aclData.enRedirectToEvif);
        if (iaclDataEntry->iAclDataType.aclData.enRedirectToEvif == 1)
        {
            PRINT_LOG("iaclDataEntry->egrVifIdOrRsnCode.eVifId : %d \n",
                      iaclDataEntry->iAclDataType.aclData.egrVifIdOrRsnCode.eVifId);
        }
        else
        {
            PRINT_LOG("iaclDataEntry->egrVifIdOrRsnCode.rsnCode : %d \n",
                      iaclDataEntry->iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode);
        }
        PRINT_LOG("iaclDataEntry->enDscpRemarkOrVrfUpd : %d \n",
                  iaclDataEntry->iAclDataType.aclData.enDscpRemarkOrVrfUpd);
        PRINT_LOG("iaclDataEntry->enPcpRemarkOrStagRw : %d \n",
                  iaclDataEntry->iAclDataType.aclData.enPcpRemarkOrStagRw);
        PRINT_LOG("iaclDataEntry->enDpRemark : %d \n",
                  iaclDataEntry->iAclDataType.aclData.enDpRemark);
        PRINT_LOG("iaclDataEntry->DP : %d \n", iaclDataEntry->iAclDataType.aclData.dp);
        PRINT_LOG("iaclDataEntry->enTcRemark : %d \n",
                  iaclDataEntry->iAclDataType.aclData.enTcRemark);
        PRINT_LOG("iaclDataEntry->tc : %d \n", iaclDataEntry->iAclDataType.aclData.tc);
        PRINT_LOG("iaclDataEntry->enMirrorSsnUpd : %d \n",
                  iaclDataEntry->iAclDataType.aclData.enMirrorSsnUpd);
        PRINT_LOG("iaclDataEntry->mirrorSessionId : %d \n",
                  iaclDataEntry->iAclDataType.aclData.mirrorSessionId);
        PRINT_LOG("iaclDataEntry->encapType : %d \n",
                  iaclDataEntry->iAclDataType.aclData.encapType);

        //
        //Set IACL entry
        //
        switch (iaclData->type)
        {
            case 0://ACL for Port
                {
                    if ((retVal = xpsIaclWritePaclKey(devId, index, &ruleData)) != XP_NO_ERR)
                    {
                        PRINT_LOG("%s:, ERROR:Failed to set iacl rule for PACL index %d\n",
                                  __FUNCTION__, index);
                        free(iaclDataEntry);
                        free(fldData.fldList);
                        return retVal;
                    }

                    if ((retVal = xpsIaclWritePaclData(devId, index, iaclDataEntry)) != XP_NO_ERR)
                    {
                        PRINT_LOG("%s:, ERROR:Failed to set iacl data for PACL index %d\n",
                                  __FUNCTION__, index);
                        free(iaclDataEntry);
                        free(fldData.fldList);
                        return retVal;
                    }

                    break;
                }
            case 1://ACL for bridge
                {
                    if ((retVal = xpsIaclWriteBaclKey(devId, index, &ruleData)) != XP_NO_ERR)
                    {
                        PRINT_LOG("%s:, ERROR:Failed to set iacl rule for BACL index %d\n",
                                  __FUNCTION__, index);
                        free(iaclDataEntry);
                        free(fldData.fldList);
                        return retVal;
                    }

                    if ((retVal = xpsIaclWriteBaclData(devId, index, iaclDataEntry)) != XP_NO_ERR)
                    {
                        PRINT_LOG("%s:, ERROR:Failed to set iacl data for BACL index %d\n",
                                  __FUNCTION__, index);
                        free(iaclDataEntry);
                        free(fldData.fldList);
                        return retVal;
                    }

                    break;
                }
            case 2://ACL for route
                {
                    if ((retVal = xpsIaclWriteRaclKey(devId, index, &ruleData)) != XP_NO_ERR)
                    {
                        PRINT_LOG("%s:, ERROR:Failed to set iacl rule for RACL index %d\n",
                                  __FUNCTION__, index);
                        free(iaclDataEntry);
                        free(fldData.fldList);
                        return retVal;
                    }

                    if ((retVal = xpsIaclWriteRaclData(devId, index, iaclDataEntry)) != XP_NO_ERR)
                    {
                        PRINT_LOG("%s:, ERROR:Failed to set iacl data for RACL index %d\n",
                                  __FUNCTION__, index);
                        free(iaclDataEntry);
                        free(fldData.fldList);
                        return retVal;
                    }

                    break;
                }
            default:
                {
                    PRINT_LOG(", ERROR: Invalid : ACL Type\n");
                    break;
                }
        }
        free(iaclDataEntry);
    }

    retVal = xpsAppConfigurePbr(devId, &fldData, kyFldLst, index);
    if (retVal != XP_NO_ERR)
    {
        PRINT_LOG("xpsAppConfigurePbr failed \n");
    }
    free(fldData.fldList);
    return retVal;

}

XP_STATUS xpsAppConfigurePbr(xpDevice_t devId, xpIaclkeyFieldList_t *fldData,
                             bool *kyFldLst, uint32_t pbrBaseIndex)
{
    XP_STATUS retVal = XP_NO_ERR;
    uint8_t kefIndex = 0;
    uint8_t tmpCtagData[2], tmpCtagMask[2], tmpStagData[2], tmpStagMask[2], tmpType,
            tmpTypeMask;
    uint32_t tmpUintArray[18];
    uint32_t  numOfPbrDataEntries = 0;
    uint32_t i, index;
    uint8_t lkpEnable = 1;
    uint32_t lkpIaclId = 1;
    uint32_t vlan = 0;
    //xpIaclKeyMask key;
    xpIaclData_t tmpPbrDataEntry, *pbrDataEntry;
    xpIaclkeyFieldList_t ruleData;

    //PRINT_LOG("iACL : devNum:%d \n",devNum);
    IaclL2Ipv4KeyAttribute *pbrKey;
    IaclL2Ipv4MaskAttribute *pbrMask;
    PbrData *pbrData;
    memset(&ruleData, 0x00, sizeof(xpIaclkeyFieldList_t));

    numOfPbrDataEntries = numOfEntry(&configFile.pbrData);

    if (numOfPbrDataEntries == 0)
    {
        return XP_NO_ERR;
    }

    /*Old implementation starts from here */
    for (i = 0; i < numOfPbrDataEntries; i++)
    {
        pbrDataEntry = &tmpPbrDataEntry;
        memset(pbrDataEntry, 0, sizeof(xpIaclData_t));

        /* Copy field data from reference key details struct and fill up values */
        memcpy(&ruleData, fldData, sizeof(xpIaclkeyFieldList_t));

        pbrData = (PbrData *)getEntry(&configFile.pbrData, i);
        pbrKey = (IaclL2Ipv4KeyAttribute *)getEntry(&configFile.pbrKey, i);
        pbrMask = (IaclL2Ipv4MaskAttribute *)getEntry(&configFile.pbrMask, i);
        index = i + pbrBaseIndex;
        lkpIaclId = index + 1;
        lkpEnable = 1;
        vlan = pbrData->vlan;

        PRINT_LOG("iaclOnRoute\n");
        //
        // Enable IACL lookup and set IACL ID on route
        //
        uint32_t l3IntfId = XPS_INTF_MAP_BD_TO_INTF(vlan);
        if ((retVal = xpsL3SetRouterAclEnable(devId, l3IntfId, lkpEnable)) != XP_NO_ERR)
        {
            PRINT_LOG("%s:, ERROR: Failed to enable RACL on vlan : %d\n", __FUNCTION__,
                      vlan);
            return retVal;
        }

        if ((retVal = xpsL3SetRouterAclId(devId, l3IntfId, lkpIaclId)) != XP_NO_ERR)
        {
            PRINT_LOG("%s:, ERROR:Failed to set RACL id for vlan : %d\n", __FUNCTION__,
                      vlan);
            return retVal;
        }

        tmpType = 0x00;
        tmpTypeMask = 0x00;
        GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_KEY_TYPE, kyFldLst, kefIndex);
        ruleData.fldList[kefIndex].value = &tmpType;
        ruleData.fldList[kefIndex].mask = &tmpTypeMask;

        if (kyFldLst[XP_IACL_COMMON_ID])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_ID, kyFldLst, kefIndex);
            tmpUintArray[14] = htole32(pbrKey->iAclId);
            tmpUintArray[15] = htole32(pbrMask->iAclId);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[14];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[15];
        }

        if (kyFldLst[XP_IACL_COMMON_MAC_SA])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_MAC_SA, kyFldLst, kefIndex);
            ruleData.fldList[kefIndex].value = pbrKey->macSA;
            ruleData.fldList[kefIndex].mask = pbrMask->macSA;
        }
        if (kyFldLst[XP_IACL_COMMON_MAC_DA])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_MAC_DA, kyFldLst, kefIndex);
            ruleData.fldList[kefIndex].value = pbrKey->macDA;
            ruleData.fldList[kefIndex].mask = pbrMask->macDA;
        }

        if (kyFldLst[XP_IACL_COMMON_ETHER_TYPE])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_ETHER_TYPE, kyFldLst, kefIndex);
            tmpUintArray[2] = htole32(pbrKey->etherType);
            tmpUintArray[3] = htole32(pbrMask->etherType);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[2];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[3];
        }

        if (kyFldLst[XP_IACL_COMMON_BD])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_BD, kyFldLst, kefIndex);

            tmpUintArray[12] = htole32(pbrKey->BD);
            tmpUintArray[13] = htole32(pbrMask->BD);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[12];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[13];
        }

        if (kyFldLst[XP_IACL_COMMON_CTAG_VID_DEI_PCP])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_CTAG_VID_DEI_PCP, kyFldLst,
                                           kefIndex);

            tmpCtagData[0] = pbrKey->cTagVid & 0xFF;
            tmpCtagData[1] = ((pbrKey->cTagPcp & 0x07) << 5) | ((pbrKey->cTagDei & 0x01) <<
                                                                4) | ((pbrKey->cTagVid >> 8) & 0x0F);

            ruleData.fldList[kefIndex].value = tmpCtagData;
            tmpCtagMask[0] = pbrMask->cTagVid & 0xFF;
            tmpCtagMask[1] = ((pbrMask->cTagPcp & 0x07) << 5) | ((pbrMask->cTagDei & 0x01)
                                                                 << 4) | ((pbrMask->cTagVid >> 8) & 0x0F);
            ruleData.fldList[kefIndex].mask = tmpCtagMask;
        }

        if (kyFldLst[XP_IACL_COMMON_STAG_VID_DEI_PCP])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_STAG_VID_DEI_PCP, kyFldLst,
                                           kefIndex);


            tmpStagData[0] = pbrKey->sTagVid & 0xFF;
            tmpStagData[1] = ((pbrKey->sTagPcp & 0x07) << 5) | ((pbrKey->sTagDei & 0x01) <<
                                                                4) | ((pbrKey->sTagVid >> 8) & 0x0F);
            ruleData.fldList[kefIndex].value = tmpStagData;
            tmpStagMask[0] = pbrMask->sTagVid & 0xFF;
            tmpStagMask[1] = ((pbrMask->sTagPcp & 0x07) << 5) | ((pbrMask->sTagDei & 0x01)
                                                                 << 4) | ((pbrMask->sTagVid >> 8) & 0x0F);
            ruleData.fldList[kefIndex].mask = tmpStagMask;
        }


        if (kyFldLst[XP_IACL_COMMON_PKTLEN])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_PKTLEN, kyFldLst, kefIndex);
            tmpUintArray[16] = htole32(pbrKey->pktLen);
            tmpUintArray[17] = htole32(pbrMask->pktLen);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[16];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[17];
        }


        if (kyFldLst[XP_IACL_COMMON_L4_DEST_PORT])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_L4_DEST_PORT, kyFldLst, kefIndex);
            tmpUintArray[4] = htole32(pbrKey->L4DestPort);
            tmpUintArray[5] = htole32(pbrMask->L4DestPort);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[4];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[5];
        }

        if (kyFldLst[XP_IACL_COMMON_L4_SRC_PORT])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_L4_SRC_PORT, kyFldLst, kefIndex);
            tmpUintArray[6] = htole32(pbrKey->L4SourcePort);
            tmpUintArray[7] = htole32(pbrMask->L4SourcePort);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[6];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[7];
        }

        if (kyFldLst[XP_IACL_COMMON_ICMP_MSG_TYPE])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_ICMP_MSG_TYPE, kyFldLst,
                                           kefIndex);
            tmpUintArray[8] = htole32(pbrKey->icmpMessageType);
            tmpUintArray[9] = htole32(pbrMask->icmpMessageType);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[8];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[9];
        }
        if (kyFldLst[XP_IACL_COMMON_PROTOCOL])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_PROTOCOL, kyFldLst, kefIndex);
            tmpUintArray[10] = htole32(pbrKey->protocol);
            tmpUintArray[11] = htole32(pbrMask->protocol);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[10];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[11];
        }
        if (kyFldLst[XP_IACL_COMMON_VRF_ID])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_VRF_ID, kyFldLst, kefIndex);
            tmpUintArray[0] = htole32(pbrKey->vrfId);
            tmpUintArray[1] = htole32(pbrMask->vrfId);
            ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[0];
            ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[1];
        }
        if (kyFldLst[XP_IACL_COMMON_DIP])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_DIP, kyFldLst, kefIndex);
            ruleData.fldList[kefIndex].value = pbrKey->DIP;
            ruleData.fldList[kefIndex].mask = pbrMask->DIP;
        }

        if (kyFldLst[XP_IACL_COMMON_SIP])
        {
            GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_IACL_COMMON_SIP, kyFldLst, kefIndex);
            ruleData.fldList[kefIndex].value = pbrKey->SIP;
            ruleData.fldList[kefIndex].mask = pbrMask->SIP;
        }

        //
        //Set IACL rules
        //
        if ((retVal = xpsIaclWriteRaclKey(devId, index, &ruleData)) != XP_NO_ERR)
        {
            PRINT_LOG("%s:, ERROR:Failed to set iacl rule for PBR index %d\n", __FUNCTION__,
                      index);
            return retVal;
        }

        //
        //Create IACL data
        //

        pbrDataEntry->entryType = 1; //PBR entry
        pbrDataEntry->iAclDataType.pbrData.ecmpEn = pbrData->ecmpEn;
        pbrDataEntry->iAclDataType.pbrData.nhId = pbrData->nhId;
        PRINT_LOG("pbrDataEntry->>iAclDataType.pbrData.ecmpEn : %d \n",
                  pbrDataEntry->iAclDataType.pbrData.ecmpEn);
        PRINT_LOG("pbrDataEntry->>iAclDataType.pbrData.nhId : %d \n",
                  pbrDataEntry->iAclDataType.pbrData.nhId);

        if ((retVal = xpsIaclWriteRaclData(devId, index, pbrDataEntry)) != XP_NO_ERR)
        {
            PRINT_LOG("%s:, ERROR:Failed to set iacl data for PBR index %d\n", __FUNCTION__,
                      index);
            return retVal;
        }
    }

    return retVal;
}

XP_STATUS xpsAppConfigureEacl(xpDevice_t devNum)
{
    /*Code flow  :
      /1. Identify unique key/mask fields required for both the types
      /2. Calculate the max key size needed from above info
      /3. Create table with new profile
      /4. Call xpEgressAclSetKeyFormat
      /5. prepare key/mask and call setEaclRule
      /6. prepare data and call setEaclData
    */
    XP_STATUS retVal = XP_NO_ERR;
    uint32_t numOfEaclDataEntries = 0;
    uint32_t i= 0;
    uint32_t keySize = 0;
    uint32_t tmpV4Index = 0;
    uint32_t tmpV6Index = 0;
    uint32_t v4KeySize = 0;
    uint32_t v6KeySize= 0;
    uint8_t rem = 0;
    uint8_t bc = 0;
    uint8_t kefIndex = 0;
    uint8_t tmpDscpData[2], tmpTypeMask, tmpTcpMask[3], tmpTcpData[3], tmpPcpMask,
            tmpPcpData;
    uint8_t tmpDscpMask[2];
    xpsInterfaceId_t egressVif = 0;
    xpsEaclData_t *eaclDataEntry;
    xpEaclTableProfile_t tblProfile;
    xpEaclkeyFieldList_t v4fldData, v4ruleData;
    xpEaclkeyFieldList_t v6fldData, v6ruleData;

    uint32_t v4ByteMask[XP_NUM_V4_FLDS] = {0,};
    uint32_t v6ByteMask[XP_NUM_V6_FLDS] = {0,};
    uint32_t tmpUintArray[40];

    uint8_t numV4Fileds = 0, numV6Fileds = 0;
    bool v4KyFldLst[XP_EACL_IPV4_MAX_FLDS] = {0,};
    bool v6KyFldLst[XP_EACL_IPV6_MAX_FLDS] = {0,};
    //PRINT_LOG("eACL : devNum:%d \n",devNum);
    EaclL2KeyAttribute *eaclKey;
    EaclL2MaskAttribute *eaclMask;
    EaclData *eaclData;
    memset(&v4fldData, 0x00, sizeof(xpEaclkeyFieldList_t));
    memset(&v6fldData, 0x00, sizeof(xpEaclkeyFieldList_t));
    memset(&v4ruleData, 0x00, sizeof(xpEaclkeyFieldList_t));
    memset(&v6ruleData, 0x00, sizeof(xpEaclkeyFieldList_t));

    numOfEaclDataEntries = numOfEntry(&configFile.eaclData);

    if (numOfEaclDataEntries == 0)
    {
        return XP_NO_ERR;
    }

    /*Identify unique entries for preparing a new key to write in SE reg */
    for (i = 0; i < numOfEaclDataEntries; i++)
    {
        eaclKey = (EaclL2KeyAttribute *)getEntry(&configFile.eaclKey, i);
        PRINT_LOG("EACL TYPE : %d\n", eaclKey->type);
        switch (eaclKey->type)
        {
            case XP_EACL_V4_TYPE:
                if (eaclKey->type != -1 && v4KyFldLst[XP_EACL_KEY_TYPE_V4] == 0)
                {
                    v4KyFldLst[XP_EACL_KEY_TYPE_V4] = 1;
                    v4ByteMask[XP_EACL_KEY_TYPE_V4] = xpEaclV4KeyByteMask[XP_EACL_KEY_TYPE_V4];
                    numV4Fileds++;
                }
                if (eaclKey->isDmacSet && v4KyFldLst[XP_EACL_MAC_DA] == 0)
                {
                    v4KyFldLst[XP_EACL_MAC_DA] = 1;
                    v4ByteMask[XP_EACL_MAC_DA] = xpEaclV4KeyByteMask[XP_EACL_MAC_DA];
                    numV4Fileds++;
                }
                if (eaclKey->isSmacSet && v4KyFldLst[XP_EACL_MAC_SA] == 0)
                {
                    v4KyFldLst[XP_EACL_MAC_SA] = 1;
                    v4ByteMask[XP_EACL_MAC_SA] = xpEaclV4KeyByteMask[XP_EACL_MAC_SA];
                    numV4Fileds++;
                }
                if (eaclKey->isDIPSet && v4KyFldLst[XP_EACL_DIP_V4] == 0)
                {
                    v4KyFldLst[XP_EACL_DIP_V4] = 1;
                    v4ByteMask[XP_EACL_DIP_V4] = xpEaclV4KeyByteMask[XP_EACL_DIP_V4];
                    numV4Fileds++;
                }
                if (eaclKey->isSIPSet && v4KyFldLst[XP_EACL_SIP_V4] == 0)
                {
                    v4KyFldLst[XP_EACL_SIP_V4] = 1;
                    v4ByteMask[XP_EACL_SIP_V4] = xpEaclV4KeyByteMask[XP_EACL_SIP_V4];
                    numV4Fileds++;
                }
                if (eaclKey->L4DestPort != -1 && v4KyFldLst[XP_EACL_L4_DEST_PORT] == 0)
                {
                    v4KyFldLst[XP_EACL_L4_DEST_PORT] = 1;
                    v4ByteMask[XP_EACL_L4_DEST_PORT] = xpEaclV4KeyByteMask[XP_EACL_L4_DEST_PORT];
                    numV4Fileds++;
                }
                if (eaclKey->L4SourcePort != -1 && v4KyFldLst[XP_EACL_L4_SRC_PORT] == 0)
                {
                    v4KyFldLst[XP_EACL_L4_SRC_PORT] = 1;
                    v4ByteMask[XP_EACL_L4_SRC_PORT] = xpEaclV4KeyByteMask[XP_EACL_L4_SRC_PORT];
                    numV4Fileds++;
                }
                if ((eaclKey->egressVif != -1) && (v4KyFldLst[XP_EACL_EGRESS_VIF] == 0))
                {
                    v4KyFldLst[XP_EACL_EGRESS_VIF] = 1;
                    v4ByteMask[XP_EACL_EGRESS_VIF] = xpEaclV4KeyByteMask[XP_EACL_EGRESS_VIF];
                    numV4Fileds++;
                }
                if ((eaclKey->eVlan != -1) && (v4KyFldLst[XP_EACL_EGRESS_BD] == 0))
                {
                    v4KyFldLst[XP_EACL_EGRESS_BD] = 1;
                    v4ByteMask[XP_EACL_EGRESS_BD] = xpEaclV4KeyByteMask[XP_EACL_EGRESS_BD];
                    numV4Fileds++;
                }
                if (eaclKey->icmpMessageType != -1 && v4KyFldLst[XP_EACL_ICMP_MSG_TYPE] == 0)
                {
                    v4KyFldLst[XP_EACL_ICMP_MSG_TYPE] = 1;
                    v4ByteMask[XP_EACL_ICMP_MSG_TYPE] = xpEaclV4KeyByteMask[XP_EACL_ICMP_MSG_TYPE];
                    numV4Fileds++;
                }
                if (eaclKey->protocol != -1 && v4KyFldLst[XP_EACL_PROTOCOL] == 0)
                {
                    v4KyFldLst[XP_EACL_PROTOCOL] = 1;
                    v4ByteMask[XP_EACL_PROTOCOL] = xpEaclV4KeyByteMask[XP_EACL_PROTOCOL];
                    numV4Fileds++;
                }
                if (eaclKey->vi != -1 && v4KyFldLst[XP_EACL_VI] == 0)
                {
                    v4KyFldLst[XP_EACL_VI] = 1;
                    v4ByteMask[XP_EACL_VI] = xpEaclV4KeyByteMask[XP_EACL_PROTOCOL];
                    numV4Fileds++;
                }
                if (((eaclKey->dscp != -1) || (eaclKey->pcp != -1) || (eaclKey->exp != -1)) &&
                    v4KyFldLst[XP_EACL_DSCP_PCP_EXP] == 0)
                {
                    v4KyFldLst[XP_EACL_DSCP_PCP_EXP] = 1;
                    v4ByteMask[XP_EACL_DSCP_PCP_EXP] = xpEaclV4KeyByteMask[XP_EACL_DSCP_PCP_EXP];
                    numV4Fileds++;
                }
                if (((eaclKey->tcpFlags != -1) || (eaclKey->reasonCode != -1)) &&
                    v4KyFldLst[XP_EACL_TCP_FLAGS_RSNCODE] == 0)
                {
                    v4KyFldLst[XP_EACL_TCP_FLAGS_RSNCODE] = 1;
                    v4ByteMask[XP_EACL_TCP_FLAGS_RSNCODE] =
                        xpEaclV4KeyByteMask[XP_EACL_TCP_FLAGS_RSNCODE];
                    numV4Fileds++;
                }
                if (eaclKey->etherType != -1 &&
                    v4KyFldLst[XP_EACL_V4_ETHER_TYPE] == 0) //TODO: issue with this field
                {
                    v4KyFldLst[XP_EACL_V4_ETHER_TYPE] = 1;
                    v4ByteMask[XP_EACL_V4_ETHER_TYPE] = xpEaclV4KeyByteMask[XP_EACL_V4_ETHER_TYPE];
                    numV4Fileds++;
                }
                break;

            case XP_EACL_V6_TYPE:

                if (eaclKey->type != -1 && v6KyFldLst[XP_EACL_KEY_TYPE_V6] == 0)
                {
                    v6KyFldLst[XP_EACL_KEY_TYPE_V6] = 1;
                    v6ByteMask[XP_EACL_KEY_TYPE_V6] = xpEaclV6KeyByteMask[XP_EACL_KEY_TYPE_V6];
                    numV6Fileds++;
                }
                if (eaclKey->isDIP6Set && v6KyFldLst[XP_EACL_DIP_V6] == 0)
                {
                    v6KyFldLst[XP_EACL_DIP_V6] = 1;
                    v6ByteMask[XP_EACL_DIP_V6] = xpEaclV6KeyByteMask[XP_EACL_DIP_V6];
                    numV6Fileds++;
                }
                if (eaclKey->isSIP6Set && v6KyFldLst[XP_EACL_SIP_V6] == 0)
                {
                    v6KyFldLst[XP_EACL_SIP_V6] = 1;
                    v6ByteMask[XP_EACL_SIP_V6] = xpEaclV6KeyByteMask[XP_EACL_SIP_V6];
                    numV6Fileds++;
                }
                if (eaclKey->L4DestPort != -1 && v6KyFldLst[XP_EACL_L4_V6_DEST_PORT] == 0)
                {
                    v6KyFldLst[XP_EACL_L4_V6_DEST_PORT] = 1;
                    v6ByteMask[XP_EACL_L4_V6_DEST_PORT] =
                        xpEaclV6KeyByteMask[XP_EACL_L4_V6_DEST_PORT];
                    numV6Fileds++;
                }
                if (eaclKey->L4SourcePort != -1 && v6KyFldLst[XP_EACL_L4_V6_SRC_PORT] == 0)
                {
                    v6KyFldLst[XP_EACL_L4_V6_SRC_PORT] = 1;
                    v6ByteMask[XP_EACL_L4_V6_SRC_PORT] =
                        xpEaclV6KeyByteMask[XP_EACL_L4_V6_SRC_PORT];
                    numV6Fileds++;
                }

                if (eaclKey->egressVif != -1 && v6KyFldLst[XP_EACL_V6_EGRESS_VIF] == 0)
                {
                    v6KyFldLst[XP_EACL_V6_EGRESS_VIF] = 1;
                    v6ByteMask[XP_EACL_V6_EGRESS_VIF] = xpEaclV6KeyByteMask[XP_EACL_V6_EGRESS_VIF];
                    numV6Fileds++;
                }
                if (eaclKey->eVlan != -1 && v6KyFldLst[XP_EACL_V6_EGRESS_BD] == 0)
                {
                    v6KyFldLst[XP_EACL_V6_EGRESS_BD] = 1;
                    v6ByteMask[XP_EACL_V6_EGRESS_BD] = xpEaclV6KeyByteMask[XP_EACL_V6_EGRESS_BD];
                    numV6Fileds++;
                }
                if (eaclKey->etherType != -1 &&
                    v6KyFldLst[XP_EACL_V6_ETHER_TYPE] == 0) //TODO: issue with this field
                {
                    v6KyFldLst[XP_EACL_V6_ETHER_TYPE] = 1;
                    v6ByteMask[XP_EACL_V6_ETHER_TYPE] = xpEaclV6KeyByteMask[XP_EACL_V6_ETHER_TYPE];
                    numV6Fileds++;
                }
                if (eaclKey->icmpMessageType != -1 && v6KyFldLst[XP_EACL_ICMP_V6_MSG_TYPE] == 0)
                {
                    v6KyFldLst[XP_EACL_ICMP_V6_MSG_TYPE] = 1;
                    v6ByteMask[XP_EACL_ICMP_V6_MSG_TYPE] =
                        xpEaclV6KeyByteMask[XP_EACL_ICMP_V6_MSG_TYPE];
                    numV6Fileds++;
                }
                /* Not sure about the XP_EACL_V6_EGRESS_BD, so skipping it */
                if (((eaclKey->pcp != -1) || (eaclKey->exp != -1)) &&
                    v6KyFldLst[XP_EACL_V6_RSNCODE_TC_DP] == 0)
                {
                    v6KyFldLst[XP_EACL_V6_RSNCODE_TC_DP] = 1;
                    v6ByteMask[XP_EACL_V6_RSNCODE_TC_DP] =
                        xpEaclV6KeyByteMask[XP_EACL_V6_RSNCODE_TC_DP];
                    numV6Fileds++;
                }

                break;


            default:
                PRINT_LOG("ERROR: invalid EACL type\n");
                break;

        }
    }

    /* For eacl ipv4 type*/
    tmpV4Index = 0;
    v4fldData.numFlds = numV4Fileds;
    v4fldData.type = XP_EACL_V4_TYPE;
    v4fldData.fldList = (xpEaclkeyField_t *)malloc(sizeof(xpEaclkeyField_t) *
                                                   numV4Fileds);
    memset(v4fldData.fldList, 0, sizeof(xpEaclkeyField_t) * numV4Fileds);
    v4fldData.isValid = 1;
    /* Initialize the field data and calculate key size from byte mask */
    for (i =0; i< XP_EACL_IPV4_MAX_FLDS; i++)
    {
        if (v4KyFldLst[i] == 1)
        {
            v4fldData.fldList[tmpV4Index].fld.v4Fld = (xpEaclV4KeyFlds)i ;
            tmpV4Index++;
            /* Count number of ones in mask byte */
            rem = 0;
            bc = 0;
            while (v4ByteMask[i])
            {
                if ((rem = (v4ByteMask[i] & 1)) == 1)
                {
                    ++bc;
                }
                v4ByteMask[i] >>= 1;
            }
            v4KeySize = v4KeySize + bc;
        }
    }
    /* Convert the key size to represent size in bits */
    v4KeySize = v4KeySize * 8;

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
            v6fldData.fldList[tmpV6Index].fld.v6Fld = (xpEaclV6KeyFlds)i ;
            tmpV6Index++;
            /* Count number of ones in mask byte */
            rem = 0;
            bc = 0;
            while (v6ByteMask[i])
            {
                if ((rem = (v6ByteMask[i] & 1)) == 1)
                {
                    ++bc;
                }
                v6ByteMask[i] >>= 1;
            }
            v6KeySize = v6KeySize + bc;
        }
    }
    /* Convert the key size to represent size in bits */
    v6KeySize = v6KeySize * 8;

    /*Create table with max keySize among ipv4 ,ipv6*/
    keySize = (v4KeySize >= v6KeySize) ? v4KeySize : v6KeySize;
    if (keySize == 0)
    {
        PRINT_LOG("Egress ACL Table not created as keySize = 0 \n");
        free(v4fldData.fldList);
        free(v6fldData.fldList);
        return retVal;
    }
    if (keySize <= 64)
    {
        keySize = WIDTH_64;
    }
    else if (keySize > 64 && keySize <= 128)
    {
        keySize = WIDTH_128;
    }
    else if (keySize > 128 && keySize <= 192)
    {
        keySize = WIDTH_192;
    }
    else
    {
        keySize = WIDTH_384;
    }
    PRINT_LOG("eACL : Key Size: %d\n", keySize);
    tblProfile.numTables = 1;
    //tblProfile.tableProfile.tblType = EACL0_TBL;
    tblProfile.tableProfile.keySize = keySize;
    tblProfile.tableProfile.numDb = 1;

    if ((retVal = xpsEaclCreateTable(devNum, tblProfile)) != XP_NO_ERR)
    {
        PRINT_LOG("Egress ACL create Table returns %d :\n", retVal);
        PRINT_LOG("%s:, ERROR: Failed to createTable \n", __FUNCTION__);
        free(v4fldData.fldList);
        free(v6fldData.fldList);
        return retVal;
    }
    PRINT_LOG("eACL : Table Created SucessFully\n");

    //To-DO: need this for all the lookup types if necessary
    /*For ipv4 eacl */
    if ((retVal = xpsEaclDefineKey(devNum, &v4fldData)) != XP_NO_ERR)
    {
        PRINT_LOG("%s:, ERROR: xpsEaclDefineKey() failed for ipv4 type \n",
                  __FUNCTION__);
        free(v4fldData.fldList);
        free(v6fldData.fldList);
        return retVal;
    }
    PRINT_LOG("eACL : xpsEaclDefineKey() for Ipv4 type Done ! \n");

    /*For ipv6 eacl */
    if ((retVal = xpsEaclDefineKey(devNum, &v6fldData)) != XP_NO_ERR)
    {
        PRINT_LOG("%s:, ERROR: xpsEaclDefineKey() failed for ipv6 type \n",
                  __FUNCTION__);
        free(v4fldData.fldList);
        free(v6fldData.fldList);
        return retVal;
    }
    PRINT_LOG("eACL : xpsEaclDefineKey() for ipv6 type Done ! \n");

    PRINT_LOG("eACL  Enabling Eacl \n");
    xpsEaclEnable(devNum);

    for (i = 0; i < numOfEaclDataEntries; i++)
    {
        eaclDataEntry = (xpsEaclData_t *)malloc(sizeof(xpsEaclData_t));
        //memset(eaclData,0,sizeof(EaclData));
        memset(eaclDataEntry, 0, sizeof(xpsEaclData_t));

        /* Copy field data from reference key details struct and fill up values */
        memcpy(&v4ruleData, &v4fldData, sizeof(xpEaclkeyFieldList_t));
        memcpy(&v6ruleData, &v6fldData, sizeof(xpEaclkeyFieldList_t));

        eaclData = (EaclData *)getEntry(&configFile.eaclData, i);
        eaclKey = (EaclL2KeyAttribute *)getEntry(&configFile.eaclKey, i);
        eaclMask = (EaclL2MaskAttribute *)getEntry(&configFile.eaclMask, i);

        tmpTypeMask = eaclMask->type | 0xFC;
        if ((v4KyFldLst[XP_EACL_EGRESS_VIF] || v6KyFldLst[XP_EACL_V6_EGRESS_VIF]) &&
            eaclKey->egressVif != -1)
        {
            //evif change for N devices
            retVal = xpsPortGetPortIntfId(devNum, eaclKey->egressVif, &egressVif);
            if (retVal)
            {
                PRINT_LOG("Error : xpsPortGetPortIntfId() Failed for devId %d portId : %d\n",
                          devNum, eaclKey->egressVif);
                free(v4fldData.fldList);
                free(v6fldData.fldList);
                free(eaclDataEntry);
                return retVal;
            }
        }

        if (eaclKey->type  == 0)
        {

            if (v4KyFldLst[XP_EACL_KEY_TYPE_V4])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_KEY_TYPE_V4, v4KyFldLst, kefIndex);
                v4ruleData.fldList[kefIndex].value = (uint8_t *)&eaclKey->type;
                v4ruleData.fldList[kefIndex].mask = &tmpTypeMask;
            }
            if (v4KyFldLst[XP_EACL_MAC_SA])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_MAC_SA, v4KyFldLst, kefIndex);
                v4ruleData.fldList[kefIndex].value = eaclKey->macSa;
                v4ruleData.fldList[kefIndex].mask = eaclMask->macSa;
            }
            if (v4KyFldLst[XP_EACL_MAC_DA])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_MAC_DA, v4KyFldLst, kefIndex);
                v4ruleData.fldList[kefIndex].value = eaclKey->macDa;
                v4ruleData.fldList[kefIndex].mask = eaclMask->macDa;
            }
            if (v4KyFldLst[XP_EACL_DIP_V4])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_DIP_V4, v4KyFldLst, kefIndex);
                v4ruleData.fldList[kefIndex].value = eaclKey->dip;
                v4ruleData.fldList[kefIndex].mask = eaclMask->dip;
            }
            if (v4KyFldLst[XP_EACL_SIP_V4])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_SIP_V4, v4KyFldLst, kefIndex);
                v4ruleData.fldList[kefIndex].value = eaclKey->sip;
                v4ruleData.fldList[kefIndex].mask = eaclMask->sip;
            }
            if (v4KyFldLst[XP_EACL_L4_DEST_PORT])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_L4_DEST_PORT, v4KyFldLst, kefIndex);
                tmpUintArray[0] = htole32(eaclKey->L4DestPort);
                tmpUintArray[1] = htole32(eaclMask->L4DestPort);
                v4ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[0];
                v4ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[1];
            }
            if (v4KyFldLst[XP_EACL_L4_SRC_PORT])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_L4_SRC_PORT, v4KyFldLst, kefIndex);
                tmpUintArray[2] = htole32(eaclKey->L4SourcePort);
                tmpUintArray[3] = htole32(eaclMask->L4SourcePort);
                v4ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[2];
                v4ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[3];
            }
            if (v4KyFldLst[XP_EACL_ICMP_MSG_TYPE])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_ICMP_MSG_TYPE, v4KyFldLst, kefIndex);
                tmpUintArray[6] = htole32(eaclKey->icmpMessageType);
                tmpUintArray[7] = htole32(eaclMask->icmpMessageType);
                v4ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[6];
                v4ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[7];
            }
            if (v4KyFldLst[XP_EACL_PROTOCOL])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_PROTOCOL, v4KyFldLst, kefIndex);
                tmpUintArray[8] = htole32(eaclKey->protocol);
                tmpUintArray[9] = htole32(eaclMask->protocol);
                v4ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[8];
                v4ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[9];
            }
            if (v4KyFldLst[XP_EACL_VI])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_VI, v4KyFldLst, kefIndex);
                tmpUintArray[10] = htole32(eaclKey->vi);
                tmpUintArray[11] = htole32(eaclMask->vi);
                v4ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[10];
                v4ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[11];
            }
            if (v4KyFldLst[XP_EACL_EGRESS_VIF])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_EGRESS_VIF, v4KyFldLst, kefIndex);
                tmpUintArray[12] = htole32(eaclKey->egressVif);
                tmpUintArray[13] = htole32(eaclMask->egressVif);
                v4ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[12];
                v4ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[13];
            }
            if (v4KyFldLst[XP_EACL_EGRESS_BD])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_EGRESS_BD, v4KyFldLst, kefIndex);
                tmpUintArray[14] = htole32(eaclKey->eVlan);
                tmpUintArray[15] = htole32(eaclMask->eVlan);
                v4ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[14];
                v4ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[15];
            }
            memset(tmpDscpData, 0x00, sizeof(uint8_t) * 2);
            memset(tmpDscpMask, 0x00, sizeof(uint8_t) * 2);
            if (v4KyFldLst[XP_EACL_DSCP_PCP_EXP])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_DSCP_PCP_EXP, v4KyFldLst, kefIndex);
                tmpDscpData[0] = (eaclKey->dscp & 0x3F) | ((eaclKey->pcp & 0x03) << 6);
                tmpDscpData[1] = ((eaclKey->pcp & 0x04) >> 2) | ((eaclKey->exp & 0x07) << 1);
                v4ruleData.fldList[kefIndex].value = tmpDscpData;

                tmpDscpMask[0] = (eaclMask->dscp & 0x3F) | ((eaclMask->pcp & 0x03) << 6);
                tmpDscpMask[1] = ((eaclMask->pcp & 0x04) >> 2) | ((eaclMask->exp & 0x07) << 1) |
                                 0xf0;
                v4ruleData.fldList[kefIndex].mask = tmpDscpMask;
            }
            memset(tmpTcpData, 0x00, sizeof(uint8_t) * 3);
            memset(tmpTcpMask, 0x00, sizeof(uint8_t) * 3);
            if (v4KyFldLst[XP_EACL_TCP_FLAGS_RSNCODE])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_TCP_FLAGS_RSNCODE, v4KyFldLst, kefIndex);
                tmpTcpData[0] = eaclKey->tcpFlags & 0xFF;
                tmpTcpData[1] = ((eaclKey->tcpFlags & 0x100) >> 8) | ((eaclKey->reasonCode &
                                                                       0x7F) << 1);
                tmpTcpData[2] = (eaclKey->reasonCode & 0x280) >> 7;
                v4ruleData.fldList[kefIndex].value = tmpTcpData;

                tmpTcpMask[0] = eaclMask->tcpFlags & 0xFF;
                tmpTcpMask[1] = ((eaclMask->tcpFlags & 0x100) >> 8) | ((
                                                                           eaclKey->reasonCode & 0x7F) << 1);
                tmpTcpMask[2] = ((eaclMask->reasonCode & 0x280) >> 7) | 0xF8;
                v4ruleData.fldList[kefIndex].mask = tmpTcpMask;
            }
            if (v4KyFldLst[XP_EACL_V4_ETHER_TYPE])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_V4_ETHER_TYPE, v4KyFldLst, kefIndex);
                tmpUintArray[4] = htole32(eaclKey->etherType);
                tmpUintArray[5] = htole32(eaclMask->etherType);
                v4ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[4];
                v4ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[5];
            }

            //
            //Set EACL rules
            //
            if ((retVal = xpsEaclWriteKey(devNum, eaclKey->eAclId,
                                          &v4ruleData)) != XP_NO_ERR)
            {
                PRINT_LOG("%s:, ERROR:Failed to set eacl v4 rule for index %d\n", __FUNCTION__,
                          eaclKey->eAclId);
                free(eaclDataEntry);
                return retVal;
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
            if (v6KyFldLst[XP_EACL_V6_ETHER_TYPE])
            {
                GET_INDEX_FOR_KEY_ENTRY_FORMAT(XP_EACL_V6_ETHER_TYPE, v6KyFldLst, kefIndex);
                tmpUintArray[20] = htole32(eaclKey->etherType);
                tmpUintArray[21] = htole32(eaclMask->etherType);
                v6ruleData.fldList[kefIndex].value = (uint8_t *)&tmpUintArray[20];
                v6ruleData.fldList[kefIndex].mask = (uint8_t *)&tmpUintArray[21];
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

            //
            //Set EACL rules
            //
            if ((retVal = xpsEaclWriteKey(devNum, eaclKey->eAclId,
                                          &v6ruleData)) != XP_NO_ERR)
            {
                PRINT_LOG("%s:, ERROR:Failed to set eacl v6 rule for index %d\n", __FUNCTION__,
                          eaclKey->eAclId);
                free(eaclDataEntry);
                free(v4fldData.fldList);
                free(v6fldData.fldList);
                return retVal;
            }

        }
        //
        //Create EACL data
        //
        eaclDataEntry->enPktCmdUpd = eaclData->enPktCmdUpd;
        eaclDataEntry->enRsnCodeUpd = eaclData->enRsnCodeUpd;
        eaclDataEntry->pktCmd = eaclData->pktCmd;
        eaclDataEntry->rsnCode = eaclData->rsnCode;
        PRINT_LOG("pkt cmd en %d \n", eaclData->enPktCmdUpd);
        PRINT_LOG("rsn code update %d \n", eaclData->enRsnCodeUpd);
        PRINT_LOG("pktCmd %d \n", eaclData->pktCmd);
        PRINT_LOG("rsn code %d \n", eaclData->rsnCode);

        //
        //Set eacl data
        //
        if ((retVal = xpsEaclWriteData(devNum, eaclKey->eAclId,
                                       eaclDataEntry)) != XP_NO_ERR)
        {
            PRINT_LOG("%s:, ERROR:Failed to set eacl data for eacl index %d\n",
                      __FUNCTION__, eaclKey->eAclId);
            free(v4fldData.fldList);
            free(v6fldData.fldList);
            return retVal;
        }
        free(eaclDataEntry);
    }
    PRINT_LOG("xpsAppConfigureEacl done successfully\n");
    free(v4fldData.fldList);
    free(v6fldData.fldList);
    return retVal;
}
#endif

int testMplsTunnel(xpsDevice_t deviceId, XP_PIPE_MODE mode)
{
    ////TODO: do we need this. if we do, please use FL apis.
    //xpMplsTunnelEntry tblEntry;
    //xpMplsTunnelEntry getTblEntry;
    //uint32_t index = 0, reHashIndex = 0;
    //int32_t lookupIdx = 0;

    //PRINT_LOG("dbg: Inside %s func\n", __FUNCTION__);


    // //
    //// Dev Init for the MPLS  Manager
    ////
    //mplsTunnelMgr = (xpMplsTunnelMgr *)xpTunnelPl::instance()->getMgr(xpMplsTunnelMgr::getId());

    //if(mplsTunnelMgr == NULL)
    //{
    //    PRINT_LOG("ERROR <dev = %d> :mplsTunnelMgr is NULL. Exiting...\n",0);
    //    return XP_ERR_INIT;
    //}
    //uint8_t tblCopyNum = tableProfile->getXpIsmeTblCopyNum(MPLS_TNL_TBL);
    //xpIsmeTableContext* mplsTunnelTblCtx[tblCopyNum];
    //for(uint8_t i = 0; i < tblCopyNum; i++)
    //{
    //    mplsTunnelTblCtx[i] = tableProfile->getXpIsmeCfg(MPLS_TNL_TBL, i);
    //}


    //xpMplsTunnelMgr *xpPmMgr = static_cast<xpMplsTunnelMgr *>(xpTunnelPl::instance()->getMgr(xpMplsTunnelMgr::getId()));


    //memset((void *)&tblEntry, 0 , sizeof(xpMplsTunnelEntry));
    //for (int i = 0; i < 5; i++)
    //{
    //    tblEntry.KeyOuterVlanId = 0;
    //    tblEntry.KeyMplsLabel = i + 29;
    //    tblEntry.isP2MPBudNode = 1U;
    //    if(xpPmMgr->addEntry(deviceId, (void *)&tblEntry, index, reHashIndex) != XP_NO_ERR)
    //    {
    //        PRINT_LOG("ERROR in inserting an entry. Exiting...\n");
    //        //return -1;
    //    }
    //    PRINT_LOG("index of %dth entry : %d \n",i+1, index);
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
    //    PRINT_LOG("ERROR in inserting an entry. Exiting...\n");
    //    return -1;
    //}

    //getTblEntry.KeyOuterVlanId = 25U;
    //getTblEntry.KeyMplsLabel = 40U;

    //xpPmMgr->lookup(deviceId, (void *)&getTblEntry, lookupIdx);
    //if(lookupIdx != (int32_t)index)
    //{
    //    PRINT_LOG("Lookup Mismatch:: Expected Value %d Actual Value %d \n", index, lookupIdx);
    //}
    //else
    //{
    //    PRINT_LOG("Lookup PASS:: Index %d Actual Value %d \n", lookupIdx, index);
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
    //    PRINT_LOG("ERROR in updating an entry. Exiting...\n");
    //    return -1;
    //}

    ///* Read whole Entry */
    //if(xpPmMgr->readEntry(deviceId, index, (void *)&getTblEntry) != XP_NO_ERR)
    //{
    //    PRINT_LOG("ERROR in reading an entry. Exiting...\n");
    //    return -1;
    //}
    ////compareMplsTunnelEntry(&tblEntry, &getTblEntry);

    //uint32_t floodVif = 123U, getFloodVif = 0;
    //xpPmMgr->setField(deviceId, index, MPLSTUNNEL_FLOOD_VIF, (void *)&floodVif);
    //xpPmMgr->getField(deviceId, index, MPLSTUNNEL_FLOOD_VIF, (void *)&getFloodVif);
    //if(floodVif != getFloodVif)
    //{
    //    PRINT_LOG("Compare floodVif :: FAIL\n");
    //} else
    //{
    //    PRINT_LOG("Compare floodVif :: PASS\n");
    //}

    return 0;
}

XP_STATUS natPacketHandler(xpsDevice_t devId, xphRxHdr *xphHdr, void *buf,
                           uint16_t bufSize)
{
    //TODO ::  Verify from Ashwini weather this is stale then remove
    XP_STATUS status = XP_NO_ERR;
#if 0
    PRINT_LOG("nat packet handler \n");
    int SIP_OFFSET = 26;
    ipv4Addr_t sip;
    ipv4Addr_t dip;

    buf = (uint8_t*)buf + SIP_OFFSET;
    memcpy(&sip, buf, sizeof(ipv4Addr_t));
    PRINT_LOG(" SIP Address :: 0x%x:0x%x:0x%x:0x%x\n", sip[0], sip[1], sip[2],
              sip[3]);
    buf = (uint8_t*)buf + XP_IPV4_ADDR_LEN;
    memcpy(&dip, buf, sizeof(ipv4Addr_t));
    PRINT_LOG(" DIP Address :: 0x%x:0x%x:0x%x:0x%x\n", dip[0], dip[1], dip[2],
              dip[3]);

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

    PRINT_LOG("Inside natPacketHandler \n");
#endif
    return status;
}

//TODO
XP_STATUS xpsAppWriteNatData(xpsDevice_t devId)
{
    XP_STATUS retVal = XP_NO_ERR;
    int numOfNatDataEntries = 0;
    uint32_t j = 0;
    xpsNatEntryKey_t xpsKey;
    xpsNatEntryMask_t xpsMask;
    xpsNatEntryData_t xpsData;
    NatEntry *natEntry;

    numOfNatDataEntries = numOfEntry(&configFile.natData);
    int i;
    for (i = 0; i < numOfNatDataEntries; i++)
    {
        natEntry = (NatEntry *)getEntry(&configFile.natData, i);
        xpsAppVlanContext *vlanCtx = (xpsAppVlanContext *)malloc(sizeof(
                                                                     xpsAppVlanContext));
        //initVlanContext(vlanCtx);
        if (!vlanCtx)
        {
            PRINT_LOG("%s:Error: Can't allocate vlan context for vlan Id\n", __FUNCTION__);
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
            PRINT_LOG("calling external entry \n");
            if ((retVal = xpsNatAddExternalEntry(devId, i, &xpsKey, &xpsMask,
                                                 &xpsData)) != XP_NO_ERR)
            {
                PRINT_LOG("%s:Error: Writing external NAT IPv4 entry at index %d failed. error-code = %d\n",
                          __FUNCTION__, i, retVal);
                free(vlanCtx);
                return retVal;
            }
        }
        else if (natEntry->natMode == 0x3 && natEntry->pktCmd != 0x1)
        {
            PRINT_LOG("calling Add entry \n");
            if ((retVal = xpsNatAddDoubleEntry(devId, i, &xpsKey, &xpsMask,
                                               &xpsData)) != XP_NO_ERR)
            {
                PRINT_LOG("%s:Error: Writing NAT entry at index %d failed error-code = %d \n",
                          __FUNCTION__, i, retVal);
                free(vlanCtx);
                return retVal;
            }
        }
        else if (natEntry->pktCmd != 0x1)
        {
            PRINT_LOG("calling internal entry \n");
            if ((retVal = xpsNatAddInternalEntry(devId, i, &xpsKey, &xpsMask,
                                                 &xpsData)) != XP_NO_ERR)
            {
                PRINT_LOG("%s:Error: Writing internal NAT IPv4 entry at index %d failed. error-code = %d\n",
                          __FUNCTION__, i, retVal);
                free(vlanCtx);
                return retVal;
            }
        }
        else
        {
            PRINT_LOG("calling filter rule \n");

            if ((retVal = xpsNatAddFilterRule(devId, i, &xpsKey, &xpsMask,
                                              &xpsData)) != XP_NO_ERR)
            {
                PRINT_LOG("%s:Error: Writing Fileter Rule at index %d failed. error-code = %d\n",
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
    //PRINT_LOG("Exiting on success\n");
    return XP_NO_ERR; //TODO
}

//TODO
XP_STATUS xpAppSendPacketToXp(xpsDevice_t deviceId)
{
    XP_STATUS retVal = XP_NO_ERR;
    int pktHexFile = 0;
    uint32_t pktLen = 0;
    char *pktBufOffset = NULL;
    struct xpPacketInfo *pktInfo = NULL;
    uint32_t numOfPkt;
    int queueNumArr[65] = {0, }, i = 0;
    uint32_t numOfPcapEntry = numOfEntry(&configFile.controlPcapList);
    xpInputControlPktEntry *pcapEntry = NULL;
    uint32_t entryNum = 0, offset = 0, mcpuSdePortNum = 0, mcpuPort = 0;
    xphTxHdr txHeader;
    xpsInterfaceId_t ingressVif;
    xpsInterfaceId_t egressVif;

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
            PRINT_LOG("%s:Error: malloc for pktInfo failed\n", __FUNCTION__);
            return XP_ERR_MEM_ALLOC_ERROR;
        }

        i = 0;
        while (queueNumArr[i] != -1)
        {
            memset(pktInfo, 0, sizeof(struct xpPacketInfo));

            pktInfo->priority = queueNumArr[i];
            pktHexFile = open(pcapEntry->pcapName, O_RDONLY, (int)0600);
            if (pktHexFile == -1)
            {
                PRINT_LOG("%s: Error: could not get Pkt Hex File: %s\n", __FUNCTION__,
                          strerror(errno));
                free(pktInfo);
                return XP_ERR_FILE_OPEN;
            }

            pktInfo->buf = malloc(XP_MAX_PACKET_SIZE);
            if (!pktInfo->buf)
            {
                PRINT_LOG("%s: ERR: Could not get buffer\n", __FUNCTION__);
                free(pktInfo);
                close(pktHexFile);
                return XP_ERR_MEM_ALLOC_ERROR;
            }

            // Retrive the pvif for the mcpu sde port
            // if ingress port is cpu port then get vif for cpu port from PortConfig
            if ((retVal = xpsGlobalSwitchControlGetMcpuPortNumber(deviceId,
                                                                  &mcpuPort)) != XP_NO_ERR)
            {
                free(pktInfo->buf);
                free(pktInfo);
                close(pktHexFile);
                return retVal;
            }
            if (pcapEntry->ingressPort == mcpuPort)
            {
                if ((retVal = xpsGlobalSwitchControlGetMcpuSdePortNumber(deviceId,
                                                                         &mcpuSdePortNum)) != XP_NO_ERR)
                {
                    free(pktInfo->buf);
                    free(pktInfo);
                    close(pktHexFile);
                    return retVal;
                }

                // Retrive the pvif for the mcpu sde port
                pcapEntry->ingressPort = mcpuSdePortNum;
            }

            if ((retVal = xpsPortGetPortIntfId(deviceId, pcapEntry->ingressPort,
                                               &ingressVif)) != XP_NO_ERR)
            {
                free(pktInfo->buf);
                free(pktInfo);
                close(pktHexFile);
                PRINT_LOG("%s: Error: Failed to get the interface ID for device : %d, port : %d\n",
                          __FUNCTION__, deviceId, pcapEntry->ingressPort);
                return retVal;
            }
            if ((retVal = xpsPortGetPortIntfId(deviceId, pcapEntry->egressPort,
                                               &egressVif)) != XP_NO_ERR)
            {
                free(pktInfo->buf);
                free(pktInfo);
                close(pktHexFile);
                PRINT_LOG("%s: Error: Failed to get the interface ID for device : %d, port : %d\n",
                          __FUNCTION__, deviceId, pcapEntry->egressPort);
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
                //xpsPacketDriverEncapHeader(deviceId, &pktInfo->buf, &txHeader);
                offset = sizeof(xphTxHdr);
            }
            else if (strcmp(pcapEntry->nextEngine, "URW") == 0)
            {
                txHeader.nextEngine = 16;
                //xpsPacketDriverEncapHeader(deviceId, &pktInfo->buf, &txHeader);
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
                free(pktInfo->buf);
                free(pktInfo);
                PRINT_LOG("%s:Error: xpGetPktDataAndPktLength failed with error code = %d\n",
                          __FUNCTION__, retVal);
                return retVal;
            }

            /* Set packet size */
            pktInfo->bufSize = pktLen + sizeof(xphTxHdr);
            numOfPkt = 1;
            uint32_t outPortNum = 63;//Hard coded value
            if ((retVal = xpsPacketDriverSend(deviceId,
                                              (const struct xpPacketInfo **)&pktInfo, &numOfPkt, outPortNum,
                                              SYNC_TX)) != XP_NO_ERR)
            {
                close(pktHexFile);
                free(pktInfo->buf);
                free(pktInfo);
                PRINT_LOG("%s:Error: xpPacketDriverSend failed with error code = %d\n",
                          __FUNCTION__, retVal);
                return retVal;
            }
            close(pktHexFile);
            free(pktInfo->buf);
            i++;
        }
        free(pktInfo);
    }
    return XP_NO_ERR;
}

XP_STATUS xpAppAddDevice(xpsDevice_t deviceId, xpsInitType_t initType,
                         xpDevConfigStruct* config)
{
    XP_STATUS status = XP_NO_ERR;
    XP_DEV_TYPE_T devType;

    if (xpSalGetDeviceType(deviceId, &devType) != XP_NO_ERR)
    {
        PRINT_LOG("Error <dev = %d>: xpSalGetDeviceType failed.\n", deviceId);
        return XP_ERR_INIT;
    }

    // XDK init for specific device and type
    if ((status = xpsAppInitDevice(deviceId, initType, config)) != XP_NO_ERR)
    {
        PRINT_LOG("xpsAppInitDevice failed. RC: %u\n", status);
        return XP_ERR_INIT_FAILED;
    }

    PRINT_LOG("xpsAppInitDevice Successful!!!\n");


    if ((status = xpAppPacketDriverCallbackConfigurations(deviceId)) != XP_NO_ERR)
    {
        return status;
    }

    if (xpsIsDevInitDone(deviceId))
    {
        xpsSetInitType(INIT_COLD);
        PRINT_LOG(" InitType set to INIT_COLD \n");
    }
    initType = xpsGetInitType();
    //
    // Demo : Program the HW Tables using the config data input
    //
    if ((status = xpsAppPopulateTables(deviceId, initType, config)) != XP_NO_ERR)
    {
        PRINT_LOG("xpsAppPopulateTables failed. RC: %u\n", status);
        return XP_ERR_INIT_FAILED;
    }

    if ((status = xpAppSchedPolicyInit(deviceId)) != XP_NO_ERR)
    {
        PRINT_LOG("xpAppSchedPolicyInit failed. RC: %u\n", status);
        return XP_ERR_INIT_FAILED;
    }

    if ((status = xpAppPortTuneAndANThreadCreate(deviceId)) != XP_NO_ERR)
    {
        PRINT_LOG("xpAppPortTuneThreadCreate failed. RC: %u\n", status);
        return XP_ERR_INIT_FAILED;
    }

    if ((status = xpAppFdbL2LearningThreadCreate(deviceId)) != XP_NO_ERR)
    {
        PRINT_LOG("xpAppFdbL2LearningThreadCreate failed. RC: %u\n", status);
        return XP_ERR_INIT_FAILED;
    }

    if (XP_CONFIG.intEnable)
    {
        if ((status = xpAppIntThreadCreate(deviceId)) != XP_NO_ERR)
        {
            PRINT_LOG("xpAppIntTableUpdateThread failed. RC: %u\n", status);
            return XP_ERR_INIT_FAILED;
        }
        else
        {
            PRINT_LOG("INT thread created..\n");
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpAppCleanup()
{
    XP_STATUS status = XP_NO_ERR;
    PRINT_LOG("Inside %s\n", __FUNCTION__);

    if (xpsGetExitStatus() <= 0)
    {
        xpsSetExitStatus();
    }
    while (xpsGetExitStatus() < 2)
    {
        usleep(100);
    }
    sleep(1);
    if ((status = xpsProcessTerminationHandler()) != XP_NO_ERR)
    {
        PRINT_LOG("xpsProcessTerminationHandler failed. RC: %u\n", status);
    }

    if (XP_CONFIG.userCli)
    {
        pyDalDeInit();
    }
    /* de-initialize entry data structure of configFile */
    deInitConfigEntryList();
    return status;
}

void getPortStat(xpsDevice_t devId, xpsPort_t* devPortList, uint32_t numOfPort)
{
    uint8_t statCounterStart = 0;
    uint8_t statCounterEnd = 31;
    uint32_t portIndex = 0;
    struct xp_Statistics stat;
    XP_STATUS ret = XP_NO_ERR;

    for (portIndex = 0; portIndex < numOfPort; portIndex++)
    {
        memset(&stat, 0, sizeof(stat));
        PRINT_LOG("-------------\n");
        PRINT_LOG("Port#:    %3d\n", devPortList[portIndex]);
        PRINT_LOG("-------------\n");
        ret = xpLinkManagerCounterStatsDirectGet(devId, devPortList[portIndex],
                                                 statCounterStart, statCounterEnd, &stat);

        if (ret == XP_NO_ERR)
        {
            PRINT_LOG("RxOk        0x%-16" PRIx64 "   Rx65-127    0x%-16" PRIx64 "\n",
                      stat.frameRxOk, stat.frameRxLength65To127);
            PRINT_LOG("RxAll       0x%-16" PRIx64 "   Rx128-255   0x%-16" PRIx64 "\n",
                      stat.frameRxAll, stat.frameRxLength128To255);
            PRINT_LOG("RxFCSEr     0x%-16" PRIx64 "   Rx256-511   0x%-16" PRIx64 "\n",
                      stat.frameRxFcsErr, stat.frameRxLength256To511);
            PRINT_LOG("FrameAnyEr  0x%-16" PRIx64 "   Rx512-1023  0x%-16" PRIx64 "\n",
                      stat.frameRxAnyErr, stat.frameRxLength512To1023);
            PRINT_LOG("RxOctGood   0x%-16" PRIx64 "   Rx1024-1518     0x%-16" PRIx64 "\n",
                      stat.octetsRxGoodFrame, stat.frameRxLength1024To1518);
            PRINT_LOG("RxOct       0x%-16" PRIx64 "   Rx1519-UP   0x%-16" PRIx64 "\n",
                      stat.octetsRx, stat.frameRxLength1519AndUp);
            PRINT_LOG("RxUC        0x%-16" PRIx64 "   RxFifoFull  0x%-16" PRIx64 "\n",
                      stat.frameRxUnicastAddr, stat.frameDroppedFromRxFIFOFullCondition);
            PRINT_LOG("RxMC        0x%-16" PRIx64 "   TxOK        0x%-16" PRIx64 "\n",
                      stat.frameRxMulticastAddr, stat.frameTransmittedOk);
            PRINT_LOG("RxBC        0x%-16" PRIx64 "   TxAll       0x%-16" PRIx64 "\n",
                      stat.frameRxBroadcastAddr, stat.frameTransmittedAll);
            PRINT_LOG("RxPause     0x%-16" PRIx64 "   TxErr       0x%-16" PRIx64 "\n",
                      stat.frameRxTypePause, stat.frameTransmittedWithErr);
            PRINT_LOG("RxLenEr     0x%-16" PRIx64 "   TxWithOErr  0x%-16" PRIx64 "\n",
                      stat.frameRxLengthErr, stat.octetsTransmittedWithoutErr);
            PRINT_LOG("RxUndSize   0x%-16" PRIx64 "   TxTotal     0x%-16" PRIx64 "\n",
                      stat.frameRxUndersize, stat.octetsTransmittedTotal);
            PRINT_LOG("RxOverSize  0x%-16" PRIx64 "   TxUC        0x%-16" PRIx64 "\n",
                      stat.frameRxOversize, stat.framesTransmittedUnicast);
            PRINT_LOG("RxFragments 0x%-16" PRIx64 "   TxMC        0x%-16" PRIx64 "\n",
                      stat.fragmentsRx, stat.framesTransmittedMulticast);
            PRINT_LOG("RxJabFrag   0x%-16" PRIx64 "   TxBC        0x%-16" PRIx64 "\n",
                      stat.jabberFrameRx, stat.framesTransmittedBroadcast0);
            PRINT_LOG("RxLen64     0x%-16" PRIx64 "   TxPause     0x%-16" PRIx64 "\n",
                      stat.frameRxLength64, stat.framesTransmittedPause);
        }

    }
}

XP_STATUS xpsAppConfigureMirrorData(xpDevice_t devId)
{
    PRINT_LOG("In Mirror / ERSPAN 2 Programming : xpsAppConfigureMirrorData \n ");
    XP_STATUS status = XP_NO_ERR;
    static MirrorEntry mirrorEntry;
    int entry= 0;
    int analyzer= 0;
    int src= 0;
    uint32_t analyzerIntfId  =0;
    uint32_t analyzerId  =0;
    xpsMirrorData_t data;
    xpsVlan_t vlanId = 0;
    xpsScope_t scopeId;

    if ((status = xpsScopeGetScopeId(devId, &scopeId)) != XP_NO_ERR)
    {
        printf("%s:Error: couldn't get ScopeId for device: %d",  __FUNCTION__, devId);
        return status;
    }

    for (entry= 0; entry < numOfEntry(&configFile.mirrorData); ++entry)
    {
        memset(&mirrorEntry, 0, sizeof(MirrorEntry));
        mirrorEntry = *((MirrorEntry *)getEntry(&configFile.mirrorData, entry));
        for (analyzer= 0; analyzer < (int) mirrorEntry.numAnalyzers; ++analyzer)
        {
            if (mirrorEntry.mirrorType == 0)
            {
                // Fetch port interface id from port db
                if ((status = xpsPortGetPortIntfId(devId,
                                                   mirrorEntry.data.mirrorData.analyzers[analyzer], &analyzerIntfId)) != XP_NO_ERR)
                {
                    PRINT_LOG("%s:Error: couldn't get portIntf id for device: %d, port: %d\n",
                              __FUNCTION__, devId, analyzer);
                    return status;
                }

                if (!analyzer)
                {
                    status= xpsMirrorCreateAnalyzerSessionScope(scopeId, XP_MIRROR_LOCAL, data,
                                                                &analyzerId);
                    if (status != XP_NO_ERR)
                    {
                        PRINT_LOG("Mirror : xpsMirrorCreateAnalyzerSession returns : %d", status);
                        return status;
                    }
                }

                status = xpsMirrorAddAnalyzerInterfaceScope(scopeId, analyzerId,
                                                            analyzerIntfId);
                if (status != XP_NO_ERR)
                {
                    PRINT_LOG("Mirror : xpsMirrorAddAnalyzerInterface returns : %d", status);
                    return status;
                }
            }

            if (mirrorEntry.mirrorType == 1)
            {
                data.erspan2Data.erspanId = mirrorEntry.data.erspanData.erspanSessionId;
                status= xpsMirrorCreateAnalyzerSessionScope(scopeId, XP_MIRROR_ERSPAN2, data,
                                                            &analyzerId);
                if (status != XP_NO_ERR)
                {
                    PRINT_LOG("Erspan2 : xpsMirrorCreateAnalyzerSession returns : %d", status);
                    return status;
                }

                status = xpsMirrorAddAnalyzerInterfaceScope(scopeId, analyzerId,
                                                            l3TnlIntf[devId][mirrorEntry.data.erspanData.tunnelTableIndex]);
                if (status != XP_NO_ERR)
                {
                    PRINT_LOG("Erspan2 : xpsMirrorAddAnalyzerInterface returns : %d", status);
                    return status;
                }
            }

            if (mirrorEntry.mirrorType == 2)
            {
                data.erspan2Data.erspanId = mirrorEntry.data.erspanData.erspanSessionId;
                status= xpsMirrorCreateAnalyzerSessionScope(scopeId, XP_MIRROR_RSPAN, data,
                                                            &analyzerId);
                if (status != XP_NO_ERR)
                {
                    PRINT_LOG("Rspan : xpsMirrorCreateAnalyzerSession returns : %d", status);
                    return status;
                }

                for (src = 0; src < 5; src++)
                {
                    if (mirrorEntry.sources[src].type == TYPE_VLAN)
                    {
                        vlanId = mirrorEntry.sources[src].id.vlan;
                    }
                }

                status = xpsMirrorGetRspanAnalyzerInterface(devId, vlanId, &analyzerIntfId);
                PRINT_LOG(" xpsMirrorGetRspanAnalyzerInterface returns : %d\n", analyzerIntfId);
                if (status != XP_NO_ERR)
                {
                    PRINT_LOG("Rspan : xpsMirrorGetRspanAnalyzerInterface returns : %d", status);
                    return status;
                }

                status = xpsMirrorAddAnalyzerInterfaceScope(scopeId, analyzerId,
                                                            analyzerIntfId);
                if (status != XP_NO_ERR)
                {
                    PRINT_LOG("Rspan : xpsMirrorAddAnalyzerInterface returns : %d", status);
                    return status;
                }
            }

            status = xpsMirrorWriteAnalyzerSession(devId, analyzerId);
            if (status != XP_NO_ERR)
            {
                PRINT_LOG("Mirror/Erspan2 : xpsMirrorWriteAnalyzerSession returns : %d",
                          status);
                return status;
            }

            for (src= 0; src < (int) mirrorEntry.numSources; ++src)
            {
                switch (mirrorEntry.sources[src].type)
                {
                    case TYPE_PORT:
                        // Fetch port interface id from port db
                        if ((status = xpsPortGetPortIntfId(devId, mirrorEntry.sources[src].id.port,
                                                           &analyzerIntfId)) != XP_NO_ERR)
                        {
                            PRINT_LOG("%s:Error: couldn't get portIntf id for device: %d, port: %d\n",
                                      __FUNCTION__, devId, analyzer);
                            return status;
                        }

                        if ((status = xpsPortEnableMirroringScope(scopeId, analyzerIntfId, analyzerId,
                                                                  1)) != XP_NO_ERR)
                        {
                            PRINT_LOG("xpsAppConfigureMirrorData: Error returned when adding source %1d in table entry #%1d.  Error code = %1d.\n",
                                      src, entry, (int) status);
                            return status;
                        }
                        break;

                    case TYPE_VLAN:
                        analyzerIntfId = vlanToL3Intf[mirrorEntry.sources[src].id.vlan];
                        if ((status = xpsVlanSetMirrorToAnalyzer(devId,
                                                                 mirrorEntry.sources[src].id.vlan, true, analyzerId)) != XP_NO_ERR)
                        {
                            return status;
                        }
                        break;

                    case TYPE_LAG:
                        analyzerIntfId = gLagIntfId[devId][mirrorEntry.sources[src].id.lag];
                        if ((status = xpsLagEnableMirroring(devId, analyzerIntfId,
                                                            analyzerId)) != XP_NO_ERR)
                        {
                            PRINT_LOG("xpsAppConfigureMirrorData: Error returned when adding source %1d in table entry #%1d.  Error code = %1d.\n",
                                      src, entry, (int) status);
                            return status;
                        }
                        break;
                    default:
                        PRINT_LOG("xpAppConfigureMirrorData: Unknown source type %1d in table entry #%1d.\n",
                                  (int)mirrorEntry.sources[src].type, entry);
                        return XP_ERR_KEY_NOT_FOUND;
                }
            }
        }
    }

    PRINT_LOG("xpsAppConfigureMirrorData: Mirror entries programmed successfully.\n");
    return status;
}

XP_STATUS xpsAppWriteQosMap(xpDevice_t devId)
{
    XP_STATUS status = XP_NO_ERR;
    int i, profileId;
    uint16_t maxNetworkPorts = 0;

    // By default configure the trust profile

    // Start with trust l2 (4 bits pcp/dei input, profile 0, same values, tc is enumerated, dp is not set)
    for (i = 0; i < (1 << 4); i++)
    {
        uint32_t pcp = (i >> 1) & (0x7);
        uint32_t dei = (i & 0x1);
        uint32_t tc = pcp;

        if ((status = xpsQosPortIngressSetTrafficClassForL2QosProfile(devId,
                                                                      QOSMAP_PROFILE_TRUST, pcp, dei, tc)) != XP_NO_ERR)
        {
            PRINT_LOG("Error: Failed to set tc for L2 QoS profile, status: %d\n", status);
            return status;
        }

        xpsQosPortIngressRemapPriorityForL2QosProfile(devId, QOSMAP_PROFILE_TRUST, pcp,
                                                      dei, pcp, dei, 0, 0);

        // By default, traffic class for other L2Qos profiles should be 0
        for (profileId = 1; profileId < QOSMAP_PROFILE_MAX; profileId++)
        {
            if ((status = xpsQosPortIngressSetTrafficClassForL2QosProfile(devId,
                                                                          (xpQosMapPfl_t)profileId, pcp, dei, 0)) != XP_NO_ERR)
            {
                PRINT_LOG("Error: Failed to set tc for L2 QoS profile, status: %d\n", status);
                return status;
            }
        }
    }

    // Now set the trust l3 (6 bits dscp input, profile 0, same values, tc is enumerated and repeated, dp is not set)
    for (i = 0; i < (1 << 6); i++)
    {
        uint32_t pcp = (i >> 1) & (0x7);
        uint32_t dei = (i & 0x1);
        uint32_t dscp = i;
        uint32_t tc  = (i >> 3);   // TC is unique for each DSCP group (8 in a group)

        if ((status = xpsQosPortIngressSetTrafficClassForL3QosProfile(devId,
                                                                      QOSMAP_PROFILE_TRUST, dscp, tc)) != XP_NO_ERR)
        {
            PRINT_LOG("Error: Failed to set tc for L3 QoS profile, status: %d\n", status);
            return status;
        }
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
    if ((status = xpsGlobalSwitchControlGetMaxNumPhysicalPorts(devId,
                                                               &maxNetworkPorts)) != XP_NO_ERR)
    {
        PRINT_LOG("%s:Error: Error to get max network ports. error code = %d\n",
                  __FUNCTION__, status);
        return status;
    }
    for (i = 0; i < maxNetworkPorts; i++)
    {
        xpsQosPortIngressSetTrustL2ForPort(devId, i);
    }

    return status;
}

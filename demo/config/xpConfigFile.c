// xpConfigFile.cpp

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpConfigFile.h"
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define XP_MAX_TOTAL_PORTS                      128    ///<  Max Total Ports

void initEntryList(entryList *list)
{
    list->data = NULL;
    list->size = 0;
    list->count = 0;
}

int numOfEntry(entryList *list)
{
    return list->count;
}

void *getEntry(entryList *list, int index)
{
    if (index >= list->count)
    {
        return NULL;
    }

    return list->data[index];
}

void freeEntryList(entryList *list)
{
    int cnt = 0;
    for (cnt = 0; cnt < list->count; cnt++)
    {
        free(list->data[cnt]);
    }
    free(list->data);
    list->data = NULL;
    list->size = 0;
    list->count = 0;
}

void addEntry(entryList *list, void *entry, uint32_t size)
{
    if (!list)
    {
        printf("ERROR: input entry list empty\n");
        return;
    }

    if (list->size == 0)
    {
        /*If no memory is yet allocated to given list,
         then initially allocate memory for 10 elements*/
        list->size = 10;
        list->data = (void **)malloc(sizeof(void*) * list->size);
        if (!list->data)
        {
            printf("ERROR: Memory allocation for entry data failed.\n");
            return;
        }
        memset(list->data, '\0', sizeof(void *) * list->size);
    }

    if (list->size == list->count)
    {
        /*If required memory for given list is more then the allocated one,
         then reallocate memory of more 10 elements for given list*/
        list->size += 10;
        list->data = (void **)realloc(list->data, sizeof(void*) * list->size);
        if (!list->data)
        {
            printf("ERROR: Memory allocation for entry data failed.\n");
            return;
        }
    }

    /*Add entry in given list*/
    void *data = malloc(size);
    if (!data)
    {
        printf("ERROR: Memory allocation for entry data failed.\n");
        return;
    }
    memcpy(data, entry, size);
    list->data[list->count] = data;
    list->count++;
}

void initConfigEntryList()
{
    /* init all data structures */
    initEntryList(&configFile.vlanData);
    initEntryList(&configFile.openFlowActionData);
    initEntryList(&configFile.macData);
    initEntryList(&configFile.ipv4HostData);
    initEntryList(&configFile.ipv6RouteData);
    initEntryList(&configFile.ipvxNhData);
    initEntryList(&configFile.ipv4RouteData);
    initEntryList(&configFile.tunnelData);
    initEntryList(&configFile.tunnelQinQData);
    initEntryList(&configFile.tunnelPBBData);
    initEntryList(&configFile.tunnelMplsData);
    initEntryList(&configFile.mplsLabelData);
    initEntryList(&configFile.bdData);
    initEntryList(&configFile.portConfigData);
    initEntryList(&configFile.portLagData);
    initEntryList(&configFile.controlMacData);
    initEntryList(&configFile.ipv4BridgeMcData);
    initEntryList(&configFile.ipv6BridgeMcData);
    initEntryList(&configFile.ipv4RouteMcData);
    initEntryList(&configFile.ipv6RouteMcData);
    initEntryList(&configFile.l2OIFEntry);
    initEntryList(&configFile.l3OIFEntry);
    initEntryList(&configFile.mvifData);
    initEntryList(&configFile.mirrorData);
    initEntryList(&configFile.iaclData);
    initEntryList(&configFile.iaclKey);
    initEntryList(&configFile.iaclMask);
    initEntryList(&configFile.srhData);
    initEntryList(&configFile.mySidData);
    initEntryList(&configFile.pbrData);
    initEntryList(&configFile.pbrKey);
    initEntryList(&configFile.pbrMask);
    initEntryList(&configFile.wcmKey);
    initEntryList(&configFile.wcmMask);
    initEntryList(&configFile.eaclData);
    initEntryList(&configFile.eaclMask);
    initEntryList(&configFile.eaclKey);
    initEntryList(&configFile.vlanTenantIdData);
    initEntryList(&configFile.vrfData);
    initEntryList(&configFile.ipv4SaiNeighborData);
    initEntryList(&configFile.ipv4SaiNhData);
    initEntryList(&configFile.ipv4SaiRouteData);
    initEntryList(&configFile.ipv6SaiNeighborData);
    initEntryList(&configFile.ipv6SaiNhData);
    initEntryList(&configFile.ipv6SaiRouteData);
    initEntryList(&configFile.ipSaiNhGroupData);
    initEntryList(&configFile.qosSchedulerData);
    initEntryList(&configFile.qosShaperPortData);
    initEntryList(&configFile.qosShaperQueueData);
    initEntryList(&configFile.qosAqmQProfileData);
    initEntryList(&configFile.vlanScopeData);
    initEntryList(&configFile.ipvxNhScopeData);
    initEntryList(&configFile.ipv4HostScopeData);
    initEntryList(&configFile.ipv4RouteScopeData);
    initEntryList(&configFile.openFlowData);
    initEntryList(&configFile.saiSchedulerData);
    initEntryList(&configFile.qosIngressMapData);
    initEntryList(&configFile.policerData);
    initEntryList(&configFile.macScopeData);
    initEntryList(&configFile.natData);
    initEntryList(&configFile.ipv6HostData);
    initEntryList(&configFile.trustBrData);
    initEntryList(&configFile.vifData);
}

void deInitConfigEntryList()
{
    int i = 0;
    /* deInit all data structures */
    freeEntryList(&configFile.vlanData);
    freeEntryList(&configFile.openFlowActionData);
    freeEntryList(&configFile.macData);
    freeEntryList(&configFile.ipv4HostData);
    for (i = 0; i < numOfEntry(&configFile.ipv6RouteData); i++)
    {
        Ipv6RouteEntry *ipv6RouteEntry = (Ipv6RouteEntry *)getEntry(
                                             &configFile.ipv6RouteData, i);
        freeEntryList(&(ipv6RouteEntry->nhId));
    }
    freeEntryList(&configFile.ipv6RouteData);
    freeEntryList(&configFile.ipvxNhData);
    for (i = 0; i < numOfEntry(&configFile.ipv4RouteData); i++)
    {
        Ipv4RouteEntry *ipv4RouteEntry = (Ipv4RouteEntry *)getEntry(
                                             &configFile.ipv4RouteData, i);
        freeEntryList(&(ipv4RouteEntry->nhId));
    }
    freeEntryList(&configFile.ipv4RouteData);
    freeEntryList(&configFile.tunnelData);
    freeEntryList(&configFile.tunnelQinQData);
    freeEntryList(&configFile.tunnelPBBData);
    freeEntryList(&configFile.tunnelMplsData);
    freeEntryList(&configFile.mplsLabelData);
    freeEntryList(&configFile.bdData);
    freeEntryList(&configFile.portConfigData);
    freeEntryList(&configFile.portLagData);
    freeEntryList(&configFile.srhData);
    freeEntryList(&configFile.mySidData);
    freeEntryList(&configFile.controlMacData);
    freeEntryList(&configFile.ipv4BridgeMcData);
    freeEntryList(&configFile.ipv6BridgeMcData);
    freeEntryList(&configFile.l2OIFEntry);
    freeEntryList(&configFile.l3OIFEntry);
    freeEntryList(&configFile.ipv4RouteMcData);
    freeEntryList(&configFile.ipv6RouteMcData);
    freeEntryList(&configFile.vlanTenantIdData);
    /*
        for (i = 0; i < numOfEntry(&configFile.l2OIFEntry); i++)
        {
            l2MdtEntry *mdtEntry = (l2MdtEntry *)getEntry(&configFile.l2OIFEntry, i);
            freeEntryList(&mdtEntry);
        }
            freeEntryList(&configFile.l2OIFEntry);

     TODO: Free the vectors
        for (j = 0; j < numOfEntry(&configFile.l3OIFData); j++)
        {
            l3MdtEntry *mdtEntry = (l3MdtEntry *)getEntry(&(l3OIFData->l3MdtTreeData), j);
            for (i = 0; i < numOfEntry(&(mdtEntry->l2MdtTreeData)); i++)
            {
                l2MdtEntry *l2MdtEntry = (l2MdtEntry *)getEntry(&(mdtEntry->l2MdtTreeData), i);
                freeEntryL));
            }
            freeEntryList(&configFile.l2OIFData);
            freeEntryList(&(L3OifEntry->l3MdtTreeData));
        }
        freeEntryList(&configFile.l3OIFData);*/

    freeEntryList(&configFile.mvifData);
    freeEntryList(&configFile.mirrorData);
    freeEntryList(&configFile.iaclData);
    freeEntryList(&configFile.iaclKey);
    freeEntryList(&configFile.iaclMask);
    freeEntryList(&configFile.pbrData);
    freeEntryList(&configFile.pbrKey);
    freeEntryList(&configFile.pbrMask);
    freeEntryList(&configFile.wcmKey);
    freeEntryList(&configFile.wcmMask);
    freeEntryList(&configFile.eaclData);
    freeEntryList(&configFile.eaclKey);
    freeEntryList(&configFile.eaclMask);
    freeEntryList(&configFile.vrfData);

    freeEntryList(&configFile.ipv4SaiNeighborData);
    freeEntryList(&configFile.ipv4SaiNhData);
    freeEntryList(&configFile.ipv4SaiRouteData);
    freeEntryList(&configFile.ipv6SaiNeighborData);
    freeEntryList(&configFile.ipv6SaiNhData);
    freeEntryList(&configFile.ipv6SaiRouteData);

    for (i = 0; i < numOfEntry(&configFile.ipSaiNhGroupData); i++)
    {
        IpvxSaiNhGroupEntry *ipvxSaiNhGroup = (IpvxSaiNhGroupEntry *)getEntry(
                                                  &configFile.ipSaiNhGroupData, i);
        freeEntryList(&(ipvxSaiNhGroup->nhIdList));
    }
    freeEntryList(&configFile.ipSaiNhGroupData);

    freeEntryList(&configFile.vlanScopeData);
    freeEntryList(&configFile.ipv4HostScopeData);
    freeEntryList(&configFile.ipvxNhScopeData);
    for (i = 0; i < numOfEntry(&configFile.ipv4RouteScopeData); i++)
    {
        Ipv4RouteScopeEntry *ipv4RouteScopeEntry = (Ipv4RouteScopeEntry *)getEntry(
                                                       &configFile.ipv4RouteScopeData, i);
        freeEntryList(&(ipv4RouteScopeEntry->nhId));
    }
    freeEntryList(&configFile.openFlowData);
    freeEntryList(&configFile.controlPcapList);
    freeEntryList(&configFile.qosSchedulerData);
    freeEntryList(&configFile.qosShaperPortData);
    freeEntryList(&configFile.qosShaperQueueData);
    freeEntryList(&configFile.qosAqmQProfileData);
    freeEntryList(&configFile.saiSchedulerData);
    freeEntryList(&configFile.qosIngressMapData);
    freeEntryList(&configFile.policerData);
    freeEntryList(&configFile.macScopeData);
    freeEntryList(&configFile.natData);
    freeEntryList(&configFile.ipv6HostData);
    freeEntryList(&configFile.ipv4RouteScopeData);
    freeEntryList(&configFile.ipv4PimBiDirData);
    freeEntryList(&configFile.trustBrData);
    freeEntryList(&configFile.vifData);
}

XP_STATUS setMirrorSourceType(MirrorSource *src, char* myType)
{
    int i = 0;
    for (i= 0; i < NUM_TYPES; ++i)
    {
        if (!strncasecmp(src->typeNames[i], myType, strlen(src->typeNames[i])))
        {
            src->type= i;
            return XP_NO_ERR;
        }
    }
    return XP_ERR_KEY_NOT_FOUND;
}

XP_STATUS setMirrorSourceId(MirrorSource *src, char* myValue)
{
    switch (src->type)
    {
        case TYPE_PORT:
            src->id.port= (xpPort_t) strtol(myValue, (char **) NULL, 10);
            break;
        case TYPE_VLAN:
            src->id.vlan= (xpVlan_t) strtol(myValue, (char **) NULL, 10);
            break;
        case TYPE_LAG:
            src->id.lag= (uint32_t) strtol(myValue, (char **) NULL, 10);
            break;
        default:
            return XP_ERR_KEY_NOT_FOUND;
            break;
    }
    if (errno == ERANGE)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }
    return XP_NO_ERR;
}

xpConfigFile configFile;


/**
 * \fn validatePortData
 * \private
 * \brief validate input data for port entry.
 *
 * \return int
*/
int validatePortData(char* buff, PortEntry *portData)
{

    int port, numOfExtPorts, tagType, stpState, errorCode;
    int setIngressVif, disableTunnelVif, mirrorToAnalyzerMask, setBd, enPbb,
        natMode, natScope, enRouterACL, routeAclId, enBridgeACL, bridgeAclId,
        enIpv4Tunnel, enMplsTunnel, enOpenFlow, setEgressVif, evif, encapType;

    uint32_t enVlanHairPining;
    char *temp = NULL;

    if (!buff || !portData)
    {
        printf("Null pointer \n");
        return 0;
    }

    memset(portData, 0, sizeof(*portData));


    temp = strstr(buff, "->");
    if (temp == NULL)
    {
        printf("portAttrib format not valid \n");
        return 0;
    }

    errorCode = sscanf(buff,
                       "%d->%d,%d,%d,%d,%d,%d,%u,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &port,
                       &tagType, &stpState,
                       &setIngressVif, &disableTunnelVif, &mirrorToAnalyzerMask, &setBd,
                       &enVlanHairPining, &enPbb, &natMode,
                       &natScope, &enRouterACL, &routeAclId, &enBridgeACL, &bridgeAclId, &enIpv4Tunnel,
                       &enMplsTunnel, &numOfExtPorts,
                       &enOpenFlow, &setEgressVif, &evif, &encapType);

    /* 23 Attribute + 1 port = 24*/
    if (errorCode != 22)
    {
        printf("portAttrib %s should be of 23 field >>%d\n", buff, errorCode);
        return 0;
    }

    if (port < 0 || port > 136)
    {
        printf("Port number ( %d ) out of range error for portAttrib %s \n", port,
               buff);
        return 0;
    }

    if (tagType < 0 || tagType > 15)
    {
        printf("Tag Type should be between 0 and 15 for portAttrib %s\n", buff);
        return 0;
    }

    if (stpState < 0 || stpState > 3)
    {
        printf("STP state should be between 0 to 3 for portAttrib %s \n", buff);
        return 0;
    }

    if (setIngressVif < 0 || setIngressVif > 1)
    {
        printf("setIngressVif should be 0 or 1 %s for portAttrib\n", buff);
        return 0;
    }

    if (mirrorToAnalyzerMask < 0 || mirrorToAnalyzerMask > 3)
    {
        printf("mirrorToAnalyzerMask should be between 0 to 7  for portAttrib %s\n",
               buff);
        return 0;
    }

    if (disableTunnelVif < 0 || disableTunnelVif > 1)
    {
        printf("disableTunnelVif should be 0 or 1 for portAttrib %s \n", buff);
        return 0;
    }

    if (setBd < 0 || setBd > 1)
    {
        printf("setBd should be 0 or 1 for portAttrib %s \n", buff);
        return 0;
    }
    if (enVlanHairPining > 1)
    {
        printf("enVlanHairPining should be 0 or 1for portAttrib %s \n", buff);
        return 0;
    }

    if (enPbb < 0 || enPbb > 1)
    {
        printf("enPbb should be 0 or 1 for portAttrib %s \n", buff);
        return 0;
    }

    if (natMode < 0 || natMode > 3)
    {
        printf("natMode should be 0 or 1 for portAttrib %s \n", buff);
        return 0;
    }

    if (natScope < 0 ||
        natScope > 2) //TODO for others values may need to be updated
    {
        printf("natScope should be 0 or 1 for portAttrib %s \n", buff);
        return 0;
    }

    if (enRouterACL < 0 || enRouterACL > 1)
    {
        printf("enRouterACL should be 0 or 1 for portAttrib %s \n", buff);
        return 0;
    }

    if (enBridgeACL < 0 || enBridgeACL > 1)
    {
        printf("enBridgeACL should be 0 or 1 for portAttrib %s \n", buff);
        return 0;
    }

    if (enIpv4Tunnel < 0 || enIpv4Tunnel > 1)
    {
        printf("enIpv4Tunnel should be 0 or 1 for portAttrib %s \n", buff);
        return 0;
    }

    if (enMplsTunnel < 0 || enMplsTunnel > 1)
    {
        printf("enMplsTunnel should be 0 or 1 for portAttrib %s \n", buff);
        return 0;
    }

    if (enOpenFlow < 0 || enOpenFlow > 1)
    {
        printf("enOpenFlow should be 0 or 1 for portAttrib %s \n", buff);
        return 0;
    }

    if (numOfExtPorts < 0 || numOfExtPorts > 1024)
    {
        printf("numOfExtPorts should be between 0 to 1024 for portAttrib %s \n", buff);
        return 0;
    }

    if (setEgressVif < 0 || setEgressVif > 1)
    {
        printf("setEgressVif should be 0 or 1 for portAttrib %s\n", buff);
        return 0;
    }

    if (evif < 0 || evif > 136)
    {
        printf("evif ( %d ) out of range error for portAttrib %s \n", evif, buff);
        return 0;
    }

    if (encapType < 0 || encapType > 4)
    {
        printf("Encap Type should be between 0 and 4 for portAttrib %s\n", buff);
        return 0;
    }

    portData->port = port;
    portData->tagType = tagType;
    portData->stpState = stpState;
    portData->setIngressVif = setIngressVif;
    portData->disableTunnelVif = disableTunnelVif;
    portData->mirrorToAnalyzerMask = mirrorToAnalyzerMask;
    portData->setBd = setBd;
    portData->enVlanHairPining = enVlanHairPining;
    portData->enPbb = enPbb;
    portData->natMode = natMode;
    portData->natScope = natScope;
    portData->enRouterACL = enRouterACL;
    portData->routeAclId = routeAclId;
    portData->enBridgeACL = enBridgeACL;
    portData->bridgeAclId = bridgeAclId;
    portData->enIpv4Tunnel = enIpv4Tunnel;
    portData->enMplsTunnel = enMplsTunnel;
    portData->enOpenFlow = enOpenFlow;
    portData->numOfExtPorts = numOfExtPorts;
    portData->setEgressVif = setEgressVif;
    portData->evif = evif;
    portData->encapType = encapType;

    return 1;
}


/**
 * \fn processVlanData
 * \private
 * \brief Parses line of text from configuration file to determine VlanEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processVlanData(int lineNum, char *line)
{
    int errorCode;
    int vlan = 0, macSAmissCmd = 0, broadcastCmd = 0,
        ipv4ARPBCCmd = 0, ipv4RouteEn = 0, ipv6RouteEn = 0, vrfId = 0,
        ipv4BridgeMcMode = 0, ipv6BridgeMcMode = 0, ipv4RouteMcEn = 0,
        ipv6RouteMcEn = 0,
        ipv4RouteMcMode = 0, ipv6RouteMcMode = 0, unregMcCmd = 0, mplsRouteEn = 0,
        vlanType = 0,
        parentVlan = 0, enL3Inf=0, macSALSBs=0, natEn = 0, natMode = 0, natScope = 0,
        ipv4UrpfEn = 0, ipv4UrpfMode = 0, ipv6UrpfEn = 0, ipv6UrpfMode = 0,
        ipv4MtuLen = 0, ipv6MtuLen = 0, mtuPktCmd = 0,
        mcVifIdx = 0, mcastIdx = 0;
    VlanEntry* newData = NULL;
    char mvif[128];
    char* mvifStrList = NULL;
    PortEntry portData;
    int entryLen = 0;
    char entryformat[] = "{:,[->]}";
    char *str1 = NULL;
    char *str2 = NULL;
    char buffer[256];
    int i=0;

    memset(buffer, 0, 256);

    int len = strlen(entryformat);
    for (i=0; i < len; i++)
    {
        str1 = strchr(line, (int)entryformat[i]);
        if (str1 == NULL)
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("enter vlan data in valid format \n");
            return;
        }
    }
    str1 = strstr(line, "{");
    str2 = strstr(line, "portAttrib:");
    if (str1 == NULL || str2 == NULL)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter vlan data in valid format\n");
        return;
    }

    str1 = str1 + 1;
    entryLen = str2 - str1;
    if (entryLen > 82 || entryLen <= 0)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("entry size is not valid\n");
        return;
    }
    strncpy(buffer,  str1, entryLen);

    errorCode = sscanf(buffer,
                       "\t%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,[%127s ],%d,%d",
                       &vlan,
                       &macSAmissCmd, &broadcastCmd, &ipv4ARPBCCmd, &ipv4RouteEn,
                       &ipv6RouteEn, &vrfId, &ipv4BridgeMcMode, &ipv6BridgeMcMode,
                       &ipv4RouteMcEn, &ipv6RouteMcEn, &ipv4RouteMcMode, &ipv6RouteMcMode, &unregMcCmd,
                       &mplsRouteEn, &vlanType,
                       &parentVlan, &enL3Inf, &macSALSBs, &natEn, &natMode, &natScope, &ipv4UrpfEn,
                       &ipv4UrpfMode, &ipv6UrpfEn,
                       &ipv6UrpfMode, &ipv4MtuLen, &ipv6MtuLen, &mtuPktCmd, mvif, &mcVifIdx,
                       &mcastIdx);
    if (errorCode < 32)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    if (vlan < 0 || vlan > 4096)
    {
        printf(", ERROR:Vlan ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               vlan, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (macSAmissCmd < 0 || macSAmissCmd > 3)
    {
        printf(", ERROR:macSAmissCmd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               macSAmissCmd, lineNum, configFile.filename);
        printf("Assuming macSAmissCmd is 1...\n");
        macSAmissCmd = 1;
    }

    if (broadcastCmd < 0 || broadcastCmd > 3)
    {
        printf(", ERROR:broadcastCmd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               broadcastCmd, lineNum, configFile.filename);
        printf("Assuming broadcastCmd is 3...");
        broadcastCmd = 3;
    }

    if (ipv4ARPBCCmd < 0 || ipv4ARPBCCmd > 3)
    {
        printf(", ERROR:ipv4ARPBCCmd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv4ARPBCCmd, lineNum, configFile.filename);
        printf("Assuming ipv4ARPBCCmd is 1...\n");
        ipv4ARPBCCmd = 1;
    }

    if (ipv4BridgeMcMode < 0 || ipv4BridgeMcMode > 3)
    {
        printf(", ERROR:ipv4BridgeMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv4BridgeMcMode, lineNum, configFile.filename);
        printf("Assuming ipv4BridgeMcMode is 0...\n");
        ipv4BridgeMcMode = 0;
    }

    if (ipv6BridgeMcMode < 0 || ipv6BridgeMcMode > 3)
    {
        printf(", ERROR:ipv6BridgeMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv6BridgeMcMode, lineNum, configFile.filename);
        printf("Assuming ipv6BridgeMcMode is 0...\n");
        ipv6BridgeMcMode = 0;
    }

    if (ipv4RouteMcEn < 0 || ipv4RouteMcEn > 1)
    {
        printf(", ERROR:ipv4RouteMcEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv4RouteMcMode, lineNum, configFile.filename);
        printf("Assuming ipv4RouteMcEn is 0...\n");
        ipv4RouteMcEn = 0;
    }

    if (ipv6RouteMcEn < 0 || ipv6RouteMcEn > 1)
    {
        printf(", ERROR:ipv6RouteMcEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv6RouteMcEn, lineNum, configFile.filename);
        printf("Assuming ipv6RouteMcEn is 0...\n");
        ipv6RouteMcEn = 0;
    }

    if (ipv4RouteMcMode < 0 || ipv4RouteMcMode > 1)
    {
        printf("ipv4RouteMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv4RouteMcMode, lineNum, configFile.filename);
        printf("Assuming ipv4RouteMcMode is 0...\n");
        ipv4RouteMcMode = 0;
    }

    if (ipv6RouteMcMode < 0 || ipv6RouteMcMode > 1)
    {
        printf("ipv6RouteMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv6RouteMcMode, lineNum, configFile.filename);
        printf("Assuming ipv6RouteMcMode is 0...\n");
        ipv6RouteMcMode = 0;
    }

    if (mplsRouteEn < 0 || mplsRouteEn > 1)
    {
        printf("mplsRouteEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               mplsRouteEn, lineNum, configFile.filename);
        printf("Assuming mplsRouteEn is 0...\n");
        mplsRouteEn = 0;
    }

    if (unregMcCmd < 0 || unregMcCmd > 1)
    {
        printf("unregMcCmd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               unregMcCmd, lineNum, configFile.filename);
        printf("Assuming unregMcCmd is 0...\n");
        unregMcCmd = 0;
    }

    if (vlanType < 0 || vlanType > 3)
    {
        printf("vlanType ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               vlanType, lineNum, configFile.filename);
        printf("Assuming vlanType is 0...\n");
        vlanType = 0;
    }

    if (parentVlan < 0 || parentVlan > 4096)
    {
        printf("parentVlan ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               parentVlan, lineNum, configFile.filename);
        printf("Assuming parentVlan is 0...\n");
        parentVlan = 0;
    }

    if (enL3Inf < 0 || enL3Inf > 1)
    {
        printf("enL3Inf ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               enL3Inf, lineNum, configFile.filename);
        printf("Assuming enL3Inf is 0...\n");
        enL3Inf = 0;
    }

    if (natEn < 0 || natEn > 1)
    {
        printf("natEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               enL3Inf, lineNum, configFile.filename);
        printf("Assuming natEn is 0...\n");
        natEn = 0;
    }
    if (macSALSBs < 0 || macSALSBs > 255)
    {
        printf("macSALSBs ( 0x%x ) out of range error on line # %d in configuration file \" %s \".\n",
               macSALSBs, lineNum, configFile.filename);
        printf("Assuming macSALSBs is 0x00...\n");
        macSALSBs = 0;
    }

    if (natMode < 0 || natMode > 255)
    {
        printf("natMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               natMode, lineNum, configFile.filename);
        printf("Assuming natMode is 0...\n");
        natMode = 0;
    }

    if (natScope < 0 || natScope > 255)
    {
        printf("natScope ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               natScope, lineNum, configFile.filename);
        printf("Assuming natScope is 0...\n");
        natScope = 0;
    }

    if (ipv4UrpfEn < 0 || ipv4UrpfEn > 1)
    {
        printf("ipv4UrpfEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv4UrpfEn, lineNum, configFile.filename);
        printf("Assuming natScope is 0...\n");
        ipv4UrpfEn = 0;
    }

    if (ipv4UrpfMode < 0 || ipv4UrpfMode > 1)
    {
        printf("ipv4UrpfMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv4UrpfMode, lineNum, configFile.filename);
        printf("Assuming natScope is 0...\n");
        ipv4UrpfMode = 0;
    }
    if (ipv6UrpfEn < 0 || ipv6UrpfEn > 1)
    {
        printf("ipv6UrpfEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv6UrpfEn, lineNum, configFile.filename);
        printf("Assuming natScope is 0...\n");
        ipv6UrpfEn = 0;
    }

    if (ipv6UrpfMode < 0 || ipv6UrpfMode > 1)
    {
        printf("ipv6UrpfMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv6UrpfMode, lineNum, configFile.filename);
        printf("Assuming natScope is 0...\n");
        ipv6UrpfMode = 0;
    }

    newData = (VlanEntry*)malloc(sizeof(VlanEntry));
    if (!newData)
    {
        printf("Not enough memoryto allocate VlanEntry object\n");
        return;
    }

    memset(newData, 0, sizeof(VlanEntry));
    if (ipv4MtuLen < 64)
    {
        printf(", ERROR:ipv4MtuLen ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv4MtuLen, lineNum, configFile.filename);
        printf("Assuming ipv4MtuLen is 1500...\n");
        ipv4MtuLen = 1500;
    }

    if (ipv6MtuLen < 64)
    {
        printf(", ERROR:ipv6MtuLen ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv6MtuLen, lineNum, configFile.filename);
        printf("Assuming ipv6MtuLen is 1500...\n");
        ipv6MtuLen = 1500;
    }

    if (mtuPktCmd != 0 && mtuPktCmd != 2)
    {
        printf(", ERROR:mtuPktCmd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               mtuPktCmd, lineNum, configFile.filename);
        printf("Assuming mtuPktCmd is 2...\n");
        mtuPktCmd = 2;
    }

    //Get mvif list
    memset(newData->mvifIdx, 0, sizeof(newData->mvifIdx));
    mvifStrList = strtok(mvif, ",");
    i = 0;
    while (mvifStrList)
    {
        newData->mvifIdx[i++] = atoi(mvifStrList);
        mvifStrList = strtok(NULL, ",");
    }

    if (mcVifIdx < -1)
    {
        printf("mvif should be -1 or positive on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        free(newData);
        return;
    }

    if (mcastIdx < 0)
    {
        printf("mcastIdx should be positive on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        free(newData);
        return;
    }

    newData->vlan = vlan;
    newData->enL3Inf = enL3Inf;
    newData->macSALSBs = macSALSBs;
    newData->macSAmissCmd = macSAmissCmd;
    newData->broadcastCmd = broadcastCmd;
    newData->ipv4ARPBCCmd = ipv4ARPBCCmd;
    newData->ipv4RouteEn = ipv4RouteEn;;
    newData->ipv6RouteEn = ipv6RouteEn;
    newData->mplsRouteEn = mplsRouteEn;
    newData->ipv4BridgeMcMode = ipv4BridgeMcMode;
    newData->ipv6BridgeMcMode = ipv6BridgeMcMode;
    newData->ipv4RouteMcEn = ipv4RouteMcEn;
    newData->ipv6RouteMcEn = ipv6RouteMcEn;
    newData->ipv4RouteMcMode = ipv4RouteMcMode;
    newData->ipv6RouteMcMode = ipv6RouteMcMode;
    newData->unregMcCmd = unregMcCmd;
    newData->vlanType = vlanType;
    newData->parentVlan = parentVlan;
    newData->vrfId = vrfId;
    newData->natEn = natEn;
    newData->natMode = natMode;
    newData->natScope = natScope;
    newData->ipv4UrpfEn = ipv4UrpfEn;
    newData->ipv4UrpfMode = ipv4UrpfMode;
    newData->ipv6UrpfEn = ipv6UrpfEn;
    newData->ipv6UrpfMode = ipv6UrpfMode;
    newData->ipv4MtuLen = ipv4MtuLen;
    newData->ipv6MtuLen = ipv6MtuLen;
    newData->mtuPktCmd = mtuPktCmd;
    newData->mcVifIdx = mcVifIdx;
    newData->mcastIdx = mcastIdx;

    str1 = str2 + 12;
    str2 = strstr(str1, "|");
    newData->numPorts = 0;
    while (str2!= NULL)
    {
        entryLen = str2 - str1;
        if (entryLen > 55 || entryLen <=0)
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("entry size is not valid %s > %d\n", str2, entryLen);
            free(newData);
            return;
        }
        memset(buffer, '\0', sizeof(buffer));
        strncpy(buffer,  str1, entryLen);
        if (validatePortData(buffer, &portData) > 0)
        {
            if (newData->numPorts < XP_MAX_TOTAL_PORTS)
            {
                newData->portData[newData->numPorts++] = portData;
                memset(buffer, '\0', sizeof(buffer));
            }
        }
        else
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            free(newData);
            return;
        }
        str1 = str2 + 1;
        str2 = strstr(str1, "|");
    }

    str2 = strstr(str1, "]");
    if (str2 != NULL)
    {
        entryLen = str2 - str1;
        if (entryLen > 55 || entryLen <= 0)
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("entry size is not valid >>%d\n", entryLen);
            free(newData);
            return;
        }
        memset(buffer, '\0', sizeof(buffer));
        strncpy(buffer,  str1, entryLen);
        if (validatePortData(buffer, &portData) > 0)
        {
            if (newData->numPorts < XP_MAX_TOTAL_PORTS)
            {
                newData->portData[newData->numPorts++] = portData;
                memset(buffer, '\0', sizeof(buffer));
            }
        }
        else
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            free(newData);
            return;
        }
    }
    else
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter vlan data in valid format\n");
        free(newData);
        return;
    }

    addEntry(&configFile.vlanData, newData, sizeof(VlanEntry));
    free(newData);
}

/**
 * \fn processVlanScopeData
 * \private
 * \brief Parses line of text from configuration file to determine VlanEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processVlanScopeData(int lineNum, char *line)
{
    int errorCode;
    int vlan = 0,  macSAmode = 0, macSAmissCmd = 0, broadcastCmd = 0,
        ipv4ARPBCCmd = 0, ipv4RouteEn = 0, ipv6RouteEn = 0, vrfId = 0,
        ipv4BridgeMcMode = 0, ipv6BridgeMcMode = 0, ipv4RouteMcEn = 0,
        ipv6RouteMcEn = 0,
        ipv4RouteMcMode = 0, ipv6RouteMcMode = 0, unregMcCmd = 0, mplsRouteEn = 0,
        vlanType = 0,
        parentVlan = 0, enL3Inf=0, natEn = 0, natMode = 0, natScope = 0,
        ipv4UrpfEn = 0, ipv4UrpfMode = 0, ipv6UrpfEn = 0, ipv6UrpfMode = 0,
        ipv4MtuLen = 0, ipv6MtuLen = 0, mtuPktCmd = 0, scopeId = 0;
    VlanScopeEntry* newData = NULL;
    unsigned int macSALSBs = 0;
    PortEntry portData;
    int entryLen = 0;
    char entryformat[] = "{:,[->]}";
    char *str1 = NULL;
    char *str2 = NULL;
    char buffer[256];
    int i=0;

    memset(buffer, 0, 60);

    int len = strlen(entryformat);
    for (i=0; i < len; i++)
    {
        str1 = strchr(line, (int)entryformat[i]);
        if (str1 == NULL)
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("enter vlan data in valid format\n");
            return;
        }
    }
    str1 = strstr(line, "{");
    str2 = strstr(line, "portAttrib:");
    if (str1 != NULL && str2 != NULL)
    {
        str1 = str1 + 1;
        entryLen = str2 - str1;
        if (entryLen > 80 || entryLen <= 0)
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("entry size is not valid\n");
            return;
        }
        strncpy(buffer,  str1, entryLen);

        errorCode = sscanf(buffer,
                           "\t%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%x,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                           &vlan, &macSAmode, &macSAmissCmd, &broadcastCmd, &ipv4ARPBCCmd, &ipv4RouteEn,
                           &ipv6RouteEn, &vrfId, &ipv4BridgeMcMode, &ipv6BridgeMcMode,
                           &ipv4RouteMcEn, &ipv6RouteMcEn, &ipv4RouteMcMode, &ipv6RouteMcMode, &unregMcCmd,
                           &mplsRouteEn, &vlanType,
                           &parentVlan, &enL3Inf, &macSALSBs, &natEn, &natMode, &natScope, &ipv4UrpfEn,
                           &ipv4UrpfMode,
                           &ipv6UrpfEn, &ipv6UrpfMode, &ipv4MtuLen, &ipv6MtuLen, &mtuPktCmd, &scopeId);
        if (errorCode < 31)
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            return;
        }

        if (vlan < 0 || vlan > 4096)
        {
            printf(", ERROR:Vlan ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   vlan, lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }

        if (macSAmode < 0 || macSAmode > 1)
        {
            printf(", ERROR:macSAmode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   macSAmode, lineNum, configFile.filename);
            printf("Assuming macSAmode is 0...\n");
            macSAmode = 0;
        }

        if (macSAmissCmd < 0 || macSAmissCmd > 3)
        {
            printf(", ERROR:macSAmissCmd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   macSAmissCmd, lineNum, configFile.filename);
            printf("Assuming macSAmissCmd is 1...\n");
            macSAmissCmd = 1;
        }

        if (broadcastCmd < 0 || broadcastCmd > 3)
        {
            printf(", ERROR:broadcastCmd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   broadcastCmd, lineNum, configFile.filename);
            printf("Assuming broadcastCmd is 3...");
            broadcastCmd = 3;
        }

        if (ipv4ARPBCCmd < 0 || ipv4ARPBCCmd > 3)
        {
            printf(", ERROR:ipv4ARPBCCmd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4ARPBCCmd, lineNum, configFile.filename);
            printf("Assuming ipv4ARPBCCmd is 1...\n");
            ipv4ARPBCCmd = 1;
        }

        if (ipv4BridgeMcMode < 0 || ipv4BridgeMcMode > 3)
        {
            printf(", ERROR:ipv4BridgeMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4BridgeMcMode, lineNum, configFile.filename);
            printf("Assuming ipv4BridgeMcMode is 0...\n");
            ipv4BridgeMcMode = 0;
        }

        if (ipv6BridgeMcMode < 0 || ipv6BridgeMcMode > 3)
        {
            printf(", ERROR:ipv6BridgeMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv6BridgeMcMode, lineNum, configFile.filename);
            printf("Assuming ipv6BridgeMcMode is 0...\n");
            ipv6BridgeMcMode = 0;
        }

        if (ipv4RouteMcEn < 0 || ipv4RouteMcEn > 1)
        {
            printf(", ERROR:ipv4RouteMcEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4RouteMcMode, lineNum, configFile.filename);
            printf("Assuming ipv4RouteMcEn is 0...\n");
            ipv4RouteMcEn = 0;
        }

        if (ipv6RouteMcEn < 0 || ipv6RouteMcEn > 1)
        {
            printf(", ERROR:ipv6RouteMcEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv6RouteMcEn, lineNum, configFile.filename);
            printf("Assuming ipv6RouteMcEn is 0...\n");
            ipv6RouteMcEn = 0;
        }

        if (ipv4RouteMcMode < 0 || ipv4RouteMcMode > 1)
        {
            printf("ipv4RouteMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4RouteMcMode, lineNum, configFile.filename);
            printf("Assuming ipv4RouteMcMode is 0...\n");
            ipv4RouteMcMode = 0;
        }

        if (ipv6RouteMcMode < 0 || ipv6RouteMcMode > 1)
        {
            printf("ipv6RouteMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv6RouteMcMode, lineNum, configFile.filename);
            printf("Assuming ipv6RouteMcMode is 0...\n");
            ipv6RouteMcMode = 0;
        }

        if (mplsRouteEn < 0 || mplsRouteEn > 1)
        {
            printf("mplsRouteEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   mplsRouteEn, lineNum, configFile.filename);
            printf("Assuming mplsRouteEn is 0...\n");
            mplsRouteEn = 0;
        }

        if (unregMcCmd < 0 || unregMcCmd > 1)
        {
            printf("unregMcCmd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   unregMcCmd, lineNum, configFile.filename);
            printf("Assuming unregMcCmd is 0...\n");
            unregMcCmd = 0;
        }

        if (vlanType < 0 || vlanType > 3)
        {
            printf("vlanType ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   vlanType, lineNum, configFile.filename);
            printf("Assuming vlanType is 0...\n");
            vlanType = 0;
        }

        if (parentVlan < 0 || parentVlan > 4096)
        {
            printf("parentVlan ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   parentVlan, lineNum, configFile.filename);
            printf("Assuming parentVlan is 0...\n");
            parentVlan = 0;
        }

        if (enL3Inf < 0 || enL3Inf > 1)
        {
            printf("enL3Inf ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   enL3Inf, lineNum, configFile.filename);
            printf("Assuming enL3Inf is 0...\n");
            enL3Inf = 0;
        }

        if (natEn < 0 || natEn > 1)
        {
            printf("natEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   enL3Inf, lineNum, configFile.filename);
            printf("Assuming natEn is 0...\n");
            natEn = 0;
        }
        if (macSALSBs > 255)
        {
            printf("macSALSBs ( 0x%x ) out of range error on line # %d in configuration file \" %s \".\n",
                   macSALSBs, lineNum, configFile.filename);
            printf("Assuming macSALSBs is 0x00...\n");
            macSALSBs = 0;
        }

        if (natMode < 0 || natMode > 255)
        {
            printf("natMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   natMode, lineNum, configFile.filename);
            printf("Assuming natMode is 0...\n");
            natMode = 0;
        }

        if (natScope < 0 || natScope > 255)
        {
            printf("natScope ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   natScope, lineNum, configFile.filename);
            printf("Assuming natScope is 0...\n");
            natScope = 0;
        }

        if (ipv4UrpfEn < 0 || ipv4UrpfEn > 1)
        {
            printf("ipv4UrpfEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4UrpfEn, lineNum, configFile.filename);
            printf("Assuming natScope is 0...\n");
            ipv4UrpfEn = 0;
        }

        if (ipv4UrpfMode < 0 || ipv4UrpfMode > 1)
        {
            printf("ipv4UrpfMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4UrpfMode, lineNum, configFile.filename);
            printf("Assuming natScope is 0...\n");
            ipv4UrpfMode = 0;
        }
        if (ipv6UrpfEn < 0 || ipv6UrpfEn > 1)
        {
            printf("ipv6UrpfEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv6UrpfEn, lineNum, configFile.filename);
            printf("Assuming natScope is 0...\n");
            ipv6UrpfEn = 0;
        }

        if (ipv6UrpfMode < 0 || ipv6UrpfMode > 1)
        {
            printf("ipv6UrpfMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv6UrpfMode, lineNum, configFile.filename);
            printf("Assuming natScope is 0...\n");
            ipv6UrpfMode = 0;
        }

        newData = (VlanScopeEntry*)malloc(sizeof(VlanScopeEntry));
        if (!newData)
        {
            printf("Not enough memory to allocate VlanScopeEntry object\n");
            return;
        }
        memset(newData, 0, sizeof(VlanScopeEntry));

        if (ipv4MtuLen < 64)
        {
            printf(", ERROR:ipv4MtuLen ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4MtuLen, lineNum, configFile.filename);
            printf("Assuming ipv4MtuLen is 1500...\n");
            ipv4MtuLen = 1500;
        }

        if (ipv6MtuLen < 64)
        {
            printf(", ERROR:ipv6MtuLen ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv6MtuLen, lineNum, configFile.filename);
            printf("Assuming ipv6MtuLen is 1500...\n");
            ipv6MtuLen = 1500;
        }

        if (mtuPktCmd != 0 && mtuPktCmd != 2)
        {
            printf(", ERROR:mtuPktCmd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   mtuPktCmd, lineNum, configFile.filename);
            printf("Assuming mtuPktCmd is 2...\n");
            mtuPktCmd = 2;
        }

        newData->vlan = vlan;
        newData->enL3Inf = enL3Inf;
        newData->macSALSBs = macSALSBs;
        newData->macSAmissCmd = macSAmissCmd;
        newData->broadcastCmd = broadcastCmd;
        newData->ipv4ARPBCCmd = ipv4ARPBCCmd;
        newData->ipv4RouteEn = ipv4RouteEn;;
        newData->ipv6RouteEn = ipv6RouteEn;
        newData->mplsRouteEn = mplsRouteEn;
        newData->ipv4BridgeMcMode = ipv4BridgeMcMode;
        newData->ipv6BridgeMcMode = ipv6BridgeMcMode;
        newData->ipv4RouteMcEn = ipv4RouteMcEn;
        newData->ipv6RouteMcEn = ipv6RouteMcEn;
        newData->ipv4RouteMcMode = ipv4RouteMcMode;
        newData->ipv6RouteMcMode = ipv6RouteMcMode;
        newData->unregMcCmd = unregMcCmd;
        newData->vlanType = vlanType;
        newData->parentVlan = parentVlan;
        newData->vrfId = vrfId;
        newData->natEn = natEn;
        newData->natMode = natMode;
        newData->natScope = natScope;
        newData->ipv4UrpfEn = ipv4UrpfEn;
        newData->ipv4UrpfMode = ipv4UrpfMode;
        newData->ipv6UrpfEn = ipv6UrpfEn;
        newData->ipv6UrpfMode = ipv6UrpfMode;
        newData->ipv4MtuLen = ipv4MtuLen;
        newData->ipv6MtuLen = ipv6MtuLen;
        newData->mtuPktCmd = mtuPktCmd;
        newData->scopeId = scopeId;
        memset(buffer, 0, 50);
    }
    else
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter vlan data in valid format\n");
        return;
    }

    str1 = str2 + 12;
    str2 = strstr(str1, "|");
    newData->numPorts = 0;
    while (str2!= NULL)
    {
        entryLen = str2 - str1;
        if (entryLen > 55 || entryLen <=0)
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("entry size is not valid %s\n", str2);

            free(newData);
            return;
        }
        memset(buffer, '\0', sizeof(buffer));
        strncpy(buffer,  str1, entryLen);
        if (validatePortData(buffer, &portData) > 0)
        {
            if (newData->numPorts < XP_MAX_TOTAL_PORTS)
            {
                newData->portData[newData->numPorts++] = portData;
                memset(buffer, '\0', sizeof(buffer));
            }
        }
        else
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            free(newData);
            return;
        }
        str1 = str2 + 1;
        str2 = strstr(str1, "|");
    }
    str2 = strstr(str1, "]");
    if (str2 != NULL)
    {
        entryLen = str2 - str1;
        if (entryLen > 55 || entryLen <= 0)
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("entry size is not valid\n");
            free(newData);
            return;
        }
        memset(buffer, '\0', sizeof(buffer));
        strncpy(buffer,  str1, entryLen);
        if (validatePortData(buffer, &portData) > 0)
        {
            if (newData->numPorts < XP_MAX_TOTAL_PORTS)
            {
                newData->portData[newData->numPorts++] = portData;
                memset(buffer, '\0', sizeof(buffer));
            }
        }
        else
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            free(newData);
            return;
        }
    }
    else
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter vlan data in valid format\n");
        free(newData);
        return;
    }

    addEntry(&configFile.vlanScopeData, newData, sizeof(VlanScopeEntry));
    free(newData);
}

/**
 * \fn processPortLagData
 * \private
 * \brief Parses line of text from configuration file to determine PortLagEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processPortLagData(int lineNum, char *line)
{
    int lagId;
    int vlan;
    int tagType;
    int stpState;
    xpPort_t validPorts[XP_MAX_TOTAL_PORTS] = {0};
    int port[5];
    int errorCode;
    int i = 0;
    PortLagEntry newData;

    errorCode= sscanf(line, "%d %d %d %d %d, %d, %d, %d, %d", &lagId, &vlan,
                      &tagType,
                      &stpState, &port[0], &port[1], &port[2], &port[3], &port[4]);
    if (errorCode < 3)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    if (lagId < 1 || lagId > 4096)
    {
        printf("lagId ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               lagId, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (vlan < 0 || vlan > 4095)
    {
        printf("vlanId ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               vlan, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (tagType < 0 || tagType > 2)
    {
        printf("tagType( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               tagType, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (stpState < 0 || stpState > 3)
    {
        printf("stpState( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               stpState, lineNum, configFile.filename);
        printf("Assuming STP state is 0...\n");
        stpState = 0;
    }
    newData.numPorts = 0;
    for (i=0; i< errorCode-4; i++)
    {
        if (port[i] > 0 && port[i] < 4096)
        {
            if (newData.numPorts < XP_MAX_TOTAL_PORTS)
            {
                validPorts[newData.numPorts++] = port[i];
            }
        }
    }
    newData.vlan = vlan;
    newData.lagId = lagId;
    newData.tagType = tagType;
    newData.stpState = stpState;
    memcpy(newData.ports, validPorts, sizeof(uint32_t) * newData.numPorts);
    addEntry(&configFile.portLagData, &newData, sizeof(PortLagEntry));
}

/**
 * \fn processMacData
 * \private
 * \brief Parses line of text from configuration file to determine MacEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processMacData(int lineNum, char *line)
{
    int port, vlan, controlMac, routerMac, pktCmd, tnlIdx, encapType;
    unsigned int macAddr[XP_MAC_ADDR_LEN];
    int errorCode, geneveTunnelType, isLag, lagId, extendedPortIdx, isMcast,
        vifMcast, mcastIdx, agingFlag;
    int i = 0;
    char mvif[128];
    char* mvifStrList = NULL;

    MacEntry newData;

    errorCode= sscanf(line,
                      "%x : %x : %x : %x : %x : %x %d %d %d %d %d %d %d %d %d %d %d %d %127s %d %d %d",
                      &macAddr[5],
                      &macAddr[4], &macAddr[3], &macAddr[2], &macAddr[1], &macAddr[0], &vlan, &port,
                      &controlMac, &routerMac,
                      &pktCmd, &tnlIdx, &encapType, &geneveTunnelType, &isLag, &lagId,
                      &extendedPortIdx, &isMcast, mvif, &vifMcast, &mcastIdx, &agingFlag);
    if (errorCode != XP_MAC_ADDR_LEN + 16)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    if (port < 0 || port > 136)
    {
        printf("Port number ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               port, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (vlan < 0 || vlan > 4095)
    {
        printf("VlanId ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               vlan, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
    {
        if (macAddr[i] > 255)
        {
            printf("MAC address value ( 0x%x ) out of range error on line # %d in configuration file \" %s \".\n",
                   macAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.macAddr[i]= macAddr[i];
        }
    }

    if (controlMac < 0 || controlMac > 1)
    {
        printf("controlMac should be 0 or 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming controlMac is 0...\n");
        controlMac = 0;
    }

    if (routerMac < 0 || routerMac > 1)
    {
        printf("routerMac should be 0 or 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming routerMac is 0...\n");
        routerMac = 0;
    }

    if (pktCmd < 0 || pktCmd > 3)
    {
        printf("pktCmd should be between 0 to 3 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming pktCmd is 0...\n");
        pktCmd = 0;
    }

    if (encapType < -1 || encapType > 9)
    {
        printf("encapType( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               encapType, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (geneveTunnelType < 0 || geneveTunnelType > 2)
    {
        printf("geneveTunnelType should be between 0 to 2 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming geneveTunnelType is 0...\n");
        geneveTunnelType = 0;
    }

    if (isLag < 0 || isLag > 1)
    {
        printf("isLag should be either 0 or 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming isLag is 0... \n");
        isLag = 0;
    }

    if (lagId < -1)
    {
        printf("lagId (%d) out of range error on line # %d in configuration file \" %s \".\n",
               lagId, lineNum, configFile.filename);
        return;
    }

    if (extendedPortIdx < -1 || extendedPortIdx > 1023)
    {
        printf("extendedPortIdx( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               extendedPortIdx, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (isMcast < 0 || isMcast > 1)
    {
        printf("isMcast should be either 0 or 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming isMcast is 0... \n");
        isMcast = 0;
    }

    //Get mvif list
    memset(newData.mvifIdx, 0, sizeof(newData.mvifIdx));
    mvifStrList = strtok(mvif, ",");
    i = 0;
    while (mvifStrList)
    {
        newData.mvifIdx[i++] = atoi(mvifStrList);
        mvifStrList = strtok(NULL, ",");
    }

    if (vifMcast < 0)
    {
        printf("mvif should be positive on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    newData.geneveTunnelType = geneveTunnelType;
    newData.port= port;
    newData.vlan= vlan;
    newData.controlMac = controlMac;
    newData.routerMac = routerMac;
    newData.pktCmd = pktCmd;
    newData.tunnelIdx = tnlIdx;
    newData.encapType = encapType;
    newData.isLag = isLag;
    newData.lagId = lagId;
    newData.extendedPortIdx = extendedPortIdx;
    newData.isMcast = isMcast;
    newData.vifMcastIdx = vifMcast;
    newData.mcastIdx = mcastIdx;
    newData.agingFlag = agingFlag;

    addEntry(&configFile.macData, &newData, sizeof(MacEntry));
}

/**
 * \fn processMacScopeData
 * \private
 * \brief Parses line of text from configuration file to determine MacEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processMacScopeData(int lineNum, char *line)
{
    int port, vlan, controlMac, routerMac, pktCmd, tnlIdx, encapType;
    unsigned int macAddr[XP_MAC_ADDR_LEN];
    int errorCode, geneveTunnelType, isLag, lagId, extendedPortIdx, isMcast,
        vifMcast, scopeId, agingFlag;
    int i = 0;
    char mvif[128];
    char* mvifStrList = NULL;

    MacScopeEntry newData;

    errorCode= sscanf(line,
                      "%x : %x : %x : %x : %x : %x %d %d %d %d %d %d %d %d %d %d %d %d %127s %d %d %d",
                      &macAddr[5],
                      &macAddr[4], &macAddr[3], &macAddr[2], &macAddr[1], &macAddr[0],
                      &vlan, &port, &controlMac, &routerMac, &pktCmd, &tnlIdx, &encapType,
                      &geneveTunnelType, &isLag, &lagId, &extendedPortIdx, &isMcast, mvif, &vifMcast,
                      &scopeId, &agingFlag);
    if (errorCode != XP_MAC_ADDR_LEN + 16)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    if (port < 0 || port > 136)
    {
        printf("Port number ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               port, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (vlan < 0 || vlan > 4095)
    {
        printf("VlanId ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               vlan, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
    {
        if (macAddr[i] > 255)
        {
            printf("MAC address value ( 0x%x ) out of range error on line # %d in configuration file \" %s \".\n",
                   macAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.macAddr[i]= macAddr[i];
        }
    }

    if (controlMac < 0 || controlMac > 1)
    {
        printf("controlMac should be 0 or 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming controlMac is 0...\n");
        controlMac = 0;
    }

    if (routerMac < 0 || routerMac > 1)
    {
        printf("routerMac should be 0 or 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming routerMac is 0...\n");
        routerMac = 0;
    }

    if (pktCmd < 0 || pktCmd > 3)
    {
        printf("pktCmd should be between 0 to 3 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming pktCmd is 0...\n");
        pktCmd = 0;
    }

    if (encapType < -1 || encapType > 9)
    {
        printf("encapType( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               encapType, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (geneveTunnelType < 0 || geneveTunnelType > 2)
    {
        printf("geneveTunnelType should be between 0 to 2 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming geneveTunnelType is 0...\n");
        geneveTunnelType = 0;
    }

    if (isLag < 0 || isLag > 1)
    {
        printf("isLag should be either 0 or 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming isLag is 0... \n");
        isLag = 0;
    }

    if (lagId < -1)
    {
        printf("lagId (%d) out of range error on line # %d in configuration file \" %s \".\n",
               lagId, lineNum, configFile.filename);
        return;
    }

    if (extendedPortIdx < -1 || extendedPortIdx > 1023)
    {
        printf("extendedPortIdx( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               extendedPortIdx, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (isMcast < 0 || isMcast > 1)
    {
        printf("isMcast should be either 0 or 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming isMcast is 0... \n");
        isMcast = 0;
    }

    //Get mvif list
    memset(newData.mvifIdx, 0, sizeof(newData.mvifIdx));
    mvifStrList = strtok(mvif, ",");
    i = 0;
    while (mvifStrList)
    {
        newData.mvifIdx[i++] = atoi(mvifStrList);
        mvifStrList = strtok(NULL, ",");
    }

    if (vifMcast < 0)
    {
        printf("mvif should be positive on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    newData.geneveTunnelType = geneveTunnelType;
    newData.port= port;
    newData.vlan= vlan;
    newData.controlMac = controlMac;
    newData.routerMac = routerMac;
    newData.pktCmd = pktCmd;
    newData.tunnelIdx = tnlIdx;
    newData.encapType = encapType;
    newData.isLag = isLag;
    newData.lagId = lagId;
    newData.extendedPortIdx = extendedPortIdx;
    newData.isMcast = isMcast;
    newData.vifMcastIdx = vifMcast;
    newData.scopeId = scopeId;
    newData.agingFlag = agingFlag;

    addEntry(&configFile.macScopeData, &newData, sizeof(MacScopeEntry));
}


/**
 * \fn processIpxNhData
 * \private
 * \brief Parses line of text from configuration file to determine IpvxEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpxNhData(int lineNum, char *line)
{
    unsigned int macAddr[XP_MAC_ADDR_LEN];
    int errorCode;
    uint32_t destPort;
    uint32_t nhId;
    int cTagVid;
    int virtualId;
    uint32_t isTagged, nhType;
    int32_t tnlIdx;
    int32_t mplsIdx, geneveTunnelType;
    int32_t pktCmd, procTTL, lagId, extendedPortIdx, reasonCode,
            allocateNhWithId = 0;
    IpxNhEntry newData;
    int i = 0;

    errorCode= sscanf(line,
                      "%u %02X : %02X : %02X : %02X : %02X : %02X %d %d %u %u %d %d %d %d %d %u %d %d %d %d",
                      &nhId, &macAddr[5], &macAddr[4], &macAddr[3], &macAddr[2], &macAddr[1],
                      &macAddr[0],
                      &cTagVid, &virtualId, &isTagged, &destPort, &tnlIdx, &mplsIdx,
                      &geneveTunnelType, &pktCmd, &procTTL, &nhType, &lagId, &extendedPortIdx,
                      &reasonCode, &allocateNhWithId);
    if (errorCode < 20)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s %d\n", line, errorCode);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
    {
        if (macAddr[i] > 255)
        {
            printf("MAC address value ( 0X%X ) out of range error on line # %d in configuration file \" %s \".\n",
                   macAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.macAddr[i]= macAddr[i];
        }
    }
    if ((tnlIdx != -1) && (mplsIdx != -1))
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" Invalid Entry for tnlIdx: %d and mplsIdx %d Both can not be set %s\":",
               lineNum, tnlIdx, mplsIdx, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }


    if (geneveTunnelType < 0 || geneveTunnelType > 2)
    {
        printf("geneveTunnelType should be between 0 to 2 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming geneveTunnelType is 0...\n");
        geneveTunnelType = 0;
    }

    if (pktCmd < 0 || pktCmd > 3)
    {
        printf("pktCmd should be between 0 to 3 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming pktCmd is 0...\n");
        pktCmd = 0;
    }

    if (procTTL < 0 || procTTL > 1)
    {
        printf("procTTL should be between 0 to 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming procTTL is 0...\n");
        procTTL = 0;
    }

    if (extendedPortIdx < -1 || extendedPortIdx > 1023)
    {
        printf("extendedPortIdx should be between -1 to 1023 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (allocateNhWithId < 0 || allocateNhWithId > 1)
    {
        printf("allocateNhWithId should be between 0 to 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming allocateNhWithId is 0...\n");
        allocateNhWithId = 0;
    }


    //TODO: Thasleem add validation for reasonCode

    newData.nhId = nhId;
    newData.cTagVid = cTagVid;
    newData.isTagged=isTagged;
    newData.virtualId = virtualId;
    newData.destPort = destPort;
    newData.tnlIdx = tnlIdx;
    newData.mplsIdx = mplsIdx;
    newData.geneveTunnelType = geneveTunnelType;
    newData.pktCmd = pktCmd;
    newData.procTTL = procTTL;
    newData.nhType = nhType;
    newData.lagId = lagId;
    newData.extendedPortIdx = extendedPortIdx;
    newData.reasonCode = reasonCode;
    newData.allocateNhWithId = allocateNhWithId;
    addEntry(&configFile.ipvxNhData, &newData, sizeof(IpxNhEntry));
}

/**
 * \fn processIpxNhScopeData
 * \private
 * \brief Parses line of text from configuration file to determine IpvxEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpxNhScopeData(int lineNum, char *line)
{
    unsigned int macAddr[XP_MAC_ADDR_LEN];
    int errorCode;
    uint32_t destPort;
    uint32_t nhId;
    int cTagVid;
    int virtualId;
    int scopeId;
    uint32_t isTagged, nhType;
    int32_t tnlIdx;
    int32_t mplsIdx, geneveTunnelType;
    int32_t pktCmd, procTTL, lagId, extendedPortIdx, reasonCode;
    IpxNhScopeEntry newData;
    int i = 0;

    errorCode= sscanf(line,
                      "%u %02X : %02X : %02X : %02X : %02X : %02X %d %d %u %u %d %d %d %d %d %u %d %d %d %d",
                      &nhId, &macAddr[5], &macAddr[4], &macAddr[3], &macAddr[2], &macAddr[1],
                      &macAddr[0],
                      &cTagVid, &virtualId, &isTagged, &destPort, &tnlIdx, &mplsIdx,
                      &geneveTunnelType, &pktCmd, &procTTL, &nhType, &lagId, &extendedPortIdx,
                      &reasonCode, &scopeId);
    if (errorCode != 21)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s %d\n", line, errorCode);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
    {
        if (macAddr[i] > 255)
        {
            printf("MAC address value ( 0X%X ) out of range error on line # %d in configuration file \" %s \".\n",
                   macAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.macAddr[i]= macAddr[i];
        }
    }
    if ((tnlIdx != -1) && (mplsIdx != -1))
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" Invalid Entry for tnlIdx: %d and mplsIdx %d Both can not be set %s\":",
               lineNum, tnlIdx, mplsIdx, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }


    if (geneveTunnelType < 0 || geneveTunnelType > 2)
    {
        printf("geneveTunnelType should be between 0 to 2 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming geneveTunnelType is 0...\n");
        geneveTunnelType = 0;
    }

    if (pktCmd < 0 || pktCmd > 3)
    {
        printf("pktCmd should be between 0 to 3 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming pktCmd is 0...\n");
        pktCmd = 0;
    }

    if (procTTL < 0 || procTTL > 1)
    {
        printf("procTTL should be between 0 to 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming procTTL is 0...\n");
        procTTL = 0;
    }

    if (extendedPortIdx < -1 || extendedPortIdx > 1023)
    {
        printf("extendedPortIdx should be between -1 to 1023 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    //TODO: Thasleem add validation for reasonCode

    newData.nhId = nhId;
    newData.cTagVid = cTagVid;
    newData.isTagged=isTagged;
    newData.virtualId = virtualId;
    newData.destPort = destPort;
    newData.tnlIdx = tnlIdx;
    newData.mplsIdx = mplsIdx;
    newData.geneveTunnelType = geneveTunnelType;
    newData.pktCmd = pktCmd;
    newData.procTTL = procTTL;
    newData.nhType = nhType;
    newData.lagId = lagId;
    newData.extendedPortIdx = extendedPortIdx;
    newData.reasonCode = reasonCode;
    newData.scopeId = scopeId;
    addEntry(&configFile.ipvxNhScopeData, &newData, sizeof(IpxNhScopeEntry));
}

/**
 * \fn processIpv4SaiNhData
 * \private
 * \brief Parses line of text from configuration file to determine IpvxEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpv4SaiNhData(int lineNum, char *line)
{
    int paramPresent;
    int ipv4Addr[XP_IPV4_ADDR_LEN];
    uint32_t nextHopType;
    uint32_t nhId;
    uint32_t l3IntfId ;
    Ipv4SaiNhEntry newData;
    int i = 0;

    paramPresent= sscanf(line, "%u %d.%d.%d.%d %u %u",
                         &nhId, &ipv4Addr[3], &ipv4Addr[2], &ipv4Addr[1], &ipv4Addr[0], &l3IntfId,
                         &nextHopType);
    if (paramPresent != 7)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_IPV4_ADDR_LEN; ++i)
    {
        if (ipv4Addr[i] < 0 || ipv4Addr[i] > 255)
        {
            printf("IPV4 address value ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4Addr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.ipv4Addr[i]= ipv4Addr[i];
        }
    }


    newData.nhId = nhId;
    newData.l3IntfId = l3IntfId;
    newData.nextHopType = nextHopType;
    addEntry(&configFile.ipv4SaiNhData, &newData, sizeof(Ipv4SaiNhEntry));
}

/**
 * \fn processIpv6SaiNhData
 * \private
 * \brief Parses line of text from configuration file to determine IpvxEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpv6SaiNhData(int lineNum, char *line)
{
    int paramPresent;
    unsigned int ipv6Addr[XP_IPV6_ADDR_LEN];
    uint32_t nextHopType;
    uint32_t nhId;
    uint32_t l3IntfId;
    Ipv6SaiNhEntry newData;
    int i = 0;

    paramPresent= sscanf(line,
                         "%u %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X %u %u",
                         &nhId, &ipv6Addr[15], &ipv6Addr[14], &ipv6Addr[13], &ipv6Addr[12],
                         &ipv6Addr[11], &ipv6Addr[10], &ipv6Addr[9], &ipv6Addr[8], &ipv6Addr[7],
                         &ipv6Addr[6], &ipv6Addr[5], &ipv6Addr[4], &ipv6Addr[3], &ipv6Addr[2],
                         &ipv6Addr[1], &ipv6Addr[0],  &l3IntfId, &nextHopType);
    if (paramPresent != 19)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_IPV6_ADDR_LEN; ++i)
    {
        if (ipv6Addr[i] > 0xFF)
        {
            printf("IPV6 address value ( 0X%X ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv6Addr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.ipv6Addr[i]= ipv6Addr[i];
        }
    }

    newData.nhId = nhId;
    newData.l3IntfId = l3IntfId;
    newData.nextHopType = nextHopType;
    addEntry(&configFile.ipv6SaiNhData, &newData, sizeof(Ipv6SaiNhEntry));
}


/**
 * \fn processIpv4SaiNeighborData
 * \private
 * \brief Parses line of text from configuration file to determine IpvxEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpv4SaiNeighborData(int lineNum, char *line)
{
    unsigned int macAddr[XP_MAC_ADDR_LEN];
    int ipv4Addr[XP_IPV4_ADDR_LEN];
    int errorCode;
    int32_t pktCmd;
    uint32_t l3IntfId;
    Ipv4SaiNeighborEntry newData;
    int i = 0;

    errorCode= sscanf(line,
                      "%d.%d.%d.%d %u %02X : %02X : %02X : %02X : %02X : %02X %d",
                      &ipv4Addr[3], &ipv4Addr[2], &ipv4Addr[1], &ipv4Addr[0], &l3IntfId, &macAddr[5],
                      &macAddr[4], &macAddr[3], &macAddr[2], &macAddr[1], &macAddr[0], &pktCmd);
    if (errorCode != 12)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
    {
        if (macAddr[i] > 255)
        {
            printf("MAC address value ( 0X%X ) out of range error on line # %d in configuration file \" %s \".\n",
                   macAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.macSA[i]= macAddr[i];
        }
    }
    if (pktCmd < 0 || pktCmd > 6)
    {
        printf("pktCmd should be between 0 to 3 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming pktCmd is 0...\n");
        pktCmd = 0;
    }
    for (i= 0; i < XP_IPV4_ADDR_LEN; ++i)
    {
        if (ipv4Addr[i] < 0 || ipv4Addr[i] > 255)
        {
            printf("IPV4 address value ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4Addr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.ipv4Addr[i]= ipv4Addr[i];
        }
    }


    newData.l3IntfId = l3IntfId;
    newData.pktCmd = pktCmd;
    addEntry(&configFile.ipv4SaiNeighborData, &newData,
             sizeof(Ipv4SaiNeighborEntry));
}

/**
 * \fn processIpv6SaiNeighborData
 * \private
 * \brief Parses line of text from configuration file to determine IpvxEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpv6SaiNeighborData(int lineNum, char *line)
{
    unsigned int macAddr[XP_MAC_ADDR_LEN];
    unsigned int ipv6Addr[XP_IPV6_ADDR_LEN];
    int errorCode;
    int32_t pktCmd;
    uint32_t l3IntfId;
    Ipv6SaiNeighborEntry newData;
    int i = 0;

    errorCode= sscanf(line,
                      "%02X : %02X : %02X : %02X :%02X : %02X : %02X : %02X :%02X : %02X : %02X : %02X :%02X : %02X : %02X : %02X %u %02X : %02X : %02X : %02X : %02X : %02X %d",
                      &ipv6Addr[15], &ipv6Addr[14], &ipv6Addr[13], &ipv6Addr[12], &ipv6Addr[11],
                      &ipv6Addr[10], &ipv6Addr[9], &ipv6Addr[8], &ipv6Addr[7], &ipv6Addr[6],
                      &ipv6Addr[5], &ipv6Addr[4], &ipv6Addr[3], &ipv6Addr[2], &ipv6Addr[1],
                      &ipv6Addr[0], &l3IntfId, &macAddr[5], &macAddr[4], &macAddr[3], &macAddr[2],
                      &macAddr[1], &macAddr[0], &pktCmd);
    if (errorCode != 24)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
    {
        if (macAddr[i] > 255)
        {
            printf("MAC address value ( 0X%X ) out of range error on line # %d in configuration file \" %s \".\n",
                   macAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.macSA[i]= macAddr[i];
        }
    }
    if (pktCmd < 0 || pktCmd > 6)
    {
        printf("pktCmd should be between 0 to 3 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming pktCmd is 0...\n");
        pktCmd = 0;
    }
    for (i= 0; i < XP_IPV6_ADDR_LEN; ++i)
    {
        if (ipv6Addr[i] > 0xFF)
        {
            printf("IPV6 address value ( 0X%X ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv6Addr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.ipv6Addr[i]= ipv6Addr[i];
        }
    }

    newData.l3IntfId = l3IntfId;
    newData.pktCmd = pktCmd;
    addEntry(&configFile.ipv6SaiNeighborData, &newData,
             sizeof(Ipv6SaiNeighborEntry));
}

/**
 * \fn processIpv4SaiRouteData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv4RouteEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpv4SaiRouteData(int lineNum, char *line)
{
    int  vrfId;
    int ipv4Addr[XP_IPV4_ADDR_LEN];
    int errorCode;
    uint32_t netMask;
    Ipv4SaiRouteEntry newData;
    uint32_t nhGroupId;
    uint32_t trapPriority;
    int i = 0;
    uint32_t nhId;
    uint32_t pktCmd;

    errorCode= sscanf(line, "%d . %d . %d . %d %d %u %u %u %u %u", &ipv4Addr[3],
                      &ipv4Addr[2], &ipv4Addr[1], &ipv4Addr[0], &vrfId,
                      &netMask, &trapPriority, &nhGroupId, &nhId, &pktCmd);
    if (errorCode != 10)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_IPV4_ADDR_LEN; ++i)
    {
        if (ipv4Addr[i] < 0 || ipv4Addr[i] > 255)
        {
            printf("IPV4 address value ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4Addr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.ipv4Addr[i]= ipv4Addr[i];
        }
    }


    if (vrfId < 0 || vrfId > 255)
    {
        printf("vrf should be between 0 to 255 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming vrf is 0...\n");
        vrfId = 0;
    }


    if (netMask > 32)
    {
        printf("netMask should be between 0 to 32 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming netMask is 32...\n");
        netMask = 32;
    }

    newData.vrfId = vrfId;
    newData.netMask = netMask;
    newData.trapPriority = trapPriority;
    newData.pktCmd = pktCmd;
    newData.nhId = nhId;
    newData.nhGroupId = nhGroupId;
    addEntry(&configFile.ipv4SaiRouteData, &newData, sizeof(Ipv4SaiRouteEntry));
}

/**
 * \fn processIpv6SaiRouteData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv6RouteEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpv6SaiRouteData(int lineNum, char *line)
{
    int  vrfId;
    unsigned int ipv6Addr[XP_IPV6_ADDR_LEN];
    int errorCode;
    uint32_t netMask;
    Ipv6SaiRouteEntry newData;
    int i = 0;
    uint32_t nhId;
    uint32_t trapPriority;
    uint32_t nhGroupId;
    uint32_t pktCmd;

    errorCode= sscanf(line,
                      " %02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X %d %u %u %u %u %u",
                      &ipv6Addr[15], &ipv6Addr[14], &ipv6Addr[13], &ipv6Addr[12], &ipv6Addr[11],
                      &ipv6Addr[10], &ipv6Addr[9], &ipv6Addr[8], &ipv6Addr[7], &ipv6Addr[6],
                      &ipv6Addr[5], &ipv6Addr[4], &ipv6Addr[3], &ipv6Addr[2], &ipv6Addr[1],
                      &ipv6Addr[0], &vrfId, &netMask, &trapPriority, &nhGroupId, &nhId, &pktCmd);
    if (errorCode != 22)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }
    if (vrfId < 0 || vrfId > 255)
    {
        printf("vrf should be between 0 to 255 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming vrf is 0...\n");
        vrfId = 0;
    }
    for (i= 0; i < XP_IPV6_ADDR_LEN; ++i)
    {
        if (ipv6Addr[i] > 0xFF)
        {
            printf("IPV6 address value ( 0X%X ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv6Addr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.ipv6Addr[i]= ipv6Addr[i];
        }
    }

    if (netMask > 128)
    {
        printf("netMask should be between 0 to 32 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming netMask is 32...\n");
        netMask = 32;
    }

    newData.vrfId = vrfId;
    newData.netMask = netMask;
    newData.trapPriority = trapPriority;
    newData.pktCmd = pktCmd;
    newData.nhId = nhId;
    newData.nhGroupId = nhGroupId;
    addEntry(&configFile.ipv6SaiRouteData, &newData, sizeof(Ipv6SaiRouteEntry));
}

/**
 * \fn processIpv4HostData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv4HostEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpv4HostData(int lineNum, char *line)
{
    int vrf;
    int ipv4Addr[XP_IPV4_ADDR_LEN];
    int errorCode;
    Ipv4HostEntry newData;
    int nhId;
    int i = 0;

    errorCode= sscanf(line, "%d . %d . %d . %d %d %d ", &ipv4Addr[3], &ipv4Addr[2],
                      &ipv4Addr[1], &ipv4Addr[0], &vrf, &nhId);
    if (errorCode != 6)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_IPV4_ADDR_LEN; ++i)
    {
        if (ipv4Addr[i] < 0 || ipv4Addr[i] > 255)
        {
            printf("IPV4 address value ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4Addr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.ipv4Addr[i]= ipv4Addr[i];
        }
    }


    if (vrf < 0 || vrf > 255)
    {
        printf("vrf should be between 0 to 255 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming vrf is 0...\n");
        vrf = 0;
    }


    newData.vrf = vrf;
    newData.nhId = nhId;

    addEntry(&configFile.ipv4HostData, &newData, sizeof(Ipv4HostEntry));
}

/**
 * \fn processIpv4HostScopeData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv4HostEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpv4HostScopeData(int lineNum, char *line)
{
    int vrf;
    int ipv4Addr[XP_IPV4_ADDR_LEN];
    int errorCode;
    int scopeId;
    Ipv4HostScopeEntry newData;
    int nhId;
    int i = 0;

    errorCode= sscanf(line, "%d . %d . %d . %d %d %d %d", &ipv4Addr[3],
                      &ipv4Addr[2],
                      &ipv4Addr[1], &ipv4Addr[0], &vrf, &nhId, &scopeId);
    if (errorCode != 7)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_IPV4_ADDR_LEN; ++i)
    {
        if (ipv4Addr[i] < 0 || ipv4Addr[i] > 255)
        {
            printf("IPV4 address value ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4Addr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.ipv4Addr[i]= ipv4Addr[i];
        }
    }

    if (vrf < 0 || vrf > 255)
    {
        printf("vrf should be between 0 to 255 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming vrf is 0...\n");
        vrf = 0;
    }


    newData.vrf = vrf;
    newData.nhId = nhId;
    newData.scopeId = scopeId;

    addEntry(&configFile.ipv4HostScopeData, &newData, sizeof(Ipv4HostScopeEntry));
}

/**
 * \fn processIpv4RouteData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv4RouteEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpv4RouteData(int lineNum, char *line)
{
    int  vrf;
    int ipv4Addr[XP_IPV4_ADDR_LEN];
    int errorCode;
    uint32_t ecmpSize;
    uint32_t netMask;
    Ipv4RouteEntry newData;
    initEntryList(&(newData.nhId));
    char temp[50];
    char buf[5];
    int tempInc=0;
    int bufInc=0;
    int i = 0;
    uint16_t nhId;

    errorCode= sscanf(line, "%d . %d . %d . %d %d %u %u %49s", &ipv4Addr[3],
                      &ipv4Addr[2], &ipv4Addr[1], &ipv4Addr[0], &vrf,
                      &netMask, &ecmpSize, temp);
    if (errorCode != 8)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    while (temp[tempInc]!='\0')
    {
        if (isdigit(temp[tempInc]))
        {
            buf[bufInc]=temp[tempInc];
            bufInc++;
        }
        else if (temp[tempInc]==',')
        {
            buf[bufInc]='\0';
            nhId = atoi(buf);
            addEntry(&(newData.nhId), &(nhId), sizeof(uint16_t));
            bufInc = 0;
        }
        else
        {
            printf(", ERROR: Unable to parse NH-ID # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            freeEntryList(&(newData.nhId));
            return;
        }
        tempInc++;
    }
    buf[bufInc]='\0';
    nhId = atoi(buf);
    addEntry(&(newData.nhId), &(nhId), sizeof(uint16_t));
    bufInc = 0;
    for (i= 0; i < XP_IPV4_ADDR_LEN; ++i)
    {
        if (ipv4Addr[i] < 0 || ipv4Addr[i] > 255)
        {
            printf("IPV4 address value ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4Addr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            freeEntryList(&(newData.nhId));
            return;
        }
        else
        {
            newData.ipv4Addr[i]= ipv4Addr[i];
        }
    }


    if (vrf < 0 || vrf > 255)
    {
        printf("vrf should be between 0 to 255 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming vrf is 0...\n");
        vrf = 0;
    }


    if (netMask > 32)
    {
        printf("netMask should be between 0 to 32 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming netMask is 32...\n");
        netMask = 32;
    }

    if (ecmpSize > 16)
    {
        printf("ecmpSize should be between 0 to 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming ecmpSize is 1...\n");
        ecmpSize = 1;
    }

    if ((uint32_t)numOfEntry(&(newData.nhId)) != ecmpSize)
    {
        printf(", ERROR: Unable to parse due to mismatch between ecmpSize and Number of NH-ID  # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        freeEntryList(&(newData.nhId));
        return;
    }
    newData.vrf = vrf;
    newData.netMask = netMask;
    newData.ecmpSize = ecmpSize;
    addEntry(&configFile.ipv4RouteData, &newData, sizeof(Ipv4RouteEntry));
}

/**
 * \fn processIpv4RouteScopeData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv4RouteEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpv4RouteScopeData(int lineNum, char *line)
{
    int  vrf;
    int ipv4Addr[XP_IPV4_ADDR_LEN];
    int errorCode;
    int scopeId;
    uint32_t ecmpSize;
    uint32_t netMask;
    Ipv4RouteScopeEntry newData;
    initEntryList(&(newData.nhId));
    char temp[50];
    char buf[5];
    int tempInc=0;
    int bufInc=0;
    int i = 0;
    uint16_t nhId;

    errorCode= sscanf(line, "%d . %d . %d . %d %d %u %u %49s %d", &ipv4Addr[3],
                      &ipv4Addr[2], &ipv4Addr[1], &ipv4Addr[0], &vrf,
                      &netMask, &ecmpSize, temp, &scopeId);
    if (errorCode != 9)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    while (temp[tempInc]!='\0')
    {
        if (isdigit(temp[tempInc]))
        {
            buf[bufInc]=temp[tempInc];
            bufInc++;
        }
        else if (temp[tempInc]==',')
        {
            buf[bufInc]='\0';
            nhId = atoi(buf);
            addEntry(&(newData.nhId), &(nhId), sizeof(uint16_t));
            bufInc = 0;
        }
        else
        {
            printf(", ERROR: Unable to parse NH-ID # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            freeEntryList(&(newData.nhId));
            return;
        }
        tempInc++;
    }
    buf[bufInc]='\0';
    nhId = atoi(buf);
    addEntry(&(newData.nhId), &(nhId), sizeof(uint16_t));
    bufInc = 0;
    for (i= 0; i < XP_IPV4_ADDR_LEN; ++i)
    {
        if (ipv4Addr[i] < 0 || ipv4Addr[i] > 255)
        {
            printf("IPV4 address value ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4Addr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            freeEntryList(&(newData.nhId));
            return;
        }
        else
        {
            newData.ipv4Addr[i]= ipv4Addr[i];
        }
    }


    if (vrf < 0 || vrf > 255)
    {
        printf("vrf should be between 0 to 255 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming vrf is 0...\n");
        vrf = 0;
    }


    if (netMask > 32)
    {
        printf("netMask should be between 0 to 32 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming netMask is 32...\n");
        netMask = 32;
    }

    if (ecmpSize > 16)
    {
        printf("ecmpSize should be between 0 to 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming ecmpSize is 1...\n");
        ecmpSize = 1;
    }

    if ((uint32_t)numOfEntry(&(newData.nhId)) != ecmpSize)
    {
        printf(", ERROR: Unable to parse due to mismatch between ecmpSize and Number of NH-ID  # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        freeEntryList(&(newData.nhId));
        return;
    }
    newData.vrf = vrf;
    newData.netMask = netMask;
    newData.ecmpSize = ecmpSize;
    newData.scopeId = scopeId;
    addEntry(&configFile.ipv4RouteScopeData, &newData, sizeof(Ipv4RouteScopeEntry));
}

/**
 * \fn processSaiNhGroupData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv4RouteEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processSaiNhGroupData(int lineNum, char *line)
{
    int errorCode;
    uint32_t ecmpSize;
    uint32_t nhGroupId;
    uint32_t groupAttrType;
    IpvxSaiNhGroupEntry newData = {0};
    initEntryList(&(newData.nhIdList));
    char temp[50] = {'\0'};
    char buf[5] = {'\0'};
    int tempInc=0;
    int bufInc=0;
    uint16_t nhId = 0;

    errorCode= sscanf(line, "%u %u %u %49s", &nhGroupId, &ecmpSize, &groupAttrType,
                      temp);
    if (errorCode != 4)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }
    while (temp[tempInc]!='\0')
    {
        if (isdigit(temp[tempInc]))
        {
            buf[bufInc]=temp[tempInc];
            bufInc++;
        }
        else if (temp[tempInc]==',')
        {
            buf[bufInc]='\0';
            nhId = atoi(buf);
            addEntry(&(newData.nhIdList), &(nhId), sizeof(uint16_t));
            bufInc = 0;
        }
        else
        {
            printf(", ERROR: Unable to parse NH-ID # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            freeEntryList(&(newData.nhIdList));
            return;
        }
        tempInc++;
    }
    buf[bufInc]='\0';
    nhId = atoi(buf);
    printf("nhId = %d\n", nhId);
    addEntry(&(newData.nhIdList), &(nhId), sizeof(uint16_t));
    bufInc = 0;
    if (ecmpSize > 16)
    {
        printf("ecmpSize should be between 0 to 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming ecmpSize is 1...\n");
        ecmpSize = 1;
    }

    if ((uint32_t)numOfEntry(&(newData.nhIdList)) != ecmpSize)
    {
        printf(", ERROR: Unable to parse due to mismatch between ecmpSize and Number of NH-ID  # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        freeEntryList(&(newData.nhIdList));
        return;
    }
    newData.nhGroupId = nhGroupId;
    newData.groupAttrType = groupAttrType;
    newData.ecmpSize = ecmpSize;
    addEntry(&configFile.ipSaiNhGroupData, &newData, sizeof(IpvxSaiNhGroupEntry));
}


/**
 * \fn processIpv6HostData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv6HostEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpv6HostData(int lineNum, char *line)
{
    int vrf;
    int nhId;
    unsigned int ipv6Addr[XP_IPV6_ADDR_LEN];
    int errorCode;
    int i;

    Ipv6HostEntry newData;

    errorCode= sscanf(line,
                      " %02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X : "
                      "%02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X %d %d",
                      &ipv6Addr[15], &ipv6Addr[14], &ipv6Addr[13], &ipv6Addr[12], &ipv6Addr[11],
                      &ipv6Addr[10], &ipv6Addr[9], &ipv6Addr[8], &ipv6Addr[7], &ipv6Addr[6],
                      &ipv6Addr[5], &ipv6Addr[4], &ipv6Addr[3], &ipv6Addr[2], &ipv6Addr[1],
                      &ipv6Addr[0],  &vrf, &nhId);

    if (errorCode != 18)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }
    //TODO: support ipv6 validation
    for (i= 0; i < XP_IPV6_ADDR_LEN; ++i)
    {
        if (ipv6Addr[i] > 0xFF)
        {
            printf("IPV6 address value ( 0X%X ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv6Addr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.ipv6Addr[i]= ipv6Addr[i];
        }
    }


    if (vrf < 0 || vrf > 255)
    {
        printf("vrf should be between 0 to 255 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming is 0...\n");
        vrf = 0;
    }

    newData.vrf = vrf;
    newData.nhId = nhId;

    addEntry(&configFile.ipv6HostData, &newData, sizeof(Ipv6HostEntry));
}


/**
 * \fn processIpv6RouteData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv6RouteEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpv6RouteData(int lineNum, char *line)
{
    int vrf;
    unsigned int ipv6Addr[XP_IPV6_ADDR_LEN];
    int errorCode;
    uint32_t ecmpSize;
    uint32_t netMask;
    Ipv6RouteEntry newData;
    initEntryList(&(newData.nhId));
    char temp[50];
    char buf[5];
    int i = 0;
    uint16_t nhId;

    memset(&newData, 0, sizeof(Ipv6RouteEntry));

    //TODO: add support to read ipv6 addr
    errorCode= sscanf(line,
                      " %2x : %02X : %02X : %02X : %02X : %02X :%02X : %02X : "
                      "%02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X %d %u %u %49s",
                      &ipv6Addr[15], &ipv6Addr[14], &ipv6Addr[13], &ipv6Addr[12], &ipv6Addr[11],
                      &ipv6Addr[10], &ipv6Addr[9], &ipv6Addr[8], &ipv6Addr[7], &ipv6Addr[6],
                      &ipv6Addr[5], &ipv6Addr[4], &ipv6Addr[3], &ipv6Addr[2], &ipv6Addr[1],
                      &ipv6Addr[0], &vrf, &netMask, &ecmpSize, temp);
    if (errorCode != 20)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }
    int tempInc=0;
    int bufInc=0;

    while (temp[tempInc]!='\0')
    {
        if (isdigit(temp[tempInc]))
        {
            buf[bufInc]=temp[tempInc];
            bufInc++;
        }
        else if (temp[tempInc] ==',')
        {
            buf[bufInc]='\0';
            nhId = atoi(buf);
            addEntry(&(newData.nhId), &(nhId), sizeof(uint16_t));
            bufInc = 0;
        }
        else
        {
            printf(", ERROR: Unable to parse NH-ID  # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            freeEntryList(&(newData.nhId));
            return;
        }
        tempInc++;
    }
    buf[bufInc]='\0';
    nhId = atoi(buf);
    addEntry(&(newData.nhId), &(nhId), sizeof(uint16_t));
    bufInc = 0;

    for (i= 0; i < XP_IPV6_ADDR_LEN; ++i)
    {
        if (ipv6Addr[i] > 0xFF)
        {
            printf("IPV6 address value ( 0X%X ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv6Addr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            freeEntryList(&(newData.nhId));
            return;
        }
        else
        {
            newData.ipv6Addr[i]= ipv6Addr[i];
        }
    }


    if (vrf < 0 || vrf > 255)
    {
        printf("vrf should be between 0 to 255 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming vrf is 0...\n");
        vrf = 0;
    }


    if (netMask > 128)
    {
        printf("netMask should be between 0 to 128 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming netMask is 128...\n");
        netMask = 128;
    }

    if (ecmpSize > 16)
    {
        printf("ecmpSize should be between 0 to 16 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming ecmpSize is 1...\n");
        ecmpSize = 1;
    }

    if ((uint32_t)numOfEntry(&(newData.nhId)) != ecmpSize)
    {
        printf(", ERROR: Unable to parse due to mismatch between ecmpSize and Number of NH-ID  # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        freeEntryList(&(newData.nhId));
        return;
    }
    newData.vrf = vrf;
    newData.netMask = netMask;
    newData.ecmpSize = ecmpSize;
    addEntry(&configFile.ipv6RouteData, &newData, sizeof(Ipv6RouteEntry));
}

/**
 * \fn processSRHData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv4BridgeMcEntry.
 *
 */
void processSRHData(int lineNum, char *line)
{
    uint32_t srIdx, i;
    uint32_t numSegment;
    uint32_t nhId;
    int32_t errorCode;
    unsigned int ip0[XP_IPV6_ADDR_LEN];
    unsigned int ip1[XP_IPV6_ADDR_LEN];
    unsigned int ip2[XP_IPV6_ADDR_LEN];
    SrhEntry newData;

    errorCode= sscanf(line,
                      "%u %u %u %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
                      &srIdx, &numSegment, &nhId, &ip0[15], &ip0[14], &ip0[13], &ip0[12], &ip0[11],
                      &ip0[10], &ip0[9], &ip0[8], &ip0[7], &ip0[6], &ip0[5], &ip0[4], &ip0[3],
                      &ip0[2], &ip0[1], &ip0[0], &ip1[15], &ip1[14], &ip1[13], &ip1[12], &ip1[11],
                      &ip1[10], &ip1[9], &ip1[8], &ip1[7], &ip1[6], &ip1[5], &ip1[4], &ip1[3],
                      &ip1[2], &ip1[1], &ip1[0], &ip2[15], &ip2[14], &ip2[13], &ip2[12], &ip2[11],
                      &ip2[10], &ip2[9], &ip2[8], &ip2[7], &ip2[6], &ip2[5], &ip2[4], &ip2[3],
                      &ip2[2], &ip2[1], &ip2[0]);

    if (errorCode != 51)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s", line);
        printf("Skipping to next line...\n");
        return;
    }

    if ((int32_t)numSegment < 0 || numSegment > 3)
    {
        printf("numSegment ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               numSegment, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    memset(&newData, 0, sizeof(SrhEntry));
    newData.srIdx = srIdx;
    newData.nhId = nhId;
    newData.numSegment = numSegment;
    for (i= 0; i < XP_IPV6_ADDR_LEN; ++i)
    {
        newData.ip0[i] = ip0[i];
        newData.ip1[i] = ip1[i];
        newData.ip2[i] = ip2[i];
    }

    addEntry(&configFile.srhData, &newData, sizeof(SrhEntry));
}

/**
 * \fn processLocalSidData
 * \private
 * \brief Parses line of text from configuration file to determine LocalSidEntry.
 *
 */
void processLocalSidData(int lineNum, char *line)
{
    uint32_t i, flag;
    int32_t errorCode;
    uint32_t endFunction = 0, args = 0, ecmpEnable = 0, vrfId = 0, nhId = 0;
    unsigned int mySid[XP_IPV6_ADDR_LEN];
    localSidEntry newData;

    errorCode= sscanf(line,
                      "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X %u %u %u %u %u %u",
                      &mySid[15], &mySid[14], &mySid[13], &mySid[12], &mySid[11], &mySid[10],
                      &mySid[9], &mySid[8], &mySid[7], &mySid[6], &mySid[5], &mySid[4], &mySid[3],
                      &mySid[2], &mySid[1], &mySid[0], &endFunction, &args, &ecmpEnable, &vrfId,
                      &nhId, &flag);

    if (errorCode != 22)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s", line);
        printf("Skipping to next line...\n");
        return;
    }

    memset(&newData, 0, sizeof(localSidEntry));
    for (i= 0; i < XP_IPV6_ADDR_LEN; ++i)
    {
        newData.localSid[i] = mySid[i];
    }
    newData.func = endFunction;
    newData.args = args;
    newData.VRFId = vrfId;
    newData.nhId = nhId;
    newData.flag = flag;
    newData.ecmpEnable = ecmpEnable;

    addEntry(&configFile.mySidData, &newData, sizeof(localSidEntry));
}


/**
 * \fn processControlMacData
 * \private
 * \brief Parses line of text from configuration file to determine ControlMacEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processControlMacData(int lineNum, char *line)
{
    int keyVlanId, maskVlanId, routerMac, controlMac, controlActionEn;
    unsigned int keyMacAddr[XP_MAC_ADDR_LEN], maskMacAddr[XP_MAC_ADDR_LEN];
    int errorCode;
    int i = 0;
    ControlMacEntry newData;

    errorCode= sscanf(line, "%d %d %02X : %02X : %02X : %02X : %02X : %02X "
                      "%02X : %02X : %02X : %02X : %02X : %02X %d %d %d", &keyVlanId, &maskVlanId,
                      &keyMacAddr[5], &keyMacAddr[4], &keyMacAddr[3], &keyMacAddr[2],
                      &keyMacAddr[1], &keyMacAddr[0], &maskMacAddr[5], &maskMacAddr[4],
                      &maskMacAddr[3], &maskMacAddr[2], &maskMacAddr[1], &maskMacAddr[0],
                      &routerMac, &controlMac, &controlActionEn);
    if (errorCode != 17)
    {
        printf(", ERROR: Unable to parse line #  %d in configuration file  %s", lineNum,
               configFile.filename);
        printf("%s", line);
        printf("Skipping to next line...");
        return;
    }

    if (keyVlanId < 0 || keyVlanId > 4095)
    {
        printf("keyVlanId should be between 0 to 4095 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming keyVlanId is 0...");

        keyVlanId = 0;
    }

    if (maskVlanId < 0 || maskVlanId > 4095)
    {
        printf("maskVlanId should be between 0 to 4095 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming maskVlanId is 0...");
        maskVlanId = 0;
    }

    if (routerMac < 0 || routerMac > 1)
    {
        printf("routerMac should be between 0 to 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming routerMac is 0...");
        routerMac = 0;
    }

    if (controlMac < 0 || controlMac > 1)
    {
        printf("controlMac should be between 0 to 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming controlMac is 0...");
        controlMac = 0;
    }

    if (controlActionEn < 0 || controlActionEn > 1)
    {
        printf("controlActionEn should be between 0 to 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming controlActionEn is 0...");
        controlActionEn = 0;
    }

    for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
    {
        if (keyMacAddr[i] > 255)
        {
            printf("key MAC address value ( 0X%X ) out of range error on line # %d in configuration file \" %s \".\n",
                   keyMacAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.keyMacAddr[i]= keyMacAddr[i];
        }
    }

    for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
    {
        if (maskMacAddr[i] > 255)
        {
            printf("mask MAC address value ( 0X%X ) out of range error on line # %d in configuration file \" %s \".\n",
                   maskMacAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.maskMacAddr[i]= maskMacAddr[i];
        }
    }

    newData.keyVlanId = keyVlanId;
    newData.maskVlanId = maskVlanId;
    newData.routerMac = routerMac;
    newData.controlMac = controlMac;
    newData.controlActionEn = controlActionEn;
    addEntry(&configFile.controlMacData, &newData, sizeof(ControlMacEntry));
}


/**
 * \fn processTunnelData
 * \private
 * \brief Parses line of text from configuration file to determine TunnelEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processTunnelData(int lineNum, char *line)
{
    int vniVal, vlanVal, udpPort, portVal, bdVal, isMcast, tagType, setBd, tnlType;
    int tnlVlan, ipv4RouteEn, ipv6RouteEn, ipv4BridgeMcMode, ipv6BridgeMcMode,
        ipv4RouteMcEn, ipv6RouteMcEn, mplsRouteEn, ipv4RouteMcMode, ipv6RouteMcMode,
        vrfId, unregMcCmd;
    unsigned int dmacAddr[XP_MAC_ADDR_LEN];
    unsigned int smacAddr;
    int ip2[XP_IPV4_ADDR_LEN];
    int ip3[XP_IPV4_ADDR_LEN];
    int errorCode;
    unsigned int greProtocolId;
    int i = 0;
    int tnlIndex;
    TunnelEntry newData;
    int  bind2Vlan = 0;
    int  bindVlan;
    int  nhId;
    int  pktCmd;

    memset(&newData, 0, sizeof(TunnelEntry));

    // I'm not proud of this:
    errorCode= sscanf(line, " %d  %x : %x : %x : %x : %x : %x   %x "
                      " %d  %d . %d . %d . %d   %d . %d . %d . %d "
                      " %d %d %d %d %d %d %d %d %x %d %d %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d %d %d",
                      &tnlIndex, &dmacAddr[5], &dmacAddr[4], &dmacAddr[3], &dmacAddr[2], &dmacAddr[1],
                      &dmacAddr[0],
                      &smacAddr,
                      &tagType,
                      &ip2[3], &ip2[2], &ip2[1], &ip2[0],
                      &ip3[3], &ip3[2], &ip3[1], &ip3[0],
                      &vniVal, &vlanVal, &udpPort, &portVal, &bdVal, &isMcast, &setBd, &tnlType,
                      &greProtocolId, &bind2Vlan, &bindVlan,
                      &tnlVlan, &ipv4RouteEn, &ipv6RouteEn, &vrfId, &ipv4BridgeMcMode,
                      &ipv6BridgeMcMode, &ipv4RouteMcEn,
                      &ipv6RouteMcEn, &ipv4RouteMcMode, &ipv6RouteMcMode, &unregMcCmd, &mplsRouteEn,
                      &nhId, &pktCmd);

    if (errorCode != 1*XP_MAC_ADDR_LEN + 2*XP_IPV4_ADDR_LEN + 14 + 14)
    {
        printf(", ERROR: Unable to parse line #  %d in configuration file  %s", lineNum,
               configFile.filename);
        printf("%s", line);
        printf("Skipping to next line...\n");
        return;
    }

    if (portVal < 0 || portVal > 127)
    {
        printf("Port number ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               portVal, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.portId= portVal;

    if (vlanVal < 0 || vlanVal > 4095)
    {
        printf("VLAN ID ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               vlanVal, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.vlan= vlanVal;

    if (bdVal < 0 || bdVal > 4095)
    {
        printf("BD ID ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               bdVal, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.bdId= bdVal;

    if (tnlType != XP_CONFIGFILE_IPTNL_TYPE_VXLAN &&
        tnlType != XP_CONFIGFILE_IPTNL_TYPE_NVGRE &&
        tnlType != XP_CONFIGFILE_IPTNL_TYPE_GRE &&
        tnlType != XP_CONFIGFILE_IPTNL_TYPE_IP_OVER_IP &&
        tnlType != XP_CONFIGFILE_IPTNL_TYPE_GENEVE &&
        tnlType != XP_CONFIGFILE_IPTNL_TYPE_VPN_GRE &&
        tnlType != XP_CONFIGFILE_IPTNL_TYPE_VPN_GRE_LOOSE &&
        tnlType != XP_CONFIGFILE_IPTNL_TYPE_GRE_ERSPAN2)
    {
        printf(" tnlType ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               tnlType, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.tnlType= (IpTunnelType)tnlType;

    if (setBd < 0 || setBd > 1)
    {
        printf("Set Bd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               setBd, lineNum, configFile.filename);
        printf("Assummming setBd 1......");
        setBd = 1;
        return;
    }

    if (bind2Vlan < 0 || bind2Vlan > 1)
    {
        printf("bind2Vlan ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               bind2Vlan, lineNum, configFile.filename);
        printf("Assummming bind2Vlan 0......");
        bind2Vlan = 0;
        return;
    }


    for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
    {
        if (dmacAddr[i] > 255)
        {
            printf("DMAC address value ( 0x%x ) out of range error on line # %d in configuration file \" %s \".\n",
                   dmacAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        newData.dmac[i]= dmacAddr[i];

    }


    if (smacAddr > 255)
    {
        printf("SMAC address value ( 0x%x ) out of range error on line # %d in configuration file \" %s \".\n",
               smacAddr, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.smac= smacAddr;

    for (i= 0; i < XP_IPV4_ADDR_LEN; ++i)
    {
        if (ip2[i] < 0 || ip2[i] > 255 || ip3[i] < 0 || ip3[i] > 255)
        {
            printf("IP address value out of range error on line # %d in configuration file \" %s \".\n",
                   lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        newData.dip[i]= ip2[i];
        newData.myVtepIp[i]= ip3[i];
    }

    if (tnlVlan < 0 || tnlVlan > 4096)
    {
        printf(", ERROR:tnlVlan ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               tnlVlan, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (ipv4BridgeMcMode < 0 || ipv4BridgeMcMode > 3)
    {
        printf(", ERROR:ipv4BridgeMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv4BridgeMcMode, lineNum, configFile.filename);
        printf("Assuming ipv4BridgeMcMode is 0...\n");
        ipv4BridgeMcMode = 0;
    }

    if (ipv6BridgeMcMode < 0 || ipv6BridgeMcMode > 3)
    {
        printf(", ERROR:ipv6BridgeMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv6BridgeMcMode, lineNum, configFile.filename);
        printf("Assuming ipv6BridgeMcMode is 0...\n");
        ipv6BridgeMcMode = 0;
    }

    if (ipv4RouteMcEn < 0 || ipv4RouteMcEn > 1)
    {
        printf(", ERROR:ipv4RouteMcEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv4RouteMcMode, lineNum, configFile.filename);
        printf("Assuming ipv4RouteMcEn is 0...\n");
        ipv4RouteMcEn = 0;
    }

    if (ipv6RouteMcEn < 0 || ipv6RouteMcEn > 1)
    {
        printf(", ERROR:ipv6RouteMcEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv6RouteMcEn, lineNum, configFile.filename);
        printf("Assuming ipv6RouteMcEn is 0...\n");
        ipv6RouteMcEn = 0;
    }

    if (ipv4RouteMcMode < 0 || ipv4RouteMcMode > 1)
    {
        printf("ipv4RouteMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv4RouteMcMode, lineNum, configFile.filename);
        printf("Assuming ipv4RouteMcMode is 0...\n");
        ipv4RouteMcMode = 0;
    }

    if (ipv6RouteMcMode < 0 || ipv6RouteMcMode > 1)
    {
        printf("ipv6RouteMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv6RouteMcMode, lineNum, configFile.filename);
        printf("Assuming ipv6RouteMcMode is 0...\n");
        ipv6RouteMcMode = 0;
    }

    if (mplsRouteEn < 0 || mplsRouteEn > 1)
    {
        printf("mplsRouteEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               mplsRouteEn, lineNum, configFile.filename);
        printf("Assuming mplsRouteEn is 0...\n");
        mplsRouteEn = 0;
    }

    if (unregMcCmd < 0 || unregMcCmd > 1)
    {
        printf("unregMcCmd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               unregMcCmd, lineNum, configFile.filename);
        printf("Assuming unregMcCmd is 0...\n");
        unregMcCmd = 0;
    }

    newData.tnlVlan = tnlVlan;
    newData.ipv4RouteEn = ipv4RouteEn;;
    newData.ipv6RouteEn = ipv6RouteEn;
    newData.mplsRouteEn = mplsRouteEn;
    newData.ipv4BridgeMcMode = ipv4BridgeMcMode;
    newData.ipv6BridgeMcMode = ipv6BridgeMcMode;
    newData.ipv4RouteMcEn = ipv4RouteMcEn;
    newData.ipv6RouteMcEn = ipv6RouteMcEn;
    newData.ipv4RouteMcMode = ipv4RouteMcMode;
    newData.ipv6RouteMcMode = ipv6RouteMcMode;
    newData.unregMcCmd = unregMcCmd;
    newData.vrfId = vrfId;
    newData.tnlIdx = tnlIndex;
    newData.tagType = tagType;
    newData.isMcast = isMcast;
    newData.vni = vniVal;
    newData.udpPort = udpPort;
    newData.setBd = setBd;
    newData.greProtocolId=greProtocolId;
    newData.bind2Vlan = bind2Vlan;
    newData.bindVlan = bindVlan;
    newData.nhId = nhId;
    newData.pktCmd = pktCmd;
    addEntry(&configFile.tunnelData, &newData, sizeof(TunnelEntry));
}


/**
 * \fn processTunnelQinQData
 * \private
 * \brief Parses line of text from configuration file to determine TunnelQinQEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processTunnelQinQData(int lineNum, char *line)
{
    int sTagVal, cTagVal, vlanVal, tunnelVal, portVal, bdVal, isMcast, tagType;
    int errorCode;
    TunnelQinQEntry newData;

    errorCode= sscanf(line, " %d %d %d %d %d %d %d %d",
                      &tagType, &sTagVal, &cTagVal, &vlanVal, &tunnelVal, &portVal, &bdVal, &isMcast);

    if (errorCode != 8)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s", line);
        printf("Skipping to next line...\n");
        return;
    }

    if (portVal < 0 || portVal > 127)
    {
        printf("Port number ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               portVal, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.portId= portVal;

    if (vlanVal < 0 || vlanVal > 4095)
    {
        printf("VLAN ID  ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               vlanVal, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.vlan = vlanVal;

    if (bdVal < 0 || bdVal > 4095)
    {
        printf("BD ID  ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               bdVal, lineNum, configFile.filename);
        printf("Skipping to next line...");
        return;
    }
    newData.bdId = bdVal;
    newData.tagType = tagType;
    newData.isMcast = isMcast;
    newData.tunnelVif = tunnelVal;
    newData.sTag = sTagVal;
    newData.cTag = cTagVal;

    addEntry(&configFile.tunnelQinQData, &newData, sizeof(TunnelQinQEntry));
}


/**
 * \fn processTunnelPBBData
 * \private
 * \brief Parses line of text from configuration file to determine TunnelPBBEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processTunnelPBBData(int lineNum, char *line)
{
    //LOG(XP_LOG_TRACE, "in xpConfigFile::processTunnelPBBData()");
    int bTagVal, iSidVal, portVal, bdVal, pktCmd, isEndPt;
    unsigned int bSa[XP_MAC_ADDR_LEN];
    int errorCode;
    unsigned int bDa[XP_MAC_ADDR_LEN];
    int i = 0;
    TunnelPBBEntry newData;
    int  bind2Vlan = 0;
    int  bindVlan;

    // I'm not proud of this:
    errorCode= sscanf(line,
                      " %x : %x : %x : %x : %x : %x   %x : %x : %x : %x : %x : %x  %d %d %d %d %d %d %d %d",
                      &bDa[5], &bDa[4], &bDa[3], &bDa[2], &bDa[1], &bDa[0], &bSa[5], &bSa[4], &bSa[3],
                      &bSa[2], &bSa[1], &bSa[0],
                      &bTagVal, &iSidVal, &portVal, &bdVal, &bind2Vlan, &bindVlan, &pktCmd, &isEndPt);

    if (errorCode != 20)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s", line);
        printf("Skipping to next line...");
        return;
    }

    if (portVal < 0 || portVal > 127)
    {
        printf("Port number  ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               portVal, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.portId= portVal;

    if (pktCmd < 0 || pktCmd > 4)
    {
        printf("pktCmd  ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               portVal, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.pktCmd = pktCmd;

    if (bdVal < 0 || bdVal > 4095)
    {
        printf("BD ID  ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               bdVal, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.bdId= bdVal;

    if (bTagVal < 0 || bTagVal > 4095)
    {
        printf("B-tag val ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               bTagVal, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.bTag = bTagVal;

    if (iSidVal < 0 || iSidVal > 0xFFFFFF)
    {
        printf("I-sid val ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               iSidVal, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.iSID = iSidVal;

    for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
    {
        if (bDa[i] > 255)
        {
            printf("PBB bDa address value  ( 0x%x ) out of range error on line # %d in configuration file \" %s \".\n",
                   bDa[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        newData.bDa[i] = bDa[i];
    }

    for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
    {
        if (bSa[i] > 255)
        {
            printf("PBB bSa address value  ( 0x%x ) out of range error on line # %d in configuration file \" %s \".\n",
                   bSa[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        newData.bSa[i] = bSa[i];
    }

    if (bind2Vlan < 0 || bind2Vlan > 1)
    {
        printf("bind2Vlan ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               bind2Vlan, lineNum, configFile.filename);
        printf("Assummming bind2Vlan 0......");
        bind2Vlan = 0;
        return;
    }
    newData.bind2Vlan = bind2Vlan;
    newData.bindVlan = bindVlan;
    newData.isEndPt = isEndPt;


    addEntry(&configFile.tunnelPBBData, &newData, sizeof(TunnelPBBEntry));
}


/**
 * \fn processTunnelMplsData
 * \private
 * \brief Parses line of text from configuration file to determine TunnelMplsEntry.
 *
 * \param int lineNum
 * \param char* line
 * # mplsIdx   Tag  numOfLables  Label0  Label1    Outer   Port#:      macDA           macSALSBs
 * #          encap                               VLANID  (Egress)
 *
 */
void processTunnelMplsData(int lineNum, char *line)
{
    int vlanVal, portVal, tagType, numOfLables, label0Val, label1Val;
    int errorCode, mplsIdx, entryType, setBd;
    unsigned int macSALSBs;
    int tnlVlan, ipv4RouteEn, ipv6RouteEn, ipv4BridgeMcMode, ipv6BridgeMcMode,
        ipv4RouteMcEn, ipv6RouteMcEn, mplsRouteEn, ipv4RouteMcMode, ipv6RouteMcMode,
        vrfId, unregMcCmd;
    int nhId;
    unsigned int macDA[6];
    int i = 0;
    TunnelMplsEntry newData;

    memset(&newData, 0, sizeof(TunnelMplsEntry));

    errorCode= sscanf(line,
                      "  %d %d %d %d %d %d %d %x:%x:%x:%x:%x:%x %x %d %d %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d %d",
                      &mplsIdx, &tagType, &numOfLables, &label0Val, &label1Val, &vlanVal,
                      &portVal, &macDA[5], &macDA[4], &macDA[3], &macDA[2], &macDA[1], &macDA[0],
                      &macSALSBs, &entryType, &setBd, &tnlVlan, &ipv4RouteEn,
                      &ipv6RouteEn, &vrfId, &ipv4BridgeMcMode, &ipv6BridgeMcMode, &ipv4RouteMcEn,
                      &ipv6RouteMcEn, &ipv4RouteMcMode, &ipv6RouteMcMode, &unregMcCmd, &mplsRouteEn,
                      &nhId);

    if (errorCode != 29)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n\n");
        return;
    }

    for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
    {
        if (macDA[i] > 255)
        {
            printf("macDA address value ( 0X%X ) out of range error on line # %d in configuration file \" %s \".\n",
                   macDA[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        newData.macDA[i]= macDA[i];
    }


    if (portVal < 0 || portVal > 127)
    {
        printf("macDA address value ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               portVal, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.portId= portVal;

    if (vlanVal < 0 || vlanVal > 4095)
    {
        printf("VLAN ID ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               vlanVal, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.vlan= vlanVal;

    if (entryType < 0 || entryType > 1)
    {
        printf("entryType ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               entryType, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.entryType= entryType;

    if (tnlVlan < 0 || tnlVlan > 4096)
    {
        printf(", ERROR:tnlVlan ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               tnlVlan, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (ipv4BridgeMcMode < 0 || ipv4BridgeMcMode > 3)
    {
        printf(", ERROR:ipv4BridgeMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv4BridgeMcMode, lineNum, configFile.filename);
        printf("Assuming ipv4BridgeMcMode is 0...\n");
        ipv4BridgeMcMode = 0;
    }

    if (ipv6BridgeMcMode < 0 || ipv6BridgeMcMode > 3)
    {
        printf(", ERROR:ipv6BridgeMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv6BridgeMcMode, lineNum, configFile.filename);
        printf("Assuming ipv6BridgeMcMode is 0...\n");
        ipv6BridgeMcMode = 0;
    }

    if (ipv4RouteMcEn < 0 || ipv4RouteMcEn > 1)
    {
        printf(", ERROR:ipv4RouteMcEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv4RouteMcMode, lineNum, configFile.filename);
        printf("Assuming ipv4RouteMcEn is 0...\n");
        ipv4RouteMcEn = 0;
    }

    if (ipv6RouteMcEn < 0 || ipv6RouteMcEn > 1)
    {
        printf(", ERROR:ipv6RouteMcEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv6RouteMcEn, lineNum, configFile.filename);
        printf("Assuming ipv6RouteMcEn is 0...\n");
        ipv6RouteMcEn = 0;
    }

    if (ipv4RouteMcMode < 0 || ipv4RouteMcMode > 1)
    {
        printf("ipv4RouteMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv4RouteMcMode, lineNum, configFile.filename);
        printf("Assuming ipv4RouteMcMode is 0...\n");
        ipv4RouteMcMode = 0;
    }

    if (ipv6RouteMcMode < 0 || ipv6RouteMcMode > 1)
    {
        printf("ipv6RouteMcMode ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               ipv6RouteMcMode, lineNum, configFile.filename);
        printf("Assuming ipv6RouteMcMode is 0...\n");
        ipv6RouteMcMode = 0;
    }

    if (mplsRouteEn < 0 || mplsRouteEn > 1)
    {
        printf("mplsRouteEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               mplsRouteEn, lineNum, configFile.filename);
        printf("Assuming mplsRouteEn is 0...\n");
        mplsRouteEn = 0;
    }

    if (unregMcCmd < 0 || unregMcCmd > 1)
    {
        printf("unregMcCmd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               unregMcCmd, lineNum, configFile.filename);
        printf("Assuming unregMcCmd is 0...\n");
        unregMcCmd = 0;
    }

    newData.tagType = tagType;
    newData.numOfLables = numOfLables;
    newData.label0 = label0Val;
    newData.label1 = label1Val;
    newData.setBd = setBd;
    newData.mplsIdx=mplsIdx;
    newData.macSALSBs=macSALSBs;
    newData.tnlVlan = tnlVlan;
    newData.ipv4RouteEn = ipv4RouteEn;;
    newData.ipv6RouteEn = ipv6RouteEn;
    newData.mplsRouteEn = mplsRouteEn;
    newData.ipv4BridgeMcMode = ipv4BridgeMcMode;
    newData.ipv6BridgeMcMode = ipv6BridgeMcMode;
    newData.ipv4RouteMcEn = ipv4RouteMcEn;
    newData.ipv6RouteMcEn = ipv6RouteMcEn;
    newData.ipv4RouteMcMode = ipv4RouteMcMode;
    newData.ipv6RouteMcMode = ipv6RouteMcMode;
    newData.unregMcCmd = unregMcCmd;
    newData.vrfId = vrfId;
    newData.nhId = nhId;
    addEntry(&configFile.tunnelMplsData, &newData, sizeof(TunnelMplsEntry));
}


/**
 * \fn processmplsLabelData
 * \private
 * \brief Parses line of text from configuration file to determine MplsLabelEntry.
 *# Label  EntryFormat    pktCmd  propTTL mplsOper  mplsSwapLabel macDA          macSALSBs    VlanTag     DestPort    encapType  numOfLabel
 * \param int lineNum
 * \param char* line
 */
void processMplsLabelData(int lineNum, char *line)
{
    int    keyMplsLabel, keyEntryFormat, pktCmd, propTTL, egressVif, mplsOper,
           mplsSwapLabel;
    unsigned int CTagVid;
    unsigned int macDA[XP_MAC_ADDR_LEN];
    int errorCode, encapType, numOfLabel, nhId;
    int i = 0;
    MplsLabelEntry newData;

    errorCode= sscanf(line, "%d %d %d %d %d %d %x:%x:%x:%x:%x:%x %x %d %d %d %d",
                      &keyMplsLabel, &keyEntryFormat, &pktCmd, &propTTL, &mplsOper, &mplsSwapLabel,
                      &macDA[5], &macDA[4], &macDA[3], &macDA[2], &macDA[1], &macDA[0],
                      &CTagVid, &egressVif, &encapType, &numOfLabel, &nhId);

    if (errorCode != 17)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s", line);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
    {
        if (macDA[i] > 255)
        {
            printf("macDA address value ( 0x%x ) out of range error on line # %d in configuration file \" %s \".\n",
                   macDA[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        newData.macDA[i]= macDA[i];
    }


    if (CTagVid > 4095)
    {
        printf("CTagVid ( %u ) out of range error on line # %d in configuration file \" %s \".\n",
               CTagVid, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (propTTL < 0 || propTTL > 1)
    {
        printf("propTTL ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               propTTL, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (mplsOper < 0 || mplsOper > 1)
    {
        printf("mplsOper ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               mplsOper, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    newData.keyMplsLabel = keyMplsLabel;
    newData.keyEntryFormat = keyEntryFormat;
    newData.pktCmd = pktCmd;
    newData.mplsOper = mplsOper;
    newData.mplsSwapLabel = mplsSwapLabel;
    newData.CTagVid = CTagVid;
    newData.egressVif = egressVif;
    newData.propTTL = propTTL;
    newData.encapType=encapType;
    newData.numOfLabel= numOfLabel;
    newData.nhId = nhId;

    addEntry(&configFile.mplsLabelData, &newData, sizeof(MplsLabelEntry));
}

/**
 * \fn processPortConfigData
 * \private
 * \brief Parses line of text from configuration file to determine PortConfigEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processPortConfigData(int lineNum, char *line)
{
    int port, portState, acceptedFrameType, disableTunnelVif, disableAclPbr, setBd,
        defaultVlan, privateWireEn, evif, addPortBasedTag, macSaMissCmd, bumPolicerEn;
    int portDefaultPCP, portDefaultDEI, etagExists;
    PortConfigEntry newData;
    int errorCode;

    errorCode= sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &port,
                      &portState, &acceptedFrameType,
                      &disableTunnelVif, &disableAclPbr, &setBd, &defaultVlan, &privateWireEn, &evif,
                      &addPortBasedTag, &macSaMissCmd, &bumPolicerEn,
                      &portDefaultPCP, &portDefaultDEI, &etagExists);
    if (errorCode != 15)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s", line);
        printf("Skipping to next line...\n");
        return;
    }
    if (port < 0 || port > 4096)
    {
        printf("Port number ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               port, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    if (portState < 0 || portState > 3)
    {
        printf("portState ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               portState, lineNum, configFile.filename);
        printf("Assuming portState is 0...");
        portState = 0;
    }
    if (acceptedFrameType < 0 || acceptedFrameType > 2)
    {
        printf("acceptedFrameType ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               acceptedFrameType, lineNum, configFile.filename);
        printf("Assuming acceptedFrameType is 0...");
        acceptedFrameType = 0;
    }
    if (disableTunnelVif < 0 || disableTunnelVif > 1)
    {
        printf("disableTunnelVif ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               disableTunnelVif, lineNum, configFile.filename);
        printf("Assuming disableTunnelVif is 0...");
        disableTunnelVif = 0;
    }
    if (disableAclPbr < 0 || disableAclPbr > 1)
    {
        printf("disableAclPbr ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               disableAclPbr, lineNum, configFile.filename);
        printf("Assuming disableAclPbr is 0...");
        disableAclPbr = 0;
    }
    if (setBd < 0 || setBd > 1)
    {
        printf("setBd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               setBd, lineNum, configFile.filename);
        printf("Assuming setBd is 0...\n");
        setBd = 0;
    }
    if (privateWireEn < 0 || privateWireEn > 1)
    {
        printf("privateWireEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               privateWireEn, lineNum, configFile.filename);
        printf("Assuming privateWireEn is 0...\n");
        privateWireEn = 0;
    }
    if (addPortBasedTag < 0 || addPortBasedTag > 1)
    {
        printf("addPortBasedTag ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               addPortBasedTag, lineNum, configFile.filename);
        printf("Assuming addPortBasedTag is 0...\n");
        addPortBasedTag = 0;
    }
    if (evif < 0 || evif > 4096)
    {
        printf("evif ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               evif, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }
    if (macSaMissCmd < 0 || macSaMissCmd > 3)
    {
        printf("macSaMissCmd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               macSaMissCmd, lineNum, configFile.filename);
        printf("Assuming macSaMissCmd is 1...\n");
        macSaMissCmd = 1;
    }
    if (bumPolicerEn < 0 || bumPolicerEn > 1)
    {
        printf("bumPolicerEn ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               bumPolicerEn, lineNum, configFile.filename);
        printf("Assuming bumPolicerEn is 0...\n");
        bumPolicerEn = 0;
    }
    if (portDefaultPCP < 0 || portDefaultPCP > 7)
    {
        printf("portDefaultPCP ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               portDefaultPCP, lineNum, configFile.filename);
        printf("Assuming portDefaultPCP is 0...\n");
        portDefaultPCP = 0;
    }
    if (portDefaultDEI < 0 || portDefaultDEI > 1)
    {
        printf("portDefaultDEI ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               portDefaultDEI, lineNum, configFile.filename);
        printf("Assuming portDefaultDEI is 0...\n");
        portDefaultDEI = 0;
    }
    if (etagExists < 0 || etagExists > 1)
    {
        printf("etagExists ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               portDefaultDEI, lineNum, configFile.filename);
        printf("Assuming etagExists is 0...\n");
        portDefaultDEI = 0;
    }

    newData.port = port;
    newData.portState = portState;
    newData.acceptedFrameType = acceptedFrameType;
    newData.disableTunnelVif = disableTunnelVif;
    newData.disableAclPbr = disableAclPbr;
    newData.setBd = setBd;
    newData.defaultVlan = defaultVlan;
    newData.privateWireEn = privateWireEn;
    newData.addPortBasedTag = addPortBasedTag;
    newData.evif = evif;
    newData.macSaMissCmd = macSaMissCmd;
    newData.bumPolicerEn = bumPolicerEn;
    newData.portDefaultDEI = portDefaultDEI;
    newData.portDefaultPCP = portDefaultPCP;
    newData.etagExists = etagExists;
    addEntry(&configFile.portConfigData, &newData, sizeof(PortConfigEntry));
}


/**
 * \fn processIpv4BridgeMcData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv4BridgeMcEntry.
 *
 */
void processIpv4BridgeMcData(int lineNum, char *line)
{
    int pktCmd, vlan, mvif, agingFlag, idx;
    int sip[XP_IPV4_ADDR_LEN];
    int grpAddr[XP_IPV4_ADDR_LEN];
    int errorCode;
    char nodeStr[128];
    char* str1 = NULL;
    int i = 0;
    Ipv4BridgeMcEntry newData;

    errorCode= sscanf(line,
                      "%d %d . %d . %d . %d  %d . %d . %d . %d %d %d %d %127s %d", &idx, &sip[3],
                      &sip[2], &sip[1], &sip[0],
                      &grpAddr[3], &grpAddr[2], &grpAddr[1], &grpAddr[0], &pktCmd, &vlan, &mvif,
                      nodeStr, &agingFlag);

    if (errorCode != 14)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s", line);
        printf("Skipping to next line...\n");
        return;
    }

    //Get nodeIdx list
    memset(newData.nodeIdx, 0, sizeof(newData.nodeIdx));
    str1 = strtok(nodeStr, ",");
    while (str1)
    {
        newData.nodeIdx[i++] = atoi(str1);
        str1 = strtok(NULL, ",");
    }

    for (i= 0; i < XP_IPV4_ADDR_LEN; ++i)
    {
        if (sip[i] < 0 || sip[i] > 255)
        {
            printf("Error: IPV4 address value  ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   sip[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.sip[i]= sip[i];
        }

        if (grpAddr[i] < 0 || grpAddr[i] > 255)
        {
            printf("Error: IPV4 address value  ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   grpAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.grpAddr[i]= grpAddr[i];
        }
    }

    if (pktCmd < 0 || pktCmd > 3)
    {
        printf("pktCmd should be between 0 to 3 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming pktCmd is 0...");
        pktCmd = 0;
    }

    if (vlan < 0 || vlan > 4095)
    {
        printf("vlan should be between 0 to 4095 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming vlan is 100...");
        vlan = 100;
    }

    if (mvif < 0)
    {
        printf("mvif should be positive on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    newData.idx = idx;
    newData.pktCmd = pktCmd;
    newData.vlan = vlan;
    newData.mvif = mvif;
    newData.agingFlag = agingFlag;

    addEntry(&configFile.ipv4BridgeMcData, &newData, sizeof(Ipv4BridgeMcEntry));
}

/**
 * \fn processIpv6BridgeMcData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv6BridgeMcEntry.
 *
 */
void processIpv6BridgeMcData(int lineNum, char *line)
{
    int pktCmd, vlan, mvif, idx;
    unsigned int sip[XP_IPV6_ADDR_LEN];
    unsigned int grpAddr[XP_IPV6_ADDR_LEN];
    int errorCode;
    char nodeStr[128];
    char* str1 = NULL;
    int i = 0;
    Ipv6BridgeMcEntry newData;

    errorCode= sscanf(line,
                      " %d %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x  %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x %d %d %d %127s",
                      &idx, &sip[15], &sip[14], &sip[13], &sip[12], &sip[11], &sip[10], &sip[9],
                      &sip[8], &sip[7], &sip[6], &sip[5], &sip[4], &sip[3], &sip[2], &sip[1], &sip[0],
                      &grpAddr[15], &grpAddr[14], &grpAddr[13], &grpAddr[12], &grpAddr[11],
                      &grpAddr[10], &grpAddr[9], &grpAddr[8], &grpAddr[7], &grpAddr[6], &grpAddr[5],
                      &grpAddr[4], &grpAddr[3], &grpAddr[2], &grpAddr[1], &grpAddr[0], &pktCmd, &vlan,
                      &mvif, nodeStr);

    if (errorCode != 37)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s", line);
        printf("Skipping to next line...\n");
        return;
    }
    //Get nodeIdx list
    memset(newData.nodeIdx, 0, sizeof(newData.nodeIdx));
    str1 = strtok(nodeStr, ",");
    while (str1)
    {
        newData.nodeIdx[i++] = atoi(str1);
        str1 = strtok(NULL, ",");
    }

    for (i= 0; i < XP_IPV6_ADDR_LEN; ++i)
    {
        if (sip[i] > 0xFF)
        {
            printf("Error: IPV6 address value  ( 0x%x ) out of range error on line # %d in configuration file \" %s \".\n",
                   sip[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.sip[i]= sip[i];
        }

        if (grpAddr[i] > 0xFF)
        {
            printf("Error: IPV6 address value  ( 0x%x ) out of range error on line # %d in configuration file \" %s \".\n",
                   grpAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.grpAddr[i]= grpAddr[i];
        }
    }

    if (pktCmd < 0 || pktCmd > 3)
    {
        printf("pktCmd should be between 0 to 3 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming pktCmd is 0...");
        pktCmd = 0;
    }

    if (vlan < 0 || vlan > 4095)
    {
        printf("vlan should be between 0 to 4095 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming vlan is 100...");
        vlan = 100;
    }

    if (mvif < 0)
    {
        printf("mvif should be positive on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    newData.idx = idx;
    newData.pktCmd = pktCmd;
    newData.vlan = vlan;
    newData.mvif = mvif;

    addEntry(&configFile.ipv6BridgeMcData, &newData, sizeof(Ipv6BridgeMcEntry));

}

/**
 * \fn processIpv4PimBiDirData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv4PimBiDirEntry.
 *
 */
void processIpv4PimBiDirData(int lineNum, char *line)
{
    int vrfId, bdId, rpfValue;
    int grpAddr[XP_IPV4_ADDR_LEN];
    int errorCode;
    int i = 0;
    Ipv4PimBiDirEntry newData;

    errorCode= sscanf(line, "%d . %d . %d . %d  %d %d %d",
                      &grpAddr[3], &grpAddr[2], &grpAddr[1], &grpAddr[0], &vrfId, &bdId, &rpfValue);

    if (errorCode != 7)
    {
        printf("Error: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s", line);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_IPV4_ADDR_LEN; ++i)
    {
        if (grpAddr[i] < 0 || grpAddr[i] > 255)
        {
            printf("Error: IPV4 group address value  ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   grpAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.grpAddr[i]= grpAddr[i];
        }
    }

    if (vrfId < 0 || vrfId > 4095)
    {
        printf("vrfId should be between 0 to 4095 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming vrfId is 0...");
        vrfId = 0;
    }

    if (bdId < 0 || bdId > 4095)
    {
        printf("bdId should be between 0 to 4095 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming bdId is 0...");
        bdId = 0;
    }

    newData.vrfId = vrfId;
    newData.bdId = bdId;
    newData.rpfValue = rpfValue;

    addEntry(&configFile.ipv4PimBiDirData, &newData, sizeof(Ipv4PimBiDirEntry));
}

/**
 * \fn processIpv4RouteMcData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv4RouteMcEntry.
 *
 */
void processIpv4RouteMcData(int lineNum, char *line)
{
    int pktCmd, mvif, vrfId, rpfType, rpfValue, rpfFailCmd, vlanForIvifRpf,
        agingFlag, idx, isMartialIp, reasonCode;
    int sip[XP_IPV4_ADDR_LEN];
    int grpAddr[XP_IPV4_ADDR_LEN];
    int errorCode;
    int i = 0, j;
    char nodeStr[128];
    char *str1 = NULL, *str2 = NULL, *str3 = NULL;
    char* token = NULL;
    char* subToken = NULL;
    char* subSubToken = NULL;
    Ipv4RouteMcEntry newData;

    errorCode= sscanf(line,
                      "%d %d . %d . %d . %d  %d . %d . %d . %d %d %d %d %d %d %d %d %127s %d %d %d",
                      &idx, &sip[3], &sip[2], &sip[1], &sip[0], &grpAddr[3], &grpAddr[2], &grpAddr[1],
                      &grpAddr[0], &pktCmd, &vrfId, &mvif, &rpfType, &rpfValue, &rpfFailCmd,
                      &vlanForIvifRpf, nodeStr, &agingFlag, &isMartialIp, &reasonCode);

    if (errorCode != 20)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s", line);
        printf("Skipping to next line...\n");
        return;
    }

    //Get nodeIdx list
    memset(newData.vlanNode, 0, sizeof(newData.vlanNode));
    token = strtok_r(nodeStr, "|", &str1);
    while (token)
    {
        subToken = strtok_r(token, ":", &str2);
        if (!subToken)
        {
            printf("ERROR: Invalid Input\n");
            return;
        }
        newData.vlanNode[i].vlanId = atoi(subToken);
        subToken = strtok_r(NULL, ":", &str2);

        //printf("DBG: newData.vlanNode[%d].vlanId = %d, newData.vlanNode[%d].nodeStr = %s\n", i, newData.vlanNode[i].vlanId, i, subToken);
        j = 0;
        subSubToken = strtok_r(subToken, ",", &str3);
        while (subSubToken)
        {
            newData.vlanNode[i].nodeIdx[j++] = atoi(subSubToken);
            subSubToken = strtok_r(NULL, ",", &str3);
        }
        i++;

        token = strtok_r(NULL, "|", &str1);
    }

    for (i= 0; i < XP_IPV4_ADDR_LEN; ++i)
    {
        if (sip[i] < 0 || sip[i] > 255)
        {
            printf("Error: IPV4 Source address value  ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   sip[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.sip[i]= sip[i];
        }

        if (grpAddr[i] < 0 || grpAddr[i] > 255)
        {
            printf("Error: IPV4 group address value  ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   grpAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.grpAddr[i]= grpAddr[i];
        }
    }

    if (pktCmd < 0 || pktCmd > 3)
    {
        printf("pktCmd should be between 0 to 3 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming pktCmd is 0...");
        pktCmd = 0;
    }

    if (isMartialIp && pktCmd)
    {
        pktCmd = 2;    /*Since pktCmd value can be either drop (i.e. 0)  or trap to cpu (i.e. 2)*/
    }

    if (mvif < 0 && isMartialIp <= 0)
    {
        printf("mvif should be positive on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (reasonCode < 0 && isMartialIp <= 0)
    {
        printf("reasonCode should be positive on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }


    newData.idx = idx;
    newData.pktCmd = pktCmd;
    newData.vrfId = vrfId;
    newData.mvif = mvif;
    newData.rpfType = rpfType;
    newData.rpfValue = rpfValue;
    newData.rpfFailCmd = rpfFailCmd;
    newData.vlanForIvifRpf = vlanForIvifRpf;
    newData.agingFlag = agingFlag;
    newData.isMartialIp = isMartialIp;
    newData.reasonCode = reasonCode;

    addEntry(&configFile.ipv4RouteMcData, &newData, sizeof(Ipv4RouteMcEntry));
}

/**
 * \fn processIpv6RouteMcData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv6RouteMcEntry.
 *
 */
void processIpv6RouteMcData(int lineNum, char *line)
{
    int pktCmd, mvif, vrfId, rpfType, rpfValue, rpfFailCmd, vlanForIvifRpf, idx,
        isMartialIp, reasonCode;
    unsigned int sip[XP_IPV6_ADDR_LEN];
    unsigned int grpAddr[XP_IPV6_ADDR_LEN];
    int errorCode;
    int i = 0, j;
    char nodeStr[128];
    char *str1 = NULL, *str2 = NULL, *str3 = NULL;
    char* token = NULL;
    char* subToken = NULL;
    char* subSubToken = NULL;
    Ipv6RouteMcEntry newData;

    errorCode= sscanf(line,
                      "%d %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x %d %d %d %d %d %d %d %127s %d %d",
                      &idx, &sip[15], &sip[14], &sip[13], &sip[12], &sip[11], &sip[10], &sip[9],
                      &sip[8], &sip[7], &sip[6], &sip[5], &sip[4], &sip[3], &sip[2], &sip[1], &sip[0],
                      &grpAddr[15], &grpAddr[14], &grpAddr[13], &grpAddr[12], &grpAddr[11],
                      &grpAddr[10], &grpAddr[9], &grpAddr[8], &grpAddr[7], &grpAddr[6], &grpAddr[5],
                      &grpAddr[4], &grpAddr[3], &grpAddr[2], &grpAddr[1], &grpAddr[0], &pktCmd,
                      &vrfId, &mvif, &rpfType, &rpfValue, &rpfFailCmd, &vlanForIvifRpf, nodeStr,
                      &isMartialIp, &reasonCode);

    if (errorCode != 43)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s", line);
        printf("Skipping to next line...\n");
        return;
    }

    //Get nodeIdx list
    memset(newData.vlanNode, 0, sizeof(newData.vlanNode));
    token = strtok_r(nodeStr, "|", &str1);
    while (token)
    {
        subToken = strtok_r(token, ":", &str2);
        if (!subToken)
        {
            printf("ERROR: Invalid Input\n");
            return;
        }
        newData.vlanNode[i].vlanId = atoi(subToken);
        subToken = strtok_r(NULL, ":", &str2);

        //printf("DBG: newData.vlanNode[%d].vlanId = %d, newData.vlanNode[%d].nodeStr = %s\n", i, newData.vlanNode[i].vlanId, i, subToken);
        j = 0;
        subSubToken = strtok_r(subToken, ",", &str3);
        while (subSubToken)
        {
            newData.vlanNode[i].nodeIdx[j++] = atoi(subSubToken);
            subSubToken = strtok_r(NULL, ",", &str3);
        }
        i++;

        token = strtok_r(NULL, "|", &str1);
    }

    for (i= 0; i < XP_IPV6_ADDR_LEN; ++i)
    {
        if (sip[i] > 0xFF)
        {
            printf("Error: IPV6 Source address value  ( 0x%x ) out of range error on line # %d in configuration file \" %s \".\n",
                   sip[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.sip[i]= sip[i];
        }

        if (grpAddr[i] > 0xFF)
        {
            printf("Error: IPV6 group address value  ( 0x%x ) out of range error on line # %d in configuration file \" %s \".\n",
                   grpAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.grpAddr[i]= grpAddr[i];
        }
    }

    if (pktCmd < 0 || pktCmd > 3)
    {
        printf("pktCmd should be between 0 to 3 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming pktCmd is 0...");
        pktCmd = 0;
    }

    if (isMartialIp && pktCmd)
    {
        pktCmd = 2;    /*Since pktCmd value can be either drop (i.e. 0)  or trap to cpu (i.e. 2)*/
    }

    if (mvif < 0 && isMartialIp <= 0)
    {
        printf("mvif should be positive on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (reasonCode < 0 && isMartialIp <= 0)
    {
        printf("reasonCode should be positive on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    newData.idx = idx;
    newData.pktCmd = pktCmd;
    newData.vrfId = vrfId;
    newData.mvif = mvif;
    newData.rpfType = rpfType;
    newData.rpfValue = rpfValue;
    newData.rpfFailCmd = rpfFailCmd;
    newData.vlanForIvifRpf = vlanForIvifRpf;
    newData.isMartialIp = isMartialIp;
    newData.reasonCode = reasonCode;


    addEntry(&configFile.ipv6RouteMcData, &newData, sizeof(Ipv6RouteMcEntry));
}

/**
 * \fn processL2MdtNodeData
 * \public
 * \brief validate input data for l2MdtEntry entry
 */
int processL2MdtNodeData(int lineNum, char *line)
{
    int mdtIdx, nxtL2Ptr, mvifIdx, mirrorSessionId, nextEngine, isLast, encap,
        cVlan, tnlData, disableVlanTranslation;
    int errorCode;
    l2MdtEntry mdtNode;

    errorCode = sscanf(line, "%d %d %d %d %d %d %d %d %d %d", &mdtIdx, &nxtL2Ptr,
                       &mvifIdx, &mirrorSessionId, &nextEngine, &isLast, &encap, &cVlan, &tnlData,
                       &disableVlanTranslation);

    if (errorCode != 10)
    {
        printf("Error: Unable to parse l2 mdt node format # %s in configuration file %s\n",
               line, configFile.filename);
        return 0;
    }

    if (encap < 0 ||  encap > 11)
    {
        printf("encap should be between 0 to 10 for l2 mdt node # %s in configuration file %s\n",
               line, configFile.filename);
        printf("Assuming encap is 0\n");
        encap = 0;
    }

    if (cVlan < 0 ||  cVlan > 4095)
    {
        printf("cVlan should be between 0 to 4095 for l2 mdt node # %s in configuration file %s\n",
               line, configFile.filename);
        printf("Assuming cVlan is 0\n");
        cVlan = 0;
    }

    if (mvifIdx < 0)
    {
        printf("Error: mvifIdx should be positive for l2 mdt node # %s in configuration file %s\n",
               line, configFile.filename);
        return 0;
    }

    mdtNode.encapData = 0;
    switch (encap)
    {
        case 0:
        case 2:
            break;
        case 1:
        case 3:
            mdtNode.encapData = cVlan;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 10:
            mdtNode.encapData = tnlData;
            break;
        default:
            printf("Error: Invalid Encap Data # %s in configuration file %s\n", line,
                   configFile.filename);
            return 0;
    }

    mdtNode.index = mdtIdx;
    mdtNode.nxtL2Ptr = nxtL2Ptr;
    mdtNode.mvifIndex = mvifIdx;
    mdtNode.mirrorSessionId = mirrorSessionId;
    mdtNode.nextEngine = nextEngine;
    mdtNode.isLast = isLast;
    mdtNode.encap = encap;
    mdtNode.disableVlanTranslation = !!(disableVlanTranslation);

    addEntry(&configFile.l2OIFEntry, &mdtNode, sizeof(l2MdtEntry));

    return 1;
}

/**
 * \fn processL3MdtNodeData
 * \public
 * \brief validate input data for l3MdtEntry entry
 */
int processL3MdtNodeData(int lineNum, char *line)
{
    int mdtIdx, nxtL2Ptr, nxtL3Ptr, altVif, mvifIdx, mirrorSessionId, nextEngine,
        isLast, encap, cVlan, tnlData;
    int errorCode;
    l3MdtEntry mdtNode;

    memset(&mdtNode, 0x00, sizeof(mdtNode));

    errorCode = sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d", &mdtIdx, &nxtL3Ptr,
                       &nxtL2Ptr, &mvifIdx, &mirrorSessionId, &nextEngine, &altVif, &isLast, &encap,
                       &cVlan, &tnlData);

    if (errorCode != 11)
    {
        printf("Error: Unable to parse l3 mdt node format # %s in configuration file %s\n",
               line, configFile.filename);
        return 0;
    }

    if (encap < 0 ||  encap > 11)
    {
        printf("encap should be between 0 to 10 for l3 mdt node # %s in configuration file %s\n",
               line, configFile.filename);
        printf("Assuming encap is 0\n");
        encap = 0;
    }

    if (cVlan < 0 ||  cVlan > 4095)
    {
        printf("cVlan should be between 0 to 4095 for l3 mdt node # %s in configuration file %s\n",
               line, configFile.filename);
        printf("Assuming cVlan is 0\n");
        cVlan = 0;
    }

    if (mvifIdx < 0)
    {
        printf("Error: mvifIdx should be positive for l3 mdt node # %s in configuration file %s\n",
               line, configFile.filename);
        return 0;
    }

    switch (encap)
    {
        case 0:
        case 2:
            break;
        case 1:
        case 3:
            mdtNode.encapData = cVlan;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 10:
            mdtNode.encapData = tnlData;
            break;
        default:
            printf("Error: Invalid Encap Data # %s in configuration file %s\n", line,
                   configFile.filename);
            return 0;
    }


    mdtNode.index = mdtIdx;
    mdtNode.nxtL2Ptr = nxtL2Ptr;
    mdtNode.nxtL3Ptr = nxtL3Ptr;
    mdtNode.altVif = altVif;
    mdtNode.mvifIndex = mvifIdx;
    mdtNode.mirrorSessionId = mirrorSessionId;
    mdtNode.nextEngine = nextEngine;
    mdtNode.isLast = isLast;
    mdtNode.encap = encap;
    mdtNode.cVlan = cVlan;

    addEntry(&configFile.l3OIFEntry, &mdtNode, sizeof(l3MdtEntry));

    return 1;
}

/**
 * \fn processMvifData
 * \private
 * \brief Parses line of text from configuration file to determine MvifEntry.
 *
 */
void processMvifData(int lineNum, char *line)
{
    int mdtRootIndex;
    int errorCode, index, vifType, tnlType, tnlIdx;
    int port;
    xpPort_t portList[XP_MAX_TOTAL_PORTS]= {0};
    int tempInc = 0, bufInc = 0;
    char *str1 = NULL;
    MvifEntry newData;
    newData.numPorts = 0;
    char temp[50];
    char buf[5];

    errorCode= sscanf(line, "%d %d %49s %d %d %d", &index, &mdtRootIndex, temp,
                      &vifType, &tnlType, &tnlIdx);
    if (errorCode != 6)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    str1 = strstr(temp, "null");

    if (str1 == NULL && mdtRootIndex >= 0)
    {
        printf("Invalid entry of mvif table on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Assuming mdtRootIndex -1");
        mdtRootIndex = -1;
    }
    else if (str1 != NULL && mdtRootIndex < 0)
    {
        printf("Error: Invalid entry of mvif table on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    if (mdtRootIndex < 0)
    {
        while (temp[tempInc]!='\0')
        {
            if (isdigit(temp[tempInc]))
            {
                buf[bufInc]=temp[tempInc];
                bufInc++;
            }
            else if (temp[tempInc]==',')
            {
                buf[bufInc]='\0';
                port = atoi(buf);
                if (port < 0 || port > 136)
                {
                    printf("Error: port should be between 0 to 127 on line # %d in configuration file \" %s \".\n",
                           lineNum, configFile.filename);
                    printf("Skipping to next line...\n");
                    return;
                }
                if (newData.numPorts < XP_MAX_TOTAL_PORTS)
                {
                    portList[newData.numPorts++]= port ;
                }
                bufInc = 0;
            }
            else
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
                       lineNum, configFile.filename);
                printf("%s", line);
                printf("Skipping to next line...\n");
                return;
            }
            tempInc++;
        }
        buf[bufInc]='\0';
        port = atoi(buf);
        if (port < 0 ||  port > 136)
        {
            printf("Error: port should be between 0 to 127 on line # %d in configuration file \" %s \".\n",
                   lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        if (newData.numPorts < XP_MAX_TOTAL_PORTS)
        {
            portList[newData.numPorts++]= port ;
        }
    }

    newData.index = index;
    newData.vifType = vifType;
    newData.tnlType = tnlType;
    newData.tnlIdx = tnlIdx;
    newData.mdtRootIndex = mdtRootIndex;
    memcpy(newData.portList, portList, sizeof(xpPort_t)*newData.numPorts);
    addEntry(&configFile.mvifData, &newData, sizeof(MvifEntry));
}

/**
 * \fn processVrfData
 * \private
 * \brief Parses line of text from configuration file to determine vrfEntry.
 *
 */
void processVrfData(int lineNum, char *line)
{
    int errorCode, vrfIndex, v4En, v6En;
    VrfEntry newData;

    errorCode= sscanf(line, "%d %d %d", &vrfIndex, &v4En, &v6En);
    if (errorCode != 3)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    newData.vrfIdx    = vrfIndex;
    newData.v4RouteEn = v4En;
    newData.v6RouteEn = v6En;
    addEntry(&configFile.vrfData, &newData, sizeof(VrfEntry));
}

/**
 * \fn processMirrorData
 * \private
 * \brief Parses line of text from configuration file to determine xpConfigFile::MirrrorEntry.
 *
 */
void processMirrorData(int lineNum, char *line)
{

    char* token;
    MirrorSource newSrc = { NUM_TYPES, {"PORT", "VLAN", "LAG"}, {0}}; //TODO:need to verify default values
    //mirrorData newData = {0,0};
    MirrorEntry* newEntry = NULL;
    xpPort_t port;
    token= strtok(line, " \t");
    if (token == NULL)
    {
        printf("No mirroring session ID found on line #%d in configuration file \"%s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n\n");
        return;
    }

    newEntry = (MirrorEntry*)malloc(sizeof(MirrorEntry));
    if (!newEntry)
    {
        printf("Not enough memory to allocate MirrorEntry object\n");
        return;
    }
    memset(newEntry, 0, sizeof(MirrorEntry));

    // The first field is the session ID:
    newEntry->sessionId= atoi(token);
    if (!XP_MIRROR_IS_USER_SESSION((int32_t)
                                   newEntry->sessionId))        // If not valid session ID:
    {
        printf("No Session ID found on line #%d in configuration file \"%s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        free(newEntry);
        return;
    }

    token= strtok(NULL, " \t");
    if (token == NULL)
    {
        printf("Invalid Input.\n");
        free(newEntry);
        return;
    }
    newEntry->bucketId = atoi(token);
    if (!XP_MIRROR_IS_USER_SESSION((int32_t)
                                   newEntry->bucketId))        // If not valid session ID:
    {
        printf("No Session ID found on line #%d in configuration file \"%s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        free(newEntry);
        return;
    }

    token= strtok(NULL, " \t");
    if (token == NULL)
    {
        printf("Invalid Input.\n");
        free(newEntry);
        return;
    }
    newEntry->mirrorType= atoi(token);

    token= strtok(NULL, " \t");
    if (token == NULL)
    {
        printf("Invalid Input.\n");
        free(newEntry);
        return;
    }
    newEntry->dir = atoi(token);
    // Read in a sequence of 1 or more port IDs until we find ":".  The port IDs will
    // eventually be mapped to analyzer VIFs.
    //newEntry->numPorts = 0;

    token= strtok(NULL, " \t");
    while (token != NULL && strcmp(token, ":"))        // If the token is NOT ":".
    {
        if (newEntry->mirrorType == 0)
        {
            port= (xpPort_t) atoi(token);
            newEntry->data.mirrorData.analyzers[newEntry->numAnalyzers++] =
                port;            // Add port ID to vector.
            token= strtok(NULL, " \t");
        }
        else if (newEntry->mirrorType == 1)
        {
            newEntry->data.erspanData.erspanSessionId = atoi(token);
            token= strtok(NULL, " ");
            if (token == NULL)
            {
                printf("Invalid Input.\n");
                free(newEntry);
                return;
            }
            newEntry->data.erspanData.tunnelTableIndex = atoi(token);
            newEntry->numAnalyzers++;
            token= strtok(NULL, " \t");
        }
        else if (newEntry->mirrorType == 2)
        {
            printf("Rspan :parsing newEntry.mirrorType :%d\n", newEntry->mirrorType);
            /*  port= (xpPort_t) atoi(token);
              printf("DP_PRINT for Rspan :port  :%d\n",port);
              newEntry.data.mirrorData.analyzers[newEntry.numAnalyzers++] = port;            // Add port ID to vector.
              printf("DP_PRINT for Rspan :port final  :%d\n",port); */
            newEntry->data.erspanData.erspanSessionId = atoi(token);

            newEntry->numAnalyzers++;
            token= strtok(NULL, " \t");
        }
    }

    // If we read no analyzer (port) IDs:
    if (token == NULL || strcmp(token, ":") ||
        newEntry->numAnalyzers == 0)        // If the token is NOT ":".
    {
        printf("No analyzers found on line #%d in configuration file \"%s \".\n",
               lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        free(newEntry);
        return;
    }

    // We've read the ":" separator.  Now read in pairs of source type & source ID:

    newEntry->numSources = 0;
    token= strtok(NULL, " \t");
    while (token != NULL && strcmp(token, "NULL\n") != 0)
    {
        if (setMirrorSourceType(&newSrc,
                                token) != XP_NO_ERR)        // If we didn't find a valid source type:
        {
            printf("Unrecognized mirror source type found on  line #%d in configuration file \"%s \".\n",
                   lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            free(newEntry);
            return;
        }

        token= strtok(NULL, " \t");
        if (token == NULL ||
            setMirrorSourceId(&newSrc,
                              token) != XP_NO_ERR)        // If we didn't find a valid source ID:
        {
            printf("Bad mirror source ID found on line #%d in configuration file \"%s \".\n",
                   lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            free(newEntry);
            return;
        }

        memcpy(&newEntry->sources[newEntry->numSources++], &newSrc,
               sizeof(MirrorSource));        // Valid source found:
        token= strtok(NULL, " \t");
    }

    //if (newEntry.numSources == 0)            // If we read no sources:
    //{
    //    printf("No mirror sources found on line #%d in configuration file \"%s \".\n", lineNum, configFile.filename);
    //    printf("Skipping to next line...\n" );
    //    return;
    //}

    addEntry(&configFile.mirrorData, newEntry, sizeof(MirrorEntry));
    free(newEntry);
}


/**
 *  * \fn processMacSAMSBs
 *   * \private
 *    * \brief Parses line of text from configuration file to determine macSAMSBs.
 *     *
 *      */


void processMacSAMSBs(int lineNum, char *line)
{

    int errorCode;
    unsigned int macSA[5];
    int i = 0;

    errorCode= sscanf(line, "%x:%x:%x:%x:%x", &macSA[4], &macSA[3], &macSA[2],
                      &macSA[1], &macSA[0]);
    if (errorCode != 5)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < 5; ++i)
    {
        if (macSA[i] > 255)
        {
            printf("mac High SA address value ( 0x%x  ) out of range error on line# %d in configuration file \" %s \".\n",
                   macSA[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        configFile.macSAMSBs[i]=macSA[i];
    }
}


/**
 * \fn processNatData
 * \private
 * \brief Parses line of text from configuration file to determine xpConfigFile::NatEntry
 *
 * \param int lineNum
 * \param char* line
 */
void processNatData(int lineNum, char *line)
{
    int keySrcAddress[XP_IPV4_ADDR_LEN], keyDestAddress[XP_IPV4_ADDR_LEN],
        sipAddress[XP_IPV4_ADDR_LEN], dipAddress[XP_IPV4_ADDR_LEN];
    int keySrcPort, keyDestPort, bd, flag, protocol, pktCmd, srcPort, destPort,
        port, mdtNatConfig, natMode, natScope;
    int errorCode = 0;
    NatEntry newEntry;

    errorCode= sscanf(line, "%d . %d . %d . %d %d %d . %d . %d . %d %d %d %d %d"
                      " %d  %d . %d . %d . %d  %d %d . %d . %d . %d %d %d %d %d %d",
                      &keySrcAddress[0], &keySrcAddress[1], &keySrcAddress[2], &keySrcAddress[3],
                      &keySrcPort, &keyDestAddress[0],
                      &keyDestAddress[1], &keyDestAddress[2], &keyDestAddress[3], &keyDestPort, &bd,
                      &flag, &protocol,
                      &pktCmd, &sipAddress[0], &sipAddress[1], &sipAddress[2], &sipAddress[3],
                      &srcPort, &dipAddress[0],
                      &dipAddress[1], &dipAddress[2], &dipAddress[3], &destPort, &port, &mdtNatConfig,
                      &natMode, &natScope);

    if (errorCode != 4*XP_IPV4_ADDR_LEN + 12)
    {
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }
    int i;
    for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
    {
        if (keySrcAddress[i] > 255)
        {
            printf("keySrcAddress ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   keySrcAddress[i], lineNum, configFile.filename);
            return;
        }
        newEntry.keySrcAddress[i] = keySrcAddress[i];
    }

    if (keySrcPort < 0 || keySrcPort > 65535)
    {
        printf("keySrcPort ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               keySrcPort, lineNum, configFile.filename);
        return;
    }
    newEntry.keySrcPort = keySrcPort;
    for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
    {
        if (keyDestAddress[i] > 255)
        {
            printf("keyDestAddress ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   keyDestAddress[i], lineNum, configFile.filename);
            return;
        }
        newEntry.keyDestAddress[i] = keyDestAddress[i];
    }

    if (keyDestPort < 0 || keyDestPort > 65535)
    {
        printf("keyDestPort ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               keyDestPort, lineNum, configFile.filename);
        return;
    }
    newEntry.keyDestPort = keyDestPort;

    if (bd < 0 || bd > 4095)
    {
        printf("bd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               bd, lineNum, configFile.filename);
        return;
    }
    newEntry.bd = bd;

    if (natMode < 0 || natMode > 3)
    {
        printf("natMode should be 0, 1 or 2 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        return;
    }
    newEntry.natMode = natMode;

    if (natScope < 0 ||
        natScope > 2) //TODO for others values may need to be updated
    {
        printf("natScope should be 1 or 2 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        return;
    }
    newEntry.natScope = natScope;

    newEntry.flag = flag;

    newEntry.protocol = protocol;

    if (pktCmd < 0 || pktCmd > 3)
    {
        printf("pktCmd ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               pktCmd, lineNum, configFile.filename);
        return;
    }
    newEntry.pktCmd = pktCmd;
    for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
    {
        if (sipAddress[i] > 255)
        {
            printf("sipAddress ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   sipAddress[i], lineNum, configFile.filename);
            return;
        }
        newEntry.sipAddress[i] = sipAddress[i];
    }

    if (srcPort < 0 || srcPort > 65535)
    {
        printf("srcPort ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               srcPort, lineNum, configFile.filename);
        return;
    }
    newEntry.srcPort = srcPort;
    for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
    {
        if (dipAddress[i] > 255)
        {
            printf("dipAddress ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   dipAddress[i], lineNum, configFile.filename);
            return;
        }
        newEntry.dipAddress[i] = dipAddress[i];
    }

    if (destPort < 0 || destPort > 65535)
    {
        printf("destPort ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               destPort, lineNum, configFile.filename);
        return;
    }
    newEntry.destPort = destPort;

    if (port < 0 || port > 136)
    {
        printf("port ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               port, lineNum, configFile.filename);
        return;
    }
    newEntry.port = port;

    if (mdtNatConfig > 2)
    {
        printf("mdtNatConfig ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               port, lineNum, configFile.filename);
        return;
    }
    newEntry.mdtNatConfig = mdtNatConfig;

    addEntry(&configFile.natData, &newEntry, sizeof(NatEntry));
}


/**
 * \fn processL2EaclData
 * \private
 * \brief Parses line of text from configuration file to determine xpConfigFile::EaclEntry
 *
 * \param int lineNum
 * \param char* line
 */

void processL2EaclData(int lineNum, char *line)
{
    int aclId, egressVif, eRif, eVlan, rsnCode, enPktCmdUpd, enRsnCodeUpd,
        l4DestPort, l4SourcePort;
    int icmpMessageType, dscp, exp, pcp, type, tcpFlags, vi, reasonCode, priority;
    unsigned int pktCmd, etherType, protocol;
    unsigned int  macDa[6], macSa[6], dip[16], sip[16];
    char  keyAttribute[300] = {0};
    char *str1, *str2, *str3, *str4;
    char key[50], data[50], mask[1];
    EaclL2KeyAttribute eaclKey = {-1, -1, {0,}, {0,}, -1, {0,}, {0,}, {0,}, {0,}, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0 };
    EaclL2MaskAttribute eaclMask;
    EaclData eaclData;

    memset(key, 0, sizeof(key));
    memset(data, 0, sizeof(data));
    memset(mask, 0, sizeof(mask));
    memset(&eaclMask, 0xFF, sizeof(EaclL2MaskAttribute));
    memset(&eaclData, 0, sizeof(EaclData));

    sscanf(line, "%d %d,%d %x %d %d %299s",
           &aclId, &enPktCmdUpd, &enRsnCodeUpd, &pktCmd, &rsnCode, &priority,
           keyAttribute);



    if (enPktCmdUpd < 0 || enPktCmdUpd > 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter enPktCmdUpd data in valid format\n");
        return;
    }
    eaclData.enPktCmdUpd = enPktCmdUpd;

    if (enRsnCodeUpd< 0 || enRsnCodeUpd> 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter enRsnCodeUpd data in valid format\n");
        return;
    }
    eaclData.enRsnCodeUpd = enRsnCodeUpd;

    if (pktCmd > 3)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter pktCmd data in valid format\n");
        return;
    }
    eaclData.pktCmd = pktCmd;

    if (rsnCode < 0 || rsnCode > 1023)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter rsnCode data in valid format\n");
        return;
    }
    eaclData.rsnCode = rsnCode;

    eaclKey.eAclId = aclId;
    eaclKey.priority = priority;
    //eaclKey.egressVif = egressVif;
    //eaclKey.eVlan = eVlan;

    str1 = keyAttribute;
    str2 = strtok(str1, ",");

    while (str2 != NULL)
    {
        str3 = strstr(str2, "->");
        if (str3)
        {
            strncpy(key, str2, str3-str2);
            str4 = strstr(str2, "|");
            if (str4)
            {
                strncpy(data, str3 +2, (str4-str3 - 2));
                mask[0]=str4[1];
            }
            else
            {
                printf("ERROR: Invalid input for keyAttribute.\n");
                return;
            }
        }
        else
        {
            printf("ERROR: Invalid input.\n");
            return;
        }

        if (strcmp(key, "macDA") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x", &macDa[5], &macDa[4], &macDa[3], &macDa[2],
                   &macDa[1], &macDa[0]);
            int i;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if (macDa[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter macDA data in valid format\n");
                    return;
                }
                eaclKey.macDa[i]= macDa[i];
                if (mask[0] == '0')
                {
                    eaclMask.macDa[i]= 0;
                }

            }
            eaclKey.isDmacSet = 1;
        }
        else if (strcmp(key, "macSA") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x", &macSa[5], &macSa[4], &macSa[3], &macSa[2],
                   &macSa[1], &macSa[0]);
            int i;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if (macSa[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter macSA data in valid format\n");
                    return;
                }
                eaclKey.macSa[i]= macSa[i];
                if (mask[0] == '0')
                {
                    eaclMask.macSa[i]= 0;
                }

            }
            eaclKey.isSmacSet = 1;
        }
        else if (strcmp(key, "etherType") == 0)
        {
            sscanf(data, "%x", &etherType);
            if (etherType > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter etherType data in valid format\n");
                return;
            }
            eaclKey.etherType = etherType;
            if (mask[0] == '0')
            {
                eaclMask.etherType = 0;
            }

        }
        else if (strcmp(key, "type") == 0)
        {
            sscanf(data, "%d", &type);
            if (type < 0 || type > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter type data in valid format\n");
                return;
            }
            eaclKey.type = type;
            if (mask[0] == '0')
            {
                eaclMask.type = 0;
            }
        }

        else if (strcmp(key, "reasonCode") == 0)
        {
            sscanf(data, "%d", &reasonCode);
            if (reasonCode < 0 || reasonCode > 512)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter reasonCode data in valid format\n");
                return;
            }
            eaclKey.reasonCode = reasonCode;
            if (mask[0] == '0')
            {
                eaclMask.reasonCode = 0;
            }
        }
        else if (strcmp(key, "eVlan") == 0)
        {
            sscanf(data, "%d", &eVlan);
            if (eVlan < 0 || eVlan > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter eVlan data in valid format\n");
                return;
            }
            eaclKey.eVlan = eVlan;
            if (mask[0] == '0')
            {
                eaclMask.eVlan = 0;
            }
        }
        else if (strcmp(key, "egressVif") == 0)
        {
            sscanf(data, "%d", &egressVif);
            if (egressVif < 0 || egressVif > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter egressVif data in valid format\n");
                return;
            }
            eaclKey.egressVif = egressVif;
            if (mask[0] == '0')
            {
                eaclMask.egressVif = 0;
            }
        }
        else if (strcmp(key, "eRif") == 0)
        {
            sscanf(data, "%d", &eRif);
            eaclKey.eRif= eRif;
            if (mask[0] == '0')
            {
                eaclMask.eRif = 0;
            }
        }



        else if (strcmp(key, "DIP") == 0)
        {
            sscanf(data, "%u.%u.%u.%u", &dip[3], &dip[2], &dip[1], &dip[0]);
            int i;
            for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
            {
                if (dip[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter DIP data in valid format\n");
                    return;
                }
                eaclKey.dip[i] = dip[i];
                if (mask[0] == '0')
                {
                    eaclMask.dip[i] = 0x0;
                }

            }
            eaclKey.isDIPSet = 1;
        }
        else if (strcmp(key, "DIP6") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x", &dip[0],
                   &dip[1], &dip[2], &dip[3], &dip[4], &dip[5], &dip[6], &dip[7], &dip[8], &dip[9],
                   &dip[10], &dip[11], &dip[12], &dip[13], &dip[14], &dip[15]);
            int i;
            for (i = 0; i < XP_IPV6_ADDR_LEN; i++)
            {
                if (dip[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter DIP6 data in valid format\n");
                    return;
                }
                eaclKey.dip6[i] = dip[i];
                if (mask[0] == '0')
                {
                    eaclMask.dip6[i] = 0x0;
                }
            }
            eaclKey.isDIP6Set = 1;
        }
        else if (strcmp(key, "SIP6") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x", &sip[15],
                   &sip[14], &sip[13], &sip[12], &sip[11], &sip[10], &sip[9], &sip[8], &sip[7],
                   &sip[6], &sip[5], &sip[4], &sip[3], &sip[2], &sip[1], &sip[0]);
            int i;
            for (i = 0; i < XP_IPV6_ADDR_LEN; i++)
            {
                if (sip[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter SIP6 data in valid format\n");
                    return;
                }
                eaclKey.sip6[i] = sip[i];
                if (mask[0] == '0')
                {
                    eaclMask.sip6[i] = 0x0;
                }
            }
            eaclKey.isSIP6Set = 1;
        }
        else if (strcmp(key, "SIP") == 0)
        {
            sscanf(data, "%u.%u.%u.%u", &sip[3], &sip[2], &sip[1], &sip[0]);
            int i;
            for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
            {
                if (sip[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter SIP data in valid format\n");
                    return;
                }
                eaclKey.sip[i] = sip[i];
                if (mask[0] == '0')
                {
                    eaclMask.sip[i] = 0x0;
                }
            }
            eaclKey.isSIPSet = 1;
        }
        else if (strcmp(key, "L4DestPort") == 0)
        {
            sscanf(data, "%d", &l4DestPort);
            if (l4DestPort < 0 || l4DestPort > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter L4DestPort data in valid format\n");
                return;
            }
            eaclKey.L4DestPort = l4DestPort;
            if (mask[0] == '0')
            {
                eaclMask.L4DestPort = 0;
            }

        }
        //else if(strcmp(key, "egressVif") == 0)
        //{
        //  sscanf(data,"%d", &egressVif);
        //  if (egressVif < 0 || egressVif > 65535)
        //    {
        //        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n", lineNum, configFile.filename);
        //        printf("%s\n", line);
        //        printf("enter egressVif data in valid format\n");
        //        return;
        //    }
        //    eaclKey.egressVif = egressVif;
        //}
        //else if(strcmp(key, "eVlan") == 0)
        //{
        //  sscanf(data,"%d", &eVlan);
        //  if (eVlan < 0 || eVlan > 4096)
        //    {
        //        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n", lineNum, configFile.filename);
        //        printf("%s\n", line);
        //        printf("enter eVlan data in valid format\n");
        //        return;
        //    }
        //    eaclKey.eVlan = eVlan;
        //}


        else if (strcmp(key, "L4SourcePort") == 0)
        {
            sscanf(data, "%d", &l4SourcePort);
            if (l4SourcePort < 0 || l4SourcePort > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter L4SourcePort data in valid format\n");
                return;
            }
            eaclKey.L4SourcePort = l4SourcePort;
            if (mask[0] == '0')
            {
                eaclMask.L4SourcePort = 0;
            }

        }
        else if (strcmp(key, "icmpMessageType") == 0)
        {
            sscanf(data, "%d", &icmpMessageType);
            if (icmpMessageType < 0 || icmpMessageType > 255)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter icmpMessageType data in valid format\n");
                return;
            }
            eaclKey.icmpMessageType = icmpMessageType;
            if (mask[0] == '0')
            {
                eaclMask.icmpMessageType = 0;
            }
        }
        else if (strcmp(key, "protocol") == 0)
        {
            sscanf(data, "%x", &protocol);
            if (protocol > 255)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter protocol data in valid format\n");
                return;
            }
            eaclKey.protocol = protocol;
            if (mask[0] == '0')
            {
                eaclMask.protocol = 0;
            }

        }
        else if (strcmp(key, "vi") == 0)
        {
            sscanf(data, "%d", &vi);
            if (vi < 0 || vi > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter vi data in valid format\n");
                return;
            }
            eaclKey.vi = vi;
            if (mask[0] == '0')
            {
                eaclMask.vi = 0;
            }

        }
        else if (strcmp(key, "tcpFlags") == 0)
        {
            sscanf(data, "%d", &tcpFlags);
            if (tcpFlags < 0 || tcpFlags > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter tcpFlags data in valid format\n");
                return;
            }
            eaclKey.tcpFlags = tcpFlags;
            if (mask[0] == '0')
            {
                eaclMask.tcpFlags = 0;
            }

        }
        else if (strcmp(key, "dscp") == 0)
        {
            sscanf(data, "%d", &dscp);
            if (dscp < 0 || dscp > 63)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter dscp data in valid format\n");
                return;
            }
            eaclKey.dscp = dscp;
            if (mask[0] == '0')
            {
                eaclMask.dscp = 0;
            }

        }
        else if (strcmp(key, "exp") == 0)
        {
            sscanf(data, "%d", &exp);
            if (exp < 0 || exp > 7)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter esp data in valid format\n");
                return;
            }
            eaclKey.exp = exp;
            if (mask[0] == '0')
            {
                eaclMask.exp = 0;
            }

        }
        else if (strcmp(key, "pcp") == 0)
        {
            sscanf(data, "%d", &pcp);
            if (pcp < 0 || pcp > 7)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter pcp data in valid format\n");
                return;
            }
            eaclKey.pcp = pcp;
            if (mask[0] == '0')
            {
                eaclMask.pcp = 0;
            }

        }

        else
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("enter keyAttrubute data (Unknown Attrubute) in valid format\n");
            //return;
        }
        //str1 = str2;
        //str2 = strstr(str1, ",");
        str2 = strtok(NULL, ",");
        memset(key, 0x00, sizeof(key));
        memset(data, 0x00, sizeof(data));
        memset(mask, 0x00, sizeof(mask));
    }

    addEntry(&configFile.eaclKey, &eaclKey, sizeof(EaclL2KeyAttribute));
    addEntry(&configFile.eaclMask, &eaclMask, sizeof(EaclL2MaskAttribute));
    addEntry(&configFile.eaclData, &eaclData, sizeof(EaclData));

}

void processOpenFlowKeyMaskData(int lineNum, char *line)
{

    int ofIdx, tableId, entryType, ofType, Priority, isGrpEntry, actionSetId, dpId,
        groupId, groupType, instType, metaDataAction, metaDataActionMask;
    char matchKey[300] = {0};
    char *str1, *str2, *str3, *str4;
    char instructionSet[50] = {0};
    OpenFlowDataEntry openFlowData = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, {-1, -1, {0}, -1, {0}, -1, -1, -1, -1, -1, -1, -1, -1, {0}, -1, {0}, -1, -1, -1, -1, -1, -1}, {-1, -1, {0}, -1, {0}, -1, -1, -1, -1, -1, -1, -1, -1, {0}, -1, {0}, -1, -1, -1, -1, -1, -1}, -1, {0}, -1};
    int32_t inPort, ipPhyPort, ethType, tagging, vlanPCP, mpls, ip4tos, ip4proto,
            ip4icmpType, ip4icmpCode, vlanVID, tpSrc, tpDst, metaDataVal;
    unsigned int ethDst[XP_MAC_ADDR_LEN];
    unsigned int ethSrc[XP_MAC_ADDR_LEN];
    int ip4srcAddr[4];
    int ip4dstAddr[4];
    char key[50] = {0}, data[50] = {0}, mask[10] = {0}, bucketNum[3] = {0},
                                                                       metaData[50]= {0};
    char *bucketActLine[10] = {NULL};

    //memset(&openFlowData.openFlowMask, 0xff, sizeof(OpenFlowKeyMaskEntry));
    sscanf(line, "%d %d %d %d %d %d %d %d %d %d %49s %299s %d %49s",
           &ofIdx, &tableId, &entryType, &ofType, &Priority, &isGrpEntry, &groupType,
           &dpId, &groupId, &instType, metaData, matchKey, &actionSetId, instructionSet);

    if (entryType< 0 || entryType> 2)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter entryType data in valid format\n");
        return;
    }
    openFlowData.entryType = entryType;
    if (tableId< 0 || tableId> 3)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter tableId data in valid format\n");
        return;
    }
    openFlowData.tableId = tableId;
    if (ofType< 0 || ofType> 2)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter ofType data in valid format\n");
        return;
    }
    openFlowData.ofType = ofType;
    if (isGrpEntry < 0 || isGrpEntry > 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter isGrpEntry data in valid format\n");
        return;
    }
    openFlowData.isGrpEntry = isGrpEntry;
    if (instType > 4 || instType < 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter instType data in valid format\n");
        return;
    }
    openFlowData.instType = instType;
    openFlowData.Priority = Priority;
    openFlowData.actSetId = actionSetId;
    openFlowData.dpId = dpId;
    openFlowData.groupId = groupId;
    openFlowData.groupType = groupType;
    str1 = matchKey;
    str2 = strtok(str1, ",");

    while (str2 != NULL)
    {
        str3 = strstr(str2, "->");
        if (str3)
        {
            strncpy(key, str2, str3-str2);
            str4 = strstr(str2, "|");
            if (str4)
            {
                strncpy(data, str3 +2, (str4-str3 - 2));
                strncpy(mask, str4+1, strlen(str4)-1);
                //strncpy(mask,str4+1, 1);
            }
            else
            {
                printf("ERROR: Invalid input for matchKey");
                return;
            }
        }
        else
        {
            printf("ERROR: Invalid input for matchKey");
            return;
        }

        if (strcmp(key, "ethDst") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x", &ethDst[5], &ethDst[4], &ethDst[3],
                   &ethDst[2], &ethDst[1], &ethDst[0]);
            int i;
            int maskNumByte = atoi(mask)/8;
            int maskNumBit = atoi(mask)%8;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if (ethDst[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter ethDst data in valid format\n");
                    return;
                }
                openFlowData.openFlowKey.ethDst[5-i]= ethDst[i];
                if (mask[0] != '0')
                {
                    //          if(i < maskNumByte)
                    {
                        openFlowData.openFlowMask.ethDst[5-i]= 0xFF;
                    }
                    if ((i == maskNumByte) && (maskNumBit != 0))
                    {
                        openFlowData.openFlowMask.ethDst[5-i] = (uint8_t)pow(2, maskNumBit);
                    }
                }
                else
                {
                    openFlowData.openFlowMask.ethDst[5-i] = 0;
                }

            }
            openFlowData.openFlowKey.isDestMac = 1;
        }
        else if (strcmp(key, "ethSrc") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x", &ethSrc[5], &ethSrc[4], &ethSrc[3],
                   &ethSrc[2], &ethSrc[1], &ethSrc[0]);
            int i;
            int maskNumByte = atoi(mask)/8;
            int maskNumBit = atoi(mask)%8;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if (ethSrc[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter ethSrc data in valid format\n");
                    return;
                }
                openFlowData.openFlowKey.ethSrc[5 -i]= ethSrc[i];
                if (mask[0] != '0')
                {
                    if (i < maskNumByte)
                    {
                        openFlowData.openFlowMask.ethSrc[5-i]= 0xFF;
                    }
                    if ((i == maskNumByte) && (maskNumBit != 0))
                    {
                        openFlowData.openFlowMask.ethSrc[5-i] = (uint8_t)pow(2, maskNumBit);
                    }
                }
                else
                {
                    openFlowData.openFlowMask.ethSrc[5-i] = 0;
                }


            }
            openFlowData.openFlowKey.isSrcMac = 1;
        }
        else if (strcmp(key, "inPort") == 0)
        {
            sscanf(data, "%d", &inPort);
            if (inPort < 0 || inPort > 128)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter inPort data in valid format\n");
                return;
            }
            openFlowData.openFlowKey.inPort = inPort;
            sscanf(mask, "%d", &inPort);
            openFlowData.openFlowMask.inPort = inPort;

        }
        else if (strcmp(key, "ipPhyPort") == 0)
        {
            sscanf(data, "%d", &ipPhyPort);
            if (ipPhyPort < 0 || ipPhyPort > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter ipPhyPort data in valid format\n");
                return;
            }
            openFlowData.openFlowKey.ipPhyPort = ipPhyPort;
            sscanf(data, "%d", &ipPhyPort);
            if (ipPhyPort < 0 || ipPhyPort > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter ipPhyPort data in valid format\n");
                return;
            }
            openFlowData.openFlowMask.ipPhyPort = ipPhyPort;
        }

        else if (strcmp(key, "ethType") == 0)
        {
            sscanf(data, "%d", &ethType);
            openFlowData.openFlowKey.ethType = ethType;
            sscanf(mask, "%d", &ethType);
            openFlowData.openFlowMask.ethType = ethType;
        }
        else if (strcmp(key, "tagging") == 0)
        {
            sscanf(data, "%d", &tagging);
            openFlowData.openFlowKey.tagging = tagging;
            sscanf(mask, "%d", &tagging);
            openFlowData.openFlowMask.tagging = tagging;

        }
        else if (strcmp(key, "vlanVID") == 0)
        {
            sscanf(data, "%d", &vlanVID);

            if (vlanVID < 0 || vlanVID > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter vlanVID data in valid format\n");
                return;
            }
            openFlowData.openFlowKey.vlanVID = vlanVID;
            sscanf(mask, "%d", &vlanVID);
            openFlowData.openFlowMask.vlanVID = vlanVID;
        }

        else if (strcmp(key, "vlanPCP") == 0)
        {
            sscanf(data, "%d", &vlanPCP);

            if (vlanPCP < 0 || vlanPCP > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter vlanPCP data in valid format\n");
                return;
            }
            openFlowData.openFlowKey.vlanPCP = vlanPCP;
            sscanf(mask, "%d", &vlanPCP);
            openFlowData.openFlowMask.vlanPCP = vlanPCP;

        }

        else if (strcmp(key, "mpls") == 0)
        {
            sscanf(data, "%d", &mpls);
            openFlowData.openFlowKey.mpls = mpls;
            sscanf(mask, "%d", &mpls);
            openFlowData.openFlowMask.mpls = mpls;
        }

        else if (strcmp(key, "tpSrc") == 0)
        {
            sscanf(data, "%d", &tpSrc);
            openFlowData.openFlowKey.tpSrc = tpSrc;
            sscanf(mask, "%d", &tpSrc);
            openFlowData.openFlowMask.tpSrc = tpSrc;
        }

        else if (strcmp(key, "tpDst") == 0)
        {
            sscanf(data, "%d", &tpDst);
            openFlowData.openFlowKey.tpDst = tpDst;
            sscanf(mask, "%d", &tpDst);
            openFlowData.openFlowMask.tpDst = tpDst;
        }
        else if (strcmp(key, "ip4srcAddr") == 0)
        {
            sscanf(data, "%d.%d.%d.%d", &ip4srcAddr[3], &ip4srcAddr[2], &ip4srcAddr[1],
                   &ip4srcAddr[0]);
            int i;
            int maskNumByte = atoi(mask)/8;
            int maskNumBit = atoi(mask)%8;
            for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
            {
                if (ip4srcAddr[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter ip4srcAddr data in valid format\n");
                    return;
                }
                openFlowData.openFlowKey.ip4srcAddr[3-i] = ip4srcAddr[i];
                if (mask[0] != '0')
                {
                    if (i < maskNumByte)
                    {
                        openFlowData.openFlowMask.ip4srcAddr[i]= 0xFF;
                    }
                    if ((i == maskNumByte) && (maskNumBit != 0))
                    {
                        openFlowData.openFlowMask.ip4srcAddr[i] = (uint8_t)pow(2, maskNumBit);
                    }
                }
                else
                {
                    openFlowData.openFlowMask.ip4srcAddr[i] = 0;
                }

            }
            openFlowData.openFlowKey.isIpv4SrcAddr = 1;
        }
        else if (strcmp(key, "ip4dstAddr") == 0)
        {
            sscanf(data, "%d.%d.%d.%d", &ip4dstAddr[3], &ip4dstAddr[2], &ip4dstAddr[1],
                   &ip4dstAddr[0]);
            int i;
            int maskNumByte = atoi(mask)/8;
            int maskNumBit = atoi(mask)%8;
            for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
            {
                if (ip4dstAddr[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter ip4dstAddr data in valid format\n");
                    return;
                }
                openFlowData.openFlowKey.ip4dstAddr[3-i] = ip4dstAddr[i];
                if (mask[0] != '0')
                {
                    if (i < maskNumByte)
                    {
                        openFlowData.openFlowMask.ip4dstAddr[i]= 0xFF;
                    }
                    if ((i == maskNumByte) && (maskNumBit != 0))
                    {
                        openFlowData.openFlowMask.ip4dstAddr[i] = (uint8_t)pow(2, maskNumBit);
                    }
                }
                else
                {
                    openFlowData.openFlowMask.ip4dstAddr[i] = 0;
                }

            }
            openFlowData.openFlowKey.isIpv4dstAddr = 1;
        }
        else if (strcmp(key, "ip4tos") == 0)
        {
            sscanf(data, "%d", &ip4tos);
            openFlowData.openFlowKey.ip4tos = ip4tos;
            sscanf(mask, "%d", &ip4tos);
            openFlowData.openFlowMask.ip4tos = ip4tos;
        }
        else if (strcmp(key, "ip4proto") == 0)
        {
            sscanf(data, "%d", &ip4proto);
            openFlowData.openFlowKey.ip4proto = ip4proto;
            sscanf(mask, "%d", &ip4proto);
            openFlowData.openFlowMask.ip4proto = ip4proto;
        }

        else if (strcmp(key, "ip4icmpType") == 0)
        {
            sscanf(data, "%d", &ip4icmpType);
            openFlowData.openFlowKey.ip4icmpType = ip4icmpType;
            sscanf(mask, "%d", &ip4icmpType);
            openFlowData.openFlowMask.ip4icmpType = ip4icmpType;
        }
        else if (strcmp(key, "ip4icmpCode") == 0)
        {
            sscanf(data, "%d", &ip4icmpCode);
            openFlowData.openFlowKey.ip4icmpCode = ip4icmpCode;
            sscanf(mask, "%d", &ip4icmpCode);
            openFlowData.openFlowMask.ip4icmpCode = ip4icmpCode;
        }
        else if (strcmp(key, "metaData") == 0)
        {
            sscanf(data, "%d", &metaDataVal);
            openFlowData.openFlowKey.metaData = metaDataVal;
            sscanf(mask, "%d", &metaDataVal);
            openFlowData.openFlowMask.metaData = metaDataVal;
        }
        else
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("enter keyAttrubute data (Unknown Attrubute) in valid format\n");
            //return;
        }
        //str1 = str2;
        //str2 = strstr(str1, ",");
        str2 = strtok(NULL, ",");
        memset(key, 0x00, sizeof(key));
        memset(data, 0x00, sizeof(data));
        memset(mask, 0x00, sizeof(mask));
    }
    str1 = NULL;
    str2 = NULL;
    str3 = NULL;
    str4 = NULL;

    int i =0;
    int j =0;
    uint8_t bucketListIndex = 0;
    str1 = instructionSet;
    if (isGrpEntry == 1)
    {
        str2 = strtok(str1, "|");
        while (str2 != NULL)
        {
            bucketActLine[i] = str2;
            i++;
            str2 = strtok(NULL, "|");
        }

        openFlowData.actLen = i;
        i = 0;
        while (bucketActLine[i])
        {
            str1 = bucketActLine[i];
            str2 = strstr(str1, "=");
            strncpy(bucketNum, str1, str2-str1);
            bucketListIndex = atoi(bucketNum);
            if (bucketListIndex >= sizeof(openFlowData.bucketList)/sizeof(
                    openFlowData.bucketList[0]))
            {
                printf("BucketList Index out of bound.\n");
            }
            str3 = strtok(str2+1, ",");
            while (str3 != NULL)
            {
                openFlowData.bucketList[bucketListIndex].bucketActList[j] = atoi(str3);
                j++;
                str3 = strtok(NULL, ",");
            }
            openFlowData.bucketList[bucketListIndex].bucketLen = j;
            j = 0;
            i++;
        }
    }
    else if (isGrpEntry == 0)
    {
        str2 = strtok(str1, ",");
        while (str2 != NULL)
        {
            openFlowData.actList[i] = atoi(str2);
            i++;
            str2 = strtok(NULL, ",");
        }
        openFlowData.actLen = i;
    }

    sscanf(metaData, "%d|%d", &metaDataAction, &metaDataActionMask);
    openFlowData.metaData = metaDataAction;
    openFlowData.metaDataMask = metaDataActionMask;
    addEntry(&configFile.openFlowData, &openFlowData, sizeof(OpenFlowDataEntry));
    return;
}

void processOpenFlowActionData(int lineNum, char *line)
{
    OpenFlowActionEntry openFlowDataAct;
    uint32_t actId, actType, ethType, mplsLabel, mplsTC, mplsBOS, arpOP, ipDSCP,
             ipECN, icmpType, icmpCode, pbbISID, pbbUCA, port, pktMaxLen, vlanVID, l4Port,
             vlanPCP, setTTL, mpls, mplsTTL;
    int32_t subActType;

    char *str1, *str2, *str3, *str4;
    unsigned int ethAddr[XP_MAC_ADDR_LEN];
    int ipv4Addr[4];
    char key[50] = {0}, data[50]= {0}, mask[3] = {0};
    char actDataMask[200] = {0};

    memset(&openFlowDataAct, 0x0, sizeof(OpenFlowActionEntry));

    sscanf(line, "%u %u %d %199s",
           &actId, &actType, &subActType, actDataMask);
    openFlowDataAct.actId = actId;
    if ((actType> 16 && !(actType == 30 || actType == 31 ||
                          actType == 32 || actType == 19 || actType == 20)))
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter actType data in valid format\n");
        return;
    }
    openFlowDataAct.actType = actType;

    if (subActType < -1 || (subActType> 39 && subActType != 42))
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter subActType data in valid format\n");
        return;
    }
    openFlowDataAct.subActType = subActType;

    str1 = actDataMask;
    str2 = strtok(str1, ",");
    //TODO: Need to add suport for NONE
    while (str2 != NULL)
    {
        str3 = strstr(str2, "->");
        if (str3)
        {
            strncpy(key, str2, str3-str2);
            str4 = strstr(str2, "|");
            if (str4 != NULL)
            {
                strncpy(data, str3 +2, (str4-str3 - 2));
                strncpy(mask, str4+1, strlen(str4)-1);
            }
            else
            {
                strcpy(data, str3);
            }
        }
        else
        {
            printf("ERROR: Invalid input for Action Data Mask");
        }

        if (strcmp(key, "ethAddr") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x", &ethAddr[0], &ethAddr[1], &ethAddr[2],
                   &ethAddr[3], &ethAddr[4], &ethAddr[5]);
            int i;
            int maskNumByte = atoi(mask)/8;
            int maskNumBit = atoi(mask)%8;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if (ethAddr[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter ethAddr data in valid format\n");
                    return;
                }
                openFlowDataAct.ofActionData.ethAddr[i]= ethAddr[i];
                if (mask[0] != '0')
                {
                    if (i < maskNumByte)
                    {
                        openFlowDataAct.ofActionMask.ethAddr[i]= 0x00;
                    }
                    if ((i == maskNumByte) && (maskNumBit != 0))
                    {
                        openFlowDataAct.ofActionMask.ethAddr[i] = ~(uint8_t)pow(2, maskNumBit);
                    }
                }
                else
                {
                    openFlowDataAct.ofActionMask.ethAddr[i] = 0;
                }


            }
        }
        else if (strcmp(key, "ethType") == 0)
        {
            sscanf(data, "%u", &ethType);
            openFlowDataAct.ofActionData.ethType = ethType;
            sscanf(mask, "%u", &ethType);
            openFlowDataAct.ofActionMask.ethType = ethType;
        }
        else if (strcmp(key, "vlanVID") == 0)
        {
            sscanf(data, "%u", &vlanVID);

            if (vlanVID > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter vlanVID data in valid format\n");
                return;
            }
            openFlowDataAct.ofActionData.vlanVID = vlanVID;
            vlanVID = 0;
            sscanf(mask, "%u", &vlanVID);
            openFlowDataAct.ofActionMask.vlanVID = vlanVID;
        }

        else if (strcmp(key, "vlanPCP") == 0)
        {
            sscanf(data, "%u", &vlanPCP);

            if (vlanPCP > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter vlanPCP data in valid format\n");
                return;
            }
            openFlowDataAct.ofActionData.vlanPCP = vlanPCP;
            sscanf(mask, "%u", &vlanPCP);
            openFlowDataAct.ofActionMask.vlanPCP = vlanPCP;

        }

        else if (strcmp(key, "mplsLabel") == 0)
        {
            sscanf(data, "%u", &mplsLabel);
            openFlowDataAct.ofActionData.mplsLabel = mplsLabel;
            sscanf(mask, "%u", &mplsLabel);
            openFlowDataAct.ofActionMask.mplsLabel = mplsLabel;
        }
        else if (strcmp(key, "mpls") == 0)
        {
            sscanf(data, "%u", &mpls);
            openFlowDataAct.ofActionData.mpls = mpls;
            sscanf(mask, "%u", &mpls);
            openFlowDataAct.ofActionMask.mpls = mpls;
        }
        else if (strcmp(key, "mplsTTL") == 0)
        {
            sscanf(data, "%u", &mplsTTL);
            openFlowDataAct.ofActionData.mplsTTL = mplsTTL;
            sscanf(mask, "%u", &mpls);
            openFlowDataAct.ofActionMask.mplsTTL = mplsTTL;
        }

        else if (strcmp(key, "mplsTC") == 0)
        {
            sscanf(data, "%u", &mplsTC);
            openFlowDataAct.ofActionData.mplsTC = mplsTC;
            sscanf(mask, "%u", &mplsTC);
            openFlowDataAct.ofActionMask.mplsTC = mplsTC;
        }
        else if (strcmp(key, "mplsBOS") == 0)
        {
            sscanf(data, "%u", &mplsBOS);
            openFlowDataAct.ofActionData.mplsBOS = mplsBOS;
            sscanf(mask, "%u", &mplsBOS);
            openFlowDataAct.ofActionMask.mplsBOS = mplsBOS;
        }

        else if (strcmp(key, "arpOP") == 0)
        {
            sscanf(data, "%u", &arpOP);
            openFlowDataAct.ofActionData.arpOP = arpOP;
            sscanf(mask, "%u", &arpOP);
            openFlowDataAct.ofActionMask.arpOP = arpOP;
        }

        else if (strcmp(key, "ipv4Addr") == 0)
        {
            sscanf(data, "%d.%d.%d.%d", &ipv4Addr[3], &ipv4Addr[2], &ipv4Addr[1],
                   &ipv4Addr[0]);
            int i;
            int maskNumByte = atoi(mask)/8;
            int maskNumBit = atoi(mask)%8;
            for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
            {
                if (ipv4Addr[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter ipv4Addr data in valid format\n");
                    return;
                }
                openFlowDataAct.ofActionData.ipv4Addr[3-i] = ipv4Addr[i];
                if (mask[0] != '0')
                {
                    if (i < maskNumByte)
                    {
                        openFlowDataAct.ofActionMask.ipv4Addr[i]= 0x00;
                    }
                    if ((i == maskNumByte) && (maskNumBit != 0))
                    {
                        openFlowDataAct.ofActionMask.ipv4Addr[i] = ~(uint8_t)pow(2, maskNumBit);
                    }
                }
                else
                {
                    openFlowDataAct.ofActionMask.ipv4Addr[i] = 0;
                }

            }
        }
        else if (strcmp(key, "ipDSCP") == 0)
        {
            sscanf(data, "%u", &ipDSCP);
            openFlowDataAct.ofActionData.ipDSCP = ipDSCP;
            sscanf(mask, "%u", &ipDSCP);
            openFlowDataAct.ofActionMask.ipDSCP = ipDSCP;
        }
        else if (strcmp(key, "ipECN") == 0)
        {
            sscanf(data, "%u", &ipECN);
            openFlowDataAct.ofActionData.ipECN = ipECN;
            sscanf(mask, "%u", &ipECN);
            openFlowDataAct.ofActionMask.ipECN = ipECN;
        }

        else if (strcmp(key, "icmpType") == 0)
        {
            sscanf(data, "%u", &icmpType);
            openFlowDataAct.ofActionData.icmpType = icmpType;
            sscanf(mask, "%u", &icmpType);
            openFlowDataAct.ofActionMask.icmpType = icmpType;
        }
        else if (strcmp(key, "icmpCode") == 0)
        {
            sscanf(data, "%u", &icmpCode);
            openFlowDataAct.ofActionData.icmpCode = icmpCode;
            sscanf(mask, "%u", &icmpCode);
            openFlowDataAct.ofActionMask.icmpCode = icmpCode;
        }
        else if (strcmp(key, "l4Port") == 0)
        {
            sscanf(data, "%u", &l4Port);
            openFlowDataAct.ofActionData.l4Port = l4Port;
            sscanf(mask, "%u", &l4Port);
            openFlowDataAct.ofActionMask.l4Port = l4Port;
        }
        else if (strcmp(key, "pbbISID") == 0)
        {
            sscanf(data, "%u", &pbbISID);
            openFlowDataAct.ofActionData.pbbISID = pbbISID;
            sscanf(mask, "%u", &pbbISID);
            openFlowDataAct.ofActionMask.pbbISID = pbbISID;
        }
        else if (strcmp(key, "pbbUCA") == 0)
        {
            sscanf(data, "%u", &pbbUCA);
            openFlowDataAct.ofActionData.pbbUCA = pbbUCA;
            sscanf(mask, "%u", &pbbUCA);
            openFlowDataAct.ofActionMask.pbbUCA = pbbUCA;
        }
        else if (strcmp(key, "port") == 0)
        {
            sscanf(data, "%u", &port);
            openFlowDataAct.ofActionData.port = port;
            sscanf(mask, "%u", &port);
            openFlowDataAct.ofActionMask.port = port;
        }
        else if (strcmp(key, "pktMaxLen") == 0)
        {
            sscanf(data, "%u", &pktMaxLen);
            openFlowDataAct.ofActionData.pktMaxLen = pktMaxLen;
            sscanf(mask, "%u", &pktMaxLen);
            openFlowDataAct.ofActionMask.pktMaxLen = pktMaxLen;
        }
        else if (strcmp(key, "setTTL") == 0)
        {
            sscanf(data, "%u", &setTTL);
            openFlowDataAct.ofActionData.setTTL = setTTL;
            sscanf(mask, "%u", &setTTL);
            openFlowDataAct.ofActionMask.setTTL = setTTL;
        }
        else
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("enter keyAttrubute data (Unknown Attrubute) in valid format\n");
            //return;
        }
        //str1 = str2;
        //str2 = strstr(str1, ",");
        str2 = strtok(NULL, ",");
        memset(data, 0x00, sizeof(data));
        memset(mask, 0x00, sizeof(mask));
        memset(key, 0x00, sizeof(key));
    }
    addEntry(&configFile.openFlowActionData, &openFlowDataAct,
             sizeof(OpenFlowActionEntry));
    return;
}

void processPureOfKeyMaskData(int lineNum, char *line)
{

    int ofIdx, tableId, entryType, ofType, Priority, isGrpEntry, actionSetId, dpId,
        groupId, groupType, instType, metaDataAction, metaDataActionMask;
    char matchKey[300] = {0};
    char *str1, *str2, *str3, *str4;
    char instructionSet[50] = {0};
    OpenFlowDataEntry openFlowData = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, {-1, -1, {0}, -1, {0}, -1, -1, -1, -1, -1, -1, -1, -1, {0}, -1, {0}, -1, -1, -1, -1, -1, -1}, {-1, -1, {0}, -1, {0}, -1, -1, -1, -1, -1, -1, -1, -1, {0}, -1, {0}, -1, -1, -1, -1, -1, -1}, -1, {0}, -1};
    int32_t inPort, ipPhyPort, ethType, tagging, vlanPCP, mpls, ip4tos, ip4proto,
            ip4icmpType, ip4icmpCode, vlanVID, tpSrc, tpDst, metaDataVal;
    unsigned int ethDst[XP_MAC_ADDR_LEN];
    unsigned int ethSrc[XP_MAC_ADDR_LEN];
    int ip4srcAddr[4];
    int ip4dstAddr[4];
    char key[50] = {0}, data[50] = {0}, mask[10] = {0}, bucketNum[3] = {0},
                                                                       metaData[50]= {0};
    char *bucketActLine[10] = {NULL};

    //memset(&openFlowData.openFlowMask, 0xff, sizeof(OpenFlowKeyMaskEntry));
    sscanf(line, "%d %d %d %d %d %d %d %d %d %d %49s %299s %d %49s",
           &ofIdx, &tableId, &entryType, &ofType, &Priority, &isGrpEntry, &groupType,
           &dpId, &groupId, &instType, metaData, matchKey, &actionSetId, instructionSet);

    if (entryType< 0 || entryType> 2)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter entryType data in valid format\n");
        return;
    }
    openFlowData.entryType = entryType;
    if (tableId< 0 || tableId> 3)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter tableId data in valid format\n");
        return;
    }
    openFlowData.tableId = tableId;
    if (ofType< 0 || ofType> 2)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter ofType data in valid format\n");
        return;
    }
    openFlowData.ofType = ofType;
    if (isGrpEntry < 0 || isGrpEntry > 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter isGrpEntry data in valid format\n");
        return;
    }
    openFlowData.isGrpEntry = isGrpEntry;
    if (instType > 4 || instType < 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter instType data in valid format\n");
        return;
    }
    openFlowData.instType = instType;
    openFlowData.Priority = Priority;
    openFlowData.actSetId = actionSetId;
    openFlowData.dpId = dpId;
    openFlowData.groupId = groupId;
    openFlowData.groupType = groupType;
    str1 = matchKey;
    str2 = strtok(str1, ",");

    while (str2 != NULL)
    {
        str3 = strstr(str2, "->");
        if (str3)
        {
            strncpy(key, str2, str3-str2);
            str4 = strstr(str2, "|");
            if (str4)
            {
                strncpy(data, str3 +2, (str4-str3 - 2));
                strncpy(mask, str4+1, strlen(str4)-1);
                //strncpy(mask,str4+1, 1);
            }
            else
            {
                printf("ERROR: Invalid input for matchKey\n");
                return;
            }
        }
        else
        {
            printf("ERROR: Invalid input\n");
            return;
        }

        if (strcmp(key, "ethDst") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x", &ethDst[5], &ethDst[4], &ethDst[3],
                   &ethDst[2], &ethDst[1], &ethDst[0]);
            int i;
            int maskNumByte = atoi(mask)/8;
            int maskNumBit = atoi(mask)%8;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if (ethDst[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter ethDst data in valid format\n");
                    return;
                }
                openFlowData.openFlowKey.ethDst[5-i]= ethDst[i];
                if (mask[0] != '0')
                {
                    //          if(i < maskNumByte)
                    {
                        openFlowData.openFlowMask.ethDst[5-i]= 0xFF;
                    }
                    if ((i == maskNumByte) && (maskNumBit != 0))
                    {
                        openFlowData.openFlowMask.ethDst[5-i] = (uint8_t)pow(2, maskNumBit);
                    }
                }
                else
                {
                    openFlowData.openFlowMask.ethDst[5-i] = 0;
                }
            }
            openFlowData.openFlowKey.isDestMac = 1;
        }
        else if (strcmp(key, "ethSrc") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x", &ethSrc[5], &ethSrc[4], &ethSrc[3],
                   &ethSrc[2], &ethSrc[1], &ethSrc[0]);
            int i;
            int maskNumByte = atoi(mask)/8;
            int maskNumBit = atoi(mask)%8;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if (ethSrc[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter ethSrc data in valid format\n");
                    return;
                }
                openFlowData.openFlowKey.ethSrc[5 -i]= ethSrc[i];
                if (mask[0] != '0')
                {
                    if (i < maskNumByte)
                    {
                        openFlowData.openFlowMask.ethSrc[5-i]= 0xFF;
                    }
                    if ((i == maskNumByte) && (maskNumBit != 0))
                    {
                        openFlowData.openFlowMask.ethSrc[5-i] = (uint8_t)pow(2, maskNumBit);
                    }
                }
                else
                {
                    openFlowData.openFlowMask.ethSrc[5-i] = 0;
                }
            }
            openFlowData.openFlowKey.isSrcMac = 1;
        }
        else if (strcmp(key, "inPort") == 0)
        {
            sscanf(data, "%d", &inPort);
            if (inPort < 0 || inPort > 128)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter inPort data in valid format\n");
                return;
            }
            openFlowData.openFlowKey.inPort = inPort;
            sscanf(mask, "%d", &inPort);
            openFlowData.openFlowMask.inPort = inPort;
        }
        else if (strcmp(key, "ipPhyPort") == 0)
        {
            sscanf(data, "%d", &ipPhyPort);
            if (ipPhyPort < 0 || ipPhyPort > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter ipPhyPort data in valid format\n");
                return;
            }
            openFlowData.openFlowKey.ipPhyPort = ipPhyPort;
            sscanf(data, "%d", &ipPhyPort);
            if (ipPhyPort < 0 || ipPhyPort > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter ipPhyPort data in valid format\n");
                return;
            }
            openFlowData.openFlowMask.ipPhyPort = ipPhyPort;
        }
        else if (strcmp(key, "ethType") == 0)
        {
            sscanf(data, "%d", &ethType);
            openFlowData.openFlowKey.ethType = ethType;
            sscanf(mask, "%d", &ethType);
            openFlowData.openFlowMask.ethType = ethType;
        }
        else if (strcmp(key, "tagging") == 0)
        {
            sscanf(data, "%d", &tagging);
            openFlowData.openFlowKey.tagging = tagging;
            sscanf(mask, "%d", &tagging);
            openFlowData.openFlowMask.tagging = tagging;

        }
        else if (strcmp(key, "vlanVID") == 0)
        {
            sscanf(data, "%d", &vlanVID);
            if (vlanVID < 0 || vlanVID > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter vlanVID data in valid format\n");
                return;
            }
            openFlowData.openFlowKey.vlanVID = vlanVID;
            sscanf(mask, "%d", &vlanVID);
            openFlowData.openFlowMask.vlanVID = vlanVID;
        }
        else if (strcmp(key, "vlanPCP") == 0)
        {
            sscanf(data, "%d", &vlanPCP);

            if (vlanPCP < 0 || vlanPCP > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter vlanPCP data in valid format\n");
                return;
            }
            openFlowData.openFlowKey.vlanPCP = vlanPCP;
            sscanf(mask, "%d", &vlanPCP);
            openFlowData.openFlowMask.vlanPCP = vlanPCP;
        }

        else if (strcmp(key, "mpls") == 0)
        {
            sscanf(data, "%d", &mpls);
            openFlowData.openFlowKey.mpls = mpls;
            sscanf(mask, "%d", &mpls);
            openFlowData.openFlowMask.mpls = mpls;
        }
        else if (strcmp(key, "tpSrc") == 0)
        {
            sscanf(data, "%d", &tpSrc);
            openFlowData.openFlowKey.tpSrc = tpSrc;
            sscanf(mask, "%d", &tpSrc);
            openFlowData.openFlowMask.tpSrc = tpSrc;
        }
        else if (strcmp(key, "tpDst") == 0)
        {
            sscanf(data, "%d", &tpDst);
            openFlowData.openFlowKey.tpDst = tpDst;
            sscanf(mask, "%d", &tpDst);
            openFlowData.openFlowMask.tpDst = tpDst;
        }
        else if (strcmp(key, "ip4srcAddr") == 0)
        {
            sscanf(data, "%d.%d.%d.%d", &ip4srcAddr[3], &ip4srcAddr[2], &ip4srcAddr[1],
                   &ip4srcAddr[0]);
            int i;
            int maskNumByte = atoi(mask)/8;
            int maskNumBit = atoi(mask)%8;
            for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
            {
                if (ip4srcAddr[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter ip4srcAddr data in valid format\n");
                    return;
                }
                openFlowData.openFlowKey.ip4srcAddr[3-i] = ip4srcAddr[i];
                if (mask[0] != '0')
                {
                    if (i < maskNumByte)
                    {
                        openFlowData.openFlowMask.ip4srcAddr[i]= 0xFF;
                    }
                    if ((i == maskNumByte) && (maskNumBit != 0))
                    {
                        openFlowData.openFlowMask.ip4srcAddr[i] = (uint8_t)pow(2, maskNumBit);
                    }
                }
                else
                {
                    openFlowData.openFlowMask.ip4srcAddr[i] = 0;
                }

            }
            openFlowData.openFlowKey.isIpv4SrcAddr = 1;
        }
        else if (strcmp(key, "ip4dstAddr") == 0)
        {
            sscanf(data, "%d.%d.%d.%d", &ip4dstAddr[3], &ip4dstAddr[2], &ip4dstAddr[1],
                   &ip4dstAddr[0]);
            int i;
            int maskNumByte = atoi(mask)/8;
            int maskNumBit = atoi(mask)%8;
            for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
            {
                if (ip4dstAddr[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter ip4dstAddr data in valid format\n");
                    return;
                }
                openFlowData.openFlowKey.ip4dstAddr[3-i] = ip4dstAddr[i];
                if (mask[0] != '0')
                {
                    if (i < maskNumByte)
                    {
                        openFlowData.openFlowMask.ip4dstAddr[i]= 0xFF;
                    }
                    if ((i == maskNumByte) && (maskNumBit != 0))
                    {
                        openFlowData.openFlowMask.ip4dstAddr[i] = (uint8_t)pow(2, maskNumBit);
                    }
                }
                else
                {
                    openFlowData.openFlowMask.ip4dstAddr[i] = 0;
                }

            }
            openFlowData.openFlowKey.isIpv4dstAddr = 1;
        }
        else if (strcmp(key, "ip4tos") == 0)
        {
            sscanf(data, "%d", &ip4tos);
            openFlowData.openFlowKey.ip4tos = ip4tos;
            sscanf(mask, "%d", &ip4tos);
            openFlowData.openFlowMask.ip4tos = ip4tos;
        }
        else if (strcmp(key, "ip4proto") == 0)
        {
            sscanf(data, "%d", &ip4proto);
            openFlowData.openFlowKey.ip4proto = ip4proto;
            sscanf(mask, "%d", &ip4proto);
            openFlowData.openFlowMask.ip4proto = ip4proto;
        }

        else if (strcmp(key, "ip4icmpType") == 0)
        {
            sscanf(data, "%d", &ip4icmpType);
            openFlowData.openFlowKey.ip4icmpType = ip4icmpType;
            sscanf(mask, "%d", &ip4icmpType);
            openFlowData.openFlowMask.ip4icmpType = ip4icmpType;
        }
        else if (strcmp(key, "ip4icmpCode") == 0)
        {
            sscanf(data, "%d", &ip4icmpCode);
            openFlowData.openFlowKey.ip4icmpCode = ip4icmpCode;
            sscanf(mask, "%d", &ip4icmpCode);
            openFlowData.openFlowMask.ip4icmpCode = ip4icmpCode;
        }
        else if (strcmp(key, "metaData") == 0)
        {
            sscanf(data, "%d", &metaDataVal);
            openFlowData.openFlowKey.metaData = metaDataVal;
            sscanf(mask, "%d", &metaDataVal);
            openFlowData.openFlowMask.metaData = metaDataVal;
        }
        else
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("enter keyAttrubute data (Unknown Attrubute) in valid format\n");
            //return;
        }
        //str1 = str2;
        //str2 = strstr(str1, ",");
        str2 = strtok(NULL, ",");
        memset(key, 0x00, sizeof(key));
        memset(data, 0x00, sizeof(data));
        memset(mask, 0x00, sizeof(mask));
    }
    str1 = NULL;
    str2 = NULL;
    str3 = NULL;
    str4 = NULL;

    int i =0;
    int j =0;
    uint8_t bucketListIndex = 0;
    str1 = instructionSet;
    if (isGrpEntry == 1)
    {
        str2 = strtok(str1, "|");
        while (str2 != NULL)
        {
            bucketActLine[i] = str2;
            i++;
            str2 = strtok(NULL, "|");
        }
        openFlowData.actLen = i;
        i = 0;
        while (bucketActLine[i])
        {
            str1 = bucketActLine[i];
            str2 = strstr(str1, "=");
            strncpy(bucketNum, str1, str2-str1);
            bucketListIndex = atoi(bucketNum);
            if (bucketListIndex >= sizeof(openFlowData.bucketList)/sizeof(
                    openFlowData.bucketList[0]))
            {
                printf("BucketList Index out of bound.\n");
            }
            str3 = strtok(str2+1, ",");
            while (str3 != NULL)
            {
                openFlowData.bucketList[bucketListIndex].bucketActList[j] = atoi(str3);
                j++;
                str3 = strtok(NULL, ",");
            }
            openFlowData.bucketList[bucketListIndex].bucketLen = j;
            j = 0;
            i++;
        }
    }
    else if (isGrpEntry == 0)
    {
        str2 = strtok(str1, ",");
        while (str2 != NULL)
        {
            openFlowData.actList[i] = atoi(str2);
            i++;
            str2 = strtok(NULL, ",");
        }
        openFlowData.actLen = i;
    }
    sscanf(metaData, "%d|%d", &metaDataAction, &metaDataActionMask);
    openFlowData.metaData = metaDataAction;
    openFlowData.metaDataMask = metaDataActionMask;
    addEntry(&configFile.pureOpenFlowData, &openFlowData,
             sizeof(OpenFlowDataEntry));
    return;
}

void processPureOfActionData(int lineNum, char *line)
{
    OpenFlowActionEntry openFlowDataAct;
    int32_t actId, actType, ethType, mplsLabel, mplsTC, mplsBOS, arpOP, ipDSCP,
            ipECN, icmpType, icmpCode, pbbISID, pbbUCA, port, pktMaxLen, vlanVID, l4Port,
            vlanPCP, setTTL, mpls, mplsTTL;
    int32_t subActType;

    char *str1, *str2, *str3, *str4;
    unsigned int ethAddr[XP_MAC_ADDR_LEN];
    int ipv4Addr[4];
    char key[50] = {0}, data[50]= {0}, mask[3] = {0};
    char actDataMask[200] = {0};

    memset(&openFlowDataAct, 0x0, sizeof(OpenFlowActionEntry));

    sscanf(line, "%d %d %d %199s",
           &actId, &actType, &subActType, actDataMask);
    openFlowDataAct.actId = actId;
    if (actType< 0 || (actType> 16 && !(actType == 30 || actType == 31 ||
                                        actType == 32 || actType == 19 || actType == 20)))
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter actType data in valid format\n");
        return;
    }
    openFlowDataAct.actType = actType;

    if (subActType < -1 || (subActType> 39 && subActType != 42))
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter subActType data in valid format\n");
        return;
    }
    openFlowDataAct.subActType = subActType;

    str1 = actDataMask;
    str2 = strtok(str1, ",");
    //TODO: Need to add suport for NONE
    while (str2 != NULL)
    {
        str3 = strstr(str2, "->");
        if (str3)
        {
            strncpy(key, str2, str3-str2);
            str4 = strstr(str2, "|");
            if (str4 != NULL)
            {
                strncpy(data, str3 +2, (str4-str3 - 2));
                strncpy(mask, str4+1, strlen(str4)-1);
            }
            else
            {
                strcpy(data, str3);
            }
        }
        else
        {
            printf("ERROR: Invalid Input for Action Data Mask.\n");
        }

        if (strcmp(key, "ethAddr") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x", &ethAddr[0], &ethAddr[1], &ethAddr[2],
                   &ethAddr[3], &ethAddr[4], &ethAddr[5]);
            int i;
            int maskNumByte = atoi(mask)/8;
            int maskNumBit = atoi(mask)%8;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if (ethAddr[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter ethAddr data in valid format\n");
                    return;
                }
                openFlowDataAct.ofActionData.ethAddr[i]= ethAddr[i];
                if (mask[0] != '0')
                {
                    if (i < maskNumByte)
                    {
                        openFlowDataAct.ofActionMask.ethAddr[i]= 0x00;
                    }
                    if ((i == maskNumByte) && (maskNumBit != 0))
                    {
                        openFlowDataAct.ofActionMask.ethAddr[i] = ~(uint8_t)pow(2, maskNumBit);
                    }
                }
                else
                {
                    openFlowDataAct.ofActionMask.ethAddr[i] = 0;
                }


            }
        }
        else if (strcmp(key, "ethType") == 0)
        {
            sscanf(data, "%d", &ethType);
            openFlowDataAct.ofActionData.ethType = ethType;
            sscanf(mask, "%d", &ethType);
            openFlowDataAct.ofActionMask.ethType = ethType;
        }
        else if (strcmp(key, "vlanVID") == 0)
        {
            sscanf(data, "%d", &vlanVID);

            if (vlanVID < 0 || vlanVID > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter vlanVID data in valid format\n");
                return;
            }
            openFlowDataAct.ofActionData.vlanVID = vlanVID;
            vlanVID = 0;
            sscanf(mask, "%d", &vlanVID);
            openFlowDataAct.ofActionMask.vlanVID = vlanVID;
        }

        else if (strcmp(key, "vlanPCP") == 0)
        {
            sscanf(data, "%d", &vlanPCP);

            if (vlanPCP < 0 || vlanPCP > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter vlanPCP data in valid format\n");
                return;
            }
            openFlowDataAct.ofActionData.vlanPCP = vlanPCP;
            sscanf(mask, "%d", &vlanPCP);
            openFlowDataAct.ofActionMask.vlanPCP = vlanPCP;

        }
        else if (strcmp(key, "mplsLabel") == 0)
        {
            sscanf(data, "%d", &mplsLabel);
            openFlowDataAct.ofActionData.mplsLabel = mplsLabel;
            sscanf(mask, "%d", &mplsLabel);
            openFlowDataAct.ofActionMask.mplsLabel = mplsLabel;
        }
        else if (strcmp(key, "mpls") == 0)
        {
            sscanf(data, "%d", &mpls);
            openFlowDataAct.ofActionData.mpls = mpls;
            sscanf(mask, "%d", &mpls);
            openFlowDataAct.ofActionMask.mpls = mpls;
        }
        else if (strcmp(key, "mplsTTL") == 0)
        {
            sscanf(data, "%d", &mplsTTL);
            openFlowDataAct.ofActionData.mplsTTL = mplsTTL;
            sscanf(mask, "%d", &mpls);
            openFlowDataAct.ofActionMask.mplsTTL = mplsTTL;
        }
        else if (strcmp(key, "mplsTC") == 0)
        {
            sscanf(data, "%d", &mplsTC);
            openFlowDataAct.ofActionData.mplsTC = mplsTC;
            sscanf(mask, "%d", &mplsTC);
            openFlowDataAct.ofActionMask.mplsTC = mplsTC;
        }
        else if (strcmp(key, "mplsBOS") == 0)
        {
            sscanf(data, "%d", &mplsBOS);
            openFlowDataAct.ofActionData.mplsBOS = mplsBOS;
            sscanf(mask, "%d", &mplsBOS);
            openFlowDataAct.ofActionMask.mplsBOS = mplsBOS;
        }
        else if (strcmp(key, "arpOP") == 0)
        {
            sscanf(data, "%d", &arpOP);
            openFlowDataAct.ofActionData.arpOP = arpOP;
            sscanf(mask, "%d", &arpOP);
            openFlowDataAct.ofActionMask.arpOP = arpOP;
        }
        else if (strcmp(key, "ipv4Addr") == 0)
        {
            sscanf(data, "%d.%d.%d.%d", &ipv4Addr[3], &ipv4Addr[2], &ipv4Addr[1],
                   &ipv4Addr[0]);
            int i;
            int maskNumByte = atoi(mask)/8;
            int maskNumBit = atoi(mask)%8;
            for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
            {
                if (ipv4Addr[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter ipv4Addr data in valid format\n");
                    return;
                }
                openFlowDataAct.ofActionData.ipv4Addr[3-i] = ipv4Addr[i];
                if (mask[0] != '0')
                {
                    if (i < maskNumByte)
                    {
                        openFlowDataAct.ofActionMask.ipv4Addr[i]= 0x00;
                    }
                    if ((i == maskNumByte) && (maskNumBit != 0))
                    {
                        openFlowDataAct.ofActionMask.ipv4Addr[i] = ~(uint8_t)pow(2, maskNumBit);
                    }
                }
                else
                {
                    openFlowDataAct.ofActionMask.ipv4Addr[i] = 0;
                }
            }
        }
        else if (strcmp(key, "ipDSCP") == 0)
        {
            sscanf(data, "%d", &ipDSCP);
            openFlowDataAct.ofActionData.ipDSCP = ipDSCP;
            sscanf(mask, "%d", &ipDSCP);
            openFlowDataAct.ofActionMask.ipDSCP = ipDSCP;
        }
        else if (strcmp(key, "ipECN") == 0)
        {
            sscanf(data, "%d", &ipECN);
            openFlowDataAct.ofActionData.ipECN = ipECN;
            sscanf(mask, "%d", &ipECN);
            openFlowDataAct.ofActionMask.ipECN = ipECN;
        }
        else if (strcmp(key, "icmpType") == 0)
        {
            sscanf(data, "%d", &icmpType);
            openFlowDataAct.ofActionData.icmpType = icmpType;
            sscanf(mask, "%d", &icmpType);
            openFlowDataAct.ofActionMask.icmpType = icmpType;
        }
        else if (strcmp(key, "icmpCode") == 0)
        {
            sscanf(data, "%d", &icmpCode);
            openFlowDataAct.ofActionData.icmpCode = icmpCode;
            sscanf(mask, "%d", &icmpCode);
            openFlowDataAct.ofActionMask.icmpCode = icmpCode;
        }
        else if (strcmp(key, "l4Port") == 0)
        {
            sscanf(data, "%d", &l4Port);
            openFlowDataAct.ofActionData.l4Port = l4Port;
            sscanf(mask, "%d", &l4Port);
            openFlowDataAct.ofActionMask.l4Port = l4Port;
        }
        else if (strcmp(key, "pbbISID") == 0)
        {
            sscanf(data, "%d", &pbbISID);
            openFlowDataAct.ofActionData.pbbISID = pbbISID;
            sscanf(mask, "%d", &pbbISID);
            openFlowDataAct.ofActionMask.pbbISID = pbbISID;
        }
        else if (strcmp(key, "pbbUCA") == 0)
        {
            sscanf(data, "%d", &pbbUCA);
            openFlowDataAct.ofActionData.pbbUCA = pbbUCA;
            sscanf(mask, "%d", &pbbUCA);
            openFlowDataAct.ofActionMask.pbbUCA = pbbUCA;
        }
        else if (strcmp(key, "port") == 0)
        {
            sscanf(data, "%d", &port);
            openFlowDataAct.ofActionData.port = port;
            sscanf(mask, "%d", &port);
            openFlowDataAct.ofActionMask.port = port;
        }
        else if (strcmp(key, "pktMaxLen") == 0)
        {
            sscanf(data, "%d", &pktMaxLen);
            openFlowDataAct.ofActionData.pktMaxLen = pktMaxLen;
            sscanf(mask, "%d", &pktMaxLen);
            openFlowDataAct.ofActionMask.pktMaxLen = pktMaxLen;
        }
        else if (strcmp(key, "setTTL") == 0)
        {
            sscanf(data, "%d", &setTTL);
            openFlowDataAct.ofActionData.setTTL = setTTL;
            sscanf(mask, "%d", &setTTL);
            openFlowDataAct.ofActionMask.setTTL = setTTL;
        }
        else
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("enter keyAttrubute data (Unknown Attrubute) in valid format\n");
            //return;
        }
        //str1 = str2;
        //str2 = strstr(str1, ",");
        str2 = strtok(NULL, ",");
        memset(data, 0x00, sizeof(data));
        memset(mask, 0x00, sizeof(mask));
        memset(key, 0x00, sizeof(key));
    }
    addEntry(&configFile.pureOpenFlowActionData, &openFlowDataAct,
             sizeof(OpenFlowActionEntry));
    return;
}

/**
 * \fn processIpvxIaclData
 * \brief Parses line of text from configuration file to determine xpConfigFile::IpvxIaclEntry
 *
 * \param lineNum line number
 * \param line line to be parse
 */
void processIpvxIaclData(int lineNum, char *line)
{
    unsigned int iaclId = 0, enCounter = 0, swicthAclCpuReasonCodeEn = 0,
                 enPolicer = 0, enSampler = 0, enTcRemark = 0, enPcpRemark = 0, enDscpRemark = 0,
                 enPktCmdTrap = 0, enPktCmdLog = 0, enPktCmdDrop = 0, enPbr = 0, enPktCmdUpd = 0,
                 type = 0, ecmpSize = 0, futureUse = 0, mirrorSessionId = 0, vlan = 0,
                 portNum = 0, tc = 0, pcp = 0, dscp = 0, policerId = 0, nextHopId = 0,
                 pktCmd = 0, enDpRemark = 0, dp = 0;
    IpvxIaclData iaclData;
    int reasonCode = 0, numOfIaclDataFields = 28;

    memset(&iaclData, '\0', sizeof(IpvxIaclData));
    if ((sscanf(line,
                "%u %u %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u %u %u %d %u %u %u %u %u %u %u %u %u %u",
                &iaclId, &type, &enCounter, &swicthAclCpuReasonCodeEn, &enPolicer, &enSampler,
                &enTcRemark, &enPcpRemark, &enDscpRemark, &enPktCmdTrap, &enPktCmdLog,
                &enPktCmdDrop, &enPbr, &enPktCmdUpd, &enDpRemark, &ecmpSize, &futureUse,
                &reasonCode, &mirrorSessionId, &vlan, &portNum, &tc, &pcp, &dscp, &policerId,
                &nextHopId, &pktCmd, &dp)) != numOfIaclDataFields)
    {
        printf("ERROR: Number of fields for Iacl Data are less than expected fields\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }

    if (type > 5)
    {
        printf("ERROR: Invalid type - should be in between 0 to 5\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.type = type;

    if (ecmpSize > 15)
    {
        printf("ERROR: Invalid ecmpSize - should be in between 0 to 15\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.ecmpSize = ecmpSize;

    if (futureUse > 511)
    {
        printf("ERROR: Invalid futureUse - should be in between 0 to 511\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.futureUse = futureUse;

    if (reasonCode > 1023)
    {
        printf("ERROR: Invalid reasonCode - should be in between 0 to 1023\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.reasonCode = reasonCode;

    if (mirrorSessionId > 255)
    {
        printf("ERROR: Invalid mirrorSessionId - should be in between 0 to 255\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.mirrorSessionId = mirrorSessionId;

    if (vlan > 4095)
    {
        printf("ERROR: Invalid vlan - should be in between 0 to 4095\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.vlan = vlan;

    if (portNum > 71)
    {
        printf("ERROR: Invalid portNum - should be in between 0 to 71\n");
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.portNum = portNum;

    if (tc > 15)
    {
        printf("ERROR: Invalid tc - should be in between 0 to 15\n");
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.tc = tc;

    if (pcp > 7)
    {
        printf("ERROR: Invalid pcp - should be in between 0 to 7\n");
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.pcp = pcp;

    if (dscp > 63)
    {
        printf("ERROR: Invalid dscp - should be in between 0 to 63\n");
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.dscp = dscp;

    if (policerId > 1023)
    {
        printf("ERROR: Invalid policerId - should be in between 0 to 1023\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.policerId = policerId;

    if (nextHopId > 65535)
    {
        printf("ERROR: Invalid nextHopId - should be in between 0 to 65535\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.nextHopId = nextHopId;

    if (pktCmd > 3)
    {
        printf("ERROR: Invalid pktCmd - should be in between 0 to 3\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.pktCmd = pktCmd;

    if (enCounter > 1)
    {
        printf("ERROR: Invalid enCounter - should be 0 or 1\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.enableData.enCounter = enCounter;

    if (swicthAclCpuReasonCodeEn > 1)
    {
        printf("ERROR: Invalid swicthAclCpuReasonCodeEn - should be 0 or 1\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.enableData.swicthAclCpuReasonCodeEn = swicthAclCpuReasonCodeEn;

    if (enPolicer > 1)
    {
        printf("ERROR: Invalid enPolicer - should be 0 or 1\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.enableData.enPolicer = enPolicer;

    if (enSampler > 1)
    {
        printf("ERROR: Invalid enSampler - should be 0 or 1\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.enableData.enSampler = enSampler;

    if (enTcRemark > 1)
    {
        printf("ERROR: Invalid enTcRemark - should be 0 or 1\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.enableData.enTcRemark = enTcRemark;

    if (enPcpRemark > 1)
    {
        printf("ERROR: Invalid enPcpRemark - should be 0 or 1\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.enableData.enPcpRemark = enPcpRemark;

    if (enDscpRemark > 1)
    {
        printf("ERROR: Invalid enDscpRemark - should be 0 or 1\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.enableData.enDscpRemark = enDscpRemark;

    if (enPktCmdTrap > 1)
    {
        printf("ERROR: Invalid enPktCmdTrap - should be 0 or 1\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.enableData.enPktCmdTrap = enPktCmdTrap;

    if (enPktCmdLog > 1)
    {
        printf("ERROR: Invalid enPktCmdLog - should be 0 or 1\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.enableData.enPktCmdLog = enPktCmdLog;

    if (enPktCmdDrop > 1)
    {
        printf("ERROR: Invalid enPktCmdDrop - should be 0 or 1\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.enableData.enPktCmdDrop = enPktCmdDrop;

    if (enPbr > 1)
    {
        printf("ERROR: Invalid enPbr - should be 0 or 1\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.enableData.enPbr = enPbr;

    if (enPktCmdUpd > 1)
    {
        printf("ERROR: Invalid enPktCmdUpd - should be 0 or 1\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclData.enableData.enPktCmdUpd = enPktCmdUpd;

    addEntry(&configFile.ipvxIaclData, &iaclData, sizeof(IpvxIaclData));

    //for debugging the table configuration data
    if (CONFIG_DEBUG_ENABLE)
    {
        printf("%s: Input config - %s > Parsing line number - %d\n=====================>\n",
               __func__, configFile.filename, lineNum);
        printf("iaclId : %u\ntype : %u\nenCounter : %u\nswicthAclCpuReasonCodeEn : %u\nenPolicer : %u\nenSampler : %u\nenTcRemark : %u\nenPcpRemark : %u\nenDscpRemark : %u\nenPktCmdTrap : %u\nenPktCmdLog : %u\nenPktCmdDrop : %u\nenPbr : %u\nenPktCmdUpd : %u\necmpSize : %u\nfutureUse : %u\nreasonCode : %u\nmirrorSessionId : %x\nvlan : %u\nportNum : %u\ntc : %u\npcp : %u\ndscp : %u\npolicerId : %u\nnextHopId : %u\npktCmd : %u\n",
               iaclId, iaclData.type, iaclData.enableData.enCounter,
               iaclData.enableData.swicthAclCpuReasonCodeEn, iaclData.enableData.enPolicer,
               iaclData.enableData.enSampler, iaclData.enableData.enTcRemark,
               iaclData.enableData.enPcpRemark, iaclData.enableData.enDscpRemark,
               iaclData.enableData.enPktCmdTrap, iaclData.enableData.enPktCmdLog,
               iaclData.enableData.enPktCmdDrop, iaclData.enableData.enPbr,
               iaclData.enableData.enPktCmdUpd, iaclData.ecmpSize, iaclData.futureUse,
               iaclData.reasonCode, iaclData.mirrorSessionId, iaclData.vlan, iaclData.portNum,
               iaclData.tc, iaclData.pcp, iaclData.dscp, iaclData.policerId,
               iaclData.nextHopId, iaclData.pktCmd);
        printf("<=====================\n");
    }
}

/**
 * \fn processIpvxIaclKey
 * \private
 * \brief Parses line of text from configuration file to determine xpConfigFile::IpvxIaclKey
 *
 * \param lineNum line number
 * \param line line to be parsed
 */
void processIpvxIaclKey(int lineNum, char *line)
{
    signed int iaclId = 0, type = 0, priority = 0, sourcePort = 0,
               cTagVid = 0, cTagDei = 0, cTagPcp = 0,
               fragment = 0, nextHeader = 0, hopLimit = 0, l4DestPort = 0, l4SourcePort = 0,
               tcpFlags = 0, ipType = 0, DSCP = 0,
               vlanId = 0, ipVersion = 0, egressVif = 0, egressBd = 0, TC = 0, DP = 0,
               aclKeyType = 0, isUcastRouted = 0, icmpCode = 0, icmpType = 0, ECN = 0;
    unsigned int etherType = 0;
    char keyAttribute[350] = {'\0'}, key[50] = {'\0'}, data[50] = {'\0'}, mask[30]
                                                                  = {'\0'};
    char *str1 = NULL, *str2 = NULL, *str3 = NULL, *str4 = NULL;
    int numOfIaclKeyFields = 6;
    unsigned int macDA[6] = {0}, macSA[6] = {0};
    unsigned int macDAmask[6] = {0}, macSAmask[6] = {0};
    unsigned int v4SIP[4] = {0}, v4DIP[4] = {0}, v4SIPmask[4] = {0}, v4DIPmask[4] = {0};
    unsigned int v6SIP[16] = {0}, v6DIP[16] = {0};
    IpvxIaclKeyAttribute iaclKey = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, {0}, {0}, {{0}}, {{0}}, {0}};
    IpvxIaclMaskAttribute iaclMask;

    memset(&iaclMask,    0xFF, sizeof(IpvxIaclMaskAttribute));

    if (sscanf(line, "%d %d %d %d %d %349s", &iaclId, &type, &ipVersion, &priority,
               &aclKeyType, keyAttribute) != numOfIaclKeyFields)
    {
        printf("ERROR: Number of fields for Iacl key are less than expected fields\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }

    if ((iaclId < 1) || (iaclId > 255))
    {
        printf("ERROR: Invalid iaclId - should be in between 1 to 255\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclKey.iaclId = iaclId;
    iaclMask.iaclId = 0;

    if ((type < 0) || (type > 5))
    {
        printf("ERROR: Invalid type - should be in between 0 to 5\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclKey.type = type;

    if ((aclKeyType < 0) || (aclKeyType > 1))
    {
        printf("ERROR: Invalid aclKeyType - should be 0 or 1\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclKey.aclKeyType = aclKeyType;
    iaclMask.aclKeyType = 0;

    //ipVersion = 0 - IPV4,1 - IPv6
    if ((ipVersion < 0) || (ipVersion > 1))
    {
        printf("ERROR: Invalid ipVersion - should be 0 or 1\n");
        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        return;
    }
    iaclKey.ipVersion = ipVersion;
    iaclKey.priority = priority;

    str1 = keyAttribute;
    str2 = strtok(str1, ",");

    while (str2 != NULL)
    {
        str3 = strstr(str2, "->");
        if (str3)
        {
            strncpy(key, str2, str3-str2);
            str4 = strstr(str2, "|");
            if (str4)
            {
                strncpy(data, str3 +2, (str4-str3 - 2));
                strncpy(mask, str4+1, (strlen(str4) - 1));
            }
            else
            {
                printf("ERROR: Invalid input for keyAttribute.\n");
                return;
            }
        }
        else
        {
            printf("ERROR: Invalid input.\n");
            return;
        }

        if (strcmp(key, "macDA") == 0)
        {
            if (sscanf(data, "%x:%x:%x:%x:%x:%x", &macDA[5], &macDA[4], &macDA[3],
                       &macDA[2], &macDA[1], &macDA[0]) != XP_MAC_ADDR_LEN)
            {
                printf("ERROR: Invalid macDA - Please enter in valid format. e.g- 11:22:33:44:55:66\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            if (strlen(mask) != 1)
            {
                sscanf(mask, "%02x:%02x:%02x:%02x:%02x:%02x", &macDAmask[5], &macDAmask[4],
                       &macDAmask[3], &macDAmask[2], &macDAmask[1], &macDAmask[0]);
            }
            int i;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if ((macDA[i] > 255) && (macDAmask[i] > 255))
                {
                    printf("ERROR: Invalid macDA - Please enter in valid format. e.g- 11:22:33:44:55:66\n");
                    printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    return;
                }
                iaclKey.macDA[i]= macDA[i];
                if (strlen(mask) == 1)
                {
                    if (mask[0] == '0')
                    {
                        iaclMask.macDA[i]= 0;
                    }
                }
                else
                {
                    iaclMask.macDA[i]= macDAmask[i];
                }
            }
            iaclKey.flags.isDAset = 1;
        }
        else if (strcmp(key, "macSA") == 0)
        {
            if (sscanf(data, "%x:%x:%x:%x:%x:%x", &macSA[5], &macSA[4], &macSA[3],
                       &macSA[2], &macSA[1], &macSA[0]) != XP_MAC_ADDR_LEN)
            {
                printf("ERROR: Invalid macSA - Please enter in valid format. e.g- 11:22:33:44:55:66\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            if (strlen(mask) != 1)
            {
                sscanf(mask, "%02x:%02x:%02x:%02x:%02x:%02x", &macSAmask[5], &macSAmask[4],
                       &macSAmask[3], &macSAmask[2], &macSAmask[1], &macSAmask[0]);
            }
            int i;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if (macSA[i] > 255)
                {
                    printf("ERROR: Invalid macSA - Please enter in valid format. e.g- 11:22:33:44:55:66\n");
                    printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    return;
                }
                iaclKey.macSA[i]= macSA[i];
                if (strlen(mask) == 1)
                {
                    if (mask[0] == '0')
                    {
                        iaclMask.macSA[i]= 0;
                    }
                }
                else
                {
                    iaclMask.macSA[i]= macSAmask[i];
                }
            }
            iaclKey.flags.isSAset = 1;
        }
        else if (strcmp(key, "SourcePort") == 0)
        {
            sscanf(data, "%d", &sourcePort);
            if ((sourcePort < 0) || (sourcePort > 255))
            {
                printf("ERROR: Invalid sourcePort - should be in between 0 to 255\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.sourcePort = sourcePort;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.sourcePort);
            }
        }
        else if (strcmp(key, "etherType") == 0)
        {
            sscanf(data, "%x", &etherType);
            if ((etherType > 65535))
            {
                printf("ERROR: Invalid etherType - should be in between 0 to 65535\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.etherType = etherType;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.etherType);
            }
        }
        else if (strcmp(key, "cTagVid") == 0)
        {
            sscanf(data, "%d", &cTagVid);
            if ((cTagVid < 0) || cTagVid > 4095)
            {
                printf("ERROR: Invalid cTagVid - should be in between 0 to 4095\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.cTagVid = cTagVid;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.cTagVid);
            }
        }
        else if (strcmp(key, "cTagDei") == 0)
        {
            sscanf(data, "%d", &cTagDei);
            if ((cTagDei < 0) || (cTagDei > 1))
            {
                printf("ERROR: Invalid cTagDei - should be 0 or 1\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.cTagDei = cTagDei;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.cTagDei);
            }
        }
        else if (strcmp(key, "cTagPcp") == 0)
        {
            sscanf(data, "%d", &cTagPcp);
            if ((cTagPcp < 0) || (cTagPcp > 7))
            {
                printf("ERROR: Invalid cTagPcp - should be in between 0 to 7\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.cTagPcp = cTagPcp;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.cTagPcp);
            }
        }
        else if (strcmp(key, "DIP") == 0)
        {
            if (ipVersion == 0) //ipVersion = 0 - IPV4, 1 - IPV6
            {
                if (sscanf(data, "%u.%u.%u.%u", &v4DIP[3], &v4DIP[2], &v4DIP[1],
                           &v4DIP[0]) != XP_IPV4_ADDR_LEN)
                {
                    printf("ERROR: Invalid DIP - should be in Ipv4 format\n");
                    printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    return;
                }
                if (strlen(mask) != 1)
                {
                    if (sscanf(mask, "%u.%u.%u.%u", &v4DIPmask[3], &v4DIPmask[2], &v4DIPmask[1],
                               &v4DIPmask[0]) != XP_IPV4_ADDR_LEN)
                    {
                        printf("ERROR: Invalid DIP Mask\n");
                        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                               lineNum, configFile.filename);
                        printf("%s\n", line);
                        return;
                    }
                }

                int i;
                for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
                {
                    if (v4DIP[i] > 255)
                    {
                        printf("ERROR: Invalid DIP - should be in Ipv4 format\n");
                        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                               lineNum, configFile.filename);
                        printf("%s\n", line);
                        return;
                    }
                    iaclKey.dip.v4DIP[i] = v4DIP[i];
                    if (strlen(mask) == 1)
                    {
                        if (mask[0] == '0')
                        {
                            iaclMask.dip.v4DIP[i]= 0x0;
                        }
                    }
                    else
                    {
                        iaclMask.dip.v4DIP[i]= v4DIPmask[i];
                    }
                }
                iaclKey.flags.isDIPset = 1;
            }
            else
            {
                if (sscanf(data, "%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x", &v6DIP[15],
                           &v6DIP[14], &v6DIP[13], &v6DIP[12], &v6DIP[11], &v6DIP[10], &v6DIP[9],
                           &v6DIP[8], &v6DIP[7], &v6DIP[6], &v6DIP[5], &v6DIP[4], &v6DIP[3], &v6DIP[2],
                           &v6DIP[1], &v6DIP[0]) != XP_IPV6_ADDR_LEN)
                {
                    printf("ERROR: Invalid DIP - should be in Ipv6 format\n");
                    printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    return;
                }

                int i;
                for (i = 0; i < XP_IPV6_ADDR_LEN; i++)
                {
                    if (v6DIP[i] > 255)
                    {
                        printf("ERROR: Invalid DIP - should be in Ipv6 format\n");
                        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                               lineNum, configFile.filename);
                        printf("%s\n", line);
                        return;
                    }
                    iaclKey.dip.v6DIP[i] = v6DIP[i];
                    if (mask[0] == '0')
                    {
                        iaclMask.dip.v6DIP[i] = 0x0;
                    }
                }
                iaclKey.flags.isDIPset = 1;
            }
        }
        else if (strcmp(key, "SIP") == 0)
        {
            if (ipVersion == 0) //ipVersion = 0 - IPV4, 1 - IPV6
            {

                if (sscanf(data, "%u.%u.%u.%u", &v4SIP[3], &v4SIP[2], &v4SIP[1],
                           &v4SIP[0]) != XP_IPV4_ADDR_LEN)
                {
                    printf("ERROR: Invalid SIP - should be in Ipv4 format\n");
                    printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    return;
                }
                if (strlen(mask) != 1)
                {
                    if (sscanf(mask, "%u.%u.%u.%u", &v4SIPmask[3], &v4SIPmask[2], &v4SIPmask[1],
                               &v4SIPmask[0]) != XP_IPV4_ADDR_LEN)
                    {
                        printf("ERROR: Invalid SIP Mask\n");
                        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                               lineNum, configFile.filename);
                        printf("%s\n", line);
                        return;
                    }
                }

                int i;
                for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
                {
                    if (v4SIP[i] > 255)
                    {
                        printf("ERROR: Invalid v4SIP - should be in between 0 to 255\n");
                        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                               lineNum, configFile.filename);
                        printf("%s\n", line);
                        return;
                    }
                    iaclKey.sip.v4SIP[i] = v4SIP[i];
                    if (strlen(mask) == 1)
                    {
                        if (mask[0] == '0')
                        {
                            iaclMask.sip.v4SIP[i]= 0x0;
                        }
                    }
                    else
                    {
                        iaclMask.sip.v4SIP[i]= v4SIPmask[i];
                    }
                }
                iaclKey.flags.isSIPset = 1;
            }
            else
            {
                if (sscanf(data, "%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x", &v6SIP[15],
                           &v6SIP[14], &v6SIP[13], &v6SIP[12], &v6SIP[11], &v6SIP[10], &v6SIP[9],
                           &v6SIP[8], &v6SIP[7], &v6SIP[6], &v6SIP[5], &v6SIP[4], &v6SIP[3], &v6SIP[2],
                           &v6SIP[1], &v6SIP[0]) != XP_IPV6_ADDR_LEN)
                {
                    printf("ERROR: Invalid SIP - should be in Ipv6 format\n");
                    printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    return;
                }

                int i;
                for (i = 0; i < XP_IPV6_ADDR_LEN; i++)
                {
                    if (v6SIP[i] > 255)
                    {
                        printf("ERROR: Invalid SIP - should be in Ipv6 format\n");
                        printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                               lineNum, configFile.filename);
                        printf("%s\n", line);
                        return;
                    }
                    iaclKey.sip.v6SIP[i] = v6SIP[i];
                    if (mask[0] == '0')
                    {
                        iaclMask.sip.v6SIP[i] = 0x0;
                    }
                }
                iaclKey.flags.isSIPset = 1;
            }
        }
        else if (strcmp(key, "L4DestPort") == 0)
        {
            sscanf(data, "%d", &l4DestPort);
            if ((l4DestPort < 0) || (l4DestPort > 65535))
            {
                printf("ERROR: Invalid l4DestPort - should be in between 0 to 65535\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.l4DestPort = l4DestPort;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.l4DestPort);
            }
        }
        else if (strcmp(key, "L4SourcePort") == 0)
        {
            sscanf(data, "%d", &l4SourcePort);
            if ((l4SourcePort < 0) || (l4SourcePort > 65535))
            {
                printf("ERROR: Invalid l4SourcePort - should be in between 0 to 65535\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.l4SourcePort = l4SourcePort;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.l4SourcePort);
            }
        }
        else if (strcmp(key, "icmpCode") == 0)
        {
            sscanf(data, "%d", &icmpCode);
            if ((icmpCode < 0) || (icmpCode > 256))
            {
                printf("ERROR: Invalid icmpCode - should be in between 0 to 255\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.icmpCode = icmpCode;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.icmpCode);
            }
        }
        else if (strcmp(key, "icmpType") == 0)
        {
            sscanf(data, "%d", &icmpType);
            if ((icmpType < 0) || (icmpType > 256))
            {
                printf("ERROR: Invalid icmpType - should be in between 0 to 255\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.icmpType = icmpType;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.icmpType);
            }
        }
        else if (strcmp(key, "vlanId") == 0)
        {
            sscanf(data, "%d", &vlanId);
            if ((vlanId < 0) || (vlanId > 4095))
            {
                printf("ERROR: Invalid vlanId - should be in between 0 to 4095\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.vlanId = vlanId;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.vlanId);
            }
        }
        else if (strcmp(key, "Fragment") == 0)
        {
            sscanf(data, "%d", &fragment);
            if ((fragment < 0) || (fragment > 3))
            {
                printf("ERROR: Invalid fragment - should be in between 0 to 3\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.fragment = fragment;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.fragment);
            }
        }
        else if (strcmp(key, "NextHeader") == 0)
        {
            sscanf(data, "%d", &nextHeader);
            if ((nextHeader < 0) || (nextHeader > 255))
            {
                printf("ERROR: Invalid nextHeader - should be in between 0 to 255\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.nextHeader = nextHeader;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.nextHeader);
            }
        }
        else if (strcmp(key, "HopLimit") == 0)
        {
            sscanf(data, "%d", &hopLimit);
            if ((hopLimit < 0) || (hopLimit > 255))
            {
                printf("ERROR: Invalid hopLimit - should be in between 0 to 255\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.hopLimit = hopLimit;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.hopLimit);
            }
        }
        else if (strcmp(key, "TcpFlags") == 0)
        {
            sscanf(data, "%d", &tcpFlags);
            if ((tcpFlags < 0) || (tcpFlags > 255))
            {
                printf("ERROR: Invalid tcpFlags - should be in between 0 to 255\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.tcpFlags = tcpFlags;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.tcpFlags);
            }
        }
        else if (strcmp(key, "IpType") == 0)
        {
            sscanf(data, "%d", &ipType);
            if ((ipType < 0) || (ipType > 15))
            {
                printf("ERROR: Invalid IpType - should be in between 0 to 15\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.ipType = ipType;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.ipType);
            }
        }
        else if (strcmp(key, "DSCP") == 0)
        {
            sscanf(data, "%d", &DSCP);
            if ((DSCP < 0) || (DSCP > 63))
            {
                printf("ERROR: Invalid DSCP - should be in between 0 to 63\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.DSCP = DSCP;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.DSCP);
            }
        }
        else if (strcmp(key, "ECN") == 0)
        {
            sscanf(data, "%d", &ECN);
            if ((ECN < 0) || (ECN > 3))
            {
                printf("ERROR: Invalid ECN - should be in between 0 to 3\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.ECN = ECN;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.ECN);
            }
        }
        else if (strcmp(key, "egressVif") == 0)
        {
            sscanf(data, "%d", &egressVif);
            if ((egressVif < 0) || (egressVif > 65535))
            {
                printf("ERROR: Invalid egressVif - should be in between 0 to 65535\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.egressVif = egressVif;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.egressVif);
            }
        }
        else if (strcmp(key, "egressBd") == 0)
        {
            sscanf(data, "%d", &egressBd);
            if ((egressBd < 0) || (egressBd > 65535))
            {
                printf("ERROR: Invalid egressBd - should be in between 0 to 65535\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.egressBd = egressBd;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.egressBd);
            }
        }
        else if (strcmp(key, "TC") == 0)
        {
            sscanf(data, "%d", &TC);
            if ((TC < 0) || (TC > 15))
            {
                printf("ERROR: Invalid TC - should be in between 0 to 15\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.TC = TC;
            if (mask[0] != '1')
            {
                sscanf(mask, "%d", &iaclMask.TC);
            }
        }
        else if (strcmp(key, "DP") == 0)
        {
            sscanf(data, "%d", &DP);
            if ((DP < 0) || (DP > 3))
            {
                printf("ERROR: Invalid DP - should be in between 0 to 3\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.DP = DP;
            if (mask[0] != '1')
            {
                sscanf(mask, "%d", &iaclMask.DP);
            }
        }
        else if (strcmp(key, "isUcastRouted") == 0)
        {
            sscanf(data, "%d", &isUcastRouted);
            if ((DP < 0) || (DP > 1))
            {
                printf("ERROR: Invalid isUcastRouted - should be 0 or 1\n");
                printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                return;
            }
            iaclKey.isUcastRouted = isUcastRouted;
            if (mask[0] != '1')
            {
                sscanf(mask, "%d", &iaclMask.isUcastRouted);
            }
        }
        else
        {
            printf("ERROR: Unknown attribute \"%s\" - Enter key attribute data in valid format\n",
                   key);
            printf("ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            return;
        }
        str2 = strtok(NULL, ",");
        memset(key,  '\0', sizeof(key));
        memset(data, '\0', sizeof(key));
        memset(mask, '\0', sizeof(mask));
    }

    addEntry(&configFile.ipvxIaclKey, &iaclKey, sizeof(IpvxIaclKeyAttribute));
    addEntry(&configFile.ipvxIaclMask, &iaclMask, sizeof(IpvxIaclMaskAttribute));

    if (CONFIG_DEBUG_ENABLE)
    {
        printf("%s: Input config - %s > Parsing line number - %d\n=====================>\n",
               __func__, configFile.filename, lineNum);
        printf("iaclId : %d | mask : %d\ntype : %d\nipVersion : %d\npriority : %daclKeyType : %d\nsourcePort : %d | mask : %d\netherType :%d | mask : %d\ncTagVid : %d | mask : %d\ncTagDei : %d | mask : %d\ncTagPcp : %d | mask : %d\nfragment : %d | mask : %d\nnextHeader : %d | mask : %d\nhopLimit : %d | mask : %d\nl4DestPort : %d | mask : %d\nl4SourcePort : %d | mask : %d\ntcpFlags : %d | mask : %d\nipType : %d | mask : %d\nvlanId : %d | mask : %d\negressVif : %d | mask : %d\negressBd : %d | mask : %d\nTC : %d | mask : %d\nDP : %d | mask : %d\nisDAset : %d\nmacDA : %02x:%02x:%02x:%02x:%02x:%02x | mask : %d\nisSAset : %d\nmacSA : %02x:%02x:%02x:%02x:%02x:%02x | mask : %d\n",
               iaclKey.iaclId, iaclMask.iaclId, iaclKey.type, iaclKey.ipVersion,
               iaclKey.priority, iaclKey.aclKeyType, iaclKey.sourcePort, iaclMask.sourcePort,
               iaclKey.etherType, iaclMask.etherType, iaclKey.cTagVid, iaclMask.cTagVid,
               iaclKey.cTagDei, iaclMask.cTagDei, iaclKey.cTagPcp, iaclMask.cTagPcp,
               iaclKey.fragment, iaclMask.fragment, iaclKey.nextHeader, iaclMask.nextHeader,
               iaclKey.hopLimit, iaclMask.hopLimit, iaclKey.l4DestPort, iaclMask.l4DestPort,
               iaclKey.l4SourcePort, iaclMask.l4SourcePort, iaclKey.tcpFlags,
               iaclMask.tcpFlags, iaclKey.ipType, iaclMask.ipType, iaclKey.vlanId,
               iaclMask.vlanId, iaclKey.egressVif, iaclMask.egressVif, iaclKey.egressBd,
               iaclMask.egressBd, iaclKey.TC, iaclMask.TC, iaclKey.DP, iaclMask.DP,
               iaclKey.flags.isDAset, iaclKey.macDA[5], iaclKey.macDA[4], iaclKey.macDA[3],
               iaclKey.macDA[2], iaclKey.macDA[1], iaclKey.macDA[0], iaclMask.macDA[0],
               iaclKey.flags.isSAset, iaclKey.macSA[5], iaclKey.macSA[4], iaclKey.macSA[3],
               iaclKey.macSA[2], iaclKey.macSA[1], iaclKey.macSA[0], iaclMask.macSA[0]);
        if (iaclKey.ipVersion)
        {
            printf("isSIPset : %d\nv6SIP : %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x | mask : %u\n",
                   iaclKey.flags.isSIPset, iaclKey.sip.v6SIP[15], iaclKey.sip.v6SIP[14],
                   iaclKey.sip.v6SIP[13], iaclKey.sip.v6SIP[12], iaclKey.sip.v6SIP[11],
                   iaclKey.sip.v6SIP[10], iaclKey.sip.v6SIP[9], iaclKey.sip.v6SIP[8],
                   iaclKey.sip.v6SIP[7], iaclKey.sip.v6SIP[6], iaclKey.sip.v6SIP[5],
                   iaclKey.sip.v6SIP[4], iaclKey.sip.v6SIP[3], iaclKey.sip.v6SIP[2],
                   iaclKey.sip.v6SIP[1], iaclKey.sip.v6SIP[0], iaclMask.sip.v6SIP[0]);
            printf("isDIPset : %d\nv6DIP : %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x | mask : %u\n",
                   iaclKey.flags.isDIPset, iaclKey.dip.v6DIP[15], iaclKey.dip.v6DIP[14],
                   iaclKey.dip.v6DIP[13], iaclKey.dip.v6DIP[12], iaclKey.dip.v6DIP[11],
                   iaclKey.dip.v6DIP[10], iaclKey.dip.v6DIP[9], iaclKey.dip.v6DIP[8],
                   iaclKey.dip.v6DIP[7], iaclKey.dip.v6DIP[6], iaclKey.dip.v6DIP[5],
                   iaclKey.dip.v6DIP[4], iaclKey.dip.v6DIP[3], iaclKey.dip.v6DIP[2],
                   iaclKey.dip.v6DIP[1], iaclKey.dip.v6DIP[0], iaclMask.dip.v6DIP[0]);
        }
        else
        {
            printf("isSIPset : %d\nv4SIP : %d.%d.%d.%d | mask : %u.%u.%u.%u\n",
                   iaclKey.flags.isSIPset, iaclKey.sip.v4SIP[3], iaclKey.sip.v4SIP[2],
                   iaclKey.sip.v4SIP[1], iaclKey.sip.v4SIP[0], iaclMask.sip.v4SIP[3],
                   iaclMask.sip.v4SIP[2], iaclMask.sip.v4SIP[1], iaclMask.sip.v4SIP[0]);
            printf("isDIPset : %d\nv4DIP : %d.%d.%d.%d | mask : %u.%u.%u.%u\n",
                   iaclKey.flags.isDIPset, iaclKey.dip.v4DIP[3], iaclKey.dip.v4DIP[2],
                   iaclKey.dip.v4DIP[1], iaclKey.dip.v4DIP[0], iaclMask.dip.v4DIP[3],
                   iaclMask.dip.v4DIP[2], iaclMask.dip.v4DIP[1], iaclMask.dip.v4DIP[0]);
        }
        printf("<=====================\n");
    }
}

/**
 * \fn processL2Ipv4IaclData
 * \private
 * \brief Parses line of text from configuration file to determine xpConfigFile::NatEntry
 *
 * \param int lineNum
 * \param char* line
 */
void processL2Ipv4IaclData(int lineNum, char *line)
{
    int aclId, type, eVifId, isTerminal, enPktCmdUpd, enRedirectToEvif, remarkDp,
        remarkTc, enPolicer, qosOrVlanRw, enMirrorSsnUpd, remarkDscp, remarkPcp, TC,
        mirrorSessionId, policerId, PCP, DSCP, DP, encapType, vlan, portNum;
    unsigned int pktCmdOrEcmpSize, rsnCode;
    //    int  cTagVid, cTagDei, cTagPcp, sTagVid, sTagDei, sTagPcp, dip[4], sip[4], l4DestPort, l4SourcePort, icmpMessageType, BD, dscp, iVif, ipv4DfSet, ipv4MfSet, pktHasCtag, pktHasStag, portNum, vlan, encapType;
    IaclData iaclData;

    memset(&iaclData, 0, sizeof(IaclData));

    sscanf(line,
           "%d %d %d,%d,%d,%d,%d,%d,%d,%d,%d,%d %x %d %d %x %d %d %d %d %d %d %d %d",
           &aclId, &type, &isTerminal, &enPktCmdUpd, &enRedirectToEvif, &remarkDp,
           &remarkTc, &enPolicer, &qosOrVlanRw, &enMirrorSsnUpd, &remarkDscp, &remarkPcp,
           &pktCmdOrEcmpSize, &eVifId, &encapType, &rsnCode, &vlan, &portNum, &DP, &TC,
           &mirrorSessionId, &policerId, &PCP, &DSCP);

    if (type < 0 || type > 2)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter type data in valid format\n");
        return;
    }
    iaclData.type = type;

    if (portNum < 1 || portNum > 127)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter portNum data in valid format\n");
        printf("portNum : %d", portNum);
        return;
    }
    iaclData.portNum = portNum;

    if (enPktCmdUpd < 0 || enPktCmdUpd > 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter enPktCmdUpd data in valid format\n");
        return;
    }
    iaclData.enPktCmdUpd = enPktCmdUpd;

    if (remarkTc< 0 || remarkTc > 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter remarkTc data in valid format\n");
        return;
    }
    iaclData.remarkTc = remarkTc;

    if (enPolicer < 0 || enPolicer > 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter enPolicer data in valid format\n");
        return;
    }
    iaclData.enPolicer = enPolicer;

    if (qosOrVlanRw < 0 || qosOrVlanRw > 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter qosOrVlanRw data in valid format\n");
        return;
    }
    iaclData.qosOrVlanRw = qosOrVlanRw;

    if (enMirrorSsnUpd < 0 || enMirrorSsnUpd > 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter enMirrorSsnUpd data in valid format\n");
        return;
    }
    iaclData.enMirrorSsnUpd = enMirrorSsnUpd;

    if (remarkDscp < 0 || remarkDscp > 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter remarkDscp data in valid format\n");
        return;
    }
    iaclData.remarkDscp = remarkDscp;

    if (remarkPcp < 0 || remarkPcp > 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter remarkPcp data in valid format\n");
        return;
    }
    iaclData.remarkPcp = remarkPcp;

    if (enRedirectToEvif < 0 || enRedirectToEvif > 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter enPktCmdUpd data in valid format\n");
        return;
    }
    iaclData.enRedirectToEvif = enRedirectToEvif;

    if (remarkDp < 0 || remarkDp > 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter remarkDp data in valid format\n");
        return;
    }
    iaclData.remarkDp = remarkDp;

    if (TC < 0 || TC > 15)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter TC data in valid format\n");
        return;
    }
    iaclData.TC = TC;

    if (mirrorSessionId < 0 || mirrorSessionId > 3)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter mirrorSessionId data in valid format\n");
        return;
    }
    iaclData.mirrorSessionId = mirrorSessionId;

    if (policerId < 0 || policerId > 65535)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter policerId data in valid format\n");
        return;
    }
    iaclData.policerId = policerId;

    if (PCP < 0 || PCP > 7)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter PCP data in valid format\n");
        return;
    }
    iaclData.PCP = PCP;

    if (DSCP < 0 || DSCP > 63)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter DSCP data in valid format\n");
        return;
    }
    iaclData.DSCP = DSCP;

    if (pktCmdOrEcmpSize > 7)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter enRedirectToEvif data in valid format\n");
        return;
    }
    iaclData.pktCmdOrEcmpSize = pktCmdOrEcmpSize;

    iaclData.eVifId = eVifId;

    if (encapType< 0 || encapType> 7)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter encapType data in valid format , setting encapType  as 0 \n");
    }
    iaclData.encapType = encapType;

    if (rsnCode > 1023)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter rsnCode data in valid format\n");
        return;
    }
    iaclData.rsnCode = rsnCode;

    if (vlan < 0 || vlan > 4095)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter vlan data in valid format\n");
        return;
    }
    iaclData.vlan = vlan;

    if (isTerminal< 0 || isTerminal> 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter isTerminal data in valid format\n");
        return;
    }
    iaclData.isTerminal = isTerminal;

    if (DP< 0 || DP> 3)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter DP data in valid format\n");
        return;
    }
    iaclData.DP = DP;

    addEntry(&configFile.iaclData, &iaclData, sizeof(IaclData));
}

/**
 * \fn processL2Ipv4IaclKey
 * \private
 * \brief Parses line of text from configuration file to determine xpConfigFile::IaclKey
 *
 * \param int lineNum
 * \param char* line
 */
void processL2Ipv4IaclKey(int lineNum, char *line)
{
    unsigned int aclId, tableId, priority;
    int  cTagVid, cTagDei, cTagPcp, sTagVid, sTagDei, sTagPcp, dip[4], dipMask[4],
         sip[4], sipMask[4], l4DestPort, l4SourcePort, icmpMessageType, BD, dscp, iVif,
         ipv4DfSet, ipv4MfSet, pktHasCtag, pktHasStag;
    unsigned int macDA[6], macDAMask[6], macSA[6],  macSAMask[6], etherType,
             protocol;
    char  keyAttribute[300];
    char *str1, *str2, *str3, *str4;
    char key[50], data[50], mask[2];
    IaclL2Ipv4KeyAttribute iaclKey = {-1, {0,}, {0,}, -1, -1, -1, -1, -1, -1, -1, {0,}, {0,}, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0};
    IaclL2Ipv4MaskAttribute iaclMask;

    memset(keyAttribute, 0, sizeof(keyAttribute));
    memset(key, 0, sizeof(key));
    memset(data, 0, sizeof(data));
    memset(mask, 0, sizeof(mask));
    memset(macDAMask, 0xFF, sizeof(macDAMask));
    memset(&iaclMask, 0xFF, sizeof(IaclL2Ipv4MaskAttribute));

    sscanf(line, "%u %u %u %299s",
           &aclId, &tableId, &priority, keyAttribute);

    if (aclId <= 0)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter type data in valid format\n");
        return;
    }

    iaclKey.iAclId = aclId;
    iaclKey.tableId = tableId;
    iaclKey.priority = priority;
    iaclMask.iAclId = 0;

    str1 = keyAttribute;
    str2 = strtok(str1, ",");

    while (str2 != NULL)
    {
        str3 = strstr(str2, "->");
        if (str3)
        {
            strncpy(key, str2, str3-str2);
            str4 = strstr(str2, "|");
            if (str4)
            {
                strncpy(data, str3 +2, (str4-str3 - 2));
                strncpy(mask, str4+1, (strlen(str4) - 1));
            }
            else
            {
                printf("ERROR: Invalid input for keyAttribute.\n");
                return;
            }
        }
        else
        {
            printf("ERROR: Invalid input.\n");
            return;
        }

        if (strcmp(key, "macDA") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x", &macDA[5], &macDA[4], &macDA[3], &macDA[2],
                   &macDA[1], &macDA[0]);
            if (strlen(mask) != 1)
            {
                sscanf(mask, "%02x:%02x:%02x:%02x:%02x:%02x", &macDAMask[5], &macDAMask[4],
                       &macDAMask[3], &macDAMask[2], &macDAMask[1], &macDAMask[0]);
            }
            int i;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if ((macDA[i] > 255) && (macDAMask[i] > 255))
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter macDA data in valid format\n");
                    return;
                }
                iaclKey.macDA[i]= macDA[i];
                if (strlen(mask) == 1)
                {
                    if (mask[0] == '0')
                    {
                        iaclMask.macDA[i]= 0;
                    }
                }
                else
                {
                    iaclMask.macDA[i]= macDAMask[i];
                }
            }

            iaclKey.isDmacSet = 1;
        }
        else if (strcmp(key, "macSA") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x", &macSA[5], &macSA[4], &macSA[3], &macSA[2],
                   &macSA[1], &macSA[0]);
            if (strlen(mask) != 1)
            {
                sscanf(mask, "%02x:%02x:%02x:%02x:%02x:%02x", &macSAMask[5], &macSAMask[4],
                       &macSAMask[3], &macSAMask[2], &macSAMask[1], &macSAMask[0]);
            }
            int i;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if (macSA[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter macSA data in valid format\n");
                    return;
                }
                iaclKey.macSA[i]= macSA[i];
                if (strlen(mask) == 1)
                {
                    if (mask[0] == '0')
                    {
                        iaclMask.macSA[i]= 0;
                    }
                }
                else
                {
                    iaclMask.macSA[i]= macSAMask[i];
                }
            }
            iaclKey.isSmacSet = 1;
        }
        else if (strcmp(key, "etherType") == 0)
        {
            sscanf(data, "%x", &etherType);
            if (etherType > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter etherType data in valid format\n");
                return;
            }
            iaclKey.etherType = etherType;
            if (mask[0] != '1')
            {
                sscanf(mask, "%x", &iaclMask.etherType);
            }
        }
        else if (strcmp(key, "cTagVid") == 0)
        {
            sscanf(data, "%d", &cTagVid);
            if (cTagVid < 0 || cTagVid > 4095)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter cTagVid data in valid format\n");
                return;
            }
            iaclKey.cTagVid = cTagVid;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.cTagVid);
            }
        }
        else if (strcmp(key, "cTagDei") == 0)
        {
            sscanf(data, "%d", &cTagDei);
            if (cTagDei < 0 || cTagDei > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter cTagDei data in valid format\n");
                return;
            }
            iaclKey.cTagDei = cTagDei;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.cTagDei);
            }
        }
        else if (strcmp(key, "cTagPcp") == 0)
        {
            sscanf(data, "%d", &cTagPcp);
            if (cTagPcp < 0 || cTagPcp > 7)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter cTagPcp data in valid format\n");
                return;
            }
            iaclKey.cTagPcp = cTagPcp;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.cTagPcp);
            }
        }
        else if (strcmp(key, "sTagVid") == 0)
        {
            sscanf(data, "%d", &sTagVid);
            if (sTagVid < 0 || sTagVid > 4095)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter sTagVid data in valid format\n");
                return;
            }
            iaclKey.sTagVid = sTagVid;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.sTagVid);
            }
        }
        else if (strcmp(key, "sTagDei") == 0)
        {
            sscanf(data, "%d", &sTagDei);
            if (sTagDei < 0 || sTagDei > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter sTagDei data in valid format\n");
                return;
            }
            iaclKey.sTagDei = sTagDei;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.sTagDei);
            }
        }
        else if (strcmp(key, "sTagPcp") == 0)
        {
            sscanf(data, "%d", &sTagPcp);
            if (sTagPcp < 0 || sTagPcp > 7)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter sTagPcp data in valid format\n");
                return;
            }
            iaclKey.sTagPcp = sTagPcp;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.sTagPcp);
            }
        }
        else if (strcmp(key, "DIP") == 0)
        {
            sscanf(data, "%d.%d.%d.%d", &dip[3], &dip[2], &dip[1], &dip[0]);
            if (strlen(mask) != 1)
            {
                sscanf(mask, "%d.%d.%d.%d", &dipMask[3], &dipMask[2], &dipMask[1], &dipMask[0]);
            }
            int i;
            for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
            {
                if (dip[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter DIP data in valid format\n");
                    return;
                }
                iaclKey.DIP[i] = dip[i];
                if (strlen(mask) == 1)
                {
                    if (mask[0] == '0')
                    {
                        iaclMask.DIP[i]= 0x0;
                    }
                }
                else
                {
                    iaclMask.DIP[i]= dipMask[i];
                }
            }
            iaclKey.isDIPSet = 1;
        }
        else if (strcmp(key, "SIP") == 0)
        {
            sscanf(data, "%d.%d.%d.%d", &sip[3], &sip[2], &sip[1], &sip[0]);
            if (strlen(mask) != 1)
            {
                sscanf(mask, "%d.%d.%d.%d", &sipMask[3], &sipMask[2], &sipMask[1], &sipMask[0]);
            }
            int i;
            for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
            {
                if (sip[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter SIP data in valid format\n");
                    return;
                }
                iaclKey.SIP[i] = sip[i];
                if (strlen(mask) == 1)
                {
                    if (mask[0] == '0')
                    {
                        iaclMask.SIP[i]= 0x0;
                    }
                }
                else
                {
                    iaclMask.SIP[i]= sipMask[i];
                }
            }
            iaclKey.isSIPSet = 1;
        }
        else if (strcmp(key, "L4DestPort") == 0)
        {
            sscanf(data, "%d", &l4DestPort);
            if (l4DestPort < 0 || l4DestPort > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter L4DestPort data in valid format\n");
                return;
            }
            iaclKey.L4DestPort = l4DestPort;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.L4DestPort);
            }
        }
        else if (strcmp(key, "L4SourcePort") == 0)
        {
            sscanf(data, "%d", &l4SourcePort);
            if (l4SourcePort < 0 || l4SourcePort > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter L4SourcePort data in valid format\n");
                return;
            }
            iaclKey.L4SourcePort = l4SourcePort;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.L4SourcePort);
            }
        }
        else if (strcmp(key, "protocol") == 0)
        {
            sscanf(data, "%x", &protocol);
            if (protocol > 255)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter protocol data in valid format\n");
                return;
            }
            iaclKey.protocol = protocol;
            if (mask[0] != '1')
            {
                sscanf(mask, "%x", &iaclMask.protocol);
            }
        }
        else if (strcmp(key, "ipv4DfSet") == 0)
        {
            sscanf(data, "%d", &ipv4DfSet);
            if (ipv4DfSet < 0 || ipv4DfSet > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter ipv4DfSet data in valid format\n");
                return;
            }
            iaclKey.ipv4DfSet = ipv4DfSet;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.ipv4DfSet);
            }
        }
        else if (strcmp(key, "ipv4MfSet") == 0)
        {
            sscanf(data, "%d", &ipv4MfSet);
            if (ipv4MfSet < 0 || ipv4MfSet > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter ipv4MfSet data in valid format\n");
                return;
            }
            iaclKey.ipv4MfSet = ipv4MfSet;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.ipv4MfSet);
            }
        }
        else if (strcmp(key, "pktHasCtag") == 0)
        {
            sscanf(data, "%d", &pktHasCtag);
            if (pktHasCtag < 0 || pktHasCtag > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter pktHasCtag data in valid format\n");
                return;
            }
            iaclKey.pktHasCtag = pktHasCtag;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.pktHasCtag);
            }
        }
        else if (strcmp(key, "BD") == 0)
        {
            sscanf(data, "%d", &BD);
            if (BD< 0 || BD > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter BD data in valid format\n");
                return;
            }
            iaclKey.BD = BD;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.BD);
            }
        }
        else if (strcmp(key, "icmpMessageType") == 0)
        {
            sscanf(data, "%d", &icmpMessageType);
            if (icmpMessageType < 0 || icmpMessageType > 255)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter icmpMessageType data in valid format\n");
                return;
            }
            iaclKey.icmpMessageType = icmpMessageType;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.icmpMessageType);
            }
        }
        else if (strcmp(key, "iVif") == 0)
        {
            sscanf(data, "%d", &iVif);
            if (iVif < 0 || iVif > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter iVif data in valid format\n");
                return;
            }
            iaclKey.iVif = iVif;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.iVif);
            }
        }
        else if (strcmp(key, "dscp") == 0)
        {
            sscanf(data, "%d", &dscp);
            if (dscp < 0 || dscp > 63)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter dscp data in valid format\n");
                return;
            }
            iaclKey.dscp = dscp;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.dscp);
            }
        }
        else if (strcmp(key, "pktHasStag") == 0)
        {
            sscanf(data, "%d", &pktHasStag);
            if (pktHasStag < 0 || pktHasStag > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter pktHasStag data in valid format\n");
                return;
            }
            iaclKey.pktHasStag = pktHasStag;
            if (mask[0] != '1')
            {
                sscanf(mask, "%u", &iaclMask.pktHasStag);
            }
        }
        else
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("enter keyAttrubute data (Unknown Attrubute) in valid format\n");
            //return;
        }
        //str1 = str2;
        //str2 = strstr(str1, ",");
        str2 = strtok(NULL, ",");
        memset(key, 0x00, sizeof(key));
        memset(data, 0x00, sizeof(key));
        memset(mask, 0x00, sizeof(mask));
    }

    addEntry(&configFile.iaclKey, &iaclKey, sizeof(IaclL2Ipv4KeyAttribute));
    addEntry(&configFile.iaclMask, &iaclMask, sizeof(IaclL2Ipv4MaskAttribute));
}


/**
 * \fn processL2Ipv4WcmKey
 * \private
 * \brief Parses line of text from configuration file to determine xpConfigFile::wcmKey
 *
 * \param int lineNum
 * \param char* line
 */
void processL2Ipv4WcmKey(int lineNum, char *line)
{
    //int aclId, type, eVifId, isTerminal, enPktCmdUpd, enRedirectToEvif, remarkDp, remarkTc, enPolicer, qosOrVlanRw, enMirrorSsnUpd, remarkDscp, remarkPcp, TC,mirrorSessionId, policerId, PCP, DSCP, DP, tableId;
    unsigned int aclId, tableId, priority;
    int  cTagVid, cTagDei, cTagPcp, sTagVid, sTagDei, sTagPcp, dip[4], sip[4],
         l4DestPort, l4SourcePort, icmpMessageType, BD, dscp, iVif, ipv4DfSet, ipv4MfSet,
         pktHasCtag, pktHasStag;
    unsigned int macDA[6], macSA[6], etherType, protocol;
    char  keyAttribute[300];
    char *str1 = NULL, *str2 = NULL, *str3 = NULL, *str4 = NULL;
    char key[50], data[50], mask[2];
    IaclL2Ipv4KeyAttribute iaclKey = {-1, {0,}, {0,}, -1, -1, -1, -1, -1, -1, -1, {0,}, {0,}, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0};
    IaclL2Ipv4MaskAttribute iaclMask;

    memset(keyAttribute, 0, sizeof(keyAttribute));
    memset(key, 0, sizeof(key));
    memset(data, 0, sizeof(data));
    memset(mask, 0, sizeof(mask));
    memset(&iaclMask, 0x00, sizeof(IaclL2Ipv4MaskAttribute));

    sscanf(line, "%u %u %u %299s",
           &aclId, &tableId, &priority, keyAttribute);

    if (aclId <= 0)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter type data in valid format\n");
        return;
    }

    iaclKey.iAclId = aclId;
    iaclKey.priority = priority;
    iaclKey.tableId = tableId;
    iaclMask.iAclId = 0;

    str1 = keyAttribute;
    if (strcmp(str1, "null") != 0)
    {
        str2 = strtok(str1, ",");
    }

    while (str2 != NULL)
    {
        str3 = strstr(str2, "->");
        if (str3)
        {
            strncpy(key, str2, str3-str2);
            str4 = strstr(str2, "|");
            if (str4)
            {
                strncpy(data, str3 +2, (str4-str3 - 2));
                strncpy(mask, str4+1, 1);
            }
            else
            {
                printf("ERROR: Invalid input for keyAttribute.\n");
                return;
            }
        }
        else
        {
            printf("ERROR: Invalid input.\n");
            return;
        }

        if (strcmp(key, "macDA") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x", &macDA[5], &macDA[4], &macDA[3], &macDA[2],
                   &macDA[1], &macDA[0]);
            int i;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if (macDA[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter macDA data in valid format\n");
                    return;
                }
                iaclKey.macDA[i]= macDA[i];
                if (mask[0] == '0')
                {
                    iaclMask.macDA[i]= 0;
                }
            }

            iaclKey.isDmacSet = 1;
        }
        else if (strcmp(key, "macSA") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x", &macSA[5], &macSA[4], &macSA[3], &macSA[2],
                   &macSA[1], &macSA[0]);
            int i;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if (macSA[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter macSA data in valid format\n");
                    return;
                }
                iaclKey.macSA[i]= macSA[i];
                if (mask[0] == '0')
                {
                    iaclMask.macSA[i]= 0;
                }
            }
            iaclKey.isSmacSet = 1;
        }
        else if (strcmp(key, "etherType") == 0)
        {
            sscanf(data, "%x", &etherType);
            if (etherType > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter etherType data in valid format\n");
                return;
            }
            iaclKey.etherType = etherType;
            if (mask[0] == '0')
            {
                iaclMask.etherType = 0;
            }
        }
        else if (strcmp(key, "cTagVid") == 0)
        {
            sscanf(data, "%d", &cTagVid);
            if (cTagVid < 0 || cTagVid > 4095)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter cTagVid data in valid format\n");
                return;
            }
            iaclKey.cTagVid = cTagVid;
            if (mask[0] == '0')
            {
                iaclMask.cTagVid = 0;
            }
        }
        else if (strcmp(key, "cTagDei") == 0)
        {
            sscanf(data, "%d", &cTagDei);
            if (cTagDei < 0 || cTagDei > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter cTagDei data in valid format\n");
                return;
            }
            iaclKey.cTagDei = cTagDei;
            if (mask[0] == '0')
            {
                iaclMask.cTagDei = 0;
            }
        }
        else if (strcmp(key, "cTagPcp") == 0)
        {
            sscanf(data, "%d", &cTagPcp);
            if (cTagPcp < 0 || cTagPcp > 7)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter cTagPcp data in valid format\n");
                return;
            }
            iaclKey.cTagPcp = cTagPcp;
            if (mask[0] == '0')
            {
                iaclMask.cTagPcp = 0;
            }
        }
        else if (strcmp(key, "sTagVid") == 0)
        {
            sscanf(data, "%d", &sTagVid);
            if (sTagVid < 0 || sTagVid > 4095)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter sTagVid data in valid format\n");
                return;
            }
            iaclKey.sTagVid = sTagVid;
            if (mask[0] == '0')
            {
                iaclMask.sTagVid = 0;
            }
        }
        else if (strcmp(key, "sTagDei") == 0)
        {
            sscanf(data, "%d", &sTagDei);
            if (sTagDei < 0 || sTagDei > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter sTagDei data in valid format\n");
                return;
            }
            iaclKey.sTagDei = sTagDei;
            if (mask[0] == '0')
            {
                iaclMask.sTagDei = 0;
            }
        }
        else if (strcmp(key, "sTagPcp") == 0)
        {
            sscanf(data, "%d", &sTagPcp);
            if (sTagPcp < 0 || sTagPcp > 7)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter sTagPcp data in valid format\n");
                return;
            }
            iaclKey.sTagPcp = sTagPcp;
            if (mask[0] == '0')
            {
                iaclMask.sTagPcp = 0;
            }
        }
        else if (strcmp(key, "DIP") == 0)
        {
            sscanf(data, "%d.%d.%d.%d", &dip[3], &dip[2], &dip[1], &dip[0]);
            int i;
            for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
            {
                if (dip[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter DIP data in valid format\n");
                    return;
                }
                iaclKey.DIP[i] = dip[i];
                if (mask[0] == '1')
                {
                    iaclMask.DIP[0] = 0xFF;
                }
            }
            iaclKey.isDIPSet = 1;
        }
        else if (strcmp(key, "SIP") == 0)
        {
            sscanf(data, "%d.%d.%d.%d", &sip[3], &sip[2], &sip[1], &sip[0]);
            int i;
            for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
            {
                if (sip[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter SIP data in valid format\n");
                    return;
                }
                iaclKey.SIP[i] = sip[i];
                if (mask[0] == '1')
                {
                    iaclMask.SIP[0] = 0xFF;
                }
            }
            iaclKey.isSIPSet = 1;
        }
        else if (strcmp(key, "L4DestPort") == 0)
        {
            sscanf(data, "%d", &l4DestPort);
            if (l4DestPort < 0 || l4DestPort > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter L4DestPort data in valid format\n");
                return;
            }
            iaclKey.L4DestPort = l4DestPort;
            if (mask[0] == '1')
            {
                iaclMask.L4DestPort = 1;
            }
        }
        else if (strcmp(key, "L4SourcePort") == 0)
        {
            sscanf(data, "%d", &l4SourcePort);
            if (l4SourcePort < 0 || l4SourcePort > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter L4SourcePort data in valid format\n");
                return;
            }
            iaclKey.L4SourcePort = l4SourcePort;
            if (mask[0] == '1')
            {
                iaclMask.L4SourcePort = 1;
            }
        }
        else if (strcmp(key, "protocol") == 0)
        {
            sscanf(data, "%x", &protocol);
            if (protocol > 255)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter protocol data in valid format\n");
                return;
            }
            iaclKey.protocol = protocol;
            if (mask[0] == '0')
            {
                iaclMask.protocol = 0;
            }
        }
        else if (strcmp(key, "ipv4DfSet") == 0)
        {
            sscanf(data, "%d", &ipv4DfSet);
            if (ipv4DfSet < 0 || ipv4DfSet > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter ipv4DfSet data in valid format\n");
                return;
            }
            iaclKey.ipv4DfSet = ipv4DfSet;
            if (mask[0] == '0')
            {
                iaclMask.ipv4DfSet = 0;
            }
        }
        else if (strcmp(key, "ipv4MfSet") == 0)
        {
            sscanf(data, "%d", &ipv4MfSet);
            if (ipv4MfSet < 0 || ipv4MfSet > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter ipv4MfSet data in valid format\n");
                return;
            }
            iaclKey.ipv4MfSet = ipv4MfSet;
            if (mask[0] == '0')
            {
                iaclMask.ipv4MfSet = 0;
            }
        }
        else if (strcmp(key, "pktHasCtag") == 0)
        {
            sscanf(data, "%d", &pktHasCtag);
            if (pktHasCtag < 0 || pktHasCtag > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter pktHasCtag data in valid format\n");
                return;
            }
            iaclKey.pktHasCtag = pktHasCtag;
            if (mask[0] == '0')
            {
                iaclMask.pktHasCtag = 0;
            }
        }
        else if (strcmp(key, "BD") == 0)
        {
            sscanf(data, "%d", &BD);
            if (BD< 0 || BD > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter BD data in valid format\n");
                return;
            }
            iaclKey.BD = BD;
            if (mask[0] == '0')
            {
                iaclMask.BD = 0;
            }
        }
        else if (strcmp(key, "icmpMessageType") == 0)
        {
            sscanf(data, "%d", &icmpMessageType);
            if (icmpMessageType < 0 || icmpMessageType > 255)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter icmpMessageType data in valid format\n");
                return;
            }
            iaclKey.icmpMessageType = icmpMessageType;
            if (mask[0] == '0')
            {
                iaclMask.icmpMessageType = 0;
            }
        }
        else if (strcmp(key, "iVif") == 0)
        {
            sscanf(data, "%d", &iVif);
            if (iVif < 0 || iVif > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter iVif data in valid format\n");
                return;
            }
            iaclKey.iVif = iVif;
            if (mask[0] == '0')
            {
                iaclMask.iVif = 0;
            }
        }

        else if (strcmp(key, "dscp") == 0)
        {
            sscanf(data, "%d", &dscp);
            if (dscp < 0 || dscp > 63)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter dscp data in valid format\n");
                return;
            }
            iaclKey.dscp = dscp;
            if (mask[0] == '0')
            {
                iaclMask.dscp = 0;
            }
        }

        else if (strcmp(key, "pktHasStag") == 0)
        {
            sscanf(data, "%d", &pktHasStag);
            if (pktHasStag < 0 || pktHasStag > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter pktHasStag data in valid format\n");
                return;
            }
            iaclKey.pktHasStag = pktHasStag;
            if (mask[0] == '0')
            {
                iaclMask.pktHasStag = 0;
            }
        }
        else
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("enter keyAttrubute data (Unknown Attrubute) in valid format\n");
            //return;
        }
        //str1 = str2;
        //str2 = strstr(str1, ",");
        str2 = strtok(NULL, ",");
        memset(key, 0x00, sizeof(key));
        memset(data, 0x00, sizeof(key));
        memset(mask, 0x00, sizeof(mask));
    }

    addEntry(&configFile.wcmKey, &iaclKey, sizeof(IaclL2Ipv4KeyAttribute));
    addEntry(&configFile.wcmMask, &iaclMask, sizeof(IaclL2Ipv4MaskAttribute));
}

/**
 * \fn processVlanTenantIdData
 * \private
 * \brief Parses line of text from configuration file to determine Vlan and tenantId mapping.
 *
 * \param int lineNum
 * \param char* line
 */
void processVlanTenantIdData(int lineNum, char *line)
{
    int vlanId, tenantId, tunnelType;
    VlanTenantIdEntry newData;
    int errorCode = 0;
    errorCode= sscanf(line, "%d %d %d", &vlanId, &tunnelType, &tenantId);
    if (errorCode != 3)
    {
        printf(", ERROR: Unable to parse line #  %d in configuration file  %s", lineNum,
               configFile.filename);
        printf("%s", line);
        printf("Skipping to next line...");
        return;
    }

    if (vlanId < 0 || vlanId > 4095)
    {
        printf("vlanId should be between 0 to 4095 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming keyVlanId is 0...");
        return;
    }

    newData.vlanId = vlanId;
    newData.tenantId = tenantId;
    newData.tunnelType = tunnelType;
    addEntry(&configFile.vlanTenantIdData, &newData, sizeof(VlanTenantIdEntry));
}

/**
 * \fn processQosSchedulerData
 * \private
 * \brief Parses line of text from configuration file to determine the QoS scheduler configurations.
 *
 * \param int lineNum
 * \param char* line
 */
void processQosSchedulerData(int lineNum, char *line)
{
    qosSchedulerDataEntry newData;
    int32_t startPortId = 0;
    int32_t endPortId = 0;
    int32_t errorCode = 0;
    int32_t priority = 0;
    int32_t weight = 0;
    uint8_t idx = 0;
    char schType[8];
    char queue[48];
    char *token;

    memset(queue, '\0', 48);
    memset(schType, '\0', 8);
    for (idx = 0; idx < XP_MAX_QUEUES_PER_PORT; idx++)
    {
        newData.queueNum[idx] = -1;
    }

    if (strchr(line, '-') != NULL)
    {
        errorCode = sscanf(line, "%d-%d %47s %d %7s %d", &startPortId, &endPortId,
                           queue, &priority, schType, &weight);
        if (errorCode != 6)
        {
            printf("%s:ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
                   __FUNCTION__, lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            return;
        }
    }
    else
    {
        errorCode = sscanf(line, "%d %47s %d %7s %d", &startPortId, queue, &priority,
                           schType, &weight);
        if (errorCode != 5)
        {
            printf("%s:ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
                   __FUNCTION__, lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            return;
        }
    }
    newData.priority = priority;
    if (strcmp(schType, "SP") == 0)
    {
        newData.enableSP = 1;
    }
    else if (strcmp(schType, "DWRR") == 0)
    {
        newData.enableSP = 0;
    }
    else
    {
        printf("ERROR: Invalid Scheduler type %s |line# %d\" %s \".\n", schType,
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }
    newData.weight = weight;

    idx = 0;
    token = strtok(queue, ",");
    while (token)
    {
        newData.queueNum[idx++] = atoi(token);
        token = strtok(NULL, ",");
    }

    if (startPortId >= SYSTEM_MAX_PORT || endPortId >= SYSTEM_MAX_PORT)
    {
        printf("ERROR: Invalid Input.\n");
        return;
    }
    uint8_t devPortId = startPortId;
    do
    {
        newData.devPortId = devPortId;
        addEntry(&configFile.qosSchedulerData, &newData, sizeof(qosSchedulerDataEntry));
        devPortId++;
    }
    while (devPortId <= endPortId);
}

/**
 * \fn processQosShaperPortData
 * \private
 * \brief Parses line of text from configuration file to determine the QoS shaper port configurations.
 *
 * \param int lineNum
 * \param char* line
 */
void processQosShaperPortData(int lineNum, char *line)
{
    int32_t   errorCode;
    uint32_t  devPort;
    uint32_t  portShaperEn;
    uint32_t  featureEn;
    uint32_t  maxBurstSize;
    long int  rateKbps;
    qosShaperPortDataEntry newData;


    errorCode = sscanf(line, "%u %ld %u %u %u", &devPort, &rateKbps, &maxBurstSize,
                       &portShaperEn, &featureEn);
    if (errorCode != 5)
    {
        printf("%s:ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               __FUNCTION__, lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    newData.devPort      = devPort;
    newData.rateKbps     = rateKbps;
    newData.maxBurstSize = maxBurstSize;
    newData.portShaperEn = portShaperEn;
    newData.featureEn    = featureEn;
    addEntry(&configFile.qosShaperPortData, &newData,
             sizeof(qosShaperPortDataEntry));
}

/**
 * \fn processQosShaperQueueData
 * \private
 * \brief Parses line of text from configuration file to determine the QoS shaper queue configurations.
 *
 * \param int lineNum
 * \param char* line
 */
void processQosShaperQueueData(int lineNum, char *line)
{
    int32_t   errorCode;
    uint32_t  devPort;
    uint32_t  queueNum;
    uint32_t  qSlowShaperEn;
    uint32_t  featureEn;
    uint32_t  maxBurstSize;
    uint64_t  rateKbps;
    qosShaperQueueDataEntry newData;

    errorCode = sscanf(line, "%u %u %ld %u %u %u", &devPort, &queueNum,
                       (long int *)&rateKbps, &maxBurstSize, &qSlowShaperEn, &featureEn);
    if (errorCode != 6)
    {
        printf("%s:ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               __FUNCTION__, lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    newData.devPort       = devPort;
    newData.queueNum      = queueNum;
    newData.rateKbps      = rateKbps;
    newData.maxBurstSize  = maxBurstSize;
    newData.qSlowShaperEn = qSlowShaperEn;
    newData.featureEn     = featureEn;
    addEntry(&configFile.qosShaperQueueData, &newData,
             sizeof(qosShaperQueueDataEntry));
}

/**
 * \fn processQosingressMapData
 * \private
 * \brief Parses line of text from configuration file to
 *        determine the Ingress Qos Map configurations.
 *
 * \param int lineNum
 * \param char* line
 */
void processQosIngressMapData(int lineNum, char *line)
{
    qosIngressMapDataEntry newData;
    int32_t startPortId = 0;
    int32_t endPortId = 0;
    int32_t defaultTC = 0;
    int32_t errorCode;
    uint32_t tc, dp, l2QosEn, l3QosEn, pcp, dei, dscp;
    uint8_t idx;

    for (idx = 0; idx < SYSTEM_MAX_PORT; idx++)
    {
        newData.portList[idx] = -1;
    }

    if (strchr(line, '-') != NULL)
    {
        errorCode = sscanf(line, "%u %u %u %u %u %u %u %d %d-%d", &pcp, &dei, &dscp,
                           &tc, &dp, &l2QosEn, &l3QosEn, &defaultTC, &startPortId, &endPortId);
        if (errorCode != 10)
        {
            printf("%s:ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
                   __FUNCTION__, lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            return;
        }
    }
    else
    {
        errorCode = sscanf(line, "%u %u %u %u %u %u %u %d %d", &pcp, &dei, &dscp, &tc,
                           &dp, &l2QosEn, &l3QosEn, &defaultTC, &startPortId);
        if (errorCode != 9)
        {
            printf("%s:ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
                   __FUNCTION__, lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            return;
        }
    }

    idx = 0;
    uint8_t devPortId = startPortId;
    do
    {
        newData.portList[idx++] = devPortId;
        devPortId++;
    }
    while (devPortId <= endPortId);

    newData.pcp       = pcp;
    newData.dei       = dei;
    newData.dscp      = dscp;
    newData.tc        = tc;
    newData.dp        = dp;
    newData.l2QosEn   = l2QosEn;
    newData.l3QosEn   = l3QosEn;
    newData.defaultTC = defaultTC;

    addEntry(&configFile.qosIngressMapData, &newData,
             sizeof(qosIngressMapDataEntry));
}

/**
 * \fn processQosEgressMapData
 * \private
 * \brief Parses line of text from configuration file to
 *        determine the Egress Qos Map configurations.
 *
 * \param int lineNum
 * \param char* line
 */
void processQosEgressMapData(int lineNum, char *line)
{
    int32_t   errorCode;
    uint32_t egressPort, tc, dp, l2QosEn, l3QosEn, pcp, dei, dscp, type, idx,
             geneveTnlType, mcastType;
    qosEgressMapDataEntry newData;

    errorCode = sscanf(line, "%u %u %u %u %u %u %u %u %u %u %u %u", &egressPort,
                       &tc, &dp, &l2QosEn, &l3QosEn, &pcp, &dei, &dscp, &type, &idx, &geneveTnlType,
                       &mcastType);
    if (errorCode != 12)
    {
        printf("%s:ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               __FUNCTION__, lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    newData.egressPort = egressPort;
    newData.tc        = tc;
    newData.dp        = dp;
    newData.l2QosEn   = l2QosEn;
    newData.l3QosEn   = l3QosEn;
    newData.pcp       = pcp;
    newData.dei       = dei;
    newData.dscp      = dscp;
    newData.type       = type;
    newData.idx       = idx;
    newData.geneveTnlType = geneveTnlType;
    newData.mcastType       = mcastType;
    newData.egressVif = 0;
    addEntry(&configFile.qosEgressMapData, &newData, sizeof(qosEgressMapDataEntry));
}

/**
 * \fn processQosAqmQProfileData
 * \private
 * \brief Parses line of text from configuration file to determine the QoS AQM Queue Profile configurations.
 *
 * \param int lineNum
 * \param char* line
 */
void processQosAqmQProfileData(int lineNum, char *line)
{
    char *token;
    uint32_t devPort;
    uint32_t queueNum;
    uint32_t profileId;
    uint32_t wredMode;
    uint32_t wredAvgLengthMinThreshold;
    uint32_t wredAvgLengthMaxThreshold;
    uint32_t wredAvgLengthWeight;
    uint32_t wredMarkDropProb;
    uint32_t lengthMaxThreshold;
    qosAqmQProfileDataEntry newData;

    //Parse profile id and wred mode
    if (NULL == line)
    {
        return;
    }

    sscanf(line, "%u %u", &profileId, &wredMode);

    newData.profileId = profileId;
    switch (wredMode)
    {
        case 0:
            newData.wredMode = XP_ENABLE_WRED_DROP;
            break;
        case 1:
            newData.wredMode = XP_ENABLE_WRED_MARK;
            break;
        case 2:
            newData.wredMode = XP_DISABLE_WRED;
            break;
        default:
            printf("%s:ERROR: Invalid WredMode %d on line# %d in configuration file \" %s \".\n",
                   __FUNCTION__, wredMode, lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
    }

    //Parse Wred Attributes
    token = strstr(line, "wredAttrib");
    token = strtok(token, "|");
    if (token != NULL)
    {
        sscanf(token, "wredAttrib:%u,%u,%u,%u", &wredAvgLengthMinThreshold,
               &wredAvgLengthMaxThreshold, &wredAvgLengthWeight, &wredMarkDropProb);
    }
    else
    {
        return;
    }
    newData.minThreshold = wredAvgLengthMinThreshold;
    newData.maxThreshold = wredAvgLengthMaxThreshold;
    newData.weight = wredAvgLengthWeight;
    newData.maxDropProb = wredMarkDropProb;

    //Parse Tail Drop Attributes
    token = strtok(NULL, " ");
    if (token != NULL)
    {
        sscanf(token, "tailDropAttrib:%u", &lengthMaxThreshold);
    }
    else
    {
        return;
    }
    newData.lengthMaxThreshold = lengthMaxThreshold;

    //Parse port and queue information
    token = strtok(NULL, "|");
    if (token == NULL)
    {
        printf("Invalid Input.\n");
        return;
    }
    while (isspace(*token))
    {
        token++;
    }
    while (token != NULL)
    {
        sscanf(token, "(%u,%u)", &devPort, &queueNum);
        newData.devPort = devPort;
        newData.queueNum = queueNum;
        token = strtok(NULL, "|");
        addEntry(&configFile.qosAqmQProfileData, &newData,
                 sizeof(qosAqmQProfileDataEntry));
    }
}

/**
 * \fn processSaiQosQueueSchedulerData
 * \private
 * \brief Parses line of text from configuration file to determine the QoS scheduler configurations.
 *
 * \param int lineNum
 * \param char* line
 */
void processSaiQosQueueSchedulerData(int lineNum, char *line)
{
    saiSchedulerDataEntry newData;
    int32_t bandwidthRate = 0;
    int32_t burstSize = 0;
    int32_t startPortId = 0;
    int32_t endPortId = 0;
    int32_t errorCode = 0;
    int32_t weight = 0;
    uint8_t idx = 0;
    char schType[8];
    char queue[48];
    char *token;

    memset(queue, '\0', 48);
    memset(schType, '\0', 8);
    for (idx = 0; idx < XP_MAX_QUEUES_PER_PORT; idx++)
    {
        newData.queueNum[idx] = -1;
    }

    if (strchr(line, '-') != NULL)
    {
        errorCode = sscanf(line, "%d-%d %47s %7s %d %d %d", &startPortId, &endPortId,
                           queue, schType, &weight, &bandwidthRate, &burstSize);
        if (errorCode != 7)
        {
            printf("%s:ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
                   __FUNCTION__, lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            return;
        }
    }
    else
    {
        errorCode = sscanf(line, "%d %47s %7s %d %d %d", &startPortId, queue, schType,
                           &weight, &bandwidthRate, &burstSize);
        if (errorCode != 6)
        {
            printf("%s:ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
                   __FUNCTION__, lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            return;
        }
    }

    if (strcmp(schType, "SP") == 0)
    {
        newData.enableSP = 1;
    }
    else if (strcmp(schType, "DWRR") == 0)
    {
        newData.enableSP = 0;
    }
    else
    {
        printf("ERROR: Invalid Scheduler type %s |line# %d\" %s \".\n", schType,
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }
    newData.weight = weight;
    newData.bandwidthRate = bandwidthRate;
    newData.burstSize = burstSize;

    idx = 0;
    token = strtok(queue, ",");
    while (token)
    {
        newData.queueNum[idx++] = atoi(token);
        token = strtok(NULL, ",");
    }

    if (startPortId >= SYSTEM_MAX_PORT || endPortId >= SYSTEM_MAX_PORT)
    {
        printf("ERROR: Invalid Input.\n");
        return;
    }

    idx = 0;
    uint8_t devPortId = startPortId;

    do
    {
        newData.portList[idx++] = devPortId;
        devPortId++;
    }
    while (devPortId <= endPortId);

    addEntry(&configFile.saiSchedulerData, &newData, sizeof(saiSchedulerDataEntry));
}

/**
 * \fn processPolicerData
 * \private
 * \brief Parses line of text from configuration file to
 *        determine the Policer configurations.
 *
 * \param int lineNum
 * \param char* line
 */
void processPolicerData(int lineNum, char *line)
{
    policerDataEntry newData;
    uint32_t startPortId = 0;
    uint32_t endPortId = 0;
    uint32_t cbs = 0, cir = 0;
    uint32_t pbs = 0, pir = 0;
    int32_t errorCode;
    uint8_t idx;
    char yellowPktAction[16];
    char redPktAction[16];
    char color[8];

    memset(color, '\0', 8);
    memset(yellowPktAction, '\0', 16);
    memset(redPktAction, '\0', 16);
    for (idx = 0; idx < SYSTEM_MAX_PORT; idx++)
    {
        newData.portList[idx] = -1;
    }

    if (strchr(line, '-') != NULL)
    {
        errorCode = sscanf(line, "%7s %u %u %u %u %15s %15s %u-%u", color, &cbs, &cir,
                           &pbs, &pir, yellowPktAction, redPktAction, &startPortId, &endPortId);
        if (errorCode != 9)
        {
            printf("%s:ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
                   __FUNCTION__, lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            return;
        }
    }
    else
    {
        errorCode = sscanf(line, "%7s %u %u %u %u %15s %15s %u", color, &cbs, &cir,
                           &pbs, &pir, yellowPktAction, redPktAction, &startPortId);
        if (errorCode != 8)
        {
            printf("%s:ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
                   __FUNCTION__, lineNum, configFile.filename);
            printf("%s\n", line);
            printf("Skipping to next line...\n");
            return;
        }
    }

    idx = 0;
    uint8_t devPortId = startPortId;
    do
    {
        newData.portList[idx++] = devPortId;
        devPortId++;
    }
    while (devPortId <= endPortId);

    newData.cbs = cbs;
    newData.cir = cir;
    newData.pbs = pbs;
    newData.pir = pir;

    /* 0 - BLIND, 1 - AWARE */
    if (strcmp(color, "BLIND") == 0)
    {
        newData.color = 0;
    }
    else if (strcmp(color, "AWARE") == 0)
    {
        newData.color = 1;
    }
    else
    {
        printf("ERROR: Invalid Color %s |line# %d\" %s \".\n", color, lineNum,
               configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    /* 0 - DROP, 1 - FORWARD*/
    if (strcmp(yellowPktAction, "DROP") == 0)
    {
        newData.yellowPktAction = 0;
    }
    else if (strcmp(yellowPktAction, "FORWARD") == 0)
    {
        newData.yellowPktAction = 1;
    }
    else
    {
        printf("ERROR: Invalid yellowPktAction %s |line# %d\" %s \".\n",
               yellowPktAction, lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    /* 0 - DROP, 1 - FORWARD*/
    if (strcmp(redPktAction, "DROP") == 0)
    {
        newData.redPktAction = 0;
    }
    else if (strcmp(redPktAction, "FORWARD") == 0)
    {
        newData.redPktAction = 1;
    }
    else
    {
        printf("ERROR: Invalid redPktAction %s |line# %d\" %s \".\n", redPktAction,
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }
    addEntry(&configFile.policerData, &newData, sizeof(policerDataEntry));
}

/**
 * \fn processFdbControlMacData
 * \private
 * \brief Parses line of text from configuration file to determine FdbControlMacEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processFdbControlMacData(int lineNum, char *line)
{
    int vlan, reasonCode;
    unsigned int macAddr[XP_MAC_ADDR_LEN];
    int i = 0;
    int errorCode;

    FdbControlMacEntry newData;

    errorCode= sscanf(line, "%x : %x : %x : %x : %x : %x %d %d ", &macAddr[5],
                      &macAddr[4], &macAddr[3], &macAddr[2], &macAddr[1], &macAddr[0], &vlan,
                      &reasonCode);

    if (errorCode != XP_MAC_ADDR_LEN + 2)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    if (vlan < 0 || vlan > 4095)
    {
        printf("VlanId ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               vlan, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
    {
        if (macAddr[i] > 255)
        {
            printf("MAC address value ( 0x%x ) out of range error on line # %d in configuration file \" %s \".\n",
                   macAddr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.macAddr[i]= macAddr[i];
        }
    }

    if (reasonCode < 0 || reasonCode > 1023)
    {
        printf("reasonCode( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               reasonCode, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    newData.vlanId= vlan;
    newData.reasonCode = reasonCode;

    addEntry(&configFile.fdbControlMacData, &newData, sizeof(FdbControlMacEntry));
}

/**
 * \fn processIpv4HostControlData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv4HostControlEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpv4HostControlData(int lineNum, char *line)
{
    int vrf, reasonCode;
    int ipv4Addr[XP_IPV4_ADDR_LEN];
    Ipv4HostControlEntry newData;
    int i = 0;
    int errorCode;

    errorCode= sscanf(line, "%d . %d . %d . %d %d %d ", &ipv4Addr[3], &ipv4Addr[2],
                      &ipv4Addr[1], &ipv4Addr[0], &vrf, &reasonCode);

    if (errorCode != XP_IPV4_ADDR_LEN + 2)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    for (i= 0; i < XP_IPV4_ADDR_LEN; ++i)
    {
        if (ipv4Addr[i] < 0 || ipv4Addr[i] > 255)
        {
            printf("IPV4 address value ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv4Addr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.ipv4Addr[i]= ipv4Addr[i];
        }
    }

    if (vrf < 0 || vrf > 255)
    {
        printf("vrf should be between 0 to 255 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming vrf is 0...\n");
        vrf = 0;
    }

    if (reasonCode < 0 || reasonCode > 1023)
    {
        printf("reasonCode( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               reasonCode, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    newData.vrf = vrf;
    newData.reasonCode = reasonCode;

    addEntry(&configFile.ipv4HostControlData, &newData,
             sizeof(Ipv4HostControlEntry));
}

/**
 * \fn processIpv6HostControlData
 * \private
 * \brief Parses line of text from configuration file to determine Ipv6HostControlEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processIpv6HostControlData(int lineNum, char *line)
{
    int vrf, reasonCode;
    unsigned int ipv6Addr[XP_IPV6_ADDR_LEN];
    Ipv6HostControlEntry newData;
    int i = 0;
    int errorCode;

    errorCode= sscanf(line,
                      " %02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X : "
                      "%02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X %d %d",
                      &ipv6Addr[15], &ipv6Addr[14], &ipv6Addr[13], &ipv6Addr[12], &ipv6Addr[11],
                      &ipv6Addr[10], &ipv6Addr[9], &ipv6Addr[8], &ipv6Addr[7], &ipv6Addr[6],
                      &ipv6Addr[5], &ipv6Addr[4], &ipv6Addr[3], &ipv6Addr[2], &ipv6Addr[1],
                      &ipv6Addr[0],  &vrf, &reasonCode);

    if (errorCode != XP_IPV6_ADDR_LEN + 2)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }
    //TODO: support ipv6 validation
    for (i= 0; i < XP_IPV6_ADDR_LEN; ++i)
    {
        if (ipv6Addr[i] > 0xFF)
        {
            printf("IPV6 address value ( 0X%X ) out of range error on line # %d in configuration file \" %s \".\n",
                   ipv6Addr[i], lineNum, configFile.filename);
            printf("Skipping to next line...\n");
            return;
        }
        else
        {
            newData.ipv6Addr[i]= ipv6Addr[i];
        }
    }

    if (vrf < 0 || vrf > 255)
    {
        printf("vrf should be between 0 to 255 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming vrf is 0...\n");
        vrf = 0;
    }

    if (reasonCode < 0 || reasonCode > 1023)
    {
        printf("reasonCode( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               reasonCode, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    newData.vrf = vrf;
    newData.reasonCode = reasonCode;

    addEntry(&configFile.ipv6HostControlData, &newData,
             sizeof(Ipv6HostControlEntry));
}

/**
 * \fn processCoppData
 * \private
 * \brief Parses line of text from configuration file to determine CoppEntry.
 *
 * \param int lineNum
 * \param char* line
 */
void processCoppData(int lineNum, char *line)
{
    int sourcePort, reasonCodeKey, enPolicer, policerId, updatePktCmd, pktCmd;
    int updateReasonCode, reasonCode;
    int errorCode;

    CoppEntry newData;

    errorCode= sscanf(line, "%d %d %d %d %d %d %d %d", &sourcePort, &reasonCodeKey,
                      &enPolicer, &policerId,
                      &updatePktCmd, &pktCmd, &updateReasonCode, &reasonCode);

    if (errorCode != 8)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    if (sourcePort < 0 || sourcePort > 136)
    {
        printf("Port number ( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               sourcePort, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (reasonCodeKey < 0 || reasonCodeKey > 1023)
    {
        printf("reasonCodeKey( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               reasonCode, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    if (enPolicer < 0 || enPolicer > 1)
    {
        printf("enPolicer should be either 0 or 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming enPolicer as 0... \n");
        enPolicer = 0;
    }

    if (policerId < 0 || policerId > 65535)
    {
        printf("policerId should be between 0 to 65535 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming policerId is 0...\n");
        policerId = 0;
    }

    if (updatePktCmd < 0 || updatePktCmd > 1)
    {
        printf("updatePktCmd should be either 0 or 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming updatePktCmd as 0... \n");
        updatePktCmd = 0;
    }

    if (pktCmd < 0 || pktCmd > 3)
    {
        printf("pktCmd should be between 0 to 3 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming pktCmd is 0...\n");
        pktCmd = 0;
    }

    if (updateReasonCode < 0 || updateReasonCode > 1)
    {
        printf("updateReasonCode should be either 0 or 1 on line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("Assuming updateReasonCode as 0... \n");
        updateReasonCode = 0;
    }

    if (reasonCode < 0 || reasonCode > 1023)
    {
        printf("reasonCode( %d ) out of range error on line # %d in configuration file \" %s \".\n",
               reasonCode, lineNum, configFile.filename);
        printf("Skipping to next line...\n");
        return;
    }

    newData.sourcePort = sourcePort;
    newData.reasonCodeKey = reasonCodeKey;
    newData.enPolicer = enPolicer;
    newData.policerId = policerId;
    newData.updatePktCmd = updatePktCmd;
    newData.pktCmd = pktCmd;
    newData.updateReasonCode = updateReasonCode;
    newData.reasonCode = reasonCode;

    addEntry(&configFile.coppData, &newData, sizeof(CoppEntry));
}

/**
 * \fn prepareListFromString
 * \private
 * \brief Parses line of text from configuration file to determine trust br configuration.
 *
 */
void prepareListFromString(char *temp, uint32_t *list, uint16_t *numOfEntry)
{
    int tempInc = 0, bufInc = 0;
    char *str1 = NULL;
    char buf[5];
    uint32_t vif;

    if (numOfEntry == NULL)
    {
        printf("ERROR: NULL Pointer numOfEntry passed into %s\n",  __FUNCTION__);
        return;
    }
    *numOfEntry = 0;

    str1 = strstr(temp, "null");
    if (!str1 && list && numOfEntry)
    {
        *numOfEntry = 0;
        while (temp[tempInc]!='\0')
        {
            if (isdigit(temp[tempInc]))
            {
                buf[bufInc]=temp[tempInc];
                bufInc++;
            }
            else if (temp[tempInc]==',')
            {
                buf[bufInc]='\0';
                vif = atoi(buf);
                if (*numOfEntry < XP_MAX_TOTAL_PORTS)
                {
                    list[(*numOfEntry)++]= vif ;
                }
                bufInc = 0;
            }
            else
            {
                printf(", ERROR: Unable to parse buffer %s in configuration file \" %s \".\n",
                       temp, configFile.filename);
                printf("Skipping to next line...\n");
                return;
            }
            tempInc++;
        }
        buf[bufInc]='\0';
        vif = atoi(buf);
        if (*numOfEntry < XP_MAX_TOTAL_PORTS)
        {
            list[(*numOfEntry)++]= vif ;
        }
    }

}

/**
 * \fn processTrustBrData
 * \private
 * \brief Parses line of text from configuration file to determine trust br configuration.
 *
 */
void processTrustBrData(int lineNum, char *line)
{
    int errorCode;
    static uint32_t tempList[XP_MAX_TOTAL_PORTS]= {0};
    uint16_t numOfEntry;

    static TrustBrEntry newData;
    static char temp[100], temp1[100], temp2[100], temp3[100], temp4[100],
           temp5[100], temp6[100], temp7[100];

    errorCode= sscanf(line,
                      "portList:->%99s | lagList:-> %99s\t\t portList:->%99s | lagList:-> %99s\t\t lagList:-> %99s | evifList:->%99s\t\t %99s \t %99s",
                      temp, temp1, temp2, temp3, temp4, temp5, temp6, temp7);
    if (errorCode != 8)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    prepareListFromString(temp, tempList, &numOfEntry);
    memcpy(newData.addEtagPorts, tempList, sizeof(uint32_t)*numOfEntry);
    newData.numOfAddEtagPorts = numOfEntry;

    prepareListFromString(temp1, tempList, &numOfEntry);
    memcpy(newData.addEtagLags, tempList, sizeof(uint32_t)*numOfEntry);
    newData.numOfAddEtagLags = numOfEntry;

    prepareListFromString(temp2, tempList, &numOfEntry);
    memcpy(newData.keepEtagPorts, tempList, sizeof(uint32_t)*numOfEntry);
    newData.numOfKeepEtagPorts = numOfEntry;

    prepareListFromString(temp3, tempList, &numOfEntry);
    memcpy(newData.keepEtagLags, tempList, sizeof(uint32_t)*numOfEntry);
    newData.numOfKeepEtagLags = numOfEntry;

    prepareListFromString(temp4, tempList, &numOfEntry);
    memcpy(newData.stripEtagLagVif, tempList, sizeof(uint32_t)*numOfEntry);
    newData.numOfStripEtagLagVif = numOfEntry;

    prepareListFromString(temp5, tempList, &numOfEntry);
    memcpy(newData.stripEtagVif, tempList, sizeof(uint32_t)*numOfEntry);
    newData.numOfStripEtagVif = numOfEntry;

    prepareListFromString(temp6, tempList, &numOfEntry);
    memcpy(newData.longEtagModePort, tempList, sizeof(uint32_t)*numOfEntry);
    newData.numOflongEtagModePort = numOfEntry;

    int i=0;
    int cint = (int)(':');
    char *str1, *str2;
    char* token = NULL;
    char* subToken = NULL;

    memset(newData.byPassPipelinePort, 0, sizeof(newData.byPassPipelinePort));
    if (strchr(temp7, cint))
    {
        token = strtok_r(temp7, ",", &str1);

        while (token)
        {
            subToken = strtok_r(token, ":", &str2);
            if (!subToken)
            {
                printf("ERROR: Invalid Input\n");
                return;
            }
            newData.byPassPipelinePort[i][0] = atoi(subToken);
            subToken = strtok_r(NULL, ":", &str2);
            if (!subToken)
            {
                printf("ERROR: Invalid Input\n");
                return;
            }

            if (strstr(subToken, "null"))
            {
                newData.byPassPipelinePort[i][1] = -1;
            }
            else
            {
                newData.byPassPipelinePort[i][1] = atoi(subToken);
            }

            //printf("DBG: newData.byPassPipelinePort[%d][0] = %d, newData.byPassPipelinePort[%d][1] = %d\n", i, newData.byPassPipelinePort[i][0], i, newData.byPassPipelinePort[i][1]);
            i++;
            token = strtok_r(NULL, ",", &str1);
        }
    }
    newData.numOfByPassPipelinePort = i;

    addEntry(&configFile.trustBrData, &newData, sizeof(TrustBrEntry));
}

/**
 * \fn processVifData
 * \private
 * \brief Parses line of text from configuration file to determine trust br configuration.
 *
 */
void processVifData(int lineNum, char *line)
{
    int errorCode;
    uint32_t tempList[XP_MAX_TOTAL_PORTS]= {0};
    uint16_t numOfEntry;

    VifEntry newData;
    char lagList[100], portList[100];
    uint32_t evif;

    errorCode= sscanf(line, "%u \t %99s \t %99s", &evif, portList, lagList);
    if (errorCode != 3)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \".\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("Skipping to next line...\n");
        return;
    }

    newData.evif = evif;

    prepareListFromString(portList, tempList, &numOfEntry);
    memcpy(newData.egressPorts, tempList, sizeof(uint32_t)*numOfEntry);
    newData.numOfEgressPorts = numOfEntry;

    prepareListFromString(lagList, tempList, &numOfEntry);
    memcpy(newData.egressLags, tempList, sizeof(uint32_t)*numOfEntry);
    newData.numOfEgressLags = numOfEntry;

    addEntry(&configFile.vifData, &newData, sizeof(VifEntry));
}


/**
 * \fn procesPBRData
 * \private
 * \brief Parses line of text from configuration file
 *
 * \param int lineNum
 * \param char* line
 */
void processPBRData(int lineNum, char *line)
{
    int ecmpEn, nhId, aclId;
    int  cTagVid, cTagDei, cTagPcp, sTagVid, sTagDei, sTagPcp, dip[4], sip[4],
         l4DestPort, l4SourcePort, icmpMessageType, BD, dscp, iVif, ipv4DfSet, ipv4MfSet,
         pktHasCtag, pktHasStag, vlan, vrfId;
    unsigned int macDA[6], macSA[6];
    int protocol = 0, pktLen = 0, etherType = 0;
    char  keyAttribute[300];
    char *str1, *str2, *str3, *str4;
    char key[50], data[50], mask[2];
    IaclL2Ipv4KeyAttribute pbrKey = {-1, {0,}, {0,}, -1, -1, -1, -1, -1, -1, -1, {0,}, {0,}, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0};
    PbrData pbrData;
    IaclL2Ipv4MaskAttribute pbrMask;

    memset(keyAttribute, 0, sizeof(keyAttribute));
    memset(key, 0, sizeof(key));
    memset(data, 0, sizeof(data));
    memset(mask, 0, sizeof(mask));
    memset(&pbrMask, 0xFF, sizeof(IaclL2Ipv4MaskAttribute));
    memset(&pbrData, 0, sizeof(PbrData));

    sscanf(line, "%d %d %d %d %299s", &aclId, &vlan, &ecmpEn, &nhId, keyAttribute);

    if (aclId < 0 || aclId > 65535)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter aclId data in valid format\n");
        return;
    }
    pbrKey.iAclId = aclId;
    pbrMask.iAclId = 0;

    if (vlan < 0 || vlan > 4095)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter vlan data in valid format\n");
        return;
    }
    pbrData.vlan = vlan;


    if (ecmpEn< 0 || ecmpEn> 1)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter ecmpEn data in valid format\n");
        return;
    }
    pbrData.ecmpEn = ecmpEn;

    if (nhId< 0 || nhId > 1023)
    {
        printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
               lineNum, configFile.filename);
        printf("%s\n", line);
        printf("enter nhId data in valid format\n");
        return;
    }
    pbrData.nhId = nhId;

    str1 = keyAttribute;
    str2 = strtok(str1, ",");

    while (str2 != NULL)
    {
        str3 = strstr(str2, "->");
        strncpy(key, str2, str3-str2);
        str4 = strstr(str2, "|");
        strncpy(data, str3 +2, (str4-str3 - 2));
        strncpy(mask, str4+1, 1);
        if (strcmp(key, "macDA") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x", &macDA[5], &macDA[4], &macDA[3], &macDA[2],
                   &macDA[1], &macDA[0]);
            int i;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if (macDA[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter macDA data in valid format\n");
                    return;
                }
                pbrKey.macDA[i]= macDA[i];
                if (mask[0] == '0')
                {
                    pbrMask.macDA[i]= 0;
                }
            }

            pbrKey.isDmacSet = 1;
        }
        else if (strcmp(key, "macSA") == 0)
        {
            sscanf(data, "%x:%x:%x:%x:%x:%x", &macSA[5], &macSA[4], &macSA[3], &macSA[2],
                   &macSA[1], &macSA[0]);
            int i;
            for (i= 0; i < XP_MAC_ADDR_LEN; ++i)
            {
                if (macSA[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter macSA data in valid format\n");
                    return;
                }
                pbrKey.macSA[i]= macSA[i];
                if (mask[0] == '0')
                {
                    pbrMask.macSA[i]= 0;
                }
            }
            pbrKey.isSmacSet = 1;
        }
        else if (strcmp(key, "etherType") == 0)
        {
            sscanf(data, "%d", &etherType);
            if (etherType > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter etherType data in valid format\n");
                return;
            }
            pbrKey.etherType = etherType;
            if (mask[0] == '0')
            {
                pbrMask.etherType = 0;
            }
        }
        else if (strcmp(key, "cTagVid") == 0)
        {
            sscanf(data, "%d", &cTagVid);
            if (cTagVid < 0 || cTagVid > 4095)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter cTagVid data in valid format\n");
                return;
            }
            pbrKey.cTagVid = cTagVid;
            if (mask[0] == '0')
            {
                pbrMask.cTagVid = 0;
            }
        }
        else if (strcmp(key, "cTagDei") == 0)
        {
            sscanf(data, "%d", &cTagDei);
            if (cTagDei < 0 || cTagDei > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter cTagDei data in valid format\n");
                return;
            }
            pbrKey.cTagDei = cTagDei;
            if (mask[0] == '0')
            {
                pbrMask.cTagDei = 0;
            }
        }
        else if (strcmp(key, "cTagPcp") == 0)
        {
            sscanf(data, "%d", &cTagPcp);
            if (cTagPcp < 0 || cTagPcp > 7)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter cTagPcp data in valid format\n");
                return;
            }
            pbrKey.cTagPcp = cTagPcp;
            if (mask[0] == '0')
            {
                pbrMask.cTagPcp = 0;
            }
        }
        else if (strcmp(key, "sTagVid") == 0)
        {
            sscanf(data, "%d", &sTagVid);
            if (sTagVid < 0 || sTagVid > 4095)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter sTagVid data in valid format\n");
                return;
            }
            pbrKey.sTagVid = sTagVid;
            if (mask[0] == '0')
            {
                pbrMask.sTagVid = 0;
            }
        }
        else if (strcmp(key, "sTagDei") == 0)
        {
            sscanf(data, "%d", &sTagDei);
            if (sTagDei < 0 || sTagDei > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter sTagDei data in valid format\n");
                return;
            }
            pbrKey.sTagDei = sTagDei;
            if (mask[0] == '0')
            {
                pbrMask.sTagDei = 0;
            }
        }
        else if (strcmp(key, "sTagPcp") == 0)
        {
            sscanf(data, "%d", &sTagPcp);
            if (sTagPcp < 0 || sTagPcp > 7)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter sTagPcp data in valid format\n");
                return;
            }
            pbrKey.sTagPcp = sTagPcp;
            if (mask[0] == '0')
            {
                pbrMask.sTagPcp = 0;
            }
        }
        else if (strcmp(key, "DIP") == 0)
        {
            sscanf(data, "%d.%d.%d.%d", &dip[3], &dip[2], &dip[1], &dip[0]);
            int i;
            for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
            {
                if (dip[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter DIP data in valid format\n");
                    return;
                }
                pbrKey.DIP[i] = dip[i];
                if (mask[0] == '0')
                {
                    pbrMask.DIP[i] = 0x0;
                }
            }
            pbrKey.isDIPSet = 1;
        }
        else if (strcmp(key, "SIP") == 0)
        {
            sscanf(data, "%d.%d.%d.%d", &sip[3], &sip[2], &sip[1], &sip[0]);
            int i;
            for (i = 0; i < XP_IPV4_ADDR_LEN; i++)
            {
                if (sip[i] > 255)
                {
                    printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                           lineNum, configFile.filename);
                    printf("%s\n", line);
                    printf("enter SIP data in valid format\n");
                    return;
                }
                pbrKey.SIP[i] = sip[i];
                if (mask[0] == '0')
                {
                    pbrMask.SIP[i] = 0x00;
                }
            }
            pbrKey.isSIPSet = 1;
        }
        else if (strcmp(key, "L4DestPort") == 0)
        {
            sscanf(data, "%d", &l4DestPort);
            if (l4DestPort < 0 || l4DestPort > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter L4DestPort data in valid format\n");
                return;
            }
            pbrKey.L4DestPort = l4DestPort;
            if (mask[0] == '0')
            {
                pbrMask.L4DestPort = 0;
            }
        }
        else if (strcmp(key, "L4SourcePort") == 0)
        {
            sscanf(data, "%d", &l4SourcePort);
            if (l4SourcePort < 0 || l4SourcePort > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter L4SourcePort data in valid format\n");
                return;
            }
            pbrKey.L4SourcePort = l4SourcePort;
            if (mask[0] == '0')
            {
                pbrMask.L4SourcePort = 0;
            }
        }
        else if (strcmp(key, "protocol") == 0)
        {
            sscanf(data, "%d", &protocol);
            if (protocol < 0 || protocol > 255)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter protocol data in valid format\n");
                return;
            }
            pbrKey.protocol = protocol;
            if (mask[0] == '0')
            {
                pbrMask.protocol = 0;
            }
        }
        else if (strcmp(key, "ipv4DfSet") == 0)
        {
            sscanf(data, "%d", &ipv4DfSet);
            if (ipv4DfSet < 0 || ipv4DfSet > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter ipv4DfSet data in valid format\n");
                return;
            }
            pbrKey.ipv4DfSet = ipv4DfSet;
            if (mask[0] == '0')
            {
                pbrMask.ipv4DfSet = 0;
            }
        }
        else if (strcmp(key, "ipv4MfSet") == 0)
        {
            sscanf(data, "%d", &ipv4MfSet);
            if (ipv4MfSet < 0 || ipv4MfSet > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter ipv4MfSet data in valid format\n");
                return;
            }
            pbrKey.ipv4MfSet = ipv4MfSet;
            if (mask[0] == '0')
            {
                pbrMask.ipv4MfSet = 0;
            }
        }
        else if (strcmp(key, "pktHasCtag") == 0)
        {
            sscanf(data, "%d", &pktHasCtag);
            if (pktHasCtag < 0 || pktHasCtag > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter pktHasCtag data in valid format\n");
                return;
            }
            pbrKey.pktHasCtag = pktHasCtag;
            if (mask[0] == '0')
            {
                pbrMask.pktHasCtag = 0;
            }
        }
        else if (strcmp(key, "BD") == 0)
        {
            sscanf(data, "%d", &BD);
            if (BD< 0 || BD > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter BD data in valid format\n");
                return;
            }
            pbrKey.BD = BD;
            if (mask[0] == '0')
            {
                pbrMask.BD = 0;
            }
        }
        else if (strcmp(key, "icmpMessageType") == 0)
        {
            sscanf(data, "%d", &icmpMessageType);
            if (icmpMessageType < 0 || icmpMessageType > 255)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter icmpMessageType data in valid format\n");
                return;
            }
            pbrKey.icmpMessageType = icmpMessageType;
            if (mask[0] == '0')
            {
                pbrMask.icmpMessageType = 0;
            }
        }
        else if (strcmp(key, "iVif") == 0)
        {
            sscanf(data, "%d", &iVif);
            if (iVif < 0 || iVif > 65535)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter iVif data in valid format\n");
                return;
            }
            pbrKey.iVif = iVif;
            if (mask[0] == '0')
            {
                pbrMask.iVif = 0;
            }
        }

        else if (strcmp(key, "dscp") == 0)
        {
            sscanf(data, "%d", &dscp);
            if (dscp < 0 || dscp > 63)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter dscp data in valid format\n");
                return;
            }
            pbrKey.dscp = dscp;
            if (mask[0] == '0')
            {
                pbrMask.dscp = 0;
            }
        }

        else if (strcmp(key, "pktHasStag") == 0)
        {
            sscanf(data, "%d", &pktHasStag);
            if (pktHasStag < 0 || pktHasStag > 1)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter pktHasStag data in valid format\n");
                return;
            }
            pbrKey.pktHasStag = pktHasStag;
            if (mask[0] == '0')
            {
                pbrMask.pktHasStag = 0;
            }
        }
        else if (strcmp(key, "vrfId") == 0)
        {
            sscanf(data, "%d", &vrfId);
            if (vrfId < 0 || vrfId > 255)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter L4DestPort data in valid format\n");
                return;
            }
            pbrKey.vrfId = vrfId;
            if (mask[0] == '0')
            {
                pbrMask.vrfId = 0;
            }
        }
        else if (strcmp(key, "pktLen") == 0)
        {
            sscanf(data, "%d", &pktLen);
            if (pktLen < 0 || pktLen > 255)
            {
                printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                       lineNum, configFile.filename);
                printf("%s\n", line);
                printf("enter pktLen data in valid format\n");
                return;
            }
            pbrKey.pktLen = pktLen;
            if (mask[0] == '0')
            {
                pbrMask.pktLen = 0;
            }
        }

        else
        {
            printf(", ERROR: Unable to parse line # %d in configuration file \" %s \":\n",
                   lineNum, configFile.filename);
            printf("%s\n", line);
            printf("enter keyAttrubute data (Unknown Attrubute) in valid format\n");
            //return;
        }
        //str1 = str2;
        //str2 = strstr(str1, ",");
        str2 = strtok(NULL, ",");
        memset(key, 0x00, sizeof(key));
        memset(data, 0x00, sizeof(key));
        memset(mask, 0x00, sizeof(mask));
    }

    addEntry(&configFile.pbrKey, &pbrKey, sizeof(IaclL2Ipv4KeyAttribute));
    addEntry(&configFile.pbrMask, &pbrMask, sizeof(IaclL2Ipv4MaskAttribute));
    addEntry(&configFile.pbrData, &pbrData, sizeof(PbrData));
}
/**
 * \fn readConfigFile
 * \public
 * \brief Reads options from configuration file.
 *
 * \param string newFileName
 */
XP_STATUS readConfigFile(char* newFileName)
{
    //LOG(XP_LOG_TRACE, "in xpConfigFile::readConfigFile()");
    //struct stat buffer;
    char line[XP_MAX_CONFIG_LINE_LEN];
    int errorCode = 0;
    int notSpace = 0;
    int part = 1;
    int lineNum = 1;

    if (newFileName != NULL &&
        strcmp(newFileName, "") > 0)            // If new filename passed in then:
    {
        // Set this->filename to new filename.
        configFile.filename = newFileName;
    }
    else
    {
        printf("xpConfigFile::readConfigFile(): No file to read.\n");
        return XP_ERR_NOT_FOUND;
    }

    //if (stat(configFile.filename, &buffer) == -1)
    //{
    //    printf("Unable to find configuration file \" %s \".  Exiting...", configFile.filename);
    //    exit(0);    // If the configuration file does not exist then give up.
    //}

    FILE* config = fopen(configFile.filename, "r");
    if (config == NULL)
    {
        printf("Unable to open configuration file \" %s \".  Exiting...",
               configFile.filename);
        //exit(0);    // If the configuration file cannot be opened then give up.
        return XP_ERR_FILE_OPEN;
    }

    // printf("Begin config.txt parsing\n");
    while (!feof(config))                    // While not at the end of the file:
    {
        if (fgets(line, XP_MAX_CONFIG_LINE_LEN, config) != NULL)
        {
            //printf("[%s]\n", line);
            notSpace= strspn(line, " \t\n\r");
            if (line[notSpace] != '\0' &&
                line[notSpace] != '#')    // If not blank line or comment:
            {
                if (strchr("~!@$%^&*_=\"'?/<", (int) line[notSpace]) != NULL)
                {
                    // Switch to next part of file:
                    ++part;
                }
                else
                {
                    switch (part)
                    {
                        case 1:
                            processVlanData(lineNum, line);
                            break;
                        case 2:
                            processMacData(lineNum, line);
                            break;
                        case 3:
                            processVlanTenantIdData(lineNum, line);
                            break;
                        case 4:
                            processTunnelData(lineNum, line);
                            break;
                        case 5:
                            processTunnelQinQData(lineNum, line);
                            break;
                        case 6:
                            processTunnelPBBData(lineNum, line);
                            break;
                        case 7:
                            processTunnelMplsData(lineNum, line);
                            break;
                        case 8:
                            processPortConfigData(lineNum, line);
                            break;
                        case 9:
                            processPortLagData(lineNum, line);
                            break;
                        case 10:
                            processIpxNhData(lineNum, line);
                            break;
                        case 11:
                            processIpv4HostData(lineNum, line);
                            break;
                        case 12:
                            processIpv4RouteData(lineNum, line);
                            break;
                        case 13:
                            processControlMacData(lineNum, line);
                            break;
                        case 14:
                            processIpv6HostData(lineNum, line);
                            break;
                        case 15:
                            processIpv6RouteData(lineNum, line);
                            break;
                        case 16:
                            processIpv4BridgeMcData(lineNum, line);
                            break;
                        case 17:
                            processIpv6BridgeMcData(lineNum, line);
                            break;
                        case 18:
                            processIpv4RouteMcData(lineNum, line);
                            break;
                        case 19:
                            processIpv6RouteMcData(lineNum, line);
                            break;
                        case 20:
                            processIpv4PimBiDirData(lineNum, line);
                            break;
                        case 21:
                            processL2MdtNodeData(lineNum, line);
                            break;
                        case 22:
                            processL3MdtNodeData(lineNum, line);
                            break;
                        case 23:
                            processMvifData(lineNum, line);
                            break;
                        case 24:
                            processMirrorData(lineNum, line);
                            break;
                        case 25:
                            processMplsLabelData(lineNum, line);
                            break;
                        case 26:
                            processMacSAMSBs(lineNum, line);
                            break;
                        case 27:
                            processNatData(lineNum, line);
                            break;
                        case 28:
                            processL2Ipv4IaclData(lineNum, line);
                            break;
                        case 29:
                            processL2Ipv4IaclKey(lineNum, line);
                            break;
                        case 30:
                            processL2Ipv4WcmKey(lineNum, line);
                            break;
                        case 32:
                            processL2EaclData(lineNum, line);
                            break;
                        case 33:
                            processOpenFlowKeyMaskData(lineNum, line);
                            break;
                        case 34:
                            processOpenFlowActionData(lineNum, line);
                            break;
                        case 35:
                            processIpv4SaiNeighborData(lineNum, line);
                            break;
                        case 36:
                            processIpv4SaiNhData(lineNum, line);
                            break;
                        case 37:
                            processIpv4SaiRouteData(lineNum, line);
                            break;
                        case 38:
                            processIpv6SaiNeighborData(lineNum, line);
                            break;
                        case 39:
                            processIpv6SaiNhData(lineNum, line);
                            break;
                        case 40:
                            processIpv6SaiRouteData(lineNum, line);
                            break;
                        case 41:
                            processVrfData(lineNum, line);
                            break;
                        case 42:
                            processSaiNhGroupData(lineNum, line);
                            break;
                        case 43:
                            processQosSchedulerData(lineNum, line);
                            break;
                        case 44:
                            processQosShaperPortData(lineNum, line);
                            break;
                        case 45:
                            processQosShaperQueueData(lineNum, line);
                            break;
                        case 46:
                            processQosAqmQProfileData(lineNum, line);
                            break;
                        case 47:
                            processQosEgressMapData(lineNum, line);
                            break;
                        case 48:
                            processVlanScopeData(lineNum, line);
                            break;
                        case 49:
                            processMacScopeData(lineNum, line);
                            break;
                        case 50:
                            processIpxNhScopeData(lineNum, line);
                            break;
                        case 51:
                            processIpv4HostScopeData(lineNum, line);
                            break;
                        case 52:
                            processIpv4RouteScopeData(lineNum, line);
                            break;
                        case 53:
                            processSaiQosQueueSchedulerData(lineNum, line);
                            break;
                        case 54:
                            processQosIngressMapData(lineNum, line);
                            break;
                        case 55:
                            processPolicerData(lineNum, line);
                            break;
                        case 56:
                            processFdbControlMacData(lineNum, line);
                            break;
                        case 57:
                            processIpv4HostControlData(lineNum, line);
                            break;
                        case 58:
                            processIpv6HostControlData(lineNum, line);
                            break;
                        case 59:
                            processCoppData(lineNum, line);
                            break;
                        case 60:
                            processTrustBrData(lineNum, line);
                            break;
                        case 61:
                            processVifData(lineNum, line);
                            break;
                        case 62:
                            processSRHData(lineNum, line);
                            break;
                        case 63:
                            processLocalSidData(lineNum, line);
                            break;
                        case 64:
                            processPBRData(lineNum, line);
                            break;
                        case 65:
                            processPureOfKeyMaskData(lineNum, line);
                            break;
                        case 66:
                            processPureOfActionData(lineNum, line);
                            break;
                        case 72:
                            processIpvxIaclData(lineNum, line);
                            break;
                        case 73:
                            processIpvxIaclKey(lineNum, line);
                            break;
                        default:
                            printf("Too many sections in file \" %s \".  Skipping line # %d .\n",
                                   configFile.filename, lineNum);
                            break;
                    }
                }
            }
        }
        else if ((errorCode= ferror(config)) != 0)
        {
            printf("Error reading line # %d of file \" %s \": %s \n", lineNum,
                   configFile.filename, strerror(errorCode));
            printf("Exiting...\n");
            fclose(config);
            //exit(0);    // If the configuration file is broken then give up.
            return XP_ERR_FILE_READ;
        }
        ++lineNum;
    }

    fclose(config);

    //    Debug code:
    //    printConfigData();
    //
    return XP_NO_ERR;
}

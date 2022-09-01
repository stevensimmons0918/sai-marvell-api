/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

//
// file cpssHalDevice.c
//


#include "xpsCommon.h"
#include "cpssHalDeviceStruct.h"
#include "cpssHalDevice.h"
#include "cpssHalProfile.h"
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define CPSSHAL_SWITCH(x)           globalSwitchDb[cpssHalCurrentSwitchId].x
#define CPSSHAL_DEV_STATE(d)        globalSwitchDb[cpssHalCurrentSwitchId].deviceCfg[d].state
#define CPSSHAL_DEV_PROFILE(d)      globalSwitchDb[cpssHalCurrentSwitchId].deviceCfg[d].profile
#define CPSSHAL_DEV_NUMPORTS(d)     globalSwitchDb[cpssHalCurrentSwitchId].deviceCfg[d].numOfLPorts
#define CPSSHAL_DEV_NUMCSCDPORTS(d) globalSwitchDb[cpssHalCurrentSwitchId].deviceCfg[d].numOfCscdPorts
#define CPSSHAL_DEV_NUMCPUPORTS(d)  globalSwitchDb[cpssHalCurrentSwitchId].deviceCfg[d].numOfCPUPorts
#define CPSSHAL_DEV_PORTMAP(d)      globalSwitchDb[cpssHalCurrentSwitchId].deviceCfg[d].portMap
#define CPSSHAL_DEV_PORTACTIVE(d)   globalSwitchDb[cpssHalCurrentSwitchId].deviceCfg[d].portActive
#define CPSSHAL_DEV_CSCDPORTMAP(d)  globalSwitchDb[cpssHalCurrentSwitchId].deviceCfg[d].cscdPortMap
#define CPSSHAL_DEV_TYPE(d)         globalSwitchDb[cpssHalCurrentSwitchId].deviceCfg[d].devType


#define UNUSED_PARAM(x) (void)(x)
cpssHalDeviceCfg globalSwitchDb[MAX_GLOBAL_DEVICES];
int cpssHalCurrentSwitchId = CPSS_GLOBAL_SWITCH_ID_0;

extern  HW_TABLE_PROFILE_STC falcon_hw_table_sizes_profile[];
extern  HW_TABLE_PROFILE_STC ac3x_3255_hw_table_sizes_profile[];
extern  HW_TABLE_PROFILE_STC ac3x_3265_hw_table_sizes_profile[];
extern  HW_TABLE_PROFILE_STC ac5x_hw_table_sizes_profile[];
extern  HW_TABLE_PROFILE_STC aldrin2_xl_hw_table_sizes_profile[];

XP_STATUS cpssHalSetDeviceSwitchId(int globalSwitchId)
{
    if (MAX_GLOBAL_DEVICES <= globalSwitchId)
    {
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }
    if (globalSwitchId > CPSS_GLOBAL_SWITCH_ID_0)
    {
        return XP_ERR_INVALID_INPUT;
    }
    cpssHalCurrentSwitchId = globalSwitchId;

    return XP_NO_ERR;
}

XP_STATUS cpssHalSetNumDevices(int numDevices)
{
    CPSSHAL_SWITCH(numPPDevices) = numDevices;

    return XP_NO_ERR;
}

int cpssHalGetMaxDevices(void)
{
    return CPSSHAL_SWITCH(numPPDevices);
}

unsigned int cpssHalGetSKUMaxPhyPorts(xpsDevice_t devId)
{
    if (NULL == CPSSHAL_SWITCH(halHwsku))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }
    return CPSSHAL_SWITCH(halHwsku)->maxPhyPorts;
}
unsigned int cpssHalGetSKUMaxVlanIds(xpsDevice_t devId)
{
    if (NULL == CPSSHAL_SWITCH(halHwsku))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }
    return CPSSHAL_SWITCH(halHwsku)->maxVlans;
}

unsigned int cpssHalGetSKUMaxLAGrps(xpsDevice_t devId)
{
    if (NULL == CPSSHAL_SWITCH(halHwsku))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }
    return CPSSHAL_SWITCH(halHwsku)->maxLAgrps;
}

unsigned int cpssHalSetSKUMaxLAGrps(uint32_t maxVal)
{
    if (NULL == CPSSHAL_SWITCH(halHwsku))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }
    CPSSHAL_SWITCH(halHwsku)->maxLAgrps = maxVal;
    return 0;
}
unsigned int cpssHalGetSKUmaxMCgrps(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku)->maxMCgrps;
}

unsigned int cpssHalGetSKUmaxMCeVidxgrps(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku)->maxMCeVidxgrps;
}
unsigned int cpssHalGetSKUmaxSTPgrps(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku)->maxSTPgrps;
}
unsigned int cpssHalGetSKUmaxLAGMbrPerGrp(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku)->maxLAGMbrPerGrp;
}
unsigned int cpssHalGetSKUmaxMCMLLPair(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxMllPair;
}
unsigned int cpssHalGetSKUmaxHashTable(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxHashTable;
}
unsigned int cpssHalGetSKUmaxRouteLPM(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxRouteLPM;
}
unsigned int cpssHalGetSKUmaxExactMatch(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxExactMatch;
}
unsigned int cpssHalGetSKUmaxPBR(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxPBR;
}
unsigned int cpssHalGetSKUmaxNH(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxNH;
}
unsigned int cpssHalGetSKUmaxNhEcmpGrps(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxNhEcmpGrps;
}
unsigned int cpssHalSetSKUmaxNhEcmpMbrPerGrp(uint32_t maxVal)
{
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }
    CPSSHAL_SWITCH(halHwsku2)->maxNhEcmpMbrPerGrp = maxVal;
    return 0;
}
unsigned int cpssHalGetSKUmaxNhEcmpMbrPerGrp(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxNhEcmpMbrPerGrp;
}
unsigned int cpssHalGetSKUmaxARP(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxARP;
}

unsigned int cpssHalGetSKUmaxNeighbours(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxNeighbours;
}
unsigned int cpssHalGetSKUmaxTunStart(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxTunStart;
}
unsigned int cpssHalGetSKUmaxNATTable(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxNATTable;
}
unsigned int cpssHalGetSKUmaxVRF(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxVRF;
}
unsigned int cpssHalGetSKUmaxIACL(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxIACL;
}
unsigned int cpssHalGetSKUmaxEACL(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxEACL;
}
unsigned int cpssHalGetSKUmaxTunTerm(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxTunTerm;
}
unsigned int cpssHalGetSKUmaxIVIF(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }
    return CPSSHAL_SWITCH(halHwsku2)->maxIVIF;

}
unsigned int cpssHalGetSKUmaxEVIF(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }
    return CPSSHAL_SWITCH(halHwsku2)->maxEVIF;

}
unsigned int cpssHalGetSKUmaxMirrorSessions(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }

    return CPSSHAL_SWITCH(halHwsku2)->maxMirrorSessions;
}
unsigned int cpssHalGetSKUmaxMac2Me(xpsDevice_t devId)
{
    UNUSED_PARAM(devId);
    if (NULL == CPSSHAL_SWITCH(halHwsku2))
    {
        cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
        return -1;
    }
    return CPSSHAL_SWITCH(halHwsku2)->maxMac2Me;
}

/*This api gets the shared memory sizes from the cpss
and upadtes the cpssHal variables.
Relevant for devices with configurable shared mem.
Current supported :- Falcon*/
extern
GT_STATUS sharedTablesParamsGet(
    IN  GT_U8   devNum,
    OUT  GT_U32  *fdbSizePtr,
    OUT  GT_U32  *emSizePtr,
    OUT  GT_U32  *lpmPrefixNumPtr,
    OUT  GT_U32  *arpSizePtr
);
XP_STATUS cpssHalSetSKUmaxValues(xpsDevice_t devId)
{
    switch (cpssHalDevPPFamilyGet(devId))
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            if (NULL == CPSSHAL_SWITCH(halHwsku2))
            {
                cpssOsPrintf("CRITICAL: %s halHwsku is NULL \n", __func__);
                return XP_ERR_NULL_POINTER;
            }
            CPSSHAL_SWITCH(halHwsku2)->maxHashTable = PRV_CPSS_DXCH_PP_MAC(
                                                          devId)->fineTuning.tableSize.fdb;
            CPSSHAL_SWITCH(halHwsku2)->maxRouteLPM = PRV_CPSS_DXCH_PP_MAC(
                                                         devId)->fineTuning.tableSize.lpmRam;
            CPSSHAL_SWITCH(halHwsku2)->maxExactMatch = PRV_CPSS_DXCH_PP_MAC(
                                                           devId)->fineTuning.tableSize.emNum;
            CPSSHAL_SWITCH(halHwsku2)->maxARP = PRV_CPSS_DXCH_PP_MAC(
                                                    devId)->fineTuning.tableSize.routerArp;

        default:
            break;
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalAddDevice(int devNum)
{
    CPSSHAL_SWITCH(isValid) = true;
    switch (CPSSHAL_DEV_STATE(devNum))
    {
        case DEVICE_NOT_VALID:
            CPSSHAL_DEV_STATE(devNum) = DEVICE_INIT_IN_PROGRESS;
            CPSSHAL_SWITCH(numDevAdded)++;
            break;
        default:
            break;
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalInitDevice(int devNum, XP_DEV_TYPE_T devType, void *profileMap,
                            int numOfLPorts)
{

    PROFILE_STC *profile = (PROFILE_STC *)profileMap;
    int itr;

    CPSSHAL_DEV_PROFILE(devNum) = profile;
    CPSSHAL_DEV_TYPE(devNum) = devType;
    CPSSHAL_DEV_NUMPORTS(devNum) = numOfLPorts;
    CPSSHAL_DEV_STATE(devNum) = DEVICE_INITED;
    CPSSHAL_DEV_NUMCSCDPORTS(devNum) = 0;
    CPSSHAL_DEV_NUMCPUPORTS(devNum) = 0;

    CPSSHAL_SWITCH(startPort) = 0xFFFF;
    CPSSHAL_SWITCH(numOfGPorts) += numOfLPorts;
    cpssOsMemSet(CPSSHAL_DEV_PORTMAP(devNum), 0,
                 sizeof(CPSSHAL_DEV_PORTMAP(devNum)));
    cpssOsMemSet(CPSSHAL_DEV_PORTACTIVE(devNum), 0,
                 sizeof(CPSSHAL_DEV_PORTACTIVE(devNum)));
    cpssOsMemSet(CPSSHAL_DEV_CSCDPORTMAP(devNum), 0,
                 sizeof(CPSSHAL_DEV_CSCDPORTMAP(devNum)));
    itr = 0;
    while (profile[itr].profileType != PROFILE_TYPE_LAST_E)
    {
        if (profile[itr].profileType == PROFILE_TYPE_PORT_MAP_E)
        {
            CPSSHAL_DEV_PORTMAP(devNum)[profile[itr].profileValue.portMap.frontPanelPortIdx]
                = profile[itr].profileValue.portMap.portNum;
            CPSSHAL_DEV_PORTACTIVE(
                devNum)[profile[itr].profileValue.portMap.frontPanelPortIdx] = 1;
            if (CPSSHAL_SWITCH(startPort) > profile[itr].profileValue.portMap.portNum)
            {
                CPSSHAL_SWITCH(startPort) = profile[itr].profileValue.portMap.portNum;
            }
        }
        else if (profile[itr].profileType == PROFILE_TYPE_CASCADE_PORT_MAP_E)
        {
            CPSSHAL_DEV_CSCDPORTMAP(devNum)[CPSSHAL_DEV_NUMCSCDPORTS(
                                                devNum)] = profile[itr].profileValue.portMap.portNum;
            CPSSHAL_DEV_NUMCSCDPORTS(devNum)++;
        }
        //else if (profile[itr].profileType == PROFILE_TYPE_HW_SKU_E)
        //{
        //    CPSSHAL_SWITCH(halHwsku) = profile[itr].profileValue.hwTableSizes;
        //}
        else if (profile[itr].profileType == PROFILE_TYPE_CPU_PORT_MAP_E)
        {
            CPSSHAL_DEV_NUMCPUPORTS(devNum)++;
        }
        itr++;
    }

    return XP_NO_ERR;
}

int cpssHalGetGlobalStartPort(void)
{
    return CPSSHAL_SWITCH(startPort);
}

XP_STATUS cpssHalGetMaxGlobalPorts(int *numOfGPorts)
{
    *numOfGPorts = CPSSHAL_SWITCH(numOfGPorts);
    return XP_NO_ERR;
}

XP_STATUS cpssHalGetMaxPorts(int devNum, int *numOfLPorts)
{
    *numOfLPorts = CPSSHAL_DEV_NUMPORTS(devNum);
    return XP_NO_ERR;
}

XP_STATUS cpssHalGetMaxCPUPorts(int devNum, int *numOfCPUPorts)
{
    *numOfCPUPorts = CPSSHAL_DEV_NUMCPUPORTS(devNum);
    return XP_NO_ERR;
}

XP_STATUS cpssHalGetMaxCascadedPorts(int devNum, int *numOfCscdPorts)
{
    *numOfCscdPorts = CPSSHAL_DEV_NUMCSCDPORTS(devNum);
    return XP_NO_ERR;
}

XP_STATUS cpssHalGetCascadedPortNum(int devNum, int portIdx, int *cscdPortNum)
{
    if (portIdx < CPSSHAL_DEV_NUMCSCDPORTS(devNum))
    {
        *cscdPortNum = CPSSHAL_DEV_CSCDPORTMAP(devNum)[portIdx];
    }
    else
    {
        *cscdPortNum = -1;
    }
    return XP_NO_ERR;
}

int cpssHalGetNextValidPort(int portIdx)
{
    int maxPorts, i;
    int devNum;
    int lPort;
    int itr;

    cpssHalG2LPortNum(portIdx, &devNum, &lPort);
    cpssHalGetMaxPorts(devNum, &maxPorts);

    i = ++portIdx;
    while (i<MAX_PORTS_PER_PP)
    {
        for (itr = 0; itr < CPSSHAL_SWITCH(numPPDevices); itr++)
        {
            if (CPSSHAL_DEV_PORTACTIVE(itr)[i])
            {
                return i;
            }
        }
        i++;
    }
    /* Invalid Port - No More Next Port */
    return 0xFFFF;

}


XP_STATUS cpssHalG2LPortNum(int gPort, int *devNum, int *lPort)
{
    int itr;

    *devNum = 0;
    *lPort = 0;
    if (gPort >= MAX_PORTS_PER_PP)
    {
        return XP_PORT_NOT_VALID;
    }

    for (itr = 0; itr < CPSSHAL_SWITCH(numPPDevices); itr++)
    {
        if (CPSSHAL_DEV_PORTACTIVE(itr)[gPort])
        {
            *devNum = itr;
            *lPort = CPSSHAL_DEV_PORTMAP(itr)[gPort];
            return XP_NO_ERR;
        }
    }

    return XP_PORT_NOT_VALID;
}

XP_STATUS cpssHalL2GPortNum(int devNum, int lPort, int *gPort)
{
    int itr;
    int idxPort;

    for (itr = 0; itr < CPSSHAL_SWITCH(numPPDevices); itr++)
    {
        for (idxPort = 0; idxPort < MAX_PORTS_PER_PP; idxPort++)
        {
            if ((CPSSHAL_DEV_PORTACTIVE(itr)[idxPort]) &&
                (CPSSHAL_DEV_PORTMAP(itr)[idxPort] == lPort))
            {
                *gPort = idxPort;
                return XP_NO_ERR;
            }
        }
    }
    *gPort = 0;
    return XP_PORT_NOT_VALID;
}

GT_U8 xpsGlobalIdToDevId(xpsDevice_t devNum, uint32_t portNum)
{
    GT_U8 ret;
    int ldev, lport;
    XP_STATUS rc;
    XP_DEV_TYPE_T   devType;

    // Change to switch
    rc = cpssHalG2LPortNum(portNum, &ldev, &lport);
    if (XP_NO_ERR == rc)
    {
        ret=(0xff & ldev);
    }
    else
    {
        ret=(0xff);
    }

    cpssHalGetDeviceType(devNum, &devType);

    if ((portNum == 264) || (!IS_DEVICE_FALCON(devType) &&
                             portNum == 63)) /*Falcon=264;Other=63*/
    {
        ret = 0;
    }

    return ret;
}

GT_U32 xpsGlobalPortToPortnum(xpsDevice_t devNum, uint32_t portNum)
{
    //Converts the data type uint8_t to GT_U32 for portnum
    GT_U32 ret;
    int ldev, lport;
    XP_STATUS rc;
    XP_DEV_TYPE_T   devType;

    // Change device to switchId
    rc = cpssHalG2LPortNum(portNum, &ldev, &lport);
    if (XP_NO_ERR == rc)
    {
        ret=(0xffff & lport);
    }
    else
    {
        ret=(0xffff);
    }

    cpssHalGetDeviceType(devNum, &devType);

    if ((portNum == 264) || (!IS_DEVICE_FALCON(devType) &&
                             portNum == 63)) /*Falcon=264;Other=63*/
    {
        ret = CPSS_CPU_PORT_NUM_CNS;
    }

    return ret;
}

GT_U32 xpsLocalPortToGlobalPortnum(xpsDevice_t devNum, uint32_t portNum)
{
    int gPort;
    cpssHalL2GPortNum(devNum, portNum, &gPort);

    return gPort;
}

XP_STATUS cpssHalGetDeviceType(xpsDevice_t devId, XP_DEV_TYPE_T *devType)
{
    if (cpssHalCurrentSwitchId < devId)
    {
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }
    *devType = CPSSHAL_DEV_TYPE(devId);
    return XP_NO_ERR;
}

int cpssHalIsEbofDevice(xpsDevice_t devId)
{
    if (cpssHalCurrentSwitchId < devId)
    {
        return 0;
    }

    if (IS_DEVICE_EBOF_PROFILE(CPSSHAL_DEV_TYPE(devId)))
    {
        return 1;
    }

    return 0;
}

XP_STATUS cpssHalGetProfile(int devNum, PROFILE_STC **profile)
{
    *profile = CPSSHAL_DEV_PROFILE(devNum);
    return XP_NO_ERR;
}

XP_STATUS cpssHalGetIpv4EntriesCount(xpsDevice_t devId, uint32_t *numEntries)
{
    GT_STATUS ret = GT_OK;
    uint32_t tblSize = 0;

    switch (cpssHalDevPPFamilyGet(devId))
    {
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            *numEntries = ALD_IPV4_ROUTE_ENTRY_MAX;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            ret = cpssDxChCfgTableNumEntriesGet(devId, CPSS_DXCH_CFG_TABLE_LPM_RAM_E,
                                                &tblSize);
            if (ret != GT_OK)
            {
                return XP_ERR_INIT;
            }
            *numEntries = tblSize;
            break;
        default:
            *numEntries = 0;
            return XP_ERR_NOT_SUPPORTED;
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalGetIpv6EntriesCount(xpsDevice_t devId, uint32_t *numEntries)
{
    GT_STATUS ret = GT_OK;
    uint32_t tblSize = 0;

    switch (cpssHalDevPPFamilyGet(devId))
    {
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            *numEntries = ALD_IPV6_ROUTE_ENTRY_MAX;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            ret = cpssDxChCfgTableNumEntriesGet(devId, CPSS_DXCH_CFG_TABLE_LPM_RAM_E,
                                                &tblSize);
            if (ret != GT_OK)
            {
                return XP_ERR_INIT;
            }
            *numEntries = tblSize/4;
            break;
        default:
            *numEntries = 0;
            return XP_ERR_NOT_SUPPORTED;
    }
    return XP_NO_ERR;
}

CPSS_PP_DEVICE_TYPE cpssHalDevPPTypeGet(GT_U8 devId)
{
    CPSS_DXCH_CFG_DEV_INFO_STC      devInfo;
    GT_STATUS st = GT_OK;

    st = cpssDxChCfgDevInfoGet(devId, &devInfo);
    if (st != GT_OK)
    {
        cpssOsPrintf("Failed to get dev info. dev = %d ret = %d\n", devId, st);
        return 0xFFFFFFFF;
    }

    return devInfo.genDevInfo.devType;
}

CPSS_PP_FAMILY_TYPE_ENT cpssHalDevPPFamilyGet(GT_U8 devId)
{
    CPSS_DXCH_CFG_DEV_INFO_STC      devInfo;
    GT_STATUS st = GT_OK;

    st = cpssDxChCfgDevInfoGet(devId, &devInfo);
    if (st != GT_OK)
    {
        cpssOsPrintf("Failed to get dev info. dev = %d ret = %d\n", devId, st);
        return CPSS_MAX_FAMILY;
    }

    return devInfo.genDevInfo.devFamily;
}

GT_STATUS cpssHalPortManagerIsSupported(GT_U8 devId)
{
    switch (cpssHalDevPPFamilyGet(devId))
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
#ifndef ASIC_SIMULATION
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
#endif
            return GT_OK;
        default:
            return GT_NOT_SUPPORTED;
    }
}

GT_STATUS cpssHalFdbManagerIsSupported(GT_U8 devId)
{
    switch (cpssHalDevPPFamilyGet(devId))
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            return GT_OK;

        case CPSS_PP_FAMILY_DXCH_AC3X_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        default:
            return GT_NOT_SUPPORTED;
    }
    return GT_NOT_SUPPORTED;
}

GT_STATUS cpssHalSwitchUpdateSku(XP_DEV_TYPE_T devType)
{
    if (IS_DEVICE_FALCON(devType))
    {
        CPSSHAL_SWITCH(halHwsku2) = &falcon_hw_table_sizes_profile[0];
    }
    else if (devType == ALDB2B)
    {
        CPSSHAL_SWITCH(halHwsku2) = &ac3x_3255_hw_table_sizes_profile[0];
    }
    else if (devType == TG48M_P)
    {
        CPSSHAL_SWITCH(halHwsku2) = &ac3x_3265_hw_table_sizes_profile[0];
    }
    else if (devType == ALDRIN2XL)
    {
        CPSSHAL_SWITCH(halHwsku2) = &aldrin2_xl_hw_table_sizes_profile[0];
    }
    else if (IS_DEVICE_AC5X(devType))
    {
        CPSSHAL_SWITCH(halHwsku2) = &ac5x_hw_table_sizes_profile[0];
    }
    else if (IS_DEVICE_FUJITSU_LARGE(devType))
    {
        CPSSHAL_SWITCH(halHwsku2) = &aldrin2_xl_hw_table_sizes_profile[0];
    }
    else if (IS_DEVICE_FUJITSU_SMALL(devType))
    {
        CPSSHAL_SWITCH(halHwsku2) = &ac3x_3255_hw_table_sizes_profile[0];
    }
    else
    {
        cpssOsPrintf("cpssHalSwitchUpdateSku invalid devType %d\n", devType);
        return GT_ERROR;
    }
    return GT_OK;
}

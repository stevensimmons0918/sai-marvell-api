/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

// cpssHalDevice.h


#ifndef _cpssHalDevice_h_
#define _cpssHalDevice_h_
#include "cpssCommonDefs.h"
#include "openXpsEnums.h"
#include "gtEnvDep.h"
#include "xpsCommon.h"
#include "cpssDxChCfgInit.h"
#include "cpssHalDeviceGlobals.h"

#ifdef __cplusplus
extern "C" {
#endif



#define CPSS_GLOBAL_START_PORT      cpssHalGetGlobalStartPort()


/**
 * \brief cpssHalSetDeviceSwitchId
 *
 * \param [in] globalSwitchId
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalSetDeviceSwitchId(int globalSwitchId);

/**
 * \brief cpssHalSetNumDevices
 *
 * \param [in] numDevices
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalSetNumDevices(int numDevices);

/**
 * \brief cpssHalAddDevice
 *
 * \param [in] devNum
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalAddDevice(int devNum);

/**
 * \brief cpssHalInitDevice
 *
 * \param [in] devNum local
 * \param [in] profile
 * \param [in] devType
 * \param [in] numOfLPorts local
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalInitDevice(int devNum, XP_DEV_TYPE_T devType, void *profile,
                            int numOfLPorts);

/**
 * \brief cpssHalGetMaxGlobalPorts
 *
 * \param [out] numOfLPorts local
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalGetMaxGlobalPorts(int *numOfGPorts);

/**
 * \brief cpssHalGetGlobalStartPort
 *
 *
 * \return startPort
 */
int cpssHalGetGlobalStartPort(void);

/**
 * \brief cpssHalGetMaxPorts
 *
 * \param [in] devNum local
 * \param [out] numOfLPorts local
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalGetMaxPorts(int devNum, int *numOfLPorts);

/**
 * \brief cpssHalGetMaxCPUPorts
 *
 * \param [in] devNum local
 * \param [out] numOfCPUPorts local
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalGetMaxCPUPorts(int devNum, int *numOfCPUPorts);

/**
 * \brief cpssHalGetMaxDevices
 *
 * \param [in] none
 *
 * \return int max devices
 */
int cpssHalGetMaxDevices(void);

/**
 * \brief cpssHalGetSKUMaxPhyPorts
 *
 * \param [in] none
 *
 * \return int max physical ports
 */
unsigned int cpssHalGetSKUMaxPhyPorts(xpsDevice_t devId);
/**
 * \brief cpssHalGetSKUMaxVlanIds
 *
 * \param [in] none
 *
 * \return int max devices
 */
unsigned int cpssHalGetSKUMaxVlanIds(xpsDevice_t devId);
/**
 * \brief cpssHalGetSKUMaxLAGrps
 *
 * \param [in] none
 *
 * \return int max devices
 */
unsigned int cpssHalGetSKUMaxLAGrps(xpsDevice_t devId);
unsigned int cpssHalSetSKUMaxLAGrps(uint32_t maxVal);
unsigned int cpssHalSetSKUmaxNhEcmpMbrPerGrp(uint32_t maxVal);

unsigned int cpssHalGetSKUmaxMCgrps(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxMCeVidxgrps(xpsDevice_t devId);
unsigned int cpssHalGetSKUmaxMCMLLPair(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxSTPgrps(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxLAGMbrPerGrp(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxHashTable(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxRouteLPM(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxExactMatch(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxPBR(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxNH(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxNhEcmpGrps(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxNhEcmpMbrPerGrp(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxARP(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxNeighbours(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxTunStart(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxNATTable(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxVRF(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxIACL(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxEACL(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxTunTerm(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxIVIF(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxEVIF(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxMirrorSessions(xpsDevice_t devId)       ;
unsigned int cpssHalGetSKUmaxMac2Me(xpsDevice_t devId)       ;
/**
 * \brief cpssHalG2LPortNum
 *
 * \param [in] gPort global portNum devNum local
 * \param [out] devNum local
 * \param [out] lPort local portNum
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalG2LPortNum(int gPort, int *devNum, int *lPort);

/**
 * \brief cpssHalL2GPortNum
 *
 * \param [in] devNum local
 * \param [in] lPort local portNum
 * \param [out] gPort global portNum
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalL2GPortNum(int devNum, int lPort, int *gPort);

/**
 * \brief xpsGlobalIdToDevId Changing the type of xpsDevice_t to GT_U8
 *
 * \param [in] switchId
 * \param [in] portNum
 *
 * \return Local devices' deviceId
 */
GT_U8 xpsGlobalIdToDevId(xpsDevice_t devNum, uint32_t portNum);

/**
 * \brief xpsGlobalPortToPortnum
 *          Changing the type of portnum from uint8_t  to GT_U32
 *
 * \param [in] devNum
 * \param [in] portNum
 *
 * \return Local devices' port num
 */
GT_U32 xpsGlobalPortToPortnum(xpsDevice_t devNum, uint32_t portNum);


/**
 * \brief xpsGlobalPortToPortnum
 *          Changing the type of portnum from uint8_t  to GT_U32
 *
 * \param [in] devNum
 * \param [in] portNum
 *
 * \return Local devices' port num
 */
GT_U32 xpsLocalPortToGlobalPortnum(xpsDevice_t devNum, uint32_t portNum);

/**
 * \brief cpssHalGetMaxCascadedPorts
 *
 * \param [in] devNum
 * \param [out] numOfCscdPorts
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalGetMaxCascadedPorts(int devNum, int *numOfCscdPorts);

/**
 * \brief cpssHalGetCascadedPortNum
 *
 * \param [in] devNum
 * \param [in] portIdx
 * \param [out] cscdPortNum
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalGetCascadedPortNum(int devNum, int portIdx, int *cscdPortNum);

/**
 * \brief cpssHalGetDeviceType
 *
 * \param [in] devId
 * \param [out] devType
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalGetDeviceType(xpsDevice_t devId, XP_DEV_TYPE_T *devType);

/**
 * \brief cpssHalIsEbofDevice
 *
 * \param [in] devId
 *
 * \return int
 */
int cpssHalIsEbofDevice(xpsDevice_t devId);

/**
 * \brief cpssHalGetNextValidPort
 *
 * \param [in] PortIdx
 * \return  next valid port
 */
XP_STATUS cpssHalSetSKUmaxValues(xpsDevice_t devId) ;

int cpssHalGetNextValidPort(int portIdx);

/* Helper macros for device / port iteration */
#define XPS_DEVICES_PER_SWITCH_ITER(_switchId, devNum)  cpssHalSetDeviceSwitchId(_switchId);    \
                                                        for(devNum=0; devNum < cpssHalGetMaxDevices(); devNum++)
/**
 * \brief cpssHalGetIpv4EntriesCount
 *
 * \param [in] devId
 * \param [out] numEntries
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalGetIpv4EntriesCount(xpsDevice_t devId, uint32_t *numEntries);

/**
 * \brief cpssHalGetIpv6EntriesCount
 *
 * \param [in] devId
 * \param [out] numEntries
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalGetIpv6EntriesCount(xpsDevice_t devId, uint32_t *numEntries);

/**
 * \brief cpssHalDevPPFamilyGet
 *
 * \param [in] devId
 *
 * \return CPSS_PP_FAMILY_TYPE_ENT on success
 *         CPSS_MAX_FAMILY on failure
 */
CPSS_PP_FAMILY_TYPE_ENT cpssHalDevPPFamilyGet(GT_U8 devId);

CPSS_PP_DEVICE_TYPE cpssHalDevPPTypeGet(GT_U8 devId);

/**
 * \brief cpssHalPortManagerIsSupported
 *
 * \param [in] devId
 *
 * \return GT_OK on success
 *         GT_NOT_SUPPORTED on failure
 */
GT_STATUS cpssHalPortManagerIsSupported(GT_U8 devId);

/**
 * \brief cpssHalFdbManagerIsSupported
 *
 * \param [in] devId
 *
 * \return GT_OK on success
 *         GT_NOT_SUPPORTED on failure
 */
GT_STATUS cpssHalFdbManagerIsSupported(GT_U8 devId);

GT_STATUS cpssHalSwitchUpdateSku(XP_DEV_TYPE_T devType);

#define XPS_B2B_CSCD_PORTS_ITER(devNum, portNum, cscdGPort, maxPorts)           \
                                                        for(devNum=0; cpssHalGetMaxCascadedPorts(devNum,  &maxPorts), devNum < cpssHalGetMaxDevices(); devNum++)   \
                                                            for(portNum=0; cpssHalGetCascadedPortNum(devNum, portNum, &cscdGPort), portNum < maxPorts; portNum++)

#define XPS_CSCD_PORT_ITER(devNum, portNum, cscdGPort, maxPorts)              \
                                                        for(portNum=0; cpssHalGetMaxCascadedPorts(devNum,  &maxPorts), cpssHalGetCascadedPortNum(devNum, portNum, &cscdGPort), portNum < maxPorts; portNum++)



#define XPS_B2B_PORTS_ITER(devNum, portNum, maxPorts)       for(devNum=0; devNum < cpssHalGetMaxDevices(), cpssHalGetMaxGlobalPorts(&maxPorts); devNum++)   \
                                                                for(portNum=CPSS_GLOBAL_START_PORT; portNum < (maxPorts+CPSS_GLOBAL_START_PORT); portNum++)


#define XPS_GLOBAL_PORT_ITER(port, maxPort)              for(port = CPSS_GLOBAL_START_PORT; ((maxPort-- )>0) && (port != 0xFFFF); port= cpssHalGetNextValidPort(port))

#ifdef __cplusplus
}
#endif

#endif //_cpssHalDevice_h_

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

// cpssHalDeviceStruct.h


#ifndef _cpssHalDeviceStruct_h_
#define _cpssHalDeviceStruct_h_

#include "cpssHalProfile.h"
#include "cpssHalDeviceGlobals.h"


typedef enum
{
    DEVICE_NOT_VALID,
    DEVICE_INIT_IN_PROGRESS,
    DEVICE_INITED,
} cpssHalDeviceState;

/* Per device config */
typedef struct
{
    int deviceId;
    cpssHalDeviceState state;
    void *profile;
    int numOfLPorts;
    int numOfCscdPorts;
    int numOfCPUPorts;
    int portActive[MAX_PORTS_PER_PP];
    int portMap[MAX_PORTS_PER_PP];
    int cscdPortMap[MAX_CASCADE_PORTS_PER_PP];
    XP_DEV_TYPE_T devType;
} cpssHalPerDeviceCfg;

/* Per switch config */
typedef struct
{
    int isValid;
    int numPPDevices;
    int numDevAdded;
    int numOfGPorts;
    PROFILE_TYPE_HW_SKU_STC *halHwsku;
    HW_TABLE_PROFILE_STC *halHwsku2;
    uint32_t startPort;
    cpssHalPerDeviceCfg deviceCfg[MAX_PP_DEVICES_PER_SWITCH];
} cpssHalDeviceCfg;

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief cpssHalGetProfile
 *
 * \param [in] devId
 * \param [out] profile
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalGetProfile(int devNum, PROFILE_STC **profile);

#ifdef __cplusplus
}
#endif

#endif /* _cpssHalDeviceStruct_h_ */

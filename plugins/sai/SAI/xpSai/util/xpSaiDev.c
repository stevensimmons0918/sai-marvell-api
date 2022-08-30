// xpSaiDev.c

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#include "xpSaiDev.h"
#include "xpSaiSwitch.h"
#include "xpSaiFdb.h"
#include "xpSaiCopp.h"
#include "xpSaiBridge.h"
#include "xpSaiAclMapper.h"
#include "xpDevTypes.h"
#include "xpSaiUtil.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif



XP_SAI_LOG_REGISTER_API(SAI_API_UNSPECIFIED);
extern pthread_t saiWmDevIpcHandlerThread;

sai_status_t sai_sdk_dev_type_get(const char* switch_hardware_id,
                                  xpSdkDevType_t* devType)
{
    if (!switch_hardware_id)
    {
        XP_SAI_LOG_ERR("Invalid parameter\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

#ifndef MAC
    *devType = xpPlatformGetBoardType(switch_hardware_id);
    if (*devType != XP_SDK_DEV_TYPE_UNKNOWN)
    {
        return SAI_STATUS_SUCCESS;
    }
#endif // MAC

    return SAI_STATUS_FAILURE;
}


int sai_sdk_init(const char* devName,
                 const xpSaiSwitchProfile_t* profile,
                 xpsRangeProfileType_t rpType)
{
    int status = XP_NO_ERR;
    status = xpsSdkLogVersion();
    if (status)
    {
        XP_SAI_LOG_ERR("xpsSdkLogVersion Failed");
        return status;
    }

    if (profile->logDest == XP_SAI_LOG_FILE)
    {
        // Connect logger to file
        xpsSdkLogToFile(XP_XDK_LOG_FILENAME);
    }
    else
    {
        xpsSdkLogConsole();
    }

    if ((XP_SDK_DEV_TYPE_WM == profile->sdkDevType) ||
        (XP_SDK_DEV_TYPE_SHADOW == profile->sdkDevType))
    {
        xpSetSalType(XP_SAL_WM_TYPE);
    }
    else
    {
        xpSetSalType(XP_SAL_HW_TYPE);
    }

    status = xpSalInit(profile->devType);
    if (status)
    {
        XP_SAI_LOG_ERR("SalInit() Failed with err: %d\n", status);
        return status;
    }

    if ((xpGetSalType() == XP_SAL_HW_TYPE) || (xpGetSalType() == XP_SAL_KERN_TYPE))
    {
    }

    XP_SAI_LOG_INFO("SDK Initialization for Hardware %s devType [%d]\n",
                    profile->hwId, profile->devType);
    status = xpsSdkInit(profile->devType, rpType, profile->initType);
    if (status)
    {
        XP_SAI_LOG_ERR("xpsSdkInit Failed\n");
    }

    return status;
}

sai_status_t sai_sdk_dev_add(xpsDevice_t devId, xpsInitType_t initType,
                             xpDevConfigStruct *devConfig)
{
    XP_STATUS status;


    status = xpsSdkDevInit(devId, initType, devConfig);
    if (status)
    {
        XP_SAI_LOG_ERR("xpsSdkDevInit(devId = 0x%x) failed.\n", devId);
        return SAI_STATUS_FAILURE;
    }

    XP_SAI_LOG_DBG("Device %u added successfully\n", devId);

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiComponentsInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS    status    = XP_NO_ERR;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    status = xpSaiVlanInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVlanInit Failed Error #%1d", status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiNeighborInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiNeighborInit Failed for device id %d Error #%1d",
                       xpSaiDevId, status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiNextHopInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopInit Failed for device id %d Error #%1d",
                       xpSaiDevId, status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiNextHopGroupInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopGroupInit Failed for device id %d Error #%1d",
                       xpSaiDevId, status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiVrfInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfInit failed for device id %d Error #%1d", xpSaiDevId,
                       status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    saiStatus = xpSaiSamplePacketInit(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiSamplePacketInit failed for device id %d Error #%1d",
                       xpSaiDevId, saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiMirrorSessionInit(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiMirrorSessionInit failed for device id %d Error #%1d",
                       xpSaiDevId, status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiRouteInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRouteInit Failed Error #%1d", status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiRouterInterfaceInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRouteInterfaceInit Failed Error #%1d", status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiPortInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiPortInit: Failed for device id %d Error #%1d", xpSaiDevId,
                       status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiFdbInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiFdbInit: Failed for device id %d Error #%1d", xpSaiDevId,
                       status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiQueueInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiQueueInit: Failed for device id %d Error #%1d", xpSaiDevId,
                       status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiSchedulerInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiSchedulerInit: Failed for device id %d Error #%1d",
                       xpSaiDevId, status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiPolicerInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiPolicerInit: Failed for device id %d Error #%1d",
                       xpSaiDevId, status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiWredInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiWredInit: Failed for device id %d Error #%1d", xpSaiDevId,
                       status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    saiStatus = xpSaiQosMapInit(xpSaiDevId);
    if (saiStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiQosMapInit: Failed for device id %d Error #%1d",
                       xpSaiDevId, saiStatus);
        return saiStatus;
    }

    /* xpSaiAclInit depends on xpSaiAclMapperInit, thus is placed inside device-specific inits */
    saiStatus = xpSaiAclInit(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAclInit: Failed for device id %d. Error #%1d\n",
                       xpSaiDevId, saiStatus);
        return saiStatus;
    }

    // TODO: HACK: Looks like init is already done during xpsSdkInit.
    status = xpSaiRouteDevInit(xpSaiDevId);
    if (status)
    {
        XP_SAI_LOG_ERR("xpSaiRouteDevInit: Failed for device id %d. Error #%1d\n",
                       xpSaiDevId, status);
        return status;
    }

    saiStatus = xpSaiHostInterfaceInit(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostInterfaceInit Failed for device id %d Error #%1d",
                       xpSaiDevId, saiStatus);
        return XP_ERR_INIT;
    }

    saiStatus = xpSaiCoppInit(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("COPP init failed for device id %d Error #%1d\n", xpSaiDevId,
                       saiStatus);
        return XP_ERR_INIT;
    }

    saiStatus = xpSaiBufferInit(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiBufferInit: Failed for device id %d Error #%1d",
                       xpSaiDevId, saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiHashInit(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHashInit failed for device id %d Error #%1d\n", xpSaiDevId,
                       saiStatus);
        return saiStatus;
    }

    status = xpSaiSchedulerGroupInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiSchedulerGrpInit: Failed for device id %d Error #%1d",
                       xpSaiDevId, status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiBridgeInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiBridgeInit: Failed for device id %d Error #%1d\n",
                       xpSaiDevId, status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiTunnelInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelInit: Failed for device id %d Error #%1d",
                       xpSaiDevId, status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiLagInit(xpSaiDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiLagInit: Failed for device id %d Error #%1d", xpSaiDevId,
                       status);
        return status;
    }

    saiStatus = xpSaiL2McGroupInit(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiL2McGroupInit: Failed for device id %d Error #%1d\n",
                       xpSaiDevId, saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiL2McInit(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiL2McInit: Failed for device id %d Error #%1d\n",
                       xpSaiDevId, saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiIpmcGroupInit(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiIpmcGroupInit: Failed for device id %d Error #%1d\n",
                       xpSaiDevId, saiStatus);
        return saiStatus;
    }


    XP_SAI_LOG_DBG("SAI components initialized successfully\n");

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiComponentsDeInit(xpsDevice_t xpsDevId)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS    status    = XP_NO_ERR;

    status = xpSaiBridgeDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiBridgeDeInit: Failed for device id %u Error #%1d",
                       xpsDevId, status);
        return xpsStatus2SaiStatus(status);
    }

    saiRetVal = xpSaiAclDeInit(xpsDevId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAclDeInit failed for device id %d Error #%1d", xpsDevId,
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiBufferDeInit(xpsDevId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiBufferDeInit: Failed for device id %u Error #%1d",
                       xpsDevId, saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiQosMapDeInit(xpsDevId);
    if (saiRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiQosMapDeInit: Failed for device id %u Error #%1d",
                       xpsDevId, saiRetVal);
        return saiRetVal;
    }

    status = xpSaiWredDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiWredDeInit: Failed for device id %u Error #%1d", xpsDevId,
                       status);
        return xpsStatus2SaiStatus(status);
    }

    status = xpSaiPolicerDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiPolicerDeInit: Failed for device id %u Error #%1d",
                       xpsDevId, status);
        return xpsStatus2SaiStatus(status);
    }

    status = xpSaiSchedulerDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiSchedulerDeInit: Failed for device id %u Error #%1d",
                       xpsDevId, status);
        return xpsStatus2SaiStatus(status);
    }

    status = xpSaiQueueDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiQueueDeInit: Failed for device id %u Error #%1d", xpsDevId,
                       status);
        return xpsStatus2SaiStatus(status);
    }

    status = xpSaiFdbDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiFdbDeInit: Failed for device id %d Error #%1d", xpsDevId,
                       status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    status = xpSaiPortDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiMirrorSessionDeinit Failed for device id %u Error #%1d",
                       xpsDevId, status);
        return xpsStatus2SaiStatus(status);
    }

    status = xpSaiRouteDeinit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRouteDeinit Failed for device id %u Error #%1d", xpsDevId,
                       status);
        return xpsStatus2SaiStatus(status);
    }

    status = xpSaiRouterInterfaceDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDeinit Failed for device id %u Error #%1d",
                       xpsDevId, status);
        return xpsStatus2SaiStatus(status);
    }

    saiRetVal = xpSaiMirrorSessionDeinit(xpsDevId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiMirrorSessionDeinit Failed for device id %u Error #%1d",
                       xpsDevId, saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiSamplePacketDeinit(xpsDevId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiSamplePacketDeinit Failed for device id %u Error #%1d",
                       xpsDevId, saiRetVal);
        return saiRetVal;
    }

    status = xpSaiVrfDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfDeInit failed for device id %u Error #%1d", xpsDevId,
                       status);
        return xpsStatus2SaiStatus(status);
    }

    status = xpSaiNextHopDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopDeInit failed for device id %u Error #%1d",
                       xpsDevId, status);
        return xpsStatus2SaiStatus(status);
    }

    status = xpSaiNextHopGroupDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopGroupDeInit: Failed for device id %u Error #%1d",
                       xpsDevId, status);
        return xpsStatus2SaiStatus(status);
    }

    status = xpSaiVlanDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVlanDeInit Failed Error #%1d", status);
        return xpsStatus2SaiStatus(status);
    }

    saiRetVal = xpSaiCoppDeInit(xpsDevId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("COPP deinit failed for device id %d Error #%1d", xpsDevId,
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiHostInterfaceDeInit(xpsDevId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostInterfaceDeInit Failed for device id %u Error #%1d",
                       xpsDevId, saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiHashDeInit(xpsDevId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHashDeInit failed for device id %d Error #%1d", xpsDevId,
                       saiRetVal);
        return saiRetVal;
    }

    status = xpSaiSchedulerGroupDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiSchedulerGrpDeInit: Failed for device id %u Error #%1d",
                       xpsDevId, status);
        return xpsStatus2SaiStatus(status);
    }

    status = xpSaiTunnelDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelDeInit: Failed for device id %u Error #%1d",
                       xpsDevId, status);
        return xpsStatus2SaiStatus(status);
    }

    status = xpSaiLagDeInit(xpsDevId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiLagDeInit: Failed for device id %u Error #%1d", xpsDevId,
                       status);
        return xpsStatus2SaiStatus(status);
    }
    saiRetVal = xpSaiL2McDeInit(xpsDevId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiL2McDeInit: Failed for device id %d Error #%1d\n",
                       xpsDevId, saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiL2McGroupDeInit(xpsDevId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiL2McGroupDeInit: Failed for device id %d Error #%1d\n",
                       xpsDevId, saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiIpmcGroupDeInit(xpsDevId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiIpmcGroupDeInit: Failed for device id %d Error #%1d\n",
                       xpsDevId, saiRetVal);
        return saiRetVal;
    }
#if 0
    saiRetVal = xpSaiRpfGroupDeInit(xpsDevId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRpfGroupDeInit: Failed for device id %d Error #%1d\n",
                       xpsDevId, saiRetVal);
        return saiRetVal;
    }
#endif
    return saiRetVal;
}

sai_status_t sai_dev_config(xpsDevice_t xpSaiDevId, void* arg)
{
    sai_status_t       saiStatus = SAI_STATUS_SUCCESS;
    xpSaiWmIpcParam_t  *param    = (xpSaiWmIpcParam_t *) arg;

    if (param == 0)
    {
        XP_SAI_LOG_ERR("Incorrect input parameter specified\n");
        return XP_ERR_NULL_POINTER;
    }

    /* XDK init for specific device and type */
    saiStatus = sai_sdk_dev_add(xpSaiDevId, param->initType, &devDefaultConfig);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to add new device %u. Error #%1d\n", xpSaiDevId,
                       saiStatus);
        return saiStatus;
    }

    /* Initialize SAI components DBs*/
    saiStatus = xpSaiComponentsInit(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not initialize SAI components DBs\n");
        return saiStatus;
    }

    return saiStatus;
}

XP_STATUS sai_sdk_dev_remove(xpsDevice_t xpSaiDevId)
{
    XP_STATUS    status    = XP_NO_ERR;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    xpSaiSetExitStatus();
    //pthread_kill(gSaiFdbLearningThread, SIGUSR2);

    saiStatus = xpSaiDeleteDefaultVlan(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove default SAI VLAN. Error #%1d\n", saiStatus);
        return XP_STATUS_NUM_OF_ENTRIES;
    }

    /* Deinitialize SAI components DBs */
    saiStatus = xpSaiComponentsDeInit(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to deinitialize SAI components DBs. Error #%1d",
                       saiStatus);
        return XP_STATUS_NUM_OF_ENTRIES;
    }

    status = xpsSdkDevDeInit(xpSaiDevId);
    if (status)
    {
        XP_SAI_LOG_ERR("Failed to remove device %u\n", xpSaiDevId);
        return status;
    }

    return XP_NO_ERR;
}

/*This is example, code like this should be some how wrapped and sent by customer probably as part of sevice table.
  For now will be handled as part of xpSaiApp*/
void *sai_dev_ipc_handler(void *arg)
{

    XP_SAI_LOG_DBG("Called \n\n");
    /*TBD: Some mutex protection to be added.*/
    for (;;)
    {

        /* Handle HW/WM incoming requests. */
        if (xpSaiGetExitStatus())
        {
            break;
        }
    }
    return NULL;
}

pthread_t sai_thread_create(const char *name, void *(*start)(void *), void *arg)
{
    pthread_t thread;
    int error;

    XP_SAI_LOG_DBG("Called \n\n");

    error = xpSalThreadCreateName(&thread, name, NULL, start, arg);
    if (error)
    {
        XP_SAI_LOG_ERR("pthread_create failed (errno=%d). Execution will be asserted.",
                       errno);
        assert(1);
    }
    return thread;
}
XP_STATUS  sai_thread_delete(pthread_t thread)
{
    int error;

    error = xpSalThreadDelete(&thread);
    if (error)
    {
        XP_SAI_LOG_ERR("pthread_delete failed (errno=%d). Execution will be asserted.",
                       errno);
        assert(1);
    }
    return (XP_STATUS)error;
}

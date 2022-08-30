// xpSaiBuffer.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiPort.h"
#include "xpSaiBuffer.h"
#include "xpSaiValidationArrays.h"
#include "xpsAllocator.h"
#include "xpSaiStub.h"
#include "xpSaiProfileMng.h"
#include "cpssHalQos.h"
#include "cpssHalUtil.h"


XP_SAI_LOG_REGISTER_API(SAI_API_BUFFER);

#define XP_SAI_BUFFER_POOL_MAX_IDS        4
#define XP_SAI_DYN_POOL_MAX_IDS           2
#define XP_SAI_DYN_POOL_RANGE_START       0
#define XP_SAI_BUFFER_POOL_RANGE_START    0
#define XP_SAI_BUFFER_PROFILE_MAX_IDS     16
#define XP_SAI_BUFFER_PROFILE_RANGE_START 0

/*
 * In SAI, congestion factor = (2^dynamic_th)
 * In Cpss, congestion factor = (Alpha==0)?0:2^(-6+alpha)
 * Based on this, relation b/w dynamic_th and alpha => alpha = dyn_th+6.
 * Alpha cannot be -ve. So for dyn_th < -5, aplha is set to 1. */
#define CONVERT_DYN_TH_TO_CPSS_ALPHA(dynTh) (dynTh < -5) ? 1 : (dynTh + 6)
#define CPSS_PFC_ALPHA_MIN                  0

sai_buffer_api_t* _xpSaiBufferApi;
static xpsDbHandle_t bufferPoolDbHandle    = XPSAI_BUFFER_POOL_DB_HNDL;
static xpsDbHandle_t bufferProfileDbHandle = XPSAI_BUFFER_PROFILE_DB_HNDL;
static xpsDbHandle_t ingressPgDbHandle     = XPSAI_INGRESS_PG_DB_HNDL;

static sai_uint32_t pfcTcActivePgCount[8];
sai_status_t xpSaiSetIngressPriorityGroupAttrBufferProfile(
    sai_object_id_t ingress_pg_id, sai_attribute_value_t value);
static sai_status_t xpSaiSetPfcAvailableBuffersOnPortTc(xpsDevice_t xpDevId,
                                                        sai_object_id_t poolObjId, uint32_t poolAvailableBuffers);


static sai_status_t xpSaiSetPfcThresholdOnPortTc(xpsDevice_t xpDevId,
                                                 uint32_t port, uint32_t pfcTc, sai_int8_t dynamicTh, sai_uint32_t xoffThInPages)
{

    /* Set the port/tc pfc xoff threshold */
    CPSS_DXCH_PFC_THRESHOLD_STC threshold;
    GT_STATUS                   rc;

    threshold.alfa = dynamicTh;
    threshold.guaranteedThreshold = xoffThInPages;

    rc = cpssHalPortPfcPortTcThresholdSet(xpDevId, port, pfcTc, &threshold);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Failed to set  port/tc PFC threshold rc %d\n", rc);
        return cpssStatus2SaiStatus(rc);
    }
    else
    {
        XP_SAI_LOG_DBG("cpssHalPortPfcPortTcThresholdSet port %d tc %d pages %d \n",
                       port, pfcTc, xoffThInPages);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolKeyComp

static sai_int32_t xpSaiBufferPoolKeyComp(void *key1, void *key2)
{
    sai_object_id_t val1 = ((xpSaiBufferPool_t*)key1)->poolObjId;
    sai_object_id_t val2 = ((xpSaiBufferPool_t*)key2)->poolObjId;

    if (val1 > val2)
    {
        return 1;
    }
    else if (val1 < val2)
    {
        return -1;
    }

    return 0;
}

//Func: xpSaiBufferProfileKeyComp

static sai_int32_t xpSaiBufferProfileKeyComp(void *key1, void *key2)
{
    sai_object_id_t val1 = ((xpSaiBufferProfile_t*)key1)->profileObjId;
    sai_object_id_t val2 = ((xpSaiBufferProfile_t*)key2)->profileObjId;

    if (val1 > val2)
    {
        return 1;
    }
    else if (val1 < val2)
    {
        return -1;
    }

    return 0;
}

//Func: xpSaiIngressPgKeyComp

static sai_int32_t xpSaiIngressPgKeyComp(void *key1, void *key2)
{
    sai_object_id_t val1 = ((xpSaiIngressPg_t*)key1)->priorityGroupId;
    sai_object_id_t val2 = ((xpSaiIngressPg_t*)key2)->priorityGroupId;

    if (val1 > val2)
    {
        return 1;
    }
    else if (val1 < val2)
    {
        return -1;
    }

    return 0;
}

//Func: xpSaiBufferInit

sai_status_t xpSaiBufferInit(xpsDevice_t xpDevId)
{
    /*sai_status_t saiStatus = SAI_STATUS_SUCCESS;*/
    XP_STATUS    xpStatus  = XP_NO_ERR;
    GT_STATUS    rc;
    GT_U32       pfcTc;

    /* Init allocators */
    xpStatus = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT,
                                           XP_SAI_ALLOC_BUFFER_POOL, XP_SAI_BUFFER_POOL_MAX_IDS,
                                           XP_SAI_BUFFER_POOL_RANGE_START);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI buffer pool allocator\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT,
                                           XP_SAI_ALLOC_BUFFER_PROFILE, XP_SAI_BUFFER_PROFILE_MAX_IDS,
                                           XP_SAI_BUFFER_PROFILE_RANGE_START);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI buffer profile allocator\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT,
                                           XP_SAI_ALLOC_INGRESS_POOL, XP_SAI_DYN_POOL_MAX_IDS,
                                           XP_SAI_DYN_POOL_RANGE_START);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI ingress pool allocator\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT,
                                           XP_SAI_ALLOC_EGRESS_POOL, XP_SAI_DYN_POOL_MAX_IDS, XP_SAI_DYN_POOL_RANGE_START);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI egress pool allocator\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Register global buffer pool state database */
    xpStatus = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Buffer Pool DB",
                                  XPS_GLOBAL, &xpSaiBufferPoolKeyComp, bufferPoolDbHandle);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to register buffer pool state database, xpStatus: %d\n",
                       xpStatus);
        bufferPoolDbHandle = XPS_STATE_INVALID_DB_HANDLE;
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Register global buffer profile state database */
    xpStatus = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Buffer Profile DB",
                                  XPS_GLOBAL, &xpSaiBufferProfileKeyComp, bufferProfileDbHandle);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to register buffer profile state database, xpStatus: %d\n",
                       xpStatus);
        bufferProfileDbHandle = XPS_STATE_INVALID_DB_HANDLE;
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Register global ingress priority group state database */
    xpStatus = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Ingress PG DB", XPS_GLOBAL,
                                  &xpSaiIngressPgKeyComp, ingressPgDbHandle);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to register ingress priority group state database, xpStatus: %d\n",
                       xpStatus);
        ingressPgDbHandle = XPS_STATE_INVALID_DB_HANDLE;
        return xpsStatus2SaiStatus(xpStatus);
    }

    for (pfcTc=0; pfcTc<8; pfcTc++)
    {
        rc  = cpssHalPortPfcTcResourceModeSet(xpDevId, pfcTc,
                                              CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Error: Failed to set pfc resource mode tc %d rc %d\n", pfcTc,
                           rc);
            return cpssStatus2SaiStatus(rc);
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferDeInit

sai_status_t xpSaiBufferDeInit(xpsDevice_t xpDevId)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    /* De-Register global buffer pool state database */
    xpStatus = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &bufferPoolDbHandle);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to deregister buffer pool state database, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* De-Register global buffer profile state database */
    xpStatus = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &bufferProfileDbHandle);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to deregister buffer profile state database, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* De-Register global ingress priority group state database */
    xpStatus = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &ingressPgDbHandle);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to deregister ingress priority group state database, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return XP_NO_ERR;
}

//Func: xpSaiBufferPoolCreateStateData

static sai_status_t xpSaiBufferPoolCreateStateData(sai_object_id_t poolObjId,
                                                   xpSaiBufferPool_t **ppPoolInfo)
{
    XP_STATUS  xpStatus  = XP_NO_ERR;

    /* Allocate memory for state data */
    xpStatus = xpsStateHeapMalloc(sizeof(xpSaiBufferPool_t), (void**)ppPoolInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }
    memset(*ppPoolInfo, 0x0, sizeof(xpSaiBufferPool_t));

    /* Populate key */
    (*ppPoolInfo)->poolObjId = poolObjId;

    /* Insert the state into database */
    xpStatus = xpsStateInsertData(XP_SCOPE_DEFAULT, bufferPoolDbHandle,
                                  (void*)*ppPoolInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't insert state into data base, xpStatus: %d\n",
                       xpStatus);
        xpsStateHeapFree((void*)*ppPoolInfo);
        return xpsStatus2SaiStatus(xpStatus);
    }

    XP_SAI_LOG_DBG("Successfully created buffer pool state data \n");
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolRemoveStateData

static sai_status_t xpSaiBufferPoolRemoveStateData(sai_object_id_t poolObjId)
{
    xpSaiBufferPool_t *pPoolInfo = NULL;
    xpSaiBufferPool_t  *key = NULL;
    XP_STATUS          xpStatus  = XP_NO_ERR;

    key = (xpSaiBufferPool_t *)xpMalloc(sizeof(xpSaiBufferPool_t));
    if (!key)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for key\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(key, 0, sizeof(xpSaiBufferPool_t));

    /* Populate the key */
    key->poolObjId = poolObjId;

    /* Delete the corresponding state from state database */
    xpStatus = xpsStateDeleteData(XP_SCOPE_DEFAULT, bufferPoolDbHandle,
                                  (xpsDbKey_t)key, (void**)&pPoolInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't not delete buffer pool state data, xpStatus: %d\\n",
                       xpStatus);
        xpFree(key);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Free the memory allocated for the corresponding state */
    xpStatus = xpsStateHeapFree(pPoolInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't free heap memory, xpStatus: %d\n", xpStatus);
        xpFree(key);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpFree(key);
    XP_SAI_LOG_DBG("Successfully removed buffer pool state data \n");
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolGetStateData

static sai_status_t xpSaiBufferPoolGetStateData(sai_object_id_t poolObjId,
                                                xpSaiBufferPool_t **ppPoolInfo)
{
    xpSaiBufferPool_t *key = NULL;
    XP_STATUS         xpStatus = XP_NO_ERR;

    key = (xpSaiBufferPool_t *)xpMalloc(sizeof(xpSaiBufferPool_t));
    if (!key)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for key\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(key, 0, sizeof(xpSaiBufferPool_t));

    /* Populate the key */
    key->poolObjId = poolObjId;

    /* Retrieve the corresponding state from state database */
    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, bufferPoolDbHandle,
                                  (xpsDbKey_t)key, (void**)ppPoolInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to retrieve the state data, xpStatus: %d\n",
                       xpStatus);
        xpFree(key);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (*ppPoolInfo == NULL)
    {
        XP_SAI_LOG_ERR("Error: Buffer pool entry does not exist\n");
        xpFree(key);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    xpFree(key);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolAllocateGuaranteedBuffer

static sai_status_t xpSaiBufferPoolAllocateGuaranteedBuffer(
    sai_object_id_t poolObjId, uint32_t bufferSize)
{
    xpSaiBufferPool_t *pPoolInfo = NULL;
    sai_status_t       saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS          xpStatus  = XP_NO_ERR;
    GT_STATUS rc = GT_OK;

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(poolObjId, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }
#if 0
    xoff buffer has been reserved.

    /* Check if sufficient memory is available */
    if (bufferSize > (pPoolInfo->sharedSize - pPoolInfo->xoffSize))
    {
        XP_SAI_LOG_ERR("Error: Pool has insufficient memory, sharedSize:%d, xoffSize:%d, size:%d\n",
                       pPoolInfo->sharedSize, pPoolInfo->xoffSize, bufferSize);
        return SAI_STATUS_NO_MEMORY;
    }
#endif
    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(poolObjId);

    /* Carve the guaranteed buffer from shared buffer */
    uint32_t sharedSizeInBytes = (pPoolInfo->sharedSize - bufferSize);
    uint32_t sharedSizeInPages = XPSAI_BYTES_TO_PAGES(sharedSizeInBytes);

    /* Calculate the total guaranteed buffer size of all the queue/port/pg.
     * This is derived from substracting shared pool size (including global
     * headroom) from the total pool size */
    uint32_t totalGuarSizeInPages = XPSAI_BYTES_TO_PAGES(pPoolInfo->size -
                                                         sharedSizeInBytes);

    if (pPoolInfo->thMode == SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC)
    {
        if (pPoolInfo->type == SAI_BUFFER_POOL_TYPE_INGRESS)
        {
            if (IS_DEVICE_XP70_XP60(xpSaiSwitchDevTypeGet()))
            {
                xpStatus = xpsQosFcSetPfcTcDynamicPoolSharedSize(xpDevId, pPoolInfo->dynPoolId,
                                                                 sharedSizeInPages);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Failed to set pfc tc dynamic pool shared size, xpStatus: %d\n",
                                   xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }

                xpStatus = xpsQosFcSetPfcTcDynamicPoolGuaranteedSize(xpDevId,
                                                                     pPoolInfo->dynPoolId, totalGuarSizeInPages);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Failed to set pfc tc dynamic pool guaranteed size, poolId: %d, xpStatus:%d\n",
                                   pPoolInfo->dynPoolId, xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }
                XP_SAI_LOG_DBG("Successfully allocated guaranteed buffer from Ingress pool id %d \n",
                               pPoolInfo->dynPoolId);
            }
            else if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
            {
                /*Tail Drop*/
                rc = cpssHalPortTxTailDropConfigureAvaileblePool(xpDevId, pPoolInfo->dynPoolId,
                                                                 sharedSizeInPages, GT_TRUE);
                if (rc != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Failed to set  dynamic pool shared threshold rc %d\n",
                                   rc);
                    return cpssStatus2SaiStatus(rc);
                }

                /*PFC - configure for all tc attached to this pool*/
                saiStatus = xpSaiSetPfcAvailableBuffersOnPortTc(xpDevId, pPoolInfo->poolObjId,
                                                                sharedSizeInPages);
                if (saiStatus != SAI_STATUS_SUCCESS)
                {
                    return saiStatus;
                }
            }
        }
        else
        {
            rc = cpssHalPortTxTailDropConfigureAvaileblePool(xpDevId, pPoolInfo->dynPoolId,
                                                             sharedSizeInPages, GT_FALSE);
            if (rc != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Failed to set  dynamic pool shared threshold rc %d\n",
                               rc);
                return cpssStatus2SaiStatus(rc);
            }
            XP_SAI_LOG_DBG("Successfully allocated guaranteed buffer from Egress pool id %d \n",
                           pPoolInfo->dynPoolId);
        }
    }

    /* Update state data */
    pPoolInfo->sharedSize = sharedSizeInBytes;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolFreeGuaranteedBuffer

static sai_status_t xpSaiBufferPoolFreeGuaranteedBuffer(
    sai_object_id_t poolObjId, uint32_t bufferSize)
{
    xpSaiBufferPool_t *pPoolInfo = NULL;
    sai_status_t       saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS          xpStatus  = XP_NO_ERR;
    GT_STATUS rc = GT_OK;

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(poolObjId, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Free the guaranteed buffer and add it to shared buffer */
    sai_uint32_t sharedSizeInBytes = pPoolInfo->sharedSize + bufferSize;

    /* Consider global shared headroom and convert shared buffer from bytes to pages */
    uint32_t sharedSizeInPages = XPSAI_BYTES_TO_PAGES(sharedSizeInBytes);

    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(poolObjId);
    /* Calculate the total guaranteed buffer size of all the queue/port/pg.
    * This is derived from substracting shared pool size (including global
    * headroom) from the total pool size */
    uint32_t totalGuarSizeInPages = XPSAI_BYTES_TO_PAGES(pPoolInfo->size -
                                                         sharedSizeInBytes);

    if (pPoolInfo->thMode == SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC)
    {
        if (pPoolInfo->type == SAI_BUFFER_POOL_TYPE_INGRESS)
        {
            if (IS_DEVICE_XP70_XP60(xpSaiSwitchDevTypeGet()))
            {
                xpStatus = xpsQosFcSetPfcTcDynamicPoolSharedSize(xpDevId, pPoolInfo->dynPoolId,
                                                                 sharedSizeInPages);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Failed to set pfc tc dynamic pool shared size, xpStatus: %d\n",
                                   xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }

                xpStatus = xpsQosFcSetPfcTcDynamicPoolGuaranteedSize(xpDevId,
                                                                     pPoolInfo->dynPoolId, totalGuarSizeInPages);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Failed to set pfc tc dynamic pool guaranteed size, poolId: %d, xpStatus:%d\n",
                                   pPoolInfo->dynPoolId, xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }
                XP_SAI_LOG_DBG("Successfully freed guaranteed buffer to Ingress pool id %d \n",
                               pPoolInfo->dynPoolId);
            }
            else if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
            {
                XP_SAI_LOG_DBG("Error: Buffer pool can be associated only with egress buffer profile");
            }
        }
        else
        {
            rc = cpssHalPortTxTailDropConfigureAvaileblePool(xpDevId, pPoolInfo->dynPoolId,
                                                             sharedSizeInPages, GT_FALSE);

            if (rc != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Failed to set aqm dynamic pool shared threshold, rc: %d\n",
                               rc);
                return cpssStatus2SaiStatus(rc);
            }
            XP_SAI_LOG_DBG("Successfully freed guaranteed buffer to Egress pool id %d \n",
                           pPoolInfo->dynPoolId);
        }
    }

    /* Update state data */
    pPoolInfo->sharedSize = sharedSizeInBytes;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolAllocateDynamicPoolId

static sai_status_t xpSaiBufferPoolAllocateDynamicPoolId(xpsDevice_t devId,
                                                         sai_int32_t poolType, sai_uint32_t* pDynPoolId)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    uint16_t  xpsPoolType;

    if (poolType == SAI_BUFFER_POOL_TYPE_EGRESS)
    {
        xpsPoolType = XP_SAI_ALLOC_EGRESS_POOL;

        xpStatus = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, xpsPoolType, pDynPoolId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to allocate dynamic pool id, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }
    else
    {
        xpsPoolType = XP_SAI_ALLOC_INGRESS_POOL;

        xpStatus = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, xpsPoolType, pDynPoolId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to allocate dynamic pool id, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiBufferPoolAllocateStaticPoolId

static sai_status_t xpSaiBufferPoolAllocateStaticPoolId(xpsDevice_t devId,
                                                        sai_int32_t poolType, sai_uint32_t* pStatPoolId)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    uint16_t  xpsPoolType;

    if (poolType == SAI_BUFFER_POOL_TYPE_EGRESS)
    {
        xpsPoolType = XP_SAI_ALLOC_EGRESS_POOL;

        xpStatus = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, xpsPoolType, pStatPoolId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to allocate static pool id, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }
    else
    {
        xpsPoolType = XP_SAI_ALLOC_INGRESS_POOL;

        xpStatus = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, xpsPoolType, pStatPoolId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to allocate static pool id, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolReleaseDynamicPoolId

static sai_status_t xpSaiBufferPoolReleaseDynamicPoolId(xpsDevice_t devId,
                                                        sai_int32_t poolType, sai_uint32_t dynPoolId)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    if (poolType == SAI_BUFFER_POOL_TYPE_INGRESS)
    {
        xpStatus = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_INGRESS_POOL,
                                         dynPoolId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to allocate ingress dynamic pool id, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
        XP_SAI_LOG_DBG("Successfully allocated Ingress dynamic pool id %d \n",
                       dynPoolId);
    }
    else
    {
        xpStatus = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_EGRESS_POOL,
                                         dynPoolId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to allocate egress dynamic pool id, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
        XP_SAI_LOG_DBG("Successfully allocated Egress dynamic pool id %d \n",
                       dynPoolId);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolReleaseStaticPoolId

static sai_status_t xpSaiBufferPoolReleaseStaticPoolId(xpsDevice_t devId,
                                                       sai_int32_t poolType, sai_uint32_t statPoolId)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    if (poolType == SAI_BUFFER_POOL_TYPE_INGRESS)
    {
        xpStatus = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_INGRESS_POOL,
                                         statPoolId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to allocate ingress static pool id, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
        XP_SAI_LOG_DBG("Successfully allocated Ingress static pool id %d \n",
                       statPoolId);
    }
    else
    {
        xpStatus = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_EGRESS_POOL,
                                         statPoolId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to allocate egress static pool id, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
        XP_SAI_LOG_DBG("Successfully allocated Egress static pool id %d \n",
                       statPoolId);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolGetDynamicPoolId

sai_status_t xpSaiBufferPoolGetDynamicPoolId(sai_object_id_t poolObjId,
                                             sai_uint32_t *pDynPoolId)
{
    xpSaiBufferPool_t *pPoolInfo = NULL;
    sai_status_t       saiStatus = SAI_STATUS_SUCCESS;

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(poolObjId, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    *pDynPoolId = pPoolInfo->dynPoolId;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiBufferPoolGetDynamicPoolSharedSize(sai_object_id_t poolObjId,
                                                     sai_uint32_t *sharedSizePtr)
{
    xpSaiBufferPool_t *pPoolInfo = NULL;
    sai_status_t       saiStatus = SAI_STATUS_SUCCESS;

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(poolObjId, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    *sharedSizePtr = pPoolInfo->sharedSize;

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiBufferPoolConfigureDynamicPool

static sai_status_t xpSaiBufferPoolConfigureDynamicPool(xpsDevice_t xpDevId,
                                                        int32_t poolType,
                                                        uint32_t dynPoolId, uint32_t totalGuarSizeInBytes,
                                                        uint32_t sharedSizeInBytes, uint32_t totalSizeInBytes)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    GT_STATUS rc;

    /* Convert thresholds from bytes to pages */
    uint32_t sharedSizeInPages  = XPSAI_BYTES_TO_PAGES(sharedSizeInBytes);
    uint32_t totalSizeInPages  = XPSAI_BYTES_TO_PAGES(totalSizeInBytes);

    /*There is a same treatment for ingress or egress*/
    /* Ingress buffer pool */
    if (poolType == SAI_BUFFER_POOL_TYPE_INGRESS)
    {
        if (IS_DEVICE_XP70_XP60(xpSaiSwitchDevTypeGet()))
        {
            /* Set dynamic pool's shared buffer size for Lossless traffic */
            xpStatus = xpsQosFcSetPfcTcDynamicPoolSharedSize(xpDevId, dynPoolId,
                                                             sharedSizeInPages);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Failed to set lossless dynamic pool shared size %d, poolId: %d, xpStatus:%d\n",
                               sharedSizeInBytes, dynPoolId, xpStatus);
                return xpsStatus2SaiStatus(xpStatus);
            }

            /* Set dynamic pool's total buffer size for Lossless traffic */
            xpStatus = xpsQosFcSetPfcTcDynamicPoolTotalSize(xpDevId, dynPoolId,
                                                            totalSizeInPages);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Failed to set lossless dynamic pool total threshold %d, poolId: %d, xpStatus:%d\n",
                               totalSizeInBytes, dynPoolId, xpStatus);
                return xpsStatus2SaiStatus(xpStatus);
            }

            /*
             * Set the dynamic pool's guaranteed buffer size for lossless traffic.
             * This value is 0 at the time of buffer pool create, as guaranteed
             * buffer size is not carved yet. Update the dynamic pool's guaranteed
             * buffer size whenever buffer pool is attached/detached to/from priority group
             */
            uint32_t totalGuaranteedSizeInPages = XPSAI_BYTES_TO_PAGES(
                                                      totalGuarSizeInBytes);

            xpStatus = xpsQosFcSetPfcTcDynamicPoolGuaranteedSize(xpDevId, dynPoolId,
                                                                 totalGuaranteedSizeInPages);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Failed to set lossless dynamic pool's guaranteed size, poolId: %d, xpStatus:%d\n",
                               dynPoolId, xpStatus);
                return xpsStatus2SaiStatus(xpStatus);
            }
            XP_SAI_LOG_DBG("Successfully set ingress lossless dynamic pool's guaranteed size, poolId: %d\n",
                           dynPoolId);
        }
        else if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
        {
            /* Set the dynamic pool's shared size */
            rc = cpssHalPortTxTailDropConfigureAvaileblePool(xpDevId, dynPoolId,
                                                             sharedSizeInPages, GT_TRUE);
            if (rc != GT_OK)
            {
                XP_SAI_LOG_ERR("Failed to configure availeble pool size  |devId:%d, dynPoolId :%d, retVal %d\n",
                               xpDevId, dynPoolId, rc);
                return cpssStatus2SaiStatus(rc);
            }

            /* Set the dynamic pool's total size */
            rc = cpssHalPortTxSharedPoolLimitsSet(xpDevId, dynPoolId, totalSizeInPages,
                                                  GT_TRUE);
            if (rc != GT_OK)
            {
                XP_SAI_LOG_ERR("Error: Failed to set dynamic pool total size %d, poolId: %d, xpStatus:%d\n",
                               totalSizeInBytes, dynPoolId, xpStatus);
                return cpssStatus2SaiStatus(rc);
            }
        }
    }
    else /* Egress buffer pool */
    {
        /* Set the dynamic pool's shared size */
        rc = cpssHalPortTxTailDropConfigureAvaileblePool(xpDevId, dynPoolId,
                                                         sharedSizeInPages, GT_FALSE);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Failed to configure availeble pool size  |devId:%d, dynPoolId :%d, retVal %d\n",
                           xpDevId, dynPoolId, rc);
            return cpssStatus2SaiStatus(rc);
        }

        /* Set the dynamic pool's total size */
        rc = cpssHalPortTxSharedPoolLimitsSet(xpDevId, dynPoolId, totalSizeInPages,
                                              GT_FALSE);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Error: Failed to set dynamic pool total size %d, poolId: %d, xpStatus:%d\n",
                           totalSizeInBytes, dynPoolId, xpStatus);
            return cpssStatus2SaiStatus(rc);
        }
        XP_SAI_LOG_DBG("Successfully set egress lossless dynamic pool's total threshold, poolId: %d\n",
                       dynPoolId);
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolCreateDynamicPool

static sai_status_t xpSaiBufferPoolCreateDynamicPool(xpsDevice_t xpDevId,
                                                     int32_t poolType,
                                                     uint32_t sharedSizeInBytes, uint32_t totalSizeInBytes,
                                                     uint32_t *pDynPoolId)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    saiStatus = xpSaiBufferPoolAllocateDynamicPoolId(xpDevId, poolType, pDynPoolId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Configure the dynamic pool with shared and total size */
    saiStatus = xpSaiBufferPoolConfigureDynamicPool(xpDevId, poolType, *pDynPoolId,
                                                    0, sharedSizeInBytes, totalSizeInBytes);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        xpSaiBufferPoolReleaseDynamicPoolId(xpDevId, poolType, *pDynPoolId);
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolRemoveDynamicPool

static sai_status_t xpSaiBufferPoolRemoveDynamicPool(xpsDevice_t xpDevId,
                                                     int32_t poolType, uint32_t dynPoolId)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    /* Configure the dynamic pool */
    saiStatus = xpSaiBufferPoolConfigureDynamicPool(xpDevId, poolType, dynPoolId, 0,
                                                    FALCON_12_8_SAI_BUFFER_TOTAL_BUFFER_SIZE,
                                                    FALCON_12_8_SAI_BUFFER_TOTAL_BUFFER_SIZE);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Release dynamic pool id */
    saiStatus = xpSaiBufferPoolReleaseDynamicPoolId(xpDevId, poolType, dynPoolId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolCreateObject

static sai_status_t xpSaiBufferPoolCreateObject(xpsDevice_t devId,
                                                uint32_t poolType, sai_object_id_t *pPoolObjId)
{
    sai_uint32_t poolId    = 0;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS    xpStatus  = XP_NO_ERR;

    /* Allocate a buffer pool id */
    xpStatus = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_BUFFER_POOL,
                                      &poolId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to allocate buffer pool id, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Create buffer pool object */
    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_BUFFER_POOL, devId,
                                 (sai_uint64_t)poolId, pPoolObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not create buffer pool object, xpStatus: %d\n",
                       saiStatus);
        xpStatus = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_BUFFER_POOL,
                                         poolId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to release buffer pool id, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolRemoveObject

static sai_status_t xpSaiBufferPoolRemoveObject(sai_object_id_t poolObjId)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpSaiBufferPool_t *pPoolInfo = NULL;
    sai_status_t       saiStatus = SAI_STATUS_SUCCESS;

    sai_uint32_t poolId = (sai_uint32_t)xpSaiObjIdValueGet(poolObjId);

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(poolObjId, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Release the buffer pool id */
    xpStatus = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_BUFFER_POOL,
                                     poolId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to release buffer pool id, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolValidatePoolObject

sai_status_t xpSaiBufferPoolValidatePoolObject(sai_object_id_t poolObjId)
{
    xpSaiBufferPool_t *pPoolInfo = NULL;
    sai_status_t       saiStatus = SAI_STATUS_SUCCESS;

    /* Validate non null Objects */
    if (poolObjId != SAI_NULL_OBJECT_ID)
    {
        /* Validate the buffer pool object */
        if (!XDK_SAI_OBJID_TYPE_CHECK(poolObjId, SAI_OBJECT_TYPE_BUFFER_POOL))
        {
            XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                           xpSaiObjIdTypeGet(poolObjId));
            return SAI_STATUS_INVALID_OBJECT_TYPE;
        }

        /* Check if buffer pool exists */
        saiStatus = xpSaiBufferPoolGetStateData(poolObjId, &pPoolInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolValidateAttributeValue

static sai_status_t xpSaiBufferPoolValidateAttributeValue(sai_attr_id_t attrId,
                                                          sai_attribute_value_t attrValue, sai_uint32_t count)
{
    switch (attrId)
    {
        case SAI_BUFFER_POOL_ATTR_TYPE:
            {
                /* Validate buffer pool type */
                if (!XPSAI_BUFFER_IS_POOL_TYPE_VALID(attrValue.s32))
                {
                    XP_SAI_LOG_ERR("Error: Invalid buffer pool type value %d provided\n",
                                   attrValue.s32);
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        case SAI_BUFFER_POOL_ATTR_SIZE:
            {
                /* Validate buffer pool size  */
                if (attrValue.u32 > XPSAI_BUFFER_TOTAL_BUFFER_BYTES)
                {
                    XP_SAI_LOG_ERR("Error: Invalid buffer pool size value %d provided\n",
                                   attrValue.s32);
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        case SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE:
            {
                /* Validate buffer pool threshold mode  */
                if (!XPSAI_BUFFER_IS_POOL_THMODE_VALID(attrValue.s32))
                {
                    XP_SAI_LOG_ERR("Error: Invalid buffer pool threshold mode value %d provided\n",
                                   attrValue.s32);
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        case SAI_BUFFER_POOL_ATTR_XOFF_SIZE:
            {
                /* Validate buffer pool xoff size */
                if (attrValue.u32 > XPSAI_BUFFER_TOTAL_BUFFER_BYTES)
                {
                    XP_SAI_LOG_ERR("Error: Invalid buffer pool xoff size value %d provided\n",
                                   attrValue.s32);
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attribute %d\n", attrId);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(count);
            }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiSetPfcAvailableBuffersOnPortTc(xpsDevice_t xpDevId,
                                                        sai_object_id_t poolObjId, uint32_t poolAvailableBuffers)
{
    xpSaiBufferPool_t *pPoolInfo = NULL;
    GT_STATUS    rc = GT_OK;
    sai_uint32_t poolId = 0;
    sai_uint32_t tcBitmap = 0, tc = 0;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(poolObjId, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }
    /* update available buffers for pfc DBA.
            Available buffer pool size is shared equally to all TCs mapped to that pool.
         */

    for (tc=0; tc<8; tc++)
    {
        rc  = cpssHalPortTxTcMapToSharedPoolGet(xpDevId, tc, &poolId);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Error: Failed to get binding for tc %d rc %d\n", tc, rc);
            return cpssStatus2SaiStatus(rc);
        }
        if (poolId == pPoolInfo->dynPoolId)
        {
            tcBitmap |= (1<<tc);
        }
    }

    for (tc=0; tc<8; tc++)
    {
        if ((tcBitmap & (1<<tc)) != 0)
        {
            rc  = cpssHalPortPfcDbaTcAvailableBuffersSet(xpDevId, tc, poolAvailableBuffers);
            if (rc != GT_OK)
            {
                XP_SAI_LOG_ERR("Error: Failed to set available buffers tc %d rc %d\n", tc, rc);
                return cpssStatus2SaiStatus(rc);
            }
            XP_SAI_LOG_NOTICE("Success: Dynamic PFC set available buffers tc %d buff %d pool %d\n",
                              tc, poolAvailableBuffers, poolId);
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferProfileCreateStateData

static sai_status_t xpSaiBufferProfileCreateStateData(sai_object_id_t
                                                      profileObjId, xpSaiBufferProfile_t **ppProfileInfo)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    /* Allocate memory for state data */
    xpStatus = xpsStateHeapMalloc(sizeof(xpSaiBufferProfile_t),
                                  (void**)ppProfileInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }
    memset(*ppProfileInfo, 0x0, sizeof(xpSaiBufferProfile_t));

    /* Populate key */
    (*ppProfileInfo)->profileObjId = profileObjId;

    /* Insert the state into database */
    xpStatus = xpsStateInsertData(XP_SCOPE_DEFAULT, bufferProfileDbHandle,
                                  (void*)*ppProfileInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't insert state into data base, xpStatus: %d\n",
                       xpStatus);
        xpsStateHeapFree((void*)*ppProfileInfo);
        return xpsStatus2SaiStatus(xpStatus);
    }

    XP_SAI_LOG_DBG("Successfully inserted state into data base, for profile obj id : 0x%lx\n",
                   profileObjId);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferProfileRemoveStateData

static sai_status_t xpSaiBufferProfileRemoveStateData(sai_object_id_t
                                                      profileObjId)
{
    xpSaiBufferProfile_t *pProfileInfo = NULL;
    xpSaiBufferProfile_t *key = NULL;
    XP_STATUS             xpStatus     = XP_NO_ERR;

    key  = (xpSaiBufferProfile_t *) xpMalloc(sizeof(xpSaiBufferProfile_t));
    if (!key)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for key\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(key, 0, sizeof(xpSaiBufferProfile_t));

    /* Populate the key */
    key->profileObjId = profileObjId;

    /* Delete the corresponding state from state database */
    xpStatus = xpsStateDeleteData(XP_SCOPE_DEFAULT, bufferProfileDbHandle,
                                  (xpsDbKey_t)key, (void**)&pProfileInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't not delete buffer profile state data, xpStatus: %d\\n",
                       xpStatus);
        xpFree(key);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Free the memory allocated for the corresponding state */
    xpStatus = xpsStateHeapFree(pProfileInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't free heap memory, xpStatus: %d\n", xpStatus);
        xpFree(key);
        return xpsStatus2SaiStatus(xpStatus);
    }
    xpFree(key);
    XP_SAI_LOG_DBG("Successfully removed state from data base, for profile obj id : 0x%lx\n",
                   profileObjId);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferProfileGetStateData

sai_status_t xpSaiBufferProfileGetStateData(sai_object_id_t profileObjId,
                                            xpSaiBufferProfile_t **ppProfileInfo)
{
    xpSaiBufferProfile_t *key = NULL;
    XP_STATUS            xpStatus = XP_NO_ERR;

    key  = (xpSaiBufferProfile_t *) xpMalloc(sizeof(xpSaiBufferProfile_t));
    if (!key)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for key\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(key, 0, sizeof(xpSaiBufferProfile_t));

    /* Populate the key */
    key->profileObjId = profileObjId;

    /* Retrieve the corresponding state from state database */
    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, bufferProfileDbHandle,
                                  (xpsDbKey_t)key, (void**)ppProfileInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to retrieve the state data, xpStatus: %d\n",
                       xpStatus);
        xpFree(key);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (*ppProfileInfo == NULL)
    {
        XP_SAI_LOG_ERR("Error: Buffer profile entry does not exist\n");
        xpFree(key);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    xpFree(key);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferProfileCreateObject

static sai_status_t xpSaiBufferProfileCreateObject(xpsDevice_t xpDevId,
                                                   sai_object_id_t *pProfileObjId)
{
    sai_uint32_t  profileId = 0;
    sai_status_t  saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS     xpStatus  = XP_NO_ERR;

    /* Allocate a buffer profile id */
    xpStatus = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_BUFFER_PROFILE,
                                      &profileId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not allocate a buffer profile id, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Create buffer pool object */
    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_BUFFER_PROFILE, xpDevId,
                                 (sai_uint64_t)profileId, pProfileObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not create buffer profile object, xpStatus: %d\n",
                       saiStatus);
        xpStatus = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_BUFFER_PROFILE,
                                         profileId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to release buffer pool id, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferProfileProfielRemoveObject

sai_status_t xpSaiBufferProfileRemoveObject(sai_object_id_t profileObjId)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    /* Get the buffer profile id from object id */
    sai_uint32_t profileId = (sai_uint32_t)xpSaiObjIdValueGet(profileObjId);

    /* Release the buffer profile id */
    xpStatus = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_BUFFER_PROFILE,
                                     profileId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to release the buffer profile id, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferProfileValidateProfileObject

sai_status_t xpSaiBufferProfileValidateProfileObject(sai_object_id_t
                                                     profileObjId)
{
    xpSaiBufferProfile_t  *pProfileInfo = NULL;
    sai_status_t           saiStatus    = SAI_STATUS_SUCCESS;

    /* Validate non null Objects */
    if (profileObjId != SAI_NULL_OBJECT_ID)
    {
        /* Check if buffer profile object is valid */
        if (!XDK_SAI_OBJID_TYPE_CHECK(profileObjId, SAI_OBJECT_TYPE_BUFFER_PROFILE))
        {
            XP_SAI_LOG_ERR("Error: Invalid object type received (%u)\n",
                           xpSaiObjIdTypeGet(profileObjId));
            return SAI_STATUS_INVALID_OBJECT_TYPE;
        }

        /* Check if buffer profile object exist */
        saiStatus = xpSaiBufferProfileGetStateData(profileObjId, &pProfileInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Buffer profile does not exist! profileObjId = %" PRIu64
                           "\n", profileObjId);
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiIngressPriorityGroupCreateStateData

static sai_status_t xpSaiIngressPriorityGroupCreateStateData(
    sai_object_id_t priorityGrpId, xpSaiIngressPg_t **ppPriorityGrpInfo)
{
    XP_STATUS         xpStatus = XP_NO_ERR;
    xpSaiIngressPg_t *entry    = NULL;

    if (ppPriorityGrpInfo == NULL)
    {
        XP_SAI_LOG_ERR("Error: pPriorityGrpInfo: null pointer provided!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Allocate memory for state data */
    xpStatus = xpsStateHeapMalloc(sizeof(xpSaiIngressPg_t), (void**)&entry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    entry->priorityGroupId = priorityGrpId;

    /* Insert the state into database */
    xpStatus = xpsStateInsertData(XP_SCOPE_DEFAULT, ingressPgDbHandle,
                                  (void*) entry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't insert state into data base, xpStatus: %d\n",
                       xpStatus);
        xpsStateHeapFree(entry);
        return xpsStatus2SaiStatus(xpStatus);
    }

    *ppPriorityGrpInfo = entry;

    XP_SAI_LOG_DBG("Successfuly allocated and inserted in DB entry  with a key: %"
                   PRIu64 "\n", entry->priorityGroupId);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiIngressPriorityGroupRemoveStateData

static sai_status_t xpSaiIngressPriorityGroupRemoveStateData(
    sai_object_id_t pgObjId)
{
    XP_SAI_LOG_DBG("Removing Ingress Priority Group id: %" PRIu64 "\n", pgObjId);

    xpSaiIngressPg_t  key;
    xpSaiIngressPg_t *pPriorityGrpInfo = NULL;
    XP_STATUS         xpStatus         = XP_NO_ERR;

    memset(&key, 0, sizeof(xpSaiIngressPg_t));

    /* Populate the key */
    key.priorityGroupId = pgObjId;

    /* Delete the corresponding state from state database */
    xpStatus = xpsStateDeleteData(XP_SCOPE_DEFAULT, ingressPgDbHandle,
                                  (xpsDbKey_t)&key, (void**)&pPriorityGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't not delete buffer profile state data, xpStatus: %d\\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Free the memory allocated for the corresponding state */
    xpStatus = xpsStateHeapFree(pPriorityGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't free heap memory, xpStatus: %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    XP_SAI_LOG_DBG("Successfully inserted state into data base for ingress priority group: 0x%lx\n",
                   pgObjId);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiIngressPriorityGroupGetStateData

sai_status_t xpSaiIngressPriorityGroupGetStateData(sai_object_id_t pgObjId,
                                                   xpSaiIngressPg_t **ppPriorityGrpInfo)
{
    xpSaiIngressPg_t key;
    XP_STATUS        xpStatus = XP_NO_ERR;

    memset(&key, 0, sizeof(xpSaiIngressPg_t));

    if (ppPriorityGrpInfo == NULL)
    {
        XP_SAI_LOG_ERR("NULL ptr provided!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Populate the key */
    key.priorityGroupId = pgObjId;

    /* Retrieve the corresponding state from state database */
    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, ingressPgDbHandle,
                                  (xpsDbKey_t)&key, (void**)ppPriorityGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to retrieve the state data, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (*ppPriorityGrpInfo == NULL)
    {
        XP_SAI_LOG_ERR("Error: Ingress priority group entry does not exist: oid - %"
                       PRIu64 "\n", pgObjId);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    XP_SAI_LOG_DBG("Successfully retrieved state from data base for ingress priority group: 0x%lx\n",
                   pgObjId);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiIngressPriorityGroupGetPortAndPfcTc

void xpSaiIngressPriorityGroupGetPortAndPfcTc(sai_object_id_t pgObjId,
                                              uint32_t *port, uint32_t *pfcTc)
{
    sai_uint32_t pgKey              = 0;
    sai_uint32_t totalSupportedPrio = 0;
    xpsDevice_t  xpDevId            = xpSaiObjIdSwitchGet(pgObjId);
    XP_STATUS    xpStatus           = XP_NO_ERR;

    /* Get the total number of supported pfc traffic class */
    xpStatus = xpSaiPortGetNumPfcPrioritiesSupported(xpDevId, &totalSupportedPrio);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get the total number of supported pfc traffic class, xpStatus: %d\n",
                       xpStatus);
        return;
    }

    /* Ingress priority group number is the absolute priority group number
     * that corresponds to port and pfc traffic class
     * Eg. absolute ingrtess priority group number for port-1 and pfc traffic
     * class-2 is ((1 * totalSupportedPrio) + 2)
     * */
    if (totalSupportedPrio)
    {
        pgKey = (sai_uint32_t)xpSaiObjIdValueGet(pgObjId);

        *port  = pgKey / totalSupportedPrio;
        *pfcTc = pgKey % totalSupportedPrio;
    }
}

//Func: xpSaiIngressPriorityGroupValidateObject

sai_status_t xpSaiIngressPriorityGroupValidateObject(sai_object_id_t pgObjId)
{
    xpSaiIngressPg_t *pPriorityGrpInfo = NULL;
    sai_status_t      saiStatus        = SAI_STATUS_SUCCESS;

    /* Validate non null Objects */
    if (pgObjId != SAI_NULL_OBJECT_ID)
    {
        /* Check if ingress priority group object is valid */
        if (!XDK_SAI_OBJID_TYPE_CHECK(pgObjId, SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP))
        {
            XP_SAI_LOG_ERR("Error: Invalid object type received (%u)\n",
                           xpSaiObjIdTypeGet(pgObjId));
            return SAI_STATUS_INVALID_OBJECT_TYPE;
        }

        /* Check if buffer profile object exist */
        saiStatus = xpSaiIngressPriorityGroupGetStateData(pgObjId, &pPriorityGrpInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Ingess priority group does not exist: oid - %" PRIu64
                           "\n", pgObjId);
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiIngressPriorityGroupApplyBufferProfile

static sai_status_t xpSaiIngressPriorityGroupApplyBufferProfile(
    sai_object_id_t pgObjId, sai_object_id_t profileObjId,
    sai_object_id_t oldProfileId)
{
    xpSaiBufferProfile_t *pProfileInfo        = NULL;
    xpSaiBufferProfile_t *pOldProfileInfo     = NULL;
    sai_uint32_t          xonThInPages        = 0;
    sai_uint32_t          xoffThInPages       = 0;
    sai_uint32_t          enableDynPool       = 0;
    sai_uint32_t          enableOldDynPool    = 0;
    sai_uint32_t          dynPoolId           = 0;
    sai_uint32_t          oldDynPoolId        = 0;
    sai_uint32_t          guarHeadroomInPages = 0;
    sai_int8_t            dynamicTh           = 0;
    sai_status_t          saiStatus           = SAI_STATUS_SUCCESS;
    XP_STATUS             xpStatus            = XP_NO_ERR;
    GT_STATUS             rc;
    sai_uint32_t availableBuffers = 0;

    XP_SAI_LOG_DBG("Trying to set port/tc pfc profile id 0x%lx on PG 0x%lx\n",
                   profileObjId, pgObjId);
    if (profileObjId == SAI_NULL_OBJECT_ID)
    {
        xonThInPages   = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_DEFAULT_XON_TH);
        xoffThInPages  = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_DEFAULT_XOFF_TH);
        dynPoolId      = 0;
        enableDynPool  = 0;
        dynamicTh      = CPSS_PFC_ALPHA_MIN;
        guarHeadroomInPages = XPSAI_BYTES_TO_PAGES(4095);
    }
    else
    {
        /* Get the buffer profile state data */
        saiStatus = xpSaiBufferProfileGetStateData(profileObjId, &pProfileInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }

        if (pProfileInfo->thMode == SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC)
        {
            /* Get the dynamic pool id */
            xpSaiBufferPoolGetDynamicPoolId(pProfileInfo->poolObjId, &dynPoolId);
            enableDynPool = 1;
            dynamicTh     = CONVERT_DYN_TH_TO_CPSS_ALPHA(pProfileInfo->sharedDynamicTh);
        }
        else
        {
            /* For Static buffer profile, set alpha to min */
            dynamicTh     = CPSS_PFC_ALPHA_MIN;
        }

        /* Convert xon and xoff thresholds from bytes to pages */
        xonThInPages  = XPSAI_BYTES_TO_PAGES(pProfileInfo->xonTh);
        xoffThInPages = XPSAI_BYTES_TO_PAGES(pProfileInfo->bufferSize);
    }

    if (oldProfileId != SAI_NULL_OBJECT_ID)
    {
        /* Get the old buffer profile state data */
        saiStatus = xpSaiBufferProfileGetStateData(oldProfileId, &pOldProfileInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }

        if (pOldProfileInfo->thMode == SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC)
        {
            /* Get the dynamic pool id */
            xpSaiBufferPoolGetDynamicPoolId(pOldProfileInfo->poolObjId, &oldDynPoolId);
            enableOldDynPool = 1;
        }
    }

    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(pgObjId);
    uint32_t pfcProfileId  = 0, port = 0, pfcTc = 0;

    /* Get port/pfcTc info from ingress priority group object */
    xpSaiIngressPriorityGroupGetPortAndPfcTc(pgObjId, &port, &pfcTc);

    if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
    {
        saiStatus = xpSaiSetPfcThresholdOnPortTc(xpDevId, port, pfcTc, dynamicTh,
                                                 xoffThInPages);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }
    }
    else if (IS_DEVICE_XP70_XP60(xpSaiSwitchDevTypeGet()))
    {
        /* Set the port/tc pfc xon threshold */
        xpStatus = xpsQosFcSetPortTcPfcStaticXonThreshold(xpDevId, port, pfcTc,
                                                          xonThInPages);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to set port/tc pfc xon threshold, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Set the port/tc pfc xoff threshold */
        xpStatus = xpsQosFcSetPortTcPfcStaticXoffThreshold(xpDevId, port, pfcTc,
                                                           xoffThInPages);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to set port/tc pfc xoff threshold, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Set guaranteed headroom buffer per PG in IBuffer */
        xpStatus = xpsQosFcConfigureMaxHeadroomThreshold(xpDevId, port, pfcTc,
                                                         guarHeadroomInPages);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to configure max headroom threshold, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Set priority to lossless in IBuffer */
        xpStatus = xpsQosIbEnableLosslessPerPortPriority(xpDevId, port, pfcTc, 1);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to enable lossless per port priority in IBuffer, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Enable duynamic thresholding */
        xpStatus = xpsQosFcEnablePfcDynamicThresholding(xpDevId, port, pfcTc,
                                                        enableDynPool);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to enable pfc dynamic thresholding, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }
    else
    {
        /* Get port to port/tc profile id */
        xpStatus = xpsQosFcGetPortToPortTcPfcStaticProfile(xpDevId, port,
                                                           &pfcProfileId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to get port to port/tc pfc profile, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Set the port/tc pfc xon threshold for profile */
        xpStatus = xpsQosFcSetPortTcPfcStaticXonThresholdForProfile(xpDevId,
                                                                    pfcProfileId, pfcTc, xonThInPages);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to set port/tc pfc xon threshold, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Set the port/tc pfc xoff threshold for profile */
        xpStatus = xpsQosFcSetPortTcPfcStaticXoffThresholdForProfile(xpDevId,
                                                                     pfcProfileId, pfcTc, xoffThInPages);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to set port/tc pfc xoff threshold, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    /* Check if Buffer Pool is or was Dynamic Pool */
    if (enableDynPool || enableOldDynPool)
    {
        /* Check for already applied binding with the same dynamicPool */
        if (pfcTcActivePgCount[pfcTc] > 0 && enableDynPool && enableOldDynPool)
        {
            sai_uint32_t  pfcTcDynPoolId = 0;

            rc  = cpssHalPortTxTcMapToSharedPoolGet(xpDevId, pfcTc, &pfcTcDynPoolId);
            if (rc != GT_OK)
            {
                XP_SAI_LOG_ERR("Error: Failed to get binding for   tc %d  rc %d\n", rc, pfcTc,
                               oldDynPoolId);
                return cpssStatus2SaiStatus(rc);
            }

            if (pfcTcDynPoolId != dynPoolId)
            {
                XP_SAI_LOG_ERR("Error: Cannot bind Pfc Tc %d to pool %d already bound to dynamic pool %d \n",
                               pfcTc, dynPoolId, pfcTcDynPoolId);
                return SAI_STATUS_FAILURE;
            }

            XP_SAI_LOG_DBG("Not binding pfc/tc %d to dynamic pool %d as tc is already bound!\n",
                           pfcTc, dynPoolId);

            ++pfcTcActivePgCount[pfcTc];
            XP_SAI_LOG_NOTICE("Incrementing ref counter of binding pfcTc %d and dynamic pool %d.\n",
                              pfcTc, dynPoolId);
        }

        if (enableOldDynPool && !enableDynPool && pfcTcActivePgCount[pfcTc] > 0)
        {
            if (pfcTcActivePgCount[pfcTc] == 1)
            {
                /* Before bind unbind the pfc/tc from dynamic pool */
                xpStatus = xpsQosFcUnbindPfcTcFromDynamicPool(xpDevId, pfcTc, oldDynPoolId);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Failed to unbind pfc/tc from dynamic pool, xpStatus: %d\n",
                                   xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }
                XP_SAI_LOG_DBG("Unbinding pfc/tc %d to dynamic pool %d as profile is already bound!\n",
                               pfcTc, dynPoolId);
            }

            --pfcTcActivePgCount[pfcTc];
            XP_SAI_LOG_NOTICE("Decrementing ref counter of binding pfcTc %d and dynamic pool %d.\n",
                              pfcTc, dynPoolId);
        }

        if (enableDynPool && !enableOldDynPool)
        {
            if (pfcTcActivePgCount[pfcTc] == 0)
            {
                /* Bind the pfc tc to dynamic pool */
                rc  = cpssHalPortTxTcMapToSharedPoolSet(xpDevId, pfcTc, dynPoolId);
                if (rc != GT_OK)
                {
                    XP_SAI_LOG_ERR("Error: Failed to bind  tc %d to pool %d rc %d\n", rc, pfcTc,
                                   oldDynPoolId);
                    return cpssStatus2SaiStatus(rc);
                }
                XP_SAI_LOG_DBG("Successfully bound pfc/tc %d to dynamic pool %d\n", pfcTc,
                               dynPoolId);
            }

            ++pfcTcActivePgCount[pfcTc];
            XP_SAI_LOG_NOTICE("Incrementing ref counter of binding pfcTc %d and dynamic pool %d.\n",
                              pfcTc, dynPoolId);
        }

        if ((pProfileInfo != NULL) && (pProfileInfo->poolObjId!=SAI_NULL_OBJECT_ID))
        {
            xpSaiBufferPoolGetDynamicPoolSharedSize(pProfileInfo->poolObjId,
                                                    &availableBuffers);

            availableBuffers=XPSAI_BYTES_TO_PAGES(availableBuffers);

            /*For other TCs attached to this buffer availeble size was configured at xpSaiBufferPoolAllocateGuaranteedBuffer*/
            rc  = cpssHalPortPfcDbaTcAvailableBuffersSet(xpDevId, pfcTc, availableBuffers);
            if (rc != GT_OK)
            {
                XP_SAI_LOG_ERR("Error: Failed to set available buffers tc %d rc %d\n", pfcTc,
                               rc);
                return cpssStatus2SaiStatus(rc);
            }

            XP_SAI_LOG_NOTICE("Success: Dynamic PFC set available buffers tc %d buff %d pool %d \n",
                              pfcTc, availableBuffers, dynPoolId);
        }

    }
    else
    {
        XP_SAI_LOG_DBG("Buffer Profile is bound to non dynamic pool. Ingress Priority Group bind has no effect! \n",
                       pfcTc, dynPoolId);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferGetPacketMemorySize

sai_uint32_t xpSaiBufferGetPacketMemorySize()
{
    XP_DEV_TYPE_T deviceType;
    sai_uint32_t bufferSize = 0;

    /* Get the device type */
    deviceType = xpSaiSwitchDevTypeGet();

    if ((deviceType == XP80) || (deviceType == XP80B0))
    {
        /* Packet memory: 96K of 256B Pages */
        bufferSize = XP80SAI_BUFFER_TOTAL_BUFFER_SIZE;
    }
    else if ((deviceType == XP60) || (deviceType == XP70) ||
             (deviceType == XP70A1) || (deviceType == XP70A2))
    {
        /* Packet memory: 16K of 256B Pages */
        bufferSize = XP70SAI_BUFFER_TOTAL_BUFFER_SIZE;
    }
    else if (IS_DEVICE_FALCON_12_8(deviceType))
    {
        /* Packet memory: 48MB */
        bufferSize = FALCON_12_8_SAI_BUFFER_TOTAL_BUFFER_SIZE;
    }
    else if (IS_DEVICE_FALCON_6_4(deviceType))
    {
        /* Packet memory: 24MB */
        bufferSize = FALCON_6_4_SAI_BUFFER_TOTAL_BUFFER_SIZE;
    }
    else if (IS_DEVICE_FALCON_3_2(deviceType))
    {
        /* Packet memory: 12MB */
        bufferSize = FALCON_3_2_SAI_BUFFER_TOTAL_BUFFER_SIZE;
    }
    else if (IS_DEVICE_FALCON_2(deviceType))
    {
        /* Packet memory: 24MB */
        bufferSize = FALCON_2_SAI_BUFFER_TOTAL_BUFFER_SIZE;
    }
    else if (IS_DEVICE_AC5X(deviceType))
    {
        /* Packet memory: 3MB */
        bufferSize = AC5X_SAI_BUFFER_TOTAL_BUFFER_SIZE;
    }

    return bufferSize;
}

//Func: xpSaiBufferBindProfileToPool

sai_status_t xpSaiBufferBindProfileToPool(sai_object_id_t poolObjId,
                                          sai_object_id_t profileObjId)
{
    xpSaiBufferPool_t *pPoolInfo = NULL;
    sai_status_t       saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS          xpStatus  = XP_NO_ERR;

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(poolObjId, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    if (xpsDAIsCtxGrowthNeeded(pPoolInfo->profileCount, XPSAI_BUFFER_MAP_LIST_SIZE))
    {
        xpSaiBufferPool_t *pNewPoolInfo = NULL;
        xpSaiBufferPool_t  *key = NULL;

        key = (xpSaiBufferPool_t *)xpMalloc(sizeof(xpSaiBufferPool_t));
        if (!key)
        {
            XP_SAI_LOG_ERR("Error: allocation failed for key\n");
            return XP_ERR_MEM_ALLOC_ERROR;
        }
        memset(key, 0, sizeof(xpSaiBufferPool_t));

        /* Double the profileList array size */
        xpStatus = xpSaiDynamicArrayGrow((void**)&pNewPoolInfo, (void*)pPoolInfo,
                                         sizeof(xpSaiBufferPool_t), sizeof(sai_object_id_t), pPoolInfo->profileCount,
                                         XPSAI_BUFFER_MAP_LIST_SIZE);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Dynamic array grow failed, xpStatus: %d\n", xpStatus);
            xpFree(key);
            return xpsStatus2SaiStatus(xpStatus);
        }

        key->poolObjId = poolObjId;

        /* Remove the corresponding state from buffer pool state data and add new state */
        xpStatus = xpsStateDeleteData(XP_SCOPE_DEFAULT, bufferPoolDbHandle,
                                      (xpsDbKey_t)key, (void**)&pPoolInfo);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Deletion of data failed, xpStatus: %d\n", xpStatus);
            xpsStateHeapFree((void*)pNewPoolInfo);
            xpFree(key);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Free the memory allocated for the corresponding state */
        xpStatus = xpsStateHeapFree(pPoolInfo);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Freeing allocated memory failed, xpStatus: %d\n",
                           xpStatus);
            xpsStateHeapFree((void*)pNewPoolInfo);
            xpFree(key);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Insert the new state */
        xpStatus = xpsStateInsertData(XP_SCOPE_DEFAULT, bufferPoolDbHandle,
                                      (void*)pNewPoolInfo);
        if (xpStatus != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)pNewPoolInfo);
            xpFree(key);
            return xpsStatus2SaiStatus(xpStatus);
        }
        xpFree(key);
        pPoolInfo = pNewPoolInfo;
    }

    pPoolInfo->profileList[pPoolInfo->profileCount] = profileObjId;
    pPoolInfo->profileCount++;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferUnbindProfileFromPool

sai_status_t xpSaiBufferUnbindProfileFromPool(sai_object_id_t poolObjId,
                                              sai_object_id_t profileObjId)
{
    xpSaiBufferPool_t  *pPoolInfo    = NULL;
    sai_uint32_t        profileExist = 0;
    sai_status_t        saiStatus    = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(poolObjId, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Remove the buffer profile from buffer pool's profileList */
    for (uint32_t count = 0; count < pPoolInfo->profileCount; count++)
    {
        if (!profileExist)
        {
            if (pPoolInfo->profileList[count] == profileObjId)
            {
                profileExist = 1;
            }
            continue;
        }
        else
        {
            pPoolInfo->profileList[count-1] = pPoolInfo->profileList[count];
        }
    }

    if (!profileExist)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Decrement the profile object count */
    pPoolInfo->profileCount--;

    if (xpsDAIsCtxShrinkNeeded(pPoolInfo->profileCount, XPSAI_BUFFER_MAP_LIST_SIZE))
    {
        xpSaiBufferPool_t *pNewPoolInfo = NULL;
        xpSaiBufferPool_t  *key = NULL;

        key = (xpSaiBufferPool_t *)xpMalloc(sizeof(xpSaiBufferPool_t));
        if (!key)
        {
            XP_SAI_LOG_ERR("Error: allocation failed for key\n");
            return XP_ERR_MEM_ALLOC_ERROR;
        }
        memset(key, 0, sizeof(xpSaiBufferPool_t));

        /* Shrink the profileList array size */
        xpStatus = xpSaiDynamicArrayShrink((void**)&pNewPoolInfo, (void*)pPoolInfo,
                                           sizeof(xpSaiBufferPool_t), sizeof(sai_object_id_t), pPoolInfo->profileCount,
                                           XPSAI_BUFFER_MAP_LIST_SIZE);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Dynamic array grow failed, xpStatus: %d\n", xpStatus);
            xpFree(key);
            return xpsStatus2SaiStatus(xpStatus);
        }

        key->poolObjId = poolObjId;

        /* Remove the corresponding state from buffer profile db and add new state */
        xpStatus = xpsStateDeleteData(XP_SCOPE_DEFAULT, bufferPoolDbHandle,
                                      (xpsDbKey_t)key, (void**)&pPoolInfo);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Deletion of data failed, xpStatus: %d\n", xpStatus);
            xpsStateHeapFree((void*)pNewPoolInfo);
            xpFree(key);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Free the memory allocated for the corresponding state */
        xpStatus = xpsStateHeapFree(pPoolInfo);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Freeing allocated memory failed, xpStatus: %d\n",
                           xpStatus);
            xpsStateHeapFree((void*)pNewPoolInfo);
            xpFree(key);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Insert the new state */
        xpStatus = xpsStateInsertData(XP_SCOPE_DEFAULT, bufferPoolDbHandle,
                                      (void*)pNewPoolInfo);
        if (xpStatus != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)pNewPoolInfo);
            xpFree(key);
            return xpsStatus2SaiStatus(xpStatus);
        }

        xpFree(key);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferGetPoolIngressPgObjCount

sai_status_t xpSaiBufferGetPoolIngressPgCount(sai_object_id_t poolObjId,
                                              sai_uint32_t *pPgCount)
{
    xpSaiBufferProfile_t  *pProfileInfo = NULL;
    xpSaiBufferPool_t     *pPoolInfo    = NULL;
    sai_status_t           saiStatus    = SAI_STATUS_SUCCESS;

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(poolObjId, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    *pPgCount = 0;
    if (pPoolInfo->type == SAI_BUFFER_POOL_TYPE_EGRESS)
    {
        return SAI_STATUS_SUCCESS;
    }

    /* Loop through profiles attached to the pool */
    for (uint32_t i = 0; i < pPoolInfo->profileCount; i++)
    {
        /* Get the buffer profile state data */
        saiStatus = xpSaiBufferProfileGetStateData(pPoolInfo->profileList[i],
                                                   &pProfileInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }

        /* Loop through ingressPgs/Queue attached to the profile */
        for (uint32_t j = 0; j < pProfileInfo->objCount; j++)
        {
            if (XDK_SAI_OBJID_TYPE_CHECK(pProfileInfo->objList[j],
                                         SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP))
            {
                *pPgCount += 1;
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferGetPoolIngressPgObjList

sai_status_t xpSaiBufferGetPoolIngressPgList(sai_object_id_t poolObjId,
                                             sai_object_list_t *pPgObjList)
{
    xpSaiBufferProfile_t  *pProfileInfo = NULL;
    xpSaiBufferPool_t     *pPoolInfo    = NULL;
    sai_status_t           saiStatus    = SAI_STATUS_SUCCESS;

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(poolObjId, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    pPgObjList->count = 0;

    if (pPoolInfo->type == SAI_BUFFER_POOL_TYPE_EGRESS)
    {
        return SAI_STATUS_SUCCESS;
    }

    /* Loop through profiles attached to the pool */
    for (uint32_t i = 0; i < pPoolInfo->profileCount; i++)
    {
        /* Get the buffer profile state data */
        saiStatus = xpSaiBufferProfileGetStateData(pPoolInfo->profileList[i],
                                                   &pProfileInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }

        /* Loop through ingressPgs/Queue attached to the profile */
        for (uint32_t j = 0; j < pProfileInfo->objCount; j++)
        {
            if (XDK_SAI_OBJID_TYPE_CHECK(pProfileInfo->objList[j],
                                         SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP))
            {
                pPgObjList->list[pPgObjList->count] = pProfileInfo->objList[j];
                pPgObjList->count += 1;
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferAddObjIdToBufferProfile

sai_status_t xpSaiBufferAddObjIdToBufferProfile(sai_object_id_t profileObjId,
                                                sai_object_id_t mapObjId)
{
    xpSaiBufferProfile_t  *pProfileInfo = NULL;
    sai_status_t           saiStatus    = SAI_STATUS_SUCCESS;
    xpSaiBufferPool_t     *pPoolInfo    = NULL;
    XP_STATUS              xpStatus     = XP_NO_ERR;
    sai_uint32_t           guarBuffer   = 0;
    sai_object_type_t      objType;

    /* Get the buffer profile state data */
    saiStatus = xpSaiBufferProfileGetStateData(profileObjId, &pProfileInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(pProfileInfo->poolObjId, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* valid value */
    objType = xpSaiObjIdTypeGet(mapObjId);
    if (objType == SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP &&
        pPoolInfo->type != SAI_BUFFER_POOL_TYPE_INGRESS)
    {
        XP_SAI_LOG_ERR("ingress PG only can be bound to ingress pool \n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (objType == SAI_OBJECT_TYPE_QUEUE &&
        pPoolInfo->type != SAI_BUFFER_POOL_TYPE_EGRESS)
    {
        XP_SAI_LOG_ERR("queue only can be bound to egress pool \n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (objType == SAI_OBJECT_TYPE_PORT &&
        pPoolInfo->type != SAI_BUFFER_POOL_TYPE_EGRESS)
    {
        XP_SAI_LOG_ERR("currently, port only can be bound to egress pool \n");
        return SAI_STATUS_NOT_SUPPORTED;
    }

    guarBuffer = pProfileInfo->bufferSize;

    /* when pool xoff == 0, the xoff buffer of profile should be reserved. */
    if (pPoolInfo->xoffSize == 0)
    {
        guarBuffer += pProfileInfo->xoffTh;
    }

    /* is buffer resource sufficient */
    if (pPoolInfo->sharedSize < guarBuffer)
    {
        XP_SAI_LOG_ERR("buffer is not sufficient, pool cur share size %u, need reserve size %u \n",
                       pPoolInfo->sharedSize, guarBuffer);
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    /* Allocate new memory memory from buffer pool */
    saiStatus = xpSaiBufferPoolAllocateGuaranteedBuffer(pProfileInfo->poolObjId,
                                                        guarBuffer);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to allocate buffer pool memory, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    if (xpsDAIsCtxGrowthNeeded(pProfileInfo->objCount, XPSAI_BUFFER_MAP_LIST_SIZE))
    {
        xpSaiBufferProfile_t *pNewProfileInfo = NULL;
        xpSaiBufferProfile_t  key;

        /* Double the queueList array size */
        xpStatus = xpSaiDynamicArrayGrow((void**)&pNewProfileInfo, (void*)pProfileInfo,
                                         sizeof(xpSaiBufferProfile_t), sizeof(sai_object_id_t), pProfileInfo->objCount,
                                         XPSAI_BUFFER_MAP_LIST_SIZE);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Dynamic array grow failed, xpStatus: %d\n", xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

        key.profileObjId = profileObjId;

        /* Remove the corresponding state from buffer profile db and add new state */
        xpStatus = xpsStateDeleteData(XP_SCOPE_DEFAULT, bufferProfileDbHandle,
                                      (xpsDbKey_t)&key, (void**)&pProfileInfo);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Deletion of data failed, xpStatus: %d\n", xpStatus);
            xpsStateHeapFree((void*)pProfileInfo);
            xpsStateHeapFree((void*)pNewProfileInfo);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Free the memory allocated for the corresponding state */
        xpStatus = xpsStateHeapFree(pProfileInfo);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Freeing allocated memory failed, xpStatus: %d\n",
                           xpStatus);
            xpsStateHeapFree((void*)pNewProfileInfo);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Insert the new state */
        xpStatus = xpsStateInsertData(XP_SCOPE_DEFAULT, bufferProfileDbHandle,
                                      (void*)pNewProfileInfo);
        if (xpStatus != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)pNewProfileInfo);
            return xpsStatus2SaiStatus(xpStatus);
        }

        pProfileInfo = pNewProfileInfo;
    }

    pProfileInfo->objList[pProfileInfo->objCount] = mapObjId;
    pProfileInfo->objCount++;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferRemoveObjIdFromBufferProfile

sai_status_t xpSaiBufferRemoveObjIdFromBufferProfile(sai_object_id_t
                                                     profileObjId, sai_object_id_t mapObjId)
{
    xpSaiBufferProfile_t  *pProfileInfo = NULL;
    xpSaiBufferPool_t     *pPoolInfo    = NULL;
    sai_status_t           saiStatus    = SAI_STATUS_SUCCESS;
    sai_uint8_t            mapObjExist  = 0;
    XP_STATUS              xpStatus     = XP_NO_ERR;
    sai_uint32_t           guarBuffer   = 0;

    /* Get the buffer profile state data */
    saiStatus = xpSaiBufferProfileGetStateData(profileObjId, &pProfileInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Remove the mapObjId from buffer profile objList */
    for (uint16_t count = 0; count < pProfileInfo->objCount; count++)
    {
        if (!mapObjExist)
        {
            if (pProfileInfo->objList[count] == mapObjId)
            {
                mapObjExist = 1;
            }
            continue;
        }
        else
        {
            pProfileInfo->objList[count-1] = pProfileInfo->objList[count];
        }
    }

    if (!mapObjExist)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(pProfileInfo->poolObjId, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    guarBuffer = pProfileInfo->bufferSize;

    if (pPoolInfo->xoffSize == 0)
    {
        guarBuffer += pProfileInfo->xoffTh;
    }

    /* Free the previously allocated memory */
    saiStatus = xpSaiBufferPoolFreeGuaranteedBuffer(pProfileInfo->poolObjId,
                                                    guarBuffer);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to release buffer pool memory, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Decrement the object count */
    pProfileInfo->objCount--;

    if (xpsDAIsCtxShrinkNeeded(pProfileInfo->objCount, XPSAI_BUFFER_MAP_LIST_SIZE))
    {
        xpSaiBufferProfile_t *pNewProfileInfo = NULL;
        xpSaiBufferProfile_t  key;

        /* Shrink the objList array size */
        xpStatus = xpSaiDynamicArrayShrink((void**)&pNewProfileInfo,
                                           (void*)pProfileInfo, sizeof(xpSaiBufferProfile_t), sizeof(sai_object_id_t),
                                           pProfileInfo->objCount, XPSAI_BUFFER_MAP_LIST_SIZE);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Dynamic array grow failed, xpStatus: %d\n", xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

        key.profileObjId = profileObjId;
        /* Remove the corresponding state from buffer profile db and add new state */
        xpStatus = xpsStateDeleteData(XP_SCOPE_DEFAULT, bufferProfileDbHandle,
                                      (xpsDbKey_t)&key, (void**)&pProfileInfo);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Deletion of data failed, xpStatus: %d\n", xpStatus);
            xpsStateHeapFree((void*)pNewProfileInfo);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Free the memory allocated for the corresponding state */
        xpStatus = xpsStateHeapFree(pProfileInfo);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Freeing allocated memory failed, xpStatus: %d\n",
                           xpStatus);
            xpsStateHeapFree((void*)pNewProfileInfo);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Insert the new state */
        xpStatus = xpsStateInsertData(XP_SCOPE_DEFAULT, bufferProfileDbHandle,
                                      (void*)pNewProfileInfo);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Insertion of state data failed, xpStatus: %d\n",
                           xpStatus);
            xpsStateHeapFree((void*)pNewProfileInfo);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferAddPortToBufferProfile

sai_status_t xpSaiBufferAddPortToBufferProfile(sai_object_id_t profileObjId,
                                               sai_object_id_t portObjId)
{
    XP_SAI_LOG_DBG("Add port object id 0x%lx to buffer profile 0x%lx \n", portObjId,
                   profileObjId);
    return xpSaiBufferAddObjIdToBufferProfile(profileObjId, portObjId);
}

//Func: xpSaiBufferRemovePortFromBufferProfile

sai_status_t xpSaiBufferRemovePortFromBufferProfile(sai_object_id_t
                                                    profileObjId, sai_object_id_t portObjId)
{
    XP_SAI_LOG_DBG("Remove port object id 0x%lx to buffer profile 0x%lx \n",
                   portObjId, profileObjId);
    return xpSaiBufferRemoveObjIdFromBufferProfile(profileObjId, portObjId);
}

//Func: xpSaiBufferAddIngressPgToBufferProfile

sai_status_t xpSaiBufferAddIngressPgToBufferProfile(sai_object_id_t
                                                    profileObjId, sai_object_id_t pgObjId)
{
    XP_SAI_LOG_DBG("Add pg object id 0x%lx to buffer profile 0x%lx \n", pgObjId,
                   profileObjId);
    return xpSaiBufferAddObjIdToBufferProfile(profileObjId, pgObjId);
}

//Func: xpSaiBufferRemoveIngressPgFromBufferProfile

sai_status_t xpSaiBufferRemoveIngressPgFromBufferProfile(
    sai_object_id_t profileObjId, sai_object_id_t pgObjId)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    sai_uint32_t port = 0;
    sai_uint32_t pfcTc = 0;
    XP_SAI_LOG_DBG("Remove pg object id 0x%lx to buffer profile 0x%lx \n", pgObjId,
                   profileObjId);
    saiStatus =  xpSaiBufferRemoveObjIdFromBufferProfile(profileObjId, pgObjId);

    if (saiStatus == SAI_STATUS_SUCCESS)
    {
        /* Get port/pfcTc info from ingress priority group object */
        xpSaiIngressPriorityGroupGetPortAndPfcTc(pgObjId, &port, &pfcTc);
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferAddQueueToBufferProfile

sai_status_t xpSaiBufferAddQueueToBufferProfile(sai_object_id_t profileObjId,
                                                sai_object_id_t queueObjId)
{
    XP_SAI_LOG_DBG("Add queue object id 0x%lx to buffer profile 0x%lx \n",
                   queueObjId, profileObjId);
    return xpSaiBufferAddObjIdToBufferProfile(profileObjId, queueObjId);
}

//Func: xpSaiBufferRemoveQueueFromBufferProfile

sai_status_t xpSaiBufferRemoveQueueFromBufferProfile(sai_object_id_t
                                                     profileObjId, sai_object_id_t queueObjId)
{
    XP_SAI_LOG_DBG("Remove queue object id 0x%lx to buffer profile 0x%lx \n",
                   queueObjId, profileObjId);
    return xpSaiBufferRemoveObjIdFromBufferProfile(profileObjId, queueObjId);
}

//Func: xpSaiSetDefaultBufferProfileAttributeVals

void xpSaiSetDefaultBufferProfileAttributeVals(xpSaiBufferProfileAttributesT*
                                               attributes)
{
    memset(attributes, 0x0, sizeof(xpSaiBufferProfileAttributesT));

#if 0 // SAI 1.2
    attributes->thMode.s32      =
        SAI_BUFFER_PROFILE_THRESHOLD_MODE_INHERIT_BUFFER_POOL_MODE;
#endif
    attributes->xoffTh.u32      = 0;
    attributes->xonTh.u32       = 0;
    attributes->xonOffsetTh.u32 = 0;
}

//Func: xpSaiUpdateBufferProfileAttributeVals

void xpSaiUpdateBufferProfileAttributeVals(const uint32_t attr_count,
                                           const sai_attribute_t* attr_list, xpSaiBufferProfileAttributesT* attributes)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_BUFFER_PROFILE_ATTR_POOL_ID:
                {
                    attributes->poolId = attr_list[count].value;
                    break;
                }
            case SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE:
                {
                    attributes->bufferSize = attr_list[count].value;
                    break;
                }
            case SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE:
                {
                    attributes->thMode = attr_list[count].value;
                    break;
                }
            case SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH:
                {
                    attributes->sharedDynamicTh = attr_list[count].value;
                    break;
                }
            case SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH:
                {
                    attributes->sharedStaticTh = attr_list[count].value;
                    break;
                }
            case SAI_BUFFER_PROFILE_ATTR_XOFF_TH:
                {
                    attributes->xoffTh = attr_list[count].value;
                    break;
                }
            case SAI_BUFFER_PROFILE_ATTR_XON_TH:
                {
                    attributes->xonTh = attr_list[count].value;
                    break;
                }
            case SAI_BUFFER_PROFILE_ATTR_XON_OFFSET_TH:
                {
                    attributes->xonOffsetTh = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Unknown attribute %d received\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiUpdateAttrListBufferProfileVals

void xpSaiUpdateAttrListBufferProfileVals(const xpSaiBufferProfileAttributesT*
                                          attributes, const uint32_t attr_count, sai_attribute_t* attr_list)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_BUFFER_PROFILE_ATTR_POOL_ID:
                {
                    attr_list[count].value = attributes->poolId;
                    break;
                }
            case SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE:
                {
                    attr_list[count].value = attributes->bufferSize;
                    break;
                }
            case SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE:
                {
                    attr_list[count].value = attributes->thMode;
                    break;
                }
            case SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH:
                {
                    attr_list[count].value = attributes->sharedDynamicTh;
                    break;
                }
            case SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH:
                {
                    attr_list[count].value = attributes->sharedStaticTh;
                    break;
                }
            case SAI_BUFFER_PROFILE_ATTR_XOFF_TH:
                {
                    attr_list[count].value = attributes->xoffTh;
                    break;
                }
            case SAI_BUFFER_PROFILE_ATTR_XON_TH:
                {
                    attr_list[count].value = attributes->xonTh;
                    break;
                }
            case SAI_BUFFER_PROFILE_ATTR_XON_OFFSET_TH:
                {
                    attr_list[count].value = attributes->xonOffsetTh;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Unknown attribute %d received\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiSetBufferProfileAttrBufferSize

sai_status_t xpSaiSetBufferProfileAttrBufferSize(sai_object_id_t profileObjId,
                                                 sai_attribute_value_t value)
{
    xpSaiBufferProfile_t  *pProfileInfo = NULL;
    xpSaiBufferPool_t     *pPoolInfo    = NULL;
    sai_status_t           saiStatus    = SAI_STATUS_SUCCESS;
    XP_STATUS              xpStatus     = XP_NO_ERR;
    sai_uint32_t           guarBuffer   = 0;
    sai_uint32_t           newGuarBuffer   = 0;
    sai_uint32_t           rollbackValue   = 0;
    GT_BOOL                isIngressPool   = GT_FALSE;

    /* Get the buffer profile state data */
    saiStatus = xpSaiBufferProfileGetStateData(profileObjId, &pProfileInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    xpsDevice_t  xpDevId  = xpSaiObjIdSwitchGet(profileObjId);
    sai_uint32_t portNum  = 0;

    if (pProfileInfo->poolObjId != SAI_NULL_OBJECT_ID)
    {
        /* Get the buffer pool state data */
        saiStatus = xpSaiBufferPoolGetStateData(pProfileInfo->poolObjId, &pPoolInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }
        isIngressPool = (pPoolInfo->type == SAI_BUFFER_POOL_TYPE_INGRESS)? GT_TRUE :
                        GT_FALSE;
    }

    /* Convert reserved buffer from bytes to pages */
    sai_uint32_t bufferSizeInPages = XPSAI_BYTES_TO_PAGES(value.u32);
    sai_object_type_t objType;

    guarBuffer = pProfileInfo->bufferSize;
    newGuarBuffer = value.u32;

    /* when pool xoff == 0, the xoff buffers of profile were reserved.
     * Otherwise, the share xoff buffers of pool were reserved
     */
    if (pPoolInfo && pPoolInfo->xoffSize == 0)
    {
        guarBuffer += pProfileInfo->xoffTh;
        newGuarBuffer += pProfileInfo->xoffTh;
    }

    /* update the share size */
    if ((pProfileInfo->poolObjId != SAI_NULL_OBJECT_ID)
        && (guarBuffer != newGuarBuffer))
    {
        guarBuffer    *= pProfileInfo->objCount;
        newGuarBuffer *= pProfileInfo->objCount;

        /* is buffer resource sufficient */
        if (pPoolInfo->sharedSize + guarBuffer < newGuarBuffer)
        {
            XP_SAI_LOG_ERR("pool cur share %u, old guar %u, new guar %u \n",
                           pPoolInfo->sharedSize, guarBuffer, newGuarBuffer);
            return SAI_STATUS_INSUFFICIENT_RESOURCES;
        }

        /* Free the existing buffer pool memory */
        saiStatus = xpSaiBufferPoolFreeGuaranteedBuffer(pProfileInfo->poolObjId,
                                                        guarBuffer);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to free the guaranteed buffer, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }

        /* Allocate new memory from buffer pool */
        saiStatus = xpSaiBufferPoolAllocateGuaranteedBuffer(pProfileInfo->poolObjId,
                                                            newGuarBuffer);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to allocate guaranteed buffer from buffer pool, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* set object to HW */
    rollbackValue = pProfileInfo->bufferSize;
    pProfileInfo->bufferSize = value.u32;

    for (uint32_t count = 0; count < pProfileInfo->objCount; count++)
    {
        objType = xpSaiObjIdTypeGet(pProfileInfo->objList[count]);

        switch (objType)
        {
            case SAI_OBJECT_TYPE_PORT:
                {
                    /* Configure buffer profile on port to HW */
                    /* (the above has allocated new share size, here should only configure the guarante size to HW ) */
                    saiStatus = xpSaiPortApplyBufferProfileOnPort(pProfileInfo->objList[count],
                                                                  profileObjId, isIngressPool);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to apply buffer profile on port, saiStatus: %d\n",
                                       saiStatus);
                        pProfileInfo->bufferSize = rollbackValue;
                        return saiStatus;
                    }
                    break;
                }
            case SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP:
                {
                    sai_uint32_t pfcTc = 0, pfcProfileId = 0;
                    sai_int8_t   dynamicTh = 0;

                    xpSaiIngressPriorityGroupGetPortAndPfcTc(pProfileInfo->objList[count], &portNum,
                                                             &pfcTc);

                    if (pProfileInfo->thMode == SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC)
                    {
                        dynamicTh      = CONVERT_DYN_TH_TO_CPSS_ALPHA(pProfileInfo->sharedDynamicTh);
                    }
                    else
                    {
                        /* For Static buffer profile, set alpha to min */
                        dynamicTh      = CPSS_PFC_ALPHA_MIN;
                    }

                    if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
                    {
                        saiStatus = xpSaiSetPfcThresholdOnPortTc(xpDevId, portNum, pfcTc,
                                                                 dynamicTh,
                                                                 (XPSAI_BYTES_TO_PAGES(value.u32)));
                        if (saiStatus != SAI_STATUS_SUCCESS)
                        {
                            pProfileInfo->bufferSize = rollbackValue;
                            XP_SAI_LOG_ERR("Error: Failed to set port/tc pfc xoff threshold, saiStatus: %d\n",
                                           saiStatus);
                            return saiStatus;
                        }
                    }
                    else if (IS_DEVICE_XP70_XP60(xpSaiSwitchDevTypeGet()))
                    {
                        /* Set the port/tc pfc xoff threshold */
                        xpStatus = xpsQosFcSetPortTcPfcStaticXoffThreshold(xpDevId, portNum, pfcTc,
                                                                           bufferSizeInPages);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set port/tc pfc xoff threshold, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }
                        XP_SAI_LOG_DBG("Successfully set port/tc pfc xoff threshold, port %d pfc tc %d, size %d \n",
                                       portNum, pfcTc, bufferSizeInPages);
                    }
                    else
                    {
                        /* Get port/tc pfc profile id */
                        xpStatus = xpsQosFcGetPortToPortTcPfcStaticProfile(xpDevId, portNum,
                                                                           &pfcProfileId);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to get port/tc pfc profile, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }

                        /* Set the port/tc pfc xoff threshold for profile */
                        xpStatus = xpsQosFcSetPortTcPfcStaticXoffThresholdForProfile(xpDevId,
                                                                                     pfcProfileId, pfcTc, bufferSizeInPages);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set port/tc pfc xoff threshold, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }
                    }
                    break;
                }
            case SAI_OBJECT_TYPE_QUEUE:
                {
                    /* Configure buffer profile on queue */
                    /* (the above has allocated new share size, here should only configure the guarante size to HW ) */
                    saiStatus = xpSaiQueueApplyBufferProfile(pProfileInfo->objList[count],
                                                             profileObjId);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to configure buffer profile on queue, saiStatus: %d\n",
                                       saiStatus);
                        pProfileInfo->bufferSize = rollbackValue;
                        return saiStatus;
                    }
                    break;
                }
            default:
                {
                    /* Control should not reach here */
                    XP_SAI_LOG_ERR("Error: Invalid object type %d received\n", objType);
                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetBufferProfileAttrSharedDynamicTh

sai_status_t xpSaiSetBufferProfileAttrSharedDynamicTh(sai_object_id_t
                                                      profileObjId, sai_attribute_value_t value)
{
    xpSaiBufferProfile_t                *pProfileInfo = NULL;
    xpSaiBufferPool_t                   *pPoolInfo    = NULL;
    sai_status_t                         saiStatus    = SAI_STATUS_SUCCESS;
    XP_STATUS                            xpStatus     = XP_NO_ERR;
    sai_object_type_t                    objType;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT alfa;
    GT_BOOL                              conversionError = GT_FALSE;
    sai_uint32_t portNum=0, qNum=0, pfcTc;

    /* Get the buffer profile state data */
    saiStatus = xpSaiBufferProfileGetStateData(profileObjId, &pProfileInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* valid value */
    if (pProfileInfo->thMode != SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC)
    {
        XP_SAI_LOG_ERR("can't modify dynamic threshold when profile is not in dynamic mode \n");
        return SAI_STATUS_FAILURE;
    }

    if (pProfileInfo->poolObjId != SAI_NULL_OBJECT_ID)
    {
        xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(profileObjId);
        /* Get the buffer pool state data */
        saiStatus = xpSaiBufferPoolGetStateData(pProfileInfo->poolObjId, &pPoolInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }
        else
        {
            if (!IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
            {
                XP_SAI_LOG_ERR("Error: Cannot modify dynamic threshold of the pool 0x%lx \n",
                               pProfileInfo->poolObjId);
                return SAI_STATUS_FAILURE;
            }
        }

        sai_uint32_t dynPoolId = pPoolInfo->dynPoolId;
        sai_int8_t   dynamicTh = value.s8;

        xpDynThldFraction_e fraction = ((dynamicTh < 0) ? XP_DIVISOR : XP_MULTIPLIER);
        xpDynThldCoeff_e    alpha    = (xpDynThldCoeff_e)((dynamicTh < 0) ?
                                                          (dynamicTh * -1) : (dynamicTh));

        if (pPoolInfo->type == SAI_BUFFER_POOL_TYPE_INGRESS)
        {
            if (!IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
            {
                xpStatus = xpsQosFcConfigurePfcTcDynamicPoolAlpha(xpDevId, dynPoolId, fraction,
                                                                  alpha);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Failed to configure pfc tc dynamic pool alpha, xpStatus:%d\n",
                                   xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }
                XP_SAI_LOG_DBG("Successfully configured pfc tc dynamic pool alpha, dynPoolId %d fraction %d alpha %d for profile 0x%lx\n",
                               dynPoolId, fraction, alpha, profileObjId);
            }
            else if ((pProfileInfo->sharedDynamicTh != value.s8)&&
                     (pProfileInfo->thMode==SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC))
            {
                for (uint32_t count = 0; count < pProfileInfo->objCount; count++)
                {
                    objType = xpSaiObjIdTypeGet(pProfileInfo->objList[count]);

                    switch (objType)
                    {
                        case SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP:

                            xpSaiIngressPriorityGroupGetPortAndPfcTc(pProfileInfo->objList[count], &portNum,
                                                                     &pfcTc);

                            saiStatus = xpSaiSetPfcThresholdOnPortTc(xpDevId, portNum, pfcTc,
                                                                     CONVERT_DYN_TH_TO_CPSS_ALPHA(value.s8),
                                                                     XPSAI_BYTES_TO_PAGES(pProfileInfo->bufferSize));
                            if (saiStatus != SAI_STATUS_SUCCESS)
                            {
                                return saiStatus;
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        else
        {
            /*For Falcon alfa will be configured when appling the object to profile*/
            XP_DEV_TYPE_T devType = xpSaiSwitchDevTypeGet();
            if (!IS_DEVICE_FALCON(devType))
            {
                xpStatus = xpsQosAqmConfigureDynamicPoolAlpha(xpDevId, dynPoolId, fraction,
                                                              alpha);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Failed to configure aqm dynamic pool alpha, xpStatus: %d\n",
                                   xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }
                XP_SAI_LOG_DBG("Successfully configured aqm dynamic pool alpha, dynPoolId %d fraction %d alpha %d for profile 0x%lx\n",
                               dynPoolId, fraction, alpha, profileObjId);
            }
            else  if ((pProfileInfo->sharedDynamicTh != value.s8)&&
                      (pProfileInfo->thMode==SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC)&&
                      (pProfileInfo->xoffTh == 0)/* Lossy*/)
                /*loop over all attached objects and re-apply*/
            {
                sai_int8_t   tmpDynamicTh = value.s8;

                /* DYN_TH_2_ALFA will modify the parameter 1 */
                DYN_TH_2_ALFA(tmpDynamicTh, alfa, conversionError);
                if (GT_TRUE==conversionError)
                {
                    XP_SAI_LOG_ERR("Failed to configure alfa from sharedDynamicTh: %d\n",
                                   pProfileInfo->sharedDynamicTh);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
                for (uint32_t count = 0; count < pProfileInfo->objCount; count++)
                {
                    objType = xpSaiObjIdTypeGet(pProfileInfo->objList[count]);

                    switch (objType)
                    {
                        case SAI_OBJECT_TYPE_PORT:

                            portNum = xpSaiObjIdValueGet(pProfileInfo->objList[count]);
                            /* Set port   alfa*/
                            xpStatus = xpSaiProfileMngConfigureGuaranteedLimitAndAlfa(xpDevId, portNum,
                                                                                      PROFILE_MNG_PER_PORT_CFG, NULL, &alfa);
                            if (xpStatus != XP_NO_ERR)
                            {
                                XP_SAI_LOG_ERR("Failed to configure port page tail drop threshold, xpStatus: %d\n",
                                               xpStatus);
                                return xpsStatus2SaiStatus(xpStatus);
                            }

                            break;
                        case SAI_OBJECT_TYPE_QUEUE:

                            /* Get the port and queue information from queue object id */
                            xpSaiQosQueueObjIdPortValueGet((pProfileInfo->objList[count]), &portNum, &qNum);
                            if (qNum >= XPS_MAX_QUEUES_PER_PORT)
                            {
                                XP_SAI_LOG_ERR("Invalid queueNum :%d \n", qNum);
                                return SAI_STATUS_FAILURE;
                            }


                            xpStatus = xpSaiProfileMngConfigureGuaranteedLimitAndAlfa(xpDevId, portNum,
                                                                                      qNum, NULL, &alfa);
                            if (xpStatus != XP_NO_ERR)
                            {
                                XP_SAI_LOG_ERR("Failed to configure queue page tail drop threshold, xpStatus: %d\n",
                                               xpStatus);
                                return xpsStatus2SaiStatus(xpStatus);
                            }

                        default:
                            break;

                    }
                }
            }
        }
    }

    /* Update state data */
    pProfileInfo->sharedDynamicTh = value.s8;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetBufferProfileAttrSharedStaticTh

sai_status_t xpSaiSetBufferProfileAttrSharedStaticTh(sai_object_id_t
                                                     profileObjId, sai_attribute_value_t value)
{
    xpSaiBufferProfile_t *pProfileInfo = NULL;
    xpSaiBufferPool_t    *pPoolInfo    = NULL;
    sai_status_t          saiStatus    = SAI_STATUS_SUCCESS;
    XP_STATUS             xpStatus     = XP_NO_ERR;
    sai_uint32_t          port = 0, queue = 0;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT alfa =
        CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

    /* Get the buffer profile state data */
    saiStatus = xpSaiBufferProfileGetStateData(profileObjId, &pProfileInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* valid value */
    if (pProfileInfo->thMode != SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC)
    {
        XP_SAI_LOG_ERR("can't modify static threshold when profile is not in static mode \n");
        return SAI_STATUS_FAILURE;
    }

    if (pProfileInfo->poolObjId != SAI_NULL_OBJECT_ID)
    {
        /* Get the buffer pool state data */
        saiStatus = xpSaiBufferPoolGetStateData(pProfileInfo->poolObjId, &pPoolInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }
        if (value.u32 > pPoolInfo->size)
        {
            XP_SAI_LOG_ERR("static threshold can't greater than pool size \n");
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(profileObjId);
    sai_object_type_t objType;


    for (uint32_t count = 0; count < pProfileInfo->objCount; count++)
    {
        objType = xpSaiObjIdTypeGet(pProfileInfo->objList[count]);

        switch (objType)
        {
            case SAI_OBJECT_TYPE_PORT:
                {
                    uint32_t portFullThInPages = 0;

                    port = xpSaiObjIdValueGet(pProfileInfo->objList[count]);

                    if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
                    {
                        uint32_t taildropThInPages = 0;
                        if (pProfileInfo->poolObjId != SAI_NULL_OBJECT_ID)
                        {
                            /* Get the buffer pool state data */
                            saiStatus = xpSaiBufferPoolGetStateData(pProfileInfo->poolObjId, &pPoolInfo);
                            if (saiStatus != SAI_STATUS_SUCCESS)
                            {
                                return saiStatus;
                            }

                            if (value.u32 == 0)
                            {
                                /* When static threshold for the shared usage is set to zero then
                                 * there is no limit for the shared usage, so set the tail drop
                                 * threshold to pool size (guaranteed + shared) */
                                taildropThInPages = XPSAI_BYTES_TO_PAGES(pPoolInfo->size);
                            }
                            else
                            {
                                /* Convert static shared threshold from bytes to pages */
                                taildropThInPages = XPSAI_BYTES_TO_PAGES(value.u32);
                            }

                            xpSaiQosQueueObjIdPortValueGet(pProfileInfo->objList[count], &port, &queue);
                            if (queue >= XPS_MAX_QUEUES_PER_PORT)
                            {
                                XP_SAI_LOG_ERR("Invalid queueNum :%d \n", queue);
                                return SAI_STATUS_FAILURE;
                            }


                            xpStatus = xpSaiProfileMngConfigureGuaranteedLimitAndAlfa(xpDevId, port, queue,
                                                                                      &taildropThInPages, &alfa);
                            if (xpStatus != XP_NO_ERR)
                            {
                                XP_SAI_LOG_ERR("Failed to configure queue page tail drop threshold, xpStatus: %d\n",
                                               xpStatus);
                                return xpsStatus2SaiStatus(xpStatus);
                            }
                        }
                    }
                    else
                    {
                        /*
                         * In loosy case, set the port's full threshould to total packet memory
                         * when shared static threshold is passes as 0, this is to disable accounting
                         * at ingress. Set the port's full threshould to passed in shared static
                         * threshold when it is non zero.
                         * In lossless case, port's full threshould is set to buffer size.
                         */
                        if (pProfileInfo->xoffTh == 0) // Lossy
                        {
                            if (value.u32 == 0)
                            {
                                portFullThInPages = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES - 1);
                            }
                            else
                            {
                                portFullThInPages = XPSAI_BYTES_TO_PAGES(value.u32);
                            }

                            xpStatus = xpsQosFcSetPortFcFullThreshold(xpDevId, port, portFullThInPages);
                            if (xpStatus != XP_NO_ERR)
                            {
                                XP_SAI_LOG_ERR("Error: Failed to set full threshold for a port, xpStatus: %d\n",
                                               xpStatus);
                                return xpsStatus2SaiStatus(xpStatus);
                            }
                        }
                    }
                    break;
                }
            case SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP:
                {
                    if (value.u32 != 0)
                    {
                        XP_SAI_LOG_ERR("Error: Ingress buffer profile with shared static thresholding is not supported\n");
                        return SAI_STATUS_NOT_SUPPORTED;
                    }
                    break;
                }
            case SAI_OBJECT_TYPE_QUEUE:
                {
                    uint32_t taildropThInPages = 0;
                    if (pProfileInfo->poolObjId != SAI_NULL_OBJECT_ID)
                    {
                        /* Get the buffer pool state data */
                        saiStatus = xpSaiBufferPoolGetStateData(pProfileInfo->poolObjId, &pPoolInfo);
                        if (saiStatus != SAI_STATUS_SUCCESS)
                        {
                            return saiStatus;
                        }

                        if (value.u32 == 0)
                        {
                            /* When static threshold for the shared usage is set to zero then
                            * there is no limit for the shared usage, so set the tail drop
                            * threshold to pool size (guaranteed + shared) */
                            taildropThInPages = XPSAI_BYTES_TO_PAGES(pPoolInfo->size);
                        }
                        else
                        {
                            /* Convert static shared threshold from bytes to pages */
                            taildropThInPages = XPSAI_BYTES_TO_PAGES(value.u32);
                        }

                        xpSaiQosQueueObjIdPortValueGet(pProfileInfo->objList[count], &port, &queue);
                        if (queue >= XPS_MAX_QUEUES_PER_PORT)
                        {
                            XP_SAI_LOG_ERR("Invalid queueNum :%d \n", queue);
                            return SAI_STATUS_FAILURE;
                        }

                        xpStatus = xpSaiProfileMngConfigureGuaranteedLimitAndAlfa(xpDevId, port, queue,
                                                                                  &taildropThInPages, &alfa);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Failed to configure queue page tail drop threshold, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }
                    }
                    break;
                }
            default:
                {
                    /* Control should not reach here */
                    XP_SAI_LOG_ERR("Error: Invalid object type %d received\n", objType);
                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                }
        }
    }

    /* Update stae data */
    pProfileInfo->sharedStaticTh = value.u32;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetBufferProfileAttrXoffTh

sai_status_t xpSaiSetBufferProfileAttrXoffTh(sai_object_id_t profileObjId,
                                             sai_attribute_value_t value)
{
    xpSaiBufferProfile_t  *pProfileInfo = NULL;
    xpSaiBufferPool_t     *pPoolInfo    = NULL;
    sai_status_t           saiStatus    = SAI_STATUS_SUCCESS;
    XP_STATUS              xpStatus     = XP_NO_ERR;
    sai_uint32_t           guarBuffer   = 0;
    sai_uint32_t           newGuarBuffer   = 0;

    /* Get the buffer profile state data */
    saiStatus = xpSaiBufferProfileGetStateData(profileObjId, &pProfileInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    uint32_t xoffSize = 0, xoffThInPages = 0;

    if (pProfileInfo->poolObjId != SAI_NULL_OBJECT_ID)
    {
        /* Get the buffer pool state data */
        saiStatus = xpSaiBufferPoolGetStateData(pProfileInfo->poolObjId, &pPoolInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }

        xoffSize = pPoolInfo->xoffSize;
    }

    guarBuffer = pProfileInfo->bufferSize;
    newGuarBuffer = pProfileInfo->bufferSize;

    /* when pool xoff == 0, the xoff buffers of profile were reserved.
     * Otherwise, the share xoff buffers of pool were reserved.
     */
    if (pPoolInfo && pPoolInfo->xoffSize == 0)
    {
        guarBuffer += pProfileInfo->xoffTh;
        newGuarBuffer += value.u32;
    }

    /* Note that the available headroom buffer is dependent on XOFF_SIZE */
    if (xoffSize > 0)
    {
        /*
         * Total headroom pool buffer for all PGs is equal to XOFF_SIZE
         * and XOFF_TH specifies the maximum amount of headroom pool
         * buffer one PG can use
         */
        if (value.u32 > xoffSize)
        {
            XP_SAI_LOG_ERR("Error: xoff can not be more than shared global headroom size\n");
            return SAI_STATUS_FAILURE;
        }
    }

    /* xoff only take effect in Ingress Pool */
    if (pPoolInfo && pPoolInfo->type != SAI_BUFFER_POOL_TYPE_INGRESS)
    {
        XP_SAI_LOG_ERR("xoff only can be modified when profile is in ingress pool \n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((pProfileInfo->poolObjId != SAI_NULL_OBJECT_ID)
        && (guarBuffer != newGuarBuffer))
    {
        guarBuffer    *= pProfileInfo->objCount;
        newGuarBuffer *= pProfileInfo->objCount;

        /* is buffer resource sufficient */
        if (pPoolInfo->sharedSize + guarBuffer < newGuarBuffer)
        {
            XP_SAI_LOG_ERR("Error: pool cur share %u, old guar %u, new guar %u \n",
                           pPoolInfo->sharedSize, guarBuffer, newGuarBuffer);
            return SAI_STATUS_INSUFFICIENT_RESOURCES;
        }

        /* Free the existing buffer pool memory */
        saiStatus = xpSaiBufferPoolFreeGuaranteedBuffer(pProfileInfo->poolObjId,
                                                        guarBuffer);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to free the guaranteed buffer, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }

        /* Allocate new memory from buffer pool */
        saiStatus = xpSaiBufferPoolAllocateGuaranteedBuffer(pProfileInfo->poolObjId,
                                                            newGuarBuffer);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to allocate guaranteed buffer from buffer pool, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    xoffThInPages = XPSAI_BYTES_TO_PAGES(pProfileInfo->bufferSize);

    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(profileObjId);
    sai_uint32_t pfcProfileId  = 0, port = 0, pfcTc = 0;
    sai_object_type_t objType;
    sai_uint32_t guarHeadroomInPages = XPSAI_BYTES_TO_PAGES(value.u32);
    sai_uint32_t fullThresholdInPages = XPSAI_BYTES_TO_PAGES(newGuarBuffer);

    for (uint32_t count = 0; count < pProfileInfo->objCount; count++)
    {
        objType = xpSaiObjIdTypeGet(pProfileInfo->objList[count]);

        switch (objType)
        {
            case SAI_OBJECT_TYPE_PORT:
                {
                    if (IS_DEVICE_XP70_XP60(xpSaiSwitchDevTypeGet()))
                    {
                        port = xpSaiObjIdValueGet(pProfileInfo->objList[count]);
                        /* Set flow control xoff threshold for a port */
                        xpStatus = xpsQosFcSetPortFcXoffThreshold(xpDevId, port, xoffThInPages);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set xoff threshold for a port, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }

                        /* Set port flow control full threshold */
                        xpStatus = xpsQosFcSetPortFcFullThreshold(xpDevId, port, fullThresholdInPages);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set port flow control full threshold, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }
                    }

                    break;
                }
            case SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP:
                {
                    xpSaiIngressPriorityGroupGetPortAndPfcTc(pProfileInfo->objList[count], &port,
                                                             &pfcTc);

                    if (IS_DEVICE_XP70_XP60(xpSaiSwitchDevTypeGet()))
                    {
                        /* Set the port/tc pfc xoff threshold */
                        xpStatus = xpsQosFcSetPortTcPfcStaticXoffThreshold(xpDevId, port, pfcTc,
                                                                           xoffThInPages);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set port/tc pfc xoff threshold, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }

                        /* Set guaranteed headroom buffer per PG in IBuffer */
                        xpStatus = xpsQosFcConfigureMaxHeadroomThreshold(xpDevId, port, pfcTc,
                                                                         guarHeadroomInPages);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to configure max headroom threshold, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }
                        XP_SAI_LOG_DBG("Successfully set PFC TC xoff threshold and Max hdrm for port %d pfc tc %d xoff thresh %u guaranteed hdrm %u \n",
                                       port, pfcTc, xoffThInPages, guarHeadroomInPages);
                    }
                    else
                    {
                        /* Get port/tc pfc profile id */
                        xpStatus = xpsQosFcGetPortToPortTcPfcStaticProfile(xpDevId, port,
                                                                           &pfcProfileId);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to get port/tc pfc profile, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }

                        /* Set the port/tc pfc xoff threshold for profile */
                        xpStatus = xpsQosFcSetPortTcPfcStaticXoffThresholdForProfile(xpDevId,
                                                                                     pfcProfileId, pfcTc, xoffThInPages);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set port/tc pfc xoff threshold, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }
                    }
                    break;
                }
            case SAI_OBJECT_TYPE_QUEUE:
                {
                    /* Do nothing */
                    break;
                }
            default:
                {
                    /* Control should not reach here */
                    XP_SAI_LOG_ERR("Error: Invalid object type %d received\n", objType);
                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                }
        }
    }

    /* Update state data */
    pProfileInfo->xoffTh = value.u32;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetBufferProfileAttrXonTh

sai_status_t xpSaiSetBufferProfileAttrXonTh(sai_object_id_t profileObjId,
                                            sai_attribute_value_t value)
{
    xpSaiBufferProfile_t  *pProfileInfo = NULL;
    xpSaiBufferPool_t     *pPoolInfo    = NULL;
    sai_status_t           saiStatus    = SAI_STATUS_SUCCESS;
    XP_STATUS              xpStatus     = XP_NO_ERR;

    /* Get the buffer profile state data */
    saiStatus = xpSaiBufferProfileGetStateData(profileObjId, &pProfileInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(pProfileInfo->poolObjId, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* xon only take effect in Ingress Pool */
    if (pPoolInfo->type != SAI_BUFFER_POOL_TYPE_INGRESS)
    {
        XP_SAI_LOG_ERR("Xon only can be modified when profile is in ingress pool \n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (value.u32 > pPoolInfo->size)
    {
        XP_SAI_LOG_ERR("Xon can't greater than pool size \n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(profileObjId);
    sai_uint32_t pfcProfileId  = 0, port = 0, pfcTc = 0;
    sai_uint32_t xonThInPages = XPSAI_BYTES_TO_PAGES(value.u32);
    sai_object_type_t objType;

    for (uint32_t count = 0; count < pProfileInfo->objCount; count++)
    {
        objType = xpSaiObjIdTypeGet(pProfileInfo->objList[count]);

        switch (objType)
        {
            case SAI_OBJECT_TYPE_PORT:
                {
                    if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
                    {
                        XP_SAI_LOG_DBG("Error: XON threshold for a port is ignored\n");
                    }
                    else
                    {
                        port = xpSaiObjIdValueGet(pProfileInfo->objList[count]);
                        /* Set flow control xon threshold for a port */
                        xpStatus = xpsQosFcSetPortFcXonThreshold(xpDevId, port, xonThInPages);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set xon threshold for a port, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }
                    }
                    break;
                }
            case SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP:
                {
                    xpSaiIngressPriorityGroupGetPortAndPfcTc(pProfileInfo->objList[count], &port,
                                                             &pfcTc);

                    if (IS_DEVICE_XP70_XP60(xpSaiSwitchDevTypeGet()))
                    {
                        /* Set the port/tc pfc xoff threshold */
                        xpStatus = xpsQosFcSetPortTcPfcStaticXonThreshold(xpDevId, port, pfcTc,
                                                                          xonThInPages);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set port/tc pfc xon threshold, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }
                        XP_SAI_LOG_DBG("Successfully set PFC TC xon threshold for port %d pfc tc %d xon thresh %u \n",
                                       port, pfcTc, xonThInPages);
                    }
                    else if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
                    {
                        XP_SAI_LOG_DBG("XON is not supported on port/tc\n");
                    }
                    else
                    {
                        /* Get port/tc fc profile id */
                        xpStatus = xpsQosFcGetPortToPortTcPfcStaticProfile(xpDevId, port,
                                                                           &pfcProfileId);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to get port/tc pfc profile, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }

                        /* Set the port/tc pfc xoff threshold for profile */
                        xpStatus = xpsQosFcSetPortTcPfcStaticXonThresholdForProfile(xpDevId,
                                                                                    pfcProfileId, pfcTc, xonThInPages);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set port/tc pfc xon threshold, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }
                    }
                    break;
                }
            case SAI_OBJECT_TYPE_QUEUE:
                {
                    /* Do nothing */
                    break;
                }
            default:
                {
                    /* Control should not reach here */
                    XP_SAI_LOG_ERR("Error: Invalid object type %d received\n", objType);
                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                }
        }
    }

    /* Update state data */
    pProfileInfo->xonTh = value.u32;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetBufferProfileAttrXonOffsetTh

sai_status_t xpSaiSetBufferProfileAttrXonOffsetTh(sai_object_id_t profileObjId,
                                                  sai_attribute_value_t value)
{
    //TODO: Check if this can be supported
    XP_SAI_LOG_ERR("Set (SAI_BUFFER_PROFILE_ATTR_XON_OFFSET_TH) is not supported\n");
    return SAI_STATUS_NOT_SUPPORTED;
}

//Func: xpSaiSetDefaultBufferPoolAttributeVals

void xpSaiSetDefaultBufferPoolAttributeVals(xpSaiBufferPoolAttributesT*
                                            attributes)
{
    memset(attributes, 0x0, sizeof(xpSaiBufferPoolAttributesT));

    /* Shared threshold mode for the buffer pool is NOT supported, default to SAI_BUFFER_POOL_DYNAMIC_TH */
    attributes->thMode.s32     = SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC;
    attributes->sharedSize.u32 = 0;
}

//Func: xpSaiUpdateBufferPoolAttributeVals

void xpSaiUpdateBufferPoolAttributeVals(const uint32_t attr_count,
                                        const sai_attribute_t* attr_list, xpSaiBufferPoolAttributesT* attributes)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_BUFFER_POOL_ATTR_SHARED_SIZE:
                {
                    attributes->sharedSize = attr_list[count].value;
                    break;
                }
            case SAI_BUFFER_POOL_ATTR_TYPE:
                {
                    attributes->type = attr_list[count].value;
                    break;
                }
            case SAI_BUFFER_POOL_ATTR_SIZE:
                {
                    attributes->size = attr_list[count].value;
                    break;
                }
            case SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE:
                {
                    attributes->thMode = attr_list[count].value;
                    break;
                }
            case SAI_BUFFER_POOL_ATTR_XOFF_SIZE:
                {
                    attributes->xoffSize = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Error: Unknown attribute %d\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiUpdateAttrListBufferPoolVals

void xpSaiUpdateAttrListBufferPoolVals(const xpSaiBufferPoolAttributesT*
                                       attributes, const uint32_t attr_count, sai_attribute_t* attr_list)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_BUFFER_POOL_ATTR_SHARED_SIZE:
                {
                    attr_list[count].value = attributes->sharedSize;
                    break;
                }
            case SAI_BUFFER_POOL_ATTR_TYPE:
                {
                    attr_list[count].value = attributes->type;
                    break;
                }
            case SAI_BUFFER_POOL_ATTR_SIZE:
                {
                    attr_list[count].value = attributes->size;
                    break;
                }
            case SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE:
                {
                    attr_list[count].value = attributes->thMode;
                    break;
                }
            case SAI_BUFFER_POOL_ATTR_XOFF_SIZE:
                {
                    attr_list[count].value = attributes->xoffSize;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Undefined attribute %d\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiSetBufferPoolAttrSize

sai_status_t xpSaiSetBufferPoolAttrSize(sai_object_id_t pool_id,
                                        sai_attribute_value_t value)
{
    xpSaiBufferPool_t *pPoolInfo = NULL;
    sai_status_t       saiStatus    = SAI_STATUS_SUCCESS;

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(pool_id, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    uint32_t totalSizeNewInBytes = value.u32;

    /* Calculate the total guaranteed buffer size of all the queue/port/pg.
     * This is derived from substracting shared pool size
     * from the total pool size */
    uint32_t totalGuaranteedSizeInBytes = (pPoolInfo->size - pPoolInfo->sharedSize);

    /* Validate buffer pool size  */
    if (totalSizeNewInBytes > XPSAI_BUFFER_TOTAL_BUFFER_BYTES)
    {
        XP_SAI_LOG_ERR("Invalid buffer pool size value %d provided\n",
                       totalSizeNewInBytes);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* New buffer pool size can not be less than total reserved size */
    if (totalSizeNewInBytes < totalGuaranteedSizeInBytes)
    {
        XP_SAI_LOG_ERR("Error: Buffer pool size %d can not be less than total reserved size %d\n",
                       totalSizeNewInBytes, totalGuaranteedSizeInBytes);
        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(0);
    }

    /* Calculate the new buffer pool shared size (including global headroom) */
    uint32_t sharedSizeNewInBytes = (totalSizeNewInBytes -
                                     totalGuaranteedSizeInBytes);
#if 0
    Reseve size already have included Xoff size.
    if (pPoolInfo->xoffSize > sharedSizeNewInBytes)
    {
        XP_SAI_LOG_ERR("Error: Global shared headroom size %d can not be greater than shared size %d\n",
                       pPoolInfo->xoffSize, sharedSizeNewInBytes);
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    /* Consider global shared headroom */
    sharedSizeNewInBytes = (sharedSizeNewInBytes - pPoolInfo->xoffSize);
#endif
    if (pPoolInfo->thMode == SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC)
    {
        /* Get the device from buffer pool object */
        xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(pool_id);

        /* Configure the dynamic pool */
        saiStatus = xpSaiBufferPoolConfigureDynamicPool(xpDevId, pPoolInfo->type,
                                                        pPoolInfo->dynPoolId,
                                                        totalGuaranteedSizeInBytes,
                                                        sharedSizeNewInBytes, totalSizeNewInBytes);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }

        XP_SAI_LOG_DBG("Successfully set dynamic pool %d guaranteed size %u bytes, shared size %u bytes, total size %u bytes \n",
                       pPoolInfo->dynPoolId, totalGuaranteedSizeInBytes, sharedSizeNewInBytes,
                       totalSizeNewInBytes);
    }

    /* Update state data */
    pPoolInfo->size       = totalSizeNewInBytes;
    pPoolInfo->sharedSize = sharedSizeNewInBytes;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetBufferPoolAttrXoffSize

sai_status_t xpSaiSetBufferPoolAttrXoffSize(sai_object_id_t pool_id,
                                            sai_attribute_value_t value)
{
    xpSaiBufferPool_t *pPoolInfo               = NULL;
    sai_status_t       saiStatus               = SAI_STATUS_SUCCESS;
    uint32_t           newPoolXoff             = (uint32_t)value.u64;
    uint32_t           oldPoolXoff             = 0;
    uint32_t           allProfileAllPGXoffSize = 0;
    uint32_t           freeSize;
    uint32_t           allocSize;

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(pool_id, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* valid value */
    if (pPoolInfo->type != SAI_BUFFER_POOL_TYPE_INGRESS)
    {
        XP_SAI_LOG_ERR("Error: pool xoff only can set to ingress pool \n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (newPoolXoff > pPoolInfo->size)
    {
        XP_SAI_LOG_ERR("Error: pool xoff size %u can't great than pool size %u \n",
                       newPoolXoff, pPoolInfo->size);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (newPoolXoff == pPoolInfo->xoffSize)
    {
        return SAI_STATUS_SUCCESS;
    }

    oldPoolXoff = pPoolInfo->xoffSize;

    /* calculate the pool's all profiles' PG's total Xoff size */
    for (uint32_t i = 0; i < pPoolInfo->profileCount; i++)
    {
        xpSaiBufferProfile_t *pProfileInfo = NULL;

        saiStatus = xpSaiBufferProfileGetStateData(pPoolInfo->profileList[i],
                                                   &pProfileInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }

        for (uint32_t j = 0; j < pProfileInfo->objCount; j++)
        {
            // currenttly, ingress buffer profile only support PG, do not support port.
            if (XDK_SAI_OBJID_TYPE_CHECK(pProfileInfo->objList[j],
                                         SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP))
            {
                allProfileAllPGXoffSize += pProfileInfo->xoffTh;
            }
        }
    }

    /* Free size */
    /* when old pool xoff != 0, the pool xoff buffers were reserved.
     * Otherwise, the profile's xoff buffers of all PG were reserved.
     */
    if (oldPoolXoff != 0)
    {
        freeSize = oldPoolXoff;
    }
    else
    {
        freeSize = allProfileAllPGXoffSize;
    }

    /* Allocate size */
    /* when new pool xoff != 0, the new pool xoff buffers are need to be reserved.
     * Otherwise, the profile's xoff buffers of all PG are need to be reserved.
     */
    if (newPoolXoff != 0)
    {
        allocSize = newPoolXoff;
    }
    else
    {
        allocSize = allProfileAllPGXoffSize;
    }

    /* is buffer resource sufficient */
    if (pPoolInfo->sharedSize + freeSize < allocSize)
    {
        XP_SAI_LOG_ERR("pool buffer insufficient. cur share %u, free %u, alloc %u. %s is too large \n",
                       pPoolInfo->sharedSize, freeSize, allocSize,
                       (newPoolXoff != 0) ? "new pool xoff" : "profile xoff");
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    /* free old xoff size */
    saiStatus = xpSaiBufferPoolFreeGuaranteedBuffer(pool_id, freeSize);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to free the guaranteed buffer, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }
    /* allocate new xoff size */
    saiStatus = xpSaiBufferPoolAllocateGuaranteedBuffer(pool_id, allocSize);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to allocate guaranteed buffer from buffer pool, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Update XPS State DB data */
    pPoolInfo->xoffSize = newPoolXoff;

    return SAI_STATUS_SUCCESS;

}

//Func: xpSaiSetBufferPoolAttrSharedSize

sai_status_t xpSaiSetBufferPoolAttrSharedSize(sai_object_id_t pool_id,
                                              sai_attribute_value_t value)
{
    xpSaiBufferPool_t *pPoolInfo = NULL;
    sai_status_t       saiStatus    = SAI_STATUS_SUCCESS;

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(pool_id, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    uint32_t sharedSizeNewInBytes = value.u32;

    if (pPoolInfo->xoffSize > sharedSizeNewInBytes)
    {
        XP_SAI_LOG_ERR("Error: Global shared headroom size %d can not be greater than shared size %d\n",
                       pPoolInfo->xoffSize, sharedSizeNewInBytes);
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    /* Consider global shared headroom */
    sharedSizeNewInBytes = (sharedSizeNewInBytes - pPoolInfo->xoffSize);

    if (pPoolInfo->thMode == SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC)
    {
        /* Get the device from buffer pool object */
        xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(pool_id);
        uint32_t totalGuaranteedSizeInBytes = 0;
        /* Configure the dynamic pool */
        saiStatus = xpSaiBufferPoolConfigureDynamicPool(xpDevId, pPoolInfo->type,
                                                        pPoolInfo->dynPoolId,
                                                        totalGuaranteedSizeInBytes,
                                                        sharedSizeNewInBytes, pPoolInfo->size);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }

        XP_SAI_LOG_DBG("Successfully set dynamic pool %d guaranteed size %u bytes, shared size %u bytes, total size %u bytes \n",
                       pPoolInfo->dynPoolId, totalGuaranteedSizeInBytes, sharedSizeNewInBytes,
                       pPoolInfo->size);
    }
    else
    {
        sharedSizeNewInBytes = 0;

        XP_SAI_LOG_DBG("Successfully set static pool %d sharedSizeInBytes %u \n",
                       pPoolInfo->dynPoolId, sharedSizeNewInBytes);

    }

    /* Update state data */
    pPoolInfo->sharedSize = sharedSizeNewInBytes;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateBufferPool

sai_status_t xpSaiCreateBufferPool(sai_object_id_t *pool_id,
                                   sai_object_id_t switch_id,
                                   uint32_t attr_count, const sai_attribute_t *attr_list)
{
    xpSaiBufferPoolAttributesT  attributes;
    xpSaiBufferPool_t          *pPoolInfo = NULL;
    sai_uint32_t                dynPoolId = 0;
    sai_status_t                saiStatus = SAI_STATUS_SUCCESS;
    GT_STATUS          rc = GT_OK;

    /* Validate attributes */
    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               BUFFER_POOL_VALIDATION_ARRAY_SIZE,
                               buffer_pool_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    if (!pool_id)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpSaiSetDefaultBufferPoolAttributeVals(&attributes);
    xpSaiUpdateBufferPoolAttributeVals(attr_count, attr_list, &attributes);

    /* Validate input attribute values */
    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiStatus = xpSaiBufferPoolValidateAttributeValue(attr_list[count].id,
                                                          attr_list[count].value, count);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }
    }

    /*
     * Both pool shared size and pool total size are same at the time of buffer
     * pool create. Update the pool shared threshold when ever buffer profile
     * is attached/detached to/from a PG or Queue. No change in pool total size
     */
    attributes.sharedSize = attributes.size;

    if (attributes.xoffSize.u32 > 0 &&
        attributes.type.s32 != SAI_BUFFER_POOL_TYPE_INGRESS)
    {
        XP_SAI_LOG_ERR("pool xoff size is only valid in ingress pool \n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (attributes.xoffSize.u32 > attributes.size.u32)
    {
        XP_SAI_LOG_ERR("Error: Global shared headroom size %d can not be greater than pool size %d\n",
                       attributes.xoffSize.u32, attributes.size);
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    /* Get the device from switch object */
    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(switch_id);

    /* Create a buffer pool object */
    saiStatus = xpSaiBufferPoolCreateObject(xpDevId, attributes.type.s32, pool_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not create buffer pool object, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Create a state database */
    saiStatus = xpSaiBufferPoolCreateStateData(*pool_id, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not create buffer pool state database, saiStatus: %d\n",
                       saiStatus);
        xpSaiBufferPoolRemoveObject(*pool_id);
        return saiStatus;
    }

    uint32_t type              = attributes.type.s32;
    uint32_t totalSizeInBytes  = attributes.size.u32;
    uint32_t sharedSizeInBytes = attributes.sharedSize.u32 -
                                 attributes.xoffSize.u32;
    uint32_t totalSizeInPages  = XPSAI_BYTES_TO_PAGES(totalSizeInBytes);

    if (attributes.thMode.s32 == SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC)
    {
        /* Create a dynamic pool and enable shared dynamic thresholding */
        saiStatus = xpSaiBufferPoolCreateDynamicPool(xpDevId, type, sharedSizeInBytes,
                                                     totalSizeInBytes, &dynPoolId);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Counld not create shared dynamic pool, saiStatus: %d\n",
                           saiStatus);
            xpSaiBufferPoolRemoveObject(*pool_id);
            xpSaiBufferPoolRemoveStateData(*pool_id);
            pool_id = NULL;
            return saiStatus;
        }
    }
    else
    {
        if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
        {
            /* Allocate a dynamic pool id */
            saiStatus = xpSaiBufferPoolAllocateStaticPoolId(xpDevId, type, &dynPoolId);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                xpSaiBufferPoolRemoveObject(*pool_id);
                xpSaiBufferPoolRemoveStateData(*pool_id);
                pool_id = NULL;
                return saiStatus;
            }
            if (type == SAI_BUFFER_POOL_TYPE_EGRESS)
            {
                /* in static mode,  when create buffer pool, pool HW's shared size = total size. */
                rc = cpssHalPortTxTailDropConfigureAvaileblePool(xpDevId, dynPoolId,
                                                                 totalSizeInPages, GT_FALSE);
                if (rc != GT_OK)
                {
                    XP_SAI_LOG_ERR("Failed to configure availeble pool size  |devId:%d, dynPoolId :%d, retVal %d\n",
                                   xpDevId, dynPoolId, rc);
                    xpSaiBufferPoolRemoveObject(*pool_id);
                    xpSaiBufferPoolRemoveStateData(*pool_id);
                    pool_id = NULL;
                    return cpssStatus2SaiStatus(rc);
                }
                /* Set the dynamic pool's total size */
                rc = cpssHalPortTxSharedPoolLimitsSet(xpDevId, dynPoolId, totalSizeInPages,
                                                      GT_FALSE);
                if (rc != GT_OK)
                {
                    XP_SAI_LOG_ERR("Error: Failed to set dynamic pool total size %d, poolId: %d, rc:%d\n",
                                   totalSizeInBytes, dynPoolId, rc);
                    xpSaiBufferPoolRemoveObject(*pool_id);
                    xpSaiBufferPoolRemoveStateData(*pool_id);
                    pool_id = NULL;
                    return cpssStatus2SaiStatus(rc);
                }
            }
        }
    }

    /* Update state data */
    pPoolInfo->dynPoolId  = dynPoolId;
    pPoolInfo->sharedSize = sharedSizeInBytes;
    pPoolInfo->type       = (sai_buffer_pool_type_t)attributes.type.s32;
    pPoolInfo->size       = attributes.size.u32;
    pPoolInfo->thMode     = (sai_buffer_pool_threshold_mode_t)attributes.thMode.s32;
    pPoolInfo->xoffSize   = attributes.xoffSize.u32;

    XP_SAI_LOG_DBG("Successfully created buffer pool obj id 0x%lx \n", *pool_id);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveBufferPool

sai_status_t xpSaiRemoveBufferPool(sai_object_id_t pool_id)
{
    xpSaiBufferPool_t *pPoolInfo = NULL;
    sai_status_t       saiStatus    = SAI_STATUS_SUCCESS;
    sai_uint32_t       totalSizeInBytes;
    GT_STATUS          rc = GT_OK;

    /* Validate the buffer pool object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(pool_id, SAI_OBJECT_TYPE_BUFFER_POOL))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(pool_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Check if buffer pool exists */
    saiStatus = xpSaiBufferPoolGetStateData(pool_id, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    if (pPoolInfo->profileCount != 0)
    {
        XP_SAI_LOG_ERR("Error: Buffer pool in use, unmap before remove\n");
        return SAI_STATUS_OBJECT_IN_USE;
    }

    /* Get the device from buffer pool object */
    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(pool_id);

    if (pPoolInfo->thMode == SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC)
    {
        /* Remove dynamic pool and disable shared dynamic thresholding */
        saiStatus = xpSaiBufferPoolRemoveDynamicPool(xpDevId, pPoolInfo->type,
                                                     pPoolInfo->dynPoolId);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Counld not remove shared dynamic pool, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }
    else
    {
        if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
        {
            if (pPoolInfo->type == SAI_BUFFER_POOL_TYPE_EGRESS)
            {
                /* Remove static pool */
                totalSizeInBytes = FALCON_12_8_SAI_BUFFER_TOTAL_BUFFER_SIZE;
                rc = cpssHalPortTxSharedPoolLimitsSet(xpDevId, pPoolInfo->dynPoolId,
                                                      totalSizeInBytes, GT_FALSE);
                if (rc != GT_OK)
                {
                    XP_SAI_LOG_ERR("Error while configuring shared size for poolId : %d",
                                   pPoolInfo->dynPoolId);
                    return cpssStatus2SaiStatus(rc);
                }
            }
            /* Release dynamic pool id */
            saiStatus = xpSaiBufferPoolReleaseStaticPoolId(xpDevId, pPoolInfo->type,
                                                           pPoolInfo->dynPoolId);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                return saiStatus;
            }
        }
    }

    /* Remove buffer pool object */
    saiStatus = xpSaiBufferPoolRemoveObject(pool_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not remove buffer pool object, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Remove state database */
    saiStatus = xpSaiBufferPoolRemoveStateData(pool_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not remove buffer pool state database, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    XP_SAI_LOG_DBG("Successfully removed buffer pool obj id 0x%lx \n", pool_id);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetBufferPoolAttribute

sai_status_t xpSaiSetBufferPoolAttribute(sai_object_id_t pool_id,
                                         const sai_attribute_t *attr)
{
    xpSaiBufferPool_t *pPoolInfo = NULL;
    sai_status_t       saiStatus    = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("HANDLE __SET BUFFER POOL  attr->id %d on object 0x%08x%08x\n",
                   attr->id, LONG_INT_FORMAT(pool_id));

    /* Validate input attributes */
    saiStatus = xpSaiAttrCheck(1, attr,
                               BUFFER_POOL_VALIDATION_ARRAY_SIZE,
                               buffer_pool_attribs,
                               SAI_COMMON_API_SET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %" PRIi32 "\n", saiStatus);
        return saiStatus;
    }

    /* Validate the buffer pool object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(pool_id, SAI_OBJECT_TYPE_BUFFER_POOL))
    {
        XP_SAI_LOG_ERR("Wrong object type received (%" PRIu64")\n",
                       xpSaiObjIdTypeGet(pool_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Check if buffer pool exists */
    saiStatus = xpSaiBufferPoolGetStateData(pool_id, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get State Data from DB! SAI error %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    switch (attr->id)
    {
        case SAI_BUFFER_POOL_ATTR_SIZE:
            {
                /* Buffer pool size in bytes */
                saiStatus = xpSaiSetBufferPoolAttrSize(pool_id, attr->value);
                if (SAI_STATUS_SUCCESS != saiStatus)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_BUFFER_POOL_ATTR_SIZE). sai error %" PRIi32
                                   "\n", saiStatus);
                    return saiStatus;
                }
                break;
            }
        case SAI_BUFFER_POOL_ATTR_XOFF_SIZE:
            {
                /* Shared headroom pool size in bytes for lossless traffic.
                 * Only valid for the ingress buffer pool */
                saiStatus = xpSaiSetBufferPoolAttrXoffSize(pool_id, attr->value);
                if (SAI_STATUS_SUCCESS != saiStatus)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_BUFFER_POOL_ATTR_XOFF_SIZE). sai error %"
                                   PRIi32 "\n", saiStatus);
                    return saiStatus;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attribute %" PRIi32 " received!\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr->id);
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetBufferPoolAttribute

static sai_status_t xpSaiGetBufferPoolAttribute(const xpSaiBufferPoolAttributesT
                                                *attributes, sai_attribute_t *attr, uint32_t attr_index)
{
    switch (attr->id)
    {
        case SAI_BUFFER_POOL_ATTR_SHARED_SIZE:
            {
                attr->value = attributes->sharedSize;
                break;
            }
        case SAI_BUFFER_POOL_ATTR_TYPE:
            {
                attr->value = attributes->type;
                break;
            }
        case SAI_BUFFER_POOL_ATTR_SIZE:
            {
                attr->value = attributes->size;
                break;
            }
        case SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE:
            {
                attr->value = attributes->thMode;
                break;
            }
        case SAI_BUFFER_POOL_ATTR_XOFF_SIZE:
            {
                attr->value = attributes->xoffSize;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Attribute %" PRIi32 " is unknown\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index);
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetBufferPoolAttributes

static sai_status_t xpSaiGetBufferPoolAttributes(sai_object_id_t pool_id,
                                                 uint32_t attr_count, sai_attribute_t *attr_list)
{
    xpSaiBufferPoolAttributesT attributes;
    xpSaiBufferPool_t *pPoolInfo = NULL;
    sai_status_t       saiStatus = SAI_STATUS_SUCCESS;

    /* Validate input attributes */
    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               BUFFER_POOL_VALIDATION_ARRAY_SIZE,
                               buffer_pool_attribs,
                               SAI_COMMON_API_GET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    /* Validate buffer pool object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(pool_id, SAI_OBJECT_TYPE_BUFFER_POOL))
    {
        XP_SAI_LOG_ERR("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(pool_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Check if buffer pool exists */
    saiStatus = xpSaiBufferPoolGetStateData(pool_id, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    memset(&attributes, 0, sizeof(xpSaiBufferPoolAttributesT));

    attributes.sharedSize.u32 = pPoolInfo->sharedSize;
    attributes.type.s32       = pPoolInfo->type;
    attributes.size.u32       = pPoolInfo->size;
    attributes.thMode.s32     = pPoolInfo->thMode;
    attributes.xoffSize.u32   = pPoolInfo->xoffSize;

    xpSaiUpdateAttrListBufferPoolVals(&attributes, attr_count, attr_list);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBulkGetBufferPoolAttributes

sai_status_t xpSaiBulkGetBufferPoolAttributes(sai_object_id_t id,
                                              uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t      saiRetVal        = SAI_STATUS_SUCCESS;
    uint32_t          idx              = 0;
    uint32_t          maxcount         = 0;
    xpSaiBufferPool_t *pPoolInfo = NULL;
    xpSaiBufferPoolAttributesT attributes;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_BUFFER_POOL))
    {
        XP_SAI_LOG_ERR("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountBufferPoolAttribute(&maxcount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get max attribute count!\n");
        return SAI_STATUS_FAILURE;
    }

    if (*attr_count < maxcount)
    {
        *attr_count = maxcount;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    /* Get the buffer pool state data */
    saiRetVal = xpSaiBufferPoolGetStateData(id, &pPoolInfo);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    memset(&attributes, 0, sizeof(xpSaiBufferPoolAttributesT));

    attributes.sharedSize.u32 = pPoolInfo->sharedSize;
    attributes.type.s32       = pPoolInfo->type;
    attributes.size.u32       = pPoolInfo->size;
    attributes.thMode.s32     = pPoolInfo->thMode;
    attributes.xoffSize.u32   = pPoolInfo->xoffSize;

    for (uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_BUFFER_POOL_ATTR_START + count;
        saiRetVal = xpSaiGetBufferPoolAttribute(&attributes, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferPoolStatisticsStoreDB

sai_status_t xpSaiBufferPoolStatisticsStoreDB(sai_object_id_t pool_id,
                                              xpSaiBufferPool_t **ppPoolInfo)
{
    sai_status_t           saiStatus = SAI_STATUS_SUCCESS;
    GT_STATUS              cpssStatus;
    uint32_t               current=0, max=0;
    GT_BOOL                isExtPool;
    xpSaiBufferPool_t      *pPoolInfo;

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(pool_id, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }
    *ppPoolInfo = pPoolInfo;

    isExtPool = (pPoolInfo->type == SAI_BUFFER_POOL_TYPE_INGRESS) ? GT_TRUE :
                GT_FALSE;

    /* Get the device from switch object */
    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(pool_id);

    /* get pool stats */
    cpssStatus = cpssHalPoolCountersGet(xpDevId, pPoolInfo->dynPoolId,
                                        isExtPool, &current, &max);
    if (cpssStatus != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Failed to get pool occupancy rc %d\n", cpssStatus);
        return cpssStatus2SaiStatus(cpssStatus);
    }
    if (max > pPoolInfo->watermark)
    {
        pPoolInfo->watermark = max;
    }
    pPoolInfo->curOccupancy = current;

    /* get pool headroom stats */
    max = 0;
    current = 0;
    cpssStatus = cpssHalPoolHeadroomCountersGet(xpDevId, pPoolInfo->dynPoolId,
                                                isExtPool, &current, &max);
    if (cpssStatus != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Failed to get pool headroom rc %d\n", cpssStatus);
        return cpssStatus2SaiStatus(cpssStatus);
    }
    if (max > pPoolInfo->headroomWatermark)
    {
        pPoolInfo->headroomWatermark = max;
    }
    pPoolInfo->headroomCurOccupancy = current;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiClearBufferPoolStatsDB

sai_status_t xpSaiClearBufferPoolStatsDB(uint32_t number_of_counters,
                                         const sai_stat_id_t *counter_ids,
                                         xpSaiBufferPool_t *pPoolInfo)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if (!counter_ids || !pPoolInfo)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t idx = 0; idx < number_of_counters; idx++)
    {
        switch (counter_ids[idx])
        {
            /* Clear current pool occupancy in bytes */
            case SAI_BUFFER_POOL_STAT_CURR_OCCUPANCY_BYTES:
                {
                    pPoolInfo->curOccupancy = 0;
                    break;
                }
            /* Clear watermark pool occupancy in bytes */
            case SAI_BUFFER_POOL_STAT_WATERMARK_BYTES:
                {
                    pPoolInfo->watermark = 0;
                    break;
                }
            /* Clear count of packest dropped in this pool */
            case SAI_BUFFER_POOL_STAT_DROPPED_PACKETS:
            /* Clear WRED green dropped packet count */
            case SAI_BUFFER_POOL_STAT_GREEN_WRED_DROPPED_PACKETS:
            /* Clear WRED green dropped byte count */
            case SAI_BUFFER_POOL_STAT_GREEN_WRED_DROPPED_BYTES:
            /* Clear WRED yellow dropped packet count */
            case SAI_BUFFER_POOL_STAT_YELLOW_WRED_DROPPED_PACKETS:
            /* Clear WRED yellow dropped byte count */
            case SAI_BUFFER_POOL_STAT_YELLOW_WRED_DROPPED_BYTES:
            /* Clear WRED red dropped packet count */
            case SAI_BUFFER_POOL_STAT_RED_WRED_DROPPED_PACKETS:
            /* Clear WRED red dropped byte count */
            case SAI_BUFFER_POOL_STAT_RED_WRED_DROPPED_BYTES:
            /* Clear WRED dropped packets count */
            case SAI_BUFFER_POOL_STAT_WRED_DROPPED_PACKETS:
            /* Clear WRED dropped bytes count */
            case SAI_BUFFER_POOL_STAT_WRED_DROPPED_BYTES:
            /* Clear WRED green marked packet count */
            case SAI_BUFFER_POOL_STAT_GREEN_WRED_ECN_MARKED_PACKETS:
            /* Clear WRED green marked byte count */
            case SAI_BUFFER_POOL_STAT_GREEN_WRED_ECN_MARKED_BYTES:
            /* Clear WRED yellow marked packet count */
            case SAI_BUFFER_POOL_STAT_YELLOW_WRED_ECN_MARKED_PACKETS:
            /* Clear WRED yellow marked byte count */
            case SAI_BUFFER_POOL_STAT_YELLOW_WRED_ECN_MARKED_BYTES:
            /* Clear WRED red marked packet count */
            case SAI_BUFFER_POOL_STAT_RED_WRED_ECN_MARKED_PACKETS:
            /* Clear WRED red marked byte count */
            case SAI_BUFFER_POOL_STAT_RED_WRED_ECN_MARKED_BYTES:
            /* Clear WRED marked packets count */
            case SAI_BUFFER_POOL_STAT_WRED_ECN_MARKED_PACKETS:
            /* Clear WRED marked bytes count */
            case SAI_BUFFER_POOL_STAT_WRED_ECN_MARKED_BYTES:
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[idx]);
                    break;
                }
            /* Clear current headroom pool occupancy in bytes */
            case SAI_BUFFER_POOL_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES:
                {
                    pPoolInfo->headroomCurOccupancy = 0;
                    break;
                }
            /* Clear watermark headroom pool in bytes */
            case SAI_BUFFER_POOL_STAT_XOFF_ROOM_WATERMARK_BYTES:
                {
                    pPoolInfo->headroomWatermark = 0;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Unknown attribute %d\n", counter_ids[idx]);
                }
        }
    }

    return retVal;
}


//Func: xpSaiGetBufferPoolStatsInReadClearMode

sai_status_t xpSaiGetBufferPoolStatsInReadClearMode(sai_object_id_t pool_id,
                                                    uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                                    uint64_t *counters, bool is_read_clear)
{
    xpSaiBufferPool_t *pPoolInfo = NULL;
    sai_status_t       saiStatus = SAI_STATUS_SUCCESS;


    if ((number_of_counters < 1) || (!counter_ids) || (!counters))
    {
        XP_SAI_LOG_ERR("%s invalid parameters received.\n", __FUNCNAME__);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Validate buffer pool object id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(pool_id, SAI_OBJECT_TYPE_BUFFER_POOL))
    {
        XP_SAI_LOG_ERR("Error: Invalid object type received (%u)\n",
                       xpSaiObjIdTypeGet(pool_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolStatisticsStoreDB(pool_id, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    for (uint32_t idx = 0; idx < number_of_counters; idx++)
    {
        switch (counter_ids[idx])
        {
            /* Get current pool occupancy in bytes */
            case SAI_BUFFER_POOL_STAT_CURR_OCCUPANCY_BYTES:
                {
                    counters[idx] = pPoolInfo->curOccupancy * XPSAI_BUFFER_PAGE_SIZE_BYTES;
                    break;
                }
            /* Get watermark pool occupancy in bytes */
            case SAI_BUFFER_POOL_STAT_WATERMARK_BYTES:
                {
                    counters[idx] = pPoolInfo->watermark * XPSAI_BUFFER_PAGE_SIZE_BYTES;
                    break;
                }
            /* Get count of packest dropped in this pool */
            case SAI_BUFFER_POOL_STAT_DROPPED_PACKETS:
            /* GetWRED green dropped packet count */
            case SAI_BUFFER_POOL_STAT_GREEN_WRED_DROPPED_PACKETS:
            /* Get WRED green dropped byte count */
            case SAI_BUFFER_POOL_STAT_GREEN_WRED_DROPPED_BYTES:
            /* Get WRED yellow dropped packet count */
            case SAI_BUFFER_POOL_STAT_YELLOW_WRED_DROPPED_PACKETS:
            /* Get WRED yellow dropped byte count */
            case SAI_BUFFER_POOL_STAT_YELLOW_WRED_DROPPED_BYTES:
            /* Get WRED red dropped packet count */
            case SAI_BUFFER_POOL_STAT_RED_WRED_DROPPED_PACKETS:
            /* Get WRED red dropped byte count */
            case SAI_BUFFER_POOL_STAT_RED_WRED_DROPPED_BYTES:
            /* Get WRED dropped packets count */
            case SAI_BUFFER_POOL_STAT_WRED_DROPPED_PACKETS:
            /* Get WRED dropped bytes count */
            case SAI_BUFFER_POOL_STAT_WRED_DROPPED_BYTES:
            /* Get WRED green marked packet count */
            case SAI_BUFFER_POOL_STAT_GREEN_WRED_ECN_MARKED_PACKETS:
            /* Get WRED green marked byte count */
            case SAI_BUFFER_POOL_STAT_GREEN_WRED_ECN_MARKED_BYTES:
            /* Get WRED yellow marked packet count */
            case SAI_BUFFER_POOL_STAT_YELLOW_WRED_ECN_MARKED_PACKETS:
            /* Get WRED yellow marked byte count */
            case SAI_BUFFER_POOL_STAT_YELLOW_WRED_ECN_MARKED_BYTES:
            /* Get WRED red marked packet count */
            case SAI_BUFFER_POOL_STAT_RED_WRED_ECN_MARKED_PACKETS:
            /* Get WRED red marked byte count */
            case SAI_BUFFER_POOL_STAT_RED_WRED_ECN_MARKED_BYTES:
            /* Get WRED marked packets count */
            case SAI_BUFFER_POOL_STAT_WRED_ECN_MARKED_PACKETS:
            /* Get WRED marked bytes count */
            case SAI_BUFFER_POOL_STAT_WRED_ECN_MARKED_BYTES:
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[idx]);
                    //saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(idx);
                    counters[idx] = 0;
                    break;
                }
            case SAI_BUFFER_POOL_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES:
                {
                    counters[idx] = pPoolInfo->headroomCurOccupancy * XPSAI_BUFFER_PAGE_SIZE_BYTES;
                    break;
                }
            case SAI_BUFFER_POOL_STAT_XOFF_ROOM_WATERMARK_BYTES:
                {
                    counters[idx] = pPoolInfo->headroomWatermark * XPSAI_BUFFER_PAGE_SIZE_BYTES;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Unknown attribute %d\n", counter_ids[idx]);
                    //saiStatus = SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(idx);
                }
        }
    }

    if (is_read_clear)
    {
        return xpSaiClearBufferPoolStatsDB(number_of_counters, counter_ids, pPoolInfo);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetBufferPoolStats

sai_status_t xpSaiGetBufferPoolStats(sai_object_id_t pool_id,
                                     uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                     uint64_t *counters)
{
    return xpSaiGetBufferPoolStatsInReadClearMode(pool_id, number_of_counters,
                                                  counter_ids, counters,
                                                  false);
}

//Func: xpSaiGetBufferPoolStatsExt

sai_status_t xpSaiGetBufferPoolStatsExt(sai_object_id_t pool_id,
                                        uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                        sai_stats_mode_t mode, uint64_t *counters)
{
    return xpSaiGetBufferPoolStatsInReadClearMode(pool_id, number_of_counters,
                                                  counter_ids, counters,
                                                  (mode == SAI_STATS_MODE_READ_AND_CLEAR));
}

//Func: xpSaiClearBufferPoolStats

sai_status_t xpSaiClearBufferPoolStats(sai_object_id_t pool_id,
                                       uint32_t number_of_counters,
                                       const sai_stat_id_t *counter_ids)
{
    xpSaiBufferPool_t *pPoolInfo = NULL;
    sai_status_t       saiStatus = SAI_STATUS_SUCCESS;

    if ((number_of_counters < 1) || (!counter_ids))
    {
        XP_SAI_LOG_ERR("%s invalid parameters received.\n", __FUNCNAME__);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Validate buffer pool object id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(pool_id, SAI_OBJECT_TYPE_BUFFER_POOL))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(pool_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* read from HW and store to XPS State DB */
    saiStatus = xpSaiBufferPoolStatisticsStoreDB(pool_id, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* clear specified type counter */
    saiStatus = xpSaiClearBufferPoolStatsDB(number_of_counters, counter_ids,
                                            pPoolInfo);

    return saiStatus;
}

//Func: xpSaiSetIngressPriorityGroupAttrBufferProfile

sai_status_t xpSaiSetIngressPriorityGroupAttrBufferProfile(
    sai_object_id_t ingress_pg_id, sai_attribute_value_t value)
{
    xpSaiIngressPg_t  *pIngressPgInfo = NULL;
    sai_object_id_t    profileObjId   = value.oid;
    sai_status_t       saiStatus      = SAI_STATUS_SUCCESS;

    /* Validate buffer profile object */
    saiStatus = xpSaiBufferProfileValidateProfileObject(profileObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to validate buffer profile 0x%lx, saiStatus: %d\n",
                       profileObjId, saiStatus);
        return saiStatus;
    }

    /* Get the ingress priority group info */
    saiStatus = xpSaiIngressPriorityGroupGetStateData(ingress_pg_id,
                                                      &pIngressPgInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to get state data for ingress pg 0x%lx, saiStatus: %d\n",
                       ingress_pg_id, saiStatus);
        return saiStatus;
    }

    if (pIngressPgInfo->bufferProfileId == profileObjId)
    {
        XP_SAI_LOG_DBG("Buffer profile 0x%lx already applied on ingress pg 0x%lx \n",
                       profileObjId, ingress_pg_id);
        return SAI_STATUS_SUCCESS;
    }

    if (pIngressPgInfo->bufferProfileId != SAI_NULL_OBJECT_ID)
    {
        /* Unbind ingress priority group from current buffer profile */
        saiStatus = xpSaiBufferRemoveIngressPgFromBufferProfile(
                        pIngressPgInfo->bufferProfileId, ingress_pg_id);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to unbind ingress priority group from buffer profile, xpStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    if (profileObjId != SAI_NULL_OBJECT_ID)
    {
        /* Bind ingress priority group to buffer profile */
        saiStatus = xpSaiBufferAddIngressPgToBufferProfile(profileObjId, ingress_pg_id);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind ingress priority group to buffer profile, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Configure buffer profile on ingress priority group */
    saiStatus = xpSaiIngressPriorityGroupApplyBufferProfile(ingress_pg_id,
                                                            profileObjId, pIngressPgInfo->bufferProfileId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        (void) xpSaiBufferRemoveObjIdFromBufferProfile(profileObjId, ingress_pg_id);
        XP_SAI_LOG_ERR("Error: Failed to apply buffer profile on ingress priority group, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Update state data */
    pIngressPgInfo->bufferProfileId = profileObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetIngressPriorityGroupAttr

sai_status_t xpSaiSetIngressPriorityGroupAttr(sai_object_id_t ingress_pg_id,
                                              const sai_attribute_t *attr)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    if (!attr)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Validate ingress priority group object */
    saiStatus = xpSaiIngressPriorityGroupValidateObject(ingress_pg_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Validation failed! sai rc: %" PRIi32 "\n", saiStatus);
        return saiStatus;
    }

    /* Validate attribute */
    saiStatus = xpSaiAttrCheck(1, attr,
                               INGRESS_PRIORITY_GROUP_VALIDATION_ARRAY_SIZE, ingress_priority_group_attribs,
                               SAI_COMMON_API_SET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute validation failed! saiError = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    switch (attr->id)
    {
        case SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE:
            {
                /* Buffer profile pointer */
                saiStatus = xpSaiSetIngressPriorityGroupAttrBufferProfile(ingress_pg_id,
                                                                          attr->value);
                if (SAI_STATUS_SUCCESS != saiStatus)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE)\n");
                    return saiStatus;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attribute %" PRIi32 " received\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(0);
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetIngressPriorityGroupAttr

sai_status_t xpSaiGetIngressPriorityGroupAttr(sai_object_id_t ingress_pg_id,
                                              uint32_t attr_count, sai_attribute_t *attr_list)
{
    xpSaiIngressPg_t *pIngressPgInfo = NULL;
    sai_status_t      saiStatus      = SAI_STATUS_SUCCESS;

    if (!attr_list)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Validate ingress priority group object */
    saiStatus = xpSaiIngressPriorityGroupValidateObject(ingress_pg_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to validate ingress priority group object! sai error %"
                       PRIi32 "\n", saiStatus);
        return saiStatus;
    }

    /* Validate attributes list*/
    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               INGRESS_PRIORITY_GROUP_VALIDATION_ARRAY_SIZE, ingress_priority_group_attribs,
                               SAI_COMMON_API_GET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attributes validation failed! saiError = %" PRIi32"\n",
                       saiStatus);
        return saiStatus;
    }

    /* Get the ingress priority group info */
    saiStatus = xpSaiIngressPriorityGroupGetStateData(ingress_pg_id,
                                                      &pIngressPgInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get State Data from DB! sai error %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    for (uint32_t count = 0; count < attr_count; ++count)
    {
        XP_SAI_LOG_DBG("HANDLE __GET INGRESS PG  attr->id %d on object 0x%08x%08x\n",
                       attr_list[count].id, LONG_INT_FORMAT(ingress_pg_id));

        switch (attr_list[count].id)
        {
            case SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE:
                {
                    /* buffer profile pointer [sai_object_id_t] */
                    attr_list[count].value.oid = pIngressPgInfo->bufferProfileId;
                    break;
                }
            case SAI_INGRESS_PRIORITY_GROUP_ATTR_PORT:
                {
                    attr_list[count].value.oid = pIngressPgInfo->portId;
                    break;
                }
            case SAI_INGRESS_PRIORITY_GROUP_ATTR_INDEX:
                {
                    attr_list[count].value.u8 = pIngressPgInfo->pgIndex;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Unknown attibute %" PRIi32 " received\n", attr_list[count].id);
                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(count);
                }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiIngressPriorityStatisticsStoreDB

sai_status_t xpSaiIngressPriorityStatisticsStoreDB(sai_object_id_t
                                                   ingress_pg_id,
                                                   xpSaiIngressPg_t **ppPriorityGrpInfo)
{
    GT_STATUS rc = GT_OK;
    sai_uint32_t countVal32bit = 0;
    sai_uint32_t maxVal32bit = 0;
    sai_uint64_t countVal64bit = 0;
    sai_uint64_t maxVal64bit = 0;
    sai_uint64_t countVal = 0;

    sai_status_t           saiStatus       = SAI_STATUS_SUCCESS;
    XP_STATUS              retVal          = XP_NO_ERR;
    xpsDevice_t            xpsDevId        = xpSaiObjIdSwitchGet(ingress_pg_id);
    sai_object_id_t        portId          = SAI_NULL_OBJECT_ID;
    xpSaiIngressPg_t      *pIngressPgInfo  = NULL;
    xpSaiPortQosInfo_t    *pPortQosInfo    = NULL;
    xpSaiQosMap_t         *pQosMapInfo     = NULL;
    xpsPort_t              xpsDevPort      = 0;
    sai_uint32_t           pfcTrafficClass = 0;
    xpSaiTcToPgMap_t      *pTcToPgMap      = NULL;
    sai_object_id_t        queue_id        = SAI_NULL_OBJECT_ID;


    /* Fetch device and port information for this particular port interface */
    xpSaiIngressPriorityGroupGetPortAndPfcTc(ingress_pg_id, &xpsDevPort,
                                             &pfcTrafficClass);

    /* Get the ingress priority group info */
    saiStatus = xpSaiIngressPriorityGroupGetStateData(ingress_pg_id,
                                                      &pIngressPgInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to get state data for ingress pg 0x%lx, saiStatus: %d\n",
                       ingress_pg_id, saiStatus);
        return saiStatus;
    }

    *ppPriorityGrpInfo = pIngressPgInfo;

    /*
     * get PG current/watermark
     */

    if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
    {
        GT_BOOL enable = GT_FALSE;

        rc = cpssHalPortPfcTcEnableGet(xpsDevId, xpsDevPort, pfcTrafficClass, &enable);
        if (rc == GT_OK && enable == GT_TRUE)
        {
            rc = cpssHalPortPfcCounterGet(xpsDevId, pfcTrafficClass,
                                          xpsDevPort /* in sip6 pfcCounterNum is used as physical port number */,
                                          &countVal32bit, &maxVal32bit);
            if (rc != GT_OK)
            {
                XP_SAI_LOG_ERR("Error: Failed to Get  port PFC counter rc %d\n", rc);
            }
            else
            {
                countVal64bit = countVal32bit;
                maxVal64bit   = maxVal32bit;
            }
        }
    }
    else
    {
        retVal = xpsQosFcGetPfcPortTcCounter(xpsDevId, xpsDevPort, pfcTrafficClass,
                                             (uint32_t*)&countVal);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get (SAI_INGRESS_PRIORITY_GROUP_STAT_CURR_OCCUPANCY_BYTES) | retVal = %d\n",
                           retVal);
        }
        else
        {
            countVal64bit = countVal;
            maxVal64bit   = 0;
        }
    }

    /* save counter to XPS state DB */
    pIngressPgInfo->curOccupancy = countVal64bit;
    if (maxVal64bit > pIngressPgInfo->watermark)
    {
        pIngressPgInfo->watermark = maxVal64bit;
    }


    /*
     * get PG pass/drop packets/bytes
     */

    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, xpsDevId, xpsDevPort,
                                 &portId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI port object could not be created, sai_error: %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    /* Get port qos state data */
    retVal = xpSaiGetPortQosInfo(portId, &pPortQosInfo);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get SAI Port QOS Info retVal:%d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (pPortQosInfo->tcToPgObjId == SAI_NULL_OBJECT_ID)
    {
        pTcToPgMap = NULL;
    }
    else
    {
        /* Check if qos map object exist */
        saiStatus = xpSaiQosMapGetStateData(pPortQosInfo->tcToPgObjId, &pQosMapInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Qos map does not exist\n");
            return saiStatus;
        }
        /* make sure that map type is TC2PG */
        if (pQosMapInfo->mapType != SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP)
        {
            XP_SAI_LOG_ERR("Error: Qos map type does not map \n");
            return SAI_STATUS_FAILURE;
        }
        pTcToPgMap = (xpSaiTcToPgMap_t*)pQosMapInfo->mapList;
    }

    if (pTcToPgMap == NULL)
    {
        xpSaiQueueQosInfo_t   *pQueueQosInfo   = NULL;
        sai_uint8_t tc =  pIngressPgInfo->pgIndex;

        /* if there are no TcToPgMap configuration in Port, PG priority = TC */

        saiStatus = xpSaiQosQueueObjIdCreate(SAI_OBJECT_TYPE_QUEUE, xpsDevPort, tc,
                                             &queue_id);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to create Queue object id for queue %d\n", tc);
            return saiStatus;
        }
        /* read from HW and store to XPS State DB */
        saiStatus = xpSaiQueueStatisticsStoreDB(queue_id, &pQueueQosInfo);
        if (SAI_STATUS_SUCCESS != saiStatus)
        {
            XP_SAI_LOG_ERR("Could not store the statistics for the port %u queue %u.\n",
                           xpsDevPort, tc);
            return saiStatus;
        }

        pIngressPgInfo->packets     += pQueueQosInfo->pg_passPkts;
        pIngressPgInfo->bytes       += pQueueQosInfo->pg_passBytes;
        pIngressPgInfo->dropPackets += pQueueQosInfo->pg_dropPkts;
        pIngressPgInfo->dropBytes   += pQueueQosInfo->pg_dropBytes;

        /* read clear */
        pQueueQosInfo->pg_passPkts  = 0;
        pQueueQosInfo->pg_passBytes = 0;
        pQueueQosInfo->pg_dropPkts  = 0;
        pQueueQosInfo->pg_dropBytes = 0;
    }
    else
    {
        for (uint32_t count = 0; count < pQosMapInfo->numEntries; count++)
        {
            xpSaiQueueQosInfo_t   *pQueueQosInfo   = NULL;

            /* find the TCs, which mapped to PG */
            if (pTcToPgMap[count].pg != pIngressPgInfo->pgIndex)
            {
                continue;
            }
            if (xpSaiQosQueueObjIdCreate(SAI_OBJECT_TYPE_QUEUE, xpsDevPort,
                                         pTcToPgMap[count].tc,
                                         &queue_id) != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to create Queue object id for queue %d\n",
                               pTcToPgMap[count].tc);
                continue;
            }
            /* read from HW and store to XPS State DB */
            saiStatus = xpSaiQueueStatisticsStoreDB(queue_id, &pQueueQosInfo);
            if (SAI_STATUS_SUCCESS != saiStatus)
            {
                XP_SAI_LOG_ERR("Could not store the statistics for the port %u queue %u.\n",
                               xpsDevPort, pTcToPgMap[count].tc);
                continue;
            }

            pIngressPgInfo->packets     += pQueueQosInfo->pg_passPkts;
            pIngressPgInfo->bytes       += pQueueQosInfo->pg_passBytes;
            pIngressPgInfo->dropPackets += pQueueQosInfo->pg_dropPkts;
            pIngressPgInfo->dropBytes   += pQueueQosInfo->pg_dropBytes;

            /* read-clear mode */
            pQueueQosInfo->pg_passPkts  = 0;
            pQueueQosInfo->pg_passBytes = 0;
            pQueueQosInfo->pg_dropPkts  = 0;
            pQueueQosInfo->pg_dropBytes = 0;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiClearIngressPriorityGroupStatsDB

sai_status_t xpSaiClearIngressPriorityGroupStatsDB(sai_object_id_t
                                                   ingress_pg_id, uint32_t number_of_counters,
                                                   const sai_stat_id_t * counter_ids, xpSaiIngressPg_t *pIngressPgInfo)
{
    for (uint32_t idx = 0; idx < number_of_counters; idx++)
    {
        switch (counter_ids[idx])
        {
            case SAI_INGRESS_PRIORITY_GROUP_STAT_PACKETS:
                {
                    pIngressPgInfo->packets = 0;
                    break;
                }
            case SAI_INGRESS_PRIORITY_GROUP_STAT_BYTES:
                {
                    pIngressPgInfo->bytes = 0;
                    break;
                }
            case SAI_INGRESS_PRIORITY_GROUP_STAT_CURR_OCCUPANCY_BYTES:
                {
                    pIngressPgInfo->curOccupancy = 0;
                    break;
                }
            case SAI_INGRESS_PRIORITY_GROUP_STAT_WATERMARK_BYTES:
                {
                    pIngressPgInfo->watermark = 0;
                    break;
                }
            case SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_CURR_OCCUPANCY_BYTES:
                {
                    break;
                }
            case SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_WATERMARK_BYTES:
                {
                    pIngressPgInfo->watermark = 0;
                    break;
                }
            case SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES:
                {
                    break;
                }
            case SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_WATERMARK_BYTES:
                {
                    break;
                }
            case SAI_INGRESS_PRIORITY_GROUP_STAT_DROPPED_PACKETS:
                {
                    pIngressPgInfo->dropPackets = 0;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Undefined attribute %d\n", counter_ids[idx]);
                    //saiStatus = SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(idx);
                }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetIngressPriorityGroupStatsInReadClearMode

sai_status_t xpSaiGetIngressPriorityGroupStatsInReadClearMode(
    sai_object_id_t ingress_pg_id,
    uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
    uint64_t *counters, bool is_read_clear)
{
    sai_status_t          saiStatus      = SAI_STATUS_SUCCESS;
    XP_STATUS             retVal         = XP_NO_ERR;
    xpsDevice_t           xpsDevId = xpSaiGetDevId();
    xpsPort_t             xpsDevPort;
    sai_uint32_t          pfcTrafficClass;
    sai_uint64_t          countVal       = 0;
    sai_uint32_t          countVal32bit  = 0;
    GT_STATUS             rc             = GT_OK;
    xpSaiIngressPg_t     *pIngressPgInfo = NULL;
    xpSaiBufferProfile_t *pProfileInfo   = NULL;
    sai_uint32_t          guarBuffer;

    if ((number_of_counters < 1) || (!counter_ids) || (!counters))
    {
        XP_SAI_LOG_ERR("%s invalid parameters received.\n", __FUNCNAME__);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(ingress_pg_id,
                                  SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(ingress_pg_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Fetch device and port information for this particular port interface */
    xpSaiIngressPriorityGroupGetPortAndPfcTc(ingress_pg_id, &xpsDevPort,
                                             &pfcTrafficClass);

    /* Get the ingress priority group info */
    saiStatus = xpSaiIngressPriorityGroupGetStateData(ingress_pg_id,
                                                      &pIngressPgInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to get state data for ingress pg 0x%lx, saiStatus: %d\n",
                       ingress_pg_id, saiStatus);
        return saiStatus;
    }

    /* Get the buffer profile state data */
    if (pIngressPgInfo->bufferProfileId != SAI_NULL_OBJECT_ID)
    {
        saiStatus = xpSaiBufferProfileGetStateData(pIngressPgInfo->bufferProfileId,
                                                   &pProfileInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_DBG("Failed to get state data for buffer profile 0x%lx, saiStatus: %d\n",
                           pIngressPgInfo->bufferProfileId, saiStatus);
            return saiStatus;
        }

        guarBuffer = pProfileInfo->bufferSize;
    }
    else
    {
        guarBuffer = 0;
    }

    guarBuffer = (guarBuffer /
                  XPSAI_BUFFER_PAGE_SIZE_BYTES);  // Convert to pages from bytes

    /* read from HW and store to XPS State DB */
    saiStatus = xpSaiIngressPriorityStatisticsStoreDB(ingress_pg_id,
                                                      &pIngressPgInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    for (uint32_t idx = 0; idx < number_of_counters; idx++)
    {
        counters[idx] = 0;
        switch (counter_ids[idx])
        {
            /* Get rx packets count */
            case SAI_INGRESS_PRIORITY_GROUP_STAT_PACKETS:
                {
                    counters[idx] = pIngressPgInfo->packets;
                    break;
                }
            /* Get rx bytes count */
            case SAI_INGRESS_PRIORITY_GROUP_STAT_BYTES:
                {
                    counters[idx] = pIngressPgInfo->bytes;
                    break;
                }
            /* Get current pg occupancy in bytes */
            case SAI_INGRESS_PRIORITY_GROUP_STAT_CURR_OCCUPANCY_BYTES:
                {
                    counters[idx] = pIngressPgInfo->curOccupancy * XPSAI_BUFFER_PAGE_SIZE_BYTES;
                    break;
                }
            /* Get watermark pg occupancy in bytes*/
            case SAI_INGRESS_PRIORITY_GROUP_STAT_WATERMARK_BYTES:
                {
                    counters[idx] = pIngressPgInfo->watermark * XPSAI_BUFFER_PAGE_SIZE_BYTES;
                    break;
                }
            /* Get current pg shared occupancy in bytes */
            case SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_CURR_OCCUPANCY_BYTES:
                {
                    if (pIngressPgInfo->curOccupancy > guarBuffer)
                    {
                        counters[idx] = (pIngressPgInfo->curOccupancy - guarBuffer) *
                                        XPSAI_BUFFER_PAGE_SIZE_BYTES;
                    }
                    break;
                }
            /* Get watermark pg shared occupancy in bytes */
            case SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_WATERMARK_BYTES:
                {
                    if (pIngressPgInfo->watermark > guarBuffer)
                    {
                        counters[idx] = (pIngressPgInfo->watermark - guarBuffer) *
                                        XPSAI_BUFFER_PAGE_SIZE_BYTES;
                    }
                    break;
                }
            /* Get current pg xoff room occupancy in bytes */
            case SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES:

                rc = cpssHalPortTcHeadroomCounterGet(xpsDevId, xpsDevPort, pfcTrafficClass,
                                                     &countVal32bit);
                if (rc != GT_OK)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES) | retVal = %d\n",
                                   rc);
                    return cpssStatus2SaiStatus(rc);
                }
                else
                {
                    counters[idx] = countVal32bit * XPSAI_BUFFER_PAGE_SIZE_BYTES;
                }

                break;

            /* Get watermark pg xoff room occupancy in bytes */
            case SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_WATERMARK_BYTES:
                // Not Clearing on read
                retVal = xpsQosFcGetMaxHeadroomCount(xpsDevId, xpsDevPort, pfcTrafficClass,
                                                     0 /*clear_on_read = false*/, &countVal);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES) | retVal = %d\n",
                                   retVal);
                    saiStatus = xpsStatus2SaiStatus(retVal);
                }
                else
                {
                    counters[idx] = countVal * XPSAI_BUFFER_PAGE_SIZE_BYTES;
                }
                break;

            /* Get dropped packets count */
            case SAI_INGRESS_PRIORITY_GROUP_STAT_DROPPED_PACKETS:
                {
                    counters[idx] = pIngressPgInfo->dropPackets;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Undefined attribute %d\n", counter_ids[idx]);
                    //saiStatus = SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(idx);
                }
        }
    }

    if (is_read_clear)
    {
        saiStatus = xpSaiClearIngressPriorityGroupStatsDB(ingress_pg_id,
                                                          number_of_counters, counter_ids, pIngressPgInfo);
    }

    return saiStatus;
}

//Func: xpSaiGetIngressPriorityGroupStats

sai_status_t xpSaiGetIngressPriorityGroupStats(sai_object_id_t ingress_pg_id,
                                               uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                               uint64_t *counters)
{
    return xpSaiGetIngressPriorityGroupStatsInReadClearMode(ingress_pg_id,
                                                            number_of_counters, counter_ids, counters,
                                                            false);
}

//Func: xpSaiGetIngressPriorityGroupStatsExt

sai_status_t xpSaiGetIngressPriorityGroupStatsExt(sai_object_id_t ingress_pg_id,
                                                  uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                                  sai_stats_mode_t mode, uint64_t *counters)
{
    return xpSaiGetIngressPriorityGroupStatsInReadClearMode(ingress_pg_id,
                                                            number_of_counters, counter_ids, counters,
                                                            (mode == SAI_STATS_MODE_READ_AND_CLEAR));
}

//Func: xpSaiClearIngressPriorityGroupStats

sai_status_t xpSaiClearIngressPriorityGroupStats(sai_object_id_t ingress_pg_id,
                                                 uint32_t number_of_counters, const sai_stat_id_t *counter_ids)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS retVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsPort_t xpsDevPort;
    sai_uint32_t pfcTrafficClass;
    sai_uint64_t countVal = 0;
    xpSaiIngressPg_t *pIngressPgInfo;

    if ((number_of_counters < 1) || (!counter_ids))
    {
        XP_SAI_LOG_ERR("%s invalid parameters received.\n", __FUNCNAME__);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(ingress_pg_id,
                                  SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(ingress_pg_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Fetch device and port information for this particular port interface */
    xpSaiIngressPriorityGroupGetPortAndPfcTc(ingress_pg_id, &xpsDevPort,
                                             &pfcTrafficClass);

    for (uint32_t idx = 0; idx < number_of_counters; idx++)
    {
        switch (counter_ids[idx])
        {
            /* Get current pg xoff room occupancy in bytes */
            case SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES:
                // Clearing on read
                retVal = xpsQosFcGetHeadroomCount(xpsDevId, xpsDevPort, pfcTrafficClass,
                                                  1 /*clear_on_read = true*/, &countVal);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to clear (SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES) | retVal = %d\n",
                                   retVal);
                    saiStatus = xpsStatus2SaiStatus(retVal);
                }
                break;

            /* Get watermark pg xoff room occupancy in bytes */
            case SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_WATERMARK_BYTES:
                // Clearing on read
                retVal = xpsQosFcGetMaxHeadroomCount(xpsDevId, xpsDevPort, pfcTrafficClass,
                                                     1 /*clear_on_read = true*/, &countVal);
                if (retVal  != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to clear (SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES) | retVal = %d\n",
                                   retVal);
                    saiStatus = xpsStatus2SaiStatus(retVal);
                }
                break;
            default:
                {
                    //XP_SAI_LOG_ERR("Undefined attribute %d\n", counter_ids[idx]);
                    //saiStatus = SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(idx);
                }
        }
    }

    /* read all from HW and store to XPS state DB */
    saiStatus = xpSaiIngressPriorityStatisticsStoreDB(ingress_pg_id,
                                                      &pIngressPgInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* clear specified type value in XPS state DB */
    xpSaiClearIngressPriorityGroupStatsDB(ingress_pg_id, number_of_counters,
                                          counter_ids, pIngressPgInfo);

    saiStatus = SAI_STATUS_SUCCESS;
    return saiStatus;
}

//Func: xpSaiCreateBufferProfile

sai_status_t xpSaiCreateBufferProfile(sai_object_id_t *buffer_profile_id,
                                      sai_object_id_t switch_id,
                                      uint32_t attr_count, const sai_attribute_t *attr_list)
{
    xpSaiBufferProfileAttributesT  attributes;
    xpSaiBufferProfile_t *pProfileInfo = NULL;
    xpSaiBufferPool_t    *pPoolInfo    = NULL;
    sai_status_t          saiStatus    = SAI_STATUS_SUCCESS;

    if (!buffer_profile_id)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Validate attributes */
    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               BUFFER_PROFILE_VALIDATION_ARRAY_SIZE,
                               buffer_profile_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    xpSaiSetDefaultBufferProfileAttributeVals(&attributes);
    xpSaiUpdateBufferProfileAttributeVals(attr_count, attr_list, &attributes);

    /* Validate buffer pool object */
    saiStatus = xpSaiBufferPoolValidatePoolObject(attributes.poolId.oid);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    /* Get the device from switch object */
    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(switch_id);

    /* Get the buffer pool state data */
    saiStatus = xpSaiBufferPoolGetStateData(attributes.poolId.oid, &pPoolInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("pool object is not exist \n");
        return saiStatus;
    }

    /* value valid */
    if ((pPoolInfo->type != SAI_BUFFER_POOL_TYPE_INGRESS) &&
        (attributes.xoffTh.u32 != 0 || attributes.xonTh.u32 != 0 ||
         attributes.xonOffsetTh.u32 != 0))
    {
        XP_SAI_LOG_ERR("Only egress pool can be set with Xoff or Xon \n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (pPoolInfo->type == SAI_BUFFER_POOL_TYPE_INGRESS &&
        attributes.xonOffsetTh.u32 != 0)
    {
        XP_SAI_LOG_ERR("not support XonOffset \n");
        return SAI_STATUS_NOT_SUPPORTED;
    }

    /* the xpSaiShellCompressAttributes() in the sai_create_buffer_profile_default()
     * or sai_create_buffer_profile_custom() will correct these.  */
    if (attributes.thMode.u32 == SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC &&
        attributes.sharedStaticTh.u32 != 0)
    {
        XP_SAI_LOG_ERR("sharedStaticTh must be 0 when profile is in dynamic mode\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (attributes.thMode.u32 == SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC &&
        attributes.sharedDynamicTh.s32 != 0)
    {
        XP_SAI_LOG_ERR("sharedDynamicTh must be 0 when profile is in static mode\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* value range valid */
    if (attributes.xoffTh.u32 + attributes.bufferSize.u32 +
        attributes.sharedStaticTh.u32 > pPoolInfo->size)
    {
        XP_SAI_LOG_ERR("xoff + reserve + shared_static > buffer pool size %u \n",
                       pPoolInfo->size);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (attributes.xonTh.u32 > pPoolInfo->size)
    {
        XP_SAI_LOG_ERR("xon > buffer pool size %u \n", pPoolInfo->size);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (attributes.xonOffsetTh.u32 > pPoolInfo->size)
    {
        XP_SAI_LOG_ERR("xon_offset > buffer pool size %u \n", pPoolInfo->size);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Create a buffer profile object */
    saiStatus = xpSaiBufferProfileCreateObject(xpDevId, buffer_profile_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not create buffer profile object, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Create state database */
    saiStatus = xpSaiBufferProfileCreateStateData(*buffer_profile_id,
                                                  &pProfileInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not create buffer profile state database, saiStatus: %d\n",
                       saiStatus);
        xpSaiBufferProfileRemoveObject(*buffer_profile_id);
        return saiStatus;
    }

    /* Update the state data base */
    pProfileInfo->poolObjId       = attributes.poolId.oid;
    pProfileInfo->bufferSize      = attributes.bufferSize.u32;
    pProfileInfo->thMode          = (sai_buffer_profile_threshold_mode_t)
                                    attributes.thMode.s32;
    pProfileInfo->sharedDynamicTh = attributes.sharedDynamicTh.s8;
    pProfileInfo->sharedStaticTh  = attributes.sharedStaticTh.u32;
    pProfileInfo->xoffTh          = attributes.xoffTh.u32;
    pProfileInfo->xonTh           = attributes.xonTh.u32;
    pProfileInfo->xonOffsetTh     = attributes.xonOffsetTh.u32;

    if (attributes.poolId.oid != SAI_NULL_OBJECT_ID)
    {
        /* Bind the profile to pool */
        saiStatus = xpSaiBufferBindProfileToPool(attributes.poolId.oid,
                                                 *buffer_profile_id);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind the profile to pool, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    XP_SAI_LOG_DBG("Successfully created buffer profile 0x%lx \n",
                   *buffer_profile_id);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveBufferProfile

sai_status_t xpSaiRemoveBufferProfile(sai_object_id_t buffer_profile_id)
{
    xpSaiBufferProfile_t *pProfileInfo = NULL;
    sai_status_t          saiStatus    = SAI_STATUS_SUCCESS;

    /* Check if buffer profile object is valid */
    if (!XDK_SAI_OBJID_TYPE_CHECK(buffer_profile_id,
                                  SAI_OBJECT_TYPE_BUFFER_PROFILE))
    {
        XP_SAI_LOG_ERR("Error: Invalid object type received (%u)\n",
                       xpSaiObjIdTypeGet(buffer_profile_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Check if buffer profile object exist */
    saiStatus = xpSaiBufferProfileGetStateData(buffer_profile_id, &pProfileInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Buffer profile does not exist\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (pProfileInfo->objCount != 0)
    {
        XP_SAI_LOG_ERR("Error: Buffer profile in use, unmap before remove\n");
        return SAI_STATUS_OBJECT_IN_USE;
    }

    if (pProfileInfo->poolObjId != SAI_NULL_OBJECT_ID)
    {
        /* Unbind the profile from pool */
        saiStatus = xpSaiBufferUnbindProfileFromPool(pProfileInfo->poolObjId,
                                                     buffer_profile_id);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to unbind the profile from pool, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Remove state database */
    saiStatus = xpSaiBufferProfileRemoveStateData(buffer_profile_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not remove buffer profile state database, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Remove buffer profile object */
    saiStatus = xpSaiBufferProfileRemoveObject(buffer_profile_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to remove buffer profile object, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    XP_SAI_LOG_DBG("Successfully removed buffer profile 0x%lx \n",
                   buffer_profile_id);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetBufferProfileAttribute

sai_status_t xpSaiSetBufferProfileAttribute(sai_object_id_t buffer_profile_id,
                                            const sai_attribute_t *attr)
{
    xpSaiBufferProfile_t *pProfileInfo = NULL;
    sai_status_t          saiStatus    = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("__SET BUFFER PROFILE attr->id %d on object 0x%08x%08x\n",
                   attr->id, LONG_INT_FORMAT(buffer_profile_id));

    /* Validate attributes */
    saiStatus = xpSaiAttrCheck(1, attr,
                               BUFFER_PROFILE_VALIDATION_ARRAY_SIZE,
                               buffer_profile_attribs,
                               SAI_COMMON_API_SET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    /* Validate buffer profile object type */
    if (!XDK_SAI_OBJID_TYPE_CHECK(buffer_profile_id,
                                  SAI_OBJECT_TYPE_BUFFER_PROFILE))
    {
        XP_SAI_LOG_ERR("Error: Invalid object type received (%u)\n",
                       xpSaiObjIdTypeGet(buffer_profile_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Validate buffer profile object */
    memset(&pProfileInfo, 0, sizeof(pProfileInfo));
    saiStatus = xpSaiBufferProfileGetStateData(buffer_profile_id, &pProfileInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Buffer profile does not exist\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    switch (attr->id)
    {
        case SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE:
            {
                /* Reserved buffer size in bytes */
                saiStatus = xpSaiSetBufferProfileAttrBufferSize(buffer_profile_id, attr->value);
                if (SAI_STATUS_SUCCESS != saiStatus)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE)\n");
                    return saiStatus;
                }
                break;
            }
        case SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE:
            {
                /* Shared threshold mode for the buffer profile
                 * If set, this overrides #SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE */
                XP_SAI_LOG_ERR("Set is Not Supported for attribute %d\n", attr->id);
                return SAI_STATUS_SUCCESS;

            }
        case SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH:
            {
                /* Dynamic threshold for the shared usage */
                saiStatus = xpSaiSetBufferProfileAttrSharedDynamicTh(buffer_profile_id,
                                                                     attr->value);
                if (SAI_STATUS_SUCCESS != saiStatus)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH)\n");
                    return saiStatus;
                }
                break;
            }
        case SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH:
            {
                /* Static threshold for the shared usage in bytes */
                saiStatus = xpSaiSetBufferProfileAttrSharedStaticTh(buffer_profile_id,
                                                                    attr->value);
                if (SAI_STATUS_SUCCESS != saiStatus)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH)\n");
                    return saiStatus;
                }
                break;
            }
        case SAI_BUFFER_PROFILE_ATTR_XOFF_TH:
            {
                /* Buffer profile XOFF threshold in bytes */
                saiStatus = xpSaiSetBufferProfileAttrXoffTh(buffer_profile_id, attr->value);
                if (SAI_STATUS_SUCCESS != saiStatus)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_BUFFER_PROFILE_ATTR_XOFF_TH)\n");
                    return saiStatus;
                }
                break;
            }
        case SAI_BUFFER_PROFILE_ATTR_XON_TH:
            {
                /* Buffer profile XON threshold in bytes */
                saiStatus = xpSaiSetBufferProfileAttrXonTh(buffer_profile_id, attr->value);
                if (SAI_STATUS_SUCCESS != saiStatus)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_BUFFER_PROFILE_ATTR_XON_TH)\n");
                    return saiStatus;
                }
                break;
            }
        case SAI_BUFFER_PROFILE_ATTR_XON_OFFSET_TH:
            {
                /* Buffer profile XON hysteresis threshold in byte */
                saiStatus = xpSaiSetBufferProfileAttrXonOffsetTh(buffer_profile_id,
                                                                 attr->value);
                if (SAI_STATUS_SUCCESS != saiStatus)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_BUFFER_PROFILE_ATTR_XON_OFFSET_TH)\n");
                    return saiStatus;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attribute %d received\n", attr->id);
                return (SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(0));
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetBufferProfileAttribute

static sai_status_t xpSaiGetBufferProfileAttribute(const
                                                   xpSaiBufferProfileAttributesT *attributes, sai_attribute_t *attr,
                                                   uint32_t attr_index)
{
    switch (attr->id)
    {
        case SAI_BUFFER_PROFILE_ATTR_POOL_ID:
            {
                attr->value = attributes->poolId;
                break;
            }
        case SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE:
            {
                attr->value = attributes->bufferSize;
                break;
            }
        case SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE:
            {
                attr->value = attributes->thMode;
                break;
            }
        case SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH:
            {
                attr->value = attributes->sharedDynamicTh;
                break;
            }
        case SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH:
            {
                attr->value = attributes->sharedStaticTh;
                break;
            }
        case SAI_BUFFER_PROFILE_ATTR_XOFF_TH:
            {
                attr->value = attributes->xoffTh;
                break;
            }
        case SAI_BUFFER_PROFILE_ATTR_XON_TH:
            {
                attr->value = attributes->xonTh;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Attribute %d is unknown\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index);
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetBufferProfileAttributes

static sai_status_t xpSaiGetBufferProfileAttributes(sai_object_id_t
                                                    buffer_profile_id, uint32_t attr_count, sai_attribute_t *attr_list)
{
    xpSaiBufferProfileAttributesT  attributes;
    xpSaiBufferProfile_t *pProfileInfo = NULL;
    sai_status_t          saiStatus    = SAI_STATUS_SUCCESS;

    if (!attr_list)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Validate buffer profile object type */
    if (!XDK_SAI_OBJID_TYPE_CHECK(buffer_profile_id,
                                  SAI_OBJECT_TYPE_BUFFER_PROFILE))
    {
        XP_SAI_LOG_ERR("Error: Invalid object type received (%u)\n",
                       xpSaiObjIdTypeGet(buffer_profile_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Validate buffer profile object value */
    saiStatus = xpSaiBufferProfileGetStateData(buffer_profile_id, &pProfileInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    memset(&attributes, 0, sizeof(xpSaiBufferProfileAttributesT));

    attributes.poolId.oid         = pProfileInfo->poolObjId;
    attributes.bufferSize.u32     = pProfileInfo->bufferSize;
    attributes.thMode.s32         = pProfileInfo->thMode;
    attributes.sharedDynamicTh.s8 = pProfileInfo->sharedDynamicTh;
    attributes.sharedStaticTh.u32 = pProfileInfo->sharedStaticTh;
    attributes.xoffTh.u32         = pProfileInfo->xoffTh;
    attributes.xonTh.u32          = pProfileInfo->xonTh;
    attributes.xonOffsetTh.u32    = pProfileInfo->xonOffsetTh;

    xpSaiUpdateAttrListBufferProfileVals(&attributes, attr_count, attr_list);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBulkGetBufferProfileAttributes

sai_status_t xpSaiBulkGetBufferProfileAttributes(sai_object_id_t id,
                                                 uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t         saiRetVal           = SAI_STATUS_SUCCESS;
    uint32_t             idx                 = 0;
    uint32_t             maxcount            = 0;
    xpSaiBufferProfile_t *pBufferProfileInfo = NULL;
    xpSaiBufferProfileAttributesT  attributes;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_BUFFER_PROFILE))
    {
        XP_SAI_LOG_ERR("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountBufferProfileAttribute(&maxcount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get max attribute count!\n");
        return SAI_STATUS_FAILURE;
    }

    if (*attr_count < maxcount)
    {
        *attr_count = maxcount;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    /* Get the buffer profile state data */
    saiRetVal = xpSaiBufferProfileGetStateData(id, &pBufferProfileInfo);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    memset(&attributes, 0, sizeof(xpSaiBufferProfileAttributesT));

    attributes.poolId.oid         = pBufferProfileInfo->poolObjId;
    attributes.bufferSize.u32     = pBufferProfileInfo->bufferSize;
    attributes.thMode.s32         = pBufferProfileInfo->thMode;
    attributes.sharedDynamicTh.s8 = pBufferProfileInfo->sharedDynamicTh;
    attributes.sharedStaticTh.u32 = pBufferProfileInfo->sharedStaticTh;
    attributes.xoffTh.u32         = pBufferProfileInfo->xoffTh;
    attributes.xonTh.u32          = pBufferProfileInfo->xonTh;
    attributes.xonOffsetTh.u32    = pBufferProfileInfo->xonOffsetTh;

    for (uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_BUFFER_PROFILE_ATTR_START + count;
        saiRetVal = xpSaiGetBufferProfileAttribute(&attributes, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBufferApiInit

XP_STATUS xpSaiBufferApiInit(uint64_t flag,
                             const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiBufferApiInit\n");

    _xpSaiBufferApi = (sai_buffer_api_t *) xpMalloc(sizeof(sai_buffer_api_t));
    if (NULL == _xpSaiBufferApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiBufferApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    /* Buffer pool APIs */
    _xpSaiBufferApi->create_buffer_pool   = xpSaiCreateBufferPool;
    _xpSaiBufferApi->remove_buffer_pool   = xpSaiRemoveBufferPool;
    _xpSaiBufferApi->set_buffer_pool_attribute = xpSaiSetBufferPoolAttribute;
    _xpSaiBufferApi->get_buffer_pool_attribute = xpSaiGetBufferPoolAttributes;

    /* Buffer pool stats APIs */
    _xpSaiBufferApi->get_buffer_pool_stats   = xpSaiGetBufferPoolStats;
    _xpSaiBufferApi->get_buffer_pool_stats_ext = xpSaiGetBufferPoolStatsExt;
    _xpSaiBufferApi->clear_buffer_pool_stats = xpSaiClearBufferPoolStats;

    /* Ingress priority group APIs */
    _xpSaiBufferApi->create_ingress_priority_group =
        xpSaiBufferCreateIngressPriorityGroupObject;
    _xpSaiBufferApi->remove_ingress_priority_group =
        xpSaiBufferRemoveIngressPriorityGroupObject;
    _xpSaiBufferApi->set_ingress_priority_group_attribute =
        xpSaiSetIngressPriorityGroupAttr;
    _xpSaiBufferApi->get_ingress_priority_group_attribute =
        xpSaiGetIngressPriorityGroupAttr;

    /* Ingress priority group stats APIs */
    _xpSaiBufferApi->get_ingress_priority_group_stats   =
        xpSaiGetIngressPriorityGroupStats;
    _xpSaiBufferApi->get_ingress_priority_group_stats_ext =
        xpSaiGetIngressPriorityGroupStatsExt;
    _xpSaiBufferApi->clear_ingress_priority_group_stats =
        xpSaiClearIngressPriorityGroupStats;

    /* Buffer profile APIs */
    _xpSaiBufferApi->create_buffer_profile   = xpSaiCreateBufferProfile;
    _xpSaiBufferApi->remove_buffer_profile   = xpSaiRemoveBufferProfile;
    _xpSaiBufferApi->set_buffer_profile_attribute = xpSaiSetBufferProfileAttribute;
    _xpSaiBufferApi->get_buffer_profile_attribute = xpSaiGetBufferProfileAttributes;

    saiRetVal = xpSaiApiRegister(SAI_API_BUFFER, (void*)_xpSaiBufferApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register Buffers API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}

//Func: xpSaiBufferApiDeinit

XP_STATUS xpSaiBufferApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiBufferApiDeinit\n");

    xpFree(_xpSaiBufferApi);
    _xpSaiBufferApi = NULL;

    return retVal;
}

sai_status_t xpSaiMaxCountBufferPoolAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_BUFFER_POOL_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiMaxCountBufferProfileAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_BUFFER_PROFILE_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountBufferPoolObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, bufferPoolDbHandle, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiCountBufferProfileObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, bufferProfileDbHandle, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetBufferPoolObjectList(uint32_t *object_count,
                                          sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;

    xpSaiBufferPool_t   *pBufferPoolNext    = NULL;

    saiRetVal = xpSaiCountBufferPoolObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get object count!\n");
        return saiRetVal;
    }

    if (objCount > *object_count)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Buffer overflow occured\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if (object_list == NULL)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *object_count = objCount;

    for (uint32_t i = 0; i < *object_count; i++)
    {
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, bufferPoolDbHandle,
                                     pBufferPoolNext, (void **)&pBufferPoolNext);
        if (retVal != XP_NO_ERR || pBufferPoolNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve bufferPool object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        object_list[i].key.object_id = pBufferPoolNext->poolObjId;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetBufferProfileObjectList(uint32_t *object_count,
                                             sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;

    xpSaiBufferProfile_t   *pBufferProfileNext    = NULL;

    saiRetVal = xpSaiCountBufferProfileObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get object count!\n");
        return saiRetVal;
    }

    if (objCount > *object_count)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Buffer overflow occured\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if (object_list == NULL)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *object_count = objCount;

    for (uint32_t i = 0; i < *object_count; i++)
    {
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, bufferProfileDbHandle,
                                     pBufferProfileNext, (void **)&pBufferProfileNext);
        if (retVal != XP_NO_ERR || pBufferProfileNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve bufferProfile object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        object_list[i].key.object_id = pBufferProfileNext->profileObjId;
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * @brief xpSaiBufferCreateIngressPriorityGroup
 *
 * @details This function is part of SAI Buffer API
 *
 * @param[out] ingress_priority_group_id Ingress priority group OID
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t xpSaiBufferCreateIngressPriorityGroupObject(
    sai_object_id_t *ingress_priority_group_id, sai_object_id_t switch_id,
    uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_attribute_value_t attrValue;
    xpSaiIngressPg_t      attributes;
    xpSaiIngressPg_t     *entry = NULL;
    sai_status_t          status       = SAI_STATUS_SUCCESS;
    XP_STATUS             retVal       = XP_NO_ERR;
    sai_object_id_t       ingPrioObjId = SAI_NULL_OBJECT_ID;
    uint32_t              ingPrioId    = 0;
    uint32_t              portNum      = 0;
    uint32_t              totalSupportedPrio = 0;
    xpDevice_t            xpDevId      = xpSaiObjIdSwitchGet(switch_id);

    memset(&attrValue, 0, sizeof(attrValue));
    memset(&attributes, 0, sizeof(attributes));

    /* Check incoming parameters */
    if (ingress_priority_group_id == NULL)
    {
        XP_SAI_LOG_ERR("ingress_priority_group_id is NULL!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(switch_id, SAI_OBJECT_TYPE_SWITCH))
    {
        XP_SAI_LOG_ERR("Wrong object type received (%" PRIu64 ")\n",
                       xpSaiObjIdTypeGet(switch_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    status = xpSaiAttrCheck(attr_count, attr_list,
                            INGRESS_PRIORITY_GROUP_VALIDATION_ARRAY_SIZE, ingress_priority_group_attribs,
                            SAI_COMMON_API_CREATE);
    if (status != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %" PRIi32 "\n", status);
        return status;
    }

    for (uint32_t i = 0; i < attr_count; ++i)
    {
        switch (attr_list[i].id)
        {
            case SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE:
                {
                    attributes.bufferProfileId = attr_list[i].value.oid;
                    break;
                }
            case SAI_INGRESS_PRIORITY_GROUP_ATTR_PORT:
                {
                    attributes.portId = attr_list[i].value.oid;
                    break;
                }
            case SAI_INGRESS_PRIORITY_GROUP_ATTR_INDEX:
                {
                    attributes.pgIndex = attr_list[i].value.u8;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Unkown attribute passed! attr.id = %" PRIi32 "\n",
                                   attr_list[i].id);
                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(i);
                }
        }
    }

    /* Get the port number from object id */
    portNum = (uint32_t) xpSaiObjIdValueGet(attributes.portId);

    /* Get the total supported pfc traffic class */
    if ((retVal = xpSaiPortGetNumPfcPrioritiesSupported(xpDevId,
                                                        &totalSupportedPrio)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get the number of pfc priorities supported |retVal: %d\n",
                       retVal);
        return retVal;
    }

    /*
     * Ingress priority group number is the absolute priority group number
     * that corresponds to port and pfc traffic class
     * Eg. absolute ingrtess priority group number for port-1 and pfc traffic
     * class-2 is ((1 * totalSupportedPrio) + 2)
     */
    ingPrioId = (portNum * totalSupportedPrio) + attributes.pgIndex;

    status = xpSaiObjIdCreate(SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP, xpDevId,
                              (sai_uint64_t) ingPrioId, &ingPrioObjId);
    if (status != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create sai object for Ingress Priority Group! status: %"
                       PRIi32 "\n", status);
        return status;
    }

    status = xpSaiIngressPriorityGroupCreateStateData(ingPrioObjId, &entry);
    if (status != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Creation of ingress priority group object failed! sai error = %"
                       PRIi32 "\n", status);
        return status;
    }

    entry->portId = attributes.portId;
    entry->pgIndex = attributes.pgIndex;
    entry->bufferProfileId = attributes.bufferProfileId;

    if (attributes.bufferProfileId != SAI_NULL_OBJECT_ID)
    {
        /* Bind ingress priority group to buffer profile */
        status = xpSaiBufferAddIngressPgToBufferProfile(attributes.bufferProfileId,
                                                        attributes.priorityGroupId);
        if (status != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to bind ingress priority group to buffer profile, status: %"
                           PRIi32 "\n", status);
            if (xpSaiIngressPriorityGroupRemoveStateData(ingPrioObjId) !=
                SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to free allocated entry in DB!\n");
            }
            return status;
        }
    }

    attrValue.oid = attributes.bufferProfileId;

    status = xpSaiSetIngressPriorityGroupAttrBufferProfile(ingPrioObjId, attrValue);
    if (status != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to apply buffer profile on ingress priority group, status: %"
                       PRIi32 "\n", status);
        if (xpSaiIngressPriorityGroupRemoveStateData(ingPrioObjId) !=
            SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to free allocated entry in DB!\n");
        }
        return status;
    }

    /* Bind PG to QoS Port DB */
    status = xpSaiPortBindIngressPriorityGrpToPort(attributes.portId, ingPrioObjId);
    if (status != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to bind PG to Port QoS DB! status %" PRIi32 "\n",
                       status);
        if (xpSaiIngressPriorityGroupRemoveStateData(ingPrioObjId) !=
            SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to free allocated entry in DB!\n");
        }
        return status;
    }

    *ingress_priority_group_id = ingPrioObjId;

    return status;
}

/**
 * @brief xpSaiBufferRemoveIngressPriorityGroup
 *
 * @details This function is part of SAI Buffer API
 *
 * @param[in] ingress_priority_group_id Ingress priority group OID
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t xpSaiBufferRemoveIngressPriorityGroupObject(
    sai_object_id_t ingress_priority_group_id)
{
    sai_attribute_value_t attrValue;
    sai_status_t          saiStatus         = SAI_STATUS_SUCCESS;
    xpSaiIngressPg_t     *pIngressPgInfo = NULL;

    if (!XDK_SAI_OBJID_TYPE_CHECK(ingress_priority_group_id,
                                  SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP))
    {
        XP_SAI_LOG_ERR("Couldn`t remove ingress priority group with invalid object type(SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP)!\n");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the ingress priority group info */
    saiStatus = xpSaiIngressPriorityGroupGetStateData(ingress_priority_group_id,
                                                      &pIngressPgInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove non existing ingress priority group from db, sai error: %"
                       PRIi32 "\n", saiStatus);
        return saiStatus;
    }

    attrValue.oid = SAI_NULL_OBJECT_ID;

    saiStatus = xpSaiSetIngressPriorityGroupAttrBufferProfile(
                    ingress_priority_group_id, attrValue);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to revert buffer profile on ingress priority group, status: %"
                       PRIi32 "\n", saiStatus);
        return saiStatus;
    }

    /* Unbind PG from QoS Port DB */
    saiStatus = xpSaiPortUnbindIngressPriorityGrpFromPort(pIngressPgInfo->portId,
                                                          pIngressPgInfo->priorityGroupId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to bind PG to Port QoS DB! status %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    /* Remove ingress priority group state data */
    saiStatus = xpSaiIngressPriorityGroupRemoveStateData(ingress_priority_group_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove State Data from DB! sai error %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    return saiStatus;
}

bool xpSaiBufferTotalThrottleGet(xpSaiBufferProfile_t *pProfileInfo)
{
    bool throttle = false;

    if (pProfileInfo&&0 == pProfileInfo->bufferSize)
    {
        if (SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC == pProfileInfo->thMode)
        {
            if (0==pProfileInfo->sharedStaticTh)
            {
                throttle = true;
            }
        }
    }

    return throttle;
}


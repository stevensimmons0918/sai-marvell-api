// xpSaiPort.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiPort.h"
#include "xpSaiSwitch.h"
#include "xpSaiDev.h"
#include "xpSaiLag.h"
#include "xpSaiPortCfgManager.h"
#include "saitypes.h"
#include "xpsXpImports.h"
#include "xpsMac.h"
#include "openXpsVlan.h"
#include "xpsSerdes.h"
#include "xpSaiValidationArrays.h"
#include "xpSaiUtil.h"
#include "xpSaiQueue.h"
#include "xpsCopp.h"
#include "xpsPort.h"
#include "xpSaiAclMapper.h"
#include "xpSaiStub.h"
#include "cpssDxChPortTx.h"
#include "cpssHalQos.h"
#include "xpSaiProfileMng.h"
#include "cpssHalUtil.h"


XP_SAI_LOG_REGISTER_API(SAI_API_PORT);

#define XP_SAI_PORT_PFC_DEFAULT_VALUE              0x00
#define XP_SAI_PORT_PFC_PRIORITIES_SUPPORTED       8
/* true - cnp rate limit enabled, false - disabled */
static bool xpSaiPortCnpRateLimitInfo[MAX_PORTNUM];

static sai_port_api_t* _xpSaiPortApi;

#define XP_SAI_LOG_LANES(laneList) \
{ \
    if (laneList->count == 1) \
    { \
        XP_SAI_LOG_NOTICE("Break-out 1x. Lanes: %u\n",    \
                        laneList->list[0]);             \
    } \
    else if (laneList->count == 2) \
    { \
        XP_SAI_LOG_INFO("Break-out 2x. Lanes: %u, %u\n",        \
                        laneList->list[0], laneList->list[1]);  \
    } \
    else if (laneList->count == 4) \
    { \
        XP_SAI_LOG_NOTICE("Break-out 4x. Lanes: %u, %u, %u, %u\n",\
                        laneList->list[0], laneList->list[1],   \
                        laneList->list[2], laneList->list[3]);  \
    } \
    else if (laneList->count == 8) \
    { \
        XP_SAI_LOG_NOTICE("Break-out 8x. Lanes: %u, %u, %u, %u, %u, %u, %u, %u\n",\
                        laneList->list[0], laneList->list[1],   \
                        laneList->list[2], laneList->list[3],   \
                        laneList->list[4], laneList->list[5],   \
                        laneList->list[6], laneList->list[7]);   \
    } \
    else \
    { \
        XP_SAI_LOG_NOTICE("Unknown break-out mode. Number of lanes: %u\n",    \
                        laneList->count);                                   \
    } \
 }

typedef struct _xpSaiPortAttributesT
{
    sai_attribute_value_t type;
    sai_attribute_value_t operStatus;
    sai_attribute_value_t supportedBreakoutModeType;
    sai_attribute_value_t currentBreakoutModeType;
    sai_attribute_value_t qosNumberOfQueues;
    sai_attribute_value_t qosQueueList;
    sai_attribute_value_t qosNumberOfSchedulerGroups;
    sai_attribute_value_t qosSchedulerGroupList;
    sai_attribute_value_t supportedSpeed;
    sai_attribute_value_t supportedHalfDuplexSpeed;
    sai_attribute_value_t supportedAutoNegMode;
    sai_attribute_value_t supportedFlowControlMode;
    sai_attribute_value_t supportedAsymmetricPauseMode;
    sai_attribute_value_t supportedMediaType;
    sai_attribute_value_t remoteAdvertisedSpeed;
    sai_attribute_value_t remoteAdvertisedHalfDuplexSpeed;
    sai_attribute_value_t remoteAdvertisedAutoNegMode;
    sai_attribute_value_t remoteAdvertisedFlowControlMode;
    sai_attribute_value_t remoteAdvertisedAsymmetricPauseMode;
    sai_attribute_value_t remoteAdvertisedMediaType;
    sai_attribute_value_t numberOfIngressPriorityGroups;
    sai_attribute_value_t ingressPriorityGroupList;
    sai_attribute_value_t hwLaneList;
    sai_attribute_value_t speed;
    sai_attribute_value_t intfType;
    sai_attribute_value_t fullDuplexMode;
    sai_attribute_value_t autoNegMode;
    sai_attribute_value_t adminState;
    sai_attribute_value_t mediaType;
    sai_attribute_value_t advertisedSpeed;
    sai_attribute_value_t advertisedHalfDuplexSpeed;
    sai_attribute_value_t advertisedAutoNegMode;
    sai_attribute_value_t advertisedFlowControlMode;
    sai_attribute_value_t advertisedAsymmetricPauseMode;
    sai_attribute_value_t advertisedMediaType;
    sai_attribute_value_t portVlanId;
    sai_attribute_value_t defaultVlanPriority;
    sai_attribute_value_t ingressFiltering;
    sai_attribute_value_t dropUntagged;
    sai_attribute_value_t dropTagged;
    sai_attribute_value_t internalLoopbackMode;
    sai_attribute_value_t fdbLearningMode;
    sai_attribute_value_t updateDscp;
    sai_attribute_value_t mtu;
    sai_attribute_value_t floodStormControlPolicerId;
    sai_attribute_value_t broadcastStormControlPolicerId;
    sai_attribute_value_t multicastStormControlPolicerId;
    sai_attribute_value_t globalFlowControlMode;
    sai_attribute_value_t maxLearnedAddresses;
    sai_attribute_value_t fdbLearningLimitViolationPacketAction;
    sai_attribute_value_t ingressMirrorSession;
    sai_attribute_value_t egressMirrorSession;
    sai_attribute_value_t ingressSamplepacketEnable;
    sai_attribute_value_t egressSamplepacketEnable;
    sai_attribute_value_t policerId;
    sai_attribute_value_t qosDefaultTc;
    sai_attribute_value_t qosDot1pToTcMap;
    sai_attribute_value_t qosDot1pToColorMap;
    sai_attribute_value_t qosDscpToTcMap;
    sai_attribute_value_t qosDscpToColorMap;
    sai_attribute_value_t qosTcToQueueMap;
    sai_attribute_value_t qosTcAndColorToDot1pMap;
    sai_attribute_value_t qosTcAndColorToDscpMap;
    sai_attribute_value_t qosTcToPriorityGroupMap;
    sai_attribute_value_t qosPfcPriorityToPriorityGroupMap;
    sai_attribute_value_t qosPfcPriorityToQueueMap;
    sai_attribute_value_t qosWredProfileId;
    sai_attribute_value_t qosSchedulerProfileId;
    sai_attribute_value_t qosIngressBufferProfileList;
    sai_attribute_value_t qosEgressBufferProfileList;
    sai_attribute_value_t priorityFlowControl;
    sai_attribute_value_t metaData;
    sai_attribute_value_t egressBlockPortList;
    sai_attribute_value_t hwProfileId;
    sai_attribute_value_t eeeEnable;
    sai_attribute_value_t eeeIdleTime;
    sai_attribute_value_t eeeWakeTime;
    sai_attribute_value_t fecMode;
    sai_attribute_value_t txEnable;
    sai_attribute_value_t pvidModeAllPkts;
    sai_attribute_value_t egressTimestamp;
    sai_attribute_value_t ingAclOid;
    sai_attribute_value_t egrAclOid;
} xpSaiPortAttributesT;

/*This has been added to model behaviour added as part of LAG membership model,
  where each some attributes of port in the LAG may be configured separately(VLAN assagned, speed, MTU etc...).
  It is assumed SAI client application will take care of consistency.*/
xpsDbHandle_t portLagDbHandle = XPSAI_PORT_LAG_DB_HNDL;
extern xpsDbHandle_t portLagPortCountDbHandle;
static xpsDbHandle_t portQosDbHandle = XPSAI_PORT_QOS_DB_HNDL;
sai_status_t xpSaiSetPortAttrAdvertisedSpeed(sai_object_id_t port_id,
                                             sai_attribute_value_t value);
/**
 * \brief Port State Database Handle
 *
 */
static xpsDbHandle_t gXpSaiPortDbHandle = XPSAI_PORT_DB_HNDL;
static xpsDbHandle_t gXpSaiPortStatisticDbHandle = XPSAI_PORT_STATISTIC_DB_HNDL;
extern bool gResetInProgress;
extern bool WARM_RESTART;
#ifdef __cplusplus
extern "C" {
#endif
pthread_t gSaiPortThread = (pthread_t)NULL;
#ifdef __cplusplus
}
#endif

extern void xpSaiPortCnpRateLimitParamSet(xpsDevice_t devId, uint32_t portNum,
                                          bool enabled)
{
    xpSaiPortCnpRateLimitInfo[portNum] = enabled;
}

extern XP_STATUS xpSaiProfileMngSyncPortToProfile
(
    xpsDevice_t  xpsDevId,
    sai_uint32_t portNum
);

static pthread_mutex_t gSaiPortDbMutex = PTHREAD_MUTEX_INITIALIZER;

static sai_status_t xpSaiPortPriorityFlowControlUpdateHW(xpsDevice_t xpsDevId,
                                                         xpsInterfaceId_t xpsInf, uint8_t pfcVector, uint8_t vectorMask,
                                                         pfcVectorUpdateType updateType);
static sai_status_t xpSaiPortPriorityFlowControlGet(xpsInterfaceId_t xpsInf,
                                                    sai_uint8_t *pfcVectorPtr, pfcVectorUpdateType updateType);
static sai_status_t xpSaiPortPriorityFlowControlSet(xpsInterfaceId_t xpsInf,
                                                    sai_uint8_t pfcVector, pfcVectorUpdateType updateType);

//Func: xpSaiPortKeyComp

static int32_t xpSaiPortKeyComp(void *key1, void *key2)
{
    return ((int32_t)(((xpSaiPortDbEntryT*)key1)->keyIntfId) - (int32_t)(((
                                                                              xpSaiPortDbEntryT*)key2)->keyIntfId));
}

//Func: xpSaiPortStatisticKeyComp

static int32_t xpSaiPortStatisticKeyComp(void *key1, void *key2)
{
    return ((int32_t)(((xpSaiPortStatisticDbEntryT*)key1)->portNum) - (int32_t)(((
            xpSaiPortStatisticDbEntryT*)key2)->portNum));
}

//Func: xpSaiPortDbInsert

sai_status_t xpSaiPortDbInsert(xpsInterfaceId_t xpsIntf)
{
    XP_STATUS         retVal      = XP_NO_ERR;
    xpSaiPortDbEntryT *pPortEntry = NULL;
    uint32_t i = 0;

    retVal = xpsStateHeapMalloc(sizeof(xpSaiPortDbEntryT), (void**)&pPortEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate port structure\n");
        return xpsStatus2SaiStatus(retVal);
    }

    memset(pPortEntry, 0, sizeof(xpSaiPortDbEntryT));

    pthread_mutex_lock(&gSaiPortDbMutex);

    pPortEntry->keyIntfId = xpsIntf;
    pPortEntry->pfc = XP_SAI_PORT_PFC_DEFAULT_VALUE;
    pPortEntry->pfcTx = XP_SAI_PORT_PFC_DEFAULT_VALUE;
    pPortEntry->pfcRx = XP_SAI_PORT_PFC_DEFAULT_VALUE;
    pPortEntry->combinedMode = true;
    pPortEntry->mediaType = SAI_PORT_MEDIA_TYPE_NOT_PRESENT;
    pPortEntry->pvidUserSetting = XPSAI_DEFAULT_VLAN_ID;
    //pPortEntry->ingressAclId = (uint64_t)SAI_OBJECT_TYPE_ACL_ENTRY << 48 | 0ULL << 40 | 0xffffffffff ;
    for (i = 0; i < XPSAI_DIRECTED_MIRROR_SESSION_NUM; i++)
    {
        pPortEntry->ingressMirrorSessionOid[i] = SAI_NULL_OBJECT_ID;
        pPortEntry->egressMirrorSessionOid[i] = SAI_NULL_OBJECT_ID;
    }
    pPortEntry->ingressSampleSessionOid = SAI_NULL_OBJECT_ID;

    pPortEntry->blockedPortList.size = 0;

    retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, gXpSaiPortDbHandle,
                                (void*)pPortEntry);

    pthread_mutex_unlock(&gSaiPortDbMutex);

    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not insert trap structure into DB, xpsIntf %u, error %d\n",
                       xpsIntf, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiPortDbDelete

sai_status_t xpSaiPortDbDelete(xpsInterfaceId_t xpsIntf,
                               xpSaiPortDbEntryT **pPortEntry)
{
    XP_STATUS         retVal      = XP_NO_ERR;
    xpSaiPortDbEntryT key;

    memset(&key, 0, sizeof(key));

    key.keyIntfId = xpsIntf;

    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gXpSaiPortDbHandle, &key,
                                (void**)pPortEntry);

    if (pPortEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find Port entry for portIntf %u in DB", xpsIntf);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, gXpSaiPortDbHandle,
                                (xpsDbKey_t)&key, (void**)pPortEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not delete entry from DB, xpsIntf %u, error %d\n",
                       xpsIntf, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    // Free the memory allocated for the corresponding state
    if ((retVal = xpsStateHeapFree((void*)*pPortEntry)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not free heap memory for, xpsIntf %u, error %d\n",
                       xpsIntf, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostIntfGroupDbInfoGet

sai_status_t xpSaiPortDbInfoGet(xpsInterfaceId_t xpsIntf,
                                xpSaiPortDbEntryT **ppPortEntry)
{
    XP_STATUS     retVal     = XP_NO_ERR;

    xpSaiPortDbEntryT key;

    memset(&key, 0, sizeof(key));

    key.keyIntfId = xpsIntf;

    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gXpSaiPortDbHandle, &key,
                                (void**)ppPortEntry);

    if (*ppPortEntry == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiPortDbInfoGetNext

sai_status_t xpSaiPortDbInfoGetNext(xpSaiPortDbEntryT *pPortEntry,
                                    xpSaiPortDbEntryT **ppPortEntry)
{
    XP_STATUS           retVal      = XP_NO_ERR;

    retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, gXpSaiPortDbHandle, pPortEntry,
                                 (void**)ppPortEntry);

    if (*ppPortEntry == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiPortDbBlockedPortListGet
sai_status_t xpSaiPortDbBlockedPortListGet(xpsInterfaceId_t xpsIntf,
                                           xpsPortList_t *blockedPortList)
{
    sai_status_t        saiStatus = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT   *dbEntry  = NULL;

    saiStatus = xpSaiPortDbInfoGet(xpsIntf, &dbEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB\n");
        return saiStatus;
    }

    pthread_mutex_lock(&gSaiPortDbMutex);
    *blockedPortList = dbEntry->blockedPortList;
    pthread_mutex_unlock(&gSaiPortDbMutex);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiPortDbBlockedPortListSet
sai_status_t xpSaiPortDbBlockedPortListSet(xpsInterfaceId_t xpsIntf,
                                           xpsPortList_t blockedPortList)
{
    sai_status_t        saiStatus = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT   *dbEntry  = NULL;

    saiStatus = xpSaiPortDbInfoGet(xpsIntf, &dbEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB\n");
        return saiStatus;
    }

    pthread_mutex_lock(&gSaiPortDbMutex);
    dbEntry->blockedPortList = blockedPortList;
    pthread_mutex_unlock(&gSaiPortDbMutex);

    return SAI_STATUS_SUCCESS;
}

// Func: xpSaiPortPriorityFlowControlSet

static sai_status_t xpSaiPortPriorityFlowControlSet(xpsInterfaceId_t xpsInf,
                                                    sai_uint8_t pfcVector, pfcVectorUpdateType updateType)
{
    sai_status_t        saiStatus = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT   *dbEntry  = NULL;

    saiStatus = xpSaiPortDbInfoGet(xpsInf, &dbEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB\n");
        return saiStatus;
    }

    pthread_mutex_lock(&gSaiPortDbMutex);
    switch (updateType)
    {
        case pfcVectorUpdateTypeTx:
            dbEntry->pfcTx = pfcVector;
            break;
        case pfcVectorUpdateTypeRx:
            dbEntry->pfcRx = pfcVector;
            break;
        case pfcVectorUpdateTypeCombined:
        default:
            dbEntry->pfc= pfcVector;
            dbEntry->pfcRx = pfcVector;
            dbEntry->pfcTx = pfcVector;
            break;
    }

    pthread_mutex_unlock(&gSaiPortDbMutex);

    return SAI_STATUS_SUCCESS;
}


static sai_status_t xpSaiPortPriorityFlowControlModeSet(xpsInterfaceId_t xpsInf,
                                                        bool combined)
{
    sai_status_t        saiStatus = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT   *dbEntry  = NULL;

    saiStatus = xpSaiPortDbInfoGet(xpsInf, &dbEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB\n");
        return saiStatus;
    }

    pthread_mutex_lock(&gSaiPortDbMutex);
    dbEntry->combinedMode= combined;
    pthread_mutex_unlock(&gSaiPortDbMutex);

    return SAI_STATUS_SUCCESS;
}


static sai_status_t xpSaiPortPriorityFlowControlModeGet(xpsInterfaceId_t xpsInf,
                                                        bool * combinedPtr)
{
    sai_status_t        saiStatus = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT   *dbEntry  = NULL;

    saiStatus = xpSaiPortDbInfoGet(xpsInf, &dbEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB\n");
        return saiStatus;
    }

    pthread_mutex_lock(&gSaiPortDbMutex);
    *combinedPtr = dbEntry->combinedMode;
    pthread_mutex_unlock(&gSaiPortDbMutex);

    return SAI_STATUS_SUCCESS;
}



// Func: xpSaiPortPriorityFlowControlGet

static sai_status_t xpSaiPortPriorityFlowControlGet(xpsInterfaceId_t xpsInf,
                                                    sai_uint8_t *pfcVectorPtr, pfcVectorUpdateType updateType)
{
    sai_status_t        saiStatus = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT   *dbEntry  = NULL;

    saiStatus = xpSaiPortDbInfoGet(xpsInf, &dbEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB\n");
        return saiStatus;
    }

    pthread_mutex_lock(&gSaiPortDbMutex);
    switch (updateType)
    {
        case pfcVectorUpdateTypeTx:
            *pfcVectorPtr = dbEntry->pfcTx;
            break;
        case pfcVectorUpdateTypeRx:
            *pfcVectorPtr = dbEntry->pfcRx ;
            break;
        case pfcVectorUpdateTypeCombined:
        default:
            *pfcVectorPtr = dbEntry->pfc;
            break;
    }
    pthread_mutex_unlock(&gSaiPortDbMutex);

    return SAI_STATUS_SUCCESS;
}

// Func: xpSaiPortPriorityFlowControlUpdateHW

static sai_status_t xpSaiPortPriorityFlowControlUpdateHW
(
    xpsDevice_t xpsDevId,
    xpsInterfaceId_t xpsInf,
    uint8_t pfcVector,
    uint8_t vectorMask,
    pfcVectorUpdateType updateType
)
{
    XP_STATUS    retVal    = XP_NO_ERR;
    sai_uint8_t  pfcEnable = 0, i;
    GT_STATUS    rc;
    sai_status_t        saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS xpStatus = XP_NO_ERR;


    for (i=0; i<XP_SAI_PORT_PFC_PRIORITIES_SUPPORTED; i++)
    {
        if ((1<<i)&vectorMask)
        {
            if (updateType == pfcVectorUpdateTypeTx||
                updateType == pfcVectorUpdateTypeCombined)
            {
                /*handle TX*/
                rc = cpssHalPortPfcTcEnableSet(xpsDevId, xpsInf, i,
                                               ((1<<i)&pfcVector)?GT_TRUE:GT_FALSE);
                if (rc != GT_OK)
                {
                    XP_SAI_LOG_ERR("Error: Couldn't  enable pfc on port: %d tc %d\n", xpsInf, i);
                    return  cpssStatus2SaiStatus(rc);
                }
            }

            if (updateType == pfcVectorUpdateTypeRx||
                updateType == pfcVectorUpdateTypeCombined)
            {
                /*handle RX*/
                rc = cpssHalPortPfcTcResponseEnableSet(xpsDevId, xpsInf, i,
                                                       ((1<<i)&pfcVector)?GT_TRUE:GT_FALSE);
                if (rc != GT_OK)
                {
                    XP_SAI_LOG_ERR("Error: Couldn't  enable pfc on port: %d tc %d\n", xpsInf, i);
                    return  cpssStatus2SaiStatus(rc);
                }
            }
        }
    }

    /* Enable/disable  Priority Flow Control */
    pfcEnable = !!pfcVector;
    switch (updateType)
    {
        case pfcVectorUpdateTypeTx:

            if (pfcEnable==0)
            {
                /*check other side*/
                saiStatus = xpSaiPortPriorityFlowControlGet(xpsInf, &pfcVector,
                                                            pfcVectorUpdateTypeRx);
                if (saiStatus != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiPriorityFlowControlGet failed with an error %d\n",
                                   saiStatus);
                    return saiStatus;
                }

                if (pfcVector)
                {
                    pfcEnable=1;
                }
            }

            break;
        case pfcVectorUpdateTypeRx:

            if (pfcEnable==0)
            {
                /*check other side*/
                saiStatus = xpSaiPortPriorityFlowControlGet(xpsInf, &pfcVector,
                                                            pfcVectorUpdateTypeTx);
                if (saiStatus != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiPriorityFlowControlGet failed with an error %d\n",
                                   saiStatus);
                    return saiStatus;
                }

                if (pfcVector)
                {
                    pfcEnable=1;
                }
            }
            break;
        case pfcVectorUpdateTypeCombined:
        default:
            break;
    }

    rc = cpssHalPortFlowControlModeSet(xpsDevId, (uint8_t) xpsInf,
                                       pfcEnable?CPSS_DXCH_PORT_FC_MODE_PFC_E:CPSS_DXCH_PORT_FC_MODE_DISABLE_E);

    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Failed to handle fc mode setting|Port:%d retVal:%d\n", xpsInf,
                       retVal);
        return  cpssStatus2SaiStatus(rc);
    }

    /* Enable/Disable PFC PCL rule per port. */
    xpStatus = xpsAclPFCRuleUpdate(xpsDevId, xpsInf, pfcEnable?false:true);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("xpsAclPFCRuleUpdate Failed : %u", xpsInf);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

static XP_SPEED xpSaiPortSpeedToXpsPgSpeed(uint32_t speed)
{
    switch (speed)
    {
        case XPSAI_PORT_SPEED_10MB:
        case XPSAI_PORT_SPEED_100MB:
        case XPSAI_PORT_SPEED_1000MB:
            return XP_PG_SPEED_1G;

        case XPSAI_PORT_SPEED_10G:
            return XP_PG_SPEED_10G;

        case XPSAI_PORT_SPEED_25G:
            return XP_PG_SPEED_25G;

        case XPSAI_PORT_SPEED_40G:
            return XP_PG_SPEED_40G;

        case XPSAI_PORT_SPEED_50G:
            return XP_PG_SPEED_50G;

        case XPSAI_PORT_SPEED_100G:
            return XP_PG_SPEED_100G;

        default:
            return XP_PG_SPEED_MISC;
    }
}

//Func: xpSaiPortThreadCb
void* xpSaiPortThreadCb(void *pArg)
{
    uint32_t    sleepTimeMs = 100;
    xpsDevice_t xpsDevId    = *((xpsDevice_t*)pArg);
    bool done = false;
    XP_SAI_LOG_DBG("SAI Port thread is started");
    {
        sleepTimeMs = xpSaiPortCycleTimeGet();
    }
    xpSaiSwitchThreadsWaitForInit();

    xpSaiSleepMsec(1000);
    while (!done)
    {
        if (gResetInProgress)
        {
            gSaiPortThread = 0;
            break;
        }

        if (xpSaiGetExitStatus())
        {
            break;
        }

        xpSaiSleepMsec(sleepTimeMs);

        xpSaiPortCfgMgrStateUpdate(xpsDevId, sleepTimeMs);
    }
    XP_SAI_LOG_DBG("SAI Port thread is finished");
    return NULL;
}

//Func: xpSaiPortQosKeyComp

static sai_int32_t xpSaiPortQosKeyComp(void *key1, void *key2)
{
    if ((((xpSaiPortQosInfo_t*)key1)->portId) > (((xpSaiPortQosInfo_t*)
                                                  key2)->portId))
    {
        return 1;
    }
    else if ((((xpSaiPortQosInfo_t*)key1)->portId) < (((xpSaiPortQosInfo_t*)
                                                       key2)->portId))
    {
        return -1;
    }
    return 0;
}

//Func: xpSaiPortLagKeyComp

static sai_int32_t xpSaiPortLagKeyComp(void *key1, void *key2)
{
    return ((((xpSaiPortLagInfo_t*)key1)->intfId) - (((xpSaiPortLagInfo_t*)
                                                      key2)->intfId));
}

//Func: xpSaiSetPortAttrFdbLearningModeHandle

static sai_status_t xpSaiSetPortAttrFdbLearningModeHandle(
    xpsInterfaceId_t xpsIntf, xpsPktCmd_e pktCmd)
{
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    XP_STATUS xpStatus = XP_NO_ERR;

    xpStatus = (XP_STATUS)xpsPortSetField(xpsDevId, xpsIntf, XPS_PORT_MAC_SAMISSCMD,
                                          pktCmd);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Failed to set FdbLearningMode for Port: %u", xpsIntf);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiPortLagInfoGet

sai_status_t xpSaiPortLagInfoGet(xpsInterfaceId_t intfId,
                                 xpSaiPortLagInfo_t **ppSaiPortLagInfoEntry)
{
    XP_STATUS     retVal     = XP_NO_ERR;
    xpSaiPortLagInfo_t portLagInfoKey;

    memset(&portLagInfoKey, 0, sizeof(portLagInfoKey));

    portLagInfoKey.intfId = intfId;
    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, portLagDbHandle, &portLagInfoKey,
                                (void**)ppSaiPortLagInfoEntry);

    if (NULL == *ppSaiPortLagInfoEntry)
    {
        XP_SAI_LOG_DBG("Could not find Port LAG entry in DB. Decided it to be a stand alone port.\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiConvertPortOid

sai_status_t xpSaiConvertPortOid(sai_object_id_t port_id,
                                 xpsInterfaceId_t* pXpsIntf)
{
    if (!(XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT) ||
          XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_LAG) ||
          XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_BRIDGE_PORT)))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *pXpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetDefaultPortAttributeVals

void xpSaiSetDefaultPortAttributeVals(xpsDevice_t xpsDevId,
                                      xpSaiPortAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiSetDefaultPortAttributeVals\n");
    XP_STATUS status = XP_NO_ERR;
    xpsVlan_t      vlanId = 0;

    if ((status = xpsVlanGetDefault(xpsDevId, &vlanId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get Default vlanId\n");
    }

    attributes->fullDuplexMode.booldata  = true;
    attributes->autoNegMode.booldata = false;
    attributes->adminState.booldata = false;
    attributes->mediaType.s32 = SAI_PORT_MEDIA_TYPE_NOT_PRESENT;
    attributes->advertisedSpeed.u32list.list = NULL;
    attributes->advertisedSpeed.u32list.count = 0;
    attributes->advertisedHalfDuplexSpeed.u32list.list = NULL;
    attributes->advertisedHalfDuplexSpeed.u32list.count = 0;
    attributes->advertisedAutoNegMode.booldata = false;
    attributes->advertisedFlowControlMode.s32 =
        SAI_PORT_ATTR_ADVERTISED_HALF_DUPLEX_SPEED;
    attributes->advertisedAsymmetricPauseMode.booldata = false;
    attributes->advertisedMediaType.s32 = SAI_PORT_MEDIA_TYPE_UNKNOWN;
    attributes->portVlanId.u16 = vlanId;
    attributes->defaultVlanPriority.u8 = 0;
    attributes->ingressFiltering.booldata = false;
    attributes->dropUntagged.booldata = false;
    attributes->dropTagged.booldata = false;
    attributes->internalLoopbackMode.s32 = SAI_PORT_INTERNAL_LOOPBACK_MODE_NONE;
    attributes->fdbLearningMode.s32 = SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW;
    attributes->updateDscp.booldata = false;
    attributes->mtu.u32 = XPSAI_PORT_DEFAULT_MTU;
    attributes->floodStormControlPolicerId.oid = SAI_NULL_OBJECT_ID;
    attributes->broadcastStormControlPolicerId.oid = SAI_NULL_OBJECT_ID;
    attributes->multicastStormControlPolicerId.oid = SAI_NULL_OBJECT_ID;
    attributes->globalFlowControlMode.s32 = SAI_PORT_FLOW_CONTROL_MODE_DISABLE;
    attributes->maxLearnedAddresses.u32 = 0;
    attributes->fdbLearningLimitViolationPacketAction.s32 = SAI_PACKET_ACTION_DROP;
    attributes->ingressMirrorSession.objlist.list = NULL;
    attributes->ingressMirrorSession.objlist.count = 0;
    attributes->egressMirrorSession.objlist.list = NULL;
    attributes->egressMirrorSession.objlist.count = 0;
    attributes->ingressSamplepacketEnable.booldata = true;
    attributes->egressSamplepacketEnable.booldata = true;
    attributes->policerId.oid = SAI_NULL_OBJECT_ID;
    attributes->qosDefaultTc.u8 = 0;
    attributes->qosDot1pToTcMap.oid = SAI_NULL_OBJECT_ID;
    attributes->qosDot1pToColorMap.oid = SAI_NULL_OBJECT_ID;
    attributes->qosDscpToTcMap.oid = SAI_NULL_OBJECT_ID;
    attributes->qosDscpToColorMap.oid = SAI_NULL_OBJECT_ID;
    attributes->qosTcToQueueMap.oid = SAI_NULL_OBJECT_ID;
    attributes->qosTcAndColorToDot1pMap.oid = SAI_NULL_OBJECT_ID;
    attributes->qosTcAndColorToDscpMap.oid = SAI_NULL_OBJECT_ID;
    attributes->qosTcToPriorityGroupMap.oid = SAI_NULL_OBJECT_ID;
    attributes->qosPfcPriorityToPriorityGroupMap.oid = SAI_NULL_OBJECT_ID;
    attributes->qosPfcPriorityToQueueMap.oid = SAI_NULL_OBJECT_ID;
    attributes->qosWredProfileId.oid = SAI_NULL_OBJECT_ID;
    attributes->qosSchedulerProfileId.oid = SAI_NULL_OBJECT_ID;
    attributes->qosIngressBufferProfileList.objlist.list = NULL;
    attributes->qosIngressBufferProfileList.objlist.count = 0;
    attributes->qosEgressBufferProfileList.objlist.list = NULL;
    attributes->qosEgressBufferProfileList.objlist.count = 0;
    attributes->priorityFlowControl.u8 = 0;
    attributes->metaData.u32 = 0;
    attributes->egressBlockPortList.objlist.list = NULL;
    attributes->egressBlockPortList.objlist.count = 0;
    attributes->hwProfileId.u64 = 0;
    attributes->txEnable.booldata  = true;

    attributes->eeeEnable.booldata = false;
    attributes->eeeIdleTime.u16 = 2500;
    attributes->eeeWakeTime.u16 = 5;
    attributes->pvidModeAllPkts.booldata = false;
    attributes->egressTimestamp.booldata = false;
    attributes->intfType.s32 =
        SAI_PORT_INTERFACE_TYPE_NONE; /*NONE means use default type*/
    attributes->internalLoopbackMode.s32 = SAI_PORT_INTERNAL_LOOPBACK_MODE_NONE;
    attributes->ingAclOid.oid = SAI_NULL_OBJECT_ID;
    attributes->egrAclOid.oid = SAI_NULL_OBJECT_ID;
}

//Func: xpSaiUpdatePortAttributeVals

void xpSaiUpdatePortAttributeVals(const uint32_t attr_count,
                                  const sai_attribute_t* attr_list, xpSaiPortAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiUpdatePortAttributeVals\n");

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_PORT_ATTR_TYPE:
                {
                    attributes->type = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_OPER_STATUS:
                {
                    attributes->operStatus = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE_TYPE:
                {
                    attributes->supportedBreakoutModeType = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE:
                {
                    attributes->currentBreakoutModeType = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES:
                {
                    attributes->qosNumberOfQueues = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_QUEUE_LIST:
                {
                    attributes->qosQueueList = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS:
                {
                    attributes->qosNumberOfSchedulerGroups = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST:
                {
                    attributes->qosSchedulerGroupList = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_SUPPORTED_SPEED:
                {
                    attributes->supportedSpeed = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_SUPPORTED_HALF_DUPLEX_SPEED:
                {
                    attributes->supportedHalfDuplexSpeed = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_SUPPORTED_AUTO_NEG_MODE:
                {
                    attributes->supportedAutoNegMode = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_SUPPORTED_FLOW_CONTROL_MODE:
                {
                    attributes->supportedFlowControlMode = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_SUPPORTED_ASYMMETRIC_PAUSE_MODE:
                {
                    attributes->supportedAsymmetricPauseMode = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_SUPPORTED_MEDIA_TYPE:
                {
                    attributes->supportedMediaType = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_REMOTE_ADVERTISED_SPEED:
                {
                    attributes->remoteAdvertisedSpeed = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_REMOTE_ADVERTISED_HALF_DUPLEX_SPEED:
                {
                    attributes->remoteAdvertisedHalfDuplexSpeed = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_REMOTE_ADVERTISED_AUTO_NEG_MODE:
                {
                    attributes->remoteAdvertisedAutoNegMode = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_REMOTE_ADVERTISED_FLOW_CONTROL_MODE:
                {
                    attributes->remoteAdvertisedFlowControlMode = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_REMOTE_ADVERTISED_ASYMMETRIC_PAUSE_MODE:
                {
                    attributes->remoteAdvertisedAsymmetricPauseMode = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_REMOTE_ADVERTISED_MEDIA_TYPE:
                {
                    attributes->remoteAdvertisedMediaType = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_NUMBER_OF_INGRESS_PRIORITY_GROUPS:
                {
                    attributes->numberOfIngressPriorityGroups = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST:
                {
                    attributes->ingressPriorityGroupList = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_HW_LANE_LIST:
                {
                    attributes->hwLaneList = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_SPEED:
                {
                    attributes->speed = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_INTERFACE_TYPE:
                {
                    attributes->intfType = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_FULL_DUPLEX_MODE:
                {
                    attributes->fullDuplexMode = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_AUTO_NEG_MODE:
                {
                    attributes->autoNegMode = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_ADMIN_STATE:
                {
                    attributes->adminState = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_MEDIA_TYPE:
                {
                    attributes->mediaType = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_ADVERTISED_SPEED:
                {
                    attributes->advertisedSpeed = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_ADVERTISED_HALF_DUPLEX_SPEED:
                {
                    attributes->advertisedHalfDuplexSpeed = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_ADVERTISED_AUTO_NEG_MODE:
                {
                    attributes->advertisedAutoNegMode = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_ADVERTISED_FLOW_CONTROL_MODE:
                {
                    attributes->advertisedFlowControlMode = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_ADVERTISED_ASYMMETRIC_PAUSE_MODE:
                {
                    attributes->advertisedAsymmetricPauseMode = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_ADVERTISED_MEDIA_TYPE:
                {
                    attributes->advertisedMediaType = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_PORT_VLAN_ID:
                {
                    attributes->portVlanId = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY:
                {
                    attributes->defaultVlanPriority = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_DROP_UNTAGGED:
                {
                    attributes->dropUntagged = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_DROP_TAGGED:
                {
                    attributes->dropTagged = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE:
                {
                    attributes->internalLoopbackMode = attr_list[count].value;
                    break;
                }
            /*            case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE:
                        {
                            // conflict with SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS;
                            attributes->fdbLearningMode = attr_list[count].value;
                            break;
                        }*/
            case SAI_PORT_ATTR_UPDATE_DSCP:
                {
                    attributes->updateDscp = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_MTU:
                {
                    attributes->mtu = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID:
                {
                    attributes->floodStormControlPolicerId = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID:
                {
                    attributes->broadcastStormControlPolicerId = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID:
                {
                    attributes->multicastStormControlPolicerId = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE:
                {
                    attributes->globalFlowControlMode = attr_list[count].value;
                    break;
                }
            /*            case SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES:
                        {
                            attributes->maxLearnedAddresses = attr_list[count].value;
                            break;
                        }
                        case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION:
                        {
                            attributes->fdbLearningLimitViolationPacketAction = attr_list[count].value;
                            break;
                        }
            */
            case SAI_PORT_ATTR_INGRESS_MIRROR_SESSION:
                {
                    attributes->ingressMirrorSession = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_EGRESS_MIRROR_SESSION:
                {
                    attributes->egressMirrorSession = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE:
                {
                    attributes->ingressSamplepacketEnable = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE:
                {
                    attributes->egressSamplepacketEnable = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_POLICER_ID:
                {
                    attributes->policerId = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_DEFAULT_TC:
                {
                    attributes->qosDefaultTc = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP:
                {
                    attributes->qosDot1pToTcMap = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP:
                {
                    attributes->qosDot1pToColorMap = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP:
                {
                    attributes->qosDscpToTcMap = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP:
                {
                    attributes->qosDscpToColorMap = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP:
                {
                    attributes->qosTcToQueueMap = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
                {
                    attributes->qosTcAndColorToDot1pMap = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
                {
                    attributes->qosTcAndColorToDscpMap = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP:
                {
                    attributes->qosTcToPriorityGroupMap = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_PRIORITY_GROUP_MAP:
                {
                    attributes->qosPfcPriorityToPriorityGroupMap = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP:
                {
                    attributes->qosPfcPriorityToQueueMap = attr_list[count].value;
                    break;
                }
#if 0 // SAI 1.2
            case SAI_PORT_POOL_ATTR_QOS_WRED_PROFILE_ID:
                {
                    attributes->qosWredProfileId = attr_list[count].value;
                    break;
                }
#endif
            case SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID:
                {
                    attributes->qosSchedulerProfileId = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST:
                {
                    attributes->qosIngressBufferProfileList = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_QOS_EGRESS_BUFFER_PROFILE_LIST:
                {
                    attributes->qosEgressBufferProfileList = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL:
                {
                    attributes->priorityFlowControl = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_META_DATA:
                {
                    attributes->metaData = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST:
                {
                    attributes->egressBlockPortList = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_HW_PROFILE_ID:
                {
                    attributes->hwProfileId = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_FEC_MODE:
                {
                    attributes->fecMode = attr_list[count].value;
                    break;
                }
#if 0
            case SAI_PORT_ATTR_EEE_ENABLE:
                {
                    attributes->eeeEnable = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_EEE_IDLE_TIME:
                {
                    attributes->eeeIdleTime = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_EEE_WAKE_TIME:
                {
                    attributes->eeeWakeTime = attr_list[count].value;
                    break;
                }
#endif
            case SAI_PORT_ATTR_PKT_TX_ENABLE:
                {
                    attributes->txEnable = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_INGRESS_ACL:
                {
                    attributes->ingAclOid = attr_list[count].value;
                    break;
                }
            case SAI_PORT_ATTR_EGRESS_ACL:
                {
                    attributes->egrAclOid = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }

    }

}

//Func: xpSaiUpdateAttrListPortVals

void xpSaiUpdateAttrListPortVals(const xpSaiPortAttributesT *attributes,
                                 const uint32_t attr_count, sai_attribute_t* attr_list)
{
    XP_SAI_LOG_DBG("Calling xpSaiUpdateAttrListPortVals\n");
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_PORT_ATTR_TYPE:
                {
                    attr_list[count].value = attributes->type;
                    break;
                }
            case SAI_PORT_ATTR_OPER_STATUS:
                {
                    attr_list[count].value = attributes->operStatus;
                    break;
                }
            case SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE_TYPE:
                {
                    attr_list[count].value = attributes->supportedBreakoutModeType;
                    break;
                }
            case SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE:
                {
                    attr_list[count].value = attributes->currentBreakoutModeType;
                    break;
                }
            case SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES:
                {
                    attr_list[count].value = attributes->qosNumberOfQueues;
                    break;
                }
            case SAI_PORT_ATTR_QOS_QUEUE_LIST:
                {
                    attr_list[count].value = attributes->qosQueueList;
                    break;
                }
            case SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS:
                {
                    attr_list[count].value = attributes->qosNumberOfSchedulerGroups;
                    break;
                }
            case SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST:
                {
                    attr_list[count].value = attributes->qosSchedulerGroupList;
                    break;
                }
            case SAI_PORT_ATTR_SUPPORTED_SPEED:
                {
                    attr_list[count].value = attributes->supportedSpeed;
                    break;
                }
            case SAI_PORT_ATTR_SUPPORTED_HALF_DUPLEX_SPEED:
                {
                    attr_list[count].value = attributes->supportedHalfDuplexSpeed;
                    break;
                }
            case SAI_PORT_ATTR_SUPPORTED_AUTO_NEG_MODE:
                {
                    attr_list[count].value = attributes->supportedAutoNegMode;
                    break;
                }
            case SAI_PORT_ATTR_SUPPORTED_FLOW_CONTROL_MODE:
                {
                    attr_list[count].value = attributes->supportedFlowControlMode;
                    break;
                }
            case SAI_PORT_ATTR_SUPPORTED_ASYMMETRIC_PAUSE_MODE:
                {
                    attr_list[count].value = attributes->supportedAsymmetricPauseMode;
                    break;
                }
            case SAI_PORT_ATTR_SUPPORTED_MEDIA_TYPE:
                {
                    attr_list[count].value = attributes->supportedMediaType;
                    break;
                }
            case SAI_PORT_ATTR_REMOTE_ADVERTISED_SPEED:
                {
                    attr_list[count].value = attributes->remoteAdvertisedSpeed;
                    break;
                }
            case SAI_PORT_ATTR_REMOTE_ADVERTISED_HALF_DUPLEX_SPEED:
                {
                    attr_list[count].value = attributes->remoteAdvertisedHalfDuplexSpeed;
                    break;
                }
            case SAI_PORT_ATTR_REMOTE_ADVERTISED_AUTO_NEG_MODE:
                {
                    attr_list[count].value = attributes->remoteAdvertisedAutoNegMode;
                    break;
                }
            case SAI_PORT_ATTR_REMOTE_ADVERTISED_FLOW_CONTROL_MODE:
                {
                    attr_list[count].value = attributes->remoteAdvertisedFlowControlMode;
                    break;
                }
            case SAI_PORT_ATTR_REMOTE_ADVERTISED_ASYMMETRIC_PAUSE_MODE:
                {
                    attr_list[count].value = attributes->remoteAdvertisedAsymmetricPauseMode;
                    break;
                }
            case SAI_PORT_ATTR_REMOTE_ADVERTISED_MEDIA_TYPE:
                {
                    attr_list[count].value = attributes->remoteAdvertisedMediaType;
                    break;
                }
            case SAI_PORT_ATTR_NUMBER_OF_INGRESS_PRIORITY_GROUPS:
                {
                    attr_list[count].value = attributes->numberOfIngressPriorityGroups;
                    break;
                }
            case SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST:
                {
                    attr_list[count].value = attributes->ingressPriorityGroupList;
                    break;
                }
            case SAI_PORT_ATTR_HW_LANE_LIST:
                {
                    attr_list[count].value = attributes->hwLaneList;
                    break;
                }
            case SAI_PORT_ATTR_SPEED:
                {
                    attr_list[count].value = attributes->speed;
                    break;
                }
            case SAI_PORT_ATTR_FULL_DUPLEX_MODE:
                {
                    attr_list[count].value = attributes->fullDuplexMode;
                    break;
                }
            case SAI_PORT_ATTR_AUTO_NEG_MODE:
                {
                    attr_list[count].value = attributes->autoNegMode;
                    break;
                }
            case SAI_PORT_ATTR_ADMIN_STATE:
                {
                    attr_list[count].value = attributes->adminState;
                    break;
                }
            case SAI_PORT_ATTR_MEDIA_TYPE:
                {
                    attr_list[count].value = attributes->mediaType;
                    break;
                }
            case SAI_PORT_ATTR_ADVERTISED_SPEED:
                {
                    attr_list[count].value = attributes->advertisedSpeed;
                    break;
                }
            case SAI_PORT_ATTR_ADVERTISED_HALF_DUPLEX_SPEED:
                {
                    attr_list[count].value = attributes->advertisedHalfDuplexSpeed;
                    break;
                }
            case SAI_PORT_ATTR_ADVERTISED_AUTO_NEG_MODE:
                {
                    attr_list[count].value = attributes->advertisedAutoNegMode;
                    break;
                }
            case SAI_PORT_ATTR_ADVERTISED_FLOW_CONTROL_MODE:
                {
                    attr_list[count].value = attributes->advertisedFlowControlMode;
                    break;
                }
            case SAI_PORT_ATTR_ADVERTISED_ASYMMETRIC_PAUSE_MODE:
                {
                    attr_list[count].value = attributes->advertisedAsymmetricPauseMode;
                    break;
                }
            case SAI_PORT_ATTR_ADVERTISED_MEDIA_TYPE:
                {
                    attr_list[count].value = attributes->advertisedMediaType;
                    break;
                }
            case SAI_PORT_ATTR_PORT_VLAN_ID:
                {
                    attr_list[count].value = attributes->portVlanId;
                    break;
                }
            case SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY:
                {
                    attr_list[count].value = attributes->defaultVlanPriority;
                    break;
                }
            case SAI_PORT_ATTR_DROP_UNTAGGED:
                {
                    attr_list[count].value = attributes->dropUntagged;
                    break;
                }
            case SAI_PORT_ATTR_DROP_TAGGED:
                {
                    attr_list[count].value = attributes->dropTagged;
                    break;
                }
            case SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE:
                {
                    attr_list[count].value = attributes->internalLoopbackMode;
                    break;
                }
            /*            case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE:
                        {
                            attr_list[count].value = attributes->fdbLearningMode;
                            break;
                        }
            */
            case SAI_PORT_ATTR_UPDATE_DSCP:
                {
                    attr_list[count].value = attributes->updateDscp;
                    break;
                }
            case SAI_PORT_ATTR_MTU:
                {
                    attr_list[count].value = attributes->mtu;
                    break;
                }
            case SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID:
                {
                    attr_list[count].value = attributes->floodStormControlPolicerId;
                    break;
                }
            case SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID:
                {
                    attr_list[count].value = attributes->broadcastStormControlPolicerId;
                    break;
                }
            case SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID:
                {
                    attr_list[count].value = attributes->multicastStormControlPolicerId;
                    break;
                }
            case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE:
                {
                    attr_list[count].value = attributes->globalFlowControlMode;
                    break;
                }
            /*            case SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES:
                        {
                            attr_list[count].value = attributes->maxLearnedAddresses;
                            break;
                        }
                        case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION:
                        {
                            attr_list[count].value = attributes->fdbLearningLimitViolationPacketAction;
                            break;
                        }
            */
            case SAI_PORT_ATTR_INGRESS_MIRROR_SESSION:
                {
                    attr_list[count].value = attributes->ingressMirrorSession;
                    break;
                }
            case SAI_PORT_ATTR_EGRESS_MIRROR_SESSION:
                {
                    attr_list[count].value = attributes->egressMirrorSession;
                    break;
                }
            case SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE:
                {
                    attr_list[count].value = attributes->ingressSamplepacketEnable;
                    break;
                }
            case SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE:
                {
                    attr_list[count].value = attributes->egressSamplepacketEnable;
                    break;
                }
            case SAI_PORT_ATTR_POLICER_ID:
                {
                    attr_list[count].value = attributes->policerId;
                    break;
                }
            case SAI_PORT_ATTR_QOS_DEFAULT_TC:
                {
                    attr_list[count].value = attributes->qosDefaultTc;
                    break;
                }
            case SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP:
                {
                    attr_list[count].value = attributes->qosDot1pToTcMap;
                    break;
                }
            case SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP:
                {
                    attr_list[count].value = attributes->qosDot1pToColorMap;
                    break;
                }
            case SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP:
                {
                    attr_list[count].value = attributes->qosDscpToTcMap;
                    break;
                }
            case SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP:
                {
                    attr_list[count].value = attributes->qosDscpToColorMap;
                    break;
                }
            case SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP:
                {
                    attr_list[count].value = attributes->qosTcToQueueMap;
                    break;
                }
            case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
                {
                    attr_list[count].value = attributes->qosTcAndColorToDot1pMap;
                    break;
                }
            case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
                {
                    attr_list[count].value = attributes->qosTcAndColorToDscpMap;
                    break;
                }
            case SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP:
                {
                    attr_list[count].value = attributes->qosTcToPriorityGroupMap;
                    break;
                }
            case SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_PRIORITY_GROUP_MAP:
                {
                    attr_list[count].value = attributes->qosPfcPriorityToPriorityGroupMap;
                    break;
                }
            case SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP:
                {
                    attr_list[count].value = attributes->qosPfcPriorityToQueueMap;
                    break;
                }
#if 0 // SAI 1.2
            case SAI_PORT_POOL_ATTR_QOS_WRED_PROFILE_ID:
                {
                    attr_list[count].value = attributes->qosWredProfileId;
                    break;
                }
#endif
            case SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID:
                {
                    attr_list[count].value = attributes->qosSchedulerProfileId;
                    break;
                }
            case SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST:
                {
                    attr_list[count].value = attributes->qosIngressBufferProfileList;
                    break;
                }
            case SAI_PORT_ATTR_QOS_EGRESS_BUFFER_PROFILE_LIST:
                {
                    attr_list[count].value = attributes->qosEgressBufferProfileList;
                    break;
                }
            case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL:
                {
                    attr_list[count].value = attributes->priorityFlowControl;
                    break;
                }
            case SAI_PORT_ATTR_META_DATA:
                {
                    attr_list[count].value = attributes->metaData;
                    break;
                }
            case SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST:
                {
                    attr_list[count].value = attributes->egressBlockPortList;
                    break;
                }
            case SAI_PORT_ATTR_HW_PROFILE_ID:
                {
                    attr_list[count].value = attributes->hwProfileId;
                    break;
                }
            case SAI_PORT_ATTR_FEC_MODE:
                {
                    attr_list[count].value = attributes->fecMode;
                    break;
                }
#if 0
            case SAI_PORT_ATTR_EEE_ENABLE:
                {
                    attr_list[count].value = attributes->eeeEnable;
                    break;
                }
            case SAI_PORT_ATTR_EEE_IDLE_TIME:
                {
                    attr_list[count].value = attributes->eeeIdleTime;
                    break;
                }
            case SAI_PORT_ATTR_EEE_WAKE_TIME:
                {
                    attr_list[count].value = attributes->eeeWakeTime;
                    break;
                }
#endif
            case SAI_PORT_ATTR_PKT_TX_ENABLE:
                {
                    attr_list[count].value = attributes->txEnable;
                    break;
                }
            case SAI_PORT_ATTR_INGRESS_ACL:
                {
                    attr_list[count].value = attributes->ingAclOid;
                    break;
                }
            case SAI_PORT_ATTR_EGRESS_ACL:
                {
                    attr_list[count].value = attributes->egrAclOid;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }

    }

}

//Func: xpSaiGetPortAttrSupportedBreakoutModeType

sai_status_t xpSaiGetPortAttrSupportedBreakoutModeType(sai_object_id_t port_id,
                                                       sai_attribute_value_t* value)
{
    XP_STATUS          xpStatus      = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf       = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    uint8_t            maxMacChannel = XPSAI_PORT_HW_LANE_CNT_MAX;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrSupportedBreakoutModeType\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).\n",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsGlobalSwitchControlGetMacMaxChannelNumber(devId, &maxMacChannel);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get MAC max channel number\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (maxMacChannel == 8)
    {
        if ((xpsIntf % maxMacChannel) == 0)
        {
            if (value->s32list.count < 4)
            {
                value->s32list.count = 4;
                return SAI_STATUS_BUFFER_OVERFLOW;
            }
            value->s32list.count = 4;
            value->s32list.list[0] = SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE;
            value->s32list.list[1] = SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE;
            value->s32list.list[2] = SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE;
            //value->s32list.list[3] = SAI_PORT_BREAKOUT_MODE_TYPE_8_LANE;   //not yet present in sai header file.
        }
        else if ((xpsIntf % maxMacChannel) == 4)
        {
            if (value->s32list.count < 3)
            {
                value->s32list.count = 3;
                return SAI_STATUS_BUFFER_OVERFLOW;
            }
            value->s32list.count = 3;

            value->s32list.list[0] = SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE;
            value->s32list.list[1] = SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE;
            value->s32list.list[2] = SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE;
        }
        else if ((xpsIntf % maxMacChannel) == 2)
        {
            if (value->s32list.count < 2)
            {
                value->s32list.count = 2;
                return SAI_STATUS_BUFFER_OVERFLOW;
            }
            value->s32list.count = 2;

            value->s32list.list[0] = SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE;
            value->s32list.list[1] = SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE;
        }
        else
        {
            if (value->s32list.count < 1)
            {
                value->s32list.count = 1;
                return SAI_STATUS_BUFFER_OVERFLOW;
            }
            value->s32list.count = 1;
            value->s32list.list[0] = SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE;
        }
    }

    else if (maxMacChannel == 4)
    {
        if ((xpsIntf % maxMacChannel) == 0)
        {
            if (value->s32list.count < 3)
            {
                value->s32list.count = 3;
                return SAI_STATUS_BUFFER_OVERFLOW;
            }
            value->s32list.count = 3;
            value->s32list.list[0] = SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE;
            value->s32list.list[1] = SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE;
            value->s32list.list[2] = SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE;
        }
        else if ((xpsIntf % maxMacChannel) == 2)
        {
            if (value->s32list.count < 2)
            {
                value->s32list.count = 2;
                return SAI_STATUS_BUFFER_OVERFLOW;
            }
            value->s32list.count = 2;

            value->s32list.list[0] = SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE;
            value->s32list.list[1] = SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE;
        }
        else if ((xpsIntf % maxMacChannel) == 2)
        {
            if (value->s32list.count < 2)
            {
                value->s32list.count = 2;
                return SAI_STATUS_BUFFER_OVERFLOW;
            }
            value->s32list.count = 2;

            value->s32list.list[0] = SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE;
            value->s32list.list[1] = SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE;
        }
        else
        {
            if (value->s32list.count < 1)
            {
                value->s32list.count = 1;
                return SAI_STATUS_BUFFER_OVERFLOW;
            }
            value->s32list.count = 1;
            value->s32list.list[0] = SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE;
        }
    }

    else if (maxMacChannel == 1)
    {
        if (value->s32list.count < 1)
        {
            value->s32list.count = 1;
            return SAI_STATUS_BUFFER_OVERFLOW;
        }
        value->s32list.count = 1;
        value->s32list.list[0] = SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE;

    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrCurrentBreakoutModeType

sai_status_t xpSaiGetPortAttrCurrentBreakoutModeType(sai_object_id_t port_id,
                                                     sai_attribute_value_t* value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (info.breakOutMode == XPSAI_PORT_BREAKOUT_MODE_1X)
    {
        value->s32 = SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE;
    }
    else if (info.breakOutMode == XPSAI_PORT_BREAKOUT_MODE_2X)
    {
        value->s32 = SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE;
    }
    else if (info.breakOutMode == XPSAI_PORT_BREAKOUT_MODE_4X)
    {
        value->s32 = SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE;
    }
    else
    {
        //value->s32 = SAI_PORT_BREAKOUT_MODE_TYPE_8_LANE;
    }
    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiGetPortAttrSupportedSpeed

sai_status_t xpSaiGetPortAttrSupportedSpeed(sai_object_id_t port_id,
                                            sai_attribute_value_t* value)
{
    XP_STATUS          xpStatus      = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf       = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;
    uint8_t maxMacChannel = XPSAI_PORT_HW_LANE_CNT_MAX;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsGlobalSwitchControlGetMacMaxChannelNumber(devId, &maxMacChannel);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get MAC max channel number\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    switch (info.breakOutMode)
    {
        case XPSAI_PORT_BREAKOUT_MODE_8X:
            if (value->s32list.count < 1)
            {
                value->s32list.count = 1;
                return SAI_STATUS_BUFFER_OVERFLOW;
            }

            value->s32list.count = 1;
            value->s32list.list[0] = XPSAI_PORT_SPEED_400G;

            break;
        case XPSAI_PORT_BREAKOUT_MODE_4X:
            if (maxMacChannel == 4)
            {
                if (value->s32list.count < 2)
                {
                    value->s32list.count = 2;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }
                value->s32list.count = 2;
                value->s32list.list[0] = XPSAI_PORT_SPEED_100G;
                value->s32list.list[1] = XPSAI_PORT_SPEED_40G;
            }
            else if (maxMacChannel == 8)
            {
                if (value->s32list.count < 1)
                {
                    value->s32list.count = 2;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }
                value->s32list.count = 1;
                value->s32list.list[0] = XPSAI_PORT_SPEED_200G;
            }
            break;
        case XPSAI_PORT_BREAKOUT_MODE_2X:
            if (value->s32list.count < 1)
            {
                value->s32list.count = 1;
                return SAI_STATUS_BUFFER_OVERFLOW;
            }

            value->s32list.count = 1;
            if (maxMacChannel == 4)
            {
                value->s32list.list[0] = XPSAI_PORT_SPEED_50G;
            }
            else if (maxMacChannel == 8)
            {
                value->s32list.list[1] = XPSAI_PORT_SPEED_100G;
            }

            break;

        case XPSAI_PORT_BREAKOUT_MODE_1X:
            if (maxMacChannel == 1)
            {
                if (value->s32list.count < 4)
                {
                    value->s32list.count = 4;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }

                value->s32list.count = 4;
                value->s32list.list[0] = XPSAI_PORT_SPEED_1000MB;
                value->s32list.list[1] = XPSAI_PORT_SPEED_100MB;
                value->s32list.list[2] = XPSAI_PORT_SPEED_10MB;
                value->s32list.list[2] = XPSAI_PORT_SPEED_10G;
            }
            else if (maxMacChannel == 4)
            {
                if (value->s32list.count < 3)
                {
                    value->s32list.count = 3;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }

                value->s32list.count = 3;
                value->s32list.list[0] = XPSAI_PORT_SPEED_25G;
                value->s32list.list[1] = XPSAI_PORT_SPEED_10G;
                value->s32list.list[2] = XPSAI_PORT_SPEED_1000MB;
            }
            else if (maxMacChannel == 8)
            {
                if (value->s32list.count < 3)
                {
                    value->s32list.count = 3;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }

                value->s32list.count = 3;
                value->s32list.list[0] = XPSAI_PORT_SPEED_50G;
                value->s32list.list[1] = XPSAI_PORT_SPEED_25G;
                value->s32list.list[2] = XPSAI_PORT_SPEED_10G;
            }

            break;
        default:
            XP_SAI_LOG_ERR("Unsupported breakout mode %u.", info.breakOutMode);
            return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrSupportedHalfDuplexSpeed

sai_status_t xpSaiGetPortAttrSupportedHalfDuplexSpeed(sai_object_id_t port_id,
                                                      sai_attribute_value_t* value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    value->objlist.count =
        0;  // SAI does not support any half duplex speed at the moment

    return saiRetVal;
}

//Func: xpSaiGetPortAttrSupportedAutoNegMode

sai_status_t xpSaiGetPortAttrSupportedAutoNegMode(sai_object_id_t port_id,
                                                  sai_attribute_value_t* value)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrSupportedAutoNegMode\n");

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    // Auto Negotiation is always supported and on all MAC types
    value->booldata = true;

    return retVal;
}

//Func: xpSaiGetPortAttrSupportedFlowControlMode

sai_status_t xpSaiGetPortAttrSupportedFlowControlMode(sai_object_id_t port_id,
                                                      sai_attribute_value_t* value)
{

    value->s32 = SAI_PORT_FLOW_CONTROL_MODE_BOTH_ENABLE;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiPortApplyPolicer

sai_status_t xpSaiPortApplyPolicer(sai_object_id_t portObjId,
                                   sai_object_id_t policerObjId, xpsPolicerStormType_e stormType)
{
    xpsPolicerEntry_t xpPolicerEntry;
    sai_uint32_t      acmIndex;
    sai_uint32_t      portId  = xpSaiObjIdValueGet(portObjId);
    xpsDevice_t       xpDevId = xpSaiObjIdSwitchGet(portObjId);
    sai_status_t      saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS         xpStatus  = XP_NO_ERR;

    memset(&xpPolicerEntry, 0, sizeof(xpPolicerEntry));

    xpStatus = xpsPolicerPortPolicerIndexGet(xpDevId, portId,
                                             XP_INGRESS_PORT_POLICER, stormType, &acmIndex);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get ingress port policer index: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (policerObjId == SAI_NULL_OBJECT_ID)
    {
        /* Remove port policer */
        if ((xpStatus = xpsPolicerRemoveEntry(xpDevId, XP_INGRESS_PORT_POLICER,
                                              acmIndex)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to remove policer entry |Error: %d\n", xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Disable policer for storm type */
        if ((xpStatus = xpsPolicerStormTypeEnable(xpDevId, portId, stormType,
                                                  false)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to disable policer for storm type %d |Error: %d\n",
                           stormType, xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }
    else
    {
        /* Get xp policer entry for given policer object */
        saiStatus = xpSaiPolicerGetXpsEntry(policerObjId, &xpPolicerEntry);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to get xp policer entry for given policer "
                           "object 0x%lx, status: %d", policerObjId, saiStatus);
            return saiStatus;
        }

        /* Add policer entry */
        xpStatus = xpsPolicerAddEntry(xpDevId, XP_INGRESS_PORT_POLICER, acmIndex,
                                      acmIndex, &xpPolicerEntry);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to add policer entry, xpStatus: %d\n", xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Enable policer for storm type */
        if ((xpStatus = xpsPolicerStormTypeEnable(xpDevId, portId, stormType,
                                                  true)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to Enable policer for storm type %d |Error: %d\n",
                           stormType, xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Clear policer counter entry */
        if ((xpStatus = xpsPolicerClearPolicerCounterEntry(xpDevId,
                                                           XP_INGRESS_PORT_POLICER, acmIndex)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to clear policer counter |Error: %d\n", xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiSetPortIngressPolicerEnable(sai_object_id_t portObjId)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;
    sai_uint32_t        portId       = xpSaiObjIdValueGet(portObjId);
    xpsDevice_t         xpDevId      = xpSaiObjIdSwitchGet(portObjId);

    /* Get the sai port qos information */
    if ((xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get SAI Port QOS Info retVal:%d\n",
                       xpStatus);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if ((pPortQosInfo->floodScPolicerObjId == SAI_NULL_OBJECT_ID) &&
        (pPortQosInfo->bcastScPolicerObjId == SAI_NULL_OBJECT_ID) &&
        (pPortQosInfo->mcastScPolicerObjId == SAI_NULL_OBJECT_ID) &&
        (pPortQosInfo->policerId == SAI_NULL_OBJECT_ID))
    {
        if ((xpStatus = xpsPortSetPolicingEnable(xpDevId, portId,
                                                 XP_INGRESS_PORT_POLICER, false)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to disable policer globally |Error: %d\n", xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }
    else
    {
        if ((xpStatus = xpsPortSetPolicingEnable(xpDevId, portId,
                                                 XP_INGRESS_PORT_POLICER, true)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to disable policer globally |Error: %d\n", xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrFloodStromControlPolicerId

sai_status_t xpSaiSetPortAttrFloodStromControlPolicerId(
    sai_object_id_t portObjId, sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    sai_object_id_t     policerObjId = value.oid;
    sai_status_t        saiStatus    = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus     = XP_NO_ERR;
    sai_uint32_t        portId       = xpSaiObjIdValueGet(portObjId);
    xpsDevice_t         xpDevId      = xpSaiObjIdSwitchGet(portObjId);

    /* Validate policer object */
    saiStatus = xpSaiPolicerValidatePolicerObject(policerObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Get the port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    /* Check if it is same policer object */
    if (pPortQosInfo->floodScPolicerObjId == policerObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    sai_uint32_t acmIndex = 0;
    xpStatus = xpsPolicerPortPolicerIndexGet(xpDevId, portId,
                                             XP_INGRESS_PORT_POLICER, XPS_UNKNOWN_STORM_CONTROL, &acmIndex);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get ingress port policer index: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Unbind port from current policer state data */
    if (pPortQosInfo->floodScPolicerObjId != SAI_NULL_OBJECT_ID)
    {
        xpStatus = xpSaiPolicerRemoveIdxFromIdxList(pPortQosInfo->floodScPolicerObjId,
                                                    XP_INGRESS_PORT_POLICER,
                                                    acmIndex);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to unbind port from current policer state data, error: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    /* Bind port to new policer state data */
    if (policerObjId != SAI_NULL_OBJECT_ID)
    {
        xpStatus = xpSaiPolicerAddIdxToIdxList(policerObjId, XP_INGRESS_PORT_POLICER,
                                               acmIndex);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind port to new policer state data, error: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    /* Configure policer on port */
    saiStatus = xpSaiPortApplyPolicer(portObjId, policerObjId,
                                      XPS_UNKNOWN_STORM_CONTROL);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to apply policer on port, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Update state data */
    pPortQosInfo->floodScPolicerObjId = policerObjId;

    /* Enable/Disable Ingress port policer */
    saiStatus = xpSaiSetPortIngressPolicerEnable(portObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to enable policer on port, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrBroadcastStromControlPolicerId

sai_status_t xpSaiSetPortAttrBroadcastStromControlPolicerId(
    sai_object_id_t portObjId, sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    sai_object_id_t     policerObjId = value.oid;
    sai_status_t        saiStatus    = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus     = XP_NO_ERR;
    sai_uint32_t        portId       = xpSaiObjIdValueGet(portObjId);
    xpsDevice_t         xpDevId      = xpSaiObjIdSwitchGet(portObjId);

    /* Validate policer object */
    saiStatus = xpSaiPolicerValidatePolicerObject(policerObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Get the port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    /* Check if it is same policer object */
    if (pPortQosInfo->bcastScPolicerObjId == policerObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    sai_uint32_t acmIndex = 0;
    xpStatus = xpsPolicerPortPolicerIndexGet(xpDevId, portId,
                                             XP_INGRESS_PORT_POLICER, XPS_BC_STORM_CONTROL, &acmIndex);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get ingress port policer index: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Unbind port from current policer state data */
    if (pPortQosInfo->bcastScPolicerObjId != SAI_NULL_OBJECT_ID)
    {
        xpStatus = xpSaiPolicerRemoveIdxFromIdxList(pPortQosInfo->bcastScPolicerObjId,
                                                    XP_INGRESS_PORT_POLICER,
                                                    acmIndex);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to unbind port from current policer state data, error: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    /* Bind port to new policer state data */
    if (policerObjId != SAI_NULL_OBJECT_ID)
    {
        xpStatus = xpSaiPolicerAddIdxToIdxList(policerObjId, XP_INGRESS_PORT_POLICER,
                                               acmIndex);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind port to new policer state data, error: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    /* Configure policer on port */
    saiStatus = xpSaiPortApplyPolicer(portObjId, policerObjId,
                                      XPS_BC_STORM_CONTROL);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to apply policer on port, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Update state data */
    pPortQosInfo->bcastScPolicerObjId = policerObjId;

    /* Enable/Disable Ingress port policer */
    saiStatus = xpSaiSetPortIngressPolicerEnable(portObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to enable policer on port, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrMulticastStromControlPolicerId

sai_status_t xpSaiSetPortAttrMulticastStromControlPolicerId(
    sai_object_id_t portObjId, sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    sai_object_id_t     policerObjId = value.oid;
    sai_status_t        saiStatus    = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus     = XP_NO_ERR;
    sai_uint32_t        portId       = xpSaiObjIdValueGet(portObjId);
    xpsDevice_t         xpDevId      = xpSaiObjIdSwitchGet(portObjId);

    /* Validate policer object */
    saiStatus = xpSaiPolicerValidatePolicerObject(policerObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Get the port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    /* Check if it is same policer object */
    if (pPortQosInfo->mcastScPolicerObjId == policerObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    sai_uint32_t acmIndex = 0;
    xpStatus = xpsPolicerPortPolicerIndexGet(xpDevId, portId,
                                             XP_INGRESS_PORT_POLICER, XPS_MC_STORM_CONTROL, &acmIndex);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get ingress port policer index: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Unbind port from current policer state data */
    if (pPortQosInfo->mcastScPolicerObjId != SAI_NULL_OBJECT_ID)
    {
        xpStatus = xpSaiPolicerRemoveIdxFromIdxList(pPortQosInfo->mcastScPolicerObjId,
                                                    XP_INGRESS_PORT_POLICER,
                                                    acmIndex);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to unbind port from current policer state data, error: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    /* Bind port to new policer state data */
    if (policerObjId != SAI_NULL_OBJECT_ID)
    {
        xpStatus = xpSaiPolicerAddIdxToIdxList(policerObjId, XP_INGRESS_PORT_POLICER,
                                               acmIndex);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind port to new policer state data, error: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    /* Configure policer on port */
    saiStatus = xpSaiPortApplyPolicer(portObjId, policerObjId,
                                      XPS_MC_STORM_CONTROL);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to apply policer on port, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Update state data */
    pPortQosInfo->mcastScPolicerObjId = policerObjId;

    /* Enable/Disable Ingress port policer */
    saiStatus = xpSaiSetPortIngressPolicerEnable(portObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to enable policer on port, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrGlobalFlowControlMode

sai_status_t xpSaiSetPortAttrGlobalFlowControlMode(sai_object_id_t port_id,
                                                   sai_attribute_value_t value)
{
    XP_STATUS   xpStatus = XP_NO_ERR;
    sai_uint8_t txEnable = 0;
    sai_uint8_t rxEnable = 0;

    xpsDevice_t  xpDevId  = xpSaiObjIdSwitchGet(port_id);
    sai_uint32_t portNum  = xpSaiObjIdValueGet(port_id);

    switch (value.s32)
    {
        case SAI_PORT_FLOW_CONTROL_MODE_BOTH_ENABLE:
            {
                rxEnable = true;
                txEnable = true;
                break;
            }
        case SAI_PORT_FLOW_CONTROL_MODE_RX_ONLY:
            {
                rxEnable = true;
                txEnable = false;
                break;
            }
        case SAI_PORT_FLOW_CONTROL_MODE_TX_ONLY:
            {
                rxEnable = false;
                txEnable = true;
                break;
            }
        case SAI_PORT_FLOW_CONTROL_MODE_DISABLE:
            {
                rxEnable = false;
                txEnable = false;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Error: Invalid input attribute value %d\n", value.s32);
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
    }

    xpStatus = xpsMacRxFlowControlDecodeEnable(xpDevId, portNum, rxEnable);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get rx flow control enable, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsMacTxFlowControlEnable(xpDevId, portNum, txEnable);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get tx flow control enable, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (txEnable)
    {
        sai_status_t saiStatus = SAI_STATUS_SUCCESS;
        sai_mac_t mac;

        saiStatus = xpSaiGetSwitchSrcMacAddress(mac);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get switch MAC address \n", saiStatus);
            return saiStatus;
        }

        xpSaiMacReverse(mac);

        xpStatus = xpsMacSetMacFlowCtrlFrmSrcAddr(xpDevId, (uint8_t) portNum, mac);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to set PFC frame SRC Mac address |Port:%d xpStatus:%d\n",
                           portNum, xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrFloodStromControlPolicerId

sai_status_t xpSaiGetPortAttrFloodStromControlPolicerId(
    sai_object_id_t portObjId, sai_attribute_value_t* value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Get the port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    value->oid = pPortQosInfo->floodScPolicerObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrBroadcastStromControlPolicerId

sai_status_t xpSaiGetPortAttrBroadcastStromControlPolicerId(
    sai_object_id_t portObjId, sai_attribute_value_t* value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Get the port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    value->oid = pPortQosInfo->bcastScPolicerObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrMulticastStromControlPolicerId

sai_status_t xpSaiGetPortAttrMulticastStromControlPolicerId(
    sai_object_id_t portObjId, sai_attribute_value_t* value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Get the port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    value->oid = pPortQosInfo->mcastScPolicerObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrGlobalFlowControlMode

sai_status_t xpSaiGetPortAttrGlobalFlowControlMode(sai_object_id_t port_id,
                                                   sai_attribute_value_t* value)
{
    XP_STATUS   xpStatus = XP_NO_ERR;
    sai_uint8_t txEnable = 0;
    sai_uint8_t rxEnable = 0;

    xpsDevice_t  xpDevId  = xpSaiObjIdSwitchGet(port_id);
    sai_uint32_t portNum  = xpSaiObjIdValueGet(port_id);

    xpStatus = xpsMacGetRxFlowControlDecodeEnable(xpDevId, portNum, &rxEnable);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get rx flow control enable, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsMacGetTxFlowControlEnable(xpDevId, portNum, &txEnable);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get tx flow control enable, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if ((rxEnable == true) && (txEnable == true))
    {
        value->s32 = SAI_PORT_FLOW_CONTROL_MODE_BOTH_ENABLE;
    }
    else if ((rxEnable == true) && (txEnable == false))
    {
        value->s32 = SAI_PORT_FLOW_CONTROL_MODE_RX_ONLY;
    }
    else if ((rxEnable == false) && (txEnable == true))
    {
        value->s32 = SAI_PORT_FLOW_CONTROL_MODE_TX_ONLY;
    }
    else
    {
        value->s32 = SAI_PORT_FLOW_CONTROL_MODE_DISABLE;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrSupportedAsymmetricPauseMode

sai_status_t xpSaiGetPortAttrSupportedAsymmetricPauseMode(
    sai_object_id_t port_id,  sai_attribute_value_t* value)
{
    sai_status_t retVal = SAI_STATUS_NOT_IMPLEMENTED;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrSupportedAsymmetricPauseMode\n");

    return retVal;
}

//Func: xpSaiGetPortAttrRemoteAdvertisedSpeed

sai_status_t xpSaiGetPortAttrRemoteAdvertisedSpeed(sai_object_id_t port_id,
                                                   sai_attribute_value_t* value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    uint16_t           count      = 0;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;
    uint32_t           remoteAdvertFdAbility;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    remoteAdvertFdAbility = info.remoteAdvertFdAbility;
    for (count = 0; remoteAdvertFdAbility != 0; remoteAdvertFdAbility >>= 1)
    {
        if (remoteAdvertFdAbility & 0x01)
        {
            count++;
        }
    }

    if ((count > value->u32list.count) ||
        (value->u32list.count == 0))
    {
        value->u32list.count = count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    count = 0;

    if (info.remoteAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_10G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_10G;
    }
    if (info.remoteAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_25G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_25G;
    }
    if (info.remoteAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_40G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_40G;
    }
    if (info.remoteAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_50G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_50G;
    }
    if (info.remoteAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_100G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_100G;
    }
    if (info.remoteAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_200G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_200G;
    }
    if (info.remoteAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_400G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_400G;
    }

    value->u32list.count = count;

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiGetPortAttrRemoteAdvertisedHalfDuplexSpeed

sai_status_t xpSaiGetPortAttrRemoteAdvertisedHalfDuplexSpeed(
    sai_object_id_t port_id,  sai_attribute_value_t* value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    uint16_t           count      = 0;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;
    uint32_t           remoteAdvertHdAbility;


    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    remoteAdvertHdAbility = info.remoteAdvertHdAbility;
    for (count = 0; remoteAdvertHdAbility != 0; remoteAdvertHdAbility >>= 1)
    {
        if (remoteAdvertHdAbility & 0x01)
        {
            count++;
        }
    }

    if ((count > value->u32list.count) ||
        (value->u32list.count == 0))
    {
        value->u32list.count = count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    count = 0;

    if (info.remoteAdvertHdAbility & XPSAI_PORT_ADVERT_SPEED_10G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_10G;
    }
    if (info.remoteAdvertHdAbility & XPSAI_PORT_ADVERT_SPEED_25G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_25G;
    }
    if (info.remoteAdvertHdAbility & XPSAI_PORT_ADVERT_SPEED_40G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_40G;
    }
    if (info.remoteAdvertHdAbility & XPSAI_PORT_ADVERT_SPEED_50G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_50G;
    }
    if (info.remoteAdvertHdAbility & XPSAI_PORT_ADVERT_SPEED_100G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_100G;
    }
    if (info.remoteAdvertHdAbility & XPSAI_PORT_ADVERT_SPEED_200G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_200G;
    }
    if (info.remoteAdvertHdAbility & XPSAI_PORT_ADVERT_SPEED_400G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_400G;
    }

    value->u32list.count = count;

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiGetPortAttrRemoteAdvertisedAutoNegMode

sai_status_t xpSaiGetPortAttrRemoteAdvertisedAutoNegMode(
    sai_object_id_t port_id,  sai_attribute_value_t* value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    value->booldata = (info.remoteAdvertFdAbility != XPSAI_PORT_ADVERT_SPEED_NONE);

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiGetPortAttrRemoteAdvertisedFlowControlMode

sai_status_t xpSaiGetPortAttrRemoteAdvertisedFlowControlMode(
    sai_object_id_t port_id,  sai_attribute_value_t* value)
{
    sai_status_t retVal = SAI_STATUS_NOT_IMPLEMENTED;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrRemoteAdvertisedFlowControlMode\n");

    return retVal;
}

//Func: xpSaiGetPortAttrRemoteAdvertisedAsymmetricPauseMode

sai_status_t xpSaiGetPortAttrRemoteAdvertisedAsymmetricPauseMode(
    sai_object_id_t port_id,  sai_attribute_value_t* value)
{
    sai_status_t retVal = SAI_STATUS_NOT_IMPLEMENTED;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrRemoteAdvertisedAsymmetricPauseMode\n");

    return retVal;
}

//Func: xpSaiGetPortAttrNumberOfIngressPriorityGroups

sai_status_t xpSaiGetPortAttrNumberOfIngressPriorityGroups(
    sai_object_id_t port_id,  sai_attribute_value_t* value)
{
    sai_status_t        status       = SAI_STATUS_SUCCESS;
    xpSaiPortQosInfo_t *portQosEntry = NULL;

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Null ptr passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    status = xpSaiGetPortQosInfo(port_id, &portQosEntry);
    if (status != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get port qos entry from DB!\n");
        return status;
    }

    value->objlist.count = portQosEntry->numOfIngPrioGrps;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrIngressPriorityGroupList

static sai_status_t xpSaiGetPortAttrIngressPriorityGroupList(
    sai_object_id_t port_id, sai_attribute_value_t *value)
{
    uint32_t            totalSupportedPrio = 0;
    xpSaiPortQosInfo_t *portQosEntry       = NULL;
    sai_status_t        saiStatus          = SAI_STATUS_SUCCESS;
    xpsDevice_t         xpsDevId           = xpSaiGetDevId();
    sai_status_t        retVal             = XP_NO_ERR;

    /* Validate input parameter */
    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Null ptr passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Validate input parameter value */
    if ((value->objlist.count == 0) || (value->objlist.list == NULL))
    {
        XP_SAI_LOG_ERR("Wrong argument passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get the total supported pfc traffic class */
    if ((retVal = xpSaiPortGetNumPfcPrioritiesSupported(xpsDevId,
                                                        &totalSupportedPrio)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get the number of pfc priorities supported |retVal: %d\n",
                       retVal);
        return retVal;
    }

    if (value->objlist.count < totalSupportedPrio)
    {
        value->objlist.count = totalSupportedPrio;
        XP_SAI_LOG_ERR("No much space provided!\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }
    else if (value->objlist.count > totalSupportedPrio)
    {
        value->objlist.count = totalSupportedPrio;
    }

    /* Get entry from DB */
    saiStatus = xpSaiGetPortQosInfo(port_id, &portQosEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Port QoS Entry!\n");
        return saiStatus;
    }

    if (portQosEntry->numOfIngPrioGrps > value->objlist.count)
    {
        XP_SAI_LOG_ERR("Wrong buffer size received!\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }
    value->objlist.count = portQosEntry->numOfIngPrioGrps;

    memcpy((void*)value->objlist.list, &portQosEntry->ingPrioGrps,
           sizeof(sai_object_id_t) * (portQosEntry->numOfIngPrioGrps));

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetLanesByPort

XP_STATUS xpSaiGetLanesByPort(xpsDevice_t devId, uint32_t portNum,
                              sai_u32_list_t *laneList)
{
    xpSaiPortMgrInfo_t  info;
    uint32_t            count           = 0;
    uint32_t            idx             = 0;
    //uint32_t            baseChannel     = 0;
    //uint32_t            channelSwapNum  = 0;
    uint8_t             maxMacChannel   = XPSAI_PORT_HW_LANE_CNT_MAX;
    XP_STATUS           xpStatus        = XP_NO_ERR;

    xpStatus = xpsGlobalSwitchControlGetMacMaxChannelNumber(devId, &maxMacChannel);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get MAC max channel number\n");
        return xpStatus;
    }

    //uint32_t  basePort = (portNum / maxMacChannel) * maxMacChannel;

    if (laneList == NULL)
    {
        XP_SAI_LOG_ERR("null pointer\n");
        return XP_ERR_NULL_POINTER;
    }

    if (laneList->list == NULL)
    {
        XP_SAI_LOG_ERR("list not initialized\n");
        return XP_ERR_NULL_POINTER;
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("xpSaiPortCfgGet call failed for port %u!\n", portNum);
        return xpStatus;
    }

    switch (info.breakOutMode)
    {
        case XPSAI_PORT_BREAKOUT_MODE_8X:
            count = 8;
            //baseChannel = 0;
            break;
        case XPSAI_PORT_BREAKOUT_MODE_4X:
            count = 4;
            //baseChannel = (portNum - basePort) * count;
            break;
        case XPSAI_PORT_BREAKOUT_MODE_2X:
            count = 2;
            //baseChannel = (portNum - basePort) * count;
            break;
        default:
            laneList->list[0] = portNum + SAI_PORT_INDEX;
            laneList->count = 1;
            return XP_NO_ERR;
    }

    if (count > laneList->count)
    {
        laneList->count = count;
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    for (idx = 0; idx < count; idx++)
    {
        //As cpss is taking care of managing lane mapping we are passing sequential lane numbers in sai.
        /*
        channelSwapNum = 0;
        xpStatus = xpsLinkManagerPlatformGetSerdesLaneSwapInfo(devId, portNum, (baseChannel + idx), &channelSwapNum);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get lane swap index. Err %u\n", xpStatus);
            return xpStatus;
        }
        */
        laneList->list[idx] = portNum + idx + SAI_PORT_INDEX;
    }

    laneList->count = count;
    return XP_NO_ERR;
}

//Func: xpSaiGetLaneBySubPort

uint32_t xpSaiGetLaneBySubPort(xpsDevice_t devId, uint32_t portNum,
                               uint8_t channelNum)
{
    uint8_t channelSwapNum = 0;
    XP_STATUS xpStatus = XP_NO_ERR;
    uint8_t maxMacChannel = XPSAI_PORT_HW_LANE_CNT_MAX;

    xpStatus = xpsGlobalSwitchControlGetMacMaxChannelNumber(devId, &maxMacChannel);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get MAC max channel number\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    uint32_t basePort = (portNum / maxMacChannel) * maxMacChannel;
    xpStatus = xpLinkManagerPlatformGetSerdesLaneSwapInfo(devId, portNum,
                                                          channelNum, &channelSwapNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SWAP error %u\n", xpStatus);
        return portNum + SAI_PORT_INDEX;
    }
    return basePort + channelSwapNum + SAI_PORT_INDEX;
}

//Func: xpSaiGetPortByLanes  //doubt

uint32_t xpSaiGetPortByLanes(xpsDevice_t devId, sai_u32_list_t laneList)
{
    uint32_t portNum = XPS_INTF_INVALID_ID;
    uint8_t maxMacChannel = XPSAI_PORT_HW_LANE_CNT_MAX;
    XP_STATUS xpStatus = XP_NO_ERR;

    xpStatus = xpsGlobalSwitchControlGetMacMaxChannelNumber(devId, &maxMacChannel);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get MAC max channel number\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (maxMacChannel == 4)
    {
        switch (laneList.count)
        {
            case 1:
                return (laneList.list[0] - SAI_PORT_INDEX);
            case 2:
                portNum = ((laneList.list[0] - SAI_PORT_INDEX) / 4) * 4;
                if (xpSaiGetLaneBySubPort(devId, portNum, 0) == laneList.list[0] &&
                    xpSaiGetLaneBySubPort(devId, portNum, 1) == laneList.list[1])
                {
                    return portNum;
                }
                else if (xpSaiGetLaneBySubPort(devId, portNum, 2) == laneList.list[0] &&
                         xpSaiGetLaneBySubPort(devId, portNum, 3) == laneList.list[1])
                {
                    return (portNum + 2);
                }
                XP_SAI_LOG_ERR("Invalid lanes for 2x mode: %u, %u\n", laneList.list[0],
                               laneList.list[1]);
                return XPS_INTF_INVALID_ID;
            case 4:
                return ((laneList.list[0] - SAI_PORT_INDEX) / 4) * 4;
            default:
                break;
        }
    }
    else if (maxMacChannel == 1)
    {
        return laneList.list[0];
    }
    else if (maxMacChannel == 8)
    {
        switch (laneList.count)
        {
            case 8:
                return (laneList.list[0] - SAI_PORT_INDEX);
            case 4:
                portNum = ((laneList.list[0] - SAI_PORT_INDEX) / 8) * 8;

                if (xpSaiGetLaneBySubPort(devId, portNum, 0) == laneList.list[0] &&
                    xpSaiGetLaneBySubPort(devId, portNum, 1) == laneList.list[1] &&
                    xpSaiGetLaneBySubPort(devId, portNum, 2) == laneList.list[2] &&
                    xpSaiGetLaneBySubPort(devId, portNum, 3) == laneList.list[3])
                {
                    return portNum;
                }
                else if (xpSaiGetLaneBySubPort(devId, portNum, 4) == laneList.list[0] &&
                         xpSaiGetLaneBySubPort(devId, portNum, 5) == laneList.list[1] &&
                         xpSaiGetLaneBySubPort(devId, portNum, 6) == laneList.list[2] &&
                         xpSaiGetLaneBySubPort(devId, portNum, 7) == laneList.list[3])
                {
                    return (portNum + 4);  //doubt
                }

                XP_SAI_LOG_ERR("Invalid lanes for 4x mode: %u, %u\n", laneList.list[0],
                               laneList.list[1]);
                return XPS_INTF_INVALID_ID;

            case 2:
            case 6:
                portNum = ((laneList.list[0] - SAI_PORT_INDEX) / 8) * 8;

                if (xpSaiGetLaneBySubPort(devId, portNum, 0) == laneList.list[0] &&
                    xpSaiGetLaneBySubPort(devId, portNum, 1) == laneList.list[1])
                {
                    return portNum;
                }
                else if (xpSaiGetLaneBySubPort(devId, portNum, 2) == laneList.list[0] &&
                         xpSaiGetLaneBySubPort(devId, portNum, 3) == laneList.list[1])
                {
                    return (portNum + 2);
                }
                else if (xpSaiGetLaneBySubPort(devId, portNum, 4) == laneList.list[0] &&
                         xpSaiGetLaneBySubPort(devId, portNum, 5) == laneList.list[1])
                {
                    return (portNum + 4);
                }
                else if (xpSaiGetLaneBySubPort(devId, portNum, 6) == laneList.list[0] &&
                         xpSaiGetLaneBySubPort(devId, portNum, 7) == laneList.list[1])
                {
                    return (portNum + 6);
                }


                XP_SAI_LOG_ERR("Invalid lanes for 2x mode: %u, %u\n", laneList.list[0],
                               laneList.list[1]);
                return XPS_INTF_INVALID_ID;

            case 1:
                return ((laneList.list[0] - SAI_PORT_INDEX));

            default:
                break;
        }
    }
    XP_SAI_LOG_ERR("Invalid number of lanes %u\n", laneList.count);
    return XPS_INTF_INVALID_ID;
}

//Func: xpSaiSetPortAttrHwLaneList

sai_status_t xpSaiSetPortAttrHwLaneList(sai_object_id_t port_id,
                                        sai_attribute_value_t value)
{
    sai_status_t retVal = SAI_STATUS_NOT_IMPLEMENTED;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrHwLaneList\n");

    return retVal;
}

//Func: xpSaiGetPortAttrHwLaneList

sai_status_t xpSaiGetPortAttrHwLaneList(sai_object_id_t port_id,
                                        sai_attribute_value_t* value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpSaiGetLanesByPort(devId, portNum, &value->u32list);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetLanesByPort call failed! Error %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    XP_SAI_LOG_LANES((&value->u32list));

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiGetPortOperStatus

sai_status_t xpSaiGetPortOperStatus(sai_object_id_t port_id,
                                    sai_attribute_value_t *value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (info.linkStatus == XPSAI_PORT_LINK_STATUS_UP)
    {
        value->s32 = SAI_PORT_OPER_STATUS_UP;
    }
    else
    {
        value->s32 = SAI_PORT_OPER_STATUS_DOWN;
    }

    return xpsStatus2SaiStatus(xpStatus);
}


//Func: xpSaiSetPortAttrFullDuplexMode

sai_status_t xpSaiSetPortAttrFullDuplexMode(sai_object_id_t port_id,
                                            sai_attribute_value_t value)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t portId = XPS_INTF_INVALID_ID;
    xpSaiPortDbEntryT *pPortEntry = NULL;

    XP_SAI_LOG_DBG("Called.\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Invalid object type received: typeId=%u.\n",
                       xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!value.booldata)
    {
        XP_SAI_LOG_DBG("Only full duplex mode is supported!\n");
    }

    portId = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    saiStatus = xpSaiPortDbInfoGet(portId, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Coulnd't find port in SAI port DB: portId=%u.\n", portId);
        return saiStatus;
    }

    pthread_mutex_lock(&gSaiPortDbMutex);
    pPortEntry->fullDuplexMode = value.booldata;
    pthread_mutex_unlock(&gSaiPortDbMutex);

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetPortAttrFullDuplexMode

sai_status_t xpSaiGetPortAttrFullDuplexMode(sai_object_id_t port_id,
                                            sai_attribute_value_t* value)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t portId = XPS_INTF_INVALID_ID;
    xpSaiPortDbEntryT *pPortEntry = NULL;

    XP_SAI_LOG_DBG("Called.\n");

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("NULL pointer provided as an argument.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Invalid object type received: typeId=%u.\n",
                       xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    portId = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    saiStatus = xpSaiPortDbInfoGet(portId, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Coulnd't find port in SAI port DB: portId=%u.\n", portId);
        return saiStatus;
    }

    pthread_mutex_lock(&gSaiPortDbMutex);
    value->booldata = pPortEntry->fullDuplexMode;
    pthread_mutex_unlock(&gSaiPortDbMutex);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAutoNegMode

sai_status_t xpSaiSetPortAutoNegMode(sai_object_id_t port_id,
                                     sai_attribute_value_t value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    sai_status_t       saiRetVal  = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpSaiPortDbEntryT *pPortEntry = NULL;
    sai_port_media_type_t mediaType;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;
    //    xpSaiPortMgrInfo_t *portInfo=NULL;
    //    sai_attribute_value_t autoInfo;
    //    sai_status_t xpSaiStatus = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    if (value.booldata)
    {
        /*        portInfo = xpSaiPortInfoGet(portNum);
                if (portInfo != NULL)
                {
                    autoInfo.u32list.count = 1;
                    autoInfo.u32list.list = &portInfo->speed;
                    xpSaiStatus = xpSaiSetPortAttrAdvertisedSpeed(port_id, autoInfo);
                    if (xpSaiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to set advertise info for interface Id (%u).", xpsIntf);
                    }
                }
        */
        saiRetVal = xpSaiPortDbInfoGet(xpsIntf, &pPortEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not retrieve port info from DB");
            return saiRetVal;
        }

        pthread_mutex_lock(&gSaiPortDbMutex);
        mediaType = pPortEntry->mediaType;
        pthread_mutex_unlock(&gSaiPortDbMutex);

        /* In case media type is fiber - disable AN */
        if (mediaType != SAI_PORT_MEDIA_TYPE_COPPER &&
            mediaType != SAI_PORT_MEDIA_TYPE_NOT_PRESENT)
        {
            XP_SAI_LOG_ERR("Please set media type to copper before enabling autoneg\n");
            return SAI_STATUS_FAILURE;
        }

        info.autoneg = XPSAI_PORT_AUTONEG_ENABLE;
    }
    else
    {
        info.autoneg = XPSAI_PORT_AUTONEG_DISABLE;
    }
    xpStatus = xpSaiPortCfgSet(devId, portNum, &info, XPSAI_PORT_ATTR_AUTONEG_MASK);

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiGetPortAttrAutoNegMode

sai_status_t xpSaiGetPortAttrAutoNegMode(sai_object_id_t port_id,
                                         sai_attribute_value_t* value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (info.autoneg == XPSAI_PORT_AUTONEG_ENABLE)
    {
        value->booldata = true;
    }
    else
    {
        value->booldata = false;
    }

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiSetPortAttrMediaType

sai_status_t xpSaiSetPortAttrMediaType(sai_object_id_t port_id,
                                       sai_attribute_value_t value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    sai_status_t       saiRetVal  = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpSaiPortDbEntryT *pPortEntry = NULL;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    saiRetVal = xpSaiPortDbInfoGet(xpsIntf, &pPortEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB");
        return saiRetVal;
    }

    /* In case media type is fiber - disable AN */
    if (value.s32 != SAI_PORT_MEDIA_TYPE_COPPER)
    {
        xpSaiPortInfoInit(&info);
        xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not retrieve port autoneg mode\n");
            return xpsStatus2SaiStatus(xpStatus);
        }

        if (info.autoneg == XPSAI_PORT_AUTONEG_ENABLE)
        {
            XP_SAI_LOG_ERR("Please disable autoneg before setting fiber media type\n");
            return SAI_STATUS_FAILURE;
        }
    }

    pthread_mutex_lock(&gSaiPortDbMutex);
    pPortEntry->mediaType = (sai_port_media_type_t)value.s32;
    pthread_mutex_unlock(&gSaiPortDbMutex);

    return saiRetVal;
}

//Func: xpSaiGetPortAttrMediaType

sai_status_t xpSaiGetPortAttrMediaType(sai_object_id_t port_id,
                                       sai_attribute_value_t* value)
{
    sai_status_t       saiRetVal  = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpSaiPortDbEntryT *pPortEntry = NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    saiRetVal = xpSaiPortDbInfoGet(xpsIntf, &pPortEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB");
        return saiRetVal;
    }

    pthread_mutex_lock(&gSaiPortDbMutex);
    value->s32 = pPortEntry->mediaType;
    pthread_mutex_unlock(&gSaiPortDbMutex);

    return saiRetVal;
}

//Func: xpSaiSetPortAttrAdvertisedSpeed

sai_status_t xpSaiSetPortAttrAdvertisedSpeed(sai_object_id_t port_id,
                                             sai_attribute_value_t value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    uint16_t           i          = 0;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);

    for (i = 0; i < value.u32list.count; i++)
    {
        if (value.u32list.list[i] == XPSAI_PORT_SPEED_10G)
        {
            info.localAdvertFdAbility |= XPSAI_PORT_ADVERT_SPEED_10G;
        }
        else if (value.u32list.list[i] == XPSAI_PORT_SPEED_25G)
        {
            info.localAdvertFdAbility |= (XPSAI_PORT_ADVERT_SPEED_25G |
                                          XPSAI_PORT_ADVERT_SPEED_10G);
        }
        else if (value.u32list.list[i] == XPSAI_PORT_SPEED_40G)
        {
            info.localAdvertFdAbility |= XPSAI_PORT_ADVERT_SPEED_40G;
        }
        else if (value.u32list.list[i] == XPSAI_PORT_SPEED_50G)
        {
            info.localAdvertFdAbility |= (XPSAI_PORT_ADVERT_SPEED_50G |
                                          XPSAI_PORT_ADVERT_SPEED_25G | XPSAI_PORT_ADVERT_SPEED_10G);
        }
        else if (value.u32list.list[i] == XPSAI_PORT_SPEED_100G)
        {
            info.localAdvertFdAbility |= (XPSAI_PORT_ADVERT_SPEED_100G |
                                          XPSAI_PORT_ADVERT_SPEED_50G | XPSAI_PORT_ADVERT_SPEED_25G |
                                          XPSAI_PORT_ADVERT_SPEED_10G);
        }
        else if (value.u32list.list[i] == XPSAI_PORT_SPEED_200G)
        {
            info.localAdvertFdAbility |= (XPSAI_PORT_ADVERT_SPEED_200G |
                                          XPSAI_PORT_ADVERT_SPEED_100G | XPSAI_PORT_ADVERT_SPEED_50G |
                                          XPSAI_PORT_ADVERT_SPEED_25G | XPSAI_PORT_ADVERT_SPEED_10G);
        }
        else if (value.u32list.list[i] == XPSAI_PORT_SPEED_400G)
        {
            info.localAdvertFdAbility |= (XPSAI_PORT_ADVERT_SPEED_400G |
                                          XPSAI_PORT_ADVERT_SPEED_200G | XPSAI_PORT_ADVERT_SPEED_100G |
                                          XPSAI_PORT_ADVERT_SPEED_50G | XPSAI_PORT_ADVERT_SPEED_25G |
                                          XPSAI_PORT_ADVERT_SPEED_10G);
        }
        else
        {
            return SAI_STATUS_INVALID_ATTRIBUTE_0 + i;
        }
    }

    xpStatus = xpSaiPortCfgSet(devId, portNum, &info,
                               XPSAI_PORT_ATTR_LOCAL_ADVERT_FD_MASK);

    return xpsStatus2SaiStatus(xpStatus);
}


//Func: xpSaiGetPortAttrAdvertisedSpeed

sai_status_t xpSaiGetPortAttrAdvertisedSpeed(sai_object_id_t port_id,
                                             sai_attribute_value_t* value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    uint16_t           count      = 0;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;
    uint32_t           localAdvertFdAbility;


    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    localAdvertFdAbility = info.localAdvertFdAbility;
    for (count = 0; localAdvertFdAbility != 0; localAdvertFdAbility >>= 1)
    {
        if (localAdvertFdAbility & 0x01)
        {
            count++;
        }
    }

    if ((count > value->u32list.count) ||
        (value->u32list.count == 0))
    {
        value->u32list.count = count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    count = 0;

    if (info.localAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_10G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_10G;
    }
    if (info.localAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_25G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_25G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_10G;
    }
    if (info.localAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_40G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_40G;
    }
    if (info.localAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_50G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_50G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_25G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_10G;
    }
    if (info.localAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_100G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_100G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_50G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_25G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_10G;
    }
    if (info.localAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_200G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_200G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_100G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_50G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_25G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_10G;
    }
    if (info.localAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_400G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_400G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_200G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_100G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_50G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_25G;
        value->u32list.list[count++] = XPSAI_PORT_SPEED_10G;
    }

    value->u32list.count = count;

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiSetPortAttrAdvertisedHalfDuplexSpeed

sai_status_t xpSaiSetPortAttrAdvertisedHalfDuplexSpeed(sai_object_id_t port_id,
                                                       sai_attribute_value_t value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    uint16_t           i          = 0;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);

    for (i = 0; i < value.u32list.count; i++)
    {
        if (value.u32list.list[i] == XPSAI_PORT_SPEED_10G)
        {
            info.localAdvertHdAbility |= XPSAI_PORT_ADVERT_SPEED_10G;
        }
        else if (value.u32list.list[i] == XPSAI_PORT_SPEED_25G)
        {
            info.localAdvertHdAbility |= XPSAI_PORT_ADVERT_SPEED_25G;
        }
        else if (value.u32list.list[i] == XPSAI_PORT_SPEED_40G)
        {
            info.localAdvertHdAbility |= XPSAI_PORT_ADVERT_SPEED_40G;
        }
        else if (value.u32list.list[i] == XPSAI_PORT_SPEED_50G)
        {
            info.localAdvertHdAbility |= XPSAI_PORT_ADVERT_SPEED_50G;
        }
        else if (value.u32list.list[i] == XPSAI_PORT_SPEED_100G)
        {
            info.localAdvertHdAbility |= XPSAI_PORT_ADVERT_SPEED_100G;
        }
        else if (value.u32list.list[i] == XPSAI_PORT_SPEED_200G)
        {
            info.localAdvertHdAbility |= XPSAI_PORT_ADVERT_SPEED_200G;
        }
        else if (value.u32list.list[i] == XPSAI_PORT_SPEED_400G)
        {
            info.localAdvertHdAbility |= XPSAI_PORT_ADVERT_SPEED_400G;
        }
        else
        {
            return SAI_STATUS_INVALID_ATTRIBUTE_0 + i;
        }
    }

    xpStatus = xpSaiPortCfgSet(devId, portNum, &info,
                               XPSAI_PORT_ATTR_LOCAL_ADVERT_HD_MASK);

    return xpsStatus2SaiStatus(xpStatus);
}


//Func: xpSaiGetPortAttrAdvertisedHalfDuplexSpeed

sai_status_t xpSaiGetPortAttrAdvertisedHalfDuplexSpeed(sai_object_id_t port_id,
                                                       sai_attribute_value_t* value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    uint16_t           count      = 0;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;
    uint32_t           localAdvertHdAbility;


    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    localAdvertHdAbility = info.localAdvertHdAbility;
    for (count = 0; localAdvertHdAbility != 0; localAdvertHdAbility >>= 1)
    {
        if (localAdvertHdAbility & 0x01)
        {
            count++;
        }
    }

    if ((count > value->u32list.count) ||
        (value->u32list.count == 0))
    {
        value->u32list.count = count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    count = 0;

    if (info.localAdvertHdAbility & XPSAI_PORT_ADVERT_SPEED_10G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_10G;
    }
    if (info.localAdvertHdAbility & XPSAI_PORT_ADVERT_SPEED_25G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_25G;
    }
    if (info.localAdvertHdAbility & XPSAI_PORT_ADVERT_SPEED_40G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_40G;
    }
    if (info.localAdvertHdAbility & XPSAI_PORT_ADVERT_SPEED_50G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_50G;
    }
    if (info.localAdvertHdAbility & XPSAI_PORT_ADVERT_SPEED_100G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_100G;
    }
    if (info.localAdvertHdAbility & XPSAI_PORT_ADVERT_SPEED_200G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_200G;
    }
    if (info.localAdvertHdAbility & XPSAI_PORT_ADVERT_SPEED_400G)
    {
        value->u32list.list[count++] = XPSAI_PORT_SPEED_400G;
    }
    value->u32list.count = count;

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiSetPortAttrAdvertisedAutoNegMode

sai_status_t xpSaiSetPortAttrAdvertisedAutoNegMode(sai_object_id_t port_id,
                                                   sai_attribute_value_t value)
{
    sai_status_t saiRetVal = SAI_STATUS_NOT_IMPLEMENTED;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrAdvertisedAutoNegMode\n");

    return saiRetVal;
}


//Func: xpSaiGetPortAttrAdvertisedAutoNegMode

sai_status_t xpSaiGetPortAttrAdvertisedAutoNegMode(sai_object_id_t port_id,
                                                   sai_attribute_value_t* value)
{
    sai_status_t saiRetVal = SAI_STATUS_NOT_IMPLEMENTED;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrAdvertisedAutoNegMode\n");

    return saiRetVal;
}


sai_status_t xpSaiSetPortAttrAdvertisedFlowControlMode(sai_object_id_t port_id,
                                                       sai_attribute_value_t value)
{
    sai_status_t saiRetVal = SAI_STATUS_NOT_IMPLEMENTED;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrAdvertisedFlowControlMode\n");

    return saiRetVal;
}


//Func: xpSaiGetPortAttrAdvertisedFlowControlMode

sai_status_t xpSaiGetPortAttrAdvertisedFlowControlMode(sai_object_id_t port_id,
                                                       sai_attribute_value_t* value)
{
    sai_status_t retVal = SAI_STATUS_NOT_IMPLEMENTED;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrAdvertisedFlowControlMode\n");

    return retVal;
}

//Func: xpSaiSetPortAttrAdvertisedAsymmetricPauseMode

sai_status_t xpSaiSetPortAttrAdvertisedAsymmetricPauseMode(
    sai_object_id_t port_id,  sai_attribute_value_t value)
{
    sai_status_t retVal = SAI_STATUS_NOT_IMPLEMENTED;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrAdvertisedAsymmetricPauseMode\n");

    return retVal;
}


//Func: xpSaiGetPortAttrAdvertisedAsymmetricPauseMode

sai_status_t xpSaiGetPortAttrAdvertisedAsymmetricPauseMode(
    sai_object_id_t port_id,  sai_attribute_value_t* value)
{
    sai_status_t retVal = SAI_STATUS_NOT_IMPLEMENTED;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrAdvertisedAsymmetricPauseMode\n");

    return retVal;
}

//Func: xpSaiSetPortAttrPortVlanId

sai_status_t xpSaiSetPortAttrPortVlanId(sai_object_id_t port_id,
                                        sai_attribute_value_t value)
{
    sai_status_t retVal = SAI_STATUS_NOT_IMPLEMENTED;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrPortVlanId\n");

    return retVal;
}


//Func: xpSaiGetPortAttrPortVlanId

sai_status_t xpSaiGetPortAttrPortVlanId(sai_object_id_t port_id,
                                        sai_attribute_value_t* value)
{
    sai_status_t retVal = SAI_STATUS_NOT_IMPLEMENTED;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrPortVlanId\n");

    return retVal;
}

//Func: xpSaiSetPortAttrSpeed

sai_status_t xpSaiSetPortAttrSpeed(sai_object_id_t port_id,
                                   sai_attribute_value_t value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    info.speed = value.u32;
    xpStatus = xpSaiPortCfgSet(devId, portNum, &info, XPSAI_PORT_ATTR_SPEED_MASK);

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiGetPortAttrSpeed

sai_status_t xpSaiGetPortAttrSpeed(sai_object_id_t port_id,
                                   sai_attribute_value_t* value, bool oper_speed)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (oper_speed)
    {
        /* Get and Update port operSpeed */
        info.operSpeed =  xpSaiPortGetOperSpeed(devId, portNum);
        value->u32 = info.operSpeed;
    }
    else
    {
        /* Get config speed */
        value->u32 = info.speed;
    }

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiSetPortAttrAdminState

sai_status_t xpSaiSetPortAttrAdminState(sai_object_id_t port_id,
                                        sai_attribute_value_t value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    XP_SAI_LOG_DBG("%s the port %u\n", value.booldata ? "Enable" : "Disable",
                   xpsIntf);

    xpSaiPortInfoInit(&info);
    if (value.booldata)
    {
        info.enable = XPSAI_PORT_ENABLE;
    }
    else
    {
        info.enable = XPSAI_PORT_DISABLE;
    }
    xpStatus = xpSaiPortCfgSet(devId, portNum, &info, XPSAI_PORT_ATTR_ENABLE_MASK);

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiGetPortAttrAdminState

sai_status_t xpSaiGetPortAttrAdminState(sai_object_id_t port_id,
                                        sai_attribute_value_t* value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (info.enable == XPSAI_PORT_ENABLE)
    {
        value->booldata = true;
    }
    else
    {
        value->booldata = false;
    }

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiSetPortAttrDefaultVlan

sai_status_t xpSaiSetPortAttrDefaultVlan(sai_object_id_t port_id,
                                         sai_attribute_value_t value)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpsDevice_t  xpsDevId = xpSaiObjIdSwitchGet(port_id);
    xpsInterfaceId_t xpsIntf = XPS_INTF_INVALID_ID;

    if ((!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT)) &&
        (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_LAG)))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrDefaultVlan (xpsIntf=%u, value=%d) \n",
                   xpsIntf, value.u16);

    xpStatus = (XP_STATUS) xpsPortSetField(xpsDevId, xpsIntf, XPS_PORT_PVID,
                                           value.u16);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Default PVID %d for port %u could not be set. xpStatus = %u\n",
                       value.u16, xpsIntf, xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }
    return  xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiGetPortAttrDefaultVlan
sai_status_t xpSaiGetPortAttrDefaultVlan(sai_object_id_t portId,
                                         sai_attribute_value_t *value)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(portId);
    xpsInterfaceId_t xpsIntf = XPS_INTF_INVALID_ID;
    uint32_t pvid = 0;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrDefaultVlan\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(portId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n", xpSaiObjIdTypeGet(portId));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(portId);

    xpStatus = xpsPortGetField(xpsDevId, xpsIntf, XPS_PORT_PVID, &pvid);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Failed to get pvid on port %u\n", xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    value->u16 = (sai_uint16_t) pvid;

    return xpsStatus2SaiStatus(xpStatus);
}

sai_status_t xpSaiSetPVIDtoDB(sai_object_id_t port_id,
                              sai_attribute_value_t value)
{
    xpsInterfaceId_t xpsIntf = XPS_INTF_INVALID_ID;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpSaiPortDbEntryT *pPortEntry = NULL;
    saiStatus = xpSaiPortDbInfoGet(xpsIntf, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB\n");
        return saiStatus;
    }
    pPortEntry->pvidUserSetting = value.u16;
    return saiStatus;
}


//Func: xpSaiSetPortAttrDefaultVlanPriority

sai_status_t xpSaiSetPortAttrDefaultVlanPriority(sai_object_id_t port_id,
                                                 sai_attribute_value_t value)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpsDevice_t  xpsDevId = xpSaiObjIdSwitchGet(port_id);
    xpsInterfaceId_t xpsIntf = 0;

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrDefaultVlanPriority\n");

    xpStatus = (XP_STATUS) xpsPortSetField(xpsDevId, xpsIntf, XPS_PORT_DEFAULT_PCP,
                                           value.u8);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Default PCP %d for port %u could not be set.", value.u8,
                       xpsIntf);
    }

    return  xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiGetPortAttrDefaultVlanPriority

sai_status_t xpSaiGetPortAttrDefaultVlanPriority(sai_object_id_t portId,
                                                 sai_attribute_value_t* value)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(portId);
    xpsInterfaceId_t xpsIntf = XPS_INTF_INVALID_ID;
    uint32_t priority = 0;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrDefaultVlanPriority\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(portId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portId));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(portId);

    xpStatus = xpsPortGetField(xpsDevId, xpsIntf, XPS_PORT_DEFAULT_PCP, &priority);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Failed to get pvid on port %u\n", xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    value->u8 = (sai_uint16_t)priority;

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiSetPortAttrDropUntagged

sai_status_t xpSaiSetPortAttrDropUntagged(sai_object_id_t port_id,
                                          sai_attribute_value_t value)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS xpStatus = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsInterfaceId_t xpsIntf = XPS_INTF_INVALID_ID;
    uint32_t frameType = FRAMETYPE_MAX;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrDropUntagged\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetField(xpsDevId, xpsIntf, XPS_PORT_ACCEPTED_FRAME_TYPE,
                               &frameType);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get drop untag on port %u\n", xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (value.booldata)
    {
        if (frameType == FRAMETYPE_UN_PRI)
        {
            return SAI_STATUS_NOT_SUPPORTED;
        }
        else
        {
            frameType = FRAMETYPE_TAG;
        }
    }
    else if (frameType == FRAMETYPE_TAG)
    {
        frameType = FRAMETYPE_ALL;
    }

    xpStatus = xpsPortSetField(xpsDevId, xpsIntf, XPS_PORT_ACCEPTED_FRAME_TYPE,
                               frameType);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Failed to set drop untag on port %u\n", xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return saiStatus;
}

//Func: xpSaiGetPortAttrDropUntagged

sai_status_t xpSaiGetPortAttrDropUntagged(sai_object_id_t port_id,
                                          sai_attribute_value_t *value)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsInterfaceId_t xpsIntf = XPS_INTF_INVALID_ID;
    uint32_t frameType = FRAMETYPE_MAX;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrDropUntagged\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetField(xpsDevId, xpsIntf, XPS_PORT_ACCEPTED_FRAME_TYPE,
                               &frameType);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Failed to get drop untag on port %u\n", xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    value->booldata = (frameType == FRAMETYPE_TAG);

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiSetPortAttrDropTagged

sai_status_t xpSaiSetPortAttrDropTagged(sai_object_id_t port_id,
                                        sai_attribute_value_t value)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS xpStatus = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsInterfaceId_t xpsIntf = XPS_INTF_INVALID_ID;
    uint32_t frameType = FRAMETYPE_MAX;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrDropTagged\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetField(xpsDevId, xpsIntf, XPS_PORT_ACCEPTED_FRAME_TYPE,
                               &frameType);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get drop tag on port %u\n", xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (value.booldata)
    {
        if (frameType == FRAMETYPE_TAG)
        {
            return SAI_STATUS_NOT_SUPPORTED;
        }
        else
        {
            frameType = FRAMETYPE_UN_PRI;
        }
    }
    else if (frameType == FRAMETYPE_UN_PRI)
    {
        frameType = FRAMETYPE_ALL;
    }

    xpStatus = xpsPortSetField(xpsDevId, xpsIntf, XPS_PORT_ACCEPTED_FRAME_TYPE,
                               frameType);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Failed to set drop tagon port %u", xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return saiStatus;
}

//Func: xpSaiGetPortAttrDropTagged

sai_status_t xpSaiGetPortAttrDropTagged(sai_object_id_t port_id,
                                        sai_attribute_value_t *value)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsInterfaceId_t xpsIntf = XPS_INTF_INVALID_ID;
    uint32_t frameType = FRAMETYPE_MAX;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrDropTagged\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetField(xpsDevId, xpsIntf, XPS_PORT_ACCEPTED_FRAME_TYPE,
                               &frameType);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Failed to get drop tag on port %u", xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    value->booldata = (frameType == FRAMETYPE_UN_PRI);

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiSetPortLagConfig
sai_status_t xpSaiSetPortLagConfig(sai_object_id_t portId,
                                   xpSaiPortLagInfo_t lagConfig)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t xpsIntf = 0;
    sai_attribute_t attr;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortLagConfig\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(portId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portId));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(portId);

    memset(&attr, 0, sizeof(attr));

    /* Apply LAG config for this port */
    attr.value.u16 = lagConfig.portVlanId;
    saiStatus = xpSaiSetPortAttrDefaultVlan(portId, attr.value);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not set pvid for port (%lu).\n",
                       xpSaiObjIdTypeGet(portId));
        return saiStatus;
    }

    attr.value.u8 = lagConfig.defaultVlanPriority;
    saiStatus = xpSaiSetPortAttrDefaultVlanPriority(portId, attr.value);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not set default VLAN priority for port (%lu).\n",
                       xpSaiObjIdTypeGet(portId));
        return saiStatus;
    }

    attr.value.booldata = lagConfig.dropUntagged;
    saiStatus = xpSaiSetPortAttrDropUntagged(portId, attr.value);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not set DropUntagged attribue for port (%lu).\n",
                       xpSaiObjIdTypeGet(portId));
        return saiStatus;
    }

    attr.value.booldata = lagConfig.dropTagged;
    saiStatus = xpSaiSetPortAttrDropTagged(portId, attr.value);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not set Untagged attribue for port (%lu).\n",
                       xpSaiObjIdTypeGet(portId));
        return saiStatus;
    }

    saiStatus = xpSaiSetPortAttrFdbLearningModeHandle(xpsIntf, lagConfig.saMissCmd);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not set SA miss command for port (%lu).\n",
                       xpSaiObjIdTypeGet(portId));
        return saiStatus;
    }

    return saiStatus;
}

//Func: xpSaiGetPortLagConfig
sai_status_t xpSaiGetPortLagConfig(sai_object_id_t portId,
                                   xpSaiPortLagInfo_t *lagConfig)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortLagConfig\n");

    memset(&attr, 0, sizeof(attr));

    saiStatus = xpSaiGetPortAttrDefaultVlan(portId, &(attr.value));
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not get pvid for port (%lu).\n",
                       xpSaiObjIdTypeGet(portId));
        return saiStatus;
    }
    lagConfig->portVlanId = attr.value.u16;

    saiStatus = xpSaiGetPortAttrDefaultVlanPriority(portId, &(attr.value));
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not get default VLAN priority for port (%lu).\n",
                       xpSaiObjIdTypeGet(portId));
        return saiStatus;
    }
    lagConfig->defaultVlanPriority = attr.value.u8;

    saiStatus = xpSaiGetPortAttrDropUntagged(portId, &(attr.value));
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not get DropUntagged attribue for port (%lu).\n",
                       xpSaiObjIdTypeGet(portId));
        return saiStatus;
    }
    lagConfig->dropUntagged = attr.value.booldata;

    saiStatus = xpSaiGetPortAttrDropTagged(portId, &(attr.value));
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not get DropUntagged attribue for port (%lu).\n",
                       xpSaiObjIdTypeGet(portId));
        return saiStatus;
    }
    lagConfig->dropTagged = attr.value.booldata;

    return saiStatus;
}

//Func: xpSaiSetPortAttrTxEnable

sai_status_t xpSaiSetPortAttrTxEnable(sai_object_id_t port_id,
                                      sai_attribute_value_t value)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t portId = XPS_INTF_INVALID_ID;
    xpSaiPortDbEntryT *pPortEntry = NULL;
    XP_STATUS          xpStatus = XP_NO_ERR;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    uint8_t            qNum;
    GT_STATUS          rc = GT_OK;

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Invalid object type received: typeId=%u.\n",
                       xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    portId = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    saiStatus = xpSaiPortDbInfoGet(portId, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Coulnd't find port in SAI port DB: portId=%u.\n", portId);
        return saiStatus;
    }

    if (pPortEntry->txEnable == value.booldata)
    {
        return SAI_STATUS_SUCCESS;
    }

    xpStatus = xpsPortGetDevAndPortNumFromIntf(portId, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       portId);
        return xpsStatus2SaiStatus(xpStatus);
    }

    GT_BOOL enable = (value.booldata) ? GT_TRUE : GT_FALSE;

    /* enable/disable pkt transmission in all queues */
    for (qNum = 0; qNum < XPS_MAX_QUEUES_PER_PORT; qNum++)
    {
        rc = cpssHalPortTxQueueTxEnableSet(devId, portNum, qNum, enable);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Error: cpssHalPortTxQueueTxEnableSet, port: %d, queue: %d, rc: %d\n",
                           portNum, qNum, rc);
            return cpssStatus2SaiStatus(rc);
        }
    }

    pthread_mutex_lock(&gSaiPortDbMutex);
    pPortEntry->txEnable = value.booldata;
    pthread_mutex_unlock(&gSaiPortDbMutex);

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetPortAttrTxEnable

sai_status_t xpSaiGetPortAttrTxEnable(sai_object_id_t port_id,
                                      sai_attribute_value_t* value)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t portId = XPS_INTF_INVALID_ID;
    xpSaiPortDbEntryT *pPortEntry = NULL;

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("NULL pointer provided as an argument.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Invalid object type received: typeId=%u.\n",
                       xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    portId = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    saiStatus = xpSaiPortDbInfoGet(portId, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Coulnd't find port in SAI port DB: portId=%u.\n", portId);
        return saiStatus;
    }

    pthread_mutex_lock(&gSaiPortDbMutex);
    value->booldata = pPortEntry->txEnable;
    pthread_mutex_unlock(&gSaiPortDbMutex);

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief The purpose of this function is to store current port config before adding to LAG.
   After remowing from LAG we can restore this config using xpSaiRestorePortConfigDB
 *
 * \param [in] sai_object_id_t portId - port ID
 * \param [in] xpSaiPortLagInfo_t lagConfig - config to store
 *
 * \return sai_status_t
 */
sai_status_t xpSaiStoreCurrentPortConfigDB(sai_object_id_t portId,
                                           xpSaiPortLagInfo_t lagConfig)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t xpsIntf = XPS_INTF_INVALID_ID;
    xpSaiPortLagInfo_t *pSaiPortLagInfoEntry = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiStoreCurrentPortConfigDB\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(portId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n", xpSaiObjIdTypeGet(portId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(portId);
    saiStatus = xpSaiPortLagInfoGet(xpsIntf, &pSaiPortLagInfoEntry);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not get LAG info from DB for port (%lu). Please add port to LAG before using xpSaiSetPortLagConfig function.\n",
                       xpSaiObjIdTypeGet(portId));
        return saiStatus;
    }

    pSaiPortLagInfoEntry->portVlanId = lagConfig.portVlanId;
    pSaiPortLagInfoEntry->defaultVlanPriority = lagConfig.defaultVlanPriority;
    pSaiPortLagInfoEntry->dropUntagged = lagConfig.dropUntagged;
    pSaiPortLagInfoEntry->dropTagged = lagConfig.dropTagged;

    return saiStatus;
}

/**
 * \brief The purpose of this function is to restore port config
 *        after removing from LAG.
 *
 * \param [in] sai_object_id_t portId - port ID
 *
 * \return sai_status_t
 */
sai_status_t xpSaiRestorePortConfigDB(sai_object_id_t portId)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS xpsStatus = XP_NO_ERR;
    xpsInterfaceId_t xpsIntf = XPS_INTF_INVALID_ID;
    xpSaiPortLagInfo_t *pSaiPortLagInfoEntry = NULL;
    xpsVlan_t xpsVlanId = 0;
    xpsDevice_t devId = xpSaiObjIdSwitchGet(portId);
    sai_attribute_t attr;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortLagConfig\n");

    memset(&attr, 0, sizeof(attr));

    if (!XDK_SAI_OBJID_TYPE_CHECK(portId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n", xpSaiObjIdTypeGet(portId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(portId);
    saiStatus = xpSaiPortLagInfoGet(xpsIntf, &pSaiPortLagInfoEntry);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not get LAG info from DB for port (%lu). Please add port to LAG before using xpSaiRestorePortConfigDB function.\n",
                       xpSaiObjIdTypeGet(portId));
        return saiStatus;
    }

    /* Apply stored port config for this port */
    attr.value.u16 = pSaiPortLagInfoEntry->portVlanId;
    saiStatus = xpSaiSetPortAttrDefaultVlan(portId, attr.value);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        /* Stored VLAN could be already deleted. Trying to set default VLAN */
        XP_SAI_LOG_WARNING("Could not restore pvid %u for port (%lu). Trying to set default pvid. \n",
                           attr.value.u16, xpSaiObjIdTypeGet(portId));
        xpsStatus = xpsVlanGetDefault(devId, &xpsVlanId);
        if (XP_NO_ERR != xpsStatus)
        {
            XP_SAI_LOG_ERR("Could not set default VLAN.\n");
            return xpsStatus2SaiStatus(xpsStatus);
        }

        saiStatus = xpSaiSetPortAttrDefaultVlan(portId, attr.value);
        if (SAI_STATUS_SUCCESS != saiStatus)
        {
            XP_SAI_LOG_ERR("Could not set PVID for port (%lu).\n",
                           xpSaiObjIdTypeGet(portId));
            return saiStatus;
        }
    }

    attr.value.u8 = pSaiPortLagInfoEntry->defaultVlanPriority;
    saiStatus = xpSaiSetPortAttrDefaultVlanPriority(portId,  attr.value);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not set default VLAN priority for port (%lu).\n",
                       xpSaiObjIdTypeGet(portId));
        return saiStatus;
    }

    attr.value.booldata = pSaiPortLagInfoEntry->dropUntagged;
    saiStatus = xpSaiSetPortAttrDropUntagged(portId,  attr.value);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not set DropUntagged attribue for port (%lu).\n",
                       xpSaiObjIdTypeGet(portId));
        return saiStatus;
    }

    attr.value.booldata = pSaiPortLagInfoEntry->dropTagged;
    saiStatus = xpSaiSetPortAttrDropTagged(portId,  attr.value);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not set DropUntagged attribue for port (%lu).\n",
                       xpSaiObjIdTypeGet(portId));
        return saiStatus;
    }

    return saiStatus;
}


//Func: xpSaiSetPortAttrInternalLoopbackMode

sai_status_t xpSaiSetPortAttrInternalLoopbackMode(sai_object_id_t port_id,
                                                  sai_attribute_value_t value)
{
    XP_STATUS          xpStatus = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    if (SAI_PORT_INTERNAL_LOOPBACK_MODE_MAC == value.s32)
    {
        info.loopbackMode = XPSAI_PORT_LOOPBACK_MODE_MAC;
    }
    else if (SAI_PORT_INTERNAL_LOOPBACK_MODE_PHY == value.s32)
    {
        info.loopbackMode = XPSAI_PORT_LOOPBACK_MODE_PHY;
    }
    else
    {
        info.loopbackMode = XPSAI_PORT_LOOPBACK_MODE_NONE;
    }

    xpStatus = xpSaiPortCfgSet(devId, portNum, &info,
                               XPSAI_PORT_ATTR_LOOPBACK_MASK);

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiGetPortAttrInternalLoopbackMode

sai_status_t xpSaiGetPortAttrInternalLoopbackMode(sai_object_id_t port_id,
                                                  sai_attribute_value_t* value)
{
    XP_STATUS          xpStatus  = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (info.loopbackMode == XPSAI_PORT_LOOPBACK_MODE_MAC)
    {
        value->s32 = SAI_PORT_INTERNAL_LOOPBACK_MODE_MAC;
    }
    else if (info.loopbackMode == XPSAI_PORT_LOOPBACK_MODE_PHY)
    {
        value->s32 = SAI_PORT_INTERNAL_LOOPBACK_MODE_PHY;
    }
    else
    {
        value->s32 = SAI_PORT_INTERNAL_LOOPBACK_MODE_NONE;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrFdbLearningMode

sai_status_t xpSaiSetPortAttrFdbLearningMode(sai_object_id_t port_id,
                                             sai_attribute_value_t value)
{
    xpsInterfaceId_t xpsIntf = 0;
    xpsPktCmd_e pktCmd = XP_PKTCMD_DROP;
    sai_status_t retValSai = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrFdbLearningMode\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    /** Drop packets with unknown source MAC. Do not learn. Do not forward */
    if (value.s32 == SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DROP)
    {
        pktCmd = XP_PKTCMD_DROP;
    }
    else if (value.s32 ==
             SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DISABLE) /** Do not learn unknown source MAC. Forward based on destination MAC */
    {
        pktCmd = XP_PKTCMD_FWD;
    }
    else if (value.s32 ==
             SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW) /** Hardware learning. Learn source MAC. Forward based on destination MAC */
    {
        pktCmd = XP_PKTCMD_FWD_MIRROR;
    }
    else if (value.s32 ==
             SAI_BRIDGE_PORT_FDB_LEARNING_MODE_CPU_TRAP) /** Software learning. Forward based on destination MAC */
    {
        pktCmd = XP_PKTCMD_TRAP;
    }
    else
    {
        XP_SAI_LOG_ERR("Currently this learning mode is not supported.\n");
        return SAI_STATUS_NOT_SUPPORTED;
    }

    retValSai = xpSaiSetPortAttrFdbLearningModeHandle(xpsIntf, pktCmd);
    if (retValSai != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiSetPortAttrFdbLearningModeHandle failed with error %d\n",
                       retValSai);
        return retValSai;
    }

    return  SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrFdbLearningMode

sai_status_t xpSaiGetPortAttrFdbLearningMode(sai_object_id_t port_id,
                                             sai_attribute_value_t* value, uint32_t attr_index)
{
    XP_STATUS   xpStatus = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsInterfaceId_t       xpsIntf = 0;
    uint32_t pktCmd = 0;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrFdbLearningMode\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = (XP_STATUS)xpsPortGetField(xpsDevId, xpsIntf, XPS_PORT_MAC_SAMISSCMD,
                                          &pktCmd);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get FdbLearningMode-type for Port:: %u", xpsIntf);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    if (pktCmd == XP_PKTCMD_DROP)
    {
        value->s32 = SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DROP;
    }
    else if (pktCmd == XP_PKTCMD_FWD)
    {
        value->s32 = SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DISABLE;
    }
    else if (pktCmd == XP_PKTCMD_FWD_MIRROR)
    {
        value->s32 = SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW;
    }
    else if (pktCmd == XP_PKTCMD_TRAP)
    {
        value->s32 = SAI_BRIDGE_PORT_FDB_LEARNING_MODE_CPU_TRAP;
    }
    else
    {
        XP_SAI_LOG_ERR("Failed to get valid SAI-FdbLearningMode-type for Port:: %u",
                       xpsIntf);
        return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(attr_index);
    }

    return  xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiSetPortAttrUpdateDscp

sai_status_t xpSaiSetPortAttrUpdateDscp(sai_object_id_t port_oid,
                                        sai_attribute_value_t value)
{
    XP_STATUS        xpStatus  = XP_NO_ERR;
    GT_STATUS        rc        = GT_OK;
    xpsInterfaceId_t xpsIntfId = 0;
    xpsDevice_t      xpDevId   = 0;
    xpsPort_t        portNum   = 0;
    GT_BOOL          enable    = (value.u32) ? GT_TRUE : GT_FALSE;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrUpdateDscp\n");

    /* Get the port id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(port_oid, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_oid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntfId = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_oid);

    /* Fetch device and port information for this particular port interface */
    if ((xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntfId, &xpDevId,
                                                    &portNum)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Fetch device and port information failed, interface id(%d)",
                       xpsIntfId);
        return xpsStatus2SaiStatus(xpStatus);
    }

    rc = cpssHalPortEgressQosDscpMappingEnableSet(xpDevId, portNum, enable);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set egress DSCP mapping enable port: %d\n",
                       portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    return  SAI_STATUS_SUCCESS;
}
//Func: xpSaiSetPortAttrMtu

sai_status_t xpSaiSetPortAttrMtu(sai_object_id_t port_id,
                                 sai_attribute_value_t value)
{
    XP_STATUS   xpStatus = XP_NO_ERR;
    XP_STATUS   xpStatus2 = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsInterfaceId_t       xpsIntf = 0;
    uint32_t               get_mtu = 0;

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);
    if (!WARM_RESTART)
    {
        xpStatus = xpsMtuGetInterfaceMtuSize(xpsDevId, xpsIntf, &get_mtu);
        if (XP_NO_ERR != xpStatus)
        {
            XP_SAI_LOG_ERR("Could not get MTU for port %u.", xpsIntf);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    xpStatus = xpsMtuSetInterfaceMtuSize(xpsDevId, xpsIntf, value.u32);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("MTU %d could not be set to port %u.", value.u32, xpsIntf);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsPortUpdateTunnelMtu(xpsDevId, xpsIntf);
    if (XP_NO_ERR != xpStatus)
    {
        if (get_mtu != value.u32)
        {
            xpStatus2 = xpsMtuSetInterfaceMtuSize(xpsDevId, xpsIntf, get_mtu);
            if (XP_NO_ERR != xpStatus2)
            {
                XP_SAI_LOG_ERR("MTU %d could not be set to port %u.", get_mtu, xpsIntf);
                return  xpsStatus2SaiStatus(xpStatus2);
            }
        }
        XP_SAI_LOG_ERR("MTU %d could not be set to tunnel %u.", value.u32, xpsIntf);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsMacSetRxMaxFrmLen(xpsDevId, (uint8_t) xpsIntf, value.u32);
    if (XP_NO_ERR != xpStatus)
    {
        xpStatus2 = xpsMtuSetInterfaceMtuSize(xpsDevId, xpsIntf, get_mtu);
        if (XP_NO_ERR != xpStatus2)
        {
            XP_SAI_LOG_ERR("MTU %d could not be set to port %u.", get_mtu, xpsIntf);
            return  xpsStatus2SaiStatus(xpStatus2);
        }
        XP_SAI_LOG_ERR("MTU/MRU %d could not be set to port %u.\n", value.u32, xpsIntf);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    return  xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiGetPortAttrMtu

sai_status_t xpSaiGetPortAttrMtu(sai_object_id_t port_id,
                                 sai_attribute_value_t* value)
{
    XP_STATUS        xpStatus = XP_NO_ERR;
    xpsDevice_t      xpsDevId = xpSaiGetDevId();
    xpsInterfaceId_t xpsIntf  = 0;
    uint32_t         mtu      = 0;

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsMtuGetInterfaceMtuSize(xpsDevId, xpsIntf, &mtu);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Failed to receive MTU size for port %u.", xpsIntf);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    value->u32 = mtu;

    return  xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiSetPortAttrIngressMirrorSession

sai_status_t xpSaiSetPortAttrIngressMirrorSession(sai_object_id_t port_id,
                                                  sai_attribute_value_t value)
{
    sai_status_t       saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT *pPortEntry  = NULL;
    xpsInterfaceId_t   portIntf    = XPS_INTF_INVALID_ID;
    uint32_t           i           = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    // Verify port oid
    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    portIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    saiStatus = xpSaiPortDbInfoGet(portIntf, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB");
        return saiStatus;
    }

    if (value.objlist.count > XPSAI_DIRECTED_MIRROR_SESSION_NUM)
    {
        XP_SAI_LOG_ERR("Mirror session count %d exceeds support limit %d",
                       value.objlist.count, XPSAI_DIRECTED_MIRROR_SESSION_NUM);
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    // If session OID taken from attribute value is SAI_NULL_OBJECT_ID
    // then mirroring should be disabled on this port. In this case take
    // the session OID from the DB.
    // also on every set call, we should remove old list of sessison IDs
    // associated with the port and apply this new list. as we don't have
    // any other way to remove an individual session ID. application will
    // invoke a set call with a list of OBJids to set(without the OBJID they want to remove).

    for (i = 0; i < XPSAI_DIRECTED_MIRROR_SESSION_NUM; i++)
    {
        if (pPortEntry->ingressMirrorSessionOid[i] != SAI_NULL_OBJECT_ID)
        {
            saiStatus = xpSaiMirrorSessionPortMirrorEnable(portIntf,
                                                           pPortEntry->ingressMirrorSessionOid[i], true, false);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not disable mirror session on port\n");
                return saiStatus;
            }

            pthread_mutex_lock(&gSaiPortDbMutex);
            pPortEntry->ingressMirrorSessionOid[i] = SAI_NULL_OBJECT_ID;
            pthread_mutex_unlock(&gSaiPortDbMutex);
        }
    }

    /*incase list is not empty, associate all those mirror sessions to port*/
    if (value.objlist.count > 0 && (value.objlist.list[0] != SAI_NULL_OBJECT_ID))
    {
        for (i = 0; i < value.objlist.count; i++)
        {
            saiStatus = xpSaiMirrorSessionPortMirrorEnable(portIntf, value.objlist.list[i],
                                                           true, true);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not enable mirror session on port\n");
                return saiStatus;
            }

            // Save OID into first free entry
            pthread_mutex_lock(&gSaiPortDbMutex);
            pPortEntry->ingressMirrorSessionOid[i] = value.objlist.list[i];
            pthread_mutex_unlock(&gSaiPortDbMutex);
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrIngressMirrorSession

sai_status_t xpSaiGetPortAttrIngressMirrorSession(sai_object_id_t port_id,
                                                  sai_attribute_value_t* value)
{
    sai_status_t       saiStatus    = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT *pPortEntry   = NULL;
    xpsInterfaceId_t   portIntf     = XPS_INTF_INVALID_ID;
    uint32_t           sessionCount = 0;
    uint32_t           i            = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    // Verify port oid
    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    portIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    saiStatus = xpSaiPortDbInfoGet(portIntf, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB");
        return saiStatus;
    }

    pthread_mutex_lock(&gSaiPortDbMutex);
    for (sessionCount = 0, i = 0; i < XPSAI_DIRECTED_MIRROR_SESSION_NUM; i++)
    {
        if (pPortEntry->ingressMirrorSessionOid[i] != SAI_NULL_OBJECT_ID)
        {
            ++sessionCount;
        }
    }

    if (sessionCount > value->objlist.count)
    {
        pthread_mutex_unlock(&gSaiPortDbMutex);
        value->objlist.count = sessionCount;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if ((sessionCount != 0) && (value->objlist.list == NULL))
    {
        pthread_mutex_unlock(&gSaiPortDbMutex);
        XP_SAI_LOG_ERR("Invalid parameters received");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (sessionCount = 0, i = 0; i < XPSAI_DIRECTED_MIRROR_SESSION_NUM; i++)
    {
        if (pPortEntry->ingressMirrorSessionOid[i] != SAI_NULL_OBJECT_ID)
        {
            value->objlist.list[sessionCount++] = pPortEntry->ingressMirrorSessionOid[i];
        }
    }
    value->objlist.count = sessionCount;
    pthread_mutex_unlock(&gSaiPortDbMutex);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrEgressMirrorSession

sai_status_t xpSaiSetPortAttrEgressMirrorSession(sai_object_id_t port_id,
                                                 sai_attribute_value_t value)
{
    sai_status_t       saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT *pPortEntry  = NULL;
    xpsInterfaceId_t   portIntf    = XPS_INTF_INVALID_ID;
    uint32_t           i           = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    // Verify port oid
    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    portIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    saiStatus = xpSaiPortDbInfoGet(portIntf, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB");
        return saiStatus;
    }

    if (value.objlist.count > XPSAI_DIRECTED_MIRROR_SESSION_NUM)
    {
        XP_SAI_LOG_ERR("Mirror session count %d exceeds support limit %d",
                       value.objlist.count, XPSAI_DIRECTED_MIRROR_SESSION_NUM);
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    // If session OID taken from attribute value is SAI_NULL_OBJECT_ID
    // then mirroring should be disabled on this port. In this case take
    // the session OID from the DB.
    // also on every set call, we should remove old list of sessison IDs
    // associated with the port and apply this new list. as we don't have
    // any other way to remove an individual session ID. application will
    // invoke a set call with a list of OBJids to set(without the OBJID they want to remove).
    for (i = 0; i < XPSAI_DIRECTED_MIRROR_SESSION_NUM; i++)
    {
        if (pPortEntry->egressMirrorSessionOid[i] != SAI_NULL_OBJECT_ID)
        {
            saiStatus = xpSaiMirrorSessionPortMirrorEnable(portIntf,
                                                           pPortEntry->egressMirrorSessionOid[i], false, false);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not disable mirror session on port\n");
                return saiStatus;
            }

            pthread_mutex_lock(&gSaiPortDbMutex);
            pPortEntry->egressMirrorSessionOid[i] = SAI_NULL_OBJECT_ID;
            pthread_mutex_unlock(&gSaiPortDbMutex);
        }
    }

    /*incase list is not empty, associate all those mirror sessions to port*/
    if (value.objlist.count > 0 && (value.objlist.list[0] != SAI_NULL_OBJECT_ID))
    {
        for (i = 0; i < value.objlist.count; i++)
        {
            saiStatus = xpSaiMirrorSessionPortMirrorEnable(portIntf, value.objlist.list[i],
                                                           false, true);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not enable mirror session on port\n");
                return saiStatus;
            }

            // Save OID into first free entry
            pthread_mutex_lock(&gSaiPortDbMutex);
            pPortEntry->egressMirrorSessionOid[i] = value.objlist.list[i];
            pthread_mutex_unlock(&gSaiPortDbMutex);
        }
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetPortAttrEgressMirrorSession

sai_status_t xpSaiGetPortAttrEgressMirrorSession(sai_object_id_t port_id,
                                                 sai_attribute_value_t* value)
{
    sai_status_t       saiStatus    = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT *pPortEntry   = NULL;
    xpsInterfaceId_t   portIntf     = XPS_INTF_INVALID_ID;
    uint32_t           sessionCount = 0;
    uint32_t           i            = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    // Verify port oid
    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    portIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    saiStatus = xpSaiPortDbInfoGet(portIntf, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB");
        return saiStatus;
    }

    pthread_mutex_lock(&gSaiPortDbMutex);
    for (sessionCount = 0, i = 0; i < XPSAI_DIRECTED_MIRROR_SESSION_NUM; i++)
    {
        if (pPortEntry->egressMirrorSessionOid[i] != SAI_NULL_OBJECT_ID)
        {
            ++sessionCount;
        }
    }

    if (sessionCount > value->objlist.count)
    {
        pthread_mutex_unlock(&gSaiPortDbMutex);
        value->objlist.count = sessionCount;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if ((sessionCount != 0) && (value->objlist.list == NULL))
    {
        pthread_mutex_unlock(&gSaiPortDbMutex);
        XP_SAI_LOG_ERR("Invalid parameters received");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (sessionCount = 0, i = 0; i < XPSAI_DIRECTED_MIRROR_SESSION_NUM; i++)
    {
        if (pPortEntry->egressMirrorSessionOid[i] != SAI_NULL_OBJECT_ID)
        {
            value->objlist.list[sessionCount++] = pPortEntry->egressMirrorSessionOid[i];
        }
    }
    value->objlist.count = sessionCount;
    pthread_mutex_unlock(&gSaiPortDbMutex);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortStatisticInfo

XP_STATUS xpSaiGetPortStatisticInfo(sai_int32_t port_id,
                                    xpSaiPortStatisticDbEntryT **ppPortStatisticInfo)
{
    xpSaiPortStatisticDbEntryT *portStatisticKey;
    XP_STATUS xpsRetVal = XP_NO_ERR;

    xpsRetVal = xpsStateHeapMalloc(sizeof(xpSaiPortStatisticDbEntryT),
                                   (void**)&portStatisticKey);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate port stats structure\n");
        return xpsRetVal;
    }

    memset(portStatisticKey, 0, sizeof(xpSaiPortStatisticDbEntryT));

    portStatisticKey->portNum = port_id;
    /* Retrieve the corresponding state from sai port statistic db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT,
                                        gXpSaiPortStatisticDbHandle, (xpsDbKey_t)portStatisticKey,
                                        (void**)ppPortStatisticInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai port statistic data |retVal:%d\n",
                       xpsRetVal);
        xpsStateHeapFree((void*)portStatisticKey);
        return xpsRetVal;
    }

    if (*ppPortStatisticInfo == NULL)
    {
        XP_SAI_LOG_ERR("SAI PORT statistic DB entry is not created for this port %d\n",
                       port_id);
        xpsStateHeapFree((void*)portStatisticKey);
        return XP_ERR_NOT_INITED;
    }

    xpsStateHeapFree((void*)portStatisticKey);
    return XP_NO_ERR;
}

//Func: xpSaiInsertPortStatisticDbEntry

XP_STATUS xpSaiInsertPortStatisticDbEntry(sai_int32_t port_id)
{
    xpSaiPortStatisticDbEntryT *portStatisticKey;
    xpSaiPortStatisticDbEntryT *ppPortStatisticInfo = NULL;
    XP_STATUS           xpsRetVal = XP_NO_ERR;

    xpsRetVal = xpsStateHeapMalloc(sizeof(xpSaiPortStatisticDbEntryT),
                                   (void**)&portStatisticKey);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate port stats structure\n");
        return xpsRetVal;
    }

    memset(portStatisticKey, 0, sizeof(xpSaiPortStatisticDbEntryT));

    portStatisticKey->portNum = port_id;
    /* Retrieve the corresponding state from sai port statistic db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT,
                                        gXpSaiPortStatisticDbHandle, (xpsDbKey_t)portStatisticKey,
                                        (void**)&ppPortStatisticInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai port statistic data |retVal:%d\n",
                       xpsRetVal);
        xpsStateHeapFree((void*)portStatisticKey);
        return xpsRetVal;
    }

    if (ppPortStatisticInfo == NULL)
    {
        /* Create a new sai port statistic db */
        if ((xpsRetVal = xpsStateHeapMalloc(sizeof(xpSaiPortStatisticDbEntryT),
                                            (void**)&ppPortStatisticInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Heap Allocation for sai port statistic db failed |retVal:%d\n",
                           xpsRetVal);
            xpsStateHeapFree((void*)portStatisticKey);
            return xpsRetVal;
        }
        memset(ppPortStatisticInfo, 0, sizeof(xpSaiPortStatisticDbEntryT));

        ppPortStatisticInfo->portNum = port_id;
        /* Insert the state into the database */
        if ((xpsRetVal = xpsStateInsertData(XP_SCOPE_DEFAULT,
                                            gXpSaiPortStatisticDbHandle, (void*)ppPortStatisticInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to insert sai port statistic data |retVal: %d\n",
                           xpsRetVal);
            xpsStateHeapFree((void*)ppPortStatisticInfo);
            xpsStateHeapFree((void*)portStatisticKey);
            return xpsRetVal;
        }
    }

    xpsStateHeapFree((void*)portStatisticKey);

    return XP_NO_ERR;
}

//Func: xpSaiRemovePortStatisticDbEntry

XP_STATUS xpSaiRemovePortStatisticDbEntry(sai_int32_t port_id)
{
    xpSaiPortStatisticDbEntryT *portStatisticKey;
    xpSaiPortStatisticDbEntryT *ppPortStatisticInfo = NULL;
    XP_STATUS           xpsRetVal = XP_NO_ERR;

    xpsRetVal = xpsStateHeapMalloc(sizeof(xpSaiPortStatisticDbEntryT),
                                   (void**)&portStatisticKey);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate port stats structure\n");
        return xpsRetVal;
    }

    memset(portStatisticKey, 0, sizeof(xpSaiPortStatisticDbEntryT));

    portStatisticKey->portNum = port_id;

    /* Retrieve the corresponding port statistics from sai port statistic db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT,
                                        gXpSaiPortStatisticDbHandle, (xpsDbKey_t)portStatisticKey,
                                        (void**)&ppPortStatisticInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai port statistic data |retVal:%d\n",
                       xpsRetVal);
        xpsStateHeapFree((void*)portStatisticKey);
        return xpsRetVal;
    }

    if (ppPortStatisticInfo == NULL)
    {
        XP_SAI_LOG_ERR("Entry doesn't exist for this sai port statistics data |retVal:%d\n",
                       xpsRetVal);
        xpsStateHeapFree((void*)portStatisticKey);
        return xpsRetVal;
    }
    else
    {
        xpsRetVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, gXpSaiPortStatisticDbHandle,
                                       (xpsDbKey_t)portStatisticKey, (void**)&ppPortStatisticInfo);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not delete entry from Port Statistic DB error %d\n",
                           xpsRetVal);
            xpsStateHeapFree((void*)portStatisticKey);
            return xpsRetVal;
        }
    }

    xpsStateHeapFree((void*)portStatisticKey);
    return XP_NO_ERR;
}

//Func: xpSaiGetPortQosInfo

XP_STATUS xpSaiGetPortQosInfo(sai_object_id_t port_id,
                              xpSaiPortQosInfo_t **ppPortQosInfo)
{
    xpSaiPortQosInfo_t portQosInfoKey;
    XP_STATUS xpsRetVal = XP_NO_ERR;

    memset(&portQosInfoKey, 0, sizeof(xpSaiPortQosInfo_t));

    portQosInfoKey.portId = port_id;
    /* Retrieve the corresponding state from sai port qos db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, portQosDbHandle,
                                        (xpsDbKey_t)&portQosInfoKey, (void**)ppPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai port qos data |retVal:%d\n",
                       xpsRetVal);
        return xpsRetVal;
    }
    if (*ppPortQosInfo == NULL)
    {
        XP_SAI_LOG_DBG("SAI QOS DB entry is not created for this port %d\n", port_id);
        return XP_ERR_NOT_INITED;
    }
    return XP_NO_ERR;
}

//Func: xpSaiInsertPortQosDbEntry

XP_STATUS xpSaiInsertPortQosDbEntry(sai_object_id_t port_id)
{
    xpSaiPortQosInfo_t  portQosInfoKey;
    xpSaiPortQosInfo_t  *ppPortQosInfo = NULL;
    XP_STATUS           xpsRetVal = XP_NO_ERR;

    memset(&portQosInfoKey, 0, sizeof(xpSaiPortQosInfo_t));

    portQosInfoKey.portId = port_id;
    /* Retrieve the corresponding state from sai port qos db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, portQosDbHandle,
                                        (xpsDbKey_t)&portQosInfoKey, (void**)&ppPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai port qos data |retVal:%d\n",
                       xpsRetVal);
        return xpsRetVal;
    }
    if (ppPortQosInfo == NULL)
    {
        /* Create a new sai port qos db */
        if ((xpsRetVal = xpsStateHeapMalloc(sizeof(xpSaiPortQosInfo_t),
                                            (void**)&ppPortQosInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Heap Allocation for sai port qos db failed |retVal:%d\n",
                           xpsRetVal);
            return xpsRetVal;
        }
        memset(ppPortQosInfo, 0, sizeof(xpSaiPortQosInfo_t));

        ppPortQosInfo->portId = port_id;
        //ppPortQosInfo->schedObjId = (uint64_t)SAI_OBJECT_TYPE_SCHEDULER << 48 | 0ULL << 40 | 0xffffffffff;
        //ppPortQosInfo->policerId = (uint64_t)SAI_OBJECT_TYPE_POLICER << 48 | 0ULL << 40 | 0xffffffffff;
        //ppPortQosInfo->floodScPolicerObjId = (uint64_t)SAI_OBJECT_TYPE_POLICER << 48 | 0ULL << 40 | 0xffffffffff;
        //ppPortQosInfo->bcastScPolicerObjId = (uint64_t)SAI_OBJECT_TYPE_POLICER << 48 | 0ULL << 40 | 0xffffffffff;
        //ppPortQosInfo->mcastScPolicerObjId = (uint64_t)SAI_OBJECT_TYPE_POLICER << 48 | 0ULL << 40 | 0xffffffffff;
        //ppPortQosInfo->dot1pToTcObjId = (uint64_t)SAI_OBJECT_TYPE_QOS_MAP << 48 | 0ULL << 40 | 0xffffffffff;
        //ppPortQosInfo->dot1pToDpObjId = (uint64_t)SAI_OBJECT_TYPE_QOS_MAP << 48 | 0ULL << 40 | 0xffffffffff;
        //ppPortQosInfo->dscpToTcObjId = (uint64_t)SAI_OBJECT_TYPE_QOS_MAP << 48 | 0ULL << 40 | 0xffffffffff;
        //ppPortQosInfo->dscpToDpObjId = (uint64_t)SAI_OBJECT_TYPE_QOS_MAP << 48 | 0ULL << 40 | 0xffffffffff;
        //ppPortQosInfo->tcToQueueObjId = (uint64_t)SAI_OBJECT_TYPE_QOS_MAP << 48 | 0ULL << 40 | 0xffffffffff;
        //ppPortQosInfo->tcDpToDot1pObjId = (uint64_t)SAI_OBJECT_TYPE_QOS_MAP << 48 | 0ULL << 40 | 0xffffffffff;
        //ppPortQosInfo->tcDpToDscpObjId = (uint64_t)SAI_OBJECT_TYPE_QOS_MAP << 48 | 0ULL << 40 | 0xffffffffff;
        //ppPortQosInfo->tcToPgObjId = (uint64_t)SAI_OBJECT_TYPE_QOS_MAP << 48 | 0ULL << 40 | 0xffffffffff;
        //ppPortQosInfo->pfcPrioToPgObjId = (uint64_t)SAI_OBJECT_TYPE_QOS_MAP << 48 | 0ULL << 40 | 0xffffffffff;;
        //ppPortQosInfo->pfcPriToQueueObjId = (uint64_t)SAI_OBJECT_TYPE_QOS_MAP << 48 | 0ULL << 40 | 0xffffffffff;;
        //ppPortQosInfo->ingBufProfileObjId = (uint64_t)SAI_OBJECT_TYPE_BUFFER_PROFILE << 48  | 0ULL << 40 | 0xffffffffff;

        /* Insert the state into the database */
        if ((xpsRetVal = xpsStateInsertData(XP_SCOPE_DEFAULT, portQosDbHandle,
                                            (void*)ppPortQosInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to insert sai port qos data |retVal: %d\n", xpsRetVal);
            xpsStateHeapFree((void*)ppPortQosInfo);
            return xpsRetVal;
        }
    }
    return XP_NO_ERR;
}

//Func: xpSaiRemovePortQosDbEntry

XP_STATUS xpSaiRemovePortQosDbEntry(sai_object_id_t port_id)
{
    xpSaiPortQosInfo_t  portQosInfoKey;
    xpSaiPortQosInfo_t  *ppPortQosInfo = NULL;
    XP_STATUS           xpsRetVal = XP_NO_ERR;

    memset(&portQosInfoKey, 0, sizeof(xpSaiPortQosInfo_t));

    portQosInfoKey.portId = port_id;
    /* Retrieve the corresponding state from sai port qos db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, portQosDbHandle,
                                        (xpsDbKey_t)&portQosInfoKey, (void**)&ppPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai port qos data |retVal:%d\n",
                       xpsRetVal);
        return xpsRetVal;
    }
    if (ppPortQosInfo == NULL)
    {
        XP_SAI_LOG_ERR("Entry doesn't exist for this sai port qos data |retVal:%d\n",
                       xpsRetVal);
        return xpsRetVal;
    }
    else
    {
        xpsRetVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, portQosDbHandle,
                                       (xpsDbKey_t)&portQosInfoKey, (void**)&ppPortQosInfo);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not delete entry from Port Qos DB error %d\n", xpsRetVal);
            return xpsRetVal;
        }
        xpsStateHeapFree((void*)ppPortQosInfo);
    }
    return XP_NO_ERR;
}

//Func: xpSaiSetPortAttrPolicerId

sai_status_t xpSaiSetPortAttrPolicerId(sai_object_id_t port_id,
                                       sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    sai_object_id_t     policerObjId = value.oid;
    sai_status_t        saiRetVal    = SAI_STATUS_SUCCESS;
    XP_STATUS           xpsRetVal    = XP_NO_ERR;
    sai_uint32_t        portId       = xpSaiObjIdValueGet(port_id);
    xpsDevice_t         xpDevId      = xpSaiObjIdSwitchGet(port_id);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((policerObjId != SAI_NULL_OBJECT_ID) &&
        (!XDK_SAI_OBJID_TYPE_CHECK(policerObjId, SAI_OBJECT_TYPE_POLICER)))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(policerObjId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get the sai port qos information */
    if ((xpsRetVal = xpSaiGetPortQosInfo(port_id, &pPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get SAI Port QOS Info |Port:%d retVal:%d\n",
                       portId, xpsRetVal);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (pPortQosInfo->policerId == policerObjId)
    {
        /* Return if policer id is same */
        return SAI_STATUS_SUCCESS;
    }

    sai_uint32_t acmIndex = 0;
    xpsRetVal = xpsPolicerPortPolicerIndexGet(xpDevId, portId,
                                              XP_INGRESS_PORT_POLICER, XPS_ALL_STORM_CONTROL, &acmIndex);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get ingress port policer index: %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (pPortQosInfo->policerId != SAI_NULL_OBJECT_ID)
    {
        /* Delete Sequence*/
        /* Remove port from sai policer db index list */
        if ((xpsRetVal = xpSaiPolicerRemoveIdxFromIdxList(pPortQosInfo->policerId,
                                                          XP_INGRESS_PORT_POLICER,
                                                          acmIndex)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed xpSaiPolicerRemoveIdxFromIdxList |Error: %d\n",
                           xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }

    if (policerObjId != SAI_NULL_OBJECT_ID)
    {
        /* Update the sai policer db index list with xpsDevPort */
        if ((xpsRetVal = xpSaiPolicerAddIdxToIdxList(policerObjId,
                                                     XP_INGRESS_PORT_POLICER, acmIndex)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to add port id to sai policer db index list |Error: %d\n",
                           xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }

    if ((saiRetVal = xpSaiPortApplyPolicer(port_id, policerObjId,
                                           XPS_ALL_STORM_CONTROL)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed xpSaiPortApplyPolicer |Error: %d\n", saiRetVal);
        return saiRetVal;
    }

    pPortQosInfo->policerId = policerObjId;

    /* Enable/Disable Ingress port policer */
    saiRetVal = xpSaiSetPortIngressPolicerEnable(port_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to enable policer on port, saiStatus: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrPolicerId

sai_status_t xpSaiGetPortAttrPolicerId(sai_object_id_t port_id,
                                       sai_attribute_value_t* value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS xpsRetVal = XP_NO_ERR;

    /* Validate SAI Port Objest Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get the port qos information */
    if ((xpsRetVal = xpSaiGetPortQosInfo(port_id, &pPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get SAI Port QOS Info |Port:%d retVal:%d\n",
                       (int)xpSaiObjIdValueGet(port_id), xpsRetVal);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    value->oid = pPortQosInfo->policerId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrQosDefaultTc

sai_status_t xpSaiSetPortAttrQosDefaultTc(sai_object_id_t port_id,
                                          sai_attribute_value_t value)
{
    xpsInterfaceId_t xpsIntfId = 0;
    xpsDevice_t xpsDevId;
    xpsPort_t xpsDevPort;
    XP_STATUS xpsRetVal;
    sai_uint32_t tc = 0;
    GT_STATUS rc = GT_OK;

    /* Get the port id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntfId = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);
    tc = value.u8;

    /* Fetch device and port information for this particular port interface */
    if ((xpsRetVal = xpsPortGetDevAndPortNumFromIntf(xpsIntfId, &xpsDevId,
                                                     &xpsDevPort)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Fetch device and port information failed, interface id(%d)",
                       xpsIntfId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Set the default TC for the port */
    if ((rc = cpssHalQoSPortDefaultTcSet(xpsDevId, xpsDevPort, tc)) != GT_OK)
    {
        XP_SAI_LOG_ERR("Failed to Set the default TC %d for a port %d\n", tc,
                       xpsIntfId);
        return  cpssStatus2SaiStatus(rc);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosDefaultTc

sai_status_t xpSaiGetPortAttrQosDefaultTc(sai_object_id_t port_id,
                                          sai_attribute_value_t* value)
{
    xpsInterfaceId_t xpsIntfId = 0;
    xpsPort_t xpsDevPort;
    xpsDevice_t xpsDevId;
    XP_STATUS xpsRetVal;
    sai_uint32_t tc = 0;
    GT_STATUS rc = GT_OK;

    /* Get the port id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntfId = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    /* Fetch device and port information for this particular port interface */
    if ((xpsRetVal = xpsPortGetDevAndPortNumFromIntf(xpsIntfId, &xpsDevId,
                                                     &xpsDevPort)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Fetch device and port information failed, interface id(%d)",
                       xpsIntfId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Get the default TC from the port */
    if ((rc = cpssHalQoSPortDefaultTcGet(xpsDevId, xpsDevPort, &tc)) != GT_OK)
    {
        XP_SAI_LOG_ERR("Failed to Set the default TC %d for a port %d\n", tc,
                       xpsIntfId);
        return  cpssStatus2SaiStatus(rc);
    }

    value->u8 = (uint8_t) tc;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrQosDot1pToTcMap

sai_status_t xpSaiSetPortAttrQosDot1pToTcMap(sai_object_id_t portObjId,
                                             sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo   = NULL;
    sai_object_id_t     dot1pToTcObjId = value.oid;
    sai_uint32_t        profileId      = 0;
    sai_status_t        saiStatus      = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus       = XP_NO_ERR;
    GT_STATUS           rc             = GT_OK;

    /* Validate port object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Validate qos map object */
    saiStatus = xpSaiQosMapValidateQosMapType(dot1pToTcObjId,
                                              SAI_QOS_MAP_TYPE_DOT1P_TO_TC);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    /* Get port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get port qos state data, xpStatus: %d\n",
                       xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (pPortQosInfo->dot1pToTcObjId == dot1pToTcObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    xpsPort_t portNum = (xpsPort_t)xpSaiObjIdValueGet(portObjId);
    xpsDevice_t  xpDevId = xpSaiObjIdSwitchGet(portObjId);

    /* Get a new qos port ingress profile */
    saiStatus = xpSaiQosMapGetPortIngressProfileId(portObjId, dot1pToTcObjId,
                                                   pPortQosInfo->dot1pToDpObjId,
                                                   pPortQosInfo->dscpToTcObjId, pPortQosInfo->dscpToDpObjId, &profileId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Configure ingress qos profile on port */
    rc = cpssHalQoSPortIngressQosMappingTableIdxSet(xpDevId, portNum, profileId);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set qos port ingress L2 profile, profile: %d, port: %d\n",
                       profileId, portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    /* Configure port qos trust mode */
    GT_BOOL l2Trust = ((dot1pToTcObjId != SAI_NULL_OBJECT_ID) ||
                       (pPortQosInfo->dot1pToDpObjId != SAI_NULL_OBJECT_ID)) ? GT_TRUE : GT_FALSE;
    GT_BOOL l3Trust = ((pPortQosInfo->dscpToTcObjId != SAI_NULL_OBJECT_ID) ||
                       (pPortQosInfo->dscpToDpObjId != SAI_NULL_OBJECT_ID)) ? GT_TRUE: GT_FALSE;
    rc = cpssHalPortQosTrustModeSet(xpDevId, portNum, l2Trust, l3Trust);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set port qos trust mode port: %d\n", portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    /* Clear existing qos port ingress profile */
    saiStatus = xpSaiQosMapClearPortIngressProfileId(portObjId,
                                                     pPortQosInfo->dot1pToTcObjId, pPortQosInfo->dot1pToDpObjId,
                                                     pPortQosInfo->dscpToTcObjId, pPortQosInfo->dscpToDpObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    if (pPortQosInfo->dot1pToTcObjId != SAI_NULL_OBJECT_ID)
    {
        /* Remove port from qos map state port list */
        saiStatus = xpSaiQosMapRemovePortFromPortList(pPortQosInfo->dot1pToTcObjId,
                                                      portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to remove port from qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    if (dot1pToTcObjId != SAI_NULL_OBJECT_ID)
    {
        /* Add port to qos map state port list */
        saiStatus = xpSaiQosMapAddPortToPortList(dot1pToTcObjId, portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to add port to qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Update port qos state data */
    pPortQosInfo->dot1pToTcObjId = dot1pToTcObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosDot1pToTcMap

sai_status_t xpSaiGetPortAttrQosDot1pToTcMap(sai_object_id_t portObjId,
                                             sai_attribute_value_t *value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Validate SAI Port Objest Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port:%d, xpStatus: %d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    value->oid = pPortQosInfo->dot1pToTcObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrQosDot1pToColorMap

sai_status_t xpSaiSetPortAttrQosDot1pToColorMap(sai_object_id_t portObjId,
                                                sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo   = NULL;
    sai_object_id_t     dot1pToDpObjId = value.oid;
    sai_uint32_t        profileId      = 0;
    sai_status_t        saiStatus      = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus       = XP_NO_ERR;
    GT_STATUS           rc             = GT_OK;

    /* Validate port object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Validate qos map object */
    saiStatus = xpSaiQosMapValidateQosMapType(dot1pToDpObjId,
                                              SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    /* Get port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get port qos state data, xpStatus: %d\n",
                       xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (pPortQosInfo->dot1pToDpObjId == dot1pToDpObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    xpsPort_t portNum = (xpsPort_t)xpSaiObjIdValueGet(portObjId);
    xpsDevice_t  xpDevId = xpSaiObjIdSwitchGet(portObjId);

    /* Get a new qos port ingress profile */
    saiStatus = xpSaiQosMapGetPortIngressProfileId(portObjId,
                                                   pPortQosInfo->dot1pToTcObjId, dot1pToDpObjId,
                                                   pPortQosInfo->dscpToTcObjId, pPortQosInfo->dscpToDpObjId, &profileId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Configure ingress qos profile on port */
    rc = cpssHalQoSPortIngressQosMappingTableIdxSet(xpDevId, portNum, profileId);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set qos port ingress L2 profile, profile: %d, port: %d\n",
                       profileId, portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    /* Configure port qos trust mode */
    GT_BOOL l2Trust = ((pPortQosInfo->dot1pToTcObjId != SAI_NULL_OBJECT_ID) ||
                       (dot1pToDpObjId != SAI_NULL_OBJECT_ID)) ? GT_TRUE: GT_FALSE;
    GT_BOOL l3Trust = ((pPortQosInfo->dscpToTcObjId != SAI_NULL_OBJECT_ID) ||
                       (pPortQosInfo->dscpToDpObjId != SAI_NULL_OBJECT_ID)) ? GT_TRUE: GT_FALSE;
    rc = cpssHalPortQosTrustModeSet(xpDevId, portNum, l2Trust, l3Trust);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set port qos trust mode port: %d\n", portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    /* Clear existing qos port ingress profile */
    saiStatus = xpSaiQosMapClearPortIngressProfileId(portObjId,
                                                     pPortQosInfo->dot1pToTcObjId, pPortQosInfo->dot1pToDpObjId,
                                                     pPortQosInfo->dscpToTcObjId, pPortQosInfo->dscpToDpObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    if (pPortQosInfo->dot1pToDpObjId != SAI_NULL_OBJECT_ID)
    {
        /* Remove port from qos map state port list */
        saiStatus = xpSaiQosMapRemovePortFromPortList(pPortQosInfo->dot1pToDpObjId,
                                                      portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to remove port from qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    if (dot1pToDpObjId != SAI_NULL_OBJECT_ID)
    {
        /* Add port to qos map state port list */
        saiStatus = xpSaiQosMapAddPortToPortList(dot1pToDpObjId, portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to add port to qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Update port qos state data */
    pPortQosInfo->dot1pToDpObjId = dot1pToDpObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosDot1pToColorMap

sai_status_t xpSaiGetPortAttrQosDot1pToColorMap(sai_object_id_t portObjId,
                                                sai_attribute_value_t *value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Validate SAI Port Objest Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port:%d, xpStatus: %d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    value->oid = pPortQosInfo->dot1pToDpObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrQosDscpToTcMap

sai_status_t xpSaiSetPortAttrQosDscpToTcMap(sai_object_id_t portObjId,
                                            sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo  = NULL;
    sai_object_id_t     dscpToTcObjId = value.oid;
    sai_uint32_t        profileId     = 0;
    sai_status_t        saiStatus     = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus      = XP_NO_ERR;
    GT_STATUS           rc            = GT_OK;
    GT_U32       curTableIdx = 0;
    GT_BOOL       usUpAsKey = GT_FALSE;

    /* Validate port object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Validate qos map object */
    saiStatus = xpSaiQosMapValidateQosMapType(dscpToTcObjId,
                                              SAI_QOS_MAP_TYPE_DSCP_TO_TC);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    xpsPort_t portNum = (xpsPort_t)xpSaiObjIdValueGet(portObjId);
    xpsDevice_t  xpDevId = xpSaiObjIdSwitchGet(portObjId);

    /* Get port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get port qos state data, port: %d, xpStatus: %d\n",
                       portNum, xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (pPortQosInfo->dscpToTcObjId == dscpToTcObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    rc = cpssHalQosPortTrustQosMappingTableIndexGet(xpDevId, portNum, &usUpAsKey,
                                                    &curTableIdx);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: cpssHalQosPortTrustQosMappingTableIndexGet port: %d\n",
                       portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    if (dscpToTcObjId == SAI_NULL_OBJECT_ID)
    {
        saiStatus = xpSaiQosMapSetTrafficClassForL3QosProfile(xpDevId, curTableIdx,
                                                              pPortQosInfo->dscpToTcObjId, true);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: xpSaiQosMapSetTrafficClassForL3QosProfile, port: %d, xpStatus: %d\n",
                           portNum, xpStatus);
            return saiStatus;
        }
    }

    /* Get a new qos port ingress profile */
    saiStatus = xpSaiQosMapGetPortIngressProfileId(portObjId,
                                                   pPortQosInfo->dot1pToTcObjId, pPortQosInfo->dot1pToDpObjId,
                                                   dscpToTcObjId, pPortQosInfo->dscpToDpObjId, &profileId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Configure ingress qos profile on port */
    rc = cpssHalQoSPortIngressQosMappingTableIdxSet(xpDevId, portNum, profileId);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set qos port ingress L3 profile, profile: %d, port: %d\n",
                       profileId, portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    /* Configure port qos trust mode */
    GT_BOOL l2Trust = ((pPortQosInfo->dot1pToTcObjId != SAI_NULL_OBJECT_ID) ||
                       (pPortQosInfo->dot1pToDpObjId != SAI_NULL_OBJECT_ID)) ? GT_TRUE : GT_FALSE;
    GT_BOOL l3Trust = ((dscpToTcObjId != SAI_NULL_OBJECT_ID) ||
                       (pPortQosInfo->dscpToDpObjId != SAI_NULL_OBJECT_ID)) ? GT_TRUE : GT_FALSE;

    /* Default trust mode set to L3 */
    if ((l2Trust == GT_FALSE) && (l3Trust == GT_FALSE))
    {
        l3Trust = GT_TRUE;
    }

    rc = cpssHalPortQosTrustModeSet(xpDevId, portNum, l2Trust, l3Trust);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set port qos trust mode port: %d\n", portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    /* Clear existing qos port ingress profile */
    saiStatus = xpSaiQosMapClearPortIngressProfileId(portObjId,
                                                     pPortQosInfo->dot1pToTcObjId, pPortQosInfo->dot1pToDpObjId,
                                                     pPortQosInfo->dscpToTcObjId, pPortQosInfo->dscpToDpObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    if (pPortQosInfo->dscpToTcObjId != SAI_NULL_OBJECT_ID)
    {
        /* Remove port from qos map state port list */
        saiStatus = xpSaiQosMapRemovePortFromPortList(pPortQosInfo->dscpToTcObjId,
                                                      portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to remove port from qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    if (dscpToTcObjId != SAI_NULL_OBJECT_ID)
    {
        /* Add port to qos map state port list */
        saiStatus = xpSaiQosMapAddPortToPortList(dscpToTcObjId, portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to add port to qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Update port qos state data */
    pPortQosInfo->dscpToTcObjId = dscpToTcObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosDscpToTcMap

sai_status_t xpSaiGetPortAttrQosDscpToTcMap(sai_object_id_t portObjId,
                                            sai_attribute_value_t *value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Validate SAI Port Objest Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port:%d, xpStatus: %d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    value->oid = pPortQosInfo->dscpToTcObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrQosDscpToColorMap

sai_status_t xpSaiSetPortAttrQosDscpToColorMap(sai_object_id_t portObjId,
                                               sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo  = NULL;
    sai_object_id_t     dscpToDpObjId = value.oid;
    sai_uint32_t        profileId     = 0;
    sai_status_t        saiStatus     = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus      = XP_NO_ERR;
    GT_STATUS           rc            = GT_OK;
    GT_U32       curTableIdx = 0;
    GT_BOOL       usUpAsKey = GT_FALSE;


    /* Validate port object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Validate qos map object */
    saiStatus = xpSaiQosMapValidateQosMapType(dscpToDpObjId,
                                              SAI_QOS_MAP_TYPE_DSCP_TO_COLOR);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    xpsPort_t portNum = (xpsPort_t)xpSaiObjIdValueGet(portObjId);
    xpsDevice_t  xpDevId = xpSaiObjIdSwitchGet(portObjId);

    /* Get port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get port qos state data, port: %d, xpStatus: %d\n",
                       portNum, xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (pPortQosInfo->dscpToDpObjId == dscpToDpObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    rc = cpssHalQosPortTrustQosMappingTableIndexGet(xpDevId, portNum, &usUpAsKey,
                                                    &curTableIdx);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: cpssHalQosPortTrustQosMappingTableIndexGet port: %d\n",
                       portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    if (dscpToDpObjId == SAI_NULL_OBJECT_ID)
    {
        saiStatus = xpSaiQosMapSetColorForL3QosProfile(xpDevId, curTableIdx,
                                                       pPortQosInfo->dscpToDpObjId, true);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: xpSaiQosMapSetColorForL3QosProfile, port: %d, xpStatus: %d\n",
                           portNum, xpStatus);
            return saiStatus;
        }
    }


    /* Get qos port ingress profile */
    saiStatus = xpSaiQosMapGetPortIngressProfileId(portObjId,
                                                   pPortQosInfo->dot1pToTcObjId, pPortQosInfo->dot1pToDpObjId,
                                                   pPortQosInfo->dscpToTcObjId, dscpToDpObjId, &profileId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Configure ingress qos profile on port */
    rc = cpssHalQoSPortIngressQosMappingTableIdxSet(xpDevId, portNum, profileId);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set qos port ingress L3 profile, profile: %d, port: %d\n",
                       profileId, portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    /* Configure port qos trust mode */
    GT_BOOL l2Trust = ((pPortQosInfo->dot1pToTcObjId != SAI_NULL_OBJECT_ID) ||
                       (pPortQosInfo->dot1pToDpObjId != SAI_NULL_OBJECT_ID)) ? GT_TRUE : GT_FALSE;
    GT_BOOL l3Trust = ((pPortQosInfo->dscpToTcObjId != SAI_NULL_OBJECT_ID) ||
                       (dscpToDpObjId != SAI_NULL_OBJECT_ID)) ? GT_TRUE : GT_FALSE;

    /* Default trust mode set to L3 */
    if ((l2Trust == GT_FALSE) && (l3Trust == GT_FALSE))
    {
        l3Trust = GT_TRUE;
    }

    rc = cpssHalPortQosTrustModeSet(xpDevId, portNum, l2Trust, l3Trust);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set port qos trust mode port: %d\n", portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    /* Clear existing qos port ingress profile */
    saiStatus = xpSaiQosMapClearPortIngressProfileId(portObjId,
                                                     pPortQosInfo->dot1pToTcObjId, pPortQosInfo->dot1pToDpObjId,
                                                     pPortQosInfo->dscpToTcObjId, pPortQosInfo->dscpToDpObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    if (pPortQosInfo->dscpToDpObjId != SAI_NULL_OBJECT_ID)
    {
        /* Remove port from qos map state port list */
        saiStatus = xpSaiQosMapRemovePortFromPortList(pPortQosInfo->dscpToDpObjId,
                                                      portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to remove port from qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    if (dscpToDpObjId != SAI_NULL_OBJECT_ID)
    {
        /* Add port to qos map state port list */
        saiStatus = xpSaiQosMapAddPortToPortList(dscpToDpObjId, portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to add port to qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Update port qos state data */
    pPortQosInfo->dscpToDpObjId = dscpToDpObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosDscpToColorMap

sai_status_t xpSaiGetPortAttrQosDscpToColorMap(sai_object_id_t portObjId,
                                               sai_attribute_value_t *value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Validate SAI Port Objest Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port:%d, xpStatus: %d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    value->oid = pPortQosInfo->dscpToDpObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrQosTcToQueueMap

sai_status_t xpSaiSetPortAttrQosTcToQueueMap(sai_object_id_t portObjId,
                                             sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo   = NULL;
    sai_object_id_t     tcToQueueObjId = value.oid;
    sai_status_t        saiStatus      = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus       = XP_NO_ERR;

    /* Validate port object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Validate qos map object */
    saiStatus = xpSaiQosMapValidateQosMapType(tcToQueueObjId,
                                              SAI_QOS_MAP_TYPE_TC_TO_QUEUE);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    xpsPort_t portNum = (xpsPort_t)xpSaiObjIdValueGet(portObjId);

    /* Get the port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get port qos state data, port: %d, xpStatus: %d\n",
                       portNum, xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (pPortQosInfo->tcToQueueObjId == tcToQueueObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (pPortQosInfo->tcToQueueObjId != SAI_NULL_OBJECT_ID)
    {
        /* Remove port from qos map state port list */
        saiStatus = xpSaiQosMapRemovePortFromPortList(pPortQosInfo->tcToQueueObjId,
                                                      portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to remove port from qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    if (tcToQueueObjId != SAI_NULL_OBJECT_ID)
    {
        /* Add port to qos map state port list */
        saiStatus = xpSaiQosMapAddPortToPortList(tcToQueueObjId, portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to add port to qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Update state data */
    pPortQosInfo->tcToQueueObjId = tcToQueueObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosTcToQueueMap

sai_status_t xpSaiGetPortAttrQosTcToQueueMap(sai_object_id_t portObjId,
                                             sai_attribute_value_t *value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Validate SAI Port Objest Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port:%d, xpStatus: %d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    value->oid = pPortQosInfo->tcToQueueObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrQosTcAndColorToDot1pMap

sai_status_t xpSaiSetPortAttrQosTcAndColorToDot1pMap(sai_object_id_t portObjId,
                                                     sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo     = NULL;
    sai_object_id_t     tcDpToDot1pObjId = value.oid;
    sai_uint32_t        profileId        = 0;
    sai_status_t        saiStatus        = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus         = XP_NO_ERR;
    GT_STATUS           rc               = GT_OK;

    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Validate qos map object */
    saiStatus = xpSaiQosMapValidateQosMapType(tcDpToDot1pObjId,
                                              SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    xpsPort_t portNum = (xpsPort_t)xpSaiObjIdValueGet(portObjId);
    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(portObjId);

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port: %d, xpStatus: %d\n", portNum,
                       xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (pPortQosInfo->tcDpToDot1pObjId == tcDpToDot1pObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    /* Get qos port egress profile */
    saiStatus = xpSaiQosMapGetPortEgressProfileId(portObjId, tcDpToDot1pObjId,
                                                  pPortQosInfo->tcDpToDscpObjId, &profileId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Configure Egress qos profile on port */
    rc = cpssHalQoSPortEgressQosMappingTableIdxSet(xpDevId, portNum, profileId);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set qos port ingress L3 profile, profile: %d, port: %d\n",
                       profileId, portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    /* enable/disable egress tc,dp mapping */
    GT_BOOL dot1pEnable = (tcDpToDot1pObjId != SAI_NULL_OBJECT_ID) ? GT_TRUE :
                          GT_FALSE;
    GT_BOOL dscpEnable = (pPortQosInfo->tcDpToDscpObjId != SAI_NULL_OBJECT_ID) ?
                         GT_TRUE : GT_FALSE;
    GT_BOOL tcdpEnable = (dot1pEnable || dscpEnable) ? GT_TRUE : GT_FALSE;

    rc = cpssHalPortEgressQosTcDpMappingEnableSet(xpDevId, portNum, tcdpEnable);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set egress TcDp mapping enable port: %d\n",
                       portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    rc = cpssHalPortEgressQosDot1pMappingEnableSet(xpDevId, portNum, dot1pEnable);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set egress Dot1p mapping enable port: %d\n",
                       portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    /* Clear existing qos port egress profile */
    saiStatus = xpSaiQosMapClearPortEgressProfileId(portObjId,
                                                    pPortQosInfo->tcDpToDot1pObjId, pPortQosInfo->tcDpToDscpObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    if (pPortQosInfo->tcDpToDot1pObjId != SAI_NULL_OBJECT_ID)
    {
        /* Remove port from qos map state port list */
        saiStatus = xpSaiQosMapRemovePortFromPortList(pPortQosInfo->tcDpToDot1pObjId,
                                                      portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to remove port from qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    if (tcDpToDot1pObjId != SAI_NULL_OBJECT_ID)
    {
        /* Add port to qos map state port list */
        saiStatus = xpSaiQosMapAddPortToPortList(tcDpToDot1pObjId, portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to add port to qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Update state data */
    pPortQosInfo->tcDpToDot1pObjId = tcDpToDot1pObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosTcAndColorToDot1pMap

sai_status_t xpSaiGetPortAttrQosTcAndColorToDot1pMap(sai_object_id_t portObjId,
                                                     sai_attribute_value_t *value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Validate SAI Port Objest Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port:%d, xpStatus: %d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    value->oid = pPortQosInfo->tcDpToDot1pObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrQosTcAndColorToDscpMap

sai_status_t xpSaiSetPortAttrQosTcAndColorToDscpMap(sai_object_id_t portObjId,
                                                    sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo    = NULL;
    sai_object_id_t     tcDpToDscpObjId = value.oid;
    sai_uint32_t        profileId       = 0;
    sai_status_t        saiStatus       = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus        = XP_NO_ERR;
    GT_STATUS           rc              = GT_OK;

    /* Validate port object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Validate qos map object */
    saiStatus = xpSaiQosMapValidateQosMapType(tcDpToDscpObjId,
                                              SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    xpsPort_t portNum = (xpsPort_t)xpSaiObjIdValueGet(portObjId);
    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(portObjId);

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port: %d, xpStatus: %d\n", portNum,
                       xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (pPortQosInfo->tcDpToDscpObjId == tcDpToDscpObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    /* Get qos port egress profile */
    saiStatus = xpSaiQosMapGetPortEgressProfileId(portObjId,
                                                  pPortQosInfo->tcDpToDot1pObjId, tcDpToDscpObjId, &profileId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Configure Egress qos profile on port */
    rc = cpssHalQoSPortEgressQosMappingTableIdxSet(xpDevId, portNum, profileId);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set qos port ingress L3 profile, profile: %d, port: %d\n",
                       profileId, portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    /* enable/disable egress tc,dp mapping */
    GT_BOOL dot1pEnable = (pPortQosInfo->tcDpToDot1pObjId != SAI_NULL_OBJECT_ID) ?
                          GT_TRUE : GT_FALSE;
    GT_BOOL dscpEnable = (tcDpToDscpObjId != SAI_NULL_OBJECT_ID) ? GT_TRUE :
                         GT_FALSE;
    GT_BOOL tcdpEnable = (dot1pEnable || dscpEnable) ? GT_TRUE : GT_FALSE;

    rc = cpssHalPortEgressQosTcDpMappingEnableSet(xpDevId, portNum, tcdpEnable);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set egress TcDp mapping enable port: %d\n",
                       portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    rc = cpssHalPortEgressQosDscpMappingEnableSet(xpDevId, portNum, dscpEnable);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set egress Dscp mapping enable port: %d\n",
                       portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    /* Clear existing qos port egress profile */
    saiStatus = xpSaiQosMapClearPortEgressProfileId(portObjId,
                                                    pPortQosInfo->tcDpToDot1pObjId, pPortQosInfo->tcDpToDscpObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    if (pPortQosInfo->tcDpToDscpObjId != SAI_NULL_OBJECT_ID)
    {
        /* Remove port from qos map state port list */
        saiStatus = xpSaiQosMapRemovePortFromPortList(pPortQosInfo->tcDpToDscpObjId,
                                                      portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to remove port from qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    if (tcDpToDscpObjId != SAI_NULL_OBJECT_ID)
    {
        /* Add port to qos map state port list */
        saiStatus = xpSaiQosMapAddPortToPortList(tcDpToDscpObjId, portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to add port to qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    pPortQosInfo->tcDpToDscpObjId = tcDpToDscpObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosTcAndColorToDscpMap

sai_status_t xpSaiGetPortAttrQosTcAndColorToDscpMap(sai_object_id_t portObjId,
                                                    sai_attribute_value_t *value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Validate SAI Port Objest Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port:%d, xpStatus: %d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    value->oid = pPortQosInfo->tcDpToDscpObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrQosTcToPriorityGroupMap

sai_status_t xpSaiSetPortAttrQosTcToPriorityGroupMap(sai_object_id_t portObjId,
                                                     sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    sai_object_id_t     tcToPgObjId  = value.oid;
    sai_status_t        saiStatus      = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Validate port object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Validate qos map object */
    saiStatus = xpSaiQosMapValidateQosMapType(tcToPgObjId,
                                              SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    xpsPort_t portNum = (xpsPort_t)xpSaiObjIdValueGet(portObjId);

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port: %d, xpStatus: %d\n", portNum,
                       xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (pPortQosInfo->tcToPgObjId == tcToPgObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (pPortQosInfo->tcToPgObjId != SAI_NULL_OBJECT_ID)
    {
        /* Remove port from qos map state port list */
        saiStatus = xpSaiQosMapRemovePortFromPortList(pPortQosInfo->tcToPgObjId,
                                                      portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to remove port from qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    if (tcToPgObjId != SAI_NULL_OBJECT_ID)
    {
        /* Add port to qos map state port list */
        saiStatus = xpSaiQosMapAddPortToPortList(tcToPgObjId, portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to add port to qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Update state data */
    pPortQosInfo->tcToPgObjId = tcToPgObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosTcToPriorityGroupMap

sai_status_t xpSaiGetPortAttrQosTcToPriorityGroupMap(sai_object_id_t portObjId,
                                                     sai_attribute_value_t* value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Validate SAI Port Objest Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port:%d, xpStatus: %d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    value->oid = pPortQosInfo->tcToPgObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrQosPfcPriorityToPriorityGroupMap

sai_status_t xpSaiSetPortAttrQosPfcPriorityToPriorityGroupMap(
    sai_object_id_t portObjId, sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo     = NULL;
    sai_object_id_t     pfcPrioToPgObjId = value.oid;
    sai_status_t        saiStatus          = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus         = XP_NO_ERR;

    /* Validate port object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Validate qos map object */
    saiStatus = xpSaiQosMapValidateQosMapType(pfcPrioToPgObjId,
                                              SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    xpsPort_t portNum = (xpsPort_t)xpSaiObjIdValueGet(portObjId);

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port: %d, xpStatus: %d\n", portNum,
                       xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (pPortQosInfo->pfcPrioToPgObjId == pfcPrioToPgObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (pPortQosInfo->pfcPrioToPgObjId != SAI_NULL_OBJECT_ID)
    {
        /* Remove port from qos map state port list */
        saiStatus = xpSaiQosMapRemovePortFromPortList(pPortQosInfo->pfcPrioToPgObjId,
                                                      portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to remove port from qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    if (pfcPrioToPgObjId != SAI_NULL_OBJECT_ID)
    {
        /* Add port to qos map state port list */
        saiStatus = xpSaiQosMapAddPortToPortList(pfcPrioToPgObjId, portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to add port to qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Update state database */
    pPortQosInfo->pfcPrioToPgObjId = pfcPrioToPgObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosPfcPriorityToPriorityGroupMap

sai_status_t xpSaiGetPortAttrQosPfcPriorityToPriorityGroupMap(
    sai_object_id_t portObjId, sai_attribute_value_t* value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Validate SAI Port Objest Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port:%d, xpStatus: %d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    value->oid = pPortQosInfo->pfcPrioToPgObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrQosPfcPriorityToQueueMap

sai_status_t xpSaiSetPortAttrQosPfcPriorityToQueueMap(sai_object_id_t portObjId,
                                                      sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo       = NULL;
    sai_object_id_t     pfcPriToQueueObjId = value.oid;
    sai_status_t        saiStatus            = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus           = XP_NO_ERR;

    /* Validate port object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Validate qos map object */
    saiStatus = xpSaiQosMapValidateQosMapType(pfcPriToQueueObjId,
                                              SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    xpsPort_t portNum = (xpsPort_t)xpSaiObjIdValueGet(portObjId);

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port: %d, xpStatus: %d\n", portNum,
                       xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (pPortQosInfo->pfcPriToQueueObjId == pfcPriToQueueObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (pPortQosInfo->pfcPriToQueueObjId != SAI_NULL_OBJECT_ID)
    {
        /* Remove port from qos map state port list */
        saiStatus = xpSaiQosMapRemovePortFromPortList(pPortQosInfo->pfcPriToQueueObjId,
                                                      portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to remove port from qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    if (pfcPriToQueueObjId != SAI_NULL_OBJECT_ID)
    {
        /* Add port to qos map state port list */
        saiStatus = xpSaiQosMapAddPortToPortList(pfcPriToQueueObjId, portNum);
        if (saiStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to add port to qos map state port list, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    pPortQosInfo->pfcPriToQueueObjId = pfcPriToQueueObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosPfcPriorityToQueueMap

sai_status_t xpSaiGetPortAttrQosPfcPriorityToQueueMap(sai_object_id_t portObjId,
                                                      sai_attribute_value_t* value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Validate SAI Port Objest Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(portObjId, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(portObjId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the port qos information */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Entry does not exist, port:%d, xpStatus: %d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    value->oid = pPortQosInfo->pfcPriToQueueObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiPortAddQueueToPortQList

XP_STATUS xpSaiPortAddQueueToPortQList(sai_object_id_t portObjId,
                                       sai_uint8_t queueNum)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpsRetVal = XP_NO_ERR;
    sai_uint8_t         portBitMapIndex = queueNum / SIZEOF_BYTE;

    /* Get the port qos state information */
    if ((xpsRetVal = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get port qos state |Port:%d retVal:%d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpsRetVal);
        return xpsRetVal;
    }

    if (portBitMapIndex >= XPS_MAX_QUEUE_BIT_MAP)
    {
        XP_SAI_LOG_ERR("Error: Invalid queueNum :%d\n", queueNum);
        return XP_ERR_INVALID_INPUT;
    }

    /* Set the queue list bit map */
    pPortQosInfo->qListBitMap[portBitMapIndex] |= (1 << (queueNum % SIZEOF_BYTE));

    /* Increment the port queue count */
    pPortQosInfo->numOfQueues++;

    return XP_NO_ERR;
}

//Func: xpSaiPortRemoveQueueFromPortQList

XP_STATUS xpSaiPortRemoveQueueFromPortQList(sai_object_id_t portObjId,
                                            sai_uint8_t queueNum)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpsRetVal = XP_NO_ERR;
    sai_uint8_t         portBitMapIndex = queueNum / SIZEOF_BYTE;

    /* Get the port qos state information */
    if ((xpsRetVal = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get port qos state |Port:%d retVal:%d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpsRetVal);
        return xpsRetVal;
    }

    if (portBitMapIndex >= XPS_MAX_QUEUE_BIT_MAP)
    {
        XP_SAI_LOG_ERR("Error: Invalid queueNum :%d\n", queueNum);
        return XP_ERR_INVALID_INPUT;
    }

    /* Unset the queue list bit map */
    pPortQosInfo->qListBitMap[portBitMapIndex] &= ~(1 << (queueNum % SIZEOF_BYTE));

    /* Decrement the port queue count */
    pPortQosInfo->numOfQueues--;

    return XP_NO_ERR;
}

//Func: xpSaiGetPortAttrQosNumberOfQueues

sai_status_t xpSaiGetPortAttrQosNumberOfQueues(sai_object_id_t port_id,
                                               sai_attribute_value_t* value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpsRetVal = XP_NO_ERR;

    /* Get the port qos state information */
    if ((xpsRetVal = xpSaiGetPortQosInfo(port_id, &pPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get port qos state |Port:%d retVal:%d\n",
                       (int)xpSaiObjIdValueGet(port_id), xpsRetVal);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    value->u32 = pPortQosInfo->numOfQueues;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosQueueList

sai_status_t xpSaiGetPortAttrQosQueueList(sai_object_id_t port_id,
                                          sai_attribute_value_t* value)
{
    sai_attribute_value_t portType;
    sai_status_t          status = SAI_STATUS_SUCCESS;
    xpSaiPortQosInfo_t   *pPortQosInfo = NULL;
    sai_object_id_t       queueObjId = SAI_NULL_OBJECT_ID;
    xpDevice_t            xpsDevId = xpSaiObjIdSwitchGet(port_id);
    xpPort_t              xpsPortId = 0;
    XP_STATUS             xpsRetVal = XP_NO_ERR;
    uint32_t              numOfQs = 0;

    /* Get the XPS interface id */
    xpsInterfaceId_t xpsIntfId = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    /* Get the port qos state information */
    if ((xpsRetVal = xpSaiGetPortQosInfo(port_id, &pPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get port qos state, xpsIntfId:%d, retVal:%d\n",
                       xpsIntfId, xpsRetVal);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if ((status = xpSaiGetPortType(xpsDevId, xpsIntfId,
                                   &portType)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get port type from interface, xpsIntfId:%d, retVal:%d",
                       xpsIntfId, status);
        return status;
    }

    if (portType.s32 == SAI_PORT_TYPE_CPU)
    {
        /* No interface id for CPU port, assume interface id as port number */
        xpsPortId = (xpPort_t)xpsIntfId;
    }
    else
    {
        /* Fetch device and port information from interface */
        if ((xpsRetVal = xpsPortGetDevAndPortNumFromIntf(xpsIntfId, &xpsDevId,
                                                         &xpsPortId)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Couldn't get device and port number from interface, xpsIntfId:%d, retVal:%d",
                           xpsIntfId, xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }

    /* Get the total number of queues per port for this device */
    if ((xpsRetVal = xpsQosAqmGetNumberOfQueuesPerPort(xpsDevId, xpsPortId,
                                                       &numOfQs)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get the total number of queues per port %d\n",
                       xpsPortId);
        return  xpsStatus2SaiStatus(xpsRetVal);
    }

    if (value->objlist.count < pPortQosInfo->numOfQueues)
    {
        XP_SAI_LOG_ERR("Unable to fit %u OIDs into the buffer with size %u\n",
                       pPortQosInfo->numOfQueues, value->objlist.count);
        value->objlist.count = pPortQosInfo->numOfQueues;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    value->objlist.count = 0;

    for (uint32_t qNum = 0; qNum < numOfQs; qNum++)
    {
        if ((pPortQosInfo->qListBitMap[qNum / SIZEOF_BYTE]) & (1 <<
                                                               (qNum % SIZEOF_BYTE)))
        {
            /* Create sai queue object id */
            if (xpSaiQosQueueObjIdCreate(SAI_OBJECT_TYPE_QUEUE, xpsPortId, qNum,
                                         &queueObjId) != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to create Queue object id for queue %d\n", qNum);
                return SAI_STATUS_FAILURE;
            }
            value->objlist.list[value->objlist.count] = queueObjId;
            value->objlist.count++;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiPortBindSchedulerGroupToPort

XP_STATUS xpSaiPortBindSchedulerGroupToPort(sai_object_id_t portObjId,
                                            sai_uint32_t schedGrpKey)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus = XP_NO_ERR;

    /* Get the port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get port qos state |Port:%d retVal:%d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpStatus);
        return xpStatus;
    }

    /* Update the port's scheduler group key list */
    pPortQosInfo->schedGrpList[pPortQosInfo->numOfSchedGrps] = schedGrpKey;

    /* Increment the port scheduler group count */
    pPortQosInfo->numOfSchedGrps++;

    return XP_NO_ERR;
}

//Func: xpSaiPortUnbindSchedulerGroupFromPort

XP_STATUS xpSaiPortUnbindSchedulerGroupFromPort(sai_object_id_t portObjId,
                                                sai_uint32_t schedGrpKey)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    sai_uint32_t        nodePresent  = 0;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Get the port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get port qos state |Port:%d retVal:%d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpStatus);
        return xpStatus;
    }

    /* Remove the node from port's scheduler group list */
    for (uint16_t idx = 0; idx < pPortQosInfo->numOfSchedGrps; idx++)
    {
        if (!nodePresent)
        {
            if (pPortQosInfo->schedGrpList[idx] == schedGrpKey)
            {
                nodePresent = 1;
            }
            continue;
        }
        else
        {
            pPortQosInfo->schedGrpList[idx-1] = pPortQosInfo->schedGrpList[idx];
        }
    }

    /* Decrement the port scheduler group count */
    pPortQosInfo->numOfSchedGrps--;

    return XP_NO_ERR;
}

//Func: xpSaiGetPortAttrQosNumberOfSchedulerGroups

sai_status_t xpSaiGetPortAttrQosNumberOfSchedulerGroups(sai_object_id_t port_id,
                                                        sai_attribute_value_t* value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Get the port qos state data */
    if ((xpStatus = xpSaiGetPortQosInfo(port_id, &pPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get port qos state | Port: %d xpStatus: %d\n",
                       (int)xpSaiObjIdValueGet(port_id), xpStatus);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    value->u32 = pPortQosInfo->numOfSchedGrps;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosSchedulerGroupList

sai_status_t xpSaiGetPortAttrQosSchedulerGroupList(sai_object_id_t port_id,
                                                   sai_attribute_value_t* value)
{
    xpSaiPortQosInfo_t   *pPortQosInfo      = NULL;
    sai_object_id_t       schedulerGrpObjId = SAI_NULL_OBJECT_ID;
    sai_status_t          saiStatus         = SAI_STATUS_SUCCESS;
    xpDevice_t            xpDevId           = xpSaiObjIdSwitchGet(port_id);
    XP_STATUS             xpStatus          = XP_NO_ERR;
    uint32_t              key               = 0;

    /* Get the port qos state data */
    if ((xpStatus = xpSaiGetPortQosInfo(port_id, &pPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get port qos state, xpsIntfId:%d, xpStatus:%d\n",
                       (int)xpSaiObjIdValueGet(port_id), xpStatus);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (value->objlist.count < pPortQosInfo->numOfSchedGrps)
    {
        XP_SAI_LOG_ERR("Unable to fit %u OIDs into the buffer with size %u\n",
                       pPortQosInfo->numOfSchedGrps, value->objlist.count);
        value->objlist.count = pPortQosInfo->numOfSchedGrps;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    value->objlist.count = pPortQosInfo->numOfSchedGrps;

    for (uint32_t schedGrpNum = 0; schedGrpNum < pPortQosInfo->numOfSchedGrps;
         schedGrpNum++)
    {
        key = pPortQosInfo->schedGrpList[schedGrpNum];

        /* Create scheduler group object id */
        saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_SCHEDULER_GROUP, xpDevId, key,
                                     &schedulerGrpObjId);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Could not create scheduler group object id, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }

        value->objlist.list[schedGrpNum] = schedulerGrpObjId;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosWredProfileId

sai_status_t xpSaiGetPortAttrQosWredProfileId(sai_object_id_t port_id,
                                              sai_attribute_value_t* value)
{
    XP_SAI_LOG_ERR("Attribute not supported\n");
    return SAI_STATUS_ATTR_NOT_SUPPORTED_0;
}

//Func: xpSaiPortSetSchedulerInfo

sai_status_t xpSaiPortSetSchedulerInfo(sai_object_id_t portObjId,
                                       sai_object_id_t schedObjId)
{
    xpSaiSchedulerAttributesT schedAttrInfo;
    sai_uint64_t burstSize      = 0;
    sai_uint64_t rateBps        = 0;
    sai_status_t retVal         = SAI_STATUS_SUCCESS;
    xpsDevice_t  xpDevId = xpSaiObjIdSwitchGet(portObjId);
    xpsPort_t    xpPort  = (xpsPort_t)xpSaiObjIdValueGet(portObjId);

    /* Only MAX_BW and MAX_burstSize attributes are valid. Ignore other attributes */
    if (schedObjId == SAI_NULL_OBJECT_ID)
    {
        /* Set shaper to default */
        rateBps        = 0;
        burstSize      = 0;
    }
    else
    {
        /* Get the scheduler attributes */
        if (xpSaiGetSchedulerAttrInfo(schedObjId, &schedAttrInfo) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get the scheduler profile info\n");
            return SAI_STATUS_FAILURE;
        }

        rateBps        = schedAttrInfo.maxBandwidthRate.u64;
        burstSize      = schedAttrInfo.maxBandwidthBurstRate.u64;
    }

    /* Configure port shaper */
    if ((retVal = xpSaiSetSchedulerPortShaperRate(xpDevId, xpPort, rateBps,
                                                  burstSize)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not set scheduler port shaper rate | retVal: %d\n",
                       retVal);
        return retVal;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrQosSchedulerProfileId

sai_status_t xpSaiSetPortAttrQosSchedulerProfileId(sai_object_id_t portObjId,
                                                   sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    sai_object_id_t     schedObjId = value.oid;
    sai_status_t        saiRetVal;
    xpsPort_t           xpPort;
    XP_STATUS           xpsRetVal;

    /* Validate scheduler object id */
    if ((schedObjId != SAI_NULL_OBJECT_ID) &&
        (!XDK_SAI_OBJID_TYPE_CHECK(schedObjId, SAI_OBJECT_TYPE_SCHEDULER)))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(schedObjId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpPort = (xpsPort_t)xpSaiObjIdValueGet(portObjId);

    /* Get the port qos information */
    if ((xpsRetVal = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get SAI Port QOS Info |Port:%d retVal:%d\n", xpPort,
                       xpsRetVal);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (pPortQosInfo->schedObjId == schedObjId)
    {
        /* Return if already programmed */
        return SAI_STATUS_SUCCESS;
    }

    /* Configure the scheduler on port */
    if ((saiRetVal = xpSaiPortSetSchedulerInfo(portObjId,
                                               schedObjId)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed xpSaiPortSetSchedulerInfo: error code: %d\n", saiRetVal);
        return saiRetVal;
    }

    if (pPortQosInfo->schedObjId != SAI_NULL_OBJECT_ID)
    {
        /* Unbind the port from existing scheduler */
        if ((xpsRetVal = xpSaiSchedulerUnbindPortFromScheduler(pPortQosInfo->schedObjId,
                                                               xpPort)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to unbind the port from scheduler, retVal: %d\n",
                           xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }

    if (schedObjId != SAI_NULL_OBJECT_ID)
    {
        /* Bind the port to new scheduler */
        if ((xpsRetVal = xpSaiSchedulerBindPortToScheduler(schedObjId,
                                                           xpPort)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind the port to scheduler, retVal: %d\n",
                           xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }

    /* Update port state database */
    pPortQosInfo->schedObjId = schedObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttrQosSchedulerProfileId

sai_status_t xpSaiGetPortAttrQosSchedulerProfileId(sai_object_id_t portObjId,
                                                   sai_attribute_value_t* value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    XP_STATUS           xpsRetVal = XP_NO_ERR;

    /* Get the port qos entry from state database */
    if ((xpsRetVal = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get the port qos entry from state database |Port:%d retVal:%d\n",
                       (int)xpSaiObjIdValueGet(portObjId), xpsRetVal);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    value->oid = pPortQosInfo->schedObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiPortApplyBufferProfileOnPort

sai_status_t xpSaiPortApplyBufferProfileOnPort(sai_object_id_t portObjId,
                                               sai_object_id_t profileObjId, bool ingressList)
{
    xpSaiBufferProfile_t *pProfileInfo         = NULL;
    sai_uint32_t          fullThresholdInPages = 0;
    sai_status_t          saiStatus            = SAI_STATUS_SUCCESS;
    XP_STATUS             xpStatus             = XP_NO_ERR;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT alfa;
    GT_BOOL                    conversionError = GT_FALSE;

    if (profileObjId != SAI_NULL_OBJECT_ID)
    {
        /* Get the buffer profile state data */
        saiStatus = xpSaiBufferProfileGetStateData(profileObjId, &pProfileInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }

        fullThresholdInPages = XPSAI_BYTES_TO_PAGES(pProfileInfo->bufferSize);
    }

    xpsDevice_t  xpDevId = xpSaiObjIdSwitchGet(portObjId);
    sai_uint32_t portNum = xpSaiObjIdValueGet(portObjId);

    if (GT_FALSE == ingressList)
    {
        if (profileObjId != SAI_NULL_OBJECT_ID)
        {
            DYN_TH_2_ALFA(pProfileInfo->sharedDynamicTh, alfa, conversionError);
            if (GT_TRUE==conversionError)
            {
                XP_SAI_LOG_ERR("Failed to configure alfa from sharedDynamicTh: %d\n",
                               pProfileInfo->sharedDynamicTh);
                return SAI_STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            fullThresholdInPages = XPSAI_BYTES_TO_PAGES(
                                       XPSAI_BUFFER_DEFAULT_DYNAMIC_POOL_TH);
            alfa = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        }

        /* Set port   tail drop  threshold*/
        xpStatus = xpSaiProfileMngConfigureGuaranteedLimitAndAlfa(xpDevId, portNum,
                                                                  PROFILE_MNG_PER_PORT_CFG, &fullThresholdInPages, &alfa);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to configure port page tail drop threshold, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }
    /*ingress is  TBD*/

    return SAI_STATUS_SUCCESS;

}

//Func: xpSaiSetPortAttrQosIngressBufferProfileList

sai_status_t xpSaiSetPortAttrQosIngressBufferProfileList(
    sai_object_id_t portObjId, sai_attribute_value_t value)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    sai_object_id_t     profileObjId = SAI_NULL_OBJECT_ID;
    sai_status_t        saiStatus    = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    if (value.objlist.count)
    {
        profileObjId = value.objlist.list[0];
    }

    /* Validate buffer profile object */
    saiStatus = xpSaiBufferProfileValidateProfileObject(profileObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Get the port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get the port qos state data, xpStatus: %d\n",
                       saiStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if ((profileObjId != SAI_NULL_OBJECT_ID) &&
        (pPortQosInfo->ingBufProfileObjId == profileObjId))
    {
        /* Return success if already programmed */
        return SAI_STATUS_SUCCESS;
    }

    /* Each buffer profile maintains a list of its port/PG/queue objects for
     * various purpose. So unbind the existing mapping i.e, port from current
     * buffer profile object list. Also free the guaranteed (profile’s buffer
     * size) from buffer pool and add it to shared memory */
    if (pPortQosInfo->ingBufProfileObjId != SAI_NULL_OBJECT_ID)
    {
        saiStatus = xpSaiBufferRemovePortFromBufferProfile(
                        pPortQosInfo->ingBufProfileObjId, portObjId);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to unbind port from buffer profile id (%llx), xpStatus: %d\n",
                           pPortQosInfo->ingBufProfileObjId, saiStatus);
            return saiStatus;
        }
    }

    /* Each buffer profile maintains a list of its port/PG/queue objects for
     * various purpose. So bind the port object to new buffer profile object
     * list. Also carve the guaranteed memory (profile’s buffer size) from
     * buffer pool */
    if (profileObjId != SAI_NULL_OBJECT_ID)
    {
        /* Bind port to new buffer profile */
        saiStatus = xpSaiBufferAddPortToBufferProfile(profileObjId, portObjId);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind port to buffer profile, xpStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Configure buffer profile on port */
    saiStatus = xpSaiPortApplyBufferProfileOnPort(portObjId, profileObjId, GT_TRUE);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to apply buffer profile on port, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Update state data */
    pPortQosInfo->ingBufProfileObjId = profileObjId;

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiSetPortAttrQosEgressBufferProfileList

sai_status_t xpSaiSetPortAttrQosEgressBufferProfileList(
    sai_object_id_t portObjId, sai_attribute_value_t value, bool forceSet)
{
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    sai_object_id_t     profileObjId = SAI_NULL_OBJECT_ID;
    sai_status_t        saiStatus    = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    if (value.objlist.count)
    {
        profileObjId = value.objlist.list[0];
    }

    /* Validate buffer profile object */
    saiStatus = xpSaiBufferProfileValidateProfileObject(profileObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Get the port qos state data */
    xpStatus = xpSaiGetPortQosInfo(portObjId, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get the port qos state data, xpStatus: %d\n",
                       saiStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if ((profileObjId != SAI_NULL_OBJECT_ID) &&
        (pPortQosInfo->egrBufProfileObjId == profileObjId)&&
        forceSet==false)
    {
        /* Return success if already programmed */
        return SAI_STATUS_SUCCESS;
    }

    /* Each buffer profile maintains a list of its port/PG/queue objects for
     * various purpose. So unbind the existing mapping i.e, port from current
     * buffer profile object list. Also free the guaranteed (profile’s buffer
     * size) from buffer pool and add it to shared memory */
    if (pPortQosInfo->egrBufProfileObjId != SAI_NULL_OBJECT_ID)
    {
        saiStatus = xpSaiBufferRemovePortFromBufferProfile(
                        pPortQosInfo->egrBufProfileObjId, portObjId);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to unbind port from buffer profile, xpStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Each buffer profile maintains a list of its port/PG/queue objects for
     * various purpose. So bind the port object to new buffer profile object
     * list. Also carve the guaranteed memory (profile’s buffer size) from
     * buffer pool */
    if (profileObjId != SAI_NULL_OBJECT_ID)
    {
        /* Bind port to new buffer profile */
        saiStatus = xpSaiBufferAddPortToBufferProfile(profileObjId, portObjId);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind port to buffer profile, xpStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Configure buffer profile on port */
    saiStatus = xpSaiPortApplyBufferProfileOnPort(portObjId, profileObjId,
                                                  GT_FALSE);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to apply buffer profile on port, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Update state data */
    pPortQosInfo->egrBufProfileObjId = profileObjId;

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetPortAttrQosIngressBufferProfileList

sai_status_t xpSaiGetPortAttrQosIngressBufferProfileList(
    sai_object_id_t port_id, sai_attribute_value_t* value)
{
    xpSaiPortQosInfo_t *pQosInfo = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Get the port qos state data */
    xpStatus = xpSaiGetPortQosInfo(port_id, &pQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get the port qos stae data, xpStatus: %d\n",
                       xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    /* Only one buffer profile is allowred to attach to the port */
    if (value->objlist.count < 1)
    {
        XP_SAI_LOG_ERR("Error: Unable to fit %u OIDs into the buffer with size %u\n", 1,
                       value->objlist.count);
        value->objlist.count = 1;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }
    /*Return NULL list if not configured*/
    if (pQosInfo->ingBufProfileObjId == SAI_NULL_OBJECT_ID)
    {
        value->objlist.count = 0;
    }
    else
    {
        value->objlist.count   = 1;
    }
    value->objlist.list[0] = pQosInfo->ingBufProfileObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrPriorityFlowControl

sai_status_t xpSaiSetPortAttrPriorityFlowControl(sai_object_id_t port_id,
                                                 sai_attribute_value_t value,
                                                 pfcVectorUpdateType updateType)
{
    sai_status_t        saiStatus = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t    xpsIntf   = 0;
    sai_uint8_t         currentPfcVector, newPfcVector;
    bool                combinedMode;

    /* Validate SAI Port Objest Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Convert port_Id to xpsIntf */
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(port_id);

    saiStatus =xpSaiPortPriorityFlowControlModeGet(xpsIntf, &combinedMode);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiPortPriorityFlowControlModeGet failed with an error %d\n",
                       saiStatus);
        return saiStatus;
    }
    if ((combinedMode&&(updateType!= pfcVectorUpdateTypeCombined))||
        ((!combinedMode)&&(updateType== pfcVectorUpdateTypeCombined)))
    {
        XP_SAI_LOG_ERR("Wrong state of PFC mode .\n", saiStatus);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /*check current*/
    saiStatus = xpSaiPortPriorityFlowControlGet(xpsIntf, &currentPfcVector,
                                                updateType);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiPriorityFlowControlGet failed with an error %d\n",
                       saiStatus);
        return saiStatus;
    }

    newPfcVector = value.u8;

    XP_SAI_LOG_DBG("xpSaiSetPortAttrPriorityFlowControl :New vector  0x%x ,current vector 0x%x\n",
                   newPfcVector, currentPfcVector);

    if (((currentPfcVector != 0) || (newPfcVector != 0)) &&
        (currentPfcVector == newPfcVector))
    {
        return SAI_STATUS_SUCCESS;
    }

    /* Update xps layer */
    saiStatus = xpSaiPortPriorityFlowControlUpdateHW(xpDevId, xpsIntf, value.u8,
                                                     currentPfcVector^newPfcVector, updateType);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiPortPriorityFlowControlUpdateHW failed with an error %d\n",
                       saiStatus);
        return SAI_STATUS_FAILURE;
    }

    /* Save pfc in db */
    saiStatus = xpSaiPortPriorityFlowControlSet(xpsIntf, value.u8, updateType);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiPortPriorityFlowControlSet failed with an error %d\n",
                       saiStatus);
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiSetPortAttrPriorityFlowControlMode(sai_object_id_t port_id,
                                                     sai_attribute_value_t value)
{
    sai_status_t        saiStatus = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t    xpsIntf   = 0;
    bool                combinedMode=0;
    bool currMode = 0;
    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(port_id);

    /* Validate SAI Port Objest Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Convert port_Id to xpsIntf */
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    if (value.u8==SAI_PORT_PRIORITY_FLOW_CONTROL_MODE_COMBINED)
    {
        combinedMode =1;
    }

    saiStatus = xpSaiPortPriorityFlowControlModeGet(xpsIntf, &currMode);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiPortPriorityFlowControlModeGet failed with an error %d\n",
                       saiStatus);
        return saiStatus;
    }

    if (currMode != combinedMode)
    {
        sai_uint8_t currPfcTxVector;
        sai_uint8_t currPfcRxVector;
        sai_uint8_t currPfcVector;

        saiStatus =xpSaiPortPriorityFlowControlModeSet(xpsIntf, combinedMode);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiPortPriorityFlowControlModeSet failed with an error %d\n",
                           saiStatus);
            return saiStatus;
        }

        /*Get current Tx */
        saiStatus = xpSaiPortPriorityFlowControlGet(xpsIntf, &currPfcTxVector,
                                                    pfcVectorUpdateTypeTx);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiPriorityFlowControlGet failed with an error %d\n",
                           saiStatus);
            return saiStatus;
        }

        /*Get current Rx */
        saiStatus = xpSaiPortPriorityFlowControlGet(xpsIntf, &currPfcRxVector,
                                                    pfcVectorUpdateTypeRx);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiPriorityFlowControlGet failed with an error %d\n",
                           saiStatus);
            return saiStatus;
        }

        /* Get current Combined */
        saiStatus = xpSaiPortPriorityFlowControlGet(xpsIntf, &currPfcVector,
                                                    pfcVectorUpdateTypeCombined);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiPriorityFlowControlGet failed with an error %d\n",
                           saiStatus);
            return saiStatus;
        }

        /* Update xps layer */
        /* COMBINED to SEPARATE */
        if (currMode == 1 && combinedMode == 0)
        {
            saiStatus = xpSaiPortPriorityFlowControlUpdateHW(xpDevId, xpsIntf,
                                                             currPfcTxVector,
                                                             currPfcVector^currPfcTxVector,
                                                             pfcVectorUpdateTypeTx);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiPortPriorityFlowControlUpdateHW failed with an error %d\n",
                               saiStatus);
                return SAI_STATUS_FAILURE;
            }

            saiStatus = xpSaiPortPriorityFlowControlUpdateHW(xpDevId, xpsIntf,
                                                             currPfcRxVector,
                                                             currPfcVector^currPfcRxVector,
                                                             pfcVectorUpdateTypeRx);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiPortPriorityFlowControlUpdateHW failed with an error %d\n",
                               saiStatus);
                return SAI_STATUS_FAILURE;
            }
        }
        /* SEPARATE to COMBINED*/
        else if (currMode == 0 && combinedMode == 1)
        {
            saiStatus = xpSaiPortPriorityFlowControlUpdateHW(xpDevId, xpsIntf,
                                                             currPfcVector,
                                                             currPfcTxVector^currPfcVector,
                                                             pfcVectorUpdateTypeTx);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiPortPriorityFlowControlUpdateHW failed with an error %d\n",
                               saiStatus);
                return SAI_STATUS_FAILURE;
            }

            saiStatus = xpSaiPortPriorityFlowControlUpdateHW(xpDevId, xpsIntf,
                                                             currPfcVector,
                                                             currPfcRxVector^currPfcVector,
                                                             pfcVectorUpdateTypeRx);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiPortPriorityFlowControlUpdateHW failed with an error %d\n",
                               saiStatus);
                return SAI_STATUS_FAILURE;
            }
        }
    }
    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetPortAttrPriorityFlowControl

sai_status_t xpSaiGetPortAttrPriorityFlowControl(sai_object_id_t port_id,
                                                 sai_attribute_value_t* value)
{
    sai_uint8_t         pfcValue    = 0;
    xpsInterfaceId_t    intfId      = 0;
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;

    intfId = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    saiStatus = xpSaiPortPriorityFlowControlGet(intfId, &pfcValue,
                                                pfcVectorUpdateTypeCombined);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiPriorityFlowControlGet failed with an error %d\n",
                       saiStatus);
        return saiStatus;
    }
    value->u8 = pfcValue;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttrEeeEnable

sai_status_t xpSaiSetPortAttrEeeEnable(sai_object_id_t port_id,
                                       sai_attribute_value_t value)
{
    sai_status_t retVal = SAI_STATUS_NOT_SUPPORTED;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrEeeEnable\n");

    return retVal;
}


//Func: xpSaiGetPortAttrEeeEnable

sai_status_t xpSaiGetPortAttrEeeEnable(sai_object_id_t port_id,
                                       sai_attribute_value_t* value)
{
    sai_status_t retVal = SAI_STATUS_NOT_SUPPORTED;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrEeeEnable\n");

    return retVal;
}


//Func: xpSaiSetPortAttrEeeIdleTime

sai_status_t xpSaiSetPortAttrEeeIdleTime(sai_object_id_t port_id,
                                         sai_attribute_value_t value)
{
    sai_status_t retVal = SAI_STATUS_NOT_SUPPORTED;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrEeeIdleTime\n");

    return retVal;
}


//Func: xpSaiGetPortAttrEeeIdleTime

sai_status_t xpSaiGetPortAttrEeeIdleTime(sai_object_id_t port_id,
                                         sai_attribute_value_t* value)
{
    sai_status_t retVal = SAI_STATUS_NOT_SUPPORTED;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrEeeIdleTime\n");

    return retVal;
}


//Func: xpSaiSetPortAttrEeeWakeTime

sai_status_t xpSaiSetPortAttrEeeWakeTime(sai_object_id_t port_id,
                                         sai_attribute_value_t value)
{
    sai_status_t retVal = SAI_STATUS_NOT_SUPPORTED;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrEeeWakeTime\n");

    return retVal;
}


//Func: xpSaiGetPortAttrEeeWakeTime

sai_status_t xpSaiGetPortAttrEeeWakeTime(sai_object_id_t port_id,
                                         sai_attribute_value_t* value)
{
    sai_status_t retVal = SAI_STATUS_NOT_SUPPORTED;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrEeeWakeTime\n");

    return retVal;
}

//Func xpSaiSetPortAttrFecMode

sai_status_t xpSaiSetPortAttrFecMode(sai_object_id_t port_id,
                                     sai_attribute_value_t value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }
    xpSaiPortMgrInfo_t *portInfo = xpSaiPortInfoGet(portNum);
    if (portInfo == NULL)
    {
        XP_SAI_LOG_ERR("Failed to get portInfo from Id (%d).",
                       portNum);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    xpSaiPortInfoInit(&info);
    if (value.s32 == SAI_PORT_FEC_MODE_RS)
    {
        if ((portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_1X &&
             portInfo->speed == XPSAI_PORT_SPEED_50G)||
            (portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_2X &&
             portInfo->speed == XPSAI_PORT_SPEED_100G)||
            (portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_4X &&
             portInfo->speed == XPSAI_PORT_SPEED_200G)||
            (portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_8X &&
             portInfo->speed == XPSAI_PORT_SPEED_400G))
        {
            info.fec = XPSAI_PORT_ADVERT_FEC_RS_544_514;
        }
        else
        {

            info.fec = XPSAI_PORT_ADVERT_FEC_RS;
        }

    }
    else if (value.s32 == SAI_PORT_FEC_MODE_FC)
    {
        info.fec = XPSAI_PORT_ADVERT_FEC_FC;
    }
    /*
       else if (value.s32 == SAI_PORT_FEC_MODE_RS_544_514) //Not yet present in sai header file
       {
       info.fec = XPSAI_PORT_ADVERT_FEC_RS_544_514;
       }
     */
    else
    {
        info.fec = XPSAI_PORT_ADVERT_FEC_NONE;
    }
    if (portInfo->autoneg==
        XPSAI_PORT_AUTONEG_ENABLE) //WA for fec mode set as the advertised speed fec mode is not getting called.
    {
        if (value.s32 == SAI_PORT_FEC_MODE_RS)
        {
            if ((portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_1X &&
                 portInfo->speed == XPSAI_PORT_SPEED_50G)||
                (portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_2X &&
                 portInfo->speed == XPSAI_PORT_SPEED_100G)||
                (portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_4X &&
                 portInfo->speed == XPSAI_PORT_SPEED_200G)||
                (portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_8X &&
                 portInfo->speed == XPSAI_PORT_SPEED_400G))
            {
                info.localAdvertFec |= XPSAI_PORT_ADVERT_FEC_RS_544_514;
            }
            else
            {
                info.localAdvertFec |= XPSAI_PORT_ADVERT_FEC_RS;
            }
        }
        else if (value.s32 == SAI_PORT_FEC_MODE_FC)
        {
            info.localAdvertFec |= XPSAI_PORT_ADVERT_FEC_FC;
        }
        else
        {
            info.localAdvertFec |= XPSAI_PORT_ADVERT_FEC_NONE;
        }

        xpStatus = xpSaiPortCfgSet(devId, portNum, &info,
                                   XPSAI_PORT_ATTR_LOCAL_ADVERT_FEC_MASK);
    }
    else
    {

        xpStatus = xpSaiPortCfgSet(devId, portNum, &info, XPSAI_PORT_ATTR_FEC_MASK);
    }
    return xpsStatus2SaiStatus(xpStatus);
}

//Func xpSaiGetPortAttrFecMode

sai_status_t xpSaiGetPortAttrFecMode(sai_object_id_t port_id,
                                     sai_attribute_value_t *value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (info.fec == XPSAI_PORT_ADVERT_FEC_RS)
    {
        value->s32 = SAI_PORT_FEC_MODE_RS;
    }
    else if (info.fec == XPSAI_PORT_ADVERT_FEC_FC)
    {
        value->s32 = SAI_PORT_FEC_MODE_FC;
    }
    else if (info.fec ==
             XPSAI_PORT_ADVERT_FEC_RS_544_514) //Not yet present in sai header file
    {
        value->s32 = SAI_PORT_FEC_MODE_RS;
    }
    else
    {
        value->s32 = SAI_PORT_FEC_MODE_NONE;
    }

    return xpsStatus2SaiStatus(xpStatus);
}

//Func xpSaiGetPortAttrSupportedFecMode

sai_status_t xpSaiGetPortAttrSupportedFecMode(sai_object_id_t port_id,
                                              sai_attribute_value_t *value)
{
    //xpsInterfaceId_t xpsIntf   = XPS_INTF_INVALID_ID;

    /* Get interface id */
    //xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    if (value->s32list.count < 2)
    {
        value->s32list.count = 2;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    value->s32list.count = 2;
    value->s32list.list[0] = SAI_PORT_FEC_MODE_FC;
    value->s32list.list[1] = SAI_PORT_FEC_MODE_RS;

#if 0
    if (value->s32list.count < 3)
    {
        value->s32list.count = 3;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    value->s32list.count = 3;
    value->s32list.list[0] = SAI_PORT_FEC_MODE_FC;
    value->s32list.list[1] = SAI_PORT_FEC_MODE_RS;
    value->s32list.list[2] = SAI_PORT_FEC_MODE_RS_544_514;
#endif

    return SAI_STATUS_SUCCESS;
}

//Func xpSaiSetPortAttrAdvertisedFecMode

sai_status_t xpSaiSetPortAttrAdvertisedFecMode(sai_object_id_t port_id,
                                               sai_attribute_value_t value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    uint8_t            i          = 0;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    for (i = 0; i < value.s32list.count; i++)
    {
        if (value.s32list.list[i] == SAI_PORT_FEC_MODE_RS)
        {
            info.localAdvertFec |= XPSAI_PORT_ADVERT_FEC_RS;
        }
        else if (value.s32list.list[i] == SAI_PORT_FEC_MODE_FC)
        {
            info.localAdvertFec |= XPSAI_PORT_ADVERT_FEC_FC;
        }
        /*
        else if (value.s32list.list[i] == SAI_PORT_FEC_MODE_RS_544_514) //Not yet supported in sai header file
        {
            info.localAdvertFec |= XPSAI_PORT_ADVERT_FEC_RS_544_514;
        }
        */
    }

    xpStatus = xpSaiPortCfgSet(devId, portNum, &info,
                               XPSAI_PORT_ATTR_LOCAL_ADVERT_FEC_MASK);

    return xpsStatus2SaiStatus(xpStatus);
}

//Func xpSaiGetPortAttrAdvertisedFecMode

sai_status_t xpSaiGetPortAttrAdvertisedFecMode(sai_object_id_t port_id,
                                               sai_attribute_value_t *value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    uint16_t           count      = 0;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;
    uint32_t           localAdvertFec;


    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    localAdvertFec = info.localAdvertFec;
    for (count = 0; localAdvertFec != 0; localAdvertFec >>= 1)
    {
        if (localAdvertFec & 0x01)
        {
            count++;
        }
    }

    if (count == 0)
    {
        count = 1;
    }

    if ((count > value->s32list.count) ||
        (value->s32list.count == 0))
    {
        value->s32list.count = count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    count = 0;

    if (info.localAdvertFec & XPSAI_PORT_ADVERT_FEC_FC)
    {
        value->s32list.list[count++] = SAI_PORT_FEC_MODE_FC;
    }

    if (info.localAdvertFec & XPSAI_PORT_ADVERT_FEC_RS)
    {
        value->s32list.list[count++] = SAI_PORT_FEC_MODE_RS;
    }

    if (info.localAdvertFec & XPSAI_PORT_ADVERT_FEC_RS_544_514)
    {
        value->s32list.list[count++] =
            SAI_PORT_FEC_MODE_RS; //Not yet supported in sai header file
    }

    if (count == 0)
    {
        value->s32list.list[0] = SAI_PORT_FEC_MODE_NONE;
        count = 1;
    }

    value->s32list.count = count;

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiGetPortAttrMacNum
sai_status_t xpSaiGetPortAttrMacNum(sai_object_id_t port_id,
                                    sai_attribute_value_t *value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    uint8_t            macNum = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsMacGetMacNumForPortNum(devId, portNum, &macNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get MAC number for port (%u)", portNum);
        return xpsStatus2SaiStatus(xpStatus);
    }

    value->u8 = macNum;

    return xpsStatus2SaiStatus(xpStatus);
}

//Func xpSaiGetPortAttrRemoteAdvertisedFecMode

sai_status_t xpSaiGetPortAttrRemoteAdvertisedFecMode(sai_object_id_t port_id,
                                                     sai_attribute_value_t *value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    uint16_t           count      = 0;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;
    uint32_t           remoteAdvertFec;


    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    remoteAdvertFec = info.remoteAdvertFec;
    for (count = 0; remoteAdvertFec != 0; remoteAdvertFec >>= 1)
    {
        if (remoteAdvertFec & 0x01)
        {
            count++;
        }
    }

    if ((count > value->s32list.count) ||
        (value->s32list.count == 0))
    {
        value->s32list.count = count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    count = 0;

    if (info.remoteAdvertFec & XPSAI_PORT_ADVERT_FEC_FC)
    {
        value->s32list.list[count++] = SAI_PORT_FEC_MODE_FC;
    }

    if (info.remoteAdvertFec & XPSAI_PORT_ADVERT_FEC_RS)
    {
        value->s32list.list[count++] = SAI_PORT_FEC_MODE_RS;
    }

    if (info.remoteAdvertFec & XPSAI_PORT_ADVERT_FEC_RS_544_514)
    {
        //value->s32list.list[count++] = SAI_PORT_FEC_MODE_RS_544_514; //Not yet supported in sai header file
    }

    if (count == 0)
    {
        value->s32list.list[0] = SAI_PORT_FEC_MODE_NONE;
        count = 1;
    }

    value->s32list.count = count;

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiSetPortQosAttributes

sai_status_t xpSaiSetPortQosAttributes(sai_object_id_t port_id,
                                       xpSaiPortAttributesT *attributes)
{
    sai_attribute_t         saiAttribute;
    sai_status_t            retVal = SAI_STATUS_SUCCESS;

    /*set policer Id*/
    saiAttribute.id = SAI_PORT_ATTR_POLICER_ID;
    saiAttribute.value = attributes->policerId;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_POLICER_ID attribute \n");
        return retVal;
    }

    /*set Default Tc*/
    saiAttribute.id = SAI_PORT_ATTR_QOS_DEFAULT_TC;
    saiAttribute.value = attributes->qosDefaultTc;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_QOS_DEFAULT_TC attribute \n");
        return retVal;
    }

    /*set QOS DOT1P TO TC MAP*/
    saiAttribute.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP;
    saiAttribute.value = attributes->qosDot1pToTcMap;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP attribute \n");
        return retVal;
    }

    /*set SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP*/
    saiAttribute.id = SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP;
    saiAttribute.value = attributes->qosDot1pToColorMap;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP attribute \n");
        return retVal;
    }

    /*set SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP*/
    saiAttribute.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP;
    saiAttribute.value = attributes->qosDscpToTcMap;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP attribute \n");
        return retVal;
    }

    /*set SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP*/
    saiAttribute.id = SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP;
    saiAttribute.value = attributes->qosDscpToColorMap;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP attribute \n");
        return retVal;
    }

    /*set SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP*/
    saiAttribute.id = SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP;
    saiAttribute.value = attributes->qosTcToQueueMap;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP attribute \n");
        return retVal;
    }

    /*set SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP*/
    saiAttribute.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP;
    saiAttribute.value = attributes->qosTcAndColorToDot1pMap;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP attribute \n");
        return retVal;
    }

    /*set SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP*/
    saiAttribute.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP;
    saiAttribute.value = attributes->qosTcAndColorToDscpMap;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP attribute \n");
        return retVal;
    }

    /*set SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP*/
    saiAttribute.id = SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP;
    saiAttribute.value = attributes->qosPfcPriorityToQueueMap;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP attribute \n");
        return retVal;
    }

    /*set SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID*/
    saiAttribute.id = SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID;
    saiAttribute.value = attributes->qosSchedulerProfileId;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID attribute \n");
        return retVal;
    }

    /* Set ingress buffer profile list */
    saiAttribute.id = SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST;
    saiAttribute.value = attributes->qosIngressBufferProfileList;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST attribute \n");
        return retVal;
    }

    saiAttribute.id = SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID;
    saiAttribute.value = attributes->floodStormControlPolicerId;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID attribute \n");
        return retVal;
    }

    saiAttribute.id = SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID;
    saiAttribute.value = attributes->broadcastStormControlPolicerId;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID attribute \n");
        return retVal;
    }

    saiAttribute.id = SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID;
    saiAttribute.value = attributes->multicastStormControlPolicerId;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID attribute \n");
        return retVal;
    }

    return retVal;
}

//Func xpSaiSetPortAttrHwProfileId
sai_status_t xpSaiSetPortAttrHwProfileId(sai_object_id_t port_id,
                                         sai_attribute_value_t value)
{
    XP_STATUS           xpStatus    = XP_NO_ERR;
    xpsInterfaceId_t    xpsIntf     = XPS_INTF_INVALID_ID;
    xpsDevice_t         devId       = xpSaiGetDevId();
    sai_uint8_t         idx         = 0;
    sai_uint64_t        profileId   = value.u64;

    xpMacConfigMode     macConfigMode;
    int16_t             portSerdesIds[MAX_SERDES_PER_PORT];
    xpSerdesTxEq_t      txEq;

    //TODO this is a hack for crb3-rev2.  Need to move logic to platform code eventually
    static const sai_uint16_t hwProfileIdCrb3Rev2[72][XP_SDK_PROFILE_MAX][3] =
    {
        {   /* Pre / Post / Attenuation */
            { 4, 15, 0 }, //profile 1 - Optic SFP / QSFP
            { 8, 0,  0 }, //profile 2 - 5M DAC
            { 6, 0,  0 }, //profile 3 - 3M DAC
            { 4, 0, 0 },  //profile 4 - 1-2M DAC
        },  //port 0
        { { 4, 12, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 1
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 2
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 3
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 4
        { { 4, 12, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 5
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 6
        { { 4, 12, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 7
        { { 4, 11, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 8
        { { 4, 14, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 9
        { { 4, 11, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 10
        { { 4, 14, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 11
        { { 4, 11, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 12
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 13
        { { 4, 11, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 14
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 15
        { { 4, 12, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 16
        { { 4, 12, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 17
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 18
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 19
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 20
        { { 4, 14, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 21
        { { 4, 14, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 22
        { { 4, 12, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 23
        { { 4, 12, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 24
        { { 4, 11, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 25
        { { 4, 11, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 26
        { { 4, 11, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 27
        { { 4, 11, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 28
        { { 4, 10, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 29
        { { 4, 10, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 30
        { { 4, 10, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 31
        { { 4, 10, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 32
        { { 4, 11, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 33
        { { 4, 11, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 34
        { { 4, 11, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 35
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 36
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 37
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 38
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 39
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 40
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 41
        { { 4, 14, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 42
        { { 4, 14, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 43
        { { 4, 14, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 44
        { { 4, 14, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 45
        { { 4, 14, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 46
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 47
        { { 4, 12, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 48
        { { 4, 14, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 49
        { { 4, 11, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 50
        { { 4, 12, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 51
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 52
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 53
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 54
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 55
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 56
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 57
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 58
        { { 4, 15, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 59
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 60
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 61
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 62
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 63
        { { 4, 12, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 64
        { { 4, 12, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 65
        { { 4, 12, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 66
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 67
        { { 4, 11, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 68
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 69
        { { 4, 13, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 70
        { { 4, 12, 0 }, { 8, 0, 0 }, { 6, 0, 0 }, { 4, 0, 0 } }, //port 71
    };

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u) \n", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (profileId == 0 || profileId > XP_SDK_PROFILE_MAX)
    {
        XP_SAI_LOG_ERR("Invalid Profile Id (%u) \n", profileId);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    //ignore for now if not XP70
    if (!IS_DEVICE_XP70(xpSaiSwitchDevTypeGet()))
    {
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    if (xpsIntf >= 72)
    {
        XP_SAI_LOG_ERR("Invalid Port Number (%u) \n", xpsIntf);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpStatus = xpsIsPortInited(devId, xpsIntf);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to set TxEq on interface (%u) port is not init, status = %d \n",
                       xpsIntf, xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsMacConfigModeGet(devId, xpsIntf, &macConfigMode);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to set TxEq on interface (%u) unable to get macMode, status = %d \n",
                       xpsIntf, xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpLinkManagerGetSerdesIdsForPort(devId, xpsIntf, macConfigMode,
                                                portSerdesIds);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to set TxEq on interface (%u) unable to get serdesIds, status = %d \n",
                       xpsIntf, xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    for (idx = 0; idx < MAX_SERDES_PER_PORT; idx++)
    {
        if (portSerdesIds[idx] != -1)
        {
            memset(&txEq, 0, sizeof(xpSerdesTxEq_t));
            txEq.pre = hwProfileIdCrb3Rev2[xpsIntf][profileId-1][0];
            txEq.post = hwProfileIdCrb3Rev2[xpsIntf][profileId-1][1];
            txEq.atten = hwProfileIdCrb3Rev2[xpsIntf][profileId-1][2];

            xpStatus = xpsSerdesSetTxEq(devId, portSerdesIds[idx], &txEq);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to set TxEq on interface (%u) SBUS (%u), status = %d \n",
                               xpsIntf, portSerdesIds[idx], xpStatus);
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
    }

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiSetCreatePortAttributes

sai_status_t xpSaiSetCreatePortAttributes(sai_object_id_t port_id,
                                          const sai_attribute_t *attr_list,
                                          uint32_t attr_count, xpSaiPortAttributesT *attributes)
{
    sai_attribute_t         saiAttribute;
    sai_status_t            retVal = SAI_STATUS_SUCCESS;

    /*set speed is already taken care in create API while doing Init port*/

    /*set admin state*/
    saiAttribute.id = SAI_PORT_ATTR_ADMIN_STATE;
    saiAttribute.value = attributes->adminState;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set SAI_PORT_ATTR_ADMIN_STATE attribute on create call \n");
        return retVal;
    }

    /*set port vlanId*/
    saiAttribute.id = SAI_PORT_ATTR_PORT_VLAN_ID;
    saiAttribute.value = attributes->portVlanId;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set SAI_PORT_ATTR_PORT_VLAN_ID attribute on create call \n");
        return retVal;
    }

    /*set port default vlan priority*/
    saiAttribute.id = SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY;
    saiAttribute.value = attributes->defaultVlanPriority;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set SAI_PORT_ATTR_PORT_VLAN_ID attribute on create call \n");
        return retVal;
    }

    /*set port MTU*/
    saiAttribute.id = SAI_PORT_ATTR_MTU;
    saiAttribute.value = attributes->mtu;

    retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set SAI_PORT_ATTR_MTU attribute on create call \n");
        return retVal;
    }

    /*Handling all QOS related attributes set default or user provided values in below API*/
    retVal = xpSaiSetPortQosAttributes(port_id, attributes);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("couldn't set QOS attributes attributes on create call \n");
        return retVal;
    }

    /* Apply SAI_PORT_ATTR_PKT_TX_ENABLE attribute */
    retVal = xpSaiSetPortAttrTxEnable(port_id, attributes->txEnable);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_PKT_TX_ENABLE) on create\n");
        return retVal;
    }

    /*set create only attributes*/

    /* Apply SAI_PORT_ATTR_FULL_DUPLEX_MODE attribute */
    retVal = xpSaiSetPortAttrFullDuplexMode(port_id, attributes->fullDuplexMode);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_FULL_DUPLEX_MODE) on create\n");
        return retVal;
    }

    /*set port INGRESS_ACL*/
    if (attributes->ingAclOid.oid != SAI_NULL_OBJECT_ID)
    {
        saiAttribute.id = SAI_PORT_ATTR_INGRESS_ACL;
        saiAttribute.value = attributes->ingAclOid;

        retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI_PORT_ATTR_INGRESS_ACL Failed \n");
            return retVal;
        }
    }
    /*set port EGRESS_ACL*/
    if (attributes->egrAclOid.oid != SAI_NULL_OBJECT_ID)
    {
        saiAttribute.id = SAI_PORT_ATTR_EGRESS_ACL;
        saiAttribute.value = attributes->egrAclOid;

        retVal = xpSaiSetPortAttribute(port_id, &saiAttribute);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI_PORT_ATTR_EGRESS_ACL Failed \n");
            return retVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiPortCreateDefaultQosResources

static sai_status_t xpSaiPortCreateDefaultQosResources(sai_object_id_t
                                                       switchObjId, sai_object_id_t portObjId)
{
    XP_STATUS        xpStatus   = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    sai_attribute_t *queueAttr  = NULL;
    sai_status_t     saiStatus  = SAI_STATUS_SUCCESS;
    uint32_t         schGrpNum  = 0;
    uint32_t         qNum       = 0;
    sai_object_id_t  portSchedGrpObjId = SAI_NULL_OBJECT_ID;
    sai_object_id_t  queueObjId        = SAI_NULL_OBJECT_ID;
    sai_object_id_t  schedulerObjId    = SAI_NULL_OBJECT_ID;
    sai_object_id_t  h1SchedGrpObjId   = SAI_NULL_OBJECT_ID;
    xpsDevice_t      xpDevId           = xpSaiObjIdSwitchGet(switchObjId);
    sai_uint32_t     portNum           = xpSaiObjIdValueGet(portObjId);

    queueAttr = (sai_attribute_t *)xpMalloc(sizeof(sai_attribute_t) * 5);
    if (!queueAttr)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for queueAttr\n");
        return xpsStatus2SaiStatus(XP_ERR_MEM_ALLOC_ERROR);
    }

    saiStatus = xpSaiQueueTxSchedulerProfileInit(xpDevId, portNum);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to init scheduler profiles list\n");
        xpFree(queueAttr);
        return saiStatus;
    }

    /* Get the default Scheduler object */
    saiStatus = xpSaiSwitchGetDefaultScheduler(&schedulerObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not Get the Default Scheduler, saiStatus: %d\n",
                       saiStatus);
        xpFree(queueAttr);
        return saiStatus;
    }

    xpSaiSchedulerGroupAttributesT schedGrpAttr;

    memset(queueAttr, 0, sizeof(sai_attribute_t)*5);
    memset(&schedGrpAttr, 0, sizeof(xpSaiSchedulerGroupAttributesT));

    schedGrpAttr.portId.oid             =
        portObjId;                     /* Port object id */
    schedGrpAttr.level.u8               =
        0;                             /* Scheduler group level*/
    schedGrpAttr.maxChilds.u8           =
        XPSAI_SG_MAX_CHILDS_AT_LEVEL0; /* Maximum Number of childs on group */
    schedGrpAttr.parentNode.oid         =
        portObjId;                     /* Scheduler group parent node */
    schedGrpAttr.schedulerProfileId.oid =
        schedulerObjId;                /* Scheduler id */

    /* Create port scheduler group at level-0 for each port */
    saiStatus = xpSaiSchedulerGroupCreate(switchObjId, &schedGrpAttr,
                                          &portSchedGrpObjId);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Error: Could not create scheduler group at level-0, "
                       "port: %d, saiStatus: %d\n", portNum, saiStatus);
        xpFree(queueAttr);
        return saiStatus;
    }

    for (schGrpNum = 0; schGrpNum < XPSAI_SG_MAX_CHILDS_AT_LEVEL0; schGrpNum++)
    {
        schedGrpAttr.portId.oid             =
            portObjId;                     /* Port object id */
        schedGrpAttr.level.u8               =
            1;                             /* Scheduler group level*/
        schedGrpAttr.maxChilds.u8           =
            XPSAI_SG_MAX_CHILDS_AT_LEVEL1; /* Maximum Number of childs on group */
        schedGrpAttr.parentNode.oid         =
            portSchedGrpObjId;             /* Scheduler group parent node */
        schedGrpAttr.schedulerProfileId.oid =
            schedulerObjId;                /* Scheduler id */

        /* Create scheduler group at level-1 for each port */
        saiStatus = xpSaiSchedulerGroupCreate(switchObjId, &schedGrpAttr,
                                              &h1SchedGrpObjId);
        if (SAI_STATUS_SUCCESS != saiStatus)
        {
            XP_SAI_LOG_ERR("Error: Could not create scheduler group at level-1, "
                           "port: %d, saiStatus: %d\n", portNum, saiStatus);
            xpFree(queueAttr);
            return saiStatus;
        }
        /* Populate queue type (All/Unicast/Multicast) */
        queueAttr[0].id = SAI_QUEUE_ATTR_TYPE;
        queueAttr[0].value.s32 = SAI_QUEUE_TYPE_ALL;

        /* Populate port object id */
        queueAttr[1].id = SAI_QUEUE_ATTR_PORT;
        queueAttr[1].value.oid = portObjId;

        /* Populate queue index with in port */
        queueAttr[2].id = SAI_QUEUE_ATTR_INDEX;
        queueAttr[2].value.u8 = schGrpNum;

        /* Populate parent scheduler node */
        queueAttr[3].id = SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE;
        queueAttr[3].value.oid = h1SchedGrpObjId;

        /* Populate parent scheduler node */
        queueAttr[4].id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
        queueAttr[4].value.oid = schedulerObjId;

        /* Create queue object */
        saiStatus = xpSaiCreateQueue(&queueObjId, switchObjId, 5, queueAttr);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to create queue, port: %d, queue: %d\n",
                           xpSaiObjIdValueGet(portObjId), schGrpNum);
            xpFree(queueAttr);
            return saiStatus;
        }
    }

    /*WA - Following queues 8 - 23 are added for UNICAST and MULTICAST type each
     * as a workaround for SONIC SNMP queue stats testcases. These are dummy queues
     * and have software states but dont have any presence in ASIC*/
    for (qNum = 8; qNum < 24; qNum++)
    {
        /* Populate queue type (All/Unicast/Multicast) */
        queueAttr[0].id = SAI_QUEUE_ATTR_TYPE;
        /* Queues 8 - 15 are Unicast and 16 - 23 are Multicast */
        if (qNum < 16)
        {
            queueAttr[0].value.s32 = SAI_QUEUE_TYPE_UNICAST;
        }
        else
        {
            queueAttr[0].value.s32 = SAI_QUEUE_TYPE_MULTICAST;
        }

        /* Populate port object id */
        queueAttr[1].id = SAI_QUEUE_ATTR_PORT;
        queueAttr[1].value.oid = portObjId;

        /* Populate queue index with in port */
        queueAttr[2].id = SAI_QUEUE_ATTR_INDEX;
        queueAttr[2].value.u8 = qNum;

        /* Populate parent scheduler node */
        queueAttr[3].id = SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE;
        queueAttr[3].value.oid = portObjId;
        /* Create queue object */
        saiStatus = xpSaiCreateQueue(&queueObjId, switchObjId, 4, queueAttr);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to create queue, port: %d, queue: %d\n",
                           xpSaiObjIdValueGet(portObjId), qNum);
            xpFree(queueAttr);
            return saiStatus;
        }
    }
    /*WA - SONIC SNMP queue stats END*/

    xpFree(queueAttr);

    sai_attribute_value_t value;

    /* Disable both LLFC and PFC on the newly created port */
    value.s32 = SAI_PORT_FLOW_CONTROL_MODE_DISABLE;

    saiStatus = xpSaiSetPortAttrGlobalFlowControlMode(portObjId, value);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to disable LLFC, port: %d, saiStatus: %d\n", portNum,
                       saiStatus);
        return saiStatus;
    }

    value.u8 = 0;

    saiStatus = xpSaiSetPortAttrPriorityFlowControl(portObjId, value,
                                                    pfcVectorUpdateTypeCombined);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to disable PFC, port: %d, saiStatus: %d\n", portNum,
                       saiStatus);
        return saiStatus;
    }

    sai_uint32_t    maxPriorityGroups  = 0;

    /* Get the total supported pfc traffic class */
    xpStatus = xpSaiPortGetNumPfcPrioritiesSupported(xpDevId, &maxPriorityGroups);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get the mumber of pfc priorities supported, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (maxPriorityGroups > 0)
    {
        xpSaiPortQosInfo_t *portQosEntry;
        saiStatus = xpSaiGetPortQosInfo(portObjId, &portQosEntry);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Getting Port QoS Info failed! PortOid = %" PRIu64
                           " saiStatus = %" PRIi32 "\n", portObjId, saiStatus);
            return saiStatus;
        }

        XP_SAI_LOG_DBG("Port QoS entry updated: changed numOfIngPrioGrps - %" PRIu32
                       "\n", portQosEntry->numOfIngPrioGrps);

        /* Create ingress priority groups supported on the port */
        for (uint32_t pfcTc = 0; pfcTc < maxPriorityGroups; ++pfcTc)
        {
            const uint8_t   attrCount = 3;
            sai_attribute_t attributes[attrCount];
            sai_object_id_t priorityGroupId = SAI_NULL_OBJECT_ID;

            memset(&attributes, 0, sizeof(attributes));

            attributes[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_PORT;
            attributes[0].value.oid = portObjId;

            attributes[1].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_INDEX;
            attributes[1].value.u8 = (sai_uint8_t) pfcTc;

            attributes[2].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
            attributes[2].value.oid = SAI_NULL_OBJECT_ID;

            saiStatus = xpSaiBufferCreateIngressPriorityGroupObject(&priorityGroupId,
                                                                    switchObjId, attrCount, attributes);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to create a ingress priority group object, saiStatus: %d\n",
                               saiStatus);
                return saiStatus;
            }

            XP_SAI_LOG_DBG("Port QoS entry updated: added ingPrioGrps item - %" PRIu64 "\n",
                           portQosEntry->ingPrioGrps[pfcTc]);
        }
    }

    if (IS_DEVICE_EBOF_PROFILE(xpSaiSwitchDevTypeGet()))
    {
        if (xpSaiPortCnpRateLimitInfo[portNum])
        {
            xpStatus = xpsAclAddCnpRateLimitRule(xpDevId, portNum);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Failed to set port CNP rate limiter, port: %d, xpStatus: %d\n",
                               portNum, xpStatus);
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
        else
        {
            XP_SAI_LOG_DBG("SKIP cnp rate limit on port: %d\n", portNum);
        }
    }

    /* By default set the Port Qos mode to L3. This is added
       to support DSCP-to-DSCP write for Tunnel*/
    rc = cpssHalPortQosTrustModeSet(xpDevId, portNum, GT_FALSE, GT_TRUE);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set port qos trust mode port: %d\n", portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    return saiStatus;
}

//Func: xpSaiPortRemoveDefaultQosResources

static sai_status_t xpSaiPortRemoveDefaultQosResources(sai_object_id_t
                                                       portObjId, uint32_t portNum)
{
    sai_attribute_value_t schedulerGrpObjList;
    sai_attribute_value_t queueObjList;
    sai_attribute_value_t count;
    sai_status_t          saiStatus = SAI_STATUS_SUCCESS;

    /* Get the total number of queues created for a given port */
    saiStatus = xpSaiGetPortAttrQosNumberOfQueues(portObjId, &count);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Error: Could not get number of queues for given port, "
                       "port: %d, saiStatus: %d\n", portNum, saiStatus);
        return saiStatus;
    }

    /* Fetch queue object list for a given port object and remove them */
    queueObjList.objlist.count = count.u32;
    queueObjList.objlist.list  = (sai_object_id_t*)xpMalloc(sizeof(
                                                                sai_object_id_t) * count.u32);
    if (queueObjList.objlist.list == NULL)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for queueObjList.objlist.list\n");
        return xpsStatus2SaiStatus(XP_ERR_MEM_ALLOC_ERROR);
    }

    saiStatus = xpSaiGetPortAttrQosQueueList(portObjId, &queueObjList);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Error: Could not fetch queue object list for a given"
                       " port: %d, saiStatus: %d\n", portNum, saiStatus);
        xpFree(queueObjList.objlist.list);
        return saiStatus;
    }

    for (uint32_t qIdx = 0; qIdx < queueObjList.objlist.count; qIdx++)
    {
        /* Remove queue API removes the queue’s resources as well */
        saiStatus = xpSaiRemoveQueue(queueObjList.objlist.list[qIdx]);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to remove queue %d, port %d\n", qIdx, portNum);
            xpFree(queueObjList.objlist.list);
            return saiStatus;
        }
    }

    xpFree(queueObjList.objlist.list);

    /* Get the total number of scheduler groups created for a given port */
    saiStatus = xpSaiGetPortAttrQosNumberOfSchedulerGroups(portObjId, &count);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Error: Could not get number of queues for given port, "
                       "port: %d, saiStatus: %d\n", portNum, saiStatus);
        return saiStatus;
    }

    /* Fetch scheduler group object list for a given port and remove them */
    schedulerGrpObjList.objlist.count = count.u32;
    schedulerGrpObjList.objlist.list  = (sai_object_id_t*)xpMalloc(sizeof(
                                                                       sai_object_id_t) * count.u32);
    if (schedulerGrpObjList.objlist.list == NULL)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for schedulerGrpObjList.objlist.list\n");
        return xpsStatus2SaiStatus(XP_ERR_MEM_ALLOC_ERROR);
    }

    saiStatus = xpSaiGetPortAttrQosSchedulerGroupList(portObjId,
                                                      &schedulerGrpObjList);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get the scheduler group object list, port: %d, saiStatus: %d\n",
                       portNum, saiStatus);
        xpFree(schedulerGrpObjList.objlist.list);
        return saiStatus;
    }

    for (uint32_t idx = 0; idx < schedulerGrpObjList.objlist.count; idx++)
    {
        /* Scheduler group remove removes the scheduler group’s resources as well */
        saiStatus = xpSaiSchedulerGroupRemove(schedulerGrpObjList.objlist.list[idx]);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to remove scheduler group %d, port %d\n", idx,
                           portNum);
            xpFree(schedulerGrpObjList.objlist.list);
            return saiStatus;
        }
    }

    xpFree(schedulerGrpObjList.objlist.list);

    xpSaiPortQosInfo_t *portQosEntry = NULL;

    saiStatus = xpSaiGetPortQosInfo(portObjId, &portQosEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get port qos entry from DB! saiStatus %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    /* Remove ingress priority groups created on the port */
    while (portQosEntry->numOfIngPrioGrps > 0)
    {
        if (portQosEntry->ingPrioGrps[portQosEntry->numOfIngPrioGrps-1] !=
            SAI_NULL_OBJECT_ID)
        {
            saiStatus = xpSaiBufferRemoveIngressPriorityGroupObject(
                            portQosEntry->ingPrioGrps[portQosEntry->numOfIngPrioGrps-1]);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to remove ingress priority group object, saiStatus: %d\n",
                               saiStatus);
                return saiStatus;
            }
        }
        else
        {
            XP_SAI_LOG_ERR("Ingress priority group object is null on portId: %" PRIu64 "\n",
                           portObjId);
            break;
        }
    }

    sai_attribute_value_t value;

    /* Disable LLFC on port */
    value.s32 = SAI_PORT_FLOW_CONTROL_MODE_DISABLE;

    saiStatus = xpSaiSetPortAttrGlobalFlowControlMode(portObjId, value);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to disable LLFC, port: %d, saiStatus: %d\n",
                       portNum, saiStatus);
        return saiStatus;
    }

    value.u8 = 0;

    /* Disable PFC on port */
    saiStatus = xpSaiSetPortAttrPriorityFlowControl(portObjId, value,
                                                    pfcVectorUpdateTypeCombined);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to disable PFC, port: %d, saiStatus: %d\n",
                       portNum, saiStatus);
        return saiStatus;
    }

    xpSaiPortAttributesT *attributes = (xpSaiPortAttributesT *)xpMalloc(sizeof(
                                                                            xpSaiPortAttributesT));
    if (!attributes)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for attributes\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(attributes, 0, sizeof(xpSaiPortAttributesT));

    /* Set port attributes to default */
    xpSaiSetDefaultPortAttributeVals(xpSaiObjIdSwitchGet(portObjId), attributes);

    saiStatus = xpSaiSetPortQosAttributes(portObjId, attributes);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        xpFree(attributes);
        XP_SAI_LOG_ERR("Error: Failed to set qos attributes to default, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    xpFree(attributes);
    return saiStatus;
}

//Func: xpSaiCreatePort

sai_status_t xpSaiCreatePort(sai_object_id_t *port_id,
                             sai_object_id_t switch_id, uint32_t attr_count,
                             const sai_attribute_t *attr_list)
{
    xpSaiPortAttributesT  *attributes = NULL;
    sai_status_t            retVal = SAI_STATUS_SUCCESS;
    sai_status_t            saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS               status = XP_NO_ERR;
    xpsPort_t               xpsPortId = 255;
    xpsInterfaceId_t        xpsPortIntf = XPS_INTF_INVALID_ID;
    xpsDevice_t             xpsDevId = 0;
    xpSaiPortDbEntryT       *pPortEntry = NULL;
    xpSaiPortMgrInfo_t      *info = NULL;
    XP_SPEED                pgSpeed;
    uint32_t                numHwQs = 0;
    sai_mac_t               switchMac;
    uint32_t                actionMask = 0;
    xpSaiPortStatisticDbEntryT *portStatistics = NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            PORT_VALIDATION_ARRAY_SIZE, port_attribs,
                            SAI_COMMON_API_CREATE);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    /*get devId from SwitchId*/
    xpsDevId = xpSaiObjIdSwitchGet(switch_id);

    attributes = (xpSaiPortAttributesT *)xpMalloc(sizeof(xpSaiPortAttributesT));
    if (!attributes)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for attributes\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(attributes, 0, sizeof(xpSaiPortAttributesT));

    xpSaiSetDefaultPortAttributeVals(xpsDevId, attributes);
    xpSaiUpdatePortAttributeVals(attr_count, attr_list, attributes);

    if (attributes->hwLaneList.u32list.count > 8)
    {
        xpFree(attributes);
        // We can handle up to 8 lanes in the list
        return SAI_STATUS_INVALID_PARAMETER;
    }

    XP_SAI_LOG_LANES((&attributes->hwLaneList.u32list));

    xpsPortId = xpSaiGetPortByLanes(xpsDevId, attributes->hwLaneList.u32list);
    if (xpsPortId == XPS_INTF_INVALID_ID)
    {
        xpFree(attributes);
        XP_SAI_LOG_ERR("Failed to convert lanes to port number\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    XP_SAI_LOG_DBG("xpsPortId : %d\n", xpsPortId);

    info = (xpSaiPortMgrInfo_t *)xpMalloc(sizeof(xpSaiPortMgrInfo_t));
    if (!info)
    {
        xpFree(attributes);
        XP_SAI_LOG_ERR("Error: allocation failed for info\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(info, 0, sizeof(xpSaiPortMgrInfo_t));

    xpSaiPortInfoInit(info);
    status = xpSaiPortCfgGet(xpsDevId, xpsPortId, info);
    if (status != XP_NO_ERR)
    {
        xpFree(attributes);
        xpFree(info);
        return xpsStatus2SaiStatus(status);
    }

    /*get port interface from portId*/
    status = xpsPortGetPortIntfId(xpsDevId, xpsPortId, &xpsPortIntf);
    if (status != XP_NO_ERR)
    {
        xpFree(attributes);
        xpFree(info);
        XP_SAI_LOG_ERR("%s:Error: couldn't get portIntf id for device: %d, port: %d\n",
                       __FUNCNAME__, xpsDevId, xpsPortId);
        return xpsStatus2SaiStatus(status);
    }
    /*insert entry in port statistic DB*/
    status = xpSaiInsertPortStatisticDbEntry(xpsPortIntf);
    if (XP_NO_ERR != status)
    {
        XP_SAI_LOG_ERR("Could not insert entry in port statistic DB for portIntf %u.",
                       xpsPortIntf);
        saiStatus = xpSaiPortDbDelete(xpsPortIntf, &pPortEntry);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to delete PortIntf %u  \n", xpsPortIntf);
            xpFree(attributes);
            xpFree(info);
            return saiStatus;
        }
        xpFree(attributes);
        xpFree(info);
        return xpsStatus2SaiStatus(status);
    }

    /*check if port is already created*/

    retVal = xpSaiPortDbInfoGet(xpsPortIntf, &pPortEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Coulnd't find port intf %u in SAI port DB, we can create port \n",
                       xpsPortIntf);
    }

    if (pPortEntry != NULL)
    {
        xpFree(attributes);
        xpFree(info);
        XP_SAI_LOG_ERR("Create is being called for port intf %u, which is already created \n",
                       xpsPortIntf);
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    xpSaiPortInfoInit(info);
    XP_SAI_LOG_DBG("Port %u break-out with number of lanes %u speed %u\n",
                   xpsPortId, attributes->hwLaneList.u32list.count,  attributes->speed.u32);

    if (attributes->hwLaneList.u32list.count == 8)
    {
        info->breakOutMode = XPSAI_PORT_BREAKOUT_MODE_8X;
    }
    else if (attributes->hwLaneList.u32list.count == 4)
    {
        info->breakOutMode = XPSAI_PORT_BREAKOUT_MODE_4X;
    }
    else if (attributes->hwLaneList.u32list.count == 2)
    {
        info->breakOutMode = XPSAI_PORT_BREAKOUT_MODE_2X;
    }
    else if (attributes->hwLaneList.u32list.count == 1)
    {
        info->breakOutMode = XPSAI_PORT_BREAKOUT_MODE_1X;
    }
    else
    {
        XP_SAI_LOG_ERR("Port %u break-out with number of lanes %u not supported\n",
                       xpsPortId, attributes->hwLaneList.u32list.count);
        xpFree(attributes);
        xpFree(info);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    info->speed = attributes->speed.u32;

    actionMask = XPSAI_PORT_ATTR_BREAKOUT_MASK | XPSAI_PORT_ATTR_SPEED_MASK;
    if (attributes->intfType.s32 != SAI_PORT_INTERFACE_TYPE_NONE)
    {
        info->intfType = attributes->intfType.s32;
        actionMask |= XPSAI_PORT_ATTR_INTF_TYPE_MASK;
    }

    if (SAI_PORT_INTERNAL_LOOPBACK_MODE_MAC == attributes->internalLoopbackMode.s32)
    {
        info->loopbackMode = XPSAI_PORT_LOOPBACK_MODE_MAC;
        actionMask |= XPSAI_PORT_ATTR_LOOPBACK_MASK;
    }
    else if (SAI_PORT_INTERNAL_LOOPBACK_MODE_PHY ==
             attributes->internalLoopbackMode.s32)
    {
        info->loopbackMode = XPSAI_PORT_LOOPBACK_MODE_PHY;
        actionMask |= XPSAI_PORT_ATTR_LOOPBACK_MASK;
    }
    else
    {
        info->loopbackMode = XPSAI_PORT_LOOPBACK_MODE_NONE;
    }

    status = xpSaiPortCfgSet(xpsDevId, xpsPortId, info, actionMask);
    if (XP_NO_ERR != status)
    {
        xpFree(attributes);
        xpFree(info);
        XP_SAI_LOG_ERR("Failed to apply break-out mode and speed config for portIntf %u\n",
                       xpsPortId);
        return xpsStatus2SaiStatus(status);
    }

    retVal = xpSaiPortDbInsert(xpsPortIntf);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        xpFree(attributes);
        xpFree(info);
        XP_SAI_LOG_ERR("Could not insert port info into DB");
        return retVal;
    }

    status = xpsPortSetField(xpsDevId, xpsPortIntf, XPS_PORT_ACCEPTED_FRAME_TYPE,
                             FRAMETYPE_ALL);
    if (XP_NO_ERR != status)
    {
        XP_SAI_LOG_ERR("Failed to set XPS_PORT_ACCEPTED_FRAME_TYPE for port %u with error %d\n",
                       xpsPortIntf, status);
        saiStatus = xpSaiPortDbDelete(xpsPortIntf, &pPortEntry);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            xpFree(attributes);
            xpFree(info);
            XP_SAI_LOG_ERR("Failed to delete PoirtIntf %u  \n", xpsPortIntf);
            return saiStatus;
        }
        xpFree(attributes);
        xpFree(info);
        return xpsStatus2SaiStatus(status);
    }

    retVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, xpsDevId,
                              (sai_uint64_t) xpsPortIntf, port_id);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error : SAI port object could not be created.\n");
        saiStatus = xpSaiPortDbDelete(xpsPortIntf, &pPortEntry);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            xpFree(attributes);
            xpFree(info);
            XP_SAI_LOG_ERR("Failed to delete PoirtIntf %u  \n", xpsPortIntf);
            return saiStatus;
        }
        xpFree(attributes);
        xpFree(info);
        return retVal;
    }

    /*insert entry in QOS port DB*/
    status = xpSaiInsertPortQosDbEntry(*port_id);
    if (XP_NO_ERR != status)
    {
        XP_SAI_LOG_ERR("Could not insert entry in QOS port DB for portIntf %u.",
                       xpsPortIntf);
        saiStatus = xpSaiPortDbDelete(xpsPortIntf, &pPortEntry);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            xpFree(attributes);
            xpFree(info);
            XP_SAI_LOG_ERR("Failed to delete PoirtIntf %u  \n", xpsPortIntf);
            return saiStatus;
        }
        xpFree(attributes);
        xpFree(info);
        return xpsStatus2SaiStatus(status);

    }

    /*do readonly,  create only  and create-set variable settings in this case*/
    retVal = xpSaiSetCreatePortAttributes(*port_id, attr_list, attr_count,
                                          attributes);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI port %u was not properly configured.\n", xpsPortIntf);
        status = xpSaiRemovePortQosDbEntry(*port_id);
        if (XP_NO_ERR != status)
        {
            xpFree(attributes);
            xpFree(info);
            XP_SAI_LOG_ERR("Could not remove entry in QOS port DB for portIntf %u.",
                           xpsPortIntf);
            return xpsStatus2SaiStatus(status);
        }
        saiStatus = xpSaiPortDbDelete(xpsPortIntf, &pPortEntry);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            xpFree(attributes);
            xpFree(info);
            XP_SAI_LOG_ERR("Failed to delete PoirtIntf %u  \n", xpsPortIntf);
            return saiStatus;
        }
        xpFree(attributes);
        xpFree(info);
        return retVal;
    }

    /* Initially disable all the queues and enable only user created queues by default */
    /* Get the number of queues per port */
    if ((status = xpsQosAqmGetNumberOfQueuesPerPort(xpsDevId, xpsPortId,
                                                    &numHwQs)) != XP_NO_ERR)
    {
        xpFree(attributes);
        xpFree(info);
        XP_SAI_LOG_ERR("Failed to get number of queues per port, device: %d, port: %d %u\n",
                       xpsDevId, xpsPortId);
        return xpsStatus2SaiStatus(status);
    }

    status = xpSaiProfileMngSyncPortToProfile(xpsDevId, xpsPortId);
    if (XP_NO_ERR != status)
    {
        xpFree(attributes);
        xpFree(info);
        XP_SAI_LOG_ERR("xpSaiProfileMngSyncPortToProfile failed for port %d",
                       xpsPortId);
        return xpsStatus2SaiStatus(status);
    }

    retVal = xpSaiPortCreateDefaultQosResources(switch_id, *port_id);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        xpFree(attributes);
        xpFree(info);
        XP_SAI_LOG_ERR("Create Default Qos Resources failed with error %d for port %d\n",
                       retVal, xpsPortId);
        return retVal;
    }

    if (IS_DEVICE_XP70_XP60(xpSaiSwitchDevTypeGet()))
    {
        // Apply Ibuffer configuration, as this device supports it
        // Determine port speed
        pgSpeed = xpSaiPortSpeedToXpsPgSpeed(info->speed);

        // From switch db, get the configuration for this port
        // Configure the Ibuffer accordingly

        sai_uint32_t llAlloc = 0;
        xpSaiSwitchGetIbufferLosslessAllocation(xpsDevId, pgSpeed, &llAlloc);
        if ((status = xpsQosIbConfigurePortLosslessDropThreshold(xpsDevId, xpsPortId,
                                                                 (uint32_t)llAlloc)) != XP_NO_ERR)
        {
            xpFree(attributes);
            xpFree(info);
            XP_SAI_LOG_ERR("Failed to configure the IBuffer lossless allocation for device: %d, port: %d",
                           xpsDevId, xpsPortId);
            return xpsStatus2SaiStatus(status);
        }

        sai_uint32_t llXoff = 0;
        xpSaiSwitchGetIBufferLosslessXoff(xpsDevId, pgSpeed, &llXoff);
        if ((status = xpsQosIbConfigurePortLosslessXoffThreshold(xpsDevId, xpsPortId,
                                                                 (uint32_t)llXoff)) != XP_NO_ERR)
        {
            xpFree(attributes);
            xpFree(info);
            XP_SAI_LOG_ERR("Failed to configure the IBuffer lossless xoff for device: %d, port: %d",
                           xpsDevId, xpsPortId);
            return xpsStatus2SaiStatus(status);
        }

        sai_uint32_t llXon = 0;
        xpSaiSwitchGetIbufferLosslessXon(xpsDevId, pgSpeed, &llXon);
        if ((status = xpsQosIbConfigurePortLosslessXonThreshold(xpsDevId, xpsPortId,
                                                                (uint32_t)llXon)) != XP_NO_ERR)
        {
            xpFree(attributes);
            xpFree(info);
            XP_SAI_LOG_ERR("Failed to configure the IBuffer lossless xoff for device: %d, port: %d",
                           xpsDevId, xpsPortId);
            return xpsStatus2SaiStatus(status);
        }

        for (uint32_t priority = 0; priority < XPSAI_MAX_PRIORITIES_SUPPORTED;
             priority++)
        {
            sai_uint32_t lyDrop = 0;
            xpSaiSwitchGetIBufferLossyDropLevel(xpsDevId, pgSpeed, priority, &lyDrop);
            if ((status = xpsQosIbConfigurePortLossyDropThresholdPerPriority(xpsDevId,
                                                                             xpsPortId, priority, (uint32_t)lyDrop)) != XP_NO_ERR)
            {
                xpFree(attributes);
                xpFree(info);
                XP_SAI_LOG_ERR("Failed to configure the IBuffer lossy drop fill level for device: %d, port: %d, priority: %d",
                               xpsDevId, xpsPortId, priority);
                return xpsStatus2SaiStatus(status);
            }
        }
    }

    saiStatus = xpSaiGetSwitchSrcMacAddress(switchMac);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        xpFree(attributes);
        xpFree(info);
        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_SRC_MAC_ADDRESS)\n");
        return saiStatus;
    }
    XP_SAI_LOG_DBG(" SwitchMac %x:%x:%x:%x:%x:%x\n", switchMac[0], switchMac[1],
                   switchMac[2], switchMac[3],
                   switchMac[4], switchMac[5]);
    /*
     * Set MAC LSB per port. Since we have switch Level MAC, set the same LSB to
     * to all ports.
     */
    status = xpsPortSAMacLsbSet(xpsDevId, xpsPortId, switchMac[5]);
    if (status != XP_NO_ERR)
    {
        xpFree(attributes);
        xpFree(info);
        XP_SAI_LOG_ERR("Failed to MAC LSB: %d, port: %d, macLSB: %d", xpsDevId,
                       xpsPortId, switchMac[5]);
        return xpsStatus2SaiStatus(status);
    }

    status = xpSaiGetPortStatisticInfo(xpsPortId, &portStatistics);
    if (XP_NO_ERR != status)
    {
        xpFree(attributes);
        xpFree(info);
        XP_SAI_LOG_ERR("Error: Failed to get the port statistic data, xpStatus: %d\n",
                       status);
        return xpsStatus2SaiStatus(status);
    }

    /*TODO Commenting creation of Mac Stat Counter rules. Will revisit if requirement arises*/
#if 0 
    /*create v4/v6 acl rule for port*/
    status = xpsMacStatCounterCreateAclRuleForV4V6(xpsDevId, xpsPortId,
                                                   &portStatistics->counterId_Ingress_v4, &portStatistics->counterId_Ingress_v6,
                                                   &portStatistics->counterId_Egress_v4,  &portStatistics->counterId_Egress_v6);
    if (XP_NO_ERR != status)
    {
        xpFree(attributes);
        xpFree(info);
        XP_SAI_LOG_ERR("Could not create Acl rule for port v4v6 statistics for the port %u.\n",
                       xpsPortId);
        return  xpsStatus2SaiStatus(status);
    }

    status = (XP_STATUS) xpsMacStatCounterReset(xpsDevId, xpsPortId);
    if (XP_NO_ERR != status)
    {
        xpFree(attributes);
        xpFree(info);
        XP_SAI_LOG_ERR("Could not clear statistics for the port %u.\n", xpsPortId);
        return  xpsStatus2SaiStatus(status);
    }
#endif

    xpFree(attributes);
    xpFree(info);
    return retVal;
}

//Func: xpSaiRemovePort

sai_status_t xpSaiRemovePort(sai_object_id_t port_id)
{
    sai_status_t        saiStatus    = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus     = XP_NO_ERR;
    xpsInterfaceId_t    xpsIntf      = XPS_INTF_INVALID_ID;
    xpsDevice_t         xpsDevId     = 0;
    xpsPort_t           xpsPortId    = 0;
    xpSaiPortDbEntryT   *pPortEntry = NULL;
    xpSaiPortMgrInfo_t  info;
    xpSaiPortStatisticDbEntryT *portStatistics = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiRemovePort\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    /*get device and Port Id*/
    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &xpsDevId, &xpsPortId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get DeviceId and Port number in xpSaiRemovePort.\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    saiStatus = xpSaiPortRemoveDefaultQosResources(port_id, xpsPortId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Remove Default Qos Resources failed with error %d for port %d\n",
                       saiStatus, xpsPortId);
        return saiStatus;
    }

    xpSaiPortInfoInit(&info);
    info.breakOutMode = XPSAI_PORT_BREAKOUT_MODE_NONE;
    xpStatus = xpSaiPortCfgSet(xpsDevId, xpsPortId, &info,
                               XPSAI_PORT_ATTR_BREAKOUT_MASK);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Could not set break out mode to NONE for portIntf %u.",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /*insert entry in QOS port DB*/

    xpStatus = xpSaiRemovePortQosDbEntry(port_id);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Could not remove entry in QOS port DB for portIntf %u.",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpSaiGetPortStatisticInfo(xpsPortId, &portStatistics);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Error: Failed to get the port statistic data, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /*Delete acl rule of v4/v6 counter for port*/
    xpStatus = xpsMacStatCounterRemoveAclRuleForV4V6(xpsDevId, xpsPortId,
                                                     portStatistics->counterId_Ingress_v4, portStatistics->counterId_Ingress_v6,
                                                     portStatistics->counterId_Egress_v4,  portStatistics->counterId_Egress_v6);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Could not remove Acl rule for port v4v6 statistics for the port %u.\n",
                       xpsPortId);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    /*Delete port statistic DB entry*/
    xpStatus = xpSaiRemovePortStatisticDbEntry(xpsPortId);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Error: Failed to remove the port statistic data, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /*Delete port DB entry*/

    saiStatus = xpSaiPortDbDelete(xpsIntf, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to delete PoirtIntf %u  \n", xpsIntf);
        return saiStatus;
    }

    return saiStatus;
}


//Func: xpSaiPortNotifyAddAcl

sai_status_t xpSaiPortNotifyAddAcl(xpsDevice_t devId,
                                   sai_object_id_t portObjId, sai_object_id_t aclObjId)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    saiStatus = xpSaiAclMapperBindToTableGroup(devId, aclObjId, portObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to bind port to table group, status:%d\n", saiStatus);
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiSetPortAttrIngressAcl(sai_object_id_t port_id,
                                        sai_attribute_value_t value)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus    = XP_NO_ERR;
    sai_uint32_t        tableId     = 0;
    sai_object_id_t     aclObjId    = value.oid;
    xpsPort_t           xpsPortId   = 0;
    sai_uint32_t        groupId     = 0;
    xpSaiPortDbEntryT  *pPortEntry  = NULL;
    xpsDevice_t         devId       = xpSaiGetDevId();
    xpSaiAclTableIdMappingT  *pSaiAclTableAttribute = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrIngressAcl\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsPortId = (xpsPort_t)xpSaiObjIdValueGet(port_id);

    saiStatus = xpSaiPortDbInfoGet(xpsPortId, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Coulnd't find port intf %u in SAI port DB\n", xpsPortId);
        return saiStatus;
    }

    if (pPortEntry->ingressAclId == aclObjId)
    {
        return SAI_STATUS_SUCCESS;
    }


    if (pPortEntry->ingressAclId != SAI_NULL_OBJECT_ID)
    {
        if (XDK_SAI_OBJID_TYPE_CHECK(pPortEntry->ingressAclId,
                                     SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
        {
            saiStatus = xpSaiAclMapperUnBindFromTableGroup(devId, pPortEntry->ingressAclId,
                                                           port_id);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to notify add ingress acl, port:%d, status:%d\n",
                               xpsPortId, saiStatus);
                return saiStatus;
            }
        }
        else if (XDK_SAI_OBJID_TYPE_CHECK(pPortEntry->ingressAclId,
                                          SAI_OBJECT_TYPE_ACL_TABLE))
        {
            tableId = (xpsPort_t)xpSaiObjIdValueGet(pPortEntry->ingressAclId);
            xpStatus = xpsAclPortUnbind(devId, xpsPortId, groupId, tableId);

            if (xpStatus != XP_NO_ERR)
            {
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
    }
    if (aclObjId != SAI_NULL_OBJECT_ID)
    {
        if (!XDK_SAI_OBJID_TYPE_CHECK(aclObjId, SAI_OBJECT_TYPE_ACL_TABLE) &&
            !XDK_SAI_OBJID_TYPE_CHECK(aclObjId, SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
        {
            XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                           xpSaiObjIdTypeGet(aclObjId));
            return SAI_STATUS_INVALID_PARAMETER;
        }

        if (XDK_SAI_OBJID_TYPE_CHECK(aclObjId, SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
        {
            // Send add ingress acl notification to acl module to update it's table group bind list
            saiStatus = xpSaiPortNotifyAddAcl(devId, port_id, aclObjId);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to notify add ingress acl, port:%d, status:%d\n",
                               xpsPortId, saiStatus);
                return saiStatus;
            }
        }
        else
        {
            // Binding ACL table to port

            // Get the acl table id from table object
            tableId = (xpsPort_t)xpSaiObjIdValueGet(aclObjId);
            saiStatus = xpSaiAclTableAttributesGet(tableId, &pSaiAclTableAttribute);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not get Table Entry, ret %d\n", saiStatus);
                return saiStatus;
            }

            xpStatus = xpsAclSetPortAcl(devId, xpsPortId, tableId, groupId);

            if (xpStatus != XP_NO_ERR)
            {
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
    }

    // Update state database
    pPortEntry->ingressAclId = aclObjId;

    return saiStatus;
}


//Func: xpSaiGetPortAttrIngressAcl

sai_status_t xpSaiGetPortAttrIngressAcl(sai_object_id_t port_id,
                                        sai_attribute_value_t* value)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpsPort_t           xpsPortId   = 0;
    xpSaiPortDbEntryT   *pPortEntry = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrIngressAcl\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsPortId = (xpsPort_t)xpSaiObjIdValueGet(port_id);

    saiStatus = xpSaiPortDbInfoGet(xpsPortId, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Coulnd't find port intf %u in SAI port DB \n", xpsPortId);
        return saiStatus;
    }

    value->oid = pPortEntry->ingressAclId;

    return saiStatus;
}


//Func: xpSaiSetPortAttrEgressAcl

sai_status_t xpSaiSetPortAttrEgressAcl(sai_object_id_t port_id,
                                       sai_attribute_value_t value)
{
    sai_status_t         saiStatus  = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus    = XP_NO_ERR;
    xpsPort_t            xpsPortId  = 0;
    sai_uint32_t         tableId    = 0;
    sai_uint32_t        groupId     = 0;
    sai_object_id_t     aclObjId    = value.oid;
    xpsDevice_t         devId       = xpSaiGetDevId();
    xpSaiPortDbEntryT   *pPortEntry = NULL;
    xpSaiAclTableIdMappingT  *pSaiAclTableAttribute = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiSetPortAttrEgressAcl\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsPortId = (xpsPort_t)xpSaiObjIdValueGet(port_id);

    saiStatus = xpSaiPortDbInfoGet(xpsPortId, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Coulnd't find port intf %u in SAI port DB \n", xpsPortId);
        return saiStatus;
    }

    if (pPortEntry->egressAclId == aclObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (pPortEntry->egressAclId != SAI_NULL_OBJECT_ID)
    {
        if (XDK_SAI_OBJID_TYPE_CHECK(pPortEntry->egressAclId,
                                     SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
        {
            saiStatus = xpSaiAclMapperUnBindFromTableGroup(devId, pPortEntry->egressAclId,
                                                           port_id);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_DBG("Failed to notify add ingress acl, port:%d, status:%d\n",
                               xpsPortId, saiStatus);
                return saiStatus;
            }
        }
        else if (XDK_SAI_OBJID_TYPE_CHECK(pPortEntry->egressAclId,
                                          SAI_OBJECT_TYPE_ACL_TABLE))
        {
            tableId = (xpsPort_t)xpSaiObjIdValueGet(pPortEntry->egressAclId);
            xpStatus = xpsAclPortUnbind(devId, xpsPortId, groupId, tableId);
            if (xpStatus != XP_NO_ERR)
            {
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
    }

    if (aclObjId != SAI_NULL_OBJECT_ID)
    {
        if (!XDK_SAI_OBJID_TYPE_CHECK(value.oid, SAI_OBJECT_TYPE_ACL_TABLE) &&
            !XDK_SAI_OBJID_TYPE_CHECK(value.oid, SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
        {
            XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(value.oid));
            return SAI_STATUS_INVALID_PARAMETER;
        }

        if (XDK_SAI_OBJID_TYPE_CHECK(aclObjId, SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
        {

            // Send add ingress acl notification to acl module to update it's table group bind list
            saiStatus = xpSaiPortNotifyAddAcl(devId, port_id, aclObjId);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_DBG("Failed to notify add ingress acl, port:%d, status:%d\n",
                               xpsPortId, saiStatus);
                return saiStatus;
            }
        }
        else
        {
            // Binding ACL table to port

            // Get the acl table id from table object
            tableId = (xpsPort_t)xpSaiObjIdValueGet(aclObjId);
            saiStatus = xpSaiAclTableAttributesGet(tableId, &pSaiAclTableAttribute);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not get Table Entry, ret %d\n", saiStatus);
                return saiStatus;
            }

            xpStatus = xpsAclSetPortAcl(devId, xpsPortId, tableId, groupId);

            if (xpStatus != XP_NO_ERR)
            {
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
    }

    pPortEntry->egressAclId = aclObjId;
    return saiStatus;

}

//Func: xpSaiGetPortAttrEgressAcl

sai_status_t xpSaiGetPortAttrEgressAcl(sai_object_id_t port_id,
                                       sai_attribute_value_t* value)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpsPort_t           xpsPortId   = 0;
    xpSaiPortDbEntryT   *pPortEntry = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrEgressAcl\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsPortId = (xpsPort_t)xpSaiObjIdValueGet(port_id);

    saiStatus = xpSaiPortDbInfoGet(xpsPortId, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Coulnd't find port intf %u in SAI port DB \n", xpsPortId);
        return saiStatus;
    }

    if (pPortEntry->egressAclId == 0)
    {
        value->oid = SAI_NULL_OBJECT_ID;
    }
    else
    {
        value->oid = (sai_object_id_t)pPortEntry->egressAclId;
    }
    return saiStatus;
}

//Func: xpSaiSetPortAttrIngressSampleSessionOid

sai_status_t xpSaiSetPortAttrIngressSampleSessionOid(xpsInterfaceId_t xpsIntf,
                                                     sai_object_id_t sampleSessionOid)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT *pPortEntry = NULL;

    saiStatus = xpSaiPortDbInfoGet(xpsIntf, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB");
        return saiStatus;
    }

    pPortEntry->ingressSampleSessionOid = sampleSessionOid;

    return  saiStatus;
}

//Func: xpSaiGetPortAttrIngressSampleSessionOid

sai_status_t xpSaiGetPortAttrIngressSampleSessionOid(xpsInterfaceId_t xpsIntf,
                                                     sai_object_id_t *sampleSessionOid)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT *pPortEntry = NULL;

    saiStatus = xpSaiPortDbInfoGet(xpsIntf, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB");
        return saiStatus;
    }

    *sampleSessionOid = pPortEntry->ingressSampleSessionOid;

    return  saiStatus;
}


sai_status_t xpSaiGetInterfaceType(sai_object_id_t port_id,
                                   sai_attribute_value_t* value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    value->s32 = info.intfType;

    return xpsStatus2SaiStatus(xpStatus);
}

sai_status_t xpSaiSetInterfaceType(sai_object_id_t port_id,
                                   sai_attribute_value_t value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    info.intfType = value.s32;
    xpStatus = xpSaiPortCfgSet(devId, portNum, &info,
                               XPSAI_PORT_ATTR_INTF_TYPE_MASK);

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiSetPortAttrEgressSampleSessionOid
sai_status_t xpSaiSetPortAttrEgressSampleSessionOid(xpsInterfaceId_t xpsIntf,
                                                    sai_object_id_t sampleSessionOid)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT *pPortEntry = NULL;

    saiStatus = xpSaiPortDbInfoGet(xpsIntf, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB");
        return saiStatus;
    }

    pPortEntry->egressSampleSessionOid = sampleSessionOid;

    return  saiStatus;
}

//Func: xpSaiGetPortAttrEgressSampleSessionOid

sai_status_t xpSaiGetPortAttrEgressSampleSessionOid(xpsInterfaceId_t xpsIntf,
                                                    sai_object_id_t *sampleSessionOid)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT *pPortEntry = NULL;

    saiStatus = xpSaiPortDbInfoGet(xpsIntf, &pPortEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB");
        return saiStatus;
    }

    *sampleSessionOid = pPortEntry->egressSampleSessionOid;

    return  saiStatus;
}

sai_status_t xpSaiIsPortInited(sai_object_id_t portOid)
{
    xpsInterfaceId_t    xpsIntf     = XPS_INTF_INVALID_ID;
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus    = XP_NO_ERR;
    xpsDevice_t         xpsDevId    = xpSaiObjIdSwitchGet(portOid);
    uint32_t             cpuPortNum  = 0;

    saiStatus = xpSaiConvertPortOid(portOid, &xpsIntf);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertPortOid() failed with error code: %d\n", saiStatus);
        return saiStatus;
    }

    xpStatus = xpsGlobalSwitchControlGetCpuPhysicalPortNum(xpsDevId, &cpuPortNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsGlobalSwitchControlGetCpuPhysicalPortNum call failed! Error %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (xpsIntf != (xpsInterfaceId_t)cpuPortNum)
    {
        if (xpSaiIsPortActive(xpsIntf) == 0)
        {
            XP_SAI_LOG_DBG("port #%u is uninitialized\n", xpsIntf);
            return SAI_STATUS_UNINITIALIZED;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPortAttribute
sai_status_t xpSaiSetPortAttribute(sai_object_id_t port_id,
                                   const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    retVal = xpSaiAttrCheck(1, attr, PORT_VALIDATION_ARRAY_SIZE, port_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    retVal = xpSaiIsPortInited(port_id);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("port #%u is uninitialized\n",
                       (uint32_t)xpSaiObjIdValueGet(port_id));
        return SAI_STATUS_UNINITIALIZED;
    }

    switch (attr->id)
    {
        case SAI_PORT_ATTR_SPEED:
            {
                retVal = xpSaiSetPortAttrSpeed(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_SPEED)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_AUTO_NEG_MODE:
            {
                retVal = xpSaiSetPortAutoNegMode(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_AUTO_NEG_MODE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID:
            {
                retVal = xpSaiSetPortAttrFloodStromControlPolicerId(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID:
            {
                retVal = xpSaiSetPortAttrBroadcastStromControlPolicerId(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID:
            {
                retVal = xpSaiSetPortAttrMulticastStromControlPolicerId(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE:
            {
                retVal = xpSaiSetPortAttrGlobalFlowControlMode(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_ADMIN_STATE:
            {
                retVal = xpSaiSetPortAttrAdminState(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_ADMIN_STATE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_MEDIA_TYPE:
            {
                retVal = xpSaiSetPortAttrMediaType(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_MEDIA_TYPE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_ADVERTISED_SPEED:
            {
                //retVal = xpSaiSetPortAttrAdvertisedSpeed(port_id, attr->value); WA to reduce number of port delete operations
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_ADVERTISED_SPEED)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_ADVERTISED_HALF_DUPLEX_SPEED:
            {
                retVal = xpSaiSetPortAttrAdvertisedHalfDuplexSpeed(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_ADVERTISED_HALF_DUPLEX_SPEED)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_ADVERTISED_AUTO_NEG_MODE:
            {
                retVal = xpSaiSetPortAttrAdvertisedAutoNegMode(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_ADVERTISED_AUTO_NEG_MODE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_ADVERTISED_FLOW_CONTROL_MODE:
            {
                retVal = xpSaiSetPortAttrAdvertisedFlowControlMode(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_ADVERTISED_FLOW_CONTROL_MODE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_ADVERTISED_ASYMMETRIC_PAUSE_MODE:
            {
                retVal = xpSaiSetPortAttrAdvertisedAsymmetricPauseMode(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_ADVERTISED_ASYMMETRIC_PAUSE_MODE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_PORT_VLAN_ID:
            {
                retVal = xpSaiSetPortAttrDefaultVlan(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_PORT_VLAN_ID)\n");
                    return retVal;
                }

                retVal = xpSaiSetPVIDtoDB(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_PORT_VLAN_ID) to DB\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY:
            {
                retVal = xpSaiSetPortAttrDefaultVlanPriority(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_DROP_UNTAGGED:
            {
                retVal = xpSaiSetPortAttrDropUntagged(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_DROP_UNTAGGED)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_DROP_TAGGED:
            {
                retVal = xpSaiSetPortAttrDropTagged(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_DROP_TAGGED)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE:
            {
                retVal = xpSaiSetPortAttrInternalLoopbackMode(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE)\n");
                    return retVal;
                }
                break;
            }
        /*
                case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE:
                    {
                        retVal = xpSaiSetPortAttrFdbLearningMode(port_id, attr->value);
                        if (SAI_STATUS_SUCCESS != retVal)
                        {
                            XP_SAI_LOG_ERR("Failed to set (SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE)\n");
                            return retVal;
                        }
                        break;
                    }
        */
        case SAI_PORT_ATTR_UPDATE_DSCP:
            {
                retVal = xpSaiSetPortAttrUpdateDscp(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_UPDATE_DSCP)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_MTU:
            {
                retVal = xpSaiSetPortAttrMtu(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_MTU)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_INGRESS_MIRROR_SESSION:
            {
                retVal = xpSaiSetPortAttrIngressMirrorSession(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_INGRESS_MIRROR_SESSION)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_EGRESS_MIRROR_SESSION:
            {
                retVal = xpSaiSetPortAttrEgressMirrorSession(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_EGRESS_MIRROR_SESSION)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE:
            {
                retVal = xpSaiSamplePacketIngressPortEnable(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE:
            {
                retVal = xpSaiSamplePacketEgressPortEnable(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_POLICER_ID:
            {
                retVal = xpSaiSetPortAttrPolicerId(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_POLICER_ID) |retVal = %d\n",
                                   retVal);
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_DEFAULT_TC:
            {
                retVal = xpSaiSetPortAttrQosDefaultTc(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_QOS_DEFAULT_TC) | retVal = %d\n",
                                   retVal);
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP:
            {
                retVal = xpSaiSetPortAttrQosDot1pToTcMap(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP) | retVal = %d\n",
                                   retVal);
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP:
            {
                retVal = xpSaiSetPortAttrQosDot1pToColorMap(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP) | retVal = %d\n",
                                   retVal);
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP:
            {
                retVal = xpSaiSetPortAttrQosDscpToTcMap(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP) | retVal = %d\n",
                                   retVal);
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP:
            {
                retVal = xpSaiSetPortAttrQosDscpToColorMap(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP) | retVal = %d\n",
                                   retVal);
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP:
            {
                /* TC to Queue map per port is not supported.
                 * Global TC to Queue map is set via switch attribute -
                 * SAI_SWITCH_ATTR_QOS_TC_TO_QUEUE_MAP */
                sai_object_id_t switch_id;
                xpsDevice_t     xpsDevId = xpSaiGetDevId();
                sai_attribute_t switchAttr;

                memset(&switchAttr, 0, sizeof(sai_attribute_t));
                switchAttr.id = SAI_SWITCH_ATTR_QOS_TC_TO_QUEUE_MAP;
                switchAttr.value = attr->value;

                xpSaiObjIdCreate(SAI_OBJECT_TYPE_SWITCH, xpsDevId, 0, &switch_id);

                retVal = xpSaiSetSwitchAttribute(switch_id, &switchAttr);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
            {
                retVal = xpSaiSetPortAttrQosTcAndColorToDot1pMap(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
            {
                retVal = xpSaiSetPortAttrQosTcAndColorToDscpMap(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP:
            {
                retVal = xpSaiSetPortAttrQosTcToPriorityGroupMap(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_PRIORITY_GROUP_MAP:
            {
                retVal = xpSaiSetPortAttrQosPfcPriorityToPriorityGroupMap(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_PRIORITY_GROUP_MAP)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP:
            {
                retVal = xpSaiSetPortAttrQosPfcPriorityToQueueMap(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID:
            {
                retVal = xpSaiSetPortAttrQosSchedulerProfileId(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST:
            {
                retVal = xpSaiSetPortAttrQosIngressBufferProfileList(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_EGRESS_BUFFER_PROFILE_LIST:
            {
                retVal = xpSaiSetPortAttrQosEgressBufferProfileList(port_id, attr->value,
                                                                    false);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_QOS_EGRESS_BUFFER_PROFILE_LIST)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL:
            {
                retVal = xpSaiSetPortAttrPriorityFlowControl(port_id, attr->value,
                                                             pfcVectorUpdateTypeCombined);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL)\n");
                    return retVal;
                }
                break;
            }

        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL_RX:
            {
                retVal = xpSaiSetPortAttrPriorityFlowControl(port_id, attr->value,
                                                             pfcVectorUpdateTypeRx);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL)\n");
                    return retVal;
                }
                break;
            }

        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL_TX:
            {
                retVal = xpSaiSetPortAttrPriorityFlowControl(port_id, attr->value,
                                                             pfcVectorUpdateTypeTx);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL)\n");
                    return retVal;
                }
                break;
            }

        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL_MODE:
            {
                retVal = xpSaiSetPortAttrPriorityFlowControlMode(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL_MODE)\n");
                    return retVal;
                }
                break;
            }

        case SAI_PORT_ATTR_INGRESS_ACL:
            {
                retVal = xpSaiSetPortAttrIngressAcl(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_INGRESS_ACL)\n");
                    return retVal;
                }
                break;

            }
        case SAI_PORT_ATTR_EGRESS_ACL:
            {
                retVal = xpSaiSetPortAttrEgressAcl(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_EGRESS_ACL)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_FEC_MODE:
            {
                retVal = xpSaiSetPortAttrFecMode(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_FEC_MODE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_ADVERTISED_FEC_MODE:
            {
                retVal = xpSaiSetPortAttrAdvertisedFecMode(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_ADVERTISED_FEC_MODE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_HW_PROFILE_ID:
            {
                retVal = xpSaiSetPortAttrHwProfileId(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_HW_PROFILE_ID)\n");
                    return retVal;
                }
                break;

            }
#if 0
        case SAI_PORT_ATTR_EEE_ENABLE:
            {
                retVal = xpSaiSetPortAttrEeeEnable(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_EEE_ENABLE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_EEE_IDLE_TIME:
            {
                retVal = xpSaiSetPortAttrEeeIdleTime(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_EEE_IDLE_TIME)\n");
                    return retVal;
                }
                break;
            }
        case SAI_PORT_ATTR_EEE_WAKE_TIME:
            {
                retVal = xpSaiSetPortAttrEeeWakeTime(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_EEE_WAKE_TIME)\n");
                    return retVal;
                }
                break;
            }
#endif

        case SAI_PORT_ATTR_FULL_DUPLEX_MODE:
            {
                retVal = xpSaiSetPortAttrFullDuplexMode(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_FULL_DUPLEX_MODE)\n");
                    return retVal;
                }
                break;
            }


        case SAI_PORT_ATTR_PKT_TX_ENABLE:
            {
                retVal = xpSaiSetPortAttrTxEnable(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_PKT_TX_ENABLE)\n");
                    return retVal;
                }
                break;
            }

        case SAI_PORT_ATTR_INTERFACE_TYPE:
            {
                retVal = xpSaiSetInterfaceType(port_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_PORT_ATTR_INTERFACE_TYPE)\n");
                    return retVal;
                }
                break;
            }

        default:
            {
                XP_SAI_LOG_ERR("Unsupported attribute %u\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetPortAttrUpdateDscp

sai_status_t xpSaiGetPortAttrUpdateDscp(sai_object_id_t port_oid,
                                        sai_attribute_value_t *value)
{
    XP_STATUS        xpStatus = XP_NO_ERR;
    GT_STATUS        rc       = GT_OK;
    xpsInterfaceId_t xpsIntf  = 0;
    xpsDevice_t      xpDevId  = 0;
    xpsPort_t        portNum  = 0;
    GT_BOOL          enable   = GT_FALSE;

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttrUpdateDscp\n");

    /* Get the port id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(port_oid, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_oid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_oid);

    /* Fetch device and port information for this particular port interface */
    if ((xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &xpDevId,
                                                    &portNum)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Fetch device and port information failed, interface id(%d)",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    rc = cpssHalPortEgressQosDscpMappingEnableGet(xpDevId, portNum, &enable);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Couldn't set egress DSCP mapping enable port: %d\n",
                       portNum);
        return  cpssStatus2SaiStatus(rc);
    }

    value->u32 = (enable) ? true : false;

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetPortMtu

sai_status_t xpSaiGetPortMtu(xpsDevice_t xpsDevId, xpsInterfaceId_t xpsIntf,
                             sai_attribute_value_t* value)
{
    XP_STATUS xpStatus  = XP_NO_ERR;

    xpStatus = xpsMtuGetInterfaceMtuSize(xpsDevId, xpsIntf, &value->u32);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get MTU on the port %u, error %d\n", xpsIntf,
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetBreakoutMode

sai_status_t xpSaiGetBreakoutMode(sai_object_id_t port_id,
                                  sai_attribute_value_t* value)
{
    XP_STATUS          xpStatus   = XP_NO_ERR;
    xpsInterfaceId_t   xpsIntf    = XPS_INTF_INVALID_ID;
    xpsDevice_t        devId;
    xpsPort_t          portNum;
    xpSaiPortMgrInfo_t info;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &devId, &portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get devId and portNum from interface Id (%u).",
                       xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpSaiPortInfoInit(&info);
    xpStatus = xpSaiPortCfgGet(devId, portNum, &info);
    if (xpStatus != XP_NO_ERR)
    {
        // ERROR message
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (info.breakOutMode == XPSAI_PORT_BREAKOUT_MODE_1X)
    {
        value->s32 = SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE;
    }
    else if (info.breakOutMode == XPSAI_PORT_BREAKOUT_MODE_2X)
    {
        value->s32 = SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE;
    }
    else if (info.breakOutMode == XPSAI_PORT_BREAKOUT_MODE_4X)
    {
        value->s32 = SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE;
    }
    else
    {
        // value->s32 = SAI_PORT_BREAKOUT_MODE_TYPE_8_LANE;
    }

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiGetPortAttribute

sai_status_t xpSaiGetPortAttribute(sai_object_id_t port_id,
                                   sai_attribute_t *attr, uint32_t attr_index)
{
    sai_status_t     saiRetVal      = SAI_STATUS_SUCCESS;
    XP_STATUS        xpStatus       = XP_NO_ERR;
    xpsDevice_t      xpsDevId       = xpSaiGetDevId();
    xpsInterfaceId_t xpsIntf        = (xpsInterfaceId_t) xpSaiObjIdValueGet(
                                          port_id);

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    switch (attr->id)
    {
        /* return inside switch is required. As current function design does not allows us to return
           result of operation for each attribute. So we need to push user to read items one by one
           to get operation status.
         */
        case SAI_PORT_ATTR_TYPE:
            {
                saiRetVal = xpSaiGetPortType(xpsDevId, xpsIntf, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_TYPE) on port: %d\n", xpsIntf);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_OPER_STATUS:
            {
                saiRetVal = xpSaiGetPortOperStatus(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_OPER_STATUS) on port: %lu\n",
                                   port_id);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_NUMBER_OF_INGRESS_PRIORITY_GROUPS:
            {
                saiRetVal = xpSaiGetPortAttrNumberOfIngressPriorityGroups(port_id,
                                                                          &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_NUMBER_OF_INGRESS_PRIORITY_GROUPS)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST:
            {
                saiRetVal = xpSaiGetPortAttrIngressPriorityGroupList(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_SPEED:
            {
                saiRetVal = xpSaiGetPortAttrSpeed(port_id, &attr->value, 0);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Could not get speed on the port %u.\n", xpsIntf);
                    return saiRetVal;
                }

                break;
            }
        case SAI_PORT_ATTR_OPER_SPEED:
            {
                saiRetVal = xpSaiGetPortAttrSpeed(port_id, &attr->value, 1);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Could not get speed on the port %u.\n", xpsIntf);
                    return saiRetVal;
                }

                break;
            }
        case SAI_PORT_ATTR_AUTO_NEG_MODE:
            {
                saiRetVal = xpSaiGetPortAttrAutoNegMode(port_id, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Could not get Auto Negotiation configuration on the port %lu.\n",
                                   port_id);
                    return saiRetVal;
                }

                break;
            }
        case SAI_PORT_ATTR_SUPPORTED_AUTO_NEG_MODE:
            {
                saiRetVal = xpSaiGetPortAttrSupportedAutoNegMode(port_id, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Could not get supported Auto Negotiation configuration on the port %lu.\n",
                                   port_id);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_SUPPORTED_FLOW_CONTROL_MODE:
            {
                saiRetVal = xpSaiGetPortAttrSupportedFlowControlMode(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_SUPPORTED_FLOW_CONTROL_MODE) on the port %lu\n",
                                   port_id);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID:
            {
                saiRetVal = xpSaiGetPortAttrFloodStromControlPolicerId(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID:
            {
                saiRetVal = xpSaiGetPortAttrBroadcastStromControlPolicerId(port_id,
                                                                           &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID:
            {
                saiRetVal = xpSaiGetPortAttrMulticastStromControlPolicerId(port_id,
                                                                           &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE:
            {
                saiRetVal = xpSaiGetPortAttrGlobalFlowControlMode(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE) on the port %lu\n",
                                   port_id);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_SUPPORTED_HALF_DUPLEX_SPEED:
            {
                saiRetVal = xpSaiGetPortAttrSupportedHalfDuplexSpeed(port_id, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Could not get supported half duplex speed on the port %lu.\n",
                                   port_id);
                    return saiRetVal;
                }

                break;
            }
        case SAI_PORT_ATTR_ADMIN_STATE:
            {
                saiRetVal = xpSaiGetPortAttrAdminState(port_id, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Could not get admin state of port %lu\n", port_id);
                    return saiRetVal;
                }

                break;
            }
        case SAI_PORT_ATTR_PORT_VLAN_ID:
            {
                saiRetVal = xpSaiGetPortAttrDefaultVlan(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_PORT_VLAN_ID)\n");
                    return saiRetVal;
                }

                break;
            }
        case SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY:
            {
                xpsPortConfig_t portConfig;
                memset(&portConfig, 0x00, sizeof(portConfig));

                xpStatus = (XP_STATUS)xpsPortGetConfig(xpsDevId, xpsIntf, &portConfig);
                if (XP_NO_ERR != xpStatus)
                {
                    XP_SAI_LOG_ERR("Could not get default PCP for the port %u.\n", xpsIntf);
                    return  xpsStatus2SaiStatus(xpStatus);
                }

                attr->value.u8 = portConfig.portDefaultPCP;
                break;
            }
        case SAI_PORT_ATTR_DROP_UNTAGGED:
            {
                saiRetVal = xpSaiGetPortAttrDropUntagged(port_id, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_DROP_UNTAGGED)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_DROP_TAGGED:
            {
                saiRetVal = xpSaiGetPortAttrDropTagged(port_id, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_DROP_TAGGED)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE:
            {
                saiRetVal = xpSaiGetPortAttrInternalLoopbackMode(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE)\n");
                    return saiRetVal;
                }
                break;
            }
        /*        case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE:
                {
                    saiRetVal = xpSaiGetPortAttrFdbLearningMode(port_id, &attr->value, attr_index);
                    if(SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE)\n");
                        return saiRetVal;
                    }
                    break;
                }
        */
        case SAI_PORT_ATTR_UPDATE_DSCP:
            {
                saiRetVal = xpSaiGetPortAttrUpdateDscp(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_UPDATE_DSCP)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_MTU:
            {
                saiRetVal = xpSaiGetPortMtu(xpsDevId, xpsIntf, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Could not get port %u MTU.\n", xpsIntf);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_HW_LANE_LIST:
            {
                saiRetVal = xpSaiGetPortAttrHwLaneList(port_id, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_HW_LANE_LIST)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE_TYPE:
            {
                saiRetVal = xpSaiGetPortAttrSupportedBreakoutModeType(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE_TYPE) | saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE:
            {
                saiRetVal = xpSaiGetBreakoutMode(port_id, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Could not get (SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE) mode for port %u.\n",
                                   xpsIntf);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_MEDIA_TYPE:
            {
                saiRetVal = xpSaiGetPortAttrMediaType(xpsIntf, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Could not get a media type for port %u.\n", xpsIntf);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_INGRESS_MIRROR_SESSION:
            {
                saiRetVal = xpSaiGetPortAttrIngressMirrorSession(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_INGRESS_MIRROR_SESSION)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_EGRESS_MIRROR_SESSION:
            {
                saiRetVal = xpSaiGetPortAttrEgressMirrorSession(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_EGRESS_MIRROR_SESSION)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE:
            {
                saiRetVal = xpSaiGetPortAttrIngressSampleSessionOid(port_id, &attr->value.oid);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE:
            {
                saiRetVal = xpSaiGetPortAttrEgressSampleSessionOid(port_id, &attr->value.oid);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_DEFAULT_TC:
            {
                saiRetVal = xpSaiGetPortAttrQosDefaultTc(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_DEFAULT_TC) | saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP:
            {
                saiRetVal = xpSaiGetPortAttrQosDot1pToTcMap(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP) |saiRetVal: %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP:
            {
                saiRetVal = xpSaiGetPortAttrQosDot1pToColorMap(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP) |saiRetVal: %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP:
            {
                saiRetVal = xpSaiGetPortAttrQosDscpToTcMap(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP) |saiRetVal: %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP:
            {
                saiRetVal = xpSaiGetPortAttrQosDscpToColorMap(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP) |saiRetVal: %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP:
            {
                saiRetVal = xpSaiGetPortAttrQosTcToQueueMap(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
            {
                saiRetVal = xpSaiGetPortAttrQosTcAndColorToDot1pMap(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
            {
                saiRetVal = xpSaiGetPortAttrQosTcAndColorToDscpMap(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP:
            {
                saiRetVal = xpSaiGetPortAttrQosTcToPriorityGroupMap(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_PRIORITY_GROUP_MAP:
            {
                saiRetVal = xpSaiGetPortAttrQosPfcPriorityToPriorityGroupMap(port_id,
                                                                             &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_PRIORITY_GROUP_MAP) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP:
            {
                saiRetVal = xpSaiGetPortAttrQosPfcPriorityToQueueMap(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES:
            {
                saiRetVal = xpSaiGetPortAttrQosNumberOfQueues(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES) | saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_QUEUE_LIST:
            {
                saiRetVal = xpSaiGetPortAttrQosQueueList(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_QUEUE_LIST) | saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS:
            {
                saiRetVal = xpSaiGetPortAttrQosNumberOfSchedulerGroups(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS) | saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST:
            {
                saiRetVal = xpSaiGetPortAttrQosSchedulerGroupList(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST) | saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
#if 0 // SAI 1.2
        case SAI_PORT_POOL_ATTR_QOS_WRED_PROFILE_ID:
            {
                saiRetVal = xpSaiGetPortAttrQosWredProfileId(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_POOL_ATTR_QOS_WRED_PROFILE_ID) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
#endif
        case SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID:
            {
                saiRetVal = xpSaiGetPortAttrQosSchedulerProfileId(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID) | saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST:
            {
                saiRetVal = xpSaiGetPortAttrQosIngressBufferProfileList(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST), saiRetVal:%d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL:
            {
                saiRetVal = xpSaiGetPortAttrPriorityFlowControl(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL_RX:
            {
                saiRetVal = xpSaiPortPriorityFlowControlGet(port_id, &attr->value.u8,
                                                            pfcVectorUpdateTypeRx);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL_RX)\n");
                    return saiRetVal;
                }
                break;
            }

        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL_TX:
            {
                saiRetVal = xpSaiPortPriorityFlowControlGet(port_id, &attr->value.u8,
                                                            pfcVectorUpdateTypeTx);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL_TX)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_POLICER_ID:
            {
                saiRetVal = xpSaiGetPortAttrPolicerId(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_POLICER_ID) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_SUPPORTED_SPEED:
            {
                saiRetVal = xpSaiGetPortAttrSupportedSpeed(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_SUPPORTED_SPEED) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_REMOTE_ADVERTISED_SPEED:
            {
                saiRetVal = xpSaiGetPortAttrRemoteAdvertisedSpeed(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_REMOTE_ADVERTISED_SPEED) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_REMOTE_ADVERTISED_HALF_DUPLEX_SPEED:
            {
                saiRetVal = xpSaiGetPortAttrRemoteAdvertisedHalfDuplexSpeed(port_id,
                                                                            &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_REMOTE_ADVERTISED_HALF_DUPLEX_SPEED) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_REMOTE_ADVERTISED_AUTO_NEG_MODE:
            {
                saiRetVal = xpSaiGetPortAttrRemoteAdvertisedAutoNegMode(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_REMOTE_ADVERTISED_AUTO_NEG_MODE) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_REMOTE_ADVERTISED_FLOW_CONTROL_MODE:
            {
                saiRetVal = xpSaiGetPortAttrRemoteAdvertisedFlowControlMode(port_id,
                                                                            &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_REMOTE_ADVERTISED_FLOW_CONTROL) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_REMOTE_ADVERTISED_ASYMMETRIC_PAUSE_MODE:
            {
                saiRetVal = xpSaiGetPortAttrRemoteAdvertisedAsymmetricPauseMode(port_id,
                                                                                &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_REMOTE_ADVERTISED_ASYMMETRIC_PAUSE_MODE) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_ADVERTISED_SPEED:
            {
                saiRetVal = xpSaiGetPortAttrAdvertisedSpeed(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_ADVERTISED_SPEED) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_ADVERTISED_HALF_DUPLEX_SPEED:
            {
                saiRetVal = xpSaiGetPortAttrAdvertisedHalfDuplexSpeed(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_ADVERTISED_HALF_DUPLEX_SPEED) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_ADVERTISED_AUTO_NEG_MODE:
            {
                saiRetVal = xpSaiGetPortAttrAdvertisedAutoNegMode(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_ADVERTISED_AUTO_NEG_MODE) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_ADVERTISED_FLOW_CONTROL_MODE:
            {
                saiRetVal = xpSaiGetPortAttrAdvertisedFlowControlMode(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_ADVERTISED_FLOW_CONTROL_MODE) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_ADVERTISED_ASYMMETRIC_PAUSE_MODE:
            {
                saiRetVal = xpSaiGetPortAttrAdvertisedAsymmetricPauseMode(port_id,
                                                                          &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_ADVERTISED_ASYMMETRIC_PAUSE_MODE) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_INGRESS_ACL:
            {
                saiRetVal = xpSaiGetPortAttrIngressAcl(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_INGRESS_ACL) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;

            }
        case SAI_PORT_ATTR_EGRESS_ACL:
            {
                saiRetVal = xpSaiGetPortAttrEgressAcl(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_EGRESS_ACL) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_FEC_MODE:
            {
                saiRetVal = xpSaiGetPortAttrFecMode(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_FEC_MODE) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_SUPPORTED_FEC_MODE:
            {
                saiRetVal = xpSaiGetPortAttrSupportedFecMode(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_SUPPORTED_FEC_MODE) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_ADVERTISED_FEC_MODE:
            {
                saiRetVal = xpSaiGetPortAttrAdvertisedFecMode(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_ADVERTISED_FEC_MODE) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_REMOTE_ADVERTISED_FEC_MODE:
            {
                saiRetVal = xpSaiGetPortAttrRemoteAdvertisedFecMode(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_REMOTE_ADVERTISED_FEC_MODE) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_CUSTOM_RANGE_START:
            {
                saiRetVal = xpSaiGetPortAttrMacNum(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_CUSTOM_RANGE_START) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
#if 0
        case SAI_PORT_ATTR_EEE_ENABLE:
            {
                saiRetVal = xpSaiGetPortAttrEeeEnable(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_EEE_ENABLE) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_EEE_IDLE_TIME:
            {
                saiRetVal = xpSaiGetPortAttrEeeIdleTime(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_EEE_IDLE_TIME) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_EEE_WAKE_TIME:
            {
                saiRetVal = xpSaiGetPortAttrEeeWakeTime(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_EEE_WAKE_TIME) |saiRetVal = %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
#endif

        case SAI_PORT_ATTR_FULL_DUPLEX_MODE:
            {
                saiRetVal = xpSaiGetPortAttrFullDuplexMode(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_FULL_DUPLEX_MODE)\n");
                    return saiRetVal;
                }
                break;
            }


        case SAI_PORT_ATTR_PKT_TX_ENABLE:
            {
                saiRetVal = xpSaiGetPortAttrTxEnable(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_PKT_TX_ENABLE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_PORT_ATTR_QOS_MAXIMUM_HEADROOM_SIZE:
            {
                attr->value.u32 = 0;
                break;
            }
        case SAI_PORT_ATTR_INTERFACE_TYPE:
            {
                saiRetVal = xpSaiGetInterfaceType(port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_PORT_ATTR_INTERFACE_TYPE)\n");
                    return saiRetVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Invalid parameter received %d.\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortAttributes
static sai_status_t xpSaiGetPortAttributes(sai_object_id_t port_id,
                                           uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               PORT_VALIDATION_ARRAY_SIZE, port_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    XP_SAI_LOG_DBG("Calling xpSaiGetPortAttribute port_id = %u, attr_count = %d \n",
                   (uint32_t)xpSaiObjIdValueGet(port_id), attr_count);

    saiRetVal = xpSaiIsPortInited(port_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("port #%u is uninitialized\n",
                       (uint32_t)xpSaiObjIdValueGet(port_id));
        return SAI_STATUS_UNINITIALIZED;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetPortAttribute(port_id, &attr_list[count], count);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("failed for port ID %lu\n", port_id);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBulkGetPortAttributes

sai_status_t xpSaiBulkGetPortAttributes(sai_object_id_t id,
                                        uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountPortAttribute(&maxcount);
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

    saiRetVal = xpSaiIsPortInited(id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("port #%u is uninitialized\n", (uint32_t)xpSaiObjIdValueGet(id));
        return SAI_STATUS_UNINITIALIZED;
    }

    for (uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_PORT_ATTR_START + count;
        attr_list[idx].value.u32 = 0;

        saiRetVal = xpSaiGetPortAttribute(id, &attr_list[idx], count);
        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiClearPortStatsDB

static sai_status_t xpSaiClearPortStatsDB(uint32_t number_of_counters,
                                          const sai_stat_id_t *counter_ids,
                                          xpSaiPortStatisticDbEntryT *portStatistics)
{
    if ((counter_ids == NULL) || (portStatistics == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t indx=0; indx < number_of_counters; indx++)
    {
        switch (counter_ids[indx])
        {
            case SAI_PORT_STAT_IF_IN_OCTETS :
                {
                    portStatistics->rxOctets = 0;
                    break;
                }
            case SAI_PORT_STAT_IF_IN_UCAST_PKTS :
                {
                    portStatistics->rxUnicast = 0;
                    break;
                }
            case SAI_PORT_STAT_IF_IN_NON_UCAST_PKTS :
                {
                    portStatistics->rxNonUnicast = 0;
                    break;
                }
            case SAI_PORT_STAT_IF_IN_ERRORS :
            case SAI_PORT_STAT_IF_IN_DISCARDS :
                {
                    portStatistics->rxErrOrDiscard = 0;
                    break;
                }
            case SAI_PORT_STAT_IF_IN_BROADCAST_PKTS :
                {
                    portStatistics->rxBroadcast = 0;
                    break;
                }
            case SAI_PORT_STAT_IF_IN_MULTICAST_PKTS :
                {
                    portStatistics->rxMulticast = 0;
                    break;
                }
            case SAI_PORT_STAT_IF_IN_UNKNOWN_PROTOS :
                {
                    portStatistics->rxUnknownProtocol = 0;
                    break;
                }
            case SAI_PORT_STAT_IF_IN_VLAN_DISCARDS :
            case SAI_PORT_STAT_IF_OUT_QLEN :
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                    break;
                }
            case SAI_PORT_STAT_IF_OUT_OCTETS :
                {
                    portStatistics->txOctets = 0;
                    break;
                }
            case SAI_PORT_STAT_IF_OUT_UCAST_PKTS :
                {
                    portStatistics->txUnicast = 0;
                    break;
                }
            case SAI_PORT_STAT_IF_OUT_NON_UCAST_PKTS :
                {
                    portStatistics->txNonUnicast = 0;
                    break;
                }
            case SAI_PORT_STAT_IF_OUT_ERRORS :
            case SAI_PORT_STAT_IF_OUT_DISCARDS :
                {
                    portStatistics->txErrOrDiscard = 0;
                    break;
                }
            case SAI_PORT_STAT_IF_OUT_BROADCAST_PKTS :
                {
                    portStatistics->txBroadcast = 0;
                    break;
                }
            case SAI_PORT_STAT_IF_OUT_MULTICAST_PKTS :
                {
                    portStatistics->txMulticast = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_FRAGMENTS :
                {
                    portStatistics->fragments = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_RX_OVERSIZE_PKTS :
                {
                    portStatistics->rxOversize = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_TX_NO_ERRORS :
                {
                    portStatistics->txNoError = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_RX_NO_ERRORS :
                {
                    portStatistics->rxNoError = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_CRC_ALIGN_ERRORS :
                {
                    portStatistics->crcAlignErr = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_OVERSIZE_PKTS :
                {
                    portStatistics->statOversize = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_UNDERSIZE_PKTS :
                {
                    portStatistics->statUnderSize = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_JABBERS :
                {
                    portStatistics->statJabbers = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_DROP_EVENTS :
            case SAI_PORT_STAT_ETHER_STATS_MULTICAST_PKTS :
            case SAI_PORT_STAT_ETHER_STATS_BROADCAST_PKTS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_64_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_65_TO_127_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_128_TO_255_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_256_TO_511_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_512_TO_1023_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_1024_TO_1518_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_1519_TO_2047_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_2048_TO_4095_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_4096_TO_9216_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_9217_TO_16383_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS :
            case SAI_PORT_STAT_ETHER_STATS_COLLISIONS :
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                    break;
                }

            case SAI_PORT_STAT_IP_IN_RECEIVES :
                {
                    portStatistics->rxV4Pkts = 0;
                    break;
                }
            case SAI_PORT_STAT_IP_IN_OCTETS :
                {
                    portStatistics->rxV4Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_IP_OUT_OCTETS :
                {
                    portStatistics->txV4Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_IPV6_IN_RECEIVES :
                {
                    portStatistics->rxV6Pkts = 0;
                    break;
                }
            case SAI_PORT_STAT_IPV6_IN_OCTETS :
                {
                    portStatistics->rxV6Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_IPV6_OUT_OCTETS :
                {
                    portStatistics->txV6Octets = 0;
                    break;
                }

            case SAI_PORT_STAT_IP_IN_UCAST_PKTS :
            case SAI_PORT_STAT_IP_IN_NON_UCAST_PKTS :
            case SAI_PORT_STAT_IP_IN_DISCARDS :

            //IPv4 Tx counters
            case SAI_PORT_STAT_IP_OUT_UCAST_PKTS :
            case SAI_PORT_STAT_IP_OUT_NON_UCAST_PKTS :
            case SAI_PORT_STAT_IP_OUT_DISCARDS :

            //IPv6 Rx counters
            case SAI_PORT_STAT_IPV6_IN_UCAST_PKTS :
            case SAI_PORT_STAT_IPV6_IN_NON_UCAST_PKTS :
            case SAI_PORT_STAT_IPV6_IN_MCAST_PKTS :
            case SAI_PORT_STAT_IPV6_IN_DISCARDS :

            //IPv6 Tx counters
            case SAI_PORT_STAT_IPV6_OUT_UCAST_PKTS :
            case SAI_PORT_STAT_IPV6_OUT_NON_UCAST_PKTS :
            case SAI_PORT_STAT_IPV6_OUT_MCAST_PKTS :
            case SAI_PORT_STAT_IPV6_OUT_DISCARDS :

            // WRED counters
            case SAI_PORT_STAT_GREEN_WRED_DROPPED_PACKETS:
            case SAI_PORT_STAT_GREEN_WRED_DROPPED_BYTES:
            case SAI_PORT_STAT_YELLOW_WRED_DROPPED_PACKETS:
            case SAI_PORT_STAT_YELLOW_WRED_DROPPED_BYTES:
            case SAI_PORT_STAT_RED_WRED_DROPPED_PACKETS:
            case SAI_PORT_STAT_RED_WRED_DROPPED_BYTES:
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                    break;
                }
            case SAI_PORT_STAT_WRED_DROPPED_PACKETS:
                {
                    portStatistics->wredDropPkts = 0;
                    break;
                }
            case SAI_PORT_STAT_WRED_DROPPED_BYTES:
                {
                    portStatistics->wredDropBytes = 0;
                    break;
                }

            // Get/set packets marked by ECN count
            case SAI_PORT_STAT_ECN_MARKED_PACKETS:
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                    break;
                }

            // Rx counters based on packet size
            case SAI_PORT_STAT_ETHER_IN_PKTS_64_OCTETS:
                {
                    portStatistics->rx64Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_65_TO_127_OCTETS:
                {
                    portStatistics->rx65To127Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_128_TO_255_OCTETS:
                {
                    portStatistics->rx128To255Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_256_TO_511_OCTETS:
                {
                    portStatistics->rx256To511Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_512_TO_1023_OCTETS:
                {
                    portStatistics->rx512To1023Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_1024_TO_1518_OCTETS:
                {
                    portStatistics->rx1024To1518Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_1519_TO_2047_OCTETS:
                {
                    portStatistics->rx1519To2047Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_2048_TO_4095_OCTETS:
                {
                    portStatistics->rx2048To4095Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_4096_TO_9216_OCTETS:
                {
                    portStatistics->rx4096To9216Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_9217_TO_16383_OCTETS:
                {
                    portStatistics->rx9217To16383Octets = 0;
                    break;
                }


            // Tx counters based on packet size
            case SAI_PORT_STAT_ETHER_OUT_PKTS_64_OCTETS:
                {
                    portStatistics->tx64Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_65_TO_127_OCTETS:
                {
                    portStatistics->tx65To127Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_128_TO_255_OCTETS:
                {
                    portStatistics->tx128To255Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_256_TO_511_OCTETS:
                {
                    portStatistics->tx256To511Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_512_TO_1023_OCTETS:
                {
                    portStatistics->tx512To1023Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_1024_TO_1518_OCTETS:
                {
                    portStatistics->tx1024To1518Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_1519_TO_2047_OCTETS:
                {
                    portStatistics->tx1519To2047Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_2048_TO_4095_OCTETS:
                {
                    portStatistics->tx2048To4095Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_4096_TO_9216_OCTETS:
                {
                    portStatistics->tx4096To9216Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_9217_TO_16383_OCTETS:
                {
                    portStatistics->tx9217To16383Octets = 0;
                    break;
                }
            case SAI_PORT_STAT_ETHER_TX_OVERSIZE_PKTS :
                {
                    portStatistics->TxOversize = 0;
                    break;
                }
            /** get current port occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_IN_CURR_OCCUPANCY_BYTES:
            /** get out port current occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_OUT_CURR_OCCUPANCY_BYTES:
            /** get watermark port occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_IN_WATERMARK_BYTES:
            /** get current port shared occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_IN_SHARED_CURR_OCCUPANCY_BYTES:
            /** get watermark port shared occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_IN_SHARED_WATERMARK_BYTES:
            /** get out port watermark occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_OUT_WATERMARK_BYTES:
            /** get out port current shared occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_OUT_SHARED_CURR_OCCUPANCY_BYTES:
            /** get out port watermark shared occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_OUT_SHARED_WATERMARK_BYTES:
            /** get in port packet drops due to buffers [uint64_t] */
            case SAI_PORT_STAT_IN_DROPPED_PKTS:
            /** get out port packet drops due to buffers [uint64_t] */
            case SAI_PORT_STAT_OUT_DROPPED_PKTS:
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                    break;
                }
            /** get the number of pause frames received on the port [uint64_t] */
            case SAI_PORT_STAT_PAUSE_RX_PKTS:
                {
                    portStatistics->rxPause = 0;
                    break;
                }
            /** get the number of pause frames transmitted on the port [uint64_t] */
            case SAI_PORT_STAT_PAUSE_TX_PKTS:
                {
                    portStatistics->txPause = 0;
                    break;
                }
            // PFC Packet Counters for RX and TX per PFC priority
            case SAI_PORT_STAT_PFC_0_RX_PKTS:
                {
                    portStatistics->rxPfc0 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_0_TX_PKTS:
                {
                    portStatistics->txPfc0 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_1_RX_PKTS:
                {
                    portStatistics->rxPfc1 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_1_TX_PKTS:
                {
                    portStatistics->txPfc1 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_2_RX_PKTS:
                {
                    portStatistics->rxPfc2 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_2_TX_PKTS:
                {
                    portStatistics->txPfc2 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_3_RX_PKTS:
                {
                    portStatistics->rxPfc3 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_3_TX_PKTS:
                {
                    portStatistics->txPfc3 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_4_RX_PKTS:
                {
                    portStatistics->rxPfc4 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_4_TX_PKTS:
                {
                    portStatistics->txPfc4 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_5_RX_PKTS:
                {
                    portStatistics->rxPfc5 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_5_TX_PKTS:
                {
                    portStatistics->txPfc5 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_6_RX_PKTS:
                {
                    portStatistics->rxPfc6 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_6_TX_PKTS:
                {
                    portStatistics->txPfc6 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_7_RX_PKTS:
                {
                    portStatistics->rxPfc7 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_7_TX_PKTS:
                {
                    portStatistics->txPfc7 = 0;
                    break;
                }
            case SAI_PORT_STAT_PFC_0_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_0_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_1_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_1_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_2_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_2_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_3_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_3_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_4_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_4_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_5_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_5_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_6_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_6_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_7_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_7_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_0_ON2OFF_RX_PKTS:
            case SAI_PORT_STAT_PFC_1_ON2OFF_RX_PKTS:
            case SAI_PORT_STAT_PFC_2_ON2OFF_RX_PKTS:
            case SAI_PORT_STAT_PFC_3_ON2OFF_RX_PKTS:
            case SAI_PORT_STAT_PFC_4_ON2OFF_RX_PKTS:
            case SAI_PORT_STAT_PFC_5_ON2OFF_RX_PKTS:
            case SAI_PORT_STAT_PFC_6_ON2OFF_RX_PKTS:
            case SAI_PORT_STAT_PFC_7_ON2OFF_RX_PKTS:
                {
                    XP_SAI_LOG_DBG("Attribute %d is not supported\n", counter_ids[indx]);
                }
            default:
                {
                    XP_SAI_LOG_WARNING("Undefined attribute %d\n", counter_ids[indx]);
                }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortStatsInReadClearMode

static sai_status_t xpSaiGetPortStatsInReadClearMode(sai_object_id_t port_id,
                                                     uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                                     uint64_t *counters, bool is_read_clear)
{
    XP_STATUS     xpStatus  = XP_NO_ERR;
    sai_status_t  saiStatus = SAI_STATUS_SUCCESS;
    xpsDevice_t   xpsDevId  = xpSaiGetDevId();
    xpSaiPortStatisticDbEntryT *portStatistics = NULL;
    xpsInterfaceId_t xpsIntf = 0;
    XP_DEV_TYPE_T devType;
    if (gResetInProgress)
    {
        return SAI_STATUS_SUCCESS;
    }


    if ((number_of_counters < 1) || (NULL == counter_ids) ||
        (NULL == counters)  /*|| (port validation check should be added)*/)
    {
        XP_SAI_LOG_ERR("%s invalid parameters received.", __FUNCNAME__);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    XP_SAI_LOG_DBG("Calling %s with port_id = %u, number_of_counters %d.\n",
                   __FUNCNAME__, xpsIntf, number_of_counters);

    devType = xpSaiSwitchDevTypeGet();

    saiStatus = xpSaiPortStatisticsStoreDB(xpsDevId, xpsIntf);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not restore the statistics for the port %u.\n", xpsIntf);
        return  saiStatus;
    }

    // get the port statistics from the sai state database
    xpStatus = xpSaiGetPortStatisticInfo(xpsIntf, &portStatistics);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get the port statistic data, xpStatus: %d\n",
                       xpStatus);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    for (uint32_t indx=0; indx < number_of_counters; indx++)
    {
        switch (counter_ids[indx])
        {
            case SAI_PORT_STAT_IF_IN_OCTETS :
                {
                    counters[indx] = portStatistics->rxOctets;
                    break;
                }
            case SAI_PORT_STAT_IF_IN_UCAST_PKTS :
                {
                    counters[indx] = portStatistics->rxUnicast;
                    break;
                }
            case SAI_PORT_STAT_IF_IN_NON_UCAST_PKTS :
                {
                    counters[indx] = portStatistics->rxNonUnicast;
                    break;
                }
            case SAI_PORT_STAT_IF_IN_ERRORS :
            case SAI_PORT_STAT_IF_IN_DISCARDS :
                {
                    counters[indx] = portStatistics->rxErrOrDiscard;
                    break;
                }
            case SAI_PORT_STAT_IF_IN_BROADCAST_PKTS :
                {
                    counters[indx] = portStatistics->rxBroadcast;
                    break;
                }
            case SAI_PORT_STAT_IF_IN_MULTICAST_PKTS :
                {
                    counters[indx] = portStatistics->rxMulticast;
                    break;
                }
            case SAI_PORT_STAT_IF_IN_UNKNOWN_PROTOS :
                {
                    counters[indx] = portStatistics->rxUnknownProtocol;
                    break;
                }
            case SAI_PORT_STAT_IF_IN_VLAN_DISCARDS :
            case SAI_PORT_STAT_IF_OUT_QLEN :
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                    //saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                    //SONIC_WA
                    saiStatus = SAI_STATUS_SUCCESS;
                    break;
                }
            case SAI_PORT_STAT_IF_OUT_OCTETS :
                {
                    counters[indx] = portStatistics->txOctets;
                    break;
                }
            case SAI_PORT_STAT_IF_OUT_UCAST_PKTS :
                {
                    counters[indx] = portStatistics->txUnicast;
                    break;
                }
            case SAI_PORT_STAT_IF_OUT_NON_UCAST_PKTS :
                {
                    counters[indx] = portStatistics->txNonUnicast;
                    break;
                }
            case SAI_PORT_STAT_IF_OUT_ERRORS :
            case SAI_PORT_STAT_IF_OUT_DISCARDS :
                {
                    counters[indx] = portStatistics->txErrOrDiscard;
                    break;
                }
            case SAI_PORT_STAT_IF_OUT_BROADCAST_PKTS :
                {
                    counters[indx] = portStatistics->txBroadcast;
                    break;
                }
            case SAI_PORT_STAT_IF_OUT_MULTICAST_PKTS :
                {
                    counters[indx] = portStatistics->txMulticast;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_FRAGMENTS :
                {
                    counters[indx] = portStatistics->fragments;
                    break;
                }
            case SAI_PORT_STAT_ETHER_RX_OVERSIZE_PKTS :
                {
                    counters[indx] = portStatistics->rxOversize;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_TX_NO_ERRORS :
                {
                    counters[indx] = portStatistics->txNoError;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_RX_NO_ERRORS :
                {
                    counters[indx] = portStatistics->rxNoError;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_CRC_ALIGN_ERRORS :
                {
                    counters[indx] = portStatistics->crcAlignErr;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_OVERSIZE_PKTS :
                {
                    counters[indx] = portStatistics->statOversize;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_UNDERSIZE_PKTS :
                {
                    counters[indx] = portStatistics->statUnderSize;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_JABBERS :
                {
                    counters[indx] = portStatistics->statJabbers;
                    break;
                }
            case SAI_PORT_STAT_ETHER_STATS_DROP_EVENTS :
            case SAI_PORT_STAT_ETHER_STATS_MULTICAST_PKTS :
            case SAI_PORT_STAT_ETHER_STATS_BROADCAST_PKTS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_64_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_65_TO_127_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_128_TO_255_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_256_TO_511_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_512_TO_1023_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_1024_TO_1518_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_1519_TO_2047_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_2048_TO_4095_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_4096_TO_9216_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS_9217_TO_16383_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_OCTETS :
            case SAI_PORT_STAT_ETHER_STATS_PKTS :
            case SAI_PORT_STAT_ETHER_STATS_COLLISIONS :
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                    saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                    break;
                }

            case SAI_PORT_STAT_IP_IN_RECEIVES :
                {
                    counters[indx] = portStatistics->rxV4Pkts;
                    break;
                }
            case SAI_PORT_STAT_IP_IN_OCTETS :
                {
                    counters[indx] = portStatistics->rxV4Octets;
                    break;
                }
            case SAI_PORT_STAT_IP_OUT_OCTETS :
                {
                    counters[indx] = portStatistics->txV4Octets;
                    break;
                }
            case SAI_PORT_STAT_IPV6_IN_RECEIVES :
                {
                    counters[indx] = portStatistics->rxV6Pkts;
                    break;
                }
            case SAI_PORT_STAT_IPV6_IN_OCTETS :
                {
                    counters[indx] = portStatistics->rxV6Octets;
                    break;
                }
            case SAI_PORT_STAT_IPV6_OUT_OCTETS :
                {
                    counters[indx] = portStatistics->txV6Octets;
                    break;
                }

            //IPv4 Rx counters
            case SAI_PORT_STAT_IP_IN_UCAST_PKTS :
            case SAI_PORT_STAT_IP_IN_NON_UCAST_PKTS :
            case SAI_PORT_STAT_IP_IN_DISCARDS :

            //IPv4 Tx counters
            case SAI_PORT_STAT_IP_OUT_UCAST_PKTS :
            case SAI_PORT_STAT_IP_OUT_NON_UCAST_PKTS :
            case SAI_PORT_STAT_IP_OUT_DISCARDS :

            //IPv6 Rx counters
            case SAI_PORT_STAT_IPV6_IN_UCAST_PKTS :
            case SAI_PORT_STAT_IPV6_IN_NON_UCAST_PKTS :
            case SAI_PORT_STAT_IPV6_IN_MCAST_PKTS :
            case SAI_PORT_STAT_IPV6_IN_DISCARDS :

            //IPv6 Tx counters
            case SAI_PORT_STAT_IPV6_OUT_UCAST_PKTS :
            case SAI_PORT_STAT_IPV6_OUT_NON_UCAST_PKTS :
            case SAI_PORT_STAT_IPV6_OUT_MCAST_PKTS :
            case SAI_PORT_STAT_IPV6_OUT_DISCARDS :

            // WRED counters
            case SAI_PORT_STAT_GREEN_WRED_DROPPED_PACKETS:
            case SAI_PORT_STAT_GREEN_WRED_DROPPED_BYTES:
            case SAI_PORT_STAT_YELLOW_WRED_DROPPED_PACKETS:
            case SAI_PORT_STAT_YELLOW_WRED_DROPPED_BYTES:
            case SAI_PORT_STAT_RED_WRED_DROPPED_PACKETS:
            case SAI_PORT_STAT_RED_WRED_DROPPED_BYTES:
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                    saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                    break;
                }
            case SAI_PORT_STAT_WRED_DROPPED_PACKETS:
                {
                    counters[indx] = portStatistics->wredDropPkts;
                    break;
                }
            case SAI_PORT_STAT_WRED_DROPPED_BYTES:
                {
                    counters[indx] = portStatistics->wredDropBytes;
                    break;
                }

            // Get/set packets marked by ECN count
            case SAI_PORT_STAT_ECN_MARKED_PACKETS:
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                    saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                    break;
                }

            // Rx counters based on packet size
            case SAI_PORT_STAT_ETHER_IN_PKTS_64_OCTETS:
                {
                    counters[indx] = portStatistics->rx64Octets;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_65_TO_127_OCTETS:
                {
                    counters[indx] = portStatistics->rx65To127Octets;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_128_TO_255_OCTETS:
                {
                    counters[indx] = portStatistics->rx128To255Octets;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_256_TO_511_OCTETS:
                {
                    counters[indx] = portStatistics->rx256To511Octets;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_512_TO_1023_OCTETS:
                {
                    counters[indx] = portStatistics->rx512To1023Octets;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_1024_TO_1518_OCTETS:
                {
                    counters[indx] = portStatistics->rx1024To1518Octets;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_1519_TO_2047_OCTETS:
                {
                    if (IS_DEVICE_XP70_XP60(devType) || IS_DEVICE_FALCON(devType))
                    {
                        counters[indx] = portStatistics->rx1519To2047Octets;
                        break;
                    }
                    else
                    {
                        XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                        saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                        break;
                    }
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_2048_TO_4095_OCTETS:
                {
                    if (IS_DEVICE_XP70_XP60(devType) || IS_DEVICE_FALCON(devType))
                    {
                        counters[indx] = portStatistics->rx2048To4095Octets;
                        break;
                    }
                    else
                    {
                        XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                        saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                        break;
                    }
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_4096_TO_9216_OCTETS:
                {
                    counters[indx] = portStatistics->rx4096To9216Octets;
                    break;
                }
            case SAI_PORT_STAT_ETHER_IN_PKTS_9217_TO_16383_OCTETS:
                {
                    counters[indx] = portStatistics->rx9217To16383Octets;
                    break;
                }


            // Tx counters based on packet size
            case SAI_PORT_STAT_ETHER_OUT_PKTS_64_OCTETS:
                {
                    if (IS_DEVICE_XP70_XP60(devType) || IS_DEVICE_FALCON(devType))
                    {
                        counters[indx] = portStatistics->tx64Octets;
                        break;
                    }
                    else
                    {
                        XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                        saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                        break;
                    }
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_65_TO_127_OCTETS:
                {
                    if (IS_DEVICE_XP70_XP60(devType) || IS_DEVICE_FALCON(devType))
                    {
                        counters[indx] = portStatistics->tx65To127Octets;
                        break;
                    }
                    else
                    {
                        XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                        saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                        break;
                    }
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_128_TO_255_OCTETS:
                {
                    if (IS_DEVICE_XP70_XP60(devType) || IS_DEVICE_FALCON(devType))
                    {
                        counters[indx] = portStatistics->tx128To255Octets;
                        break;
                    }
                    else
                    {
                        XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                        saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                        break;
                    }
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_256_TO_511_OCTETS:
                {
                    if (IS_DEVICE_XP70_XP60(devType) || IS_DEVICE_FALCON(devType))
                    {
                        counters[indx] = portStatistics->tx256To511Octets;
                        break;
                    }
                    else
                    {
                        XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                        saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                        break;
                    }
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_512_TO_1023_OCTETS:
                {
                    if (IS_DEVICE_XP70_XP60(devType) || IS_DEVICE_FALCON(devType))
                    {
                        counters[indx] = portStatistics->tx512To1023Octets;
                        break;
                    }
                    else
                    {
                        XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                        saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                        break;
                    }
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_1024_TO_1518_OCTETS:
                {
                    if (IS_DEVICE_XP70_XP60(devType) || IS_DEVICE_FALCON(devType))
                    {
                        counters[indx] = portStatistics->tx1024To1518Octets;
                        break;
                    }
                    else
                    {
                        XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                        saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                        break;
                    }
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_1519_TO_2047_OCTETS:
                {
                    if (IS_DEVICE_XP70_XP60(devType) || IS_DEVICE_FALCON(devType))
                    {
                        counters[indx] = portStatistics->tx1519To2047Octets;
                        break;
                    }
                    else
                    {
                        XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                        saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                        break;
                    }
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_2048_TO_4095_OCTETS:
                {
                    if (IS_DEVICE_XP70_XP60(devType) || IS_DEVICE_FALCON(devType))
                    {
                        counters[indx] = portStatistics->tx2048To4095Octets;
                        break;
                    }
                    else
                    {
                        XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                        saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                        break;
                    }
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_4096_TO_9216_OCTETS:
                {
                    counters[indx] = portStatistics->tx4096To9216Octets;
                    break;
                }
            case SAI_PORT_STAT_ETHER_OUT_PKTS_9217_TO_16383_OCTETS:
                {
                    counters[indx] = portStatistics->tx9217To16383Octets;
                    break;
                }
            case SAI_PORT_STAT_ETHER_TX_OVERSIZE_PKTS :
                {
                    counters[indx] = portStatistics->TxOversize;
                    break;
                }
            /** get current port occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_IN_CURR_OCCUPANCY_BYTES:
                {
                    sai_uint32_t count = 0;

                    counters[indx] = 0;
                    for (uint32_t pfcTc = 0; pfcTc < XPSAI_MAX_PRIORITIES_SUPPORTED; pfcTc++)
                    {
                        xpStatus = xpsQosFcGetPfcPortTcCounter(xpsDevId, xpsIntf, pfcTc, &count);
                        if (XP_NO_ERR != xpStatus)
                        {
                            XP_SAI_LOG_DBG("Error: Failed to get pfc port tc counter, "
                                           "port: %u, xpStatus: %d\n", xpsIntf, xpStatus);
                            saiStatus = xpsStatus2SaiStatus(xpStatus);
                        }
                        counters[indx] += count;
                    }

                    counters[indx] = (counters[indx] * XPSAI_BUFFER_PAGE_SIZE_BYTES);
                    break;
                }
            /** get out port current occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_OUT_CURR_OCCUPANCY_BYTES:
                {
                    sai_attribute_value_t numQs;
                    sai_uint32_t count = 0;
                    counters[indx] = 0;

                    saiStatus = xpSaiGetPortAttrQosNumberOfQueues(port_id, &numQs);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_DBG("Error: Could not get number of queues for given port, "
                                       "port: %u, saiStatus: %d\n", xpsIntf, saiStatus);
                        break;
                    }

                    for (uint32_t qIdx = 0; qIdx < numQs.u32; qIdx++)
                    {
                        xpStatus = xpsQosQcGetCurrentQueuePageDepth(xpsDevId, xpsIntf, qIdx, &count);
                        if (XP_NO_ERR != xpStatus)
                        {
                            XP_SAI_LOG_DBG("Error: Failed to get current queue page depth, "
                                           "port: %u, xpStatus: %d\n", xpsIntf, xpStatus);
                            saiStatus = xpsStatus2SaiStatus(xpStatus);
                        }
                        counters[indx] += count;
                    }

                    counters[indx] = (counters[indx] * XPSAI_BUFFER_PAGE_SIZE_BYTES);
                    break;
                }
            /** get watermark port occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_IN_WATERMARK_BYTES:
            /** get current port shared occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_IN_SHARED_CURR_OCCUPANCY_BYTES:
            /** get watermark port shared occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_IN_SHARED_WATERMARK_BYTES:
            /** get out port watermark occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_OUT_WATERMARK_BYTES:
            /** get out port current shared occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_OUT_SHARED_CURR_OCCUPANCY_BYTES:
            /** get out port watermark shared occupancy in bytes [uint64_t] */
            case SAI_PORT_STAT_OUT_SHARED_WATERMARK_BYTES:
            /** get in port packet drops due to buffers [uint64_t] */
            case SAI_PORT_STAT_IN_DROPPED_PKTS:
            /** get out port packet drops due to buffers [uint64_t] */
            case SAI_PORT_STAT_OUT_DROPPED_PKTS:
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[indx]);
                    saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(indx);
                    break;
                }

            /** get the number of pause frames received on the port [uint64_t] */
            case SAI_PORT_STAT_PAUSE_RX_PKTS:
                {
                    counters[indx] = portStatistics->rxPause;
                    break;
                }

            /** get the number of pause frames transmitted on the port [uint64_t] */
            case SAI_PORT_STAT_PAUSE_TX_PKTS:
                {
                    counters[indx] = portStatistics->txPause;
                    break;
                }


            // PFC Packet Counters for RX and TX per PFC priority
            case SAI_PORT_STAT_PFC_0_RX_PKTS:
                {
                    counters[indx] = portStatistics->rxPfc0;
                    break;
                }
            case SAI_PORT_STAT_PFC_0_TX_PKTS:
                {
                    counters[indx] = portStatistics->txPfc0;
                    break;
                }
            case SAI_PORT_STAT_PFC_1_RX_PKTS:
                {
                    counters[indx] = portStatistics->rxPfc1;
                    break;
                }
            case SAI_PORT_STAT_PFC_1_TX_PKTS:
                {
                    counters[indx] = portStatistics->txPfc1;
                    break;
                }
            case SAI_PORT_STAT_PFC_2_RX_PKTS:
                {
                    counters[indx] = portStatistics->rxPfc2;
                    break;
                }
            case SAI_PORT_STAT_PFC_2_TX_PKTS:
                {
                    counters[indx] = portStatistics->txPfc2;
                    break;
                }
            case SAI_PORT_STAT_PFC_3_RX_PKTS:
                {
                    counters[indx] = portStatistics->rxPfc3;
                    break;
                }
            case SAI_PORT_STAT_PFC_3_TX_PKTS:
                {
                    counters[indx] = portStatistics->txPfc3;
                    break;
                }
            case SAI_PORT_STAT_PFC_4_RX_PKTS:
                {
                    counters[indx] = portStatistics->rxPfc4;
                    break;
                }
            case SAI_PORT_STAT_PFC_4_TX_PKTS:
                {
                    counters[indx] = portStatistics->txPfc4;
                    break;
                }
            case SAI_PORT_STAT_PFC_5_RX_PKTS:
                {
                    counters[indx] = portStatistics->rxPfc5;
                    break;
                }
            case SAI_PORT_STAT_PFC_5_TX_PKTS:
                {
                    counters[indx] = portStatistics->txPfc5;
                    break;
                }
            case SAI_PORT_STAT_PFC_6_RX_PKTS:
                {
                    counters[indx] = portStatistics->rxPfc6;
                    break;
                }
            case SAI_PORT_STAT_PFC_6_TX_PKTS:
                {
                    counters[indx] = portStatistics->txPfc6;
                    break;
                }
            case SAI_PORT_STAT_PFC_7_RX_PKTS:
                {
                    counters[indx] = portStatistics->rxPfc7;
                    break;
                }
            case SAI_PORT_STAT_PFC_7_TX_PKTS:
                {
                    counters[indx] = portStatistics->txPfc7;
                    break;
                }
            case SAI_PORT_STAT_PFC_0_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_0_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_1_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_1_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_2_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_2_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_3_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_3_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_4_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_4_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_5_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_5_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_6_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_6_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_7_RX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_7_TX_PAUSE_DURATION:
            case SAI_PORT_STAT_PFC_0_ON2OFF_RX_PKTS:
            case SAI_PORT_STAT_PFC_1_ON2OFF_RX_PKTS:
            case SAI_PORT_STAT_PFC_2_ON2OFF_RX_PKTS:
            case SAI_PORT_STAT_PFC_3_ON2OFF_RX_PKTS:
            case SAI_PORT_STAT_PFC_4_ON2OFF_RX_PKTS:
            case SAI_PORT_STAT_PFC_5_ON2OFF_RX_PKTS:
            case SAI_PORT_STAT_PFC_6_ON2OFF_RX_PKTS:
            case SAI_PORT_STAT_PFC_7_ON2OFF_RX_PKTS:
                {
                    XP_SAI_LOG_DBG("Attribute %d is not supported\n", counter_ids[indx]);
                    //return SAI_STATUS_NOT_SUPPORTED;
                }
            default:
                {
                    XP_SAI_LOG_DBG("Undefined attribute %d\n", counter_ids[indx]);
                    saiStatus = SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(indx);
                }
        }
    }

    if (is_read_clear)
    {
        saiStatus = xpSaiClearPortStatsDB(number_of_counters, counter_ids,
                                          portStatistics);
    }

    return saiStatus;
}

/**
 * Routine Description:
 *   @brief Clear port statistics counters.
 *
 * Arguments:
 *    @param[in] port_id - port id
 *    @param[in] counter_ids - specifies the array of counter ids
 *    @param[in] number_of_counters - number of counters in the array
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t xpSaiClearPortStats(sai_object_id_t port_id,
                                 uint32_t number_of_counters, const sai_stat_id_t *counter_ids)
{
    XP_STATUS     xpStatus  = XP_NO_ERR;
    sai_status_t  saiStatus = SAI_STATUS_SUCCESS;
    xpsDevice_t   xpsDevId  = xpSaiGetDevId();
    xpSaiPortStatisticDbEntryT *portStatistics = NULL;
    xpsInterfaceId_t xpsIntf = 0;

    if ((number_of_counters < 1) || (NULL == counter_ids))
    {
        XP_SAI_LOG_ERR("%s invalid parameters received.", __FUNCNAME__);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    XP_SAI_LOG_DBG("Calling %s with port_id = %u, number_of_counters %d.\n",
                   __FUNCNAME__, xpsIntf, number_of_counters);

    // read port all type statistics to XPS State DB, this operation will clear statistics in HW
    saiStatus = xpSaiPortStatisticsStoreDB(xpsDevId, xpsIntf);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not restore the statistics for the port %u.\n", xpsIntf);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    // get the port statistics from the sai state database
    xpStatus = xpSaiGetPortStatisticInfo(xpsIntf, &portStatistics);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get the port statistic data, xpStatus: %d\n",
                       xpStatus);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    return xpSaiClearPortStatsDB(number_of_counters, counter_ids, portStatistics);
}

/**
 * Routine Description:
 *   @brief Clear port's all statistics counters.
 *
 * Arguments:
 *    @param[in] port_id - port id
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t xpSaiClearPortAllStats(sai_object_id_t port_id)
{

    XP_STATUS     xpStatus = XP_NO_ERR;
    xpsDevice_t   xpsDevId = xpSaiGetDevId();
    xpsInterfaceId_t xpsIntf = 0;
    xpSaiPortStatisticDbEntryT *portStatistics;
    uint32_t counterId_Ingress_v4 = 0;
    uint32_t counterId_Ingress_v6 = 0;
    uint32_t counterId_Egress_v4 = 0;
    uint32_t counterId_Egress_v6 = 0;

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    XP_SAI_LOG_DBG("Calling %s with port_id = %u\n", __FUNCNAME__, xpsIntf);

    xpStatus = (XP_STATUS) xpsMacStatCounterReset(xpsDevId, xpsIntf);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Could not clear statistics for the port %u.\n", xpsIntf);
        return  xpsStatus2SaiStatus(xpStatus);
    }
    // clear port statistics in DB
    xpStatus = xpSaiGetPortStatisticInfo(xpsIntf, &portStatistics);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get the port statistic data, xpStatus: %d\n",
                       xpStatus);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    counterId_Ingress_v4 = portStatistics->counterId_Ingress_v4;
    counterId_Ingress_v6 = portStatistics->counterId_Ingress_v6;
    counterId_Egress_v4 = portStatistics->counterId_Egress_v4;
    counterId_Egress_v6 = portStatistics->counterId_Egress_v6;

    memset(portStatistics, 0, sizeof(xpSaiPortStatisticDbEntryT));
    portStatistics->portNum = xpsIntf;
    portStatistics->counterId_Ingress_v4 = counterId_Ingress_v4;
    portStatistics->counterId_Ingress_v6 = counterId_Ingress_v6;
    portStatistics->counterId_Egress_v4 = counterId_Egress_v4;
    portStatistics->counterId_Egress_v6 = counterId_Egress_v6;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPortStats

sai_status_t xpSaiGetPortStats(sai_object_id_t port_id,
                               uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                               uint64_t *counters)
{
    return xpSaiGetPortStatsInReadClearMode(port_id, number_of_counters,
                                            counter_ids, counters, false);
}

//Func: xpSaiGetPortStatsExt

sai_status_t xpSaiGetPortStatsExt(sai_object_id_t port_id,
                                  uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                  sai_stats_mode_t mode, uint64_t *counters)
{
    return xpSaiGetPortStatsInReadClearMode(port_id, number_of_counters,
                                            counter_ids, counters, (mode == SAI_STATS_MODE_READ_AND_CLEAR));
}


//Func: xpSaiPortInit

XP_STATUS xpSaiPortInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    static xpsDevice_t DevId = xpSaiDevId;

    // Create a Global Sai Port Db
    if ((xpsRetVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Port Db", XPS_GLOBAL,
                                        &xpSaiPortKeyComp, gXpSaiPortDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to Register Sai Port Db\n");
        return xpsRetVal;
    }

    // Create a Global Sai Port Qos Db
    if ((xpsRetVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Port Qos Db",
                                        XPS_GLOBAL, &xpSaiPortQosKeyComp, portQosDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to Register Sai Port Qos Db\n");
        return xpsRetVal;
    }

    // Create a Global Sai Port Lag Db
    if ((xpsRetVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Port LAG Db",
                                        XPS_GLOBAL, &xpSaiPortLagKeyComp, portLagDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to Register Sai Port LAG Db!\n");
        return xpsRetVal;
    }

    // Create a Global Sai Port Lag Db
    if ((xpsRetVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai LAG Port Count Db",
                                        XPS_GLOBAL, &xpSaiLagPortCountKeyComp, portLagPortCountDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to Register Sai LAG Port Count Db!\n");
        return xpsRetVal;
    }

    // Create a Global Sai Port statistic DB
    if ((xpsRetVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Port Statistics Db",
                                        XPS_GLOBAL, &xpSaiPortStatisticKeyComp,
                                        gXpSaiPortStatisticDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to Register Sai Port Statistics Db\n");
        return xpsRetVal;
    }

    xpsRetVal = xpSaiPortCfgMgrInit(xpSaiDevId);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to xpSaiPortCfgMgrInit()!\n");
        return xpsRetVal;
    }
    if (!IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
    {
        gSaiPortThread = sai_thread_create("xpSaiPort", xpSaiPortThreadCb,
                                           &DevId);
    }
    else
    {
        /* Register Link State Callback */
        xpsLinkStateNotify(&xpSaiPortCfgMgrStatusUpdate);
        gSaiPortThread = sai_thread_create("xpSaiPort", xpSaiPortThreadCb,
                                           &DevId);
    }

    return XP_NO_ERR;
}

//Func: xpSaiPortDeInit

XP_STATUS xpSaiPortDeInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS   xpsRetVal = XP_NO_ERR;

    if ((xpsRetVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                          &portQosDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("De-Register Sai Port Qos Db handler failed\n");
        return xpsRetVal;
    }

    if ((xpsRetVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                          &portLagDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("De-Register Sai Port LAG Db handler failed!\n");
        return xpsRetVal;
    }

    if ((xpsRetVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                          &portLagPortCountDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("De-Register Sai LAG Port Count Db handler failed!\n");
        return xpsRetVal;
    }

    if ((xpsRetVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                          &gXpSaiPortDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("De-Register Sai Port Db handler failed!\n");
        return xpsRetVal;
    }

    if ((xpsRetVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                          &gXpSaiPortStatisticDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("De-Register Sai Port Statistic Db handler failed!\n");
        return xpsRetVal;
    }

    xpsRetVal = sai_thread_delete(gSaiPortThread);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("De-Register Sai port thread ret %d\n", xpsRetVal);
        return xpsRetVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiPortApiInit

XP_STATUS xpSaiPortApiInit(uint64_t flag,
                           const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiPortApiInit\n");

    _xpSaiPortApi = (sai_port_api_t *) xpMalloc(sizeof(sai_port_api_t));
    if (NULL == _xpSaiPortApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiPortApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiPortApi->create_port = xpSaiCreatePort;
    _xpSaiPortApi->remove_port = xpSaiRemovePort;
    _xpSaiPortApi->set_port_attribute = xpSaiSetPortAttribute;
    _xpSaiPortApi->get_port_attribute = xpSaiGetPortAttributes;
    _xpSaiPortApi->get_port_stats = xpSaiGetPortStats;
    _xpSaiPortApi->get_port_stats_ext = xpSaiGetPortStatsExt;
    _xpSaiPortApi->clear_port_stats = xpSaiClearPortStats;
    _xpSaiPortApi->clear_port_all_stats = xpSaiClearPortAllStats;

    _xpSaiPortApi->create_port_pool = (sai_create_port_pool_fn)xpSaiStubGenericApi;
    _xpSaiPortApi->remove_port_pool = (sai_remove_port_pool_fn)xpSaiStubGenericApi;
    _xpSaiPortApi->set_port_pool_attribute = (sai_set_port_pool_attribute_fn)
                                             xpSaiStubGenericApi;
    _xpSaiPortApi->get_port_pool_attribute = (sai_get_port_pool_attribute_fn)
                                             xpSaiStubGenericApi;
    _xpSaiPortApi->get_port_pool_stats = (sai_get_port_pool_stats_fn)
                                         xpSaiStubGenericApi;
    _xpSaiPortApi->get_port_pool_stats_ext = (sai_get_port_pool_stats_ext_fn)
                                             xpSaiStubGenericApi;
    _xpSaiPortApi->clear_port_pool_stats = (sai_clear_port_pool_stats_fn)
                                           xpSaiStubGenericApi;

    saiRetVal = xpSaiApiRegister(SAI_API_PORT, (void*)_xpSaiPortApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register port API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}


//Func: xpSaiPortApiDeinit

XP_STATUS xpSaiPortApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiPortApiDeinit\n");

    xpFree(_xpSaiPortApi);
    _xpSaiPortApi = NULL;

    return  retVal;
}

sai_status_t xpSaiMaxCountPortAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_PORT_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountPortObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, gXpSaiPortDbHandle, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetPortObjectList(uint32_t *object_count,
                                    sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpsDevice_t     devId       = xpSaiGetDevId();

    xpSaiPortDbEntryT *pPortNext = NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiCountPortObjects(&objCount);
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
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, gXpSaiPortDbHandle, pPortNext,
                                     (void **)&pPortNext);
        if (retVal != XP_NO_ERR || pPortNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve port object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                     (sai_uint64_t)pPortNext->keyIntfId, &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetPortType(xpsDevice_t xpsDevId, xpsInterfaceId_t xpsIntf,
                              sai_attribute_value_t *value)
{
    XP_STATUS xpStatus    = XP_NO_ERR;
    uint32_t   cpuPortNum  = 0;

    xpStatus = xpsGlobalSwitchControlGetCpuPhysicalPortNum(xpsDevId, &cpuPortNum);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("xpsGlobalSwitchControlGetCpuPhysicalPortNum call failed! Error %d\n",
                       xpStatus);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    if (xpsIntf == cpuPortNum)
    {
        value->s32 = SAI_PORT_TYPE_CPU;
    }
    else
    {
        value->s32 = SAI_PORT_TYPE_LOGICAL;
    }

    return xpsStatus2SaiStatus(xpStatus);
}

//Func: xpSaiPortBindIngressPriorityGrpToPort

sai_status_t xpSaiPortBindIngressPriorityGrpToPort(sai_object_id_t portObjId,
                                                   sai_object_id_t priorityGroupId)
{
    xpSaiPortQosInfo_t *portQosEntry = NULL;
    sai_status_t        status       = SAI_STATUS_SUCCESS;

    if (portObjId == SAI_NULL_OBJECT_ID)
    {
        XP_SAI_LOG_ERR("portObjId is SAI_NULL_OBJECT_ID!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (priorityGroupId == SAI_NULL_OBJECT_ID)
    {
        XP_SAI_LOG_ERR("priorityGroupId is SAI_NULL_OBJECT_ID!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    status = xpSaiGetPortQosInfo(portObjId, &portQosEntry);
    if (status != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get port qos entry from db! error %" PRIi32 "\n",
                       status);
        return status;
    }

    /* Insert item and grow */
    portQosEntry->ingPrioGrps[portQosEntry->numOfIngPrioGrps] = priorityGroupId;
    ++portQosEntry->numOfIngPrioGrps;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiPortUnbingIngressPriorityGrpFromPort

sai_status_t xpSaiPortUnbindIngressPriorityGrpFromPort(sai_object_id_t
                                                       portObjId, sai_object_id_t priorityGroupId)
{
    uint8_t             found        = 0;
    sai_status_t        status       = SAI_STATUS_SUCCESS;
    xpSaiPortQosInfo_t *portQosEntry = NULL;

    if (portObjId == SAI_NULL_OBJECT_ID)
    {
        XP_SAI_LOG_ERR("portObjId is SAI_NULL_OBJECT_ID!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (priorityGroupId == SAI_NULL_OBJECT_ID)
    {
        XP_SAI_LOG_ERR("priorityGroupId is SAI_NULL_OBJECT_ID!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    status = xpSaiGetPortQosInfo(portObjId, &portQosEntry);
    if (status != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get port qos entry from DB! error %" PRIi32 "\n",
                       status);
        return status;
    }

    /* Shift array if one item found */
    found = 0;
    for (uint32_t i = 0; i < portQosEntry->numOfIngPrioGrps; ++i)
    {
        if (found == 1)
        {
            portQosEntry->ingPrioGrps[i-1] = portQosEntry->ingPrioGrps[i];
            portQosEntry->ingPrioGrps[i] = SAI_NULL_OBJECT_ID;
        }
        else if (portQosEntry->ingPrioGrps[i] == priorityGroupId)
        {
            found = 1;
            portQosEntry->ingPrioGrps[i] = SAI_NULL_OBJECT_ID;
        }
    }

    /* Shrink the array size if one item found */
    if (found == 1 && (portQosEntry->numOfIngPrioGrps > 0))
    {
        --portQosEntry->numOfIngPrioGrps;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiPortStatisticsStoreDB(xpsDevice_t devId, uint32_t portNum)
{
    XP_STATUS xpStatus    = XP_NO_ERR;
    xp_Statistics *xpsPortStatistics = NULL;
    xpSaiPortStatisticDbEntryT *portStatistics = NULL;
    uint8_t isAvailable = 0;
    uint32_t cpuPortNum = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    xpStatus = xpsMacIsPortStatsAvailable(devId, portNum, &isAvailable);
    if (XP_NO_ERR != xpStatus)
    {
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (!isAvailable)
    {
        return SAI_STATUS_SUCCESS;
    }

    xpStatus = xpSaiGetPortStatisticInfo(portNum, &portStatistics);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get the port statistic data, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsGlobalSwitchControlGetCpuPhysicalPortNum(devId, &cpuPortNum);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("xpsGlobalSwitchControlGetCpuPhysicalPortNum call failed! Error %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (cpuPortNum != portNum)
    {

        xpStatus = xpsStateHeapMalloc(sizeof(xp_Statistics),
                                      (void**)&xpsPortStatistics);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not allocate port stats structure\n");
            return xpsStatus2SaiStatus(xpStatus);
        }

        memset(xpsPortStatistics, 0, sizeof(xp_Statistics));

        if (xpSaiPortInfoGet(portNum) == NULL)
        {
            XP_SAI_LOG_ERR("Error: Failed to get the port info, xpStatus: %d\n", xpStatus);
            xpsStateHeapFree((void*)xpsPortStatistics);
            return SAI_STATUS_FAILURE;
        }

        if (xpSaiPortInfoGet(portNum)->countersWALinkState == 1)
            xpStatus = xpsMacGetCounterStatsClearOnRead(devId, portNum, 0x0,
                                                        UMAC_MAX_STATISTICS_COUNT_VALUE-1,
                                                        xpsPortStatistics);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_DBG("Error in reading the mac statistics counter \n");
            xpsStateHeapFree((void*)xpsPortStatistics);
            return xpsStatus2SaiStatus(xpStatus);
        }

        for (uint32_t qNum=0; qNum < XPS_MAX_QUEUES_PER_PORT; qNum++)
        {
            sai_status_t        saiStatus      = SAI_STATUS_SUCCESS;
            xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
            sai_object_id_t     queue_id       = SAI_NULL_OBJECT_ID;

            /* Create queue object ID */
            saiStatus = xpSaiQosQueueObjIdCreate(SAI_OBJECT_TYPE_QUEUE, portNum, qNum,
                                                 &queue_id);
            if (SAI_STATUS_SUCCESS != saiStatus)
            {
                XP_SAI_LOG_ERR("Failed to create Queue object id for queue %d\n", qNum);
                return SAI_STATUS_FAILURE;
            }
            /* read queue statistics from HW and store to XPS State DB */
            saiStatus = xpSaiQueueStatisticsStoreDB(queue_id, &pQueueQosInfo);
            if (SAI_STATUS_SUCCESS != saiStatus)
            {
                XP_SAI_LOG_ERR("Could not restore the statistics for the port %u.\n", portNum);
                return  saiStatus;
            }
            /* save 8 queue wred drop stats to port */
            portStatistics->wredDropPkts  += pQueueQosInfo->port_wredDropPkts;
            portStatistics->wredDropBytes += pQueueQosInfo->port_wredDropBytes;

            pQueueQosInfo->port_wredDropPkts  = 0;
            pQueueQosInfo->port_wredDropBytes = 0;
        }

        portStatistics->rxOctets            +=   xpsPortStatistics->octetsRx;
        portStatistics->rxUnicast           +=   xpsPortStatistics->frameRxUnicastAddr;
        portStatistics->rxNonUnicast        +=   xpsPortStatistics->frameRxOk -
                                                 xpsPortStatistics->frameRxUnicastAddr;
        portStatistics->rxErrOrDiscard      +=   xpsPortStatistics->frameRxAnyErr;
        portStatistics->rxBroadcast         +=
            xpsPortStatistics->frameRxBroadcastAddr;
        portStatistics->rxMulticast         +=
            xpsPortStatistics->frameRxMulticastAddr;
        portStatistics->rxUnknownProtocol   +=
            xpsPortStatistics->frameRxUnknownProtocol;
        portStatistics->txOctets            +=
            xpsPortStatistics->octetsTransmittedTotal;
        portStatistics->txUnicast           +=
            xpsPortStatistics->framesTransmittedUnicast;
        portStatistics->txNonUnicast        +=   xpsPortStatistics->frameTransmittedOk -
                                                 xpsPortStatistics->framesTransmittedUnicast;
        portStatistics->txErrOrDiscard      +=
            xpsPortStatistics->frameTransmittedWithErr;
        portStatistics->txBroadcast         +=
            xpsPortStatistics->framesTransmittedBroadcast0;
        portStatistics->txMulticast         +=
            xpsPortStatistics->framesTransmittedMulticast;
        portStatistics->fragments           +=   xpsPortStatistics->fragmentsRx;
        portStatistics->rxOversize          +=   xpsPortStatistics->frameRxOversize;
        portStatistics->txNoError           +=   xpsPortStatistics->frameTransmittedOk;
        portStatistics->rxNoError           +=   xpsPortStatistics->frameRxOk;
        portStatistics->crcAlignErr         +=   xpsPortStatistics->frameRxFcsErr;
        portStatistics->statOversize        +=   xpsPortStatistics->frameRxOversize;
        portStatistics->statUnderSize       +=   xpsPortStatistics->frameRxUndersize;
        portStatistics->statJabbers         +=   xpsPortStatistics->jabberFrameRx;
        portStatistics->rx64Octets          +=   xpsPortStatistics->frameRxLength64;
        portStatistics->rx65To127Octets     +=
            xpsPortStatistics->frameRxLength65To127;
        portStatistics->rx128To255Octets    +=
            xpsPortStatistics->frameRxLength128To255;
        portStatistics->rx256To511Octets    +=
            xpsPortStatistics->frameRxLength256To511;
        portStatistics->rx512To1023Octets   +=
            xpsPortStatistics->frameRxLength512To1023;
        portStatistics->rx1024To1518Octets  +=
            xpsPortStatistics->frameRxLength1024To1518;
        portStatistics->rx1519To2047Octets  +=
            xpsPortStatistics->framesRxLength1519to2047;
        portStatistics->rx2048To4095Octets  +=
            xpsPortStatistics->framesRxLength2048to4095;
        portStatistics->rx4096To9216Octets  +=
            xpsPortStatistics->framesRxLength4096to8191 +
            xpsPortStatistics->framesRxLength8192to9215;
        portStatistics->rx9217To16383Octets  +=
            xpsPortStatistics->framesRxLength9216to16383;
        portStatistics->tx64Octets          +=
            xpsPortStatistics->framesTransmittedLength64;
        portStatistics->tx65To127Octets     +=
            xpsPortStatistics->framesTransmittedLength65to127;
        portStatistics->tx128To255Octets    +=
            xpsPortStatistics->framesTransmittedLength128to255;
        portStatistics->tx256To511Octets    +=
            xpsPortStatistics->framesTransmittedLength256to511;
        portStatistics->tx512To1023Octets   +=
            xpsPortStatistics->framesTransmittedLength512to1023;
        portStatistics->tx1024To1518Octets  +=
            xpsPortStatistics->framesTransmittedLength1024to1518;
        portStatistics->tx1519To2047Octets  +=
            xpsPortStatistics->framesTransmittedLength1519to2047;
        portStatistics->tx2048To4095Octets  +=
            xpsPortStatistics->framesTransmittedLength2048to4095;
        portStatistics->tx4096To9216Octets  +=
            xpsPortStatistics->framesTransmittedLength4096to8191 +
            xpsPortStatistics->framesTransmittedLength8192to9215;
        portStatistics->tx9217To16383Octets  +=
            xpsPortStatistics->framesTransmittedLength9216to16383;
        portStatistics->TxOversize  +=
            xpsPortStatistics->framesTransmittedOversize;
        portStatistics->rxPause             +=   xpsPortStatistics->frameRxTypePause;
        portStatistics->txPause             +=
            xpsPortStatistics->framesTransmittedPause;
        portStatistics->rxPfc0              +=   xpsPortStatistics->framesRxPriNum0;
        portStatistics->txPfc0              +=
            xpsPortStatistics->framesTransmittedPriNum0;
        portStatistics->rxPfc1              +=   xpsPortStatistics->framesRxPriNum1;
        portStatistics->txPfc1              +=
            xpsPortStatistics->framesTransmittedPriNum1;
        portStatistics->rxPfc2              +=   xpsPortStatistics->framesRxPriNum2;
        portStatistics->txPfc2              +=
            xpsPortStatistics->framesTransmittedPriNum2;
        portStatistics->rxPfc3              +=   xpsPortStatistics->framesRxPriNum3;
        portStatistics->txPfc3              +=
            xpsPortStatistics->framesTransmittedPriNum3;
        portStatistics->rxPfc4              +=   xpsPortStatistics->framesRxPriNum4;
        portStatistics->txPfc4              +=
            xpsPortStatistics->framesTransmittedPriNum4;
        portStatistics->rxPfc5              +=   xpsPortStatistics->framesRxPriNum5;
        portStatistics->txPfc5              +=
            xpsPortStatistics->framesTransmittedPriNum5;
        portStatistics->rxPfc6              +=   xpsPortStatistics->framesRxPriNum6;
        portStatistics->txPfc6              +=
            xpsPortStatistics->framesTransmittedPriNum6;
        portStatistics->rxPfc7              +=   xpsPortStatistics->framesRxPriNum7;
        portStatistics->txPfc7              +=
            xpsPortStatistics->framesTransmittedPriNum7;
    }

    xpStatus = xpsMacGetCounterV4V6Stats(devId, portNum,
                                         portStatistics->counterId_Ingress_v4, portStatistics->counterId_Ingress_v6,
                                         portStatistics->counterId_Egress_v4, portStatistics->counterId_Egress_v6,
                                         &portStatistics->txV4Pkts, &portStatistics->txV4Octets,
                                         &portStatistics->rxV4Pkts, &portStatistics->rxV4Octets,
                                         &portStatistics->txV6Pkts, &portStatistics->txV6Octets,
                                         &portStatistics->rxV6Pkts, &portStatistics->rxV6Octets);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Could not get v4/v6 statistics for the port %u.\n", portNum);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    if (NULL != xpsPortStatistics)
    {
        xpsStateHeapFree((void*)xpsPortStatistics);
    }

    return xpsStatus2SaiStatus(xpStatus);
}

XP_STATUS xpSaiPortGetNumPfcPrioritiesSupported(xpsDevice_t devId,
                                                uint32_t *totalSupportedPrio)
{
    if (NULL == totalSupportedPrio)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
    {
        *totalSupportedPrio =XP_SAI_PORT_PFC_PRIORITIES_SUPPORTED;
        return XP_NO_ERR;
    }

    *totalSupportedPrio = 0;
    return XP_NO_ERR;
}




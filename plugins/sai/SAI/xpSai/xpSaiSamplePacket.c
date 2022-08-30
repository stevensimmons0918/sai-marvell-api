// xpSaiSamplePacket.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiSamplePacket.h"
#include "xpSaiPort.h"
#include "xpsState.h"
#include "xpSaiValidationArrays.h"

XP_SAI_LOG_REGISTER_API(SAI_API_SAMPLEPACKET);

static sai_samplepacket_api_t* _xpSaiSamplePacketApi;
static sai_status_t xpSaiSamplePacketUpdateAttribute(sai_object_id_t session_id,
                                                     const sai_attribute_t *attr);

xpsDbHandle_t gsaiSamplePacketDbHdle = XPS_STATE_INVALID_DB_HANDLE;

static int32_t xpSaiSamplePacketEntryKeyComp(void* key1, void* key2)
{
    return ((((xpsSamplePacket_t*)key1)->keyXpsSflowId) - (((
                                                                xpsSamplePacket_t*)key2)->keyXpsSflowId));
}

//Func: xpSaiSamplePacketDbInfoGet

sai_status_t xpSaiSamplePacketDbInfoGet(sai_object_id_t session_id,
                                        xpsSamplePacket_t **xpSamplePacketEntry)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    uint32_t xpsSflowId;
    xpsSamplePacket_t xpSamplePacketKey;

    memset(&xpSamplePacketKey, 0, sizeof(xpSamplePacketKey));
    // Form a key to search database
    xpsSflowId = (uint32_t)xpSaiObjIdValueGet(session_id);
    xpSamplePacketKey.keyXpsSflowId = xpsSflowId;

    xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gsaiSamplePacketDbHdle,
                                   &xpsSflowId, (void**)xpSamplePacketEntry);
    if (*xpSamplePacketEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find Sample Packet entry in DB");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiSamplePacketGetXpsSamplerConfig
// Each sampler samples nSample continious events out of M events.
// M = mBase * 2^mExpo, where M is in range [1, 255*2^15]

static sai_status_t xpSaiSamplePacketGetXpsSamplerConfig(uint32_t sampleRate,
                                                         uint32_t *nSample, uint32_t *mBase, uint32_t *mExpo)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    bool nextIterOverflow = false;
    bool paramsWrong = false;
    uint32_t iter = 0;
    uint32_t base = 0;
    uint32_t expo = 0;
    uint32_t sample = 0;
    double eps = 0;

    if (nSample == NULL || mBase == NULL || mExpo == NULL || sampleRate == 0 ||
        sampleRate > 1 << XPSAI_MAX_SFLOW_M_VALUE)
    {
        XP_SAI_LOG_ERR("Wrong param is passed! sampleRate %d [1..8388608 (2^23)]. "
                       "nSample 0x%" PRIXPTR " [!NULL], "
                       "mBase 0x%" PRIXPTR " [!NULL], "
                       "mExpo 0x%" PRIXPTR " [!NULL]\n",
                       sampleRate, nSample, mBase, mExpo);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    //Calculate nSample, mBase and mExpo and return
    //Increase accuracy, if possible
    while (!paramsWrong)
    {
        if (iter > XPSAI_MAX_SFLOW_M_VALUE)
        {
            paramsWrong = true;
        }

        //Limit expo in range [0..15]
        expo = iter < XPSAI_MAX_SFLOW_EXP_VALUE ? iter : XPSAI_MAX_SFLOW_EXP_VALUE;

        //Limit base in range [1..255]
        if (iter > XPSAI_MAX_SFLOW_EXP_VALUE)
        {
            base = (iter >= XPSAI_MAX_SFLOW_M_VALUE) ? XPSAI_MAX_SFLOW_BASE_VALUE :
                   (uint32_t) pow(2, iter - expo);
        }
        else
        {
            base = XPSAI_SAMPLER_DEFAULT_BASE_VALUE;
        }

        //Limit sample in range [1..255]
        sample = (uint32_t) round(base * pow(2, expo) / sampleRate);
        sample = (sample == 0) ? 1 : sample;
        sample = (sample == XPSAI_MAX_SFLOW_SAMPLE_VALUE + 1) ?
                 XPSAI_MAX_SFLOW_SAMPLE_VALUE : sample;

        if ((uint32_t) round(base * pow(2,
                                        expo + 1) / sampleRate) > XPSAI_MAX_SFLOW_SAMPLE_VALUE + 1)
        {
            nextIterOverflow = true;
        }

        // Calculate relative error, eps = delta(x) / x * 100 %
        eps = abs(((double) sample) / (base * pow(2, expo)) - (1.0 / sampleRate))
              * 100.0 / (((double) sample) / (base * pow(2, expo)));
        if ((eps <= XPSAI_SLOW_HIGH_RELATIVE_ERROR) || (nextIterOverflow &&
                                                        (eps <= XPSAI_SLOW_MID_RELATIVE_ERROR)))
        {

            XP_SAI_LOG_DBG("SAI to XPS relative error: %5.1f%%.\n", eps);
            break;
        }

        ++iter;
    }

    if (paramsWrong)
    {
        XP_SAI_LOG_WARNING("Reached maximum sample rate! Couldn`t provide desired "
                           "sample rate 1 of %d precisely!\n", sampleRate);
    }

    *nSample = sample;
    *mBase = base;
    *mExpo = expo;

    XP_SAI_LOG_DBG("samplerate 1 of %d, mapped to "
                   "nSample : %d mBase : %d, mExpo : %d\n",
                   sampleRate, *nSample, *mBase, *mExpo);

    return saiStatus;
}

static sai_status_t xpSaiSamplePacketCreateMirrorSession(xpsDevice_t xpsDevId,
                                                         xpsSamplePacket_t *xpSamplePacket, sai_object_id_t switch_id)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    sai_object_id_t saiMirrorSessionId = 0;
    sai_object_id_t cpuPort = 0;
    sai_object_id_t trapOid = 0;

    saiRetVal = xpSaiGetSwitchAttrCPUPort(&cpuPort);
    sai_attribute_t mirror_attr_list[4];
    memset(&mirror_attr_list, 0, sizeof(mirror_attr_list));
    mirror_attr_list[0].id = SAI_MIRROR_SESSION_ATTR_TYPE;
    mirror_attr_list[0].value.u32 = SAI_MIRROR_SESSION_TYPE_LOCAL;
    mirror_attr_list[1].id = SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    mirror_attr_list[1].value.oid = cpuPort;
    mirror_attr_list[2].id = SAI_MIRROR_SESSION_ATTR_TRUNCATE_SIZE;
    mirror_attr_list[2].value.u32 = 128;
    mirror_attr_list[3].id = SAI_MIRROR_SESSION_ATTR_SAMPLE_RATE;
    mirror_attr_list[3].value.s32 = xpSamplePacket->rate;

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_HOSTIF_TRAP, xpsDevId,
                                 SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET, &trapOid);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, retVal %d\n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiMirrorSessionCreate(&saiMirrorSessionId, switch_id, 4,
                                         mirror_attr_list);
    xpSamplePacket->mirrorSessionId = saiMirrorSessionId;

    return saiRetVal;
}

static sai_status_t xpSaiSamplePacketRemoveMirrorSession(
    xpsSamplePacket_t *xpSamplePacket, sai_object_id_t saiMirrorSessionId)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    if (xpSamplePacket &&
        (XDK_SAI_OBJID_TYPE_CHECK(saiMirrorSessionId, SAI_OBJECT_TYPE_MIRROR_SESSION)))
    {
        saiRetVal = xpSaiMirrorSessionRemove(saiMirrorSessionId);
    }
    xpSamplePacket->mirrorSessionId = 0;
    return saiRetVal;
}

static sai_status_t xpSaiSamplePacketUpdateMirrorSessionSampleRate(
    xpsDevice_t  devId,
    xpsSamplePacket_t *xpSamplePacket, sai_object_id_t session_id)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiMirrorSession_t *session = NULL;

    if (!(XDK_SAI_OBJID_TYPE_CHECK(session_id, SAI_OBJECT_TYPE_MIRROR_SESSION)))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    saiRetVal = xpSaiMirrorDbInfoGet(session_id, &session);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to locate session %" PRIu64
                       " in data base, error: %d \n", session_id, saiRetVal);
        return saiRetVal;
    }

    sai_attribute_t attr;
    attr.id = SAI_MIRROR_SESSION_ATTR_SAMPLE_RATE;
    attr.value.u32 = xpSamplePacket->rate;

    saiRetVal = xpSaiMirrorSessionUpdate(devId, session, &attr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to update mirror session sample rate with error: %d\n",
                       saiRetVal);
        return saiRetVal;
    }
    session->sampleRate = xpSamplePacket->rate;
    return saiRetVal;
}

//Func: xpSaiSamplePacketDefaultAttributeValsSet

void xpSaiSamplePacketDefaultAttributeValsSet(sai_object_id_t session_id)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    sai_samplepacket_type_t defaultType = SAI_SAMPLEPACKET_TYPE_SLOW_PATH;

    XP_SAI_LOG_DBG("Calling xpSaiSamplePacketDefaultAttributeValsSet\n");

    // SAI_SAMPLEPACKET_ATTR_TYPE default value is SAI_SAMPLEPACKET_TYPE_SLOW_PATH
    if ((saiRetVal = xpSaiSamplePacketAttrSampleTypeSet(session_id,
                                                        defaultType)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set sample packet type %d with error %d \n",
                       defaultType, saiRetVal);
        return;
    }

    return;
}

//Func: xpSaiSamplePacketAttrSampleRateSet

sai_status_t xpSaiSamplePacketAttrSampleRateSet(sai_object_id_t session_id,
                                                sai_uint32_t rate)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = 0;
    xpsSamplePacket_t *xpSamplePacket = NULL;
    sai_object_id_t sampleSessionId = SAI_NULL_OBJECT_ID;
    uint32_t port = 0;
    uint32_t mBase = 0;
    uint32_t mExpo = 0;
    uint32_t nSample = 0;
    uint32_t  maxPortNum  = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(session_id, SAI_OBJECT_TYPE_SAMPLEPACKET))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(session_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    // Retrieve device Id from session Id
    xpsDevId = xpSaiObjIdSwitchGet(session_id);

    // Get max port num
    xpsRetVal = xpsPortGetMaxNum(xpsDevId, &maxPortNum);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve maximum port number\n");
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    saiRetVal = xpSaiSamplePacketDbInfoGet(session_id, &xpSamplePacket);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to locate session %" PRIu64
                       " in database: error code: %d\n", session_id, saiRetVal);
        return saiRetVal;
    }

    xpSamplePacket->rate = rate;
    if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
    {
        if (xpSamplePacket->mirrorSessionId != SAI_NULL_OBJECT_ID)
        {
            saiRetVal = xpSaiSamplePacketUpdateMirrorSessionSampleRate(xpsDevId,
                                                                       xpSamplePacket, xpSamplePacket->mirrorSessionId);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to update mirror session sampleRate error : %d\n",
                               saiRetVal);
                return saiRetVal;
            }
        }
        return SAI_STATUS_SUCCESS;
    }
    // Calculate XPS SFlow required parameters based on sampling rate
    saiRetVal = xpSaiSamplePacketGetXpsSamplerConfig(xpSamplePacket->rate, &nSample,
                                                     &mBase, &mExpo);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetSamplerConfig fails for sampleRate: %d \n",
                       xpSamplePacket->rate);
        return saiRetVal;
    }

    // Loop throught all ports
    XPS_GLOBAL_PORT_ITER(port, maxPortNum)
    {
        // Get sample packet session id applied to the port
        saiRetVal = xpSaiGetPortAttrIngressSampleSessionOid(port, &sampleSessionId);
        // In case port is down
        if (saiRetVal == SAI_STATUS_ITEM_NOT_FOUND)
        {
            continue;
        }
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get port ingress sample packet session Id error : %d\n",
                           saiRetVal);
            return saiRetVal;
        }

        // Check if port sample session is the same as modified session
        if (session_id == sampleSessionId)
        {
            // Configure port sampling rate
            xpsRetVal = xpsSflowSetPortSamplingConfig(port, XP_ACM_ING_PORT_SAMPLER,
                                                      nSample, mBase, mExpo);
            if (xpsRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("xpsSflowSetPortSamplingConfig fails xpsRetVal:%d \n",
                               xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }
        }
    }

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiSetSamplePacketAttrSampleTypeSet

sai_status_t xpSaiSamplePacketAttrSampleTypeSet(sai_object_id_t session_id,
                                                sai_samplepacket_type_t type)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsSamplePacket_t *xpSamplePacket = NULL;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(session_id, SAI_OBJECT_TYPE_SAMPLEPACKET))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(session_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiSamplePacketDbInfoGet(session_id, &xpSamplePacket);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to locate session %" PRIu64
                       " in database: error code: %d\n", session_id, saiRetVal);
        return saiRetVal;
    }

    xpSamplePacket->type = type;
    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiSamplePacketAttrSampleModeSet

sai_status_t xpSaiSamplePacketAttrSampleModeSet(sai_object_id_t session_id,
                                                sai_samplepacket_mode_t mode)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsSamplePacket_t *xpSamplePacket = NULL;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(session_id, SAI_OBJECT_TYPE_SAMPLEPACKET))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(session_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    // We do not support other modes
    if (mode != SAI_SAMPLEPACKET_MODE_SHARED)
    {
        XP_SAI_LOG_ERR("Wrong sample packet mode %d used\n", mode);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiSamplePacketDbInfoGet(session_id, &xpSamplePacket);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to locate session %" PRIu64
                       " in database: error code: %d\n", session_id, saiRetVal);
        return saiRetVal;
    }

    xpSamplePacket->mode = mode;

    return xpsStatus2SaiStatus(xpsRetVal);
}

sai_status_t xpSaiSamplePacketInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    gsaiSamplePacketDbHdle = XPSAI_SAMPLE_PACKET_DB_HNDL;
    xpsInterfaceId_t cpuPortIfId = 0;

    xpsRetVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sample Packet Db", XPS_GLOBAL,
                                   &xpSaiSamplePacketEntryKeyComp, gsaiSamplePacketDbHdle);
    if (xpsRetVal != XP_NO_ERR)
    {
        gsaiSamplePacketDbHdle = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Sample Packet state DB register failed: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    // Enable SFLow feature on per-device basis
    xpsRetVal = xpsSflowSetEnable(xpSaiDevId, true);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to enable Sample Packet SFlow, error %d\n", xpsRetVal);
        return xpsRetVal;
    }

    // Configure sampler client mode
    xpsRetVal = xpsSetCountMode(xpSaiDevId,
                                XP_ACM_ING_PORT_SAMPLER,
                                XP_ACM_SAMPLING,
                                XP_ANA_BANK_MODE_S,
                                XPSAI_SAMPLER_CLEAR_ON_READ,
                                XPSAI_SAMPLER_WRAP_AROUND,
                                XPSAI_SAMPLER_COUNT_OFFSET);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to configure sampling count mode, error %d\n",
                       xpsRetVal);
        return xpsRetVal;
    }

    // Get CPU port interface Id
    xpsRetVal = xpsPortGetCPUPortIntfId(xpSaiDevId, &cpuPortIfId);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsPortGetCPUPortIntfId call failed, Error %d\n", xpsRetVal);
        return  xpsStatus2SaiStatus(xpsRetVal);
    }

    // Set SFlow analyzer port
    xpsRetVal = xpsSflowSetIntfId(xpSaiDevId, cpuPortIfId);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed setting SFlow analyzer port, error %d\n", xpsRetVal);
        return xpsRetVal;
    }

    // Set SFlow trap action
    xpsRetVal = xpsSflowSetPktCmd(xpSaiDevId, XP_PKTCMD_TRAP);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed setting SFlow trap pkt mode, error %d\n", xpsRetVal);
        return xpsRetVal;
    }

    return xpsStatus2SaiStatus(xpsRetVal);
}

sai_status_t xpSaiSamplePacketDeinit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    xpsRetVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &gsaiSamplePacketDbHdle);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Sample Packet state DB Deregister failed: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    // Disable SFlow on per-device basis
    xpsRetVal = xpsSflowSetEnable(xpSaiDevId, false);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to disable Sample Packet SFlow, error %d\n", xpsRetVal);
        return xpsRetVal;
    }

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiSamplePacketCreate
sai_status_t xpSaiSamplePacketSessionCreate(sai_object_id_t* session_id,
                                            sai_object_id_t switch_id, uint32_t attr_count,
                                            const sai_attribute_t *attr_list)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t xpsSflowId = 0, i = 0;
    sai_object_id_t saiSamplePacketId = 0;
    xpsSamplePacket_t *xpSamplePacket = NULL;
    xpsDevice_t xpsDevId = 0;
    bool needCreateSession = true;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               SAMPLEPACKET_VALIDATION_ARRAY_SIZE, samplepacket_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d \n", saiRetVal);
        return saiRetVal;
    }

    // Retrieve device Id from switch Id
    xpsDevId = xpSaiObjIdSwitchGet(switch_id);

    // Allocate Sample Packet ID
    saiRetVal = xpSaiSwitchSampleSessionIdAllocate(&xpsSflowId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiSwitchSampleSessionIdAllocate failed with retVal : %d \n",
                       saiRetVal);
        return saiRetVal;
    }

    // Add the allocated sample paclet Id into the tree along with its properties
    // Allocate space for the xpsSamplePacket data
    if ((xpsRetVal = (XP_STATUS)xpsStateHeapMalloc(sizeof(xpsSamplePacket_t),
                                                   (void**)&xpSamplePacket)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapMalloc failed: retVal %d \n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    // Save SflowId and dummy attributes
    memset(xpSamplePacket, 0, sizeof(xpsSamplePacket_t));
    xpSamplePacket->keyXpsSflowId = xpsSflowId;
    xpSamplePacket->rate = 1;
    xpSamplePacket->type = SAI_SAMPLEPACKET_TYPE_SLOW_PATH;
    xpSamplePacket->mode = SAI_SAMPLEPACKET_MODE_SHARED;
    xpSamplePacket->sampleRefCnt = 1;

    // Insert the entry structure into the databse, using the SamplePacketId
    if ((xpsRetVal = xpsStateInsertData(XP_SCOPE_DEFAULT, gsaiSamplePacketDbHdle,
                                        xpSamplePacket)) != XP_NO_ERR)
    {
        // Free Allocated Memory
        xpsStateHeapFree((void*)xpSamplePacket);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    // Create SAI sample packet OID
    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_SAMPLEPACKET, xpsDevId,
                         (sai_uint64_t) xpsSflowId, &saiSamplePacketId) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object can not be created\n");
        return SAI_STATUS_FAILURE;
    }

    xpSaiSamplePacketDefaultAttributeValsSet(saiSamplePacketId);

    /* For slow path, we need to create a sample session
     * before setting the properties.
     */
    for (i = 0; i < attr_count; ++i)
    {
        if (attr_list[i].id == SAI_SAMPLEPACKET_ATTR_TYPE &&
            (sai_samplepacket_type_t)attr_list[i].value.u32 !=
            SAI_SAMPLEPACKET_TYPE_SLOW_PATH)
        {
            needCreateSession = false;
        }
    }

    if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()) && needCreateSession)
    {
        saiRetVal = xpSaiSamplePacketCreateMirrorSession(xpsDevId, xpSamplePacket,
                                                         switch_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            xpsSamplePacket_t xpSamplePacketKey;
            xpSamplePacketKey.keyXpsSflowId = xpsSflowId;
            if ((xpsRetVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, gsaiSamplePacketDbHdle,
                                                (xpsDbKey_t)&xpSamplePacketKey, (void**)&xpSamplePacket)) != XP_NO_ERR)
            {
                xpsStateHeapFree((void*)xpSamplePacket);
                xpSaiSwitchSampleSessionIdRelease(xpsSflowId);
                return xpsStatus2SaiStatus(xpsRetVal);
            }

            xpsStateHeapFree((void*)xpSamplePacket);
            xpSaiSwitchSampleSessionIdRelease(xpsSflowId);

            XP_SAI_LOG_ERR("Failed to set sample packet attribute for packet sampler %"
                           PRIu64 " \n", saiSamplePacketId);
            return saiRetVal;
        }
    }

    // Apply attributes to newly created sampler
    for (i = 0; i < attr_count; ++i)
    {
        if ((saiRetVal = xpSaiSamplePacketUpdateAttribute(saiSamplePacketId,
                                                          &attr_list[i])) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to set sample packet attribute for packet sampler %"
                           PRIu64 " \n", saiSamplePacketId);
            return saiRetVal;
        }
    }
    *session_id = saiSamplePacketId;

    XP_SAI_LOG_DBG("Succesfully created Sample Packet Session %" PRIu64 " \n\n",
                   saiSamplePacketId);

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiSamplePacketRemove
sai_status_t xpSaiSamplePacketSessionRemove(sai_object_id_t session_id)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    uint32_t xpsSflowId = 0;
    xpsSamplePacket_t *xpSamplePacket = NULL;
    xpsSamplePacket_t xpSamplePacketKey;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(session_id, SAI_OBJECT_TYPE_SAMPLEPACKET))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(session_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsSflowId = (uint32_t)xpSaiObjIdValueGet(session_id);
    xpSamplePacketKey.keyXpsSflowId = xpsSflowId;

    // Search for the node in global sflow Tree database
    xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gsaiSamplePacketDbHdle,
                                   (xpsDbKey_t)&xpSamplePacketKey, (void **) &xpSamplePacket);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsStateSearchData an entry: error code: %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (xpSamplePacket == NULL)
    {
        XP_SAI_LOG_DBG("xpsRBTSearchNode Failure:\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    // Remove ingress analyzer session from hw
    if (xpSamplePacket->sampleRefCnt > 1)
    {
        XP_SAI_LOG_ERR("Sample packet session %" PRIu64 " is in use.\n", session_id);
        XP_SAI_LOG_ERR("Please disable sampling before session remove. \n");
        return SAI_STATUS_FAILURE;
    }
    else
    {
        xpSamplePacket->sampleRefCnt = 0;
    }

    if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()) &&
        xpSamplePacket->type == SAI_SAMPLEPACKET_TYPE_SLOW_PATH)
    {
        xpSaiSamplePacketRemoveMirrorSession(xpSamplePacket,
                                             xpSamplePacket->mirrorSessionId);
    }

    // Remove the sampler from the db
    if ((xpsRetVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, gsaiSamplePacketDbHdle,
                                        (xpsDbKey_t)&xpSamplePacketKey, (void**)&xpSamplePacket)) != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    // Free Allocated Memory
    xpsStateHeapFree((void*)xpSamplePacket);

    //Deallocate Sample Packet ID from xps allocator
    xpSaiSwitchSampleSessionIdRelease(xpsSflowId);

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiSetSamplePacketAttributeInternal

static sai_status_t xpSaiSamplePacketUpdateAttribute(sai_object_id_t session_id,
                                                     const sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    if (!attr)
    {
        XP_SAI_LOG_ERR("attr is NULL!\n");
        return SAI_STATUS_FAILURE;
    }

    switch (attr->id)
    {
        case SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE:
            {
                saiRetVal = xpSaiSamplePacketAttrSampleRateSet(session_id, attr->value.u32);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SAMPLEPACKET_ATTR_TYPE:
            {
                saiRetVal = xpSaiSamplePacketAttrSampleTypeSet(session_id,
                                                               (sai_samplepacket_type_t)attr->value.u32);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_SAMPLEPACKET_ATTR_SAMPLE_TYPE\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SAMPLEPACKET_ATTR_MODE:
            {
                saiRetVal = xpSaiSamplePacketAttrSampleModeSet(session_id,
                                                               (sai_samplepacket_mode_t)attr->value.u32);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_SAMPLEPACKET_ATTR_SAMPLE_MODE\n");
                    return saiRetVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to set %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }
    return saiRetVal;
}

//Func: xpSaiSetSamplePacketAttribute

sai_status_t xpSaiSetSamplePacketAttribute(sai_object_id_t session_id,
                                           const sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    saiRetVal = xpSaiAttrCheck(1, attr,
                               SAMPLEPACKET_VALIDATION_ARRAY_SIZE, samplepacket_attribs,
                               SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d \n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiSamplePacketUpdateAttribute(session_id, attr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Set attributes failed with an error %d \n", saiRetVal);
        return saiRetVal;
    }

    return saiRetVal;
}

//Func: xpSaiSamplePacketSampleRateGet

sai_status_t xpSaiSamplePacketSampleRateGet(sai_object_id_t session_id,
                                            sai_uint32_t *rate)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsSamplePacket_t *xpSamplePacket = NULL;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(session_id, SAI_OBJECT_TYPE_SAMPLEPACKET))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(session_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiSamplePacketDbInfoGet(session_id, &xpSamplePacket);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to locate session %" PRIu64
                       " in database: error code: %d\n", session_id, saiRetVal);
        return saiRetVal;
    }

    *rate = xpSamplePacket->rate;

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiSamplePacketSampleTypeGet

sai_status_t xpSaiSamplePacketSampleTypeGet(sai_object_id_t session_id,
                                            sai_samplepacket_type_t *type)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsSamplePacket_t *xpSamplePacket = NULL;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(session_id, SAI_OBJECT_TYPE_SAMPLEPACKET))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(session_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiSamplePacketDbInfoGet(session_id, &xpSamplePacket);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to locate session %" PRIu64
                       " in database: error code: %d\n", session_id, saiRetVal);
        return saiRetVal;
    }

    *type = xpSamplePacket->type;

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiGetSamplePacketSampleMode

sai_status_t xpSaiSamplePacketSampleModeGet(sai_object_id_t session_id,
                                            sai_samplepacket_mode_t *mode)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsSamplePacket_t *xpSamplePacket = NULL;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(session_id, SAI_OBJECT_TYPE_SAMPLEPACKET))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(session_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiSamplePacketDbInfoGet(session_id, &xpSamplePacket);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to locate session %" PRIu64
                       " in database: error code: %d\n", session_id, saiRetVal);
        return saiRetVal;
    }

    *mode = xpSamplePacket->mode;

    return xpsStatus2SaiStatus(xpsRetVal);
}


//Func: xpSaiGetSamplePacketAttribute

static sai_status_t xpSaiGetSamplePacketAttribute(sai_object_id_t session_id,
                                                  sai_attribute_t* attr, uint32_t attr_index)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    switch (attr->id)
    {
        case SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE:
            {
                saiRetVal = xpSaiSamplePacketSampleRateGet(session_id, &attr->value.u32);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SAMPLEPACKET_ATTR_TYPE:
            {
                saiRetVal = xpSaiSamplePacketSampleTypeGet(session_id,
                                                           (sai_samplepacket_type_t*)&attr->value.u32);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get SAI_SAMPLEPACKET_ATTR_TYPE\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SAMPLEPACKET_ATTR_MODE:
            {
                saiRetVal = xpSaiSamplePacketSampleModeGet(session_id,
                                                           (sai_samplepacket_mode_t*)&attr->value.u32);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get SAI_SAMPLEPACKET_ATTR_MODE\n");
                    return saiRetVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return saiRetVal;
}

//Func: xpSaiGetSamplePacketAttributes

static sai_status_t xpSaiGetSamplePacketAttributes(sai_object_id_t session_id,
                                                   uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t count = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               SAMPLEPACKET_VALIDATION_ARRAY_SIZE, samplepacket_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d \n", saiRetVal);
        return saiRetVal;
    }

    for (count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetSamplePacketAttribute(session_id, &attr_list[count], count);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetSamplePacketAttribute failed\n");
            return saiRetVal;
        }
    }

    return saiRetVal;
}

//Func: xpSaiBulkGetSamplePacketAttributes

sai_status_t xpSaiBulkGetSamplePacketAttributes(sai_object_id_t id,
                                                uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_SAMPLEPACKET))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountSamplePacketAttribute(&maxcount);
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

    for (uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_SAMPLEPACKET_ATTR_START + count;
        saiRetVal = xpSaiGetSamplePacketAttribute(id, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSamplePacketApiInit

XP_STATUS xpSaiSamplePacketApiInit(uint64_t flag,
                                   const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiSamplePacketApiInit\n");

    _xpSaiSamplePacketApi = (sai_samplepacket_api_t *) xpMalloc(sizeof(
                                                                    sai_samplepacket_api_t));
    if (NULL == _xpSaiSamplePacketApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for __xpSaiSamplePacketApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiSamplePacketApi->create_samplepacket = xpSaiSamplePacketSessionCreate;
    _xpSaiSamplePacketApi->remove_samplepacket = xpSaiSamplePacketSessionRemove;
    _xpSaiSamplePacketApi->set_samplepacket_attribute =
        xpSaiSetSamplePacketAttribute;
    _xpSaiSamplePacketApi->get_samplepacket_attribute =
        xpSaiGetSamplePacketAttributes;

    saiRetVal = xpSaiApiRegister(SAI_API_SAMPLEPACKET,
                                 (void*)_xpSaiSamplePacketApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register sample packet API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}


//Func: xpSaiSamplePacketApiDeinit

XP_STATUS xpSaiSamplePacketApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiSamplePacketApiDeinit\n");

    xpFree(_xpSaiSamplePacketApi);
    _xpSaiSamplePacketApi = NULL;

    return retVal;
}

sai_status_t xpSaiMaxCountSamplePacketAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_SAMPLEPACKET_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountSamplePacketObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, gsaiSamplePacketDbHdle, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetSamplePacketObjectList(uint32_t *object_count,
                                            sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpsDevice_t     devId       = xpSaiGetDevId();

    xpsSamplePacket_t *pSamplePacketNext = NULL;

    saiRetVal = xpSaiCountSamplePacketObjects(&objCount);
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
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiSamplePacketDbHdle,
                                     pSamplePacketNext, (void **)&pSamplePacketNext);
        if (retVal != XP_NO_ERR || pSamplePacketNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve sample packet object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_SAMPLEPACKET, devId,
                                     (sai_uint64_t)pSamplePacketNext->keyXpsSflowId, &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiSamplePacketIngressPortEnable(sai_object_id_t port_id,
                                                sai_attribute_value_t value)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t xpsIntf = 0;
    xpsSamplePacket_t *xpSamplePacket = NULL;
    sai_object_id_t sampleSessionId = SAI_NULL_OBJECT_ID;
    uint32_t mBase = 0;
    uint32_t mExpo = 0;
    uint32_t nSample = 0;
    xpsDevice_t  devId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    // Verify current ingress sample packet settings
    saiRetVal = xpSaiGetPortAttrIngressSampleSessionOid(xpsIntf, &sampleSessionId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get port ingress sample packet session Id error : %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    // Same sample packet session is assigned to the port
    if (sampleSessionId == value.oid)
    {
        return SAI_STATUS_SUCCESS;
    }

    // Other sample packet sesion is already assigned to the port
    if ((sampleSessionId != SAI_NULL_OBJECT_ID) &&
        (value.oid != SAI_NULL_OBJECT_ID))
    {
        if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
        {
            saiRetVal = xpSaiSamplePacketDbInfoGet(sampleSessionId, &xpSamplePacket);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to get session %" PRIu64
                               " in database: error code: %d\n", sampleSessionId, saiRetVal);
                return saiRetVal;
            }
            if (xpSamplePacket->type == SAI_SAMPLEPACKET_TYPE_SLOW_PATH)
            {
                saiRetVal = xpSaiMirrorSessionPortMirrorEnable(xpsIntf,
                                                               xpSamplePacket->mirrorSessionId, true, false);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set port ingress mirror session Id error : %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                xpSamplePacket->sampleRefCnt --;
                saiRetVal = xpSaiSetPortAttrIngressSampleSessionOid(xpsIntf,
                                                                    SAI_NULL_OBJECT_ID);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set port ingress sample packet session Id error : %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
            }
            else
            {
                XP_SAI_LOG_ERR("Currently sample packet session %" PRIu64
                               " is assigned to port %" PRIu64 " \n", sampleSessionId, port_id);
                XP_SAI_LOG_ERR("Please disable ingress sampling on this port before assigning new session.\n");
                return SAI_STATUS_OBJECT_IN_USE;
            }
        }
    }

    // Disable ingress sampling in case attribute value is #SAI_NULL_OBJECT_ID
    if (value.oid == SAI_NULL_OBJECT_ID)
    {
        if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
        {
            saiRetVal = xpSaiSamplePacketDbInfoGet(sampleSessionId, &xpSamplePacket);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to get session %" PRIu64
                               " in database: error code: %d\n", sampleSessionId, saiRetVal);
                return saiRetVal;
            }
            if (xpSamplePacket->type == SAI_SAMPLEPACKET_TYPE_SLOW_PATH)
            {
                saiRetVal = xpSaiMirrorSessionPortMirrorEnable(xpsIntf,
                                                               xpSamplePacket->mirrorSessionId, true, false);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set port ingress mirror session Id error : %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                xpSamplePacket->sampleRefCnt --;
            }
            else
            {
                // Check if it is referenced
                if ((--(xpSamplePacket->sampleRefCnt)) == 1)
                {
                    xpSamplePacket->mirrorSessionId = SAI_NULL_OBJECT_ID;
                }
            }
        }
        else
        {
            // Disable sampling on a port
            xpsRetVal = xpsSflowEnablePortSampling(xpsIntf, XPSAI_PORT_SAMPLER_DISABLE);
            if (xpsRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("xpsSflowEnablePortSampling fails xpsRetVal:%d \n", xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }
        }

        // Disable sampling on a port
        saiRetVal = xpSaiSetPortAttrIngressSampleSessionOid(xpsIntf, value.oid);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to set port ingress sample packet session Id error : %d\n",
                           saiRetVal);
            return saiRetVal;
        }

        return SAI_STATUS_SUCCESS;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(value.oid, SAI_OBJECT_TYPE_SAMPLEPACKET))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(value.oid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiSamplePacketDbInfoGet(value.oid, &xpSamplePacket);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to locate session %" PRIu64
                       " in database: error code: %d\n", value.oid, saiRetVal);
        return saiRetVal;
    }

    if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
    {
        if (xpSamplePacket->type == SAI_SAMPLEPACKET_TYPE_SLOW_PATH)
        {
            saiRetVal = xpSaiMirrorSessionPortMirrorEnable(xpsIntf,
                                                           xpSamplePacket->mirrorSessionId, true, true);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to set port ingress mirror session Id error : %d\n",
                               saiRetVal);
                return saiRetVal;
            }
            xpSaiSamplePacketUpdateMirrorSessionSampleRate(devId, xpSamplePacket,
                                                           xpSamplePacket->mirrorSessionId);
            xpSamplePacket->sampleRefCnt ++;
        }
        else
        {
            xpSaiPortDbEntryT *pPortEntry = NULL;

            saiRetVal = xpSaiPortDbInfoGet(xpsIntf, &pPortEntry);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not retrieve port info from DB");
                return saiRetVal;
            }
            sai_object_id_t session_id = SAI_NULL_OBJECT_ID;
            xpSaiMirrorSession_t *session = NULL;
            int i;
            for (i = 0; i < XPSAI_DIRECTED_MIRROR_SESSION_NUM; i++)
            {
                if (pPortEntry->ingressMirrorSessionOid[i] != SAI_NULL_OBJECT_ID)
                {
                    saiRetVal = xpSaiMirrorDbInfoGet(pPortEntry->ingressMirrorSessionOid[i],
                                                     &session);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_DBG("Failed to locate session %" PRIu64
                                       " in data base, error: %d \n", session_id, saiRetVal);
                        return saiRetVal;
                    }
                    if (session && session->sessionType == SAI_MIRROR_SESSION_TYPE_SFLOW)
                    {
                        xpSamplePacket->mirrorSessionId = pPortEntry->ingressMirrorSessionOid[i];
                        saiRetVal = xpSaiSamplePacketUpdateMirrorSessionSampleRate(devId,
                                                                                   xpSamplePacket, pPortEntry->ingressMirrorSessionOid[i]);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("Failed to update mirror session sampleRate error : %d\n",
                                           saiRetVal);
                            return saiRetVal;
                        }
                        xpSamplePacket->sampleRefCnt ++;
                        break;
                    }
                }
            }
            if (i == XPSAI_DIRECTED_MIRROR_SESSION_NUM)
            {
                XP_SAI_LOG_ERR("No sflow mirror in port.");
                return SAI_STATUS_INVALID_PARAMETER;
            }
        }
    }
    else
    {
        // Calculate XPS SFlow required parameters based on sampling rate
        saiRetVal = xpSaiSamplePacketGetXpsSamplerConfig(xpSamplePacket->rate, &nSample,
                                                         &mBase, &mExpo);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiGetSamplerConfig fails for sampleRate: %d \n",
                           xpSamplePacket->rate);
            return saiRetVal;
        }
        // Configure port sampling rate
        xpsRetVal = xpsSflowSetPortSamplingConfig(xpsIntf, XP_ACM_ING_PORT_SAMPLER,
                                                  nSample,
                                                  mBase, mExpo);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsSflowSetPortSamplingConfig fails xpsRetVal:%d \n",
                           xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }

        // Enable packet sampling on port
        xpsRetVal = xpsSflowEnablePortSampling(xpsIntf, XPSAI_PORT_SAMPLER_ENABLE);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsSflowEnablePortSampling fails xpsRetVal:%d \n", xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }

    saiRetVal = xpSaiSetPortAttrIngressSampleSessionOid(xpsIntf, value.oid);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set port ingress sample packet session Id error : %d\n",
                       saiRetVal);
        return saiRetVal;
    }
    return saiRetVal;
}

sai_status_t xpSaiSamplePacketEgressPortEnable(sai_object_id_t port_id,
                                               sai_attribute_value_t value)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t xpsIntf = 0;
    xpsSamplePacket_t *xpSamplePacket = NULL;
    sai_object_id_t sampleSessionId = SAI_NULL_OBJECT_ID;
    uint32_t mBase = 0;
    uint32_t mExpo = 0;
    uint32_t nSample = 0;
    xpsDevice_t  devId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCTION__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    // Verify current engress sample packet settings
    saiRetVal = xpSaiGetPortAttrEgressSampleSessionOid(xpsIntf, &sampleSessionId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get port egress sample packet session Id error : %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    // Same sample packet session is assigned to the port
    if (sampleSessionId == value.oid)
    {
        return SAI_STATUS_SUCCESS;
    }

    // Other sample packet sesion is already assigned to the port
    if ((sampleSessionId != SAI_NULL_OBJECT_ID) &&
        (value.oid != SAI_NULL_OBJECT_ID))
    {
        XP_SAI_LOG_ERR("Currently sample packet session %" PRIu64
                       " is assigned to port %" PRIu64 " \n", sampleSessionId, port_id);
        XP_SAI_LOG_ERR("Please disable ingress sampling on this port before assigning new session.\n");
        return SAI_STATUS_OBJECT_IN_USE;
    }

    // Disable ingress sampling in case attribute value is #SAI_NULL_OBJECT_ID
    if (value.oid == SAI_NULL_OBJECT_ID)
    {
        if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
        {
            saiRetVal = xpSaiSamplePacketDbInfoGet(sampleSessionId, &xpSamplePacket);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to get session %" PRIu64
                               " in database: error code: %d\n", sampleSessionId, saiRetVal);
                return saiRetVal;
            }
            if (xpSamplePacket->type == SAI_SAMPLEPACKET_TYPE_SLOW_PATH)
            {
                saiRetVal = xpSaiMirrorSessionPortMirrorEnable(xpsIntf,
                                                               xpSamplePacket->mirrorSessionId, false, false);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set port egress mirror session Id error : %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                xpSamplePacket->sampleRefCnt --;
            }
            else
            {
                // Check if it is referenced
                if ((--(xpSamplePacket->sampleRefCnt)) == 1)
                {
                    xpSamplePacket->mirrorSessionId = SAI_NULL_OBJECT_ID;
                }
            }
        }
        else
        {
            // Disable sampling on a port
            xpsRetVal = xpsSflowEnablePortSampling(xpsIntf, XPSAI_PORT_SAMPLER_DISABLE);
            if (xpsRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("xpsSflowEnablePortSampling fails xpsRetVal:%d \n", xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }
        }
        saiRetVal = xpSaiSetPortAttrEgressSampleSessionOid(xpsIntf, value.oid);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to set port ingress sample packet session Id error : %d\n",
                           saiRetVal);
            return saiRetVal;
        }
        return SAI_STATUS_SUCCESS;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(value.oid, SAI_OBJECT_TYPE_SAMPLEPACKET))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(value.oid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiSamplePacketDbInfoGet(value.oid, &xpSamplePacket);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to locate session %" PRIu64
                       " in database: error code: %d\n", value.oid, saiRetVal);
        return saiRetVal;
    }

    if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
    {
        if (xpSamplePacket->type == SAI_SAMPLEPACKET_TYPE_SLOW_PATH)
        {
            saiRetVal = xpSaiMirrorSessionPortMirrorEnable(xpsIntf,
                                                           xpSamplePacket->mirrorSessionId, false, true);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to set port ingress mirror session Id error : %d\n",
                               saiRetVal);
                return saiRetVal;
            }
            xpSaiSamplePacketUpdateMirrorSessionSampleRate(devId, xpSamplePacket,
                                                           xpSamplePacket->mirrorSessionId);
            xpSamplePacket->sampleRefCnt ++;
        }
        else
        {
            xpSaiPortDbEntryT *pPortEntry = NULL;

            saiRetVal = xpSaiPortDbInfoGet(xpsIntf, &pPortEntry);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not retrieve port info from DB");
                return saiRetVal;
            }
            sai_object_id_t session_id = SAI_NULL_OBJECT_ID;
            xpSaiMirrorSession_t *session = NULL;
            int i;
            for (i = 0; i < XPSAI_DIRECTED_MIRROR_SESSION_NUM; i++)
            {
                if (pPortEntry->egressMirrorSessionOid[i] != SAI_NULL_OBJECT_ID)
                {
                    saiRetVal = xpSaiMirrorDbInfoGet(pPortEntry->egressMirrorSessionOid[i],
                                                     &session);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_DBG("Failed to locate session %" PRIu64
                                       " in data base, error: %d \n", session_id, saiRetVal);
                        return saiRetVal;
                    }
                    if (session && session->sessionType == SAI_MIRROR_SESSION_TYPE_SFLOW)
                    {
                        xpSamplePacket->mirrorSessionId = pPortEntry->egressMirrorSessionOid[i];
                        saiRetVal = xpSaiSamplePacketUpdateMirrorSessionSampleRate(devId,
                                                                                   xpSamplePacket, pPortEntry->egressMirrorSessionOid[i]);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("Failed to update mirror session sampleRate error : %d\n",
                                           saiRetVal);
                            return saiRetVal;
                        }
                        xpSamplePacket->sampleRefCnt ++;
                        break;
                    }
                }
            }
            if (i == XPSAI_DIRECTED_MIRROR_SESSION_NUM)
            {
                XP_SAI_LOG_ERR("No sflow mirror in port.");
                return SAI_STATUS_INVALID_PARAMETER;
            }
        }
    }
    else
    {
        // Calculate XPS SFlow required parameters based on sampling rate
        saiRetVal = xpSaiSamplePacketGetXpsSamplerConfig(xpSamplePacket->rate, &nSample,
                                                         &mBase, &mExpo);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiGetSamplerConfig fails for sampleRate: %d \n",
                           xpSamplePacket->rate);
            return saiRetVal;
        }
        // Configure port sampling rate
        xpsRetVal = xpsSflowSetPortSamplingConfig(xpsIntf, XP_ACM_ING_PORT_SAMPLER,
                                                  nSample,
                                                  mBase, mExpo);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsSflowSetPortSamplingConfig fails xpsRetVal:%d \n",
                           xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }

        // Enable packet sampling on port
        xpsRetVal = xpsSflowEnablePortSampling(xpsIntf, XPSAI_PORT_SAMPLER_ENABLE);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsSflowEnablePortSampling fails xpsRetVal:%d \n", xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }

    saiRetVal = xpSaiSetPortAttrEgressSampleSessionOid(xpsIntf, value.oid);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set port ingress sample packet session Id error : %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    return saiRetVal;
}



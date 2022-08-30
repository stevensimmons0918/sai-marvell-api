// xpSaiPolicer.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiPolicer.h"
#include "xpSaiValidationArrays.h"
#include "xpSaiStub.h"
#include "cpssHalQos.h"

XP_SAI_LOG_REGISTER_API(SAI_API_POLICER);

#define XP_SAI_POLICER_MAX_IDS     256
#define XP_SAI_POLICER_RANGE_START 1

sai_policer_api_t* _xpSaiPolicerApi;
xpsDbHandle_t policerDbHandle = XPSAI_POLICER_DB_HNDL;

//Func: xpSaiConvertPolicerOid

sai_status_t xpSaiConvertPolicerOid(sai_object_id_t policer_oid,
                                    uint32_t* pPolicerId)
{
    if (!XDK_SAI_OBJID_TYPE_CHECK(policer_oid, SAI_OBJECT_TYPE_POLICER))
    {
        XP_SAI_LOG_ERR("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(policer_oid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *pPolicerId = (uint32_t)xpSaiObjIdValueGet(policer_oid);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiPolicerKeyComp

static sai_int32_t xpSaiPolicerKeyComp(void *key1, void *key2)
{
    return ((((xpSaiPolicer_t*)key1)->policerId) - (((xpSaiPolicer_t*)
                                                     key2)->policerId));
}

//Func: xpSaiPolicerCreate

sai_status_t xpSaiPolicerCreate(uint32_t *pPolicerId)
{
    XP_STATUS xpsRetVal;

    if ((xpsRetVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_POLICER,
                                            pPolicerId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to allocate Sai Policer Id |Error: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiPolicerRemove

sai_status_t xpSaiPolicerRemove(uint32_t policerId)
{
    XP_STATUS xpsRetVal;

    if ((xpsRetVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_POLICER,
                                           policerId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to release Sai Policer |Error: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPolicerAttrInfo

XP_STATUS xpSaiGetPolicerAttrInfo(sai_object_id_t policer_id,
                                  xpSaiPolicerAttributes_t *pPolicerAttrInfo)
{
    xpSaiPolicer_t policerInfoKey;
    xpSaiPolicer_t *pPolicerInfo = NULL;
    XP_STATUS xpsRetVal = XP_NO_ERR;

    policerInfoKey.policerId = policer_id;
    /* Retrieve the corresponding state from sai policer db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, policerDbHandle,
                                        (xpsDbKey_t)&policerInfoKey, (void**)&pPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai policer data |Error: %d\n",
                       xpsRetVal);
        return xpsRetVal;
    }
    if (!pPolicerInfo)
    {
        XP_SAI_LOG_DBG("Sai policer entry not found\n");
        return XP_ERR_NOT_FOUND;
    }
    memcpy(pPolicerAttrInfo, &pPolicerInfo->attrInfo,
           sizeof(xpSaiPolicerAttributes_t));
    return XP_NO_ERR;
}

//Func: xpSaiPolicerValidatePolicerObject

sai_status_t xpSaiPolicerValidatePolicerObject(sai_object_id_t policerObjId)
{
    xpSaiPolicer_t  key;
    xpSaiPolicer_t *pPolicerInfo = NULL;
    XP_STATUS       xpStatus     = XP_NO_ERR;

    /* Validate non null Objects */
    if (policerObjId != SAI_NULL_OBJECT_ID)
    {
        /* Validate the buffer pool object */
        if (!XDK_SAI_OBJID_TYPE_CHECK(policerObjId, SAI_OBJECT_TYPE_POLICER))
        {
            XP_SAI_LOG_ERR("Error: Wrong object type received (%u)\n",
                           xpSaiObjIdTypeGet(policerObjId));
            return SAI_STATUS_INVALID_OBJECT_TYPE;
        }

        /* Populate the key */
        key.policerId = policerObjId;

        /* Retrieve the corresponding state from state database */
        xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, policerDbHandle,
                                      (xpsDbKey_t)&key, (void**)&pPolicerInfo);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to retrieve the state data, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);;
        }

        if (!pPolicerInfo)
        {
            XP_SAI_LOG_ERR("Error: Policer entry does not exist\n");
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiPolicerGetXpsEntry

sai_status_t xpSaiPolicerGetXpsEntry(sai_object_id_t policerOid,
                                     xpsPolicerEntry_t *policerEntry)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpSaiPolicerAttributes_t policerAttrInfo;

    if (policerEntry == NULL)
    {
        XP_SAI_LOG_ERR("NULL pointer: status=%i.\n", SAI_STATUS_INVALID_PARAMETER);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpStatus = xpSaiGetPolicerAttrInfo(policerOid, &policerAttrInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetPolicerAttrInfo failed: status=%i.\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    memset(policerEntry, 0, sizeof(xpsPolicerEntry_t));

    if (policerAttrInfo.meterType.s32 == SAI_METER_TYPE_PACKETS)
    {
        policerEntry->cir = (uint32_t)policerAttrInfo.cir.u64;
        policerEntry->pir = (uint32_t)policerAttrInfo.pir.u64;

        policerEntry->isPacketBasedPolicing = 1;
    }
    else
    {
        /* Convert Bytes to Kbps */
        policerEntry->cir = (policerAttrInfo.cir.u64 * XP_BITS_IN_BYTE) /
                            XP_KBPS_TO_BITS_FACTOR;
        policerEntry->pir = (policerAttrInfo.pir.u64 * XP_BITS_IN_BYTE) /
                            XP_KBPS_TO_BITS_FACTOR;

        policerEntry->isPacketBasedPolicing = 0;
    }

    policerEntry->cbs = (uint32_t)policerAttrInfo.cbs.u64;
    policerEntry->pbs = (uint32_t)policerAttrInfo.pbs.u64;

    if (policerAttrInfo.mode.s32 == SAI_POLICER_MODE_SR_TCM)
    {
        policerEntry->mode = XPS_POLICER_MODE_SRTC_E;
    }
    else if (policerAttrInfo.mode.s32 == SAI_POLICER_MODE_STORM_CONTROL)
    {
        policerEntry->mode = XPS_POLICER_MODE_STORM_CONTROL;
    }
    else
    {
        policerEntry->mode = XPS_POLICER_MODE_TRTC_E;
    }

    policerEntry->colorAware = (policerAttrInfo.colorSource.s32 ==
                                SAI_POLICER_COLOR_SOURCE_AWARE) ? 1 : 0;
    policerEntry->dropRed = (policerAttrInfo.redPacketAction.s32 ==
                             SAI_PACKET_ACTION_DROP) ? 1 : 0;
    policerEntry->dropYellow = (policerAttrInfo.yellowPacketAction.s32 ==
                                SAI_PACKET_ACTION_DROP) ? 1 : 0;
    policerEntry->dropGreen = (policerAttrInfo.greenPacketAction.s32 ==
                               SAI_PACKET_ACTION_DROP) ? 1 : 0;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiPolicerAddIdxToIdxList

XP_STATUS xpSaiPolicerAddIdxToIdxList(sai_object_id_t policerId,
                                      xpsPolicerType_e client, sai_uint32_t index)
{
    xpSaiPolicer_t *pNewPolicerInfo = NULL;
    xpSaiPolicer_t *pPolicerInfo = NULL;
    xpSaiPolicer_t policerInfoKey;
    sai_uint32_t indexCount = 0;
    XP_STATUS xpsRetVal;

    policerInfoKey.policerId = policerId;
    /* Retrieve the corresponding state from sai policer db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, policerDbHandle,
                                        (xpsDbKey_t)&policerInfoKey, (void**)&pPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai policer data |Error: %d\n",
                       xpsRetVal);
        return xpsRetVal;
    }
    if (!pPolicerInfo)
    {
        XP_SAI_LOG_ERR("Sai policer entry not found\n");
        return XP_ERR_NOT_FOUND;
    }
    indexCount = pPolicerInfo->numOfIdx;
    if (xpsDAIsCtxGrowthNeeded(pPolicerInfo->numOfIdx,
                               XPSAI_DEFAULT_POLICER_IDX_LIST_SIZE))
    {
        /* Double the indexList array size */
        if ((xpsRetVal = xpSaiDynamicArrayGrow((void**)&pNewPolicerInfo,
                                               (void*)pPolicerInfo, sizeof(xpSaiPolicer_t), sizeof(xpSaiPolicerIdxMapList_t),
                                               pPolicerInfo->numOfIdx, XPSAI_DEFAULT_POLICER_IDX_LIST_SIZE)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Dynamic array grow failed |Error: %d\n", xpsRetVal);
            return xpsRetVal;
        }

        /* Remove the corresponding state from sai policer db and add new state */
        if ((xpsRetVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, policerDbHandle,
                                            (xpsDbKey_t)&policerInfoKey, (void**)&pPolicerInfo)) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)pNewPolicerInfo);
            XP_SAI_LOG_ERR("Deletion of data failed |Error: %d\n", xpsRetVal);
            return xpsRetVal;
        }

        /* Free the memory allocated for the corresponding state */
        if ((xpsRetVal = xpsStateHeapFree(pPolicerInfo)) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)pNewPolicerInfo);
            XP_SAI_LOG_ERR("Freeing allocated memory failed |Error: %d\n", xpsRetVal);
            return xpsRetVal;
        }
        /* Insert the new state */
        if ((xpsRetVal = xpsStateInsertData(XP_SCOPE_DEFAULT, policerDbHandle,
                                            (void*)pNewPolicerInfo)) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)pNewPolicerInfo);
            XP_SAI_LOG_ERR("Failed to insert data |Error: %d\n", xpsRetVal);
            return xpsRetVal;
        }
        pPolicerInfo = pNewPolicerInfo;
    }
    pPolicerInfo->indexList[indexCount].client = client;
    pPolicerInfo->indexList[indexCount].index = index;
    pPolicerInfo->numOfIdx += 1;

    return XP_NO_ERR;
}

//Func: xpSaiPolicerRemoveIdxFromIdxList

XP_STATUS xpSaiPolicerRemoveIdxFromIdxList(sai_object_id_t policerId,
                                           xpsPolicerType_e client, sai_uint32_t index)
{
    xpSaiPolicer_t *pNewPolicerInfo = NULL;
    xpSaiPolicer_t *pPolicerInfo = NULL;
    xpSaiPolicer_t policerInfoKey;
    sai_uint8_t indexPresent = 0;
    XP_STATUS xpsRetVal;

    policerInfoKey.policerId = policerId;
    /* Retrieve the corresponding state from sai policer db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, policerDbHandle,
                                        (xpsDbKey_t)&policerInfoKey, (void**)&pPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai policer data |Error: %d\n",
                       xpsRetVal);
        return xpsRetVal;
    }
    if (!pPolicerInfo)
    {
        XP_SAI_LOG_ERR("Sai policer entry not found\n");
        return XP_ERR_NOT_FOUND;
    }

    /* Remove the index from policer indexList */
    for (uint16_t count = 0; count < pPolicerInfo->numOfIdx; count++)
    {
        if (!indexPresent)
        {
            if ((pPolicerInfo->indexList[count].index == index) &&
                (pPolicerInfo->indexList[count].client == client))
            {
                indexPresent = 1;
            }
            continue;
        }
        else
        {
            pPolicerInfo->indexList[count-1].index = pPolicerInfo->indexList[count].index;
            pPolicerInfo->indexList[count-1].client = pPolicerInfo->indexList[count].client;
        }
    }
    if (!indexPresent)
    {
        return XP_ERR_INVALID_ARG;
    }

    /* Decrement the numOfIdx count */
    pPolicerInfo->numOfIdx -= 1;

    if (xpsDAIsCtxShrinkNeeded(pPolicerInfo->numOfIdx,
                               XPSAI_DEFAULT_POLICER_IDX_LIST_SIZE))
    {
        /* Shrink the indexList array size */
        if ((xpsRetVal = xpSaiDynamicArrayShrink((void**)&pNewPolicerInfo,
                                                 (void*)pPolicerInfo, sizeof(xpSaiPolicer_t), sizeof(xpSaiPolicerIdxMapList_t),
                                                 pPolicerInfo->numOfIdx, XPSAI_DEFAULT_POLICER_IDX_LIST_SIZE)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Dynamic array shrink failed |Error: %d\n", xpsRetVal);
            return xpsRetVal;
        }

        /* Remove the corresponding state from sai policer db and add new state */
        if ((xpsRetVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, policerDbHandle,
                                            (xpsDbKey_t)&policerInfoKey, (void**)&pPolicerInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Deletion of data failed |Error: %d\n", xpsRetVal);
            xpsStateHeapFree((void*)pNewPolicerInfo);
            return xpsRetVal;
        }

        /* Free the memory allocated for the corresponding state */
        if ((xpsRetVal = xpsStateHeapFree(pPolicerInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Freeing allocated memory failed |Error: %d\n", xpsRetVal);
            xpsStateHeapFree((void*)pNewPolicerInfo);
            return xpsRetVal;
        }
        /* Insert the new state */
        if ((xpsRetVal = xpsStateInsertData(XP_SCOPE_DEFAULT, policerDbHandle,
                                            (void*)pNewPolicerInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Inserting policer DB handle failed |Error: %d\n", xpsRetVal);
            xpsStateHeapFree((void*)pNewPolicerInfo);
            return xpsRetVal;
        }
    }
    return XP_NO_ERR;
}

//xpSaiIsCounterListValid

static bool xpSaiIsCounterListValid(sai_attribute_value_t value)
{
    for (uint32_t count = 0; count < value.s32list.count; count++)
    {
        switch (value.s32list.list[count])
        {
            case SAI_PACKET_ACTION_DROP:
                {
                    break;
                }
            case SAI_PACKET_ACTION_FORWARD:
                {
                    break;
                }
            case SAI_PACKET_ACTION_COPY:
            case SAI_PACKET_ACTION_COPY_CANCEL:
            case SAI_PACKET_ACTION_TRAP:
            case SAI_PACKET_ACTION_LOG:
            case SAI_PACKET_ACTION_DENY:
            case SAI_PACKET_ACTION_TRANSIT:
                {
                    XP_SAI_LOG_ERR("Only DROP and FORWARD Packet actions are supported for policer counter, "
                                   "\n Invalid Packet Action (%d)\n", value.s32list.list[count]);
                    return false;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Invalid PACKET ACTION type %d\n", value.s32list.list[count]);
                    return false;
                }
        }
    }
    return true;
}

//Func: xpSaiSetDefaultPolicerAttributeVals

void xpSaiSetDefaultPolicerAttributeVals(xpSaiPolicerAttributes_t* attributes)
{
    memset(attributes, 0, sizeof(xpSaiPolicerAttributes_t));

    /* Policer Color Source [sai_policer_color_source_t], Default is SAI_POLICER_COLOR_SOURCE_AWARE*/
    attributes->colorSource.s32 = SAI_POLICER_COLOR_SOURCE_AWARE;

    /* Action to take for Green color packets [sai_packet_action_t], Default action SAI_PACKET_ACTION_FORWARD*/
    attributes->greenPacketAction.s32 = SAI_PACKET_ACTION_FORWARD;

    /* Action to take for Yellow color packets [sai_packet_action_t], Default action SAI_PACKET_ACTION_FORWARD*/
    attributes->yellowPacketAction.s32 = SAI_PACKET_ACTION_FORWARD;

    /* Action to take for Red color packets [sai_packet_action_t], Default action SAI_PACKET_ACTION_FORWARD*/
    attributes->redPacketAction.s32 = SAI_PACKET_ACTION_FORWARD;

    /* Initialize with zero */
    attributes->enableCounterList.s32list.count = 0;

    /* No memory is allocated, just used for verifying */
    attributes->enableCounterList.s32list.list =  NULL;

}

//Func: xpSaiUpdatePolicerAttributeVals

void xpSaiUpdatePolicerAttributeVals(const uint32_t attr_count,
                                     const sai_attribute_t* attr_list, xpSaiPolicerAttributes_t* attributes)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_POLICER_ATTR_METER_TYPE:
                {
                    attributes->meterType = attr_list[count].value;
                    break;
                }
            case SAI_POLICER_ATTR_MODE:
                {
                    attributes->mode = attr_list[count].value;
                    break;
                }
            case SAI_POLICER_ATTR_COLOR_SOURCE:
                {
                    attributes->colorSource = attr_list[count].value;
                    break;
                }
            case SAI_POLICER_ATTR_CBS:
                {
                    attributes->cbs = attr_list[count].value;
                    break;
                }
            case SAI_POLICER_ATTR_CIR:
                {
                    attributes->cir = attr_list[count].value;
                    break;
                }
            case SAI_POLICER_ATTR_PBS:
                {
                    attributes->pbs = attr_list[count].value;
                    break;
                }
            case SAI_POLICER_ATTR_PIR:
                {
                    attributes->pir = attr_list[count].value;
                    break;
                }
            case SAI_POLICER_ATTR_GREEN_PACKET_ACTION:
                {
                    attributes->greenPacketAction = attr_list[count].value;
                    break;
                }
            case SAI_POLICER_ATTR_YELLOW_PACKET_ACTION:
                {
                    attributes->yellowPacketAction = attr_list[count].value;
                    break;
                }
            case SAI_POLICER_ATTR_RED_PACKET_ACTION:
                {
                    attributes->redPacketAction = attr_list[count].value;
                    break;
                }
            case SAI_POLICER_ATTR_ENABLE_COUNTER_PACKET_ACTION_LIST:
                {
                    attributes->enableCounterList.s32list.count =
                        attr_list[count].value.s32list.count;

                    /* When this API is used for verify purpose,
                       out parameter is not allocated. This check will
                       help to know the purpose of this API */
                    if (attributes->enableCounterList.s32list.list)
                    {
                        memcpy(attributes->enableCounterList.s32list.list,
                               attr_list[count].value.s32list.list,
                               sizeof(int32_t) * attr_list[count].value.s32list.count);
                    }
                    else
                    {
                        /* User memory is copied to verify structure */
                        attributes->enableCounterList = attr_list[count].value;
                    }
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiUpdateAttrListPolicerVals

void xpSaiUpdateAttrListPolicerVals(const xpSaiPolicerAttributes_t* attributes,
                                    const uint32_t attr_count, sai_attribute_t* attr_list)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_POLICER_ATTR_METER_TYPE:
                {
                    attr_list[count].value = attributes->meterType;
                    break;
                }
            case SAI_POLICER_ATTR_MODE:
                {
                    attr_list[count].value = attributes->mode;
                    break;
                }
            case SAI_POLICER_ATTR_COLOR_SOURCE:
                {
                    attr_list[count].value = attributes->colorSource;
                    break;
                }
            case SAI_POLICER_ATTR_CBS:
                {
                    attr_list[count].value = attributes->cbs;
                    break;
                }
            case SAI_POLICER_ATTR_CIR:
                {
                    attr_list[count].value = attributes->cir;
                    break;
                }
            case SAI_POLICER_ATTR_PBS:
                {
                    attr_list[count].value = attributes->pbs;
                    break;
                }
            case SAI_POLICER_ATTR_PIR:
                {
                    attr_list[count].value = attributes->pir;
                    break;
                }
            case SAI_POLICER_ATTR_GREEN_PACKET_ACTION:
                {
                    attr_list[count].value = attributes->greenPacketAction;
                    break;
                }
            case SAI_POLICER_ATTR_YELLOW_PACKET_ACTION:
                {
                    attr_list[count].value = attributes->yellowPacketAction;
                    break;
                }
            case SAI_POLICER_ATTR_RED_PACKET_ACTION:
                {
                    attr_list[count].value = attributes->redPacketAction;
                    break;
                }
            case SAI_POLICER_ATTR_ENABLE_COUNTER_PACKET_ACTION_LIST:
                {
                    if (attributes->enableCounterList.s32list.count)
                    {
                        attr_list[count].value.s32list.count =
                            attributes->enableCounterList.s32list.count;
                        memcpy(attr_list[count].value.s32list.list,
                               attributes->enableCounterList.s32list.list,
                               (sizeof(int32_t) * attributes->enableCounterList.s32list.count));
                    }
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiSetPolicerAttrEnableCounterList

sai_status_t xpSaiSetPolicerAttrEnableCounterList(sai_object_id_t policer_oid,
                                                  sai_attribute_value_t value)
{
    xpSaiPolicer_t    policerInfoKey;
    xpSaiPolicer_t    *pPolicerInfo = NULL;
    sai_status_t      saiRetVal     = SAI_STATUS_SUCCESS;
    XP_STATUS         xpsRetVal     = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiSetPolicerAttrEnableCounterList\n");

    policerInfoKey.policerId = policer_oid;

    /* Retrieve the corresponding state from sai policer db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, policerDbHandle,
                                        (xpsDbKey_t)&policerInfoKey, (void**)&pPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the data |Error: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    if (!pPolicerInfo)
    {
        XP_SAI_LOG_DBG("Sai policer entry not found\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (!xpSaiIsCounterListValid(value))
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }

    if (!pPolicerInfo->attrInfo.enableCounterList.s32list.list)
    {
        XP_SAI_LOG_DBG("No memory allocated for Counter list");
        return SAI_STATUS_NO_MEMORY;
    }
    if (value.s32list.count)
    {
        /* Only SW maintaining of enableCounter list, statistics will be retreived
           based on state DB stored value */
        memcpy(pPolicerInfo->attrInfo.enableCounterList.s32list.list,
               value.s32list.list,
               (sizeof(int32_t) * value.s32list.count));
    }

    pPolicerInfo->attrInfo.enableCounterList.s32list.count = value.s32list.count;

    return saiRetVal;
}

//Func: xpSaiCreatePolicer

sai_status_t xpSaiCreatePolicer(sai_object_id_t *policer_id,
                                sai_object_id_t switch_id,
                                uint32_t attr_count, const sai_attribute_t *attr_list)
{
    xpSaiPolicerAttributes_t    attributes;
    xpSaiPolicer_t             *pPolicerInfo = NULL;
    sai_uint32_t                policerId;
    sai_status_t                saiRetVal;
    XP_STATUS                   xpsRetVal;
    xpsDevice_t                 devId = xpSaiGetDevId();

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               POLICER_VALIDATION_ARRAY_SIZE, policer_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    xpSaiSetDefaultPolicerAttributeVals(&attributes);
    xpSaiUpdatePolicerAttributeVals(attr_count, attr_list, &attributes);

    if (attributes.enableCounterList.s32list.count)
    {
        if (!xpSaiIsCounterListValid(attributes.enableCounterList))
        {
            XP_SAI_LOG_ERR("Policer count error\n");
            return SAI_STATUS_SUCCESS;
        }
    }

    /* Create the sai policer id */
    if ((saiRetVal = xpSaiPolicerCreate(&policerId)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to Create Policer Id |Error: %d\n", saiRetVal);
        return saiRetVal;
    }

    /* Create policer object id */
    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_POLICER, devId, (sai_uint64_t)policerId,
                         policer_id) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI Policer object could not be created\n");
        return SAI_STATUS_FAILURE;
    }

    /* Create a new structure for sai policer info */
    if ((xpsRetVal = xpsStateHeapMalloc(sizeof(xpSaiPolicer_t),
                                        (void**)&pPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to allocate memory\n");
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    memset(pPolicerInfo, 0x0, sizeof(xpSaiPolicer_t));

    /* Initialize the allocated entry */
    pPolicerInfo->xpDevId   = xpSaiObjIdSwitchGet(switch_id);
    pPolicerInfo->policerId = *policer_id;
    xpSaiSetDefaultPolicerAttributeVals(&pPolicerInfo->attrInfo);

    /* Allocating memory for state DB counter list, allocating with maximum
       number of packet actions supported, Currenlty its 2 */
    pPolicerInfo->attrInfo.enableCounterList.s32list.count = 0;
    pPolicerInfo->attrInfo.enableCounterList.s32list.list =
        (int32_t *)xpMalloc(sizeof(int32_t) * (SAI_PACKET_ACTION_FORWARD + 1));

    if (NULL == pPolicerInfo->attrInfo.enableCounterList.s32list.list)
    {
        XP_SAI_LOG_DBG("Failed to allocate memory for Counter list");
        xpFree((void*)pPolicerInfo->attrInfo.enableCounterList.s32list.list);
        xpsStateHeapFree((void*)pPolicerInfo);
        return SAI_STATUS_NO_MEMORY;
    }

    xpSaiUpdatePolicerAttributeVals(attr_count, attr_list, &pPolicerInfo->attrInfo);

    /* Insert the state into the database */
    if ((xpsRetVal = xpsStateInsertData(XP_SCOPE_DEFAULT, policerDbHandle,
                                        (void*)pPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to insert data into Sai Policer Db |Error: %d\n",
                       xpsRetVal);
        xpFree((void*)pPolicerInfo->attrInfo.enableCounterList.s32list.list);
        xpsStateHeapFree((void*)pPolicerInfo);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemovePolicer

sai_status_t xpSaiRemovePolicer(sai_object_id_t policer_id)
{
    xpSaiPolicer_t *pPolicerInfo = NULL;
    xpSaiPolicer_t policerInfoKey;
    sai_uint32_t policerId;
    sai_status_t saiRetVal;
    XP_STATUS xpsRetVal;

    policerInfoKey.policerId = policer_id;
    /* Retrieve the corresponding state from sai policer db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, policerDbHandle,
                                        (xpsDbKey_t)&policerInfoKey, (void**)&pPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the data |Error: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    if (!pPolicerInfo)
    {
        XP_SAI_LOG_DBG("Sai policer entry not found\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    if (pPolicerInfo->numOfIdx > 0)
    {
        XP_SAI_LOG_ERR("Policer objest is in use, unmap before remove\n");
        return SAI_STATUS_OBJECT_IN_USE;
    }

    xpFree(pPolicerInfo->attrInfo.enableCounterList.s32list.list);
    pPolicerInfo->attrInfo.enableCounterList.s32list.list =NULL;

    /* Delete the corresponding state from sai policer db */
    if ((xpsRetVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, policerDbHandle,
                                        (xpsDbKey_t)&policerInfoKey, (void**)&pPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to delete sai policer db |Error: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    /* Free the memory allocated for the corresponding state */
    if ((xpsRetVal = xpsStateHeapFree(pPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Freeing allocated memory failed |Error: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Get the policer id value */
    policerId = (sai_uint32_t)xpSaiObjIdValueGet(policer_id);

    /* Remove the policer */
    if ((saiRetVal = xpSaiPolicerRemove(policerId)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to Remove SAI Policer %d |Error: %d\n", policerId,
                       saiRetVal);
        return saiRetVal;
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetPolicerAttribute

sai_status_t xpSaiSetPolicerAttribute(sai_object_id_t policer_id,
                                      const sai_attribute_t *attr)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsPolicerEntry_t xpsPolicerEntry;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpSaiPolicer_t *pPolicerInfo = NULL;
    xpSaiPolicer_t *policerInfoKey = NULL;

    memset(&xpsPolicerEntry, 0, sizeof(xpsPolicerEntry));

    xpsDevId = xpSaiGetDevId();

    policerInfoKey = (xpSaiPolicer_t *)xpMalloc(sizeof(xpSaiPolicer_t));
    if (!policerInfoKey)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for policerInfoKey\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(policerInfoKey, 0, sizeof(xpSaiPolicer_t));

    saiRetVal = xpSaiAttrCheck(1, attr,
                               POLICER_VALIDATION_ARRAY_SIZE, policer_attribs,
                               SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        xpFree(policerInfoKey);
        return saiRetVal;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(policer_id, SAI_OBJECT_TYPE_POLICER))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(policer_id));
        xpFree(policerInfoKey);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    policerInfoKey->policerId = policer_id;
    /* Retrieve the corresponding state from sai policer db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, policerDbHandle,
                                        (xpsDbKey_t)policerInfoKey, (void**)&pPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the data |Error: %d\n", xpsRetVal);
        xpFree(policerInfoKey);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    if (!pPolicerInfo)
    {
        XP_SAI_LOG_DBG("Sai policer entry not found\n");
        xpFree(policerInfoKey);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    switch (attr->id)
    {
        case SAI_POLICER_ATTR_CBS:
            {
                pPolicerInfo->attrInfo.cbs = attr->value;
                break;
            }
        case SAI_POLICER_ATTR_CIR:
            {
                pPolicerInfo->attrInfo.cir = attr->value;
                break;
            }
        case SAI_POLICER_ATTR_PBS:
            {
                pPolicerInfo->attrInfo.pbs = attr->value;
                break;
            }
        case SAI_POLICER_ATTR_PIR:
            {
                pPolicerInfo->attrInfo.pir = attr->value;
                break;
            }
        case SAI_POLICER_ATTR_GREEN_PACKET_ACTION:
            {
                pPolicerInfo->attrInfo.greenPacketAction = attr->value;
                break;
            }
        case SAI_POLICER_ATTR_YELLOW_PACKET_ACTION:
            {
                pPolicerInfo->attrInfo.yellowPacketAction = attr->value;
                break;
            }
        case SAI_POLICER_ATTR_RED_PACKET_ACTION:
            {
                pPolicerInfo->attrInfo.redPacketAction = attr->value;
                break;
            }
        case SAI_POLICER_ATTR_ENABLE_COUNTER_PACKET_ACTION_LIST:
            {
                saiRetVal = xpSaiSetPolicerAttrEnableCounterList(policer_id, attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_POLICER_ATTR_ENABLE_COUNTER_PACKET_ACTION_LIST)\n");
                    xpFree(policerInfoKey);
                    return saiRetVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attibute type %d\n", attr->id);
                xpFree(policerInfoKey);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    saiRetVal = xpSaiPolicerGetXpsEntry(policer_id, &xpsPolicerEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiPolicerGetXpsEntry failed: status=%i.\n", saiRetVal);
        xpFree(policerInfoKey);
        return saiRetVal;
    }

    /* Call XPS Policer Add API to update the values */
    for (uint32_t count = 0; count < pPolicerInfo->numOfIdx; count++)
    {
        if ((xpsRetVal = xpsPolicerAddEntry(xpsDevId,
                                            pPolicerInfo->indexList[count].client, pPolicerInfo->indexList[count].index,
                                            pPolicerInfo->indexList[count].index,
                                            &xpsPolicerEntry)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to add policer entry |Error: %d\n", xpsRetVal);
            xpFree(policerInfoKey);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }

    xpFree(policerInfoKey);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetPolicerAttribute

sai_status_t xpSaiGetPolicerAttribute(sai_object_id_t policer_id,
                                      uint32_t attr_count, sai_attribute_t *attr_list)
{
    xpSaiPolicer_t *pPolicerInfo = NULL;
    xpSaiPolicer_t policerInfoKey;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    sai_status_t            saiRetVal;

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               POLICER_VALIDATION_ARRAY_SIZE, policer_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(policer_id, SAI_OBJECT_TYPE_POLICER))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(policer_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    policerInfoKey.policerId = policer_id;
    /* Retrieve the corresponding state from sai policer db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, policerDbHandle,
                                        (xpsDbKey_t)&policerInfoKey, (void**)&pPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the data |Error: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    if (!pPolicerInfo)
    {
        XP_SAI_LOG_DBG("Sai policer entry not found\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    xpSaiUpdateAttrListPolicerVals(&pPolicerInfo->attrInfo, attr_count, attr_list);

    return SAI_STATUS_SUCCESS;
}

XP_STATUS xpSaiGetPolicerInfo(sai_object_id_t policer_oid,
                              xpSaiPolicer_t **pPolicerInfo)
{
    xpSaiPolicer_t  policerInfoKey;
    XP_STATUS retVal = XP_NO_ERR;

    policerInfoKey.policerId = policer_oid;
    /* Retrieve the corresponding state from sai policer db */
    if ((retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, policerDbHandle,
                                     (xpsDbKey_t)&policerInfoKey, (void**)pPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai policer data |Error: %d\n",
                       retVal);
        return retVal;
    }
    if (!pPolicerInfo)
    {
        XP_SAI_LOG_ERR("Sai policer entry not found\n");
        return XP_ERR_NOT_FOUND;
    }

    return XP_NO_ERR;
}

//Func: xpSaiPolicerStatisticsStoreDB
sai_status_t xpSaiPolicerStatisticsStoreDB(sai_object_id_t policer_oid,
                                           xpSaiPolicer_t **ppPolicerInfo)
{
    XP_STATUS       xpStatus      = XP_NO_ERR;
    xpsPolicerCounterEntry_t entry;
    xpSaiPolicer_t  *pPolicerInfo = NULL;
    bool            countDropPkts    = false;
    bool            countForwardPkts = false;
    uint64_t        redCounts = 0;
    uint64_t        yellowCounts = 0;
    uint64_t        greenCounts = 0;

    if ((xpStatus = xpSaiGetPolicerInfo(policer_oid, &pPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get sai policer info |retVal: %d\n",
                       xpStatus);
        return xpStatus;
    }

    if (!pPolicerInfo)
    {
        XP_SAI_LOG_ERR("Sai policer entry not found\n");
        return XP_ERR_NOT_FOUND;
    }

    *ppPolicerInfo = pPolicerInfo;
    for (uint16_t count = 0; count < pPolicerInfo->numOfIdx; count++)
    {
        memset(&entry, 0, sizeof(xpsPolicerCounterEntry_t));

        if ((xpStatus = xpsPolicerGetPolicerCounterEntry(pPolicerInfo->xpDevId,
                                                         pPolicerInfo->indexList[count].client,
                                                         pPolicerInfo->indexList[count].index, &entry)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get policer counter |Error: %d\n", xpStatus);
            return xpStatus;
        }
        greenCounts  += entry.greenEvent;
        yellowCounts += entry.yellowEvent;
        redCounts    += entry.redEvent;
    }
    for (uint32_t count = 0;
         count < pPolicerInfo->attrInfo.enableCounterList.s32list.count; count++)
    {
        if (pPolicerInfo->attrInfo.enableCounterList.s32list.list[count] ==
            SAI_PACKET_ACTION_FORWARD)
        {
            countForwardPkts = true;
        }
        if (pPolicerInfo->attrInfo.enableCounterList.s32list.list[count] ==
            SAI_PACKET_ACTION_DROP)
        {
            countDropPkts = true;
        }

    }
    if (countForwardPkts && countDropPkts)
    {
        /* do nothing */
    }
    else
    {
        if (countDropPkts)
        {
            if (pPolicerInfo->attrInfo.greenPacketAction.s32 != SAI_PACKET_ACTION_DROP)
            {
                greenCounts = 0;
            }
            if (pPolicerInfo->attrInfo.yellowPacketAction.s32 != SAI_PACKET_ACTION_DROP)
            {
                yellowCounts = 0;
            }
            if (pPolicerInfo->attrInfo.redPacketAction.s32 != SAI_PACKET_ACTION_DROP)
            {
                redCounts = 0;
            }

        }
        if (countForwardPkts)
        {
            if (pPolicerInfo->attrInfo.greenPacketAction.s32 != SAI_PACKET_ACTION_FORWARD)
            {
                greenCounts = 0;
            }
            if (pPolicerInfo->attrInfo.yellowPacketAction.s32 != SAI_PACKET_ACTION_FORWARD)
            {
                yellowCounts = 0;
            }
            if (pPolicerInfo->attrInfo.redPacketAction.s32 != SAI_PACKET_ACTION_FORWARD)
            {
                redCounts = 0;
            }
        }
    }

    if (pPolicerInfo->attrInfo.meterType.u32 == SAI_METER_TYPE_BYTES)
    {
        pPolicerInfo->attrInfo.greenBytes  += greenCounts;
        pPolicerInfo->attrInfo.yellowBytes += yellowCounts;
        pPolicerInfo->attrInfo.redBytes    += redCounts;
    }
    else
    {
        pPolicerInfo->attrInfo.greenPackets  += greenCounts;
        pPolicerInfo->attrInfo.yellowPackets += yellowCounts;
        pPolicerInfo->attrInfo.redPackets    += redCounts;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiClearPolicerStatsDB(uint32_t number_of_counters,
                                             const sai_stat_id_t *counter_ids,
                                             xpSaiPolicer_t *pPolicerInfo)
{
    sai_status_t  saiStatus        = SAI_STATUS_SUCCESS;

    if ((counter_ids == NULL) || (pPolicerInfo == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t idx = 0; idx < number_of_counters; idx++)
    {
        switch (counter_ids[idx])
        {
            case SAI_POLICER_STAT_PACKETS:
                {
                    pPolicerInfo->attrInfo.greenPackets = 0;
                    pPolicerInfo->attrInfo.yellowPackets = 0;
                    pPolicerInfo->attrInfo.redPackets = 0;
                    break;
                }
            case SAI_POLICER_STAT_ATTR_BYTES:
                {
                    pPolicerInfo->attrInfo.greenBytes = 0;
                    pPolicerInfo->attrInfo.yellowBytes = 0;
                    pPolicerInfo->attrInfo.redBytes = 0;
                    break;
                }
            case SAI_POLICER_STAT_GREEN_PACKETS:
                {
                    pPolicerInfo->attrInfo.greenPackets = 0;
                    break;
                }
            case SAI_POLICER_STAT_GREEN_BYTES:
                {
                    pPolicerInfo->attrInfo.greenBytes = 0;
                    break;
                }
            case SAI_POLICER_STAT_YELLOW_PACKETS:
                {
                    pPolicerInfo->attrInfo.yellowPackets = 0;
                    break;
                }
            case SAI_POLICER_STAT_YELLOW_BYTES:
                {
                    pPolicerInfo->attrInfo.yellowBytes = 0;
                    break;
                }
            case SAI_POLICER_STAT_RED_PACKETS:
                {
                    pPolicerInfo->attrInfo.redPackets = 0;
                    break;
                }
            case SAI_POLICER_STAT_RED_BYTES:
                {
                    pPolicerInfo->attrInfo.redBytes = 0;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Undefined attribute %d\n", counter_ids[idx]);
                    saiStatus = SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(idx);
                }
        }
    }
    return saiStatus;
}

static sai_status_t xpSaiGetPolicerStatsInReadClearMode(
    sai_object_id_t policer_oid,
    uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
    uint64_t *counters, bool is_read_clear)
{
    xpSaiPolicer_t           *pPolicerInfo     = NULL;
    sai_status_t              saiStatus        = SAI_STATUS_SUCCESS;

    if ((counter_ids == NULL) || (counters == NULL) || (number_of_counters < 1))
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(policer_oid, SAI_OBJECT_TYPE_POLICER))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(policer_oid));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    saiStatus = xpSaiPolicerStatisticsStoreDB(policer_oid, &pPolicerInfo);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not store the statistics for the policer %" PRIx64".\n",
                       policer_oid);
        return saiStatus;
    }

    for (uint32_t idx = 0; idx < number_of_counters; idx++)
    {
        switch (counter_ids[idx])
        {
            /* get packet count */
            case SAI_POLICER_STAT_PACKETS:
                {
                    counters[idx] = pPolicerInfo->attrInfo.greenPackets +
                                    pPolicerInfo->attrInfo.yellowPackets + pPolicerInfo->attrInfo.redPackets;
                    break;
                }
            /* get byte count */
            case SAI_POLICER_STAT_ATTR_BYTES:
                {
                    counters[idx] = pPolicerInfo->attrInfo.greenBytes +
                                    pPolicerInfo->attrInfo.yellowBytes + pPolicerInfo->attrInfo.redBytes;
                    break;
                }
            /* get green packets count */
            case SAI_POLICER_STAT_GREEN_PACKETS:
                {
                    counters[idx] = pPolicerInfo->attrInfo.greenPackets;
                    break;
                }
            /* get green bytes count */
            case SAI_POLICER_STAT_GREEN_BYTES:
                {
                    counters[idx] = pPolicerInfo->attrInfo.greenBytes;
                    break;
                }
            /* get yellow packets count */
            case SAI_POLICER_STAT_YELLOW_PACKETS:
                {
                    counters[idx] = pPolicerInfo->attrInfo.yellowPackets;
                    break;
                }
            /* get yellow bytes count */
            case SAI_POLICER_STAT_YELLOW_BYTES:
                {
                    counters[idx] = pPolicerInfo->attrInfo.yellowBytes;
                    break;
                }
            /* get red packet count */
            case SAI_POLICER_STAT_RED_PACKETS:
                {
                    counters[idx] = pPolicerInfo->attrInfo.redPackets;
                    break;
                }
            /* get red bytes count */
            case SAI_POLICER_STAT_RED_BYTES:
                {
                    counters[idx] = pPolicerInfo->attrInfo.redBytes;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Undefined attribute %d\n", counter_ids[idx]);
                    saiStatus = SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(idx);
                }
        }
    }

    if (is_read_clear)
    {
        xpSaiClearPolicerStatsDB(number_of_counters, counter_ids, pPolicerInfo);
    }

    return saiStatus;
}

//Func: xpSaiGetPolicerStats
sai_status_t xpSaiGetPolicerStats(sai_object_id_t policer_oid,
                                  uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                  uint64_t *counters)
{
    return xpSaiGetPolicerStatsInReadClearMode(policer_oid, number_of_counters,
                                               counter_ids,
                                               counters, false);
}

//Func: xpSaiGetPolicerStats
sai_status_t xpSaiGetPolicerStatsExt(sai_object_id_t policer_oid,
                                     uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                     sai_stats_mode_t mode, uint64_t *counters)
{
    return xpSaiGetPolicerStatsInReadClearMode(policer_oid, number_of_counters,
                                               counter_ids,
                                               counters, (mode == SAI_STATS_MODE_READ_AND_CLEAR));
}

//Func: xpSaiClearPolicerStats

sai_status_t xpSaiClearPolicerStats(sai_object_id_t policer_oid,
                                    uint32_t number_of_counters, const sai_stat_id_t *counter_ids)
{
    xpSaiPolicer_t           *pPolicerInfo     = NULL;
    sai_status_t              saiStatus        = SAI_STATUS_SUCCESS;

    if ((counter_ids == NULL) || (number_of_counters < 1))
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(policer_oid, SAI_OBJECT_TYPE_POLICER))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(policer_oid));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    saiStatus = xpSaiPolicerStatisticsStoreDB(policer_oid, &pPolicerInfo);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not store the statistics for the policer %" PRIx64 ".\n",
                       policer_oid);
        return saiStatus;
    }

    saiStatus = xpSaiClearPolicerStatsDB(number_of_counters, counter_ids,
                                         pPolicerInfo);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not clear the statistics for the DB.\n");
        return saiStatus;
    }

    return saiStatus;
}


//Func: xpSaiPolicerInit

XP_STATUS xpSaiPolicerInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    xpsRetVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_POLICER,
                                            XP_SAI_POLICER_MAX_IDS, XP_SAI_POLICER_RANGE_START);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI Policer ID allocator\n");
        return xpsRetVal;
    }

    /* Create a Global Sai Policer Db */
    xpsRetVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Policer Db", XPS_GLOBAL,
                                   &xpSaiPolicerKeyComp, policerDbHandle);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to Register Sai Policer Db |Error: %d\n", xpsRetVal);
        return xpsRetVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiPolicerDeInit

XP_STATUS xpSaiPolicerDeInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    /* Remove Global Sai Policer Db */
    if ((xpsRetVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                          &policerDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to Deregister Sai Policer Db |Error: %d\n", xpsRetVal);
        return xpsRetVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiPolicerApiInit

XP_STATUS xpSaiPolicerApiInit(uint64_t flag,
                              const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiPolicerApiInit\n");

    _xpSaiPolicerApi = (sai_policer_api_t *)xpMalloc(sizeof(sai_policer_api_t));
    if (NULL == _xpSaiPolicerApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiPolicerApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiPolicerApi->create_policer         = xpSaiCreatePolicer;
    _xpSaiPolicerApi->remove_policer         = xpSaiRemovePolicer;
    _xpSaiPolicerApi->set_policer_attribute  = xpSaiSetPolicerAttribute;
    _xpSaiPolicerApi->get_policer_attribute  = xpSaiGetPolicerAttribute;
    _xpSaiPolicerApi->get_policer_stats = xpSaiGetPolicerStats;

    _xpSaiPolicerApi->get_policer_stats_ext = xpSaiGetPolicerStatsExt;
    _xpSaiPolicerApi->clear_policer_stats = xpSaiClearPolicerStats;

    saiRetVal = xpSaiApiRegister(SAI_API_POLICER, (void*)_xpSaiPolicerApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register policer API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}

//Func: xpSaiPolicerApiDeinit

XP_STATUS xpSaiPolicerApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiPolicerApiDeinit\n");

    xpFree(_xpSaiPolicerApi);
    _xpSaiPolicerApi = NULL;

    return retVal;
}


// TO BE IMPLEMENTED
// Returned success due to undefined symbol error
sai_status_t xpSaiGetPolicerObjectList(uint32_t *object_count,
                                       sai_object_key_t *object_list)
{
    return SAI_STATUS_SUCCESS;
}

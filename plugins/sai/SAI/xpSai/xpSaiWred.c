// xpSaiWred.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiWred.h"
#include "xpSaiValidationArrays.h"
#include "xpSaiProfileMng.h"
#include "cpssHalQos.h"
#include "xpSaiBuffer.h"

XP_SAI_LOG_REGISTER_API(SAI_API_WRED);

sai_wred_api_t* _xpSaiWredApi;
static xpsDbHandle_t xpSaiWredDbHndl = XPSAI_WRED_DB_HNDL;

#define MAX_SIZE_OF_WRED_WORD       (0xFFFFF)
#define MAX_SIZE_OF_WRED_WORD_EBOF  (_8K)

#define PROBABILITY_CONVERT_TO_DESCRETE(_probInCpssFormat,_probInInt) \
    do\
    {\
        if(_probInInt<=35)\
        {\
            _probInCpssFormat = CPSS_PORT_TX_TAIL_DROP_PROBABILITY_25_E;\
        }\
        else if(_probInInt<=60)\
        {\
            _probInCpssFormat = CPSS_PORT_TX_TAIL_DROP_PROBABILITY_50_E;\
        }\
        else if(_probInInt<=80)\
        {\
            _probInCpssFormat = CPSS_PORT_TX_TAIL_DROP_PROBABILITY_73_E;\
        }\
        else if(_probInInt<=95)\
        {\
            _probInCpssFormat = CPSS_PORT_TX_TAIL_DROP_PROBABILITY_89_E;\
        }\
        else\
        {\
           _probInCpssFormat = CPSS_PORT_TX_TAIL_DROP_PROBABILITY_100_E;\
        }\
    }\
    while(0);


#define SAI_PROFILE_TO_CPSS_PROFILE_CONVERT(_saiColour,_cpssDpNum)\
   do\
   {\
       if(true == _saiColour##Enable)\
       {\
           pCpssWredInfo->dp##_cpssDpNum##WredAttributes.wredSize = MAX_SIZE_OF_WRED_WORD;\
           pCpssWredInfo->dp##_cpssDpNum##WredAttributes.guaranteedLimit = XPSAI_BYTES_TO_PAGES( _saiColour##MinThreshold);\
           /*pWredInfo->greenMaxThreshold - TBD*/\
           PROBABILITY_CONVERT_TO_DESCRETE(pCpssWredInfo->dp##_cpssDpNum##WredAttributes.probability,_saiColour##DropProb);\
       }\
       else\
       {\
           pCpssWredInfo->dp##_cpssDpNum##WredAttributes.wredSize = 0;\
       }\
   }while(0);\

#define SAI_EBOF_PROFILE_TO_CPSS_PROFILE_CONVERT(_saiColour,_cpssDpNum)\
    do\
{\
    if(true == _saiColour##Enable)\
{\
    pCpssWredInfo->dp##_cpssDpNum##WredAttributes.wredSize = MAX_SIZE_OF_WRED_WORD_EBOF;\
    pCpssWredInfo->dp##_cpssDpNum##WredAttributes.guaranteedLimit = XPSAI_BYTES_TO_PAGES( _saiColour##MinThreshold);\
    /*pWredInfo->greenMaxThreshold - TBD*/\
    PROBABILITY_CONVERT_TO_DESCRETE(pCpssWredInfo->dp##_cpssDpNum##WredAttributes.probability,_saiColour##DropProb);\
}\
       else\
{\
    pCpssWredInfo->dp##_cpssDpNum##WredAttributes.wredSize = 0;\
}\
}while(0);\



//Func: xpSaiWredKeyComp

static sai_int32_t xpSaiWredKeyComp(void *key1, void *key2)
{
    return ((int32_t)(((xpSaiWred_t*)key1)->wredObjId) - (int32_t)(((
                                                                        xpSaiWred_t*)key2)->wredObjId));
}

//Func: xpSaiWredObjIdCreate

sai_status_t xpSaiWredObjIdCreate(xpsDevice_t xpDevId,
                                  sai_uint32_t greenProfileId, sai_uint32_t yellowProfileId,
                                  sai_uint32_t redProfileId, sai_object_id_t* wredObjId)
{
    sai_uint64_t wredId = 0;
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    wredId = wredId | (((sai_uint64_t)(redProfileId)) << XPSAI_WRED_RED_PFL_OFFSET);
    wredId = wredId | (((sai_uint64_t)(yellowProfileId)) <<
                       XPSAI_WRED_YELLOW_PFL_OFFSET);
    wredId = wredId | (((sai_uint64_t)(greenProfileId)) <<
                       XPSAI_WRED_GREEN_PFL_OFFSET);

    /* Create wred object id */
    if ((retVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_WRED, xpDevId, wredId,
                                   wredObjId)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create wred object id |retVal: %d\n", retVal);
        return retVal;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiWredObjIdValueGet

sai_status_t xpSaiWredObjIdValueGet(sai_object_id_t wredObjId,
                                    sai_uint32_t *pGreenProfileId, sai_uint32_t *pYellowProfileId,
                                    sai_uint32_t *pRedProfileId)
{
    if (pGreenProfileId)
    {
        *pGreenProfileId = (sai_uint32_t)((wredObjId & XPSAI_WRED_GREEN_PFL_VAL_MASK) >>
                                          XPSAI_WRED_GREEN_PFL_OFFSET);
    }

    if (pYellowProfileId)
    {
        *pYellowProfileId = (sai_uint32_t)((wredObjId & XPSAI_WRED_YELLOW_PFL_VAL_MASK)
                                           >> XPSAI_WRED_YELLOW_PFL_OFFSET);
    }

    if (pRedProfileId)
    {
        *pRedProfileId = (sai_uint32_t)((wredObjId & XPSAI_WRED_RED_PFL_VAL_MASK) >>
                                        XPSAI_WRED_RED_PFL_OFFSET);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiWredCreateState

XP_STATUS xpSaiWredCreateState(sai_object_id_t wredObjId,
                               xpSaiWred_t **ppWredInfo)
{
    XP_STATUS xpRetVal = XP_NO_ERR;

    /* Allocate memory for wred info */
    if ((xpRetVal = xpsStateHeapMalloc(sizeof(xpSaiWred_t),
                                       (void**)ppWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to allocate memory |retVal: %d\n", xpRetVal);
        return xpRetVal;
    }

    memset(*ppWredInfo, 0x0, sizeof(xpSaiWred_t));

    /* Initialize the allocated entry */
    (*ppWredInfo)->wredObjId = wredObjId;

    /* Insert the state into the database */
    if ((xpRetVal = xpsStateInsertData(XP_SCOPE_DEFAULT, xpSaiWredDbHndl,
                                       (void*)*ppWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to insert the state into database |retVal: %d\n",
                       xpRetVal);
        xpsStateHeapFree((void*)*ppWredInfo);
        return xpRetVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiWredRemoveState

XP_STATUS xpSaiWredRemoveState(sai_object_id_t wredObjId)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;
    xpSaiWred_t  wredInfoKey;

    /* Populate the key */
    wredInfoKey.wredObjId = wredObjId;

    /* Delete the corresponding state from database */
    if ((xpRetVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, xpSaiWredDbHndl,
                                       (xpsDbKey_t)&wredInfoKey, (void**)&pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to delete wred state data |retVal: %d\n", xpRetVal);
        return xpRetVal;
    }

    /* Free the memory allocated for the corresponding state */
    if ((xpRetVal = xpsStateHeapFree(pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to free the wred state memory |retVal: %d\n", xpRetVal);
        return xpRetVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiWredGetState

XP_STATUS xpSaiWredGetState(sai_object_id_t wredObjId, xpSaiWred_t **ppWredInfo)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t  wredInfoKey;

    /* Populate the key */
    wredInfoKey.wredObjId = wredObjId;

    /* Retrieve the corresponding state from database */
    if ((xpRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiWredDbHndl,
                                       (xpsDbKey_t)&wredInfoKey, (void**)ppWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve wred state data |retVal: %d\n", xpRetVal);
        return xpRetVal;
    }

    if (*ppWredInfo == NULL)
    {
        XP_SAI_LOG_ERR("Error: Entry not found |retVal: %d\n", xpRetVal);
        return XP_ERR_KEY_NOT_FOUND;
    }

    return XP_NO_ERR;
}

//Func: xpSaiWredBindQueueToWred

sai_status_t xpSaiWredBindQueueToWred(sai_object_id_t wredObjId,
                                      sai_object_id_t queueObjId)
{
    xpSaiWred_t *pWredInfo = NULL;
    XP_STATUS    xpStatus  = XP_NO_ERR;

    /* Check if wred object exist */
    xpStatus = xpSaiWredGetState(wredObjId, &pWredInfo);
    if (xpStatus != XP_NO_ERR)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    /* For now queueObjId is not used. Just increment the object count.
     * This is needed to validate wred before remove
     * */

    /* Increment the object count */
    pWredInfo->objList[pWredInfo->objCount]=queueObjId;
    pWredInfo->objCount += 1;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiWredUnbindQueueToWred

sai_status_t xpSaiWredUnbindQueueToWred(sai_object_id_t wredObjId,
                                        sai_object_id_t queueObjId)
{
    xpSaiWred_t *pWredInfo = NULL;
    XP_STATUS    xpStatus  = XP_NO_ERR;
    bool         mapObjExist=false;

    /* Check if wred object exist */
    xpStatus = xpSaiWredGetState(wredObjId, &pWredInfo);
    if (xpStatus != XP_NO_ERR)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    /*
     * This is needed to validate wred before remove
     * */


    /* Remove the mapObjId from buffer profile objList */
    for (uint16_t count = 0; count < pWredInfo->objCount; count++)
    {
        if (!mapObjExist)
        {
            if (pWredInfo->objList[count] == queueObjId)
            {
                mapObjExist = 1;
            }
            continue;
        }
        else
        {
            pWredInfo->objList[count-1] = pWredInfo->objList[count];
        }
    }

    /* Decrement the object count */
    pWredInfo->objCount -= 1;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiWredValidateWredObject

sai_status_t xpSaiWredValidateWredObject(sai_object_id_t wredObjId)
{
    xpSaiWred_t *pWredInfo = NULL;
    XP_STATUS    xpStatus  = XP_NO_ERR;

    /* Validate non null Objects */
    if (wredObjId != SAI_NULL_OBJECT_ID)
    {
        /* Check if wred object is valid */
        if (!XDK_SAI_OBJID_TYPE_CHECK(wredObjId, SAI_OBJECT_TYPE_WRED))
        {
            XP_SAI_LOG_ERR("Wrong object type received (%u)\n",
                           xpSaiObjIdTypeGet(wredObjId));
            return SAI_STATUS_INVALID_OBJECT_TYPE;
        }

        /* Check if wred object exist */
        xpStatus = xpSaiWredGetState(wredObjId, &pWredInfo);
        if (xpStatus != XP_NO_ERR)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiWredCreateProfileObject

sai_status_t xpSaiWredCreateProfileObject(xpsDevice_t xpDevId,
                                          sai_object_id_t *pWredObjId)
{
    sai_status_t  saiRetVal       = SAI_STATUS_SUCCESS;
    sai_uint32_t  wredId  = 0;;
    XP_STATUS     xpStatus;

    xpStatus = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XPS_ALLOCATOR_WRED_ID,
                                      &wredId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to allocate wred id, xpStatus: %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }
    /* Create wred object id */
    if ((saiRetVal = xpSaiWredObjIdCreate(xpDevId, wredId, 0, 0,
                                          pWredObjId)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create wred object id |retVal: %d\n", saiRetVal);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiWredRemoveProfileObject

sai_status_t xpSaiWredRemoveProfileObject(xpsDevice_t xpDevId,
                                          sai_object_id_t wredObjId)
{
    XP_STATUS    xpRetVal        = XP_NO_ERR;
    sai_uint32_t greenProfileId  = 0;
    sai_uint32_t yellowProfileId = 0;
    sai_uint32_t redProfileId    = 0;

    /* Get the green, yellow and red Aqm queue profile ids */
    xpSaiWredObjIdValueGet(wredObjId, &greenProfileId, &yellowProfileId,
                           &redProfileId);

    xpRetVal= xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XPS_ALLOCATOR_WRED_ID,
                                    greenProfileId);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to release ingress dynamic pool id, xpStatus: %d\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiSyncWredProfile(xpSaiWred_t  *pWredInfo)
{
    sai_uint32_t i;
    sai_uint32_t portNum, queueNum;
    xpsDevice_t   xpDevId ;
    XP_STATUS     xpRetVal  = XP_NO_ERR;

    for (i=0; i<pWredInfo->objCount; i++)
    {
        XP_SAI_LOG_DBG("Apply WRED obj 0x%08x%08x on queue obj 0x%08x%08x\n",
                       LONG_INT_FORMAT(pWredInfo->wredObjId), LONG_INT_FORMAT(pWredInfo->objList[i]));
        /* Check if wred object is valid */
        if (!XDK_SAI_OBJID_TYPE_CHECK(pWredInfo->objList[i], SAI_OBJECT_TYPE_QUEUE))
        {
            XP_SAI_LOG_ERR("Wrong object type received (%u)\n",
                           xpSaiObjIdTypeGet(pWredInfo->objList[i]));
            return SAI_STATUS_INVALID_OBJECT_TYPE;
        }
        xpDevId = xpSaiObjIdSwitchGet(pWredInfo->objList[i]);
        xpSaiQosQueueObjIdPortValueGet(pWredInfo->objList[i], &portNum, &queueNum);
        if (queueNum >= XPS_MAX_QUEUES_PER_PORT)
        {
            XP_SAI_LOG_ERR("Invalid queueNum :%d \n", queueNum);
            return SAI_STATUS_FAILURE;
        }

        xpRetVal=xpSaiProfileMngConfigureWredAndEcn(xpDevId, portNum, queueNum,
                                                    pWredInfo);
        if (xpRetVal != XP_NO_ERR)
        {
            return xpsStatus2SaiStatus(xpRetVal);
        }
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiConfigureWred

XP_STATUS xpSaiConfigureWred(xpDevice_t xpDevId, uint32_t profileId,
                             uint32_t minTh, uint32_t maxTh,
                             uint32_t weight, uint32_t maxDropProb, xpWredMode_e wredMode)
{
    XP_STATUS xpRetVal = XP_NO_ERR;
    uint32_t minPageTh = 0;
    uint32_t maxPageTh = 0;

    /* Configure WRED mode */
    if ((xpRetVal = xpsQosAqmConfigureWredMode(xpDevId, profileId,
                                               wredMode)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to configure wred mode |retVal: %d\n", xpRetVal);
        return xpRetVal;
    }

    /* Convert thresholds from bytes to pages */
    if (minTh)
    {
        minPageTh = XPSAI_BYTES_TO_PAGES(minTh);
    }
    else
    {
        /* If min threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        minPageTh = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES -
                                         XPSAI_BUFFER_PAGE_SIZE_BYTES);
    }

    if (maxTh)
    {
        maxPageTh = XPSAI_BYTES_TO_PAGES(maxTh);
    }
    else
    {
        /* If max threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        maxPageTh = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES);
    }

    if (minPageTh == maxPageTh)
    {
        minPageTh = minPageTh - 1;
    }

    /* Configure WRED */
    if ((xpRetVal = xpsQosAqmConfigureWred(xpDevId, profileId, minPageTh, maxPageTh,
                                           weight, maxDropProb)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to configure wred parameters |retVal: %d\n", xpRetVal);
        return xpRetVal;
    }

    /* If tail drop threshold is less than wred threshold, queue will tail drop the packets.
     * So set the queue page tail drop threshold for wred profile to max.
     * This is applicable only in XP80A0 */
    if ((xpRetVal = xpsQosAqmConfigureQueuePageTailDropThresholdForProfile(xpDevId,
                                                                           profileId, XPSAI_BUFFER_LENGTH_MAX_THRESHOLD)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to configure queue page tail drop threshold for profile |retVal: %d\n",
                       xpRetVal);
        return xpRetVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiWredGetXpWredMode

XP_STATUS xpSaiWredGetXpWredMode(sai_ecn_mark_mode_t ecnMarkMode,
                                 xpWredMode_e *pGreenWredMode, xpWredMode_e *pYellowWredMode,
                                 xpWredMode_e *pRedWredMode)
{
    xpWredMode_e greenWredMode  = XP_DISABLE_WRED;
    xpWredMode_e yellowWredMode = XP_DISABLE_WRED;
    xpWredMode_e redWredMode    = XP_DISABLE_WRED;

    switch (ecnMarkMode)
    {
        case SAI_ECN_MARK_MODE_NONE:
            {
                /* Disable ECN marking and enable drop for all colors */
                greenWredMode  = XP_ENABLE_WRED_DROP;
                yellowWredMode = XP_ENABLE_WRED_DROP;
                redWredMode    = XP_ENABLE_WRED_DROP;
                break;
            }
        case SAI_ECN_MARK_MODE_GREEN:
            {
                /* Enable ECN marking for green color. Yellow and red are dropped */
                greenWredMode  = XP_ENABLE_WRED_MARK;
                yellowWredMode = XP_ENABLE_WRED_DROP;
                redWredMode    = XP_ENABLE_WRED_DROP;
                break;
            }
        case SAI_ECN_MARK_MODE_YELLOW:
            {
                /* Enable ECN marking for yellow color. Green and red are dropped */
                greenWredMode  = XP_ENABLE_WRED_DROP;
                yellowWredMode = XP_ENABLE_WRED_MARK;
                redWredMode    = XP_ENABLE_WRED_DROP;
                break;
            }
        case SAI_ECN_MARK_MODE_RED:
            {
                /* Enable ECN marking for red color. Green and yellow are dropped */
                greenWredMode  = XP_ENABLE_WRED_DROP;
                yellowWredMode = XP_ENABLE_WRED_DROP;
                redWredMode    = XP_ENABLE_WRED_MARK;
                break;
            }
        case SAI_ECN_MARK_MODE_GREEN_YELLOW:
            {
                /* Enable ECN marking for green and yellow colors. Red is dropped */
                greenWredMode  = XP_ENABLE_WRED_MARK;
                yellowWredMode = XP_ENABLE_WRED_MARK;
                redWredMode    = XP_ENABLE_WRED_DROP;
                break;
            }
        case SAI_ECN_MARK_MODE_GREEN_RED:
            {
                /* Enable ECN marking for green and red colors. Yellow is dropped */
                greenWredMode  = XP_ENABLE_WRED_MARK;
                yellowWredMode = XP_ENABLE_WRED_DROP;
                redWredMode    = XP_ENABLE_WRED_MARK;
                break;
            }
        case SAI_ECN_MARK_MODE_YELLOW_RED:
            {
                /* Enable ECN marking for yellow and red colors. Green is dropped */
                greenWredMode  = XP_ENABLE_WRED_DROP;
                yellowWredMode = XP_ENABLE_WRED_MARK;
                redWredMode    = XP_ENABLE_WRED_MARK;
                break;
            }
        case SAI_ECN_MARK_MODE_ALL:
            {
                /* Enable ECN marking for all colors */
                greenWredMode  = XP_ENABLE_WRED_MARK;
                yellowWredMode = XP_ENABLE_WRED_MARK;
                redWredMode    = XP_ENABLE_WRED_MARK;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Invalid ecn mark mode %d\n", ecnMarkMode);
                return XP_ERR_INVALID_ARG;
            }
    }

    if (pGreenWredMode)
    {
        *pGreenWredMode = greenWredMode;
    }

    if (pYellowWredMode)
    {
        *pYellowWredMode = yellowWredMode;
    }

    if (pRedWredMode)
    {
        *pRedWredMode = redWredMode;
    }

    return XP_NO_ERR;
}

//Func: xpSaiWredValidateAttributeValue

sai_status_t xpSaiWredValidateAttributeValue(sai_attr_id_t attrId,
                                             sai_attribute_value_t attrValue, sai_uint32_t count)
{
    switch (attrId)
    {
        case SAI_WRED_ATTR_GREEN_ENABLE:
        case SAI_WRED_ATTR_YELLOW_ENABLE:
        case SAI_WRED_ATTR_RED_ENABLE:
            {
                /* Validate wred enable */
                int8_t attrValueBooldata = attrValue.booldata;
                if (!XPSAI_WRED_IS_ENABLE_VALID(attrValueBooldata))
                {
                    XP_SAI_LOG_ERR("Error: Invalid enable value %d provided\n", attrValue.booldata);
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        case SAI_WRED_ATTR_GREEN_MIN_THRESHOLD:
        case SAI_WRED_ATTR_GREEN_MAX_THRESHOLD:
        case SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD:
        case SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD:
        case SAI_WRED_ATTR_RED_MIN_THRESHOLD:
        case SAI_WRED_ATTR_RED_MAX_THRESHOLD:
            {
                /* Validate wred threshould */
                if (!XPSAI_WRED_IS_THRESHOLD_VALID(attrValue.u32))
                {
                    XP_SAI_LOG_ERR("Error: Invalid threshould value %d provided\n", attrValue.u32);
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        case SAI_WRED_ATTR_GREEN_DROP_PROBABILITY:
        case SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY:
        case SAI_WRED_ATTR_RED_DROP_PROBABILITY:
            {
                /* Validate wred drop probability */
                if (!XPSAI_WRED_IS_DROP_PROBABILITY_VALID((int32_t)attrValue.u32))
                {
                    XP_SAI_LOG_ERR("Error: Invalid drop probability value %d provided\n",
                                   attrValue.u32);
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        case SAI_WRED_ATTR_WEIGHT:
            {
                /* Validate wred weight */
                if (!XPSAI_WRED_IS_WEIGHT_VALID((int8_t)attrValue.u8))
                {
                    XP_SAI_LOG_ERR("Error: Invalid weight value %d provided\n", attrValue.u8);
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        case SAI_WRED_ATTR_ECN_MARK_MODE:
            {
                /* Validate wred ecn mark mode */
                if (!XPSAI_WRED_IS_ECN_MARK_MODE_VALID(attrValue.s32))
                {
                    XP_SAI_LOG_ERR("Error: Invalid ecn mark mode value %d provided\n",
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

//Func: xpSaiSetDefaultWredAttributeVals

void xpSaiSetDefaultWredAttributeVals(xpSaiWredAttributesT* attributes)
{
    memset(attributes, 0, sizeof(xpSaiWredAttributesT));

    attributes->greenEnable.booldata = false;
    attributes->greenMinThreshold.u32 =
        0; /* default to 0 i.e Maximum buffer size */
    attributes->greenMaxThreshold.u32 =
        0; /* default to 0 i.e Maximum buffer size */
    attributes->greenDropProbability.u32 = 100;

    attributes->yellowEnable.booldata = false;
    attributes->yellowMinThreshold.u32 =
        0; /* default to 0 i.e Maximum buffer size */
    attributes->yellowMaxThreshold.u32 =
        0; /* default to 0 i.e Maximum buffer size */
    attributes->yellowDropProbability.u32 = 100;

    attributes->redEnable.booldata = false;
    attributes->redMinThreshold.u32 = 0; /* default to 0 i.e Maximum buffer size */
    attributes->redMaxThreshold.u32 = 0; /* default to 0 i.e Maximum buffer size */
    attributes->redDropProbability.u32 = 100;

    attributes->weight.u8 = 0;
    attributes->ecnMarkMode.s32 = SAI_ECN_MARK_MODE_NONE;
}

//Func: xpSaiUpdateWredAttributeVals

void xpSaiUpdateWredAttributeVals(const uint32_t attr_count,
                                  const sai_attribute_t* attr_list, xpSaiWredAttributesT* attributes)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_WRED_ATTR_GREEN_ENABLE:
                {
                    attributes->greenEnable = attr_list[count].value;
                    break;
                }
            case SAI_WRED_ATTR_GREEN_MIN_THRESHOLD:
                {
                    attributes->greenMinThreshold = attr_list[count].value;
                    break;
                }
            case SAI_WRED_ATTR_GREEN_MAX_THRESHOLD:
                {
                    attributes->greenMaxThreshold = attr_list[count].value;
                    break;
                }
            case SAI_WRED_ATTR_GREEN_DROP_PROBABILITY:
                {
                    attributes->greenDropProbability = attr_list[count].value;
                    break;
                }
            case SAI_WRED_ATTR_YELLOW_ENABLE:
                {
                    attributes->yellowEnable = attr_list[count].value;
                    break;
                }
            case SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD:
                {
                    attributes->yellowMinThreshold = attr_list[count].value;
                    break;
                }
            case SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD:
                {
                    attributes->yellowMaxThreshold = attr_list[count].value;
                    break;
                }
            case SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY:
                {
                    attributes->yellowDropProbability = attr_list[count].value;
                    break;
                }
            case SAI_WRED_ATTR_RED_ENABLE:
                {
                    attributes->redEnable = attr_list[count].value;
                    break;
                }
            case SAI_WRED_ATTR_RED_MIN_THRESHOLD:
                {
                    attributes->redMinThreshold = attr_list[count].value;
                    break;
                }
            case SAI_WRED_ATTR_RED_MAX_THRESHOLD:
                {
                    attributes->redMaxThreshold = attr_list[count].value;
                    break;
                }
            case SAI_WRED_ATTR_RED_DROP_PROBABILITY:
                {
                    attributes->redDropProbability = attr_list[count].value;
                    break;
                }
            case SAI_WRED_ATTR_WEIGHT:
                {
                    attributes->weight = attr_list[count].value;
                    break;
                }
            case SAI_WRED_ATTR_ECN_MARK_MODE:
                {
                    attributes->ecnMarkMode = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Unknown attribute %d\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiUpdateAttrListWredVals

void xpSaiUpdateAttrListWredVals(const xpSaiWredAttributesT* attributes,
                                 const uint32_t attr_count, sai_attribute_t* attr_list)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_WRED_ATTR_GREEN_ENABLE:
                {
                    attr_list[count].value = attributes->greenEnable;
                    break;
                }
            case SAI_WRED_ATTR_GREEN_MIN_THRESHOLD:
                {
                    attr_list[count].value = attributes->greenMinThreshold;
                    break;
                }
            case SAI_WRED_ATTR_GREEN_MAX_THRESHOLD:
                {
                    attr_list[count].value = attributes->greenMaxThreshold;
                    break;
                }
            case SAI_WRED_ATTR_GREEN_DROP_PROBABILITY:
                {
                    attr_list[count].value = attributes->greenDropProbability;
                    break;
                }
            case SAI_WRED_ATTR_YELLOW_ENABLE:
                {
                    attr_list[count].value = attributes->yellowEnable;
                    break;
                }
            case SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD:
                {
                    attr_list[count].value = attributes->yellowMinThreshold;
                    break;
                }
            case SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD:
                {
                    attr_list[count].value = attributes->yellowMaxThreshold;
                    break;
                }
            case SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY:
                {
                    attr_list[count].value = attributes->yellowDropProbability;
                    break;
                }
            case SAI_WRED_ATTR_RED_ENABLE:
                {
                    attr_list[count].value = attributes->redEnable;
                    break;
                }
            case SAI_WRED_ATTR_RED_MIN_THRESHOLD:
                {
                    attr_list[count].value = attributes->redMinThreshold;
                    break;
                }
            case SAI_WRED_ATTR_RED_MAX_THRESHOLD:
                {
                    attr_list[count].value = attributes->redMaxThreshold;
                    break;
                }
            case SAI_WRED_ATTR_RED_DROP_PROBABILITY:
                {
                    attr_list[count].value = attributes->redDropProbability;
                    break;
                }
            case SAI_WRED_ATTR_WEIGHT:
                {
                    attr_list[count].value = attributes->weight;
                    break;
                }
            case SAI_WRED_ATTR_ECN_MARK_MODE:
                {
                    attr_list[count].value = attributes->ecnMarkMode;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Unknown attribute %d\n", attr_list[count].id);
                }
        }
    }
}

static sai_status_t xpSaiSetWredAttrColourEnable(sai_object_id_t wredObjId,
                                                 sai_attribute_value_t value, sai_uint32_t colour)
{
    XP_STATUS     xpRetVal  = XP_NO_ERR;
    xpSaiWred_t  *pWredInfo = NULL;;
    bool          *enablePtr = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    switch (colour)
    {
        case XPSAI_WRED_COLOUR_GREEN:
            enablePtr = &pWredInfo->greenEnable;
            break;
        case XPSAI_WRED_COLOUR_YELLOW:
            enablePtr = &pWredInfo->yellowEnable;
            break;
        case XPSAI_WRED_COLOUR_RED:
            enablePtr = &pWredInfo->redEnable;
            break;
    }

    if ((enablePtr) && (*enablePtr != value.booldata))
    {
        /* Update the state database */
        *enablePtr = value.booldata;
        return xpSaiSyncWredProfile(pWredInfo);
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiSetWredAttrGreenEnable

sai_status_t xpSaiSetWredAttrGreenEnable(sai_object_id_t wredObjId,
                                         sai_attribute_value_t value)
{

    return xpSaiSetWredAttrColourEnable(wredObjId, value, XPSAI_WRED_COLOUR_GREEN);

}

//Func: xpSaiGetWredAttrGreenEnable

sai_status_t xpSaiGetWredAttrGreenEnable(sai_object_id_t wredObjId,
                                         sai_attribute_value_t* value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    value->booldata = pWredInfo->greenEnable;

    return SAI_STATUS_SUCCESS;
}


static sai_status_t xpSaiSetWredAttrColourMinThreshold(sai_object_id_t
                                                       wredObjId,
                                                       sai_attribute_value_t value,
                                                       sai_uint32_t colour)
{
    XP_STATUS     xpRetVal  = XP_NO_ERR;
    xpSaiWred_t  *pWredInfo = NULL;
    sai_uint32_t  *minThresholdPtr = NULL;  /* Min threshold for  color in bytes */
    sai_uint32_t  *maxThresholdPtr = NULL;  /* Max threshold for  color in bytes */

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    switch (colour)
    {
        case XPSAI_WRED_COLOUR_GREEN:
            minThresholdPtr = &pWredInfo->greenMinThreshold;
            maxThresholdPtr = &pWredInfo->greenMaxThreshold;
            break;
        case XPSAI_WRED_COLOUR_YELLOW:
            minThresholdPtr = &pWredInfo->yellowMinThreshold;
            maxThresholdPtr = &pWredInfo->yellowMaxThreshold;
            break;
        case XPSAI_WRED_COLOUR_RED:
            minThresholdPtr = &pWredInfo->redMinThreshold;
            maxThresholdPtr = &pWredInfo->redMaxThreshold;
            break;
    }


    /* Convert threshold from bytes to pages */
    sai_uint32_t minThresholdInPages = 0;
    sai_uint32_t maxThresholdInPages = 0;

    if (value.u32)
    {
        minThresholdInPages = XPSAI_BYTES_TO_PAGES(value.u32);
    }
    else
    {
        /* If min threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        minThresholdInPages = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES);
    }

    if ((maxThresholdPtr) && (*maxThresholdPtr))
    {
        maxThresholdInPages = XPSAI_BYTES_TO_PAGES(*maxThresholdPtr);
    }
    else
    {
        /* If max threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        maxThresholdInPages = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES);
    }

    if (minThresholdInPages == maxThresholdInPages)
    {
        minThresholdInPages = minThresholdInPages - 1;
    }

    if ((minThresholdPtr) && (*minThresholdPtr != value.u32))
    {
        if (minThresholdInPages < maxThresholdInPages)
        {
            /* Update the state database */
            *minThresholdPtr = value.u32;

            return xpSaiSyncWredProfile(pWredInfo);
        }
        else
        {
            XP_SAI_LOG_ERR("Min Threshold is greater than Max Threshold. Hence not applying in hw \n");
        }
    }
    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiSetWredAttrGreenMinThreshold

sai_status_t xpSaiSetWredAttrGreenMinThreshold(sai_object_id_t wredObjId,
                                               sai_attribute_value_t value)
{
    return xpSaiSetWredAttrColourMinThreshold(wredObjId, value,
                                              XPSAI_WRED_COLOUR_GREEN);
}

//Func: xpSaiGetWredAttrGreenMinThreshold

sai_status_t xpSaiGetWredAttrGreenMinThreshold(sai_object_id_t wredObjId,
                                               sai_attribute_value_t* value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    value->u32 = pWredInfo->greenMinThreshold;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetWredAttrGreenMaxThreshold

sai_status_t xpSaiSetWredAttrGreenMaxThreshold(sai_object_id_t wredObjId,
                                               sai_attribute_value_t value)
{
    XP_STATUS     xpRetVal  = XP_NO_ERR;
    xpSaiWred_t  *pWredInfo = NULL;
    sai_uint32_t  profileId = 0;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Get the Aqm Queue Green profile Id */
    xpSaiWredObjIdValueGet(wredObjId, &profileId, NULL, NULL);

    /* Convert threshold from bytes to pages */
    sai_uint32_t minThreshold = 0;
    sai_uint32_t maxThreshold = 0;

    if (pWredInfo->greenMinThreshold)
    {
        minThreshold = XPSAI_BYTES_TO_PAGES(pWredInfo->greenMinThreshold);
    }
    else
    {
        /* If min threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        minThreshold = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES -
                                            XPSAI_BUFFER_PAGE_SIZE_BYTES);
    }

    if (value.u32)
    {
        maxThreshold = XPSAI_BYTES_TO_PAGES(value.u32);
    }
    else
    {
        /* If max threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        maxThreshold = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES);
    }

    if (minThreshold == maxThreshold)
    {
        minThreshold = minThreshold - 1;
    }

    /* Write the wred configuration */
    if ((xpRetVal = xpsQosAqmConfigureWred(pWredInfo->xpDevId, profileId,
                                           minThreshold, maxThreshold, pWredInfo->weight,
                                           pWredInfo->greenDropProb)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to configure wred parameters |retVal: %d\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Update the state database */
    pWredInfo->greenMaxThreshold = value.u32;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetWredAttrGreenMaxThreshold

sai_status_t xpSaiGetWredAttrGreenMaxThreshold(sai_object_id_t wredObjId,
                                               sai_attribute_value_t* value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    value->u32 = pWredInfo->greenMaxThreshold;

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiSetWredAttrGreenDropProbability

sai_status_t xpSaiSetWredAttrColourDropProbability
(
    sai_object_id_t wredObjId,
    sai_attribute_value_t value,
    sai_uint32_t colour
)
{
    XP_STATUS     xpRetVal  = XP_NO_ERR;
    xpSaiWred_t  *pWredInfo = NULL;
    sai_uint32_t  *dropProbPtr = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }


    switch (colour)
    {
        case XPSAI_WRED_COLOUR_GREEN:
            dropProbPtr = &pWredInfo->greenDropProb;
            break;
        case XPSAI_WRED_COLOUR_YELLOW:
            dropProbPtr = &pWredInfo->yellowDropProb;
            break;
        case XPSAI_WRED_COLOUR_RED:
            dropProbPtr = &pWredInfo->redDropProb;
            break;
    }

    if ((dropProbPtr) && (*dropProbPtr != value.u32))
    {
        /* Update the state database */
        pWredInfo->greenDropProb = value.u32;
        return xpSaiSyncWredProfile(pWredInfo);
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiSetWredAttrGreenDropProbability

sai_status_t xpSaiSetWredAttrGreenDropProbability(sai_object_id_t wredObjId,
                                                  sai_attribute_value_t value)
{
    return xpSaiSetWredAttrColourDropProbability(wredObjId, value,
                                                 XPSAI_WRED_COLOUR_GREEN);
}

//Func: xpSaiGetWredAttrGreenDropProbability

sai_status_t xpSaiGetWredAttrGreenDropProbability(sai_object_id_t wredObjId,
                                                  sai_attribute_value_t* value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    value->u32 = pWredInfo->greenDropProb;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetWredAttrYellowEnable

sai_status_t xpSaiSetWredAttrYellowEnable(sai_object_id_t wredObjId,
                                          sai_attribute_value_t value)
{

    return xpSaiSetWredAttrColourEnable(wredObjId, value, XPSAI_WRED_COLOUR_YELLOW);

}


//Func: xpSaiGetWredAttrYellowEnable

sai_status_t xpSaiGetWredAttrYellowEnable(sai_object_id_t wredObjId,
                                          sai_attribute_value_t* value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    value->booldata = pWredInfo->yellowEnable;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetWredAttrYellowMinThreshold

sai_status_t xpSaiSetWredAttrYellowMinThreshold(sai_object_id_t wredObjId,
                                                sai_attribute_value_t value)
{
    return xpSaiSetWredAttrColourMinThreshold(wredObjId, value,
                                              XPSAI_WRED_COLOUR_YELLOW);
}


//Func: xpSaiGetWredAttrYellowMinThreshold

sai_status_t xpSaiGetWredAttrYellowMinThreshold(sai_object_id_t wredObjId,
                                                sai_attribute_value_t* value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    value->u32 = pWredInfo->yellowMinThreshold;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetWredAttrYellowMaxThreshold

sai_status_t xpSaiSetWredAttrYellowMaxThreshold(sai_object_id_t wredObjId,
                                                sai_attribute_value_t value)
{
    XP_STATUS     xpRetVal  = XP_NO_ERR;
    xpSaiWred_t  *pWredInfo = NULL;
    sai_uint32_t  profileId = 0;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Get the Aqm Queue Yellow profile Id */
    xpSaiWredObjIdValueGet(wredObjId, NULL, &profileId, NULL);

    /* Convert threshold from bytes to pages */
    sai_uint32_t minThreshold = 0;
    sai_uint32_t maxThreshold = 0;

    if (pWredInfo->yellowMinThreshold)
    {
        minThreshold = XPSAI_BYTES_TO_PAGES(pWredInfo->yellowMinThreshold);
    }
    else
    {
        /* If min threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        minThreshold = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES -
                                            XPSAI_BUFFER_PAGE_SIZE_BYTES);
    }

    if (value.u32)
    {
        maxThreshold = XPSAI_BYTES_TO_PAGES(value.u32);
    }
    else
    {
        /* If max threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        maxThreshold = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES);
    }

    if (minThreshold == maxThreshold)
    {
        minThreshold = minThreshold - 1;
    }

    /* Write the wred configuration */
    if ((xpRetVal = xpsQosAqmConfigureWred(pWredInfo->xpDevId, profileId,
                                           minThreshold, maxThreshold, pWredInfo->weight,
                                           pWredInfo->yellowDropProb)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to configure wred parameters |retVal: %d\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Update the state database */
    pWredInfo->yellowMaxThreshold = value.u32;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetWredAttrYellowMaxThreshold

sai_status_t xpSaiGetWredAttrYellowMaxThreshold(sai_object_id_t wredObjId,
                                                sai_attribute_value_t* value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    value->u32 = pWredInfo->yellowMaxThreshold;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetWredAttrYellowDropProbability

sai_status_t xpSaiSetWredAttrYellowDropProbability(sai_object_id_t wredObjId,
                                                   sai_attribute_value_t value)
{
    return xpSaiSetWredAttrColourDropProbability(wredObjId, value,
                                                 XPSAI_WRED_COLOUR_YELLOW);
}


//Func: xpSaiGetWredAttrYellowDropProbability

sai_status_t xpSaiGetWredAttrYellowDropProbability(sai_object_id_t wredObjId,
                                                   sai_attribute_value_t* value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    value->u32 = pWredInfo->yellowDropProb;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetWredAttrRedEnable

sai_status_t xpSaiSetWredAttrRedEnable(sai_object_id_t wredObjId,
                                       sai_attribute_value_t value)
{

    return xpSaiSetWredAttrColourEnable(wredObjId, value, XPSAI_WRED_COLOUR_RED);
}


//Func: xpSaiGetWredAttrRedEnable

sai_status_t xpSaiGetWredAttrRedEnable(sai_object_id_t wredObjId,
                                       sai_attribute_value_t* value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    value->booldata = pWredInfo->redEnable;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetWredAttrRedMinThreshold

sai_status_t xpSaiSetWredAttrRedMinThreshold(sai_object_id_t wredObjId,
                                             sai_attribute_value_t value)
{
    return xpSaiSetWredAttrColourMinThreshold(wredObjId, value,
                                              XPSAI_WRED_COLOUR_RED);
}


//Func: xpSaiGetWredAttrRedMinThreshold

sai_status_t xpSaiGetWredAttrRedMinThreshold(sai_object_id_t wredObjId,
                                             sai_attribute_value_t* value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    value->u32 = pWredInfo->redMinThreshold;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetWredAttrRedMaxThreshold

sai_status_t xpSaiSetWredAttrRedMaxThreshold(sai_object_id_t wredObjId,
                                             sai_attribute_value_t value)
{
    XP_STATUS     xpRetVal  = XP_NO_ERR;
    xpSaiWred_t  *pWredInfo = NULL;
    sai_uint32_t  profileId = 0;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Get the Aqm Queue Red profile Id */
    xpSaiWredObjIdValueGet(wredObjId, NULL, NULL, &profileId);

    /* Convert threshold from bytes to pages */
    sai_uint32_t minThreshold = 0;
    sai_uint32_t maxThreshold = 0;

    if (pWredInfo->redMinThreshold)
    {
        minThreshold = XPSAI_BYTES_TO_PAGES(pWredInfo->redMinThreshold);
    }
    else
    {
        /* If min threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        minThreshold = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES -
                                            XPSAI_BUFFER_PAGE_SIZE_BYTES);
    }

    if (value.u32)
    {
        maxThreshold = XPSAI_BYTES_TO_PAGES(value.u32);
    }
    else
    {
        /* If max threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        maxThreshold = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES);
    }

    if (minThreshold == maxThreshold)
    {
        minThreshold = minThreshold - 1;
    }

    /* Write the wred configuration */
    if ((xpRetVal = xpsQosAqmConfigureWred(pWredInfo->xpDevId, profileId,
                                           minThreshold, maxThreshold, pWredInfo->weight,
                                           pWredInfo->redDropProb)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to configure wred parameters |retVal: %d\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Update the state database */
    pWredInfo->redMaxThreshold = value.u32;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetWredAttrRedMaxThreshold

sai_status_t xpSaiGetWredAttrRedMaxThreshold(sai_object_id_t wredObjId,
                                             sai_attribute_value_t* value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    value->u32 = pWredInfo->redMaxThreshold;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetWredAttrRedDropProbability

sai_status_t xpSaiSetWredAttrRedDropProbability(sai_object_id_t wredObjId,
                                                sai_attribute_value_t value)
{
    return xpSaiSetWredAttrColourDropProbability(wredObjId, value,
                                                 XPSAI_WRED_COLOUR_RED);
}


//Func: xpSaiGetWredAttrRedDropProbability

sai_status_t xpSaiGetWredAttrRedDropProbability(sai_object_id_t wredObjId,
                                                sai_attribute_value_t* value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    value->u32 = pWredInfo->redDropProb;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetWredAttrWeight

sai_status_t xpSaiSetWredAttrWeight(sai_object_id_t wredObjId,
                                    sai_attribute_value_t value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    sai_uint32_t greenProfileId, yellowProfileId, redProfileId;

    /* Get the Aqm Queue Green, Yellow and Red profile Ids */
    xpSaiWredObjIdValueGet(wredObjId, &greenProfileId, &yellowProfileId,
                           &redProfileId);

    /* Convert green thresholds from bytes to pages */
    sai_uint32_t minThreshold = 0;
    sai_uint32_t maxThreshold = 0;

    if (pWredInfo->greenMinThreshold)
    {
        minThreshold = XPSAI_BYTES_TO_PAGES(pWredInfo->greenMinThreshold);
    }
    else
    {
        /* If min threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        minThreshold = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES -
                                            XPSAI_BUFFER_PAGE_SIZE_BYTES);
    }

    if (pWredInfo->greenMaxThreshold)
    {
        maxThreshold = XPSAI_BYTES_TO_PAGES(pWredInfo->greenMaxThreshold);
    }
    else
    {
        /* If max threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        maxThreshold = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES);
    }

    if (minThreshold == maxThreshold)
    {
        minThreshold = minThreshold - 1;
    }

    /* Configure the weight for green color */
    if ((xpRetVal = xpsQosAqmConfigureWred(pWredInfo->xpDevId, greenProfileId,
                                           minThreshold, maxThreshold, value.u8, pWredInfo->greenDropProb)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to configure wred parameters |retVal: %d\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Convert yellow thresholds from bytes to pages */
    if (pWredInfo->yellowMinThreshold)
    {
        minThreshold = XPSAI_BYTES_TO_PAGES(pWredInfo->yellowMinThreshold);
    }
    else
    {
        /* If min threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        minThreshold = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES -
                                            XPSAI_BUFFER_PAGE_SIZE_BYTES);
    }

    if (pWredInfo->yellowMaxThreshold)
    {
        maxThreshold = XPSAI_BYTES_TO_PAGES(pWredInfo->yellowMaxThreshold);
    }
    else
    {
        /* If max threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        maxThreshold = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES);
    }

    if (minThreshold == maxThreshold)
    {
        minThreshold = minThreshold - 1;
    }

    /* Configure the weight for yellow color */
    if ((xpRetVal = xpsQosAqmConfigureWred(pWredInfo->xpDevId, yellowProfileId,
                                           minThreshold, maxThreshold, value.u8, pWredInfo->yellowDropProb)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to configure wred parameters |retVal: %d\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Convert red thresholds from bytes to pages */
    if (pWredInfo->redMinThreshold)
    {
        minThreshold = XPSAI_BYTES_TO_PAGES(pWredInfo->redMinThreshold);
    }
    else
    {
        /* If min threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        minThreshold = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES -
                                            XPSAI_BUFFER_PAGE_SIZE_BYTES);
    }

    if (pWredInfo->redMaxThreshold)
    {
        maxThreshold = XPSAI_BYTES_TO_PAGES(pWredInfo->redMaxThreshold);
    }
    else
    {
        /* If max threshold is set to default value (i.e 0) then set threshold to maximum buffer size */
        maxThreshold = XPSAI_BYTES_TO_PAGES(XPSAI_BUFFER_TOTAL_BUFFER_BYTES);
    }

    if (minThreshold == maxThreshold)
    {
        minThreshold = minThreshold - 1;
    }

    /* Configure the weight for red color */
    if ((xpRetVal = xpsQosAqmConfigureWred(pWredInfo->xpDevId, redProfileId,
                                           minThreshold, maxThreshold, value.u8, pWredInfo->redDropProb)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to configure wred parameters |retVal: %d\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Update the state database */
    pWredInfo->weight = value.u8;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetWredAttrWeight

sai_status_t xpSaiGetWredAttrWeight(sai_object_id_t wredObjId,
                                    sai_attribute_value_t* value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    value->u8 = pWredInfo->weight;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetWredAttrEcnMarkMode

sai_status_t xpSaiSetWredAttrEcnMarkMode(sai_object_id_t wredObjId,
                                         sai_attribute_value_t value)
{
    XP_STATUS     xpRetVal  = XP_NO_ERR;
    xpSaiWred_t  *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    if (pWredInfo->ecnMarkMode != (sai_ecn_mark_mode_t)value.s32)
    {
        /* Update the state database */
        pWredInfo->ecnMarkMode = (sai_ecn_mark_mode_t)value.s32;
        return xpSaiSyncWredProfile(pWredInfo);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetWredAttrEcnMarkMode

sai_status_t xpSaiGetWredAttrEcnMarkMode(sai_object_id_t wredObjId,
                                         sai_attribute_value_t* value)
{
    XP_STATUS    xpRetVal  = XP_NO_ERR;
    xpSaiWred_t *pWredInfo = NULL;

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    value->s32 = pWredInfo->ecnMarkMode;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateWred

sai_status_t xpSaiCreateWred(sai_object_id_t *wred_id,
                             sai_object_id_t switch_id,
                             uint32_t attr_count, const sai_attribute_t *attr_list)
{
    xpSaiWredAttributesT  *attributes = NULL;
    xpSaiWred_t          *pWredInfo = NULL;
    XP_STATUS             xpRetVal  = XP_NO_ERR;
    sai_status_t          saiRetVal = SAI_STATUS_SUCCESS;

    attributes = (xpSaiWredAttributesT *)xpMalloc(sizeof(xpSaiWredAttributesT));
    if (!attributes)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for attributes\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(attributes, 0, sizeof(xpSaiWredAttributesT));

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               WRED_VALIDATION_ARRAY_SIZE, wred_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        xpFree(attributes);
        return saiRetVal;
    }

    xpSaiSetDefaultWredAttributeVals(attributes);
    xpSaiUpdateWredAttributeVals(attr_count, attr_list, attributes);

    /* Validate input attribute values */
    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiWredValidateAttributeValue(attr_list[count].id,
                                                    attr_list[count].value, count);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            xpFree(attributes);
            return saiRetVal;
        }
    }

    /* Get the xpDevId switch object id */
    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(switch_id);

    /* Create wred profile object id */
    if ((saiRetVal = xpSaiWredCreateProfileObject(xpDevId,
                                                  wred_id)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create wred profile object\n");
        xpFree(attributes);
        return saiRetVal;
    }

    /* Create wred state database */
    if ((xpRetVal = xpSaiWredCreateState(*wred_id, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to create wred state database\n");
        xpFree(attributes);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Update the state database */
    pWredInfo->greenEnable        = attributes->greenEnable.booldata;
    pWredInfo->greenMinThreshold  = attributes->greenMinThreshold.u32;
    pWredInfo->greenMaxThreshold  = attributes->greenMaxThreshold.u32;
    pWredInfo->greenDropProb      = attributes->greenDropProbability.u32;

    pWredInfo->yellowEnable       = attributes->yellowEnable.booldata;
    pWredInfo->yellowMinThreshold = attributes->yellowMinThreshold.u32;
    pWredInfo->yellowMaxThreshold = attributes->yellowMaxThreshold.u32;
    pWredInfo->yellowDropProb     = attributes->yellowDropProbability.u32;

    pWredInfo->redEnable          = attributes->redEnable.booldata;
    pWredInfo->redMinThreshold    = attributes->redMinThreshold.u32;
    pWredInfo->redMaxThreshold    = attributes->redMaxThreshold.u32;
    pWredInfo->redDropProb        = attributes->redDropProbability.u32;

    pWredInfo->weight             = attributes->weight.u8;
    pWredInfo->ecnMarkMode        = (sai_ecn_mark_mode_t)
                                    attributes->ecnMarkMode.s32;
    pWredInfo->xpDevId            = xpDevId;
    xpFree(attributes);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveWred

sai_status_t xpSaiRemoveWred(sai_object_id_t wred_id)
{
    XP_STATUS     xpRetVal     = XP_NO_ERR;
    sai_status_t  saiRetVal    = SAI_STATUS_SUCCESS;
    xpSaiWred_t  *pWredInfo    = NULL;

    /* Validate Wred Object Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(wred_id, SAI_OBJECT_TYPE_WRED))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(wred_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wred_id, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    if (pWredInfo->objCount != 0)
    {
        XP_SAI_LOG_ERR("Error: Wred is in use, unmap before remove\n");
        return SAI_STATUS_OBJECT_IN_USE;
    }

    /* Remove wred profile object id */
    if ((saiRetVal = xpSaiWredRemoveProfileObject(pWredInfo->xpDevId,
                                                  wred_id)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create wred profile object\n");
        return saiRetVal;
    }

    /* Remove wred state database */
    if ((xpRetVal = xpSaiWredRemoveState(wred_id)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to create wred state database\n");
        return xpsStatus2SaiStatus(xpRetVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetWredAttribute

sai_status_t xpSaiSetWredAttribute(sai_object_id_t wred_id,
                                   const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("__SET WRED attr->id %d on object 0x%08x%08x\n", attr->id,
                   LONG_INT_FORMAT(wred_id));

    retVal = xpSaiAttrCheck(1, attr,
                            WRED_VALIDATION_ARRAY_SIZE, wred_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    /* Validate wred object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(wred_id, SAI_OBJECT_TYPE_WRED))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(wred_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Validate input attribute values */
    retVal = xpSaiWredValidateAttributeValue(attr->id, attr->value, 0);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        return retVal;
    }

    switch (attr->id)
    {
        case SAI_WRED_ATTR_GREEN_ENABLE:
            {
                retVal = xpSaiSetWredAttrGreenEnable(wred_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_WRED_ATTR_GREEN_ENABLE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_WRED_ATTR_GREEN_MIN_THRESHOLD:
            {
                retVal = xpSaiSetWredAttrGreenMinThreshold(wred_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_WRED_ATTR_GREEN_MIN_THRESHOLD)\n");
                    return retVal;
                }
                break;
            }
        case SAI_WRED_ATTR_GREEN_MAX_THRESHOLD:
            {
                retVal = xpSaiSetWredAttrGreenMaxThreshold(wred_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_WRED_ATTR_GREEN_MAX_THRESHOLD)\n");
                    return retVal;
                }
                break;
            }
        case SAI_WRED_ATTR_GREEN_DROP_PROBABILITY:
            {
                retVal = xpSaiSetWredAttrGreenDropProbability(wred_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_WRED_ATTR_GREEN_DROP_PROBABILITY)\n");
                    return retVal;
                }
                break;
            }
        case SAI_WRED_ATTR_YELLOW_ENABLE:
            {
                retVal = xpSaiSetWredAttrYellowEnable(wred_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_WRED_ATTR_YELLOW_ENABLE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD:
            {
                retVal = xpSaiSetWredAttrYellowMinThreshold(wred_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD)\n");
                    return retVal;
                }
                break;
            }
        case SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD:
            {
                retVal = xpSaiSetWredAttrYellowMaxThreshold(wred_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD)\n");
                    return retVal;
                }
                break;
            }
        case SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY:
            {
                retVal = xpSaiSetWredAttrYellowDropProbability(wred_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY)\n");
                    return retVal;
                }
                break;
            }
        case SAI_WRED_ATTR_RED_ENABLE:
            {
                retVal = xpSaiSetWredAttrRedEnable(wred_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_WRED_ATTR_RED_ENABLE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_WRED_ATTR_RED_MIN_THRESHOLD:
            {
                retVal = xpSaiSetWredAttrRedMinThreshold(wred_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_WRED_ATTR_RED_MIN_THRESHOLD)\n");
                    return retVal;
                }
                break;
            }
        case SAI_WRED_ATTR_RED_MAX_THRESHOLD:
            {
                retVal = xpSaiSetWredAttrRedMaxThreshold(wred_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_WRED_ATTR_RED_MAX_THRESHOLD)\n");
                    return retVal;
                }
                break;
            }
        case SAI_WRED_ATTR_RED_DROP_PROBABILITY:
            {
                retVal = xpSaiSetWredAttrRedDropProbability(wred_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_WRED_ATTR_RED_DROP_PROBABILITY)\n");
                    return retVal;
                }
                break;
            }
        case SAI_WRED_ATTR_WEIGHT:
            {
                retVal = xpSaiSetWredAttrWeight(wred_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_WRED_ATTR_WEIGHT)\n");
                    return retVal;
                }
                break;
            }
        case SAI_WRED_ATTR_ECN_MARK_MODE:
            {
                retVal = xpSaiSetWredAttrEcnMarkMode(wred_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_WRED_ATTR_ECN_MARK_MODE)\n");
                    return retVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to set %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetWredAttribute

sai_status_t xpSaiGetWredAttribute(sai_object_id_t wred_id, uint32_t attr_count,
                                   sai_attribute_t *attr_list)
{
    xpSaiWredAttributesT  *attributes = NULL;
    xpSaiWred_t          *pWredInfo  = NULL;
    sai_status_t          retVal     = SAI_STATUS_SUCCESS;
    XP_STATUS             xpRetVal   = XP_NO_ERR;

    attributes = (xpSaiWredAttributesT *)xpMalloc(sizeof(xpSaiWredAttributesT));
    if (!attributes)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for attributes\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(attributes, 0, sizeof(xpSaiWredAttributesT));

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            WRED_VALIDATION_ARRAY_SIZE, wred_attribs,
                            SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        xpFree(attributes);
        return retVal;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(wred_id, SAI_OBJECT_TYPE_WRED))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(wred_id));
        xpFree(attributes);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the state data for given wred object id */
    if ((xpRetVal = xpSaiWredGetState(wred_id, &pWredInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get wred info from state database\n");
        xpFree(attributes);
        return xpsStatus2SaiStatus(xpRetVal);
    }


    attributes->greenEnable.booldata      = pWredInfo->greenEnable;
    attributes->greenMinThreshold.u32     = pWredInfo->greenMinThreshold;
    attributes->greenMaxThreshold.u32     = pWredInfo->greenMaxThreshold;
    attributes->greenDropProbability.u32  = pWredInfo->greenDropProb;

    attributes->yellowEnable.booldata     = pWredInfo->yellowEnable;
    attributes->yellowMinThreshold.u32    = pWredInfo->yellowMinThreshold;
    attributes->yellowMaxThreshold.u32    = pWredInfo->yellowMaxThreshold;
    attributes->yellowDropProbability.u32 = pWredInfo->yellowDropProb;

    attributes->redEnable.booldata        = pWredInfo->redEnable;
    attributes->redMinThreshold.u32       = pWredInfo->redMinThreshold;
    attributes->redMaxThreshold.u32       = pWredInfo->redMaxThreshold;
    attributes->redDropProbability.u32    = pWredInfo->redDropProb;

    attributes->weight.u8                 = pWredInfo->weight;
    attributes->ecnMarkMode.s32           = pWredInfo->ecnMarkMode;

    xpSaiUpdateAttrListWredVals(attributes, attr_count, attr_list);
    xpFree(attributes);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiWredInit

XP_STATUS xpSaiWredInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    /* Init allocators */
    xpsRetVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT, XPS_ALLOCATOR_WRED_ID,
                                            16, 0);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI wred allocator\n");
        return xpsRetVal;
    }

    /* Create a Global Sai Wred Db */
    xpsRetVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Wred Db", XPS_GLOBAL,
                                   &xpSaiWredKeyComp, xpSaiWredDbHndl);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to Register Sai Wred Db |retVal: %d\n", xpsRetVal);
        return xpsRetVal;
    }

    if ((xpsRetVal = cpssHalEcnMarkingEnableSet(xpSaiDevId, GT_TRUE))!= XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to enable ECN, xpStatus: %d\n", xpsRetVal);
        return xpsRetVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiWredDeInit

XP_STATUS xpSaiWredDeInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    /* Remove Global Sai Wred Db */
    if ((xpsRetVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                          &xpSaiWredDbHndl)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to Deregister Sai Wred Db |retVal: %d\n", xpsRetVal);
        return xpsRetVal;
    }

    if ((xpsRetVal = cpssHalEcnMarkingEnableSet(xpSaiDevId, GT_FALSE))!= XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to disable ECN, xpStatus: %d\n", xpsRetVal);
        return xpsRetVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiWredApiInit

XP_STATUS xpSaiWredApiInit(uint64_t flag,
                           const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiWredApiInit\n");

    _xpSaiWredApi = (sai_wred_api_t *) xpMalloc(sizeof(sai_wred_api_t));
    if (NULL == _xpSaiWredApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiWredApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiWredApi->create_wred = xpSaiCreateWred;
    _xpSaiWredApi->remove_wred = xpSaiRemoveWred;
    _xpSaiWredApi->set_wred_attribute  = xpSaiSetWredAttribute;
    _xpSaiWredApi->get_wred_attribute  = xpSaiGetWredAttribute;

    saiRetVal = xpSaiApiRegister(SAI_API_WRED, (void*)_xpSaiWredApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register WRED API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}

//Func: xpSaiWredApiDeinit

XP_STATUS xpSaiWredApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiWredApiDeinit\n");

    xpFree(_xpSaiWredApi);
    _xpSaiWredApi = NULL;

    return retVal;
}

sai_status_t xpSaiMaxCountWredAttribute(uint32_t *count)
{
    *count = SAI_WRED_ATTR_END;
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountWredObjects(uint32_t *count)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsScope_t scopeId = XP_SCOPE_DEFAULT;
    retVal = xpsStateGetCount(scopeId, xpSaiWredDbHndl, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

static sai_status_t xpSaiGetWredIdList(xpsScope_t scopeId,
                                       sai_object_id_t *wred_id)
{
    XP_STATUS result = XP_NO_ERR;
    xpSaiWred_t *curWredDbEntry = NULL;
    xpSaiWred_t *prevWredDbEntry = NULL;
    uint64_t prevWredDbKey = 0xFFFFFFFFFFFFFFFF;//wredId is the key
    uint32_t count = 0;
    result=  xpsStateGetNextData(scopeId, xpSaiWredDbHndl, NULL,
                                 (void **)&curWredDbEntry);
    if (result != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get data, return Value : %d", result);
        return xpsStatus2SaiStatus(result);
    }
    while (curWredDbEntry)
    {
        //Store the node information in previous and get-next
        prevWredDbEntry = curWredDbEntry;
        prevWredDbKey = prevWredDbEntry->wredObjId;
        wred_id[count] = prevWredDbKey;
        result=  xpsStateGetNextData(scopeId, xpSaiWredDbHndl,
                                     (xpsDbKey_t)&prevWredDbKey, (void **)&curWredDbEntry);
        if (result != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get data, return Value : %d", result);
            return xpsStatus2SaiStatus(result);
        }
        count++;
    }
    return xpsStatus2SaiStatus(result);
}

sai_status_t xpSaiGetWredObjectList(uint32_t *object_count,
                                    sai_object_key_t *object_list)
{
    sai_status_t     retVal = SAI_STATUS_SUCCESS;
    sai_object_id_t *wred_id;

    /*TODO Handling of object_count by getting objCount and buffer overflow condition*/

    wred_id = (sai_object_id_t *)xpMalloc(sizeof(sai_object_id_t)*(*object_count));
    if (wred_id == NULL)
    {
        XP_SAI_LOG_ERR("Error: allocation failed \n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    retVal = xpSaiGetWredIdList(XP_SCOPE_DEFAULT, wred_id);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        xpFree(wred_id);
        return retVal;
    }
    for (uint32_t i = 0; i < *object_count; i++)
    {
        object_list[i].key.object_id = wred_id[i];
    }

    xpFree(wred_id);
    return retVal;
}

XP_STATUS xpSaiWredSai2CpssProfileGet
(
    IN  xpSaiWred_t *pWredInfo,
    OUT CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_STC * pCpssWredInfo,
    OUT CPSS_PORT_QUEUE_ECN_PARAMS_STC                  * pCpssEcnInfo
)
{
    if (NULL==pCpssWredInfo||NULL==pCpssEcnInfo)
    {
        XP_SAI_LOG_ERR("null pointer detected \n");
        return XP_ERR_AAPL_NULL_POINTER;
    }

    pCpssEcnInfo->dp0EcnAttributes.ecnEnable = GT_FALSE;
    pCpssEcnInfo->dp1EcnAttributes.ecnEnable = GT_FALSE;
    pCpssEcnInfo->dp2EcnAttributes.ecnEnable = GT_FALSE;

    switch (pWredInfo->ecnMarkMode)
    {
        case SAI_ECN_MARK_MODE_GREEN:
            pCpssEcnInfo->dp0EcnAttributes.ecnEnable = GT_TRUE;
            break;
        case SAI_ECN_MARK_MODE_GREEN_RED:
            pCpssEcnInfo->dp0EcnAttributes.ecnEnable = GT_TRUE;
            pCpssEcnInfo->dp2EcnAttributes.ecnEnable = GT_TRUE;
            break;
        case SAI_ECN_MARK_MODE_GREEN_YELLOW:
            pCpssEcnInfo->dp0EcnAttributes.ecnEnable = GT_TRUE;
            pCpssEcnInfo->dp1EcnAttributes.ecnEnable = GT_TRUE;
            break;
        case SAI_ECN_MARK_MODE_YELLOW:
            pCpssEcnInfo->dp1EcnAttributes.ecnEnable = GT_TRUE;
            break;
        case SAI_ECN_MARK_MODE_RED:
            pCpssEcnInfo->dp2EcnAttributes.ecnEnable = GT_TRUE;
            break;
        case SAI_ECN_MARK_MODE_YELLOW_RED:
            pCpssEcnInfo->dp1EcnAttributes.ecnEnable = GT_TRUE;
            pCpssEcnInfo->dp2EcnAttributes.ecnEnable = GT_TRUE;
            break;
        case SAI_ECN_MARK_MODE_ALL:
            pCpssEcnInfo->dp0EcnAttributes.ecnEnable = GT_TRUE;
            pCpssEcnInfo->dp1EcnAttributes.ecnEnable = GT_TRUE;
            pCpssEcnInfo->dp2EcnAttributes.ecnEnable = GT_TRUE;
            break;
        default:/*SAI_ECN_MARK_MODE_NONE*/
            break;
    }

    if (true==pWredInfo->greenEnable)
    {
        if (!IS_DEVICE_EBOF_PROFILE(xpSaiSwitchDevTypeGet()))
        {
            SAI_PROFILE_TO_CPSS_PROFILE_CONVERT(pWredInfo->green, 0);
        }
        else
        {
            SAI_EBOF_PROFILE_TO_CPSS_PROFILE_CONVERT(pWredInfo->green, 0);
        }
    }
    else
    {
        pCpssWredInfo->dp0WredAttributes.wredSize = 0;
    }

    if (true==pWredInfo->yellowEnable)
    {
        if (!IS_DEVICE_EBOF_PROFILE(xpSaiSwitchDevTypeGet()))
        {
            SAI_PROFILE_TO_CPSS_PROFILE_CONVERT(pWredInfo->yellow, 0);
        }
        else
        {
            SAI_EBOF_PROFILE_TO_CPSS_PROFILE_CONVERT(pWredInfo->yellow, 0);
        }
    }
    else
    {
        pCpssWredInfo->dp1WredAttributes.wredSize = 0;
    }

    if (true==pWredInfo->redEnable)
    {
        if (!IS_DEVICE_EBOF_PROFILE(xpSaiSwitchDevTypeGet()))
        {
            SAI_PROFILE_TO_CPSS_PROFILE_CONVERT(pWredInfo->red, 0);
        }
        else
        {
            SAI_EBOF_PROFILE_TO_CPSS_PROFILE_CONVERT(pWredInfo->red, 0);
        }
    }
    else
    {
        pCpssWredInfo->dp2WredAttributes.wredSize = 0;
    }

    return XP_NO_ERR;

}



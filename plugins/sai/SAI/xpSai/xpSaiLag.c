// xpSaiLag.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiLag.h"
#include "xpsLag.h"
#include "xpSaiValidationArrays.h"
#include "xpSaiAclMapper.h"
#include "xpSaiStub.h"

XP_SAI_LOG_REGISTER_API(SAI_API_LAG);

static sai_lag_api_t* _xpSaiLagApi;

extern xpsDbHandle_t portLagDbHandle;

xpsDbHandle_t portLagPortCountDbHandle = XPSAI_LAG_PORT_COUNT_DB_HNDL;
xpsDbHandle_t xpSaiLagInfoDbHandle = XPSAI_LAG_INFO_DB_HNDL;

//Func: xpSaiLagPortCountKeyComp

sai_int32_t xpSaiLagPortCountKeyComp(void *key1, void *key2)
{
    if (((xpSaiLagPortCountInfo_t*)key1)->vlanId > ((xpSaiLagPortCountInfo_t*)
                                                    key2)->vlanId)
    {
        return 1;
    }
    else if (((xpSaiLagPortCountInfo_t*)key1)->vlanId < ((xpSaiLagPortCountInfo_t*)
                                                         key2)->vlanId)
    {
        return -1;
    }
    else /* That's the same VLAN. */
    {
        return ((((xpSaiLagPortCountInfo_t*)key1)->lagId) - (((xpSaiLagPortCountInfo_t*)
                                                              key2)->lagId));
    }
}

sai_status_t xpSaiLagPortCountInfoGet(xpsInterfaceId_t lagId, xpsVlan_t vlanId,
                                      xpSaiLagPortCountInfo_t **ppSaiLagPortCountInfoEntry)
{
    XP_STATUS                   retVal     = XP_NO_ERR;
    xpSaiLagPortCountInfo_t     lagPortCountInfoKey;

    memset(&lagPortCountInfoKey, 0, sizeof(lagPortCountInfoKey));

    lagPortCountInfoKey.lagId = lagId;
    lagPortCountInfoKey.vlanId = vlanId;
    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, portLagPortCountDbHandle,
                                &lagPortCountInfoKey, (void**)ppSaiLagPortCountInfoEntry);

    if (NULL == *ppSaiLagPortCountInfoEntry)
    {
        XP_SAI_LOG_DBG("Could not find LAG entry in DB.");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiSetDefaultLagMemberAttributeVals

void xpSaiSetDefaultLagMemberAttributeVals(xpSaiLagMemberAttributesT*
                                           attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiSetDefaultLagMemberAttributeVals\n");

    attributes->egressDisable.booldata= false;
    attributes->ingressDisable.booldata = false;
}

//Func: xpSaiUpdateLagMemberAttributeVals

void xpSaiUpdateLagMemberAttributeVals(const uint32_t attr_count,
                                       const sai_attribute_t* attr_list, xpSaiLagMemberAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiUpdateLagMemberAttributeVals\n");

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_LAG_MEMBER_ATTR_LAG_ID:
                {
                    attributes->lagId = attr_list[count].value;
                    break;
                }
            case SAI_LAG_MEMBER_ATTR_PORT_ID:
                {
                    attributes->portId = attr_list[count].value;
                    break;
                }
            case SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE:
                {
                    attributes->egressDisable = attr_list[count].value;
                    break;
                }
            case SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE:
                {
                    attributes->ingressDisable = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }

    }

}


//Func: xpSaiLagInfoKeyComp

sai_int32_t xpSaiLagInfoKeyComp(void *key1, void *key2)
{
    return ((((xpSaiLagInfo_t*)key1)->lagId) - (((xpSaiLagInfo_t*)key2)->lagId));
}

/**
 * \brief Create LAG redirection node
 *
 * \param [in] xpsInterfaceId_t lagId - LAG interface
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiLagInfoNodeCreate(xpsInterfaceId_t lagId,
                                           bool autoDistribute)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpSaiLagInfo_t* pSaiLagInfoEntry = NULL;

    xpsRetVal = xpsStateHeapMalloc(sizeof(xpSaiLagInfo_t),
                                   (void**)&pSaiLagInfoEntry);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate channel structure\n");
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    memset(pSaiLagInfoEntry, 0, sizeof(xpSaiLagInfo_t));

    pSaiLagInfoEntry->lagId = lagId;
    pSaiLagInfoEntry->primaryLagId = XPS_INTF_INVALID_ID;
    pSaiLagInfoEntry->isLagEgressRedirectEnabled = false;
    pSaiLagInfoEntry->isEgressBlockListEn = false;
    pSaiLagInfoEntry->refCnt = 0;
    pSaiLagInfoEntry->lagConfig.defaultVlanPriority = 0;
    pSaiLagInfoEntry->lagConfig.portVlanId = 1;
    pSaiLagInfoEntry->lagConfig.dropTagged = false;
    pSaiLagInfoEntry->lagConfig.dropUntagged = false;
    pSaiLagInfoEntry->lagConfig.saMissCmd = XP_PKTCMD_MAX;
    pSaiLagInfoEntry->lagConfig.autoDistribute = autoDistribute;
    pSaiLagInfoEntry->pvidUserSetting = XPSAI_DEFAULT_VLAN_ID;

    xpsRetVal = xpsStateInsertData(XP_SCOPE_DEFAULT, xpSaiLagInfoDbHandle,
                                   (void*)pSaiLagInfoEntry);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not insert a channel structure into DB, error %d\n",
                       xpsRetVal);
        xpsStateHeapFree((void*)pSaiLagInfoEntry);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return xpsStatus2SaiStatus(xpsRetVal);
}

/**
 * \brief Remove LAG redirection node
 *
 * \param [in] xpsInterfaceId_t lagId - LAG interface
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiLagInfoNodeRemove(xpsInterfaceId_t lagId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpSaiLagInfo_t* pSaiLagInfoEntry = NULL;
    xpSaiLagInfo_t  saiLagInfoKey;

    saiLagInfoKey.lagId = lagId;

    xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiLagInfoDbHandle,
                                   (xpsDbKey_t)&saiLagInfoKey, (void**)&pSaiLagInfoEntry);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Lag interface %u failed", lagId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!pSaiLagInfoEntry)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    xpsRetVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, xpSaiLagInfoDbHandle,
                                   &saiLagInfoKey, (void**)&pSaiLagInfoEntry);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not delete channel structure from DB, saiLagInfoKey.intfId %d\n",
                       lagId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    xpsRetVal = xpsStateHeapFree((void*)pSaiLagInfoEntry);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not deallocate channel structure for LAG %u, error %d\n",
                       lagId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return xpsStatus2SaiStatus(xpsRetVal);
}

/**
 * \brief Check if there are external dependencies to this LAG
 *
 * \param [in] xpsInterfaceId_t lagId - LAG interface
 *
 * \return sai_status_t
 * \retval true - There are external dependencies
 * \retval false - No external dependencies
 */
static bool xpSaiLagInfoDependenciesCheck(xpsInterfaceId_t lagId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpSaiLagInfo_t* pSaiLagInfoEntry = NULL;
    xpSaiLagInfo_t  saiLagInfoKey;

    saiLagInfoKey.lagId = lagId;

    xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiLagInfoDbHandle,
                                   (xpsDbKey_t)&saiLagInfoKey, (void**)&pSaiLagInfoEntry);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Lag interface %u failed", lagId);
        return false;
    }

    if (!pSaiLagInfoEntry)
    {
        return false;
    }

    return !!pSaiLagInfoEntry->refCnt;
}

/**
 * \brief Set LAG redirection info
 *
 * \param [in] xpsDevice_t devId - Device ID
 * \param [in] xpsInterfaceId_t primaryLag - LAG interface to switch traffic from
 * \param [in] xpsInterfaceId_t secondaryLag - LAG interface to switch traffic to
 * \param [in] bool enable - redirection enabled/disabled
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiLagInfoSet(xpsDevice_t devId,
                                    xpsInterfaceId_t primaryLag, xpsInterfaceId_t secondaryLag, bool enable)
{
    XP_STATUS       xpsRetVal  = XP_NO_ERR;
    xpSaiLagInfo_t  lagInfoKey;
    xpSaiLagInfo_t  *primaryLagInfo = NULL;
    xpSaiLagInfo_t  *secondaryLagInfo = NULL;
    xpsScope_t      scopeId;
    xpSaiPrimaryLagIdNode_t *lagIdNode = NULL;
    xpSaiPrimaryLagIdNode_t *tempNode = NULL, *prevNode = NULL;


    /* Get Scope Id from devId */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Get the Primary Lag information */
    lagInfoKey.lagId = primaryLag;
    if ((xpsRetVal = xpsStateSearchData(scopeId, xpSaiLagInfoDbHandle,
                                        (xpsDbKey_t)&lagInfoKey, (void**)&primaryLagInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Primary Lag interface failed, lag interface(%d)",
                       primaryLag);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Get the Secondary Lag information */
    lagInfoKey.lagId = secondaryLag;
    if ((xpsRetVal = xpsStateSearchData(scopeId, xpSaiLagInfoDbHandle,
                                        (xpsDbKey_t)&lagInfoKey, (void**)&secondaryLagInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Secondary Lag interface failed, lag interface(%d)",
                       secondaryLag);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!primaryLagInfo || !secondaryLagInfo)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    primaryLagInfo->isLagEgressRedirectEnabled = enable;
    secondaryLagInfo->isLagEgressRedirectEnabled = enable;
    secondaryLagInfo->primaryLagEgressPortList.size = 0;
    primaryLagInfo->primaryLagId = XPS_INTF_INVALID_ID;
    secondaryLagInfo->primaryLagId = enable ? primaryLag : XPS_INTF_INVALID_ID;

    if (enable)
    {
        /*xpsRetVal = xpsInterfaceGetPortBitMapIndex(devId, secondaryLag, &primaryLagInfo->secondaryLagPortBitMapPtrIdx);//xpsInterfaceGetPorts(devId, secondaryLag, &primaryLagInfo->secondaryLagEgressPortList);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get LAG %d port list, error %u", secondaryLag, xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }*/
        secondaryLagInfo->secondaryLagRedirectCount++;
        lagIdNode = (xpSaiPrimaryLagIdNode_t*)xpMalloc(sizeof(xpSaiPrimaryLagIdNode_t));
        if (!lagIdNode)
        {
            XP_SAI_LOG_ERR("Error: allocation failed for lagIdNode\n");
            return XP_ERR_MEM_ALLOC_ERROR;
        }
        lagIdNode->next = NULL;
        if (!(secondaryLagInfo->primaryLagIdList))
        {
            secondaryLagInfo->primaryLagIdList = lagIdNode;
        }
        else
        {
            tempNode = secondaryLagInfo->primaryLagIdList;
            while (tempNode->next !=NULL)
            {
                tempNode = tempNode->next;
            }
            tempNode->next = lagIdNode;
        }
    }
    else
    {
        secondaryLagInfo->secondaryLagRedirectCount--;
        primaryLagInfo->primaryLagEgressPortList.size = 0;
        primaryLagInfo->secondaryLagPortBitMapPtrIdx = 0;

        prevNode = secondaryLagInfo->primaryLagIdList;
        tempNode = prevNode->next;

        if (prevNode->lagId == primaryLag)
        {
            secondaryLagInfo->primaryLagIdList = tempNode;
            free(prevNode);
        }
        else
        {
            while (tempNode!=NULL)
            {
                if (tempNode->lagId == primaryLag)
                {
                    prevNode->next = tempNode->next;
                    free(tempNode);
                    break;
                }
                else
                {
                    prevNode=tempNode;
                    tempNode= tempNode->next;
                }
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Retrieve LAG egress redirect status
 *
 * \param [in] xpsDevice_t devId - Device ID
 * \param [in] xpsInterfaceId_t primaryLag - LAG interface to switch traffic from
 * \param [out] bool *enabled - redirection status
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiLagEgressRedirectStatusGet(xpsDevice_t devId,
                                                    xpsInterfaceId_t lagId, bool *enabled)
{
    XP_STATUS      xpsRetVal  = XP_NO_ERR;
    xpSaiLagInfo_t lagInfoKey;
    xpSaiLagInfo_t *primaryLagInfo = NULL;
    xpsScope_t     scopeId;

    /* Get Scope Id from devId */
    xpsRetVal = xpsScopeGetScopeId(devId, &scopeId);
    if (xpsRetVal != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Get the Primary Lag information */
    lagInfoKey.lagId = lagId;
    xpsRetVal = xpsStateSearchData(scopeId, xpSaiLagInfoDbHandle,
                                   (xpsDbKey_t)&lagInfoKey, (void**)&primaryLagInfo);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Primary Lag interface failed, lag interface(%d)",
                       lagId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!primaryLagInfo)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *enabled = primaryLagInfo->isLagEgressRedirectEnabled;

    return SAI_STATUS_SUCCESS;
}
/**
 * \brief Get Lag Port Bitmap Index Pointer
 *
 * \param [in] xpsInterfaceId_t primaryLagId - LAG interface
 * \param [out] uint32_t *portBitMapPtrIdx - lag's port bit map pointer index
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiLagInfoLagPortBitMapIndexGet(xpsInterfaceId_t
                                                      primaryLagId, uint32_t *portBitMapPtrIdx)
{
    XP_STATUS      xpsRetVal  = XP_NO_ERR;
    xpSaiLagInfo_t lagInfoKey;
    xpSaiLagInfo_t *lagInfo   = NULL;

    /* Get the Primary Lag information */
    lagInfoKey.lagId = primaryLagId;
    xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiLagInfoDbHandle,
                                   (xpsDbKey_t)&lagInfoKey, (void**)&lagInfo);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Primary Lag interface failed, lag interface(%d)",
                       primaryLagId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!lagInfo)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *portBitMapPtrIdx = lagInfo->lagPortBitMapPtrIdx;

    return SAI_STATUS_SUCCESS;
}
/**
 * \brief Get redirect LAG primary port list
 *
 * \param [in] xpsInterfaceId_t primaryLagId - LAG interface
 * \param [out] xpsPortList_t *portList - primary port list
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiLagInfoPrimaryPortListGet(xpsInterfaceId_t
                                                   primaryLagId, xpsPortList_t *portList)
{
    XP_STATUS      xpsRetVal  = XP_NO_ERR;
    xpSaiLagInfo_t lagInfoKey;
    xpSaiLagInfo_t *lagInfo   = NULL;
    uint32_t       i          = 0;

    if (portList == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get the Primary Lag information */
    lagInfoKey.lagId = primaryLagId;
    xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiLagInfoDbHandle,
                                   (xpsDbKey_t)&lagInfoKey, (void**)&lagInfo);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Primary Lag interface failed, lag interface(%d)",
                       primaryLagId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!lagInfo)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (!lagInfo->isLagEgressRedirectEnabled)
    {
        XP_SAI_LOG_ERR("LAG %u is not in redirect state\n", primaryLagId);
        return SAI_STATUS_FAILURE;
    }

    if (lagInfo->primaryLagId != XPS_INTF_INVALID_ID)
    {
        XP_SAI_LOG_ERR("LAG %u is not primary\n", primaryLagId);
        return SAI_STATUS_FAILURE;
    }

    portList->size = lagInfo->primaryLagEgressPortList.size;
    for (i = 0; i < portList->size; i++)
    {
        portList->portList[i] = lagInfo->primaryLagEgressPortList.portList[i];
    }

    return SAI_STATUS_SUCCESS;
}
/**
 * \brief Update primary redirect LAG primary port bit map pointer index
 *
 * \param [in] xpsInterfaceId_t primaryLagId - LAG interface
 * \param [in] uint32_t portBitMapIndex - portbitmap pointer to update
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiLagInfoLagPortBitMapPointerUpdate(
    xpsInterfaceId_t primaryLagId, uint32_t portBitMapIndex)
{
    XP_STATUS      xpsRetVal  = XP_NO_ERR;
    xpSaiLagInfo_t lagInfoKey;
    xpSaiLagInfo_t *lagInfo   = NULL;

    /* Get the Primary Lag information */
    lagInfoKey.lagId = primaryLagId;
    xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiLagInfoDbHandle,
                                   (xpsDbKey_t)&lagInfoKey, (void**)&lagInfo);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Primary Lag interface failed, lag interface(%d)",
                       primaryLagId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!lagInfo)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    lagInfo->lagPortBitMapPtrIdx = portBitMapIndex;

    return SAI_STATUS_SUCCESS;
}


/**
 * \brief Update primary redirect LAG's secondary port bit map pointer index
 *
 * \param [in] xpsInterfaceId_t primaryLagId - LAG interface
 * \param [in] uint32_t portBitMapIndex - portbitmap pointer to update
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiLagInfoLagSecondaryPortBitMapPointerUpdate(
    xpsInterfaceId_t primaryLagId, uint32_t portBitMapIndex, bool enable)
{
    XP_STATUS      xpsRetVal  = XP_NO_ERR;
    xpSaiLagInfo_t lagInfoKey;
    xpSaiLagInfo_t *lagInfo   = NULL;

    /* Get the Primary Lag information */
    lagInfoKey.lagId = primaryLagId;
    xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiLagInfoDbHandle,
                                   (xpsDbKey_t)&lagInfoKey, (void**)&lagInfo);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Primary Lag interface failed, lag interface(%d)",
                       primaryLagId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!lagInfo)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (!lagInfo->isLagEgressRedirectEnabled)
    {
        XP_SAI_LOG_ERR("LAG %u is not in redirect state\n", primaryLagId);
        lagInfo->secondaryLagPortBitMapPtrIdx = 0;
        return SAI_STATUS_SUCCESS;
    }

    lagInfo->secondaryLagPortBitMapPtrIdx = portBitMapIndex;

    return SAI_STATUS_SUCCESS;
}



/**
 * \brief Update primary redirect LAG primary port list
 *
 * \param [in] xpsInterfaceId_t primaryLagId - LAG interface
 * \param [in] xpsInterfaceId_t port - interface to update with
 * \param [in] bool isAdded - Add or remove interface to/from the list
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiLagInfoPrimaryPortListUpdate(xpsInterfaceId_t
                                                      primaryLagId, xpsInterfaceId_t port, bool isAdded)
{
    XP_STATUS      xpsRetVal  = XP_NO_ERR;
    xpSaiLagInfo_t lagInfoKey;
    xpSaiLagInfo_t *lagInfo   = NULL;
    uint32_t       i          = 0;
    uint32_t       j          = 0;

    /* Get the Primary Lag information */
    lagInfoKey.lagId = primaryLagId;
    xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiLagInfoDbHandle,
                                   (xpsDbKey_t)&lagInfoKey, (void**)&lagInfo);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Primary Lag interface failed, lag interface(%d)",
                       primaryLagId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!lagInfo)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (!lagInfo->isLagEgressRedirectEnabled)
    {
        XP_SAI_LOG_ERR("LAG %u is not in redirect state\n", primaryLagId);
        return SAI_STATUS_FAILURE;
    }

    if (lagInfo->primaryLagId != XPS_INTF_INVALID_ID)
    {
        XP_SAI_LOG_ERR("LAG %u is not primary\n", primaryLagId);
        return SAI_STATUS_FAILURE;
    }

    /* Perform primary port list update */
    if (isAdded) /* Interface is added */
    {
        lagInfo->primaryLagEgressPortList.size++;
        lagInfo->primaryLagEgressPortList.portList[lagInfo->primaryLagEgressPortList.size
                                                   - 1] = port;
    }
    else /* Interface is removed */
    {
        for (i = 0; i < lagInfo->primaryLagEgressPortList.size; i++)
        {
            if (lagInfo->primaryLagEgressPortList.portList[i] == port)
            {
                /* Move list elements one place left */
                for (j = i+1; j < lagInfo->primaryLagEgressPortList.size; j++)
                {
                    lagInfo->primaryLagEgressPortList.portList[j-1] =
                        lagInfo->primaryLagEgressPortList.portList[j];
                }
                lagInfo->primaryLagEgressPortList.size--;

                return SAI_STATUS_SUCCESS;
            }
        }
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Check LAG interface redirect flag
 *
 * \param [in] xpsInterfaceId_t lagId - LAG id to set redirect status
 *
 * \return bool - true if redirect flag is set
 */
static bool xpSaiLagRedirectStatusCheck(xpsInterfaceId_t lagId)
{
    sai_status_t        saiRetVal      = SAI_STATUS_SUCCESS;
    bool                redirectStatus = false;
    xpsDevice_t         devId          = xpSaiGetDevId();

    saiRetVal = xpSaiLagEgressRedirectStatusGet(devId, lagId, &redirectStatus);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Get LAG %u redirection status failed with error %d\n", lagId,
                       saiRetVal);
        return false;
    }

    return redirectStatus;
}

/**
 * \brief Verify if redirect LAG is primary
 *
 * This API assumes that LAG is in redirect state
 *
 * \param [in] xpsInterfaceId_t lagId - LAG Id
 *
 * \return bool - true if redirect LAG is primary
 */
static bool xpSaiLagRedirectIsPrimaryCheck(xpsInterfaceId_t lagId)
{
    XP_STATUS      xpsRetVal    = XP_NO_ERR;
    xpSaiLagInfo_t lagInfoKey;
    xpSaiLagInfo_t *lagInfo     = NULL;
    bool           isPrimaryLag = false;

    /* Get the Primary Lag information */
    lagInfoKey.lagId = lagId;
    xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiLagInfoDbHandle,
                                   (xpsDbKey_t)&lagInfoKey, (void**)&lagInfo);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Primary Lag interface failed, lag interface(%d)",
                       lagId);
        return false;
    }

    if (!lagInfo)
    {
        return false;
    }

    isPrimaryLag = lagInfo->primaryLagId ==
                   XPS_INTF_INVALID_ID; // Primary LAG has primaryLagId value set to XPS_INTF_INVALID_ID

    return isPrimaryLag;
}

/**
 * \brief Save LAG interface blocked port list
 *
 * \param [in] xpsInterfaceId_t lagId - LAG Id
 * \param [in] xpsPortList_t blockedPortList
 *
 * \return sai_status_t
 */
sai_status_t xpSaiLagInfoBlockedPortListSet(xpsInterfaceId_t lagId,
                                            xpsPortList_t *blockedPortList, uint32_t enable)
{
    XP_STATUS      xpsRetVal    = XP_NO_ERR;
    xpSaiLagInfo_t lagInfoKey;
    xpSaiLagInfo_t *lagInfo     = NULL;

    /* Get the Primary Lag information */
    lagInfoKey.lagId = lagId;
    xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiLagInfoDbHandle,
                                   (xpsDbKey_t)&lagInfoKey, (void**)&lagInfo);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Lag %u interface failed", lagId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    if (!lagInfo)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    lagInfo->egressBlockPortList = *blockedPortList;

    if (enable)
    {
        lagInfo->isEgressBlockListEn = true;
    }
    else
    {
        lagInfo->isEgressBlockListEn = false;
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Retrieve LAG interface blocked port list
 *
 * \param [in] xpsInterfaceId_t lagId - LAG Id
 * \param [out] xpsPortList_t *blockedPortList
 *
 * \return sai_status_t
 */
sai_status_t xpSaiLagInfoBlockedPortListGet(xpsInterfaceId_t lagId,
                                            xpsPortList_t *blockedPortList)
{
    XP_STATUS      xpsRetVal    = XP_NO_ERR;
    xpSaiLagInfo_t lagInfoKey;
    xpSaiLagInfo_t *lagInfo     = NULL;

    /* Get the Primary Lag information */
    lagInfoKey.lagId = lagId;
    xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiLagInfoDbHandle,
                                   (xpsDbKey_t)&lagInfoKey, (void**)&lagInfo);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Lag interface %u failed", lagId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!lagInfo)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *blockedPortList = lagInfo->egressBlockPortList;

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Verify LAG egress blocking state
 *
 * \param [in] xpsInterfaceId_t lagId - LAG Id
 *
 * \return bool - true if egress is blocked on LAG
 */
static sai_status_t xpSaiLagEgressBlockedCheck(xpsInterfaceId_t lagId)
{
    XP_STATUS      xpsRetVal    = XP_NO_ERR;
    xpSaiLagInfo_t lagInfoKey;
    xpSaiLagInfo_t *lagInfo     = NULL;
    bool           isBlocked    = false;

    /* Get the Primary Lag information */
    lagInfoKey.lagId = lagId;
    xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiLagInfoDbHandle,
                                   (xpsDbKey_t)&lagInfoKey, (void**)&lagInfo);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Lag interface %u failed", lagId);
        return false;
    }

    if (!lagInfo)
    {
        return false;
    }

    isBlocked = lagInfo->isEgressBlockListEn;

    return isBlocked;
}

/**
 * \brief Verify LAG egress blocking state and update port blocking mask if required
 * Required because we re-deploy LAG each time we update LAG members
 *
 * \param [in] xpsDevice_t      devId - Device Id
 * \param [in] xpsInterfaceId_t lagId - LAG Id
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiLagBlockedCheckAndUpdate(xpsDevice_t devId,
                                                  xpsInterfaceId_t lagId)
{
    sai_status_t   saiRetVal  = SAI_STATUS_SUCCESS;
    XP_STATUS      xpsRetVal  = XP_NO_ERR;
    sai_uint32_t   countIdx   = 0;
    xpsPortList_t  blockedPortList, intfBlockedPortList;

    memset(&blockedPortList, 0, sizeof(xpsPortList_t));
    memset(&intfBlockedPortList, 0, sizeof(xpsPortList_t));

    // If LAG is egress blocked (i.e. ACL IN_PORT), then add ACL blocked port list
    if (xpSaiLagEgressBlockedCheck(lagId))
    {
        saiRetVal = xpSaiLagInfoBlockedPortListGet(lagId, &blockedPortList);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get blocked port list with error %d\n", saiRetVal);
            return saiRetVal;
        }

        // Retrieve currently blocked ports on the interface
        xpsRetVal = xpsInterfaceGetIngressPorts(devId, lagId, &intfBlockedPortList);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not get blocked ports list for port: (%u), ret %d\n",
                           lagId, xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }

        for (countIdx = 0; countIdx < blockedPortList.size; countIdx++)
        {
            /* If we have port as part of IN_PORTs of ACL
             * and try to add it to a LAG which is member of IN_PORTs ACL
             * we should return error.
             * Otherwise we may be able to remove the blocking from the previously standalone port
             * and that will allow traffic output to the port, despite it is LAG member
             */
            saiRetVal = xpSaiAddPortToPortList(&intfBlockedPortList,
                                               blockedPortList.portList[countIdx]);

            if (saiRetVal == SAI_STATUS_ITEM_ALREADY_EXISTS)
            {
                XP_SAI_LOG_ERR("Could not add ACL-blocked port %d to ACL-blocked LAG!\n",
                               blockedPortList.portList[countIdx]);
                return SAI_STATUS_INVALID_PARAMETER;
            }
            else if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not add port %d to ACL-blocked LAG!, ret = %d\n",
                               blockedPortList.portList[countIdx], saiRetVal);
                return saiRetVal;
            }
        }

        // Apply blocked port list to LAG
        xpsRetVal = xpsInterfaceSetIngressPorts(devId, lagId, &intfBlockedPortList);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not set blocked ports list for interface: (%u), ret %d\n",
                           lagId, xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }

    return saiRetVal;
}

/**
 * \brief Retrieve LAG config
 *
 * \param [in] xpsInterfaceId_t lagId - LAG Id
 * \param [out] xpSaiPortLagInfo_t *lagConfig
 *
 * \return sai_status_t
 */
sai_status_t xpSaiLagConfigInfoGet(xpsInterfaceId_t lagId,
                                   xpSaiLagInfo_t **lagConfig)
{
    XP_STATUS      xpsRetVal    = XP_NO_ERR;
    xpSaiLagInfo_t lagInfoKey;

    /* Get the Primary Lag information */
    lagInfoKey.lagId = lagId;
    xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiLagInfoDbHandle,
                                   (xpsDbKey_t)&lagInfoKey, (void**)lagConfig);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for Lag interface %u failed", lagId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!(*lagConfig))
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*     Cteates SAI member Object ID. by taking input as LAG ID and PortID
*
* Arguments:
*     [in] sai_object_type_t : object type
*     [in] sai_uint64_t : lagId
*     [in] sai_uint64_t : only 24 bits of income value are being used
*     [out] sai_object_id_t
*
* Return Values:
*    sai_status_t
*/
sai_status_t xpSaiLagMemberObjIdCreate(sai_object_type_t sai_object_type,
                                       sai_uint64_t lag_id,  sai_uint64_t port_id, sai_object_id_t *sai_object_id)
{
    if ((SAI_OBJECT_TYPE_NULL >= sai_object_type) &&
        (SAI_OBJECT_TYPE_MAX <= sai_object_type))
    {
        XP_SAI_LOG_ERR("Invalid SAI object type received(%u).", sai_object_type);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((lag_id > SAI_LAG_MEMBER_OBJ_VALUE_MASK) ||
        (port_id > SAI_LAG_MEMBER_OBJ_VALUE_MASK))
    {
        XP_SAI_LOG_ERR("Provided local identifier is to big(lag_id %lu, port_id %lu) and can not be paked. Only 24 bits are allowed.",
                       (long unsigned int) lag_id, (long unsigned int) port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *sai_object_id = ((((sai_uint64_t)sai_object_type <<
                        SAI_LAG_MEMBER_SUB_OBJID_VALUE_BITS) | lag_id) <<
                      SAI_LAG_MEMBER_SUB_OBJID_VALUE_BITS) | port_id;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiLagRef
sai_status_t xpSaiLagRef(xpsInterfaceId_t xpsLagId)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiLagInfo_t *lagConfig = NULL;
    xpsInterfaceType_e type = XPS_PORT;

    retVal = xpsInterfaceGetType(xpsLagId, &type);
    if (XP_NO_ERR != retVal)
    {
        XP_SAI_LOG_ERR("Failed to get type for interface %u, error: %d\n", xpsLagId,
                       retVal);
        return retVal;
    }

    if (XPS_LAG != type)
    {
        XP_SAI_LOG_ERR("Invalid object type received, type: %d\n", type);
        return XP_ERR_INVALID_ID;
    }

    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    lagConfig->refCnt++;

    return saiRetVal;
}

//Func: xpSaiLagUnref
sai_status_t xpSaiLagUnref(xpsInterfaceId_t xpsLagId)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiLagInfo_t *lagConfig = NULL;
    xpsInterfaceType_e type = XPS_PORT;

    retVal = xpsInterfaceGetType(xpsLagId, &type);
    if (XP_NO_ERR != retVal)
    {
        XP_SAI_LOG_ERR("Failed to get type for interface %u, error: %d\n", xpsLagId,
                       retVal);
        return retVal;
    }

    if (XPS_LAG != type)
    {
        XP_SAI_LOG_ERR("Invalid object type received, type: %d\n", type);
        return XP_ERR_INVALID_ID;
    }

    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    if (lagConfig->refCnt)
    {
        lagConfig->refCnt--;
    }
    else
    {
        XP_SAI_LOG_ERR("Lag reference counter is already 0. xpsLagId=%u, error=%u\n",
                       xpsLagId, saiRetVal);
        saiRetVal = SAI_STATUS_INVALID_OBJECT_ID;
    }

    return saiRetVal;
}

//Func: xpSaiUpdateLagAttributeVals

static void xpSaiUpdateLagAttributeVals(const uint32_t attr_count,
                                        const sai_attribute_t* attr_list, xpSaiPortLagInfo_t* attributes)
{
    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);
    const sai_uint16_t defaultPvid = 1;
    attributes->autoDistribute = true;

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_LAG_ATTR_PORT_VLAN_ID:
                {
                    /* Set default PVID if not specified */
                    if (!attr_list[count].value.u16)
                    {
                        attributes->portVlanId = defaultPvid;
                    }
                    else
                    {
                        attributes->portVlanId = attr_list[count].value.u16;
                    }
                    break;
                }
            case SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY:
                {
                    attributes->defaultVlanPriority = attr_list[count].value.u8;
                    break;
                }
            case SAI_LAG_ATTR_DROP_UNTAGGED:
                {
                    attributes->dropUntagged = attr_list[count].value.booldata;
                    break;
                }
            case SAI_LAG_ATTR_DROP_TAGGED:
                {
                    attributes->dropTagged = attr_list[count].value.booldata;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiSetLagConfig
sai_status_t xpSaiSetLagConfig(xpsInterfaceId_t xpsLagId,
                               xpSaiPortLagInfo_t lagConfig)
{
    sai_status_t            saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS               xpStatus  = XP_NO_ERR;
    xpsLagPortIntfList_t    *portList = NULL;
    sai_object_id_t         portOid   = SAI_NULL_OBJECT_ID;
    xpsDevice_t             devId     = xpSaiGetDevId();

    portList = (xpsLagPortIntfList_t *)xpMalloc(sizeof(xpsLagPortIntfList_t));
    if (!portList)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for portList\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(portList, 0, sizeof(xpsLagPortIntfList_t));

    if (!xpSaiLagRedirectStatusCheck(xpsLagId))
    {
        xpStatus = xpsLagGetPortIntfList(xpsLagId, portList);
        if (xpStatus != XP_NO_ERR)
        {
            xpFree(portList);
            XP_SAI_LOG_ERR("could not get Lag Portlist\n");
            return xpsStatus2SaiStatus(xpStatus);
        }
    }
    else
    {
        if (xpSaiLagRedirectIsPrimaryCheck(xpsLagId))
        {
            saiStatus = xpSaiLagInfoPrimaryPortListGet(xpsLagId, (xpsPortList_t *)portList);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to update LAG %u primary port list, error %u\n",
                               xpsLagId, saiStatus);
                xpFree(portList);
                return saiStatus;
            }
        }
        else
        {
            xpStatus = xpsLagGetPortIntfList(xpsLagId, portList);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("could not get Lag Portlist\n");
                xpFree(portList);
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
    }

    for (uint32_t i = 0; i < portList->size; i++)
    {
        saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId, portList->portIntf[i],
                                     &portOid);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: SAI port object could not be created.\n");
            xpFree(portList);
            return saiStatus;
        }

        saiStatus = xpSaiSetPortLagConfig(portOid, lagConfig);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Could not apply port config on port: %lu.\n", portOid);
            xpFree(portList);
            return saiStatus;
        }
    }
    xpFree(portList);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateLag

sai_status_t xpSaiCreateLag(sai_object_id_t* lag_id, sai_object_id_t switch_id,
                            uint32_t attr_count, const sai_attribute_t* attr_list)
{
    XP_STATUS           retVal      = XP_NO_ERR;
    sai_status_t        retValSai   = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t    xpsLagId;
    xpsDevice_t         devId       = xpSaiObjIdSwitchGet(switch_id);
    xpSaiPortLagInfo_t  lagConfig;

    if (lag_id == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retValSai = xpSaiAttrCheck(attr_count, attr_list,
                               LAG_VALIDATION_ARRAY_SIZE, lag_attribs,
                               SAI_COMMON_API_CREATE);
    if (retValSai != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retValSai);
        return retValSai;
    }

    xpSaiUpdateLagAttributeVals(attr_count, attr_list, &lagConfig);

    retVal = (XP_STATUS) xpsLagCreate(&xpsLagId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsLagCreate failed lagId %d | retVal : %d \n", xpsLagId,
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retValSai = xpSaiLagInfoNodeCreate(xpsLagId, lagConfig.autoDistribute);
    if (retValSai != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create LAG info Db node with error %d\n", retValSai);
        return retValSai;
    }

    lagConfig.saMissCmd = XP_PKTCMD_MAX;

    retValSai = xpSaiSetLagConfig(xpsLagId, lagConfig);
    if (retValSai != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set lag config. Error %d\n", retValSai);
        return retValSai;
    }

    retValSai = xpSaiObjIdCreate(SAI_OBJECT_TYPE_LAG, devId,
                                 (sai_uint64_t) xpsLagId, lag_id);

    return retValSai;
}

//Func: xpSaiRemoveLag

sai_status_t xpSaiRemoveLag(sai_object_id_t lag_id)
{
    XP_STATUS           retVal = XP_NO_ERR;
    xpsInterfaceId_t    xpsLagId;
    sai_status_t        saiRetVal = SAI_STATUS_SUCCESS;

    if (!XDK_SAI_OBJID_TYPE_CHECK(lag_id, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(lag_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsLagId = (xpsInterfaceId_t)xpSaiObjIdValueGet(lag_id);

    XP_SAI_LOG_DBG("%s: lag_id %d \n", __FUNCNAME__, xpsLagId);

    /* Check if some component still uses this LAG */
    if (xpSaiLagInfoDependenciesCheck(xpsLagId))
    {
        XP_SAI_LOG_ERR("LAG OID %lu is still is use. Please remove all references to this object.\n",
                       lag_id);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    saiRetVal = xpSaiLagInfoNodeRemove(xpsLagId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove LAG info Db node with error %d\n", saiRetVal);
        return saiRetVal;
    }

    retVal = (XP_STATUS) xpsLagDestroy(xpsLagId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsLagDestroy failed lagId %d | retVal : %d \n", xpsLagId,
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiPortNotifyAddIngressAcl

sai_status_t xpSaiLagNotifyAddAcl(xpsDevice_t devId,
                                  sai_object_id_t lagObjId, sai_object_id_t aclObjId)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    saiStatus = xpSaiAclMapperBindToTableGroup(devId, aclObjId, lagObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to bind lag to table group, status:%d\n", saiStatus);
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetLagAttrAcl

sai_status_t xpSaiSetLagAttrAcl(sai_object_id_t lagOid,
                                sai_attribute_value_t value, bool isIngress)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus    = XP_NO_ERR;
    sai_uint32_t        tableId     = 0;
    sai_object_id_t     aclObjId    = value.oid;
    xpSaiLagInfo_t     *pLagEntry   = NULL;
    xpsDevice_t         devId       = xpSaiGetDevId();
    xpsInterfaceId_t    lagId       = 0;
    sai_uint32_t        groupId     = 0;
    sai_object_id_t prevOid = SAI_NULL_OBJECT_ID;

    XP_SAI_LOG_DBG("Calling xpSaiSetLagAttrAcl\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(lagOid, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).",
                       xpSaiObjIdTypeGet(lagOid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    lagId = (xpsInterfaceId_t)xpSaiObjIdValueGet(lagOid);

    saiStatus = xpSaiLagConfigInfoGet(lagId, &pLagEntry);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", lagId, saiStatus);
        return saiStatus;
    }

    if (isIngress)
    {
        if (pLagEntry->ingressAclId == aclObjId)
        {
            return SAI_STATUS_SUCCESS;
        }
        prevOid = pLagEntry->ingressAclId;
    }
    else
    {
        if (pLagEntry->egressAclId == aclObjId)
        {
            return SAI_STATUS_SUCCESS;
        }
        prevOid = pLagEntry->egressAclId;
    }

    if (prevOid != SAI_NULL_OBJECT_ID)
    {
        if (XDK_SAI_OBJID_TYPE_CHECK(prevOid, SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
        {
            saiStatus = xpSaiAclMapperUnBindFromTableGroup(devId, prevOid,
                                                           lagOid);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to notify add ingress acl, port:%d, status:%d\n",
                               lagOid, saiStatus);
                return saiStatus;
            }
        }
        else if (XDK_SAI_OBJID_TYPE_CHECK(prevOid, SAI_OBJECT_TYPE_ACL_TABLE))
        {
            tableId = (uint32_t)xpSaiObjIdValueGet(prevOid);
            xpStatus = xpsAclLagUnbind(devId, lagId, groupId, tableId);

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
            saiStatus = xpSaiLagNotifyAddAcl(devId, lagOid, aclObjId);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_DBG("Failed to notify add acl, lag:%d, status:%d\n", lagId,
                               saiStatus);
                return saiStatus;
            }
        }
        else
        {
            // Binding ACL table to lag
            // Get the acl table id from table object
            tableId = (xpsInterfaceId_t)xpSaiObjIdValueGet(aclObjId);
            xpStatus = xpsAclSetLagAcl(devId, lagId, tableId, groupId);
            if (xpStatus != XP_NO_ERR)
            {
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
    }

    // Update state database
    if (isIngress)
    {
        pLagEntry->ingressAclId = aclObjId;
    }
    else
    {
        pLagEntry->egressAclId = aclObjId;
    }

    return saiStatus;
}

//Func: xpSaiGetLagAttrAcl

sai_status_t xpSaiGetLagAttrAcl(xpsInterfaceId_t lagId,
                                sai_attribute_value_t* value, bool isIngress)
{
    xpSaiLagInfo_t *pLagEntry = NULL;
    sai_status_t    saiStatus = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiGetLagAttrAcl\n");

    saiStatus = xpSaiLagConfigInfoGet(lagId, &pLagEntry);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", lagId, saiStatus);
        return saiStatus;
    }

    if (isIngress)
    {
        value->oid = pLagEntry->ingressAclId;
    }
    else
    {
        value->oid = pLagEntry->egressAclId;
    }
    return saiStatus;
}

//Func: xpSaiSetLagAttrPortVlanId
sai_status_t xpSaiSetLagAttrPortVlanId(sai_object_id_t lagId,
                                       sai_attribute_value_t value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t xpsLagId = 0;
    xpSaiLagInfo_t *lagConfig = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiSetLagAttrPortVlanId\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(lagId, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(lagId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsLagId = (xpsInterfaceId_t)xpSaiObjIdValueGet(lagId);

    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    lagConfig->lagConfig.portVlanId = value.u16;

    saiRetVal = xpSaiSetLagConfig(xpsLagId, lagConfig->lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    return saiRetVal;
}

//Func: xpSaiGetLagAttrPortVlanId
sai_status_t xpSaiGetLagAttrPortVlanId(sai_object_id_t lagId,
                                       sai_attribute_value_t* value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t xpsLagId = 0;
    xpSaiLagInfo_t *lagConfig = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiGetLagAttrPortVlanId\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(lagId, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(lagId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsLagId = (xpsInterfaceId_t)xpSaiObjIdValueGet(lagId);

    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    value->u16 = lagConfig->lagConfig.portVlanId;

    return saiRetVal;
}

sai_status_t xpSaiSetLagPVIDtoDB(sai_object_id_t lagId,
                                 sai_attribute_value_t value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t xpsLagId = 0;
    xpSaiLagInfo_t *lagConfig = NULL;
    if (!XDK_SAI_OBJID_TYPE_CHECK(lagId, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(lagId));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    xpsLagId = (xpsInterfaceId_t)xpSaiObjIdValueGet(lagId);

    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    lagConfig->pvidUserSetting = value.u16;
    return saiRetVal;
}

//Func: xpSaiSetLagAttrDefaultVlanPriority
sai_status_t xpSaiSetLagAttrDefaultVlanPriority(sai_object_id_t lagId,
                                                sai_attribute_value_t value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t xpsLagId = 0;
    xpSaiLagInfo_t *lagConfig = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiSetLagAttrDefaultVlanPriority\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(lagId, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(lagId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsLagId = (xpsInterfaceId_t)xpSaiObjIdValueGet(lagId);

    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %d\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    lagConfig->lagConfig.defaultVlanPriority = value.u8;

    saiRetVal = xpSaiSetLagConfig(xpsLagId, lagConfig->lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set config for LAG %u, error %d\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    return saiRetVal;
}

//Func: xpSaiGetLagAttrDefaultVlanPriority
sai_status_t xpSaiGetLagAttrDefaultVlanPriority(sai_object_id_t lagId,
                                                sai_attribute_value_t* value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t xpsLagId = 0;
    xpSaiLagInfo_t *lagConfig = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiGetLagAttrDefaultVlanPriority\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(lagId, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(lagId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsLagId = (xpsInterfaceId_t)xpSaiObjIdValueGet(lagId);

    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    value->u8 = lagConfig->lagConfig.defaultVlanPriority;

    return saiRetVal;
}

//Func: xpSaiSetLagAttrDropUntagged
sai_status_t xpSaiSetLagAttrDropUntagged(sai_object_id_t lagId,
                                         sai_attribute_value_t value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t xpsLagId = 0;
    xpSaiLagInfo_t *lagConfig = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiSetLagAttrDropUntagged\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(lagId, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(lagId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsLagId = (xpsInterfaceId_t)xpSaiObjIdValueGet(lagId);

    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    lagConfig->lagConfig.dropUntagged = value.booldata;

    saiRetVal = xpSaiSetLagConfig(xpsLagId, lagConfig->lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    return saiRetVal;
}

//Func: xpSaiGetLagAttrDropUntagged
sai_status_t xpSaiGetLagAttrDropUntagged(sai_object_id_t lagId,
                                         sai_attribute_value_t* value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t xpsLagId = 0;
    xpSaiLagInfo_t *lagConfig = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiGetLagAttrDropUntagged\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(lagId, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(lagId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsLagId = (xpsInterfaceId_t)xpSaiObjIdValueGet(lagId);

    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    value->booldata = lagConfig->lagConfig.dropUntagged;

    return saiRetVal;
}

//Func: xpSaiSetLagAttrDropTagged
sai_status_t xpSaiSetLagAttrDropTagged(sai_object_id_t lagId,
                                       sai_attribute_value_t value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t xpsLagId = 0;
    xpSaiLagInfo_t *lagConfig = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiSetLagAttrDropTagged\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(lagId, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(lagId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsLagId = (xpsInterfaceId_t)xpSaiObjIdValueGet(lagId);

    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    lagConfig->lagConfig.dropTagged = value.booldata;

    saiRetVal = xpSaiSetLagConfig(xpsLagId, lagConfig->lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    return saiRetVal;
}

//Func: xpSaiGetLagAttrDropTagged
sai_status_t xpSaiGetLagAttrDropTagged(sai_object_id_t lagId,
                                       sai_attribute_value_t* value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t xpsLagId = 0;
    xpSaiLagInfo_t *lagConfig = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiGetLagAttrDropTagged\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(lagId, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(lagId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsLagId = (xpsInterfaceId_t)xpSaiObjIdValueGet(lagId);

    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    value->booldata = lagConfig->lagConfig.dropTagged;

    return saiRetVal;
}

sai_status_t xpSaiGetLagAttrAutoDistribute(sai_object_id_t lagId,
                                           sai_attribute_value_t* value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t xpsLagId = 0;
    xpSaiLagInfo_t *lagConfig = NULL;

    if (!XDK_SAI_OBJID_TYPE_CHECK(lagId, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(lagId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsLagId = (xpsInterfaceId_t)xpSaiObjIdValueGet(lagId);

    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    value->booldata = lagConfig->lagConfig.autoDistribute;

    return saiRetVal;
}

/**
 * \brief Set SA miss command on LAG interface
 *
 * \param [in] xpsDevice_t devId
 * \param [in] xpsInterfaceId_t lag - LAG interface
 * \param [in] xpsPktCmd_e pktCmd - packet command
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiLagSetSaMissCmd(xpsInterfaceId_t xpsLagId, xpsPktCmd_e pktCmd)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiLagInfo_t *lagConfig = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiLagSetSaMissCmd\n");

    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return XP_PORT_NOT_VALID;
    }

    lagConfig->lagConfig.saMissCmd = pktCmd;

    saiRetVal = xpSaiSetLagConfig(xpsLagId, lagConfig->lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set config for LAG %u, error %u\n", xpsLagId,
                       saiRetVal);
        return XP_PORT_NOT_VALID;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSetLagAttribute

sai_status_t xpSaiSetLagAttribute(sai_object_id_t lag_id,
                                  const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if (!XDK_SAI_OBJID_TYPE_CHECK(lag_id, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(lag_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(1, attr,
                            LAG_VALIDATION_ARRAY_SIZE, lag_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    switch (attr->id)
    {
        case SAI_LAG_ATTR_PORT_LIST:
            {
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Read only attribute %u received on set attribute call.\n",
                                   attr->id);
                    return SAI_STATUS_INVALID_ATTRIBUTE_0;
                }
                break;
            }
        case SAI_LAG_ATTR_INGRESS_ACL:
            {
                retVal = xpSaiSetLagAttrAcl(lag_id, attr->value, true);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_LAG_ATTR_INGRESS_ACL)\n");
                    return retVal;
                }
                break;
            }
        case SAI_LAG_ATTR_EGRESS_ACL:
            {
                retVal = xpSaiSetLagAttrAcl(lag_id, attr->value, false);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_LAG_ATTR_EGRESS_ACL)\n");
                    return retVal;
                }
                break;
            }
        case SAI_LAG_ATTR_PORT_VLAN_ID:
            {
                retVal = xpSaiSetLagAttrPortVlanId(lag_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_LAG_ATTR_PORT_VLAN_ID)\n");
                    return retVal;
                }
                retVal = xpSaiSetLagPVIDtoDB(lag_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_LAG_ATTR_PORT_VLAN_ID) to DB\n");
                    return retVal;
                }
                break;
            }
        case SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY:
            {
                retVal = xpSaiSetLagAttrDefaultVlanPriority(lag_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY)\n");
                    return retVal;
                }
                break;
            }
        case SAI_LAG_ATTR_DROP_UNTAGGED:
            {
                retVal = xpSaiSetLagAttrDropUntagged(lag_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_LAG_ATTR_DROP_UNTAGGED)\n");
                    return retVal;
                }
                break;
            }
        case SAI_LAG_ATTR_DROP_TAGGED:
            {
                retVal = xpSaiSetLagAttrDropTagged(lag_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_LAG_ATTR_DROP_TAGGED)\n");
                    return retVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attribute %d recieved\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetLagAttributePortList

sai_status_t xpSaiGetLagAttrPortList(xpsInterfaceId_t xpsLagId,
                                     sai_object_list_t* pSaiObjList)
{
    sai_status_t            saiStatus   = SAI_STATUS_SUCCESS;
    XP_STATUS               xpStatus    = XP_NO_ERR;
    xpsLagPortIntfList_t    *portList = NULL;

    portList = (xpsLagPortIntfList_t *)xpMalloc(sizeof(xpsLagPortIntfList_t));
    if (!portList)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for portList\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(portList, 0, sizeof(xpsLagPortIntfList_t));

    if ((NULL == pSaiObjList) || (0 == pSaiObjList->count))
    {
        XP_SAI_LOG_ERR("Invalid parameters received.");
        xpFree(portList);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (!xpSaiLagRedirectStatusCheck(xpsLagId))
    {
        xpStatus = xpsLagGetPortIntfList(xpsLagId, portList);
        if (xpStatus != XP_NO_ERR)
        {
            xpFree(portList);
            XP_SAI_LOG_ERR("could not get Lag Portlist\n");
            return xpsStatus2SaiStatus(xpStatus);
        }
    }
    else
    {
        if (xpSaiLagRedirectIsPrimaryCheck(xpsLagId))
        {
            saiStatus = xpSaiLagInfoPrimaryPortListGet(xpsLagId, (xpsPortList_t *)portList);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to update LAG %u primary port list, error %u\n",
                               xpsLagId, saiStatus);
                xpFree(portList);
                return saiStatus;
            }
        }
        else
        {
            xpStatus = xpsLagGetPortIntfList(xpsLagId, portList);

            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("could not get Lag Portlist\n");
                xpFree(portList);
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
    }

    if (pSaiObjList->count < portList->size)
    {
        xpFree(portList);
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    pSaiObjList->count = portList->size;

    for (uint32_t i=0; i<pSaiObjList->count; i++)
    {
        saiStatus = xpSaiLagMemberObjIdCreate(SAI_OBJECT_TYPE_LAG_MEMBER, xpsLagId,
                                              portList->portIntf[i], &pSaiObjList->list[i]);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error : SAI port object could not be created.\n");
            xpFree(portList);
            return saiStatus;
        }

    }
    xpFree(portList);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetLagAttribute

sai_status_t xpSaiGetLagAttribute(sai_object_id_t lag_id, uint32_t attr_count,
                                  sai_attribute_t *attr_list)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t    xpsLagId    = 0;

    XP_SAI_LOG_DBG("Calling xpSaiGetLagAttribute\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(lag_id, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(lag_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               LAG_VALIDATION_ARRAY_SIZE, lag_attribs,
                               SAI_COMMON_API_GET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    xpsLagId  = (xpsInterfaceId_t)xpSaiObjIdValueGet(lag_id);

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_LAG_ATTR_PORT_LIST:
                {
                    saiStatus = xpSaiGetLagAttrPortList(xpsLagId, &attr_list[count].value.objlist);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("xpSaiGetLagAttrPortList call failed!\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_LAG_ATTR_INGRESS_ACL:
                {
                    saiStatus = xpSaiGetLagAttrAcl(xpsLagId, &attr_list[count].value, true);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_LAG_ATTR_INGRESS_ACL)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_LAG_ATTR_EGRESS_ACL:
                {
                    saiStatus = xpSaiGetLagAttrAcl(xpsLagId, &attr_list[count].value, false);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_LAG_ATTR_EGRESS_ACL)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_LAG_ATTR_PORT_VLAN_ID:
                {
                    saiStatus = xpSaiGetLagAttrPortVlanId(lag_id, &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_LAG_ATTR_PORT_VLAN_ID)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY:
                {
                    saiStatus = xpSaiGetLagAttrDefaultVlanPriority(lag_id, &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_LAG_ATTR_DROP_UNTAGGED:
                {
                    saiStatus = xpSaiGetLagAttrDropUntagged(lag_id, &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_LAG_ATTR_DROP_UNTAGGED)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_LAG_ATTR_DROP_TAGGED:
                {
                    saiStatus = xpSaiGetLagAttrDropTagged(lag_id, &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_LAG_ATTR_DROP_TAGGED)\n");
                        return saiStatus;
                    }
                    break;
                }

            default:
                {
                    XP_SAI_LOG_ERR("Invalid parameter %d for function %s received.\n",
                                   attr_list[count].id, __FUNCNAME__);
                    return  SAI_STATUS_INVALID_PARAMETER;
                }
        }
    }

    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*     Query LAG ID from sai object value.
*
* Arguments:
*     [out] sai_object_id_t
*
* Return Values:
*    sai_uint64_t : only 48 bits of income value are usable
*/
sai_uint64_t xpSaiLagMemberLagIdValueGet(sai_object_id_t sai_object_id)
{
    /*Income validation haven't sense as there will be present upcoming validation for the local identifier.*/
    return (sai_object_id >> SAI_LAG_MEMBER_SUB_OBJID_VALUE_BITS) &
           SAI_LAG_MEMBER_OBJ_VALUE_MASK;
}

/*
* Routine Description:
*     Query Port ID from sai object value.
*
* Arguments:
*     [out] sai_object_id_t
*
* Return Values:
*    sai_uint64_t : only 48 bits of income value are usable
*/
sai_uint64_t xpSaiLagMemberPortIdValueGet(sai_object_id_t sai_object_id)
{
    /*Income validation haven't sense as there will be present upcoming validation for the local identifier.*/
    return sai_object_id & SAI_LAG_MEMBER_OBJ_VALUE_MASK;
}

/**
 * \brief Enable traffic redirection between LAG interfaces
 *
 * \param [in] xpsDevice_t xpsDevId
 * \param [in] xpsInterfaceId_t primaryLag - LAG interface to switch traffic from
 * \param [in] xpsInterfaceId_t secondaryLag - LAG interface to switch traffic to
 *
 * \return sai_status_t
 */
sai_status_t xpSaiLagRedirectEnable(xpsDevice_t devId,
                                    xpsInterfaceId_t primaryLag, xpsInterfaceId_t secondaryLag)
{
    sai_status_t        saiRetVal      = SAI_STATUS_SUCCESS;
    XP_STATUS           xpsRetVal      = XP_NO_ERR;
    uint32_t            i              = 0;
    xpsPortList_t       primaryLagPortList;
    xpsPortList_t       secondaryLagPortList;
    uint32_t secondaryLagPortBitMapPtrIdx=0, primaryLagPortBitMapPtrIdx=0;

    memset(&primaryLagPortList, 0, sizeof(xpsPortList_t));
    memset(&secondaryLagPortList, 0, sizeof(xpsPortList_t));

    /* Set LAG redirection data. */
    saiRetVal = xpSaiLagInfoSet(devId, primaryLag, secondaryLag, true);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Set LAG %u redirection data failed with error %d\n", primaryLag,
                       saiRetVal);
        return saiRetVal;
    }

    /* Get egress port List of Primary LAG */
    xpsRetVal = xpsInterfaceGetPorts(devId, primaryLag, &primaryLagPortList);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get LAG %d port list, error %u", primaryLag,
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    // Remove primary LAG ports from primary LAG and save them to SAI Db
    for (i = 0; i < primaryLagPortList.size; i++)
    {
        /* Update primary LAG primary port list */
        saiRetVal = xpSaiLagInfoPrimaryPortListUpdate(primaryLag,
                                                      primaryLagPortList.portList[i], true /*add*/);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to update LAG %u primary port list, error %u\n",
                           primaryLag, saiRetVal);
            return saiRetVal;
        }
    }
    /* Update primary LAG lagPortBitMapPtrIdx */
    saiRetVal = xpSaiLagInfoLagPortBitMapPointerUpdate(primaryLag,
                                                       primaryLagPortBitMapPtrIdx);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update LAG %u primary port bitMapPointer, error %u\n",
                       primaryLag, saiRetVal);
        return saiRetVal;
    }


    /* Update secondary LAG lagPortBitMapPtrIdx */
    saiRetVal = xpSaiLagInfoLagPortBitMapPointerUpdate(secondaryLag,
                                                       secondaryLagPortBitMapPtrIdx);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update LAG %u primary port bitMapPointer, error %u\n",
                       secondaryLag, saiRetVal);
        return saiRetVal;
    }
    /* Update primary LAG secondarylagPortBitMapPtrIdx */
    saiRetVal = xpSaiLagInfoLagSecondaryPortBitMapPointerUpdate(primaryLag,
                                                                secondaryLagPortBitMapPtrIdx, true);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update LAG %u secondary port bitMapPointer, error %u\n",
                       secondaryLag, saiRetVal);
        return saiRetVal;
    }

    return saiRetVal;
}
/**
 * \brief Restore traffic flow for LAG interfaces
 *
 * \param [in] xpsDevice_t xpsDevId
 * \param [in] xpsInterfaceId_t primaryLag - LAG interface to switch traffic from
 * \param [in] xpsInterfaceId_t secondaryLag - LAG interface to switch traffic to
 *
 * \return sai_status_t
 */
sai_status_t xpSaiLagRedirectDisable(xpsDevice_t devId,
                                     xpsInterfaceId_t primaryLag, xpsInterfaceId_t secondaryLag)
{
    sai_status_t  saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS     xpsRetVal = XP_NO_ERR;
    uint32_t lagPortBitmapPointerIndex=0;
    xpsPortList_t primaryLagPortList;

    memset(&primaryLagPortList, 0, sizeof(xpsPortList_t));

    /* Get the Primary Lag Port Bit Map Index Pointer*/
    saiRetVal = xpSaiLagInfoLagPortBitMapIndexGet(primaryLag,
                                                  &lagPortBitmapPointerIndex);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get primary lag's portbitmap pointer index for redirect LAG %u, error %u \n",
                       primaryLag, saiRetVal);
        return saiRetVal;
    }
    /* Add primary LAG ports to primary LAG */
    saiRetVal = xpSaiLagInfoPrimaryPortListGet(primaryLag, &primaryLagPortList);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get primary port list for redirect LAG %u, error %u \n",
                       primaryLag, saiRetVal);
        return saiRetVal;
    }

    /* Clear LAG redirection data. */
    saiRetVal = xpSaiLagInfoSet(devId, primaryLag, secondaryLag, false);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Clear LAG %u redirection data failed with error %d\n",
                       primaryLag, saiRetVal);
        return saiRetVal;
    }
    // Deploy primary LAG
    xpsRetVal = xpsLagDeploy(devId, primaryLag, AUTO_DIST_ENABLE);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not deploy LAG %d on device %d!\n", primaryLag, devId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return saiRetVal;
}

/**
    xpFree(portList);
 * \brief Deploy LAG on create LAG member. Helper API for xpSaiCreateLagMember()
 *
 * \param [in] xpsDevice_t xpsDevId
 * \param [in] xpsInterfaceId_t xpsLagId
 * \param [in] bool ingressDisable
 * \param [in] bool egressDisable
 *
 * \return sai_status_t
 */
//Func: xpSaiCreateLagMember

sai_status_t xpSaiCreateLagMember(sai_object_id_t* lag_member_id,
                                  sai_object_id_t switch_id,
                                  uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_STATUS                   retVal      = XP_NO_ERR;
    sai_status_t                saiRetVal   = SAI_STATUS_SUCCESS;
    sai_object_id_t             lag_id      = 0;
    sai_object_id_t             port_id     = 0;
    xpsInterfaceId_t            intfId      = 0;
    xpsInterfaceId_t            xpsLagId    = 0;
    xpsPort_t                   xpsPortId   = 0;
    xpsDevice_t                 xpsDevId    = 0;
    xpSaiPortLagInfo_t*         pSaiPortLagInfoEntry = NULL;
    xpSaiPortLagInfo_t          currentPortConfig;
    xpSaiLagInfo_t*             lagConfig = NULL;
    bool                        ingressDisable = false;
    bool                        egressDisable = false;
    xpSaiLagMemberAttributesT   attributes;
    xpsPortList_t primaryLagPortList;
    uint32_t portBitMapPtrIdx=0;
    xpsLagAutoDistributeControl_e autoDistribute = AUTO_DIST_ENABLE;

    XP_SAI_LOG_DBG("Calling \n");

    if (NULL == lag_member_id)
    {
        XP_SAI_LOG_ERR("Invalid parameter received!");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               LAG_MEMBER_VALIDATION_ARRAY_SIZE, lag_member_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    /*set default values to member attributes*/
    xpSaiSetDefaultLagMemberAttributeVals(&attributes);

    /*update member attributes*/
    xpSaiUpdateLagMemberAttributeVals(attr_count, attr_list, &attributes);

    /*Check for incoming attributes.*/
    for (sai_uint32_t indx = 0; indx < attr_count; indx++)
    {
        switch (attr_list[indx].id)
        {
            case SAI_LAG_MEMBER_ATTR_LAG_ID:
                lag_id = attributes.lagId.oid;
                break;
            case SAI_LAG_MEMBER_ATTR_PORT_ID:
                port_id = attributes.portId.oid;
                break;
            case SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE:
                egressDisable = attributes.egressDisable.booldata;
                break;
            case SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE:
                ingressDisable = attributes.ingressDisable.booldata;
                break;
            default:
                XP_SAI_LOG_ERR("Invalid attribute received(%u)!", attr_list[indx].id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
        }
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(lag_id, SAI_OBJECT_TYPE_LAG))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u), LAG object received!\n",
                       xpSaiObjIdTypeGet(lag_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(port_id, SAI_OBJECT_TYPE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u), port object expected!\n",
                       xpSaiObjIdTypeGet(port_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&currentPortConfig, 0, sizeof(currentPortConfig));

    saiRetVal = xpSaiGetPortLagConfig(port_id, &currentPortConfig);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get current port config. saiRetVal %d\n", saiRetVal);
        return saiRetVal;
    }

    xpsLagId  = (xpsInterfaceId_t)xpSaiObjIdValueGet(lag_id);
    intfId = (xpsInterfaceId_t)xpSaiObjIdValueGet(port_id);

    if (xpSaiPortBasedRifEntryExists(port_id))
    {
        XP_SAI_LOG_ERR("LAG OID %lu portid %lu is still is use. Please remove all references to this object.\n",
                       lag_id, port_id);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    /*get device and Port Id*/
    if ((retVal = xpsPortGetDevAndPortNumFromIntf(intfId, &xpsDevId,
                                                  &xpsPortId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not DeviceId and Port Number from InterfaceId: %d\n",
                       intfId);
        return xpsStatus2SaiStatus(retVal);
    }

    /* Validate the Port number. */
    if (xpsLinkManagerIsPortNumValid(xpsDevId, xpsPortId) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Port %d Not Found \n", xpsPortId);
        return SAI_STATUS_INVALID_PORT_NUMBER;
    }
    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %d\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }
    saiRetVal = xpSaiSetPortLagConfig(port_id, lagConfig->lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set LAG %u config on port %lu, error %d\n", xpsLagId,
                       port_id, saiRetVal);
        return saiRetVal;
    }


    // Deploy LAG if it is not redirected
    if (!xpSaiLagRedirectStatusCheck(xpsLagId))
    {
        // Add it to the Lag
        if ((retVal = (XP_STATUS)xpsLagAddPort(xpsLagId, intfId)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not add portIntf %d to lagId %d!\n", intfId, xpsLagId);
            return xpsStatus2SaiStatus(retVal);
        }

        autoDistribute = (lagConfig->lagConfig.autoDistribute) ? AUTO_DIST_ENABLE :
                         AUTO_DIST_DISABLE;
        if ((retVal = (XP_STATUS)xpsLagDeploy(xpsDevId, xpsLagId,
                                              autoDistribute)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not deploy lag after adding portIntf %d to lagId %d\n",
                           intfId, xpsLagId);
            return xpsStatus2SaiStatus(retVal);
        }

        if (ingressDisable)
        {
            if ((retVal = xpsLagMemberIngressEnablePort(xpsDevId, xpsLagId,
                                                        intfId, false)) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("xpsLagMemberIngressEnablePort failed:%d", retVal);
                return xpsStatus2SaiStatus(retVal);
            }
        }
        if (egressDisable)
        {
            if ((retVal = xpsLagMemberEgressEnablePort(xpsDevId, xpsLagId,
                                                       intfId, false)) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("xpsLagMemberEgressEnablePort failed:%d", retVal);
                return xpsStatus2SaiStatus(retVal);
            }
        }
    }
    else /* This is redirect LAG */
    {
        //xpsInterfaceId_t primaryLag = XPS_INTF_INVALID_ID;
        /* Handling intermediate updates of secondary and primary LAGS:
         *  Steps to be taken care for intermediate updates.
         *  1. If new port is added / deleted in secondary lag, deploy the secondary LAG accordingly and also add/delete this port from Primary LAG “secondary_egress_port_list” and call egress deploy for secondary port list of primary LAG.
         *  2. If new port is added / deleted in primary LAG, just keep updating the original port list info for primary LAG but skip deploying for the LAG. -- No action is required in here
         */
        if (!xpSaiLagRedirectIsPrimaryCheck(
                xpsLagId)) /* Process secondary LAG update */
        {
            // Add it to the Lag
            if ((retVal = (XP_STATUS)xpsLagAddPort(xpsLagId, intfId)) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not add portIntf %d to lagId %d!\n", intfId, xpsLagId);
                return xpsStatus2SaiStatus(retVal);
            }

            // Deploy Secondary Lag
            autoDistribute = (lagConfig->lagConfig.autoDistribute) ? AUTO_DIST_ENABLE :
                             AUTO_DIST_DISABLE;
            if ((retVal = (XP_STATUS)xpsLagDeploy(xpsDevId, xpsLagId,
                                                  autoDistribute)) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not deploy lag after adding portIntf %d to lagId %d\n",
                               intfId, xpsLagId);
                return xpsStatus2SaiStatus(retVal);
            }

            // Set Ingress member disable
            if (ingressDisable)
            {
                if ((retVal = (XP_STATUS)xpsLagIngressDisablePort(xpsDevId, xpsLagId,
                                                                  intfId)) != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Could not disable port ingress");
                    return xpsStatus2SaiStatus(retVal);
                }
            }
            else
            {
                if ((retVal = (XP_STATUS)xpsLagIngressEnablePort(xpsDevId, xpsLagId,
                                                                 intfId)) != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Could not enable port ingress");
                    return xpsStatus2SaiStatus(retVal);
                }
            }

            if (egressDisable)
            {
                if ((retVal = (XP_STATUS)xpsLagEgressDisablePort(xpsDevId, xpsLagId, intfId,
                                                                 AUTO_DIST_ENABLE)) != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Could not disable port egress");
                    return xpsStatus2SaiStatus(retVal);
                }
            }
            else
            {
                if ((retVal = (XP_STATUS)xpsLagEgressEnablePort(xpsDevId, xpsLagId, intfId,
                                                                AUTO_DIST_ENABLE)) != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Could not disable port egress");
                    return xpsStatus2SaiStatus(retVal);
                }
            }
        }
        else // Primary LAG
        {
            // Add it to the Lag
            if ((retVal = (XP_STATUS)xpsLagAddPort(xpsLagId, intfId)) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not add portIntf %d to lagId %d!\n", intfId, xpsLagId);
                return xpsStatus2SaiStatus(retVal);
            }

            if ((retVal = xpsLagDeployIngress(xpsDevId, xpsLagId)) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not ingress deploy LAG on device: %d, lagId: %d\n",
                               xpsDevId, xpsLagId);
                return xpsStatus2SaiStatus(retVal);
            }
            autoDistribute = (lagConfig->lagConfig.autoDistribute) ? AUTO_DIST_ENABLE :
                             AUTO_DIST_DISABLE;
            if ((retVal = xpsLagDeployEgress(xpsDevId, xpsLagId, autoDistribute,
                                             true)) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not egress deploy LAG on device: %d, lagId: %d\n",
                               xpsDevId, xpsLagId);
                return xpsStatus2SaiStatus(retVal);
            }
            /* Update primary LAG primary port list */
            saiRetVal = xpSaiLagInfoPrimaryPortListUpdate(xpsLagId, intfId, true /*add*/);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to update LAG %u primary port list, error %u\n",
                               xpsLagId, saiRetVal);
                return saiRetVal;
            }
            saiRetVal = xpSaiLagInfoPrimaryPortListGet(xpsLagId,
                                                       (xpsPortList_t *)&primaryLagPortList);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to get  LAG %u primary port list, error %u\n", xpsLagId,
                               saiRetVal);
                return saiRetVal;
            }
            saiRetVal = xpSaiLagInfoLagPortBitMapIndexGet(xpsLagId, &portBitMapPtrIdx);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to get LAG %u primary port bitmap Index, error %u\n",
                               xpsLagId, saiRetVal);
                return saiRetVal;
            }
        }
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiPortLagInfo_t),
                                (void**)&pSaiPortLagInfoEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate channel structure\n");
        return xpsStatus2SaiStatus(retVal);
    }

    memset(pSaiPortLagInfoEntry, 0, sizeof(xpSaiPortLagInfo_t));

    pSaiPortLagInfoEntry->intfId = intfId;
    pSaiPortLagInfoEntry->lagId = xpsLagId;
    pSaiPortLagInfoEntry->ingressDisable = ingressDisable;
    pSaiPortLagInfoEntry->egressDisable = egressDisable;

    retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, portLagDbHandle,
                                (void*)pSaiPortLagInfoEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not insert a channel structure into DB, error %d\n",
                       retVal);
        xpsStateHeapFree((void*)pSaiPortLagInfoEntry);
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = xpSaiStoreCurrentPortConfigDB(port_id, currentPortConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set LAG %u config on port %lu, error %d\n", xpsLagId,
                       port_id, saiRetVal);
        return saiRetVal;
    }

    if (xpSaiLagMemberObjIdCreate(SAI_OBJECT_TYPE_LAG_MEMBER, xpsLagId, intfId,
                                  lag_member_id) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI LAG member object could not be created.\n");
        return SAI_STATUS_FAILURE;
    }

    // Update the ACL config on each Lag member
    saiRetVal = xpSaiAclUpdateMemberInfo(lagConfig->ingressAclId, xpsPortId, true);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set ACL %u config on port %lu, error %d\n", xpsLagId,
                       port_id, saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiAclUpdateMemberInfo(lagConfig->egressAclId, xpsPortId, true);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set ACL %u config on port %lu, error %d\n", xpsLagId,
                       port_id, saiRetVal);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

#define XP_TRUNK_RESOLUTION_MAX_SIZE 256
//Func: xpSaiRemoveLagMember

sai_status_t xpSaiRemoveLagMember(sai_object_id_t lag_member_id)
{
    XP_STATUS           retVal = XP_NO_ERR;
    xpsInterfaceId_t    intfId = 0;
    xpsInterfaceId_t    xpsLagId = 0;
    xpsDevice_t         xpsDevId = 0;
    xpSaiPortLagInfo_t* pSaiPortLagInfoEntry = NULL;
    xpSaiPortLagInfo_t  saiPortLagInfoKey;
    xpsPort_t           xpsPortId = 0;
    sai_status_t        saiRetVal = SAI_STATUS_SUCCESS;
    xpsPortList_t primaryLagPortList;
    uint32_t portBitMapPtrIdx=0;
    sai_object_id_t portId = SAI_NULL_OBJECT_ID;
    xpSaiLagInfo_t*             lagConfig;
    xpsLagAutoDistributeControl_e autoDistribute = AUTO_DIST_ENABLE;

    memset(&saiPortLagInfoKey, 0, sizeof(saiPortLagInfoKey));

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(lag_member_id, SAI_OBJECT_TYPE_LAG_MEMBER))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(lag_member_id));
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    xpsLagId  = (xpsInterfaceId_t)xpSaiLagMemberLagIdValueGet(lag_member_id);
    intfId = (xpsInterfaceId_t)xpSaiLagMemberPortIdValueGet(lag_member_id);

    // Get device and Port Id
    if ((retVal = xpsPortGetDevAndPortNumFromIntf(intfId, &xpsDevId,
                                                  &xpsPortId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not DeviceId and Port Number from InterfaceId: %d\n",
                       intfId);
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = xpSaiLagConfigInfoGet(xpsLagId, &lagConfig);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %d\n", xpsLagId,
                       saiRetVal);
        return saiRetVal;
    }

    // Deploy the LAG if it is not redirected
    if (!xpSaiLagRedirectStatusCheck(xpsLagId))
    {
        // Remove it from the Lag
        if ((retVal = (XP_STATUS)xpsLagRemovePort(xpsLagId, intfId)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not remove portIntf %d from lagId %d!\n", intfId,
                           xpsLagId);
            return xpsStatus2SaiStatus(retVal);
        }

        autoDistribute = (lagConfig->lagConfig.autoDistribute) ? AUTO_DIST_ENABLE :
                         AUTO_DIST_DISABLE;
        if ((retVal = xpsLagDeploy(xpsDevId, xpsLagId, autoDistribute)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not deploy LAG %d on device %d!\n", xpsLagId, xpsDevId);
            return xpsStatus2SaiStatus(retVal);
        }

        // If LAG is ingress blocked, then overwrite it with saved blocked port list
        saiRetVal = xpSaiLagBlockedCheckAndUpdate(xpsDevId, xpsLagId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to verify and update blocked port list with error %d\n",
                           saiRetVal);
            return saiRetVal;
        }
    }
    else /* This is redirect LAG */
    {
        //xpsInterfaceId_t primaryLag = XPS_INTF_INVALID_ID;
        sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
        /* Handling intermediate updates of secondary and primary LAGS:
         *  Steps to be taken care for intermediate updates.
         *  1. If new port is added / deleted in secondary lag, deploy the secondary LAG accordingly and also add/delete this port from Primary LAG “secondary_egress_port_list” and call egress deploy for secondary port list of primary LAG.
         *  2. If new port is added / deleted in primary LAG, just keep updating the original port list info for primary LAG but skip deploying for the LAG. -- No action is required in here
         */
        if (!xpSaiLagRedirectIsPrimaryCheck(
                xpsLagId)) /* Process secondary LAG update */
        {
            // Remove it from the Lag
            if ((retVal = (XP_STATUS)xpsLagRemovePort(xpsLagId, intfId)) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not remove portIntf %d from lagId %d!\n", intfId,
                               xpsLagId);
                return xpsStatus2SaiStatus(retVal);
            }

            /* Deploy secondary LAG */
            autoDistribute = (lagConfig->lagConfig.autoDistribute) ? AUTO_DIST_ENABLE :
                             AUTO_DIST_DISABLE;
            retVal = xpsLagDeploy(xpsDevId, xpsLagId, autoDistribute);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not deploy LAG %d on device %d!\n", xpsLagId, xpsDevId);
                return xpsStatus2SaiStatus(retVal);
            }
        }
        else // Primary LAG
        {
            // Remove it from the Lag
            if ((retVal = (XP_STATUS)xpsLagRemovePort(xpsLagId, intfId)) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not remove portIntf %d from lagId %d!\n", intfId,
                               xpsLagId);
                return xpsStatus2SaiStatus(retVal);
            }

            if ((retVal = xpsLagDeployIngress(xpsDevId, xpsLagId)) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not deploy LAG on device: %d, lagId: %d\n", xpsDevId,
                               xpsLagId);
                return xpsStatus2SaiStatus(retVal);
            }

            if ((retVal = xpsLagDeployEgress(xpsDevId, xpsLagId, AUTO_DIST_ENABLE,
                                             true)) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not egress deploy LAG on device: %d, lagId: %d\n",
                               xpsDevId, xpsLagId);
                return xpsStatus2SaiStatus(retVal);
            }
            /* Update primary LAG primary port list */
            saiRetVal = xpSaiLagInfoPrimaryPortListUpdate(xpsLagId, intfId,
                                                          false /*remove*/);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to update LAG %u primary port list, error %u\n",
                               xpsLagId, saiRetVal);
                return saiRetVal;
            }
            saiRetVal = xpSaiLagInfoPrimaryPortListGet(xpsLagId,
                                                       (xpsPortList_t *)&primaryLagPortList);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to get LAG %u primary port list, error %u\n", xpsLagId,
                               saiRetVal);
                return saiRetVal;
            }
            saiRetVal = xpSaiLagInfoLagPortBitMapIndexGet(xpsLagId, &portBitMapPtrIdx);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to get LAG %u primary port bitmap Index, error %u\n",
                               xpsLagId, saiRetVal);
                return saiRetVal;
            }
        }
    }

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, xpsDevId, intfId, &portId);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Error: SAI port object could not be created.\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiRestorePortConfigDB(portId);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to restore port %lu config, error %u\n", portId,
                       saiRetVal);
        return saiRetVal;
    }

    saiPortLagInfoKey.intfId = intfId;

    retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, portLagDbHandle,
                                &saiPortLagInfoKey, (void**)&pSaiPortLagInfoEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not delete channel structure from DB, saiPortLagInfoKey.intfId %d\n",
                       saiPortLagInfoKey.intfId);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateHeapFree(pSaiPortLagInfoEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not deallocate channel structure, saiPortLagInfoKey.intfId %d\n",
                       saiPortLagInfoKey.intfId);
        return xpsStatus2SaiStatus(retVal);
    }

    xpsPortList_t portList;
    memset(&portList, 0, sizeof(portList));

    for (int i=0; i < XP_TRUNK_RESOLUTION_MAX_SIZE; i++)
    {
        portList.size = 1;
        portList.portList[0] = intfId;
        retVal = xpsLagAddPortToDistributionEntry(xpsDevId, i, &portList);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Unable to set Trunk Resolution for index %d \n", i);
        }
    }

    // Update the ACL config on each Lag member
    saiRetVal = xpSaiAclUpdateMemberInfo(lagConfig->ingressAclId, xpsPortId, false);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set ACL %u config on port %u, error %d\n", xpsLagId,
                       xpsPortId, saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiAclUpdateMemberInfo(lagConfig->egressAclId, xpsPortId, false);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set ACL %u config on port %u, error %d\n", xpsLagId,
                       xpsPortId, saiRetVal);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiSetLagMemberAttributeIngressDeploy(sai_object_id_t
                                                     lag_member_id, sai_attribute_value_t disableIngress)
{
    XP_STATUS           retVal = XP_NO_ERR;
    xpsInterfaceId_t    xpsLagId = 0;
    xpsInterfaceId_t    intfId = 0;
    xpsDevice_t         xpsDevId = 0;
    xpsPort_t           xpsPortId = 0;

    intfId = (xpsInterfaceId_t)xpSaiLagMemberPortIdValueGet(lag_member_id);
    xpsLagId  = (xpsInterfaceId_t)xpSaiLagMemberLagIdValueGet(lag_member_id);

    /*get device and Port Id*/
    if ((retVal = xpsPortGetDevAndPortNumFromIntf(intfId, &xpsDevId,
                                                  &xpsPortId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not DeviceId and Port Number from InterfaceId: %d\n",
                       intfId);
        return xpsStatus2SaiStatus(retVal);
    }

    if (!disableIngress.booldata)
    {
        if ((retVal = xpsLagDeployIngress(xpsDevId, xpsLagId)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not deploy LAG on device: %d, lagId: %d\n", xpsDevId,
                           xpsLagId);
            return xpsStatus2SaiStatus(retVal);
        }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetLagMemberAttributeIngressDisable
sai_status_t xpSaiSetLagMemberAttributeIngressDisable(sai_object_id_t
                                                      lag_member_id, sai_attribute_value_t disableIngress)
{
    XP_STATUS           retVal = XP_NO_ERR;
    xpsInterfaceId_t    xpsLagId = 0;
    xpsInterfaceId_t    intfId = 0;
    xpsDevice_t         xpsDevId = 0;
    xpsPort_t           xpsPortId = 0;
    uint32_t            isMember = 0;
    xpSaiPortLagInfo_t  pSaiPortLagInfoKey;
    xpSaiPortLagInfo_t *pSaiPortLagInfo = NULL;

    memset(&pSaiPortLagInfoKey, 0, sizeof(xpSaiPortLagInfo_t));

    intfId = (xpsInterfaceId_t)xpSaiLagMemberPortIdValueGet(lag_member_id);
    xpsLagId  = (xpsInterfaceId_t)xpSaiLagMemberLagIdValueGet(lag_member_id);

    /*get device and Port Id*/
    if ((retVal = xpsPortGetDevAndPortNumFromIntf(intfId, &xpsDevId,
                                                  &xpsPortId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not DeviceId and Port Number from InterfaceId: %d\n",
                       intfId);
        return xpsStatus2SaiStatus(retVal);
    }

    // Verify port is LAG member
    if ((retVal = xpsLagIsPortMember(xpsDevId, xpsPortId, xpsLagId,
                                     &isMember)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not verify if portIntf %d is lagId %d member.\n", intfId,
                       xpsLagId);
        return xpsStatus2SaiStatus(retVal);
    }

    if (!isMember)
    {
        XP_SAI_LOG_ERR("Error: Port: %d isn't a configured member of lag: %d",
                       xpsPortId, xpsLagId);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    pSaiPortLagInfoKey.intfId = intfId;

    if ((retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, portLagDbHandle,
                                     (xpsDbKey_t)&pSaiPortLagInfoKey,
                                     (void**)&pSaiPortLagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search for Lag interface failed, lag interface(%d)", retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    if (!pSaiPortLagInfo)
    {
        return xpsStatus2SaiStatus(XP_ERR_NOT_FOUND);
    }

    if (pSaiPortLagInfo->ingressDisable == disableIngress.booldata)
    {
        return SAI_STATUS_SUCCESS;
    }

    if ((retVal = xpsLagMemberIngressEnablePort(xpsDevId, xpsLagId, intfId,
                                                !(disableIngress.booldata))) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsLagMemberIngressEnablePort failed:%d", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    pSaiPortLagInfo->ingressDisable = disableIngress.booldata;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetLagMemberAttributeEgressDisable
sai_status_t xpSaiSetLagMemberAttributeEgressDisable(sai_object_id_t
                                                     lag_member_id, sai_attribute_value_t disableEgress)
{
    XP_STATUS           retVal = XP_NO_ERR;
    xpsInterfaceId_t    xpsLagId = 0;
    xpsInterfaceId_t    intfId = 0;
    xpsDevice_t         xpsDevId = 0;
    xpsPort_t           xpsPortId = 0;
    uint32_t            isMember = 0;
    xpSaiPortLagInfo_t  pSaiPortLagInfoKey;
    xpSaiPortLagInfo_t *pSaiPortLagInfo = NULL;

    memset(&pSaiPortLagInfoKey, 0, sizeof(xpSaiPortLagInfo_t));

    intfId = (xpsInterfaceId_t)xpSaiLagMemberPortIdValueGet(lag_member_id);
    xpsLagId  = (xpsInterfaceId_t)xpSaiLagMemberLagIdValueGet(lag_member_id);

    /*get device and Port Id*/
    if ((retVal = xpsPortGetDevAndPortNumFromIntf(intfId, &xpsDevId,
                                                  &xpsPortId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not DeviceId and Port Number from InterfaceId: %d\n",
                       intfId);
        return xpsStatus2SaiStatus(retVal);
    }

    // Verify port is LAG member
    if ((retVal = xpsLagIsPortMember(xpsDevId, xpsPortId, xpsLagId,
                                     &isMember)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not verify if portIntf %d is lagId %d member.\n", intfId,
                       xpsLagId);
        return xpsStatus2SaiStatus(retVal);
    }

    if (!isMember)
    {
        XP_SAI_LOG_ERR("Error: Port: %d isn't a configured member of lag: %d",
                       xpsPortId, xpsLagId);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    pSaiPortLagInfoKey.intfId = intfId;

    if ((retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, portLagDbHandle,
                                     (xpsDbKey_t)&pSaiPortLagInfoKey,
                                     (void**)&pSaiPortLagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search for Lag interface failed, lag interface(%d)", retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    if (!pSaiPortLagInfo)
    {
        return xpsStatus2SaiStatus(XP_ERR_NOT_FOUND);
    }

    if (pSaiPortLagInfo->egressDisable == disableEgress.booldata)
    {
        return SAI_STATUS_SUCCESS;
    }

    if ((retVal = xpsLagMemberEgressEnablePort(xpsDevId, xpsLagId, intfId,
                                               !(disableEgress.booldata))) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsLagMemberEgressEnablePort failed:%d", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    pSaiPortLagInfo->egressDisable = disableEgress.booldata;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetLagMemberAttribute

sai_status_t xpSaiSetLagMemberAttribute(sai_object_id_t  lag_member_id,
                                        const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiSetLagMemberAttribute\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(lag_member_id, SAI_OBJECT_TYPE_LAG_MEMBER))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(lag_member_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(1, attr,
                            LAG_MEMBER_VALIDATION_ARRAY_SIZE, lag_member_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    switch (attr->id)
    {
        case SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE:
            {
                retVal = xpSaiSetLagMemberAttributeEgressDisable(lag_member_id, attr->value);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE is unsupported\n");
                    return retVal;
                }
                break;
            }
        case SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE:
            {
                retVal = xpSaiSetLagMemberAttributeIngressDisable(lag_member_id, attr->value);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE \n");
                    return retVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return retVal;
}

//Func: xpSaiGetLagMemberAttributeIngressDisable

sai_status_t xpSaiGetLagMemberAttributeIngressDisable(sai_object_id_t
                                                      lag_member_id, bool *disableIngress)
{
    XP_STATUS           retVal = XP_NO_ERR;
    xpSaiPortLagInfo_t  pSaiPortLagInfoKey;
    xpSaiPortLagInfo_t *pSaiPortLagInfo = NULL;

    memset(&pSaiPortLagInfoKey, 0, sizeof(xpSaiPortLagInfo_t));

    pSaiPortLagInfoKey.intfId = (xpsInterfaceId_t)xpSaiLagMemberPortIdValueGet(
                                    lag_member_id);

    if ((retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, portLagDbHandle,
                                     (xpsDbKey_t)&pSaiPortLagInfoKey,
                                     (void**)&pSaiPortLagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search for Lag interface failed, lag interface(%d)", retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    if (!pSaiPortLagInfo)
    {
        return xpsStatus2SaiStatus(XP_ERR_NOT_FOUND);
    }

    *disableIngress = pSaiPortLagInfo->ingressDisable;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetLagMemberAttributeEgressDisable

sai_status_t xpSaiGetLagMemberAttributeEgressDisable(sai_object_id_t
                                                     lag_member_id, bool *disableEgress)
{
    XP_STATUS           retVal = XP_NO_ERR;
    xpSaiPortLagInfo_t  pSaiPortLagInfoKey;
    xpSaiPortLagInfo_t *pSaiPortLagInfo = NULL;

    memset(&pSaiPortLagInfoKey, 0, sizeof(xpSaiPortLagInfo_t));

    pSaiPortLagInfoKey.intfId = (xpsInterfaceId_t)xpSaiLagMemberPortIdValueGet(
                                    lag_member_id);

    if ((retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, portLagDbHandle,
                                     (xpsDbKey_t)&pSaiPortLagInfoKey,
                                     (void**)&pSaiPortLagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search for Lag interface failed, lag interface(%d)", retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    if (!pSaiPortLagInfo)
    {
        return xpsStatus2SaiStatus(XP_ERR_NOT_FOUND);
    }

    *disableEgress = pSaiPortLagInfo->egressDisable;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetLagMemberAttribute

static sai_status_t xpSaiGetLagMemberAttribute(sai_object_id_t lag_member_id,
                                               sai_attribute_t *attr, uint32_t attr_index)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t devId = xpSaiGetDevId();

    switch (attr->id)
    {
        case SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE:
            {
                saiRetVal = xpSaiGetLagMemberAttributeEgressDisable(lag_member_id,
                                                                    &attr->value.booldata);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to get SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE attribute\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE:
            {
                saiRetVal = xpSaiGetLagMemberAttributeIngressDisable(lag_member_id,
                                                                     &attr->value.booldata);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to get SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE attribute\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_LAG_MEMBER_ATTR_LAG_ID:
            {
                xpsInterfaceId_t xpsLagId = 0;

                xpsLagId  = (xpsInterfaceId_t)xpSaiLagMemberLagIdValueGet(lag_member_id);
                saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_LAG, devId,
                                             (sai_uint64_t) xpsLagId, &attr->value.oid);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Couldn't get SAI LAG OBJID for Member.\n");
                    return SAI_STATUS_FAILURE;
                }
                break;
            }
        case SAI_LAG_MEMBER_ATTR_PORT_ID:
            {
                xpsInterfaceId_t xpsPortIfId = 0;

                xpsPortIfId = (xpsInterfaceId_t)xpSaiLagMemberPortIdValueGet(lag_member_id);
                saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                             (sai_uint64_t) xpsPortIfId, &attr->value.oid);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Couldn't get SAI PORT  OBJID for Member.\n");
                    return SAI_STATUS_FAILURE;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetLagMemberAttributes

static sai_status_t xpSaiGetLagMemberAttributes(sai_object_id_t lag_member_id,
                                                uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t        saiRetVal   = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (attr_list == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetLagMemberAttribute(lag_member_id, &attr_list[count], count);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiGetLagMemberAttribute failed with saiRetVal: %d!\n",
                           saiRetVal);
            return saiRetVal;
        }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBulkGetLagMemberAttributes

sai_status_t xpSaiBulkGetLagMemberAttributes(sai_object_id_t id,
                                             uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_LAG_MEMBER))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountLagMemberAttribute(&maxcount);
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
        attr_list[idx].id = SAI_LAG_MEMBER_ATTR_START + count;
        saiRetVal = xpSaiGetLagMemberAttribute(id, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveAllLags

sai_status_t xpSaiRemoveAllLags(void)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_SAI_LOG_DBG("Calling %s\n", __FUNCNAME__);
    return retVal;
}

//Func: xpSaiLagApiInit

XP_STATUS xpSaiLagApiInit(uint64_t flag,
                          const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiLagApiInit\n");

    _xpSaiLagApi = (sai_lag_api_t *) xpMalloc(sizeof(sai_lag_api_t));
    if (NULL == _xpSaiLagApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiLagApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiLagApi->create_lag = xpSaiCreateLag;
    _xpSaiLagApi->remove_lag = xpSaiRemoveLag;
    _xpSaiLagApi->set_lag_attribute = xpSaiSetLagAttribute;
    _xpSaiLagApi->get_lag_attribute = xpSaiGetLagAttribute;
    _xpSaiLagApi->create_lag_member = xpSaiCreateLagMember;
    _xpSaiLagApi->remove_lag_member = xpSaiRemoveLagMember;
    _xpSaiLagApi->set_lag_member_attribute = xpSaiSetLagMemberAttribute;
    _xpSaiLagApi->get_lag_member_attribute = xpSaiGetLagMemberAttributes;

    _xpSaiLagApi->create_lag_members = (sai_bulk_object_create_fn)
                                       xpSaiStubGenericApi;
    _xpSaiLagApi->remove_lag_members = (sai_bulk_object_remove_fn)
                                       xpSaiStubGenericApi;

    saiRetVal = xpSaiApiRegister(SAI_API_LAG, (void*)_xpSaiLagApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register LAG API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}

//Func: xpSaiLagApiDeinit

XP_STATUS xpSaiLagApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiLagApiDeinit\n");

    xpFree(_xpSaiLagApi);
    _xpSaiLagApi = NULL;

    return  retVal;
}

//Func: xpSaiLagInit

XP_STATUS xpSaiLagInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling %s\n", __FUNCNAME__);

    // Create global SAI LAG redirect DB
    if ((xpsRetVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "SAI LAG info DB",
                                        XPS_GLOBAL, &xpSaiLagInfoKeyComp, xpSaiLagInfoDbHandle)) != XP_NO_ERR)
    {
        xpSaiLagInfoDbHandle = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create SAI LAG info DB\n");
        return xpsRetVal;
    }

    return xpsRetVal;
}

//Func: xpSaiLagDeInit

XP_STATUS xpSaiLagDeInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling %s\n", __FUNCNAME__);

    // Purge global SAI LAG redirect DB
    if ((xpsRetVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                          &xpSaiLagInfoDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge SAI LAG info DB\n");
        return xpsRetVal;
    }

    return xpsRetVal;
}

sai_status_t xpSaiMaxCountLagAttribute(uint32_t *count)
{
    *count = SAI_LAG_ATTR_END;
    return  SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountLagObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    retVal = xpsLagGetLagIdCount(count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetLagObjectList(uint32_t *object_count,
                                   sai_object_key_t *object_list)
{
    XP_STATUS         retVal  = XP_NO_ERR;
    xpsInterfaceId_t *lagId;
    xpsScope_t        scopeId = XP_SCOPE_DEFAULT;
    xpsDevice_t       devId   = xpSaiGetDevId();

    /*TODO  Handling of object_count by getting objCount and also buffer overflow condition*/

    lagId = (xpsInterfaceId_t *)xpMalloc(sizeof(xpsInterfaceId_t)*(*object_count));
    if (!lagId)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for lagId\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    retVal = xpsLagGetLagIdList(scopeId, lagId);
    if (retVal != XP_NO_ERR)
    {
        xpFree(lagId);
        return xpsStatus2SaiStatus(retVal);
    }

    for (uint32_t i = 0; i < *object_count; i++)
    {
        if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_LAG, devId, (sai_uint64_t)lagId[i],
                             &object_list[i].key.object_id) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created.\n");
            xpFree(lagId);
            return SAI_STATUS_FAILURE;
        }
    }

    xpFree(lagId);
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiMaxCountLagMemberAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_LAG_MEMBER_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountLagMemberObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, portLagDbHandle, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetLagMemberObjectList(uint32_t *object_count,
                                         sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpsDevice_t     devId       = xpSaiGetDevId();

    xpSaiPortLagInfo_t *pLagMemberNext = NULL;

    saiRetVal = xpSaiCountLagMemberObjects(&objCount);
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
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, portLagDbHandle, pLagMemberNext,
                                     (void **)&pLagMemberNext);
        if (retVal != XP_NO_ERR || pLagMemberNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve LAG member object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_LAG_MEMBER, devId,
                                     (sai_uint64_t)pLagMemberNext->intfId, &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

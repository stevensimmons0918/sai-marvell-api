// xpSaiMirror.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiMirror.h"
#include "xpTypes.h"
#include "xpSaiValidationArrays.h"
#include "xpsErspanGre.h"
#include "xpsPolicer.h"
#include "xpsGlobalSwitchControl.h"

XP_SAI_LOG_REGISTER_API(SAI_API_MIRROR);

static sai_mirror_api_t* _xpSaiMirrorApi;
xpsDbHandle_t gsaiMirrorDbHdle = XPS_STATE_INVALID_DB_HANDLE;

static sai_status_t xpSaiMirrorSessionSetDefaults(xpSaiMirrorSession_t
                                                  *session);

static sai_status_t xpSaiMirrorEnhancedRemoteTunCreate(xpsDevice_t xpsDevId,
                                                       xpSaiMirrorSession_t *session);
static sai_status_t xpSaiMirrorEnhancedRemoteTunRemove(xpsDevice_t xpsDevId,
                                                       xpSaiMirrorSession_t *session);
static sai_status_t xpSaiMirrorSessionAnalyzerCreateOrRef(xpsDevice_t xpsDevId,
                                                          xpSaiMirrorSession_t *session, bool ingress);
static sai_status_t xpSaiMirrorSessionAnalyzerRemoveOrUnref(
    xpsDevice_t xpsDevId, xpSaiMirrorSession_t *session, bool ingress,
    bool forceRemove);
static sai_status_t xpSaiMirrorAttributeSetInternal(sai_object_id_t session_id,
                                                    const sai_attribute_t *attr, bool onCreate);
static sai_status_t xpSaiMirrorSessionAnalyzerUpdate(xpsDevice_t xpsDevId,
                                                     xpSaiMirrorSession_t *session);

static sai_status_t xpSaiMirrorSessionPolicerAdd(uint32_t analyzerId,
                                                 sai_object_id_t policerId);
static sai_status_t xpSaiMirrorSessionPolicerRemove(uint32_t analyzerId,
                                                    sai_object_id_t policerId);

// Func: xpSaiMirrorSessionEntryKeyComp

static int32_t xpSaiMirrorSessionEntryKeyComp(void* key1, void* key2)
{
    return ((((xpSaiMirrorSession_t*)key1)->keySessionId) - (((
                                                                  xpSaiMirrorSession_t*)key2)->keySessionId));
}

//Func: xpSaiMirrorDbInfoGet

sai_status_t xpSaiMirrorDbInfoGet(sai_object_id_t session_id,
                                  xpSaiMirrorSession_t **xpSaiMirrorSessionEntry)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpSaiMirrorSession_t xpSaiMirrorSessionKey;

    memset(&xpSaiMirrorSessionKey, 0, sizeof(xpSaiMirrorSessionKey));

    // Form a key to search database
    xpSaiMirrorSessionKey.keySessionId = (uint32_t)xpSaiObjIdValueGet(session_id);

    xpsRetVal = xpsStateSearchData(xpSaiObjIdScopeGet(session_id), gsaiMirrorDbHdle,
                                   &xpSaiMirrorSessionKey, (void**)xpSaiMirrorSessionEntry);
    if (*xpSaiMirrorSessionEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find Mirror entry in DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiMirrorDbInfoGetNext

static sai_status_t xpSaiMirrorDbInfoGetNext(int sessionKey,
                                             xpSaiMirrorSession_t **xpSaiMirrorSessionEntry)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpSaiMirrorSession_t xpSaiMirrorSessionKey;

    memset(&xpSaiMirrorSessionKey, 0, sizeof(xpSaiMirrorSessionKey));

    if (sessionKey == -1) // Find first entry
    {
        xpsRetVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiMirrorDbHdle,
                                        NULL, (void**)xpSaiMirrorSessionEntry);
    }
    else   // Find next entry
    {
        // Form a key to search database
        xpSaiMirrorSessionKey.keySessionId = sessionKey;

        xpsRetVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiMirrorDbHdle,
                                        &xpSaiMirrorSessionKey, (void**)xpSaiMirrorSessionEntry);
    }
    if (*xpSaiMirrorSessionEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find Mirror entry in DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiMirrorDbInfoCreate

sai_status_t xpSaiMirrorDbInfoCreate(xpsDevice_t xpsDevId,
                                     uint32_t internalSessionId, xpSaiMirrorSession_t **xpSaiMirrorSession)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    // Add the allocated mirror session Id into the tree along with its properties
    // Allocate space for the xpSaiMirrorSession data
    if ((xpsRetVal = xpsStateHeapMalloc(sizeof(xpSaiMirrorSession_t),
                                        (void**)xpSaiMirrorSession)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapMalloc failed: xpsRetVal %d \n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (*xpSaiMirrorSession == NULL)
    {
        XP_SAI_LOG_ERR("No memory allocated by xpsStateHeapMalloc\n");
        return SAI_STATUS_NO_MEMORY;
    }

    saiRetVal = xpSaiMirrorSessionSetDefaults(*xpSaiMirrorSession);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        xpsStateHeapFree((void*)*xpSaiMirrorSession);
        *xpSaiMirrorSession = NULL;
        return saiRetVal;
    }
    (*xpSaiMirrorSession)->keySessionId = internalSessionId;
    (*xpSaiMirrorSession)->sampleRate = 1;


    // Insert the entry structure into the databse, using the keyMirrorSessionId
    if ((xpsRetVal = xpsStateInsertData(xpSaiScopeFromDevGet(xpsDevId),
                                        gsaiMirrorDbHdle, (void*)*xpSaiMirrorSession)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateInsertData failed: xpsRetVal %d \n", xpsRetVal);
        // Free Allocated Memory
        xpsStateHeapFree((void*)*xpSaiMirrorSession);
        *xpSaiMirrorSession = NULL;
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return xpsStatus2SaiStatus(xpsRetVal);
}

/**
 * \brief Creates some features used by enhanced remote session
 *        without programming analyzer session on HW
 *
 *  Creates ERSPAN tunnel and all its related data(L3 VLAN, NH)
 *  and programs it to HW. The analyzer session creation is postponed
 *  till at least one mirrored port is added to the session.
 *
 * \param xpsDevId - device ID
 * \param session - pointer to xpSaiMirrorSession_t instance
 *
 * \return void
 */
sai_status_t xpSaiMirrorSessionAdd(xpsDevice_t xpsDevId,
                                   xpSaiMirrorSession_t *session)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    // Create Mirror session
    saiRetVal = xpSaiMirrorSessionAnalyzerCreateOrRef(xpsDevId, session, true);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create Mirror analyzer session with error %d \n",
                       saiRetVal);
        return saiRetVal;
    }

    return saiRetVal;
}

/**
 * \brief Updates mirror session
 *
 * \param xpsDevId - device ID
 * \param session - pointer to xpSaiMirrorSession_t instance
 *
 * \return void
 */
sai_status_t xpSaiMirrorSessionUpdate(xpsDevice_t xpsDevId,
                                      xpSaiMirrorSession_t *session, const sai_attribute_t *attr)
{
    XP_STATUS             xpsRetVal   = XP_NO_ERR;
    sai_status_t          saiRetVal   = SAI_STATUS_SUCCESS;
    xpSaiMirrorSession_t  tmpSession;

    memcpy(&tmpSession, session, sizeof(xpSaiMirrorSession_t));

    switch (attr->id)
    {
        case SAI_MIRROR_SESSION_ATTR_MONITOR_PORTLIST_VALID:
            {
                session->monitorPortListValid = attr->value.booldata;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
            {
                sai_uint32_t monitorPort = (sai_uint32_t)xpSaiObjIdValueGet(attr->value.oid);

                if (XDK_SAI_OBJID_TYPE_CHECK(attr->value.oid, SAI_OBJECT_TYPE_PORT))
                {
                    // Fetch port interface id from port db
                    xpsRetVal = xpsPortGetPortIntfId(xpsDevId, monitorPort,
                                                     &tmpSession.monitorIntf);
                    if (xpsRetVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to get interface Id for port %d, error %d \n",
                                       monitorPort, xpsRetVal);
                        session->monitorIntf = XPS_INTF_INVALID_ID;
                        return xpsStatus2SaiStatus(xpsRetVal);
                    }
                }
                else
                {
                    tmpSession.monitorIntf = (xpsInterfaceId_t)monitorPort;
                }

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
            {
                tmpSession.vlanId = attr->value.u16;
                if (!session->vlanHdrValid)
                {
                    return SAI_STATUS_SUCCESS;
                }
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_HEADER_VALID:
            {
                tmpSession.vlanHdrValid = attr->value.booldata;
                if (!session->vlanId)
                {
                    return SAI_STATUS_SUCCESS;
                }
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_PRI:
            {
                tmpSession.vlanPri = attr->value.u8;
                if (!session->vlanHdrValid)
                {
                    return SAI_STATUS_SUCCESS;
                }
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_TPID:
            {
                tmpSession.vlanTpid = attr->value.u16;
                if (!session->vlanHdrValid)
                {
                    return SAI_STATUS_SUCCESS;
                }
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_CFI:
            {
                tmpSession.vlanCfi = attr->value.u8;
                if (!session->vlanHdrValid)
                {
                    return SAI_STATUS_SUCCESS;
                }
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS:
            {
                // Remove old tunnel
                saiRetVal = xpSaiMirrorEnhancedRemoteTunRemove(xpsDevId, &tmpSession);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to remove old tunnel\n");
                    return saiRetVal;
                }

                tmpSession.srcIpAddr.addr = attr->value.ipaddr.addr;

                // Create new tunnel
                saiRetVal = xpSaiMirrorEnhancedRemoteTunCreate(xpsDevId, &tmpSession);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to create new tunnel\n");
                    session->tnlIntfId = XPS_INTF_INVALID_ID;
                    return saiRetVal;
                }
                else
                {
                    session->tnlIntfId = tmpSession.tnlIntfId;
                }

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS:
            {
                // Remove old tunnel
                saiRetVal = xpSaiMirrorEnhancedRemoteTunRemove(xpsDevId, &tmpSession);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    return saiRetVal;
                }

                tmpSession.dstIpAddr.addr = attr->value.ipaddr.addr;

                // Create new tunnel
                saiRetVal = xpSaiMirrorEnhancedRemoteTunCreate(xpsDevId, &tmpSession);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    session->tnlIntfId = XPS_INTF_INVALID_ID;
                    return saiRetVal;
                }
                else
                {
                    session->tnlIntfId = tmpSession.tnlIntfId;
                }

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS:
            {
                COPY_MAC_ADDR_T(tmpSession.macSA, attr->value.mac);

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS:
            {
                COPY_MAC_ADDR_T(tmpSession.macDA, attr->value.mac);

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_TOS:
            {
                tmpSession.tos = attr->value.u8;

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE:
            {
                tmpSession.greProtoType = attr->value.u16;

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_TTL:
            {
                tmpSession.ttlHopLimit = attr->value.u8;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_TC:
            {
                tmpSession.tc = attr->value.u8;

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_TRUNCATE_SIZE:
            {
                tmpSession.truncateSize = attr->value.u16;

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_POLICER:
            {
                if (tmpSession.analyzerId != XP_MIRROR_ILLEGAL_SESSION)
                {
                    /* Remove old policer */
                    saiRetVal = xpSaiMirrorSessionPolicerRemove(tmpSession.analyzerId,
                                                                tmpSession.policer);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to remove policer\n");
                        return saiRetVal;
                    }
                    /* Add new policer */
                    saiRetVal = xpSaiMirrorSessionPolicerAdd(tmpSession.analyzerId,
                                                             attr->value.oid);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to add policer\n");
                        return saiRetVal;
                    }
                }

                tmpSession.policer = attr->value.oid;

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_SAMPLE_RATE:
            {
                tmpSession.sampleRate = attr->value.u32;

                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to set %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    saiRetVal = xpSaiMirrorSessionAnalyzerUpdate(xpsDevId, &tmpSession);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update analyzer session\n");
        return saiRetVal;
    }

    return saiRetVal;
}

//Func: xpSaiMirrorSessionInit

sai_status_t xpSaiMirrorSessionInit(xpsDevice_t xpsDevId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    gsaiMirrorDbHdle = XPSAI_MIRROR_SESSION_DB_HNDL;

    xpsRetVal = xpsStateRegisterDb(xpSaiScopeFromDevGet(xpsDevId), "Sai Mirror Db",
                                   XPS_GLOBAL, &xpSaiMirrorSessionEntryKeyComp, gsaiMirrorDbHdle);
    if (xpsRetVal != XP_NO_ERR)
    {
        gsaiMirrorDbHdle = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("SAI Mirror state DB deRegister failed :%d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiMirrorSessionDeInit

sai_status_t xpSaiMirrorSessionDeinit(xpsDevice_t xpsDevId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    xpsRetVal = xpsStateDeRegisterDb(xpSaiScopeFromDevGet(xpsDevId),
                                     &gsaiMirrorDbHdle);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI Mirror state DB Deregister failed :%d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiMirrorSessionCreate

sai_status_t xpSaiMirrorSessionCreate(sai_object_id_t* session_id,
                                      sai_object_id_t switch_id,
                                      uint32_t attr_count,
                                      const sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiMirrorSession_t *session = NULL;
    sai_object_id_t sessionOid = SAI_NULL_OBJECT_ID;
    sai_mirror_session_type_t sessionType = SAI_MIRROR_SESSION_TYPE_LOCAL;
    uint32_t internalSessionId = XP_SAI_MAX_MIRROR_SESSIONS_NUM;
    xpsDevice_t xpsDevId = 0;
    uint32_t count = 0;
    XP_STATUS retVal;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               MIRROR_SESSION_VALIDATION_ARRAY_SIZE, mirror_session_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    // Retrieve device Id from switch Id
    xpsDevId = xpSaiObjIdSwitchGet(switch_id);

    // Get session type
    for (count = 0; count < attr_count; count++)
    {
        if (attr_list[count].id == SAI_MIRROR_SESSION_ATTR_TYPE)
        {
            if ((attr_list[count].value.s32 < SAI_MIRROR_SESSION_TYPE_LOCAL) ||
                (attr_list[count].value.s32 > SAI_MIRROR_SESSION_TYPE_SFLOW))
            {
                XP_SAI_LOG_ERR("Error: SAI Mirror type is incorrect\n");
                return (SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count));
            }
            if ((attr_list[count].value.s32 == SAI_MIRROR_SESSION_TYPE_REMOTE))
            {
                XP_SAI_LOG_ERR("Error: SAI Mirror type RSPAN not supported\n");
                return (SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(count));
            }

            sessionType = (sai_mirror_session_type_t)attr_list[count].value.s32;
            break;
        }
    }

    if (count == attr_count)
    {
        XP_SAI_LOG_ERR("Error: SAI Mirror type not specified\n");
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    if ((saiRetVal = xpSaiSwitchMirrorSessionIdAllocate(&internalSessionId)) !=
        SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to allocate mirror session ID: saiRetVal %d \n",
                       saiRetVal);
        return saiRetVal;
    }

    // Create sessionOid for mirror object
    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_MIRROR_SESSION, xpsDevId,
                         (sai_uint64_t)internalSessionId, &sessionOid) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Error: SAI object can not be created\n");
        return SAI_STATUS_FAILURE;
    }

    if ((saiRetVal = xpSaiMirrorDbInfoCreate(xpsDevId, internalSessionId,
                                             &session)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create SAI mirror DB leaf: saiRetVal %d \n",
                       saiRetVal);
        xpSaiSwitchMirrorSessionIdRelease(internalSessionId);
        return saiRetVal;
    }

    session->sessionType = sessionType;

    // Set session attributes
    for (count = 0; count < attr_count; count++)
    {
        if (attr_list[count].id == SAI_MIRROR_SESSION_ATTR_TYPE)
        {
            continue;
        }
        if ((saiRetVal = xpSaiMirrorAttributeSetInternal(sessionOid, &attr_list[count],
                                                         true)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_NOTICE("Failed to set attribute %d : retVal %d\n",
                              attr_list[count].id, saiRetVal);

            // Releasing mirror session ID
            xpSaiSwitchMirrorSessionIdRelease(internalSessionId);

            // Remove the mirror session from the db
            retVal = xpsStateDeleteData(xpSaiObjIdScopeGet(sessionOid), gsaiMirrorDbHdle,
                                        (xpsDbKey_t)session, (void**)&session);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to delete data from Db\n");
                return xpsStatus2SaiStatus(retVal);
            }

            // Free the memory allocated for the corresponding state
            retVal = xpsStateHeapFree((void*)session);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to free allocated memory\n");
                return xpsStatus2SaiStatus(retVal);
            }
            return saiRetVal;
        }
    }

    saiRetVal = xpSaiMirrorSessionAdd(xpsDevId, session);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        // Releasing mirror session ID
        xpSaiSwitchMirrorSessionIdRelease(internalSessionId);

        // Remove the mirror session from the db
        retVal = xpsStateDeleteData(xpSaiObjIdScopeGet(sessionOid), gsaiMirrorDbHdle,
                                    (xpsDbKey_t)session, (void**)&session);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to delete data from Db\n");
            return xpsStatus2SaiStatus(retVal);
        }

        // Free the memory allocated for the corresponding state
        retVal = xpsStateHeapFree((void*)session);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to free allocated memory\n");
            return xpsStatus2SaiStatus(retVal);
        }
        return saiRetVal;
    }

    *session_id = sessionOid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiMirrorSessionRemove

sai_status_t xpSaiMirrorSessionRemove(sai_object_id_t session_id)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiMirrorSession_t *session = NULL;
    xpsDevice_t xpsDevId = 0;
    uint32_t internalSessionId = XP_SAI_MAX_MIRROR_SESSIONS_NUM;
    xpsInterfaceType_e intfType = XPS_PORT;
    uint32_t cpuPortNum  = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(session_id, SAI_OBJECT_TYPE_MIRROR_SESSION))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(session_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    // Retrieve device Id from session_id
    xpsDevId = xpSaiObjIdSwitchGet(session_id);
    internalSessionId = (uint32_t)xpSaiObjIdValueGet(session_id);

    saiRetVal = xpSaiMirrorDbInfoGet(session_id, &session);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to locate session %" PRIu64
                       " in data base, error: %d \n", session_id, saiRetVal);
        return saiRetVal;
    }

    if (session->sessionType == SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)
    {
        // Destroy Tunnel
        saiRetVal = xpSaiMirrorEnhancedRemoteTunRemove(xpsDevId, session);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed in xpSaiMirrorEnhancedRemoteTunRemove with error %d \n",
                           saiRetVal);
            return saiRetVal;
        }

    }

    xpsRetVal = xpsGlobalSwitchControlGetCpuPhysicalPortNum(xpsDevId, &cpuPortNum);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get CPU physical port number, error %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (session->monitorIntf != XPS_INTF_INVALID_ID &&
        session->monitorIntf != cpuPortNum)
    {
        xpsRetVal = xpsInterfaceGetType(session->monitorIntf, &intfType);
        if (XP_NO_ERR != xpsRetVal)
        {
            XP_SAI_LOG_ERR("Failed to get type for interface %u, error: %d\n",
                           session->monitorIntf, xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
        if (XPS_LAG == intfType)
        {
            /* If current analyzer interface is LAG - remove reference before update */
            xpsRetVal = xpsLagSetMirrorEnable(session->monitorIntf, false, 0);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                return xpsStatus2SaiStatus(xpsRetVal);
            }

            saiRetVal = xpSaiLagUnref(session->monitorIntf);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Unable to add LAG reference, sai_error: %d\n", saiRetVal);
                return XP_ERR_INVALID_ID;
            }
        }
    }

    // Remove ingress analyzer session from hw
    saiRetVal = xpSaiMirrorSessionAnalyzerRemoveOrUnref(xpsDevId, session, true,
                                                        true);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    // Releasing mirror session ID
    xpSaiSwitchMirrorSessionIdRelease(internalSessionId);

    // Remove the mirror session from the db
    xpsRetVal = xpsStateDeleteData(xpSaiObjIdScopeGet(session_id), gsaiMirrorDbHdle,
                                   (xpsDbKey_t)session, (void**)&session);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to delete data from Db\n");
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    // Free the memory allocated for the corresponding state
    xpsRetVal = xpsStateHeapFree((void*)session);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to free allocated memory\n");
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return xpsStatus2SaiStatus(xpsRetVal);
}

/**
 * \brief Enables/disables mirroring on a particular port
 *
 * \param portNum - mirroring port number
 * \param session - mirroring session OID
 * \param ingress - mirroring type("true" if ingress, otherwise - "false")
 * \param enable -  enable or disable mirroring
 *
 * \return sai_status_t
 */
sai_status_t xpSaiMirrorSessionPortMirrorEnable(xpsInterfaceId_t portIntf,
                                                sai_object_id_t session_id, bool ingress, bool enable)
{
    sai_status_t          saiRetVal   = SAI_STATUS_SUCCESS;
    XP_STATUS             xpsStatus   = XP_NO_ERR;
    xpsDevice_t           xpsDevId    = 0;
    xpSaiMirrorSession_t *session     = NULL;
    uint32_t              analyzerId  = XP_MIRROR_ILLEGAL_SESSION;
    bool                  isErspan    = false;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(session_id, SAI_OBJECT_TYPE_MIRROR_SESSION))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(session_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    // Retrieve device Id from session_id
    xpsDevId = xpSaiObjIdSwitchGet(session_id);

    saiRetVal = xpSaiMirrorDbInfoGet(session_id, &session);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to locate session %" PRIu64
                       " in data base, error: %d \n", session_id, saiRetVal);
        return saiRetVal;
    }

    if (session->sessionType == SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)
    {
        isErspan = true;
    }

    analyzerId = session->analyzerId;
    if (enable)
    {
        // Enable mirroring on port
        saiRetVal = xpSaiMirrorSessionAnalyzerCreateOrRef(xpsDevId, session, ingress);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            return saiRetVal;
        }

        // Enable mirroring on port
        xpsStatus = xpsPortEnableMirroring(portIntf, analyzerId,
                                           ingress, isErspan, true);
        if (xpsStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to enable mirroring on port %u", portIntf);
            xpSaiMirrorSessionAnalyzerRemoveOrUnref(xpsDevId, session, ingress, false);
            return xpsStatus2SaiStatus(xpsStatus);
        }

        // Mark mirroring enabled on a port.
        if (ingress)
        {
            session->mirroredPortList[portIntf].ingressEn = true;
        }
        else
        {
            session->mirroredPortList[portIntf].egressEn = true;
        }
    }
    else
    {
        // Disable mirroring on port
        if ((ingress && !session->mirroredPortList[portIntf].ingressEn) ||
            (!ingress && !session->mirroredPortList[portIntf].egressEn))
        {
            // Mirroring was not enabled previously on this port so nothing to do here
            return SAI_STATUS_SUCCESS;
        }

        xpsStatus = xpsPortEnableMirroring(portIntf, analyzerId,
                                           ingress, isErspan, false);
        if (xpsStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to disable mirroring on port %u", portIntf);
            return xpsStatus2SaiStatus(xpsStatus);
        }

        saiRetVal = xpSaiMirrorSessionAnalyzerRemoveOrUnref(xpsDevId, session, ingress,
                                                            false);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            return saiRetVal;
        }

        // Mark mirroring disabled on a port.
        if (ingress)
        {
            session->mirroredPortList[portIntf].ingressEn = false;
        }
        else
        {
            session->mirroredPortList[portIntf].egressEn = false;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiMirrorAttributeSet

sai_status_t xpSaiMirrorAttributeSet(sai_object_id_t session_id,
                                     const sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    saiRetVal = xpSaiAttrCheck(1, attr,
                               MIRROR_SESSION_VALIDATION_ARRAY_SIZE, mirror_session_attribs,
                               SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiMirrorAttributeSetInternal(session_id, attr, false);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute processing failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    return saiRetVal;
}

/**
 * \brief Sets mirror session attribute
 *
 *  Internal API which implements attribute set logic
 *
 * \param session_id - mirroring session OID
 * \param attr - attribute to be set
 * \param onCreate - signalizes whether function is
 *                   called on session create (true)
 *                   or update (false)
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiMirrorAttributeSetInternal(sai_object_id_t session_id,
                                                    const sai_attribute_t *attr, bool onCreate)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpSaiMirrorSession_t *session = NULL;
    xpsDevice_t xpsDevId = 0;
    sai_uint32_t portNum = 0;
    xpsInterfaceType_e intfType = XPS_PORT;
    int sessionKey = -1;
    xpSaiMirrorSession_t *tmpSessionEntry;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(session_id, SAI_OBJECT_TYPE_MIRROR_SESSION))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(session_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    // Retrieve device Id from session_id
    xpsDevId = xpSaiObjIdSwitchGet(session_id);

    saiRetVal = xpSaiMirrorDbInfoGet(session_id, &session);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to locate session %" PRIu64
                       " in data base, error: %d \n", session_id, saiRetVal);
        return saiRetVal;
    }

    switch (attr->id)
    {
        case SAI_MIRROR_SESSION_ATTR_MONITOR_PORTLIST_VALID:
            {
                session->monitorPortListValid = attr->value.booldata;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
            {
                if (session->monitorPortListValid == true)
                {
                    XP_SAI_LOG_ERR("Invalid attribute condition. SAI_MIRROR_SESSION_ATTR_MONITOR_PORTLIST_VALID should be false");
                    return SAI_STATUS_INVALID_PARAMETER;
                }

                // Verify port oid
                if (!XDK_SAI_OBJID_TYPE_CHECK(attr->value.oid, SAI_OBJECT_TYPE_PORT)
                    && !XDK_SAI_OBJID_TYPE_CHECK(attr->value.oid, SAI_OBJECT_TYPE_LAG))
                {
                    XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                                   xpSaiObjIdTypeGet(attr->value.oid));
                    return SAI_STATUS_INVALID_PARAMETER;
                }

                portNum = (sai_uint32_t)xpSaiObjIdValueGet(attr->value.oid);

                if (attr->value.oid != session->monitorPortOid)
                {
                    xpsInterfaceId_t intf = XPS_INTF_INVALID_ID;
                    sai_object_id_t cpuPort;
                    xpSaiGetSwitchAttrCPUPort(&cpuPort);
                    if (XDK_SAI_OBJID_TYPE_CHECK(attr->value.oid, SAI_OBJECT_TYPE_PORT) &&
                        attr->value.oid != cpuPort)
                    {
                        // Fetch port interface id from port db
                        xpsRetVal = xpsPortGetPortIntfId(xpsDevId, portNum, &intf);
                        if (xpsRetVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Failed to get interface Id for port %d, error %d \n", portNum,
                                           xpsRetVal);
                            return xpsStatus2SaiStatus(xpsRetVal);
                        }
                    }
                    else
                    {
                        intf = (xpsInterfaceId_t)portNum;
                    }

                    if (!onCreate)
                    {
                        xpsInterfaceId_t currAnalyzer = session->monitorIntf;
                        xpsRetVal = xpsInterfaceGetType(currAnalyzer, &intfType);
                        if (XP_NO_ERR != xpsRetVal)
                        {
                            XP_SAI_LOG_ERR("Failed to get type for interface %u, error: %d\n", currAnalyzer,
                                           xpsRetVal);
                            return xpsStatus2SaiStatus(xpsRetVal);
                        }

                        if (XPS_LAG == intfType)
                        {
                            /* If current analyzer interface is LAG - remove reference before update */
                            xpsRetVal = xpsLagSetMirrorEnable(session->monitorIntf, false, 0);
                            if (saiRetVal != SAI_STATUS_SUCCESS)
                            {
                                return xpsStatus2SaiStatus(xpsRetVal);
                            }

                            saiRetVal = xpSaiLagUnref(session->monitorIntf);
                            if (saiRetVal != SAI_STATUS_SUCCESS)
                            {
                                XP_SAI_LOG_ERR("Unable to add LAG reference, sai_error: %d\n", saiRetVal);
                                return XP_ERR_INVALID_ID;
                            }
                        }

                        saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_DBG("Failed to update mirror session monitor port with error: %d\n",
                                           saiRetVal);
                            return saiRetVal;
                        }
                    }

                    if (XDK_SAI_OBJID_TYPE_CHECK(attr->value.oid, SAI_OBJECT_TYPE_LAG))
                    {
                        saiRetVal = xpSaiLagRef(intf);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("Unable to add LAG reference, sai_error: %d\n", saiRetVal);
                            return XP_ERR_INVALID_ID;
                        }

                        xpsRetVal = xpsLagSetMirrorEnable(intf, true, session->keySessionId);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            return xpsStatus2SaiStatus(xpsRetVal);
                        }
                    }

                    session->monitorIntf = intf;
                    session->monitorPortOid = attr->value.oid;
                }

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
            {
                if (session->sessionType == SAI_MIRROR_SESSION_TYPE_LOCAL ||
                    session->sessionType == SAI_MIRROR_SESSION_TYPE_SFLOW)
                {
                    return SAI_STATUS_FAILURE;
                }

                if (attr->value.u16 != session->vlanId)
                {
                    if (!onCreate)
                    {
                        saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_DBG("Failed to update mirror session VLAN ID with error: %d\n",
                                           saiRetVal);
                            return saiRetVal;
                        }
                    }

                    session->vlanId = attr->value.u16;
                }

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_HEADER_VALID:
            {
                if (session->sessionType != SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)
                {
                    return SAI_STATUS_FAILURE;
                }

                if (session->vlanHdrValid != attr->value.booldata && session->vlanId)
                {
                    saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                }

                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_DBG("Failed to update mirror session VLAN ID with error: %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }

                session->vlanHdrValid = attr->value.booldata;

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_PRI:
            {
                if (session->vlanHdrValid != true &&
                    session->sessionType != SAI_MIRROR_SESSION_TYPE_REMOTE)
                {
                    return SAI_STATUS_FAILURE;
                }

                if (attr->value.u8 != session->vlanPri)
                {
                    if (!onCreate)
                    {
                        saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_DBG("Failed to update mirror session VLAN priority with error: %d\n",
                                           saiRetVal);
                            return saiRetVal;
                        }
                    }

                    session->vlanPri = attr->value.u8;
                }

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_CFI:
            {
                if (session->vlanHdrValid != true &&
                    session->sessionType != SAI_MIRROR_SESSION_TYPE_REMOTE)
                {
                    return SAI_STATUS_FAILURE;
                }

                if (attr->value.u8 != session->vlanCfi)
                {
                    if (!onCreate)
                    {
                        saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_DBG("Failed to update mirror session VLAN priority with error: %d\n",
                                           saiRetVal);
                            return saiRetVal;
                        }
                    }

                    session->vlanCfi = attr->value.u8;
                }

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_TPID:
            {
                if (session->vlanHdrValid != true &&
                    session->sessionType != SAI_MIRROR_SESSION_TYPE_REMOTE)
                {
                    return SAI_STATUS_FAILURE;
                }

                if (attr->value.u16 != session->vlanTpid)
                {
                    if (!onCreate)
                    {
                        saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_DBG("Failed to update mirror session VLAN priority with error: %d\n",
                                           saiRetVal);
                            return saiRetVal;
                        }
                    }

                    session->vlanTpid = attr->value.u16;
                }

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_ERSPAN_ENCAPSULATION_TYPE:
            {
                if (session->sessionType != SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)
                {
                    return SAI_STATUS_FAILURE;
                }

                if ((sai_erspan_encapsulation_type_t)attr->value.s32 !=
                    SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL)
                {
                    return SAI_STATUS_INVALID_ATTRIBUTE_0;
                }

                session->encapType = (sai_erspan_encapsulation_type_t)attr->value.s32;
                //TODO: Set to HW
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION:
            {
                if (session->sessionType != SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)
                {
                    return SAI_STATUS_FAILURE;
                }

                if (attr->value.s8 == SAI_IPHDR_VERSION_IPV4) //SONIC_WA
                {
                    session->ipType = SAI_IP_ADDR_FAMILY_IPV4;
                }
                else if (attr->value.s8 == SAI_IPHDR_VERSION_IPV6)
                {
                    session->ipType = SAI_IP_ADDR_FAMILY_IPV6;
                }
                else
                {
                    return SAI_STATUS_NOT_SUPPORTED;
                }
                session->srcIpAddr.addr_family = session->dstIpAddr.addr_family =
                                                     (sai_ip_addr_family_t)session->ipType;
                //TODO: Set to HW
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS:
            {
                if (session->sessionType != SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)
                {
                    return SAI_STATUS_FAILURE;
                }

                if (!xpSaiIpIsEqual(&session->srcIpAddr,
                                    (sai_ip_address_t *)&attr->value.ipaddr))
                {
                    if (!onCreate)
                    {
                        saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("Failed to update enhanced remote session source IP with error %d\n",
                                           saiRetVal);
                            return saiRetVal;
                        }
                    }

                    session->srcIpAddr.addr = attr->value.ipaddr.addr;
                }

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS:
            {
                if (session->sessionType != SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)

                {
                    return SAI_STATUS_FAILURE;
                }

                if (!xpSaiIpIsEqual(&session->dstIpAddr,
                                    (sai_ip_address_t *)&attr->value.ipaddr))
                {
                    if (!onCreate)
                    {
                        saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("Failed to update enhanced remote session destination IP with error %d\n",
                                           saiRetVal);
                            return saiRetVal;
                        }
                    }

                    session->dstIpAddr.addr = attr->value.ipaddr.addr;
                }

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS:
            {
                if (session->sessionType != SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)
                {
                    return SAI_STATUS_FAILURE;
                }

                if (memcmp(session->macSA, attr->value.mac, sizeof(sai_mac_t)) != 0)
                {
                    memcpy(session->macSA, attr->value.mac, sizeof(sai_mac_t));
                    if (!onCreate)
                    {
                        saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("Failed to update enhanced remote session SRC MAC with error %d\n",
                                           saiRetVal);
                            return saiRetVal;
                        }
                    }
                }
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS:
            {
                if (session->sessionType != SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)
                {
                    return SAI_STATUS_FAILURE;
                }

                if (memcmp(session->macDA, attr->value.mac, sizeof(sai_mac_t)) != 0)
                {
                    memcpy(session->macDA, attr->value.mac, sizeof(sai_mac_t));
                    if (!onCreate)
                    {
                        saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("Failed to update enhanced remote session DST MAC with error %d\n",
                                           saiRetVal);
                            return saiRetVal;
                        }
                    }
                }

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_TOS:
            {
                if (session->sessionType != SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)
                {
                    return SAI_STATUS_FAILURE;
                }
                session->tos = attr->value.u8;

                if (!onCreate)
                {
                    saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to update enhanced remote session TOS with error %d\n",
                                       saiRetVal);
                        return saiRetVal;
                    }
                }
                break;
            }

        case SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE:
            {
                if (session->sessionType != SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)
                {
                    return SAI_STATUS_FAILURE;
                }
                session->greProtoType= attr->value.u16;

                if (!onCreate)
                {
                    saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to update enhanced remote session GRE protocol type with error %d\n",
                                       saiRetVal);
                        return saiRetVal;
                    }
                }
                break;
            }

        case SAI_MIRROR_SESSION_ATTR_TTL:
            {
                if (session->sessionType != SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)
                {
                    return SAI_STATUS_FAILURE;
                }
                session->ttlHopLimit = attr->value.u8;

                if (!onCreate)
                {
                    saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to update enhanced remote session TTL with error %d\n",
                                       saiRetVal);
                        return saiRetVal;
                    }
                }
                break;
            }

        case SAI_MIRROR_SESSION_ATTR_TRUNCATE_SIZE:
            {
                session->truncateSize = attr->value.u16;

                if (!onCreate)
                {
                    saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to update mirror session TRUNCATE size with error %d\n",
                                       saiRetVal);
                        return saiRetVal;
                    }
                }
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_TC:
            {
                /* only single tc value is supported */
                while (xpSaiMirrorDbInfoGetNext(sessionKey, &tmpSessionEntry) != XP_NO_ERR)
                {
                    sessionKey = tmpSessionEntry->keySessionId;
                    if (session == tmpSessionEntry)
                    {
                        continue;
                    }
                    if (tmpSessionEntry->tc != attr->value.u8)
                    {
                        XP_SAI_LOG_ERR("Only single TC value is supported for all mirror session. Previous used value %d\n",
                                       tmpSessionEntry->tc);
                        return SAI_STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                if (session->tc != attr->value.u8)
                {
                    if (!onCreate)
                    {
                        saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("Failed to update mirror session TC with error %d\n",
                                           saiRetVal);
                            return saiRetVal;
                        }
                    }

                    session->tc = attr->value.u8;
                }
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_POLICER:
            {
                // Verify policer oid
                if ((attr->value.oid != SAI_NULL_OBJECT_ID) &&
                    (!XDK_SAI_OBJID_TYPE_CHECK(attr->value.oid, SAI_OBJECT_TYPE_POLICER)))
                {
                    XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                                   xpSaiObjIdTypeGet(attr->value.oid));
                    return SAI_STATUS_INVALID_PARAMETER;
                }

                if (session->policer != attr->value.oid)
                {
                    if (!onCreate)
                    {
                        saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_DBG("Failed to update mirror session POLICER ID with error: %d\n",
                                           saiRetVal);
                            return saiRetVal;
                        }
                    }

                    session->policer = attr->value.oid;
                }

                break;
            }
        case SAI_MIRROR_SESSION_ATTR_CONGESTION_MODE:
            {
                if (attr->value.u32 != SAI_MIRROR_SESSION_CONGESTION_MODE_INDEPENDENT)
                {
                    saiRetVal = SAI_STATUS_NOT_SUPPORTED;
                }
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_SAMPLE_RATE:
            {
                if (attr->value.u32 != session->sampleRate)
                {
                    if (!onCreate)
                    {
                        saiRetVal = xpSaiMirrorSessionUpdate(xpsDevId, session, attr);
                        if (saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_DBG("Failed to update mirror session sample rate with error: %d\n",
                                           saiRetVal);
                            return saiRetVal;
                        }
                    }

                    session->sampleRate = attr->value.u32;
                }
                break;
            }

        default:
            {
                XP_SAI_LOG_ERR("Failed to set %d\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    return saiRetVal;
}

//Func: xpSaiGetMirrorAttribute

static sai_status_t xpSaiGetMirrorAttribute(sai_object_id_t session_id,
                                            sai_attribute_t* attr, uint32_t attr_index)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiMirrorSession_t *session = NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(session_id, SAI_OBJECT_TYPE_MIRROR_SESSION))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(session_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMirrorDbInfoGet(session_id, &session);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to locate session %" PRIu64
                       " in data base, error: %d \n", session_id, saiRetVal);
        return saiRetVal;
    }

    switch (attr->id)
    {
        case SAI_MIRROR_SESSION_ATTR_TYPE:
            {
                attr->value.s32 = session->sessionType;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
            {
                attr->value.oid = session->monitorPortOid;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
            {
                attr->value.u16 = session->vlanId;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_HEADER_VALID:
            {
                attr->value.booldata = session->vlanHdrValid;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_ERSPAN_ENCAPSULATION_TYPE:
            {
                attr->value.s32 = session->encapType;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION:
            {
                attr->value.s8 = session->ipType;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS:
            {
                attr->value.ipaddr.addr = session->srcIpAddr.addr;
                attr->value.ipaddr.addr_family = session->srcIpAddr.addr_family;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS:
            {
                attr->value.ipaddr.addr = session->dstIpAddr.addr;
                attr->value.ipaddr.addr_family = session->dstIpAddr.addr_family;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS:
            {
                memcpy(attr->value.mac, session->macSA, sizeof(sai_mac_t));
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS:
            {
                memcpy(attr->value.mac, session->macDA, sizeof(sai_mac_t));
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_TRUNCATE_SIZE:
            {
                attr->value.u16 = session->truncateSize;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_TC:
            {
                attr->value.u8 = session->tc;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_TOS:
            {
                attr->value.u8 = session->tos;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_TTL:
            {
                attr->value.u8 = session->ttlHopLimit;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE:
            {
                attr->value.u16 = session->greProtoType;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_CONGESTION_MODE:
            {
                attr->value.u32 = SAI_MIRROR_SESSION_CONGESTION_MODE_INDEPENDENT;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_SAMPLE_RATE:
            {
                attr->value.u32 = session->sampleRate;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_PRI:
            {
                attr->value.u8 = session->vlanPri;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_CFI:
            {
                attr->value.u8 = session->vlanCfi;
                break;
            }
        case SAI_MIRROR_SESSION_ATTR_VLAN_TPID:
            {
                attr->value.u16 = session->vlanTpid;
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

//Func: xpSaiGetMirrorAttributes

static sai_status_t xpSaiGetMirrorAttributes(sai_object_id_t session_id,
                                             uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               MIRROR_SESSION_VALIDATION_ARRAY_SIZE, mirror_session_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }


    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetMirrorAttribute(session_id, &attr_list[count], count);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetMirrorAttribute failed with error %d \n", saiRetVal);
            return saiRetVal;
        }
    }
    return saiRetVal;
}

//Func: xpSaiBulkGetMirrorAttributes

sai_status_t xpSaiBulkGetMirrorAttributes(sai_object_id_t id,
                                          uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_MIRROR_SESSION))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountMirrorAttribute(&maxcount);
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
        attr_list[idx].id = SAI_MIRROR_SESSION_ATTR_START + count;
        saiRetVal = xpSaiGetMirrorAttribute(id, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiMirrorApiInit

sai_status_t xpSaiMirrorApiInit(uint64_t flag,
                                const sai_service_method_table_t* adapHostServiceMethodTable)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiMirrorApiInit\n");

    _xpSaiMirrorApi = (sai_mirror_api_t *) xpMalloc(sizeof(sai_mirror_api_t));
    if (NULL == _xpSaiMirrorApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for __xpSaiMirrorApi\n");
        return SAI_STATUS_NO_MEMORY;
    }

    _xpSaiMirrorApi->create_mirror_session = xpSaiMirrorSessionCreate;
    _xpSaiMirrorApi->remove_mirror_session = xpSaiMirrorSessionRemove;
    _xpSaiMirrorApi->set_mirror_session_attribute = xpSaiMirrorAttributeSet;
    _xpSaiMirrorApi->get_mirror_session_attribute = xpSaiGetMirrorAttributes;

    saiRetVal = xpSaiApiRegister(SAI_API_MIRROR, (void*)_xpSaiMirrorApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register mirror API\n");
        return saiRetVal;
    }

    return saiRetVal;
}

//Func: xpSaiMirrorApiDeInit

sai_status_t xpSaiMirrorApiDeInit()
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiMirrorApiDeInit\n");

    xpFree(_xpSaiMirrorApi);
    _xpSaiMirrorApi = NULL;

    return saiRetVal;
}

/**
 * \brief Resets xpSaiMirrorSession_t instance to defaults
 *
 * \param session - pointer to xpSaiMirrorSession_t instance to be reset
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiMirrorSessionSetDefaults(xpSaiMirrorSession_t *session)
{
    XP_STATUS   xpsRetVal  = XP_NO_ERR;
    xpsDevice_t xpsDevId   = 0;
    uint32_t    maxPortNum = 0;

    // Get max port num
    xpsRetVal = xpsPortGetMaxNum(xpsDevId, &maxPortNum);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve maximum port number\n");
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (session)
    {
        memset(session, 0, sizeof(xpSaiMirrorSession_t));
        session->keySessionId = XP_SAI_MAX_MIRROR_SESSIONS_NUM;
        session->analyzerId = XP_MIRROR_ILLEGAL_SESSION;
        session->analyzerRefCnt = 0;
        session->monitorIntf = XPS_INTF_INVALID_ID;
        session->tnlIntfId = XPS_INTF_INVALID_ID;
        session->monitorPortOid = SAI_NULL_OBJECT_ID;
        session->vlanHdrValid = false;
        session->ttlHopLimit = 255;
        session->tos = 0;
        session->greProtoType = 0;
        session->policer = SAI_NULL_OBJECT_ID;
        session->tc = 0;
        session->truncateSize = 0;
        session->sampleRate = 0;
    }

    return SAI_STATUS_SUCCESS;
}


/**
 * \brief Creates tunnel for mirror enhanced remote session
 *
 * \param xpsDevId - device ID
 * \param session - pointer to xpSaiMirrorSession_t instance
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiMirrorEnhancedRemoteTunCreate(xpsDevice_t xpsDevId,
                                                       xpSaiMirrorSession_t *session)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsMirrorData_t mirrorData;

    memset(&mirrorData, 0, sizeof(mirrorData));

    if (session->ipType == SAI_IP_ADDR_FAMILY_IPV4)
    {
        mirrorData.erspan2Data.ipType = XP_PREFIX_TYPE_IPV4;
    }
    else
    {
        mirrorData.erspan2Data.ipType = XP_PREFIX_TYPE_IPV6;
    }
    xpSaiIpCopy((uint8_t*)&mirrorData.erspan2Data.srcIpAddr,
                (uint8_t*)&session->srcIpAddr.addr,
                (sai_ip_addr_family_t)session->ipType);
    xpSaiIpCopy((uint8_t*)&mirrorData.erspan2Data.dstIpAddr,
                (uint8_t*)&session->dstIpAddr.addr,
                (sai_ip_addr_family_t)session->ipType);

    xpsRetVal = xpsMirrorErspanTunCreate(xpsDevId, session->analyzerId,
                                         &mirrorData);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed in xpsMirrorErspanTunCreate with error %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Removes tunnel for mirror enhanced remote session
 *
 * \param xpsDevId - device ID
 * \param session - pointer to xpSaiMirrorSession_t instance
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiMirrorEnhancedRemoteTunRemove(xpsDevice_t xpsDevId,
                                                       xpSaiMirrorSession_t *session)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    xpsRetVal = xpsMirrorErspanTunRemove(xpsDevId, session->analyzerId);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed in xpsMirrorErspanTunRemove with error %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Add policer to monitor port
 *
 * \param analyzerId  - analyzer index
 * \param polcierId   - policer object id
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiMirrorSessionPolicerAdd(uint32_t analyzerId,
                                                 sai_object_id_t policerId)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS    xpsRetVal = XP_NO_ERR;
    xpsDevice_t  xpDevId   = xpSaiObjIdSwitchGet(policerId);
    sai_uint32_t index     = 0;
    sai_uint32_t ePort     = 0;
    xpsPolicerEntry_t xpPolicerEntry;

    memset(&xpPolicerEntry, 0, sizeof(xpPolicerEntry));

    if (policerId == SAI_NULL_OBJECT_ID)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(policerId, SAI_OBJECT_TYPE_POLICER))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(policerId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsRetVal = xpsMirrorGetAnalyzerIntfPolicerIndex(analyzerId, &ePort, &index);
    if (xpsRetVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return xpsRetVal;
    }

    /* Update the sai policer db index list */
    if ((xpsRetVal = xpSaiPolicerAddIdxToIdxList(policerId,
                                                 XP_EGRESS_FLOW_POLICER, index)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to add port id to sai policer db index list |Error: %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Get xp policer entry for given policer object */
    saiRetVal = xpSaiPolicerGetXpsEntry(policerId, &xpPolicerEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to get xp policer entry for given policer "
                       "object 0x%lx, status: %d", policerId, saiRetVal);
        return saiRetVal;
    }

    /* Add policer entry */
    xpsRetVal = xpsPolicerAddEntry(xpDevId, XP_EGRESS_FLOW_POLICER, index,
                                   index, &xpPolicerEntry);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to add policer entry, xpsRetVal: %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Clear policer counter entry */
    if ((xpsRetVal = xpsPolicerClearPolicerCounterEntry(xpDevId,
                                                        XP_EGRESS_FLOW_POLICER, index)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to clear policer counter |Error: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Enable e-port policer */
    if ((xpsRetVal = xpsPolicerEPortPolicingEnable(xpDevId, ePort,
                                                   XP_EGRESS_FLOW_POLICER, index, true)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to enable egress eport policer |Error: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Disable policer to monitor port
 *
 * \param analyzerId  - analyzer index
 * \param polcierId   - policer object id
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiMirrorSessionPolicerRemove(uint32_t analyzerId,
                                                    sai_object_id_t policerId)
{
    XP_STATUS    xpsRetVal = XP_NO_ERR;
    xpsDevice_t  xpDevId   = xpSaiObjIdSwitchGet(policerId);
    sai_uint32_t index     = 0;
    sai_uint32_t ePort     = 0;

    if (policerId == SAI_NULL_OBJECT_ID)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(policerId, SAI_OBJECT_TYPE_POLICER))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(policerId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsRetVal = xpsMirrorGetAnalyzerIntfPolicerIndex(analyzerId, &ePort, &index);
    if (xpsRetVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return xpsRetVal;
    }

    /* Remove port from sai policer db index list */
    if ((xpsRetVal = xpSaiPolicerRemoveIdxFromIdxList(policerId,
                                                      XP_EGRESS_FLOW_POLICER,
                                                      index)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed xpSaiPolicerRemoveIdxFromIdxList |Error: %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Remove port policer */
    if ((xpsRetVal = xpsPolicerRemoveEntry(xpDevId, XP_EGRESS_FLOW_POLICER,
                                           index)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to remove policer entry |Error: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Disable e-port policer */
    if ((xpsRetVal = xpsPolicerEPortPolicingEnable(xpDevId, ePort,
                                                   XP_EGRESS_FLOW_POLICER, index, false)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to disable egress eport policer |Error: %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Creates or references analyzer session
 *
 *  If there is no analyzer session created on HW
 *  this function will create it, otherwise it
 *  increments session reference counter
 *
 * \param xpsDevId - device ID
 * \param session - pointer to xpSaiMirrorSession_t instance
 * \param ingress - analyzer type(ingress or egress)
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiMirrorSessionAnalyzerCreateOrRef(xpsDevice_t xpsDevId,
                                                          xpSaiMirrorSession_t *session, bool ingress)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    uint32_t *analyzerId = NULL;
    uint32_t *analyzerRefCnt = NULL;
    xpMirrorType_e mirrorType = XP_MIRROR_TOTAL_TYPE;
    xpsMirrorData_t mirrorData;

    if (session == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&mirrorData, 0, sizeof(xpsMirrorData_t));

    analyzerId = &session->analyzerId;
    analyzerRefCnt = &session->analyzerRefCnt;

    if (*analyzerId != XP_MIRROR_ILLEGAL_SESSION)
    {
        /* Update the session-id based on the direction.
           This check will hit whenever mirror is attached to Port/ACL */

        if (session->sessionType == SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)
        {
            xpsRetVal = xpsMirrorUpdateSessionId(xpsDevId, *analyzerId, ingress);
            if (xpsRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to create analyzer session: xpsRetVal %d \n", xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }
        }

        // Session exists. Just increment ref counter and return
        (*analyzerRefCnt)++;
        return SAI_STATUS_SUCCESS;
    }


    if (session->sessionType == SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)
    {
        mirrorData.erspan2Data.isIngress = ingress;
        mirrorData.erspan2Data.erspanId = session->tnlIntfId;
        mirrorData.erspan2Data.tos = session->tos;
        mirrorData.erspan2Data.greProtoType = session->greProtoType;
        mirrorData.erspan2Data.truncateSize = session->truncateSize;
        mirrorData.erspan2Data.analyzerIntfId = session->monitorIntf;
        mirrorData.erspan2Data.tc = session->tc;
        mirrorData.erspan2Data.ttlHopLimit = session->ttlHopLimit;
        mirrorData.erspan2Data.tos = session->tos;
        mirrorData.erspan2Data.sampleRate = session->sampleRate;
        mirrorData.erspan2Data.policerId = (uint32_t)xpSaiObjIdValueGet(
                                               session->policer);
        COPY_MAC_ADDR_T(&mirrorData.erspan2Data.srcMacAddr, &session->macSA);
        COPY_MAC_ADDR_T(&mirrorData.erspan2Data.dstMacAddr, &session->macDA);

        if (session->ipType == SAI_IP_ADDR_FAMILY_IPV4)
        {
            mirrorData.erspan2Data.ipType = XP_PREFIX_TYPE_IPV4;
        }
        else
        {
            mirrorData.erspan2Data.ipType = XP_PREFIX_TYPE_IPV6;
        }
        xpSaiIpCopy((uint8_t*)&mirrorData.erspan2Data.srcIpAddr,
                    (uint8_t*)&session->srcIpAddr.addr,
                    (sai_ip_addr_family_t)session->ipType);
        xpSaiIpCopy((uint8_t*)&mirrorData.erspan2Data.dstIpAddr,
                    (uint8_t*)&session->dstIpAddr.addr,
                    (sai_ip_addr_family_t)session->ipType);

        if (session->vlanHdrValid)
        {
            mirrorData.erspan2Data.vlanId = session->vlanId;
            mirrorData.erspan2Data.vlanPri = session->vlanPri;
            mirrorData.erspan2Data.vlanCfi = session->vlanCfi;
            mirrorData.erspan2Data.vlanTpid = session->vlanTpid;
        }
        mirrorType = XP_MIRROR_ERSPAN2;
    }
    else if (session->sessionType == SAI_MIRROR_SESSION_TYPE_REMOTE)
    {
        mirrorData.rspanData.bdId = session->vlanId;
        mirrorType = XP_MIRROR_RSPAN;
    }
    else
    {
        mirrorData.spanData.truncateSize = session->truncateSize;
        mirrorData.spanData.isIngress = ingress;
        mirrorData.spanData.analyzerIntfId = session->monitorIntf;
        mirrorData.spanData.sampleRate = session->sampleRate;
        mirrorData.spanData.tc = session->tc;
        mirrorData.spanData.policerId = (uint32_t)xpSaiObjIdValueGet(
                                            session->policer);
        mirrorType = XP_MIRROR_LOCAL;
    }

    // Create mirror session
    if ((xpsRetVal = xpsMirrorCreateAnalyzerSession(mirrorType, mirrorData,
                                                    analyzerId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to create analyzer session: xpsRetVal %d \n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if ((xpsRetVal = xpsMirrorAddAnalyzerInterface(*analyzerId,
                                                   session->monitorIntf)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to add analyzer interface to analyzer session: xpsRetVal %d \n",
                       xpsRetVal);

        xpsMirrorRemoveAnalyzerSession(xpsDevId, *analyzerId);
        *analyzerId = XP_MIRROR_ILLEGAL_SESSION;

        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if ((xpsRetVal = xpsMirrorWriteAnalyzerSession(xpsDevId,
                                                   *analyzerId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to write analyzer session: xpsRetVal %d \n", xpsRetVal);

        xpsMirrorRemoveAnalyzerSession(xpsDevId, *analyzerId);
        *analyzerId = XP_MIRROR_ILLEGAL_SESSION;

        return xpsStatus2SaiStatus(xpsRetVal);
    }

    saiRetVal = xpSaiMirrorSessionPolicerAdd(*analyzerId, session->policer);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to add mirror session policer: %d\n", saiRetVal);
        return saiRetVal;
    }

    (*analyzerRefCnt)++;

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Unreferences and possibly removes analyzer session from HW
 *
 * \param xpsDevId - device ID
 * \param session - pointer to xpSaiMirrorSession_t instance
 * \param ingress - analyzer type(ingress or egress)
 * \param forceRemove - notifies that analiser has to be removed anyway
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiMirrorSessionAnalyzerRemoveOrUnref(
    xpsDevice_t xpsDevId, xpSaiMirrorSession_t *session, bool ingress,
    bool forceRemove)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    uint32_t *analyzerId = NULL;
    uint32_t *analyzerRefCnt = NULL;

    if (session == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    analyzerId = &session->analyzerId;
    analyzerRefCnt = &session->analyzerRefCnt;

    if (*analyzerId == XP_MIRROR_ILLEGAL_SESSION)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (forceRemove || (--(*analyzerRefCnt) == 0))
    {
        saiRetVal = xpSaiMirrorSessionPolicerRemove(*analyzerId,
                                                    session->policer);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to Remove mirror session policer: %d\n", saiRetVal);
            return saiRetVal;
        }

        // Destroy mirror session
        if ((xpsRetVal = xpsMirrorRemoveAnalyzerSession(xpsDevId,
                                                        *analyzerId)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to destroy SAI %s mirror session: %u: xpsRetVal %d \n",
                           ingress ? "ingress" : "egress", *analyzerId, xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }

        *analyzerId = XP_MIRROR_ILLEGAL_SESSION;
        *analyzerRefCnt = 0;
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Updates analyzer session
 *
 * \param xpsDevId - device ID
 * \param session - pointer to xpSaiMirrorSession_t instance
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiMirrorSessionAnalyzerUpdate(xpsDevice_t xpsDevId,
                                                     xpSaiMirrorSession_t *session)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsMirrorData_t mirrorData;

    memset(&mirrorData, 0, sizeof(mirrorData));

    if (session->sessionType == SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE)
    {
        mirrorData.erspan2Data.erspanId = session->tnlIntfId;
        mirrorData.erspan2Data.tos = session->tos;
        mirrorData.erspan2Data.greProtoType = session->greProtoType;
        mirrorData.erspan2Data.policerId = (uint32_t)xpSaiObjIdValueGet(
                                               session->policer);
        mirrorData.erspan2Data.truncateSize = session->truncateSize;
        mirrorData.erspan2Data.analyzerIntfId = session->monitorIntf;
        mirrorData.erspan2Data.tc = session->tc;
        mirrorData.erspan2Data.sampleRate = session->sampleRate;
        mirrorData.erspan2Data.ttlHopLimit = session->ttlHopLimit;
        mirrorData.erspan2Data.tos = session->tos;
        COPY_MAC_ADDR_T(&mirrorData.erspan2Data.srcMacAddr, &session->macSA);
        COPY_MAC_ADDR_T(&mirrorData.erspan2Data.dstMacAddr, &session->macDA);

        if (session->ipType == SAI_IP_ADDR_FAMILY_IPV4)
        {
            mirrorData.erspan2Data.ipType = XP_PREFIX_TYPE_IPV4;
        }
        else
        {
            mirrorData.erspan2Data.ipType = XP_PREFIX_TYPE_IPV6;
        }
        xpSaiIpCopy((uint8_t*)&mirrorData.erspan2Data.srcIpAddr,
                    (uint8_t*)&session->srcIpAddr.addr,
                    (sai_ip_addr_family_t)session->ipType);
        xpSaiIpCopy((uint8_t*)&mirrorData.erspan2Data.dstIpAddr,
                    (uint8_t*)&session->dstIpAddr.addr,
                    (sai_ip_addr_family_t)session->ipType);

        if (session->vlanHdrValid)
        {
            mirrorData.erspan2Data.vlanId = session->vlanId;
            mirrorData.erspan2Data.vlanPri = session->vlanPri;
            mirrorData.erspan2Data.vlanCfi = session->vlanCfi;
            mirrorData.erspan2Data.vlanTpid = session->vlanTpid;
        }

    }
    else if (session->sessionType == SAI_MIRROR_SESSION_TYPE_LOCAL ||
             session->sessionType == SAI_MIRROR_SESSION_TYPE_SFLOW)
    {
        mirrorData.spanData.truncateSize = session->truncateSize;
        mirrorData.spanData.analyzerIntfId = session->monitorIntf;
        mirrorData.spanData.sampleRate = session->sampleRate;
        mirrorData.spanData.tc = session->tc;
        mirrorData.spanData.policerId = (uint32_t)xpSaiObjIdValueGet(
                                            session->policer);
    }
    else
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }

    if (session->analyzerId != XP_MIRROR_ILLEGAL_SESSION)
    {
        xpsRetVal = xpsMirrorUpdateAnalyzerSession(xpsDevId, session->analyzerId,
                                                   &mirrorData);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to write ingress ananlyzer session: %u: xpsRetVal %d \n",
                           session->analyzerId, xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiMirrorAclMirrorEnable(sai_object_id_t session_id,
                                        bool ingress, bool enable,
                                        uint32_t* analyzerId, sai_mirror_session_type_t *sessionType)
{
    sai_status_t          saiRetVal   = SAI_STATUS_SUCCESS;
    xpsDevice_t           xpsDevId    = 0;
    xpSaiMirrorSession_t *session     = NULL;
    *analyzerId = XP_MIRROR_ILLEGAL_SESSION;

    if (!XDK_SAI_OBJID_TYPE_CHECK(session_id, SAI_OBJECT_TYPE_MIRROR_SESSION))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(session_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    // Retrieve device Id from session_id
    xpsDevId = xpSaiObjIdSwitchGet(session_id);

    saiRetVal = xpSaiMirrorDbInfoGet(session_id, &session);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to locate session %" PRIu64
                       " in data base, error: %d \n", session_id, saiRetVal);
        return saiRetVal;
    }

    if (enable)
    {
        saiRetVal = xpSaiMirrorSessionAnalyzerCreateOrRef(xpsDevId, session, ingress);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            return saiRetVal;
        }
        *analyzerId = session->analyzerId;
        *sessionType = session->sessionType;
    }
    else
    {
        saiRetVal = xpSaiMirrorSessionAnalyzerRemoveOrUnref(xpsDevId, session, ingress,
                                                            false);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            return saiRetVal;
        }
    }

    return saiRetVal;
}

/*
 * All functionality below was created to support
 * SAI object id retrieve API's
 */

// Func: xpSaiCountMirrorObjects

sai_status_t xpSaiCountMirrorObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, gsaiMirrorDbHdle, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

// Func: xpSaiGetMirrorObjectList

sai_status_t xpSaiGetMirrorObjectList(uint32_t *object_count,
                                      sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;

    xpSaiMirrorSession_t *pMirrorSessionNext = NULL;

    saiRetVal = xpSaiCountMirrorObjects(&objCount);
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
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiMirrorDbHdle,
                                     pMirrorSessionNext, (void **)&pMirrorSessionNext);
        if (retVal != XP_NO_ERR || pMirrorSessionNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve mirror object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_MIRROR_SESSION, xpSaiGetDevId(),
                                     (sai_uint64_t)pMirrorSessionNext->keySessionId,
                                     &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

// Func: xpSaiMaxCountMirrorAttribute

sai_status_t xpSaiMaxCountMirrorAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_MIRROR_SESSION_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiMirrorGetSessionAnalyzerId(sai_object_id_t session_id,
                                             uint32_t *analyzerId)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiMirrorSession_t *session = NULL;
    saiRetVal = xpSaiMirrorDbInfoGet(session_id, &session);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to locate session %" PRIu64
                       " in data base, error: %d \n", session_id, saiRetVal);
        return saiRetVal;
    }
    if (analyzerId)
    {
        *analyzerId = session->analyzerId;
    }
    return saiRetVal;
}

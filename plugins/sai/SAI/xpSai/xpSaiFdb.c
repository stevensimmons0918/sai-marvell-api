// xpSaiFdb.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSai.h"
#include "xpsFdb.h"
#include "xpSaiUtil.h"
#include "xpsAging.h"
#include "xpSaiDev.h"
#include "xpSaiSwitch.h"
#include "xpSaiValidationArrays.h"
#include "xpSaiStub.h"

XP_SAI_LOG_REGISTER_API(SAI_API_FDB);


static sai_fdb_api_t* _xpSaiFdbApi;

xpSaiFdbParams_t gSaiFdbParams;

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \brief Pointer to Aging Handler thread.
 */
pthread_t saiAgingHandlerThread = (pthread_t)NULL;
#ifdef __cplusplus
}
#endif

/**
 * \brief Pointer to Learning Handler thread.
 */
pthread_t gSaiFdbLearningThread;

extern void SetFdbFlushInProgress(bool val);
static pthread_mutex_t gSaiFdbLock = PTHREAD_MUTEX_INITIALIZER;

static sai_status_t xpSaiFlushAllFdbEntriesAll(xpsFdbEntryType_e entry_type);
static sai_status_t xpSaiFlushAllFdbEntriesByPort(xpsInterfaceId_t intfId,
                                                  uint8_t entryType);
static sai_status_t xpSaiFlushAllFdbEntriesByVlan(sai_vlan_id_t vlan_id,
                                                  uint8_t entryType);
static sai_status_t xpSaiFlushAllFdbEntriesByPortVlan(xpsInterfaceId_t intfId,
                                                      sai_vlan_id_t vlan_id,
                                                      uint8_t entryType);

/**
 * \brief Add new static FDB entry
 *
 * \param devId     - device id
 * \param fdbEntry  - actual content of new FDB entry
 * \param indexList - hash index
 *
 * This API is used to add new static FDB entry if max limit is not reached
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiFdbAddEntry(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                  xpsHashIndexList_t *indexList);

extern sai_switch_notification_t switch_notifications_g[XP_MAX_DEVICES];

static inline sai_status_t xpSaiFdbInitLock()
{
    if (pthread_mutex_init(&gSaiFdbLock, NULL) != 0)
    {
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static inline sai_status_t xpSaiFdbDeinitLock()
{
    if (pthread_mutex_destroy(&gSaiFdbLock) != 0)
    {
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static inline void xpSaiFdbLock()
{
    pthread_mutex_lock(&gSaiFdbLock);
}

static inline void xpSaiFdbUnlock()
{
    pthread_mutex_unlock(&gSaiFdbLock);
}

// Func: xpSaiFdbGetXpsEntry

sai_status_t xpSaiFdbGetXpsEntry(sai_fdb_entry_t *entryKey,
                                 xpsFdbEntry_t *xpsFdbEntry)
{
    XP_STATUS   status;
    xpsDevice_t xpsDevId = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    xpSaiMacCopy(xpsFdbEntry->macAddr, entryKey->mac_address);
    xpsFdbEntry->vlanId = (xpsVlan_t)xpSaiObjIdValueGet(entryKey->bv_id);

    /* Retrieve xpsDevId from SAI object Id */
    xpsDevId = xpSaiObjIdSwitchGet(entryKey->bv_id);

    status = xpsFdbGetEntry(xpsDevId, xpsFdbEntry);
    if (status != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(status);
    }

    return SAI_STATUS_SUCCESS;
}

// Func: Notify the interested modules for a change in an FDB entry

static sai_status_t xpSaiFdbNotify(const xpSaiFdbChangeNotif_t *notif)
{
    // Update SAI Neighbor.
    return xpSaiNeighborHandleFdbNotification(notif);
}

/**
 * \brief Validate input sai_attribute value to be comliment with xpsFdbEntryType
 * \param [out] type - pointer to xpsFdbEntryType variable to store value
 * \param [in] attribute - value to parse
 * \return SAI_STATUS_SUCCESS if value correct and was stored in buffer otherwise SAI_STATUS_FAILURE
 */
static sai_status_t xpSaiFdbEntryTypeGet(xpsFdbEntryType_e* type,
                                         const sai_attribute_t attribute)
{
    switch (attribute.value.s32)
    {
        case XP_FDB_ENTRY_TYPE_ALL:
        case XP_FDB_ENTRY_TYPE_STATIC:
        case XP_FDB_ENTRY_TYPE_DYNAMIC:
            *type = (xpsFdbEntryType_e)attribute.value.s32;
            return SAI_STATUS_SUCCESS;
    }

    XP_SAI_LOG_ERR("Invalid Fdb Entry Type %d\n", (int)attribute.value.s32);
    return SAI_STATUS_INVALID_PARAMETER;
}

//Func: xpSaiSetDefaultFdbEntryAttributeVals

void xpSaiSetDefaultFdbEntryAttributeVals(xpSaiFdbEntryAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    attributes->brPortId.oid = SAI_NULL_OBJECT_ID;
    attributes->packetAction.s32 = SAI_PACKET_ACTION_FORWARD;
}

//Func: xpSaiUpdateFdbEntryAttributeVals

void xpSaiUpdateFdbEntryAttributeVals(const uint32_t attr_count,
                                      const sai_attribute_t* attr_list, xpSaiFdbEntryAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);
    uint32_t attrIdx = 0;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID:
                {
                    attributes->brPortId = attr_list[count].value;
                    break;
                }
            case SAI_FDB_ENTRY_ATTR_PACKET_ACTION:
                {
                    attributes->packetAction = attr_list[count].value;
                    break;
                }
            case SAI_FDB_ENTRY_ATTR_TYPE:
                {
                    attributes->type = attr_list[count].value;
                    break;
                }
            case SAI_FDB_ENTRY_ATTR_ENDPOINT_IP:
                {
                    attributes->endPointIp = attr_list[count].value;
                }
            default:
                {
                    XP_SAI_LOG_INFO("Failed to set count = %d for id %d\n", count,
                                    attr_list[count].id);
                }
        }
    }

    /*
     * Default values for absent attributes.
     */
    if (xpSaiFindAttrById(SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID, attr_count, attr_list,
                          &attrIdx) == NULL)
    {
        sai_object_id_t intfOid;
        xpSaiObjIdCreate(SAI_OBJECT_TYPE_BRIDGE_PORT, xpsDevId, XPS_INTF_INVALID_ID,
                         &intfOid);
        attributes->brPortId.oid = intfOid;
    }

}

//Func: xpSaiConvertFdbEntry

sai_status_t xpSaiConvertFdbEntry(const sai_fdb_entry_t *pSai_fdb_entry,
                                  xpsFdbEntry_t* pXpsFdbEntry)
{
    xpsVlan_t vlanId = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    memcpy(pXpsFdbEntry->macAddr, pSai_fdb_entry->mac_address,
           sizeof(pXpsFdbEntry->macAddr));

    if ((!XDK_SAI_OBJID_TYPE_CHECK(pSai_fdb_entry->bv_id, SAI_OBJECT_TYPE_VLAN)) &&
        (!XDK_SAI_OBJID_TYPE_CHECK(pSai_fdb_entry->bv_id, SAI_OBJECT_TYPE_BRIDGE)))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(pSai_fdb_entry->bv_id));
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    vlanId = (xpsVlan_t)xpSaiObjIdValueGet(pSai_fdb_entry->bv_id);

    pXpsFdbEntry->vlanId = vlanId;
    pXpsFdbEntry->serviceInstId = vlanId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBridgeVlanObjIdCreate

sai_status_t xpSaiBridgeVlanObjIdCreate(xpsDevice_t xpsDevId, xpsVlan_t bvId,
                                        sai_object_id_t *bvOid)
{
    sai_status_t      saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS         xpsRetVal = XP_NO_ERR;
    bool              isVlanBd  = false;
    xpsScope_t        scopeId   = xpSaiScopeFromDevGet(xpsDevId);
    sai_object_type_t oidType   = SAI_OBJECT_TYPE_NULL;

    xpsRetVal = xpsVlanIsBd(scopeId, bvId, &isVlanBd);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("VLAN ID: %d validation failed, error: %d\n", bvId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    oidType = (0 == isVlanBd) ? SAI_OBJECT_TYPE_VLAN : SAI_OBJECT_TYPE_BRIDGE;

    saiRetVal = xpSaiObjIdCreate(oidType, xpsDevId, (sai_uint64_t)bvId, bvOid);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, error: %d\n", saiRetVal);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiConvertXpstoSaiFdbEntry

sai_status_t xpSaiConvertXpstoSaiFdbEntry(xpsDevice_t xpsDevId,
                                          const xpsFdbEntry_t *pXpsFdbEntry, sai_fdb_entry_t *pSaiFdbEntry)
{
    sai_status_t    saiRetVal = SAI_STATUS_SUCCESS;
    sai_object_id_t bvOid     = SAI_NULL_OBJECT_ID;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    memcpy(pSaiFdbEntry->mac_address, pXpsFdbEntry->macAddr,
           sizeof(pSaiFdbEntry->mac_address));

    saiRetVal = xpSaiBridgeVlanObjIdCreate(xpsDevId, pXpsFdbEntry->vlanId, &bvOid);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiBridgeVlanObjIdCreate: Failed with retVal : %d \n",
                       saiRetVal);
        return saiRetVal;
    }

    pSaiFdbEntry->bv_id = bvOid;

    return saiRetVal;
}

sai_status_t xpSaiConvertToHwTunnelEntry(xpsFdbEntry_t *fdbEntry)
{
    XP_STATUS   result    = XP_NO_ERR;
    xpsDevice_t devId     = 0;
    xpsVlan_t   hwEVlanId = 0;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    uint32_t    fdbEntryIndex = 0;

    /* check if entry exist - no need to convert to tunnel parameters */
    result = xpsFdbFindEntry(xpsDevId, fdbEntry, &fdbEntryIndex);
    if (result == XP_NO_ERR)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (fdbEntry->vlanId)
    {
        if ((result = xpsVxlanGetTunnelHwEVlanId(devId, fdbEntry->vlanId,
                                                 &hwEVlanId)) == XP_NO_ERR)
        {
            if (!hwEVlanId)
            {
                return xpsStatus2SaiStatus(result);
            }
            fdbEntry->vlanId = hwEVlanId;
        }
    }

    return xpsStatus2SaiStatus(result);
}

//Func: xpSaiSetFdbEntryAttrType

sai_status_t xpSaiSetFdbEntryAttrType(const sai_fdb_entry_t *fdb_entry,
                                      sai_attribute_value_t value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS retVal = XP_NO_ERR;
    xpsFdbEntry_t fdbEntry;
    bool isStatic = false;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    memset(&fdbEntry, 0, sizeof(fdbEntry));

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiConvertFdbEntry(fdb_entry, &fdbEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertFdbEntry failed retVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertToHwTunnelEntry(&fdbEntry);

    isStatic = value.s32 == SAI_FDB_ENTRY_TYPE_STATIC ? true : false;

    retVal = (XP_STATUS) xpsFdbSetAttribute(xpsDevId, &fdbEntry,
                                            XPS_FDB_IS_STATIC_MAC, (void*)&isStatic);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbSetAttribute failed retVal : %d \n", retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiSetFdbEntryAttrBridgePortId

sai_status_t xpSaiSetFdbEntryAttrBridgePortId(const sai_fdb_entry_t *fdb_entry,
                                              sai_attribute_value_t value)
{
    sai_status_t          saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS             xpsRetVal = XP_NO_ERR;
    xpsFdbEntry_t         fdbEntry;
    xpsFdbEntry_t         fdbEntryGet;
    uint32_t              fdbEntryIndex = 0;
    xpsHashIndexList_t    indexList;
    xpsDevice_t           xpsDevId = 0;
    xpsInterfaceId_t      inputIntfId;
    xpSaiFdbChangeNotif_t notif;

    memset(&fdbEntry,    0, sizeof(fdbEntry));
    memset(&fdbEntryGet, 0, sizeof(fdbEntryGet));
    memset(&notif, 0, sizeof(notif));

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Retrieve xpsDevId from SAI object Id */
    xpsDevId = xpSaiObjIdSwitchGet(value.oid);

    inputIntfId = (xpsInterfaceId_t)xpSaiObjIdValueGet(value.oid);

    saiRetVal = xpSaiConvertFdbEntry(fdb_entry, &fdbEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertFdbEntry failed saiRetVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertToHwTunnelEntry(&fdbEntry);

    /*We are doing this additional actions as we want to safe all entry attributes.*/
    xpsRetVal = (XP_STATUS) xpsFdbFindEntry(xpsDevId, &fdbEntry, &fdbEntryIndex);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbFindEntry: Failed to find an entry: error code: %d.\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    memcpy(fdbEntryGet.macAddr, fdbEntry.macAddr, sizeof(fdbEntryGet.macAddr));
    fdbEntryGet.vlanId = fdbEntry.vlanId;

    /*Update an local entry.*/
    xpsRetVal = (XP_STATUS) xpsFdbGetEntry(xpsDevId, &fdbEntryGet);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbGetEntryByIndex: Failed to get an entry: error code: %d.\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* If it is same value, return success */
    if (inputIntfId == fdbEntryGet.intfId)
    {
        return SAI_STATUS_SUCCESS;
    }
    xpsRetVal = (XP_STATUS) xpsFdbRemoveEntry(xpsDevId, &fdbEntry);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbRemoveEntry: Error on removing an entry: error code: %d.\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    /*Copy previous data to new variable*/
    memcpy(&fdbEntry, &fdbEntryGet, sizeof(xpsFdbEntry_t));
    fdbEntry.intfId = inputIntfId;
    XP_SAI_LOG_ERR("DBG: add intf %d, vlan %d to FDB\n", fdbEntry.intfId,
                   fdbEntry.vlanId);
    xpsRetVal = xpSaiFdbAddEntry(xpsDevId, &fdbEntry, &indexList);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiFdbAddEntry: Error in inserting an entry: error code: %d.\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Notify this change to interested */
    notif.entry = fdb_entry;
    notif.notifType = XP_SAI_PROPERTY_CHANGED;

    notif.props.egrIntfId = inputIntfId;
    notif.props.egrIntfValid = true;

    saiRetVal = xpSaiFdbNotify(&notif);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiFdbNotify: Failed to notify: error code: %d.\n",
                       saiRetVal);
    }

    return saiRetVal;
}

//Func: xpSaiSetFdbEntryAttrPacketAction

sai_status_t xpSaiSetFdbEntryAttrPacketAction(const sai_fdb_entry_t *fdb_entry,
                                              sai_attribute_value_t value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS retVal = XP_NO_ERR;
    xpsPktCmd_e xpsPktCmd;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsFdbEntry_t fdbEntry;
    xpSaiFdbChangeNotif_t notif;

    memset(&fdbEntry, 0, sizeof(fdbEntry));
    memset(&notif, 0, sizeof(notif));

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (xpSaiConvertSaiPacketAction2xps((sai_packet_action_t)value.s32,
                                        &xpsPktCmd) != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiConvertFdbEntry(fdb_entry, &fdbEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertFdbEntry failed retVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertToHwTunnelEntry(&fdbEntry);

    retVal = (XP_STATUS) xpsFdbSetAttribute(xpsDevId, &fdbEntry, XPS_FDB_PKT_CMD,
                                            (void*)&xpsPktCmd);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbSetAttribute failed retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /* Notify this change to the interested modules */
    notif.entry = fdb_entry;
    notif.notifType = XP_SAI_PROPERTY_CHANGED;

    notif.props.pktCmd = xpsPktCmd;
    notif.props.pktCmdValid = true;

    saiRetVal = xpSaiFdbNotify(&notif);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiFdbNotify: Failed to notify reVal: %d \n", saiRetVal);
    }

    return saiRetVal;
}

//Func: xpSaiSetFdbEntryAttrEndpointIp

sai_status_t xpSaiSetFdbEntryAttrEndpointIp(const sai_fdb_entry_t *fdb_entry,
                                            sai_attribute_value_t value)
{
    /* TODO: Clarify what to do with ENDPIINT_IP passed to FDB */
#if 0
    sai_status_t     saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS        retVal    = XP_NO_ERR;
    xpsDevice_t      xpsDevId  = 0;
    xpsFdbEntry_t    fdbEntry;
    ipv4Addr_t       remoteIp;

    memset(&fdbEntry, 0, sizeof(fdbEntry));

    xpsDevId = xpSaiObjIdSwitchGet(fdb_entry->switch_id);

    saiRetVal = xpSaiConvertFdbEntry(fdb_entry, &fdbEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertFdbEntry failed retVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertToHwTunnelEntry(&fdbEntry);

    retVal = xpsFdbGetEntry(xpsDevId, &fdbEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbGetEntry failed retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = xpSaiConvertFromHwTunnelEntry(&fdbEntry);

    retVal = xpsVxlanSetTunnelRemoteIp(xpsDevId, fdbEntry.intfId,
                                       value->ipaddr.addr.ip4);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVxlanSetTunnelRemoteIp failed retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    xpSaiIpCopy((uint8_t*)&value->ipaddr.addr.ip4, (uint8_t*)&remoteIp,
                SAI_IP_ADDR_FAMILY_IPV4);
    value->ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
#endif

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetFdbEntryAttrAllowMacMove

sai_status_t xpSaiSetFdbEntryAttrAllowMacMove(const sai_fdb_entry_t *fdb_entry,
                                              sai_attribute_value_t value)
{
    xpSaiHostInterfaceTrapDbEntryT *pTrapEntry = NULL;
    xpSaiHostInterfaceTrapDefaultT *pDefTrapEntry = NULL;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    XP_STATUS retVal = XP_NO_ERR;
    sai_packet_action_t pktCmd;
    sai_object_id_t trapOid;
    xpsPktCmd_e xpsPktCmd;


    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (value.u8 == true)
    {
        /* Get the Cmd from HostIf */

        /* Create Trap object ID */
        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_HOSTIF_TRAP, xpSaiGetDevId(),
                                     (sai_uint64_t)SAI_HOSTIF_TRAP_TYPE_STATIC_FDB_MOVE, &trapOid);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI object could not be created, retVal %d\n", saiRetVal);
            return saiRetVal;
        }

        saiRetVal = xpSaiHostIntfTrapDbInfoGet(trapOid, &pTrapEntry);
        if ((saiRetVal == SAI_STATUS_ITEM_NOT_FOUND) || (pTrapEntry == NULL))
        {
            XP_SAI_LOG_NOTICE("Could not find the trap 0x%08X in DB\n",
                              (uint32_t)xpSaiObjIdValueGet(trapOid));
            pDefTrapEntry = xpSaiHostIntfDefaultTrapGet(
                                SAI_HOSTIF_TRAP_TYPE_STATIC_FDB_MOVE);
            if (pDefTrapEntry == NULL)
            {
                XP_SAI_LOG_ERR("Invalid Host Interface Trap %d\n",
                               SAI_HOSTIF_TRAP_TYPE_STATIC_FDB_MOVE);
                return SAI_STATUS_INVALID_PARAMETER;
            }
            pktCmd = pDefTrapEntry->action;
        }
        else
        {
            pktCmd = pTrapEntry->action;
        }
    }
    else
    {
        pDefTrapEntry = xpSaiHostIntfDefaultTrapGet(
                            SAI_HOSTIF_TRAP_TYPE_STATIC_FDB_MOVE);
        if (pDefTrapEntry == NULL)
        {
            XP_SAI_LOG_ERR("Invalid Host Interface Trap %d\n",
                           SAI_HOSTIF_TRAP_TYPE_STATIC_FDB_MOVE);
            return SAI_STATUS_INVALID_PARAMETER;
        }
        pktCmd = pDefTrapEntry->action;
    }

    saiRetVal = xpSaiConvertSaiPacketAction2xps(pktCmd, &xpsPktCmd);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to convert trap action, error %d\n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpsFdbFdbMgrBrgSecurBreachCommandSet(xpsDevId, xpsPktCmd);

    if (saiRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbSetAttribute failed retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return saiRetVal;
}
//Func: xpSaiConvertFdbAttributes

sai_status_t xpSaiConvertFdbAttributes(xpSaiFdbEntryAttributesT* pAttributes,
                                       xpsFdbEntry_t* pFdbEntry)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    sai_bridge_port_type_t bridgePortType;
    if (!pAttributes)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!pFdbEntry)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (xpSaiConvertSaiPacketAction2xps((sai_packet_action_t)
                                        pAttributes->packetAction.s32, &pFdbEntry->pktCmd) != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((!XDK_SAI_OBJID_TYPE_CHECK(pAttributes->brPortId.oid,
                                   SAI_OBJECT_TYPE_BRIDGE_PORT)) &&
        (!XDK_SAI_OBJID_TYPE_CHECK(pAttributes->brPortId.oid, SAI_OBJECT_TYPE_TUNNEL)))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(pAttributes->brPortId.oid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiGetBridgePortTypeById(pAttributes->brPortId.oid,
                                           &bridgePortType);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for brPortId %llu, error: %d\n",
                       pAttributes->brPortId.oid, saiRetVal);
        return saiRetVal;
    }
    if (bridgePortType == SAI_BRIDGE_PORT_TYPE_TUNNEL)
    {
        xpSaiBridgePort_t dest;
        xpsInterfaceId_t tunnelInterfaceId = 0;

        saiRetVal = xpSaiGetBridgePortById(pAttributes->brPortId.oid, &dest);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiGetBridgePortById: Failed to fetch from XPS DB for bridgePortId: %"
                           PRIu64 " | error: %d.\n", pAttributes->brPortId.oid, saiRetVal)
            return saiRetVal;
        }
        /* Get xpsDevId from tunnel_map_entry_id */
        xpsDevice_t devId = xpSaiObjIdSwitchGet(dest.brPortDescr.objId);

        saiRetVal = xpSaiTunnelGetHwTunnelIntfIdByTunnIdAndRemoteVtepIp(devId,
                                                                        dest.brPortDescr.objId, pAttributes->endPointIp.ipaddr,
                                                                        &tunnelInterfaceId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiXpsTunnelIntfByTunnelObjIdGet: Failed to fetch from XPS DB for tunnelId: %"
                           PRIu64 " | error: %d.\n", pAttributes->brPortId.oid, saiRetVal)
            return saiRetVal;
        }
        pFdbEntry->intfId = tunnelInterfaceId;
        /*set service instance Id*/
        saiRetVal = xpSaiTunnelGetEgressEncapInfoFromTunnelIdandBridgeId(0,
                                                                         dest.brPortDescr.objId, dest.bridgeId, &pFdbEntry->serviceInstId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelGetEgressEncapInfoFromTunnelIdandBridgeId: Failed to get egress Encap for tunnelId: %"
                           PRIu64 " | error: %d.\n", pAttributes->brPortId.oid, saiRetVal)
            return saiRetVal;
        }
    }
    else
    {
        pFdbEntry->intfId = (xpsInterfaceId_t)xpSaiObjIdValueGet(
                                pAttributes->brPortId.oid);
    }
    pFdbEntry->isStatic = (pAttributes->type.s32 == SAI_FDB_ENTRY_TYPE_STATIC) ?
                          true : false;
    XP_SAI_LOG_DBG("FDB entry validation :\n  pFdbEntry->intfId %d\n  pFdbEntry->isStatic %d\n",
                   pFdbEntry->intfId, pFdbEntry->isStatic);

    return SAI_STATUS_SUCCESS;
}

/* FDB aging handler */
void *sai_aging_handler(void *arg)
{
    xpsDevice_t devId = 0;
    uint32_t ageCnt = 1, ageTime = 0;
    XP_STATUS status = XP_NO_ERR;
    XP_AGE_MODE_T ageMode = XP_AGE_MODE_TRIGGER;
    bool done = false, done1 = false;
    xpSaiSwitchThreadsWaitForInit();

    devId = ((xpSaiFdbParams_t *)arg)->xpDevId;
    ageTime = ((xpSaiFdbParams_t *)arg)->ageTime;

    XP_SAI_LOG_DBG("DevId is %u\n\n", devId);
    XP_SAI_LOG_DBG("Initial Aging-Time is %u (seconds)\n\n", ageTime);

    while (!done)
    {

        if (xpsGetExitStatus() != 0)
        {
            done = true;
            break;
        }

        sleep(1);

        xpsGetAgingMode(devId, &ageMode);
        if (ageMode == XP_AGE_MODE_TRIGGER)
        {
            ageTime = ((xpSaiFdbParams_t *)arg)->ageTime;
            if (ageTime)
            {
                if (ageCnt >= ageTime)
                {
                    ageCnt = 1;
                    XP_SAI_LOG_DBG("Call xpsFdbTriggerAging\n");
                    //XP_SYS_MT_EXCLUDE_LOCK_WRITE_PROTECT(devId, XP_LOCKINDEX_FDB_MGR_LOCK);
                    xpsFdbTriggerAging(devId);
                }
                else
                {
                    ageCnt++;
                }
            }
            else
            {
                ageCnt = 1;
            }
        }

        while (!done1)
        {

            if (xpsGetExitStatus() != 0)
            {
                done1 = true;
                break;
            }

            status = xpsAgeFifoHandler(devId);
            if (status == XP_NO_ERR)
            {
                XP_SAI_LOG_DBG("Entry aged-out and removed successfully\n");
            }
            else
            {
                //Uncomment next line for debug purpose
                //XP_SAI_LOG_DBG("xpsAgeFifoHandler failed with %d\n", status);
                break;
            }
        }
    }
    return NULL;
}

//Func: xpSaiCreateFdbEntry

sai_status_t xpSaiHandleCreateFdbEntry(const sai_fdb_entry_t *fdb_entry,
                                       uint32_t attr_count, const sai_attribute_t *attr_list)
{
    xpsHashIndexList_t indexList;
    uint32_t index;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS retVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpSaiFdbEntryAttributesT attributes;
    xpsFdbEntry_t fdbEntry;
    xpSaiFdbChangeNotif_t notif;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;
    xpsFdbEntry_t tmpFdbEntry;
    XP_STATUS tmpRc;

    memset(&notif, 0, sizeof(notif));
    memset(&fdbEntry, 0, sizeof(fdbEntry));
    memset(&tmpFdbEntry, 0, sizeof(tmpFdbEntry));

    if (fdb_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(fdb_entry->switch_id, SAI_OBJECT_TYPE_SWITCH))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(fdb_entry->switch_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               FDB_ENTRY_VALIDATION_ARRAY_SIZE, fdb_entry_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    xpSaiSetDefaultFdbEntryAttributeVals(&attributes);
    xpSaiUpdateFdbEntryAttributeVals(attr_count, attr_list, &attributes);

    saiRetVal = xpSaiConvertFdbEntry(fdb_entry, &fdbEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertFdbEntry failed retVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertFdbAttributes(&attributes, &fdbEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertFdbAttributes failed, retVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    XP_SAI_LOG_DBG("\n\nFDB entry validation :\n  fdbEntry.intfId %d\n  fdbEntry.isStatic %d\n",
                   fdbEntry.intfId, fdbEntry.isStatic);
    XP_SAI_LOG_DBG("  fdbEntry.isControl %d\n  fdbEntry.isRouter %d\n",
                   fdbEntry.isControl, fdbEntry.isRouter);
    XP_SAI_LOG_DBG("  fdbEntry.macAddr %x:%x:%x:%x:%x:%x\n  fdbEntry.vlanId %d\n  fdbEntry.pktCmd %d\n  fdbEntry.serviceInstId %d \n",
                   fdbEntry.macAddr[0], fdbEntry.macAddr[1], fdbEntry.macAddr[2],
                   fdbEntry.macAddr[3], fdbEntry.macAddr[4], fdbEntry.macAddr[5],
                   fdbEntry.vlanId, fdbEntry.pktCmd, fdbEntry.serviceInstId);

    retVal = (XP_STATUS) xpsFdbFindEntry(xpsDevId, &fdbEntry, &index);
    if (retVal == XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbAddEntry: Entry with MAC: " XP_SAI_MAC_ADDR_FMT
                       " and VLAN: %u is already present in the table: error code: %d\n",
                       XP_SAI_XP_MAC_ADDR_ARGS(fdbEntry.macAddr), fdbEntry.vlanId, XP_ERR_KEY_EXISTS);
        tmpRc = xpsFdbGetEntryByIndex(xpsDevId, index, &tmpFdbEntry);

        if (tmpRc != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to Get Fdb entry. Index: %d, RC: %d\n", index, tmpRc);
            return xpsStatus2SaiStatus(XP_ERR_KEY_EXISTS);
        }

        if (tmpFdbEntry.intfId == fdbEntry.intfId)
        {
            return SAI_STATUS_SUCCESS;
        }
        tmpFdbEntry.intfId = fdbEntry.intfId;
        if (tmpFdbEntry.isRouter)
        {
            xpFdbTblEntry_t fdbEntryT;
            memset(&fdbEntryT, 0, sizeof(fdbEntryT));

            memcpy(fdbEntryT.macAddr, fdbEntry.macAddr, sizeof(macAddr_t));
            fdbEntryT.bdId = tmpFdbEntry.vlanId;
            fdbEntryT.pktCmd = XP_PKTCMD_FWD;
            fdbEntryT.routerMAC = 1;
            fdbEntryT.encapType = XP_L2_ENCAP_DOT1Q_UNTAGGED;
            fdbEntryT.isStaticMac = 1;
            fdbEntryT.vif = fdbEntry.intfId;
        }
        else
        {
            retVal = xpsFdbWriteEntry(xpsDevId, index, &tmpFdbEntry);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to Write Fdb entry. Index: %d, RC: %d\n", index, retVal);
                return xpsStatus2SaiStatus(retVal);
            }
        }

        notif.notifType = XP_SAI_PROPERTY_CHANGED;
    }
    else
    {
        retVal = xpSaiFdbAddEntry(xpsDevId, &fdbEntry, &indexList);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiFdbAddEntry: Could not insert new entry: error code %d.\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Notify the creation to the interested */
        notif.notifType = XP_SAI_CREATED;
    }

    saiRetVal = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s: Failed to get Table Entry Count DB , retVal : %d \n",
                       __FUNCNAME__, saiRetVal);
        return saiRetVal;
    }
    entryCountCtxPtr->fdbEntries++;

    notif.entry = fdb_entry;

    notif.props.egrIntfId = fdbEntry.intfId;
    notif.props.egrIntfValid = true;
    notif.props.pktCmd = fdbEntry.pktCmd;
    notif.props.pktCmdValid = true;
    if (fdbEntry.serviceInstId != fdbEntry.vlanId)
    {
        notif.props.serviceInstId = fdbEntry.serviceInstId;
    }
    else
    {
        /*Sevice instanceId is same as vlanId,thus mark it, to identify this case*/
        notif.props.serviceInstId = XPS_INTF_INVALID_ID;
    }
    saiRetVal = xpSaiFdbNotify(&notif);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiCreateFdbEntry(const sai_fdb_entry_t *fdb_entry,
                                 uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t status;

    xpSaiFdbLock();
    status = xpSaiHandleCreateFdbEntry(fdb_entry, attr_count, attr_list);
    xpSaiFdbUnlock();

    return status;
}
//Func: xpSaiRemoveFdbEntry

sai_status_t xpSaiHandleRemoveFdbEntry(const sai_fdb_entry_t *fdb_entry)
{
    XP_STATUS     retVal    = XP_NO_ERR;
    sai_status_t  saiStatus = SAI_STATUS_SUCCESS;
    xpsDevice_t   xpsDevId  = 0;
    xpsFdbEntry_t fdbEntry;
    xpsFdbEntry_t tmpFdbEntry;
    uint32_t index;
    xpSaiFdbChangeNotif_t notif;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    memset(&notif, 0, sizeof(notif));
    memset(&tmpFdbEntry, 0, sizeof(tmpFdbEntry));

    XP_SAI_LOG_DBG("%s \n", __FUNCNAME__);

    if (fdb_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(fdb_entry->switch_id, SAI_OBJECT_TYPE_SWITCH))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(fdb_entry->switch_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memcpy(fdbEntry.macAddr, fdb_entry->mac_address, sizeof(fdbEntry.macAddr));
    fdbEntry.vlanId = (xpsVlan_t)xpSaiObjIdValueGet(fdb_entry->bv_id);
    /* Retrieve xpsDevId from SAI object Id */
    xpsDevId = xpSaiObjIdSwitchGet(fdb_entry->bv_id);

    saiStatus = xpSaiConvertToHwTunnelEntry(&fdbEntry);

    retVal = (XP_STATUS) xpsFdbFindEntry(xpsDevId, &fdbEntry, &index);
    if (retVal == XP_NO_ERR)
    {
        retVal = xpsFdbGetEntryByIndex(xpsDevId, index, &tmpFdbEntry);

        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to Get Fdb entry. Index: %d, RC: %d\n", index, retVal);
            return xpsStatus2SaiStatus(XP_ERR_KEY_EXISTS);
        }
        if (tmpFdbEntry.isRouter)
        {
            return SAI_STATUS_SUCCESS;
        }
    }

    retVal = (XP_STATUS) xpsFdbRemoveEntry(xpsDevId, &fdbEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbRemoveEntry: Error on removing an entry: error code: %d\n",
                       retVal);
    }

    saiStatus = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s: Failed to get Table Entry Count DB , retVal : %d \n",
                       __FUNCNAME__, saiStatus);
        return saiStatus;
    }
    if (entryCountCtxPtr->fdbEntries > 0)
    {
        entryCountCtxPtr->fdbEntries--;
    }
    else
    {
        XP_SAI_LOG_NOTICE("Invalid Fdb-cnt : %d \n", entryCountCtxPtr->fdbEntries);
        XP_SAI_LOG_NOTICE(" mac %x:%x:%x:%x:%x:%x vId %d pCmd %d sId %d \n",
                          fdbEntry.macAddr[0], fdbEntry.macAddr[1], fdbEntry.macAddr[2],
                          fdbEntry.macAddr[3], fdbEntry.macAddr[4], fdbEntry.macAddr[5],
                          fdbEntry.vlanId, fdbEntry.pktCmd, fdbEntry.serviceInstId);
    }
    /* Notify this removal to the interested */
    notif.notifType = XP_SAI_REMOVED;
    notif.entry = fdb_entry;

    saiStatus = xpSaiFdbNotify(&notif);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiRemoveFdbEntry(const sai_fdb_entry_t *fdb_entry)
{
    sai_status_t status;

    xpSaiFdbLock();
    status = xpSaiHandleRemoveFdbEntry(fdb_entry);
    xpSaiFdbUnlock();

    return status;
}

//Func: xpSaiSetFdbEntryAttribute

sai_status_t xpSaiHandleSetFdbEntryAttribute(const sai_fdb_entry_t *fdb_entry,
                                             const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if (fdb_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(fdb_entry->switch_id, SAI_OBJECT_TYPE_SWITCH))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(fdb_entry->switch_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(1, attr,
                            FDB_ENTRY_VALIDATION_ARRAY_SIZE, fdb_entry_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    switch (attr->id)
    {
        case SAI_FDB_ENTRY_ATTR_TYPE:
            {
                retVal = xpSaiSetFdbEntryAttrType(fdb_entry, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_FDB_ENTRY_ATTR_TYPE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID:
            {
                retVal = xpSaiSetFdbEntryAttrBridgePortId(fdb_entry, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID)\n");
                    return retVal;
                }
                break;
            }
        case SAI_FDB_ENTRY_ATTR_PACKET_ACTION:
            {
                retVal = xpSaiSetFdbEntryAttrPacketAction(fdb_entry, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_FDB_ENTRY_ATTR_PACKET_ACTION)\n");
                    return retVal;
                }
                break;
            }
        case SAI_FDB_ENTRY_ATTR_ENDPOINT_IP:
            {
                retVal = xpSaiSetFdbEntryAttrEndpointIp(fdb_entry, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_FDB_ENTRY_ATTR_ENDPOINT_IP)\n");
                    return retVal;
                }
                break;
            }
        case SAI_FDB_ENTRY_ATTR_ALLOW_MAC_MOVE:
            {
                retVal = xpSaiSetFdbEntryAttrAllowMacMove(fdb_entry, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_FDB_ENTRY_ATTR_ALLOW_MAC_MOVE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_FDB_ENTRY_ATTR_USER_TRAP_ID:
        case SAI_FDB_ENTRY_ATTR_META_DATA:
            {
                XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr->id);
                return  SAI_STATUS_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to set %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return retVal;
}

sai_status_t xpSaiSetFdbEntryAttribute(const sai_fdb_entry_t *fdb_entry,
                                       const sai_attribute_t *attr)
{
    sai_status_t status;

    xpSaiFdbLock();
    status = xpSaiHandleSetFdbEntryAttribute(fdb_entry, attr);
    xpSaiFdbUnlock();

    return status;
}

//Func: xpSaiGetFdbEntryAttrType

sai_status_t xpSaiHandleGetFdbEntryAttrType(const sai_fdb_entry_t *fdb_entry,
                                            sai_attribute_value_t* value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS retVal = XP_NO_ERR;
    xpsFdbEntry_t fdbEntry;
    bool isStatic = false;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    memset(&fdbEntry, 0, sizeof(fdbEntry));

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiConvertFdbEntry(fdb_entry, &fdbEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertFdbEntry failed retVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertToHwTunnelEntry(&fdbEntry);

    retVal = (XP_STATUS) xpsFdbGetAttribute(xpsDevId, &fdbEntry,
                                            XPS_FDB_IS_STATIC_MAC, (void*)&isStatic);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbGetAttribute failed retVal : %d \n", retVal);
    }

    value->s32 = (isStatic == true) ? SAI_FDB_ENTRY_TYPE_STATIC :
                 SAI_FDB_ENTRY_TYPE_DYNAMIC;

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetFdbEntryAttrType(const sai_fdb_entry_t *fdb_entry,
                                      sai_attribute_value_t* value)
{
    sai_status_t status;

    xpSaiFdbLock();
    status = xpSaiHandleGetFdbEntryAttrType(fdb_entry, value);
    xpSaiFdbUnlock();

    return status;
}

//Func: xpSaiFdbGetIntfOid

XP_STATUS xpSaiFdbGetIntfOid(xpsDevice_t xpsDevId, sai_object_id_t brId,
                             uint32_t ingressVif, sai_object_id_t *intfOid)
{
    XP_STATUS          status    = XP_NO_ERR;
    sai_status_t       saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceType_e intfType;
    xpsInterfaceId_t   xpsIntfId   = XPS_INTF_INVALID_ID;
    sai_object_id_t    saiTnnlId  = SAI_NULL_OBJECT_ID;
    xpsVlan_t          bdId      = 0;
    uint32_t           localOid  = 0;

    status = xpsInterfaceGetType(ingressVif, &intfType);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Interface Get type failed, vif %d, error %d\n", ingressVif,
                       status);
        return status;
    }

    switch (intfType)
    {
        case XPS_PORT:
        case XPS_LAG:
            {
                saiRetVal = xpSaiBridgePortObjIdCreate(xpsDevId, ingressVif,
                                                       SAI_BRIDGE_PORT_TYPE_PORT, intfOid);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiBridgePortObjIdCreate failed retVal : %d \n", saiRetVal);
                    return XP_ERR_INVALID_PARAMS;
                }
                break;
            }
        case XPS_BRIDGE_PORT:
            {
                saiRetVal = xpSaiBridgePortObjIdCreate(xpsDevId, ingressVif,
                                                       SAI_BRIDGE_PORT_TYPE_SUB_PORT, intfOid);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiObjIdCreate failed retVal : %d \n", saiRetVal);
                    return XP_ERR_INVALID_PARAMS;
                }
                break;
            }
        case XPS_TUNNEL_VXLAN:
            {
                /*Fix for SAI-2732*/
                saiRetVal = xpSaiTunnelGetTunnelId(ingressVif, &saiTnnlId);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("failed to get tunnelBridgePortId from xpsIntf ,retVal : %d \n",
                                   saiRetVal);
                    return XP_ERR_INVALID_PARAMS;
                }

                xpsIntfId = xpSaiObjIdValueGet(saiTnnlId);
                bdId = xpSaiObjIdValueGet(brId);
                /*Tunnel Bridge port is a combination of tunnelId and bridgeId*/
                localOid = (xpsIntfId << (XDK_SAI_OBJID_TUNNEL_BRIDGE_ID_BITS)) | bdId;
                xpsIntfId = localOid;

                saiRetVal = xpSaiBridgePortObjIdCreate(xpsDevId, (uint32_t)xpsIntfId,
                                                       SAI_BRIDGE_PORT_TYPE_TUNNEL, intfOid);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiObjIdCreate failed retVal : %d \n", saiRetVal);
                    return XP_ERR_INVALID_PARAMS;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("%s Invalid interface type : %d \n", __FUNCNAME__, intfType);
                return XP_ERR_INVALID_PARAMS;
            }
    }

    return XP_NO_ERR;
}

//Func: xpSaiGetFdbEntryAttrBridgePortId

sai_status_t xpSaiGetFdbEntryAttrBridgePortId(const sai_fdb_entry_t *fdb_entry,
                                              sai_attribute_value_t* value)
{
    sai_status_t  saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS     retVal    = XP_NO_ERR;
    xpsDevice_t   xpsDevId  = 0;
    xpsFdbEntry_t fdbEntry;

    memset(&fdbEntry, 0, sizeof(fdbEntry));

    xpsDevId = xpSaiObjIdSwitchGet(fdb_entry->switch_id);

    saiRetVal = xpSaiConvertFdbEntry(fdb_entry, &fdbEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertFdbEntry failed retVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertToHwTunnelEntry(&fdbEntry);

    retVal = xpsFdbGetEntry(xpsDevId, &fdbEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbFindEntry failed retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiFdbGetIntfOid(xpsDevId, fdb_entry->bv_id, fdbEntry.intfId,
                                &value->oid);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiFdbGetIntfOid failed retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetFdbEntryAttrPacketAction

sai_status_t xpSaiGetFdbEntryAttrPacketAction(const sai_fdb_entry_t *fdb_entry,
                                              sai_attribute_value_t* value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS retVal = XP_NO_ERR;
    xpPktCmd_e xpPktCmd = XP_PKTCMD_MAX;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsFdbEntry_t fdbEntry;

    memset(&fdbEntry, 0, sizeof(fdbEntry));

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiConvertFdbEntry(fdb_entry, &fdbEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertFdbEntry failed retVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertToHwTunnelEntry(&fdbEntry);

    retVal = (XP_STATUS) xpsFdbGetAttribute(xpsDevId, &fdbEntry, XPS_FDB_PKT_CMD,
                                            (void*)&xpPktCmd);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbGetAttribute failed retVal : %d \n", retVal);
    }

    saiRetVal = xpSaiConvertXpsPacketAction2Sai(xpPktCmd,
                                                (sai_packet_action_t*)&value->s32);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertXpsPacketAction2Sai failed with error %d\n",
                       saiRetVal);
        // In xpSaiGetFdbEntryAttrPacketAction(), if we fail to get xpPktCmd based on given fdbEntry
        // that means we are assuming that the given fdbEntry doesn't exist and return appropriate
        // error to caller.
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiGetFdbEntryAttrEndpointIp

sai_status_t xpSaiGetFdbEntryAttrEndpointIp(const sai_fdb_entry_t *fdb_entry,
                                            sai_attribute_value_t* value)
{
    sai_status_t     saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS        retVal    = XP_NO_ERR;
    xpsDevice_t      xpsDevId  = 0;
    xpsFdbEntry_t    fdbEntry;
    ipv4Addr_t       remoteIp;
    xpsInterfaceType_e intfType;

    memset(&fdbEntry, 0, sizeof(fdbEntry));
    memset(&remoteIp, 0, sizeof(remoteIp));

    xpsDevId = xpSaiObjIdSwitchGet(fdb_entry->switch_id);

    saiRetVal = xpSaiConvertFdbEntry(fdb_entry, &fdbEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertFdbEntry failed retVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertToHwTunnelEntry(&fdbEntry);

    retVal = xpsFdbGetEntry(xpsDevId, &fdbEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbGetEntry failed retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsInterfaceGetType(fdbEntry.intfId, &intfType);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Interface Get type failed, vif %d, error %d\n", fdbEntry.intfId,
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (intfType == XPS_TUNNEL_VXLAN)
    {
        retVal = xpsVxlanGetTunnelRemoteIp(xpsDevId, fdbEntry.intfId, &remoteIp);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsVxlanGetTunnelRemoteIp failed retVal : %d \n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    xpSaiIpCopy((uint8_t*)&value->ipaddr.addr.ip4, (uint8_t*)&remoteIp,
                SAI_IP_ADDR_FAMILY_IPV4);
    value->ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetFdbEntryAttrAllowMacMove

sai_status_t xpSaiGetFdbEntryAttrAllowMacMove(const sai_fdb_entry_t *fdb_entry,
                                              sai_attribute_value_t* value)
{
    XP_STATUS        retVal    = XP_NO_ERR;
    xpsDevice_t      xpsDevId  = xpSaiGetDevId();
    xpsPktCmd_e      xpsPktCmd;

    retVal = xpsFdbFdbMgrBrgSecurBreachCommandGet(xpsDevId, &xpsPktCmd);

    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbSetAttribute failed retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (xpsPktCmd == XP_PKTCMD_DROP)
    {
        value->u8 = false;
    }
    else if (xpsPktCmd == XP_PKTCMD_FWD ||
             xpsPktCmd == XP_PKTCMD_FWD_MIRROR || xpsPktCmd == XP_PKTCMD_TRAP)
    {
        value->u8 = true;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetFdbEntryAttribute

static sai_status_t xpSaiGetFdbEntryAttribute(const sai_fdb_entry_t *fdb_entry,
                                              sai_attribute_t *attr, uint32_t attr_index)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    switch (attr->id)
    {
        case SAI_FDB_ENTRY_ATTR_TYPE:
            {
                retVal = xpSaiGetFdbEntryAttrType(fdb_entry, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_FDB_ENTRY_ATTR_TYPE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID:
            {
                retVal = xpSaiGetFdbEntryAttrBridgePortId(fdb_entry, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID)\n");
                    return retVal;
                }
                break;
            }
        case SAI_FDB_ENTRY_ATTR_PACKET_ACTION:
            {
                retVal = xpSaiGetFdbEntryAttrPacketAction(fdb_entry, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_FDB_ENTRY_ATTR_PACKET_ACTION)\n");
                    return retVal;
                }
                break;
            }
        case SAI_FDB_ENTRY_ATTR_ENDPOINT_IP:
            {
                retVal = xpSaiGetFdbEntryAttrEndpointIp(fdb_entry, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_FDB_ENTRY_ATTR_ENDPOINT_IP)\n");
                    return retVal;
                }
                break;
            }
        case SAI_FDB_ENTRY_ATTR_USER_TRAP_ID:
        case SAI_FDB_ENTRY_ATTR_META_DATA:
            {
                XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr->id);
                return  SAI_STATUS_NOT_SUPPORTED;
            }
        case SAI_FDB_ENTRY_ATTR_ALLOW_MAC_MOVE:
            {
                retVal = xpSaiGetFdbEntryAttrAllowMacMove(fdb_entry, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_FDB_ENTRY_ATTR_ALLOW_MAC_MOVE)\n");
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
    return retVal;
}


//Func: xpSaiGetFdbEntryAttributes

static sai_status_t xpSaiGetFdbEntryAttributes(const sai_fdb_entry_t *fdb_entry,
                                               uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (fdb_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(fdb_entry->switch_id, SAI_OBJECT_TYPE_SWITCH))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(fdb_entry->switch_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            FDB_ENTRY_VALIDATION_ARRAY_SIZE, fdb_entry_attribs,
                            SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        retVal = xpSaiGetFdbEntryAttribute(fdb_entry, &attr_list[count], count);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetFdbEntryAttribute failed\n");
            return retVal;
        }
    }

    return retVal;
}

//Func: xpSaiBulkGetFdbEntryAttributes

sai_status_t xpSaiBulkGetFdbEntryAttributes(const sai_fdb_entry_t *fdb_entry,
                                            uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if ((fdb_entry == NULL) || (attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountFdbAttribute(&maxcount);
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
        attr_list[idx].id = SAI_FDB_ENTRY_ATTR_START + count;
        saiRetVal = xpSaiGetFdbEntryAttribute(fdb_entry, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRaiseFdbEvent

static void xpSaiRaiseFdbEvent(xpsDevice_t devId, xpsFdbEntry_t fdbEntry,
                               sai_fdb_event_t event_type)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("called with event_type = %d vlan %d\n", event_type,
                   fdbEntry.vlanId);

    if (devId >= XP_MAX_DEVICES)
    {
        XP_SAI_LOG_ERR("Invalid device id %d\n", devId);
        return;
    }

    //fdb notify is valid only for std 12 bit vlan
    if (fdbEntry.vlanId >= XPS_L3_RESERVED_ROUTER_VLAN(devId))
    {
        XP_SAI_LOG_DBG("not a valid vlan\n");
        return;
    }

    if (switch_notifications_g[devId].on_fdb_event != NULL)
    {
        sai_object_id_t switchObjId = 0;
        sai_fdb_event_notification_data_t data;
        sai_attribute_t fdb_attribute[3];
        sai_object_id_t intfOid = SAI_NULL_OBJECT_ID;
        sai_object_id_t bvOid = SAI_NULL_OBJECT_ID;
        sai_packet_action_t saiAction;

        memset(&data, 0x0, sizeof(sai_fdb_event_notification_data_t));

        //Fill up required fields as data before sending to user
        if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_SWITCH, devId, 0,
                             &switchObjId) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI switch object could not be created.\n");
            return; //Equivalent of SAI_STATUS_INVALID_PARAMETER
        }
        data.fdb_entry.switch_id = switchObjId;
        data.event_type = event_type;

        if (fdbEntry.vlanId > 0)
        {
            saiStatus = xpSaiBridgeVlanObjIdCreate(devId, fdbEntry.vlanId, &bvOid);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiBridgeVlanObjIdCreate: Failed with retVal : %d \n",
                               saiStatus);
                return;
            }
        }

        data.fdb_entry.bv_id = bvOid;
        xpSaiMacCopy(data.fdb_entry.mac_address, fdbEntry.macAddr);

        XP_SAI_LOG_DBG("Preparing FDB notification! event_type:%d, bv_id:%d\n",
                       data.event_type, data.fdb_entry.bv_id);

        data.attr_count = 3;

        //Fdb entry type
        fdb_attribute[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
        fdb_attribute[0].value.s32 = (fdbEntry.isStatic == true) ?
                                     SAI_FDB_ENTRY_TYPE_STATIC : SAI_FDB_ENTRY_TYPE_DYNAMIC;


        if (fdbEntry.intfId != XPS_INTF_INVALID_ID)
        {
            //Bridge Port object ID
            xpStatus = xpSaiFdbGetIntfOid(devId, data.fdb_entry.bv_id, fdbEntry.intfId,
                                          &intfOid);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("xpSaiFdbGetIntfOid failed with error %d\n", xpStatus);
                return;
            }
        }

        fdb_attribute[1].id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID;
        fdb_attribute[1].value.oid = intfOid;

        //Packet action
        fdb_attribute[2].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION;
        saiStatus = xpSaiConvertXpsPacketAction2Sai(fdbEntry.pktCmd, &saiAction);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiConvertXpsPacketAction2Sai failed with error %d\n",
                           saiStatus);
            return;
        }

        fdb_attribute[2].value.s32 = saiAction;

        data.attr = fdb_attribute;

        //Call notification function registered by SAI application

        switch_notifications_g[devId].on_fdb_event(1, &data);
    }

    return;
}

//Func: xpSaiFlushAllFdbEntries

sai_status_t xpSaiHandleFlushFdbEntries(_In_ sai_object_id_t switch_id,
                                        uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t      retVal      = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t  intfId      = XPS_INTF_INVALID_ID;
    sai_object_id_t   brPortOid   = 0;
    sai_object_id_t   bvOid       = 0;
    sai_vlan_id_t     vlanId      = 0;

    xpsFdbEntryType_e entryType   = XP_FDB_ENTRY_TYPE_DYNAMIC;
    xpsDevice_t       xpsDevId    = 0;
    xpsFdbEntry_t     xpsFdbEntry;
    sai_fdb_entry_t   saiFdbEntry;
    xpSaiFdbChangeNotif_t notif;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;
    uint32_t fdbEntriesCount                      = 0;
    XP_STATUS xpStatus                            = XP_NO_ERR;
    sai_bridge_port_type_t bridgePortType;
    xpSaiBridgePort_t      bridgePort;
    sai_object_id_t        saiPortId = SAI_NULL_OBJECT_ID;
    xpsInterfaceId_t       xpsIntf = 0;

    XP_SAI_LOG_DBG("Calling xpSaiFlushAllFdbEntries\n");
    /* Set a flag to drop all learn events while Flsuh in-progress. */
    SetFdbFlushInProgress(true);

    //XP_SYS_MT_EXCLUDE_LOCK_WRITE_PROTECT(0u, XP_LOCKINDEX_FDB_MGR_LOCK);
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_FDB_FLUSH_ATTR_BRIDGE_PORT_ID:
                {
                    brPortOid = attr_list[count].value.oid;
#ifdef ASIC_SIMULATION
                    if (brPortOid == 0)
                    {
                        continue;
                    }
#endif

                    if ((!XDK_SAI_OBJID_TYPE_CHECK(brPortOid, SAI_OBJECT_TYPE_BRIDGE_PORT)) &&
                        (!XDK_SAI_OBJID_TYPE_CHECK(brPortOid, SAI_OBJECT_TYPE_TUNNEL)))
                    {
                        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                                       xpSaiObjIdTypeGet(brPortOid));
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }

                    retVal = xpSaiGetBridgePortTypeById(brPortOid, &bridgePortType);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for brPortId %llu, error: %d\n",
                                       brPortOid, retVal);
                        return retVal;
                    }
                    if (bridgePortType == SAI_BRIDGE_PORT_TYPE_TUNNEL)
                    {
                        retVal = xpSaiGetBridgePortById(brPortOid, &bridgePort);
                        if (retVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("xpSaiGetBridgePortById: Failed to fetch from XPS DB for bridgePortId: %d, error: %d.\n",
                                           brPortOid, retVal)
                            return retVal;
                        }
                        saiPortId = bridgePort.brPortDescr.objId;
                        xpsDevId = xpSaiObjIdSwitchGet(saiPortId);
                        xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(saiPortId);

                        if (XDK_SAI_OBJID_TYPE_CHECK(saiPortId, SAI_OBJECT_TYPE_TUNNEL))
                        {
                            retVal = xpSaiTunnelGetTunnelIntf(xpsDevId, saiPortId, &xpsIntf);
                            if (retVal != SAI_STATUS_SUCCESS)
                            {
                                XP_SAI_LOG_ERR("xpSaiTunnelGetTunnelId retVal : %d \n",
                                               retVal);
                                return retVal;
                            }

                            intfId = xpsIntf;
                        }
                    }
                    else
                    {
                        intfId = (xpsInterfaceId_t)xpSaiObjIdValueGet(brPortOid);
                    }
                    break;
                }
            case SAI_FDB_FLUSH_ATTR_BV_ID:
                {
                    bvOid = attr_list[count].value.oid;
#ifdef ASIC_SIMULATION
                    if (bvOid == 0)
                    {
                        continue;
                    }
#endif
                    if ((!XDK_SAI_OBJID_TYPE_CHECK(bvOid, SAI_OBJECT_TYPE_VLAN)) &&
                        (!XDK_SAI_OBJID_TYPE_CHECK(bvOid, SAI_OBJECT_TYPE_BRIDGE)))
                    {
                        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(bvOid));
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }

                    vlanId = (sai_vlan_id_t)xpSaiObjIdValueGet(bvOid);

                    break;
                }
            case SAI_FDB_FLUSH_ATTR_ENTRY_TYPE:
                {
                    retVal = xpSaiFdbEntryTypeGet(&entryType, attr_list[count]);
                    if (retVal)
                    {
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Unknown attribute %d\n", attr_list[count].id);
                    return SAI_STATUS_INVALID_ATTRIBUTE_0;
                }
        }
    }

    if ((intfId != XPS_INTF_INVALID_ID) && (vlanId > 0))
    {
        retVal = xpSaiFlushAllFdbEntriesByPortVlan(intfId, vlanId,
                                                   entryType);
    }
    else if (intfId != XPS_INTF_INVALID_ID)
    {
        retVal = xpSaiFlushAllFdbEntriesByPort(intfId, entryType);
    }
    else if (vlanId > 0)
    {
        retVal = xpSaiFlushAllFdbEntriesByVlan(vlanId, entryType);
    }
    else
    {
        retVal = xpSaiFlushAllFdbEntriesAll(entryType);
    }

    XP_SAI_LOG_DBG("flushing FDB entries, vlan = %d interface = %d \n", intfId,
                   vlanId);

    xpStatus = xpsFdbGetEntriesCount(xpsDevId, &fdbEntriesCount);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get number of FDB entries\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    retVal = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB\n");
        return retVal;
    }
    entryCountCtxPtr->fdbEntries = fdbEntriesCount;

    /*Consolidated Flush event notification*/

    memset(&xpsFdbEntry, 0, sizeof(xpsFdbEntry));
    xpsFdbEntry.intfId = intfId;
    xpsFdbEntry.vlanId = vlanId;

    /* Convert xpsFdbEntry to SAI entry */
    memset(&saiFdbEntry, 0, sizeof(saiFdbEntry));
    xpSaiConvertXpstoSaiFdbEntry(xpsDevId, &xpsFdbEntry, &saiFdbEntry);

    /* Notify this removal to the interested */
    memset(&notif, 0, sizeof(notif));
    notif.notifType = XP_SAI_REMOVED;
    notif.entry = &saiFdbEntry;

    retVal = xpSaiFdbNotify(&notif);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("FDB notification for FDB entry failed\n");
    }

    xpSaiRaiseFdbEvent(xpsDevId, xpsFdbEntry, SAI_FDB_EVENT_FLUSHED);

    /* Reset a flag to drop all learn events while Flsuh in-progress. */
    SetFdbFlushInProgress(false);
    return retVal;
}

sai_status_t xpSaiFlushFdbEntries(_In_ sai_object_id_t switch_id,
                                  uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t status;

    status = xpSaiHandleFlushFdbEntries(switch_id, attr_count, attr_list);

    return status;
}

//Func: xpSaiFlushAllFdbEntriesAll

static sai_status_t xpSaiFlushAllFdbEntriesAll(xpsFdbEntryType_e entry_type)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    retVal = (XP_STATUS) xpsFdbFlushEntry(xpsDevId, entry_type);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbFlushEntry failed, retVal : %d \n", retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}


//Func: xpSaiFlushAllFdbEntriesByPort

static sai_status_t xpSaiFlushAllFdbEntriesByPort(xpsInterfaceId_t intfId,
                                                  uint8_t entryType)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("intfId %u \n", intfId);
    retVal = (XP_STATUS) xpsFdbFlushEntryByIntf(xpsDevId, intfId,
                                                (xpsFdbEntryType_e)entryType);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbFlushEntryByIntf failed intfId %u | entryType %hhd | retVal : %d \n",
                       intfId, entryType, retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}


//Func: xpSaiFlushAllFdbEntriesByVlan

static sai_status_t xpSaiFlushAllFdbEntriesByVlan(sai_vlan_id_t vlan_id,
                                                  uint8_t entryType)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("%s: vlan_id %d \n", __FUNCNAME__, vlan_id);

    retVal = (XP_STATUS) xpsFdbFlushEntryByVlan(xpsDevId, vlan_id,
                                                (xpsFdbEntryType_e)entryType);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbFlushEntryByVlan failed vlan_id %d | entryType %hhd | retVal : %d \n",
                       vlan_id, entryType, retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}


//Func: xpSaiFlushAllFdbEntriesByPortVlan
static sai_status_t xpSaiFlushAllFdbEntriesByPortVlan(xpsInterfaceId_t intfId,
                                                      sai_vlan_id_t vlan_id,
                                                      uint8_t entryType)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    retVal = (XP_STATUS) xpsFdbFlushEntryByIntfVlan(xpsDevId, intfId,
                                                    vlan_id,
                                                    (xpsFdbEntryType_e)entryType);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiFlushAllFdbEntriesByPortVlan failed, intfId %d | vlan_id %d | entryType %hhd | retVal : %d \n",
                       intfId, vlan_id, entryType, retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}


//Func: xpSaiFdbAgingEventNotification

/*
 *                     Hash table                                                         Aging table
 *      ---------------------------------------                               -------------------------------------
 *      | Key (SRC_MAC-VLAN) | MAC_HIT | Data |  SRC_MAC-VLAN -> entry index  | Entry index  | Aging enable (0/1) |
 *      |-------------------------------------|                               |-----------------------------------|
 *      |  SRC_MAC1-VLAN1    |  (0/1)  | Data |                               | entry_index1 |       (0/1)        |
 *      |-------------------------------------|                               |-----------------------------------|
 *      |                . . .                |                               |                . . .              |
 *      |-------------------------------------|                               |-----------------------------------|
 *      |  SRC_MACn-VLANn    |  (0/1)  | Data |                               | entry_indexn |       (0/1)        |
 *      ---------------------------------------                               -------------------------------------
 *
 * When FDB entry is added to FDB table index is calculated, based on SRC_MAC-VLAN.
 * The calculated index is used in Aging table to enable aging for FDB entry.
 * The FDB entry can be removed either by:
 *     1. A request to remove given FDB entry is received:
 *        No issues neither with sync in shadow nor with aging enbale is issued.
 *        The shadow is synced and aging disbale is called by FDB entry remove call.
 *     2. No packets received with SRC_MAC-VLAN on a port/LAG and age time expires:
 *        The FDB aging event notification is called and entry index is passed.
 *        The entry is removed with method described in #1.
 *     3. Bulk flush of FDB entries based on port/VLAN is performed:
 *        The entry is removed from HW by bulk flush call, but aging flag is not reset.
 *        This leads to situation that aging event is generated for index that points to invalid (removed) entry in hash table.
 *        In this case we need to reset aging enable flag manually from SW.
 * Examples:
 *     a) No new FDB entry is added:
 *      ---------------------------------------                               -------------------------------------
 *      | Key (SRC_MAC-VLAN) | MAC_HIT | Data |  SRC_MAC-VLAN -> entry index  | Entry index  | Aging enable (0/1) |
 *      |-------------------------------------|                               |-----------------------------------|
 *      |   SRC_MAC1-VLAN1   |    1    | Data |                               | entry_index1 |          1         |
 *      ---------------------------------------                               -------------------------------------
 *     b) New FDB entry is added:
 *      ---------------------------------------                               -------------------------------------
 *      | Key (SRC_MAC-VLAN) | MAC_HIT | Data |  SRC_MAC-VLAN -> entry index  | Entry index  | Aging enable (0/1) |
 *      |-------------------------------------|                               |-----------------------------------|
 *      |   SRC_MAC1-VLAN1   |    0    | Data |                               | entry_index1 |          0         |
 *      |-------------------------------------|                               |-----------------------------------|
 *      |   SRC_MAC1-VLAN2   |    1    | Data |                               | entry_index2 |          1         |
 *      ---------------------------------------                               -------------------------------------
 *     c) Dynamic FDB entry is flushed by bulk flush call:
 *      ---------------------------------------                               -------------------------------------
 *      | Key (SRC_MAC-VLAN) | MAC_HIT | Data |  SRC_MAC-VLAN -> entry index  | Entry index  | Aging enable (0/1) |
 *      |-------------------------------------|                               |-----------------------------------|
 *      |             Entry flushed           |                               | entry_index2 |          1         |
 *      ---------------------------------------                               -------------------------------------
 */

void xpSaiHandleFdbAgingEventNotification(xpDevice_t devId,
                                          xpsFdbEntry_t fdbEntry)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    sai_fdb_entry_t saiFdbEntry;
    xpSaiFdbChangeNotif_t notif;

    XP_SAI_LOG_DBG("xpSaiFdbLearnEventNotification called!\n\n");
    memset(&saiFdbEntry, 0, sizeof(saiFdbEntry));
    xpSaiConvertXpstoSaiFdbEntry(devId, &fdbEntry, &saiFdbEntry);
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    /* Notify this removal to the interested */
    memset(&notif, 0, sizeof(notif));
    notif.notifType = XP_SAI_REMOVED;
    notif.entry = &saiFdbEntry;

    saiStatus = xpSaiFdbNotify(&notif);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Notification for aged FDB entry at index failed\n");
        return;
    }

    saiStatus = xpSaiGetTableEntryCountCtxDb(devId, &entryCountCtxPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s: Failed to get Table Entry Count DB , retVal : %d \n",
                       __FUNCNAME__, saiStatus);
        return;
    }
    if (entryCountCtxPtr->fdbEntries > 0)
    {
        entryCountCtxPtr->fdbEntries--;
    }
    else
    {
        XP_SAI_LOG_NOTICE("Invalid Fdb-cnt : %d \n", entryCountCtxPtr->fdbEntries);
        XP_SAI_LOG_NOTICE("  mac %x:%x:%x:%x:%x:%x vId %d pCmd %d sId %d \n",
                          fdbEntry.macAddr[0], fdbEntry.macAddr[1], fdbEntry.macAddr[2],
                          fdbEntry.macAddr[3], fdbEntry.macAddr[4], fdbEntry.macAddr[5],
                          fdbEntry.vlanId, fdbEntry.pktCmd, fdbEntry.serviceInstId);
    }

    xpSaiRaiseFdbEvent(devId, fdbEntry, SAI_FDB_EVENT_AGED);

    return;
}

void xpSaiFdbAgingEventNotification(xpDevice_t devId, xpsFdbEntry_t fdbEntry)
{
    xpSaiHandleFdbAgingEventNotification(devId, fdbEntry);
}

//Func: xpSaiFdbLearnEventNotification

XP_STATUS xpSaiHandleFdbLearnEventNotification(xpDevice_t devId,
                                               xpsFdbEntry_t fdbEntry, xpsFdbEntryAction_e fdbAction)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    //XP_SYS_MT_EXCLUDE_LOCK_WRITE_PROTECT(devId, XP_LOCKINDEX_FDB_MGR_LOCK);
    /* Prior to learn new FDB entry ensure that it will not exceed
     * allowed FDB entries limit */
    xpStatus = xpSaiFdbLimitValidate(devId);
    if (xpStatus != XP_NO_ERR)
    {
        /* Return XP_NO_ERR not to force error logging by XDK */
        return XP_NO_ERR;
    }
    if (fdbAction == XP_FDB_ENTRY_NO_ACTION)
    {
        // Nothing was updated on HW so just return.
        return XP_NO_ERR;
    }

    // Get SAI FDB Entry.
    sai_object_id_t switchObjId = SAI_NULL_OBJECT_ID;
    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_SWITCH, devId, 0, &switchObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiObjIdCreate: Failed with status=%i.\n", saiStatus);
        return XP_ERR_INVALID_PARAMS; // Equivalent of SAI_STATUS_INVALID_PARAMETER.
    }

    sai_fdb_entry_t saiFdbEntry;
    saiFdbEntry.switch_id = switchObjId;
    sai_object_id_t bvObjId = SAI_NULL_OBJECT_ID;

    xpSaiMacCopy(saiFdbEntry.mac_address, fdbEntry.macAddr);

    saiStatus = xpSaiBridgeVlanObjIdCreate(devId, fdbEntry.vlanId, &bvObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiBridgeVlanObjIdCreate: Failed with retVal : %d \n",
                       saiStatus);
        return XP_ERR_INVALID_DATA;
    }

    saiFdbEntry.bv_id = bvObjId;

    // Notify the creation to the interested.
    xpSaiFdbChangeNotif_t notif;
    notif.notifType = XP_SAI_CREATED;
    notif.entry = &saiFdbEntry;

    notif.props.egrIntfId = fdbEntry.intfId;
    notif.props.egrIntfValid = true;
    notif.props.pktCmd = fdbEntry.pktCmd;
    notif.props.pktCmdValid = true;

    saiStatus = xpSaiFdbNotify(&notif);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiFdbNotify: Failed with status=%i.\n", saiStatus);
    }

    fdbEntry.isStatic = false;

    saiStatus = xpSaiGetTableEntryCountCtxDb(devId, &entryCountCtxPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s: Failed to get Table Entry Count DB , retVal : %d \n",
                       __FUNCNAME__, saiStatus);
        return XP_ERR_KEY_NOT_FOUND;
    }

    if (fdbAction == XP_FDB_ENTRY_ADD)
    {
        entryCountCtxPtr->fdbEntries++;
    }

    xpSaiRaiseFdbEvent(devId, fdbEntry,
                       (fdbAction == XP_FDB_ENTRY_ADD) ? SAI_FDB_EVENT_LEARNED : SAI_FDB_EVENT_MOVE);

    return XP_NO_ERR;
}

XP_STATUS xpSaiFdbLearnEventNotification(xpsDevice_t devId,
                                         xpsFdbEntry_t fdbEntry, xpsFdbEntryAction_e fdbAction)
{
    XP_STATUS status;
    xpSaiFdbLock();
    status = xpSaiHandleFdbLearnEventNotification(devId, fdbEntry, fdbAction);
    xpSaiFdbUnlock();

    return status;
}

//Func: xpSaiFdbApiInit

XP_STATUS xpSaiFdbApiInit(uint64_t flag,
                          const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiFdbApiInit\n");

    saiRetVal = xpSaiFdbInitLock();
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to initialize the SAI FDB lock\n");
        return XP_ERR_RESOURCE_NOT_AVAILABLE;
    }

    _xpSaiFdbApi = (sai_fdb_api_t *) xpMalloc(sizeof(sai_fdb_api_t));
    if (NULL == _xpSaiFdbApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiFdbApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiFdbApi->create_fdb_entry = xpSaiCreateFdbEntry;
    _xpSaiFdbApi->remove_fdb_entry = xpSaiRemoveFdbEntry;
    _xpSaiFdbApi->set_fdb_entry_attribute = xpSaiSetFdbEntryAttribute;
    _xpSaiFdbApi->get_fdb_entry_attribute = xpSaiGetFdbEntryAttributes;
    _xpSaiFdbApi->flush_fdb_entries = xpSaiFlushFdbEntries;

    _xpSaiFdbApi->create_fdb_entries = (sai_bulk_create_fdb_entry_fn)
                                       xpSaiStubGenericApi;
    _xpSaiFdbApi->remove_fdb_entries = (sai_bulk_remove_fdb_entry_fn)
                                       xpSaiStubGenericApi;

    _xpSaiFdbApi->set_fdb_entries_attribute = (sai_bulk_set_fdb_entry_attribute_fn)
                                              xpSaiStubGenericApi;
    _xpSaiFdbApi->get_fdb_entries_attribute = (sai_bulk_get_fdb_entry_attribute_fn)
                                              xpSaiStubGenericApi;

    saiRetVal = xpSaiApiRegister(SAI_API_FDB, (void*)_xpSaiFdbApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register FDB API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}


//Func: _xpSaiFdbApiDeinit

XP_STATUS xpSaiFdbApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiFdbApiDeinit\n");

    xpFree(_xpSaiFdbApi);
    _xpSaiFdbApi = NULL;

    saiStatus = xpSaiFdbDeinitLock();
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to deinitialize the SAI FDB lock\n");
        return XP_ERR_RESOURCE_NOT_AVAILABLE;
    }

    return  retVal;
}

sai_status_t xpSaiMaxCountFdbAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_FDB_ENTRY_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountFdbObjects(uint32_t *count)
{
    XP_STATUS   retVal   = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsCountFdbObjects(xpSaiGetDevId(), count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetFdbObjectList(uint32_t *object_count,
                                   sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    sai_fdb_entry_t SfdbEntry, *pfdbEntry;
    xpsFdbEntry_t   fdbEntry;
    uint32_t        tsize  = 0;
    xpsDevice_t     xpsDevId = xpSaiGetDevId();

    saiRetVal = xpSaiCountFdbObjects(&objCount);
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

    retVal = xpsFdbGetTableSize(xpsDevId, &tsize);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get table size!\n");
        return xpsStatus2SaiStatus(retVal);
    }

    pfdbEntry = (sai_fdb_entry_t *)xpMalloc(sizeof(sai_fdb_entry_t) *
                                            (*object_count));
    for (uint32_t index = 0, j = 0; index < tsize; index++)
    {
        retVal = xpsFdbGetValidEntryByIndex(xpsDevId, index, &fdbEntry);
        if (retVal == XP_NO_ERR)
        {
            xpSaiConvertXpstoSaiFdbEntry(xpsDevId, &fdbEntry, &SfdbEntry);
            pfdbEntry[j++] = SfdbEntry;
        }
    }

    *object_count = objCount;

    for (uint32_t i = 0; i < *object_count; i++)
    {
        object_list[i].key.fdb_entry = pfdbEntry[i];
    }

    xpFree(pfdbEntry);
    return SAI_STATUS_SUCCESS;
}

/**
 * \brief SAI wrapper for adding new Control MAC FDB entry
 *
 * \param devId      - device id
 * \param vlanId     - VLAN ID
 * \param macAddr    - control MAC address
 * \param reasonCode - reason code ID
 * \param indexList  - hash index
 *
 * This API is used to add new Control MAC FDB entry and
 * increment control entries counter
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiFdbAddControlMacEntry(xpsDevice_t devId, uint32_t vlanId,
                                     macAddr_t macAddr,
                                     uint32_t reasonCode, xpsHashIndexList_t *indexList)
{
    XP_STATUS retVal = XP_NO_ERR;

    if (indexList == 0)
    {
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsFdbAddControlMacEntry(devId, vlanId, macAddr, reasonCode,
                                      indexList);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to insert Control MAC entry into Control MAC table. Error code: %d\n",
                       retVal);
        return retVal;
    }

    /* increment Control MAC entries counter */
    retVal = xpSaiSwitchNewCtrlMacEntryAdd(devId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to add new Control MAC entry in SAI DB. Error code: %d\n",
                       retVal);
        return retVal;
    }

    return XP_NO_ERR;
}

/**
 * \brief Add new static FDB entry
 *
 * \param devId     - device id
 * \param fdbEntry  - actual content of new FDB entry
 * \param indexList - hash index
 *
 * This API is used to add new static FDB entry if max limit is not reached
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiFdbAddEntry(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                  xpsHashIndexList_t *indexList)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    if ((fdbEntry == 0) || (indexList == 0))
    {
        return XP_ERR_NULL_POINTER;
    }

    /* check if max FDB limit is not reached */
    xpStatus = xpSaiFdbLimitValidate(devId);
    if (xpStatus != XP_NO_ERR)
    {
        return xpStatus;
    }

    xpStatus = xpsFdbAddEntry(devId, fdbEntry, indexList);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbAddEntry: Error in inserting an entry: error code: %d\n",
                       xpStatus);
        return xpStatus;
    }

    return XP_NO_ERR;
}

/**
 * \brief Validate if FDB entries limit is not reached
 *
 * \param devId         - device id
 *
 * This API is used to validate if new FDB entry can be added without
 * violation of FDB entries limit
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiFdbLimitValidate(xpsDevice_t devId)
{
    XP_STATUS    xpStatus        = XP_NO_ERR;
    sai_status_t saiStatus       = SAI_STATUS_SUCCESS;
    uint32_t     maxFdbEntries   = 0;
    uint32_t     currFdbEntries  = 0;
    uint32_t     cntrlMacEntries = 0;

    saiStatus = xpSaiSwitchMaxFdbEntriesGet(devId, &maxFdbEntries);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get max learned FDB entries from SAI database "
                       "for device %u. Error %u\n", devId, saiStatus);
        return XP_ERR_NOT_FOUND;
    }

    /* Zero means that FDB limit is off */
    if (maxFdbEntries != 0)
    {
        xpStatus = xpsFdbGetNumOfValidEntries(devId, &currFdbEntries);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get current number of FDB entries for device %u."
                           "Error %u\n", devId, xpStatus);
            return xpStatus;
        }

        /* Get number of Control MAC entries */
        xpStatus = xpSaiSwitchCtrlMacEntriesGet(devId, &cntrlMacEntries);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get current number of Control MAC entries "
                           "from SAI database for device %u. Error %u\n", devId, xpStatus);
            return xpStatus;
        }

        /* User is not aware of any Control MAC entry added by SAI adapter.
           So compensate this value from overal FDB entries number. */
        currFdbEntries -= cntrlMacEntries;

        if (currFdbEntries > maxFdbEntries)
        {
            XP_SAI_LOG_ERR("Maximum number of learned FDB entries (%u) reached for device %u\n",
                           maxFdbEntries, devId);
            return XP_ERR_MAX_LIMIT;
        }
    }

    return XP_NO_ERR;
}

static XP_STATUS setFdbAgingTime(xpsDevice_t devId, uint32_t ageTime)
{
    XP_STATUS status  = XP_NO_ERR;


    status = xpsFdbSetAgingTime(devId, ageTime);
    if (XP_NO_ERR != status)
    {
        XP_SAI_LOG_ERR("FDB aging time could not be set to %u.", ageTime);
    }

    return status;
}

//Func: xpSaiSetSwitchAttrFdbAgingTime

sai_status_t xpSaiSetSwitchAttrFdbAgingTime(sai_attribute_value_t value)
{
    XP_STATUS status  = XP_NO_ERR;
    xpsDevice_t devId = xpSaiGetDevId();

    gSaiFdbParams.ageTime = value.u32;

    /* Set FDB aging time to HW */
    status = setFdbAgingTime(devId, gSaiFdbParams.ageTime);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbConfigureTableAging failed to enable fdb table againg. Error #%d\n",
                       status);
        return xpsStatus2SaiStatus(status);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrFdbAgingTime

sai_status_t xpSaiGetSwitchAttrFdbAgingTime(sai_attribute_value_t* value)
{
    value->u32 = gSaiFdbParams.ageTime;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiFdbLearningThreadCb

void* xpSaiFdbLearningThreadCb(void *pArg)
{
    xpsDevice_t *devId = (xpsDevice_t *)pArg;

    XP_SAI_LOG_DBG("SAI FDB learning thread is started");

    xpSaiSwitchThreadsWaitForInit();

    while (!xpSaiGetExitStatus())
    {
    }

    XP_SAI_LOG_DBG("SAI FDB learning thread is finished");

    return devId;
}

//Func: xpSaiFdbInit

XP_STATUS xpSaiFdbInit(xpsDevice_t devId)
{
    XP_STATUS status = XP_NO_ERR;
    // Mac learning event handler
    status = xpsFdbRegisterLearnHandler(devId, xpSaiFdbLearnEventNotification);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbRegisterLearnHandler Failed for device id %d Error #%1d",
                       devId, status);
        return status;
    }
    // FDB Aging event handler
    status = xpsFdbRegisterAgingHandler(devId, xpSaiFdbAgingEventNotification);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbRegisterAgingHandler Failed for device id %d Error #%1d",
                       devId, status);
        return status;
    }

    // Configure Default Aging Attributes
    status = xpsSetAgingCycleUnitTime(devId, XPSAI_TABLETIME);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsSetAgingCycleUnitTime failed to set time to %d. Error #%d\n",
                       XPSAI_TABLETIME, status);
        return status;
    }

    status = xpsSetAgingMode(devId,
                             (xpGetSalType() == XP_SAL_HW_TYPE) ? XP_AGE_MODE_AUTO : XP_AGE_MODE_TRIGGER);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAgeSetAgingMode failed to set XP_AGE_MODE_TRIGGER. Error #%d\n",
                       status);
        return status;
    }

    status = xpsFdbSetRehashLevel(devId, 4);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsSetRehashLevel failed to set rehash level to 4. Error #%d\n",
                       status);
        return status;
    }

    status = setFdbAgingTime(devId, gSaiFdbParams.ageTime);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("setFdbAgingTime failed to configure againg. Error #%d\n",
                       status);
        return status;
    }

#if 0
    saiAgingHandlerThread = sai_thread_create("sai-aging", sai_aging_handler,
                                              (void*)&gSaiFdbParams);
    gSaiFdbLearningThread = sai_thread_create("xpSaiFdbLearningThread",
                                              xpSaiFdbLearningThreadCb, (void*) &devId);
#endif
    return XP_NO_ERR;
}

//Func: xpSaiFdbDeInit
XP_STATUS xpSaiFdbDeInit(xpsDevice_t devId)
{
    XP_STATUS status = XP_NO_ERR;

    status = xpsFdbUnregisterAgingHandler(devId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbUnregisterAgingHandler Failed for device id %d. Error #%d",
                       devId, status);
        return status;
    }

    status = xpsFdbUnregisterLearnHandler(devId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbUnregisterLearnHandler Failed for device id %d. Error #%d",
                       devId, status);
        return status;
    }

    return XP_NO_ERR;
}

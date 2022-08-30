// xpSaiFdb.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiFdb_h_
#define _xpSaiFdb_h_

//Used value zero for aging disable
/* Set 300s(5 minutes) as default aging time */
#define XPSAI_DEFAULT_FDB_AGING_TIME_IN_SEC 300

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _xpSaiFdbEntryAttributesT
{
    sai_attribute_value_t brPortId;
    sai_attribute_value_t packetAction;
    sai_attribute_value_t type;
    sai_attribute_value_t endPointIp;
} xpSaiFdbEntryAttributesT;

/*
 * Properties pertains to an FDB entry.
 */
typedef struct _xpSaiFdbProperties_t
{

    /* Boolean flags to indicate validity of fields */
    uint8_t egrIntfValid;
    uint8_t pktCmdValid;

    /* Properties */
    xpsInterfaceId_t egrIntfId;
    xpsPktCmd_e pktCmd;
    uint32_t serviceInstId;
} xpSaiFdbProperties_t;

/*
 * FDB change notification to interested modules.
 */
typedef struct _xpSaiFdbChangeNotif_t
{
    /* Entry that changed */
    const sai_fdb_entry_t *entry;

    /* Notification type */
    xpSaiNotifType_t notifType;

    /* Properties */
    xpSaiFdbProperties_t props;
} xpSaiFdbChangeNotif_t;

typedef struct xpSaiFdbParams_s
{
    xpsDevice_t xpDevId;
    uint32_t ageTime;
} xpSaiFdbParams_t;

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

XP_STATUS xpSaiFdbLimitValidate(xpsDevice_t devId);

extern pthread_t gSaiFdbLearningThread;
XP_STATUS xpSaiFdbLearnEventNotification(xpDevice_t devId,
                                         xpsFdbEntry_t fdbEntry, xpsFdbEntryAction_e fdbAction);
XP_STATUS xpSaiFdbApiInit(uint64_t flag,
                          const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiFdbApiDeinit();
XP_STATUS xpSaiFdbInit(xpsDevice_t devId);
XP_STATUS xpSaiFdbDeInit(xpsDevice_t devId);

void xpSaiFdbAgingEventNotification(xpDevice_t devId, xpsFdbEntry_t fdbEntry);
void *sai_aging_handler(void *arg);

sai_status_t xpSaiFdbGetXpsEntry(sai_fdb_entry_t *entryKey,
                                 xpsFdbEntry_t *xpsFdbEntry);

sai_status_t xpSaiSetSwitchAttrFdbAgingTime(sai_attribute_value_t value);
sai_status_t xpSaiGetSwitchAttrFdbAgingTime(sai_attribute_value_t* value);
sai_status_t xpSaiMaxCountFdbAttribute(uint32_t *count);
sai_status_t xpSaiCountFdbObjects(uint32_t *count);
sai_status_t xpSaiGetFdbObjectList(uint32_t *object_count,
                                   sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetFdbEntryAttributes(const sai_fdb_entry_t *fdb_entry,
                                            uint32_t *attr_count, sai_attribute_t *attr_list);
sai_status_t xpSaiBridgeVlanObjIdCreate(xpsDevice_t xpsDevId, xpsVlan_t bvId,
                                        sai_object_id_t *bvOid);

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
                                     uint32_t reasonCode, xpsHashIndexList_t *indexList);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiFdb_h_

// xpSaiMcastFdb.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiMcastFdb.h"
#include "xpsMulticast.h"
#include "xpSaiFdb.h"
#include "xpSaiPort.h"
#include "xpsVlan.h"
#include "xpSaiSwitch.h"
#include "xpSaiBridge.h"
#include "xpSaiValidationArrays.h"

XP_SAI_LOG_REGISTER_API(SAI_API_MCAST_FDB);

static sai_mcast_fdb_api_t* _xpSaiMcastFdbApi;

sai_status_t xpSaiConvertMcastFdbAttributes(xpsDevice_t xpsDevId,
                                            xpSaiMcastFdbAttributesT pAttributes, xpsFdbEntry_t *xpsFdbEntry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if (xpsFdbEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (xpSaiConvertSaiPacketAction2xps((sai_packet_action_t) pAttributes.pktCmd,
                                        &xpsFdbEntry->pktCmd) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertSaiPacketAction2xps failed\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsFdbEntry->isStatic = true;

    return retVal;
}

void xpSaiUpdateMcastFdbAttributeVals(const uint32_t attr_count,
                                      const sai_attribute_t* attr_list, xpSaiMcastFdbAttributesT* attributes)
{
    if ((attributes == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid input parameter\n");
        return;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_MCAST_FDB_ENTRY_ATTR_PACKET_ACTION:
                {
                    attributes->pktCmd = attr_list[count].value.s32;
                    break;
                }
            case SAI_MCAST_FDB_ENTRY_ATTR_GROUP_ID:
                {
                    attributes->groupId = attr_list[count].value.oid;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }
}

static sai_status_t xpSaiConvertMcastFdbEntry(sai_mcast_fdb_entry_t
                                              mcast_fdb_entry, xpsFdbEntry_t* xpsFdbEntry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    uint32_t vlanId = xpSaiObjIdValueGet(mcast_fdb_entry.bv_id);

    if (xpsFdbEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memcpy(xpsFdbEntry->macAddr, mcast_fdb_entry.mac_address,
           sizeof(xpsFdbEntry->macAddr));
    xpSaiMacReverse(xpsFdbEntry->macAddr);

    xpsFdbEntry->vlanId = vlanId;
    xpsFdbEntry->serviceInstId = vlanId;

    return retVal;
}

sai_status_t xpSaiSetMcastFdbEntryAttrPacketAction(sai_mcast_fdb_entry_t
                                                   mcast_fdb_entry, sai_attribute_value_t value)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsPktCmd_e xpsPktCmd;
    xpsDevice_t xpsDevId = xpSaiObjIdValueGet(mcast_fdb_entry.switch_id);
    xpsFdbEntry_t fdbEntry;

    memset(&fdbEntry, 0, sizeof(fdbEntry));

    if (xpSaiConvertSaiPacketAction2xps((sai_packet_action_t)value.s32,
                                        &xpsPktCmd) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertSaiPacketAction2xps failed\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiConvertMcastFdbEntry(mcast_fdb_entry, &fdbEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertMcastFdbEntry failed (retVal: %d)\n", retVal);
        return retVal;
    }

    xpRetVal = xpsFdbSetAttribute(xpsDevId, &fdbEntry, XPS_FDB_PKT_CMD,
                                  (void*) &xpsPktCmd);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbSetAttribute failed (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    return retVal;
}

sai_status_t xpSaiGetMcastFdbEntryAttrPacketAction(const sai_mcast_fdb_entry_t
                                                   mcast_fdb_entry, sai_attribute_value_t *value)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpPktCmd_e xpPktCmd = XP_PKTCMD_MAX;
    xpsDevice_t xpsDevId = xpSaiObjIdValueGet(mcast_fdb_entry.switch_id);
    xpsFdbEntry_t fdbEntry;

    memset(&fdbEntry, 0, sizeof(fdbEntry));

    retVal = xpSaiConvertMcastFdbEntry(mcast_fdb_entry, &fdbEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertMcastFdbEntry failed (retVal: %d)\n", retVal);
        return retVal;
    }

    xpRetVal = xpsFdbGetAttribute(xpsDevId, &fdbEntry, XPS_FDB_PKT_CMD,
                                  (void*) &xpPktCmd);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbGetAttribute failed (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    retVal = xpSaiConvertXpsPacketAction2Sai(xpPktCmd,
                                             (sai_packet_action_t*) &value->s32);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        /* In xpSaiGetMcastFdbEntryAttrPacketAction(), if we fail to get xpPktCmd based on given fdbEntry
         * that means we are assuming that the given fdbEntry doesn't exist and return appropriate
         * error to caller.
         */
        XP_SAI_LOG_ERR("xpSaiConvertXpsPacketAction2Sai failed (saiRetVal: %d)\n",
                       retVal);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return retVal;
}

sai_status_t xpSaiSetMcastFdbEntryAttrGroupId(sai_mcast_fdb_entry_t
                                              mcast_fdb_entry, sai_object_id_t groupIdOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsFdbEntry_t fdbEntry;
    uint32_t fdbEntryIndex = 0;
    xpsDevice_t xpsDevId = xpSaiObjIdValueGet(mcast_fdb_entry.switch_id);
    sai_object_id_t oldGroupIdOid;

    memset(&fdbEntry, 0, sizeof(fdbEntry));

    retVal = xpSaiConvertMcastFdbEntry(mcast_fdb_entry, &fdbEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertMcastFdbEntry failed (retVal: %d)\n", retVal);
        return retVal;
    }

    /* We are doing this additional actions as we want to safe all entry attributes.*/
    xpRetVal = xpsFdbFindEntry(xpsDevId, &fdbEntry, &fdbEntryIndex);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbFindEntry failed (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Update an local entry. */
    xpRetVal = xpsFdbGetEntryByIndex(xpsDevId, fdbEntryIndex, &fdbEntry);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbGetEntryByIndex failed (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    retVal = xpSaiGetL2McGroupIdOid(xpsDevId, fdbEntry.intfId, fdbEntry.vlanId,
                                    &oldGroupIdOid);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetL2McGroupIdOid failed (intfId: %d, retVal: %d)\n",
                       fdbEntry.intfId, retVal);
        return retVal;
    }

    /* If it is same value, return success */
    if (groupIdOid == oldGroupIdOid)
    {
        return SAI_STATUS_SUCCESS;
    }

    /* DeActivate L2 Multicast Group previously assigned */
    retVal = xpSaiDeActivateVlanL2McGroup(xpsDevId, mcast_fdb_entry.bv_id,
                                          oldGroupIdOid);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiDeActivateVlanL2McGroup failed (groupIdOid: %" PRIu64
                       ", retVal: %d)\n", oldGroupIdOid, retVal);
        return retVal;
    }

    /* Activate L2 Multicast Group newly assigned */
    if (XDK_SAI_OBJID_TYPE_CHECK(mcast_fdb_entry.bv_id, SAI_OBJECT_TYPE_BRIDGE))
    {
        retVal = xpSaiActivateBridgeL2McGroup(xpsDevId, mcast_fdb_entry.bv_id,
                                              groupIdOid, &fdbEntry.intfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiActivateBridgeL2McGroup failed (groupIdOid: %" PRIu64
                           ", vlanId: %u, retVal: %d)\n",
                           groupIdOid, mcast_fdb_entry.bv_id, retVal);
            return retVal;
        }
    }
    else if (XDK_SAI_OBJID_TYPE_CHECK(mcast_fdb_entry.bv_id, SAI_OBJECT_TYPE_VLAN))
    {
        retVal = xpSaiActivateVlanL2McGroup(xpsDevId, mcast_fdb_entry.bv_id, groupIdOid,
                                            &fdbEntry.intfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiActivateVlanL2McGroup failed (groupIdOid: %" PRIu64
                           ", vlanId: %u, retVal: %d)\n",
                           groupIdOid, mcast_fdb_entry.bv_id, retVal);
            return retVal;
        }
    }
    else
    {
        XP_SAI_LOG_ERR("Invalid bvid passed: bvid= %lu\n", mcast_fdb_entry.bv_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }



    /* Update entry with actual data */
    xpRetVal = xpsFdbWriteEntry(xpsDevId, fdbEntryIndex, &fdbEntry);
    if (xpRetVal != XP_NO_ERR)
    {
        xpSaiDeActivateVlanL2McGroup(xpsDevId, mcast_fdb_entry.bv_id, groupIdOid);
        XP_SAI_LOG_ERR("xpsFdbWriteEntry failed (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    return retVal;
}

sai_status_t xpSaiGetMcastFdbEntryAttrGroupId(sai_mcast_fdb_entry_t
                                              mcast_fdb_entry, sai_attribute_value_t *value)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsFdbEntry_t fdbEntry;
    xpsDevice_t xpsDevId = xpSaiObjIdValueGet(mcast_fdb_entry.switch_id);

    memset(&fdbEntry, 0, sizeof(fdbEntry));

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiConvertMcastFdbEntry(mcast_fdb_entry, &fdbEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertMcastFdbEntry failed (retVal: %d)\n", retVal);
        return retVal;
    }

    xpRetVal = xpsFdbGetEntry(xpsDevId, &fdbEntry);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsFdbFindEntry failed (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    retVal = xpSaiGetL2McGroupIdOid(xpsDevId, fdbEntry.intfId, fdbEntry.vlanId,
                                    &value->oid);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetL2McGroupIdOid failed (retVal: %d)\n", retVal);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiCreateMcastFdbEntry(const sai_mcast_fdb_entry_t
                                      *mcast_fdb_entry,
                                      uint32_t attr_count,
                                      const sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsFdbEntry_t xpsFdbEntry;
    xpSaiMcastFdbAttributesT attributes;
    xpsHashIndexList_t indexList;
    xpsDevice_t xpsDevId;
    sai_uint32_t xpsFdbEntryIndex = -1;

    if (mcast_fdb_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!xpSaiUtilIsMulticastMac(mcast_fdb_entry->mac_address))
    {
        XP_SAI_LOG_ERR("Mac address isn't multicast, mac %x:%x:%x:%x:%x:%x\n",
                       mcast_fdb_entry->mac_address[0],
                       mcast_fdb_entry->mac_address[1],
                       mcast_fdb_entry->mac_address[2],
                       mcast_fdb_entry->mac_address[3],
                       mcast_fdb_entry->mac_address[4],
                       mcast_fdb_entry->mac_address[5]);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            MCAST_FDB_ENTRY_VALIDATION_ARRAY_SIZE,
                            mcast_fdb_entry_attribs, SAI_COMMON_API_CREATE);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    memset(&xpsFdbEntry, 0, sizeof(xpsFdbEntry));

    retVal = xpSaiConvertMcastFdbEntry(*mcast_fdb_entry, &xpsFdbEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to convert SAI Fdb Entry to XPS FDB Entry (retVal: %d)\n",
                       retVal);
        return retVal;
    }

    xpSaiUpdateMcastFdbAttributeVals(attr_count, attr_list, &attributes);

    xpsDevId = xpSaiObjIdValueGet(mcast_fdb_entry->switch_id);

    /* check if max FDB limit is not reached */
    xpRetVal = xpSaiFdbLimitValidate(xpsDevId);
    if (xpRetVal != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpRetVal);
    }

    retVal = xpSaiConvertMcastFdbAttributes(xpsDevId, attributes, &xpsFdbEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to convert SAI Fdb attributes to XPS (retVal: %d)\n",
                       retVal);
        return retVal;
    }

    retVal = xpsFdbFindEntry(xpsDevId, &xpsFdbEntry, &xpsFdbEntryIndex);
    if (retVal == XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Entry is already present in the Fdb table, mac %x:%x:%x:%x:%x:%x\n",
                       mcast_fdb_entry->mac_address[0],
                       mcast_fdb_entry->mac_address[1],
                       mcast_fdb_entry->mac_address[2],
                       mcast_fdb_entry->mac_address[3],
                       mcast_fdb_entry->mac_address[4],
                       mcast_fdb_entry->mac_address[5]);

        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }


    if (XDK_SAI_OBJID_TYPE_CHECK(mcast_fdb_entry->bv_id, SAI_OBJECT_TYPE_BRIDGE))
    {
        retVal = xpSaiActivateBridgeL2McGroup(xpsDevId, mcast_fdb_entry->bv_id,
                                              attributes.groupId, &xpsFdbEntry.intfId);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiActivateBridgeL2McGroup failed (groupIdOid: %" PRIu64
                           ", bridgeId: %lu, retVal: %d)\n",
                           attributes.groupId, mcast_fdb_entry->bv_id, retVal);
            return retVal;
        }
    }
    else if (XDK_SAI_OBJID_TYPE_CHECK(mcast_fdb_entry->bv_id, SAI_OBJECT_TYPE_VLAN))
    {
        retVal = xpSaiActivateVlanL2McGroup(xpsDevId, mcast_fdb_entry->bv_id,
                                            attributes.groupId, &xpsFdbEntry.intfId);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiActivateVlanL2McGroup failed (groupIdOid: %" PRIu64
                           ", vlanId: %lu, retVal: %d)\n",
                           attributes.groupId, mcast_fdb_entry->bv_id, retVal);
            return retVal;
        }
    }
    else
    {
        XP_SAI_LOG_ERR("Invalid bvid passed: bvid= %lu\n", mcast_fdb_entry->bv_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpRetVal = xpsFdbAddEntry(xpsDevId, &xpsFdbEntry, &indexList);
    if (xpRetVal != XP_NO_ERR)
    {
        xpSaiDeActivateVlanL2McGroup(xpsDevId, mcast_fdb_entry->bv_id,
                                     attributes.groupId);
        XP_SAI_LOG_ERR("Failed to add Multicast Fdb Entry on hardware (retVal: %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    return retVal;
}

sai_status_t xpSaiRemoveMcastFdbEntry(const sai_mcast_fdb_entry_t
                                      *mcast_fdb_entry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsFdbEntry_t xpsFdbEntry;
    sai_attribute_value_t groupId;
    xpsDevice_t xpsDevId;

    memset(&xpsFdbEntry, 0, sizeof(xpsFdbEntry));

    if (mcast_fdb_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (mcast_fdb_entry: NULL)\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiConvertMcastFdbEntry(*mcast_fdb_entry, &xpsFdbEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to convert SAI Fdb Entry to XPS FDB Entry (retVal: %d)\n",
                       retVal);
        return retVal;
    }

    retVal = xpSaiGetMcastFdbEntryAttrGroupId(*mcast_fdb_entry, &groupId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get groupIdOid (retVal: %u)\n", retVal);
        return retVal;
    }

    xpsDevId = xpSaiObjIdValueGet(mcast_fdb_entry->switch_id);
    xpRetVal = xpsFdbRemoveEntry(xpsDevId, &xpsFdbEntry);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to remove Multicast Fdb Entry from hardware (retVal: %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    retVal = xpSaiDeActivateVlanL2McGroup(xpsDevId, mcast_fdb_entry->bv_id,
                                          groupId.oid);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiDeActivateVlanL2McGroup failed (groupIdOid: %" PRIu64
                       ", retVal: %d)\n", groupId.oid, retVal);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiSetMcastFdbEntryAttribute(const sai_mcast_fdb_entry_t
                                            *mcast_fdb_entry,
                                            const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if ((mcast_fdb_entry == NULL) || (attr == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(1, attr, MCAST_FDB_ENTRY_VALIDATION_ARRAY_SIZE,
                            mcast_fdb_entry_attribs, SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    switch (attr->id)
    {
        case SAI_MCAST_FDB_ENTRY_ATTR_GROUP_ID:
            {
                retVal = xpSaiSetMcastFdbEntryAttrGroupId(*mcast_fdb_entry, attr->value.oid);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_MCAST_FDB_ENTRY_ATTR_GROUP_ID attribute on hardware (groupIdOid: %"
                                   PRIu64 ", retVal: %d)\n",
                                   attr->value.oid, retVal);
                    return retVal;
                }

                break;
            }
        case SAI_MCAST_FDB_ENTRY_ATTR_PACKET_ACTION:
            {
                retVal = xpSaiSetMcastFdbEntryAttrPacketAction(*mcast_fdb_entry, attr->value);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_MCAST_FDB_ENTRY_ATTR_PACKET_ACTION attribute on hardware (retVal: %d)\n",
                                   retVal);
                    return retVal;
                }

                break;
            }
        case SAI_MCAST_FDB_ENTRY_ATTR_META_DATA:
            {
                XP_SAI_LOG_ERR("Attribute (SAI_MCAST_FDB_ENTRY_ATTR_META_DATA) is not supported\n");
                return SAI_STATUS_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to set %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return retVal;
}

sai_status_t xpSaiGetMcastFdbEntryAttribute(const sai_mcast_fdb_entry_t
                                            *mcast_fdb_entry,
                                            uint32_t attr_count,
                                            sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if (mcast_fdb_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            MCAST_FDB_ENTRY_VALIDATION_ARRAY_SIZE,
                            mcast_fdb_entry_attribs, SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_MCAST_FDB_ENTRY_ATTR_GROUP_ID:
                {
                    retVal = xpSaiGetMcastFdbEntryAttrGroupId(*mcast_fdb_entry,
                                                              &attr_list[count].value);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to get SAI_MCAST_FDB_ENTRY_ATTR_GROUP_ID attribute (retVal: %u)\n",
                                       retVal);
                        return retVal;
                    }

                    break;
                }
            case SAI_MCAST_FDB_ENTRY_ATTR_PACKET_ACTION:
                {
                    retVal = xpSaiGetMcastFdbEntryAttrPacketAction(*mcast_fdb_entry,
                                                                   &attr_list[count].value);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to get SAI_MCAST_FDB_ENTRY_ATTR_PACKET_ACTION attribute (retVal: %u)\n",
                                       retVal);
                        return retVal;
                    }

                    break;
                }
            case SAI_MCAST_FDB_ENTRY_ATTR_META_DATA:
                {
                    XP_SAI_LOG_ERR("Attribute (SAI_MCAST_FDB_ENTRY_ATTR_META_DATA) is not supported\n");
                    return SAI_STATUS_NOT_SUPPORTED;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to get %d\n", attr_list[count].id);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
    }

    return retVal;
}

sai_status_t xpSaiMcastFdbApiInit(uint64_t flag,
                                  const sai_service_method_table_t* adapHostServiceMethodTable)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    _xpSaiMcastFdbApi = (sai_mcast_fdb_api_t *) xpMalloc(sizeof(
                                                             sai_mcast_fdb_api_t));

    if (_xpSaiMcastFdbApi == NULL)
    {
        XP_SAI_LOG_ERR("Failed to allocate _xpSaiMcastFdbApi\n");
        return SAI_STATUS_NO_MEMORY;
    }

    _xpSaiMcastFdbApi->create_mcast_fdb_entry = xpSaiCreateMcastFdbEntry;
    _xpSaiMcastFdbApi->remove_mcast_fdb_entry = xpSaiRemoveMcastFdbEntry;
    _xpSaiMcastFdbApi->set_mcast_fdb_entry_attribute =
        xpSaiSetMcastFdbEntryAttribute;
    _xpSaiMcastFdbApi->get_mcast_fdb_entry_attribute =
        xpSaiGetMcastFdbEntryAttribute;

    retVal = xpSaiApiRegister(SAI_API_MCAST_FDB, (void*) _xpSaiMcastFdbApi);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to register MCAST FDB API\n");
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiMcastFdbApiDeinit()
{
    xpFree(_xpSaiMcastFdbApi);
    _xpSaiMcastFdbApi = NULL;

    return SAI_STATUS_SUCCESS;
}

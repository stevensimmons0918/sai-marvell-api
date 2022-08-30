// xpSaiObject.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiObject.h"

XP_SAI_LOG_REGISTER_API(SAI_API_UNSPECIFIED);

/** \SAIOBJECT SAI - Object API definitions.
 *
 *  \{
 */

/**
 * Routine Description:
 *   @brief Get maximum number of attributes for an object type
 * Arguments:
 *  [in] object_type - SAI object type
 *  [inout] count - maximum number of attribute for an object type
 *
 * Return Values:
 *  SAI_STATUS_SUCCESS on success
 *  Failure status code on error
 */
sai_status_t sai_get_maximum_attribute_count(sai_object_id_t switch_id,
                                             sai_object_type_t object_type,
                                             uint32_t *count)
{
    sai_status_t retVal = xpsStatus2SaiStatus(XP_NO_ERR);

    switch (object_type)
    {
        case SAI_OBJECT_TYPE_PORT:
            retVal = xpSaiMaxCountPortAttribute(count);
            break;
        case SAI_OBJECT_TYPE_STP:
            retVal = xpSaiMaxCountStpAttribute(count);
            break;
        case SAI_OBJECT_TYPE_MIRROR_SESSION:
            retVal = xpSaiMaxCountMirrorAttribute(count);
            break;
#ifdef TBD
        case SAI_OBJECT_TYPE_LAG:
            retVal = xpSaiMaxCountLagAttribute(count);
            break;
        case SAI_OBJECT_TYPE_POLICER:
            retVal = xpSaiMaxCountPolicerAttribute(count);
            break;
        case SAI_OBJECT_TYPE_WRED:
            retVal = xpSaiMaxCountWredAttribute(count);
            break;

        case SAI_OBJECT_TYPE_QOS_MAP:
            retVal = xpSaiMaxCountQosMapsAttribute(count);
            break;
#endif
        case SAI_OBJECT_TYPE_FDB_ENTRY:
            retVal = xpSaiMaxCountFdbAttribute(count);
            break;
        case SAI_OBJECT_TYPE_ACL_TABLE:
            retVal = xpSaiMaxCountAclTableAttribute(count);
            break;
        case SAI_OBJECT_TYPE_ACL_COUNTER:
            retVal = xpSaiMaxCountAclCounterAttribute(count);
            break;
        case SAI_OBJECT_TYPE_ACL_ENTRY:
            retVal = xpSaiMaxCountAclEntryAttribute(count);
            break;
        case SAI_OBJECT_TYPE_ACL_RANGE:
            retVal = xpSaiMaxCountAclRangeAttribute(count);
            break;
        case SAI_OBJECT_TYPE_VIRTUAL_ROUTER:
            retVal = xpSaiMaxCountVirtualRouterAttribute(count);
            break;
        case SAI_OBJECT_TYPE_NEXT_HOP:
            retVal = xpSaiMaxCountNextHopAttribute(count);
            break;
        case SAI_OBJECT_TYPE_NEXT_HOP_GROUP:
            retVal = xpSaiMaxCountNextHopGroupAttribute(count);
            break;
        case SAI_OBJECT_TYPE_HOSTIF:
            retVal = xpSaiMaxCountHostInterfaceAttribute(count);
            break;
        case SAI_OBJECT_TYPE_HOSTIF_TRAP:
            retVal = xpSaiMaxCountHostInterfaceTrapAttribute(count);
            break;
        case SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP:
            retVal = xpSaiMaxCountHostInterfaceTrapGroupAttribute(count);
            break;
        case SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY:
            retVal = xpSaiMaxCountHostInterfaceTableEntryAttribute(count);
            break;
        case SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP:
            retVal = xpSaiMaxCountHostInterfaceUserTrapAttribute(count);
            break;
        case SAI_OBJECT_TYPE_SAMPLEPACKET:
            retVal = xpSaiMaxCountSamplePacketAttribute(count);
            break;
        case SAI_OBJECT_TYPE_SCHEDULER:
            retVal = xpSaiMaxCountSchedulerAttribute(count);
            break;
        case SAI_OBJECT_TYPE_BUFFER_POOL:
            retVal = xpSaiMaxCountBufferPoolAttribute(count);
            break;
        case SAI_OBJECT_TYPE_BUFFER_PROFILE:
            retVal = xpSaiMaxCountBufferProfileAttribute(count);
            break;
        case SAI_OBJECT_TYPE_LAG_MEMBER:
            retVal = xpSaiMaxCountLagMemberAttribute(count);
            break;
        case SAI_OBJECT_TYPE_NEIGHBOR_ENTRY:
            retVal = xpSaiMaxCountNeighborAttribute(count);
            break;
        case SAI_OBJECT_TYPE_QUEUE:
            retVal = xpSaiMaxCountQueueAttribute(count);
            break;
        case SAI_OBJECT_TYPE_VLAN:
            retVal = xpSaiMaxCountVlanAttribute(count);
            break;
        case SAI_OBJECT_TYPE_VLAN_MEMBER:
            retVal = xpSaiMaxCountVlanMemberAttribute(count);
            break;
        case SAI_OBJECT_TYPE_ROUTER_INTERFACE:
            retVal = xpSaiMaxCountRouterInterfaceAttribute(count);
            break;
        default:
            {
                if ((object_type > SAI_OBJECT_TYPE_NULL) &&
                    (object_type < SAI_OBJECT_TYPE_MAX))
                {
                    retVal = SAI_STATUS_NOT_SUPPORTED;
                    XP_SAI_LOG_ERR("Object Type ret : %d is not currently supported\n",
                                   object_type);
                }
                else
                {
                    retVal = SAI_STATUS_INVALID_OBJECT_TYPE;
                    XP_SAI_LOG_ERR("Invalid Object Type ret : %d \n", retVal);
                }
                break;
            }
    }
    return retVal;
}

/**
 * Routine Description:
 *   @brief Get the number of objects present in SAI
 * Arguments:
 *  [in] object_type - SAI object type
 *  [inout] count - number of objects in SAI
 *
 * Return Values:
 *  SAI_STATUS_SUCCESS on success
 *  Failure status code on error
 */
sai_status_t sai_get_object_count(sai_object_id_t switch_id,
                                  sai_object_type_t object_type,
                                  uint32_t *count)
{
    sai_status_t retVal = xpsStatus2SaiStatus(XP_NO_ERR);

    switch (object_type)
    {
        case SAI_OBJECT_TYPE_PORT:
            retVal = xpSaiCountPortObjects(count);
            break;
        case SAI_OBJECT_TYPE_STP:
            retVal = xpSaiCountStpObjects(count);
            break;
        case SAI_OBJECT_TYPE_MIRROR_SESSION:
            retVal = xpSaiCountMirrorObjects(count);
            break;
#ifdef TBD
        case SAI_OBJECT_TYPE_LAG:
            retVal = xpSaiCountLagObjects(count);
            break;
        case SAI_OBJECT_TYPE_POLICER:
            retVal = xpSaiCountPolicerObjects(count);
            break;
        case SAI_OBJECT_TYPE_WRED:
            retVal = xpSaiCountWredObjects(count);
            break;
        case SAI_OBJECT_TYPE_QOS_MAP:
            retVal = xpSaiCountQosMapsObjects(count);
            break;
#endif
        case SAI_OBJECT_TYPE_FDB_ENTRY:
            retVal = xpSaiCountFdbObjects(count);
            break;
        case SAI_OBJECT_TYPE_ACL_COUNTER:
            retVal = xpSaiCountAclCounterObjects(count);
            break;
        case SAI_OBJECT_TYPE_ACL_TABLE:
            retVal = xpSaiCountAclTableObjects(count);
            break;
        case SAI_OBJECT_TYPE_ACL_ENTRY:
            retVal = xpSaiCountAclEntryObjects(count);
            break;
        case SAI_OBJECT_TYPE_ACL_RANGE:
            break;
        case SAI_OBJECT_TYPE_VIRTUAL_ROUTER:
            retVal = xpSaiCountVirtualRouterObjects(count);
            break;
        case SAI_OBJECT_TYPE_NEXT_HOP:
            retVal = xpSaiCountNextHopObjects(count);
            break;
        case SAI_OBJECT_TYPE_NEXT_HOP_GROUP:
            retVal = xpSaiCountNextHopGroupObjects(count);
            break;
        case SAI_OBJECT_TYPE_HOSTIF:
            retVal = xpSaiCountHostInterfaceObjects(count);
            break;
        case SAI_OBJECT_TYPE_HOSTIF_TRAP:
            retVal = xpSaiCountHostInterfaceTrapObjects(count);
            break;
        case SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP:
            retVal = xpSaiCountHostInterfaceTrapGroupObjects(count);
            break;
        case SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY:
            retVal = xpSaiCountHostInterfaceTableEntryObjects(count);
            break;
        case SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP:
            retVal = xpSaiCountHostInterfaceUserTrapObjects(count);
            break;
        case SAI_OBJECT_TYPE_SAMPLEPACKET:
            retVal = xpSaiCountSamplePacketObjects(count);
            break;
        case SAI_OBJECT_TYPE_SCHEDULER:
            retVal = xpSaiCountSchedulerObjects(count);
            break;
        case SAI_OBJECT_TYPE_BUFFER_POOL:
            retVal = xpSaiCountBufferPoolObjects(count);
            break;
        case SAI_OBJECT_TYPE_BUFFER_PROFILE:
            retVal = xpSaiCountBufferProfileObjects(count);
            break;
        case SAI_OBJECT_TYPE_NEIGHBOR_ENTRY:
            retVal = xpSaiCountNeighborObjects(count);
            break;
        case SAI_OBJECT_TYPE_LAG_MEMBER:
            retVal = xpSaiCountLagMemberObjects(count);
            break;
        case SAI_OBJECT_TYPE_QUEUE:
            retVal = xpSaiCountQueueObjects(count);
            break;
        case SAI_OBJECT_TYPE_VLAN:
            retVal = xpSaiCountVlanObjects(count);
            break;
        case SAI_OBJECT_TYPE_VLAN_MEMBER:
            retVal = xpSaiCountVlanMemberObjects(count);
            break;
        case SAI_OBJECT_TYPE_ROUTER_INTERFACE:
            retVal = xpSaiCountRouterInterfaceObjects(count);
            break;
        default:
            {
                if ((object_type > SAI_OBJECT_TYPE_NULL) &&
                    (object_type < SAI_OBJECT_TYPE_MAX))
                {
                    retVal = SAI_STATUS_NOT_SUPPORTED;
                    XP_SAI_LOG_ERR("Object Type ret : %d is not currently supported\n",
                                   object_type);
                }
                else
                {
                    retVal = SAI_STATUS_INVALID_OBJECT_TYPE;
                    XP_SAI_LOG_ERR("Invalid Object Type ret : %d \n", retVal);
                }
                break;
            }
    }
    return retVal;
}

/**
 * Routine Description:
 *   @brief Get the list of object keys present in SAI
 * Arguments:
 *  [in] object_type - SAI object type
 *  [in] count - number of objects in SAI
 *  [in_out] object_list - List of SAI objects or keys
 *
 * Return Values:
 *  SAI_STATUS_SUCCESS on success
 *  Failure status code on error
 */
sai_status_t sai_get_object_key(sai_object_id_t switch_id,
                                sai_object_type_t object_type,
                                uint32_t *object_count,
                                sai_object_key_t *object_list)
{
    sai_status_t retVal = xpsStatus2SaiStatus(XP_NO_ERR);

    if (object_count == NULL)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (object_type)
    {
        case SAI_OBJECT_TYPE_PORT:
            retVal = xpSaiGetPortObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_STP:
            retVal = xpSaiGetStpObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_MIRROR_SESSION:
            retVal = xpSaiGetMirrorObjectList(object_count, object_list);
            break;
#ifdef TBD
        case SAI_OBJECT_TYPE_LAG:
            retVal = xpSaiGetLagObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_POLICER:
            retVal = xpSaiGetPolicerObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_WRED:
            retVal = xpSaiGetWredObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_QOS_MAP:
            retVal = xpSaiGetQosMapsObjectList(object_count, object_list);
            break;
#endif
        case SAI_OBJECT_TYPE_FDB_ENTRY:
            retVal = xpSaiGetFdbObjectList(object_count, object_list);
            break;

        case SAI_OBJECT_TYPE_ACL_TABLE:
            retVal = xpSaiGetAclTableObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_ACL_COUNTER:
            retVal = xpSaiGetAclCounterObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_ACL_ENTRY:
            retVal = xpSaiGetAclEntryObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_ACL_RANGE:
            break;
        case SAI_OBJECT_TYPE_VIRTUAL_ROUTER:
            retVal = xpSaiGetVirtualRouterObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_NEXT_HOP:
            retVal = xpSaiGetNextHopObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_HOSTIF:
            retVal = xpSaiGetHostInterfaceObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_HOSTIF_TRAP:
            retVal = xpSaiGetHostInterfaceTrapObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP:
            retVal = xpSaiGetHostInterfaceTrapGroupObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY:
            retVal = xpSaiGetHostInterfaceTableEntryObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP:
            retVal = xpSaiGetHostInterfaceUserTrapObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_SAMPLEPACKET:
            retVal = xpSaiGetSamplePacketObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_SCHEDULER:
            retVal = xpSaiGetSchedulerObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_BUFFER_POOL:
            retVal = xpSaiGetBufferPoolObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_BUFFER_PROFILE:
            retVal = xpSaiGetBufferProfileObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_NEXT_HOP_GROUP:
            retVal = xpSaiGetNextHopGroupObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_NEIGHBOR_ENTRY:
            retVal = xpSaiGetNeighborObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_LAG_MEMBER:
            retVal = xpSaiGetLagMemberObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_QUEUE:
            retVal = xpSaiGetQueueObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_VLAN:
            retVal = xpSaiGetVlanObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_VLAN_MEMBER:
            retVal = xpSaiGetVlanMemberObjectList(object_count, object_list);
            break;
        case SAI_OBJECT_TYPE_ROUTER_INTERFACE:
            retVal = xpSaiGetRouterInterfaceObjectList(object_count, object_list);
            break;
        default:
            {
                if ((object_type > SAI_OBJECT_TYPE_NULL) &&
                    (object_type < SAI_OBJECT_TYPE_MAX))
                {
                    retVal = SAI_STATUS_NOT_SUPPORTED;
                    XP_SAI_LOG_ERR("Object Type ret : %d is not currently supported\n",
                                   object_type);
                }
                else
                {
                    retVal = SAI_STATUS_INVALID_OBJECT_TYPE;
                    XP_SAI_LOG_ERR("Invalid Object Type ret : %d \n", retVal);
                }
                break;
            }
    }
    return retVal;
}

/*
 * Routine Description:
 *   @brief Get the bulk list of valid attributes for a given list of
 *   object keys.Only valid attributes for an objects are returned.
 *
 * Arguments:
 *  [in] object_type - sai object type
 *  [in] object_count - number of objects
 *  [in] object_key - List of object keys
 *  [inout] attr_count - List of attr_count. Caller passes the number
 *          of attribute allocated in. Callee returns with the actual
 *          number of attributes filled in. If the count is less than
 *          needed, callee fills with the needed count and do not fill
 *          the attributes. Callee also set the corresponding status to
 *          SAI_STATUS_BUFFER_OVERFLOW.
 *
 *  [inout] attrs - list of attributes for every object. Caller is
 *          responsible for allocating and freeing buffer for the attributes.
 *          For list based attribute, e.g., s32list, oidlist, callee should
 *          assume the caller has not allocate the memory for the list and
 *          should only to fill the count but not list. Then, caller
 *          can use corresponding get_attribute to get the list.
 *
 *  [inout] object_statuses - status for each object. If the object does
 *          not exist, callee sets the correpsonding status to SAI_STATUS_INVALID_OBJECT_ID.
 *          If the allocated attribute count is not large enough,
 *          set the status to SAI_STATUS_BUFFER_OVERFLOW.
 *
 * Return Values:
 *  SAI_STATUS_SUCCESS on success
 *  Failure status code on error
 **/

sai_status_t sai_bulk_get_attribute(sai_object_id_t switch_id,
                                    sai_object_type_t object_type,
                                    uint32_t object_count,
                                    const sai_object_key_t *object_key,
                                    uint32_t *attr_count,
                                    sai_attribute_t **attrs,
                                    sai_status_t *object_statuses)
{
    sai_status_t retVal = xpsStatus2SaiStatus(XP_NO_ERR);
    uint32_t i;

    /* Check incoming parameters */
    if ((object_key == NULL) || (attr_count == NULL) || (attrs == NULL) ||
        (object_statuses == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (i = 0; i < object_count; ++i)
    {
        switch (object_type)
        {
            case SAI_OBJECT_TYPE_PORT:
                object_statuses[i] = xpSaiBulkGetPortAttributes(object_key[i].key.object_id,
                                                                &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_STP:
                object_statuses[i] = xpSaiBulkGetStpAttributes(object_key[i].key.object_id,
                                                               &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_MIRROR_SESSION:
                object_statuses[i] = xpSaiBulkGetMirrorAttributes(object_key[i].key.object_id,
                                                                  &attr_count[i], attrs[i]);
                break;
#ifdef TBD
            case SAI_OBJECT_TYPE_LAG:
                object_statuses[i] = xpSaiGetLagAttributes(object_key[i].key.object_id,
                                                           attr_count[i], attrs[i]);
                break;

            case SAI_OBJECT_TYPE_POLICER:
                object_statuses[i] = xpSaiGetPolicerAttribute(object_key[i].key.object_id,
                                                              attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_WRED:
                object_statuses[i] = xpSaiGetWredAttribute(object_key[i].key.object_id,
                                                           attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_QOS_MAP:
                object_statuses[i] = xpSaiGetQosMapAttribute(object_key[i].key.object_id,
                                                             attr_count[i], attrs[i]);
                break;
#endif
            case SAI_OBJECT_TYPE_FDB_ENTRY:
                object_statuses[i] = xpSaiBulkGetFdbEntryAttributes(
                                         &object_key[i].key.fdb_entry, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_ACL_TABLE:
                object_statuses[i] = xpSaiBulkGetAclTableAttributes(object_key[i].key.object_id,
                                                                    &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_ACL_COUNTER:
                object_statuses[i] = xpSaiBulkGetAclCounterAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_ACL_ENTRY:
                object_statuses[i] = xpSaiBulkGetAclEntryAttributes(object_key[i].key.object_id,
                                                                    &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_VIRTUAL_ROUTER:
                object_statuses[i] = xpSaiBulkGetVirtualRouterAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_NEXT_HOP:
                object_statuses[i] = xpSaiBulkGetNextHopAttributes(object_key[i].key.object_id,
                                                                   &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_NEXT_HOP_GROUP:
                object_statuses[i] = xpSaiBulkGetNextHopGroupAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_HOSTIF:
                object_statuses[i] = xpSaiBulkGetHostInterfaceAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_HOSTIF_TRAP:
                object_statuses[i] = xpSaiBulkGetHostInterfaceTrapAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP:
                object_statuses[i] = xpSaiBulkGetHostInterfaceTrapGroupAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY:
                object_statuses[i] = xpSaiBulkGetHostInterfaceTableEntryAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP:
                object_statuses[i] = xpSaiBulkGetHostInterfaceUserTrapAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_SAMPLEPACKET:
                object_statuses[i] = xpSaiBulkGetSamplePacketAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_SCHEDULER:
                object_statuses[i] = xpSaiBulkGetSchedulerAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_BUFFER_POOL:
                object_statuses[i] = xpSaiBulkGetBufferPoolAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_BUFFER_PROFILE:
                object_statuses[i] = xpSaiBulkGetBufferProfileAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_NEIGHBOR_ENTRY:
                object_statuses[i] = xpSaiBulkGetNeighborAttributes(
                                         &object_key[i].key.neighbor_entry, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_LAG_MEMBER:
                object_statuses[i] = xpSaiBulkGetLagMemberAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_QUEUE:
                object_statuses[i] = xpSaiBulkGetQueueAttributes(object_key[i].key.object_id,
                                                                 &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_VLAN:
                object_statuses[i] = xpSaiBulkGetVlanAttributes(object_key[i].key.object_id,
                                                                &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_VLAN_MEMBER:
                object_statuses[i] = xpSaiBulkGetVlanMemberAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            case SAI_OBJECT_TYPE_ROUTER_INTERFACE:
                object_statuses[i] = xpSaiBulkGetRouterInterfaceAttributes(
                                         object_key[i].key.object_id, &attr_count[i], attrs[i]);
                break;
            default:
                {
                    if ((object_type > SAI_OBJECT_TYPE_NULL) &&
                        (object_type < SAI_OBJECT_TYPE_MAX))
                    {
                        retVal = SAI_STATUS_NOT_SUPPORTED;
                        XP_SAI_LOG_ERR("Object Type ret : %d is not currently supported\n",
                                       object_type);
                    }
                    else
                    {
                        retVal = SAI_STATUS_INVALID_OBJECT_TYPE;
                        XP_SAI_LOG_ERR("Invalid Object Type ret : %d \n", retVal);
                    }
                    break;
                }
        }
    }
    return retVal;
}

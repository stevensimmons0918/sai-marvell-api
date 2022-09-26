// xpSaiAcl.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiAcl.h"
#include "xpSaiAclMapper.h"
#include "xpSaiMirror.h"
#include "xpSaiValidationArrays.h"
#include "cpssHalUtil.h"
#include "cpssHalQos.h"

XP_SAI_LOG_REGISTER_API(SAI_API_ACL);
static sai_acl_api_t* _xpSaiAclApi;
static xpsDbHandle_t sXpSaiAclDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t sXpSaiAclPolicerInfoDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static void xpSaiAclEntryAttributesPrint(xpSaiAclEntryAttributesT* pAttributes);
static const char* xpSaiAclKeyFldNameGet(xpsAclKeyFlds_t fld);
//static sai_status_t xpSaiAclSetXpsComparator(xpSaiAclEntryAttributesT* pAttributes);
//static sai_status_t xpSaiAclGetComparatorIndex( xpSaiAclEntryAttributesT* pAttributes, sai_uint8_t *index);
static sai_status_t xpSaiAclEntryAclIdSet(xpsAclkeyFieldList_t *pFieldData,
                                          sai_uint32_t acl_id, bool isSwitchAclEn);
static sai_status_t xpSaiAclFldIdConvert(sai_acl_entry_attr_t attrId,
                                         void * value, xpsAclKeyFlds_t* fld);
static sai_status_t xpSaiAclGetXpsIpType(sai_acl_ip_type_t ipType,
                                         xpsAclKeyFlds_t *pXpsAttr);
static sai_status_t xpSaiAclMirrorErspanEnable(sai_object_id_t session_id,
                                               bool ingress, bool enable,
                                               uint32_t tableId);
#define SAI_ACL_FIELD_MAX_SIZE sizeof(sai_acl_field_data_t)
#define SAI_ACL_GLOBAL_SWITCH_BIND_POINT 0
#define SAI_L4_PORT_ATTRIBUTES_LENGTH 3
typedef struct _xpSaiAclFieldT
{
    sai_uint8_t data[SAI_ACL_FIELD_MAX_SIZE];
} xpSaiAclFieldT;

int32_t ingress_action_list_arr[] = { SAI_ACL_ACTION_TYPE_PACKET_ACTION, SAI_ACL_ACTION_TYPE_MIRROR_INGRESS, SAI_ACL_ACTION_TYPE_COUNTER, SAI_ACL_ACTION_TYPE_REDIRECT};
int32_t egress_action_list_arr[] = { SAI_ACL_ACTION_TYPE_PACKET_ACTION, SAI_ACL_ACTION_TYPE_COUNTER, SAI_ACL_ACTION_TYPE_REDIRECT};

uint32_t ingress_action_list_count = sizeof(ingress_action_list_arr)/sizeof(
                                         ingress_action_list_arr[0]);
uint32_t egress_action_list_count = sizeof(egress_action_list_arr)/sizeof(
                                        egress_action_list_arr[0]);
#if 0
sai_s32_list_t ingress_action_list = { 4, (int32_t[])
{
    SAI_ACL_ACTION_TYPE_PACKET_ACTION, SAI_ACL_ACTION_TYPE_MIRROR_INGRESS,
                                       SAI_ACL_ACTION_TYPE_COUNTER, SAI_ACL_ACTION_TYPE_REDIRECT
}
                                     };

sai_s32_list_t egress_action_list = { 3, (int32_t[])
{
    SAI_ACL_ACTION_TYPE_PACKET_ACTION, SAI_ACL_ACTION_TYPE_COUNTER,
                                       SAI_ACL_ACTION_TYPE_REDIRECT
}
                                    };
#endif

/**
 * \brief SAI ACL State Key Compare function
 *
 * This API is used by the state manager as a method to compare
 * keys
 *
 * \param [in] void* key1
 * \param [in] void* key2
 *
 * \return int32_t
 */
static int32_t sXpSaiAclDbEntryComp(void *key1, void *key2)
{
    return ((sai_uint32_t)(((xpSaiAclEntry_t*)key1)->keyStaticDataType) -
            (sai_uint32_t)(((xpSaiAclEntry_t*)key2)->keyStaticDataType));
}

/**
 * \brief SAI ACL POLICER Info State Key Compare function
 *
 * This API is used by the state manager as a method to compare
 * keys
 *
 * \param [in] void* key1
 * \param [in] void* key2
 *
 * \return int32_t
 */
static int32_t sXpSaiAclPolicerInfoDbEntryComp(void *key1, void *key2)
{
    if ((((xpSaiAclPolicerInfo_t*)key1)->policerId) < (((xpSaiAclPolicerInfo_t*)
                                                        key2)->policerId))
    {
        return -1;
    }
    if ((((xpSaiAclPolicerInfo_t*)key1)->policerId) > (((xpSaiAclPolicerInfo_t*)
                                                        key2)->policerId))
    {
        return 1;
    }
    return (int32_t)((((xpSaiAclPolicerInfo_t*)key1)->client) - (((
                                                                      xpSaiAclPolicerInfo_t*)key2)->client));
}

static uint32_t xpSaiAclConvertRulePriorityToXpPriority(uint32_t priority)
{
    uint32_t xpRulePriority = 0;

    // Assuming priority is validated before calling this API.
    xpRulePriority = (XP_SAI_ENTRY_PRI_NUM_MAX - priority) +
                     XP_SAI_TABLE_PRI_NUM_MIN;

    return xpRulePriority;
}

static sai_status_t xpSaiGetIaclKeyFormat(sai_uint32_t tableId,
                                          sai_acl_ip_type_t  entryType, xpsAclKeyFormat *keyFormat)
{
    XP_STATUS         retVal = XP_NO_ERR;
    sai_status_t      saiStatus = SAI_STATUS_SUCCESS;
    xpsAclTableType_t tableType;
    xpsAclKeyFlds_t   xpsAttr;

    retVal = xpsGetAclTableType(tableId, &tableType);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsGetIaclTableType failed with error %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiStatus = xpSaiAclGetXpsIpType(entryType, &xpsAttr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpsGetXpsIpType failed with error %d \n", saiStatus);
        return saiStatus;
    }

    retVal = xpsGetAclKeyFormat(xpsAttr, tableType, keyFormat);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsGetAclKeyFormat failed with error %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}
//Func: xpSaiConvertAclTableOid
sai_status_t xpSaiConvertAclTableOid(sai_object_id_t acl_table_id,
                                     sai_uint32_t* pTableId)
{
    xpSaiAclTableIdMappingT *pSaiAclTableAttribute = NULL;
    sai_status_t             saiRetVal = SAI_STATUS_SUCCESS;

    if (pTableId == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(acl_table_id, SAI_OBJECT_TYPE_ACL_TABLE))
    {
        XP_SAI_LOG_DBG("Unknown object 0x%" PRIx64 "\n", acl_table_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    *pTableId = (sai_uint32_t)xpSaiObjIdValueGet(acl_table_id);

    saiRetVal = xpSaiAclTableAttributesGet(*pTableId, &pSaiAclTableAttribute);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get Table Entry, ret %d\n", saiRetVal);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiConvertAclEntryOid
sai_status_t xpSaiConvertAclEntryOid(sai_object_id_t acl_entry_id,
                                     sai_uint32_t* pEntryId)
{
    if (pEntryId == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(acl_entry_id, SAI_OBJECT_TYPE_ACL_ENTRY))
    {
        XP_SAI_LOG_DBG("Unknown object 0x%" PRIx64 "\n", acl_entry_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    *pEntryId = (sai_uint32_t)xpSaiObjIdValueGet(acl_entry_id);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiConvertAclCounterOid
sai_status_t xpSaiConvertAclCounterOid(sai_object_id_t acl_counter_id,
                                       sai_uint32_t* pCounterId)
{
    if (pCounterId == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(acl_counter_id, SAI_OBJECT_TYPE_ACL_COUNTER))
    {
        XP_SAI_LOG_DBG("Unknown object 0x%" PRIx64 "\n", acl_counter_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    *pCounterId = (sai_uint32_t)xpSaiObjIdValueGet(acl_counter_id);

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiActionSupportCheck
sai_status_t xpSaiActionSupportCheck(sai_uint32_t attrId)
{
    sai_status_t    retVal = SAI_STATUS_SUCCESS;

    switch (attrId)
    {
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER:
        case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS:
        case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS:
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_TC:
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP:
        case SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION:
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_PACKET_COLOR:
        case SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT:
        case SAI_ACL_ENTRY_ATTR_ACTION_COUNTER:
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_USER_TRAP_ID:
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_ACL_META_DATA:
        //case SAI_ACL_ENTRY_ATTR_ACTION_SET_CPU_QUEUE:
        case SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_BLOCK_PORT_LIST:
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID:
            retVal = SAI_STATUS_SUCCESS;
            break;
        default:
            XP_SAI_LOG_ERR("Not supported Attribute %s(%d) n",
                           xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)attrId),
                           attrId);
            break;
    }

    return retVal;
}

//Func: xpSaiAclPortAclIdSet
sai_status_t xpSaiAclPortAclIdSet(xpsInterfaceId_t intfId, sai_uint32_t aclId,
                                  bool enable)
{
    XP_STATUS        retVal = XP_NO_ERR;

    xpsDevice_t devId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Set acl_id %d to the port %d\n", aclId, intfId);

    retVal = xpsPortSetField(devId, intfId, XPS_PORT_ACL_EN, enable);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsPortSetField(XPS_PORT_ACL_EN) failed with error %d \n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsPortSetField(devId, intfId, XPS_PORT_ACL_ID, aclId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsPortSetField(XPS_PORT_ACL_ID) failed with error %d \n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;

}

//Func: xpSaiAclBridgeAclIdSet
sai_status_t xpSaiAclBridgeAclIdSet(xpsVlan_t vlanId, sai_uint32_t aclId,
                                    bool enable)
{
    XP_STATUS        retVal = XP_NO_ERR;

    xpsDevice_t devId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Set acl_id %d to the bridge %d\n", aclId, vlanId);

    retVal = xpsVlanSetBridgeAclEnable(devId, vlanId, enable);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanSetBridgeAclEnable failed with error %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsVlanSetBridgeAclId(devId, vlanId, aclId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanSetBridgeAclId failed with error %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAclRouterIntfAclIdSet
sai_status_t xpSaiAclRouterAclIdSet(xpsInterfaceId_t l3IntfId, uint32_t groupId,
                                    sai_uint32_t tableId, bool enable)
{
    XP_STATUS        retVal = XP_NO_ERR;

    xpsDevice_t devId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Set acl_id %d to the router port %d\n", tableId, l3IntfId);

    retVal = xpsL3SetRouterAclEnable(devId, l3IntfId, enable);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3SetRouterAclEnable failed with error %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsL3SetRouterAclId(devId, l3IntfId, groupId, tableId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3SetRouterAclId failed with error %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAclEgressAclIdSet
sai_status_t xpSaiAclEgressAclIdSet(xpsInterfaceId_t eVif, sai_uint32_t tableId,
                                    sai_acl_bind_point_type_t aclBindPoint)
{
    sai_status_t  saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiAclTableIdMappingT *pSaiAclTableAttribute = NULL;
    xpSaiAclEntryAttributesT *pSaiAclEntryAttributes = NULL;

    XP_SAI_LOG_DBG("Set acl_id %d to the Evif %d\n", tableId, eVif);

    saiRetVal = xpSaiAclTableAttributesGet(tableId, &pSaiAclTableAttribute);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get Table Entry, ret %d\n", saiRetVal);
        return saiRetVal;
    }

    for (sai_uint16_t count = 0; count < pSaiAclTableAttribute->numEntries; count++)
    {
        saiRetVal = xpSaiAclMapperEntryAttributesGet(
                        pSaiAclTableAttribute->entryId[count], &pSaiAclEntryAttributes);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get entry attributes from DB\n");
            return saiRetVal;
        }

        if (aclBindPoint != SAI_ACL_BIND_POINT_TYPE_SWITCH)
        {
            for (sai_uint16_t index = 0; index < pSaiAclEntryAttributes->out_ports_count;
                 index++)
            {
                if (pSaiAclEntryAttributes->out_ports[index] == eVif)
                {
                    //already present not need to create again
                    return SAI_STATUS_SUCCESS;
                }
            }
        }
        else
        {
            if (pSaiAclEntryAttributes->isSwInstDone == TRUE)
            {
                //already present not need to create again
                continue;
            }
        }

        if (pSaiAclEntryAttributes->entryType == SAI_ACL_IP_TYPE_IPV6ANY)
        {
        }


        if (aclBindPoint == SAI_ACL_BIND_POINT_TYPE_SWITCH)
        {
            pSaiAclEntryAttributes->isSwInstDone = TRUE;
        }
        pSaiAclEntryAttributes->out_ports[pSaiAclEntryAttributes->out_ports_count] =
            eVif;
        pSaiAclEntryAttributes->out_ports_count++;
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetDefaultAclTableAttributeVals
void xpSaiSetDefaultAclTableAttributeVals(xpSaiAclTableAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiSetDefaultAclTableAttributeVals\n");

    /* Assign default table priority */
    attributes->stage_valid = false;
}

//Func: xpSaiAclTableAttributeSet
static sai_status_t xpSaiAclTableAttributeSet(xpSaiAclTableAttributesT*
                                              pAttributes, sai_acl_table_attr_t attr, bool enabled)
{
    sai_uint32_t matchId  = attr - SAI_ACL_TABLE_ATTR_FIELD_START;

    if (matchId >= sizeof(pAttributes->match))
    {
        XP_SAI_LOG_ERR("Table attribute is out of range\n");
        return SAI_STATUS_FAILURE;
    }

    pAttributes->match[matchId] = enabled;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAclTableAttributeEnabledCheck
bool xpSaiAclTableAttributeEnabledCheck(xpSaiAclTableAttributesT* pAttributes,
                                        sai_acl_table_attr_t attr)
{
    sai_uint32_t matchId  = attr - SAI_ACL_TABLE_ATTR_FIELD_START;

    if (matchId >= sizeof(pAttributes->match))
    {
        XP_SAI_LOG_ERR("Table attribute is out of range\n");
        return SAI_STATUS_FAILURE;
    }

    return pAttributes->match[matchId];
}


//Func: xpSaiUpdateAclTableAttributeVals

sai_status_t xpSaiUpdateAclTableAttributeVals(const sai_uint32_t attr_count,
                                              const sai_attribute_t* attr_list, xpSaiAclTableAttributesT* attributes)
{
    sai_status_t    retVal              = SAI_STATUS_SUCCESS;
    xpSaiAclEntry_t xpSaiAclEntryVar;
    sai_uint32_t jj = 0;


    retVal = xpSaiAclStaticVariablesGet(&xpSaiAclEntryVar);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("Failed to get SAI ACL related attributes\n");
        return retVal;
    }

    attributes->isMirror = false;

    for (sai_uint32_t count = 0; count < attr_count; count++)
    {

        switch (attr_list[count].id)
        {

            case SAI_ACL_TABLE_ATTR_ACL_STAGE:
                {
                    attributes->stage = (sai_acl_stage_t)attr_list[count].value.s32;
                    if (attributes->stage != SAI_ACL_STAGE_INGRESS &&
                        attributes->stage != SAI_ACL_STAGE_EGRESS)
                    {
                        XP_SAI_LOG_ERR("Invalid table stage %u\n", attributes->stage);
                        return (SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count));
                    }

                    attributes->stage_valid = true;
                    break;
                }
            case SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST:
                {
                    if (attr_list[count].value.s32list.count >1)
                    {

                        attributes->bindPointCount = attr_list[count].value.s32list.count;
                        memcpy(attributes->bp_list, attr_list[count].value.s32list.list,
                               sizeof(sai_uint32_t)*attributes->bindPointCount);

                    }
                    if (attr_list[count].value.s32list.count == 0)
                    {
                        XP_SAI_LOG_ERR("No bind-point list is sent\n");
                        return (SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count));
                    }
                    //This is dummy for now that can be use in future enhancement
                    attributes->aclBindPoint = (sai_acl_bind_point_type_t)
                                               attr_list[count].value.s32list.list[0];
                    attributes->isBindPointSet = TRUE;
                    break;
                }

            case SAI_ACL_TABLE_ATTR_SIZE:
                {
                    attributes->tableSize = attr_list[count].value.u32;
                    break;
                }


            case SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST:
                {
                    XP_SAI_LOG_NOTICE("Recieved SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST attribute\n");
                    for (jj = 0; jj < attr_list[count].value.s32list.count; jj++)
                    {
                        if ((attr_list[count].value.s32list.list[jj] == (int32_t)
                             SAI_ACL_ACTION_TYPE_MIRROR_INGRESS)
                            || (attr_list[count].value.s32list.list[jj] == (int32_t)
                                SAI_ACL_ACTION_TYPE_MIRROR_EGRESS))
                        {
                            attributes->isMirror = true;
                        }
                    }
                    break;
                }

            default:
                {
                    retVal = xpSaiAclTableAttributeSet(attributes,
                                                       (sai_acl_table_attr_t)attr_list[count].id, attr_list[count].value.booldata);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to set attribute %s(%d)\n",
                                       xpSaiTableAttrNameGet((sai_acl_table_attr_t)attr_list[count].id),
                                       attr_list[count].id);
                        return (SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(count));
                    }

                    break;
                }
        }
    }

    if (attributes->stage_valid == false)
    {
        XP_SAI_LOG_ERR("SAI_ACL_TABLE_ATTR_ACL_STAGE is missing");
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }
    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiAclTableTypeGet
//
// This function determines whether list of attributes belongs to  non ip or V4 or V6
// type of entries.
//
// Parameters:
//     pAttributes -- (input/output) incomming attributes
//
// Returns:
// sai_status_t value

static sai_status_t xpSaiAclTableTypeSet(xpSaiAclTableAttributesT* pAttributes)
{
    bool keyTypeIsV4 = false;
    bool keyTypeIsV6 = false;

    if (xpSaiAclTableAttributeEnabledCheck(pAttributes,
                                           SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6) ||
        xpSaiAclTableAttributeEnabledCheck(pAttributes,
                                           SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6) ||
        xpSaiAclTableAttributeEnabledCheck(pAttributes,
                                           SAI_ACL_TABLE_ATTR_FIELD_IPV6_FLOW_LABEL) ||
        xpSaiAclTableAttributeEnabledCheck(pAttributes,
                                           SAI_ACL_TABLE_ATTR_FIELD_IPV6_NEXT_HEADER) ||
        xpSaiAclTableAttributeEnabledCheck(pAttributes,
                                           SAI_ACL_TABLE_ATTR_FIELD_ICMPV6_TYPE) ||
        xpSaiAclTableAttributeEnabledCheck(pAttributes,
                                           SAI_ACL_TABLE_ATTR_FIELD_ICMPV6_CODE)
       )
    {
        keyTypeIsV6 = true;
        XP_SAI_LOG_DBG("Table type is IPV6\n");
    }
    if (xpSaiAclTableAttributeEnabledCheck(pAttributes,
                                           SAI_ACL_TABLE_ATTR_FIELD_SRC_IP) ||
        xpSaiAclTableAttributeEnabledCheck(pAttributes,
                                           SAI_ACL_TABLE_ATTR_FIELD_DST_IP) ||
        xpSaiAclTableAttributeEnabledCheck(pAttributes,
                                           SAI_ACL_TABLE_ATTR_FIELD_INNER_SRC_IP) ||
        xpSaiAclTableAttributeEnabledCheck(pAttributes,
                                           SAI_ACL_TABLE_ATTR_FIELD_INNER_DST_IP) ||
        xpSaiAclTableAttributeEnabledCheck(pAttributes,
                                           SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE) ||
        xpSaiAclTableAttributeEnabledCheck(pAttributes,
                                           SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE)
       )
    {
        keyTypeIsV4 = true;
        XP_SAI_LOG_DBG("Table type is IPV4\n");

    }

    if (keyTypeIsV6 && keyTypeIsV4)
    {
        pAttributes->keyType = XP_SAI_KEY_IPV4_AND_IPV6;
    }
    else if (keyTypeIsV4)
    {
        pAttributes->keyType = XP_SAI_KEY_IPV4;
    }
    else if (keyTypeIsV6)
    {
        pAttributes->keyType = XP_SAI_KEY_IPV6;
    }
    else
    {
        pAttributes->keyType = XP_SAI_KEY_NON_IP;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetAclPolicerInfo

sai_status_t xpSaiGetAclPolicerInfo(xpsPolicerType_e client,
                                    sai_object_id_t policer_id, xpAcm_t *acmIndex)
{
    xpSaiAclPolicerInfo_t aclPolicerInfoKey;
    xpSaiAclPolicerInfo_t *pAclPolicerInfo = NULL;
    XP_STATUS xpsRetVal = XP_NO_ERR;

    aclPolicerInfoKey.policerId = policer_id;
    aclPolicerInfoKey.client = client;

    /* Retrieve the corresponding state from sai acl policer info db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT,
                                        sXpSaiAclPolicerInfoDbHndl, (xpsDbKey_t)&aclPolicerInfoKey,
                                        (void**)&pAclPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai acl policer info data |Error : %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!pAclPolicerInfo)
    {
        /* No error log, this case is handled silently in higher API calls */
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (acmIndex)
    {
        *acmIndex = pAclPolicerInfo->acmIndex;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAddAclPolicerEntry

sai_status_t xpSaiAddAclPolicerEntry(sai_object_id_t policer_id,
                                     xpsPolicerType_e client, xpAcm_t acmIndex)
{
    sai_status_t        saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS           xpsRetVal = XP_NO_ERR;
    xpsPolicerEntry_t   xpsPolicerEntry;
    xpsDevice_t         xpsDevId = xpSaiGetDevId();

    memset(&xpsPolicerEntry, 0, sizeof(xpsPolicerEntry));

    saiStatus = xpSaiPolicerGetXpsEntry(policer_id, &xpsPolicerEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiPolicerGetXpsEntry failed: status=%i.\n", saiStatus);
        return saiStatus;
    }

    /* Add index to sai policer db index list */
    if ((xpsRetVal = xpSaiPolicerAddIdxToIdxList(policer_id, client,
                                                 acmIndex)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed xpSaiPolicerAddIdxToIdxList |Error: %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Add the ACL policer entry */
    if ((xpsRetVal = xpsPolicerAddEntry(xpsDevId, client, acmIndex, acmIndex,
                                        &xpsPolicerEntry)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to Add ACL policer entry |Error: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Clear policer counter entry */
    if ((xpsRetVal = xpsPolicerClearPolicerCounterEntry(xpsDevId, client,
                                                        acmIndex)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to clear policer counter |Error: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveAclPolicerEntry

sai_status_t xpSaiRemoveAclPolicerEntry(sai_object_id_t policerId,
                                        xpsPolicerType_e client, xpAcm_t acmIndex)
{
    xpsDevice_t  xpsDevId = xpSaiGetDevId();
    XP_STATUS    xpsRetVal = XP_NO_ERR;

    /* Remove index from sai policer db index list */
    if ((xpsRetVal = xpSaiPolicerRemoveIdxFromIdxList(policerId, client,
                                                      acmIndex)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed xpSaiPolicerRemoveIdxFromIdxList |Error: %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /* Remove the ACL policer entry */
    if ((xpsRetVal = xpsPolicerRemoveEntry(xpsDevId, client,
                                           acmIndex)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to remove ACL policer entry |Error: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetAclToPolicerMap

sai_status_t xpSaiSetAclToPolicerMap(sai_object_id_t policer_id,
                                     xpsPolicerType_e client)
{
    xpSaiAclPolicerInfo_t aclPolicerInfoKey;
    xpSaiAclPolicerInfo_t *pAclPolicerInfo = NULL;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    XP_STATUS xpsRetValFinal = XP_NO_ERR;

    aclPolicerInfoKey.policerId = policer_id;
    aclPolicerInfoKey.client = client;

    /* Retrieve the corresponding state from sai acl policer info db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT,
                                        sXpSaiAclPolicerInfoDbHndl, (xpsDbKey_t)&aclPolicerInfoKey,
                                        (void**)&pAclPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai acl policer info data |Error: %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!pAclPolicerInfo)
    {
        /* Create a new sai policer info db */
        if ((xpsRetVal = xpsStateHeapMalloc(sizeof(xpSaiAclPolicerInfo_t),
                                            (void**)&pAclPolicerInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Heap Allocation for sai policer info db failed |Error: %d\n",
                           xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
        memset(pAclPolicerInfo, 0, sizeof(xpSaiAclPolicerInfo_t));

        /* Insert the state into the database */
        pAclPolicerInfo->policerId = policer_id;
        pAclPolicerInfo->client = client;
        if ((xpsRetVal = xpsStateInsertData(XP_SCOPE_DEFAULT,
                                            sXpSaiAclPolicerInfoDbHndl, (void*)pAclPolicerInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to insert sai policer info data |Error: %d\n",
                           xpsRetVal);
            xpsRetValFinal = xpsRetVal;
            /* Free the memory allocated for the corresponding state */
            if ((xpsRetVal = xpsStateHeapFree(pAclPolicerInfo)) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Freeing allocated memory failed |Error: %d\n", xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }
            return xpsStatus2SaiStatus(xpsRetValFinal);
        }
    }

    if (pAclPolicerInfo->refCount == 0)
    {
        if ((saiRetVal = xpsPolicerIndexAllocate(client,
                                                 &pAclPolicerInfo->acmIndex)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to retrieve the sai policer counter data |Error: %d\n",
                           saiRetVal);
            return saiRetVal;
        }

        if ((saiRetVal = xpSaiAddAclPolicerEntry(policer_id, client,
                                                 pAclPolicerInfo->acmIndex))  != SAI_STATUS_SUCCESS)
        {
            if ((saiRetVal = xpsPolicerIndexRelease(client,
                                                    pAclPolicerInfo->acmIndex)) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to release policer index |Error: %d\n", saiRetVal);
                return saiRetVal;
            }

            xpsRetVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, sXpSaiAclPolicerInfoDbHndl,
                                           (xpsDbKey_t)&aclPolicerInfoKey, (void**)&pAclPolicerInfo);
            if (xpsRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to delete sai police info data |Error: %d\n", xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }
            xpsRetVal = xpsStateHeapFree(pAclPolicerInfo);
            if (xpsRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Freeing allocated memory failed |Error: %d\n", xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }
            XP_SAI_LOG_ERR("Failed xpSaiAddAclPolicerEntry |Error: %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    /* Increment the ref count */
    pAclPolicerInfo->refCount += 1;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiUnSetAclToPolicerMap

sai_status_t xpSaiUnSetAclToPolicerMap(sai_object_id_t policer_id,
                                       xpsPolicerType_e client)
{
    xpSaiAclPolicerInfo_t aclPolicerInfoKey;
    xpSaiAclPolicerInfo_t *pAclPolicerInfo = NULL;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;

    /* Get the sai acl policer info and decrement the refCount for corresponding policer_id */
    aclPolicerInfoKey.policerId = policer_id;
    aclPolicerInfoKey.client = client;

    /* Retrieve the corresponding state from sai acl policer info db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT,
                                        sXpSaiAclPolicerInfoDbHndl, (xpsDbKey_t)&aclPolicerInfoKey,
                                        (void**)&pAclPolicerInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai acl policer info data |Error: %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!pAclPolicerInfo)
    {
        XP_SAI_LOG_ERR("SAI ACL Policer info entry not found |Error : %d\n", xpsRetVal);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    /* Decrement the ref count */
    pAclPolicerInfo->refCount -= 1;

    if (pAclPolicerInfo->refCount == 0)
    {
        if ((saiRetVal = xpSaiRemoveAclPolicerEntry(policer_id, client,
                                                    pAclPolicerInfo->acmIndex))  != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed xpSaiRemoveAclPolicerEntry |Error: %d\n", saiRetVal);
            return saiRetVal;
        }

        if ((saiRetVal = xpsPolicerIndexRelease(client,
                                                pAclPolicerInfo->acmIndex)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to release policer index |Error: %d\n", saiRetVal);
            return saiRetVal;
        }

        /* Remove the corresponding state from sai policer info db */
        if ((xpsRetVal = xpsStateDeleteData(XP_SCOPE_DEFAULT,
                                            sXpSaiAclPolicerInfoDbHndl, (xpsDbKey_t)&aclPolicerInfoKey,
                                            (void**)&pAclPolicerInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Deletion of data failed |Error: %d\n", xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }

        /* Free the memory allocated for the corresponding state */
        if ((xpsRetVal = xpsStateHeapFree(pAclPolicerInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Freeing allocated memory failed |Error: %d\n", xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetTblAclEntryAttrActionSetPolicer
sai_status_t xpSaiSetTblAclEntryAttrActionSetPolicer(xpSaiAclEntryAttributesT*
                                                     pSaiAclEntryAttribute, uint32_t tblId,
                                                     sai_object_id_t policer_id,
                                                     xpsPclAction_t *entryData)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS    xpsRetVal = XP_NO_ERR;
    xpsPolicerType_e policerType;

    xpSaiAclTableIdMappingT  *pSaiAclTableAttribute  = NULL;

    xpSaiAclPolicerInfo_t  aclPolicerInfoKey;
    xpSaiAclPolicerInfo_t *pAclPolicerInfo = NULL;

    if ((policer_id != SAI_NULL_OBJECT_ID) &&
        (!XDK_SAI_OBJID_TYPE_CHECK(policer_id, SAI_OBJECT_TYPE_POLICER)))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(policer_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    XP_SAI_LOG_DBG("Set policer_id %d to the acl_id %d\n", policer_id, tblId);

    saiRetVal = xpSaiAclTableAttributesGet(tblId, &pSaiAclTableAttribute);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get Table Entry, ret %d\n", saiRetVal);
        return saiRetVal;
    }

    if (pSaiAclTableAttribute->stage == SAI_ACL_STAGE_INGRESS)
    {
        policerType = XP_INGRESS_FLOW_POLICER;
    }
    else
    {
        policerType = XP_EGRESS_FLOW_POLICER;
    }

    if (pSaiAclEntryAttribute->policerId != SAI_NULL_OBJECT_ID)
    {
        if (pSaiAclEntryAttribute->isPolicerEnable)
        {
            if (pSaiAclEntryAttribute->policerId == policer_id)
            {
                return SAI_STATUS_SUCCESS;
            }
            else
            {
                /* Unset the Acl to PolicerMap */
                if ((saiRetVal = xpSaiUnSetAclToPolicerMap(pSaiAclEntryAttribute->policerId,
                                                           policerType)) != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed xpSaiUnSetAclToPolicerMap |Error: %d\n", saiRetVal);
                    return saiRetVal;
                }

                entryData->policer.policerEnable = CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E;
                entryData->policer.policerId = 0;
                pSaiAclEntryAttribute->isPolicerEnable = GT_FALSE;
                pSaiAclEntryAttribute->policerId = SAI_NULL_OBJECT_ID;
            }
        }
    }

    if (policer_id != SAI_NULL_OBJECT_ID)
    {
        /* Set the Acl to PolicerMap */
        if ((saiRetVal = xpSaiSetAclToPolicerMap(policer_id,
                                                 policerType)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed xpSaiSetAclToPolicerMap |Error: %d\n", saiRetVal);
            return saiRetVal;
        }

        aclPolicerInfoKey.policerId = policer_id;
        aclPolicerInfoKey.client = policerType;

        /* Retrieve the corresponding state from sai acl policer info db */
        if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT,
                                            sXpSaiAclPolicerInfoDbHndl, (xpsDbKey_t)&aclPolicerInfoKey,
                                            (void**)&pAclPolicerInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to retrieve the sai acl policer info data |Error: %d\n",
                           xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }

        if (!pAclPolicerInfo)
        {
            XP_SAI_LOG_ERR("Failed to get acl policer info\n");
            return XP_ERR_NULL_POINTER;
        }

        entryData->policer.policerEnable =
            CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
        entryData->policer.policerId = pAclPolicerInfo->acmIndex;
        pSaiAclEntryAttribute->isPolicerEnable = GT_TRUE;
        pSaiAclEntryAttribute->policerId = policer_id;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetTblAclEntryAttrActionSetMirror
sai_status_t xpSaiSetTblAclEntryAttrActionSetMirror(xpSaiAclEntryAttributesT*
                                                    pSaiAclEntryAttribute, uint32_t tblId,
                                                    sai_object_id_t mirror_session_id,
                                                    xpsPclAction_t *entryData)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    bool         ingress   = false;
    GT_U32       analyzerId = 0;
    sai_mirror_session_type_t sessionType;

    xpSaiAclTableIdMappingT  *pSaiAclTableAttribute  = NULL;

    if ((mirror_session_id != SAI_NULL_OBJECT_ID) &&
        (!XDK_SAI_OBJID_TYPE_CHECK(mirror_session_id, SAI_OBJECT_TYPE_MIRROR_SESSION)))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(mirror_session_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (pSaiAclEntryAttribute == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    XP_SAI_LOG_DBG("Set mirror_id %d to the acl_id %d\n", mirror_session_id, tblId);

    saiRetVal = xpSaiAclTableAttributesGet(tblId, &pSaiAclTableAttribute);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get Table Entry, ret %d\n", saiRetVal);
        return saiRetVal;
    }

    if (pSaiAclTableAttribute->stage == SAI_ACL_STAGE_INGRESS)
    {
        ingress = true;
    }

    if (pSaiAclEntryAttribute->mirrorId != SAI_NULL_OBJECT_ID)
    {
        if (pSaiAclEntryAttribute->isMirrorEnable)
        {
            if ((saiRetVal = xpSaiAclMirrorErspanEnable(pSaiAclEntryAttribute->mirrorId,
                                                        ingress, false, pSaiAclEntryAttribute->tableId)) != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to disable ERSPAN acl mirror |Error: %d\n", saiRetVal);
                return saiRetVal;
            }

            if ((saiRetVal = xpSaiMirrorAclMirrorEnable(pSaiAclEntryAttribute->mirrorId,
                                                        ingress, false, &analyzerId, &sessionType)) != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to unset acl mirror |Error: %d\n", saiRetVal);
                return saiRetVal;
            }

            if (ingress)
            {
                entryData->mirror.mirrorToRxAnalyzerPort = GT_FALSE;
                entryData->mirror.ingressMirrorToAnalyzerIndex = 0;
            }
            else
            {
                entryData->mirror.mirrorToTxAnalyzerPortEn = GT_FALSE;
                entryData->mirror.egressMirrorToAnalyzerIndex = 0;
            }
            pSaiAclEntryAttribute->isMirrorEnable = GT_FALSE;
            pSaiAclEntryAttribute->mirrorId = SAI_NULL_OBJECT_ID;
        }
    }

    if (mirror_session_id != SAI_NULL_OBJECT_ID)
    {
        if ((saiRetVal = xpSaiMirrorAclMirrorEnable(mirror_session_id, ingress, true,
                                                    &analyzerId, &sessionType)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to set acl mirror |Error: %d\n", saiRetVal);
            return saiRetVal;
        }

        if ((saiRetVal = xpSaiAclMirrorErspanEnable(mirror_session_id,
                                                    ingress, true, pSaiAclEntryAttribute->tableId)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to disable ERSPAN acl mirror |Error: %d\n", saiRetVal);
            return saiRetVal;
        }

        if (ingress)
        {
            entryData->mirror.mirrorToRxAnalyzerPort = GT_TRUE;
            entryData->mirror.ingressMirrorToAnalyzerIndex = analyzerId;
        }
        else
        {
            entryData->mirror.mirrorToTxAnalyzerPortEn = GT_TRUE;
            entryData->mirror.egressMirrorToAnalyzerIndex = analyzerId;
        }
        pSaiAclEntryAttribute->isMirrorEnable = GT_TRUE;
        pSaiAclEntryAttribute->mirrorId = mirror_session_id;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetTblAclEntryAttrActionSetMirror
sai_status_t xpSaiSetTblAclEntryAttrActionSetSample(xpSaiAclEntryAttributesT*
                                                    pSaiAclEntryAttribute, uint32_t tblId,
                                                    sai_object_id_t sample_session_id,
                                                    xpsPclAction_t *entryData)
{

    sai_status_t  saiRetVal = SAI_STATUS_SUCCESS;
    bool          ingress = false;
    GT_U32        analyzerId = 0;

    xpSaiAclTableIdMappingT  *pSaiAclTableAttribute  = NULL;
    sai_mirror_session_type_t sessionType;

    if ((sample_session_id != SAI_NULL_OBJECT_ID) &&
        (!XDK_SAI_OBJID_TYPE_CHECK(sample_session_id, SAI_OBJECT_TYPE_SAMPLEPACKET)))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(sample_session_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (pSaiAclEntryAttribute == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAclTableAttributesGet(tblId, &pSaiAclTableAttribute);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get Table Entry, ret %d\n", saiRetVal);
        return saiRetVal;
    }

    if (pSaiAclTableAttribute->stage == SAI_ACL_STAGE_INGRESS)
    {
        ingress = true;
    }

    if (pSaiAclEntryAttribute->sampleId != SAI_NULL_OBJECT_ID)
    {
        if (pSaiAclEntryAttribute->isSampleEnable)
        {
            xpsSamplePacket_t *xpSamplePacketEntry = NULL;
            saiRetVal = xpSaiSamplePacketDbInfoGet(pSaiAclEntryAttribute->sampleId,
                                                   &xpSamplePacketEntry);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to locate session %" PRIu64
                               " in database: error code: %d\n", pSaiAclEntryAttribute->sampleId, saiRetVal);
                return saiRetVal;
            }
            if (xpSamplePacketEntry->type == SAI_SAMPLEPACKET_TYPE_SLOW_PATH)
            {
                if (xpSamplePacketEntry->mirrorSessionId != SAI_NULL_OBJECT_ID)
                {
                    if ((saiRetVal = xpSaiMirrorAclMirrorEnable(
                                         xpSamplePacketEntry->mirrorSessionId,
                                         ingress, false, &analyzerId, &sessionType)) != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to unset acl mirror |Error: %d\n", saiRetVal);
                        return saiRetVal;
                    }
                    xpSamplePacketEntry->sampleRefCnt --;
                }
                if (ingress)
                {
                    entryData->mirror.mirrorToRxAnalyzerPort = GT_FALSE;
                    entryData->mirror.ingressMirrorToAnalyzerIndex = 0;
                    entryData->mirror.cpuCode = CPSS_NET_UNDEFINED_CPU_CODE_E;
                }
                else
                {
                    entryData->mirror.mirrorToTxAnalyzerPortEn = GT_FALSE;
                    entryData->mirror.egressMirrorToAnalyzerIndex = 0;
                    entryData->mirror.cpuCode = CPSS_NET_UNDEFINED_CPU_CODE_E;
                }
            }
            else
            {
                if (xpSamplePacketEntry->sampleRefCnt > 0)
                {
                    if ((--(xpSamplePacketEntry->sampleRefCnt)) == 1)
                    {
                        xpSamplePacketEntry->mirrorSessionId = SAI_NULL_OBJECT_ID;
                    }
                }
                if (ingress)
                {
                    entryData->mirror.cpuCode = CPSS_NET_UNDEFINED_CPU_CODE_E;
                }
                else
                {
                    entryData->mirror.cpuCode = CPSS_NET_UNDEFINED_CPU_CODE_E;
                }
            }

            pSaiAclEntryAttribute->isSampleEnable = GT_FALSE;
            pSaiAclEntryAttribute->sampleId = SAI_NULL_OBJECT_ID;
        }
    }

    if (sample_session_id != SAI_NULL_OBJECT_ID)
    {
        xpsSamplePacket_t *xpSamplePacketEntry = NULL;
        saiRetVal = xpSaiSamplePacketDbInfoGet(sample_session_id, &xpSamplePacketEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to locate session %" PRIu64
                           " in database: error code: %d\n", sample_session_id, saiRetVal);
            return saiRetVal;
        }

        if (xpSamplePacketEntry->type == SAI_SAMPLEPACKET_TYPE_SLOW_PATH)
        {
            if ((saiRetVal = xpSaiMirrorAclMirrorEnable(
                                 xpSamplePacketEntry->mirrorSessionId, ingress, true,
                                 &analyzerId, &sessionType)) != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to set acl mirror |Error: %d\n", saiRetVal);
                return saiRetVal;
            }
            xpSamplePacketEntry->sampleRefCnt ++;
            if (ingress)
            {
                entryData->mirror.mirrorToRxAnalyzerPort = GT_TRUE;
                entryData->mirror.ingressMirrorToAnalyzerIndex = analyzerId;
                entryData->mirror.cpuCode = CPSS_NET_INGRESS_MIRRORED_TO_ANLYZER_E;
            }
            else
            {
                entryData->mirror.mirrorToTxAnalyzerPortEn = GT_TRUE;
                entryData->mirror.egressMirrorToAnalyzerIndex = analyzerId;
                entryData->mirror.cpuCode = CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E;

            }
        }
        else
        {
            xpSaiMirrorSession_t *session = NULL;
            if (pSaiAclEntryAttribute->isMirrorEnable == false)
            {
                XP_SAI_LOG_ERR("No mirror in acl entry\n");
                return SAI_STATUS_FAILURE;
            }
            saiRetVal = xpSaiMirrorDbInfoGet(pSaiAclEntryAttribute->mirrorId, &session);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_DBG("Failed to locate session %" PRIu64
                               " in data base, error: %d \n", pSaiAclEntryAttribute->mirrorId, saiRetVal);
                return saiRetVal;
            }
            if ((!session) || (session->sessionType != SAI_MIRROR_SESSION_TYPE_SFLOW))
            {
                XP_SAI_LOG_ERR("No sflow mirror in acl entry\n");
                return SAI_STATUS_FAILURE;
            }
            xpSamplePacketEntry->mirrorSessionId = pSaiAclEntryAttribute->mirrorId;
            if (ingress)
            {
                entryData->mirror.mirrorToRxAnalyzerPort = GT_TRUE;
                entryData->mirror.cpuCode = CPSS_NET_INGRESS_MIRRORED_TO_ANLYZER_E;
            }
            else
            {
                entryData->mirror.mirrorToTxAnalyzerPortEn = GT_TRUE;
                entryData->mirror.cpuCode = CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E;
            }
            xpSamplePacketEntry->sampleRefCnt ++;
        }

        pSaiAclEntryAttribute->isSampleEnable = GT_TRUE;
        pSaiAclEntryAttribute->sampleId = sample_session_id;
    }

    return SAI_STATUS_SUCCESS;
}
/*
//Func: xpSaiAclEntryDataSet
//
// This function fills in entryData structure with action information for
// IPv4 rule
//
// Parameters:
//     xpSaiAclEntryAttributesT attributes -- (input) incomming attributes array
//     xpsPclAction_t *entryData --  (output) Ingress data for V4 entry
//
// Returns:
//     sai_status_t value
*/
static sai_status_t xpSaiAclEntryDataSet(xpSaiAclEntryAttributesT* pAttributes,
                                         xpsPclAction_t *entryData, sai_uint32_t tblId)
{
    sai_status_t            saiRetVal   = SAI_STATUS_SUCCESS;
    xpsPktCmd_e             xpsPktCmd   = XP_PKTCMD_MAX;
    //sai_uint32_t            aclQueueId  = 0;
    sai_uint32_t            devId = xpSaiGetDevId();
    sai_acl_action_data_t   action;
    sai_uint32_t            counterId  = 0;
    sai_object_id_t         policer_id = SAI_NULL_OBJECT_ID;
    xpsScope_t  scopeId     = 0;
    XP_STATUS xpsRetVal = XP_NO_ERR;

    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if ((pAttributes == NULL) || (entryData == NULL))
    {
        XP_SAI_LOG_ERR("%s:, Error: Invalid parameter\n", __FUNCNAME__);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (sai_uint32_t attrId = SAI_ACL_ENTRY_ATTR_ACTION_START;
         attrId < SAI_ACL_ENTRY_ATTR_ACTION_END + 1; attrId++)
    {

        if (xpSaiAclEntryActionAttributeIsSetByUser(pAttributes,
                                                    (sai_acl_entry_attr_t)attrId) == FALSE)
        {
            continue;
        }
        switch ((sai_acl_entry_attr_t)attrId)
        {

            case SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION:

                if (xpSaiAclEntryActionAttributeEnabledCheck(pAttributes,
                                                             SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION) == TRUE)
                {
                    xpSaiAclEntryActionAttributeGet(pAttributes,
                                                    SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION, &action);

                    saiRetVal = xpSaiConvertSaiPacketAction2xps((sai_packet_action_t)
                                                                action.parameter.s32, &xpsPktCmd);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not convert SAI action %d\n", action.parameter.s32);
                        return saiRetVal;
                    }

                    if (action.parameter.s32 == SAI_PACKET_ACTION_DROP)
                    {
                        entryData->pktCmd  =  XPS_PACKET_CMD_DROP_SOFT_E;
                        entryData->mirror.cpuCode = XPS_SOFT_DROP_REASON_CODE;
                    }
                    else if (action.parameter.s32 == SAI_PACKET_ACTION_DENY)
                    {
                        entryData->pktCmd  =  XPS_PACKET_CMD_DROP_HARD_E;
                        entryData->mirror.cpuCode = XPS_HARD_DROP_REASON_CODE;
                    }
                    else if (action.parameter.s32 == SAI_PACKET_ACTION_TRAP)
                    {
                        entryData->pktCmd  = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
                        entryData->mirror.cpuCode = XPS_TRAP_DROP_REASON_CODE;
                    }
                    else if ((action.parameter.s32 == SAI_PACKET_ACTION_LOG) ||
                             (action.parameter.s32 == SAI_PACKET_ACTION_COPY))
                    {
                        entryData->pktCmd  = XPS_PACKET_CMD_MIRROR_TO_CPU_E;
                        entryData->mirror.cpuCode = XPS_SOFT_DROP_REASON_CODE;;
                    }
                    else if ((action.parameter.s32 == SAI_PACKET_ACTION_FORWARD) ||
                             (action.parameter.s32 == SAI_PACKET_ACTION_TRANSIT))
                    {
                        entryData->pktCmd = XPS_PACKET_CMD_FORWARD_E;
                    }
                    else
                    {
                        return SAI_STATUS_SUCCESS;
                    }

                }
                else
                {
                    entryData->pktCmd = XPS_PACKET_CMD_FORWARD_E;
                }
                break;

            case SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP:

                if (xpSaiAclEntryActionAttributeEnabledCheck(pAttributes,
                                                             SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP) == TRUE)
                {
                    xpSaiAclEntryActionAttributeGet(pAttributes, SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP,
                                                    &action);
                    entryData->qos.egress.modifyDscp =
                        XPS_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E;
                    entryData->qos.egress.dscp = action.parameter.u8;
                }
                else
                {
                    entryData->qos.egress.modifyDscp = XPS_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E;
                }
                break;

            case SAI_ACL_ENTRY_ATTR_ACTION_COUNTER:
                if (xpSaiAclEntryActionAttributeEnabledCheck(pAttributes,
                                                             SAI_ACL_ENTRY_ATTR_ACTION_COUNTER) == TRUE)
                {
                    //call the api to modify the counter id to a smaller counter id :xpSaiAclEntryCounterApply here
                    xpSaiAclEntryActionAttributeGet(pAttributes, SAI_ACL_ENTRY_ATTR_ACTION_COUNTER,
                                                    &action);
                    saiRetVal = xpSaiConvertAclCounterOid(action.parameter.oid, &counterId);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        if (saiRetVal == SAI_STATUS_INVALID_OBJECT_TYPE)
                        {
                            XP_SAI_LOG_ERR("Invalid value of oid provided in attribute\n");
                            return SAI_STATUS_INVALID_ATTR_VALUE_0;
                        }
                        else
                        {
                            return saiRetVal;
                        }
                    }
                    entryData->matchCounter.enableMatchCount = (XPS_BOOL)action.enable;
                    entryData->matchCounter.matchCounterIndex = counterId;
                    //printf("\n entryData matchCount and matchCounterIndex Values %d %d",entryData->matchCounter.enableMatchCount, entryData->matchCounter.matchCounterIndex);
                }
                else
                {
                    XP_SAI_LOG_ERR("SAI_ACL_ENTRY_ATTR_ACTION_COUNTER is not enabled..\n");
                    return SAI_STATUS_ATTR_NOT_SUPPORTED_0;
                }
                break;

            case SAI_ACL_ENTRY_ATTR_ACTION_SET_PACKET_COLOR:
                break;

            case SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT:
                if (xpSaiAclEntryActionAttributeEnabledCheck(pAttributes,
                                                             SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT) == TRUE)
                {
                    sai_object_type_t saiObjType = SAI_OBJECT_TYPE_NULL;
                    xpSaiAclEntryActionAttributeGet(pAttributes, SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT,
                                                    &action);
                    saiObjType = sai_object_type_query(action.parameter.oid);
                    switch (saiObjType)
                    {
                        case SAI_OBJECT_TYPE_PORT:
                            xpsInterfaceId_t xpsIntf;
                            xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(action.parameter.oid);
                            entryData->redirect.redirectCmd = XPS_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
                            entryData->redirect.data.outIf.outInterface.devPort.hwDevNum = xpSaiGetDevId();
                            entryData->redirect.data.outIf.outInterface.devPort.portNum = xpsIntf;

                            break;
                        case SAI_OBJECT_TYPE_NEXT_HOP:
                            {
                                /*Get xps NHDbInfo.*/
                                uint32_t xpsNhId = 0;
                                uint32_t hwLeafId = 0;
                                xpsL3NhSwDbEntry_t *pxpsNhSwDbInfo = NULL;
                                XP_STATUS    xpsRetVal = XP_NO_ERR;
                                xpsNhId = (uint32_t)xpSaiObjIdValueGet(action.parameter.oid);
                                xpsRetVal = xpsL3GetRouteNextHopSwDb(scopeId, xpsNhId, &pxpsNhSwDbInfo);
                                if (xpsRetVal != XP_NO_ERR)
                                {
                                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                          "Could not get next hop, nhId %d, xpsRetVal %d",
                                          xpsNhId, xpsRetVal);
                                    return xpsStatus2SaiStatus(xpsRetVal);
                                }

                                hwLeafId = pxpsNhSwDbInfo->hwPbrLeafId;

                                if (hwLeafId == XPS_INTF_INVALID_ID)
                                {
                                    xpsRetVal = xpsL3SetRouteNextHop(devId, xpsNhId,
                                                                     &(pxpsNhSwDbInfo->nhEntry));
                                    if (xpsRetVal != XP_NO_ERR)
                                    {
                                        XP_SAI_LOG_ERR("ACL xpsL3SetRouteNextHop : error code: %d\n",
                                                       xpsRetVal);
                                        return xpsStatus2SaiStatus(xpsRetVal);
                                    }

                                    /*Allocate a new Leaf Idx corresponding to NH*/
                                    xpsRetVal = xpsAllocatorAllocateId(scopeId, XP_ALLOC_PBR_HW_ENTRY,
                                                                       &hwLeafId);
                                    if (xpsRetVal != XP_NO_ERR)
                                    {
                                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                              "xpsAllocatorAllocateId Failed : %d\n", xpsRetVal);
                                        return xpsStatus2SaiStatus(xpsRetVal);
                                    }
                                    xpsRetVal = xpsL3PbrEntrySet(devId, hwLeafId,
                                                                 pxpsNhSwDbInfo->hwNhId,
                                                                 false);
                                    if (xpsRetVal != XP_NO_ERR)
                                    {
                                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                              "xps Pbr Leaf set failed, nhId %d, hwleafId %d xpsRetVal %d",
                                              xpsNhId, hwLeafId, xpsRetVal);
                                        return xpsStatus2SaiStatus(xpsRetVal);
                                    }
                                    pxpsNhSwDbInfo->hwPbrLeafId = hwLeafId;
                                }
                                entryData->pktCmd = XPS_PACKET_CMD_FORWARD_E;
                                entryData->redirect.redirectCmd = XPS_PCL_ACTION_REDIRECT_CMD_ROUTER_E;
                                entryData->redirect.data.routerLttIndex = hwLeafId;
                            }
                            break;
                        case SAI_OBJECT_TYPE_NEXT_HOP_GROUP:
                            {
                                /*Get xps NHGrp DbInfo.*/
                                uint32_t xpsNhGrpId = 0;
                                xpsL3NextHopGroupEntry_t* pNhGrpEntry = NULL;
                                uint32_t hwLeafId = 0;
                                XP_STATUS    xpsRetVal = XP_NO_ERR;
                                xpsNhGrpId = (uint32_t)xpSaiObjIdValueGet(action.parameter.oid);
                                xpsRetVal = xpsL3GetRouteNextHopGroup(scopeId, xpsNhGrpId, &pNhGrpEntry);
                                if (xpsRetVal != XP_NO_ERR)
                                {
                                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                          "Could not get next hop group nhGrpId %d, xpsRetVal %d",
                                          xpsNhGrpId, xpsRetVal);
                                    return xpsStatus2SaiStatus(xpsRetVal);
                                }
                                hwLeafId = pNhGrpEntry->hwPbrLeafId;

                                if (hwLeafId == XPS_INTF_INVALID_ID)
                                {
                                    /*Allocate a new Leaf Idx corresponding to NH*/
                                    xpsRetVal = xpsAllocatorAllocateId(scopeId, XP_ALLOC_PBR_HW_ENTRY, &hwLeafId);
                                    if (xpsRetVal != XP_NO_ERR)
                                    {
                                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                              "xpsAllocatorAllocateId Failed : %d\n", xpsRetVal);
                                        return xpsStatus2SaiStatus(xpsRetVal);
                                    }
                                    xpsRetVal = xpsL3PbrEntrySet(devId, hwLeafId, xpsNhGrpId, true);
                                    if (xpsRetVal != XP_NO_ERR)
                                    {
                                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                              "xps Pbr Leaf set failed nhGrpId %d,hwleafId %d xpsRetVal %d",
                                              xpsNhGrpId, hwLeafId, xpsRetVal);
                                        return xpsStatus2SaiStatus(xpsRetVal);
                                    }
                                    pNhGrpEntry->hwPbrLeafId = hwLeafId;
                                }
                                entryData->pktCmd = XPS_PACKET_CMD_FORWARD_E;
                                entryData->redirect.redirectCmd = XPS_PCL_ACTION_REDIRECT_CMD_ROUTER_E;
                                entryData->redirect.data.routerLttIndex = hwLeafId;
                            }
                            break;

                        default:
                            break;
                    }
                }
                else
                {
                    entryData->redirect.redirectCmd = XPS_PCL_ACTION_REDIRECT_CMD_NONE_E;
                }
                break;

            case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS:
                if (xpSaiAclEntryActionAttributeEnabledCheck(pAttributes,
                                                             SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS) == TRUE)
                {
                    xpSaiAclEntryActionAttributeGet(pAttributes,
                                                    SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS, &action);
                    saiRetVal = xpSaiSetTblAclEntryAttrActionSetMirror(pAttributes, tblId,
                                                                       *action.parameter.objlist.list,
                                                                       entryData);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("Failed to set ingress mirror session |retVal = %d\n",
                                       saiRetVal);
                        return saiRetVal;
                    }
                }
                else
                {
                    entryData->mirror.mirrorToRxAnalyzerPort = GT_FALSE;
                }
                break;

            case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS:
                if (xpSaiAclEntryActionAttributeEnabledCheck(pAttributes,
                                                             SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS) == TRUE)
                {
                    xpSaiAclEntryActionAttributeGet(pAttributes,
                                                    SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS, &action);
                    saiRetVal = xpSaiSetTblAclEntryAttrActionSetMirror(pAttributes, tblId,
                                                                       *action.parameter.objlist.list,
                                                                       entryData);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("Failed to set egress mirror session |retVal = %d\n",
                                       saiRetVal);
                        return saiRetVal;
                    }
                }
                else
                {
                    entryData->mirror.mirrorToTxAnalyzerPortEn = GT_FALSE;
                }
                break;

            case SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER:

                if (xpSaiAclEntryActionAttributeEnabledCheck(pAttributes,
                                                             SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER) == TRUE)
                {
                    xpSaiAclEntryActionAttributeGet(pAttributes,
                                                    SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER, &action);

                    policer_id = action.parameter.oid;
                    entryData->policer.policerEnable =
                        CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
                    entryData->policer.policerId = xpSaiObjIdValueGet(action.parameter.oid);
                    saiRetVal = xpSaiSetTblAclEntryAttrActionSetPolicer(pAttributes, tblId,
                                                                        policer_id,
                                                                        entryData);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("Failed to set (SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER) |retVal = %d\n",
                                       saiRetVal);
                        return saiRetVal;
                    }
                }
                break;
            case SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE:
                if (xpSaiAclEntryActionAttributeEnabledCheck(pAttributes,
                                                             SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE) == TRUE)
                {
                    xpSaiAclEntryActionAttributeGet(pAttributes,
                                                    SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE, &action);
                    saiRetVal = xpSaiSetTblAclEntryAttrActionSetSample(pAttributes, tblId,
                                                                       action.parameter.oid,
                                                                       entryData);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("Failed to enable ingress sample packet |retVal = %d\n",
                                       saiRetVal);
                        return saiRetVal;
                    }
                }
                break;

            case SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE:
                if (xpSaiAclEntryActionAttributeEnabledCheck(pAttributes,
                                                             SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE) == TRUE)
                {
                    xpSaiAclEntryActionAttributeGet(pAttributes,
                                                    SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE, &action);
                    saiRetVal = xpSaiSetTblAclEntryAttrActionSetSample(pAttributes, tblId,
                                                                       action.parameter.oid,
                                                                       entryData);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("Failed to enable egress sample packet |retVal = %d\n",
                                       saiRetVal);
                        return saiRetVal;
                    }
                }
                break;

            case SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID:
                if (xpSaiAclEntryActionAttributeEnabledCheck(pAttributes,
                                                             SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID) == TRUE)
                {
                    xpSaiAclTableIdMappingT *pSaiAclTableAttribute = NULL;
                    saiRetVal = xpSaiAclTableAttributesGet(tblId, &pSaiAclTableAttribute);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not get Table Entry, ret %d\n", saiRetVal);
                        return saiRetVal;
                    }

                    xpSaiAclEntryActionAttributeGet(pAttributes,
                                                    SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID, &action);
                    if (pSaiAclTableAttribute->stage == SAI_ACL_STAGE_INGRESS)
                    {
                        // entryData->vlan.ingress.vlanId1Cmd = CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E;
                        // entryData->vlan.ingress.vlanId1 = action.parameter.u16;
                        entryData->vlan.ingress.vlanId = action.parameter.u16;
                        entryData->vlan.ingress.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E;
                    }
                    else
                    {
                        // entryData->vlan.egress.vlanId1ModifyEnable = GT_TRUE;
                        // entryData->vlan.egress.vlanId1 = action.parameter.u16;
                        entryData->vlan.egress.vlanId = action.parameter.u16;
                    }

                }
                break;

            default:
                XP_SAI_LOG_DBG("Unsupported attr %s\n",
                               xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)attrId));
                return SAI_STATUS_SUCCESS;
        }
    }

    return saiRetVal;
}

//Func: xpSaiAclEntryFieldDataSet
//
// This function fills in entryData structure with match information for
// IPv4 rule
//
// Parameters:
//     xpSaiAclEntryAttributesT attributes -- (input) incomming attributes array
//     xpsAclkeyFieldList_t *fieldData --  (output) Ingress data for V4 entry
//     sai_uint32_t tblId    -- (input) Table Id
// Returns:
//     sai_status_t value

static sai_status_t xpSaiAclEntryFieldDataSet(xpSaiAclEntryAttributesT*
                                              pAttributes,
                                              xpsAclkeyFieldList_t *pFieldData,
                                              sai_uint32_t tblId)
{
    sai_status_t             retVal = SAI_STATUS_SUCCESS;
    sai_acl_field_data_t     match;
    xpSaiAclTableIdMappingT *pSaiAclTableAttribute = NULL;
    xpsAclKeyFlds_t          fld;
    sai_uint32_t             aclId = 0;
    sai_uint32_t             keyIndex = 0;

    memset(&fld, 0, sizeof(xpsAclKeyFlds_t));

    /* Get table info */
    if ((retVal = xpSaiAclTableAttributesGet(tblId,
                                             &pSaiAclTableAttribute)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:, Failed to get table %u related data\n", __FUNCNAME__,
                       tblId);
        return retVal;
    }

    if (pFieldData == NULL)
    {
        XP_SAI_LOG_ERR("%s:, Error: Invalid parameter\n", __FUNCNAME__);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    // this field is in same place for both XP_EACL_V6_TYPE/XP_EACL_V4_TYPE
    //Converting table Id to aclId.
    aclId = (XP_SAI_ACL_ID_MASK & tblId);

    retVal = xpSaiAclEntryAclIdSet(pFieldData, aclId,
                                   pSaiAclTableAttribute->isSwitchAclEn);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set acl id, ret = %u \n", retVal);
        return retVal;
    }
    keyIndex = pFieldData->numFlds;

    for (sai_uint32_t attrId = SAI_ACL_ENTRY_ATTR_FIELD_START;
         attrId < SAI_ACL_ENTRY_ATTR_FIELD_END + 1; attrId++)
    {
        if (xpSaiAclEntryMatchAttributeIsSetByUser(pAttributes,
                                                   (sai_acl_entry_attr_t)attrId) == FALSE)
        {
            continue;
        }
        retVal = xpSaiAclFldIdConvert((sai_acl_entry_attr_t)attrId,
                                      &(pAttributes->entryType), &fld);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not convert match field id\n");
            return retVal;
        }
        XP_SAI_LOG_DBG("Set value and mask for %s\n",
                       xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)attrId));

        retVal = xpSaiAclEntryMatchAttributeGet(pAttributes,
                                                (sai_acl_entry_attr_t)attrId, &match);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not get match attribute\n");
            return SAI_STATUS_FAILURE;
        }

        if (xpSaiAclEntryMatchAttributeEnabledCheck(pAttributes,
                                                    (sai_acl_entry_attr_t)attrId) == TRUE)
        {

            retVal = xpSaiAclEntryMatchAttributeConvert(match, (sai_acl_entry_attr_t)attrId,
                                                        pFieldData, fld, &keyIndex, pAttributes);
            if (retVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Can not convert value and mask\n");
                return SAI_STATUS_FAILURE;
            }

        }
        else
        {
            retVal = xpSaiAclEntryUnMatchAttributeConvert((sai_acl_entry_attr_t)attrId,
                                                          pFieldData, fld, &keyIndex);
            if (retVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiAclEntryUnMatchAttributeConvert failed for %s \n",
                               xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)attrId));
                return SAI_STATUS_FAILURE;
            }


        }
    }

    pFieldData->numFlds = keyIndex;
    return retVal;
}


static const char* xpSaiAclKeyFldNameGet(xpsAclKeyFlds_t fld)
{
    switch (fld)
    {
        case XPS_PCL_PKT_TYPE:
            return "XPS_PCL_PKT_TYPE";
        case XPS_PCL_PCLID:
            return "XPS_PCL_PCLID";
        case XPS_PCL_MAC_DA:
            return "XPS_PCL_MAC_DA";
        case XPS_PCL_MAC_SA:
            return "XPS_PCL_MAC_SA";
        case XPS_PCL_ETHER_TYPE:
            return "XPS_PCL_ETHER_TYPE";
        case XPS_PCL_VID1:
            return "XPS_PCL_VID1";
        case XPS_PCL_DIP:
            return "XPS_PCL_DIP";
        case XPS_PCL_SIP:
            return "XPS_PCL_SIP";
        case XPS_PCL_L4_BYTE0:
            return "XPS_PCL_L4_BYTE0";
        case XPS_PCL_L4_BYTE2:
            return "XPS_PCL_L4_BYTE2";
        case XP_PCL_SRC_PORT:
            return "XP_PCL_SRC_PORT";
        case XPS_PCL_L4_BYTE1:
            return "XPS_PCL_L4_BYTE1";
        case XPS_PCL_IP_PROTOCOL:
            return "XPS_PCL_IP_PROTOCOL";
        case XPS_PCL_DSCP:
            return "XPS_PCL_DSCP";
        case XPS_PCL_TTL:
            return "XPS_PCL_TTL";
        case XPS_PCL_IP_PKT_LEN:
            return "XPS_PCL_IP_PKT_LEN";
        case XPS_PCL_L4_BYTE13:
            return "XPS_PCL_L4_BYTE13";
        case XPS_PCL_VRF_ID:
            return "XPS_PCL_VRF_ID";
        case XPS_PCL_IP_FRAGMENT:
            return "XPS_PCL_IP_FRAGMENT";
        case XPS_PCL_MAX_NUM_FLDS:
            return "XPS_PCL_MAX_NUM_FLDS";
        default:
            return "UNKNOWN";
    }
}



const char* xpSaiTableAttrNameGet(sai_acl_table_attr_t saiAttr)
{
    switch (saiAttr)
    {
        case SAI_ACL_TABLE_ATTR_ACL_STAGE:
            return "SAI_ACL_TABLE_ATTR_ACL_STAGE";
        case SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST:
            return "SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST";
        case SAI_ACL_TABLE_ATTR_SIZE:
            return "SAI_ACL_TABLE_ATTR_SIZE";
        case SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6:
            return "SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6";
        case SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6:
            return "SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6";
        case SAI_ACL_TABLE_ATTR_FIELD_INNER_SRC_IPV6:
            return "SAI_ACL_TABLE_ATTR_FIELD_INNER_SRC_IPV6";
        case SAI_ACL_TABLE_ATTR_FIELD_INNER_DST_IPV6:
            return "SAI_ACL_TABLE_ATTR_FIELD_INNER_DST_IPV6";
        case SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC:
            return "SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC";
        case SAI_ACL_TABLE_ATTR_FIELD_DST_MAC:
            return "SAI_ACL_TABLE_ATTR_FIELD_DST_MAC";
        case SAI_ACL_TABLE_ATTR_FIELD_SRC_IP:
            return "SAI_ACL_TABLE_ATTR_FIELD_SRC_IP";
        case SAI_ACL_TABLE_ATTR_FIELD_DST_IP:
            return "SAI_ACL_TABLE_ATTR_FIELD_DST_IP";
        case SAI_ACL_TABLE_ATTR_FIELD_INNER_SRC_IP:
            return "SAI_ACL_TABLE_ATTR_FIELD_INNER_SRC_IP";
        case SAI_ACL_TABLE_ATTR_FIELD_INNER_DST_IP:
            return "SAI_ACL_TABLE_ATTR_FIELD_INNER_DST_IP";
        case SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS:
            return "SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS";
        case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS:
            return "SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS";
        case SAI_ACL_TABLE_ATTR_FIELD_IN_PORT:
            return "SAI_ACL_TABLE_ATTR_FIELD_IN_PORT";
        case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT:
            return "SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT";
        case SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT:
            return "SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT";
        case SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID:
            return "SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID";
        case SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI:
            return "SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI";
        case SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI:
            return "SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI";
        case SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID:
            return "SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID";
        case SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI:
            return "SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI";
        case SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI:
            return "SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI";
        case SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT:
            return "SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT";
        case SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT:
            return "SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT";
        case SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE:
            return "SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE";
        case SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL:
            return "SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL";
        case SAI_ACL_TABLE_ATTR_FIELD_IP_IDENTIFICATION:
            return "SAI_ACL_TABLE_ATTR_FIELD_IP_IDENTIFICATION";
        case SAI_ACL_TABLE_ATTR_FIELD_DSCP:
            return "SAI_ACL_TABLE_ATTR_FIELD_DSCP";
        case SAI_ACL_TABLE_ATTR_FIELD_ECN:
            return "SAI_ACL_TABLE_ATTR_FIELD_ECN";
        case SAI_ACL_TABLE_ATTR_FIELD_TTL:
            return "SAI_ACL_TABLE_ATTR_FIELD_TTL";
        case SAI_ACL_TABLE_ATTR_FIELD_TOS:
            return "SAI_ACL_TABLE_ATTR_FIELD_TOS";
        case SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS:
            return "SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS";
        case SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS:
            return "SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS";
        case SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE:
            return "SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE";
        case SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG:
            return "SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG";
        case SAI_ACL_TABLE_ATTR_FIELD_IPV6_FLOW_LABEL:
            return "SAI_ACL_TABLE_ATTR_FIELD_IPV6_FLOW_LABEL";
        case SAI_ACL_TABLE_ATTR_FIELD_TC:
            return "SAI_ACL_TABLE_ATTR_FIELD_TC";
        case SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE:
            return "SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE";
        case SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE:
            return "SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE";
        case SAI_ACL_TABLE_ATTR_FIELD_PACKET_VLAN:
            return "SAI_ACL_TABLE_ATTR_FIELD_PACKET_VLAN";
        case SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_USER_META:
            return "SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_USER_META";
        case SAI_ACL_TABLE_ATTR_FIELD_ROUTE_DST_USER_META:
            return "SAI_ACL_TABLE_ATTR_FIELD_ROUTE_DST_USER_META";
        case SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_USER_META:
            return "SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_USER_META";
        case SAI_ACL_TABLE_ATTR_FIELD_PORT_USER_META:
            return "SAI_ACL_TABLE_ATTR_FIELD_PORT_USER_META";
        case SAI_ACL_TABLE_ATTR_FIELD_VLAN_USER_META:
            return "SAI_ACL_TABLE_ATTR_FIELD_VLAN_USER_META";
        case SAI_ACL_TABLE_ATTR_FIELD_ACL_USER_META:
            return "SAI_ACL_TABLE_ATTR_FIELD_ACL_USER_META";
        case SAI_ACL_TABLE_ATTR_FIELD_FDB_NPU_META_DST_HIT:
            return "SAI_ACL_TABLE_ATTR_FIELD_FDB_NPU_META_DST_HIT";
        case SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT:
            return "SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT";
        case SAI_ACL_TABLE_ATTR_FIELD_ROUTE_NPU_META_DST_HIT:
            return "SAI_ACL_TABLE_ATTR_FIELD_ROUTE_NPU_META_DST_HIT";
        case SAI_ACL_TABLE_ATTR_USER_DEFINED_FIELD_GROUP_MIN:
            return "SAI_ACL_TABLE_ATTR_USER_DEFINED_FIELD_GROUP_MIN";
        case SAI_ACL_TABLE_ATTR_USER_DEFINED_FIELD_GROUP_MAX:
            return "SAI_ACL_TABLE_ATTR_USER_DEFINED_FIELD_GROUP_MAX";
        case SAI_ACL_TABLE_ATTR_FIELD_ACL_RANGE_TYPE:
            return "SAI_ACL_TABLE_ATTR_FIELD_ACL_RANGE_TYPE";
        case SAI_ACL_TABLE_ATTR_FIELD_ICMPV6_TYPE:
            return "SAI_ACL_TABLE_ATTR_FIELD_ICMPV6_TYPE";
        case SAI_ACL_TABLE_ATTR_FIELD_ICMPV6_CODE:
            return "SAI_ACL_TABLE_ATTR_FIELD_ICMPV6_CODE";
        case SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST:
            return "SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST";

        default:
            return "UNKNOWN";
    }

}

const char* xpSaiEntryAttrNameGet(sai_acl_entry_attr_t saiAttr)
{
    switch (saiAttr)
    {
        case SAI_ACL_ENTRY_ATTR_TABLE_ID:
            return "SAI_ACL_ENTRY_ATTR_TABLE_ID";
        case SAI_ACL_ENTRY_ATTR_PRIORITY:
            return "SAI_ACL_ENTRY_ATTR_PRIORITY";
        case SAI_ACL_ENTRY_ATTR_ADMIN_STATE:
            return "SAI_ACL_ENTRY_ATTR_ADMIN_STATE";
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6:
            return "SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6";
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6:
            return "SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6";
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IPV6:
            return "SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IPV6";
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC:
            return "SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC";
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC:
            return "SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC";
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
            return "SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP";
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
            return "SAI_ACL_ENTRY_ATTR_FIELD_DST_IP";
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IP:
            return "SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IP";
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IP:
            return "SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IP";
        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS:
            return "SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS";
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS:
            return "SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS";
        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT:
            return "SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT";
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT:
            return "SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT";
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT:
            return "SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT";
        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID:
            return "SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID";
        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI:
            return "SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI";
        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI:
            return "SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI";
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID:
            return "SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID";
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI:
            return "SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI";
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI:
            return "SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI";
        case SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT:
            return "SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT";
        case SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT:
            return "SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT";
        case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
            return "SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE";
        case SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL:
            return "SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL";
        case SAI_ACL_ENTRY_ATTR_FIELD_IP_IDENTIFICATION:
            return "SAI_ACL_ENTRY_ATTR_FIELD_IP_IDENTIFICATION";
        case SAI_ACL_ENTRY_ATTR_FIELD_DSCP:
            return "SAI_ACL_ENTRY_ATTR_FIELD_DSCP";
        case SAI_ACL_ENTRY_ATTR_FIELD_ECN:
            return "SAI_ACL_ENTRY_ATTR_FIELD_ECN";
        case SAI_ACL_ENTRY_ATTR_FIELD_TTL:
            return "SAI_ACL_ENTRY_ATTR_FIELD_TTL";
        case SAI_ACL_ENTRY_ATTR_FIELD_TOS:
            return "SAI_ACL_ENTRY_ATTR_FIELD_TOS";
        case SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS:
            return "SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS";
        case SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS:
            return "SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS";
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE:
            return "SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE";
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG:
            return "SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG";
        case SAI_ACL_ENTRY_ATTR_FIELD_IPV6_FLOW_LABEL:
            return "SAI_ACL_ENTRY_ATTR_FIELD_IPV6_FLOW_LABEL";
        case SAI_ACL_ENTRY_ATTR_FIELD_TC:
            return "SAI_ACL_ENTRY_ATTR_FIELD_TC";
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE:
            return "SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE";
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE:
            return "SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE";
        case SAI_ACL_ENTRY_ATTR_FIELD_PACKET_VLAN:
            return "SAI_ACL_ENTRY_ATTR_FIELD_PACKET_VLAN";
        case SAI_ACL_ENTRY_ATTR_FIELD_FDB_DST_USER_META:
            return "SAI_ACL_ENTRY_ATTR_FIELD_FDB_DST_USER_META";
        case SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_DST_USER_META:
            return "SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_DST_USER_META";
        case SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_DST_USER_META:
            return "SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_DST_USER_META";
        case SAI_ACL_ENTRY_ATTR_FIELD_PORT_USER_META:
            return "SAI_ACL_ENTRY_ATTR_FIELD_PORT_USER_META";
        case SAI_ACL_ENTRY_ATTR_FIELD_VLAN_USER_META:
            return "SAI_ACL_ENTRY_ATTR_FIELD_VLAN_USER_META";
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_USER_META:
            return "SAI_ACL_ENTRY_ATTR_FIELD_ACL_USER_META";
        case SAI_ACL_ENTRY_ATTR_FIELD_FDB_NPU_META_DST_HIT:
            return "SAI_ACL_ENTRY_ATTR_FIELD_FDB_NPU_META_DST_HIT";
        case SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT:
            return "SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT";
        case SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_NPU_META_DST_HIT:
            return "SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_NPU_META_DST_HIT";
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE:
            return "SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE";
        case SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT:
            return "SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT";
        case SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT_LIST:
            return "SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT_LIST";
        case SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION:
            return "SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION";
        case SAI_ACL_ENTRY_ATTR_ACTION_FLOOD:
            return "SAI_ACL_ENTRY_ATTR_ACTION_FLOOD";
        case SAI_ACL_ENTRY_ATTR_ACTION_COUNTER:
            return "SAI_ACL_ENTRY_ATTR_ACTION_COUNTER";
        case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS:
            return "SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS";
        case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS:
            return "SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER";
        case SAI_ACL_ENTRY_ATTR_ACTION_DECREMENT_TTL:
            return "SAI_ACL_ENTRY_ATTR_ACTION_DECREMENT_TTL";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_TC:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_TC";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_PACKET_COLOR:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_PACKET_COLOR";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_MAC:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_MAC";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_MAC:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_MAC";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IP:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IP";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IP:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IP";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IPV6:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IPV6";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IPV6:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IPV6";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_ECN:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_ECN";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_SRC_PORT:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_SRC_PORT";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_DST_PORT:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_DST_PORT";
        case SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE:
            return "SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE";
        case SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE:
            return "SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE";
        //        case SAI_ACL_ENTRY_ATTR_ACTION_SET_CPU_QUEUE:
        //            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_CPU_QUEUE";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_ACL_META_DATA:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_ACL_META_DATA";
        case SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_BLOCK_PORT_LIST:
            return "SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_BLOCK_PORT_LIST";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_USER_TRAP_ID:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_USER_TRAP_ID";
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_DO_NOT_LEARN:
            return "SAI_ACL_ENTRY_ATTR_ACTION_SET_DO_NOT_LEARN";
        default:
            return "UNKNOWN";
    }

}

static sai_status_t xpSaiAclEntryAttr2XpsConvert(sai_acl_entry_attr_t saiAttr,
                                                 void * value, xpsAclKeyFlds_t *pXpsAttr)
{
    sai_status_t        retVal = SAI_STATUS_SUCCESS;

    switch (saiAttr)
    {
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC:
            *pXpsAttr = XPS_PCL_MAC_SA;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC:
            *pXpsAttr = XPS_PCL_MAC_DA;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
            *pXpsAttr = XPS_PCL_SIP;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
            *pXpsAttr = XPS_PCL_DIP;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID:
        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI:
        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI:
            *pXpsAttr = XPS_PCL_VID1;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID:
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI:
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI:
            *pXpsAttr = XPS_PCL_VID1;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT:
            *pXpsAttr = XPS_PCL_L4_BYTE2;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT:
            *pXpsAttr = XPS_PCL_L4_BYTE0;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
            *pXpsAttr = XPS_PCL_ETHER_TYPE;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL:
            *pXpsAttr = XPS_PCL_IP_PROTOCOL;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT:
            *pXpsAttr = XP_PCL_SRC_PORT;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_DSCP:
        case SAI_ACL_ENTRY_ATTR_FIELD_ECN:
        case SAI_ACL_ENTRY_ATTR_FIELD_TOS:
            *pXpsAttr =  XPS_PCL_DSCP;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_TTL:
            *pXpsAttr =  XPS_PCL_TTL;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS:
            *pXpsAttr =  XPS_PCL_L4_BYTE13;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6:
            *pXpsAttr = XPS_PCL_IPV6_SIP;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6:
            *pXpsAttr = XPS_PCL_IPV6_DIP;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_IPV6_NEXT_HEADER:
            *pXpsAttr = XPS_PCL_IP_PROTOCOL;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE:
            *pXpsAttr = XPS_PCL_ICMP_MSG_TYPE;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE:
            *pXpsAttr = XPS_PCL_ICMP_CODE;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMPV6_TYPE:
            *pXpsAttr = XPS_PCL_ICMPV6_MSG_TYPE;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMPV6_CODE:
            *pXpsAttr = XPS_PCL_ICMPV6_CODE;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG:
            *pXpsAttr = XPS_PCL_IP_FRAGMENT;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_PACKET_VLAN:
            *pXpsAttr = XPS_PCL_IS_TAGGED;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE:

            retVal = xpSaiAclGetXpsIpType(*(sai_acl_ip_type_t *)(value), pXpsAttr);
            if (retVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not convert match field id\n");
                return retVal;
            }
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE:
            *pXpsAttr = XPS_PCL_TCP_UDP_PORT_COMPRATOR;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT:
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT:
        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS:
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS:
            *pXpsAttr = XP_PCL_PORT_BIT_MAP;
            break;
        default:
            XP_SAI_LOG_DBG("Unsupported attribute %d with name %s\n", saiAttr,
                           xpSaiEntryAttrNameGet(saiAttr));
            return SAI_STATUS_SUCCESS;
    }

    return SAI_STATUS_SUCCESS;
}


static sai_status_t xpSaiAclGetXpsIpType(sai_acl_ip_type_t ipType,
                                         xpsAclKeyFlds_t *pXpsAttr)
{
    switch (ipType)
    {

        case SAI_ACL_IP_TYPE_ARP_REQUEST:
            *pXpsAttr = XPS_PCL_IS_ARP_REQUEST;
            break;
        case SAI_ACL_IP_TYPE_ARP_REPLY:
            *pXpsAttr = XPS_PCL_IS_ARP_REPLY;
            break;
        case SAI_ACL_IP_TYPE_ARP:
            *pXpsAttr = XPS_PCL_IS_ARP;
            break;
        case SAI_ACL_IP_TYPE_NON_IP:
            *pXpsAttr = XPS_PCL_IS_NON_IP;
            break;
        case SAI_ACL_IP_TYPE_IPV4ANY:
            *pXpsAttr = XPS_PCL_IS_IPV4;
            break;
        case SAI_ACL_IP_TYPE_ANY:
            *pXpsAttr = XPS_PCL_IS_ANY;
            break;
        case SAI_ACL_IP_TYPE_IP:
            *pXpsAttr = XPS_PCL_IS_IP;
            break;
        case SAI_ACL_IP_TYPE_IPV6ANY:
            *pXpsAttr = XPS_PCL_IS_IPV6;
            break;
        case SAI_ACL_IP_TYPE_NON_IPV4:
            *pXpsAttr = XPS_PCL_IS_NON_IPV4;
            break;
        case SAI_ACL_IP_TYPE_NON_IPV6:
            *pXpsAttr = XPS_PCL_IS_NON_IPV6;
            break;


        default:
            return SAI_STATUS_SUCCESS;// returning SUCCESS to avoid orchagent crash due to unsupported sai_acl_ip_type_t attributes



    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiAclEntryAttrToTableAttrConvert(
    sai_acl_entry_attr_t saiEntryAttr,
    sai_acl_table_attr_t *saiTableAttr)
{
    switch (saiEntryAttr)
    {
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IPV6:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_INNER_SRC_IPV6;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IPV6:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_INNER_DST_IPV6;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_SRC_IP;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_DST_IP;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IP:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_INNER_SRC_IP;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IP:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_INNER_DST_IP;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_IN_PORT;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_IP_IDENTIFICATION:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_IP_IDENTIFICATION;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_DSCP:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_DSCP;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ECN:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_ECN;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_TTL:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_TTL;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_TOS:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_TOS;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_IPV6_FLOW_LABEL:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_IPV6_FLOW_LABEL;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_TC:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_TC;
            break;
        //TODO
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMPV6_TYPE:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_ICMPV6_TYPE;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMPV6_CODE:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_ICMPV6_CODE;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_PACKET_VLAN:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_PACKET_VLAN;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_FDB_DST_USER_META:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_USER_META;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_DST_USER_META:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_USER_META;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_PORT_USER_META:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_PORT_USER_META;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_VLAN_USER_META:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_VLAN_USER_META;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_USER_META:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_ACL_USER_META;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_FDB_NPU_META_DST_HIT:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_FDB_NPU_META_DST_HIT;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT:
            *saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_NPU_META_DST_HIT:
            *saiTableAttr =  SAI_ACL_TABLE_ATTR_FIELD_ROUTE_NPU_META_DST_HIT;
            break;

        default :
            return SAI_STATUS_NOT_SUPPORTED;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t  xpSaiAclValidateEntryAttr(const sai_uint32_t attr_count,
                                        const sai_attribute_t* attr_list,
                                        xpSaiAclTableIdMappingT *pSaiAclTableAttribute)
{
    sai_status_t              retVal = SAI_STATUS_SUCCESS;
    sai_acl_table_attr_t      saiTableAttr = SAI_ACL_TABLE_ATTR_FIELD_END;
    sai_acl_entry_attr_t      saiEntryAttr = SAI_ACL_ENTRY_ATTR_FIELD_END;
    xpSaiAclTableAttributesT  *attributes = NULL;
    attributes = (xpSaiAclTableAttributesT *)xpMalloc(sizeof(
                                                          xpSaiAclTableAttributesT));
    if (!attributes)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for attributes\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(attributes, 0, sizeof(xpSaiAclTableAttributesT));

    XP_SAI_LOG_DBG("Parsing %u attributes in xpSaiAclValidateEntryAttr \n",
                   attr_count);

    memcpy(&(attributes->match), &(pSaiAclTableAttribute->match),
           sizeof((*attributes).match));

    for (sai_uint32_t count = 0; count < attr_count; count++)
    {
        saiEntryAttr = (sai_acl_entry_attr_t)attr_list[count].id;

        if ((saiEntryAttr == SAI_ACL_ENTRY_ATTR_TABLE_ID) ||
            (saiEntryAttr == SAI_ACL_ENTRY_ATTR_PRIORITY) ||
            (saiEntryAttr == SAI_ACL_ENTRY_ATTR_ADMIN_STATE) ||
            (saiEntryAttr == SAI_ACL_ENTRY_ATTR_FIELD_DSCP)
           )
        {
            continue;
        }

        retVal = xpSaiAclEntryAttrToTableAttrConvert(saiEntryAttr, &saiTableAttr);
        if (retVal == SAI_STATUS_SUCCESS)
        {

            if (!xpSaiAclTableAttributeEnabledCheck(attributes, saiTableAttr))
            {
                XP_SAI_LOG_ERR("InValid Attr %s(%d) for acl table \n",
                               xpSaiEntryAttrNameGet(saiEntryAttr),
                               saiEntryAttr);

                xpFree(attributes);
                return SAI_STATUS_INVALID_ATTRIBUTE_0 + SAI_STATUS_CODE(count);
            }
        }
    }
    xpFree(attributes);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAclEntryFieldSet
//
// This function creates new rule on ingress ACL table and sets its match data
// and action data
//
// Parameters:
//     xpSaiAclEntryAttributesT attributes -- (input) incomming attributes array
//     xpsAclKeyType_e type -- (input) entry type
//     xpsAclkeyFieldList_t *fieldData --  (output) match data for new rule
//     xpsPclAction_t *entryData --  (output) action data for new rule.
//     sai_uint32_t tblId  - (input)table Id.
//
// Returns:
//     sai_status_t value


static sai_status_t xpSaiAclEntryFieldSet(xpSaiAclEntryAttributesT* pAttributes,
                                          xpsAclkeyFieldList_t *fieldData,
                                          xpsPclAction_t *entryData,
                                          sai_uint32_t tblId)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if ((fieldData == NULL) || (entryData == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    fieldData->isValid = pAttributes->adminState;

    retVal = xpSaiAclEntryFieldDataSet(pAttributes, fieldData, tblId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not set match fields\n");
        return retVal;
    }

    retVal = xpSaiAclEntryDataSet(pAttributes, entryData, tblId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not set action fields\n");
        return retVal;
    }

    return SAI_STATUS_SUCCESS;
}

void xpSaiKeySizeByteConvert(sai_uint32_t bitKeySize,
                             sai_uint32_t *pByteKeySize)
{
    sai_uint32_t byteKeySize = 0;

    /* Convert the key size to represent size in bits */
    byteKeySize = bitKeySize * 8;

    if (byteKeySize <= 64)
    {
        *pByteKeySize = WIDTH_64;
    }
    else if (byteKeySize > 64 && byteKeySize <= 128)
    {
        *pByteKeySize = WIDTH_128;
    }
    else if (byteKeySize > 128 && byteKeySize <= 192)
    {
        *pByteKeySize = WIDTH_192;
    }
    else
    {
        *pByteKeySize = WIDTH_384;
    }

    XP_SAI_LOG_DBG("Original key size is %d, final key size is %d \n", byteKeySize,
                   *pByteKeySize);
}


//Func: xpSaiSetDefaultAclEntryAttributeVals
void xpSaiSetDefaultAclEntryAttributeVals(xpSaiAclEntryAttributesT* attributes)
{
    /* Assign default table priority */
    attributes->adminState = TRUE;
    attributes->priority = 0;
    attributes->out_ports_count = 0;
    attributes->entryType = SAI_ACL_IP_TYPE_IPV4ANY;
    memset(attributes->match, 0,
           XP_SAI_ACL_MATCH_NUMBER_MAX*sizeof(xpSaiAclEntryFieldData));
    memset(attributes->action, 0,
           XP_SAI_ACL_ACTION_NUMBER_MAX*sizeof(xpSaiAclEntryActionData));
}

//Func: xpSaiAclEntryMatchAttributeSet
sai_status_t xpSaiAclEntryMatchAttributeSet(xpSaiAclEntryAttributesT*
                                            pAttributes, sai_acl_entry_attr_t attr, sai_attribute_value_t value)
{
    sai_uint32_t matchId = attr - SAI_ACL_ENTRY_ATTR_FIELD_START;
    sai_uint32_t countIdx = 0;
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS    xpsRetVal = XP_NO_ERR;
    xpsDevice_t  devId = xpSaiGetDevId();

    if (matchId >= XP_SAI_ACL_MATCH_NUMBER_MAX)
    {
        XP_SAI_LOG_ERR("The attribute %s is out of range\n",
                       xpSaiEntryAttrNameGet(attr));
        return SAI_STATUS_FAILURE;
    }

    // allocate list if not already allocated for object lists
    if (xpSaiValidateEntryListAttributes(attr) &&
        (pAttributes->match[matchId].isSetByUser == FALSE) && (value.aclfield.enable))
    {
        pAttributes->match[matchId].entryValue.enable = value.aclfield.enable;
        pAttributes->match[matchId].entryValue.data.objlist.count =
            value.aclfield.data.objlist.count;

        if ((xpsRetVal = xpsStateHeapMalloc(value.aclfield.data.objlist.count * sizeof(
                                                sai_object_id_t), (void**)
                                            &pAttributes->match[matchId].entryValue.data.objlist.list)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to allocate memory\n");
            return xpsStatus2SaiStatus(xpsRetVal);
        }

        for (countIdx = 0;
             countIdx < pAttributes->match[matchId].entryValue.data.objlist.count;
             countIdx++)
        {

            if ((attr == SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS) ||
                (attr == SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS))
            {
                if (countIdx >= XP_SAI_ACL_PORT_NUMBER_MAX)
                {
                    XP_SAI_LOG_ERR("Out of the max portNum %d.\n", XP_SAI_ACL_PORT_NUMBER_MAX);
                    return SAI_STATUS_FAILURE;
                }
                if (!XDK_SAI_OBJID_TYPE_CHECK(value.aclfield.data.objlist.list[countIdx],
                                              SAI_OBJECT_TYPE_PORT))
                {
                    XP_SAI_LOG_ERR("Unknown object 0x%" PRIx64 "\n",
                                   value.aclfield.data.objlist.list[countIdx]);
                    return SAI_STATUS_INVALID_OBJECT_TYPE;
                }
                xpsInterfaceId_t xpsIntf;
                xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(
                              value.aclfield.data.objlist.list[countIdx]);
                if (attr == SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS)
                {
                    pAttributes->in_ports[countIdx] = xpsGlobalPortToPortnum(devId, xpsIntf);
                    pAttributes->in_ports_count = countIdx + 1;
                }
                else
                {
                    pAttributes->out_ports[countIdx] = xpsGlobalPortToPortnum(devId, xpsIntf);
                    pAttributes->out_ports_count = countIdx + 1;
                }
            }
            pAttributes->match[matchId].entryValue.data.objlist.list[countIdx] =
                value.aclfield.data.objlist.list[countIdx];
        }
    }
    // if User request for already set attribute again, re-allocate the value and assigned new count and data in previously allocated list
    else if ((xpSaiValidateEntryListAttributes(attr) &&
              (pAttributes->match[matchId].isSetByUser == TRUE)))
    {
        if (value.aclfield.enable)
        {
            if (value.aclfield.data.objlist.count !=
                pAttributes->match[matchId].entryValue.data.objlist.count)
            {
                if (pAttributes->match[matchId].entryValue.data.objlist.count)
                {
                    if ((xpsRetVal = xpsStateHeapFree(
                                         pAttributes->match[matchId].entryValue.data.objlist.list)) != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to to free allocated memory\n");
                        return xpsStatus2SaiStatus(xpsRetVal);
                    }
                }
                if ((xpsRetVal = xpsStateHeapMalloc(value.aclfield.data.objlist.count * sizeof(
                                                        sai_object_id_t), (void**)
                                                    &pAttributes->match[matchId].entryValue.data.objlist.list)) != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to allocate memory\n");
                    return xpsStatus2SaiStatus(xpsRetVal);
                }

                pAttributes->match[matchId].entryValue.data.objlist.count =
                    value.aclfield.data.objlist.count;
            }

            for (countIdx = 0;
                 countIdx < pAttributes->match[matchId].entryValue.data.objlist.count;
                 countIdx++)
            {
                if ((attr == SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS) ||
                    (attr == SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS))
                {
                    if (countIdx >= XP_SAI_ACL_PORT_NUMBER_MAX)
                    {
                        XP_SAI_LOG_ERR("Out of the max portNum %d.\n", XP_SAI_ACL_PORT_NUMBER_MAX);
                        return SAI_STATUS_FAILURE;
                    }
                    if (!XDK_SAI_OBJID_TYPE_CHECK(value.aclfield.data.objlist.list[countIdx],
                                                  SAI_OBJECT_TYPE_PORT))
                    {
                        XP_SAI_LOG_ERR("Unknown object 0x%" PRIx64 "\n",
                                       value.aclfield.data.objlist.list[countIdx]);
                        return SAI_STATUS_INVALID_OBJECT_TYPE;
                    }
                    xpsInterfaceId_t xpsIntf;
                    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(
                                  value.aclfield.data.objlist.list[countIdx]);
                    if (attr == SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS)
                    {
                        pAttributes->in_ports[countIdx] = xpsGlobalPortToPortnum(devId, xpsIntf);
                        pAttributes->in_ports_count = countIdx + 1;
                    }
                    else
                    {
                        pAttributes->out_ports[countIdx] = xpsGlobalPortToPortnum(devId, xpsIntf);
                        pAttributes->out_ports_count = countIdx + 1;
                    }
                }
                pAttributes->match[matchId].entryValue.data.objlist.list[countIdx] =
                    value.aclfield.data.objlist.list[countIdx];
            }
            pAttributes->match[matchId].entryValue.enable = value.aclfield.enable;
        }
        else
        {
            if (pAttributes->match[matchId].entryValue.data.objlist.count)
            {
                if ((xpsRetVal = xpsStateHeapFree(
                                     pAttributes->match[matchId].entryValue.data.objlist.list)) != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to to free allocated memory\n");
                    return xpsStatus2SaiStatus(xpsRetVal);
                }
            }

            pAttributes->match[matchId].entryValue.enable = false;
            pAttributes->match[matchId].entryValue.data.objlist.list = NULL;
            pAttributes->match[matchId].entryValue.data.objlist.count = 0;
            if (attr == SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS)
            {
                pAttributes->in_ports_count = 0;
            }
            else if (attr == SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS)
            {
                pAttributes->out_ports_count = 0;
            }
        }
    }
    else if (xpSaiValidateEntryListAttributes(attr))
    {
        return retVal;
    }
    else
    {
        if ((attr == SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT) ||
            (attr == SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT))
        {
            if (value.aclfield.enable)
            {
                if (!XDK_SAI_OBJID_TYPE_CHECK(value.aclfield.data.oid, SAI_OBJECT_TYPE_PORT))
                {
                    XP_SAI_LOG_ERR("Unknown object 0x%" PRIx64 "\n", value.aclfield.data.oid);
                    return SAI_STATUS_INVALID_OBJECT_TYPE;
                }
                xpsInterfaceId_t xpsIntf;
                xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(value.aclfield.data.oid);
                if (attr == SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT)
                {
                    pAttributes->in_ports[0] = xpsGlobalPortToPortnum(devId, xpsIntf);
                    pAttributes->in_ports_count = 1;
                }
                else
                {
                    pAttributes->out_ports[0] = xpsGlobalPortToPortnum(devId, xpsIntf);
                    pAttributes->out_ports_count = 1;
                }
            }
            else
            {
                if (attr == SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT)
                {
                    pAttributes->in_ports_count = 0;
                }
                else
                {
                    pAttributes->out_ports_count = 0;
                }
            }
        }
        pAttributes->match[matchId].entryValue = value.aclfield;
    }

    pAttributes->match[matchId].isSetByUser = TRUE;

    return retVal;
}

//Func: xpSaiAclEntryMatchAttributeEnabledCheck
bool xpSaiAclEntryMatchAttributeEnabledCheck(xpSaiAclEntryAttributesT*
                                             pAttributes, sai_acl_entry_attr_t attr)
{
    sai_uint32_t matchId = attr - SAI_ACL_ENTRY_ATTR_FIELD_START;
    if (matchId >= XP_SAI_ACL_MATCH_NUMBER_MAX)
    {
        XP_SAI_LOG_ERR("The attribute %s is out of range\n",
                       xpSaiEntryAttrNameGet(attr));
        return SAI_STATUS_FAILURE;
    }

    return pAttributes->match[matchId].entryValue.enable;
}

//Func: xpSaiAclEntryMatchAttributeIsSetByUser
bool xpSaiAclEntryMatchAttributeIsSetByUser(xpSaiAclEntryAttributesT*
                                            pAttributes, sai_acl_entry_attr_t attr)
{
    sai_uint32_t matchId = attr - SAI_ACL_ENTRY_ATTR_FIELD_START;

    if (matchId >= XP_SAI_ACL_MATCH_NUMBER_MAX)
    {
        XP_SAI_LOG_ERR("The attribute %s is out of range\n",
                       xpSaiEntryAttrNameGet(attr));
        return SAI_STATUS_FAILURE;
    }

    return pAttributes->match[matchId].isSetByUser;
}

//Func: xpSaiAclEntryMatchAttributeGet
sai_status_t  xpSaiAclEntryMatchAttributeGet(xpSaiAclEntryAttributesT*
                                             pAttributes,
                                             sai_acl_entry_attr_t attr,
                                             sai_acl_field_data_t* match)
{
    sai_uint32_t matchId = attr - SAI_ACL_ENTRY_ATTR_FIELD_START;

    if (matchId >= XP_SAI_ACL_MATCH_NUMBER_MAX)
    {
        XP_SAI_LOG_ERR("The attribute %s is out of range\n",
                       xpSaiEntryAttrNameGet(attr));
        return SAI_STATUS_FAILURE;
    }

    *match = pAttributes->match[matchId].entryValue;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAclEntryMatchAttributeConvert
sai_status_t xpSaiAclEntryMatchAttributeConvert(sai_acl_field_data_t match,
                                                sai_acl_entry_attr_t attr,
                                                xpsAclkeyFieldList_t *pFieldData,
                                                xpsAclKeyFlds_t  fld,
                                                sai_uint32_t *keyIndex,
                                                xpSaiAclEntryAttributesT* pAttributes)
{

    sai_uint32_t  value = 0;
    sai_uint32_t  mask = 0;
    xpsDevice_t  devId = xpSaiGetDevId();
    xpsInterfaceId_t xpsIntf;
    uint32_t portNum;

    switch (attr)
    {
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6:
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memcpy(pFieldData->fldList[*keyIndex].value, &match.data.ip6,
                   sizeof(sai_ip6_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &match.mask.ip6, sizeof(sai_ip6_t));
            (*keyIndex)++;
            //xpSaiIPv6Reverse(value);
            //xpSaiIPv6Reverse(mask);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC:
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memcpy(pFieldData->fldList[*keyIndex].value, &match.data.mac,
                   sizeof(sai_mac_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &match.mask.mac, sizeof(sai_mac_t));
            (*keyIndex)++;
            //xpSaiMacReverse(value);
            //xpSaiMacReverse(mask);
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memcpy(pFieldData->fldList[*keyIndex].value, &match.data.ip4,
                   sizeof(sai_ip4_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &match.mask.ip4, sizeof(sai_ip4_t));
            (*keyIndex)++;
            //xpSaiIPv4ReverseCopy(match.data.ip4, value);
            //xpSaiIPv4ReverseCopy(match.mask.ip4, mask);
            break;

        //supporting only 16 bit so converting into 16.
        case SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT:
            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_L4_BYTE0;
            value = ((match.data.u16 & 0xFF00) >> 8);
            mask = ((match.mask.u16 & 0xFF00) >> 8);
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;

            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_L4_BYTE1;
            value = (match.data.u16 & 0x00FF);
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;

            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_IS_L4_VALID;
            value = 1;
            mask =  1;
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT:
            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_L4_BYTE2;
            value = ((match.data.u16 & 0xFF00) >> 8);
            mask = ((match.mask.u16 & 0xFF00) >> 8);
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;

            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_L4_BYTE3;
            value = (match.data.u16 & 0x00FF);
            mask = (match.mask.u16 & 0x00FF);
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;

            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_IS_L4_VALID;
            value = 1;
            mask =  1;
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;
            break;


        case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
            if (match.data.u16 == 0x800)
            {
                value = 1;
                mask  = 1;
                pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_IS_IPV4;
                memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
                memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
                (*keyIndex)++;
            }
            else if (match.data.u16 == 0x86DD)
            {
                value = 1;
                mask  = 1;
                pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_IS_IPV6;
                memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
                memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
                (*keyIndex)++;
            }
            else if (match.data.u16 == 0x0806)
            {
                value = 1;
                mask  = 1;
                pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_IS_ARP;
                memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
                memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
                (*keyIndex)++;
            }
            else
            {
                /*TODO: do we need to use ntoh */
                pFieldData->fldList[*keyIndex].keyFlds = fld;

                memcpy(pFieldData->fldList[*keyIndex].value, &match.data.u16,
                       sizeof(sai_uint16_t));
                memcpy(pFieldData->fldList[*keyIndex].mask, &match.mask.u16,
                       sizeof(sai_uint16_t));
                (*keyIndex)++;
            }
            break;


        //supporting only 8 bit so converting into 8.
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE:
            value = 1;
            mask  = 1;
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;

            if (fld==XPS_PCL_IS_ARP_REQUEST)
            {
                value = 1;
                mask  = 0xffff;
                pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_ARP_OPCODE;
                memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
                memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
                (*keyIndex)++;
            }
            else if (fld==XPS_PCL_IS_ARP_REPLY)
            {
                value = 2;
                mask = 0xffff;
                pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_ARP_OPCODE;
                memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
                memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
                (*keyIndex)++;
            }


#if 0
            if (fld==XPS_PCL_IS_NON_IPV4 | fld==XPS_PCL_IS_NON_IPV6)
            {
                pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_IS_ARP;
                memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
                memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
                (*keyIndex)++;
            }
#endif
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_PACKET_VLAN:
            if (match.data.u8 == SAI_PACKET_VLAN_SINGLE_OUTER_TAG ||
                match.data.u8 == SAI_PACKET_VLAN_DOUBLE_TAG)
            {
                value = 1;
                mask  = 1;
            }
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_IPV6_NEXT_HEADER:
        case SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL:
        case SAI_ACL_ENTRY_ATTR_FIELD_DSCP:
        case SAI_ACL_ENTRY_ATTR_FIELD_TOS:
        case SAI_ACL_ENTRY_ATTR_FIELD_ECN:
        case SAI_ACL_ENTRY_ATTR_FIELD_TTL:
        case SAI_ACL_ENTRY_ATTR_FIELD_TC:
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memcpy(pFieldData->fldList[*keyIndex].value, &match.data.u8,
                   sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &match.mask.u8,
                   sizeof(sai_uint8_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS:
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE:
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE:
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMPV6_TYPE:
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMPV6_CODE:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memcpy(pFieldData->fldList[*keyIndex].value, &match.data.u8,
                   sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &match.mask.u8,
                   sizeof(sai_uint8_t));
            (*keyIndex)++;

            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_IS_L4_VALID;
            value = 1;
            mask =  1;
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID:
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memcpy(pFieldData->fldList[*keyIndex].value, &match.data.u32,
                   sizeof(sai_uint32_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &match.mask.u32,
                   sizeof(sai_uint32_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI:
        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memcpy(pFieldData->fldList[*keyIndex].value, &match.data.u32,
                   sizeof(sai_uint32_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &match.mask.u32,
                   sizeof(sai_uint32_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI:
        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memcpy(pFieldData->fldList[*keyIndex].value, &match.data.u32,
                   sizeof(sai_uint32_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &match.mask.u32,
                   sizeof(sai_uint32_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT:
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT:
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            xpsIntf = (sai_uint32_t)xpSaiObjIdValueGet(match.data.oid);
            portNum = xpsGlobalPortToPortnum(devId, xpsIntf);
            pFieldData->fldList[*keyIndex].value[portNum/8] |= 1 << (portNum%8);
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS:
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            for (uint16_t i = 0; i < match.data.objlist.count; i++)
            {
                xpsIntf = (sai_uint32_t)xpSaiObjIdValueGet(match.data.objlist.list[i]);
                portNum = xpsGlobalPortToPortnum(devId, xpsIntf);
                pFieldData->fldList[*keyIndex].value[portNum/8] |= 1 << (portNum%8);
            }
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE:
            /*
                        sai_status_t saiRetVal;
                        sai_uint8_t index;

                        pFieldData->fldList[*keyIndex].keyFlds = fld;
                        saiRetVal = xpSaiAclGetComparatorIndex(pAttributes, &index);
                        if(saiRetVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("Failed to get comparator Index %d\n", saiRetVal);
                            return saiRetVal;
                        }
                        value = 0x03 << index;
                        mask = 0x03 << index;
                        memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
                        memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
                        (*keyIndex)++;

                        pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_IS_L4_VALID;
                        value = 1;
                        mask =  1;
                        memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
                        memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
                        (*keyIndex)++;*/
            break;
        default:
            XP_SAI_LOG_ERR("Can not convert value of attribute %s\n",
                           xpSaiEntryAttrNameGet(attr));
            return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAclEntryUnMatchAttributeConvert
sai_status_t xpSaiAclEntryUnMatchAttributeConvert(sai_acl_entry_attr_t attr,
                                                  xpsAclkeyFieldList_t *pFieldData,
                                                  xpsAclKeyFlds_t  fld,
                                                  sai_uint32_t *keyIndex)

{
    switch (attr)
    {
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6:
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memset(pFieldData->fldList[*keyIndex].mask, 0x0, sizeof(sai_ip6_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC:
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memset(pFieldData->fldList[*keyIndex].mask, 0x0, sizeof(sai_mac_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
        case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memset(pFieldData->fldList[*keyIndex].mask, 0x0, sizeof(sai_ip4_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT:
            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_L4_BYTE0;
            memset(pFieldData->fldList[*keyIndex].mask, 0x0, sizeof(sai_uint8_t));
            (*keyIndex)++;

            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_L4_BYTE1;
            memset(pFieldData->fldList[*keyIndex].mask, 0x0, sizeof(sai_uint8_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT:
            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_L4_BYTE2;
            memset(pFieldData->fldList[*keyIndex].mask, 0x0, sizeof(sai_uint8_t));
            (*keyIndex)++;

            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_L4_BYTE3;
            memset(pFieldData->fldList[*keyIndex].mask, 0x0, sizeof(sai_uint8_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memset(pFieldData->fldList[*keyIndex].mask, 0x0, sizeof(sai_uint16_t));
            (*keyIndex)++;
            break;

        //supporting only 8 bit so converting into 8.
        case SAI_ACL_ENTRY_ATTR_FIELD_IPV6_NEXT_HEADER:
        case SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL:
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE:
        case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE:
        case SAI_ACL_ENTRY_ATTR_FIELD_DSCP:
        case SAI_ACL_ENTRY_ATTR_FIELD_TOS:
        case SAI_ACL_ENTRY_ATTR_FIELD_ECN:
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE:
        case SAI_ACL_ENTRY_ATTR_FIELD_TTL:
        case SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS:
        case SAI_ACL_ENTRY_ATTR_FIELD_TC:
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memset(pFieldData->fldList[*keyIndex].mask, 0x0, sizeof(sai_uint8_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID:
        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memset(pFieldData->fldList[*keyIndex].mask, 0x0, sizeof(sai_uint16_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI:
        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memset(pFieldData->fldList[*keyIndex].mask, 0x0, sizeof(sai_uint8_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI:
        case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memset(pFieldData->fldList[*keyIndex].mask, 0x0, sizeof(sai_uint8_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT:
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT:
        case SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memset(pFieldData->fldList[*keyIndex].mask, 0x0, sizeof(sai_uint16_t));
            (*keyIndex)++;
            break;
        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS:
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS:
            pFieldData->fldList[*keyIndex].keyFlds = fld;
            memset(pFieldData->fldList[*keyIndex].mask, 0x0,
                   sizeof(sai_uint8_t)*(SYSTEM_MAX_PORT/8));
            (*keyIndex)++;
            break;

        default:
            XP_SAI_LOG_ERR("Can not convert value of attribute %s\n",
                           xpSaiEntryAttrNameGet(attr));
            return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAclEntryActionAttributeSet
sai_status_t xpSaiAclEntryActionAttributeSet(xpSaiAclEntryAttributesT*
                                             pAttributes, sai_acl_entry_attr_t attr, sai_attribute_value_t value)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_uint32_t countIdx = 0;
    sai_uint32_t actionId = attr - SAI_ACL_ENTRY_ATTR_ACTION_START;

    if (actionId >= XP_SAI_ACL_ACTION_NUMBER_MAX)
    {
        XP_SAI_LOG_ERR("The attribute is out of range\n");
        return SAI_STATUS_FAILURE;
    }

    XP_SAI_LOG_DBG("Set attribute %s\n", xpSaiEntryAttrNameGet(attr));

    // allocate list if not already allocated for object lists
    if (xpSaiValidateEntryListAttributes(attr) &&
        (pAttributes->action[actionId].isSetByUser == FALSE) &&
        (value.aclaction.enable))
    {
        pAttributes->action[actionId].actionValue.enable = value.aclaction.enable;
        pAttributes->action[actionId].actionValue.parameter.objlist.count =
            value.aclaction.parameter.objlist.count;

        if ((retVal = xpsStateHeapMalloc(value.aclaction.parameter.objlist.count *
                                         sizeof(sai_object_id_t),
                                         (void**)&pAttributes->action[actionId].actionValue.parameter.objlist.list)) !=
            XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to allocate memory\n");
            return xpsStatus2SaiStatus(retVal);
        }

        for (countIdx = 0;
             countIdx < pAttributes->action[actionId].actionValue.parameter.objlist.count;
             countIdx++)
        {
            pAttributes->action[actionId].actionValue.parameter.objlist.list[countIdx] =
                value.aclaction.parameter.objlist.list[countIdx];
        }
    }
    // if User request for already set attribute again, re-allocate the value and assigned new count and data in previously allocated list
    else if ((xpSaiValidateEntryListAttributes(attr) &&
              (pAttributes->action[actionId].isSetByUser == TRUE)))
    {
        if (value.aclaction.enable)
        {
            if (value.aclaction.parameter.objlist.count !=
                pAttributes->action[actionId].actionValue.parameter.objlist.count)
            {
                if (pAttributes->action[actionId].actionValue.parameter.objlist.count)
                {
                    if ((retVal = xpsStateHeapFree(
                                      pAttributes->action[actionId].actionValue.parameter.objlist.list)) != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to to free allocated memory\n");
                        return xpsStatus2SaiStatus(retVal);
                    }
                }

                if ((retVal = xpsStateHeapMalloc(value.aclaction.parameter.objlist.count *
                                                 sizeof(sai_object_id_t),
                                                 (void**)&pAttributes->action[actionId].actionValue.parameter.objlist.list)) !=
                    XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to allocate memory\n");
                    return xpsStatus2SaiStatus(retVal);
                }

                pAttributes->action[actionId].actionValue.parameter.objlist.count =
                    value.aclaction.parameter.objlist.count;
            }

            for (countIdx = 0;
                 countIdx < pAttributes->action[actionId].actionValue.parameter.objlist.count;
                 countIdx++)
            {
                pAttributes->action[actionId].actionValue.parameter.objlist.list[countIdx] =
                    value.aclaction.parameter.objlist.list[countIdx];
            }
            pAttributes->action[actionId].actionValue.enable = value.aclaction.enable;
        }
        else
        {
            if (pAttributes->action[actionId].actionValue.parameter.objlist.list != NULL)
            {
                if ((retVal = xpsStateHeapFree(
                                  pAttributes->action[actionId].actionValue.parameter.objlist.list)) != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to to free allocated memory\n");
                    return xpsStatus2SaiStatus(retVal);
                }
            }

            pAttributes->action[actionId].actionValue.enable = false;
            pAttributes->action[actionId].actionValue.parameter.objlist.list = NULL;
            pAttributes->action[actionId].actionValue.parameter.objlist.count = 0;
        }
    }
    else
    {
        pAttributes->action[actionId].actionValue = value.aclaction;
    }

    pAttributes->action[actionId].isSetByUser = TRUE;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAclEntryActionAttributeEnabledCheck
bool xpSaiAclEntryActionAttributeEnabledCheck(xpSaiAclEntryAttributesT*
                                              pAttributes, sai_acl_entry_attr_t attr)
{
    sai_uint32_t actionId = attr - SAI_ACL_ENTRY_ATTR_ACTION_START;

    if (actionId >= XP_SAI_ACL_ACTION_NUMBER_MAX)
    {
        XP_SAI_LOG_ERR("The attribute is out of range\n");
        return SAI_STATUS_FAILURE;
    }

    return pAttributes->action[actionId].actionValue.enable;
}
//Func: xpSaiAclEntryActionAttributeIsSetByUser
bool xpSaiAclEntryActionAttributeIsSetByUser(xpSaiAclEntryAttributesT*
                                             pAttributes, sai_acl_entry_attr_t attr)
{
    sai_uint32_t actionId = attr - SAI_ACL_ENTRY_ATTR_ACTION_START;

    if (actionId >= XP_SAI_ACL_ACTION_NUMBER_MAX)
    {
        XP_SAI_LOG_ERR("The attribute is out of range\n");
        return SAI_STATUS_FAILURE;
    }

    return pAttributes->action[actionId].isSetByUser;
}

//Func: xpSaiAclEntryActionAttributeGet
sai_status_t xpSaiAclEntryActionAttributeGet(xpSaiAclEntryAttributesT*
                                             pAttributes, sai_acl_entry_attr_t attr, sai_acl_action_data_t* pAction)
{
    sai_uint32_t actionId = attr - SAI_ACL_ENTRY_ATTR_ACTION_START;

    if (actionId >= XP_SAI_ACL_ACTION_NUMBER_MAX)
    {
        XP_SAI_LOG_ERR("The attribute is out of range\n");
        return SAI_STATUS_FAILURE;
    }

    *pAction = pAttributes->action[actionId].actionValue;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAclEntryAttributeUpdate
sai_status_t xpSaiAclEntryAttributeUpdate(const sai_attribute_t* pAttr,
                                          xpSaiAclEntryAttributesT* pAttributes)
{
    sai_status_t            retVal           = SAI_STATUS_SUCCESS;
    xpSaiAclEntry_t         xpSaiAclEntryVar;
    sai_uint32_t            count;

    XP_SAI_LOG_DBG("attr %u, name %s\n", pAttr->id,
                   xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)pAttr->id));

    // Parse match attributes
    if ((pAttr->id >= SAI_ACL_ENTRY_ATTR_FIELD_START) &&
        (pAttr->id <= SAI_ACL_ENTRY_ATTR_FIELD_END))
    {
        switch (pAttr->id)
        {
            case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE:
                {
                    pAttributes->entryType = (sai_acl_ip_type_t)pAttr->value.aclfield.data.s32;
                    pAttributes->isEntryTypeValid = true;
                    retVal = xpSaiAclEntryMatchAttributeSet(pAttributes,
                                                            (sai_acl_entry_attr_t)pAttr->id, pAttr->value);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to set attribute %s(%d)\n",
                                       xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)pAttr->id),
                                       pAttr->id);
                        return SAI_STATUS_INVALID_ATTRIBUTE_0;
                    }
                    break;
                }
            case SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE:
                {
                    count = pAttr->value.aclfield.data.objlist.count;
                    pAttributes->aclRangeObjList.list =(sai_object_id_t *)xpMalloc(sizeof(
                                                                                       sai_object_id_t) * count);
                    pAttributes->aclRangeObjList.count= count;
                    memcpy(pAttributes->aclRangeObjList.list,
                           pAttr->value.aclfield.data.objlist.list, sizeof(sai_object_id_t)*count);
                    /*   retVal = xpSaiAclSetXpsComparator(pAttributes);
                       if (retVal != SAI_STATUS_SUCCESS)
                       {
                           XP_SAI_LOG_ERR ("Failed to set comparator %s(%d)\n");
                           xpFree(pAttributes->aclRangeObjList.list);
                           return SAI_STATUS_INVALID_ATTRIBUTE_0;
                       }
                    */
                    retVal = xpSaiAclEntryMatchAttributeSet(pAttributes,
                                                            (sai_acl_entry_attr_t)pAttr->id, pAttr->value);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to set attribute %s(%d)\n",
                                       xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)pAttr->id),
                                       pAttr->id);
                        xpFree(pAttributes->aclRangeObjList.list);
                        return SAI_STATUS_INVALID_ATTRIBUTE_0;
                    }
                    break;
                }

            default:
                {
                    retVal = xpSaiAclEntryMatchAttributeSet(pAttributes,
                                                            (sai_acl_entry_attr_t)pAttr->id, pAttr->value);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to set attribute %s(%d)\n",
                                       xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)pAttr->id),
                                       pAttr->id);
                        return SAI_STATUS_INVALID_ATTRIBUTE_0;
                    }
                }
        }
    }
    // Parse action attributes
    else if ((pAttr->id >= SAI_ACL_ENTRY_ATTR_ACTION_START) &&
             (pAttr->id <= SAI_ACL_ENTRY_ATTR_ACTION_END))
    {
        retVal = xpSaiAclEntryActionAttributeSet(pAttributes,
                                                 (sai_acl_entry_attr_t)pAttr->id, pAttr->value);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to set attribute %s(%d)\n",
                           xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)pAttr->id),
                           pAttr->id);
            return SAI_STATUS_INVALID_ATTRIBUTE_0;
        }
    }
    // Parse basic attributes
    else
    {
        switch (pAttr->id)
        {
            case SAI_ACL_ENTRY_ATTR_TABLE_ID:
                {
                    retVal = xpSaiConvertAclTableOid(pAttr->value.oid, &pAttributes->tableId);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to convert ACL table oid");
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }

                    pAttributes->tableOid = pAttr->value.oid;
                    break;
                }
            case SAI_ACL_ENTRY_ATTR_PRIORITY:
                {
                    retVal = xpSaiAclStaticVariablesGet(&xpSaiAclEntryVar);
                    if (SAI_STATUS_SUCCESS != retVal)
                    {
                        XP_SAI_LOG_ERR("Failed to get SAI ACL related attributes\n");
                        return retVal;
                    }

                    if ((pAttr->value.u32 < xpSaiAclEntryVar.saiAclEntryMinimumPriority) ||
                        (pAttr->value.u32 > xpSaiAclEntryVar.saiAclEntryMaximumPriority))
                    {
                        XP_SAI_LOG_ERR("Entry priority is out of range %u - %u\n",
                                       xpSaiAclEntryVar.saiAclEntryMinimumPriority,
                                       xpSaiAclEntryVar.saiAclEntryMaximumPriority);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }

                    pAttributes->priority = pAttr->value.u32;

                    break;
                }
            case SAI_ACL_ENTRY_ATTR_ADMIN_STATE:
                {
                    pAttributes->adminState = pAttr->value.booldata;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set attribute %s(%d)\n",
                                   xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)pAttr->id),
                                   pAttr->id);
                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAclEntryAttributesUpdate
sai_status_t xpSaiAclEntryAttributesUpdate(const sai_uint32_t attr_count,
                                           const sai_attribute_t* attr_list, xpSaiAclEntryAttributesT* pAttributes)
{
    sai_status_t        retVal           = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Parsing %u attributes\n", attr_count);

    for (sai_uint32_t count = 0; count < attr_count; count++)
    {
        retVal = xpSaiAclEntryAttributeUpdate(attr_list + count, pAttributes);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to set attribute %s(%d)\n",
                           xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)attr_list[count].id),
                           attr_list[count].id);
            return (retVal + SAI_STATUS_CODE(count));
        }
    }

    xpSaiAclEntryAttributesPrint(pAttributes);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAclEntryAttributesApply
sai_status_t xpSaiAclEntryCounterApply(sai_uint32_t devId, sai_uint32_t entryId,
                                       xpSaiAclEntryAttributesT* pAttributes)
{
    sai_status_t            retVal     = SAI_STATUS_SUCCESS;
    sai_uint32_t            counterId  = 0;
    sai_acl_action_data_t   action;

    if (!xpSaiAclEntryActionAttributeEnabledCheck(pAttributes,
                                                  SAI_ACL_ENTRY_ATTR_ACTION_COUNTER))
    {
        XP_SAI_LOG_DBG("No counter id set, entry %u\n", entryId);
        return SAI_STATUS_SUCCESS;
    }

    xpSaiAclEntryActionAttributeGet(pAttributes, SAI_ACL_ENTRY_ATTR_ACTION_COUNTER,
                                    &action);

    retVal = xpSaiConvertAclCounterOid(action.parameter.oid, &counterId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        if (retVal == SAI_STATUS_INVALID_OBJECT_TYPE)
        {
            // oid is passed as an attribute value
            XP_SAI_LOG_ERR("Invalid value of oid provided in attribute\n");
            return SAI_STATUS_INVALID_ATTR_VALUE_0;
        }
        else
        {
            return retVal;
        }
    }

    if (xpSaiAclMapperAddEntryInCounter(counterId, entryId) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to update counter %u for entry %u \n", counterId,
                       entryId);
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetDefaultAclCounterAttributeVals

void xpSaiSetDefaultAclCounterAttributeVals(xpSaiAclCounterAttributesT*
                                            attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiSetDefaultAclCounterAttributeVals\n");

    memset(attributes, 0, sizeof(xpSaiAclCounterAttributesT));
    //ToDO SEt the packet and byte enable to false.
}

//Func: xpSaiUpdateAclCounterAttributeVals

sai_status_t xpSaiUpdateAclCounterAttributeVals(const sai_uint32_t attr_count,
                                                const sai_attribute_t* attr_list, xpSaiAclCounterAttributesT* attributes)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiUpdateAclCounterAttributeVals\n");

    for (sai_uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_ACL_COUNTER_ATTR_TABLE_ID:
                {
                    attributes->tableOid = attr_list[count].value.oid;

                    retVal = xpSaiConvertAclTableOid(attributes->tableOid, &attributes->tableId);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to convert ACL table oid");
                        return (SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count));
                    }

                    attributes->table_valid = true;

                    break;
                }
            case SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT:
                {
                    attributes->enablePacketCount = attr_list[count].value.booldata;
                    break;
                }
            case SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT:
                {
                    attributes->enableByteCount = attr_list[count].value.booldata;
                    break;
                }
            case SAI_ACL_COUNTER_ATTR_PACKETS:
                {
                    attributes->packets = attr_list[count].value.u64;
                    break;
                }
            case SAI_ACL_COUNTER_ATTR_BYTES:
                {
                    attributes->bytes = attr_list[count].value.u64;
                    break;
                }

            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                    return (SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(count));
                }
        }
    }

    if (attributes->table_valid == false)
    {
        XP_SAI_LOG_ERR("SAI_ACL_COUNTER_ATTR_TABLE_ID is missing");
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiUpdateAttrListAclCounterVals

void xpSaiUpdateAttrListAclCounterVals(xpSaiAclCounterAttributesT attributes,
                                       const sai_uint32_t attr_count, sai_attribute_t* attr_list)
{
    XP_SAI_LOG_DBG("Calling xpSaiUpdateAttrListAclCounterVals\n");
    for (sai_uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_ACL_COUNTER_ATTR_TABLE_ID:
                {
                    attr_list[count].value.oid = attributes.tableOid;
                    break;
                }
            case SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT:
                {
                    attr_list[count].value.booldata = attributes.enablePacketCount;
                    break;
                }
            /* Counter work with both together packet and byte count,
              so added this*/
            case SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT:
                {
                    /* Byte count is not supported */
                    attr_list[count].value.booldata = false;
                    break;
                }
            case SAI_ACL_COUNTER_ATTR_PACKETS:
                {
                    attr_list[count].value.u64 = attributes.packets;
                    break;
                }
            case SAI_ACL_COUNTER_ATTR_BYTES:
                {
                    attr_list[count].value.u64 = attributes.bytes;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }

    }

}

//Func: xpSaiCreateAclTable
/* Code flow for flex key implementation
 *  1. Identify unique entries which we will need for preparing a new key to write it in SE reg
 *  2. Compute the key size required for new Create table(may be we need to add the mapping of keyfield Vs length some where in our code)
 *  3. Call deleteTable/Create table with required profile(xpAclTableProfile_t   -> keySize, numDb)
 *  4. Program the SE command with appropriate keyFieldData_t and xpAclLookupType
 *        We need to call this API for Ipv4 IACL type only as we dont have any TC for Ipv6 type.
 *        We need to call this API for each IACL lookup type(PACL, VACL, RACL).
 *  5. Fetch the data fields info from config struct and call xpAclMgr::setAclData()
 *  6. Call setAclEntry() for each test case in appTest.
 *        Which will have the key and mask values for required key fields.
 */
sai_status_t xpSaiCreateAclTable(sai_object_id_t *acl_table_id,
                                 sai_object_id_t switch_id,
                                 sai_uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t    tableId     = 0;
    sai_uint32_t    devId       = 0;
    xpAclTableInfo_t            tableInfo;
    xpSaiAclTableIdMappingT    *keyFormat = NULL;
    XP_STATUS                   retVal = XP_NO_ERR;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;


    xpSaiAclTableAttributesT *attributes = NULL;
    attributes = (xpSaiAclTableAttributesT *)xpMalloc(sizeof(
                                                          xpSaiAclTableAttributesT));
    if (!attributes)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for attributes\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(switch_id, SAI_OBJECT_TYPE_SWITCH))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(switch_id));
        xpFree(attributes);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    devId = (sai_uint32_t)xpSaiObjIdValueGet(switch_id);

    memset(attributes, 0x00, sizeof(xpSaiAclTableAttributesT));

    if (acl_table_id == NULL)
    {
        XP_SAI_LOG_ERR("Invalid parameter \n");
        xpFree(attributes);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               ACL_TABLE_VALIDATION_ARRAY_SIZE, acl_table_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        xpFree(attributes);
        return saiRetVal;
    }

    xpSaiSetDefaultAclTableAttributeVals(attributes);

    saiRetVal = xpSaiUpdateAclTableAttributeVals(attr_count, attr_list, attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to parse attributes %u \n", saiRetVal);
        xpFree(attributes);
        return saiRetVal;
    }

    if ((saiRetVal = xpSaiAclTableTypeSet(attributes)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to determine ACL key type %d \n", saiRetVal);
        xpFree(attributes);
        return saiRetVal;
    }

    // Creating Tcam table
    memset(&tableInfo, 0x00, sizeof(xpAclTableInfo_t));
    tableInfo.stage = (xpsAclStage_e) attributes->stage;
    tableInfo.vTcamInfo.clientGroup = 0;
    tableInfo.vTcamInfo.hitNumber = 0;
    tableInfo.vTcamInfo.ruleSize = XPS_VIRTUAL_TCAM_RULE_SIZE_STD_E;
    tableInfo.vTcamInfo.autoResize = true;
    /* guaranteedNumOfRules is applicable only for legacy ACL design.
     * Not used in falcon */
    tableInfo.vTcamInfo.guaranteedNumOfRules = 64;
    tableInfo.vTcamInfo.ruleAdditionMethod =
        XPS_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;
    tableInfo.isMirror = attributes->isMirror;

    if (attributes->keyType == XP_SAI_KEY_NON_IP)
    {
        tableInfo.vTcamInfo.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        tableInfo.tableType = XPS_ACL_TABLE_TYPE_NON_IP;
    }
    else if (attributes->keyType == XP_SAI_KEY_IPV4)
    {
        tableInfo.vTcamInfo.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E;
        tableInfo.tableType = XPS_ACL_TABLE_TYPE_IPV4;
    }
    else if (attributes->keyType == XP_SAI_KEY_IPV4_AND_IPV6)
    {
        tableInfo.vTcamInfo.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
        tableInfo.tableType = XPS_ACL_TABLE_TYPE_IPV4_IPV6;
    }
    else
    {
        tableInfo.vTcamInfo.ruleSize = XPS_VIRTUAL_TCAM_RULE_SIZE_EXT_E;
        tableInfo.vTcamInfo.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E;
        tableInfo.tableType = XPS_ACL_TABLE_TYPE_IPV6;
    }

    retVal = xpsAclCreateTable(devId, tableInfo, &tableId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to create table for the device %u \n", devId);
        xpFree(attributes);
        return retVal;
    }

    XP_SAI_LOG_NOTICE("ACL Table created %d isMirror %d stage %d keytype %d bpcount %d\n",
                      tableId, attributes->isMirror,
                      attributes->stage, attributes->keyType, attributes->bindPointCount);

    retVal = xpsStateHeapMalloc(sizeof(xpSaiAclTableIdMappingT),
                                (void**)&keyFormat);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to allocate memory for the device %u \n", devId);
        xpFree(attributes);
        return xpsStatus2SaiStatus(retVal);
    }


    memset(keyFormat, 0x00, sizeof(xpSaiAclTableIdMappingT));
    keyFormat->tableId = tableId;
    keyFormat->stage = attributes->stage;
    //TODO now consider as 512
    keyFormat->tableSize = 512;
    keyFormat->priority = 0;
    keyFormat->keyType = attributes->keyType;
    keyFormat->bindPointCount = attributes->bindPointCount;
    keyFormat->isMirror = attributes->isMirror;
    memcpy(keyFormat->match, attributes->match,
           (XP_SAI_ACL_TABLE_MATCH_NUMBER_MAX));
    memcpy(keyFormat->bp_list, attributes->bp_list,
           sizeof(sai_uint32_t)*attributes->bindPointCount);
    /* Insert SAI ACL table structure into DB */
    saiRetVal = xpSaiAclMapperInsertTableId(devId, keyFormat);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to add table Id DB to the device %u \n", devId);
        xpsStateHeapFree((void*)keyFormat);
        xpFree(attributes);
        return saiRetVal;
    }


    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ACL_TABLE, devId,
                                 (sai_uint64_t) tableId, acl_table_id);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, error %d\n", saiRetVal);
        xpFree(attributes);
        return saiRetVal;
    }

    saiRetVal = xpSaiGetTableEntryCountCtxDb(devId, &entryCountCtxPtr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB, retVal = %d \n", saiRetVal);
        xpFree(attributes);
        return saiRetVal;
    }
    if (entryCountCtxPtr)
    {
        entryCountCtxPtr->aclTables++;
    }

    XP_SAI_LOG_DBG("Created a table, id %d, oid 0x%" PRIx64 "\n", tableId,
                   *acl_table_id);
    xpFree(attributes);
    return saiRetVal;
}

//Func: xpSaiRemoveAclTable

sai_status_t xpSaiRemoveAclTable(sai_object_id_t acl_table_id)
{
    sai_status_t             retVal = SAI_STATUS_SUCCESS;
    sai_uint32_t             tableId     = 0;
    sai_uint32_t             devId     = 0;
    xpSaiAclTableIdMappingT  *pSaiAclTableAttribute = NULL;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;


    retVal = xpSaiConvertAclTableOid(acl_table_id, &tableId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to convert ACL table oid");
        return retVal;
    }

    /* Get table info */
    if ((retVal = xpSaiAclTableAttributesGet(tableId,
                                             &pSaiAclTableAttribute)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:, Failed to get table %u related data\n", __FUNCNAME__,
                       tableId);
        return retVal;
    }

    if (pSaiAclTableAttribute->numEntries != 0)
    {
        XP_SAI_LOG_ERR("%s:, Delete all entries in table %u before deleting table\n",
                       __FUNCNAME__, tableId);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    if ((retVal = xpsAclDeleteTable(devId, tableId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("%s:, fail to delete table\n", __FUNCNAME__, tableId);
        return retVal;
    }

    /* Insert SAI ACL table structure into DB */
    retVal = xpSaiAclMapperRemoveTableId(devId, tableId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to remove table Id DB to the device %u \n",
                       devId);
        return retVal;
    }

    retVal = xpSaiGetTableEntryCountCtxDb(devId, &entryCountCtxPtr);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB, retVal = %d \n", retVal);
        return retVal;
    }
    if (entryCountCtxPtr)
    {
        entryCountCtxPtr->aclTables--;
    }
    return retVal;
}


//Func: xpSaiSetAclTableAttribute

sai_status_t xpSaiSetAclTableAttribute(sai_object_id_t acl_table_id,
                                       const sai_attribute_t *attr)
{
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    XP_SAI_LOG_ERR("The attribute %d cannot be changed after the table group has been created\n",
                   attr->id);
    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetAclTableAttributes

static sai_status_t xpSaiGetAclTableAttributes(sai_object_id_t acl_table_id,
                                               sai_uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    sai_uint32_t tableId = 0;
    retVal = xpSaiConvertAclTableOid(acl_table_id, &tableId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to convert ACL table oid");
        return retVal;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            ACL_TABLE_VALIDATION_ARRAY_SIZE, acl_table_attribs,
                            SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }
    /* Get table attributes from mapper */
    if ((retVal = xpSaiGetAclMapperTableAttributes(tableId, attr_count,
                                                   attr_list)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get table %u related data\n", tableId);
        return retVal;
    }

    return retVal;
}

//Func: xpSaiBulkGetAclTableAttributes

sai_status_t xpSaiBulkGetAclTableAttributes(sai_object_id_t id,
                                            sai_uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal  = SAI_STATUS_SUCCESS;
    sai_uint32_t tableId = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiConvertAclTableOid(id, &tableId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to convert ACL table oid");
        return saiRetVal;
    }

    saiRetVal = xpSaiBulkGetAclMapperTableAttributes(tableId, attr_count,
                                                     attr_list);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiBulkGetAclMapperTableAttributes failed with retVal: %d",
                       saiRetVal);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetACLTableAttrStage

static sai_status_t xpSaiGetACLTableAttrStage(sai_object_id_t tableId,
                                              sai_acl_stage_t *stage)
{
    sai_status_t    saiRetVal = SAI_STATUS_SUCCESS;
    sai_attribute_t attrs[1]  = {{0}};

    attrs[0].id = SAI_ACL_TABLE_ATTR_ACL_STAGE;

    saiRetVal = xpSaiGetAclTableAttributes(tableId, 1, attrs);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get table %u stage\n", tableId);
    }
    *stage = (sai_acl_stage_t)(attrs[0].value.u32);
    return saiRetVal;
}

//Func: xpSaiGetACLTableGrpTableIdList

sai_status_t xpSaiGetACLTableGrpTableIdList(sai_object_id_t groupId,
                                            sai_uint32_t *count, sai_uint32_t *tableIdList)
{
    sai_status_t saiRetVal  = SAI_STATUS_SUCCESS;

    saiRetVal = xpSaiGetACLMapperTableGrpTableIdList(groupId, count, tableIdList);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetACLMapperTableGrpTableIdList failed with saiRetVal: %d",
                       saiRetVal);
    }
    return saiRetVal;
}

//Func: xpSaiSetDefaultAclTableGrpAttributeVals

void xpSaiSetDefaultAclTableGrpAttributeVals(xpSaiAclTableGrpAttributesT*
                                             attributes)
{
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    attributes->type = SAI_ACL_TABLE_GROUP_TYPE_SEQUENTIAL;
}

//Func: xpSaiUpdateAclTableGrpAttributeVals

static sai_status_t xpSaiUpdateAclTableGrpAttributeVals(
    const sai_uint32_t attr_count, const sai_attribute_t* attr_list,
    xpSaiAclTableGrpAttributesT* attributes)
{
    for (sai_uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE:
                {
                    attributes->stage = (sai_acl_stage_t)(attr_list[count].value.u32);
                    break;
                }

            case SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST:
                {
                    if (attr_list[count].value.s32list.count > 1)
                    {
                        XP_SAI_LOG_ERR("Currently supporting only one bind point type per table!\n");
                        return (SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count));
                    }

                    if (attr_list[count].value.s32list.count < 1)
                    {
                        XP_SAI_LOG_ERR("Invalid list items count: %d!\n",
                                       attr_list[count].value.s32list.count);
                        return (SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count));
                    }
                    attributes->bp_list[0] = attr_list[count].value.s32list.list[0];
                    break;
                }

            case SAI_ACL_TABLE_GROUP_ATTR_TYPE:
                {
                    attributes->type = (sai_acl_table_group_type_t)(attr_list[count].value.u32);
                    break;
                }

            default:
                {
                    XP_SAI_LOG_ERR("Invalid attribute: %d!\n", attr_list[count].id);
                    return (SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(count));
                }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateACLTableGroup

static sai_status_t xpSaiCreateACLTableGroup(sai_object_id_t
                                             *acl_table_group_id, sai_object_id_t switch_id, uint32_t attr_count,
                                             const sai_attribute_t *attr_list)
{
    sai_status_t                saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t                groupId     = 0;
    sai_uint32_t                devId       = 0;
    xpSaiAclTableGrpAttributesT attributes;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(switch_id, SAI_OBJECT_TYPE_SWITCH))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(switch_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (acl_table_group_id == NULL)
    {
        XP_SAI_LOG_ERR("Invalid parameter received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               ACL_TABLE_GROUP_VALIDATION_ARRAY_SIZE, acl_table_group_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    memset(&attributes, 0, sizeof(attributes));

    devId = (sai_uint32_t)xpSaiObjIdValueGet(switch_id);

    xpSaiSetDefaultAclTableGrpAttributeVals(&attributes);

    saiRetVal = xpSaiUpdateAclTableGrpAttributeVals(attr_count, attr_list,
                                                    &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update attributes, saiRetVal %u \n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiAclMapperCreateTableGrpId(devId, &groupId, &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, error %d\n", saiRetVal);
        XP_SAI_LOG_ERR("[skip] attribute: SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST: 1:SAI_ACL_BIND_POINT_TYPE_PORT");
        return SAI_STATUS_SUCCESS;
    }
    XP_SAI_LOG_NOTICE("SAI ACL Table Group Id created %d\n", groupId);
    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ACL_TABLE_GROUP, devId,
                                 (sai_uint64_t)groupId, acl_table_group_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, error %d\n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiGetTableEntryCountCtxDb(devId, &entryCountCtxPtr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB, retVal = %d \n", saiRetVal);
        return saiRetVal;
    }
    if (entryCountCtxPtr)
    {
        entryCountCtxPtr->aclTableGroups++;
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveACLTableGroup

static sai_status_t xpSaiRemoveACLTableGroup(sai_object_id_t acl_table_group_id)
{
    sai_status_t saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t groupId     = 0;
    sai_uint32_t devId       = xpSaiGetDevId();
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(acl_table_group_id,
                                  SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
    {
        XP_SAI_LOG_DBG("Invalid object type received 0x%" PRIx64 "\n",
                       acl_table_group_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    groupId = (sai_uint32_t)xpSaiObjIdValueGet(acl_table_group_id);

    //TODO: Check if members need to be removed also!

    saiRetVal = xpSaiAclMapperRemoveTableGrpId(devId, groupId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, error %d\n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiGetTableEntryCountCtxDb(devId, &entryCountCtxPtr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB, retVal = %d \n", saiRetVal);
        return saiRetVal;
    }
    if (entryCountCtxPtr)
    {
        entryCountCtxPtr->aclTableGroups--;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetACLTableGroupAttribute

static sai_status_t xpSaiSetACLTableGroupAttribute(sai_object_id_t
                                                   acl_table_group_id, const sai_attribute_t *attr)
{
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    XP_SAI_LOG_ERR("The attribute %d cannot be changed after the table group has been created\n",
                   attr->id);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetACLTableGroupAttribute

static sai_status_t xpSaiGetACLTableGroupAttributes(sai_object_id_t
                                                    acl_table_group_id, uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    sai_uint32_t groupId   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(acl_table_group_id,
                                  SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
    {
        XP_SAI_LOG_DBG("Invalid object type received 0x%" PRIx64 "\n",
                       acl_table_group_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if (attr_list == NULL || !attr_count)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               ACL_TABLE_GROUP_VALIDATION_ARRAY_SIZE, acl_table_group_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    groupId = (sai_uint32_t)xpSaiObjIdValueGet(acl_table_group_id);

    /* Get table group attributes from mapper */
    saiRetVal = xpSaiGetAclMapperTableGrpAttributes(groupId, attr_count, attr_list);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get table group %u related data\n", groupId);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetDefaultAclTableGrpMembAttributeVals

void xpSaiSetDefaultAclTableGrpMembAttributeVals(
    xpSaiAclTableGrpMembAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    attributes->groupId = 0;
    attributes->tableId = 0;
    attributes->priority = 0;
}

//Func: xpSaiUpdateAclTableGrpMembAttributeVals

static sai_status_t xpSaiUpdateAclTableGrpMembAttributeVals(
    const sai_uint32_t attr_count, const sai_attribute_t* attr_list,
    xpSaiAclTableGrpMembAttributesT* attributes)
{
    for (sai_uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID:
                {
                    if (!XDK_SAI_OBJID_TYPE_CHECK(attr_list[count].value.oid,
                                                  SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
                    {
                        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                                       xpSaiObjIdTypeGet(attr_list[count].value.oid));
                        return SAI_STATUS_INVALID_PARAMETER;
                    }
                    attributes->groupId = attr_list[count].value.oid;
                    break;
                }

            case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID:
                {
                    if (!XDK_SAI_OBJID_TYPE_CHECK(attr_list[count].value.oid,
                                                  SAI_OBJECT_TYPE_ACL_TABLE))
                    {
                        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                                       xpSaiObjIdTypeGet(attr_list[count].value.oid));
                        return SAI_STATUS_INVALID_PARAMETER;
                    }
                    attributes->tableId = attr_list[count].value.oid;
                    break;
                }

            case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY:
                {
                    attributes->priority = attr_list[count].value.u32;
                    break;
                }

            default:
                {
                    XP_SAI_LOG_ERR("Invalid attribute: %d!\n", attr_list[count].id);
                    return (SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(count));
                }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateACLTableGroupMember

static sai_status_t xpSaiCreateACLTableGroupMember(sai_object_id_t
                                                   *acl_table_group_member_id, sai_object_id_t switch_id, uint32_t attr_count,
                                                   const sai_attribute_t *attr_list)
{
    XP_STATUS                       xpStatus    = XP_NO_ERR;
    sai_status_t                    saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t                    memberId    = 0;
    sai_uint32_t                    devId       = 0;
    sai_acl_stage_t                 tableStage;
    xpSaiAclTableGrpMembAttributesT attributes;
    xpSaiAclTableIdMappingT         *pSaiAclTableAttribute = NULL;
    xpSaiRouterInterfaceDbEntryT    *pRifEntry = NULL;
    sai_object_id_t rifObjId = 0;
    sai_status_t                    saiStatus = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(switch_id, SAI_OBJECT_TYPE_SWITCH))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(switch_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (acl_table_group_member_id == NULL)
    {
        XP_SAI_LOG_ERR("Invalid parameter received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               ACL_TABLE_GROUP_MEMBER_VALIDATION_ARRAY_SIZE, acl_table_group_member_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    memset(&attributes, 0, sizeof(attributes));

    devId = (sai_uint32_t)xpSaiObjIdValueGet(switch_id);

    xpSaiSetDefaultAclTableGrpMembAttributeVals(&attributes);

    saiRetVal = xpSaiUpdateAclTableGrpMembAttributeVals(attr_count, attr_list,
                                                        &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update attributes, saiRetVal %u \n", saiRetVal);
        return saiRetVal;
    }

    xpSaiAclTableGrpIdMappingT *pTblGrpEntry = NULL;

    // Get the table group state data
    saiRetVal = xpSaiAclMapperGetTableGroupStateData(devId, attributes.groupId,
                                                     &pTblGrpEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get the acl table group state data base, status: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiGetACLTableAttrStage(attributes.tableId, &tableStage);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetACLTableAttrStage failed with error %d \n", saiRetVal);
        return saiRetVal;
    }
#ifdef ASIC_SIMULATION
    if (tableStage != pTblGrpEntry->stage)
    {
        XP_SAI_LOG_ERR("Wrong table stage!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
#endif
    sai_object_type_t objType = SAI_OBJECT_TYPE_MAX;
    uint32_t portId = 0, lagId = 0, vlanId = 0;
    uint32_t tableId = (uint32_t)xpSaiObjIdValueGet(attributes.tableId);

    saiRetVal = xpSaiAclTableAttributesGet(tableId, &pSaiAclTableAttribute);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get Table Entry, ret %d\n", saiRetVal);
        return saiRetVal;
    }

    //    if(!(pSaiAclTableAttribute->isMirror))
    //    {
    // Set Switch/Port/Lag/Vlan ingress ACL
    for (uint32_t i = 0; i < (pTblGrpEntry->bindPointCount); i++)
    {
        objType = sai_object_type_query(pTblGrpEntry->bindPointList[i]);

        switch (objType)
        {
            case SAI_OBJECT_TYPE_SWITCH:
                {
                    // TODO: Handle switch ingress acl
                    break;
                }
            case SAI_OBJECT_TYPE_PORT:
                {
                    portId = (uint32_t)xpSaiObjIdValueGet(pTblGrpEntry->bindPointList[i]);


                    xpStatus = xpsAclSetPortAcl(devId, portId, tableId, attributes.groupId);
                    if (xpStatus != XP_NO_ERR)
                    {
                        return xpsStatus2SaiStatus(xpStatus);
                    }
                    break;
                }
            case SAI_OBJECT_TYPE_LAG:
                {
                    lagId = (uint32_t)xpSaiObjIdValueGet(pTblGrpEntry->bindPointList[i]);

                    xpStatus = xpsAclSetLagAcl(devId, lagId, tableId, attributes.groupId);
                    if (xpStatus != XP_NO_ERR)
                    {
                        return xpsStatus2SaiStatus(xpStatus);
                    }
                    break;
                }
            case SAI_OBJECT_TYPE_VLAN:
                {
                    vlanId = (uint32_t)xpSaiObjIdValueGet(pTblGrpEntry->bindPointList[i]);

                    xpStatus = xpsAclSetVlanAcl(devId, vlanId, tableId, attributes.groupId);
                    if (xpStatus != XP_NO_ERR)
                    {
                        return xpsStatus2SaiStatus(xpStatus);
                    }

                    break;
                }
            case SAI_OBJECT_TYPE_ROUTER_INTERFACE:
                {
                    rifObjId = pTblGrpEntry->bindPointList[i];

                    saiStatus = xpSaiRouterInterfaceDbInfoGet(rifObjId, &pRifEntry);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                                       saiStatus);
                        return saiStatus;
                    }

                    saiStatus = xpSaiAclRouterAclIdSet(pRifEntry->l3IntfId, attributes.groupId,
                                                       tableId, TRUE);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("xpSaiAclRouterAclIdSet failed with error %d \n", saiStatus);
                        return saiStatus;
                    }
                    break;
                }

            default: /* Do Nothing */
                break;
        }

        //      }
    }

    saiRetVal = xpSaiAclMapperCreateTableGrpMembId(devId, &memberId, &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, error %d\n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER, devId,
                                 (sai_uint64_t)memberId, acl_table_group_member_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, error %d\n", saiRetVal);
        return saiRetVal;
    }

    if (pTblGrpEntry->memberCount >= SAI_ACL_TBL_GROUP_SIZE)
    {
        XP_SAI_LOG_ERR("Acl table group member list array full\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    pTblGrpEntry->memberList[pTblGrpEntry->memberCount] =
        *acl_table_group_member_id;
    pTblGrpEntry->memberCount += 1;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveACLTableGroupMember

static sai_status_t xpSaiRemoveACLTableGroupMember(sai_object_id_t
                                                   acl_table_group_member_id)
{
    sai_status_t saiRetVal   = SAI_STATUS_SUCCESS;
    XP_STATUS xpsStatus;
    sai_uint32_t memberId    = 0;
    sai_object_id_t groupId = 0;
    sai_uint32_t devId       = xpSaiGetDevId();
    xpSaiAclTableGrpIdMappingT *pTblGrpEntry = NULL;
    xpSaiAclTableGrpMembIdMappingT *keyFormat  = NULL;
    sai_object_type_t objType = SAI_OBJECT_TYPE_MAX;
    uint32_t portId = 0;
    uint32_t lagId = 0;
    uint32_t vlanId = 0;
    sai_object_id_t rifObjId = 0;
    xpSaiRouterInterfaceDbEntryT    *pRifEntry = NULL;


    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(acl_table_group_member_id,
                                  SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER))
    {
        XP_SAI_LOG_DBG("Invalid object type received 0x%" PRIx64 "\n",
                       acl_table_group_member_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    keyFormat = (xpSaiAclTableGrpMembIdMappingT *)xpMalloc(sizeof(
                                                               xpSaiAclTableGrpMembIdMappingT));
    pTblGrpEntry = (xpSaiAclTableGrpIdMappingT *)xpMalloc(sizeof(
                                                              xpSaiAclTableGrpIdMappingT));
    memberId = (sai_uint32_t)xpSaiObjIdValueGet(acl_table_group_member_id);

    saiRetVal = xpSaiAclMapperAclGroupMemberAttributesGet(memberId, &keyFormat);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpsStateSearchDataForDevice failed with xpStatus: %d\n",
                       saiRetVal);
        xpFree(pTblGrpEntry);
        return saiRetVal;
    }

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ACL_TABLE_GROUP, devId,
                                 (sai_uint64_t)keyFormat->groupId, &groupId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, error %d\n", saiRetVal);
        xpFree(pTblGrpEntry);
        return saiRetVal;
    }
    // Get the table group state data
    saiRetVal = xpSaiAclMapperGetTableGroupStateData(devId, groupId, &pTblGrpEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get the acl table group state data base, status: %d\n",
                       saiRetVal);
        xpFree(pTblGrpEntry);
        return saiRetVal;
    }
    for (uint32_t i = 0; i < (pTblGrpEntry->bindPointCount); i++)
    {
        objType = sai_object_type_query(pTblGrpEntry->bindPointList[i]);

        switch (objType)
        {
            case SAI_OBJECT_TYPE_SWITCH:
                {
                    // TODO: Handle switch ingress acl
                    break;
                }
            case SAI_OBJECT_TYPE_PORT:
                {
                    portId = (uint32_t)xpSaiObjIdValueGet(pTblGrpEntry->bindPointList[i]);
                    xpsStatus = xpsAclPortUnbind(devId, portId, keyFormat->groupId,
                                                 keyFormat->tableId);
                    if (xpsStatus != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to remove port bindpoint for groupId : %d, status: %d\n",
                                       keyFormat->groupId, saiRetVal);
                        return xpsStatus2SaiStatus(xpsStatus);
                    }
                    break;
                }
            case SAI_OBJECT_TYPE_LAG:
                {
                    lagId = (uint32_t)xpSaiObjIdValueGet(pTblGrpEntry->bindPointList[i]);

                    xpsStatus = xpsAclLagUnbind(devId, lagId, keyFormat->groupId,
                                                keyFormat->tableId);
                    if (xpsStatus != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to remove lag  bindpoint for groupId : %d, status: %d\n",
                                       keyFormat->groupId, saiRetVal);
                        return xpsStatus2SaiStatus(xpsStatus);
                    }
                    break;
                }
            case SAI_OBJECT_TYPE_VLAN:
                {
                    vlanId = (uint32_t)xpSaiObjIdValueGet(pTblGrpEntry->bindPointList[i]);

                    xpsStatus = xpsAclVlanUnbind(devId, vlanId, keyFormat->groupId,
                                                 keyFormat->tableId);
                    if (xpsStatus != XP_NO_ERR)
                    {
                        return xpsStatus2SaiStatus(xpsStatus);
                    }

                    break;
                }
            case SAI_OBJECT_TYPE_ROUTER_INTERFACE:
                {
                    rifObjId = pTblGrpEntry->bindPointList[i];

                    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rifObjId, &pRifEntry);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                                       saiRetVal);
                        return saiRetVal;
                    }

                    saiRetVal = xpSaiAclRouterAclIdSet(pRifEntry->l3IntfId, keyFormat->groupId,
                                                       keyFormat->tableId, false);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("xpSaiAclRouterAclIdSet failed with error %d \n", saiRetVal);
                        return saiRetVal;
                    }
                    break;
                }
            default: /* Do Nothing */
                break;
        }
    }
    saiRetVal = xpSaiAclMapperRemoveTableGrpMembId(devId, memberId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, error %d\n", saiRetVal);
        return saiRetVal;
    }

    uint32_t i, j;
    for (i = 0; i< pTblGrpEntry->memberCount; i++)
    {
        if (pTblGrpEntry->memberList[i] == acl_table_group_member_id)
        {
            j=i;
            for (; j< pTblGrpEntry->memberCount-1; j++)
            {
                pTblGrpEntry->memberList[j] = pTblGrpEntry->memberList[j+1];
            }
            pTblGrpEntry->memberList[j] = SAI_NULL_OBJECT_ID;
            break;
        }
    }
    pTblGrpEntry->memberCount -= 1;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetACLTableGroupMemberAttribute

static sai_status_t xpSaiSetACLTableGroupMemberAttribute(
    sai_object_id_t acl_table_group_member_id, const sai_attribute_t *attr)
{
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    XP_SAI_LOG_ERR("The attribute %d cannot be changed after the table group member has been created\n",
                   attr->id);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetACLTableGroupMemberAttribute

static sai_status_t xpSaiGetACLTableGroupMemberAttributes(
    sai_object_id_t acl_table_group_member_id, uint32_t attr_count,
    sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    sai_uint32_t memberId  = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(acl_table_group_member_id,
                                  SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER))
    {
        XP_SAI_LOG_DBG("Invalid object type received 0x%" PRIx64 "\n",
                       acl_table_group_member_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if (attr_list == NULL || !attr_count)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               ACL_TABLE_GROUP_MEMBER_VALIDATION_ARRAY_SIZE, acl_table_group_member_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    memberId = (sai_uint32_t)xpSaiObjIdValueGet(acl_table_group_member_id);

    /* Get table group member attributes from mapper */
    saiRetVal = xpSaiGetAclMapperTableGrpMembAttributes(memberId, attr_count,
                                                        attr_list);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get table group member %u related data\n", memberId);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiAclEntryAclIdSet(xpsAclkeyFieldList_t *pFieldData,
                                          sai_uint32_t acl_id, bool isSwitchAclEn)
{
    uint32_t pclMask = 0x3FF;
    uint32_t switchBindPointVal = SAI_ACL_GLOBAL_SWITCH_BIND_POINT;

    if (pFieldData == NULL)
    {
        XP_SAI_LOG_ERR("%s:, Error: Invalid parameter\n", __FUNCNAME__);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (!isSwitchAclEn)
    {
        pFieldData->fldList[pFieldData->numFlds].keyFlds = XPS_PCL_PCLID;
        memcpy(pFieldData->fldList[pFieldData->numFlds].value, &acl_id,
               sizeof(uint32_t));
        memcpy(pFieldData->fldList[pFieldData->numFlds].mask, &pclMask,
               sizeof(uint32_t));

        pFieldData->numFlds++;
    }
    else
    {
        pFieldData->fldList[pFieldData->numFlds].keyFlds = XPS_PCL_PCLID;
        memcpy(pFieldData->fldList[pFieldData->numFlds].value, &switchBindPointVal,
               sizeof(uint32_t));
        memcpy(pFieldData->fldList[pFieldData->numFlds].mask, &switchBindPointVal,
               sizeof(uint32_t));

        pFieldData->numFlds++;
    }
    return SAI_STATUS_SUCCESS;
}


static void xpSaiAclEntryAttributesPrint(xpSaiAclEntryAttributesT* pAttributes)
{
    sai_uint32_t i = 0;

    XP_SAI_LOG_DBG("########## SAI ACL Entry ##########\n\n");
    XP_SAI_LOG_DBG("Table oid 0x%" PRIx64 "\n", pAttributes->tableOid);
    XP_SAI_LOG_DBG("Priority %u\n", pAttributes->priority);
    XP_SAI_LOG_DBG("Admin state %u\n", pAttributes->adminState);

    XP_SAI_LOG_DBG("########## In ports ###############\n");
    for (i = 0; i < pAttributes->in_ports_count; i++)
    {
        XP_SAI_LOG_DBG("0x%lx\n", pAttributes->in_ports[i]);
    }

    XP_SAI_LOG_DBG("########## Out ports  #############\n");
    for (i = 0; i < pAttributes->out_ports_count; i++)
    {
        XP_SAI_LOG_DBG("0x%lx\n", pAttributes->out_ports[i]);
    }

    XP_SAI_LOG_DBG("########## Match fields ###########\n");
    for (i = 0; i < XP_SAI_ACL_MATCH_NUMBER_MAX; i++)
    {
        if (pAttributes->match[i].isSetByUser == TRUE)
        {
            sai_acl_entry_attr_t attrId = (sai_acl_entry_attr_t)(i +
                                                                 SAI_ACL_ENTRY_ATTR_FIELD_START);
            XP_SAI_LOG_DBG("%s\n", xpSaiEntryAttrNameGet(attrId));
        }
    }

    XP_SAI_LOG_DBG("########## Action fields ###########\n");

    for (i = 0; i < XP_SAI_ACL_ACTION_NUMBER_MAX; i++)
    {
        if (pAttributes->action[i].isSetByUser == TRUE)
        {
            sai_acl_entry_attr_t attrId = (sai_acl_entry_attr_t)(i +
                                                                 SAI_ACL_ENTRY_ATTR_ACTION_START);
            XP_SAI_LOG_DBG("%s\n", xpSaiEntryAttrNameGet(attrId));
        }
    }
}


static void xpSaiAclEntryKeyPrint(sai_uint32_t rule_id,
                                  xpsAclkeyFieldList_t* pFieldData)
{
    sai_uint32_t i = 0;

    XP_SAI_LOG_DBG("\n########## IACL Rules ##########\n\n");

    XP_SAI_LOG_DBG("Rule id is %u\n", rule_id);
    XP_SAI_LOG_DBG("Valid is %u\n", pFieldData->isValid);
    XP_SAI_LOG_DBG("Fields number is %u\n", pFieldData->numFlds);

    XP_SAI_LOG_DBG("########## IACL Field List ##########\n");

    for (i = 0; i < SAI_IACL_IPV4_FIELD_LIST_SIZE; i++)
    {
        XP_SAI_LOG_DBG("%u: v4Fld %s(%u), ", i,
                       xpSaiAclKeyFldNameGet(pFieldData->fldList[i].keyFlds),
                       pFieldData->fldList[i].keyFlds);

        if (pFieldData->fldList[i].keyFlds == XPS_PCL_SIP ||
            pFieldData->fldList[i].keyFlds == XPS_PCL_DIP)
        {
            XP_SAI_LOG_DBG("ip %u.%u.%u.%u",
                           pFieldData->fldList[i].value[0],
                           pFieldData->fldList[i].value[1],
                           pFieldData->fldList[i].value[2],
                           pFieldData->fldList[i].value[3]);

            XP_SAI_LOG_DBG(", mask %u.%u.%u.%u",
                           pFieldData->fldList[i].mask[0],
                           pFieldData->fldList[i].mask[1],
                           pFieldData->fldList[i].mask[2],
                           pFieldData->fldList[i].mask[3]);

            XP_SAI_LOG_DBG("\n");
        }
        if (pFieldData->fldList[i].keyFlds == XPS_PCL_MAC_SA ||
            pFieldData->fldList[i].keyFlds == XPS_PCL_MAC_DA)
        {
            XP_SAI_LOG_DBG("mac %02x:%02x:%02x:%02x:%02x:%02x",
                           pFieldData->fldList[i].value[0],
                           pFieldData->fldList[i].value[1],
                           pFieldData->fldList[i].value[2],
                           pFieldData->fldList[i].value[3],
                           pFieldData->fldList[i].value[4],
                           pFieldData->fldList[i].value[5]);

            XP_SAI_LOG_DBG(", mask %02x:%02x:%02x:%02x:%02x:%02x",
                           pFieldData->fldList[i].mask[0],
                           pFieldData->fldList[i].mask[1],
                           pFieldData->fldList[i].mask[2],
                           pFieldData->fldList[i].mask[3],
                           pFieldData->fldList[i].mask[4],
                           pFieldData->fldList[i].mask[5]);

            XP_SAI_LOG_DBG("\n");
        }
        else if (pFieldData->fldList[i].keyFlds == XPS_PCL_ETHER_TYPE ||
                 pFieldData->fldList[i].keyFlds == XPS_PCL_L4_BYTE0 ||
                 pFieldData->fldList[i].keyFlds == XPS_PCL_L4_BYTE2)
        {
            XP_SAI_LOG_DBG("value 0x%04x, mask 0x%04x\n",
                           *(sai_uint16_t *)pFieldData->fldList[i].value,
                           *(sai_uint16_t *)pFieldData->fldList[i].mask);
        }
        else
        {
            XP_SAI_LOG_DBG("value %#1x, mask %#1x\n",
                           pFieldData->fldList[i].value[0],
                           pFieldData->fldList[i].mask[0]);
        }
    }

    XP_SAI_LOG_DBG("\n#####################################\n");
}

void xpSaiAclEntryDataPrint(sai_uint32_t rule_id, xpsPclAction_t* pAclEntryData)
{
    XP_SAI_LOG_DBG("\n########## IACL Actions #############\n\n");


    XP_SAI_LOG_DBG("\n#####################################\n");
}

sai_status_t xpSaiAclEntryFillL4PortAttr(xpsAclkeyFieldList_t *pFieldData,
                                         sai_uint16_t l4PortValue,
                                         sai_uint16_t l4PortMask,
                                         sai_acl_range_type_t l4PortType)
{
    sai_uint32_t *keyIndex = &(pFieldData->numFlds);
    sai_uint8_t value, mask;

    switch (l4PortType)
    {
        //supporting only 16 bit so converting into 16.
        case SAI_ACL_RANGE_TYPE_L4_SRC_PORT_RANGE:
            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_L4_BYTE0;
            value = ((l4PortValue & 0xFF00) >> 8);
            mask = ((l4PortMask & 0xFF00) >> 8);
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;

            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_L4_BYTE1;
            value = (l4PortValue & 0x00FF);
            mask = (l4PortMask & 0x00FF);
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;

            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_IS_L4_VALID;
            value = 1;
            mask =  1;
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;
            break;

        case SAI_ACL_RANGE_TYPE_L4_DST_PORT_RANGE:
            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_L4_BYTE2;
            value = ((l4PortValue & 0xFF00) >> 8);
            mask = ((l4PortMask & 0xFF00) >> 8);
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;

            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_L4_BYTE3;
            value = (l4PortValue & 0x00FF);
            mask = (l4PortMask & 0x00FF);
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;

            pFieldData->fldList[*keyIndex].keyFlds = XPS_PCL_IS_L4_VALID;
            value = 1;
            mask =  1;
            memcpy(pFieldData->fldList[*keyIndex].value, &value, sizeof(sai_uint8_t));
            memcpy(pFieldData->fldList[*keyIndex].mask, &mask, sizeof(sai_uint8_t));
            (*keyIndex)++;
            break;
        default:
            XP_SAI_LOG_ERR("Error: Unsupported or Invalid Range type\n");
            break;

    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t xpSaiAclEntryInstallRangeRule(xpsDevice_t devId,
                                           sai_uint32_t entryId,
                                           xpSaiAclEntryAttributesT* pAttributes,
                                           xpsAclkeyFieldList_t *pFieldData,
                                           xpsPclAction_t  aclEntryData,
                                           xpSaiAclTableIdMappingT *pSaiAclTableAttribute,
                                           xpsAclKeyFormat keyFormat)
{
    sai_status_t                saiRetVal   = SAI_STATUS_SUCCESS;
    XP_STATUS                   retVal = XP_NO_ERR;
    sai_uint32_t                range_entry_id    = 0;
    sai_object_id_t             acl_range_oid;
    xpSaiAclRangeAttributesT    *saiAclRangeAttributes;
    sai_uint16_t                 numSubIntervals;
    sai_uint16_t                l4PortValue[30] = {0};
    sai_uint16_t                l4PortMask[30] = {0};
    sai_uint32_t                tableId = pAttributes->tableId;
    sai_uint32_t                xpRulePriority =0;
    sai_uint32_t                prevNumFlds = 0;

    prevNumFlds = pFieldData->numFlds;
    /* In SAI, least rule priority is 0 and highest priority is SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY
     * whereas in CPSS/XPS, it is the other way. So convert the rule priority before passing it to XPS. */
    xpRulePriority = xpSaiAclConvertRulePriorityToXpPriority(pAttributes->priority);

    for (sai_uint32_t i=0; i<pAttributes->aclRangeObjList.count; i++)
    {

        acl_range_oid = *(pAttributes->aclRangeObjList.list + i);
        if (!XDK_SAI_OBJID_TYPE_CHECK(acl_range_oid, SAI_OBJECT_TYPE_ACL_RANGE))
        {
            XP_SAI_LOG_DBG("Unknown object 0x%" PRIx64 "\n", acl_range_oid);
            return SAI_STATUS_INVALID_OBJECT_TYPE;
        }

        range_entry_id = (sai_uint32_t)xpSaiObjIdValueGet(acl_range_oid);

        saiRetVal = xpSaiAclMapperAclRangeAttributesGet(range_entry_id,
                                                        &saiAclRangeAttributes);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get entry attributes from DB\n");
            return saiRetVal;
        }

        saiRetVal = xpSaiUtilGetSubIntervals(saiAclRangeAttributes->from,
                                             saiAclRangeAttributes->to,
                                             l4PortValue, l4PortMask, &numSubIntervals);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get subIntervals for l4 port range\n");
            return saiRetVal;
        }

        /*Increment count for each range object*/
        pAttributes->entryIdCount += numSubIntervals;

        for (sai_uint16_t subIntervalIndex = 0; subIntervalIndex < numSubIntervals;
             subIntervalIndex++)
        {
            /*Allocate entryId for each subinterval*/
            if (i || subIntervalIndex)
            {
                entryId = 0;

                saiRetVal = xpSaiAclMapperEntryCreate(&entryId);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to add entry into DB, ret = %u \n", saiRetVal);
                    return saiRetVal;
                }
                //Everytime we need to write at the same location, cant keep on increasing the keyIndex
                pFieldData->numFlds = prevNumFlds;
                pAttributes->isRangeBase = false;
            }
            else
            {
                pAttributes->isRangeBase = true;
            }


            /*Save all the entries that are created for the ranges and save count
             * All subrules must be deleted when base rule is deleted*/
            pAttributes->entryIds[pAttributes->entryIdCount-numSubIntervals
                                  +subIntervalIndex] = entryId;

            saiRetVal = xpSaiAclEntryFillL4PortAttr(pFieldData,
                                                    l4PortValue[subIntervalIndex], l4PortMask[subIntervalIndex],
                                                    saiAclRangeAttributes->rangeType);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to fill l4 port attributes in ACL entry\n");
                return saiRetVal;
            }

            retVal = xpsAclWriteEntry(devId, tableId, xpRulePriority, entryId, pFieldData,
                                      aclEntryData, keyFormat, pSaiAclTableAttribute->isSwitchAclEn);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to write ACL entry, ret = %u \n", retVal);
                return xpsStatus2SaiStatus(retVal);
            }
        }


        saiRetVal = xpSaiAclMapperAclRangeAttributesSet(range_entry_id,
                                                        saiAclRangeAttributes);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to set range attributes in DB\n");
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiWriteAclEntry

sai_status_t xpSaiWriteAclEntry(xpsDevice_t devId,
                                sai_uint32_t tableId,
                                sai_uint32_t entryId,
                                xpSaiAclEntryAttributesT* pAttributes)
{
    XP_STATUS                   retVal      = XP_NO_ERR;
    sai_status_t                saiRetVal   = SAI_STATUS_SUCCESS;
    xpSaiAclFieldT              *acl_value = NULL;
    xpSaiAclFieldT              *acl_mask = NULL;
    xpsAclkeyFieldList_t        aclFieldData;
    xpsAclkeyField_t            *aclFieldList = NULL;
    xpsPclAction_t              aclEntryData;
    xpSaiAclTableIdMappingT     *pSaiAclTableAttribute = NULL;
    xpsAclKeyFormat             keyFormat = XPS_PCL_ARP_KEY;
    sai_uint32_t                xpRulePriority = 0;
    uint32_t                    intfId = 0xFFFFFFFF;

    saiRetVal = xpSaiGetIaclKeyFormat(tableId, pAttributes->entryType, &keyFormat);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: xpSaiGetIaclKeyFormat failed\n");
        return saiRetVal;
    }

    if (keyFormat == XPS_PCL_ARP_KEY)
    {
        XP_SAI_LOG_ERR("Invalid table type and key format combination\n");
        return saiRetVal;

    }
    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));

    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to allocate table group id\n");
        return xpsStatus2SaiStatus(retVal);
    }

    acl_value = (xpSaiAclFieldT *)xpMalloc(sizeof(xpSaiAclFieldT)
                                           *SAI_IACL_IPV4_FIELD_LIST_SIZE);
    if (!acl_value)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for acl_value\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(acl_value, 0x00, sizeof(xpSaiAclFieldT)*SAI_IACL_IPV4_FIELD_LIST_SIZE);

    acl_mask = (xpSaiAclFieldT *)xpMalloc(sizeof(xpSaiAclFieldT)
                                          *SAI_IACL_IPV4_FIELD_LIST_SIZE);
    if (!acl_mask)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for acl_mask\n");
        xpFree(acl_value);
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(acl_mask, 0x00, sizeof(xpSaiAclFieldT)*SAI_IACL_IPV4_FIELD_LIST_SIZE);


    aclFieldList = (xpsAclkeyField_t *)xpMalloc(sizeof(xpsAclkeyField_t)
                                                *SAI_IACL_IPV4_FIELD_LIST_SIZE);
    if (!aclFieldList)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for aclFieldList\n");
        xpFree(acl_value);
        xpFree(acl_mask);
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(aclFieldList, 0xFF,
           sizeof(xpsAclkeyField_t)*SAI_IACL_IPV4_FIELD_LIST_SIZE);

    for (sai_uint32_t i = 0; i < SAI_IACL_IPV4_FIELD_LIST_SIZE; i++)
    {
        aclFieldList[i].value = acl_value[i].data;
        aclFieldList[i].mask = acl_mask[i].data;
    }
    aclFieldData.fldList = aclFieldList;

    saiRetVal = xpSaiAclEntryFieldSet(pAttributes, &aclFieldData, &aclEntryData,
                                      tableId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set acl entry and field data, ret = %u \n",
                       saiRetVal);
        xpFree(acl_value);
        xpFree(acl_mask);
        xpFree(aclFieldList);
        return saiRetVal;
    }

    xpSaiAclEntryKeyPrint(pAttributes->priority, &aclFieldData);
    /* Get table info */
    if ((saiRetVal = xpSaiAclTableAttributesGet(tableId,
                                                &pSaiAclTableAttribute)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:, Failed to get table %u related data\n", __FUNCNAME__,
                       tableId);
        xpFree(acl_value);
        xpFree(acl_mask);
        xpFree(aclFieldList);
        return saiRetVal;
    }

    if ((pAttributes->aclRangeObjList.count))
    {

        /*1. For range - Fetch range and divide it into multiple rules of source/dest port*/
        /*2. For each value/mask pair fill the aclFieldData*/
        /*3. Write entry for each of these*/

        saiRetVal = xpSaiAclEntryInstallRangeRule(devId, entryId, pAttributes,
                                                  &aclFieldData, aclEntryData,  pSaiAclTableAttribute, keyFormat);

        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to install l4 port range\n");
            xpFree(acl_value);
            xpFree(acl_mask);
            xpFree(aclFieldList);
            return saiRetVal;
        }
    }
    else
    {
        /* In SAI, least rule priority is 0 and highest priority is SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY
         * whereas in CPSS/XPS, it is the other way. So convert the rule priority before passing it to XPS. */
        xpRulePriority = xpSaiAclConvertRulePriorityToXpPriority(pAttributes->priority);
        retVal = xpsAclWriteEntry(devId, tableId, xpRulePriority, entryId,
                                  &aclFieldData, aclEntryData, keyFormat, pSaiAclTableAttribute->isSwitchAclEn);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to write ACL entry, ret = %u \n", retVal);
            xpFree(acl_value);
            xpFree(acl_mask);
            xpFree(aclFieldList);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    /*Update mirror-index field*/
    if (pAttributes->in_ports_count && pAttributes->isMirrorEnable)
    {
        for (uint32_t idx = 0 ; idx < pAttributes->in_ports_count; idx++)
        {
            intfId = xpsLocalPortToGlobalPortnum(devId, pAttributes->in_ports[idx]);
            retVal = xpsMirrorUpdateErSpan2Index(devId, intfId, true, true);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("%s:, Failed in validation \n", __FUNCNAME__,
                               "xpsMirrorUpdateErSpan2Index Failed %d \n", retVal);
                return xpsStatus2SaiStatus(retVal);
            }
        }
    }
    if (pAttributes->out_ports_count && pAttributes->isMirrorEnable)
    {
        for (uint32_t idx = 0 ; idx < pAttributes->out_ports_count; idx++)
        {
            intfId = xpsLocalPortToGlobalPortnum(devId, pAttributes->out_ports[idx]);
            retVal = xpsMirrorUpdateErSpan2Index(devId, intfId, false, true);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("%s:, Failed in validation \n", __FUNCNAME__,
                               "xpsMirrorUpdateErSpan2Index Failed %d \n", retVal);
                return xpsStatus2SaiStatus(retVal);
            }
            retVal = xpsAclErspanRuleUpdate(devId, intfId, true,
                                            aclEntryData.mirror.egressMirrorToAnalyzerIndex);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to enable mirror on port %d with error code %d\n",
                               intfId, retVal);
                return xpsStatus2SaiStatus(retVal);
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t xpSaiDeleteAclEntry(xpsDevice_t devId,
                                 sai_uint32_t tableId,
                                 xpSaiAclEntryRuleT *entryRule)
{
    XP_STATUS                retVal      = XP_NO_ERR;
    sai_status_t             saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t             ruleId      = 0;
    xpSaiAclTableIdMappingT *pSaiAclTableAttribute = NULL;


    /* Get table info */
    if ((saiRetVal = xpSaiAclTableAttributesGet(tableId,
                                                &pSaiAclTableAttribute)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:, Failed to get table %u related data\n", __FUNCNAME__,
                       tableId);
        return saiRetVal;
    }

    saiRetVal = xpSaiAclMapperRuleIdGet(devId, tableId, entryRule->id, &ruleId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get ruleId array for table %u entry %u with error %u \n",
                       tableId, entryRule->id, saiRetVal);
        return saiRetVal;
    }

    XP_SAI_LOG_DBG("table %d, entry id %d, rule id %d\n", tableId, entryRule->id,
                   ruleId);

    retVal = xpsAclSetRuleValid(devId, tableId, ruleId, 0x0);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to delete rule %u from table %u\n", ruleId, tableId);
        return xpsStatus2SaiStatus(retVal);
    }


    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiUpdateAclEntry(xpsDevice_t devId,
                                 sai_uint32_t tableId,
                                 sai_uint32_t aclEntryId, // Generated EntryId in ACL
                                 sai_uint32_t entryId,
                                 const sai_attribute_t* attr,
                                 xpSaiAclEntryAttributesT *pSaiAclEntryAttributes)
{
    sai_status_t         saiRetVal   = SAI_STATUS_SUCCESS;
    XP_STATUS            retVal      = XP_NO_ERR;
    xpsPclAction_t       aclEntryData;
    xpsAclkeyFieldList_t aclFieldData;
    xpSaiAclTableIdMappingT *pSaiAclTableAttribute = NULL;
    xpsAclkeyField_t        *aclFieldList = NULL;
    xpSaiAclFieldT           *acl_value = NULL;
    xpSaiAclFieldT           *acl_mask = NULL;
    xpsAclKeyFormat keyFormat = XPS_PCL_IPV4_L4_KEY;
    sai_uint32_t counterId            = 0;
    sai_uint32_t xpRulePriority = 0;
    uint32_t                    intfId = 0xFFFFFFFF;

    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(xpsPclAction_t));

    saiRetVal = xpSaiGetIaclKeyFormat(tableId, pSaiAclEntryAttributes->entryType,
                                      &keyFormat);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: xpSaiGetIaclKeyFormat failed\n");
        return saiRetVal;
    }

    acl_value = (xpSaiAclFieldT *)xpMalloc(sizeof(xpSaiAclFieldT)
                                           *SAI_IACL_IPV4_FIELD_LIST_SIZE);
    if (!acl_value)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for acl_value\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(acl_value, 0x00, sizeof(xpSaiAclFieldT)*SAI_IACL_IPV4_FIELD_LIST_SIZE);

    acl_mask = (xpSaiAclFieldT *)xpMalloc(sizeof(xpSaiAclFieldT)
                                          *SAI_IACL_IPV4_FIELD_LIST_SIZE);
    if (!acl_mask)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for acl_mask\n");
        xpFree(acl_value);
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(acl_mask, 0x00, sizeof(xpSaiAclFieldT)*SAI_IACL_IPV4_FIELD_LIST_SIZE);

    aclFieldList = (xpsAclkeyField_t *)xpMalloc(sizeof(xpsAclkeyField_t)
                                                *SAI_IACL_IPV4_FIELD_LIST_SIZE);
    if (!aclFieldList)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for aclFieldList\n");
        xpFree(acl_value);
        xpFree(acl_mask);
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(aclFieldList, 0xFF,
           sizeof(xpsAclkeyField_t)*SAI_IACL_IPV4_FIELD_LIST_SIZE);

    for (sai_uint32_t i = 0; i < SAI_IACL_IPV4_FIELD_LIST_SIZE; i++)
    {
        aclFieldList[i].value = &acl_value[i].data[0];
        aclFieldList[i].mask = &acl_mask[i].data[0];
    }
    aclFieldData.fldList = aclFieldList;


    /* Get table info */
    if ((saiRetVal = xpSaiAclTableAttributesGet(tableId,
                                                &pSaiAclTableAttribute)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:, Failed to get table %u related data\n", __FUNCNAME__,
                       tableId);
        xpFree(acl_value);
        xpFree(acl_mask);
        xpFree(aclFieldList);
        return saiRetVal;
    }

    retVal = xpsAclReadData(devId, tableId, aclEntryId, &aclEntryData, keyFormat);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to read ACL action, ret = %u \n", retVal);
        xpFree(acl_value);
        xpFree(acl_mask);
        xpFree(aclFieldList);
        return xpsStatus2SaiStatus(retVal);
    }


    saiRetVal = xpSaiAclEntryFieldSet(pSaiAclEntryAttributes, &aclFieldData,
                                      &aclEntryData, tableId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set acl entry and field data, ret = %u \n",
                       saiRetVal);
        xpFree(acl_value);
        xpFree(acl_mask);
        xpFree(aclFieldList);
        return saiRetVal;
    }
    if (aclEntryData.matchCounter.enableMatchCount)
    {
        //try to get the counterId here and get the entryId of the aclMappingEntry ]
        //and invoke xpSaiAclMapperAddEntryInCounter
        counterId = aclEntryData.matchCounter.matchCounterIndex;
        if (xpSaiAclMapperAddEntryInCounter(counterId,
                                            aclEntryId) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_DBG("Failed to update counter %u for entry %u \n", counterId,
                           entryId);
            xpFree(acl_value);
            xpFree(acl_mask);
            xpFree(aclFieldList);
            return SAI_STATUS_INVALID_ATTR_VALUE_0;
        }

    }

    /* In SAI, least rule priority is 0 and highest priority is SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY
     * whereas in CPSS/XPS, it is the other way. So convert the rule priority before passing it to XPS. */
    xpRulePriority = xpSaiAclConvertRulePriorityToXpPriority(
                         pSaiAclEntryAttributes->priority);

    if (pSaiAclTableAttribute->stage == SAI_ACL_STAGE_INGRESS)
    {
        retVal = xpsPortsAclEntryKeySet(devId, tableId, aclEntryId,
                                        pSaiAclEntryAttributes->in_ports, pSaiAclEntryAttributes->in_ports_count);
    }
    else if (pSaiAclTableAttribute->stage == SAI_ACL_STAGE_EGRESS)
    {
        retVal = xpsPortsAclEntryKeySet(devId, tableId, aclEntryId,
                                        pSaiAclEntryAttributes->out_ports, pSaiAclEntryAttributes->out_ports_count);
    }
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to update IN_PORTS, ret = %u \n", retVal);
        xpFree(acl_value);
        xpFree(acl_mask);
        xpFree(aclFieldList);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsUpdateAclEntry(devId, tableId, xpRulePriority, aclEntryId,
                               &aclFieldData, aclEntryData, keyFormat, pSaiAclTableAttribute->isSwitchAclEn);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to update ACL entry, ret = %u \n", retVal);
        xpFree(acl_value);
        xpFree(acl_mask);
        xpFree(aclFieldList);
        return xpsStatus2SaiStatus(retVal);
    }

    /*Update mirror-index field*/
    if (pSaiAclEntryAttributes->in_ports_count &&
        pSaiAclEntryAttributes->isMirrorEnable)
    {
        for (uint32_t idx = 0 ; idx < pSaiAclEntryAttributes->in_ports_count; idx++)
        {
            intfId = xpsLocalPortToGlobalPortnum(devId,
                                                 pSaiAclEntryAttributes->in_ports[idx]);
            retVal = xpsMirrorUpdateErSpan2Index(devId, intfId, true, true);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("%s:, Failed in validation \n", __FUNCNAME__,
                               "xpsMirrorUpdateErSpan2Index Failed %d \n", retVal);
                return xpsStatus2SaiStatus(retVal);
            }
        }
    }

    if (pSaiAclEntryAttributes->out_ports_count &&
        pSaiAclEntryAttributes->isMirrorEnable)
    {
        for (uint32_t idx = 0 ; idx < pSaiAclEntryAttributes->out_ports_count; idx++)
        {
            intfId = xpsLocalPortToGlobalPortnum(devId,
                                                 pSaiAclEntryAttributes->out_ports[idx]);
            retVal = xpsMirrorUpdateErSpan2Index(devId, intfId, false, true);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("%s:, Failed in validation \n", __FUNCNAME__,
                               "xpsMirrorUpdateErSpan2Index Failed %d \n", retVal);
                return xpsStatus2SaiStatus(retVal);
            }
            retVal = xpsAclErspanRuleUpdate(devId, intfId, true,
                                            aclEntryData.mirror.egressMirrorToAnalyzerIndex);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to enable mirror on port %d with error code %d\n",
                               intfId, retVal);
                return xpsStatus2SaiStatus(retVal);
            }
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiAclAddDuplicateAclEntry(sai_uint32_t tableId,
                                          sai_uint32_t entryId)
{
    xpSaiAclEntryAttributesT *pSaiAclEntryAttribute = NULL;
    sai_status_t             saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS                retVal;
    sai_uint32_t             devId=0;
    xpsAclKeyFormat         keyFormat;
    xpsAclkeyFieldList_t     aclFieldData;
    xpsPclAction_t           aclActionData;
    xpsAclkeyField_t         *aclFieldList = NULL;
    xpSaiAclFieldT           *acl_value = NULL;
    xpSaiAclFieldT           *acl_mask = NULL;
    sai_uint32_t             xpRulePriority = 0;

    saiRetVal = xpSaiAclMapperEntryAttributesGet(entryId, &pSaiAclEntryAttribute);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get entry attributes from DB\n");
        return saiRetVal;
    }

    switch (pSaiAclEntryAttribute->entryType)
    {
        case SAI_ACL_IP_TYPE_NON_IP:
            keyFormat = XPS_PCL_NON_IP_KEY;
            break;
        case SAI_ACL_IP_TYPE_NON_IPV6:
        case SAI_ACL_IP_TYPE_IPV4ANY:
        case SAI_ACL_IP_TYPE_ARP:
        case SAI_ACL_IP_TYPE_ARP_REQUEST:
        case SAI_ACL_IP_TYPE_ARP_REPLY:
            keyFormat = XPS_PCL_IPV4_L4_KEY;
            break;
        case SAI_ACL_IP_TYPE_NON_IPV4:
        case SAI_ACL_IP_TYPE_IPV6ANY:
            keyFormat = XPS_PCL_IPV6_L4_KEY;
            break;

        case SAI_ACL_IP_TYPE_ANY:
            XP_SAI_LOG_ERR("[skip] Unsupported attribute SAI_ACL_IP_TYPE_ANY \n");
            saiRetVal=SAI_STATUS_SUCCESS;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
    }
    acl_value = (xpSaiAclFieldT *)xpMalloc(sizeof(xpSaiAclFieldT)
                                           *SAI_IACL_IPV4_FIELD_LIST_SIZE);
    if (acl_value == NULL)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for attributes\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    acl_mask  = (xpSaiAclFieldT *)xpMalloc(sizeof(xpSaiAclFieldT)
                                           *SAI_IACL_IPV4_FIELD_LIST_SIZE);
    if (acl_mask == NULL)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for attributes\n");
        xpFree(acl_value);
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    aclFieldList = (xpsAclkeyField_t *)xpMalloc(sizeof(xpsAclkeyField_t)
                                                *SAI_IACL_IPV4_FIELD_LIST_SIZE);
    if (aclFieldList == NULL)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for attributes\n");
        xpFree(acl_value);
        xpFree(acl_mask);
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    memset(acl_value, 0x00, sizeof(xpSaiAclFieldT)*SAI_IACL_IPV4_FIELD_LIST_SIZE);
    memset(acl_mask, 0x00, sizeof(xpSaiAclFieldT)*SAI_IACL_IPV4_FIELD_LIST_SIZE);
    memset(aclFieldList, 0xFF,
           sizeof(xpsAclkeyField_t)*SAI_IACL_IPV4_FIELD_LIST_SIZE);
    memset(&aclActionData, 0x00, sizeof(xpsPclAction_t));
    memset(&aclFieldData, 0x00, sizeof(xpsAclkeyFieldList_t));

    for (sai_uint32_t i = 0; i < SAI_IACL_IPV4_FIELD_LIST_SIZE; i++)
    {
        aclFieldList[i].value = &acl_value[i].data[0];
        aclFieldList[i].mask =  &acl_mask[i].data[0];
    }
    aclFieldData.fldList = aclFieldList;
    retVal = xpsAclReadData(devId, tableId, entryId, &aclActionData, keyFormat);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to read ACL action, ret = %u \n", retVal);
        xpFree(acl_value);
        xpFree(acl_mask);
        xpFree(aclFieldList);
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = xpSaiAclEntryFieldSet(pSaiAclEntryAttribute, &aclFieldData,
                                      &aclActionData, tableId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set acl entry and field data, ret = %u \n",
                       saiRetVal);
        xpFree(acl_value);
        xpFree(acl_mask);
        xpFree(aclFieldList);
        return saiRetVal;
    }

    /* In SAI, least rule priority is 0 and highest priority is SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY
     * whereas in CPSS/XPS, it is the other way. So convert the rule priority before passing it to XPS. */
    xpRulePriority = xpSaiAclConvertRulePriorityToXpPriority(
                         pSaiAclEntryAttribute->priority);

    retVal = xpsAclWriteEntry(devId, tableId, xpRulePriority, entryId,
                              &aclFieldData, aclActionData, keyFormat, true);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to update ACL entry, ret = %u \n", retVal);
        xpFree(acl_value);
        xpFree(acl_mask);
        xpFree(aclFieldList);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}
sai_status_t xpSaiAclRemoveDuplicateAclEntry(sai_uint32_t tableId,
                                             sai_uint32_t entryId)
{
    xpSaiAclEntryAttributesT *pSaiAclEntryAttribute = NULL;
    sai_status_t             saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS                retVal;
    sai_uint32_t             devId=0;
    xpsAclKeyFormat         keyFormat;
    xpsAclkeyFieldList_t     aclFieldData;
    xpsPclAction_t           aclActionData;
    xpsAclkeyField_t         *aclFieldList = NULL;
    xpSaiAclFieldT           *acl_value = NULL;
    xpSaiAclFieldT           *acl_mask = NULL;
    sai_uint32_t             xpRulePriority = 0;

    retVal = xpsAclDeleteEntry(devId, tableId, entryId, true);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to delete entry from HW %d\n", entryId);
        return xpsStatus2SaiStatus(retVal);
    }
    saiRetVal = xpSaiAclMapperEntryAttributesGet(entryId, &pSaiAclEntryAttribute);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get entry attributes from DB\n");
        return saiRetVal;
    }

    switch (pSaiAclEntryAttribute->entryType)
    {
        case SAI_ACL_IP_TYPE_NON_IP:
            keyFormat = XPS_PCL_NON_IP_KEY;
            break;
        case SAI_ACL_IP_TYPE_NON_IPV6:
        case SAI_ACL_IP_TYPE_IPV4ANY:
            keyFormat = XPS_PCL_IPV4_L4_KEY;
            break;
        case SAI_ACL_IP_TYPE_NON_IPV4:
        case SAI_ACL_IP_TYPE_IPV6ANY:
            keyFormat = XPS_PCL_IPV6_L4_KEY;
            break;

        case SAI_ACL_IP_TYPE_ANY:
            XP_SAI_LOG_ERR("[skip] Unsupported attribute SAI_ACL_IP_TYPE_ANY \n");
            saiRetVal=SAI_STATUS_SUCCESS;
        default:
            return SAI_STATUS_INVALID_PARAMETER;
    }
    acl_value = (xpSaiAclFieldT *)xpMalloc(sizeof(xpSaiAclFieldT)
                                           *SAI_IACL_IPV4_FIELD_LIST_SIZE);
    if (acl_value == NULL)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for attributes\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    acl_mask  = (xpSaiAclFieldT *)xpMalloc(sizeof(xpSaiAclFieldT)
                                           *SAI_IACL_IPV4_FIELD_LIST_SIZE);
    if (acl_mask == NULL)
    {
        xpFree(acl_value);
        XP_SAI_LOG_ERR("Error: allocation failed for attributes\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    aclFieldList = (xpsAclkeyField_t *)xpMalloc(sizeof(xpsAclkeyField_t)
                                                *SAI_IACL_IPV4_FIELD_LIST_SIZE);
    if (aclFieldList == NULL)
    {
        xpFree(acl_value);
        xpFree(acl_mask);
        XP_SAI_LOG_ERR("Error: allocation failed for attributes\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    memset(acl_value, 0x00, sizeof(xpSaiAclFieldT)*SAI_IACL_IPV4_FIELD_LIST_SIZE);
    memset(acl_mask, 0x00, sizeof(xpSaiAclFieldT)*SAI_IACL_IPV4_FIELD_LIST_SIZE);
    memset(aclFieldList, 0xFF,
           sizeof(xpsAclkeyField_t)*SAI_IACL_IPV4_FIELD_LIST_SIZE);
    memset(&aclActionData, 0x00, sizeof(xpsPclAction_t));
    memset(&aclFieldData, 0x00, sizeof(xpsAclkeyFieldList_t));

    for (sai_uint32_t i = 0; i < SAI_IACL_IPV4_FIELD_LIST_SIZE; i++)
    {
        aclFieldList[i].value = &acl_value[i].data[0];
        aclFieldList[i].mask =  &acl_mask[i].data[0];
    }
    aclFieldData.fldList = aclFieldList;
    retVal = xpsAclReadData(devId, tableId, entryId, &aclActionData, keyFormat);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to read ACL action, ret = %u \n", retVal);
        xpFree(acl_value);
        xpFree(acl_mask);
        xpFree(aclFieldList);
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = xpSaiAclEntryFieldSet(pSaiAclEntryAttribute, &aclFieldData,
                                      &aclActionData, tableId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set acl entry and field data, ret = %u \n",
                       saiRetVal);
        xpFree(acl_value);
        xpFree(acl_mask);
        xpFree(aclFieldList);
        return saiRetVal;
    }

    /* In SAI, least rule priority is 0 and highest priority is SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY
     * whereas in CPSS/XPS, it is the other way. So convert the rule priority before passing it to XPS. */
    xpRulePriority = xpSaiAclConvertRulePriorityToXpPriority(
                         pSaiAclEntryAttribute->priority);

    retVal = xpsAclWriteEntry(devId, tableId, xpRulePriority, entryId,
                              &aclFieldData, aclActionData, keyFormat, false);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to update ACL entry, ret = %u \n", retVal);
        xpFree(acl_value);
        xpFree(acl_mask);
        xpFree(aclFieldList);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}
static sai_status_t xpSaiAclUpdateKeyType(xpSaiAclTableIdMappingT
                                          *pSaiAclTableAttribute, xpSaiAclEntryAttributesT *attributes)
{
    switch (pSaiAclTableAttribute->keyType)
    {
        case XP_SAI_KEY_NON_IP:
            attributes->entryType = SAI_ACL_IP_TYPE_NON_IP;
            break;
        case XP_SAI_KEY_IPV4:
            attributes->entryType = SAI_ACL_IP_TYPE_IPV4ANY;
            break;
        case XP_SAI_KEY_IPV6:
            attributes->entryType = SAI_ACL_IP_TYPE_IPV6ANY;
            break;
        case XP_SAI_KEY_IPV4_AND_IPV6:
            attributes->entryType = SAI_ACL_IP_TYPE_ANY;
            break;
        default:
            XP_SAI_LOG_ERR("Invalid parameter \n");
            return SAI_STATUS_INVALID_PARAMETER;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCreateAclEntry(sai_object_id_t *acl_entry_id,
                                 sai_object_id_t switch_id,
                                 sai_uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t                saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t                entryId     = 0;
    sai_uint32_t                devId = (sai_uint32_t)xpSaiObjIdValueGet(switch_id);
    static xpSaiAclEntryAttributesT    attributes;
    xpSaiAclTableIdMappingT     *pSaiAclTableAttribute = NULL;
    sai_acl_action_data_t   action;
    uint32_t xpsNhId = 0;

    memset(&attributes, 0x00, sizeof(attributes));

    if (acl_entry_id == NULL)
    {
        XP_SAI_LOG_ERR("Invalid parameter \n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               ACL_ENTRY_VALIDATION_ARRAY_SIZE, acl_entry_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    xpSaiSetDefaultAclEntryAttributeVals(&attributes);

    if ((saiRetVal = xpSaiAclEntryAttributesUpdate(attr_count, attr_list,
                                                   &attributes)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to parse the attributes\n");
        return saiRetVal;
    }

    /* Get table info */
    if ((saiRetVal = xpSaiAclTableAttributesGet(attributes.tableId,
                                                &pSaiAclTableAttribute)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:, Failed to get table %u related data\n", __FUNCNAME__,
                       attributes.tableId);
        return saiRetVal;
    }


    if (!attributes.isEntryTypeValid)
    {
        if ((saiRetVal = xpSaiAclUpdateKeyType(pSaiAclTableAttribute,
                                               &attributes) != SAI_STATUS_SUCCESS))
        {
            XP_SAI_LOG_ERR("Failed to update keyType\n");
            return saiRetVal;
        }

    }
    if ((saiRetVal = xpSaiAclValidateEntryAttr(attr_count, attr_list,
                                               pSaiAclTableAttribute)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:, Failed in validation \n", __FUNCNAME__,
                       attributes.tableId);
        return saiRetVal;
    }
    saiRetVal = xpSaiAclMapperEntryCreate(&entryId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to add entry into DB, ret = %u \n", saiRetVal);
        return saiRetVal;
    }

    /* We have to install one rule for every port it is assigned to */

    saiRetVal = xpSaiWriteAclEntry(devId, attributes.tableId, entryId, &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not create Ingress ACL, ret %d\n", saiRetVal);
        return saiRetVal;
    }

    /*Increment NH ref-count if set*/
    if (xpSaiAclEntryActionAttributeEnabledCheck(&attributes,
                                                 SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT) == TRUE)
    {
        sai_object_type_t saiObjType = SAI_OBJECT_TYPE_NULL;
        xpSaiAclEntryActionAttributeGet(&attributes, SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT,
                                        &action);
        saiObjType = sai_object_type_query(action.parameter.oid);
        xpsNhId = (uint32_t)xpSaiObjIdValueGet(action.parameter.oid);
        if (saiObjType == SAI_OBJECT_TYPE_NEXT_HOP)
        {
            saiRetVal = xpSaiIncrementNextHopRouteRefCount(xpsNhId, false);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiIncrementNextHopRefCount failed, retVal : %d \n",
                               saiRetVal);
                return saiRetVal;
            }
        }
        else if (saiObjType == SAI_OBJECT_TYPE_NEXT_HOP_GROUP)
        {
            saiRetVal = xpSaiIncrementNextHopGroupRouteRefCount(xpsNhId, false);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiIncrementNextHopRefCount failed, retVal : %d \n",
                               saiRetVal);
                return saiRetVal;
            }
        }
    }

    attributes.rule_count++;

    saiRetVal = xpSaiAclMapperEntryAttributesSet(entryId, &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set entry attributes in DB\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiAclEntryCounterApply(devId, entryId, &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to apply the attributes\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiAclMapperAddEntryInTable(attributes.tableId, entryId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to map entry id to tablei err %d \n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ACL_ENTRY, devId,
                                 (sai_uint64_t) entryId, acl_entry_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created\n");
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t xpSaiRemoveAclEntry(sai_object_id_t acl_entry_id)
{
    sai_status_t        saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t        entryId     = 0;
    sai_uint32_t        rangeEntryId     = 0;
    xpSaiAclEntryAttributesT        *pSaiAclEntryAttributes =NULL;
    xpSaiAclTableIdMappingT         *pSaiAclTableAttribute = NULL;
    xpsDevice_t         devId       = xpSaiGetDevId();
    sai_uint32_t counterId            = 0;
    sai_acl_action_data_t   action;
    uint32_t xpsNhId = 0;
    uint32_t hwLeafId = 0;
    uint32_t intfId = 0xFFFFFFFF;
    bool curRedirectEna = false;
    sai_object_type_t saiObjType = SAI_OBJECT_TYPE_NULL;
    sai_acl_action_data_t   curAction;
    sai_object_id_t curRedirectOid = SAI_NULL_OBJECT_ID;
    xpsScope_t  scopeId     = 0;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsL3NhSwDbEntry_t *pxpsNhSwDbInfo = NULL;
    xpsL3NextHopGroupEntry_t* pNhGrpEntry = NULL;
    memset(&curAction, 0, sizeof(curAction));

    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    saiRetVal = xpSaiConvertAclEntryOid(acl_entry_id, &entryId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        if (saiRetVal == SAI_STATUS_INVALID_OBJECT_TYPE)
        {
            XP_SAI_LOG_ERR("Failed to convert ACL entry oid");
            return saiRetVal;
        }
        else
        {
            return saiRetVal;
        }
    }

    saiRetVal = xpSaiAclMapperEntryAttributesGet(entryId, &pSaiAclEntryAttributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get entry attributes from DB\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiAclTableAttributesGet(pSaiAclEntryAttributes->tableId,
                                           &pSaiAclTableAttribute);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get table %u related data\n",
                       pSaiAclEntryAttributes->tableId);
        return saiRetVal;
    }

    xpSaiAclEntryActionAttributeGet(pSaiAclEntryAttributes,
                                    SAI_ACL_ENTRY_ATTR_ACTION_COUNTER, &action);

    saiRetVal = xpSaiConvertAclCounterOid(action.parameter.oid, &counterId);
    if (saiRetVal == SAI_STATUS_SUCCESS)
    {
        saiRetVal = xpSaiAclMapperRemoveEntryInCounter(counterId, entryId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to delete counter%u from entry %u in xpSaiAclMapperRemoveEntryInCounter \n",
                           counterId, entryId);
            return saiRetVal;
        }
    }

    curRedirectEna = xpSaiAclEntryActionAttributeEnabledCheck(
                         pSaiAclEntryAttributes,
                         SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT);
    xpSaiAclEntryActionAttributeGet(pSaiAclEntryAttributes,
                                    SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT,
                                    &curAction);
    curRedirectOid = curAction.parameter.oid;
    if ((curRedirectEna) && (curRedirectOid != SAI_NULL_OBJECT_ID))
    {
        saiObjType = sai_object_type_query(curAction.parameter.oid);
        xpsNhId = (uint32_t)xpSaiObjIdValueGet(curAction.parameter.oid);
        if (saiObjType == SAI_OBJECT_TYPE_NEXT_HOP)
        {
            xpsRetVal = xpsL3GetRouteNextHopSwDb(scopeId, xpsNhId, &pxpsNhSwDbInfo);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Could not get next hop, nhId %d, xpsRetVal %d",
                      xpsNhId, xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }

            saiRetVal = xpSaiDecrementNextHopRouteRefCount(xpsNhId, false);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiDecrementNextHopRefCount failed, retVal : %d \n",
                               saiRetVal);
                return saiRetVal;
            }
            saiRetVal = xpSaiIsAclNextHopInUse(devId, xpsNhId);
            if (saiRetVal == SAI_STATUS_SUCCESS)
            {
                if (pxpsNhSwDbInfo->hwNhId != INVALID_INDEX)
                {
                    xpsRetVal =   xpsL3NextHopMapRemove(xpsNhId, pxpsNhSwDbInfo->hwNhId);
                    if (xpsRetVal != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "xpsL3RemoveIpRouteEntry remMap failed nhId  %u hwNhId %u ret %d\n",
                              xpsNhId, pxpsNhSwDbInfo->hwNhId, xpsRetVal);
                        return xpsRetVal;
                    }
                }
                hwLeafId = pxpsNhSwDbInfo->hwPbrLeafId;
                /*Release a new Leaf Idx corresponding to NH*/
                xpsRetVal = xpsAllocatorReleaseId(scopeId, XP_ALLOC_PBR_HW_ENTRY, hwLeafId);
                if (xpsRetVal != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "xpsAllocatorAllocateId Failed : %d\n", xpsRetVal);
                    return xpsStatus2SaiStatus(xpsRetVal);
                }
                pxpsNhSwDbInfo->hwPbrLeafId = XPS_INTF_INVALID_ID;
            }
        }
        else if (saiObjType == SAI_OBJECT_TYPE_NEXT_HOP_GROUP)
        {
            saiRetVal = xpSaiDecrementNextHopGroupRouteRefCount(xpsNhId, false);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiDecrementNextHopRefCount failed, retVal : %d \n",
                               saiRetVal);
                return saiRetVal;
            }
            saiRetVal = xpSaiIsAclNextHopGroupInUse(xpsNhId);
            if (saiRetVal == SAI_STATUS_SUCCESS)
            {
                xpsRetVal = xpsL3GetRouteNextHopGroup(scopeId, xpsNhId, &pNhGrpEntry);
                if (xpsRetVal != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Could not get next hop, nhId %d, xpsRetVal %d",
                          xpsNhId, xpsRetVal);
                    return xpsStatus2SaiStatus(xpsRetVal);
                }
                hwLeafId = pNhGrpEntry->hwPbrLeafId;
                /*Release a new Leaf Idx corresponding to NH*/
                xpsRetVal = xpsAllocatorReleaseId(scopeId, XP_ALLOC_PBR_HW_ENTRY, hwLeafId);
                if (xpsRetVal != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "xpsAllocatorAllocateId Failed : %d\n", xpsRetVal);
                    return xpsStatus2SaiStatus(xpsRetVal);
                }
                pNhGrpEntry->hwPbrLeafId = XPS_INTF_INVALID_ID;
            }
        }
    }

    if (pSaiAclEntryAttributes->in_ports_count &&
        pSaiAclEntryAttributes->isMirrorEnable)
    {
        for (uint32_t idx = 0 ; idx < pSaiAclEntryAttributes->in_ports_count; idx++)
        {
            intfId = xpsLocalPortToGlobalPortnum(devId,
                                                 pSaiAclEntryAttributes->in_ports[idx]);
            xpsRetVal = xpsMirrorUpdateErSpan2Index(devId, intfId, true, false);
            if (xpsRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("%s:, Failed in validation \n", __FUNCNAME__,
                               "xpsMirrorUpdateErSpan2Index Failed %d \n", xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }
        }
    }
    if (pSaiAclEntryAttributes->out_ports_count &&
        pSaiAclEntryAttributes->isMirrorEnable)
    {
        uint32_t analyzerId = 0xFFFFFFFF;
        saiRetVal = xpSaiMirrorGetSessionAnalyzerId(pSaiAclEntryAttributes->mirrorId,
                                                    &analyzerId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get MirrIdx (%d)\n",
                           pSaiAclEntryAttributes->mirrorId);
            return saiRetVal;
        }

        for (uint32_t idx = 0 ; idx < pSaiAclEntryAttributes->out_ports_count; idx++)
        {
            intfId = xpsLocalPortToGlobalPortnum(devId,
                                                 pSaiAclEntryAttributes->out_ports[idx]);
            xpsRetVal = xpsMirrorUpdateErSpan2Index(devId, intfId, false, false);
            if (xpsRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("%s:, Failed in validation \n", __FUNCNAME__,
                               "xpsMirrorUpdateErSpan2Index Failed %d \n", xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }

            xpsRetVal = xpsAclErspanRuleUpdate(devId, intfId, false, analyzerId);
            if (xpsRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to enable mirror on port %d with error code %d\n",
                               intfId, xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }
        }
    }

    if (pSaiAclEntryAttributes->isSampleEnable)
    {
        xpsSamplePacket_t *xpSamplePacketEntry = NULL;
        saiRetVal = xpSaiSamplePacketDbInfoGet(pSaiAclEntryAttributes->sampleId,
                                               &xpSamplePacketEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to locate session %" PRIu64
                           " in database: error code: %d\n", pSaiAclEntryAttributes->sampleId, saiRetVal);
            return saiRetVal;
        }
        if (xpSamplePacketEntry->type == SAI_SAMPLEPACKET_TYPE_SLOW_PATH)
        {
            if (xpSamplePacketEntry->mirrorSessionId != SAI_NULL_OBJECT_ID)
            {
                uint32_t analyzerId = XP_MIRROR_ILLEGAL_SESSION;
                sai_mirror_session_type_t sessionType = SAI_MIRROR_SESSION_TYPE_LOCAL;
                if ((saiRetVal = xpSaiMirrorAclMirrorEnable(
                                     xpSamplePacketEntry->mirrorSessionId,
                                     true, false, &analyzerId, &sessionType)) != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to unset acl mirror |Error: %d\n", saiRetVal);
                    return saiRetVal;
                }
                xpSamplePacketEntry->sampleRefCnt --;
            }
        }
        else
        {
            if (xpSamplePacketEntry->sampleRefCnt > 0)
            {
                xpSamplePacketEntry->sampleRefCnt --;
            }
        }
        pSaiAclEntryAttributes->isSampleEnable = GT_FALSE;
        pSaiAclEntryAttributes->sampleId = SAI_NULL_OBJECT_ID;
    }

    /*For range objects all the subrules including the base rule must be deleted*/
    if ((pSaiAclEntryAttributes->entryIdCount > 1))
    {
        for (sai_uint16_t subIntervalIndex = 1;
             subIntervalIndex < pSaiAclEntryAttributes->entryIdCount; subIntervalIndex++)
        {
            rangeEntryId = pSaiAclEntryAttributes->entryIds[subIntervalIndex];

            saiRetVal = xpsAclDeleteEntry(devId, pSaiAclEntryAttributes->tableId,
                                          rangeEntryId, pSaiAclTableAttribute->isSwitchAclEn);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to delete entry from HW %d\n", rangeEntryId);
                return saiRetVal;
            }
            saiRetVal = xpSaiAclMapperEntryKeyDeleteWithoutCntr(devId, rangeEntryId);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to delete entry %u from table %u in SAI ACL mapper\n",
                               rangeEntryId, pSaiAclEntryAttributes->tableId);
                return saiRetVal;
            }
        }

    }

    saiRetVal = xpsAclDeleteEntry(devId, pSaiAclEntryAttributes->tableId, entryId,
                                  pSaiAclTableAttribute->isSwitchAclEn);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to delete entry from HW %d\n", entryId);
        return saiRetVal;
    }

    saiRetVal = xpSaiAclMapperEntryDelete(devId, entryId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to delete entry %u from table %u in SAI ACL mapper\n",
                       entryId, pSaiAclEntryAttributes->tableId);
        return saiRetVal;
    }

    saiRetVal = xpSaiAclMapperRemoveEntryInTable(pSaiAclTableAttribute->tableId,
                                                 entryId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to delete entry %u from table %u in xpSaiAclMapperRemoveEntryInTable \n",
                       entryId, pSaiAclEntryAttributes->tableId);
        return saiRetVal;
    }

    return saiRetVal;
}

//Func: xpSaiSetAclEntryAttribute

sai_status_t xpSaiSetAclEntryAttribute(sai_object_id_t acl_entry_id,
                                       const sai_attribute_t *attr)
{
    sai_status_t             saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t             entry_id    = 0;
    sai_uint32_t             count       = 0;
    xpSaiAclEntryAttributesT *pSaiAclEntryAttributes = NULL;
    xpSaiAclEntryAttributesT prevSaiAclEntryAttributes;
    xpSaiAclTableIdMappingT  *pSaiAclTableAttribute = NULL;
    xpsDevice_t devId = xpSaiGetDevId();
    sai_object_type_t saiObjType = SAI_OBJECT_TYPE_NULL;
    sai_acl_action_data_t   prevAction;
    sai_acl_action_data_t   curAction;
    sai_object_id_t prevRedirectOid = SAI_NULL_OBJECT_ID;
    sai_object_id_t curRedirectOid = SAI_NULL_OBJECT_ID;
    bool prevRedirectEna = false;
    bool curRedirectEna = false;
    uint32_t xpsNhId = 0;
    uint32_t hwLeafId = 0;
    uint32_t intfId = 0xFFFFFFFF;
    xpsL3NhSwDbEntry_t *pxpsNhSwDbInfo = NULL;
    xpsL3NextHopGroupEntry_t* pNhGrpEntry = NULL;
    xpsScope_t  scopeId     = 0;
    XP_STATUS xpsRetVal = XP_NO_ERR;

    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpsRetVal);
    }


    saiRetVal = xpSaiAttrCheck(1, attr,
                               ACL_ENTRY_VALIDATION_ARRAY_SIZE, acl_entry_attribs,
                               SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertAclEntryOid(acl_entry_id, &entry_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        if (saiRetVal == SAI_STATUS_INVALID_OBJECT_TYPE)
        {
            XP_SAI_LOG_ERR("Failed to convert ACL entry oid");
            return saiRetVal;
        }
        else
        {
            return saiRetVal;
        }
    }

    saiRetVal = xpSaiAclMapperEntryAttributesGet(entry_id, &pSaiAclEntryAttributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get entry attributes from DB\n");
        return saiRetVal;
    }
    memset(&prevSaiAclEntryAttributes, 0, sizeof(prevSaiAclEntryAttributes));
    memcpy(&prevSaiAclEntryAttributes, pSaiAclEntryAttributes,
           sizeof(prevSaiAclEntryAttributes));

    /* Get table info */
    saiRetVal = xpSaiAclTableAttributesGet(pSaiAclEntryAttributes->tableId,
                                           &pSaiAclTableAttribute);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get table %u related data, error %u\n",
                       pSaiAclEntryAttributes->tableId, saiRetVal);
        return saiRetVal;
    }

    if ((saiRetVal = xpSaiAclValidateEntryAttr(1, attr,
                                               pSaiAclTableAttribute)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:, Failed in validation \n", __FUNCNAME__,
                       pSaiAclTableAttribute->tableId);
        return saiRetVal;
    }

    /*Clean-up previous in_ports/out_ports, if any*/
    if (prevSaiAclEntryAttributes.in_ports_count &&
        prevSaiAclEntryAttributes.isMirrorEnable)
    {
        for (uint32_t idx = 0 ; idx < prevSaiAclEntryAttributes.in_ports_count; idx++)
        {
            intfId = xpsLocalPortToGlobalPortnum(devId,
                                                 prevSaiAclEntryAttributes.in_ports[idx]);
            xpsRetVal = xpsMirrorUpdateErSpan2Index(devId, intfId, true, false);
            if (xpsRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("%s:, Failed in validation \n", __FUNCNAME__,
                               "xpsMirrorUpdateErSpan2Index Failed %d \n", xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }
        }
    }
    if (prevSaiAclEntryAttributes.out_ports_count &&
        prevSaiAclEntryAttributes.isMirrorEnable)
    {
        uint32_t analyzerId = 0xFFFFFFFF;
        saiRetVal = xpSaiMirrorGetSessionAnalyzerId(prevSaiAclEntryAttributes.mirrorId,
                                                    &analyzerId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get MirrIdx (%d)\n",
                           pSaiAclEntryAttributes->mirrorId);
            return saiRetVal;
        }

        for (uint32_t idx = 0 ; idx < prevSaiAclEntryAttributes.out_ports_count; idx++)
        {
            intfId = xpsLocalPortToGlobalPortnum(devId,
                                                 prevSaiAclEntryAttributes.out_ports[idx]);
            xpsRetVal = xpsMirrorUpdateErSpan2Index(devId, intfId, false, false);
            if (xpsRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("%s:, Failed in validation \n", __FUNCNAME__,
                               "xpsMirrorUpdateErSpan2Index Failed %d \n", xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }
            xpsRetVal = xpsAclErspanRuleUpdate(devId, intfId, false, analyzerId);
            if (xpsRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to enable mirror on port %d with error code %d\n",
                               intfId, xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }
        }
    }

    saiRetVal = xpSaiAclEntryAttributeUpdate(attr, pSaiAclEntryAttributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set attribute %s(%d)\n",
                       xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)attr->id),
                       attr->id);
        return saiRetVal;
    }

    saiRetVal = xpSaiAclMapperEntryAttributesSet(entry_id, pSaiAclEntryAttributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set entry attributes in DB\n");
        return saiRetVal;
    }

    for (count = 0; count < pSaiAclEntryAttributes->rule_count; count++)
    {
        saiRetVal = xpSaiUpdateAclEntry(devId, pSaiAclEntryAttributes->tableId,
                                        entry_id, pSaiAclEntryAttributes->ruleMap[count].id, attr,
                                        pSaiAclEntryAttributes);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not update acl rule, ret %d\n", saiRetVal);
            return saiRetVal;
        }
    }



    memset(&prevAction, 0, sizeof(prevAction));
    memset(&curAction, 0, sizeof(curAction));

    prevRedirectEna = xpSaiAclEntryActionAttributeEnabledCheck(
                          &prevSaiAclEntryAttributes,
                          SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT);
    curRedirectEna = xpSaiAclEntryActionAttributeEnabledCheck(
                         pSaiAclEntryAttributes,
                         SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT);
    xpSaiAclEntryActionAttributeGet(&prevSaiAclEntryAttributes,
                                    SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT,
                                    &prevAction);
    xpSaiAclEntryActionAttributeGet(pSaiAclEntryAttributes,
                                    SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT,
                                    &curAction);

    prevRedirectOid = prevAction.parameter.oid;
    curRedirectOid = curAction.parameter.oid;

    if ((prevRedirectOid != curRedirectOid) || (prevRedirectEna != curRedirectEna))
    {
        if ((prevRedirectEna) && (prevRedirectOid != SAI_NULL_OBJECT_ID))
        {
            saiObjType = sai_object_type_query(prevAction.parameter.oid);
            xpsNhId = (uint32_t)xpSaiObjIdValueGet(prevAction.parameter.oid);
            if (saiObjType == SAI_OBJECT_TYPE_NEXT_HOP)
            {
                xpsRetVal = xpsL3GetRouteNextHopSwDb(scopeId, xpsNhId, &pxpsNhSwDbInfo);
                if (xpsRetVal != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Could not get next hop, nhId %d, xpsRetVal %d",
                          xpsNhId, xpsRetVal);
                    return xpsStatus2SaiStatus(xpsRetVal);
                }

                saiRetVal = xpSaiDecrementNextHopRouteRefCount(xpsNhId, false);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiDecrementNextHopRefCount failed, retVal : %d \n",
                                   saiRetVal);
                    return saiRetVal;
                }
                saiRetVal = xpSaiIsAclNextHopInUse(devId, xpsNhId);
                if (saiRetVal == SAI_STATUS_SUCCESS)
                {
                    if (pxpsNhSwDbInfo->hwNhId != INVALID_INDEX)
                    {
                        xpsRetVal =   xpsL3NextHopMapRemove(xpsNhId, pxpsNhSwDbInfo->hwNhId);
                        if (xpsRetVal != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "xpsL3RemoveIpRouteEntry remMap failed nhId  %u hwNhId %u ret %d\n",
                                  xpsNhId, pxpsNhSwDbInfo->hwNhId, xpsRetVal);
                            return xpsRetVal;
                        }
                    }

                    hwLeafId = pxpsNhSwDbInfo->hwPbrLeafId;
                    /*Release a new Leaf Idx corresponding to NH*/
                    xpsRetVal = xpsAllocatorReleaseId(scopeId, XP_ALLOC_PBR_HW_ENTRY, hwLeafId);
                    if (xpsRetVal != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "xpsAllocatorReleaseId Failed : %d\n", xpsRetVal);
                        return xpsStatus2SaiStatus(xpsRetVal);
                    }
                    pxpsNhSwDbInfo->hwPbrLeafId = XPS_INTF_INVALID_ID;
                }
            }
            else if (saiObjType == SAI_OBJECT_TYPE_NEXT_HOP_GROUP)
            {
                saiRetVal = xpSaiDecrementNextHopGroupRouteRefCount(xpsNhId, false);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiDecrementNextHopRefCount failed, retVal : %d \n",
                                   saiRetVal);
                    return saiRetVal;
                }
                saiRetVal = xpSaiIsAclNextHopGroupInUse(xpsNhId);
                if (saiRetVal == SAI_STATUS_SUCCESS)
                {
                    xpsRetVal = xpsL3GetRouteNextHopGroup(scopeId, xpsNhId, &pNhGrpEntry);
                    if (xpsRetVal != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Could not get next hop, nhId %d, xpsRetVal %d",
                              xpsNhId, xpsRetVal);
                        return xpsStatus2SaiStatus(xpsRetVal);
                    }
                    hwLeafId = pNhGrpEntry->hwPbrLeafId;
                    /*Release a new Leaf Idx corresponding to NH*/
                    xpsRetVal = xpsAllocatorReleaseId(scopeId, XP_ALLOC_PBR_HW_ENTRY, hwLeafId);
                    if (xpsRetVal != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "xpsAllocatorAllocateId Failed : %d\n", xpsRetVal);
                        return xpsStatus2SaiStatus(xpsRetVal);
                    }
                    pNhGrpEntry->hwPbrLeafId = XPS_INTF_INVALID_ID;
                }
            }
        }

        if ((curRedirectEna) && (curRedirectOid != SAI_NULL_OBJECT_ID))
        {
            saiObjType = sai_object_type_query(curAction.parameter.oid);
            xpsNhId = (uint32_t)xpSaiObjIdValueGet(curAction.parameter.oid);
            if (saiObjType == SAI_OBJECT_TYPE_NEXT_HOP)
            {
                saiRetVal = xpSaiIncrementNextHopRouteRefCount(xpsNhId, false);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiIncrementNextHopRefCount failed, retVal : %d \n",
                                   saiRetVal);
                    return saiRetVal;
                }
            }
            else if (saiObjType == SAI_OBJECT_TYPE_NEXT_HOP_GROUP)
            {
                saiRetVal = xpSaiIncrementNextHopGroupRouteRefCount(xpsNhId, false);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiIncrementNextHopRefCount failed, retVal : %d \n",
                                   saiRetVal);
                    return saiRetVal;
                }
            }
        }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetAclEntryAttribute

static sai_status_t xpSaiGetAclEntryAttribute(xpSaiAclEntryAttributesT
                                              *attributes, sai_attribute_t *attr, sai_uint32_t attr_index)
{
    sai_uint32_t countIdx = 0;
    if ((attributes == NULL) || (attr == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (attr->id)
    {
        case SAI_ACL_ENTRY_ATTR_TABLE_ID:
            {
                attr->value.oid = attributes->tableOid;
                break;
            }
        case SAI_ACL_ENTRY_ATTR_PRIORITY:
            {
                attr->value.u32 = attributes->priority;
                break;
            }
        case SAI_ACL_ENTRY_ATTR_ADMIN_STATE:
            {
                attr->value.booldata = attributes->adminState;
                break;
            }
        case SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE:
            {
                memcpy(attr->value.aclfield.data.objlist.list, attributes->aclRangeObjList.list,
                       sizeof(sai_object_id_t)*(attributes->aclRangeObjList.count));
                attr->value.aclfield.data.objlist.count = attributes->aclRangeObjList.count;
                break;
            }

        default:
            {
                if ((attr->id >= SAI_ACL_ENTRY_ATTR_FIELD_START) &&
                    (attr->id <= SAI_ACL_ENTRY_ATTR_FIELD_END))
                {
                    sai_uint32_t matchId = attr->id - SAI_ACL_ENTRY_ATTR_FIELD_START;

                    if (matchId >= XP_SAI_ACL_MATCH_NUMBER_MAX)
                    {
                        XP_SAI_LOG_ERR("Entry attribute %d is out of range\n", attr->id);
                        return SAI_STATUS_INVALID_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index);
                    }

                    if (attributes->match[matchId].isSetByUser == FALSE)
                    {
                        XP_SAI_LOG_INFO("Entry attribute %d is not set\n", attr->id);
                    }

                    if (attributes->match[matchId].entryValue.enable)
                    {
                        attr->value.aclfield = attributes->match[matchId].entryValue;
                    }
                    else
                    {
                        memset(&attr->value.aclfield, 0, sizeof(sai_acl_field_data_t));
                    }
                }
                else if ((attr->id >= SAI_ACL_ENTRY_ATTR_ACTION_START) &&
                         (attr->id <= SAI_ACL_ENTRY_ATTR_ACTION_END))
                {
                    sai_uint32_t actionId = attr->id - SAI_ACL_ENTRY_ATTR_ACTION_START;

                    if (actionId >= XP_SAI_ACL_ACTION_NUMBER_MAX)
                    {
                        XP_SAI_LOG_ERR("Entry attribute %d is out of range\n", attr->id);
                        return (SAI_STATUS_INVALID_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index));
                    }

                    if (attributes->action[actionId].isSetByUser == FALSE)
                    {
                        XP_SAI_LOG_ERR("Entry attribute %s(%d) is not set\n",
                                       xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)attr->id),
                                       attr->id);

                        return SAI_STATUS_INVALID_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index);
                    }

                    if (attributes->action[actionId].actionValue.enable &&
                        xpSaiValidateEntryListAttributes((sai_acl_entry_attr_t)attr->id))
                    {
                        attr->value.aclaction.enable = attributes->action[actionId].actionValue.enable;
                        attr->value.aclaction.parameter.objlist.count =
                            attributes->action[actionId].actionValue.parameter.objlist.count;
                        for (countIdx = 0;
                             countIdx < attributes->action[actionId].actionValue.parameter.objlist.count;
                             countIdx++)
                        {
                            attr->value.aclaction.parameter.objlist.list[countIdx] =
                                attributes->action[actionId].actionValue.parameter.objlist.list[countIdx];
                        }
                    }
                    else if (!attributes->action[actionId].actionValue.enable &&
                             xpSaiValidateEntryListAttributes((sai_acl_entry_attr_t)attr->id))
                    {
                        attr->value.aclaction.enable = attributes->action[actionId].actionValue.enable;
                    }
                    else
                    {
                        attr->value.aclaction = attributes->action[actionId].actionValue;
                    }
                }
                else
                {
                    XP_SAI_LOG_ERR("Attribute %d is unknown\n", attr->id);
                    return (SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index));
                }
                break;
            }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetAclEntryAttributes

static sai_status_t xpSaiGetAclEntryAttributes(sai_object_id_t acl_entry_id,
                                               sai_uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    sai_uint32_t        entryId         = 0;

    xpSaiAclEntryAttributesT *pSaiAclEntryAttributes = NULL;

    saiRetVal = xpSaiConvertAclEntryOid(acl_entry_id, &entryId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        if (saiRetVal == SAI_STATUS_INVALID_OBJECT_TYPE)
        {
            XP_SAI_LOG_ERR("Failed to convert ACL entry oid");
            return saiRetVal;
        }
        else
        {
            return saiRetVal;
        }
    }

    if (attr_count == 0)
    {
        XP_SAI_LOG_ERR("No attributes requested\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               ACL_ENTRY_VALIDATION_ARRAY_SIZE, acl_entry_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }


    saiRetVal = xpSaiAclMapperEntryAttributesGet(entryId, &pSaiAclEntryAttributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get entry attributes from DB\n");
        return saiRetVal;
    }

    for (sai_uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetAclEntryAttribute(pSaiAclEntryAttributes, &attr_list[count],
                                              count);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get entry attribute %d\n", count);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t xpSaiCreateAclRange(sai_object_id_t *acl_range_id,
                                 sai_object_id_t switch_id, sai_uint32_t attr_count, const sai_attribute_t *attr)
{

    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t    devId       = 0;
    sai_uint32_t                entryId     = 0;

    xpSaiAclRangeAttributesT  attributes;

    if (!XDK_SAI_OBJID_TYPE_CHECK(switch_id, SAI_OBJECT_TYPE_SWITCH))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(switch_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr,
                               ACL_RANGE_VALIDATION_ARRAY_SIZE, acl_range_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    devId = (sai_uint32_t)xpSaiObjIdValueGet(switch_id);

    memset(&attributes, 0x00, sizeof(xpSaiAclRangeAttributesT));

    if ((saiRetVal = xpSaiAclRangeAttributesUpdate(attr_count, attr,
                                                   &attributes)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to parse the attributes\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiAclMapperAclRangeCreate(&entryId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to add entry into DB, ret = %u \n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiAclMapperAclRangeAttributesSet(entryId, &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set entry attributes in DB\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ACL_RANGE, devId,
                                 (sai_uint64_t) entryId, acl_range_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created\n");
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}


static sai_status_t xpSaiAclValidateRangeAttr(const sai_attribute_t *attr)
{
    if (attr -> id ==SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE)
    {
        XP_SAI_LOG_ERR("Must not update range type\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiSetAclRangeAttribute(sai_object_id_t acl_range_id,
                                       const sai_attribute_t *attr)
{
    sai_status_t             saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t             entry_id    = 0;
    xpSaiAclRangeAttributesT *pSaiAclRangeAttributes = NULL;

    saiRetVal = xpSaiAttrCheck(1, attr,
                               ACL_RANGE_VALIDATION_ARRAY_SIZE, acl_range_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(acl_range_id, SAI_OBJECT_TYPE_ACL_RANGE))
    {
        XP_SAI_LOG_DBG("Unknown object 0x%" PRIx64 "\n", acl_range_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    entry_id = (sai_uint32_t)xpSaiObjIdValueGet(acl_range_id);

    saiRetVal = xpSaiAclMapperAclRangeAttributesGet(entry_id,
                                                    &pSaiAclRangeAttributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get entry attributes from DB\n");
        return saiRetVal;
    }


    if ((saiRetVal = xpSaiAclValidateRangeAttr(attr)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("%s:, Failed in validation \n", __FUNCNAME__);
        return saiRetVal;
    }

    saiRetVal = xpSaiAclRangeAttributeUpdate(attr, pSaiAclRangeAttributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set attribute %s(%d)\n",
                       xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)attr->id),
                       attr->id);
        return saiRetVal;
    }

    saiRetVal = xpSaiAclMapperAclRangeAttributesSet(entry_id,
                                                    pSaiAclRangeAttributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set entry attributes in DB\n");
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;

}


sai_status_t xpSaiAclRangeAttributesUpdate(const sai_uint32_t attr_count,
                                           const sai_attribute_t* attr_list, xpSaiAclRangeAttributesT* pAttributes)
{
    sai_status_t        retVal           = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Parsing %u attributes\n", attr_count);

    for (sai_uint32_t count = 0; count < attr_count; count++)
    {
        retVal = xpSaiAclRangeAttributeUpdate(attr_list + count, pAttributes);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to set attribute (%d)\n",
                           attr_list[count].id);
            return (retVal + SAI_STATUS_CODE(count));
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiAclRangeAttributeUpdate(const sai_attribute_t* pAttr,
                                          xpSaiAclRangeAttributesT* pAttributes)
{

    XP_SAI_LOG_DBG("attr %u, name %s\n", pAttr->id,
                   xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)pAttr->id));

    switch (pAttr->id)
    {
        case SAI_ACL_RANGE_ATTR_TYPE:

            pAttributes->rangeType =(sai_acl_range_type_t)pAttr -> value.s32;
            break;

        case SAI_ACL_RANGE_ATTR_LIMIT:

            pAttributes->from = pAttr->value.u32range.min;
            pAttributes->to = pAttr->value.u32range.max;
            break;
        default:
            XP_SAI_LOG_ERR("Wrong range type received.\n");
            return SAI_STATUS_INVALID_PARAMETER;


    }
    return SAI_STATUS_SUCCESS;
}

#if 0
static sai_status_t  xpSaiAclSetXpsComparator(xpSaiAclEntryAttributesT*
                                              pAttributes)
{
    sai_status_t                saiRetVal   = SAI_STATUS_SUCCESS;
    XP_STATUS                   retVal = XP_NO_ERR;
    xpsDevice_t                 devId = xpSaiGetDevId();
    xpsAclKeyFormat            keyFormat;
    sai_uint32_t                entry_id    = 0;
    sai_object_id_t             acl_range_id;
    XPS_L4_PROTOCOL_PORT_TYPE   xpsL4PortType;
    xpSaiAclRangeAttributesT    *saiAclRangeAttributes;
    sai_uint8_t                 index;


    for (sai_uint32_t i=0; i<pAttributes->aclRangeObjList.count; i++)
    {

        acl_range_id = *(pAttributes->aclRangeObjList.list + i);
        if (!XDK_SAI_OBJID_TYPE_CHECK(acl_range_id, SAI_OBJECT_TYPE_ACL_RANGE))
        {
            XP_SAI_LOG_DBG("Unknown object 0x%" PRIx64 "\n", acl_range_id);
            return SAI_STATUS_INVALID_OBJECT_TYPE;
        }

        entry_id = (sai_uint32_t)xpSaiObjIdValueGet(acl_range_id);

        saiRetVal = xpSaiAclMapperAclRangeAttributesGet(entry_id,
                                                        &saiAclRangeAttributes);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get entry attributes from DB\n");
            return saiRetVal;
        }
        if (!saiAclRangeAttributes->isRangeSet)
        {
            switch (saiAclRangeAttributes->rangeType)
            {
                case SAI_ACL_RANGE_TYPE_L4_SRC_PORT_RANGE:
                    xpsL4PortType = XPS_L4_PROTOCOL_PORT_SRC;
                    break;
                case SAI_ACL_RANGE_TYPE_L4_DST_PORT_RANGE:
                    xpsL4PortType = XPS_L4_PROTOCOL_PORT_DST;
                    break;

                default:
                    XP_SAI_LOG_ERR("invalid l4 port type\n");
                    return SAI_STATUS_INVALID_PARAMETER;
                    break;
            }
            saiRetVal = xpSaiGetIaclKeyFormat(pAttributes->tableId, pAttributes->entryType,
                                              &keyFormat);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Error: xpSaiGetIaclKeyFormat failed\n");
                return saiRetVal;
            }

            retVal = xpsAclAddL4PortRange(devId, keyFormat, xpsL4PortType,
                                          saiAclRangeAttributes->from, saiAclRangeAttributes->to, &index);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to set comparator, ret = %u \n", retVal);
                return xpsStatus2SaiStatus(retVal);
            }


            saiAclRangeAttributes->xpsComparatorIndex = index;
            saiAclRangeAttributes->isRangeSet = true;

            saiRetVal = xpSaiAclMapperAclRangeAttributesSet(entry_id,
                                                            saiAclRangeAttributes);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to set range attributes in DB\n");
                return saiRetVal;
            }
        }

    }
    return SAI_STATUS_SUCCESS;

}

static sai_status_t xpSaiAclGetComparatorIndex(xpSaiAclEntryAttributesT*
                                               pAttributes, sai_uint8_t *index)
{

    sai_status_t                saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t                entry_id    = 0;
    sai_object_id_t             acl_range_id;
    xpSaiAclRangeAttributesT    *saiAclRangeAttributes;

    for (sai_uint32_t i=0; i<pAttributes->aclRangeObjList.count; i++)
    {

        acl_range_id = *(pAttributes->aclRangeObjList.list + i);
        if (!XDK_SAI_OBJID_TYPE_CHECK(acl_range_id, SAI_OBJECT_TYPE_ACL_RANGE))
        {
            XP_SAI_LOG_DBG("Unknown object 0x%" PRIx64 "\n", acl_range_id);
            return SAI_STATUS_INVALID_OBJECT_TYPE;
        }

        entry_id = (sai_uint32_t)xpSaiObjIdValueGet(acl_range_id);

        saiRetVal = xpSaiAclMapperAclRangeAttributesGet(entry_id,
                                                        &saiAclRangeAttributes);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get entry attributes from DB\n");
            return saiRetVal;
        }
        *(index)= saiAclRangeAttributes->xpsComparatorIndex;
    }
    return SAI_STATUS_SUCCESS;
}
#endif

static sai_status_t xpSaiGetAclRangeAttribute(xpSaiAclRangeAttributesT
                                              *attributes, sai_attribute_t *attr, sai_uint32_t attr_index)
{
    if ((attributes == NULL) || (attr == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (attr->id)
    {
        case SAI_ACL_RANGE_ATTR_TYPE:
            {
                attr->value.s32 = attributes->rangeType;
                break;
            }
        case SAI_ACL_RANGE_ATTR_LIMIT:
            {
                attr->value.u32range.min = attributes->from;
                attr->value.u32range.max = attributes->to;
                break;
            }
        default:
            XP_SAI_LOG_ERR("Wrong range type received.\n");
            return SAI_STATUS_INVALID_PARAMETER;


    }
    return SAI_STATUS_SUCCESS;
}



sai_status_t xpSaiGetAclRangeAttributes(sai_object_id_t acl_range_id,
                                        sai_uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    sai_uint32_t        entryId         = 0;

    xpSaiAclRangeAttributesT *pSaiAclRangeAttributes = NULL;

    if (!XDK_SAI_OBJID_TYPE_CHECK(acl_range_id, SAI_OBJECT_TYPE_ACL_RANGE))
    {
        XP_SAI_LOG_DBG("Unknown object 0x%" PRIx64 "\n", acl_range_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    entryId = (sai_uint32_t)xpSaiObjIdValueGet(acl_range_id);

    if (attr_count == 0)
    {
        XP_SAI_LOG_ERR("No attributes requested\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               ACL_RANGE_VALIDATION_ARRAY_SIZE, acl_range_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }


    saiRetVal = xpSaiAclMapperAclRangeAttributesGet(entryId,
                                                    &pSaiAclRangeAttributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get entry attributes from DB\n");
        return saiRetVal;
    }

    for (sai_uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetAclRangeAttribute(pSaiAclRangeAttributes, &attr_list[count],
                                              count);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get entry attribute %d\n", count);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiRemoveAclRange(sai_object_id_t acl_range_id)
{
    sai_status_t        saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t        entryId     = 0;
    xpSaiAclRangeAttributesT        *pSaiAclRangeAttributes =NULL;
    xpsDevice_t         devId       = xpSaiGetDevId();

    if (!XDK_SAI_OBJID_TYPE_CHECK(acl_range_id, SAI_OBJECT_TYPE_ACL_RANGE))
    {
        XP_SAI_LOG_DBG("Unknown object 0x%" PRIx64 "\n", acl_range_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    entryId = (sai_uint32_t)xpSaiObjIdValueGet(acl_range_id);

    saiRetVal = xpSaiAclMapperAclRangeAttributesGet(entryId,
                                                    &pSaiAclRangeAttributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get entry attributes from DB\n");
        return saiRetVal;
    }

    saiRetVal = xpsAclDeleteL4PortRange(devId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to delete entry from HW %d\n", entryId);
        return saiRetVal;
    }

    saiRetVal = xpSaiAclMapperAclRangeDelete(devId, entryId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to delete acl range %u  in SAI ACL mapper\n",
                       entryId);
        return saiRetVal;
    }


    return saiRetVal;
}
//Func: xpSaiBulkGetAclEntryAttributes

sai_status_t xpSaiBulkGetAclEntryAttributes(sai_object_id_t id,
                                            sai_uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t                saiRetVal  = SAI_STATUS_SUCCESS;
    sai_uint32_t                idx        = 0;
    sai_uint32_t                count      = 0;
    sai_uint32_t                maxcount   = 0;
    sai_uint32_t                entryId    = 0;
    xpSaiAclEntryAttributesT    *attributes;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_ACL_ENTRY))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountAclEntryAttribute(&maxcount);
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

    entryId = (sai_uint32_t)xpSaiObjIdValueGet(id);

    /* Get entry attributes from mapper */
    saiRetVal = xpSaiAclMapperEntryAttributesGet(entryId, &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get counter %u attributes from mapper \n", entryId);
        return SAI_STATUS_FAILURE;
    }

    for (sai_uint32_t i = 0; i < maxcount; i++)
    {
        if (count > SAI_ACL_ENTRY_ATTR_ADMIN_STATE &&
            count < SAI_ACL_ENTRY_ATTR_FIELD_START)
        {
            count = SAI_ACL_ENTRY_ATTR_FIELD_START;
        }
        else if (count > SAI_ACL_ENTRY_ATTR_FIELD_END &&
                 count < SAI_ACL_ENTRY_ATTR_ACTION_START)
        {
            count = SAI_ACL_ENTRY_ATTR_ACTION_START;
        }

        attr_list[idx].id = SAI_ACL_ENTRY_ATTR_START + count;
        saiRetVal = xpSaiGetAclEntryAttribute(attributes, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
        count++;
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateAclCounter

sai_status_t xpSaiCreateAclCounter(sai_object_id_t *acl_counter_id,
                                   sai_object_id_t switch_id,
                                   sai_uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t                retVal      = SAI_STATUS_SUCCESS;
    sai_uint32_t                counterId   = 0;
    sai_uint32_t                devId = (sai_uint32_t)xpSaiObjIdValueGet(switch_id);
    xpSaiAclCounterAttributesT  attributes;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiCreateAclCounter\n");
    if (acl_counter_id == NULL)
    {
        XP_SAI_LOG_ERR("ERROR: Invalid parameter \n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            ACL_COUNTER_VALIDATION_ARRAY_SIZE, acl_counter_attribs,
                            SAI_COMMON_API_CREATE);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    xpSaiSetDefaultAclCounterAttributeVals(&attributes);

    retVal = xpSaiUpdateAclCounterAttributeVals(attr_count, attr_list, &attributes);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to parse counter attributes\n");
        return retVal;
    }

    /* Generate counterId */
    retVal = xpSaiAclMapperCounterIdGet(devId, attributes.tableId, &counterId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create counter for table %u \n", attributes.tableId);
        return retVal;
    }

    /* Set counter attributes in mapper */
    retVal = xpSaiAclMapperCounterIdAttributesUpdate(counterId, attributes);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update counter %u attributes \n", counterId);
        return retVal;
    }

    /* Create unified object ID */
    retVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ACL_COUNTER,
                              devId,
                              (sai_uint64_t) counterId,
                              acl_counter_id);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created\n");
        return retVal;
    }

    retVal = xpSaiGetTableEntryCountCtxDb(devId, &entryCountCtxPtr);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB, retVal = %d \n", retVal);
        return retVal;
    }
    entryCountCtxPtr->aclCounters++;

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiRemoveAclCounter

sai_status_t xpSaiRemoveAclCounter(sai_object_id_t acl_counter_id)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    sai_uint32_t counterId = 0;
    sai_uint32_t devId = xpSaiGetDevId();
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiRemoveAclCounter\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(acl_counter_id, SAI_OBJECT_TYPE_ACL_COUNTER))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(acl_counter_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    counterId = (sai_uint32_t)xpSaiObjIdValueGet(acl_counter_id);

    /* Delete counterId, HW counter is deleted inside */
    if ((xpSaiAclMapperCounterIdDelete(devId, counterId)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove counter %u \n", counterId);
        return SAI_STATUS_FAILURE;
    }

    retVal = xpSaiGetTableEntryCountCtxDb(devId, &entryCountCtxPtr);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB, retVal = %d \n", retVal);
        return retVal;
    }
    entryCountCtxPtr->aclCounters--;

    return retVal;
}


//Func: xpSaiSetAclCounterAttribute

sai_status_t xpSaiSetAclCounterAttribute(sai_object_id_t acl_counter_id,
                                         const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiAclCounterAttributesT attributes;
    sai_uint32_t counterId = 0;

    XP_SAI_LOG_DBG("Calling xpSaiSetAclCounterAttribute\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(acl_counter_id, SAI_OBJECT_TYPE_ACL_COUNTER))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(acl_counter_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    counterId = (sai_uint32_t)xpSaiObjIdValueGet(acl_counter_id);

    retVal = xpSaiAttrCheck(1, attr,
                            ACL_COUNTER_VALIDATION_ARRAY_SIZE, acl_counter_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    memset(&attributes, 0, sizeof(xpSaiAclCounterAttributesT));

    /* Get counter attributes */
    if ((xpSaiAclMapperCounterIdAttributesGet(counterId,
                                              &attributes)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to read counter %u attributes \n", counterId);
        return SAI_STATUS_FAILURE;
    }

    switch (attr->id)
    {
        case SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT:
            {
                attributes.enableByteCount = attr->value.booldata;
                break;
            }
        case SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT:
            {
                attributes.enablePacketCount = attr->value.booldata;
                break;
            }
        case SAI_ACL_COUNTER_ATTR_PACKETS:
            {
                attributes.packets = attr->value.u64;
                break;
            }
        case SAI_ACL_COUNTER_ATTR_BYTES:
            {
                attributes.bytes = attr->value.u64;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to set %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    /* Update counter attributes in mapper structures */
    if ((xpSaiAclMapperCounterIdAttributesUpdate(counterId,
                                                 attributes)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update counter %u attributes \n", counterId);
        return SAI_STATUS_FAILURE;
    }

    return retVal;
}

//Func: xpSaiGetAclCounterAttribute

static sai_status_t xpSaiGetAclCounterAttribute(xpSaiAclCounterAttributesT
                                                *attributes, sai_attribute_t *attr, sai_uint32_t attr_index)
{
    if ((attributes == NULL) || (attr == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (attr->id)
    {
        case SAI_ACL_COUNTER_ATTR_TABLE_ID:
            {
                attr->value.oid = attributes->tableOid;
                break;
            }
        case SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT:
            {
                attr->value.booldata = attributes->enablePacketCount;
                break;
            }
        /* Counter work with both together packet and byte count,
        so added this*/
        case SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT:
            {
                /* Byte count is not supported */
                attr->value.booldata = false;
                break;
            }
        case SAI_ACL_COUNTER_ATTR_PACKETS:
            {
                attr->value.u64 = attributes->packets;
                break;
            }
        /* Counter work with both together packet and byte count,
        so added this*/
        case SAI_ACL_COUNTER_ATTR_BYTES:
            {
                /* Not supported. Returned zero instead of error */
                attr->value.u64 = 0;
                break;
            }

        default:
            {
                XP_SAI_LOG_ERR("Attribute %d is unknown\n", attr->id);
                return (SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index));
            }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetAclCounterAttributes

static sai_status_t xpSaiGetAclCounterAttributes(sai_object_id_t acl_counter_id,
                                                 sai_uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiAclCounterAttributesT attributes;
    sai_uint32_t counterId = 0;

    memset(&attributes, 0, sizeof(attributes));

    XP_SAI_LOG_DBG("Calling xpSaiGetAclCounterAttributes\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(acl_counter_id, SAI_OBJECT_TYPE_ACL_COUNTER))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(acl_counter_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    counterId = (sai_uint32_t)xpSaiObjIdValueGet(acl_counter_id);

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            ACL_COUNTER_VALIDATION_ARRAY_SIZE, acl_counter_attribs,
                            SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    /* Get counter attributes from mapper */
    if ((xpSaiAclMapperCounterIdAttributesGet(counterId,
                                              &attributes)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get counter %u attributes from mapper \n", counterId);
        return SAI_STATUS_FAILURE;
    }

    xpSaiUpdateAttrListAclCounterVals(attributes, attr_count, attr_list);

    return retVal;
}

//Func: xpSaiBulkGetAclCounterAttributes

sai_status_t xpSaiBulkGetAclCounterAttributes(sai_object_id_t id,
                                              sai_uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t                saiRetVal  = SAI_STATUS_SUCCESS;
    sai_uint32_t                idx        = 0;
    sai_uint32_t                maxcount   = 0;
    sai_uint32_t                counterId  = 0;
    xpSaiAclCounterAttributesT  attributes;

    memset(&attributes, 0, sizeof(attributes));

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_ACL_COUNTER))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountAclCounterAttribute(&maxcount);
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

    counterId = (sai_uint32_t)xpSaiObjIdValueGet(id);

    /* Get counter attributes from mapper */
    saiRetVal = xpSaiAclMapperCounterIdAttributesGet(counterId, &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get counter %u attributes from mapper \n", counterId);
        return SAI_STATUS_FAILURE;
    }

    for (sai_uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_ACL_COUNTER_ATTR_START + count;
        saiRetVal = xpSaiGetAclCounterAttribute(&attributes, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiAclInit(xpsDevice_t xpsDevId)
{
    XP_STATUS           retVal              = XP_NO_ERR;
    sai_status_t        saiRetVal           = SAI_STATUS_SUCCESS;
    xpSaiAclEntry_t*    pSaiAclStaticVar    = NULL;

    sXpSaiAclDbHndl = XPSAI_ACL_DB_HNDL;
    retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "xpSaiAclDb state", XPS_GLOBAL,
                                &sXpSaiAclDbEntryComp, sXpSaiAclDbHndl);
    if (retVal != XP_NO_ERR)
    {
        sXpSaiAclDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Failed to register SAI ACL DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    if (INIT_COLD == xpsGetInitType())
    {
        // Create a new structure
        if ((retVal = xpsStateHeapMalloc(sizeof(xpSaiAclEntry_t),
                                         (void**)&pSaiAclStaticVar)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to allocate memory\n");
            return xpsStatus2SaiStatus(retVal);
        }

        // Initialize the allocated entry
        pSaiAclStaticVar->keyStaticDataType = SAI_ACL_STATIC_VARIABLES;
        pSaiAclStaticVar->saiAclTableMinimumPriority = XP_SAI_TABLE_PRI_NUM_MIN;
        pSaiAclStaticVar->saiAclEntryMinimumPriority = XP_SAI_ENTRY_PRI_NUM_MIN;
        pSaiAclStaticVar->saiAclTableMaximumPriority = XP_SAI_TABLE_PRI_NUM_MAX;
        pSaiAclStaticVar->saiAclEntryMaximumPriority = XP_SAI_ENTRY_PRI_NUM_MAX;
        // Insert the state into the database
        if ((retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, sXpSaiAclDbHndl,
                                         (void*)pSaiAclStaticVar)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to insert data into SAI ACL DB\n");
            return xpsStatus2SaiStatus(retVal);
        }

        XP_SAI_LOG_DBG("Static DB initialized: tableMaxPrio=%u, entryMaxPrio=%u.\n",
                       pSaiAclStaticVar->saiAclTableMaximumPriority,
                       pSaiAclStaticVar->saiAclEntryMaximumPriority);
    }
    /* Initialize SAI ACL mapper */
    saiRetVal = xpSaiAclMapperInit(xpsDevId);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAclMapperInit failed with return code %d\n", saiRetVal);
        if (INIT_COLD == xpsGetInitType())
        {
            return saiRetVal;
        }
    }

    sXpSaiAclPolicerInfoDbHndl = XPSAI_ACL_POLICER_DB_HNDL;
    retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "xpSaiAclPolicerInfoDb state",
                                XPS_GLOBAL, &sXpSaiAclPolicerInfoDbEntryComp, sXpSaiAclPolicerInfoDbHndl);
    if (retVal != XP_NO_ERR)
    {
        sXpSaiAclPolicerInfoDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Failed to register SAI ACL POLICER INFO DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    /* Enable policer globally */
    if ((retVal = xpsAclSetPolicingEnable(xpsDevId, 1)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to Enable policer globally |Error: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if ((retVal = xpsAclAddEgressDropPacketsRule(xpsDevId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to add egress drop rules |Error: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiAclDeInit(xpsDevice_t xpsDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiAclEntry_t* pSaiAclStaticVar  = NULL;
    xpSaiAclEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_ACL_STATIC_VARIABLES;

    // Remove the corresponding state
    if ((retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, sXpSaiAclDbHndl, &key,
                                     (void**)&pSaiAclStaticVar)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to delete data from SAI ACL DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    // Free the memory allocated for the corresponding state
    if ((retVal = xpsStateHeapFree(pSaiAclStaticVar)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to to free allocated memory\n");
        return xpsStatus2SaiStatus(retVal);
    }

    /* Disable policer globally */
    if ((retVal = xpsAclSetPolicingEnable(xpsDevId, 0)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to disable policer globally |Error: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &sXpSaiAclPolicerInfoDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to De-register SAI ACL POLICER INFO DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    /* Deinitialize SAI ACL mapper */
    xpSaiAclMapperDeinit(xpsDevId);

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Get a pointer to the SAI ACL variable entry
 *
 * \param [out] pSaiAclEntryVar
 *
 * \return XP_STATUS
 */
sai_status_t xpSaiAclStaticVariablesGet(xpSaiAclEntry_t* pSaiAclEntryVar)
{
    XP_STATUS           retVal      = XP_NO_ERR;
    xpSaiAclEntry_t*    pAclEntry   = NULL;
    xpSaiAclEntry_t     key;

    memset(&key, 0x00, sizeof(key));

    if (pSaiAclEntryVar == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_ACL_STATIC_VARIABLES;

    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, sXpSaiAclDbHndl, &key,
                                (void**)&pAclEntry);
    if ((retVal != XP_NO_ERR) || (pAclEntry == NULL))
    {
        XP_SAI_LOG_ERR("Failed to to find entry in SAI ACL DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    memcpy(pSaiAclEntryVar, pAclEntry, sizeof(xpSaiAclEntry_t));

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Set static variables in the SAI ACL DB
 *
 * \param [in] xpSaiAclEntryVar
 *
 * \return XP_STATUS
 */
sai_status_t xpSaiAclStaticVariablesSet(xpSaiAclEntry_t xpSaiAclEntryVar)
{
    XP_STATUS           retVal      = XP_NO_ERR;
    xpSaiAclEntry_t*    pAclEntry   = NULL;
    xpSaiAclEntry_t     key;

    memset(&key, 0x00, sizeof(xpSaiAclEntry_t));

    key.keyStaticDataType = SAI_ACL_STATIC_VARIABLES;

    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, sXpSaiAclDbHndl, &key,
                                (void**)&pAclEntry);
    if ((retVal != XP_NO_ERR) || (pAclEntry == NULL))
    {
        XP_SAI_LOG_ERR("Failed to to find entry in SAI ACL DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    memcpy(pAclEntry, &xpSaiAclEntryVar, sizeof(xpSaiAclEntry_t));

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAclApiInit

XP_STATUS xpSaiAclApiInit(sai_uint64_t flag,
                          const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiAclApiInit\n");

    _xpSaiAclApi = (sai_acl_api_t *) xpMalloc(sizeof(sai_acl_api_t));
    if (NULL == _xpSaiAclApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiAclApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    memset(_xpSaiAclApi, 0, sizeof(sai_acl_api_t));

    // ACL Table
    _xpSaiAclApi->create_acl_table = xpSaiCreateAclTable;
    _xpSaiAclApi->remove_acl_table = xpSaiRemoveAclTable;
    _xpSaiAclApi->set_acl_table_attribute = xpSaiSetAclTableAttribute;
    _xpSaiAclApi->get_acl_table_attribute = xpSaiGetAclTableAttributes;

    // ACL Entry
    _xpSaiAclApi->create_acl_entry = xpSaiCreateAclEntry;
    _xpSaiAclApi->remove_acl_entry = xpSaiRemoveAclEntry;
    _xpSaiAclApi->set_acl_entry_attribute = xpSaiSetAclEntryAttribute;
    _xpSaiAclApi->get_acl_entry_attribute = xpSaiGetAclEntryAttributes;

    // ACL Range
    _xpSaiAclApi->create_acl_range = xpSaiCreateAclRange;
    _xpSaiAclApi->set_acl_range_attribute = xpSaiSetAclRangeAttribute;
    _xpSaiAclApi->get_acl_range_attribute = xpSaiGetAclRangeAttributes;
    _xpSaiAclApi->remove_acl_range= xpSaiRemoveAclRange;

    // ACL Counter
    _xpSaiAclApi->create_acl_counter = xpSaiCreateAclCounter;
    _xpSaiAclApi->remove_acl_counter = xpSaiRemoveAclCounter;
    _xpSaiAclApi->set_acl_counter_attribute = xpSaiSetAclCounterAttribute;
    _xpSaiAclApi->get_acl_counter_attribute = xpSaiGetAclCounterAttributes;

    // ACL Table Group
    _xpSaiAclApi->create_acl_table_group = xpSaiCreateACLTableGroup;
    _xpSaiAclApi->remove_acl_table_group = xpSaiRemoveACLTableGroup;
    _xpSaiAclApi->set_acl_table_group_attribute = xpSaiSetACLTableGroupAttribute;
    _xpSaiAclApi->get_acl_table_group_attribute = xpSaiGetACLTableGroupAttributes;

    // ACL Table Group Member
    _xpSaiAclApi->create_acl_table_group_member = xpSaiCreateACLTableGroupMember;
    _xpSaiAclApi->remove_acl_table_group_member = xpSaiRemoveACLTableGroupMember;
    _xpSaiAclApi->set_acl_table_group_member_attribute =
        xpSaiSetACLTableGroupMemberAttribute;
    _xpSaiAclApi->get_acl_table_group_member_attribute =
        xpSaiGetACLTableGroupMemberAttributes;

    saiRetVal = xpSaiApiRegister(SAI_API_ACL, (void*)_xpSaiAclApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register ACL API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return  retVal;
}


//Func: xpSaiAclApiDeinit

XP_STATUS xpSaiAclApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiAclApiDeinit\n");

    xpFree(_xpSaiAclApi);
    _xpSaiAclApi = NULL;

    return  retVal;
}

sai_status_t xpSaiMaxCountAclTableAttribute(sai_uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = (SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST + 1) +
             XP_SAI_ACL_TABLE_MATCH_NUMBER_MAX;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiMaxCountAclCounterAttribute(sai_uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_ACL_COUNTER_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiMaxCountAclEntryAttribute(sai_uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = (SAI_ACL_ENTRY_ATTR_ADMIN_STATE + 1) + XP_SAI_ACL_MATCH_NUMBER_MAX +
             XP_SAI_ACL_ACTION_NUMBER_MAX;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiMaxCountAclRangeAttribute(sai_uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_ACL_RANGE_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountAclTableObjects(sai_uint32_t *count)
{
    return xpSaiCountAclMapperTableObjects(count);
}

sai_status_t xpSaiCountAclCounterObjects(sai_uint32_t *count)
{
    return xpSaiCountAclMapperCounterObjects(count);
}

sai_status_t xpSaiCountAclEntryObjects(sai_uint32_t *count)
{
    return xpSaiCountAclMapperEntryObjects(count);
}

sai_status_t xpSaiGetAclTableObjectList(sai_uint32_t *object_count,
                                        sai_object_key_t *object_list)
{
    return xpSaiGetAclMapperTableObjectList(object_count, object_list);
}

sai_status_t xpSaiGetAclCounterObjectList(sai_uint32_t *object_count,
                                          sai_object_key_t *object_list)
{
    return xpSaiGetAclMapperCounterObjectList(object_count, object_list);
}

sai_status_t xpSaiGetAclEntryObjectList(sai_uint32_t *object_count,
                                        sai_object_key_t *object_list)
{
    return xpSaiGetAclMapperEntryObjectList(object_count, object_list);
}

bool xpSaiValidateTableListAttributes(sai_acl_table_attr_t attr)
{
    if (attr == SAI_ACL_TABLE_ATTR_FIELD_ACL_RANGE_TYPE)
    {
        return true;
    }

    return false;
}

bool xpSaiValidateEntryListAttributes(sai_acl_entry_attr_t attr)
{
    switch (attr)
    {
        case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS:
        case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS:
        case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS:
        case SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS: //As of now this attribute only supports for list
        case SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_BLOCK_PORT_LIST:
            return true;
        default:
            XP_SAI_LOG_DBG("Entry attribute (%s) does not support list value (%d)\n",
                           xpSaiEntryAttrNameGet((sai_acl_entry_attr_t)attr),
                           attr);
            break;
    }

    return false;
}

sai_status_t xpSaiRemovePbrNextHopGroup(sai_object_id_t nhGroupOid)
{
    XP_STATUS   status      = XP_NO_ERR;
    xpsDevice_t devId       = xpSaiGetDevId();
    uint32_t    i           = 0, nhCount = 0;
    xpsScope_t  scopeId     = 0;
    uint32_t    nhGrpId     = xpSaiObjIdValueGet(nhGroupOid);
    xpsL3NextHopGroupEntry_t* pNhGrpEntry = NULL;

    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(status);
    }

    status = xpsL3GetRouteNextHopGroup(scopeId, nhGrpId, &pNhGrpEntry);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("%s:, Could not get next hop group from DB, nhGrpId %d\n",
                       __FUNCNAME__, nhGrpId);
        return xpsStatus2SaiStatus(status);
    }

    xpsL3GetCountRouteNextHopGroup(nhGrpId, &nhCount);

    if (pNhGrpEntry->enablePbr == true)
    {
        for (i = nhCount; (i < XPSAI_ACL_PBR_MAX_ECMP_SIZE) && (nhCount != 0); i++)
        {
            /* Clear the the provided NH id */
            status = xpsL3ClearRouteNextHop(devId, nhGrpId + i);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not clear next hop, nhId %d", nhGrpId + i);
                return xpsStatus2SaiStatus(status);
            }
        }

        pNhGrpEntry->enablePbr = false;
    }

    return status;
}

//Func: xpSaiIaclFldIdConvert
static sai_status_t xpSaiAclFldIdConvert(sai_acl_entry_attr_t attrId,
                                         void * value,
                                         xpsAclKeyFlds_t* fld)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    retVal = xpSaiAclEntryAttr2XpsConvert(attrId, value, fld);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        return retVal;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiAclMirrorErspanEnable(sai_object_id_t session_id,
                                               bool ingress, bool enable,
                                               uint32_t tableId)
{
    XP_STATUS             xpsRetVal   = XP_NO_ERR;
    sai_status_t          saiRetVal   = SAI_STATUS_SUCCESS;
    xpsDevice_t           xpsDevId    = 0;
    uint32_t              intfId      = 0;
    uint32_t              portGroup   = 0;
    uint32_t              offset      = 0;
    uint32_t              *pPortList  = NULL;
    xpSaiMirrorSession_t  *session    = NULL;

    // Retrieve device Id from session_id
    xpsDevId = xpSaiObjIdSwitchGet(session_id);

    saiRetVal = xpSaiMirrorDbInfoGet(session_id, &session);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to locate session %" PRIu64
                       " in data base, error: %d \n", session_id, saiRetVal);
        return saiRetVal;
    }

    xpsRetVal = xpsAclGetPortMapFromTableId(xpsDevId, tableId, &pPortList);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_DBG("Failed to get PortMap from tableid %d, error: %d \n", tableId,
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (pPortList != NULL)
    {
        for (portGroup = 0; portGroup < XPS_PORT_LIST_PORT_GROUP_MAX; portGroup++)
        {
            if (pPortList[portGroup] == 0)
            {
                continue;
            }
            for (offset = 0; offset < XPS_PORT_LIST_OFFSET; offset++)
            {
                if ((pPortList[portGroup] & (1 << offset)) == 0)
                {
                    continue;
                }

                intfId = portGroup * XPS_PORT_LIST_OFFSET + offset;
                xpsRetVal = xpsMirrorUpdateErSpan2Index(xpsDevId, intfId, ingress, enable);
                if (xpsRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("%s:, Failed in validation \n", __FUNCNAME__,
                                   "xpsMirrorUpdateErSpan2Index Failed %d \n", xpsRetVal);
                    return xpsStatus2SaiStatus(xpsRetVal);
                }

                if (ingress == false)
                {
                    xpsRetVal = xpsAclErspanRuleUpdate(xpsDevId, intfId, enable,
                                                       session->analyzerId);
                    if (xpsRetVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to enable mirror on port %d with error code %d\n",
                                       intfId, xpsRetVal);
                        return xpsStatus2SaiStatus(xpsRetVal);
                    }
                }
            }
        }
    }

    return saiRetVal;
}


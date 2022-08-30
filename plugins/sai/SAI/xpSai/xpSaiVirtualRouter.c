// xpSaiVirtualRouter.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiVirtualRouter.h"
#include "xpSaiVrf.h"
#include "xpSaiValidationArrays.h"

XP_SAI_LOG_REGISTER_API(SAI_API_VIRTUAL_ROUTER);

static sai_virtual_router_api_t* _xpSaiVirtualRouterApi;

//Func: xpSaiConvertVirtualRouterOid

sai_status_t xpSaiConvertVirtualRouterOid(sai_object_id_t vr_id,
                                          uint32_t* pVrfId)
{
    if (!XDK_SAI_OBJID_TYPE_CHECK(vr_id, SAI_OBJECT_TYPE_VIRTUAL_ROUTER))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(vr_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    *pVrfId = (uint32_t)xpSaiObjIdValueGet(vr_id);

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiSetDefaultVirtualRouterAttributeVals

void xpSaiSetDefaultVirtualRouterAttributeVals(xpSaiVirtualRouterAttributesT*
                                               attributes)
{
    attributes->adminV4State.booldata = true;
    attributes->adminV6State.booldata = true;

    xpSaiGetSwitchSrcMacAddress(attributes->srcMacAddress.mac);
}

//Func: xpSaiUpdateVirtualRouterAttributeVals

sai_status_t xpSaiUpdateVirtualRouterAttributeVals(const uint32_t attr_count,
                                                   const sai_attribute_t* attr_list, xpSaiVirtualRouterAttributesT* attributes)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_SAI_LOG_DBG("Calling xpSaiUpdateVirtualRouterAttributeVals\n");
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE:
                {
                    attributes->adminV4State = attr_list[count].value;
                    break;
                }
            case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE:
                {
                    attributes->adminV6State = attr_list[count].value;
                    break;
                }
            case SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS:
                {
                    attributes->srcMacAddress = attr_list[count].value;

                    /* Check whether MAC is valid */
                    if ((saiRetVal = xpSaiIsValidUcastMac(attributes->srcMacAddress.mac)) !=
                        SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("VRF can not be created with invalid mac address\n");
                        return saiRetVal;
                    }

                    break;
                }
            case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_PACKET_ACTION:
                {
                    attributes->violationTtl1Action = attr_list[count].value;
                    if ((attributes->violationTtl1Action.s32 < SAI_PACKET_ACTION_DROP) ||
                        (attributes->violationTtl1Action.s32 > SAI_PACKET_ACTION_TRANSIT))
                    {
                        XP_SAI_LOG_ERR("invalid packet_action is received\n");
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }
                    break;
                }
            case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_IP_OPTIONS_PACKET_ACTION:
                {
                    attributes->violationIpOptions = attr_list[count].value;
                    if ((attributes->violationIpOptions.s32 < SAI_PACKET_ACTION_DROP) ||
                        (attributes->violationIpOptions.s32 > SAI_PACKET_ACTION_TRANSIT))
                    {
                        XP_SAI_LOG_ERR("invalid packet_action is received\n");
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }
                    break;
                }
            case SAI_VIRTUAL_ROUTER_ATTR_UNKNOWN_L3_MULTICAST_PACKET_ACTION:
                {
                    attributes->unkL3MCPktAction = attr_list[count].value;
                    if ((attributes->unkL3MCPktAction.s32 < SAI_PACKET_ACTION_DROP) ||
                        (attributes->unkL3MCPktAction.s32 > SAI_PACKET_ACTION_TRANSIT))
                    {
                        XP_SAI_LOG_ERR("invalid packet_action is received\n");
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                }
        }

    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetVirtualRouterAttrAdminV4State

sai_status_t xpSaiSetVirtualRouterAttrAdminV4State(uint32_t vrfId,
                                                   sai_attribute_value_t value)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Debug\n");

    retVal = (XP_STATUS) xpSaiVrfAdminV4StateSet(xpsDevId, vrfId, value.booldata);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfAdminV4StateSet() failed with error code: %d\n",
                       retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}


//Func: xpSaiGetVirtualRouterAttrAdminV4State

sai_status_t xpSaiGetVirtualRouterAttrAdminV4State(uint32_t vrfId,
                                                   sai_attribute_value_t* pValue)
{
    XP_STATUS   retVal      = XP_NO_ERR;
    uint32_t    adminStatus = 0;
    xpsDevice_t xpsDevId    = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Debug\n");

    retVal = (XP_STATUS) xpSaiVrfAdminV4StateGet(xpsDevId, vrfId, &adminStatus);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfAdminV4StateGet() failed with error code: %d\n",
                       retVal);
    }

    pValue->booldata = adminStatus;

    return xpsStatus2SaiStatus(retVal);
}


//Func: xpSaiSetVirtualRouterAttrAdminV6State

sai_status_t xpSaiSetVirtualRouterAttrAdminV6State(uint32_t vrfId,
                                                   sai_attribute_value_t value)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Debug\n");

    retVal = (XP_STATUS) xpSaiVrfAdminV6StateSet(xpsDevId, vrfId, value.booldata);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfAdminV6StateSet() failed with error code: %d\n",
                       retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}


//Func: xpSaiGetVirtualRouterAttrAdminV6State

sai_status_t xpSaiGetVirtualRouterAttrAdminV6State(uint32_t vrfId,
                                                   sai_attribute_value_t* pValue)
{
    XP_STATUS   retVal      = XP_NO_ERR;
    uint32_t    adminStatus = 0;
    xpsDevice_t xpsDevId    = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Debug\n");

    retVal = (XP_STATUS) xpSaiVrfAdminV6StateGet(xpsDevId, vrfId, &adminStatus);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfAdminV6StateGet() failed with error code: %d\n",
                       retVal);
    }

    pValue->booldata = adminStatus;

    return xpsStatus2SaiStatus(retVal);
}


//Func: xpSaiSetVirtualRouterAttrSrcMacAddress

sai_status_t xpSaiSetVirtualRouterAttrSrcMacAddress(uint32_t vrfId,
                                                    sai_attribute_value_t value)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Debug\n");

    retVal = (XP_STATUS) xpSaiVrfMacAddressSet(xpsDevId, vrfId, value.mac);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfMacAddressSet() failed with error code: %d\n", retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiSetVirtualRouterAttrUnkL3MCPktAction(uint32_t vrfId,
                                                       sai_attribute_value_t value)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    retVal = xpSaiVrfUnkL3MCPktActionSet(xpsDevId, vrfId,
                                         (sai_packet_action_t)value.s32);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfUnkL3MCPktActionSet() failed with error code: %d\n",
                       retVal);
    }

    return retVal;
}

//Func: xpSaiGetVirtualRouterAttrSrcMacAddress

sai_status_t xpSaiGetVirtualRouterAttrSrcMacAddress(uint32_t vrfId,
                                                    sai_attribute_value_t* value)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Debug\n");

    retVal = (XP_STATUS) xpSaiVrfMacAddressGet(xpsDevId, vrfId, value->mac);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfMacAddressGet() failed with error code: %d\n", retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetVirtualRouterAttrUnkL3McPktAction(uint32_t vrfId,
                                                       sai_attribute_value_t* value)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    sai_packet_action_t action;

    XP_SAI_LOG_DBG("Debug\n");

    retVal = xpSaiVrfUnkL3McPktActionGet(xpsDevId, vrfId, &action);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfMacAddressGet() failed with error code: %d\n", retVal);
    }
    value->s32 = action;

    return retVal;
}

//Func: xpSaiSetVirtualRouterAttrViolationTtl1Action

sai_status_t xpSaiSetVirtualRouterAttrViolationTtl1Action(uint32_t vrfId,
                                                          sai_attribute_value_t value)
{
    if (value.s32 == SAI_PACKET_ACTION_TRAP)
    {
        return SAI_STATUS_SUCCESS;
    }

    XP_SAI_LOG_ERR("The action is not supported. Only SAI_PACKET_ACTION_TRAP is supported\n");
    return SAI_STATUS_ATTR_NOT_IMPLEMENTED_0;
}


//Func: xpSaiGetVirtualRouterAttrViolationTtl1Action

sai_status_t xpSaiGetVirtualRouterAttrViolationTtl1Action(uint32_t vrfId,
                                                          sai_attribute_value_t* value)
{
    value->s32 = SAI_PACKET_ACTION_DROP;
    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiSetVirtualRouterAttrViolationIpOptions

sai_status_t xpSaiSetVirtualRouterAttrViolationIpOptions(uint32_t vrfId,
                                                         sai_attribute_value_t value)
{
    if (value.s32 == SAI_PACKET_ACTION_TRAP)
    {
        return SAI_STATUS_SUCCESS;
    }

    XP_SAI_LOG_ERR("The action is not supported. Only SAI_PACKET_ACTION_TRAP is supported\n");
    return SAI_STATUS_ATTR_NOT_IMPLEMENTED_0;
}

//Func: xpSaiGetVirtualRouterAttrViolationIpOptions

sai_status_t xpSaiGetVirtualRouterAttrViolationIpOptions(uint32_t vrfId,
                                                         sai_attribute_value_t* value)
{
    value->s32 = SAI_PACKET_ACTION_DROP;
    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiCreateVirtualRouter

sai_status_t xpSaiCreateVirtualRouter(sai_object_id_t *vr_id,
                                      sai_object_id_t switch_id,
                                      uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_STATUS    retVal    = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t     vrfId     = 0;

    xpSaiVirtualRouterAttributesT attributes;

    if (vr_id == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsDevice_t xpsDevId = xpSaiGetDevId();

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               VIRTUAL_ROUTER_VALIDATION_ARRAY_SIZE, virtual_router_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    xpSaiSetDefaultVirtualRouterAttributeVals(&attributes);
    saiRetVal = xpSaiUpdateVirtualRouterAttributeVals(attr_count, attr_list,
                                                      &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiUpdateVirtualRouterAttributeVals, retVal : %d \n",
                       saiRetVal);
        return saiRetVal;
    }

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    retVal = xpSaiVrfCreate(xpsDevId, &vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfCreate() failed with error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, xpsDevId,
                         (sai_uint64_t) vrfId, vr_id) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created.\n");
        return SAI_STATUS_FAILURE;
    }

    saiRetVal = xpSaiSetVirtualRouterAttrAdminV4State(vrfId,
                                                      attributes.adminV4State);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE)\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiSetVirtualRouterAttrAdminV6State(vrfId,
                                                      attributes.adminV6State);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE)\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiSetVirtualRouterAttrSrcMacAddress(vrfId,
                                                       attributes.srcMacAddress);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS)\n");
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiRemoveVirtualRouter

sai_status_t xpSaiRemoveVirtualRouter(sai_object_id_t vr_id)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        vrfId       = 0;
    xpsDevice_t     xpsDevId    = xpSaiGetDevId();
    sai_object_id_t vrfOid      = SAI_NULL_OBJECT_ID;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiSwitchDefaultVrfGet(&vrfOid);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get default VRF. RC %d\n", saiRetVal);
        return SAI_STATUS_FAILURE;
    }

    if (vrfOid == vr_id)
    {
        XP_SAI_LOG_ERR("Default VRF can not be removed.\n");
        return SAI_STATUS_OBJECT_IN_USE;
    }

    saiRetVal = xpSaiConvertVirtualRouterOid(vr_id, &vrfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertVirtualRouterOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    retVal = (XP_STATUS) xpSaiVrfRemove(xpsDevId, vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfRemove() failed with error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiSetVirtualRouterAttribute

sai_status_t xpSaiSetVirtualRouterAttribute(sai_object_id_t vr_id,
                                            const sai_attribute_t *attr)
{
    sai_status_t saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t     vrfId      = 0;

    saiRetVal = xpSaiAttrCheck(1, attr,
                               VIRTUAL_ROUTER_VALIDATION_ARRAY_SIZE, virtual_router_attribs,
                               SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertVirtualRouterOid(vr_id, &vrfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertVirtualRouterOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    switch (attr->id)
    {
        case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE:
            {
                saiRetVal = xpSaiSetVirtualRouterAttrAdminV4State(vrfId, attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE:
            {
                saiRetVal = xpSaiSetVirtualRouterAttrAdminV6State(vrfId, attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS:
            {
                /* Check whether MAC is valid */
                if ((saiRetVal = xpSaiIsValidUcastMac((uint8_t *)attr->value.mac)) !=
                    SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS)\n");
                    return saiRetVal;
                }
                saiRetVal = xpSaiSetVirtualRouterAttrSrcMacAddress(vrfId, attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_PACKET_ACTION:
            {
                if ((attr->value.s32 < SAI_PACKET_ACTION_DROP) ||
                    (attr->value.s32 > SAI_PACKET_ACTION_TRANSIT))
                {
                    XP_SAI_LOG_ERR("Invalid Packet_action is received\n");
                    return SAI_STATUS_INVALID_ATTR_VALUE_0;
                }
                return xpSaiSetVirtualRouterAttrViolationTtl1Action(vrfId, attr->value);
            }
        case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_IP_OPTIONS_PACKET_ACTION:
            {
                if ((attr->value.s32 < SAI_PACKET_ACTION_DROP) ||
                    (attr->value.s32 > SAI_PACKET_ACTION_TRANSIT))
                {
                    XP_SAI_LOG_ERR("Invalid Packet_action is received\n");
                    return SAI_STATUS_INVALID_ATTR_VALUE_0;
                }
                return xpSaiSetVirtualRouterAttrViolationIpOptions(vrfId, attr->value);
            }
        case SAI_VIRTUAL_ROUTER_ATTR_UNKNOWN_L3_MULTICAST_PACKET_ACTION:
            {
                if ((attr->value.s32 < SAI_PACKET_ACTION_DROP) ||
                    (attr->value.s32 > SAI_PACKET_ACTION_TRANSIT))
                {
                    XP_SAI_LOG_ERR("Invalid Packet_action is received\n");
                    return SAI_STATUS_INVALID_ATTR_VALUE_0;
                }

                saiRetVal = xpSaiSetVirtualRouterAttrUnkL3MCPktAction(vrfId, attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (UNKNOWN_L3 MC PKT ACT)\n");
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
    return  saiRetVal;
}


//Func: xpSaiGetVirtualRouterAttribute

sai_status_t xpSaiGetVirtualRouterAttribute(uint32_t vrfId,
                                            sai_attribute_t *attr, uint32_t attr_index)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    switch (attr->id)
    {
        case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE:
            {
                retVal = xpSaiGetVirtualRouterAttrAdminV4State(vrfId, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE:
            {
                retVal = xpSaiGetVirtualRouterAttrAdminV6State(vrfId, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS:
            {
                retVal = xpSaiGetVirtualRouterAttrSrcMacAddress(vrfId, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS)\n");
                    return retVal;
                }
                break;
            }
        case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_PACKET_ACTION:
        case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_IP_OPTIONS_PACKET_ACTION:
            {
                return SAI_STATUS_ATTR_NOT_SUPPORTED_0;
            }
        case SAI_VIRTUAL_ROUTER_ATTR_UNKNOWN_L3_MULTICAST_PACKET_ACTION:
            {
                return xpSaiGetVirtualRouterAttrUnkL3McPktAction(vrfId, &attr->value);
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }
    return retVal;
}


//Func: xpSaiGetVirtualRouterAttributes

static sai_status_t xpSaiGetVirtualRouterAttributes(sai_object_id_t vr_id,
                                                    uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t     vrfId     = 0;

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               VIRTUAL_ROUTER_VALIDATION_ARRAY_SIZE, virtual_router_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertVirtualRouterOid(vr_id, &vrfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertVirtualRouterOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetVirtualRouterAttribute(vrfId, &attr_list[count], count);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetVirtualRouterAttribute failed\n");
            return saiRetVal;
        }
    }

    return saiRetVal;
}

//Func: xpSaiBulkGetVirtualRouterAttributes

sai_status_t xpSaiBulkGetVirtualRouterAttributes(sai_object_id_t id,
                                                 uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;
    uint32_t         vrfId      = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_VIRTUAL_ROUTER))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountVirtualRouterAttribute(&maxcount);
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

    vrfId = (uint32_t)xpSaiObjIdValueGet(id);

    for (uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_VIRTUAL_ROUTER_ATTR_START + count;
        saiRetVal = xpSaiGetVirtualRouterAttribute(vrfId, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCreateDefaultVirtualRouter(xpsDevice_t xpSaiDevId)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    sai_object_id_t     switchOid;
    sai_object_id_t     vrfOid;

    if (xpSaiVrfExists(XP_SAI_VRF_DEFAULT_ID))
    {
        XP_SAI_LOG_ERR("Default VRF already exists\n");
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_SWITCH, xpSaiDevId, 0, &switchOid);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI switch object could not be created. RC %d\n", saiRetVal);
        return SAI_STATUS_FAILURE;
    }

    saiRetVal = xpSaiCreateVirtualRouter(&vrfOid, switchOid, 0, NULL);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create default VRF. RC %d\n", saiRetVal);
        return SAI_STATUS_FAILURE;
    }

    saiRetVal = xpSaiSwitchDefaultVrfSet(vrfOid);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set default VRF. RC %d\n", saiRetVal);
        return SAI_STATUS_FAILURE;
    }

    return saiRetVal;
}

//Func: xpSaiVirtualRouterApiInit

XP_STATUS xpSaiVirtualRouterApiInit(uint64_t flag,
                                    const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiVirtualRouterApiInit\n");

    _xpSaiVirtualRouterApi = (sai_virtual_router_api_t *) xpMalloc(sizeof(
                                                                       sai_virtual_router_api_t));
    if (NULL == _xpSaiVirtualRouterApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiVirtualRouterApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiVirtualRouterApi->create_virtual_router = xpSaiCreateVirtualRouter;
    _xpSaiVirtualRouterApi->remove_virtual_router = xpSaiRemoveVirtualRouter;
    _xpSaiVirtualRouterApi->set_virtual_router_attribute =
        xpSaiSetVirtualRouterAttribute;
    _xpSaiVirtualRouterApi->get_virtual_router_attribute =
        xpSaiGetVirtualRouterAttributes;

    saiRetVal = xpSaiApiRegister(SAI_API_VIRTUAL_ROUTER,
                                 (void*)_xpSaiVirtualRouterApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register virtual router API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}


//Func: xpSaiVirtualRouterApiDeinit

XP_STATUS xpSaiVirtualRouterApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiVirtualRouterApiDeinit\n");

    xpFree(_xpSaiVirtualRouterApi);
    _xpSaiVirtualRouterApi = NULL;

    return  retVal;
}

sai_status_t xpSaiMaxCountVirtualRouterAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_VIRTUAL_ROUTER_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountVirtualRouterObjects(uint32_t *count)
{
    return xpSaiCountVrfObjects(count);
}

sai_status_t xpSaiGetVirtualRouterObjectList(uint32_t *object_count,
                                             sai_object_key_t *object_list)
{
    return xpSaiGetVrfObjectList(object_count, object_list);
}



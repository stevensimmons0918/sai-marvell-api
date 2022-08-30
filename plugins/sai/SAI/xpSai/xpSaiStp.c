// xpSaiStp.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiStp.h"
#include "xpSaiValidationArrays.h"
#include "xpSaiStub.h"

XP_SAI_LOG_REGISTER_API(SAI_API_STP);

static sai_stp_api_t* _xpSaiStpApi;

sai_status_t xpSaiConvertStpPortStateXpsToSai(xpVlanStgState_e xpsStpState,
                                              sai_stp_port_state_t* saiStpState)
{
    switch (xpsStpState)
    {
        case SPAN_STATE_BLOCK:
            {
                *saiStpState = SAI_STP_PORT_STATE_BLOCKING;
                break;
            }
        case SPAN_STATE_LEARN:
            {
                *saiStpState =  SAI_STP_PORT_STATE_LEARNING;
                break;
            }
        case SPAN_STATE_DISABLE:
        case SPAN_STATE_FORWARD:
            {
                *saiStpState =  SAI_STP_PORT_STATE_FORWARDING;
                break;
            }
        default:
            {
                printf("Unknown stp state %d\n", xpsStpState);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiConvertStpPortStateSaiToXps(sai_stp_port_state_t saiStpState,
                                              xpVlanStgState_e* xpsStpState)
{
    switch (saiStpState)
    {
        case SAI_STP_PORT_STATE_LEARNING:
            {
                *xpsStpState = SPAN_STATE_LEARN;
                break;
            }
        case SAI_STP_PORT_STATE_FORWARDING:
            {
                *xpsStpState = SPAN_STATE_FORWARD;
                break;
            }
        case SAI_STP_PORT_STATE_BLOCKING:
            {
                *xpsStpState = SPAN_STATE_BLOCK;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown saiStpState %d\n", saiStpState);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateStp
// Supported Total 256 STPs and 4096 VLAN are there.
sai_status_t xpSaiCreateStp(sai_object_id_t* stp_id, sai_object_id_t switch_id,
                            uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_STATUS       xpsRetVal   = XP_NO_ERR;
    xpsStp_t        xpsStpId    = 0;
    xpsDevice_t     devId       = xpSaiGetDevId();
    sai_status_t    saiRetVal   = SAI_STATUS_FAILURE;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (stp_id == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               STP_VALIDATION_ARRAY_SIZE, stp_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    xpsRetVal = xpsStpCreateWithState(XP_SCOPE_DEFAULT, SPAN_STATE_BLOCK,
                                      &xpsStpId);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStpCreate failed stpId %d | retVal : %d \n", xpsStpId,
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_STP, devId, (sai_uint64_t) xpsStpId,
                         stp_id) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created.\n");
        return SAI_STATUS_FAILURE;
    }

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiRemoveStp

sai_status_t xpSaiRemoveStp(sai_object_id_t stp_id)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsStp_t xpsStpId = 0;
    xpsDevice_t         xpsDevId        = xpSaiGetDevId();
    xpsInterfaceId_t    *xpsIntfList    = NULL;
    uint16_t            numOfIntfs      = 0;
    xpsScope_t      scopeId     = XP_SCOPE_DEFAULT;
    xpsStgStaticDbEntry * staticVarDb = NULL;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(stp_id, SAI_OBJECT_TYPE_STP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(stp_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }
    xpsStpId = (xpsStp_t)xpSaiObjIdValueGet(stp_id);

    xpsRetVal = xpsStpGetInterfaceList(xpsDevId, xpsStpId, &xpsIntfList,
                                       &numOfIntfs);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStpGetInterfaceList failed stpId %d | retVal : %d \n",
                       xpsStpId, xpsRetVal);
        xpsStateHeapFree((void *)xpsIntfList);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (numOfIntfs != 0)
    {
        XP_SAI_LOG_ERR("failed to remove RSTP instance remove member ports first \n");
        xpsStateHeapFree((void *)xpsIntfList);
        return xpsStatus2SaiStatus(XP_ERR_FAILED);
    }

    xpsRetVal = xpsStgGetStaticVariablesDb(scopeId, &staticVarDb);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStgGetStaticVariablesDb failed");
        xpsStateHeapFree((void *)xpsIntfList);
        return xpsStatus2SaiStatus(XP_ERR_FAILED);
    }

    if (staticVarDb->defaultStg == xpsStpId)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Default STP instance Delete not allowed !!!");
        xpsStateHeapFree((void *)xpsIntfList);
        return SAI_STATUS_NOT_SUPPORTED;
    }

    xpsStateHeapFree((void *)xpsIntfList);

    xpsRetVal = xpsStpDestroy(xpsStpId);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStpDestroy failed stpId %d | retVal : %d \n", xpsStpId,
                       xpsRetVal);
        /*SAI_STATUS_INVALID_OBJECT_ID is used when object id is already deleted*/
        if (xpsRetVal == XP_ERR_KEY_NOT_FOUND)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    return xpsStatus2SaiStatus(xpsRetVal);
}


//Func: xpSaiGetStpAttrPortList
sai_status_t xpSaiGetStpAttrPortList(sai_object_id_t stp_id,
                                     sai_attribute_value_t* value)
{
    XP_STATUS           xpsRetVal       = XP_NO_ERR;
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t    *xpsIntfList    = NULL;
    xpsStp_t            xpsStpId        = 0;
    uint16_t            numOfIntfs      = 0;
    xpsDevice_t         xpsDevId        = xpSaiGetDevId();
    sai_object_id_t     stpPortId       = SAI_NULL_OBJECT_ID;
    sai_uint64_t        localId         = 0;;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(stp_id, SAI_OBJECT_TYPE_STP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(stp_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }
    xpsStpId = (xpsStp_t)xpSaiObjIdValueGet(stp_id);

    xpsRetVal = xpsStpGetInterfaceList(xpsDevId, xpsStpId, &xpsIntfList,
                                       &numOfIntfs);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStpGetInterfaceList failed stpId %d | retVal : %d \n",
                       xpsStpId, xpsRetVal);
        xpsStateHeapFree((void *)xpsIntfList);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (numOfIntfs > value->objlist.count)
    {
        value->objlist.count = numOfIntfs;
        xpsStateHeapFree((void *)xpsIntfList);
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if ((numOfIntfs != 0) && (value->objlist.list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameters received");
        xpsStateHeapFree((void *)xpsIntfList);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    value->objlist.count = numOfIntfs;
    for (sai_uint16_t indx = 0; indx < value->objlist.count; indx++)
    {
        localId = XPS_STP_GET_STGINTFSTATE_DBKEY(xpsIntfList[indx], xpsStpId);
        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_STP_PORT, xpsDevId, localId,
                                     &stpPortId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI object could not be created, error %d\n", saiRetVal);
            xpsStateHeapFree((void *)xpsIntfList);
            return saiRetVal;
        }
        value->objlist.list[indx] = stpPortId;
    }
    xpsStateHeapFree((void *)xpsIntfList);

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiGetStpAttrVlanList

sai_status_t xpSaiGetStpAttrVlanList(sai_object_id_t stp_id,
                                     sai_attribute_value_t* value)
{
    XP_STATUS   xpsRetVal       = XP_NO_ERR;
    xpsVlan_t   *xpsVlanList    = NULL;
    xpsStp_t    xpsStpId        = 0;
    uint16_t    numOfVlans      = 0;
    xpsDevice_t xpsDevId        = xpSaiGetDevId();

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(stp_id, SAI_OBJECT_TYPE_STP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(stp_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }
    xpsStpId = (xpsStp_t)xpSaiObjIdValueGet(stp_id);

    xpsRetVal = xpsStgGetVlanList(xpsDevId, xpsStpId, &xpsVlanList, &numOfVlans);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStgGetVlanList failed stpId %d | retVal : %d \n", xpsStpId,
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    if (numOfVlans > value->vlanlist.count)
    {
        value->vlanlist.count = numOfVlans;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if ((numOfVlans != 0) && (value->vlanlist.list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameters received");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    value->vlanlist.count = numOfVlans;
    memcpy(value->vlanlist.list, xpsVlanList, sizeof(xpsVlan_t)*numOfVlans);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetStpAttribute

static sai_status_t xpSaiGetStpAttribute(sai_object_id_t stp_id,
                                         sai_attribute_t* attr, uint32_t attr_index)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    switch (attr->id)
    {
        case SAI_STP_ATTR_VLAN_LIST:
            {
                saiRetVal = xpSaiGetStpAttrVlanList(stp_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_STP_ATTR_VLAN_LIST)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_STP_ATTR_PORT_LIST:
            {
                saiRetVal = xpSaiGetStpAttrPortList(stp_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_STP_ATTR_PORT_LIST)\n");
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

    return saiRetVal;
}

//Func: xpSaiGetStpAttributes

static sai_status_t xpSaiGetStpAttributes(sai_object_id_t stp_id,
                                          uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               STP_VALIDATION_ARRAY_SIZE, stp_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetStpAttribute(stp_id, &attr_list[count], count);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetStpAttributes failed\n");
            return saiRetVal;
        }
    }

    return saiRetVal;
}

//Func: xpSaiBulkGetStpAttributes

sai_status_t xpSaiBulkGetStpAttributes(sai_object_id_t id, uint32_t *attr_count,
                                       sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_STP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountStpAttribute(&maxcount);
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
        attr_list[idx].id = SAI_STP_ATTR_START + count;
        saiRetVal = xpSaiGetStpAttribute(id, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetStpAttribute

sai_status_t xpSaiSetStpAttribute(sai_object_id_t stp_id,
                                  const sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    saiRetVal = xpSaiAttrCheck(1, attr,
                               STP_VALIDATION_ARRAY_SIZE, stp_attribs,
                               SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    return saiRetVal;
}

//Func: xpSaiUpdateStpPortAttributeVals

void xpSaiUpdateStpPortAttributeVals(const uint32_t attr_count,
                                     const sai_attribute_t* attr_list, xpSaiStpPortAttributesT* attributes)
{

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_STP_PORT_ATTR_STP:
                {
                    attributes->stp = attr_list[count].value;
                    break;
                }
            case SAI_STP_PORT_ATTR_BRIDGE_PORT:
                {
                    attributes->port = attr_list[count].value;
                    break;
                }
            case SAI_STP_PORT_ATTR_STATE:
                {
                    attributes->state = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }

    }

}


//Func: xpSaiSetStpPortAttrState

sai_status_t xpSaiSetStpPortAttrState(sai_object_id_t stp_port_id,
                                      sai_attribute_value_t value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    uint32_t stpIntfDbKey = xpSaiObjIdValueGet(stp_port_id);
    //uint32_t iVif = 0;
    xpsStp_t stpId = XPS_STP_GET_STPID_FROM_DBKEY(stpIntfDbKey);
    xpsInterfaceId_t intfId = XPS_STP_GET_INTFID_FROM_DBKEY(stpIntfDbKey);
    xpsStp_t portStpId;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(stp_port_id, SAI_OBJECT_TYPE_STP_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(stp_port_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    xpsRetVal = xpsGetPortStpId(xpsDevId, intfId, &portStpId);
    if (xpsRetVal != XP_NO_ERR)
    {
        return xpsRetVal;
    }

    if (portStpId != stpId)
    {
        XP_SAI_LOG_ERR("Port is not part stp :%d \n", stpId);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /*Enum types are not present in sai_attribute_value_t. Should be dereferenced as s32*/
    sai_stp_port_state_t saiStpState = (sai_stp_port_state_t) value.s32;
    xpVlanStgState_e xpsStpState;

    saiRetVal = xpSaiConvertStpPortStateSaiToXps(saiStpState, &xpsStpState);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertStpPortStateSaiToXps error\n");
        return saiRetVal;
    }

#if 0
    xpsRetVal = xpsPortGetField(xpsDevId, intfId, XPS_PORT_INGRESS_VIF, &iVif);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStpSetState failed xpsPortGetField port %d | retVal : %d \n",
                       intfId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /*this way it makes sure that we apply stp state on either LAG vif in case of Lag Member and on port in normal case*/
    intfId = iVif;
#endif

    xpsRetVal = xpsStpSetState(xpsDevId, stpId, intfId, xpsStpState);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStpSetState failed stpId %d | retVal : %d \n", stpId,
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return  SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetStpPortAttrStp

sai_status_t xpSaiGetStpPortAttrStp(sai_object_id_t stp_port_id,
                                    sai_attribute_value_t* value)
{
    xpsDevice_t     devId       = xpSaiGetDevId();

    if (!XDK_SAI_OBJID_TYPE_CHECK(stp_port_id, SAI_OBJECT_TYPE_STP_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(stp_port_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    uint32_t stpIntfDbKey = xpSaiObjIdValueGet(stp_port_id);
    xpsStp_t stpId = XPS_STP_GET_STPID_FROM_DBKEY(stpIntfDbKey);

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    return xpSaiObjIdCreate(SAI_OBJECT_TYPE_STP, devId, stpId, &value->oid);

}


//Func: xpSaiGetStpPortAttrBridgePort

sai_status_t xpSaiGetStpPortAttrBridgePort(sai_object_id_t stpPortId,
                                           sai_attribute_value_t* value)
{
    sai_status_t       saiRetVal    = SAI_STATUS_SUCCESS;
    XP_STATUS          xpsRetVal    = XP_NO_ERR;
    sai_object_id_t    brPortId     = SAI_NULL_OBJECT_ID;
    xpsDevice_t        xpsDevId     = 0;
    uint32_t           stpIntfDbKey = 0;
    xpsInterfaceId_t   intfId       = XPS_INTF_INVALID_ID;
    xpsInterfaceType_e intfType;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    /* Retrieve xpsDevId from SAI object Id */
    xpsDevId = xpSaiObjIdSwitchGet(stpPortId);

    if (!XDK_SAI_OBJID_TYPE_CHECK(stpPortId, SAI_OBJECT_TYPE_STP_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(stpPortId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    stpIntfDbKey = xpSaiObjIdValueGet(stpPortId);
    intfId = XPS_STP_GET_INTFID_FROM_DBKEY(stpIntfDbKey);

    xpsRetVal = xpsInterfaceGetType(intfId, &intfType);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Interface Get type failed, vif %d, retVal %d\n", intfId,
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    switch (intfType)
    {
        case XPS_PORT:
        case XPS_LAG:
            {
                saiRetVal = xpSaiBridgePortObjIdCreate(xpsDevId, intfId,
                                                       SAI_BRIDGE_PORT_TYPE_PORT, &brPortId);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiBridgePortObjIdCreate failed retVal : %d \n", saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case XPS_BRIDGE_PORT:
            {
                XP_SAI_LOG_ERR("1D Bridge is not supported for STP.\n");
                return SAI_STATUS_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("%s Invalid interface type : %d \n", __FUNCNAME__, intfType);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    value->oid = brPortId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetStpPortAttrState

sai_status_t xpSaiGetStpPortAttrState(sai_object_id_t stp_port_id,
                                      sai_attribute_value_t* value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    //uint32_t iVif = 0;
    xpsStp_t portStpId;

    if (!XDK_SAI_OBJID_TYPE_CHECK(stp_port_id, SAI_OBJECT_TYPE_STP_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(stp_port_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    uint32_t stpIntfDbKey = xpSaiObjIdValueGet(stp_port_id);
    xpsStp_t stpId = XPS_STP_GET_STPID_FROM_DBKEY(stpIntfDbKey);
    xpsInterfaceId_t intfId = XPS_STP_GET_INTFID_FROM_DBKEY(stpIntfDbKey);

    sai_stp_port_state_t saiStpState;
    xpVlanStgState_e xpsStpState;

    xpsRetVal = xpsGetPortStpId(xpsDevId, intfId, &portStpId);
    if (xpsRetVal != XP_NO_ERR)
    {
        return xpsRetVal;
    }

    if (portStpId != stpId)
    {
        XP_SAI_LOG_ERR("Port is not part stp :%d \n", stpId);
        return SAI_STATUS_INVALID_PARAMETER;
    }

#if 0
    /*get IVIF from intfId. incase of Lag member port we need to retrieve state from lag VIF*/
    xpsRetVal = xpsPortGetField(xpsDevId, intfId, XPS_PORT_INGRESS_VIF, &iVif);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStpSetState failed xpsPortGetField port %d | retVal : %d \n",
                       intfId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
#endif
    xpsRetVal = xpsStpGetState(xpsDevId, stpId, intfId, &xpsStpState);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStpGetState failed stpId %d | retVal : %d \n", stpId,
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    saiRetVal = xpSaiConvertStpPortStateXpsToSai(xpsStpState, &saiStpState);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertStpPortStateXpsToSai error\n");
        return saiRetVal;
    }

    value->s32 = saiStpState;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    return  SAI_STATUS_SUCCESS;

}


//Func: xpSaiConvertStpPortToXps
static sai_status_t xpSaiConvertStpPortToXps(xpSaiStpPortAttributesT
                                             *attributes, xpSaiStpPortAttrEntry_t *stpPortData)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    stpPortData->portId = (xpsPort_t) xpSaiObjIdValueGet(attributes->port.oid);
    stpPortData->intfId= (xpsInterfaceId_t) xpSaiObjIdValueGet(
                             attributes->port.oid);
    stpPortData->stpId = (xpsStp_t) xpSaiObjIdValueGet(attributes->stp.oid);

    //Enum types are not present in sai_attribute_value_t. Should be dereferenced as s32
    sai_stp_port_state_t saiStpState = (sai_stp_port_state_t) attributes->state.s32;

    saiRetVal = xpSaiConvertStpPortStateSaiToXps(saiStpState,
                                                 &(stpPortData->state));
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(attributes->port.oid));
        return saiRetVal;
    }

    return saiRetVal;
}


//Func: xpSaiCreateStpPort
sai_status_t xpSaiCreateStpPort(sai_object_id_t *stp_port_id,
                                sai_object_id_t switch_id, uint32_t attr_count,
                                const sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpSaiStpPortAttributesT attributes;
    xpSaiStpPortAttrEntry_t stpPortData;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsStgIntfStateDbEntry *stgIntfState = NULL;
    xpsScope_t      scopeId     = XP_SCOPE_DEFAULT;

    memset(&attributes, 0, sizeof(xpSaiStpPortAttributesT));

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               STP_PORT_VALIDATION_ARRAY_SIZE, stp_port_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    xpSaiUpdateStpPortAttributeVals(attr_count, attr_list, &attributes);

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if ((saiRetVal = xpSaiConvertStpPortToXps(&attributes,
                                              &stpPortData)) != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    xpsRetVal = xpsStgGetStgIntfEntryDb(scopeId, stpPortData.stpId,
                                        stpPortData.intfId, &stgIntfState);
    if (xpsRetVal == XP_NO_ERR && (stgIntfState != NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get interface entry failed,"
              "Already Exits stpId(%d) interface(%d)", stpPortData.stpId, stpPortData.intfId);
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    xpsRetVal = xpsStpIsExistOnDevice(xpsDevId, stpPortData.stpId);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI STP instance not found, error %d stpId :%d \n", saiRetVal,
                       stpPortData.stpId);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    xpsRetVal = xpsValidatePortStpId(xpsDevId, stpPortData.intfId,
                                     stpPortData.stpId);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsValidatePortStpId, error %d stpId :%d \n", saiRetVal,
                       stpPortData.stpId);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }


    /**1. Create the stp_port_id object
    stp_port_id is an object of new type SAI_OBJECT_TYPE_STP_PORT
    Here its derived from XPS_STP_GET_STGINTFSTATE_DBKEY(intfId, stgId);*/

    sai_uint64_t local_id;
    local_id = XPS_STP_GET_STGINTFSTATE_DBKEY(stpPortData.intfId,
                                              stpPortData.stpId);

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_STP_PORT, xpsDevId, local_id,
                                 stp_port_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, error %d\n", saiRetVal);
        return saiRetVal;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(attributes.stp.oid, SAI_OBJECT_TYPE_STP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(attributes.stp.oid));
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }


    if (!(XDK_SAI_OBJID_TYPE_CHECK(attributes.port.oid,
                                   SAI_OBJECT_TYPE_BRIDGE_PORT)))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(attributes.port.oid));
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }


    XP_SAI_LOG_DBG("xpsStpId %u, xpsIntfId %u, stp_port_state %u, xpsStpState %u (2 - Forwarding)\n",
                   stpPortData.stpId, stpPortData.intfId, attributes.state.s32, stpPortData.state);

    /*Set stp state*/
    xpsRetVal = xpsStpSetState(xpsDevId, stpPortData.stpId, stpPortData.intfId,
                               stpPortData.state);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStpSetState failed stpId %d | retVal : %d \n",
                       stpPortData.stpId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /*3. Add the port to portList of the stp. Not needed to do it explicitly here.
    xps intf-stp tree will be walked through to obtain the list*/

    return xpsStatus2SaiStatus(xpsRetVal);
}


//Func: xpSaiRemoveStpPort

sai_status_t xpSaiRemoveStpPort(sai_object_id_t stp_port_id)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsStpState_e defaultSaiStpState = SPAN_STATE_DISABLE;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    uint32_t stpIntfDbKey = xpSaiObjIdValueGet(stp_port_id);
    xpsStp_t stpId = XPS_STP_GET_STPID_FROM_DBKEY(stpIntfDbKey);
    xpsInterfaceId_t intfId = XPS_STP_GET_INTFID_FROM_DBKEY(stpIntfDbKey);
    xpsStgIntfStateDbEntry *stgIntfState = NULL;
    xpsScope_t      scopeId     = XP_SCOPE_DEFAULT;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    xpsRetVal = xpsStgGetStgIntfEntryDb(scopeId, stpId, intfId, &stgIntfState);
    if (xpsRetVal != XP_NO_ERR || (stgIntfState == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get interface entry failed,"
              " STP PortOid not Found stpId(%d) interface(%d)", stpId, intfId);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    xpsRetVal = xpsStpResetState(xpsDevId, stpId, intfId, defaultSaiStpState);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRemoveStpPort failed stpId %d | retVal : %d \n", stpId,
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiSetStpPortAttribute

sai_status_t xpSaiSetStpPortAttribute(sai_object_id_t stp_port_id,
                                      const sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    saiRetVal = xpSaiAttrCheck(1, attr,
                               STP_PORT_VALIDATION_ARRAY_SIZE, stp_port_attribs,
                               SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    switch (attr->id)
    {
        case SAI_STP_PORT_ATTR_STATE:
            {
                saiRetVal = xpSaiSetStpPortAttrState(stp_port_id, attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_STP_PORT_ATTR_STATE)\n");
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


sai_status_t xpSaiGetStpPortAttribute(sai_object_id_t stp_port_id,
                                      sai_attribute_t* attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    switch (attr->id)
    {
        case SAI_STP_PORT_ATTR_STP:
            {
                saiRetVal = xpSaiGetStpPortAttrStp(stp_port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_STP_PORT_ATTR_STP)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_STP_PORT_ATTR_BRIDGE_PORT:
            {
                saiRetVal = xpSaiGetStpPortAttrBridgePort(stp_port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_STP_PORT_ATTR_BRIDGE_PORT)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_STP_PORT_ATTR_STATE:
            {
                saiRetVal = xpSaiGetStpPortAttrState(stp_port_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_STP_PORT_ATTR_STATE)\n");
                    return saiRetVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return  saiRetVal;
}

//Func: xpSaiGetStpPortAttribute

sai_status_t xpSaiGetStpPortAttributes(sai_object_id_t stp_port_id,
                                       uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               STP_PORT_VALIDATION_ARRAY_SIZE, stp_port_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetStpPortAttribute(stp_port_id, &attr_list[count]);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetStpPortAttributes failed\n");
            return saiRetVal;
        }
    }

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    return  saiRetVal;
}

sai_status_t xpSaiCreateStpPorts(sai_object_id_t switch_id,
                                 uint32_t object_count, const uint32_t *attr_count,
                                 const sai_attribute_t **attr_list, sai_bulk_op_error_mode_t mode,
                                 sai_object_id_t *object_id, sai_status_t *object_statuses)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t count = 0;
    uint32_t idx   = 0;

    if ((attr_count == NULL) || (attr_count == NULL) ||
        (object_id == NULL) || (object_statuses == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!object_count)
    {
        XP_SAI_LOG_ERR("Zero object_count passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (mode != SAI_BULK_OP_ERROR_MODE_STOP_ON_ERROR &&
        mode != SAI_BULK_OP_ERROR_MODE_IGNORE_ERROR)
    {
        XP_SAI_LOG_ERR("Invalid Bulk error mode\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (count = 0; count < object_count; count++)
    {
        object_statuses[count] = xpSaiCreateStpPort(&(object_id[count]), switch_id,
                                                    attr_count[count], attr_list[count]);
        if (object_statuses[count] != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("STP port create failed for index %d\n", count);
            status = SAI_STATUS_FAILURE;

            if (mode == SAI_BULK_OP_ERROR_MODE_STOP_ON_ERROR)
            {
                for (idx = (count + 1); idx < object_count; idx++)
                {
                    object_statuses[idx] = SAI_STATUS_NOT_EXECUTED;
                }
                break;
            }
        }
    }

    return status;
}

sai_status_t xpSaiRemoveStpPorts(uint32_t object_count,
                                 const sai_object_id_t *object_id, sai_bulk_op_error_mode_t mode,
                                 sai_status_t *object_statuses)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t count = 0;
    uint32_t idx   = 0;

    if ((object_id == NULL) || (object_statuses == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!object_count)
    {
        XP_SAI_LOG_ERR("Zero object_count passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (mode != SAI_BULK_OP_ERROR_MODE_STOP_ON_ERROR &&
        mode != SAI_BULK_OP_ERROR_MODE_IGNORE_ERROR)
    {
        XP_SAI_LOG_ERR("Invalid Bulk error mode\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (count = 0; count < object_count; count++)
    {
        object_statuses[count] = xpSaiRemoveStpPort(object_id[count]);
        if (object_statuses[count] != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("STP port delete failed for index %d\n", count);
            status = SAI_STATUS_FAILURE;

            if (mode == SAI_BULK_OP_ERROR_MODE_STOP_ON_ERROR)
            {
                for (idx = (count + 1); idx < object_count; idx++)
                {
                    object_statuses[idx] = SAI_STATUS_NOT_EXECUTED;
                }
                break;
            }
        }
    }

    return status;
}

//Func: xpSaiStpApiInit

XP_STATUS xpSaiStpApiInit(uint64_t flag,
                          const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    _xpSaiStpApi = (sai_stp_api_t *) xpMalloc(sizeof(sai_stp_api_t));
    if (NULL == _xpSaiStpApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiStpApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiStpApi->create_stp = xpSaiCreateStp;
    _xpSaiStpApi->remove_stp = xpSaiRemoveStp;
    _xpSaiStpApi->set_stp_attribute = xpSaiSetStpAttribute;
    _xpSaiStpApi->get_stp_attribute = xpSaiGetStpAttributes;
    _xpSaiStpApi->create_stp_port = xpSaiCreateStpPort;
    _xpSaiStpApi->remove_stp_port = xpSaiRemoveStpPort;
    _xpSaiStpApi->set_stp_port_attribute = xpSaiSetStpPortAttribute;
    _xpSaiStpApi->get_stp_port_attribute = xpSaiGetStpPortAttributes;

    _xpSaiStpApi->create_stp_ports = xpSaiCreateStpPorts;
    _xpSaiStpApi->remove_stp_ports = xpSaiRemoveStpPorts;

    saiRetVal = xpSaiApiRegister(SAI_API_STP, (void*)_xpSaiStpApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register STP API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}


//Func: xpSaiStpApiDeinit

XP_STATUS xpSaiStpApiDeinit()
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    xpFree(_xpSaiStpApi);
    _xpSaiStpApi = NULL;

    return xpsRetVal;
}

sai_status_t xpSaiMaxCountStpAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_STP_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountStpObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStgGetStgCount(XP_SCOPE_DEFAULT, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetStpObjectList(uint32_t *object_count,
                                   sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpsScope_t      scopeId     = XP_SCOPE_DEFAULT;
    xpsStp_t       *stpId;
    xpsDevice_t     devId       = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiCountStpObjects(&objCount);
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

    stpId = (xpsStp_t *)xpMalloc(sizeof(xpsStp_t) * (*object_count));
    if (stpId == NULL)
    {
        XP_SAI_LOG_ERR("Could not allocate memory for stpId array!\n");
        return SAI_STATUS_NO_MEMORY;
    }

    retVal = xpsStpGetStpIdList(scopeId, stpId);
    if (retVal != XP_NO_ERR)
    {
        xpFree(stpId);
        return xpsStatus2SaiStatus(retVal);
    }

    *object_count = objCount;

    for (uint32_t i = 0; i < *object_count; i++)
    {
        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_STP, devId, (sai_uint64_t)stpId[i],
                                     &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            xpFree(stpId);
            return saiRetVal;
        }
    }

    xpFree(stpId);
    return SAI_STATUS_SUCCESS;
}

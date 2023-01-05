// xpSaiVlan.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiVlan.h"
#include "xpSaiPort.h"
#include "xpsVlan.h"
#include "xpSaiSwitch.h"
#include "xpSaiBridge.h"
#include "xpSaiLag.h"
#include "xpSaiValidationArrays.h"
#include "xpSaiAclMapper.h"
#include "xpSaiStub.h"

#define XP_SAI_MAX_LEARNED_ADDRESSES  (64*1024)

XP_SAI_LOG_REGISTER_API(SAI_API_VLAN);

extern xpsDbHandle_t portLagPortCountDbHandle;

static sai_vlan_api_t* _xpSaiVlanApi;

static xpsDbHandle_t xpSaiVlanStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;

extern sai_status_t xpSaiLagConfigInfoGet(xpsInterfaceId_t lagId,
                                          xpSaiLagInfo_t **lagConfig);

#define XPSAI_VLAN_MASK 0xFFF;
static int32_t saiVlanCtxKeyComp(void* key1, void* key2)
{
    return ((((xpSaiVlanContextDbEntry *) key1)->vlanId) - (((
                                                                 xpSaiVlanContextDbEntry *) key2)->vlanId));
}

XP_STATUS xpSaiVlanGetVlanCtxDb(xpsScope_t scopeId, xpsVlan_t vlanId,
                                xpSaiVlanContextDbEntry ** vlanCtxPtr)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiVlanContextDbEntry keyVlanCtx;

    memset(&keyVlanCtx, 0x0, sizeof(xpSaiVlanContextDbEntry));
    keyVlanCtx.vlanId= vlanId;
    if ((retVal = xpsStateSearchData(scopeId, xpSaiVlanStateDbHndl,
                                     (xpsDbKey_t)&keyVlanCtx, (void**)vlanCtxPtr)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search data failed, vlanId(%d) | retVal : %d \n", vlanId,
                       retVal);
        return retVal;
    }

    if (!(*vlanCtxPtr))
    {
        XP_SAI_LOG_ERR("Vlan does not exist, vlanId:%d \n", vlanId);
        return XP_ERR_KEY_NOT_FOUND;
    }

    return retVal;
}

static XP_STATUS xpSaiRemoveVlanCtxDb(xpsScope_t scopeId, xpsVlan_t vlanId)
{

    XP_STATUS retVal = XP_NO_ERR;
    xpSaiVlanContextDbEntry * vlanCtx = NULL;
    xpSaiVlanContextDbEntry keyVlanCtx;

    memset(&keyVlanCtx, 0x0, sizeof(xpSaiVlanContextDbEntry));
    keyVlanCtx.vlanId= vlanId;

    if ((retVal = xpsStateDeleteData(scopeId, xpSaiVlanStateDbHndl,
                                     (xpsDbKey_t)&keyVlanCtx, (void**)&vlanCtx)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Delete vlan data failed, vlanId(%d) | retVal : %d \n", vlanId,
                       retVal);
        return retVal;
    }

    if ((retVal = xpsStateHeapFree((void*)vlanCtx)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("XPS state Heap Free failed\n");
        return retVal;
    }

    return retVal;
}

static XP_STATUS xpSaiInsertVlanCtxDb(xpsScope_t scopeId, xpsVlan_t vlanId,
                                      xpSaiVlanContextDbEntry ** vlanCtxPtr)
{
    XP_STATUS retVal = XP_NO_ERR;

    if ((retVal = xpsStateHeapMalloc(sizeof(xpSaiVlanContextDbEntry),
                                     (void**)vlanCtxPtr)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("XPS state Heap Allocation failed \n");
        return retVal;
    }
    if (*vlanCtxPtr == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    memset(*vlanCtxPtr, 0, sizeof(xpSaiVlanContextDbEntry));

    (*vlanCtxPtr)->vlanId = vlanId;

    // Insert the vlan context into the database, vlanId is the key
    if ((retVal = xpsStateInsertData(scopeId, xpSaiVlanStateDbHndl,
                                     (void*)*vlanCtxPtr)) != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)*vlanCtxPtr);
        *vlanCtxPtr = NULL;
        return retVal;
    }

    return retVal;
}

void xpSaiVlanUpdatePktCmd(bool cpuFloodEnable, xpsPktCmd_e *pktCmd)
{
    if (!pktCmd)
    {
        XP_SAI_LOG_ERR("Invalid pktCmd \n");
        return;
    }

    if (cpuFloodEnable == true)
    {
        switch (*pktCmd)
        {
            case XP_PKTCMD_FWD:
            case XP_PKTCMD_FWD_MIRROR:
                {
                    *pktCmd = XP_PKTCMD_FWD_MIRROR;
                    break;
                }
            case XP_PKTCMD_DROP:
            case XP_PKTCMD_TRAP:
                {
                    *pktCmd = XP_PKTCMD_TRAP;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_WARNING("Unknown pktCmd: %d \n", *pktCmd);
                    break;
                }
        }
    }
}

/*This is called in switch init*/
XP_STATUS xpSaiVlanInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS retVal = XP_NO_ERR;

    // Create global SAI VLAN DB
    xpSaiVlanStateDbHndl = XPSAI_VLAN_STATE_DB_HNDL;
    if ((retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "SAI VLAN DB", XPS_GLOBAL,
                                     &saiVlanCtxKeyComp, xpSaiVlanStateDbHndl)) != XP_NO_ERR)
    {
        xpSaiVlanStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create SAI VLAN DB\n");
        return retVal;
    }

    return retVal;
}

XP_STATUS xpSaiVlanDeInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS retVal = XP_NO_ERR;

    // Purge global SAI VLAN DB
    if ((retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                       &xpSaiVlanStateDbHndl)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge SAI VLAN DB\n");
        return retVal;
    }

    return retVal;
}

/*  VLAN Member hash implemantetion*/
/*
* Routine Description:
*     Query VLAN ID from sai object value.
*
* Arguments:
*     [out] sai_object_id_t
*
* Return Values:
*    sai_uint64_t : only 40 bits of income value are usable for ID storage
*/
sai_uint64_t xpSaiVlanMemberVlanIdValueGet(sai_object_id_t sai_object_id)
{
    /*Income validation haven't sense as there will be present upcoming validation for the local identifier.*/
    return (sai_object_id >> SAI_VLAN_MEMBER_SUB_OBJID_VALUE_BITS) &
           SAI_VLAN_MEMBER_OBJ_VALUE_MASK;
}

/*
* Routine Description:
*     Query Port ID from sai object value.
*
* Arguments:
*     [out] sai_object_id_t
*
* Return Values:
*    sai_uint64_t : only 40 bits of income value are usable
*/
sai_uint64_t xpSaiVlanMemberPortIdValueGet(sai_object_id_t sai_object_id)
{
    /*Income validation haven't sense as there will be present upcoming validation for the local identifier.*/
    return sai_object_id & SAI_VLAN_MEMBER_OBJ_VALUE_MASK;
}

/*
* Routine Description:
*     Creates SAI member Object ID.
*
* Arguments:
*     [in] sai_uint32_t : device ID
*     [in] sai_uint64_t : vlan ID
*     [in] sai_uint64_t : only 20 bits of income value are being used
*     [out] sai_object_id_t
*
* Return Values:
*     sai_status_t
*/
sai_status_t xpSaiVlanMemberObjIdCreate(sai_uint32_t dev_id,
                                        sai_uint64_t vlan_id, sai_uint64_t port_id, sai_object_id_t *sai_object_id)
{
    const sai_object_type_t sai_object_type = SAI_OBJECT_TYPE_VLAN_MEMBER;

    if ((SAI_OBJECT_TYPE_NULL >= sai_object_type) &&
        (SAI_OBJECT_TYPE_MAX <= sai_object_type))
    {
        XP_SAI_LOG_ERR("Invalid SAI object type received : %u.\n", sai_object_type);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((vlan_id > SAI_VLAN_MEMBER_OBJ_VALUE_MASK) ||
        (port_id > SAI_VLAN_MEMBER_OBJ_VALUE_MASK))
    {
        XP_SAI_LOG_ERR("Provided local identifier is to big (vlan_id=%lu, port_id=%lu) and can not be paked. "
                       "Only 20 bits are allowed.\n", (long unsigned int) vlan_id,
                       (long unsigned int) port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_uint64_t vlan_tmp = vlan_id << SAI_VLAN_MEMBER_SUB_OBJID_VALUE_BITS;

    return xpSaiObjIdCreate(sai_object_type, dev_id, vlan_tmp | port_id,
                            sai_object_id);
}

/* End of VLAN member hash implementaton */

//Func: xpSaiUpdateVlanAttributeVals

void xpSaiUpdateVlanAttributeVals(const uint32_t attr_count,
                                  const sai_attribute_t* attr_list, xpSaiVlanAttributesT* attributes)
{
    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_VLAN_ATTR_VLAN_ID:
                {
                    attributes->vlanId = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES:
                {
                    attributes->maxLearnedAddresses = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_STP_INSTANCE:
                {
                    attributes->stpInstance = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_LEARN_DISABLE:
                {
                    attributes->learnDisable = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_COUNT_MODE:
                {
                    attributes->countMode = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_INGRESS_ACL:
                {
                    attributes->ingressAcl = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_EGRESS_ACL:
                {
                    attributes->egressAcl = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
                {
                    attributes->unknownUcFloodControlType = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
                {
                    attributes->unknownUcFloodGroup = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
                {
                    attributes->unknownMcFloodControlType = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
                {
                    attributes->unknownMcFloodGroup = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
                {
                    attributes->broadcastFloodControlType = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_BROADCAST_FLOOD_GROUP:
                {
                    attributes->broadcastFloodGroup = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_IPV4_MCAST_LOOKUP_KEY_TYPE:
                {
                    attributes->broadcastFloodGroup = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_IPV6_MCAST_LOOKUP_KEY_TYPE:
                {
                    attributes->broadcastFloodGroup = attr_list[count].value;
                    break;
                }
            case SAI_VLAN_ATTR_CUSTOM_IGMP_SNOOPING_ENABLE:
                {
                    attributes->igmpSnoopingEnable = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }

    }

}

//Func: xpSaiGetFdbTableDepth

static XP_STATUS xpSaiGetFdbTableDepth(xpsDevice_t xpsDevId, uint32_t *fdbLimit)
{
    return XP_NO_ERR;
}


//Func: xpSaiCreateVlan

sai_status_t xpSaiCreateVlan(_Out_ sai_object_id_t *vlanIdObj,
                             _In_ sai_object_id_t switch_id,
                             _In_ uint32_t attr_count,
                             _In_ const sai_attribute_t *attr_list)
{
    XP_STATUS               retVal         = XP_NO_ERR;
    sai_status_t            saiRetVal      = SAI_STATUS_SUCCESS;
    xpsVlanConfig_t         vlanConfig;
    sai_uint16_t            vlanId         = 1;
    xpSaiVlanAttributesT    attributes;
    xpSaiVlanContextDbEntry *saiVlanCtx;
    uint32_t                countingEnable = 1;
    bool                    enableFlag     = false;     //Default value false
    xpsDevice_t             devId          = xpSaiGetDevId();

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    memset(&vlanConfig, 0, sizeof(xpsVlanConfig_t));
    memset(&attributes, 0, sizeof(xpSaiVlanAttributesT));

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               VLAN_VALIDATION_ARRAY_SIZE, vlan_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    xpSaiUpdateVlanAttributeVals(attr_count, attr_list, &attributes);

    vlanId = attributes.vlanId.u16;

    xpsDevice_t xpsDevId =
        xpSaiGetDevId();  //TODO: revisit after multidevice changes
    //xpsDevice_t xpsDevId = (xpsDevice_t) switch_id;

    retVal = xpsVlanCreate(xpsDevId, vlanId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanCreate failed vlanId %d | retVal : %d \n", vlanId,
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /*Create vlan object after successful creation of vlan*/
    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, devId, vlanId, vlanIdObj);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI vlan object could not be created\n");
        return saiRetVal;
    }

    retVal = xpSaiInsertVlanCtxDb(XP_SCOPE_DEFAULT, vlanId, &saiVlanCtx);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Insert sai vlan context failed, vlanId(%d)", vlanId);
        return retVal;
    }

    saiVlanCtx->vlanId = vlanId;

    saiVlanCtx->monitorPortRefCount = 0;

    retVal = xpSaiBridgeCreateMappingEntry(*vlanIdObj);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiBridgeCreateMappingEntry failed vlanId %d | retVal : %d \n",
                       vlanId, retVal);
        return retVal;
    }

    /*Set the max learned address limit for the vlan. A max address of 0 means there
      is no limit from SAI application. As XPS creates vlan with maximum hw limit
      as vlan's limit to learn by default, only non-zero inputs need to be processed.*/
    if (attributes.maxLearnedAddresses.u32)
    {
        retVal = xpsVlanSetFdbLimit(xpsDevId, vlanId,
                                    attributes.maxLearnedAddresses.u32);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsVlanSetFdbLimit failed vlanId %d | retVal : %d \n", vlanId,
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    saiVlanCtx->maxLearnedAddresses = attributes.maxLearnedAddresses.u32;

    /* Get default value and then update */
    retVal = xpsVlanGetConfig(xpsDevId, vlanId, &vlanConfig);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanGetConfig failed vlanId %d | retVal : %d \n", vlanId,
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    vlanConfig.stpId = xpSaiObjIdValueGet(attributes.stpInstance.oid);

    vlanConfig.arpBcCmd         = XP_PKTCMD_FWD_MIRROR;
    vlanConfig.saMissCmd        = XP_PKTCMD_FWD_MIRROR;
    vlanConfig.bcCmd            = XP_PKTCMD_FWD;

    /*Disable learning based on config*/
    if (attributes.learnDisable.booldata == true)
    {
        vlanConfig.saMissCmd  = XP_PKTCMD_FWD;
    }

    /* If broadcastFloodControlType attribute has not been specified then value will be 0, which is the default value as well */
    saiVlanCtx->broadcastFloodType = attributes.broadcastFloodControlType.s32;
    saiRetVal = xpSaiConvertSaiFloodControlType2xps((sai_vlan_flood_control_type_t)
                                                    saiVlanCtx->broadcastFloodType, &vlanConfig.bcCmd);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("xpSaiConvertSaiFloodControlType2xps failed. pktAction = %d, retVal = %d \n",
                       (sai_vlan_flood_control_type_t)saiVlanCtx->broadcastFloodType, saiRetVal);
        return saiRetVal;
    }

    /* If unknownUcFloodControlType attribute has not been specified then value will be 0, which is the default value as well */
    saiVlanCtx->unknownUnicastFloodType = attributes.unknownUcFloodControlType.s32;
    saiRetVal = xpSaiConvertSaiFloodControlType2xps((sai_vlan_flood_control_type_t)
                                                    saiVlanCtx->unknownUnicastFloodType, &vlanConfig.unknownUcCmd);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("xpSaiConvertSaiFloodControlType2xps failed. pktAct = %d, retVal = %d \n",
                       (sai_vlan_flood_control_type_t)saiVlanCtx->unknownUnicastFloodType, saiRetVal);
        return saiRetVal;
    }

    vlanConfig.arpBcCmd         = XP_PKTCMD_FWD_MIRROR;
    vlanConfig.icmpv6Cmd        = XP_PKTCMD_FWD_MIRROR;

    /* If unknownMcFloodControlType attribute has not been specified then value will be 0, which is the default value as well */
    saiVlanCtx->unregMulticastFloodType = attributes.unknownMcFloodControlType.s32;
    saiRetVal = xpSaiConvertSaiFloodControlType2xps((sai_vlan_flood_control_type_t)
                                                    saiVlanCtx->unregMulticastFloodType, &vlanConfig.unRegMcCmd);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("xpSaiConvertSaiFloodControlType2xps failed. pktAct = %d, retVal = %d \n",
                       (sai_vlan_flood_control_type_t)saiVlanCtx->unregMulticastFloodType, saiRetVal);
        return saiRetVal;
    }

    /*Enable L2 broadcast flood control to CPU port (Bcast and DA miss)*/
    saiRetVal = xpSaiGetSwitchAttrBcastCpuFloodEnable(xpsDevId, &enableFlag);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get xpSaiGetSwitchAttrBcastCpuFloodEnable()!\n");
        return saiRetVal;
    }

    if ((vlanConfig.unknownUcCmd == XP_PKTCMD_FWD) && (enableFlag == true))
    {
        /* Enable/disable unknown unicast flood control to CPU port */
        vlanConfig.unknownUcCmd = XP_PKTCMD_FWD_MIRROR;
    }

    if ((vlanConfig.bcCmd == XP_PKTCMD_FWD) && (enableFlag == true))
    {
        /* Enable/disable broadcast flood control to CPU port */
        vlanConfig.bcCmd =  XP_PKTCMD_FWD_MIRROR;
    }

    /*Enable L2 multicast flood control to CPU port*/
    saiRetVal = xpSaiGetSwitchAttrMcastCpuFloodEnable(xpsDevId, &enableFlag);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get xpSaiGetSwitchAttrMcastCpuFloodEnable()!\n");
        return saiRetVal;
    }

    if ((vlanConfig.unRegMcCmd == XP_PKTCMD_FWD) && (enableFlag == true))
    {
        /* Enable/disable multicast flood control to CPU port */
        vlanConfig.unRegMcCmd =  XP_PKTCMD_FWD_MIRROR;
    }

    retVal = xpsVlanSetConfig(xpsDevId, vlanId, &vlanConfig);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanSetConfig failed vlanId %d | retVal : %d \n", vlanId,
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    uint32_t countMode = attributes.countMode.s32;

    /*Enable packet and octet counting on ingress and egress bd*/
    switch (countMode)
    {
        case SAI_VLAN_COUNT_MODE_PACKET_OCTET_BOTH:
            {
                /*Octet support for counting.*/
                int clearOnRead = 1;
                int wrapAround = 1;
                int countOffset = 15; //By default 15. For taking care of FCS and preamble

                retVal = xpsSetCountMode(xpsDevId, XP_ACM_ING_BD_COUNTER, XP_ACM_COUNTING,
                                         XP_ANA_BANK_MODE_B,
                                         clearOnRead, wrapAround, countOffset);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsSetCountMode failed retVal : %d \n", retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                retVal = xpsSetCountMode(xpsDevId, XP_ACM_EGR_BD_COUNTER, XP_ACM_COUNTING,
                                         XP_ANA_BANK_MODE_B,
                                         clearOnRead, wrapAround, countOffset);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsSetCountMode failed retVal : %d \n", retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                break;
            }
        case SAI_VLAN_COUNT_MODE_PACKET_ONLY:
            {
                /*Do nothing*/
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("For wrong countMode received(%u)\n", countMode);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    /*Create counters*/

    /*1. Enable ingress and egress couting*/
    retVal = xpsVlanSetCountingEnable(xpsDevId, XP_INGRESS, countingEnable);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanSetCountingEnable failed retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /*2. Enable egress couting*/
    retVal = xpsVlanSetCountingEnable(xpsDevId, XP_EGRESS, countingEnable);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanSetCountingEnable failed retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }


    /*3. Set ingress count mode. vlanId is the counter index.*/
    retVal = xpsVlanSetCountMode(xpsDevId, vlanId,
                                 BD_COUNT_MODE_ROUTED_AND_SWITCHED);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanSetCountingEnable failed retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /*4. Allocate and set egress counter*/
    xpAcm_t egressCounterId = 0;
    retVal = xpsAllocateAcm(XP_ACM_EGR_BD_COUNTER, XP_ACM_COUNTING,
                            &egressCounterId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAllocateAcm failed retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiVlanCtx->egrCounterId = egressCounterId;

    retVal = xpsVlanSetEgressCounterId(xpsDevId, vlanId, egressCounterId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanSetEgressCounterId failed retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    xpsVlanBridgeMcMode_e vlanBrMcMode = MC_BRIDGE_MODE_MAX;
    if (!attributes.ipv4McastLookupKeyType.s32)
    {
        // set default value
        vlanBrMcMode = MC_BRIDGE_MODE_FDB;
    }
    else
    {
        saiRetVal = xpSaiConvertSaiMcastLookupKeyType2xps((
                                                              sai_vlan_mcast_lookup_key_type_t)attributes.ipv4McastLookupKeyType.s32,
                                                          &vlanBrMcMode);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("xpSaiConvertSaiMcastLookupKeyType2xps failed. sai_vlan_mcast_lookup_key = %d, saiRetVal = %d \n",
                           attributes.ipv4McastLookupKeyType.s32, saiRetVal);
            return saiRetVal;
        }
    }

    retVal = xpsVlanSetIpv4McBridgeMode(xpsDevId, vlanId, vlanBrMcMode);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("xpsVlanSetIpv4McBridgeMode failed. sai_vlan_mcast_lookup_key = %d, vlanId = %d, retVal = %d \n",
                       attributes.ipv4McastLookupKeyType.s32, vlanId, retVal);
        return retVal;
    }


    if (!attributes.ipv6McastLookupKeyType.s32)
    {
        // set default value
        vlanBrMcMode = MC_BRIDGE_MODE_FDB;
    }
    else
    {
        saiRetVal = xpSaiConvertSaiMcastLookupKeyType2xps((
                                                              sai_vlan_mcast_lookup_key_type_t)attributes.ipv6McastLookupKeyType.s32,
                                                          &vlanBrMcMode);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("xpSaiConvertSaiMcastLookupKeyType2xps failed. sai_vlan_mcast_lookup_key = %d, saiRetVal = %d \n",
                           attributes.ipv6McastLookupKeyType.s32, saiRetVal);
            return saiRetVal;
        }
    }

    retVal = xpsVlanSetIpv6McBridgeMode(xpsDevId, vlanId, vlanBrMcMode);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("xpsVlanSetIpv6McBridgeMode failed. sai_vlan_mcast_lookup_key = %d, vlanId = %d, retVal = %d \n",
                       attributes.ipv6McastLookupKeyType.s32, vlanId, retVal);
        return retVal;
    }


    saiRetVal = xpSaiSetVlanAttrIgmpSnoopingEnable(vlanId,
                                                   attributes.igmpSnoopingEnable);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("xpSaiSetVlanAttrIgmpSnoopingEnable failed vlanId %u | saiRetVal: %d \n",
                       vlanId, saiRetVal);
        return saiRetVal;
    }


    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveVlan

sai_status_t xpSaiRemoveVlan(sai_object_id_t vlan_id)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsVlan_t xpsVlanId = xpSaiObjIdValueGet(vlan_id);

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(vlan_id, SAI_OBJECT_TYPE_VLAN))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(vlan_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    retVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, xpsVlanId, &saiVlanCtx);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVlanGetVlanCtxDb failed retVal : %d \n", retVal);
        /*
         * As per SAI 1.0 header, If Invalid SAI Object ID passed to a function and SAI_STATUS_INVALID_OBJECT_TYPE is
         * not a case (Object is not found in DB) then return SAI_STATUS_INVALID_OBJECT_ID.
         * In this case, SAI_STATUS_INVALID_VLAN_ID is more suitable than SAI_STATUS_INVALID_OBJECT_ID
         */
        if (retVal == XP_ERR_KEY_NOT_FOUND)
        {
            return SAI_STATUS_INVALID_VLAN_ID;
        }
        return xpsStatus2SaiStatus(retVal);
    }

    /*Release the egress vlan counter*/
    retVal = xpsReleaseAcm(XP_ACM_EGR_BD_COUNTER, XP_ACM_COUNTING,
                           saiVlanCtx->egrCounterId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsReleaseAcm failed retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsVlanDestroy(xpsDevId, xpsVlanId);

    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanDestroy failed vlanId %u | retVal : %d \n", xpsVlanId,
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiBridgeRemoveMappingEntry(vlan_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiBridgeRemoveMappingEntry failed vlanId %u | retVal : %d \n",
                       xpsVlanId, retVal);
        return retVal;
    }

    retVal = xpSaiRemoveVlanCtxDb(XP_SCOPE_DEFAULT, xpsVlanId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Remove sai vlan context failed, vlanId(%d)", xpsVlanId);
        return retVal;
    }

    return SAI_STATUS_SUCCESS;

}

//Func: xpSaiVlanNotifyAddAcl

sai_status_t xpSaiVlanNotifyAddAcl(xpsDevice_t devId,
                                   sai_object_id_t vlanObjId, sai_object_id_t aclObjId)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    saiStatus = xpSaiAclMapperBindToTableGroup(devId, aclObjId, vlanObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to bind vlan to table group, status:%d\n", saiStatus);
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetVlanAttrAcl

sai_status_t xpSaiSetVlanAttrAcl(sai_object_id_t vlanObjId,
                                 sai_attribute_value_t value, bool isIngress)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus    = XP_NO_ERR;
    sai_uint32_t        tableId     = 0;
    sai_object_id_t     aclObjId    = value.oid;
    xpsDevice_t         devId       = xpSaiGetDevId();
    sai_uint32_t        groupId     = 0;
    sai_object_id_t prevOid = SAI_NULL_OBJECT_ID;
    xpSaiAclTableIdMappingT  *pSaiAclTableAttribute = NULL;
    xpSaiVlanContextDbEntry *pVlanEntry = NULL;
    xpsVlan_t                vlanId     = 0;

    XP_SAI_LOG_DBG("Calling xpSaiSetVlanAttrIngressAcl\n");

    devId  = (xpsDevice_t)xpSaiObjIdSwitchGet(vlanObjId);
    vlanId = (sai_uint32_t)xpSaiObjIdValueGet(vlanObjId);
    // Get the acl table id from table object

    if (!XDK_SAI_OBJID_TYPE_CHECK(vlanObjId, SAI_OBJECT_TYPE_VLAN))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).",
                       xpSaiObjIdTypeGet(vlanObjId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpStatus = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, vlanId, &pVlanEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVlanGetVlanCtxDb failed error:%d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (isIngress)
    {
        if (pVlanEntry->ingressAclId == aclObjId)
        {
            return SAI_STATUS_SUCCESS;
        }
        prevOid = pVlanEntry->ingressAclId;
    }
    else
    {
        if (pVlanEntry->egressAclId == aclObjId)
        {
            return SAI_STATUS_SUCCESS;
        }
        prevOid = pVlanEntry->egressAclId;
    }

    if (prevOid != SAI_NULL_OBJECT_ID)
    {
        if (XDK_SAI_OBJID_TYPE_CHECK(prevOid, SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
        {
            saiStatus = xpSaiAclMapperUnBindFromTableGroup(devId, prevOid,
                                                           vlanObjId);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to notify add ingress acl, vlan:%d, status:%d\n",
                               vlanId, saiStatus);
                return saiStatus;
            }
        }
        else if (XDK_SAI_OBJID_TYPE_CHECK(prevOid, SAI_OBJECT_TYPE_ACL_TABLE))
        {
            tableId = (uint32_t)xpSaiObjIdValueGet(prevOid);
            xpStatus = xpsAclVlanUnbind(devId, vlanId, groupId, tableId);

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
            saiStatus = xpSaiVlanNotifyAddAcl(devId, vlanObjId, aclObjId);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_DBG("Failed to notify add acl, vlan:%d, status:%d\n", vlanId,
                               saiStatus);
                return saiStatus;
            }
        }
        else
        {
            // Get the acl table id from table object
            tableId = (xpsInterfaceId_t)xpSaiObjIdValueGet(aclObjId);
            saiStatus = xpSaiAclTableAttributesGet(tableId, &pSaiAclTableAttribute);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not get Table Entry, ret %d\n", saiStatus);
                return saiStatus;
            }

            xpStatus = xpsAclSetVlanAcl(devId, vlanId, tableId, groupId);
            if (xpStatus != XP_NO_ERR)
            {
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
    }

    // Update state database
    if (isIngress)
    {
        pVlanEntry->ingressAclId = aclObjId;
    }
    else
    {
        pVlanEntry->egressAclId = aclObjId;
    }

    return saiStatus;
}

//Func: xpSaiGetVlanAttrAcl

sai_status_t xpSaiGetVlanAttrAcl(xpsDevice_t devId, xpsVlan_t vlanId,
                                 sai_attribute_value_t* value, bool isIngress)
{
    xpSaiVlanContextDbEntry *pVlanEntry = NULL;
    sai_status_t             saiStatus  = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiGetVlanAttrAcl\n");

    saiStatus = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, vlanId, &pVlanEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiVlanGetVlanCtxDb failed error : %d \n", saiStatus);
        return saiStatus;
    }

    if (isIngress)
    {
        value->oid = pVlanEntry->ingressAclId;
    }
    else
    {
        value->oid = pVlanEntry->egressAclId;
    }
    return  saiStatus;
}

//Func: xpSaiGetVlanAttrEgressAcl

sai_status_t xpSaiGetVlanAttrEgressAcl(xpsVlan_t xpsVlanId,
                                       sai_attribute_value_t* value)
{
    sai_status_t            saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;
    xpsDevice_t             devId       = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Calling xpSaiGetVlanAttrEgressAcl\n");

    saiStatus = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, xpsVlanId, &saiVlanCtx);
    if (saiStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVlanGetVlanCtxDb failed retVal : %d \n", saiStatus);
        return saiStatus;
    }

    if (saiVlanCtx->egressAclId == 0)
    {
        value->oid = SAI_NULL_OBJECT_ID;
    }
    else
    {
        saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ACL_TABLE, devId,
                                     (sai_uint64_t)saiVlanCtx->egressAclId, &value->oid);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error :Not able to get Table OID\n");
            return saiStatus;
        }

    }

    return  saiStatus;
}

//Func: xpSaiSetVlanAttrUnknownUnicastFloodControlType
sai_status_t xpSaiSetVlanAttrUnknownUnicastFloodControlType(xpsVlan_t xpsVlanId,
                                                            sai_int32_t value, bool cpuFloodEnable)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;
    xpsPktCmd_e pktCmd = XP_PKTCMD_FWD;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    retVal = xpSaiConvertSaiFloodControlType2xps((sai_vlan_flood_control_type_t)
                                                 value, &pktCmd);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("xpSaiConvertSaiFloodControlType2xps failed. pktCmd = %d, retVal = %d \n",
                       value, retVal);
        return retVal;
    }

    // If CPU flooding is enabled - set appropriate pktCmd
    xpSaiVlanUpdatePktCmd(cpuFloodEnable, &pktCmd);

    xpsRetVal = xpsVlanSetUnknownUcCmd(xpsDevId, xpsVlanId, pktCmd);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanSetUnknownSaCmd() failed vlanId=%u, retVal=%d\n",
                       xpsVlanId, xpsRetVal);
        return retVal;
    }

    xpsRetVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, xpsVlanId, &saiVlanCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVlanGetVlanCtxDb failed retVal : %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    saiVlanCtx->unknownUnicastFloodType = value;

    return retVal;
}

//Func: xpSaiSetVlanAttrUnknownMulticastFloodControlType
sai_status_t xpSaiSetVlanAttrUnknownMulticastFloodControlType(
    xpsVlan_t xpsVlanId, sai_int32_t value, bool cpuFloodEnable)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;
    xpsPktCmd_e pktCmd = XP_PKTCMD_FWD;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    xpsRetVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, xpsVlanId, &saiVlanCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVlanGetVlanCtxDb failed retVal : %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (saiVlanCtx->unregMulticastFloodType == value)
    {
        // Unknown Multicast Flood Control Type is already correct
        return SAI_STATUS_SUCCESS;
    }

    retVal = xpSaiConvertSaiFloodControlType2xps((sai_vlan_flood_control_type_t)
                                                 value, &pktCmd);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("xpSaiConvertSaiFloodControlType2xps failed. pktCmd = %d, retVal = %d \n",
                       value, retVal);
        return retVal;
    }

    // If CPU flooding is enabled - set appropriate pktCmd
    xpSaiVlanUpdatePktCmd(cpuFloodEnable, &pktCmd);

    xpsRetVal = xpsVlanSetUnregMcCmd(xpsDevId, xpsVlanId, pktCmd);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanSetUnregMcCmd() failed vlanId=%u, retVal=%d\n",
                       xpsVlanId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    saiVlanCtx->unregMulticastFloodType = value;

    return retVal;
}

//Func: xpSaiSetVlanAttrIgmpSnoopingEnable

sai_status_t xpSaiSetVlanAttrIgmpSnoopingEnable(xpsVlan_t xpsVlanId,
                                                sai_attribute_value_t value)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsPktCmd_e igmpCmd;
    xpsDevice_t devId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Calling xpSaiSetVlanAttrIgmpSnoopingEnable\n");

    igmpCmd = (value.booldata == true) ? XP_PKTCMD_FWD_MIRROR : XP_PKTCMD_FWD;

    retVal = xpsVlanSetIgmpCmd(devId, xpsVlanId, igmpCmd);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Set IGMP packet command failed retVal: %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    // Enable when MLD supported
    /*
        retVal = xpsVlanSetIcmpv6Cmd(devId, xpsVlanId, igmpCmd);
        if(retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Set ICMP v6 packet command failed retVal: %d \n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    */

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiGetVlanAttrIgmpSnoopingEnable

sai_status_t xpSaiGetVlanAttrIgmpSnoopingEnable(xpsVlan_t xpsVlanId,
                                                sai_attribute_value_t *value)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsPktCmd_e igmpCmd = XP_PKTCMD_DROP;
    xpsDevice_t devId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Calling xpSaiGetVlanAttrIgmpSnoopingEnable\n");

    retVal = xpsVlanGetIgmpCmd(devId, xpsVlanId, &igmpCmd);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Get IGMP packet command failed retVal: %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (igmpCmd == XP_PKTCMD_FWD_MIRROR)
    {
        value->booldata = true;
    }
    else if (igmpCmd == XP_PKTCMD_FWD)
    {
        value->booldata = false;
    }
    else
    {
        XP_SAI_LOG_ERR("Incorrect IGMP packet command retrieved. igmpCmd = %d \n",
                       igmpCmd);
        return SAI_STATUS_FAILURE;
    }

    return  xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiSetVlanAttrBroadcastFloodControlType
sai_status_t xpSaiSetVlanAttrBroadcastFloodControlType(xpsVlan_t xpsVlanId,
                                                       sai_int32_t value, bool cpuFloodEnable)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;
    xpsPktCmd_e pktCmd = XP_PKTCMD_FWD;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    retVal = xpSaiConvertSaiFloodControlType2xps((sai_vlan_flood_control_type_t)
                                                 value, &pktCmd);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("xpSaiConvertSaiFloodControlType2xps failed. pktCmd = %d, retVal = %d \n",
                       value, retVal);
        return retVal;
    }

    // If CPU flooding is enabled - set appropriate pktCmd
    xpSaiVlanUpdatePktCmd(cpuFloodEnable, &pktCmd);

    xpsRetVal = xpsVlanSetBcCmd(xpsDevId, xpsVlanId, pktCmd);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanSetBcCmd() failed vlanId=%u, retVal=%d\n", xpsVlanId,
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    xpsRetVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, xpsVlanId, &saiVlanCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVlanGetVlanCtxDb failed retVal : %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    saiVlanCtx->broadcastFloodType = value;

    return retVal;
}

//Func: xpSaiGetVlanAttrUnknownUnicastFloodControlType
sai_status_t xpSaiGetVlanAttrUnknownUnicastFloodControlType(xpsVlan_t xpsVlanId,
                                                            sai_int32_t* value)
{
    XP_STATUS               xpsRetVal   = XP_NO_ERR;
    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;

    if (!value)
    {
        XP_SAI_LOG_ERR("Invelid parameter: value\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsRetVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, xpsVlanId, &saiVlanCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVlanGetVlanCtxDb failed retVal: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    *value = saiVlanCtx->unknownUnicastFloodType;

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiGetVlanAttrUnknownMulticastFloodControlType
sai_status_t xpSaiGetVlanAttrUnknownMulticastFloodControlType(
    xpsVlan_t xpsVlanId,  sai_int32_t* value)
{
    XP_STATUS               xpsRetVal   = XP_NO_ERR;
    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;

    if (!value)
    {
        XP_SAI_LOG_ERR("Invelid parameter: value\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsRetVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, xpsVlanId, &saiVlanCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVlanGetVlanCtxDb failed retVal: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    *value = saiVlanCtx->unregMulticastFloodType;

    return xpsStatus2SaiStatus(xpsRetVal);
}

//Func: xpSaiGetVlanAttrBroadcastFloodControlType
sai_status_t xpSaiGetVlanAttrBroadcastFloodControlType(xpsVlan_t xpsVlanId,
                                                       sai_int32_t* value)
{
    XP_STATUS               xpsRetVal   = XP_NO_ERR;
    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;

    if (!value)
    {
        XP_SAI_LOG_ERR("Invelid parameter: value\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsRetVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, xpsVlanId, &saiVlanCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVlanGetVlanCtxDb failed retVal: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    *value = saiVlanCtx->broadcastFloodType;

    return xpsStatus2SaiStatus(xpsRetVal);
}

sai_status_t xpSaiSetVlanBroadcastFloodControlGroup(sai_object_id_t vlanIdOid,
                                                    sai_object_id_t listOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsMcL2InterfaceListId_t listId = 0;
    xpsVlan_t xpsVlanId = xpSaiObjIdValueGet(vlanIdOid);

    if (listOid != SAI_NULL_OBJECT_ID)
    {
        retVal = xpSaiActivateVlanL2McGroup(xpsDevId, vlanIdOid, listOid,
                                            (uint32_t*) &listId);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiActivateVlanL2McGroup() failed. xpsVlanId = %d, listId = %d, retVal = %d \n",
                           xpsVlanId, listId, retVal);
            return retVal;
        }
    }
    else
    {
        listId = xpSaiGetDefaultL2McGroupId();
    }

    xpsRetVal = xpsVlanSetFloodL2InterfaceList(xpsDevId, xpsVlanId, listId);
    if (XP_NO_ERR != xpsRetVal)
    {
        xpSaiDeActivateVlanL2McGroup(xpsDevId, vlanIdOid, listOid);
        XP_SAI_LOG_ERR("xpsVlanSetFloodL2InterfaceList() failed. xpsVlanId = %d, listId = %d, retVal = %d \n",
                       xpsVlanId, listId, retVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return retVal;
}

sai_status_t xpSaiResetVlanBroadcastFloodControlGroup(sai_object_id_t vlanIdObj,
                                                      int32_t floodType)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsMcL2InterfaceListId_t listId = 0;
    sai_object_id_t listOid = SAI_NULL_OBJECT_ID;
    xpsVlan_t xpsVlanId = xpSaiObjIdValueGet(vlanIdObj);
    uint32_t defaultListId = xpSaiGetDefaultL2McGroupId();

    xpsRetVal = xpsVlanGetFloodL2InterfaceList(xpsDevId, xpsVlanId, &listId);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanGetFloodL2InterfaceList() failed xpsVlanId = %u, retVal = %d\n",
                       xpsVlanId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (defaultListId != listId &&
        floodType == SAI_VLAN_FLOOD_CONTROL_TYPE_L2MC_GROUP)
    {
        retVal = xpSaiGetL2McGroupIdOid(xpsDevId, listId, xpsVlanId, &listOid);
        if (SAI_STATUS_INVALID_PARAMETER == retVal)
        {
            /* L2 MC group is not set on this VLAN. Just return success */
            return SAI_STATUS_SUCCESS;
        }
        else if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("Couldn't get SAI L2MC GROUP OBJID for xpsVlanId = %d, listId = %u, retVal = %d.\n",
                           xpsVlanId, listId, retVal);
            return retVal;
        }

        retVal = xpSaiDeActivateVlanL2McGroup(xpsDevId, vlanIdObj, listOid);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiDeActivateVlanL2McGroup() failed. listOid = %d, retVal = %d \n",
                           listOid, retVal);
            return retVal;
        }
    }

    xpsRetVal = xpsVlanResetFloodL2InterfaceList(xpsDevId, xpsVlanId);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanResetFloodL2InterfaceList() failed. xpsVlanId = %d, retVal = %d \n",
                       xpsVlanId, retVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return retVal;
}

sai_status_t xpSaiSetVlanUnregMulicastFloodControlGroup(
    sai_object_id_t vlanIdOid, sai_object_id_t listOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsVlan_t xpsVlanId = xpSaiObjIdValueGet(vlanIdOid);
    xpsMcL2InterfaceListId_t listId = 0;
    xpsMcL2InterfaceListId_t oldListId = 0;

    if (listOid != SAI_NULL_OBJECT_ID)
    {
        /* Get current listId */
        xpsRetVal = xpsVlanGetIPv4UnregMcastL2InterfaceList(xpsDevId, xpsVlanId,
                                                            &oldListId);
        if (XP_NO_ERR != xpsRetVal)
        {
            XP_SAI_LOG_ERR("Could not get current L2MC list id: xpsVlanGetIPv4UnregMcastL2InterfaceList() failed. xpsDevId = %d, xpsVlanId = %d, retVal = %d \n",
                           xpsDevId, xpsVlanId, retVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }

        retVal = xpSaiActivateVlanL2McGroup(xpsDevId, vlanIdOid, listOid,
                                            (uint32_t*) &listId);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiActivateVlanL2McGroup() failed. xpsVlanId = %d, listId = %d, retVal = %d \n",
                           xpsVlanId, listId, retVal);
            return retVal;
        }
    }
    else
    {
        listId = xpSaiGetDefaultL2McGroupId();
    }

    xpsRetVal = xpsVlanSetIPv4UnregMcastL2InterfaceList(xpsDevId, xpsVlanId,
                                                        listId);
    if (XP_NO_ERR != xpsRetVal)
    {
        xpSaiDeActivateVlanL2McGroup(xpsDevId, vlanIdOid, listOid);
        XP_SAI_LOG_ERR("xpsVlanSetIPv4UnregMcastL2InterfaceList() failed. xpsVlanId = %d, listId = %d, retVal = %d \n",
                       xpsVlanId, listId, retVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    xpsRetVal = xpsVlanSetIPv6UnregMcastL2InterfaceList(xpsDevId, xpsVlanId,
                                                        listId);
    if (XP_NO_ERR != xpsRetVal)
    {
        /* Set previous value of IPv4 MC Interface list*/
        xpsVlanSetIPv4UnregMcastL2InterfaceList(xpsDevId, xpsVlanId, oldListId);
        xpSaiDeActivateVlanL2McGroup(xpsDevId, vlanIdOid, listOid);
        XP_SAI_LOG_ERR("xpsVlanSetIPv6UnregMcastL2InterfaceList() failed. xpsVlanId = %d, listId = %d, retVal = %d \n",
                       xpsVlanId, listId, retVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return retVal;
}

sai_status_t xpSaiResetVlanUnregMulicastFloodControlGroup(
    sai_object_id_t vlanIdObj, int32_t floodType)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsMcL2InterfaceListId_t listId = 0;
    sai_object_id_t listOid = SAI_NULL_OBJECT_ID;
    xpsVlan_t xpsVlanId = xpSaiObjIdValueGet(vlanIdObj);
    uint32_t defaultListId = xpSaiGetDefaultL2McGroupId();

    xpsRetVal = xpsVlanGetIPv4UnregMcastL2InterfaceList(xpsDevId, xpsVlanId,
                                                        &listId);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanGetFloodL2InterfaceList() failed xpsVlanId = %u, retVal = %d\n",
                       xpsVlanId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (defaultListId != listId &&
        floodType == SAI_VLAN_FLOOD_CONTROL_TYPE_L2MC_GROUP)
    {
        retVal = xpSaiGetL2McGroupIdOid(xpsDevId, listId, xpsVlanId, &listOid);
        if (SAI_STATUS_INVALID_PARAMETER == retVal)
        {
            /* L2 MC group is not set on this VLAN. Just return success */
            return SAI_STATUS_SUCCESS;
        }
        else if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("Couldn't get SAI L2MC GROUP OBJID for xpsVlanId = %d, listId = %u, retVal = %d.\n",
                           xpsVlanId, listId, retVal);
            return retVal;
        }

        retVal = xpSaiDeActivateVlanL2McGroup(xpsDevId, vlanIdObj, listOid);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiDeActivateVlanL2McGroup() failed. listOid = %d, retVal = %d \n",
                           listOid, retVal);
            return retVal;
        }
    }

    xpsRetVal = xpsVlanResetIPv4UnregMcastL2InterfaceList(xpsDevId, xpsVlanId);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanResetIPv4UnregMcastL2InterfaceList() failed. xpsVlanId = %d, listId = %d, retVal = %d \n",
                       xpsVlanId, listId, retVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    xpsRetVal = xpsVlanResetIPv6UnregMcastL2InterfaceList(xpsDevId, xpsVlanId);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanSetIPv6UnregMcastL2InterfaceList() failed. xpsVlanId = %d, listId = %d, retVal = %d \n",
                       xpsVlanId, listId, retVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return retVal;
}

//Func: xpSaiSetVlanAttribute
sai_status_t xpSaiSetVlanAttributes(sai_object_id_t vlan_id,
                                    const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsVlan_t xpsVlanId = xpSaiObjIdValueGet(vlan_id);
    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    retVal = xpSaiAttrCheck(1, attr,
                            VLAN_VALIDATION_ARRAY_SIZE, vlan_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    xpsRetVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, xpsVlanId, &saiVlanCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVlanGetVlanCtxDb failed retVal: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    switch (attr->id)
    {
        case SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES:
            {
                uint32_t fdbLimit = attr->value.u32;

                saiVlanCtx->maxLearnedAddresses = attr->value.u32;

                if (!fdbLimit)
                {
                    xpsRetVal = xpSaiGetFdbTableDepth(xpsDevId, &fdbLimit);
                    if (xpsRetVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("xpSaiGetFdbTableDepth failed devId %u | retVal : %d \n",
                                       xpsDevId, xpsRetVal);
                        return xpsStatus2SaiStatus(XP_ERR_NULL_POINTER);
                    }
                }

                /*Set the max learned address limit for the vlan*/
                xpsRetVal = xpsVlanSetFdbLimit(xpsDevId, xpsVlanId, fdbLimit);
                if (xpsRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsVlanSetFdbLimit failed vlanId %u | retVal : %d \n",
                                   xpsVlanId, xpsRetVal);
                    return xpsStatus2SaiStatus(xpsRetVal);
                }
                break;
            }

        case SAI_VLAN_ATTR_STP_INSTANCE:
            {
                xpsStp_t stgId = 0;

                if (!XDK_SAI_OBJID_TYPE_CHECK(attr->value.oid, SAI_OBJECT_TYPE_STP))
                {
                    XP_SAI_LOG_ERR("For wrong object type received(%u).\n",
                                   xpSaiObjIdTypeGet(attr->value.oid));
                    return SAI_STATUS_INVALID_ATTR_VALUE_0;
                }

                stgId = (xpsStp_t)xpSaiObjIdValueGet(attr->value.oid);

                xpsRetVal = xpsVlanBindStp(xpsDevId, xpsVlanId, stgId);

                if (XP_NO_ERR != xpsRetVal)
                {
                    XP_SAI_LOG_ERR("It is not possible to assosiate VLAN(%d) with STP(%u) instance.",
                                   xpsVlanId, stgId);
                    return  xpsStatus2SaiStatus(xpsRetVal);
                }

                break;
            }
        case SAI_VLAN_ATTR_LEARN_DISABLE:
            {
                xpsPktCmd_e saMissCmd = XP_PKTCMD_FWD_MIRROR;
                /*Disable learning based on config*/
                if (attr->value.booldata == true)
                {
                    saMissCmd = XP_PKTCMD_FWD;
                }

                xpsRetVal = xpsVlanSetUnknownSaCmd(xpsDevId, xpsVlanId, saMissCmd);
                if (XP_NO_ERR != xpsRetVal)
                {
                    XP_SAI_LOG_ERR("xpsVlanSetUnknownSaCmd failed vlanId %u | retVal : %d \n",
                                   xpsVlanId, xpsRetVal);
                    return  xpsStatus2SaiStatus(xpsRetVal);
                }

                break;
            }

        case SAI_VLAN_ATTR_INGRESS_ACL:
            {
                retVal = xpSaiSetVlanAttrAcl(vlan_id, attr->value, true);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_VLAN_ATTR_INGRESS_ACL)\n");
                    return retVal;
                }
                break;
            }
        case SAI_VLAN_ATTR_EGRESS_ACL:
            {
                retVal = xpSaiSetVlanAttrAcl(vlan_id, attr->value, false);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_VLAN_ATTR_EGRESS_ACL)\n");
                    return retVal;
                }
                break;
            }

        case SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
            {
                bool enableFlag = false;

                if (saiVlanCtx->unknownUnicastFloodType == attr->value.s32)
                {
                    return SAI_STATUS_SUCCESS;
                }

                /* Check if unknown unicast flood control to CPU port is enabled */
                retVal = xpSaiGetSwitchAttrBcastCpuFloodEnable(xpsDevId, &enableFlag);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiGetSwitchAttrBcastCpuFloodEnable() failed retVal : %d\n",
                                   retVal);
                    return retVal;
                }

                if (SAI_VLAN_FLOOD_CONTROL_TYPE_L2MC_GROUP == attr->value.s32)
                {
                    retVal = xpSaiSetVlanBroadcastFloodControlGroup(vlan_id, SAI_NULL_OBJECT_ID);
                    if (SAI_STATUS_SUCCESS != retVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiSetVlanBroadcastFloodControlGroup failed: xpsVlanId = %d, retVal = %d.\n",
                                       xpsVlanId, retVal);
                        return retVal;
                    }
                }
                else
                {
                    retVal = xpSaiResetVlanBroadcastFloodControlGroup(vlan_id,
                                                                      saiVlanCtx->unknownUnicastFloodType);
                    if (SAI_STATUS_SUCCESS != retVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiResetVlanBroadcastFloodControlGroup failed: xpsVlanId = %d, retVal = %d.\n",
                                       xpsVlanId, retVal);
                        return retVal;
                    }
                }

                retVal = xpSaiSetVlanAttrUnknownUnicastFloodControlType(vlan_id,
                                                                        attr->value.s32, enableFlag);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE)\n");
                    return retVal;
                }

                break;
            }

        case SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
            {
                if (saiVlanCtx->unknownUnicastFloodType ==
                    SAI_VLAN_FLOOD_CONTROL_TYPE_L2MC_GROUP)
                {
                    retVal = xpSaiSetVlanBroadcastFloodControlGroup(vlan_id, attr->value.oid);
                    if (SAI_STATUS_SUCCESS != retVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiSetVlanBroadcastFloodControlGroup failed: xpsVlanId = %d, retVal = %d.\n",
                                       xpsVlanId, retVal);
                        return retVal;
                    }
                }
                else
                {
                    XP_SAI_LOG_ERR("Unicast flood control type should be set to SAI_VLAN_FLOOD_CONTROL_TYPE_L2MC_GROUP\n");
                    return SAI_STATUS_INVALID_PARAMETER;
                }
                break;
            }

        case SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
            {
                bool enableFlag = false;

                if (saiVlanCtx->unregMulticastFloodType == attr->value.s32)
                {
                    return SAI_STATUS_SUCCESS;
                }

                /* Check if unknown multicast flood control to CPU port is enabled */
                retVal = xpSaiGetSwitchAttrMcastCpuFloodEnable(xpsDevId, &enableFlag);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiGetSwitchAttrMcastCpuFloodEnable() failed retVal : %d\n",
                                   retVal);
                    return retVal;
                }

                if (SAI_VLAN_FLOOD_CONTROL_TYPE_L2MC_GROUP == attr->value.s32)
                {
                    retVal = xpSaiSetVlanUnregMulicastFloodControlGroup(vlan_id,
                                                                        SAI_NULL_OBJECT_ID);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("xpSaiSetVlanUnregMulicastFloodControlGroup() failed. Group OID: %lu, retVal: %d\n",
                                       attr->value.oid, retVal);
                        return retVal;
                    }
                }
                else
                {
                    retVal = xpSaiResetVlanUnregMulicastFloodControlGroup(vlan_id,
                                                                          saiVlanCtx->unregMulticastFloodType);
                    if (SAI_STATUS_SUCCESS != retVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiResetVlanUnregMulicastFloodControlGroup failed: xpsVlanId = %d, retVal = %d.\n",
                                       xpsVlanId, retVal);
                        return retVal;
                    }
                }

                retVal = xpSaiSetVlanAttrUnknownMulticastFloodControlType(vlan_id,
                                                                          attr->value.s32, enableFlag);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE)\n");
                    return retVal;
                }

                break;
            }

        case SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
            {
                if (saiVlanCtx->unregMulticastFloodType ==
                    SAI_VLAN_FLOOD_CONTROL_TYPE_L2MC_GROUP)
                {
                    retVal = xpSaiSetVlanUnregMulicastFloodControlGroup(vlan_id, attr->value.oid);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("xpSaiSetVlanUnregMulicastFloodControlGroup() failed. Group OID: %lu, retVal: %d\n",
                                       attr->value.oid, retVal);
                        return retVal;
                    }
                }
                else
                {
                    XP_SAI_LOG_ERR("Multicast flood control type should be set to SAI_VLAN_FLOOD_CONTROL_TYPE_L2MC_GROUP\n");
                    return SAI_STATUS_INVALID_PARAMETER;
                }
                break;
            }

        case SAI_VLAN_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
            {
                bool enableFlag = false;

                if (saiVlanCtx->broadcastFloodType == attr->value.s32)
                {
                    return SAI_STATUS_SUCCESS;
                }

                /* Check if unknown broadcast flood control to CPU port is enabled */
                retVal = xpSaiGetSwitchAttrBcastCpuFloodEnable(xpsDevId, &enableFlag);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiGetSwitchAttrBcastCpuFloodEnable() failed retVal : %d\n",
                                   retVal);
                    return retVal;
                }

                if (SAI_VLAN_FLOOD_CONTROL_TYPE_L2MC_GROUP == attr->value.s32)
                {
                    retVal = xpSaiSetVlanBroadcastFloodControlGroup(vlan_id, SAI_NULL_OBJECT_ID);
                    if (SAI_STATUS_SUCCESS != retVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiSetVlanBroadcastFloodControlGroup failed: xpsVlanId = %d, retVal = %d.\n",
                                       xpsVlanId, retVal);
                        return retVal;
                    }
                }
                else
                {
                    retVal = xpSaiResetVlanBroadcastFloodControlGroup(vlan_id,
                                                                      saiVlanCtx->broadcastFloodType);
                    if (SAI_STATUS_SUCCESS != retVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiResetVlanBroadcastFloodControlGroup failed: xpsVlanId = %d, retVal = %d.\n",
                                       xpsVlanId, retVal);
                        return retVal;
                    }
                }

                retVal = xpSaiSetVlanAttrBroadcastFloodControlType(vlan_id, attr->value.s32,
                                                                   enableFlag);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_VLAN_ATTR_BROADCAST_FLOOD_CONTROL_TYPE)\n");
                    return retVal;
                }

                break;
            }

        case SAI_VLAN_ATTR_BROADCAST_FLOOD_GROUP:
            {
                if (saiVlanCtx->broadcastFloodType == SAI_VLAN_FLOOD_CONTROL_TYPE_L2MC_GROUP)
                {
                    retVal = xpSaiSetVlanBroadcastFloodControlGroup(vlan_id, attr->value.oid);
                    if (SAI_STATUS_SUCCESS != retVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiSetVlanBroadcastFloodControlGroup failed: xpsVlanId = %d, retVal = %d.\n",
                                       xpsVlanId, retVal);
                        return retVal;
                    }
                }
                else
                {
                    XP_SAI_LOG_ERR("Broadcast flood control type should be set to SAI_VLAN_FLOOD_CONTROL_TYPE_L2MC_GROUP\n");
                    return SAI_STATUS_INVALID_PARAMETER;
                }
                break;
            }

        case SAI_VLAN_ATTR_IPV4_MCAST_LOOKUP_KEY_TYPE:
            {
                xpsVlanBridgeMcMode_e vlanBrMcMode = MC_BRIDGE_MODE_MAX;
                retVal = xpSaiConvertSaiMcastLookupKeyType2xps((
                                                                   sai_vlan_mcast_lookup_key_type_t)attr->value.s32, &vlanBrMcMode);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("xpSaiConvertSaiMcastLookupKeyType2xps failed. sai_vlan_mcast_lookup_key = %d, retVal = %d \n",
                                   attr->value.s32, retVal);
                    return retVal;
                }

                retVal = xpsVlanSetIpv4McBridgeMode(xpsDevId, xpsVlanId, vlanBrMcMode);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("xpsVlanSetIpv4McBridgeMode failed. sai_vlan_mcast_lookup_key = %d, xpsVlanId = %d, retVal = %d \n",
                                   attr->value.s32, xpsVlanId, retVal);
                    return retVal;
                }
                break;
            }

        case SAI_VLAN_ATTR_IPV6_MCAST_LOOKUP_KEY_TYPE:
            {
                xpsVlanBridgeMcMode_e vlanBrMcMode = MC_BRIDGE_MODE_MAX;
                retVal = xpSaiConvertSaiMcastLookupKeyType2xps((
                                                                   sai_vlan_mcast_lookup_key_type_t)attr->value.s32, &vlanBrMcMode);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("xpSaiConvertSaiMcastLookupKeyType2xps failed. sai_vlan_mcast_lookup_key = %d, retVal = %d \n",
                                   attr->value.s32, retVal);
                    return retVal;
                }

                retVal = xpsVlanSetIpv6McBridgeMode(xpsDevId, xpsVlanId, vlanBrMcMode);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("xpsVlanSetIpv6McBridgeMode failed. sai_vlan_mcast_lookup_key = %d, xpsVlanId = %d, retVal = %d \n",
                                   attr->value.s32, xpsVlanId, retVal);
                    return retVal;
                }
                break;
            }
        case SAI_VLAN_ATTR_CUSTOM_IGMP_SNOOPING_ENABLE:
            {
                retVal = xpSaiSetVlanAttrIgmpSnoopingEnable(xpsVlanId, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("xpSaiSetVlanAttrIgmpSnoopingEnable failed vlanId %u | retVal: %d \n",
                                   xpsVlanId, retVal);
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

//Func: xpSaiGetVlanAttrVlanId

sai_status_t xpSaiGetVlanAttrVlanId(sai_vlan_id_t vlanId,
                                    sai_attribute_value_t* value)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;

    //Fetch from SAI DB
    retVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, vlanId, &saiVlanCtx);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Get sai vlan context failed, vlanId(%d)", vlanId);
        return xpsStatus2SaiStatus(retVal);
    }

    value->u16 = vlanId;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    return xpsStatus2SaiStatus(retVal);
}


//Func: xpSaiGetVlanAttrPortList

sai_status_t xpSaiGetVlanAttrPortList(sai_vlan_id_t vlanId,
                                      sai_attribute_value_t* value)
{
    XP_STATUS         xpsRetVal  = XP_NO_ERR;
    xpsDevice_t       xpsDevId   = xpSaiGetDevId();
    xpsInterfaceId_t  *intfList  = NULL;
    uint16_t          numOfIntfs = 0;
    sai_status_t      saiStatus  = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    xpsRetVal = xpsVlanGetInterfaceList(xpsDevId, vlanId, &intfList, &numOfIntfs);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("Do not possible to retrive port list for vlanId %u | retVal : %d \n",
                       vlanId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    XP_SAI_LOG_DBG("Vlan %u contains %u ports\n", vlanId, numOfIntfs);

    if (value->objlist.count < numOfIntfs)
    {
        XP_SAI_LOG_ERR("Not enough room for %u interfaces!\n", numOfIntfs);
        value->objlist.count = numOfIntfs;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if ((numOfIntfs != 0) && (value->objlist.list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameters received");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    value->objlist.count = 0;

    for (uint16_t portIndx = 0; portIndx < numOfIntfs; portIndx++)
    {
        saiStatus = xpSaiVlanMemberObjIdCreate(xpsDevId, vlanId, intfList[portIndx],
                                               &value->objlist.list[value->objlist.count]);
        if (SAI_STATUS_SUCCESS != saiStatus)
        {
            XP_SAI_LOG_ERR("Vlan member object could not be created, port %u\n",
                           intfList[portIndx]);
            return saiStatus;
        }

        value->objlist.count++;
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetVlanAttribute

sai_status_t xpSaiGetVlanAttrMaxLearnedAddresses(sai_vlan_id_t vlanId,
                                                 sai_attribute_value_t* value)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;

    //Fetch from SAI DB
    retVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, vlanId, &saiVlanCtx);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Insert sai vlan context failed, vlanId(%d)", vlanId);
        return xpsStatus2SaiStatus(retVal);
    }

    value->u32 = saiVlanCtx->maxLearnedAddresses;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetVlanAttributeStpInstance(sai_vlan_id_t vlanId,
                                              sai_attribute_value_t* value)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    xpsDevice_t     xpsDevId    = xpSaiGetDevId();
    xpsStp_t        stpId;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    retVal = xpsVlanGetStp(xpsDevId, vlanId, &stpId);

    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanGetStp failed vlanId %u | retVal : %d \n", vlanId,
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_STP, xpsDevId, (sai_uint64_t) stpId,
                         &value->oid) != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetVlanAttrLearnDisable(sai_vlan_id_t vlanId,
                                          sai_attribute_value_t* value)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsPktCmd_e saMissCmd = XP_PKTCMD_FWD_MIRROR;

    xpsRetVal = xpsVlanGetUnknownSaCmd(xpsDevId, vlanId, &saMissCmd);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanGetUnknownSaCmd failed vlanId %u | retVal : %d \n",
                       vlanId, xpsRetVal);
        return  xpsStatus2SaiStatus(xpsRetVal);
    }

    value->booldata = (saMissCmd == XP_PKTCMD_FWD);

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    return  SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiGetVlanFloodGroup(xpsDevice_t devId,
                                           sai_vlan_id_t vlanId, sai_attribute_t* attr, uint32_t listId, int32_t floodType)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    sai_object_id_t defaultGroupId = xpSaiGetDefaultL2McGroupId();

    if (floodType != SAI_VLAN_FLOOD_CONTROL_TYPE_L2MC_GROUP ||
        defaultGroupId == listId)
    {
        attr->value.oid = SAI_NULL_OBJECT_ID;
    }
    else
    {
        retVal = xpSaiGetL2McGroupIdOid(devId, listId, vlanId, &attr->value.oid);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("Couldn't get SAI L2MC GROUP OBJID for vlanId = %d, listId = %u, floodType = %d, retVal = %d.\n",
                           vlanId, listId, floodType, retVal);
            return retVal;
        }
    }

    return retVal;
}

//Func: xpSaiGetVlanAttribute

static sai_status_t xpSaiGetVlanAttribute(sai_vlan_id_t vlanId,
                                          sai_attribute_t* attr, uint32_t attr_index)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t devId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    switch (attr->id)
    {
        case SAI_VLAN_ATTR_VLAN_ID:
            {
                retVal = xpSaiGetVlanAttrVlanId(vlanId, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_VLAN_ATTR_VLAN_ID)\n");
                    return retVal;
                }

                break;
            }
        case SAI_VLAN_ATTR_MEMBER_LIST:
            {
                retVal = xpSaiGetVlanAttrPortList(vlanId, &attr->value);

                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_VLAN_ATTR_MEMBER_LIST)\n");
                    return retVal;
                }

                break;
            }
        case SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES:
            {
                retVal = xpSaiGetVlanAttrMaxLearnedAddresses(vlanId, &attr->value);

                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES)\n");
                    return retVal;
                }

                break;
            }
        case SAI_VLAN_ATTR_STP_INSTANCE:
            {
                retVal = xpSaiGetVlanAttributeStpInstance(vlanId, &attr->value);

                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_VLAN_ATTR_STP_INSTANCE)\n");
                    return retVal;
                }

                break;
            }
        case SAI_VLAN_ATTR_LEARN_DISABLE:
            {
                retVal = xpSaiGetVlanAttrLearnDisable(vlanId, &attr->value);

                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_VLAN_ATTR_LEARN_DISABLE)\n");
                    return retVal;
                }

                break;
            }
        case SAI_VLAN_ATTR_INGRESS_ACL:
            {
                retVal = xpSaiGetVlanAttrAcl(devId, vlanId, &attr->value, true);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_VLAN_ATTR_INGRESS_ACL)\n");
                    return retVal;
                }
                break;
            }
        case SAI_VLAN_ATTR_EGRESS_ACL:
            {
                retVal = xpSaiGetVlanAttrAcl(devId, vlanId, &attr->value, false);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_VLAN_ATTR_EGRESS_ACL)\n");
                    return retVal;
                }
                break;
            }
        case SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
            {
                retVal = xpSaiGetVlanAttrUnknownUnicastFloodControlType(vlanId,
                                                                        &attr->value.s32);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiGetVlanAttrUnknownUnicastFloodControlType failed retVal : %d\n",
                                   retVal);
                    return retVal;
                }
                break;
            }

        case SAI_VLAN_ATTR_BROADCAST_FLOOD_GROUP:
            {
                xpsMcL2InterfaceListId_t listId = 0;
                int32_t floodType;

                xpsRetVal = xpsVlanGetFloodL2InterfaceList(devId, (xpsVlan_t)vlanId, &listId);
                if (XP_NO_ERR != xpsRetVal)
                {
                    XP_SAI_LOG_ERR("xpsVlanGetFloodL2InterfaceList() failed vlanId = %u, retVal = %d\n",
                                   vlanId, xpsRetVal);
                    return xpsStatus2SaiStatus(xpsRetVal);
                }

                retVal = xpSaiGetVlanAttrBroadcastFloodControlType(vlanId, &floodType);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiGetVlanAttrBroadcastFloodControlType failed vlanId = %u, retVal : %d\n",
                                   vlanId, retVal);
                    return retVal;
                }

                retVal = xpSaiGetVlanFloodGroup(devId, vlanId, attr, listId, floodType);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiGetVlanFloodGroup failed vlanId = %u, listId = %u, floodType = %u, retVal : %d\n",
                                   vlanId, listId, floodType, retVal);
                    return retVal;
                }
                break;
            }
        case SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
            {
                xpsMcL2InterfaceListId_t listId = 0;
                int32_t floodType;

                xpsRetVal = xpsVlanGetFloodL2InterfaceList(devId, (xpsVlan_t)vlanId, &listId);
                if (XP_NO_ERR != xpsRetVal)
                {
                    XP_SAI_LOG_ERR("xpsVlanGetFloodL2InterfaceList() failed vlanId = %u, retVal = %d\n",
                                   vlanId, xpsRetVal);
                    return xpsStatus2SaiStatus(xpsRetVal);
                }

                retVal = xpSaiGetVlanAttrUnknownUnicastFloodControlType(vlanId, &floodType);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiGetVlanAttrBroadcastFloodControlType failed vlanId = %u, retVal : %d\n",
                                   vlanId, retVal);
                    return retVal;
                }

                retVal = xpSaiGetVlanFloodGroup(devId, vlanId, attr, listId, floodType);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiGetVlanFloodGroup failed vlanId = %u, listId = %u, floodType = %u, retVal : %d\n",
                                   vlanId, listId, floodType, retVal);
                    return retVal;
                }

                break;
            }

        case SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
            {
                retVal = xpSaiGetVlanAttrUnknownMulticastFloodControlType(vlanId,
                                                                          &attr->value.s32);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiGetVlanAttrUnknownMulticastFloodControlType failed retVal : %d\n",
                                   retVal);
                    return retVal;
                }
                break;
            }

        case SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
            {
                xpsMcL2InterfaceListId_t listId = 0;
                int32_t floodType;

                xpsRetVal = xpsVlanGetIPv4UnregMcastL2InterfaceList(devId, vlanId, &listId);
                if (XP_NO_ERR != xpsRetVal)
                {
                    XP_SAI_LOG_ERR("xpsVlanGetIPv4UnregMcastL2InterfaceList() failed vlanId = %u, retVal = %d\n",
                                   vlanId, xpsRetVal);
                    return xpsStatus2SaiStatus(xpsRetVal);
                }

                retVal = xpSaiGetVlanAttrUnknownMulticastFloodControlType(vlanId, &floodType);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiGetVlanAttrUnknownMulticastFloodControlType failed vlanId = %u, retVal : %d\n",
                                   vlanId, retVal);
                    return retVal;
                }

                retVal = xpSaiGetVlanFloodGroup(devId, vlanId, attr, listId, floodType);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiGetVlanFloodGroup failed vlanId = %u, listId = %u, floodType = %u, retVal : %d\n",
                                   vlanId, listId, floodType, retVal);
                    return retVal;
                }
                break;
            }

        case SAI_VLAN_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
            {
                retVal = xpSaiGetVlanAttrBroadcastFloodControlType(vlanId, &attr->value.s32);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiGetVlanAttrBroadcastFloodControlType failed retVal : %d\n",
                                   retVal);
                    return retVal;
                }
                break;
            }

        case SAI_VLAN_ATTR_IPV4_MCAST_LOOKUP_KEY_TYPE:
            {
                xpsVlanBridgeMcMode_e vlanBrMcMode = MC_BRIDGE_MODE_MAX;
                xpsRetVal = xpsVlanGetIpv4McBridgeMode(devId, vlanId, &vlanBrMcMode);
                if (xpsRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsVlanGetIpv4McBridgeMode failed retVal : %d\n", retVal);
                    return retVal;
                }

                retVal = xpSaiConvertXpsMcastLookupKeyType2sai(vlanBrMcMode,
                                                               (sai_vlan_mcast_lookup_key_type_t *) &attr->value.s32);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Couldn't convert Vlan Mcast Lookup Key for vlanId = %d, retVal = %d.\n",
                                   vlanId, retVal);
                    return retVal;
                }
                break;
            }

        case SAI_VLAN_ATTR_IPV6_MCAST_LOOKUP_KEY_TYPE:
            {
                xpsVlanBridgeMcMode_e vlanBrMcMode = MC_BRIDGE_MODE_MAX;
                xpsRetVal = xpsVlanGetIpv6McBridgeMode(devId, vlanId, &vlanBrMcMode);
                if (xpsRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsVlanGetIpv6McBridgeMode failed retVal : %d\n", retVal);
                    return retVal;
                }

                retVal = xpSaiConvertXpsMcastLookupKeyType2sai(vlanBrMcMode,
                                                               (sai_vlan_mcast_lookup_key_type_t *) &attr->value.s32);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Couldn't convert Vlan Mcast Lookup Key for vlanId = %d, retVal = %d.\n",
                                   vlanId, retVal);
                    return retVal;
                }
                break;
            }

        case SAI_VLAN_ATTR_CUSTOM_IGMP_SNOOPING_ENABLE:
            {
                retVal = xpSaiGetVlanAttrIgmpSnoopingEnable(vlanId, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("xpSaiGetVlanAttrIgmpSnoopingEnable failed vlanId %u | retVal : %d \n",
                                   vlanId, retVal);
                    return  retVal;
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

//Func: xpSaiGetVlanAttributes

static sai_status_t xpSaiGetVlanAttributes(sai_object_id_t vlan_id_obj,
                                           uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    sai_vlan_id_t vlanId = xpSaiObjIdValueGet(vlan_id_obj);

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            VLAN_VALIDATION_ARRAY_SIZE, vlan_attribs,
                            SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        retVal = xpSaiGetVlanAttribute(vlanId, &attr_list[count], count);

        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetVlanAttribute failed\n");
            return retVal;
        }
    }

    return retVal;
}

//Func: xpSaiBulkGetVlanAttributes

sai_status_t xpSaiBulkGetVlanAttributes(sai_object_id_t id,
                                        uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;
    sai_vlan_id_t    vlanId     = xpSaiObjIdValueGet(id);

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_VLAN))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountVlanAttribute(&maxcount);
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
        attr_list[idx].id = SAI_VLAN_ATTR_START + count;
        saiRetVal = xpSaiGetVlanAttribute(vlanId, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveAllVlans

sai_status_t xpSaiRemoveAllVlans(void)
{

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    return xpsStatus2SaiStatus(xpsVlanDeleteAll(xpSaiGetDevId()));
}

//Func: xpSaiConvertTaggingMode

xpsL2EncapType_e xpSaiConvertTaggingMode(sai_vlan_tagging_mode_t
                                         sai_tagging_mode)
{
    switch (sai_tagging_mode)
    {
        case SAI_VLAN_TAGGING_MODE_UNTAGGED:
            {
                return XP_L2_ENCAP_DOT1Q_UNTAGGED;
            }

        case SAI_VLAN_TAGGING_MODE_TAGGED:
            {
                return XP_L2_ENCAP_DOT1Q_TAGGED;
            }

        case SAI_VLAN_TAGGING_MODE_PRIORITY_TAGGED:
            {
                return XP_L2_ENCAP_DOT1Q_PRIOTAGGED;
            }

        default:
            {
                XP_SAI_LOG_ERR("Unknown tagging mode %d\n", sai_tagging_mode);
                return XP_L2_ENCAP_INVALID;
            }
    }
}

//Func: xpSaiConvertTaggingMode

sai_vlan_tagging_mode_t xpSaiConvertTaggingModeToSai(xpsL2EncapType_e
                                                     tagging_mode)
{
    switch (tagging_mode)
    {
        case XP_L2_ENCAP_DOT1Q_UNTAGGED:
            {
                return SAI_VLAN_TAGGING_MODE_UNTAGGED;
            }
        case XP_L2_ENCAP_DOT1Q_TAGGED:
            {
                return SAI_VLAN_TAGGING_MODE_TAGGED;
            }
        case XP_L2_ENCAP_DOT1Q_PRIOTAGGED:
            {
                return SAI_VLAN_TAGGING_MODE_PRIORITY_TAGGED;
            }
        default:
            {
                XP_SAI_LOG_ERR("SAI unknown tagging mode %d received.\n", tagging_mode);
                return SAI_VLAN_TAGGING_MODE_UNTAGGED;
            }
    }
}

//Func: xpSaiConvertRspanTagging

xpsL2EncapType_e xpSaiConvertRspanTagging(xpsL2EncapType_e tagging_mode)
{
    switch (tagging_mode)
    {
        case XP_L2_ENCAP_DOT1Q_UNTAGGED:
            {
                return XP_L2_ENCAP_QINQ_CTAGGED;
            }

        case XP_L2_ENCAP_DOT1Q_TAGGED:
            {
                return XP_L2_ENCAP_QINQ_STAGGED;
            }

        case XP_L2_ENCAP_DOT1Q_PRIOTAGGED:
            {
                return XP_L2_ENCAP_QINQ_STAGGED;
            }

        default:
            {
                XP_SAI_LOG_ERR("Unknown tagging mode %d\n", tagging_mode);
                return XP_L2_ENCAP_INVALID;
            }
    }
}

//Func: xpSaiRevertRspanTagging

xpsL2EncapType_e xpSaiRevertRspanTagging(xpsL2EncapType_e tagging_mode)
{
    switch (tagging_mode)
    {
        case XP_L2_ENCAP_QINQ_CTAGGED:
            {
                return XP_L2_ENCAP_DOT1Q_UNTAGGED;
            }
        case XP_L2_ENCAP_QINQ_STAGGED:
            {
                return XP_L2_ENCAP_DOT1Q_TAGGED;
            }
        default:
            {
                XP_SAI_LOG_ERR("SAI unknown tagging mode %d received.\n", tagging_mode);
                return XP_L2_ENCAP_INVALID;
            }
    }
}

const char* xpSaiVlanStatAttrString[] =
{
    "SAI_VLAN_STAT_IN_OCTETS",
    "SAI_VLAN_STAT_IN_PACKETS",
    "SAI_VLAN_STAT_IN_UCAST_PKTS",
    "SAI_VLAN_STAT_IN_NON_UCAST_PKTS",
    "SAI_VLAN_STAT_IN_DISCARDS",
    "SAI_VLAN_STAT_IN_ERRORS",
    "SAI_VLAN_STAT_IN_UNKNOWN_PROTOS",
    "SAI_VLAN_STAT_OUT_OCTETS",
    "SAI_VLAN_STAT_OUT_PACKETS",
    "SAI_VLAN_STAT_OUT_UCAST_PKTS",
    "SAI_VLAN_STAT_OUT_NON_UCAST_PKTS",
    "SAI_VLAN_STAT_OUT_DISCARDS",
    "SAI_VLAN_STAT_OUT_ERRORS",
    "SAI_VLAN_STAT_OUT_QLEN"
};


//Func: xpSaiGetVlanStats

sai_status_t xpSaiGetVlanStats(sai_object_id_t  vlan_id,
                               uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                               uint64_t *counters)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t devId = xpSaiGetDevId();
    xpsVlan_t xpsVlanId = xpSaiObjIdValueGet(vlan_id);
    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;
    uint32_t egrCounterId = 0;
    uint64_t nPkts = 0;
    uint64_t nBytes = 0;

    xpsRetVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, xpsVlanId, &saiVlanCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVlanGetVlanCtxDb failed retVal : %d \n", xpsRetVal);
        return xpsRetVal;
    }

    egrCounterId = saiVlanCtx->egrCounterId;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    /*Read the ingress counters. Hw counters are cleared on read. Hence sw counters incremented*/
    xpsRetVal = xpsAcmGetCounterValue(devId, XP_ACM_ING_BD_COUNTER, xpsVlanId,
                                      &nPkts, &nBytes);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAcmGetCounterValue failed retVal : %d \n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    saiVlanCtx->ingPkts += nPkts;
    saiVlanCtx->ingOctets += nBytes;

    /*Read the egress counters*/
    xpsRetVal = xpsAcmGetCounterValue(devId, XP_ACM_EGR_BD_COUNTER, egrCounterId,
                                      &nPkts, &nBytes);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAcmGetCounterValue failed retVal : %d \n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    saiVlanCtx->egrPkts += nPkts;
    saiVlanCtx->egrOctets += nBytes;

    for (uint32_t index = 0; index < number_of_counters; index++)
    {
        switch (counter_ids[index])
        {
            case SAI_VLAN_STAT_IN_OCTETS:
                {
                    counters[index] = saiVlanCtx->ingOctets;
                    break;
                }
            case SAI_VLAN_STAT_IN_PACKETS:
                {
                    counters[index] = saiVlanCtx->ingPkts;
                    break;
                }
            case SAI_VLAN_STAT_OUT_OCTETS:
                {
                    counters[index] = saiVlanCtx->egrOctets;
                    break;
                }
            case SAI_VLAN_STAT_OUT_PACKETS:
                {
                    counters[index] = saiVlanCtx->egrPkts;
                    break;
                }
            case SAI_VLAN_STAT_IN_UCAST_PKTS:
            case SAI_VLAN_STAT_IN_NON_UCAST_PKTS:
            case SAI_VLAN_STAT_IN_DISCARDS:
            case SAI_VLAN_STAT_IN_ERRORS:
            case SAI_VLAN_STAT_IN_UNKNOWN_PROTOS:
            case SAI_VLAN_STAT_OUT_UCAST_PKTS:
            case SAI_VLAN_STAT_OUT_NON_UCAST_PKTS:
            case SAI_VLAN_STAT_OUT_DISCARDS:
            case SAI_VLAN_STAT_OUT_ERRORS:
            case SAI_VLAN_STAT_OUT_QLEN:
                {
                    XP_SAI_LOG_DBG("Unsupported attribute %s\n",
                                   xpSaiVlanStatAttrString[counter_ids[index]]);
                    return SAI_STATUS_NOT_SUPPORTED;
                }
            default:
                {
                    XP_SAI_LOG_DBG("Invalid attribute\n");
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }

    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiClearVlanStats(_In_  sai_object_id_t  vlan_id,
                                 _In_ uint32_t number_of_counters, _In_ const sai_stat_id_t *counter_ids)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t devId = xpSaiGetDevId();
    xpsVlan_t xpsVlanId = xpSaiObjIdValueGet(vlan_id);
    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;
    uint32_t egrCounterId = 0;
    uint64_t nPkts = 0;
    uint64_t nBytes = 0;

    xpsRetVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, xpsVlanId, &saiVlanCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVlanGetVlanCtxDb failed retVal : %d \n", xpsRetVal);
        return xpsRetVal;
    }

    egrCounterId = saiVlanCtx->egrCounterId;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    /*Read the ingress counters. Hw counters are cleared on read. Hence explicit clear of hw counters not needed
    The state counter values are reset to 0*/
    xpsRetVal = xpsAcmGetCounterValue(devId, XP_ACM_ING_BD_COUNTER, xpsVlanId,
                                      &nPkts, &nBytes);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAcmGetCounterValue failed retVal : %d \n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /*Read and clear the egress counters*/
    xpsRetVal = xpsAcmGetCounterValue(devId, XP_ACM_EGR_BD_COUNTER, egrCounterId,
                                      &nPkts, &nBytes);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAcmGetCounterValue failed retVal : %d \n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    for (uint32_t index = 0; index < number_of_counters; index++)
    {
        switch (counter_ids[index])
        {
            case SAI_VLAN_STAT_IN_OCTETS:
                {
                    saiVlanCtx->ingOctets = 0;
                    break;
                }
            case SAI_VLAN_STAT_IN_PACKETS:
                {
                    saiVlanCtx->ingPkts = 0;
                    break;
                }
            case SAI_VLAN_STAT_OUT_OCTETS:
                {
                    saiVlanCtx->egrOctets = 0;
                    break;
                }
            case SAI_VLAN_STAT_OUT_PACKETS:
                {
                    saiVlanCtx->egrPkts = 0;
                    break;
                }
            case SAI_VLAN_STAT_IN_UCAST_PKTS:
            case SAI_VLAN_STAT_IN_NON_UCAST_PKTS:
            case SAI_VLAN_STAT_IN_DISCARDS:
            case SAI_VLAN_STAT_IN_ERRORS:
            case SAI_VLAN_STAT_IN_UNKNOWN_PROTOS:
            case SAI_VLAN_STAT_OUT_UCAST_PKTS:
            case SAI_VLAN_STAT_OUT_NON_UCAST_PKTS:
            case SAI_VLAN_STAT_OUT_DISCARDS:
            case SAI_VLAN_STAT_OUT_ERRORS:
            case SAI_VLAN_STAT_OUT_QLEN:
                {
                    XP_SAI_LOG_DBG("Unsupported attribute %s\n",
                                   xpSaiVlanStatAttrString[counter_ids[index]]);
                    return SAI_STATUS_NOT_SUPPORTED;
                }
            default:
                {
                    XP_SAI_LOG_DBG("Invalid attribute\n");
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }

    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t xpSaiCreateVlanMember(_Out_ sai_object_id_t* vlan_member_id,
                                   _In_ sai_object_id_t switch_id,
                                   _In_ uint32_t attr_count, _In_ const sai_attribute_t *attr_list)
{
    sai_status_t            retVal          = SAI_STATUS_SUCCESS;
    XP_STATUS               xpsRetVal       = XP_NO_ERR;
    sai_object_id_t         saiVlanOid      = 0;
    uint16_t                vlanId          = 0;
    xpsL2EncapType_e        tagType         = XP_L2_ENCAP_INVALID;
    sai_vlan_tagging_mode_t saiTagType      = SAI_VLAN_TAGGING_MODE_UNTAGGED;
    sai_object_id_t         saiPortId       = SAI_NULL_OBJECT_ID;
    sai_object_id_t         saiBrPortId     = SAI_NULL_OBJECT_ID;
    sai_object_id_t         lagOid          = SAI_NULL_OBJECT_ID;
    xpsInterfaceId_t        xpsIntf         = 0;
    xpsDevice_t             xpsDevId = xpSaiGetDevId();
    xpSaiPortLagInfo_t*      pSaiPortLagInfoEntry = NULL;
    xpSaiLagPortCountInfo_t* pSaiLagPortCountInfoEntry = NULL;
    xpSaiBridgePort_t        bridgePort;
    uint32_t                 iVif = 0;
    sai_attribute_value_t    pvidValue;
    xpsPortList_t            pList;
    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    memset(&pList, 0, sizeof(xpsPortList_t));

    if ((NULL == vlan_member_id) || (NULL == attr_list))
    {
        XP_SAI_LOG_ERR("Invalid parameter received!");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            VLAN_MEMBER_VALIDATION_ARRAY_SIZE, vlan_member_attribs,
                            SAI_COMMON_API_CREATE);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    /* Check input conditions.*/
    if (attr_count < 2)
    {
        XP_SAI_LOG_ERR("Wrong number of input attributes received. At least SAI_VLAN_MEMBER_ATTR_VLAN_ID and SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID are expected.");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (sai_uint32_t indx = 0; indx < attr_count; indx++)
    {
        switch (attr_list[indx].id)
        {
            case SAI_VLAN_MEMBER_ATTR_VLAN_ID:
                saiVlanOid = attr_list[indx].value.oid;
                break;
            case SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID:
                saiBrPortId = attr_list[indx].value.oid;
                break;
            case SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE:
                saiTagType = (sai_vlan_tagging_mode_t)attr_list[indx].value.u32;
                break;
            default:
                XP_SAI_LOG_ERR("Invalid attribute received(%u)!", attr_list[indx].id);
                break;
        }
    }
    tagType = xpSaiConvertTaggingMode(saiTagType);

    memset(&bridgePort, 0, sizeof(xpSaiBridgePort_t));

    xpsRetVal = xpSaiGetBridgePortById(saiBrPortId, &bridgePort);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetBridgePortById failed vlanId %u | bridgePort %lu | xpsRetVal %d \n",
                       vlanId, saiBrPortId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    saiPortId = bridgePort.brPortDescr.objId;

    if (!(XDK_SAI_OBJID_TYPE_CHECK(saiPortId, SAI_OBJECT_TYPE_PORT) ||
          XDK_SAI_OBJID_TYPE_CHECK(saiPortId, SAI_OBJECT_TYPE_LAG) ||
          XDK_SAI_OBJID_TYPE_CHECK(saiPortId, SAI_OBJECT_TYPE_TUNNEL)))
    {
        XP_SAI_LOG_ERR("For wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(saiPortId));
        retVal = SAI_STATUS_INVALID_ATTR_VALUE_0;
        return retVal;
    }

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
    }

    if (!(XDK_SAI_OBJID_TYPE_CHECK(saiVlanOid, SAI_OBJECT_TYPE_VLAN)))
    {
        XP_SAI_LOG_ERR("For wrong object type received(%u).",
                       xpSaiObjIdTypeGet(saiVlanOid));
        retVal = SAI_STATUS_INVALID_ATTR_VALUE_0;
        return retVal;
    }

    vlanId = (uint16_t)xpSaiObjIdValueGet(saiVlanOid);

    XP_SAI_LOG_DBG("Input values vlanId %u, xpsIntf %d, saiTagType %d, tagType %d.\n",
                   vlanId, xpsIntf, saiTagType, tagType);

    if (XDK_SAI_OBJID_TYPE_CHECK(saiPortId, SAI_OBJECT_TYPE_PORT))
    {
        xpsRetVal = xpsPortGetField(xpsDevId, xpsIntf, XPS_PORT_INGRESS_VIF, &iVif);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsPortGetField failed. port %d | retVal : %d \n", xpsIntf,
                           xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }
    else if (XDK_SAI_OBJID_TYPE_CHECK(saiPortId, SAI_OBJECT_TYPE_LAG) ||
             (XDK_SAI_OBJID_TYPE_CHECK(saiPortId, SAI_OBJECT_TYPE_TUNNEL)))
    {
        iVif = xpsIntf;
    }

    xpsRetVal = xpsVlanInterfaceDefaultIngressStpSet(xpsDevId, iVif, vlanId);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanInterfaceDefaultIngressStpSet failed vlanId %u | iVif %u | xpsRetVal %d \n",
                       vlanId, iVif, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /*This has been added to model behaviour added as part of LAG membership model,
      where each some attributes of port in the LAG may be configured separately(VLAN assagned, speed, MTU etc...).
      It is assumed SAI client application will take care of consistency.*/
    pSaiLagPortCountInfoEntry = NULL;
    retVal = xpSaiPortLagInfoGet(xpsIntf, &pSaiPortLagInfoEntry);
    if ((retVal == SAI_STATUS_SUCCESS) && (NULL != pSaiPortLagInfoEntry))
    {
        /*Calculates how many ports from the same LAG has been added to same VLAN. Needed to feet new SAI LAG member model.*/
        if (xpSaiLagPortCountInfoGet(pSaiPortLagInfoEntry->lagId, vlanId,
                                     &pSaiLagPortCountInfoEntry) != SAI_STATUS_SUCCESS)
        {
            if (pSaiLagPortCountInfoEntry == NULL)
            {
                /*In case port is LAG is not in DB yet insert an entry.*/
                xpsRetVal = xpsStateHeapMalloc(sizeof(xpSaiLagPortCountInfo_t),
                                               (void**)&pSaiLagPortCountInfoEntry);
                if (xpsRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Could not allocate channel structure\n");
                    return xpsStatus2SaiStatus(xpsRetVal);
                }

                memset(pSaiLagPortCountInfoEntry, 0, sizeof(xpSaiLagPortCountInfo_t));

                pSaiLagPortCountInfoEntry->lagId = pSaiPortLagInfoEntry->lagId;
                pSaiLagPortCountInfoEntry->vlanId = vlanId;
                pSaiLagPortCountInfoEntry->membersPortCount = 1;

                xpsRetVal = xpsStateInsertData(XP_SCOPE_DEFAULT, portLagPortCountDbHandle,
                                               (void*)pSaiLagPortCountInfoEntry);
                if (xpsRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Could not insert a channel structure into DB, error %d\n",
                                   xpsRetVal);
                    xpsStateHeapFree((void*)pSaiLagPortCountInfoEntry);
                    return xpsStatus2SaiStatus(xpsRetVal);
                }
            }

            XP_SAI_LOG_DBG("LAG=%u, count=%d !\n", pSaiPortLagInfoEntry->lagId,
                           pSaiLagPortCountInfoEntry->membersPortCount);
        }
        else /*Increase the counter.*/
        {
            if (pSaiLagPortCountInfoEntry != NULL)
            {
                pSaiLagPortCountInfoEntry->membersPortCount++;

                XP_SAI_LOG_DBG("LAG=%u, count=%d !\n", pSaiPortLagInfoEntry->lagId,
                               pSaiLagPortCountInfoEntry->membersPortCount);
            }
        }

        // Fetch from SAI DB
        xpsRetVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, vlanId, &saiVlanCtx);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Insert sai vlan context failed, vlanId(%d)", vlanId);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
        if (saiVlanCtx->isRspanVlan == true)
        {
            tagType = xpSaiConvertRspanTagging(tagType);
            if (xpsRetVal != XP_NO_ERR)
            {
                return xpsStatus2SaiStatus(xpsRetVal);
            }
        }
        /*In case port is part of LAG configure VLAN on LAG.*/
        xpsRetVal = (XP_STATUS) xpsVlanAddInterface(xpsDevId, vlanId,
                                                    pSaiPortLagInfoEntry->lagId, tagType);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsVlanAddInterface failed vlanId %u | port_id %u | xpsRetVal %d \n",
                           vlanId, xpsIntf, xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }

        // get LAG OID
        retVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_LAG, xpsDevId,
                                  (sai_uint64_t) pSaiPortLagInfoEntry->lagId, &lagOid);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("Could not get LAG OID. vlanId %u | port_id %u | lagId %d | retVal %d \n",
                           vlanId, xpsIntf, pSaiPortLagInfoEntry->lagId, retVal);
            return retVal;
        }

        retVal = xpSaiGetLagAttrPortVlanId(lagOid, &pvidValue);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetLagAttrPortVlanId failed. vlanId %u | port_id %u | lagOid %lu | retVal %d \n",
                           vlanId, xpsIntf, lagOid, retVal);
            return retVal;
        }
        retVal = xpSaiSetLagAttrPortVlanId(lagOid, pvidValue);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiSetLagAttrPortVlanId failed. vlanId %u | port_id %u | lagOid %lu | retVal %d \n",
                           vlanId, xpsIntf, lagOid, retVal);
            return retVal;
        }

        xpsRetVal = xpsVlanInterfaceDefaultIngressStpSet(xpsDevId,
                                                         pSaiPortLagInfoEntry->lagId, vlanId);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsVlanInterfaceDefaultIngressStpSet failed vlanId %u| port_id %u | xpsRetVal %d \n",
                           vlanId, xpsIntf, xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }

        XP_SAI_LOG_DBG("Set VLAN on LAG=%u, value=%d) \n", pSaiPortLagInfoEntry->lagId,
                       vlanId);
    }
    else if (SAI_STATUS_ITEM_NOT_FOUND == retVal)
    {
        //Fetch from SAI DB
        xpsRetVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, vlanId, &saiVlanCtx);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Insert sai vlan context failed, vlanId(%d)", vlanId);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
        if (saiVlanCtx->isRspanVlan == true)
        {
            tagType = xpSaiConvertRspanTagging(tagType);
            if (xpsRetVal != XP_NO_ERR)
            {
                return xpsStatus2SaiStatus(xpsRetVal);
            }
        }
        xpsRetVal = xpsVlanAddInterface(xpsDevId, vlanId, xpsIntf, tagType);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsVlanAddInterface failed vlanId %u | port_id %u | xpsRetVal %d \n",
                           vlanId, xpsIntf, xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }

        // In case when saiPortId is lag - set PVID on LAG
        if (XDK_SAI_OBJID_TYPE_CHECK(saiPortId, SAI_OBJECT_TYPE_LAG) &&
            tagType == XP_L2_ENCAP_DOT1Q_UNTAGGED)
        {
            pvidValue.u16 = vlanId;
            retVal = xpSaiSetLagAttrPortVlanId(saiPortId, pvidValue);
            if (SAI_STATUS_SUCCESS != retVal)
            {
                XP_SAI_LOG_ERR("xpSaiSetLagAttrPortVlanId failed vlanId %u | port_id %u | retVal %d \n",
                               vlanId, xpsIntf, retVal);
                return retVal;
            }
        }
    }

    /*Create an object*/
    if (xpSaiVlanMemberObjIdCreate(xpsDevId, vlanId, xpsIntf,
                                   vlan_member_id) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI LAG member object could not be created.\n");
        return SAI_STATUS_FAILURE;
    }

    xpsRetVal = xpSaiBridgeAddMappingElement(saiVlanOid, *vlan_member_id,
                                             saiBrPortId);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiBridgeAddMappingElement failed vlanId %u | xpsRetVal %d \n",
                       vlanId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    retVal = xpSaiIpmcGroupOnVlanMemberUpdateCallback(xpsDevId, xpsIntf, vlanId,
                                                      true);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiIpmcGroupOnVlanMemberUpdateCallback failed (vlanId: %u, xpsIntf: %u, create: %s, retVal: %d)\n",
                       vlanId, xpsIntf, "true", retVal);
        return retVal;
    }

    // Update the ACL config on each Lag member
    retVal = xpSaiAclUpdateMemberInfo(saiVlanCtx->ingressAclId, xpsIntf, true);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("Failed to set ACL %u config on port %lu, error %d\n", vlanId,
                       xpsIntf, retVal);
        return retVal;
    }

    retVal = xpSaiAclUpdateMemberInfo(saiVlanCtx->egressAclId, xpsIntf, true);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("Failed to set ACL %u config on port %lu, error %d\n", vlanId,
                       xpsIntf, retVal);
        return retVal;
    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t xpSaiRemoveVlanMember(_In_ sai_object_id_t vlan_member_id)
{
    sai_status_t           retVal          = SAI_STATUS_SUCCESS;
    XP_STATUS              xpsRetVal          = XP_NO_ERR;
    xpsInterfaceId_t       xpsIntf         = 0;
    xpsDevice_t            xpsDevId        = xpSaiGetDevId();
    sai_vlan_id_t          vlanId         = 0;
    sai_object_id_t        vlan_id        = SAI_NULL_OBJECT_ID;
    sai_object_id_t        brPort_id      = SAI_NULL_OBJECT_ID;
    xpSaiPortLagInfo_t* pSaiPortLagInfoEntry = NULL;
    xpSaiLagPortCountInfo_t* pSaiLagPortCountInfoEntry = NULL;
    xpSaiLagPortCountInfo_t saiLagPortCountInfoKey;
    xpSaiBridgePort_t       bridgePort;
    sai_attribute_value_t    pvidValue;
    xpsL2EncapType_e        tagType         = XP_L2_ENCAP_INVALID;
    xpSaiVlanContextDbEntry *saiVlanCtx = NULL;

    memset(&saiLagPortCountInfoKey, 0, sizeof(saiLagPortCountInfoKey));

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(vlan_member_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    vlanId = xpSaiVlanMemberVlanIdValueGet(vlan_member_id);
    retVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, xpsDevId, vlanId, &vlan_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiObjIdCreate failed vlanId %u | xpsRetVal %d \n",
                       vlanId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    xpsRetVal = xpSaiBridgeGetMappingElement(vlan_id, vlan_member_id, &brPort_id);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiBridgeGetMappingElement failed vlanId %u | xpsRetVal %d \n",
                       vlanId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (SAI_NULL_OBJECT_ID == brPort_id)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    memset(&bridgePort, 0, sizeof(xpSaiBridgePort_t));

    xpsRetVal = xpSaiGetBridgePortById(brPort_id, &bridgePort);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetBridgePortById failed vlanId %u | bridgePort %lu | xpsRetVal %d \n",
                       vlanId, brPort_id, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    xpsIntf = xpSaiVlanMemberPortIdValueGet(bridgePort.brPortDescr.objId);

    if (XDK_SAI_OBJID_TYPE_CHECK(bridgePort.brPortDescr.objId,
                                 SAI_OBJECT_TYPE_TUNNEL))
    {
        retVal = xpSaiTunnelGetTunnelIntf(xpsDevId, bridgePort.brPortDescr.objId,
                                          &xpsIntf);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelGetTunnelId retVal : %d \n",
                           retVal);
            return retVal;
        }
    }

    XP_SAI_LOG_DBG("Got vlanId %u, xpsIntf %d.\n", vlanId, xpsIntf);

    /* delete bridge mapping info*/
    retVal = xpSaiBridgeDelMappingElement(vlan_id, vlan_member_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiBridgeDelMappingElement failed vlanId %u | xpsRetVal %d \n",
                       vlanId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    retVal = xpSaiIpmcGroupOnVlanMemberUpdateCallback(xpsDevId, xpsIntf, vlanId,
                                                      false);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiIpmcGroupOnVlanMemberUpdateCallback failed (vlanId: %u, xpsIntf: %u, create: %s, retVal: %d)\n",
                       vlanId, xpsIntf, "false", retVal);
        return retVal;
    }

    xpsRetVal = xpsVlanGetIntfTagTypeScope(XP_SCOPE_DEFAULT, vlanId, xpsIntf,
                                           &tagType);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("Not able to retrive data for tagtype Error %d\n",
                       xpsRetVal);
        return  xpsStatus2SaiStatus(xpsRetVal);
    }

    xpsRetVal = xpsVlanRemoveInterface(xpsDevId, vlanId, xpsIntf/*, tagType*/);

    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanRemoveInterface failed vlanId %u | port_id %u | xpsRetVal %u\n",
                       vlanId, xpsIntf, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    /*This has been added to model behaviour added as part of LAG membership model,
      where each some attributes of port in the LAG may be configured separately(VLAN assagned, speed, MTU etc...).
      It is assumed SAI client application will take care of consistency.*/
    if (xpSaiPortLagInfoGet(xpsIntf, &pSaiPortLagInfoEntry) == SAI_STATUS_SUCCESS)
    {
        if (xpSaiLagPortCountInfoGet(pSaiPortLagInfoEntry->lagId, vlanId,
                                     &pSaiLagPortCountInfoEntry) == SAI_STATUS_SUCCESS)
        {
            if (1 == pSaiLagPortCountInfoEntry->membersPortCount)
            {
                /*In case port is part of LAG configure VLAN on LAG.*/
                xpsRetVal = (XP_STATUS) xpsVlanRemoveInterface(xpsDevId, vlanId,
                                                               pSaiPortLagInfoEntry->lagId/*, tagType*/);
                if (xpsRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_DBG("xpsVlanRemoveInterface failed vlanId %u | port_id %u | retVal %d \n",
                                   vlanId, xpsIntf, retVal);
                }

                /*clean-up*/
                saiLagPortCountInfoKey.lagId = pSaiPortLagInfoEntry->lagId;
                saiLagPortCountInfoKey.vlanId = vlanId;
                if ((xpsRetVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, portLagPortCountDbHandle,
                                                    &saiLagPortCountInfoKey, (void**)&pSaiLagPortCountInfoEntry)) != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Could not delete channel structure from DB, saiLagPortCountInfoKey.lagId %d\n",
                                   saiLagPortCountInfoKey.lagId);
                    return xpsStatus2SaiStatus(xpsRetVal);
                }

                xpsRetVal = xpsStateHeapFree(pSaiLagPortCountInfoEntry);
                if (xpsRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Could not deallocate structure for LAG %d\n",
                                   pSaiPortLagInfoEntry->lagId);
                    return xpsStatus2SaiStatus(xpsRetVal);
                }

                XP_SAI_LOG_DBG("Remove VLAN %u on LAG=%u) \n", vlanId,
                               pSaiPortLagInfoEntry->lagId);
            }
            else /*Decrease the counter.*/
            {
                pSaiLagPortCountInfoEntry->membersPortCount--;
            }
        }
    }


    // In case when remove untagged vlan member, re-configure PVID on LAG or port

    if (tagType == XP_L2_ENCAP_DOT1Q_UNTAGGED)
    {
        if (XDK_SAI_OBJID_TYPE_CHECK(bridgePort.brPortDescr.objId, SAI_OBJECT_TYPE_LAG))
        {
            xpSaiLagInfo_t *lagConfig = NULL;
            retVal = xpSaiLagConfigInfoGet(xpsIntf, &lagConfig);
            if (SAI_STATUS_SUCCESS != retVal)
            {
                XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", xpsIntf,
                               retVal);
                return XP_PORT_NOT_VALID;
            }
            pvidValue.u16 = lagConfig->pvidUserSetting;

            retVal = xpSaiSetLagAttrPortVlanId(bridgePort.brPortDescr.objId, pvidValue);
            if (SAI_STATUS_SUCCESS != retVal)
            {
                XP_SAI_LOG_ERR("xpSaiSetLagAttrPortVlanId failed vlanId %u | port_id %u | retVal %d \n",
                               vlanId, xpsIntf, retVal);
                return retVal;
            }
        }

        if (XDK_SAI_OBJID_TYPE_CHECK(bridgePort.brPortDescr.objId,
                                     SAI_OBJECT_TYPE_PORT))
        {
            xpSaiPortDbEntryT *pPortEntry = NULL;
            retVal = xpSaiPortDbInfoGet(xpsIntf, &pPortEntry);
            if (retVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not retrieve port info from DB\n");
                return retVal;
            }
            pvidValue.u16 = pPortEntry->pvidUserSetting;
            retVal = xpSaiSetPortAttrDefaultVlan(bridgePort.brPortDescr.objId, pvidValue);
            if (SAI_STATUS_SUCCESS != retVal)
            {
                XP_SAI_LOG_ERR("xpSaiSetPortAttrDefaultVlan failed vlanId %u | port_id %u | retVal %d \n",
                               vlanId, xpsIntf, retVal);
                return retVal;
            }
        }
    }
    // Fetch from SAI DB
    xpsRetVal = xpSaiVlanGetVlanCtxDb(XP_SCOPE_DEFAULT, vlanId, &saiVlanCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Insert sai vlan context failed, vlanId(%d)", vlanId);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    // Update the ACL config on each Lag member
    retVal = xpSaiAclUpdateMemberInfo(saiVlanCtx->ingressAclId, xpsIntf, false);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("Failed to set ACL %u config on port %lu, error %d\n", vlanId,
                       xpsIntf, retVal);
        return retVal;
    }

    retVal = xpSaiAclUpdateMemberInfo(saiVlanCtx->egressAclId, xpsIntf, false);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("Failed to set ACL %u config on port %lu, error %d\n", vlanId,
                       xpsIntf, retVal);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiSetVlanMemberAttribute(_In_ sai_object_id_t vlan_member_id,
                                         _In_ const sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    saiRetVal = xpSaiAttrCheck(1, attr,
                               VLAN_MEMBER_VALIDATION_ARRAY_SIZE, vlan_member_attribs,
                               SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    switch (attr->id)
    {
        case SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE:
            {
                XP_SAI_LOG_ERR("SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE is unsupported\n");
                return SAI_STATUS_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return saiRetVal;
}

//Func: xpSaiGetVlanMemberAttribute

static sai_status_t xpSaiGetVlanMemberAttribute(sai_object_id_t vlan_member_id,
                                                sai_attribute_t *attr, uint32_t attr_index)
{
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    xpsDevice_t     devId       = xpSaiGetDevId();

    switch (attr->id)
    {
        case SAI_VLAN_MEMBER_ATTR_VLAN_ID:
            {
                sai_vlan_id_t vlanId = xpSaiVlanMemberVlanIdValueGet(vlan_member_id);

                saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, devId, vlanId,
                                             &attr->value.oid);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Error : SAI object can not be created.\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID:
            {
                XP_STATUS        xpsRetVal = XP_NO_ERR;
                sai_vlan_id_t    vlanId    = xpSaiVlanMemberVlanIdValueGet(vlan_member_id);
                sai_object_id_t  vlan_id   = SAI_NULL_OBJECT_ID;

                saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, devId, vlanId, &vlan_id);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Error : SAI object can not be created.\n");
                    return saiRetVal;
                }

                xpsRetVal = xpSaiBridgeGetMappingElement(vlan_id, vlan_member_id,
                                                         &attr->value.oid);
                if (xpsRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiBridgeGetMappingElement failed, vlanId(%d) | retVal : %d \n",
                                   vlanId, xpsRetVal);
                    return xpsStatus2SaiStatus(xpsRetVal);
                }

                if (SAI_NULL_OBJECT_ID == attr->value.oid)
                {
                    XP_SAI_LOG_ERR("Bridge port not found, vlanId(%d)\n", vlanId);
                    return SAI_STATUS_ITEM_NOT_FOUND;
                }
                break;
            }
        case SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE:
            {
                XP_STATUS        xpsRetVal = XP_NO_ERR;
                sai_vlan_id_t    vlanId    = xpSaiVlanMemberVlanIdValueGet(vlan_member_id);
                xpsInterfaceId_t xpsIntf   = xpSaiVlanMemberPortIdValueGet(vlan_member_id);
                xpsL2EncapType_e tagType   = XP_L2_ENCAP_INVALID;

                xpsRetVal = xpsVlanGetIntfTagTypeScope(XP_SCOPE_DEFAULT, vlanId, xpsIntf,
                                                       &tagType);
                if (XP_NO_ERR != xpsRetVal)
                {
                    XP_SAI_LOG_ERR("Not able to retrive data for SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE! Error %d\n",
                                   xpsRetVal);
                    return  xpsStatus2SaiStatus(xpsRetVal);
                }
                attr->value.u32 = xpSaiConvertTaggingModeToSai(tagType);
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Wrong attribute received %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetVlanMemberAttributes

static sai_status_t xpSaiGetVlanMemberAttributes(_In_ sai_object_id_t
                                                 vlan_member_id, _In_ const uint32_t attr_count,
                                                 _Inout_ sai_attribute_t *attr_list)
{
    sai_status_t           saiRetVal      = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t       xpsIntf        = 0;
    sai_vlan_id_t          vlanId         = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(vlan_member_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    vlanId = xpSaiVlanMemberVlanIdValueGet(vlan_member_id);
    xpsIntf = xpSaiVlanMemberPortIdValueGet(vlan_member_id);
    XP_SAI_LOG_DBG("Got vlanId %u, xpsIntf %d.\n", vlanId, xpsIntf);

    for (sai_uint32_t indx = 0; indx < attr_count; indx++)
    {
        saiRetVal = xpSaiGetVlanMemberAttribute(vlan_member_id, &attr_list[indx], indx);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiGetVlanMemberAttribute failed with saiRetVal : %d\n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    return saiRetVal;
}

//Func: xpSaiBulkGetVlanMemberAttributes

sai_status_t xpSaiBulkGetVlanMemberAttributes(sai_object_id_t id,
                                              uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_VLAN_MEMBER))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountVlanMemberAttribute(&maxcount);
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
        attr_list[idx].id = SAI_VLAN_MEMBER_ATTR_START + count;
        saiRetVal = xpSaiGetVlanMemberAttribute(id, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

// Func: xpSaiCreateVlanStack

sai_status_t xpSaiCreateVlanStack(_Out_ sai_object_id_t *vlanStackIdObj,
                                  _In_ sai_object_id_t switch_id,
                                  _In_ uint32_t attr_count,
                                  _In_ const sai_attribute_t *attr_list)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsVlanStack_t vlanStackId;
    xpsDevice_t devId = xpSaiGetDevId();
    sai_vlan_stack_stage_t saiVlanStackStage = SAI_VLAN_STACK_STAGE_INGRESS;
    sai_vlan_stack_action_t saiVlanStackAction = SAI_VLAN_STACK_ACTION_SWAP;
    sai_object_id_t saiPortId = SAI_NULL_OBJECT_ID;
    xpsVlanStackStage_e xpsVlanStackStage;
    xpsVlanStackAction_e xpsVlanStackAction;
    xpsInterfaceId_t xpsIntf;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if ((NULL == vlanStackIdObj) || (NULL == attr_list))
    {
        XP_SAI_LOG_ERR("Invalid parameter received!");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               VLAN_STACK_VALIDATION_ARRAY_SIZE, vlan_stack_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    /* Check input conditions.*/
    if (attr_count < 3)
    {
        XP_SAI_LOG_ERR("Wrong number of input attributes received");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (sai_uint32_t indx = 0; indx < attr_count; indx++)
    {
        switch (attr_list[indx].id)
        {
        case SAI_VLAN_STACK_ATTR_STAGE:
            saiVlanStackStage = (sai_vlan_stack_stage_t)attr_list[indx].value.s32;
            break;
        case SAI_VLAN_STACK_ATTR_ACTION:
            saiVlanStackAction = (sai_vlan_stack_action_t)attr_list[indx].value.s32;
            break;
        case SAI_VLAN_STACK_ATTR_PORT:
            saiPortId = attr_list[indx].value.oid;
            break;
        default:
            XP_SAI_LOG_ERR("Invalid attribute received(%u)!", attr_list[indx].id);
            break;
        }
    }

    if (!(XDK_SAI_OBJID_TYPE_CHECK(saiPortId, SAI_OBJECT_TYPE_PORT) ||
          XDK_SAI_OBJID_TYPE_CHECK(saiPortId, SAI_OBJECT_TYPE_LAG)))
    {
        XP_SAI_LOG_ERR("For wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(saiPortId));
        saiRetVal = SAI_STATUS_INVALID_ATTR_VALUE_0;
        return saiRetVal;
    }

    if (saiVlanStackStage == SAI_VLAN_STACK_STAGE_INGRESS)
    {
        xpsVlanStackStage = XP_INGRESS_PORT_STACK;
    }
    else
    {
        xpsVlanStackStage = XP_EGRESS_PORT_STACK;
    }

    if (saiVlanStackAction == SAI_VLAN_STACK_ACTION_SWAP)
    {
        XP_SAI_LOG_INFO("SAI_VLAN_STACK_ACTION_SWAP is not supported.");
        return SAI_STATUS_NOT_SUPPORTED;
        // xpsVlanStackAction = XP_ACTION_SWAP;
    }
    else if (saiVlanStackAction == SAI_VLAN_STACK_ACTION_PUSH)
    {
        xpsVlanStackAction = XP_ACTION_PUSH;
    }
    else
    {
        xpsVlanStackAction = XP_ACTION_POP;
    }

    xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(saiPortId);

    retVal = xpsVlanCreateStack(XP_SCOPE_DEFAULT, xpsVlanStackStage,
                                xpsVlanStackAction, xpsIntf, &vlanStackId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanCreateStack failed vlanStackId %d | retVal : %d \n", vlanStackId,
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /*Create vlan stack object after successful creation of vlan*/
    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN_STACK, devId, vlanStackId, vlanStackIdObj);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI vlan object could not be created\n");
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

// Func: xpSaiRemoveVlanStack

sai_status_t xpSaiRemoveVlanStack(_In_ sai_object_id_t vlanStackIdObj)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsVlanStack_t vlanStackId;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(vlanStackIdObj, SAI_OBJECT_TYPE_VLAN_STACK))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(vlanStackIdObj));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    vlanStackId = (xpsStp_t)xpSaiObjIdValueGet(vlanStackIdObj);

    retVal = xpsVlanRemoveStack(XP_SCOPE_DEFAULT, vlanStackId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanCreateStack failed vlanStackId %d | retVal : %d \n", vlanStackId,
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*     Removes interface from all VLAN in which it participates
*
* Arguments:
*     [in] sai_object_id_t : portOid
*
* Return Values:
*    sai_status_t
*/
sai_status_t xpSaiExcludeIntfAllVlans(sai_object_id_t portOid)
{
    sai_status_t      saiRetVal    = SAI_STATUS_SUCCESS;
    XP_STATUS         retVal       = XP_NO_ERR;
    xpsDevice_t       xpsDevId     = xpSaiGetDevId();
    xpsInterfaceId_t  xpsPortId    = 0;
    xpsVlan_t         vlanId       = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    saiRetVal = xpSaiConvertPortOid(portOid, &xpsPortId);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("xpSaiConvertPortOid() failed with error code: %d\n", saiRetVal);
        return saiRetVal;
    }

    retVal = xpsVlanGetFirst(xpsDevId, &vlanId);
    do
    {
        if (retVal == XP_ERR_NOT_FOUND)
        {
            break;
        }
        else if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get VLAN ID. Error code: %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        retVal = xpsVlanIsIntfExist(xpsDevId, vlanId, xpsPortId);
        if (retVal == XP_NO_ERR)
        {
            if (xpsVlanRemoveInterface(xpsDevId, vlanId, xpsPortId) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to remove port %u from VLAN %u\n", xpsPortId, vlanId);
                return SAI_STATUS_FAILURE;
            }
        }

        retVal = xpsVlanGetNext(xpsDevId, vlanId, &vlanId);

    }
    while (1);

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiVlanCreateDefaultVlanState(xpsDevice_t xpSaiDevId,
                                             sai_vlan_id_t vlanid)
{
    xpSaiVlanContextDbEntry *vlanCtxPtr = NULL;
    sai_object_id_t         vlanIdObj;
    sai_status_t            saiStatus   = SAI_STATUS_SUCCESS;
    XP_STATUS               xpStatus    = XP_NO_ERR;

    XP_SAI_LOG_DBG("Configuring default VLAN. VLAN ID is %d.\n", vlanid);

    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, xpSaiDevId, vlanid,
                                 &vlanIdObj);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI vlan object could not be created\n");
        return saiStatus;
    }

    xpStatus = xpSaiInsertVlanCtxDb(XP_SCOPE_DEFAULT, vlanid, &vlanCtxPtr);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Insert sai vlan context failed, vlanId(%d)", vlanid);
        return xpStatus;
    }

    vlanCtxPtr->vlanId = vlanid;
    vlanCtxPtr->monitorPortRefCount = 0;

    xpStatus = xpSaiBridgeCreateMappingEntry(vlanIdObj);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiBridgeCreateMappingEntry failed vlanId %d | retVal : %d \n",
                       vlanid, xpStatus);
        return xpStatus;
    }

    XP_SAI_LOG_DBG("Created default VLAN. VLAN ID is %d.\n", vlanid);

    return saiStatus;
}

sai_status_t xpSaiVlanCreateVlanMembersState(xpsDevice_t xpSaiDevId,
                                             sai_vlan_id_t vlanid)
{
    sai_status_t        saiStatus    = SAI_STATUS_SUCCESS;
    uint32_t            count        = 0;
    uint32_t            brPortsCount = 1;
    sai_object_id_t     switchOid;
    sai_object_id_t     vlanOid;
    XP_STATUS           xpsRetVal    = XP_NO_ERR;
    sai_object_id_t     bridgeOid;
    sai_object_id_t     tmpOid;
    sai_object_id_t     vlan_member_id;
    uint16_t            vlanId       = 0;
    sai_object_id_t     saiPortId    = SAI_NULL_OBJECT_ID;
    xpsInterfaceId_t    xpsIntf      = 0;
    xpSaiBridgePort_t   bridgePort;
    sai_object_id_t    *objList      = &tmpOid;
    sai_attribute_t     attr_list[3];

    XP_SAI_LOG_DBG("Configuring members of default VLAN. VLAN ID is %d.\n", vlanid);

    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_SWITCH, xpSaiDevId, 0,
                         &switchOid) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI switch object could not be created.\n");
        return SAI_STATUS_FAILURE;
    }

    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, xpSaiDevId, vlanid,
                         &vlanOid) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI VLAN object could not be created.\n");
        return SAI_STATUS_FAILURE;
    }

    if (xpSaiSwitchDefaultBridgeIdGet(&bridgeOid) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get default .1Q bridge OID\n");
        return SAI_STATUS_FAILURE;
    }

    attr_list[0].id = SAI_BRIDGE_ATTR_PORT_LIST;
    attr_list[0].value.objlist.count = 1;
    attr_list[0].value.objlist.list  = objList;

    // by default all ports added to the default .1Q bridge, so we can simple
    // retrieve all bridge ports and add it as VLAN members
    saiStatus = xpSaiGetBridgeAttributes(bridgeOid, 1, attr_list);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        if (SAI_STATUS_BUFFER_OVERFLOW == saiStatus)
        {
            objList  = (sai_object_id_t*)xpMalloc(sizeof(sai_object_id_t) *
                                                  attr_list[0].value.objlist.count);
            if (NULL == objList)
            {
                XP_SAI_LOG_ERR("Could not allocate memory\n");
                return SAI_STATUS_FAILURE;
            }

            attr_list[0].value.objlist.list = objList;

            saiStatus = xpSaiGetBridgeAttributes(bridgeOid, 1, attr_list);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                xpFree(objList);
                XP_SAI_LOG_ERR("Could not get .1Q bridge attributes\n");
                return saiStatus;
            }
        }
        else
        {
            XP_SAI_LOG_ERR("Could not get .1Q bridge attributes\n");
            return saiStatus;
        }
    }

    brPortsCount = attr_list[0].value.objlist.count;

    for (count = 0; count < brPortsCount; ++count)
    {
        // Updating state for default Vlan Members
        vlanId = (uint16_t)xpSaiObjIdValueGet(vlanOid);
        memset(&bridgePort, 0, sizeof(xpSaiBridgePort_t));
        xpsRetVal = xpSaiGetBridgePortById(objList[count], &bridgePort);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiGetBridgePortById failed vlanId ");
            return xpsStatus2SaiStatus(xpsRetVal);
        }
        saiPortId = bridgePort.brPortDescr.objId;
        xpsIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(saiPortId);
        if (xpSaiVlanMemberObjIdCreate(xpSaiDevId, vlanId, xpsIntf,
                                       &vlan_member_id) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI LAG member object could not be created.\n");
            return SAI_STATUS_FAILURE;
        }

        xpsRetVal = xpSaiBridgeAddMappingElement(vlanOid, vlan_member_id,
                                                 objList[count]);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiBridgeAddMappingElement failed vlanId %u | xpsRetVal %d \n",
                           vlanId, xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }

    if (brPortsCount > 1 && objList != &tmpOid)
    {
        xpFree(objList);
    }

    return saiStatus;
}

sai_status_t xpSaiDeleteDefaultVlan(xpsDevice_t xpSaiDevId)
{
    sai_status_t        saiStatus  = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus   = XP_NO_ERR;
    xpsVlan_t           vlanId     = 0;
    sai_object_id_t     vlanIdObj;

    xpStatus = xpsVlanGetDefault(xpSaiDevId, &vlanId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default VLAN ID\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    XP_SAI_LOG_DBG("Removing default VLAN ID %u for device %u\n", vlanId,
                   xpSaiDevId);

    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, xpSaiDevId, vlanId,
                                 &vlanIdObj);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create SAI VLAN object ID\n");
        return saiStatus;
    }

    saiStatus = xpSaiRemoveVlan(vlanIdObj);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove default VLAN\n");
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiVlanApiInit

XP_STATUS xpSaiVlanApiInit(uint64_t flag,
                           const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    _xpSaiVlanApi = (sai_vlan_api_t *) xpMalloc(sizeof(sai_vlan_api_t));

    if (NULL == _xpSaiVlanApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiVlanApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiVlanApi->create_vlan = xpSaiCreateVlan;
    _xpSaiVlanApi->remove_vlan = xpSaiRemoveVlan;
    _xpSaiVlanApi->set_vlan_attribute = xpSaiSetVlanAttributes;
    _xpSaiVlanApi->get_vlan_attribute = xpSaiGetVlanAttributes;
    _xpSaiVlanApi->create_vlan_member = xpSaiCreateVlanMember;
    _xpSaiVlanApi->remove_vlan_member = xpSaiRemoveVlanMember;
    _xpSaiVlanApi->set_vlan_member_attribute = xpSaiSetVlanMemberAttribute;
    _xpSaiVlanApi->get_vlan_member_attribute = xpSaiGetVlanMemberAttributes;
    _xpSaiVlanApi->get_vlan_stats = xpSaiGetVlanStats;
    _xpSaiVlanApi->clear_vlan_stats = xpSaiClearVlanStats;
    _xpSaiVlanApi->create_vlan_stack = xpSaiCreateVlanStack;
    _xpSaiVlanApi->remove_vlan_stack = xpSaiRemoveVlanStack;
    // _xpSaiVlanApi->set_vlan_stack_attribute = xpSaiSetVlanStackAttributes;
    // _xpSaiVlanApi->get_vlan_stack_attribute = xpSaiGetVlanStackAttributes;
    _xpSaiVlanApi->clear_vlan_stats = xpSaiClearVlanStats;

    _xpSaiVlanApi->create_vlan_members = (sai_bulk_object_create_fn)
                                         xpSaiStubGenericApi;
    _xpSaiVlanApi->remove_vlan_members = (sai_bulk_object_remove_fn)
                                         xpSaiStubGenericApi;
    _xpSaiVlanApi->get_vlan_stats_ext = (sai_get_vlan_stats_ext_fn)
                                        xpSaiStubGenericApi;

    saiRetVal = xpSaiApiRegister(SAI_API_VLAN, (void*)_xpSaiVlanApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register VLAN API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}

//Func: xpSaiVlanApiDeinit

XP_STATUS xpSaiVlanApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    xpFree(_xpSaiVlanApi);
    _xpSaiVlanApi = NULL;

    return  retVal;
}

sai_status_t xpSaiMaxCountVlanAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_VLAN_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiMaxCountVlanMemberAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_VLAN_MEMBER_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountVlanObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsCountVlanObjects(xpSaiGetDevId(), count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiCountVlanMemberObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsCountVlanMemberObjects(xpSaiGetDevId(), count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetVlanObjectList(uint32_t *object_count,
                                    sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpsVlan_t       *vlanId;
    xpsDevice_t     devId       = xpSaiGetDevId();

    saiRetVal = xpSaiCountVlanObjects(&objCount);
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

    vlanId = (xpsVlan_t *)xpMalloc(sizeof(xpsVlan_t) * (*object_count));
    if (vlanId == NULL)
    {
        XP_SAI_LOG_ERR("Could not allocate memory for vlanId array!\n");
        return SAI_STATUS_NO_MEMORY;
    }

    retVal = xpsVlanGetVlanIdList(xpSaiGetDevId(), vlanId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
        xpFree(vlanId);
        return xpsStatus2SaiStatus(retVal);
    }

    *object_count = objCount;

    for (uint32_t i = 0; i < *object_count; i++)
    {
        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, devId,
                                     (sai_uint64_t)vlanId[i], &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            xpFree(vlanId);
            return saiRetVal;
        }
    }

    xpFree(vlanId);
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetVlanMemberObjectList(uint32_t *object_count,
                                          sai_object_key_t *object_list)
{
    XP_STATUS       retVal          = XP_NO_ERR;
    sai_status_t    saiRetVal       = SAI_STATUS_SUCCESS;
    uint32_t        objCount        = 0;
    uint32_t        *vlanMemberId;
    xpsInterfaceId_t xpsIntf         = 0;
    uint16_t        vlanId;
    xpsDevice_t     devId           = xpSaiGetDevId();

    saiRetVal = xpSaiCountVlanMemberObjects(&objCount);
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

    vlanMemberId = (uint32_t *)xpMalloc(sizeof(uint32_t) * (*object_count));
    if (vlanMemberId == NULL)
    {
        XP_SAI_LOG_ERR("Could not allocate memory for vlanMemberId array!\n");
        return SAI_STATUS_NO_MEMORY;
    }

    retVal = xpsVlanGetVlanMemberIdList(xpSaiGetDevId(), vlanMemberId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
        xpFree(vlanMemberId);
        return xpsStatus2SaiStatus(retVal);
    }

    *object_count = objCount;

    for (uint32_t i = 0; i < *object_count; i++)
    {
        vlanId = vlanMemberId[i] & XPSAI_VLAN_MASK;
        xpsIntf = vlanMemberId[i] >> 12;
        saiRetVal = xpSaiVlanMemberObjIdCreate(devId, vlanId, xpsIntf,
                                               &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            xpFree(vlanMemberId);
            return saiRetVal;
        }
    }

    xpFree(vlanMemberId);
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiConvertSaiFloodControlType2xps(sai_vlan_flood_control_type_t
                                                 sai_packet_action, xpsPktCmd_e* pXpsPktCmd)
{
    if (!pXpsPktCmd)
    {
        XP_SAI_LOG_ERR("Invelid parameter: pXpsPktCmd\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (sai_packet_action)
    {
        case SAI_VLAN_FLOOD_CONTROL_TYPE_ALL:
        case SAI_VLAN_FLOOD_CONTROL_TYPE_L2MC_GROUP:
            {
                *pXpsPktCmd = XP_PKTCMD_FWD;
                break;
            }
        case SAI_VLAN_FLOOD_CONTROL_TYPE_NONE:
            {
                *pXpsPktCmd = XP_PKTCMD_DROP;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown flood control type %d\n", sai_packet_action);
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiConvertSaiMcastLookupKeyType2xps(
    sai_vlan_mcast_lookup_key_type_t mcast_lookup_key_type,
    xpsVlanBridgeMcMode_e *pXpsBrMcMode)
{
    if (!pXpsBrMcMode)
    {
        XP_SAI_LOG_ERR("Invelid parameter: pXpsPktCmd\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (mcast_lookup_key_type)
    {
        case SAI_VLAN_MCAST_LOOKUP_KEY_TYPE_XG_AND_SG:
            {
                *pXpsBrMcMode = MC_BRIDGE_MODE_XG_SG;
                break;
            }
        case SAI_VLAN_MCAST_LOOKUP_KEY_TYPE_XG:
            {
                *pXpsBrMcMode = MC_BRIDGE_MODE_0_G;
                break;
            }
        case SAI_VLAN_MCAST_LOOKUP_KEY_TYPE_SG:
            {
                *pXpsBrMcMode = MC_BRIDGE_MODE_S_G;
                break;
            }
        case SAI_VLAN_MCAST_LOOKUP_KEY_TYPE_MAC_DA:
            {
                *pXpsBrMcMode = MC_BRIDGE_MODE_FDB;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown mcast lookup key type: %d\n", mcast_lookup_key_type);
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiConvertXpsMcastLookupKeyType2sai(xpsVlanBridgeMcMode_e
                                                   xpsBrMcMode, sai_vlan_mcast_lookup_key_type_t *mcast_lookup_key_type)
{
    if (!mcast_lookup_key_type)
    {
        XP_SAI_LOG_ERR("Invelid parameter: mcast_lookup_key_type\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (xpsBrMcMode)
    {
        case MC_BRIDGE_MODE_0_G:
            {
                *mcast_lookup_key_type = SAI_VLAN_MCAST_LOOKUP_KEY_TYPE_XG;
                break;
            }

        case MC_BRIDGE_MODE_S_G:
            {
                *mcast_lookup_key_type = SAI_VLAN_MCAST_LOOKUP_KEY_TYPE_SG;
                break;
            }

        case MC_BRIDGE_MODE_XG_SG:
            {
                *mcast_lookup_key_type = SAI_VLAN_MCAST_LOOKUP_KEY_TYPE_XG_AND_SG;
                break;
            }
        case MC_BRIDGE_MODE_FDB:
            {
                *mcast_lookup_key_type = SAI_VLAN_MCAST_LOOKUP_KEY_TYPE_MAC_DA;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown mcast lookup key type: %d\n", xpsBrMcMode);
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiBridgePortIsVlanMember(sai_object_id_t saiVlanOid,
                                         sai_object_id_t saiBrPortOid)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpSaiBridgeMapItem_t *brPortList = NULL;
    xpSaiBridgeMapItem_t brPort;
    uint32_t brPortListSize = 1;

    xpsRetVal = xpSaiBridgeGetMappingEntry(saiVlanOid, &brPortListSize, &brPort);
    switch (xpsRetVal)
    {
        case XP_NO_ERR:
            {
                /* There is only 1 bridge-port associated with this VLAN. */
                if (brPort.bridgePort == saiBrPortOid)
                {
                    return SAI_STATUS_SUCCESS;
                }
                break;
            }
        case XP_ERR_INVALID_DATA_SIZE:
            {
                /* There are many ports associated with this VLAN. Trying again with brPortListSize updated */
                brPortList = (xpSaiBridgeMapItem_t *)xpMalloc(sizeof(xpSaiBridgeMapItem_t) *
                                                              brPortListSize);

                xpsRetVal = xpSaiBridgeGetMappingEntry(saiVlanOid, &brPortListSize, brPortList);
                if (XP_NO_ERR != xpsRetVal)
                {
                    XP_SAI_LOG_ERR("xpSaiBridgeGetMappingEntry failed: saiVlanOid %u | saiBrPortOid %lu | xpsRetVal %d \n",
                                   saiVlanOid, saiBrPortOid, xpsRetVal);
                    xpFree(brPortList);
                    return xpsRetVal;
                }

                for (uint32_t i = 0; i < brPortListSize; ++i)
                {
                    if (brPortList[i].bridgePort == saiBrPortOid)
                    {
                        xpFree(brPortList);
                        return SAI_STATUS_SUCCESS;
                    }
                }

                xpFree(brPortList);
                break;
            }
        default:
            {
                /* xpSaiBridgeGetMappingEntry failed */
                XP_SAI_LOG_ERR("xpSaiBridgeGetMappingEntry failed: saiVlanOid %u | saiBrPortOid %lu | xpsRetVal %d \n",
                               saiVlanOid, saiBrPortOid, xpsRetVal);
                return xpsRetVal;
            }
    }
    return SAI_STATUS_ITEM_NOT_FOUND;
}

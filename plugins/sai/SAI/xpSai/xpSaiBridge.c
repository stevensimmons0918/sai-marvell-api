// xpSaiBridge.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiPort.h"
#include "xpsPort.h"
#include "xpsVlan.h"
#include "xpSaiSwitch.h"
#include "xpSaiValidationArrays.h"
#include "xpSaiBridge.h"
#include "xpSaiStub.h"

#define XP_SAI_BRIDGE_THRESHOLD       (8)
#define SAI_BRIDGE_PORT_OBJ_TYPE_BITS (32)
#define SAI_BRIDGE_PORT_OBJ_TYPE_MASK (0x000000FF00000000ULL)

#define XP_SAI_BRIDGE_DEV_DEBUG 0
#if XP_SAI_BRIDGE_DEV_DEBUG == 1
#define XP_SAI_BRIDGE_TRACE()     XP_SAI_LOG_DBG ("\n")
#else
#define XP_SAI_BRIDGE_TRACE()
#endif

XP_SAI_LOG_REGISTER_API(SAI_API_BRIDGE);

static sai_bridge_api_t* _xpSaiBridgeApi;

#define XPSAI_CHECK_ERROR_OFFSET(e)   (SAI_STATUS_IS_INVALID_ATTRIBUTE((e)) || \
                                       SAI_STATUS_IS_INVALID_ATTR_VALUE((e)) || \
                                       SAI_STATUS_IS_ATTR_NOT_IMPLEMENTED((e)) || \
                                       SAI_STATUS_IS_UNKNOWN_ATTRIBUTE((e)) || \
                                       SAI_STATUS_IS_ATTR_NOT_SUPPORTED((e)))

/**
 * \brief XPS DB handle for bridge objects
 */
static xpsDbHandle_t xpSaiBridgeStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
/**
 * \brief XPS DB handle for bridge ports objects
 */
static xpsDbHandle_t xpSaiBridgePortStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
/**
 * \brief XPS DB handle for external objects mapping interface
 */
static xpsDbHandle_t xpSaiBrObjsMappingStateDbHndl =
    XPS_STATE_INVALID_DB_HANDLE;

/**
 * \brief XPS DB entry for mapping objects
 */
typedef struct _xpSaiObjsMappingDbEntry
{
    //Key
    sai_object_id_t objId;

    // context
    uint32_t             numItems;
    xpSaiBridgeMapItem_t items[XP_SAI_BRIDGE_THRESHOLD];
} xpSaiBrObjsMapInfoT;

/**
 * \brief Bridge attributes holder
 */
typedef struct _xpSaiBridgeAttributesT
{
    sai_bridge_type_t     bridgeType;
    sai_attribute_value_t maxLearnedAddresses;
    sai_attribute_value_t learnDisable;
    sai_attribute_value_t unUcFloodCtrl;
    sai_attribute_value_t unUcFloodGrp;
    sai_attribute_value_t unMcFloodCtrl;
    sai_attribute_value_t unMcFloodGrp;
    sai_attribute_value_t bcFloodCtrl;
    sai_attribute_value_t bcFloodGrp;
} xpSaiBridgeAttributesT;

/**
 * \brief XPS DB entry for bridges
 */
typedef struct _xpSaiBridgeDbEntry
{
    //Key
    sai_object_id_t                 bridgeId;

    // context
    uint16_t                        bdId;
    sai_bridge_type_t               bridgeType;
    sai_bridge_flood_control_type_t unUcFloodCtrl;
    sai_object_id_t                 unUcFloodGrp;
    sai_bridge_flood_control_type_t unMcFloodCtrl;
    sai_object_id_t                 unMcFloodGrp;
    sai_bridge_flood_control_type_t bcFloodCtrl;
    sai_object_id_t                 bcFloodGrp;
} xpSaiBridgeInfoT;

/**
 * \brief Bridge port attributes holder
 */
typedef struct _xpSaiBridgePortAttributesT
{
    sai_bridge_port_type_t  brPortType;
    sai_uint16_t            vlanId;
    sai_object_id_t         portId;
    sai_object_id_t         rifId;
    sai_object_id_t         tunId;
    sai_object_id_t         brId;
    sai_attribute_value_t   maxLearnedAddresses;
    sai_attribute_value_t   adminState;
    sai_attribute_value_t   learnMode;
    sai_attribute_value_t   learnLimitAction;
    sai_attribute_value_t   taggingMode;
    sai_attribute_value_t   ingressFiltering;
} xpSaiBridgePortAttributesT;

/**
 * \brief XPS DB entry for bridge ports
 */
typedef struct _xpSaiBridgePortDbEntry
{
    //Key
    sai_object_id_t                     brPortId;

    // context
    sai_bridge_port_type_t              brPortType;
    sai_object_id_t                     bridgeId;
    xpSaiBridgePortDescr_t              brPortDescr;

    // options
    uint32_t                            maxLearnedAddresses;    // NYI
    bool                                adminState;             // TBD
    sai_bridge_port_fdb_learning_mode_t learnMode;              // NYI
    sai_packet_action_t                 learnLimitAction;       // NYI
    sai_bridge_port_tagging_mode_t      taggingMode;
    bool                                ingressFiltering;
} xpSaiBridgePortInfoT;

static XP_STATUS xpSaiBridgePortHandleRemove(xpsDevice_t xpsDevId,
                                             sai_object_id_t brPortId);
static xpsL2EncapType_e xpSaiBridgeTaggingMode(sai_bridge_port_tagging_mode_t
                                               sai_tagging_mode);
static XP_STATUS xpSaiBridgeLearningModeToPktCmd(int32_t mode,
                                                 xpsPktCmd_e *pktCmd);

// Comparators

static int32_t saiBridgeCtxKeyComp(void* key1, void* key2)
{
    xpSaiBridgeInfoT *objA = (xpSaiBridgeInfoT*)key1;
    xpSaiBridgeInfoT *objB = (xpSaiBridgeInfoT*)key2;

    return (objA->bridgeId > objB->bridgeId) ? 1 : ((objA->bridgeId <
                                                     objB->bridgeId) ? -1 : 0);
}

static int32_t saiBridgePortCtxKeyComp(void* key1, void* key2)
{
    xpSaiBridgePortInfoT *objA = (xpSaiBridgePortInfoT*)key1;
    xpSaiBridgePortInfoT *objB = (xpSaiBridgePortInfoT*)key2;

    return (objA->brPortId > objB->brPortId) ? 1 : ((objA->brPortId <
                                                     objB->brPortId) ? -1 : 0);
}

static int32_t saiBrObjsMappingCtxKeyComp(void* key1, void* key2)
{
    xpSaiBrObjsMapInfoT *objA = (xpSaiBrObjsMapInfoT*)key1;
    xpSaiBrObjsMapInfoT *objB = (xpSaiBrObjsMapInfoT*)key2;

    return (objA->objId > objB->objId) ? 1 : ((objA->objId < objB->objId) ? -1 : 0);
}

// Support for Bridge mapping objects

static XP_STATUS xpSaiBrMappingGetCtxDb(xpsDevice_t xpsDevId,
                                        sai_object_id_t objId, xpSaiBrObjsMapInfoT **dest)
{
    XP_STATUS           retVal = XP_NO_ERR;
    xpsScope_t          scope  = xpSaiScopeFromDevGet(xpsDevId);
    xpSaiBrObjsMapInfoT key;

    if (NULL == dest)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    memset(&key, 0, sizeof(xpSaiBrObjsMapInfoT));
    key.objId = objId;

    retVal = xpsStateSearchData(scope, xpSaiBrObjsMappingStateDbHndl,
                                (xpsDbKey_t)&key, (void**)dest);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search for entry failed, objId(%lu) | retVal : %d\n", objId,
                       retVal);
        return retVal;
    }

    if (!(*dest))
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    return retVal;
}

static XP_STATUS xpSaiBrMappingRemoveCtxDb(xpsDevice_t xpsDevId,
                                           sai_object_id_t objId)
{
    XP_STATUS            retVal = XP_NO_ERR;
    xpsScope_t           scope  = xpSaiScopeFromDevGet(xpsDevId);
    xpSaiBrObjsMapInfoT *entry  = NULL;
    xpSaiBrObjsMapInfoT  key;

    memset(&key, 0, sizeof(xpSaiBrObjsMapInfoT));
    key.objId= objId;

    retVal = xpsStateDeleteData(scope, xpSaiBrObjsMappingStateDbHndl,
                                (xpsDbKey_t)&key, (void**)&entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Delete entry failed, objId(%lu) | retVal : %d \n", objId,
                       retVal);
        return retVal;
    }

    retVal = xpsStateHeapFree((void*)entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("XPS state Heap Free failed\n");
        return retVal;
    }

    return retVal;
}

static XP_STATUS xpSaiBrMappingInsertCtxDb(xpsDevice_t xpsDevId,
                                           sai_object_id_t objId, xpSaiBrObjsMapInfoT **data)
{
    xpsScope_t scope   = xpSaiScopeFromDevGet(xpsDevId);
    XP_STATUS  retVal  = XP_NO_ERR;

    if (NULL == data)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiBrObjsMapInfoT), (void**)data);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("XPS state Heap Allocation failed \n");
        return retVal;
    }

    memset(*data, 0, sizeof(xpSaiBrObjsMapInfoT));

    (*data)->objId = objId;

    /* Insert the mapping context into the database, objectId is the key */
    retVal = xpsStateInsertData(scope, xpSaiBrObjsMappingStateDbHndl, (void*)*data);
    if (retVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)*data);
        *data = NULL;
        return retVal;
    }

    return retVal;
}

static XP_STATUS xpSaiBrMappingReplaceItem(xpsDevice_t xpsDevId,
                                           sai_object_id_t entryId, xpSaiBrObjsMapInfoT *newEntry)
{
    XP_STATUS            retVal   = XP_NO_ERR;
    xpsScope_t           scope    = xpSaiScopeFromDevGet(xpsDevId);

    XP_SAI_BRIDGE_TRACE();

    if ((SAI_NULL_OBJECT_ID == entryId) || (NULL == newEntry))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_INVALID_INPUT;
    }

    /* Remove the corresponding state */
    retVal = xpSaiBrMappingRemoveCtxDb(xpsDevId, entryId);
    if (retVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)newEntry);
        XP_SAI_LOG_ERR("Deletion of data failed, error: %d\n", retVal);
        return retVal;
    }

    /* Insert the new state */
    retVal = xpsStateInsertData(scope, xpSaiBrObjsMappingStateDbHndl,
                                (void*)newEntry);
    if (retVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)newEntry);
        XP_SAI_LOG_ERR("Insertion of data failed, error: %d\n", retVal);
        return retVal;
    }

    return retVal;
}

// Support for Bridge objects

static XP_STATUS xpSaiBridgeGetCtxDb(xpsDevice_t xpsDevId,
                                     sai_object_id_t bridgeId, xpSaiBridgeInfoT **dest)
{
    XP_STATUS        retVal  = XP_NO_ERR;
    xpsScope_t       scope   = xpSaiScopeFromDevGet(xpsDevId);
    xpSaiBridgeInfoT key;

    if (NULL == dest)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    memset(&key, 0, sizeof(xpSaiBridgeInfoT));
    key.bridgeId = bridgeId;

    retVal = xpsStateSearchData(scope, xpSaiBridgeStateDbHndl, (xpsDbKey_t)&key,
                                (void**)dest);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search data failed, bridgeId(%llu) | retVal : %d \n", bridgeId,
                       retVal);
        return retVal;
    }

    if (!(*dest))
    {
        XP_SAI_LOG_ERR("Bridge does not exist \n");
        return XP_ERR_KEY_NOT_FOUND;
    }

    return retVal;
}

static XP_STATUS xpSaiBridgeRemoveCtxDb(xpsDevice_t xpsDevId,
                                        sai_object_id_t bridgeId)
{
    XP_STATUS         retVal  = XP_NO_ERR;
    xpsScope_t        scope   = xpSaiScopeFromDevGet(xpsDevId);
    xpSaiBridgeInfoT *bridge  = NULL;
    xpSaiBridgeInfoT  key;

    memset(&key, 0, sizeof(xpSaiBridgeInfoT));
    key.bridgeId= bridgeId;

    retVal = xpsStateDeleteData(scope, xpSaiBridgeStateDbHndl, (xpsDbKey_t)&key,
                                (void**)&bridge);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Delete bridge data failed, bridgeId(%llu) | retVal : %d \n",
                       bridgeId, retVal);
        return retVal;
    }

    retVal = xpsStateHeapFree((void*)bridge);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("XPS state Heap Free failed\n");
        return retVal;
    }

    return retVal;
}

static XP_STATUS xpSaiBridgeInsertCtxDb(xpsDevice_t xpsDevId,
                                        sai_object_id_t bridgeId, xpSaiBridgeInfoT **data)
{
    xpsScope_t scope  = xpSaiScopeFromDevGet(xpsDevId);
    XP_STATUS  retVal = XP_NO_ERR;

    if (NULL == data)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiBridgeInfoT), (void**)data);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("XPS state Heap Allocation failed \n");
        return retVal;
    }

    memset(*data, 0, sizeof(xpSaiBridgeInfoT));

    (*data)->bridgeId = bridgeId;

    /* Insert the bridge context into the database, bridgeId is the key */
    retVal = xpsStateInsertData(scope, xpSaiBridgeStateDbHndl, (void*)*data);
    if (retVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)*data);
        *data = NULL;
        return retVal;
    }

    return retVal;
}

// Support for Bridge port objects

static XP_STATUS xpSaiBridgePortGetCtxDb(xpsDevice_t xpsDevId,
                                         sai_object_id_t brPortId, xpSaiBridgePortInfoT **dest)
{
    XP_STATUS    retVal = XP_NO_ERR;
    xpsScope_t   scope  = xpSaiScopeFromDevGet(xpsDevId);
    xpSaiBridgePortInfoT key;

    if (NULL == dest)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    memset(&key, 0, sizeof(xpSaiBridgePortInfoT));
    key.brPortId = brPortId;

    retVal = xpsStateSearchData(scope, xpSaiBridgePortStateDbHndl, (xpsDbKey_t)&key,
                                (void**)dest);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search data failed, brPortId(%llu) | retVal : %d \n", brPortId,
                       retVal);
        return retVal;
    }

    if (!(*dest))
    {
        XP_SAI_LOG_ERR("Bridge port does not exist \n");
        return XP_ERR_KEY_NOT_FOUND;
    }

    return retVal;
}

static XP_STATUS xpSaiBridgePortRemoveCtxDb(xpsDevice_t xpsDevId,
                                            sai_object_id_t brPortId)
{
    XP_STATUS             retVal = XP_NO_ERR;
    xpsScope_t            scope  = xpSaiScopeFromDevGet(xpsDevId);
    xpSaiBridgePortInfoT *brPort = NULL;
    xpSaiBridgePortInfoT  key;

    memset(&key, 0, sizeof(xpSaiBridgePortInfoT));
    key.brPortId = brPortId;

    retVal = xpsStateDeleteData(scope, xpSaiBridgePortStateDbHndl, (xpsDbKey_t)&key,
                                (void**)&brPort);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Delete bridge port data failed, brPortId(%llu) | retVal : %d \n",
                       brPortId, retVal);
        return retVal;
    }

    retVal = xpsStateHeapFree((void*)brPort);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("XPS state Heap Free failed\n");
        return retVal;
    }

    return retVal;
}

static XP_STATUS xpSaiBridgePortInsertCtxDb(xpsDevice_t xpsDevId,
                                            sai_object_id_t brPortId, xpSaiBridgePortInfoT **data)
{
    XP_STATUS  retVal      = XP_NO_ERR;
    xpsScope_t scope       = xpSaiScopeFromDevGet(xpsDevId);

    if (NULL == data)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiBridgePortInfoT), (void**)data);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("XPS state Heap Allocation failed \n");
        return retVal;
    }

    memset(*data, 0, sizeof(xpSaiBridgePortInfoT));

    (*data)->brPortId = brPortId;

    /* Insert the bridge port context into the database, brPortId is the key */
    retVal = xpsStateInsertData(scope, xpSaiBridgePortStateDbHndl, (void*)*data);
    if (retVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)*data);
        *data = NULL;
        return retVal;
    }

    return retVal;
}

//Func: xpSaiConvertSaiBrFloodControlType2xps
static sai_status_t xpSaiConvertSaiBrFloodControlType2xps(
    sai_bridge_flood_control_type_t sai_packet_action, xpsPktCmd_e* pXpsPktCmd)
{
    if (!pXpsPktCmd)
    {
        XP_SAI_LOG_ERR("Invelid parameter: pXpsPktCmd\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (sai_packet_action)
    {
        case SAI_BRIDGE_FLOOD_CONTROL_TYPE_SUB_PORTS:
        case SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP:
            {
                *pXpsPktCmd = XP_PKTCMD_FWD;
                break;
            }
        case SAI_BRIDGE_FLOOD_CONTROL_TYPE_NONE:
            {
                *pXpsPktCmd = XP_PKTCMD_DROP;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown bridge flood control type %d\n", sai_packet_action);
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiBridgeSetAttrUnknownUnicastFloodControlType(
    uint16_t bdId, sai_int32_t value, bool cpuFloodEnable)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsPktCmd_e pktCmd = XP_PKTCMD_FWD;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    retVal = xpSaiConvertSaiBrFloodControlType2xps((sai_bridge_flood_control_type_t)
                                                   value, &pktCmd);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("xpSaiConvertSaiBrFloodControlType2xps failed. pktCmd = %d, retVal = %d \n",
                       value, retVal);
        return retVal;
    }

    // If CPU flooding is enabled - set appropriate pktCmd
    xpSaiVlanUpdatePktCmd(cpuFloodEnable, &pktCmd);

    xpsRetVal = xpsVlanSetUnknownUcCmd(xpsDevId, (xpsVlan_t) bdId, pktCmd);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanSetUnknownSaCmd() failed bdId=%u, retVal=%d\n", bdId,
                       xpsRetVal);
        return retVal;
    }

    return retVal;
}

static sai_status_t xpSaiBridgeSetAttrUnknownMulticastFloodControlType(
    uint16_t bdId, sai_int32_t value, bool cpuFloodEnable)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsPktCmd_e pktCmd = XP_PKTCMD_FWD;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    retVal = xpSaiConvertSaiBrFloodControlType2xps((sai_bridge_flood_control_type_t)
                                                   value, &pktCmd);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("xpSaiConvertSaiBrFloodControlType2xps failed. pktCmd = %d, retVal = %d \n",
                       value, retVal);
        return retVal;
    }

    // If CPU flooding is enabled - set appropriate pktCmd
    xpSaiVlanUpdatePktCmd(cpuFloodEnable, &pktCmd);

    xpsRetVal = xpsVlanSetUnregMcCmd(xpsDevId, bdId, pktCmd);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanSetUnregMcCmd() failed bdId=%u, retVal=%d\n", bdId,
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return retVal;
}

static sai_status_t xpSaiBridgeSetAttrBroadcastFloodControlType(xpsVlan_t bdId,
                                                                sai_int32_t value, bool cpuFloodEnable)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsPktCmd_e pktCmd = XP_PKTCMD_FWD;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    retVal = xpSaiConvertSaiBrFloodControlType2xps((sai_bridge_flood_control_type_t)
                                                   value, &pktCmd);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("xpSaiConvertSaiBrFloodControlType2xps failed. pktCmd = %d, retVal = %d \n",
                       value, retVal);
        return retVal;
    }

    // If CPU flooding is enabled - set appropriate pktCmd
    xpSaiVlanUpdatePktCmd(cpuFloodEnable, &pktCmd);

    xpsRetVal = xpsVlanSetBcCmd(xpsDevId, bdId, pktCmd);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanSetBcCmd() failed bdId=%u, retVal=%d\n", bdId,
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return retVal;
}

static sai_status_t xpSaiBridgeSetBroadcastFloodControlGroup(
    sai_object_id_t bridgeOid, sai_object_id_t listOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsMcL2InterfaceListId_t listId = 0;
    xpsVlan_t bdId = (xpsVlan_t)xpSaiObjIdValueGet(bridgeOid);

    if (listOid != SAI_NULL_OBJECT_ID)
    {
        retVal = xpSaiActivateBridgeL2McGroup(xpsDevId, bridgeOid, listOid,
                                              (uint32_t*) &listId);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiActivateBridgeL2McGroup() failed. bdId = %d, listId = %d, retVal = %d \n",
                           bdId, listId, retVal);
            return retVal;
        }
    }
    else
    {
        listId = xpSaiGetDefaultL2McGroupId();
    }

    xpsRetVal = xpsVlanSetFloodL2InterfaceList(xpsDevId, bdId, listId);
    if (XP_NO_ERR != xpsRetVal)
    {
        if (listOid != SAI_NULL_OBJECT_ID)
        {
            xpSaiDeActivateVlanL2McGroup(xpsDevId, bridgeOid, listOid);
        }

        XP_SAI_LOG_ERR("xpsVlanSetFloodL2InterfaceList() failed. bridgeOid = %lu, listId = %d, retVal = %d \n",
                       bridgeOid, listId, retVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return retVal;
}

static sai_status_t xpSaiBridgeResetBroadcastFloodControlGroup(
    sai_object_id_t bridgeOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsMcL2InterfaceListId_t listId = 0;
    sai_object_id_t listOid = SAI_NULL_OBJECT_ID;
    xpsVlan_t bdId = xpSaiObjIdValueGet(bridgeOid);

    xpsRetVal = xpsVlanGetFloodL2InterfaceList(xpsDevId, bdId, &listId);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanGetFloodL2InterfaceList() failed bdId = %u, retVal = %d\n",
                       bdId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    retVal = xpSaiGetL2McGroupIdOid(xpsDevId, listId, bdId, &listOid);
    if (SAI_STATUS_INVALID_PARAMETER == retVal)
    {
        // L2 MC group is not set on this bdId. Just return success
        return SAI_STATUS_SUCCESS;
    }
    else if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("Couldn't get SAI L2MC GROUP OBJID for bdId = %d, listId = %u, retVal = %d.\n",
                       bdId, listId, retVal);
        return retVal;
    }

    retVal = xpSaiDeActivateVlanL2McGroup(xpsDevId, bridgeOid, listOid);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("xpSaiDeActivateVlanL2McGroup() failed. listOid = %lu, retVal = %d \n",
                       listOid, retVal);
        return retVal;
    }

    xpsRetVal = xpsVlanResetFloodL2InterfaceList(xpsDevId, bdId);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanResetFloodL2InterfaceList() failed. bdId = %d, retVal = %d \n",
                       bdId, retVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return retVal;
}

static sai_status_t xpSaiBridgeSetUnregMulicastFloodControlGroup(
    sai_object_id_t bridgeOid, sai_object_id_t listOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsVlan_t bdId = xpSaiObjIdValueGet(bridgeOid);
    xpsMcL2InterfaceListId_t listId = 0;
    xpsMcL2InterfaceListId_t oldListId = 0;

    if (listOid != SAI_NULL_OBJECT_ID)
    {
        /* Get current listId */
        xpsRetVal = xpsVlanGetIPv4UnregMcastL2InterfaceList(xpsDevId, bdId, &oldListId);
        if (XP_NO_ERR != xpsRetVal)
        {
            XP_SAI_LOG_ERR("Could not get current L2MC list id: xpsVlanGetIPv4UnregMcastL2InterfaceList() failed. xpsDevId = %d, bdId = %d, retVal = %d \n",
                           xpsDevId, bdId, retVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }

        retVal = xpSaiActivateBridgeL2McGroup(xpsDevId, bridgeOid, listOid,
                                              (uint32_t*) &listId);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiActivateBridgeL2McGroup() failed. bdId = %d, listOid = %lu, retVal = %d \n",
                           bdId, listOid, retVal);
            return retVal;
        }
    }
    else
    {
        listId = xpSaiGetDefaultL2McGroupId();
    }

    xpsRetVal = xpsVlanSetIPv4UnregMcastL2InterfaceList(xpsDevId, bdId, listId);
    if (XP_NO_ERR != xpsRetVal)
    {
        xpSaiDeActivateVlanL2McGroup(xpsDevId, bridgeOid, listOid);
        XP_SAI_LOG_ERR("xpsVlanSetIPv4UnregMcastL2InterfaceList() failed. bdId = %d, listId = %d, retVal = %d \n",
                       bdId, listId, retVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    xpsRetVal = xpsVlanSetIPv6UnregMcastL2InterfaceList(xpsDevId, bdId, listId);
    if (XP_NO_ERR != xpsRetVal)
    {
        /* Set previous value of IPv4 MC Interface list*/
        xpsVlanSetIPv4UnregMcastL2InterfaceList(xpsDevId, bdId, oldListId);

        // Deactivate L2 MC group if it's not default group
        if (listOid != SAI_NULL_OBJECT_ID)
        {
            xpSaiDeActivateVlanL2McGroup(xpsDevId, bridgeOid, listOid);
        }

        XP_SAI_LOG_ERR("xpsVlanSetIPv6UnregMcastL2InterfaceList() failed. bdId = %d, listId = %d, retVal = %d \n",
                       bdId, listId, retVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return retVal;
}

static sai_status_t xpSaiBridgeResetUnregMulicastFloodControlGroup(
    sai_object_id_t bridgeOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsMcL2InterfaceListId_t listId = 0;
    sai_object_id_t listOid = SAI_NULL_OBJECT_ID;
    xpsVlan_t bdId = xpSaiObjIdValueGet(bridgeOid);
    uint32_t defaultListId = xpSaiGetDefaultL2McGroupId();

    xpsRetVal = xpsVlanGetIPv4UnregMcastL2InterfaceList(xpsDevId, bdId, &listId);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanGetFloodL2InterfaceList() failed bdId = %u, retVal = %d\n",
                       bdId, xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (defaultListId != listId)
    {
        retVal = xpSaiGetL2McGroupIdOid(xpsDevId, listId, bdId, &listOid);
        if (SAI_STATUS_INVALID_PARAMETER == retVal)
        {
            // L2 MC group is not set on this bdId. Just return success
            return SAI_STATUS_SUCCESS;
        }
        else if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("Couldn't get SAI L2MC GROUP OBJID for bdId = %d, listId = %u, retVal = %d.\n",
                           bdId, listId, retVal);
            return retVal;
        }

        retVal = xpSaiDeActivateVlanL2McGroup(xpsDevId, bridgeOid, listOid);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiDeActivateVlanL2McGroup() failed. listOid = %d, retVal = %d \n",
                           listOid, retVal);
            return retVal;
        }
    }

    xpsRetVal = xpsVlanResetIPv4UnregMcastL2InterfaceList(xpsDevId, bdId);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanResetIPv4UnregMcastL2InterfaceList() failed. bdId = %d, listId = %d, retVal = %d \n",
                       bdId, listId, retVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    xpsRetVal = xpsVlanResetIPv6UnregMcastL2InterfaceList(xpsDevId, bdId);
    if (XP_NO_ERR != xpsRetVal)
    {
        XP_SAI_LOG_ERR("xpsVlanSetIPv6UnregMcastL2InterfaceList() failed. bdId = %d, listId = %d, retVal = %d \n",
                       bdId, listId, retVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    return retVal;
}

static sai_status_t xpSaiBridgeGetFloodGroup(xpsDevice_t devId, xpsVlan_t bdId,
                                             sai_attribute_t* attr, uint32_t listId, int32_t floodType)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    sai_object_id_t defaultGroupId = xpSaiGetDefaultL2McGroupId();

    if (floodType != SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP ||
        defaultGroupId == listId)
    {
        attr->value.oid = SAI_NULL_OBJECT_ID;
    }
    else
    {
        retVal = xpSaiGetL2McGroupIdOid(devId, listId, bdId, &attr->value.oid);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("Couldn't get SAI L2MC GROUP OBJID for bdId = %d, listId = %u, floodType = %d, retVal = %d.\n",
                           bdId, listId, floodType, retVal);
            return retVal;
        }
    }

    return retVal;
}

//Func: xpSaiBridgeGetAttrPortList

static sai_status_t xpSaiBridgeGetAttrPortList(xpsDevice_t xpsDevId,
                                               xpSaiBridgeInfoT *bridge, sai_attribute_value_t *value)
{
    XP_STATUS            retVal   = XP_NO_ERR;
    xpSaiBrObjsMapInfoT *entry    = NULL;
    uint32_t             iter     = 0;

    XP_SAI_BRIDGE_TRACE();

    if ((NULL == value) || (NULL == bridge))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    /* Search for corresponding object */
    retVal = xpSaiBrMappingGetCtxDb(xpsDevId, bridge->bridgeId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Mapping entry %lu not exist\n", bridge->bridgeId);
        return retVal;
    }

    XP_SAI_LOG_DBG("Bridge %d contains %d ports\n", bridge->bridgeId,
                   entry->numItems);

    if (value->objlist.count < entry->numItems)
    {
        XP_SAI_LOG_DBG("Not enough room for %d ports!\n", entry->numItems);
        value->objlist.count = entry->numItems;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if ((entry->numItems != 0) && (NULL == value->objlist.list))
    {
        XP_SAI_LOG_ERR("Invalid parameters received\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (iter = 0; iter < entry->numItems; ++iter)
    {
        value->objlist.list[iter] = entry->items[iter].bridgePort;
    }

    value->objlist.count = entry->numItems;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBridgeSetLearnDisable

static XP_STATUS xpSaiBridgeSetLearnDisable(xpsDevice_t xpsDevId,
                                            xpsVlan_t bdId, bool mode)
{
    xpsPktCmd_e pktCmd = XP_PKTCMD_MAX; // enable, by default

    XP_SAI_BRIDGE_TRACE();

    /*Disable, based on config*/
    if (true == mode)
    {
        pktCmd = XP_PKTCMD_FWD;
    }

    return xpsVlanSetUnknownSaCmd(xpsDevId, bdId, pktCmd);
}

//Func: xpSaiBridgeGetLearnDisable

static XP_STATUS xpSaiBridgeGetLearnDisable(xpsDevice_t xpsDevId,
                                            xpsVlan_t bdId, bool *mode)
{
    XP_STATUS   retVal = XP_NO_ERR;
    xpsPktCmd_e pktCmd = XP_PKTCMD_MAX;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == mode)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsVlanGetUnknownSaCmd(xpsDevId, bdId, &pktCmd);
    if (XP_NO_ERR != retVal)
    {
        XP_SAI_LOG_ERR("Failed to get learning options for bridge %u, error: %d\n",
                       bdId, retVal);
        return retVal;
    }

    *mode = (pktCmd == XP_PKTCMD_FWD);

    return  retVal;
}

// Func: xpSaiBridgeHandleCreate

static XP_STATUS xpSaiBridgeHandleCreate(xpsDevice_t xpsDevId,
                                         xpSaiBridgeAttributesT *attributes, sai_object_id_t *sai_object_id)
{
    XP_STATUS          retVal     = XP_NO_ERR;
    sai_status_t       saiStatus  = SAI_STATUS_SUCCESS;
    xpsVlan_t          bdId       = 0;
    xpSaiBridgeInfoT  *bridge     = NULL;

    XP_SAI_BRIDGE_TRACE();

    if ((NULL == attributes) || (NULL == sai_object_id))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    if (SAI_BRIDGE_TYPE_1Q == attributes->bridgeType)
    {
        bdId = 0; // do not use allocator for .1Q bridge
    }
    else // SAI_BRIDGE_TYPE_1D
    {
        // allocate new BD
        retVal = xpsVlanCreateBd(xpsDevId, &bdId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Unable to create .1D Bridge, error: %d\n", retVal);
            return retVal;
        }
    }

    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_BRIDGE, xpsDevId,
                                 (sai_uint64_t)bdId, sai_object_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to create SAI object, error: %d\n", saiStatus);
        return XP_ERR_INVALID_ID;
    }

    retVal = xpSaiBridgeInsertCtxDb(xpsDevId, *sai_object_id, &bridge);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to allocate XPS state Heap\n");
        return retVal;
    }

    bridge->bridgeType    = attributes->bridgeType;
    bridge->bdId          = bdId;
    bridge->unUcFloodCtrl = (sai_bridge_flood_control_type_t)
                            attributes->unUcFloodCtrl.s32;
    bridge->unUcFloodGrp  = attributes->unUcFloodGrp.oid;
    bridge->unMcFloodCtrl = (sai_bridge_flood_control_type_t)
                            attributes->unMcFloodCtrl.s32;
    bridge->unMcFloodGrp  = attributes->unMcFloodGrp.oid;
    bridge->bcFloodCtrl   = (sai_bridge_flood_control_type_t)
                            attributes->bcFloodCtrl.s32;
    bridge->bcFloodGrp    = attributes->bcFloodGrp.oid;

    // reserve bridge ports holder
    retVal = xpSaiBridgeCreateMappingEntry(*sai_object_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to create mapping entry\n");
        return retVal;
    }

    // handle bridge options
    if (SAI_BRIDGE_TYPE_1D == bridge->bridgeType)
    {
        bool cpuFloodEnable = false;
        xpsVlanBridgeMcMode_e vlanBrMcMode = MC_BRIDGE_MODE_FDB;

        retVal = xpsVlanSetFdbLimit(xpsDevId, bdId,
                                    attributes->maxLearnedAddresses.u32);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Unable to set FdbLimit on a bridge %d, error: %d\n", bdId,
                           retVal);
            return retVal;
        }

        retVal = xpSaiBridgeSetLearnDisable(xpsDevId, bdId,
                                            attributes->learnDisable.booldata);
        if (XP_NO_ERR != retVal)
        {
            XP_SAI_LOG_ERR("Unable to set learning options on a bridge %u, error: %d\n",
                           bdId, retVal);
            return  retVal;
        }

        /* Check if unknown unicast or broadcast flood control to CPU port is enabled */
        saiStatus = xpSaiGetSwitchAttrBcastCpuFloodEnable(xpsDevId, &cpuFloodEnable);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiGetSwitchAttrBcastCpuFloodEnable() failed saiStatus: %d\n",
                           saiStatus);
            return XP_ERR_INVALID_ID;
        }

        saiStatus = xpSaiBridgeSetAttrUnknownUnicastFloodControlType(bdId,
                                                                     attributes->unUcFloodCtrl.s32, cpuFloodEnable);
        if (SAI_STATUS_SUCCESS != saiStatus)
        {
            XP_SAI_LOG_ERR("xpSaiBridgeSetAttrUnknownUnicastFloodControlType failed. bridge->bdId = %d, saiStatus = %d \n",
                           bridge->bdId, saiStatus);
            return XP_ERR_INVALID_ID;
        }

        saiStatus = xpSaiBridgeSetAttrBroadcastFloodControlType(bdId,
                                                                attributes->bcFloodCtrl.s32, cpuFloodEnable);
        if (SAI_STATUS_SUCCESS != saiStatus)
        {
            XP_SAI_LOG_ERR("xpSaiBridgeSetAttrBroadcastFloodControlType failed. bridge->bdId = %d, saiStatus = %d \n",
                           bridge->bdId, saiStatus);
            return XP_ERR_INVALID_ID;
        }

        /* Check if unknown multicast flood control to CPU port is enabled */
        saiStatus = xpSaiGetSwitchAttrMcastCpuFloodEnable(xpsDevId, &cpuFloodEnable);
        if (SAI_STATUS_SUCCESS != saiStatus)
        {
            XP_SAI_LOG_ERR("xpSaiGetSwitchAttrMcastCpuFloodEnable() failed saiStatus: %d\n",
                           saiStatus);
            return XP_ERR_INVALID_ID;
        }

        saiStatus = xpSaiBridgeSetAttrUnknownMulticastFloodControlType(bdId,
                                                                       attributes->unMcFloodCtrl.s32, cpuFloodEnable);
        if (SAI_STATUS_SUCCESS != saiStatus)
        {
            XP_SAI_LOG_ERR("xpSaiBridgeSetAttrUnknownMulticastFloodControlType failed. bridge->bdId = %d, saiStatus = %d \n",
                           bridge->bdId, saiStatus);
            return XP_ERR_INVALID_ID;
        }

        retVal = xpsVlanSetIpv4McBridgeMode(xpsDevId, bdId, vlanBrMcMode);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpsVlanSetIpv4McBridgeMode failed. vlanBrMcMode = %d, bdId = %d, retVal = %d \n",
                           vlanBrMcMode, bdId, retVal);
            return retVal;
        }

        retVal = xpsVlanSetIpv6McBridgeMode(xpsDevId, bdId, vlanBrMcMode);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpsVlanSetIpv6McBridgeMode failed. vlanBrMcMode = %d, bdId = %d, retVal = %d \n",
                           vlanBrMcMode, bdId, retVal);
            return retVal;
        }
    }
    else // SAI_BRIDGE_TYPE_1Q
    {
        /// TODO: params handler
    }

    return retVal;
}

// Func: xpSaiBridgeHandleRemove

static XP_STATUS xpSaiBridgeHandleRemove(xpsDevice_t xpsDevId,
                                         sai_object_id_t bridgeId)
{
    XP_STATUS            retVal = XP_NO_ERR;
    xpSaiBridgeInfoT    *bridge = NULL;
    xpSaiBrObjsMapInfoT *entry  = NULL;

    XP_SAI_BRIDGE_TRACE();

    if (SAI_NULL_OBJECT_ID == bridgeId)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_INVALID_INPUT;
    }

    /* Search corresponding table API */
    retVal = xpSaiBridgeGetCtxDb(xpsDevId, bridgeId, &bridge);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for bridgeId %lu\n", bridgeId);
        return retVal;
    }

    /* Search for corresponding object */
    retVal = xpSaiBrMappingGetCtxDb(xpsDevId, bridgeId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Mapping entry %llu not exist\n", bridgeId);
        return retVal;
    }

    /* Can't remove bridge without removing bridgePort*/
    if (entry->numItems != 0)
    {
        XP_SAI_LOG_ERR("Failed to remove bridgeId %llu\n", bridgeId);
        return XP_ERR_RESOURCE_BUSY;
    }

    if (SAI_BRIDGE_TYPE_1D == bridge->bridgeType)
    {
        retVal = xpsVlanDestroy(xpsDevId, bridge->bdId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Unable to remove .1D Bridge %d from device %d, error: %d\n",
                           bridge->bdId, xpsDevId, retVal);
            return retVal;
        }
    }

    retVal = xpSaiBridgeRemoveMappingEntry(bridgeId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to remove mapping entry\n");
        return retVal;
    }

    retVal = xpSaiBridgeRemoveCtxDb(xpsDevId, bridgeId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to deallocate XPS state entry\n");
        return retVal;
    }

    return retVal;
}

//Func: xpSaiSetDefaultBridgeAttributeVals

static void xpSaiSetDefaultBridgeAttributeVals(xpSaiBridgeAttributesT
                                               *attributes)
{
    XP_SAI_BRIDGE_TRACE();

    if (NULL == attributes)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return;
    }

    memset(attributes, 0, sizeof(xpSaiBridgeAttributesT));
    attributes->bridgeType              = SAI_BRIDGE_TYPE_1Q;
    attributes->maxLearnedAddresses.u32 = 0; // no limit
    attributes->learnDisable.booldata   = false; // learning is enabled
    attributes->unUcFloodCtrl.s32       = SAI_BRIDGE_FLOOD_CONTROL_TYPE_SUB_PORTS;
    attributes->unUcFloodGrp.oid        = SAI_NULL_OBJECT_ID;
    attributes->unMcFloodCtrl.s32       = SAI_BRIDGE_FLOOD_CONTROL_TYPE_SUB_PORTS;
    attributes->unMcFloodGrp.oid        = SAI_NULL_OBJECT_ID;
    attributes->bcFloodCtrl.s32         = SAI_BRIDGE_FLOOD_CONTROL_TYPE_SUB_PORTS;
    attributes->bcFloodGrp.oid          = SAI_NULL_OBJECT_ID;
}

//Func: xpSaiUpdateBridgeAttributeVals

static XP_STATUS xpSaiUpdateBridgeAttributeVals(const uint32_t attr_count,
                                                const sai_attribute_t *attr_list, xpSaiBridgeAttributesT *attributes)
{
    uint32_t               count  = 0;
    const sai_attribute_t *attr   = NULL;

    XP_SAI_BRIDGE_TRACE();

    if ((NULL == attr_list) || (NULL == attributes))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    attr = xpSaiFindAttrById(SAI_BRIDGE_ATTR_TYPE, attr_count, attr_list, NULL);
    if (NULL == attr)
    {
        XP_SAI_LOG_ERR("Mandatory parameter doesn't exists.\n");
        return XP_ERR_INVALID_ARG;
    }

    for (count = 0; count < attr_count; ++count)
    {
        const sai_attribute_value_t *value = &attr_list[count].value;

        switch (attr_list[count].id)
        {
            case SAI_BRIDGE_ATTR_TYPE:
                {
                    attributes->bridgeType = (sai_bridge_type_t)value->u32;
                    break;
                }
            case SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES:
                {
                    attributes->maxLearnedAddresses = *value;
                    break;
                }
            case SAI_BRIDGE_ATTR_LEARN_DISABLE:
                {
                    attributes->learnDisable = *value;
                    break;
                }
            case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
                {
                    attributes->unUcFloodCtrl = *value;
                    break;
                }
            case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
                {
                    attributes->unUcFloodGrp = *value;
                    break;
                }
            case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
                {
                    attributes->unMcFloodCtrl = *value;
                    break;
                }
            case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
                {
                    attributes->unMcFloodGrp = *value;
                    break;
                }
            case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
                {
                    attributes->bcFloodCtrl = *value;
                    break;
                }
            case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP:
                {
                    attributes->bcFloodGrp = *value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }

    return XP_NO_ERR;
}

//Func: xpSaiGetBridgeAttribute

static sai_status_t xpSaiGetBridgeAttribute(xpsDevice_t xpsDevId,
                                            xpSaiBridgeInfoT *bridge, sai_attribute_t *dest)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS    retVal    = XP_NO_ERR;

    XP_SAI_BRIDGE_TRACE();

    if ((NULL == bridge) || (NULL == dest))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    switch (dest->id)
    {
        case SAI_BRIDGE_ATTR_PORT_LIST:
            {
                saiRetVal = xpSaiBridgeGetAttrPortList(xpsDevId, bridge, &dest->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    if (saiRetVal != SAI_STATUS_BUFFER_OVERFLOW)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_BRIDGE_ATTR_PORT_LIST), error: %d\n",
                                       saiRetVal);
                    }
                    return saiRetVal;
                }
                break;
            }
        case SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES:
            {
                if (SAI_BRIDGE_TYPE_1Q == bridge->bridgeType)
                {
                    saiRetVal = SAI_STATUS_ATTR_NOT_IMPLEMENTED_0;
                }
                else
                {
                    retVal = xpsVlanGetFdbLimit(xpsDevId, bridge->bdId, &dest->value.u32);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES), error: %d\n",
                                       retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }
                    saiRetVal = xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_BRIDGE_ATTR_LEARN_DISABLE:
            {
                if (SAI_BRIDGE_TYPE_1Q == bridge->bridgeType)
                {
                    saiRetVal = SAI_STATUS_ATTR_NOT_IMPLEMENTED_0;
                }
                else
                {
                    retVal = xpSaiBridgeGetLearnDisable(xpsDevId, bridge->bdId,
                                                        &dest->value.booldata);
                    if (XP_NO_ERR != retVal)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_BRIDGE_ATTR_LEARN_DISABLE) for bridge %u, error: %d\n",
                                       bridge->bdId, retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }
                    saiRetVal = xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
            {
                if (bridge->bridgeType == SAI_BRIDGE_TYPE_1Q)
                {
                    XP_SAI_LOG_INFO("Get of SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE attribute for .1Q bridge is not supported.");
                    return SAI_STATUS_NOT_SUPPORTED;
                }
                dest->value.s32 = bridge->unUcFloodCtrl;
                break;
            }
        case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
            {
                xpsMcL2InterfaceListId_t listId = 0;

                if (bridge->bridgeType == SAI_BRIDGE_TYPE_1Q)
                {
                    XP_SAI_LOG_INFO("Get of SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP attribute for .1Q bridge is not supported.");
                    return SAI_STATUS_NOT_SUPPORTED;
                }

                retVal = xpsVlanGetFloodL2InterfaceList(xpsDevId, (xpsVlan_t)bridge->bdId,
                                                        &listId);
                if (XP_NO_ERR != retVal)
                {
                    XP_SAI_LOG_ERR("xpsVlanGetFloodL2InterfaceList() failed bdId = %u, retVal = %d\n",
                                   bridge->bdId, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                saiRetVal = xpSaiBridgeGetFloodGroup(xpsDevId, bridge->bdId, dest, listId,
                                                     bridge->unUcFloodCtrl);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("xpSaiBridgeGetFloodGroup failed bdId = %u, bdId = %u, floodType = %u, saiRetVal : %d\n",
                                   bridge->bdId, listId, bridge->unUcFloodCtrl, saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
            {
                if (bridge->bridgeType == SAI_BRIDGE_TYPE_1Q)
                {
                    XP_SAI_LOG_INFO("Get of SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE attribute for .1Q bridge is not supported.");
                    return SAI_STATUS_NOT_SUPPORTED;
                }

                dest->value.s32 = bridge->unMcFloodCtrl;
                break;
            }
        case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
            {
                xpsMcL2InterfaceListId_t listId = 0;

                if (bridge->bridgeType == SAI_BRIDGE_TYPE_1Q)
                {
                    XP_SAI_LOG_INFO("Get of SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP attribute for .1Q bridge is not supported.");
                    return SAI_STATUS_NOT_SUPPORTED;
                }

                retVal = xpsVlanGetIPv4UnregMcastL2InterfaceList(xpsDevId,
                                                                 (xpsVlan_t)bridge->bdId, &listId);
                if (XP_NO_ERR != retVal)
                {
                    XP_SAI_LOG_ERR("xpsVlanGetIPv4UnregMcastL2InterfaceList() failed bdId = %u, retVal = %d\n",
                                   bridge->bdId, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                saiRetVal = xpSaiBridgeGetFloodGroup(xpsDevId, bridge->bdId, dest, listId,
                                                     bridge->unMcFloodCtrl);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("xpSaiGetVlanFloodGroup failed bdId = %u, listId = %u, floodType = %u, saiRetVal : %d\n",
                                   bridge->bdId, listId, bridge->unMcFloodCtrl, saiRetVal);
                    return saiRetVal;
                }

                dest->value.oid = bridge->unMcFloodGrp;
                break;
            }
        case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
            {
                if (bridge->bridgeType == SAI_BRIDGE_TYPE_1Q)
                {
                    XP_SAI_LOG_INFO("Get of SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE attribute for .1Q bridge is not supported.");
                    return SAI_STATUS_NOT_SUPPORTED;
                }

                dest->value.s32 = bridge->bcFloodCtrl;
                break;
            }
        case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP:
            {
                xpsMcL2InterfaceListId_t listId = 0;

                if (bridge->bridgeType == SAI_BRIDGE_TYPE_1Q)
                {
                    XP_SAI_LOG_INFO("Get of SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP attribute for .1Q bridge is not supported.");
                    return SAI_STATUS_NOT_SUPPORTED;
                }

                retVal = xpsVlanGetFloodL2InterfaceList(xpsDevId, (xpsVlan_t)bridge->bdId,
                                                        &listId);
                if (XP_NO_ERR != retVal)
                {
                    XP_SAI_LOG_ERR("xpsVlanGetFloodL2InterfaceList() failed bdId = %u, retVal = %d\n",
                                   bridge->bdId, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                saiRetVal = xpSaiBridgeGetFloodGroup(xpsDevId, bridge->bdId, dest, listId,
                                                     bridge->bcFloodCtrl);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("xpSaiBridgeGetFloodGroup failed bridge->bdId = %u, listId = %u, floodType = %u, saiRetVal : %d\n",
                                   bridge->bdId, listId, bridge->bcFloodCtrl, saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", dest->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    return saiRetVal;
}

//Func: xpSaiGetBridgeAttributes

sai_status_t xpSaiGetBridgeAttributes(sai_object_id_t bridge_id,
                                      uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t      saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t       xpsDevId  = xpSaiObjIdSwitchGet(bridge_id);
    XP_STATUS         xpsRetVal = XP_NO_ERR;
    xpSaiBridgeInfoT *bridge    = NULL;
    uint32_t          count     = 0;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == attr_list)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list, BRIDGE_VALIDATION_ARRAY_SIZE,
                               bridge_attribs, SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to check attribute, error: %d\n", saiRetVal);
        return saiRetVal;
    }

    /* Fetch from state DB */
    xpsRetVal = xpSaiBridgeGetCtxDb(xpsDevId, bridge_id, &bridge);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for bridgeId %llu\n", bridge_id);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    for (count = 0; count < attr_count; ++count)
    {
        saiRetVal = xpSaiGetBridgeAttribute(xpsDevId, bridge, &attr_list[count]);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            if (XPSAI_CHECK_ERROR_OFFSET(saiRetVal))
            {
                saiRetVal += count;
            }

            if (saiRetVal != SAI_STATUS_BUFFER_OVERFLOW)
            {
                XP_SAI_LOG_ERR("xpSaiGetBridgeAttribute failed\n");
            }
            return saiRetVal;
        }
    }

    return saiRetVal;
}

//Func: xpSaiSetBridgeAttribute

static sai_status_t xpSaiSetBridgeAttribute(sai_object_id_t bridge_id,
                                            const sai_attribute_t *attr)
{
    sai_status_t      saiRetVal   = SAI_STATUS_SUCCESS;
    xpsDevice_t       xpsDevId    = xpSaiObjIdSwitchGet(bridge_id);
    XP_STATUS         retVal      = XP_NO_ERR;
    xpSaiBridgeInfoT *bridge      = NULL;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == attr)
    {
        XP_SAI_LOG_ERR("Invalid parameter received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(1, attr, BRIDGE_VALIDATION_ARRAY_SIZE,
                               bridge_attribs, SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to check attribute, error: %d\n", saiRetVal);
        return saiRetVal;
    }

    /* Fetch from state DB */
    retVal = xpSaiBridgeGetCtxDb(xpsDevId, bridge_id, &bridge);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for bridgeId %llu\n", bridge_id);
        return xpsStatus2SaiStatus(retVal);
    }
    saiRetVal = xpsStatus2SaiStatus(retVal);

    switch (attr->id)
    {
        case SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES:
            {
                if (SAI_BRIDGE_TYPE_1Q == bridge->bridgeType)
                {
                    saiRetVal = SAI_STATUS_ATTR_NOT_IMPLEMENTED_0;
                }
                else
                {
                    retVal = xpsVlanSetFdbLimit(xpsDevId, (xpsVlan_t)bridge->bridgeId,
                                                attr->value.u32);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to set (SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES), error: %d\n",
                                       retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }
                    saiRetVal = xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_BRIDGE_ATTR_LEARN_DISABLE:
            {
                if (SAI_BRIDGE_TYPE_1Q == bridge->bridgeType)
                {
                    saiRetVal = SAI_STATUS_ATTR_NOT_IMPLEMENTED_0;
                }
                else
                {
                    retVal = xpSaiBridgeSetLearnDisable(xpsDevId, bridge->bdId,
                                                        attr->value.booldata);
                    if (XP_NO_ERR != retVal)
                    {
                        XP_SAI_LOG_ERR("Failed to set (SAI_BRIDGE_ATTR_LEARN_DISABLE) on a bridge %u, error: %d\n",
                                       bridge->bdId, retVal);
                        return  xpsStatus2SaiStatus(retVal);
                    }
                    saiRetVal = xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
            {
                bool cpuFloodEnable = false;

                if (bridge->bridgeType == SAI_BRIDGE_TYPE_1Q)
                {
                    XP_SAI_LOG_ERR("Set of SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE attribute for .1Q bridge is not supported.");
                    return SAI_STATUS_NOT_SUPPORTED;
                }

                if (bridge->unUcFloodCtrl == attr->value.s32)
                {
                    XP_SAI_LOG_INFO("SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE is already %d\n",
                                    attr->value.s32);
                    break;
                }

                /* Check if unknown unicast flood control to CPU port is enabled */
                saiRetVal = xpSaiGetSwitchAttrBcastCpuFloodEnable(xpsDevId, &cpuFloodEnable);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiGetSwitchAttrBcastCpuFloodEnable() failed retVal: %d\n",
                                   retVal);
                    return saiRetVal;
                }

                /* In case when we need to set flooding type as L2MC group - set empty group by default (SAI_NULL_OBJECT_ID)
                   and reset flood group to default BD in other cases */
                if (SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP == attr->value.s32)
                {
                    saiRetVal = xpSaiBridgeSetBroadcastFloodControlGroup(bridge_id,
                                                                         SAI_NULL_OBJECT_ID);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiBridgeSetBroadcastFloodControlGroup failed: bridge_id = %lu, saiRetVal = %d.\n",
                                       bridge_id, saiRetVal);
                        return saiRetVal;
                    }
                }
                else
                {
                    saiRetVal = xpSaiBridgeResetBroadcastFloodControlGroup(bridge_id);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiBridgeResetBroadcastFloodControlGroup failed: bridge_id = %lu, saiRetVal = %d.\n",
                                       bridge_id, saiRetVal);
                        return saiRetVal;
                    }
                }

                saiRetVal = xpSaiBridgeSetAttrUnknownUnicastFloodControlType(bridge->bdId,
                                                                             attr->value.s32, cpuFloodEnable);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("xpSaiBridgeSetAttrUnknownUnicastFloodControlType failed. bridge->bdId = %d, saiRetVal = %d \n",
                                   bridge->bdId, saiRetVal);
                    return saiRetVal;
                }

                bridge->unUcFloodCtrl = (sai_bridge_flood_control_type_t)attr->value.s32;
                break;
            }
        case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
            {
                if (bridge->bridgeType == SAI_BRIDGE_TYPE_1Q)
                {
                    XP_SAI_LOG_ERR("Set of SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP attribute for .1Q bridge is not supported.");
                    return SAI_STATUS_NOT_SUPPORTED;
                }

                if (bridge->unUcFloodCtrl == SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP)
                {
                    saiRetVal = xpSaiBridgeSetBroadcastFloodControlGroup(bridge_id,
                                                                         attr->value.oid);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiSetVlanBroadcastFloodControlGroup failed: bridge_id = %lu, retVal = %d.\n",
                                       bridge_id, saiRetVal);
                        return saiRetVal;
                    }
                }
                else
                {
                    XP_SAI_LOG_ERR("Unicast flood control type should be set to SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP\n");
                    return SAI_STATUS_INVALID_PARAMETER;
                }

                bridge->unUcFloodGrp = attr->value.oid;
                break;
            }
        case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
            {
                bool cpuFloodEnable = false;

                if (bridge->bridgeType == SAI_BRIDGE_TYPE_1Q)
                {
                    XP_SAI_LOG_ERR("Set of SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE attribute for .1Q bridge is not supported.");
                    return SAI_STATUS_NOT_SUPPORTED;
                }

                if (bridge->unMcFloodCtrl == attr->value.s32)
                {
                    XP_SAI_LOG_INFO("SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE is already %d\n",
                                    attr->value.s32);
                    break;
                }

                /* Check if unknown multicast flood control to CPU port is enabled */
                saiRetVal = xpSaiGetSwitchAttrMcastCpuFloodEnable(xpsDevId, &cpuFloodEnable);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiGetSwitchAttrMcastCpuFloodEnable() failed retVal: %d\n",
                                   retVal);
                    return saiRetVal;
                }

                /* In case when we need to set flooding type as L2MC group - set empty group by default (SAI_NULL_OBJECT_ID)
                   and reset flood group to default BD in other cases */
                if (SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP == attr->value.s32)
                {
                    saiRetVal = xpSaiBridgeSetUnregMulicastFloodControlGroup(bridge_id,
                                                                             SAI_NULL_OBJECT_ID);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiBridgeSetBroadcastFloodControlGroup failed: bridge_id = %lu, saiRetVal = %d.\n",
                                       bridge_id, saiRetVal);
                        return saiRetVal;
                    }
                }
                else
                {
                    saiRetVal = xpSaiBridgeResetUnregMulicastFloodControlGroup(bridge_id);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiBridgeResetBroadcastFloodControlGroup failed: bridge_id = %lu, saiRetVal = %d.\n",
                                       bridge_id, saiRetVal);
                        return saiRetVal;
                    }
                }

                saiRetVal = xpSaiBridgeSetAttrUnknownMulticastFloodControlType(bridge->bdId,
                                                                               attr->value.s32, cpuFloodEnable);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("xpSaiBridgeSetAttrUnknownMulticastFloodControlType failed. bridge->bdId = %d, saiRetVal = %d \n",
                                   bridge->bdId, saiRetVal);
                    return saiRetVal;
                }

                bridge->unMcFloodCtrl = (sai_bridge_flood_control_type_t)attr->value.s32;
                break;
            }
        case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
            {
                if (bridge->bridgeType == SAI_BRIDGE_TYPE_1Q)
                {
                    XP_SAI_LOG_ERR("Set of SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP attribute for .1Q bridge is not supported.");
                    return SAI_STATUS_NOT_SUPPORTED;
                }

                if (bridge->unMcFloodCtrl == SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP)
                {
                    saiRetVal = xpSaiBridgeSetUnregMulicastFloodControlGroup(bridge_id,
                                                                             attr->value.oid);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiBridgeSetUnregMulicastFloodControlGroup failed: bridge_id = %lu, saiRetVal = %d.\n",
                                       bridge_id, saiRetVal);
                        return saiRetVal;
                    }
                }
                else
                {
                    XP_SAI_LOG_ERR("Unregistered multicast flood control type should be set to SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP\n");
                    return SAI_STATUS_INVALID_PARAMETER;
                }

                bridge->unMcFloodGrp = attr->value.oid;
                break;
            }
        case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
            {
                bool cpuFloodEnable = false;

                if (bridge->bridgeType == SAI_BRIDGE_TYPE_1Q)
                {
                    XP_SAI_LOG_ERR("Set of SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE attribute for .1Q bridge is not supported.");
                    return SAI_STATUS_NOT_SUPPORTED;
                }

                if (bridge->bcFloodCtrl == attr->value.s32)
                {
                    XP_SAI_LOG_INFO("SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE is already %d\n",
                                    attr->value.s32);
                    break;
                }

                /* Check if broadcast flood control to CPU port is enabled */
                saiRetVal = xpSaiGetSwitchAttrBcastCpuFloodEnable(xpsDevId, &cpuFloodEnable);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiGetSwitchAttrBcastCpuFloodEnable() failed retVal: %d\n",
                                   retVal);
                    return saiRetVal;
                }

                /* In case when we need to set flooding type as L2MC group - set empty group by default (SAI_NULL_OBJECT_ID)
                   and reset flood group to default BD in other cases */
                if (SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP == attr->value.s32)
                {
                    saiRetVal = xpSaiBridgeSetBroadcastFloodControlGroup(bridge_id,
                                                                         SAI_NULL_OBJECT_ID);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiBridgeSetBroadcastFloodControlGroup failed: bridge_id = %lu, saiRetVal = %d.\n",
                                       bridge_id, saiRetVal);
                        return saiRetVal;
                    }
                }
                else
                {
                    saiRetVal = xpSaiBridgeResetBroadcastFloodControlGroup(bridge_id);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiBridgeResetBroadcastFloodControlGroup failed: bridge_id = %lu, saiRetVal = %d.\n",
                                       bridge_id, saiRetVal);
                        return saiRetVal;
                    }
                }

                saiRetVal = xpSaiBridgeSetAttrBroadcastFloodControlType(bridge->bdId,
                                                                        attr->value.s32, cpuFloodEnable);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("xpSaiBridgeSetAttrBroadcastFloodControlType failed. bridge->bdId = %d, saiRetVal = %d \n",
                                   bridge->bdId, saiRetVal);
                    return saiRetVal;
                }

                bridge->bcFloodCtrl = (sai_bridge_flood_control_type_t)attr->value.s32;
                break;
            }
        case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP:
            {
                if (bridge->bridgeType == SAI_BRIDGE_TYPE_1Q)
                {
                    XP_SAI_LOG_ERR("Set of SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP attribute for .1Q bridge is not supported.");
                    return SAI_STATUS_NOT_SUPPORTED;
                }

                if (bridge->bcFloodCtrl == SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP)
                {
                    saiRetVal = xpSaiBridgeSetBroadcastFloodControlGroup(bridge_id,
                                                                         attr->value.oid);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("xpSaiSetVlanBroadcastFloodControlGroup failed: bridge_id = %lu, saiRetVal = %d.\n",
                                       bridge_id, saiRetVal);
                        return saiRetVal;
                    }
                }
                else
                {
                    XP_SAI_LOG_ERR("Broadcast flood control type should be set to SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP\n");
                    return SAI_STATUS_INVALID_PARAMETER;
                }

                bridge->bcFloodGrp = attr->value.oid;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    return saiRetVal;
}

//Func: xpSaiCreateBridge

static sai_status_t xpSaiCreateBridge(sai_object_id_t* bridge_id,
                                      sai_object_id_t switch_id,
                                      uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t           saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS              retVal    = XP_NO_ERR;
    xpsDevice_t            xpsDevId  = 0;
    xpSaiBridgeAttributesT attributes;

    XP_SAI_BRIDGE_TRACE();

    if ((NULL == bridge_id) || (NULL == attr_list))
    {
        XP_SAI_LOG_ERR("Invalid parameter received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list, BRIDGE_VALIDATION_ARRAY_SIZE,
                               bridge_attribs, SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return saiRetVal;
    }

    xpSaiSetDefaultBridgeAttributeVals(&attributes);
    xpSaiUpdateBridgeAttributeVals(attr_count, attr_list, &attributes);

    /*get devId from SwitchId*/
    xpsDevId = xpSaiObjIdSwitchGet(switch_id);

    if (SAI_BRIDGE_TYPE_1Q == attributes.bridgeType)
    {
        XP_SAI_LOG_ERR("Creation of .1Q bridge instance is not allowed\n");
        return SAI_STATUS_NOT_SUPPORTED;
    }

    retVal = xpSaiBridgeHandleCreate(xpsDevId, &attributes, bridge_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI object bridge could not be created, error: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiRemoveBridge

sai_status_t xpSaiRemoveBridge(sai_object_id_t bridge_id)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    xpsDevice_t     xpsDevId    = xpSaiObjIdSwitchGet(bridge_id);
    sai_object_id_t defltBridge = SAI_NULL_OBJECT_ID;

    XP_SAI_BRIDGE_TRACE();

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(bridge_id, SAI_OBJECT_TYPE_BRIDGE))
    {
        XP_SAI_LOG_ERR("Wrong object type received (%lu)\n",
                       xpSaiObjIdTypeGet(bridge_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    saiRetVal = xpSaiSwitchDefaultBridgeIdGet(&defltBridge);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get default 1Q bridge ID.\n");
        return saiRetVal;
    }

    if (defltBridge == bridge_id)
    {
        XP_SAI_LOG_ERR("Can not delete default 1Q bridge.\n");
        return SAI_STATUS_OBJECT_IN_USE;
    }

    retVal = xpSaiBridgeHandleRemove(xpsDevId, bridge_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI object bridge could not be deleted, error: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiBridgePortSetAdminState

static XP_STATUS xpSaiBridgePortSetAdminState(xpsDevice_t xpsDevId,
                                              xpSaiBridgePortInfoT *brPort, bool state)
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == brPort)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    /// TODO: handler

    brPort->adminState = state;

    return retVal;
}

//Func: xpSaiBridgePortSetTaggingMode

static sai_status_t xpSaiBridgePortSetTaggingMode(xpsDevice_t xpsDevId,
                                                  xpSaiBridgePortInfoT *brPort, int32_t value)
{
    XP_SAI_LOG_ERR("Setting TAG mode is unsupported! It`s only possible to set it on create routine.\n");
    return SAI_STATUS_NOT_SUPPORTED;
}

//Func: xpSaiBridgePortSetIngressFiltering

static XP_STATUS xpSaiBridgePortSetIngressFiltering(xpsDevice_t xpsDevId,
                                                    xpSaiBridgePortInfoT *brPort, bool state)
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == brPort)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    /// TODO: handler

    brPort->ingressFiltering = state;

    return retVal;
}

//Func: xpSaiBridgePortSetLearningMode

static XP_STATUS xpSaiBridgePortSetLearningMode(xpsDevice_t xpsDevId,
                                                xpSaiBridgePortInfoT *brPort, int32_t mode)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS             retVal    = XP_NO_ERR;
    xpsInterfaceId_t      xpsIntf   = XPS_INTF_INVALID_ID;
    xpsPktCmd_e           pktCmd    = XP_PKTCMD_DROP;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == brPort)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpSaiBridgeLearningModeToPktCmd(mode, &pktCmd);
    if (XP_NO_ERR != retVal)
    {
        return retVal;
    }

    if (SAI_BRIDGE_PORT_TYPE_PORT == brPort->brPortType)
    {
        xpsInterfaceType_e type = XPS_PORT;

        xpsIntf = xpSaiObjIdValueGet(brPort->brPortDescr.objId);

        retVal = xpsInterfaceGetType(xpsIntf, &type);
        if (XP_NO_ERR != retVal)
        {
            XP_SAI_LOG_ERR("Failed to get type for interface %u, error: %d\n", xpsIntf,
                           retVal);
            return retVal;
        }

        if (XPS_LAG == type)
        {
            retVal = xpSaiLagSetSaMissCmd(xpsIntf, pktCmd);
            if (XP_NO_ERR != retVal)
            {
                XP_SAI_LOG_ERR("Failed to set SA miss command on LAG %d, pktCmd %u, error: %d\n",
                               xpsIntf, pktCmd, retVal);
                return retVal;
            }
        }
        else
        {
            retVal = xpsPortSetField(xpsDevId, xpsIntf, XPS_PORT_MAC_SAMISSCMD,
                                     (uint32_t)pktCmd);
            if (XP_NO_ERR != retVal)
            {
                XP_SAI_LOG_ERR("Failed to set SA miss command for Port %u, error: %d\n",
                               xpsIntf, retVal);
                return retVal;
            }
        }
    }
    else if (SAI_BRIDGE_PORT_TYPE_SUB_PORT == brPort->brPortType)
    {
        xpsIntf = xpSaiObjIdValueGet(brPort->brPortId);
        retVal = xpsPortBridgePortSetMacSaMissCmd(xpsDevId, xpsIntf, pktCmd);
        if (XP_NO_ERR != retVal)
        {
            XP_SAI_LOG_ERR("Failed to set SA miss command for Sub-Port %u", xpsIntf);
            return retVal;
        }
    }
    else if (SAI_BRIDGE_PORT_TYPE_TUNNEL == brPort->brPortType)
    {
        saiRetVal = xpSaiTunnelBridgePortSetFdbLearningMode(xpsDevId,
                                                            brPort->brPortDescr.objId,
                                                            pktCmd);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("FDB Learning mode in TunnelBridgePort failed, error: %d\n",
                           saiRetVal);
            return XP_ERR_NOT_SUPPORTED;
        }
    }
    else
    {
        XP_SAI_LOG_ERR("Currently learning mode is not supported for port type %d \n",
                       brPort->brPortType);
        return XP_ERR_NOT_SUPPORTED;
    }

    brPort->learnMode = (sai_bridge_port_fdb_learning_mode_t)mode;

    return retVal;
}

//Func: xpSaiBridgePortSetMaxLearnedAddrs

static XP_STATUS xpSaiBridgePortSetMaxLearnedAddrs(xpsDevice_t xpsDevId,
                                                   xpSaiBridgePortInfoT *brPort, uint32_t value)
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == brPort)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    /// TODO: handler

    brPort->maxLearnedAddresses = value;

    return retVal;
}

//Func: xpSaiBridgePortSetLimitAction

static XP_STATUS xpSaiBridgePortSetLimitAction(xpsDevice_t xpsDevId,
                                               xpSaiBridgePortInfoT *brPort, int32_t value)
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == brPort)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    /// TODO: handler

    brPort->learnLimitAction = (sai_packet_action_t)value;

    return retVal;
}

// Func: xpSaiBridgeAddLagRef
static XP_STATUS xpSaiBridgeAddLagRef(xpsInterfaceId_t intfId)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceType_e type = XPS_PORT;

    /* get intfId from bridge intfId */
    if (XPS_INTF_IS_BRIDGE_PORT(intfId))
    {
        intfId = XPS_INTF_MAP_BRIDGE_PORT_TO_INTF(intfId);
    }

    retVal = xpsInterfaceGetType(intfId, &type);
    if (XP_NO_ERR != retVal)
    {
        XP_SAI_LOG_ERR("Failed to get type for interface %u, error: %d\n", intfId,
                       retVal);
        return retVal;
    }

    if (XPS_LAG == type)
    {
        saiRetVal = xpSaiLagRef(intfId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Unable to add LAG reference, sai_error: %d\n", saiRetVal);
            return XP_ERR_INVALID_ID;
        }
    }

    return XP_NO_ERR;
}

// Func: xpSaiBridgeRemoveLagRef
static XP_STATUS xpSaiBridgeRemoveLagRef(xpsInterfaceId_t intfId)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceType_e type = XPS_PORT;

    /* get intfId from bridge intfId */
    if (XPS_INTF_IS_BRIDGE_PORT(intfId))
    {
        intfId = XPS_INTF_MAP_BRIDGE_PORT_TO_INTF(intfId);
    }

    retVal = xpsInterfaceGetType(intfId, &type);
    if (XP_NO_ERR != retVal)
    {
        XP_SAI_LOG_ERR("Failed to get type for interface %u, error: %d\n", intfId,
                       retVal);
        return retVal;
    }

    if (XPS_LAG == type)
    {
        saiRetVal = xpSaiLagUnref(intfId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Unable to remove LAG reference, sai_error: %d\n", saiRetVal);
            return XP_ERR_INVALID_ID;
        }
    }

    return XP_NO_ERR;
}

// Func: xpSaiBridgePortHandleCreate

static XP_STATUS xpSaiBridgePortHandleCreate(xpsDevice_t xpsDevId,
                                             xpSaiBridgePortAttributesT *attributes, sai_object_id_t *sai_object_id)
{
    XP_STATUS             retVal    = XP_NO_ERR;
    sai_status_t          saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t      intfId    = XPS_INTF_INVALID_ID;
    xpSaiBridgeInfoT     *bridge    = NULL;
    xpSaiBridgePortInfoT *brPort    = NULL;
    xpsVlan_t             bdId      = 0;
    uint32_t              localOid  = 0;

    XP_SAI_BRIDGE_TRACE();

    if ((NULL == attributes) || (NULL == sai_object_id))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    /* checks */
    if (SAI_NULL_OBJECT_ID == attributes->brId)
    {
        XP_SAI_LOG_ERR("Mandatory parameter BridgeId not exist.\n");
        return XP_ERR_INVALID_ARG;
    }

    /* Search corresponding state entry */
    retVal = xpSaiBridgeGetCtxDb(xpsDevId, attributes->brId, &bridge);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for bridgeId %llu, error: %d\n",
                       attributes->brId, retVal);
        return retVal;
    }

    /* bridge port ID allocation */
    if (SAI_BRIDGE_PORT_TYPE_TUNNEL == attributes->brPortType)
    {
        if (SAI_NULL_OBJECT_ID == attributes->tunId)
        {
            XP_SAI_LOG_ERR("Invalid args for port type SAI_BRIDGE_PORT_TYPE_TUNNEL.\n");
            return XP_ERR_INVALID_ARG;
        }
        intfId = xpSaiObjIdValueGet(
                     attributes->tunId); // since there is no xpsInterfaceId pre-allocation procedure for tunnels
        // here we will extract just some integer value in 0..TunnelAllocatorMax range,
        // that's why we have to append port type to the created OID.
        // this comment together with 'xpSaiBridgePortObjIdCreate' function has to be
        // removed once xpsInterfaceId pre-allocation for tunnels will be implemented && merged
        bdId = xpSaiObjIdValueGet(attributes->brId);
        /*Tunnel Bridge port is a combination of tunnelId and bdId*/
        localOid = (intfId << (XDK_SAI_OBJID_TUNNEL_BRIDGE_ID_BITS)) | bdId;
        intfId = localOid;
    }
    else if (SAI_BRIDGE_PORT_TYPE_PORT == attributes->brPortType)
    {
        if (SAI_NULL_OBJECT_ID == attributes->portId)
        {
            XP_SAI_LOG_ERR("Invalid args for port type SAI_BRIDGE_PORT_TYPE_PORT.\n");
            return XP_ERR_INVALID_ARG;
        }
        intfId = (xpsInterfaceId_t)xpSaiObjIdValueGet(attributes->portId);

        retVal = xpSaiBridgeAddLagRef(intfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to add lag reference to br-port, intfId = %d, error %d\n",
                           intfId, retVal);
            return retVal;
        }
    }
    else if (SAI_BRIDGE_PORT_TYPE_SUB_PORT == attributes->brPortType)
    {
        if ((SAI_NULL_OBJECT_ID == attributes->portId) ||
            (attributes->vlanId <= XPS_VLANID_MIN) ||
            (attributes->vlanId > XPS_VLANID_MAX))
        {
            XP_SAI_LOG_ERR("Invalid args for port type SAI_BRIDGE_PORT_TYPE_SUB_PORT.\n");
            return XP_ERR_INVALID_ARG;
        }

        xpsInterfaceId_t portIntf = (xpsInterfaceId_t)xpSaiObjIdValueGet(
                                        attributes->portId);

        retVal = xpsInterfaceCreateBridgePort(portIntf, attributes->vlanId, &intfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Unable to create interface, error: %d\n", retVal);
            return retVal;
        }

        retVal = xpSaiBridgeAddLagRef(portIntf);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to add lag reference to br-port, intfId = %d, error %d\n",
                           intfId, retVal);
            return retVal;
        }
    }
    else if (SAI_BRIDGE_PORT_TYPE_1D_ROUTER == attributes->brPortType)
    {
        if (SAI_NULL_OBJECT_ID == attributes->rifId)
        {
            XP_SAI_LOG_ERR("Invalid args for port type SAI_BRIDGE_PORT_TYPE_1D_ROUTER.\n");
            return XP_ERR_INVALID_ARG;
        }

        if (xpSaiRouterInterfaceVirtualCheck(attributes->rifId))
        {
            XP_SAI_LOG_ERR("Virtual RIF OID received\n");
            return XP_ERR_INVALID_ARG;
        }

        saiRetVal = xpSaiRouterInterfaceOnBridgePortAdded(attributes->rifId,
                                                          attributes->brId, &intfId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Unable to create SAI object, sai_error: %d\n", saiRetVal);
            return XP_ERR_INVALID_ID;
        }
    }
    else // SAI_BRIDGE_PORT_TYPE_1Q_ROUTER
    {
        /// TODO: tbd
    }

    saiRetVal = xpSaiBridgePortObjIdCreate(xpsDevId, (uint32_t)intfId,
                                           attributes->brPortType, sai_object_id);
    //xpSaiObjIdCreate(SAI_OBJECT_TYPE_BRIDGE_PORT, xpsDevId, (sai_uint64_t)intfId, sai_object_id); // once xpsInterfaceId pre-allocation
    // for tunnel will be merged
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to create SAI object, sai_error: %d\n", saiRetVal);
        return XP_ERR_INVALID_ID;
    }

    /* save context in state DB*/
    retVal = xpSaiBridgePortInsertCtxDb(xpsDevId, *sai_object_id, &brPort);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to allocate XPS state Heap, error: %d\n", retVal);
        return retVal;
    }

    /* map it to the bridge */
    retVal = xpSaiBridgeAddMappingElement(attributes->brId, *sai_object_id,
                                          *sai_object_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to add bridgePort %llu to bridge %llu, error: %d\n",
                       *sai_object_id, attributes->brId, retVal);
        return retVal;
    }

    brPort->bridgeId            = attributes->brId;
    brPort->brPortType          = attributes->brPortType;
    brPort->maxLearnedAddresses = attributes->maxLearnedAddresses.u32;
    brPort->adminState          = attributes->adminState.booldata;
    brPort->ingressFiltering    = attributes->ingressFiltering.booldata;
    brPort->learnMode           = (sai_bridge_port_fdb_learning_mode_t)
                                  attributes->learnMode.s32;
    brPort->learnLimitAction    = (sai_packet_action_t)attributes->learnMode.s32;
    brPort->taggingMode         = (sai_bridge_port_tagging_mode_t)
                                  attributes->taggingMode.s32;

    bdId = xpSaiObjIdValueGet(attributes->brId);

    if (SAI_BRIDGE_PORT_TYPE_TUNNEL == attributes->brPortType)
    {
        // can't bind tunnel to .1D bridge as Service Instance is not defined.
        // just save BridgePort entry and notify saiTunnel module
        saiRetVal = xpSaiTunnelOnBridgePortAdded(attributes->brId, *sai_object_id,
                                                 attributes->tunId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to add bridge port %llu to the bridge %llu, sai_error: %d\n",
                           *sai_object_id, attributes->brId, saiRetVal);
            return XP_ERR_INVALID_DATA;
        }

        brPort->brPortDescr.objId = attributes->tunId;
    }
    else if (SAI_BRIDGE_PORT_TYPE_1D_ROUTER == attributes->brPortType)
    {
        brPort->brPortDescr.objId = attributes->rifId;
    }
    else if (SAI_BRIDGE_PORT_TYPE_PORT == attributes->brPortType)
    {
        brPort->brPortDescr.objId = attributes->portId;
        retVal = xpSaiBridgePortSetLearningMode(xpsDevId, brPort,
                                                attributes->learnMode.s32);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to set the bridge port default fdb learn type, error %d\n",
                           retVal);
            return retVal;
        }
    }
    else if (SAI_BRIDGE_PORT_TYPE_SUB_PORT == attributes->brPortType)
    {
        retVal = xpsVlanAddInterface(xpsDevId, bdId, intfId,
                                     xpSaiBridgeTaggingMode(brPort->taggingMode));
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to bind sub-interface (%d.%d) to bridge %d, error: %d\n",
                           intfId, attributes->vlanId, bdId, retVal);
            return retVal;
        }

        retVal = xpSaiBridgePortSetLearningMode(xpsDevId, brPort,
                                                attributes->learnMode.s32);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not set the bridge port default fdb learn type, saiRetVal %d\n",
                           saiRetVal);
            return retVal;
        }

        brPort->brPortDescr.subportId.portId = attributes->portId;
        brPort->brPortDescr.subportId.vlanId = attributes->vlanId;
    }
    else if (SAI_BRIDGE_PORT_TYPE_1Q_ROUTER == attributes->brPortType)
    {
        /// TODO: tbd
    }

    /// TODO: params handler

    return retVal;
}

// Func: xpSaiBridgePortHandleRemove

static XP_STATUS xpSaiBridgePortHandleRemove(xpsDevice_t xpsDevId,
                                             sai_object_id_t brPortId)
{
    XP_STATUS              retVal    = XP_NO_ERR;
    sai_status_t           saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiBridgePortInfoT  *entry     = NULL;
    xpsInterfaceId_t       intfId    = XPS_INTF_INVALID_ID;
    xpsVlan_t              brId      = 0;

    XP_SAI_BRIDGE_TRACE();

    /* Search corresponding object */
    retVal = xpSaiBridgePortGetCtxDb(xpsDevId, brPortId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for brPortId %llu\n", brPortId);
        return retVal;
    }

    /* switch it off */
    retVal = xpSaiBridgePortSetAdminState(xpsDevId, entry, false);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to set 'adminState' for brPortId %llu, error: %d\n",
                       brPortId, retVal);
        return retVal;
    }

    brId   = (xpsVlan_t)xpSaiObjIdValueGet(entry->bridgeId);
    intfId = (xpsInterfaceId_t)xpSaiObjIdValueGet(brPortId);

    if (SAI_BRIDGE_PORT_TYPE_TUNNEL == entry->brPortType)
    {
        saiRetVal = xpSaiTunnelOnBridgePortRemoved(entry->bridgeId, brPortId,
                                                   entry->brPortDescr.objId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to remove bridge port %llu from bridge %d, sai_error: %d\n",
                           brPortId, brId, saiRetVal);
            return XP_ERR_INVALID_DATA;
        }
    }
    else if (SAI_BRIDGE_PORT_TYPE_1D_ROUTER == entry->brPortType)
    {
        saiRetVal = xpSaiRouterInterfaceOnBridgePortRemoved(entry->brPortDescr.objId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to remove bridge port %llu from bridge %d, sai_error: %d\n",
                           brPortId, brId, saiRetVal);
            return XP_ERR_INVALID_DATA;
        }
    }
    else if (SAI_BRIDGE_PORT_TYPE_SUB_PORT == entry->brPortType)
    {
        retVal = xpSaiBridgeRemoveLagRef(intfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to remove lag reference from br-port, intfId = %d, error %d\n",
                           intfId, retVal);
            return retVal;
        }

        retVal = xpsVlanRemoveInterface(xpsDevId, brId, intfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to remove vlan %d from bridge %d, error: %d\n", intfId,
                           brId, retVal);
            return retVal;
        }

        retVal = xpsInterfaceDestroyBridgePort(intfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to remove interface %d from bridge %d, error: %d\n",
                           intfId, brId, retVal);
            return retVal;
        }
    }
    else if (SAI_BRIDGE_PORT_TYPE_PORT == entry->brPortType)
    {
        retVal = xpSaiBridgeRemoveLagRef(intfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to remove lag reference from br-port, intfId = %d, error %d\n",
                           intfId, retVal);
            return retVal;
        }
    }
    else if (SAI_BRIDGE_PORT_TYPE_1Q_ROUTER == entry->brPortType)
    {
        /// TODO: tbd
    }

    /* delete from bridge */
    retVal = xpSaiBridgeDelMappingElement(entry->bridgeId, brPortId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to delete BridgePort %llu from bridge %llu, error: %d\n",
                       brPortId, entry->bridgeId, retVal);
        return retVal;
    }

    /* finally - delete object context from state DB */
    retVal = xpSaiBridgePortRemoveCtxDb(xpsDevId, brPortId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to deallocate XPS state entry\n");
        return retVal;
    }

    return retVal;
}

//Func: xpSaiSetDefaultBridgePortAttributeVals

static void xpSaiSetDefaultBridgePortAttributeVals(xpSaiBridgePortAttributesT
                                                   *attributes)
{
    XP_SAI_BRIDGE_TRACE();

    if (NULL == attributes)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return;
    }

    memset(attributes, 0, sizeof(xpSaiBridgePortAttributesT));
    attributes->brPortType                = SAI_BRIDGE_PORT_TYPE_PORT;
    attributes->vlanId                    = XPSAI_DEFAULT_VLAN_ID;
    attributes->portId                    = SAI_NULL_OBJECT_ID;
    attributes->rifId                     = SAI_NULL_OBJECT_ID;
    attributes->tunId                     = SAI_NULL_OBJECT_ID;
    attributes->brId                      = SAI_NULL_OBJECT_ID;
    attributes->learnMode.s32             = SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW;
    attributes->maxLearnedAddresses.u32   = 0; // no limit
    attributes->learnLimitAction.u32      = SAI_PACKET_ACTION_DROP;
    attributes->adminState.booldata       = false;
    attributes->ingressFiltering.booldata = false;
    attributes->taggingMode.s32           = SAI_BRIDGE_PORT_TAGGING_MODE_TAGGED;
}

//Func: xpSaiUpdateBridgePortAttributeVals

static XP_STATUS xpSaiUpdateBridgePortAttributeVals(const uint32_t attr_count,
                                                    const sai_attribute_t *attr_list,
                                                    xpSaiBridgePortAttributesT *attributes)
{
    uint32_t                count = 0;
    const sai_attribute_t  *attr  = NULL;
    sai_status_t           saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_BRIDGE_TRACE();

    if ((NULL == attributes) || (NULL == attr_list))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    attr = xpSaiFindAttrById(SAI_BRIDGE_PORT_ATTR_TYPE, attr_count, attr_list,
                             NULL);
    if (NULL == attr)
    {
        XP_SAI_LOG_ERR("Mandatory parameter not exist.\n");
        return XP_ERR_INVALID_ARG;
    }

    attributes->brPortType = (sai_bridge_port_type_t)attr->value.u32;

    if ((SAI_BRIDGE_PORT_TYPE_SUB_PORT == attributes->brPortType) ||
        (SAI_BRIDGE_PORT_TYPE_1D_ROUTER == attributes->brPortType) ||
        (SAI_BRIDGE_PORT_TYPE_TUNNEL == attributes->brPortType))
    {
        attr = xpSaiFindAttrById(SAI_BRIDGE_PORT_ATTR_BRIDGE_ID, attr_count, attr_list,
                                 NULL);
        if (NULL == attr)
        {
            XP_SAI_LOG_ERR("Mandatory parameter not exist.\n");
            return XP_ERR_INVALID_ARG;
        }

        attributes->brId = attr->value.oid;
    }
    else
    {
        saiRetVal = xpSaiSwitchDefaultBridgeIdGet(&attributes->brId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get Bridge Id , error: %d\n", saiRetVal);
            return XP_ERR_AN_FAILED;
        }
    }

    for (count = 0; count < attr_count; ++count)
    {
        const sai_attribute_value_t *value = &attr_list[count].value;

        switch (attr_list[count].id)
        {
            case SAI_BRIDGE_PORT_ATTR_TYPE:
            case SAI_BRIDGE_PORT_ATTR_BRIDGE_ID:
                {
                    break;  // as we already extracted it
                }
            case SAI_BRIDGE_PORT_ATTR_PORT_ID:
                {
                    attributes->portId = value->oid;
                    break;
                }
            case SAI_BRIDGE_PORT_ATTR_TAGGING_MODE:
                {
                    attributes->taggingMode = *value;
                    break;
                }
            case SAI_BRIDGE_PORT_ATTR_VLAN_ID:
                {
                    attributes->vlanId = value->u16;
                    break;
                }
            case SAI_BRIDGE_PORT_ATTR_RIF_ID:
                {
                    attributes->rifId = value->oid;
                    break;
                }
            case SAI_BRIDGE_PORT_ATTR_TUNNEL_ID:
                {
                    attributes->tunId = value->oid;
                    break;
                }
            case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE:
                {
                    attributes->learnMode = *value;
                    break;
                }
            case SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES:
                {
                    attributes->maxLearnedAddresses = *value;
                    break;
                }
            case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION:
                {
                    attributes->learnLimitAction = *value;
                    break;
                }
            case SAI_BRIDGE_PORT_ATTR_ADMIN_STATE:
                {
                    attributes->adminState = *value;
                    break;
                }
            case SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING:
                {
                    attributes->ingressFiltering = *value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }

    return XP_NO_ERR;
}

//Func: xpSaiSetBridgePortAttribute

static sai_status_t xpSaiSetBridgePortAttribute(sai_object_id_t bridge_port_id,
                                                const sai_attribute_t *attr)
{
    sai_status_t          saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t           xpsDevId  = xpSaiObjIdSwitchGet(bridge_port_id);
    XP_STATUS             retVal    = XP_NO_ERR;
    xpSaiBridgePortInfoT *brPort    = NULL;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == attr)
    {
        XP_SAI_LOG_ERR("Invalid parameter received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(1, attr, BRIDGE_PORT_VALIDATION_ARRAY_SIZE,
                               bridge_port_attribs, SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to check attribute, error: %d\n", saiRetVal);
        return saiRetVal;
    }

    /* Fetch from SAI DB */
    retVal = xpSaiBridgePortGetCtxDb(xpsDevId, bridge_port_id, &brPort);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for brPortId %d\n", bridge_port_id);
        return xpsStatus2SaiStatus(retVal);
    }

    switch (attr->id)
    {
        case SAI_BRIDGE_PORT_ATTR_BRIDGE_ID:
            {
                /* Change of bridge ID for some bridge port types (like a .1D router) will cause change
                   of OID of the bridge port so we can't support set operation of this attribute. */
                return SAI_STATUS_NOT_SUPPORTED;
            }
        case SAI_BRIDGE_PORT_ATTR_TAGGING_MODE:
            {
                if (brPort->brPortType == SAI_BRIDGE_PORT_TYPE_SUB_PORT)
                {
                    saiRetVal = xpSaiBridgePortSetTaggingMode(xpsDevId, brPort, attr->value.s32);
                    if (saiRetVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to set SAI_BRIDGE_PORT_ATTR_TAGGING_MODE for brPortId %"
                                       PRIu64 ", error: %" PRIi32 "\n", bridge_port_id, saiRetVal);
                        return saiRetVal;
                    }
                }
                else
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_BRIDGE_PORT_ATTR_TAGGING_MODE on bridge port as non-subport type!\n");
                    return SAI_STATUS_INVALID_PARAMETER;
                }
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE:
            {
                retVal = xpSaiBridgePortSetLearningMode(xpsDevId, brPort, attr->value.s32);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE for brPortId %llu, error: %d\n",
                                   bridge_port_id, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES:
            {
                retVal = xpSaiBridgePortSetMaxLearnedAddrs(xpsDevId, brPort, attr->value.u32);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES for brPortId %llu, error: %d\n",
                                   bridge_port_id, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION:
            {
                retVal = xpSaiBridgePortSetLimitAction(xpsDevId, brPort, attr->value.s32);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION for brPortId %llu, error: %d\n",
                                   bridge_port_id, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_ADMIN_STATE:
            {
                retVal = xpSaiBridgePortSetAdminState(xpsDevId, brPort, attr->value.booldata);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to set 'adminState' for brPortId %llu, error: %d\n",
                                   bridge_port_id, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING:
            {
                retVal = xpSaiBridgePortSetIngressFiltering(xpsDevId, brPort,
                                                            attr->value.booldata);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to set 'ingressFiltering' for brPortId %llu, error: %d\n",
                                   bridge_port_id, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiGetBridgePortAttribute

static sai_status_t xpSaiGetBridgePortAttribute(xpsDevice_t xpsDevId,
                                                xpSaiBridgePortInfoT *brPort, sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    XP_SAI_BRIDGE_TRACE();

    if ((NULL == attr) || (NULL == brPort))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    switch (attr->id)
    {
        case SAI_BRIDGE_PORT_ATTR_TYPE:
            {
                attr->value.s32 = brPort->brPortType;
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_PORT_ID:
            {
                attr->value.oid = brPort->brPortDescr.objId;
                if (brPort->brPortType == SAI_BRIDGE_PORT_TYPE_SUB_PORT)
                {
                    attr->value.oid = brPort->brPortDescr.subportId.portId;
                }
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_VLAN_ID:
            {
                attr->value.oid = brPort->brPortDescr.subportId.vlanId;
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_RIF_ID:
            {
                attr->value.oid = brPort->brPortDescr.objId;
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_TUNNEL_ID:
            {
                attr->value.oid = brPort->brPortDescr.objId;
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_BRIDGE_ID:
            {
                attr->value.oid = brPort->bridgeId;
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_TAGGING_MODE:
            {
                attr->value.s32 = brPort->taggingMode;
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE:
            {
                attr->value.s32 = brPort->learnMode;
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES:
            {
                attr->value.u32 = brPort->maxLearnedAddresses;
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION:
            {
                attr->value.s32 = brPort->learnLimitAction;
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING:
            {
                attr->value.booldata = brPort->ingressFiltering;
                break;
            }
        case SAI_BRIDGE_PORT_ATTR_ADMIN_STATE:
            {
                attr->value.booldata = brPort->adminState;
                break;
            }
        default:
            {
                XP_SAI_LOG_INFO("Failed to get %d\n", attr->id);
                retVal = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    return retVal;
}

//Func: xpSaiGetBridgePortAttributes

static sai_status_t xpSaiGetBridgePortAttributes(sai_object_id_t bridge_port_id,
                                                 uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t         saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t          xpsDevId  = xpSaiObjIdSwitchGet(bridge_port_id);
    XP_STATUS            retVal    = XP_NO_ERR;
    xpSaiBridgePortInfoT *brPort   = NULL;
    uint32_t             count     = 0;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == attr_list)
    {
        XP_SAI_LOG_ERR("Invalid parameter received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               BRIDGE_PORT_VALIDATION_ARRAY_SIZE,
                               bridge_port_attribs, SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to check attribute, sai_error: %d\n", saiRetVal);
        return saiRetVal;
    }

    /* Fetch from SAI DB */
    retVal = xpSaiBridgePortGetCtxDb(xpsDevId, bridge_port_id, &brPort);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for brPortId %llu, error: %d\n",
                       bridge_port_id, retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    saiRetVal = xpsStatus2SaiStatus(retVal);
    for (count = 0; count < attr_count; ++count)
    {
        saiRetVal = xpSaiGetBridgePortAttribute(xpsDevId, brPort, &attr_list[count]);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiGetBridgeAttribute failed\n");

            if (XPSAI_CHECK_ERROR_OFFSET(saiRetVal))
            {
                saiRetVal += count;
            }

            return saiRetVal;
        }
    }

    return saiRetVal;
}

//Func: xpSaiCreateBridgePort

static sai_status_t xpSaiCreateBridgePort(sai_object_id_t* bridge_port_id,
                                          sai_object_id_t switch_id,
                                          uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_STATUS                   retVal     = XP_NO_ERR;
    sai_status_t                saiRetVal  = SAI_STATUS_SUCCESS;
    xpsDevice_t                 xpsDevId   = 0;
    xpSaiBridgePortAttributesT  attributes;
    xpPktCmd_e learnMode = XP_PKTCMD_MAX;
    sai_object_id_t             saiTnlId   = SAI_NULL_OBJECT_ID;
    XP_SAI_BRIDGE_TRACE();

    if ((NULL == bridge_port_id) || (NULL == attr_list))
    {
        XP_SAI_LOG_ERR("Invalid parameter received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               BRIDGE_PORT_VALIDATION_ARRAY_SIZE,
                               bridge_port_attribs, SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed, error: %d\n", saiRetVal);
        return saiRetVal;
    }

    xpSaiSetDefaultBridgePortAttributeVals(&attributes);
    xpSaiUpdateBridgePortAttributeVals(attr_count, attr_list, &attributes);

    /*get devId from SwitchId*/
    xpsDevId = xpSaiObjIdSwitchGet(switch_id);

    retVal = xpSaiBridgePortHandleCreate(xpsDevId, &attributes, bridge_port_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI object bridgePort could not be created, error: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /*set the fdb learning mode attribute from BridgePort to TunnelTermEntry*/
    if (attributes.brPortType == SAI_BRIDGE_PORT_TYPE_TUNNEL)
    {
        saiRetVal = xpSaiGetTunnelIdByBridgePort(*bridge_port_id, &saiTnlId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get TunnelId from Bridge Port: %d\n", saiRetVal);
            return saiRetVal;
        }
        switch (attributes.learnMode.s32)
        {
            case SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DROP:
                {
                    learnMode = XP_PKTCMD_DROP;
                    break;
                }
            case SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DISABLE:
                {
                    learnMode = XP_PKTCMD_FWD;
                    break;
                }
            case SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW:
            case SAI_BRIDGE_PORT_FDB_LEARNING_MODE_FDB_NOTIFICATION:
                {
                    learnMode = XP_PKTCMD_MAX; // There is no cmd to differentiate b/w learn-mode
                    break;
                }
            case SAI_BRIDGE_PORT_FDB_LEARNING_MODE_CPU_TRAP:
                {
                    learnMode = XP_PKTCMD_TRAP;
                    break;
                }
            default :
                XP_SAI_LOG_ERR("Invalid FDB Learning mode passed, error: %d\n", saiRetVal);
                return saiRetVal;
        }
        saiRetVal = xpSaiTunnelBridgePortSetFdbLearningMode(xpsDevId, saiTnlId,
                                                            learnMode);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("FDB Learning mode in TunnelBridgePort failed, error: %d\n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    return xpsStatus2SaiStatus(retVal);
}


static sai_status_t xpSaiRemoveBridgePort(sai_object_id_t bridge_port_id)
{
    XP_STATUS   retVal   = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(bridge_port_id);

    XP_SAI_BRIDGE_TRACE();

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(bridge_port_id, SAI_OBJECT_TYPE_BRIDGE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received (%llu)\n",
                       xpSaiObjIdTypeGet(bridge_port_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    retVal = xpSaiBridgePortHandleRemove(xpsDevId, bridge_port_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI object bridgePort could not be deleted, error: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiBridgeInit

XP_STATUS xpSaiBridgeInit(xpsDevice_t xpsDevId)
{
    XP_STATUS              retVal        = XP_NO_ERR;
    sai_status_t           saiRetVal     = SAI_STATUS_SUCCESS;
    xpsScope_t             scope         = xpSaiScopeFromDevGet(xpsDevId);
    sai_object_id_t        defltBridgeId = SAI_NULL_OBJECT_ID;
    xpSaiBridgeAttributesT attributes;

    /*  Create global SAI BRIDGE DB */
    xpSaiBridgeStateDbHndl = XPSAI_BRIDGE_DB_HNDL;
    retVal = xpsStateRegisterDb(scope, "SAI BRIDGE DB", XPS_GLOBAL,
                                &saiBridgeCtxKeyComp, xpSaiBridgeStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        xpSaiBridgeStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create SAI BRIDGE DB\n");
        return retVal;
    }

    /* Create global SAI BRIDGE PORT DB */
    xpSaiBridgePortStateDbHndl = XPSAI_BRIDGE_PORT_DB_HNDL;
    retVal = xpsStateRegisterDb(scope, "SAI BRIDGE PORT DB", XPS_GLOBAL,
                                &saiBridgePortCtxKeyComp, xpSaiBridgePortStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        xpSaiBridgePortStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create SAI BRIDGE PORT DB\n");
        return retVal;
    }

    /* Create global SAI BRIDGE MAPPING DB */
    xpSaiBrObjsMappingStateDbHndl = XPSAI_BRIDGE_MAPPING_DB_HNDL;
    retVal = xpsStateRegisterDb(scope, "SAI BRIDGE MAPPING DB", XPS_GLOBAL,
                                &saiBrObjsMappingCtxKeyComp, xpSaiBrObjsMappingStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        xpSaiBrObjsMappingStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create SAI BRIDGE MAPPING DB\n");
        return retVal;
    }

    /* create a default bridge */
    xpSaiSetDefaultBridgeAttributeVals(&attributes);

    retVal = xpSaiBridgeHandleCreate(xpsDevId, &attributes, &defltBridgeId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, error: %d\n", retVal);
        return retVal;
    }

    /* make the default bridge visible for external software
       via SAI switch API (SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID) */

    saiRetVal = xpSaiSwitchDefaultBridgeIdSet(defltBridgeId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, saiRetVal %d\n", saiRetVal);
        return XP_ERR_INIT_FAILED;
    }

    return retVal;
}

//Func: xpSaiCreateDefaultBridgePorts

sai_status_t xpSaiCreateDefaultBridgePorts(xpsDevice_t xpsDevId)
{
    sai_status_t                saiRetVal     = SAI_STATUS_SUCCESS;
    XP_STATUS                   retVal        = XP_NO_ERR;
    xpsInterfaceId_t            portIfId      = 0;
    xpsInterfaceId_t            cpuIfId       = 0;
    sai_object_id_t             portId        = SAI_NULL_OBJECT_ID;
    sai_object_id_t             brPortId      = SAI_NULL_OBJECT_ID;
    sai_object_id_t             switchId      = SAI_NULL_OBJECT_ID;
    sai_object_id_t             defltBridgeId = SAI_NULL_OBJECT_ID;
    xpSaiBridgePortAttributesT  attributes;

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_SWITCH, xpsDevId, 0, &switchId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI switch object could not be created, sai_error: %d\n",
                       saiRetVal);
        return SAI_STATUS_FAILURE;
    }

    saiRetVal = xpSaiSwitchDefaultBridgeIdGet(&defltBridgeId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get default .1Q bridge OID, sai_error: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    retVal = xpsGlobalSwitchControlGetCpuPhysicalPortNum(xpsDevId,
                                                         (uint32_t*)&cpuIfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get interface ID of CPU port, error: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsPortGetFirst(&portIfId);
    if (XP_NO_ERR != retVal)
    {
        XP_SAI_LOG_ERR("xpsPortGetFirst call failed, error: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    xpSaiSetDefaultBridgePortAttributeVals(&attributes);
    attributes.brPortType          = SAI_BRIDGE_PORT_TYPE_PORT;
    attributes.brId                = defltBridgeId;
    attributes.adminState.booldata = true;

    do
    {
        // skip CPU port
        if (!xpSaiIsPortActive(portIfId) ||
            (portIfId == cpuIfId))
        {
            continue;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, xpsDevId, portIfId, &portId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI port object could not be created, sai_error: %d\n",
                           saiRetVal);
            return saiRetVal;
        }

        attributes.portId = portId;

        retVal = xpSaiBridgePortHandleCreate(xpsDevId, &attributes, &brPortId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR(".1Q bridgePort could not be created, error: %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

    }
    while (xpsPortGetNext(portIfId, &portIfId) == XP_NO_ERR);

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiBridgeDeInit

XP_STATUS xpSaiBridgeDeInit(xpsDevice_t xpsDevId)
{
    XP_STATUS    retVal    = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsScope_t   scope     = xpSaiScopeFromDevGet(xpsDevId);

    saiRetVal = xpSaiSwitchDefaultBridgeIdSet(SAI_NULL_OBJECT_ID);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not unset default 1Q bridge id\n");
        return XP_ERR_INIT_FAILED;
    }

    /* Purge global SAI BRIDGE PORT DB */
    retVal = xpsStateDeRegisterDb(scope, &xpSaiBridgePortStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge SAI BRIDGE PORT DB\n");
        return retVal;
    }

    /* Purge global SAI BRIDGE DB */
    retVal = xpsStateDeRegisterDb(scope, &xpSaiBridgeStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge SAI BRIDGE DB\n");
        return retVal;
    }

    /* Purge global SAI BR_MAPPING DB */
    retVal = xpsStateDeRegisterDb(scope, &xpSaiBrObjsMappingStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge SAI BRIDGE MAPPING DB\n");
        return retVal;
    }

    return retVal;
}

//Func: xpSaiBridgeApiInit

XP_STATUS xpSaiBridgeApiInit(uint64_t flag,
                             const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS    retVal    = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_BRIDGE_TRACE();

    _xpSaiBridgeApi = (sai_bridge_api_t *) xpMalloc(sizeof(sai_bridge_api_t));
    if (NULL == _xpSaiBridgeApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiBridgeApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiBridgeApi->create_bridge = xpSaiCreateBridge;
    _xpSaiBridgeApi->remove_bridge = xpSaiRemoveBridge;
    _xpSaiBridgeApi->set_bridge_attribute = xpSaiSetBridgeAttribute;
    _xpSaiBridgeApi->get_bridge_attribute = xpSaiGetBridgeAttributes;
    _xpSaiBridgeApi->create_bridge_port = xpSaiCreateBridgePort;
    _xpSaiBridgeApi->remove_bridge_port = xpSaiRemoveBridgePort;
    _xpSaiBridgeApi->set_bridge_port_attribute = xpSaiSetBridgePortAttribute;
    _xpSaiBridgeApi->get_bridge_port_attribute = xpSaiGetBridgePortAttributes;

    _xpSaiBridgeApi->get_bridge_stats = (sai_get_bridge_stats_fn)
                                        xpSaiStubGenericApi;
    _xpSaiBridgeApi->get_bridge_stats_ext = (sai_get_bridge_stats_ext_fn)
                                            xpSaiStubGenericApi;
    _xpSaiBridgeApi->clear_bridge_stats = (sai_clear_bridge_stats_fn)
                                          xpSaiStubGenericApi;
    _xpSaiBridgeApi->get_bridge_port_stats = (sai_get_bridge_port_stats_fn)
                                             xpSaiStubGenericApi;
    _xpSaiBridgeApi->get_bridge_port_stats_ext = (sai_get_bridge_port_stats_ext_fn)
                                                 xpSaiStubGenericApi;
    _xpSaiBridgeApi->clear_bridge_port_stats = (sai_clear_bridge_port_stats_fn)
                                               xpSaiStubGenericApi;

    saiRetVal = xpSaiApiRegister(SAI_API_BRIDGE, (void*)_xpSaiBridgeApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register BRIDGE API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}

//Func: xpSaiBridgeApiDeinit

XP_STATUS xpSaiBridgeApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_BRIDGE_TRACE();

    xpFree(_xpSaiBridgeApi);
    _xpSaiBridgeApi = NULL;

    return retVal;
}

//Func: xpSaiConvertTaggingMode

static xpsL2EncapType_e xpSaiBridgeTaggingMode(sai_bridge_port_tagging_mode_t
                                               sai_tagging_mode)
{
    switch (sai_tagging_mode)
    {
        case SAI_BRIDGE_PORT_TAGGING_MODE_TAGGED:
            {
                return XP_L2_ENCAP_DOT1Q_TAGGED;
            }
        case SAI_BRIDGE_PORT_TAGGING_MODE_UNTAGGED:
            {
                return XP_L2_ENCAP_DOT1Q_UNTAGGED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown tagging mode %d\n", sai_tagging_mode);
                return XP_L2_ENCAP_INVALID;
            }
    }
}

//Func: xpSaiBridgeLearningModeToPktCmd

static XP_STATUS xpSaiBridgeLearningModeToPktCmd(int32_t mode,
                                                 xpsPktCmd_e *pktCmd)
{
    if (NULL == pktCmd)
    {
        XP_SAI_LOG_ERR("Null pointer passed.\n");
        return XP_ERR_NULL_POINTER;
    }

    switch (mode)
    {
        case SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DROP:
            {
                *pktCmd = XP_PKTCMD_DROP;
                break;
            }
        case SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DISABLE:
            {
                *pktCmd = XP_PKTCMD_FWD;
                break;
            }
        case SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW:
        case SAI_BRIDGE_PORT_FDB_LEARNING_MODE_FDB_NOTIFICATION:
            {
                *pktCmd = XP_PKTCMD_MAX;
                break;
            }
        case SAI_BRIDGE_PORT_FDB_LEARNING_MODE_CPU_LOG:
            {
                *pktCmd = XP_PKTCMD_FWD_MIRROR;
                break;
            }
        case SAI_BRIDGE_PORT_FDB_LEARNING_MODE_CPU_TRAP:
            {
                *pktCmd = XP_PKTCMD_TRAP;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Learning mode %d is not supported\n", mode);
                return XP_ERR_NOT_SUPPORTED;
            }
    }

    return XP_NO_ERR;
}

//Func: xpSaiBridgePortObjIdCreate

sai_status_t xpSaiBridgePortObjIdCreate(xpsDevice_t xpsDevId, uint32_t localId,
                                        sai_bridge_port_type_t brPortType, sai_object_id_t *oid)
{
    sai_status_t    retVal = SAI_STATUS_SUCCESS;
    sai_object_id_t localOid = SAI_NULL_OBJECT_ID;
    sai_object_id_t mask = (sai_object_id_t)((uint8_t)brPortType & 0xff);

    retVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_BRIDGE_PORT, xpsDevId,
                              (sai_uint64_t)localId, &localOid);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to create SAI object, sai_error: %d\n", retVal);
        return retVal;
    }

    *oid = localOid | (mask << (SAI_BRIDGE_PORT_OBJ_TYPE_BITS));

    return retVal;
}

//Func: xpSaiBridgePortObjIdTypeGet

sai_int32_t xpSaiBridgePortObjIdTypeGet(sai_object_id_t brPortObjId)
{
    return (sai_int32_t)((brPortObjId & SAI_BRIDGE_PORT_OBJ_TYPE_MASK) >>
                         SAI_BRIDGE_PORT_OBJ_TYPE_BITS);
}

//Func: xpSaiBridgeBindVxTunnel

sai_status_t xpSaiBridgeAddVxTunnel(sai_object_id_t brPortId,
                                    xpsInterfaceId_t intfId, uint32_t servInstanceId)
{
    xpsDevice_t           xpsDevId = xpSaiObjIdSwitchGet(brPortId);
    XP_STATUS             retVal   = XP_NO_ERR;
    xpSaiBridgePortInfoT *entry    = NULL;
    xpsVlan_t             bdId     = 0;

    XP_SAI_BRIDGE_TRACE();

    /* Fetch from SAI DB */
    retVal = xpSaiBridgePortGetCtxDb(xpsDevId, brPortId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for BridgePort %llu, error: %d\n",
                       brPortId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (entry->brPortType != SAI_BRIDGE_PORT_TYPE_TUNNEL)
    {
        XP_SAI_LOG_ERR("Wrong object type for BridgePort %llu\n", entry->brPortId);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    bdId = xpSaiObjIdValueGet(entry->bridgeId);

    retVal = xpsVlanAddEndpoint(xpsDevId, bdId, intfId, XP_L2_ENCAP_VXLAN,
                                servInstanceId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to add endpoint Tunnel %d to the .1D bridge %d, error: %d\n",
                       intfId, entry->bridgeId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiBridgeRemoveVxTunnel

sai_status_t xpSaiBridgeRemoveVxTunnel(sai_object_id_t brPortId,
                                       xpsInterfaceId_t intfId)
{
    xpsDevice_t           xpsDevId  = xpSaiObjIdSwitchGet(brPortId);
    XP_STATUS             retVal    = XP_NO_ERR;
    xpSaiBridgePortInfoT *entry     = NULL;

    XP_SAI_BRIDGE_TRACE();

    /* Fetch from SAI DB */
    retVal = xpSaiBridgePortGetCtxDb(xpsDevId, brPortId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for BridgePort %llu, error: %d\n",
                       brPortId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (entry->brPortType != SAI_BRIDGE_PORT_TYPE_TUNNEL)
    {
        XP_SAI_LOG_ERR("Wrong object type for BridgePort %llu\n", entry->brPortId);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    retVal = xpsVlanRemoveEndpoint(xpsDevId, xpSaiObjIdValueGet(entry->bridgeId),
                                   intfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to remove endpoint Tunnel %d from the .1D bridge %llu, error: %d\n",
                       intfId, entry->bridgeId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiBridgeCreateMappingEntry

XP_STATUS xpSaiBridgeCreateMappingEntry(sai_object_id_t entryId)
{
    XP_STATUS            retVal = XP_NO_ERR;
    xpsDevice_t          xpsDevId  = xpSaiObjIdSwitchGet(entryId);
    xpSaiBrObjsMapInfoT *entry  = NULL;

    XP_SAI_BRIDGE_TRACE();

    if (SAI_NULL_OBJECT_ID == entryId)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_INVALID_INPUT;
    }

    retVal = xpSaiBrMappingInsertCtxDb(xpsDevId, entryId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to allocate new mapping entry\n");
        return retVal;
    }

    return retVal;
}

//Func: xpSaiBridgeRemoveMappingEntry

XP_STATUS xpSaiBridgeRemoveMappingEntry(sai_object_id_t entryId)
{
    XP_STATUS            retVal   = XP_NO_ERR;
    xpsDevice_t          xpsDevId = xpSaiObjIdSwitchGet(entryId);
    xpSaiBrObjsMapInfoT *entry    = NULL;

    XP_SAI_BRIDGE_TRACE();

    if (SAI_NULL_OBJECT_ID == entryId)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_INVALID_INPUT;
    }

    /* Search corresponding object */
    retVal = xpSaiBrMappingGetCtxDb(xpsDevId, entryId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Mapping entry not %lu not exist\n", entryId);
        return retVal;
    }

    retVal = xpSaiBrMappingRemoveCtxDb(xpsDevId, entryId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to deallocate XPS state entry\n");
        return retVal;
    }

    return retVal;
}

// Func: xpSaiBridgeAddMappingElement

XP_STATUS xpSaiBridgeAddMappingElement(sai_object_id_t entryId,
                                       sai_object_id_t elemId, sai_object_id_t bridgePortId)
{
    XP_STATUS            retVal   = XP_NO_ERR;
    xpsDevice_t          xpsDevId = xpSaiObjIdSwitchGet(entryId);
    xpSaiBrObjsMapInfoT *entry    = NULL;
    xpSaiBrObjsMapInfoT *newEntry = NULL;

    XP_SAI_BRIDGE_TRACE();

    if ((SAI_NULL_OBJECT_ID == entryId) || (SAI_NULL_OBJECT_ID == elemId))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_INVALID_INPUT;
    }

    /* Search for corresponding object */
    retVal = xpSaiBrMappingGetCtxDb(xpsDevId, entryId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Mapping entry %llu not exist\n", entryId);
        return retVal;
    }

    if (xpsDAIsCtxGrowthNeeded(entry->numItems, XP_SAI_BRIDGE_THRESHOLD))
    {
        /* Extend the array size */
        retVal = xpSaiDynamicArrayGrow((void**)&newEntry, (void*)entry,
                                       sizeof(xpSaiBrObjsMapInfoT), sizeof(xpSaiBridgeMapItem_t),
                                       entry->numItems, XP_SAI_BRIDGE_THRESHOLD);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Dynamic array grow failed, error: %d\n", retVal);
            return retVal;
        }

        /* Replace the corresponding state */
        retVal = xpSaiBrMappingReplaceItem(xpsDevId, entryId, newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("Replacement of data failed, error: %d\n", retVal);
            return retVal;
        }

        entry = newEntry;
    }

    entry->items[entry->numItems].key = elemId;
    entry->items[entry->numItems].bridgePort = bridgePortId;
    entry->numItems++;

    return retVal;
}

// Func: xpSaiBridgeDelPortInBridge

XP_STATUS xpSaiBridgeDelMappingElement(sai_object_id_t entryId,
                                       sai_object_id_t elemId)
{
    XP_STATUS            retVal   = XP_NO_ERR;
    xpsDevice_t          xpsDevId = xpSaiObjIdSwitchGet(entryId);
    xpSaiBrObjsMapInfoT *entry    = NULL;
    xpSaiBrObjsMapInfoT *newEntry = NULL;
    uint8_t              found    = 0;
    uint32_t             iter     = 0;

    XP_SAI_BRIDGE_TRACE();

    if ((SAI_NULL_OBJECT_ID == entryId) || (SAI_NULL_OBJECT_ID == elemId))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_INVALID_INPUT;
    }

    /* Search for corresponding object */
    retVal = xpSaiBrMappingGetCtxDb(xpsDevId, entryId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Mapping entry %llu not exist\n", entryId);
        return retVal;
    }

    /* Remove the Entry Id from Table */
    for (iter = 0; iter < entry->numItems; ++iter)
    {
        if (!found)
        {
            found = (entry->items[iter].key == elemId) ? 1 : 0;
            continue;
        }
        else
        {
            entry->items[iter-1] = entry->items[iter];
        }
    }

    if (!found)
    {
        return XP_ERR_INVALID_ARG;
    }

    /* Decrement the numPorts count */
    entry->numItems--;
    if (xpsDAIsCtxShrinkNeeded(entry->numItems, XP_SAI_BRIDGE_THRESHOLD))
    {
        /* Shrink the queueList array size */
        retVal = xpSaiDynamicArrayShrink((void**)&newEntry, (void*)entry,
                                         sizeof(xpSaiBrObjsMapInfoT), sizeof(xpSaiBridgeMapItem_t),
                                         entry->numItems, XP_SAI_BRIDGE_THRESHOLD);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Dynamic array shrink failed, error: %d\n", retVal);
            return retVal;
        }

        /* Replace the corresponding state */
        retVal = xpSaiBrMappingReplaceItem(xpsDevId, entryId, newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("Replacement of data failed, error: %d\n", retVal);
            return retVal;
        }
    }

    return retVal;
}

// Func: xpSaiBridgeGetMappingElement

XP_STATUS xpSaiBridgeGetMappingElement(sai_object_id_t entryId,
                                       sai_object_id_t elemId, sai_object_id_t *dest)
{
    XP_STATUS            retVal   = XP_NO_ERR;
    xpsDevice_t          xpsDevId = xpSaiObjIdSwitchGet(entryId);
    xpSaiBrObjsMapInfoT *entry    = NULL;
    uint32_t             iter     = 0;

    XP_SAI_BRIDGE_TRACE();

    if ((SAI_NULL_OBJECT_ID == entryId) || (SAI_NULL_OBJECT_ID == elemId))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_INVALID_INPUT;
    }

    if (NULL == dest)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    /* Search for corresponding object */
    retVal = xpSaiBrMappingGetCtxDb(xpsDevId, entryId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Mapping entry %llu not exist\n", entryId);
        return retVal;
    }

    *dest = SAI_NULL_OBJECT_ID;

    /* Looking up in items */
    for (iter = 0; iter < entry->numItems; ++iter)
    {
        if (entry->items[iter].key == elemId)
        {
            *dest = entry->items[iter].bridgePort;
            break;
        }
    }

    return (SAI_NULL_OBJECT_ID == *dest) ? XP_ERR_KEY_NOT_FOUND : XP_NO_ERR;
}

// Func: xpSaiBridgeGetMappingEntrySize

XP_STATUS xpSaiBridgeGetMappingEntrySize(sai_object_id_t entryId,
                                         uint32_t *dest)
{
    XP_STATUS            retVal   = XP_NO_ERR;
    xpsDevice_t          xpsDevId = xpSaiObjIdSwitchGet(entryId);
    xpSaiBrObjsMapInfoT *entry    = NULL;

    XP_SAI_BRIDGE_TRACE();

    if ((SAI_NULL_OBJECT_ID == entryId) || (NULL == dest))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    /* Search for corresponding object */
    retVal = xpSaiBrMappingGetCtxDb(xpsDevId, entryId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Mapping entry %llu not exist\n", entryId);
        return retVal;
    }

    *dest = entry->numItems;

    return XP_NO_ERR;
}

// Func: xpSaiBridgeGetMappingEntry

XP_STATUS xpSaiBridgeGetMappingEntry(sai_object_id_t entryId,
                                     uint32_t *dest_size, xpSaiBridgeMapItem_t *dest)
{
    XP_STATUS            retVal   = XP_NO_ERR;
    xpsDevice_t          xpsDevId = xpSaiObjIdSwitchGet(entryId);
    xpSaiBrObjsMapInfoT *entry    = NULL;
    uint32_t             iter     = 0;

    XP_SAI_BRIDGE_TRACE();

    if (SAI_NULL_OBJECT_ID == entryId)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_INVALID_INPUT;
    }

    if ((NULL == dest_size) || (NULL == dest))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    /* Search for corresponding object */
    retVal = xpSaiBrMappingGetCtxDb(xpsDevId, entryId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Mapping entry %llu not exist\n", entryId);
        return retVal;
    }

    if (*dest_size < entry->numItems)
    {
        XP_SAI_LOG_DBG("Not enough room for %d ports!\n", entry->numItems);
        *dest_size = entry->numItems;
        return XP_ERR_INVALID_DATA_SIZE;
    }

    for (iter = 0; iter < entry->numItems; ++iter)
    {
        memcpy(dest+iter, entry->items+iter, sizeof(xpSaiBridgeMapItem_t));
    }

    *dest_size = entry->numItems;

    return XP_NO_ERR;
}

//Func: xpSaiGetBridgePortById

XP_STATUS xpSaiGetBridgePortById(sai_object_id_t brPortId,
                                 xpSaiBridgePort_t* dest)
{
    XP_STATUS              retVal   = XP_NO_ERR;
    xpsDevice_t            xpsDevId = xpSaiObjIdSwitchGet(brPortId);
    xpSaiBridgePortInfoT  *brPort   = NULL;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == dest)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    /* Search corresponding object */
    retVal = xpSaiBridgePortGetCtxDb(xpsDevId, brPortId, &brPort);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for bridgePortId %llu\n", brPortId);
        return retVal;
    }

    dest->bridgeId   = brPort->bridgeId;
    dest->brPortType = brPort->brPortType;
    memcpy(&dest->brPortDescr, &brPort->brPortDescr,
           sizeof(xpSaiBridgePortDescr_t));

    return retVal;
}

//Func: xpSaiGetBridgePortsSize

XP_STATUS xpSaiGetBridgePortsSize(sai_object_id_t bridgeId, uint32_t *dest)
{
    XP_STATUS            retVal   = XP_NO_ERR;
    xpsDevice_t          xpsDevId = xpSaiObjIdSwitchGet(bridgeId);
    xpSaiBrObjsMapInfoT *entry    = NULL;

    if (NULL == dest)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    /* Search for corresponding object */
    retVal = xpSaiBrMappingGetCtxDb(xpsDevId, bridgeId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Mapping entry %llu not exist\n", bridgeId);
        return retVal;
    }

    *dest = entry->numItems;

    return retVal;
}

//Func: xpSaiGetBridgeTypeByBridgeId

XP_STATUS xpSaiGetBridgeTypeByBridgeId(sai_object_id_t bridgeId,
                                       sai_bridge_type_t *bridgeType)
{
    XP_STATUS              retVal   = XP_NO_ERR;
    xpsDevice_t            xpsDevId = xpSaiObjIdSwitchGet(bridgeId);
    xpSaiBridgeInfoT       *bridge   = NULL;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == bridgeType)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    /* Search corresponding object */
    retVal = xpSaiBridgeGetCtxDb(xpsDevId, bridgeId, &bridge);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for bridgeId %lu\n", bridgeId);
        return retVal;
    }

    *bridgeType = bridge->bridgeType;

    return retVal;
}

//Func: xpSaiGetBridgePortsByBridgeId

XP_STATUS xpSaiGetBridgePortsByBridgeId(sai_object_id_t bridgeId,
                                        uint32_t *dest_size, xpSaiBridgePort_t* dest)
{
    XP_STATUS            retVal   = XP_NO_ERR;
    xpsDevice_t          xpsDevId = xpSaiObjIdSwitchGet(bridgeId);
    xpSaiBridgeInfoT    *bridge   = NULL;
    xpSaiBrObjsMapInfoT *entry    = NULL;
    uint32_t             iter     = 0;

    if ((NULL == dest) || (NULL == dest_size))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    /* Search corresponding bridge */
    retVal = xpSaiBridgeGetCtxDb(xpsDevId, bridgeId, &bridge);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for bridgeId %llu.\n", bridgeId);
        return retVal;
    }

    /* Search for corresponding object */
    retVal = xpSaiBrMappingGetCtxDb(xpsDevId, bridgeId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Mapping entry %llu not exist\n", bridgeId);
        return retVal;
    }

    if (*dest_size < entry->numItems)
    {
        *dest_size = entry->numItems; // tell him about real size
        return XP_ERR_INVALID_DATA_SIZE;
    }

    for (iter = 0; iter < entry->numItems; ++iter)
    {
        retVal = xpSaiGetBridgePortById(entry->items[iter].bridgePort, (dest + iter));
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get bridgePortId %llu.\n",
                           entry->items[iter].bridgePort);
            return retVal;
        }
    }

    *dest_size = entry->numItems;

    return retVal;
}

//Func: xpSaiGetBridgeIdByBridgePort

sai_status_t xpSaiGetBridgeIdByBridgePort(sai_object_id_t brPortId,
                                          sai_object_id_t *bridgeId)
{
    xpsDevice_t           xpsDevId = xpSaiObjIdSwitchGet(brPortId);
    XP_STATUS             retVal   = XP_NO_ERR;
    xpSaiBridgePortInfoT *brPort   = NULL;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == bridgeId)
    {
        XP_SAI_LOG_ERR("Invalid parameter received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Fetch from SAI DB */
    retVal = xpSaiBridgePortGetCtxDb(xpsDevId, brPortId, &brPort);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for brPortId %llu, error: %d\n",
                       brPortId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    *bridgeId = brPort->bridgeId;

    return xpsStatus2SaiStatus(retVal);
}


//Func: xpSaiGetTunnelIdByBridgePort

sai_status_t xpSaiGetTunnelIdByBridgePort(sai_object_id_t brPortId,
                                          sai_object_id_t *tunnelId)
{
    xpsDevice_t           xpsDevId = xpSaiObjIdSwitchGet(brPortId);
    XP_STATUS             retVal   = XP_NO_ERR;
    xpSaiBridgePortInfoT *brPort   = NULL;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == tunnelId)
    {
        XP_SAI_LOG_ERR("Invalid parameter received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Fetch from SAI DB */
    retVal = xpSaiBridgePortGetCtxDb(xpsDevId, brPortId, &brPort);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for brPortId %llu, error: %d\n",
                       brPortId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (brPort->brPortType != SAI_BRIDGE_PORT_TYPE_TUNNEL)
    {
        XP_SAI_LOG_ERR("Wrong bridge port type (%d) passed.\n", brPort->brPortType);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    *tunnelId = brPort->brPortDescr.objId;

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiGetBridgePortTypeById
sai_status_t xpSaiGetBridgePortTypeById(sai_object_id_t brPortId,
                                        sai_bridge_port_type_t* type)
{
    XP_STATUS              retVal   = XP_NO_ERR;
    xpsDevice_t            xpsDevId = xpSaiObjIdSwitchGet(brPortId);
    xpSaiBridgePortInfoT  *brPort   = NULL;

    XP_SAI_BRIDGE_TRACE();

    if (NULL == type)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    /* Search corresponding object */
    retVal = xpSaiBridgePortGetCtxDb(xpsDevId, brPortId, &brPort);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to fetch from XPS DB for bridgePortId %llu\n", brPortId);
        return retVal;
    }


    *type = brPort->brPortType;
    return xpsStatus2SaiStatus(retVal);
}


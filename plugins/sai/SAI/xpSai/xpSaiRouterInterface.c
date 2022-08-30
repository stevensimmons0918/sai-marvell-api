// xpSaiRouterInterface.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiRouterInterface.h"
#include "xpSaiVrf.h"
#include "xpSaiPort.h"
#include "xpSaiVlan.h"
#include "xpSaiValidationArrays.h"
#include "xpsAllocator.h"
#include "xpSaiStub.h"
#include "xpSaiAclMapper.h"
#include "xpSaiHostInterface.h"

XP_SAI_LOG_REGISTER_API(SAI_API_ROUTER_INTERFACE);

#define XP_SAI_LOOPBACK_RIF_MAX_IDS     256
#define XP_SAI_LOOPBACK_RIF_RANGE_START 0
#define XP_SAI_RIF_MAX_IDS              12*1024
#define XP_SAI_RIF_RANGE_START          1
#define XDK_SAI_VIRT_RIF_OBJID_VALUE_OFFSET   30
#define XDK_SAI_VIRT_RIF_OBJID_VALUE_MASK     0x000000FFC0000000ULL


static xpsDbHandle_t gXpSaiRouterInterfaceDbHandle =
    XPSAI_ROUTER_INTERFACE_DB_HNDL;
static xpsDbHandle_t gXpSaiRifStatisticDbHandle = XPSAI_RIF_STATISTIC_DB_HNDL;

static sai_router_interface_api_t* _xpSaiRouterInterfaceApi;

static sai_status_t xpSaiSetRouterInterfaceAttribute(sai_object_id_t rif_id,
                                                     const sai_attribute_t *attr);
static sai_status_t xpSaiRouterInterfaceVirtIdGet(sai_object_id_t rif_id,
                                                  uint16_t* pVirtId);
static sai_status_t xpSaiRemoveVirtRouterInterfaces(xpSaiRouterInterfaceDbEntryT
                                                    *pRifEntry);
static sai_status_t xpSaiRemoveRouterInterfaceMacAddress(
    sai_object_id_t rif_id);
static sai_status_t xpSaiRouterInterfaceObjIdCreate(sai_uint32_t dev_id,
                                                    xpsInterfaceId_t rif_id,
                                                    uint32_t virt_rif_id, sai_object_id_t *sai_object_id);
extern sai_status_t xpSaiLagConfigInfoGet(xpsInterfaceId_t lagId,
                                          xpSaiLagInfo_t **lagConfig);
static XP_STATUS xpSaiRouterInterfaceCommonDrop(sai_object_id_t rif_id,
                                                bool enable);


//Func: xpSaiConvertRouterInterfaceOid

sai_status_t xpSaiConvertRouterInterfaceOid(sai_object_id_t rif_oid,
                                            uint32_t *rif_id)
{
    if (!XDK_SAI_OBJID_TYPE_CHECK(rif_oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(rif_oid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (rif_id == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *rif_id = (uint32_t)(xpSaiObjIdValueGet(rif_oid) &
                         ~XDK_SAI_VIRT_RIF_OBJID_VALUE_MASK);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRouterInterfaceTypeGet

sai_status_t xpSaiRouterInterfaceTypeGet(sai_object_id_t rif_oid,
                                         sai_router_interface_type_t *rif_type)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    xpSaiRouterInterfaceDbEntryT *pRouterInterfaceEntry = NULL;

    if (!XDK_SAI_OBJID_TYPE_CHECK(rif_oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(rif_oid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (rif_type == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_oid, &pRouterInterfaceEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    *rif_type = (sai_router_interface_type_t)pRouterInterfaceEntry->rifType;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetRouterInterfaceL3IntfId

sai_status_t xpSaiGetRouterInterfaceL3IntfId(sai_object_id_t rif_oid,
                                             xpsInterfaceId_t *pL3IntfId)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    xpSaiRouterInterfaceDbEntryT *pRouterInterfaceEntry = NULL;

    if (!XDK_SAI_OBJID_TYPE_CHECK(rif_oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(rif_oid));
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (pL3IntfId == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_oid, &pRouterInterfaceEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    *pL3IntfId = pRouterInterfaceEntry->l3IntfId;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiRouterInterfaceVirtIdGet(sai_object_id_t rif_id,
                                                  uint16_t* pVirtId)
{
    sai_uint64_t oidValue = 0;

    if (pVirtId == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(rif_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    oidValue = xpSaiObjIdValueGet(rif_id);
    *pVirtId = (uint16_t)((oidValue & XDK_SAI_VIRT_RIF_OBJID_VALUE_MASK) >>
                          XDK_SAI_VIRT_RIF_OBJID_VALUE_OFFSET);

    return SAI_STATUS_SUCCESS;
}

bool xpSaiRouterInterfaceVirtualCheck(sai_object_id_t rif_id)
{
    uint16_t virtRifId = 0;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    saiRetVal = xpSaiRouterInterfaceVirtIdGet(rif_id, &virtRifId);
    return ((saiRetVal == SAI_STATUS_SUCCESS) && (virtRifId > 0));
}

/*
* Routine Description:
*     Creates SAI RIF Object ID.
*
* Arguments:
*     [in] sai_uint32_t : device ID
*     [in] uint32_t : RIF ID
*     [in] uint32_t : Virtual RIF ID
*     [out] sai_object_id_t
*
* Return Values:
*     sai_status_t
*/
static sai_status_t xpSaiRouterInterfaceObjIdCreate(sai_uint32_t dev_id,
                                                    uint32_t rif_id,
                                                    uint32_t virt_rif_id, sai_object_id_t *sai_object_id)
{
    const sai_object_type_t sai_object_type = SAI_OBJECT_TYPE_ROUTER_INTERFACE;

    if ((~XDK_SAI_VIRT_RIF_OBJID_VALUE_MASK & rif_id) != rif_id)
    {
        XP_SAI_LOG_WARNING("RIF ID %u out of range\n", rif_id);
    }

    sai_uint64_t rif_tmp = ((sai_uint64_t)virt_rif_id <<
                            XDK_SAI_VIRT_RIF_OBJID_VALUE_OFFSET) & XDK_SAI_VIRT_RIF_OBJID_VALUE_MASK;

    return xpSaiObjIdCreate(sai_object_type, dev_id, rif_tmp | rif_id,
                            sai_object_id);
}

/**
 * \brief Verify router interface loopback ID
 *
 * \param rif - Router interface loopback ID
 *
 * This API is used to check is rif is router interface loopback ID
 *
 * \return TRUE if rif is loopback
 */
bool xpSaiRouterInterfaceLoopbackIdCheck(sai_object_id_t rif_id)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t     rif       = 0;

    saiRetVal = xpSaiConvertRouterInterfaceOid(rif_id, &rif);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertRouterInterfaceOid() failed with error code: %d\n",
                       saiRetVal);
        return false;
    }

    return (rif & XPSAI_ROUTER_INTERFACE_LOOPBACK_RIF_MASK);
}

//Func: xpSaiRouterInterfaceLoopbackIdAllocate

static sai_status_t xpSaiRouterInterfaceLoopbackIdAllocate(uint32_t *rif_id)
{
    XP_STATUS retVal = XP_NO_ERR;

    retVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_LOOPBACK_RIF,
                                    rif_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate loopback rif id, retVal %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    *rif_id = *rif_id | XPSAI_ROUTER_INTERFACE_LOOPBACK_RIF_MASK;

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiRouterInterfaceLoopbackIdRelease

static sai_status_t xpSaiRouterInterfaceLoopbackIdRelease(
    sai_object_id_t rif_oid)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS    retVal    = XP_NO_ERR;
    sai_uint32_t rifId     = 0;

    saiRetVal = xpSaiConvertRouterInterfaceOid(rif_oid, &rifId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertRouterInterfaceOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    rifId = rifId & ~XPSAI_ROUTER_INTERFACE_LOOPBACK_RIF_MASK;

    retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_LOOPBACK_RIF,
                                   rifId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not release loopback rif id %d\n", rifId);
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiRouterInterfaceKeyComp

static int32_t xpSaiRouterInterfaceKeyComp(void *key1, void *key2)
{
    return ((int32_t)(((xpSaiRouterInterfaceDbEntryT*)key1)->keyRifId) -
            (int32_t)(((xpSaiRouterInterfaceDbEntryT*)key2)->keyRifId));
}

//Func: xpSaiRifStatisticKeyComp

static int32_t xpSaiRifStatisticKeyComp(void *key1, void *key2)
{
    return ((int32_t)(((xpSaiRouterInterfaceStatisticDbEntryT*)key1)->keyRifId) -
            (int32_t)(((
                           xpSaiRouterInterfaceStatisticDbEntryT*)key2)->keyRifId));
}


sai_status_t xpSaiRouterInterfaceDbInfoGet(sai_object_id_t rif_oid,
                                           xpSaiRouterInterfaceDbEntryT **ppRouterInterfaceEntry)
{
    XP_STATUS                    retVal    = XP_NO_ERR;
    sai_status_t                 saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t                     rifId     = 0;
    xpSaiRouterInterfaceDbEntryT *routerIntfkey;

    if (ppRouterInterfaceEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiConvertRouterInterfaceOid(rif_oid, &rifId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertRouterInterfaceOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    routerIntfkey = (xpSaiRouterInterfaceDbEntryT *) xpMalloc(sizeof(
                                                                  xpSaiRouterInterfaceDbEntryT));
    if (!routerIntfkey)
    {
        XP_SAI_LOG_ERR("Could not allocate Router Interface structure\n");
        return xpsStatus2SaiStatus(XP_ERR_MEM_ALLOC_ERROR);
    }
    memset(routerIntfkey, 0, sizeof(xpSaiRouterInterfaceDbEntryT));

    routerIntfkey->keyRifId = rifId;

    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gXpSaiRouterInterfaceDbHandle,
                                routerIntfkey, (void**)ppRouterInterfaceEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData() failed with error code: %d!\n", retVal);
        xpFree(routerIntfkey);
        return xpsStatus2SaiStatus(retVal);
    }

    if (*ppRouterInterfaceEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find Router Interface entry in DB\n");
        xpFree(routerIntfkey);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    xpFree(routerIntfkey);
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiRouterInterfaceDbInsert(xpSaiRouterInterfaceDbEntryT
                                          *pRifEntry)
{
    XP_STATUS                    retVal                 = XP_NO_ERR;
    xpSaiRouterInterfaceDbEntryT *pRouterInterfaceEntry = NULL;
    xpSaiRouterInterfaceDbEntryT *routerIntfkey;

    if (pRifEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    routerIntfkey = (xpSaiRouterInterfaceDbEntryT *) xpMalloc(sizeof(
                                                                  xpSaiRouterInterfaceDbEntryT));
    if (!routerIntfkey)
    {
        XP_SAI_LOG_ERR("Could not allocate Router Interface structure\n");
        return xpsStatus2SaiStatus(XP_ERR_MEM_ALLOC_ERROR);
    }
    memset(routerIntfkey, 0, sizeof(xpSaiRouterInterfaceDbEntryT));

    routerIntfkey->keyRifId = pRifEntry->keyRifId;

    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gXpSaiRouterInterfaceDbHandle,
                                routerIntfkey, (void**)&pRouterInterfaceEntry);
    if (pRouterInterfaceEntry != NULL)
    {
        XP_SAI_LOG_ERR("Router InterfaceEntry already present\n");
        xpFree(routerIntfkey);
        return SAI_STATUS_SUCCESS;
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiRouterInterfaceDbEntryT),
                                (void**)&pRouterInterfaceEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate Router Interface structure\n");
        xpFree(routerIntfkey);
        return xpsStatus2SaiStatus(retVal);
    }

    memset(pRouterInterfaceEntry, 0, sizeof(xpSaiRouterInterfaceDbEntryT));
    memcpy(pRouterInterfaceEntry, pRifEntry, sizeof(xpSaiRouterInterfaceDbEntryT));

    retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, gXpSaiRouterInterfaceDbHandle,
                                (void*)pRouterInterfaceEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not insert RIF structure into DB, keyRifId %u, error %d\n",
                       pRifEntry->keyRifId, retVal);
        xpFree(routerIntfkey);
        return xpsStatus2SaiStatus(retVal);
    }

    xpFree(routerIntfkey);
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiRouterInterfaceDbDelete(xpSaiRouterInterfaceDbEntryT
                                          *pRouterInterfaceEntry)
{
    XP_STATUS                    retVal = XP_NO_ERR;
    xpSaiRouterInterfaceDbEntryT *key;

    if (pRouterInterfaceEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key = (xpSaiRouterInterfaceDbEntryT *)xpMalloc(sizeof(
                                                       xpSaiRouterInterfaceDbEntryT));
    if (!key)
    {
        XP_SAI_LOG_ERR("Could not allocate Router Interface structure\n");
        return xpsStatus2SaiStatus(XP_ERR_MEM_ALLOC_ERROR);
    }
    memset(key, 0, sizeof(xpSaiRouterInterfaceDbEntryT));

    key->keyRifId = pRouterInterfaceEntry->keyRifId;

    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gXpSaiRouterInterfaceDbHandle,
                                key, (void**)&pRouterInterfaceEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData() failed with error code: %d!\n", retVal);
        xpFree(key);
        return xpsStatus2SaiStatus(retVal);
    }

    if (pRouterInterfaceEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find Router Interface entry for keyRifId %u in DB",
                       key->keyRifId);
        xpFree(key);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, gXpSaiRouterInterfaceDbHandle,
                                (xpsDbKey_t)key, (void**)&pRouterInterfaceEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not delete entry from DB, keyRifId %u, error %d\n",
                       key->keyRifId, retVal);
        xpFree(key);
        return xpsStatus2SaiStatus(retVal);
    }

    // Free the memory allocated for the corresponding state
    retVal = xpsStateHeapFree((void*)pRouterInterfaceEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not free heap memory for keyRifId %u, error %d\n",
                       key->keyRifId, retVal);
        xpFree(key);
        return xpsStatus2SaiStatus(retVal);
    }

    xpFree(key);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetDefaultRouterInterfaceAttributeVals

void xpSaiSetDefaultRouterInterfaceAttributeVals(const uint32_t attr_count,
                                                 const sai_attribute_t* attr_list, xpSaiRouterInterfaceAttributesT* attributes)
{
    XP_STATUS         retVal    = XP_NO_ERR;
    sai_status_t      saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t          vrfId     = 0;
    uint32_t          count     = 0;

    attributes->virtualRouterId.oid = 0;
    attributes->portType.s32 = -1;
    attributes->portId.oid = 0;
    attributes->vlanId.oid = 0;
    attributes->adminV4State.booldata = true;
    attributes->adminV6State.booldata = true;
    attributes->mcAdminV4State.booldata = false;
    attributes->mcAdminV6State.booldata = false;
    attributes->mtu.u32 = XP_SAI_DEFAULT_MTU;
    attributes->neighMissAction.u32 = SAI_PACKET_ACTION_TRAP;
    attributes->isVirtual.booldata = false;

    memset(&attributes->srcMacAddress, 0, sizeof(attributes->srcMacAddress));

    for (count = 0; count < attr_count; count++)
    {
        if (attr_list[count].id == SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID)
        {
            saiRetVal = xpSaiConvertVirtualRouterOid(attr_list[count].value.oid, &vrfId);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiConvertVirtualRouterOid() failed with error code: %d\n",
                               saiRetVal);
                return;
            }

            retVal = xpSaiVrfMacAddressGet(xpSaiObjIdSwitchGet(attr_list[count].value.oid),
                                           vrfId, attributes->srcMacAddress.mac);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("xpSaiVrfMacAddressGet() failed with error code: %d\n", retVal);
                return;
            }
        }
    }
}

//Func: xpSaiUpdateRouterInterfaceAttributeVals

sai_status_t xpSaiUpdateRouterInterfaceAttributeVals(const uint32_t attr_count,
                                                     const sai_attribute_t* attr_list, xpSaiRouterInterfaceAttributesT* attributes)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID:
                {
                    attributes->virtualRouterId = attr_list[count].value;
                    break;
                }
            case SAI_ROUTER_INTERFACE_ATTR_TYPE:
                {
                    if ((attr_list[count].value.s32 != SAI_ROUTER_INTERFACE_TYPE_PORT) &&
                        (attr_list[count].value.s32 != SAI_ROUTER_INTERFACE_TYPE_VLAN) &&
                        (attr_list[count].value.s32 != SAI_ROUTER_INTERFACE_TYPE_LOOPBACK) &&
                        (attr_list[count].value.s32 != SAI_ROUTER_INTERFACE_TYPE_SUB_PORT) &&
                        (attr_list[count].value.s32 != SAI_ROUTER_INTERFACE_TYPE_BRIDGE))
                    {
                        XP_SAI_LOG_ERR("Wrong RIF type passed!\n");
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }

                    attributes->portType = attr_list[count].value;
                    break;
                }
            case SAI_ROUTER_INTERFACE_ATTR_PORT_ID:
                {
                    if (!XDK_SAI_OBJID_TYPE_CHECK(attr_list[count].value.oid,
                                                  SAI_OBJECT_TYPE_PORT) &&
                        !XDK_SAI_OBJID_TYPE_CHECK(attr_list[count].value.oid, SAI_OBJECT_TYPE_LAG))
                    {
                        XP_SAI_LOG_ERR("Wrong Port/LAG OID type passed!\n");
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }

                    if (xpSaiObjIdValueGet(attr_list[count].value.oid) > MAX_PORTNUM &&
                        !XDK_SAI_OBJID_TYPE_CHECK(attr_list[count].value.oid, SAI_OBJECT_TYPE_LAG))
                    {
                        XP_SAI_LOG_ERR("Invalid Port: %lu\n",
                                       xpSaiObjIdValueGet(attr_list[count].value.oid));
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }

                    attributes->portId = attr_list[count].value;
                    break;
                }
            case SAI_ROUTER_INTERFACE_ATTR_VLAN_ID:
                {
                    if (!XDK_SAI_OBJID_TYPE_CHECK(attr_list[count].value.oid, SAI_OBJECT_TYPE_VLAN))
                    {
                        XP_SAI_LOG_ERR("Wrong Vlan OID type passed!\n");
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }

                    if (xpSaiObjIdValueGet(attr_list[count].value.oid) > XP_MAX_VLAN_NUM)
                    {
                        XP_SAI_LOG_ERR("Vlan OID is out of bounds!\n");
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }

                    attributes->vlanId = attr_list[count].value;
                    break;
                }
            case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
                {
                    /* Check whether MAC is valid */
                    saiRetVal = xpSaiIsValidUcastMac((uint8_t *)attr_list[count].value.mac);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS)\n");
                        return saiRetVal;
                    }

                    attributes->srcMacAddress = attr_list[count].value;
                    break;
                }
            case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
                {
                    attributes->adminV4State = attr_list[count].value;
                    break;
                }
            case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE:
                {
                    attributes->adminV6State = attr_list[count].value;
                    break;
                }
            case SAI_ROUTER_INTERFACE_ATTR_MTU:
                {
                    attributes->mtu = attr_list[count].value;
                    break;
                }
            case SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION:
                {
                    attributes->neighMissAction = attr_list[count].value;
                    break;
                }
            case SAI_ROUTER_INTERFACE_ATTR_INGRESS_ACL:
                {
                    attributes->ingressAcl = attr_list[count].value;
                    break;
                }
            case SAI_ROUTER_INTERFACE_ATTR_EGRESS_ACL:
                {
                    attributes->egressAcl = attr_list[count].value;
                    break;
                }
            case SAI_ROUTER_INTERFACE_ATTR_V4_MCAST_ENABLE:
                {
                    attributes->mcAdminV4State = attr_list[count].value;
                    break;
                }
            case SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE:
                {
                    attributes->mcAdminV6State = attr_list[count].value;
                    break;
                }
            case SAI_ROUTER_INTERFACE_ATTR_IS_VIRTUAL:
                {
                    attributes->isVirtual = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(count);
                }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetRouterInterfaceAttrVirtualRouterId

sai_status_t xpSaiSetRouterInterfaceAttrVirtualRouterId(sai_object_id_t rif_oid,
                                                        sai_attribute_value_t value)
{
    XP_STATUS         retVal    = XP_NO_ERR;
    xpsInterfaceId_t  l3IntfId  = 0;
    uint32_t          vrfId     = 0;
    sai_status_t      saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t       xpsDevId  = xpSaiObjIdSwitchGet(rif_oid);

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(rif_oid, &l3IntfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertVirtualRouterOid(value.oid, &vrfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertVirtualRouterOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    if (!xpSaiVrfExists(vrfId))
    {
        XP_SAI_LOG_ERR("No VRF with %d id exists\n", vrfId);
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    retVal = xpsL3SetIntfVrf(xpsDevId, l3IntfId, vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3SetIntfVrf() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    XP_SAI_LOG_DBG("\nSet L3 interface %d to VRF %d\n", l3IntfId, vrfId);

    retVal = xpSaiVrfIntfVrfIdUpdate(xpsDevId, l3IntfId, vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfVrfIdUpdate() failed, l3IntfId %d, vrfId %d, error code %d\n",
                       l3IntfId, vrfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetRouterInterfaceAttrVirtualRouterId

sai_status_t xpSaiGetRouterInterfaceAttrVirtualRouterId(sai_object_id_t rif_oid,
                                                        sai_attribute_value_t* value)
{
    XP_STATUS         retVal    = XP_NO_ERR;
    sai_status_t      saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t  l3IntfId  = 0;
    uint32_t          vrfId     = 0;
    xpsDevice_t       xpsDevId  = xpSaiObjIdSwitchGet(rif_oid);

    if (!value)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(rif_oid, &l3IntfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    retVal = xpsL3GetIntfVrf(xpsDevId, l3IntfId, &vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3GetIntfVrf() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
    }

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, xpsDevId,
                                 (sai_uint64_t)vrfId, &value->oid);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
        return saiRetVal;
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiSetRouterInterfaceAttrPortId

sai_status_t xpSaiSetRouterInterfaceAttrPortId(xpsInterfaceId_t l3IntfId,
                                               sai_attribute_value_t value)
{
    XP_STATUS         retVal    = XP_NO_ERR;
    sai_status_t      saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t  portId    = 0;
    xpsDevice_t       xpsDevId  = xpSaiObjIdSwitchGet(value.oid);

    saiRetVal = xpSaiConvertPortOid(value.oid, &portId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertPortOid() failed with error code: %d\n", saiRetVal);
        return saiRetVal;
    }

    retVal = xpsL3InitPortIntf(xpsDevId, l3IntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3InitPortIntf() for portId %d failed with error code: %d\n",
                       portId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsL3BindPortIntf(portId, l3IntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3BindPortIntf() for portId %d failed with error code: %d\n",
                       portId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    XP_SAI_LOG_DBG("\nPort %d is binded to L3 interface %d\n", portId, l3IntfId);

    retVal = xpSaiVrfIntfPortSet(l3IntfId, value.oid);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfPortSet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetRouterInterfaceAttrPortId

sai_status_t xpSaiGetRouterInterfaceAttrPortId(sai_object_id_t rif_id,
                                               sai_attribute_value_t* value)
{
    XP_STATUS         retVal    = XP_NO_ERR;
    sai_status_t      saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t  l3IntfId  = 0;
    sai_object_id_t   portOid   = 0;
    int32_t           rifType   = 0;

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(rif_id, &l3IntfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    retVal = xpSaiVrfIntfTypeGet(l3IntfId, &rifType);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfTypeGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (rifType != SAI_ROUTER_INTERFACE_TYPE_PORT &&
        rifType != SAI_ROUTER_INTERFACE_TYPE_SUB_PORT)
    {
        return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    retVal = xpSaiVrfIntfPortGet(l3IntfId, &portOid);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfPortGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    value->oid = portOid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBindRouterInterfaceSubPort

sai_status_t xpSaiBindRouterInterfaceSubPort(xpsInterfaceId_t l3IntfId,
                                             sai_attribute_value_t portOid, sai_attribute_value_t vlanOid)
{
    XP_STATUS         retVal    = XP_NO_ERR;
    sai_status_t      saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t  portId    = 0;

    saiRetVal = xpSaiConvertPortOid(portOid.oid, &portId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertPortOid() failed with error code: %d\n", saiRetVal);
        return saiRetVal;
    }

    retVal = xpsL3BindSubIntf(portId, l3IntfId, xpSaiObjIdValueGet(vlanOid.oid));
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3BindSubIntf() failed with error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiVrfIntfPortSet(l3IntfId, portOid.oid);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfPortSet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiVrfIntfVlanIdSet(l3IntfId, vlanOid.oid);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfVlanIdSet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetRouterInterfaceAttrVlanId

sai_status_t xpSaiGetRouterInterfaceAttrVlanId(sai_object_id_t rif_id,
                                               sai_attribute_value_t* value)
{
    XP_STATUS         retVal      = XP_NO_ERR;
    sai_status_t      saiRetVal   = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t  l3IntfId    = 0;
    int32_t           rifType     = 0;

    if (!value)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(rif_id, &l3IntfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    retVal = xpSaiVrfIntfTypeGet(l3IntfId, &rifType);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfTypeGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (rifType != SAI_ROUTER_INTERFACE_TYPE_VLAN &&
        rifType != SAI_ROUTER_INTERFACE_TYPE_SUB_PORT)
    {
        return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    retVal = xpSaiVrfIntfVlanIdGet(l3IntfId, &value->oid);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfVlanIdGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetRouterInterfaceAttrRifType(sai_object_id_t rif_id,
                                                sai_attribute_value_t* value)
{
    XP_STATUS         retVal    = XP_NO_ERR;
    sai_status_t      saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t  l3IntfId  = 0;
    int32_t           rifType = 0 ;

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(rif_id, &l3IntfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    retVal = xpSaiVrfIntfTypeGet(l3IntfId, &rifType);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfTypeGet for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    value->u32 = rifType;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetRouterInterfaceAttrSrcMacAddress

sai_status_t xpSaiSetRouterInterfaceAttrSrcMacAddress(sai_object_id_t rif_id,
                                                      sai_attribute_value_t value)
{
    XP_STATUS           retVal      = XP_NO_ERR;
    sai_status_t        saiRetVal   = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t    l3IntfId    = 0;
    uint16_t            virtId      = 0;
    xpsDevice_t         xpsDevId    = xpSaiObjIdSwitchGet(rif_id);
    sai_mac_t           mac;
    sai_mac_t           vrfMacAddr;
    uint32_t vrfId = 0;
    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    if ((pRifEntry->l3IntfId == XPS_INTF_INVALID_ID) &&
        (pRifEntry->rifType == SAI_ROUTER_INTERFACE_TYPE_BRIDGE))
    {
        /* No L3 interface created yet.
         * Only fill corresponding RIF entry field */
        COPY_MAC_ADDR_T(pRifEntry->mac, value.mac);

        return SAI_STATUS_SUCCESS;
    }

    l3IntfId = pRifEntry->l3IntfId;

    saiRetVal = xpSaiRouterInterfaceVirtIdGet(rif_id, &virtId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceVirtIdGet() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    // Remove old RIF MAC address from HW if any
    saiRetVal = xpSaiRemoveRouterInterfaceMacAddress(rif_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRemoveRouterInterfaceMacAddress() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    if (xpSaiEthAddrIsZero(value.mac))
    {
        XP_SAI_LOG_NOTICE("Zero MAC-address passed!\n");
        return saiRetVal;
    }

    COPY_MAC_ADDR_T(mac, value.mac);

    saiRetVal = xpSaiConvertVirtualRouterOid(pRifEntry->vrfId, &vrfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertVirtualRouterOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }
    retVal = xpSaiVrfMacAddressGet(xpsDevId, vrfId, vrfMacAddr);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfMacAddressGet() failed with error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /* Add the MAC address to the L3 interface */
    if (memcmp(vrfMacAddr, mac, sizeof(sai_mac_t)) == 0)
    {
        retVal = xpsL3AddIngressRouterMac(xpsDevId, mac);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Adding interface source mac for L3 interface %d failed with error %d\n",
                           l3IntfId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }
    else
    {
        retVal = xpsL3AddIntfIngressRouterMac(xpsDevId, l3IntfId, mac);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Adding interface source mac for L3 interface %d failed with error %d\n",
                           l3IntfId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    if (virtId > 0)
    {
        // On virtual RIF, it's allowed to set ingress router MAC address only
        COPY_MAC_ADDR_T(pRifEntry->rifInfo[virtId - 1].mac, mac);

        return saiRetVal;
    }

    retVal = xpsL3SetEgressRouterMac(xpsDevId, l3IntfId, mac);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3SetEgressRouterMac() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiVrfIntfSrcMacAddrSet(l3IntfId, mac);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfSrcMacAddrSet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetRouterInterfaceAttrSrcMacAddress

sai_status_t xpSaiGetRouterInterfaceAttrSrcMacAddress(sai_object_id_t rif_id,
                                                      sai_attribute_value_t* value)
{
    XP_STATUS         retVal      = XP_NO_ERR;
    sai_status_t      saiRetVal   = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t  l3IntfId    = 0;
    macAddr_t         macAddress;
    uint16_t          virtId      = 0;

    xpSaiRouterInterfaceDbEntryT *pRouterInterfaceEntry = NULL;

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(value->mac, 0x0, sizeof(sai_mac_t));

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(rif_id, &l3IntfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        memset(value->mac, 0x0, sizeof(sai_mac_t));
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiRouterInterfaceVirtIdGet(rif_id, &virtId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        memset(value->mac, 0x0, sizeof(sai_mac_t));
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceVirtIdGet() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    if (virtId > 0)
    {
        saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_id, &pRouterInterfaceEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            memset(value->mac, 0x0, sizeof(sai_mac_t));
            XP_SAI_LOG_ERR("Couldn't find intf %u in SAI Router Interface DB \n", l3IntfId);
            return saiRetVal;
        }

        memcpy(value->mac, pRouterInterfaceEntry->rifInfo[virtId - 1].mac,
               sizeof(sai_mac_t));
    }
    else
    {
        retVal = xpSaiVrfIntfSrcMacAddrGet(l3IntfId, macAddress);
        if (retVal != XP_NO_ERR)
        {
            memset(value->mac, 0x0, sizeof(sai_mac_t));
            XP_SAI_LOG_ERR("xpSaiVrfIntfSrcMacAddrGet() for l3IntfId %u failed with error code: %d\n",
                           l3IntfId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        memcpy(value->mac, macAddress, sizeof(sai_mac_t));
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiRemoveRouterInterfaceMacAddress(sai_object_id_t rif_id)
{
    XP_STATUS           retVal      = XP_NO_ERR;
    sai_status_t        saiRetVal   = SAI_STATUS_SUCCESS;
    xpsDevice_t         xpsDevId    = xpSaiObjIdSwitchGet(rif_id);
    xpsInterfaceId_t    l3IntfId    = 0;
    uint16_t            virtId      = 0;
    sai_mac_t vrfMacAddr;
    uint32_t vrfId = 0;

    sai_attribute_value_t   attrValue;
    sai_attribute_value_t   vrfAttr;

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(rif_id, &l3IntfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiRouterInterfaceVirtIdGet(rif_id, &virtId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceVirtIdGet() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiGetRouterInterfaceAttrSrcMacAddress(rif_id, &attrValue);
    if ((saiRetVal == SAI_STATUS_ITEM_NOT_FOUND) && (virtId > 0))
    {
        XP_SAI_LOG_NOTICE("Couldn't find vRIF in SAI Router Interface DB. Probably already removed as part of Native RIF\n");
        return SAI_STATUS_SUCCESS;
    }

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceAttrSrcMacAddress() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    if (xpSaiEthAddrIsZero(attrValue.mac))
    {
        return saiRetVal;
    }

    saiRetVal = xpSaiGetRouterInterfaceAttrVirtualRouterId(rif_id, &vrfAttr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("VirtualRouterId failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertVirtualRouterOid(vrfAttr.oid, &vrfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertVirtualRouterOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    retVal = xpSaiVrfMacAddressGet(xpsDevId, vrfId, vrfMacAddr);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfMacAddressGet() failed with error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /* Remove source mac address from the L3 interface */
    if (memcmp(vrfMacAddr, attrValue.mac, sizeof(sai_mac_t)) == 0)
    {
        retVal = xpsL3RemoveIngressRouterMac(xpsDevId, attrValue.mac);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to remove source mac address from L3 interface error code %d\n",
                           l3IntfId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }
    else
    {
        /* Remove source mac address from the L3 interface */
        retVal = xpsL3RemoveIntfIngressRouterMac(xpsDevId, l3IntfId, attrValue.mac);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to remove source mac address from L3 interface error code %d\n",
                           l3IntfId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    return saiRetVal;
}

//Func: xpSaiRouterInterfaceAdminV4StateApply

XP_STATUS xpSaiRouterInterfaceAdminV4StateApply(xpsDevice_t xpsDevId,
                                                xpsInterfaceId_t l3IntfId)
{
    XP_STATUS       retVal          = XP_NO_ERR;
    uint32_t        vrfId           = 0;
    uint32_t        vrfAdminState   = 0;
    uint32_t        intfAdminState  = 0;

    retVal = xpSaiVrfIntfVrfIdGet(l3IntfId, &vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfVrfIdGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    retVal = xpSaiVrfAdminV4StateGet(xpsDevId, vrfId, &vrfAdminState);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfAdminV4StateGet() for vrfId %u failed with error code: %d\n",
                       vrfId, retVal);
        return retVal;
    }

    retVal = xpSaiVrfIntfAdminV4StateGet(l3IntfId, &intfAdminState);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfAdminV4StateGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    retVal = xpsL3SetIntfIpv4UcRoutingEn(xpsDevId, l3IntfId, (vrfAdminState &&
                                                              intfAdminState));
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3SetIntfIpv4UcRoutingEn() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiRouterInterfaceMcAdminV4StateApply(xpsDevice_t xpsDevId,
                                                  xpsInterfaceId_t l3IntfId)
{
    XP_STATUS       retVal          = XP_NO_ERR;
    uint32_t        vrfId           = 0;
    uint32_t        vrfAdminState   = 0;
    uint32_t        intfAdminState  = 0;

    retVal = xpSaiVrfIntfVrfIdGet(l3IntfId, &vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfVrfIdGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    retVal = xpSaiVrfAdminV4StateGet(xpsDevId, vrfId, &vrfAdminState);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfAdminV4StateGet() for vrfId %u failed with error code: %d\n",
                       vrfId, retVal);
        return retVal;
    }

    retVal = xpSaiVrfIntfMcAdminV4StateGet(l3IntfId, &intfAdminState);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfMcAdminV4StateGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    retVal = xpsL3SetIntfIpv4McRoutingEn(xpsDevId, l3IntfId, (vrfAdminState &&
                                                              intfAdminState));
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3SetIntfIpv4McRoutingEn() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSetRouterInterfaceAttrAdminV4State

sai_status_t xpSaiSetRouterInterfaceAttrAdminV4State(sai_object_id_t rif_id,
                                                     sai_attribute_value_t value)
{
    XP_STATUS         retVal      = XP_NO_ERR;
    sai_status_t      saiRetVal   = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t  l3IntfId    = 0;
    xpsDevice_t       xpsDevId    = xpSaiObjIdSwitchGet(rif_id);

    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    if ((pRifEntry->l3IntfId == XPS_INTF_INVALID_ID) &&
        (pRifEntry->rifType == SAI_ROUTER_INTERFACE_TYPE_BRIDGE))
    {
        /* No L3 interface created yet.
         * Only fill corresponding RIF entry field */
        pRifEntry->adminV4State = value.booldata;

        return SAI_STATUS_SUCCESS;
    }

    l3IntfId = pRifEntry->l3IntfId;

    retVal = xpSaiVrfIntfAdminV4StateSet(l3IntfId, value.booldata);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfAdminV4StateSet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiRouterInterfaceAdminV4StateApply(xpsDevId, l3IntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceAdminV4StateApply() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    pRifEntry->adminV4State = value.booldata;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiSetRouterInterfaceAttrMcAdminV4State(sai_object_id_t rif_id,
                                                       sai_attribute_value_t value)
{
    XP_STATUS         retVal      = XP_NO_ERR;
    sai_status_t      saiRetVal   = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t  l3IntfId    = 0;
    xpsDevice_t       xpsDevId    = xpSaiObjIdSwitchGet(rif_id);

    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    if ((pRifEntry->l3IntfId == XPS_INTF_INVALID_ID) &&
        (pRifEntry->rifType == SAI_ROUTER_INTERFACE_TYPE_BRIDGE))
    {
        /* No L3 interface created yet.
         * Only fill corresponding RIF entry field */
        pRifEntry->mcAdminV4State = value.booldata;

        return SAI_STATUS_SUCCESS;
    }

    l3IntfId = pRifEntry->l3IntfId;

    retVal = xpSaiVrfIntfMcAdminV4StateSet(l3IntfId, value.booldata);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfMcAdminV4StateSet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiRouterInterfaceMcAdminV4StateApply(xpsDevId, l3IntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceMcAdminV4StateApply() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    pRifEntry->mcAdminV4State = value.booldata;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetRouterInterfaceAttrAdminV4State

sai_status_t xpSaiGetRouterInterfaceAttrAdminV4State(sai_object_id_t rif_id,
                                                     sai_attribute_value_t* value)
{
    XP_STATUS           retVal          = XP_NO_ERR;
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t    l3IntfId        = 0;
    uint32_t            intfAdminState  = 0;

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(rif_id, &l3IntfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    retVal = xpSaiVrfIntfAdminV4StateGet(l3IntfId, &intfAdminState);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfAdminV4StateGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    value->booldata = intfAdminState;

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetRouterInterfaceAttrMcAdminV4State(sai_object_id_t rif_id,
                                                       sai_attribute_value_t* value)
{
    XP_STATUS           retVal          = XP_NO_ERR;
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t    l3IntfId        = 0;
    uint32_t            intfAdminState  = 0;

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(rif_id, &l3IntfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    retVal = xpSaiVrfIntfMcAdminV4StateGet(l3IntfId, &intfAdminState);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfMcAdminV4StateGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    value->booldata = intfAdminState;

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiRouterInterfaceAdminV6StateApply

XP_STATUS xpSaiRouterInterfaceAdminV6StateApply(xpsDevice_t xpsDevId,
                                                xpsInterfaceId_t l3IntfId)
{
    XP_STATUS       retVal          = XP_NO_ERR;
    uint32_t        vrfId           = 0;
    uint32_t        vrfAdminState   = 0;
    uint32_t        intfAdminState  = 0;

    retVal = xpSaiVrfIntfVrfIdGet(l3IntfId, &vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfVrfIdGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    retVal = xpSaiVrfAdminV6StateGet(xpsDevId, vrfId, &vrfAdminState);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfAdminV6StateGet() for vrfId %u failed with error code: %d\n",
                       vrfId, retVal);
        return retVal;
    }

    retVal = xpSaiVrfIntfAdminV6StateGet(l3IntfId, &intfAdminState);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfAdminV6StateGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    retVal = xpsL3SetIntfIpv6UcRoutingEn(xpsDevId, l3IntfId, (vrfAdminState &&
                                                              intfAdminState));
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3SetIntfIpv6UcRoutingEn() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }


    return XP_NO_ERR;
}

XP_STATUS xpSaiRouterInterfaceMcAdminV6StateApply(xpsDevice_t xpsDevId,
                                                  xpsInterfaceId_t l3IntfId)
{
    XP_STATUS       retVal          = XP_NO_ERR;
    uint32_t        vrfId           = 0;
    uint32_t        vrfAdminState   = 0;
    uint32_t        intfAdminState  = 0;

    retVal = xpSaiVrfIntfVrfIdGet(l3IntfId, &vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfVrfIdGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    retVal = xpSaiVrfAdminV6StateGet(xpsDevId, vrfId, &vrfAdminState);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfAdminV6StateGet() for vrfId %u failed with error code: %d\n",
                       vrfId, retVal);
        return retVal;
    }

    retVal = xpSaiVrfIntfMcAdminV6StateGet(l3IntfId, &intfAdminState);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfMcAdminV6StateGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    retVal = xpsL3SetIntfIpv6McRoutingEn(xpsDevId, l3IntfId, (vrfAdminState &&
                                                              intfAdminState));
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3SetIntfIpv6McRoutingEn() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSetRouterInterfaceAttrAdminV6State

sai_status_t xpSaiSetRouterInterfaceAttrAdminV6State(sai_object_id_t rif_id,
                                                     sai_attribute_value_t value)
{
    XP_STATUS         retVal      = XP_NO_ERR;
    sai_status_t      saiRetVal   = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t  l3IntfId    = 0;
    xpsDevice_t       xpsDevId    = xpSaiObjIdSwitchGet(rif_id);

    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    if ((pRifEntry->l3IntfId == XPS_INTF_INVALID_ID) &&
        (pRifEntry->rifType == SAI_ROUTER_INTERFACE_TYPE_BRIDGE))
    {
        /* No L3 interface created yet.
         * Only fill corresponding RIF entry field */
        pRifEntry->adminV6State = value.booldata;

        return SAI_STATUS_SUCCESS;
    }

    l3IntfId = pRifEntry->l3IntfId;

    retVal = xpSaiVrfIntfAdminV6StateSet(l3IntfId, value.booldata);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfAdminV6StateSet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiRouterInterfaceAdminV6StateApply(xpsDevId, l3IntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceAdminV6StateApply() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    pRifEntry->adminV6State = value.booldata;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiSetRouterInterfaceAttrMcAdminV6State(sai_object_id_t rif_id,
                                                       sai_attribute_value_t value)
{
    XP_STATUS         retVal      = XP_NO_ERR;
    sai_status_t      saiRetVal   = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t  l3IntfId    = 0;
    xpsDevice_t       xpsDevId    = xpSaiObjIdSwitchGet(rif_id);

    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    if ((pRifEntry->l3IntfId == XPS_INTF_INVALID_ID) &&
        (pRifEntry->rifType == SAI_ROUTER_INTERFACE_TYPE_BRIDGE))
    {
        /* No L3 interface created yet.
         * Only fill corresponding RIF entry field */
        pRifEntry->mcAdminV6State = value.booldata;

        return SAI_STATUS_SUCCESS;
    }

    l3IntfId = pRifEntry->l3IntfId;

    retVal = xpSaiVrfIntfMcAdminV6StateSet(l3IntfId, value.booldata);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfAdminV6StateSet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiRouterInterfaceMcAdminV6StateApply(xpsDevId, l3IntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceMcAdminV6StateApply() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    pRifEntry->mcAdminV6State = value.booldata;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetRouterInterfaceAttrAdminV6State

sai_status_t xpSaiGetRouterInterfaceAttrAdminV6State(sai_object_id_t rif_id,
                                                     sai_attribute_value_t* value)
{
    XP_STATUS           retVal          = XP_NO_ERR;
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t    l3IntfId        = 0;
    uint32_t            intfAdminState  = 0;

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(rif_id, &l3IntfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    retVal = xpSaiVrfIntfAdminV6StateGet(l3IntfId, &intfAdminState);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfAdminV6StateGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    value->booldata = intfAdminState;

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetRouterInterfaceAttrMcAdminV6State(sai_object_id_t rif_id,
                                                       sai_attribute_value_t* value)
{
    XP_STATUS           retVal          = XP_NO_ERR;
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t    l3IntfId        = 0;
    uint32_t            intfAdminState  = 0;

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(rif_id, &l3IntfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    retVal = xpSaiVrfIntfMcAdminV6StateGet(l3IntfId, &intfAdminState);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfMcAdminV6StateGet() for l3IntfId %u failed with error code: %d\n",
                       l3IntfId, retVal);
        return retVal;
    }

    value->booldata = intfAdminState;

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiSetRouterInterfaceAttrMtu

sai_status_t xpSaiSetRouterInterfaceAttrMtu(sai_object_id_t rif_id,
                                            sai_attribute_value_t value)
{
    XP_STATUS                    retVal    = XP_NO_ERR;
    sai_status_t                 saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    if ((pRifEntry->l3IntfId == XPS_INTF_INVALID_ID) &&
        (pRifEntry->rifType == SAI_ROUTER_INTERFACE_TYPE_BRIDGE))
    {
        /* No L3 interface created yet.
         * Only fill corresponding RIF entry field */
        pRifEntry->mtu = value.u32;

        return SAI_STATUS_SUCCESS;
    }

    retVal = xpsL3SetIpv4MtuLenForInterface(xpSaiGetDevId(), pRifEntry->l3IntfId,
                                            value.u32);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3SetIpv4MtuLenForInterface() failed with error code: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsL3SetIpv6MtuLenForInterface(xpSaiGetDevId(), pRifEntry->l3IntfId,
                                            value.u32);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3SetIpv6MtuLenForInterface() failed with error code: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    pRifEntry->mtu = value.u32;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetRouterInterfaceAttrMtu

sai_status_t xpSaiGetRouterInterfaceAttrMtu(sai_object_id_t rif_id,
                                            sai_attribute_value_t *value)
{
    sai_status_t                 saiRetVal  = SAI_STATUS_SUCCESS;
    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    value->u32 = pRifEntry->mtu;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRouterInterfaceOnBridgePortAdded

sai_status_t xpSaiRouterInterfaceOnBridgePortAdded(sai_object_id_t rif_oid,
                                                   sai_object_id_t bridge_oid, xpsInterfaceId_t *l3_intf_id)
{
    XP_STATUS                    retVal     = XP_NO_ERR;
    sai_status_t                 saiRetVal  = SAI_STATUS_SUCCESS;
    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;
    sai_attribute_value_t        mac, vrfId, mtu, adminV4State, adminV6State;
    xpsVlan_t                    bdId       = 0;

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_oid, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    if (pRifEntry->rifType != SAI_ROUTER_INTERFACE_TYPE_BRIDGE)
    {
        XP_SAI_LOG_ERR("Invalid RIF type passed: %d!\n", pRifEntry->rifType);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    pRifEntry->brOid = bridge_oid;

    bdId = (xpsVlan_t)xpSaiObjIdValueGet(bridge_oid);

    retVal = xpsL3CreateVlanIntf(bdId, &pRifEntry->l3IntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3CreateVlanIntf() failed with retVal: %d!\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiVrfIntfAdd(pRifEntry->l3IntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfAdd() for interface %u failed with error code: %d\n",
                       pRifEntry->l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiVrfIntfTypeSet(pRifEntry->l3IntfId, pRifEntry->rifType);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfRifTypeSet() for l3IntfId %u failed with error code: %d\n",
                       pRifEntry->l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    *l3_intf_id = pRifEntry->l3IntfId;

    /* Setup attributes, related to L3 interface ID */

    COPY_MAC_ADDR_T(mac.mac, pRifEntry->mac);
    vrfId.oid = pRifEntry->vrfId;
    mtu.u32 = pRifEntry->mtu;
    adminV4State.booldata = pRifEntry->adminV4State;
    adminV6State.booldata = pRifEntry->adminV6State;

    saiRetVal = xpSaiSetRouterInterfaceAttrSrcMacAddress(rif_oid, mac);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS)\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiSetRouterInterfaceAttrVirtualRouterId(rif_oid, vrfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID)\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiSetRouterInterfaceAttrMtu(rif_oid, mtu);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_MTU)\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiSetRouterInterfaceAttrAdminV4State(rif_oid, adminV4State);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE)\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiSetRouterInterfaceAttrAdminV6State(rif_oid, adminV6State);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE)\n");
        return saiRetVal;
    }

    adminV4State.booldata = pRifEntry->mcAdminV4State;
    adminV6State.booldata = pRifEntry->mcAdminV6State;

    saiRetVal = xpSaiSetRouterInterfaceAttrMcAdminV4State(rif_oid, adminV4State);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_V4_MCAST_ENABLE)\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiSetRouterInterfaceAttrMcAdminV6State(rif_oid, adminV6State);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE)\n");
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRouterInterfaceOnBridgePortRemoved

sai_status_t xpSaiRouterInterfaceOnBridgePortRemoved(sai_object_id_t rif_oid)
{
    XP_STATUS                     retVal    = XP_NO_ERR;
    sai_status_t                  saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_oid, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiRemoveRouterInterfaceMacAddress(rif_oid);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRemoveRouterInterfaceMacAddress() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    retVal = xpSaiVrfIntfRemove(pRifEntry->l3IntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfIntfRemove() for interface %u failed with error code: %d\n",
                       pRifEntry->l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsL3DestroyVlanIntf((xpsVlan_t)xpSaiObjIdValueGet(pRifEntry->brOid),
                                  pRifEntry->l3IntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3DestroyVlanIntf() failed with retVal: %d!\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    pRifEntry->l3IntfId = XPS_INTF_INVALID_ID;
    pRifEntry->brOid = SAI_NULL_OBJECT_ID;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAllocateInterfaceVirtualRifId

sai_status_t xpSaiAllocVirtRifId(xpSaiRouterInterfaceDbEntryT *pRifEntry,
                                 uint16_t *pVirtId)
{
    if ((pRifEntry == NULL) || (pVirtId == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (pRifEntry->rifCount == XP_SAI_INTERFACE_VIRTUAL_RIF_MAX)
    {
        XP_SAI_LOG_ERR("Cannot allocate ID for vRIF creation, IDs exhausted\n");
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    for (uint16_t i = 0; i < XP_SAI_INTERFACE_VIRTUAL_RIF_MAX; i++)
    {
        if (pRifEntry->rifInfo[i].id == 0)
        {
            if (pRifEntry->rifCount >= (XDK_SAI_VIRT_RIF_OBJID_VALUE_MASK >>
                                        XDK_SAI_VIRT_RIF_OBJID_VALUE_OFFSET))
            {
                XP_SAI_LOG_ERR("Cannot allocate ID for vRIF creation, vRIF OID range overflow\n");
                return SAI_STATUS_INSUFFICIENT_RESOURCES;
            }

            pRifEntry->rifCount++;
            *pVirtId = pRifEntry->rifInfo[i].id = i + 1;

            return SAI_STATUS_SUCCESS;
        }
    }

    return SAI_STATUS_FAILURE;
}

sai_status_t xpSaiFreeVirtRifId(xpSaiRouterInterfaceDbEntryT *pRifEntry,
                                uint16_t virtId)
{
    if (virtId == 0)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (pRifEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((pRifEntry->rifCount == 0) || (pRifEntry->rifInfo[virtId - 1].id == 0))
    {
        XP_SAI_LOG_NOTICE("Virtual RIF %u not found. Probably already removed as part of Native RIF %u\n",
                          virtId, pRifEntry->keyRifId);
        return SAI_STATUS_SUCCESS;
    }

    pRifEntry->rifCount--;
    memset(&pRifEntry->rifInfo[virtId - 1], 0,
           sizeof(xpSaiInterfaceVirtualRifInfoT));

    return SAI_STATUS_SUCCESS;
}

static xpSaiRouterInterfaceDbEntryT *xpSaiFindRifEntry(sai_object_id_t port_oid,
                                                       sai_object_id_t vlan_oid)
{
    xpSaiRouterInterfaceDbEntryT    *pRifEntry  = NULL;
    XP_STATUS                       retVal      = XP_NO_ERR;

    static xpSaiRouterInterfaceDbEntryT rifEntryKey;

    retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, gXpSaiRouterInterfaceDbHandle,
                                 NULL, (void **)&pRifEntry);
    if (retVal != XP_NO_ERR)
    {
        return NULL;
    }

    do
    {
        if (pRifEntry == NULL)
        {
            return NULL;
        }

        if ((pRifEntry->portOid == port_oid) && (pRifEntry->vlanOid == vlan_oid))
        {
            return pRifEntry;
        }

        rifEntryKey.keyRifId = pRifEntry->keyRifId;
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, gXpSaiRouterInterfaceDbHandle,
                                     &rifEntryKey, (void **)&pRifEntry);
        if (retVal != XP_NO_ERR)
        {
            return NULL;
        }
    }
    while (pRifEntry != NULL);

    return NULL;
}

XP_STATUS xpSaiInsertRifStatisticDbEntry(sai_int32_t l3IntfId)
{
    xpSaiRouterInterfaceStatisticDbEntryT *rifStatisticKey;
    xpSaiRouterInterfaceStatisticDbEntryT *ppRifStatisticInfo = NULL;
    XP_STATUS           xpsRetVal = XP_NO_ERR;

    xpsRetVal = xpsStateHeapMalloc(sizeof(xpSaiRouterInterfaceStatisticDbEntryT),
                                   (void**)&rifStatisticKey);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate rif stats structure\n");
        return xpsRetVal;
    }

    memset(rifStatisticKey, 0, sizeof(xpSaiRouterInterfaceStatisticDbEntryT));

    rifStatisticKey->keyRifId = l3IntfId;
    /* Retrieve the corresponding state from sai rif statistic db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT,
                                        gXpSaiRifStatisticDbHandle, (xpsDbKey_t)rifStatisticKey,
                                        (void**)&ppRifStatisticInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai rif statistic data |retVal:%d\n",
                       xpsRetVal);
        xpsStateHeapFree((void*)rifStatisticKey);
        return xpsRetVal;
    }

    if (ppRifStatisticInfo == NULL)
    {
        /* Create a new sai port statistic db */
        if ((xpsRetVal = xpsStateHeapMalloc(sizeof(
                                                xpSaiRouterInterfaceStatisticDbEntryT),
                                            (void**)&ppRifStatisticInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Heap Allocation for sai rif statistic db failed |retVal:%d\n",
                           xpsRetVal);
            xpsStateHeapFree((void*)rifStatisticKey);
            return xpsRetVal;
        }
        memset(ppRifStatisticInfo, 0, sizeof(xpSaiRouterInterfaceStatisticDbEntryT));

        ppRifStatisticInfo->keyRifId = l3IntfId;
        /* Insert the state into the database */
        if ((xpsRetVal = xpsStateInsertData(XP_SCOPE_DEFAULT,
                                            gXpSaiRifStatisticDbHandle, (void*)ppRifStatisticInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to insert sai rif statistic data |retVal: %d\n",
                           xpsRetVal);
            xpsStateHeapFree((void*)ppRifStatisticInfo);
            xpsStateHeapFree((void*)rifStatisticKey);
            return xpsRetVal;
        }
    }

    xpsStateHeapFree((void*)rifStatisticKey);

    return XP_NO_ERR;
}

//Func: xpSaiRemovePortStatisticDbEntry

XP_STATUS xpSaiRemoveRifStatisticDbEntry(sai_int32_t l3IntfId)
{
    xpSaiRouterInterfaceStatisticDbEntryT *rifStatisticKey;
    xpSaiRouterInterfaceStatisticDbEntryT *ppRifStatisticInfo = NULL;
    XP_STATUS           xpsRetVal = XP_NO_ERR;

    xpsRetVal = xpsStateHeapMalloc(sizeof(xpSaiRouterInterfaceStatisticDbEntryT),
                                   (void**)&rifStatisticKey);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate rif stats structure\n");
        return xpsRetVal;
    }

    memset(rifStatisticKey, 0, sizeof(xpSaiRouterInterfaceStatisticDbEntryT));

    rifStatisticKey->keyRifId = l3IntfId;

    /* Retrieve the corresponding rif statistics from sai port statistic db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT,
                                        gXpSaiRifStatisticDbHandle, (xpsDbKey_t)rifStatisticKey,
                                        (void**)&ppRifStatisticInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai rif statistic data |retVal:%d\n",
                       xpsRetVal);
        xpsStateHeapFree((void*)rifStatisticKey);
        return xpsRetVal;
    }

    if (ppRifStatisticInfo == NULL)
    {
        XP_SAI_LOG_ERR("Entry doesn't exist for this sai rif statistics data |retVal:%d\n",
                       xpsRetVal);
        xpsStateHeapFree((void*)rifStatisticKey);
        return xpsRetVal;
    }
    else
    {
        xpsRetVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, gXpSaiRifStatisticDbHandle,
                                       (xpsDbKey_t)rifStatisticKey, (void**)&ppRifStatisticInfo);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not delete entry from Rif Statistic DB error %d\n",
                           xpsRetVal);
            xpsStateHeapFree((void*)rifStatisticKey);
            return xpsRetVal;
        }
    }

    xpsStateHeapFree((void*)rifStatisticKey);
    return XP_NO_ERR;
}


static sai_status_t xpSaiFreeRifDbEntry(sai_object_id_t switch_id,
                                        xpSaiRouterInterfaceDbEntryT *pRifEntry)
{
    sai_status_t        saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS           retVal    = XP_NO_ERR;
    xpsDevice_t         devId     = xpSaiObjIdSwitchGet(switch_id);

    if (pRifEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiRemoveRifStatisticDbEntry(pRifEntry->l3IntfId);
    if (XP_NO_ERR != retVal)
    {
        XP_SAI_LOG_ERR("Could not remove entry in rif statistic DB for rif Intf %u.",
                       pRifEntry->l3IntfId);
        return xpsStatus2SaiStatus(retVal);
    }

    if (pRifEntry->rifType != SAI_ROUTER_INTERFACE_TYPE_BRIDGE)
    {
        /* Reset the HW vrf-id to default. */
        retVal = xpsL3SetIntfVrf(devId, pRifEntry->l3IntfId, 0);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsL3SetIntfVrf() for l3IntfId %u failed with error code: %d\n",
                           pRifEntry->l3IntfId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        retVal = xpSaiVrfIntfRemove(pRifEntry->l3IntfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiVrfIntfRemove() for interface %u failed with error code: %d\n",
                           pRifEntry->l3IntfId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    saiRetVal = xpSaiConfigIpCtrlTrap(switch_id, true);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set (xpSaiConfigIpCtrlTrap)\n");
        return saiRetVal;
    }

    switch (pRifEntry->rifType)
    {
        case SAI_ROUTER_INTERFACE_TYPE_PORT:
            {
                retVal = xpsL3DestroyPortIntf(pRifEntry->l3IntfId);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsL3DestroyPortIntf() failed with error code: %d\n", retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_TYPE_VLAN:
            {
                retVal = xpsL3DestroyVlanIntf((xpsVlan_t)xpSaiObjIdValueGet(pRifEntry->vlanOid),
                                              pRifEntry->l3IntfId);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsL3DestroyVlanIntf() failed with error code: %d\n", retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_TYPE_SUB_PORT:
            {
                retVal = xpsL3DeInitSubIntf(devId, pRifEntry->l3IntfId);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsL3DeInitSubIntf() for l3IntfId %d failed with error code: %d\n",
                                   pRifEntry->l3IntfId, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                retVal = xpsL3DestroySubIntf(pRifEntry->l3IntfId);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsL3DestroySubIntf() failed with error code: %d\n", retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_TYPE_BRIDGE:
            {
                if (pRifEntry->l3IntfId != XPS_INTF_INVALID_ID)
                {
                    XP_SAI_LOG_ERR("Bridge router L3 Interface (%d) still present!\n",
                                   pRifEntry->l3IntfId);
                    return SAI_STATUS_OBJECT_IN_USE;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unsupported RIF type %d\n", pRifEntry->rifType);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_RIF,
                                   pRifEntry->keyRifId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_WARNING("Could not release router interface id %d\n",
                           pRifEntry->keyRifId);
    }

    saiRetVal = xpSaiRouterInterfaceDbDelete(pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbDelete() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiAllocRifDbEntry(sai_object_id_t switch_id,
                                         const xpSaiRouterInterfaceAttributesT * const rifAttr,
                                         xpSaiRouterInterfaceDbEntryT **ppRifEntry)
{
    xpSaiRouterInterfaceDbEntryT    *pRifEntry      = NULL;

    sai_status_t        saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS           retVal    = XP_NO_ERR;
    xpsInterfaceId_t    l3IntfId  = XPS_INTF_INVALID_ID;
    uint32_t            rifId     = 0;
    xpsDevice_t         devId     = xpSaiObjIdSwitchGet(switch_id);

    static xpSaiRouterInterfaceDbEntryT rifEntry;

    if (ppRifEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (rifAttr->portType.s32)
    {
        case SAI_ROUTER_INTERFACE_TYPE_PORT:
            {
                pRifEntry = xpSaiFindRifEntry(rifAttr->portId.oid, SAI_NULL_OBJECT_ID);
                if (pRifEntry != NULL)
                {
                    *ppRifEntry = pRifEntry;
                    return SAI_STATUS_ITEM_ALREADY_EXISTS;
                }

                /* Remove port from VLAN before assigning it as router interface */
                saiRetVal = xpSaiExcludeIntfAllVlans(rifAttr->portId.oid);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiExcludeIntfAllVlans() failed with error code: %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }

                retVal = xpsL3CreatePortIntf(&l3IntfId);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsL3CreatePortIntf() failed with error code: %d\n", retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_TYPE_VLAN:
            {
                if (!XDK_SAI_OBJID_TYPE_CHECK(rifAttr->vlanId.oid, SAI_OBJECT_TYPE_VLAN))
                {
                    XP_SAI_LOG_ERR("For wrong object type received(%u).\n",
                                   xpSaiObjIdTypeGet(rifAttr->vlanId.oid));
                    return SAI_STATUS_INVALID_ATTR_VALUE_0;
                }

                pRifEntry = xpSaiFindRifEntry(SAI_NULL_OBJECT_ID, rifAttr->vlanId.oid);
                if (pRifEntry != NULL)
                {
                    *ppRifEntry = pRifEntry;
                    return SAI_STATUS_ITEM_ALREADY_EXISTS;
                }

                retVal = xpsL3CreateVlanIntf((xpsVlan_t)xpSaiObjIdValueGet(rifAttr->vlanId.oid),
                                             &l3IntfId);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsL3CreateVlanIntf() for vlanId %lu failed with error code: %d\n",
                                   xpSaiObjIdValueGet(rifAttr->vlanId.oid), retVal);
                    if (XP_ERR_RESOURCE_NOT_AVAILABLE == retVal)
                    {
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }
                    return xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_TYPE_LOOPBACK:
            {
                /* DB entry is not required for Loopback RIF */
                *ppRifEntry = NULL;
                return SAI_STATUS_SUCCESS;
            }
        case SAI_ROUTER_INTERFACE_TYPE_SUB_PORT:
            {
                pRifEntry = xpSaiFindRifEntry(rifAttr->portId.oid, rifAttr->vlanId.oid);
                if (pRifEntry != NULL)
                {
                    *ppRifEntry = pRifEntry;
                    return SAI_STATUS_ITEM_ALREADY_EXISTS;
                }

                retVal = xpsL3CreateSubIntf(&l3IntfId);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsL3CreateSubIntf() failed with error code: %d\n", retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                retVal = xpsL3InitSubIntf(devId, l3IntfId);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsL3InitSubIntf() failed with error code: %d\n", retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_TYPE_BRIDGE:
            {
                XP_SAI_LOG_DBG("SAI_ROUTER_INTERFACE_TYPE_BRIDGE case handling...\n");

                l3IntfId = XPS_INTF_INVALID_ID;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Port type %d is unknown\n", rifAttr->portType.s32);
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
    }

    // Allocate RIF
    retVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_RIF, &rifId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate router interface id, error %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    memset(&rifEntry, 0, sizeof(rifEntry));

    rifEntry.keyRifId   = rifId;
    rifEntry.vrfId      = rifAttr->virtualRouterId.oid;
    rifEntry.rifType    = rifAttr->portType.s32;
    rifEntry.l3IntfId   = l3IntfId;
    rifEntry.portOid    = rifAttr->portId.oid;
    rifEntry.vlanOid    = rifAttr->vlanId.oid;
    rifEntry.rifCreated = false;

    saiRetVal = xpSaiRouterInterfaceDbInsert(&rifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not insert Router Interface info into DB");
        return saiRetVal;
    }

    if (rifAttr->portType.s32 != SAI_ROUTER_INTERFACE_TYPE_BRIDGE)
    {
        /* Add and fill VrfIntf DB entry only if we handle not
         * a SAI_ROUTER_INTERFACE_TYPE_BRIDGE RIF type!
         * For SAI_ROUTER_INTERFACE_TYPE_BRIDGE, L3 interface will be created
         * later, from bridge port APIs, using specific callback.
         */
        retVal = xpSaiVrfIntfAdd(rifEntry.l3IntfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiVrfIntfAdd() for interface %u failed with error code: %d\n",
                           rifEntry.l3IntfId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        retVal = xpSaiVrfIntfTypeSet(rifEntry.l3IntfId, rifEntry.rifType);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiVrfIntfRifTypeSet() for l3IntfId %u failed with error code: %d\n",
                           rifEntry.l3IntfId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gXpSaiRouterInterfaceDbHandle,
                                &rifEntry, (void **)ppRifEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData() failed with error code: %d!\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiInsertRifStatisticDbEntry(l3IntfId);
    if (XP_NO_ERR != retVal)
    {
        XP_SAI_LOG_ERR("Could not insert entry in rif statistic DB for rif %u.",
                       l3IntfId);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiCreateVirtRif(sai_object_id_t *rif_id,
                                       sai_object_id_t switch_id,
                                       const xpSaiRouterInterfaceAttributesT * const rifAttr)
{
    xpSaiRouterInterfaceDbEntryT    *pRifEntry      = NULL;

    xpsDevice_t         devId     = xpSaiObjIdSwitchGet(switch_id);
    sai_status_t        saiRetVal = SAI_STATUS_SUCCESS;
    uint16_t            virtId    = 0;

    if (rif_id == NULL || rifAttr == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((rifAttr->portType.s32 == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK)
        || (rifAttr->portType.s32 == SAI_ROUTER_INTERFACE_TYPE_BRIDGE))
    {
        XP_SAI_LOG_ERR("Virtual RIF is not supported on RIF with type %d\n",
                       rifAttr->portType.s32);
        return SAI_STATUS_NOT_SUPPORTED;
    }

    saiRetVal = xpSaiAllocRifDbEntry(switch_id, rifAttr, &pRifEntry);
    if (((saiRetVal != SAI_STATUS_SUCCESS) &&
         (saiRetVal != SAI_STATUS_ITEM_ALREADY_EXISTS))
        || (pRifEntry == NULL))
    {
        XP_SAI_LOG_ERR("Failed to allocate RIF DB entry. Error %u\n", saiRetVal);
        return saiRetVal;
    }

    if (rifAttr->portType.s32 != pRifEntry->rifType)
    {
        XP_SAI_LOG_ERR("Port type mismatch for virtual RIF\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiAllocVirtRifId(pRifEntry, &virtId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, saiRetVal %d", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiRouterInterfaceObjIdCreate(devId, pRifEntry->keyRifId, virtId,
                                                rif_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, saiRetVal %d", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiSetRouterInterfaceAttrSrcMacAddress(*rif_id,
                                                         rifAttr->srcMacAddress);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS)\n");
        return saiRetVal;
    }

    if (!xpSaiEthAddrIsZero((rifAttr->srcMacAddress).mac))
    {
        saiRetVal = xpSaiConfigIpCtrlTrap(*rif_id, false);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to set (xpSaiConfigIpCtrlTrap)\n");
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiCreatePhysRif(sai_object_id_t *rif_id,
                                       sai_object_id_t switch_id,
                                       const xpSaiRouterInterfaceAttributesT * const rifAttr)
{
    xpSaiRouterInterfaceDbEntryT    *pRifEntry      = NULL;

    xpsDevice_t         devId     = xpSaiObjIdSwitchGet(switch_id);
    sai_status_t        saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS           retVal    = XP_NO_ERR;
    xpsInterfaceId_t    xpsPortId = 0;
    xpsInterfaceType_e  intfType  = XPS_PORT;

    if (rif_id == NULL || rifAttr == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (rifAttr->portType.s32 == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK)
    {
        sai_uint32_t loopback_rif = 0;

        saiRetVal = xpSaiRouterInterfaceLoopbackIdAllocate(&loopback_rif);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to allocate router interface loopback id, error %d\n",
                           saiRetVal);
            return saiRetVal;
        }

        // Allocate loopback rif
        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ROUTER_INTERFACE, devId,
                                     (sai_uint64_t) loopback_rif, rif_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Loopback interface oid could not be created, saiRetVal %d",
                           saiRetVal);
            return saiRetVal;
        }

        return SAI_STATUS_SUCCESS;
    }

    saiRetVal = xpSaiAllocRifDbEntry(switch_id, rifAttr, &pRifEntry);
    if (((saiRetVal != SAI_STATUS_SUCCESS) &&
         (saiRetVal != SAI_STATUS_ITEM_ALREADY_EXISTS))
        || (pRifEntry == NULL))
    {
        XP_SAI_LOG_ERR("Failed to allocate RIF DB entry. Error %u\n", saiRetVal);
        return saiRetVal;
    }

    if ((saiRetVal == SAI_STATUS_ITEM_ALREADY_EXISTS) &&
        (pRifEntry->rifCreated == true))
    {
        XP_SAI_LOG_ERR("RIF already exists. Error %u\n", saiRetVal);
        return saiRetVal;
    }

    pRifEntry->mtu             = rifAttr->mtu.u32;
    pRifEntry->neighMissAction = rifAttr->neighMissAction.u32;
    pRifEntry->adminV4State    = rifAttr->adminV4State.booldata;
    pRifEntry->adminV6State    = rifAttr->adminV6State.booldata;
    pRifEntry->mcAdminV4State    = rifAttr->mcAdminV4State.booldata;
    pRifEntry->mcAdminV6State    = rifAttr->mcAdminV6State.booldata;
    COPY_MAC_ADDR_T(pRifEntry->mac, rifAttr->srcMacAddress.mac);

    saiRetVal = xpSaiRouterInterfaceObjIdCreate(devId, pRifEntry->keyRifId, 0,
                                                rif_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, saiRetVal %d", saiRetVal);
        return saiRetVal;
    }

    if (rifAttr->portType.s32 == SAI_ROUTER_INTERFACE_TYPE_BRIDGE)
    {
        return SAI_STATUS_SUCCESS;
    }

    switch (rifAttr->portType.s32)
    {
        case SAI_ROUTER_INTERFACE_TYPE_PORT:
            {
                saiRetVal = xpSaiSetRouterInterfaceAttrPortId(pRifEntry->l3IntfId,
                                                              rifAttr->portId);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set port id\n");
                    return saiRetVal;
                }

                saiRetVal = xpSaiConvertPortOid(rifAttr->portId.oid, &xpsPortId);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("xpSaiConvertPortOid() failed with error code: %d\n", saiRetVal);
                    return saiRetVal;
                }

                retVal = xpsInterfaceGetType(xpsPortId, &intfType);
                if (XP_NO_ERR != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get type for interface %u, error: %d\n", xpsPortId,
                                   retVal);
                    return retVal;
                }

                /* Add LAG reference when Router interface is LAG */
                if (XPS_LAG == intfType)
                {
                    xpSaiLagInfo_t *lagConfig = NULL;
                    saiRetVal = xpSaiLagRef(xpsPortId);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Unable to add LAG reference, sai_error: %d\n", saiRetVal);
                        return XP_ERR_INVALID_ID;
                    }
                    saiRetVal = xpSaiLagConfigInfoGet(xpsPortId, &lagConfig);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", xpsPortId,
                                       saiRetVal);
                        return XP_PORT_NOT_VALID;
                    }

                    xpsInterfaceInfo_t * l3IntfInfo = NULL;
                    retVal = xpsInterfaceGetInfoScope(XP_SCOPE_DEFAULT, pRifEntry->l3IntfId,
                                                      &l3IntfInfo);
                    if (retVal != XP_NO_ERR || (l3IntfInfo == NULL))
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Failed to get mapped L3 interface with port inteface %d\n",
                              pRifEntry->l3IntfId);
                        return xpsStatus2SaiStatus(retVal);
                    }
                    lagConfig->lagConfig.portVlanId = l3IntfInfo->bdHWId;

                    pRifEntry->prevPortPktCmd = lagConfig->lagConfig.saMissCmd;
                    saiRetVal = xpSaiLagSetSaMissCmd(xpsPortId, XP_PKTCMD_FWD);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("Failed to set SMAC miss for LAG %u, error %u\n", xpsPortId,
                                       saiRetVal);
                        return saiRetVal;
                    }

                }
                else
                {
                    // store current value of Port saMissCmd to DB
                    retVal = xpsPortGetField(devId, xpsPortId, XPS_PORT_MAC_SAMISSCMD,
                                             &pRifEntry->prevPortPktCmd);
                    if (XP_NO_ERR != retVal)
                    {
                        XP_SAI_LOG_ERR("Failed to get SA miss command for Port %u, error: %d\n",
                                       xpsPortId, retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }

                    // disable learning on the port
                    retVal = xpsPortSetField(devId, xpsPortId, XPS_PORT_MAC_SAMISSCMD,
                                             XP_PKTCMD_FWD);
                    if (XP_NO_ERR != retVal)
                    {
                        XP_SAI_LOG_ERR("Failed to set SA miss command for Port %u, error: %d\n",
                                       xpsPortId, retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_TYPE_VLAN:
            {
                retVal = xpSaiVrfIntfVlanIdSet(pRifEntry->l3IntfId, rifAttr->vlanId.oid);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiVrfIntfVlanIdSet() for l3IntfId %u failed with error code: %d\n",
                                   pRifEntry->l3IntfId, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_TYPE_SUB_PORT:
            {
                saiRetVal = xpSaiConvertPortOid(rifAttr->portId.oid, &xpsPortId);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("xpSaiConvertPortOid() failed with error code: %d\n", saiRetVal);
                    return saiRetVal;
                }

                // store current value of Port saMissCmd to DB
                retVal = xpsPortGetField(devId, xpsPortId, XPS_PORT_MAC_SAMISSCMD,
                                         &pRifEntry->prevPortPktCmd);
                if (XP_NO_ERR != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get SA miss command for Port %u, error: %d\n",
                                   xpsPortId, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                // disable learning on the port
                retVal = xpsPortSetField(devId, xpsPortId, XPS_PORT_MAC_SAMISSCMD,
                                         XP_PKTCMD_FWD);
                if (XP_NO_ERR != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set SA miss command for Port %u, error: %d\n",
                                   xpsPortId, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                saiRetVal = xpSaiBindRouterInterfaceSubPort(pRifEntry->l3IntfId,
                                                            rifAttr->portId, rifAttr->vlanId);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set sub-port attributes\n");
                    return saiRetVal;
                }
                break;
            }
    }

    saiRetVal = xpSaiSetRouterInterfaceAttrSrcMacAddress(*rif_id,
                                                         rifAttr->srcMacAddress);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS)\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiSetRouterInterfaceAttrVirtualRouterId(*rif_id,
                                                           rifAttr->virtualRouterId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID)\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiSetRouterInterfaceAttrMtu(*rif_id, rifAttr->mtu);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_MTU)\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiSetRouterInterfaceAttrAdminV4State(*rif_id,
                                                        rifAttr->adminV4State);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE)\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiSetRouterInterfaceAttrAdminV6State(*rif_id,
                                                        rifAttr->adminV6State);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE)\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiSetRouterInterfaceAttrMcAdminV4State(*rif_id,
                                                          rifAttr->mcAdminV4State);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_V4_MCAST_ENABLE)\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiSetRouterInterfaceAttrMcAdminV6State(*rif_id,
                                                          rifAttr->mcAdminV6State);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE)\n");
        return saiRetVal;
    }

    if (!xpSaiEthAddrIsZero((rifAttr->srcMacAddress).mac))
    {
        saiRetVal = xpSaiConfigIpCtrlTrap(*rif_id, false);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to set (xpSaiConfigIpCtrlTrap)\n");
            return saiRetVal;
        }
    }

    retVal = xpSaiRouterInterfaceCommonDrop(*rif_id, true);
    if (XP_NO_ERR != retVal)
    {
        XP_SAI_LOG_ERR("Failed xpSaiRouterInterfaceCommonDrop , error: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    pRifEntry->rifCreated = true;

    return SAI_STATUS_SUCCESS;
}

bool xpSaiPortBasedRifEntryExists(sai_object_id_t oid)
{
    if (!XDK_SAI_OBJID_TYPE_CHECK(oid, SAI_OBJECT_TYPE_PORT) &&
        !XDK_SAI_OBJID_TYPE_CHECK(oid, SAI_OBJECT_TYPE_LAG))
    {
        return false;
    }

    return (xpSaiFindRifEntry(oid, SAI_NULL_OBJECT_ID) == NULL) ? false : true;
}

//Func: xpSaiCreateRouterInterface

sai_status_t xpSaiCreateRouterInterface(sai_object_id_t *rif_id,
                                        sai_object_id_t switch_id,
                                        uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t            saiRetVal   = SAI_STATUS_SUCCESS;
    const sai_attribute_t   *macAttr    = NULL;

    xpSaiRouterInterfaceAttributesT attributes;

    if (rif_id == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               ROUTER_INTERFACE_VALIDATION_ARRAY_SIZE,
                               router_interface_attribs, SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    memset(&attributes, 0, sizeof(attributes));

    xpSaiSetDefaultRouterInterfaceAttributeVals(attr_count, attr_list, &attributes);

    saiRetVal = xpSaiUpdateRouterInterfaceAttributeVals(attr_count, attr_list,
                                                        &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("failed in xpSaiUpdateRouterInterfaceAttributeVals()");
        return saiRetVal;
    }

    if (attributes.isVirtual.booldata == true)
    {
        // The only purpose to create virtual RIF is to assign additional MAC
        // address on RIF. So, clean-up default MAC address value in case
        // MAC address has not been provided in virtual RIF create request.
        // That means that no HW entry will be created.
        macAttr = xpSaiFindAttrById(SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS,
                                    attr_count, attr_list, 0);
        if (!macAttr)
        {
            memset(&attributes.srcMacAddress, 0x0, 6);
        }

        saiRetVal = xpSaiCreateVirtRif(rif_id, switch_id, &attributes);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to create virtual RIF. Error %d\n", saiRetVal);
            return saiRetVal;
        }
    }
    else
    {
        saiRetVal = xpSaiCreatePhysRif(rif_id, switch_id, &attributes);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to create physical RIF. Error %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

XP_STATUS xpSaiGetRifStatisticInfo(sai_int32_t l3IntfId,
                                   xpSaiRouterInterfaceStatisticDbEntryT **ppRifStatisticInfo)
{
    xpSaiRouterInterfaceStatisticDbEntryT *rifStatisticKey;
    XP_STATUS xpsRetVal = XP_NO_ERR;

    xpsRetVal = xpsStateHeapMalloc(sizeof(xpSaiRouterInterfaceStatisticDbEntryT),
                                   (void**)&rifStatisticKey);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate rif stats structure\n");
        return xpsRetVal;
    }

    memset(rifStatisticKey, 0, sizeof(xpSaiRouterInterfaceStatisticDbEntryT));

    rifStatisticKey->keyRifId = l3IntfId;
    /* Retrieve the corresponding state from sai port statistic db */
    if ((xpsRetVal = xpsStateSearchData(XP_SCOPE_DEFAULT,
                                        gXpSaiRifStatisticDbHandle, (xpsDbKey_t)rifStatisticKey,
                                        (void**)ppRifStatisticInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai rif statistic data |retVal:%d\n",
                       xpsRetVal);
        xpsStateHeapFree((void*)rifStatisticKey);
        return xpsRetVal;
    }

    if (*ppRifStatisticInfo == NULL)
    {
        XP_SAI_LOG_ERR("SAI Rif statistic DB entry is not created for this intf %d\n",
                       l3IntfId);
        xpsStateHeapFree((void*)rifStatisticKey);
        return XP_ERR_NOT_INITED;
    }

    xpsStateHeapFree((void*)rifStatisticKey);
    return XP_NO_ERR;
}

static sai_status_t xpSaiRemoveVirtRouterInterfaces(xpSaiRouterInterfaceDbEntryT
                                                    *pRifEntry)
{
    sai_status_t       saiRetVal = SAI_STATUS_SUCCESS;
    uint16_t           virtId    = 0;
    sai_object_id_t    virtRifId = 0;
    xpsDevice_t        xpsDevId  = xpSaiGetDevId();

    for (uint16_t i = 0; (i < XP_SAI_INTERFACE_VIRTUAL_RIF_MAX) &&
         (pRifEntry->rifCount > 0); i++)
    {
        if (pRifEntry->rifInfo[i].id == 0)
        {
            continue;
        }

        virtId = pRifEntry->rifInfo[i].id;

        saiRetVal = xpSaiRouterInterfaceObjIdCreate(xpsDevId, pRifEntry->keyRifId,
                                                    virtId, &virtRifId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI object could not be created, saiRetVal %d", saiRetVal);
            return saiRetVal;
        }

        saiRetVal = xpSaiRemoveRouterInterfaceMacAddress(virtRifId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiRemoveRouterInterfaceMacAddress() failed with error code: %d\n",
                           saiRetVal);
            return saiRetVal;
        }

        pRifEntry->rifCount--;
        memset(&pRifEntry->rifInfo[i], 0, sizeof(xpSaiInterfaceVirtualRifInfoT));
    }

    return saiRetVal;
}

//Func: xpSaiRemoveRouterInterface

sai_status_t xpSaiRemoveRouterInterface(sai_object_id_t rif_oid)
{
    XP_STATUS          retVal    = XP_NO_ERR;
    sai_status_t       saiRetVal = SAI_STATUS_SUCCESS;
    xpsInterfaceId_t   l3IntfId  = 0;
    sai_object_id_t    portOid   = 0;
    xpsInterfaceId_t   portId    = 0;
    sai_object_id_t    vlanOid   = 0;
    uint16_t           virtId    = 0;
    xpsInterfaceType_e intfType  = XPS_PORT;
    xpsDevice_t        xpsDevId  = xpSaiObjIdSwitchGet(rif_oid);

    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    // Verify if RIF is loopback interface
    if (xpSaiRouterInterfaceLoopbackIdCheck(rif_oid))
    {
        saiRetVal = xpSaiRouterInterfaceLoopbackIdRelease(rif_oid);
        if (saiRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to release router interface loopback id, error %d\n",
                           saiRetVal);
            return saiRetVal;
        }

        return SAI_STATUS_SUCCESS;
    }

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_oid, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    l3IntfId = pRifEntry->l3IntfId;

    saiRetVal = xpSaiRouterInterfaceVirtIdGet(rif_oid, &virtId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceVirtIdGet() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    if (pRifEntry->rifType != SAI_ROUTER_INTERFACE_TYPE_BRIDGE)
    {
        saiRetVal = xpSaiRemoveRouterInterfaceMacAddress(rif_oid);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiRemoveRouterInterfaceMacAddress() failed with error code: %d\n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    /* Remove all virtual router interfaces during removing physical or last virtual */
    if (virtId == 0)
    {
        saiRetVal = xpSaiRemoveVirtRouterInterfaces(pRifEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiRemoveVirtRouterInterfaces() failed with error code: %d\n",
                           saiRetVal);
            return saiRetVal;
        }
    }
    else
    {
        saiRetVal = xpSaiFreeVirtRifId(pRifEntry, virtId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiFreeVirtRifId() failed with error code: %d\n", saiRetVal);
            return saiRetVal;
        }

        if ((pRifEntry->rifCount == 0) && !pRifEntry->rifCreated)
        {
            saiRetVal = xpSaiFreeRifDbEntry(rif_oid, pRifEntry);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiFreeRifDbEntry() failed with error code: %d!\n",
                               saiRetVal);
                return saiRetVal;
            }
        }

        return SAI_STATUS_SUCCESS;
    }

    /* Revert the SW and cpss states of the port. */
    if (pRifEntry->adminV4State == true)
    {
        retVal = xpsL3SetIntfIpv4UcRoutingEn(xpsDevId, l3IntfId, 0);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsL3SetIntfIpv4UcRoutingEn() for l3IntfId %u failed with error code: %d\n",
                           l3IntfId, retVal);
            return retVal;
        }
    }

    if (pRifEntry->adminV6State == true)
    {
        retVal = xpsL3SetIntfIpv6UcRoutingEn(xpsDevId, l3IntfId, 0);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsL3SetIntfIpv6UcRoutingEn() for l3IntfId %u failed with error code: %d\n",
                           l3IntfId, retVal);
            return retVal;
        }
    }

    retVal = xpSaiRouterInterfaceCommonDrop(rif_oid, false);
    if (XP_NO_ERR != retVal)
    {
        XP_SAI_LOG_ERR("Failed xpSaiRouterInterfaceCommonDrop , error: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    switch (pRifEntry->rifType)
    {
        case SAI_ROUTER_INTERFACE_TYPE_PORT:
            {
                retVal = xpSaiVrfIntfPortGet(l3IntfId, &portOid);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiVrfIntfPortGet() for l3IntfId %u failed with error code: %d\n",
                                   l3IntfId, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                saiRetVal = xpSaiConvertPortOid(portOid, &portId);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiConvertPortOid() failed with error code: %d\n", saiRetVal);
                    return saiRetVal;
                }

                retVal = xpsL3UnBindPortIntf(portId);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsL3UnBindPortIntf() failed with error code: %d\n", retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                retVal = xpsL3DeInitPortIntf(xpsDevId, l3IntfId);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsL3DeInitPortIntf() for l3IntfId %d failed with error code: %d\n",
                                   l3IntfId, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                retVal = xpsInterfaceGetType(portId, &intfType);
                if (XP_NO_ERR != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get type for interface %u, error: %d\n", portId,
                                   retVal);
                    return retVal;
                }

                if (XPS_LAG == intfType)
                {
                    xpSaiLagInfo_t *lagConfig = NULL;
                    saiRetVal = xpSaiLagUnref(portId);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Unable to remove LAG reference, sai_error: %d\n", saiRetVal);
                        return XP_ERR_INVALID_ID;
                    }

                    saiRetVal = xpSaiLagConfigInfoGet(portId, &lagConfig);
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("Failed to get config for LAG %u, error %u\n", portId,
                                       saiRetVal);
                        return XP_PORT_NOT_VALID;
                    }
                    lagConfig->lagConfig.portVlanId = lagConfig->pvidUserSetting;

                    saiRetVal = xpSaiLagSetSaMissCmd(portId,
                                                     (xpsPktCmd_e)pRifEntry->prevPortPktCmd);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to Set SMAC miss for LAG %u, error: %d\n", portId,
                                       saiRetVal);
                        return saiRetVal;
                    }

                }
                else
                {
                    // Return previous value of macSAmissCmd
                    retVal = xpsPortSetField(xpsDevId, portId, XPS_PORT_MAC_SAMISSCMD,
                                             pRifEntry->prevPortPktCmd);
                    if (XP_NO_ERR != retVal)
                    {
                        XP_SAI_LOG_ERR("Failed to set SA miss command for Port %u, error: %d\n", portId,
                                       retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }
                    // set pvid for port
                    xpSaiPortDbEntryT *pPortEntry = NULL;
                    saiRetVal = xpSaiPortDbInfoGet(portId, &pPortEntry);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not retrieve port info from DB\n");
                        return saiRetVal;
                    }
                    retVal = xpsPortSetField(xpsDevId, portId, XPS_PORT_PVID,
                                             pPortEntry->pvidUserSetting);
                    if (XP_NO_ERR != retVal)
                    {
                        XP_SAI_LOG_ERR("Failed to set PORT_PVID for Port %u, error: %d\n", portId,
                                       retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_TYPE_SUB_PORT:
            {
                retVal = xpSaiVrfIntfPortGet(l3IntfId, &portOid);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiVrfIntfPortGet() for l3IntfId %u failed with error code: %d\n",
                                   l3IntfId, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                saiRetVal = xpSaiConvertPortOid(portOid, &portId);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiConvertPortOid() failed with error code: %d\n", saiRetVal);
                    return saiRetVal;
                }

                // Return previous value of macSAmissCmd
                retVal = xpsPortSetField(xpsDevId, portId, XPS_PORT_MAC_SAMISSCMD,
                                         pRifEntry->prevPortPktCmd);
                if (XP_NO_ERR != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set SA miss command for Port %u, error: %d\n", portId,
                                   retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                retVal = xpSaiVrfIntfVlanIdGet(l3IntfId, &vlanOid);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiVrfIntfVlanIdGet() for l3IntfId %u failed with error code: %d\n",
                                   l3IntfId, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                retVal = xpsL3UnBindSubIntf(portId, l3IntfId,
                                            (xpsVlan_t)xpSaiObjIdValueGet(vlanOid));
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsL3UnBindSubIntf() failed with error code: %d\n", retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_TYPE_BRIDGE:
        case SAI_ROUTER_INTERFACE_TYPE_VLAN:
            {
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unsupported RIF type %d\n", pRifEntry->rifType);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    saiRetVal = xpSaiFreeRifDbEntry(rif_oid, pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiFreeRifDbEntry() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetRouterInterfaceStats(sai_object_id_t rif_oid,
                                          uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                          uint64_t *counters)
{
    XP_STATUS     xpStatus  = XP_NO_ERR;
    sai_status_t  saiStatus = SAI_STATUS_SUCCESS;
    xpsDevice_t   xpsDevId  = xpSaiGetDevId();
    xpSaiRouterInterfaceStatisticDbEntryT *rifStatistics = NULL;
    xpsInterfaceId_t l3IntfId = 0;
    xpsL3StatsInfo_t xpsRifStatistics;
    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    memset(&xpsRifStatistics, 0, sizeof(xpsL3StatsInfo_t));

    if ((number_of_counters < 1) || (NULL == counter_ids) ||
        (NULL == counters)  /*|| (port validation check should be added)*/)
    {
        XP_SAI_LOG_ERR("%s invalid parameters received.", __FUNCNAME__);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(rif_oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(rif_oid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiStatus = xpSaiRouterInterfaceDbInfoGet(rif_oid, &pRifEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiStatus);
        return saiStatus;
    }

    l3IntfId = pRifEntry->l3IntfId;

    xpStatus = xpSaiGetRifStatisticInfo(l3IntfId, &rifStatistics);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get the rif statistic data, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsL3IntfStatsRead(xpsDevId, l3IntfId, &xpsRifStatistics);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get xps rif statistic data, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    rifStatistics->rxFrames += xpsRifStatistics.rxPkts;
    rifStatistics->rxOctets += xpsRifStatistics.rxOctets;
    rifStatistics->rxErrorFrames += xpsRifStatistics.rxErrPkts;
    rifStatistics->rxErrorOctets += xpsRifStatistics.rxErrOctets;
    rifStatistics->txFrames += xpsRifStatistics.txPkts;
    rifStatistics->txOctets += xpsRifStatistics.txOctets;
    rifStatistics->txErrorFrames += xpsRifStatistics.txErrPkts;
    rifStatistics->txErrorOctets += xpsRifStatistics.txErrOctets;

    for (uint32_t indx=0; indx < number_of_counters; indx++)
    {
        switch (counter_ids[indx])
        {
            case SAI_ROUTER_INTERFACE_STAT_IN_PACKETS :
                {
                    counters[indx] = rifStatistics->rxFrames;
                    break;
                }
            case SAI_ROUTER_INTERFACE_STAT_IN_OCTETS :
                {
                    counters[indx] = rifStatistics->rxOctets;
                    break;
                }

            case SAI_ROUTER_INTERFACE_STAT_IN_ERROR_PACKETS :
                {
                    counters[indx] = rifStatistics->rxErrorFrames;
                    break;
                }
            case SAI_ROUTER_INTERFACE_STAT_IN_ERROR_OCTETS :
                {
                    counters[indx] = rifStatistics->rxErrorOctets;
                    break;
                }
            case SAI_ROUTER_INTERFACE_STAT_OUT_PACKETS :
                {
                    counters[indx] = rifStatistics->txFrames;
                    break;
                }
            case SAI_ROUTER_INTERFACE_STAT_OUT_OCTETS :
                {
                    counters[indx] = rifStatistics->txOctets;
                    break;
                }
            case SAI_ROUTER_INTERFACE_STAT_OUT_ERROR_PACKETS :
                {
                    counters[indx] = rifStatistics->txErrorFrames;
                    break;
                }
            case SAI_ROUTER_INTERFACE_STAT_OUT_ERROR_OCTETS :
                {
                    counters[indx] = rifStatistics->txErrorOctets;
                    break;
                }
            default :
                {
                    XP_SAI_LOG_ERR("Undefined attribute %d\n", counter_ids[indx]);
                    saiStatus = SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(indx);
                }
        }
    }
    return saiStatus;
}

//Func: xpSaiRifNotifyAddAcl

sai_status_t xpSaiRifNotifyAddAcl(xpsDevice_t devId,
                                  sai_object_id_t rifObjId, sai_object_id_t aclObjId)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    saiStatus = xpSaiAclMapperBindToTableGroup(devId, aclObjId, rifObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Failed to bind vlan to table group, status:%d\n", saiStatus);
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetRouterInterfaceAttrAcl

sai_status_t xpSaiSetRouterInterfaceAttrAcl(sai_object_id_t rif_oid,
                                            sai_attribute_value_t value, bool isIngress)
{
    sai_status_t             saiStatus              = SAI_STATUS_SUCCESS;
    xpsDevice_t              devId                  = 0;
    sai_uint32_t             tableId                = 0;
    sai_uint32_t             groupId                = 0;
    sai_object_id_t prevOid = SAI_NULL_OBJECT_ID;

    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    devId  = (xpsDevice_t)xpSaiObjIdSwitchGet(rif_oid);

    XP_SAI_LOG_DBG("Calling xpSaiSetRouterInterfaceAttrAcl\n");

    saiStatus = xpSaiRouterInterfaceDbInfoGet(rif_oid, &pRifEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiStatus);
        return saiStatus;
    }

    if ((pRifEntry->l3IntfId == XPS_INTF_INVALID_ID) &&
        (pRifEntry->rifType == SAI_ROUTER_INTERFACE_TYPE_BRIDGE))
    {
        /* No L3 interface created yet.
         * Only fill corresponding RIF entry field */
        pRifEntry->ingressAclId = xpSaiObjIdValueGet(value.oid);

        return SAI_STATUS_SUCCESS;
    }

    if (isIngress)
    {
        if (pRifEntry->ingressAclId == value.oid)
        {
            return SAI_STATUS_SUCCESS;
        }
        prevOid = pRifEntry->ingressAclId;
    }
    else
    {
        if (pRifEntry->egressAclId == value.oid)
        {
            return SAI_STATUS_SUCCESS;
        }
        prevOid = pRifEntry->egressAclId;
    }

    if (prevOid != SAI_NULL_OBJECT_ID)
    {
        if (XDK_SAI_OBJID_TYPE_CHECK(prevOid, SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
        {
            saiStatus = xpSaiAclMapperUnBindFromTableGroup(devId, prevOid,
                                                           rif_oid);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to notify add ingress acl, port:%d, status:%d\n",
                               rif_oid, saiStatus);
                return saiStatus;
            }
        }
        else if (XDK_SAI_OBJID_TYPE_CHECK(prevOid, SAI_OBJECT_TYPE_ACL_TABLE))
        {
            tableId = (uint32_t)xpSaiObjIdValueGet(prevOid);
            saiStatus = xpSaiAclRouterAclIdSet(pRifEntry->l3IntfId, groupId, tableId,
                                               false);
            if (saiStatus != XP_NO_ERR)
            {
                return (saiStatus);
            }
        }
    }

    if (value.oid != SAI_NULL_OBJECT_ID)
    {
        if (!XDK_SAI_OBJID_TYPE_CHECK(value.oid, SAI_OBJECT_TYPE_ACL_TABLE) &&
            !XDK_SAI_OBJID_TYPE_CHECK(value.oid, SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
        {
            XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                           xpSaiObjIdTypeGet(value.oid));
            return SAI_STATUS_INVALID_PARAMETER;
        }

        if (XDK_SAI_OBJID_TYPE_CHECK(value.oid, SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
        {
            // Send add ingress acl notification to acl module to update it's table group bind list
            saiStatus = xpSaiRifNotifyAddAcl(devId, rif_oid, value.oid);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_DBG("Failed to notify add ingress acl, rif:%d, status:%d\n", rif_oid,
                               saiStatus);
                return saiStatus;
            }
        }
        else
        {
            // Get the acl table id from table object
            tableId = (uint32_t)xpSaiObjIdValueGet(value.oid);
            saiStatus = xpSaiAclRouterAclIdSet(pRifEntry->l3IntfId, groupId, tableId, true);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiAclRouterAclIdSet failed with error %d \n", saiStatus);
                return saiStatus;
            }
        }
    }
    else
    {
        saiStatus = xpSaiAclRouterAclIdSet(pRifEntry->l3IntfId, groupId, 0, false);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiAclRouterAclIdSet failed with error %d \n", saiStatus);
            return saiStatus;
        }
    }

    // Update state database
    if (isIngress)
    {
        pRifEntry->ingressAclId = value.oid;
    }
    else
    {
        pRifEntry->egressAclId = value.oid;
    }

    return saiStatus;
}


//Func: xpSaiGetRouterInterfaceAttrAcl

sai_status_t xpSaiGetRouterInterfaceAttrAcl(sai_object_id_t rif_oid,
                                            sai_attribute_value_t* value, bool isIngress)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiGetRouterInterfaceAttrAcl\n");

    saiStatus = xpSaiRouterInterfaceDbInfoGet(rif_oid, &pRifEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiStatus);
        return saiStatus;
    }

    if (isIngress)
    {
        value->oid = pRifEntry->ingressAclId;
    }
    else
    {
        value->oid = pRifEntry->egressAclId;
    }

    return saiStatus;
}

//Func: xpSaiSetRouterInterfaceAttrNeighMissAction

static sai_status_t xpSaiSetRouterInterfaceAttrNeighMissAction(
    sai_object_id_t rif_id, sai_attribute_value_t value)
{
    sai_status_t                 saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    pRifEntry->neighMissAction = value.u32;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetRouterInterfaceAttrNeighMissAction

static sai_status_t xpSaiGetRouterInterfaceAttrNeighMissAction(
    sai_object_id_t rif_id, sai_attribute_value_t *value)
{
    sai_status_t                 saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    value->u32 = pRifEntry->neighMissAction;

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiSetRouterInterfaceAttribute

static sai_status_t xpSaiSetRouterInterfaceAttribute(sai_object_id_t rif_id,
                                                     const sai_attribute_t *attr)
{
    sai_status_t                 saiRetVal    = SAI_STATUS_SUCCESS;
    XP_STATUS                    status       = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t*       pVrfIntfInfo = NULL;
    sai_attribute_value_t curMac;

    xpSaiRouterInterfaceDbEntryT *pRifEntry   = NULL;

    saiRetVal = xpSaiAttrCheck(1, attr,
                               ROUTER_INTERFACE_VALIDATION_ARRAY_SIZE, router_interface_attribs,
                               SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    if (xpSaiRouterInterfaceLoopbackIdCheck(rif_id))
    {
        XP_SAI_LOG_ERR("Could not set attribute %d on SAI_ROUTER_INTERFACE_TYPE_LOOPBACK\n",
                       attr->id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    if ((pRifEntry->l3IntfId != XPS_INTF_INVALID_ID) &&
        (pRifEntry->rifType != SAI_ROUTER_INTERFACE_TYPE_BRIDGE))
    {
        status = xpSaiVrfIntfInfoGet(pRifEntry->l3IntfId, &pVrfIntfInfo);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("VrfIntf is not present %d\n", pRifEntry->l3IntfId);
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }

    switch (attr->id)
    {
        case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
            {
                /* Check whether MAC is valid */
                saiRetVal = xpSaiIsValidUcastMac((uint8_t *)attr->value.mac);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS)\n");
                    return saiRetVal;
                }


                saiRetVal = xpSaiGetRouterInterfaceAttrSrcMacAddress(rif_id, &curMac);
                if ((saiRetVal == SAI_STATUS_ITEM_NOT_FOUND))
                {
                    XP_SAI_LOG_NOTICE("Couldn't find vRIF in SAI Router Interface DB. Probably already removed as part of Native RIF\n");
                    return SAI_STATUS_SUCCESS;
                }

                /* Delete the Old MAC and Update the new MAC*/
                if (!xpSaiEthAddrIsZero(curMac.mac))
                {
                    saiRetVal = xpSaiConfigIpCtrlTrap(rif_id, true);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to set (xpSaiConfigIpCtrlTrap)\n");
                        return saiRetVal;
                    }
                }

                saiRetVal = xpSaiSetRouterInterfaceAttrSrcMacAddress(rif_id, attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS)\n");
                    return saiRetVal;
                }

                saiRetVal = xpSaiConfigIpCtrlTrap(rif_id, false);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (xpSaiConfigIpCtrlTrap)\n");
                    return saiRetVal;
                }

                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
            {
                saiRetVal = xpSaiSetRouterInterfaceAttrAdminV4State(rif_id, attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE:
            {
                saiRetVal = xpSaiSetRouterInterfaceAttrAdminV6State(rif_id, attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_V4_MCAST_ENABLE:
            {
                saiRetVal = xpSaiSetRouterInterfaceAttrMcAdminV4State(rif_id, attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE:
            {
                saiRetVal = xpSaiSetRouterInterfaceAttrMcAdminV6State(rif_id, attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_MTU:
            {
                return xpSaiSetRouterInterfaceAttrMtu(rif_id, attr->value);
            }
        case SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION:
            {
                saiRetVal = xpSaiSetRouterInterfaceAttrNeighMissAction(rif_id, attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_INGRESS_ACL:
            {
                saiRetVal = xpSaiSetRouterInterfaceAttrAcl(rif_id, attr->value, true);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_INGRESS_ACL)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_EGRESS_ACL:
            {
                saiRetVal = xpSaiSetRouterInterfaceAttrAcl(rif_id, attr->value, false);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTER_INTERFACE_ATTR_EGRESS_ACL)\n");
                    return saiRetVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attribute %d\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetRouterInterfaceAttribute

static sai_status_t xpSaiGetRouterInterfaceAttribute(sai_object_id_t rif_id,
                                                     sai_attribute_t *attr, uint32_t attr_index)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if (xpSaiRouterInterfaceLoopbackIdCheck(rif_id))
    {
        XP_SAI_LOG_ERR("Could not get attribute %d on SAI_ROUTER_INTERFACE_TYPE_LOOPBACK\n",
                       attr->id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (attr->id)
    {
        case SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID:
            {
                retVal = xpSaiGetRouterInterfaceAttrVirtualRouterId(rif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_PORT_ID:
            {
                retVal = xpSaiGetRouterInterfaceAttrPortId(rif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_ROUTER_INTERFACE_ATTR_PORT_ID)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_VLAN_ID:
            {
                retVal = xpSaiGetRouterInterfaceAttrVlanId(rif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_ROUTER_INTERFACE_ATTR_VLAN_ID)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_TYPE:
            {
                retVal = xpSaiGetRouterInterfaceAttrRifType(rif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_ROUTER_INTERFACE_ATTR_TYPE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
            {
                retVal = xpSaiGetRouterInterfaceAttrSrcMacAddress(rif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
            {
                retVal = xpSaiGetRouterInterfaceAttrAdminV4State(rif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE:
            {
                retVal = xpSaiGetRouterInterfaceAttrAdminV6State(rif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_V4_MCAST_ENABLE:
            {
                retVal = xpSaiGetRouterInterfaceAttrMcAdminV4State(rif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_ROUTER_INTERFACE_ATTR_V4_MCAST_ENABLE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE:
            {
                retVal = xpSaiGetRouterInterfaceAttrMcAdminV6State(rif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_MTU:
            {
                return xpSaiGetRouterInterfaceAttrMtu(rif_id, &attr->value);
            }
        case SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION:
            {
                retVal = xpSaiGetRouterInterfaceAttrNeighMissAction(rif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_INGRESS_ACL:
            {
                retVal = xpSaiGetRouterInterfaceAttrAcl(rif_id, &attr->value, true);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_ROUTER_INTERFACE_ATTR_INGRESS_ACL)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTER_INTERFACE_ATTR_EGRESS_ACL:
            {
                retVal = xpSaiGetRouterInterfaceAttrAcl(rif_id, &attr->value, false);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_ROUTER_INTERFACE_ATTR_EGRESS_ACL)\n");
                    return retVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }
    return retVal;
}


//Func: xpSaiGetRouterInterfaceAttributes

static sai_status_t xpSaiGetRouterInterfaceAttributes(sai_object_id_t rif_id,
                                                      uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            ROUTER_INTERFACE_VALIDATION_ARRAY_SIZE, router_interface_attribs,
                            SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        retVal = xpSaiGetRouterInterfaceAttribute(rif_id, &attr_list[count], count);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceAttribute failed\n");
            if (SAI_STATUS_ITEM_NOT_FOUND  == retVal)
            {
                return SAI_STATUS_INVALID_OBJECT_ID;
            }
            return retVal;
        }
    }

    return retVal;
}

//Func: xpSaiBulkGetRouterInterfaceAttributes

sai_status_t xpSaiBulkGetRouterInterfaceAttributes(sai_object_id_t id,
                                                   uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_ROUTER_INTERFACE))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountRouterInterfaceAttribute(&maxcount);
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
        attr_list[idx].id = SAI_ROUTER_INTERFACE_ATTR_START + count;
        saiRetVal = xpSaiGetRouterInterfaceAttribute(id, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

XP_STATUS xpSaiRouterInterfaceInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    xpsRetVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT,
                                            XP_SAI_ALLOC_LOOPBACK_RIF, XP_SAI_LOOPBACK_RIF_MAX_IDS,
                                            XP_SAI_LOOPBACK_RIF_RANGE_START);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI Loopback RIF ID allocator\n");
        return xpsRetVal;
    }

    xpsRetVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_RIF,
                                            XP_SAI_RIF_MAX_IDS, XP_SAI_RIF_RANGE_START);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI Loopback RIF ID allocator\n");
        return xpsRetVal;
    }

    // Create a Global Sai RIF Db
    if ((xpsRetVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Router Interface Db",
                                        XPS_GLOBAL,
                                        &xpSaiRouterInterfaceKeyComp, gXpSaiRouterInterfaceDbHandle))!= XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to Register Sai RouterInterface Db\n");
        return xpsRetVal;
    }
    // Create a Global Sai RIF stats Db
    if ((xpsRetVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT,
                                        "Sai Router Interface Stats Db",
                                        XPS_GLOBAL,
                                        &xpSaiRifStatisticKeyComp, gXpSaiRifStatisticDbHandle))!= XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to Register Sai RouterInterface Stats Db\n");
        return xpsRetVal;
    }
    return XP_NO_ERR;
}

XP_STATUS xpSaiRouterInterfaceDeInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS   xpsRetVal = XP_NO_ERR;

    if ((xpsRetVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                          &gXpSaiRifStatisticDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("De-Register Sai RouterInterface handler failed!\n");
        return xpsRetVal;
    }

    if ((xpsRetVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                          &gXpSaiRouterInterfaceDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("De-Register Sai RouterInterface handler failed!\n");
        return xpsRetVal;
    }
    return XP_NO_ERR;
}
//Func: xpSaiRouterInterfaceApiInit

XP_STATUS xpSaiRouterInterfaceApiInit(uint64_t flag,
                                      const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    _xpSaiRouterInterfaceApi = (sai_router_interface_api_t *) xpMalloc(sizeof(
                                                                           sai_router_interface_api_t));
    if (NULL == _xpSaiRouterInterfaceApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiRouterInterfaceApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiRouterInterfaceApi->create_router_interface = xpSaiCreateRouterInterface;
    _xpSaiRouterInterfaceApi->remove_router_interface = xpSaiRemoveRouterInterface;
    _xpSaiRouterInterfaceApi->set_router_interface_attribute =
        xpSaiSetRouterInterfaceAttribute;
    _xpSaiRouterInterfaceApi->get_router_interface_attribute =
        xpSaiGetRouterInterfaceAttributes;

    _xpSaiRouterInterfaceApi->get_router_interface_stats =
        xpSaiGetRouterInterfaceStats;
    _xpSaiRouterInterfaceApi->clear_router_interface_stats =
        (sai_clear_router_interface_stats_fn)xpSaiStubGenericApi;
    _xpSaiRouterInterfaceApi->get_router_interface_stats_ext =
        (sai_get_router_interface_stats_ext_fn)xpSaiStubGenericApi;

    saiRetVal = xpSaiApiRegister(SAI_API_ROUTER_INTERFACE,
                                 (void*)_xpSaiRouterInterfaceApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register router interface API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}



//Func: xpSaiRouterInterfaceApiDeinit

XP_STATUS xpSaiRouterInterfaceApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    xpFree(_xpSaiRouterInterfaceApi);
    _xpSaiRouterInterfaceApi = NULL;

    return retVal;
}

sai_status_t xpSaiMaxCountRouterInterfaceAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_ROUTER_INTERFACE_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountRouterInterfaceObjects(uint32_t *count)
{
    return xpSaiCountVrfIntfObjects(count);
}

sai_status_t xpSaiGetRouterInterfaceObjectList(uint32_t *object_count,
                                               sai_object_key_t *object_list)
{
    return xpSaiGetVrfIntfObjectList(object_count, object_list);
}

sai_status_t xpSaiConfigIpCtrlTrap(sai_object_id_t rif_id, bool isDelete)
{
    XP_STATUS           retVal      = XP_NO_ERR;
    sai_status_t        saiRetVal   = SAI_STATUS_SUCCESS;
    xpsPktCmd_e         xpsPktCmd   = XP_PKTCMD_FWD;
    xpsCoppCtrlPkt_e xpsTrap = XPS_COPP_CTRL_PKT_MAX;
    sai_packet_action_t pktCmd;
    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;
    uint32_t i;
    sai_object_id_t trapOid;
    xpSaiHostInterfaceTrapDbEntryT  *pTrapEntry     = NULL;
    xpSaiHostInterfaceTrapDefaultT  *pDefTrapEntry  = NULL;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;
    /* MLD, NDP uses same CPSS API.
       ARP_REQ, OSPF,PIM,VRRP uses same API.
       All IGMP types uses same API in CPSS per eVlan.
       Hence no need to explicitly call for additional types.
    */
    sai_hostif_trap_type_t trapEnt[] = {SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST,
                                        SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE,
                                        SAI_HOSTIF_TRAP_TYPE_IPV6_NEIGHBOR_DISCOVERY,
                                        SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_QUERY
                                       };

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS || pRifEntry == NULL)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    /* Get Trap packet action and configure in Datapath.*/

    for (i= 0; i < (sizeof(trapEnt)/sizeof(sai_hostif_trap_type_t)); i++)
    {
        if (isDelete == false)
        {
            /* Create Trap object ID */
            saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_HOSTIF_TRAP, xpSaiGetDevId(),
                                         (sai_uint64_t)trapEnt[i], &trapOid);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("SAI object could not be created, retVal %d\n", saiRetVal);
                return saiRetVal;
            }

            saiRetVal = xpSaiHostIntfTrapDbInfoGet(trapOid, &pTrapEntry);
            if ((saiRetVal == SAI_STATUS_ITEM_NOT_FOUND) && (pTrapEntry == NULL))
            {
                XP_SAI_LOG_NOTICE("Could not find the trap 0x%08X in DB\n",
                                  (uint32_t)xpSaiObjIdValueGet(trapOid));
                pDefTrapEntry = xpSaiHostIntfDefaultTrapGet(trapEnt[i]);
                if (pDefTrapEntry == NULL)
                {
                    XP_SAI_LOG_ERR("Invalid Host Interface Trap %d\n", trapEnt[i]);
                    return SAI_STATUS_INVALID_PARAMETER;
                }

            }
            if (pTrapEntry)
            {
                pktCmd = pTrapEntry->action;
            }
            else if (pDefTrapEntry)
            {
                pktCmd = pDefTrapEntry->action;
            }
            else
            {
                XP_SAI_LOG_ERR("Invalid Host Interface Trap Entry \n");
                return SAI_STATUS_INVALID_PARAMETER;
            }

            saiRetVal = xpSaiConvertSaiPacketAction2xps(pktCmd, &xpsPktCmd);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to convert trap action, error %d\n", saiRetVal);
                return saiRetVal;
            }
        }

        saiRetVal = xpSaiHostInterfaceGetXpsCoppCtrlPktType(trapEnt[i], &xpsTrap);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get xpsTrap for saiTrap %d error %d\n", trapEnt[i],
                           saiRetVal);
            return saiRetVal;
        }

        /* pktCmd DROP may not be valid action. May need to return. */

        retVal = xpsUpdateIpCtrlTrapOnL3Entry(xpSaiGetDevId(), xpsTrap, xpsPktCmd,
                                              pRifEntry->l3IntfId, NULL, isDelete);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsUpdateIpCtrlTrapOnL3Entry Failed : %d", retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    /* Each RIF consumes a FDB mac entry with myMac + eVlan.
       This is used in trapping  ARP response. Increment the global FDB count.*/
    saiRetVal = xpSaiGetTableEntryCountCtxDb(xpSaiGetDevId(), &entryCountCtxPtr);
    if ((saiRetVal != SAI_STATUS_SUCCESS) || (entryCountCtxPtr == NULL))
    {
        XP_SAI_LOG_ERR("%s: Failed to get Table Entry Count DB , retVal : %d \n",
                       __FUNCNAME__, saiRetVal);
        return saiRetVal;
    }
    if (isDelete)
    {
        entryCountCtxPtr->fdbEntries--;
    }
    else
    {
        entryCountCtxPtr->fdbEntries++;
    }

    return saiRetVal;
}

static XP_STATUS xpSaiRouterInterfaceCommonDrop(sai_object_id_t rif_id,
                                                bool enable)
{
    XP_STATUS         retVal      = XP_NO_ERR;
    sai_status_t      saiRetVal   = SAI_STATUS_SUCCESS;
    xpsDevice_t       xpsDevId    = xpSaiObjIdSwitchGet(rif_id);
    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return XP_ERR_FAILED;
    }

    retVal = xpsAclRouterCommonDropUpdate(xpsDevId, pRifEntry->l3IntfId, enable);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    return XP_NO_ERR;
}

// xpSaiRouterInterface.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiRouterInterface_h_
#define _xpSaiRouterInterface_h_

#include "xpSai.h"

#define XP_SAI_DEFAULT_MTU 1514
#define XP_SAI_INTERFACE_VIRTUAL_RIF_MAX 512
#define XPSAI_ROUTER_INTERFACE_LOOPBACK_RIF_MASK (0x10000000)  ///< Mask to identify loopback rif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _xpSaiRouterInterfaceAttributesT
{
    sai_attribute_value_t virtualRouterId;
    sai_attribute_value_t portType;
    sai_attribute_value_t portId;
    sai_attribute_value_t vlanId;
    sai_attribute_value_t srcMacAddress;
    sai_attribute_value_t adminV4State;
    sai_attribute_value_t adminV6State;
    sai_attribute_value_t mtu;
    sai_attribute_value_t ingressAcl;
    sai_attribute_value_t egressAcl;
    sai_attribute_value_t neighMissAction;
    sai_attribute_value_t isVirtual;
    sai_attribute_value_t mcAdminV4State;
    sai_attribute_value_t mcAdminV6State;
} xpSaiRouterInterfaceAttributesT;

typedef struct _xpSaiInterfaceVirtualRifInfoT
{
    sai_mac_t   mac;
    uint16_t    id;
} xpSaiInterfaceVirtualRifInfoT;

typedef struct _xpSaiRouterInterfaceDbEntryT
{
    uint32_t                      keyRifId;
    sai_object_id_t               vrfId;
    sai_object_id_t               portOid;
    sai_object_id_t               vlanOid;
    sai_object_id_t               brOid;
    sai_mac_t                     mac;
    sai_object_id_t               ingressAclId;
    sai_object_id_t               egressAclId;
    uint16_t                      rifCount;
    xpSaiInterfaceVirtualRifInfoT rifInfo[XP_SAI_INTERFACE_VIRTUAL_RIF_MAX];
    xpsInterfaceId_t              l3IntfId;
    int32_t                       rifType;
    uint32_t                      prevPortPktCmd;
    uint32_t                      mtu;
    uint32_t                      neighMissAction;
    bool                          adminV4State;
    bool                          adminV6State;
    bool                          mcAdminV4State;
    bool                          mcAdminV6State;
    bool                          rifCreated;
} xpSaiRouterInterfaceDbEntryT;


typedef struct _xpSaiRouterInterfaceStatisticDbEntryT
{
    // key
    uint32_t        keyRifId;
    sai_uint64_t    rxFrames;
    sai_uint64_t    rxOctets;
    sai_uint64_t    rxErrorFrames;
    sai_uint64_t    rxErrorOctets;
    sai_uint64_t    txFrames;
    sai_uint64_t    txOctets;
    sai_uint64_t    txErrorFrames;
    sai_uint64_t    txErrorOctets;

} xpSaiRouterInterfaceStatisticDbEntryT;

sai_status_t xpSaiConvertRouterInterfaceOid(sai_object_id_t rif_oid,
                                            uint32_t *rif_id);
XP_STATUS xpSaiRouterInterfaceApiInit(uint64_t flag,
                                      const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiRouterInterfaceApiDeinit();
XP_STATUS xpSaiRouterInterfaceInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiRouterInterfaceDeInit(xpsDevice_t xpSaiDevId);
sai_status_t xpSaiGetRouterInterfaceAttrPortId(sai_object_id_t rif_id,
                                               sai_attribute_value_t* value);
sai_status_t xpSaiGetRouterInterfaceAttrVlanId(sai_object_id_t rif_id,
                                               sai_attribute_value_t* value);
sai_status_t xpSaiRouterInterfaceGetVrfId(sai_object_id_t rif_id,
                                          uint32_t *vrfId);
bool xpSaiRouterInterfaceLoopbackIdCheck(sai_object_id_t rif_id);
bool xpSaiRouterInterfaceVirtualCheck(sai_object_id_t rif_id);
XP_STATUS xpSaiRouterInterfaceAdminV4StateApply(xpsDevice_t xpsDevId,
                                                xpsInterfaceId_t l3IntfId);
XP_STATUS xpSaiRouterInterfaceAdminV6StateApply(xpsDevice_t xpsDevId,
                                                xpsInterfaceId_t l3IntfId);


sai_status_t xpSaiMaxCountRouterInterfaceAttribute(uint32_t *count);
sai_status_t xpSaiCountRouterInterfaceObjects(uint32_t *count);
sai_status_t xpSaiGetRouterInterfaceObjectList(uint32_t *object_count,
                                               sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetRouterInterfaceAttributes(sai_object_id_t id,
                                                   uint32_t *attr_count, sai_attribute_t *attr_list);

/**
 * \brief Function to obtain L3 interface ID from RIF OID
 *
 * \param [in]  rif_oid   Router Interface object ID
 * \param [out] pL3IntfId pointer to L3 interface value storage
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetRouterInterfaceL3IntfId(sai_object_id_t rif_oid,
                                             xpsInterfaceId_t *pL3IntfId);

/**
 * \brief Function to obtain RIF entry from RIF database
 *
 * \param [in]  rifId                  Router Interface object ID
 * \param [out] ppRouterInterfaceEntry pointer to pointer for obtained RIF DB entry
 *
 * \return sai_status_t
 */
sai_status_t xpSaiRouterInterfaceDbInfoGet(sai_object_id_t rifId,
                                           xpSaiRouterInterfaceDbEntryT **ppRouterInterfaceEntry);

/**
 * \brief Callback function for Bridge to create bridge based L3 interface ID
 * and attach it to the Bridge Router Interface
 *
 * \param [in]  rif_oid    Router Interface object ID
 * \param [in]  bridge_id  Bridge object ID
 * \param [out] l3_intf_id pointer to L3 interface value storage
 *
 * \return sai_status_t
 */
sai_status_t xpSaiRouterInterfaceOnBridgePortAdded(sai_object_id_t rif_oid,
                                                   sai_object_id_t bridge_id, xpsInterfaceId_t *l3_intf_id);

/**
 * \brief Callback function for Bridge to detach from RIF and remove bridge based L3 interface ID
 *
 * \param [in] rif_oid Router Interface object ID
 *
 * \return sai_status_t
 */
sai_status_t xpSaiRouterInterfaceOnBridgePortRemoved(sai_object_id_t rif_oid);

/**
 * \brief Check if router interface based on some LAG or port exists
 *
 * \param [in] oid port or LAG oid
 *
 * \return bool
 */
bool xpSaiPortBasedRifEntryExists(sai_object_id_t oid);


sai_status_t xpSaiConfigIpCtrlTrap(sai_object_id_t rif_id, bool isDelete);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiRouterInterface_h_

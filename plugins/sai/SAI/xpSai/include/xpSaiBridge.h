// xpSaiBridge.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiBridge_h_
#define _xpSaiBridge_h_

#include "xpSai.h"
#include "xpSaiVlan.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief sub-port data representation
 */
typedef struct _xpSaiSubportT
{
    sai_object_id_t portId;     //!< port OID
    sai_uint16_t    vlanId;     //!< VLAN ID
} xpSaiSubport_t;

/**
 *  \brief Support for unified DataTransferObject
 */
typedef union
{
    sai_object_id_t objId;      //!< object OID holder
    xpSaiSubport_t  subportId;  //!< sub-port holder
} xpSaiBridgePortDescr_t;

/**
 * \brief DataTransferObject represent BridgePort mapping for an
 *        any external (for bridge module) modules' requests
 */
typedef struct _xpSaiBridgePortT
{
    sai_object_id_t        bridgeId;        //!< corresponding bridge
    sai_bridge_port_type_t brPortType;      //!< port type
    xpSaiBridgePortDescr_t brPortDescr;     //!< port mapping
} xpSaiBridgePort_t;

/**
 * \brief data holder for external mapping entries
 */
typedef struct _xpSaiBridgeMapItemT
{
    sai_object_id_t key;            //!< general key OID
    sai_object_id_t bridgePort;     //!< corresponded BridgePort OID
} xpSaiBridgeMapItem_t;


/**
 * \brief Standard xpSai module initialization interface
 */
XP_STATUS xpSaiBridgeInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiBridgeDeInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiBridgeApiInit(uint64_t flag,
                             const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiBridgeApiDeinit();

/**
 * \brief Bridge attributes getter
 * \param bridge_id  bridge OID
 * \param attr_count attrs count
 * \param attr_list  attributes to get
 * \return SAI_STATUS
 */
sai_status_t xpSaiGetBridgeAttributes(sai_object_id_t bridge_id,
                                      uint32_t attr_count, sai_attribute_t *attr_list);

/**
 * \brief Create .1Q bridge ports for all PHY ports
 * \param xpsDevId  device identifier
 * \return SAI_STATUS
 */
sai_status_t xpSaiCreateDefaultBridgePorts(xpsDevice_t xpsDevId);

/**
 * \brief Allocate new XPS entry for holding mapping information related to external modules
 *        (for example: module VLAN can hold its members relations)
 * \param entryId  unique SAI object ID
 * \return XP_STATUS
 */
XP_STATUS xpSaiBridgeCreateMappingEntry(sai_object_id_t entryId);

/**
 * \brief Delete previously allocated mapping entry
 * \param entryId  unique SAI object ID
 * \return XP_STATUS
 */
XP_STATUS xpSaiBridgeRemoveMappingEntry(sai_object_id_t entryId);

/**
 * \brief Add new element to the existing mapping entry
 * \param entryId  existing entry ID
 * \param elemId key data, must be unique inside concrete 'entryId' namespace
 * \param bridgePortId value data, refers to the existing bridge port linked to an object referred by 'elemId'
 * \return XP_STATUS
 */
XP_STATUS xpSaiBridgeAddMappingElement(sai_object_id_t entryId,
                                       sai_object_id_t elemId, sai_object_id_t bridgePortId);

/**
 * \brief Delete existing mapping element
 * \param entryId  existing entry ID
 * \param elemId key data refers to the existing mapping element
 * \return XP_STATUS
 */
XP_STATUS xpSaiBridgeDelMappingElement(sai_object_id_t entryId,
                                       sai_object_id_t elemId);

/**
 * \brief Get mapping element by its key ID
 * \param entryId  entry ID
 * \param elemId element ID
 * \param dest pointer to a destination container
 * \return XP_STATUS
 */
XP_STATUS xpSaiBridgeGetMappingElement(sai_object_id_t entryId,
                                       sai_object_id_t elemId, sai_object_id_t *dest);

/**
 * \brief Get number of elements related to the specified mapping entry
 * \param entryId  entry ID
 * \param dest_size pointer to a array size holder
 * \return XP_STATUS
 */
XP_STATUS xpSaiBridgeGetMappingEntrySize(sai_object_id_t entryId,
                                         uint32_t *dest_size);

/**
 * \brief Get all elements related to the specified mapping entry
 * \param entryId  entry ID
 * \param dest_size pointer to a array size holder
 * \param dest pointer to a destination container
 * \return XP_STATUS
 * \note in case mapping data has more elements than container can hold XP_ERR_INVALID_DATA_SIZE will return
 *       and real size will be stored in 'dest_size'
 */
XP_STATUS xpSaiBridgeGetMappingEntry(sai_object_id_t entryId,
                                     uint32_t *dest_size, xpSaiBridgeMapItem_t *dest);

/**
 * \brief Get 'real' objects representation for specified Bridge Port
 * \param brPortId  pridge port ID
 * \param dest pointer to a destination container
 * \return XP_STATUS
 */
XP_STATUS xpSaiGetBridgePortById(sai_object_id_t brPortId,
                                 xpSaiBridgePort_t* dest);

/**
 * \brief Count bridge port in a particular bridge
 * \param bridgeId  pridge ID
 * \param dest pointer to a destination container
 * \return XP_STATUS
 */
XP_STATUS xpSaiGetBridgePortsSize(sai_object_id_t bridgeId, uint32_t *dest);

/**
 * \brief Get 'real' objects representation all ports connected to the specified Bridge
 * \param bridgeId  pridge ID
 * \param dest_size pointer to a array size holder
 * \param dest pointer to a destination container
 * \return XP_STATUS
 * \note in case mapping data has more elements than container can hold XP_ERR_INVALID_DATA_SIZE will return
 *       and real size will be stored in 'dest_size'
 */
XP_STATUS xpSaiGetBridgePortsByBridgeId(sai_object_id_t bridgeId,
                                        uint32_t *dest_size, xpSaiBridgePort_t* dest);

/**
 * \brief Compose SAI bridge port OID from xpsInterfaceId_t
 * \param xpsDevId  device ID
 * \param localId  interface ID
 * \param brPortType  type of bridge port to create
 * \param oid resulting SAI object ID
 * \return sai_status_t
 * \node It is used as we have to encode object ID using extended parameter list (type of bridge port)
 */
sai_status_t xpSaiBridgePortObjIdCreate(xpsDevice_t xpsDevId, uint32_t localId,
                                        sai_bridge_port_type_t brPortType, sai_object_id_t *oid);

/**
 * \brief Get SAI bridge port type from bridge port OID
 * \param brPortObjId  bridge port OID
 */
sai_int32_t xpSaiBridgePortObjIdTypeGet(sai_object_id_t brPortObjId);

/**
 * \brief Bind service instance ID to the BridgePort and program it to the .1D bridge
 * \param brPortId  bridge port
 * \param servInstanceId  corresponded service instance ID
 * \node BridgePort type has to be equal to SAI_BRIDGE_PORT_TYPE_TUNNEL
 * \return sai_status_t
 */
sai_status_t xpSaiBridgeAddVxTunnel(sai_object_id_t brPortId,
                                    xpsInterfaceId_t intfId, uint32_t servInstanceId);

/**
 * \brief Remove binding for corresponded service instance and BridgePort
 * \param brPortId  bridge port
 * \node BridgePort type has to be equal to SAI_BRIDGE_PORT_TYPE_TUNNEL
 * \return sai_status_t
 */
sai_status_t xpSaiBridgeRemoveVxTunnel(sai_object_id_t brPortId,
                                       xpsInterfaceId_t intfId);

/**
 * \brief Get bridge ID by bridge port ID
 * \param brPortId  bridge port
 * \param bridgeId pointer to a destination container
 * \return sai_status_t
 */
sai_status_t xpSaiGetBridgeIdByBridgePort(sai_object_id_t brPortId,
                                          sai_object_id_t *bridgeId);

/**
 * \brief Get bridge ID by bridge port ID
 * \param brPortId  bridge port
 * \param tunnelId pointer to a destination container
 * \return sai_status_t
 */
sai_status_t xpSaiGetTunnelIdByBridgePort(sai_object_id_t brPortId,
                                          sai_object_id_t *tunnelId);

/**
 * \brief Get Type by bridge port ID
 * \param brPortId  bridge port
 * \param type pointer to a bridge port type
 * \return sai_status_t
 */
sai_status_t xpSaiGetBridgePortTypeById(sai_object_id_t brPortId,
                                        sai_bridge_port_type_t* type);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiBridge_h_

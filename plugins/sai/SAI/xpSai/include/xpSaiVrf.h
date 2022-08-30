// xpSaiVrf.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiVrf_h_
#define _xpSaiVrf_h_

#include "xpSai.h"

#include "xpEnums.h"
#include "xpTypes.h"
#include "xpsInterface.h"

/**
 * \brief VRF identifier that is present by default
 */
#define XP_SAI_VRF_DEFAULT_ID 0

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file xpSaiVrf.h
 * \brief This file contains API prototypes and type definitions
 *        for the VRF
 */


/**
 * \brief State structure maintained by VRF
 *
 * This state contains the VRF configuration
 *
 * This state is internal is not exposed to the user
 */
typedef struct xpSaiVrfDbEntry_t
{
    uint32_t    keyVrfId;     ///< VRF identifier. It is as a key
    uint8_t     adminV4State; ///< IPv4 routing administrative state
    uint8_t     adminV6State; ///< IPv6 routing administrative state
    macAddr_t   macAddress;   ///< Source MAC address
    uint32_t    routesNum;    ///< Routes number
    uint32_t    hostsNum;     ///< Hosts number
    uint32_t    rifsNum;      ///< Routing interfaces number
    sai_packet_action_t     unkL3MCPktAction;
    uint32_t    xpsVrfdefUcNhId; // NH TBL HW ID
    uint32_t    xpsVrfdefMcNhId; // NH TBL HW ID
} xpSaiVrfDbEntry_t;

/**
 * \brief State structure maintained by VRF
 *
 * This state contains the relationship between a VRF
 * and the routing interfaces
 *
 * This state is internal and is not exposed to the user
 */
typedef struct xpSaiVrfIntfDbEntry_t
{
    xpsInterfaceId_t keyIntfId;    ///< Routing interface identifier. It is as a key
    uint32_t         vrfId;        ///< VRF identifier
    uint8_t          adminV4State; ///< IPv4 routing administrative state
    uint8_t          adminV6State; ///< IPv6 routing administrative state
    uint8_t
    mcAdminV4State; ///< IPv4 multicast routing administrative state
    uint8_t
    mcAdminV6State; ///< IPv6 multicast routing administrative state
    macAddr_t        mac;          ///< Source MAC address
    sai_object_id_t  portOid;      ///< Port/Lag Oid
    sai_object_id_t  vlanOid;      ///< VLAN ID
    int32_t          rifType;      ///< type of RIF (vlan/Port)
} xpSaiVrfIntfDbEntry_t;

/**
 * \brief API that initializes the VRF
 *
 * API will register for the state databases that are
 * needed by the VRF
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfInit(xpsDevice_t xpSaiDevId);

/**
 * \brief API to De-Init the VRF
 *
 * This API will Deregister all state databases for VRF
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfDeInit(xpsDevice_t xpSaiDevId);

/**
 * \brief API to create a new VRF with an available id
 *
 * \param [in] devId Device identifier
 * \param [out] pVrfId VRF identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfCreate(xpDevice_t devId, uint32_t* pVrfId);

/**
 * \brief API to remove an existing VRF in the system
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfRemove(xpDevice_t devId, uint32_t vrfId);

/**
 * \brief API to search an existing VRF in the system
 *
 * \param [in] vrfId VRF identifier
 *
 * \return int
 */
int xpSaiVrfExists(uint32_t vrfId);

/**
 * \brief API to set IPv4 admin state for a particular VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 * \param [in] adminV4State IPv4 routing administrative state
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfAdminV4StateSet(xpDevice_t devId, uint32_t vrfId,
                                  uint32_t adminV4State);

/**
 * \brief API to get IPv4 admin state for a particular VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 * \param [out] pAdminV4State IPv4 routing administrative state
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfAdminV4StateGet(xpDevice_t devId, uint32_t vrfId,
                                  uint32_t* pAdminV4State);

/**
 * \brief API to set IPv6 admin state for a particular VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 * \param [in] adminV6State IPv6 routing administrative state
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfAdminV6StateSet(xpDevice_t devId, uint32_t vrfId,
                                  uint32_t adminV6State);

/**
 * \brief API to get IPv6 admin state for a particular VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 * \param [out] pAdminV6State IPv6 routing administrative state
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfAdminV6StateGet(xpDevice_t devId, uint32_t vrfId,
                                  uint32_t* pAdminV6State);

/**
 * \brief API to set MAC address for a particular VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 * \param [in] macAddress Source MAC address
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfMacAddressSet(xpDevice_t devId, uint32_t vrfId,
                                macAddr_t macAddress);

/**
 * \brief API to get MAC address for a particular VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 * \param [out] macAddress Source MAC address
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfMacAddressGet(xpDevice_t devId, uint32_t vrfId,
                                macAddr_t macAddress);

/**
 * \brief API to increment number of ip routes associted with the VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfRoutesNumInc(xpDevice_t devId, uint32_t vrfId);

/**
 * \brief API to decrement number of ip routes associted with the VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfRoutesNumDec(xpDevice_t devId, uint32_t vrfId);

/**
 * \brief API to increment number of ip hosts associted with the VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfHostsNumInc(xpDevice_t devId, uint32_t vrfId);

/**
 * \brief API to decrement number of ip hosts associted with the VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfHostsNumDec(xpDevice_t devId, uint32_t vrfId);

/**
 * \brief Set the number of ip hosts to 0 for all VRFs
 *
 * \param [in] devId Device identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfHostsClear(xpDevice_t devId);

/**
 * \brief API find that port/vlan already added
 *
 * \param [oid] vlanOid/portOid
 *
 * \return xpSaiVrfIntfDbEntry_t
 */
xpSaiVrfIntfDbEntry_t *xpSaiIsVlanOrPortRifExist(sai_object_id_t oid);

/**
 * \brief API to add an interface to VRF internal DB
 *
 * \param [in] intfId Routing interface identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfAdd(xpsInterfaceId_t intfId);

/**
 * \brief API to remove an interface from VRF internal DB
 *
 * \param [in] intfId Routing interface identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfRemove(xpsInterfaceId_t intfId);

/**
 * \brief API to update a routing interface VRF id
 *
 * \param [in] devId Device identifier
 * \param [in] intfId Routing interface identifier
 * \param [in] vrfId VRF identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfVrfIdUpdate(xpDevice_t devId, xpsInterfaceId_t intfId,
                                  uint32_t vrfId);

/**
 * \brief API to set IPv4 admin state for a routing interface
 *
 * \param [in] intfId Routing interface identifier
 * \param [in] adminV4State IPv4 routing administrative state
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfAdminV4StateSet(xpsInterfaceId_t intfId,
                                      uint32_t adminV4State);

/**
 * \brief API to get IPv4 admin state for a routing interfaceF
 *
 * \param [in] intfId Routing interface identifier
 * \param [out] pAdminV4State IPv4 routing administrative state
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfAdminV4StateGet(xpsInterfaceId_t intfId,
                                      uint32_t* pAdminV4State);

/**
 * \brief API to set IPv6 admin state for a routing interface
 *
 * \param [in] intfId Routing interface identifier
 * \param [in] adminV6State IPv6 routing administrative state
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfAdminV6StateSet(xpsInterfaceId_t intfId,
                                      uint32_t adminV6State);

/**
 * \brief API to get IPv6 admin state for a routing interface
 *
 * \param [in] intfId Routing interface identifier
 * \param [out] pAdminV6State IPv6 routing administrative state
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfAdminV6StateGet(xpsInterfaceId_t intfId,
                                      uint32_t* pAdminV6State);

/**
 * \brief API to set IPv4 admin state for a multicast routing interface
 *
 * \param [in] intfId Routing interface identifier
 * \param [in] mcAdminV4State IPv4 multicast routing administrative state
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfMcAdminV4StateSet(xpsInterfaceId_t intfId,
                                        uint32_t mcAdminV4State);

/**
 * \brief API to get IPv4 admin state for a multicast routing interface
 *
 * \param [in] intfId Routing interface identifier
 * \param [out] pAdminV4State IPv4 multicast routing administrative state
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfMcAdminV4StateGet(xpsInterfaceId_t intfId,
                                        uint32_t* pMcAdminV4State);

/**
 * \brief API to set IPv6 admin state for a multicast routing interface
 *
 * \param [in] intfId Routing interface identifier
 * \param [in] adminV6State IPv6 multicast routing administrative state
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfMcAdminV6StateSet(xpsInterfaceId_t intfId,
                                        uint32_t mcAdminV6State);

/**
 * \brief API to get IPv6 admin state for a multicast routing interface
 *
 * \param [in] intfId Routing interface identifier
 * \param [out] pAdminV6State IPv6 multicast routing administrative state
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfMcAdminV6StateGet(xpsInterfaceId_t intfId,
                                        uint32_t* pMcAdminV6State);

/**
 * \brief API to set Source Mac address for a routing interface
 *
 * \param [in] intfId Routing interface identifier
 * \param [in] macAddress Mac address
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfSrcMacAddrSet(xpsInterfaceId_t intfId,
                                    macAddr_t macAddress);

/**
 * \brief API to get Source Mac address for a routing interface
 *
 * \param [in] intfId Routing interface identifier
 * \param [out] macAddress Mac address
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfSrcMacAddrGet(xpsInterfaceId_t intfId,
                                    macAddr_t macAddress);

/**
 * \brief API to set a port for a routing interface
 *
 * \param [in] intfId Routing interface identifier
 * \param [in] portOid Port identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfPortSet(xpsInterfaceId_t intfId, sai_object_id_t portOid);

/**
 * \brief API to set a VLAN ID for a routing subinterface
 *
 * \param [in] intfId Routing interface identifier
 * \param [in] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfVlanIdSet(xpsInterfaceId_t intfId,
                                sai_object_id_t vlanOid);

/**
 * \brief API to set a Type (vlan/port) for a routing interface
 *
 * \param [in] intfId Routing interface identifier
 * \param [in] rifType Type of RIF
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfTypeSet(xpsInterfaceId_t intfId, int32_t rifType);
/**
 * \brief API to get a type of a routing interface
 *
 * \param [in] intfId Routing interface identifier
 * \param [out] rifType rif type (Vlan/port)
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfTypeGet(xpsInterfaceId_t intfId, int32_t *rifType);

/**
 * \brief API to get a port for a routing interface
 *
 * \param [in] intfId Routing interface identifier
 * \param [out] portOid Port identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfPortGet(xpsInterfaceId_t intfId,
                              sai_object_id_t *portOid);

/**
 * \brief API to get a VLAN ID for a routing subinterface
 *
 * \param [in] intfId Routing interface identifier
 * \param [out] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfVlanIdGet(xpsInterfaceId_t intfId,
                                sai_object_id_t *vlanOid);

/**
 * \brief API to get a routing interface VRF id
 *
 * \param [in] intfId Routing interface identifier
 * \param [out] pVrfId VRF identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfVrfIdGet(xpsInterfaceId_t intfId, uint32_t* pVrfId);

/**
 * \brief API to get a VRF Max id
 *
 * \param [out] pMaxVrfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfMaxIdGet(xpDevice_t devId, uint32_t* pMaxVrfId);

sai_status_t xpSaiCountVrfObjects(uint32_t *count);
sai_status_t xpSaiCountVrfIntfObjects(uint32_t *count);

sai_status_t xpSaiGetVrfObjectList(uint32_t *object_count,
                                   sai_object_key_t *object_list);
sai_status_t xpSaiGetVrfIntfObjectList(uint32_t *object_count,
                                       sai_object_key_t *object_list);
XP_STATUS xpSaiVrfIntfInfoGet(xpsInterfaceId_t intfId,
                              xpSaiVrfIntfDbEntry_t **ppVrfIntfInfo);

sai_status_t xpSaiVrfUnkL3MCPktActionSet(xpDevice_t devId, uint32_t vrfId,
                                         sai_packet_action_t pktAct);
sai_status_t xpSaiVrfUnkL3McPktActionGet(xpDevice_t devId, uint32_t vrfId,
                                         sai_packet_action_t *action);
sai_status_t xpSaiVrfFlushAll(xpDevice_t devId);

#ifdef __cplusplus
}
#endif


#endif  //_xpSaiVrf_h_

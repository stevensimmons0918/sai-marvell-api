// xpsInterface.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpsInterface.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Interface Manager
 */

#ifndef _xpsInterface_h_
#define _xpsInterface_h_

#include "xpTypes.h"
#include "xpEnums.h"
#include "xpsEnums.h"
#include "xpsRBTree.h"
#include "openXpsInterface.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def XPS_VERSION
 * \brief Versioning information for the current release
 */
#define XPS_VERSION 3


#define XPS_INVALID_STP 0xFFFF

/**
 * \def XPS_INTF_BD_OFFSET
 *
 * This offset is used to differentiate between vif interfaces
 * and bd interfaces
 */
#define XPS_INTF_BD_OFFSET 65536

/**
 * \def XPS_INTF_MAP_INTF_TO_BD(intf)
 *
 * This Macro will offset an interface id to that of a BD id
 */
#define XPS_INTF_MAP_INTF_TO_BD(intf) ((uint32_t)((intf != 0xFFFFFFFF)  ? ((intf >= XPS_INTF_BD_OFFSET) ? intf - XPS_INTF_BD_OFFSET : intf) : 0 ))

/**
 * \def XPS_INTF_MAP_BD_TO_INTF(bd)
 *
 * This Macro will add an offset to a BD Id to map it into an
 * interface Id
 */
#define XPS_INTF_MAP_BD_TO_INTF(bd) ((xpsInterfaceId_t)((bd < XPS_INTF_BD_OFFSET) ? bd + XPS_INTF_BD_OFFSET : bd))

#define XPS_INTF_MAP_INTF_AND_VLAN_TO_BRIDGE_PORT(intfId, vlanId) (((vlanId & 0xFFF) << 20) | (intfId & 0xFFFFF))
#define XPS_INTF_MAP_BRIDGE_PORT_TO_VLAN(bridgePort) ((bridgePort >> 20) & 0xFFF)
#define XPS_INTF_MAP_BRIDGE_PORT_TO_INTF(bridgePort) (bridgePort & 0xFFFFF)
#define XPS_INTF_IS_BRIDGE_PORT(intfId) ((intfId >> 20) & 0xFFF) ? true : false

/**
 * \def XPS_INTF_DEFAULT_ECMP_SIZE
 *
 * Default ECMP size
 */
#define XPS_INTF_DEFAULT_ECMP_SIZE 1

/**
 * \def XPS_AC_INTERFACE_START_OFFSET
 *
 * Start interface Id for AC interfaces
 *
 * TODO: Consider moving this to range profiles
 */
#define XPS_AC_INTERFACE_START_OFFSET       (19*1024)

typedef struct xpsInterfaceChildInfo_t
{
    xpsInterfaceId_t    keyIntfId;
    xpsInterfaceType_e  type;
    uint32_t hwId; // can be eVlan/ePort
} xpsInterfaceChildInfo_t;

/*
 * Interace Structure Definitions
 */

/**
 * \struct xpsInterfaceInfo_t
 *
 * This structure maintains type and l2 ecmp size information for
 * each interface
 */
typedef struct xpsInterfaceInfo_t
{
    xpsInterfaceId_t    keyIntfId;
    xpsInterfaceType_e  type;
    uint32_t            l2EcmpSize;
    /* Stores the mapping between L3Interface and PortInterface*/
    struct interfaceMappingInfo_t
    {
        xpsInterfaceType_e type;
        xpsInterfaceId_t keyIntfId;
    } intfMappingInfo;
    macAddr_t           egressSAMac; /*only valid if the intf is of type l3*/
    uint32_t
    mtuPorfileIndex;     /* MTU Profile Index for L3 MTU used in Next Hop*/
    void*               pMetadata;
    bool                isAclEnable;
    uint32_t            aclTableId;
    uint32_t            aclGroupId;
    uint32_t            bdHWId; /*Holds extend Vlan Id configured in HW*/
    uint32_t            stpId;
    uint32_t            vrfId; /* Holds Vrf-Id for L3 interface */
    uint32_t            mac2meHwIdx; /* Holds Mac2Me HW index for L3 interface */
    /* Holds Child rif to tnl list. For port can LAG or Vlan list.
       NOTE: same port cannot be part of LAG and VLAN.
             untag port cannot be part of multiple vlans.
             tagged port can be part of multiple vlans.
       For other obj, this will be NULL*/
    xpsRBTree_t         *rbtree;
} xpsInterfaceInfo_t;

/**
 * \struct xps8021BrInterfaceMetadata_t
 *
 * This structure is 802.1BR extended port metadata for
 * xpsInterfaceInfo_t
 */
typedef struct
{
    xpsPeg_t            groupId;
    uint32_t            cascadePortsCount;
    uint32_t            maxCascadePortsCount;
    xpsInterfaceId_t*   cascadePorts; // size equals maxCascadePortsCount
} xps8021BrInterfaceMetadata_t;

/**
 * \struct xpsPhysicalPortMetadata_t
 *
 * This structure is physical port metadata for
 * xpsInterfaceInfo_t
 */
typedef struct
{
    uint32_t numberOfUsagesAsCascadePort;
} xpsPhysicalPortMetadata_t;

/*
 * Interface Mgr API Prototypes
 */

/**
 * \public
 * \brief Init API for Interface Manager for a scope
 *
 * This API will create and initialize the global interface
 * database, that will be used to maintain the Interface Id to
 * type relationship
 *
 * \param [in] scopeId
 *
 * \return int
 */
XP_STATUS xpsInterfaceInitScope(xpsScope_t scopeId);

/**
 * \brief Add Device API for Interface Manager
 *
 * Add Device API for Interface Manager. Is a NOP as the
 * Interface Mgr does not maintain any per device information
 *
 * \param [in] devId
 *
 * \return int
 */
XP_STATUS xpsInterfaceAddDevice(xpsDevice_t devId);

/**
 * \brief Remove Device API for Interface Manager
 *
 * Remove Device API for Interface Manager. Is a NOP as the
 * Interface Mgr does not maintain any per device information
 *
 * \param [in] devId
 *
 * \return int
 */
XP_STATUS xpsInterfaceRemoveDevice(xpsDevice_t devId);

/**
 * \brief De-Init API for Interface Manager for a scope
 *
 * De-Init API for Interface Manager. This API will destroy the
 * global Interface Id to Type database
 *
 * \param [in] scopeId
 *
 * \return int
 */
XP_STATUS xpsInterfaceDeInitScope(xpsScope_t scopeId);

/**
 * \brief This method checks if a particular interface exists.
 *
 * \param [in] scopeId
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceIsExistScope(xpsScope_t scopeId, xpsInterfaceId_t intfId);

/**
 * \brief Create a new Interface in the system for a scope.
 *
 * Creates a new interface and stores its type information
 * inside the database
 *
 * \param [in] scopeId
 * \param [in] type
 * \param [out] intfId
 *
 * \return int
 */
XP_STATUS xpsInterfaceCreateScope(xpsScope_t scopeId, xpsInterfaceType_e type,
                                  xpsInterfaceId_t *intfId);

/**
 * \brief Create a router interface over an existing bd for a scope
 *
 * Creates a router interface over an existing vlan. This router
 * interface id will be used for subsequent router properties
 * programming
 *
 * \param [in] scopeId
 * \param [in] vlanId
 * \param [out] intfId
 *
 * \return int
 */
XP_STATUS xpsInterfaceCreateRouterOverVlanScope(xpsScope_t scopeId,
                                                uint32_t vlanId, xpsInterfaceId_t *intfId);

/**
 * \brief Destroys an existing Interface in the system for a scope
 *
 * Destroys an existing interface and removes the information
 * stored from the database
 *
 * \param [in] scopeId
 * \param [in] intfId
 *
 * \return int
 */
XP_STATUS xpsInterfaceDestroyScope(xpsScope_t scopeId, xpsInterfaceId_t intfId);

/**
 * \brief Destroys a Router interface over a vlan for a scope.
 *
 * Destroys a router interface over an existing vlan
 *
 * \param [in] scopeId
 * \param [in] vlanId
 * \param [in] intfId
 *
 * \return int
 */
XP_STATUS xpsInterfaceDestroyRouterOverVlanScope(xpsScope_t scopeId,
                                                 uint32_t vlanId, xpsInterfaceId_t intfId);

/**
 * \brief Retrieve an interface's properties for a scope
 *
 * Fetches information about an existing interface created in
 * the system
 *
 *
 * \param [in] scopeId
 * \param [in] intfId
 * \param [out] info
 *
 * \return int
 */
XP_STATUS xpsInterfaceGetInfoScope(xpsScope_t scopeId, xpsInterfaceId_t intfId,
                                   xpsInterfaceInfo_t **info);

/**
 * \brief Retrieve's an interface's type information for a scope
 *
 * There is no set interface type API. In order to change an
 * interface's type, the interface must be destroyed and
 * recreated
 *
 * \param [in] scopeId
 * \param [in] intfId
 * \param [out]  type
 *
 * \return int
 */
XP_STATUS xpsInterfaceGetTypeScope(xpsScope_t scopeId, xpsInterfaceId_t intfId,
                                   xpsInterfaceType_e *type);

/**
 * \brief Sets an interface's L2 ecmp size property for a scope
 *
 * \param [in] scopeId
 * \param [in] intfId
 * \param [in] l2EcmpSize
 *
 * \return int
 */
XP_STATUS xpsInterfaceSetL2EcmpSizeScope(xpsScope_t scopeId,
                                         xpsInterfaceId_t intfId, uint32_t l2EcmpSize);

/**
 * \public
 * \brief Retrieves an interface's L2 ecmp size property for scope
 *
 *
 * \param [in] scopeId
 * \param [in] intfId
 * \param [out] l2EcmpSize
 *
 * \return int
 */
XP_STATUS xpsInterfaceGetL2EcmpSizeScope(xpsScope_t scopeId,
                                         xpsInterfaceId_t intfId, uint32_t *l2EcmpSize);

/**
 * \brief Sets ports into an ingress interface on a per device basis
 *
 * This API will set ports into an ingress interface on a per device
 * basis
 *
 * \param [in] devId
 * \param [in] intfId
 * \param [in] portList
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceSetIngressPorts(xpsDevice_t devId,
                                      xpsInterfaceId_t intfId, xpsPortList_t *portList);

/**
 * \brief Sets ports into an egress interface on a per device basis
 *
 * This API will set ports into an egress interface on a per device
 * basis
 *
 * \param [in] devId
 * \param [in] intfId
 * \param [in] portList
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceSetEgressPorts(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                     xpsPortList_t *portList);

/**
 * \brief Sets ports into an interface on a per device basis
 *
 * This API will set ports into an interface on a per device
 * basis
 *
 * \param [in] devId
 * \param [in] intfId
 * \param [in] portList
 *
 * \return int
 */
XP_STATUS xpsInterfaceSetPorts(xpsDevice_t devId, xpsInterfaceId_t intfId,
                               xpsPortList_t *portList);

/**
 * \brief Retrieves ports form an ingress interface on a per device
 *        basis
 *
 * This API will get ports from an ingress interface on a per device
 * basis
 *
 * \param [in] devId
 * \param [in] intfId
 * \param [out] portList
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceGetIngressPorts(xpsDevice_t devId,
                                      xpsInterfaceId_t intfId, xpsPortList_t *portList);

/**
 * \brief Retrieves ports form an interface on a per device
 *        basis
 *
 * This API will get ports from an interface on a per device
 * basis
 *
 * \param [in] devId
 * \param [in] intfId
 * \param [out] portList
 *
 * \return int
 */
XP_STATUS xpsInterfaceGetPorts(xpsDevice_t devId, xpsInterfaceId_t intfId,
                               xpsPortList_t *portList);

/**
 * \brief Pick the first interface id available in the system for a scope
 *
 * \param [in] scopeId
 * \param [out] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceGetFirstScope(xpsScope_t scopeId,
                                    xpsInterfaceId_t *intfId);

/**
 * \brief Get the next interface id available in the system for  a scope
 *
 * \param [in] scopeId
 * \param [in] intfId
 * \param [out] intfIdNext
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceGetNextScope(xpsScope_t scopeId, xpsInterfaceId_t intfId,
                                   xpsInterfaceId_t *intfIdNext);

/**
 * \brief Adds interface info into database for a scope
 *
 * \param [in] scopeId
 * \param [in] info
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceAddToDBScope(xpsScope_t scopeId,
                                   xpsInterfaceInfo_t *info);
/**
 * \brief Removes interface info from database for a scope
 *
 * \param [in] scopeId
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceRemoveFromDBScope(xpsScope_t scopeId,
                                        xpsInterfaceId_t intfId);

/**
 * \public
 * \brief Init API for Interface Manager
 *
 * This API will create and initialize the global interface
 * database, that will be used to maintain the Interface Id to
 * type relationship
 *
 * \return int
 */
XP_STATUS xpsInterfaceInit(void);

/**
 * \brief De-Init API for Interface Manager
 *
 * De-Init API for Interface Manager. This API will destroy the
 * global Interface Id to Type database
 *
 * \return int
 */
XP_STATUS xpsInterfaceDeInit(void);

/**
 * \brief This method checks if a particular interface exists.
 *
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceIsExist(xpsInterfaceId_t intfId);

/**
 * \brief Create a new Interface in the system
 *
 * Creates a new interface and stores its type information
 * inside the database
 *
 * \param [in] type
 * \param [out] intfId
 *
 * \return int
 */
XP_STATUS xpsInterfaceCreate(xpsInterfaceType_e type, xpsInterfaceId_t *intfId);

/**
 * \public
 * \brief Create a new SAI bridge port interface
 *
 * Creates a new logical bridgePort interface and stores its type information
 * inside the database
 *
 * \param [in] interface vifId
 * \param [in] vlanId
 * \param [out] bridgePort interfaceId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceCreateBridgePort(uint32_t vifId, uint16_t vlanId,
                                       xpsInterfaceId_t *intfId);

/**
 * \brief Create a router interface over an existing bd
 *
 * Creates a router interface over an existing vlan. This router
 * interface id will be used for subsequent router properties
 * programming
 *
 * \param [in] vlanId
 * \param [out] intfId
 *
 * \return int
 */
XP_STATUS xpsInterfaceCreateRouterOverVlan(uint32_t vlanId,
                                           xpsInterfaceId_t *intfId);

/**
 * \brief Destroys an existing Interface in the system
 *
 * Destroys an existing interface and removes the information
 * stored from the database
 *
 * \param [in] intfId
 *
 * \return int
 */
XP_STATUS xpsInterfaceDestroy(xpsInterfaceId_t intfId);

/**
 * \brief Destroys an existing bridgePort interface in the system
 *
 * Destroys an existing logical bridgePort interface and removes the information
 * stored from the database
 *
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceDestroyBridgePort(xpsInterfaceId_t intfId);

/**
 * \brief Destroys a Router interface over a vlan
 *
 * Destroys a router interface over an existing vlan
 *
 * \param [in] vlanId
 * \param [in] intfId
 *
 * \return int
 */
XP_STATUS xpsInterfaceDestroyRouterOverVlan(uint32_t vlanId,
                                            xpsInterfaceId_t intfId);

/**
 * \brief Retrieve an interface's properties
 *
 * Fetches information about an existing interface created in
 * the system
 *
 *
 * \param [in] intfId
 * \param [out] info
 *
 * \return int
 */
XP_STATUS xpsInterfaceGetInfo(xpsInterfaceId_t intfId,
                              xpsInterfaceInfo_t **info);

/**
 * \brief Sets an interface's L2 ecmp size property
 *
 * \param [in] intfId
 * \param [in] l2EcmpSize
 *
 * \return int
 */
XP_STATUS xpsInterfaceSetL2EcmpSize(xpsInterfaceId_t intfId,
                                    uint32_t l2EcmpSize);

/**
 * \public
 * \brief Retrieves an interface's L2 ecmp size property
 *
 *
 * \param [in] intfId
 * \param [out] l2EcmpSize
 *
 * \return int
 */
XP_STATUS xpsInterfaceGetL2EcmpSize(xpsInterfaceId_t intfId,
                                    uint32_t *l2EcmpSize);

/**
 * \brief Pick the first interface id available in the system
 *
 * \param [out] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceGetFirst(xpsInterfaceId_t *intfId);

/**
 * \brief Get the next interface id available in the system
 *
 * \param [in] intfId
 * \param [out] intfIdNext
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceGetNext(xpsInterfaceId_t intfId,
                              xpsInterfaceId_t *intfIdNext);

/**
 * \brief Adds interface info into database
 *
 * \param [in] info
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceAddToDB(xpsInterfaceInfo_t *info);

/**
 * \brief Removes interface info from database
 *
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceRemoveFromDB(xpsInterfaceId_t intfId);

/**
 * \brief To add instruction to strip E-tag from incoming packet
 *
 * \param [in] xpDevice_t devId
 * \param [in] uint32_t vifId
 * \param [in] unit8_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceStripIncomingEtag(xpsDevice_t devId,  uint32_t vifId,
                                        uint8_t enable);


XP_STATUS xpsVlanInterfaceDefaultIngressStpSet(xpsDevice_t devId,
                                               xpsInterfaceId_t intfId, xpsVlan_t vlanId);

/**
 * \brief Retrieves an interface's IP MTU Profile Index
 *        information
 *
 *
 * \param [in]  intfId                      Interface ID
 * \param [out] ipMtuProfileIndex           IP MTU profile index
 *
 * \return XP_STATUS
 */
XP_STATUS xpsInterfaceGetIpMtuProfileIndex(xpsInterfaceId_t intfId,
                                           uint32_t *ipMtuProfileIndex);

XP_STATUS xpsInterfaceGetFirstInfo(xpsScope_t scopeId,
                                   xpsInterfaceInfo_t **info);

XP_STATUS xpsInterfaceGetNextInfo(xpsScope_t scopeId, xpsInterfaceId_t intfId,
                                  xpsInterfaceInfo_t **intfo);

XP_STATUS xpsIntfChildAdd(xpsInterfaceInfo_t *info,
                          xpsInterfaceId_t intfId,
                          xpsInterfaceType_e type,
                          uint32_t hwId);
XP_STATUS xpsIntfChildRemove(xpsInterfaceInfo_t *info, xpsInterfaceId_t intfId);
XP_STATUS xpsIntfChildFind(xpsInterfaceInfo_t *info, xpsInterfaceId_t intfId,
                           xpsInterfaceChildInfo_t **ppInfo);
XP_STATUS xpsIntfChildGetNext(xpsInterfaceInfo_t *info,
                              xpsInterfaceChildInfo_t **nextInfo);

#ifdef __cplusplus
}
#endif

#endif // _xpsInterface_h_


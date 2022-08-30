// xpsLag.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpsLag.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Lag Manager
 */

#ifndef _xpsLag_h_
#define _xpsLag_h_

#include "xpTypes.h"
#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsLag.h"
#include "openXpsStp.h"
#include "xpsUtil.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XP_LAG_MAX_PORT_MASK_SIZE_IN_BITS   256     ///< XP_LAG_MAX_PORT_MASK_SIZE_IN_BITS   
#define XP_LAG_MAX_PORT_MASK_SIZE_BYTES     XP_LAG_MAX_PORT_MASK_SIZE_IN_BITS / SIZEOF_BYTE     ///< XP_LAG_MAX_PORT_MASK_SIZE_BYTES     

#define XP_LAG_START_BIT_FOR_CRC_32HI   16
#define XP_LAG_START_BIT_FOR_CRC_32LO   0
#define XP_LAG_NUM_OF_BITS_FOR_CRC      16
#define XP_LAG_START_BIT_FOR_ECMP_CRC   16
#define XP_LAG_START_BIT_FOR_ECMP_XOR   0
#define XP_LAG_NUM_OF_BITS_FOR_XOR      6
extern xpsDbHandle_t lagDbHandle;

/**
 * \brief Enum to describe if a port is operationally up or down
 *        after deployment
 */
typedef enum xpsLagPortOperState_e
{
    PORT_OPER_DISABLED = 0,
    PORT_OPER_ENABLED,
    PORT_OPER_MAX
} xpsLagPortOperState_e;
/**
 * \brief State structure maintained by XPS Lag
 *
 * This state contains the relationship between a lag interface
 * and the ports that are contained by it (these ports are port
 * interfaces)
 *
 * This state is internal to XPS and is not exposed to the user
 */
typedef struct xpLagDbEntry_t
{
    xpsInterfaceId_t      lagIntf;
    uint32_t              numPorts;
    bool                  ipv4RoutingEn;
    bool                  ipv6RoutingEn;
    bool                  ipv4McRoutingEn;
    bool                  ipv6McRoutingEn;
    xpsInterfaceId_t      portIntf[XPS_LAG_MAX_PORTS];
    xpsLagPortOperState_e ingrOper[XPS_LAG_MAX_PORTS];
    xpsLagPortOperState_e egrOper[XPS_LAG_MAX_PORTS];
    xpsDeviceMap          deviceMap;
    xpsInterfaceId_t
    l3IntfId;  // use this state variable to maintain L3Intf bind to LagIntf
    bool                  isMirrorIntf;
    uint32_t              mirrorAnalyzerId;
} xpLagDbEntry_t;

/**
 * \brief Structure defining the port mask for the Lag
 *        Distribution entry
 */
typedef struct xpLagDistributionEntry_t
{
    uint32_t portMaskSize;      ///< portMaskSize
    uint8_t  portMask[XP_LAG_MAX_PORT_MASK_SIZE_BYTES];     ///< portMask
} xpLagDistributionEntry_t;


/**
 * \enum for LAG HASH ALGO TYPES
 */
typedef enum xpsLagHashAlgoType_e
{
    /*HASH XOR based on packet data*/
    XPS_LAG_HASH_ALGO_XOR_E,

    /*HASH XOR based on ingress port*/
    XPS_LAG_HASH_ALGO_XOR_INGRESS_PORT_E,

    /*HASH CRC based on packet data*/
    XPS_LAG_HASH_ALGO_CRC_E,
    XPS_LAG_HASH_ALGO_RANDOM_E,
    XPS_LAG_HASH_ALGO_CRC_32HI_E,
    XPS_LAG_HASH_ALGO_CRC_32LO_E,
    XPS_LAG_HASH_ALGO_MAX
} xpsLagHashAlgoType_e;

/**
 * \brief API that initializes the XPS Lag Mgr
 *
 * This API will register for the state databases that are
 * needed by the Lag Mgr code base for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagInitScope(xpsScope_t scopeId);

/**
 * \brief API to perform Add Device operations for Lag Mgr
 *
 * This API is a NOP for Lag. As Lag operates primarily in a
 * global scope
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief API to perform Remove Device operations for Lag Mgr
 *
 * This API is a NOP for Lag. As Lag operates primiarily in a
 * global scope
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagRemoveDevice(xpsDevice_t devId);

/**
 * \brief API to De-Init the XPS Lag Mgr
 *
 * This API will Deregister all state databases for Lag Mgr for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagDeInitScope(xpsScope_t scopeId);

/**
 * \brief API to create a new Lag in the system for a scope.
 *
 * This API will use the facility provided by xpsInterface to
 * create a new Lag Interface and an associated Id. This Id will
 * be used by subsequent APIs to perform Lag operations and
 * deployment
 *
 * This API only modify software states. Deploy must be called
 * on each device in order to commit changes to Lag
 * configuration to Hardware
 *
 * \param [in] scopeId
 * \param [out] lagIntf
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagCreateScope(xpsScope_t scopeId, xpsInterfaceId_t *lagIntf);

/**
 * \brief API to destroy an existing Lag in the system for a scope
 *
 * This API will use the facility provided by xpsInterface to
 * destroy an existing Lag Interface and its associated Id
 *
 * There is an expectation, that in prior to calling the
 * xpsLagDestroy API, that all ports have been removed from the
 * Lag and using the xpsLagRemovePort API. In addition, after
 * removing all of these ports, that xpsDeployLag has been
 * called for all devices
 *
 * Calling xpsLagDestroy without following these steps can lead
 * to an inconsistent hardware state
 *
 * \see xpsLagRemovePort
 * \see xpsLagDeploy
 *
 * \param [in] scopeId
 * \param [in] lagIntf
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagDestroyScope(xpsScope_t scopeId, xpsInterfaceId_t lagIntf);

/**
 * \brief API to add a port to a Lag for a scope
 *
 * This API will add a port to a Lag. This API will only change
 * the software state of the Lag Interface. xpsLagDeploy will
 * need to be called to commit these changes
 *
 * This API expects a Port Interface Id
 *
 * \param [in] scopeId
 * \param [in] lagIntf
 * \param [in] portIntf
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagAddPortScope(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                             xpsInterfaceId_t portIntf);

/**
 * \brief API to remove a port from a Lag for a scope
 *
 * This API will remove a port from a Lag. This API will only
 * change the software state of the Lag Interface. xpsLagDeploy
 * will need tob e called to commit these changes
 *
 * This API expects a Port Interface Id
 *
 * \param [in] scopeId
 * \param [in] lagIntf
 * \param [in] portIntf
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagRemovePortScope(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                                xpsInterfaceId_t portIntf);

/**
 * \brief API to get list of Ingress port interfaces in the LAG
 *
 * \param [in] scopeId
 * \param [in] lagIntf
 * \param [out] portIntfList
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetIngressPortIntfList(xpsScope_t scopeId,
                                       xpsInterfaceId_t lagIntf, xpsLagPortIntfList_t *portIntfList);

/**
 * \brief API to get list of Egress port interfaces in the LAG
 *
 * \param [in] scopeId
 * \param [in] lagIntf
 * \param [out] portIntfList
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetEgressPortIntfList(xpsScope_t scopeId,
                                      xpsInterfaceId_t lagIntf, xpsLagPortIntfList_t *portIntfList);

/**
 * \brief API to get list of port interfaces in the LAG for a scope
 *
 * \param [in] scopeId
 * \param [in] lagIntf
 * \param [out] portIntfList
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetPortIntfListScope(xpsScope_t scopeId,
                                     xpsInterfaceId_t lagIntf, xpsLagPortIntfList_t *portIntfList);

/**
 * \public
 * \brief API to retrieve a Lag member's port ingress
 *        operational state
 *
 * This API will return a Lag member's port ingress operational
 * state if the member configuration has been applied to
 * hardware
 *
 * \param devId
 * \param lagIntf
 * \param portIntf
 * \param state
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetIngressOperationalState(xpsDevice_t devId,
                                           xpsInterfaceId_t lagIntf, xpsInterfaceId_t portIntf, uint32_t *state);

/**
 * \public
 * \brief API to set a Lag member's port ingress operational
 *        state to enabled
 *
 * This API will configure a member port in a LAG to
 * operationally on. This means that this port's Link is up and
 * is eligible to participate in a LAG. This concept is
 * different from a LAG membership as a whole as a port must be
 * a member to have a its operational state changed
 *
 * This can be used when the NOS detects a link up event for a
 * LAG member. It will allow for a quick rebalancing of the
 * LAG with minimal impact to traffic
 *
 * \param devId
 * \param lagIntf
 * \param portIntf
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagIngressEnablePort(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                                  xpsInterfaceId_t portIntf);

/**
 * \public
 * \brief API to set a Lag member's port ingress operational
 *        state to disabled
 *
 * This API will configure a member port in a LAG to
 * operationally on. This means that this port's Link is up and
 * is eligible to participate in a LAG. This concept is
 * different from a LAG membership as a whole as a port must be
 * a member to have a its operational state changed
 *
 * This can be used when the NOS detects a link down event for a
 * LAG member. It will allow for a quick rebalancing of the
 * LAG with minimal impact to traffic as well as allowing this
 * port to operate as a non member of the LAG in the case of
 * Link Down or specific configuration
 *
 * \param devId
 * \param lagIntf
 * \param portIntf
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagIngressDisablePort(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                                   xpsInterfaceId_t portIntf);

/**
 * \public
 * \brief API to retrieve a Lag member's port egress operational
 *        state
 *
 * This API will return a Lag member's port egress operational
 * state if the member configuration has been applied to
 * hardware
 *
 * \param devId
 * \param lagIntf
 * \param portIntf
 * \param state
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetEgressOperationalState(xpsDevice_t devId,
                                          xpsInterfaceId_t lagIntf, xpsInterfaceId_t portIntf, uint32_t *state);

/**
 * \public
 * \brief API to set a Lag member's port egress operational
 *        state to enabled
 *
 * This API will configure a member port in a LAG to
 * operationally on. This means that this port's Link is up and
 * is eligible to participate in a LAG. This concept is
 * different from a LAG membership as a whole as a port must be
 * a member to have a its operational state changed
 *
 * This can be used when the NOS detects a link up event for a
 * LAG member. It will allow for a quick rebalancing of the
 * LAG with minimal impact to traffic
 *
 * \param devId
 * \param lagIntf
 * \param portIntf
 * \param autoEnable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagEgressEnablePort(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                                 xpsInterfaceId_t portIntf, xpsLagAutoDistributeControl_e autoEnable);

/**
 * \public
 * \brief API to set a Lag member's port egress operational
 *        state to disabled
 *
 * This API will configure a member port in a LAG to
 * operationally on. This means that this port's Link is up and
 * is eligible to participate in a LAG. This concept is
 * different from a LAG membership as a whole as a port must be
 * a member to have a its operational state changed
 *
 * This can be used when the NOS detects a link down event for a
 * LAG member. It will allow for a quick rebalancing of the
 * LAG with minimal impact to traffic as well as allowing this
 * port to operate as a non member of the LAG in the case of
 * Link Down or specific configuration
 *
 * \param devId
 * \param lagIntf
 * \param portIntf
 * \param autoEnable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagEgressDisablePort(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                                  xpsInterfaceId_t portIntf, xpsLagAutoDistributeControl_e autoEnable);

/**
 * \brief API to commit changes made to a Ingress Lag configuration to
 *        Hardware
 *
 * This API is like a "repository commit". In essence, it's job
 * is to perform all of the hardware writes on a per device
 * basis based off of changes made to a Ingress Lag Configuration
 *
 * These changes include the following:
 *  - Creating a new Lag
 *  - Adding Ports to a Ingress Lag
 *  - Removing Ports from a Ingress Lag
 *
 * Lag destruction is a multi step process, and xpsLagDeploy
 * plays a role by virtue of performing the writes to the
 * hardware post removal of ports of a Lag
 *
 * \param [in] devId
 * \param [in] lagIntf
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagDeployIngress(xpsDevice_t devId, xpsInterfaceId_t lagIntf);

/**
 * \brief API to commit changes made to a Egress Lag configuration to
 *        Hardware
 *
 * This API is like a "repository commit". In essence, it's job
 * is to perform all of the hardware writes on a per device
 * basis based off of changes made to a Egress Lag Configuration
 *
 * These changes include the following:
 *  - Creating a new Lag
 *  - Adding Ports to a Egress Lag
 *  - Removing Ports from a Egress Lag
 *
 * Lag destruction is a multi step process, and xpsLagDeploy
 * plays a role by virtue of performing the writes to the
 * hardware post removal of ports of a Lag
 *
 * \param [in] devId
 * \param [in] lagIntf
 * \param [in] autoEnable
 * \param [in] isRedirected
 * \return XP_STATUS
 */
XP_STATUS xpsLagDeployEgress(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                             xpsLagAutoDistributeControl_e autoEnable, bool isRedirected);
/**
 * \brief API to read the entire Lag Distriubtion Table
 *
 * For some use cases, auto distribution of Lag ports may not be
 * desired. For finer grained control over distribution of Lag
 * ports to account for weights and such. This API can be used
 * to fetch the entire table into a single buffer
 *
 * This buffer can be manipulated separately and rewritten back
 * to hardware. APIs to manipulate this buffer are also provided
 *
 * \param [in] devId
 * \param [out] tableBuffer
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagReadDistributionTable(xpsDevice_t devId, uint8_t *tableBuffer);

/**
 * \brief API to write the entire Lag Distribution Table
 *
 * For some use cases, auto distribution of Lag ports may not be
 * desired. For finer grained control over distribution of Lag
 * ports to account for weights and such. This API can be used
 * to fetch the entire table into a single buffer
 *
 * \param [in] devId
 * \param [in] tableBuffer
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagWriteDistributionTable(xpsDevice_t devId, uint8_t *tableBuffer);

/**
 * \brief API that will add a port to a hash (index) of the Lag
 *        Distribution Table
 *
 * Each entry in the Lag Distribution Table represents a Port
 * Mask of all the ports on a particular device. This table is
 * populated in such a way as to attempt to evenly distribute
 * traffic across ports in a Lag. Each entry in the Lag
 * Distribution Table is indexed by the Lag Packet Hash.
 *
 * This Port Mask is then applied to the Lag Ports contained in
 * the Lag Vif. The resulting Port Mask will contain a single
 * Lag Port where this particular flow will egress out of.
 *
 * This API performs a Read-Modify-Write for a specified 8-bit
 * hash value and turn bits corresponding to the ports in the
 * port list on for a specified device.
 *
 * \param [in] devId
 * \param [in] idx
 * \param [in] portList
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagAddPortToDistributionEntry(xpsDevice_t devId, uint32_t idx,
                                           xpsPortList_t *portList);

/**
 * \brief API that will remove a port from a hash (index) of the
 *        Lag Distribution Table
 *
 * Each entry in the Lag Distribution Table represents a Port
 * Mask of all the ports on a particular device. This table is
 * populated in such a way as to attempt to evenly distribute
 * traffic across ports in a Lag. Each entry in the Lag
 * Distribution Table is indexed by the Lag Packet Hash.
 *
 * This Port Mask is then applied to the Lag Ports contained in
 * the Lag Vif. The resulting Port Mask will contain a single
 * Lag Port where this particular flow will egress out of.
 *
 * This API performs a Read-Modify-Write for a specified 8-bit
 * hash value and turn bits corresponding to the ports in the
 * port list off for a specified device.
 *
 * \param [in] devId
 * \param [in] idx
 * \param [in] portList
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagRemovePortFromDistributionEntry(xpsDevice_t devId, uint32_t idx,
                                                xpsPortList_t *portList);

/**
 * \brief API to read the entry for a specified 8-bit hash value
 *        from Lag Distriubtion Table
 *
 * For some use cases, auto distribution of Lag ports may not be
 * desired. For finer grained control over distribution of Lag
 * ports to account for weights and such. This API can be used
 * to fetch the entry for a specified 8-bit hash value
 *
 * This entry can be manipulated separately and rewritten back
 * to hardware. APIs to manipulate this entry are also provided
 *
 * \see xpsLagWriteDistributionTableEntry
 *
 * \param [in] devId
 * \param [in] idx
 * \param [out] entry
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagReadDistributionTableEntry(xpsDevice_t devId, uint32_t idx,
                                           xpLagDistributionEntry_t *entry);

/**
 * \brief API to write the entry for a specified 8-bit hash value
 *        into Lag Distribution Table
 *
 * For some use cases, auto distribution of Lag ports may not be
 * desired. For finer grained control over distribution of Lag
 * ports to account for weights and such. This API can be used
 * to update the entry for a specified 8-bit hash value in hardware
 *
 * \see xpsLagReadDistributionTableEntry
 *
 * \param [in] devId
 * \param [in] idx
 * \param [in] entry
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagWriteDistributionTableEntry(xpsDevice_t devId, uint32_t idx,
                                            xpLagDistributionEntry_t *entry);

/**
 * \brief API that will retrieve the width in bits of the Lag
 *        Distribution Table for a given device
 *
 * \param [in]  devId
 * \param [out] entryWidthBits
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetDistributionTableEntryWidth(xpDevice_t devId,
                                               uint32_t* entryWidthBits);

/**
 * \brief API that will retrieve the depth of the Lag
 *        Distributino table for a given device
 *
 * \param [in]  devId
 * \param [out] tableDepth
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetDistributionTableDepth(xpDevice_t devId,
                                          uint32_t* tableDepth);

/**
 * \brief API to check if a device/port combination is part of
 *        this Lag interface
 *
 * This API will check to see if a device and port combination
 * participates in the passed in Lag interface
 *
 * \param [in] devId
 * \param [in] port
 * \param [in] lagIntf
 * \param [out] isMember
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagIsPortMember(xpsDevice_t devId, xpPort_t port,
                             xpsInterfaceId_t lagIntf, uint32_t *isMember);

/**
 * \public
 * \brief API to check if a port interface is a part of this Lag
 *        interface for a scope
 *
 * This API will check to see if a port interface participates
 * in the passed in Lag interface
 *
 * \param [in] scopeId
 * \param [in] portIntf
 * \param [in] lagIntf
 * \param [out] isMember
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagIsPortIntfMemberScope(xpsScope_t scopeId,
                                      xpsInterfaceId_t portIntf, xpsInterfaceId_t lagIntf, uint32_t *isMember);

/**
 * \public
 * \deprecated use \ref xpsGlobalSwitchControlSetLagHashPolynomial
 * \brief This API Sets the lag hash polynomial
 *
 * \param [in] deviceId
 * \param [in] instance
 * \param [in] polynomialId
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsLagSetLagHashPolynomial(xpsDevice_t deviceId, uint32_t instance,
                                     uint32_t polynomialId);
/**
 * \public
 * \deprecated use \ref xpsGlobalSwitchControlGetLagHashPolynomial
 * \brief Gets the LAG hash polynomial for this device and
 *           instance
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which LAG hash to select
 * \param [out] polynomialId polynomial number
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsLagGetLagHashPolynomial(xpsDevice_t deviceId, uint32_t instance,
                                     uint32_t* polynomialId);

/**
 * \public
 * \deprecated use \ref xpsGlobalSwitchControlSetLagHashPolynomialSeed
 * \brief This API Sets the lag hash polynomial seed
 *
 * \param [in] deviceId   Device id
 * \param [in] instance   which LAG hash to select
 * \param [in] seed
 *
 * \return XP_STATUS XP_NO_ERR if there is no error
 */
XP_STATUS xpsLagSetLagHashPolynomialSeed(xpsDevice_t deviceId,
                                         uint32_t instance, uint32_t seed);

/**
 * \public
 * \deprecated use \ref xpsGlobalSwitchControlGetLagHashPolynomialSeed
 * \brief This API Gets the lag hash polynomial seed
 *
 * \param [in] deviceId
 * \param [in] instance
 * \param [out] seed
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetLagHashPolynomialSeed(xpsDevice_t deviceId,
                                         uint32_t instance, uint32_t* seed);

/**
 * \public
 * \brief This API Enables Mirroring on a lag
 *
 * \param [in] devId
 * \param [in] lagIntf
 * \param [in] analyzerId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagEnableMirroring(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                                uint32_t analyzerId);

/**
 * \public
 * \brief This API Disable Mirorring on a lag
 *
 * \param [in] devId
 * \param [in] lagIntf
 * \param [in] analyzerId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagDisableMirroring(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                                 uint32_t analyzerId);

/**
 * \brief Pick the first LAG interface id available in the system for a scope
 *
 * \param [in] scopeId
 * \param [out] lagIntf
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetFirstScope(xpsScope_t scopeId, xpsInterfaceId_t *lagIntf);

/**
 * \brief Get the next LAG interface id available in the system for a scope
 *
 * \param [in] scopeId
 * \param [in] lagIntf
 * \param [out] lagIntfNext
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetNextScope(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                             xpsInterfaceId_t *lagIntfNext);

/**
 * \brief API that initializes the XPS Lag Mgr
 *
 * This API will register for the state databases that are
 * needed by the Lag Mgr code base
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagInit(void);

/**
 * \brief API to De-Init the XPS Lag Mgr
 *
 * This API will Deregister all state databases for Lag Mgr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagDeInit(void);

/**
 * \brief Get ports from lag interface available in the system for a scope
 *
 * \param [in] scopeId
 * \param [in] lagIntf
 * \param [out] portIntfList
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetIngressPortIntfList(xpsScope_t scopeId,
                                       xpsInterfaceId_t lagIntf, xpsLagPortIntfList_t *portIntfList);

/**
 * \brief API to get list of port interfaces in the egress LAG for a scope
 *
 * This API retrieves the ports form an egress LAG interface.
 * Pass scopeId as XP_SCOPE_DEFAULT, if scope is not required.
 *
 * \param [in] lagIntf
 * \param [out] portIntfList
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetEgressPortIntfList(xpsScope_t scopeId,
                                      xpsInterfaceId_t lagIntf, xpsLagPortIntfList_t *portIntfList);

/**
 * \brief API to set Hash Algo for LAG
 *
 * This API sets hash algorithm for LAG on given device ID.
 *
 * \param [in] devId
 * \param [in] xpsHashAlgo
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagSetHashAlgo(xpsDevice_t devId,
                            xpsLagHashAlgoType_e xpsHashAlgo, GT_U32 hashSeed);

/**
 * \brief API to get Hash Algo for LAG
 *
 * This API gets hash algorithm for LAG on given device ID.
 *
 * \param [in] devId
 * \param [out] xpsHashAlgo
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetHashAlgo(xpsDevice_t devId,
                            xpsLagHashAlgoType_e *xpsHashAlgo);

/**
 * \brief API to set Hash BIT for LAG
 *
 * This API sets hash algorithm for LAG on given device ID.
 *
 * \param [in] devId
 * \param [in] xpsHashAlgo
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagSetHashBit(xpsDevice_t devId,
                           xpsLagHashAlgoType_e xpsHashAlgo, bool isLag);

/**
 * \brief API to setDefault Hash Params for LAG
 *
 * This API gets hash algorithm for LAG on given device ID.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagSetHashDefaults(xpsDevice_t devId);

/**
 * \brief API to set Hash seed for LAG
 *
 * This API sets hash algorithm for LAG on given device ID.
 *
 * \param [in] devId
 * \param [in] hashSeed
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagSetHashSeed(xpsDevice_t devId, uint32_t hashSeed);

/**
 * \brief API to get Hash seed for LAG
 *
 * This API gets hash algorithm for LAG on given device ID.
 *
 * \param [in] devId
 * \param [out] hashSeed
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetHashSeed(xpsDevice_t devId, uint32_t *hashSeed);

XP_STATUS xpsLagGetLagIdCount(uint32_t *count);

XP_STATUS xpsLagGetLagIdList(xpsScope_t scopeId, xpsInterfaceId_t* lagId);

XP_STATUS xpsLagIpV4RouteEnable(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                                bool isMcast, uint32_t enable);

XP_STATUS xpsLagIpV4RouteEnableGet(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                                   bool isMcast, uint32_t *enable);

XP_STATUS xpsLagIpV6RouteEnable(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                                bool isMcast, uint32_t enable);

XP_STATUS xpsLagIpV6RouteEnableGet(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                                   bool isMcast, uint32_t *enable);

XP_STATUS xpsLagMemberIngressEnablePort(xpsDevice_t devId,
                                        xpsInterfaceId_t lagIntf,
                                        xpsInterfaceId_t portIntf, bool enable);

XP_STATUS xpsLagMemberEgressEnablePort(xpsDevice_t devId,
                                       xpsInterfaceId_t lagIntf,
                                       xpsInterfaceId_t portIntf, bool enable);

XP_STATUS xpsLagSetMirrorEnable(xpsInterfaceId_t lagIntf,
                                bool enable, uint32_t analyzerId);

XP_STATUS xpsLagGetMirrorEnable(xpsInterfaceId_t lagIntf,
                                bool *enable, uint32_t *analyzerId);

bool xpsLagRandomEnableGet(void);
void xpsLagRandomEnableSet(bool enable);
XP_STATUS xpsLagRandomEnableRewrite(xpsDevice_t devId, bool enable);
XP_STATUS xpsLagSetRandomHashSeed(xpsDevice_t devId, uint32_t hashSeed);

#ifdef __cplusplus
}
#endif

#endif  //_xpsLag_h_

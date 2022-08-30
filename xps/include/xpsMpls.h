// xpsMpls.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpsMpls.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Mpls Manager
 */

#ifndef _xpsMpls_h_
#define _xpsMpls_h_

#include "xpsState.h"
#include "xpsVlan.h"
#include "xpsL3.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XP_MPLS_VPN_SET_PKT_CMD 1
#define XP_MPLS_VPN_SET_BACL_ID 2
#define XP_MPLS_VPN_SET_RACL_ID 4
#define XP_MPLS_VPN_SET_PACL_ID 8

/**
 * \brief This type defines a structure representing a
 *        MPLS NH Entry and MPLS Encap Type for MPLS
 *        Transit Label Entry.
 */

typedef enum xpMplsNhEncapType
{
    XP_MPLS_NH_OVER_UNTAGGED_PORT = 0,
    XP_MPLS_NH_OVER_TAGGED_PORT,
    XP_MPLS_NH_OVER_Q_IN_Q_PORT,
    XP_MPLS_NH_OVER_MPLS_TUNNEL,
    XP_MPLS_NH_INVALID
} xpMplsNhEncapType;


/**
 * \brief xpMplsLabelOper
 */
typedef enum xpMplsLabelOper
{
    XP_MPLS_OPER_SWAP = 0,
    XP_MPLS_OPER_PHP,
    XP_MPLS_OPER_INVALID
} xpMplsLabelOper;

/**************************************** MPLS Tunnel ***************************************************/
typedef struct xpsMplsTunnelKey_t
{
    uint8_t         isP2MP;
    uint8_t         numOfLables;
    uint32_t        firstLabel;
    uint32_t        secondLabel;
} xpsMplsTunnelKey_t;

typedef struct xpsMplsUhpLabelData_t
{
    uint8_t     propTTL;
    uint8_t     countMode;
    uint32_t    cntId;
} xpsMplsUhpLabelData_t;

typedef struct xpsMplsMpTunnelData_t
{
    uint8_t     propTTL;
    uint8_t     countMode;
    uint32_t    cntId;
    uint8_t     isBudNode;
} xpsMplsMpTunnelData_t;

typedef union xpsMplsTunnelParams_t
{
    xpsMplsUhpLabelData_t   p2pLabelTnl;
    xpsMplsMpTunnelData_t   p2mpLabelTnl;
} xpsMplsTunnelParams_t;

/**************************************** MPLS VPN ***************************************************/
typedef struct xpsMplsVpnParams_t
{
    uint8_t flags;
    xpPktCmd_e pktCmd;
    uint8_t countMode;
    uint32_t cntId;
    uint32_t paclId;
    uint32_t raclId;
} xpsMplsVpnParams_t;

/**************************************** MPLS GRE VPN ***************************************************/
typedef struct xpsMplsVpnGreLooseModeParams_t
{
    xpsCountMode_e countMode;
    uint32_t counterId;
    uint32_t propTTL;
    uint16_t bdId;
} xpsMplsVpnGreLooseModeParams_t;

/**************************************** MPLS Label Entry *********************************************/
typedef struct xpsMplsLabelEntry_t
{
    uint32_t             keyLabel;
    xpsPktCmd_e          pktCmd;
    uint32_t             mirrorMask;
    xpsCountMode_e       countMode;
    uint32_t             counterId;
    uint32_t             propTTL;
    uint32_t             swapLabel;
    xpMplsLabelOper      mplsOper;
    xpsL3NextHopData_t   nextHopData;
    //ecmpsize?
} xpsMplsLabelEntry_t;

/********************************************* Init ***************************************************/

/**
 * \brief This method initializes xps MPLS manager with system specific
 *        references to various primitive managers and their initialization
 *        for a scope.
 *        This method also registers all necessary databases required
 *        maintain the relevant states with the state manager.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMplsInitScope(xpsScope_t scopeId);

/**
 * \brief This method uninitializes xps MPLS manager and frees system specific
 *        references to various primitive managers and their initialization
 *        for a scope.
 *        This method also deregisters all necessary databases required
 *        maintain the relevant states with the state manager.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMplsDeInitScope(xpsScope_t scopeId);

/**
 * \brief This method adds device specific primitives for this feature.
 *          This method also initializes all necessary device specific
 *          databases required maintain the relevant states.
 *
 * \param [in] devId    Device Id of device
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMplsAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief This method removes all device specific primitives for this feature.
 *          This method also frees all used device specific databases by
 *          this manager.
 *
 * \param [in] devId    Device Id of device
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMplsRemoveDevice(xpsDevice_t devId);

/**************************************** MPLS Tunnel ***************************************************/
/**
 * \brief This method creates a MPLS tunnel interface for a scope.
 *
 * \param [in] scopeId      Scope Id for scope
 * \param [out] mplsTnlId   MPLS tunnel interface Id
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsCreateTunnelInterfaceScope(xpsScope_t scopeId,
                                            xpsInterfaceId_t *mplsTnlId);

/**
 * \brief This method adds a MPLS tunnel origination entry.
 *
 * \param [in] devId        Device Id of device
 * \param [in] mplsTnlKey   Key for the MPLS tunnel to be added
 * \param [in] mplsTnlId    MPLS tunnel interface Id
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsAddTunnelOriginationEntry(xpsDevice_t devId,
                                           xpsMplsTunnelKey_t *mplsTnlKey, xpsInterfaceId_t mplsTnlId);

/**
 * \brief This method adds a MPLS tunnel termination entry.
 *
 * \param [in] devId        Device Id of device
 * \param [in] mplsTnlKey   Key for the MPLS tunnel to be added
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsAddTunnelTerminationEntry(xpsDevice_t devId,
                                           xpsMplsTunnelKey_t *mplsTnlKey);

/**
 * \brief This method removes a MPLS tunnel origination entry.
 *
 * \param [in] devId        Device Id of device
 * \param [in] mplsTnlId    MPLS tunnel interface Id
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsRemoveTunnelOriginationEntry(xpsDevice_t devId,
                                              xpsInterfaceId_t mplsTnlId);

/**
 * \brief This method removes a MPLS tunnel termination entry.
 *
 * \param [in] devId        Device Id of device
 * \param [in] mplsTnlKey   Key for the MPLS tunnel to be removed
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsRemoveTunnelTerminationEntry(xpsDevice_t devId,
                                              xpsMplsTunnelKey_t *mplsTnlKey);

/**
 * \brief This method destroys a MPLS tunnel interface for a scope.
 *
 * \param [in] scopeId      Scope Id for scope
 * \param [in] mplsTnlId    MPLS tunnel interface Id
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                             xpsInterfaceId_t mplsTnlId);

/**
 * \brief This method sets the parameters for the MPLS tunnel termination entry.
 *
 *
 * \param [in] devId        Device Id of device
 * \param [in] mplsTnlKey   Key for the MPLS tunnel
 * \param [in] tunnelParams Contains MPLS tunnel parameters
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsSetTunnelConfig(xpsDevice_t devId,
                                 xpsMplsTunnelKey_t *mplsTnlKey, xpsMplsTunnelParams_t *tunnelParams);

/**
 * \brief This method gets the parameters for the MPLS tunnel termination entry.
 *
 *
 * \param [in] devId        Device Id of device
 * \param [in] mplsTnlKey   Key for the MPLS tunnel
 * \param [out] tunnelParams    Retrieves MPLS tunnel parameters
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsGetTunnelConfig(xpsDevice_t devId,
                                 xpsMplsTunnelKey_t *mplsTnlKey, xpsMplsTunnelParams_t *tunnelParams);

/**
 * \brief This method sets the next hop for the MPLS tunnel.
 *
 *
 * \param [in] devId        Device Id of device
 * \param [in] mplsTnlId    MPLS tunnel interface Id
 * \param [in] nextHopId    Id of the Next hop to be set
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsSetTunnelNextHopData(xpsDevice_t devId,
                                      xpsInterfaceId_t mplsTnlId, uint32_t nextHopId);

/**
 * \brief Updates the nexthop information to the tunnel entry
 *
 * This Api updates the tunnel entry with the latest data programmed on  nexthop of the tunnel.
 * This Api needs to be called after the nexthop is updated with newer data using L3 apis.
 *
 * \param [in] devId        Device Id of device
 * \param [in] mplsTnlId    MPLS tunnel interface Id
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsUpdateTunnelNextHopData(xpsDevice_t devId,
                                         xpsInterfaceId_t mplsTnlId);


/**************************************** MPLS VPN ***************************************************/

/**
 * \brief Sets the configuration parameters for a MPLS VPN on a device.
 *
 * \param [in] devId        Device Id of device
 * \param [in] vpnLabel     VPN label
 * \param [in] vpnParams    Contains VPN parameters
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsSetVpnConfig(xpsDevice_t devId, uint32_t vpnLabel,
                              xpsMplsVpnParams_t *vpnParams);

/**
 * \brief Gets the configuration parameters for a MPLS VPN on a device.
 *
 * \param [in]  devId       Device Id of device
 * \param [in]  vpnLabel    VPN label
 * \param [out] vpnParams   Retrieves VPN parameters
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsGetVpnConfig(xpsDevice_t devId, uint32_t vpnLabel,
                              xpsMplsVpnParams_t *vpnParams);


/**************************************** MPLS GRE VPN ***************************************************/

/**
 * \brief This method adds a MPLS GRE VPN entry on a device
 *
 * \param [in] devId        Device Id of device
 * \param [in] vpnLabel     GRE VPN label
 * \param [in] vpnParams    Contains GRE VPN parameters
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMplsAddVpnGreLooseModeEntry(xpsDevice_t devId, uint32_t vpnLabel,
                                         xpsMplsVpnGreLooseModeParams_t *vpnParams);

/**
 * \brief This method removes a MPLS GRE VPN entry on a device
 *
 * \param [in] devId        Device Id of device
 * \param [in] vpnLabel     GRE VPN label
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMplsRemoveVpnGreLooseModeEntry(xpsDevice_t devId,
                                            uint32_t vpnLabel);

/**
 * \brief Sets the configuration parameters for a MPLS GRE VPN on a device.
 *
 * \param [in] devId        Device Id of device
 * \param [in] vpnLabel     GRE VPN label
 * \param [in] vpnParams    Contains GRE VPN parameters
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsSetVpnGreLooseModeConfig(xpsDevice_t devId, uint32_t vpnLabel,
                                          xpsMplsVpnGreLooseModeParams_t *vpnParams);

/**
 * \brief Gets the configuration parameters for a MPLS GRE VPN on a device.
 *
 * \param [in]  devId       Device Id of device
 * \param [in]  vpnLabel    GRE VPN label
 * \param [out] vpnParams   Retrieves GRE VPN parameters
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsGetVpnGreLooseModeConfig(xpsDevice_t devId, uint32_t vpnLabel,
                                          xpsMplsVpnGreLooseModeParams_t *vpnParams);


/**************************************** MPLS Label Entry *********************************************/

/**
 * \brief This method adds a MPLS label entry on a device
 *
 * \param [in] devId            Device Id of device
 * \param [in] mplsLblEntry     Contains MPLS label entry
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsAddLabelEntry(xpsDevice_t devId,
                               xpsMplsLabelEntry_t *mplsLblEntry);

/**
 * \brief This method removes a MPLS label entry from a given device
 *
 * \param [in] devId        Device Id of device
 * \param [in] mplsLabel    MPLS label
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsRemoveLabelEntry(xpsDevice_t devId, uint32_t mplsLabel);

/**
 * \brief This method gets a MPLS label entry on the device
 *
 * \param [in] devId            Device Id of device
 * \param [in] mplsLabel        MPLS label
 * \param [out] mplsLblEntry    Retrieves MPLS label entry
 *
 * \return XP_STATUS Operation status
 *                          - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsGetLabelEntry(xpsDevice_t devId, uint32_t mplsLabel,
                               xpsMplsLabelEntry_t *mplsLblEntry);

/********************************************* Init ***************************************************/

/**
 * \brief This method initializes xps MPLS manager with system specific
 *        references to various primitive managers and their initialization.
 *        This method also registers all necessary databases required
 *        maintain the relevant states with the state manager.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMplsInit(void);

/**
 * \brief This method uninitializes xps MPLS manager and frees system specific
 *        references to various primitive managers and their initialization.
 *        This method also deregisters all necessary databases required
 *        maintain the relevant states with the state manager.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMplsDeInit(void);

/**************************************** MPLS Tunnel ***************************************************/
/**
 * \brief This method creates a MPLS tunnel interface.
 *
 * \param [out] mplsTnlId       MPLS tunnel interface Id
 *
 * \return XP_STATUS Operation status
 *                                                      - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsCreateTunnelInterface(xpsInterfaceId_t *mplsTnlId);

/**
 * \brief This method destroys a MPLS tunnel interface.
 *
 * \param [in] mplsTnlId        MPLS tunnel interface Id
 *
 * \return XP_STATUS Operation status
 *                                                      - XP_NO_ERR: Successful execution
 *                                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsMplsDestroyTunnelInterface(xpsInterfaceId_t mplsTnlId);

/**
 * \brief This method sets rehash level for MPLS Label table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMplsSetLabelEntryRehashLevel(xpDevice_t devId,
                                          uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for MPLS Label table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMplsGetLabelEntryRehashLevel(xpDevice_t devId,
                                          uint8_t* numOfRehashLevels);

#ifdef __cplusplus
}
#endif

#endif  //_xpsMpls_h_

// xpsStp.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsStp.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Stp Manager
 */

#ifndef _xpsStp_h_
#define _xpsStp_h_

#include "xpTypes.h"
#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsStp.h"

#ifdef __cplusplus
extern "C" {
#endif


#define XPS_STP_GET_STGINTFSTATE_DBKEY(intfId,stgId)  (((uint64_t)intfId) << 8 | (stgId))
#define XPS_STP_GET_STPID_FROM_DBKEY(stpIntfDbKey)  ((stpIntfDbKey) & 0xFF)
#define XPS_STP_GET_INTFID_FROM_DBKEY(stpIntfDbKey)  ((uint32_t)((stpIntfDbKey) >> 8))

typedef enum
{
    XPS_STP_MODE_VLAN_E,
    XPS_STP_MODE_PORT_E
} xpsStpMode_e;

typedef enum xpsStgStaticDataType_e
{
    XPS_STG_STATIC_VARIABLES,
} xpsStgStaticDataType_e;


//Holds the system defaultStg value.
typedef struct xpsStgStaticDbEntry
{
    //Key
    xpsStgStaticDataType_e staticDataType;

    //Data
    xpsStp_t    defaultStg;
} xpsStgStaticDbEntry;

typedef struct xpsStgIntfStateDbEntry
{
    //Key
    uint64_t stgIntfDbKey;

    //Data
    xpsStpState_e state;
} xpsStgIntfStateDbEntry;


/**
 * \brief This method initializes xps stp manager with system specific
 *        references to various primitive managers and their initialization.
 *        This method also registers all necessary databases required
 *        maintain the relevant states with the state manager per scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpInitScope(xpsScope_t scopeId);

/**
 * \brief This method uninitializes xps stp manager and frees system specific
 *        references to various primitive managers and their initialization.
 *        This method also deregisters all necessary databases required
 *        maintain the relevant states with the state manager per scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpDeInitScope(xpsScope_t scopeId);

/**
 * \brief This method adds device specific primitives for this feature.
 *          This method also initializes all necessary device specific
 *          databases required maintain the relevant states.
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief This method removes all device specific primitives for this feature.
 *          This method also frees all used device specific databases by
 *          this manager.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpRemoveDevice(xpsDevice_t devId);

/**
 * \brief This method sets a stpId as the defualt stp for
 *          the device.
 *
 * \param [in] devId
 * \param [in] stpId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpSetDefault(xpsDevice_t devId, xpsStp_t stpId);

/**
 * \brief This method configures spanning tree in a  scope and returns the
 *          returns the id to the user.
 *
 * \param [in] scopeId
 * \param [out] *stpId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpCreateScope(xpsScope_t scopeId, xpsStp_t *stpId);


/**
 * \brief This method configures spanning tree in a  scope and returns the
 *          returns the id to the user. Additionally it sets a default state for all
 *          ports provided by user.
 *
 * \param [in] scopeId
 * \param [in] stpDefState
 * \param [out] *stpId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpCreateWithState(xpsScope_t scopeId, xpsStpState_e stpDefState,
                                xpsStp_t *stpId);

/**
 * \brief This method configures a spanning tree with a given id for a scope.
 *
 * \param [in] scopeId
 * \param [in] stpId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpReserveScope(xpsScope_t scopeId, xpsStp_t stpId);

/**
 * \brief This method destroys a spanning tree in a scope.
 *
 * \param [in] scopeId
 * \param [in] stpId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpDestroyScope(xpsScope_t scopeId, xpsStp_t stpId);

/**
 * \brief This method clears the software states used for the given stp and interface
 *        and resets the stp state to default
 *
 * \param [in] devId
 * \param [in] stpId
 * \param [in] intfId
 * \param [in] defaultStpState
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpResetState(xpsDevice_t devId, xpsStp_t stpId,
                           xpsInterfaceId_t intfId, xpsStpState_e defaultStpState);

/**
 * \brief This method returns the reference to a vlan list and
 *          also returns the number of vlans subscribed to the stg.
 *
 * \param [in] devId
 * \param [in] stpId
 * \param [out] **vlanList
 * \param [out] *numOfvlans
 *
 * \return XP_STATUS
 */

XP_STATUS xpsStgGetVlanList(xpsDevice_t devId, xpsStp_t stpId,
                            xpsVlan_t **vlanList, uint16_t *numOfVlans);

/**
 * \brief This method returns the reference to a interface list and
 *          also returns the number of interfaces part of the stp.
 *
 * \param [in] devId
 * \param [in] stpId
 * \param [out] **interfaceList
 * \param [out] *numOfIntfs
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpGetInterfaceList(xpsDevice_t devId, xpsStp_t stpId,
                                 xpsInterfaceId_t **interfaceList, uint16_t *numOfIntfs);

/**
 * \brief This method initializes xps stp manager with system specific
 *        references to various primitive managers and their initialization.
 *        This method also registers all necessary databases required
 *        maintain the relevant states with the state manager.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpInit(void);

/**
 * \brief This method uninitializes xps stp manager and frees system specific
 *        references to various primitive managers and their initialization.
 *        This method also deregisters all necessary databases required
 *        maintain the relevant states with the state manager.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpDeInit(void);

/**
 * \brief This method configures a spanning tree with a given id.
 *
 * \param [in] stpId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpReserve(xpsStp_t stpId);

XP_STATUS xpsStgGetStgCount(xpsScope_t scopeId, uint32_t *count);

XP_STATUS xpsStpGetStpIdList(xpsScope_t scopeId, xpsStp_t *stpId);

/**
 * \brief This method verifies whether spanning tree with a
 *        given id exists on device.
 *
 * \param [in] devId
 * \param [in] stpId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpIsExistOnDevice(xpsDevice_t devId, xpsStp_t stpId);

XP_STATUS xpsGetPortStpId(xpsDevice_t devId, xpsInterfaceId_t intfId,
                          xpsStp_t *stpId);
XP_STATUS xpsValidatePortStpId(xpsDevice_t devId, xpsInterfaceId_t intfId,
                               xpsStp_t stpId);

XP_STATUS xpsStgGetStaticVariablesDb(xpsScope_t scopeId,
                                     xpsStgStaticDbEntry ** staticVarDbPtr);
XP_STATUS xpsStgGetStgIntfEntryDb(xpsScope_t scopeId, xpsStp_t stgId,
                                  xpsInterfaceId_t intfId, xpsStgIntfStateDbEntry **stgIntfStatePtr);


#ifdef __cplusplus
}
#endif

#endif  //_xpsStp_h_


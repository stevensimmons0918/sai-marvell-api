// xpsMtuProfile.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpsMtuProfile.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Mtu profile manager Manager
 */

#ifndef _xpsMtuProfile_h_
#define _xpsMtuProfile_h_

#include "xpsInterface.h"
#include "xpsState.h"
//#include "xpMtuProfileMgr.h"
#include "xpsPort.h"
#ifdef __cplusplus
extern "C" {
#endif

//TODO_RPR
#define UMAC_MTU_FRAME_SIZE       1514       /// Default mtu frame size
/**
* \enum xpsMtuProfileIndex_e
* \
* \brief This type defines an enumeration of possible MTU profile indexes
*/
typedef enum
{
    MTU_PROFILE_INDEX0,
    MTU_PROFILE_INDEX1,
    MTU_PROFILE_INDEX2,
    MTU_PROFILE_INDEX3,
    MTU_PROFILE_INDEX_MAX_VAL
} xpsMtuProfileIndex_e;

/**
* \enum xpsMtuL3ProfileIndex_e
* \
* \brief This type defines an enumeration of possible MTU L3
*        profile indexes
*/
typedef enum
{
    MTU_L3_PROFILE_INDEX0,
    MTU_L3_PROFILE_INDEX1,
    MTU_L3_PROFILE_INDEX2,
    MTU_L3_PROFILE_INDEX3,
    MTU_L3_PROFILE_INDEX4,
    MTU_L3_PROFILE_INDEX5,
    MTU_L3_PROFILE_INDEX6,
    MTU_L3_PROFILE_INDEX7,
    MTU_L3_PROFILE_INDEX_MAX_VAL
} xpsMtuL3ProfileIndex_e;

/**
 * \struct xpsMtuMap_t
 * \brief This structure maintains profileIndex and mtuSize mapping
 */
typedef struct xpsMtuMap
{
    uint8_t profileIdx;
    uint32_t mtuSize;
    uint32_t refCount;    // Counts the number of ports using a profile
} xpsMtuMap_t;

/**
 * \brief Initilize Mtu profile module for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuInitScope(xpsScope_t scopeId);

/**
 * \brief DeInitilize Mtu profile module for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuDeInitScope(xpsScope_t scopeId);

/**
 * \brief Set mtu default frame size.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSetDefaultMtuFrmSize(xpsDevice_t devId);

/**
 * \brief Add device for Mtu profile module.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuAddDevice(xpsDevice_t devId);

/**
 * \brief Remove device for Mtu profile module.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuRemoveDevice(xpsDevice_t devId);

/**
 * \brief Set Mtu size for interface
 *
 * Set the mtuSize at the profileIndex
 * \param [in] devId
 * \param [in] intfId
 * \param [in] mtuSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuSetInterfaceMtuSize(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                    uint32_t mtuSize);

/**
* @  xpsMtuSetIL3nterfaceMtuSize function
*
* @brief   Set L3 Mtu Size to L3Interface. And it creatres a
*          profile if not exists.
*
* @param[in] devId          - device Id
*
* @param[in] xpsInterfaceId_t      - L3 Interface which will be
*       converted to Port Num or Lag based on type.
* @param[in] mtuSize              - MTU Size
*
*/
XP_STATUS xpsMtuSetIL3nterfaceMtuSize(xpsDevice_t devId,
                                      xpsInterfaceId_t intfL3Id, uint32_t mtuSize);

/**
* @Function  xpsMtuSetDefaultL3InterfaceMtuSize
*
* @brief     Set default L3 Mtu Size to L3Interface.
*
* @param[in] devId     device Id
* @param[in] intfL3Id  L3 Interface id
* @param[in] mtuSize   MTU Size
*/
XP_STATUS xpsMtuSetDefaultL3InterfaceMtuSize(xpsDevice_t devId,
                                             xpsInterfaceId_t intfL3Id);

/**
 * \brief Update Mtu size for interface
 *
 * Set the mtuSize at the profileIndex
 * \param [in] devId
 * \param [in] intfId
 * \param [in] mtuSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuUpdateInterfaceMtuSize(xpsDevice_t devId,
                                       xpsInterfaceId_t intfId, uint32_t mtuSize);

/**
 * \brief Get Mtu size for interface
 *
 * Get the mtuSize at the profileIndex
 * \param [in] devId
 * \param [in] intfId
 * \param [out] *mtuSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuGetInterfaceMtuSize(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                    uint32_t* mtuSize);

/**
 * \brief Set the Mtu Size for a profile index .
 *
 * \param [in] devId
 * \param [in] profileIdx
 * \param [out] mtuSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuGetMtuProfile(xpsDevice_t devId, uint32_t profileIdx,
                              uint32_t* mtuSize);

/**
 * \brief Set the Mtu Size for a profile index .
 *
 * \param [in] devId
 * \param [in] profileIdx
 * \param [out] mtuSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuGetIpMtuProfile(xpsDevice_t devId, uint32_t profileIdx,
                                uint32_t* mtuSize);

/**
 * \brief Displays NH table to MTU profile binding for a given egress port.
 *        Pass egrPortNum as 255 to display all entries.
 *
 * \param [in] devId
 * \param [in] egrPortNum
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuDisplayNhTable(xpsDevice_t devId, uint32_t egrPortNum);

/**
 * \brief Get the Mtu profile index configured on a interface(port,lag or tunnel)
 *
 * Get Mtu profile index on the interface
 * \param [in] devId
 * \param [in] intfId
 * \param [out] mtuProfileIdx
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuGetInterfaceMtuProfileIdx(xpsDevice_t devId,
                                          xpsInterfaceId_t intfId, uint32_t* mtuProfileIdx);

/**
 * \public
 * \brief Enable MTU Error Checking
 *
 * MTU error checking should be disabled when operating in
 * cut-thru switching mode
 *
 * \param [in] devId Device ID
 * \param [in] enable 1 - Enable, 0 - Disable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuEnableMtuCheck(xpsDevice_t devId, uint32_t enable);

/**
 * \public
 * \brief Get whether or not MTU Error checking is enabled
 *
 * MTU Error checking should be disabled when operating in
 * cut-thru switching mode
 *
 * \param [in] devId Device ID
 * \param [out] enable 1 - Enable, 0 - Disable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuIsMtuCheckEnabled(xpsDevice_t devId, uint32_t *enable);

/**
 * \brief Initilize Mtu profile module.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuInit(void);

/**
 * \brief DeInitilize Mtu profile module.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuDeInit(void);
/**
 * \can set pktCmd as DROP or Trap when MTU check fails
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMtuSetMtuErrorPktCmd(xpsDevice_t devId, uint32_t pktCmd);
#ifdef __cplusplus
}
#endif

#endif //_xpsMtuProfile_h_


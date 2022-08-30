// xpsNhGrp.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpsNhGrp.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Next Hop Groups Manager
 */

#ifndef _xpsNhGrp_h_
#define _xpsNhGrp_h_

#include "xpEnums.h"
#include "xpTypes.h"
#include "xpsInterface.h"
#include "xpsL3.h"
#include <assert.h>
/**
 * \brief  Maximum nh in group
 */
extern uint32_t nhGrpMaxSize;

/**
 * \brief State structure maintained by Next Hop Group
 *
 * This state contains the Next Hop Group configuration
 *
 * This state is internal to XPS and is not exposed to the user
 */
typedef struct xpsL3NextHopGroupEntry_t
{
    uint8_t enablePbr;   ///< Group for pbr
    uint32_t keyNhGroupId; ///< Group identifier
    uint32_t baseNhId;   ///< Base NH enry id
    uint32_t groupCount; ///< Number of used next hops
    uint32_t groupSize;  ///< Number of allocated next hops
    uint32_t *nhIdMap;   ///< Source next hops identifiers
    uint32_t nhCount;    ///< Number of unique NextHops in a group
    uint32_t *nextHopIdx;
    uint32_t hwPbrLeafId;
} xpsL3NextHopGroupEntry_t;

/**
 * \brief  Enum used to for static variables DB
 */
typedef enum xpsL3NextHopGroupStaticDataType_e
{
    XPS_L3_NHGRP_STATIC_VARIABLES,
} xpsL3NextHopGroupStaticDataType_e;

/**
 * \brief State structure maintained by Next Hop Group
 *
 * This state contains the Next Hop Group global variables
 *
 * This state is internal to XPS and is not exposed to the user
 */
typedef struct xpsL3NextHopGroupStaticEntry_t
{
    xpsL3NextHopGroupStaticDataType_e keyStaticDataType; ///< Key
    uint32_t groupSize; ///< Current maximum group size
} xpsL3NextHopGroupStaticEntry_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief API that adds a device to XPS NH gorup manager.
 *
 * \param [in] devId Device Id
 * \param [in] initType Init type
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3RouteNextHopGroupAddDevice(xpsDevice_t devId,
                                          xpsInitType_t initType);

/**
 * \brief API that removes a device to XPS NH gorup manager.
 *
 * \param [in] devId Device Id
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3RouteNextHopGroupRemoveDevice(xpsDevice_t devId);

/**
 * \brief API that initializes the XPS Next Hop Group for a scope.
 *
 * API will register for the state databases
 *
 * \param [in] scopeId Scope identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3InitRouteNextHopGroupScope(xpsScope_t scopeId);

/**
 * \brief API that deinitializes the XPS Next Hop Group for a scope.
 *
 * API will deregister from the state databases
 *
 * \param [in] scopeId Scope identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DeInitRouteNextHopGroupScope(xpsScope_t scopeId);

/**
 * \brief API that sets the XPS Next Hop Group max size for a scope.
 *
 * \param [in] scopeId Scope identifier
 * \param [in] size Maximum group size
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetRouteNextHopGlobalMaxSizeScope(xpsScope_t scopeId,
                                                 uint32_t size);

/**
 * \brief Get the Next Hop group global size for a scope
 *
 * \param [in] scopeId Scope identifier
 * \param [out] pGroupSize
 *
 * \return uint32_t
 */
XP_STATUS xpsL3GetRouteNextHopGlobalMaxSizeScope(xpsScope_t scopeId,
                                                 uint32_t* pGroupSize);

/**
 * \brief API to create a new Next Hop Group with an available id
 *
 * \param [in] scopeId Scope identifier
 * \param [out] pNhGrpId Allocated group identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3CreateRouteNextHopGroupScope(xpsScope_t scopeId,
                                            uint32_t *pNhGrpId);

/**
 * \brief API to destroy an existing Next Hop Group
 *
 * \param [in] devId Device identifier
 * \param [in] nhGrpId Group identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DestroyRouteNextHopGroup(xpsDevice_t devId, uint32_t nhGrpId);

/**
 * \brief API to add a new Next Hop to an existing Next Hop Group
 *
 * \param [in] devId Device identifier
 * \param [in] nhGrpId Group identifier
 * \param [in] nhId Next hop identifier
 * \param [in] weight weight of NH in the NH group
 *
 * Weight indicates the probability for the NH to be used in the group during packet flow.
 * Internally the nexthop is copied to group multiple times based on the weight. So,
 * more the weight more resources it takes.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3AddNextHopRouteNextHopGroup(xpsDevice_t devId, uint32_t nhGrpId,
                                           uint32_t nhId, uint32_t weight);

/**
 * \brief API to remove the Next Hop from an existing Next Hop Group
 *
 * \param [in] devId Device identifier
 * \param [in] nhGrpId Group identifier
 * \param [in] nhId Next hop identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3RemoveNextHopRouteNextHopGroup(xpsDevice_t devId,
                                              uint32_t nhGrpId, uint32_t nhId);

/**
 * \brief API to get the weight of nexthop from a nexthop group
 *
 * \param [in] devId Device identifier
 * \param [in] nhGrpId Group identifier
 * \param [in] nhId Next hop identifier
 * \param [out] weight output weight
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetNextHopWeightRouteNextHopGroup(xpsDevice_t devId,
                                                 uint32_t nhGrpId, uint32_t nhId, uint32_t *weight);

/**
 * \brief API to set the weight of nexthop in a nexthop group
 *
 * \param [in] devId Device identifier
 * \param [in] nhGrpId Group identifier
 * \param [in] nhId Next hop identifier
 * \param [in] weight  weight
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetNextHopWeightRouteNextHopGroup(xpsDevice_t devId,
                                                 uint32_t nhGrpId, uint32_t nhId, uint32_t weight);

/**
 * \brief API to get the weight of nexthop from a nexthop group per scope
 *
 * \param [in] scopeId scope identifier
 * \param [in] nhGrpId Group identifier
 * \param [in] nhId Next hop identifier
 * \param [out] weight output weight
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetNextHopWeightRouteNextHopGroupScope(xpsScope_t scopeId,
                                                      uint32_t nhGrpId, uint32_t nhId, uint32_t *weight);

/**
 * \brief API to set the weight of nexthop in a nexthop group per scope
 *
 * \param [in] scopeId scope identifier
 * \param [in] nhGrpId Group identifier
 * \param [in] nhId Next hop identifier
 * \param [in] weight  weight
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetNextHopWeightRouteNextHopGroupScope(xpsScope_t scopeId,
                                                      uint32_t nhGrpId, uint32_t nhId, uint32_t weight);

/**
 * \brief Get a count of Next Hops in the group for a scope.
 *
 * \param [in] scopeId Scope identifier
 * \param [in] nhGrpId Group identifier
 * \param [out] pCount Number of used next hops
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetCountRouteNextHopGroupScope(xpsScope_t scopeId,
                                              uint32_t nhGrpId, uint32_t* pCount);

/**
 * \brief Iterate over next hops in the group for a scope.
 *
 * \param [in] scopeId Scope identifier
 * \param [in] nhGrpId Group identifier
 * \param [in] pNhId Next hop identifier to start from
 * \param [out] pNextNhId Consequent next hop identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetNextNextHopRouteNextHopGroupScope(xpsScope_t scopeId,
                                                    uint32_t nhGrpId, uint32_t* pNhId, uint32_t* pNextNhId);

/**
 * \brief Check if Next Hop group exists in DB for a scope.
 *
 * \param [in] scopeId Scope identifier
 * \param [in] nhGrpId Group identifier
 *
 * \return int
 */
int xpsL3ExistsRouteNextHopGroupScope(xpsScope_t scopeId, uint32_t nhGrpId);

/**
 * \public
 * \brief Set the Next hop contents
 *
 * This is a wrapper around xpsL3SetRouteNextHop
 * that has to be used to properly update source and
 * destination next hops
 *
 * \param [in] devId Device identifier
 * \param [in] nhId Next hop identifier
 * \param [in] pL3NextHopEntry Next hop contents
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetRouteNextHopNextHopGroup(xpsDevice_t devId, uint32_t nhId,
                                           xpsL3NextHopEntry_t *pL3NextHopEntry);

/**
 * \public
 * \brief Destroy a range of next hops
 *
 * This is a wrapper around xpsL3DestroyRouteNextHop
 * that has to be used to avoid breaking existing groups
 *
 * \param [in] devId Device identifier
 * \param [in] nhEcmpSize Number of next hops
 * \param [in] nhId Base next hop identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DestroyRouteNextHopNextHopGroup(xpsDevice_t devId,
                                               uint32_t nhEcmpSize, uint32_t nhId);

/**
 * \brief API that initializes the XPS Next Hop Group
 *
 * API will register for the state databases
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3InitRouteNextHopGroup(void);

/**
 * \brief API that deinitializes the XPS Next Hop Group
 *
 * API will deregister from the state databases
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DeInitRouteNextHopGroup(void);

/**
 * \brief API that sets the XPS Next Hop Group max size
 *
 * \param [in] size Maximum group size
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetRouteNextHopGlobalMaxSize(uint32_t size);

/**
 * \brief Get the Next Hop group global size
 *
 * \param [out] pGroupSize
 *
 * \return uint32_t
 */
XP_STATUS xpsL3GetRouteNextHopGlobalMaxSize(uint32_t* pGroupSize);

/**
 * \brief API to create a new Next Hop Group with an available id
 *
 * \param [out] pNhGrpId Allocated group identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3CreateRouteNextHopGroup(uint32_t *pNhGrpId);

/**
 * \brief Get a count of Next Hops in the group
 *
 * \param [in] nhGrpId Group identifier
 * \param [out] pCount Number of used next hops
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetCountRouteNextHopGroup(uint32_t nhGrpId, uint32_t* pCount);

/**
 * \brief Iterate over next hops in the group
 *
 * \param [in] nhGrpId Group identifier
 * \param [in] pNhId Next hop identifier to start from
 * \param [out] pNextNhId Consequent next hop identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetNextNextHopRouteNextHopGroup(uint32_t nhGrpId,
                                               uint32_t* pNhId, uint32_t* pNextNhId);

/**
 * \brief Check if Next Hop group exists in DB
 *
 * \param [in] nhGrpId Group identifier
 *
 * \return int
 */
int xpsL3ExistsRouteNextHopGroup(uint32_t nhGrpId);

/**
 * \brief Checs if a Nexthop is a member of Next Hop group for a scope.
 *
 * \param [in] nhGrpId Group identifier
 * \param [in] nhId nexthop identifier
 *
 * \return int true or false
 */
int xpsL3ExistsNextHopRouteNextHopGroupScope(xpsScope_t scopeId,
                                             uint32_t nhGrpId, uint32_t nhId);

/**
 * \brief Checs if a Nexthop is a member of Next Hop group for a device.
 *
 * \param [in] nhGrpId Group identifier
 * \param [in] nhId nexthop identifier
 *
 * \return int true or false
 */
int xpsL3ExistsNextHopRouteNextHopGroup(xpsDevice_t devId, uint32_t nhGrpId,
                                        uint32_t nhId);
/**
 * \brief Count  Next Hop group Objects exists in DB
 *
 * \param [out] count Number of next hop group objects.
 *
 * \return XP_STATUS
 */

XP_STATUS xpsCountNextHopGroupObjects(uint32_t *count);
/**
 * \brief Get  Next Hop group ObjectIds exists in DB
 *
 * \param [in] scopeId
 *
 * \param [out] nexthopGrp_id  Next hop group objectIds. Memory allocation to be done by caller
 *
 * \return XP_STATUS
 */

XP_STATUS xpsGetNhGrpIdList(xpsScope_t scopeId, uint32_t *nexthopGrp_id);

/**
 * \brief Check whether given nhId is associated with any nexthop group
 *
 * \param [in] devId
 * \param [in] nhId Nexthop Id to be searched for
 * \param [out] nhGrpId Nexthop group Id to which given nhId is associated
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetRouteNextHopNextHopGroup(xpsDevice_t devId, uint32_t nhId,
                                           uint32_t *nhGrpId);

/**
 * \brief gives nh group info structure stored in state
 *
 * \param [in] scopeId
 * \param [in] nhGrpId Nexthop group Id to be searched for
 * \param [out] ppNhGrpEntry nhGrp infor structure. It contains the nh group information
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetRouteNextHopGroup(xpsScope_t scopeId, uint32_t nhGrpId,
                                    xpsL3NextHopGroupEntry_t** ppNhGrpEntry);

/**
 * \brief Reads max ecmp size from hw
 *
 * \param [in] deviceId   Device id
 * \param [out] maxEcmpSize maximum ecmp size supported in hw
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsL3GetMaxEcmpSize(xpDevice_t devId, uint32_t *ecmpSize);

/**
 * \brief it copies provided nhId to the destination index
 *
 * \param [in] devId device id
 * \param [in] srcNhId next hop id to be copied
 * \param [in] dstNhId index where to copy srcId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3CopyNextHopRouteNextHopGroup(xpsDevice_t devId, uint32_t srcNhId,
                                            uint32_t dstNhId);

/**
 *  * \brief it copies provided nhId to the destination index
 *
 * \param [in] devId device id
 * \param [in] srcNhId next hop id to be copied
 * \param [in] dstNhId index where to copy srcId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3CopyNextHopAndProgramNextHopInGroup(xpsDevice_t devId,
                                                   uint32_t srcNhId, uint32_t dstNhId);

/**
 * \brief Updates the next hop group entry in db
 *
 * \param [in/out] pNhGrpEntry next hop group entry structure
 * \param [in] nhId next hop ids to be added in nhIdMap
 *
 * \return
 */
void xpsL3NextHopGroupSetDbEntryNhIdMaps(xpsL3NextHopGroupEntry_t* pNhGrpEntry,
                                         uint32_t *nhId);

/**
 * \brief API to get max ecmp size from xps
 *
 * \param [out] maxEcmpSize maximum ecmp size supported in hw
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsL3GetNextHopGroupMaxEcmpSize(uint32_t *ecmpSize);

/**
 * \brief API to set max ecmp size from xps
 *
 * \param [in] maxEcmpSize maximum ecmp size supported in hw
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsL3SetNextHopGroupMaxEcmpSize(uint32_t ecmpSize);

/**
 * \brief API to get max nh groups size from xps
 *
 * \param [in] maxNum maximum nh groups number supported in hw
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsL3GetNextHopGroupsMaxNum(uint32_t *maxNum);

XP_STATUS xpsL3GetNextRouteNextHopGroup(xpsScope_t scopeId,
                                        xpsL3NextHopGroupEntry_t* pNhGrpEntry,
                                        xpsL3NextHopGroupEntry_t** ppNhGrpEntry);

bool xpsL3EcmpRandomEnableGet(void);
void xpsL3EcmpRandomEnableSet(bool enable);
XP_STATUS xpsL3RandomEnableRewrite(xpsDevice_t devId, bool enable);
XP_STATUS xpsL3SetRandomHashSeed(xpsDevice_t devId, uint32_t hashSeed);

#ifdef __cplusplus
}
#endif


#endif  //_xpsNhGrp_h_

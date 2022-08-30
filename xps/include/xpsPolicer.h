// xpsPolicer.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsPolicer.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Policer Manager
 */

#ifndef _xpsPolicer_h_
#define _xpsPolicer_h_

#include "xpTypes.h"
#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsPolicer.h"
#include "xpsInternal.h"
#include <math.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief xpPolicerEntry_t
 */
typedef struct xpPolicerCounterEntry_t
{
    uint64_t    redSum;     ///< redSum
    uint64_t    redEvent;       ///< redEvent
    uint64_t    yellowSum;      ///< yellowSum
    uint64_t    yellowEvent;        ///< yellowEvent
    uint64_t    greenSum;       ///< greenSum
    uint64_t    greenEvent;     ///< greenEvent
} xpPolicerCounterEntry_t;

typedef struct xpPolicerEntry_t
{

    uint16_t   cbsBase;     ///< cbsBase
    uint8_t    cbsExpo;     ///< cbsExpo
    uint16_t   pbsBase;     ///< pbsBase
    uint8_t    pbsExpo;     ///< pbsExpo

    uint16_t   cirBase;     ///< cirBase
    uint8_t    cirExpo;     ///< cirExpo
    uint8_t    cirTimeGranExpo;     ///< cirTimeGranExpo

    uint16_t   pirBase;     ///< pirBase
    uint8_t    pirExpo;     ///< pirExpo
    uint8_t    pirTimeGranExpo;     ///< pirTimeGranExpo

    uint8_t    colorAware;      ///< colorAware
    uint8_t    dropRed;         ///< dropRed
    uint8_t    dropYellow;      ///< dropYellow
    uint8_t    remarkGreen;     ///< remarkGreen
    uint8_t    remarkYellow;    ///< remarkYellow
    uint8_t    remarkRed;       ///< remarkRed
    xpPolicingResultType    polResult;      ///< polResult
    uint8_t
    isPacketBasedPolicing;   ///< Is packet based policing or byte based policing

} xpPolicerEntry_t;

/**
 * \brief xpPolicerEntity
 */

typedef struct xpPolicerEntity
{

    uint32_t cir;       ///< cir
    uint32_t pir;       ///< pir
    uint32_t cbs;       ///< cbs
    uint32_t pbs;       ///< pbs
    uint32_t colorAware;        ///< colorAware
    uint32_t dropRed;       ///< dropRed
    uint32_t dropYellow;    ///< dropYellow
    uint32_t updateResultRed;       ///< updateResultRed
    uint32_t updateResultYellow;       ///<  updateResultYellow
    uint32_t updateResultGreen;     ///< updateResultGreen
    xpPolicingResultType polResult;     ///< polResult

} xpPolicerEntity_t;

typedef xpPolicerCounterEntry_t xpsPolicerCounterEntry_t;

/* Storm type enum
 * This enum is used as index policer entry table */
typedef enum xpsPolicerStormType_e
{
    XPS_ALL_STORM_CONTROL = 0,
    XPS_UNKNOWN_STORM_CONTROL,
    XPS_MC_STORM_CONTROL,
    XPS_BC_STORM_CONTROL
} xpsPolicerStormType_e;

/*
* XPS Policer Mgr Public APIs
*/

/**
* \public
* \brief xpsPolicerInit routine for the Policer manager in a scope
*
* \param [in] scopeId
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerInitScope(xpsScope_t scopeId);

/**
* \public
* \brief uninitialize routine for the Policer manager  in a scope
*
* \param [in] scopeId
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerDeInitScope(xpsScope_t scopeId);

/**
* \public
* \brief API to initialize and add device speicific primitives
*        in primitive manager(s).
*
* \param [in] devId
* \param [in] initType
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
* \public
* \brief API to de-initialize and remove device speicific primitives
*        in primitive manager(s).
*
* \param [in] devId
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerRemoveDevice(xpsDevice_t devId);

/**
* \public
* \brief Enables port policing on a particular port in a scope.
*
* \param [in] scopeId
* \param [in] portIntfId
* \param [in] enable
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerEnablePortPolicingScope(xpsScope_t scopeId,
                                            xpsInterfaceId_t portIntfId, uint16_t enable);

/**
* \public
* \brief Removes a policer entry at a given index
*
* \param [in] devId
* \param [in] client
* \param [in] index
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerRemoveEntry(xpsDevice_t devId, xpsPolicerType_e client,
                                uint32_t index);

/**
* \public
* \brief Obtains a policer entry at a given index
*
* \param [in] devId
* \param [in] client
* \param [in] index
* \param [out] pEntry
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerGetEntry(xpsDevice_t devId, xpsPolicerType_e client,
                             uint32_t index, xpsPolicerEntry_t *pEntry);

/**
* \public
* \brief Obtains policer counter entry for a policer index.
*
* \note The counter banks should be configured with mode XP_ANA_BANK_MODE_PC
* and "dualBank" should be enabled on the client interface in order to avail
* policer counters.
*
* \param [in] devId
* \param [in]  client
* \param [in] index
* \param [out] pEntry
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerGetPolicerCounterEntry(xpsDevice_t devId,
                                           xpsPolicerType_e client, uint32_t index, xpsPolicerCounterEntry_t *pEntry);

/**
* \public
* \brief Adds a policing entry for a given port id in a scope
*
* \param [in] scopeId
* \param [in] portIntfId
* \param [in] client
* \param [in] pEntry
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerAddPortPolicingEntryScope(xpsScope_t scopeId,
                                              xpsInterfaceId_t portIntfId, xpsPolicerType_e client,
                                              xpsPolicerEntry_t *pEntry);

/**
* \public
* \brief Removes a policing entry for a given port id in a scope
*
* \param [in] scopeId
* \param [in] portIntfId
* \param [in] client
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerRemovePortPolicingEntryScope(xpsScope_t scopeId,
                                                 xpsInterfaceId_t portIntfId, xpsPolicerType_e client);

/**
* \public
* \brief Sets an attribute for a policer entry
*
* \param [in] devId
* \param [in] client
* \param [in] index
* \param [in] field
* \param [in] *data
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerSetAttribute(xpsDevice_t devId, xpsPolicerType_e client,
                                 uint32_t index, xpPolicerField_t field, void *data);

/**
* \public
* \brief Obtains an attribute for a policer entry
*
* \param [in] devId
* \param [in] client
* \param [in] index
* \param [in] field
* \param [out] *data
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerGetAttribute(xpsDevice_t devId,  xpsPolicerType_e client,
                                 uint32_t index, xpPolicerField_t field, void *data);

/**
* \public
* \brief Sets the policing result, given the result type and the policing results for red, yellow and green
* \param [in] devId
* \param [in] client
* \param [in] paramIndex
* \param [in] resultType
* \param [in] *redPolResult
* \param [in] *yellowPolResult
* \param [in] *greenPolResult
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerSetResultByType(xpsDevice_t devId, xpsPolicerType_e client,
                                    uint32_t index, xpPolicingResultType resultType,
                                    xpPolicerResult_t *redPolResult, xpPolicerResult_t *yellowPolResult,
                                    xpPolicerResult_t *greenPolResult);

/**
* \public
* \brief Gets the policing result, given the result type and the policing results for red, yellow and green
* \param [in] devId
* \param [in] client
* \param [in] paramIndex
* \param [in] resultType
* \param [out] *redPolResult
* \param [out] *yellowPolResult
* \param [out] *greenPolResult
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerGetResultByType(xpsDevice_t devId, xpsPolicerType_e client,
                                    uint32_t index, xpPolicingResultType resultType,
                                    xpPolicerResult_t *redPolResult, xpPolicerResult_t *yellowPolResult,
                                    xpPolicerResult_t *greenPolResult);

/**
* \public
* \brief Sets the policing result for a given color, given the result type and the policing result
* \param [in] devId
* \param [in] client
* \param [in] paramIndex
* \param [in] resultType
* \param [in] color
* \param [in] *polResult
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerSetResultByColor(xpsDevice_t devId,
                                     xpsPolicerType_e client,
                                     uint32_t index, xpPolicingResultType resultType, xpPolicingResultColor color,
                                     xpPolicerResult_t *polResult);

/**
* \public
* \brief xpsPolicerInit routine for the Policer manager
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerInit(void);

/**
* \public
* \brief uninitialize routine for the Policer manager
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerDeInit(void);

/**
* \public
* \brief Removes a policing entry for a given port id
*
* \param [in] portIntfId
* \param [in] client
*
* \return XP_STATUS
*/
XP_STATUS xpsPolicerRemovePortPolicingEntry(xpsInterfaceId_t portIntfId,
                                            xpsPolicerType_e client);

/**
 * \brief Set the policer standard type
 *
 * In device, two types of policer standards are supported.
 * One is RFC 2698 and other is MEF 10.1 section 7.11. Use
 * this API to set the policer standard to RFC/MEF.
 *
 * This API is not supported on xp80 devices.
 *
 * \param [in] devId Device id
 * \param [in] polStandard Policer standard either RFC or MEF
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPolicerSetPolicerStandard(xpDevice_t devId,
                                       xpPolicerStandard_e polStandard);

/**
 * \brief Get the policer standard type
 *
 * In device, two types of policer standards are supported.
 * One is RFC 2698 and other is MEF 10.1 section 7.11. Use
 * this API to set the policer standard to RFC/MEF.
 *
 * This API is not supported on xp80 devices.
 *
 * \param [in]  devId Device id
 * \param [out] polStandard Policer standard either RFC or MEF
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPolicerGetPolicerStandard(xpDevice_t devId,
                                       xpPolicerStandard_e* polStandard);

/**
 * \brief API to enable packet based policing
 *
 * \param [in] devId
 * \param [in] client
 * \param [in] index
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPolicerEnablePacketBasedPolicing(xpDevice_t devId,
                                              xpsPolicerType_e client, uint32_t index, uint32_t enable);

/**
 * \brief APT to test if packet based policing is enabled
 *
 * \param [in] devId
 * \param [in] client
 * \param [in] index
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPolicerIsPacketBasedPolicingEnabled(xpDevice_t devId,
                                                 xpsPolicerType_e client, uint32_t index, uint32_t* enable);

/**
 * \brief API to enable byte based policing
 *
 * \param [in] devId
 * \param [in] client
 * \param [in] index
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPolicerEnableByteBasedPolicing(xpDevice_t devId,
                                            xpsPolicerType_e client, uint32_t index, uint32_t enable);

/**
 * \brief APT to test if byte based policing is enabled
 *
 * \param [in] devId
 * \param [in] client
 * \param [in] index
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPolicerIsByteBasedPolicingEnabled(xpDevice_t devId,
                                               xpsPolicerType_e client, uint32_t index, uint32_t* enable);

XP_STATUS xpsPolicerIndexAllocate(xpsPolicerType_e client,
                                  uint32_t *pPolicerIndex);

XP_STATUS xpsPolicerIndexRelease(xpsPolicerType_e client,
                                 uint32_t pPolicerIndex);

XP_STATUS xpsPolicerPortPolicerIndexGet(xpsDevice_t devId, uint32_t portId,
                                        xpsPolicerType_e policerType, xpsPolicerStormType_e stormType,
                                        uint32_t *pPolicerIndex);

XP_STATUS xpsPolicerStormTypeEnable(xpsDevice_t devId, uint32_t portId,
                                    xpsPolicerStormType_e stormType, bool enable);

XP_STATUS xpsPolicerClearPolicerCounterEntry(xpsDevice_t devId,
                                             xpsPolicerType_e policerType, uint32_t index);

XP_STATUS xpsPolicerEPortPolicingEnable(xpsDevice_t devId, uint32_t ePort,
                                        xpsPolicerType_e policerType, uint32_t index, bool enable);

#ifdef __cplusplus
}
#endif

#endif  //_xpsPolicer_h_

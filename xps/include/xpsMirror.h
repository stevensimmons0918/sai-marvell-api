// xpsMirror.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpsMirror.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Mirror Manager
 */

#ifndef _xpsMirror_h_
#define _xpsMirror_h_

#include "xpEnums.h"
#include "xpTypes.h"
#include "xpsInterface.h"
#include "xpsVlan.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xpMirrorSpanData_t
{
    xpsInterfaceId_t analyzerIntfId;    ///< Destination Interface
    uint32_t         truncateSize;
    uint8_t          tc;
    uint32_t         policerId;
    uint32_t         ePortPolicerIndex;
    uint32_t         sampleRate;
    uint32_t         ePort;
    bool isIngress;

} xpMirrorSpanData_t;

typedef struct xpMirrorErspan2Data_t
{
    uint32_t         erspanId;      ///< Gre Tunnel id
    xpsInterfaceId_t analyzerIntfId;    ///< Destination Interface
    uint32_t         tos;
    uint32_t         greProtoType;
    uint32_t         truncateSize;
    macAddr_t        srcMacAddr;
    macAddr_t        dstMacAddr;
    uint8_t          tc;
    uint32_t         policerId;
    uint32_t         ePortPolicerIndex;
    uint32_t         sampleRate;
    uint32_t         ePort;
    uint8_t          ttlHopLimit;
    xpIpPrefixType_t ipType;
    inetAddr_t       srcIpAddr;
    inetAddr_t       dstIpAddr;
    uint16_t         vlanId;
    uint8_t          vlanPri;
    uint8_t          vlanCfi;
    uint16_t         vlanTpid;
    bool isIngress;

} xpMirrorErspan2Data_t;

/**
 * \public
 * \brief This struct defines the data needed for rspan mirroring
 *
 */
typedef struct xpMirrorRspanData_t
{
    uint32_t bdId;      ///< Bd(=vlan)h for RSPAN
} xpMirrorRspanData_t;

/**
 * \public
 * \brief This struct defines the data needded for different types of mirroring.
 *
 */
typedef union xpMirrorData_t
{
    xpMirrorSpanData_t spanData;
    xpMirrorErspan2Data_t
    erspan2Data; ///< This union has the potential include other data when we support rspan and erspan3.
    xpMirrorRspanData_t   rspanData;
} xpMirrorData_t;

typedef struct
{
    uint32_t        bucketId;
    uint32_t        numSessions;
} xpsMirrorBktDbEntry_t;

typedef xpMirrorType_e xpsMirrorType_e;
typedef xpMirrorData_t xpsMirrorData_t;

/**
 * \brief Store Analyzer session entry into Mirror bucket.
 *
 * \param [in] scopeId
 * \param [in] analyzerId
 * \param [in] mirrortype Local or ERSPAN session
 * \param [in] mirrorData relevant for ERSPAN mirroring
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorDbAddAnalyzer(xpsScope_t scopeId, uint32_t analyzerId,
                                 xpsMirrorType_e mirrorType, xpsMirrorData_t mirrorData);

/**
 * \brief Init API for initialising the function pointers
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsMirrorInitApi(xpsDevice_t deviceId);

/**
 * \brief This method initializes Mirror Mgr with system specific
 *        references to various primitive managers and their initialization.
 *
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorInit(void);

/**
 * \brief This method cleans up from the Mirror Mgr system specific
 *        references to various primitive managers.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorDeInit(void);

/**
 * \brief This method initializes Mirror Mgr with system specific
 *        references to various primitive managers and their initialization
 *        for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorInitScope(xpsScope_t scopeId);

/**
 * \brief This method cleans up from the Mirror Mgr system specific
 *        references to various primitive managers in a Scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorDeInitScope(xpsScope_t scopeId);

/**
 * \brief This method configures and add device specific primitives
 *        required for the Mirror FL manager.
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief This method removes device specific primitives
 *        required for the Mirror FL manager.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorRemoveDevice(xpsDevice_t devId);

/**
 * \brief Create analyzer session.
 * Allocates an analyzer id and returns it to the user.
 *
 * \param [in] type
 * \param [in] data
 * \param [out] analyzerId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorCreateAnalyzerSession(xpsMirrorType_e type,
                                         xpsMirrorData_t data, uint32_t *analyzerId);

/**
 * \brief Create analyzer session for a scope.
 * Allocates an analyzer id and returns it to the user.
 *
 * \param [in] scopeId
 * \param [in] type
 * \param [in] data
 * \param [out] analyzerId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorCreateAnalyzerSessionScope(xpsScope_t scopeId,
                                              xpsMirrorType_e type, xpsMirrorData_t data, uint32_t *analyzerId);

/**
 * \brief Remove the analyzer sessions and all the interfaces associated with it from a particular device.
 *
 * \param [in] deviceId
 * \param [in] analyzerId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorRemoveAnalyzerSession(xpsDevice_t deviceId,
                                         uint32_t analyzerId);

/**
 * \brief Writes the analyzer session onto a particular device.
 *
 * \param [in] deviceId
 * \param [in] analyzerId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorWriteAnalyzerSession(xpsDevice_t deviceId,
                                        uint32_t analyzerId);

/**
 * \brief Writes the analyzer session onto a particular device.
 *
 * \param [in] deviceId
 * \param [in] analyzerId
 * \param [in] mirrorData
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorUpdateAnalyzerSession(xpsDevice_t deviceId,
                                         uint32_t analyzerId, xpsMirrorData_t * mirrorData);

/**
 * \brief Add an analyzer interface to an existing analyzer session.
 *
 * \param [in] analyzerId
 * \param [in] analyzerIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorAddAnalyzerInterface(uint32_t analyzerId,
                                        xpsInterfaceId_t analyzerIntfId);

/**
 * \brief Add an analyzer interface to an existing analyzer session.
 *
 * \param [in] analyzerId
 * \param [in] analyzerIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorAddAnalyzerInterfaceScope(xpsScope_t scopeId,
                                             uint32_t analyzerId, xpsInterfaceId_t analyzerIntfId);

/**
 * \brief Get analyzer interface (corresponding to RSPAN vlan) to be added for a mirror session
 *
 * \param [in] vlanId
 * \param [out] analyzerIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMirrorGetRspanAnalyzerInterface(xpsDevice_t deviceId,
                                             xpsVlan_t vlanId, uint32_t *analyzerIntfId);

/**
 * \brief Get ePort for each analyzerId
 *
 * \param [in] scopeId
 * \param [in] analyzerId
 * \param [out] ePort
 *
 */
XP_STATUS xpsMirrorGetAnalyzerEPort(xpsScope_t scopeId, uint32_t analyzerId,
                                    uint32_t *ePort);

/**
 * \brief Set sampleRate for analyzerId
 *
 * \param [in] scopeId
 * \param [in] analyzerId
 * \param [in] truncateSize
 *
 */
XP_STATUS xpsMirrorSetSampleRateFalcon(xpsDevice_t deviceId,
                                       uint32_t analyzerId, uint32_t sampleRate, bool ingress);
/**
 * \brief Set truncateState for analyzerId
 *
 * \param [in] scopeId
 * \param [in] analyzerId
 * \param [in] truncateSize
 *
 */
XP_STATUS xpsMirrorSetTruncateStateFalcon(xpsDevice_t deviceId,
                                          uint32_t analyzerId, uint32_t truncateSize);

/**
 * \brief Set truncateState for analyzerId
 *
 * \param [in] scopeId
 * \param [in] analyzerId
 * \param [in] truncateSize
 *
 */
XP_STATUS xpsMirrorSetTruncateStateLegacy(xpsDevice_t deviceId,
                                          uint32_t analyzerId, uint32_t truncateSize);

/**
 * \brief Set policer index for analyzerId
 *
 * \param [in] analyzerId
 * \param [out] eport
 * \param [out] polcierIndex
 *
 */
XP_STATUS xpsMirrorGetAnalyzerIntfPolicerIndex(uint32_t analyzerId,
                                               uint32_t *ePort, uint32_t *policerIndex);

XP_STATUS xpsMirrorUpdLagMember(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                                xpsInterfaceId_t iface, bool removed, uint32_t analyzerId);

XP_STATUS xpsMirrorErspanCfgSet(xpsDevice_t devId,
                                uint32_t analyzerId, xpsMirrorData_t *mirrorData);

XP_STATUS xpsMirrorErspanTunCreate(xpsDevice_t devId,
                                   uint32_t analyzerId, xpsMirrorData_t *mirrorData);

XP_STATUS xpsMirrorErspanTunRemove(xpsDevice_t devId,
                                   uint32_t analyzerId);

XP_STATUS xpsMirrorErspanSessionTypeGet(xpsDevice_t deviceId,
                                        uint32_t analyzerId, bool *isIpv4,
                                        xpsMirrorType_e *mirType, uint32_t *ePort);

XP_STATUS xpsMirrorUpdateSessionId(xpsDevice_t devId, uint32_t analyzerId,
                                   bool isIngress);
XP_STATUS xpsMirrorUpdateErSpan2Index(xpsDevice_t devId, uint32_t xpsIntfId,
                                      bool isIngress, bool enable);
XP_STATUS xpsMirrorVlanUpdateErSpan2Index(xpsDevice_t devId, uint32_t xpsIntfId,
                                          bool isIngress, bool enable);

#ifdef __cplusplus
}
#endif

#endif  //_xpsMirror_h_

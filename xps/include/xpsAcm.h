// xpsAcm.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsAcm.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS ACM Manager
 */


#ifndef _xpsAcm_h_
#define _xpsAcm_h_

#include "openXpsAcm.h"
//#include "xpAcmMgr.h"
#include "xpsInit.h"

#ifdef __cplusplus
extern "C" {
#endif



/*
 * XPS Acm Mgr Public APIs
 */

/**
* \public
* \brief xpsAcmInit routine for the ACM manager for a scope
*
* \param [in] scopeId
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmInitScope(xpsScope_t scopeId);

/**
* \public
* \brief uninitialize routine for the ACM manager for a scope
*
* \param [in] scopeId
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmDeInitScope(xpsScope_t scopeId);

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
XP_STATUS xpsAcmAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
* \public
* \brief API to de-initialize and remove device speicific primitives
*        in primitive manager(s).
*
* \param [in] devId
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmRemoveDevice(xpsDevice_t devId);

/**
* \public
* \brief Updates a field in the entry.
*
* This function prints the counter value from the specified
* index in the device
*
* \param [in] devId
* \param [in] client
* \param [in] countIndex
* \param [in] printZeros
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmPrintCounterValue(xpsDevice_t devId, xpAcmClient_e client,
                                  uint32_t countIndex, int printZeros);

/**
 * \public
 * \brief Updates a field in the entry.
 *
 * This function prints the counter value for client's all
 * index in the device
 *
 * \param [in] devId
 * \param [in] client
 * \param [in] printZeros
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcmPrintAllCounterValue(xpsDevice_t devId, xpAcmClient_e client,
                                     int printZeros);

/**
* \public
* \brief Updates a field in the entry.
*
* This sets the sampling parameters from the specified index in
* the device
*
* \param [in] devId
* \param [in] client
* \param [in] index
* \param [in] nSample
* \param [in] mBase
* \param [in] mExpo
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmGetSamplingConfig(xpsDevice_t devId, xpAcmClient_e client,
                                  uint32_t index, uint32_t *nSample, uint32_t *mBase, uint32_t *mExpo);

/**
* \public
* \brief Updates a field in the entry.
*
* This sets the sampling parameters from the specified index in
* the device
*
* \param [in] devId
* \param [in] client
* \param [in] index
* \param [in] nSample
* \param [in] mBase
* \param [in] mExpo
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmSetSamplingConfig(xpsDevice_t devId, xpAcmClient_e client,
                                  uint32_t index, uint32_t nSample, uint32_t mBase, uint32_t mExpo);

/**
* \public
* \brief Updates a field in the entry.
*
* This gets the sampling parameters from the specified index in
* the device
*
* \param [in] devId
* \param [in] client
* \param [in] index
* \param [in] totalCnt
* \param [in] interEventCnt
* \param [in] interSampleStart
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmGetSamplingState(xpsDevice_t devId,  xpAcmClient_e client,
                                 uint32_t index, uint64_t *totalCnt, uint32_t *interEventCnt,
                                 uint32_t *interSampleStart);


/**
* \public
* \brief Updates a field in the entry.
*
* This sets the sampling parameters from the specified index in
* the device
*
* \param [in] devId
* \param [in] client
* \param [in] index
* \param [in] totalCnt
* \param [in] interEventCnt
* \param [in] interSampleStart
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmSetSamplingState(xpsDevice_t devId,  xpAcmClient_e client,
                                 uint32_t index, uint64_t totalCnt, uint32_t interEventCnt,
                                 uint32_t interSampleStart);

/**
* \public
* \brief xpsAcmCntSetGlobalConfigBucketization
*
* \param [in] devId
* \param [in] enable
* \param [in] startRange
* \param [in] endRange
* \param [in] numBkts
* \param [in] granularity
* \param [in] addAddr
* \param [in] bktUseAddr
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmCntSetGlobalConfigBucketization(xpsDevice_t devId,
                                                uint8_t enable, int startRange, int endRange,
                                                uint8_t numBkts, uint8_t granularity, uint8_t addAddr, uint8_t bktUseAddr);

/**
* \public
* \brief xpCntSetGlobalConfigModePol
*
* \param [in] devId
* \param [in] refreshEnable
* \param [in] unitTime
* \param [in] refrTimeGranularity
* \param [in] updateWeight
* \param [in] billingCntrEnable
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmCntSetGlobalConfigModePol(xpsDevice_t devId,
                                          uint8_t refreshEnable, uint8_t unitTime, uint8_t refrTimeGranularity,
                                          uint8_t updateWeight, uint8_t billingCntrEnable);

/**
* \public
* \brief Updates a field in the entry.
*
* This prints the sampling parameters from the specified index
* in the device
*
* \param [in] devId
* \param [in] client
* \param [in] index
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmPrintSamplingConfig(xpsDevice_t devId, xpAcmClient_e client,
                                    uint32_t index);

/**
* \public
* \brief Updates a field in the entry.
*
* This gets the sampling parameters from the specified index in
* the device
*
* \param [in] devId
* \param [in] client
* \param [in] index
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmPrintSamplingState(xpsDevice_t devId,  xpAcmClient_e client,
                                   uint32_t index);

/**
* \public
* \brief Allocate a Acm of a passed in type
*
* Will allocate a Acm of a desired type provided that there are
* Vifs remaining that are free
*
* \param [in] client
* \param [in] type
* \param [out] acmId
*
* \return XP_STATUS
*/
XP_STATUS xpsAllocateAcm(xpAcmClient_e client, xpAcmType_t type,
                         xpAcm_t *acmId);

/**
* \public
* \brief Release a Acm of a passed in type
*
* Will allocate a Acm of a desired type
*
* \param [in] client
* \param [in] type
* \param [in] acmId
*
* \return XP_STATUS
*/
XP_STATUS xpsReleaseAcm(xpAcmClient_e client, xpAcmType_t type, xpAcm_t acmId);

/**
 * \public
 * \brief Enables/disables drop reason code counting in a device
 *
 * \param [in] devId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcmSetDropReasonCodeCountingEnable(xpsDevice_t devId,
                                                uint32_t enable);

/**
* \public
* \brief xpsAcmInit routine for the ACM manager
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmInit(void);

/**
* \public
* \brief uninitialize routine for the ACM manager
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmDeInit(void);

/**
 * \brief API to set the ACM lane index base sel for a given device and ACM client
 *
 * This allow to add a base to ACM req indexes before sending to the ACM
 * based on base sel.
 *   0: not add
 *   1: add Token.PortID
 *   2: add Token.iVif
 *
 * \param [in] devId Device Id
 * \param [in] client Acm client type
 * \param [in] baseSel Acm lane index base sel
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcmSetLaneIndexBaseSel(xpDevice_t devId, xpAcmClient_e client,
                                    xpAcmType_t type, uint32_t baseSel);

/**
 * \brief API to get the ACM lane index base sel for a given device and ACM client
 *
 * This allow to add a base to ACM req indexes before sending to the ACM
 * based on base sel.
 *   0: not add
 *   1: add Token.PortID
 *   2: add Token.iVif
 *
 * \param [in]  devId Device Id
 * \param [in]  client Acm client type
 * \param [out] baseSel Acm lane index base sel
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcmGetLaneIndexBaseSel(xpDevice_t devId, xpAcmClient_e client,
                                    xpAcmType_t type, uint32_t *baseSel);

#ifdef __cplusplus
}
#endif

#endif  //_xpsAcm_h_

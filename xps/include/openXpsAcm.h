// openXpsAcm.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file openXpsAcm.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS ACM Manager
 */

#ifndef _openXpsAcm_h_
#define _openXpsAcm_h_

#include "openXpsTypes.h"
#include "openXpsEnums.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \public
 * \brief set counter bank mode, clear on read, wrapAround and
 *        countOffset
 *
 * \param [in] devId
 * \param [in] client
 * \param [in] type
 * \param [in] bankMode
 * \param [in] clearOnRead
 * \param [in] wrapAround
 * \param [in] countOffset
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSetCountMode(xpsDevice_t devId, xpAcmClient_e client,
                          xpAcmType_t type, int bankMode,
                          int clearOnRead, int wrapAround, int countOffset);

/**
 * \public
 * \brief Updates a field in the entry.
 *
 * This gets the counter value from the specified index in the
 * device
 *
 * \param [in] devId
 * \param [in] countIndex
 * \param [out] * nPkts
 * \param [out] * nBytes
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcmGetCounterValue(xpsDevice_t devId, xpAcmClient_e client,
                                uint32_t countIndex, uint64_t *nPkts, uint64_t *nBytes);

/**
 * \public
 * \brief Updates a field in the entry.
 *
 * This gets the counter value from the sde specified index in the
 * device
 *
 * \param [in] devId
 * \param [in] client
 * \param [in] seVal
 * \param [in] countIndex
 * \param [out] * nPkts
 * \param [out] * nBytes
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAcmGetSeCounterValue(xpsDevice_t devId, xpAcmClient_e client,
                                  uint32_t seVal, uint32_t countIndex, uint64_t *nPkts, uint64_t *nBytes);
#ifdef __cplusplus
}
#endif

#endif  //_xpsAcm_h_



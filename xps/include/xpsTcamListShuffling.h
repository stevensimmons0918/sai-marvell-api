// xpsTcamListShuffling.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsTcamListShuffling.h
 * \brief This file contains API prototypes and type definitions for xps
 * implementation of the shuffling algortihm.
 */

#ifndef _xpsTcamListShuffling_h_
#define _xpsTcamListShuffling_h_

#include "xpTypes.h"
#include "xpsEnums.h"
#include "xpsTcamMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * \brief Initialize table data structures
 *
 * \param [in] devId
 * \param [in] tableId
 * \param [in] entryNum
 * \param [in] prioNum
 * \param [in] ruleMove
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTcamListShufflingConfigTable(xpsDevice_t devId, uint32_t tableId,
                                          uint32_t entryNum, uint32_t prioNum, xpsTcamMgrRuleMove_fn ruleMove);

/*
 * \brief Uninitialize table structures
 *
 * \param [in] devId
 * \param [in] tableId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTcamListShufflingDeleteTable(xpsDevice_t devId, uint32_t tableId);

/*
 * \brief Add entry
 *
 * \param [in]  xpsDevice_t devId
 * \param [in] tableId
 * \param [in] priority
 * \param [out] entryId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTcamListShufflingAddEntry(xpsDevice_t devId, uint32_t tableId,
                                       uint32_t priority, uint32_t* entryId);

/*
 * \brief Delete entry
 *
 * \param [in] devId
 * \param [in] tableId
 * \param [in] entryId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTcamListShufflingDeleteEntry(xpsDevice_t devId, uint32_t tableId,
                                          uint32_t entryId);

/*
 * \brief Deliver TCAM ID from entry ID
 *
 * \param [in] devId
 * \param [in] tableId
 * \param [in] entryId
 * \param [out] tcamId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTcamListShufflingGetTcamId(xpsDevice_t devId, uint32_t tableId,
                                        uint32_t entryId, uint32_t *tcamId);

/*
 * \brief Deliver Entry ID from TCAM ID
 *
 * \param [in] devId
 * \param [in] tableId
 * \param [in] tcamId
 * \param [out] entryId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTcamListShufflingGetEntryId(xpsDevice_t devId, uint32_t tableId,
                                         uint32_t tcamId, uint32_t *entryId);


#ifdef __cplusplus
}
#endif

#endif // _xpsTcamListShuffling_h_


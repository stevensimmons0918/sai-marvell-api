// xpsTcamAclShuffling.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsTcamAclShuffling.h
 * \brief This file contains API prototypes and type definitions for xps
 * implementation of the shuffling algortihm.
 */

#ifndef _xpsTcamAclShuffling_h_
#define _xpsTcamAclShuffling_h_

#include "xpTypes.h"
#include "xpsEnums.h"
#include "xpsTcamMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Some variables explanation:
 *     tableId - HW table ID
 *     entryId - ID that corresponds TCAM hit index, but never changes. Application has to remeber it.
 *     tcamId  - HW hit index. Should be used for HW access.
 */

/**
 * \brief ruleMap structure maps tcamId to entryId and stores entry priority
 *        TCAM ID is equal to array index
 *
 */
typedef struct _xpsTcamMgrRuleMap
{
    bool     isEnabled;     // Entry state. TRUE if entry is in use
    uint32_t entryId;       // Application ID
    uint32_t entryPrio;     // Application level priority
} xpsTcamMgrRuleMap_t;

/**
 * \struct xpsTcamMgrAclShufflingKeyFormatDbData_t
 *         This structure maintains mapping between tableId -> TCAM Mgr related data
 *
 */
typedef struct _xpsTcamMgrAclShufflingKeyFormatDbData
{
    uint32_t tableId;
    uint32_t tableEntryNum;
    xpsTcamMgrRuleMap_t ruleMap[XP_ACL_MAX_ENTRY_COUNT];
    xpsTcamMgrRuleMove_fn ruleMove;
} xpsTcamMgrAclShufflingKeyFormatDbData_t;

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
XP_STATUS xpsTcamAclShufflingConfigTable(xpsDevice_t devId, uint32_t tableId,
                                         uint32_t entryNum, uint32_t prioNum, xpsTcamMgrRuleMove_fn ruleMove);

/*
 * \brief Uninitialize table structures
 *
 * \param [in] devId
 * \param [in] tableId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTcamAclShufflingRemoveTable(xpsDevice_t devId, uint32_t tableId);

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
XP_STATUS xpsTcamAclShufflingAddEntry(xpsDevice_t devId, uint32_t tableId,
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
XP_STATUS xpsTcamAclShufflingDeleteEntry(xpsDevice_t devId, uint32_t tableId,
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
XP_STATUS xpsTcamAclShufflingGetTcamId(xpsDevice_t devId, uint32_t tableId,
                                       uint32_t entryId, uint32_t *tcamId);

/*
 * \brief Deliver entry ID from TCAM ID
 *
 * \param [in] devId
 * \param [in] tableId
 * \param [in] tcamId
 * \param [out] entryId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTcamAclShufflingGetEntryId(xpsDevice_t devId, uint32_t tableId,
                                        uint32_t tcamId, uint32_t *entryId);

/*
 * \brief Print debug related information
 *
 * \param [in] devId
 * \param [in] tableId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTcamAclShufflingDebug(xpsDevice_t devId, uint32_t tableId);

#ifdef __cplusplus
}
#endif

#endif // _xpsTcamAclShuffling_h_


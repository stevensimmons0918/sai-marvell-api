// openXpsAclIdMgr.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file openXpsAclIdMgr.h
 * \brief This file contains API prototypes and type definitions
 *        for the ACL ID allocator
 *
 */

#ifndef _openXpsAclIdMgr_h_
#define _openXpsAclIdMgr_h_

#include "openXpsTypes.h"
#include "openXpsEnums.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
* Some variables explanation:
*     AclId - Access Control List Id
*/

/**
 * \brief API to allocate AclId
 *
 * \param [in] devId
 * \param [out] aclId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAclIdAllocEntry(xpsDevice_t devId, uint8_t* aclId);

/**
 * \brief API to free AclId
 *
 * \param [in] devId
 * \param [in] aclId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAclIdFreeEntry(xpsDevice_t devId, uint8_t aclId);

/**
 * \brief API to allocate AclId of corresponding table type
 *
 * \param [in] devId
 * \param [in] idType
 * \param [out] result
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAclIdAllocTableId(xpsDevice_t devId, xpAclType_e idType,
                               uint32_t* result);

/**
 * \brief API to free AclId of corresponding table type
 *
 * \param [in] devId
 * \param [in] idType
 * \param [in] id
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAclIdFreeTableId(xpsDevice_t devId, xpAclType_e idType,
                              uint32_t id);

/**
 * \brief API to allocate AclId of AclEntry type
 *
 * \param [in] devId
 * \param [out] result
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAclIdAllocEntryId(xpsDevice_t devId, uint32_t* result);

/**
 * \brief API to free AclId of AclEntry type
 *
 * \param [in] devId
 * \param [in] id
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAclIdFreeEntryId(xpsDevice_t devId, uint32_t id);

/**
 * \brief API to allocate AclId of AclCounter type
 *
 * \param [in] devId
 * \param [in] tableId
 * \param [out] result
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAclIdAllocCounterId(xpsDevice_t devId, uint32_t tableId,
                                 uint32_t* result);

/**
 * \brief API to free AclId of AclCounter type
 *
 * \param [in] devId
 * \param [in] tableId
 * \param [in] id
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAclIdFreeCounterId(xpsDevice_t devId, uint32_t tableId,
                                uint32_t id);

#ifdef __cplusplus
}
#endif

#endif // _openXpsAclIdMgr_h_

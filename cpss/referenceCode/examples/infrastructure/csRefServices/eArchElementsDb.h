/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* eArchElementsDb.h
*
* DESCRIPTION:
*  similar to file prvTgfBrgVplsBasicTest.c but with next changes:
*       1. supports 3 modes :
*           a. 'pop tag' - the mode that is tested in prvTgfBrgVplsBasicTest.c
*           b. 'raw mode' + 'QinQ terminal'(delete double Vlan)
*           c. 'tag mode' + 'add double vlan tag'
*
*       2. in RAW mode ethernet packets come with 2 tags and the passenger on MPLS
*           tunnels is without vlan tags.
*       3. in TAG mode the ethernet packets come with one tag but considered untagged
*           when become passenger on MPLS tunnels and so added additional 2 vlan tags.
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/
#ifndef __eArchElementsDb_h
#define __eArchElementsDb_h

#include <cpss/generic/cpssTypes.h>


/***********************************************/
/*         Definitions for entry management purposes only */
/***********************************************/

typedef enum
{
   VPN_EARCH_DB_TYPE_EVID_E,
   VPN_EARCH_DB_TYPE_EVIDX_E,
   VPN_EARCH_DB_TYPE_EPORT_E,
   VPN_EARCH_DB_TYPE_L2MLL_E,
   VPN_EARCH_DB_TYPE_TS_E,
   VPN_EARCH_DB_TYPE_TTI_E,
   VPN_EARCH_DB_TYPE_PCL_E,
   VPN_EARCH_DB_TYPE_MAX_E
}VPN_EARCH_DB_TYPE_ENT;


typedef enum
{
   VPN_INDEX_DB_OPERATION_ALLOC_E,  /* Alloc from DB, returns baseIndex and relative offset from base. */
   VPN_INDEX_DB_OPERATION_FREE_E,   /* Return to DB, recieve absolute entryIndex (base + offset) */
   VPN_INDEX_DB_OPERATION_RESET_E   /* Reset DB to un-initialized state.  */
}VPN_INDEX_DB_OPERATION_ENT;


/***********************************************/
/*         API                                          */
/***********************************************/

/**
* @internal infraIndexElementsDbInit function
* @endinternal
*
* @brief   Create and intialize DB for given db name - dbType.
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dbType            - data base name.
* @param[in] factor        - Number of lines in an entry.
* @param[in] firstIndex          - first index in the given range.
* @param[in] numOfElements  - Number of element to manage.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*
*/
GT_STATUS csRefInfraIndexDbInit
(
  IN VPN_EARCH_DB_TYPE_ENT  dbType,
  IN GT_U32                 factor,
  IN GT_U32                 firstIndex,
  IN GT_U32                 numOfElements
);


/**
* @internal csRefInfraIndexDbOp function
* @endinternal
*
* @brief   Set of operation to apply on the db represented by dbType.
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] op             - Operation to apply.
* @param[in] entryIndex  - ALLOC op: output allocated entry index.
*                                  - FREE  op: pointer to entry index to free.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*
* @retval GT_OK                     - on success
* @retval GT_NOT_INITIALIZED - DB was not initialized.
* @retval GT_FAIL                  - otherwise
*
*
*/
GT_STATUS csRefInfraIndexDbOp
(
  IN VPN_EARCH_DB_TYPE_ENT       dbType,
  IN VPN_INDEX_DB_OPERATION_ENT  op,
  IN OUT GT_U32                 *entryIndex
);



/**
* @internal csRefInfraIndexDbUserInfoSet function
* @endinternal
*
* @brief   Attach user info for given entry index in the db represented by dbType.
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dbType      - data base name.
* @param[in] entryIndex  - Absolute entry index, after adding baseIndex.
* @param[in] userInfoPtr - Pointer to user info element.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*
* @retval GT_OK                     - on success
* @retval GT_NOT_INITIALIZED - DB was not initialized.
* @retval GT_FAIL                  - otherwise
*
*
* @Note: DB management assumes that non valid user info pointer is NULL.
* @        Before free an element(csRefInfraIndexDbOp), set user info pointer to NULL.
*/
GT_STATUS csRefInfraIndexDbUserInfoSet
(
  IN VPN_EARCH_DB_TYPE_ENT      dbType,
  IN GT_U32                     entryIndex,
  IN void                      *userInfoPtr
);



/**
* @internal csRefInfraIndexDbUserInfoGet function
* @endinternal
*
* @brief   Return address to user info that was attached by user.
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dbType      - data base name.
* @param[in] entryIndex  - Absolute entry index, after adding baseIndex.
* @param[in] userInfoPtr - Pointer to user info element.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*
* @retval GT_OK                     - on success
* @retval GT_NOT_INITIALIZED - DB was not initialized.
* @retval GT_FAIL                  - otherwise
*
*
*/
GT_STATUS csRefInfraIndexDbUserInfoGet
(
  VPN_EARCH_DB_TYPE_ENT      dbType,
  GT_U32                     entryIndex,
  void                     **userInfoPtr
);

#endif


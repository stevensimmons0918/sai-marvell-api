/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* eArchElementsDb.c
*
* DESCRIPTION:
*
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/

#include "eArchElementsDb.h"

#include <gtOs/gtOsMem.h>

#include <cpss/generic/cpssTypes.h>


/*****************************************************/
/**************** Static Varaibles       *********************/
/*****************************************************/

#define REFERENCE_INVALID_INDEX_CNS  0xFFFFFFA0
#define REFERENCE_MAX_INDEX_CNS  0xFFFFFFF0


typedef struct{
   GT_U32     nextFreeIndex;
    void     *userInfoPtr; /* User additional infromation */
}REFCS_INDEX_ELEMENT_STC;


static GT_BOOL  dbInitDone[VPN_EARCH_DB_TYPE_MAX_E] = {GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE};
static GT_U32   dbCurrentFreeIndex[VPN_EARCH_DB_TYPE_MAX_E];
static REFCS_INDEX_ELEMENT_STC *dbElementsArray[VPN_EARCH_DB_TYPE_MAX_E];
static GT_U32   dbBaseIndex[VPN_EARCH_DB_TYPE_MAX_E];
static GT_U32   dbEntryFactor[VPN_EARCH_DB_TYPE_MAX_E];
static GT_U32   dbNumberOfElements[VPN_EARCH_DB_TYPE_MAX_E];
static GT_U32   dbNumberOfFreeElements[VPN_EARCH_DB_TYPE_MAX_E];

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
)
{
    GT_U32 i;

    if(dbInitDone[dbType] == GT_FALSE)
    {
      dbElementsArray[dbType] = osMalloc(numOfElements*sizeof(REFCS_INDEX_ELEMENT_STC));
      if(dbElementsArray[dbType] == NULL)
          return GT_OUT_OF_CPU_MEM;

      dbBaseIndex[dbType] = firstIndex;
      dbEntryFactor[dbType] = factor;
      dbCurrentFreeIndex[dbType] = 0;


      for(i=0 ; i<numOfElements ; i++)
        dbElementsArray[dbType][i].nextFreeIndex = i+1;
      dbElementsArray[dbType][numOfElements-1].nextFreeIndex = REFERENCE_MAX_INDEX_CNS;

      dbNumberOfElements[dbType] = numOfElements;
      dbNumberOfFreeElements[dbType] = numOfElements;
      dbInitDone[dbType] = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal csRefInfraIndexDbOp function
* @endinternal
*
* @brief   Set of operation to apply on the db represented by dbType.
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dbType      - data base name.
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
)
{
   GT_U32    baseIndex, index, factor;

   if(dbType >= VPN_EARCH_DB_TYPE_MAX_E)
       return GT_BAD_PARAM;

   if(dbInitDone[dbType] == GT_FALSE)
     return GT_NOT_INITIALIZED;

   baseIndex = dbBaseIndex[dbType];
   factor    = dbEntryFactor[dbType];

   /* allocate or free element */
   switch(op)
   {
     case VPN_INDEX_DB_OPERATION_RESET_E:
        dbInitDone[dbType] = GT_FALSE;
        break;

     case VPN_INDEX_DB_OPERATION_ALLOC_E:
       if(dbCurrentFreeIndex[dbType] == REFERENCE_MAX_INDEX_CNS)
         return GT_NO_RESOURCE;

       index = dbCurrentFreeIndex[dbType];
       dbCurrentFreeIndex[dbType] = dbElementsArray[dbType][dbCurrentFreeIndex[dbType]].nextFreeIndex;
       dbNumberOfFreeElements[dbType]--;
       /* Allocated entry has it nextFreeIndex set to invalid index constant */
       dbElementsArray[dbType][index].nextFreeIndex = REFERENCE_INVALID_INDEX_CNS;
       dbElementsArray[dbType][index].userInfoPtr   = NULL;
       *entryIndex = baseIndex + index*factor;
     break;

     case VPN_INDEX_DB_OPERATION_FREE_E:
       if(dbNumberOfFreeElements[dbType] == dbNumberOfElements[dbType])
         return GT_BAD_STATE; /* Attemt to free more element, than actual number.*/

       index = *entryIndex;

       if((index < baseIndex) || ((index%factor) != 0))
         return GT_BAD_STATE; /* Corruption in DB. */

       index = (index - baseIndex)/factor;
       if(index >= dbNumberOfElements[dbType])
         return GT_BAD_PARAM;/* Attemt to free index not in the range. */

       dbElementsArray[dbType][index].nextFreeIndex = dbCurrentFreeIndex[dbType];
       dbCurrentFreeIndex[dbType] = index;
       dbNumberOfFreeElements[dbType]++;
     break;

     default:
       return GT_BAD_PARAM;
   }

   return GT_OK;
}

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
)
{
    if(dbType >= VPN_EARCH_DB_TYPE_MAX_E)
     return GT_BAD_PARAM;

    if(dbInitDone[dbType] == GT_FALSE)
      return GT_NOT_INITIALIZED;

    /* Check that element allocated. */
    if(dbElementsArray[dbType][entryIndex-dbBaseIndex[dbType]].nextFreeIndex != REFERENCE_INVALID_INDEX_CNS)
        return GT_NOT_FOUND;

    dbElementsArray[dbType][entryIndex-dbBaseIndex[dbType]].userInfoPtr = userInfoPtr;

    return GT_OK;
}


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
)
{
    if(dbType >= VPN_EARCH_DB_TYPE_MAX_E)
     return GT_BAD_PARAM;

    if(dbInitDone[dbType] == GT_FALSE)
      return GT_NOT_INITIALIZED;

    /* Check that element allocated, hence user info may be valid. */
    if(dbElementsArray[dbType][entryIndex-dbBaseIndex[dbType]].nextFreeIndex != REFERENCE_INVALID_INDEX_CNS)
        return GT_NOT_FOUND;

    *userInfoPtr = dbElementsArray[dbType][entryIndex-dbBaseIndex[dbType]].userInfoPtr;

    return GT_OK;
}



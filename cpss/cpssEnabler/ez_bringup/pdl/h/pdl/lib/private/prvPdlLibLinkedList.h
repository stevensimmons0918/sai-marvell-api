/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\lib\private\prvpdlliblinkedlist.h.
 *
 * @brief   Declares the prvpdlliblinkedlist class
 */

#ifndef __prvPdlLibLinkedListLinkedListh

#define __prvPdlLibLinkedListLinkedListh
/**
********************************************************************************
 * @file prvPdlLibLinkedList.h   
 * @copyright
 *    (c), Copyright 2001, Marvell International Ltd.
 *    THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 *    NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *    OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 *    DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 *    THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
 *    IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
********************************************************************************
 * 
 * @brief Platform driver layer - Library private declarations and APIs
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/lib/private/prvPdlLib.h>

/**
 * @addtogroup Library
 * @{Library APIs
 */

/**
 * @defgroup Library_linked_list Linked List
 * @{Library APIs
 */

/**
 * @struct  PDL_DB_PRV_LIST_ENTRY_STCT
 *
 * @brief   defines structure for linked list entry
 */

typedef struct PDL_DB_PRV_LIST_ENTRY_STCT{

    /**
     * @struct  PDL_DB_PRV_LIST_ENTRY_STCT*
     *
     * @brief   A pdl database prv list entry stct*.
     */

    struct PDL_DB_PRV_LIST_ENTRY_STCT   * nextPtr;
    /** @brief   The data pointer */
    void                                * dataPtr;
    /** @brief   The key pointer */
    void                                * keyPtr;
    /** @brief   Size of the data */
    UINT_32                               dataSize;    
}PDL_DB_PRV_LIST_ENTRY_STC;

/**
 * @struct  PDL_DB_PRV_LIST_STC
 *
 * @brief   defines structure for linked list
 */

typedef struct  {
    /** @brief   The list name pointer */
    char                              * listNamePtr;  /* name - for debug */
    /** @brief   Size of the data */
    UINT_32                             dataSize;       /* size of entry, excluding next/prev pointers or other additional */
    /** @brief   Size of the key */
    UINT_32                             keySize;
    /** @brief   The head pointer */
    PDL_DB_PRV_LIST_ENTRY_STC         * headPtr;
    /** @brief   The tail pointer */
    PDL_DB_PRV_LIST_ENTRY_STC         * tailPtr;
    /** @brief   Size of the list */
    UINT_32                             listSize;       /* number of entries in the list*/
} PDL_DB_PRV_LIST_STC;

/**
 * @fn  PDL_STATUS prvPdlDbLinkedListGetNumOfEntries ( IN PDL_DB_PRV_STC * dbPtr, OUT UINT_32 * numPtr );
 *
 * @brief   Get Linked List size (number of entries)
 *
 * @param [in]  dbPtr   - pointer to Linked List.
 * @param [out] numPtr  - num of entries in linked list.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlDbLinkedListGetNumOfEntries (
    IN  PDL_DB_PRV_STC        * dbPtr,
    OUT UINT_32               * numPtr
);

/**
 * @fn  PDL_STATUS prvPdlDbLinkedListCreate ( IN PDL_DB_PRV_ATTRIBUTES_STC * dbAttributes, OUT PDL_DB_PRV_STC * dbPtr );
 *
 * @brief   Get Linked List size (number of entries)
 *
 * @param [in]  dbAttributes    - Linked list attributes.
 * @param [out] dbPtr           - pointer to created db structure.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlDbLinkedListCreate (
    IN  PDL_DB_PRV_ATTRIBUTES_STC   * dbAttributes,
    OUT PDL_DB_PRV_STC              * dbPtr
);

/**
 * @fn  PDL_STATUS prvPdlDbLinkedListAdd ( IN PDL_DB_PRV_STC * dbPtr, IN void * keyPtr, IN void * srcEntryPtr, OUT void ** outEntryPtrPtr );
 *
 * @brief   Add instance to linked list DB
 *
 * @param [in]  dbPtr           - pointer to DB.
 * @param [in]  keyPtr          - key pointer.
 * @param [in]  srcEntryPtr     - entry pointer.
 * @param [out] outEntryPtrPtr  - pointer to inserted entry.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlDbLinkedListAdd (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,     
    IN  void                  * srcEntryPtr,
    OUT void                 ** outEntryPtrPtr
);

/**
 * @fn  PDL_STATUS prvPdlDbLinkedListFind ( IN PDL_DB_PRV_STC * dbPtr, IN void * keyPtr, OUT void ** outEntryPtrPtr );
 *
 * @brief   Find instance in linked list DB
 *
 * @param [in]  dbPtr           - pointer to DB.
 * @param [in]  keyPtr          - pointer to searched key.
 * @param [out] outEntryPtrPtr  - pointer entry found in DB (or NULL if not found)
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlDbLinkedListFind (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,
    OUT void                 ** outEntryPtrPtr
);

/**
 * @fn  PDL_STATUS prvPdlDbLinkedListGetNext ( IN PDL_DB_PRV_STC * dbPtr, IN void * keyPtr, OUT void ** outEntryPtrPtr );
 *
 * @brief   Find next instance in linked list DB
 *
 * @param [in]  dbPtr           - pointer to DB.
 * @param [in]  keyPtr          - pointer to searched key (or NULL to get first entry)
 * @param [out] outEntryPtrPtr  - pointer entry found in DB (or NULL if not found)
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlDbLinkedListGetNext (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,
    OUT void                 ** outEntryPtrPtr
);

/**
 * @fn  PDL_STATUS prvPdlDbLinkedListGetFirst ( IN PDL_DB_PRV_STC * dbPtr, OUT void ** outEntryPtrPtr )
 *
 * @brief   find first entry in linked list
 *
 * @param [in,out]  dbPtr           If non-null, the database pointer.
 * @param [in,out]  outEntryPtrPtr  If non-null, the out entry pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbLinkedListGetFirst (
    IN  PDL_DB_PRV_STC        * dbPtr,
    OUT void                 ** outEntryPtrPtr
);

/*$ END OF prvPdlDbLinkedListGetFirst */

/*****************************************************************************
* FUNCTION NAME: prvPdlDbLinkedListGetNextKey
*
* DESCRIPTION:  find the key of the next entry
*      
*
*****************************************************************************/

PDL_STATUS prvPdlDbLinkedListGetNextKey (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,
    OUT void                  * nextKeyPtr
);

/*$ END OF prvPdlDbLinkedListGetNextKey */

/**
 * @fn  PDL_STATUS prvPdlDbLinkedListDestroy ( IN PDL_DB_PRV_STC * dbPtr )
 *
 * @brief   free memory used by list db
 *
 * @param [in,out]  dbPtr           If non-null, the database pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbLinkedListDestroy (
    IN  PDL_DB_PRV_STC        * dbPtr
);

/*$ END OF prvPdlDbLinkedListDestroy */

/* @}*/
/* @}*/

#endif
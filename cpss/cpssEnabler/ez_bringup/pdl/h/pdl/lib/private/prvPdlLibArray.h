/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\lib\private\prvpdllibarray.h.
 *
 * @brief   Declares the prvpdllibarray class
 */

#ifndef __prvPdlLibArrayh

#define __prvPdlLibArrayh
/**
********************************************************************************
 * @file prvPdlLibArray.h   
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
* @defgroup Library_array Array
* @{
*/

/**
 * @struct  PRV_PDL_DB_PRV_ARRAY_ENTRY_STCT
 *
 * @brief   defines structure for array entry
 */

typedef struct PRV_PDL_DB_PRV_ARRAY_ENTRY_STCT{
    /** @brief   True if is valid, false if not */
    BOOLEAN                             isValid;            /* is entry valid (was added)   */
    /** @brief   The key pointer */
    void                               *keyPtr;             /* key data ptr                 */
    /** @brief   The entry pointer */
    void                               *entryPtr;           /* entry data ptr               */
}PRV_PDL_DB_PRV_ARRAY_ENTRY_STC;

/**
 * @struct  PRV_PDL_DB_PRV_ARRAY_STC
 *
 * @brief   defines structure for array db private data
 */

typedef struct  {
    /** @brief   Number of entries */
    UINT_32                             numOfEntries;       /* number of entries in the array         */
    /** @brief   Size of the key */
    UINT_32                             keySize;            /* size of key                            */
    /** @brief   Size of the entry */
    UINT_32                             entrySize;          /* size of entry                          */
    /** @brief   Number of valids */
    UINT_32                             validCount;         /* number of valid (Set) entries          */
    /** @brief   The entries pointer */
    PRV_PDL_DB_PRV_ARRAY_ENTRY_STC    * entriesPtr;         /* array private data                     */
    /** @brief   The memory pointer */
    UINT_8                            * memoryPtr;          /* array application data memory          */
    /** @brief   The key to index function pointer */
    pdlArrayKeyToIndex_FUN            * keyToIndexFuncPtr;  /* function to convert key to array index */
} PRV_PDL_DB_PRV_ARRAY_STC;

/**
 * @fn  PDL_STATUS prvPdlDbArrayCreate ( IN PDL_DB_PRV_ATTRIBUTES_STC * dbAttributes, OUT PDL_DB_PRV_STC * dbPtr );
 *
 * @brief   Create array db
 *
 * @param [in]  dbAttributes    - Array attributes.
 * @param [out] dbPtr           - pointer to created db structure.
 *
 * @return  PDL_NO_RESOURCE if memory allocation failed.
 */

PDL_STATUS prvPdlDbArrayCreate (
    IN  PDL_DB_PRV_ATTRIBUTES_STC   * dbAttributes,
    OUT PDL_DB_PRV_STC              * dbPtr
);

/**
 * @fn  PDL_STATUS prvPdlDbArrayAdd ( IN PDL_DB_PRV_STC * dbPtr, IN void * keyPtr, IN void * srcEntryPtr, OUT void ** outEntryPtrPtr );
 *
 * @brief   Add instance to array DB
 *
 * @param [in]  dbPtr           - pointer to DB.
 * @param [in]  keyPtr          - key pointer.
 * @param [in]  srcEntryPtr     - entry pointer.
 * @param [out] outEntryPtrPtr  - pointer to inserted entry.
 *
 * @return  PDL_BAD_PTR if one of the supplied pointer invalid.
 * @return  PDL_NOT_INITIALIZED if db wasn't created.
 * @return  PDL_FULL if array is maxed out.
 * @return  PDL_OUT_OF_RANGE if key converted to index is more than number of entries in array.
 * @return  PDL_ALREADY_EXIST if trying to add existing entry.
 */

PDL_STATUS prvPdlDbArrayAdd (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,     
    IN  void                  * srcEntryPtr,
    OUT void                 ** outEntryPtrPtr
);

/**
 * @fn  PDL_STATUS prvPdlDbArrayFind ( IN PDL_DB_PRV_STC * dbPtr, IN void * keyPtr, OUT void ** outEntryPtrPtr );
 *
 * @brief   Find instance in array DB
 *
 * @param [in]  dbPtr           - pointer to DB.
 * @param [in]  keyPtr          - pointer to searched key.
 * @param [out] outEntryPtrPtr  - pointer entry found in DB.
 *
 * @return  PDL_BAD_PTR if one of the supplied pointer invalid.
 * @return  PDL_NOT_INITIALIZED if db wasn't created.
 * @return  PDL_EMPTY if array is empty.
 * @return  PDL_OUT_OF_RANGE if key converted to index is more than number of entries in array.
 * @return  PDL_NO_SUCH if invalid entry was found or keys don't match.
 */

PDL_STATUS prvPdlDbArrayFind (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,
    OUT void                 ** outEntryPtrPtr
);

/**
 * @fn  PDL_STATUS prvPdlDbArrayGetNext ( IN PDL_DB_PRV_STC * dbPtr, IN void * keyPtr, OUT void ** outEntryPtrPtr );
 *
 * @brief   Find next instance in array DB
 *
 * @param [in]  dbPtr           - pointer to DB.
 * @param [in]  keyPtr          - pointer to searched key (or NULL to get first)
 * @param [out] outEntryPtrPtr  - pointer entry found in DB.
 *
 * @return  PDL_BAD_PTR if one of the supplied pointer invalid.
 * @return  PDL_NOT_INITIALIZED if db wasn't created.
 * @return  PDL_EMPTY if array is empty.
 * @return  PDL_OUT_OF_RANGE if key converted to index is more than number of entries in array.
 * @return  PDL_NO_SUCH if invalid entry was found or keys don't match.
 * @return  PDL_NO_MORE if last entry reached.
 */

PDL_STATUS prvPdlDbArrayGetNext (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,
    OUT void                 ** outEntryPtrPtr
);

/**
 * @fn  PDL_STATUS prvPdlDbArrayGetNumOfEntries ( IN PDL_DB_PRV_STC * dbPtr, OUT UINT_32 * numPtr );
 *
 * @brief   Get array size (number of entries)
 *
 * @param [in]  dbPtr   - pointer to Array.
 * @param [out] numPtr  - number of valid entries in Array.
 *
 * @return  PDL_BAD_PTR if one of the supplied pointer invalid.
 * @return  PDL_NOT_INITIALIZED if db wasn't created.
 */

PDL_STATUS prvPdlDbArrayGetNumOfEntries (
    IN  PDL_DB_PRV_STC        * dbPtr,
    OUT UINT_32               * numPtr
);

/**
 * @fn  PDL_STATUS prvPdlDbArrayGetFirst ( IN PDL_DB_PRV_STC * dbPtr, OUT void ** outEntryPtrPtr )
 *
 * @brief   find first entry in array
 *
 * @param [in,out]  dbPtr           If non-null, the database pointer.
 * @param [in,out]  outEntryPtrPtr  If non-null, the out entry pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbArrayGetFirst (
    IN  PDL_DB_PRV_STC        * dbPtr,
    OUT void                 ** outEntryPtrPtr
);

/**
 * @fn  PDL_STATUS prvPdlDbArrayDestroy ( IN PDL_DB_PRV_STC * dbPtr )
 *
 * @brief   free memory used by array db
 *
 * @param [in,out]  dbPtr           If non-null, the database pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbArrayDestroy (
    IN  PDL_DB_PRV_STC        * dbPtr
);

/* @}*/
/* @}*/

#endif
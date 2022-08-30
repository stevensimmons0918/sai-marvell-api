/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlLibLinkedList.c
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
 * @brief Platform driver layer - Lib related API
 * 
 * @version   1 
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/lib/private/prvPdlLibLinkedList.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*****************************************************************************
* FUNCTION NAME: prvPdlDbLinkedListCreate
*
* DESCRIPTION:  create a new list
*      
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlDbLinkedListCreate ( IN PDL_DB_PRV_ATTRIBUTES_STC * dbAttributes, OUT PDL_DB_PRV_STC * dbPtr )
 *
 * @brief   Prv pdl database linked list create
 *
 * @param [in,out]  dbAttributes    If non-null, the database attributes.
 * @param [in,out]  dbPtr           If non-null, the database pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbLinkedListCreate (
    IN  PDL_DB_PRV_ATTRIBUTES_STC   * dbAttributes,
    OUT PDL_DB_PRV_STC              * dbPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_DB_PRV_LIST_ATTRIBUTES_STC          * listAttributesPtr;
    PDL_DB_PRV_LIST_STC                     * listDbPtr;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    listAttributesPtr = (PDL_DB_PRV_LIST_ATTRIBUTES_STC*)&dbAttributes->listAttributes;

    dbPtr->dbType = PDL_DB_PRV_TYPE_LIST_E;
    listDbPtr = (PDL_DB_PRV_LIST_STC*) prvPdlOsMalloc(sizeof(PDL_DB_PRV_LIST_STC));
    if (listDbPtr == NULL) {
        return PDL_NO_RESOURCE;
    }
    dbPtr->dbPtr = (void*) listDbPtr;
    listDbPtr->dataSize = listAttributesPtr->entrySize;
    listDbPtr->keySize = listAttributesPtr->keySize;
    listDbPtr->headPtr = listDbPtr->tailPtr = NULL;
    listDbPtr->listSize = 0;
    return PDL_OK;
}
/*$ END OF prvPdlDbLinkedListCreate */

/*****************************************************************************
* FUNCTION NAME: prvPdlDbLinkedListAddAllocated
*
* DESCRIPTION:  adds an allocated entry to list               
*      
*
*****************************************************************************/

/**
 * @fn  static PDL_STATUS prvPdlDbLinkedListAddAllocated ( IN PDL_DB_PRV_LIST_STC * listDbPtr, IN void * srcEntryPtr, IN UINT_32 dataSize, OUT PDL_DB_PRV_LIST_ENTRY_STC * addedEntryPtr )
 *
 * @brief   Prv pdl database linked list add allocated
 *
 * @param [in,out]  listDbPtr       If non-null, the list database pointer.
 * @param [in,out]  srcEntryPtr     If non-null, source entry pointer.
 * @param           dataSize        Size of the data.
 * @param [in,out]  addedEntryPtr   If non-null, the added entry pointer.
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlDbLinkedListAddAllocated (
    IN  PDL_DB_PRV_LIST_STC         * listDbPtr,
    IN  void                        * srcEntryPtr,
    IN  UINT_32                       dataSize,
    OUT PDL_DB_PRV_LIST_ENTRY_STC   * addedEntryPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (listDbPtr == NULL || srcEntryPtr == NULL || addedEntryPtr == NULL) {
        return PDL_BAD_PTR;
    }
    if (dataSize == 0) {
        return PDL_BAD_PARAM;
    }

    /* add to end of list */
    if (listDbPtr->tailPtr != NULL) {
        listDbPtr->tailPtr->nextPtr = addedEntryPtr;
    }
    listDbPtr->tailPtr = addedEntryPtr;
    /* first entry */
    if (listDbPtr->headPtr == NULL) {
        listDbPtr->headPtr = addedEntryPtr;
    }

    listDbPtr->listSize++;

    return PDL_OK;
   
}
/*$ END OF prvPdlDbLinkedListAddAllocated */



/*****************************************************************************
* FUNCTION NAME: prvPdlDbLinkedListAdd
*
* DESCRIPTION:  allocate space for entry
*               add entry to end of list
*               return pointer to this entry
*      
*
*****************************************************************************/

/**
 * @fn  static PDL_STATUS prvPdlDbLinkedListAddWithSize ( IN PDL_DB_PRV_STC * dbPtr, IN void * keyPtr, IN void * srcEntryPtr, IN UINT_32 dataSize, OUT void ** outEntryPtrPtr )
 *
 * @brief   Prv pdl database linked list add with size
 *
 * @param [in,out]  dbPtr           If non-null, the database pointer.
 * @param [in,out]  keyPtr          If non-null, the key pointer.
 * @param [in,out]  srcEntryPtr     If non-null, source entry pointer.
 * @param           dataSize        Size of the data.
 * @param [in,out]  outEntryPtrPtr  If non-null, the out entry pointer.
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlDbLinkedListAddWithSize (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,             
    IN  void                  * srcEntryPtr,
    IN  UINT_32                 dataSize,
    OUT void                 ** outEntryPtrPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_STATUS                            pdlRc;
    PDL_DB_PRV_LIST_STC                 * listDbPtr;
    PDL_DB_PRV_LIST_ENTRY_STC           * addedEntryPtr;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (dataSize == 0) {
        return PDL_BAD_PARAM;
    }
    if (dbPtr == NULL || srcEntryPtr == NULL || outEntryPtrPtr == NULL) {
        return PDL_BAD_PTR;
    }

    listDbPtr = (PDL_DB_PRV_LIST_STC*) dbPtr->dbPtr;
    if (listDbPtr == NULL) {
        return PDL_NOT_INITIALIZED;
    }
    /* allocate entry */
    addedEntryPtr = (PDL_DB_PRV_LIST_ENTRY_STC*) prvPdlOsMalloc (sizeof(PDL_DB_PRV_LIST_ENTRY_STC));
    if (addedEntryPtr == NULL) {
        return PDL_NO_RESOURCE;
    }
    
    /* allocate data for entry */
    addedEntryPtr->dataPtr = prvPdlOsMalloc (dataSize);
    if (addedEntryPtr->dataPtr == NULL) {
        free(addedEntryPtr);
        return PDL_NO_RESOURCE;
    }

    addedEntryPtr->keyPtr = prvPdlOsMalloc (listDbPtr->keySize);
    if (addedEntryPtr->keyPtr == NULL) {
        free(addedEntryPtr->dataPtr);
        free(addedEntryPtr);
        return PDL_NO_RESOURCE;
    }

    memcpy(addedEntryPtr->dataPtr, srcEntryPtr, dataSize);
    memcpy(addedEntryPtr->keyPtr, keyPtr, listDbPtr->keySize);
    addedEntryPtr->dataSize = dataSize;
    addedEntryPtr->nextPtr = NULL;

    pdlRc = prvPdlDbLinkedListAddAllocated(listDbPtr, srcEntryPtr, dataSize, addedEntryPtr);
    PDL_CHECK_STATUS(pdlRc);
    *outEntryPtrPtr = addedEntryPtr;
    
    return PDL_OK;
}
/*$ END OF prvPdlDbLinkedListAdd */

/*****************************************************************************
* FUNCTION NAME: prvPdlDbLinkedListAdd
*
* DESCRIPTION:  allocate space for entry
*               add entry to end of list
*               return pointer to this entry
*      
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlDbLinkedListAdd ( IN PDL_DB_PRV_STC * dbPtr, IN void * keyPtr, IN void * srcEntryPtr, OUT void ** outEntryPtrPtr )
 *
 * @brief   Prv pdl database linked list add
 *
 * @param [in,out]  dbPtr           If non-null, the database pointer.
 * @param [in,out]  keyPtr          If non-null, the key pointer.
 * @param [in,out]  srcEntryPtr     If non-null, source entry pointer.
 * @param [in,out]  outEntryPtrPtr  If non-null, the out entry pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbLinkedListAdd (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,             
    IN  void                  * srcEntryPtr,
    OUT void                 ** outEntryPtrPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_DB_PRV_LIST_STC                 * listDbPtr;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (dbPtr == NULL || srcEntryPtr == NULL || outEntryPtrPtr == NULL) {
        return PDL_BAD_PTR;
    }
    listDbPtr = (PDL_DB_PRV_LIST_STC*)dbPtr->dbPtr;
    return prvPdlDbLinkedListAddWithSize(dbPtr, keyPtr, srcEntryPtr, listDbPtr->dataSize, outEntryPtrPtr);
}
/*$ END OF prvPdlDbLinkedListAdd */

/*****************************************************************************
* FUNCTION NAME: pdlPrvDbLinkedListGetSize
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlDbLinkedListGetNumOfEntries ( IN PDL_DB_PRV_STC * dbPtr, OUT UINT_32 * numPtr )
 *
 * @brief   Prv pdl database linked list get number of entries
 *
 * @param [in,out]  dbPtr   If non-null, the database pointer.
 * @param [in,out]  numPtr  If non-null, number of pointers.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbLinkedListGetNumOfEntries (
    IN  PDL_DB_PRV_STC        * dbPtr,
    OUT UINT_32               * numPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_DB_PRV_LIST_STC                 * listDbPtr;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (dbPtr == NULL) {
        return PDL_BAD_PTR;
    }
    listDbPtr = (PDL_DB_PRV_LIST_STC*) dbPtr->dbPtr;
    if (listDbPtr == NULL){
        return PDL_NOT_INITIALIZED;
    }
    *numPtr = listDbPtr->listSize;

    return PDL_OK;
}
/*$ END OF pdlPrvDbLinkedListGetSize */

/*****************************************************************************
* FUNCTION NAME: prvPdlDbLinkedListFind
*
* DESCRIPTION:  find an entry based on key comparison (entry MUST include key at start)
*      
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlDbLinkedListFind ( IN PDL_DB_PRV_STC * dbPtr, IN void * keyPtr, OUT void ** outEntryPtrPtr )
 *
 * @brief   Prv pdl database linked list find
 *
 * @param [in,out]  dbPtr           If non-null, the database pointer.
 * @param [in,out]  keyPtr          If non-null, the key pointer.
 * @param [in,out]  outEntryPtrPtr  If non-null, the out entry pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbLinkedListFind (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,
    OUT void                 ** outEntryPtrPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_DB_PRV_LIST_STC                 * listDbPtr;
    PDL_DB_PRV_LIST_ENTRY_STC           * entryPtr = NULL;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (dbPtr == NULL || keyPtr == NULL || outEntryPtrPtr == NULL) {
        return PDL_BAD_PTR;
    }
    listDbPtr = (PDL_DB_PRV_LIST_STC*) dbPtr->dbPtr;
    if (listDbPtr == NULL){
        return PDL_NOT_INITIALIZED;
    }

    entryPtr = listDbPtr->headPtr;
    while(entryPtr) {
        if (memcmp(entryPtr->keyPtr, keyPtr,listDbPtr->keySize) == 0) {
            break;
        }
        entryPtr = entryPtr->nextPtr;
    }

    if (entryPtr == NULL) {
        *outEntryPtrPtr = NULL;
        return PDL_NOT_FOUND;
    }
    *outEntryPtrPtr = entryPtr->dataPtr;
    return PDL_OK;  
}
/*$ END OF prvPdlDbLinkedListFind */

/*****************************************************************************
* FUNCTION NAME: prvPdlDbLinkedListGetNext
*
* DESCRIPTION:  find an entry based on key comparison (entry MUST include key at start)
*      
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlDbLinkedListGetNext ( IN PDL_DB_PRV_STC * dbPtr, IN void * keyPtr, OUT void ** outEntryPtrPtr )
 *
 * @brief   Prv pdl database linked list get next
 *
 * @param [in,out]  dbPtr           If non-null, the database pointer.
 * @param [in,out]  keyPtr          If non-null, the key pointer.
 * @param [in,out]  outEntryPtrPtr  If non-null, the out entry pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlDbLinkedListGetNext (
    IN  PDL_DB_PRV_STC        * dbPtr,
    IN  void                  * keyPtr,
    OUT void                 ** outEntryPtrPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_DB_PRV_LIST_STC                 * listDbPtr;
    PDL_DB_PRV_LIST_ENTRY_STC           * entryPtr = NULL;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    
    if (dbPtr == NULL || outEntryPtrPtr == NULL) {
        return PDL_BAD_PTR;
    }

    listDbPtr = (PDL_DB_PRV_LIST_STC*) dbPtr->dbPtr;
    if (listDbPtr == NULL)
    {
        return PDL_NOT_INITIALIZED;
    }

    entryPtr = listDbPtr->headPtr;

    if (keyPtr)
    {
        while(entryPtr)
        {
            if (memcmp(entryPtr->keyPtr, keyPtr,listDbPtr->keySize) == 0)
            {
                entryPtr = entryPtr->nextPtr;
                break;
            }
            entryPtr = entryPtr->nextPtr;
        }
    }

    if (entryPtr == NULL) {
        *outEntryPtrPtr = NULL;
        return PDL_NO_MORE;
    }

    *outEntryPtrPtr = entryPtr->dataPtr;
    return PDL_OK;  
}
/*$ END OF prvPdlDbLinkedListGetNext */

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
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_DB_PRV_LIST_STC                 * listDbPtr;
    PDL_DB_PRV_LIST_ENTRY_STC           * entryPtr = NULL;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    
    if (dbPtr == NULL || nextKeyPtr == NULL) {
        return PDL_BAD_PTR;
    }

    listDbPtr = (PDL_DB_PRV_LIST_STC*) dbPtr->dbPtr;
    if (listDbPtr == NULL)
    {
        return PDL_NOT_INITIALIZED;
    }

    entryPtr = listDbPtr->headPtr;

    if (keyPtr)
    {
        while(entryPtr)
        {
            if (memcmp(entryPtr->keyPtr, keyPtr,listDbPtr->keySize) == 0)
            {
                entryPtr = entryPtr->nextPtr;
                break;
            }
            entryPtr = entryPtr->nextPtr;
        }
    }

    if (entryPtr == NULL) {
        return PDL_NO_MORE;
    }

    memcpy (nextKeyPtr, entryPtr->keyPtr, listDbPtr->keySize);
    return PDL_OK;  
}

/*$ END OF prvPdlDbLinkedListGetNextKey */

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
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_DB_PRV_LIST_STC                 * listDbPtr;
    PDL_DB_PRV_LIST_ENTRY_STC           * entryPtr = NULL;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    
    if (dbPtr == NULL || outEntryPtrPtr == NULL) {
        return PDL_BAD_PTR;
    }

    listDbPtr = (PDL_DB_PRV_LIST_STC*) dbPtr->dbPtr;
    if (listDbPtr == NULL)
    {
        return PDL_NOT_INITIALIZED;
    }

    entryPtr = listDbPtr->headPtr;
    if (entryPtr != NULL) {
        *outEntryPtrPtr = entryPtr->dataPtr;
        return PDL_OK;
    }

    return PDL_NOT_FOUND;  
}
/*$ END OF prvPdlDbLinkedListGetFirst */

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
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_DB_PRV_LIST_STC                 * listDbPtr;
    PDL_DB_PRV_LIST_ENTRY_STC           * entryPtr = NULL, * nextEntryPtr = NULL;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    if (dbPtr == NULL) {
        return PDL_BAD_PTR;
    }

    listDbPtr = (PDL_DB_PRV_LIST_STC*) dbPtr->dbPtr;
    if (listDbPtr == NULL)
    {
        return PDL_NOT_INITIALIZED;
    }

    entryPtr = listDbPtr->headPtr;
    while (entryPtr) {
        nextEntryPtr = entryPtr->nextPtr;

        prvPdlOsFree(entryPtr->dataPtr);
        prvPdlOsFree(entryPtr->keyPtr);
        prvPdlOsFree(entryPtr);
        entryPtr = nextEntryPtr;
    }

    prvPdlOsFree (listDbPtr);
    dbPtr->dbPtr = NULL;
    return PDL_OK;
}

/*$ END OF prvPdlDbLinkedListDestroy */

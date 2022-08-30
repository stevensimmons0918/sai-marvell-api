/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\lib\private\prvpdllib.h.
 *
 * @brief   Declares the prvpdllib class
 */

#ifndef __prvPdlLibh

#define __prvPdlLibh
/**
********************************************************************************
 * @file prvPdlLib.h
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
#include <pdl/init/pdlInit.h>
#include <pdl/sensor/pdlSensor.h>
#include <pdl/fan/pdlFan.h>

/**
 * @defgroup Library Library
 * @{Library APIs
 */

extern BOOLEAN prvPdlInitDone;
extern BOOLEAN prvPdlBtnDebugFlag;
extern BOOLEAN prvPdlInitDebugFlag;
extern BOOLEAN prvPdlPpDebugFlag;
extern BOOLEAN prvPdlSfpDebugFlag;
extern BOOLEAN prvPdlSerdesDebugFlag;
extern BOOLEAN prvPdlOobPortDebugFlag;
extern BOOLEAN prvPdlFanDebugFlag;
extern BOOLEAN prvPdlSensorDebugFlag;
extern BOOLEAN prvPdlPowerDebugFlag;
extern BOOLEAN prvPdlPhyDebugFlag;
extern BOOLEAN prvPdlLedDebugFlag;
extern BOOLEAN prvPdlCpuDebugFlag;
extern BOOLEAN prvPdlLibDebugFlag;
extern BOOLEAN prvPdlInterfaceDebugFlag;
extern BOOLEAN prvPdlFanControllerDebugFlag;
extern BOOLEAN prvPdlInitDebugFlag;
extern BOOLEAN prvPdlI2CInterfaceMuxDebugFlag;


/**
 * Return in case status != PDL_OK
 * If debug flag enabled, prints debug info
 */
#define PDL_DEBUG_STATUS(__status, __flag) if (__status==PDL_OK && __flag) prvPdlDebugRawLog
#define PDL_LIB_DEBUG_MAC(__id) if (__id) prvPdlDebugRawLog

/**
 * Return in case status != XML_PARSER_RET_CODE_OK
 * If debug flag enabled, prints debug info
 */
#define XML_DEBUG_STATUS(__status, __flag) if (__status==XML_PARSER_RET_CODE_OK  && __flag) prvPdlDebugRawLog
#define PDL_XML_BOARD_DESC_LEN 200

/**
 * @enum    PDL_DB_PRV_TYPE_ENT
 *
 * @brief   Enumerator for DB types
 */

typedef enum {
    PDL_DB_PRV_TYPE_ARRAY_E,
    PDL_DB_PRV_TYPE_LIST_E,
    PDL_DB_PRV_TYPE_HASH_E,
    PDL_DB_PRV_TYPE_LAST_E
} PDL_DB_PRV_TYPE_ENT;

/**
 * @struct  PRV_PDL_LIB_STR_TO_ENUM_STC
 *
 * @brief   defines structure used to convert pair name to number
 */

typedef struct {
    /** @brief   The name pointer */
    char                   *namePtr;
    /** @brief   The value */
    UINT_32                 value;
} PRV_PDL_LIB_STR_TO_ENUM_STC;

/**
 * @struct  PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC
 *
 * @brief   defines structure used to convert enumerations to names
 */

typedef struct {
    /** @brief   The 2 enum */
    PRV_PDL_LIB_STR_TO_ENUM_STC     * str2Enum;
    /** @brief   The size */
    UINT_32                           size;
} PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC;

/**
 * @struct  PDL_DB_PRV_STC
 *
 * @brief   defines structure used by DB APIs
 */

typedef struct {
    /** @brief   Type of the database */
    PDL_DB_PRV_TYPE_ENT                 dbType;
    /** @brief   The database pointer */
    void                              * dbPtr;
} PDL_DB_PRV_STC;

/**
 * @typedef UINT_32 pdlArrayKeyToIndex_FUN (void* key)
 *
 * @brief  Array key to index
 */

/**
 * @typedef UINT_32 pdlArrayKeyToIndex_FUN (void* key)
 *
 * @brief   Defines an alias representing the key
 */

typedef UINT_32 pdlArrayKeyToIndex_FUN (void* key);

/**
 * @struct  PDL_DB_PRV_ARRAY_ATTRIBUTES_STC
 *
 * @brief   defines attributes that should be supplied for array creation
 */

typedef struct {
    /** @brief   Size of the key */
    UINT_32                              keySize;
    /** @brief   Number of entries */
    UINT_32                              numOfEntries;
    /** @brief   Size of the entry */
    UINT_32                              entrySize;
    /** @brief   The key to index function */
    pdlArrayKeyToIndex_FUN             * keyToIndexFunc;
} PDL_DB_PRV_ARRAY_ATTRIBUTES_STC;

/**
 * @struct  PDL_DB_PRV_HASH_ATTRIBUTES_STC
 *
 * @brief   defines attributes that should be supplied for hash creation
 */

typedef struct {
    /** @brief   Size of the key */
    UINT_32                         keySize;
    /** @brief   Size of the entry */
    UINT_32                         entrySize;
    /** @brief   The minimum number of entries */
    UINT_32                         minNumOfEntries;
    /** @brief   The maximum number of entries */
    UINT_32                         maxNumOfEntries;
} PDL_DB_PRV_HASH_ATTRIBUTES_STC;

/**
 * @struct  PDL_DB_PRV_LIST_ATTRIBUTES_STC
 *
 * @brief   defines attributes that should be supplied for list creation
 */

typedef struct {
    /** @brief   Size of the key */
    UINT_32                              keySize;
    /** @brief   Size of the entry */
    UINT_32                              entrySize;
} PDL_DB_PRV_LIST_ATTRIBUTES_STC;

/**
 * @union   PDL_DB_PRV_ATTRIBUTES_STC
 *
 * @brief   union of all db creation attributes
 */

typedef union {
    /** @brief   The list attributes */
    PDL_DB_PRV_LIST_ATTRIBUTES_STC      listAttributes;
    /** @brief   The array attributes */
    PDL_DB_PRV_ARRAY_ATTRIBUTES_STC     arrayAttributes;
    /** @brief   The hash attributes */
    PDL_DB_PRV_HASH_ATTRIBUTES_STC      hashAttributes;
} PDL_DB_PRV_ATTRIBUTES_STC;

/* typedefs used in array of generic db function callbacks */

/**
 * @typedef PDL_STATUS prvPdlDbCreate_FUN ( IN PDL_DB_PRV_ATTRIBUTES_STC * dbAttributes, OUT PDL_DB_PRV_STC * dbPtr )
 *
 * @brief  PDL create function prororype
 */

/**
 * @typedef PDL_STATUS prvPdlDbCreate_FUN ( IN PDL_DB_PRV_ATTRIBUTES_STC * dbAttributes, OUT PDL_DB_PRV_STC * dbPtr )
 *
 * @brief   Defines an alias representing the database pointer
 */

typedef PDL_STATUS prvPdlDbCreate_FUN (
    IN  PDL_DB_PRV_ATTRIBUTES_STC   * dbAttributes,
    OUT PDL_DB_PRV_STC              * dbPtr
);


/*! Add function creation */
#define prvPdlDbAddStubMac(__func_name)             \
    PDL_STATUS __func_name (                        \
    IN  PDL_DB_PRV_STC        * dbPtr,              \
    IN  void                  * keyPtr,             \
    IN  void                  * entryPtr,           \
    OUT void                 ** outEntryPtrPtr      \
)


/*! Add function creation */
#define prvPdlDbFindStubMac(__func_name)            \
    PDL_STATUS __func_name (                        \
    IN  PDL_DB_PRV_STC        * dbPtr,              \
    IN  void                  * keyPtr,             \
    OUT void                 ** outEntryPtrPtr      \
)


/*! Get next function creation */
#define prvPdlDbGetNextStubMac(__func_name)         \
    PDL_STATUS __func_name (                        \
    IN  PDL_DB_PRV_STC        * dbPtr,              \
    IN  void                  * keyPtr,             \
    OUT void                 ** outEntryPtrPtr      \
)

/*! Get next key function creation */
#define prvPdlDbGetNextKeyStubMac(__func_name)         \
    PDL_STATUS __func_name (                        \
    IN  PDL_DB_PRV_STC        * dbPtr,              \
    IN  void                  * keyPtr,             \
    OUT void                  * nextKeyPtr          \
)

/*! Get number of entries function creation */
#define prvPdlDbGetNumOfEntriesStubMac(__func_name) \
    PDL_STATUS __func_name (                        \
    IN  PDL_DB_PRV_STC        * dbPtr,              \
    OUT UINT_32               * numPtr              \
)

/*! Get first entry function creation */
#define prvPdlDbGetFirstStubMac(__func_name) \
    PDL_STATUS __func_name (                        \
    IN  PDL_DB_PRV_STC        * dbPtr,              \
    OUT void                 ** outEntryPtrPtr      \
)

/*!destroy function creation */
#define prvPdlDbDestroyStubMac(__func_name)         \
    PDL_STATUS __func_name (                        \
    IN  PDL_DB_PRV_STC        * dbPtr               \
)

/**
 * @typedef prvPdlDbAddStubMac(prvPdlDbAdd_FUN)
 *
 * @brief  Add to DB function
 */

/**
 * @typedef prvPdlDbAddStubMac(prvPdlDbAdd_FUN)
 *
 * @brief   Defines an alias representing the prv pdl database add fun
 */

typedef prvPdlDbAddStubMac(prvPdlDbAdd_FUN);

/**
 * @typedef prvPdlDbFindStubMac(prvPdlDbFind_FUN)
 *
 * @brief  Find in DB function
 */

/**
 * @typedef prvPdlDbFindStubMac(prvPdlDbFind_FUN)
 *
 * @brief   Defines an alias representing the prv pdl database find fun
 */

typedef prvPdlDbFindStubMac(prvPdlDbFind_FUN);

/**
 * @typedef prvPdlDbGetNextStubMac(prvPdlDbGetNext_FUN)
 *
 * @brief  Gen Next in DB function
 */

/**
 * @typedef prvPdlDbGetNextStubMac(prvPdlDbGetNext_FUN)
 *
 * @brief   Defines an alias representing the prv pdl database get next fun
 */

typedef prvPdlDbGetNextStubMac(prvPdlDbGetNext_FUN);
typedef prvPdlDbGetNextKeyStubMac(prvPdlDbGetNextKey_FUN);

/**
 * @typedef prvPdlDbGetNumOfEntriesStubMac(prvPdlDbGetNumOfEntries_FUN)
 *
 * @brief  Get number of entries function
 */

/**
 * @typedef prvPdlDbGetNumOfEntriesStubMac(prvPdlDbGetNumOfEntries_FUN)
 *
 * @brief   Defines an alias representing the prv pdl database get number of entries fun
 */

typedef prvPdlDbGetNumOfEntriesStubMac(prvPdlDbGetNumOfEntries_FUN);


/**
 * @typedef prvPdlDbGetFirstStubMac(prvPdlDbGetFirstStubMac)
 *
 * @brief   Defines an alias representing the prv pdl database get first
 */

typedef prvPdlDbGetFirstStubMac(prvPdlDbGetFirstStubMac_FUN);

typedef prvPdlDbDestroyStubMac(prvPdlDbDestroy_FUN);


/**
 * @struct  PDL_DB_PRV_FUNC_STC
 *
 * @brief   defines functions that MUST be implemented by a pdl db library
 */

typedef struct {
    /** @brief   The database create fun */
    prvPdlDbCreate_FUN            * dbCreateFun;
    /** @brief   The database add fun */
    prvPdlDbAdd_FUN               * dbAddFun;
    /** @brief   The database find fun */
    prvPdlDbFind_FUN              * dbFindFun;
    /** @brief   The database get next fun */
    prvPdlDbGetNext_FUN           * dbGetNextFun;
    /** @brief   The database get number of entries fun */
    prvPdlDbGetNumOfEntries_FUN   * dbGetNumOfEntriesFun;
    prvPdlDbGetFirstStubMac_FUN   * dbGetFirstFun;
    prvPdlDbGetNextKey_FUN        * dbGetNextKeyFun;
    prvPdlDbDestroy_FUN           * dbDestroyFun;
} PDL_DB_PRV_FUNC_STC;

/**
 * @fn  void prvPdlStatusDebugLogHdr ( IN const char * func_name_PTR, IN const char * line_num, IN PDL_STATUS   status, IN UINT_32  pdlIdentifier)
 *
 * @brief   Prv pdl debug log
 *
 * @param   func_name_PTR          The function name pointer.
 * @param   line_num               Describes the format to use.
 * @param   status                 error code.
 * @param   pdlIdentifier          identifier for the status type.
 */

extern BOOLEAN prvPdlStatusDebugLogHdr (
    IN   const char * func_name_PTR,
    IN   UINT_32      line_num,
    IN   PDL_STATUS   status,
    IN   UINT_32      pdlIdentifier
);

/**
 * @fn  PDL_STATUS pdlLibDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Lib debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

extern PDL_STATUS pdlLibDebugSet (
    IN  BOOLEAN             state
);

/**
 * @fn  PDL_STATUS prvPdlLibStrToEnumConvert( IN PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC * convertDbPtr, IN char * namePtr, OUT UINT_32 * valuePtr );
 *
 * @brief   Convert string representation of enum to value
 *
 * @param [in]  convertDbPtr    db used to convert between string and enum pairs.
 * @param [in]  namePtr         string representation of enum.
 * @param [out] valuePtr        int representation of string.
 *
 * @return  PDL_OK           if convert succeeded.
 * @return  PDL_BAD_PTR      if one of the pointers is NULL.
 * @return  PDL_NOT_FOUND    if string could not be converted.
 */

PDL_STATUS prvPdlLibStrToEnumConvert(
    IN  PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC * convertDbPtr,
    IN  char                                * namePtr,
    OUT UINT_32                             * valuePtr
);

/**
 * @fn  PDL_STATUS prvPdlLibEnumToStrConvert( IN PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC * convertDbPtr, IN UINT_32 value, OUT char ** namePtrPtr );
 *
 * @brief   Convert enum to string representation
 *
 * @param [in]      convertDbPtr    db used to convert between string and enum pairs.
 * @param           value           The value.
 * @param [in,out]  namePtrPtr      If non-null, the name pointer.
 *
 * @return  PDL_OK           if convert succeeded.
 * @return  PDL_BAD_PTR      if one of the pointers is NULL.
 * @return  PDL_NOT_FOUND    if string could not be converted.
 */

PDL_STATUS prvPdlLibEnumToStrConvert(
    IN  PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC * convertDbPtr,
    IN  UINT_32                               value,
    OUT char                               ** namePtrPtr
);

/**
 * @fn  PDL_STATUS prvPdlDbCreate ( IN PDL_DB_PRV_TYPE_ENT dbType, IN PDL_DB_PRV_ATTRIBUTES_STC * dbAttributes, OUT PDL_DB_PRV_STC * dbPtr );
 *
 * @brief   create DB of given type
 *
 * @param [in]  dbType          - db type.
 * @param [in]  dbAttributes    - attributes that are db type specific.
 * @param [out] dbPtr           - pointer to created db structure.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlDbCreate (
    IN  PDL_DB_PRV_TYPE_ENT           dbType,
    IN  PDL_DB_PRV_ATTRIBUTES_STC   * dbAttributes,
    OUT PDL_DB_PRV_STC              * dbPtr
);

/**
 * @fn  prvPdlDbAddStubMac (prvPdlDbAdd);
 *
 * @brief   Add instance to DB
 *
 * @param   parameter1  The first parameter.
 */

prvPdlDbAddStubMac (prvPdlDbAdd);

/**
 * @fn  prvPdlDbFindStubMac(prvPdlDbFind);
 *
 * @brief   Find instance in DB
 *
 * @param   parameter1  The first parameter.
 */

prvPdlDbFindStubMac(prvPdlDbFind);

/**
 * @fn  prvPdlDbGetNextStubMac(prvPdlDbGetNext);
 *
 * @brief   Find next instance in DB
 *
 * @param   parameter1  The first parameter.
 */

prvPdlDbGetNextStubMac(prvPdlDbGetNext);
prvPdlDbGetNextKeyStubMac (prvPdlDbGetNextKey);

/**
 * @fn  prvPdlDbGetNumOfEntriesStubMac(prvPdlDbGetNumOfEntries);
 *
 * @brief   Get DB number of entries
 *
 * @param   parameter1  The first parameter.
 */

prvPdlDbGetNumOfEntriesStubMac(prvPdlDbGetNumOfEntries);

/**
 * @fn  PDL_STATUS pdlLibInit ( IN PDL_OS_CALLBACK_API_STC * callbacksPTR );
 *
 * @brief   Init database module - Bind all implemented DB callbacks
 *
 * @param [in]  callbacksPTR    application-specific implementation for os services.
 * @param [in]  initType        initialization type (stack-partial/full)
 *
 * @return  PDL_STATUS.
 */

prvPdlDbGetFirstStubMac(prvPdlDbGetFirst);

prvPdlDbDestroyStubMac(prvPdlDbDestroy);

PDL_STATUS pdlLibInit (
    IN PDL_OS_CALLBACK_API_STC    * callbacksPTR,
    IN PDL_OS_INIT_TYPE_ENT         initType
);

/**
 * @fn  void * prvPdlOsMalloc ( IN UINT_32 size );
 *
 * @brief   malloc implementation
 *
 * @param [in]  size    memory size to allocate.
 *
 * @return  pointer to allocated memory or NULL if not possible.
 */

void * prvPdlOsMalloc (
    IN UINT_32                    size
);

/**
 * @fn  void * prvPdlOsCalloc ( IN UINT_32 numOfBlocks, IN UINT_32 blockSize );
 *
 * @brief   calloc implementation
 *
 * @param [in]  numOfBlocks number of memory blocks to allocate.
 * @param [in]  blockSize   memory block size to allocate.
 *
 * @return  pointer to allocated memory or NULL if not possible.
 */

void * prvPdlOsCalloc (
    IN UINT_32                  numOfBlocks,
    IN UINT_32                  blockSize
);

/**
 * @fn  void prvPdlOsFree ( IN void * ptr );
 *
 * @brief   free implementation
 *
 * @param [in]  ptr memory to free.
 */

void prvPdlOsFree (
    IN void         *   ptr
);

/**
 * @fn  void prvPdlOsPrintf ( IN char * format, ... );
 *
 * @brief   printf implementation
 *
 * @param [in]  format  format of string to print.
 * @param [in]  ...     additional arguments.
 */

void prvPdlOsPrintf (
    IN char * format,
    ...
);

/* ***************************************************************************
* FUNCTION NAME: prvPdlOsSnprintf
*
* DESCRIPTION:
*
*
*****************************************************************************/

extern int prvPdlOsSnprintf (
    /*!     INPUTS:             */
    char                *str,
    size_t               size,
    const char          * format,
    ...
    /*!     OUTPUTS:            */
);

/**
 * @fn  BOOLEAN prvPdlDebugLogHdr ( IN const char * func_name_PTR, IN UINT_32 line_num);
 *
 * @brief   printf implementation
 *
 * @param [in]  func_name_PTR   calling function name.
 * @param [in]  line_num        line number.
 * @return true if succeeded.
 */

BOOLEAN prvPdlDebugLogHdr (
   IN const char    * func_name_PTR,
   IN UINT_32         line_num
);

/**
 * @fn  void prvPdlStatusDebugLogHdr ( IN const char * func_name_PTR, IN const char * line_num, IN PDL_STATUS   status, IN UINT_32  pdlIdentifier)
 *
 * @brief   Prv pdl debug log
 *
 * @param   func_name_PTR          The function name pointer.
 * @param   line_num               Describes the format to use.
 * @param   status                 error code.
 * @param   pdlIdentifier          identifier for the status type.
 */

BOOLEAN prvPdlStatusDebugLogHdr (
    IN   const char * func_name_PTR,
    IN   UINT_32      line_num,
    IN   PDL_STATUS   status,
    IN   UINT_32      pdlIdentifier
);

/**
 * @fn  BOOLEAN prvPdlDebugRawLog ( IN const char * func_name_PTR, IN UINT_32 line_num);
 *
 * @brief   printf implementation
 *
 * @param [in]  func_name_PTR   calling function name.
 * @param [in]  line_num        line number.
 * @return true if succeeded.
 */

void prvPdlDebugRawLog (
    IN const char * func_name_PTR,
    IN UINT_32      line,
    IN const char * format, 
    IN ...
);

/**
 * @fn  void prvPdlDebugLog ( IN const char * func_name_PTR, IN const char * format, IN ... );
 *
 * @brief   debug log implementation
 *
 * @param [in]  func_name_PTR   calling function name.
 * @param [in]  format          format of string to print.
 * @param [in]  ...             additional arguments.
 */

void prvPdlDebugLog (
    IN const char * func_name_PTR,
    IN const char * format, 
    IN ...
);

/**
 * @fn  void prvPdlLock ( IN PDL_OS_LOCK_TYPE_ENT lockType )
 *
 * @brief   uses lock mechanism given by higher layer to provide mutual exclusion
 *
 * @param [in]      lockType        what are we protecting
 */

void prvPdlLock (
    IN PDL_OS_LOCK_TYPE_ENT         lockType
);

/*$ END OF prvPdlLock */

/**
* @fn  void prvPdlUnlock ( IN PDL_OS_LOCK_TYPE_ENT lockType )
 *
 * @brief   uses lock mechanism given by higher layer to provide mutual exclusion
 *
 * @param [in]      lockType        what are we protecting
 */

void prvPdlUnlock (
    IN PDL_OS_LOCK_TYPE_ENT         lockType
);

/*$ END OF prvPdlUnlock */

/**
 * @fn  void prvPdlCmRegRead 
 *
 * @brief   Reads PP C&M register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlCmRegRead (
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    OUT GT_U32    * dataPtr
);

/*$ END OF prvPdlCmRegRead */

/**
 * @fn  PDL_STATUS prvPdlCmRegWrite 
 *
 * @brief   Writes PP C&M register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [in]      data          data to write
 */

PDL_STATUS prvPdlCmRegWrite (
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    IN  GT_U32      data
);

/*$ END OF prvPdlCmRegWrite */

/**
 * @fn  void prvPdlRegRead 
 *
 * @brief   Reads PP register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlRegRead (
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    OUT GT_U32    * dataPtr
);

/*$ END OF prvPdlRegRead */

/**
 * @fn  PDL_STATUS prvPdlRegWrite 
 *
 * @brief   Writes PP register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [in]      data          data to write
 */

PDL_STATUS prvPdlRegWrite (
    IN  GT_U8       devNum,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    IN  GT_U32      data
);

/*$ END OF prvPdlRegWrite */

/**
 * @fn  PDL_STATUS prvPdlSmiRegRead 
 *
 * @brief   Reads SMI register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      portGroupsBmp bitmap of ports
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      smiAddr       smi adresss
 * @param [in]      regAddr       register address
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlSmiRegRead (
    IN  GT_U8      devIdx,
    IN  GT_U32     portGroupsBmp,
    IN  GT_U32     smiInterface,
    IN  GT_U32     smiAddr,
	IN  GT_U8      phyPageSelReg,
	IN  GT_U8      phyPage,
    IN  GT_U32     regAddr,
    OUT GT_U16    *dataPtr
);

/*$ END OF prvPdlSmiRegRead */

/**
 * @fn  PDL_STATUS prvPdlSmiRegWrite 
 *
 * @brief   Reads SMI register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      portGroupsBmp bitmap of ports
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      smiAddr       smi adresss
 * @param [in]      regAddr       register address
 * @param [out]     dataPtr       data to write
 */

PDL_STATUS prvPdlSmiRegWrite (
    IN  GT_U8      devIdx,
    IN  GT_U32     portGroupsBmp,
    IN  GT_U32     smiInterface,
    IN  GT_U32     smiAddr,
	IN  GT_U8      phyPageSelReg,
	IN  GT_U8      phyPage,
    IN  GT_U32     regAddr,
    IN  GT_U16     mask,
    IN  GT_U16     data
);

/**
 * @fn  PDL_STATUS prvPdlSmiPpuLock
 *
 * @brief   Control the SMI MAC polling unit using given callback
 *
 * @param [in]      devIdx        device index
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      lock  		  stop/start smi auto polling unit
 * @param [out]     prevLockStatePtr previous state of the lock
 */

PDL_STATUS prvPdlSmiPpuLock (
	IN  GT_U8       devIdx,
	IN  GT_U32      smiInterface,
	IN  BOOLEAN     lock,
	OUT BOOLEAN    *prevLockStatePtr
);

/*$ END OF prvPdlSmiRegWrite */

/**
 * @fn  PDL_STATUS prvPdlXsmiRegRead 
 *
 * @brief   Reads XSMI register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      xsmiInterface Xsmi interface (0..3)
 * @param [in]      xsmiAddr      Xsmi adresss
 * @param [in]      regAddr       register address
 * @param [in]      phyDev        the PHY device to read from (APPLICABLE RANGES: 0..31).
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlXsmiRegRead (
    IN  GT_U8      devIdx,
    IN  GT_U32     xsmiInterface,
    IN  GT_U32     xsmiAddr,
    IN  GT_U32     regAddr,
    IN  GT_U32     phyDev, 
    OUT GT_U16     *dataPtr
);

/*$ END OF prvPdlXsmiRegRead */

/**
 * @fn  PDL_STATUS prvPdlXsmiRegWrite 
 *
 * @brief   Reads SMI register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      portGroupsBmp bitmap of ports
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      smiAddr       smi adresss
 * @param [in]      regAddr       register address
 * @param [out]     data          data to write
 */

PDL_STATUS prvPdlXsmiRegWrite (
    IN  GT_U8      devIdx,
    IN  GT_U32     xsmiInterface,
    IN  GT_U32     xsmiAddr,
    IN  GT_U32     regAddr,
    IN  GT_U32     phyDev, 
    IN  GT_U16     mask,
    IN  GT_U16     data
);

/*$ END OF prvPdlXsmiRegWrite */

/**
 * @fn  void prvPdlI2cResultHandler 
 *
 * @brief   Call to given call-back which handle I2C operation result
 *
 * @param [in]      i2c_ret_status  return status from I2C operation
 * @param [in]      slave_address   I2C device (slave) address
 * @param [in]      bus_id          I2C bus id
 * @param [in]      offset          I2C offset
 * @param [in]      i2c_write       whether the operation was I2C write or read
 */
void prvPdlI2cResultHandler (
    /*!     INPUTS:             */
    IN  PDL_STATUS i2c_ret_status,
    IN  UINT_8     slave_address,
    IN  UINT_8     bus_id,
    IN  UINT_8     offset,
    IN  BOOLEAN    i2c_write
);

/**
 * @fn  BOOLEAN prvPdlXmlArchiveUncompressHandler (IN  char  *archiveFileNamePtr, OUT char  *xmlFileNamePtr)
 *
 * @brief   Call to given call-back which handle I2C operation result
 *
 * @param [in]      archiveFileNamePtr        archive name to uncompress
 * @param [out]     xmlFileNamePtr            resulting xml file name after uncompress process
 * @param [out]     signatureFileNamePtr      resulting signature file name after uncompress process
 */
extern BOOLEAN prvPdlXmlArchiveUncompressHandler (
    IN  char       *archiveFileNamePtr,
    OUT char       *xmlFileNamePtr,
    OUT char       *signatureFileNamePtr
);
/*$ END OF prvPdlXmlArchiveUncompressHandler */

/**
 * @fn  BOOLEAN prvPdlXmlVerificationHandler (IN  char  *xmlFileNamePtr, IN char  *signatureFileNamePtr)
 *
 * @brief   Call to given call-back which handle XML signature verification
 *
 * @param [in]      xmlFileNamePtr          xml file name
 * @param [out]     signatureFileNamePtr    signature file name
 */
extern BOOLEAN prvPdlXmlVerificationHandler (
    IN  char       *xmlFileNamePtr,
    IN  char       *signatureFileNamePtr
);

/*$ END OF prvPdlXmlVerificationHandler */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLibStrICmp
*
* DESCRIPTION:   case insensitive string compare
*
*****************************************************************************/

UINT_32 prvPdlLibStrICmp (
    /*!     INPUTS:             */
    const char *string1Ptr,  /* string1_PTR - pointer to the first string */
    const char *string2Ptr   /* string2_PTR - pointer to the second string */
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);

/*$ END OF prvPdlLibStrICmp */

/**
 * @fn  PDL_STATUS prvPdlLibValidateEnumString ( IN XML_PARSER_ENUM_ID_ENT enumId, IN char * strValuePtr )
 *
 * @brief   Validate enum string value existence.
 *
 * @param [in]  enumId          Enum identification.
 * @param [in]  intValuePtr     pointer to string value to convert.
 *
 * @return  PDL_BAD_PARAM       if invalid id or pointer was supplied.
 * @return  PDL_NOT_INITIALIZED if database wasn't initialized.
 * @return  PDL_NOT_FOUND       if no match was found.
 *
 */
extern PDL_STATUS prvPdlLibValidateEnumString (
    IN XML_PARSER_ENUM_ID_ENT   enumId,
    IN char                   * strValuePtr
);

/*$ END OF prvPdlLibValidateEnumString */

/**
 * @fn  PDL_STATUS xmlParserValidateEnumValue ( IN XML_PARSER_ENUM_ID_ENT enumId, IN UINT_32 intValue )
 *
 * @brief   Validate enum int value existence.
 *
 * @param [in]  enumId          Enum identification.
 * @param [in]  intValue        enum int value to convert.
 *
 * @return  PDL_BAD_PARAM       if invalid id or pointer was supplied.
 * @return  PDL_NOT_INITIALIZED if database wasn't initialized.
 * @return  PDL_NOT_FOUND       if no match was found.
 *
 */
extern PDL_STATUS prvPdlLibValidateEnumValue (
    IN XML_PARSER_ENUM_ID_ENT   enumId,
    IN UINT_32                  intValue
);

/*$ END OF prvPdlLibValidateEnumValue */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLibStrdup
*
* DESCRIPTION:   string duplication using local memory allocation callback
*
*****************************************************************************/

extern char *prvPdlLibStrdup (
    /*!     INPUTS:             */
    const char *str1
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);

/*$ END OF prvPdlLibStrdup */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLibStrtok_r
*
* DESCRIPTION:      The function STRINGG_strtok_r() breaks the string s into a sequence
*                   of tokens, each of which is delimited by a character from the string
*                   pointed to by sep.
*
*
*****************************************************************************/

char *prvPdlLibStrtok_r (
    /*!     INPUTS:             */
    char        *s1_PTR,        /* s1_PTR - Points to NULL, or the string from
                                    which to extract tokens. */
    /*!     INPUTS:             */
    const char  *delim_PTR,     /* delim_PTR - Points to a null-terminated set of
                                   delimiter characters that separate the
                                   tokens. */
    /*!     INPUTS / OUTPUTS:   */
    char        **save_ptr_PTR  /* save_ptr_PTR - Is a value-return parameter used by
                                   the function to record its progress
                                   through s. */
);

/*$ END OF prvPdlLibStrtok_r */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLibPortModeSupported
*
* DESCRIPTION:   verify port interface mode & speed are supported by HW
*
*****************************************************************************/

BOOLEAN prvPdlLibPortModeSupported(
    UINT_8                      devIdx,
    UINT_32                     mac_port,
    PDL_PORT_SPEED_ENT          speed,
    PDL_INTERFACE_MODE_ENT      interface_mode
);

/*$ END OF prvPdlLibPortModeSupported */

/**
 * @fn  PDL_STATUS prvPdlLibBoardDescriptionInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlRootId )
 *
 * @brief   parse board description section
 *
 * @param [in]  xmlRootId   Xml root id.
 *
 * @return  PDL_OK                  success
 */

PDL_STATUS prvPdlLibBoardDescriptionInit(
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlRootId
);

/*$ END OF prvPdlLibBoardDescriptionInit */

/**
 * @fn  PDL_STATUS prvPdlLedStreamPortPositionSet 
 *
 * @brief   set ledstream port position
 *
 * @param [in]      devIdx        device index
 * @param [in]      portNum       port number
 * @param [in]      position      led stream port position
 */

PDL_STATUS prvPdlLedStreamPortPositionSet (
    IN  GT_U8                   devIdx,
    IN  GT_U32                  portNum,
    IN  GT_U32                  position
);

/*$ END OF prvPdlLedStreamPortPositionSet */

/**
 * @fn  PDL_STATUS prvPdlLedStreamPortClassPolarityInvertEnableSet 
 *
 * @brief   set ledstream port polarity
 *
 * @param [in]      devIdx        device index
 * @param [in]      portNum       port number
 * @param [in]      classNum      class number
 * @param [in]      invertEnable  polarity invert enable/disable
 */

PDL_STATUS prvPdlLedStreamPortClassPolarityInvertEnableSet (
   IN  GT_U8                           devIdx,
   IN  GT_U32                          portNum,
   IN  GT_U32                          classNum,
   IN  BOOLEAN                         invertEnable
);

/*$ END OF prvPdlLedStreamPortClassPolarityInvertEnableSet */

/**
 * @fn  PDL_STATUS prvPdlLedStreamConfigSet 
 *
 * @brief   set ledstream configuration
 *
 * @param [in]      devIdx                  device index
 * @param [in]      ledInterfaceNum         ledstream interface number
 * @param [in]      ledConfPtr              led stream configuration paramters
 */

PDL_STATUS prvPdlLedStreamConfigSet (
    IN  GT_U8                               devIdx,
    IN  GT_U32                              ledInterfaceNum,
    IN  PDL_LED_STEAM_INTERFACE_CONF_STC   *ledConfPtr
);

/*$ END OF prvPdlLedStreamConfigSet */

/**
 * @fn  PDL_STATUS prvPdlLedStreamClassManipulationGet 
 *
 * @brief   ledstream manipulation get value
 *
 * @param [in]      devIdx                  device index
 * @param [in]      ledInterfaceNum         ledstream interface number
 * @param [in]      portType                port type
 * @param [in]      classNum                class number
 * @param [out]     classParamsPtr          value
 */

PDL_STATUS prvPdlLedStreamClassManipulationGet (
    IN  GT_U8                           devIdx,
    IN  GT_U32                          ledInterfaceNum,
    IN  PDL_LED_PORT_TYPE_ENT           portType,
    IN  GT_U32                          classNum,
    OUT PDL_LED_CLASS_MANIPULATION_STC *classParamsPtr
);

/*$ END OF prvPdlLedStreamClassManipulationGet */

/**
 * @fn  PDL_STATUS prvPdlLedStreamClassManipulationSet 
 *
 * @brief   ledstream manipulation set value
 *
 * @param [in]      devIdx                  device index
 * @param [in]      ledInterfaceNum         ledstream interface number
 * @param [in]      portType                port type
 * @param [in]      classNum                class number
 * @param [in]      classParamsPtr          value
 */

PDL_STATUS prvPdlLedStreamClassManipulationSet (
    IN  GT_U8                           devIdx,
    IN  GT_U32                          ledInterfaceNum,
    IN  PDL_LED_PORT_TYPE_ENT           portType,
    IN  GT_U32                          classNum,
    IN  PDL_LED_CLASS_MANIPULATION_STC *classParamsPtr
);

/*$ END OF prvPdlLedStreamClassManipulationSet */

/* @}*/

#endif

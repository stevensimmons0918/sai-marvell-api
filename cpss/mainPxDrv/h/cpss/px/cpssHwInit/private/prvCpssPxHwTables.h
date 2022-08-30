/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssPxHwTables.h
*
* @brief PX : Private API definition for tables access (direct and indirect).
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssPxHwTables_h
#define __prvCpssPxHwTables_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/private/prvCpssMisc.h>
#include <cpss/px/cpssHwInit/cpssPxTables.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>

/* value for field of 'fieldWordNum' that indicates that the 'word' is not used,
   and instead the field of 'fieldOffset' represents 'Global bit offset' and not
   offset in specific word

   see relevant APIs :
   prvCpssPxReadTableEntryField(...)
   prvCpssPxWriteTableEntryField(...)
*/
#define PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS   0xFFFFFFFF

/* value for field of 'fieldWordNum' that indicates that the 'word' is not used,
   and instead the field of 'fieldOffset' represents 'field name' and not
   offset in specific word.

   important: the field 'fieldLength' is used ONLY if  it's value != 0
              and then it means 'Mumber of consecutive fields' !!!

   see relevant APIs :
   prvCpssPxReadTableEntryField(...)
   prvCpssPxWriteTableEntryField(...)
*/
#define PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS   0xEEEEEEEE

#define PRV_CPSS_PX_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS     0

/* value of 'sub field' for field of 'fieldWordNum' that indicates that
   the 'word' is not used,
   and instead the field of 'fieldOffset' represents 'field name' and not
   offset in specific word.
   and the 'fieldLength' is used as 'sub field offset' and as 'sub field length'.
   use macro

   see relevant APIs :
   prvCpssPxReadTableEntryField(...)
   prvCpssPxWriteTableEntryField(...)
*/
#define PRV_CPSS_PX_TABLE_WORD_INDICATE_SUB_FIELD_IN_FIELD_NAME_CNS 0xDDDDDDDD

/* macro to use with PRV_CPSS_PX_TABLE_WORD_INDICATE_SUB_FIELD_IN_FIELD_NAME_CNS
    for the field of fieldLength
   The 'fieldLength' is used as 'sub field offset' and as 'sub field length'.
   use macro
   see relevant APIs :
   prvCpssPxReadTableEntryField(...)
   prvCpssPxWriteTableEntryField(...)
*/
#define PRV_CPSS_PX_TABLE_SUB_FIELD_OFFSET_AND_LENGTH_MAC(_offset,_length)    \
    (((_offset)& 0xFFFF) | ((_length) << 16))

/* flag in param <*numBitsPerEntryPtr> in
    prvCpssPxTableEngineMultiEntriesInLineIndexAndGlobalBitConvert*/
/* indication that the number in the lower 30 bits is 'fraction' (1/x) and not (x) */
#define FRACTION_INDICATION_CNS         BIT_31

/* flag in param <*numBitsPerEntryPtr> in
    prvCpssPxTableEngineMultiEntriesInLineIndexAndGlobalBitConvert*/
/* indication that the number in the lower 30 bits is 'vertical' :
            the x entries in the first 'line' are not : 0,1
            the x entries in the first 'line' are     : 0,depth
 */
#define VERTICAL_INDICATION_CNS         BIT_30

/* flag in param <*numBitsPerEntryPtr> in
    prvCpssPxTableEngineMultiEntriesInLineIndexAndGlobalBitConvert*/
/* indication that the number in the lower 30 bits is 'fraction' (1/x) and not (x)
    but 1/2 of the entry in second half of the table

    meaning that 'read'  of entry requires 2 'read' lines . from 2 half indexes in the table.
    meaning that 'write' of entry requires 2 'write' lines. to 2 half indexes in the table.
*/
#define FRACTION_HALF_TABLE_INDICATION_CNS         BIT_29

/* common used types */

/**
* @struct PRV_CPSS_PX_PP_TABLE_FORMAT_INFO_STC
 *
 * @brief A structure to hold device tables format info.
*/
typedef struct{

    /** @brief number of fields in table entry
     *  fieldsInfoPtr  - pointer to table fields info
     */
    GT_U32 numFields;

    PRV_CPSS_ENTRY_FORMAT_TABLE_STC *   fieldsInfoPtr;

    /** number of patterns in table entry */
    GT_U32 patternNum;

    /** number of bits in pattern */
    GT_U32 patternBits;

} PRV_CPSS_PX_PP_TABLE_FORMAT_INFO_STC;

/* indication that field is calculated in runtime based on other fields */
#define PRV_CPSS_PX_AUTO_CALC_INDICATION

/**
* @struct PRV_CPSS_PX_TABLES_INFO_DIRECT_STC
 *
 * @brief A structure to hold Cheetah's direct access table entry info.
*/
typedef struct{

    /** base address */
    GT_U32 baseAddress;

    /** @brief number of bits in the entry.
     *  (will be used to calculate the entry   width in words)
     *  (will be used to calculate the alignment width in bytes)
     */
    GT_U32 numOfBits;

    /** @brief number of entries in the table.
     *  AUTO CALC section
     *  alignmentWidthInBytes - the alignment width in bytes
     *  when value in this field is 0 ... meaning need 'AUTO CALC (from numOfBits)
     *  when value in NOT 0 --> do not modify it
     *  entryWidthInWords   - the entry   width in words
     *  ALWAYS 'AUTO calc' (from numOfBits)
     */
    GT_U32 maxNumOfEntries;

    GT_U32 alignmentWidthInBytes PRV_CPSS_PX_AUTO_CALC_INDICATION;

    GT_U32 entryWidthInWords     PRV_CPSS_PX_AUTO_CALC_INDICATION;

} PRV_CPSS_PX_TABLES_INFO_DIRECT_STC;

typedef GT_CHAR*   PRV_CPSS_PX_TABLE_NAME;

/**
* @struct PRV_CPSS_PX_TABLES_INFO_STC
 *
 * @brief A structure to hold Pipe's direct access table entry info.
*/
typedef struct{

    /** table index */
    CPSS_PX_TABLE_ENT globalIndex;

    /** the table name (for debug and for LOGGER) */
    PRV_CPSS_PX_TABLE_NAME tableName;

    /** unit index */
    PRV_CPSS_DXCH_UNIT_ENT unitIndex;

    /** direct access table info */
    PRV_CPSS_PX_TABLES_INFO_DIRECT_STC directAccessInfo;

} PRV_CPSS_PX_TABLES_INFO_STC;



/**
* @enum PRV_CPSS_PX_TABLE_FORMAT_ENT
 *
 * @brief Enumerator of tables formats names
*/
typedef enum{

    /** dummy */
    PRV_CPSS_PX_TABLE___DUMMY___E,

    PRV_CPSS_PX_TABLE_LAST_FORMAT_E

} PRV_CPSS_PX_TABLE_FORMAT_ENT;

/**
* @internal prvCpssPxTablesAccessInit function
* @endinternal
*
* @brief   Initializes all structures for table access.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*/
GT_STATUS  prvCpssPxTablesAccessInit
(
    IN GT_SW_DEV_NUM    devNum
);

/**
* @internal prvCpssPxReadTableEntry function
* @endinternal
*
* @brief   Read a whole entry from the table.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
*
* @param[out] entryValuePtr            - (pointer to) the data read from the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send read entry command to device. And entry is stored in the
*       data registers of a indirect table
*
*/
GT_STATUS prvCpssPxReadTableEntry
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PX_TABLE_ENT        tableType,
    IN GT_U32                   entryIndex,
    OUT GT_U32                 *entryValuePtr
);

/**
* @internal prvCpssPxWriteTableEntry function
* @endinternal
*
* @brief   Write a whole entry to the table.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send write entry command to device. And entry is taken from the
*       data registers of a indirect table.
*
*/
GT_STATUS prvCpssPxWriteTableEntry
(
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PX_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr
);

/**
* @internal prvCpssPxReadTableEntryField function
* @endinternal
*
* @brief   Read a field from the table.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
*                                      use PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                                      if need global offset in the field of fieldOffset
*                                      use PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
*                                      when fieldWordNum == PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      the field 'fieldLength' is used ONLY if  it's value != 0
*                                      and then it means 'Mumber of consecutive fields' !!!
*
* @param[out] fieldValuePtr            - (pointer to) the data read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxReadTableEntryField
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PX_TABLE_ENT        tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                   fieldWordNum,
    IN GT_U32                   fieldOffset,
    IN GT_U32                   fieldLength,
    OUT GT_U32                 *fieldValuePtr
);

/**
* @internal prvCpssPxWriteTableEntryField function
* @endinternal
*
* @brief   Write a field to the table.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
*                                      use PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                                      if need global offset in the field of fieldOffset
*                                      use PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
*                                      when fieldWordNum == PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      the field 'fieldLength' is used ONLY if  it's value != 0
*                                      and then it means 'Mumber of consecutive fields' !!!
* @param[in] fieldValue               - the data write to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxWriteTableEntryField
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PX_TABLE_ENT        tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                   fieldWordNum,
    IN GT_U32                   fieldOffset,
    IN GT_U32                   fieldLength,
    IN GT_U32                   fieldValue
);


/**
* @internal prvCpssPxTableNumEntriesGet function
* @endinternal
*
* @brief   get the number of entries in a table
*         needed for debug purpose
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*
* @param[out] numEntriesPtr            - (pointer to) number of entries in the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxTableNumEntriesGet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PX_TABLE_ENT  tableType,
    OUT GT_U32                  *numEntriesPtr
);



/**
* @internal prvCpssPxWriteTableMultiEntry function
* @endinternal
*
* @brief   Write number of entries to the table in consecutive indexes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] startIndex               - index to the first table entry
* @param[in] numOfEntries             - the number of consecutive entries to write
* @param[in] entryValueArrayPtr       - (pointer to) the data that will be written to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxWriteTableMultiEntry
(
    IN GT_SW_DEV_NUM                            devNum,
    IN CPSS_PX_TABLE_ENT          tableType,
    IN GT_U32                           startIndex,
    IN GT_U32                           numOfEntries,
    IN GT_U32                           *entryValueArrayPtr
);

/**
* @internal prvCpssPxReadTableMultiEntry function
* @endinternal
*
* @brief   Read number of entries from the table in consecutive indexes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] startIndex               - index to the first table entry
* @param[in] numOfEntries             - the number of consecutive entries to read
*
* @param[out] entryValueArrayPtr       - (pointer to) the data that will be read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxReadTableMultiEntry
(
    IN  GT_SW_DEV_NUM                            devNum,
    IN  CPSS_PX_TABLE_ENT          tableType,
    IN  GT_U32                           startIndex,
    IN  GT_U32                           numOfEntries,
    OUT GT_U32                           *entryValueArrayPtr
);

/**
* @internal prvCpssPxTableBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of table.(for 'Direct access' tables)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*
* @param[out] errorPtr                 - (pointer to) indication that function did error. (can be NULL)
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*/
GT_U32  prvCpssPxTableBaseAddrGet
(
    IN GT_SW_DEV_NUM                            devNum,
    IN CPSS_PX_TABLE_ENT          tableType,
    OUT GT_BOOL                         *errorPtr
);

/**
* @internal prvCpssPxTableEngineToHwRatioGet function
* @endinternal
*
* @brief   return the ratio between the index that the 'table engine' gets from the
*         'cpss API' to the 'HW index' in the table.
*         NOTE: this to support 'multi entries' in line or fraction of entry in line.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*
* @param[out] ratioPtr                 - (pointer to) number of entries for in single line or
*                                      number of lines  for in single entry.
* @param[out] isMultiplePtr            - (pointer to) indication that ratio is 'multiple' or 'fraction'.
*                                      GT_TRUE  - ratio is 'multiple' (number of entries for in single line)
*                                      GT_FALSE - ratio is 'fraction' (number of lines  for in single entry)
*                                       GT_OK
*/
GT_STATUS prvCpssPxTableEngineToHwRatioGet
(
    IN GT_SW_DEV_NUM    devNum,
    IN CPSS_PX_TABLE_ENT      tableType,
    OUT GT_U32                 *ratioPtr,
    OUT GT_BOOL                *isMultiplePtr
);

/**
* @internal prvCpssPxWriteTableEntryMasked function
* @endinternal
*
* @brief   Either write a whole entry into HW table or update HW entry bits
*         specified by a mask. If Shadow DB table exists it will be updated too.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
* @param[in] entryMaskPtr             - (pointer to) a mask array.
*                                      Ignored if entryValuePtr is NULL.
*                                      If NULL -  the entry specified by entryValuePtr will be
*                                      written to HW table as is.
*                                      If not NULL  - only bits that are raised in the mask
*                                      will be updated in the HW entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send write entry command to device. And entry is taken from the
*       data registers of a indirect table.
*
*/
GT_STATUS prvCpssPxWriteTableEntryMasked
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PX_TABLE_ENT        tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr,
    IN GT_U32                  *entryMaskPtr
);


/**
* @internal prvCpssPxPortGroupReadTableEntry function
* @endinternal
*
* @brief   Read a whole entry from the table. - for specific portGroupId
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
*
* @param[out] entryValuePtr            - (pointer to) the data read from the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send read entry command to device. And entry is stored in the
*       data registers of a indirect table
*
*/
GT_STATUS prvCpssPxPortGroupReadTableEntry
(
    IN GT_SW_DEV_NUM           devNum,
    IN GT_U32                  portGroupId,
    IN CPSS_PX_TABLE_ENT       tableType,
    IN GT_U32                  entryIndex,
    OUT GT_U32                 *entryValuePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxHwTables_h */



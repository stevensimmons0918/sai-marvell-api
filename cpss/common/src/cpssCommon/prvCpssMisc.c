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
* @file prvCpssMisc.c
*
* @brief Miscellaneous operations for CPSS.
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* return GT_U32-mask with n lowest bits set to 0. Rest of mask's bits are 1. */
#define LO_ZEROES_MAC(n) ((n)>=32 ? (GT_U32)0  : (GT_U32)-1<<(n))

/* return GT_U32-mask with n lowest bits set to 1. Rest of mask's bits are 0. */
#define LO_ONES_MAC(n)   ((n)>=32 ? (GT_U32)-1 : (GT_U32)(1<<(n))-1)


/**
* @internal prvCpssPeriodicFieldValueSet function
* @endinternal
*
* @brief   Set a value of a field into entry that lay in array of words.
*         the entry length is in bits and can be any non-zero number
* @param[in,out] dataArray[]              - array of words to set the field value into.
* @param[in] entryIndex               - entry index inside dataArray[]
* @param[in] entryNumBits             - number of bits for each entry.(non-zero)
* @param[in] fieldStartBit            - start bit of field . this bit is counted from the start
*                                      of the entry(and not from entry[0]).
* @param[in] fieldNumBits             - number of bits to set (field length) (1..32)
* @param[in] fieldValue               - the value of the field that need to set.
* @param[in,out] dataArray[]              - array of words with updated data on the field.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad parameter entryNumBits or fieldNumBits
*                                       or fieldNumBits > entryNumBits
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssPeriodicFieldValueSet(
    INOUT GT_U32    dataArray[],
    IN GT_U32       entryIndex,
    IN GT_U32       entryNumBits,
    IN GT_U32       fieldStartBit,
    IN GT_U32       fieldNumBits,
    IN GT_U32       fieldValue
)
{
    /* the field bits that can be spread on 2 registers max */
    GT_U32  firstWordIndex;/* temporary index of the word to update in dataArray[] */
    GT_U32  offset;/* start bit index , in the first updated word */
    GT_U32  indexOfStartingBit; /* index of bit to start with , indexed from the
                                   start of the array dataArray[]*/
    GT_U32  freeBits;/*number of bits in the first updated word from the offset
                       to end of word --> meaning (32-offset)
                       this value allow as to understand if we update single
                       word or 2  words.
                       */

    CPSS_NULL_PTR_CHECK_MAC(dataArray);

    if(fieldNumBits > 32  || fieldNumBits == 0 ||
       entryNumBits == 0 ||
       ((fieldNumBits + fieldStartBit) > entryNumBits))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    indexOfStartingBit = entryNumBits * entryIndex + fieldStartBit;

    firstWordIndex =  indexOfStartingBit >> 5 /* /32 */;

    offset = indexOfStartingBit & 0x1f /* % 32 */;
    freeBits = 32 - offset;
    if(freeBits >= fieldNumBits)
    {
        /* set the value in the field -- all in single word */
        U32_SET_FIELD_MASKED_MAC(dataArray[firstWordIndex],offset,fieldNumBits,fieldValue);
    }
    else
    {
        /* set the start of value in the first word */
        U32_SET_FIELD_MASKED_MAC(dataArray[firstWordIndex],offset,freeBits,fieldValue);
        /* set the rest of the value in the second word */
        U32_SET_FIELD_MASKED_MAC(dataArray[firstWordIndex + 1],0,fieldNumBits - freeBits,(fieldValue>>freeBits));
    }

    return GT_OK;
}

/**
* @internal prvCpssPeriodicFieldValueGet function
* @endinternal
*
* @brief   Get a value of a field from entry that lay in array of words.
*         the entry length is in bits and can be any non-zero number
* @param[in] dataArray[]              - array of words to Get the field value from.
* @param[in] entryIndex               - entry index inside dataArray[]
* @param[in] entryNumBits             - number of bits for each entry.(non-zero)
* @param[in] fieldStartBit            - start bit of field . this bit is counted from the start
*                                      of the entry(and not from entry[0]).
* @param[in] fieldNumBits             - number of bits to get (field length) (1..32)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad parameter entryNumBits or fieldNumBits
*                                       or fieldNumBits > entryNumBits
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssPeriodicFieldValueGet(
    IN GT_U32       dataArray[],
    IN GT_U32       entryIndex,
    IN GT_U32       entryNumBits,
    IN GT_U32       fieldStartBit,
    IN GT_U32       fieldNumBits,
    OUT GT_U32      *fieldValuePtr
)
{
    /* the field bits that can be spread on 2 registers max */
    GT_U32  firstWordIndex;/* temporary index of the word to read from dataArray[] */
    GT_U32  offset;/* start bit index , in the first read word */
    GT_U32  indexOfStartingBit; /* index of bit to start with , indexed from the
                                   start of the array dataArray[]*/
    GT_U32  freeBits;/*number of bits in the first read word from the offset
                       to end of word --> meaning (32-offset)
                       this value allow as to understand if we read single
                       word or 2  words.
                       */

    CPSS_NULL_PTR_CHECK_MAC(dataArray);
    CPSS_NULL_PTR_CHECK_MAC(fieldValuePtr);

    if(fieldNumBits > 32  || fieldNumBits == 0 ||
       entryNumBits == 0 ||
       ((fieldNumBits + fieldStartBit)  > entryNumBits))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    indexOfStartingBit = entryNumBits * entryIndex + fieldStartBit;

    firstWordIndex =  indexOfStartingBit >> 5 /* /32 */;

    offset = indexOfStartingBit & 0x1f /* % 32 */;
    freeBits = 32 - offset;
    if(freeBits >= fieldNumBits)
    {
        /* get the value of the field -- all in single word */
        *fieldValuePtr = U32_GET_FIELD_MAC(dataArray[firstWordIndex],offset,fieldNumBits);
    }
    else
    {
        /* get the start of value from the first word */
        *fieldValuePtr = U32_GET_FIELD_MAC(dataArray[firstWordIndex],offset,freeBits);
        /* get the rest of the value from the second word */
        *fieldValuePtr |=
            ((U32_GET_FIELD_MAC(dataArray[firstWordIndex + 1],0,(fieldNumBits - freeBits))) << freeBits);
    }

    return GT_OK;
}

/**
* @internal prvCpssFieldValueGet function
* @endinternal
*
* @brief   get the value of field (up to 32 bits) that located in any start bit in
*         memory
* @param[in] startMemPtr              - pointer to memory
* @param[in] startBit                 - start bit of field (0..)
* @param[in] numBits                  - number of bits of field (1..32)
* @param[in] startmemSize             - size in bytes of start memory to check out of range.
* @param[out] valuePtr                 - (pointer to) value get
*                                      Returns:
*                                      COMMENTS:
*                                      GT_OK - no error
*                                      GT_BAD_PTR - on NULL pointer
*                                      GT_BAD_PARAM - on bad parameter : numBits > 32 or numBits == 0
*
* @note GT_OK - no error
*       GT_BAD_PTR - on NULL pointer
*       GT_BAD_PARAM - on bad parameter : numBits > 32 or numBits == 0
*
*/
GT_STATUS  prvCpssFieldValueOorCheckGet
(
    IN GT_U32                  *startMemPtr,
    IN GT_U32                  startBit,
    IN GT_U32                  numBits,
    IN GT_U32                  startMemSize,
    OUT GT_U32                 *valuePtr
)
{
    GT_U32  value;    /* value of field */
    GT_U32  wordIndex = 0;/* word index in startMemPtr[]*/
    GT_U32  bitIndex  = 0; /* bit index in startMemPtr[wordIndex]*/
    GT_U32  len0;       /* length of field in first startMemPtr[wordIndex] */
    GT_U32  tmpValue;   /* temo value to help calculation */
    GT_U32  secondWordIndex; /* index of second word where last bits are */

    CPSS_NULL_PTR_CHECK_MAC(startMemPtr);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    if(numBits > 32 || numBits == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((startBit + numBits) > (startMemSize * 8))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    wordIndex = OFFSET_TO_WORD_MAC(startBit);
    bitIndex  = OFFSET_TO_BIT_MAC(startBit);

    secondWordIndex = OFFSET_TO_WORD_MAC(startBit + numBits - 1);
    if(wordIndex == secondWordIndex)
    {
        /* read in the same word */
        if(numBits == 32)
        {
            value = startMemPtr[wordIndex];
        }
        else
        {
            value = U32_GET_FIELD_MAC(startMemPtr[wordIndex],bitIndex,(numBits));
        }
    }
    else
    {
        len0 = (32 - bitIndex);
        /* read from 2 words */
        /* in first word , start from bitIndex , and read to end of word */
        value = U32_GET_FIELD_MAC(startMemPtr[wordIndex],bitIndex,len0);

        /* in second word , start from bit 0 , and read the rest of data */
        tmpValue = U32_GET_FIELD_MAC(startMemPtr[secondWordIndex],0,(numBits - len0));
        value |= tmpValue << len0;
    }

    *valuePtr =  value;

    return GT_OK;
}

/**
* @internal prvCpssFieldValueGet function
* @endinternal
*
* @brief   get the value of field (up to 32 bits) that located in any start bit in
*         memory
* @param[in] startMemPtr              - pointer to memory
* @param[in] startBit                 - start bit of field (0..)
* @param[in] numBits                  - number of bits of field (1..32)
*
* @param[out] valuePtr                 - (pointer to) value get
*                                      Returns:
*                                      COMMENTS:
*                                      GT_OK - no error
*                                      GT_BAD_PTR - on NULL pointer
*                                      GT_BAD_PARAM - on bad parameter : numBits > 32 or numBits == 0
*
* @note GT_OK - no error
*       GT_BAD_PTR - on NULL pointer
*       GT_BAD_PARAM - on bad parameter : numBits > 32 or numBits == 0
*
*/
GT_STATUS  prvCpssFieldValueGet
(
    IN GT_U32                  *startMemPtr,
    IN GT_U32                  startBit,
    IN GT_U32                  numBits,
    OUT GT_U32                 *valuePtr
)
{
    GT_U32  value;    /* value of field */
    GT_U32  wordIndex = OFFSET_TO_WORD_MAC(startBit);/* word index in startMemPtr[]*/
    GT_U32  bitIndex  = OFFSET_TO_BIT_MAC(startBit); /* bit index in startMemPtr[wordIndex]*/
    GT_U32  len0;       /* length of field in first startMemPtr[wordIndex] */
    GT_U32  tmpValue;   /* temo value to help calculation */
    GT_U32  secondWordIndex; /* index of second word where last bits are */

    CPSS_NULL_PTR_CHECK_MAC(startMemPtr);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    if(numBits > 32 || numBits == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* calculate word index for last bits */
    secondWordIndex = OFFSET_TO_WORD_MAC(startBit + numBits - 1);
    if(wordIndex == secondWordIndex)
    {
        /* read in the same word */
        if(numBits == 32)
        {
            value = startMemPtr[wordIndex];
        }
        else
        {
            value = U32_GET_FIELD_MAC(startMemPtr[wordIndex],bitIndex,(numBits));
        }
    }
    else
    {
        len0 = (32 - bitIndex);
        /* read from 2 words */
        /* in first word , start from bitIndex , and read to end of word */
        value = U32_GET_FIELD_MAC(startMemPtr[wordIndex],bitIndex,len0);

        /* in second word , start from bit 0 , and read the rest of data */
        tmpValue = U32_GET_FIELD_MAC(startMemPtr[secondWordIndex],0,(numBits - len0));
        value |= tmpValue << len0;
    }

    *valuePtr =  value;

    return GT_OK;
}

/**
* @internal prvCpssFieldValueSet function
* @endinternal
*
* @brief   set the value of field (up to 32 bits) that located in any start bit in
*         memory
* @param[in] startMemPtr              - pointer to memory
* @param[in] startBit                 - start bit of field (0..)
* @param[in] numBits                  - number of bits of field (1..32)
* @param[in] value                    - the  to set
*                                      Returns:
*                                      COMMENTS:
*                                      GT_OK - no error
*                                      GT_BAD_PTR - on NULL pointer
*                                      GT_BAD_PARAM - on bad parameter : numBits > 32 or numBits == 0
*
* @note GT_OK - no error
*       GT_BAD_PTR - on NULL pointer
*       GT_BAD_PARAM - on bad parameter : numBits > 32 or numBits == 0
*       COMMENTS:
*       None.
*
*/
GT_STATUS  prvCpssFieldValueSet
(
    IN GT_U32                  *startMemPtr,
    IN GT_U32                  startBit,
    IN GT_U32                  numBits,
    IN GT_U32                  value
)
{
    GT_U32  wordIndex = OFFSET_TO_WORD_MAC(startBit);/* word index in startMemPtr[]*/
    GT_U32  bitIndex  = OFFSET_TO_BIT_MAC(startBit); /* bit index in startMemPtr[wordIndex]*/
    GT_U32  len0;       /* length of field in first startMemPtr[wordIndex] */
    GT_U32  secondWordIndex; /* index of second word where last bits are */

    CPSS_NULL_PTR_CHECK_MAC(startMemPtr);

    if(numBits > 32 || numBits == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* calculate word index for last bits */
    secondWordIndex = OFFSET_TO_WORD_MAC(startBit + numBits - 1);
    if(wordIndex == secondWordIndex)
    {
        /* write in the same word */
        U32_SET_FIELD_MASKED_MAC(startMemPtr[wordIndex],bitIndex,(numBits),(value));
    }
    else
    {
        len0 = (32 - bitIndex);
        /* write in 2 words */
        /* in first word , start from bitIndex , and write to end of word */
        U32_SET_FIELD_MASKED_MAC(startMemPtr[wordIndex],bitIndex,len0,value);

        /* in second word , start from bit 0 , and write the rest of data */
        U32_SET_FIELD_MASKED_MAC(startMemPtr[secondWordIndex],0,(numBits - len0),(value >> len0));
    }

    return GT_OK;
}

/**
* @internal prvCpssFieldFromEntry_GT_U32_Get function
* @endinternal
*
* @brief   Get GT_U32 value of a field from the table entry.
*
* @param[in] entryPtr                 - pointer to memory.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[])
*
* @param[out] valuePtr                 - (pointer to) value get
*                                      RETURN:
*                                      GT_OK   -   on success
*                                      GT_BAD_PTR  - on NULL pointer
*                                      GT_BAD_PARAM - on bad parameter : fieldInfo.numBits > 32 or fieldInfo.numBits == 0
*                                      COMMENTS:
*/
GT_STATUS prvCpssFieldFromEntry_GT_U32_Get(
    IN GT_U32                           *entryPtr,
    IN const PRV_CPSS_ENTRY_FORMAT_TABLE_STC  fieldsInfoArr[],
    IN GT_U32                           fieldIndex,
    OUT GT_U32                          *valuePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoArr);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    if(fieldsInfoArr[fieldIndex].startBit == PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    return prvCpssFieldValueGet(entryPtr,fieldsInfoArr[fieldIndex].startBit,fieldsInfoArr[fieldIndex].numOfBits,valuePtr);
}

/**
* @internal prvCpssFieldMaxVal_GT_U32_Get function
* @endinternal
*
* @brief   Get GT_U32 max value of a field from the table entry.
*
* @param[in] entryPtr                 - pointer to memory.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[])
*
* @param[out] valuePtr                 - (pointer to) value get
*
* @retval GT_OK                        - on success.
* @retval GT_BAD_PARAM                 - on bad parameter.
* @retval GT_BAD_PTR                   - on NULL pointer.
*
*/
GT_STATUS prvCpssFieldMaxVal_GT_U32_Get(
    IN const PRV_CPSS_ENTRY_FORMAT_TABLE_STC  fieldsInfoArr[],
    IN GT_U32                           fieldIndex,
    OUT GT_U32                          *valuePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoArr);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    if(fieldsInfoArr[fieldIndex].startBit == PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (fieldsInfoArr[fieldIndex].numOfBits < 32) {
        *valuePtr = ((1 << (fieldsInfoArr[fieldIndex].numOfBits)) - 1);
    }
    else
    {
        *valuePtr = 0xFFFFFFFF;
    }


    return GT_OK;
}

/**
* @internal prvCpssFieldToEntry_GT_U32_Set function
* @endinternal
*
* @brief   Set GT_U32 value to a field in a table entry.
*         NOTE:the value MUST not be bigger then the max value for the field !
* @param[in] entryPtr                 - pointer to memory.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[])
* @param[in] value                    - the  to set
*/
GT_STATUS prvCpssFieldToEntry_GT_U32_Set(
    IN GT_U32                           *entryPtr,
    IN const PRV_CPSS_ENTRY_FORMAT_TABLE_STC  fieldsInfoArr[],
    IN GT_U32                           fieldIndex,
    IN GT_U32                           value
)
{
    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoArr);

    if(fieldsInfoArr[fieldIndex].startBit == PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if(fieldsInfoArr[fieldIndex].numOfBits < 32 &&
        (value > BIT_MASK_MAC(fieldsInfoArr[fieldIndex].numOfBits)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssFieldValueSet(entryPtr,fieldsInfoArr[fieldIndex].startBit,fieldsInfoArr[fieldIndex].numOfBits,value);
}

/**
* @internal prvCpssFieldToEntry_GT_Any_Set function
* @endinternal
*
* @brief   Set 'any number of bits' value to a field in a table entry.
*         the bits are in little Endian order in the array of words.
*         NOTE:the value MUST not be bigger then the max value for the field !
* @param[in] entryPtr                 - pointer to memory.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[])
* @param[in] valueArr[]               - the array of values to set
*/
GT_STATUS prvCpssFieldToEntry_GT_Any_Set(
    IN GT_U32                           *entryPtr,
    IN const PRV_CPSS_ENTRY_FORMAT_TABLE_STC  fieldsInfoArr[],
    IN GT_U32                           fieldIndex,
    IN GT_U32                           valueArr[]
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  numOfWords;/* number of words in the field */
    GT_U32  startBit;/* start bit of current section of the field */
    GT_U32  numOfBits;/* number of bits of current section of the field */
    GT_U32  value;/* value of current section of the field */

    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoArr);
    CPSS_NULL_PTR_CHECK_MAC(valueArr);

    if(fieldsInfoArr[fieldIndex].startBit == PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    else if(fieldsInfoArr[fieldIndex].numOfBits == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    numOfWords = (fieldsInfoArr[fieldIndex].numOfBits + 31) / 32;
    startBit = fieldsInfoArr[fieldIndex].startBit;
    numOfBits = 32;

    /* handle the 'Full words' (32 bits) sections */
    for(ii = 0 ; ii < (numOfWords - 1); ii++ , startBit += 32)
    {
        value = valueArr[ii];
        rc = prvCpssFieldValueSet(entryPtr,startBit,numOfBits,value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    value = valueArr[ii];

    /* handle the last word */
    numOfBits = fieldsInfoArr[fieldIndex].numOfBits & 0x1f;
    if(numOfBits == 0)
    {
        /* last word is full 32 bits */
        numOfBits = 32;
    }

    return prvCpssFieldValueSet(entryPtr,startBit,numOfBits,value);
}

/**
* @internal prvCpssFieldToEntry_GT_Any_Get function
* @endinternal
*
* @brief   Get 'any number of bits' value to a field in a table entry.
*         the bits returned are in little Endian order in the array of words.
* @param[in] entryPtr                 - pointer to memory.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[])
*
* @param[out] valueArr[]               - the array of values to get
*                                      RETURN:
*                                      GT_OK   -   on success
*                                      GT_BAD_PTR  - on NULL pointer
*                                      GT_BAD_PARAM    - on bad parameter : info.length == 0
*                                      COMMENTS:
*/
GT_STATUS prvCpssFieldToEntry_GT_Any_Get(
    IN GT_U32                           *entryPtr,
    IN const PRV_CPSS_ENTRY_FORMAT_TABLE_STC  fieldsInfoArr[],
    IN GT_U32                           fieldIndex,
    OUT GT_U32                          valueArr[]
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  numOfWords;/* number of words in the field */
    GT_U32  startBit;/* start bit of current section of the field */
    GT_U32  numOfBits;/* number of bits of current section of the field */
    GT_U32  value;/* value of current section of the field */

    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoArr);
    CPSS_NULL_PTR_CHECK_MAC(valueArr);

    if(fieldsInfoArr[fieldIndex].startBit == PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    else if(fieldsInfoArr[fieldIndex].numOfBits == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    numOfWords = (fieldsInfoArr[fieldIndex].numOfBits + 31) / 32;
    startBit = fieldsInfoArr[fieldIndex].startBit;
    numOfBits = 32;

    /* handle the 'Full words' (32 bits) sections */
    for(ii = 0 ; ii < (numOfWords - 1); ii++ , startBit += 32)
    {
        rc = prvCpssFieldValueGet(entryPtr,startBit,numOfBits,&value);
        if(rc != GT_OK)
        {
            return rc;
        }

        valueArr[ii] = value;
    }

    /* handle the last word */
    numOfBits = fieldsInfoArr[fieldIndex].numOfBits & 0x1f;
    if(numOfBits == 0)
    {
        /* last word is full 32 bits */
        numOfBits = 32;
    }

    rc = prvCpssFieldValueGet(entryPtr,startBit,numOfBits,&value);

    valueArr[ii] = value;

    return rc;
}



/**
* @internal prvCpssFillFieldsStartBitInfo function
* @endinternal
*
* @brief   Fill during init the 'start bit' of the fields in the table format.
*
* @param[in] numOfFields              - the number of elements in in fieldsInfoArr[].
* @param[in,out] fieldsInfoArr[]          - array of fields info
* @param[in,out] fieldsInfoArr[]          - array of fields info , after modify the <startBit> of the fields.
*                                      RETURN:
*                                      GT_OK   -   on success
*                                      GT_BAD_PTR  - on NULL pointer
*                                      COMMENTS:
*/
GT_STATUS prvCpssFillFieldsStartBitInfo(
    IN GT_U32                           numOfFields,
    INOUT PRV_CPSS_ENTRY_FORMAT_TABLE_STC   fieldsInfoArr[]
)
{
    GT_U32                      ii;
    PRV_CPSS_ENTRY_FORMAT_TABLE_STC *currentFieldInfoPtr;
    PRV_CPSS_ENTRY_FORMAT_TABLE_STC *prevFieldInfoPtr;
    GT_U32                      prevIndex;

    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoArr);

    for( ii = 0 ; ii < numOfFields ; ii++)
    {
        currentFieldInfoPtr = &fieldsInfoArr[ii];

        prevIndex = currentFieldInfoPtr->previousFieldType;

        if(currentFieldInfoPtr->startBit == PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS)
        {
            if(ii == 0)
            {
                /* first field got no options other then to start in bit 0 */
                currentFieldInfoPtr->startBit = 0;
            }
            else /* use the previous field info */
            {
                if(prevIndex == PRV_CPSS_FIELD_CONSECUTIVE_CNS)
                {
                    /* this field is consecutive to the previous field */
                    prevIndex = ii-1;
                }
                else
                {
                    /* this field come after other previous field */
                }

                prevFieldInfoPtr = &fieldsInfoArr[prevIndex];
                currentFieldInfoPtr->startBit = prevFieldInfoPtr->startBit + prevFieldInfoPtr->numOfBits;
            }
        }
        else
        {
            /* no need to calculate the start bit -- it is FORCED by the entry format */
        }
    }


    return GT_OK;
}

/**
* @internal prvCpssFieldInEntryInfoGet function
* @endinternal
*
* @brief   Get the start bit and the length of specific field in entry format .
*
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[])
*
* @param[out] startBitPtr              - (pointer to) the start bit of the field.
* @param[out] numOfBitsPtr             - (pointer to) the number of bits of the field.
*                                      RETURN:
*                                      GT_OK   -   on success
*                                      GT_BAD_PTR  - on NULL pointer
*                                      COMMENTS:
*/
GT_STATUS prvCpssFieldInEntryInfoGet(
    IN const PRV_CPSS_ENTRY_FORMAT_TABLE_STC  fieldsInfoArr[],
    IN GT_U32                           fieldIndex,
    OUT GT_U32                          *startBitPtr,
    OUT GT_U32                          *numOfBitsPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoArr);
    CPSS_NULL_PTR_CHECK_MAC(startBitPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfBitsPtr);

    *startBitPtr = fieldsInfoArr[fieldIndex].startBit;
    *numOfBitsPtr = fieldsInfoArr[fieldIndex].numOfBits;

    return GT_OK;
}

/* generic GT_U32 bitmap functions */

/**
* @internal prvCpssBitmapNextOneBitIndexFind function
* @endinternal
*
* @brief   Find index of next bit in bitmap that contains one.
*
* @param[in] bitmapArrPtr             - array of 32-bit words of bitmap
* @param[in] bitmapWordsNum           - amount of words in bitmap
* @param[in] startBit                 - index of start bit, to find first bit specify -1
*/
GT_32 prvCpssBitmapNextOneBitIndexFind
(
    IN   GT_U32   *bitmapArrPtr,
    IN   GT_U32   bitmapWordsNum,
    IN   GT_32    startBit
)
{
    GT_U32  searchFrom;
    GT_U32  wordIdx;
    GT_U32  bitIdx;
    GT_U32  word;
    GT_U32  shiftBits;
    GT_U32  shiftIn4Bits;
    /* table of 16 2-bit values of first one-bit 0..3             */
    /* index 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 */
    /* data  -, 0, 1, 0, 2, 0, 1, 0, 3, 0,  1,  0,  2,  0,  1,  0 */
    static GT_U32 firstBitTable = 0x12131210;

    /* look for word containing the bit being searched */
    searchFrom = (startBit < 0) ? 0 : (GT_U32)(startBit + 1) ;
    wordIdx = (searchFrom / 32);
    if (wordIdx >= bitmapWordsNum) return -1;

    word = bitmapArrPtr[wordIdx];
    bitIdx  = (searchFrom % 32);
    if (bitIdx != 0)
    {
        /* relevant for first checked word only */
        word &= (~ ((1 << bitIdx) - 1));
    }
    if (word == 0)
    {
        /* continue search not zero word */
        for (wordIdx++; (wordIdx < bitmapWordsNum); wordIdx++)
        {
            word = bitmapArrPtr[wordIdx];
            if (word != 0) break;
        }
    }

    /* not zero word not found */
    if (word == 0) return -1;

    /* look for first one bit in already found word */
    shiftBits = 0;
    if ((word & 0xFFFF) == 0)
    {
        shiftBits += 16;
        word >>= 16;
    }
    if ((word & 0xFF) == 0)
    {
        shiftBits += 8;
        word >>= 8;
    }
    if ((word & 0x0F) == 0)
    {
        shiftBits += 4;
        word >>= 4;
    }
    /* found non zero nibble - get 2-bit value from table */
    shiftIn4Bits = ((firstBitTable >> ((word & 0x0F) * 2)) & 3);
    return (GT_32)((32 * wordIdx) + shiftBits + shiftIn4Bits);
}

/**
* @internal prvCpssBitmapNextZeroBitIndexFind function
* @endinternal
*
* @brief   Find index of next bit in bitmap that contains zero.
*
* @param[in] bitmapArrPtr             - array of 32-bit words of bitmap
* @param[in] bitmapWordsNum           - amount of words in bitmap
* @param[in] startBit                 - index of start bit, to find first bit specify -1
*/
GT_32 prvCpssBitmapNextZeroBitIndexFind
(
    IN   GT_U32   *bitmapArrPtr,
    IN   GT_U32   bitmapWordsNum,
    IN   GT_32    startBit
)
{
    GT_U32  searchFrom;
    GT_U32  wordIdx;
    GT_U32  bitIdx;
    GT_U32  word;
    GT_U32  shiftBits;
    GT_U32  shiftIn4Bits;
    /* table of 16 2-bit values of first zero-bit 0..3             */
    /* index 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 */
    /* data  0, 1, 0, 2, 0, 1, 0, 3, 0, 1,  0,  2,  0,  1,  0,  - */
    static GT_U32 firstBitTable = 0x0484C484;

    /* look for word containing the bit being searched */
    searchFrom = (startBit < 0) ? 0 : (GT_U32)(startBit + 1) ;
    wordIdx = (searchFrom / 32);
    if (wordIdx >= bitmapWordsNum) return -1;

    word = bitmapArrPtr[wordIdx];
    bitIdx  = (searchFrom % 32);
    if (bitIdx != 0)
    {
        /* relevant for first checked word only */
        word |= ((1 << bitIdx) - 1);
    }
    if (word == 0xFFFFFFFF)
    {
        /* continue search not zero word */
        for (wordIdx++; (wordIdx < bitmapWordsNum); wordIdx++)
        {
            word = bitmapArrPtr[wordIdx];
            if (word != 0xFFFFFFFF) break;
        }
    }

    /* not full-ones word not found */
    if (word == 0xFFFFFFFF) return -1;

    /* look for first zero bit in already found word */
    shiftBits = 0;
    if ((word & 0xFFFF) == 0xFFFF)
    {
        shiftBits += 16;
        word >>= 16;
    }
    if ((word & 0xFF) == 0xFF)
    {
        shiftBits += 8;
        word >>= 8;
    }
    if ((word & 0x0F) == 0x0F)
    {
        shiftBits += 4;
        word >>= 4;
    }
    /* found non zero nibble - get 2-bit value from table */
    shiftIn4Bits = ((firstBitTable >> ((word & 0x0F) * 2)) & 3);
    return (GT_32)((32 * wordIdx) + shiftBits + shiftIn4Bits);
}

/**
* @internal prvCpssBitmapOneBitsCount function
* @endinternal
*
* @brief   Count amount of bits that contains one in bitmap.
*
* @param[in] bitmapArrPtr             - array of 32-bit words of bitmap
* @param[in] bitmapWordsNum           - amount of words in bitmap
*/
GT_32 prvCpssBitmapOneBitsCount
(
    IN   GT_U32   *bitmapArrPtr,
    IN   GT_U32    bitmapWordsNum
)
{
    GT_U32  wordIdx;
    GT_U32 word;
    GT_U32  numOfBits;
    GT_U32  shiftBits;
    /* table of 16 2-bit values of (numOfBits - 1) of one-bits 0..3 */
    /* index 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15   */
    /* data  0, 0, 0, 1, 0, 1, 1, 2, 0, 1,  1,  2,  1,  2,  2,  3   */
    static GT_32 numOfBitsTable = 0xE9949440;

    numOfBits = 0;
    for (wordIdx = 0; (wordIdx < bitmapWordsNum); wordIdx++)
    {
        word = bitmapArrPtr[wordIdx];
        if (word == 0) continue;
        for (shiftBits = 0; (shiftBits < 32); /*none*/)
        {
            if ((word & 0xFFFF) == 0)
            {
                shiftBits += 16;
                word >>= 16;
                continue;
            }
            if ((word & 0xFF) == 0)
            {
                shiftBits += 8;
                word >>= 8;
                continue;
            }
            if ((word & 0x0F) == 0)
            {
                shiftBits += 4;
                word >>= 4;
                continue;
            }
            /* found non zero nibble - get 2-bit value from table */
            numOfBits += (((numOfBitsTable >> ((word & 0x0F) * 2)) & 3) + 1);
            shiftBits += 4;
            word >>= 4;
        }
    }
    return numOfBits;
}

/**
* @internal prvCpssBitmapLastOneBitIndexInRangeFind function
* @endinternal
*
* @brief   Find index of last bit in range of bits in bitmap that contains one.
*
* @param[in] bitmapArrPtr            - array of 32-bit words of bitmap
* @param[in] startBit                - index of start bit of search range
* @param[in] endBit                  - index of end bit of search range
* @param[in] notFoundReturnValue     - value to return if all bits in range are zeros
*/
GT_U32 prvCpssBitmapLastOneBitIndexInRangeFind
(
    IN   GT_U32    *bitmapArrPtr,
    IN   GT_U32    startBit,
    IN   GT_U32    endBit,
    IN   GT_U32    notFoundReturnValue
)
{
    GT_32   firstWordIdx;
    GT_32   lastWordIdx;
    GT_32   firstWordMask;
    GT_32   lastWordMask;
    GT_32   wordIdx;
    GT_U32  word;
    GT_U32  shiftBits;
    GT_U32  shiftIn4Bits;
    /* 4-bit field last one bit 0..3                             */
    /* table of 16 2-bit values of last one-bit 0..3             */
    /* index 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 */
    /* data  -, 0, 1, 1, 2, 2, 2, 2, 3, 3,  3,  3,  3,  3,  3,  3 */
    static GT_U32 lastBitTable = 0xFFFFAA50;

    if (startBit > endBit) return notFoundReturnValue;

    word          = 0;
    firstWordIdx  = (GT_32)(startBit / 32);
    lastWordIdx   = (GT_32)(endBit / 32);
    firstWordMask = (GT_32)(0xFFFFFFFF << (startBit % 32));
    lastWordMask  = (GT_32)(((GT_U32)0xFFFFFFFF) >> (31 - (endBit % 32)));

    for (wordIdx = lastWordIdx; (wordIdx >= firstWordIdx); wordIdx--)
    {
        word = bitmapArrPtr[wordIdx];
        if (wordIdx == firstWordIdx) word &= firstWordMask;
        if (wordIdx == lastWordIdx) word &= lastWordMask;
        if (word != 0) break;
    }
    /* not found not-zero word */
    if (wordIdx < firstWordIdx) return notFoundReturnValue;

    /* search the highes one-bit in the word */
    shiftBits = 0;
    if (word & 0xFFFF0000)
    {
        shiftBits += 16;
        word >>= 16;
    }
    if (word & 0xFF00)
    {
        shiftBits += 8;
        word >>= 8;
    }
    if (word & 0xF0)
    {
        shiftBits += 4;
        word >>= 4;
    }
    /* found non zero nibble - get 2-bit value from table */
    shiftIn4Bits = ((lastBitTable >> ((word & 0x0F) * 2)) & 3);
    return (GT_32)((32 * wordIdx) + shiftBits + shiftIn4Bits);
}

/**
* @internal prvCpssBitmapRangeSet function
* @endinternal
*
* @brief   Set range of bits in bitmap to 0 or 1.
*
* @param[in] bitmapArrPtr             - array of 32-bit words of bitmap
* @param[in] startBit                 - index of start bit
* @param[in] numOfBits                - number of bits to set
* @param[in] value                    -  to set, any not zero treated as one
*/
GT_VOID prvCpssBitmapRangeSet
(
    IN   GT_U32    *bitmapArrPtr,
    IN   GT_U32    startBit,
    IN   GT_U32    numOfBits,
    IN   GT_U32    value
)
{
    GT_U32 wordIdx;
    GT_U32 low, high, mask, highZeros;

    while (numOfBits)
    {
        wordIdx = startBit / 32;
        low = startBit % 32;
        high = (low + numOfBits);
        if (high > 32) high = 32;

        /* update middle */
        if ((low == 0) && (high == 32))
        {
            bitmapArrPtr[wordIdx] = ((value == 0) ? 0 : 0xFFFFFFFF);
            startBit  += 32;
            numOfBits -= 32;
            continue;
        }

        /* update first or last word */
        highZeros = 32 - high;
        mask = ((GT_U32)(0xFFFFFFFF << (low + highZeros)) >> highZeros);
        if (value == 0)
        {
            bitmapArrPtr[wordIdx] &= (~ mask);
        }
        else
        {
            bitmapArrPtr[wordIdx] |= mask;
        }
        startBit  += (high - low);
        numOfBits -= (high - low);
    }
}

/**
* @internal prvCpssFieldToEntryAndMask_GT_U32_Set function
* @endinternal
*
* @brief   Set GT_U32 value to a field in a table entry and raise bits
*         appropriate to field in the mask
*         NOTE:the value MUST not be bigger then the max value for the field !
* @param[in] entryPtr                 - pointer to entry.
* @param[in] maskPtr                  - pointer to mask.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldId                  - the index of the field (used as index in fieldsInfoArr[])
* @param[in] value                    - the  to set
*/
GT_STATUS prvCpssFieldToEntryAndMask_GT_U32_Set
(
    IN GT_U32                           *entryPtr,
    IN GT_U32                           *maskPtr,
    IN const PRV_CPSS_ENTRY_FORMAT_TABLE_STC  fieldsInfoArr[],
    IN GT_U32                           fieldId,
    IN GT_U32                           value

)
{
    GT_STATUS rc;
    GT_U32    fieldStartBit;
    GT_U32    fieldLength;

    rc = prvCpssFieldToEntry_GT_U32_Set(entryPtr, fieldsInfoArr, fieldId, value);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* call prvCpssFieldInEntryInfoGet after prvCpssFieldToEntry_GT_U32_Set
       to be sure fieldStartBit, fieldLength are valid */
    rc = prvCpssFieldInEntryInfoGet(fieldsInfoArr,fieldId, &fieldStartBit, &fieldLength);
    if (rc != GT_OK)
    {
        return rc;
    }
    raiseBitsInMemory(maskPtr, fieldStartBit, fieldLength);
    return GT_OK;
}

/**
* @internal raiseBitsInMemory function
* @endinternal
*
* @brief   set specified bits in memory to 1.
*         Bits out of range specified by pair (startBit, numBits) stays the same.
* @param[in,out] memPtr                   - (pointer to) memory to update. if NULL do nothing
* @param[in] startBit                 - start bit in the memory to set.
* @param[in] numBits                  - number of bits (unlimited num of bits)
* @param[in,out] memPtr                   - (pointer to) updated memory
*                                      COMMENTS:
*/
void  raiseBitsInMemory(
    INOUT GT_U32                  *memPtr,
    IN    GT_U32                  startBit,
    IN    GT_U32                  numBits
)
{
    GT_U32 lastBit = (startBit + numBits - 1);
    GT_U32 i;
    GT_U32 firstIx;
    GT_U32 lastIx;

    if (memPtr == NULL)
    {
        return;
    }

    if (numBits)
    {
        firstIx   = startBit >> 5;  /* >>5 == /32 */
        lastIx    = lastBit  >> 5;  /* >>5 == /32 */
        startBit &= 0x1f;           /* &0x1f ==  %32 */
        lastBit  &= 0x1f;           /* &0x1f ==  %32 */

        /* set to 0xffffffff all range's words except first and last ones */
        for (i = firstIx + 1; i< lastIx; i++)
        {
            memPtr[i] = (GT_U32)(-1);
        }

        /* update first, last  words */
        if (firstIx != lastIx)
        {
            /* for first word: clear mask bits lower than startBit */
            memPtr[firstIx] |= LO_ZEROES_MAC(startBit);

            /* for last word: clear mask bits higher than lastBit */
            memPtr[lastIx] |= LO_ONES_MAC(lastBit+1);
        }
        else
        {
            /* first word is the last word. Combine (&) masks for first, last words  */
            memPtr[firstIx] |= LO_ZEROES_MAC(startBit) & LO_ONES_MAC(lastBit+1);
        }
    }
}

/**
* @internal printBitmapArr function
* @endinternal
*
* @brief   print a bitmap array .
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[in] bmpName                  - the 'name' of the BMP. (relevant only in case of error in expected_numOfSetBits)
* @param[in] bitmapArr                - pointer to bitmap array
* @param[in] numOfBits                - number of bits in bitmapArr (set or unset)
* @param[in] expected_numOfSetBits    - expected amount of set bits
*                                      value 0xFFFFFFFF meaning do not compare
*                                       number of bits that are set (not zero) in the bmp
*/
GT_U32 printBitmapArr
(
    IN GT_CHAR* bmpName,
    IN GT_U32   *bitmapArr,
    IN GT_U32   numOfBits,
    IN GT_U32   expected_numOfSetBits
)
{
    GT_U32 i,beginOfRange; /* indexes, helper variable for printing range */
    GT_BOOL inRange; /* helper variable for printing range */
    GT_U32  actualUsedAmount = 0;

    inRange = GT_FALSE;
    beginOfRange = 0;
    /* iterating over the bitmap */
    for (i = 0; i < numOfBits; i++)
    {
        if (0 == (bitmapArr[i>>5] & (1<<(i & 0x1f))))
        {
            /* end of range */
            if (inRange == GT_TRUE)
            {
                /* printing the range that just ended*/
                if ( i != 0 && ((i-1) != beginOfRange))
                {
                    cpssOsPrintf("..%d", i - 1);
                }
                if (i != (numOfBits-1))
                {
                    cpssOsPrintf(", ");
                }
                inRange = GT_FALSE;
            }
        }
        /* usedRulesBitmapArr[i] is on */
        else
        {
            actualUsedAmount++;
            /* beginning of range */
            if (inRange == GT_FALSE)
            {
                cpssOsPrintf("%d",i);
                inRange = GT_TRUE;
                beginOfRange = i;
            }
        }
    }
    /* dealing with the last range */
    if (inRange == GT_TRUE)
    {
        /* printing the range that just ended*/
        if ((i != 0) && ((i-1) != beginOfRange))
        {
            cpssOsPrintf("..%d", i - 1);
        }
    }

    if (actualUsedAmount == 0)
    {
        cpssOsPrintf("(empty)");
    }
    else
    {
        cpssOsPrintf("([%d] bits are set)",actualUsedAmount);
    }
    cpssOsPrintf("\n");

    if(expected_numOfSetBits != 0xFFFFFFFF &&
       actualUsedAmount != expected_numOfSetBits)
    {
        cpssOsPrintf("DB ERROR detected : [%s] : actualUsedAmount[%d] expected[%d] \n",
            bmpName,actualUsedAmount,expected_numOfSetBits);
    }

    return actualUsedAmount;
}

/**
* @internal prvCpssTimesampCompare function
* @endinternal
*
* @brief   compare two times from timestamps .
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[in] ts1Ptr    - pointer to first timestamp
* @param[in] ts2Ptr    - pointer to second timestamp
*
* @retval 1            - first is more than second
* @retval 0            - first is equal to second
* @retval -1           - first is less than second
*
*/
GT_32 prvCpssTimesampCompare
(
    IN PRV_CPSS_TIMESTAMP_STC *ts1Ptr,
    IN PRV_CPSS_TIMESTAMP_STC *ts2Ptr
)
{
    if (ts1Ptr->secondsHigh > ts2Ptr->secondsHigh) return 1;
    if (ts1Ptr->secondsHigh < ts2Ptr->secondsHigh) return -1;
    if (ts1Ptr->secondsLow > ts2Ptr->secondsLow) return 1;
    if (ts1Ptr->secondsLow < ts2Ptr->secondsLow) return -1;
    if (ts1Ptr->nanoseconds > ts2Ptr->nanoseconds) return 1;
    if (ts1Ptr->nanoseconds < ts2Ptr->nanoseconds) return -1;
    return 0;
}

/**
* @internal prvCpssTimesampLoad function
* @endinternal
*
* @brief   load timestamp from packet buffer.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[in] typestampFormat - sequence of symbols for timesamp layout:
*                              'S' or 's' - seconds, 'N' or 'n' - nanoseconds
*                              examle "SSSSSSNNNN" when packet is 010203040506070809
*                              secondsHigh = 0x0102, secondsLow = 03040506, nanoseconds = 0x06070809
* @param[in] packetTsPtr     - pointer to the part of packet containing timestamp
* @param[out] tsPtr          - pointer to timestamp to be loaded
*
* @retval GT_OK             - on success
* @retval GT_BAD_PARAM      - on wrong format
*
*/
GT_STATUS prvCpssTimesampLoad
(
    IN  const char             *typestampFormat,
    IN  const char             *packetTsPtr,
    OUT PRV_CPSS_TIMESTAMP_STC *tsPtr
)
{
    GT_U32 i;
    GT_U32 numOfBytesForSecondsLow;
    GT_U32 numOfBytesForSecondsHigh;
    GT_U32 numOfBytesForNanoSeconds;
    GT_U32 bytesForSecondsHighLoaded;

    tsPtr->secondsHigh = 0;
    tsPtr->secondsLow  = 0;
    tsPtr->nanoseconds = 0;

    /* parse format */
    numOfBytesForSecondsLow  = 0;
    numOfBytesForSecondsHigh = 0;
    numOfBytesForNanoSeconds = 0;
    for (i = 0; (typestampFormat[i] != 0); i++)
    {
        switch (typestampFormat[i])
        {
            case 'S':
            case 's':
                if (numOfBytesForSecondsLow < 4) { numOfBytesForSecondsLow ++; }
                else if (numOfBytesForSecondsHigh < 4) { numOfBytesForSecondsHigh ++;}
                else { CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);}
                break;
            case 'N':
            case 'n':
                if (numOfBytesForNanoSeconds < 4) { numOfBytesForNanoSeconds ++; }
                else { CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);}
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* load timestamp from packet buffer to structure */
    /* assumed that format is valid                   */
    bytesForSecondsHighLoaded = 0;
    for (i = 0; (typestampFormat[i] != 0); i++)
    {
        switch (typestampFormat[i])
        {
            case 'S':
            case 's':
                if (numOfBytesForSecondsHigh > bytesForSecondsHighLoaded)
                {
                    tsPtr->secondsHigh <<= 8;
                    tsPtr->secondsHigh |= (packetTsPtr[i] & 0xFF);
                    bytesForSecondsHighLoaded ++;
                }
                else
                {
                    tsPtr->secondsLow <<= 8;
                    tsPtr->secondsLow |= (packetTsPtr[i] & 0xFF);
                }
                break;
            case 'N':
            case 'n':
                tsPtr->nanoseconds <<= 8;
                tsPtr->nanoseconds |= (packetTsPtr[i] & 0xFF);
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssTimesampToLongNumber function
* @endinternal
*
* @brief   Convert timestamp to long (32 * 4) bits number containing time in nanoseconds.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[in] tsPtr       - pointer to source timestamp
* @param[out]lnArrPtr    - pointer long number members array
*
*/
GT_VOID prvCpssTimesampToLongNumber
(
    IN   PRV_CPSS_TIMESTAMP_STC *tsPtr,
    OUT  GT_U32                 *lnArrPtr
)
{
    lnArrPtr[0] = tsPtr->secondsLow;
    lnArrPtr[1] = tsPtr->secondsHigh;
    lnArrPtr[2] = 0;
    lnArrPtr[3] = 0;

    prvCpssLongNumberMultiply32(lnArrPtr, 4/*arrSize*/, 1000000000);
    prvCpssLongNumberAdd32(lnArrPtr, 4/*arrSize*/, tsPtr->nanoseconds);
}

/**
* @internal prvCpssTimesampFromLongNumber function
* @endinternal
*
* @brief   Convert long (32 * 4) bits number containing time in nanoseconds to timestamp.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[in]lnArrPtr    - pointer source long number members array
* @param[out] tsPtr     - pointer to calculated timestamp
*
*/
GT_VOID prvCpssTimesampFromLongNumber
(
    IN   GT_U32                 *lnArrPtr,
    OUT  PRV_CPSS_TIMESTAMP_STC *tsPtr
)
{
    GT_U32      workArr[4];
    GT_U32      reminder;

    workArr[0] = lnArrPtr[0];
    workArr[1] = lnArrPtr[1];
    workArr[2] = lnArrPtr[2];
    workArr[3] = lnArrPtr[3];
    prvCpssLongNumberDevide32(workArr, 4/*arrSize*/, 1000000000, &reminder);
    tsPtr->nanoseconds = reminder;
    tsPtr->secondsLow = workArr[0];
    tsPtr->secondsHigh= workArr[1];
}

/**
* @internal prvCpssTimesampScale function
* @endinternal
*
* @brief   Scale timestamp multiplying by given fraction.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[in] tsPtr          - pointer to source timestamp
* @param[in] numerator      - fraction numerator
* @param[in] denumerator    - fraction denumerator
* @param[out] resultTsPtr   - pointer to result timestamp,
*                             supported inplace recalculation when tsPtr == resultTsPtr
*
*/
GT_VOID prvCpssTimesampScale
(
    IN  PRV_CPSS_TIMESTAMP_STC *tsPtr,
    IN  GT_U32                 numerator,
    IN  GT_U32                 denumerator,
    OUT PRV_CPSS_TIMESTAMP_STC *resultTsPtr
)
{
    GT_U32      lnArr[4];
    GT_U32      reminder;

    prvCpssTimesampToLongNumber(tsPtr, lnArr);
    prvCpssLongNumberMultiply32(lnArr, 4/*arrSize*/, numerator);
    prvCpssLongNumberDevide32(lnArr, 4/*arrSize*/, denumerator, &reminder);
    prvCpssTimesampFromLongNumber(lnArr, resultTsPtr);
}


#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#endif
#ifdef PX_FAMILY
    #include <cpss/px/config/private/prvCpssPxInfo.h>
#endif

/*  enum to state the 'IPC of CPSS to WM' message types.
    SIM_IPC_TYPE_STRING_E - a free style string (str) that will be operated on the WM side with : osShellExecute(str)
    SIM_IPC_TYPE_PHA_THREAD_PIPE_E - message for PHA for threadId,firmwareInstructionPointer in pipe
    SIM_IPC_TYPE_PHA_THREAD_FALCON_E - message for PHA for threadId,firmwareInstructionPointer in falcon
    SIM_IPC_TYPE_PHA_THREAD_AC5P_AC5X_HARRIER_E - message for PHA for threadId,firmwareInstructionPointer in Ac5p,Ac5x and Harrier
    SIM_IPC_TYPE_DX_BYPASS_SOHO_DIRECT_REG_E - message for 'DX device' to by bypassed and trigger the SOHO address directly.
    SIM_IPC_TYPE_DISCONNECT_ALL_OUTER_PORTS_E - message for all devices to disconnect from outer ports
    SIM_IPC_TYPE_CORE_CLOCK_SET_E - message for the devices to set coreClock
    SIM_IPC_TYPE_SLAN_NAME_GET_E  - message to get from WN the name of SLAN of the port
    SIM_IPC_TYPE_SLAN_CONNECT_E   - message to connect SLAN to a port
    SIM_IPC_TYPE_SLAN_DISCONNECT_E- message to disconnect SLAN from a port
    SIM_IPC_TYPE_PORT_LINK_CHANGED_E - message to notify WM to change the link status of a port
    SIM_IPC_TYPE_TEST_SCIB_DMA_WRITE_64_BIT_ADDR_E - message to notify WM to start write to DMA to test it.
    SIM_IPC_TYPE_TEST_SCIB_DMA_READ_64_BIT_ADDR_E  - message to notify WM to start read from DMA to test it.

*/
enum {
    SIM_IPC_TYPE_STRING_E               = 0,
    SIM_IPC_TYPE_PHA_THREAD_PIPE_E,
    SIM_IPC_TYPE_PHA_THREAD_FALCON_E,
    SIM_IPC_TYPE_PHA_THREAD_AC5P_AC5X_HARRIER_E,
    SIM_IPC_TYPE_DX_BYPASS_SOHO_DIRECT_REG_E,
    SIM_IPC_TYPE_DISCONNECT_ALL_OUTER_PORTS_E,
    SIM_IPC_TYPE_CORE_CLOCK_SET_E,
    SIM_IPC_TYPE_SLAN_NAME_GET_E,
    SIM_IPC_TYPE_SLAN_CONNECT_E,
    SIM_IPC_TYPE_SLAN_DISCONNECT_E,
    SIM_IPC_TYPE_PORT_LINK_CHANGED_E,
    SIM_IPC_TYPE_TEST_SCIB_DMA_WRITE_64_BIT_ADDR_E,
    SIM_IPC_TYPE_TEST_SCIB_DMA_READ_64_BIT_ADDR_E,

    SIM_IPC_TYPE_PHA_THREAD___LAST___E = 0x7fffffff
};

/* code that supports the 'switch' that is WM device */
#ifdef GM_USED
    extern GT_STATUS   osShellExecute(IN  char*   command);
#endif /*GM_USED*/

/**
* @internal cpssTriggerIpcToWm function
* @endinternal
*
* @brief   The CPSS triggered 'string' to operate on the WM.
*/
static GT_STATUS internal_cpssTriggerIpcToWm (
    IN GT_U8  devNum,
    IN GT_U32 simIpcType,
    IN GT_U32 numParams,
    IN GT_U32 paramsArr[],
    IN char* command,
    OUT GT_U32  *outParam1Ptr, /* ignored if NULL */
    OUT GT_U32  *outParam2Ptr, /* ignored if NULL */
    OUT GT_U32  *outParam3Ptr  /* ignored if NULL */
)
{
    GT_STATUS rc;
#define NUM_OF_BYTES_CNS    (256-4/*enum type*/-4*(1/*last address (trigger)*/ + 3/*3 registers for 'Get' operation*/))
    GT_U32  ii;
    GT_U32  mg0base;
    GT_U32  startAddr   = 0x000FFF00;
    GT_U32  triggerAddr = 0x000FFFFC;
    GT_U32  getAddr     = 0x000FFFF0;
    GT_U32  value;
    GT_U32  commandLen;

    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS ||
       PRV_CPSS_PP_MAC(devNum) == NULL)
    {
        /* need to access the WM directly via it's BARs */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "None exists CPSS device [%d] (not implemented yet to access 'WM device') \n",
            devNum);
    }
#ifdef CHX_FAMILY
    else
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        mg0base = prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_MG_E, NULL);
    }
#endif
    else
    {
        mg0base = 0;
    }

    startAddr   += mg0base;
    triggerAddr += mg0base;
    getAddr     += mg0base;

    value = command == NULL ? simIpcType : SIM_IPC_TYPE_STRING_E;
    rc = prvCpssDrvHwPpWriteRegister(devNum,startAddr,value);
    if(rc != GT_OK)
    {
        return rc;
    }
    startAddr += 4;

    if(command == NULL)
    {
        if(numParams >= ((NUM_OF_BYTES_CNS/4)-1))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "command with [%d] params too long . max is [%d]",
                numParams , (NUM_OF_BYTES_CNS/4)-1);
        }
        value = numParams;
        for(ii = 0 ; ii < (numParams+1) ; ii++)
        {
            rc = prvCpssDrvHwPpWriteRegister(devNum,startAddr + ii*4,value);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(ii < numParams)
            {
                value = paramsArr[ii];
            }
        }
    }
    else
    {
#ifdef GM_USED
        if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
        {
            /* the GM not supports the tricks of the WM */
            /* this is needed for the 'wm_ssl' for example that came from
               'do shell-execute ssl'
            */
            return osShellExecute(command);
        }
#endif /*GM_USED*/

        commandLen = cpssOsStrlen(command);
        if(commandLen >= NUM_OF_BYTES_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "command[%s] too long [%d] max is [%d]",
                command , commandLen , NUM_OF_BYTES_CNS);
        }

        for(ii = 0 ; ii < ((commandLen+3)/4) ; ii++)
        {
            value =
                command[ii*4 + 0] << 24 |
                command[ii*4 + 1] << 16 |
                command[ii*4 + 2] <<  8 |
                command[ii*4 + 3] <<  0 ;

            rc = prvCpssDrvHwPpWriteRegister(devNum,startAddr + ii*4,value);
            if(rc != GT_OK)
            {
                return rc;
            }

        }

        if(0 == (commandLen % 4))
        {
            value = 0;/* put the '/0' of the end of the string , in the next word */
            rc = prvCpssDrvHwPpWriteRegister(devNum,startAddr + ii*4,value);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    value = 0xFFFFFFFF;/* trigger the action */
    rc = prvCpssDrvHwPpWriteRegister(devNum,triggerAddr,value);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(outParam1Ptr)
    {
        ii = 0;
        rc = prvCpssDrvHwPpReadRegister(devNum,getAddr + ii*4,outParam1Ptr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(outParam2Ptr)
        {
            ii++;
            rc = prvCpssDrvHwPpReadRegister(devNum,getAddr + ii*4,outParam2Ptr);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(outParam3Ptr)
            {
                ii++;
                rc = prvCpssDrvHwPpReadRegister(devNum,getAddr + ii*4,outParam3Ptr);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssTriggerIpcToWm_string function
* @endinternal
*
* @brief   The CPSS triggered 'string' to operate on the WM.
*/
GT_STATUS cpssTriggerIpcToWm_string (
    IN GT_U8  devNum,
    IN char* command,
    OUT GT_U32  *outParam1Ptr, /* ignored if NULL */
    OUT GT_U32  *outParam2Ptr, /* ignored if NULL */
    OUT GT_U32  *outParam3Ptr  /* ignored if NULL */
)
{
    GT_STATUS rc;

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    rc = internal_cpssTriggerIpcToWm(devNum,0,0,NULL,command,outParam1Ptr,outParam2Ptr,outParam3Ptr);

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal cpssTriggerIpcToWm_enum function
* @endinternal
*
* @brief   The CPSS triggered 'enum' to operate on the WM.
*/
GT_STATUS cpssTriggerIpcToWm_enum (
    IN GT_U8  devNum,
    IN GT_U32 simIpcType,
    IN GT_U32 numParams,
    IN GT_U32 paramsArr[],
    OUT GT_U32  *outParam1Ptr, /* ignored if NULL */
    OUT GT_U32  *outParam2Ptr, /* ignored if NULL */
    OUT GT_U32  *outParam3Ptr  /* ignored if NULL */
)
{
    GT_STATUS rc;

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    rc = internal_cpssTriggerIpcToWm(devNum,simIpcType,numParams,paramsArr,NULL,outParam1Ptr,outParam2Ptr,outParam3Ptr);

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal smemSohoDirectAccess function
* @endinternal
*
* @brief   The CPSS 'wrapper' to trigger the WM to : bypass the DX device and
*           trigger the SOHO address directly.
*/
void smemSohoDirectAccess(
    IN  GT_U8    deviceNumber,
    IN  GT_U32   DevAddr,
    IN  GT_U32   RegAddr,
    IN  GT_U32 * memPtr,
    IN  GT_BOOL   doRead/*GT_TRUE - read , GT_FALSE - write*/
)
{
    GT_U32 buff[5];
    GT_U8  devNum;

    buff[0] = deviceNumber;/*simulation devId*/
    buff[1] = DevAddr;
    buff[2] = RegAddr;
    buff[3] = *memPtr;/*write value*/
    buff[4] = doRead;

    for(devNum = 0 ;devNum < PRV_CPSS_MAX_PP_DEVICES_CNS; devNum++)
    {
        if(PRV_CPSS_PP_MAC(devNum))
        {
            (void) cpssTriggerIpcToWm_enum(devNum,SIM_IPC_TYPE_DX_BYPASS_SOHO_DIRECT_REG_E,
                5,buff,doRead ? memPtr : NULL,NULL,NULL);
            return;
        }
    }

    return;
}

/**
* @internal cpssSimSoftResetDoneWait function
* @endinternal
*
* @brief   The CPSS 'wrapper' to wait for the WM to : end the 'soft reset'.
*/
GT_STATUS cpssSimSoftResetDoneWait(void)
{
    GT_U32 pciBus, pciDev, pciFunc ,regAddr,data , found;
    GT_STATUS rc;
    GT_U32 kk = 1000;     /* Wait for a limited time */

    pciFunc = 0;
    regAddr = 0;
    found   = 0;
    pciDev  = 0;
    do
    {
        for(pciBus = 0; pciBus < _4K; pciBus++)
        {
            for(pciDev = 0; pciDev < 32; pciDev++)
            {
                rc = cpssExtDrvPciConfigReadReg(pciBus, pciDev, pciFunc ,regAddr,&data);
                if(rc == GT_OK && ((data & 0x0000FFFF) == 0x11ab))
                {
                    found = 1;
                    break;
                }
            }
            if(found)
            {
                break;
            }
        }
        if(found)
        {
            break;
        }
        cpssOsTimerWkAfter(5);
    }while(kk--);
    if(!found)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    regAddr = 0x38;/* reserved register ! */
    for(;kk;kk--)
    {
        rc = cpssExtDrvPciConfigReadReg(pciBus, pciDev, pciFunc ,regAddr,&data);
        if(rc == GT_OK && ((data & 0x80000000) == 0x80000000))
        {
            return GT_OK;
        }

        cpssOsTimerWkAfter(5);
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
}

#if !(defined ASIC_SIMULATION_ENV_FORBIDDEN) && (defined ASIC_SIMULATION)
extern void smemUpdateCoreClockRegister
(
    IN GT_U32   wmDevNum,
    IN GT_U32   coreClockInMHz
);
#endif /*!(defined ASIC_SIMULATION_ENV_FORBIDDEN) && (defined ASIC_SIMULATION)*/

/**
* @internal simCoreClockOverwrite function
* @endinternal
*
* @brief   The CPSS 'wrapper' to set the WM to : new core clock.
*/
GT_STATUS simCoreClockOverwrite
(
    GT_U32 coreClockMHz
)
{
    PRV_SHARED_GLOBAL_VAR_SET(
        commonMod.genericHwInitDir.simUserDefinedCoreClockMHz, coreClockMHz);

    /* the command need to be called before 'cpss init' so we don't use the cpssTriggerIpcToWm_enum(...) */
#if !(defined ASIC_SIMULATION_ENV_FORBIDDEN) && (defined ASIC_SIMULATION)
    /* we do direct call to simulation code */
    smemUpdateCoreClockRegister(0,coreClockMHz);
#endif /*!(defined ASIC_SIMULATION_ENV_FORBIDDEN) && (defined ASIC_SIMULATION)*/

    return GT_OK;
}

/**
* @internal simCoreClockOverwrittenGet function
* @endinternal
*
* @brief   The CPSS 'wrapper' to set the WM to : new core clock.
*/
GT_U32 simCoreClockOverwrittenGet
(
    GT_VOID
)
{
    return PRV_SHARED_GLOBAL_VAR_GET(
        commonMod.genericHwInitDir.simUserDefinedCoreClockMHz);
}

/**
* @internal internal_cpssSimFirmwareThreadsInfoSet function
* @endinternal
*
* @brief   The CPSS 'wrapper' to set the WM to PHA info .
*/
static GT_STATUS internal_cpssSimFirmwareThreadsInfoSet
(
    IN GT_U32   cpssDevNum,
    IN GT_U32   simIpcEnum,
    IN GT_U32   threadId,
    IN GT_U32   firmwareInstructionPointer,
    IN GT_U32   firmwareImageId,
    IN GT_U32   firmwareVersionId,
    IN GT_BOOL  firstTime
)
{
    GT_U32 buff[5];
    GT_U8   devNum = (GT_U8)cpssDevNum;

    buff[0]=threadId;
    buff[1]=firmwareInstructionPointer;
    buff[2]=firmwareImageId;
    buff[3]=firmwareVersionId;
    buff[4]=firstTime;

    return cpssTriggerIpcToWm_enum(devNum,simIpcEnum,5,buff,NULL,NULL,NULL);
}

/**
* @internal cpssSimPipeFirmwareThreadIdToInstructionPointerSet function
* @endinternal
*
* @brief   The CPSS 'wrapper' to set the WM to PHA info for PIPE.
*/
GT_STATUS cpssSimPipeFirmwareThreadIdToInstructionPointerSet(
    IN GT_U32 cpssDevNum,
    IN GT_U32 threadId,
    IN GT_U32 firmwareInstructionPointer
)
{
    return internal_cpssSimFirmwareThreadsInfoSet(
        cpssDevNum,
        SIM_IPC_TYPE_PHA_THREAD_PIPE_E,
        threadId,firmwareInstructionPointer,0/*not used in pipe*/,0/*not used in pipe*/,0/*not used in pipe*/);
}
/**
* @internal cpssSimAc5pFirmwareThreadsInfoSet function
* @endinternal
*
* @brief   The CPSS 'wrapper' to set the WM to PHA info for Ac5p, Ac5x and Harrier.
*/
GT_STATUS cpssSimAc5pFirmwareThreadsInfoSet(
    IN GT_U32   cpssDevNum,
    IN GT_U32   threadId,
    IN GT_U32   firmwareInstructionPointer,
    IN GT_U32   firmwareImageId,
    IN GT_U32   firmwareVersionId,
    IN GT_BOOL  firstTime
)
{
    return internal_cpssSimFirmwareThreadsInfoSet(
        cpssDevNum,
        SIM_IPC_TYPE_PHA_THREAD_AC5P_AC5X_HARRIER_E,
        threadId,firmwareInstructionPointer,firmwareImageId,firmwareVersionId,firstTime);
}
/**
* @internal cpssSimFalconFirmwareThreadsInfoSet function
* @endinternal
*
* @brief   The CPSS 'wrapper' to set the WM to PHA info for Falcon.
*/
GT_STATUS cpssSimFalconFirmwareThreadsInfoSet(
    IN GT_U32   cpssDevNum,
    IN GT_U32   threadId,
    IN GT_U32   firmwareInstructionPointer,
    IN GT_U32   firmwareImageId,
    IN GT_U32   firmwareVersionId,
    IN GT_BOOL  firstTime
)
{
    return internal_cpssSimFirmwareThreadsInfoSet(
        cpssDevNum,
        SIM_IPC_TYPE_PHA_THREAD_FALCON_E,
        threadId,firmwareInstructionPointer,firmwareImageId,firmwareVersionId,firstTime);
}

#ifdef GM_USED
extern GT_STATUS skernelDevPortSlanGet
(
    IN  GT_U32                deviceId,
    IN  GT_U32                portNum,
    IN  GT_U32                slanMaxLength,
    OUT GT_BOOL              *portBoundPtr,
    OUT GT_CHAR              *slanNamePtr
);
extern GT_STATUS skernelBindDevPort2Slan
(
    IN  GT_U32                      deviceId,
    IN  GT_U32                      portNum,
    IN  GT_CHAR                    *slanNamePtr,
    IN  GT_BOOL                     unbindOtherPortsOnThisSlan
);
extern GT_STATUS skernelUnbindDevPort2Slan
(
    IN  GT_U32                      deviceId,
    IN  GT_U32                      portNum
);

#endif /*GM_USED*/
/**
* @internal cpssSimDevPortSlanGet function
* @endinternal
*
* @brief   The CPSS 'wrapper' to skernelDevPortSlanGet.
*/
GT_STATUS cpssSimDevPortSlanGet
(
    IN  GT_U32                      cpssDevNum,
    IN  GT_U32                wmDevNum,
    IN  GT_U32                macNum,
    IN  GT_U32                slanMaxLength,
    OUT GT_BOOL              *portBoundPtr,
    OUT GT_CHAR              *slanNamePtr
)
{
#ifdef GM_USED
    cpssDevNum    = cpssDevNum;
    /* get the info */
    return skernelDevPortSlanGet(wmDevNum,macNum,slanMaxLength,portBoundPtr,slanNamePtr);
#else /*!GM_USED*/

    GT_STATUS   rc;
    GT_U32 buff[2];
    GT_U32 outBuff[3]={0,0,0};
    GT_U8  devNum;
    GT_U32  ii;

    slanMaxLength = slanMaxLength;

    buff[0] = wmDevNum;
    buff[1] = macNum;

    devNum =  (GT_U8)cpssDevNum;
    {
        if(PRV_CPSS_PP_MAC(devNum))
        {
            rc = cpssTriggerIpcToWm_enum(devNum,SIM_IPC_TYPE_SLAN_NAME_GET_E,
                2,buff,&outBuff[0],&outBuff[1],&outBuff[2]);
            if(rc != GT_OK)
            {
                return rc;
            }

            for(ii = 0 ; ii < 3 ; ii ++)
            {
                slanNamePtr[ii*4 + 0] = (GT_CHAR)(outBuff[ii] >> 24);
                slanNamePtr[ii*4 + 1] = (GT_CHAR)(outBuff[ii] >> 16);
                slanNamePtr[ii*4 + 2] = (GT_CHAR)(outBuff[ii] >>  8);
                slanNamePtr[ii*4 + 3] = (GT_CHAR)(outBuff[ii] >>  0);
            }

            *portBoundPtr = (slanNamePtr[0] == 0) ? GT_FALSE : GT_TRUE;

            return GT_OK;
        }
    }

    *portBoundPtr  = GT_FALSE;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
#endif /*!GM_USED*/
}

/**
* @internal cpssSimDevPortSlanBind function
* @endinternal
*
* @brief   The CPSS 'wrapper' to skernelBindDevPort2Slan.
*/
GT_STATUS cpssSimDevPortSlanBind
(
    IN  GT_U32                      cpssDevNum,
    IN  GT_U32                      wmDevNum,
    IN  GT_U32                      macNum,
    IN  GT_CHAR                    *slanNamePtr,
    IN  GT_BOOL                     unbindOtherPortsOnThisSlan
)
{
#ifdef GM_USED
    cpssDevNum    = cpssDevNum;
    /* get the info */
    return skernelBindDevPort2Slan(wmDevNum,macNum,slanNamePtr,unbindOtherPortsOnThisSlan);
#else /*!GM_USED*/
    GT_U32 buff[6];
    GT_U8  devNum;
    GT_U32  ii;
    GT_CHAR  slanNameBuff[13];
    GT_U32  maxLen;

    buff[0] = wmDevNum;
    buff[1] = macNum;
    buff[2] = unbindOtherPortsOnThisSlan;

    cpssOsMemSet(slanNameBuff,0,13);
    maxLen = cpssOsStrlen(slanNamePtr);
    maxLen = maxLen >= 12 ? 12 : maxLen;
    cpssOsStrNCpy(slanNameBuff,slanNamePtr,maxLen);

    for(ii = 0 ; ii < 3 ; ii ++)
    {
        buff[ii+3] = slanNameBuff[ii*4 + 0] << 24 |
                     slanNameBuff[ii*4 + 1] << 16 |
                     slanNameBuff[ii*4 + 2] <<  8 |
                     slanNameBuff[ii*4 + 3] <<  0  ;
    }

    devNum = (GT_U8)cpssDevNum;

    {
        if(PRV_CPSS_PP_MAC(devNum))
        {
            return cpssTriggerIpcToWm_enum(devNum,SIM_IPC_TYPE_SLAN_CONNECT_E,
                6,buff,NULL,NULL,NULL);
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
#endif /*!GM_USED*/
}
/**
* @internal cpssSimDevPortSlanUnbind function
* @endinternal
*
* @brief   The CPSS 'wrapper' to skernelUnbindDevPort2Slan.
*/
GT_STATUS cpssSimDevPortSlanUnbind
(
    IN  GT_U32                      cpssDevNum,
    IN  GT_U32                      wmDevNum,
    IN  GT_U32                      macNum
)
{
#ifdef GM_USED
    cpssDevNum    = cpssDevNum;
    /* get the info */
    return skernelUnbindDevPort2Slan(wmDevNum,macNum);
#else /*!GM_USED*/
    GT_U32 buff[2];
    GT_U8  devNum;

    buff[0] = wmDevNum;
    buff[1] = macNum;

    devNum = (GT_U8)cpssDevNum;

    {
        if(PRV_CPSS_PP_MAC(devNum))
        {
            return cpssTriggerIpcToWm_enum(devNum,SIM_IPC_TYPE_SLAN_DISCONNECT_E,
                2,buff,NULL,NULL,NULL);
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
#endif /*!GM_USED*/
}


/**
* @internal skernelPortLinkStatusChange function
* @endinternal
*
* @brief   The CPSS 'wrapper' to skernelPortLinkStatusChange.
*/
GT_STATUS cpssSimDevPortLinkStatusChange
(
    IN  GT_U32                      cpssDevNum,
    IN  GT_U32   wmDevNum,
    IN  GT_U32   macNum,
    IN  GT_BOOL  newStatus
)
{
    GT_U32 buff[3];
    GT_U8  devNum;

    buff[0] = wmDevNum;
    buff[1] = macNum;
    buff[2] = newStatus;

    devNum = (GT_U8)cpssDevNum;

    {
        if(PRV_CPSS_PP_MAC(devNum))
        {
            return cpssTriggerIpcToWm_enum(devNum,SIM_IPC_TYPE_PORT_LINK_CHANGED_E,
                3,buff,NULL,NULL,NULL);
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}


/**
* @internal simDisconnectAllOuterPorts function
* @endinternal
*
* @brief   set all WM devices to disconnect from outer ports
*/
GT_STATUS simDisconnectAllOuterPorts(void)
{
    GT_STATUS   rc;
    GT_U8   devNum;

    for(devNum = 0 ;devNum < PRV_CPSS_MAX_PP_DEVICES_CNS; devNum++)
    {
        if(PRV_CPSS_PP_MAC(devNum))
        {
            rc = cpssTriggerIpcToWm_enum(devNum,SIM_IPC_TYPE_DISCONNECT_ALL_OUTER_PORTS_E,0,NULL,NULL,NULL,NULL);
            cpssOsTimerWkAfter(100);/* allow to handle all the interrupts due to link change */
            return rc;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}
/**
* @internal cpssSimNoParams function
* @endinternal
*
* @brief   The CPSS 'wrapper' to set the WM to generic string without parameters.
*/
static GT_STATUS cpssSimNoParams(IN char* name)
{
    char    buff[256];
    GT_U8   devNum;
    GT_U32  length;

    length = cpssOsStrlen(name);
    if(length >= (256 - 4 /*wm__*/ - 4/* ZERO padding */))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "buff size is not enough for [%s]",
            name);
    }

    cpssOsSprintf(buff,"wm__%s",
        name);

    length = cpssOsStrlen(buff);
    /*fix for JIRA : CPSS-11636 : WM simulation log file doe snot open with dbg simulation startSimulationLog */
    /* make sure that the arithmetic of : will not cause wrong word value !!! (GCC compiler nonsense!) */
    /*  in function : internal_cpssTriggerIpcToWm

            the compiler converts the 'char[]' into 'int' and negative value of
                char[] ... cause 25 bits of '1' (bits 7..31) in the value.

                so need to make sure not 'negative values' in the char[]

            value =
                command[ii*4 + 0] << 24 |
                command[ii*4 + 1] << 16 |
                command[ii*4 + 2] <<  8 |
                command[ii*4 + 3] <<  0 ;
    */
    /* so putting 'well defined' ZEROs as a padding at the end of the string */
    buff[length  ] = 0;
    buff[length+1] = 0;
    buff[length+2] = 0;
    buff[length+3] = 0;

    for(devNum = 0 ;devNum < PRV_CPSS_MAX_PP_DEVICES_CNS; devNum++)
    {
        if(PRV_CPSS_PP_MAC(devNum))
        {
            return cpssTriggerIpcToWm_string(devNum,buff,NULL,NULL,NULL);
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

#define CPSS_SIM_NO_PARAMS_MAC(name)    \
GT_STATUS name(void)                    \
{                                       \
    return cpssSimNoParams(#name);      \
}
/* do implementation of the WM 'log' functions to be available via the IPC mechanism */
CPSS_SIM_NO_PARAMS_MAC(ssl);
CPSS_SIM_NO_PARAMS_MAC(sslw);
CPSS_SIM_NO_PARAMS_MAC(nssl);
CPSS_SIM_NO_PARAMS_MAC(sslf);
CPSS_SIM_NO_PARAMS_MAC(startSimulationLog);
CPSS_SIM_NO_PARAMS_MAC(startSimulationLogWithFromCpu);
CPSS_SIM_NO_PARAMS_MAC(stopSimulationLog);
CPSS_SIM_NO_PARAMS_MAC(startSimulationLogFull);

/**
* @internal cpssSimDebugDmaRead64BitAddr function
* @endinternal
*
* @brief   The CPSS 'wrapper' to wmForDebugDmaRead64BitAddr.
*/
GT_STATUS cpssSimDebugDmaRead64BitAddr(
    IN GT_U32   cpssDevNum,
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords
)
{
    GT_U32 buff[3];
    GT_U8  devNum;

    buff[0] = addr_high;
    buff[1] = addr_low;
    buff[2] = numOfWords;

    devNum = (GT_U8)cpssDevNum;

    {
        if(PRV_CPSS_PP_MAC(devNum))
        {
            return cpssTriggerIpcToWm_enum(devNum,SIM_IPC_TYPE_TEST_SCIB_DMA_READ_64_BIT_ADDR_E,
                3,buff,NULL,NULL,NULL);
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}
/**
* @internal cpssSimDebugDmaWrite64BitAddr function
* @endinternal
*
* @brief   The CPSS 'wrapper' to wmForDebugDmaWrite64BitAddr.
*/
GT_STATUS cpssSimDebugDmaWrite64BitAddr(
    IN GT_U32   cpssDevNum,
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords,
    IN GT_U32   pattern
)
{
    GT_U32 buff[4];
    GT_U8  devNum;

    buff[0] = addr_high;
    buff[1] = addr_low;
    buff[2] = numOfWords;
    buff[3] = pattern;

    devNum = (GT_U8)cpssDevNum;

    {
        if(PRV_CPSS_PP_MAC(devNum))
        {
            return cpssTriggerIpcToWm_enum(devNum,SIM_IPC_TYPE_TEST_SCIB_DMA_WRITE_64_BIT_ADDR_E,
                4,buff,NULL,NULL,NULL);
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}


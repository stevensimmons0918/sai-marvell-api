/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file cpssDxChBrgFdbHash.c
*
* @brief Hash calculate for MAC address table implementation.
*
* @version   20
********************************************************************************
*/

/* get the device info and common info */
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgLog.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrg.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbHash.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbHashCrc.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>



#define RV_CPSS_DXCH_FDB_HASH_MAX_BITS_NUM_CNS 14

#define PRV_GET_BIT_MAC(word,bitNum) ((word) >> (bitNum))

/* get number of bits use calc according to used FDB size : XOR and CRC */
#define NUM_BITS_TO_USE_GET_MAC(_size,_numOfBitsToUse)                \
    switch (_size)   \
    {                                                                \
        case CPSS_DXCH_BRG_FDB_TBL_SIZE_0_E:                         \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,              \
            "The FDB table is ZERO size and can't be used for hash calculations"); \
                                                                     \
        case CPSS_DXCH_BRG_FDB_TBL_SIZE_4K_E:                        \
            (_numOfBitsToUse) = 10;                                  \
            break;                                                   \
                                                                     \
        case CPSS_DXCH_BRG_FDB_TBL_SIZE_8K_E:                        \
            (_numOfBitsToUse) = 11;                                  \
            break;                                                   \
                                                                     \
        case CPSS_DXCH_BRG_FDB_TBL_SIZE_16K_E:                       \
            (_numOfBitsToUse) = 12;                                  \
            break;                                                   \
                                                                     \
        case CPSS_DXCH_BRG_FDB_TBL_SIZE_32K_E:                       \
            (_numOfBitsToUse) = 13;                                  \
            break;                                                   \
                                                                     \
        case CPSS_DXCH_BRG_FDB_TBL_SIZE_64K_E:                       \
            (_numOfBitsToUse) = 14;                                  \
            break;                                                   \
                                                                     \
        case CPSS_DXCH_BRG_FDB_TBL_SIZE_128K_E:                      \
            (_numOfBitsToUse) = 15;                                  \
            break;                                                   \
                                                                     \
        case CPSS_DXCH_BRG_FDB_TBL_SIZE_256K_E:                      \
            (_numOfBitsToUse) = 16;                                  \
            break;                                                   \
                                                                     \
        default:                                                     \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);     \
    }

/* get number of bits use calc according to used FDB size : CRC multi hash mode
   the hash value is one of 16 banks as appose to 'legacy' mode that
   value is one of 4 ... so there are 2 bits that not needed */
#define NUM_BITS_TO_USE_GET_MULTI_HASH_MODE_MAC(_dev,_numOfBitsToUse) \
    NUM_BITS_TO_USE_GET_MAC(PRV_CPSS_DXCH_PP_MAC(_dev)->bridge.fdbHashParams.size,_numOfBitsToUse);                    \
    (_numOfBitsToUse) -= 2

/* get number of banks used in multiple hash calculation */
#define NUM_BANKS_TO_USE_GET_MAC(_multipleHashes,_numOfBanksToUse)              \
    switch (_multipleHashes)                                                    \
    {                                                                           \
        case 0:                                                                 \
        case 16:                                                                \
            (_numOfBanksToUse) = 16;                                            \
            break;                                                              \
        case 4:                                                                 \
            (_numOfBanksToUse) = 4;                                             \
            break;                                                              \
        case 8:                                                                 \
            (_numOfBanksToUse) = 8;                                             \
            break;                                                              \
        default:                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);      \
    }

/* factor for the FDB index out from the CRC/XOR calculations */
#define LEGACY_NUM_BANKS_FACTOR_CNS     4

/* factor for the FDB index out from the CRC/XOR calculations */
#define NUM_BANKS_MULTI_HASH_CNS     16

#define GT_MAC_LOW32_MAC(macAddr)                \
                (macAddr[5] |          \
                (macAddr[4] << 8) |    \
                (macAddr[3] << 16) |   \
                (macAddr[2] << 24))

#define GT_MAC_HIGH16_MAC(macAddr)           \
        (macAddr[1] | (macAddr[0] << 8))

/* typedef for all functions like:
GT_VOID crc_X_d36(GT_U32 *D, GT_U32 *c, GT_U32 *h);

where X is one of : 16a,16b,16c,16d,32a,32b,32k,32q,64

*/
/*
 * Typedef: function CRC_TYPE_FUNC
 *
 * Description: typedef for all functions like:
 *   GT_VOID crc_X_d36(GT_U32 *D, GT_U32 *c, GT_U32 *h);
 *   where X is one of : 16a,16b,16c,16d,32a,32b,32k,32q,64
 * Fields:
 *      D - array of data.
 *      c - array of crc init values.
 *      h - CRC 16/32/64 bits result (array of 1 or 2 words).
 */
typedef GT_VOID (*CRC_TYPE_FUNC)
(
    IN  GT_U32 *D,
    IN  GT_U32 *c,
    OUT GT_U32 *h
);

/**
* @struct CRC_FUNC_STC
 *
 * @brief Defines the CRC functions that needed for calculation of up to 140 bits.
 * for Sip5.
 * the 44+36+60 = 140 allow 140 bits data
 * the 60+36  = 96 allow 96 bits data
 * the 44+36  = 80 allow 80 bits data
 * the 44    = 44 allow 44 bits data
*/
typedef struct{

    /** call back for function that get 36 bits data and do hash calc. */
    CRC_TYPE_FUNC calc36BitsData;

    /** call back for function that get 44 bits data and do hash calc. */
    CRC_TYPE_FUNC calc44BitsData;

    /** call back for function that get 60 bits data and do hash calc. */
    CRC_TYPE_FUNC calc60BitsData;

} CRC_FUNC_STC;

/* array of actual functions that the SIP5 uses for calculation of up to 140 bits data*/
/* relevant only to <Multi Hash Enable> = 1 */
static const CRC_FUNC_STC crcFuncArray[] =
{
    /*CRC_FUNC_16A_E*/     {prvCpssDxChBrgFdbHashCrc_16a_d36,   prvCpssDxChBrgFdbHashCrc_16a_d44,    prvCpssDxChBrgFdbHashCrc_16a_d60}
    /*CRC_FUNC_16B_E*/    ,{prvCpssDxChBrgFdbHashCrc_16b_d36,   prvCpssDxChBrgFdbHashCrc_16b_d44,    prvCpssDxChBrgFdbHashCrc_16b_d60}
    /*CRC_FUNC_16C_E*/    ,{prvCpssDxChBrgFdbHashCrc_16c_d36,   prvCpssDxChBrgFdbHashCrc_16c_d44,    prvCpssDxChBrgFdbHashCrc_16c_d60}
    /*CRC_FUNC_16D_E*/    ,{prvCpssDxChBrgFdbHashCrc_16d_d36,   prvCpssDxChBrgFdbHashCrc_16d_d44,    prvCpssDxChBrgFdbHashCrc_16d_d60}
    /*CRC_FUNC_32A_E*/    ,{prvCpssDxChBrgFdbHashCrc_32a_d36,   prvCpssDxChBrgFdbHashCrc_32a_d44,    prvCpssDxChBrgFdbHashCrc_32a_d60}
    /*CRC_FUNC_32B_E*/    ,{prvCpssDxChBrgFdbHashCrc_32b_d36,   prvCpssDxChBrgFdbHashCrc_32b_d44,    prvCpssDxChBrgFdbHashCrc_32b_d60}
    /*CRC_FUNC_32K_E*/    ,{prvCpssDxChBrgFdbHashCrc_32k_d36,   prvCpssDxChBrgFdbHashCrc_32k_d44,    prvCpssDxChBrgFdbHashCrc_32k_d60}
    /*CRC_FUNC_32Q_E*/    ,{prvCpssDxChBrgFdbHashCrc_32q_d36,   prvCpssDxChBrgFdbHashCrc_32q_d44,    prvCpssDxChBrgFdbHashCrc_32q_d60}
    /*CRC_FUNC_64_E */    ,{prvCpssDxChBrgFdbHashCrc_64_d36 ,   prvCpssDxChBrgFdbHashCrc_64_d44 ,    prvCpssDxChBrgFdbHashCrc_64_d60 }
};

#define BIT_AS_END_BIT_CNS  (0x00010000)

/**
* @struct CRC_FUNC_BITS_USED_INFO_STC
 *
 * @brief Defines info about start bit for every hash bank
*/
typedef struct{

    /** type of hash. */
    CRC_FUNC_ENT funcType;

    /** the start bit to use in the hash result of 'funcType' */
    GT_U32 startBit;

} CRC_FUNC_BITS_USED_INFO_STC;

/* relevant only to <Multi Hash Enable> = 1 */
static const CRC_FUNC_BITS_USED_INFO_STC crcFuncBitsUsedArr[NUM_BANKS_MULTI_HASH_CNS] =
{
    {CRC_FUNC_16A_E       ,   0}, /*[X-1:0]*/

    {CRC_FUNC_16B_E       ,   0}, /*[X-1:0]*/

    {CRC_FUNC_16C_E       ,   0}, /*[X-1:0]*/

    {CRC_FUNC_16D_E       ,   0}, /*[X-1:0]*/

    {CRC_FUNC_32A_E       ,   0}, /*[X-1:0]*/
    {CRC_FUNC___DUMMY__E  ,  16}, /*[16+x-1:16]*/

    {CRC_FUNC_32B_E       ,   0}, /*[X-1:0]*/
    {CRC_FUNC___DUMMY__E  ,  16}, /*[16+x-1:16]*/

    {CRC_FUNC_32K_E       ,   0}, /*[X-1:0]*/
    {CRC_FUNC___DUMMY__E  ,  16}, /*[16+x-1:16]*/  /* NOT --> [31:31-X+1]*/

    {CRC_FUNC_32Q_E       ,   0}, /*[X-1:0]*/
    {CRC_FUNC___DUMMY__E  ,  16}, /*[16+x-1:16]*/  /* NOT --> [31:31-X+1]*/

    {CRC_FUNC_64_E        ,   0}, /*[X-1:0]*/
    {CRC_FUNC___DUMMY__E  ,  16}, /*[16+x-1:16]*/
    {CRC_FUNC___DUMMY__E  ,  32}, /*[32+X-1:32]*/
    {CRC_FUNC___DUMMY__E  ,  48}  /*[48+x-1:48]*/

};

/**
* @internal hashCalcDataXor function
* @endinternal
*
* @brief   This function calculates the hash index for the
*         mac address table using XOR algorithm.
* @param[in] dataPtr                  - array of 32bit words, contain data.
* @param[in] numOfWords               - amount of words of data
*
* @param[out] hashPtr                  - the hash index.
*                                       none
*/
static GT_VOID hashCalcDataXor
(
    IN  GT_U32          *dataPtr,
    IN  GT_U32          numOfWords,
    OUT GT_U32          *hashPtr
)
{
    GT_U32  idx ;

    /* compute the transposed hash */
    (*hashPtr) = dataPtr[0];
    for (idx = 1; idx < numOfWords; idx++)
    {
        (*hashPtr) ^= dataPtr[idx];
    }

    return ;
}

/**
* @internal crc_11_12 function
* @endinternal
*
* @brief   This function calculates CRC (11-12 bits)
*
* @param[in] dataPtr                  - array of 32bit words, contain data
* @param[in] bitsWidth                - num of bits used in hash
* @param[in] useZeroInitValueForCrcHash - Use zero for initialization value of CRC function
*
* @param[out] hash_bitPtr              - the hash bits array
*                                       None.
*/
static GT_VOID crc_11_12
(
    IN  GT_U32  *dataPtr,
    IN  GT_U32   bitsWidth,
    OUT GT_U32  *hash_bitPtr,
    IN  GT_U32   useZeroInitValueForCrcHash
)
{
    GT_U32  *D = dataPtr;
    GT_U32  *hash_bit = hash_bitPtr;

    hash_bit[0] =
                 PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],8)  ^ PRV_GET_BIT_MAC(D[2],6)  ^
                 PRV_GET_BIT_MAC(D[2],5)  ^ PRV_GET_BIT_MAC(D[2],2)  ^ PRV_GET_BIT_MAC(D[2],0)  ^
                 PRV_GET_BIT_MAC(D[1],31) ^ PRV_GET_BIT_MAC(D[1],29) ^ PRV_GET_BIT_MAC(D[1],28) ^
                 PRV_GET_BIT_MAC(D[1],27) ^ PRV_GET_BIT_MAC(D[1],26) ^ PRV_GET_BIT_MAC(D[1],22) ^
                 PRV_GET_BIT_MAC(D[1],20) ^ PRV_GET_BIT_MAC(D[1],19) ^ PRV_GET_BIT_MAC(D[1],18) ^
                 PRV_GET_BIT_MAC(D[1],17) ^ PRV_GET_BIT_MAC(D[1],16) ^ PRV_GET_BIT_MAC(D[1],15) ^
                 PRV_GET_BIT_MAC(D[1],12) ^ PRV_GET_BIT_MAC(D[1],3)  ^ PRV_GET_BIT_MAC(D[1],2)  ^
                 PRV_GET_BIT_MAC(D[1],1)  ^ PRV_GET_BIT_MAC(D[0],30) ^ PRV_GET_BIT_MAC(D[0],29) ^
                 PRV_GET_BIT_MAC(D[0],26) ^ PRV_GET_BIT_MAC(D[0],25) ^ PRV_GET_BIT_MAC(D[0],24) ^
                 PRV_GET_BIT_MAC(D[0],23) ^ PRV_GET_BIT_MAC(D[0],22) ^ PRV_GET_BIT_MAC(D[0],17) ^
                 PRV_GET_BIT_MAC(D[0],16) ^ PRV_GET_BIT_MAC(D[0],15) ^ PRV_GET_BIT_MAC(D[0],14) ^
                 PRV_GET_BIT_MAC(D[0],13) ^ PRV_GET_BIT_MAC(D[0],12) ^ PRV_GET_BIT_MAC(D[0],11) ^
                 PRV_GET_BIT_MAC(D[0],8)  ^ PRV_GET_BIT_MAC(D[0],7)  ^ PRV_GET_BIT_MAC(D[0],6)  ^
                 PRV_GET_BIT_MAC(D[0],5)  ^ PRV_GET_BIT_MAC(D[0],4)  ^ PRV_GET_BIT_MAC(D[0],3)  ^
                 PRV_GET_BIT_MAC(D[0],2)  ^ PRV_GET_BIT_MAC(D[0],1)  ^ PRV_GET_BIT_MAC(D[0],0)  ;

   hash_bit[1] =
                 PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],9)  ^ PRV_GET_BIT_MAC(D[2],8)  ^
                 PRV_GET_BIT_MAC(D[2],7)  ^ PRV_GET_BIT_MAC(D[2],5)  ^ PRV_GET_BIT_MAC(D[2],3)  ^
                 PRV_GET_BIT_MAC(D[2],2)  ^ PRV_GET_BIT_MAC(D[2],1)  ^ PRV_GET_BIT_MAC(D[1],31) ^
                 PRV_GET_BIT_MAC(D[1],30) ^ PRV_GET_BIT_MAC(D[1],26) ^ PRV_GET_BIT_MAC(D[1],23) ^
                 PRV_GET_BIT_MAC(D[1],22) ^ PRV_GET_BIT_MAC(D[1],21) ^ PRV_GET_BIT_MAC(D[1],15) ^
                 PRV_GET_BIT_MAC(D[1],13) ^ PRV_GET_BIT_MAC(D[1],12) ^ PRV_GET_BIT_MAC(D[1],4)  ^
                 PRV_GET_BIT_MAC(D[1],1)  ^ PRV_GET_BIT_MAC(D[0],31) ^ PRV_GET_BIT_MAC(D[0],29) ^
                 PRV_GET_BIT_MAC(D[0],27) ^ PRV_GET_BIT_MAC(D[0],22) ^ PRV_GET_BIT_MAC(D[0],18) ^
                 PRV_GET_BIT_MAC(D[0],11) ^ PRV_GET_BIT_MAC(D[0],9)  ^ PRV_GET_BIT_MAC(D[0],0)  ;

   hash_bit[2] = PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],10) ^ PRV_GET_BIT_MAC(D[2],9)  ^
                 PRV_GET_BIT_MAC(D[2],5)  ^ PRV_GET_BIT_MAC(D[2],4)  ^ PRV_GET_BIT_MAC(D[2],3)  ^
                 PRV_GET_BIT_MAC(D[1],29) ^ PRV_GET_BIT_MAC(D[1],28) ^ PRV_GET_BIT_MAC(D[1],26) ^
                 PRV_GET_BIT_MAC(D[1],24) ^ PRV_GET_BIT_MAC(D[1],23) ^ PRV_GET_BIT_MAC(D[1],20) ^
                 PRV_GET_BIT_MAC(D[1],19) ^ PRV_GET_BIT_MAC(D[1],18) ^ PRV_GET_BIT_MAC(D[1],17) ^
                 PRV_GET_BIT_MAC(D[1],15) ^ PRV_GET_BIT_MAC(D[1],14) ^ PRV_GET_BIT_MAC(D[1],13) ^
                 PRV_GET_BIT_MAC(D[1],12) ^ PRV_GET_BIT_MAC(D[1],5)  ^ PRV_GET_BIT_MAC(D[1],3)  ^
                 PRV_GET_BIT_MAC(D[1],1)  ^ PRV_GET_BIT_MAC(D[1],0)  ^ PRV_GET_BIT_MAC(D[0],29) ^
                 PRV_GET_BIT_MAC(D[0],28) ^ PRV_GET_BIT_MAC(D[0],26) ^ PRV_GET_BIT_MAC(D[0],25) ^
                 PRV_GET_BIT_MAC(D[0],24) ^ PRV_GET_BIT_MAC(D[0],22) ^ PRV_GET_BIT_MAC(D[0],19) ^
                 PRV_GET_BIT_MAC(D[0],17) ^ PRV_GET_BIT_MAC(D[0],16) ^ PRV_GET_BIT_MAC(D[0],15) ^
                 PRV_GET_BIT_MAC(D[0],14) ^ PRV_GET_BIT_MAC(D[0],13) ^ PRV_GET_BIT_MAC(D[0],11) ^
                 PRV_GET_BIT_MAC(D[0],10) ^ PRV_GET_BIT_MAC(D[0],8)  ^ PRV_GET_BIT_MAC(D[0],7)  ^
                 PRV_GET_BIT_MAC(D[0],6)  ^ PRV_GET_BIT_MAC(D[0],5)  ^ PRV_GET_BIT_MAC(D[0],4)  ^
                 PRV_GET_BIT_MAC(D[0],3)  ^ PRV_GET_BIT_MAC(D[0],2)  ^ PRV_GET_BIT_MAC(D[0],0)  ;

   hash_bit[3] =
                 PRV_GET_BIT_MAC(D[2],10) ^ PRV_GET_BIT_MAC(D[2],8) ^ PRV_GET_BIT_MAC(D[2],4)   ^
                 PRV_GET_BIT_MAC(D[2],2)  ^ PRV_GET_BIT_MAC(D[2],0) ^ PRV_GET_BIT_MAC(D[1],31) ^
                 PRV_GET_BIT_MAC(D[1],30) ^ PRV_GET_BIT_MAC(D[1],28) ^ PRV_GET_BIT_MAC(D[1],26) ^
                 PRV_GET_BIT_MAC(D[1],25) ^ PRV_GET_BIT_MAC(D[1],24) ^ PRV_GET_BIT_MAC(D[1],22) ^
                 PRV_GET_BIT_MAC(D[1],21) ^ PRV_GET_BIT_MAC(D[1],17) ^ PRV_GET_BIT_MAC(D[1],14) ^
                 PRV_GET_BIT_MAC(D[1],13) ^ PRV_GET_BIT_MAC(D[1],12) ^ PRV_GET_BIT_MAC(D[1],6)  ^
                 PRV_GET_BIT_MAC(D[1],4)  ^ PRV_GET_BIT_MAC(D[1],3)  ^ PRV_GET_BIT_MAC(D[0],27) ^
                 PRV_GET_BIT_MAC(D[0],24) ^ PRV_GET_BIT_MAC(D[0],22) ^ PRV_GET_BIT_MAC(D[0],20) ^
                 PRV_GET_BIT_MAC(D[0],18) ^ PRV_GET_BIT_MAC(D[0],13) ^ PRV_GET_BIT_MAC(D[0],9)  ^
                 PRV_GET_BIT_MAC(D[0],2)  ^ PRV_GET_BIT_MAC(D[0],0);

   hash_bit[4] = PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],9)  ^ PRV_GET_BIT_MAC(D[2],5)  ^
                 PRV_GET_BIT_MAC(D[2],3)  ^ PRV_GET_BIT_MAC(D[2],1)  ^ PRV_GET_BIT_MAC(D[2],0)  ^
                 PRV_GET_BIT_MAC(D[1],31) ^ PRV_GET_BIT_MAC(D[1],29) ^ PRV_GET_BIT_MAC(D[1],27) ^
                 PRV_GET_BIT_MAC(D[1],26) ^ PRV_GET_BIT_MAC(D[1],25) ^ PRV_GET_BIT_MAC(D[1],23) ^
                 PRV_GET_BIT_MAC(D[1],22) ^ PRV_GET_BIT_MAC(D[1],18) ^ PRV_GET_BIT_MAC(D[1],15) ^
                 PRV_GET_BIT_MAC(D[1],14) ^ PRV_GET_BIT_MAC(D[1],13) ^ PRV_GET_BIT_MAC(D[1],7)  ^
                 PRV_GET_BIT_MAC(D[1],5)  ^ PRV_GET_BIT_MAC(D[1],4)  ^ PRV_GET_BIT_MAC(D[0],28) ^
                 PRV_GET_BIT_MAC(D[0],25) ^ PRV_GET_BIT_MAC(D[0],23) ^ PRV_GET_BIT_MAC(D[0],21) ^
                 PRV_GET_BIT_MAC(D[0],19) ^ PRV_GET_BIT_MAC(D[0],14) ^ PRV_GET_BIT_MAC(D[0],10) ^
                 PRV_GET_BIT_MAC(D[0],3)  ^ PRV_GET_BIT_MAC(D[0],1)  ;

   hash_bit[5] = PRV_GET_BIT_MAC(D[2],10) ^ PRV_GET_BIT_MAC(D[2],6)  ^ PRV_GET_BIT_MAC(D[2],4)  ^
                 PRV_GET_BIT_MAC(D[2],2)  ^ PRV_GET_BIT_MAC(D[2],1)  ^ PRV_GET_BIT_MAC(D[2],0)  ^
                 PRV_GET_BIT_MAC(D[1],30) ^ PRV_GET_BIT_MAC(D[1],28) ^ PRV_GET_BIT_MAC(D[1],27) ^
                 PRV_GET_BIT_MAC(D[1],26) ^ PRV_GET_BIT_MAC(D[1],24) ^ PRV_GET_BIT_MAC(D[1],23) ^
                 PRV_GET_BIT_MAC(D[1],19) ^ PRV_GET_BIT_MAC(D[1],16) ^ PRV_GET_BIT_MAC(D[1],15) ^
                 PRV_GET_BIT_MAC(D[1],14) ^ PRV_GET_BIT_MAC(D[1],8)  ^ PRV_GET_BIT_MAC(D[1],6)  ^
                 PRV_GET_BIT_MAC(D[1],5)  ^ PRV_GET_BIT_MAC(D[0],29) ^ PRV_GET_BIT_MAC(D[0],26) ^
                 PRV_GET_BIT_MAC(D[0],24) ^ PRV_GET_BIT_MAC(D[0],22) ^ PRV_GET_BIT_MAC(D[0],20) ^
                 PRV_GET_BIT_MAC(D[0],15) ^ PRV_GET_BIT_MAC(D[0],11) ^ PRV_GET_BIT_MAC(D[0],4)  ^
                 PRV_GET_BIT_MAC(D[0],2)  ;

   hash_bit[6] =
                 PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],7)  ^ PRV_GET_BIT_MAC(D[2],5)  ^
                 PRV_GET_BIT_MAC(D[2],3)  ^ PRV_GET_BIT_MAC(D[2],2)  ^ PRV_GET_BIT_MAC(D[2],1)  ^
                 PRV_GET_BIT_MAC(D[1],31) ^ PRV_GET_BIT_MAC(D[1],29) ^ PRV_GET_BIT_MAC(D[1],28) ^
                 PRV_GET_BIT_MAC(D[1],27) ^ PRV_GET_BIT_MAC(D[1],25) ^ PRV_GET_BIT_MAC(D[1],24) ^
                 PRV_GET_BIT_MAC(D[1],20) ^ PRV_GET_BIT_MAC(D[1],17) ^ PRV_GET_BIT_MAC(D[1],16) ^
                 PRV_GET_BIT_MAC(D[1],15) ^ PRV_GET_BIT_MAC(D[1],9)  ^ PRV_GET_BIT_MAC(D[1],7)  ^
                 PRV_GET_BIT_MAC(D[1],6)  ^ PRV_GET_BIT_MAC(D[0],30) ^ PRV_GET_BIT_MAC(D[0],27) ^
                 PRV_GET_BIT_MAC(D[0],25) ^ PRV_GET_BIT_MAC(D[0],23) ^ PRV_GET_BIT_MAC(D[0],21) ^
                 PRV_GET_BIT_MAC(D[0],16) ^ PRV_GET_BIT_MAC(D[0],12) ^ PRV_GET_BIT_MAC(D[0],5)  ^
                 PRV_GET_BIT_MAC(D[0],3)  ;

   hash_bit[7] = PRV_GET_BIT_MAC(D[2],8)  ^ PRV_GET_BIT_MAC(D[2],6)  ^ PRV_GET_BIT_MAC(D[2],4)  ^
                 PRV_GET_BIT_MAC(D[2],3)  ^ PRV_GET_BIT_MAC(D[2],2)  ^ PRV_GET_BIT_MAC(D[2],0)  ^
                 PRV_GET_BIT_MAC(D[1],30) ^ PRV_GET_BIT_MAC(D[1],29) ^ PRV_GET_BIT_MAC(D[1],28) ^
                 PRV_GET_BIT_MAC(D[1],26) ^ PRV_GET_BIT_MAC(D[1],25) ^ PRV_GET_BIT_MAC(D[1],21) ^
                 PRV_GET_BIT_MAC(D[1],18) ^ PRV_GET_BIT_MAC(D[1],17) ^ PRV_GET_BIT_MAC(D[1],16) ^
                 PRV_GET_BIT_MAC(D[1],10) ^ PRV_GET_BIT_MAC(D[1],8)  ^ PRV_GET_BIT_MAC(D[1],7)  ^
                 PRV_GET_BIT_MAC(D[0],31) ^ PRV_GET_BIT_MAC(D[0],28) ^ PRV_GET_BIT_MAC(D[0],26) ^
                 PRV_GET_BIT_MAC(D[0],24) ^ PRV_GET_BIT_MAC(D[0],22) ^ PRV_GET_BIT_MAC(D[0],17) ^
                 PRV_GET_BIT_MAC(D[0],13) ^ PRV_GET_BIT_MAC(D[0],6)  ^ PRV_GET_BIT_MAC(D[0],4)  ;

   hash_bit[8] = PRV_GET_BIT_MAC(D[2],9)  ^ PRV_GET_BIT_MAC(D[2],7)  ^ PRV_GET_BIT_MAC(D[2],5)  ^
                 PRV_GET_BIT_MAC(D[2],4)  ^ PRV_GET_BIT_MAC(D[2],3)  ^ PRV_GET_BIT_MAC(D[2],1)  ^
                 PRV_GET_BIT_MAC(D[1],31) ^ PRV_GET_BIT_MAC(D[1],30) ^ PRV_GET_BIT_MAC(D[1],29) ^
                 PRV_GET_BIT_MAC(D[1],27) ^ PRV_GET_BIT_MAC(D[1],26) ^ PRV_GET_BIT_MAC(D[1],22) ^
                 PRV_GET_BIT_MAC(D[1],19) ^ PRV_GET_BIT_MAC(D[1],18) ^ PRV_GET_BIT_MAC(D[1],17) ^
                 PRV_GET_BIT_MAC(D[1],11) ^ PRV_GET_BIT_MAC(D[1],9)  ^ PRV_GET_BIT_MAC(D[1],8)  ^
                 PRV_GET_BIT_MAC(D[1],0)  ^ PRV_GET_BIT_MAC(D[0],29) ^ PRV_GET_BIT_MAC(D[0],27) ^
                 PRV_GET_BIT_MAC(D[0],25) ^ PRV_GET_BIT_MAC(D[0],23) ^ PRV_GET_BIT_MAC(D[0],18) ^
                 PRV_GET_BIT_MAC(D[0],14) ^ PRV_GET_BIT_MAC(D[0],7)  ^ PRV_GET_BIT_MAC(D[0],5)  ;

   hash_bit[9] = PRV_GET_BIT_MAC(D[2],10) ^ PRV_GET_BIT_MAC(D[2],8)  ^ PRV_GET_BIT_MAC(D[2],6)  ^
                 PRV_GET_BIT_MAC(D[2],5)  ^ PRV_GET_BIT_MAC(D[2],4)  ^ PRV_GET_BIT_MAC(D[2],2)  ^
                 PRV_GET_BIT_MAC(D[2],0)  ^ PRV_GET_BIT_MAC(D[1],31) ^ PRV_GET_BIT_MAC(D[1],30) ^
                 PRV_GET_BIT_MAC(D[1],28) ^ PRV_GET_BIT_MAC(D[1],27) ^ PRV_GET_BIT_MAC(D[1],23) ^
                 PRV_GET_BIT_MAC(D[1],20) ^ PRV_GET_BIT_MAC(D[1],19) ^ PRV_GET_BIT_MAC(D[1],18) ^
                 PRV_GET_BIT_MAC(D[1],12) ^ PRV_GET_BIT_MAC(D[1],10) ^ PRV_GET_BIT_MAC(D[1],9)  ^
                 PRV_GET_BIT_MAC(D[1],1)  ^ PRV_GET_BIT_MAC(D[0],30) ^ PRV_GET_BIT_MAC(D[0],28) ^
                 PRV_GET_BIT_MAC(D[0],26) ^ PRV_GET_BIT_MAC(D[0],24) ^ PRV_GET_BIT_MAC(D[0],19) ^
                 PRV_GET_BIT_MAC(D[0],15) ^ PRV_GET_BIT_MAC(D[0],8)  ^ PRV_GET_BIT_MAC(D[0],6);

   hash_bit[10] = PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],9)  ^ PRV_GET_BIT_MAC(D[2],7)  ^
                  PRV_GET_BIT_MAC(D[2],6)  ^ PRV_GET_BIT_MAC(D[2],5)  ^ PRV_GET_BIT_MAC(D[2],3)  ^
                  PRV_GET_BIT_MAC(D[2],1)  ^ PRV_GET_BIT_MAC(D[2],0)  ^ PRV_GET_BIT_MAC(D[1],31) ^
                  PRV_GET_BIT_MAC(D[1],29) ^ PRV_GET_BIT_MAC(D[1],28) ^ PRV_GET_BIT_MAC(D[1],24) ^
                  PRV_GET_BIT_MAC(D[1],21) ^ PRV_GET_BIT_MAC(D[1],20) ^ PRV_GET_BIT_MAC(D[1],19) ^
                  PRV_GET_BIT_MAC(D[1],13) ^ PRV_GET_BIT_MAC(D[1],11) ^ PRV_GET_BIT_MAC(D[1],10) ^
                  PRV_GET_BIT_MAC(D[1],2)  ^ PRV_GET_BIT_MAC(D[0],31) ^ PRV_GET_BIT_MAC(D[0],29) ^
                  PRV_GET_BIT_MAC(D[0],27) ^ PRV_GET_BIT_MAC(D[0],25) ^ PRV_GET_BIT_MAC(D[0],20) ^
                  PRV_GET_BIT_MAC(D[0],16) ^ PRV_GET_BIT_MAC(D[0],9)  ^ PRV_GET_BIT_MAC(D[0],7)  ;

    if (bitsWidth == 12)
    {
        hash_bit[11] = PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],10) ^ PRV_GET_BIT_MAC(D[2],7)  ^
                       PRV_GET_BIT_MAC(D[2],5)  ^ PRV_GET_BIT_MAC(D[2],4)  ^ PRV_GET_BIT_MAC(D[2],1)  ^
                       PRV_GET_BIT_MAC(D[1],31) ^ PRV_GET_BIT_MAC(D[1],30) ^ PRV_GET_BIT_MAC(D[1],28) ^
                       PRV_GET_BIT_MAC(D[1],27) ^ PRV_GET_BIT_MAC(D[1],26) ^ PRV_GET_BIT_MAC(D[1],25) ^
                       PRV_GET_BIT_MAC(D[1],21) ^ PRV_GET_BIT_MAC(D[1],19) ^ PRV_GET_BIT_MAC(D[1],18) ^
                       PRV_GET_BIT_MAC(D[1],17) ^ PRV_GET_BIT_MAC(D[1],16) ^ PRV_GET_BIT_MAC(D[1],15) ^
                       PRV_GET_BIT_MAC(D[1],14) ^ PRV_GET_BIT_MAC(D[1],11) ^ PRV_GET_BIT_MAC(D[1],2)  ^
                       PRV_GET_BIT_MAC(D[1],1)  ^ PRV_GET_BIT_MAC(D[1],0)  ^ PRV_GET_BIT_MAC(D[0],29) ^
                       PRV_GET_BIT_MAC(D[0],28) ^ PRV_GET_BIT_MAC(D[0],25) ^ PRV_GET_BIT_MAC(D[0],24) ^
                       PRV_GET_BIT_MAC(D[0],23) ^ PRV_GET_BIT_MAC(D[0],22) ^ PRV_GET_BIT_MAC(D[0],21) ^
                       PRV_GET_BIT_MAC(D[0],16) ^ PRV_GET_BIT_MAC(D[0],15) ^ PRV_GET_BIT_MAC(D[0],14) ^
                       PRV_GET_BIT_MAC(D[0],13) ^ PRV_GET_BIT_MAC(D[0],12) ^ PRV_GET_BIT_MAC(D[0],11) ^
                       PRV_GET_BIT_MAC(D[0],10) ^ PRV_GET_BIT_MAC(D[0],7)  ^ PRV_GET_BIT_MAC(D[0],6)  ^
                       PRV_GET_BIT_MAC(D[0],5)  ^ PRV_GET_BIT_MAC(D[0],4)  ^ PRV_GET_BIT_MAC(D[0],3)  ^
                       PRV_GET_BIT_MAC(D[0],2)  ^ PRV_GET_BIT_MAC(D[0],1)  ^ PRV_GET_BIT_MAC(D[0],0)  ;
    }
    else
    {
        hash_bit[11] = 0;
    }

    /* take care of init values */
    if (!useZeroInitValueForCrcHash)
    {
        hash_bit[3] ^= 1;
        hash_bit[9] ^= 1;
    }
}

/**
* @internal crc_13 function
* @endinternal
*
* @brief   This function calculates CRC (13 bits)
*
* @param[in] dataPtr                  - array of 32bit words, contain data
* @param[in] useZeroInitValueForCrcHash - Use zero for initialization value of CRC function.
*
* @param[out] hash_bitPtr              - the hash bits array
*                                       None.
*/
static GT_VOID crc_13
(
    IN  GT_U32  *dataPtr,
    OUT GT_U32  *hash_bitPtr,
    IN  GT_U32   useZeroInitValueForCrcHash
)
{
    GT_U32  *D = dataPtr;
    GT_U32  *hash_bit = hash_bitPtr;

    hash_bit[0] =
                PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],10) ^ PRV_GET_BIT_MAC(D[2],9)  ^
                PRV_GET_BIT_MAC(D[2],7)  ^ PRV_GET_BIT_MAC(D[2],4)  ^ PRV_GET_BIT_MAC(D[2],2)  ^
                PRV_GET_BIT_MAC(D[2],0)  ^
                PRV_GET_BIT_MAC(D[1],31) ^ PRV_GET_BIT_MAC(D[1],30) ^
                PRV_GET_BIT_MAC(D[1],29) ^ PRV_GET_BIT_MAC(D[1],28) ^ PRV_GET_BIT_MAC(D[1],26) ^
                PRV_GET_BIT_MAC(D[1],25) ^ PRV_GET_BIT_MAC(D[1],24) ^ PRV_GET_BIT_MAC(D[1],23) ^
                PRV_GET_BIT_MAC(D[1],20) ^ PRV_GET_BIT_MAC(D[1],13) ^ PRV_GET_BIT_MAC(D[1],11) ^
                PRV_GET_BIT_MAC(D[1],10) ^ PRV_GET_BIT_MAC(D[1],9)  ^ PRV_GET_BIT_MAC(D[1],7)  ^
                PRV_GET_BIT_MAC(D[1],5)  ^ PRV_GET_BIT_MAC(D[1],3)  ^ PRV_GET_BIT_MAC(D[1],2)  ^
                PRV_GET_BIT_MAC(D[1],1)  ^ PRV_GET_BIT_MAC(D[1],0)  ^ PRV_GET_BIT_MAC(D[0],31) ^
                PRV_GET_BIT_MAC(D[0],28) ^ PRV_GET_BIT_MAC(D[0],26) ^ PRV_GET_BIT_MAC(D[0],25) ^
                PRV_GET_BIT_MAC(D[0],21) ^ PRV_GET_BIT_MAC(D[0],19) ^ PRV_GET_BIT_MAC(D[0],18) ^
                PRV_GET_BIT_MAC(D[0],17) ^ PRV_GET_BIT_MAC(D[0],14) ^ PRV_GET_BIT_MAC(D[0],13) ^
                PRV_GET_BIT_MAC(D[0],12) ^ PRV_GET_BIT_MAC(D[0],11) ^ PRV_GET_BIT_MAC(D[0],9)  ^
                PRV_GET_BIT_MAC(D[0],7)  ^ PRV_GET_BIT_MAC(D[0],6)  ^ PRV_GET_BIT_MAC(D[0],4)  ^
                PRV_GET_BIT_MAC(D[0],3)  ^ PRV_GET_BIT_MAC(D[0],1)  ^ PRV_GET_BIT_MAC(D[0],0);

    hash_bit[1] =
                PRV_GET_BIT_MAC(D[2],9)  ^ PRV_GET_BIT_MAC(D[2],8)  ^ PRV_GET_BIT_MAC(D[2],7)  ^
                PRV_GET_BIT_MAC(D[2],5)  ^ PRV_GET_BIT_MAC(D[2],4)  ^ PRV_GET_BIT_MAC(D[2],3)  ^
                PRV_GET_BIT_MAC(D[2],2)  ^ PRV_GET_BIT_MAC(D[2],1)  ^
                PRV_GET_BIT_MAC(D[1],28) ^
                PRV_GET_BIT_MAC(D[1],27) ^ PRV_GET_BIT_MAC(D[1],23) ^ PRV_GET_BIT_MAC(D[1],21) ^
                PRV_GET_BIT_MAC(D[1],20) ^ PRV_GET_BIT_MAC(D[1],14) ^ PRV_GET_BIT_MAC(D[1],13) ^
                PRV_GET_BIT_MAC(D[1],12) ^ PRV_GET_BIT_MAC(D[1],9)  ^ PRV_GET_BIT_MAC(D[1],8)  ^
                PRV_GET_BIT_MAC(D[1],7)  ^ PRV_GET_BIT_MAC(D[1],6)  ^ PRV_GET_BIT_MAC(D[1],5)  ^
                PRV_GET_BIT_MAC(D[1],4)  ^ PRV_GET_BIT_MAC(D[0],31) ^ PRV_GET_BIT_MAC(D[0],29) ^
                PRV_GET_BIT_MAC(D[0],28) ^ PRV_GET_BIT_MAC(D[0],27) ^ PRV_GET_BIT_MAC(D[0],25) ^
                PRV_GET_BIT_MAC(D[0],22) ^ PRV_GET_BIT_MAC(D[0],21) ^ PRV_GET_BIT_MAC(D[0],20) ^
                PRV_GET_BIT_MAC(D[0],17) ^ PRV_GET_BIT_MAC(D[0],15) ^ PRV_GET_BIT_MAC(D[0],11) ^
                PRV_GET_BIT_MAC(D[0],10) ^ PRV_GET_BIT_MAC(D[0],9)  ^ PRV_GET_BIT_MAC(D[0],8)  ^
                PRV_GET_BIT_MAC(D[0],6)  ^ PRV_GET_BIT_MAC(D[0],5)  ^ PRV_GET_BIT_MAC(D[0],3)  ^
                PRV_GET_BIT_MAC(D[0],2)  ^ PRV_GET_BIT_MAC(D[0],0);

    hash_bit[2] =
                PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],8)  ^ PRV_GET_BIT_MAC(D[2],7)  ^
                PRV_GET_BIT_MAC(D[2],6)  ^ PRV_GET_BIT_MAC(D[2],5)  ^ PRV_GET_BIT_MAC(D[2],3)  ^
                PRV_GET_BIT_MAC(D[2],0)  ^
                PRV_GET_BIT_MAC(D[1],31) ^ PRV_GET_BIT_MAC(D[1],30) ^
                PRV_GET_BIT_MAC(D[1],26) ^ PRV_GET_BIT_MAC(D[1],25) ^ PRV_GET_BIT_MAC(D[1],23) ^
                PRV_GET_BIT_MAC(D[1],22) ^ PRV_GET_BIT_MAC(D[1],21) ^ PRV_GET_BIT_MAC(D[1],20) ^
                PRV_GET_BIT_MAC(D[1],15) ^ PRV_GET_BIT_MAC(D[1],14) ^ PRV_GET_BIT_MAC(D[1],11) ^
                PRV_GET_BIT_MAC(D[1],8)  ^ PRV_GET_BIT_MAC(D[1],6)  ^ PRV_GET_BIT_MAC(D[1],3)  ^
                PRV_GET_BIT_MAC(D[1],2)  ^ PRV_GET_BIT_MAC(D[1],1)  ^ PRV_GET_BIT_MAC(D[0],31) ^
                PRV_GET_BIT_MAC(D[0],30) ^ PRV_GET_BIT_MAC(D[0],29) ^ PRV_GET_BIT_MAC(D[0],25) ^
                PRV_GET_BIT_MAC(D[0],23) ^ PRV_GET_BIT_MAC(D[0],22) ^ PRV_GET_BIT_MAC(D[0],19) ^
                PRV_GET_BIT_MAC(D[0],17) ^ PRV_GET_BIT_MAC(D[0],16) ^ PRV_GET_BIT_MAC(D[0],14) ^
                PRV_GET_BIT_MAC(D[0],13) ^ PRV_GET_BIT_MAC(D[0],10) ^ PRV_GET_BIT_MAC(D[0],0);

    hash_bit[3] =
                PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],10) ^ PRV_GET_BIT_MAC(D[2],8)  ^
                PRV_GET_BIT_MAC(D[2],6)  ^ PRV_GET_BIT_MAC(D[2],2)  ^ PRV_GET_BIT_MAC(D[2],1)  ^
                PRV_GET_BIT_MAC(D[1],30) ^ PRV_GET_BIT_MAC(D[1],29) ^ PRV_GET_BIT_MAC(D[1],28) ^
                PRV_GET_BIT_MAC(D[1],27) ^ PRV_GET_BIT_MAC(D[1],25) ^ PRV_GET_BIT_MAC(D[1],22) ^
                PRV_GET_BIT_MAC(D[1],21) ^ PRV_GET_BIT_MAC(D[1],20) ^ PRV_GET_BIT_MAC(D[1],16) ^
                PRV_GET_BIT_MAC(D[1],15) ^ PRV_GET_BIT_MAC(D[1],13) ^ PRV_GET_BIT_MAC(D[1],12) ^
                PRV_GET_BIT_MAC(D[1],11) ^ PRV_GET_BIT_MAC(D[1],10) ^ PRV_GET_BIT_MAC(D[1],5)  ^
                PRV_GET_BIT_MAC(D[1],4)  ^ PRV_GET_BIT_MAC(D[1],1)  ^ PRV_GET_BIT_MAC(D[0],30) ^
                PRV_GET_BIT_MAC(D[0],28) ^ PRV_GET_BIT_MAC(D[0],25) ^ PRV_GET_BIT_MAC(D[0],24) ^
                PRV_GET_BIT_MAC(D[0],23) ^ PRV_GET_BIT_MAC(D[0],21) ^ PRV_GET_BIT_MAC(D[0],20) ^
                PRV_GET_BIT_MAC(D[0],19) ^ PRV_GET_BIT_MAC(D[0],15) ^ PRV_GET_BIT_MAC(D[0],13) ^
                PRV_GET_BIT_MAC(D[0],12) ^ PRV_GET_BIT_MAC(D[0],9)  ^ PRV_GET_BIT_MAC(D[0],7)  ^
                PRV_GET_BIT_MAC(D[0],6)  ^ PRV_GET_BIT_MAC(D[0],4)  ^ PRV_GET_BIT_MAC(D[0],3)  ^
                PRV_GET_BIT_MAC(D[0],0);

    hash_bit[4] =
                PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],9)  ^ PRV_GET_BIT_MAC(D[2],7)  ^
                PRV_GET_BIT_MAC(D[2],3)  ^ PRV_GET_BIT_MAC(D[2],2)  ^
                PRV_GET_BIT_MAC(D[1],31) ^
                PRV_GET_BIT_MAC(D[1],30) ^ PRV_GET_BIT_MAC(D[1],29) ^ PRV_GET_BIT_MAC(D[1],28) ^
                PRV_GET_BIT_MAC(D[1],26) ^ PRV_GET_BIT_MAC(D[1],23) ^ PRV_GET_BIT_MAC(D[1],22) ^
                PRV_GET_BIT_MAC(D[1],21) ^ PRV_GET_BIT_MAC(D[1],17) ^ PRV_GET_BIT_MAC(D[1],16) ^
                PRV_GET_BIT_MAC(D[1],14) ^ PRV_GET_BIT_MAC(D[1],13) ^ PRV_GET_BIT_MAC(D[1],12) ^
                PRV_GET_BIT_MAC(D[1],11) ^ PRV_GET_BIT_MAC(D[1],6)  ^ PRV_GET_BIT_MAC(D[1],5)  ^
                PRV_GET_BIT_MAC(D[1],2)  ^ PRV_GET_BIT_MAC(D[0],31) ^ PRV_GET_BIT_MAC(D[0],29) ^
                PRV_GET_BIT_MAC(D[0],26) ^ PRV_GET_BIT_MAC(D[0],25) ^ PRV_GET_BIT_MAC(D[0],24) ^
                PRV_GET_BIT_MAC(D[0],22) ^ PRV_GET_BIT_MAC(D[0],21) ^ PRV_GET_BIT_MAC(D[0],20) ^
                PRV_GET_BIT_MAC(D[0],16) ^ PRV_GET_BIT_MAC(D[0],14) ^ PRV_GET_BIT_MAC(D[0],13) ^
                PRV_GET_BIT_MAC(D[0],10) ^ PRV_GET_BIT_MAC(D[0],8)  ^ PRV_GET_BIT_MAC(D[0],7)  ^
                PRV_GET_BIT_MAC(D[0],5)  ^ PRV_GET_BIT_MAC(D[0],4)  ^ PRV_GET_BIT_MAC(D[0],1);

    hash_bit[5] =
                PRV_GET_BIT_MAC(D[2],10)  ^ PRV_GET_BIT_MAC(D[2],8)  ^ PRV_GET_BIT_MAC(D[2],4)  ^
                PRV_GET_BIT_MAC(D[2],3)   ^ PRV_GET_BIT_MAC(D[2],0)  ^
                PRV_GET_BIT_MAC(D[1],31) ^
                PRV_GET_BIT_MAC(D[1],30) ^ PRV_GET_BIT_MAC(D[1],29) ^ PRV_GET_BIT_MAC(D[1],27) ^
                PRV_GET_BIT_MAC(D[1],24)  ^ PRV_GET_BIT_MAC(D[1],23) ^ PRV_GET_BIT_MAC(D[1],22) ^
                PRV_GET_BIT_MAC(D[1],18)  ^ PRV_GET_BIT_MAC(D[1],17) ^ PRV_GET_BIT_MAC(D[1],15) ^
                PRV_GET_BIT_MAC(D[1],14)  ^ PRV_GET_BIT_MAC(D[1],13) ^ PRV_GET_BIT_MAC(D[1],12) ^
                PRV_GET_BIT_MAC(D[1],7)   ^ PRV_GET_BIT_MAC(D[1],6)  ^ PRV_GET_BIT_MAC(D[1],3)^
                PRV_GET_BIT_MAC(D[1],0)   ^ PRV_GET_BIT_MAC(D[0],30) ^ PRV_GET_BIT_MAC(D[0],27) ^
                PRV_GET_BIT_MAC(D[0],26)  ^ PRV_GET_BIT_MAC(D[0],25) ^ PRV_GET_BIT_MAC(D[0],23) ^
                PRV_GET_BIT_MAC(D[0],22)  ^ PRV_GET_BIT_MAC(D[0],21) ^ PRV_GET_BIT_MAC(D[0],17) ^
                PRV_GET_BIT_MAC(D[0],15)  ^ PRV_GET_BIT_MAC(D[0],14) ^ PRV_GET_BIT_MAC(D[0],11) ^
                PRV_GET_BIT_MAC(D[0],9)   ^ PRV_GET_BIT_MAC(D[0],8)  ^ PRV_GET_BIT_MAC(D[0],6) ^
                PRV_GET_BIT_MAC(D[0],5)   ^ PRV_GET_BIT_MAC(D[0],2);

    hash_bit[6] =
                PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],9)  ^ PRV_GET_BIT_MAC(D[2],5)  ^
                PRV_GET_BIT_MAC(D[2],4)  ^ PRV_GET_BIT_MAC(D[2],1)  ^ PRV_GET_BIT_MAC(D[2],0)  ^
                PRV_GET_BIT_MAC(D[1],31) ^ PRV_GET_BIT_MAC(D[1],30) ^ PRV_GET_BIT_MAC(D[1],28) ^
                PRV_GET_BIT_MAC(D[1],25) ^ PRV_GET_BIT_MAC(D[1],24) ^ PRV_GET_BIT_MAC(D[1],23) ^
                PRV_GET_BIT_MAC(D[1],19) ^ PRV_GET_BIT_MAC(D[1],18) ^ PRV_GET_BIT_MAC(D[1],16) ^
                PRV_GET_BIT_MAC(D[1],15) ^ PRV_GET_BIT_MAC(D[1],14) ^ PRV_GET_BIT_MAC(D[1],13) ^
                PRV_GET_BIT_MAC(D[1],8)  ^ PRV_GET_BIT_MAC(D[1],7)  ^ PRV_GET_BIT_MAC(D[1],4)  ^
                PRV_GET_BIT_MAC(D[1],1)  ^ PRV_GET_BIT_MAC(D[0],31) ^ PRV_GET_BIT_MAC(D[0],28) ^
                PRV_GET_BIT_MAC(D[0],27) ^ PRV_GET_BIT_MAC(D[0],26) ^ PRV_GET_BIT_MAC(D[0],24) ^
                PRV_GET_BIT_MAC(D[0],23) ^ PRV_GET_BIT_MAC(D[0],22) ^ PRV_GET_BIT_MAC(D[0],18) ^
                PRV_GET_BIT_MAC(D[0],16) ^ PRV_GET_BIT_MAC(D[0],15) ^ PRV_GET_BIT_MAC(D[0],12) ^
                PRV_GET_BIT_MAC(D[0],10) ^ PRV_GET_BIT_MAC(D[0],9)  ^ PRV_GET_BIT_MAC(D[0],7)  ^
                PRV_GET_BIT_MAC(D[0],6)  ^ PRV_GET_BIT_MAC(D[0],3);

    hash_bit[7] =
                PRV_GET_BIT_MAC(D[2],10) ^ PRV_GET_BIT_MAC(D[2],6)  ^ PRV_GET_BIT_MAC(D[2],5)  ^
                PRV_GET_BIT_MAC(D[2],2)  ^ PRV_GET_BIT_MAC(D[2],1)  ^ PRV_GET_BIT_MAC(D[2],0)  ^
                PRV_GET_BIT_MAC(D[1],31) ^ PRV_GET_BIT_MAC(D[1],29) ^ PRV_GET_BIT_MAC(D[1],26) ^
                PRV_GET_BIT_MAC(D[1],25) ^ PRV_GET_BIT_MAC(D[1],24) ^ PRV_GET_BIT_MAC(D[1],20) ^
                PRV_GET_BIT_MAC(D[1],19) ^ PRV_GET_BIT_MAC(D[1],17) ^ PRV_GET_BIT_MAC(D[1],16) ^
                PRV_GET_BIT_MAC(D[1],15) ^ PRV_GET_BIT_MAC(D[1],14) ^ PRV_GET_BIT_MAC(D[1],9)  ^
                PRV_GET_BIT_MAC(D[1],8)  ^ PRV_GET_BIT_MAC(D[1],5)  ^ PRV_GET_BIT_MAC(D[1],2)  ^
                PRV_GET_BIT_MAC(D[1],0)  ^ PRV_GET_BIT_MAC(D[0],29) ^ PRV_GET_BIT_MAC(D[0],28) ^
                PRV_GET_BIT_MAC(D[0],27) ^ PRV_GET_BIT_MAC(D[0],25) ^ PRV_GET_BIT_MAC(D[0],24) ^
                PRV_GET_BIT_MAC(D[0],23) ^ PRV_GET_BIT_MAC(D[0],19) ^ PRV_GET_BIT_MAC(D[0],17) ^
                PRV_GET_BIT_MAC(D[0],16) ^ PRV_GET_BIT_MAC(D[0],13) ^ PRV_GET_BIT_MAC(D[0],11) ^
                PRV_GET_BIT_MAC(D[0],10) ^ PRV_GET_BIT_MAC(D[0],8)  ^ PRV_GET_BIT_MAC(D[0],7)  ^
                PRV_GET_BIT_MAC(D[0],4);

    hash_bit[8] =
                PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],7)  ^ PRV_GET_BIT_MAC(D[2],6)  ^
                PRV_GET_BIT_MAC(D[2],3)  ^ PRV_GET_BIT_MAC(D[2],2)  ^ PRV_GET_BIT_MAC(D[2],1)  ^
                PRV_GET_BIT_MAC(D[2],0)  ^
                PRV_GET_BIT_MAC(D[1],30) ^ PRV_GET_BIT_MAC(D[1],27) ^
                PRV_GET_BIT_MAC(D[1],26) ^ PRV_GET_BIT_MAC(D[1],25) ^ PRV_GET_BIT_MAC(D[1],21) ^
                PRV_GET_BIT_MAC(D[1],20) ^ PRV_GET_BIT_MAC(D[1],18) ^ PRV_GET_BIT_MAC(D[1],17) ^
                PRV_GET_BIT_MAC(D[1],16) ^ PRV_GET_BIT_MAC(D[1],15) ^ PRV_GET_BIT_MAC(D[1],10) ^
                PRV_GET_BIT_MAC(D[1],9)  ^ PRV_GET_BIT_MAC(D[1],6)  ^ PRV_GET_BIT_MAC(D[1],3)  ^
                PRV_GET_BIT_MAC(D[1],1)  ^ PRV_GET_BIT_MAC(D[0],30) ^ PRV_GET_BIT_MAC(D[0],29) ^
                PRV_GET_BIT_MAC(D[0],28) ^ PRV_GET_BIT_MAC(D[0],26) ^ PRV_GET_BIT_MAC(D[0],25) ^
                PRV_GET_BIT_MAC(D[0],24) ^ PRV_GET_BIT_MAC(D[0],20) ^ PRV_GET_BIT_MAC(D[0],18) ^
                PRV_GET_BIT_MAC(D[0],17) ^ PRV_GET_BIT_MAC(D[0],14) ^ PRV_GET_BIT_MAC(D[0],12) ^
                PRV_GET_BIT_MAC(D[0],11) ^ PRV_GET_BIT_MAC(D[0],9)  ^ PRV_GET_BIT_MAC(D[0],8)  ^
                PRV_GET_BIT_MAC(D[0],5);

    hash_bit[9] =
                PRV_GET_BIT_MAC(D[2],8)  ^ PRV_GET_BIT_MAC(D[2],7)  ^ PRV_GET_BIT_MAC(D[2],4) ^
                PRV_GET_BIT_MAC(D[2],3)  ^ PRV_GET_BIT_MAC(D[2],2)  ^ PRV_GET_BIT_MAC(D[2],1) ^
                PRV_GET_BIT_MAC(D[1],31)^ PRV_GET_BIT_MAC(D[1],28) ^ PRV_GET_BIT_MAC(D[1],27) ^
                PRV_GET_BIT_MAC(D[1],26)^ PRV_GET_BIT_MAC(D[1],22) ^ PRV_GET_BIT_MAC(D[1],21) ^
                PRV_GET_BIT_MAC(D[1],19)^ PRV_GET_BIT_MAC(D[1],18) ^ PRV_GET_BIT_MAC(D[1],17) ^
                PRV_GET_BIT_MAC(D[1],16)^ PRV_GET_BIT_MAC(D[1],11) ^ PRV_GET_BIT_MAC(D[1],10) ^
                PRV_GET_BIT_MAC(D[1],7) ^ PRV_GET_BIT_MAC(D[1],4)  ^ PRV_GET_BIT_MAC(D[1],2)  ^
                PRV_GET_BIT_MAC(D[0],31)^ PRV_GET_BIT_MAC(D[0],30) ^ PRV_GET_BIT_MAC(D[0],29) ^
                PRV_GET_BIT_MAC(D[0],27)^ PRV_GET_BIT_MAC(D[0],26) ^ PRV_GET_BIT_MAC(D[0],25) ^
                PRV_GET_BIT_MAC(D[0],21)^ PRV_GET_BIT_MAC(D[0],19) ^ PRV_GET_BIT_MAC(D[0],18) ^
                PRV_GET_BIT_MAC(D[0],15)^ PRV_GET_BIT_MAC(D[0],13) ^ PRV_GET_BIT_MAC(D[0],12) ^
                PRV_GET_BIT_MAC(D[0],10)^ PRV_GET_BIT_MAC(D[0],9)  ^ PRV_GET_BIT_MAC(D[0],6);

    hash_bit[10] =
                PRV_GET_BIT_MAC(D[2],9)  ^ PRV_GET_BIT_MAC(D[2],8)  ^ PRV_GET_BIT_MAC(D[2],5)  ^
                PRV_GET_BIT_MAC(D[2],4)  ^ PRV_GET_BIT_MAC(D[2],3)  ^ PRV_GET_BIT_MAC(D[2],2)  ^
                PRV_GET_BIT_MAC(D[2],0)  ^
                PRV_GET_BIT_MAC(D[1],29) ^ PRV_GET_BIT_MAC(D[1],28) ^
                PRV_GET_BIT_MAC(D[1],27) ^ PRV_GET_BIT_MAC(D[1],23) ^ PRV_GET_BIT_MAC(D[1],22) ^
                PRV_GET_BIT_MAC(D[1],20) ^ PRV_GET_BIT_MAC(D[1],19) ^ PRV_GET_BIT_MAC(D[1],18) ^
                PRV_GET_BIT_MAC(D[1],17) ^ PRV_GET_BIT_MAC(D[1],12) ^ PRV_GET_BIT_MAC(D[1],11) ^
                PRV_GET_BIT_MAC(D[1],8)  ^ PRV_GET_BIT_MAC(D[1],5)  ^ PRV_GET_BIT_MAC(D[1],3)  ^
                PRV_GET_BIT_MAC(D[1],0)  ^ PRV_GET_BIT_MAC(D[0],31) ^ PRV_GET_BIT_MAC(D[0],30) ^
                PRV_GET_BIT_MAC(D[0],28) ^ PRV_GET_BIT_MAC(D[0],27) ^ PRV_GET_BIT_MAC(D[0],26) ^
                PRV_GET_BIT_MAC(D[0],22) ^ PRV_GET_BIT_MAC(D[0],20) ^ PRV_GET_BIT_MAC(D[0],19) ^
                PRV_GET_BIT_MAC(D[0],16) ^ PRV_GET_BIT_MAC(D[0],14) ^ PRV_GET_BIT_MAC(D[0],13) ^
                PRV_GET_BIT_MAC(D[0],11) ^ PRV_GET_BIT_MAC(D[0],10) ^ PRV_GET_BIT_MAC(D[0],7);

    hash_bit[11] =
                PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],7)  ^ PRV_GET_BIT_MAC(D[2],6)  ^
                PRV_GET_BIT_MAC(D[2],5)  ^ PRV_GET_BIT_MAC(D[2],3)  ^ PRV_GET_BIT_MAC(D[2],2)  ^
                PRV_GET_BIT_MAC(D[2],1)  ^ PRV_GET_BIT_MAC(D[2],0)  ^
                PRV_GET_BIT_MAC(D[1],31) ^
                PRV_GET_BIT_MAC(D[1],26) ^ PRV_GET_BIT_MAC(D[1],25) ^ PRV_GET_BIT_MAC(D[1],21) ^
                PRV_GET_BIT_MAC(D[1],19) ^ PRV_GET_BIT_MAC(D[1],18) ^ PRV_GET_BIT_MAC(D[1],12) ^
                PRV_GET_BIT_MAC(D[1],11) ^ PRV_GET_BIT_MAC(D[1],10) ^ PRV_GET_BIT_MAC(D[1],7)  ^
                PRV_GET_BIT_MAC(D[1],6)  ^ PRV_GET_BIT_MAC(D[1],5)  ^ PRV_GET_BIT_MAC(D[1],4)  ^
                PRV_GET_BIT_MAC(D[1],3)  ^ PRV_GET_BIT_MAC(D[1],2)  ^ PRV_GET_BIT_MAC(D[0],29) ^
                PRV_GET_BIT_MAC(D[0],27) ^ PRV_GET_BIT_MAC(D[0],26) ^ PRV_GET_BIT_MAC(D[0],25) ^
                PRV_GET_BIT_MAC(D[0],23) ^ PRV_GET_BIT_MAC(D[0],20) ^ PRV_GET_BIT_MAC(D[0],19) ^
                PRV_GET_BIT_MAC(D[0],18) ^ PRV_GET_BIT_MAC(D[0],15) ^ PRV_GET_BIT_MAC(D[0],13) ^
                PRV_GET_BIT_MAC(D[0],9)  ^ PRV_GET_BIT_MAC(D[0],8)  ^ PRV_GET_BIT_MAC(D[0],7)  ^
                PRV_GET_BIT_MAC(D[0],6)  ^ PRV_GET_BIT_MAC(D[0],4)  ^ PRV_GET_BIT_MAC(D[0],3)  ^
                PRV_GET_BIT_MAC(D[0],1)  ^ PRV_GET_BIT_MAC(D[0],0);

    hash_bit[12] =
                PRV_GET_BIT_MAC(D[2],11) ^ PRV_GET_BIT_MAC(D[2],10) ^ PRV_GET_BIT_MAC(D[2],9)  ^
                PRV_GET_BIT_MAC(D[2],8)  ^ PRV_GET_BIT_MAC(D[2],6)  ^ PRV_GET_BIT_MAC(D[2],3)  ^
                PRV_GET_BIT_MAC(D[2],1)  ^
                PRV_GET_BIT_MAC(D[1],31) ^ PRV_GET_BIT_MAC(D[1],30) ^
                PRV_GET_BIT_MAC(D[1],29) ^ PRV_GET_BIT_MAC(D[1],28) ^ PRV_GET_BIT_MAC(D[1],27) ^
                PRV_GET_BIT_MAC(D[1],25) ^ PRV_GET_BIT_MAC(D[1],24) ^ PRV_GET_BIT_MAC(D[1],23) ^
                PRV_GET_BIT_MAC(D[1],22) ^ PRV_GET_BIT_MAC(D[1],19) ^ PRV_GET_BIT_MAC(D[1],12) ^
                PRV_GET_BIT_MAC(D[1],10) ^ PRV_GET_BIT_MAC(D[1],9)  ^ PRV_GET_BIT_MAC(D[1],8)  ^
                PRV_GET_BIT_MAC(D[1],6)  ^ PRV_GET_BIT_MAC(D[1],4)  ^ PRV_GET_BIT_MAC(D[1],2)  ^
                PRV_GET_BIT_MAC(D[1],1)  ^ PRV_GET_BIT_MAC(D[1],0)  ^ PRV_GET_BIT_MAC(D[0],31) ^
                PRV_GET_BIT_MAC(D[0],30) ^ PRV_GET_BIT_MAC(D[0],27) ^ PRV_GET_BIT_MAC(D[0],25) ^
                PRV_GET_BIT_MAC(D[0],24) ^ PRV_GET_BIT_MAC(D[0],20) ^ PRV_GET_BIT_MAC(D[0],18) ^
                PRV_GET_BIT_MAC(D[0],17) ^ PRV_GET_BIT_MAC(D[0],16) ^ PRV_GET_BIT_MAC(D[0],13) ^
                PRV_GET_BIT_MAC(D[0],12) ^ PRV_GET_BIT_MAC(D[0],11) ^ PRV_GET_BIT_MAC(D[0],10) ^
                PRV_GET_BIT_MAC(D[0],8)  ^ PRV_GET_BIT_MAC(D[0],6)  ^ PRV_GET_BIT_MAC(D[0],5)  ^
                PRV_GET_BIT_MAC(D[0],3)  ^ PRV_GET_BIT_MAC(D[0],2)  ^ PRV_GET_BIT_MAC(D[0],0);

    /* take care of init values */
    if (!useZeroInitValueForCrcHash)
    {
        hash_bit[6]  ^= 1;
        hash_bit[7]  ^= 1;
        hash_bit[8]  ^= 1;
        hash_bit[9]  ^= 1;
        hash_bit[10] ^= 1;
        hash_bit[11] ^= 1;
    }
};

/**
* @internal prvCpssDxChBrgFdbSip5CrcCalc function
* @endinternal
*
* @brief   This function calculates the CRC hash according to hash function type.
*         A length of data can be 44, 80, 96, 140 only
* @param[in] funcType                 - hash function type:
*                                      one of : 16a,16b,16c,16d,32a,32b,32k,32q,64
* @param[in] numOfBits                - amount of bits in words data array
*                                      not more than 140 bits
* @param[in] initCrcArr[2]            - the CRC polynomial pattern (crc init value)
* @param[in] dataArr[5]               - array of 32bit words, contain data.
*                                      for 140 bits data needed 5 words.
*                                      for 80 bits data needed 3 words.
*                                      for 36 bits data needed 2 words.
*
* @param[out] hashBitArr[2]            - the hash result. (16 or 32 or 64 bits)
*
* @retval GT_BAD_VALUE             - unexpected value of inner variable. An error in algorithm
* @retval GT_BAD_PARAM             - bad input parameter
* @retval GT_OK                    - ok
*/
GT_STATUS prvCpssDxChBrgFdbSip5CrcCalc
(
    IN CRC_FUNC_ENT funcType,
    IN GT_U32  numOfBits,
    IN GT_U32  initCrcArr[2],
    IN GT_U32  dataArr[5],
    OUT GT_U32 hashBitArr[2]
)
{
    GT_U32  tmpInitCrc[2];/* temp initCrc as input to the interim hash calculations */
    GT_U32  tmpHash[2] = {0, 0};  /* temp hash result between interim hash calculations */
    GT_U32  tmpData[2] = {0, 0};  /* temp data for the specific interim hash calculation */
    GT_U32  startBit;            /* start of data chunk used for CRC calculation */
    GT_U32  len;                 /* length of a data chunk */
    GT_BOOL need60   = GT_FALSE; /* if needed to calculate CRC for 60 bit data */
    GT_BOOL need44   = GT_FALSE; /* if needed to calculate CRC for 44 bit data */
    GT_BOOL need36   = GT_FALSE; /* if needed to calculate CRC for 36 bit data */
    CRC_TYPE_FUNC func;          /* pointer to a CRC function */

    /* the logic is to break the hash calc to cases:
       need 140 bits: calc as 60 + 44 + 36
       need 96  bits: calc as 60 + 36
       need 80  bits: calc as 44 + 36
       need 44  bits: cals as 44
    */

    switch (numOfBits)
    {
        case 140:
            need60 = GT_TRUE;
            need44 = GT_TRUE;
            need36 = GT_TRUE;
            break;
        case 96:
            need60 = GT_TRUE;
            need36 = GT_TRUE;
            break;
        case 80:
            need44 = GT_TRUE;
            need36 = GT_TRUE;
            break;
        case 44:
            need44 = GT_TRUE;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(numOfBits);
    }

    tmpInitCrc[0] = initCrcArr[0];
    tmpInitCrc[1] = initCrcArr[1];
    startBit = numOfBits;       /* set at the end of data */

    do
    {
        if (GT_TRUE == need60)
        {
            need60 = GT_FALSE;
            len = 60;
            func = crcFuncArray[funcType].calc60BitsData;
        }
        else if (GT_TRUE == need44)
        {
            need44 = GT_FALSE;
            len = 44;
            func = crcFuncArray[funcType].calc44BitsData;
        }
        else if (GT_TRUE == need36)
        {
            need36 = GT_FALSE;
            len = 36;
            func = crcFuncArray[funcType].calc36BitsData;
        }
        else
        {
            /* should not happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        startBit -= len;

        /* input data cannot exceed 140 bits */
        if ((startBit+len) > 140)
        {
            /* should not happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        U32_GET_FIELD_IN_ENTRY_MAC(dataArr, startBit,    32,     tmpData[0]);
        U32_GET_FIELD_IN_ENTRY_MAC(dataArr, startBit+32,(len-32),tmpData[1]);
        func(tmpData,  tmpInitCrc, tmpHash);
        tmpInitCrc[0] = tmpHash[0];
        tmpInitCrc[1] = tmpHash[1];

    } while (startBit != 0);

    hashBitArr[0] = tmpHash[0];
    hashBitArr[1] = tmpHash[1];

    return GT_OK;
}

/**
* @internal hashCalcDataCrc function
* @endinternal
*
* @brief   This function calculates the hash index for the
*         mac address table using CRC algorithm.
*         Not used if multi-hash mode (SIP5 devices) enabled.
* @param[in] dataPtr                  - array of 32bit words, contain data.
* @param[in] dataLen                  - length of data (in bits)
* @param[in] bitsWidth                - amount of used bits in a hash.
* @param[in] useZeroInitValueForCrcHash - Use zero for initialization value of CRC function.
*
* @param[out] hashPtr                  - the hash index.
*
* @retval GT_OK                    - success.
* @retval GT_BAD_VALUE             - unexpected value of inner variable.
* @retval GT_BAD_PARAM             - bad parameter
*/
static GT_STATUS hashCalcDataCrc
(
    IN  GT_U32          *dataPtr,
    IN  GT_U32          dataLen,
    IN  GT_U32          bitsWidth,
    OUT GT_U32          *hashPtr,
    IN  GT_U32          useZeroInitValueForCrcHash
)
{
    GT_U32      hash_bit[32] = {0};
    GT_U32      i; /* iterator */
    GT_BOOL     alreadyGotHashAsBits = GT_FALSE;
    GT_U32      initArr[2] = {0, 0};
    GT_STATUS   rc;

    if (bitsWidth < 13) /*8K,16K*/
    {/* number of hash bits is 12 or 11 */
        crc_11_12(dataPtr, bitsWidth, hash_bit, useZeroInitValueForCrcHash);
    }
    else if (bitsWidth == 13)/*32K*/
    {
        crc_13(dataPtr, hash_bit, useZeroInitValueForCrcHash);
    }
    else/* >= 64K */
    {
        if (96 == dataLen)
        {
            rc = prvCpssDxChBrgFdbSip5CrcCalc(CRC_FUNC_16A_E, dataLen,  initArr, dataPtr,
                             hash_bit);
            if (rc != 0)
            {
                return rc;
            }
        }
        else
        {
            if(useZeroInitValueForCrcHash == GT_TRUE)
            {
                /* CRC32a with 76bits data , without any 'init CRC value' */
                prvCpssDxChBrgFdbHashCrc_32a_d76_noInitValueByNumBits(
                    bitsWidth,dataPtr, hash_bit);
            }
            else
            {
                GT_U32  crcInitValue[1] = {0xFFFFFFFF};
                /* CRC32a with 76bits data , with crc init values */
                prvCpssDxChBrgFdbHashCrc_32a_d76(
                    bitsWidth,dataPtr,crcInitValue, hash_bit);
            }
        }

        alreadyGotHashAsBits = GT_TRUE;
    }

    if(alreadyGotHashAsBits == GT_FALSE)
    {
        (*hashPtr) = 0;
        for(i = 0 ; i < bitsWidth; i++)
        {
            (*hashPtr) |= (hash_bit[i] & 0x01) << i;
        }
    }
    else
    {
        /* already got the hash as bits bmp (not as bit per word) */
        (*hashPtr) = hash_bit[0] & BIT_MASK_MAC(bitsWidth);
    }

    return GT_OK;
}

/**
* @internal sip5CrcMultiHashCalc function
* @endinternal
*
* @brief   This function calculates the 16 CRC hashes for the 'multi hash'
*
* @param[in] dataPtr                  - array of 32bit words, contain data.
* @param[in] numBitsInData            - amount of bits in words data array
*                                      all 32 used in all words
*                                      the bits used in the last are from MSB side
* @param[in] numBitsInHashResult      - number of bits to generate into the hash results
* @param[in] initCrc                  - the CRC polynomial pattern (crc init value)
* @param[in] startBankIndex           - the first bank for 'multi hash' result.
* @param[in] numOfHashToCalc          - the number of hash functions to generate
*                                      (also it is the number of elements in calculatedHashArr[])
* @param[in] totalNumOfHashes         - total number of hashes for current FDB size
* @param[in] returnRawHashValue       - indication to keep the calculated hash without
*                                       adding the bankId into it as 'first nibble'
*                                       the DDE key hashing differ from all other entry types.
*                                       GT_TRUE  - for DDE key
*                                       GT_FALSE - for all other key types
*
* @param[out] calculatedHashArr[NUM_BANKS_MULTI_HASH_CNS] - the hash array (16 hashes)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter.
*/
static GT_STATUS sip5CrcMultiHashCalc
(
    IN  GT_U32   *dataPtr,
    IN  GT_U32   numBitsInData,
    IN  GT_U32   numBitsInHashResult,
    IN  GT_U32   initCrc,
    IN  GT_U32   startBankIndex,
    IN  GT_U32   numOfHashToCalc,
    IN  GT_U32   totalNumOfHashes,
    IN  GT_BOOL  returnRawHashValue,
    OUT GT_U32   calculatedHashArr[NUM_BANKS_MULTI_HASH_CNS]
)
{
    GT_STATUS   rc;
    GT_U32  hashBitArr[2]={0,0};/* 2 words to support CRC64 */
    GT_U32  initCrcArr[2];/* 2 words to support CRC64 */
    GT_U32  bank; /* iterator for FDB bank - for the needed hash function to used */
    GT_U32  startBit;/* the start bit from the result of the current temp hash
                        to be taken as the calculatedHashArr[bank] */
    CRC_FUNC_ENT    funcType;/* the needed hash function to used */
    GT_U32  index = 0;/* index in calculatedHashArr[] */
    GT_U32  numOfIterationToSkip = 0;/*number of iterations to skip in loop of hash functions */

    if(((startBankIndex + numOfHashToCalc) > NUM_BANKS_MULTI_HASH_CNS) ||
       (startBankIndex == NUM_BANKS_MULTI_HASH_CNS)||(numOfHashToCalc == 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* prepare initCrc array */
    initCrcArr[0] = initCrc;
    initCrcArr[1] = initCrc;

    bank = startBankIndex;
    /* look for the first bank that must be calculated to allow start from the
       needed 'startBankIndex' */
    while(crcFuncBitsUsedArr[bank].funcType == CRC_FUNC___DUMMY__E)
    {
        if(bank == 0)
        {
            /* should not happen -- DB error in crcFuncBitsUsedArr[]  */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        bank--;
        numOfIterationToSkip++;
    }

    for(/*already initialized*/; bank < totalNumOfHashes; bank++)
    {
        /* get values from the DB */
        funcType = crcFuncBitsUsedArr[bank].funcType;
        startBit = crcFuncBitsUsedArr[bank].startBit;

        if(startBit & BIT_AS_END_BIT_CNS)
        {
            /* this is special indication that the bit is the 'end bit+1' */
            /* remove the bit , and then decrement by numBitsInHashResult*/
            startBit = ((startBit & (~BIT_AS_END_BIT_CNS)) - numBitsInHashResult);
        }

        if(funcType != CRC_FUNC___DUMMY__E)/* the dummy are skipped */
        {
            rc = prvCpssDxChBrgFdbSip5CrcCalc(funcType,numBitsInData,initCrcArr,dataPtr,hashBitArr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if(numOfIterationToSkip)
        {
            numOfIterationToSkip--;
            continue;
        }

        /* get the needed amount of bits */
        U32_GET_FIELD_IN_ENTRY_MAC(hashBitArr,startBit,numBitsInHashResult,
            calculatedHashArr[index]);

        if(returnRawHashValue == GT_FALSE)
        {
            calculatedHashArr[index] *= totalNumOfHashes;
            calculatedHashArr[index] += bank;
        }
        else
        {
            /* do not modify the calculatedHashArr[index] , we need it in raw value */
            /* this is for the DDE key hashing */
        }

        index++;
    }

    return GT_OK;
}


/**
* @internal hashCalcSwapBits function
* @endinternal
*
* @brief   This function swaps bits in 32 bits word
*
* @param[in] data                     - 32bit value to be bit-swapped.
* @param[in] bitsWidth                - amount of used bits in the value .
*                                       swapped value
*/
static GT_U32 hashCalcSwapBits
(
    IN  GT_U32          data,
    IN  GT_U32          bitsWidth
)
{
    GT_U32 idx;
    GT_U32 result ;

    data &= BIT_MASK_MAC(bitsWidth);

    if (data == 0)
    {
        return 0 ;
    }

    result = 0 ;
    for (idx = 0; idx < bitsWidth; idx++)
    {
        result |= (((data >> idx) & 0x01) << (bitsWidth - idx - 1));
    }

    return result ;
}

/**
* @internal hashCalcFillMacVidDataXor function
* @endinternal
*
* @brief   This function prepares MAC+VID data
*         for calculating numBits hash using XOR algorithm
* @param[in] numBits                  - number of bits used to address FDB mac entry in FDB table,
*                                      depends on FDB size
* @param[in] fid16BitHashEn           - indication if FID used 12 or 16 bits
* @param[in] macAddrPtr               - MAC address to put to array (in network order) .
* @param[in] vlanId                   - Vlan id  to put to array, for shared entry specify "0"
*
* @param[out] dataPtr                  - 7-words array to receive data.
*
* @retval GT_OK                    - success.
*/
static GT_STATUS hashCalcFillMacVidDataXor
(
    IN  GT_U32          numBits,
    IN  GT_BOOL         fid16BitHashEn,
    IN  GT_U8           *macAddrPtr,
    IN  GT_U32          vlanId,
    OUT GT_U32          *dataPtr
)
{
    GT_U32  tmpWords[2];
    GT_U32  startBit = 0;
    GT_U32  maxWordSwapped = 3;
    GT_U32  ii;
    GT_U32  tempValue;

    tmpWords[0] = GT_MAC_LOW32_MAC(macAddrPtr);
    tmpWords[1] = GT_MAC_HIGH16_MAC(macAddrPtr);

    U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,numBits,tempValue);
    dataPtr[0] = tempValue;
    startBit += numBits;
    U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,numBits,tempValue);
    dataPtr[1] = tempValue;
    startBit += numBits;
    U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,numBits,tempValue);
    dataPtr[2] = tempValue;
    startBit += numBits;

    dataPtr[4] = 0 ;

    if(startBit >= 48) /* we already got 48 bits of MAC */
    {
        dataPtr[3] = 0;
        maxWordSwapped --;
    }
    else
    {
        U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,numBits,tempValue);
        dataPtr[3] = tempValue;
        startBit += numBits;

        if(startBit < 48)
        {
            maxWordSwapped++;
            /* we are with the last bits */
            U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,(48 - startBit),tempValue);
            dataPtr[4] = tempValue;
        }
    }

    if(fid16BitHashEn == 0)
    {
        dataPtr[5] = vlanId & BIT_MASK_MAC(numBits);
        dataPtr[6] = 0 ;
        if(numBits > 12)
        {
            dataPtr[5] |= ((vlanId & BIT_MASK_MAC(numBits-12)) << 12);/*additional bits*/
        }
        else if(numBits < 12)
        {
            dataPtr[6] = vlanId >> numBits ;
        }

    }
    else
    {
        tmpWords[0] = vlanId;
        startBit = 0;
        U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,numBits,tempValue);
        dataPtr[5] = tempValue;
        startBit += numBits;
        U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,numBits,tempValue);
        dataPtr[6] = tempValue;
    }

    /* swap bits in MAC address (but no in vlanId) */
    /* swap bits in words 0..3 , -- not in 4 - not in 5,6 (bits of the FID)*/
    for(ii = 0 ; ii <= maxWordSwapped ; ii ++)
    {
        dataPtr[ii] = hashCalcSwapBits(dataPtr[ii], numBits);
    }

    return GT_OK;
}

/**
* @internal hashCalcFillDipSipVidDataXor function
* @endinternal
*
* @brief   This function prepares DIP+SIP+VID data
*         for calculating numBits hash using XOR algorithm
* @param[in] numBits                  - number of bits used to address FDB mac entry in FDB table,
*                                      depends on FDB size
* @param[in] fid16BitHashEn           - indication if FID used 12 or 16 bits
* @param[in] destIpAddr               - destination IP to put to array .
* @param[in] sourceIpAddr             - source IP to put to array, if not used - specify "0"
* @param[in] vlanId                   - Vlan id  to put to array, for shared entry specify "0"
*
* @param[out] dataPtr                  - 7-words array to receive data.
*                                       none
*
* @note GT_OK - success.
*
*/
static GT_STATUS hashCalcFillDipSipVidDataXor
(
    IN  GT_U32          numBits,
    IN  GT_BOOL         fid16BitHashEn,
    IN  GT_U32          destIpAddr,
    IN  GT_U32          sourceIpAddr,
    IN  GT_U32          vlanId,
    OUT GT_U32          *dataPtr
)
{
    GT_U32  startBit = 0;
    GT_U32  tmpWords[2];
    GT_U32  ii;
    GT_U32  tempValue;

    /*start with DIP */
    tmpWords[0] = destIpAddr;
    tmpWords[1] = 0;

    U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,numBits,tempValue);
    dataPtr[0] = tempValue;
    startBit += numBits;
    U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,numBits,tempValue);
    dataPtr[1] = tempValue;
    startBit += numBits;
    if(startBit >= 32) /* we already got 32 bits of DIP */
    {
        dataPtr[2] = 0;
    }
    else
    {
        U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,numBits,tempValue);
        dataPtr[2] = tempValue;
    }

    /*continue with DIP */
    tmpWords[0] = sourceIpAddr;
    startBit = 0;

    U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,numBits,tempValue);
    dataPtr[3] = tempValue;
    startBit += numBits;
    U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,numBits,tempValue);
    dataPtr[4] = tempValue;
    startBit += numBits;
    if(startBit >= 32) /* we already got 32 bits of SIP */
    {
        dataPtr[5] = 0;
    }
    else
    {
        U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,numBits,tempValue);
        dataPtr[5] = tempValue;
    }

    if(fid16BitHashEn == 0)
    {
        dataPtr[6] = vlanId & BIT_MASK_MAC(numBits) ;
        dataPtr[7] = 0;
        if(numBits > 12)
        {
            dataPtr[6] |= ((vlanId & BIT_MASK_MAC(numBits-12)) << 12);/*additional bits*/
        }
        else if(numBits < 12)
        {
            dataPtr[7] = vlanId >> numBits ;
        }
    }
    else
    {
        tmpWords[0] = vlanId;
        startBit = 0;
        U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,numBits,tempValue);
        dataPtr[6] = tempValue;
        startBit += numBits;

        if(startBit >= 16) /* we already got 16 bits of FID */
        {
            dataPtr[7] = 0;
        }
        else
        {
            U32_GET_FIELD_IN_ENTRY_MAC(tmpWords,startBit,numBits,tempValue);
            dataPtr[7] = tempValue;
        }
    }

    /* swap bits in words 0..5  -- not in 6,7 (FID info)*/
    for(ii = 0 ; ii <= 5 ; ii ++)
    {
        dataPtr[ii] = hashCalcSwapBits(dataPtr[ii], numBits);
    }

    return GT_OK;
}


/**
* @internal hashCalcFillMacVidDataCrc function
* @endinternal
*
* @brief   This function prepares MAC+VID data
*         for calculating hash using CRC algorithm
*         Generate data of 76/80 bits.
* @param[in] fid16BitHashEn           - indication if FID used 12 or 16 bits
* @param[in] macAddrPtr               - MAC address to put to array (in network order) .
* @param[in] vlanId                   - Vlan id  to put to array, for shared entry specify "0"
* @param[in] crcHashUpperBitsMode     - 16 MSbits mode for of DATA into the hash function
*                                      relevant only to CRC for MAC+FID mode.
*
* @param[out] dataPtr                  - 3-words array to receive data.(for 76/80 bits)
*
* @retval GT_OK                    - success.
*/
static GT_STATUS hashCalcFillMacVidDataCrc
(
    IN  GT_BOOL         fid16BitHashEn,
    IN  GT_U8           *macAddrPtr,
    IN  GT_U32          vlanId,
    OUT GT_U32          *dataPtr,
    IN  CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT crcHashUpperBitsMode
)
{
    GT_U32  lowWordMacAddress = GT_MAC_LOW32_MAC(macAddrPtr);
                                  /* low 32 bits of mac address */

    GT_U32  hiHalfWordMacAddress = GT_MAC_HIGH16_MAC(macAddrPtr);
                                  /* hi 16 bits of mac address*/
    GT_U32  tmpBitIndex;
    GT_U32  fid;

    if(fid16BitHashEn)
    {
        fid = vlanId;
        tmpBitIndex = 16;
    }
    else
    {
        fid = vlanId & 0xFFF;
        tmpBitIndex = 12;
    }

    /* 16 zeros, 48bit MAC, 12bit VID */

    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,0           ,tmpBitIndex  ,fid);
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,tmpBitIndex,32            ,lowWordMacAddress);
    tmpBitIndex += 32;
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,tmpBitIndex,16            ,hiHalfWordMacAddress);
    tmpBitIndex += 16;

    switch(crcHashUpperBitsMode)
    {
        default:
        case CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E:
            U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,tmpBitIndex,16,0);/*64..79 or 60..75 */
            break;
        case CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_FID_E:
            U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,tmpBitIndex,16,fid);/*64..79 or 60..75 */
            break;
        case CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_MAC_E:
            U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,tmpBitIndex,16,lowWordMacAddress);/*64..79 or 60..75 */
            break;
    }

    if(fid16BitHashEn == 0)
    {
        U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,76,4,0);/*76..79*/
    }

    return GT_OK;
}


/**
* @internal hashCalcFillMacFidVid1DataCrc function
* @endinternal
*
* @brief   This function prepares MAC, FID, VID1 data for calculating hash
*         using CRC algorithm.
*         Generates data 80 bits.
* @param[in] macAddrPtr               - MAC address (in network order) .
* @param[in] fid                      - FID. For shared entry specify "0"
* @param[in] vid1                     - VID1
*
* @param[out] dataPtr                  - 3-words array to receive data.
*
* @retval GT_OK                    - success.
*/
static GT_STATUS hashCalcFillMacFidVid1DataCrc
(
    IN  GT_U8     *macAddrPtr,
    IN  GT_U16    fid,
    IN  GT_U32    vid1,
    OUT GT_U32    *dataPtr
)
{
    GT_U32  lowWordMacAddress = GT_MAC_LOW32_MAC(macAddrPtr);
                                  /* low 32 bits of mac address */

    GT_U32  hiHalfWordMacAddress = GT_MAC_HIGH16_MAC(macAddrPtr);
                                  /* hi 16 bits of mac address*/
    GT_U32  offset = 0;

    /* 4 zeros, 48bit MAC, 16 bit FID, 12 bit VID1 */

    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr, 0, 12,  vid1);
    offset += 12;
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr, offset, 16, fid);
    offset += 16;
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr, offset, 32, lowWordMacAddress);
    offset += 32;
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr, offset, 16, hiHalfWordMacAddress);
    offset += 16;
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr, offset, 4, 0);

    return GT_OK;
}


/**
* @internal hashCalcFillDipSipFidVid1DataCrc function
* @endinternal
*
* @brief   This function prepares SIP + DIP + VID [+ VID1] for calculating
*         hash using CRC algorithm. VID1 is used if useVid1 = GT_TRUE
*         Generate data of 80 or 96 bits:
*         if useVid1 = GT_TRUE (double lookup key mode) use:
*         (4'h0, SIP[31:0], DIP[31:0] , FID[15:0] , vid1[11:0])
*         else if use 12b FID use
*         (4'h0, SIP[31:0], DIP[31:0] , FID[11:0])
*         else
*         (SIP[31:0], DIP[31:0] , FID[16:0])
* @param[in] useVid1                  - whether to use vid1 in the crc data
*                                      (see "double tag FDB Lookup key mode")
* @param[in] fid16BitHashEn           - indication if FID used 12 or 16 bits
* @param[in] destIpAddr               - destination IP to put to array .
* @param[in] sourceIpAddr             - source IP to put to array, if not used - specify "0"
* @param[in] vlanId                   - Vlan id  to put to array, for shared entry specify "0"
* @param[in] vid1                     - VID1 to put to array
*
* @param[out] dataPtr                  - 3-words array to receive data.(for 76/80 bits)
*                                       none
*
* @note GT_OK - success.
*
*/
static GT_STATUS hashCalcFillDipSipFidVid1DataCrc
(
    IN  GT_BOOL         useVid1,
    IN  GT_BOOL         fid16BitHashEn,
    IN  GT_U32          destIpAddr,
    IN  GT_U32          sourceIpAddr,
    IN  GT_U32          vlanId,
    IN  GT_U32          vid1,
    OUT GT_U32          *dataPtr
)
{
    GT_U32  fidLen; /* length of FID field in hash's input data */
    GT_U32  fid;    /* FID value in hash's input data */
    GT_U32  offset; /* offset */
    if (GT_TRUE == useVid1 || GT_TRUE == fid16BitHashEn)
    {
        fid = vlanId;
        fidLen = 16;
    }
    else
    {
        fid = vlanId & 0xFFF;
        fidLen = 12;
    }
    offset = 0;
    if (GT_TRUE == useVid1)
    {
        U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,offset, 12, vid1);
        offset += 12;
    }
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr, offset, fidLen, fid);
    offset += fidLen;
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr, offset, 32, destIpAddr);
    offset += 32;
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr, offset, 32, sourceIpAddr);
    offset += 32;

    if(12 == fidLen || GT_TRUE == useVid1)
    {
        U32_SET_FIELD_IN_ENTRY_MAC(dataPtr, offset, 4, 0);/*76..79 or 92..95*/
    }

    return GT_OK;
}

/**
* @internal hashCalcFillDipVrfIdDataCrc function
* @endinternal
*
* @brief   This function prepares DIP + VRF-ID data
*         for calculating hash using CRC algorithm
*         Generate data of 44 bits.
* @param[in] destIpAddr               - destination IP to put to array.
* @param[in] vrfId                    - VRF-ID to put to array.
*
* @param[out] dataPtr                  - 2-words array to receive data.(for 44 bits)
*                                       none
*
* @note GT_OK - success.
*
*/
static GT_STATUS hashCalcFillDipVrfIdDataCrc
(
    IN  GT_U32          destIpAddr,
    IN  GT_U32          vrfId,
    OUT GT_U32          *dataPtr
)
{
    GT_U32  tmpBitIndex = 0;

    /* Data_to_CRC_Func = { VRF-ID[43:32], IP Address[31:0]} */
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,tmpBitIndex,32,destIpAddr);
    tmpBitIndex += 32;
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,tmpBitIndex,12,vrfId);

    return GT_OK;
}

/**
* @internal hashCalcFillIpv6DipVrfIdDataCrc function
* @endinternal
*
* @brief   This function prepares IPv6 DIP + VRF-ID data
*         for calculating hash using CRC algorithm
*         Generate data of 140 bits.
* @param[in] destIpAddr[4]            - destination IP to put to array.
* @param[in] vrfId                    - VRF-ID to put to array.
*
* @param[out] dataPtr                  - 5-words array to receive data.(for 140 bits)
*                                       none
*
* @note GT_OK - success.
*
*/
static GT_STATUS hashCalcFillIpv6DipVrfIdDataCrc
(
    IN  GT_U32          destIpAddr[4],
    IN  GT_U32          vrfId,
    OUT GT_U32          *dataPtr
)
{
    GT_U32  tmpBitIndex = 0;

    /* Data_to_CRC_Func = { VRF-ID[139:128], IP Address[127:0]} */
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,tmpBitIndex,32,destIpAddr[0]);
    tmpBitIndex += 32;
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,tmpBitIndex,32,destIpAddr[1]);
    tmpBitIndex += 32;
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,tmpBitIndex,32,destIpAddr[2]);
    tmpBitIndex += 32;
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,tmpBitIndex,32,destIpAddr[3]);
    tmpBitIndex += 32;
    U32_SET_FIELD_IN_ENTRY_MAC(dataPtr,tmpBitIndex,12,vrfId);

    return GT_OK;
}

/**
* @internal xorCalc function
* @endinternal
*
* @brief   This function calculates the XOR hash index for the FDB table.
*         for more details see CPSS_MAC_ENTRY_EXT_KEY_STC description.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hashParamsPtr            - pointer to key parameters of the hash entry.
* @param[in] macEntryKeyPtr           - pointer to key parameters of the mac entry.
* @param[in] fid                      - the FDB id.
* @param[in] sip                      - the  to use.
* @param[in] dip                      - the  to use.
* @param[in] numBitsToUse             - number of bits used to address FDB mac entry in FDB table,
*                                      depends on FDB size
*
* @param[out] hashPtr                  - (pointer to) the hash index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum, bad vlan-Id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS xorCalc
(
    IN  CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC *hashParamsPtr,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC      *macEntryKeyPtr,
    IN  GT_U16                          fid,
    IN  GT_U32                          sip,
    IN  GT_U32                          dip,
    IN  GT_U32                          numBitsToUse,
    OUT GT_U32                          *hashPtr
)
{
    GT_U32  data[8] = {0}; /* use init to avoid GCC warnings */
    GT_U32  xor_numOfWords;/* in XOR mode - number of words used from data[]*/

    if(hashParamsPtr->fid16BitHashEn)
    {
        xor_numOfWords = 8;
    }
    else
    {
        xor_numOfWords = 7;
    }

    switch (macEntryKeyPtr->entryType)
    {
        case CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E:
            xor_numOfWords = 7;
            /* build data */
            hashCalcFillMacVidDataXor(numBitsToUse,
                hashParamsPtr->fid16BitHashEn,
                macEntryKeyPtr->key.macVlan.macAddr.arEther, fid, &(data[0])) ;
            break;
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E:
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E:
            xor_numOfWords = 8;
            /* build data */
            hashCalcFillDipSipVidDataXor(numBitsToUse,
                hashParamsPtr->fid16BitHashEn,
                dip, sip, fid, &(data[0])) ;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(macEntryKeyPtr->entryType);
    }

    /* calc hash according to data */
    hashCalcDataXor(
        &(data[0]), xor_numOfWords, hashPtr) ;
    return GT_OK;

}

/**
* @internal crcCalc function
* @endinternal
*
* @brief   This function calculates the CRC hash index for the FDB table.
*         For more details see CPSS_MAC_ENTRY_EXT_KEY_STC description.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hashParamsPtr            - pointer to key parameters of the hash entry.
* @param[in] macEntryKeyPtr           - pointer to key parameters of the mac entry.
* @param[in] fid                      - the FDB id.
* @param[in] sip                      - the  to use.
* @param[in] dip                      - the  to use.
* @param[in] numBitsToUse             - number of bits used to address FDB mac entry in FDB table,
*                                      depends on FDB size
* @param[in] isSip5_10                - is calculation done for SIP5_10 device.
*
* @param[out] hashPtr                  - (pointer to) the hash index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - bad vlan-Id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported hash function
*/
static GT_STATUS crcCalc
(
    IN  CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC *hashParamsPtr,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC      *macEntryKeyPtr,
    IN  GT_U16                          fid,
    IN  GT_U32                          sip,
    IN  GT_U32                          dip,
    IN  GT_U32                          vid1,
    IN  GT_U32                          numBitsToUse,
    IN  GT_U32                          isSip5_10,
    OUT GT_U32                          *hashPtr
)
{
    GT_U32  data[5]; /* data for hash calculation. Support needed up to 96 bits */
    GT_U32  dataLen; /* length of data (in bits) used for hash calculation. */
    GT_U32  isFid16Bits = hashParamsPtr->fid16BitHashEn;
    GT_STATUS rc = GT_OK;

    switch (macEntryKeyPtr->entryType)
    {
        case CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E:
            /* build data */
            hashCalcFillMacVidDataCrc(
                isFid16Bits,
                macEntryKeyPtr->key.macVlan.macAddr.arEther, fid, &(data[0]),
                hashParamsPtr->crcHashUpperBitsMode);
            dataLen = (isFid16Bits ? 80 : 76);
            break;
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E:
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E:
            /* build data */
            hashCalcFillDipSipFidVid1DataCrc(GT_FALSE, isFid16Bits,
                                             dip, sip, fid, vid1, &(data[0]));
            dataLen = (isFid16Bits ? 80 : 76);
            break;
        case CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E:
            if (!isSip5_10)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            hashCalcFillMacFidVid1DataCrc(
                macEntryKeyPtr->key.macVlan.macAddr.arEther,
                fid, vid1, &data[0]);
            dataLen = 80;
            break;
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E:
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E:
            /* CRC hash function is not supported for double tag lookup mode for
               IPV4/6 FDB entries */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(macEntryKeyPtr->entryType);
    }

    /* calc hash according to data */
    rc = hashCalcDataCrc(&(data[0]), dataLen, numBitsToUse,hashPtr,hashParamsPtr->useZeroInitValueForCrcHash);
    return rc;
}

/**
* @internal fdbHashVrfIdIpv4DipGet function
* @endinternal
*
* @brief   the function return the VRF-ID,IPV4 DIP that need to be used for hashing.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] macEntryKeyPtr           - pointer to key parameters of the mac entry
*                                      according to the entry type:
*                                      CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E -> VRF-ID + IPV4 DIP
*
* @param[out] vrfIdPtr                 - (pointer to) the VRF ID to use
* @param[out] ipv4DipPtr               - (pointer to) the ipv4 dip to use.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - bad vlan-Id
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS fdbHashVrfIdIpv4DipGet
(
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC      *macEntryKeyPtr,
    OUT GT_U32                          *vrfIdPtr,
    OUT GT_U32                          *ipv4DipPtr
)
{
    GT_U32  vrfId = 0;
    GT_U32  ipv4Dip = 0;
    GT_U32  ii;

    switch (macEntryKeyPtr->entryType)
    {
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E:
             /* check vrfId */
            if (macEntryKeyPtr->key.ipv4Unicast.vrfId >= BIT_12)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            vrfId = macEntryKeyPtr->key.ipv4Unicast.vrfId;

            ii = 0;
            /* set the dip field */
            U32_SET_FIELD_NO_CLEAR_MAC(ipv4Dip,24,8,macEntryKeyPtr->key.ipv4Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv4Dip,16,8,macEntryKeyPtr->key.ipv4Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv4Dip, 8,8,macEntryKeyPtr->key.ipv4Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv4Dip, 0,8,macEntryKeyPtr->key.ipv4Unicast.dip[ii++]);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(macEntryKeyPtr->entryType);
    }

    *vrfIdPtr = vrfId;
    *ipv4DipPtr = ipv4Dip;

    return GT_OK;
}

/**
* @internal fdbHashVrfIdIpv6DipGet function
* @endinternal
*
* @brief   the function return the VRF-ID,IPV6 DIP that need to be used for hashing.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] macEntryKeyPtr           - pointer to key parameters of the mac entry
*                                      according to the entry type:
*                                      CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_FULL_ENTRY_E -> VRF-ID + IPV6 DIP
*
* @param[out] vrfIdPtr                 - (pointer to) the VRF ID to use
* @param[out] ipv6DipPtr               - (pointer to) the ipv6 dip to use.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - bad vlan-Id
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS fdbHashVrfIdIpv6DipGet
(
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC      *macEntryKeyPtr,
    OUT GT_U32                          *vrfIdPtr,
    OUT GT_U32                          *ipv6DipPtr
)
{
    GT_U32  ii = 0;

    switch (macEntryKeyPtr->entryType)
    {
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E:
             /* check vrfId */
            if (macEntryKeyPtr->key.ipv6Unicast.vrfId >= BIT_12)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            *vrfIdPtr = macEntryKeyPtr->key.ipv6Unicast.vrfId;

            ii = 0;
            /* set the dip field */
            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[3],24,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[3],16,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[3], 8,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[3], 0,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);

            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[2],24,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[2],16,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[2], 8,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[2], 0,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);

            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[1],24,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[1],16,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[1], 8,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[1], 0,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);

            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[0],24,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[0],16,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[0], 8,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(ipv6DipPtr[0], 0,8,macEntryKeyPtr->key.ipv6Unicast.dip[ii++]);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(macEntryKeyPtr->entryType);
    }

    return GT_OK;
}

/**
* @internal fdbHashFidSipDipVid1Get function
* @endinternal
*
* @brief   the function returns the FID,SIP,DIP,VID1 that need to be used for
*         hashing
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hashParamsPtr            - pointer to hash parameters
* @param[in] macEntryKeyPtr           - pointer to key parameters of the mac entry
*                                      according to the entry type:
*                                      ..._MAC_ADDR_E:        MAC Address   + vlan ID
*                                      ..._IPV4_MCAST_E:      srcIP + dstIP + vlan ID
*                                      ..._IPV6_MCAST_E:      srcIP + dstIP + vlan ID
*                                      ..._MAC_ADDR_VID1_E:   MAC Address   + vlan ID + vid1
*                                      ..._IPV4_MCAST_VID1_E: srcIP + dstIP + vlan ID + vid1
*                                      ..._IPV6_MCAST_VID1_E: srcIP + dstIP + vlan ID + vid1
* @param[in] maxFidValue              - maximum value of Forwarding Id
*
* @param[out] fidPtr                   - (pointer to) the fid to use.
* @param[out] sipPtr                   - (pointer to) the sip to use.
* @param[out] dipPtr                   - (pointer to) the dip to use.
* @param[out] vid1Ptr                  - (pointer to) the vid1 to use.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - bad vlan-Id
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS fdbHashFidSipDipVid1Get
(
    IN  CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC *hashParamsPtr,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC        *macEntryKeyPtr,
    IN  GT_U32                             maxFidValue,
    OUT GT_U16                            *fidPtr,
    OUT GT_U32                            *sipPtr,
    OUT GT_U32                            *dipPtr,
    OUT GT_U32                            *vid1Ptr
)
{
    GT_U16 fid;
    GT_U32 vid1 = 0;
    GT_U32 sip  = 0;
    GT_U32 dip  = 0;
    GT_U32 ii;
    GT_U32 useAllFidBits = 0;
    switch (macEntryKeyPtr->entryType)
    {
        case CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E:
            useAllFidBits = 1;
            vid1 = macEntryKeyPtr->vid1;
            /* No break. Next case should be CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E */
            GT_ATTR_FALLTHROUGH;

        case CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E:
            fid = macEntryKeyPtr->key.macVlan.vlanId;
            break;

        case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E:
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E:
            useAllFidBits = 1;
            vid1 = macEntryKeyPtr->vid1;
            /* no break. Next case should be CPSS_MAC_ENTRY_EXT_TYPE_IPV[46]_MCAST_E */
            GT_ATTR_FALLTHROUGH;

        case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E:
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E:
            fid = macEntryKeyPtr->key.ipMcast.vlanId;
            ii = 0;
            /* set the sip field */
            U32_SET_FIELD_NO_CLEAR_MAC(sip,24,8,macEntryKeyPtr->key.ipMcast.sip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(sip,16,8,macEntryKeyPtr->key.ipMcast.sip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(sip, 8,8,macEntryKeyPtr->key.ipMcast.sip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(sip, 0,8,macEntryKeyPtr->key.ipMcast.sip[ii++]);

            ii = 0;
            /* set the dip field */
            U32_SET_FIELD_NO_CLEAR_MAC(dip,24,8,macEntryKeyPtr->key.ipMcast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(dip,16,8,macEntryKeyPtr->key.ipMcast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(dip, 8,8,macEntryKeyPtr->key.ipMcast.dip[ii++]);
            U32_SET_FIELD_NO_CLEAR_MAC(dip, 0,8,macEntryKeyPtr->key.ipMcast.dip[ii++]);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(macEntryKeyPtr->entryType);
    }
    if(useAllFidBits ||
       hashParamsPtr->fid16BitHashEn)
    {
        /* check vlan-id parameter */
        if(fid > maxFidValue)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* check vlan-id parameter */
        PRV_CPSS_VLAN_VALUE_CHECK_MAC(fid);
    }

    if(hashParamsPtr->vlanMode == CPSS_SVL_E &&
       macEntryKeyPtr->entryType == CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)
    {
        /* the 'SVL' impact only the MAC+FID mode */
        fid = 0;
    }

    *fidPtr  = fid;
    *sipPtr  = sip;
    *dipPtr  = dip;
    *vid1Ptr = vid1;

    return GT_OK;
}

/**
* @internal internal_cpssDxChBrgFdbHashByParamsCalc function
* @endinternal
*
* @brief   This function calculates the hash index for the FDB table.
*         The FDB table holds 4 types of entries :
*         1. for specific mac address and VLAN id.
*         2. for specific src IP, dst IP and VLAN id.
*         3. for specific mac address, VLAN id and VID1.
*         (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*         4. for specific src IP, dst IP, VLAN id and VID1.
*         (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*         for more details see CPSS_MAC_ENTRY_EXT_KEY_STC description.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman
*
* @param[in] hashMode                 - hash mode
* @param[in] hashParamsPtr            - pointer to key parameters of the hash entry
* @param[in] macEntryKeyPtr           - pointer to key parameters of the mac entry
*                                      according to the entry type:
*                                      MAC Address entry -> MAC Address + vlan ID.
*                                      IPv4/IPv6 entry -> srcIP + dstIP + vald ID.
* @param[in] isSip5_10                - is calculation done for SIP5_10 device.
* @param[in] maxFidValue              - maximum value of Forwarding Id
*
* @param[out] hashPtr                  - (pointer to) the hash index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum, bad vlan-Id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported hash function for key type
*/
static GT_STATUS internal_cpssDxChBrgFdbHashByParamsCalc
(
    IN  CPSS_MAC_HASH_FUNC_MODE_ENT        hashMode,
    IN  CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC *hashParamsPtr,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC        *macEntryKeyPtr,
    IN  GT_U32                             isSip5_10,
    IN  GT_U32                             maxFidValue,
    OUT GT_U32                            *hashPtr
)
{
    GT_STATUS   rc;
    GT_U16  fid;
    GT_U32  vid1;
    GT_U32  numBitsToUse;
    GT_U32  sip;
    GT_U32  dip;

    CPSS_NULL_PTR_CHECK_MAC(hashParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(macEntryKeyPtr);
    CPSS_NULL_PTR_CHECK_MAC(hashPtr);

    NUM_BITS_TO_USE_GET_MAC(hashParamsPtr->size, numBitsToUse);

    rc = fdbHashFidSipDipVid1Get(hashParamsPtr,macEntryKeyPtr, maxFidValue,&fid,&sip,&dip,&vid1);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(hashMode)
    {
        case CPSS_MAC_HASH_FUNC_XOR_E:
            rc = xorCalc(hashParamsPtr,macEntryKeyPtr,fid,sip,dip,numBitsToUse,hashPtr);
            break;
        case CPSS_MAC_HASH_FUNC_CRC_E:
            rc = crcCalc(hashParamsPtr,macEntryKeyPtr,fid,sip,dip,vid1,
                         numBitsToUse, isSip5_10, hashPtr);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(hashMode);
    }

    /* the hash START index DOES NOT depend on the macChainLen */
    (*hashPtr) *= LEGACY_NUM_BANKS_FACTOR_CNS;/* 4 is HARD CODED - do not change */

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbHashCalc function
* @endinternal
*
* @brief   This function calculates the hash index for the FDB table.
*         The FDB table holds 4 types of entries :
*         1. for specific mac address and VLAN id.
*         2. for specific src IP, dst IP and VLAN id.
*         3. for specific mac address, VLAN id and VID1.
*         (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
*         4. for specific src IP, dst IP, VLAN id and VID1.
*         (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
*         for more details see CPSS_MAC_ENTRY_EXT_KEY_STC description.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   not supported by Falcon because this API relevant to 'xor'/'crc' modes
*         that not supported by the device.
*
* @param[in] devNum                   - device number
* @param[in] macEntryKeyPtr           - pointer to key parameters of the mac entry
*                                      according to the entry type:
*                                      MAC Address entry -> MAC Address + vlan ID.
*                                      IPv4/IPv6 entry -> srcIP + dstIP + vald ID.
*
* @param[out] hashPtr                  - (pointer to) the hash index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum, bad vlan-Id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported hash function for key type
*/
static GT_STATUS internal_cpssDxChBrgFdbHashCalc
(
    IN  GT_U8                           devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC      *macEntryKeyPtr,
    OUT GT_U32                          *hashPtr
)
{
    GT_STATUS   rc;
    GT_U32  isSip5;      /*flag of supporting SIP 5    devices*/
    GT_U32  isSip5_10;   /*flag of supporting SIP 5_10 devices*/
    GT_U32  isSip5_15;   /*flag of supporting SIP 5_15 devices*/
    GT_U32  maxFidValue; /*maximum value of Forwarding Id*/
    CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC hashParams; /* hash parameters */
    CPSS_MAC_HASH_FUNC_MODE_ENT hashMode;         /* hash mode */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    /* not supported by Falcon because this API relevant to 'xor'/'crc' modes
       that not supported by the device.*/
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(macEntryKeyPtr);
    CPSS_NULL_PTR_CHECK_MAC(hashPtr);

    isSip5    = PRV_CPSS_SIP_5_CHECK_MAC(devNum);
    isSip5_10 = PRV_CPSS_SIP_5_10_CHECK_MAC(devNum);
    isSip5_15 = PRV_CPSS_SIP_5_15_CHECK_MAC(devNum);

    /* fill in hash parameters based on current configuration of device */
    hashParams.vlanMode = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.vlanMode;
    hashParams.crcHashUpperBitsMode = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.crcHashUpperBitsMode;
    hashParams.size = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size;
    hashParams.fid16BitHashEn = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.fid16BitHashEn;
    hashMode = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.hashMode;

    if(hashParams.size < CPSS_DXCH_BRG_FDB_TBL_SIZE_64K_E)
    {
        /* BC2 behave different than Lion2,ch1,2,3,xcat1,2,3 and Bobk */
        hashParams.useZeroInitValueForCrcHash = (isSip5 & (!isSip5_15)) ?
            GT_TRUE : GT_FALSE;
    }
    else
    {
        /* Bobk behave different than Lion2 and Bc2 */
        hashParams.useZeroInitValueForCrcHash = (!isSip5_15) ? GT_TRUE : GT_FALSE;
    }

    maxFidValue = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(devNum);

    rc = internal_cpssDxChBrgFdbHashByParamsCalc(hashMode, &hashParams, macEntryKeyPtr, isSip5_10, maxFidValue, hashPtr);

    return rc;
}

/**
* @internal cpssDxChBrgFdbHashCalc function
* @endinternal
*
* @brief   This function calculates the hash index for the FDB table.
*         The FDB table holds 4 types of entries :
*         1. for specific mac address and VLAN id.
*         2. for specific src IP, dst IP and VLAN id.
*         3. for specific mac address, VLAN id and VID1.
*         (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
*         4. for specific src IP, dst IP, VLAN id and VID1.
*         (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
*         for more details see CPSS_MAC_ENTRY_EXT_KEY_STC description.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   not supported by Falcon because this API relevant to 'xor'/'crc' modes
*         that not supported by the device.
*
* @param[in] devNum                   - device number
* @param[in] macEntryKeyPtr           - pointer to key parameters of the mac entry
*                                      according to the entry type:
*                                      MAC Address entry -> MAC Address + vlan ID.
*                                      IPv4/IPv6 entry -> srcIP + dstIP + vald ID.
*
* @param[out] hashPtr                  - (pointer to) the hash index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum, bad vlan-Id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported hash function for key type
*/
GT_STATUS cpssDxChBrgFdbHashCalc
(
    IN  GT_U8                           devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC      *macEntryKeyPtr,
    OUT GT_U32                          *hashPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbHashCalc);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, macEntryKeyPtr, hashPtr));

    rc = internal_cpssDxChBrgFdbHashCalc(devNum, macEntryKeyPtr, hashPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, macEntryKeyPtr, hashPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChBrgFdbHashByParamsCalc function
* @endinternal
*
* @brief   This function calculates the hash index for the FDB table.
*         The FDB table holds 4 types of entries :
*         1. for specific mac address and VLAN id.
*         2. for specific src IP, dst IP and VLAN id.
*         3. for specific mac address, VLAN id and VID1.
*         (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*         4. for specific src IP, dst IP, VLAN id and VID1.
*         (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*         for more details see CPSS_MAC_ENTRY_EXT_KEY_STC description.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hashMode                 - hash mode
* @param[in] hashParamsPtr            - pointer to CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC
* @param[in] macEntryKeyPtr           - pointer to key parameters of the mac entry
*                                      according to the entry type:
*                                      MAC Address entry -> MAC Address + vlan ID.
*                                      IPv4/IPv6 entry -> srcIP + dstIP + vald ID.
*
* @param[out] hashPtr                  - (pointer to) the hash index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum, bad vlan-Id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported hash function for key type
*/
GT_STATUS cpssDxChBrgFdbHashByParamsCalc
(
    IN  CPSS_MAC_HASH_FUNC_MODE_ENT        hashMode,
    IN  CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC *hashParamsPtr,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC        *macEntryKeyPtr,
    OUT GT_U32                            *hashPtr
)
{
    GT_STATUS rc;
    /* isSip5_10 and maxFidValue enable superset features for API without devNum*/
    GT_U32  isSip5_10 = 1;     /*flag of supporting SIP 5 10 devices*/
    GT_U32  maxFidValue = _8K-1; /*maximum value of Forwarding Id*/

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbHashByParamsCalc);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, hashMode, hashParamsPtr, macEntryKeyPtr, hashPtr));

    rc = internal_cpssDxChBrgFdbHashByParamsCalc(hashMode, hashParamsPtr, macEntryKeyPtr, isSip5_10, maxFidValue, hashPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, hashMode, hashParamsPtr, macEntryKeyPtr, hashPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the FDB table).
*         NOTE: the function do not access the HW , and do only SW calculations.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] hashParamsPtr            - pointer to hash parameters
* @param[in] entryKeyPtr              - (pointer to) entry key
* @param[in] multiHashStartBankIndex  - the first bank for 'multi hash' result.
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] numOfBanks               - number of banks for 'multi hash' result.
*                                      this value indicates the number of elements that will be
*                                      retrieved by crcMultiHashArr[]
*                                      restriction of (numOfBanks + multiHashStartBankIndex) <= 16 .
*                                      (APPLICABLE RANGES: 1..16)
* @param[in] isSip5_10                - flag of supporting SIP 5 10
* @param[in] maxFidValue              - maximum value of Forwarding
*
* @param[out] crcMultiHashArr[]        - (array of) 'multi hash' CRC results. index in this array is 'bank Id'
*                                      (index 0 will hold value relate to bank multiHashStartBankIndex).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc
(
    IN  CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC *hashParamsPtr,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC        *entryKeyPtr,
    IN  GT_U32                             multiHashStartBankIndex,
    IN  GT_U32                             numOfBanks,
    IN  GT_U32                             isSip5_10,
    IN  GT_U32                             maxFidValue,
    OUT GT_U32                             crcMultiHashArr[]
)
{
    GT_STATUS   rc;
    GT_U16  fid;
    GT_U32  sip;
    GT_U32  dip;
    GT_U32  vrfId;
    GT_U32  ipv6Dip[4] = {0,0,0,0};
    GT_U32  hashInputDataArr[5];/* support 140 bits data */
    GT_U32  initCrc = 0xFFFFFFFF;/* use hard coded instead of read from register */
    GT_U32  inputDataNumOfBits;
    GT_U32  vid1;
    GT_U32  numBitsToUse; /*number of bits used to address FDB mac entry in FDB table*/
    GT_BOOL useVid1InIpMcKey = GT_FALSE;
    GT_U32  totalNumOfHashes;

    CPSS_NULL_PTR_CHECK_MAC(hashParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(entryKeyPtr);
    CPSS_NULL_PTR_CHECK_MAC(crcMultiHashArr);

    switch (entryKeyPtr->entryType)
    {
        case CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E:
            if (!isSip5_10)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            rc = fdbHashFidSipDipVid1Get(hashParamsPtr,entryKeyPtr, maxFidValue,&fid,&sip,&dip,&vid1);
            if(rc != GT_OK)
            {
                return rc;
            }

            hashCalcFillMacFidVid1DataCrc(entryKeyPtr->key.macVlan.macAddr.arEther,
                                          fid, vid1, hashInputDataArr);
            inputDataNumOfBits = 80;
            break;

        case CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E:
            rc = fdbHashFidSipDipVid1Get(hashParamsPtr,entryKeyPtr, maxFidValue,&fid,&sip,&dip,&vid1);
            if(rc != GT_OK)
            {
                return rc;
            }
            hashCalcFillMacVidDataCrc(
                hashParamsPtr->fid16BitHashEn,
                entryKeyPtr->key.macVlan.macAddr.arEther,
                fid,
                hashInputDataArr,
                hashParamsPtr->crcHashUpperBitsMode) ;

            inputDataNumOfBits = 80;
            break;
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E:
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E:
            if (!isSip5_10)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            useVid1InIpMcKey = GT_TRUE;
            /* no break. Next case should be CPSS_MAC_ENTRY_EXT_TYPE_IPV[46]_MCAST_E */
            GT_ATTR_FALLTHROUGH;
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E:
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E:
            rc = fdbHashFidSipDipVid1Get(hashParamsPtr,entryKeyPtr, maxFidValue,&fid,&sip,&dip,&vid1);
            if(rc != GT_OK)
            {
                return rc;
            }
            hashCalcFillDipSipFidVid1DataCrc(
                useVid1InIpMcKey,
                hashParamsPtr->fid16BitHashEn,
                dip, sip, fid, vid1, hashInputDataArr) ;

            inputDataNumOfBits = (useVid1InIpMcKey == GT_TRUE? 96 : 80);
           break;
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E:
            rc = fdbHashVrfIdIpv4DipGet(entryKeyPtr,&vrfId,&dip);
            if(rc != GT_OK)
            {
                return rc;
            }
            hashCalcFillDipVrfIdDataCrc(dip,vrfId,hashInputDataArr);

            inputDataNumOfBits = 44;
            break;
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E:
            rc = fdbHashVrfIdIpv6DipGet(entryKeyPtr,&vrfId,&(ipv6Dip[0]));
            if(rc != GT_OK)
            {
                return rc;
            }
            hashCalcFillIpv6DipVrfIdDataCrc(ipv6Dip,vrfId,hashInputDataArr);

            inputDataNumOfBits = 140;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryKeyPtr->entryType);
    }

    NUM_BITS_TO_USE_GET_MAC(hashParamsPtr->size, numBitsToUse);
    NUM_BANKS_TO_USE_GET_MAC(hashParamsPtr->numOfMultipleHashes, totalNumOfHashes)

    numBitsToUse = (totalNumOfHashes == 16) ? numBitsToUse - 2 :
                   (totalNumOfHashes ==  8) ? numBitsToUse - 1 : numBitsToUse;

    return sip5CrcMultiHashCalc(hashInputDataArr, inputDataNumOfBits, numBitsToUse,
                                initCrc, multiHashStartBankIndex, numOfBanks, totalNumOfHashes, GT_FALSE , crcMultiHashArr);
}

/**
* @internal internal_cpssDxChBrgFdbHashCrcMultiResultsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the FDB table).
*         NOTE: the function do not access the HW , and do only SW calculations.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] entryKeyPtr              - (pointer to) entry key
* @param[in] multiHashStartBankIndex  - the first bank for 'multi hash' result.
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] numOfBanks               - number of banks for 'multi hash' result.
*                                      this value indicates the number of elements that will be
*                                      retrieved by crcMultiHashArr[]
*                                      restriction of (numOfBanks + multiHashStartBankIndex) <= 16 .
*                                      (APPLICABLE RANGES: 1..16)
*
* @param[out] crcMultiHashArr[]        - (array of) 'multi hash' CRC results. index in this array is 'bank Id'
*                                      (index 0 will hold value relate to bank multiHashStartBankIndex).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbHashCrcMultiResultsCalc
(
    IN  GT_U8           devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *entryKeyPtr,
    IN  GT_U32       multiHashStartBankIndex,
    IN  GT_U32       numOfBanks,
    OUT GT_U32       crcMultiHashArr[]
)
{
    GT_STATUS   rc;
    GT_U32      isSip5_10;   /*flag of supporting SIP 5 10 device*/
    GT_U32      maxFidValue; /*maximum value of Forwarding Id*/
    GT_U32      ii;
    GT_U32      fdbNumOfBitsPerBank;

    CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC hashParams;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(entryKeyPtr);
    CPSS_NULL_PTR_CHECK_MAC(crcMultiHashArr);

    isSip5_10 = PRV_CPSS_SIP_5_10_CHECK_MAC(devNum);
    hashParams.vlanMode = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.vlanMode;
    hashParams.fid16BitHashEn = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.fid16BitHashEn;
    hashParams.crcHashUpperBitsMode = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.crcHashUpperBitsMode;
    hashParams.size = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size;
    maxFidValue = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(devNum);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        fdbNumOfBitsPerBank = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbNumOfBitsPerBank;
        switch (fdbNumOfBitsPerBank)
        {
            case 2:
                hashParams.numOfMultipleHashes = 4;
                break;
            case 3:
                hashParams.numOfMultipleHashes = 8;
                break;
            default:
                hashParams.numOfMultipleHashes = 16;
        }
    }
    else
    {
        hashParams.numOfMultipleHashes = NUM_BANKS_MULTI_HASH_CNS;
    }


    rc = internal_cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc(&hashParams,
                           entryKeyPtr, multiHashStartBankIndex, numOfBanks,
                                  isSip5_10, maxFidValue,crcMultiHashArr);

    if(entryKeyPtr->entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E &&
        PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        /* for IPv6 odd bank hash eqauls to even bank hash fix for Erratum : FE-2501894 */
        for(ii = (multiHashStartBankIndex+1) & 0xFFFFFFFE ;
            (ii + 1) < numOfBanks;
            ii += 2)
        {
            crcMultiHashArr[(ii + 1) - multiHashStartBankIndex] =
                /* 4 LSbits are 'bank index' the reset like use by bank [ii] */
                (crcMultiHashArr[ii - multiHashStartBankIndex] & 0xFFFFFFF0) + (ii + 1);
        }
    }

    return rc;
}

/**
* @internal cpssDxChBrgFdbHashCrcMultiResultsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the FDB table).
*         NOTE: the function do not access the HW , and do only SW calculations.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] entryKeyPtr              - (pointer to) entry key
* @param[in] multiHashStartBankIndex  - the first bank for 'multi hash' result.
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] numOfBanks               - number of banks for 'multi hash' result.
*                                      this value indicates the number of elements that will be
*                                      retrieved by crcMultiHashArr[]
*                                      restriction of (numOfBanks + multiHashStartBankIndex) <= 16 .
*                                      (APPLICABLE RANGES: 1..16)
*
* @param[out] crcMultiHashArr[]        - (array of) 'multi hash' CRC results. index in this array is 'bank Id'
*                                      (index 0 will hold value relate to bank multiHashStartBankIndex).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbHashCrcMultiResultsCalc
(
    IN  GT_U8           devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *entryKeyPtr,
    IN  GT_U32       multiHashStartBankIndex,
    IN  GT_U32       numOfBanks,
    OUT GT_U32       crcMultiHashArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbHashCrcMultiResultsCalc);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryKeyPtr, multiHashStartBankIndex, numOfBanks, crcMultiHashArr));

    rc = internal_cpssDxChBrgFdbHashCrcMultiResultsCalc(devNum, entryKeyPtr, multiHashStartBankIndex, numOfBanks, crcMultiHashArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryKeyPtr, multiHashStartBankIndex, numOfBanks, crcMultiHashArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the FDB table).
*         NOTE: the function do not access the HW , and do only SW calculations.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] hashParamsPtr            - pointer to CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC
* @param[in] entryKeyPtr              - (pointer to) entry key
* @param[in] multiHashStartBankIndex  - the first bank for 'multi hash' result.
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] numOfBanks               - number of banks for 'multi hash' result.
*                                      this value indicates the number of elements that will be
*                                      retrieved by crcMultiHashArr[]
*                                      restriction of (numOfBanks + multiHashStartBankIndex) <= 16 .
*                                      (APPLICABLE RANGES: 1..16)
*
* @param[out] crcMultiHashArr[]        - (array of) 'multi hash' CRC results. index in this array is 'bank Id'
*                                      (index 0 will hold value relate to bank multiHashStartBankIndex).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc
(
    IN  CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC *hashParamsPtr,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC        *entryKeyPtr,
    IN  GT_U32                             multiHashStartBankIndex,
    IN  GT_U32                             numOfBanks,
    OUT GT_U32                             crcMultiHashArr[]
)
{
    GT_STATUS rc;
    GT_U32    isSip5_10 = 1;
    GT_U32    maxFidValue = _8K-1;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, hashParamsPtr, entryKeyPtr, multiHashStartBankIndex,
                            numOfBanks, crcMultiHashArr));

    rc = internal_cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc(hashParamsPtr,
                                                                entryKeyPtr,
                                                                multiHashStartBankIndex,
                                                                numOfBanks,
                                                                isSip5_10, maxFidValue, crcMultiHashArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, hashParamsPtr, entryKeyPtr,
                              multiHashStartBankIndex, numOfBanks, crcMultiHashArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbHashRequestSend function
* @endinternal
*
* @brief   The function Send Hash request (HR) message to PP to generate all values
*         of hash results that relate to the 'Entry KEY'.
*         The function checks that AU messaging is ready before using it.
*         The function does not check that HR message was processed by PP.
*         For the results Application can call cpssDxChBrgFdbHashResultsGet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] entryKeyPtr              - (pointer to) entry key
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or entryKeyPtr->entryType
* @retval GT_OUT_OF_RANGE          - one of the parameters of entryKeyPtr is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the PP is not ready to get a message from CPU.
*                                       (PP still busy with previous message)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Application should synchronize call of cpssDxChBrgFdbMacEntrySet,
*       cpssDxChBrgFdbQaSend , cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbHashRequestSend
*       , cpssDxChBrgFdbPortGroupMacEntrySet, cpssDxChBrgFdbPortGroupQaSend ,
*       cpssDxChBrgFdbPortGroupMacEntryDelete , cpssDxChBrgFdbMacEntryMove functions.
*       Invocations of these functions should be mutual exclusive because they
*       use same HW resources.
*       Application can check that the HR message processing has completed by
*       cpssDxChBrgFdbFromCpuAuMsgStatusGet or cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet.
*
*/
static GT_STATUS internal_cpssDxChBrgFdbHashRequestSend
(
    IN  GT_U8                       devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *entryKeyPtr
)
{
    return prvCpssDxChBrgFdbHashRequestSend(devNum,entryKeyPtr);
}

/**
* @internal cpssDxChBrgFdbHashRequestSend function
* @endinternal
*
* @brief   The function Send Hash request (HR) message to PP to generate all values
*         of hash results that relate to the 'Entry KEY'.
*         The function checks that AU messaging is ready before using it.
*         The function does not check that HR message was processed by PP.
*         For the results Application can call cpssDxChBrgFdbHashResultsGet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] entryKeyPtr              - (pointer to) entry key
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or entryKeyPtr->entryType
* @retval GT_OUT_OF_RANGE          - one of the parameters of entryKeyPtr is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the PP is not ready to get a message from CPU.
*                                       (PP still busy with previous message)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Application should synchronize call of cpssDxChBrgFdbMacEntrySet,
*       cpssDxChBrgFdbQaSend , cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbHashRequestSend
*       , cpssDxChBrgFdbPortGroupMacEntrySet, cpssDxChBrgFdbPortGroupQaSend ,
*       cpssDxChBrgFdbPortGroupMacEntryDelete , cpssDxChBrgFdbMacEntryMove functions.
*       Invocations of these functions should be mutual exclusive because they
*       use same HW resources.
*       Application can check that the HR message processing has completed by
*       cpssDxChBrgFdbFromCpuAuMsgStatusGet or cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet.
*
*/
GT_STATUS cpssDxChBrgFdbHashRequestSend
(
    IN  GT_U8                       devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *entryKeyPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbHashRequestSend);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryKeyPtr));

    rc = internal_cpssDxChBrgFdbHashRequestSend(devNum, entryKeyPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryKeyPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbHashResultsGet function
* @endinternal
*
* @brief   The function return from the HW the result of last performed AU message
*         from the CPU to PP.
*         Meaning the hash results updated after every call to one of the next:
*         cpssDxChBrgFdbQaSend , cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbHashRequestSend
*         , cpssDxChBrgFdbPortGroupMacEntrySet, cpssDxChBrgFdbPortGroupQaSend ,
*         cpssDxChBrgFdbPortGroupMacEntryDelete , cpssDxChBrgFdbMacEntryMove functions.
*         Application can check that any of those message processing has completed by
*         cpssDxChBrgFdbFromCpuAuMsgStatusGet or cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet.
*         Retrieving the hash calc are optional per type:
*         1. xorHashPtr = NULL --> no retrieve XOR calc
*         2. crcHashPtr = NULL --> no retrieve CRC calc
*         3. numOfBanks = 0  --> no retrieve of 'multi hash' CRC results
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] multiHashStartBankIndex  - the first bank for 'multi hash' result.
*                                      this parameter relevant only when numOfBanks != 0
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] numOfBanks               - number of banks for 'multi hash' result.
*                                      this value indicates the number of elements that will be
*                                      retrieved by crcMultiHashArr[]
*                                      value 0 means that no need to retrieve any 'multi hash' result.
*                                      restriction of (numOfBanks + multiHashStartBankIndex) <= 16 when numOfBanks !=0.
*                                      (APPLICABLE RANGES: 0..16)
* @param[in,out] xorHashPtr               - (pointer to) to receive the XOR hash result.
*                                      NULL value means that caller not need to retrieve this value.
* @param[in,out] crcHashPtr               - (pointer to) to receive the CRC hash result.
*                                      NULL value means that caller not need to retrieve this value.
*
* @param[out] crcMultiHashArr[]        - (array of) 'multi hash' CRC results. index in this array is 'bank Id'.
*                                      NULL is allowed only when numOfBanks = 0
*                                      (index 0 will hold value relate to bank multiHashStartBankIndex).
* @param[in,out] xorHashPtr               - (pointer to) the XOR hash result.
*                                      ignored when NULL pointer.
* @param[in,out] crcHashPtr               - (pointer to) the CRC hash result.
*                                      ignored when NULL pointer.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or multiHashStartBankIndex or numOfBanks
* @retval GT_OUT_OF_RANGE          - one of the parameters of entryKeyPtr is out of range
* @retval GT_BAD_PTR               - crcMultiHashArr[] is NULL pointer when numOfBanks != 0
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbHashResultsGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       multiHashStartBankIndex,
    IN  GT_U32       numOfBanks,
    OUT GT_U32       crcMultiHashArr[],
    INOUT GT_U32     *xorHashPtr,
    INOUT GT_U32     *crcHashPtr
)
{
    GT_STATUS   rc = GT_BAD_PARAM;/* if all input parameters are NULL/0 --> bad param */
    GT_U32      regAddr;        /* register address             */
    GT_U32      hwData;         /* data to read from register   */
    GT_U32      numOfBitsToUse;
    GT_U32      ii,bankIndex;
    GT_U32      factor;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_TBD_BOOKMARK_EARCH/* need to relevant to <supportSingleFdbInstance> */
/*
    if(GT_FALSE == PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.supportSingleFdbInstance)
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }
*/
    if(numOfBanks)
    {
        CPSS_NULL_PTR_CHECK_MAC(crcMultiHashArr);

        if((numOfBanks + multiHashStartBankIndex) > NUM_BANKS_MULTI_HASH_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        NUM_BITS_TO_USE_GET_MULTI_HASH_MODE_MAC(devNum,numOfBitsToUse);
        /* the hash value is one of 16 banks as appose to 'legacy' mode that
           value is one of 4 ... so there are 2 bits that not needed */
        factor = NUM_BANKS_MULTI_HASH_CNS;

        bankIndex = multiHashStartBankIndex;

        for(ii = (multiHashStartBankIndex/2) ; ii < (NUM_BANKS_MULTI_HASH_CNS/2) ; ii++)
        {
            regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBHashResults.FDBMultiHashCRCResultReg[ii];

            rc = prvCpssHwPpReadRegister(devNum,regAddr,&hwData);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(0 == (bankIndex & 1))/*even number*/
            {
                /* need to read value that starts at bit 0 */
                crcMultiHashArr[bankIndex - multiHashStartBankIndex] = factor *
                    (U32_GET_FIELD_MAC(hwData,0,numOfBitsToUse));
                bankIndex++;

                if((bankIndex - multiHashStartBankIndex) >= numOfBanks)
                {
                    /* no need to get more value from the HW */
                    break;
                }
            }

            /* need to value that starts at bit 16 */
            crcMultiHashArr[bankIndex - multiHashStartBankIndex] = factor *
                (U32_GET_FIELD_MAC(hwData,16,numOfBitsToUse));
            bankIndex++;

            if((bankIndex - multiHashStartBankIndex) >= numOfBanks)
            {
                /* no need to get more value from the HW */
                break;
            }
        }
    }

    if(crcHashPtr)
    {
        regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBHashResults.FDBNonMulti_hash_crc_result;

        rc = prvCpssHwPpReadRegister(devNum,regAddr,&hwData);
        if(rc != GT_OK)
        {
            return rc;
        }

        NUM_BITS_TO_USE_GET_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size,numOfBitsToUse);
        factor = LEGACY_NUM_BANKS_FACTOR_CNS;

        /* get according to actual bits */
        (*crcHashPtr) = factor * (U32_GET_FIELD_MAC(hwData,0,numOfBitsToUse));
    }

    if(xorHashPtr)
    {
        regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBHashResults.FDBXorHash;

        rc = prvCpssHwPpReadRegister(devNum,regAddr,&hwData);
        if(rc != GT_OK)
        {
            return rc;
        }

        NUM_BITS_TO_USE_GET_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size,numOfBitsToUse);
        factor = LEGACY_NUM_BANKS_FACTOR_CNS;

        /* get according to actual bits */
        (*xorHashPtr) = factor * (U32_GET_FIELD_MAC(hwData,0,numOfBitsToUse));
    }

    return rc;
}

/**
* @internal cpssDxChBrgFdbHashResultsGet function
* @endinternal
*
* @brief   The function return from the HW the result of last performed AU message
*         from the CPU to PP.
*         Meaning the hash results updated after every call to one of the next:
*         cpssDxChBrgFdbQaSend , cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbHashRequestSend
*         , cpssDxChBrgFdbPortGroupMacEntrySet, cpssDxChBrgFdbPortGroupQaSend ,
*         cpssDxChBrgFdbPortGroupMacEntryDelete , cpssDxChBrgFdbMacEntryMove functions.
*         Application can check that any of those message processing has completed by
*         cpssDxChBrgFdbFromCpuAuMsgStatusGet or cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet.
*         Retrieving the hash calc are optional per type:
*         1. xorHashPtr = NULL --> no retrieve XOR calc
*         2. crcHashPtr = NULL --> no retrieve CRC calc
*         3. numOfBanks = 0  --> no retrieve of 'multi hash' CRC results
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] multiHashStartBankIndex  - the first bank for 'multi hash' result.
*                                      this parameter relevant only when numOfBanks != 0
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] numOfBanks               - number of banks for 'multi hash' result.
*                                      this value indicates the number of elements that will be
*                                      retrieved by crcMultiHashArr[]
*                                      value 0 means that no need to retrieve any 'multi hash' result.
*                                      restriction of (numOfBanks + multiHashStartBankIndex) <= 16 when numOfBanks !=0.
*                                      (APPLICABLE RANGES: 0..16)
* @param[in,out] xorHashPtr               - (pointer to) to receive the XOR hash result.
*                                      NULL value means that caller not need to retrieve this value.
* @param[in,out] crcHashPtr               - (pointer to) to receive the CRC hash result.
*                                      NULL value means that caller not need to retrieve this value.
*
* @param[out] crcMultiHashArr[]        - (array of) 'multi hash' CRC results. index in this array is 'bank Id'.
*                                      NULL is allowed only when numOfBanks = 0
*                                      (index 0 will hold value relate to bank multiHashStartBankIndex).
* @param[in,out] xorHashPtr               - (pointer to) the XOR hash result.
*                                      ignored when NULL pointer.
* @param[in,out] crcHashPtr               - (pointer to) the CRC hash result.
*                                      ignored when NULL pointer.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or multiHashStartBankIndex or numOfBanks
* @retval GT_OUT_OF_RANGE          - one of the parameters of entryKeyPtr is out of range
* @retval GT_BAD_PTR               - crcMultiHashArr[] is NULL pointer when numOfBanks != 0
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbHashResultsGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       multiHashStartBankIndex,
    IN  GT_U32       numOfBanks,
    OUT GT_U32       crcMultiHashArr[],
    INOUT GT_U32     *xorHashPtr,
    INOUT GT_U32     *crcHashPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbHashResultsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, multiHashStartBankIndex, numOfBanks, crcMultiHashArr, xorHashPtr, crcHashPtr));

    rc = internal_cpssDxChBrgFdbHashResultsGet(devNum, multiHashStartBankIndex, numOfBanks, crcMultiHashArr, xorHashPtr, crcHashPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, multiHashStartBankIndex, numOfBanks, crcMultiHashArr, xorHashPtr, crcHashPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgFdbCrcHashUpperBitsModeSet function
* @endinternal
*
* @brief   Set mode how the CRC hash (CPSS_MAC_HASH_FUNC_CRC_E or
*         CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
*         will use 16 'most upper bits' for lookup key of type 'MAC+FID'
*         (CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)
*         relevant when using 'Independent VLAN Learning' (CPSS_IVL_E)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - the upper bits mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbCrcHashUpperBitsModeSet
(
    IN  GT_U8           devNum,
    IN  CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT     mode
)
{
    GT_U32  hwValue;/*value to write to HW */
    GT_U32  regAddr;/* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    switch(mode)
    {
        case CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E:
            hwValue = 0;
            break;
        case CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_FID_E:
            hwValue = 1;
            break;
        case CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_MAC_E:
            hwValue = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(mode);
    }

    /* save info to DB , it influence the HASH calc */
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.crcHashUpperBitsMode = mode;

    /*skip hw write in case of FDB under high availabilty */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbUnderHa == GT_TRUE)
    {
        return GT_OK;
    }

    regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig1;

    /* Set <CRCHashUpperBitsMode> */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 2, hwValue);
}

/**
* @internal cpssDxChBrgFdbCrcHashUpperBitsModeSet function
* @endinternal
*
* @brief   Set mode how the CRC hash (CPSS_MAC_HASH_FUNC_CRC_E or
*         CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
*         will use 16 'most upper bits' for lookup key of type 'MAC+FID'
*         (CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)
*         relevant when using 'Independent VLAN Learning' (CPSS_IVL_E)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - the upper bits mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbCrcHashUpperBitsModeSet
(
    IN  GT_U8           devNum,
    IN  CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT     mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbCrcHashUpperBitsModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChBrgFdbCrcHashUpperBitsModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbCrcHashUpperBitsModeGet function
* @endinternal
*
* @brief   Get mode how the CRC hash (CPSS_MAC_HASH_FUNC_CRC_E or
*         CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
*         will use 16 'most upper bits' for lookup key of type 'MAC+FID'
*         (CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)
*         relevant when using 'Independent VLAN Learning' (CPSS_IVL_E)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) the upper bits mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbCrcHashUpperBitsModeGet
(
    IN  GT_U8           devNum,
    OUT  CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT     *modePtr
)
{
    GT_STATUS   rc;
    GT_U32  hwValue;/* value read from the HW */
    GT_U32  regAddr;/* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig1;

    /* Get <CRCHashUpperBitsMode> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 2, &hwValue);

    switch(hwValue)
    {
        case 0:
            *modePtr = CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;
            break;
        case 1:
            *modePtr = CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_FID_E;
            break;
        case 2:
            *modePtr = CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_MAC_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChBrgFdbCrcHashUpperBitsModeGet function
* @endinternal
*
* @brief   Get mode how the CRC hash (CPSS_MAC_HASH_FUNC_CRC_E or
*         CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
*         will use 16 'most upper bits' for lookup key of type 'MAC+FID'
*         (CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)
*         relevant when using 'Independent VLAN Learning' (CPSS_IVL_E)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) the upper bits mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbCrcHashUpperBitsModeGet
(
    IN  GT_U8           devNum,
    OUT  CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT     *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbCrcHashUpperBitsModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChBrgFdbCrcHashUpperBitsModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChBrgFdbHashDdeByParamsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results for the DDE key
*         (indexes into the DDE partition in the FDB table).
*         NOTE: the function do not access the HW , and do only SW calculations.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] macAddrPtr            - (pointer to) the macAddr
* @param[in] fid                   - the FID  of the key
* @param[in] vid1                  - the vid1 of the key
* @param[in] startBank             - the first bank for hash result.
*                                   (APPLICABLE RANGES: 0..15)
* @param[in] numOfBanks            - number of banks for the hash result.
*                                   this value indicates the number of elements that will be
*                                   retrieved by hashArr[]
*                                   restriction of (numOfBanks + startBank) <= 16 .
*                                   (APPLICABLE RANGES: 1..16)
*
* @param[out] crcMultiHashArr[]    - (array of) 'multi hash' CRC results. index in this array is 'bank Id'
*                                      (index 0 will hold value relate to bank startBank).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBrgFdbHashDdeByParamsCalc
(
    IN  GT_U8     *macAddrPtr,
    IN  GT_U16    fid,
    IN  GT_U16    vid1,
    IN  GT_U32    startBank,
    IN  GT_U32    numOfBanks,
    OUT GT_U32    crcMultiHashArr[]
)
{
    GT_U32  hashInputDataArr[3];/* support 80 bits data */

    hashCalcFillMacFidVid1DataCrc(macAddrPtr,
        fid, vid1, hashInputDataArr);

    return sip5CrcMultiHashCalc(hashInputDataArr,
        80 /*inputDataNumOfBits*/, 16/*numBitsToUse*/, 0xFFFFFFFF/*initCrc*/,
        startBank, numOfBanks, numOfBanks/*totalNumOfHashes*/, GT_TRUE/*returnRawHashValue*/ ,
        crcMultiHashArr);
}


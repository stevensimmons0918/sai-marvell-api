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
* @file prvCpssDxChBrgFdbHashCrc.h
*
* @brief CRC Hash functions
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChBrgFdbHashCrc_h
#define __prvCpssDxChBrgFdbHashCrc_h

#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum CRC_FUNC_ENT
 *
 * @brief Enumeration for hash functions that the SIP5 uses
*/
typedef enum{

    /** CRC type 16A (16 bits result) */
    CRC_FUNC_16A_E,

    /** CRC type 16B (16 bits result) */
    CRC_FUNC_16B_E,

    /** CRC type 16C (16 bits result) */
    CRC_FUNC_16C_E,

    /** CRC type 16D (16 bits result) */
    CRC_FUNC_16D_E,

    /** CRC type 32A (32 bits result) */
    CRC_FUNC_32A_E,

    /** CRC type 32B (32 bits result) */
    CRC_FUNC_32B_E,

    /** CRC type 32K (32 bits result) */
    CRC_FUNC_32K_E,

    /** CRC type 32Q (32 bits result) */
    CRC_FUNC_32Q_E,

    /** CRC type 64 (64 bits result) */
    CRC_FUNC_64_E,

    /** dummy value */
    CRC_FUNC___DUMMY__E

} CRC_FUNC_ENT;

GT_VOID prvCpssDxChBrgFdbHashCrc_16a_d36(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_16a_d44(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_16a_d60(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_16b_d36(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_16b_d44(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_16b_d60(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_16c_d36(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_16c_d44(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_16c_d60(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_16d_d36(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_16d_d44(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_16d_d60(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_32a_d36(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_32a_d44(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_32a_d60(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_32b_d36(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_32b_d44(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_32b_d60(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_32k_d36(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_32k_d44(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_32k_d60(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_32q_d36(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_32q_d44(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_32q_d60(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_64_d36(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_64_d44(GT_U32 *D, GT_U32 *c, GT_U32 *h);
GT_VOID prvCpssDxChBrgFdbHashCrc_64_d60(GT_U32 *D, GT_U32 *c, GT_U32 *h);

/**
* @internal prvCpssDxChBrgFdbHashCrc_32a_d76_noInitValueByNumBits function
* @endinternal
*
* @brief   This function calculates CRC 32a bits according to 76 bits DATA.
*         from the CRC32 only numOfBits are calculated.
* @param[in] numOfBits                - amount of bits in words data array
* @param[in] D                        - array of 32bit words, contain data
*
* @param[out] h                        - the hasbits array
*                                       None.
*
* @note based on HW file : sip_mt_crc_32a_d76.v
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32a_d76_noInitValueByNumBits
(
    IN  GT_U32  numOfBits,
    IN  GT_U32  *D,
    OUT GT_U32  *h
);

/**
* @internal prvCpssDxChBrgFdbHashCrc_32a_d76 function
* @endinternal
*
* @brief   This function calculates CRC 32a bits according to 76 bits DATA.
*         from the CRC32 only numOfBits are calculated.
* @param[in] numOfBits                - amount of bits in words data array
* @param[in] D                        - array of 32bit words, contain data
* @param[in] C                        - array of 32bit words, contain init values of CRC
*
* @param[out] h                        - the hasbits array
*                                       None.
*
* @note based on HW file : sip_mt_crc_32a_d76.v
*
*/
GT_VOID prvCpssDxChBrgFdbHashCrc_32a_d76
(
    IN  GT_U32  numOfBits,
    IN  GT_U32  *D,
    IN  GT_U32  *C,
    OUT GT_U32  *h
);

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
);

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
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __prvCpssDxChBrgFdbHashCrc_h */



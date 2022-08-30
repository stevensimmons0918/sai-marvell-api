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
* @file smacHashExactMatch.h
*
* @brief multi Hash calculate for Exact match key implementation for Falcon.
*
* @version   1
********************************************************************************
*/
#ifndef __smacHashExactMatchh
#define __smacHashExactMatchh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <os/simTypes.h>
#include <asicSimulation/SKernel/suserframes/snetFalconExactMatch.h>

/* multiHash init value  */
#define SIP6_EXACT_MATCH_CRC_INIT_VALUE_CNS                   0xFFFFFFFF

/* Enum values represent size of em Table */
typedef enum{

    SIP6_EXACT_MATCH_TABLE_SIZE_4KB      ,
    SIP6_EXACT_MATCH_TABLE_SIZE_8KB      ,
    SIP6_EXACT_MATCH_TABLE_SIZE_16KB     ,
    SIP6_EXACT_MATCH_TABLE_SIZE_32KB     ,
    SIP6_EXACT_MATCH_TABLE_SIZE_64KB     ,
    SIP6_EXACT_MATCH_TABLE_SIZE_128KB    ,
    SIP6_EXACT_MATCH_TABLE_SIZE_256KB    ,
    SIP6_EXACT_MATCH_TABLE_SIZE_512KB    ,

    SIP6_EXACT_MATCH_TABLE_SIZE_LAST_E    /* last value */

}SIP6_EXACT_MATCH_TABLE_SIZE_ENT;

/* Enum values represent num of Multiple Hash Tables */
typedef enum{

    SIP6_EXACT_MATCH_MHT_4_E    ,         /* Four Multiple Hash Tables */
    SIP6_EXACT_MATCH_MHT_8_E    ,         /* Eight Multiple Hash Tables */
    SIP6_EXACT_MATCH_MHT_16_E  ,         /* Sixteen Multiple Hash Tables */

    SIP6_EXACT_MATCH_MHT_LAST_E           /* last value */

}SIP6_EXACT_MATCH_MHT_ENT;

/**
* @internal smacHashExactMatchMultiHashResultsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*          (indexes into the Exact Match table).
*          NOTE: the function do not access the HW , and do only SW calculations.
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] entryKeyPtr              - (pointer to) entry key
* @param[in] keySize                  -  key size
* @param[out] crcMultiHashArr[]       - (array of) 'multi hash' CRC results. index in this array is 'bank Id'
*                                       (index 0 will hold value relate to bank multiHashStartBankIndex).
* @param[out] numOfValidBank          - (pointer to) num Of Valid Banks
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS smacHashExactMatchMultiHashResultsCalc
(
   IN  IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
   IN  GT_U32                              *entryKeyPtr,
   IN  SIP6_EXACT_MATCH_KEY_SIZE_ENT        keySize,
   OUT GT_U32                               crcMultiHashArr[],
   OUT GT_U32                              *numOfValidBank
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __smacHashExactMatchh */




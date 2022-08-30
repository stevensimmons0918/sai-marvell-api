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
* @file prvCpssDxChExactMatchHash.c
*
* @brief Hash calculate for EXACT MATCH table implementation.
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatchHash.h>
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatchHashCrc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*the table gives back nuber of bits to take from the hash calc as sizeOfEm (column) / MTH (row)
 * 0x00ff 8 bits  ,0x01ff 9 bits  ,0x03ff 10 bits ,0x07ff 11 bits ,
 * 0x0fff 12 bits ,0x1fff 13 bits ,0x3fff 14 bits ,0x7fff 15 bits ,
 * 0xffff 16 bits ,0x1ffff 17 bits
 */
static  GT_U32 bitsTotake[3][8] ={
        /*  4k     8k     16k    32k   64k    128k   256k   512k  */
/*4MTH*/ {0x03ff,0x07ff,0x0fff,0x1fff,0x3fff,0x7fff,0xffff,0x1ffff},
/*8MTH*/ {0x01ff,0x03ff,0x07ff,0x0fff,0x1fff,0x3fff,0x7fff,0xffff },
/*16MTH*/{0x00ff,0x01ff,0x03ff,0x07ff,0x0fff,0x1fff,0x3fff,0x7fff }
         };

/**
* @internal hashConfig function
* @endinternal
*
* @brief   This function get the key size and array of indexes and
*          replicates the index accordingly .
*
* @param[in]  keySize    - size of Exact Match key.
* @param[in]  mask       - mask of bits .
* @param[in]  Banks      - number of MHT .
* @param[out] HashArr[]  - (array of) 'multi hash' CRC results.
*
* @retval GT_OK          - on success
* @retval GT_FAIL        - on error
*/
static GT_STATUS hashConfig
(
    IN CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT    keySize,
    IN GT_U32                                mask,
    IN GT_U32                                banks,
    OUT GT_U32                               HashArr[]
)
{
    GT_STATUS rc =GT_OK;
    GT_U32 ii=0;

    switch (keySize)
    {
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
            for (ii=0; ii<(banks/2) ;ii++)
            {
                HashArr[banks - 2*ii - 1] = HashArr[(banks/2 - ii - 1)];
                HashArr[banks - 2*ii - 2] = HashArr[(banks/2 - ii - 1)];
            }
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
            HashArr[15] = 0;
            HashArr[14] = HashArr[13] = HashArr[12] = HashArr[4];
            HashArr[11] = HashArr[10] = HashArr[9]  = HashArr[3];
            HashArr[8]  = HashArr[7]  = HashArr[6]  = HashArr[2];
            HashArr[5]  = HashArr[4]  = HashArr[3]  = HashArr[1];
            HashArr[2]  = HashArr[1]                = HashArr[0];

            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
            for (ii=0; ii<(banks/4) ;ii++)
            {
                HashArr[banks - 4*ii - 1] = HashArr[(banks/4 - ii - 1)];
                HashArr[banks - 4*ii - 2] = HashArr[(banks/4 - ii - 1)];
                HashArr[banks - 4*ii - 3] = HashArr[(banks/4 - ii - 1)];
                HashArr[banks - 4*ii - 4] = HashArr[(banks/4 - ii - 1)];
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "fail in prvDxChExactMatchEntryToHwformat illegal keySize\n") ;
    }

    /*take mask bits of the index and add to the lower x bits  the bank index
     * x value: 2 (4 banks)
     *          3 (8 banks)
     *          4 (16 banks)
    */
    for (ii=0 ;ii<banks ;ii++)
       {
         HashArr[ii] = HashArr[ii] & mask ;
         HashArr[ii] = (HashArr[ii]*banks) + ii ;
       }
    /*clear not valid banks index*/
    for (ii=banks;ii<CPSS_DXCH_EXACT_MATCH_MAX_NUM_BANKS_CNS ;ii++)
    {
        HashArr[ii]= 0;
    }
    return rc;
}

/**
* @internal exactMatchGlobalConfigurationGet function
* @endinternal
*
* @brief   get Exact match table size , number of banks and crc init vlaue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in]  devNum               - the device number
* @param[out]  exactMatchSize       - (pointer to) Exact Match total Size
* @param[out]  exactMatchBanksNum   - (pointer to) Exact Match Banks number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_VALUE             - on Illegal value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_U32 exactMatchGlobalConfigurationGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_EXACT_MATCH_TABLE_SIZE_ENT      *exactMatchSizePtr,
    OUT CPSS_EXACT_MATCH_MHT_ENT             *exactMatchBanksNumPtr
)
{
    GT_U32  exactMatchSize=0;
    GT_U32  exactMatchNumOfBanks=0;

    CPSS_NULL_PTR_CHECK_MAC(exactMatchSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(exactMatchBanksNumPtr);

    exactMatchSize = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum ;

    PRV_CPSS_DXCH_EXACT_MATCH_NUM_OF_BANKS_GET_MAC(devNum,exactMatchNumOfBanks);


   switch(exactMatchSize)
    {
        /*case _4KB: *exactMatchSizePtr=CPSS_EXACT_MATCH_TABLE_SIZE_4KB;
                break;  */
        /*case _8KB: *exactMatchSizePtr=CPSS_EXACT_MATCH_TABLE_SIZE_8KB;
                break;  */
        case _16KB: *exactMatchSizePtr=CPSS_EXACT_MATCH_TABLE_SIZE_16KB;
                break;
        case _32KB: *exactMatchSizePtr=CPSS_EXACT_MATCH_TABLE_SIZE_32KB;
                break;
        case _64KB: *exactMatchSizePtr=CPSS_EXACT_MATCH_TABLE_SIZE_64KB;
                break;
        case _128KB: *exactMatchSizePtr=CPSS_EXACT_MATCH_TABLE_SIZE_128KB;
                break;
        /*case _256KB: *exactMatchSizePtr=CPSS_EXACT_MATCH_TABLE_SIZE_256KB;
                break;  */
        /*case _512KB: *exactMatchSizePtr=CPSS_EXACT_MATCH_TABLE_SIZE_512KB;
                break;  */
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }


    switch(exactMatchNumOfBanks)
    {
        case 4:
            *exactMatchBanksNumPtr=CPSS_EXACT_MATCH_MHT_4_E;
            break;
        case 8:
            *exactMatchBanksNumPtr=CPSS_EXACT_MATCH_MHT_8_E;
            break;
        case 16:
            *exactMatchBanksNumPtr=CPSS_EXACT_MATCH_MHT_16_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
            break;
    }

    return GT_OK;
}

/**
* @internal internal_prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the Exact Match table).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[out] exactMatchSize       - (pointer to) Exact Match total Size
* @param[out] exactMatchMht        - (pointer to) num of Multiple Hash Tables
* @param[in]  entryKeyPtr          - (pointer to) entry key
* @param[out] numberOfElemInCrcMultiHashArrPtr - (pointer to) number of valid
*                                                elements in the exactMatchCrcMultiHashArr
* @param[out] crcMultiHashArr[]    - (array of) 'multi hash' CRC results.
*                                               index in this array is entry inside the bank
*                                               + bank Id'
*                                               size of exactMatchCrcMultiHashArr must be 16
*                                               (CPSS_DXCH_EXACT_MATCH_MAX_NUM_BANKS_CNS)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: the function doesn't access the HW,
*       and do only SW calculations.
*
* The output of the function is indexes that should be
* used for setting the Exact Match entry with cpssDxChExactMatchPortGroupEntrySet.
* Entry with Key size 5B  occupy 1 entry
* Entry with Key size 19B occupy 2 entries
* Entry with Key size 33B occupy 3 entries
* Entry with Key size 47B occupy 4 entries
*
* The logic is as follow according to the key size and bank number.
* keySize   exactMatchSize    function output
* 5 bytes     4 banks         x,y,z,w      (4 separate indexes)
* 19 bytes    4 banks         x,x+1,y,y+1  (2 indexes to be used in pairs)
* 33 bytes    4 banks         x,x+1,x+2    (1 indexes to be used for a single entry)
* 47 bytes    4 banks         x,x+1,x+2,x+3(1 indexes to be used for a single entry)
*
* keySize   exactMatchSize    function output
* 5 bytes     8 banks         x,y,z,w,a,b,c,d            (8 separate indexes)
* 19 bytes    8 banks         x,x+1,y,y+1,z,z+1,w,w+1    (4 indexes to be used in pairs)
* 33 bytes    8 banks         x,x+1,x+2,y,y+1,y+2        (2 indexes to be used for a single entry)
* 47 bytes    8 banks         x,x+1,x+2,x+3,y,y+1,y+2,y+3(2 indexes to be used for a single entry)
*
* and so on for 16 banks
*
* The cpssDxChExactMatchPortGroupEntrySet should get only the first index to be used in the API
* The API set the consecutive indexes according to the key size
*
* keySize   exactMatchSize    function input
* 5 bytes     4 banks         x or y or z or w(4 separate indexes options)
* 19 bytes    4 banks         x or y          (2 indexes options)
* 33 bytes    4 banks         x               (1 indexes option)
* 47 bytes    4 banks         x               (1 indexes option)
*
* keySize   exactMatchSize    function output
* 5 bytes     8 banks         x or y or z or w or a or b or c or d(8 separate indexes options)
* 19 bytes    8 banks         x or y or z or w(4 indexes options)
* 33 bytes    8 banks         x or y          (2 indexes options)
* 47 bytes    8 banks         x or y          (2 indexes options)
*
*/
static GT_STATUS internal_prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt
(
    IN  CPSS_EXACT_MATCH_TABLE_SIZE_ENT exactMatchSize,
    IN  CPSS_EXACT_MATCH_MHT_ENT        exactMatchMht,
    IN  CPSS_DXCH_EXACT_MATCH_KEY_STC   *entryKeyPtr,
    OUT GT_U32                          *numberOfElemInCrcMultiHashArrPtr,
    OUT GT_U32                          crcMultiHashArr[] /*maxArraySize=16*/
)
{
   GT_STATUS   rc = GT_OK;
   GT_U32  i,numBitsInData = PRV_CPSS_DXCH_EXACT_MATCH_HASH_DATA_SIZE_VALUE_CNS;
   GT_U32  initCrc = PRV_CPSS_DXCH_EXACT_MATCH_HASH_CRC_INIT_VALUE_CNS ;
   GT_U32  numOfBanks,mask;
   GT_U64  initCrc64,poly64;
   GT_U32  exactMatchKeysize=0;
   GT_U8   pattern[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS] = {0};

   CPSS_NULL_PTR_CHECK_MAC(entryKeyPtr);
   CPSS_NULL_PTR_CHECK_MAC(numberOfElemInCrcMultiHashArrPtr);
   CPSS_NULL_PTR_CHECK_MAC(crcMultiHashArr);

   if(exactMatchSize>=CPSS_EXACT_MATCH_TABLE_SIZE_LAST_E)
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "exactMatchSize[%d] is not a legal value supported",exactMatchSize);
   }
   if (exactMatchMht>=CPSS_EXACT_MATCH_MHT_LAST_E)
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "exactMatchMht[%d] is not a legal value supported",exactMatchMht);
   }

   mask = bitsTotake[exactMatchMht][exactMatchSize]; 

   /* build pattern according to key size */
   switch (entryKeyPtr->keySize)
   {
       case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
           exactMatchKeysize = 5;
           break;
       case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
           exactMatchKeysize = 19;
           break;
       case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
           exactMatchKeysize = 33;
           break;
       case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
           exactMatchKeysize = 47;
           break;
       default:
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
   }

   for (i=0;i<exactMatchKeysize;i++)
   {
       pattern[i] = entryKeyPtr->pattern[i];
   }

   prvCpssDxChExactMatchHashCrc_16(pattern, numBitsInData, 0x8005, (GT_U16) initCrc  , &crcMultiHashArr[0]) ;
   prvCpssDxChExactMatchHashCrc_16(pattern, numBitsInData, 0x1021, (GT_U16) initCrc  , &crcMultiHashArr[1]) ;
   prvCpssDxChExactMatchHashCrc_16(pattern, numBitsInData, 0x8bb7, (GT_U16) initCrc  , &crcMultiHashArr[2]) ;
   prvCpssDxChExactMatchHashCrc_16(pattern, numBitsInData, 0x3d65, (GT_U16) initCrc  , &crcMultiHashArr[3]) ;

   if (exactMatchMht > CPSS_EXACT_MATCH_MHT_4_E )/*have more than 4 banks*/
   {
       prvCpssDxChExactMatchHashCrc_32(pattern, numBitsInData, 0x04C11DB7, initCrc ,0, &crcMultiHashArr[4]) ;
       prvCpssDxChExactMatchHashCrc_32(pattern, numBitsInData, 0x04C11DB7, initCrc ,1, &crcMultiHashArr[5]) ;
       prvCpssDxChExactMatchHashCrc_32(pattern, numBitsInData, 0x1EDC6F41, initCrc ,0, &crcMultiHashArr[6]) ;
       prvCpssDxChExactMatchHashCrc_32(pattern, numBitsInData, 0x1EDC6F41, initCrc ,1, &crcMultiHashArr[7]) ;

       if (exactMatchMht > CPSS_EXACT_MATCH_MHT_8_E )/*have more than 8 banks*/
       {
           initCrc64.l[0]= initCrc;
           initCrc64.l[1]= initCrc;
           poly64.l[0]= 0xA9EA3693;
           poly64.l[1]= 0x42F0E1EB;

           prvCpssDxChExactMatchHashCrc_32(pattern, numBitsInData, 0x741B8CD7, initCrc ,0, &crcMultiHashArr[8])  ;
           prvCpssDxChExactMatchHashCrc_32(pattern, numBitsInData, 0x741B8CD7, initCrc ,1, &crcMultiHashArr[9])  ;
           prvCpssDxChExactMatchHashCrc_32(pattern, numBitsInData, 0x814141AB, initCrc ,0, &crcMultiHashArr[10]) ;
           prvCpssDxChExactMatchHashCrc_32(pattern, numBitsInData, 0x814141AB, initCrc ,1, &crcMultiHashArr[11]) ;
           prvCpssDxChExactMatchHashCrc_64(pattern, numBitsInData, poly64, initCrc64 ,0, &crcMultiHashArr[12]) ;
           prvCpssDxChExactMatchHashCrc_64(pattern, numBitsInData, poly64, initCrc64 ,1, &crcMultiHashArr[13]) ;
           prvCpssDxChExactMatchHashCrc_64(pattern, numBitsInData, poly64, initCrc64 ,2, &crcMultiHashArr[14]) ;
           prvCpssDxChExactMatchHashCrc_64(pattern, numBitsInData, poly64, initCrc64 ,3, &crcMultiHashArr[15]) ;
       }
   }

   switch (exactMatchMht)
   {
        case CPSS_EXACT_MATCH_MHT_4_E: numOfBanks = 4;
            break;
        case CPSS_EXACT_MATCH_MHT_8_E: numOfBanks = 8;
            break;
        case CPSS_EXACT_MATCH_MHT_16_E: numOfBanks = 16;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
   }

   rc = hashConfig(entryKeyPtr->keySize,mask,numOfBanks,&crcMultiHashArr[0]);
   if (rc != GT_OK)
   {
       return rc;
   }

   numOfBanks = (((numOfBanks / (((GT_U32)entryKeyPtr->keySize)+1))) * (((GT_U32)entryKeyPtr->keySize)+1));

   *numberOfElemInCrcMultiHashArrPtr = numOfBanks;
   return rc;
}

/**
* @internal prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the Exact Match table).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[out] exactMatchSize                    - (pointer to) Exact Match total Size
* @param[out] exactMatchMht                     - (pointer to) num of Multiple Hash Tables
* @param[in] entryKeyPtr                        - (pointer to) entry key
* @param[in] numberOfElemInCrcMultiHashArrPtr   - (pointer to) number of valid
*                                                   elements in the exactMatchCrcMultiHashArr
* @param[out] exactMatchCrcMultiHashArr[]       - (array of) 'multi hash' CRC results.
*                                                   index in this array is entry inside the bank
*                                                   + bank Id'
*                                                   size of exactMatchCrcMultiHashArr must be 16
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: the function doesn't access the HW, and do only SW calculations.
*/
GT_STATUS prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt
(
   IN  CPSS_EXACT_MATCH_TABLE_SIZE_ENT    exactMatchSize,
   IN  CPSS_EXACT_MATCH_MHT_ENT           exactMatchMht,
   IN  CPSS_DXCH_EXACT_MATCH_KEY_STC      *entryKeyPtr,
   OUT GT_U32                             *numberOfElemInCrcMultiHashArrPtr,
   OUT GT_U32                             crcMultiHashArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchSize,exactMatchMht,entryKeyPtr, numberOfElemInCrcMultiHashArrPtr,crcMultiHashArr));

    rc = internal_prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt(exactMatchSize,
                                                                          exactMatchMht,
                                                                          entryKeyPtr,
                                                                          numberOfElemInCrcMultiHashArrPtr,
                                                                          crcMultiHashArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchSize,exactMatchMht, entryKeyPtr, numberOfElemInCrcMultiHashArrPtr, crcMultiHashArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the Exact Match table).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                           - device number
* @param[in] entryKeyPtr                      - (pointer to) entry key
* @param[in] numberOfElemInCrcMultiHashArrPtr - (pointer to) number of valid
*                                                elements in the exactMatchCrcMultiHashArr
* @param[out] exactMatchCrcMultiHashArr[]     - (array of) 'multi hash' CRC results.
*                                               index in this array is entry inside the bank
*                                               + bank Id'
*                                               size of exactMatchCrcMultiHashArr must be 16
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: the function doesn't access the HW,
*       and do only SW calculations.
*/
GT_STATUS internal_prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalc
(
   IN  GT_U8                               devNum,
   IN  CPSS_DXCH_EXACT_MATCH_KEY_STC       *entryKeyPtr,
   OUT GT_U32                              *numberOfElemInCrcMultiHashArrPtr,
   OUT GT_U32                              crcMultiHashArr[]

)
{
   GT_STATUS   rc = GT_OK;
   CPSS_EXACT_MATCH_TABLE_SIZE_ENT  exactMatchSize; /*for system value*/
   CPSS_EXACT_MATCH_MHT_ENT         mht;            /*for system value*/

   PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
   PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT2_E |
                                         CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);
   PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

   CPSS_NULL_PTR_CHECK_MAC(entryKeyPtr);
   CPSS_NULL_PTR_CHECK_MAC(numberOfElemInCrcMultiHashArrPtr);
   CPSS_NULL_PTR_CHECK_MAC(crcMultiHashArr);

   rc = exactMatchGlobalConfigurationGet(devNum,&exactMatchSize,&mht);
   if(rc != GT_OK) {
       return rc;
   }

   rc = internal_prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt(exactMatchSize,
                                                                         mht,
                                                                         entryKeyPtr,
                                                                         numberOfElemInCrcMultiHashArrPtr,
                                                                         crcMultiHashArr);
   return rc;
}

/**
* @internal prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the Exact Match table).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                           - device number
* @param[in] entryKeyPtr                      - (pointer to) entry key
* @param[in] numberOfElemInCrcMultiHashArrPtr - (pointer to) number of valid
*                                                elements in the exactMatchCrcMultiHashArr
* @param[out] exactMatchCrcMultiHashArr[]     - (array of) 'multi hash' CRC results.
*                                               index in this array is entry inside the bank
*                                               + bank Id'
*                                               size of exactMatchCrcMultiHashArr must be 16
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: the function doesn't access the HW,
*       and do only SW calculations.
*/
GT_STATUS prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalc
(
   IN  GT_U8                               devNum,
   IN  CPSS_DXCH_EXACT_MATCH_KEY_STC      *entryKeyPtr,
   OUT GT_U32                             *numberOfElemInCrcMultiHashArrPtr,
   OUT GT_U32                              crcMultiHashArr[]

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChEmMultiHashResultsCalc);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryKeyPtr, numberOfElemInCrcMultiHashArrPtr,crcMultiHashArr));

    rc = internal_prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum,
                                                                       entryKeyPtr,
                                                                       numberOfElemInCrcMultiHashArrPtr,
                                                                       crcMultiHashArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryKeyPtr, numberOfElemInCrcMultiHashArrPtr,crcMultiHashArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


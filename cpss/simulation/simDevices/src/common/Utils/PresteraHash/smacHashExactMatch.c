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
* @file smacHashExactMatch.c
*
* @brief multi Hash calculate for Exact match key implementation for Falcon.
*
* @version   1
********************************************************************************
*/
#include <common/Utils/PresteraHash/smacHashExactMatch.h>
#include <asicSimulation/SKernel/suserframes/snetFalconExactMatch.h>
#include <asicSimulation/SKernel/cheetahCommon/sregFalcon.h>
#include <asicSimulation/SLog/simLog.h>


#define GT_UL64 unsigned long long

#define CRC_16_WIDTH  (8 * sizeof(GT_U16))
#define CRC_16_TOPBIT 0x8000

#define CRC_32_WIDTH  (8 * sizeof(GT_U32))
#define CRC_32_TOPBIT 0x80000000

#define CRC_64_WIDTH  (8 * sizeof(GT_UL64))
#define CRC_64_TOPBIT 0x8000000000000000


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

/*convert from 32 bit array into 8 bit  */
static void convert
(
    IN GT_U32 * dataIn,
    OUT GT_U8 * dataOut
)
{
    GT_U32 byte,word;

    for (byte = 0; byte < SIP6_EXACT_MATCH_MAX_KEY_SIZE_WORD_CNS; byte++)
    {
        word=dataIn[byte];
        if (byte!= (SIP6_EXACT_MATCH_MAX_KEY_SIZE_WORD_CNS-1) )
        {
            dataOut[(byte*4)]=word&0xff;
            dataOut[(byte*4)+1]=(word&0xff00)>>8;
            dataOut[(byte*4)+2]=(GT_U8)((word&0xff0000)>>16);
            dataOut[(byte*4)+3]=(word&0xff000000)>>24;
        }
        else
        {
            dataOut[44]=word&0xff;
            dataOut[45]=(word&0xff00)>>8;
            dataOut[46]=(GT_U8)((word&0xff0000)>>16);
        }
    }
}

/**
* @internal hashConfig function
* @endinternal
*
* @brief   This function get the key size and array of indexes and
*          Replicates the index accordingly .
*
* @param[in]  keySize    - size of EM key.
* @param[in]  Banks      - number of MHT .
* @param[out] HashArr[]  - (array of) 'multi hash' CRC results.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS hashConfig
(
    IN SIP6_EXACT_MATCH_KEY_SIZE_ENT       keySize,
    IN GT_U32                     mask,
    IN GT_U32                     banks,
    OUT GT_U32                    HashArr[]
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 ii = 0;

    switch (keySize)
    {
        case SIP6_EXACT_MATCH_KEY_SIZE_5B_E :
            break;
        case SIP6_EXACT_MATCH_KEY_SIZE_19B_E : /*bank n+1 should have the same index as n*/
            for (ii=0; ii<(banks/2) ;ii++)
            {
                HashArr[banks - 2*ii - 1] = HashArr[(banks/2 - ii - 1)];
                HashArr[banks - 2*ii - 2] = HashArr[(banks/2 - ii - 1)];
            }

            break;
        case SIP6_EXACT_MATCH_KEY_SIZE_33B_E : /*banks n+1 n+2 should have the same index as n*/
            HashArr[14] = HashArr[13] = HashArr[12] = HashArr[4];
            HashArr[11] = HashArr[10] = HashArr[9]  = HashArr[3];
            HashArr[8]  = HashArr[7]  = HashArr[6]  = HashArr[2];
            HashArr[5]  = HashArr[4]  = HashArr[3]  = HashArr[1];
            HashArr[2]  = HashArr[1]                = HashArr[0];
            break;
        case SIP6_EXACT_MATCH_KEY_SIZE_47B_E : /*banks n+1 n+2 n+3 should have the same index as n*/
            for (ii=0; ii<(banks/4) ;ii++)
            {
                HashArr[banks - 4*ii - 1] = HashArr[(banks/4 - ii - 1)];
                HashArr[banks - 4*ii - 2] = HashArr[(banks/4 - ii - 1)];
                HashArr[banks - 4*ii - 3] = HashArr[(banks/4 - ii - 1)];
                HashArr[banks - 4*ii - 4] = HashArr[(banks/4 - ii - 1)];
            }

            break;
        default:
            return GT_FAIL ;
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
    for (ii=banks;ii<SIP6_EXACT_MATCH_MAX_NUM_BANKS_CNS ;ii++)
    {
        HashArr[ii]= 0;
    }
    return rc;
}

/**
* @internal crcEM64 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in]  message   - array of 8 bit words, contain data.
* @param[in]  nBits     - num of bits in the message .
* @param[in]  poly      - the polynomial value .
* @param[in]  seed      - init value .
* @param[out] ans       - the hash result.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static void crcEm64
(
    IN GT_U8                   * message,
    IN GT_U32                    nBits,
    IN long long                 poly,
    IN GT_UL64                   seed,
    IN GT_U8                     mode,
    OUT GT_U32                 * ans
)
{
    GT_U8 byteMsg,bit ;
    GT_UL64  remainder = seed ,tmp;
    GT_U32 nBytes ,byte;
    nBytes = nBits / 8 ;

    /*
     * Perform modulo-2 division, a byte at a time.
     */
    for (byte = 0; byte < nBytes; ++byte)
    {
        /*
         * Bring the next byte into the remainder.
         */
        byteMsg = message[nBytes-byte-1];
        tmp= 0x0100000000000000;
        remainder ^= (byteMsg * tmp);

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for ( bit = 8; bit > 0; --bit)
        {
            if (remainder & (GT_UL64) CRC_64_TOPBIT)
            {
                remainder = (remainder << 1) ^ poly;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }
    switch (mode)
    {
        case 0: remainder=remainder&0xffff;            break;
        case 1: remainder=(remainder&0xffff0000)>>16;  break;
        case 2: remainder=(remainder>>32) & 0xffff;    break;
        case 3: remainder=(remainder>>48) & 0xffff;    break;
        default:  break;
    }
    *ans=(GT_U16)remainder;
}

/**
* @internal crcEM32 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in]  message   - array of 8 bit words, contain data.
* @param[in]  nBits     - num of bits in the message.
* @param[in]  poly      - the polynomial value .
* @param[in]  seed      - init value.
* @param[out] ans       - the hash result.
*
*/
static void crcEm32
(
    IN GT_U8                   * message,
    IN GT_U32                    nBits,
    IN long long                 poly,
    IN GT_U32                    seed,
    IN GT_U8                     mode,
    OUT GT_U32                 * ans
)
{
    GT_U8 byteMsg,bit ;
    GT_U32  remainder = seed;
    GT_U32 nBytes ,byte;

    nBytes = nBits / 8 ;

    /*
     * Perform modulo-2 division, a byte at a time - full byte .
     */
    for (byte = 0; byte < nBytes; ++byte)
    {
        /*
         * Bring the next byte into the remainder.
         */
        byteMsg = message[nBytes-byte-1];
        remainder ^= (byteMsg << (CRC_32_WIDTH - 8));

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (bit = 8; bit > 0; --bit)
        {
            if (remainder & CRC_32_TOPBIT)
            {
                remainder = (GT_U32)((remainder << 1) ^ poly);
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }
    if (mode == 0)
    {
        remainder=(remainder & 0xffff );
    }
    else
    {
        remainder=(remainder & 0xffff0000 )>>16;
    }
    *ans=(GT_U16)remainder;
}

/**
* @internal crcEM16 function
* @endinternal
*
* @brief   This function calculates CRC.
*
* @param[in]  message   - array of 8 bit words, contain data.
* @param[in]  nBits     - num of bits in the message
* @param[in]  poly      - the polynomial value .
* @param[in]  seed      - init value
* @param[out] ans       - the hash result.
*
*/

static void crcEm16
(
    IN GT_U8                   * message,
    IN GT_U32                    nBits,
    IN long long                 poly,
    IN GT_U16                    seed,
    OUT GT_U32                 * ans
)
{
    GT_U8 byteMsg,bit ;
    GT_U16  remainder = seed;
    GT_U32 nBytes ,byte;

    nBytes = nBits / 8 ;

    for (byte = 0; byte < nBytes; ++byte)
    {
        /*
         * Bring the next byte into the remainder.
         */
          byteMsg = message[nBytes-byte-1];
          remainder ^= (byteMsg << (CRC_16_WIDTH - 8));

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (bit = 8; bit > 0; --bit)
        {
            if (remainder & CRC_16_TOPBIT)
            {
                remainder = (GT_U16)((remainder << 1) ^ poly);
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }
    *ans=remainder;

}

/**
* @internal ExactMatchGlobalConfigurationGet function
* @endinternal
*
* @brief   get Exact match table size , number of banks and crc init vlaue
*
* @param[in]  devObjPtr            - (pointer to) the device object
* @param[out] exactMatchSize       - (pointer to) Exact Match total Size
* @param[out] exactMatchBanksNum   - (pointer to) Exact Match number of banks
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS ExactMatchGlobalConfigurationGet
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr,
    OUT SIP6_EXACT_MATCH_TABLE_SIZE_ENT      *exactMatchSize,
    OUT SIP6_EXACT_MATCH_MHT_ENT             *exactMatchBanksNum
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32  emNumEntries=0;
    GT_U32  emNumOfBanks=0;

    DECLARE_FUNC_NAME(ExactMatchGlobalConfigurationGet);
    __LOG(("start ExactMatchGlobalConfigurationGet  \n"));

    emNumEntries = devObjPtr->emNumEntries ;
    emNumOfBanks = devObjPtr->emNumOfBanks ;

    switch(emNumEntries)
    {
        case SMEM_MAC_TABLE_SIZE_4KB: *exactMatchSize=SIP6_EXACT_MATCH_TABLE_SIZE_4KB;
            break;
        case SMEM_MAC_TABLE_SIZE_8KB: *exactMatchSize=SIP6_EXACT_MATCH_TABLE_SIZE_8KB;
            break;
        case SMEM_MAC_TABLE_SIZE_16KB: *exactMatchSize=SIP6_EXACT_MATCH_TABLE_SIZE_16KB;
            break;
        case SMEM_MAC_TABLE_SIZE_32KB: *exactMatchSize=SIP6_EXACT_MATCH_TABLE_SIZE_32KB;
            break;
        case SMEM_MAC_TABLE_SIZE_64KB: *exactMatchSize=SIP6_EXACT_MATCH_TABLE_SIZE_64KB;
            break;
        case SMEM_MAC_TABLE_SIZE_128KB: *exactMatchSize=SIP6_EXACT_MATCH_TABLE_SIZE_128KB;
            break;
        case SMEM_MAC_TABLE_SIZE_256KB: *exactMatchSize=SIP6_EXACT_MATCH_TABLE_SIZE_256KB;
            break;
        case SMEM_MAC_TABLE_SIZE_512KB: *exactMatchSize=SIP6_EXACT_MATCH_TABLE_SIZE_512KB;
            break;
        default:
            rc = GT_FAIL ;
            break;
    }

    switch(emNumOfBanks)
    {
        case 4:  *exactMatchBanksNum=SIP6_EXACT_MATCH_MHT_4_E;
            break;
        case 8:  *exactMatchBanksNum=SIP6_EXACT_MATCH_MHT_8_E;
            break;
        case 16: *exactMatchBanksNum=SIP6_EXACT_MATCH_MHT_16_E;
            break;
    default:
            rc = GT_FAIL ;
            break;
    }
    __LOG(("ExactMatchGlobalConfigurationGet : exactMatchSize [%d] exactMatchBanksNum [%d]\n" ,
                *exactMatchSize,*exactMatchBanksNum));
    return rc;
}

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
    IN  SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN  GT_U32                              *entryKeyPtr,
    IN  SIP6_EXACT_MATCH_KEY_SIZE_ENT        keySize,
    OUT GT_U32                               crcMultiHashArr[],
    OUT GT_U32                              *numOfValidBank
)
{

    GT_STATUS   rc = GT_OK;
    GT_U8   cKey[SIP6_EXACT_MATCH_MAX_KEY_SIZE_BYTE_CNS]={0};
    GT_U32  numBitsInData= SIP6_EXACT_MATCH_MAX_KEY_SIZE_BIT_CNS ; /* num of bits in the data (fixed 376)*/
    GT_U32  numOfBanks,mask;
    GT_U32 initCrc = SIP6_EXACT_MATCH_CRC_INIT_VALUE_CNS ;

    GT_UL64 initCrc64;
    SIP6_EXACT_MATCH_TABLE_SIZE_ENT  exactMatchSize = 0; /*for system value*/
    SIP6_EXACT_MATCH_MHT_ENT         mth = 0;            /*for system value*/

    DECLARE_FUNC_NAME(smacHashExactMatchMultiHashResultsCalc);
    __LOG(("start smacHashExactMatchMultiHashResultsCalc  \n"));

    /*convert 32bit array into 8 bit array */
    convert(entryKeyPtr,&cKey[0]);

    ExactMatchGlobalConfigurationGet(devObjPtr,&exactMatchSize,&mth);

    crcEm16(cKey, numBitsInData, 0x18005,  (GT_U16) initCrc  , &crcMultiHashArr[0])  ;
    crcEm16(cKey, numBitsInData, 0x11021,  (GT_U16) initCrc  , &crcMultiHashArr[1])  ;
    crcEm16(cKey, numBitsInData, 0x18bb7,  (GT_U16) initCrc  , &crcMultiHashArr[2])  ;
    crcEm16(cKey, numBitsInData, 0x13d65,  (GT_U16) initCrc  , &crcMultiHashArr[3])  ;

    if (mth > SIP6_EXACT_MATCH_MHT_4_E )/*have more than 4 banks*/
    {
        crcEm32(cKey, numBitsInData, 0x104C11DB7ULL,  (GT_U32) initCrc    ,0, &crcMultiHashArr[4])  ;
        crcEm32(cKey, numBitsInData, 0x104C11DB7ULL,  (GT_U32) initCrc    ,1, &crcMultiHashArr[5])  ;
        crcEm32(cKey, numBitsInData, 0x11EDC6F41ULL,  (GT_U32) initCrc    ,0, &crcMultiHashArr[6])  ;
        crcEm32(cKey, numBitsInData, 0x11EDC6F41ULL,  (GT_U32) initCrc    ,1, &crcMultiHashArr[7])  ;


        if (mth > SIP6_EXACT_MATCH_MHT_8_E )/*have more than 8 banks*/
        {
            initCrc64 =  (GT_UL64) initCrc ;
            initCrc64 =  (initCrc64<<32) + (GT_UL64) initCrc ;

            crcEm32(cKey, numBitsInData, 0x1741B8CD7ULL,  (GT_U32) initCrc    ,0, &crcMultiHashArr[8])  ;
            crcEm32(cKey, numBitsInData, 0x1741B8CD7ULL,  (GT_U32) initCrc    ,1, &crcMultiHashArr[9])  ;
            crcEm32(cKey, numBitsInData, 0x1814141ABULL,  (GT_U32) initCrc    ,0, &crcMultiHashArr[10]) ;
            crcEm32(cKey, numBitsInData, 0x1814141ABULL,  (GT_U32) initCrc    ,1, &crcMultiHashArr[11]) ;
            crcEm64(cKey, numBitsInData, 0x42F0E1EBA9EA3693ULL, (GT_UL64) initCrc64 ,0, &crcMultiHashArr[12]) ;
            crcEm64(cKey, numBitsInData, 0x42F0E1EBA9EA3693ULL, (GT_UL64) initCrc64 ,1, &crcMultiHashArr[13]) ;
            crcEm64(cKey, numBitsInData, 0x42F0E1EBA9EA3693ULL, (GT_UL64) initCrc64 ,2, &crcMultiHashArr[14]) ;
            crcEm64(cKey, numBitsInData, 0x42F0E1EBA9EA3693ULL, (GT_UL64) initCrc64 ,3, &crcMultiHashArr[15]) ;
        }
    }

    __LOG(("smacHashExactMatchMultiHashResultsCalc: hash index befor Config : \n"
           "B0 0x%X B1 0x%X B2 0x%X B3 0x%X B4 0x%X B5 0x%X B6 0x%X B7 0x%X "
           "B8 0x%X B9 0x%X B10 0x%X B11 0x%X B12 0x%X B13 0x%X B14 0x%X B15 0x%X \n",
           crcMultiHashArr[0],crcMultiHashArr[1],crcMultiHashArr[2],crcMultiHashArr[3],crcMultiHashArr[4],
           crcMultiHashArr[5],crcMultiHashArr[6],crcMultiHashArr[7],crcMultiHashArr[8],crcMultiHashArr[9],
           crcMultiHashArr[10],crcMultiHashArr[11],crcMultiHashArr[12],crcMultiHashArr[13],crcMultiHashArr[14],
           crcMultiHashArr[15]));

    switch (mth)
    {
        case SIP6_EXACT_MATCH_MHT_4_E: numOfBanks = 4;
            break;
        case SIP6_EXACT_MATCH_MHT_8_E: numOfBanks = 8;
            break;
        case SIP6_EXACT_MATCH_MHT_16_E: numOfBanks = 16;
            break;
        default:
            return GT_FAIL;
    }

    mask=bitsTotake[mth][exactMatchSize];

    rc = hashConfig(keySize,mask,numOfBanks,&crcMultiHashArr[0]);
    if (rc != 0)
    {
        return GT_FAIL;
    }

    *numOfValidBank = numOfBanks;
    __LOG(("smacHashExactMatchMultiHashResultsCalc: hash index after Config : \n"
       "B0 0x%X B1 0x%X B2 0x%X B3 0x%X B4 0x%X B5 0x%X B6 0x%X B7 0x%X "
       "B8 0x%X B9 0x%X B10 0x%X B11 0x%X B12 0x%X B13 0x%X B14 0x%X B15 0x%X \n",
       crcMultiHashArr[0],crcMultiHashArr[1],crcMultiHashArr[2],crcMultiHashArr[3],crcMultiHashArr[4],
       crcMultiHashArr[5],crcMultiHashArr[6],crcMultiHashArr[7],crcMultiHashArr[8],crcMultiHashArr[9],
       crcMultiHashArr[10],crcMultiHashArr[11],crcMultiHashArr[12],crcMultiHashArr[13],crcMultiHashArr[14],
       crcMultiHashArr[15]));
    return rc;
}


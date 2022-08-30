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
* @file snetLion3Tcam.c
*
* @brief SIP5 Lion3 Tcam
*
* @version   16
********************************************************************************
*/

#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/suserframes/snetLion3Tcam.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypeTcam.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>

/* macro to return result of which bits caused the 'NO MATCH' */
#define X_Y_K_FIND_NO_MATCH(x,y,k,mask)  \
    ((~(((~x) & (~k)) | ((~y) & (k)))) & mask )

#define SIP5_10_HIT_SEGMENTS_IN_FLOOR_CNS   6 /* bc2 : 2 , bobk : 6 */
#define MAX_HIT_SEGMENTS_IN_FLOOR_CNS       SIP5_10_HIT_SEGMENTS_IN_FLOOR_CNS

/* max number of floors */
#define SIP5_TCAM_MAX_NUM_OF_FLOORS_CNS             16 /* 16 in Hawk */
/* size of segments info array :
    in sip5     : 2 segments per floor
    in sip 5_15 : 6 segments per floor
    in sip 5_20 : 2 segments per floor (like sip5)

*/
#define SIP5_TCAM_SEGMENTS_INFO_SIZE_CNS     (MAX_HIT_SEGMENTS_IN_FLOOR_CNS * SIP5_TCAM_MAX_NUM_OF_FLOORS_CNS)

#define NUM_HIT_SEGMENTS_IN_FLOOR_MAC(_devObjPtr)   (_devObjPtr->tcam_numBanksForHitNumGranularity)

/* number of words in key chunk */
#define SIP5_TCAM_NUM_OF_WORDS_IN_KEY_CHUNK_CNS     3

/* number of bits in key chunk */
#define SIP5_TCAM_NUM_OF_BITS_IN_KEY_CHUNK_CNS     (32 * \
                SIP5_TCAM_NUM_OF_WORDS_IN_KEY_CHUNK_CNS)

static GT_CHAR* tcamClientName[SIP5_TCAM_CLIENT_LAST_E + 1] =
{
    STR(SIP5_TCAM_CLIENT_TTI_E   ),
    STR(SIP5_TCAM_CLIENT_IPCL0_E ),
    STR(SIP5_TCAM_CLIENT_IPCL1_E ),
    STR(SIP5_TCAM_CLIENT_IPCL2_E ),
    STR(SIP5_TCAM_CLIENT_EPCL_E  ),

    NULL
};


/*
 * Typedef: struct SIP5_TCAM_SEGMENT_INFO_STC
 *
 * description :
 *          info about the segment in tcam to look into.
 *
 * Fields:
 *         startBankIndex : the index of the start bank (0/6)
 *         floorNum  : floor number (0..11)
 *         hitNum    : hit number   (0..3)
*/
typedef struct{
    GT_U32       startBankIndex;
    GT_U32       floorNum;
    GT_U32       hitNum;
} SIP5_TCAM_SEGMENT_INFO_STC;

#define MAX_HIT_NUM_CNS 4
/*
 * Typedef: struct TCAM_FLOOR_INFO_STC
 *
 * description :
 *          info about the floor.
 *
 * Fields:
 *         validBanksBmp : bmp of valid banks for the lookup
*/
typedef struct{
    GT_U32       validBanksBmp;
}TCAM_FLOOR_INFO_STC;

typedef struct{
    TCAM_FLOOR_INFO_STC floorInfo[SIP5_TCAM_MAX_NUM_OF_FLOORS_CNS];
}TCAM_FLOORS_INFO_STC;

/**
* @internal snetLion3TcamGetTcamGroupId function
* @endinternal
*
* @brief   sip5 function that returns tcam group id
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] tcamClient               - tcam client
*
* @retval status                   - GT_OK    - found,
* @retval GT_NOT_FOUND             - not found
*/
GT_STATUS snetLion3TcamGetTcamGroupId
(
    IN  SKERNEL_DEVICE_OBJECT  *devObjPtr,
    IN  SIP5_TCAM_CLIENT_ENT    tcamClient,
    OUT GT_U32                 *groupIdPtr
)
{
    GT_STATUS   st = GT_NOT_FOUND;
    GT_U32      isClientEnabled;
    GT_U32      enableCounter = 0;
    GT_U32      numClientIds;     /* Number of TCAM clients */
    GT_U32      hwClientId;       /* TCAM clients HW number */
    GT_U32      group;   /* 5 groups supported, 0..4 */
    GT_U32      regAddr; /* registry address */
    GT_U32      portGroupId = (devObjPtr->portGroupId & 0xf);/* serve 4 port groups */

    if(devObjPtr->numOfCoreDevs == 0)
    {
        portGroupId = smemGetCurrentPipeId(devObjPtr);
        if(devObjPtr->numOfTiles)
        {
            /* convert the global pipeId to local pipeId */
            portGroupId = smemConvertGlobalPipeIdToTileAndLocalPipeIdInTile(devObjPtr,
                portGroupId,NULL);
        }
    }

    numClientIds = devObjPtr->numofTcamClients;
    if(devObjPtr->isIpcl0NotValid)/* IPCL0 is bypassed */
    {
        /* convert TCAM client to HW client number */
        switch (tcamClient)
        {
            case SIP5_TCAM_CLIENT_TTI_E:
                hwClientId = 0;
                break;
            case SIP5_TCAM_CLIENT_IPCL1_E:
                hwClientId = 1;
                break;
            case SIP5_TCAM_CLIENT_IPCL2_E:
                hwClientId = 2;
                break;
            case SIP5_TCAM_CLIENT_EPCL_E:
                hwClientId = 3;
                break;
            case SIP5_TCAM_CLIENT_IPCL0_E:  /* IPCL0 is bypassed */
            default:
                skernelFatalError(
                  "snetLion3TcamGetTcamGroupId: wrong TCAM client [%d]\n", tcamClient);
                return st;
        }
    }
    else /*sip5*/
    {
        numClientIds = SIP5_TCAM_NUM_OF_GROUPS_CNS;
        hwClientId = (GT_U32)tcamClient;
    }
    /* get groupId */
    for(group = 0; group < SIP5_TCAM_NUM_OF_GROUPS_CNS; group++)
    {
        if(devObjPtr->tcamHardWiredInfo.isValid)
        {
            /* info is hardwired and is not read from the register */
            isClientEnabled = (hwClientId == devObjPtr->tcamHardWiredInfo.groupsArr[group]) ?
                1 : 0;
        }
        else
        {
            regAddr = SMEM_LION3_TCAM_GROUP_CLIENT_ENABLE_REG(devObjPtr, group);
            smemRegFldGet(devObjPtr, regAddr,
                          portGroupId*numClientIds + hwClientId,
                          1, &isClientEnabled);
        }

        if (isClientEnabled)
        {
            /* save port group id */
            *groupIdPtr = group;
            enableCounter++;
        }
    }

    switch(enableCounter)
    {
        case 0:
            break;
        case 1:
            st = GT_OK;
            break;
        default:
            skernelFatalError(
              "snetLion3TcamGetTcamGroupId: wrong number of groups id (groupId > 1)\n");
    }

    return st;
}

/**
* @internal snetLion3TcamSegmentsListGet function
* @endinternal
*
* @brief   sip5 function that gets tcam segments list
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] groupId                  - tcam group id
*
* @param[out] tcamSegmentsInfoArr      - array of segments info
* @param[out] tcamSegmentsNumPtr       - (pointer to) number of elements in tcamSegmentsInfoArr
*                                      COMMENTS:
*/
static GT_VOID snetLion3TcamSegmentsListGet
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  GT_U32                    groupId,
    OUT SIP5_TCAM_SEGMENT_INFO_STC *tcamSegmentsInfoArr,
    OUT GT_U32                   *tcamSegmentsNumPtr
)
{
    GT_U32   floorNum;
    GT_U32   regAddr; /* registry address */
    GT_U32   regValue;
    GT_U32   startBankIndex, index ,step , startBitHitNum;
    GT_U32   numOfActiveFloors;
    GT_U32   blockId;

    regAddr = SMEM_LION3_TCAM_POWER_ON_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddr, 0, 8, &numOfActiveFloors);

    if(numOfActiveFloors > devObjPtr->tcamNumOfFloors)
    {
        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("WARNING: (In Register 'tcamActiveFloors') [%d] active floors , but the device hold only [%d] floors \n",
            numOfActiveFloors,
            devObjPtr->tcamNumOfFloors));

        numOfActiveFloors = devObjPtr->tcamNumOfFloors;
    }
    else
    {
        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("TCAM : there are:[%d] active floors \n",
            numOfActiveFloors));
    }

    /* clear out values first */
    *tcamSegmentsNumPtr = 0;

    step = SIP5_TCAM_NUM_OF_BANKS_IN_FLOOR_CNS /
                NUM_HIT_SEGMENTS_IN_FLOOR_MAC(devObjPtr);

    startBitHitNum = SMEM_CHT_IS_SIP5_15_GET(devObjPtr) ? 18 : 6;

    /* get banks info */
    for(floorNum = 0; floorNum < numOfActiveFloors; floorNum++)
    {
        regAddr = SMEM_LION3_TCAM_HIT_NUM_AND_GROUP_SEL_FLOOR_REG(devObjPtr, floorNum);
        smemRegGet(devObjPtr, regAddr, &regValue);

        index = 0;

        for(startBankIndex = 0 ;
            startBankIndex < SIP5_TCAM_NUM_OF_BANKS_IN_FLOOR_CNS ;
            startBankIndex += step , index++)
        {
            blockId = SMEM_CHT_IS_SIP5_15_GET(devObjPtr) ?
                    (index * (6 / NUM_HIT_SEGMENTS_IN_FLOOR_MAC(devObjPtr))) :
                    index;

            if(SMEM_U32_GET_FIELD(regValue , blockId * 3 , 3) == groupId)
            {
                /* save floor, bank, hit num */
                tcamSegmentsInfoArr[*tcamSegmentsNumPtr].startBankIndex = startBankIndex;
                tcamSegmentsInfoArr[*tcamSegmentsNumPtr].floorNum = floorNum;
                tcamSegmentsInfoArr[*tcamSegmentsNumPtr].hitNum   = SMEM_U32_GET_FIELD(regValue , startBitHitNum + (blockId*2) , 2);
                (*tcamSegmentsNumPtr)++;
            }
        }
    }
}

/**
* @internal snetLion3TcamGetKeySizeBits function
* @endinternal
*
* @brief   sip5 function that returns size bits (4 bits),
*         these bits must be added to each chunk (bits 0..3)
* @param[in] keySize                  - key size
*                                       sizeBits value
*/
GT_U32 snetLion3TcamGetKeySizeBits
(
    IN  SIP5_TCAM_KEY_SIZE_ENT    keySize
)
{
    GT_U32 sizeBits = 0xFF;

    switch(keySize)
    {
        case SIP5_TCAM_KEY_SIZE_10B_E: sizeBits = 0; break;
        case SIP5_TCAM_KEY_SIZE_20B_E: sizeBits = 1; break;
        case SIP5_TCAM_KEY_SIZE_30B_E: sizeBits = 2; break;
        case SIP5_TCAM_KEY_SIZE_40B_E: sizeBits = 3; break;
        case SIP5_TCAM_KEY_SIZE_50B_E: sizeBits = 4; break;
        case SIP5_TCAM_KEY_SIZE_60B_E: sizeBits = 5; break;
        case SIP5_TCAM_KEY_SIZE_80B_E: sizeBits = 7; break;
        default: skernelFatalError("snetLion3TcamGetKeySizeBits: wrong key given %d\n", keySize);
    }

    return sizeBits;
}

/**
* @internal snetLion3TcamPrepareKeyChunksArray function
* @endinternal
*
* @brief   sip5 function that prepares key chunks (84 bits each chunk)
*
* @param[in] keyArrayPtr              - array of keys bytes
* @param[in] keySize                  - size of key array
*
* @param[out] chunksArrayPtr           - array of key chunks
*                                      COMMENTS:
*/
static GT_VOID snetLion3TcamPrepareKeyChunksArray
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  GT_U32                   *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT    keySize,
    OUT GT_U32                   *chunksArrayPtr
)
{
    GT_U32  chunkNum;
    GT_U32  keyWordsNum;
    GT_U32  sizeBits = snetLion3TcamGetKeySizeBits(keySize);
    GT_U32  valuesArr[SIP5_TCAM_NUM_OF_WORDS_IN_KEY_CHUNK_CNS]={0};

    /* dump the key that the client give */
    __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("dump key size [%d] bytes from client: \n",
        (keySize*10)));

    /* loop of the number of chunks that need to build*/
    for(chunkNum = 0; chunkNum < (GT_U32)keySize; chunkNum++)
    {
        /*each 84 bits start at new 96 bits */

        /* set the 4 bits of the 'sizeBits' as start of the 84 bits data */
        snetFieldValueSet(chunksArrayPtr,
            (chunkNum * SIP5_TCAM_NUM_OF_BITS_IN_KEY_CHUNK_CNS),
            4,
            sizeBits);

        /*continue the chunk with 10bytes (80 bits)*/

        /* first 64 bits */
        for(keyWordsNum = 0;
            keyWordsNum < SIP5_TCAM_NUM_OF_WORDS_IN_KEY_CHUNK_CNS - 1;/*words0,1*/
            keyWordsNum++)
        {
            valuesArr[keyWordsNum] = snetFieldValueGet(keyArrayPtr,(32*keyWordsNum) + (80*chunkNum),32);
            snetFieldValueSet(chunksArrayPtr,
                4 + (32*keyWordsNum) + (chunkNum * SIP5_TCAM_NUM_OF_BITS_IN_KEY_CHUNK_CNS),
                32,
                valuesArr[keyWordsNum]);
        }

        /* next 16 bits (total of 80 bits)*/
        keyWordsNum = SIP5_TCAM_NUM_OF_WORDS_IN_KEY_CHUNK_CNS - 1;/*word2*/
        valuesArr[keyWordsNum] = snetFieldValueGet(keyArrayPtr,(32*keyWordsNum) + (80*chunkNum),16);

        snetFieldValueSet(chunksArrayPtr,
            4 + (32*keyWordsNum) + (chunkNum * SIP5_TCAM_NUM_OF_BITS_IN_KEY_CHUNK_CNS),
            16,
            valuesArr[keyWordsNum]);

        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("0x%8.8x 0x%8.8x 0x%4.4x \n",
            valuesArr[0],valuesArr[1],valuesArr[2]));
    }
}


/**
* @internal snetLion3TcamCompareKeyChunk function
* @endinternal
*
* @brief   sip5 function that compares key chunk to tcam memory
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] keyChunksArrayPtr        - array of keys chunks
* @param[in] chunkIdx                 - index of chunk in the key
* @param[in] xdataPtr                 - pointer to x entry
* @param[in] ydataPtr                 - pointer to y entry
*
* @retval GT_BOOL                  - whether chunk found (GT_TRUE) or not (GT_FALSE)
*/
static GT_BOOL snetLion3TcamCompareKeyChunk
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GT_U32                      *keyChunksArrayPtr,
    IN  GT_U32                       chunkIdx,
    IN  GT_U32                      *xdataPtr,
    IN  GT_U32                      *ydataPtr
)
{
    GT_U32  index;                  /* word index */
    GT_U32  numWords = (84 / 32);   /* Number of words to be compared */
    GT_BOOL result;                 /* Compare result status */
    GT_U32  lastWordMask = SMEM_BIT_MASK((84%32));/* mask for remaining bits in last word */
    GT_U32  *keyPtr = &keyChunksArrayPtr[chunkIdx * SIP5_TCAM_NUM_OF_WORDS_IN_KEY_CHUNK_CNS];

    /* compare the first 2 full words */
    for (index = 0; index < numWords; index++)
    {
        result = CH3_TCAM_X_Y_K_MATCH(xdataPtr[index],ydataPtr[index],keyPtr[index],0xffffffff);
        if (result == 0)
        {
            return GT_FALSE;
        }
    }

    /* compare the next word , only 20 bits */
    result = CH3_TCAM_X_Y_K_MATCH(xdataPtr[index],ydataPtr[index],keyPtr[index],lastWordMask);
    if (result == 0)
    {
        return GT_FALSE;
    }

    return GT_TRUE;
}

/**
* @internal snetLion3TcamCheckIsHit function
* @endinternal
*
* @brief   sip5 function that does lookup on single segment (returns hit)
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] floorNum                 - floor number
* @param[in] validBanksBmp            - valid banks for lookup
* @param[in] validStartBanksBmp       - banks that lookup can start from
* @param[in] keyChunksArrayPtr        - array with key chunks
* @param[in] keySize                  - size of the key
*
* @retval GT_BOOL                  - whether hit found (GT_TRUE) or not (GT_FALSE)
*/
static GT_BOOL snetLion3TcamCheckIsHit
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GT_U32                       floorNum,
    IN  GT_U32                       validBanksBmp,
    IN  GT_U32                       validStartBanksBmp,
    IN  GT_U32                      *keyChunksArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT       keySize,
    OUT GT_U32                      *actionIndexPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TcamCheckIsHit);

    GT_BOOL matchFound = GT_FALSE;
    GT_U32  entryIdx;/*current entry index in the segment*/
    GT_U32  bankIdx; /*current bank index in the segment*/
    GT_U32  numOfBankHits;/*number of hits on consecutive banks */
    GT_U32  numOfBanksToHit;/*number of consecutive banks that must hit to achieve 'HIT' */
    GT_U32  bankIndexOfCurrSequenceBanks;/*bank index of current sequence banks that 'HIT' */
    GT_U32  actionIndex;/*index of the action*/
    GT_U32  globalXLineNumber = 0;/* global X line number */
    GT_U32  *xDataPtr, *yDataPtr;/*pointer to the memory of X and Y in current bank,line*/
    GT_U32  *base_xDataPtr, *base_yDataPtr;/*pointer to the start of the TCAM memory of X and Y*/
    GT_U32  numWordsOffsetBetweenXandYInTcam;/*num words Offset Between X and Y lines*/
    GT_U32  logIsOpen = simLogIsOpenFlag;
    GT_U32  cpssCurrentIndex = 0;/* the CPSS index of the current entry  . this for 'helpful' info to the LOG */
    GT_U32  cpssActionIndex = 0; /* the CPSS index of the current action . this for 'helpful' info to the LOG */
    GT_U32  keyStartIndex = 0;
    GT_U32  ii;
    GT_U32  globalBitIndexForLog;
    GT_U32  x,y,k;

    numOfBanksToHit = keySize;

    __LOG(("Start TCAM lookup on single segment : floorNum[%d] ,validBanksBmp[0x%x],validStartBanksBmp[0x%x],numOfBanksToHit[%d]  \n",
        floorNum , validBanksBmp,validStartBanksBmp , numOfBanksToHit));

    /*Entry Number = {floor_num[3:0], array_addr[7:0], bank[3:0], XY}*/
    SMEM_U32_SET_FIELD(globalXLineNumber,0, 1,0);/*XY*/
    SMEM_U32_SET_FIELD(globalXLineNumber,13,4,floorNum);/*floor_num[3:0]*/


    /*
        for 2 purposes access only once the memory to get the pointer
        1. to not record into LOG every access to the TCAM ... to reduce size of LOG
            ... to reduce time for LOG creation ... to reduce 'not helpful' info in the LOG
        2. to speed up performance.
    */

    /*  Get Tcam X data entry */
    base_xDataPtr = smemMemGet(devObjPtr, SMEM_LION3_TCAM_MEMORY_TBL_MEM(devObjPtr, 0));

    /*  Get Tcam Y data entry */
    base_yDataPtr = smemMemGet(devObjPtr, SMEM_LION3_TCAM_MEMORY_TBL_MEM(devObjPtr, 1));

    numWordsOffsetBetweenXandYInTcam = (base_yDataPtr - base_xDataPtr);

    for(entryIdx = 0 ; entryIdx < SIP5_TCAM_NUM_OF_X_LINES_IN_BANK_CNS ; entryIdx++)
    {
        /* for this line init that not started hits on the banks */
        numOfBankHits = 0;

        SMEM_U32_SET_FIELD(globalXLineNumber,5, 8,entryIdx);/*array_addr[7:0]*/

        bankIndexOfCurrSequenceBanks = 0;

        for(bankIdx = 0; bankIdx < SIP5_TCAM_NUM_OF_BANKS_IN_FLOOR_CNS; bankIdx++)
        {
            /* check if we are with 'no match' yet */
            if(numOfBankHits == 0)
            {
                /*jump to the next bank that may start sequence*/
                for(/*bankIdx*/;bankIdx < SIP5_TCAM_NUM_OF_BANKS_IN_FLOOR_CNS; bankIdx++)
                {
                    if(validStartBanksBmp & (1 << bankIdx))
                    {
                        /* found valid bank to start new sequence */
                        break;
                    }
                }

                if(bankIdx == SIP5_TCAM_NUM_OF_BANKS_IN_FLOOR_CNS)
                {
                    /* not found new bank to start lookup in current line */
                    break;
                }

                /* save the bank index for the next sequence */
                bankIndexOfCurrSequenceBanks = bankIdx;
            }

            /* check that the bank valid for the lookup */
            if(0 == (validBanksBmp & (1 << bankIdx)))
            {
                if(numOfBankHits)
                {
                    __LOG(("Warning : The non valid bankIdx[%d] broke the lookup sequence \n",bankIdx));
                }

                goto bankNoMatch_lbl;
            }

            SMEM_U32_SET_FIELD(globalXLineNumber,1, 4,bankIdx);/*bank[3:0]*/

            xDataPtr = base_xDataPtr + (numWordsOffsetBetweenXandYInTcam * globalXLineNumber);
            yDataPtr = xDataPtr + numWordsOffsetBetweenXandYInTcam;

            if(logIsOpen)
            {
                /*  For the Application the relation between floor_Id, entry_Id , bank_index and global_Index are:
                    cpssCurrentIndex = (floor_Id * (12*256)) + (entry_Id * 12) + bank_index
                    the actual index that the CPSS will write is:
                    hw_global_Index = (floor_Id * (16*256)) + (entry_Id * 16) + bank_index
                */
                cpssCurrentIndex = (floorNum*256*12) + (entryIdx * 12) + bankIdx;
            }


            if(GT_TRUE ==
               snetLion3TcamCompareKeyChunk(devObjPtr, keyChunksArrayPtr, numOfBankHits, xDataPtr, yDataPtr) )
            {
                /* match on current bank */
                numOfBankHits++;

                if(numOfBankHits == numOfBanksToHit)
                { /* found a match after all the needed consecutive banks matched */

                    /*action index is according to the bank <bankIndexOfCurrSequenceBanks> value */
                    actionIndex = globalXLineNumber;
                    SMEM_U32_SET_FIELD(actionIndex,1, 4, bankIndexOfCurrSequenceBanks);/*bank[3:0]*/
                    actionIndex >>= 1;/*action index is 1/2 from the corresponding X entry*/

                    *actionIndexPtr = actionIndex;

                    __LOG(("TCAM match found (found HIT): floor[%d] entryIdx[%d] bank[%d] -  actionIndex [0x%x]([%d]) \n",
                        floorNum , entryIdx , bankIndexOfCurrSequenceBanks ,
                        actionIndex, actionIndex));

                    cpssActionIndex = (floorNum*256*12) + (entryIdx * 12) + bankIndexOfCurrSequenceBanks;

                    __LOG(("NOTE: in terms of CPSS the cpssActionIndex [0x%x] ([%d]) \n",
                        cpssActionIndex,cpssActionIndex));


                    __LOG(("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n"));
                    matchFound = GT_TRUE;
                    goto exitCleanly_lbl;
                }
                else
                if (logIsOpen)
                {
                    __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("[%d] matched banks (out of [%d]) \n",
                        numOfBankHits,
                        numOfBanksToHit));
                }

                /* we have match on this bank but need more banks to hit ... */
                continue;
            }
            else
            if (logIsOpen)
            {
                if(numOfBankHits == 0)
                {
                    if(X_Y_K_FIND_NO_MATCH(xDataPtr[0],yDataPtr[0],keyChunksArrayPtr[0],0xF))
                    {
                        /* the first 4 bits are the size , we have not match on size */
                        /* we not want to give any log indication for those not valid / not same size entries */
                        /* condition changed as the test high_availability_vtcam_manager leave after
                           it huge amount of tcam entries of 'not valid' size 60 that cause any later 'tcam' test
                           to print huge amount of skipped entries in tcam */
                        goto bankNoMatch_lbl;
                    }
                }

                scibAccessLock();

                __LOG(("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n"));

                __LOG(("Floor[%d] entryIdx[%d] bank[%d] \n",
                    floorNum , entryIdx , bankIndexOfCurrSequenceBanks));

                __LOG(("NOTE: in terms of CPSS the 'no match' cpssCurrentIndex [0x%x]([%d]) \n",
                    cpssCurrentIndex,cpssCurrentIndex));

                /* need to explain why the entry not match */
                if(numOfBankHits)
                {
                    __LOG_NO_LOCATION_META_DATA(("the no match happen after [%d] matched banks (out of [%d]) \n",
                        numOfBankHits,
                        numOfBanksToHit));
                }

                __LOG_PARAM_NO_LOCATION_META_DATA(((globalXLineNumber/2)));

                keyStartIndex = SIP5_TCAM_NUM_OF_WORDS_IN_KEY_CHUNK_CNS * numOfBankHits;

                /* check the key size */
                x = snetFieldValueGet(xDataPtr,0,4);
                y = snetFieldValueGet(yDataPtr,0,4);
                k = snetFieldValueGet(keyChunksArrayPtr,(keyStartIndex*32)+0,4);

                if(X_Y_K_FIND_NO_MATCH(x,y,k,0xF))
                {
                    /* the 'size' do not match */
                    __LOG(("the 4 bits of the 'key size' do not match : X[0x%x],Y[0x%x],K[0x%x] \n",
                        x,y,k));
                }
                else
                {
                    __LOG_NO_LOCATION_META_DATA(("X,Y,K (84 bits include the 'key size'(0..3))- explain reason for no match in this entry: \n"));
                    __LOG_NO_LOCATION_META_DATA(("format:[31..0]  [63..32]   [83..64] \n"));
                    __LOG_NO_LOCATION_META_DATA(("X [%8.8x][%8.8x][%5.5x]\n",
                        xDataPtr[0],xDataPtr[1],xDataPtr[2]&0x000FFFFF));
                    __LOG_NO_LOCATION_META_DATA(("Y [%8.8x][%8.8x][%5.5x]\n",
                        yDataPtr[0],yDataPtr[1],yDataPtr[2]&0x000FFFFF));
                    __LOG_NO_LOCATION_META_DATA(("K [%8.8x][%8.8x][%5.5x]\n",
                        keyChunksArrayPtr[keyStartIndex+0],keyChunksArrayPtr[keyStartIndex+1],keyChunksArrayPtr[keyStartIndex+2]&0x000FFFFF));
                    /* print which bits caused the NO match */
                    __LOG_NO_LOCATION_META_DATA(("bits that caused no match [%8.8x][%8.8x][%5.5x] \n",
                        X_Y_K_FIND_NO_MATCH(xDataPtr[0],yDataPtr[0],keyChunksArrayPtr[keyStartIndex+0],0xFFFFFFFF),/*32 bits*/
                        X_Y_K_FIND_NO_MATCH(xDataPtr[1],yDataPtr[1],keyChunksArrayPtr[keyStartIndex+1],0xFFFFFFFF),/*32 bits*/
                        X_Y_K_FIND_NO_MATCH(xDataPtr[2],yDataPtr[2],keyChunksArrayPtr[keyStartIndex+2],0x000FFFFF) /*20 bits --> total of 32+32+20 = 84 */
                    ));

                    /* analyze the bits that are 'NO match' */
                    __LOG_NO_LOCATION_META_DATA(("analyze the GLOBAL bits that are 'NO match' \n"));
                    __LOG_NO_LOCATION_META_DATA(("the global index is in terms of the FS that describes the TTI/PCL key \n"));

                    /* the global index is in terms of the FS that describes the TTI/PCL key */
                    globalBitIndexForLog = (80*numOfBankHits);

                    __LOG_NO_LOCATION_META_DATA(("Bits:"));
                    /*start from bit 4 because the no match is not from there ! */
                    /*and the (globalBitIndexForLog++) must not be done for bits 0..3 ! */
                    for(ii = 4 ; ii < 84 ; ii++ , globalBitIndexForLog++)
                    {
                        x = snetFieldValueGet(xDataPtr,ii,1);
                        y = snetFieldValueGet(yDataPtr,ii,1);
                        k = snetFieldValueGet(keyChunksArrayPtr,(keyStartIndex*32)+ii,1);

                        if(X_Y_K_FIND_NO_MATCH(x,y,k,1))
                        {
                            __LOG_NO_LOCATION_META_DATA(("%d,",
                                globalBitIndexForLog));
                        }
                    }
                    __LOG_NO_LOCATION_META_DATA((". \n End of not matched Bits \n"));
                }

                __LOG(("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n"));
                scibAccessUnlock();

            }

bankNoMatch_lbl:
            /*reset the number of hits*/
            numOfBankHits = 0;
        }
    }

exitCleanly_lbl:

    __LOG(("Ended TCAM lookup on this single segment %s match \n",
        (matchFound == GT_TRUE ? "with" : "WITHOUT")));

    return matchFound;
}

/**
* @internal snetLion3TcamSegmentsListIsHit function
* @endinternal
*
* @brief   sip5 function that does lookup on segments in the list (returns hit)
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] hitNum                   - current hit number to check
* @param[in] tcamSegmentsInfoArr      - segments info
* @param[in] tcamSegmentsNum          - size of banks info array
* @param[in] keyChunksArrayPtr        - array with key chunks
* @param[in] keySize                  - size of the key
*
* @retval GT_BOOL                  - whether hit found (GT_TRUE) or not (GT_FALSE)
*/
static GT_BOOL snetLion3TcamSegmentsListIsHit
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GT_U32                       hitNum,
    IN  SIP5_TCAM_SEGMENT_INFO_STC  *tcamSegmentsInfoArr,
    IN  GT_U32                       tcamSegmentsNum,
    IN  GT_U32                      *keyChunksArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT       keySize,
    IN  GT_U32                      validBanksStartIndexBmp,
    OUT GT_U32                      *actionIndexPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TcamSegmentsListIsHit);

    TCAM_FLOORS_INFO_STC    floorsInfo;
    GT_U32  floorNum;
    GT_U32  ii;
    GT_U32  isFirstValidFloor = 1;
    GT_U32  numBanksInHitSegment;

    *actionIndexPtr = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;

    numBanksInHitSegment = SIP5_TCAM_NUM_OF_BANKS_IN_FLOOR_CNS / NUM_HIT_SEGMENTS_IN_FLOOR_MAC(devObjPtr);

    memset(&floorsInfo,0,sizeof(floorsInfo));
    /* build valid banks per floor */
    for(ii = 0 ; ii < tcamSegmentsNum ; ii++)
    {
        if(hitNum != tcamSegmentsInfoArr[ii].hitNum)
        {
            continue;
        }

        floorNum = tcamSegmentsInfoArr[ii].floorNum;

        floorsInfo.floorInfo[floorNum].validBanksBmp |=
            SMEM_BIT_MASK(numBanksInHitSegment)
                << tcamSegmentsInfoArr[ii].startBankIndex;
    }

    for(floorNum = 0 ; floorNum < SIP5_TCAM_MAX_NUM_OF_FLOORS_CNS ; floorNum ++)
    {
        if(floorsInfo.floorInfo[floorNum].validBanksBmp == 0)
        {
            /* the floor not valid or not relevant for current hitNum */
            continue;
        }

        if(isFirstValidFloor)
        {
            isFirstValidFloor = 0;

            /* print only if any of the floors bound to this hitNum */
            __LOG_PARAM(hitNum);
        }

        __LOG_PARAM(floorNum);
        __LOG_PARAM(floorsInfo.floorInfo[floorNum].validBanksBmp);

        if(GT_TRUE ==
            snetLion3TcamCheckIsHit(devObjPtr,
                    floorNum,
                    floorsInfo.floorInfo[floorNum].validBanksBmp,
                    validBanksStartIndexBmp,
                    keyChunksArrayPtr,
                    keySize,
                    actionIndexPtr))
        {
            /* found a match */
            return GT_TRUE;
        }
    }

    return GT_FALSE;
}

/**
* @internal tcamvalidBanksStartIndexBmpGet function
* @endinternal
*
* @brief   function that get valid bank indexes for start of lookup.
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] keySize                  - size of the key
*
* @param[out] validBanksStartIndexBmpPtr - (pointer to )valid bank indexes bitmap for start of lookup.
*                                       none.
*/
static void tcamvalidBanksStartIndexBmpGet
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT       keySize,

    OUT  GT_U32                      *validBanksStartIndexBmpPtr
)
{
    DECLARE_FUNC_NAME(tcamvalidBanksStartIndexBmpGet);

    GT_U32  validBanksStartIndexBmp;
    GT_U32  regValue;

    /*set validity checks and valid banks that must be power up*/
    switch(keySize)
    {
        case SIP5_TCAM_KEY_SIZE_80B_E:
            validBanksStartIndexBmp = 1<<0;
            break;
        case SIP5_TCAM_KEY_SIZE_60B_E:
        case SIP5_TCAM_KEY_SIZE_50B_E:
        case SIP5_TCAM_KEY_SIZE_40B_E:
            validBanksStartIndexBmp = 1<<0 | 1<<6;
            break;
        case SIP5_TCAM_KEY_SIZE_30B_E:
            validBanksStartIndexBmp = 1<<0 | 1<<3 | 1<<6 | 1<<9;
            break;
        case SIP5_TCAM_KEY_SIZE_20B_E:
            validBanksStartIndexBmp = 1<<0 | 1<<2 | 1<<4 | 1<<6 | 1<<8 | 1<<10;
            break;
        default:
        case SIP5_TCAM_KEY_SIZE_10B_E:
            validBanksStartIndexBmp = 0xFFF;
            break;
    }

    if ((SMEM_CHT_IS_SIP5_15_GET(devObjPtr) && (! SMEM_CHT_IS_SIP6_10_GET(devObjPtr)))
        && (keySize == SIP5_TCAM_KEY_SIZE_40B_E || keySize == SIP5_TCAM_KEY_SIZE_30B_E))
    {
        smemRegGet(devObjPtr, SMEM_LION3_TCAM_GLOBAL_REG(devObjPtr), &regValue);

        if (keySize == SIP5_TCAM_KEY_SIZE_40B_E)
        {
            /*
                Controls the key expend during 40-Byte Key.

                 0x0 = Mode0; Mode0; 40B key starts at Bank0, Bank6.; BC2 Compatible Mode
                 0x1 = Mode1; Mode1; 40B key starts at Bank0, Bank4, Bank8.; Better Utilization Mode
            */
            if(0 == SMEM_U32_GET_FIELD(regValue , 5 ,1))
            {
                __LOG(("40-Byte Key : mode : 40B key starts at Bank0, Bank6.; BC2 Compatible Mode"));
            }
            else
            {
                __LOG(("40-Byte Key : mode : 40B key starts at Bank0, Bank4, Bank8.; Better Utilization Mode"));

                validBanksStartIndexBmp = 1<<0 | 1<<4 | 1<<8;
            }
        }
        else
        {
            /*
                Controls the key expend during 30-Byte Key.

                 0x0 = Mode0; Mode0; 30B key starts at Bank0, Bank3, Bank6, Bank9.
                 0x1 = Mode1; Mode1; 30B key starts at Bank0, Bank4, Bank8.
            */
            if(0 == SMEM_U32_GET_FIELD(regValue , 4 ,1))
            {
                __LOG(("30-Byte Key : mode : 30B key starts at Bank0, Bank3, Bank6, Bank9"));
            }
            else
            {
                __LOG(("30-Byte Key : mode : 30B key starts at Bank0, Bank4, Bank8"));

                validBanksStartIndexBmp = 1<<0 | 1<<4 | 1<<8;
            }
        }
    }

    if (SMEM_CHT_IS_SIP6_10_GET(devObjPtr) && (keySize == SIP5_TCAM_KEY_SIZE_40B_E))
    {
        __LOG(("40-Byte Key : mode : 40B key starts at Bank0, Bank4, Bank8.; Better Utilization Mode"));
        validBanksStartIndexBmp = 1<<0 | 1<<4 | 1<<8;
    }


    *validBanksStartIndexBmpPtr = validBanksStartIndexBmp;

    return;
}

/**
* @internal snetLion3TcamDoLookup function
* @endinternal
*
* @brief   sip5 function that does sequential lookup on all banks
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] tcamSegmentsInfoArr      - segments info
* @param[in] tcamSegmentsNum          - size of banks info array
* @param[in] keyChunksArrayPtrArr     - Array of pointers to per hit arrays with key chunks
* @param[in] keySizeArray             - Array of sizes of the keys per hit
*                                       number of results
*/
static GT_U32 snetLion3TcamDoLookup
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  SIP5_TCAM_SEGMENT_INFO_STC  *tcamSegmentsInfoArr,
    IN  GT_U32                       tcamSegmentsNum,
    IN  GT_U32                       *keyChunksArrayPtrArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS],
    IN  SIP5_TCAM_KEY_SIZE_ENT       keySizeArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS],
    OUT GT_U32                       resultArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS]
)
{
    DECLARE_FUNC_NAME(snetLion3TcamDoLookup);

    GT_BOOL gotHit = GT_FALSE;
    GT_U32  hitNum;
    GT_U32  matchIdx;
    GT_U32  hitsCounter = 0;
    GT_U32  validBanksStartIndexBmpArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS];

    /* get valid bmp of banks that can start lookup for the 'keySize' */
    for(hitNum = 0; hitNum < SIP5_TCAM_MAX_NUM_OF_HITS_CNS; hitNum++)
    {
        if(devObjPtr->limitedNumOfParrallelLookups &&
           hitNum >= devObjPtr->limitedNumOfParrallelLookups)
        {
            break;
        }

        tcamvalidBanksStartIndexBmpGet(
            devObjPtr, keySizeArr[hitNum], &(validBanksStartIndexBmpArr[hitNum]));
        __LOG(("hitNum [%d]: validBanksStartIndexBmpArr [0x%X] \n", hitNum, validBanksStartIndexBmpArr[hitNum]));
    }

    for(hitNum = 0; hitNum < SIP5_TCAM_MAX_NUM_OF_HITS_CNS; hitNum++)
    {
        if(devObjPtr->limitedNumOfParrallelLookups &&
           hitNum >= devObjPtr->limitedNumOfParrallelLookups)
        {
            break;
        }

        __LOG(("start lookup for hitNum [%d] keySize [%d]\n", hitNum, (keySizeArr[hitNum] * 10)));

        gotHit = snetLion3TcamSegmentsListIsHit(
            devObjPtr, hitNum, tcamSegmentsInfoArr, tcamSegmentsNum,
            keyChunksArrayPtrArr[hitNum], keySizeArr[hitNum],
            validBanksStartIndexBmpArr[hitNum], &matchIdx);
        if(gotHit)
        {
            resultArr[hitNum] = matchIdx;
            hitsCounter++;
            __LOG(("for hitNum [%d]: got hit: matchIdx [%d] \n", hitNum,matchIdx));
        }
        else
        {
            /*init for the caller that no match for this lookup*/
            resultArr[hitNum] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
            __LOG(("for hitNum [%d]: NO hit \n", hitNum));
        }
    }

    return hitsCounter;
}

/**
* @internal sip5TcamConvertPclKeyFormatToKeySize function
* @endinternal
*
* @brief   sip5 function that do convertation of old tcam key format to sip5 key size
*
* @param[in] keyFormat                - format of the key
*
* @param[out] sip5KeySizePtr           - sip5 key size
*                                      COMMENTS:
*/
GT_VOID sip5TcamConvertPclKeyFormatToKeySize
(
    IN  CHT_PCL_KEY_FORMAT_ENT    keyFormat,
    OUT SIP5_TCAM_KEY_SIZE_ENT   *sip5KeySizePtr
)
{
    switch(keyFormat)
    {
        case CHT_PCL_KEY_10B_E:
            *sip5KeySizePtr = SIP5_TCAM_KEY_SIZE_10B_E;
            break;

        case CHT_PCL_KEY_20B_E:
            *sip5KeySizePtr = SIP5_TCAM_KEY_SIZE_20B_E;
            break;

        case CHT_PCL_KEY_REGULAR_E:
        case CHT_PCL_KEY_30B_E:
            *sip5KeySizePtr = SIP5_TCAM_KEY_SIZE_30B_E;
            break;

        case CHT_PCL_KEY_40B_E:
            *sip5KeySizePtr = SIP5_TCAM_KEY_SIZE_40B_E;
            break;

        case CHT_PCL_KEY_50B_E:
            *sip5KeySizePtr = SIP5_TCAM_KEY_SIZE_50B_E;
            break;

        case CHT_PCL_KEY_EXTENDED_E:
        case CHT_PCL_KEY_60B_E:
        case CHT_PCL_KEY_60B_NO_FIXED_FIELDS_E:
            *sip5KeySizePtr = SIP5_TCAM_KEY_SIZE_60B_E;
            break;

        case CHT_PCL_KEY_TRIPLE_E:
        case CHT_PCL_KEY_80B_E:
            *sip5KeySizePtr = SIP5_TCAM_KEY_SIZE_80B_E;
            break;

        case CHT_PCL_KEY_UNLIMITED_SIZE_E:
        default:
            skernelFatalError("Wrong key format given\n");
    }
}

/**
* @internal sip5TcamLookupByGroupId function
* @endinternal
*
* @brief   sip5 function that do lookup in tcam for given key and fill the
*         results array
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] groupId                  - tcam client group Id
* @param[in] tcamProfileId            - tcam profile Id (relevant to SIP6_10 devices)
* @param[in] keyArrayPtr              - key array (size up to 80 bytes)
* @param[in] keySize                  - size of the key
*                                       number of hits found
*/
GT_U32 sip5TcamLookupByGroupId
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  GT_U32                    groupId,
    IN  GT_U32                    tcamProfileId,
    IN  GT_U32                   *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT    keySize,
    OUT GT_U32                    resultArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS]
)
{
    DECLARE_FUNC_NAME(sip5TcamLookupByGroupId);

    GT_U32      tcamSegmentsNum;/* number of tcam segments (half floors) */
    GT_U32      numOfHits;
    GT_U32      workU32keyArray[20];
    GT_U32      keyChunksArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS][8/*chunks*/ * SIP5_TCAM_NUM_OF_WORDS_IN_KEY_CHUNK_CNS];
    SIP5_TCAM_SEGMENT_INFO_STC tcamSegmentsInfoArr[SIP5_TCAM_SEGMENTS_INFO_SIZE_CNS];
    GT_U32      ii, jj, kk;
    GT_U32      *keyChunksArrPtrArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS];
    SIP5_TCAM_KEY_SIZE_ENT    keySizeArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS];
    GT_U32      index;
    GT_U32      regAddr;
    GT_U32      regVal;
    GT_U32      subKeySize;
    GT_U32      muxTableLine;
    GT_U32      muxTableOffset;
    GT_U32      muxBytePairMap;
    GT_U32      muxBytePairIndex;
    GT_U32      muxBytePairVal;
    GT_U32      muxedKeySize;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TCAM_E);

    /* get segments array info */
    snetLion3TcamSegmentsListGet(devObjPtr, groupId, tcamSegmentsInfoArr, &tcamSegmentsNum);

    if( 0 == tcamSegmentsNum )
    {
        __LOG(("groupId [%d]: there are no relevant banks found \n",
            groupId));
        numOfHits = 0;
        resultArr[0] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
        resultArr[1] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
        resultArr[2] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
        resultArr[3] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
        goto exitCleanly_lbl;
    }
    else
    {
        __LOG(("groupId[%d]: will look in next 'start bank indexes' : \n"
            "index  | floorNum    |    startBankIndex |  hitNum \n"
            "-------------------------------------------------- \n",
            groupId));

        for(ii = 0 ; ii < tcamSegmentsNum ; ii++ )
        {
            __LOG(("%2.2d   |  %2.2d      |       %2.2d      |     %d \n",
                         ii     ,
                         tcamSegmentsInfoArr[ii].floorNum,
                         tcamSegmentsInfoArr[ii].startBankIndex,
                         tcamSegmentsInfoArr[ii].hitNum));
        }
    }

    if ((! SMEM_CHT_IS_SIP6_10_GET(devObjPtr)) || (tcamProfileId == 0))
    {
        /*default - for SIP6_10 devices when tcamProfileId != 0 rewritten */
        /* build the array of keys + 'key size' per block */
        snetLion3TcamPrepareKeyChunksArray(devObjPtr, keyArrayPtr, keySize, keyChunksArr[0]);
        for (ii = 0; (ii < SIP5_TCAM_MAX_NUM_OF_HITS_CNS); ii++)
        {
            if(devObjPtr->limitedNumOfParrallelLookups &&
               ii >= devObjPtr->limitedNumOfParrallelLookups)
            {
                break;
            }

            keyChunksArrPtrArr[ii] = keyChunksArr[0];
            keySizeArr[ii]         = keySize;
        }
    }
    else
    {
        for (ii = 0; (ii < SIP5_TCAM_MAX_NUM_OF_HITS_CNS); ii++)
        {
            if(devObjPtr->limitedNumOfParrallelLookups &&
               ii >= devObjPtr->limitedNumOfParrallelLookups)
            {
                break;
            }

            index = ((tcamProfileId - 1) * 4) + ii;
            regAddr = SMEM_TABLE_ENTRY_INDEX_GET_MAC(devObjPtr, tcamProfileSubkeySizeAndMux, index);
            smemRegGet(devObjPtr, regAddr, &regVal);
            subKeySize      = ((regVal & 0x7) + SIP5_TCAM_KEY_SIZE_10B_E);
            muxTableLine    = (regVal >> 3) & 0x3F; /* index of mux table line plus one */
            muxTableOffset  = (regVal >> 9) & 0x7;
            keySizeArr[ii]  = subKeySize; /* amount of 10-byte parts minus one */
            if ((ii == 0) || (muxTableLine == 0))
            {
                /* for 0-th parallel sublookup no muxing */
                /* if muxTableLine is 0 no muxing        */
                snetLion3TcamPrepareKeyChunksArray(devObjPtr, keyArrayPtr, keySizeArr[ii], keyChunksArr[ii]);
                keyChunksArrPtrArr[ii] = keyChunksArr[ii];
                continue;
            }
            /* byte pairs muxing  keyArrayPtr ==> workU32keyArray */
            /* 60 bytes HW key size value is 5 */
            for (kk = 0; (kk < 20); kk++)
            {
                workU32keyArray[kk] = 0;
            }
            muxedKeySize = (subKeySize <= 5) ? subKeySize : 5;
            for (jj = 0; (jj <= muxedKeySize); jj++)
            {
                index = ((muxTableLine - 1) * 6) + muxTableOffset + jj;
                regAddr = SMEM_TABLE_ENTRY_INDEX_GET_MAC(devObjPtr, tcamSubkeyMux2byteUnits, index);
                smemRegGet(devObjPtr, regAddr, &regVal);
                for (kk = 0; (kk < 5); kk++)
                {
                    muxBytePairMap   = (regVal >> (6 * kk)) & 0x3F;
                    muxBytePairIndex = (jj * 5) + kk;
                    muxBytePairVal = (keyArrayPtr[muxBytePairMap / 2] >> ((muxBytePairMap % 2) * 16)) & 0xFFFF;
                    workU32keyArray[muxBytePairIndex / 2] |= muxBytePairVal << ((muxBytePairIndex % 2) * 16);
                }
            }
            /* copy fixed fields - needed when size > 60 bytes */
            for (kk = 15; (kk < 20); kk++)
            {
                workU32keyArray[kk] = keyArrayPtr[kk];
            }
            snetLion3TcamPrepareKeyChunksArray(devObjPtr, workU32keyArray, keySizeArr[ii], keyChunksArr[ii]);
            keyChunksArrPtrArr[ii] = keyChunksArr[ii];
        }
    }

    /* sequential lookup on all segments */
    numOfHits = snetLion3TcamDoLookup(
        devObjPtr, tcamSegmentsInfoArr, tcamSegmentsNum,
        keyChunksArrPtrArr, keySizeArr, resultArr);

    if(devObjPtr->limitedNumOfParrallelLookups)
    {
       __LOG_PARAM(resultArr[0]);
       if(devObjPtr->limitedNumOfParrallelLookups > 1)
       {
           __LOG_PARAM(resultArr[1]);
           if(devObjPtr->limitedNumOfParrallelLookups > 2)
           {
               __LOG_PARAM(resultArr[2]);
               if(devObjPtr->limitedNumOfParrallelLookups > 3)
               {
                   __LOG_PARAM(resultArr[3]);
               }
           }
       }
    }
    else
    {
       __LOG_PARAM(resultArr[0]);
       __LOG_PARAM(resultArr[1]);
       __LOG_PARAM(resultArr[2]);
       __LOG_PARAM(resultArr[3]);
    }

exitCleanly_lbl:
    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___RESTORE_PREVIOUS_UNIT___E);

    return numOfHits;
}

/**
* @internal sip5TcamLookup function
* @endinternal
*
* @brief   sip5 function that do lookup in tcam for given key and fill the
*         results array
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] tcamClient               - tcam client
* @param[in] tcamProfileId            - tcam profile Id (relevant to SIP6_10 devices)
* @param[in] keyArrayPtr              - key array (size up to 80 bytes)
* @param[in] keySize                  - size of the key
*                                       number of hits found
*/
GT_U32 sip5TcamLookup
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  SIP5_TCAM_CLIENT_ENT      tcamClient,
    IN  GT_U32                    tcamProfileId,
    IN  GT_U32                   *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT    keySize,
    OUT GT_U32                    resultArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS]
)
{
    DECLARE_FUNC_NAME(sip5TcamLookup);

    GT_STATUS   st; /* return status */
    GT_U32      groupId = 0;
    GT_U32      numOfHits;
    GT_CHAR*    clientNamePtr = ((tcamClient < SIP5_TCAM_CLIENT_LAST_E) ? tcamClientName[tcamClient] : "unknown");

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TCAM_E);

    __LOG(("Start Tcam check for [%s] \n",
        clientNamePtr));

    /* get group id */
    st = snetLion3TcamGetTcamGroupId(devObjPtr, tcamClient, &groupId);
    if(GT_OK != st)
    {
        __LOG(("client [%s] disabled",
            clientNamePtr));
        numOfHits = 0;
        simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___RESTORE_PREVIOUS_UNIT___E);
    }
    else
    {
        numOfHits = sip5TcamLookupByGroupId(devObjPtr, groupId, tcamProfileId, keyArrayPtr, keySize, resultArr);
    }

    return numOfHits;
}

/**
* @internal snetSip5PclTcamLookup function
* @endinternal
*
* @brief   sip5 function that do pcl lookup in tcam
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] iPclTcamClient           - tcam client
* @param[in] u32keyArrayPtr           key array (GT_U32)
* @param[in] keyFormat                - format of the key
*/
GT_VOID snetSip5PclTcamLookup
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  SIP5_TCAM_CLIENT_ENT      iPclTcamClient,
    IN  GT_U32                   *u32keyArrayPtr,
    IN  CHT_PCL_KEY_FORMAT_ENT    keyFormat,
    OUT GT_U32                   *matchIndexPtr
)
{
    SIP5_TCAM_KEY_SIZE_ENT   sip5KeySize;
    GT_U32                   resultArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS];
    GT_U32                   ii;
    GT_U32                   numOfHits;

    /* convert old key format to new key size */
    sip5TcamConvertPclKeyFormatToKeySize(keyFormat, &sip5KeySize);

    /* search the key */
    numOfHits = sip5TcamLookup(
        devObjPtr, iPclTcamClient,
        0/*tcamProfileId*/, u32keyArrayPtr, sip5KeySize, resultArr);

    if (numOfHits)
    {
        for (ii = 0; ii < SIP5_TCAM_MAX_NUM_OF_HITS_CNS; ii++)
        {
            if(devObjPtr->limitedNumOfParrallelLookups &&
               ii >= devObjPtr->limitedNumOfParrallelLookups)
            {
                break;
            }
           matchIndexPtr[ii] = resultArr[ii];
        }
    }
}



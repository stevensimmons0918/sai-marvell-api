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
* @file snetHawkExactMatch.c
*
* @brief SIP6_10 Hawk Exact Match
*
* @version   1
********************************************************************************
*/

#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>
#include <asicSimulation/SLog/simLogInfoTypeDevice.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahPcl.h>
#include <asicSimulation/SKernel/suserframes/snetFalconExactMatch.h>
#include <asicSimulation/SKernel/suserframes/snetHawkExactMatch.h>
#include <asicSimulation/SKernel/cheetahCommon/sregFalcon.h>
#include <asicSimulation/SKernel/suserframes/snetLion3Tcam.h>
#include <common/Utils/PresteraHash/smacHashExactMatch.h>
#include <asicSimulation/SKernel/cheetahCommon/sregHawk.h>


/**
* @internal sip6_10_ExactMatchAutoLearnHitNumEnable function
* @endinternal
*
* @brief   return GT_TRUE incase the hitNum is enabled for Auto Learn
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] hitNum                     - hit number (0,1)
*/
GT_BOOL sip6_10_ExactMatchAutoLearnHitNumEnable
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN  GT_U32                                hitNum
)
{
    GT_U32 regAddr , lookupNum =0 ;

    DECLARE_FUNC_NAME(sip6_10_ExactMatchAutoLearnHitNumEnable);
    __LOG(("start sip6_10_ExactMatchAutoLearnHitNumEnable  \n"));

    regAddr = SMEM_FALCON_EXACT_MATCH_GLOBAL_CONFIGURATION_REG(devObjPtr,emGlobalConfiguration1);
    smemRegFldGet(devObjPtr, regAddr, 6, 1, &lookupNum);

    if (hitNum==lookupNum)
    {
        return GT_TRUE;
    }
    else
    {
        return GT_FALSE;
    }
}
/**
* @internal sip6_10_ExactMatchAutoLearnStatusCountersEnableGet function
* @endinternal
*
* @brief   return GT_TRUE if EM status counters are enabled else GT_FALSE
*
* @param[in] devObjPtr                  - (pointer to) the device object
*
*/
GT_BOOL sip6_10_ExactMatchAutoLearnStatusCountersEnableGet
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr
)
{
    GT_U32 regAddr, statusCountersEnable;

    DECLARE_FUNC_NAME(sip6_10_ExactMatchAutoLearnStatusCountersEnableGet);
    __LOG(("start sip6_10_ExactMatchAutoLearnStatusCountersEnableGet  \n"));

    regAddr = SMEM_FALCON_EXACT_MATCH_GLOBAL_CONFIGURATION_REG(devObjPtr,emGlobalConfiguration1);
    smemRegFldGet(devObjPtr, regAddr, 7, 1, &statusCountersEnable);

    if (statusCountersEnable==1)
    {
         return GT_TRUE;
    }

    return GT_FALSE;
}
/**
* @internal sip6_10_ExactMatchAutoLearnProfileIndexEnable function
* @endinternal
*
* @brief   return GT_TRUE incase the profileIndex is enabled for Auto Learn
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] exactMatchProfileIndex     - profile index (0,15)
*/
GT_BOOL sip6_10_ExactMatchAutoLearnProfileIndexEnable
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN  GT_U32                                exactMatchProfileIndex
)
{
    GT_U32 regAddr;
    GT_U32 autoLearnEn;


    DECLARE_FUNC_NAME(sip6_10_ExactMatchAutoLearnProfileIndexEnable);
    __LOG(("start sip6_10_ExactMatchAutoLearnProfileIndexEnable  \n"));

    if (exactMatchProfileIndex==0)/* profile is disabled for Exact Match */
    {
        return GT_FALSE;
    }
    regAddr = SMEM_SIP6_10_EXACT_MATCH_AUTO_LEARN_PROFILE_CONFIG_1_REG(devObjPtr, exactMatchProfileIndex);
    smemRegFldGet(devObjPtr, regAddr, 0, 1, &autoLearnEn);

    if (autoLearnEn==1)
    {
        return GT_TRUE;
    }
    else
    {
        return GT_FALSE;
    }
}

/**
* @internal sip6_10_ExactMatchAutoLearnProfileFlowIdBitOffset function
* @endinternal
*
* @brief   return the flowId bit offset for the given profileIndex
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] exactMatchProfileIndex     - profile index (0,15)
*/
GT_U32 sip6_10_ExactMatchAutoLearnProfileFlowIdBitOffset
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN  GT_U32                                exactMatchProfileIndex
)
{
    GT_U32 regAddr;
    GT_U32 flowIdBitOffset=0;


    DECLARE_FUNC_NAME(sip6_10_ExactMatchAutoLearnProfileFlowIdBitOffset);
    __LOG(("start sip6_10_ExactMatchAutoLearnProfileFlowIdBitOffset  \n"));

    regAddr = SMEM_SIP6_10_EXACT_MATCH_AUTO_LEARN_PROFILE_CONFIG_1_REG(devObjPtr, exactMatchProfileIndex);
    smemRegFldGet(devObjPtr, regAddr, 1, 6, &flowIdBitOffset);

    return flowIdBitOffset;
}


/**
* @internal sip6_10_ExactMatchFreeIndex function
* @endinternal
*
* @brief   return GT_TRUE incase there is a free index in the
*          Exact match table that can fit the given key
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] hitNum                     - hit number (0,1)
* @param[in] exactMatchNewKeyDataPtr    - EM-Key data
* @param[in] exactMatchNewKeySizePtr    - EM-Key size
* @param[out] freeIndexPtr              - pointer to free Index
* @param[out] bankNumPtr                - pointer to bank of the free Index
* @param[out] numberOfBanksPtr          - the number of valid banks in the system
*/
GT_BOOL sip6_10_ExactMatchFreeIndex
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN  GT_U32                                hitNum,
    IN  GT_U32                               *exactMatchNewKeyDataPtr,
    IN  SIP6_EXACT_MATCH_KEY_SIZE_ENT         exactMatchNewKeySize,
    OUT GT_U32                               *freeIndexPtr,
    OUT GT_U32                               *bankNumPtr,
    OUT GT_U32                               *numberOfBanksPtr
)
{
    GT_U32 numOfValidBanks = 0;
    GT_U32 multiHashIndexArr[SIP6_EXACT_MATCH_MAX_NUM_BANKS_CNS] = {0};
    GT_U32 bank=0 ,relativeBank=0;
    GT_U32 regAddr;
    GT_U32 * entryPtr;                  /* Exact Match table entry pointer */
    SNET_SIP6_EXACT_MATCH_ENTRY_INFO exactMatchEntryInfo;
    GT_U32 n=0;
    GT_U32 indexFailCounter=0;
    GT_U32 collisionStatusValid;

    DECLARE_FUNC_NAME(sip6_10_ExactMatchFreeIndex);
    __LOG(("start sip6_10_ExactMatchFreeIndex  \n"));

    smacHashExactMatchMultiHashResultsCalc(devObjPtr,exactMatchNewKeyDataPtr,exactMatchNewKeySize,&multiHashIndexArr[0],&numOfValidBanks);

    *numberOfBanksPtr = numOfValidBanks;

    for (bank = 0;bank < numOfValidBanks ;bank+=(exactMatchNewKeySize+1) )
    {
        for ( relativeBank = 0 ;((relativeBank <= (GT_U32)exactMatchNewKeySize)&&((bank+relativeBank)<numOfValidBanks)) ;relativeBank ++ )
        {
            memset(&exactMatchEntryInfo, 0, sizeof(SNET_SIP6_EXACT_MATCH_ENTRY_INFO));
            /* Get entryPtr according to entry index */
            entryPtr = SMEM_SIP6_HIDDEN_EXACT_MATCH_PTR(devObjPtr, multiHashIndexArr[bank+relativeBank]);
            snetFalconExactMatchParseEntry(devObjPtr,entryPtr,multiHashIndexArr[bank+relativeBank],&exactMatchEntryInfo);

            if (exactMatchEntryInfo.valid) /* valid entry is not free */
            {
                __LOG(("sip6_10_ExactMatchFreeIndex :no free bank index [0x%x] \n",multiHashIndexArr[bank+relativeBank]));
                break;
            }

            if (relativeBank ==(GT_U32) exactMatchNewKeySize) /*freeIndex is continuous GT_TRUE in all checked bank */
            {
                *freeIndexPtr=multiHashIndexArr[bank];
                *bankNumPtr = bank;

                __LOG(("sip6_10_ExactMatchFreeIndex :free bank index [0x%x] \n",
                       multiHashIndexArr[bank]));
                return GT_TRUE;
            }
        }/*end for ralativeBank loop*/
    }/*end for bank loop*/

    /* Generate interrupt :  Auto Learning Collision bit 6
    Indicates that a flow auto learning failed due to a hash collision.*/
    __LOG(("sip6_10_ExactMatchFreeIndex : Generate interrupt :  Auto Learning Collision \n"));
    snetChetahDoInterrupt(devObjPtr,
              SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_INTERRUPT_CAUSE_REGISTER_REG(devObjPtr),
              SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_INTERRUPT_MASK_REGISTER_REG(devObjPtr),
              (1<<6),/*  Auto Learning Collision */
              0/*not used in sip5*/);

    /* set collision status */
    regAddr = SMEM_SIP6_10_AUTO_LEARN_COLLISION_STATUS_1_REGISTER_REG(devObjPtr);

    /* set Collision Status Valid
       This field protects collision status registers data integrity.
       Once EM updates the collision status registers, it sets this bit to Valid.
       After SW reads all these registers, it should un-set this field and a new data can be latched.*/
    smemRegFldGet(devObjPtr, regAddr, 0, 1, &collisionStatusValid);

    if(collisionStatusValid == 0)
    {
        /* new data can be latched */
        smemRegFldSet(devObjPtr, regAddr, 0, 1, 1);/* set collision Status to valid*/

        /* set Collision Entry Lookup Number
           Indicates the lookup number of the entry that the EM engine can’t learn
           because of Hash collision*/
        smemRegFldSet(devObjPtr, regAddr, 1, 1, hitNum);

        /* set Collision entry key size
           Indicates the size of the key of the entry that the EM engine can’t learn
           because of Hash collision.*/
        smemRegFldSet(devObjPtr, regAddr, 2, 2, exactMatchNewKeySize);

        /* set Collision entry key
           The key that the EM engine can’t learn because of Hash collision.
           The size of the key actually used depends on the <Key Size> field.*/
        for (n=0; n<SIP6_EXACT_MATCH_MAX_KEY_SIZE_WORD_CNS; n++)
        {
            /* set collision status n */
            regAddr = SMEM_SIP6_10_AUTO_LEARN_COLLISION_STATUS_N_REGISTER_REG(devObjPtr,n);
            smemRegFldSet(devObjPtr, regAddr, 0, 32, exactMatchNewKeyDataPtr[n]);
        }
    }

    if (sip6_10_ExactMatchAutoLearnStatusCountersEnableGet(devObjPtr)==GT_TRUE)
    {
        /* update fail counter */
        regAddr = SMEM_SIP6_10_EXACT_MATCH_INDEX_FAIL_COUNTER_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddr, 0, 1, &indexFailCounter);
        smemRegFldSet(devObjPtr, regAddr, 0, 1, (indexFailCounter+1));
    }

    return GT_FALSE;
}

/**
* @internal sip6_10_ExactMatchBuildAutoLearnEntry function
* @endinternal
*
* @brief   return GT_TRUE if a new auto learn entry was added correctly
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] hitNum                     - hit number (0,1)
* @param[in] exactMatchProfileIndex     - profile index (0,15)
* @param[in] exactMatchNewKeyDataPtr    - EM-Key data
* @param[in] exactMatchNewKeySizePtr    - EM-Key size
* @param[in] newFlowId                  - flowId value to add in the reduced entry
* @param[in] entryIndex                 - the Index for the new learned entry
*/
GT_BOOL sip6_10_ExactMatchBuildAutoLearnEntry
(
    IN  SKERNEL_DEVICE_OBJECT                   *devObjPtr,
    IN  GT_U32                                  hitNum,
    IN  GT_U32                                  exactMatchProfileIndex,
    IN  GT_U32                                  *exactMatchNewKeyDataPtr,
    IN  SIP6_EXACT_MATCH_KEY_SIZE_ENT           exactMatchNewKeySize,
    IN  GT_U32                                  newFlowId,
    IN  GT_U32                                  entryIndex
)
{
    GT_U32 relativeBank=0;
    GT_U32 regAddr;
    SNET_SIP6_EXACT_MATCH_ENTRY_INFO exactMatchEntryInfo;
    GT_U32 flowIdBitOffset=0, bitsUsed=0;
    GT_U32 globleBitOffset=0;
    GT_U32 hwData[SMEM_EXACT_MATCH_ENTRY_BANK_SIZE_IN_WORDS_CNS];/* 4 words of one bank hw entry - 115 bits  */

    DECLARE_FUNC_NAME(sip6_10_ExactMatchBuildAutoLearnEntry);
    __LOG(("start sip6_10_ExactMatchBuildAutoLearnEntry  \n"));

    exactMatchEntryInfo.valid=1;
    exactMatchEntryInfo.entry_type=0;/*  EXACT_MATCH Key and Action */
    exactMatchEntryInfo.age=1;
    switch (exactMatchNewKeySize)
    {
        case SIP6_EXACT_MATCH_KEY_SIZE_5B_E:
            exactMatchEntryInfo.key_size=0;
            break;
        case SIP6_EXACT_MATCH_KEY_SIZE_19B_E:
            exactMatchEntryInfo.key_size =1;
            break;
        case SIP6_EXACT_MATCH_KEY_SIZE_33B_E:
            exactMatchEntryInfo.key_size=2;
            break;
        case SIP6_EXACT_MATCH_KEY_SIZE_47B_E:
            exactMatchEntryInfo.key_size=3;
            break;
        default:
            skernelFatalError("exactMatchNewKeySize is illegal\n");
                    return GT_FALSE;
    }

    exactMatchEntryInfo.lookup_number = hitNum;
    exactMatchEntryInfo.key_31_0  = snetFieldValueGet(exactMatchNewKeyDataPtr,0, 32);
    exactMatchEntryInfo.key_39_32 = snetFieldValueGet(exactMatchNewKeyDataPtr,32, 8);

    /*  Auto Learn Action in compressed form, Bits [31:0] out of [67:0] */
    regAddr = SMEM_SIP6_10_EXACT_MATCH_AUTO_LEARN_PROFILE_CONFIG_2_REG(devObjPtr, exactMatchProfileIndex);
    smemRegFldGet(devObjPtr, regAddr, 0, 32, &exactMatchEntryInfo.action_31_0);

    /*  Auto Learn Action in compressed form, Bits [63:32] out of [67:0]. */
    regAddr = SMEM_SIP6_10_EXACT_MATCH_AUTO_LEARN_PROFILE_CONFIG_3_REG(devObjPtr, exactMatchProfileIndex);
    smemRegFldGet(devObjPtr, regAddr, 0, 32, &exactMatchEntryInfo.action_63_32);

    /*  Auto Learn Action in compressed form, Bits [67:64] out of [67:0]. */
    regAddr = SMEM_SIP6_10_EXACT_MATCH_AUTO_LEARN_PROFILE_CONFIG_4_REG(devObjPtr, exactMatchProfileIndex);
    smemRegFldGet(devObjPtr, regAddr, 0, 4, &exactMatchEntryInfo.action_67_64);

    /* get the location of the flowId in the reduced entry */
    flowIdBitOffset = sip6_10_ExactMatchAutoLearnProfileFlowIdBitOffset(devObjPtr,exactMatchProfileIndex);

    /* set the flowId new value - hold 16 bit of data */
    if (flowIdBitOffset<=16)/* all 16 bits fit in action_31_0 */
    {
        /* clear bits */
        exactMatchEntryInfo.action_31_0 &= (~(0xFFFFFFFF<<flowIdBitOffset));
        /* set new value */
        exactMatchEntryInfo.action_31_0 |= (newFlowId<<flowIdBitOffset);
    }
     else
    {
        if (flowIdBitOffset > 31 && flowIdBitOffset<=48)/* all 16 bits fit in action_63_32 */
        {
            /* clear bits */
            exactMatchEntryInfo.action_63_32 &= (~(0xFFFFFFFF<<flowIdBitOffset));
            /* set new value */
            exactMatchEntryInfo.action_63_32 |= (newFlowId<<flowIdBitOffset);
        }
        else
        {
            if(flowIdBitOffset>48)
            {
                skernelFatalError("flowIdBitOffset can not be bigger then 48\n");
                return GT_FALSE;
            }
            else
            {
                /* need to split flowId value in 2 */
                bitsUsed = 32 - flowIdBitOffset;
                 /* clear bits */
                exactMatchEntryInfo.action_31_0 &= (~(0xFFFFFFFF<<flowIdBitOffset));
                /* set new value */
                exactMatchEntryInfo.action_31_0 |= (newFlowId<<flowIdBitOffset);
                 /* clear bits */
                exactMatchEntryInfo.action_63_32 &= (~(0xFFFFFFFF<<(16-bitsUsed)));
                /* set new value */
                exactMatchEntryInfo.action_63_32 |= (newFlowId>>bitsUsed);
            }
        }
    }

    for (relativeBank = 0 ;relativeBank <= (GT_U32)exactMatchNewKeySize ;relativeBank ++ )
    {
        /* reset HW data */
        memset(&hwData,0,sizeof(hwData));

        if (relativeBank == 0)
        {
             /* EM KEY+ACTION */
             SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, entryIndex,
                                                   SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_VALID,exactMatchEntryInfo.valid);
             SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, entryIndex,
                                                   SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE,exactMatchEntryInfo.entry_type);
             SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, entryIndex,
                                                   SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_AGE,exactMatchEntryInfo.age);
             SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, entryIndex,
                                                   SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_SIZE,exactMatchEntryInfo.key_size);
             SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, entryIndex,
                                                   SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_LOOKUP_NUMBER,exactMatchEntryInfo.lookup_number);
             SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, entryIndex,
                                                   SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_31_0,exactMatchEntryInfo.key_31_0);
             SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, entryIndex,
                                                   SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_39_32,exactMatchEntryInfo.key_39_32);
             SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, entryIndex,
                                                   SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_31_0,exactMatchEntryInfo.action_31_0);
             SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, entryIndex,
                                                   SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_63_32,exactMatchEntryInfo.action_63_32);
             SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, entryIndex,
                                                   SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_67_64,exactMatchEntryInfo.action_67_64);

        }
        else
        {
            exactMatchEntryInfo.valid=1;
            exactMatchEntryInfo.entry_type=1;/*  EXACT_MATCH Key Only */
            globleBitOffset= 40+((relativeBank-1)* SIP6_EXACT_MATCH_KEY_ONLY_FIELD_SIZE_CNS ); /*40 for the first bank and 112 per bank */
            exactMatchEntryInfo.keyOnly_31_0  = snetFieldValueGet(exactMatchNewKeyDataPtr, globleBitOffset+0  ,32);
            exactMatchEntryInfo.keyOnly_63_32 = snetFieldValueGet(exactMatchNewKeyDataPtr, globleBitOffset+32 ,32);
            exactMatchEntryInfo.keyOnly_95_64 = snetFieldValueGet(exactMatchNewKeyDataPtr, globleBitOffset+64 ,32);
            exactMatchEntryInfo.keyOnly_111_96 = snetFieldValueGet(exactMatchNewKeyDataPtr, globleBitOffset+96 ,16);

            /*EM KEY*/
            SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, (entryIndex+relativeBank),
                                                  SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_VALID,exactMatchEntryInfo.valid);
            SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, (entryIndex+relativeBank),
                                                  SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE,exactMatchEntryInfo.entry_type);
            SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, (entryIndex+relativeBank),
                                                  SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_31_0,exactMatchEntryInfo.keyOnly_31_0);
            SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, (entryIndex+relativeBank),
                                                  SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_63_32,exactMatchEntryInfo.keyOnly_63_32);
            SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, (entryIndex+relativeBank),
                                                  SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_95_64,exactMatchEntryInfo.keyOnly_95_64);
            SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, hwData, (entryIndex+relativeBank),
                                                  SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_111_96,exactMatchEntryInfo.keyOnly_111_96);
        }

        /*  change the index format to match GM pattern (4 msb bits for bank number )
            if the index goes to WM, the index will change back to its original pattern
            (number of bits depends on number of banks )
        */

        if (devObjPtr->emNumOfBanks == 0)
        {
            skernelFatalError("devObjPtr->emNumOfBanks == 0\n");
                return GT_FALSE;
        }

        smemGenericHiddenMemSet(devObjPtr,SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_E ,
                                (entryIndex+relativeBank), hwData,
                                SMEM_EXACT_MATCH_ENTRY_BANK_SIZE_IN_WORDS_CNS);
    }/*end for ralativeBank loop*/

    return GT_TRUE;
}

/**
* @internal sip6_10_ExactMatchFreeFlowId function
* @endinternal
*
* @brief   return GT_TRUE if a free flowId was found
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[out] freeFlowIdPtr             - free flowId value
*/
GT_BOOL sip6_10_ExactMatchFreeFlowId
(
    IN  SKERNEL_DEVICE_OBJECT                   *devObjPtr,
    OUT GT_U32                                  *freeFlowIdPtr
)
{
    GT_U32 regAddr;
    GT_U32 rangeFull;
    GT_U32 numberOfAllocatedFlowId;
    GT_U32 firstOldestFlowId;
    GT_U32 nextFlowId;
    GT_U32 ipfixBaseFlowId;
    GT_U32 flowIdAllocationThreshold;
    GT_U32 maxNumberOfAllocatedFlowId;
    GT_U32 flowIdFailCounter;

    DECLARE_FUNC_NAME(sip6_10_ExactMatchFreeFlowId);
    __LOG(("start sip6_10_ExactMatchFreeFlowId  \n"));

    /* This field indicates the current state of the Flow-ID allocation range.
       If all the allocated Flow-IDs have been consumed, then this bit is automatically set.
       When the Flow-ID range is not fully utilized this bit is automatically cleared. */
    regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_1_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddr, 0, 1, &rangeFull);

    if (rangeFull)
    {
        if (sip6_10_ExactMatchAutoLearnStatusCountersEnableGet(devObjPtr)==GT_TRUE)
        {
            /* update fail counter */
            regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_FAIL_COUNTER_REG(devObjPtr);
            smemRegFldGet(devObjPtr, regAddr, 0, 1, &flowIdFailCounter);
            smemRegFldSet(devObjPtr, regAddr, 0, 1, (flowIdFailCounter+1));
        }

       /* Generate interrupt :  Flow ID Allocation Failed bit 2
           Invoked when a Flow-ID allocation has failed. This means that the entire
           allocatable Flow-ID range has been consumed by the exact match automatic
           learning mechanism, and the current EM cannot be learned */
        __LOG(("sip6_10_ExactMatchFreeFlowId : Generate interrupt :  Flow ID Allocation Failed - no free flowId\n"));
        snetChetahDoInterrupt(devObjPtr,
                      SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_INTERRUPT_CAUSE_REGISTER_REG(devObjPtr),
                      SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_INTERRUPT_MASK_REGISTER_REG(devObjPtr),
                      (1<<2),/* Flow ID Allocation Failed */
                      0/*not used in sip5*/);

        return GT_FALSE;
    }

    /* Specifies the next Flow-ID, to be used when the next EM entry is added.
       When no Flow-IDs are currently in use: <First Flow ID>=<Next Flow ID>.
       When all the Flow-IDs in the range have been consumed, the first and next
       are equal, and also the <Flow ID Range Full> is set. */
    regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_3_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddr, 0, 16, &nextFlowId);

    /* This field indicated the base Flow-ID that is used for indexing the IPFIX table. */
    regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_CONFIG_2_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddr, 0, 16, &ipfixBaseFlowId);

    *freeFlowIdPtr = nextFlowId + ipfixBaseFlowId;

    /* Specifies the maximum number of Flow-IDs*/
    regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_CONFIG_2_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddr, 16, 16, &maxNumberOfAllocatedFlowId);

    /* Increment <Next Flow ID> */
    if((nextFlowId+1) == maxNumberOfAllocatedFlowId)
    {
        nextFlowId=0;
    }
    else
    {
        nextFlowId++;
    }
    regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_3_REG(devObjPtr);
    smemRegFldSet(devObjPtr, regAddr, 0, 16, nextFlowId);

    /* Specifies the first (oldest) Flow-ID in that is currently in use. */
    regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_2_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddr, 0, 16, &firstOldestFlowId);

    if (nextFlowId==firstOldestFlowId)
    {
        /* we reach the maximum flowId, need to update all flowId was taken and set interrupt */
        rangeFull = GT_TRUE;
        regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_1_REG(devObjPtr);
        smemRegFldSet(devObjPtr, regAddr, 0, 1, 1);

       /* Generate interrupt : flowId range full bit 3
        invoked when the entire allocatable Flow-ID range has been consumed by
        the exact match automatic learning mechanism.
        Future new flows will not be learned until at least one of the existing
        flows is removed.
        This interrupt is invoked for the last packet that has successfully allocated a
        Flow-ID. At this point the Flow-ID range is full, and future new flows that will
        be received will trigger the <Flow ID Allocation Failed Interrupt>*/
        __LOG(("sip6_10_ExactMatchFreeFlowId : Generate interrupt :  flowId range full \n"));
        snetChetahDoInterrupt(devObjPtr,
                      SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_INTERRUPT_CAUSE_REGISTER_REG(devObjPtr),
                      SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_INTERRUPT_MASK_REGISTER_REG(devObjPtr),
                      (1<<3),/*  flowId range full */
                      0/*not used in sip5*/);
    }

    /* Specifies how many Flow-IDs are currently allocated by the EM Engine. */
    regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_1_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddr, 1, 17, &numberOfAllocatedFlowId);
    smemRegFldSet(devObjPtr, regAddr, 1, 17, (numberOfAllocatedFlowId+1));/* increase allocated flowId */

    /* When the number of allocated Flow-IDs is greater than this threshold,
      an interrupt is invoked */
    regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_CONFIG_1_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddr, 1, 16, &flowIdAllocationThreshold);
    if ((numberOfAllocatedFlowId+1)>flowIdAllocationThreshold)
    {
        /* Generate interrupt : Flow ID Threshold Crossed bit 4
        Invoked when the number of allocated Flow-IDs in the automatic learning
        mechanism exceeds the <Flow ID Allocation Threshold> */
        __LOG(("sip6_10_ExactMatchFreeFlowId : Generate interrupt : Flow ID Threshold Crossed \n"));
        snetChetahDoInterrupt(devObjPtr,
                      SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_INTERRUPT_CAUSE_REGISTER_REG(devObjPtr),
                      SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_INTERRUPT_MASK_REGISTER_REG(devObjPtr),
                      (1<<4),/*  Flow ID Threshold Crossed */
                      0/*not used in sip5*/);
    }

    return GT_TRUE;
}

/**
* @internal sip6_10_ExactMatchRecycleFlowId function
* @endinternal
*
* @brief   When bit is set by the CPU, it causes the <First Flow ID> to
*          be marked as free, and this bit is automatically cleared.
*          return GT_TRUE if recycle was successful
*
* @param[in] devObjPtr                  - (pointer to) the device object
*/
GT_BOOL sip6_10_ExactMatchRecycleFlowId
(
    IN  SKERNEL_DEVICE_OBJECT                   *devObjPtr
)
{
    GT_U32 regAddr;
    GT_U32 rangeFull;
    GT_U32 numberOfAllocatedFlowId;
    GT_U32 firstOldestFlowId;
    GT_U32 nextFlowId;
    GT_U32 maxNumberOfAllocatedFlowId;

    GT_U32 recycleFlowIdEn;

    DECLARE_FUNC_NAME(sip6_10_ExactMatchRecycleFlowId);
    __LOG(("start sip6_10_ExactMatchRecycleFlowId  \n"));


    /* When this bit is set by the CPU, it causes the <First Flow ID> to be marked as
       free, and this bit is automatically cleared. */
    regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_CONFIG_1_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddr, 0, 1, &recycleFlowIdEn);

    if(recycleFlowIdEn==0)
    {
         /* recycle flow id was not triggered */
        __LOG(("sip6_10_ExactMatchRecycleFlowId :no recycle trigger \n"));
        return GT_FALSE;
    }
    else
    {
        /* check if the allocated Flow-ID list is empty: */

        /*  This field indicates the current state of the Flow-ID allocation range.
            If all the allocated Flow-IDs have been consumed, then this bit is automatically set.
            When the Flow-ID range is not fully utilized this bit is automatically cleared. */
        regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_1_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddr, 0, 1, &rangeFull);

        /*  Specifies the next Flow-ID, to be used when the next EM entry is added.
            When no Flow-IDs are currently in use: <First Flow ID>=<Next Flow ID>.
            When all the Flow-IDs in the range have been consumed, the first and next
            are equal, and also the <Flow ID Range Full> is set. */
        regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_3_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddr, 0, 16, &nextFlowId);

        /* Specifies the first (oldest) Flow-ID in that is currently in use. */
        regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_2_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddr, 0, 16, &firstOldestFlowId);

        if ((nextFlowId==firstOldestFlowId) && (rangeFull==0))
        {
            /* Generate interrupt :  Flow ID Empty bit 5
               Invoked when the software tries to recycle a Flow-ID, and no Flow-IDs are currently allocated */
            __LOG(("sip6_10_ExactMatchRecycleFlowId : Generate interrupt :  Flow ID Empty - can not recycle \n"));
            snetChetahDoInterrupt(devObjPtr,
                          SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_INTERRUPT_CAUSE_REGISTER_REG(devObjPtr),
                          SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_INTERRUPT_MASK_REGISTER_REG(devObjPtr),
                          (1<<5),/* Flow ID Empty */
                          0/*not used in sip5*/);
            return GT_FALSE;
        }
        else
        {
            /* increment <First Flow ID> */

             /* Specifies the maximum number of Flow-IDs*/
            regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_CONFIG_2_REG(devObjPtr);
            smemRegFldGet(devObjPtr, regAddr, 16, 16, &maxNumberOfAllocatedFlowId);

            regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_2_REG(devObjPtr);
            if ((firstOldestFlowId+1) == maxNumberOfAllocatedFlowId)
            {
                /* set firstOldestFlowId = 0 */
                smemRegFldSet(devObjPtr, regAddr, 0, 16, 0);
            }
            else
            {
                 /* set firstOldestFlowId = firstOldestFlowId+1 */
                smemRegFldSet(devObjPtr, regAddr, 0, 16, (firstOldestFlowId+1));
            }

            if (rangeFull==1)
            {
                /* set  <Flow ID Range Full> = 0*/
                regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_1_REG(devObjPtr);
                smemRegFldSet(devObjPtr, regAddr, 0, 1, 0);
            }

            /* Update <Number of Allocated Flow IDs> */

            /* Specifies how many Flow-IDs are currently allocated by the EM Engine. */
            regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_1_REG(devObjPtr);
            smemRegFldGet(devObjPtr, regAddr, 1, 17, &numberOfAllocatedFlowId);
            if(numberOfAllocatedFlowId==0)
            {
                 skernelFatalError("numberOfAllocatedFlowId is 0 and can not be decremented \n");
                 return GT_FALSE;
            }
            smemRegFldSet(devObjPtr, regAddr, 1, 17, (numberOfAllocatedFlowId-1));/* decrease allocated flowId */
        }
    }

    return GT_TRUE;
}
/**
* @internal sip6_10_ExactMatchAutoLearnEntryIndexSet function
* @endinternal
*
* @brief   Set the index of the exact mathc entry bind to the
*          flowId
*
* @param[in] devObjPtr              - (pointer to) the device object
* @param[in] freeFlowIdPtr          - free flowId value,index to the table
* @param[in] exactMatchIndex        - value of the learnd exact match index
* @param[in] bankNum                - value of the learnd exact match bank
*/
GT_VOID sip6_10_ExactMatchAutoLearnEntryIndexSet
(
    IN  SKERNEL_DEVICE_OBJECT                   *devObjPtr,
    IN  GT_U32                                  freeFlowId,
    IN  GT_U32                                  exactMatchIndex,
    IN  GT_U32                                  bankNum
)
{
    GT_U32 regAddr;
    GT_U32 ipfixBaseFlowId;
    GT_U32 autoLearnEntryIndex;
    GT_U32 autoLearnEmEntryAddr;
    GT_U32 exactMatchIndexPlusBank;

    /* update Auto Learn Exact Match Entry index table*/
     regAddr = SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_CONFIG_2_REG(devObjPtr);
     smemRegFldGet(devObjPtr, regAddr, 0, 16, &ipfixBaseFlowId);

     autoLearnEntryIndex = freeFlowId - ipfixBaseFlowId;
     autoLearnEmEntryAddr = SMEM_SIP6_10_AUTO_LEARN_EXACT_MATCH_ENTRY_TBL_MEM(devObjPtr, autoLearnEntryIndex);

     exactMatchIndexPlusBank = (exactMatchIndex<<4) | bankNum;
     smemMemSet(devObjPtr, autoLearnEmEntryAddr, &exactMatchIndexPlusBank, 1);

     return;
}

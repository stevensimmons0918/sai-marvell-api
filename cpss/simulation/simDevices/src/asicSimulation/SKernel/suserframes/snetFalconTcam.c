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
* @file snetFalconTcam.c
*
* @brief Sip6 Falcon Tcam
*
* @version   1
********************************************************************************
*/

/*#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>

#include <asicSimulation/SKernel/suserframes/snet.h>
*/
#include <asicSimulation/SKernel/cheetahCommon/sregLion2.h>
#include <asicSimulation/SKernel/suserframes/snetLion3Tcam.h>
#include <asicSimulation/SKernel/suserframes/snetFalconExactMatch.h>

#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypeTcam.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>

#define MAX_HIT_NUM_CNS 4

static GT_CHAR* tcamClientName[SIP5_TCAM_CLIENT_LAST_E + 1] =
{
    STR(SIP5_TCAM_CLIENT_TTI_E   ),
    STR(SIP5_TCAM_CLIENT_IPCL0_E ),
    STR(SIP5_TCAM_CLIENT_IPCL1_E ),
    STR(SIP5_TCAM_CLIENT_IPCL2_E ),
    STR(SIP5_TCAM_CLIENT_EPCL_E  ),

    NULL
};


/**
* @internal snetFalconTcamLookup function
* @endinternal
*
* @brief   do lookup in tcam for given key
*          and fill the results array
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] tcamClient               - tcam client
* @param[in] emProfileAccessIndex     - index to the EM Profile table
* @param[in] tcamProfileId            - TCAM Profile id
* @param[in] keyArrayPtr              - key array (size up to 80 bytes)
* @param[in] keySize                  - size of the key
*                                       number of hits found
* @param[out] resultArr               - the index hits
* @param[out] resultHitDoneInTcamArr  - the hits location.
*                                       GT_TRUE for TCAM hits
*                                       GT_FALSE for non TCAM
*                                       hits
*/
GT_U32 snetFalconTcamLookup
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr ,
    IN  SIP5_TCAM_CLIENT_ENT      tcamClient,
    IN  GT_U32                    emProfileAccessIndex,
    IN  GT_U32                    tcamProfileId,
    IN  GT_U32                   *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT    keySize,
    OUT GT_U32                    resultArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS],
    OUT GT_BOOL                   resultHitDoneInTcamArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS]
)
{
    DECLARE_FUNC_NAME(snetFalconTcamLookup);

    GT_U32      tcamNumOfHits=0;
    GT_U32      exactMatchNumOfHits=0;
    GT_U32      numOfHits=0;
    GT_CHAR     *clientNamePtr = ((tcamClient < SIP5_TCAM_CLIENT_LAST_E) ? tcamClientName[tcamClient] : "unknown");

    GT_U32      exactMatchProfileIdArr[SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS];
    GT_U32      exactMatchIndexArr[SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS];
    GT_BOOL     exactMatchClientMatchArr[SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS]={GT_FALSE,GT_FALSE};

    GT_U32      tcamOverExactMatchPriorityArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS]={GT_TRUE};

    GT_U32      *actionEntryDataPtr;
    GT_U32      indexBaseAddress;
    GT_U32      ii=0;

    SIP6_EXACT_MATCH_KEY_SIZE_ENT       exactMatchNewKeySize;
    GT_U32                              exactMatchProfileTableKeyStart;
    GT_U32                              exactMatchProfileTableEnableDefault;
    GT_U32                              *memPtr,matchIndex;


    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TCAM_E);

    __LOG(("Start Tcam check for [%s] \n",
        clientNamePtr));

    /* search the key in TCAM */
    tcamNumOfHits = sip5TcamLookup(
        devObjPtr, tcamClient, tcamProfileId, keyArrayPtr, keySize, resultArr);

    for (ii=0;ii<SIP5_TCAM_MAX_NUM_OF_HITS_CNS;ii++)
    {
        if(devObjPtr->limitedNumOfParrallelLookups &&
           ii >= devObjPtr->limitedNumOfParrallelLookups)
        {
            break;
        }

        /* reset array with GT_TRUE value - give TCAM priority over EM */
        resultHitDoneInTcamArr[ii]=GT_TRUE;

        if(resultArr[ii]!=SNET_CHT_POLICY_NO_MATCH_INDEX_CNS)
        {
            indexBaseAddress = SMEM_LION3_TCAM_ACTION_TBL_MEM(devObjPtr, resultArr[ii]);
            actionEntryDataPtr = smemMemGet(devObjPtr,indexBaseAddress);

            memPtr     = actionEntryDataPtr; /* parameter for SMEM_SIP6_IPCL_ACTION_ENTRY_FIELD_GET */
            matchIndex = resultArr[ii];      /* parameter for SMEM_SIP6_IPCL_ACTION_ENTRY_FIELD_GET */

            /* read the priority bit from the TCAM action */
            tcamOverExactMatchPriorityArr[ii] =
                SMEM_SIP6_IPCL_ACTION_ENTRY_FIELD_GET(SMEM_SIP6_TCAM_ACTION_TABLE_FIELDS_TCAM_OVER_EXACT_MATCH_ENABLE) ?
                GT_TRUE : GT_FALSE;
        }
        else
        {
            if (ii<SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS)
            {
                tcamOverExactMatchPriorityArr[ii] = GT_FALSE; /*no hit in TCAM */
            }
        }
    }

    /*get exact match profile id */
    snetFalconExactMatchProfileIdGet(
        devObjPtr, descrPtr, tcamClient, emProfileAccessIndex ,&exactMatchProfileIdArr[0]);

    if ((exactMatchProfileIdArr[0]!=0)||(exactMatchProfileIdArr[1]!=0))
    {
        if(SMAIN_NOT_VALID_CNS == UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_EM))
        {
            /* The EM unit not exists */
            __LOG(("ERROR : configuration ERROR : The TCAM client point to EM lookup although the EM unit not exists in the device \n"));
            __LOG(("ERROR : configuration ERROR : EM lookup ignored (will HW also be forgiven ?) \n"));
            return tcamNumOfHits;
        }

        /* search for a match */
        exactMatchNumOfHits = snetFalconExactMacthLookup(devObjPtr, tcamClient,
                                                         keyArrayPtr, keySize,
                                                         exactMatchProfileIdArr,
                                                         exactMatchClientMatchArr,
                                                         exactMatchIndexArr);

        for (ii=0;ii<SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS;ii++)
        {
            if (exactMatchClientMatchArr[ii]==GT_TRUE)
            {
                if (resultArr[ii] != SNET_CHT_POLICY_NO_MATCH_INDEX_CNS) /* there is hit in Tcam*/
                {
                    if( (exactMatchIndexArr[ii]!=SNET_CHT_POLICY_NO_MATCH_INDEX_CNS) && /* there is hit in em*/
                        (tcamOverExactMatchPriorityArr[ii] == GT_FALSE)                )/*em have priority*/
                    {
                        __LOG(("take exact match index - reason : got priority \n "));
                        resultArr[ii]=exactMatchIndexArr[ii];
                        resultHitDoneInTcamArr[ii]=GT_FALSE;
                    }
                }
                else /*no hit in tcam*/
                {
                    if(exactMatchIndexArr[ii]!=SNET_CHT_POLICY_NO_MATCH_INDEX_CNS ) /* there is hit in em*/
                    {
                        __LOG(("take exact match index - reason : no Tcam hit  \n "));
                        resultArr[ii]=exactMatchIndexArr[ii];
                        resultHitDoneInTcamArr[ii]=GT_FALSE;
                    }
                    else /*no hit in em , check for default action*/
                    {
                        if (exactMatchProfileIdArr[ii]!= 0)
                        {
                            snetFalconExactMatchProfileTableControlGet(devObjPtr,
                                exactMatchProfileIdArr[ii],
                                &exactMatchNewKeySize,
                                &exactMatchProfileTableKeyStart,
                                &exactMatchProfileTableEnableDefault);

                            if (exactMatchProfileTableEnableDefault==GT_TRUE)
                            {
                                __LOG(("take default action \n "));
                                resultHitDoneInTcamArr[ii] = GT_FALSE;
                                resultArr[ii] =(ii==0) ?  SNET_SIP6_EXACT_MATCH_DEFAULT_INDEX_LOOKUP0_CNS :
                                                          SNET_SIP6_EXACT_MATCH_DEFAULT_INDEX_LOOKUP1_CNS ;
                                exactMatchNumOfHits++;
                            }
                        }
                    }
                }
            }/*end if exactMatchClientMatchArr 0 or 1 == GT_TRUE*/
        }/*end for */
    }/*if profileId 0 or 1 != 0*/
    else
    {
        __LOG(("profileId1 and profileId2 set to 0 - Skipping Exact Match engine  \n "));
    }

    /* need to return the bigger num of hits */
    numOfHits = ((tcamNumOfHits > exactMatchNumOfHits) ? tcamNumOfHits : exactMatchNumOfHits);

    return numOfHits;
}

/**
* @internal snetFalconTcamPclLookup function
* @endinternal
*
* @brief   do pcl lookup in tcam
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] iPclTcamClient           - tcam client
* @param[in] keyIndex                 - index to the PCL Profile
*                                       table
* @param[in] u32keyArrayPtr           - key array (GT_U32)
* @param[in] keyFormat                - format of the key
*
* @param[out] matchIndexPtr           - the index hits
* @param[out] matchDoneInTcamPtr      - the hits location.
*                                       GT_TRUE for TCAM hits
*                                       GT_FALSE for non TCAM
*                                       hits
*/
GT_VOID snetFalconTcamPclLookup
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  SIP5_TCAM_CLIENT_ENT      iPclTcamClient,
    IN  GT_U32                    keyIndex,
    IN  GT_U32                   *u32keyArrayPtr,
    IN  CHT_PCL_KEY_FORMAT_ENT    keyFormat,
    OUT GT_U32                   *matchIndexPtr,
    OUT GT_BOOL                  *matchDoneInTcamPtr
)
{
    SIP5_TCAM_KEY_SIZE_ENT   sip5KeySize;
    GT_U32                   resultArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS];
    GT_BOOL                  resulDoneInTcamArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS];
    GT_U32                   ii;
    GT_U32                   numOfHits;
    GT_U32                   tcamProfileIndex;
    GT_U32                   regAddr;
    GT_U32                   regVal;

    tcamProfileIndex = 0;
    if (SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        /* remap profile index */
        if (iPclTcamClient == SIP5_TCAM_CLIENT_EPCL_E)
        {
            /* EPCL */
            regAddr = SMEM_SIP6_EPCL_EXACT_MATCH_PROFILE_ID_MAP_TBL_MEM(devObjPtr, keyIndex);
            smemRegGet(devObjPtr, regAddr, &regVal);
            tcamProfileIndex = (regVal >> 8) & 0x3F;
        }
        else
        {
            /* IPCL0-2 */
            regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.exactMatchProfileIdMapEntry[keyIndex];
            smemRegGet(devObjPtr, regAddr, &regVal);
            tcamProfileIndex = (regVal >> 8) & 0x3F;
        }
    }

    /* convert old key format to new key size */
    sip5TcamConvertPclKeyFormatToKeySize(keyFormat, &sip5KeySize);

    /* search the key */
    memset(resultArr, 0, sizeof(resultArr));
    memset(resulDoneInTcamArr, 0, sizeof(resulDoneInTcamArr));
    numOfHits = snetFalconTcamLookup(
        devObjPtr,NULL/*descrPtr*/, iPclTcamClient, keyIndex, tcamProfileIndex,
        u32keyArrayPtr, sip5KeySize, resultArr,resulDoneInTcamArr);

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
           matchDoneInTcamPtr[ii] = resulDoneInTcamArr[ii];
        }
    }
}



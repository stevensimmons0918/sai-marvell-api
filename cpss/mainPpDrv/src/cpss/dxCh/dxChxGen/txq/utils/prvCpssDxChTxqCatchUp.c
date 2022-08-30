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
* @file prvCpssDxChTxqCatchUp.c
*
* @brief CPSS SIP6 TXQ  catch up utilities.
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>


#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPds.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqSearchUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>


extern GT_STATUS prvCpssDxChTxqPdsLongQueueAttributesGet
(
    IN  GT_U32      devNum,
    IN  GT_BOOL     speedToProfile,
    INOUT  GT_U32   *speedinGPtr,
    INOUT PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC **profilePtr
);

static GT_STATUS prvCpssDxChTxqtc4PfcToEgfIndexCatchUp
(
    IN GT_U8 devNum
)
{
    GT_U32 i;
    GT_U32 value,tc4Pfc;
    GT_STATUS rc;
    GT_U32 arraySize;

    arraySize = sizeof(GT_U32)*CPSS_TC_RANGE_CNS*PRV_CPSS_DXCH_TC4PFC_TO_EGF_INDEX_ARR_SIZE_CNS;

    cpssOsMemSet(PRV_CPSS_DXCH_PP_MAC(devNum)->tc4PfcToEgfIndex,0,arraySize);

    for(i=0;i<_4K;i++)
    {
        rc = prvCpssDxChReadTableEntry(devNum,CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E,i,&value);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChReadTableEntry EGF_QAG_TC_DP_MAPPER  failed  for  index %d  ",i);
        }

        tc4Pfc = U32_GET_FIELD_MAC(value,6,3);

        PRV_CPSS_DXCH_PP_MAC(devNum)->tc4PfcToEgfIndex[tc4Pfc][i>>5] |=0x1<<(i&0x1F);
    }

    return GT_OK;
}


/**
* @internal  prvCpssDxChTxqPdsProfilesCatchUp  function
* @endinternal
*
* @brief   Perform catch up of  long queue and length adjust profiles.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum               PP's device number.
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
static GT_STATUS prvCpssDxChTxqPdsProfilesCatchUp
(
    IN GT_U8 devNum
)
{
    GT_U32 localPortNum,physicalPortNum;
    GT_STATUS rc;
    GT_U32 tileNum,dpNum,numberOfTiles;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE  *aNodePtr;
    GT_U32                          profileIndex;
    GT_U32                          lengthAdjustIndex=0;
    GT_U32                          wordIndex,bitIndex,*currentWordPtr;
    GT_BOOL                         speedSet;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC profile;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC lengthAdjustParameters;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC        longQueueParameters;
    GT_U32                          speed_in_G;
    GT_U32                         lengthAdjustResored [CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM]={0};
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pNodePtr = NULL;
    GT_U32                         pNodeIndex;

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }


    profile.lengthAdjustParametersPtr = &lengthAdjustParameters;
    profile.longQueueParametersPtr= NULL;


    /*Go over all the ports and update binded profile number*/
    for(tileNum=0;tileNum<numberOfTiles;tileNum++)
    {
         for(dpNum=0;dpNum<MAX_DP_IN_TILE(devNum);dpNum++)
         {
            for(localPortNum=0;localPortNum<CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum);localPortNum++)
            {
                /*skip Txq reserved ports*/
                PRV_CPSS_TXQ_UTILS_SKIP_RESERVED_MAC(devNum,localPortNum);

                /*check if mapped*/
                rc  = prvCpssSip6TxqUtilsPnodeIndexGet(devNum, dpNum, localPortNum,&pNodeIndex);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqUtilsPnodeIndexGet failed ");
                }

                PRV_TXQ_SIP_6_PNODE_INSTANCE_NO_ERROR_GET(pNodePtr,devNum,tileNum,pNodeIndex);
                if(pNodePtr==NULL)
                {   /*not mapped*/
                    continue;
                }
                rc = prvCpssFalconTxqUtilsPhysicalPortNumberGet(devNum,tileNum,dpNum,localPortNum,&physicalPortNum);
                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Physical port that belong to  tile %d dp %d port %d not found\n",
                        tileNum,dpNum,localPortNum);
                }


                if(CPSS_SIP6_TXQ_INVAL_DATA_CNS!=physicalPortNum)
                {
                    wordIndex = physicalPortNum/32;
                    bitIndex=physicalPortNum%32;

                    if(wordIndex>=PRV_CPSS_DXCH_SIP_6_TXQ_PROFILE_BMP_WORDS_NUM_MAC)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Physical port number [%d] is out of range\n",
                            physicalPortNum);
                    }

                    /*Get first queue number*/
                    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,physicalPortNum,&aNodePtr);
                    if(rc !=GT_OK)
                    {
                       return rc;
                    }

                     /*For SIP 6.10 and higher length adjust is not coupled with long queue profile*/
                    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
                    {
                      rc = prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapGet(devNum,tileNum,dpNum,aNodePtr->queuesData.queueBase,&lengthAdjustIndex);
                      if(rc !=GT_OK)
                      {
                         return rc;
                      }

                      /*all port mapped to default at init*/
                      if(lengthAdjustIndex!=DEFAULT_LENGTH_ADJUST_PROFILE)
                      {
                        /*delete from default*/
                        currentWordPtr= &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[DEFAULT_LENGTH_ADJUST_PROFILE].bindedPortsBmp[wordIndex]);
                        (*currentWordPtr) &= ~((1 << bitIndex));
                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[DEFAULT_LENGTH_ADJUST_PROFILE].numberOfBindedPorts--;

                        /*add to new*/
                        currentWordPtr= &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[lengthAdjustIndex].bindedPortsBmp[wordIndex]);
                        (*currentWordPtr)|=((1<<bitIndex));
                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[lengthAdjustIndex].numberOfBindedPorts++;

                      }

                      if(lengthAdjustResored[lengthAdjustIndex]==0)
                      {
                           /*get the profile*/
                          rc = prvCpssDxChTxqFalconPdsProfileGet(devNum,tileNum,dpNum,lengthAdjustIndex,GT_TRUE,&profile);
                          if(rc !=GT_OK)
                          {
                             return rc;
                          }

                          lengthAdjustResored[lengthAdjustIndex]=1;
                          /*update db*/
                          PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustDb[lengthAdjustIndex] = lengthAdjustParameters;
                       }


                    }

                    /*check that speed is configured*/
                    rc = prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapGet(devNum,physicalPortNum,&speedSet);
                    if(rc!=GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapGet failed\n");
                    }

                    if(GT_FALSE==speedSet)
                    {
                        continue;
                    }




                    /*Now check to which PDS profile it is binded*/
                    rc = prvCpssDxChTxqFalconPdsQueueProfileMapGet(devNum,tileNum,dpNum,aNodePtr->queuesData.queueBase,&profileIndex);
                    if(rc !=GT_OK)
                    {
                       return rc;
                    }

                    currentWordPtr= &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[profileIndex].bindedPortsBmp[wordIndex]);
                    (*currentWordPtr)|=((1<<bitIndex));
                    PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[profileIndex].numberOfBindedPorts++;


                    /*recover speed and long queue parameters*/
                    if(0==PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[profileIndex].speed_in_G)
                    {
                        profile.longQueueParametersPtr=&longQueueParameters;

                        /*get the profile*/
                        rc = prvCpssDxChTxqFalconPdsProfileGet(devNum,tileNum,dpNum,profileIndex,GT_FALSE,&profile);
                        if(rc !=GT_OK)
                        {
                           return rc;
                        }

                        /*now get the speed*/

                        rc =prvCpssDxChTxqPdsLongQueueAttributesGet(devNum,GT_FALSE,&speed_in_G,
                            &(profile.longQueueParametersPtr));
                        if(rc !=GT_OK)
                        {
                           return rc;
                        }

                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[profileIndex].longQueueParametersPtr=profile.longQueueParametersPtr;
                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[profileIndex].speed_in_G = speed_in_G;

                        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[profileIndex].lengthAdjustParametersPtr)
                        {
                            *(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[profileIndex].lengthAdjustParametersPtr) =lengthAdjustParameters;
                        }

                    }
                }
            }
         }
    }
    return GT_OK;
}


/**
* @internal  prvCpssDxChTxqCatchUp  function
* @endinternal
*
* @brief   Perform catch up of  txQ SW database
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum               PP's device number.
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
static GT_STATUS prvCpssDxChTxqCatchUp
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc=GT_OK;
    GT_U32 i;

    GT_STATUS (*txQCatchupFunc[])(GT_U8) =
        {
            prvCpssDxChTxqPdsProfilesCatchUp,
            prvCpssDxChTxqtc4PfcToEgfIndexCatchUp,
            NULL
        };

    for(i=0;txQCatchupFunc[i];i++)
    {
        if((rc =txQCatchupFunc[i](devNum))!=GT_OK)
        {
            break;
        }
    }

    return rc;
}


/**
* @internal  prvCpssDxChTxqPdsProfilesCatchUpTest  function
* @endinternal
*
* @brief  Debug function to check catch up procedure of PDS profiles.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum               PP's device number.
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/

GT_STATUS prvCpssDxChTxqPdsProfilesCatchUpTest
(
    IN GT_U8 devNum
)
{
   GT_U32 i,j;
   GT_STATUS rc;
   PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC pdsProfiles[CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM];
   PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC lengthAdjust[CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM];
   PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC * lengthAdjustParametersPtr;
   PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC  lengthAdjustParametersArr[CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM];
   PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION                   *tileConfigsPtr;

   cpssOsMemCpy(pdsProfiles,PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles,sizeof(PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC)*CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM);
   cpssOsMemCpy(lengthAdjust,PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles,sizeof(PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC)*CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM);

   for(i=0;i<CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM;i++)
   {
     if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
     {
        cpssOsMemCpy(lengthAdjustParametersArr+i,PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].lengthAdjustParametersPtr,
            sizeof(PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC));
     }
     else
     {
        cpssOsMemCpy(lengthAdjustParametersArr+i,PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[i].lengthAdjustParametersPtr,
            sizeof(PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC));
     }
   }

   tileConfigsPtr  = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[0]);

   rc = prvCpssFalconTxqUtilsInitPdsProfilesDb(devNum,tileConfigsPtr ->general.numberOfMappedPorts);
   if(rc !=GT_OK)
   {
      return rc;
   }


   rc =prvCpssDxChTxqPdsProfilesCatchUp(devNum);
   if(rc !=GT_OK)
   {
      return rc;
   }

   for(i=0;i<CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM;i++)
   {
     if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].numberOfBindedPorts!=pdsProfiles[i].numberOfBindedPorts)
     {
         cpssOsPrintf("Missmatch  profile %d numberOfBindedPorts =[after %d before %d]\n",i,
             PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].numberOfBindedPorts,lengthAdjust[i].numberOfBindedPorts);
     }

     if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
     {
       if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[i].numberOfBindedPorts!=lengthAdjust[i].numberOfBindedPorts)
       {
           cpssOsPrintf("Missmatch length adjust profile %d numberOfBindedPorts =[after %d before %d]\n",i,
               PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[i].numberOfBindedPorts,lengthAdjust[i].numberOfBindedPorts);
       }
     }

     if(0!=pdsProfiles[i].numberOfBindedPorts)
     {
         for(j=0;j<32;j++)
         {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].bindedPortsBmp[j]!=pdsProfiles[i].bindedPortsBmp[j])
            {
             cpssOsPrintf("Missmatch profile %d bindedPortsBmp[%d] =[before 0x%X after 0x%X]\n",i,j,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].bindedPortsBmp[j],pdsProfiles[i].bindedPortsBmp[j]);
            }
         }

         if(pdsProfiles[i].longQueueParametersPtr!=PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].longQueueParametersPtr)
         {
             cpssOsPrintf("Missmatch profile %d longQueueParametersPtr is different\n",i);
         }
     }

     if((0!=pdsProfiles[i].numberOfBindedPorts&&(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE))||
        (0!=lengthAdjust[i].numberOfBindedPorts&&(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)))
     {
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
        {
         lengthAdjustParametersPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].lengthAdjustParametersPtr;
        }
        else
        {
         lengthAdjustParametersPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[i].lengthAdjustParametersPtr;
        }

        if(0!=(cpssOsMemCmp(lengthAdjustParametersArr+i,lengthAdjustParametersPtr,
         sizeof(PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC))))
         {
             cpssOsPrintf("Missmatch profile %d lengthAdjustParametersPtr  is different\n",i);
             cpssOsPrintf("New/Old  :lengthAdjustByteCount  %d/%d\n",lengthAdjustParametersPtr->lengthAdjustByteCount,
                lengthAdjustParametersArr[i].lengthAdjustByteCount);
             cpssOsPrintf("New/Old  :lengthAdjustEnable  %d/%d\n",lengthAdjustParametersPtr->lengthAdjustEnable,
                lengthAdjustParametersArr[i].lengthAdjustEnable);
             cpssOsPrintf("New/Old  :lengthAdjustByteCount  %d/%d\n",lengthAdjustParametersPtr->lengthAdjustSubstruct,
                lengthAdjustParametersArr[i].lengthAdjustSubstruct);
         }

        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
        {
          for(j=0;j<32;j++)
          {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[i].bindedPortsBmp[j]!=lengthAdjust[i].bindedPortsBmp[j])
            {
             cpssOsPrintf("Missmatch lengthAdjustProfiles profile %d bindedPortsBmp[%d] =[after  0x%X before 0x%X]\n",i,j,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[i].bindedPortsBmp[j],lengthAdjust[i].bindedPortsBmp[j]);
            }
          }
        }

     }






   }
   return GT_OK;
}


/**
* @internal  prvCpssDxChTxqPdsProfilesCatchUpTest  function
* @endinternal
*
* @brief  Debug function to check catch up procedure of Tc4PfcToEgfIndex.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum               PP's device number.
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/

GT_STATUS prvCpssDxChTxqTc4PfcToEgfIndexCatchUpTest
(
    IN GT_U8 devNum
)
{
   GT_U32 i,j;
   GT_STATUS rc;
   GT_U32                              tc4PfcToEgfIndex[CPSS_TC_RANGE_CNS][PRV_CPSS_DXCH_TC4PFC_TO_EGF_INDEX_ARR_SIZE_CNS];
   GT_U32 arraySize;


   arraySize = sizeof(GT_U32)*CPSS_TC_RANGE_CNS*PRV_CPSS_DXCH_TC4PFC_TO_EGF_INDEX_ARR_SIZE_CNS;

   cpssOsMemCpy(tc4PfcToEgfIndex,PRV_CPSS_DXCH_PP_MAC(devNum)->tc4PfcToEgfIndex,arraySize);
   cpssOsMemSet(PRV_CPSS_DXCH_PP_MAC(devNum)->tc4PfcToEgfIndex,0,arraySize);


   rc =prvCpssDxChTxqtc4PfcToEgfIndexCatchUp(devNum);
   if(rc !=GT_OK)
   {
      return rc;
   }

   for(i=0;i<CPSS_TC_RANGE_CNS;i++)
   {
     for(j=0;j<PRV_CPSS_DXCH_TC4PFC_TO_EGF_INDEX_ARR_SIZE_CNS;j++)
     {
        if(tc4PfcToEgfIndex[i][j]!=PRV_CPSS_DXCH_PP_MAC(devNum)->tc4PfcToEgfIndex[i][j])
        {
            cpssOsPrintf("Missmatch at tc4PfcToEgfIndex[%d][%d]  Saved 0x%x vs Catched 0x%x\n",i,j,
                 tc4PfcToEgfIndex[i][j],PRV_CPSS_DXCH_PP_MAC(devNum)->tc4PfcToEgfIndex[i][j]);
        }
     }
   }
   return GT_OK;
}


/**
* @internal  prvCpssDxChTxqSyncSwHwForHa  function
* @endinternal
*
* @brief   Perform catch up of  long queue and length adjust profiles for all devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChTxqSyncSwHwForHa
(
    GT_VOID
)
{
    GT_U8 devNum;
    GT_STATUS rc;

    for (devNum = 0; devNum < PRV_CPSS_MAX_PP_DEVICES_CNS; devNum++)
    {
        CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        if ( (PRV_CPSS_PP_CONFIG_ARR_MAC[devNum] == NULL) ||
             (PRV_CPSS_DXCH_FAMILY_CHECK_MAC(devNum) == 0) || (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE))
        {
            CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            continue;
        }

        rc = prvCpssDxChTxqCatchUp(devNum);

        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


GT_STATUS prvCpssDxChTxqPdsCatchUpValidationFunction
(
    IN GT_U8 devNum,
    IN GT_U32 tile,
    IN GT_U32 dp
)
{
    GT_U32 i,j,longQueueCompare,lengthAdjustCompare,tmp,shift,portNum;
    GT_STATUS rc;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC profileFromHw;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC lengthAdjustParameters;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC        longQueueParameters;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE  *aNodePtr;
    GT_U32                           profileIndexFromHw,bindedNumFromHw[2];

    profileFromHw.lengthAdjustParametersPtr = &lengthAdjustParameters;
    profileFromHw.longQueueParametersPtr=&longQueueParameters;

    cpssOsPrintf("\n+----------+-------+------+-------------------+-------------------+");
    cpssOsPrintf("\n| Profile  |length | queue|binded ports num sw|binded ports num hw|");
    cpssOsPrintf("\n+----------+-------+------+-------------------+-------------------+");

    for(i=0;i<CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM;i++)
    {
        rc = prvCpssDxChTxqFalconPdsProfileGet(devNum,tile,dp,i,GT_FALSE,&profileFromHw);

        if(rc!=GT_OK)
        {
            cpssOsPrintf("%d = prvCpssDxChTxqFalconPdsProfileGet\n",rc );
            return rc;
        }

        /*compare to SW*/
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].lengthAdjustParametersPtr)
        {
            lengthAdjustCompare = cpssOsMemCmp(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].lengthAdjustParametersPtr,
                profileFromHw.lengthAdjustParametersPtr,sizeof(PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC));
        }
        else if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE &&
           (PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[i].lengthAdjustParametersPtr))
        {
           lengthAdjustCompare = cpssOsMemCmp(PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[i].lengthAdjustParametersPtr,
                profileFromHw.lengthAdjustParametersPtr,sizeof(PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC));
        }
        else
        {
            lengthAdjustCompare =0xFFFFFFFF;
        }

        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].longQueueParametersPtr)
        {
            longQueueCompare =cpssOsMemCmp(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].longQueueParametersPtr,
                            profileFromHw.longQueueParametersPtr,sizeof(PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC));
        }
        else
        {
            longQueueCompare =0xFFFFFFFF;
        }

        bindedNumFromHw[0] =0;

         for(j=0;j<32;j++)
         {
             tmp= PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].bindedPortsBmp[j];
             if(tmp )
             {
                 shift =0;
                 while(tmp)
                 {
                     if(tmp&0x1)
                     {
                         portNum = j*32+shift;

                         /*Get first queue number*/
                         rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&aNodePtr);
                         if(rc !=GT_OK)
                         {
                            return rc;
                         }
                         /*Now check to which PDS profile it is binded*/
                         rc = prvCpssDxChTxqFalconPdsQueueProfileMapGet(devNum,aNodePtr->queuesData.tileNum,
                            aNodePtr->queuesData.dp,aNodePtr->queuesData.queueBase,&profileIndexFromHw);
                         if(rc !=GT_OK)
                         {
                            return rc;
                         }

                         if(i==profileIndexFromHw)
                         {
                            bindedNumFromHw[0]++;
                         }

                     }
                     tmp>>=1;
                     shift++;
                 }
             }
         }

        bindedNumFromHw[1] =0;

        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
        {
             for(j=0;j<32;j++)
             {
                 tmp= PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[i].bindedPortsBmp[j];
                 if(tmp )
                 {
                     shift =0;
                     while(tmp)
                     {
                         if(tmp&0x1)
                         {
                             portNum = j*32+shift;

                             /*Get first queue number*/
                             rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&aNodePtr);
                             /*There may be unmapped ports*/
                             if(rc ==GT_OK)
                             {
                                /*Now check to which PDS profile it is binded*/
                                 rc = prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapGet(devNum,aNodePtr->queuesData.tileNum,
                                    aNodePtr->queuesData.dp,aNodePtr->queuesData.queueBase,&profileIndexFromHw);
                                 if(rc !=GT_OK)
                                 {
                                    return rc;
                                 }

                                 if(i==profileIndexFromHw)
                                 {
                                    bindedNumFromHw[1]++;
                                 }

                             }

                         }
                         tmp>>=1;
                         shift++;
                     }
                 }
             }

        }

        cpssOsPrintf("\n|%9d|%8c|%6c|que %4d | len %4d|que %4d | len %4d|",i,(lengthAdjustCompare==0)?'V':(lengthAdjustCompare==0xFFFFFFFF)?'?':'X',
            (longQueueCompare==0)?'V':(longQueueCompare==0xFFFFFFFF)?'?':'X',
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].numberOfBindedPorts,
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[i].numberOfBindedPorts,
            bindedNumFromHw[0], bindedNumFromHw[1]);
        cpssOsPrintf("\n+----------+-------+------+-------------------+-------------------+");


    }

    cpssOsPrintf("\n");

    return GT_OK;
}



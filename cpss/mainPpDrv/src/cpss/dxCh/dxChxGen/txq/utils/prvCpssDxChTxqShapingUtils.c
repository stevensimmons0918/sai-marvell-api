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
* @file prvCpssDxChTxqShapingUtils.c
*
* @brief CPSS SIP6 TXQ  shaping operation functions
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqSearchUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
 * @internal prvCpssFalconTxqUtilsShapingParametersGet function
 * @endinternal
 *
 * @brief   Convert physicalPortNum into scheduler node and read shaping parameters from specific node(SW)
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] physicalPortNum          -physical port number        -
 * @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
 * @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
 *
 * @param[out] burstSizePtr             - burst size in units of 4K bytes
 *                                      (max value is 4K which results in 16K burst size)
 * @param[out] maxRatePtr               -(pointer to) the requested shaping Rate value in Kbps.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqUtilsShapingParametersGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 queueNumber,
    IN GT_BOOL perQueue,
    IN PRV_QUEUE_SHAPING_ACTION_ENT rateType,
    OUT GT_U16                              *burstSizePtr,
    OUT GT_U32                              *maxRatePtr
)
{
    GT_U32                  tileNum;
    GT_U32                  nodeIndex, qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr;
    GT_BOOL                 isCascade = GT_FALSE;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr = NULL;
    GT_STATUS               rc = GT_OK;
    PRV_CPSS_PDQ_LEVEL_ENT level;


   rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,physicalPortNum,&isCascade,&pNodePtr);
   if (rc!=GT_OK)
   {
       return rc;
   }


    if(GT_FALSE == isCascade)
    {
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &nodeIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        level =PRV_CPSS_PDQ_LEVEL_A_E;
    }
    else
    {
        nodeIndex = pNodePtr->pNodeIndex;
        /*P[i]=C[i]=B[i]*/
        level =PRV_CPSS_PDQ_LEVEL_B_E;
        tileNum = pNodePtr->aNodelist[0].queuesData.tileNum;
    }

    if (perQueue == GT_FALSE)
    {
        rc = prvCpssSip6TxqPdqShapingRateFromHwGet(devNum, tileNum, nodeIndex, level, rateType, burstSizePtr, maxRatePtr);
        if (rc)
        {
            return rc;
        }
    }
    else
    {
        if(GT_TRUE==isCascade)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Q level shaping can not be configured on cascade port\n");
        }

        /*Find the queue index*/
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr);
        if (rc)
        {
            return rc;
        }
        qNodeIndex  = aNodePtr->queuesData.pdqQueueFirst + queueNumber;
        rc      = prvCpssSip6TxqPdqShapingRateFromHwGet(devNum, tileNum, qNodeIndex, PRV_CPSS_PDQ_LEVEL_Q_E, rateType, burstSizePtr, maxRatePtr);
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqUtilsShapingParametersSet function
 * @endinternal
 *
 * @brief   Convert physicalPortNum into scheduler node and write shaping parameters to specific node
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] physicalPortNum          -physical port number        -
 * @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
 * @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
 * @param[in] shapingActionType    -token bucket type to configure
 * @param[in] burstSize                - burst size in units of 4K bytes
 *                                      (max value is 4K which results in 16K burst size)
 * @param[in,out] maxRatePtr               - (pointer to)Requested Rate in Kbps
 * @param[in,out] maxRatePtr               -(pointer to) the actual Rate value in Kbps.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqUtilsShapingParametersSet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 queueNumber,
    IN GT_BOOL perQueue,
    IN PRV_QUEUE_SHAPING_ACTION_ENT shapingActionType,
    IN GT_U16 burstSize,
    INOUT GT_U32                              *maxRatePtr
)
{
    GT_U32                  tileNum,i;
    GT_U32                  aNodeIndex, qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr[2]={NULL,NULL};
    GT_STATUS               rc = GT_OK;
    GT_BOOL                 preemptionEnabled = GT_FALSE;
    GT_BOOL                 actAsPreemptiveChannel = GT_FALSE;
    GT_BOOL                 isCascade = GT_FALSE;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr = NULL;

    /*check  if this is cascade port*/
    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,physicalPortNum,&isCascade,&pNodePtr);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if(GT_FALSE==isCascade)
    {
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &aNodeIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssSip6TxqUtilsPreemptionStatusGet(devNum,physicalPortNum,&preemptionEnabled,&actAsPreemptiveChannel,NULL,NULL);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc , "prvCpssSip6TxqUtilsPreemptionStatusGet failed for port %d\n", physicalPortNum);
        }

        if(GT_TRUE==actAsPreemptiveChannel)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Port %d act as preemptive channel.\n", physicalPortNum);
        }
    }
    else
    {
        tileNum = pNodePtr->aNodelist[0].queuesData.tileNum;
    }
    if (perQueue == GT_FALSE)
    {
        if(GT_TRUE==preemptionEnabled)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Shaping can not be configured on port %d due to preemption\n", physicalPortNum);
        }

        rc = prvCpssFalconTxqPdqShapingOnNodeSet(devNum, tileNum,GT_TRUE==isCascade?pNodePtr->pNodeIndex:aNodeIndex,
            GT_TRUE==isCascade?PRV_CPSS_PDQ_LEVEL_B_E :PRV_CPSS_PDQ_LEVEL_A_E, burstSize, maxRatePtr);
        if (rc)
        {
            return rc;
        }
    }
    else
    {
        /*Find the queue index*/
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &(aNodePtr[0]));
        if (rc)
        {
            return rc;
        }
        if(GT_TRUE==preemptionEnabled)
        {
            rc = prvCpssDxChTxqSip6PreeptivePartnerAnodeGet(devNum, aNodePtr[0],&(aNodePtr[1]));
            if (rc != GT_OK)
            {
                return rc;
            }
        }


         /*Duplicate to preemptive channel*/
            for(i=0;i<2;i++)
            {
                if(NULL!=aNodePtr[i])
                {
                    qNodeIndex = aNodePtr[i]->queuesData.pdqQueueFirst + queueNumber;
                    if (shapingActionType == PRV_QUEUE_SHAPING_ACTION_PRIORITY_DOWNGRADE_ENT)
                    {
                        rc = prvCpssFalconTxqPdqMinBwOnNodeSet(devNum, tileNum, qNodeIndex, burstSize, maxRatePtr);
                        if(rc==GT_OK)
                        {
                            if(i==0)
                            {
                                    /*save only 8 queues*/
                                if(queueNumber<PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
                                {
                                    /*save for restore*/
                                    aNodePtr[i]->queuesData.restoreData[queueNumber].cir = *maxRatePtr;
                                    aNodePtr[i]->queuesData.restoreData[queueNumber].burstCir= burstSize;
                                }
                             }
                        }
                    }
                    else
                    {
                        rc = prvCpssFalconTxqPdqShapingOnNodeSet(devNum, tileNum, qNodeIndex, PRV_CPSS_PDQ_LEVEL_Q_E, burstSize, maxRatePtr);
                        if(rc==GT_OK)
                        {
                            if(i==0)
                            {
                                    /*save only 8 queues*/
                                if(queueNumber<PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
                                {
                                    /*save for restore*/
                                    aNodePtr[i]->queuesData.restoreData[queueNumber].eir = *maxRatePtr;
                                    aNodePtr[i]->queuesData.restoreData[queueNumber].burstEir= burstSize;
                                }
                             }
                        }
                    }
                    if (rc)
                    {
                        return rc;
                    }
                }
            }

    }
    return rc;
}
/**
 * @internal prvCpssSip6TxqUtilsShapingEnableSet function
 * @endinternal
 *
 * @brief   Convert physicalPortNum into scheduler node and Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] physicalPortNum          -physical port number        -
 * @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
 * @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
 * @param[in] enable                   - GT_TRUE,  Shaping
 *                                      GT_FALSE, disable Shaping
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssSip6TxqUtilsShapingEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 queueNumber,
    IN GT_BOOL perQueue,
    IN GT_BOOL enable
)
{
    GT_U32                  tileNum,i;
    GT_U32                  aNodeIndex, qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr[2]={NULL,NULL};
    GT_STATUS               rc = GT_OK;
    GT_BOOL                 preemptionEnabled = GT_FALSE;
    GT_BOOL                 actAsPreemptiveChannel = GT_FALSE;
    GT_BOOL                 isCascade = GT_FALSE;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr = NULL;


    rc = prvCpssSip6TxqUtilsPreemptionStatusGet(devNum,physicalPortNum,&preemptionEnabled,&actAsPreemptiveChannel,NULL,NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc , "prvCpssSip6TxqUtilsPreemptionStatusGet failed for port %d\n", physicalPortNum);
    }


    if(GT_TRUE==actAsPreemptiveChannel)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Port %d act as preemptive channel.\n", physicalPortNum);
    }

    /*check  if this is cascade port*/
    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,physicalPortNum,&isCascade,&pNodePtr);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if(GT_TRUE==isCascade)
    {
       if(GT_TRUE== perQueue)
       {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Q level shaping can not be configured on cascade port\n");
       }
       /*P node index is equal to B node idex*/
       rc = prvCpssFalconTxqPdqShapingEnableSet(devNum, pNodePtr->aNodelist[0].queuesData.tileNum,
                pNodePtr->pNodeIndex, PRV_CPSS_PDQ_LEVEL_B_E, enable);
       if (rc != GT_OK)
       {
           return rc;
       }
    }
    else
    {
        /*chek that port is mapped*/
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &aNodeIndex);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (perQueue == GT_FALSE)
        {
         if(GT_TRUE==preemptionEnabled)
         {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Shaping can not be configured on port %d due to preemption\n", physicalPortNum);
         }

         rc = prvCpssFalconTxqPdqShapingEnableSet(devNum, tileNum, aNodeIndex, PRV_CPSS_PDQ_LEVEL_A_E, enable);
         if (rc != GT_OK)
         {
             return rc;
         }
        }
        else
        {
            /*Find the queue index*/
            rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &(aNodePtr[0]));
            if (rc)
            {
                return rc;
            }
            /*Duplicate to preemptive channel*/
            if(GT_TRUE==preemptionEnabled)
            {
                rc = prvCpssDxChTxqSip6PreeptivePartnerAnodeGet(devNum, aNodePtr[0],&(aNodePtr[1]));
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            for(i=0;i<2;i++)
            {
               if(NULL!=aNodePtr[i])
               {
                   qNodeIndex = aNodePtr[i]->queuesData.pdqQueueFirst + queueNumber;
                   rc      = prvCpssFalconTxqPdqShapingEnableSet(devNum, tileNum, qNodeIndex, PRV_CPSS_PDQ_LEVEL_Q_E, enable);
                   if(rc==GT_OK)
                   {
                       if(i==0)
                       {
                                  /*save only 8 queues*/
                          if(queueNumber<PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
                          {
                               /*save for restore*/
                               aNodePtr[i]->queuesData.restoreData[queueNumber].eirEnable = enable;
                          }
                        }
                   }

                   if (rc)
                   {
                       return rc;
                   }
               }
             }
        }
     }
    return rc;
}

/**
 * @internal prvCpssSip6TxqUtilsShapingEnableGet function
 * @endinternal
 *
 * @brief   Convert physicalPortNum into scheduler node and get Enable/Disable Token Bucket rate shaping on specified port or queue of specified device.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] physicalPortNum          -physical port number        -
 * @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
 * @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
 *
 * @param[out] enablePtr                - GT_TRUE, enable Shaping
 *                                      GT_FALSE, disable Shaping
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssSip6TxqUtilsShapingEnableGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 queueNumber,
    IN GT_BOOL perQueue,
    IN GT_BOOL  *  enablePtr
)
{
    GT_U32                  tileNum;
    GT_U32                  aNodeIndex, qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr;
    GT_STATUS               rc = GT_OK;
    GT_BOOL                 isCascade = GT_FALSE;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr = NULL;

    /*check  if this is cascade port*/
    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,physicalPortNum,&isCascade,&pNodePtr);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if(GT_TRUE==isCascade)
    {
       if(GT_TRUE== perQueue)
       {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Q level shaping can not be configured on cascade port\n");
       }
       /*P node index is equal to B node idex*/
       rc = prvCpssFalconTxqPdqShapingEnableGet(devNum, pNodePtr->aNodelist[0].queuesData.tileNum,
                pNodePtr->pNodeIndex, PRV_CPSS_PDQ_LEVEL_B_E, enablePtr);

       return rc;
    }

    rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &aNodeIndex);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (perQueue == GT_FALSE)
    {
        rc = prvCpssFalconTxqPdqShapingEnableGet(devNum, tileNum, aNodeIndex, PRV_CPSS_PDQ_LEVEL_A_E, enablePtr);
        if (rc)
        {
            return rc;
        }
    }
    else
    {
        /*Find the queue index*/
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr);
        if (rc)
        {
            return rc;
        }
        qNodeIndex  = aNodePtr->queuesData.pdqQueueFirst + queueNumber;
        rc      = prvCpssFalconTxqPdqShapingEnableGet(devNum, tileNum, qNodeIndex, PRV_CPSS_PDQ_LEVEL_Q_E, enablePtr);
        if (rc)
        {
            return rc;
        }
    }
    return rc;
}

/**
 * @internal prvCpssFalconTxqUtilsMinimalBwEnableGet function
 * @endinternal
 *
 * @brief   Convert physicalPortNum into scheduler node and get
 *             Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
 *             Once token bucket is empty the priority is lowered to lowest.
 *
 * @note   APPLICABLE DEVICES:        Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:    xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] physicalPortNum          -physical port number        -
 * @param[in] queueNumber              - Queue offset from base queue mapped to the current port
 * @param[in] enablePtr                   - (pointer to)GT_TRUE,  Set lowest  priority once token bucket is empty
 *                                                       GT_FALSE, otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqUtilsMinimalBwEnableGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 queueNumber,
    OUT GT_BOOL    *enablePtr
)
{
    GT_U32                  tileNum;
    GT_U32                  aNodeIndex, qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr;
    GT_STATUS               rc = GT_OK;
    /*chek that port is mapped*/
    rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &aNodeIndex);
    if (rc != GT_OK)
    {
        return rc;
    }
    /*Find the queue index*/
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr);
    if (rc)
    {
        return rc;
    }
    qNodeIndex  = aNodePtr->queuesData.pdqQueueFirst + queueNumber;
    rc      = prvCpssFalconTxqPdqMinBwEnableGet(devNum, tileNum, qNodeIndex, enablePtr);
    if (rc)
    {
        return rc;
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqUtilsMinimalBwEnableSet function
 * @endinternal
 *
 * @brief   Convert physicalPortNum into scheduler node and
 *             Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
 *             Once token bucket is empty the priority is lowered to lowest.
 *
 * @note   APPLICABLE DEVICES:        Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:    xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] physicalPortNum          -physical port number        -
 * @param[in] queueNumber              - Queue offset from base queue mapped to the current port
 * @param[in] enable                   - GT_TRUE,  Set lowest  priority once token bucket is empty
 *                                      GT_FALSE, otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqUtilsMinimalBwEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 queueNumber,
    IN GT_BOOL enable
)
{
    GT_U32                  tileNum,i;
    GT_U32                  aNodeIndex, qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr[2]={NULL,NULL};
    GT_STATUS               rc = GT_OK;
    GT_BOOL                 preemptionEnabled,actAsPreemptiveChannel;

    /*chek that port is mapped*/
    rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &aNodeIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssSip6TxqUtilsPreemptionStatusGet(devNum,physicalPortNum,&preemptionEnabled,&actAsPreemptiveChannel,NULL,NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc , "prvCpssSip6TxqUtilsPreemptionStatusGet failed for port %d\n", physicalPortNum);
    }

    if(GT_TRUE==actAsPreemptiveChannel)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Port %d act as preemptive channel.\n", physicalPortNum);
    }
    /*Find the A node */
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr[0]);
    if (rc)
    {
        return rc;
    }

    /*Duplicate to preemptive channel*/
    if(GT_TRUE==preemptionEnabled)
    {
        rc = prvCpssDxChTxqSip6PreeptivePartnerAnodeGet(devNum, aNodePtr[0],&(aNodePtr[1]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    for(i=0;i<2;i++)
    {
       if(NULL!=aNodePtr[i])
       {
           qNodeIndex = aNodePtr[i]->queuesData.pdqQueueFirst + queueNumber;

           rc      = prvCpssFalconTxqPdqMinBwEnableSet(devNum, tileNum, qNodeIndex,enable);
           if (rc)
           {
               return rc;
           }
           if(rc==GT_OK)
           {
               if(i==0)
               {
                      /*save only 8 queues*/
                  if(queueNumber<PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
                  {
                    /*save for restore*/
                    aNodePtr[i]->queuesData.restoreData[queueNumber].cirEnable = enable;
                   }
                }
           }
       }
     }

    return rc;
}


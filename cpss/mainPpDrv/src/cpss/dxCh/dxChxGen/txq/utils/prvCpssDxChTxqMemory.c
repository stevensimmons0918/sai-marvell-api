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
* @file prvCpssDxChTxqMemory.c
*
* @brief CPSS SIP6 TXQ  memory operation functions
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
 * @internal prvCpssSip6TxqUtilsDevClose function
 * @endinternal
 *
 * @brief  Release dynamic memory allocation for TxQ scheduler
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -         `                          physical device number
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssSip6TxqUtilsDevClose
(
    IN GT_U8 devNum
)
{
    GT_STATUS   rc;
    GT_U32      i,j, numberOfTiles;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION *tilePtr;

    PRV_CPSS_NUM_OF_TILE_GET_MAC(devNum, numberOfTiles);
    for (i = 0; i < numberOfTiles; i++)
    {
        rc = prvCpssFalconTxqPdqClose(devNum, i);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPdqClose failed for tile %d", i);
        }
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr == NULL)
        {
            continue;
        }
        tilePtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[i]);

        for(j=0;j<tilePtr->mapping.size;j++)
        {
            if(NULL!=tilePtr->mapping.pNodeMappingConfiguration[j])
            {
                FREE_PTR_MAC(tilePtr->mapping.pNodeMappingConfiguration[j]->aNodelist);
                FREE_PTR_MAC(tilePtr->mapping.pNodeMappingConfiguration[j]);
            }
        }

        FREE_PTR_MAC(tilePtr->mapping.pNodeMappingConfiguration);
        FREE_PTR_MAC(tilePtr->mapping.searchTable.aNodeIndexToPnodeIndex);
        FREE_PTR_MAC(tilePtr->mapping.searchTable.qNodeIndexToAnodeIndex);
        FREE_PTR_MAC(tilePtr->mapping.searchTable.goqToPhyPortIndex);
    }
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr);
    return GT_OK;
}


/**
 * @internal prvCpssDxChTxqSip6InitAnode function
 * @endinternal
 *       NONE
 *
 * @brief   Initialize A  Node structure
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman;AC3X;
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;  Bobcat3; Aldrin2.
 *
 * @param[in] aNodePtr           - pointer to A node structure
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR          - NULL pointer used
 */
static GT_STATUS  prvCpssDxChTxqSip6InitAnode
(
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * aNodePtr
)
{
        GT_U32 i;
    CPSS_NULL_PTR_CHECK_MAC(aNodePtr);

    aNodePtr->aNodeIndex            = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    aNodePtr->physicalPort          = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    aNodePtr->queuesData.queueGroupIndex    = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    aNodePtr->queuesData.semiEligBmp = 0;
    aNodePtr->queuesData.dataPathState = PRV_CPSS_PDQ_A_NODE_DATA_PATH_STATE_TXQ_PORT_ENABLED_ENT;/*mapped ports are enabled*/
    for(i=0;i<PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC;i++)
    {
        aNodePtr->queuesData.restoreData[i].cirEnable= GT_FALSE;
        aNodePtr->queuesData.restoreData[i].cir= CPSS_SIP6_TXQ_INVAL_DATA_CNS;
        aNodePtr->queuesData.restoreData[i].eir= CPSS_SIP6_TXQ_INVAL_DATA_CNS;
        aNodePtr->queuesData.restoreData[i].eirEnable= GT_FALSE;
        aNodePtr->queuesData.restoreData[i].burstEir = 0xFF;
        aNodePtr->queuesData.restoreData[i].burstCir = 0xFF;
    }
    /*default profile*/
    aNodePtr->restoreSchedProfile = CPSS_PORT_TX_SCHEDULER_PROFILE_3_E;
    aNodePtr->qGlobalOffset = CPSS_SIP6_TXQ_INVAL_DATA_CNS;

    aNodePtr->preemptionActivated = GT_FALSE;
    aNodePtr->preemptionType = PRV_DXCH_TXQ_SIP6_PREEMTION_A_NODE_TYPE_NONE_E;
    aNodePtr->implicitAllocation = GT_FALSE;
    aNodePtr->partnerIndex = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    return GT_OK;
}
/**
 * @internal prvCpssDxChTxqSip6InitPnode function
 * @endinternal
 *       NONE
 *
 * @brief   Initialize P  Node structure
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman;AC3X;
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;  Bobcat3; Aldrin2.
 *
 * @param[in] pNodePtr           - pointer to P node structure
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR          - NULL pointer used
 */
static GT_STATUS  prvCpssDxChTxqSip6InitPnode
(
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pNodePtr,
    GT_U32                           numberOfAnodes
)
{
    GT_U32      i;
    GT_STATUS   rc;
    CPSS_NULL_PTR_CHECK_MAC(pNodePtr);
    pNodePtr->aNodeListSize     = 0;
    pNodePtr->sdqQueueFirst     = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    pNodePtr->sdqQueueLast      = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    pNodePtr->isCascade     = GT_FALSE;
    pNodePtr->cascadePhysicalPort   = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    pNodePtr->pNodeIndex = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    pNodePtr->alertCounters.macFifoDrainFailureCount =0;
    pNodePtr->alertCounters.txQDrainFailureCount = 0;

    pNodePtr->aNodelist = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE)*numberOfAnodes);
    if(NULL == pNodePtr->aNodelist)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    for (i = 0; i < numberOfAnodes; i++)
    {
        rc = prvCpssDxChTxqSip6InitAnode(&(pNodePtr->aNodelist[i]));
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}


/**
 * @internal prvCpssDxChTxqSip6PnodeAllocateAndInit function
 * @endinternal
 *       NONE
 *
 * @brief   Allocate Initialize P  Node structure
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman;AC3X;
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;  Bobcat3; Aldrin2.
 *
 * @param[in] pNodePtrPtr                   - pointer to pointer toP node structure
  * @param[in] numberOfAnodes           -number of A nodes to initilize
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR          - NULL pointer used
  * @retval GT_OUT_OF_CPU_MEM          -memory allocation failure
 */
GT_STATUS  prvCpssDxChTxqSip6PnodeAllocateAndInit
(
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE ** pNodePtrPtr,
    GT_U32                           numberOfAnodes
)
{
    GT_STATUS rc = GT_OK;

   if( NULL==pNodePtrPtr)
   {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
   }

     /*avoid memory leak ,release memory if already allocated*/
    if(*pNodePtrPtr)
    {
        cpssOsFree(*pNodePtrPtr);
    }

    *pNodePtrPtr = (PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE ));
    if( NULL==pNodePtrPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChTxqSip6InitPnode(*pNodePtrPtr,numberOfAnodes);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
 * @internal prvCpssDxChTxqSip6PnodeArrayAllocateAndInit function
 * @endinternal
 *       NONE
 *
 * @brief   Allocate Initialize P  Node array
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman;AC3X;
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;  Bobcat3; Aldrin2.
 *
 * @param[in] pNodePtrPtrPtr                   - pointer to pointer to pointer to P node structure
 * @param[in] numberOfAnodes           -number of A nodes to initilize
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR          - NULL pointer used
  * @retval GT_OUT_OF_CPU_MEM          -memory allocation failure
 */
GT_STATUS  prvCpssDxChTxqSip6PnodeArrayAllocateAndInit
(
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *** pNodePtrPtrPtr,
    GT_U32                             arraySize
)
{
    if(0!=arraySize)
    {
          /*avoid memory leak ,release memory if already allocated*/
         FREE_PTR_MAC(*pNodePtrPtrPtr);

         *pNodePtrPtrPtr= (PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE **)cpssOsMalloc(arraySize*sizeof(PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *));
         if( NULL==*pNodePtrPtrPtr)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
         }
         cpssOsMemSet(*pNodePtrPtrPtr,0,(arraySize*sizeof(PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *)));
    }

     return GT_OK;
}


/**
 * @internal prvCpssDxChTxqSip6SearchArrayAllocateAndInit function
 * @endinternal
 *       NONE
 *
 * @brief   Allocate and initialize  search array
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman;AC3X;
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;  Bobcat3; Aldrin2.
 *
 * @param[in] pNodePtrPtrPtr                   - pointer to pointer to pointer to P node structure
 * @param[in] numberOfAnodes           -number of A nodes to initilize
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR          - NULL pointer used
  * @retval GT_OUT_OF_CPU_MEM          -memory allocation failure
 */

GT_STATUS  prvCpssDxChTxqSip6SearchArrayAllocateAndInit
(
    GT_U32                             ** searchArrayPtrPtr,
    GT_U32                             *localMappingPtr,
    GT_U32                             arraySize
)
{
     CPSS_NULL_PTR_CHECK_MAC(searchArrayPtrPtr);

     if(0!=arraySize)
     {
         /*avoid memory leak ,release memory if already allocated*/
         FREE_PTR_MAC(*searchArrayPtrPtr);

         *searchArrayPtrPtr= (GT_U32 *)cpssOsMalloc(arraySize*sizeof(GT_U32));
         if( NULL==*searchArrayPtrPtr)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
         }
         cpssOsMemCpy(*searchArrayPtrPtr,localMappingPtr,(arraySize*sizeof(GT_U32)));
     }

     return GT_OK;
}




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
 * @file prvCpssDxChTxqMain.c
 *
 * @brief CPSS SIP6 TXQ high level configurations.
 *
 * @version   1
 ********************************************************************************
 */
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPdx.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPsi.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPdq.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqQfc.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPds.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPfcc.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpssCommon/private/prvCpssEmulatorMode.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqFcGopUtils.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxDba.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceFalcon.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceHawk.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedHwConfigurationInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqDebugUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqMemory.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqSearchUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqTailDropUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqFcUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/*
     Mapping mode that reserve queue gaps at DPs 4-7 .
     Alligned to port mode excel ,with each DP has defined range of queues
 */
#define PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC 0
/*Mapping mode that do not reserve queue gaps. All queues are consequitive , no limitation.*/
#define PRV_CPSS_QUEUE_MAPPING_MODE_CONSEQUTIVE_MAC 1
#define PRV_CPSS_QUEUE_MAPPING_MODE_DEFAULT_MAC PRV_CPSS_QUEUE_MAPPING_MODE_CONSEQUTIVE_MAC
#define PRV_CPSS_DXCH_FALCON_TXQ_DEFAULT_LENGTH_ADJUST_BYTES_MAC 24
#define PRV_CPSS_SEMI_ELIG_DEFAULT_VALUE_MAC      GT_TRUE
#define PRV_CPSS_SEMI_ELIG4DRY_DEFAULT_VALUE_MAC  GT_TRUE
#define PRV_CPSS_QBV_SCAN_ENABLE_DEFAULT_VALUE_MAC      GT_FALSE


#define QUEUES_PER_LOCAL_PORT_DEFAULT_VAL_MAC 8
#define MAX_QUEUES_PER_PREEMPTIVE_LOCAL_PORT_MAC 8
#define MAX_PHY_PORT_PER_PREEMPTIVE_LOCAL_PORT_MAC 1


#define PRV_PB_READ_REQ_FIFO_LIMIT_MAC     5

#define PRV_TXQ_PDS_DEF_PROFILE_MAC PRV_TXQ_PDS_10G_PROFILE_MAC
#define PRV_CPSS_DXCH_TXQ_SIP_6_CREDIT_CNS _4K
#define PRV_CPSS_DXCH_TXQ_SIP_6_10_CREDIT_CNS _2K
#define PRV_CPSS_DXCH_TXQ_SIP_6_15_CREDIT_CNS _1K
#define PRV_CPSS_DXCH_TXQ_SIP_6_20_CREDIT_CNS _4K



#define PRV_CPSS_TXQ_UTILS_CHECK_RESULT_MAC(_rc, _param)                               \
    do                                                     \
    {                                                      \
        if (_rc != GT_OK)                                          \
        {                                                  \
            CPSS_LOG_ERROR_AND_RETURN_MAC(_rc, "%s failed for %s %d\n", __FUNCNAME__,#_param, _param); \
        }                                                  \
    }                                                      \
    while(0);

#define ABS(val) (((val) < 0) ? -(val) : (val))
#define PRV_CPSS_TXQ_Q_LEVEL_L2_CLUSTER_SIZE 256
#define PRV_CPSS_TXQ_A_LEVEL_L2_CLUSTER_SIZE 128


#define PREEMPTIVE_DEVICE_MAX_DP_PER_TILE_CNS 4

/* maximal amount of channels in DP                  */
/* for Hawk was 27, for Ironman needed 57            */
/* 64 is a dimension of per channel registers of DP  */
/* and PCA units in CIder - will always be enough    */
#define PREEMPTIVE_DEVICE_MAX_LOCAL_POR_PER_DP_CNS 64

/*global variables macros*/

#define PRV_NON_SHARED_TXQ_DIR_TXQ_MAIN_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_NON_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.txqDir.txqMainSrc._var,_value)

#define PRV_NON_SHARED_TXQ_DIR_TXQ_MAIN_SRC_GLOBAL_VAR_GET(_var)\
    PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.txqDir.txqMainSrc._var)



/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_PORT_MAP_PARAM_STC
 *
 * @brief SIP 6 mapping structure used for building tree.
*/
typedef struct{

    IN GT_U32                                             tileNum;
    IN GT_U32                                             dataPath;
    IN GT_U32                                             localPort;
    IN GT_U32                                             physicalPortNumber;
    /*number of ports that connected to the same P node as the current port*/
    IN GT_U32                                             numOfPorts;
     /*index in foundIndexes  foundIdxsArray[remotePortsIterator]*/
    GT_U32                                                indexInSource;

} PRV_CPSS_DXCH_TXQ_SIP_6_PORT_MAP_PARAM_STC;

/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_ITERATORS_PARAM_STC
 *
 * @brief SIP 6 iterators structure used for building tree.
*/
typedef struct{
    GT_U32                                          *currentQnumIteratorPtr;
    GT_U32                                          *currentAnodeIteratorPtr;
    GT_U32                                          *qroupOfQIteratorPtr;
    GT_U32                                          *maxGoQPtr;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE                  *currentPnodePtr;
} PRV_CPSS_DXCH_TXQ_SIP_ITERATORS_PARAM_STC;

/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_SEARCH_DB_PARAM_STC
 *
 * @brief SIP 6 search db params  structure used for building tree.
*/
typedef struct{
   GT_U32                                          *aNodeIndexToPnodeIndexArr;
   GT_U32                                          *qNodeIndexToAnodeIndexArr;
   GT_U32                                          *goqToPhyPortIndexArr;
} PRV_CPSS_DXCH_TXQ_SIP_SEARCH_DB_PARAM_STC;


/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_SEARCH_DB_PARAM_STC
 *
 * @brief SIP 6 implicit allocation data base
*/
typedef struct{
   GT_BOOL                                         valid;
   /* physical port number that represent express channel*/
   GT_U32                                          expPhysicalPort;
   /*A node  number that represent express channel*/
   GT_U32                                          expAnode;
   /*Number of requested queues*/
   GT_U32                                          numOfQueues;
} PRV_CPSS_DXCH_TXQ_SIP_IMPLICIT_ALLOCATION_STC;



GT_STATUS prvCpssFalconTxqUtilsGetCascadePort
(
    IN GT_U8 devNum,
    IN GT_U32 remotePortNum,
    IN GT_BOOL remote,
    OUT GT_U32 *casCadePortNumPtr,
    OUT GT_U32 *pNodeIndPtr
);
extern GT_STATUS prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortBaseMapGet
(
    IN GT_U8 devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 physPort,
    OUT GT_U32 *txqPortBasePtr
);


static GT_STATUS prvCpssSip6TxqUtilsMaxNumOfQueuesPerPortGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *maxNumOfQueuesPtr
);

PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * prvCpssSip6TxqUtilsPnodeInstanceGet
(
    GT_U32 devNum,
    GT_U32 tileNum,
    GT_U32 index
)
{
    GT_U32  numOfTiles =0;
    GT_BOOL errorDetected = GT_FALSE;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    /*chek tile*/
    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        case 0:
        case 1:
            numOfTiles = 1;
            break;
       case 2:
       case 4:
            numOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
            break;
        default:
            errorDetected = GT_TRUE;
            break;
    }

    errorDetected = (errorDetected)||(tileNum>=numOfTiles)||(index == CPSS_SIP6_TXQ_INVAL_DATA_CNS);

    if(GT_FALSE== errorDetected)
    {
       tileConfigsPtr  = PRV_CPSS_TILE_HANDLE_GET_MAC(devNum,tileNum);
       if(index<tileConfigsPtr->mapping.size)
       {
         return tileConfigsPtr->mapping.pNodeMappingConfiguration[index];
       }
    }

    return NULL;

}


static GT_STATUS  prvCpssSip6TxqUtilsGlobalQueueIndexesSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    GT_U32                                  i,pdxTableSize;
    PRV_CPSS_DXCH_TXQ_SIP6_SEARCH_STC       *searchTable;
    GT_U32                           globalQueueIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE          *aNodePtr;
    GT_STATUS rc;
    GT_U32    additionalAnodeInd=0,pNodeInd=0;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pNodePtr = NULL;
    GT_U32  phyPortNum;
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    /*Reset index ,this maybe after DB reset*/
    if(tileNum==0)
    {
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.globalQueueIndexIterator = 0;
    }

    globalQueueIndex = PRV_CPSS_DXCH_PP_MAC(devNum)->port.globalQueueIndexIterator;

    tileConfigsPtr  = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum]);
    searchTable = &(tileConfigsPtr->mapping.searchTable);
    pdxTableSize = searchTable->goqToPhyPortSize;

    for(i=0;i<pdxTableSize;i++)
    {
        rc = prvCpssSip6TxqGoQToPhysicalPortGet(devNum,tileNum,i,&phyPortNum);
        if(rc==GT_OK&&phyPortNum!=CPSS_SIP6_TXQ_INVAL_DATA_CNS)
        {
            rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,phyPortNum,&aNodePtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            if(CPSS_SIP6_TXQ_INVAL_DATA_CNS != aNodePtr->qGlobalOffset)
            {
                /*This is implicitly mapped GoQ*/
                additionalAnodeInd =aNodePtr->partnerIndex;

                rc  = prvCpssSip6TxqAnodeToPnodeIndexGet(devNum,tileNum,additionalAnodeInd,&pNodeInd);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqAnodeToPnodeIndexGet failed for index %d",additionalAnodeInd);
                }

                PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pNodePtr,devNum,tileNum,pNodeInd);
                aNodePtr= &(pNodePtr->aNodelist[0]);

                /*sanity*/
                if(aNodePtr->implicitAllocation!=GT_TRUE)
                {
                  CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Anode should be implicitly allocated\n");
                }

            }

            aNodePtr->qGlobalOffset = globalQueueIndex;
            globalQueueIndex+=(aNodePtr->queuesData.pdqQueueLast - aNodePtr->queuesData.pdqQueueFirst+1);
        }
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.globalQueueIndexIterator = globalQueueIndex;

    return GT_OK;
}




GT_U32 prvCpssTxqUtilsCreditSizeGet
(
    IN GT_U8 devNum
)
{
    GT_U32 creditSize;
    if (GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        creditSize = PRV_CPSS_DXCH_TXQ_SIP_6_CREDIT_CNS;
    }
    else if (GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        creditSize = PRV_CPSS_DXCH_TXQ_SIP_6_10_CREDIT_CNS;
    }
      else if (GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        creditSize = PRV_CPSS_DXCH_TXQ_SIP_6_15_CREDIT_CNS;
    }
    else
    {
        creditSize = PRV_CPSS_DXCH_TXQ_SIP_6_20_CREDIT_CNS;
    }
    return creditSize;
}



/**
 * @internal prvCpssFalconTxqClusterSegmentCrossingGet function
 * @endinternal
 *
 * @brief   Dected L2 cluster crossing at scheduler tree
 *
 * @note   APPLICABLE DEVICES:          Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -  physical device number
 * @param[in] startNodeId              - index of first node
 * @param[in] endNodeId               - index of last node
 * @param[in] clusterSize               - size of L2 cluster
 * @param[out] newStartClusterPtr -  index of cluster that contain endNodeId
 * @retval GT_TRUE                    -Cluster segment crossing detected
 * @retval GT_FALSE                    -Cluster segment crossing is not detected
 */
static GT_BOOL prvCpssFalconTxqClusterSegmentCrossingGet
(
    IN GT_U8 devNum,
    IN GT_U32 startNodeId,
    IN GT_U32 endNodeId,
    IN GT_U32 clusterSize,
    OUT GT_U32                   *newStartClusterPtr
);
/**
 * @internal prvCpssFalconTxqPdsProfileGet  function
 * @endinternal
 *
 * @brief   Get the profile according to speed and  length adjust parameters.
 *             If profile does not exist the function create one.
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -              physical device number
 * @param[in] speed_in_G             -              Required speed in Gygabyte
 * @param[in] lengthAdjustParametersPtr  (pointer to)lenght adjust parameters
 * @param[out] profilePtr                             -(pointer to)profile index
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device number, profile set or
 *                                       traffic class
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters is NULL pointer
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
 *                                                contain latency queue.
 */
static GT_STATUS prvCpssFalconTxqPdsProfileGet
(
    IN GT_U8 devNum,
    IN GT_U32 speed_in_G,
    IN PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC *lengthAdjustParametersPtr,
    OUT GT_U32  *profilePtr
);
/**
 * @internal prvCpssFalconTxqUtilsSetDmaToPnodeMapping function
 * @endinternal
 *
 * @brief    Write entry to DMA to Pnode data base
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @param[in] devNum                   -  device number
 * @param[in] dmaNum                  - Global DMA number(0..263).
 * @param[in] tileNum                  - traffic class queue on this device (0..7).
 * @param[in] pNodeNum                  - Index of P node

 */
static GT_STATUS prvCpssFalconTxqUtilsSetDmaToPnodeMapping
(
    IN GT_U8 devNum,
    IN GT_U32 dmaNum,
    IN GT_U32 tileNum,
    IN GT_U32 pNodeNum
);
/**
 * @internal prvCpssFalconTxqUtilsGetDmaToPnodeMapping function
 * @endinternal
 *
 * @brief    Read entry from DMA to Pnode data base
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @param[in] devNum                   -  device number
 * @param[in] dmaNum                  - Global DMA number(0..263).
 * @param[in] tileNum                  - traffic class queue on this device (0..7).
 * @param[out] pNodeNum                  - Index of P node
 * @param[in] isErrorForLog            - indication the CPSS ERROR should be in the LOG or not

 */
static GT_STATUS prvCpssFalconTxqUtilsGetDmaToPnodeMapping
(
    IN GT_U8 devNum,
    IN GT_U32 dmaNum,
    OUT GT_U32   *tileNumPtr,
    OUT GT_U32   * pNodeNumPtr,
    IN GT_BOOL isErrorForLog
);
/*******************************************************************************
* prvCpssFalconTxqUtilsSetDmaToPnodeMapping
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* INPUTS:
*       devNum        -  device number
*       dmaNum       - Global DMA number(0..263).
*       tileNum         - traffic class queue on this device (0..7).
*       aNodeNum   - Index of A node
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong sdq number.
*       GT_HW_ERROR              - on writing to HW error.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_STATUS prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingSet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNumber,
    IN GT_U32 tileNum,
    IN GT_U32 aNodeNum
);

/**
 * @internal prvDxChPortTxQueueProfileHwSet  function
 * @endinternal
 *
 * @brief   Update HW with PDS queue profile
 *
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -              physical device number
 * @param[in] speed_in_G             -              Required speed in Gygabyte
 * @param[in] profileIndex                            index to set
 * @param[in] profilePtr                             -(pointer to)profile attributes
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device number, profile set or
 *                                       traffic class
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters is NULL pointer
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
 *                                                contain latency queue.
 */
static GT_STATUS prvDxChPortTxQueueProfileHwSet
(
    IN GT_U8 devNum,
    IN GT_U32 profileIndex,
    IN PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC * profilePtr
);
/**
 * @internal prvCpssFalconTxqUtilsLowLatencyQueuePresentInProfileGet function
 * @endinternal
 *
 * @brief  Check if pNode contain TC configured to SP
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                                            physical device number
 * @param[in] pNode                                                          p node
 * @param[out] presentPtr                                            (pointer to)GT_TRUE pNode is contain SP queue,GT_FALSE otherwise
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
static GT_STATUS prvCpssFalconTxqUtilsLowLatencyQueuePresentInPnodeGet
(
    IN GT_U8 devNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNode,
    OUT GT_BOOL                        *presentPtr,
    OUT GT_U32                          *queueOffsetPtr
)
{
    GT_STATUS   rc;
    GT_U32      dp, tile;
    GT_U32      i, queueOffset;
    *presentPtr = GT_FALSE;
    dp      = pNode->aNodelist[0].queuesData.dp;
    tile        = pNode->aNodelist[0].queuesData.tileNum;
    for (i = 0; i < pNode->aNodeListSize; i++)
    {
        for (queueOffset = pNode->sdqQueueFirst; queueOffset <= pNode->sdqQueueLast; queueOffset++)
        {
            rc = prvCpssFalconTxqSdqQueueStrictPriorityGet(devNum, tile, dp, queueOffset, presentPtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqSdqQueueStrictPriorityGet failed ");
            }
            if (GT_TRUE == *presentPtr)
            {
                *queueOffsetPtr = queueOffset;
                break;
            }
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsFindPnodeByPhysicalPortNumber function
 * @endinternal
 *
 * @brief  Find P node connected to physical port
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                                            physical device number
 * @param[in] portNum                                                          physical port number
 * @param[out] outPtr                                            (pointer to)p node
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsFindPnodeByPhysicalPortNumber
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE ** outPtr
)
{
    GT_U32                  pNodeIndex, aNodeIndex;
    GT_STATUS               rc;
    GT_U32                  tileNum;
    rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, portNum, &tileNum, &aNodeIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc  = prvCpssSip6TxqAnodeToPnodeIndexGet(devNum,tileNum,aNodeIndex,&pNodeIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqAnodeToPnodeIndexGet failed for index %d",aNodeIndex);
    }

    PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(*outPtr,devNum,tileNum,pNodeIndex);

    return GT_OK;
}
GT_STATUS prvCpssSip6TxqUtilsPnodeIndexGet
(
    GT_U32 devNum,
    GT_U32 dpNum,
    GT_U32 localdpPortNum,
    GT_U32 *pNoteIndPtr
)
{
    /*Verify local port number validity*/
    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localdpPortNum);

    /*Harrier : DP[0],DP[1] hold 16 'used ports' but the 'cpu' is in index 26.*/
    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)&&
        (localdpPortNum ==(CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum)-1)))
    {
       localdpPortNum = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].dataPathNumOfPorts-1;
    }

    *pNoteIndPtr = dpNum + (MAX_DP_IN_TILE(devNum)) * localdpPortNum;
    return GT_OK;
}

/**
 * @internal prvCpssSip6TxqUtilsTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvertWithMuxed function
 * @endinternal
 *
 * @brief   Falcon : convert the local DMA number in the Local DataPath (DP index in tile),
 *         to global DMA number in the device.
 *         the global DMA number must be registered in 'port mapping' at this stage
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   - The PP's device number.
 * @param[in] tileId                   - The tile Id
 * @param[in] localDpIndexInTile       - the Data Path (DP) Index (local DP in the tile !!!)
 * @param[in] localDmaNumInDp          - the DMA local number (local DMA in the DP !!!)
 *
 * @param[out] globalDmaNumPtr          - (pointer to) the DMA global number in the device.
 *                                       GT_OK on success
 */
GT_STATUS prvCpssSip6TxqUtilsTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvertWithMuxed
(
    IN GT_U8 devNum,
    IN GT_U32 tileId,
    IN GT_U32 localDpIndexInTile,
    IN GT_U32 localDmaNumInDp,
    OUT GT_U32  *globalDmaNumPtr
)
{
    GT_STATUS       rc;
    GT_U32          globalDmaNum;
    GT_PHYSICAL_PORT_NUM    physicalPortNum;
    /* get the global DMA number of the port */
    rc = prvCpssFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert(devNum, tileId,
                                        localDpIndexInTile, localDmaNumInDp, &globalDmaNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    do
    {
        rc = prvCpssDxChPortPhysicalPortMapReverseMappingGet(devNum,
                                     PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E, globalDmaNum, &physicalPortNum);
        if (rc == GT_OK)
        {
            *globalDmaNumPtr = globalDmaNum;
            return GT_OK;
        }
        /****************************************************************/
        /* maybe the {dpNum,localPortNum} hold other DMA port number(s) */
        /****************************************************************/
        rc = prvCpssFalconDmaGlobalDmaMuxed_getNext(devNum, &globalDmaNum);
        if (rc != GT_OK)
        {
            break;
        }
    }
    while(1);
    return /* not error for the LOG */ GT_NOT_FOUND;
}
/**
 * @internal prvCpssFalconTxqUtilsFindByLocalPortAndDpInSource function
 * @endinternal
 *
 * @brief   Find index of port in CPSS_DXCH_PORT_MAP_STC by dp index/local port
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] sourcePtr                -pointer to structure that is going to be searched(array of mappings)
 * @param[in] size                     - of structure (Number of ports to map, array size)
 * @param[in] tileNum                  -  Number of the tile (APPLICABLE RANGES:0..4).
 * @param[in] dpNum                    - Number of the dp (APPLICABLE RANGES:0..7).
 * @param[in] localPortNum             - Number of the local port (APPLICABLE RANGES:0..8).
 *
 * @param[out] indexPtr                 - index of found port in sourcePtr
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong tile number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
static GT_STATUS prvCpssFalconTxqUtilsFindByLocalPortAndDpInSource
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_PORT_MAP_STC * sourcePtr,
    IN GT_U32 size,
    IN GT_U32 tileNum,
    IN GT_U32 dpNum,
    IN GT_U32 localPortNum,
    OUT GT_U32 * indexesArr,
    OUT GT_U32 * indexesSizePtr
)
{
    GT_U32      i, j, physicalPorts[PRV_CPSS_DXCH_FALCON_TXQ_MAX_REMOTE_PORT_MAC + 1 /*cascade*/];
    GT_U32      physicalPortsNum = 0;
    GT_STATUS   rc;
    GT_U32      globalDmaNum;
    /* get the global DMA number of the port */
    rc = prvCpssSip6TxqUtilsTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvertWithMuxed(devNum, tileNum,
                                              dpNum, localPortNum, &globalDmaNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssDxChPortPhysicalPortMapReverseDmaMappingGet(devNum, globalDmaNum, &physicalPortsNum, physicalPorts);
    if (rc != GT_OK)
    {
        return rc;
    }
    *indexesSizePtr = 0;
    if (indexesArr)
    {
        for (j = 0; j < size; j++)
        {
            for (i = 0; i < physicalPortsNum; i++)
            {
                if (sourcePtr[j].physicalPortNumber == physicalPorts[i])
                {
                    indexesArr[*indexesSizePtr] = j;
                    (*indexesSizePtr)++;
                }
            }
        }
        if (*indexesSizePtr > PRV_CPSS_DXCH_FALCON_TXQ_MAX_REMOTE_PORT_MAC + 1)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "indexesSizePtr [%d] > %d", *indexesSizePtr, PRV_CPSS_DXCH_FALCON_TXQ_MAX_REMOTE_PORT_MAC + 1);
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget function
 * @endinternal
 *
 * @brief   Find index of port in pNodeMappingConfiguration by dp index/local port
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNumber               - Number of the tile (APPLICABLE RANGES:0..3).
 * @param[in] dpNum                    - Number of the dp (APPLICABLE RANGES:0..7).
 * @param[in] localPortNum             - Number of the local port (APPLICABLE RANGES:0..8).
 * @param[out] indexPtr                - index of found port in pNodeMappingConfiguration
 * @param[in] isErrorForLog            - indication the CPSS ERROR should be in the LOG or not
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong tile number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
 GT_STATUS prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget
(
    IN GT_U8 devNum,
    IN GT_U32 tileNumber,
    IN GT_U32 dpNum,
    IN GT_U32 localPortNum,
    OUT GT_U32 * indexPtr,
    IN GT_BOOL isErrorForLog
)
{
    GT_STATUS   rc;
    GT_U32      globalDmaNum;
    GT_U32      cpuLocalPortNum;
    /*1. Find port RxDma by sdq/local port*/
    /*2. Find Pnode by rxDma*/
    /* get the global DMA number of the port  */

    cpuLocalPortNum = CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum)-1;

    if(localPortNum == cpuLocalPortNum)
    {
        rc = prvCpssSip6TxqUtilsTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvertWithMuxed(devNum, tileNumber,
                                              dpNum, localPortNum, &globalDmaNum);
    }
    else
    {
     rc = prvCpssFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert(devNum, tileNumber,
                                              dpNum, localPortNum, &globalDmaNum);
    }
    if (rc != GT_OK)
    {
        return rc;
    }
    return prvCpssFalconTxqUtilsGetDmaToPnodeMapping(devNum, globalDmaNum, NULL, indexPtr, isErrorForLog);
}
/**
 * @internal prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber function
 * @endinternal
 *
 * @brief   Find dp index/local port of physical port
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] physPort                 - physical port number
 *
 * @param[out] tileNumPtr               - Number of the tile (APPLICABLE RANGES:0..3).
 * @param[out] dpNumPtr                 - Number of the dp (APPLICABLE RANGES:0..7).
 * @param[out] localPortNumPtr - Number of the local port (APPLICABLE RANGES:0..8).
 * @param[out] mappingTypePtr           - mapping type
 *
 * @retval GT_OK                    -           on success.
 * @retval GT_BAD_PARAM             - wrong tile number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physPort,
    OUT GT_U32      * tileNumPtr,
    OUT GT_U32      * dpNumPtr,
    OUT GT_U32      * localdpPortNumPtr,
    OUT CPSS_DXCH_PORT_MAPPING_TYPE_ENT * mappingTypePtr
)
{
    GT_STATUS               rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *   portMapShadowPtr;
    GT_U32                  globalDmaNum;
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physPort, /*OUT*/ &portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (GT_FALSE == portMapShadowPtr->valid)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    globalDmaNum    = portMapShadowPtr->portMap.rxDmaNum;
    rc      = prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert(devNum, globalDmaNum, tileNumPtr, dpNumPtr, localdpPortNumPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (mappingTypePtr)
    {
        *mappingTypePtr = portMapShadowPtr->portMap.mappingType;
    }
    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqUtilsLastQueueIndexForDpGet function
 * @endinternal
 *
 * @brief   Set first  queue index for DP
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                   -tile number
 * @param[in] ,                   -           DP number.
 * @param[out]queueIndex      first valid queue index
 */
static GT_STATUS  prvCpssFalconTxqUtilsStartQueueIndexForDpSet
(
    GT_U8 devNum,
    GT_U32 tileNum,
    GT_U32 dp,
    GT_U32 queueIndex
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(dp);
    if ((queueIndex >= CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Trying to configure start queue %d\n", queueIndex);
    }
    tileConfigsPtr              = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum]);
    tileConfigsPtr->mapping.firstQInDp[dp]  = queueIndex;
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsDpActiveStateGet function
 * @endinternal
 *
 * @brief   Check if this DP has mapped queues
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                   -tile number
 * @param[in]dp                   -           DP number.
 * @param[out]dpActivePtr      (pointer to)GT_TRUE if DP has queue mapped ,GT_FALSE otherwise
 */
static GT_STATUS  prvCpssFalconTxqUtilsDpActiveStateGet
(
    GT_U8 devNum,
    GT_U32 tileNum,
    GT_U32 dp,
    GT_BOOL  *dpActivePtr
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(dp);
    tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum]);
    if (tileConfigsPtr->mapping.firstQInDp[dp] == CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM)
    {
        *dpActivePtr = GT_FALSE;
    }
    else
    {
        *dpActivePtr = GT_TRUE;
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsStartQueueIndexForDpGet function
 * @endinternal
 *
 * @brief   In order to be alligned with port mode excel start queue of each DP is fixed.
 *         This function return first queue for each DP.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 *                                       Start queue number per DP
 */
GT_U32  prvCpssFalconTxqUtilsStartQueueIndexForDpGet
(
    GT_U8 devNum,
    GT_U32 tileNum,
    GT_U32 dp,
    GT_U32  *queueIndexPtr
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    GT_U32                  startIndex;
    GT_BOOL                 mirroredTile = (tileNum % 2) ? GT_TRUE : GT_FALSE;
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum]);
    if (tileConfigsPtr->mapping.mappingMode == PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC)
    {
        if (mirroredTile == GT_FALSE)
        {
            /*Fixed queue start per DP*/
            switch (dp)
            {
            case 0:
                startIndex = 0;
                break;
            case 1:
                startIndex = 72;
                break;
            case 2:
                startIndex = 144;
                break;
            case 3:
                startIndex = 216;
                break;
            case 4:
                startIndex = 288;
                break;
            case 5:
                startIndex = 680;
                break;
            case 6:
                startIndex = 1072;
                break;
            case 7:
                startIndex = 1464;
                break;
            default:/*For last DP*/
                startIndex = 1856;
                break;
            }
        }
        else
        {
            /*Fixed queue start per DP*/
            switch (dp)
            {
            case 0:
                startIndex = 1784;
                break;
            case 1:
                startIndex = 1712;
                break;
            case 2:
                startIndex = 1640;
                break;
            case 3:
                startIndex = 1568;
                break;
            case 4:
                startIndex = 1176;
                break;
            case 5:
                startIndex = 784;
                break;
            case 6:
                startIndex = 392;
                break;
            case 7:
                startIndex = 0;
                break;
            default:/*For last DP*/
                startIndex = 1856;
                break;
            }
        }
    }
    else
    {
        TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(dp);
        startIndex = tileConfigsPtr->mapping.firstQInDp[dp];
        if (startIndex >= CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Bad start queue %d for DP %d\n", startIndex, dp);
        }
    }
    *queueIndexPtr = startIndex;
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsLastQueueIndexForDpGet function
 * @endinternal
 *
 * @brief   Get last valid queue index for DP
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                   -tile number
 * @param[in] ,                   -           DP number.
 * @param[out]queueIndexPtr      (pointer to)last valid queue index
 */
GT_STATUS  prvCpssFalconTxqUtilsLastQueueIndexForDpGet
(
    GT_U8 devNum,
    GT_U32 tileNum,
    GT_U32 dp,
    GT_U32  *queueIndexPtr
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    GT_STATUS               rc;
    GT_U32                  lastIndex;
    GT_BOOL                 mirroredTile = (tileNum % 2) ? GT_TRUE : GT_FALSE;
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum]);
    if (tileConfigsPtr->mapping.mappingMode == PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC)
    {
        if (mirroredTile == GT_FALSE)
        {
            /*Fixed queue start per DP*/
            switch (dp)
            {
            case 0:
                lastIndex = 71;
                break;
            case 1:
                lastIndex = 143;
                break;
            case 2:
                lastIndex = 215;
                break;
            case 3:
                lastIndex = 287;
                break;
            case 4:
                lastIndex = 679;
                break;
            case 5:
                lastIndex = 1071;
                break;
            case 6:
                lastIndex = 1463;
                break;
            case 7:
                lastIndex = 1855;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Requested last queue %d for DP %d\n", dp);
                break;
            }
        }
        else
        {
            /*Fixed queue start per DP*/
            switch (dp)
            {
            case 0:
                lastIndex = 1855;
                break;
            case 1:
                lastIndex = 1783;
                break;
            case 2:
                lastIndex = 1711;
                break;
            case 3:
                lastIndex = 1639;
                break;
            case 4:
                lastIndex = 1567;
                break;
            case 5:
                lastIndex = 1175;
                break;
            case 6:
                lastIndex = 783;
                break;
            case 7:
                lastIndex = 391;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Requested last queue %d for DP %d\n", dp);
                break;
            }
        }
    }
    else
    {   rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum, tileNum, dp, &lastIndex);
        if (rc != GT_OK)
        {
            return rc;
        }
        lastIndex += (CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC - 1); }
    *queueIndexPtr = lastIndex;
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsInitDmaToPnodeDb function
 * @endinternal
 *
 * @brief   Initialize DmaToPnode database
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 */
GT_VOID  prvCpssFalconTxqUtilsInitDmaToPnodeDb
(
    IN GT_U8 devNum
)
{
    GT_U32 i;
    for (i = 0; i < PRV_CPSS_MAX_DMA_NUM_CNS; i++)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[i].pNodeInd   = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[i].tileInd    = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    }
}

GT_VOID  prvCpssFalconTxqUtilsInitPdsLengthAdjust
(
    INOUT PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC *lengthAdjustParametersPtr
)
{
    lengthAdjustParametersPtr->lengthAdjustEnable       = GT_TRUE;
    lengthAdjustParametersPtr->lengthAdjustSubstruct    = GT_FALSE;
    lengthAdjustParametersPtr->lengthAdjustByteCount    =
        PRV_CPSS_DXCH_FALCON_TXQ_DEFAULT_LENGTH_ADJUST_BYTES_MAC;
}
GT_STATUS  prvCpssFalconTxqUtilsInitPdsProfile
(
    IN GT_U8 devNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC *profilePtr,
    IN GT_U32 profileIndex,
    IN GT_BOOL initLengthAdjust
)
{
    GT_U32      i;
    GT_STATUS   rc;
    CPSS_NULL_PTR_CHECK_MAC(profilePtr);
    profilePtr->longQueueParametersPtr  = NULL;
    profilePtr->numberOfBindedPorts     = 0;
    if (GT_TRUE == initLengthAdjust)
    {
        profilePtr->lengthAdjustParametersPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustDb[profileIndex]);
        prvCpssFalconTxqUtilsInitPdsLengthAdjust(profilePtr->lengthAdjustParametersPtr);
        /*Need to init default length adjust explicitly because it will not be initialize on setting speed.
                   In Falcon the default will be initialize at prvCpssDxChTxqBindPortQueuesToPdsProfile*/
        if (GT_TRUE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) &&
            DEFAULT_LENGTH_ADJUST_PROFILE == profileIndex)
        {
            rc = prvDxChPortTxQueueProfileHwSet(devNum, DEFAULT_LENGTH_ADJUST_PROFILE, profilePtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvDxChPortTxQueueProfileHwSet fail");
            }
        }
    }
    else
    {
        profilePtr->lengthAdjustParametersPtr = NULL;
    }
    for (i = 0; i < 32; i++)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[profileIndex].bindedPortsBmp[i]  = 0x0;
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[profileIndex].speed_in_G     = 0;
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsInitPdsProfilesDb function
 * @endinternal
 *
 * @brief   Initialize SW db of PDS profiles.For SIP 6.10 also write the default profile to HW and bind all the ports.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] portMappedToDefault  - number of ports that should be mapped to default
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS  prvCpssFalconTxqUtilsInitPdsProfilesDb
(
    IN GT_U8 devNum,
    IN GT_U32 portMappedToDefault
)
{
    GT_STATUS   rc;
    GT_U32      i, j;

    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    /*Set only tile 0*/
    tileConfigsPtr  = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[0]);

    tileConfigsPtr ->general.numberOfMappedPorts = portMappedToDefault;


    /*init pds profiles*/
    for (i = 0; i < CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM; i++)
    {
        rc = prvCpssFalconTxqUtilsInitPdsProfile(devNum,
                             &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i]), i,
                             PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? GT_FALSE : GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsInitPdsProfile fail");
        }
        /*For  not SIP 6.10 device this function set length adjust pointers to NULL,
                    since length adjust is a logical part of PDS profile*/
        rc = prvCpssFalconTxqUtilsInitPdsProfile(devNum,
                             &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[i]), i,
                             PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? GT_TRUE : GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsInitPdsProfile fail");
        }
        if (i == DEFAULT_LENGTH_ADJUST_PROFILE && PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            for (j = 0; j < 32; j++)
            {
                PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[DEFAULT_LENGTH_ADJUST_PROFILE].bindedPortsBmp[j] = 0xFFFFFFFF;
            }
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles[DEFAULT_LENGTH_ADJUST_PROFILE].numberOfBindedPorts = portMappedToDefault;
        }
    }
    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqUtilsInitDb function
 * @endinternal
 *
 * @brief   Initialize txq database
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] tileConfigsPtr             - pointer to SW  shadow
 * @param[in] tileNumber                 - the number of the tile(APPLICABLE RANGES:0..3)
  * @param[in] numberOfTiles                 - the number of the tiles on device(APPLICABLE RANGES:1..4)
 */
GT_VOID  prvCpssFalconTxqUtilsInitDb
(
    IN GT_U8 devNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr,
    IN GT_U32 tileNumber,
    IN GT_U32 numberOfTiles
)
{
    GT_U32      i;
    const GT_U32 falconPdxInterfaceMap[4][4] =
    {
        { 0, 1, 2, 3 },
        { 1, 0, 3, 2 },
        { 2, 3, 0, 1 },
        { 3, 2, 1, 0 }
    };

    for (i = 0; i < 4; i++)
    { /*No need to configure this for single tile device*/
        tileConfigsPtr->general.pdxMapping[i] = (numberOfTiles == 1) ? 0 : falconPdxInterfaceMap[tileNumber][i];
    }
    for (i = 0; i < CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC; i++)
    {
        tileConfigsPtr->mapping.firstQInDp[i] = CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM;/*Treat max is uninitialized*/
    }
    tileConfigsPtr->headRoomDb.monitorActive    = GT_FALSE;
    tileConfigsPtr->mapping.mappingMode     = PRV_CPSS_QUEUE_MAPPING_MODE_DEFAULT_MAC;
    tileConfigsPtr->mapping.aNodeAllignedMapping    = GT_FALSE;
    tileConfigsPtr->general.txLibInitDone= GT_FALSE;
    tileConfigsPtr->general.poolLimitWaBmp = 0x0;
    tileConfigsPtr->general.addressSpacePtr = NULL;
    tileConfigsPtr->general.burstOptimization = GT_TRUE;
    tileConfigsPtr->general.logShapingResults = GT_FALSE;
    tileConfigsPtr->general.logEnable = GT_FALSE;
    tileConfigsPtr->general.dataPathEventLog =  GT_FALSE;
    tileConfigsPtr->general.schedDbg.schedLogWriteRequests = GT_FALSE;
    tileConfigsPtr->general.schedDbg.schedLogReadRequests = GT_FALSE;

    if(0==tileNumber)
    {
        prvCpssTxqSip6DebugLogInit(devNum);
    }
    if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(
        devNum,PRV_CPSS_DXCH_FALCON_TAIL_DROP_ON_VIRTUAL_BUFFERS_E))
    {
       tileConfigsPtr->general.poolLimitWaBmp = 0x1;
    }

}

static GT_STATUS prvCpssSip6TxqUtilsConnectPnodes
(
    IN GT_U8                                              devNum,
    IN CPSS_DXCH_PORT_MAP_STC                             *sourcePtr,
    IN GT_U32                                             size,
    IN GT_U32                                             tileNum,
    OUT PRV_CPSS_DXCH_TXQ_SIP_IMPLICIT_ALLOCATION_STC     implicitAllocationsArr[][PREEMPTIVE_DEVICE_MAX_LOCAL_POR_PER_DP_CNS],
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION    *targetPtr
)
{

    GT_U32                              i,j;
    GT_STATUS                           rc;
    GT_U32                              foundIdxsArray[PRV_CPSS_DXCH_FALCON_TXQ_MAX_REMOTE_PORT_MAC + 1 /*cascade*/];
    GT_U32                              numOfPorts;
    CPSS_DXCH_DETAILED_PORT_MAP_STC     *portMapShadowPtr;
    GT_U32                              maxPnodeIndex = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    GT_U32                              preChannel;
    GT_U32                              preChannelGlobalDma;
    GT_U32                              currentIndex        = 0;
    GT_BOOL                             preemptionAllowed = GT_FALSE;/*Whether preemption feature is allowed for this device*/


    rc = prvCpssDxChTxqSip6PreemptionFeatureAllowedGet(devNum,&preemptionAllowed);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqSip6PreemptionFeatureAllowedGet  failed .");
    }


    CPSS_TXQ_DEBUG_LOG("Start Pnodes connect for tileNum %d  Source size %d preemptionAllowed %d device %d\n",tileNum,
            size,preemptionAllowed,devNum);

    for (i = 0; (GT_U32)i < CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum); i++)
    {
      for (j = 0; j < MAX_DP_IN_TILE(devNum); j++)
      {
        rc =  prvCpssFalconTxqUtilsFindByLocalPortAndDpInSource(devNum, sourcePtr, size, tileNum, j, i, foundIdxsArray, &numOfPorts);

        if (rc == GT_OK)
        {
            CPSS_TXQ_DEBUG_LOG("Device %d Tile %d Local port %d DP %d contain port\n",devNum,tileNum,i,j);

            rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, sourcePtr[foundIdxsArray[0]].physicalPortNumber, /*OUT*/ &portMapShadowPtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if(GT_TRUE==preemptionAllowed)
            {
                if(portMapShadowPtr->extPortMap.preemptionChannelAllocated)
                {
                     /*check that number of requested queues is in range*/
                     if(sourcePtr[foundIdxsArray[0]].txqPortNumber>MAX_QUEUES_PER_PREEMPTIVE_LOCAL_PORT_MAC)
                     {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Preemptive port support only %d queues\n",
                            MAX_QUEUES_PER_PREEMPTIVE_LOCAL_PORT_MAC);
                     }

                      /*check that number of requested ports is in range*/
                     if(numOfPorts > MAX_PHY_PORT_PER_PREEMPTIVE_LOCAL_PORT_MAC)
                     {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Preemptive port support only %d physical port\n",
                            MAX_PHY_PORT_PER_PREEMPTIVE_LOCAL_PORT_MAC);
                     }

                     rc = prvCpssDxChTxqSip6_10PreChannelGet(devNum, 0, i,&preChannel);
                     if (rc != GT_OK)
                     {
                         return rc;
                     }
                     /*in case preemption is requested for port that can not support preemption,
                                                this should fail here*/

                     if(CPSS_SIP6_TXQ_INVAL_DATA_CNS == preChannel)
                     {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Local port %d does not support preemption\n",i);
                     }
                     implicitAllocationsArr[j][preChannel].expPhysicalPort = sourcePtr[foundIdxsArray[0]].physicalPortNumber;
                     implicitAllocationsArr[j][preChannel].valid = GT_TRUE;
                }
            }
            else
            {
                portMapShadowPtr->extPortMap.preemptionChannelAllocated = GT_FALSE;
                sourcePtr[foundIdxsArray[0]].reservePreemptiveChannel = GT_FALSE;
            }


            rc    = prvCpssSip6TxqUtilsPnodeIndexGet(devNum, j, i,&currentIndex);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsPnodeIndexGet  failed for local port  %d  ",i);
            }

            if(currentIndex>maxPnodeIndex||maxPnodeIndex == CPSS_SIP6_TXQ_INVAL_DATA_CNS)
            {
                maxPnodeIndex = currentIndex;
            }

            rc      = prvCpssFalconTxqUtilsSetDmaToPnodeMapping(devNum, portMapShadowPtr->portMap.rxDmaNum, tileNum, currentIndex);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else if((GT_TRUE==preemptionAllowed)&&(GT_TRUE==implicitAllocationsArr[j][i].valid))
        {
            CPSS_TXQ_DEBUG_LOG("Device %d Tile %d Local port %d DP %d NOT contain port ,but implicit allocation is present\n",devNum,tileNum,i,j);
            /*find rxDma*/
            rc = prvCpssFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert(devNum,tileNum,j,i,&preChannelGlobalDma);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc    = prvCpssSip6TxqUtilsPnodeIndexGet(devNum, j, i,&currentIndex);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsPnodeIndexGet  failed for local port  %d  ",i);
            }

            if(currentIndex>maxPnodeIndex||maxPnodeIndex == CPSS_SIP6_TXQ_INVAL_DATA_CNS)
            {
                maxPnodeIndex = currentIndex;
            }

            rc      = prvCpssFalconTxqUtilsSetDmaToPnodeMapping(devNum, preChannelGlobalDma, tileNum, currentIndex);
            if (rc != GT_OK)
            {
                return rc;
            }

            CPSS_TXQ_DEBUG_LOG("Preemption : Allocating implicitly for port %d ->DP %d Port %d DMA %d ",
                implicitAllocationsArr[j][i].expPhysicalPort,j,i,preChannelGlobalDma);
            CPSS_TXQ_DEBUG_LOG(" Pnode %d [%d%d%d]\n",currentIndex,0,0,0);
        }
        else
        {
             CPSS_TXQ_DEBUG_LOG("Device %d Tile %d Local port %d DP %d NOT contain port.\n",devNum,tileNum,i,j);
        }
      }
    }



    if(CPSS_SIP6_TXQ_INVAL_DATA_CNS!=maxPnodeIndex)
    {
        /* 72 in Falcon , 128 in Hawk */
        if(maxPnodeIndex>=PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdqNumPorts)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " maxPnodeIndex   %d  is more then %d ",maxPnodeIndex,
                PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdqNumPorts);
        }
        targetPtr->size = maxPnodeIndex+1;
    }
    else
    {
        targetPtr->size =0;
    }

    /*initialize P nodes array*/
     rc = prvCpssDxChTxqSip6PnodeArrayAllocateAndInit(&(targetPtr->pNodeMappingConfiguration),targetPtr->size);
     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
     }

    return GT_OK;
}

static GT_STATUS prvCpssSip6TxqUtilsRemotePortConfigure
(
    IN    GT_U8                                              devNum,
    IN    CPSS_DXCH_PORT_MAP_STC                             *sourcePtr,
    IN    PRV_CPSS_DXCH_TXQ_SIP_6_PORT_MAP_PARAM_STC         *mappingParamsPtr,
    INOUT PRV_CPSS_DXCH_TXQ_SIP_ITERATORS_PARAM_STC          *iteratorsPtr,
    INOUT PRV_CPSS_DXCH_TXQ_SIP_SEARCH_DB_PARAM_STC          *searchDbPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC  *portMapShadowPtr,*cascadePortMapShadowPtr;
    GT_U32                           foundIndexInTarget = 0;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE   *pNodePtr;
    GT_U32                           aNodeListSize;
    GT_U32                           numberOfRequestedQueues;
    GT_U32                           k;
    GT_U32                           lastAllowedQueueInDp;
    GT_U32                           maxNumOfQueuesPerPort;
    GT_U32                           newCluster;
    GT_U32                           dataPath,localPort,physicalPortNumber,numOfPorts,tileNum,indexInSource;
    GT_U32                           *currentQnumIteratorPtr;
    GT_U32                           *currentAnodeIteratorPtr;
    GT_U32                           *qroupOfQIteratorPtr;
    GT_U32                           *maxGoQPtr;
    PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION     *targetPtr;
    GT_U32                          *aNodeIndexToPnodeIndexArr,*qNodeIndexToAnodeIndexArr,*goqToPhyPortIndexArr;
    GT_U32                           physicalPorts[PRV_CPSS_DXCH_FALCON_TXQ_MAX_REMOTE_PORT_MAC + 1 /*cascade*/];
    GT_U32                           physicalPortsNum = 0;

    tileNum = mappingParamsPtr->tileNum;
    dataPath = mappingParamsPtr->dataPath;
    localPort = mappingParamsPtr->localPort;
    physicalPortNumber = mappingParamsPtr->physicalPortNumber;
    numOfPorts = mappingParamsPtr->numOfPorts;
    indexInSource = mappingParamsPtr->indexInSource;

    currentQnumIteratorPtr = iteratorsPtr-> currentQnumIteratorPtr;
    currentAnodeIteratorPtr= iteratorsPtr->currentAnodeIteratorPtr;
    qroupOfQIteratorPtr= iteratorsPtr->qroupOfQIteratorPtr;
    maxGoQPtr= iteratorsPtr->maxGoQPtr;

    aNodeIndexToPnodeIndexArr = searchDbPtr ->aNodeIndexToPnodeIndexArr;
    qNodeIndexToAnodeIndexArr = searchDbPtr ->qNodeIndexToAnodeIndexArr;;
    goqToPhyPortIndexArr= searchDbPtr ->goqToPhyPortIndexArr;

    CPSS_TXQ_DEBUG_LOG("Configure RPP: tileNum %d  dataPath %d localPort %d physicalPortNumber %d\n",tileNum,
            dataPath,localPort,physicalPortNumber);

    targetPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum].mapping);

    if ((dataPath < 4) && (targetPtr->mappingMode == PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Remote port is mapped on DP %d.Must be lower then 4", dataPath);
    }
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physicalPortNumber, /*OUT*/ &portMapShadowPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssFalconTxqUtilsGetDmaToPnodeMapping(devNum, portMapShadowPtr->portMap.rxDmaNum, NULL, &foundIndexInTarget, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*allocate P node*/
    if(NULL==targetPtr->pNodeMappingConfiguration[foundIndexInTarget])
    {
        rc = prvCpssDxChTxqSip6PnodeAllocateAndInit(&(targetPtr->pNodeMappingConfiguration[foundIndexInTarget]),numOfPorts);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
         }

     }

    PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pNodePtr,devNum,tileNum,foundIndexInTarget);
    iteratorsPtr->currentPnodePtr = pNodePtr;

    aNodeListSize = pNodePtr->aNodeListSize;
    (pNodePtr->aNodeListSize)++;

    rc = prvCpssSip6TxqUtilsMaxNumOfQueuesPerPortGet(devNum,&maxNumOfQueuesPerPort);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsMaxNumOfQueuesPerPortGet  failed .");
    }

    if (localPort == CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum) - 1)
    {
         rc = prvCpssDxChPortPhysicalPortMapReverseDmaMappingGet(devNum,portMapShadowPtr->portMap.rxDmaNum,
              &physicalPortsNum,physicalPorts);

         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
         }

          rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physicalPorts[0], /*OUT*/ &cascadePortMapShadowPtr);
          if (rc != GT_OK)
          {
              CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
          }
        if (cascadePortMapShadowPtr->valid)
        {
            if (cascadePortMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Remote port can not be located at local port %d that is TYPE_CPU_SDMA ", localPort);
            }
        }
    }
    /*A nodes are consequitive*/
    pNodePtr->aNodelist[aNodeListSize].aNodeIndex    = *currentAnodeIteratorPtr;
    pNodePtr->aNodelist[aNodeListSize].physicalPort  = sourcePtr[indexInSource].physicalPortNumber;
    rc                                              = prvCpssFalconTxqUtilsLastQueueIndexForDpGet(devNum, tileNum, dataPath, &lastAllowedQueueInDp);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (targetPtr->mappingMode == PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC)
    {
        /*last 8  queues are reserved for CPU (j<8 this is a remote)*/
        lastAllowedQueueInDp -= QUEUES_PER_LOCAL_PORT_DEFAULT_VAL_MAC;
    }
    numberOfRequestedQueues = sourcePtr[indexInSource].txqPortNumber;
    if (numberOfRequestedQueues <= 0)
    {
        numberOfRequestedQueues = QUEUES_PER_LOCAL_PORT_DEFAULT_VAL_MAC;
    }
    if (numberOfRequestedQueues > maxNumOfQueuesPerPort)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "numberOfRequestedQueues is too big %d ,  for tile %d dp %d local port %d ",
            numberOfRequestedQueues, tileNum, dataPath,localPort);
    }
    /*WA to DRR issue,queues should be  alligned*/
    if (GT_TRUE == prvCpssFalconTxqClusterSegmentCrossingGet(devNum,
                                 *currentQnumIteratorPtr, *currentQnumIteratorPtr + numberOfRequestedQueues - 1,
                                 PRV_CPSS_TXQ_Q_LEVEL_L2_CLUSTER_SIZE, &newCluster))
    {
        *currentQnumIteratorPtr = newCluster * PRV_CPSS_TXQ_Q_LEVEL_L2_CLUSTER_SIZE;
    }
    if ((*currentQnumIteratorPtr) + numberOfRequestedQueues - 1 > lastAllowedQueueInDp)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "DP queue limits are exhausted for dp %d ", dataPath);
    }
    pNodePtr->aNodelist[aNodeListSize].queuesData.pdqQueueFirst  = *currentQnumIteratorPtr;
    pNodePtr->aNodelist[aNodeListSize].mappingType           = CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E;

    aNodeIndexToPnodeIndexArr[*currentAnodeIteratorPtr] = foundIndexInTarget;
    rc   = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingSet(devNum, sourcePtr[indexInSource].physicalPortNumber,
                                                                                         tileNum, *currentAnodeIteratorPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    /*Queues arrangment need to be TBD also ,need to be taken from source */
    pNodePtr->aNodelist[aNodeListSize].queuesData.pdqQueueLast =
        *currentQnumIteratorPtr + numberOfRequestedQueues - 1;
    rc = prvCpssDxChTxqSip6LocalPortFreeGoQGet(devNum, &(pNodePtr->aNodelist[aNodeListSize]),
                           dataPath,localPort, qroupOfQIteratorPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (k = *currentQnumIteratorPtr; k < pNodePtr->aNodelist[aNodeListSize].queuesData.pdqQueueLast + 1; k++)
    {
        qNodeIndexToAnodeIndexArr[k] = *currentAnodeIteratorPtr;
    }
    /*Q nodes are consequitive*/
    *currentQnumIteratorPtr = pNodePtr->aNodelist[aNodeListSize].queuesData.pdqQueueLast + 1;
    (*currentAnodeIteratorPtr)++;
    targetPtr->mappedQNum += numberOfRequestedQueues;

    goqToPhyPortIndexArr[pNodePtr->aNodelist[aNodeListSize].queuesData.queueGroupIndex]=sourcePtr[indexInSource].physicalPortNumber;
    if(pNodePtr->aNodelist[aNodeListSize].queuesData.queueGroupIndex>(*maxGoQPtr))
    {
        (*maxGoQPtr) = pNodePtr->aNodelist[aNodeListSize].queuesData.queueGroupIndex;
    }

    return GT_OK;
}

static GT_STATUS prvCpssSip6TxqUtilsEthMacPortConfigure
(
    IN    GT_U8                                              devNum,
    IN    CPSS_DXCH_PORT_MAP_STC                             *sourcePtr,
    IN    PRV_CPSS_DXCH_TXQ_SIP_IMPLICIT_ALLOCATION_STC      implicitAllocationsArr[][PREEMPTIVE_DEVICE_MAX_LOCAL_POR_PER_DP_CNS],
    IN    GT_BOOL                                            implicitAllocation,
    IN    PRV_CPSS_DXCH_TXQ_SIP_6_PORT_MAP_PARAM_STC         *mappingParamsPtr,
    INOUT PRV_CPSS_DXCH_TXQ_SIP_ITERATORS_PARAM_STC          *iteratorsPtr,
    INOUT PRV_CPSS_DXCH_TXQ_SIP_SEARCH_DB_PARAM_STC          *searchDbPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC  *portMapShadowPtr;
    GT_U32                           foundIndexInTarget = 0;
    GT_U32                           rxDmaNum;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE   *pNodePtr;
    GT_U32                           aNodeListSize;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE   *aNodePtr;
    GT_U32                           additionalAnodeInd = 0 ;
    GT_U32                           numberOfRequestedQueues;
    GT_U32                           k;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT  mappingType;
    GT_U32                           lastAllowedQueueInDp;
    GT_U32                           maxNumOfQueuesPerPort;
    GT_U32                           newCluster;
    GT_BOOL                          preemptionAllowed = GT_FALSE;/*Whether preemption feature is allowed for this device*/
    GT_U32                           dataPath,localPort,physicalPortNumber,numOfPorts,tileNum,indexInSource;
    GT_U32                           *currentQnumIteratorPtr;
    GT_U32                           *currentAnodeIteratorPtr;
    GT_U32                           *qroupOfQIteratorPtr;
    GT_U32                           *maxGoQPtr;
    PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION     *targetPtr;
    GT_U32                          *aNodeIndexToPnodeIndexArr,*qNodeIndexToAnodeIndexArr,*goqToPhyPortIndexArr;

    tileNum = mappingParamsPtr->tileNum;
    dataPath = mappingParamsPtr->dataPath;
    localPort = mappingParamsPtr->localPort;
    physicalPortNumber = mappingParamsPtr->physicalPortNumber;
    numOfPorts = mappingParamsPtr->numOfPorts;
    indexInSource = mappingParamsPtr->indexInSource;

     CPSS_TXQ_DEBUG_LOG("Configure ETH: tileNum %d  dataPath %d localPort %d physicalPortNumber %d\n",tileNum,
            dataPath,localPort,physicalPortNumber);

    currentQnumIteratorPtr = iteratorsPtr-> currentQnumIteratorPtr;
    currentAnodeIteratorPtr= iteratorsPtr->currentAnodeIteratorPtr;
    qroupOfQIteratorPtr= iteratorsPtr->qroupOfQIteratorPtr;
    maxGoQPtr= iteratorsPtr->maxGoQPtr;

    aNodeIndexToPnodeIndexArr = searchDbPtr ->aNodeIndexToPnodeIndexArr;
    qNodeIndexToAnodeIndexArr = searchDbPtr ->qNodeIndexToAnodeIndexArr;
    goqToPhyPortIndexArr= searchDbPtr ->goqToPhyPortIndexArr;

    targetPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum].mapping);

    rc = prvCpssSip6TxqUtilsMaxNumOfQueuesPerPortGet(devNum,&maxNumOfQueuesPerPort);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsMaxNumOfQueuesPerPortGet  failed .");
    }

    rc = prvCpssDxChTxqSip6PreemptionFeatureAllowedGet(devNum,&preemptionAllowed);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqSip6PreemptionFeatureAllowedGet  failed .");
    }

    if(GT_TRUE==implicitAllocation)
    {
        mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
        physicalPortNumber = implicitAllocationsArr[dataPath][localPort].expPhysicalPort;
        numberOfRequestedQueues = implicitAllocationsArr[dataPath][localPort].numOfQueues;
        additionalAnodeInd= implicitAllocationsArr[dataPath][localPort].expAnode;
    }
    else
    {   /*this channel is act as preemptive but not implicitly allocated*/
        if((GT_TRUE == preemptionAllowed)&&(GT_TRUE==implicitAllocationsArr[dataPath][localPort].valid))
        {
            additionalAnodeInd= implicitAllocationsArr[dataPath][localPort].expAnode;
        }

        mappingType =sourcePtr[indexInSource].mappingType;
        physicalPortNumber = sourcePtr[indexInSource].physicalPortNumber;
        numberOfRequestedQueues = sourcePtr[indexInSource].txqPortNumber;
        if(GT_TRUE == preemptionAllowed)
        {
            /*find preemptive channel and update physical port*/
            for(k=0;k<PREEMPTIVE_DEVICE_MAX_LOCAL_POR_PER_DP_CNS;k++)
            {
                if(GT_FALSE == implicitAllocationsArr[dataPath][k].valid)
                {
                    continue;
                }

                if(implicitAllocationsArr[dataPath][k].expPhysicalPort ==physicalPortNumber)
                {
                    implicitAllocationsArr[dataPath][k].expAnode = *currentAnodeIteratorPtr;
                    implicitAllocationsArr[dataPath][k].numOfQueues =numberOfRequestedQueues;
                    break;
                }
            }
        }
    }

     if(GT_TRUE==implicitAllocation)
     {
          /*find rxDma*/
         rc = prvCpssFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert(devNum,tileNum,dataPath,localPort,&rxDmaNum);
         if (rc != GT_OK)
         {
             return rc;
         }
     }
     else
     {
         rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physicalPortNumber, /*OUT*/ &portMapShadowPtr);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
         }

         rxDmaNum = portMapShadowPtr->portMap.rxDmaNum;
     }

     rc = prvCpssFalconTxqUtilsGetDmaToPnodeMapping(devNum,rxDmaNum, NULL, &foundIndexInTarget, GT_TRUE);
     if (rc != GT_OK)
     {
         return rc;
     }

     if (targetPtr->mappingMode == PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC)
     {
         if (localPort == CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum) - 1)
         {
             if (tileNum % 2)
             {
                 rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum, tileNum, dataPath - 1, currentQnumIteratorPtr);
             }
             else
             {
                 rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum, tileNum, dataPath + 1,currentQnumIteratorPtr);
             }
             if (rc != GT_OK)
             {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
             }
             /*Last 8 are reserved for CPU*/
             (*currentQnumIteratorPtr) -= QUEUES_PER_LOCAL_PORT_DEFAULT_VAL_MAC;
         }
     }

     /*allocate P node*/
     if(NULL==targetPtr->pNodeMappingConfiguration[foundIndexInTarget])
     {
         rc = prvCpssDxChTxqSip6PnodeAllocateAndInit(&(targetPtr->pNodeMappingConfiguration[foundIndexInTarget]),numOfPorts);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
         }
     }

     PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pNodePtr,devNum,tileNum,foundIndexInTarget);
     iteratorsPtr->currentPnodePtr = pNodePtr;

     aNodeListSize = pNodePtr->aNodeListSize;

     /*PFC mapping*/
     rc = prvCpssFalconTxqQfcLocalPortToSourcePortSet(devNum, tileNum, dataPath, localPort, physicalPortNumber,
                              CPSS_DXCH_PORT_FC_MODE_DISABLE_E, GT_FALSE);
     if (rc != GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcLocalPortToSourcePortSet failed for dp %d local port %d \n", dataPath,localPort);
     }
     if (aNodeListSize > 0 || numOfPorts > 1)
     {
         if (sourcePtr[indexInSource].mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
         {
             pNodePtr->isCascade      = GT_TRUE;
             pNodePtr->cascadePhysicalPort    = physicalPortNumber;
             CPSS_LOG_INFORMATION_MAC("Cascaded port  :%d\n", physicalPortNumber);
             return GT_OK; /*This must be cascaded port - no need to map*/
         }
         else /*CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E*/
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc,
                               "physical port[%d] try to use global DMA[%d] that is already used (Local  port  %d already contain  mapped physical port)",
                               physicalPortNumber,
                               rxDmaNum,
                               localPort);
         }
     }
     pNodePtr->aNodeListSize++;
     /*A nodes are consequitive*/
     pNodePtr->aNodelist[aNodeListSize].aNodeIndex    = *currentAnodeIteratorPtr;
     pNodePtr->aNodelist[aNodeListSize].physicalPort  = physicalPortNumber;

     if(GT_TRUE==implicitAllocation)/*preemptive - implicit allocation*/
     {
        pNodePtr->aNodelist[aNodeListSize].implicitAllocation = GT_TRUE;
        pNodePtr->aNodelist[aNodeListSize].partnerIndex = additionalAnodeInd;
        pNodePtr->aNodelist[aNodeListSize].preemptionType = PRV_DXCH_TXQ_SIP6_PREEMTION_A_NODE_TYPE_PRE_E;
        /*search array is used in prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber,so do this trick*/
        targetPtr->searchTable.aNodeIndexToPnodeIndex= aNodeIndexToPnodeIndexArr;
        targetPtr->searchTable.aNodeIndexToPnodeIndexSize = CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC;
        /*find express*/
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,physicalPortNumber,&aNodePtr);
        targetPtr->searchTable.aNodeIndexToPnodeIndex=NULL;
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber failed for port %d \n",physicalPortNumber);
        }

        aNodePtr->partnerIndex = *currentAnodeIteratorPtr;
        CPSS_TXQ_DEBUG_LOG("Explicit Connect Anode(E) %d  to Anode (P)%d (dp %d,local port %d)\n",aNodePtr->aNodeIndex,
            aNodePtr->partnerIndex,mappingParamsPtr->dataPath,mappingParamsPtr->localPort);

        if(additionalAnodeInd!=aNodePtr->aNodeIndex)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "additionalAnodeInd  %d not equal to aNodePtr->aNodeIndex %d ", additionalAnodeInd, aNodePtr->aNodeIndex);
        }
     }
     else if(GT_TRUE==sourcePtr[indexInSource].reservePreemptiveChannel)/*express*/
     {
        pNodePtr->aNodelist[aNodeListSize].preemptionType = PRV_DXCH_TXQ_SIP6_PREEMTION_A_NODE_TYPE_EXP_E;
     }
     else if((GT_TRUE==preemptionAllowed)&&(GT_TRUE == implicitAllocationsArr[dataPath][localPort].valid))/*preemptive - explicit  allocation*/
     {
        pNodePtr->aNodelist[aNodeListSize].partnerIndex = additionalAnodeInd;
        pNodePtr->aNodelist[aNodeListSize].preemptionType = PRV_DXCH_TXQ_SIP6_PREEMTION_A_NODE_TYPE_PRE_E;

        /*search array is used in prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber,so do this trick*/
        targetPtr->searchTable.aNodeIndexToPnodeIndex= aNodeIndexToPnodeIndexArr;
        targetPtr->searchTable.aNodeIndexToPnodeIndexSize = CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC;

        /*find express*/
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,implicitAllocationsArr[dataPath][localPort].expPhysicalPort,&aNodePtr);
        targetPtr->searchTable.aNodeIndexToPnodeIndex=NULL;
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber failed for port %d \n",physicalPortNumber);
        }
        aNodePtr->partnerIndex = *currentAnodeIteratorPtr;
        CPSS_TXQ_DEBUG_LOG("Implicit connect Anode(E) %d  to Anode (P)%d (dp %d,local port %d)\n",aNodePtr->aNodeIndex,
            aNodePtr->partnerIndex,mappingParamsPtr->dataPath,mappingParamsPtr->localPort);
        if(additionalAnodeInd!=aNodePtr->aNodeIndex)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "additionalAnodeInd  %d not equal to aNodePtr->aNodeIndex %d ", additionalAnodeInd, aNodePtr->aNodeIndex);
        }
     }

     /*Check that currentQnumIterator+number of mapped queues confirm the limitations*/
     rc = prvCpssFalconTxqUtilsLastQueueIndexForDpGet(devNum, tileNum, dataPath, &lastAllowedQueueInDp);
     if (rc != GT_OK)
     {
        return rc;
     }
     if (targetPtr->mappingMode == PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC)
     {
        if (localPort < 8)
        {
            /*last 8  queues are reserved for CPU*/
            lastAllowedQueueInDp -= QUEUES_PER_LOCAL_PORT_DEFAULT_VAL_MAC;
        }
     }

     if (numberOfRequestedQueues <= 0)
     {
        numberOfRequestedQueues = QUEUES_PER_LOCAL_PORT_DEFAULT_VAL_MAC;
     }
     if (numberOfRequestedQueues > maxNumOfQueuesPerPort)
     {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "numberOfRequestedQueues is too big %d ,  for tile %d dp %d local port %d ", numberOfRequestedQueues, tileNum, dataPath, localPort);
     }

     /*WA to DRR issue,queues should be  alligned*/
     if (GT_TRUE == prvCpssFalconTxqClusterSegmentCrossingGet(devNum,
                                 *currentQnumIteratorPtr, *currentQnumIteratorPtr + numberOfRequestedQueues - 1,
                                 PRV_CPSS_TXQ_Q_LEVEL_L2_CLUSTER_SIZE, &newCluster))
     {
        *currentQnumIteratorPtr = newCluster * PRV_CPSS_TXQ_Q_LEVEL_L2_CLUSTER_SIZE;
     }
     if (*currentQnumIteratorPtr + numberOfRequestedQueues - 1 > lastAllowedQueueInDp)
     {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "DP queue limits are exhausted for tile %d dp %d ", tileNum,dataPath);
     }
     pNodePtr->aNodelist[aNodeListSize].queuesData.pdqQueueFirst  = *currentQnumIteratorPtr;
     pNodePtr->aNodelist[aNodeListSize].mappingType           =  mappingType;
     aNodeIndexToPnodeIndexArr[*currentAnodeIteratorPtr] = foundIndexInTarget;


     if(GT_FALSE==implicitAllocation)
     {
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingSet(devNum, sourcePtr[indexInSource].physicalPortNumber,
                                                                                             tileNum, *currentAnodeIteratorPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
     }
     else
     {
        pNodePtr->aNodelist[aNodeListSize].implicitAllocation=  GT_TRUE;
        CPSS_TXQ_DEBUG_LOG("Preemption : Implicitly allocated Anode %d for physical %d [dp %d local port %d]\n",*currentAnodeIteratorPtr,
                    physicalPortNumber,mappingParamsPtr->dataPath,mappingParamsPtr->localPort);
     }

     /*Queues arrangment need to be TBD also ,need to be taken from source */
     pNodePtr->aNodelist[aNodeListSize].queuesData.pdqQueueLast =
        *currentQnumIteratorPtr + numberOfRequestedQueues - 1;
     rc = prvCpssDxChTxqSip6LocalPortFreeGoQGet(devNum, &(pNodePtr->aNodelist[aNodeListSize]),
                           dataPath,localPort,qroupOfQIteratorPtr);
     if (rc != GT_OK)
     {
        return rc;
     }

     for (k = *currentQnumIteratorPtr; k < pNodePtr->aNodelist[aNodeListSize].queuesData.pdqQueueLast + 1; k++)
     {
        qNodeIndexToAnodeIndexArr[k] = *currentAnodeIteratorPtr;
     }

     goqToPhyPortIndexArr[pNodePtr->aNodelist[aNodeListSize].queuesData.queueGroupIndex]= physicalPortNumber;
     if(pNodePtr->aNodelist[aNodeListSize].queuesData.queueGroupIndex>*maxGoQPtr)
     {
        *maxGoQPtr = pNodePtr->aNodelist[aNodeListSize].queuesData.queueGroupIndex;
     }
     /*Q nodes are consequitive*/
     *currentQnumIteratorPtr = pNodePtr->aNodelist[aNodeListSize].queuesData.pdqQueueLast + 1;
     (*currentAnodeIteratorPtr)++;
     targetPtr->mappedQNum  += numberOfRequestedQueues;

    return GT_OK;
}

static GT_STATUS prvCpssSip6TxqUtilsConnectAnodes
(
    IN GT_U8                                              devNum,
    IN CPSS_DXCH_PORT_MAP_STC                             *sourcePtr,
    IN GT_U32                                             size,
    IN GT_U32                                             tileNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_IMPLICIT_ALLOCATION_STC      implicitAllocationsArr[][PREEMPTIVE_DEVICE_MAX_LOCAL_POR_PER_DP_CNS],
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION    *targetPtr
)
{
    GT_U32                           startDp;
    GT_U32                           currentQnumIterator = 0;
    GT_BOOL                          firstQueueInDpSet;
    GT_U32                           j;
    GT_32                            i;/*signed*/
    GT_U32                           foundIdxsArray[PRV_CPSS_DXCH_FALCON_TXQ_MAX_REMOTE_PORT_MAC + 1 /*cascade*/];
    GT_U32                           numOfPorts;
    GT_BOOL                          implicitAllocation = GT_FALSE;
    GT_STATUS                        rc;
    GT_U32                           currentAnodeIterator    = 0;
    GT_U32                           newCluster;
    GT_U32                           remotePortsIterator;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE   *pNodePtr;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT  mappingType;
    GT_U32                           physicalPortNumber;
    GT_U32                           firstQueueInDp;
    GT_U32                           aNodeIndexToPnodeIndex[CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC];
    GT_U32                           maxNumOfQueuesPerPort;
    GT_U32                           qNodeIndexToAnodeIndex[CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM];
    GT_U32                           goqToPhyPortIndex[CPSS_DXCH_SIP_6_MAX_Q_GROUP_SIZE_MAC];
    GT_U32                           qroupOfQIterator    = 0;
    GT_U32                           maxGoQ =0;
    GT_BOOL                          preemptionAllowed = GT_FALSE;/*Whether preemption feature is allowed for this device*/
    PRV_CPSS_DXCH_TXQ_SIP_6_PORT_MAP_PARAM_STC mappingParams;
    PRV_CPSS_DXCH_TXQ_SIP_ITERATORS_PARAM_STC          iterators;
    PRV_CPSS_DXCH_TXQ_SIP_SEARCH_DB_PARAM_STC          searchDb;

    cpssOsMemSet(aNodeIndexToPnodeIndex,CPSS_SIP6_TXQ_INVAL_DATA_CNS,(CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC*sizeof(GT_U32 )));
    cpssOsMemSet(qNodeIndexToAnodeIndex,CPSS_SIP6_TXQ_INVAL_DATA_CNS,(CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM*sizeof(GT_U32 )));
    cpssOsMemSet(goqToPhyPortIndex,CPSS_SIP6_TXQ_INVAL_DATA_CNS,(CPSS_DXCH_SIP_6_MAX_Q_GROUP_SIZE_MAC*sizeof(GT_U32 )));

    searchDb.aNodeIndexToPnodeIndexArr = aNodeIndexToPnodeIndex;
    searchDb.goqToPhyPortIndexArr = goqToPhyPortIndex;
    searchDb.qNodeIndexToAnodeIndexArr = qNodeIndexToAnodeIndex;

    iterators.currentAnodeIteratorPtr = &currentAnodeIterator;
    iterators.currentQnumIteratorPtr = &currentQnumIterator;
    iterators.maxGoQPtr = &maxGoQ;
    iterators.qroupOfQIteratorPtr = &qroupOfQIterator;
    iterators.currentPnodePtr = NULL;

    rc = prvCpssSip6TxqUtilsMaxNumOfQueuesPerPortGet(devNum,&maxNumOfQueuesPerPort);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsMaxNumOfQueuesPerPortGet  failed .");
    }

    rc = prvCpssDxChTxqSip6PreemptionFeatureAllowedGet(devNum,&preemptionAllowed);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqSip6PreemptionFeatureAllowedGet  failed .");
    }

    /*

    1.loop over PDS and local port
    2.Find physical port by (PDS,local port)
    3a.If CPU or local port create A node with 8 queues
    3b.If remote port then look in source all the remote ports that have the same MAC and give then 8 queues - TBD
    */
    /*For mirrored tile A nodes are mirrored in port mode xcl*/
    if (tileNum % 2)
    {
        startDp = MAX_DP_IN_TILE(devNum) - 1;
    }
    else
    {
        startDp = 0;
    }
    currentQnumIterator = 0;


    for (i = startDp; (tileNum % 2) ? i >= 0 : (GT_U32)i < MAX_DP_IN_TILE(devNum); (tileNum % 2) ? i-- : i++)
    {
        if (targetPtr->mappingMode == PRV_CPSS_QUEUE_MAPPING_MODE_FIXED_MAC)
        {
            rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum, tileNum, i, &currentQnumIterator);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        firstQueueInDpSet = GT_FALSE;
        for (j = 0; j < CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum); j++)
        {
            rc =  prvCpssFalconTxqUtilsFindByLocalPortAndDpInSource(devNum, sourcePtr, size, tileNum, i, j, foundIdxsArray, &numOfPorts);
            if (rc == GT_OK ||((GT_TRUE==preemptionAllowed)&&(GT_TRUE== implicitAllocationsArr[i][j].valid)))
            {
                /*This is implicit allocation*/
                if(rc != GT_OK )
                {
                    implicitAllocation = GT_TRUE;
                }
                else
                {
                    implicitAllocation = GT_FALSE;
                }

                /*WA to DRR issue,A nodes should be  alligned*/
                if (GT_TRUE == prvCpssFalconTxqClusterSegmentCrossingGet(devNum,
                    currentAnodeIterator, currentAnodeIterator + numOfPorts - 1,
                    PRV_CPSS_TXQ_A_LEVEL_L2_CLUSTER_SIZE, &newCluster))
                {
                    currentAnodeIterator                                    = newCluster * PRV_CPSS_TXQ_A_LEVEL_L2_CLUSTER_SIZE;
                    PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum].mapping.aNodeAllignedMapping = GT_TRUE;
                }
                if (firstQueueInDpSet == GT_FALSE)
                {
                    rc = prvCpssFalconTxqUtilsStartQueueIndexForDpSet(devNum, tileNum, i, currentQnumIterator);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }
                    firstQueueInDpSet = GT_TRUE;
                }

                if(GT_TRUE==implicitAllocation)
                {
                    numOfPorts =1;
                }

                for (remotePortsIterator = 0; remotePortsIterator < numOfPorts; remotePortsIterator++)
                {
                    if(GT_TRUE==implicitAllocation)
                    {
                        mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
                        physicalPortNumber = implicitAllocationsArr[i][j].expPhysicalPort;
                    }
                    else
                    {
                        mappingType =sourcePtr[foundIdxsArray[remotePortsIterator]].mappingType;
                        physicalPortNumber = sourcePtr[foundIdxsArray[remotePortsIterator]].physicalPortNumber;
                    }

                    mappingParams.dataPath = i;
                    mappingParams.localPort = j;
                    mappingParams.indexInSource = foundIdxsArray[remotePortsIterator];
                    mappingParams.numOfPorts = numOfPorts;
                    mappingParams.physicalPortNumber = physicalPortNumber;
                    mappingParams.tileNum = tileNum;
                    iterators.currentPnodePtr = NULL;

                    switch (mappingType)
                    {
                    case CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E:
                    case CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E:
                        rc = prvCpssSip6TxqUtilsEthMacPortConfigure(devNum,sourcePtr,implicitAllocationsArr,implicitAllocation,
                            &mappingParams,&iterators,&searchDb);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                        }
                        break;
                    case CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E:
                        rc = prvCpssSip6TxqUtilsRemotePortConfigure(devNum,sourcePtr,&mappingParams,&iterators,&searchDb);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                        }

                        break;
                    default:
                        break;
                    }
                }

                /*get current P node*/
                pNodePtr = iterators.currentPnodePtr;

                /*sanity*/
                if(NULL == pNodePtr)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"Pnode is not set correctly at dp %d ,local port %d",i,j);
                }


                rc      = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum, tileNum, i, &firstQueueInDp);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                if(pNodePtr->aNodeListSize==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "A node list size == 0");
                }

                rc = prvCpssFalconTxqUtilsUpdateSelectListParamsSet(devNum, tileNum, i, j, pNodePtr->aNodelist[0].queuesData.pdqQueueFirst - firstQueueInDp,
                    pNodePtr->aNodelist[pNodePtr->aNodeListSize - 1].queuesData.pdqQueueLast - firstQueueInDp, GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsUpdateSelectListParamsSet failed ");
                }
                /*Enable only prio 0 lists*/
                rc = prvCpssSip6_10TxqSdqSelectListEnableSet(devNum, tileNum, i, j, GT_TRUE, GT_TRUE);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6_10TxqSdqSelectListEnableSet failed ");
                }
                /*Disable only prio 1 lists*/
                rc = prvCpssSip6_10TxqSdqSelectListEnableSet(devNum, tileNum, i, j, GT_FALSE, GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6_10TxqSdqSelectListEnableSet failed ");
                }
            }
            else
            {
                CPSS_LOG_INFORMATION_MAC("Converting to txq data base : Empty slot dp %d local port %d\n", i, j);
            }
        }
    }


    targetPtr->searchTable.aNodeIndexToPnodeIndexSize = currentAnodeIterator;
    targetPtr->searchTable.qNodeIndexToAnodeIndexSize= currentQnumIterator;
    targetPtr->searchTable.goqToPhyPortSize = maxGoQ+1;

    targetPtr->searchTable.aNodeIndexToPnodeIndex = NULL;

    rc = prvCpssDxChTxqSip6SearchArrayAllocateAndInit(&(targetPtr->searchTable.aNodeIndexToPnodeIndex),aNodeIndexToPnodeIndex,
        targetPtr->searchTable.aNodeIndexToPnodeIndexSize);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChTxqSip6SearchArrayAllocateAndInit(&(targetPtr->searchTable.qNodeIndexToAnodeIndex),qNodeIndexToAnodeIndex,
        targetPtr->searchTable.qNodeIndexToAnodeIndexSize);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChTxqSip6SearchArrayAllocateAndInit(&(targetPtr->searchTable.goqToPhyPortIndex),goqToPhyPortIndex,
        targetPtr->searchTable.goqToPhyPortSize);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqUtilsConvertToDb function
 * @endinternal
 *
 * @brief   Convert to txq database format from CPSS_DXCH_PORT_MAP_STC .
 *         Create mapping between P nodes /A nodes and physical port numbers
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman;AC3X;
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;  Bobcat3; Aldrin2.
 *
 * @param[in] sourcePtr                - Pointer to CPSS_DXCH_PORT_MAP_STC to be converted into scheduler tree
 * @param[in] size                     - of source array.Number of ports to map, array size
 * @param[in] tileNum                  -  The number of tile
 *
 * @param[out] targetPtr                - Pointer to schedualler mappping configuration
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsConvertToDb
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_PORT_MAP_STC * sourcePtr,
    IN GT_U32 size,
    IN GT_U32 tileNum,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION  *  targetPtr
)
{

    GT_STATUS               rc;
    GT_U32                  foundIndexInTarget = 0;
    GT_U32                   i,j;
    GT_U32                  aNodeTileStartIndex = 0, tmp;

    PRV_CPSS_DXCH_TXQ_SIP_IMPLICIT_ALLOCATION_STC   implicitAllocations[PREEMPTIVE_DEVICE_MAX_DP_PER_TILE_CNS][PREEMPTIVE_DEVICE_MAX_LOCAL_POR_PER_DP_CNS];

    for(i=0;i<PREEMPTIVE_DEVICE_MAX_DP_PER_TILE_CNS;i++)
    {
        for(j=0;j<PREEMPTIVE_DEVICE_MAX_LOCAL_POR_PER_DP_CNS;j++)
        {
            implicitAllocations[i][j].valid = GT_FALSE;
        }
    }

    targetPtr->mappedQNum = 0;
    for (i = 0; i < tileNum; i++)
    {
        rc = prvCpssFalconTxqUtilsMappedQueueNumGet(devNum, i, &tmp);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        aNodeTileStartIndex += tmp;
    }

    CPSS_TXQ_DEBUG_LOG("Start building map for tileNum %d  Source size %d Number of Q already mapped %d device %d\n",tileNum,
            size,aNodeTileStartIndex,devNum);

    /*First configure P nodes .Allocate required Pnodes in Pnodes pointer array*/
    rc = prvCpssSip6TxqUtilsConnectPnodes(devNum,sourcePtr,size,tileNum,implicitAllocations,targetPtr);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsPnodesDbConfigure  failed for tile %d \n",tileNum);
    }

    /*Now add A nodes*/
    rc = prvCpssSip6TxqUtilsConnectAnodes(devNum,sourcePtr,size,tileNum,implicitAllocations,targetPtr);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsAnodesDbConfigure  failed for tile %d \n",tileNum);
    }

    /*Configure number of pizza slices*/
    if (GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        for (i = 0;i < MAX_DP_IN_TILE(devNum); i++)
        {
            if (GT_OK == prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget
                    (devNum, tileNum, i, CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum) - 1, &foundIndexInTarget, GT_FALSE))
            {
                targetPtr->pdxPizza.pdxPizzaNumberOfSlices[i] = TXQ_PDX_MAX_SLICE_NUMBER_MAC;
            }
            else
            {
                targetPtr->pdxPizza.pdxPizzaNumberOfSlices[i] = TXQ_PDX_MIN_SLICE_NUMBER_MAC;
            }
        }
     }


    rc = prvCpssSip6TxqUtilsGlobalQueueIndexesSet(devNum, tileNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqUtilsUpdateGlobalQueueIndexes failed ");
    }

    rc = prvCpssFalconTxqUtilsMappedQueueNumGet(devNum, tileNum, &tmp);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsMappedQueueNumGet failed ");
    }

    CPSS_TXQ_DEBUG_LOG("Finish building map for tileNum %d  Source size %d Number of Q  mapped %d device %d\n",tileNum,
            size,tmp,devNum);

    return GT_OK;
}
/**
 * @internal prvCpssDxChTxQFalconPizzaArbiterInitPerTile function
 * @endinternal
 *
 * @brief   Initialize the number of silices ta each PDX PDS pizza arbiter,also initialize all the slices to unused
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] pdxNum                   -tile number
 * @param[in] pdxPizzaNumberOfSlicesArr-number of slices per dp
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *@param[in] pdxPizzaNumberOfSlicesArr-number of slices per dp
 */
GT_STATUS prvCpssDxChTxQFalconPizzaArbiterInitPerTile
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 * pdxPizzaNumberOfSlicesArr
)
{
    GT_U32      i;
    GT_STATUS   rc;
    for (i = 0; i < MAX_DP_IN_TILE(devNum); i++)
    {
        rc = prvCpssDxChTxQFalconPizzaArbiterInitPerDp(devNum, pdxNum, i, pdxPizzaNumberOfSlicesArr[i]);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsInitPdx function
 * @endinternal
 *
 * @brief   Initialize PDX unit to default values
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                  - tile number.(APPLICABLE RANGES:0..3)
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsInitPdx
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
)
{
    GT_STATUS                       rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_GENERAL_CONFIGURATION *     generalCfgPtr;
    GT_U32                          i, size;
    PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC  txPortMap = { 0, 0, 0, 0 };
    size        = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdxNumQueueGroups;
    generalCfgPtr   = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum].general);
    rc      = prvCpssDxChTxqFalconPdxInterfaceMapSet(devNum, tileNum, &(generalCfgPtr->pdxMapping[0]));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxqFalconPdxInterfaceMapSet failed for tile %d", tileNum);
    }
    rc = prvCpssDxChTxqFalconPdxBurstFifoInit(devNum, tileNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxqFalconPdxBurstFifoInit failed for tile %d", tileNum);
    }
    /*do it only once at the first tile,copy to other tiles is done automaticly*/
    if (0 == tileNum)
    {
        for (i = 0; i < size; i++)
        {
            rc = prvCpssDxChTxqFalconPdxQueueGroupMapSet(devNum, i, txPortMap);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "queueGroupIndex %d \n", i );
            }
        }
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqUtilsPdxQroupMapTableInit function
 * @endinternal
 *
 * @brief   Initialize PDX DX_QGRPMAP table (write to HW) accoring to SW txq shadow
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] mappingPtr               - pointer to SW mapping shadow
 * @param[out] entriesAddedPtr               - pointer to number of added entries
 *
 *
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsPdxQroupMapTableInit
(
    IN  GT_U8 devNum,
    IN  PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION  * mappingPtr,
    IN GT_U32 tileNum,
    OUT GT_U32 * entriesAddedPtr
)
{
    GT_U32                          portIterator, aNodeListSize, aNodeListIterator, queueGroupIndex;
    PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC  txPortMap = { 0, 0, 0, 0 };
    GT_STATUS                       rc;
    GT_U32                          physicalPort;
    GT_U32                          firstQueueInDp;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE  * pNodePtr = NULL;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;

    CPSS_NULL_PTR_CHECK_MAC(entriesAddedPtr);

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (portIterator = 0; portIterator < mappingPtr->size; portIterator++)
    {
        PRV_TXQ_SIP_6_PNODE_INSTANCE_NO_ERROR_GET(pNodePtr,devNum,tileNum,portIterator);

        if(NULL == pNodePtr)
        {
            continue;/*not mapped*/
        }

        aNodeListSize = pNodePtr->aNodeListSize;
        for (aNodeListIterator = 0; aNodeListIterator < aNodeListSize; aNodeListIterator++)
        {
            queueGroupIndex =
                pNodePtr->aNodelist[aNodeListIterator].queuesData.queueGroupIndex;

            physicalPort                                = pNodePtr->aNodelist[aNodeListIterator].physicalPort;
            rc                                  = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, physicalPort,
                                                                                 &txPortMap.queuePdxIndex,
                                                                                 &txPortMap.queuePdsIndex,
                                                                                 &txPortMap.dpCoreLocalTrgPort, NULL);
            if (rc != GT_OK)
            {
                return rc;
            }
            /*This is preemptive channel*/
            if(GT_TRUE==pNodePtr->aNodelist[aNodeListIterator].implicitAllocation)
            {
                rc = prvCpssDxChTxqSip6_10PreChannelGet(devNum,0,txPortMap.dpCoreLocalTrgPort,&txPortMap.dpCoreLocalTrgPort);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            /*Find first  Q  in current DP*/
            rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum, txPortMap.queuePdxIndex, txPortMap.queuePdsIndex, &firstQueueInDp);
            if (rc != GT_OK)
            {
                return rc;
            }
            txPortMap.queueBase =
                pNodePtr->aNodelist[aNodeListIterator].queuesData.pdqQueueFirst -
                firstQueueInDp;
            pNodePtr->aNodelist[aNodeListIterator].queuesData.queueBase   = txPortMap.queueBase;
            pNodePtr->aNodelist[aNodeListIterator].queuesData.dp      = txPortMap.queuePdsIndex;
            pNodePtr->aNodelist[aNodeListIterator].queuesData.tileNum = txPortMap.queuePdxIndex;
            pNodePtr->aNodelist[aNodeListIterator].queuesData.localPort   = txPortMap.dpCoreLocalTrgPort;
            /*Init local PDS/SDQ db*/
            if (pNodePtr->isCascade == GT_FALSE )
            {
                pNodePtr->sdqQueueFirst   = txPortMap.queueBase;
                pNodePtr->sdqQueueLast    = pNodePtr->aNodelist[aNodeListIterator].queuesData.pdqQueueLast -
                                                      firstQueueInDp;
            }
            else
            {
                if (aNodeListIterator == 0)
                {
                    pNodePtr->sdqQueueFirst   = txPortMap.queueBase;
                }
                else
                /*check consequity*/
                if (pNodePtr->sdqQueueLast != txPortMap.queueBase - 1)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "portMapPtr->sdqQueueLast [%d] !=txPortMap.queueBase-1 [%d]\n", pNodePtr->sdqQueueLast, txPortMap.queueBase - 1);
                }
                /*adding Qs*/
                pNodePtr->sdqQueueLast = pNodePtr->aNodelist[aNodeListIterator].queuesData.pdqQueueLast -
                                                   firstQueueInDp;
            }

            if (!((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
                  (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)) )
            {
                /*copy for all PDXs is done inside at prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum*/
                rc = prvCpssDxChTxqFalconPdxQueueGroupMapSet(devNum, queueGroupIndex, txPortMap);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "queueGroupIndex %d \n", queueGroupIndex);
                }
            }
             (*entriesAddedPtr)++;
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsInitPds function
 * @endinternal
 *
 * @brief   Initialize PDS .Set length adjust parameters for all the queues in perticular PDS.
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   PP's device number.
 * @param[in] tileNum                  -Then number of tile (Applicable range 0)
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsInitPds
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
)
{
    GT_STATUS   rc  = GT_OK;
    GT_U32      pdsNum  = 0;
    GT_U32      queueNum;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    for (pdsNum = 0; pdsNum < MAX_DP_IN_TILE(devNum); pdsNum++)
    {
        for (queueNum = 0; queueNum < CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC; queueNum++)
        {
            rc = prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapSet(devNum, tileNum, pdsNum, queueNum, DEFAULT_LENGTH_ADJUST_PROFILE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapSet fail for %d  %d \n", pdsNum, queueNum );
            }
        }

        rc = prvCpssDxChTxqSip6PdsPbReadReqFifoLimitSet(devNum,pdsNum,PRV_PB_READ_REQ_FIFO_LIMIT_MAC);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapSet fail for %d  %d \n", pdsNum, queueNum );
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsInitSdq function
 * @endinternal
 *
 * @brief   Initialize SDQ (write to HW) accoring to SW txq shadow
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                  -Then number of tile (Applicable range 0..3)
 * @param[in] totalTiles                  -Number of tiles in the device (Applicable range 1..4)
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsInitSdq
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 totalTiles
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      sdqNum = 0;
    GT_U32                      localPortNum, foundIndex;
    GT_U32                      i, j;
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_QUEUE_RANGE range;
    GT_U32                      tc;
    GT_BOOL                     localPortMapped;
    GT_U32                      localQueueIndex, firstQueueInDp;
    GT_U32                      pbFullLimit;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr = NULL;

    for (sdqNum = 0; sdqNum < MAX_DP_IN_TILE(devNum); sdqNum++)
    {
        rc = prvCpssFalconTxqSdqSemiEligEnableSet(devNum, tileNum, sdqNum, PRV_CPSS_SEMI_ELIG_DEFAULT_VALUE_MAC,
            PRV_CPSS_SEMI_ELIG4DRY_DEFAULT_VALUE_MAC,(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum) == GT_TRUE)?GT_TRUE:GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqSdqSemiEligEnableSet failed for dp  %d \n", sdqNum );
        }

        rc = prvCpssSip6_30TxqSdqQbvScanEnableSet(devNum, tileNum, sdqNum, PRV_CPSS_QBV_SCAN_ENABLE_DEFAULT_VALUE_MAC);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqSdqSemiEligEnableSet failed for dp  %d \n", sdqNum );
        }

        for (localPortNum = 0; localPortNum < CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum); localPortNum++)
        {
            PRV_CPSS_TXQ_UTILS_SKIP_RESERVED_MAC(devNum,localPortNum)
            localPortMapped = GT_FALSE;
            if (prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget(devNum, tileNum, sdqNum, localPortNum, &foundIndex, GT_FALSE) == GT_OK)
            {
                localPortMapped = GT_TRUE;
            }
            /*enable/disable  the port*/
            rc = prvCpssFalconTxqSdqLocalPortEnableSet(devNum, tileNum, sdqNum, localPortNum, localPortMapped ? GT_TRUE : GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "localPortNum %d \n", localPortNum );
            }
            if (GT_TRUE == localPortMapped)
            {
                PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pNodePtr,devNum,tileNum,foundIndex);

                i           = pNodePtr->sdqQueueFirst;
                range.lowQueueNumber    = i;
                range.hiQueueNumber = pNodePtr->sdqQueueLast;
                rc          = prvCpssFalconTxqSdqLocalPortQueueRangeSet(devNum, tileNum, sdqNum, localPortNum, range);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "localPortNum %d \n", localPortNum );
                }
                for (i = 0; i < pNodePtr->aNodeListSize; i++)
                {
                    for (j = pNodePtr->aNodelist[i].queuesData.pdqQueueFirst,tc =0;
                         j <= pNodePtr->aNodelist[i].queuesData.pdqQueueLast;
                         j++,tc++)
                    {
                        rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum, tileNum, sdqNum, &firstQueueInDp);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                        }
                        localQueueIndex = j - firstQueueInDp;

                        rc  = prvCpssFalconTxqSdqQueueTcSet(devNum, tileNum, sdqNum, localQueueIndex, tc&0xF/*we have 16 priorities*/);
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                        }
                    }
                }
            }
        }
        /*SIP 6_10 use HW defaults ,so no need to configure*/
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
        {
            switch (totalTiles)
            {
            case 1:
                pbFullLimit = 190;
                break;
            case 2:
                pbFullLimit = 381;
                break;
            case 3:
                pbFullLimit = 571;
                break;
            case 4:
                pbFullLimit = 763;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "totalTiles illigal number %d ", totalTiles);
            }
            rc = prvCpssDxChTxqFalconPdsMaxPdsSizeLimitSet(devNum, tileNum, sdqNum, 0xBA4, 0xBE4, pbFullLimit);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxqFalconPdsMaxPdsSizeLimitSet fail for tile %d dp %d ", tileNum, sdqNum);
            }
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsInitPsi function
 * @endinternal
 *
 * @brief   Initialize PSI (write to HW) accoring to SW txq shadow
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                  -Then number of tile (Applicable range 0..3)
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsInitPsi
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      sdqNum, foundIndex;
    GT_U32      localPortNum;
    GT_U32      firstQueueInDp;
    GT_U32      credit;
    GT_BOOL     dpActive;
    credit  =  prvCpssTxqUtilsCreditSizeGet(devNum);

    rc  = prvCpssFalconPsiCreditValueSet(devNum, tileNum, credit);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc  = prvCpssSip6PsiClockGatingEnableSet(devNum,GT_TRUE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    for (sdqNum = 0; sdqNum < MAX_DP_IN_TILE(devNum); sdqNum++)
    {
        rc = prvCpssFalconTxqUtilsDpActiveStateGet(devNum, tileNum, sdqNum, &dpActive);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        /*scip this DP if it does not contain any queues*/
        if (dpActive == GT_FALSE)
        {
            continue;
        }
        rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum, tileNum, sdqNum, &firstQueueInDp);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        for (localPortNum = 0; localPortNum < CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum); localPortNum++)
        {
            PRV_CPSS_TXQ_UTILS_SKIP_RESERVED_MAC(devNum,localPortNum)
            if (prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget
                    (devNum, tileNum, sdqNum, localPortNum, &foundIndex, GT_FALSE) == GT_OK)
            {
                rc = prvCpssFalconPsiMapPortToPdqQueuesSet(devNum, tileNum, foundIndex,
                                       firstQueueInDp, sdqNum, localPortNum);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsBuildPdqTree function
 * @endinternal
 *
 * @brief   Build default schedualing tree
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNumber               -Then number of tile (Applicable range 0..3)
 * @param[in] configPtr                - pointer to SW mapping shadow
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsBuildPdqTree
(
    IN GT_U8 devNum,
    IN GT_U32 tileNumber,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION  * configPtr
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_CPSS_PDQ_SCHED_LEVEL_PERIODIC_PARAMS_STC    params[5];
    GT_U32                      ii, i, j;
    PRV_CPSS_PDQ_SCHD_C_NODE_PARAMS_STC     cParams;
    PRV_CPSS_PDQ_SCHD_B_NODE_PARAMS_STC     bParams;
    PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION * mappingPtr;
    PRV_CPSS_PDQ_SCHD_PORT_PARAMS_STC       portParams;
    GT_U32                      aNodesSize, qNodesSize;
    GT_U32                      aNodeIndex, aNodeIndexNew = 0, portIndex;
    PRV_CPSS_PDQ_SCHD_A_NODE_PARAMS_STC     aParams;
    GT_U32                      bNodeIndex, cNodeIndex;
    GT_U32                      queueIndex, queueIndexNew, queuePriority;
    PRV_CPSS_PDQ_SCHD_QUEUE_PARAMS_STC      queueParams;
    PRV_CPSS_SCHED_PORT_TO_Q_CREATE_OPTIONS_STC createOpt;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE          *pNodePtr = NULL;
    GT_U32                      creditSize = prvCpssTxqUtilsCreditSizeGet(devNum);

    mappingPtr = &(configPtr->mapping);
    if(0 == mappingPtr->size)
    {   /*empty tile*/
        return GT_OK;
    }

    rc = prvCpssFalconTxqPdqInit(devNum, tileNumber, creditSize);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "pdqNum %d \n", 0 );
    }




    /*****************************************************/
    /* Configure Periodic Scheme    - no shaping                     */
    /*****************************************************/
    /*port level*/
    params[CPSS_TM_LEVEL_P_E].periodicState     = GT_FALSE;
    params[CPSS_TM_LEVEL_P_E].shaperDecoupling  = GT_FALSE;
    /* c level*/
    params[CPSS_TM_LEVEL_C_E].periodicState     = GT_FALSE;
    params[CPSS_TM_LEVEL_C_E].shaperDecoupling  = GT_FALSE;
    /* b level*/
    params[CPSS_TM_LEVEL_B_E].periodicState     = GT_TRUE;
    params[CPSS_TM_LEVEL_B_E].shaperDecoupling  = GT_FALSE;
    /* a level*/
    params[CPSS_TM_LEVEL_A_E].periodicState     = GT_TRUE;
    params[CPSS_TM_LEVEL_A_E].shaperDecoupling  = GT_FALSE;
    /* q level*/
    params[CPSS_TM_LEVEL_Q_E].periodicState     = GT_TRUE;
    params[CPSS_TM_LEVEL_Q_E].shaperDecoupling  = GT_FALSE;
    rc                      = prvCpssFalconTxqPdqPeriodicSchemeConfig(devNum, tileNumber, params);
    if (rc != GT_OK)
    {
        return rc;
    }
    /*****************************************************/
    /* Configure C parameters                        */
    /*****************************************************/
    cParams.shapingProfilePtr   = CPSS_PDQ_SCHED_SHAPING_INFINITE_PROFILE_INDEX_CNS;
    cParams.quantum         = CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum);
    cParams.eligiblePrioFuncId  = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_C_NODE;
    cParams.numOfChildren       = 1;
    for (ii = 0; ii < 8; ii++) {
        cParams.schdModeArr[ii] = CPSS_PDQ_SCHD_MODE_RR_E; /* No DWRR configured*/
    }
    /*****************************************************/
    /* Configure B parameters                        */
    /*****************************************************/
    bParams.shapingProfilePtr   = CPSS_PDQ_SCHED_SHAPING_INFINITE_PROFILE_INDEX_CNS;
    bParams.quantum         = CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum);
    bParams.eligiblePrioFuncId  = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_B_NODE;
    for (ii = 0; ii < 8; ii++) {
        bParams.schdModeArr[ii] = CPSS_PDQ_SCHD_MODE_DWRR_E; /* DWRR configured*/
    }

    /*create all ports - no shaping*/
    for (i = 0; i < mappingPtr->size; i++)
    {
        PRV_TXQ_SIP_6_PNODE_INSTANCE_NO_ERROR_GET(pNodePtr,devNum,tileNumber,i);
        if (NULL== pNodePtr)
        {
            /*this local port is not mapped*/
            continue;
        }
        pNodePtr->pNodeIndex = i;

        portParams.cirBw        = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
        portParams.eirBw        = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
        portParams.cbs          = CPSS_SIP6_TXQ_INVAL_DATA_CNS; /*KBytes*/
        portParams.ebs          = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
        portParams.numOfChildren    = 1;                            /*one C level*/
        for (j = 0; j < 8; j++)
        {                                                               /*the value is already in chunks ,no need to multiply by chunk size*/
            portParams.quantumArr[j] = CPSS_PDQ_SCHED_MIN_PORT_QUANTUM_CNS;
            /*C level scheduling ,so RR*/
            portParams.schdModeArr[j] = CPSS_TM_SCHD_MODE_RR_E;
        }
        portParams.eligiblePrioFuncId   = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_P_NODE;
        rc              = prvCpssFalconTxqPdqAsymPortCreate(devNum, tileNumber, i, GT_TRUE /*fixed mapping to C node*/, &portParams);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    /*now create all A levels and connect them to port*/
    /*count all A nodes*/
    for (i = 0, aNodesSize = 0; i < mappingPtr->size; i++)
    {
        PRV_TXQ_SIP_6_PNODE_INSTANCE_NO_ERROR_GET(pNodePtr,devNum,tileNumber,i);

        if (NULL== pNodePtr)
        {
            /*this local port is not mapped*/
            continue;
        }
        aNodesSize +=  pNodePtr->aNodeListSize;
    }
    for (i = 0, aNodeIndex = 0; i < aNodesSize; i++, aNodeIndex++)
    {
        /*****************************************************/
        /* Configure A parameters                        */
        /*****************************************************/
        aParams.shapingProfilePtr   = CPSS_PDQ_SCHED_SHAPING_INFINITE_PROFILE_INDEX_CNS; /*no shaping*/;
        aParams.quantum         = CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum);
        for (ii = 0; ii < 7; ii++) {
            aParams.schdModeArr[ii] = CPSS_TM_SCHD_MODE_DWRR_E; /*  DWRR configured*/
        }
        aParams.schdModeArr[7] = CPSS_TM_SCHD_MODE_RR_E; /*  RR configured to priority 7 in order to support guaranteed BW*/
        aParams.eligiblePrioFuncId = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_A_NODE(0);
        do
        {
            rc  = prvCpssSip6TxqAnodeToPnodeIndexGet(devNum,tileNumber,aNodeIndex,&portIndex);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqAnodeToPnodeIndexGet failed for index %d",aNodeIndex);
            }
            PRV_TXQ_SIP_6_PNODE_INSTANCE_NO_ERROR_GET(pNodePtr,devNum,tileNumber,portIndex);
            if (NULL==pNodePtr)
            {
                /*This A node is not mapped*/
                aNodeIndex++;
            }
            /*avoid infinite loop*/
            if (aNodeIndex == CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC)
            {
                break;
            }
        }
        while(portIndex == CPSS_SIP6_TXQ_INVAL_DATA_CNS);

        PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pNodePtr,devNum,tileNumber,portIndex);

        if (aNodeIndex == CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Mapping error at interation %d for creation of a node\n", i );
        }
        for (ii = 0; ii < pNodePtr->aNodeListSize; ii++)
        {
            if (pNodePtr->aNodelist[ii].aNodeIndex == aNodeIndex)
            {
                break;
            }
        }
        queueIndex = pNodePtr->aNodelist[ii].queuesData.pdqQueueFirst;
        /*Fill requested creation options*/
        /*check if need to create or add new*/
        if (ii == 0)
        {
            createOpt.addToExisting = GT_FALSE;
        }
        else
        {
            createOpt.addToExisting = GT_TRUE;
        }
        createOpt.explicitQueueStart        = GT_TRUE;
        createOpt.queueStart            = queueIndex;
        createOpt.fixedPortToBnodeMapping   = GT_TRUE;
        createOpt.explicitAnodeInd      = PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNumber].mapping.aNodeAllignedMapping;
        createOpt.aNodeInd          = aNodeIndex;
        aParams.numOfChildren           = pNodePtr->aNodelist[ii].queuesData.pdqQueueLast - queueIndex + 1;
        /*Support remote ports*/
        bParams.numOfChildren   = pNodePtr->aNodeListSize;
        rc          = prvCpssFalconTxqPdqAnodeToPortCreate(devNum, tileNumber, portIndex,
                                           &aParams, &bParams, &cParams,
                                           &createOpt,
                                           &aNodeIndexNew, &bNodeIndex, &cNodeIndex);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "portIndex %d \n", portIndex );
        }
        if (aNodeIndexNew != aNodeIndex)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "aNodeIndexNew [%d]  != aNodeIndex [%d]\n", aNodeIndexNew, aNodeIndex);
        }
    }
    /*now create all queue levels and connect them to A level*/
    /*****************************************************/
    /* Configure Queue parameters                        */
    /*****************************************************/
    queueParams.shapingProfilePtr   = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
    queueParams.quantum     = CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum);
    aNodeIndex          = aNodeIndexNew;
    rc  = prvCpssSip6TxqAnodeToPnodeIndexGet(devNum,tileNumber,aNodeIndex,&portIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqAnodeToPnodeIndexGet failed for index %d",aNodeIndex);
    }

    qNodesSize          = 0;
    /*Check that there is some mapping for this tile*/
    if (portIndex != CPSS_SIP6_TXQ_INVAL_DATA_CNS )
    {
        /*sanity,at least one pNode should be present.Here we are refer to  the last Pnode in the tile*/
        if(NULL == pNodePtr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "P node is NULL pointer\n");\
        }
        qNodesSize = pNodePtr->aNodelist[pNodePtr->aNodeListSize - 1].queuesData.pdqQueueLast + 1;
    }
    for (i = 0; i < qNodesSize; i++)
    {
        rc = prvCpssSip6TxqQnodeToAnodeIndexGet(devNum,tileNumber,i,&aNodeIndex);

        if (rc!=GT_OK||aNodeIndex == CPSS_SIP6_TXQ_INVAL_DATA_CNS)
        {       /*unmapped*/
            continue;
        }
        queueIndex          = i;
        queuePriority           = queueIndex % 8;
        queueParams.eligiblePrioFuncId  = PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_Q_NODE(0);
        /*To be alligned to current CPSS init*/
        if (queuePriority < 4)
        {
            queueParams.quantum = CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum);
        }
        else
        {
            queueParams.quantum = 5 * CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum);
        }

        if (aNodeIndex != aNodeIndexNew)
        {
            aNodeIndexNew = aNodeIndex;
        }
        queueIndexNew   = queueIndex;
        rc      = prvCpssFalconTxqPdqQueueToAnodeCreate(devNum, tileNumber, aNodeIndex, &queueParams, GT_TRUE, &queueIndexNew);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (queueIndexNew != queueIndex)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "queueIndexNew [%d]  != queueIndex [%d]\n", queueIndexNew, queueIndex);
        }
    }
    /*Disable  DWRR at P level*/
    rc =  prvCpssFalconTxqPdqTreePlevelDwrrEnableSet(devNum, tileNumber, GT_FALSE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "prvCpssFalconTxqPdqTreePlevelDwrrEnableSet failed for tile %d \n", tileNumber);
    }
    return rc;
}

/**
 * @internal prvCpssFalconTxqUtilsPortTxBufNumberGet function
 * @endinternal
 *
 * @brief   Gets the current number of buffers allocated per specified port.
 * Note that in case port is in preemptive mode function return sum of both express and preemptive channel
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 *                                      portNum  - physical or CPU port number
 *
 * @param[out] numPtr                   - number of buffers
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqUtilsPortTxBufNumberGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    OUT GT_U32 *numPtr
)
{
    GT_STATUS           rc;
    GT_U32              tileNum, dpNum, localPortNum[2];
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    GT_BOOL             portInPreemptiveMode    = GT_FALSE,portMapped =GT_FALSE,portContainMac=GT_FALSE;
    GT_U32              bufNum[2]       = { 0, 0 };
    GT_U32              speedInMb = 0;
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, physicalPortNum, &tileNum, &dpNum, localPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    if (mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {     /*local or CPU */
        rc = prvCpssFalconTxqQfcBufNumberGet(devNum, tileNum, dpNum, localPortNum[0], GT_TRUE, bufNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail in prvCpssFalconTxqQfcBufNumberGet");
        }

        rc = prvCpssSip6TxqUtilsPortParametersGet(devNum,physicalPortNum,&portMapped,&portContainMac);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(GT_TRUE==portMapped &&GT_TRUE==portContainMac)
        {
            /*check if port in preemptive mode*/
            PRV_CPSS_DXCH_CURRENT_PORT_SPEED_IN_MB_MAC(devNum, physicalPortNum,speedInMb);

            rc = prvCpssDxChTxqSip6_10PreemptionEnableGet(devNum, physicalPortNum,
             speedInMb,
             &portInPreemptiveMode);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail in prvCpssDxChTxqSip6_10PreemptionEnableGet for port %d", physicalPortNum);
            }
            if (GT_TRUE == portInPreemptiveMode)
            {
                rc = prvCpssDxChTxqSip6_10PreChannelGet(devNum,0,localPortNum[0], &localPortNum[1]);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail in prvCpssDxChTxqSip6_10PreChannelGet for local port %d", localPortNum[0]);
                }
                rc = prvCpssFalconTxqQfcBufNumberGet(devNum, tileNum, dpNum, localPortNum[1], GT_TRUE, &bufNum[1]);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail in prvCpssFalconTxqQfcBufNumberGet");
                }
            }
         }

        *numPtr = bufNum[0] + bufNum[1];
    }
    else
    {
        /*we do not have counters for remote port.So return GT_NOT_SUPPORTED*/
        rc = GT_NOT_SUPPORTED;
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqUtilsQueueBufPerTcNumberGet function
 * @endinternal
 *
 * @brief   Gets the current number of buffers allocated on specified port
 *         for specified Traffic Class queues.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 * @param[in] portNum                  - physical or CPU port number
 * @param[in] trafClass                - trafiic class (0..7)
 *
 * @param[out] numPtr                   - number of buffers
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqUtilsCounterPerPortTcGet
(
    IN GT_U8 devNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_COUNTER_TYPE_ENT counterType,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U8 trafClass,
    OUT GT_U32                                      *numPtr
)
{
    GT_U32                  tileNum, dpNum, localPortNum, queueNum;
    GT_STATUS               rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *   portMapShadowPtr;/*port mapping shadow*/
    /*Find tile*/
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &aNodePtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    *numPtr = 0;
    if (PRV_CPSS_DXCH_TXQ_SIP_6_COUNTER_TYPE_OCCUPIED_BUFFERS == counterType)
    {
        queueNum    = aNodePtr->queuesData.queueBase + trafClass;
        rc      = prvCpssFalconTxqQfcBufNumberGet(devNum, tileNum, dpNum, queueNum, GT_FALSE, numPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail in prvCpssFalconTxqQfcQueueBufNumberGet");
        }
    }
    else if (PRV_CPSS_DXCH_TXQ_SIP_6_COUNTER_TYPE_HEADROOM_BUFFERS == counterType)
    {
        if (mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Headroom counter is not supported for remote port");
        }
        rc = prvCpssFalconTxqQfcHeadroomCounterGet(devNum, tileNum, dpNum, localPortNum, trafClass, numPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail in prvCpssFalconTxqQfcHeadroomCounterGet");
        }
    }
    else if (PRV_CPSS_DXCH_TXQ_SIP_6_COUNTER_TYPE_PFC_BUFFERS == counterType)
    {
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/ &portMapShadowPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortPhysicalPortMapShadowDBGet failed for port %d",
                              portNum );
        }
        if (portMapShadowPtr->valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "portMapShadowPtr->valid == GT_FALSE for port %d",
                              portNum );
        }
        rc = prvCpssFalconTxqPfccPortTcCounterGet(devNum, PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC, trafClass,
                              portMapShadowPtr->portMap.rxDmaNum, portNum, numPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccCounterGet failed for port %d",
                              portNum );
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Counter type not supported");
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsQueueEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable queue
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - device number.
 * @param[in] portNum                  - physical or CPU port number.
 * @param[in] tcQueue                  - traffic class queue on this device (0..7).
 * @param[in] enable                   - if GT_TRUE queue  is enabled , otherwise disabled.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsQueueEnableSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U8 tcQueue,
    IN GT_BOOL enable
)
{
    GT_U32                      tileNum, dpNum, localPortNum;
    GT_STATUS                   rc;
    GT_BOOL                     preemptionEnabled = GT_FALSE,actAsPreemptiveChannel = GT_FALSE;
    GT_U32                      preemptiveLocalDpPortNum = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    GT_PHYSICAL_PORT_NUM        preemptivePhysicalPortNum;

     /*check that port does not serve as additional preeemption  channel*/
     rc = prvCpssTxqPreemptionUtilsPortConfigurationAllowedGet(devNum,portNum);
     if(rc!=GT_OK)
     {
         return rc;
     }

    /*Find tile*/
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localPortNum, NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

   if (TXQ_IS_PREEMPTIVE_DEVICE(devNum)&&(GT_FALSE == PRV_CPSS_PP_MAC(devNum)->isGmDevice))
   {
       /*check preemption*/
       rc = prvCpssSip6TxqUtilsPreemptionStatusGet(devNum, portNum,&preemptionEnabled,&actAsPreemptiveChannel,
                        &preemptivePhysicalPortNum,&preemptiveLocalDpPortNum);
       if (rc != GT_OK)
       {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsPreemptionStatusGet  failed for portNum  %d  ", portNum);
       }

       if(GT_TRUE==actAsPreemptiveChannel)
       {
           /*can not configure port that is serve as preemptive channel*/
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
       }
    }
    rc = prvCpssFalconTxqSdqQueueEnableSet(devNum, tileNum, dpNum, localPortNum, tcQueue, enable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail in prvCpssFalconTxqSdqQueueEnableSet");
    }

    if(GT_TRUE== preemptionEnabled)
    {
        rc = prvCpssFalconTxqSdqQueueEnableSet(devNum, tileNum, dpNum, preemptiveLocalDpPortNum, tcQueue, enable);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail in prvCpssFalconTxqSdqQueueEnableSet");
        }
    }

    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsQueueEnableGet function
 * @endinternal
 *
 * @brief   Get Enable/disable queue status
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -  device number
 * @param[in] portNum                  - physical or CPU port number.
 * @param[in] tcQueue                  - traffic class queue on this device (0..7).
 *
 * @param[out] enablePtr                - if GT_TRUE queue  is enabled , otherwise disabled
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsQueueEnableGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U8 tcQueue,
    IN GT_BOOL * enablePtr
)
{
    GT_U32      tileNum, dpNum, localPortNum;
    GT_STATUS   rc;
    /*Find tile*/
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localPortNum, NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssFalconTxqSdqQueueEnableGet(devNum, tileNum, dpNum, localPortNum, tcQueue, enablePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail in prvCpssFalconTxqSdqQueueEnableGet");
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsSetDmaToPnodeMapping function
 * @endinternal
 *
 * @brief    Write entry to DMA to Pnode data base
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @param[in] devNum                   -  device number
 * @param[in] dmaNum                  - Global DMA number(0..263).
 * @param[in] tileNum                  - traffic class queue on this device (0..7).
 * @param[in] pNodeNum                  - Index of P node
 */
static GT_STATUS prvCpssFalconTxqUtilsSetDmaToPnodeMapping
(
    IN GT_U8 devNum,
    IN GT_U32 dmaNum,
    IN GT_U32 tileNum,
    IN GT_U32 pNodeNum
)
{
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    if ((dmaNum >= PRV_CPSS_MAX_DMA_NUM_CNS) || pNodeNum >= CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Fail in prvCpssFalconTxqUtilsSetDmaToPnodeMapping");
    }
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[dmaNum].pNodeInd  = pNodeNum;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[dmaNum].tileInd   = tileNum;
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsGetDmaToPnodeMapping function
 * @endinternal
 *
 * @brief    Read entry from DMA to Pnode data base
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @param[in] devNum                   -  device number
 * @param[in] dmaNum                  - Global DMA number(0..263).
 * @param[in] tileNum                  - traffic class queue on this device (0..7).
 * @param[out] pNodeNum                  - Index of P node
 * @param[in] isErrorForLog            - indication the CPSS ERROR should be in the LOG or not

 */
static GT_STATUS prvCpssFalconTxqUtilsGetDmaToPnodeMapping
(
    IN GT_U8 devNum,
    IN GT_U32 dmaNum,
    OUT GT_U32   *tileNumPtr,
    OUT GT_U32   * pNodeNumPtr,
    IN GT_BOOL isErrorForLog
)
{
    if (dmaNum >= PRV_CPSS_MAX_DMA_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "dmaNum[%d] >= 'max'[%d]",
                          dmaNum, PRV_CPSS_MAX_DMA_NUM_CNS);
    }
    if (tileNumPtr)
    {
        *tileNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[dmaNum].tileInd;
    }
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[dmaNum].pNodeInd == CPSS_SIP6_TXQ_INVAL_DATA_CNS)
    {
        if (GT_FALSE == isErrorForLog)
        {
            return /* not error for the LOG */ GT_NOT_FOUND;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "dmaNum[%d] not hold valid pNodeInd",
                              dmaNum);
        }
    }
    *pNodeNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[dmaNum].pNodeInd;
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingSet function
 * @endinternal
 *
 * @brief    Write entry to physical port to Anode index data base
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @param[in] devNum                   -  device number
 * @param[in] dmaNum                  - Global DMA number(0..263).
 * @param[in] tileNum                  - traffic class queue on this device (0..7).
 * @param[in] aNodeNum                  - Index of A node
 */
static GT_STATUS prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingSet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNumber,
    IN GT_U32 tileNum,
    IN GT_U32 aNodeNum
)
{
    GT_STATUS               rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *   portMapShadowPtr;
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physicalPortNumber, /*OUT*/ &portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNumber].aNodeInd = aNodeNum;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNumber].tileInd  = tileNum;
    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqUtilsInitPhysicalPortToAnodeDb function
 * @endinternal
 *
 * @brief  Initialize physical port to A nodedatabase
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                  -  device number
 */
GT_VOID  prvCpssFalconTxqUtilsInitPhysicalPortToAnodeDb
(
    IN GT_U8 devNum
)
{
    GT_U32 i;
    for (i = 0; i < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; i++)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[i].aNodeInd  = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[i].tileInd   = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    }
}
/**
 * @internal prvCpssFalconTxqUtilsGetQueueGroupAttributes function
 * @endinternal
 *
 * @brief  Get attributes of queue group
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @param[in] devNum                  -  device number
 * @param[in] queueGroup           - queue group index
 * @param[out] tilePtr                   - tile of the queue group
 * @param[out] dpPtr  - dp of the queue group
 * @param[out]  queueBaseInDpPtr - local port of the queue group
 * @param[out] queueBaseInTilePtr - global queue index (per PDQ)
 */
GT_STATUS  prvCpssFalconTxqUtilsGetQueueGroupAttributes
(
    IN GT_U8 devNum,
    IN GT_U32 queueGroup,
    OUT GT_U32 * tilePtr,
    OUT GT_U32 * dpPtr,
    OUT GT_U32 * localPortPtr,
    OUT GT_U32 * queueBaseInDpPtr,
    OUT GT_U32 * queueBaseInTilePtr
)
{
    PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC  txPortMap;
    GT_STATUS                       rc;
    GT_U32                          pNodeIndex, i;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE                  *pNodePtr = NULL;
    rc = prvCpssDxChTxqFalconPdxQueueGroupMapGet(devNum, queueGroup, &txPortMap);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Error in prvCpssFalconTxqUtilsGetQueueGroupAttributes" );
    }
    *tilePtr        = txPortMap.queuePdxIndex;
    *dpPtr          = txPortMap.queuePdsIndex;
    *localPortPtr       = txPortMap.dpCoreLocalTrgPort;
    *queueBaseInDpPtr   = txPortMap.queueBase;
    /*calculate P node index*/
    rc  = prvCpssSip6TxqUtilsPnodeIndexGet(devNum, *dpPtr, *localPortPtr,&pNodeIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqUtilsPnodeIndexGet failed ");
    }

    PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pNodePtr,devNum,*tilePtr ,pNodeIndex);

    /*find A node*/
    for (i = 0; i < pNodePtr->aNodeListSize; i++)
    {
        if ((pNodePtr->aNodelist[i].queuesData.queueGroupIndex == queueGroup)
            ||  (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType == CPSS_CHANNEL_PEX_FALCON_Z_E) /* Not relevant for FalconZ since it has no Eagle hw */)
        {
            *queueBaseInTilePtr = pNodePtr->aNodelist[i].queuesData.pdqQueueFirst;
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}
/**
 * @internal prvCpssFalconTxqUtilsIsCascadePort function
 * @endinternal
 *
 * @brief  Check if port has queues mapped at TxQ
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @param[in] devNum                  -  device number
 * @param[in] physicalPortNum           -port number
 * @param[out] isCascadePtr  - if equal GT_TRUE this port is cascade,GT_FALSE otherwise

 */
GT_STATUS prvCpssFalconTxqUtilsIsCascadePort
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    OUT GT_BOOL * isCascadePtr,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE **pNodePtr
)
{
    GT_STATUS               rc;
    /*In order to avoid compiler warning initialize pNode*/
    GT_U32                  pNode   = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    GT_U32                  tileNum = 0;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *   portMapShadowPtr;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE  * outPtr =NULL;

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physicalPortNum, /*OUT*/ &portMapShadowPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if (portMapShadowPtr->valid)
    {
        if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            *isCascadePtr = GT_FALSE;
            return GT_OK;
        }
        rc = prvCpssFalconTxqUtilsGetDmaToPnodeMapping(devNum, portMapShadowPtr->portMap.txDmaNum, &tileNum, &pNode, GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(outPtr,devNum,tileNum,pNode);

        if(isCascadePtr)
        {
            *isCascadePtr   = GT_FALSE;
            if (outPtr->isCascade)
            {
                if (outPtr->cascadePhysicalPort == physicalPortNum)
                {
                    *isCascadePtr = GT_TRUE;
                }
            }
        }

        if (pNodePtr)
        {
            *pNodePtr = outPtr;
        }
        return GT_OK;
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

GT_STATUS prvCpssSip6TxqUtilsPhysicalPortToPnodeIndexGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    OUT GT_BOOL * isCascadePtr,
    OUT GT_U32  *pNodeIndPtr,
    OUT GT_U32  *tileIndPtr
)
{
    GT_STATUS               rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr;
    GT_U32 tileNum,aNodeIndex;
    CPSS_NULL_PTR_CHECK_MAC(isCascadePtr)
    CPSS_NULL_PTR_CHECK_MAC(pNodeIndPtr)
    CPSS_NULL_PTR_CHECK_MAC(tileIndPtr)


    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,physicalPortNum,isCascadePtr,&pNodePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(GT_TRUE == *isCascadePtr)
    {
        *pNodeIndPtr = pNodePtr->pNodeIndex;
        *tileIndPtr = pNodePtr->aNodelist[0].queuesData.tileNum;
    }
    else
    {
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &aNodeIndex);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        rc  = prvCpssSip6TxqAnodeToPnodeIndexGet(devNum,tileNum,aNodeIndex,pNodeIndPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqAnodeToPnodeIndexGet failed for index %d",aNodeIndex);
        }
    }

    return GT_OK;

}


/**
 * @internal prvCpssFalconTxqGlobalTxEnableSet function
 * @endinternal
 *
 * @brief   Enable/Disable transmission of specified device.
 *                Set enable/disable on all tiles (1/2/4)
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 */
GT_STATUS   prvCpssFalconTxqGlobalTxEnableSet
(
    IN GT_U8 devNum,
    IN GT_BOOL enable
)
{
    GT_U32      numberOfTiles, i;
    GT_STATUS   rc;
    switch (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
    case 0:
    case 1:
        numberOfTiles = 1;
        break;
    case 2:
    case 4:
        numberOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d  ", PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
        break;
    }
    for (i = 0; i < numberOfTiles; i++)
    {
        /*set all tiles to same value*/
        rc = prvCpssFalconTxqPdqGlobalTxEnableSet(devNum, i, enable);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqGlobalTxEnableSet failed for tile - %d", i);
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqGlobalTxEnableGet function
 * @endinternal
 *
 * @brief   Get the status of transmission of specified device (Enable/Disable).
 *                Note this function check the status in all availeble tiles (The status should be the SAME)
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
 *
 * @param[in] devNum                   - physical device number
 *
 * @param[out] enablePtr                - (pointer to)
 *                                      GT_TRUE, enable transmission
 *                                      GT_FALSE, disable transmission
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters in NULL pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS   prvCpssFalconTxqGlobalTxEnableGet
(
    IN GT_U8 devNum,
    IN GT_BOOL  *enablePtr
)
{
    GT_U32      numberOfTiles, i;
    GT_STATUS   rc;
    GT_BOOL     enablePerTile[4]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    *enablePtr = GT_FALSE;

    switch (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
    case 0:
    case 1:
        numberOfTiles = 1;
        break;
    case 2:
    case 4:
        numberOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d  ", PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
        break;
    }
    for (i = 0; i < numberOfTiles; i++)
    {
        /*read from all tiles */
        rc = prvCpssFalconTxqPdqGlobalTxEnableGet(devNum, i, enablePerTile + i);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGlobalTxEnableGet failed for tile - %d", i);
        }
    }
    /*check that all tiles are synced */
    for (i = 1; i < numberOfTiles; i++)
    {
        if (enablePerTile[i] != enablePerTile[i - 1])
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " GlobalTxEnable has different values at tile %d  and  tile - %d", i, i - 1);
        }
    }
    *enablePtr = *enablePerTile;
    return GT_OK;
}

/**
 * @internal prvCpssDxChTxqBindQueueToProfileSet function
 * @endinternal
 *
 * @brief Bind queue to queue  profile( Long queue and Lenght adjust)
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] tc                      - traffic class (0..7)
 * @param[in] profileIndex           - index of profile (0..15)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvCpssDxChTxqBindQueueToProfileSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U8 tc,
    IN GT_U32 profileIndex
)
{
    GT_STATUS               rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    anodePtr;
    GT_U32                  queueNum;
    GT_U32                  tileNum;
    GT_U32                  pdsNum;
    rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, portNum, &tileNum, &queueNum /*just for dummy*/);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet  failed for portNum  %d  ", portNum);
    }
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &anodePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    queueNum    = anodePtr->queuesData.queueBase + tc;
    pdsNum      = anodePtr->queuesData.dp;
    rc      = prvCpssDxChTxqFalconPdsQueueProfileMapSet(devNum, tileNum, pdsNum, queueNum, profileIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsQueueProfileMapSet  failed for queueNum  %d  ", queueNum);
    }
    return rc;
}
/**
 * @internal prvCpssDxChTxqBindQueueToLengthAdjustProfileGet function
 * @endinternal
 *
 * @brief Get binded  queue  profile index(  Lenght adjust)
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] tc                      - traffic class (0..7)
 * @param[out] profileIndexPtr           - (pointer to)index of profile (0..15)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvCpssDxChTxqBindQueueToLengthAdjustProfileGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U8 tc,
    OUT GT_U32  * profileIndexPtr
)
{
    GT_STATUS               rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    anodePtr;
    GT_U32                  queueNum;
    GT_U32                  tileNum;
    GT_U32                  pdsNum;
    rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, portNum, &tileNum, &queueNum /*just for dummy*/);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet  failed for portNum  %d  ", portNum);
    }
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &anodePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    queueNum    = anodePtr->queuesData.queueBase + tc;
    pdsNum      = anodePtr->queuesData.dp;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
    {
        rc = prvCpssDxChTxqFalconPdsQueueProfileMapGet(devNum, tileNum, pdsNum, queueNum, profileIndexPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsQueueProfileMapGet  failed for queueNum  %d  ", queueNum);
        }
    }
    else
    {
        rc = prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapGet(devNum, tileNum, pdsNum, queueNum, profileIndexPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapGet  failed for queueNum  %d  ", queueNum);
        }
    }
    return rc;
}
/**
 * @internal prvCpssDxChTxqSetPortSdqThresholds function
 * @endinternal
 *
 * @brief Set port and queue thresholds (credit allocation requests to PDQ)
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] speed                      - port speed
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 *
 * @note   NOTE:      Remote ports are not handled.The configuration for remote ports should be done while configuring cascade port.
 */
GT_STATUS prvCpssDxChTxqSetPortSdqThresholds
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_SPEED_ENT speed
)
{
    GT_STATUS               rc;
    GT_U32                  tileNum;
    GT_U32                  i,aNodeIterator, lastQueue;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    anodePtr = NULL;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *    pnodePtr;
    GT_U32                  dummy = 0, localdpPortNum, dpNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
    GT_BOOL                 isCascade;
    GT_U32                  pNodeNum, aNodeListSize;

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    /*Threshold set only for local ports ,remote ports thresholds will be set via cascade port*/
    if (mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum, &isCascade, NULL);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsIsCascadePort  failed for portNum  %d  ", portNum);
        }
        if (isCascade == GT_TRUE)
        {
            rc = prvCpssSip6TxqUtilsPnodeIndexGet(devNum,dpNum,localdpPortNum,&pNodeNum);
            if (rc != GT_OK)
            {
               CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsPnodeIndexGet failed for portNum  %d  ", portNum);
            }
            PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pnodePtr,devNum,tileNum,pNodeNum);

            aNodeListSize   = pnodePtr->aNodeListSize;
            anodePtr = &(pnodePtr->aNodelist[0]);
        }
        else
        {
            aNodeListSize = 1;
            rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, portNum, &tileNum, &dummy);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet  failed for portNum  %d  ", portNum);
            }
            rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &anodePtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
            }
        }

        for(aNodeIterator=0;aNodeIterator<aNodeListSize;anodePtr++, aNodeIterator++)
        {
           if(anodePtr==NULL)
           {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Bad pointer  ");
           }

           i       = anodePtr->queuesData.queueBase;
           lastQueue   = i + (anodePtr->queuesData.pdqQueueLast - anodePtr->queuesData.pdqQueueFirst);
           /*set queue thresholds */
          for (; i <= lastQueue; i++)
          {
            rc = prvCpssFalconTxqSdqQueueAttributesSet(devNum, tileNum, dpNum, i, speed,(anodePtr->queuesData.semiEligBmp&1<<i)==0?GT_FALSE:GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqQueueAttributesSet  failed for q  %d  ", i);
            }
          }
        }
        /*set port BP thresholds */
        rc = prvCpssFalconTxqSdqPortAttributesSet(devNum, tileNum, dpNum, localdpPortNum, speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqPortAttributesSet  failed for local port %d",localdpPortNum );
        }
    }
    return GT_OK;
}
static GT_STATUS prvCpssDxChTxqPdsLongQueueAttributesInit
(
    IN GT_U32 devNum,
    INOUT PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC *profilePtr
)
{
    GT_U32 i;

    CPSS_NULL_PTR_CHECK_MAC(profilePtr);

    for(i=0;i<PRV_CPSS_DXCH_SIP6_TXQ_MAX_NUM_OF_SUPPORTED_SPEEDS_CNS;i++)
    {
        profilePtr[i].longQueueEnable = GT_TRUE;
    }

    i=0;

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
    {
        profilePtr[i++].headEmptyLimit = 16;  /*0- 1*/
        profilePtr[i++].headEmptyLimit = 17;/*1-2.5*/
        profilePtr[i++].headEmptyLimit = 18;/*2- 5*/
        profilePtr[i++].headEmptyLimit = 21;/*3 10*/
        profilePtr[i++].headEmptyLimit = 28;/*4- 25*/
        profilePtr[i++].headEmptyLimit = 34;/*5-40*/
        profilePtr[i++].headEmptyLimit = 39;/*6- 50*/
        profilePtr[i++].headEmptyLimit = 61;/*7- 100*/
        profilePtr[i++].headEmptyLimit = 106;/*8- 200*/
        profilePtr[i  ].headEmptyLimit = 195;/*9-400*/

        i=0;

        profilePtr[i++].longQueueLimit = 18; /*0- 1*/
        profilePtr[i++].longQueueLimit = 19; /*1-2.5*/
        profilePtr[i++].longQueueLimit = 20 ;/*2- 5*/
        profilePtr[i++].longQueueLimit = 24; /*3 10*/
        profilePtr[i++].longQueueLimit = 31; /*4- 25*/
        profilePtr[i++].longQueueLimit = 37 ;/*5-40*/
        profilePtr[i++].longQueueLimit = 43 ;/*6- 50*/
        profilePtr[i++].longQueueLimit = 68 ;/*7- 100*/
        profilePtr[i++].longQueueLimit = 117;/*8- 200*/
        profilePtr[i  ].longQueueLimit = 215;/*9-400*/

    }
    else
    {
        profilePtr[i++] .headEmptyLimit= 16;   /*0 - 1*/
        profilePtr[i++] .headEmptyLimit= 17;   /*1 -2.5*/
        profilePtr[i++] .headEmptyLimit= 17;   /*2 -5*/
        profilePtr[i++] .headEmptyLimit= 19;   /*3- 10*/
        profilePtr[i++] .headEmptyLimit= 22;   /*4 -20*/
        profilePtr[i++] .headEmptyLimit= 23;   /*5 - 25*/
        profilePtr[i++] .headEmptyLimit= 28;   /*6 -40*/
        profilePtr[i++] .headEmptyLimit= 31;   /*7 - 50*/
        profilePtr[i++] .headEmptyLimit= 46;   /*8 - 100*/
        profilePtr[i++] .headEmptyLimit= 48;   /*9 - 106*/
        profilePtr[i++] .headEmptyLimit= 76;   /*10 - 200*/
        profilePtr[i++] .headEmptyLimit= 80;   /*11 - 212*/
        profilePtr[i++] .headEmptyLimit= 135;  /*12 -400*/
        profilePtr[i  ] .headEmptyLimit= 144;  /*13 -424*/

        i=0;

        profilePtr[i++].longQueueLimit = 18;   /*0 - 1*/
        profilePtr[i++].longQueueLimit =18; /*1 -2.5*/
        profilePtr[i++].longQueueLimit =19 ;/*2 -5*/
        profilePtr[i++].longQueueLimit =21 ;/*3- 10*/
        profilePtr[i++].longQueueLimit =24 ;/*4 -20*/
        profilePtr[i++].longQueueLimit =26 ;/*5 - 25*/
        profilePtr[i++].longQueueLimit =31 ;/*6 -40*/
        profilePtr[i++].longQueueLimit =34 ;/*7 - 50*/
        profilePtr[i++].longQueueLimit =50 ;/*8 - 100*/
        profilePtr[i++].longQueueLimit =53 ;/*9 - 106*/
        profilePtr[i++].longQueueLimit =83 ;/*10 - 200*/
        profilePtr[i++].longQueueLimit =88 ;/*11 - 212*/
        profilePtr[i++].longQueueLimit =149; /*12 -400*/
        profilePtr[i  ].longQueueLimit =159 ;/*13 -424*/

    }

    return GT_OK;
}

/**
 * @internal prvCpssDxChTxqInitPdsLongQueueProfiles function
 * @endinternal
 *
 * @brief Init long queue and length adjust profiles (per all the possible speeds)
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 *
 * @note   NOTE: CPSS copy all the profiles to all the DP cores in all tiles
 */
GT_STATUS prvCpssDxChTxqPdsLongQueueAttributesGet
(
    IN GT_U32 devNum,
    IN GT_BOOL speedToProfile,
    INOUT GT_U32   *speedinGPtr,
    INOUT PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC **profilePtr
)
{
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC *   queuePdsProfilesPtr;
    GT_STATUS rc = GT_OK;


    GT_U32                          speedArr_Sip6 []    = { 1,2/*2.5*/,5,10, 25,40,50,100,200,400};
    GT_U32                          speedArr_Sip_6_10 []    = { 1,2/*2.5*/,5,10,20,25,40,50,100,106,200,212,400,424};
    GT_U32 *                        speedArrPtr;
    GT_U32                          arraySize;
    GT_U32                          i;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
    {
        speedArrPtr     = speedArr_Sip6;
        arraySize       = sizeof(speedArr_Sip6) / sizeof(speedArr_Sip6[0]);
    }
    else
    {
        speedArrPtr     = speedArr_Sip_6_10;
        arraySize       = sizeof(speedArr_Sip_6_10) / sizeof(speedArr_Sip_6_10[0]);
    }

    queuePdsProfilesPtr = PRV_NON_SHARED_TXQ_DIR_TXQ_MAIN_SRC_GLOBAL_VAR_GET(queuePdsProfiles);

    if(GT_FALSE==PRV_NON_SHARED_TXQ_DIR_TXQ_MAIN_SRC_GLOBAL_VAR_GET(profilesInitialized))
    {
        rc = prvCpssDxChTxqPdsLongQueueAttributesInit(devNum,queuePdsProfilesPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChTxqPdsLongQueueAttributesInit\n");
        }
        PRV_NON_SHARED_TXQ_DIR_TXQ_MAIN_SRC_GLOBAL_VAR_SET(profilesInitialized,GT_TRUE);
    }



    for (i = 0; i < arraySize; i++)
    {
        if (GT_TRUE == speedToProfile)
        {
            if (speedArrPtr[i]>=*speedinGPtr)
            {
                if(speedArrPtr[i] ==*speedinGPtr||i==0)
                {
                    *profilePtr = queuePdsProfilesPtr+i;
                }
                else
                {
                   *profilePtr = queuePdsProfilesPtr + i-1;
                }
                return GT_OK;
            }
            else if(i==arraySize-1)
            {
                *profilePtr = queuePdsProfilesPtr + i;
                return GT_OK;
            }
        }
        else if (0 == cpssOsMemCmp(*profilePtr, queuePdsProfilesPtr + i,
                       sizeof(PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC)))
        {
            *speedinGPtr    = speedArrPtr[i];
            *profilePtr = queuePdsProfilesPtr + i;
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
}
/**
 * @internal prvCpssDxChTxqInitPdsLongQueueEnableSet function
 * @endinternal
 *
 * @brief  DEBUG FUNCTION : to change use of long queue / 'short queue' and length adjust profiles
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum          - device number
 * @param[in] speed_in_G      - speed in G units : 10/50/25/100/200/400 .
 * @param[in] enableLongQueue - enable/disable long queue
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 *
 * @note   NOTE: CPSS copy all the profiles to all the DP cores in all tiles
 */
GT_STATUS prvCpssDxChTxqInitPdsLongQueueEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 speed_in_G,
    IN GT_BOOL enableLongQueue
)
{
    GT_STATUS                       rc;
    GT_U32                          index;/* index for cpssDxChPortTxQueueProfileSet */
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC        profile;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC     longQueueAttr;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC  lengthAdjustParameters;
    profile.longQueueParametersPtr      = &longQueueAttr;
    profile.lengthAdjustParametersPtr   = &lengthAdjustParameters;
    for (index = 0; index < CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM; index++)
    {
        if (speed_in_G == PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[index].speed_in_G)
        {
            rc = prvCpssDxChTxqFalconPdsProfileGet(devNum, 0, 0, index, GT_FALSE, &profile);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChTxqFalconPdsProfileGet, profileIndex = %d\n", index);
            }
            profile.longQueueParametersPtr->longQueueEnable = enableLongQueue;
            rc                      = prvDxChPortTxQueueProfileHwSet(devNum, index, &profile);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvDxChPortTxQueueProfileHwSet, profileIndex = %d\n", index);
            }
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssDxChTxqPhyPortSdqQueueRangeGet function
 * @endinternal
 *
 * @brief    Get local (SDQ) queue range by physical port number
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum          - device number
 * @param[in] portNum         - physical port number
 * @param[out] mappingTypePtr -(pointer to)mapping type of physical port
 * @param[out] firstQueueIndexPtr -(pointer to)First SDQ queue mapped to physical port
 * @param[out] lastQueueIndexPtr -(pointer to)LastSDQ queue mapped to physical port
 * @param[out] tileNumPtr -(pointer to)Tile physical port mapped to
 * @param[out] tileNumPtr -(pointer to)Data path physical port mapped to
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 *
 * @note   NOTE: CPSS copy all the profiles to all the DP cores in all tiles
 */
static GT_STATUS prvCpssDxChTxqPhyPortSdqQueueRangeGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PORT_MAPPING_TYPE_ENT                        *mappingTypePtr,
    OUT GT_U32                                                 *firstQueueIndexPtr,
    OUT GT_U32                                                 *lastQueueIndexPtr,
    OUT GT_U32                                                 *tileNumPtr,
    OUT GT_U32                                                 *dpNumPtr
)
{
    GT_STATUS               rc;
    GT_BOOL                 isCascade;
    GT_U32                  localdpPortNum;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    anodePtr = NULL;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *    pnodePtr;

    GT_U32                  pNodeNum, aNodeListSize;
    GT_U32                  dummy = 0;
    CPSS_NULL_PTR_CHECK_MAC(firstQueueIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(lastQueueIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(mappingTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(tileNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(dpNumPtr);
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, tileNumPtr, dpNumPtr, &localdpPortNum, mappingTypePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum, &isCascade, NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsIsCascadePort  failed for portNum  %d  ", portNum);
    }
    if (isCascade == GT_TRUE)
    {
        pNodeNum        = (*dpNumPtr) + (MAX_DP_IN_TILE(devNum)) * localdpPortNum;
        rc = prvCpssSip6TxqUtilsPnodeIndexGet(devNum,*dpNumPtr,localdpPortNum,&pNodeNum);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsPnodeIndexGet  failed for local port  %d  ",localdpPortNum);
        }
        PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pnodePtr,devNum, *tileNumPtr,pNodeNum);
        *firstQueueIndexPtr = pnodePtr->aNodelist[0].queuesData.queueBase;
        aNodeListSize       = pnodePtr->aNodeListSize;
        *lastQueueIndexPtr  = (*firstQueueIndexPtr) + (pnodePtr->aNodelist[aNodeListSize - 1].queuesData.pdqQueueLast - pnodePtr->aNodelist[0].queuesData.pdqQueueFirst);
    }
    else
    {
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, portNum, tileNumPtr, &dummy);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet  failed for portNum  %d  ", portNum);
        }
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &anodePtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
        }
        *firstQueueIndexPtr = anodePtr->queuesData.queueBase;
        *lastQueueIndexPtr  = *firstQueueIndexPtr + (anodePtr->queuesData.pdqQueueLast - anodePtr->queuesData.pdqQueueFirst);
    }
    return GT_OK;
}
/**
 * @internal prvCpssDxChTxqBindPortQueuesToPdsProfile function
 * @endinternal
 *
 * @brief Bind port queues to PDS profile  (depend on port speed)
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] speed                      - port speed
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 *
 * @note   NOTE:      Remote ports are not handled.The configuration for remote ports should be done while configuring cascade port.
 */
GT_STATUS prvCpssDxChTxqBindPortQueuesToPdsProfile
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_SPEED_ENT speed,
    IN PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC *adjustAttributesPtr
)
{
    GT_STATUS                       rc;
    GT_U32                          tileNum, dpNum, i, lastQueue, firstQueue;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT             mappingType;
    GT_U32                          speed_in_G = 0, wordIndex, bitIndex, *currentWordPtr;
    GT_U32                          profile = 0;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC  defaultAdjust;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssSystemRecoveryStateGet failed\n ");
    }
    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        /*this will be done during  CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E via
                prvCpssDxChTxqCatchUp */
        return GT_OK;
    }
    /*Threshold set only for local ports ,remote ports thresholds will be set via cascade port*/
    rc = prvCpssDxChTxqPhyPortSdqQueueRangeGet(devNum, portNum, &mappingType, &firstQueue, &lastQueue, &tileNum, &dpNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqPhyPortSdqQueueRangeGet  failed for portNum  %d  ", portNum);
    }
    if (mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        speed_in_G = prvCpssCommonPortSpeedEnumToMbPerSecConvert(speed);
        speed_in_G /=1000;

        wordIndex   = portNum / 32;
        bitIndex    = portNum % 32;
        /*Find old profile*/
        for (i = 0; i < CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM; i++)
        {
            currentWordPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].bindedPortsBmp[wordIndex]);
            if ((*currentWordPtr) & (1 << bitIndex))
            {
                break;
            }
        }
        /* 3 cases :
                     a. There is a speed change for port binded to another profile,so we will use his lenght adjust
                     b. There is a speed change for port not binded to another profile,so we will usedefault  lenght adjust.
                     c.There is a length adjust change ,so we will use it.
         */
        if (adjustAttributesPtr == NULL)
        {
            /*found*/
            if (i < CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM)
            {
                PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].bindedPortsBmp[wordIndex] &= ~((1 << bitIndex));
                PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].numberOfBindedPorts--;
                /*case a*/
                /*get the profile length adjust*/
                rc = prvCpssFalconTxqPdsProfileGet(devNum, speed_in_G,
                                   (PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].lengthAdjustParametersPtr),
                                   &profile);
            }
            else
            {
                /*case b*/
                prvCpssFalconTxqUtilsInitPdsLengthAdjust(&defaultAdjust);
                /*get the profile w/o    specific  lengthAdjustParameters ,lengthAdjustParameters will be
                   created according to the speed*/
                rc = prvCpssFalconTxqPdsProfileGet(devNum, speed_in_G, &defaultAdjust, &profile);
            }
        }
        else
        {
            /*case c*/
            if (i < CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM)
            {
                PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].bindedPortsBmp[wordIndex] &= ~((1 << bitIndex));
                PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].numberOfBindedPorts--;
                speed_in_G = PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].speed_in_G;
            }
            else /*port has no speed configured*/
            {
                /*bind to default speed*/
                speed_in_G = 50;
            }
            rc = prvCpssFalconTxqPdsProfileGet(devNum, speed_in_G, adjustAttributesPtr, &profile);
        }
        if (rc == GT_OK)
        {
            /*Now update new profile shadow*/
            currentWordPtr      = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[profile].bindedPortsBmp[wordIndex]);
            (*currentWordPtr)   |= ((1 << bitIndex));
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[profile].numberOfBindedPorts++;
            if (i < CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM)
            {
                if (0 == PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].numberOfBindedPorts)
                {
                    rc = prvCpssFalconTxqUtilsInitPdsProfile(devNum, &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i]), i,
                                         PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].lengthAdjustParametersPtr == NULL ? GT_FALSE : GT_TRUE);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
            /*Bind HW*/
            for (i = firstQueue; i <= lastQueue; i++)
            {
                rc = prvCpssDxChTxqFalconPdsQueueProfileMapSet(devNum, tileNum, dpNum, i, profile);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqQueueAttributesSet  failed for q  %d  ", i);
                }
            }
        }
    }
    return rc;
}
/**
 * @internal prvCpssDxChTxqBindPortQueuesToLengthAdjustProfile function
 * @endinternal
 *
 * @brief Bind port queues to length adjust profile
 *
 * @note   APPLICABLE DEVICES:       AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] adjustAttributesPtr                      -(pointer to)length adjust attributes
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 *
 * @note   NOTE:      Remote ports are not handled.The configuration for remote ports should be done while configuring cascade port.
 */
GT_STATUS prvCpssDxChTxqBindPortQueuesToLengthAdjustProfile
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC *adjustAttributesPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      tileNum;
    GT_U32                      i, j, lastQueue, firstQueue;
    GT_U32                      dpNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT         mappingType;
    GT_U32                      wordIndex, bitIndex, *currentWordPtr;
    GT_U32                      profile = CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM, oldProfile = CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC *  profilesArrayPtr;
    rc = prvCpssDxChTxqPhyPortSdqQueueRangeGet(devNum, portNum, &mappingType, &firstQueue, &lastQueue, &tileNum, &dpNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqPhyPortSdqQueueRangeGet  failed for portNum  %d  ", portNum);
    }
    profilesArrayPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles;
    /*Threshold set only for local ports ,remote ports thresholds will be set via cascade port*/
    if (mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        /*Check if we already have this profilein HW*/
        wordIndex   = portNum / 32;
        bitIndex    = portNum % 32;
        /*Find old profile*/
        for (i = 0; i < CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM; i++)
        {
            currentWordPtr = &(profilesArrayPtr[i].bindedPortsBmp[wordIndex]);
            if ((*currentWordPtr) & (1 << bitIndex))
            {
                oldProfile = i;
            }
            if (cpssOsMemCmp(adjustAttributesPtr,
                     profilesArrayPtr[i].lengthAdjustParametersPtr,
                     sizeof(PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC)) == 0)
            {
                if (profilesArrayPtr[i].numberOfBindedPorts > 0)
                {
                    profile = i;
                }
            }
        }
        /*found*/
        if (oldProfile < CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM)
        {
            if (oldProfile == profile)
            {
                return GT_OK;
            }
            /*delete from old*/
            profilesArrayPtr[oldProfile].bindedPortsBmp[wordIndex] &= ~((1 << bitIndex));
            profilesArrayPtr[oldProfile].numberOfBindedPorts--;
        }
        if (profile == CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM)
        {
            /*Create new profile*/
            for (j = 0; j < CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM; j++)
            {
                if (profilesArrayPtr[j].numberOfBindedPorts == 0)
                {
                    profile = j;
                    break;
                }
            }
            if (profile == CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, " No more profiles availeble");
            }
            *(profilesArrayPtr[profile].lengthAdjustParametersPtr) =
                *adjustAttributesPtr;
            /*Update HW*/
            rc = prvDxChPortTxQueueProfileHwSet(devNum, profile, &profilesArrayPtr[profile]);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvDxChPortTxQueueProfileHwSet, profileIndex = %d\n", profile);
            }
        }
        /*Now update new profile shadow*/
        currentWordPtr      = &(profilesArrayPtr[profile].bindedPortsBmp[wordIndex]);
        (*currentWordPtr)   |= ((1 << bitIndex));
        profilesArrayPtr[profile].numberOfBindedPorts++;
        /*Bind HW*/
        for (i = firstQueue; i <= lastQueue; i++)
        {
            rc = prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapSet(devNum, tileNum, dpNum, i, profile);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapSet  failed for q  %d  ", i);
            }
        }
    }
    return rc;
}

/**
 * @internal prvCpssFalconTxqPfccCalendarDefaultEntriesSet function
 * @endinternal
 *
 * @brief Set default entries at PFCC calendar. All global TCs are set in order to use tail drop by pool.
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                  - device number
 * @param[in] tileNum                    -tile number
 * @param[in] qfcNum                   - qfc number
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 *
 */
static GT_STATUS prvCpssFalconTxqPfccCalendarDefaultEntriesSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
)
{
    GT_U32              globalTcIndex, tc;
    PRV_CPSS_PFCC_CFG_ENTRY_STC entry;
    GT_STATUS           rc;
    entry.entryType = PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC;
    globalTcIndex   = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp) * CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum);
    for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
    {
        entry.globalTc  = tc;
        rc      = prvCpssFalconTxqPfccCfgTableEntrySet(devNum, tileNum, globalTcIndex + tc, &entry);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccCfgTableEntrySet failed for tc %d", tc);
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssDxChTxqPfccTableInit function
 * @endinternal
 *
 * @brief The function initialize PFCC CFG table and Global_pfcc_CFG register. Only master tile is configured
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                  - device number
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 *
 */
GT_STATUS prvCpssDxChTxqPfccTableInit
(
    IN GT_U8 devNum
)
{
    GT_U32              tableSize, i;
    PRV_CPSS_PFCC_TILE_INIT_STC initDb;
    GT_STATUS           rc;
    PRV_CPSS_PFCC_CFG_ENTRY_STC bubble      = { 0, 0, 0, 0, GT_FALSE, 0 };
    GT_U32              masterTile  = PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC;
    GT_U32              dataPathNum,maxLocalPortNum;
    /*First decide on table size*/
    /*currently waisting (maxDP-2) entries because only 2 CPU NW ports are availeble.Since we have 32 spare entries it is ok*/
    dataPathNum         = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp);
    maxLocalPortNum     = CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum);
    tableSize           = dataPathNum *maxLocalPortNum;
    tableSize           += 8;   /*Global TC - always last 16 till 8*/
    tableSize           += 8;   /*Global TC HR - always last  8*/
    initDb.pfccLastEntryIndex   = tableSize - 1;
    initDb.isMaster         = GT_TRUE;
    initDb.pfccEnable       = GT_FALSE;/*Do not enable chain yet ,this may cause interrupts since availeble buffers is not configured yet*/
    rc              = prvCpssFalconTxqPfccUnitCfgSet(devNum, masterTile, &initDb);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccUnitCfgSet  failed for tile  %d  ", masterTile);
    }


    bubble.entryType    = PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE;
    bubble.numberOfBubbles  = 1;
    for (i = 0; i < tableSize; i++)
    {
        /*Now set all entries to bubbles of size 1*/
        rc = prvCpssFalconTxqPfccCfgTableEntrySet(devNum, masterTile, i, &bubble);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccCfgTableEntrySet  failed for tile  %d  ", masterTile);
        }
    }
    /*Set global TC entries,required for pool tail drop engine*/
    rc = prvCpssFalconTxqPfccCalendarDefaultEntriesSet(devNum, masterTile);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccCalendarDefaultEntriesSet  failed for tile  %d  ", masterTile);
    }
    return GT_OK;
}
/**
 * @internal prvCpssDxChTxqQfcUnitsInit function
 * @endinternal
 *
 * @brief Enable local PFC generation at all QFCs.PFC generation will be controlled at PFCC
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                  - device number
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 *
 */
GT_STATUS prvCpssDxChTxqQfcUnitsInit
(
    IN GT_U8 devNum
)
{
    GT_U32      numberOfTiles, i, j;
    GT_STATUS   rc;
    switch (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
    case 0:
    case 1:
        numberOfTiles = 1;
        break;
    case 2:
    case 4:
        numberOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ", PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
        break;
    }
    for (i = 0; i < numberOfTiles; i++)
    {
        for (j = 0; j < MAX_DP_IN_TILE(devNum); j++)
        {
            rc = prvCpssFalconTxqQfcGlobalPfcCfgGlobalEnableSet(devNum, i, j, GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcGlobalPbLimitSet fail");
            }
            /*HR counting is enabled by default in HW.So no need to enable in initialization*/
        }
    }
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.lastUburstDpChecked = (numberOfTiles * (MAX_DP_IN_TILE(devNum))) - 1;
    return GT_OK;
}

GT_STATUS prvCpssFalconTxqUtilsNumberOfConsumedLanesGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 dpNum,
    IN GT_U32 localPort,
    IN GT_U32                  *numberOfConsumedLanesPtr
)
{
    GT_STATUS           rc;
    GT_U32              i;
    GT_U32              activeSliceNum;
    GT_U32              activeSliceMap[TXQ_PDX_MAX_SLICE_NUMBER_MAC];
    GT_BOOL             sliceValid[TXQ_PDX_MAX_SLICE_NUMBER_MAC];
    GT_PHYSICAL_PORT_NUM        portNum;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT     speed;
    *numberOfConsumedLanesPtr = 0;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
    {
        rc = prvCpssDxChTxqFalconPdxBurstFifoPdsArbitrationMapGet(devNum, tileNum, dpNum, &activeSliceNum, activeSliceMap, sliceValid);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdxBurstFifoPdsArbitrationMapGet  failed for dpNum  %d  ", dpNum);
        }
        /*CPU port*/
        if (localPort == CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum) - 1)
        {
            if (activeSliceNum != TXQ_PDX_MAX_SLICE_NUMBER_MAC)
            {
                /*no CPU port mapped on this DP*/
                return GT_OK;
            }
            if (sliceValid[TXQ_PDX_MAX_SLICE_NUMBER_MAC - 1] == GT_TRUE)
            {
                *numberOfConsumedLanesPtr = 1;
                return GT_OK;
            }
        }
        /*Read only first 8 entries due to duplication of pizza*/
        for (i = 0; i < 8; i++)
        {
            if ((sliceValid[i] == GT_TRUE) && (activeSliceMap[i] == localPort))
            {
                (*numberOfConsumedLanesPtr)++;
            }
        }
    }
    else
    {
        /*Find first mapped port*/
        rc = prvCpssFalconTxqUtilsPhysicalPortNumberGet(devNum, tileNum, dpNum, localPort, &portNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsPhysicalPortNumberGet failed\n");
        }
        /*querry regarding speed and mode*/
        rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChPortInterfaceModeGet failed\n");
        }
        rc = cpssDxChPortSpeedGet(devNum, portNum, &speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortSpeedGet command failed in port %d\n", portNum);
        }
        rc = prvCpssDxChPortFalconSlicesFromPortSpeedGet(devNum, portNum, ifMode, speed, numberOfConsumedLanesPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortFalconSlicesFromPortSpeedGet command failed in port %d\n", portNum);
        }
    }
    return GT_OK;
}
static GT_STATUS prvCpssFalconTxqPffcTableSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 dpNum,
    IN GT_U32 localPort,
    IN GT_U32 tcBmp,
    IN GT_U32 numberOfConsumedLanes
)
{
    GT_STATUS           rc;
    GT_U32              i;
    GT_U32              startIndex;
    PRV_CPSS_PFCC_CFG_ENTRY_STC entry;
    startIndex = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp) * localPort + dpNum + tileNum * (MAX_DP_IN_TILE(devNum));
    if (tcBmp!=0)
    {
        /*Get source port*/
        rc = prvCpssFalconTxqQfcLocalPortToSourcePortGet(devNum, tileNum, dpNum, localPort, &entry.sourcePort, NULL);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcLocalPortToSourcePortGet  failed for localPort  %d  ", localPort);
        }
        entry.entryType     = PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT;
        entry.pfcMessageTrigger = GT_TRUE;
        entry.tcBitVecEn    = tcBmp;
    }
    else
    {
        entry.entryType     = PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE;
        entry.numberOfBubbles   = 1;
    }
    for (i = 0; i < numberOfConsumedLanes; i++)
    {
        rc = prvCpssFalconTxqPfccCfgTableEntrySet(devNum, PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC, startIndex, &entry);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for localPort  %d  ", localPort);
        }
        /*Prepare for next iteration*/
        localPort++;
        startIndex = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp) * (localPort) + dpNum + tileNum * (MAX_DP_IN_TILE(devNum));
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqPffcTableSyncSet function
 * @endinternal
 *
 * @brief   This function read from PDX pizza and configure PFCC table according to number of lanes consumed by port
 *
 * @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                          PP's device number.
 * @param[in] tileNum                           tile number
 * @param[in] dpNum         -                 data path[0..7]
 * @param[in] localPort                         local port number[0..8]
 * @param[in] tcBmp                           traffic class that is set for flow control,0xFF mean flow control on port
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqPffcTableSyncSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 dpNum,
    IN GT_U32 localPort,
    IN GT_U32 tcBmp
)
{
    GT_STATUS   rc;
    GT_U32      numberOfConsumedLanes;
    rc = prvCpssFalconTxqUtilsNumberOfConsumedLanesGet(devNum, tileNum, dpNum, localPort, &numberOfConsumedLanes);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for localPort  %d  ", localPort);
    }
    rc = prvCpssFalconTxqPffcTableSet(devNum, tileNum, dpNum, localPort, tcBmp, numberOfConsumedLanes);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPffcTableSet  failed for localPort  %d  ", localPort);
    }
    return GT_OK;
}
/**
* @internal prvCpssFalconTxqPffcTableSyncSet function
* @endinternal
*
* @brief   This function  PFCC table according to given configuration of consumed lanes
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                         PP's device number.
* @param[in] tileNum                           tile number
* @param[in] dpNum         -                data path[0..7]
* @param[in] localPort                        local port number[0..8]
* @param[in] portPizzaSlices-           desired PDX pizza configuration
* @param[in] tcBmp                            traffic class that is set for flow control,0xFF mean flow control on port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPffcTableExplicitSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  dpNum,
    IN  GT_U32                  localPort,
    IN  GT_U32                  portPizzaSlices,
    IN  GT_U32                  tcBmp
)
{
    GT_STATUS   rc;
    GT_U32      i;
    GT_U32      numberOfConsumedLanes;
    numberOfConsumedLanes = 0;
    /*CPU port*/
    if (localPort == CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum) - 1)
    {
        numberOfConsumedLanes = 1;
    }
    else
    {
        /*Read only first 8 entries due to duplication of pizza*/
        for (i = 0; i < 8; i++)
        {
            if ((portPizzaSlices & 1 << i))
            {
                numberOfConsumedLanes++;
            }
        }
    }
    rc = prvCpssFalconTxqPffcTableSet(devNum, tileNum, dpNum, localPort, tcBmp, numberOfConsumedLanes);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPffcTableSet  failed for localPort  %d  ", localPort);
    }
    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqUtilsQueueTcSet function
 * @endinternal
 *
 * @brief   Sets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
 *               perticular TC
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -physical device number
 * @param[in] portNum                    physical port number
 * @param[in] queueNumber                                              local queue offset [0..15]
 * @param[in] tcForPfcResponse                       -      Traffic class[0..15]
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsQueueTcSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 queueNumber,
    IN GT_U32 tc
)
{
    GT_STATUS           rc;
    GT_U32              tileNum, dpNum, localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *anodePtr;
    GT_U32              lastValidQueue;
    /*Find  local SDQ queue number*/
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    if (mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "PFC responce configured not on local port");
    }
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &anodePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    /*Check range*/
    lastValidQueue = anodePtr->queuesData.pdqQueueLast - anodePtr->queuesData.pdqQueueFirst;
    if (queueNumber > lastValidQueue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Wrong queue number %d", queueNumber);
    }
    /*TC validity is checked inside prvCpssFalconTxqSdqQueueTcSet*/
    rc = prvCpssFalconTxqSdqQueueTcSet(devNum, tileNum, dpNum, anodePtr->queuesData.queueBase + queueNumber, tc);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqQueueTcSet  failed for portNum  %d  ", portNum);
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqUtilsQueueTcGet function
 * @endinternal
 *
 * @brief   Gets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
 *               perticular TC
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -physical device number
 * @param[in] portNum                    physical port number
 * @param[in] queueNumber           local queue offset [0..15]
 * @param[in] tcPtr                      -        (pointer to)Traffic class[0..15]
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsQueueTcGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 queueNumber,
    IN GT_U32                  *tcPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      tileNum, dpNum, localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT         mappingType;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *        anodePtr;
    GT_U32                      lastValidQueue;
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES queueAttributes;
    /*Find  local SDQ queue number*/
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    if (mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "PFC responce configured not on local port");
    }
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &anodePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    /*Check range*/
    lastValidQueue = anodePtr->queuesData.pdqQueueLast - anodePtr->queuesData.pdqQueueFirst;
    if (queueNumber > lastValidQueue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Wrong queue number %d", queueNumber);
    }
    rc = prvCpssFalconTxqSdqQueueAttributesGet(devNum, tileNum, dpNum, anodePtr->queuesData.queueBase + queueNumber,
                           &queueAttributes);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqQueueAttributesGet  failed for portNum  %d  ", portNum);
    }
    *tcPtr = queueAttributes.tc;
    return rc;
}


/**
 * @internal prvCpssFalconTxqUtilsGetCascadePort function
 * @endinternal
 *
 * @brief  Get Cascade port related to remote port.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -         `                          physical device number
 * @param[in] remotePortNum                                             physical remote port number
 * @param[out] casCadePortNumPtr                                  (pointer to) cascade port
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsGetCascadePort
(
    IN GT_U8 devNum,
    IN GT_U32 remotePortNum,
    IN GT_BOOL remote,
    OUT GT_U32 *casCadePortNumPtr,
    OUT GT_U32 * pNodeIndPtr
)
{
    GT_STATUS               rc;
    /*In order to avoid compiler warning initialize pNode*/
    GT_U32                  pNode   = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    GT_U32                  tileNum = 0;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *   portMapShadowPtr;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pNodePtr = NULL;
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, remotePortNum, /*OUT*/ &portMapShadowPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if (portMapShadowPtr->valid)
    {
        if (remote && portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Port %d is not remote", remotePortNum);
        }
        rc = prvCpssFalconTxqUtilsGetDmaToPnodeMapping(devNum, portMapShadowPtr->portMap.txDmaNum, &tileNum, &pNode, GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pNodePtr,devNum,tileNum,pNode);

        if (pNodePtr->isCascade)
        {
            *casCadePortNumPtr = pNodePtr->cascadePhysicalPort;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "The remote port %d has no cascade port", remotePortNum);
        }
        if (pNodeIndPtr)
        {
            *pNodeIndPtr = pNode;
        }
        return GT_OK;
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}
/**
 * @internal prvCpssFalconTxqUtilsPortEnableSet function
 * @endinternal
 *
 * @brief  Set port to enable in SDQ.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -         `                          physical device number
 * @param[in] portNum                                                         physical port number
 * @param[in] enable                                                             Enable/disable "credit ignore" mode
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsPortEnableSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL enable
)
{
    GT_STATUS           rc;
    GT_U32              tileNum, dpNum, localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    CPSS_PORT_INTERFACE_MODE_ENT *portIfModePtr;/* interface configured on port now */
    CPSS_PORT_SPEED_ENT          *portSpeedPtr; /* speed configured on port now */
    GT_U32                        currentBufferNumber;
    GT_BOOL                       preemptionEnabled = GT_FALSE;
    GT_BOOL                       actAsPreemptiveChannel;
    GT_PHYSICAL_PORT_NUM          preemptivePhysicalPortNum;
    GT_U32                        preemptiveLocalDpPortNum;
    GT_BOOL                       currentState =  GT_FALSE;
    CPSS_PORT_SPEED_ENT           noSpeedDummy = CPSS_PORT_SPEED_NA_E;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum, portMacMap);
    portIfModePtr = &(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap));
    portSpeedPtr = &(PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap));

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    if (mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        return GT_OK;
    }

    rc = prvCpssFalconTxqUtilsPortEnableGet(devNum, portNum, &currentState);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc,"error in prvCpssFalconTxqUtilsPortEnableGet, portNum = %d \n",
            portNum);
    }

    if ( (currentState == GT_TRUE) && (enable == GT_TRUE) )
    {
        /* If port TXQ is already enabled and enabled is asked again - no need to do anything */
        return GT_OK;
    }
    /* check that the port is empty,if not return error */
    rc = prvCpssFalconTxqUtilsPortTxBufNumberGet(devNum, portNum,&currentBufferNumber);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc,"error in prvCpssFalconTxqUtilsPortTxBufNumberGet, portNum = %d \n",
            portNum);
    }

    if (currentBufferNumber>0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_BAD_STATE," portNum = %d contain %d buffers \n",
            portNum,currentBufferNumber);
    }

    if (TXQ_IS_PREEMPTIVE_DEVICE(devNum))
    {

        /*check preemption*/

        rc = prvCpssSip6TxqUtilsPreemptionStatusGet(devNum, portNum,&preemptionEnabled,&actAsPreemptiveChannel,
                         &preemptivePhysicalPortNum,&preemptiveLocalDpPortNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsPreemptionStatusGet  failed for portNum  %d  ", portNum);
        }

        if(GT_TRUE==actAsPreemptiveChannel)
        {
            /*no need to handle pizza ,will be done inside pizza arbiter code*/
            portSpeedPtr = &noSpeedDummy;
        }
     }

    /*If there is speed set then first disable pizza*/
    if(*portSpeedPtr!=CPSS_PORT_SPEED_NA_E &&*portIfModePtr!=CPSS_PORT_INTERFACE_MODE_NA_E )
    {
        /*In order to avoid UnMap Port Sel interrupt need to disable pizza at TXDMA/TXFIFO first.
          The problem is at the "no speed" (or link down event)
          Since first SDQ is disabled then pizza is deleted.TxQ raise interrupt since pizza should not be configured for disabled port.
        */
         if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
         {
             CPSS_TBD_BOOKMARK_AC5P
         }
         else
         {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesSet(devNum, portNum, *portIfModePtr, *portSpeedPtr, GT_FALSE);
            if (rc != GT_OK)
            {
               CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prevCpssDxChTxPortSpeedResourcesSet, portNum = %d\n", portNum);
            }
         }
    }
    rc = prvCpssFalconTxqSdqLocalPortEnableSet(devNum, tileNum, dpNum, localdpPortNum, enable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqLocalPortEnableSet  failed for localdpPortNum  %d  ", localdpPortNum);
    }

    rc = prvCpssSip6TxqUtilsDataPathEvent(devNum, portNum,GT_TRUE== enable?PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXQ_PORT_ENABLED:
        PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXQ_PORT_DISABLED,GT_FALSE);

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsDataPathEvent  failed for port  %d  ", portNum);
    }
    /*when enable physical  port , prinmary channel is always expess*/
    rc = prvCpssSip6_30TxqSdqLocalPortPreemptionTypeSet(devNum, tileNum, dpNum, localdpPortNum, GT_FALSE);
    if (rc != GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqLocalPortPreemptionTypeSet  failed for port  %d  ", portNum);
    }


    if(GT_TRUE == preemptionEnabled)
    {
      rc = prvCpssFalconTxqSdqLocalPortEnableSet(devNum, tileNum, dpNum, preemptiveLocalDpPortNum, enable);
      if (rc != GT_OK)
      {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqLocalPortEnableSet  failed for localdpPortNum  %d  ", preemptiveLocalDpPortNum);
      }

       rc = prvCpssSip6TxqUtilsDataPathEvent(devNum, portNum,GT_TRUE== enable?PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXQ_PORT_ENABLED:
        PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXQ_PORT_DISABLED,GT_TRUE);

       if (rc != GT_OK)
       {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsDataPathEvent  failed for port  %d  ", portNum);
       }

       /*secondary  channel is always preemptive if preemption is configured*/
       rc =prvCpssSip6_30TxqSdqLocalPortPreemptionTypeSet(devNum, tileNum, dpNum, preemptiveLocalDpPortNum, GT_TRUE);
       if (rc != GT_OK)
       {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqLocalPortPreemptionTypeSet  failed for port  %d  ", portNum);
       }
    }

    if(GT_TRUE == enable)
    {
        /*part of enable port sequence w/o speed configuration*/
        if(*portSpeedPtr!=CPSS_PORT_SPEED_NA_E &&*portIfModePtr!=CPSS_PORT_INTERFACE_MODE_NA_E )
        {
             if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
             {
                 CPSS_TBD_BOOKMARK_AC5P
             }
             else
             {
              rc = prvCpssDxChTxPortSpeedPizzaResourcesSet(devNum, portNum, *portIfModePtr, *portSpeedPtr, GT_TRUE);
              if (rc != GT_OK)
              {
                  CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prevCpssDxChTxPortSpeedResourcesSet, portNum = %d\n", portNum);
              }
             }
        }
        /*else -assume to  be a part of speed configuration,txPizza configuration will follow later*/
    }

    return GT_OK;
}
GT_STATUS prvCpssFalconTxqUtilsPortEnableGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL * enablePtr
)
{
    GT_STATUS           rc;
    GT_U32              tileNum, dpNum, localdpPortNum;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);


    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum,NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }

    rc = prvCpssFalconTxqSdqLocalPortEnableGet(devNum, tileNum, dpNum, localdpPortNum, enablePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqLocalPortEnableSet  failed for localdpPortNum  %d  ", localdpPortNum);
    }

    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqUtilsPortEnableSet function
 * @endinternal
 *
 * @brief  Flush queues of remote ports.Wait until the queues are empty.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -         `                          physical device number
 * @param[in] portNum                                                         physical port number
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsRemotePortFlush
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS                       rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *            aNodePtr;
    GT_U32                          numberOfMappedQueues, i, queueIterator;
    GT_U32                          prevDescNum, tileNum, dpNum, localPortNum;
    GT_U32                          queueDescNum = (GT_U32)-1; /* number of not treated TXQ descriptors */
    PRV_CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTER_STC    perQueueDescCount;
    GT_U32                          pfcRespBitMap;
    CPSS_DXCH_PORT_FC_MODE_ENT              pfcGenerationMode;
    GT_U32                          cascadePort = 0 /*avoid warning*/;
    /*Find the pointer to A node*/
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &aNodePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " A node is not found for  physical port %d  ", portNum);
    }
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localPortNum, NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssFalconTxqUtilsGetCascadePort(devNum, portNum, GT_TRUE, &cascadePort, NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsGetCascadePort fail");
    }
    /*Disable PFC on cascade*/
    rc = prvCpssFalconTxqUtilsPfcDisable(devNum, cascadePort, &pfcRespBitMap, &pfcGenerationMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsPfcDisable fail");
    }
    numberOfMappedQueues = aNodePtr->queuesData.pdqQueueLast - aNodePtr->queuesData.pdqQueueFirst + 1;
    for (i = 0; i < numberOfMappedQueues; i++)
    {
        queueIterator = aNodePtr->queuesData.queueBase + i;
        do
        {
            prevDescNum = queueDescNum;
            rc      = prvCpssDxChTxqFalconPdsPerQueueCountersGet(devNum, tileNum, dpNum, queueIterator, &perQueueDescCount);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet  failed for tile  %d pds %d ", tileNum, i);
            }
            queueDescNum = perQueueDescCount.headCounter;
            if (GT_TRUE == perQueueDescCount.longQueue)
            {
                queueDescNum += perQueueDescCount.tailCounter + (16 * perQueueDescCount.fragCounter);
            }
            if (0 == queueDescNum)
            {
                break;
            }
            else
            {
                cpssOsTimerWkAfter(10);
            }
        } while (queueDescNum < prevDescNum);
        if (0 == queueDescNum)
        {
            /*move to next Q*/
            i++;
        }
        else
        {
            /*Failure*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Can not flush queue %d port %d ", i, portNum);
        }
    }
    /*Restore  PFC state*/
    rc = prvCpssFalconTxqUtilsPfcEnable(devNum, cascadePort, &pfcRespBitMap, &pfcGenerationMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsPfcEnable fail");
    }
    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqUtilsUburstEventInfoGet function
 * @endinternal
 *
 * @brief  Scan  all available DP for micro burst event
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -         `                          physical device number
 * @param[out] uBurstDataPtr                   -         `               (pointer to)micro  burst event
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_NO_MORE - no more new events found
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsUburstEventInfoGet
(
    IN GT_U8 devNum,
    INOUT GT_U32                               *uBurstEventsSizePtr,
    OUT CPSS_DXCH_UBURST_INFO_STC            *uBurstDataPtr
)
{
    GT_STATUS               rc, rc1;
    GT_U32                  numberOfTiles;
    GT_U32                  i, k;
    GT_U32                  dpQueueNum;
    GT_U32                  tileQueueNum;
    GT_U32                  thresholdId;
    GT_U32                  startQueueIndexForDp;
    CPSS_DXCH_UBURST_TIME_STAMP_STC     timestamp;
    CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT  trigger;
    GT_U32                  aNodeIndex;
    GT_U32                  pNodeIndex;
    GT_U32                  portQueueBase;
    GT_U32                  numberOfDps;
    GT_U32                  tileNum;
    GT_U32                  dpNum, localDpNum;
    GT_U32                  dpEventsPending[MAX_DP_CNS];
    GT_U32                  totalEvents = 0;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pNodePtr = NULL;

    PRV_CPSS_NUM_OF_TILE_GET_MAC(devNum, numberOfTiles);
    numberOfDps = (MAX_DP_IN_TILE(devNum)) * numberOfTiles;
    /*Check which DPs has events*/
    for (i = 0; i < numberOfDps; i++)
    {
        tileNum     = i / (MAX_DP_IN_TILE(devNum));
        localDpNum  = i % (MAX_DP_IN_TILE(devNum));
        rc      = prvCpssFalconTxqQfcUburstEventPendingGet(devNum, tileNum, localDpNum, GT_FALSE, dpEventsPending + i);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcUburstEventPendingGet failed for tile %d dp %d\n", tileNum, localDpNum);
        }
        totalEvents += dpEventsPending[i];
    }
    /*Clip if required*/
    if (totalEvents > *uBurstEventsSizePtr)
    {
        totalEvents = *uBurstEventsSizePtr;
        rc      = GT_OK;
    }
    else
    {
        *uBurstEventsSizePtr    = totalEvents;
        rc          = GT_NO_MORE;
    }
    /*Get the events*/
    while (totalEvents)
    {
        dpNum                           = (PRV_CPSS_DXCH_PP_MAC(devNum)->port.lastUburstDpChecked) + 1;
        dpNum                           %= numberOfDps;
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.lastUburstDpChecked  = dpNum;
        if (dpEventsPending[dpNum])
        {
            totalEvents--;
            dpEventsPending[dpNum]--;
            tileNum     = dpNum / (MAX_DP_IN_TILE(devNum));
            localDpNum  = dpNum % (MAX_DP_IN_TILE(devNum));
            rc1     = prvCpssFalconTxqQfcUburstEventInfoGet(devNum, tileNum, localDpNum, &dpQueueNum, &thresholdId, &timestamp, &trigger);
            if (rc1 != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc1, "prvCpssFalconTxqQfcUburstEventInfoGet failed for tile %d dp %d\n", tileNum, dpNum);
            }
            if (trigger != CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_NOT_VALID_E)
            {
                /*convert DP queue number to  tile  queue number*/
                rc1 = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum, tileNum, localDpNum, &startQueueIndexForDp);
                if (rc1 != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc1, "prvCpssFalconTxqUtilsStartQueueIndexForDpGet failed for tile %d dp %d\n", tileNum, dpNum);
                }
                tileQueueNum = dpQueueNum + startQueueIndexForDp;
                /*Now get the physical port num*/

                rc = prvCpssSip6TxqQnodeToAnodeIndexGet(devNum,tileNum,tileQueueNum,&aNodeIndex);
                /*We got event for queue that is not mapped.Log and continue*/
                if (rc!=GT_OK||aNodeIndex == CPSS_SIP6_TXQ_INVAL_DATA_CNS)
                {
                    CPSS_LOG_INFORMATION_MAC( "Uburst event for unmapped Q node. A node not found for queue  %d\n", tileQueueNum);
                    continue;
                }
                rc  = prvCpssSip6TxqAnodeToPnodeIndexGet(devNum,tileNum,aNodeIndex,&pNodeIndex);
                /*We got event for Anode that is not mapped.Log and continue*/
                if (rc!=GT_OK||pNodeIndex == CPSS_SIP6_TXQ_INVAL_DATA_CNS)
                {
                    CPSS_LOG_INFORMATION_MAC("Uburst event for unmapped A node. P node not found for A node  %d\n", aNodeIndex);
                    continue;
                }

                PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pNodePtr,devNum,tileNum,pNodeIndex);

                for (k = 0; k < pNodePtr->aNodeListSize; k++)
                {
                    if (pNodePtr->aNodelist[k].aNodeIndex == aNodeIndex)
                    {
                        break;
                    }
                }
                if (k == pNodePtr->aNodeListSize)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "A node not found at Pnode  %d\n", pNodeIndex);
                }
                uBurstDataPtr->portNum      = pNodePtr->aNodelist[k].physicalPort;
                portQueueBase           = pNodePtr->aNodelist[k].queuesData.queueBase;
                uBurstDataPtr->queueOffset  = dpQueueNum - portQueueBase;
                uBurstDataPtr->thresholdId  = thresholdId;
                uBurstDataPtr->timestamp    = timestamp;
                uBurstDataPtr->trigger      = trigger;
                uBurstDataPtr++;
            }
            else
            {
                (*uBurstEventsSizePtr)--;/*False alarm was raised*/
            }
        }
    }
    /*
            in the end of events read, another read should be done from registers to clear fill level and prepare
            value in register for next read.
     */
    for (i = 0; i < numberOfDps; i++)
    {
        tileNum     = i / (MAX_DP_IN_TILE(devNum));
        localDpNum  = i % (MAX_DP_IN_TILE(devNum));
        rc1     = prvCpssFalconTxqQfcUburstEventPendingGet(devNum, tileNum, localDpNum, GT_TRUE, dpEventsPending + i);
        if (rc1 != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc1, "prvCpssFalconTxqQfcUburstEventPendingGet failed for tile %d dp %d\n", tileNum, localDpNum);
        }
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqUtilsUburstEnableSet
 * @endinternal
 *
 * @brief   Enable/disable micro burst event generation per port/queue
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -                     physical device number
 * @param[in] physicalPortNumber                  - physical port number
 * @param[in] queueOffset                                   local  queue offset (APPLICABLE RANGES:0..15).
 * @param[in] enable                                             Enable/disable micro burst feature
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsUburstEnableSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNumber,
    IN GT_U32 queueOffset,
    IN GT_BOOL enable
)
{
    GT_STATUS           rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *aNodePtr;
    GT_U32              localQueueNumber;
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNumber, &aNodePtr);
    PRV_CPSS_TXQ_UTILS_CHECK_RESULT_MAC(rc, physicalPortNumber);
    localQueueNumber    = queueOffset + aNodePtr->queuesData.queueBase;
    rc          = prvCpssFalconTxqQfcUburstEnableSet(devNum, aNodePtr->queuesData.tileNum,
                                     aNodePtr->queuesData.dp, localQueueNumber, enable);
    PRV_CPSS_TXQ_UTILS_CHECK_RESULT_MAC(rc, localQueueNumber);
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsUburstEnableGet
 * @endinternal
 *
 * @brief   Get enable/disable configuration micro burst event generation per port/queue
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -                     physical device number
 * @param[in] physicalPortNumber                  - physical port number
 * @param[in] queueOffset                                   local  queue offset (APPLICABLE RANGES:0..15).
 * @param[out] enable                                             Enable/disable micro burst feature
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsUburstEnableGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNumber,
    IN GT_U32 queueOffset,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS           rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *aNodePtr;
    GT_U32              localQueueNumber;
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNumber, &aNodePtr);
    PRV_CPSS_TXQ_UTILS_CHECK_RESULT_MAC(rc, physicalPortNumber);
    localQueueNumber    = queueOffset + aNodePtr->queuesData.queueBase;
    rc          = prvCpssFalconTxqQfcUburstEnableGet(devNum, aNodePtr->queuesData.tileNum,
                                     aNodePtr->queuesData.dp, localQueueNumber, enablePtr);
    PRV_CPSS_TXQ_UTILS_CHECK_RESULT_MAC(rc, localQueueNumber);
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsUburstProfileBindSet
 * @endinternal
 *
 * @brief  Set queue micro burst profile binding
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                       - physical device number
 * @param[in] physicalPortNumber                  - physical port number
 * @param[in] queueOffset                                   port queue offset (APPLICABLE RANGES:0..15).
 * @param[in] profileNum                                      profile number[0..31]
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsUburstProfileBindSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNumber,
    IN GT_U32 queueOffset,
    IN GT_U32 profileNum
)
{
    GT_STATUS           rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *aNodePtr;
    GT_U32              localQueueNumber;
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNumber, &aNodePtr);
    PRV_CPSS_TXQ_UTILS_CHECK_RESULT_MAC(rc, physicalPortNumber);
    localQueueNumber    = queueOffset + aNodePtr->queuesData.queueBase;
    rc          = prvCpssFalconTxqQfcUburstProfileBindSet(devNum, aNodePtr->queuesData.tileNum,
                                      aNodePtr->queuesData.dp, localQueueNumber, profileNum);
    PRV_CPSS_TXQ_UTILS_CHECK_RESULT_MAC(rc, localQueueNumber);
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsUburstProfileBindGet
 * @endinternal
 *
 * @brief  Set queue micro burst profile binding
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                       - physical device number
 * @param[in] physicalPortNumber                  - physical port number
 * @param[in] queueOffset                                   port queue offset (APPLICABLE RANGES:0..15).
 * @param[out] profileNum                                   (pointer to)profile number[0..31]
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsUburstProfileBindGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNumber,
    IN GT_U32 queueOffset,
    OUT GT_U32                             *profileNumPtr
)
{
    GT_STATUS           rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *aNodePtr;
    GT_U32              localQueueNumber;
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNumber, &aNodePtr);
    PRV_CPSS_TXQ_UTILS_CHECK_RESULT_MAC(rc, physicalPortNumber);
    localQueueNumber    = queueOffset + aNodePtr->queuesData.queueBase;
    rc          = prvCpssFalconTxqQfcUburstProfileBindGet(devNum, aNodePtr->queuesData.tileNum,
                                      aNodePtr->queuesData.dp, localQueueNumber, profileNumPtr);
    PRV_CPSS_TXQ_UTILS_CHECK_RESULT_MAC(rc, localQueueNumber);
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsHeadroomMonitorSwDbSet
 * @endinternal
 *
 * @brief  Set monitored (for headroom peak conters)  tile and DP to software DB.
 *            This is used in order to query from correct tile and dp when getting peak counters.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                          - physical device number
 * @param[in] tileNum                  -          current tile id(APPLICABLE RANGES:0..3).
 * @param[in] localDpNum                   -  DP (APPLICABLE RANGES:0..7).

 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssFalconTxqUtilsHeadroomMonitorSwDbSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 localDpNum
)
{
    GT_U32                  i, numberOfTiles;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    PRV_CPSS_NUM_OF_TILE_GET_MAC(devNum, numberOfTiles);
    for (i = 0; i < numberOfTiles; i++)
    {
        tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[i]);
        if (i == tileNum)
        {
            tileConfigsPtr->headRoomDb.activeDp     = localDpNum;
            tileConfigsPtr->headRoomDb.monitorActive    = GT_TRUE;
        }
        else
        {
            tileConfigsPtr->headRoomDb.monitorActive = GT_FALSE;
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsHeadroomMonitorSwDbGet
 * @endinternal
 *
 * @brief  Get monitored (for headroom peak conters)  tile and DP to software DB.
 *            This is used in order to query from correct tile and dp when getting peak counters.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                          -       physical device number
 * @param[in] tileNumPtr                  -          (pointer to)current tile id(APPLICABLE RANGES:0..3).
 * @param[in] localDpNumPtr                   -  (pointer to)DP (APPLICABLE RANGES:0..7).

 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_FOUND -             in case no tile/DP is assigned for monitoring
 *
 *
 */
GT_STATUS  prvCpssFalconTxqUtilsHeadroomMonitorSwDbGet
(
    IN GT_U8 devNum,
    OUT GT_U32                             *tileNumPtr,
    OUT GT_U32                             *localDpNumPtr
)
{
    GT_U32                  i, numberOfTiles;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    PRV_CPSS_NUM_OF_TILE_GET_MAC(devNum, numberOfTiles);
    for (i = 0; i < numberOfTiles; i++)
    {
        tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[i]);
        if (GT_TRUE == tileConfigsPtr->headRoomDb.monitorActive)
        {
            *tileNumPtr = i;
            *localDpNumPtr  = tileConfigsPtr->headRoomDb.activeDp;
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}
/**
 * @internal prvCpssFalconTxqUtilsHeadroomThresholdSet
 * @endinternal
 *
 * @brief  Set threshold for headroom size interrupt.Once one  or more of the Port. TC has
 *   crossed the configured Threshold, an interrupt is asserted and the Status of the Port. TC is written
 *   to the HR crossed Threshold Status.
 *
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -physical device number
 * @param[in] portNum                    physical port number
 * @param[in] threshold                   Headroom threshold
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsHeadroomThresholdSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 threshold
)
{
    GT_U32              tileNum, dpNum, localPortNum;
    GT_STATUS           rc;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    /*Find tile*/
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    if (mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Headroom threshold is not supported for remote port");
    }
    rc = prvCpssFalconTxqQfcHeadroomThresholdSet(devNum, tileNum, dpNum, localPortNum, threshold);
    PRV_CPSS_TXQ_UTILS_CHECK_RESULT_MAC(rc, dpNum);
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsHeadroomThresholdGet
 * @endinternal
 *
 * @brief  Set threshold for headroom size interrupt.Once one  or more of the Port. TC has
 *   crossed the configured Threshold, an interrupt is asserted and the Status of the Port. TC is written
 *   to the HR crossed Threshold Status.
 *
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -physical device number
 * @param[in] portNum                    physical port number
 * @param[out] thresholdPtr                   (pointer to)Headroom threshold
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsHeadroomThresholdGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32                                      *thresholdPtr
)
{
    GT_U32              tileNum, dpNum, localPortNum;
    GT_STATUS           rc;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    /*Find tile*/
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    if (mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Headroom threshold is not supported for remote port");
    }
    rc = prvCpssFalconTxqQfcHeadroomThresholdGet(devNum, tileNum, dpNum, localPortNum, thresholdPtr);
    PRV_CPSS_TXQ_UTILS_CHECK_RESULT_MAC(rc, dpNum);
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsUpdateSelectList  function
 * @endinternal
 *
 * @brief   Update select list pointers according to mapping with reservation for low latency.
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 * @param[in] physicalPortNum                  - physical or CPU port number
 * @param[in] queueOffset                  - traffic class queue on this device (0..7)
 * @param[in] enablePtr                   -(pointer to) GT_TRUE,  enable queue  low latency mode,
 *                                                         GT_FALSE, disable queue  low latency mode
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device number, profile set or
 *                                       traffic class
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters is NULL pointer
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
 *                                                contain latency queue.
 */
GT_STATUS prvCpssFalconTxqUtilsUpdateSelectListParamsSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 dpNum,
    IN GT_U32 localPortNum,
    IN GT_U32 firstQ,
    IN GT_U32 lastQ,
    IN GT_BOOL hiPrioExist
)
{
    GT_STATUS                   rc;
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE   range;
    range.prio0LowLimit = firstQ;
    if (GT_FALSE == hiPrioExist)
    {
        range.prio0HighLimit    = lastQ;
        range.prio1LowLimit = 0;
        range.prio1HighLimit    = 0;
    }
    else
    {
        range.prio0HighLimit    = lastQ - 1;
        range.prio1LowLimit = lastQ;
        range.prio1HighLimit    = lastQ;
    }
    /*set SDQ related configuration*/
    rc = prvCpssFalconTxqSdqSelectListSet(devNum, tileNum, dpNum, localPortNum, &range);
    if (rc)
    {
        return rc;
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsLowLatencyGet  function
 * @endinternal
 *
 * @brief   Get enable/disble low latency mode on queue.
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 * @param[in] physicalPortNum                  - physical or CPU port number
 * @param[in] queueOffset                  - traffic class queue on this device (0..7)
 * @param[out] enablePtr                   -(pointer to) GT_TRUE,  enable queue  low latency mode,
 *                                                         GT_FALSE, disable queue  low latency mode
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device number, profile set or
 *                                       traffic class
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters is NULL pointer
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
 *                                                contain latency queue.
 */
GT_STATUS prvCpssFalconTxqUtilsLowLatencyGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    IN GT_U32 queueOffset,
    OUT GT_BOOL                                     *enablePtr
)
{
    GT_STATUS           rc;
    GT_U32              localQueueOffset;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *aNodePtr;
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    localQueueOffset = aNodePtr->queuesData.queueBase + queueOffset;
    return prvCpssFalconTxqSdqQueueStrictPriorityGet(devNum, aNodePtr->queuesData.tileNum,
                             aNodePtr->queuesData.dp, localQueueOffset, enablePtr);
}
/**
 * @internal prvCpssFalconTxqUtilsLowLatencySet  function
 * @endinternal
 *
 * @brief   Enable/disble low latency mode on queue.
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 * @param[in] physicalPortNum                  - physical or CPU port number
 * @param[in] queueOffset                  - traffic class queue on this device (0..7)
 * @param[in] enable                   - GT_TRUE,  enable queue  low latency mode,
 *                                                         GT_FALSE, disable queue  low latency mode
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device number, profile set or
 *                                       traffic class
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters is NULL pointer
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
 *                                                contain latency queue.
 */
GT_STATUS prvCpssFalconTxqUtilsLowLatencySet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    IN GT_U32 queueOffset,
    IN GT_BOOL enable
)
{
    GT_STATUS           rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr;
    GT_BOOL             lowLatencyQueuePresent;
    GT_U32              lowLatencyQueueOffset = 0;
    GT_U32              localQueueOffset;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *aNodePtr;
    GT_U32              dummy[2], localdpPortNum, firstQueueInDp;
    GT_BOOL             isResourceValid;
    rc = prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapGet(devNum, physicalPortNum, &isResourceValid);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapGet failed for %d", physicalPortNum);
    }
    if (GT_TRUE == isResourceValid)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Low latency queue should be configured when port state is DOWN", physicalPortNum);
    }
    /*Check if there is  low latency queue enabled in P node*/
    rc = prvCpssFalconTxqUtilsFindPnodeByPhysicalPortNumber(devNum, physicalPortNum, &pNodePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsFindPnodeByPhysicalPortNumber failed for %d", physicalPortNum);
    }
    rc = prvCpssFalconTxqUtilsLowLatencyQueuePresentInPnodeGet(devNum, pNodePtr, &lowLatencyQueuePresent, &lowLatencyQueueOffset);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsLowLatencyQueuePresentInPnodeGet failed ");
    }
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    localQueueOffset = aNodePtr->queuesData.queueBase + queueOffset;
    if ((GT_TRUE == enable) && (GT_TRUE == lowLatencyQueuePresent))
    {
        if (lowLatencyQueueOffset != localQueueOffset)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Low latency already configured for queue %d ", lowLatencyQueueOffset);
        }
        return GT_OK;
    }
    rc = prvCpssFalconTxqSdqQueueStrictPrioritySet(devNum, aNodePtr->queuesData.tileNum,
                               aNodePtr->queuesData.dp, localQueueOffset, enable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqSdqQueueStrictPrioritySet failed ");
    }
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, physicalPortNum, dummy, dummy + 1, &localdpPortNum, NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", physicalPortNum);
    }
    rc = prvCpssFalconTxqUtilsStartQueueIndexForDpGet(devNum, aNodePtr->queuesData.tileNum, aNodePtr->queuesData.dp, &firstQueueInDp);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssFalconTxqUtilsUpdateSelectListParamsSet(devNum, aNodePtr->queuesData.tileNum, aNodePtr->queuesData.dp, localdpPortNum,
                                pNodePtr->aNodelist[0].queuesData.pdqQueueFirst - firstQueueInDp,
                                pNodePtr->aNodelist[pNodePtr->aNodeListSize - 1].queuesData.pdqQueueLast - firstQueueInDp,
                                enable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssSip6_10TxqSdqSelectListEnableSet(devNum, aNodePtr->queuesData.tileNum, aNodePtr->queuesData.dp, localdpPortNum, GT_FALSE, enable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6_10TxqSdqSelectListEnableSet failed ");
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqPdsProfileGet  function
 * @endinternal
 *
 * @brief   Get the profile according to speed and  length adjust parameters.
 *             If profile does not exist the function create one.
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -              physical device number
 * @param[in] speed_in_G             -              Required speed in Gygabyte
 * @param[in] lengthAdjustParametersPtr  (pointer to)lenght adjust parameters
 * @param[out] profilePtr                             -(pointer to)profile index
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device number, profile set or
 *                                       traffic class
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters is NULL pointer
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
 *                                                contain latency queue.
 */
static GT_STATUS prvCpssFalconTxqPdsProfileGet
(
    IN GT_U8 devNum,
    IN GT_U32 speed_in_G,
    IN PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC *sourcePtr,
    OUT GT_U32  *profilePtr
)
{
    GT_U32                              i;
    GT_U32                              firstFree   = CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM;
    GT_STATUS                           rc      = GT_OK;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC *    target;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(profilePtr);
    /*check already existing profiles*/
    for (i = 0; i < CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM; i++)
    {
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].numberOfBindedPorts > 0)
        {
            if (speed_in_G == PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].speed_in_G)
            {
                if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
                {
                    target = PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[i].lengthAdjustParametersPtr;
                    if ((sourcePtr == target) || (0 == cpssOsMemCmp((GT_VOID *)sourcePtr, (GT_VOID *)target,
                                            sizeof(PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC)))
                        )
                    {
                        *profilePtr = i;
                        break;
                    }
                }
                else
                {
                    *profilePtr = i;
                    /*in SIP 6_10 there is no coupling between long queue and length adjust*/
                    break;
                }
            }
        }
        else
        {
            if (firstFree == CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM)
            {
                firstFree = i;
            }
        }
    }
    /*there is no existing profile,create new*/
    if (i == CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM)
    {
        if (firstFree == CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM)
        {
            rc = GT_FAIL;
        }
        else
        {
            *profilePtr                             = firstFree;
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[firstFree].speed_in_G    = speed_in_G;
            if (sourcePtr && (PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[firstFree].lengthAdjustParametersPtr))
            {
                *(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[firstFree].lengthAdjustParametersPtr) =
                    *sourcePtr;
            }
            else
            {
            }
            rc = prvCpssDxChTxqPdsLongQueueAttributesGet(devNum, GT_TRUE, &speed_in_G,
                                     &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[firstFree].longQueueParametersPtr));
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvDxChPortTxQueueProfileHwSet(devNum, firstFree,
                                &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[firstFree]));
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return rc;
}
/**
 * @internal prvDxChPortTxQueueProfileHwSet  function
 * @endinternal
 *
 * @brief   Update HW with PDS queue profile
 *
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -              physical device number
 * @param[in] speed_in_G             -              Required speed in Gygabyte
 * @param[in] profileIndex                            index to set
 * @param[in] profilePtr                             -(pointer to)profile attributes
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device number, profile set or
 *                                       traffic class
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters is NULL pointer
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
 *                                                contain latency queue.
 */
static GT_STATUS prvDxChPortTxQueueProfileHwSet
(
    IN GT_U8 devNum,
    IN GT_U32 profileIndex,
    IN PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC * profilePtr
)
{
    GT_U32      tileNumber;
    GT_U32      numDpPerTile;/* num of DP units per tile */
    GT_U32      i, j;
    GT_STATUS   rc;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(profilePtr);
    switch (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
    case 0:
    case 1:
        tileNumber = 1;
        break;
    case 2:
    case 4:
        tileNumber = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ", PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
        break;
    }
    numDpPerTile = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp / tileNumber;
    /*Duplicate to all tiles and PDS*/
    for (i = 0; i < tileNumber; i++)
    {
        for (j = 0; j < numDpPerTile; j++)
        {
            rc = prvCpssDxChTxqFalconPdsProfileSet(devNum, i, j, profileIndex, profilePtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxqFalconPdsProfileSet fails with code %d ", rc);
            }
        }
    }
    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqUtilsGlobalFlowControlEnableGet  function
 * @endinternal
 *
 * @brief   Sets the number of bytes added/subtracted to the length of every frame
 *         for shaping/scheduling rate calculation for given port when
 *         Byte Count Change is enabled.
 *
 * @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 * @param[in] portNum                  - physical or CPU port number
 * @param[in] bcValue                  - number of bytes added to the length of every frame
 *                                      for shaping/scheduling rate calculation.
 *                                      (APPLICABLE RANGES:  0..63)
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device number, profile set or
 *                                       traffic class
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters is NULL pointer
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
 *                                                contain latency queue.
 */
GT_STATUS prvCpssFalconTxqPdsByteCountChangeValueSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_32 bcValue
)
{
    GT_STATUS                       rc = GT_OK;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC  lengthAdjustParameters;
    lengthAdjustParameters.lengthAdjustEnable   = GT_TRUE;
    lengthAdjustParameters.lengthAdjustSubstruct    = (bcValue < 0) ? GT_TRUE : GT_FALSE;
    lengthAdjustParameters.lengthAdjustByteCount    = ABS(bcValue);
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
    {
        rc = prvCpssDxChTxqBindPortQueuesToPdsProfile(devNum, portNum,
                                  CPSS_PORT_SPEED_50000_E /*don't care*/, &lengthAdjustParameters);
    }
    else
    {
        rc = prvCpssDxChTxqBindPortQueuesToLengthAdjustProfile(devNum, portNum,
                                       &lengthAdjustParameters);
    }
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChTxqBindPortQueuesToPdsProfile, portNum = %d\n", portNum);
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqPdsByteCountChangeValueGet function
 * @endinternal
 *
 * @brief   Gets the number of bytes added/subtracted to the length of every frame
 *         for shaping/scheduling rate calculation for given port when
 *         Byte Count Change is enabled.
 *
 * @note   APPLICABLE DEVICES:          Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 * @param[in] portNum                  - physical or CPU port number
 *
 * @param[out] bcValuePtr               - (pointer to) number of bytes added to the length of
 *                                      every frame for shaping/scheduling rate calculation.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqPdsByteCountChangeValueGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_32                    *bcValue
)
{
    GT_STATUS                       rc;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC        profileParameters;
    GT_U32                          profileIndex;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC     longQueueAttributes;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC  lengthAdjustParameters;
    profileParameters.longQueueParametersPtr    = &longQueueAttributes;
    profileParameters.lengthAdjustParametersPtr = &lengthAdjustParameters;
    rc                      = prvCpssDxChTxqBindQueueToLengthAdjustProfileGet(devNum, portNum, 0, &profileIndex);

    /*backword compatability*/
    if(rc == GT_NOT_FOUND)
    {
       rc = GT_BAD_PARAM;
    }
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChTxqBindQueueToProfileGet, portNum = %d\n", portNum);
    }
    rc = prvCpssDxChTxqFalconPdsProfileGet(devNum, 0, 0, profileIndex, GT_TRUE, &profileParameters);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChTxqFalconPdsProfileGet, profileIndex = %d\n", profileIndex);
    }
    *bcValue = profileParameters.lengthAdjustParametersPtr->lengthAdjustByteCount;
    if (GT_TRUE == profileParameters.lengthAdjustParametersPtr->lengthAdjustSubstruct)
    {
        (*bcValue) *= (-1);
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqClusterSegmentCrossingGet function
 * @endinternal
 *
 * @brief   Dected L2 cluster crossing at scheduler tree
 *
 * @note   APPLICABLE DEVICES:          Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -  physical device number
 * @param[in] startNodeId              - index of first node
 * @param[in] endNodeId               - index of last node
 * @param[in] clusterSize               - size of L2 cluster
 * @param[out] newStartClusterPtr -  index of cluster that contain endNodeId
 * @retval GT_TRUE                    -Cluster segment crossing detected
 * @retval GT_FALSE                    -Cluster segment crossing is not detected
 */
static GT_BOOL prvCpssFalconTxqClusterSegmentCrossingGet
(
    IN GT_U8 devNum,
    IN GT_U32 startNodeId,
    IN GT_U32 endNodeId,
    IN GT_U32 clusterSize,
    OUT GT_U32                   *newStartClusterPtr
)
{
    GT_U32 startSegmentId;
    /*Relevant only for Falcon*/
    if (GT_TRUE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        return GT_FALSE;
    }
    startSegmentId      = startNodeId / clusterSize;
    *newStartClusterPtr = endNodeId / clusterSize;
    return (*newStartClusterPtr > startSegmentId);
}
/**
 * @internal prvCpssFalconTxqUtilsPortQueueOffsetValidGet function
 * @endinternal
 *
 * @brief  This function check that queue offset is within port range
 *
 * @note   APPLICABLE DEVICES:          Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -  physical device number
 * @param[in] portNum                  - physical or CPU port number
 * @param[in] offset                  -       traffic class queue on this device (0..15)
 * @param[out] validPtr                  -  equals GT_TRUE in case the offset is within alowed range,GT_FALSE otherwise
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsPortQueueOffsetValidGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 offset,
    OUT GT_BOOL                  *validPtr
)
{
    GT_STATUS           rc;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT dummyMapping;
    GT_U32              firstQueue, lastQueue, dummy[2];
    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    rc = prvCpssDxChTxqPhyPortSdqQueueRangeGet(devNum, portNum, &dummyMapping, &firstQueue, &lastQueue, dummy, dummy + 1);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqPhyPortSdqQueueRangeGet  failed for portNum  %d  ", portNum);
    }
    if ( offset > lastQueue - firstQueue)
    {
        *validPtr = GT_FALSE;
    }
    else
    {
        *validPtr = GT_TRUE;
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqUtilsPhysicalPortNumberGet function
 * @endinternal
 *
 * @brief  Find mapped physical port by tile/dp/local port
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -  physical device number
 * @param[in] tileNum                  -  Number of the tile (APPLICABLE RANGES:0..4).
 * @param[in] dpNum                    - Number of the dp (APPLICABLE RANGES:0..7).
 * @param[in] localPortNum             - Number of the local port (APPLICABLE RANGES:0..8).
 * @param[out] portNumPtr             -Physical port number.
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsPhysicalPortNumberGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 dp,
    IN GT_U32 localPort,
    OUT GT_PHYSICAL_PORT_NUM *portNumPtr
)
{

    GT_U32                  pNodeIndex;
    GT_STATUS               rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pNodePtr = NULL;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(dp);
    *portNumPtr = CPSS_SIP6_TXQ_INVAL_DATA_CNS;

    rc  = prvCpssSip6TxqUtilsPnodeIndexGet(devNum, dp, localPort,&pNodeIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqUtilsPnodeIndexGet failed ");
    }

    PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pNodePtr,devNum,tileNum,pNodeIndex);

    if (pNodePtr->isCascade)
    {
        *portNumPtr = pNodePtr->cascadePhysicalPort;
    }
    else
    {
        if (pNodePtr->aNodeListSize > 0)
        {
            *portNumPtr = pNodePtr->aNodelist[0].physicalPort;
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsPhysicalPortNumberGet function
 * @endinternal
 *
 * @brief  Find  physical port that triggered headroom crossed interrupt by info
 *       delivered by event
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -  physical device number
 * @param[in] tileNum                  -  Number of the tile (APPLICABLE RANGES:0..4).
 * @param[in] pipe                     -     Number of control pipe (APPLICABLE RANGES:0..1).
 * @param[in] dpNum                    - Local (per pipe) Number of the dp (APPLICABLE RANGES:0..3).
 * @param[in]reg                              Register index (APPLICABLE RANGES:0..3).
 * @param[in] port             -             Port index in register(APPLICABLE RANGES:0..2).
 * @param[out] portNumPtr             -Physical port number.
 * @retval GT_OK                    -            on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               -    on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsHeadroomTriggerPortGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pipe,
    IN GT_U32 dp,
    IN GT_U32 reg,
    IN GT_U32 port,
    OUT GT_PHYSICAL_PORT_NUM *portNumPtr
)
{
    return prvCpssFalconTxqUtilsPhysicalPortNumberGet(devNum, tileNum, pipe * 4 + dp,
                              reg * PRV_QFC_GROUP_SIZE_CNS + port, portNumPtr);
}
/**
 * @internal prvCpssTxqUtilsTreeIsReadyGet function
 * @endinternal
 *
 * @brief  Gheck if scheduling tree is build for all device
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -  physical device number
 * @param[out] readyPtr             -equals GT_TRUE is scheduling tree is ready,GT_FALSE otherwise
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssTxqUtilsTreeIsReadyGet
(
    IN GT_U8 devNum,
    OUT GT_BOOL *readyPtr
)
{
    GT_U32 numberOfTiles, i;
    PRV_CPSS_NUM_OF_TILE_GET_MAC(devNum, numberOfTiles);
    CPSS_NULL_PTR_CHECK_MAC(readyPtr);
    *readyPtr = GT_TRUE;
    for (i = 0; i < numberOfTiles; i++)
    {
        if ((PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedLibIsInitilized[i] != GT_TRUE))
        {
            *readyPtr = GT_FALSE;
            break;
        }
    }
    return GT_OK;
}
GT_STATUS prvCpssTxqUtilsTileLocalDpLocalDmaNumToMacConvert
(
    IN GT_U8 devNum,
    IN GT_U32 tileId,
    IN GT_U32 localDpIndexInTile,
    IN GT_U32 localDmaNumInDp,
    OUT GT_U32  *macNumPtr
)
{
    GT_STATUS       rc;
    GT_PHYSICAL_PORT_NUM    physicalPortNum;
    /* get physical  port */
    rc = prvCpssFalconTxqUtilsPhysicalPortNumberGet(devNum, tileId,
                            localDpIndexInTile, localDmaNumInDp, &physicalPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (physicalPortNum != CPSS_SIP6_TXQ_INVAL_DATA_CNS)
    {
        if(physicalPortNum>=PRV_CPSS_MAX_PP_PORTS_NUM_CNS)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "physicalPortNum bigger then %d ",PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
        }
        PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, physicalPortNum, *macNumPtr);
    }
    else
    {
        rc = GT_NOT_FOUND;
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqUtilsMappedQueueNumGet function
 * @endinternal
 *
 * @brief  Get number of mapped queues per tile
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -  physical device number
 * @param[in] tileNum                   -  tile number
 * @param[out] numOfMappedPortsPtr             number of mapped ports
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsMappedQueueNumGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    OUT GT_U32                               *numOfMappedPtr
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    CPSS_NULL_PTR_CHECK_MAC(numOfMappedPtr);
    tileConfigsPtr  = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum]);
    *numOfMappedPtr = tileConfigsPtr->mapping.mappedQNum;
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsMappedQueueNumGet function
 * @endinternal
 *
 * @brief  Get global index of the first queue mapped to physical port
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -                  physical device number
 * @param[in] physicalPortNum                   -    physical port number
 * @param[out] globalUniqueIndexPtr             global index of the first queue mapped to physical port
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsPortGlobalUniqueIndexGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    OUT GT_U32                   *globalUniqueIndexPtr
)
{
    GT_STATUS               rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr;
    GT_BOOL                             isCascade;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE      *pNodePtr;

    CPSS_NULL_PTR_CHECK_MAC(globalUniqueIndexPtr);

    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,physicalPortNum,&isCascade,&pNodePtr);
    if (GT_OK != rc )
    {
        return rc;
    }

    if(GT_FALSE == isCascade)
    {
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        *globalUniqueIndexPtr = aNodePtr->qGlobalOffset;
    }
    else
    {
        /*map to first A node*/
        *globalUniqueIndexPtr = pNodePtr->aNodelist[0].qGlobalOffset;
    }
    return GT_OK;
}

/**
 * @internal prvCpssSip6TxqLibInitDoneSet function
 * @endinternal
 *
 * @brief  Set txQ library init finished
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                       -physical device number
 * @param[in] initDone                                          GT_TRUE if txQ library init is done,GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS  prvCpssSip6TxqLibInitDoneSet
(
    IN GT_U8 devNum,
    IN GT_BOOL initDone
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    /*Set only tile 0*/
    tileConfigsPtr  = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[0]);

    tileConfigsPtr ->general.txLibInitDone = initDone;
    return GT_OK;
}
/**
 * @internal prvCpssSip6TxqLibInitDoneSet function
 * @endinternal
 *
 * @brief  Set txQ library init finished
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                       -physical device number
 * @param[out] initDonePtr                                 (pointer to)GT_TRUE if txQ library init is done,GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS  prvCpssSip6TxqLibInitDoneGet
(
    IN GT_U8 devNum,
    OUT GT_BOOL *initDonePtr
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    /*Get only tile 0*/
    tileConfigsPtr  = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[0]);

    *initDonePtr = tileConfigsPtr ->general.txLibInitDone;

    return GT_OK;
}

GT_STATUS prvCpssSip6TxQUtilsOptimizedThroughputEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8                       tcQueue,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS               rc;
    GT_U32                  tileNum;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    anodePtr = NULL;
    GT_U32                  dummy = 0, localdpPortNum, dpNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
    GT_BOOL                 isCascade;

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }

    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum, &isCascade, NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsIsCascadePort  failed for portNum  %d  ", portNum);
    }
    if (isCascade == GT_TRUE)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " Cascade port %d is not supported. Optimized throughput should be set to local or remote port.", portNum);
    }
    else
    {
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, portNum, &tileNum, &dummy);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet  failed for portNum  %d  ", portNum);
        }
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &anodePtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
        }

        if(GT_TRUE == enable)
        {
            anodePtr->queuesData.semiEligBmp|=(1<<tcQueue);
        }
        else
        {
            anodePtr->queuesData.semiEligBmp&=(~(1<<tcQueue));
        }

     }

    return GT_OK;
}

GT_STATUS prvCpssSip6TxQUtilsOptimizedThroughputEnableGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8                       tcQueue,
    IN  GT_BOOL                     *enablePtr
)
{
    GT_STATUS               rc;
    GT_U32                  tileNum;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    anodePtr = NULL;
    GT_U32                  dummy = 0, localdpPortNum, dpNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
    GT_BOOL                 isCascade;

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }

    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum, &isCascade, NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsIsCascadePort  failed for portNum  %d  ", portNum);
    }
    if (isCascade == GT_TRUE)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " Cascade port %d is not supported. Optimized throughput should be set to local or remote port.", portNum);
    }
    else
    {
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, portNum, &tileNum, &dummy);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet  failed for portNum  %d  ", portNum);
        }
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &anodePtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
        }

        if(anodePtr->queuesData.semiEligBmp&(1<<tcQueue))
        {
           *enablePtr = GT_TRUE;
        }
        else
        {
           *enablePtr = GT_FALSE;
        }

     }

    return GT_OK;
}


GT_STATUS prvCpssSip6TxqUtilsDataPathEvent
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    IN PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_ENT event,
    IN GT_BOOL              preemtedChannelEvent
)
{
    GT_STATUS rc ;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * aNodePtr[2]={NULL,NULL};
    PRV_CPSS_PDQ_A_NODE_DATA_PATH_STATE_ENT currentState,newState;
    GT_U32 currentPort =0;
    GT_CHAR_PTR stateNames []= {"INITIAL","TXQ_PORT_ENABLED","TXD_RESOURCE_CONFIGURED","FULL"};
    GT_CHAR_PTR eventNames []= {"TXD_RESOURCE_SET","TXQ_PORT_ENABLE","TXD_RESOURCE_CLEAR","TXQ_PORT_DISABLE"};
    GT_BOOL     isCascade = GT_FALSE;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr;


    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,physicalPortNum,&isCascade,&pNodePtr);
    if (rc)
    {
        return rc;
    }

    /*in cascade case take child 0*/
    if(GT_TRUE==isCascade)
    {
        physicalPortNum = pNodePtr->aNodelist[0].physicalPort;
    }

    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr[0]);
    if (rc)
    {
        return rc;
    }

    if(aNodePtr[0]->mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        return GT_OK;
    }

    /*Duplicate to preemptive channel*/
    if(GT_TRUE==preemtedChannelEvent)
    {
        rc = prvCpssDxChTxqSip6PreeptivePartnerAnodeGet(devNum, aNodePtr[0],&(aNodePtr[1]));
        if (rc != GT_OK)
        {
            return rc;
        }

        currentState = aNodePtr[1]->queuesData.dataPathState;
        currentPort = aNodePtr[1]->physicalPort;
    }
    else
    {
        currentState = aNodePtr[0]->queuesData.dataPathState;
        currentPort = aNodePtr[0]->physicalPort;
    }

    newState = currentState;

    switch(currentState)
    {
        case PRV_CPSS_PDQ_A_NODE_DATA_PATH_STATE_TXQ_PORT_ENABLED_ENT:
            switch(event)
            {
                case PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXQ_PORT_DISABLED:
                    newState = PRV_CPSS_PDQ_A_NODE_DATA_PATH_STATE_INITIAL_ENT;
                    break;
                case PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXD_RESOURCE_SET:
                    newState = PRV_CPSS_PDQ_A_NODE_DATA_PATH_STATE_FULLY_CONFIGURED_ENT;
                    break;
                default:
                    break;
            }
            break;
        case PRV_CPSS_PDQ_A_NODE_DATA_PATH_STATE_TXD_RESOURCE_CONFIGURED_ENT:
            switch(event)
            {
                case PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXQ_PORT_ENABLED:
                    newState = PRV_CPSS_PDQ_A_NODE_DATA_PATH_STATE_FULLY_CONFIGURED_ENT;
                    break;
                case PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXD_RESOURCE_CLEAR:
                    newState = PRV_CPSS_PDQ_A_NODE_DATA_PATH_STATE_INITIAL_ENT;
                    break;
                default:
                    break;
            }
            break;
        case PRV_CPSS_PDQ_A_NODE_DATA_PATH_STATE_FULLY_CONFIGURED_ENT:
            switch(event)
            {
                case PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXD_RESOURCE_CLEAR:
                    newState = PRV_CPSS_PDQ_A_NODE_DATA_PATH_STATE_TXQ_PORT_ENABLED_ENT;
                    break;
                case PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXQ_PORT_DISABLED:
                    newState = PRV_CPSS_PDQ_A_NODE_DATA_PATH_STATE_TXD_RESOURCE_CONFIGURED_ENT;
                    break;
                default:
                    break;
            }
            break;
        default:/*PRV_CPSS_PDQ_A_NODE_DATA_PATH_STATE_INITIAL_ENT*/
            switch(event)
            {
                case PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXD_RESOURCE_SET:
                    newState = PRV_CPSS_PDQ_A_NODE_DATA_PATH_STATE_TXD_RESOURCE_CONFIGURED_ENT;
                    break;
                case PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXQ_PORT_ENABLED:
                    newState = PRV_CPSS_PDQ_A_NODE_DATA_PATH_STATE_TXQ_PORT_ENABLED_ENT;
                    break;
                default:
                    break;
            }
            break;
    }

    if(GT_TRUE==preemtedChannelEvent)
    {
      aNodePtr[1]->queuesData.dataPathState = newState;
    }
    else
    {
      aNodePtr[0]->queuesData.dataPathState = newState;
    }


    if(GT_TRUE == prvCpssSip6TxqUtilsDataPathEventLogGet(devNum))
    {
      cpssOsPrintf("Port %d : Current state  %s -> [ event %s ]->  New state %s\n",currentPort,stateNames[currentState],eventNames[event],
        stateNames[newState]);
    }

    return GT_OK;
}

/**
 * @internal prvCpssSip6TxqUtilsMaxNumOfQueuesPerPortGet function
 * @endinternal
 *
 * @brief  Get maximal number of queues that can be mapped to port
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X;Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                       -physical device number
 * @param[out] maxNumOfQueuesPtr             (pointer to)maximal number of ports
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssSip6TxqUtilsMaxNumOfQueuesPerPortGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *maxNumOfQueuesPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_PORT_TXQ_OFFSET_WIDTH_ENT offset;

    CPSS_NULL_PTR_CHECK_MAC(maxNumOfQueuesPtr);

     *maxNumOfQueuesPtr = PRV_CPSS_DXCH_SIP6_QUEUES_PER_PORT_MAX_Q_NUM_VAL_MAC;

    if (GT_TRUE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
       rc = cpssDxChPortTxQueueOffsetWidthGet(devNum,&offset);
       if(rc!=GT_OK)
       {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChPortTxQueueOffsetWidthGet  failed .");
       }
       if(offset==CPSS_DXCH_PORT_TXQ_OFFSET_WIDTH_8_BIT_ENT)
       {
        *maxNumOfQueuesPtr = PRV_CPSS_DXCH_SIP6_20_QUEUES_PER_PORT_MAX_Q_NUM_VAL_MAC;
       }
    }

    return GT_OK;
}


/**
 * @internal prvCpssSip6TxqUtilsMaxQueueOffsetGet function
 * @endinternal
 *
 * @brief  Get  number of queues that  mapped to port
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X;Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                       -physical device number
 * @param[out] maxNumOfQueuesPtr             (pointer to)maximal number of ports
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssSip6TxqUtilsMaxQueueOffsetGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               *maxQOffsetPtr
)
{
    GT_STATUS           rc;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT dummyMapping;
    GT_U32              firstQueue, lastQueue, dummy[2];
    CPSS_NULL_PTR_CHECK_MAC(maxQOffsetPtr);
    rc = prvCpssDxChTxqPhyPortSdqQueueRangeGet(devNum, portNum, &dummyMapping, &firstQueue, &lastQueue, dummy, dummy + 1);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqPhyPortSdqQueueRangeGet  failed for portNum  %d  ", portNum);
    }

    *maxQOffsetPtr =  lastQueue - firstQueue;

    return rc;
}


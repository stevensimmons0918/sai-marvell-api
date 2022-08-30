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
* @file prvCpssDxChTxqPreemptionUtils.c
*
* @brief CPSS SIP6 TXQ preemtion feature implementation layer
*
* @version   1
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS



#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPdx.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqSearchUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqShapingUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqSchedulingUtils.h>
#include <cpss/common/port/cpssPortStat.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* include debug functions */
#define INC_DEBUG_FUNCTIONS


/* start of register BR_TX_FRAG_COUNT*/

#define      PRV_BR_STAT_MAC100_REG_BR_TX_FRAG_COUNT_TX_FRAG_COUNT_FIELD_OFFSET            0

#define      PRV_BR_STAT_MAC100_REG_BR_TX_FRAG_COUNT_TX_FRAG_COUNT_FIELD_SIZE          32

/* end of BR_TX_FRAG_COUNT*/


/* start of register BR_RX_FRAG_COUNT*/

#define      PRV_BR_STAT_MAC100_REG_BR_RX_FRAG_COUNT_RX_FRAG_COUNT_FIELD_OFFSET            0

#define      PRV_BR_STAT_MAC100_REG_BR_RX_FRAG_COUNT_RX_FRAG_COUNT_FIELD_SIZE          32

/* end of BR_RX_FRAG_COUNT*/


/* start of register BR_TX_HOLD_COUNT*/

#define      PRV_BR_STAT_MAC100_REG_BR_TX_HOLD_COUNT_TX_HOLD_COUNT_FIELD_OFFSET            0

#define      PRV_BR_STAT_MAC100_REG_BR_TX_HOLD_COUNT_TX_HOLD_COUNT_FIELD_SIZE          32

/* end of BR_TX_HOLD_COUNT*/


/* start of register BR_RX_SMD_ERR_COUNT*/

#define      PRV_BR_STAT_MAC100_REG_BR_RX_SMD_ERR_COUNT_RX_SMD_ERR_COUNT_FIELD_OFFSET          0

#define      PRV_BR_STAT_MAC100_REG_BR_RX_SMD_ERR_COUNT_RX_SMD_ERR_COUNT_FIELD_SIZE            8

/* end of BR_RX_SMD_ERR_COUNT*/


/* start of register BR_RX_ASSY_ERR_COUNT*/

#define      PRV_BR_STAT_MAC100_REG_BR_RX_ASSY_ERR_COUNT_RX_ASSY_ERR_COUNT_FIELD_OFFSET            0

#define      PRV_BR_STAT_MAC100_REG_BR_RX_ASSY_ERR_COUNT_RX_ASSY_ERR_COUNT_FIELD_SIZE          8

/* end of BR_RX_ASSY_ERR_COUNT*/


/* start of register BR_RX_ASSY_OK_COUNT*/

#define      PRV_BR_STAT_MAC100_REG_BR_RX_ASSY_OK_COUNT_RX_ASSY_OK_COUNT_FIELD_OFFSET          0

#define      PRV_BR_STAT_MAC100_REG_BR_RX_ASSY_OK_COUNT_RX_ASSY_OK_COUNT_FIELD_SIZE            32

/* end of BR_RX_ASSY_OK_COUNT*/


/* start of register BR_RX_VERIFY_COUNT*/

#define      PRV_BR_STAT_MAC100_REG_BR_RX_VERIFY_COUNT_RX_VERIFY_COUNT_GOOD_FIELD_OFFSET           0
#define      PRV_BR_STAT_MAC100_REG_BR_RX_VERIFY_COUNT_RX_VERIFY_COUNT_BAD_FIELD_OFFSET            16

#define      PRV_BR_STAT_MAC100_REG_BR_RX_VERIFY_COUNT_RX_VERIFY_COUNT_GOOD_FIELD_SIZE         8
#define      PRV_BR_STAT_MAC100_REG_BR_RX_VERIFY_COUNT_RX_VERIFY_COUNT_BAD_FIELD_SIZE          8

/* end of BR_RX_VERIFY_COUNT*/


/* start of register BR_RX_RESPONSE_COUNT*/

#define      PRV_BR_STAT_MAC100_REG_BR_RX_RESPONSE_COUNT_RX_RESP_COUNT_GOOD_FIELD_OFFSET           0
#define      PRV_BR_STAT_MAC100_REG_BR_RX_RESPONSE_COUNT_RX_RESP_COUNT_BAD_FIELD_OFFSET            16


#define      PRV_BR_STAT_MAC100_REG_BR_RX_RESPONSE_COUNT_RX_RESP_COUNT_GOOD_FIELD_SIZE         8
#define      PRV_BR_STAT_MAC100_REG_BR_RX_RESPONSE_COUNT_RX_RESP_COUNT_BAD_FIELD_SIZE          8

/* end of BR_RX_RESPONSE_COUNT*/


/* start of register BR_TX_VERIF_COUNT*/

#define      PRV_BR_STAT_MAC100_REG_BR_TX_VERIF_COUNT_TX_VERIF_COUNT_FIELD_OFFSET          0
#define      PRV_BR_STAT_MAC100_REG_BR_TX_VERIF_COUNT_TX_RESP_COUNT_FIELD_OFFSET           16


#define      PRV_BR_STAT_MAC100_REG_BR_TX_VERIF_COUNT_TX_VERIF_COUNT_FIELD_SIZE            8
#define      PRV_BR_STAT_MAC100_REG_BR_TX_VERIF_COUNT_TX_RESP_COUNT_FIELD_SIZE         8

/* end of BR_TX_VERIF_COUNT*/


/*maximal number of queues per port in order to allow preemption*/
#define PRV_DXCH_SIP6_PREEMPTION_MAX_Q_NUM_CNS 8

extern GT_STATUS prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortBaseMapGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 physPort,
    OUT GT_U32 *txqPortBasePtr
);

extern  GT_STATUS prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget
(
    IN GT_U8 devNum,
    IN GT_U32 tileNumber,
    IN GT_U32 dpNum,
    IN GT_U32 localPortNum,
    OUT GT_U32 * indexPtr,
    IN GT_BOOL isErrorForLog
);

/**
 * @internal prvCpssSip6_10TxqUtilsPreemptionSpeedValidGet function
 * @endinternal
 *
 * @brief  Check that required speed is less thne maximal speed supported in preemption mode
 *               for perticular port couple
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 *  @param[in] speedInMb               desired speed (for validation purpose)
 *  @param[in] coupleIndex               index of the couple({express channel,preemptive channel})
*
 *@param[out] preemptionEnabledPtr                            (pointer to )preemption enabled for port
* @param[out] actAsPreemptiveChannelPtr                  (pointer to )port act as preemptive channel
* @param[out] preemptivePhysicalPortNumPtr            (pointer to )physical port number used as preemptive channel
*
 * @retval GT_TRUE                                          -valid speed
 * @retval GT_FALSE                                         - speed is too big and can't be used
*
 *
 */
static GT_BOOL prvCpssSip6_10TxqUtilsPreemptionSpeedValidGet
(
    IN GT_U8                   devNum,
    IN GT_U32                   speedInMb,
    IN GT_U32                  coupleIndex
);

/**
 * @internal prvCpssFalconTxqUtilsPortPreemptionAllowedGet function
 * @endinternal
 *
 * @brief  Check if physical port resources are taken in order to act as preemptive channel
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] portNum                  -physical port number
 * @param[out] actAsPreemptiveChannelPtr            (pointer to )GT_TRUE if physical port resources are taken in order to act
*                                                                                        as preemptive channel,GT_FALSE otherwise
 * @retval GT_OK                    -                on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                              on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 on not applicable device
 *
 */
GT_STATUS  prvCpssDxChTxqSip6PortActAsPreemptiveChannelGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    OUT GT_BOOL              *actAsPreemptiveChannelPtr,
    OUT GT_U32              *expPortNumPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *aNodePtr;
    GT_U32 pNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *    pnodePtr;
    GT_BOOL         isCascade = GT_FALSE;

    CPSS_NULL_PTR_CHECK_MAC(actAsPreemptiveChannelPtr);
    *actAsPreemptiveChannelPtr = GT_FALSE;

    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,physicalPortNum,&isCascade,NULL);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if(GT_FALSE == isCascade)
    {
        /*Find A node*/
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr);
        if (rc!=GT_OK)
        {
            return rc;
        }

        if((GT_TRUE==aNodePtr->preemptionActivated)&&
            (PRV_DXCH_TXQ_SIP6_PREEMTION_A_NODE_TYPE_PRE_E==aNodePtr->preemptionType))
         {
            *actAsPreemptiveChannelPtr = GT_TRUE;

            if(expPortNumPtr)
            {
                rc = prvCpssSip6TxqAnodeToPnodeIndexGet(devNum,aNodePtr->queuesData.tileNum,aNodePtr->partnerIndex,&pNodeIndex);
                if (rc!=GT_OK)
                {
                    return rc;
                }

                PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pnodePtr,devNum,aNodePtr->queuesData.tileNum,pNodeIndex);
                *expPortNumPtr = pnodePtr->aNodelist[0].physicalPort;
            }
         }
    }

    return GT_OK;
}
/**
 * @internal prvCpssDxChTxqSip6GoQIndexGet function
 * @endinternal
 *
 * @brief  Get group of queue number for physical port
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] physicalPortNum        physical port
 * @param[in] readFromHw                   if equal GT_TRUE then thenumber is fetched from EGF table,
 *  otherwise the number is fetched from txQ software data base
 * @param[out] goQPtr                   (pointer to)group of queues number
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6GoQIndexGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    IN GT_BOOL readFromHw,
    OUT GT_U32                              *goQPtr
)
{
    GT_STATUS           rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *aNodePtr;
    GT_BOOL                        isCascade;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr;

    if (GT_FALSE == readFromHw)
    {
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
            *goQPtr = aNodePtr->queuesData.queueGroupIndex;
        }
        else
        {
            /*map to first A node*/
            *goQPtr = pNodePtr->aNodelist[0].queuesData.queueGroupIndex;
        }
    }
    else
    {
        rc = prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortBaseMapGet(devNum, 0 /*unused*/, physicalPortNum, goQPtr);
        if (GT_OK != rc )
        {
            return rc;
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssDxChTxqSip6PreemptionFeatureAllowedGet function
 * @endinternal
 *
 * @brief  Check if preemption feature is enabled for this device/tile
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] enablePtr                   if equal GT_TRUE then preemption feature is enabled per tile ,GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6PreemptionFeatureAllowedGet
(
    IN GT_U8 devNum,
    OUT GT_BOOL   *enablePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    /*GM does not support preemption*/
    if(GT_FALSE == PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
      *enablePtr  = PRV_CPSS_PP_MAC(devNum)->preemptionSupported;
    }
    else
    {
      *enablePtr  = GT_FALSE;
    }
    return GT_OK;
}
/**
 * @internal prvCpssDxChTxqSip6_10RestoreChannel function
 * @endinternal
 *
 * @brief  Restore preemption channel configuration that were configured before preemption channel was used .
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] expPhyPortNum                       physical port that was used as express channel
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10RestoreChannel
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM expPhyPortNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * aNodePtr[2];
    GT_U32 i,maxRate;
    GT_U32  numberOfHandledQueues = 0;
    GT_U32  numberOfMappedQueues = 0 ;


    /*Find A node*/
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, expPhyPortNum, &(aNodePtr[0]));
    if (rc)
    {
        return rc;
    }

    rc = prvCpssDxChTxqSip6PreeptivePartnerAnodeGet(devNum, aNodePtr[0],&(aNodePtr[1]));
    if (rc != GT_OK)
    {
        return rc;
    }

    numberOfMappedQueues = aNodePtr[1]->queuesData.pdqQueueLast - aNodePtr[1]->queuesData.pdqQueueFirst+1;

    /*restore shaping*/
    for(numberOfHandledQueues =0,i=aNodePtr[1]->queuesData.pdqQueueFirst;i<=aNodePtr[1]->queuesData.pdqQueueLast;numberOfHandledQueues++,i++)
    {
        if(numberOfHandledQueues>=PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
        {
            break;
        }
        rc      = prvCpssFalconTxqPdqShapingEnableSet(devNum, aNodePtr[1]->queuesData.tileNum,i, PRV_CPSS_PDQ_LEVEL_Q_E,
            aNodePtr[1]->queuesData.restoreData[i-aNodePtr[1]->queuesData.pdqQueueFirst].eirEnable);
        if (rc != GT_OK)
        {
            return rc;
        }


        maxRate = aNodePtr[1]->queuesData.restoreData[i-aNodePtr[1]->queuesData.pdqQueueFirst].eir;

        rc = prvCpssFalconTxqPdqShapingOnNodeSet(devNum, aNodePtr[1]->queuesData.tileNum,i,PRV_CPSS_PDQ_LEVEL_Q_E,
            aNodePtr[1]->queuesData.restoreData[i-aNodePtr[1]->queuesData.pdqQueueFirst].burstEir,
            &maxRate);

        if (rc != GT_OK)
        {
            return rc;
        }



       maxRate = aNodePtr[1]->queuesData.restoreData[i-aNodePtr[1]->queuesData.pdqQueueFirst].cir;
       rc = prvCpssFalconTxqPdqMinBwOnNodeSet(devNum, aNodePtr[1]->queuesData.tileNum,i,
           aNodePtr[1]->queuesData.restoreData[i-aNodePtr[1]->queuesData.pdqQueueFirst].burstCir,
           &maxRate);

       if (rc != GT_OK)
       {
           return rc;
       }

      rc      = prvCpssFalconTxqPdqMinBwEnableSet(devNum, aNodePtr[1]->queuesData.tileNum,i,
          aNodePtr[1]->queuesData.restoreData[i-aNodePtr[1]->queuesData.pdqQueueFirst].cirEnable);
      if (rc != GT_OK)
      {
          return rc;
      }


    }

    aNodePtr[0]->preemptionActivated = GT_FALSE;
    aNodePtr[1]->preemptionActivated = GT_FALSE;

    /*restore scheduling only if different physical ports*/
    if(aNodePtr[1]->physicalPort!=aNodePtr[0]->physicalPort)
    {
        rc =prvCpssFalconTxqUtilsBindPortToSchedulerProfile(devNum,aNodePtr[1]->physicalPort,aNodePtr[1]->restoreSchedProfile,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /*restore Rx Pfc*/
    rc =prvCpssFalconTxqSdqPfcControlEnableBitmapSet(devNum,aNodePtr[1]->queuesData.tileNum,aNodePtr[1]->queuesData.dp,
      aNodePtr[1]->queuesData.localPort,aNodePtr[1]->queuesData.restorePfcBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*restore  TX pause TC*/
    rc = prvCpssSip6TxqSdqPausedTcBmpSet(devNum,aNodePtr[1]->queuesData.tileNum,aNodePtr[1]->queuesData.dp,
      aNodePtr[1]->queuesData.localPort,aNodePtr[1]->queuesData.restorePauseTc);
      if (rc != GT_OK)
    {
        return rc;
    }

    /*restore  QCN*/
    rc =prvCpssSip6TxqSdqLocalPortQcnFactorSet(devNum,aNodePtr[1]->queuesData.tileNum,aNodePtr[1]->queuesData.dp,
         aNodePtr[1]->queuesData.queueBase,aNodePtr[1]->queuesData.queueBase+numberOfMappedQueues-1,aNodePtr[1]->queuesData.restoreQcnFactor);
    if (rc != GT_OK)
    {
       return rc;
    }

    return rc;
}
/**
 * @internal prvCpssDxChTxqSip6_10CopyChannel function
 * @endinternal
 *
 * @brief  Copy configuration of express channel to preemptive channel .
 *
 * @note   APPLICABLE DEVICES:      AC5P;
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;  Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] expPhyPortNum                       physical port that  used as express channel
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10CopyChannel
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM expPhyPortNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * aNodePtr[2];
    GT_U32 i,j;
    GT_U32 maxRate;
    GT_U16 burstSizeFromHw;
    GT_BOOL txLibInitDone = GT_FALSE;
    GT_U32  numberOfHandledQueues = 0;


    /*Find A node*/
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, expPhyPortNum, &(aNodePtr[0]));
    if (rc)
    {
        return rc;
    }

    rc = prvCpssDxChTxqSip6PreeptivePartnerAnodeGet(devNum, aNodePtr[0],&(aNodePtr[1]));
    if (rc != GT_OK)
    {
        return rc;
    }


    /*save shaping*/
    for(j=0;j<2;j++)
    {
        for(numberOfHandledQueues = 0,i=aNodePtr[j]->queuesData.pdqQueueFirst;i<=aNodePtr[j]->queuesData.pdqQueueLast;i++,numberOfHandledQueues++)
        {
             if(numberOfHandledQueues>=PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
            {
                break;
            }
            /*check if need to read from HW*/
            if(j==0&&
                aNodePtr[j]->queuesData.restoreData[i-aNodePtr[j]->queuesData.pdqQueueFirst].eir!=CPSS_SIP6_TXQ_INVAL_DATA_CNS)
           {
                continue;
           }
            rc      = prvCpssFalconTxqPdqShapingEnableGet(devNum, aNodePtr[j]->queuesData.tileNum,i, PRV_CPSS_PDQ_LEVEL_Q_E,
                &(aNodePtr[j]->queuesData.restoreData[i-aNodePtr[j]->queuesData.pdqQueueFirst].eirEnable));
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = prvCpssSip6TxqPdqShapingRateFromHwGet(devNum, aNodePtr[j]->queuesData.tileNum, i, PRV_CPSS_PDQ_LEVEL_Q_E,
                               PRV_QUEUE_SHAPING_ACTION_STOP_TRANSMITION_ENT, &burstSizeFromHw, &maxRate);
            if (rc != GT_OK)
            {
                return rc;
            }

            aNodePtr[j]->queuesData.restoreData[i-aNodePtr[j]->queuesData.pdqQueueFirst].burstEir = burstSizeFromHw;
            aNodePtr[j]->queuesData.restoreData[i-aNodePtr[j]->queuesData.pdqQueueFirst].eir = maxRate;
        }
    }

    /*copy  shaping*/
    for(numberOfHandledQueues = 0,i=aNodePtr[1]->queuesData.pdqQueueFirst;i<=aNodePtr[1]->queuesData.pdqQueueLast;i++,numberOfHandledQueues++)
    {
        if(numberOfHandledQueues>=PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
        {
            break;
        }

        rc      = prvCpssFalconTxqPdqShapingEnableSet(devNum, aNodePtr[1]->queuesData.tileNum,i, PRV_CPSS_PDQ_LEVEL_Q_E,
            aNodePtr[0]->queuesData.restoreData[i-aNodePtr[1]->queuesData.pdqQueueFirst].eirEnable);
        if (rc != GT_OK)
        {
            return rc;
        }

        maxRate = aNodePtr[0]->queuesData.restoreData[i-aNodePtr[1]->queuesData.pdqQueueFirst].eir;

        rc = prvCpssFalconTxqPdqShapingOnNodeSet(devNum, aNodePtr[1]->queuesData.tileNum,i,PRV_CPSS_PDQ_LEVEL_Q_E,
            aNodePtr[0]->queuesData.restoreData[i-aNodePtr[1]->queuesData.pdqQueueFirst].burstEir,
            &maxRate);

        if (rc != GT_OK)
        {
            return rc;
        }

    }

    /*save  minimal bandwith*/
    for(j=0;j<2;j++)
    {
      for(numberOfHandledQueues =0,i=aNodePtr[j]->queuesData.pdqQueueFirst;i<=aNodePtr[j]->queuesData.pdqQueueLast;i++,numberOfHandledQueues++)
      {
          if(numberOfHandledQueues>=PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
          {
              break;
          }
          /*check if need to read from HW*/
          if(j==0&&
                aNodePtr[j]->queuesData.restoreData[i-aNodePtr[j]->queuesData.pdqQueueFirst].cir!=CPSS_SIP6_TXQ_INVAL_DATA_CNS)
          {
               continue;
          }
          rc      = prvCpssFalconTxqPdqMinBwEnableGet(devNum, aNodePtr[j]->queuesData.tileNum,i,
              &(aNodePtr[j]->queuesData.restoreData[i-aNodePtr[j]->queuesData.pdqQueueFirst].cirEnable));
          if (rc != GT_OK)
          {
              return rc;
          }

          rc = prvCpssSip6TxqPdqShapingRateFromHwGet(devNum, aNodePtr[j]->queuesData.tileNum, i, PRV_CPSS_PDQ_LEVEL_Q_E,
                             PRV_QUEUE_SHAPING_ACTION_PRIORITY_DOWNGRADE_ENT, &burstSizeFromHw, &maxRate);
          if (rc != GT_OK)
          {
              return rc;
          }

          aNodePtr[j]->queuesData.restoreData[i-aNodePtr[j]->queuesData.pdqQueueFirst].burstCir = burstSizeFromHw;
          aNodePtr[j]->queuesData.restoreData[i-aNodePtr[j]->queuesData.pdqQueueFirst].cir = maxRate;

      }
     }

    /*copy  minimal bandwith*/
    for(numberOfHandledQueues =0,i=aNodePtr[1]->queuesData.pdqQueueFirst;i<=aNodePtr[1]->queuesData.pdqQueueLast;i++,numberOfHandledQueues++)
    {
        if(numberOfHandledQueues>=PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
        {
            break;
        }

        rc      = prvCpssFalconTxqPdqMinBwEnableSet(devNum, aNodePtr[1]->queuesData.tileNum,i,
            aNodePtr[0]->queuesData.restoreData[i-aNodePtr[1]->queuesData.pdqQueueFirst].cirEnable);
        if (rc != GT_OK)
        {
            return rc;
        }

        maxRate = aNodePtr[0]->queuesData.restoreData[i-aNodePtr[1]->queuesData.pdqQueueFirst].cir;

        rc = prvCpssFalconTxqPdqMinBwOnNodeSet(devNum, aNodePtr[1]->queuesData.tileNum,i,
            aNodePtr[0]->queuesData.restoreData[i-aNodePtr[1]->queuesData.pdqQueueFirst].burstCir,
            &maxRate);

        if (rc != GT_OK)
        {
            return rc;
        }

    }

    rc = prvCpssSip6TxqLibInitDoneGet(devNum,&txLibInitDone);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*maybe a case when scheduling is not configured yet*/
    if(GT_TRUE==txLibInitDone)
    {
       /*save scheduling*/
        if(aNodePtr[0]->physicalPort!=aNodePtr[1]->physicalPort)
        {
           /*save profile only if preemptive and express has different physical port numbers.
                        Otherwise we do not restore preemptive channel*/
          rc = prvCpssFalconTxqUtilsPortSchedulerProfileGet(devNum,aNodePtr[1]->physicalPort,&aNodePtr[1]->restoreSchedProfile);
          if (rc != GT_OK)
          {
              return rc;
          }
        }

        /*save EXP parameters in raw format*/
        for(numberOfHandledQueues = 0,i=aNodePtr[0]->queuesData.pdqQueueFirst;i<=aNodePtr[0]->queuesData.pdqQueueLast;i++,numberOfHandledQueues++)
        {
               if(numberOfHandledQueues>=PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
              {
                  break;
              }

              rc      = prvCpssSip6TxqPdqQSchedParamsGet(devNum, aNodePtr[0]->queuesData.tileNum,i,
                  &(aNodePtr[0]->queuesData.restoreData[i-aNodePtr[0]->queuesData.pdqQueueFirst].eligFunc),
                  &(aNodePtr[0]->queuesData.restoreData[i-aNodePtr[0]->queuesData.pdqQueueFirst].quantum));
              if (rc != GT_OK)
              {
                  return rc;
              }
         }



        /*copy scheduling*/
       for(numberOfHandledQueues =0,i=aNodePtr[1]->queuesData.pdqQueueFirst;i<=aNodePtr[1]->queuesData.pdqQueueLast;i++,numberOfHandledQueues++)
       {
           if(numberOfHandledQueues>=PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
           {
               break;
           }

           rc      = prvCpssSip6TxqPdqQSchedParamsSet(devNum, aNodePtr[1]->queuesData.tileNum,i,
                  (aNodePtr[0]->queuesData.restoreData[i-aNodePtr[1]->queuesData.pdqQueueFirst].eligFunc),
                  (aNodePtr[0]->queuesData.restoreData[i-aNodePtr[1]->queuesData.pdqQueueFirst].quantum));
              if (rc != GT_OK)
              {
                  return rc;
              }

       }

        /*save Rx Pfc*/
        rc =prvCpssFalconTxqSdqPfcControlEnableBitmapGet(devNum,aNodePtr[1]->queuesData.tileNum,aNodePtr[1]->queuesData.dp,
          aNodePtr[1]->queuesData.localPort,&aNodePtr[1]->queuesData.restorePfcBmp);
        if (rc != GT_OK)
        {
            return rc;
        }

         /*copy  Rx Pfc*/

        rc =prvCpssFalconTxqSdqPfcControlEnableBitmapGet(devNum,aNodePtr[0]->queuesData.tileNum,aNodePtr[0]->queuesData.dp,
          aNodePtr[0]->queuesData.localPort,&aNodePtr[0]->queuesData.restorePfcBmp);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc =prvCpssFalconTxqSdqPfcControlEnableBitmapSet(devNum,aNodePtr[1]->queuesData.tileNum,aNodePtr[1]->queuesData.dp,
          aNodePtr[1]->queuesData.localPort,aNodePtr[0]->queuesData.restorePfcBmp);
        if (rc != GT_OK)
        {
            return rc;
        }

        /*save Tx Pause*/

        rc =prvCpssSip6TxqSdqPausedTcBmpGet(devNum,aNodePtr[1]->queuesData.tileNum,aNodePtr[1]->queuesData.dp,
          aNodePtr[1]->queuesData.localPort,&aNodePtr[1]->queuesData.restorePauseTc);
        if (rc != GT_OK)
        {
            return rc;
        }

         /*copy Tx Pause*/

         rc =prvCpssSip6TxqSdqPausedTcBmpGet(devNum,aNodePtr[0]->queuesData.tileNum,aNodePtr[0]->queuesData.dp,
               aNodePtr[0]->queuesData.localPort,&aNodePtr[0]->queuesData.restorePauseTc);
         if (rc != GT_OK)
         {
             return rc;
         }

         rc =prvCpssSip6TxqSdqPausedTcBmpSet(devNum,aNodePtr[1]->queuesData.tileNum,aNodePtr[1]->queuesData.dp,
          aNodePtr[1]->queuesData.localPort,aNodePtr[0]->queuesData.restorePauseTc);
         if (rc != GT_OK)
         {
            return rc;
         }


        /*save QCN*/

        rc =prvCpssSip6TxqSdqLocalPortQcnFactorGet(devNum,aNodePtr[1]->queuesData.tileNum,aNodePtr[1]->queuesData.dp,
                      aNodePtr[1]->queuesData.queueBase,&aNodePtr[1]->queuesData.restoreQcnFactor);
        if (rc != GT_OK)
        {
            return rc;
        }

         /*copy QCN will be done upon calling prvCpssDxChFalconPortQueueRateSet*/

     }

     aNodePtr[0]->preemptionActivated = GT_TRUE;
     aNodePtr[1]->preemptionActivated = GT_TRUE;

    return rc;
}


/**
 * @internal prvCpssDxChTxqSip6_10PreemptionEnableSet function
 * @endinternal
 *
 * @brief  Set enable/disable preemption at PDX glue for specific physical port.
 *  Function convert physical port to group of queues and then update PDX table
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] physicalPortNum        physical port
 * @param[in] enablePtr                   if equal GT_TRUE then preemption is enabled in PDX,GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10PreemptionEnableSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    IN GT_BOOL enable
)
{
    GT_STATUS   rc;
    GT_U32      goQ;
    GT_BOOL     preModeAllowed, shapingEnable = GT_FALSE,currentState;
    GT_U32      currentSpeedInMb =0;

    PRV_CPSS_DXCH_CURRENT_PORT_SPEED_IN_MB_MAC(devNum, physicalPortNum,currentSpeedInMb);

    if(currentSpeedInMb>0)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Port %d should be set to no speed .Current speed is %d Mb", physicalPortNum,currentSpeedInMb);
    }

    if(enable == GT_TRUE)
    {
        /*Check that shaping is disabled on this port*/
        rc = prvCpssSip6TxqUtilsShapingEnableGet(devNum, physicalPortNum, 0, GT_FALSE, &shapingEnable);
        if (rc != GT_OK)
        {
            return rc;
        }
        /*Shaping at port level should be disabled*/
        if (GT_TRUE == shapingEnable)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Port level shaping is enabled on port %d ", physicalPortNum);
        }
    }

    rc = prvCpssFalconTxqUtilsPortPreemptionAllowedGet(devNum, physicalPortNum, &preModeAllowed);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (GT_TRUE != preModeAllowed)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Port can not work in preemption mode %d ", physicalPortNum);
    }



    rc = prvCpssDxChTxqSip6_10PreemptionEnableGet(devNum, physicalPortNum,
            currentSpeedInMb,&currentState);
    if (rc != GT_OK)
    {
        return rc;
    }
    if(currentState!=enable)
    {
        /*get GoQ*/
        rc = prvCpssDxChTxqSip6GoQIndexGet(devNum, physicalPortNum, GT_FALSE, &goQ);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChTxqSip6_10PdxPreemptionEnableSet(devNum, goQ, enable);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(enable == GT_FALSE)
        {
           /*restore   preemptive channel*/
           rc = prvCpssDxChTxqSip6_10RestoreChannel(devNum, physicalPortNum);
        }
        else
        {
            /*copy to  preemptive channel*/
            rc = prvCpssDxChTxqSip6_10CopyChannel(devNum, physicalPortNum);
        }
    }

    return rc;
}
/**
 * @internal prvCpssDxChTxqSip6_10PreemptionEnableGet function
 * @endinternal
 *
 * @brief  Get enable/disable preemption at PDX glue for specific physical port
 *  Function convert physical port to group of queues and then querry PDX table
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] physicalPortNum        physical port
*  @param[in] speedInMb                            desired speed (in Mb)
 * @param[out] enablePtr                   (pointer to)if equal GT_TRUE then preemption is enabled in PDX,GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10PreemptionEnableGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    IN GT_U32               speedInMb,
    OUT GT_BOOL             *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      goQ;
    GT_BOOL     preemptionFeatureAllowed;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /*not used yet*/
    speedInMb=speedInMb;

    *enablePtr = GT_FALSE;

    rc = prvCpssDxChTxqSip6PreemptionFeatureAllowedGet(devNum, &preemptionFeatureAllowed);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(GT_TRUE == preemptionFeatureAllowed)
    {
        /*get GoQ*/
        rc = prvCpssDxChTxqSip6GoQIndexGet(devNum, physicalPortNum, GT_FALSE, &goQ);
        if (rc != GT_OK)
        {
            return rc;
        }
        return prvCpssDxChTxqSip6_10PdxPreemptionEnableGet(devNum, goQ, enablePtr);
    }

    return GT_OK;
}
/**
 * @internal prvCpssDxChTxqSip6_10PreChannelGet function
 * @endinternal
 *
 * @brief  Get preemptive channel for express channel.
 *
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 *  @param[in] requiredSpeedInMb                            desired speed (for validation purpose ,0 if validation is not required)
 * @param[in] expPort                            express local port in data path
 * @param[out] prePortPtr                        (pointer to)preemption channel number in data path
 *
 * @retval GT_OK                                  - on success.
 * @retval GT_BAD_PARAM                           - wrong pdx number.
 * @retval GT_HW_ERROR                            -on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE               - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10PreChannelGet
(
    IN GT_U8                devNum,
    IN GT_U32               requiredSpeedInMb,
    IN GT_U32               expPort,
    OUT GT_U32              *prePortPtr
)
{
    const GT_U32    couples[] =
    {
        TXQ_ALL_DP_PREEMPTIVE_COUPLES_MAC
    };

    GT_U32      i,startIndex,size;
    CPSS_NULL_PTR_CHECK_MAC(prePortPtr);

    *prePortPtr = CPSS_SIP6_TXQ_INVAL_DATA_CNS;

    if (TXQ_IS_PREEMPTIVE_DEVICE(devNum))
    {
        if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
        {
         startIndex = TXQ_IRONMAN_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC;
         size = TXQ_IRONMAN_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC;
        }
        else if(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum) == GT_TRUE)
        {
         startIndex = TXQ_HARRRIER_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC;
         size = TXQ_HARRRIER_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC;
        }
        else
        {
         startIndex = TXQ_AC5P_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC;
         size = TXQ_AC5P_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC;
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Device %d does not support preemption couples",devNum);
    }

    for (i = startIndex; i < startIndex+size; i++)
    {
        /*looking for preemptive*/
        if ( couples[i * 2] == expPort)
        {
            /*check if speed is acceptable*/
            if (GT_TRUE == prvCpssSip6_10TxqUtilsPreemptionSpeedValidGet(
                devNum,requiredSpeedInMb, (i - startIndex)))
            {
                *prePortPtr = couples[i * 2 + 1];
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Speed %d cannot be set on channel %d",requiredSpeedInMb,i);
            }
        }
    }

    return GT_OK;
}

/**
 * @internal prvCpssDxChTxqSip6_10ExpChannelGet function
 * @endinternal
 *
 * @brief  Get express channel for preemptive channel.
 *
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
*  @param[in] devNum                   -PP's device number.
 * @param[in] speedInMb                            desired speed (for validation purpose ,0 if validation is not required)
 * @param[in] expPort                            express local port in data path
 * @param[out] prePortPtr                        (pointer to)preemption channel number in data path
 *
 * @retval GT_OK                                    -   on success.
 * @retval GT_BAD_PARAM                             -  wrong pdx number.
 * @retval GT_HW_ERROR                              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10ExpChannelGet
(
    IN GT_U8                devNum,
    IN GT_U32               speedInMb,
    IN GT_U32               prePort,
    OUT GT_U32              *expPortPtr
)
{
    const GT_U32    couples[] =
    {
        TXQ_ALL_DP_PREEMPTIVE_COUPLES_MAC
    };
    GT_U32      i,startIndex,size;
    CPSS_NULL_PTR_CHECK_MAC(expPortPtr);

      /*not in use yet*/
    speedInMb = speedInMb;

    *expPortPtr = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
    if (TXQ_IS_PREEMPTIVE_DEVICE(devNum))
    {
        if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
        {
         startIndex = TXQ_IRONMAN_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC;
         size = TXQ_IRONMAN_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC;
        }
        else if(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum) == GT_TRUE)
        {
         startIndex = TXQ_HARRRIER_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC;
         size = TXQ_HARRRIER_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC;
        }
        else
        {
         startIndex = TXQ_AC5P_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC;
         size = TXQ_AC5P_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC;
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Device %d does not support preemption couples",devNum);
    }

    for (i = startIndex; i < startIndex+size; i++)
    {
        /*looking for preemptive*/
        if ( couples[i * 2 + 1] == prePort)
        {
            *expPortPtr = couples[i * 2];
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssDxChTxqSip6PreeptivePartnerMappedGet function
 * @endinternal
 *
 * @brief  Get preemptive channel for express channel
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] expPort                     express local port in data path
 * @param[in] dp                               data path index
 * @param[out] prePortMappedPtr    (pointer to)if equal GT_TRUE then preemption channel is mapped at port mapping,
                                                                                  GT_FALSE otherwise
 * @param[out] prePortPtr                   (pointer to)preemption channel number in data path
 *
 * @retval GT_OK                    -                           on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                                 on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE      on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6PreeptivePartnerMappedGet
(
    IN GT_U8 devNum,
    IN GT_U32 expPort,
    IN GT_U32 dp,
    OUT GT_BOOL                             *prePortMappedPtr,
    OUT GT_U32                              *prePortPtr
)
{
    GT_STATUS       rc;
    GT_BOOL         preemptionAllowed;
    GT_PHYSICAL_PORT_NUM    phyPortNum;
    CPSS_NULL_PTR_CHECK_MAC(prePortMappedPtr);
    CPSS_NULL_PTR_CHECK_MAC(prePortPtr);
    *prePortMappedPtr   = GT_FALSE;
    rc          = prvCpssDxChTxqSip6PreemptionFeatureAllowedGet(devNum, &preemptionAllowed);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (GT_TRUE == preemptionAllowed)
    {
        rc = prvCpssDxChTxqSip6_10PreChannelGet(devNum,0,expPort, prePortPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (CPSS_SIP6_TXQ_INVAL_DATA_CNS == *prePortPtr)
        {
            return GT_OK;
        }
        rc = prvCpssFalconTxqUtilsPhysicalPortNumberGet(devNum, 0,
                                dp, *prePortPtr, &phyPortNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (phyPortNum != CPSS_SIP6_TXQ_INVAL_DATA_CNS)
        {
            *prePortMappedPtr = GT_TRUE;
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssDxChTxqSip6ExpressPartnerGoQGet function
 * @endinternal
 *
 * @brief  Get express channel  group of queues for preemptive channel
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] prePort                  - express local port in data path
 * @param[in] dp                       - data path index
 * @param[out] expPortGoQPtr            (pointer togroup of queues index of the express channel
 *
 * @retval GT_OK                    -                on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                              on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 on not applicable device
 *
 */
GT_STATUS  prvCpssDxChTxqSip6ExpressPartnerGoQGet
(
    IN GT_U8 devNum,
    IN GT_U32 prePort,
    IN GT_U32 dp,
    OUT GT_U32                              *expPortGoQPtr
)
{
    GT_STATUS               rc;
    GT_U32                  expPort, pNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pNodePtr = NULL;

    CPSS_NULL_PTR_CHECK_MAC(expPortGoQPtr);
    *expPortGoQPtr  = CPSS_SIP6_TXQ_INVAL_DATA_CNS;

    rc = prvCpssDxChTxqSip6_10ExpChannelGet(devNum,0,prePort, &expPort);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (CPSS_SIP6_TXQ_INVAL_DATA_CNS == expPort)
    {
        *expPortGoQPtr = CPSS_SIP6_TXQ_INVAL_DATA_CNS;
        return GT_OK;
    }
    rc = prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget(devNum, 0,
                                   dp, expPort, &pNodeIndex, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (pNodeIndex != CPSS_SIP6_TXQ_INVAL_DATA_CNS)
    {
        /*should be only one A node mapped*/

        PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pNodePtr,devNum,0,pNodeIndex);

        if (pNodePtr->aNodeListSize == 1)
        {
            *expPortGoQPtr =  pNodePtr->aNodelist[0].queuesData.queueGroupIndex;
        }
    }

    return GT_OK;
}

/**
 * @internal prvCpssDxChTxqSip6_10LocalPortTypeGet function
 * @endinternal
 *
 * @brief  Get info regarding local port preemption ability
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 *  @param[in] devNum                   -PP's device number.
 * @param[in] speedInMb                            desired speed (for validation purpose ,0 if validation is not required)
 * @param[in] localPort                  -  local port in data path
 * @param[out] portTypePtr            (pointer to)port preemption tyoe
 *
 * @retval GT_OK                    -                on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                              on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10LocalPortTypeGet
(
    IN GT_U8                devNum,
    IN GT_U32               speedInMb,
    IN GT_U32 localPort,
    OUT PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_ENT *portTypePtr
)
{
    GT_U32      i,startIndex,size;
    const GT_U32    couples[] =
    {
        TXQ_ALL_DP_PREEMPTIVE_COUPLES_MAC
    };
    CPSS_NULL_PTR_CHECK_MAC(portTypePtr);
    *portTypePtr = PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E;

      /*not in use yet*/
    speedInMb = speedInMb;
    if (TXQ_IS_PREEMPTIVE_DEVICE(devNum))
    {
        if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
        {
         startIndex = TXQ_IRONMAN_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC;
         size = TXQ_IRONMAN_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC;
        }
        else if(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum) == GT_TRUE)
        {
         startIndex = TXQ_HARRRIER_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC;
         size = TXQ_HARRRIER_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC;
        }
        else
        {
         startIndex = TXQ_AC5P_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC;
         size = TXQ_AC5P_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC;
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Device %d does not support preemption couples",devNum);
    }

    for (i = startIndex; i < startIndex+size; i++)
    {
        if ( couples[i * 2] == localPort)
        {
            *portTypePtr = PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E;
            break;
        }
        if ( couples[i * 2 + 1] == localPort)
        {
            *portTypePtr = PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_PRE_E;
            break;
        }
    }
    return GT_OK;
}

/**
 * @internal prvCpssDxChTxqSip6_10LocalPortActualTypeGet function
 * @endinternal
 *
 * @brief  Get info regarding local port preemption current configuration
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in]  devNum                - PP's device number.
 * @param[in]  dpIndex               - data path index
 * @param[in]  localDmaNum           - local port in data path
 * @param[in]  speedInMb             - port speed in megabits/sec
 * @param[out] portTypePtr           - (pointer to)port preemption type
 *
 * @retval GT_OK                      on success.
 * @retval GT_BAD_PARAM               wrong PARAMETER VALUE.
 * @retval GT_HW_ERROR                on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE   on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10LocalPortActualTypeGet
(
    IN  GT_U8                                     devNum,
    IN  GT_U32                                    dpIndex,
    IN  GT_U32                                    localDmaNum,
    IN  GT_U32                                    speedInMb,
    OUT PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_ENT *portTypePtr
)
{
    GT_STATUS             rc;
    GT_U32                globalDmaNum;
    GT_U32                preemptiveLocalDmaNum;
    GT_U32                expressLocalDmaNum;
    GT_PHYSICAL_PORT_NUM  physicalPortArr[10];
    GT_U32                physicalPortsFound;
    GT_BOOL               enable;

    rc = prvCpssDxChTxqSip6_10PreChannelGet(
        devNum, speedInMb, localDmaNum/*expPort*/, &preemptiveLocalDmaNum);
    if (rc != GT_OK)
    {
        preemptiveLocalDmaNum = 0xFFFFFFFF; /* port cannot have preeptive (i.e. be express)*/
    }

    rc =  prvCpssDxChTxqSip6_10ExpChannelGet(
        devNum,speedInMb, localDmaNum/*prePort*/, &expressLocalDmaNum);
    if (rc != GT_OK)
    {
        expressLocalDmaNum = 0xFFFFFFFF; /* port cannot have express (i.e. be preeptive)*/
    }

    if ((preemptiveLocalDmaNum == 0xFFFFFFFF) && (expressLocalDmaNum == 0xFFFFFFFF))
    {
        /* port cannot be both express and preemptive */
        *portTypePtr = PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E;
        return GT_OK;
    }

    rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
        devNum, dpIndex, localDmaNum, &globalDmaNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (preemptiveLocalDmaNum != 0xFFFFFFFF)
    {
        /* check port to be express */
        rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
            devNum, dpIndex, localDmaNum, &globalDmaNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortPhysicalPortMapReverseDmaMappingGet(
            devNum, globalDmaNum, &physicalPortsFound, physicalPortArr);
        if (rc != GT_OK)
        {
            /* physical port not found */
            /* DMA channel not seen to application, can be preemptive only */
            /* but preemptive port cannot have pree mpive pair             */
            /* this case should not occure                                 */
            *portTypePtr = PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E;
            return GT_OK;
        }

        rc =  prvCpssDxChTxqSip6_10PreemptionEnableGet(
            devNum,  physicalPortArr[0]/*physicalPortNum*/, speedInMb, &enable);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (enable != GT_FALSE)
        {
            *portTypePtr = PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E;
            return GT_OK;
        }
    }

    if (expressLocalDmaNum != 0xFFFFFFFF)
    {
        /* check port to be preemptive */
        rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
            devNum, dpIndex, expressLocalDmaNum, &globalDmaNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortPhysicalPortMapReverseDmaMappingGet(
            devNum, globalDmaNum, &physicalPortsFound, physicalPortArr);
        if (rc != GT_OK)
        {
            /* physical port not found - should never occur for express port */
            /* there is no port to be preemptive for it                      */
            *portTypePtr = PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E;
            return GT_OK;
        }
        rc =  prvCpssDxChTxqSip6_10PreemptionEnableGet(
            devNum,  physicalPortArr[0]/*physicalPortNum*/, speedInMb, &enable);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (enable != GT_FALSE)
        {
            *portTypePtr = PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_PRE_E;
            return GT_OK;
        }
    }

    *portTypePtr = PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E;
    return GT_OK;
}

/**
 * @internal prvCpssDxChTxqSip6LocalPortFreeGoQGet function
 * @endinternal
 *
 * @brief  Get free group of queues index for current local port.
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] prePort                  - express local port in data path
 * @param[in] dp                       -      data path index
 * @param[out] expPortGoQPtr            (pointer to group of queues index of the express channel
 * @retval GT_OK                    -                on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                              on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6LocalPortFreeGoQGet
(
    IN GT_U8 devNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE      *aNodePtr,
    IN GT_U32 dp,
    IN GT_U32 localPort,
    INOUT GT_U32                              *qroupOfQIteratorPtr
)
{
    GT_STATUS                   rc;
    GT_BOOL                     preemptionAllowed;
    PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_ENT   portType;
    GT_U32                      expGoQ;
    CPSS_NULL_PTR_CHECK_MAC(aNodePtr);
    CPSS_NULL_PTR_CHECK_MAC(qroupOfQIteratorPtr);
    if ( aNodePtr->queuesData.queueGroupIndex != CPSS_SIP6_TXQ_INVAL_DATA_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Port %d already has group of queue index %d", localPort, aNodePtr->queuesData.queueGroupIndex);
    }
    rc = prvCpssDxChTxqSip6PreemptionFeatureAllowedGet(devNum,  &preemptionAllowed);
    if (rc != GT_OK)
    {
        return rc;
    }
    /*GM does not support preemption,but still we would like same mapping*/
    if((GT_FALSE == preemptionAllowed)&&(GT_TRUE== PRV_CPSS_PP_MAC(devNum)->isGmDevice))
    {
        if(TXQ_IS_PREEMPTIVE_DEVICE(devNum))
        {
            preemptionAllowed = PRV_CPSS_SIP_6_10_DEFAULT_PREEMPTION_ALLOWED_MAC;
        }
    }

    /*no preemption for remote ports*/
    if(aNodePtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        preemptionAllowed = GT_FALSE;
    }

    if(GT_FALSE == preemptionAllowed)
    { /*one to one mapping*/
        aNodePtr->queuesData.queueGroupIndex = aNodePtr->physicalPort;
    }
    else
    {
        rc = prvCpssDxChTxqSip6_10LocalPortTypeGet(devNum,0,localPort, &portType);
        if (rc != GT_OK)
        {
            return rc;
        }
        switch (portType)
        {
        case PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E:
            /*should be even numbers*/
            if ((*qroupOfQIteratorPtr) % 2)
            {
                (*qroupOfQIteratorPtr)++;
            }
            aNodePtr->queuesData.queueGroupIndex    = (*qroupOfQIteratorPtr);
            (*qroupOfQIteratorPtr)          += 2;/*reserve one for preemptive channel*/
            break;
        case PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_PRE_E:
            rc = prvCpssDxChTxqSip6ExpressPartnerGoQGet(devNum, localPort, dp, &expGoQ);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (expGoQ != CPSS_SIP6_TXQ_INVAL_DATA_CNS)
            {
                if (expGoQ >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdxNumQueueGroups)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Index of exp channel invalid %d", expGoQ);
                }
                else
                {
                    aNodePtr->queuesData.queueGroupIndex = expGoQ + 1;
                }
            }
            break;
        default:
            aNodePtr->queuesData.queueGroupIndex = (*qroupOfQIteratorPtr)++;
            break;
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsPortPreemptionAllowedGet function
 * @endinternal
 *
 * @brief  Check if preemption can be enabled on specific port
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] portNum                  -physical port number
 * @param[out] preemptionAllowedPtr            (pointer to )preemption can be activated boolean
 * @retval GT_OK                    -                on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                              on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsPortPreemptionAllowedGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL                            *preemptionAllowedPtr
)
{
    GT_STATUS                   rc;
    GT_BOOL                     isCascadePort;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *        aNodePtr;
    PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_ENT   portPreemptionType;
    GT_U32                      numberOfMappedQueues;
    GT_BOOL                     featureAllowed;

    CPSS_NULL_PTR_CHECK_MAC(preemptionAllowedPtr);
    *preemptionAllowedPtr   = GT_FALSE;

    rc = prvCpssDxChTxqSip6PreemptionFeatureAllowedGet(devNum,&featureAllowed);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*we are done,no further check needed*/
    if(GT_FALSE == featureAllowed)
    {
        return GT_OK;
    }

    rc          = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum, &isCascadePort, NULL);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (GT_FALSE == isCascadePort)
    {
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &aNodePtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber failed  ");
        }

        numberOfMappedQueues = aNodePtr->queuesData.pdqQueueLast -aNodePtr->queuesData.pdqQueueFirst+1;

        /*check that port has maximum 8 queues*/
        if(numberOfMappedQueues>PRV_DXCH_SIP6_PREEMPTION_MAX_Q_NUM_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Not valid number of queues %d should be smaller/equal to %d\n",
                numberOfMappedQueues,PRV_DXCH_SIP6_PREEMPTION_MAX_Q_NUM_CNS);
        }

        if (aNodePtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
        {
            rc = prvCpssDxChTxqSip6_10LocalPortTypeGet(devNum,0,aNodePtr->queuesData.localPort, &portPreemptionType);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqSip6_10LocalPortTypeGet failed  ");
            }
            if (portPreemptionType == PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E)
            {
                /*now check if preemptive partner mapped*/
               if(aNodePtr->partnerIndex!=CPSS_SIP6_TXQ_INVAL_DATA_CNS)
               {
                *preemptionAllowedPtr   = GT_TRUE;
               }
            }
        }
    }
    return GT_OK;
}

/**
 * @internal prvCpssDxChTxqSip6PreeptivePartnerAnodeGet function
 * @endinternal
 *
 * @brief  Get A node that act as preemptive channel for A node that act as epress channel
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] expANodePtr                  -(pointer to )A node that act as epress channel
 * @param[out] preANodePtr                  (pointer to )pointer to A node that act as preemptive channel
 * @retval GT_OK                    -                on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                              on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6PreeptivePartnerAnodeGet
(
    IN GT_U8                            devNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE   *expANodePtr,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE  **preANodePtr
)
{
    GT_U32    pNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pNodePtr = NULL;
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(expANodePtr);
    CPSS_NULL_PTR_CHECK_MAC(preANodePtr);

    *preANodePtr = NULL;

    /*check  validity*/
     if(expANodePtr->preemptionType!=PRV_DXCH_TXQ_SIP6_PREEMTION_A_NODE_TYPE_EXP_E)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Anode %d is not express ",expANodePtr->aNodeIndex);
     }

     rc  = prvCpssSip6TxqAnodeToPnodeIndexGet(devNum,expANodePtr->queuesData.tileNum,expANodePtr->partnerIndex,&pNodeIndex);
     if (rc != GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqAnodeToPnodeIndexGet failed for index %d",expANodePtr->partnerIndex);
     }

     if(pNodeIndex==CPSS_SIP6_TXQ_INVAL_DATA_CNS)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "P node is not found");
     }

     PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pNodePtr,devNum,expANodePtr->queuesData.tileNum,pNodeIndex);

     /*sanity check*/
     if(pNodePtr->aNodeListSize!=1)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "pChannel contain more then 1 port");
     }

      *preANodePtr = &(pNodePtr->aNodelist[0]);

     /*sanity check*/
     if((*preANodePtr)->preemptionType!=PRV_DXCH_TXQ_SIP6_PREEMTION_A_NODE_TYPE_PRE_E)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Preemptive Anode type is incorrect");
     }


    return GT_OK;

}

/**
 * @internal prvCpssSip6TxqUtilsPreemptionStatusGet function
 * @endinternal
 *
 * @brief  Get preemption configuration status
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] physicalPortNum        physical port
*
 *@param[out] preemptionEnabledPtr                            (pointer to )preemption enabled for port
* @param[out] actAsPreemptiveChannelPtr                  (pointer to )port act as preemptive channel
* @param[out] preemptivePhysicalPortNumPtr            (pointer to )physical port number used as preemptive channel
*
 * @retval GT_OK                    -                on success.
 * @retval GT_BAD_PARAM                             - wrong pdx number.
 * @retval GT_HW_ERROR                              on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE                 on not applicable device
 *
 *
 */
GT_STATUS prvCpssSip6TxqUtilsPreemptionStatusGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    physicalPortNum,
    OUT GT_BOOL                *preemptionEnabledPtr,
    OUT GT_BOOL                *actAsPreemptiveChannelPtr,
    OUT GT_PHYSICAL_PORT_NUM   *preemptivePhysicalPortNumPtr,
    OUT GT_U32                 *preemptiveLocalPortNumPtr
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * aNodePtr[2];
    GT_U32                           speedInMb  = 0;
    GT_BOOL                          portMapped = GT_FALSE,portContainMac = GT_FALSE;

    CPSS_NULL_PTR_CHECK_MAC(preemptionEnabledPtr);
    CPSS_NULL_PTR_CHECK_MAC(actAsPreemptiveChannelPtr);

    *preemptionEnabledPtr      = GT_FALSE;
    *actAsPreemptiveChannelPtr = GT_FALSE;

    rc = prvCpssSip6TxqUtilsPortParametersGet(devNum,physicalPortNum,&portMapped,&portContainMac);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(TXQ_IS_PREEMPTIVE_DEVICE(devNum)&&(portMapped==GT_TRUE)&&(portContainMac==GT_TRUE))
    {
      PRV_CPSS_DXCH_CURRENT_PORT_SPEED_IN_MB_MAC(devNum, physicalPortNum,speedInMb);
      /*check if preemption is enabled on port.*/
      rc = prvCpssDxChTxqSip6_10PreemptionEnableGet(devNum,physicalPortNum,
       speedInMb,
       preemptionEnabledPtr);
      if (rc)
      {
          return rc;
      }
      rc =prvCpssDxChTxqSip6PortActAsPreemptiveChannelGet(devNum,physicalPortNum,actAsPreemptiveChannelPtr,NULL);
      if (rc)
      {
          return rc;
      }

      if((NULL!=preemptivePhysicalPortNumPtr)&&(*preemptionEnabledPtr==GT_TRUE))
      {
       rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &(aNodePtr[0]));
       if (rc!=GT_OK)
       {
           return rc;
       }
       rc = prvCpssDxChTxqSip6PreeptivePartnerAnodeGet(devNum, aNodePtr[0],&(aNodePtr[1]));
       if (rc != GT_OK)
       {
           return rc;
       }
       *preemptivePhysicalPortNumPtr = aNodePtr[1]->physicalPort;

       if(preemptiveLocalPortNumPtr)
       {
         *preemptiveLocalPortNumPtr = aNodePtr[1]->queuesData.localPort;
       }
      }
    }
    return GT_OK;

}

/**
 * @internal prvCpssSip6_10TxqUtilsPreemptionSpeedValidGet function
 * @endinternal
 *
 * @brief  Check that required speed is less thne maximal speed supported in preemption mode
 *               for perticular port couple
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                   -PP's device number.
 *  @param[in] speedInMb               desired speed (for validation purpose)
 *  @param[in] coupleIndex               index of the couple({express channel,preemptive channel})
*
 *@param[out] preemptionEnabledPtr                            (pointer to )preemption enabled for port
* @param[out] actAsPreemptiveChannelPtr                  (pointer to )port act as preemptive channel
* @param[out] preemptivePhysicalPortNumPtr            (pointer to )physical port number used as preemptive channel
*
 * @retval GT_TRUE                                          -valid speed
 * @retval GT_FALSE                                         - speed is too big and can't be used
*
 *
 */
GT_BOOL prvCpssSip6_10TxqUtilsPreemptionSpeedValidGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  speedInMb,
    IN GT_U32                  coupleIndex
)
{
    GT_BOOL result = GT_TRUE;
    GT_U32  startIndex;
    GT_U32  size;
    const GT_U32    maxSpeed[] =
    {
        TXQ_AC5P_DP_PREEMPTIVE_COUPLES_MAX_SPEED_MAC,
        TXQ_HARRIER_DP_PREEMPTIVE_COUPLES_MAX_SPEED_MAC,
        TXQ_IRONMAN_DP_PREEMPTIVE_COUPLES_MAX_SPEED_MAC
    };

    if (! TXQ_IS_PREEMPTIVE_DEVICE(devNum))
    {
        return GT_FALSE;
    }

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
    {
     startIndex = TXQ_IRONMAN_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC;
     size = TXQ_IRONMAN_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC;
    }
    else if(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum) == GT_TRUE)
    {
     startIndex = TXQ_HARRRIER_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC;
     size = TXQ_HARRRIER_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC;
    }
    else
    {
     startIndex = TXQ_AC5P_DP_MAX_PREEMPTIVE_COUPLES_FIRST_MAC;
     size = TXQ_AC5P_DP_MAX_PREEMPTIVE_COUPLES_NUM_MAC;
    }

    if (coupleIndex >= size)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE,"coupleIndex %d out of range",coupleIndex);
    }

    result = speedInMb>maxSpeed[startIndex + coupleIndex] ? GT_FALSE : GT_TRUE;
    return result;
}


GT_STATUS prvCpssSip6TxqUtilsPortParametersGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    OUT GT_BOOL             *portMappedPtr,
    OUT GT_BOOL             *containMacPtr
)
{
    GT_STATUS rc;
    GT_U32    aNodeInd;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *aNodePtr;
    CPSS_NULL_PTR_CHECK_MAC(portMappedPtr);
    CPSS_NULL_PTR_CHECK_MAC(containMacPtr);

    rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum,physicalPortNum,NULL,&aNodeInd);
    if (rc == GT_NOT_FOUND)
    {
        *portMappedPtr = GT_FALSE;
        return GT_OK;
    }
    else if(rc == GT_OK)
    {
     *portMappedPtr = GT_TRUE;
    }

    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,physicalPortNum,&aNodePtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if(aNodePtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
      *containMacPtr = GT_TRUE;
    }
    else
    {
      *containMacPtr = GT_FALSE;
    }

    return rc;

}


/**
 * @internal prvCpssTxqPreemptionUtilsPortConfigurationAllowedGet function
 * @endinternal
 *
 * @brief  Check thatport is not serving as preemptive channel and can be configured
 *
 * @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman; AC5P
 * @note   NOT APPLICABLE DEVICES: None
 *
 * @param[in] devNum                   -PP's device number.
 *  @param[in] physicalPortNum              portNum
*
 * @retval GT_OK                                          -port can be configured
 * @retval GT_BAD_STATE                           port can not be configured
*
 *
 */
GT_STATUS prvCpssTxqPreemptionUtilsPortConfigurationAllowedGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum
)
{
    GT_BOOL actAsPreemptiveChannel = GT_FALSE;
    GT_PHYSICAL_PORT_NUM expPort;
    GT_STATUS rc;

    if((GT_FALSE == PRV_CPSS_PP_MAC(devNum)->isGmDevice)&&(PRV_CPSS_PP_MAC(devNum)->preemptionSupported))
    {
      /*check that port does not serve as additional preeemption  channel*/
      rc =prvCpssDxChTxqSip6PortActAsPreemptiveChannelGet(devNum,physicalPortNum,&actAsPreemptiveChannel,&expPort);
      if (rc)
      {
          return rc;
      }
      if(GT_TRUE==actAsPreemptiveChannel)
      {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, \
            " portNum = %d is used for preemption and can not be configured.EXP port is %d \n", \
            physicalPortNum,expPort);
      }
    }

    return GT_OK;

}
/**
* @internal prvCpssTxqPreemptionUtilsPortStatCounterGet function
* @endinternal
*
* @brief   Gets port preemption statistic counter.
*
* @note   APPLICABLE DEVICES:      AC5P; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] counter                    - statistic counter tye
*
* @param[out] valuePtr                  - (pointer to)counter value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssTxqPreemptionUtilsPortStatCounterGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_MAC_MTI_BR_STAT_COUNTER_ENT counter,
    OUT GT_U32  *valuePtr
)
{

   GT_STATUS rc;

   GT_U32 portMacNum; /* MAC number */
   PRV_CPSS_PORT_TYPE_ENT macType;
   GT_U32 regAddr,value;
   GT_U32 offset,size;

   CPSS_NULL_PTR_CHECK_MAC(valuePtr);

   PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

   macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

   *valuePtr = 0;

   if (TXQ_IS_PREEMPTIVE_DEVICE(devNum)&& (macType == PRV_CPSS_PORT_MTI_100_E))
   {
       switch(counter)
       {
            case CPSS_MTI_BR_TX_FRAG_COUNT_TX_FRAG_COUNT_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.BR.brTxFragCount;
                offset = PRV_BR_STAT_MAC100_REG_BR_TX_FRAG_COUNT_TX_FRAG_COUNT_FIELD_OFFSET;
                size = PRV_BR_STAT_MAC100_REG_BR_TX_FRAG_COUNT_TX_FRAG_COUNT_FIELD_SIZE;
                break;
            case CPSS_MTI_BR_RX_FRAG_COUNT_RX_FRAG_COUNT_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.BR.brRxFragCount;
                offset = PRV_BR_STAT_MAC100_REG_BR_RX_FRAG_COUNT_RX_FRAG_COUNT_FIELD_OFFSET;
                size = PRV_BR_STAT_MAC100_REG_BR_RX_FRAG_COUNT_RX_FRAG_COUNT_FIELD_SIZE;
                break;
            case CPSS_MTI_BR_TX_HOLD_COUNT_TX_HOLD_COUNT_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.BR.brTxHoldCount;
                offset = PRV_BR_STAT_MAC100_REG_BR_TX_HOLD_COUNT_TX_HOLD_COUNT_FIELD_OFFSET;
                size = PRV_BR_STAT_MAC100_REG_BR_TX_HOLD_COUNT_TX_HOLD_COUNT_FIELD_SIZE;
                break;
            case CPSS_MTI_BR_RX_SMD_ERR_COUNT_RX_SMD_ERR_COUNT_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.BR.brRxSmdErrCount;
                offset = PRV_BR_STAT_MAC100_REG_BR_RX_SMD_ERR_COUNT_RX_SMD_ERR_COUNT_FIELD_OFFSET;
                size = PRV_BR_STAT_MAC100_REG_BR_RX_SMD_ERR_COUNT_RX_SMD_ERR_COUNT_FIELD_SIZE;
                break;
            case CPSS_MTI_BR_RX_ASSY_ERR_COUNT_RX_ASSY_ERR_COUNT_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.BR.brRxAssyErrCount;
                offset = PRV_BR_STAT_MAC100_REG_BR_RX_ASSY_ERR_COUNT_RX_ASSY_ERR_COUNT_FIELD_OFFSET;
                size = PRV_BR_STAT_MAC100_REG_BR_RX_ASSY_ERR_COUNT_RX_ASSY_ERR_COUNT_FIELD_SIZE;
                break;
            case CPSS_MTI_BR_RX_ASSY_OK_COUNT_RX_ASSY_OK_COUNT_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.BR.brRxAssyOkCount;
                offset = PRV_BR_STAT_MAC100_REG_BR_RX_ASSY_OK_COUNT_RX_ASSY_OK_COUNT_FIELD_OFFSET;
                size = PRV_BR_STAT_MAC100_REG_BR_RX_ASSY_OK_COUNT_RX_ASSY_OK_COUNT_FIELD_SIZE;
                break;
            case CPSS_MTI_BR_RX_VERIFY_COUNT_RX_VERIFY_COUNT_GOOD_E:
            case CPSS_MTI_BR_RX_VERIFY_COUNT_RX_VERIFY_COUNT_BAD_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.BR.brRxVerifyCount;
                if(CPSS_MTI_BR_RX_VERIFY_COUNT_RX_VERIFY_COUNT_GOOD_E==counter)
                {
                    offset = PRV_BR_STAT_MAC100_REG_BR_RX_VERIFY_COUNT_RX_VERIFY_COUNT_GOOD_FIELD_OFFSET;
                    size = PRV_BR_STAT_MAC100_REG_BR_RX_VERIFY_COUNT_RX_VERIFY_COUNT_GOOD_FIELD_SIZE;
                }
                else
                {
                    offset = PRV_BR_STAT_MAC100_REG_BR_RX_VERIFY_COUNT_RX_VERIFY_COUNT_BAD_FIELD_OFFSET;
                    size = PRV_BR_STAT_MAC100_REG_BR_RX_VERIFY_COUNT_RX_VERIFY_COUNT_BAD_FIELD_SIZE;
                }
                break;
            case CPSS_MTI_BR_RX_RESPONSE_COUNT_RX_RESP_COUNT_GOOD_E:
            case CPSS_MTI_BR_RX_RESPONSE_COUNT_RX_RESP_COUNT_BAD_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.BR.brRxResponseCount;
                if(CPSS_MTI_BR_RX_RESPONSE_COUNT_RX_RESP_COUNT_GOOD_E==counter)
                {
                    offset = PRV_BR_STAT_MAC100_REG_BR_RX_RESPONSE_COUNT_RX_RESP_COUNT_GOOD_FIELD_OFFSET;
                    size = PRV_BR_STAT_MAC100_REG_BR_RX_RESPONSE_COUNT_RX_RESP_COUNT_GOOD_FIELD_SIZE;
                }
                else
                {
                    offset = PRV_BR_STAT_MAC100_REG_BR_RX_RESPONSE_COUNT_RX_RESP_COUNT_BAD_FIELD_OFFSET;
                    size = PRV_BR_STAT_MAC100_REG_BR_RX_RESPONSE_COUNT_RX_RESP_COUNT_BAD_FIELD_SIZE;
                }
                break;
            case CPSS_MTI_BR_TX_VERIF_COUNT_TX_VERIF_COUNT_E:
            case CPSS_MTI_BR_TX_VERIF_COUNT_TX_RESP_COUNT_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.BR.brTxVerifCount;
                if(CPSS_MTI_BR_TX_VERIF_COUNT_TX_VERIF_COUNT_E==counter)
                {
                    offset = PRV_BR_STAT_MAC100_REG_BR_TX_VERIF_COUNT_TX_VERIF_COUNT_FIELD_OFFSET;
                    size = PRV_BR_STAT_MAC100_REG_BR_TX_VERIF_COUNT_TX_VERIF_COUNT_FIELD_SIZE;
                }
                else
                {
                    offset = PRV_BR_STAT_MAC100_REG_BR_TX_VERIF_COUNT_TX_RESP_COUNT_FIELD_OFFSET;
                    size = PRV_BR_STAT_MAC100_REG_BR_TX_VERIF_COUNT_TX_RESP_COUNT_FIELD_SIZE;
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "unknown BR statistic type %d \n",counter);
                break;
       }



       if(PRV_CPSS_SW_PTR_ENTRY_UNUSED != regAddr)
       {
           rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
               PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum),
                   regAddr,&value);
           if(rc!=GT_OK)
           {
              return rc;
           }

          *valuePtr = U32_GET_FIELD_MAC(value,offset,size);
       }
   }

   return GT_OK;
}

#ifdef INC_DEBUG_FUNCTIONS

/**
 * @internal prvCpssDxChTxqSip6_10PreemptionPortBitmapEnableSet function
 * @endinternal
 *
 * @brief  Set enable/disable preemption at PDX glue for set of physical ports.
 * @note   used for debugging
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                - PP's device number.
 * @param[in] basePortNum           - base physical port number
 * @param[in] portIncBmp            - bitmap as set of nombers 0-31 to add to basePortNum
 * @param[in] enable                - if equal GT_TRUE then preemption is enabled in PDX,GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS  prvCpssDxChTxqSip6_10PreemptionPortBitmapEnableSet
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 basePortNum,
    IN    GT_U32                               portIncBmp,
    IN    GT_BOOL                              enable
)
{
    GT_STATUS rc;
    GT_U32    i;

    for (i = 0; (i < 32); i++)
    {
        if ((portIncBmp & (1 << i)) == 0) continue;
        rc = prvCpssDxChTxqSip6_10PreemptionEnableSet(
            devNum, (basePortNum + i), enable);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

#endif /*INC_DEBUG_FUNCTIONS*/


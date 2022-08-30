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
* @file prvCpssDxChTxqSchedulingUtils.c
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
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqSearchUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#define DEFAULT_SCHED_PROFILE 0


/**
 * @internal prvCpssFalconTxqUtilsInitSchedProfilesDb function
 * @endinternal
 *
 * @brief   Initialize schedProfiles database . All the queues are in WRR gropu 0. TC 0-3 weight 1 , TC 0-3 weight 5
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 */
GT_VOID  prvCpssFalconTxqUtilsInitSchedProfilesDb
(
    IN GT_U8 devNum
)
{
    GT_U32 i, j;

    /*init scheduling profiles*/
    for (i = 0; i < CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM; i++)
    {
        /*all ports are binded to profile 0 */
        for (j = 0; j < 32; j++)
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].bindedPortsBmp[j] = 0x0;
        }

        for (j = 0; j < 8; j++)
        {
            /*all queues in WRR group 0 */
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].wrrEnableBmp[j]    = 0xFFFFFFFF;
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].wrrGroupBmp[j]     = 0;
        }


        for (j = 0; j < PRV_CPSS_DXCH_SIP6_20_QUEUES_PER_PORT_MAX_Q_NUM_VAL_MAC; j++)
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].weights[j] = (j%8 < 4) ? CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum) : (5 * CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum));
        }
    }
}


GT_VOID prvCpssFalconTxqUtilsDefaultBindPortsToSchedDb
(
    IN GT_U8 devNum
)
{
    GT_U32  i;
    GT_U32  wordIndex;
    GT_U32  bitIndex;
    for (i = 0; i < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; i++)
    {
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[i].aNodeInd != CPSS_SIP6_TXQ_INVAL_DATA_CNS)
        {
            wordIndex   = i / 32;
            bitIndex    = i % 32;
            /*bind  to the default profile*/
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[DEFAULT_SCHED_PROFILE].bindedPortsBmp[wordIndex] |= (1 << bitIndex);
        }
    }
}

static GT_STATUS prvCpssFalconTxqUtilsSyncPortToSchedulerProfile
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profileSet
)
{
    GT_U32                  tileNum, aNodeIndex;
    GT_STATUS               rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr[2];
    GT_BOOL     preemptionEnabled,actAsPreemptiveChannel;

    rc = prvCpssSip6TxqUtilsPreemptionStatusGet(devNum,physicalPortNum,&preemptionEnabled,&actAsPreemptiveChannel,NULL,NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc , "prvCpssSip6TxqUtilsPreemptionStatusGet failed for port %d\n", physicalPortNum);
    }
    /*Find A-node index*/
    rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &aNodeIndex);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &(aNodePtr[0]));
    if (rc != GT_OK)
    {
        return rc;
    }

    rc =  prvCpssFalconTxqPdqUpdateTypeANodeSchedulingAttributes(devNum, tileNum, aNodePtr[0],
                                      &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet]));
    if (rc != GT_OK)
    {
        return rc;
    }

    if(GT_TRUE==preemptionEnabled)
    {
        /*Find Preeemtive  A-node index*/

        rc = prvCpssDxChTxqSip6PreeptivePartnerAnodeGet(devNum, aNodePtr[0],&(aNodePtr[1]));
        if (rc != GT_OK)
        {
            return rc;
        }

        rc =  prvCpssFalconTxqPdqUpdateTypeANodeSchedulingAttributes(devNum, tileNum, aNodePtr[1],
                                      &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc ;
}
static GT_STATUS prvCpssFalconTxqUtilsSyncBindedPortsToSchedulerProfile
(
    IN GT_U8 devNum,
    IN CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profileSet
)
{
    GT_U32      j;
    GT_STATUS   rc;
    GT_U32      tmp, portNum, shift;
    GT_BOOL     preemptionEnabled,actAsPreemptiveChannel;
    for (j = 0; j < 32; j++)
    {
        tmp = PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].bindedPortsBmp[j];
        if (tmp)
        {
            shift = 0;
            while (tmp)
            {
                if (tmp & 0x1)
                {
                    portNum = j * 32 + shift;
                    rc = prvCpssSip6TxqUtilsPreemptionStatusGet(devNum,portNum,&preemptionEnabled,&actAsPreemptiveChannel,NULL,NULL);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc , "prvCpssSip6TxqUtilsPreemptionStatusGet failed for port %d\n", portNum);
                    }

                    if(GT_TRUE==actAsPreemptiveChannel)
                    {
                        /*do not sync preemptive partners*/
                        continue;
                    }

                    rc  = prvCpssFalconTxqUtilsSyncPortToSchedulerProfile(devNum, portNum, profileSet);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                tmp >>= 1;
                shift++;
            }
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsBindPortToSchedulerProfile function
 * @endinternal
 *
 * @brief  Bind a port to scheduler profile set.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                  -  device number
 * @param[in] physicalPortNum           -port number
 * @param[in] profileSet  -The Profile Set in which the scheduler's parameters are
 *                                      associated.
*
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsBindPortToSchedulerProfile
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profileSet,
    IN GT_BOOL  forceUpdate
)
{
    GT_U32      wordIndex, i;
    GT_U32      bitIndex;
    GT_STATUS   rc;

    /*check if mapped*/
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNum].aNodeInd == CPSS_SIP6_TXQ_INVAL_DATA_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Unmapped port number %d\n", physicalPortNum);
    }

     wordIndex   = physicalPortNum / 32;
     bitIndex    = physicalPortNum % 32;
     /*First delete from old profile shadow*/
     for (i = 0; i < CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM; i++)
     {
         if ( (PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].bindedPortsBmp[wordIndex]) & (1 << bitIndex))
         {
             if (i == (GT_U32)profileSet)
             {
                if(GT_FALSE == forceUpdate)
                {
                 /*The port is already binded to this profile*/
                  return GT_OK;
                 }
             }
             PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].bindedPortsBmp[wordIndex] &= ~((1 << bitIndex));
             break;
         }
     }
     /*Now update new profile shadow*/
     PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].bindedPortsBmp[wordIndex] |= ((1 << bitIndex));

     /*Now update HW*/
     rc = prvCpssFalconTxqUtilsSyncPortToSchedulerProfile(devNum, physicalPortNum, profileSet);
     if(rc!=GT_OK)
     {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
     }

    return rc;
}
/**
 * @internal prvCpssFalconTxqUtilsBindPortToSchedulerProfile function
 * @endinternal
 *
 * @brief   Get scheduler profile set that is binded to the port.
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
 * @param[out] profileSetPtr  -The Profile Set in which the scheduler's parameters are
 *                                      associated.
 */
GT_STATUS prvCpssFalconTxqUtilsPortSchedulerProfileGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    OUT CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  * profileSetPtr
)
{
    GT_U32      wordIndex, i;
    GT_U32      bitIndex;
    GT_STATUS   rc;
    /*check if mapped*/
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNum].aNodeInd == CPSS_SIP6_TXQ_INVAL_DATA_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Unmapped port number %d\n", physicalPortNum);
    }
    wordIndex   = physicalPortNum / 32;
    bitIndex    = physicalPortNum % 32;
    for (i = 0; i < CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM; i++)
    {
        if ( (PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].bindedPortsBmp[wordIndex]) & (1 << bitIndex))
        {
            break;
        }
    }
    if (i == CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM)
    {
        rc = GT_NOT_FOUND;
    }
    else
    {
        rc      = GT_OK;
        *profileSetPtr  = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E + i;
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqUtilsWrrProfileSet function
 * @endinternal
 *
 * @brief   Set Weighted Round Robin profile on the specified port's
 *         Traffic Class Queue.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 * @param[in] tcQueue                  - traffic class queue on this Port (0..7)
 * @param[in] wrrWeight                - proportion of bandwidth assigned to this queue
 *                                      relative to the other queues in this Arbitration Group
 * @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the wrrWeight
 *                                      Parameter is associated.
 */
GT_STATUS prvCpssFalconTxqUtilsWrrProfileSet
(
    IN GT_U8 devNum,
    IN GT_U8 tcQueue,
    IN GT_U8 wrrWeight,
    IN CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profileSet
)
{
    GT_STATUS   rc;
    GT_U32      *weightPtr;
    GT_U32      oldWeight;
    GT_U32      newWeight;

    weightPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].weights[tcQueue];
    oldWeight = *weightPtr;
    newWeight = wrrWeight * CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum);

    /* Update SW shadow */
    *weightPtr = newWeight;

    /* Update HW */
    rc = prvCpssFalconTxqUtilsSyncBindedPortsToSchedulerProfile(devNum, profileSet);
    if (rc != GT_OK)
    {
        /* Restore SW shadow since HW was not successfully updated */
        *weightPtr = oldWeight;
    }

    return rc;
}

/**
 * @internal prvCpssFalconTxqUtilsWrrProfileGet function
 * @endinternal
 *
 * @brief   Get Weighted Round Robin profile on the specified port's
 *         Traffic Class Queue.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 * @param[in] tcQueue                  - traffic class queue on this Port (0..7)
 * @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the wrrWeight
 *                                      Parameter is associated.
 *
 * @param[out] wrrWeightPtr             - Pointer to proportion of bandwidth assigned to this queue
 *                                      relative to the other queues in this  Arbitration Group
 */
GT_STATUS prvCpssFalconTxqUtilsWrrProfileGet
(
    IN GT_U8 devNum,
    IN GT_U8 tcQueue,
    IN CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profileSet,
    OUT GT_U8                                  *wrrWeightPtr
)
{
    *wrrWeightPtr = (GT_U8)(PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].weights[tcQueue] / CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum));
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsArbGroupSet function
 * @endinternal
 *
 * @brief   Set Traffic Class Queue scheduling arbitration group on
 *         specificed profile of specified device.
 *
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 * @param[in] tcQueue                  - traffic class queue (0..7)
 * @param[in] arbGroup                 - scheduling arbitration group:
 *                                      1) Strict Priority
 *                                      2) WRR Group 1
 *                                      3) WRR Group 0
 * @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
 *                                      parameter is associated.
 */
GT_STATUS prvCpssFalconTxqUtilsArbGroupSet
(
    IN GT_U8 devNum,
    IN GT_U8 tcQueue,
    IN CPSS_PORT_TX_Q_ARB_GROUP_ENT arbGroup,
    IN CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profileSet
)
{
    GT_U32 * wrrGroupBmpPtr =  PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrGroupBmp;
    GT_U32 * wrrEnableBmpPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrEnableBmp;

    /*256 queues support*/
    wrrGroupBmpPtr+=(tcQueue>>5);
    wrrEnableBmpPtr+=(tcQueue>>5);
    tcQueue=tcQueue&0x1F;

    if (arbGroup == CPSS_PORT_TX_SP_ARB_GROUP_E)
    {
        if (((*wrrEnableBmpPtr) & (1 << tcQueue)) == 0)
        {
            /*Already configured*/
            return GT_OK;
        }
        /*Move from WRR to SP*/
        (*wrrEnableBmpPtr) &= ~(1 << tcQueue);
    }
    else
    {
        if (((*wrrEnableBmpPtr) & (1 << tcQueue)) == ((GT_U32)1 << tcQueue))
        {
            if (arbGroup == CPSS_PORT_TX_WRR_ARB_GROUP_0_E)
            {
                if ((*wrrGroupBmpPtr & (1 << tcQueue)) == 0)
                {
                    /*Already configured*/
                    return GT_OK;
                }
                /*Move from WRR1 to WRR0*/
                *wrrGroupBmpPtr &= ~(1 << tcQueue);
            }
            else
            {
                if (((*wrrGroupBmpPtr) & (1 << tcQueue)) == 1)
                {
                    /*Already configured*/
                    return GT_OK;
                }
                /*Move from WRR0 to WRR1*/
                (*wrrGroupBmpPtr) |= (1 << tcQueue);
            }
        }
        /*Move from SP to WRR*/
        (*wrrEnableBmpPtr) |= (1 << tcQueue);
        if (arbGroup == CPSS_PORT_TX_WRR_ARB_GROUP_0_E)
        {
            (*wrrGroupBmpPtr) &= ~(1 << tcQueue);
        }
        else
        {
            (*wrrGroupBmpPtr) |= (1 << tcQueue);
        }
    }
    /*Update HW*/
    return prvCpssFalconTxqUtilsSyncBindedPortsToSchedulerProfile(devNum, profileSet);
}
/**
 * @internal prvCpssFalconTxqUtilsArbGroupGet function
 * @endinternal
 *
 * @brief   Get Traffic Class Queue scheduling arbitration group on
 *         specificed profile of specified device.
 *
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 * @param[in] tcQueue                  - traffic class queue (0..7)
 * @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
 *                                      parameter is associated.
 *
 * @param[out] arbGroupPtr              - Pointer to scheduling arbitration group:
 *                                      1) Strict Priority
 *                                      2) WRR Group 1
 *                                      3) WRR Group 0
 */
GT_STATUS prvCpssFalconTxqUtilsArbGroupGet
(
    IN GT_U8 devNum,
    IN GT_U8 tcQueue,
    IN CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profileSet,
    OUT CPSS_PORT_TX_Q_ARB_GROUP_ENT            *arbGroupPtr
)
{
    GT_U32 * wrrGroupBmpPtr,*wrrEnableBmpPtr;
    wrrGroupBmpPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrGroupBmp;
    wrrEnableBmpPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[(GT_U32)profileSet].wrrEnableBmp;

    /*256 queues support*/
    wrrGroupBmpPtr+=(tcQueue>>5);
    wrrEnableBmpPtr+=(tcQueue>>5);
    tcQueue=tcQueue&0x1F;

    if (((*wrrEnableBmpPtr) & (1 << tcQueue)) == 0)
    {
        *arbGroupPtr = CPSS_PORT_TX_SP_ARB_GROUP_E;
    }
    else if ((*wrrGroupBmpPtr) & (1 << tcQueue))
    {
        *arbGroupPtr = CPSS_PORT_TX_WRR_ARB_GROUP_1_E;
    }
    else
    {
        *arbGroupPtr = CPSS_PORT_TX_WRR_ARB_GROUP_0_E;
    }
    return GT_OK;
}


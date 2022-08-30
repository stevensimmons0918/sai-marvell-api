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
* @file prvCpssDxChTxqSdq.c
*
* @brief CPSS SIP6 TXQ Sdq low level configurations.
*
* @version   1
********************************************************************************
*/


#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/falcon/pipe/prvCpssFalconTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5p/pipe/prvCpssAc5pTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5x/pipe/prvCpssAc5xTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/harrier/pipe/prvCpssHarrierTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ironman/pipe/prvCpssIronmanTxqSdqRegFile.h>


#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>


#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqDebugUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

typedef float GT_FLOAT;

#define PRV_TH_GRANULARITY_SHIFT 8 /* 256*/
#define PRV_TXQ_UNIT_NAME "SDQ"
#define PRV_TXQ_LOG_REG GT_FALSE
#define PRV_TXQ_LOG_TABLE GT_FALSE


static GT_STATUS prvCpssSip6TxqSdqSemiEligThresholdGet
(
     IN  GT_U8                      devNum,
     IN  CPSS_PORT_SPEED_ENT        speed,
     OUT GT_U32                     *thresholdPtr
);


/**
* @internal prvCpssFalconTxqSdqInitQueueThresholdAttributes function
* @endinternal
*
* @brief   Initialize queue thresholds for given speed
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] speed                   - port speed
* @param[out] queueAttributesPtr     - (pointer to) queue thresholds
*
* @retval GT_OK               - on success.
* @retval GT_FAIL             - no thresholds are configured for this speed
*/

static GT_STATUS prvCpssFalconTxqSdqInitQueueThresholdAttributes
(
    GT_U32                                              devNum,
    IN  CPSS_PORT_SPEED_ENT                             speed,
    IN  PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES     * queueAttributesPtr
);

static GT_STATUS prvCpssFalconTxqSdqInitPortAttributes
(
    IN  CPSS_PORT_SPEED_ENT                         speed,
    IN  PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES  * portAttributesPtr
);

static GT_STATUS prvCpssSip6TxqSdqLogInit
(
   INOUT PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC * logDataPtr,
   IN GT_U32                                   sdqNum,
   IN GT_CHAR_PTR                              regName
)
{
    CPSS_NULL_PTR_CHECK_MAC(logDataPtr);

    logDataPtr->log = PRV_TXQ_LOG_REG;
    logDataPtr->unitName = PRV_TXQ_UNIT_NAME;
    logDataPtr->unitIndex = sdqNum;
    logDataPtr->regName = regName;
    logDataPtr->regIndex = PRV_TXQ_LOG_NO_INDEX;

    return GT_OK;
}
static GT_STATUS prvCpssSip6TxqSdqTableLogInit
(
   INOUT PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC * logDataPtr,
   IN GT_U32                                   tileNum,
   IN GT_U32                                   sdqNum,
   IN CPSS_DXCH_TABLE_ENT                      tableIdentity,
   IN GT_CHAR_PTR                              tableName,
   IN GT_U32                                   entryIndex
)
{
    CPSS_NULL_PTR_CHECK_MAC(logDataPtr);

    GT_UNUSED_PARAM(tableIdentity);

    logDataPtr->log = PRV_TXQ_LOG_TABLE;
    logDataPtr->unitName = PRV_TXQ_UNIT_NAME;
    logDataPtr->tileIndex = tileNum;
    logDataPtr->unitIndex = sdqNum;
    logDataPtr->tableName = tableName;
    logDataPtr->entryIndex = entryIndex;

    return GT_OK;
}



GT_STATUS prvCpssSip6TxqSdqLocalPortQcnFactorSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       startQ,
    IN GT_U32                                       endQ,
    IN GT_U32                                       qcnFactor
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"queue_config");
    if (rc != GT_OK)
    {
        return rc;
    }

    for(i= startQ;i<=endQ;i++)
    {
        log.regIndex = i;

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
             tileNum,&log,
             PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[i],
             TXQ_SDQ_FIELD_GET(devNum,QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_OFFSET),
             TXQ_SDQ_FIELD_GET(devNum,QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_SIZE),
             qcnFactor);

        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

GT_STATUS prvCpssSip6TxqSdqLocalPortQcnFactorGet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       queue,
    IN GT_U32                                       *qcnFactorPtr
)
{
    GT_STATUS rc = GT_OK;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    CPSS_NULL_PTR_CHECK_MAC(qcnFactorPtr);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
             tileNum,
             PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queue],
             TXQ_SDQ_FIELD_GET(devNum,QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_OFFSET),
             TXQ_SDQ_FIELD_GET(devNum,QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_SIZE),
             qcnFactorPtr);


    return rc;
}

GT_STATUS prvCpssSip6TxqSdqLocalPortQcnFactorDump
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       queue,
    IN GT_U32                                       numberOfQueues
)
{
    GT_STATUS rc;
    GT_U32    i,data;

    for(i=0;i<numberOfQueues;i++)
    {
        rc =prvCpssSip6TxqSdqLocalPortQcnFactorGet(devNum,tileNum,sdqNum,queue+i,&data);
        if(rc!=GT_OK)
        {
          return rc;
        }

        cpssOsPrintf("Q %d QCN factor 0x%02x\n",queue+i,data);
    }

    return GT_OK;

}


/**
* @internal prvCpssFalconTxqSdqLocalPortQueueRangeSet function
* @endinternal
*
* @brief   Map port to queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] range                    - the  of the queues(APPLICABLE RANGES:0..399).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqLocalPortQueueRangeSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       localPortNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_QUEUE_RANGE  range
)
{
    GT_U32 regValue = 0;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);


    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if((range.hiQueueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)||
        (range.lowQueueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)||
        (range.lowQueueNumber>=range.hiQueueNumber))
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Port_Range_Low");
    if (rc != GT_OK)
    {
        return rc;
    }
    log.regIndex = localPortNum;

    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_SDQ_FIELD_GET(devNum,PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET),
                            TXQ_SDQ_FIELD_GET(devNum,PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE),
                            range.lowQueueNumber);

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Range_Low[localPortNum],
                 0,
                 32,
                 regValue);

     if(rc!=GT_OK)
     {
        return rc;
     }

     regValue = 0;

     U32_SET_FIELD_MASKED_MAC(regValue,TXQ_SDQ_FIELD_GET(devNum,PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET),
                                TXQ_SDQ_FIELD_GET(devNum,PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE),
                                range.hiQueueNumber);

     log.regName = "Port_Range_High";

     rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Range_High[localPortNum],
                 0,
                 32,
                 regValue);

    return rc;
}
/**
* @internal prvCpssFalconTxqSdqLocalPortQueueRangeGet function
* @endinternal
*
* @brief   Read port to queue from specific SDQ
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqLocalPortQueueRangeGet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           tileNum,
    IN GT_U32                                           sdqNum,
    IN GT_U32                                           localPortNum,
    OUT PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_QUEUE_RANGE     * rangePtr
)
{

    GT_STATUS rc;
    GT_U32 regValue;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Range_Low[localPortNum],
                 0,
                 32,
                 &regValue);

     rangePtr->lowQueueNumber= U32_GET_FIELD_MAC(regValue,
        TXQ_SDQ_FIELD_GET(devNum,PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET),
        TXQ_SDQ_FIELD_GET(devNum,PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE));


    if(rc!=GT_OK)
    {
        return rc;
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Range_High[localPortNum],
                 0,
                 32,
                 &regValue);

     rangePtr->hiQueueNumber = U32_GET_FIELD_MAC(regValue,
        TXQ_SDQ_FIELD_GET(devNum,PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET),
        TXQ_SDQ_FIELD_GET(devNum,PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE));

     return rc;

}
/**
* @internal prvCpssFalconTxqSdqQueueAttributesSet function
* @endinternal
*
* @brief   Queue attributes set
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] queueNumber              - queue number(APPLICABLE RANGES:0..399).
* @param[in] speed                    - speed of port that contain the queue
* @param[in] enable                   - queue enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqQueueAttributesSet
(
    IN GT_U8                devNum,
    IN GT_U32               tileNum,
    IN GT_U32               sdqNum,
    IN GT_U32               queueNumber,
    IN CPSS_PORT_SPEED_ENT  speed,
    IN GT_BOOL              semiEligEnable
)
{
    GT_U32 regValue[2];
    GT_STATUS rc;
    GT_U32 size;
    GT_U32 value;
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES  queueAttributes = { GT_FALSE, 0, 0, 0, 0, 0, 0 };

    GT_U32 semiEligThreshold;
    CPSS_DXCH_TABLE_ENT table;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC tableLog;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber >= CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssFalconTxqSdqInitQueueThresholdAttributes(devNum,speed, &queueAttributes);

    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqSdqInitQueueAttributes failed\n");
    }

    if(queueAttributes.agingThreshold >= (GT_U32)(1 << TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_AGING_TH_FIELD_SIZE)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*According to Cider : "This threshold is 256B granule and is signed number defined by the msb"*/

    if((queueAttributes.highCreditThreshold >> PRV_TH_GRANULARITY_SHIFT) >= (GT_U32)(1 << TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((queueAttributes.lowCreditTheshold >> PRV_TH_GRANULARITY_SHIFT) >= (GT_U32)(1 << TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((queueAttributes.negativeCreditThreshold >> PRV_TH_GRANULARITY_SHIFT) >= (GT_U32)(1 << TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((queueAttributes.semiEligThreshold >> PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT) >= (GT_U32)(1 << TXQ_SDQ_FIELD_GET(devNum, QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(regValue, 0, sizeof(GT_U32) * 2);
    queueAttributes.enable = GT_TRUE; /*don't care - this bit does not work*/
    regValue[0] = 0;
    U32_SET_FIELD_MASKED_MAC(regValue[0], TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_QUEUE_EN_FIELD_OFFSET),
                             TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_QUEUE_EN_FIELD_SIZE),
                             queueAttributes.enable);


    U32_SET_FIELD_MASKED_MAC(regValue[0], TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_AGING_TH_FIELD_OFFSET),
                             TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_AGING_TH_FIELD_SIZE),
                             queueAttributes.agingThreshold >> PRV_TH_GRANULARITY_SHIFT);



    U32_SET_FIELD_MASKED_MAC(regValue[0], TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET),
                             TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE),
                             queueAttributes.negativeCreditThreshold > PRV_TH_GRANULARITY_SHIFT);



    value = U32_GET_FIELD_MAC(queueAttributes.lowCreditTheshold >> PRV_TH_GRANULARITY_SHIFT, 0, 32 - TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET));
    size = 32 - TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET);

    U32_SET_FIELD_MASKED_MAC(regValue[0], TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET),
                             size,
                             value);

    value = U32_GET_FIELD_MAC(queueAttributes.lowCreditTheshold >> PRV_TH_GRANULARITY_SHIFT,
                              32 - TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET),
                              TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE) - (32 - TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET)));

    size = TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE - (32 - TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET));

    U32_SET_FIELD_MASKED_MAC(regValue[1], 0,
                             size,
                             value);



    U32_SET_FIELD_MASKED_MAC(regValue[1], TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET) - 32,
                             TXQ_SDQ_FIELD_GET(devNum, QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE),
                             queueAttributes.highCreditThreshold >> PRV_TH_GRANULARITY_SHIFT);


    table= CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E + sdqNum + tileNum * CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC;

    rc = prvCpssSip6TxqSdqTableLogInit(&tableLog,tileNum,sdqNum,table,"TBL_queue_cfg",queueNumber);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssSip6TxqWriteTableEntry(devNum,&tableLog,
                                    table,
                                    queueNumber,
                                    regValue);

    if(rc != GT_OK)
    {
        return rc;
    }

    /*Now set semi-elig threshold*/

    semiEligThreshold = queueAttributes.semiEligThreshold >> PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT;

    /*round up*/
    if(queueAttributes.semiEligThreshold % (1 << PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT) > ((1 << (PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT - 1))))
    {
        semiEligThreshold++;
    }

    if(GT_FALSE == semiEligEnable)
    {
        semiEligThreshold = 0;
    }

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"queue_config");
    if (rc != GT_OK)
    {
        return rc;
    }

    log.regIndex = queueNumber;

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                                             tileNum,&log,
                                             PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queueNumber],
                                             TXQ_SDQ_FIELD_GET(devNum, QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_OFFSET),
                                             TXQ_SDQ_FIELD_GET(devNum, QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE),
                                             semiEligThreshold);

    return rc;
}

/**
* @internal prvCpssFalconTxqSdqQueueEnableSet function
* @endinternal
*
* @brief   Enable/disable queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] enable                - if GT_TRUE credit aging is enabled, otherwise disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqQueueEnableSet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNumber,
 IN GT_U32  tc,
 IN GT_BOOL enable
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(tc>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNumber);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Pause_TC");
    if (rc != GT_OK)
    {
        return rc;
    }

    log.regIndex = localPortNumber;

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Pause_TC[localPortNumber],
                 TXQ_SDQ_FIELD_GET(devNum,PAUSE_TC_PAUSE_TC_0_FIELD_OFFSET)+(tc&0xF) /*only 16 traffic classes*/,
                 1,
                 /*reverse logic*/
                 enable?0:1);

     return rc;



}

/**
* @internal prvCpssFalconTxqSdqQueueEnableGet function
* @endinternal
*
* @brief   Get Enable/disable queue status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..7).
*
* @param[out] enablePtr                - if GT_TRUE queue  is enabled , otherwise disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqQueueEnableGet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNumber,
 IN GT_U32  queueNumber,
 IN GT_BOOL * enablePtr
)
{
    GT_STATUS rc;
    GT_U32          regValue;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Pause_TC[localPortNumber],
                 TXQ_SDQ_FIELD_GET(devNum,PAUSE_TC_PAUSE_TC_0_FIELD_OFFSET)+(queueNumber&0xF)/*only 16 traffic classes*/,
                 1,
                 &regValue);

     if(rc==GT_OK)
     {
          /*reverse logic*/
        *enablePtr = (regValue == 1)?GT_FALSE:GT_TRUE;
     }

     return rc;



}

/**
* @internal prvCpssSip6TxqSdqPausedTcBmpSet function
* @endinternal
*
* @brief   Pause queues for local port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] localPortNum          - local port number of SDQ
* @param[in] pausedTcBmp           - bitmap of paused queues
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqSdqPausedTcBmpSet
(
 IN  GT_U8   devNum,
 IN  GT_U32  tileNum,
 IN  GT_U32  sdqNum,
 IN  GT_U32  localPortNum,
 IN  GT_U32  pausedTcBmp
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Pause_TC");
    if (rc != GT_OK)
    {
        return rc;
    }

    log.regIndex = localPortNum;

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Pause_TC[localPortNum],
                 0,
                 32,
                 pausedTcBmp);

   return rc;
}

/**
* @internal prvCpssSip6TxqSdqPausedTcBmpSet function
* @endinternal
*
* @brief   Pause queues for local port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] localPortNum          -local port number of SDQ
* @param[out] pausedTcBmpPtr       -(pointer to )bitmap of paused queues
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqSdqPausedTcBmpGet
(
 IN  GT_U8   devNum,
 IN  GT_U32  tileNum,
 IN  GT_U32  sdqNum,
 IN  GT_U32  localPortNum,
 OUT GT_U32  *pausedTcBmpPtr
)
{
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(pausedTcBmpPtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Pause_TC[localPortNum],
                 0,
                 32,
                 pausedTcBmpPtr);

    return rc;
}



/**
* @internal prvCpssFalconTxqSdqQueueAttributesGet function
* @endinternal
*
* @brief   Read Queue attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] queueNumber              - queue number(APPLICABLE RANGES:0..399).
*
* @param[out] queueAttributesPtr      - pointer to queue attributes
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqQueueAttributesGet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       queueNumber,
    OUT PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES * queueAttributesPtr
)
{
    GT_U32 regValue[2];
    GT_STATUS rc;
    GT_U32 size,value;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queueNumber],
                 0,
                 32,
                 regValue);


     if(rc!=GT_OK)
     {
        return rc;
     }

     queueAttributesPtr->tc = U32_GET_FIELD_MAC(regValue[0],
        TXQ_SDQ_FIELD_GET(devNum,QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET),
        TXQ_SDQ_FIELD_GET(devNum,QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE));

     rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNumber,
                                        regValue);
     if(rc!=GT_OK)
     {
        return rc;
     }

     queueAttributesPtr->enable = (GT_BOOL)U32_GET_FIELD_MAC(regValue[0],
        TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_QUEUE_EN_FIELD_OFFSET),
        TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_QUEUE_EN_FIELD_SIZE));

     queueAttributesPtr->agingThreshold = U32_GET_FIELD_MAC(regValue[0],
        TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_AGING_TH_FIELD_OFFSET),
        TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_AGING_TH_FIELD_SIZE));

     queueAttributesPtr->negativeCreditThreshold = U32_GET_FIELD_MAC(regValue[0],
        TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET),
        TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE));

     queueAttributesPtr->negativeCreditThreshold <<=PRV_TH_GRANULARITY_SHIFT;

     queueAttributesPtr->highCreditThreshold = U32_GET_FIELD_MAC(regValue[1],
        TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET)-32,
        TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE));

     queueAttributesPtr->highCreditThreshold <<=PRV_TH_GRANULARITY_SHIFT;

     size = 32 - TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET);

     queueAttributesPtr->lowCreditTheshold = 0;


     queueAttributesPtr->lowCreditTheshold= U32_GET_FIELD_MAC(regValue[0],
             TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET),
             size);

     value =  U32_GET_FIELD_MAC(regValue[1],
             0,
             TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE) - size);

     queueAttributesPtr->lowCreditTheshold|= ((value)<<size);

     queueAttributesPtr->lowCreditTheshold <<=PRV_TH_GRANULARITY_SHIFT;


     rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                                             tileNum,
                                             PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queueNumber],
                                             TXQ_SDQ_FIELD_GET(devNum, QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_OFFSET),
                                             TXQ_SDQ_FIELD_GET(devNum, QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE),
                                             &queueAttributesPtr->semiEligThreshold);

     if(rc==GT_OK)
     {
        queueAttributesPtr->semiEligThreshold<<= PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT;
     }


     return rc;


}


/**
* @internal prvCpssFalconTxqSdqLocalPortEnableSet function
* @endinternal
*
* @brief   Set port to enable state in SDQ
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] enable                   - If equal GT_TRUE port is enabled at SDQ,else otherwise
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqLocalPortEnableSet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNum,
 IN GT_BOOL enable
)
{
    GT_STATUS rc;
    GT_U32 selig4DryEnableBit;
    GT_U32 regValue = 0;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(localPortNum>= CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum))
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "local portNum[%d] must be less than [%d]",localPortNum,CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum));
    }
    /*reserved ports  can still be configured to disable*/
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E&&GT_TRUE==enable)
    {
        if(PRV_CPSS_TXQ_HARRIER_RESERVED_PORTS_BMP_MAC&(1<<localPortNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "local portNum[%d]considered reserved - can not be enabled.",localPortNum);\
        }
    }

    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_PORT_EN_0_FIELD_OFFSET),
                                      TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_PORT_EN_0_FIELD_SIZE),
                                      enable?1:0);

     U32_SET_FIELD_MASKED_MAC(regValue,TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_PORT_BP_EN_FIELD_OFFSET),
                                      TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_PORT_BP_EN_FIELD_SIZE),
                                      enable?1:0);

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
    {
      U32_SET_FIELD_MASKED_MAC(regValue,TXQ_IRONMAN_SDQ_PORT_CONFIG_PORT_GLBL_SLCT_EN_0_FIELD_OFFSET,
                                      TXQ_IRONMAN_SDQ_PORT_CONFIG_PORT_GLBL_SLCT_EN_0_FIELD_SIZE,
                                      enable?1:0);
    }


    if(GT_TRUE == enable)
    {

      U32_SET_FIELD_MASKED_MAC(regValue,TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_CRDT_IGN_0_FIELD_OFFSET),
                                       TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_CRDT_IGN_0_FIELD_SIZE),
                                       0);

     if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
     {
        /*check if the feature is enabled*/

        rc =prvCpssSip6TxqRegisterFieldRead(devNum,
             tileNum,
             PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].global_config,
             PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_CONFIG_SELIG4DRY_EN_FIELD_OFFSET),
             PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_CONFIG_SELIG4DRY_EN_FIELD_SIZE),
             &selig4DryEnableBit);

        if(rc!=GT_OK)
        {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " Error at   prvCpssSip6TxqRegisterFieldRead \n");
        }


         U32_SET_FIELD_MASKED_MAC(regValue,PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(devNum,PORT_CONFIG_QDRYER_SELIG_TH0_0_FIELD_OFFSET),
                                  PRV_SDQ_SIP_6_15_DEV_DEP_FIELD_GET_MAC(devNum,PORT_CONFIG_QDRYER_SELIG_TH0_0_FIELD_SIZE),
                                  (selig4DryEnableBit==1)?PRV_QDRYER_SELIG_TH>>PRV_QDRYER_GRANULARITY_SHIFT:0);

         U32_SET_FIELD_MASKED_MAC(regValue,PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(devNum,PORT_CONFIG_QDRYER_SELIG_TH1_0_FIELD_OFFSET),
                                  PRV_SDQ_SIP_6_15_DEV_DEP_FIELD_GET_MAC(devNum,PORT_CONFIG_QDRYER_SELIG_TH1_0_FIELD_SIZE),
                                  (selig4DryEnableBit==1)?PRV_QDRYER_SELIG_TH>>PRV_QDRYER_GRANULARITY_SHIFT:0);


     }
    }



    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Port_Config");
    if (rc != GT_OK)
    {
        return rc;
    }

    log.regIndex = localPortNum;

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Config[localPortNum],
                 0,32,regValue);

    return  rc;

}

/**
* @internal prvCpssFalconTxqSdqLocalPortEnableGet function
* @endinternal
*
* @brief   Get port to enable state in SDQ
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
*
* @param[out] enablePtr               - If equal GT_TRUE port is enabled at SDQ,else otherwise
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqLocalPortEnableGet
(
 IN GT_U8       devNum,
 IN GT_U32      tileNum,
 IN GT_U32      sdqNum,
 IN GT_U32      localPortNum,
 OUT GT_BOOL    * enablePtr
)
{

    GT_U32 value;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Config[localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_PORT_EN_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_PORT_EN_0_FIELD_SIZE),
                 &value);

     if(rc==GT_OK)
     {
        *enablePtr = (value==1)?GT_TRUE:GT_FALSE;
     }

    return rc;
}

/**
* @internal prvCpssFalconTxqSdqInitQueueThresholdAttributes function
* @endinternal
*
* @brief   Initialize queue thresholds for given speed
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] speed                   -  port speed
* @param[out] queueAttributesPtr     - (pointer to) queue thresholds
*
* @retval GT_OK               - on success.
* @retval GT_FAIL             - no thresholds are configured for this speed
*/
static GT_STATUS prvCpssFalconTxqSdqInitQueueThresholdAttributes
(
    GT_U32                                          devNum,
    IN  CPSS_PORT_SPEED_ENT                         speed,
    IN  PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES * queueAttributesPtr
)
{
    GT_STATUS rc = GT_OK;

    queueAttributesPtr->agingThreshold = 0x0;
    queueAttributesPtr->negativeCreditThreshold = 0;
    queueAttributesPtr->lowCreditTheshold = 149000;
    queueAttributesPtr->highCreditThreshold = 150000;

    if(speed == CPSS_PORT_SPEED_NA_E)
    {
         /*Default initialization on power up. The speed is undefined yet*/
         queueAttributesPtr->semiEligThreshold =  0;
         queueAttributesPtr->negativeCreditThreshold = 0;
         queueAttributesPtr->lowCreditTheshold = (0x1FF) << PRV_TH_GRANULARITY_SHIFT;
         queueAttributesPtr->highCreditThreshold = (0x3FF) << PRV_TH_GRANULARITY_SHIFT;
    }
    else
    {
        rc = prvCpssSip6TxqSdqSemiEligThresholdGet(devNum,speed,&(queueAttributesPtr->semiEligThreshold));
        if(rc !=GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqSdqInitQueueAttributes failed\n");
        }
    }
    return rc;
}

/**
* @internal prvCpssFalconTxqSdqInitQueueThresholdAttributes function
* @endinternal
*
* @brief   Initialize port thresholds for given speed
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] speed                   - port speed
* @param[out] portAttributesPtr      -(pointer to) port thresholds
*
* @retval GT_OK                - on success.
* @retval GT_FAIL              - no thresholds are configured for this speed
*/
static GT_STATUS prvCpssFalconTxqSdqInitPortAttributes
(
    IN  CPSS_PORT_SPEED_ENT                         speed,
    IN  PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES  * portAttributesPtr
)
{
    GT_STATUS rc = GT_OK;


    switch(speed)
    {
        case CPSS_PORT_SPEED_10_E:
        case CPSS_PORT_SPEED_100_E:
        case CPSS_PORT_SPEED_1000_E:
            portAttributesPtr->portBackPressureLowThreshold = 14020;
            break;
        case CPSS_PORT_SPEED_10000_E:
            portAttributesPtr->portBackPressureLowThreshold =  14200;
            break;
        case CPSS_PORT_SPEED_25000_E:
            portAttributesPtr->portBackPressureLowThreshold =  14500;
            break;
        case CPSS_PORT_SPEED_40000_E:
        case CPSS_PORT_SPEED_42000_E:
            portAttributesPtr->portBackPressureLowThreshold =  14800;
            break;
        case CPSS_PORT_SPEED_47200_E:
        case CPSS_PORT_SPEED_50000_E:
        case CPSS_PORT_SPEED_53000_E:
            portAttributesPtr->portBackPressureLowThreshold = 15000;
            break;
        case CPSS_PORT_SPEED_100G_E:
            portAttributesPtr->portBackPressureLowThreshold = 16000;
            break;
        case CPSS_PORT_SPEED_102G_E:
        case CPSS_PORT_SPEED_106G_E:
        case CPSS_PORT_SPEED_107G_E:
            portAttributesPtr->portBackPressureLowThreshold = 16140;
            break;
        case CPSS_PORT_SPEED_200G_E:
            portAttributesPtr->portBackPressureLowThreshold = 18000;
            break;
        case CPSS_PORT_SPEED_212G_E:
            portAttributesPtr->portBackPressureLowThreshold = 18280;
            break;
        case CPSS_PORT_SPEED_400G_E:
            portAttributesPtr->portBackPressureLowThreshold =  22000;
            break;
        case CPSS_PORT_SPEED_424G_E:
            portAttributesPtr->portBackPressureLowThreshold =  22560;
            break;
        case CPSS_PORT_SPEED_2500_E:
            portAttributesPtr->portBackPressureLowThreshold =  14050;
            break;
        case CPSS_PORT_SPEED_5000_E:
            portAttributesPtr->portBackPressureLowThreshold =  14100;
            break;
        case CPSS_PORT_SPEED_20000_E:
            portAttributesPtr->portBackPressureLowThreshold =  14400;
            break;
        default :
            rc = GT_FAIL;
            break;
    }

    portAttributesPtr->portBackPressureHighThreshold = portAttributesPtr->portBackPressureLowThreshold;

    return rc;
}

/**
* @internal prvCpssFalconTxqSdqPortAttributesSet function
* @endinternal
*
* @brief   Get port thresholds
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
*
* @param[in] speed                 - port speed
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqPortAttributesSet
(
    IN GT_U8                devNum,
    IN GT_U32               tileNum,
    IN GT_U32               sdqNum,
    IN GT_U32               localPortNum,
    IN CPSS_PORT_SPEED_ENT  speed
)
{
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES  portAttributes;

    GT_STATUS rc ;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);


    rc = prvCpssFalconTxqSdqInitPortAttributes(speed,&portAttributes);

    if(rc !=GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqSdqInitQueueAttributes failed\n");
    }


    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Port_Back_Pressure_Low_Threshold");
    if (rc != GT_OK)
    {
        return rc;
    }

    log.regIndex = localPortNum;


    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Back_Pressure_Low_Threshold[localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_LOW_THRESHOLD_PORT_BP_LOW_TH_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_LOW_THRESHOLD_PORT_BP_LOW_TH_FIELD_SIZE),
                 portAttributes.portBackPressureLowThreshold);

    if(rc !=GT_OK)
    {
       return rc;
    }

    log.regName = "Port_Back_Pressure_High_Threshold";

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                     tileNum,&log,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Back_Pressure_High_Threshold[localPortNum],
                     TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_HIGH_THRESHOLD_PORT_BP_HIGH_TH_FIELD_OFFSET),
                     TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_HIGH_THRESHOLD_PORT_BP_HIGH_TH_FIELD_SIZE),
                     portAttributes.portBackPressureHighThreshold);


      return rc;
}

/**
* @internal prvCpssFalconTxqSdqPortAttributesGet function
* @endinternal
*
* @brief   Get port thresholds
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
*
* @param[out] portAttributesPtr       - (pointer to) port thresholds
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqPortAttributesGet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       localPortNum,
    OUT PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES  * portAttributesPtr
)
{
       GT_STATUS rc ;

      TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

       TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

       TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);


        rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Back_Pressure_Low_Threshold[localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_LOW_THRESHOLD_PORT_BP_LOW_TH_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_LOW_THRESHOLD_PORT_BP_LOW_TH_FIELD_SIZE),
                 &(portAttributesPtr->portBackPressureLowThreshold));

     if(rc !=GT_OK)
     {
        return rc;
     }

     rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                     tileNum,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Back_Pressure_High_Threshold[localPortNum],
                     TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_HIGH_THRESHOLD_PORT_BP_HIGH_TH_FIELD_OFFSET),
                     TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_HIGH_THRESHOLD_PORT_BP_HIGH_TH_FIELD_SIZE),
                     &(portAttributesPtr->portBackPressureHighThreshold));


      return rc;
}


GT_STATUS prvCpssFalconTxqSdqQueueStrictPrioritySet
(
    IN GT_U8    devNum,
    IN GT_U32   tileNum,
    IN GT_U32   sdqNum,
    IN GT_U32   queueNumber,
    IN GT_BOOL  sp
)
{
     GT_STATUS rc;
     PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
     {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
     }


     rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"queue_config");
     if (rc != GT_OK)
     {
         return rc;
     }

     log.regIndex = queueNumber;

     rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queueNumber],
                 TXQ_SDQ_FIELD_GET(devNum,QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE),
                 sp?1:0);

     return rc;


}

/**
* @internal prvCpssFalconTxqSdqQueueStrictPriorityGet function
* @endinternal
*
* @brief   Get queue strict priority bit
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] queueNumber           - local queue number  (APPLICABLE RANGES:0..399).
*
* @param[out] spPtr                - (pointer to)strict priority bit
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqQueueStrictPriorityGet
(
    IN GT_U8    devNum,
    IN GT_U32   tileNum,
    IN GT_U32   sdqNum,
    IN GT_U32   queueNumber,
    IN GT_BOOL  *spPtr
)
{
     GT_STATUS rc;
     GT_U32 tmp;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
     {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
     }

     rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queueNumber],
                 TXQ_SDQ_FIELD_GET(devNum,QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE),
                 &tmp);

     *spPtr= tmp?GT_TRUE:GT_FALSE;

     return rc;


}


GT_STATUS prvCpssFalconTxqSdqSelectListSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       localPortNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE    * rangePtr
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);


    if(rangePtr->prio0LowLimit >=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(rangePtr->prio0HighLimit >=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if((rangePtr->prio0LowLimit >rangePtr->prio0HighLimit)||
       (rangePtr->prio1LowLimit >rangePtr->prio1HighLimit))
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Sel_List_Range_Low_0");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = localPortNum;

    rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_Low[0][localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE),
                 rangePtr->prio0LowLimit);

     if(rc!=GT_OK)
     {
        return rc;
     }

    log.regName ="Sel_List_Range_High_0";

    rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_High[0][localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE),
                 rangePtr->prio0HighLimit);

     if(rc!=GT_OK)
     {
        return rc;
     }

     log.regName ="Sel_List_Range_Low_1";

     rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_Low[1][localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE),
                 rangePtr->prio1LowLimit);

     if(rc!=GT_OK)
     {
        return rc;
     }

     log.regName ="Sel_List_Range_High_1";

     rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_High[1][localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE),
                 rangePtr->prio1HighLimit);

     return rc;
}


/**
* @internal prvCpssFalconTxqSdqSelectListGet function
* @endinternal
*
* @brief   Initialize queue thresholds for given speed
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] tileNum           - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum            - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum      - local port (APPLICABLE RANGES:0..8).
* @param[in] range             - the  range of the queues
*
* @retval GT_OK                - on success.
* @retval GT_FAIL              - no thresholds are configured for this speed
*/
GT_STATUS prvCpssFalconTxqSdqSelectListGet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       localPortNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE    * rangePtr
)
{
        GT_STATUS rc;

         TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

         TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

         TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

        rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_Low[0][localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE),
                 &(rangePtr->prio0LowLimit));

         if(rc!=GT_OK)
         {
            return rc;
         }


        rc =prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_High[0][localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE),
                 &(rangePtr->prio0HighLimit));

         if(rc!=GT_OK)
         {
            return rc;
         }


         rc =prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_Low[1][localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE),
                 &(rangePtr->prio1LowLimit));

         if(rc!=GT_OK)
         {
            return rc;
         }


         rc =prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_High[1][localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE),
                 &(rangePtr->prio1HighLimit));

         return rc;
}


GT_STATUS prvCpssFalconTxqSdqQCreditBalanceGet
(
    IN GT_U8        devNum,
    IN GT_U32       tileNum,
    IN GT_U32       sdqNum,
    IN GT_U32       queueNum,
    OUT  GT_U32     * balancePtr
)
{
    GT_STATUS rc;

     rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        TXQ_SDQ_FIELD_GET(devNum,QUEUE_CRDT_BLNC_CRDT_BLNC_FIELD_OFFSET),
                                        TXQ_SDQ_FIELD_GET(devNum,QUEUE_CRDT_BLNC_CRDT_BLNC_FIELD_SIZE),
                                        balancePtr);

     return rc;


}

GT_STATUS prvCpssFalconTxqSdqPortCreditBalanceGet
(
    IN GT_U8     devNum,
    IN GT_U32    tileNum,
    IN GT_U32    sdqNum,
    IN GT_U32    portNum,
    OUT  GT_U32  * balancePtr,
    OUT  GT_U32  * qCountPtr
)
{
    GT_STATUS rc;

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        TXQ_SDQ_PORT_CRDT_BLNC_PORT_BLNC_FIELD_OFFSET,
                                        TXQ_SDQ_PORT_CRDT_BLNC_PORT_BLNC_FIELD_SIZE,
                                        balancePtr);
    if(rc!=GT_OK)
    {
        return rc;
    }

   rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        TXQ_SDQ_PORT_CRDT_BLNC_PORT_QCNT_FIELD_OFFSET,
                                        TXQ_SDQ_PORT_CRDT_BLNC_PORT_QCNT_FIELD_SIZE,
                                        qCountPtr);


    return rc;


}


GT_STATUS prvCpssFalconTxqSdqEligStateGet
(
    IN GT_U8        devNum,
    IN GT_U32       tileNum,
    IN GT_U32       sdqNum,
    IN GT_U32       queueNum,
    OUT  GT_U32     * eligStatePtr
)
{
    GT_STATUS rc;

    rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNum,
                                        eligStatePtr);

    return rc;
}

/**
* @internal prvCpssFalconTxqSdqPfcControlEnableSet function
* @endinternal
*
* @brief   Enable /Disable  PFC response per port/TC.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset           - queue offset[0..15]
* @param[in] enable                - port/tc PFC responce enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32  queueOffset,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueOffset>=1<<TXQ_SDQ_PFC_CONTROL_TC_EN0_FIELD_SIZE)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNumber);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"PFC_Control");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = localPortNumber;


    rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].PFC_Control[localPortNumber],
                 TXQ_SDQ_FIELD_GET(devNum,PFC_CONTROL_TC_EN0_FIELD_OFFSET)+queueOffset,
                 1,
                 enable?1:0);

    return rc;
}

/**
* @internal prvCpssFalconTxqSdqPfcControlEnableGet function
* @endinternal
*
* @brief   Get enable /Disable  PFC response per port/TC.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset           - queue offset[0..15]
* @param[in] enablePtr             - (pointer to)port/tc PFC responce enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableGet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32  queueNumber,
    IN GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    GT_U32          regValue;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber>=(GT_U32)1<<TXQ_SDQ_FIELD_GET(devNum,PFC_CONTROL_TC_EN0_FIELD_SIZE))
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


   rc =prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].PFC_Control[localPortNumber],
                 TXQ_SDQ_FIELD_GET(devNum,PFC_CONTROL_TC_EN0_FIELD_OFFSET)+queueNumber,
                 1,
                 &regValue);

     if(rc==GT_OK)
     {
        *enablePtr = (regValue == 1)?GT_TRUE:GT_FALSE;
     }

     return rc;
}

/**
* @internal prvCpssFalconTxqSdqQueueTcSet function
* @endinternal
*
* @brief   Sets PFC TC to queue map. Meaning this table define
*          which Q should be paused on reception of perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset              - queue offset[0..15]
* @param[in] tc                       - tc that is mapped to.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqSdqQueueTcSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNumber,
    IN GT_U32 tc
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(tc>= (1<<TXQ_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"queue_config");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = queueNumber;

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queueNumber],
                 TXQ_SDQ_FIELD_GET(devNum,QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE),
                 tc);


}

/**
* @internal prvCpssFalconTxqSdqPfcControlEnableBitmapSet function
* @endinternal
*
* @brief   Enable /Disable  PFC response per port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset           - queue offset[0..15]
* @param[in] enableBtmp            - port PFC responce enable bitmap option.(each bit represent queue)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableBitmapSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32  enableBtmp
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);


    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"PFC_Control");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = localPortNumber;

    rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].PFC_Control[localPortNumber],
                 0,
                 32,
                 enableBtmp);

    return rc;
}

/**
* @internal prvCpssFalconTxqSdqPfcControlEnableBitmapGet  function
* @endinternal
*
* @brief   Get Enable /Disable  PFC response per port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset           - queue offset[0..15]
* @param[in] enableBtmpPtr         - (pointer to)port PFC responce enable bitmap option.(each bit represent queue)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableBitmapGet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32 *enableBtmpPtr
)
{
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc =prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].PFC_Control[localPortNumber],
                 0,
                 32,
                 enableBtmpPtr);

    return rc;
}

/**
* @internal prvCpssFalconTxqSdqLocalPortFlushSet function
* @endinternal
*
* @brief  Set port to "credit ignore" mode.This make port to transmit whenever there is data to send ignoring credits.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] enable                - Enable/disable "credit ignore" mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqSdqLocalPortFlushSet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNum,
 IN GT_BOOL enable
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Port_Config");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = localPortNum;

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Config[localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_CRDT_IGN_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_CRDT_IGN_0_FIELD_SIZE),
                 enable?1:0);

    return  rc;
}

/**
* @internal prvCpssFalconTxqSdqQueueStatusDump function
* @endinternal
*
* @brief  Dump queue eligeble status in parsed format
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - physical device number
* @param[in] tileNum          - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum           - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] startQ           - first queue to dump
* @param[in] size             - number of queues to dump.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqSdqQueueStatusDump
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNumber,
    IN  GT_U32                  sdqNum,
    IN  GT_U32                  startQ,
    IN  GT_U32                  size,
    IN  CPSS_OS_FILE_TYPE_STC   * file
)
{
    GT_U32           i;
    GT_STATUS        rc;
    GT_U32 eligState,credit,qcn,dry,inPortCnt,InSel,enq,SemiElig;
    PRV_CPSS_TXQ_UTILS_CHECK_REDIRECT_MAC(file);


    cpssOsFprintf(file->fd,"\n queue_elig_state tile %d dp %d\n",tileNumber,sdqNum);

    cpssOsFprintf(file->fd,"\n+-------------+------+---+--------+---------+-------+----+--------+");
    cpssOsFprintf(file->fd,"\n| queueNumber |credit|QCN|  DRY   |inPortCnt|InSel  |ENQ |SemiElig|");
    cpssOsFprintf(file->fd,"\n+-------------+------+---+--------+---------+-------+----+--------+");


    for(i =startQ;i<(startQ+size);i++)
    {

       rc = prvCpssFalconTxqSdqEligStateGet(devNum,tileNumber,sdqNum,i,&eligState);

        if(rc!=GT_OK)
        {
            cpssOsPrintf("%d = prvCpssFalconTxqSdqQueueAttributesGet\n",rc );
            return rc;
        }

        credit = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_CRDT_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_CRDT_FIELD_SIZE));

        qcn = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_QCN_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_QCN_FIELD_SIZE));

        dry = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_DRY_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_DRY_FIELD_SIZE));

        inPortCnt = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_INPORTCNT_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_INPORTCNT_FIELD_SIZE));

        InSel =  U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_INSEL_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_INSEL_FIELD_SIZE));

        enq = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_ENQ_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_ENQ_FIELD_SIZE));

        SemiElig = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_SELIG_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_SELIG_FIELD_SIZE));

        cpssOsFprintf(file->fd,"\n|%13d|%6d|%2d|%8d|%10d|%7d|%4d|%8d|",i,credit,qcn,dry,inPortCnt,InSel,enq,SemiElig);
        cpssOsFprintf(file->fd,"\n+-------------+------+---+--------+---------+-------+----+--------+");

    }

    cpssOsFprintf(file->fd,"\n");

    return GT_OK;
}

/**
* @internal prvCpssFalconTxqSdqSemiEligEnableSet function
* @endinternal
*
* @brief   Enable/disable semi elig feature
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] seligEnable              - if GT_TRUE semi elig feature is enabled, otherwise disabled
* @param[in] selig4DryEnable          - if GT_TRUE Enable selection slow down when semi elig by entering queue to the qdryer after selection,
*                                       otherwise disabled
* @param[in] granularity1kEnable      - Change Semi Elig threshold granule to 1KB. In that case, Semi Elig Threshold range will be 1KB-128KB
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqSemiEligEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_BOOL seligEnable,
    IN GT_BOOL selig4DryEnable,
    IN GT_BOOL granularity1kEnable
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"global_config");
    if (rc != GT_OK)
    {
      return rc;
    }

    rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].global_config,
                 TXQ_SDQ_FIELD_GET(devNum,GLOBAL_CONFIG_SELIG_EN_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,GLOBAL_CONFIG_SELIG_EN_FIELD_SIZE),
                 seligEnable?1:0);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqSemiEligEnableSet  failed \n");
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
    {
       rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
            tileNum,&log,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].global_config,
            PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_CONFIG_SELIG4DRY_EN_FIELD_OFFSET),
            PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_CONFIG_SELIG4DRY_EN_FIELD_SIZE),
            selig4DryEnable?1:0);

       if(rc!=GT_OK)
       {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqSemiEligEnableSet  failed \n");
       }

       if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum) == GT_TRUE)
       {
           rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                tileNum,&log,
                PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].global_config,
                PRV_SDQ_SIP_6_15_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_CONFIG_SELIG_1K_EN_FIELD_OFFSET),
                PRV_SDQ_SIP_6_15_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_CONFIG_SELIG_1K_EN_FIELD_SIZE),
                granularity1kEnable?1:0);

           if(rc!=GT_OK)
           {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqSemiEligEnableSet  failed \n");
           }
       }

     }

     return rc;
 }

 /**
 * @internal prvCpssSip6_10TxqSdqSelectListEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable select list range
 *
 * @note   APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 *
 * @param[in] devNum                - device number
 * @param[in] tileNum               - Then number of tile (Applicable range 0..3)
 * @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
 * @param[in] localPortNum          - local port number  (APPLICABLE RANGES:0..26).
 * @param[in] lowPriority           - Defines the range.If equal GT_TRUE then range=0 ,else range=1.
 * @param[in] enable                - Enable/Disable select list range(0,1)
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
 GT_STATUS prvCpssSip6_10TxqSdqSelectListEnableSet
 (
     IN GT_U8   devNum,
     IN GT_U32  tileNum,
     IN GT_U32  sdqNum,
     IN GT_U32  localPortNum,
     IN GT_BOOL lowPriority,
     IN GT_BOOL enable
 )
 {
     GT_STATUS rc;
     GT_U32    index0;
     PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

     /*Supported only from SIP 6.10*/
     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
        return GT_OK;
     }

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

     index0 =lowPriority?0:1;

     rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,lowPriority?"Sel_List_Enable_0":"Sel_List_Enable_1");
     if (rc != GT_OK)
     {
       return rc;
     }

     log.regIndex = localPortNum;

     rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Enable[index0][localPortNum],
                 TXQ_SDQ_FIELD_GET_ADV(devNum,SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET_ADV(devNum,SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_SIZE),
                 BOOL2BIT_MAC(enable));

      return rc;
 }

 /**
  * @internal prvCpssFalconTxqSdqSelectListEnableGet function
  * @endinternal
  *
  * @brief   Get enable/disable select list range
  *
  * @note   APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman
  * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
  *
  * @param[in] devNum                - device number
  * @param[in] tileNum               - Then number of tile (Applicable range 0..3)
  * @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
  * @param[in] localPortNum          - local port number  (APPLICABLE RANGES:0..26).
  * @param[in] lowPriority           - Defines the range.If equal GT_TRUE then range=0 ,else range=1.
  * @param[out] enablePtr            -(pointer to)Enable/Disable select list range(0,1)
  *
  * @retval GT_OK                    - on success.
  * @retval GT_BAD_PARAM             - wrong sdq number.
  * @retval GT_HW_ERROR              - on writing to HW error.
  * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
  */
 GT_STATUS prvCpssFalconTxqSdqSelectListEnableGet
 (
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  sdqNum,
     IN  GT_U32  localPortNum,
     IN  GT_BOOL lowPriority,
     OUT GT_BOOL *enablePtr
 )
 {
     GT_STATUS rc;
     GT_U32    value;
     GT_U32    index0;

     CPSS_NULL_PTR_CHECK_MAC(enablePtr);

     /*Supported only from SIP 6.10*/
     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
        return GT_OK;
     }

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

     index0 =lowPriority?0:1;

     rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Enable[index0][localPortNum],
                TXQ_SDQ_FIELD_GET_ADV(devNum,SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_OFFSET),
                TXQ_SDQ_FIELD_GET_ADV(devNum,SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_SIZE),
                 &value);

     *enablePtr = BIT2BOOL_MAC(value);

     return rc;
 }


 GT_STATUS prvCpssFalconTxqSdqSelectListPtrGet
 (
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  sdqNum,
     IN  GT_U32  ind,
     OUT GT_U32 *valuePtr
 )
 {
     GT_STATUS rc;

     CPSS_NULL_PTR_CHECK_MAC(valuePtr);

     /*Supported only from SIP 6.10*/
     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
        return GT_OK;
     }

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     rc = prvCpssDxChReadTableEntry(devNum,
                                     CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_PTRS_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                     ind,
                                     valuePtr);


     return rc;
 }

/**
* @internal prvCpssSip6TxqRegisterFieldWrite function
* @endinternal
*
* @brief   Function write to SDQ register.Also check that register is defined add tile offset
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] regAddr               - Adress of the register in SDQ
* @param[in] fieldOffset           - The start bit number in the register.
* @param[in] fieldLength           - The number of bits to be written to register.
* @param[in] fieldData             - Data to be written into the register.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqRegisterFieldWrite
(
     IN  GT_U8    devNum,
     IN  GT_U32   tileNum,
     IN PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC * logDataPtr,
     IN  GT_U32   regAddr,
     IN  GT_U32   fieldOffset,
     IN  GT_U32   fieldLength,
     IN  GT_U32   fieldData
)
{
    GT_U32 tileOffset;
    GT_U32 mask = 0xFFFFFFFF;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED==regAddr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Adress is unitnitialized. \n");
    }

    if(fieldLength==0||fieldLength>32)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Incorrect field length %d \n",fieldLength);
    }
    if(fieldOffset>31)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Incorrect field offset  %d \n",fieldOffset);
    }


    regAddr+=tileOffset;

    if(NULL!=logDataPtr&&GT_TRUE==logDataPtr->log)
    {
        if(fieldLength<32)
        {
            mask=(1<<fieldLength)-1;
            mask<<=fieldOffset;
        }

         cpssOsPrintf("\nWR  [addr = 0x%08x] REG TILE %d  %s       ",
            regAddr,tileNum,logDataPtr->unitName);

        if(PRV_TXQ_LOG_NO_INDEX != logDataPtr->unitIndex)
        {
            cpssOsPrintf("%d,",
            logDataPtr->unitIndex);
        }

        cpssOsPrintf(" %s",logDataPtr->regName);

        if(PRV_TXQ_LOG_NO_INDEX != logDataPtr->regIndex)
        {
            cpssOsPrintf("_%d",logDataPtr->regIndex);
        }

        cpssOsPrintf(" 0x%08x mask 0x%08x\n",fieldData<<fieldOffset,mask);


    }

    return prvCpssHwPpSetRegField(devNum,regAddr,fieldOffset,fieldLength,fieldData);
}


/**
* @internal prvCpssSip6TxqRegisterFieldRead function
* @endinternal
*
* @brief   Function read from  SDQ register.Also check that register is defined add tile offset.
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] regAddr               - Adress of the register in SDQ
* @param[in] fieldOffset           - The start bit number in the register.
* @param[in] fieldLength           - The number of bits to be read.
*
* @param[out] fieldDataPtr         - (pointer to) Data to read from the register.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqRegisterFieldRead
(
     IN   GT_U8    devNum,
     IN   GT_U32   tileNum,
     IN   GT_U32   regAddr,
     IN   GT_U32   fieldOffset,
     IN   GT_U32   fieldLength,
     OUT  GT_U32   *fieldDataPtr
)
{
    GT_U32 tileOffset;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED==regAddr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Adress is unitnitialized. \n");
    }

    if(fieldLength==0||fieldLength>32)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Incorrect field length %d \n",fieldLength);
    }
    if(fieldOffset>31)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Incorrect field offset  %d \n",fieldOffset);
    }


    regAddr+=tileOffset;

    return prvCpssHwPpGetRegField(devNum,regAddr,fieldOffset,fieldLength,fieldDataPtr);
}
GT_STATUS prvCpssSip6TxqWriteTableEntry
(
    IN GT_U8                  devNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC *logDataPtr,
    IN CPSS_DXCH_TABLE_ENT    tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                  *entryValuePtr

)
{
    GT_U32 i,numOfWords;
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *tablePtr;

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);
    tablePtr = (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->writeTablePtr);

    numOfWords = tableInfoPtr->entrySize;

    if(NULL!=logDataPtr&&GT_TRUE==logDataPtr->log)
    {
        cpssOsPrintf("\nWR  [addr = 0x%08x] MEM TILE %d  %s       ",
            tablePtr->baseAddress+(tablePtr->step*entryIndex),
            logDataPtr->tileIndex,logDataPtr->unitName);

        if(PRV_TXQ_LOG_NO_INDEX != logDataPtr->unitIndex)
        {
            cpssOsPrintf("%d,",
            logDataPtr->unitIndex);
        }

        cpssOsPrintf(" %s",logDataPtr->tableName);

        if(PRV_TXQ_LOG_NO_INDEX != logDataPtr->entryIndex)
        {
            cpssOsPrintf(" %d",logDataPtr->entryIndex);
        }

        for(i=0;i<numOfWords;i++)
        {
            cpssOsPrintf(" 0x%08x",*(entryValuePtr+i));
        }

        cpssOsPrintf(" mask 0xFFFFFFFF\n");
    }
    return prvCpssDxChWriteTableEntry(devNum,tableType,entryIndex,
       entryValuePtr);
}


GT_STATUS prvCpssPhoenixTxqSdqPortPfcStateGet
(
    IN GT_U8    devNum,
    IN GT_U32   localPortNum,
    IN GT_U32   *statePtr
)
{
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

     /*Supported only from SIP 6.15*/
     if(GT_FALSE == PRV_CPSS_TXQ_LIKE_PHOENIX_MAC(devNum))
     {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
     }

     rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E,
                                        localPortNum,
                                        statePtr);
     return rc;

}


static GT_STATUS prvCpssSip6TxqSdqSemiEligThresholdGet
(
     IN  GT_U8                          devNum,
     IN  CPSS_PORT_SPEED_ENT            speed,
     OUT GT_U32                         *thresholdPtr
)
{
    GT_U32    minSemiElig = 0,speed_in_K;
    GT_FLOAT semiEligTime,speed_in_G;

    CPSS_NULL_PTR_CHECK_MAC(thresholdPtr);


    speed_in_K = prvCpssCommonPortSpeedEnumToMbPerSecConvert(speed);
    speed_in_G = speed_in_K/1000.0;


     if(GT_TRUE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
        semiEligTime = 0.6;
     }
     else if(GT_TRUE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
         semiEligTime = 0.7;
         minSemiElig =_8K;
     }
     else
     {
        semiEligTime = 0.6;
     }

     /*From architectural defenition:
                      SEMI_ELIGE_TIME = 0.6  (AC5X =0.7)

               [@[PDQ P Level Speed]]*SEMI_ELIGE_TIME/(8*1000)) + min semi elig*/

     *thresholdPtr = (GT_U32)((((speed_in_G*semiEligTime)/8)*1000) + minSemiElig);

     return GT_OK;

}

/**
* @internal prvCpssSip6TxqSdqDebugInterruptDisableSet function
* @endinternal
*
* @brief   Debug function that disable iterrupt in SDQ
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqSdqDebugInterruptDisableSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"SDQ_Interrupt_Summary_Mask");
    if (rc != GT_OK)
    {
      return rc;
    }

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].SDQ_Interrupt_Summary_Mask,
                 0,32,0);
}
/**
* @internal prvCpssSip6TxqSdqDebugInterruptGet function
* @endinternal
*
* @brief   Get interrupt cause for SDQ
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqSdqDebugInterruptGet
(
    IN GT_U8    devNum,
    IN GT_U32   tileNum,
    IN GT_U32   sdqNum,
    OUT GT_U32  *functionalPtr,
    OUT GT_U32  *debugPtr
)
{
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(functionalPtr);
    CPSS_NULL_PTR_CHECK_MAC(debugPtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].SDQ_Interrupt_Functional_Cause,
                 0,32,functionalPtr);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].SDQ_Interrupt_Debug_Cause,
                 0,32,debugPtr);

    return rc;
}

GT_U32 prvCpssSip6TxqSdqUnMapPortErrorBmp
(
    IN GT_U8  devNum
)
{
   GT_U32 bitOffset;
   GT_U32 bmp=0;

   bitOffset = TXQ_SDQ_FIELD_GET(devNum,SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_ENQ_FIELD_OFFSET);
   bmp =  1<<bitOffset;
   bitOffset = TXQ_SDQ_FIELD_GET(devNum,SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_DEQ_FIELD_OFFSET);
   bmp |= (1<<bitOffset);

   return bmp;
}

/**
* @internal prvCpssSip6TxqSdqErrorCaptureGet function
* @endinternal
*
* @brief  Capture SDQ errors
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               -  physical device number
* @param[in] tileNum              -  the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum               -  the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] selectErrorValidPtr  -  (pointer to)GT_TRUE if select error is valid,GT_FALSE otherwise
* @param[in] selectErrorPortPtr   -  (pointer to) port that caused select error
* @param[in] selectErrorPortPtr   -  (pointer to) elig error raw format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6TxqSdqErrorCaptureGet
(
 IN  GT_U8      devNum,
 IN  GT_U32     tileNum,
 IN  GT_U32     sdqNum,
 OUT GT_BOOL    *selectErrorValidPtr,
 OUT GT_U32     *selectErrorPortPtr,
 OUT GT_U32     *eligPtr
)
{
    GT_STATUS rc;
    GT_U32    data;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    CPSS_NULL_PTR_CHECK_MAC(selectErrorValidPtr);
    CPSS_NULL_PTR_CHECK_MAC(selectErrorPortPtr);
    CPSS_NULL_PTR_CHECK_MAC(eligPtr);


    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Select_Func_Error_Capture,
                 0,
                 32,
                 &data);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
    }

     *selectErrorPortPtr =  U32_GET_FIELD_MAC(data,
            TXQ_SDQ_FIELD_GET(devNum,SELECT_FUNC_ERROR_CAPTURE_SEL_ERROR_PORT_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,SELECT_FUNC_ERROR_CAPTURE_SEL_ERROR_PORT_FIELD_SIZE));

     *selectErrorValidPtr =  U32_GET_FIELD_MAC(data,
            TXQ_SDQ_FIELD_GET(devNum,SELECT_FUNC_ERROR_CAPTURE_SEL_ERROR_VALID_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,SELECT_FUNC_ERROR_CAPTURE_SEL_ERROR_VALID_FIELD_SIZE));

     rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Elig_Func_Error_Capture,
                 0,
                 32,
                 eligPtr);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
    }

    return  rc;
}

GT_STATUS prvCpssSip6TxqSdqErrorCaptureDump
(
 IN  GT_U8 devNum,
 IN  GT_U32 tileNum,
 IN  GT_U32 sdqNum
)
{
   GT_BOOL selectErrorValid;
   GT_U32  selectPort,eligError;
   GT_STATUS rc;

   TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
   TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

   rc = prvCpssSip6TxqSdqErrorCaptureGet (devNum,tileNum,sdqNum,&selectErrorValid,&selectPort,&eligError);
   if(rc!=GT_OK)
   {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
   }

   cpssOsPrintf("selectErrorValid %d , selectPort %d ,elig error 0x%08x\n",selectErrorValid,selectPort,eligError);

   return GT_OK;
}

/**
* @internal prvCpssSip6_30TxqSdqQbvScanEnableSet function
* @endinternal
*
* @brief   Set QBV scan
*
* @note   APPLICABLE DEVICES: Ironman L
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
* @param[in] enable                -enable/disable qbv scan
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6_30TxqSdqQbvScanEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_BOOL enable
)
{
     GT_STATUS rc = GT_OK;
     PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

     if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
     {

         TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
         TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);


         rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"global_config");
         if (rc != GT_OK)
         {
           return rc;
         }

         rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                     tileNum,&log,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].global_config,
                     TXQ_IRONMAN_SDQ_GLOBAL_CONFIG_QBV_SCAN_EN_FIELD_OFFSET,
                     TXQ_IRONMAN_SDQ_GLOBAL_CONFIG_QBV_SCAN_EN_FIELD_SIZE,
                     enable?1:0);

         if(rc!=GT_OK)
         {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqQbvScanEnableSet  failed \n");
         }

     }

     return rc;
 }
 /**
 * @internal prvCpssSip6TxqSdqLocalPortConfigGet function
 * @endinternal
 *
 * @brief   Get port configuration in raw format
 *
 * @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                - device number
 * @param[in] tileNum               - The number of tile (Applicable range 0..3)
 * @param[in] sdqNum                - data path index
 * @param[out] dataPtr                -(pointer to)value of port configuration register
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
 GT_STATUS prvCpssSip6TxqSdqLocalPortConfigGet
 (
  IN GT_U8       devNum,
  IN GT_U32      tileNum,
  IN GT_U32      sdqNum,
  IN GT_U32      localPortNum,
  OUT GT_U32    * dataPtr
 )
 {

     GT_STATUS rc;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);
     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     CPSS_NULL_PTR_CHECK_MAC(dataPtr);

     rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                  tileNum,
                  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Config[localPortNum],
                  0,
                  32,
                  dataPtr);

     return rc;
 }



/**
* @internal prvCpssSip6_30TxqSdqLocalPortPreemptionTypeSet function
* @endinternal
*
* @brief   Set preemption type for SDQ port(required for hold/release feature configuration)
*
* @note   APPLICABLE DEVICES: Ironman L
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
* @param[in] preemptive                - act as preemptive channel
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6_30TxqSdqLocalPortPreemptionTypeSet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNum,
 IN GT_BOOL preemptive
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
    {
       rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Port_Config");
       if (rc != GT_OK)
       {
         return rc;
       }

       log.regIndex = localPortNum;

       rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                     tileNum,&log,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Config[localPortNum],
                     TXQ_IRONMAN_SDQ_PORT_CONFIG_PORT_TYPE_PRMPT_0_FIELD_OFFSET,
                     TXQ_IRONMAN_SDQ_PORT_CONFIG_PORT_TYPE_PRMPT_0_FIELD_SIZE,preemptive?1:0);
    }

    return rc;
}


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
 * @file prvCpssDxChTxqPdx.c
 *
 * @brief CPSS SIP6 TXQ PDX low level configurations.
 *
 * @version   1
 ********************************************************************************
 */
/*#define CPSS_LOG_IN_MODULE_ENABLE - do not log currently*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/falcon/central/prvCpssFalconTxqPdxRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5p/central/prvCpssAc5pTxqPdxRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5x/central/prvCpssAc5xTxqPdxRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/harrier/central/prvCpssHarrierTxqPdxRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ironman/central/prvCpssIronmanTxqPdxRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPdx.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>

#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


static GT_U32 prvCpssDxChTxqPdxInterfaceMapGetOffset(GT_U32 pdxNum);

/*macro that get offset of specific PDX2PDX_INTERFACE*/
#define PDX2PDX_INTERFACE_OFFSET_MAC(_X)    prvCpssDxChTxqPdxInterfaceMapGetOffset(_X)
/*macro that get size of specific PDX2PDX_INTERFACE*/
#define PDX2PDX_INTERFACE_FIELD_SIZE_MAC(_X)    prvCpssDxChTxqPdxInterfaceMapGetFieldSize(_X)

/*macro that get offset of specific register field at TXQ/PDX*/
#define GET_PDX_REGISTER_FIELD_OFFSET_MAC(REG, FIELD) GET_REGISTER_FIELD_OFFSET_MAC(TXQ, PDX, REG, FIELD)
/*macro that get size of specific register field at TXQ/PDX*/
#define GET_PDX_REGISTER_FIELD_SIZE_MAC(REG, FIELD) GET_REGISTER_FIELD_SIZE_MAC(TXQ, PDX, REG, FIELD)



#define PRV_PDX_SIP_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_30_CHECK_MAC(_dev)?TXQ_IRONMAN_PDX_##_fieldName:TXQ_HARRIER_PDX_##_fieldName)

#define PRV_PDX_SIP_6_15_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)         \
    (PRV_CPSS_SIP_6_20_CHECK_MAC(_dev)? \
     (PRV_PDX_SIP_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_PHOENIX_PDX_##_fieldName)


#define PRV_PDX_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_15_CHECK_MAC(_dev)?\
     (PRV_PDX_SIP_6_15_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_HAWK_PDX_##_fieldName)

#define TXQ_PDX_FIELD_GET(_dev,_fieldName) (PRV_CPSS_SIP_6_10_CHECK_MAC(_dev)?\
    (PRV_PDX_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):\
    (TXQ_PDX_ITX_##_fieldName))


#define TXQ_PDX_CPU_SLICE_INDEX (TXQ_PDX_MAX_SLICE_NUMBER_MAC - 1)

/*
   . This threshold controls the query fill level returned.
   It takes the STATISTICAL bank fill into account.
   If either of the PDX banks (16 banks, each has 341 entries) crosses this threshold,
   the query returns the bank level with the highest level.
   Otherwise, it returns the sum of all
 */
#define PRV_TXQ_PDX_BURST_FIFO_THR_CNS 256

/*This threshold controls the PDX flow control panic indication.
   If either of the PDX banks crosses this threshold and flow control (either .3x or .1qbb) flow control pause message
   is generated to all ports*/

#define PRV_TXQ_PDX_PFCC_FIFO_THR_CNS 310

#define PRV_TXQ_PDX_AC5P_ONLY_MAC(_devNum)\
    do\
    {\
        if ((PRV_CPSS_SIP_6_10_CHECK_MAC(_devNum) == GT_FALSE)||\
        (PRV_CPSS_TXQ_LIKE_PHOENIX_MAC(_devNum) == GT_TRUE))\
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);\
        }\
    }\
    while(0);

#define PRV_TXQ_UNIT_NAME "PDX"
#define PRV_TXQ_LOG_REG GT_FALSE
#define PRV_TXQ_LOG_TABLE GT_FALSE



static GT_STATUS prvCpssSip6TxqPdxLogInit
(
   INOUT PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC * logDataPtr,
   IN GT_CHAR_PTR                              regName
)
{
    CPSS_NULL_PTR_CHECK_MAC(logDataPtr);

    logDataPtr->log = PRV_TXQ_LOG_REG;
    logDataPtr->unitName = PRV_TXQ_UNIT_NAME;
    logDataPtr->unitIndex = PRV_TXQ_LOG_NO_INDEX;
    logDataPtr->regName = regName;
    logDataPtr->regIndex = PRV_TXQ_LOG_NO_INDEX;

    return GT_OK;
}

static GT_STATUS prvCpssSip6TxqPdxTableLogInit
(
   INOUT PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC * logDataPtr,
   IN GT_U32                                   tileNum,
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
    logDataPtr->unitIndex = PRV_TXQ_LOG_NO_INDEX;
    logDataPtr->tableName = tableName;
    logDataPtr->entryIndex = entryIndex;

    return GT_OK;
}


/**
 * @internal prvCpssDxChTxqPdxInterfaceMapGetOffset function
 * @endinternal
 *
 * @brief   Get offset of PDX2PDX_INTERFACE_MAPPING for specific interface
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 *
 * @retval offset                   - the of PDX2PDX_INTERFACE_MAPPING for specific interface
 */


static GT_U32 prvCpssDxChTxqPdxInterfaceMapGetOffset(GT_U32 interfaceId)
{
    GT_U32 offset;

    switch (interfaceId)
    {
    case 0:
        offset = GET_PDX_REGISTER_FIELD_OFFSET_MAC(ITX_PDX2PDX_INTERFACE_MAPPING, PDX_INTERFACE_ID_0);
        break;
    case 1:
        offset = GET_PDX_REGISTER_FIELD_OFFSET_MAC(ITX_PDX2PDX_INTERFACE_MAPPING, PDX_INTERFACE_ID_1);
        break;
    case 2:
        offset = GET_PDX_REGISTER_FIELD_OFFSET_MAC(ITX_PDX2PDX_INTERFACE_MAPPING, PDX_INTERFACE_ID_2);
        break;
    case 3:
    default:/*avoid warnings*/
        offset = GET_PDX_REGISTER_FIELD_OFFSET_MAC(ITX_PDX2PDX_INTERFACE_MAPPING, PDX_INTERFACE_ID_3);
        break;

    }

    return offset;
}

/**
 *
 * @internal prvCpssDxChTxqPdxInterfaceMapGetFieldSize function
 * @endinternal
 *
 * @brief   Get size of PDX2PDX_INTERFACE_MAPPING for specific interface
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 *
 * @retval size                     - of PDX2PDX_INTERFACE_MAPPING field
 *
 * @note Currently only one tile is supported
 *
 */

static GT_U32 prvCpssDxChTxqPdxInterfaceMapGetFieldSize(GT_U32 interfaceId)
{
    GT_U32 size;

    switch (interfaceId)
    {
    case 0:
        size = GET_PDX_REGISTER_FIELD_SIZE_MAC(ITX_PDX2PDX_INTERFACE_MAPPING, PDX_INTERFACE_ID_0);
        break;
    case 1:
        size = GET_PDX_REGISTER_FIELD_SIZE_MAC(ITX_PDX2PDX_INTERFACE_MAPPING, PDX_INTERFACE_ID_1);
        break;
    case 2:
        size = GET_PDX_REGISTER_FIELD_SIZE_MAC(ITX_PDX2PDX_INTERFACE_MAPPING, PDX_INTERFACE_ID_2);
        break;
    case 3:
    default:/*avoid warning*/
        size = GET_PDX_REGISTER_FIELD_SIZE_MAC(ITX_PDX2PDX_INTERFACE_MAPPING, PDX_INTERFACE_ID_3);
        break;

    }

    return size;
}

/**
* @internal prvCpssDxChTxqFalconPdxInterfaceMapSet function
* @endinternal
*
* @brief   Set current PDX to another PDX/s interface mapping
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] pdxNum                - current PDX id(APPLICABLE RANGES:0..3).
* @param[in] connectedPdxId        - the mapping of connected PDXs
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*/
GT_STATUS prvCpssDxChTxqFalconPdxInterfaceMapSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 * connectedPdxId
)
{
    GT_U32      regAddr;
    GT_U32      regValue = 0;
    GT_U32      i;

    GT_STATUS   rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* the device not supports this register */
        return GT_OK;
    }

    rc = prvCpssSip6TxqPdxLogInit(&log,"pdx2pdx_interface_mapping");
    if (rc != GT_OK)
    {
      return rc;
    }


    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.pdx2pdx_interface_mapping;

    for (i = 0; i < 4; i++)
    {
        U32_SET_FIELD_MASKED_MAC(regValue, PDX2PDX_INTERFACE_OFFSET_MAC(i),
                     PDX2PDX_INTERFACE_FIELD_SIZE_MAC(i),
                     connectedPdxId[i]);
    }

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,pdxNum,&log,regAddr,0,32,regValue);

    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdxQueueGroupMapSet function
* @endinternal
*
* @brief   Set entry at PDX DX_QGRPMAP table
 *
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - PP's device number.
* @param[in] queueGroupIndex       - queue group number , entry index
* @param[in] txPortMap             - entry to be written to HW
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdxQueueGroupMapSet
(
    IN GT_U8 devNum,
    IN GT_U32 queueGroupIndex,
    IN PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC txPortMap
)
{
    GT_STATUS   rc;
    GT_U32      regValue = 0;
    PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC tableLog;

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum, txPortMap.dpCoreLocalTrgPort);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(txPortMap.queuePdsIndex);
    TXQ_SIP_6_CHECK_PDX_NUM_MAC(txPortMap.queuePdxIndex);

    TXQ_SIP_6_CHECK_QUEUE_GROUP_NUM_MAC(devNum, queueGroupIndex);

    U32_SET_FIELD_MASKED_MAC(regValue, TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_QUEUE_BASE_FIELD_OFFSET),
                 TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_QUEUE_BASE_FIELD_SIZE),
                 txPortMap.queueBase);
    /*sip 6_30 does not have this field*/
    TXQ_U32_SET_FIELD_MASKED_CHECK_OBSOLETE_MAC(regValue, TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_QUEUE_PDX_INDEX_FIELD_OFFSET),
                 TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_QUEUE_PDX_INDEX_FIELD_SIZE),
                 txPortMap.queuePdxIndex);
    U32_SET_FIELD_MASKED_MAC(regValue, TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_QUEUE_PDS_INDEX_FIELD_OFFSET),
                 TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_QUEUE_PDS_INDEX_FIELD_SIZE),
                 txPortMap.queuePdsIndex);
    U32_SET_FIELD_MASKED_MAC(regValue, TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_DP_CORE_LOCAL_TRG_PORT_FIELD_OFFSET),
                 TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_DP_CORE_LOCAL_TRG_PORT_FIELD_SIZE),
                 txPortMap.dpCoreLocalTrgPort);


    rc = prvCpssSip6TxqPdxTableLogInit(&tableLog,0,
        CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E,"Queue_Group_Map",queueGroupIndex);
    if (rc != GT_OK)
    {
        return rc;
    }


    rc = prvCpssSip6TxqWriteTableEntry(devNum,&tableLog,
                    CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E,
                    queueGroupIndex,
                    &regValue);

    return rc;

}

/**
* @internal prvCpssDxChTxqFalconPdxQueueGroupMapGet function
* @endinternal
*
* @brief  Read entry from PDX DX_QGRPMAP table
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                -PP's device number.
* @param[in] queueGroupIndex       - queue group number , entry index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdxQueueGroupMapGet
(
    IN GT_U8 devNum,
    IN GT_U32 queueGroupIndex,
    OUT PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC * txPortMapPtr
)
{
    GT_STATUS   rc;
    GT_U32      regValue = 0;

    TXQ_SIP_6_CHECK_QUEUE_GROUP_NUM_MAC(devNum, queueGroupIndex);

    rc = prvCpssDxChReadTableEntry(devNum,
                       CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E,
                       queueGroupIndex,
                       &regValue);

    if (rc != GT_OK)
    {
        return rc;
    }


    txPortMapPtr->queueBase = U32_GET_FIELD_MAC(regValue,
                            TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_QUEUE_BASE_FIELD_OFFSET),
                            TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_QUEUE_BASE_FIELD_SIZE));

    /*sip 6_30 does not have this field*/
    if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        txPortMapPtr->queuePdxIndex = U32_GET_FIELD_MAC(regValue,
                            TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_QUEUE_PDX_INDEX_FIELD_OFFSET),
                            TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_QUEUE_PDX_INDEX_FIELD_SIZE));
    }
    else
    {
        txPortMapPtr->queuePdxIndex = 0;
    }

    txPortMapPtr->queuePdsIndex = U32_GET_FIELD_MAC(regValue,
                            TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_QUEUE_PDS_INDEX_FIELD_OFFSET),
                            TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_QUEUE_PDS_INDEX_FIELD_SIZE));

    txPortMapPtr->dpCoreLocalTrgPort = U32_GET_FIELD_MAC(regValue,
                                 TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_DP_CORE_LOCAL_TRG_PORT_FIELD_OFFSET),
                                 TXQ_PDX_FIELD_GET(devNum, QCX_QGRPMAP_DP_CORE_LOCAL_TRG_PORT_FIELD_SIZE));


    return GT_OK;

}

/**
* @internal prvCpssDxChTxqFalconPdxBurstFifoPdsArbitrationMapGet function
* @endinternal
*
* @brief   Read arbitration PIZZA on PDS BurstFifo at PDX
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number
* @param[in] pdxNum                 - current PDX id(APPLICABLE RANGES:0..3).
* @param[in] pdsNum                 - PDS burst fifo number (APPLICABLE RANGES:0..7).
*
* @param[out] activeSliceNumPtr     - the   number of active slices
* @param[out] activeSliceMapArr     - PIZZA configuration
* @param[out] sliceValidArr         - array that mark used slices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*/
GT_STATUS prvCpssDxChTxqFalconPdxBurstFifoPdsArbitrationMapGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 pdsNum,
    OUT GT_U32* activeSliceNumPtr,
    OUT GT_U32* activeSliceMapArr,
    OUT GT_BOOL * sliceValidArr
)
{

    GT_STATUS       rc = GT_OK;
    GT_U32          regValue = 0, pacId = 0, i;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* no PAC unit */
        *activeSliceNumPtr = 0;
        return GT_OK;
    }


    /*read activeSliceNum*/

    if (pdsNum > 3)
    {
        pacId   = 1;
        pdsNum  %= 4;
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum, pdxNum,
                        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[pacId].pizza_Arbiter_Control_Register_pds[pdsNum],
                        TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_MAX_SLICE_FIELD_OFFSET,
                        TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_MAX_SLICE_FIELD_SIZE,
                        activeSliceNumPtr);

    if (rc != GT_OK)
    {
        return rc;
    }


    (*activeSliceNumPtr)++;

    for (i = 0; i < *activeSliceNumPtr; i++)
    {

        rc = prvCpssSip6TxqRegisterFieldRead(devNum, pdxNum,
                        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[pacId].pizza_arbiter_configuration_register_pds[pdsNum][i],
                        0,
                        32,
                        &regValue);

        if (rc != GT_OK)
        {
            return rc;
        }

        activeSliceMapArr[i] = U32_GET_FIELD_MAC(regValue,
                             TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_MAP_0_FIELD_OFFSET,
                             TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_MAP_0_FIELD_SIZE);

        sliceValidArr[i] = U32_GET_FIELD_MAC(regValue,
                             TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_VALID_0_FIELD_OFFSET,
                             TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_VALID_0_FIELD_SIZE);
    }



    return GT_OK;


}

/**
 * @internal prvCpssDxChTxqFalconPdxBurstFifoLoadNewPizza function
 * @endinternal
 *
 * @brief   Triger loading of a new pizza into arbiter
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -  device number
 * @param[in] pdxNum                   -  current PDX id(APPLICABLE RANGES:0..3).
 * @param[in] pdsNum                   -  PDS burst fifo number (APPLICABLE RANGES:0..7).
 *
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 * @note Currently only one tile is supported
 *
 */

GT_STATUS prvCpssDxChTxqFalconPdxBurstFifoLoadNewPizza
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 pdsNum
)
{

    GT_STATUS   rc  = GT_OK;
    GT_U32      pacId   = 0;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* no PAC unit */
        return GT_OK;
    }


    if (pdsNum > 3)
    {
        pacId   = 1;
        pdsNum  %= 4;
    }

    rc = prvCpssSip6TxqPdxLogInit(&log,"pizza_Arbiter_Control_Register_pds");
    if (rc != GT_OK)
    {
      return rc;
    }
    log.regIndex = pdsNum;

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum, pdxNum,&log,
                         PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[pacId].pizza_Arbiter_Control_Register_pds[pdsNum],
                         TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_LOAD_NEW_PIZZA_FIELD_OFFSET,
                         TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_LOAD_NEW_PIZZA_FIELD_SIZE,
                         1);

    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum, pdxNum,&log,
                         PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[pacId].pizza_Arbiter_Control_Register_pds[pdsNum],
                         TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_LOAD_NEW_PIZZA_FIELD_OFFSET,
                         TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_LOAD_NEW_PIZZA_FIELD_SIZE,
                         0);


    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdxBurstFifoSetMaxSlice function
* @endinternal
*
* @brief   Set maximal numer of slices per DP at PDX arbiter
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum      - device number
* @param[in] pdxNum      - current PDX id(APPLICABLE RANGES:0..3).
* @param[in] pdsNum      - DP number(APPLICABLE RANGES:0..7).
* @param[in] maxSlice    - the number of slices used(APPLICABLE RANGES:8..33).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChTxqFalconPdxBurstFifoSetMaxSlice
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 pdsNum,
    IN GT_U32 maxSlice
)
{

    GT_STATUS   rc  = GT_OK;
    GT_U32      pacId   = 0;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;


    TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* no PAC unit */
        return GT_OK;
    }

    if (maxSlice > TXQ_PDX_MAX_SLICE_NUMBER_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected maxSlice - %d ", maxSlice);
    }

    if (pdsNum > 3)
    {
        pacId   = 1;
        pdsNum  %= 4;
    }

    rc = prvCpssSip6TxqPdxLogInit(&log,"pizza_Arbiter_Control_Register_pds");
    if (rc != GT_OK)
    {
      return rc;
    }
    log.regIndex = pdsNum;

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum, pdxNum,&log,
                         PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[pacId].pizza_Arbiter_Control_Register_pds[pdsNum],
                         TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_MAX_SLICE_FIELD_OFFSET,
                         TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_MAX_SLICE_FIELD_SIZE,
                         maxSlice - 1);

    return rc;
}

/**
 * @internal prvCpssDxChTxQFalconPizzaConfigurationSet function
 * @endinternal
 *
 * @brief   Update pizza arbiter
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @param[in] devNum                   -PP's device number.
 *@param[in] pdxNum                   -tile number
 *@param[in] pdsNum                   -pdsNum number
 *@param[in] localChannelIndex -the slice owner[0-7]
 *@param[in] portPizzaSlicesBitMap -the slice owner[0-7]
 *@param[in] enable -equal GT_TRUE in case this is owned ,GT_FALSE otherwise
 *@param[in] isCpu -equal GT_TRUE in case this is slice for CPU ,GT_FALSE otherwise
 */

GT_STATUS prvCpssDxChTxQFalconPizzaConfigurationSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 pdsNum,
    IN GT_U32 localChannelIndex,
    IN GT_U32 portPizzaSlicesBitMap,
    IN GT_BOOL enable,
    IN GT_BOOL isCpu
)
{
    GT_STATUS       rc      = GT_OK;
    GT_U32          i       = 0;
    GT_U32          regValue    = 0;
    GT_U32          originalPdsNum  = pdsNum;
    GT_U32          pacId = 0;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;


    TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* no PAC unit */
        return GT_OK;
    }
    if ((enable == GT_TRUE) && (isCpu == GT_FALSE))
    {
        TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum, localChannelIndex);
    }


    if (pdsNum > 3)
    {
        pacId        = 1;
        pdsNum      %= 4;
    }

    if (isCpu == GT_TRUE)
    {
        i = TXQ_PDX_CPU_SLICE_INDEX;
    }

    rc = prvCpssSip6TxqPdxLogInit(&log,"pizza_arbiter_configuration_register_pds");
    if (rc != GT_OK)
    {
      return rc;
    }


    while (portPizzaSlicesBitMap || (isCpu == GT_TRUE))
    {
        log.regIndex=i;

        if ((isCpu == GT_TRUE) || (portPizzaSlicesBitMap & 1))
        {
            U32_SET_FIELD_MASKED_MAC(regValue, TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_VALID_0_FIELD_OFFSET,
                         TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_VALID_0_FIELD_SIZE,
                         (enable == GT_TRUE) ? 1 : 0);

            U32_SET_FIELD_MASKED_MAC(regValue, TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_MAP_0_FIELD_OFFSET,
                         TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_MAP_0_FIELD_SIZE,
                         localChannelIndex);

             rc = prvCpssSip6TxqRegisterFieldWrite(devNum, pdxNum,&log,
                         PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[pacId].pizza_arbiter_configuration_register_pds[pdsNum][i],
                         0,
                         32,
                         regValue);
        }



        i++;
        portPizzaSlicesBitMap   >>= 1;
        isCpu           = GT_FALSE;
    }


    /*Load*/

    rc = prvCpssDxChTxqFalconPdxBurstFifoLoadNewPizza(devNum, pdxNum, originalPdsNum);

    return rc;
}
/**
 * @internal prvCpssDxChTxQFalconPizzaArbiterInitPerDp function
 * @endinternal
 *
 * @brief   Initialize the number of silices to each PDS pizza arbiter,also initialize all the slices to unused
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] pdxNum                   -tile number
 * @param[in] pdsNum                   -pdsNum number
 * @param[in] pdxPizzaNumberOfSlicesArr-number of slices per dp
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 * @note Currently only one tile is supported
 *
 */
GT_STATUS prvCpssDxChTxQFalconPizzaArbiterInitPerDp
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 pdsNum,
    IN GT_U32 pdxPizzaNumberOfSlices
)
{
    GT_U32          i;
    GT_STATUS       rc      = GT_OK;
    GT_U32          pacId = 0;
    GT_U32          originalPdsNum  = pdsNum;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* no PAC unit */
        return GT_OK;
    }

    if (pdsNum > 3)
    {
        pacId        = 1;
        pdsNum      %= 4;
    }

     rc = prvCpssSip6TxqPdxLogInit(&log,"pizza_arbiter_configuration_register_pds");
     if (rc != GT_OK)
     {
       return rc;
     }

    /*invalidate all the slots*/
    for (i = 0; i < TXQ_PDX_MAX_SLICE_NUMBER_MAC; i++)
    {
        log.regIndex = i;
        rc = prvCpssSip6TxqRegisterFieldWrite(devNum, pdxNum,&log,
                         PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[pacId].pizza_arbiter_configuration_register_pds[pdsNum][i],
                         TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_VALID_0_FIELD_OFFSET,
                         TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_VALID_0_FIELD_SIZE,
                         0);
        if (rc != GT_OK)
        {
            return rc;
        }

    }


    /*set slice numbers*/
    rc = prvCpssDxChTxqFalconPdxBurstFifoSetMaxSlice(devNum, pdxNum, originalPdsNum, pdxPizzaNumberOfSlices);

    if (rc != GT_OK)
    {
        return rc;
    }

    /*load*/

    rc = prvCpssDxChTxqFalconPdxBurstFifoLoadNewPizza(devNum, pdxNum,originalPdsNum);


    return rc;


}

/**
* @internal prvCpssDxChTxqFalconPdxPacPortDescCounterGet function
* @endinternal
*
* @brief   Get descriptor counter per port at PDX PAC unit
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - PP's device number.
* @param[in] pdxNum                - tile number
* @param[in] portNum               - global port num (0..71)
* @param[out ] descNumPtr          - number of descriptors
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/
GT_STATUS prvCpssDxChTxqFalconPdxPacPortDescCounterGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 portNum,
    OUT GT_U32* descNumPtr
)
{

    GT_STATUS   rc  = GT_OK;
    GT_U32      pacId   = 0;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);
    TXQ_SIP_6_CHECK_GLOBAL_PORT_NUM_MAC(devNum, portNum);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* no PAC unit */
        return GT_OK;
    }

    if (portNum >= 36)
    {
        pacId   = 1;
        portNum %= 36;
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum, pdxNum,
                        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[pacId].port_desc_counter[portNum],
                        TXQ_PDX_PAC_PORT_DESC_COUNTER_PORT_DESC_COUNTER_FIELD_OFFSET,
                        TXQ_PDX_PAC_PORT_DESC_COUNTER_PORT_DESC_COUNTER_FIELD_SIZE,
                        descNumPtr);

    return rc;


}

/**
 * @internal prvCpssDxChTxqFalconPdxBurstFifoInit function
 * @endinternal
 *
 * @brief  Initialize PDX burst fifo thresholds
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman,Hawk
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                - PP's device number.
 * @param[in] pdxNum                - tile number
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 */
GT_STATUS prvCpssDxChTxqFalconPdxBurstFifoInit
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum
)
{
    GT_U32      regAddr;
    GT_U32      regValue    = 0;
    GT_STATUS   rc      = GT_OK;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(pdxNum);

    /*For Hawk use HW defaults*/
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
    {
        regValue = PRV_TXQ_PDX_BURST_FIFO_THR_CNS;
        TXQ_SIP_6_CHECK_FIELD_LENGTH_VALID_MAC(regValue, TXQ_PDX_FIELD_GET(devNum, GLOBAL_BURST_FIFO_THR_GLOBAL_BURST_FIFO_THR_FIELD_SIZE));
        regAddr     = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.global_burst_fifo_thr[0];

        rc = prvCpssSip6TxqPdxLogInit(&log,"pdx.global_burst_fifo_thr");
        if (rc != GT_OK)
        {
          return rc;
        }
        log.regIndex=0;

         rc = prvCpssSip6TxqRegisterFieldWrite(devNum,pdxNum,&log,regAddr,0,32,regValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        regValue = PRV_TXQ_PDX_PFCC_FIFO_THR_CNS;
        TXQ_SIP_6_CHECK_FIELD_LENGTH_VALID_MAC(regValue, TXQ_PDX_FIELD_GET(devNum, PFCC_BURST_FIFO_THR_PFCC_BURST_FIFO_THR_FIELD_SIZE));

        log.regName = "pfcc_burst_fifo_thr";
        regAddr     = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.pfcc_burst_fifo_thr[0];

         rc = prvCpssSip6TxqRegisterFieldWrite(devNum,pdxNum,&log,regAddr,0,32,regValue);
    }


    return rc;
}

/**
 * @internal prvCpssDxChTxqSip6_10PdxPreemptionEnableSet function
 * @endinternal
 *
 * @brief  Enable/disable preemption at PDX glue
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                - PP's device number.
 * @param[in] groupOfQueues         - group of queues
 * @param[in] enable                - if equal GT_TRUE then preemption is enabled in PDX,
 *                                    GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 */
GT_STATUS prvCpssDxChTxqSip6_10PdxPreemptionEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  groupOfQueues,
    IN GT_BOOL enable
)
{
    GT_U32      regAddr,index,bit;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;

    PRV_TXQ_PDX_AC5P_ONLY_MAC(devNum);

    rc = prvCpssSip6TxqPdxLogInit(&log,"Preemption_Enable");
    if (rc != GT_OK)
    {
      return rc;
    }

    index = groupOfQueues>>5;
    bit   = groupOfQueues&0x1F;
    regAddr =PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.Preemption_Enable[index];

    log.regIndex = index;

    return prvCpssSip6TxqRegisterFieldWrite(devNum,0,&log,regAddr,bit,1,BOOL2BIT_MAC(enable));

}

/**
 * @internal prvCpssDxChTxqSip6_10PdxPreemptionEnableSet function
 * @endinternal
 *
 * @brief  Get enable/disable preemption at PDX glue
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                - PP's device number.
 * @param[in] groupOfQueues         - group of queues
 * @param[out] enablePtr            -(pointer to)if equal GT_TRUE then preemption is enabled in PDX,
 *                                    GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 */
GT_STATUS prvCpssDxChTxqSip6_10PdxPreemptionEnableGet
(
    IN GT_U8   devNum,
    IN GT_U32  groupOfQueues,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32      regAddr,index,bit;
    GT_STATUS   rc,data;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_TXQ_PDX_AC5P_ONLY_MAC(devNum);

    index = groupOfQueues>>5;
    bit   = groupOfQueues&0x1F;
    regAddr =PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.Preemption_Enable[index];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, bit,1,&data);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(data);

    return rc;
}

/**
* @internal prvCpssSip6TxqPdxDebugInterruptDisableSet function
* @endinternal
*
* @brief   Debug function that disable iterrupt in PDX
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqPdxDebugInterruptDisableSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
)
{
    GT_U32  regAddr;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    rc = prvCpssSip6TxqPdxLogInit(&log,"PDX_Interrupt_Summary_Mask");
    if (rc != GT_OK)
    {
      return rc;
    }


    if(GT_TRUE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.PDX_Interrupt_Summary_Mask;
    }
    else
    {   /*pdx itx*/
        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.Interrupt_Mask;
        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 regAddr,
                 0,32,0);
         if (rc != GT_OK)
        {
            return rc;
        }

       /*pdx pac*/
       log.regName = "Interrupt_Mask";
       regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[0].Interrupt_Mask;
       rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 regAddr,
                 0,32,0);
       if (rc != GT_OK)
       {
            return rc;
       }

       regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[1].Interrupt_Mask;

    }

    return   prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 regAddr,
                 0,32,0);
}

/**
* @internal prvCpssSip6TxqPdxDebugInterruptGet function
* @endinternal
*
* @brief   Get interrupt cause for PDX
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] qfcNum                - data path index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqPdxDebugInterruptGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    OUT GT_U32 *itxFunctionalPtr,
    OUT GT_U32 *itxDebugPtr,
    OUT GT_U32 *pac0FunctionalPtr,
    OUT GT_U32 *pac1FunctionalPtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;

    CPSS_NULL_PTR_CHECK_MAC(itxFunctionalPtr);
    CPSS_NULL_PTR_CHECK_MAC(itxDebugPtr);
    CPSS_NULL_PTR_CHECK_MAC(pac0FunctionalPtr);
    CPSS_NULL_PTR_CHECK_MAC(pac1FunctionalPtr);

    *itxDebugPtr = 0;
    *pac0FunctionalPtr =0;
    *pac1FunctionalPtr =0;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    if(GT_TRUE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.PDX_Functional_Interrupt_Cause;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.Interrupt_Cause;
    }


    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 regAddr,
                 0,32,itxFunctionalPtr);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
    }

    if(GT_TRUE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.PDX_Debug_Interrupt_Cause;
         rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 regAddr,
                 0,32,itxDebugPtr);

        if(rc!=GT_OK)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[0].Interrupt_Cause;
        rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 regAddr,
                 0,32,pac0FunctionalPtr);

        if(rc!=GT_OK)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
        }

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[1].Interrupt_Cause;
        rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 regAddr,
                 0,32,pac1FunctionalPtr);

        if(rc!=GT_OK)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
        }
    }

    return rc;
}



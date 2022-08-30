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
* @file prvCpssDxChTxqPds.c
*
* @brief CPSS SIP6 TXQ PDS low level configurations.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/falcon/pipe/prvCpssFalconTxqPdsRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5p/pipe/prvCpssAc5pTxqPdsRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5x/pipe/prvCpssAc5xTxqPdsRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/harrier/pipe/prvCpssHarrierTxqPdsRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ironman/pipe/prvCpssIronmanTxqPdsRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPds.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>


#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_PDS_SIP_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_30_CHECK_MAC(_dev)?TXQ_IRONMAN_PDS_##_fieldName:TXQ_HARRIER_PDS_##_fieldName)

#define PRV_PDS_SIP_6_15_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_20_CHECK_MAC(_dev)? \
     (PRV_PDS_SIP_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_PHOENIX_PDS_##_fieldName)


#define PRV_PDS_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_15_CHECK_MAC(_dev)?\
     (PRV_PDS_SIP_6_15_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_HAWK_PDS_##_fieldName)

#define TXQ_PDS_FIELD_GET(_dev,_fieldName) (PRV_CPSS_SIP_6_10_CHECK_MAC(_dev)?\
    (PRV_PDS_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):\
    (TXQ_PDS_##_fieldName))

/*Macro for fileds discontinued for SIP 6_30*/

#define PRV_PDS_SIP_6_20_LAST_DEV_FIELD_GET_MAC(_dev,_fieldName) \
    TXQ_HARRIER_PDS_##_fieldName

#define PRV_PDS_SIP_6_15_UNTIL_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_20_CHECK_MAC(_dev)? \
     (PRV_PDS_SIP_6_20_LAST_DEV_FIELD_GET_MAC(_dev,_fieldName)):TXQ_PHOENIX_PDS_##_fieldName)


#define PRV_PDS_SIP_6_10_UNTIL_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_15_CHECK_MAC(_dev)?\
     (PRV_PDS_SIP_6_15_UNTIL_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_HAWK_PDS_##_fieldName)

#define TXQ_PDS_UNTIL_6_20_FIELD_GET(_dev,_fieldName) (PRV_CPSS_SIP_6_10_CHECK_MAC(_dev)?\
    (PRV_PDS_SIP_6_10_UNTIL_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):\
    (TXQ_PDS_##_fieldName))

#define PRV_TXQ_UNIT_NAME "PDS"
#define PRV_TXQ_LOG_UNIT GT_FALSE


GT_STATUS prvCpssSip6TxqPdsLogInit
(
   INOUT PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC * logDataPtr,
   IN GT_U32                                   pdsNum,
   IN GT_CHAR_PTR                              regName
)
{
    CPSS_NULL_PTR_CHECK_MAC(logDataPtr);

    logDataPtr->log = PRV_TXQ_LOG_UNIT;
    logDataPtr->unitName = PRV_TXQ_UNIT_NAME;
    logDataPtr->unitIndex = pdsNum;
    logDataPtr->regName = regName;
    logDataPtr->regIndex = PRV_TXQ_LOG_NO_INDEX;

    return GT_OK;
}



/**
* @internal prvCpssDxChTxqFalconPdsMaxPdsSizeLimitSet function
* @endinternal
*
* @brief  Set limit on PDS capacity .When this limit is reached, stop accepting descriptors into the PDS.
*         The motivation is to reduce risk of PDS getting FULL
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] pdsSizeForPdx         - limit on PDS capacity for PDX
* @param[in] pdsSizeForPb          - limit on PDS capacity for PB
* @param[in] pbFullLimit           - limit on PB for PDS(used for long queues mechanism)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsMaxPdsSizeLimitSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 pdsSizeForPdx,
    IN GT_U32 pdsSizeForPb,
    IN GT_U32 pbFullLimit
)
{
    GT_U32 regAddr;
    GT_U32 regValue = 0;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if(pdsSizeForPdx>= (GT_U32)(1<<TXQ_PDS_FIELD_GET(devNum,MAX_PDS_SIZE_LIMIT_FOR_PDX_MAX_PDS_SIZE_LIMIT_FOR_PDX_FIELD_SIZE)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Max_PDS_size_limit_for_pdx");
    if (rc != GT_OK)
    {
      return rc;
    }


    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Max_PDS_size_limit_for_pdx;

    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_FIELD_GET(devNum,MAX_PDS_SIZE_LIMIT_FOR_PDX_MAX_PDS_SIZE_LIMIT_FOR_PDX_FIELD_OFFSET),
        TXQ_PDS_FIELD_GET(devNum,MAX_PDS_SIZE_LIMIT_FOR_PDX_MAX_PDS_SIZE_LIMIT_FOR_PDX_FIELD_SIZE),
        pdsSizeForPdx);

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);
    if(rc!=GT_OK)
    {
        return rc;
    }


    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)==GT_FALSE)
    {
        if(pdsSizeForPdx>= (GT_U32)(1<<TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,MAX_PDS_SIZE_LIMIT_FOR_PB_MAX_PDS_SIZE_LIMIT_FOR_PB_FIELD_SIZE)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(pbFullLimit>= (GT_U32)(1<<TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PB_FULL_LIMIT_PB_FULL_LIMIT_FIELD_SIZE)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }



        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Max_PDS_size_limit_for_PB;
        log.regName = "Max_PDS_size_limit_for_PB";

        regValue =0;

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,MAX_PDS_SIZE_LIMIT_FOR_PB_MAX_PDS_SIZE_LIMIT_FOR_PB_FIELD_OFFSET),
            TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,MAX_PDS_SIZE_LIMIT_FOR_PB_MAX_PDS_SIZE_LIMIT_FOR_PB_FIELD_SIZE),
            pdsSizeForPb);

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);
        if(rc!=GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].PB_Full_Limit;
        log.regName = "PB_Full_Limit";


        regValue =0;

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PB_FULL_LIMIT_PB_FULL_LIMIT_FIELD_OFFSET),
            TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PB_FULL_LIMIT_PB_FULL_LIMIT_FIELD_SIZE),
            pbFullLimit);

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);

    }


    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdsProfileSet function
* @endinternal
*
* @brief   Profile :
*         Profile_%p_Long_Queue_Enable - Enables writing a queue to the PB when it is getting long
*         Profile_%p_Long_Queue_Limit [0..8K] - When the total queue counter reaches this limit, a long queue is opened
*         Profile_%p_Head_Empty_Limit [0..8K] - When the head counter is below this limit,
*         the head is considered “empty” and fragment read from PB is triggered
*         Length_Adjust_Constant_byte_count_Profile_%p_ [0..64] -
*         Sets the value which will be decremented or incremented from the packet's byte count
*         Length_Adjust_Sub_Profile_%p_ - Subtract enable bit - for profile <%p>
*         0x0 = ADD; ADD; When ADD, the value of constant byte count field is added to the descriptor byte count
*         0x1 = SUB; SUB; When SUB, the value of constant byte count field is subtracted from the descriptor byte count
*         Length_Adjust_Enable_profile_%p -
*         RW 0x0
*         Enable the length adjust
*         0x0 = Disable Length Adjust; Disable_Length_Adjust
*         0x1 = Enable Length Adjust; Enable_Length_Adjust
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] profileIndex          - index of the profile(APPLICABLE RANGES:0..15.)
* @param[in] profilePtr            - (pointer to)PDS  profile parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsProfileSet
(
    IN GT_U8    devNum,
    IN GT_U32   tileNum,
    IN GT_U32   pdsNum,
    IN GT_U32   profileIndex,
    IN PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC *profilePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 regValue = 0;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);


    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Profile_Head_Empty_Limit");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = profileIndex;

    if(profileIndex>=PDS_PROFILE_MAX_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)==GT_FALSE)
    {
        if(profilePtr->longQueueParametersPtr)
        {
            if(profilePtr->longQueueParametersPtr->headEmptyLimit>=(GT_U32)(1<<TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_SIZE)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if(profilePtr->longQueueParametersPtr->longQueueLimit>=(GT_U32)(1<<TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_SIZE)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_OFFSET),
                TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_SIZE),
                profilePtr->longQueueParametersPtr->headEmptyLimit);

           rc =prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Head_Empty_Limit[profileIndex],
            0,32,regValue);

            if(rc!=GT_OK)
            {
                return rc;
            }

            regValue = 0;

            U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_OFFSET),
                TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_SIZE),
                profilePtr->longQueueParametersPtr->longQueueLimit);

            log.regName="Profile_Long_Queue_Limit";

            rc =prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,
             PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Long_Queue_Limit[profileIndex],
             0,32,regValue);

            if(rc!=GT_OK)
            {
                return rc;
            }

            regValue = 0;



            U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PROFILE_LONG_QUEUE_ENABLE_PROFILE_LONG_QUEUE_ENABLE_FIELD_OFFSET),
                TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PROFILE_LONG_QUEUE_ENABLE_PROFILE_LONG_QUEUE_ENABLE_FIELD_SIZE),
                profilePtr->longQueueParametersPtr->longQueueEnable?1:0);

            log.regName="Profile_Long_Queue_Enable";

            rc =prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,
               PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Long_Queue_Enable[profileIndex],
               0,32,regValue);

            if(rc!=GT_OK)
            {
                return rc;
            }
        }
    }

    if(profilePtr->lengthAdjustParametersPtr)
    {
        if(profilePtr->lengthAdjustParametersPtr->lengthAdjustByteCount>=(GT_U32)(1<<TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_SIZE)))
        {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regValue = 0;

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_ENABLE_PROFILE_FIELD_OFFSET),
            TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_ENABLE_PROFILE_FIELD_SIZE),
            profilePtr->lengthAdjustParametersPtr->lengthAdjustEnable);



        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_SUB_PROFILE_FIELD_OFFSET),
            TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_SUB_PROFILE_FIELD_SIZE),
            profilePtr->lengthAdjustParametersPtr->lengthAdjustSubstruct);


        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_OFFSET),
            TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_SIZE),
            profilePtr->lengthAdjustParametersPtr->lengthAdjustByteCount);

       log.regName="Profile_Dequeue_Length_Adjust";

       rc =prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,
           PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Dequeue_Length_Adjust[profileIndex],
           0,32,regValue);
    }

    return rc;


}

/**
* @internal prvCpssDxChTxqFalconPdsProfileGet function
* @endinternal
*
* @brief   Profile :
*         Profile_%p_Long_Queue_Enable - Enables writing a queue to the PB when it is getting long
*         Profile_%p_Long_Queue_Limit [0..8K] - When the total queue counter reaches this limit, a long queue is opened
*         Profile_%p_Head_Empty_Limit [0..8K] - When the head counter is below this limit,
*         the head is considered “empty” and fragment read from PB is triggered
*         Length_Adjust_Constant_byte_count_Profile_%p_ [0..64] -
*         Sets the value which will be decremented or incremented from the packet's byte count
*         Length_Adjust_Sub_Profile_%p_ - Subtract enable bit - for profile <%p>
*         0x0 = ADD; ADD; When ADD, the value of constant byte count field is added to the descriptor byte count
*         0x1 = SUB; SUB; When SUB, the value of constant byte count field is subtracted from the descriptor byte count
*         Length_Adjust_Enable_profile_%p -
*         RW 0x0
*         Enable the length adjust
*         0x0 = Disable Length Adjust; Disable_Length_Adjust
*         0x1 = Enable Length Adjust; Enable_Length_Adjust
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] profileIndex          - index of the profile(APPLICABLE RANGES:0..15.)
* @param[in] lengthAdjustOnly      - GT_TRUE in order to get only length adjust attributes,GT_FALSE otherwise
* @param[out] profilePtr           - (pointer to)pds profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsProfileGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 profileIndex,
    IN GT_BOOL   lengthAdjustOnly,
    OUT  PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC * profilePtr
)
{
    GT_STATUS rc;
    GT_U32 regValue = 0;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);
    CPSS_NULL_PTR_CHECK_MAC(profilePtr);

    if(profileIndex>=PDS_PROFILE_MAX_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)==GT_FALSE)
    {

        if(lengthAdjustOnly==GT_FALSE)
        {
           CPSS_NULL_PTR_CHECK_MAC(profilePtr->longQueueParametersPtr);

           rc =prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,
                PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Head_Empty_Limit[profileIndex],
                TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_OFFSET),
                 TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_SIZE),
                 &(profilePtr->longQueueParametersPtr->headEmptyLimit));

            if(rc!=GT_OK)
            {
                return rc;
            }

           rc =prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,
                PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Long_Queue_Limit[profileIndex],
                TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_OFFSET),
                TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_SIZE),
                &(profilePtr->longQueueParametersPtr->longQueueLimit));

            if(rc!=GT_OK)
            {
                return rc;
            }

            rc =prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Long_Queue_Enable[profileIndex],
                 TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PROFILE_LONG_QUEUE_ENABLE_PROFILE_LONG_QUEUE_ENABLE_FIELD_OFFSET),
                 TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PROFILE_LONG_QUEUE_ENABLE_PROFILE_LONG_QUEUE_ENABLE_FIELD_SIZE),
                 &regValue);

            if(rc!=GT_OK)
            {
                return rc;
            }

            profilePtr->longQueueParametersPtr->longQueueEnable = (regValue==1)?GT_TRUE:GT_FALSE;

        }
    }

    CPSS_NULL_PTR_CHECK_MAC(profilePtr->lengthAdjustParametersPtr);

    rc =prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,
           PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Dequeue_Length_Adjust[profileIndex],
           0,32,&regValue);

    if(rc == GT_OK)
    {
        profilePtr->lengthAdjustParametersPtr->lengthAdjustEnable = (GT_BOOL)U32_GET_FIELD_MAC(regValue,
         TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_ENABLE_PROFILE_FIELD_OFFSET),
         TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_ENABLE_PROFILE_FIELD_SIZE));

        profilePtr->lengthAdjustParametersPtr->lengthAdjustSubstruct= (GT_BOOL)U32_GET_FIELD_MAC(regValue,
         TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_SUB_PROFILE_FIELD_OFFSET),
         TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_SUB_PROFILE_FIELD_SIZE));

        profilePtr->lengthAdjustParametersPtr->lengthAdjustByteCount= (GT_BOOL)U32_GET_FIELD_MAC(regValue,
         TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_OFFSET),
         TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_SIZE));
    }

    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdsQueueProfileMapSet function
* @endinternal
*
* @brief   Set queue profile to PDS queue.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] queueNumber           - the number of the queue(APPLICABLE RANGES:0..399)
* @param[in] profileIndex          - index of the profile(APPLICABLE RANGES:0..15.)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsQueueProfileMapSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueNumber,
    IN GT_U32 profileIndex
)
{
    GT_U32 regValue = 0;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    /*not supported*/
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)==GT_TRUE)
    {
        return GT_OK;
    }

    if(profileIndex>=PDS_PROFILE_MAX_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Queue_PDS_Profile");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = queueNumber;

    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,QUEUE_PDS_PROFILE_QUEUE_PROFILE_3_0_FIELD_OFFSET),
        TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,QUEUE_PDS_PROFILE_QUEUE_PROFILE_3_0_FIELD_SIZE),
        profileIndex);


    rc =prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Queue_PDS_Profile[queueNumber],
        0,32,regValue);

    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdsQueueProfileMapGet function
* @endinternal
*
* @brief   Get queue profile that binded to  PDS queue.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] queueNumber           - the number of the queue(APPLICABLE RANGES:0..399)
* @param[in] profileIndexPtr       - (pointer to) index of the profile(APPLICABLE RANGES:0..15.)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsQueueProfileMapGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueNumber,
    OUT  GT_U32 * profileIndexPtr
)
{
    GT_U32 regValue = 0;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

        /*not supported*/
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)==GT_TRUE)
    {
        return GT_OK;
    }

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc =prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Queue_PDS_Profile[queueNumber],
        0,32,&regValue);

    if(rc == GT_OK)
    {
        *profileIndexPtr  = (GT_BOOL)U32_GET_FIELD_MAC(regValue,
                         TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,QUEUE_PDS_PROFILE_QUEUE_PROFILE_3_0_FIELD_OFFSET),
                         TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,QUEUE_PDS_PROFILE_QUEUE_PROFILE_3_0_FIELD_SIZE));
    }

    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapSet function
* @endinternal
*
* @brief   Set lenght adjust profile to PDS queue.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                -  device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] queueNumber           - the number of the queue(APPLICABLE RANGES:0..255)
* @param[in] profileIndex          - index of the profile(APPLICABLE RANGES:0..15.)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueNumber,
    IN GT_U32 profileIndex
)
{
    GT_U32 regValue = 0;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    /*only supported for HAWK*/
    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)==GT_FALSE)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if(profileIndex>=PDS_PROFILE_MAX_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_MASKED_MAC(regValue,
        PRV_PDS_SIP_6_10_DEV_DEP_FIELD_GET_MAC(devNum,QUEUE_LENGTH_ADJUST_PROFILE_QUEUE_LENGTH_ADJUST_PROFILE_FIELD_OFFSET),
        PRV_PDS_SIP_6_10_DEV_DEP_FIELD_GET_MAC(devNum,QUEUE_LENGTH_ADJUST_PROFILE_QUEUE_LENGTH_ADJUST_PROFILE_FIELD_SIZE),
        profileIndex);


   rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Queue_Length_Adjust_Profile");
   if (rc != GT_OK)
   {
     return rc;
   }

   log.regIndex = queueNumber;

   rc =prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Queue_Length_Adjust_Profile[queueNumber],
        0,32,regValue);

    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapGet function
* @endinternal
*
* @brief   Get lenght adjust profile binded to PDS queue.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] queueNumber           - the number of the queue(APPLICABLE RANGES:0..255)
* @param[out] profileIndexPtr      - (pointer to)index of the profile(APPLICABLE RANGES:0..15.)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueNumber,
    OUT  GT_U32 * profileIndexPtr
)
{
    GT_U32 regValue = 0;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc =prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Queue_Length_Adjust_Profile[queueNumber],
        0,32,&regValue);

    if(rc == GT_OK)
     {

        *profileIndexPtr  = (GT_BOOL)U32_GET_FIELD_MAC(regValue,
                         TXQ_HAWK_PDS_QUEUE_LENGTH_ADJUST_PROFILE_QUEUE_LENGTH_ADJUST_PROFILE_FIELD_OFFSET,
                          TXQ_HAWK_PDS_QUEUE_LENGTH_ADJUST_PROFILE_QUEUE_LENGTH_ADJUST_PROFILE_FIELD_SIZE);
    }

    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet function
* @endinternal
*
* @brief   Counts the number of descriptors of all queues in both the cache and the PB
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
*
* @param[out] pdsDescCountPtr      - the number of descriptors of all queues in both the cache and the PB
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    OUT  GT_U32 * pdsDescCountPtr
)
{
    GT_U32 regAddr,tileOffset ;
    GT_U32 regValue = 0;
    GT_STATUS rc;

    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC * regAddrData;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    regAddrData = &(PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1);

    regAddr = regAddrData->TXQ.pds[pdsNum].Total_PDS_Counter;

    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

    if(rc == GT_OK)
     {
        *pdsDescCountPtr = U32_GET_FIELD_MAC(regValue,TXQ_PDS_FIELD_GET(devNum,TOTAL_PDS_COUNTER_TOTAL_PDS_COUNTER_FIELD_OFFSET),
            TXQ_PDS_FIELD_GET(devNum,TOTAL_PDS_COUNTER_TOTAL_PDS_COUNTER_FIELD_SIZE));
     }

    return rc;
}


/**
* @internal prvCpssDxChTxqFalconPdsCacheDescCounterGet function
* @endinternal
*
* @brief   Counts the number of descriptors of all queues in the cache (not including the PB)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
*
* @param[out] pdsDescCountPtr      - the number of descriptors of all queues in the cache (not including the PB)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsCacheDescCounterGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    OUT  GT_U32 * pdsDescCountPtr
)
{
    GT_U32 regAddr,tileOffset ;
    GT_U32 regValue = 0;
    GT_STATUS rc;

    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC * regAddrData;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

     /*not supported*/
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)==GT_TRUE)
    {
        return GT_OK;
    }

    regAddrData = &(PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1);

    regAddr = regAddrData->TXQ.pds[pdsNum].PDS_Cache_Counter;

     tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
     regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

    if(rc == GT_OK)
     {
        *pdsDescCountPtr = U32_GET_FIELD_MAC(regValue,TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PDS_CACHE_COUNTER_PDS_CACHE_COUNTER_FIELD_OFFSET),
            TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PDS_CACHE_COUNTER_PDS_CACHE_COUNTER_FIELD_SIZE));
     }

    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdsPerQueueCountersGet function
* @endinternal
*
* @brief   Counts the number of descriptors per queues in the cache and  the PB
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] queueNumber           - the local queue number (APPLICABLE RANGES:0..399).
*
* @param[out] perQueueDescCountPtr - the number of descriptors at the queue
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsPerQueueCountersGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueNumber,
    IN PRV_CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTER_STC * perQueueDescCountPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 regValue[2],size,value;
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);
    CPSS_NULL_PTR_CHECK_MAC(perQueueDescCountPtr);

     /*not supported*/
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)==GT_TRUE)
    {
        return GT_OK;
    }

    if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        queueNumber = queueNumber;/*avoid  warning*/
        perQueueDescCountPtr->headCounter =0;
        perQueueDescCountPtr->longQueue=GT_FALSE;
    }
    else
    {
        rc = prvCpssDxChReadTableEntry(devNum,
                                    CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E+pdsNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC,
                                    queueNumber,
                                    regValue);

        if(rc == GT_OK)
        {
            perQueueDescCountPtr->longQueue = (GT_BOOL)U32_GET_FIELD_MAC(regValue[0],
                     TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PER_QUEUE_COUNTERS_LONG_FIELD_OFFSET),
                      TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PER_QUEUE_COUNTERS_LONG_FIELD_SIZE));

            perQueueDescCountPtr->fragIndex = (GT_BOOL)U32_GET_FIELD_MAC(regValue[0],
                     TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PER_QUEUE_COUNTERS_FIDX_FIELD_OFFSET),
                      TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PER_QUEUE_COUNTERS_FIDX_FIELD_SIZE));

            perQueueDescCountPtr->headCounter = (GT_BOOL)U32_GET_FIELD_MAC(regValue[0],
                     TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PER_QUEUE_COUNTERS_HEAD_COUNTER_FIELD_OFFSET),
                      TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PER_QUEUE_COUNTERS_HEAD_COUNTER_FIELD_SIZE));

            perQueueDescCountPtr->fragCounter = (GT_BOOL)U32_GET_FIELD_MAC(regValue[0],
                     TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PER_QUEUE_COUNTERS_FRAG_COUNTER_FIELD_OFFSET),
                      TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PER_QUEUE_COUNTERS_FRAG_COUNTER_FIELD_SIZE));

                     size = 32 - TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PER_QUEUE_COUNTERS_TAIL_COUNTER_FIELD_OFFSET);

            perQueueDescCountPtr->tailCounter = 0;

            perQueueDescCountPtr->tailCounter= U32_GET_FIELD_MAC(regValue[0],
                            TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PER_QUEUE_COUNTERS_TAIL_COUNTER_FIELD_OFFSET),
                size);

            value =  U32_GET_FIELD_MAC(regValue[1],
                    0,
                    TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,PER_QUEUE_COUNTERS_TAIL_COUNTER_FIELD_SIZE)- size);

            perQueueDescCountPtr->tailCounter|= ((value)<<size);
        }
    }

    return rc;
}

GT_STATUS prvCpssDxChTxqFalconPdsDataStorageGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 descriptorInd,
    IN GT_U32 *descriptorPtr
)
{
    GT_STATUS rc = GT_OK;
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);
    CPSS_NULL_PTR_CHECK_MAC(descriptorPtr);

    if(descriptorInd>TXQ_PDS_MAX_DESC_NUMBER_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "descriptorInd[%d] must be less than [%d]",descriptorInd,TXQ_PDS_MAX_DESC_NUMBER_MAC);
    }


    rc = prvCpssDxChReadTableEntry(devNum,
                                    CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E+pdsNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC,
                                    descriptorInd,
                                    descriptorPtr);
    return rc;
}

/**
* @internal prvCpssDxChTxqSip6PdsPbReadReqFifoLimitSet function
* @endinternal
*
* @brief  Configure read request Fifo limit
*
* @note   APPLICABLE DEVICES:AC5X;Harrier; Ironman;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P .
*
* @param[in] devNum                - device number
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] pbReadReqFifoLimit    - fifo limit.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqSip6PdsPbReadReqFifoLimitSet
(
    IN GT_U8 devNum,
    IN GT_U32 pdsNum,
    IN GT_U32 pbReadReqFifoLimit
)
{
    GT_U32 regAddr;
    GT_U32 regValue = 0;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        return GT_OK;
    }

     /*not supported*/
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)==GT_TRUE)
    {
        return GT_OK;
    }

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if(pbReadReqFifoLimit>= (GT_U32)(1<<PRV_PDS_SIP_6_15_UNTIL_6_20_DEV_DEP_FIELD_GET_MAC(devNum,PB_READ_REQ_FIFO_LIMIT_PB_RD_REQ_FIFO_THRESHOLD_FIELD_SIZE)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"PB_Read_Req_FIFO_Limit");
    if (rc != GT_OK)
    {
      return rc;
    }

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].PB_Read_Req_FIFO_Limit ;

    U32_SET_FIELD_MASKED_MAC(regValue,PRV_PDS_SIP_6_15_UNTIL_6_20_DEV_DEP_FIELD_GET_MAC(devNum,PB_READ_REQ_FIFO_LIMIT_PB_RD_REQ_FIFO_THRESHOLD_FIELD_OFFSET),
        PRV_PDS_SIP_6_15_UNTIL_6_20_DEV_DEP_FIELD_GET_MAC(devNum,PB_READ_REQ_FIFO_LIMIT_PB_RD_REQ_FIFO_THRESHOLD_FIELD_SIZE),
        pbReadReqFifoLimit);

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,0,&log,regAddr,0,32,regValue);
    if(rc!=GT_OK)
    {
        return rc;
    }

    return rc;
}

/**
* @internal prvCpssSip6TxqPdsDebugInterruptDisableSet function
* @endinternal
*
* @brief   Debug function that disable iterrupt in PDS
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqPdsDebugInterruptDisableSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);


    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"PDS_Interrupt_Summary_Mask");
    if (rc != GT_OK)
    {
      return rc;
    }

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].PDS_Interrupt_Summary_Mask,
                 0,32,0);
}

/**
* @internal prvCpssSip6TxqPdsDebugInterruptGet function
* @endinternal
*
* @brief   Get interrupt cause for PDS
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
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
GT_STATUS prvCpssSip6TxqPdsDebugInterruptGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    OUT GT_U32 *functionalPtr,
    OUT GT_U32 *debugPtr
)
{
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(functionalPtr);
    CPSS_NULL_PTR_CHECK_MAC(debugPtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].PDS_Interrupt_Functional_Cause,
                 0,32,functionalPtr);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].PDS_Interrupt_Debug_Cause,
                 0,32,debugPtr);

    return rc;
}




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
* @file prvCpssDxChTxqPfcc.c
*
* @brief CPSS SIP6 TXQ Pfcc low level configurations.
*
* @version   1
********************************************************************************
*/

/*Synced to \Cider \EBU-IP \TXQ_IP \SIP7.0 (Falcon) \TXQ_PR_IP \TXQ_PR_IP {7.0.11}*/


#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/falcon/central/prvCpssFalconTxqPfccRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5p/central/prvCpssAc5pTxqPfccRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5x/central/prvCpssAc5xTxqPfccRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/harrier/central/prvCpssHarrierTxqPfccRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ironman/central/prvCpssIronmanTxqPfccRegFile.h>

#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPfcc.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#define PRV_MAX_PFCC_RETRY_MAC 10

#define PRV_PFCC_SIP_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_30_CHECK_MAC(_dev)?TXQ_IRONMAN_##_fieldName:TXQ_HARRIER_##_fieldName)

#define PRV_PFCC_SIP_6_15_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_20_CHECK_MAC(_dev)? \
     (PRV_PFCC_SIP_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_PHOENIX_##_fieldName)

#define PRV_PFCC_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_15_CHECK_MAC(_dev)?\
     (PRV_PFCC_SIP_6_15_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_HAWK_##_fieldName)

#define TXQ_PFCC_FIELD_NAME_GET(_dev,_fieldName) (PRV_CPSS_SIP_6_10_CHECK_MAC(_dev)?\
    (PRV_PFCC_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):\
    (TXQ_##_fieldName))



#define PRV_PFCC_FIELD_GET_MAC(__devNum,_name,_data,_value) _value = U32_GET_FIELD_MAC(_data,\
                                                     TXQ_PFCC_FIELD_NAME_GET(__devNum,PFCC_##_name##_FIELD_OFFSET),\
                                                     TXQ_PFCC_FIELD_NAME_GET(__devNum,PFCC_##_name##_FIELD_SIZE))


#define PRV_TXQ_UNIT_NAME "PFCC"
#define PRV_TXQ_LOG_UNIT GT_FALSE
#define PRV_TXQ_LOG_REG GT_FALSE
#define PRV_TXQ_LOG_TABLE GT_FALSE



static GT_STATUS prvCpssSip6TxqPfccLogInit
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


static GT_STATUS prvCpssSip6TxqPccTableLogInit
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
* @internal prvCpssFalconTxqPfccCfgTableEntrySet function
* @endinternal
*
* @brief   Write  entry to PFCC CFG table
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - PP's device number.
* @param[in] tileNum          - tile number
* @param[in] index            - index of PFCC CFG table entry [0..285]
* @param[in] entryPtr         - PFCC CFG table entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPfccCfgTableEntrySet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 index,
    IN  PRV_CPSS_PFCC_CFG_ENTRY_STC * entryPtr
)
{

    GT_STATUS rc;
    GT_U32 regValue = 0;
    GT_U32 maxTableSize = PFCC_TABLE_SIZE_GET_MAC(devNum);
    PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC tableLog;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    if(index>=maxTableSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Index %d is bigger(equal)   then %d",index,maxTableSize);
    }

    if((entryPtr->entryType== PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC ||entryPtr->entryType== PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC_HR)&&
        (entryPtr->globalTc >= (GT_U32)1<<(TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_TC_FIELD_SIZE))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Tc %d is bigger(equal)   then %d",entryPtr->globalTc,1<<(TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_TC_FIELD_SIZE)));
    }


    if((entryPtr->entryType== PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT)&&
        (entryPtr->sourcePort >= (GT_U32)1<<(TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_PORT_FIELD_SIZE))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Port %d is bigger(equal)   then %d",entryPtr->sourcePort,1<<(TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_PORT_FIELD_SIZE)));
    }


    if((entryPtr->entryType== PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT)&&
        (entryPtr->tcBitVecEn>= (GT_U32)1<<(TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_TC_BIT_VEC_EN_FIELD_SIZE))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "tcBitVecEn %d is bigger(equal)  then %d",entryPtr->tcBitVecEn,1<<(TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_TC_BIT_VEC_EN_FIELD_SIZE)));
    }


    if((entryPtr->entryType== PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE)&&
        (entryPtr->numberOfBubbles>= (GT_U32)1<<(TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_NUM_OF_BUBBLES_FIELD_SIZE))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "numberOfBubbles %d is bigger(equal)  then %d",entryPtr->numberOfBubbles,1<<(TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_NUM_OF_BUBBLES_FIELD_SIZE)));
    }

     U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_ENTRY_TYPE_FIELD_OFFSET),
                            TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_ENTRY_TYPE_FIELD_SIZE),
                            entryPtr->entryType);

     if(entryPtr->entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE)
     {

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_NUM_OF_BUBBLES_FIELD_OFFSET),
                            TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_NUM_OF_BUBBLES_FIELD_SIZE),
                            entryPtr->numberOfBubbles);
     }
     else if (entryPtr->entryType== PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC ||
        entryPtr->entryType== PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC_HR)
     {

            U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_TC_FIELD_OFFSET),
                            TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_TC_FIELD_SIZE),
                            entryPtr->globalTc);
     }
     else
     {

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_PORT_FIELD_OFFSET),
                                TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_PORT_FIELD_SIZE),
                                entryPtr->sourcePort);

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_PFC_MESSAGE_TRIGGER_FIELD_OFFSET),
                                TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_PFC_MESSAGE_TRIGGER_FIELD_SIZE),
                                entryPtr->pfcMessageTrigger);

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_TC_BIT_VEC_EN_FIELD_OFFSET),
                                TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_TC_BIT_VEC_EN_FIELD_SIZE),
                                entryPtr->tcBitVecEn);
       }

      rc = prvCpssSip6TxqPccTableLogInit(&tableLog,0,
        CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_E+tileNum,"pfcc_CFG",index);
        if (rc != GT_OK)
        {
            return rc;
        }

       rc = prvCpssSip6TxqWriteTableEntry(devNum,&tableLog,
                                    CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_E+tileNum,
                                    index,
                                    &regValue);

       return rc;

}

/**
* @internal prvCpssFalconTxqPfccCfgTableEntryGet function
* @endinternal
*
* @brief   Read from  entry to PFCC CFG table
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - PP's device number.
* @param[in] tileNum          - tile number
* @param[in] index            - index of PFCC CFG table entry [0..285]
* @param[out] entryPtr        - PFCC CFG table entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPfccCfgTableEntryGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 index,
    OUT  PRV_CPSS_PFCC_CFG_ENTRY_STC * entryPtr
)
{

    GT_STATUS rc;
    GT_U32 regValue = 0;
    GT_U32 maxTableSize = PFCC_TABLE_SIZE_GET_MAC(devNum);;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    if(index>=maxTableSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Index %d is bigger(equal)   then %d",index,maxTableSize);
    }


    rc = prvCpssDxChReadTableEntry(devNum,
                                    CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_E+tileNum,
                                    index,
                                    &regValue);

     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccCfgTableEntryGet  failed \n");
     }

    entryPtr->entryType = (PRV_CPSS_PFCC_CFG_ENTRY_TYPE_ENT) U32_GET_FIELD_MAC(regValue,
                            TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_ENTRY_TYPE_FIELD_OFFSET),
                            TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_ENTRY_TYPE_FIELD_SIZE));

     if(entryPtr->entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE)
    {
        entryPtr->numberOfBubbles = U32_GET_FIELD_MAC(regValue,
                            TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_NUM_OF_BUBBLES_FIELD_OFFSET),
                            TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_NUM_OF_BUBBLES_FIELD_SIZE));
     }
     else if ((entryPtr->entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC)||
        (entryPtr->entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC_HR))
     {

        entryPtr->globalTc = U32_GET_FIELD_MAC(regValue,
                            TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_TC_FIELD_OFFSET),
                            TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_TC_FIELD_SIZE));
      }
     else
     {

        entryPtr->sourcePort = U32_GET_FIELD_MAC(regValue,
                                TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_PORT_FIELD_OFFSET),
                                TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_PORT_FIELD_SIZE));

        entryPtr->pfcMessageTrigger = (GT_BOOL)U32_GET_FIELD_MAC(regValue,
                                TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_PFC_MESSAGE_TRIGGER_FIELD_OFFSET),
                                TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_PFC_MESSAGE_TRIGGER_FIELD_SIZE));

        entryPtr->tcBitVecEn = U32_GET_FIELD_MAC(regValue,
                                TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_TC_BIT_VEC_EN_FIELD_OFFSET),
                                TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_PFCC_CFG_TC_BIT_VEC_EN_FIELD_SIZE));
      }



    return rc;

}

/**
* @internal prvCpssFalconTxqPfccUnitCfgGet function
* @endinternal
*
* @brief   Write to  PFCC unit global configuration register
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - PP's device number.
* @param[in] tileNum          - tile number
* @param[in] index            - index of PFCC CFG table entry [0..285]
* @param[in] initDbPtr        - PFCC unit configuration data base
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPfccUnitCfgSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  PRV_CPSS_PFCC_TILE_INIT_STC * initDbPtr
)
{
        GT_STATUS rc;
        GT_U32 regAddr,regValue = 0;
        PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
        GT_U32 maxTableSize = PFCC_TABLE_SIZE_GET_MAC(devNum);

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        if(initDbPtr->pfccLastEntryIndex>=maxTableSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "pfccLastEntryIndex %d is bigger(equal)   then %d",initDbPtr->pfccLastEntryIndex,maxTableSize);
        }

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_GLOBAL_PFCC_CFG_MASTER_EN_FIELD_OFFSET),
                            TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_GLOBAL_PFCC_CFG_MASTER_EN_FIELD_SIZE),
                            initDbPtr->isMaster);

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_GLOBAL_PFCC_CFG_LAST_PFCC_ENTRY_IDX_FIELD_OFFSET),
                           TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_GLOBAL_PFCC_CFG_LAST_PFCC_ENTRY_IDX_FIELD_SIZE),
                            initDbPtr->pfccLastEntryIndex);

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_GLOBAL_PFCC_CFG_PFCC_EN_FIELD_OFFSET),
                            TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_GLOBAL_PFCC_CFG_PFCC_EN_FIELD_SIZE),
                            initDbPtr->pfccEnable);

        rc = prvCpssSip6TxqPfccLogInit(&log,"global_pfcc_cfg");
        if (rc != GT_OK)
        {
          return rc;
        }
        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.global_pfcc_cfg;

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);

        return rc;


}

/**
* @internal prvCpssFalconTxqPfccUnitCfgGet function
* @endinternal
*
* @brief   Read PFCC unit global configuration register
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             - PP's device number.
* @param[in] tileNum            - tile number
* @param[in] index              - index of PFCC CFG table entry [0..285]
* @param[out] initDbPtr         - PFCC unit configuration data base
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPfccUnitCfgGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    OUT  PRV_CPSS_PFCC_TILE_INIT_STC  *initDbPtr
)
{
        GT_STATUS rc;
        GT_U32 regAddr,tileOffset,regValue = 0;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);


        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.global_pfcc_cfg;
        tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
        regAddr+=tileOffset;

         rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

         if(rc!=GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccUnitCfgGet  failed \n");
         }

        if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
        {
            /* the unit not exists in GM and it returns 0x00badadd */
            regValue = 0;
        }

        initDbPtr->isMaster =(GT_BOOL) U32_GET_FIELD_MAC(regValue,
                TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_GLOBAL_PFCC_CFG_MASTER_EN_FIELD_OFFSET),
                TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_GLOBAL_PFCC_CFG_MASTER_EN_FIELD_SIZE));


        initDbPtr->pfccEnable=(GT_BOOL)  U32_GET_FIELD_MAC(regValue,
                    TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_GLOBAL_PFCC_CFG_PFCC_EN_FIELD_OFFSET),
                    TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_GLOBAL_PFCC_CFG_PFCC_EN_FIELD_SIZE));



        initDbPtr->pfccLastEntryIndex = U32_GET_FIELD_MAC(regValue,
                     TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_GLOBAL_PFCC_CFG_LAST_PFCC_ENTRY_IDX_FIELD_OFFSET),
                     TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_GLOBAL_PFCC_CFG_LAST_PFCC_ENTRY_IDX_FIELD_SIZE));


         return rc;


}

/**
* @internal prvCpssFalconTxqPfccPortTcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value per  port and traffic class.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - PP's device number.
* @param[in] masterTileNum          - master tile number
* @param[in] tc                     - traffic class [0..7](packet attribute) or 0xFF for source port
* @param[in] dmaNumber              - Source port dma number
* @param[out] pfcCounterValuePtr    - counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPfccPortTcCounterGet
(
  IN  GT_U8   devNum,
  IN  GT_U32  masterTileNum,
  IN  GT_U8   tc,
  IN  GT_U32  dmaNumber,
  IN  GT_U32   physicalPortNumber,
  OUT GT_U32  *pfcCounterValuePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr,tileOffset,regValue = 0;
    GT_U32 counter = 0;
    GT_BOOL dataValid = GT_FALSE;
    GT_U32 startIndex,localPort,dpNum,tileNum;
    PRV_CPSS_PFCC_CFG_ENTRY_STC  entry;
    GT_BOOL                      entryValidForRead;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(masterTileNum);

    /*First check if  PFCC table contain this entry*/

    rc = prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert(devNum,dmaNumber,&tileNum,&dpNum,&localPort);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert  failed \n");
    }

    startIndex = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)*localPort+dpNum+tileNum*(MAX_DP_IN_TILE(devNum));
    rc = prvCpssFalconTxqPfccCfgTableEntryGet(devNum,masterTileNum,startIndex,&entry);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccCfgTableEntryGet  failed \n");
    }

    entryValidForRead = GT_FALSE;

    if(entry.entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT)
    {
        if(tc  == PRV_CPSS_DXCH_SIP6_TXQ_ALL_Q_MAC)
        {
            entryValidForRead = GT_TRUE;
        }
        else
        {
            if(entry.tcBitVecEn&1<<tc)
            {
                entryValidForRead = GT_TRUE;
            }
        }
    }



    if(GT_TRUE == entryValidForRead)
    {
        /*Write source port to register*/
        rc = prvCpssSip6TxqPfccLogInit(&log,"Source_Port_Requested_For_Read");
        if (rc != GT_OK)
        {
          return rc;
        }
        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Source_Port_Requested_For_Read;


        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_SOURCE_PORT_REQUESTED_FOR_READ_SOURCE_PORT_REQUESTED_TO_READ_FIELD_OFFSET),
                                TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_SOURCE_PORT_REQUESTED_FOR_READ_SOURCE_PORT_REQUESTED_TO_READ_FIELD_SIZE),
                                physicalPortNumber);

         rc = prvCpssSip6TxqRegisterFieldWrite(devNum,masterTileNum,&log,regAddr,0,32,regValue);

         if(rc!=GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldWrite  failed \n");
         }

          /*Busy wait until the value is latched*/
          while(counter <300)counter++;

         /*Now read*/
          counter = 0;
          tileOffset = prvCpssSip6TileOffsetGet(devNum,masterTileNum);
          regAddr+=tileOffset;

            while(counter<PRV_MAX_PFCC_RETRY_MAC)
            {
                 rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

                 if(rc!=GT_OK)
                 {
                        return rc;
                 }

#ifdef ASIC_SIMULATION  /* Yet not implemented in simulation */
                 dataValid = GT_TRUE;
#else
                if(tc  == PRV_CPSS_DXCH_SIP6_TXQ_ALL_Q_MAC)
                {
                     dataValid = (GT_BOOL)U32_GET_FIELD_MAC(regValue,TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_SOURCE_PORT_REQUESTED_FOR_READ_SOURCE_PORT_COUNTER_FOR_DEBUG_VALID_FIELD_OFFSET),
                                            TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_SOURCE_PORT_REQUESTED_FOR_READ_SOURCE_PORT_COUNTER_FOR_DEBUG_VALID_FIELD_SIZE));
                }
                else
                {
                     dataValid = (GT_BOOL)U32_GET_FIELD_MAC(regValue,TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_SOURCE_PORT_REQUESTED_FOR_READ_SOURCE_PORT_TC_COUNTER_FOR_DEBUG_VALID_FIELD_OFFSET)+tc,
                                            TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_SOURCE_PORT_REQUESTED_FOR_READ_SOURCE_PORT_TC_COUNTER_FOR_DEBUG_VALID_FIELD_SIZE) );

                }
#endif

                 if(dataValid == GT_TRUE)
                 {
                     break;
                 }
                 else
                 {
                     counter++;
                 }
          }

         if(counter ==PRV_MAX_PFCC_RETRY_MAC)
          {
                rc = GT_FAIL;
          }
          else
          {
             if(tc  == PRV_CPSS_DXCH_SIP6_TXQ_ALL_Q_MAC)
             {
                regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Source_Port_Counter;
                tileOffset = prvCpssSip6TileOffsetGet(devNum,masterTileNum);
                regAddr+=tileOffset;
                rc = prvCpssHwPpGetRegField(devNum,regAddr,
                 TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_SOURCE_PORT_COUNTER_SOURCE_PORT_COUNTER_VALUE_FOR_DEBUG_FIELD_OFFSET),
                 TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_SOURCE_PORT_COUNTER_SOURCE_PORT_COUNTER_VALUE_FOR_DEBUG_FIELD_SIZE),pfcCounterValuePtr);
             }
             else
             {
                if(tc>= CPSS_TC_RANGE_CNS)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Source_Port_Counter_value[tc];
                tileOffset = prvCpssSip6TileOffsetGet(devNum,masterTileNum);
                regAddr+=tileOffset;

                rc = prvCpssHwPpGetRegField(devNum,regAddr,
                 TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_SOURCE_PORT_COUNTER_VALUE_SOURCE_PORT_TC_COUNTER_VALUE_FOR_DEBUG_FIELD_OFFSET),
                 TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_SOURCE_PORT_COUNTER_VALUE_SOURCE_PORT_TC_COUNTER_VALUE_FOR_DEBUG_FIELD_SIZE),pfcCounterValuePtr);
             }
           }

          return rc;
    }
    else
    {
         /*No threshold was set for this port/TC . No information is availeble*/
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

}

/**
* @internal prvCpssFalconTxqPfccPortTcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value per  global  traffic class.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - PP's device number.
* @param[in] tileNum               - tile number
* @param[in] tc                    - traffic class [0..7](packet attribute)
* @param[out] pfcCounterValuePtr   - counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPfccGlobalTcCounterGet
(
  IN  GT_U8   devNum,
  IN  GT_U32  tileNum,
  IN  GT_U8   tc,
  OUT GT_U32  *pfcCounterValuePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr,tileOffset,regData;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Global_status_counters[tc];
    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regData);
    if(rc!=GT_OK)
    {
        return rc;
    }

    *pfcCounterValuePtr = U32_GET_FIELD_MAC(regData,TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_GLOBAL_STATUS_COUNTERS_TC_COUNTER_VAL_FIELD_OFFSET),
        TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_GLOBAL_STATUS_COUNTERS_TC_COUNTER_VAL_FIELD_SIZE));

   return rc;
}

/**
 * @internal  prvCpssFalconTxqPfccMapTcToPoolSet function
 * @endinternal
 *
 * @brief  Get tail drop mapping between TC to pool
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                        - Device number
 * @param[in] trafClass                     - Traffic class [0..7]
 * @param[in] poolIdPtr                     - Pool id[0..1]
 * @param[in] update_EGF_QAG_TC_DP_MAPPER   - indication to update the CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E
 *                                             (during init must be 'GT_FALSE' to reduce the number of HW accessing !)
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS prvCpssFalconTxqPfccMapTcToPoolSet
(
    IN  GT_U8                                devNum,
    IN  GT_U8                                trafClass,
    IN  GT_U32                               poolId,
    IN  GT_BOOL                              update_EGF_QAG_TC_DP_MAPPER
)
{
    GT_U32 regAddr;
    GT_STATUS rc;
    GT_U32 offset,tileNum,numberOfTiles;
    GT_U32 oldPoolId=0;
    GT_U32  i,j,chunkValue;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(trafClass);

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

    if( poolId >= SIP6_SHARED_REGULAR_POOLS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for(tileNum=0;tileNum<numberOfTiles;tileNum++)
    {
        /*Get old mapping*/
         rc = prvCpssFalconTxqPfccMapTcToPoolGet(devNum,tileNum,trafClass,GT_FALSE,&oldPoolId);
         if(rc!=GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccMapTcToPoolGet  failed \n");
         }
         if(oldPoolId == poolId)
         {  /*Same value - do nothing*/
            return GT_OK;
         }
         rc = prvCpssSip6TxqPfccLogInit(&log,"pool_CFG");
         if (rc != GT_OK)
         {
           return rc;
         }

         regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.pool_CFG;


         offset = trafClass;

         if(poolId==1)
         {
            offset+=TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_POOL_CFG_POOL_1_TC_EN_VEC_FIELD_OFFSET);
         }
         else if(poolId==2)
         {
            offset+=TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_POOL_CFG_POOL_2_TC_EN_VEC_FIELD_OFFSET);
         }
          else if(poolId==3)
         {
            offset+=TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_POOL_CFG_POOL_3_TC_EN_VEC_FIELD_OFFSET);
         }

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,offset,1,1);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccMapTcToPoolSet  failed \n");
        }

        /*delete from old pool*/
        if(oldPoolId!=SIP6_SHARED_TOTAL_POOLS_NUM_CNS)
        {
            offset = trafClass;

            if(oldPoolId==1)
            {
               offset+=TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_POOL_CFG_POOL_1_TC_EN_VEC_FIELD_OFFSET);
            }
            else if(oldPoolId==2)
            {
               offset+=TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_POOL_CFG_POOL_2_TC_EN_VEC_FIELD_OFFSET);
            }
             else if(oldPoolId==3)
            {
               offset+=TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_POOL_CFG_POOL_3_TC_EN_VEC_FIELD_OFFSET);
            }
            rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,offset,1,0);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccMapTcToPoolGet  failed \n");
            }
         }

           /*update TC_to_pool_CFG if required*/
          rc =prvCpssFalconTxqPfccTcResourceModeSet(devNum,tileNum,trafClass,GT_FALSE,poolId,GT_FALSE,GT_FALSE);
          if(rc!=GT_OK)
          {
              CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccTcResourceModeSet  failed \n");
          }
     }


    if(update_EGF_QAG_TC_DP_MAPPER == GT_FALSE)
    {
        /* must not update the entries during initialization , because it modify
           same entries over and over again ! */
        /* saves 220608 times of calling
           prvCpssDxChWriteTableEntryField(..CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E..)

           this reduce cpssInitSystem time for GM from 37 seconds to 8 seconds (in VC10).
        */

        return GT_OK;
    }
    /*Now sync all the EGF entries that are mapped to this tc4Pfc.
            Table size is 4K ,each cell in the shadow array represent 32 entries,hence the array size is 128 (4K/32).

            Note:

        Tail-Drop/PFC pool usage is scaled to work well with up to two pools (0,1)
        When using pools #2,3 we have some limitations

        4 pools limitations:
        -        Tail Drop cannot be applied on pools 1,2,3
        -        CP will map all PFC flows to pool1 for tail drop
        -        Pool1 TailDrop must be disabled
        -        Only global taildrop is enabled


        If TC is mapped to pool2 or pool3 , in EGF  pool1 will be configured in this entry.

        Application responsibility is  to disable Pool1 TailDrop since the counting will be incorrect.

        See below table

        |Pool that TC is mapped at CPSS                       |                                      |                                                         |
        |(cpssDxChPortTxTcMapToSharedPoolSet)   | Pool mapped at txQ  |          Pool mapped at EGF table   |
        ======================================================================
       | 0                                                                            |      0                             |                     0                                   |
       | 1                                                                            |      1                             |                     1                                   |
       | 2                                                                            |      2                             |                     1                                   |
       | 3                                                                            |      3                             |                     1                                    |
        */

    /*update only if relevant*/
    if((oldPoolId==0 &&poolId>0)||(oldPoolId>0 &&poolId==0))
    {
        for(i=0;i<128;i++)
        {
           chunkValue = PRV_CPSS_DXCH_PP_MAC(devNum)->tc4PfcToEgfIndex[trafClass][i];
           for(j=0;chunkValue&&(j<32);j++,chunkValue>>=1)
           {
               if(chunkValue&0x1)
               {
                   rc = prvCpssDxChWriteTableEntryField(devNum,
                       CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E,
                       (i<<5)+j,
                       PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,9,1,
                       (poolId==0)?0:1);

                    if(rc!=GT_OK)
                   {
                       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Updating EGF_QAG_TC_DP_MAPPER table failed \n");
                   }
               }
           }
        }
     }

    return GT_OK;

}

/**
 * @internal  prvCpssFalconTxqPfccMapTcToPoolGet function
 * @endinternal
 *
 * @brief  Get tail drop mapping between TC to pool
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number
 * @param[in] tileNum                  - Tile number
 * @param[in] trafClass                - Traffic class [0..7]
 * @param[in] extPoolQuery             - GT_TRUE if the querry is regarding extended pools,GT_FALSE otherwise.
 * @param[out] poolIdPtr                (Pointer to)Pool id[0..1]
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS prvCpssFalconTxqPfccMapTcToPoolGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U8                                trafClass,
    IN  GT_BOOL                              extPoolQuery,
    OUT  GT_U32                              *poolIdPtr
)
{
    GT_U32 rc;
    GT_U32 regValue;
    GT_U32 regAddr,tileOffset;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(trafClass);

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.pool_CFG;
    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    *poolIdPtr=SIP6_SHARED_TOTAL_POOLS_NUM_CNS;/*unmapped by default*/

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccMapTcToPoolGet  failed \n");
    }

    if(GT_FALSE == extPoolQuery)
    {
        if((regValue>>TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_POOL_CFG_POOL_TC_EN_VEC_FIELD_OFFSET))&(1<<trafClass))
        {
            *poolIdPtr=0;
        }
        else if((regValue>>TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_POOL_CFG_POOL_1_TC_EN_VEC_FIELD_OFFSET))&(1<<trafClass))
        {
            *poolIdPtr=1;
        }
    }
    else
    {
        if((regValue>>TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_POOL_CFG_POOL_2_TC_EN_VEC_FIELD_OFFSET))&(1<<trafClass))
        {
            *poolIdPtr=2;
        }
        else if((regValue>>TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_POOL_CFG_POOL_3_TC_EN_VEC_FIELD_OFFSET))&(1<<trafClass))
        {
            *poolIdPtr=3;
        }
    }

    return GT_OK;
}

/**
 * @internal  prvCpssFalconTxqPfccTcMapVectorGet function
 * @endinternal
 *
 * @brief  Get tail drop mapping vector between TC to pool
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                  - Device number
 * @param[in] tileNum                 - Tile number
 * @param[out]valuePtr                - (Pointer to)Mapping vector
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS prvCpssFalconTxqPfccTcMapVectorGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    OUT GT_U32                               *valuePtr
)
{
    GT_U32 regAddr,tileOffset;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.pool_CFG;
    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    return prvCpssHwPpReadRegister(devNum,regAddr,valuePtr);
}

/**
* @internal prvCpssFalconTxqPfccTcResourceModeSet
* @endinternal
*
* @brief  Configure mode that count occupied buffers (Packet buffer occupied or Pool occupied).
*         In case Pool mode is selected ,headroom subtraction  can be enabled by settin mode to
*         CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum            - physical device number
* @param[in] trafClass         - Traffic class [0..7]
* @param[in] forceSet          - If GT_TRUE set resource mode regardless of currently congiured mode,
                                 else if resourse mode is PB do not set currenly requested mode
* @param[in] poolId            - Pool ID to configure
* @param[in] configureHeadroom - If GT_TRUE configure headroom subtruction field ,
                                 else do not configure headroom subtruction field
* @param[in] headroomEnable    -  headroom subtraction mode,only relevant if pool id is not equal PB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqPfccTcResourceModeSet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U8                                trafClass,
    IN  GT_BOOL                              forceSet,
    IN  GT_U32                               poolId,
    IN  GT_BOOL                              configureHeadroom,
    IN  GT_BOOL                              headroomEnable
)
{
    GT_U32    regAddr,regValue,globalTcHrIndex;
    GT_STATUS rc;
    GT_BOOL oldHeadRoom;
    PRV_CPSS_PFCC_CFG_ENTRY_STC  entry;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_U32  modeBit;

     /*Read current */
    rc = prvCpssFalconTxqPfccTcResourceModeGet(devNum,tileNum,trafClass,&regValue,&oldHeadRoom);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccTcResourceModeGet  failed \n");
    }

    if(regValue!=PRV_TC_MAP_PB_MAC)
    {
        forceSet = GT_TRUE;
    }

    rc = prvCpssSip6TxqPfccLogInit(&log,"TC_to_pool_CFG");
    if (rc != GT_OK)
    {
      return rc;
    }
    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.TC_to_pool_CFG;


    if((forceSet == GT_TRUE)&&(poolId!=regValue))
    {
        regValue = 0;
        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,
            regAddr,(TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_TC_TO_POOL_CFG_TC_TO_POOL_MAPPING_FIELD_SIZE)*trafClass),
            TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_TC_TO_POOL_CFG_TC_TO_POOL_MAPPING_FIELD_SIZE),poolId);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldWrite  failed \n");
        }
    }
    /*
             From Cider :
            =========
             0x0 = Shared pool; shared_pool; The amount of published occupied buffers is shown in the pool_counter-pool_HR_counter.
             0x1 = total; total; The amount of published occupied buffers is shown in pool_counter.
        */
    if((GT_TRUE==configureHeadroom)&&(headroomEnable!=oldHeadRoom))
    {
        if(GT_TRUE == headroomEnable)
        {
            modeBit = 0x0;
        }
        else
        {
            modeBit = 0x1;
        }
        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,
           regAddr,(TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_TC_TO_POOL_CFG_TC_POOL_OCCUPIED_BUFFERS_MODE_FIELD_OFFSET)+trafClass),
           TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_TC_TO_POOL_CFG_TC_POOL_OCCUPIED_BUFFERS_MODE_FIELD_SIZE),modeBit);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldWrite  failed \n");
        }

        /*Configure calendar*/

        if(GT_TRUE == headroomEnable)
        {
            entry.entryType = PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC_HR;
            entry.globalTc = trafClass;
        }
        else
        {
            entry.entryType = PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE;
            entry.numberOfBubbles = 1;
        }

        /*After global TC*/
        globalTcHrIndex = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)*CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum)+8;
        rc =prvCpssFalconTxqPfccCfgTableEntrySet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,
            globalTcHrIndex+trafClass,&entry);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccCfgTableEntrySet  failed \n");
        }

    }

    return GT_OK;
}

/**
* @internal prvCpssFalconTxqPfccTcResourceModeGet
* @endinternal
*
* @brief  Get configured  mode that count occupied buffers (Packet buffer occupied or Pool occupied).
*         In case Pool mode is selected ,headroom subtraction  can be enabled by settin mode to
*         CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - physical device number
* @param[in] trafClass      - Traffic class [0..7]
* @param[in] poolIdPtr      - (pointer to)Pool ID to configured
* @param[in] headRoomPtr    - (pointer to)headroom subtraction mode,only relevant if pool id is not equal PB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqPfccTcResourceModeGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U8                                trafClass,
    IN  GT_U32                               *poolIdPtr,
    IN  GT_BOOL                              *headRoomPtr
)
{
    GT_U32    regAddr,tileOffset,regData;
    GT_STATUS rc;
    GT_U32    modeBit;

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.TC_to_pool_CFG;
    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regData);
    if(rc!=GT_OK)
    {
        return rc;
    }

    *poolIdPtr = U32_GET_FIELD_MAC(regData,(TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_TC_TO_POOL_CFG_TC_TO_POOL_MAPPING_FIELD_SIZE)*trafClass),
        TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_TC_TO_POOL_CFG_TC_TO_POOL_MAPPING_FIELD_SIZE));

    modeBit= (GT_BOOL)U32_GET_FIELD_MAC(regData,(TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_TC_TO_POOL_CFG_TC_POOL_OCCUPIED_BUFFERS_MODE_FIELD_OFFSET)+trafClass),
        TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_TC_TO_POOL_CFG_TC_POOL_OCCUPIED_BUFFERS_MODE_FIELD_SIZE));
    /*
        0x0 = Shared pool; shared_pool; The amount of published occupied buffers is shown in the pool_counter-pool_HR_counter.
         0x1 = total; total; The amount of published occupied buffers is shown in pool_counter.
        */
    if(0 == modeBit)
    {
        *headRoomPtr = GT_TRUE;
    }
    else
    {
        *headRoomPtr = GT_FALSE;
    }

    return rc;
}

/**
* @internal prvCpssFalconTxqPfccHeadroomCounterGet
* @endinternal
*
* @brief  Get headroom size counter per TC or per POOL
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] tileNum                - Tile number
* @param[in] trafClass              - Traffic class [0..7]
* @param[in] poolId                 - Pool ID [0..1]
* @param[in] hrType                 - TC or POOL
* @param[out] currentCountValPtr    - (pointer to)current headroom size
* @param[out] maxCountValPtr        - (pointer to)maximal headroom size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqPfccHeadroomCounterGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      tileNum,
    IN  GT_U32      tc,
    IN  GT_U32      poolId,
    IN  PRV_CPSS_DXCH_TXQ_SIP_6_PFCC_HEADROOM_TYPE_ENT      hrType,
    OUT GT_U32     *currentCountValPtr,
    OUT GT_U32     *maxCountValPtr
)
{
    GT_U32    regAddr[2],tileOffset,regData[2],i;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);

    if(hrType == PRV_CPSS_DXCH_TXQ_SIP_6_PFCC_HEADROOM_TYPE_GLOBAL_TC)
    {
       regAddr[0] = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Global_HR_status_counters[tc];
       regAddr[1] = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Global_TC_HR_Counter_Max_Peak[tc];
    }
    else
    {
       regAddr[0] = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Pool_HR_counter_status[poolId];
       regAddr[1] = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Pool_HR_Counter_Max_Peak[poolId];
    }

    for(i=0;i<2;i++)
    {
      regAddr[i] += tileOffset;

      rc = prvCpssHwPpReadRegister(devNum,regAddr[i],regData+i);
      if(rc!=GT_OK)
      {
          return rc;
      }
    }

     if(hrType == PRV_CPSS_DXCH_TXQ_SIP_6_PFCC_HEADROOM_TYPE_GLOBAL_TC)
    {
       PRV_PFCC_FIELD_GET_MAC(devNum,GLOBAL_HR_STATUS_COUNTERS_GLOBAL_TC_HR_COUNTER_VALUE,regData[0],*currentCountValPtr);
       PRV_PFCC_FIELD_GET_MAC(devNum,GLOBAL_TC_HR_COUNTER_MAX_PEAK_GLOBAL_TC_HR_COUNTER_MAX_VAL,regData[1],*maxCountValPtr);
    }
    else
    {

       PRV_PFCC_FIELD_GET_MAC(devNum,POOL_HR_COUNTER_STATUS_POOL_HR_COUNTER_VALUE,regData[0],*currentCountValPtr);
       PRV_PFCC_FIELD_GET_MAC(devNum,POOL_HR_COUNTER_MAX_PEAK_POOL_HR_COUNTER_MAX_VAL,regData[1],*maxCountValPtr);
    }


    return GT_OK;
}

/**
* @internal prvCpssSip6TxqPoolCounterGet
* @endinternal
*
* @brief  Get POOL size
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - physical device number
* @param[in] poolId         - Pool ID [0..3]
* @param[out] countValPtr   - pointer to)pool size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6TxqPoolCounterGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      poolId,
    OUT GT_U32      *countValPtr
)
{
    GT_U32    regAddr,regData = 0,tileOffset;
    GT_STATUS rc;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_XCAT3_E | CPSS_AC5_E);

    CPSS_NULL_PTR_CHECK_MAC(countValPtr);

    if( poolId >= SIP6_SHARED_TOTAL_POOLS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    tileOffset = prvCpssSip6TileOffsetGet(devNum,PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC);

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Pool_counter[poolId];

    regAddr += tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regData);
    if(rc!=GT_OK)
    {
        return rc;
    }

    *countValPtr = U32_GET_FIELD_MAC(regData,TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_COUNTER_STATUS_POOL_VALUE_FIELD_OFFSET),
        TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_COUNTER_STATUS_POOL_VALUE_FIELD_SIZE));

    return GT_OK;
}

/**
 * @internal  prvCpssFalconTxqPfccMapTcToExtendedPoolSet function
 * @endinternal
 *
 * @brief Map TC to extended pool.Done for counting purpose only.
 *
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number
 * @param[in] trafClass                - Traffic class [0..7]
 * @param[in]  extPoolId               - Extended Pool id[0..1]
 *
 * @retval GT_OK                       - on success
 * @retval GT_FAIL                     - on error
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_BAD_PARAM                - on invalid input parameters value
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 * @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS prvCpssFalconTxqPfccMapTcToExtendedPoolSet
(
    IN  GT_U8                                devNum,
    IN  GT_U8                                trafClass,
    IN  GT_U32                               extPoolId
)
{
    GT_U32 regAddr;
    GT_STATUS rc;
    GT_U32 offset,tileNum,numberOfTiles;
    GT_U32 oldPoolId;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;


    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(trafClass);

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

    if( extPoolId >= SIP6_SHARED_EXT_POOLS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    extPoolId+= SIP6_SHARED_REGULAR_POOLS_NUM_CNS;

    for(tileNum=0;tileNum<numberOfTiles;tileNum++)
    {
        /*Get old mapping */
         rc = prvCpssFalconTxqPfccMapTcToPoolGet(devNum,tileNum,trafClass,GT_TRUE,&oldPoolId);
         if(rc!=GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccMapTcToPoolGet  failed \n");
         }
         if(oldPoolId == extPoolId)
         {  /*Same value - do nothing*/
            return GT_OK;
         }

         rc = prvCpssSip6TxqPfccLogInit(&log,"pool_CFG");
         if (rc != GT_OK)
         {
          return rc;
         }

         regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.pool_CFG;

         offset = trafClass;

         if(extPoolId==2)
         {
            offset+=TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_POOL_CFG_POOL_2_TC_EN_VEC_FIELD_OFFSET);
         }
          else if(extPoolId==3)
         {
            offset+=TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_POOL_CFG_POOL_3_TC_EN_VEC_FIELD_OFFSET);
         }

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,offset,1,1);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccMapTcToPoolSet  failed \n");
        }

        /*delete from old pool*/
        if(oldPoolId!=SIP6_SHARED_TOTAL_POOLS_NUM_CNS)
        {
            offset = trafClass;

            if(oldPoolId==2)
            {
               offset+=TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_POOL_CFG_POOL_2_TC_EN_VEC_FIELD_OFFSET);
            }
             else if(oldPoolId==3)
            {
               offset+=TXQ_PFCC_FIELD_NAME_GET(devNum,PFCC_POOL_CFG_POOL_3_TC_EN_VEC_FIELD_OFFSET);
            }
            rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,offset,1,0);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccMapTcToPoolGet  failed \n");
            }
         }

     }

    return GT_OK;

}

/**
* @internal prvCpssSip6TxqPfccDebugInterruptDisableSet function
* @endinternal
*
* @brief   Debug function that disable iterrupt in PFCC
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqPfccDebugInterruptDisableSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    rc = prvCpssSip6TxqPfccLogInit(&log,"PFCC_Interrupt_Mask");
    if (rc != GT_OK)
    {
      return rc;
    }

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.PFCC_Interrupt_Mask,
                 0,32,0);
}
/**
* @internal prvCpssSip6TxqSdqDebugInterruptGet function
* @endinternal
*
* @brief   Get interrupt cause for PFCC
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
GT_STATUS prvCpssSip6TxqPfccDebugInterruptGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    OUT GT_U32 *interruptPtr
)
{

    CPSS_NULL_PTR_CHECK_MAC(interruptPtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    return  prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.PFCC_Interrupt_Cause,
                 0,32,interruptPtr);
}


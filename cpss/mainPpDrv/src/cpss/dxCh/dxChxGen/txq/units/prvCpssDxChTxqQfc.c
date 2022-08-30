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
* @file prvCpssDxChTxqQfc.c
*
* @brief CPSS SIP6 TXQ Qfc low level configurations.
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/falcon/pipe/prvCpssFalconTxqQfcRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5p/pipe/prvCpssAc5pTxqQfcRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5x/pipe/prvCpssAc5xTxqQfcRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/harrier/pipe/prvCpssHarrierTxqQfcRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ironman/pipe/prvCpssIronmanTxqQfcRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqQfc.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_MAX_QFC_RETRY_MAC 10

#define PRV_MAX_QFC_BUSY_WAIT_CYCLE_MAC 300


#define PRV_QFC_SIP_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_30_CHECK_MAC(_dev)?TXQ_IRONMAN_QFC_##_fieldName:TXQ_HARRIER_QFC_##_fieldName)


#define PRV_QFC_SIP_6_15_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_20_CHECK_MAC(_dev)?\
    (PRV_QFC_SIP_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_PHOENIX_QFC_##_fieldName)


#define PRV_QFC_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_15_CHECK_MAC(_dev)?\
     (PRV_QFC_SIP_6_15_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_HAWK_QFC_##_fieldName)

#define PRV_QFC_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) (PRV_CPSS_SIP_6_10_CHECK_MAC(_dev)?\
    (PRV_QFC_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):\
    (TXQ_QFC_##_fieldName))

#define PRV_QFC_REG_FIELD_MAC(_devNum,_name)  \
     PRV_QFC_DEV_DEP_FIELD_GET_MAC(_devNum, _name##_FIELD_OFFSET),PRV_QFC_DEV_DEP_FIELD_GET_MAC(_devNum, _name##_FIELD_SIZE)


#define PRV_QFC_TABLE_FIELD_MAC(_devNum,_name)  PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,\
     PRV_QFC_DEV_DEP_FIELD_GET_MAC(_devNum, _name##_FIELD_OFFSET),PRV_QFC_DEV_DEP_FIELD_GET_MAC(_devNum, _name##_FIELD_SIZE)

#define PRV_QFC_TABLE_MAC(_table,_tile,_qfc)  _table+_qfc+_tile*CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC

#define PRV_QFC_FIELD_SET_MAC(_devNum,_name,_rawData,_valueToSet)    U32_SET_FIELD_MASKED_MAC(_rawData,\
                                                     PRV_QFC_DEV_DEP_FIELD_GET_MAC(_devNum, _name##_FIELD_OFFSET),\
                                                     PRV_QFC_DEV_DEP_FIELD_GET_MAC(_devNum, _name##_FIELD_SIZE),\
                                                     _valueToSet)

#define PRV_QFC_FIELD_GET_WITH_TYPE_MAC(_devNum,_name,_rawData,_valueToGet,_type) _valueToGet =(_type) U32_GET_FIELD_MAC(_rawData,\
                                                     PRV_QFC_DEV_DEP_FIELD_GET_MAC(_devNum, _name##_FIELD_OFFSET),\
                                                     PRV_QFC_DEV_DEP_FIELD_GET_MAC(_devNum, _name##_FIELD_SIZE))

#define PRV_QFC_DEV_SPEC_FIELD_GET_WITH_TYPE_MAC(_name,_rawData,_valueToGet,_type) _valueToGet =(_type) U32_GET_FIELD_MAC(_rawData,\
                                                     _name##_FIELD_OFFSET,\
                                                     _name##_FIELD_SIZE)



#define PRV_QFC_FIELD_GET_MAC(_devNum,_name,_rawData,_valueToGet) PRV_QFC_FIELD_GET_WITH_TYPE_MAC(_devNum,_name,_rawData,_valueToGet,GT_U32)


#define PRV_QFC_UBURST_TRIGGER_SW_2_HW_FORMAT(_typeInSwFormat,_typeInHwFormat)\
    do\
    {\
        switch(_typeInSwFormat)\
        {\
            case CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_NOT_VALID_E:\
                _typeInHwFormat = 0x0;\
                break;\
           case CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_FILL_E:\
                _typeInHwFormat = 0x1;\
                break;\
           case CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_DRAIN_E:\
                _typeInHwFormat = 0x2;\
                break;\
           default:\
                _typeInHwFormat = 0x3;\
                break;\
        }\
     }\
     while(0);

 #define PRV_QFC_UBURST_TRIGGER_HW_2_SW_FORMAT(_typeInSwFormat,_typeInHwFormat)\
    do\
    {\
        switch(_typeInHwFormat)\
        {\
            case 0x0:\
                _typeInSwFormat = CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_NOT_VALID_E;\
                break;\
           case 0x1:\
                _typeInSwFormat = CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_FILL_E;\
                break;\
           case 0x2:\
                _typeInSwFormat = CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_DRAIN_E;\
                break;\
           case 0x3:\
                _typeInSwFormat = CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_EITHER_E;\
                break;\
           default:\
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid trigger mode");\
                break;\
        }\
     }\
     while(0);


#define PRV_QFC_UBURST_INTERRUPT_TYPE_HW_2_SW_FORMAT(_typeInSwFormat,_typeInHwFormat)\
   do\
   {\
       switch(_typeInHwFormat)\
       {\
           case 0x0:\
               _typeInSwFormat = CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_DRAIN_E;\
               break;\
          case 0x1:\
               _typeInSwFormat = CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_FILL_E;\
               break;\
          default:\
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid trigger mode");\
               break;\
       }\
    }\
    while(0);

#define PRV_TXQ_UNIT_NAME "QFC"
#define PRV_TXQ_LOG_UNIT GT_FALSE

GT_STATUS prvCpssSip6TxqQfcLogInit
(
   INOUT PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC * logDataPtr,
   IN GT_U32                                   qfcNum,
   IN GT_CHAR_PTR                              regName
)
{
    CPSS_NULL_PTR_CHECK_MAC(logDataPtr);

    logDataPtr->log = PRV_TXQ_LOG_UNIT;
    logDataPtr->unitName = PRV_TXQ_UNIT_NAME;
    logDataPtr->unitIndex = qfcNum;
    logDataPtr->regName = regName;
    logDataPtr->regIndex = PRV_TXQ_LOG_NO_INDEX;

    return GT_OK;
}

/**
* @internal prvCpssFalconTxqQfcPortBufNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated per specified local port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               - device number
* @param[in] tileNum              - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum               - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] index                - if perPort then local port number (APPLICABLE RANGES:0..8) else local queue number (APPLICABLE RANGES:0..399).
* @param[in] perPort              - if equal GT_TRUE then it is per port querry,else per queue
*
* @param[out] numPtr              - number of buffers
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcBufNumberGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileNum,
    IN  GT_U32  qfcNum,
    IN  GT_U32  index,
    IN  GT_BOOL perPort,
    OUT  GT_U32 *numPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr,tileOffset,regValue;
    GT_U32 counter = 0;
    GT_BOOL dataValid = GT_FALSE;
    GT_BOOL supportTable = GT_TRUE;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"Counter_Table_1_Indirect_Read_Address");
    if (rc != GT_OK)
    {
      return rc;
    }

    if(perPort == GT_TRUE)
    {
        TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,index);
         regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Counter_Table_1_Indirect_Read_Address ;
    }
    else
    {
        if(index >= CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        log.unitName="Counter_Table_0_Indirect_Read_Address";
        regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Counter_Table_0_Indirect_Read_Address ;
    }

    if ((PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType == CPSS_CHANNEL_PEX_FALCON_Z_E)||
        (PRV_CPSS_PP_MAC(devNum)->isGmDevice))
    {
        supportTable = GT_FALSE;
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if ( (  (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) ||
            (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) )&&
            (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E ) )
    {
        /* same conditions as in 'SYSTEM_STATE_CAUSED_SKIP_MAC' that used inside prvCpssHwPpWriteRegister --> prvCpssDrvHwPpWriteRegBitMaskDrv */
        /* the write to_Indirect_Read_Address trigger the 'read request' is going to be ignored ! */
        /* so the read from _Indirect_Read_Data hold no meaning !                                 */
        /* so treat it as table not supported                                                     */
        supportTable = GT_FALSE;
    }

    if(supportTable == GT_FALSE)
    {
        /* the 'polling' on end of action below will 'never' come , so just state 'no buffers' ... as no eagle exists */
        *numPtr = 0;
        return GT_OK;
    }

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,index);
    if(rc!=GT_OK)
    {
        return rc;
    }

     /*Busy wait until the value is latched*/
    while(counter <PRV_MAX_QFC_BUSY_WAIT_CYCLE_MAC)
    {
        counter++;
    }

     /*Now read*/

    if(perPort == GT_TRUE)
    {
        regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Counter_Table_1_Indirect_Read_Data ;
    }
    else
    {
        regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Counter_Table_0_Indirect_Read_Data ;
    }

     tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
     regAddr+=tileOffset;

     counter = 0;

     while((dataValid == GT_FALSE)&&(counter<PRV_MAX_QFC_RETRY_MAC))
     {
        rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

        if(rc!=GT_OK)
        {
            return rc;
        }
        PRV_QFC_FIELD_GET_WITH_TYPE_MAC(devNum,INDIRECT_ACCESS_DATA_VALID,regValue,dataValid,GT_BOOL);

        if(dataValid == GT_TRUE)
        {
            PRV_QFC_FIELD_GET_MAC(devNum,INDIRECT_ACCESS_COUNTER_DATA,regValue,*numPtr);

        }
        else
        {
            counter++;
        }
    }

    if(counter ==PRV_MAX_QFC_RETRY_MAC)
    {
        rc = GT_FAIL;
    }

    return rc;
}

/**
* @internal prvCpssFalconTxqQfcLocalPortToSourcePortSet function
* @endinternal
*
* @brief   Map local DP port to global dma
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                -  device number
* @param[in] tileNum               -  tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                -  QFC unit id (APPLICABLE RANGES:0..7).
* @param[in] localPort             -  local DP port (APPLICABLE RANGES:0..8).
* @param[in] sourcePort            -  Source port(0..2^9-1).
* @param[in] fcMode                -  flow control mode
* @param[in] ignoreMapping         -  Ignore mapping configuration ,configure only PFC generation
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcLocalPortToSourcePortSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      tileNum,
    IN  GT_U32                      qfcNum,
    IN  GT_U32                      localPort,
    IN  GT_U32                      sourcePort,
    IN CPSS_DXCH_PORT_FC_MODE_ENT   fcMode,
    IN GT_BOOL                      ignoreMapping
)
{
        GT_STATUS rc = GT_OK;
        GT_U32 regValue =0;
        GT_U32 localPortFcType;
        GT_U32 regAddr;
        PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

        if(sourcePort>=(GT_U32)(1<<PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_GLOBAL_SOURCE_PORT_FIELD_SIZE)))
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "dmaPort port is too big %d",sourcePort);
        }


        switch(fcMode)
        {
            case CPSS_DXCH_PORT_FC_MODE_802_3X_E:
                localPortFcType = 0x2;
                break;
             case CPSS_DXCH_PORT_FC_MODE_PFC_E:
                localPortFcType = 0x1;
                break;
             case  CPSS_DXCH_PORT_FC_MODE_DISABLE_E:
                localPortFcType = 0x0;
                break;
             default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid FC mode ");
                break;
        }

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].local_port_source_port_config[localPort];

        rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"local_port_source_port_config");
        if (rc != GT_OK)
        {
          return rc;
        }

        log.regIndex = localPort;

        if(ignoreMapping == GT_TRUE)
        {
            /*configure only port type*/
            rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,
                PRV_QFC_REG_FIELD_MAC(devNum,LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_TYPE),
                localPortFcType);
        }
        else
        {

             PRV_QFC_FIELD_SET_MAC(devNum,LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_TYPE,regValue,localPortFcType);
             PRV_QFC_FIELD_SET_MAC(devNum,LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_GLOBAL_SOURCE_PORT,regValue,sourcePort);
             rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);
        }

        return rc;

}

/**
* @internal prvCpssFalconTxqQfcLocalPortToSourcePortGet function
* @endinternal
*
* @brief   Get local DP port to global dma mapping
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                -  device number
* @param[in] tileNum               -  tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                -  QFC unit id (APPLICABLE RANGES:0..7).
* @param[in] localPort             -  local DP port (APPLICABLE RANGES:0..8).
* @param[out] sourcePortPtr        -  (Pointer to)Source port number(physical port).
* @param[out] fcModePtr            - (Pointer to) Enable PFC generation for the local port.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcLocalPortToSourcePortGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      tileNum,
    IN  GT_U32                      qfcNum,
    IN  GT_U32                      localPort,
    OUT  GT_U32                     *sourcePortPtr,
    OUT CPSS_DXCH_PORT_FC_MODE_ENT  *fcModePtr
)
{
    GT_STATUS rc;
    GT_U32 regValue =0,localPortFcType;
    GT_U32 regAddr,tileOffset;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);
    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);


    if(GT_FALSE==PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        /* the unit not exists in GM and it returns 0x00badadd */
        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].local_port_source_port_config[localPort];
        tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
        regAddr+=tileOffset;
        rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    if(fcModePtr!=NULL)
    {
        PRV_QFC_FIELD_GET_MAC(devNum,LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_TYPE,regValue,localPortFcType);

        switch(localPortFcType)
        {
            case 0x2 :
                *fcModePtr = CPSS_DXCH_PORT_FC_MODE_802_3X_E;
                break;
             case 0x1:
               *fcModePtr = CPSS_DXCH_PORT_FC_MODE_PFC_E;
                break;
             case 0x0:
               *fcModePtr = CPSS_DXCH_PORT_FC_MODE_DISABLE_E;
                break;
             default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid FC mode ");
                break;
            }
    }

    if(sourcePortPtr!=NULL)
    {
        PRV_QFC_FIELD_GET_MAC(devNum,LOCAL_PORT_SOURCE_PORT_CONFIG_LOCAL_PORT_GLOBAL_SOURCE_PORT,regValue,*sourcePortPtr);
    }

    return GT_OK;
}

/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgSet function
* @endinternal
*
* @brief   Set global PFC configuration per QFC (pbAvailableBuffers ,pbCongestionPfcEnable,pdxCongestionPfcEnable,globalPfcEnable)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] cfgPtr                - (pointer to)QFC PFC global configuration

* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgSet
(
     IN  GT_U8                                          devNum,
     IN  GT_U32                                         tileNum,
     IN  GT_U32                                         qfcNum,
     IN  PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC  * cfgPtr
)
{
    GT_STATUS rc;
    GT_U32 regValue=0,regAddr;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    if(cfgPtr->pbAvailableBuffers>=(GT_U32)1<<PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_PB_AVAILABLE_BUFFERS_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "pbAvailableBuffers  is too big %d",cfgPtr->pbAvailableBuffers);
    }

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Global_PFC_conf;

    /*read HW values*/
    rc = prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,regAddr,0,32,&regValue);
    if(rc!=GT_OK)
    {
       return rc;
    }

    rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"Global_PFC_conf");
    if (rc != GT_OK)
    {
      return rc;
    }

    PRV_QFC_FIELD_SET_MAC(devNum,GLOBAL_PFC_CONF_GLOBAL_PFC_ENABLE,regValue,cfgPtr->globalPfcEnable);
    PRV_QFC_FIELD_SET_MAC(devNum,GLOBAL_PFC_CONF_PDX_CONGESTION_PFC_ENABLE,regValue,cfgPtr->pdxCongestionPfcEnable);
    PRV_QFC_FIELD_SET_MAC(devNum,GLOBAL_PFC_CONF_PB_CONGESTION_PFC_ENABLE,regValue,cfgPtr->pbCongestionPfcEnable);
    PRV_QFC_FIELD_SET_MAC(devNum,GLOBAL_PFC_CONF_PB_AVAILABLE_BUFFERS,regValue,cfgPtr->pbAvailableBuffers);
    PRV_QFC_FIELD_SET_MAC(devNum,GLOBAL_PFC_CONF_IA_TC_PFC_ENABLE,regValue,cfgPtr->ingressAgregatorTcPfcBitmap);
    PRV_QFC_FIELD_SET_MAC(devNum,GLOBAL_PFC_CONF_HR_COUNTING_ENABLE,regValue,cfgPtr->hrCountingEnable);

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);

    return rc;
}

/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgGet function
* @endinternal
*
* @brief   Get global PFC configuration per QFC (pbAvailableBuffers ,pbCongestionPfcEnable,pdxCongestionPfcEnable,globalPfcEnable)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[out] cfgPtr               - (pointer to)QFC PFC global configuration

* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgGet
(
     IN  GT_U8                                          devNum,
     IN  GT_U32                                         tileNum,
     IN  GT_U32                                         qfcNum,
     IN  PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC  *cfgPtr
)
{
    GT_STATUS rc;
    GT_U32 regValue,regAddr,tileOffset;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Global_PFC_conf;
    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

    if(rc!=GT_OK)
    {
           return rc;
    }

    PRV_QFC_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_GLOBAL_PFC_ENABLE,regValue,cfgPtr->globalPfcEnable);
    PRV_QFC_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_PDX_CONGESTION_PFC_ENABLE,regValue,cfgPtr->pdxCongestionPfcEnable);
    PRV_QFC_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_PB_CONGESTION_PFC_ENABLE,regValue,cfgPtr->pbCongestionPfcEnable);
    PRV_QFC_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_PB_AVAILABLE_BUFFERS,regValue,cfgPtr->pbAvailableBuffers);
    PRV_QFC_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_IA_TC_PFC_ENABLE,regValue,cfgPtr->ingressAgregatorTcPfcBitmap);
    PRV_QFC_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_HR_COUNTING_ENABLE,regValue,cfgPtr->hrCountingEnable);

    return rc;
}

/**
* @internal prvCpssFalconTxqQfcGlobalPbLimitSet function
* @endinternal
*
* @brief   Set global packet buffer limit
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] globalPbLimit         -  packet buffer limit

* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalPbLimitSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN  GT_U32  globalPbLimit
)
{
    GT_STATUS rc;
    GT_U32 regValue = 0,regAddr;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    if(globalPbLimit>=(GT_U32)1<<PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_PB_LIMIT_GLOBAL_PB_LIMIT_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "globalPbLimit  is too big %d",globalPbLimit);
    }

    rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"global_pb_limit");
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].global_pb_limit;

    PRV_QFC_FIELD_SET_MAC(devNum,GLOBAL_PB_LIMIT_GLOBAL_PB_LIMIT,regValue,globalPbLimit);

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);

    return rc;
}

/**
* @internal prvCpssFalconTxqQfcGlobalPbLimitGet function
* @endinternal
*
* @brief   Set global packet buffer limit
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] globalPbLimitPtr      - (pointer to)packet buffer limit

* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalPbLimitGet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     OUT  GT_U32 * globalPbLimitPtr
)
{
    GT_STATUS rc;
    GT_U32 regValue,regAddr,tileOffset;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);
    CPSS_NULL_PTR_CHECK_MAC(globalPbLimitPtr);


    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].global_pb_limit;
    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;


    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

    if(rc == GT_OK)
    {
        PRV_QFC_FIELD_GET_MAC(devNum,GLOBAL_PB_LIMIT_GLOBAL_PB_LIMIT,regValue,*globalPbLimitPtr);
    }

    return rc;
}

/**
* @internal prvCpssFalconTxqQfcGlobalTcPfcThresholdSet function
* @endinternal
*
* @brief   Set global TC threshold .
*    Note - Duplication to all units is done internally in prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - device number
* @param[in] tc               - traffic class(APPLICABLE RANGES:0..7).
* @param[in] enable           - Global TC PFC enable option.
* @param[in] thresholdCfgPtr  - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalTcPfcThresholdSet
(
  IN  GT_U8                             devNum,
  IN  GT_U32                            tc,
  IN  GT_BOOL                           enable,
  IN  CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
  IN  CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{
    GT_U32 regValue[2] = {0,0},numberOfTiles;
    GT_STATUS rc = GT_OK;
    GT_U32 regAddr,i,j;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    CPSS_NULL_PTR_CHECK_MAC(thresholdCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(hysteresisCfgPtr);

    if(enable == GT_TRUE)
    {
      if(thresholdCfgPtr->alfa>=(GT_U32)1<<PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ALPHA_FIELD_SIZE))
      {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "alfa  is too big %d",thresholdCfgPtr->alfa);
      }

      if(thresholdCfgPtr->guaranteedThreshold>=(GT_U32)1<<PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_GUR_THRESHOLD_FIELD_SIZE))
      {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "guaranteedThreshold  is too big %d",thresholdCfgPtr->alfa);
      }

      if(hysteresisCfgPtr->xonOffsetValue>=(GT_U32)1<<PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,TC_PFC_HYSTERESIS_CONF_TC_XON_OFFSET_VALUE_FIELD_SIZE))
      {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "xonOffsetValue  is too big %d",hysteresisCfgPtr->xonOffsetValue);
      }
    }

    PRV_CPSS_NUM_OF_TILE_GET_MAC(devNum,numberOfTiles);

    PRV_QFC_FIELD_SET_MAC(devNum,GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_GUR_THRESHOLD,regValue[0],thresholdCfgPtr->guaranteedThreshold);
    PRV_QFC_FIELD_SET_MAC(devNum,GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ALPHA,regValue[0],thresholdCfgPtr->alfa);
    PRV_QFC_FIELD_SET_MAC(devNum,GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ENABLE,regValue[0],enable?1:0);

    PRV_QFC_FIELD_SET_MAC(devNum,TC_PFC_HYSTERESIS_CONF_TC_XON_OFFSET_VALUE,regValue[1],hysteresisCfgPtr->xonOffsetValue);
    PRV_QFC_FIELD_SET_MAC(devNum,TC_PFC_HYSTERESIS_CONF_TC_XON_MODE,regValue[1],(hysteresisCfgPtr->xonMode==CPSS_DXCH_PORT_PFC_XON_MODE_OFFSET_E)?0:1);

    /*duplicate to all tiles*/
    for(i=0;i<numberOfTiles;i++)
    {
       for(j=0;j<MAX_DP_IN_TILE(devNum);j++)
       {
         rc = prvCpssSip6TxqQfcLogInit(&log,j,"global_tc_pfc_threshold");
         if (rc != GT_OK)
         {
           return rc;
         }

         log.regIndex = tc;

         regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[j].global_tc_pfc_threshold[tc];


        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,i,&log,regAddr,0,32,regValue[0]);
        if(rc!=GT_OK)
        {
            return rc;
        }

        log.regName = "TC_PFC_Hysteresis_Conf";

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[j].TC_PFC_Hysteresis_Conf[tc];


        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,i,&log,regAddr,0,32,regValue[1]);
        if(rc!=GT_OK)
        {
            return rc;
        }
       }
    }
    return rc;
}

/**
* @internal prvCpssFalconTxqQfcGlobalTcPfcThresholdGet function
* @endinternal
*
* @brief   Get global TC threshold .Note - The threshold is read from QFC0(all other DPs should contain the same value)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - device number
* @param[in] tc               - traffic class(APPLICABLE RANGES:0..7).
* @param[in] tileNum          - current tile id(APPLICABLE RANGES:0..3).
* @param[in] dp               - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[out] enablePtr       - (pointer to )Global TC PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalTcPfcThresholdGet
(
     IN  GT_U8                             devNum,
     IN  GT_U32                            tc,
     IN  GT_U32                            tile,
     IN  GT_U32                            dp,
     OUT GT_BOOL                           *enablePtr,
     OUT CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
     OUT CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{
       GT_U32 regValue,xonMode,regAddr,tileOffset;
       GT_STATUS rc;

       CPSS_NULL_PTR_CHECK_MAC(thresholdCfgPtr);
       CPSS_NULL_PTR_CHECK_MAC(hysteresisCfgPtr);

       regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[dp].global_tc_pfc_threshold[tc];
       tileOffset = prvCpssSip6TileOffsetGet(devNum,tile);
       regAddr+=tileOffset;

       rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);
       if(rc == GT_OK)
       {
        PRV_QFC_FIELD_GET_MAC(devNum,GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_GUR_THRESHOLD,regValue,thresholdCfgPtr->guaranteedThreshold);
        PRV_QFC_FIELD_GET_MAC(devNum,GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ALPHA,regValue,thresholdCfgPtr->alfa);
        PRV_QFC_FIELD_GET_WITH_TYPE_MAC(devNum,GLOBAL_TC_PFC_THRESHOLD_GLOBAL_TC_PFC_ENABLE,regValue,*enablePtr,GT_BOOL);
       }

       if(hysteresisCfgPtr==NULL||rc!=GT_OK)
       {
           return rc;
       }


       regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[0].TC_PFC_Hysteresis_Conf[tc];
       tileOffset = prvCpssSip6TileOffsetGet(devNum,0);
       regAddr+=tileOffset;

       rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

       if(rc == GT_OK)
       {
            PRV_QFC_FIELD_GET_MAC(devNum,TC_PFC_HYSTERESIS_CONF_TC_XON_OFFSET_VALUE,regValue,hysteresisCfgPtr->xonOffsetValue);
            PRV_QFC_FIELD_GET_MAC(devNum,TC_PFC_HYSTERESIS_CONF_TC_XON_MODE,regValue,xonMode);

            if(xonMode==0)
            {
                hysteresisCfgPtr->xonMode=CPSS_DXCH_PORT_PFC_XON_MODE_OFFSET_E;
            }
            else
            {
                hysteresisCfgPtr->xonMode=CPSS_DXCH_PORT_PFC_XON_MODE_FIXED_E;
            }
        }
        return rc;
}

/**
* @internal prvCpssFalconTxqQfcPortTcPfcThresholdSet function
* @endinternal
*
* @brief   Set  Port/TC threshold .
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number
* @param[in] tileNum         - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum          - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort       - local port (APPLICABLE RANGES:0..8).
* @param[in] tc              - traffic class(APPLICABLE RANGES:0..7).
* @param[in] enable          - Port/TC PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct                -
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortTcPfcThresholdSet
(
     IN  GT_U8                       devNum,
     IN  GT_U32                      tileNum,
     IN  GT_U32                      qfcNum,
     IN  GT_U32                      localPort,
     IN  GT_U32                      tc,
     IN  GT_BOOL                     enable,
     IN CPSS_DXCH_PFC_THRESHOLD_STC  *thresholdCfgPtr
)
{
        GT_STATUS rc;
        GT_U32    regValue=0,regAddr;
        PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
        GT_U32    maxThresholdValue;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        CPSS_NULL_PTR_CHECK_MAC(thresholdCfgPtr);

        if(thresholdCfgPtr->alfa>=(GT_U32)1<<PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ALPHA_FIELD_SIZE))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "alfa  is too big %d",thresholdCfgPtr->alfa);
        }

        if(thresholdCfgPtr->guaranteedThreshold>=(GT_U32)1<<PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_GUR_THRESHOLD_FIELD_SIZE))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "guaranteedThreshold  is too big %d",thresholdCfgPtr->alfa);
        }



        rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"local_port_source_port_config");
        if (rc != GT_OK)
        {
          return rc;
        }
        log.regIndex = localPort*8+tc;

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].port_tc_pfc_threshold[localPort*8+tc];

        /*Raise the treshold to maximum so MAC will not stuck in XOFF state*/
        if(GT_FALSE==enable)
        {
            maxThresholdValue = ((GT_U32)1<<PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_GUR_THRESHOLD_FIELD_SIZE))-1;
            PRV_QFC_FIELD_SET_MAC(devNum,PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_GUR_THRESHOLD,regValue,maxThresholdValue);
            PRV_QFC_FIELD_SET_MAC(devNum,PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ALPHA,regValue,0);
            PRV_QFC_FIELD_SET_MAC(devNum,PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ENABLE,regValue,1);
            rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);

            if (rc != GT_OK)
            {
              return rc;
            }
        }

        PRV_QFC_FIELD_SET_MAC(devNum,PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_GUR_THRESHOLD,regValue,thresholdCfgPtr->guaranteedThreshold);
        PRV_QFC_FIELD_SET_MAC(devNum,PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ALPHA,regValue,thresholdCfgPtr->alfa);
        PRV_QFC_FIELD_SET_MAC(devNum,PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ENABLE,regValue,enable?1:0);

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);

        return rc;
}

/**
* @internal prvCpssFalconTxqQfcPortTcPfcThresholdGet function
* @endinternal
*
* @brief   Get  Port/TC threshold .
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - device number
* @param[in] tileNum          - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum           - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort        - local port (APPLICABLE RANGES:0..8).
* @param[in] tc               - traffic class(APPLICABLE RANGES:0..7).
* @param[out] enablePtr       - (pointer to )Port TC PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortTcPfcThresholdGet
(
     IN  GT_U8                          devNum,
     IN  GT_U32                         tileNum,
     IN  GT_U32                         qfcNum,
     IN  GT_U32                         localPort,
     IN  GT_U32                         tc,
     IN  GT_BOOL                        *enablePtr,
     OUT CPSS_DXCH_PFC_THRESHOLD_STC    *thresholdCfgPtr
)
{
        GT_STATUS rc;
        GT_U32    regValue,regAddr,tileOffset;

        CPSS_NULL_PTR_CHECK_MAC(enablePtr);
        CPSS_NULL_PTR_CHECK_MAC(thresholdCfgPtr);

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].port_tc_pfc_threshold[localPort*8+tc];
        tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
        regAddr+=tileOffset;

        rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

        if(rc == GT_OK)
        {
            PRV_QFC_FIELD_GET_MAC(devNum,PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_GUR_THRESHOLD,regValue,thresholdCfgPtr->guaranteedThreshold);
            PRV_QFC_FIELD_GET_MAC(devNum,PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ALPHA,regValue,thresholdCfgPtr->alfa);
            PRV_QFC_FIELD_GET_WITH_TYPE_MAC(devNum,PORT_TC_PFC_THRESHOLD_PORT_0_TC_PFC_ENABLE,regValue,*enablePtr,GT_BOOL);
        }

        return rc;
}

/**
* @internal prvCpssFalconTxqQfcPortPfcThresholdSet function
* @endinternal
*
* @brief   Set  Port threshold .
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number
* @param[in] tileNum         - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum          - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort       - local port (APPLICABLE RANGES:0..8).
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortPfcThresholdSet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tileNum,
     IN  GT_U32                             qfcNum,
     IN  GT_U32                             localPort,
     IN  GT_BOOL                            enable,
     IN  CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
     IN  CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{
        GT_STATUS rc;
        GT_U32    regValue=0,regAddr;
        PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
        GT_U32    maxThresholdValue;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        if(thresholdCfgPtr->alfa>=(GT_U32)1<<PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,PORT_PFC_THRESHOLD_PORT_PFC_ALPHA_FIELD_SIZE))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "alfa  is too big %d",thresholdCfgPtr->alfa);
        }

        if(thresholdCfgPtr->guaranteedThreshold>=(GT_U32)1<<PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,PORT_PFC_THRESHOLD_PORT_PFC_GUR_THRESHOLD_FIELD_SIZE))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "guaranteedThreshold  is too big %d",thresholdCfgPtr->alfa);
        }

        if(hysteresisCfgPtr->xonOffsetValue>=(GT_U32)1<<PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,TC_PFC_HYSTERESIS_CONF_TC_XON_OFFSET_VALUE_FIELD_SIZE))
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "xonOffsetValue  is too big %d",hysteresisCfgPtr->xonOffsetValue);
        }

        rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"port_pfc_threshold");
        if (rc != GT_OK)
        {
          return rc;
        }
        log.regIndex = localPort;

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].port_pfc_threshold[localPort];

        /*Raise the treshold to maximum so MAC will not stuck in XOFF state,set  XON to 0*/
        if(GT_FALSE==enable)
        {
            maxThresholdValue = ((GT_U32)1<<PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,PORT_PFC_THRESHOLD_PORT_PFC_GUR_THRESHOLD_FIELD_SIZE))-1;
            PRV_QFC_FIELD_SET_MAC(devNum,PORT_PFC_THRESHOLD_PORT_PFC_GUR_THRESHOLD,regValue,maxThresholdValue);
            PRV_QFC_FIELD_SET_MAC(devNum,PORT_PFC_THRESHOLD_PORT_PFC_ALPHA,regValue,0);
            PRV_QFC_FIELD_SET_MAC(devNum,PORT_PFC_THRESHOLD_PORT_PFC_ENABLE,regValue,1);
            rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);

            if (rc != GT_OK)
            {
              return rc;
            }

            regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Port_PFC_Hysteresis_Conf[localPort];
            log.regName = "Port_PFC_Hysteresis_Conf";

            PRV_QFC_FIELD_SET_MAC(devNum,PORT_PFC_HYSTERESIS_CONF_PORT_XON_OFFSET_VALUE,regValue,1);
            PRV_QFC_FIELD_SET_MAC(devNum,PORT_PFC_HYSTERESIS_CONF_PORT_XON_MODE,regValue,0);

            rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);
            if(rc!=GT_OK)
            {
                return rc;
            }

            rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"port_pfc_threshold");
            if (rc != GT_OK)
            {
              return rc;
            }

            log.regIndex = localPort;

            regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].port_pfc_threshold[localPort];
        }


        PRV_QFC_FIELD_SET_MAC(devNum,PORT_PFC_THRESHOLD_PORT_PFC_GUR_THRESHOLD,regValue,thresholdCfgPtr->guaranteedThreshold);
        PRV_QFC_FIELD_SET_MAC(devNum,PORT_PFC_THRESHOLD_PORT_PFC_ALPHA,regValue,thresholdCfgPtr->alfa);
        PRV_QFC_FIELD_SET_MAC(devNum,PORT_PFC_THRESHOLD_PORT_PFC_ENABLE,regValue,enable?1:0);

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);
        if(rc!=GT_OK)
        {
            return rc;
        }

        regValue =0;

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Port_PFC_Hysteresis_Conf[localPort];
        log.regName = "Port_PFC_Hysteresis_Conf";

        PRV_QFC_FIELD_SET_MAC(devNum,PORT_PFC_HYSTERESIS_CONF_PORT_XON_OFFSET_VALUE,regValue,hysteresisCfgPtr->xonOffsetValue);
        PRV_QFC_FIELD_SET_MAC(devNum,PORT_PFC_HYSTERESIS_CONF_PORT_XON_MODE,regValue,(hysteresisCfgPtr->xonMode==CPSS_DXCH_PORT_PFC_XON_MODE_OFFSET_E)?0:1);

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);
        if(rc!=GT_OK)
        {
            return rc;
        }
        return rc;
}
/**
* @internal prvCpssFalconTxqQfcPortPfcThresholdGet function
* @endinternal
*
* @brief   Get  Port threshold .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - device number
* @param[in] tileNum          - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum           - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort        - local port (APPLICABLE RANGES:0..8).
* @param[out] enablePtr       - (pointer to )Port PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortPfcThresholdGet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tileNum,
     IN  GT_U32                             qfcNum,
     IN  GT_U32                             localPort,
     OUT GT_BOOL                            *enablePtr,
     OUT  CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
     OUT CPSS_DXCH_PFC_HYSTERESIS_CONF_STC  *hysteresisCfgPtr
)
{
        GT_STATUS rc;
        GT_U32 regValue,xonMode;
        GT_U32 regAddr,tileOffset;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].port_pfc_threshold[localPort];
        tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
        regAddr+=tileOffset;

        rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

        if(rc == GT_OK)
        {
            PRV_QFC_FIELD_GET_MAC(devNum,PORT_PFC_THRESHOLD_PORT_PFC_GUR_THRESHOLD,regValue,thresholdCfgPtr->guaranteedThreshold);
            PRV_QFC_FIELD_GET_MAC(devNum,PORT_PFC_THRESHOLD_PORT_PFC_ALPHA,regValue,thresholdCfgPtr->alfa);
            PRV_QFC_FIELD_GET_WITH_TYPE_MAC(devNum,PORT_PFC_THRESHOLD_PORT_PFC_ENABLE,regValue,*enablePtr,GT_BOOL);
        }

        if(hysteresisCfgPtr==NULL||rc!=GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Port_PFC_Hysteresis_Conf[localPort];
        tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
        regAddr+=tileOffset;

        rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

        if(rc == GT_OK)
        {
            PRV_QFC_FIELD_GET_MAC(devNum,PORT_PFC_HYSTERESIS_CONF_PORT_XON_OFFSET_VALUE,regValue,hysteresisCfgPtr->xonOffsetValue);
            PRV_QFC_FIELD_GET_MAC(devNum,PORT_PFC_HYSTERESIS_CONF_PORT_XON_MODE,regValue,xonMode);

            if(xonMode==0)
            {
                hysteresisCfgPtr->xonMode=CPSS_DXCH_PORT_PFC_XON_MODE_OFFSET_E;
            }
            else
            {
                hysteresisCfgPtr->xonMode=CPSS_DXCH_PORT_PFC_XON_MODE_FIXED_E;
            }
        }
        return rc;
}

/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgPbCongestionSet function
* @endinternal
*
* @brief   Set PBB PFC enable per QFC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum              - device number
* @param[in] tileNum             - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum              - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] pbCongestionEnable  - when Enabled, if the PB occupancy is above the configured limit,
*                                  PFC OFF message will be sent to all port.tc
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgPbCongestionSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN GT_BOOL  pbCongestionEnable
)
{
        GT_STATUS rc;
        GT_U32 regAddr;

        PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"Global_PFC_conf");
        if (rc != GT_OK)
        {
          return rc;
        }

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Global_PFC_conf;

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,
        PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_PB_CONGESTION_PFC_ENABLE_FIELD_OFFSET),
        PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_PB_CONGESTION_PFC_ENABLE_FIELD_SIZE),
        pbCongestionEnable?1:0);


        return rc;
}

/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgGlobalEnableSet function
* @endinternal
*
* @brief   Set global PFC enable per QFC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum              - device number
* @param[in] tileNum             - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum              - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] pbCongestionEnable  - when Enabled, if the PB occupancy is above the configured limit,
*                                  PFC OFF message will be sent to all port.tc
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgGlobalEnableSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN GT_BOOL  enable
)
{
        GT_STATUS rc;
        GT_U32 regAddr;

        PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

        TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

        TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

        rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"Global_PFC_conf");
        if (rc != GT_OK)
        {
          return rc;
        }


        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Global_PFC_conf;


       rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,
        PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_GLOBAL_PFC_ENABLE_FIELD_OFFSET),
        PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_GLOBAL_PFC_ENABLE_FIELD_SIZE),
        enable?1:0);

       if(rc!=GT_OK)
       {
         return rc;
       }

       rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,
        PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_IA_TC_PFC_ENABLE_FIELD_OFFSET),
        PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_IA_TC_PFC_ENABLE_FIELD_SIZE),
        enable?0xFF:0x0);

       if(rc!=GT_OK)
       {
         return rc;
       }

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,
            PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_PDX_CONGESTION_PFC_ENABLE_FIELD_OFFSET),
            PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_PFC_CONF_PDX_CONGESTION_PFC_ENABLE_FIELD_SIZE),
            enable?1:0);

       return rc;
}

/**
* @internal prvCpssFalconTxqDumpQfcLocalPfcCounter function
* @endinternal
*
* @brief   Print  the current number of buffers  per source port/tc and Global TC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] sourcePort            - sourcePort number
* @param[in] tc                    - traffic class
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqDumpQfcLocalPfcCounter
(
    IN  GT_U8    devNum,
    IN  GT_U32   tileNum,
    IN  GT_U32   qfcNum,
    IN  GT_U32   sourcePort,
    IN  GT_U32   tc
)
{
    GT_STATUS rc;
    GT_U32 regAddr,regValue,pfcCount = 0;
    GT_U32 counter = 0;
    GT_BOOL dataValid = GT_FALSE;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

    if((sourcePort >= BIT_9) )
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Fail in prvCpssFalconTxqDumpQfcLocalPfcCounter");
    }

     rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"Counter_Table_2_Indirect_Read_Address");
     if (rc != GT_OK)
     {
       return rc;
     }


     regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Counter_Table_2_Indirect_Read_Address ;


     rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,sourcePort*8+tc);
     if(rc!=GT_OK)
     {
        return rc;
     }


     /*Busy wait until the value is latched*/
      while(counter <PRV_MAX_QFC_BUSY_WAIT_CYCLE_MAC)
      {
         counter++;
      }

     /*Now read*/

     regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Counter_Table_2_Indirect_Read_Data ;

     counter = 0;

     while((dataValid == GT_FALSE)&&(counter<PRV_MAX_QFC_RETRY_MAC))
     {
         rc = prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,regAddr,0,32,&regValue);

         if(rc!=GT_OK)
         {
                return rc;
         }

#ifndef ASIC_SIMULATION  /* Yet not implemented in simulation */

        PRV_QFC_FIELD_GET_WITH_TYPE_MAC(devNum,INDIRECT_ACCESS_DATA_VALID,regValue,dataValid,GT_BOOL);

#else
         dataValid = GT_TRUE;
#endif

         if(dataValid == GT_TRUE)
         {
                PRV_QFC_FIELD_GET_MAC(devNum,INDIRECT_ACCESS_COUNTER_DATA,regValue,pfcCount);
         }
         else
         {
                counter++;
         }
      }

     if(counter ==PRV_MAX_QFC_RETRY_MAC)
      {
            rc = GT_FAIL;
      }

     if(rc == GT_OK)
     {
        cpssOsPrintf("\n qfcNum          = %d",qfcNum);
        cpssOsPrintf("\n sourcePort      = %d",sourcePort);
        cpssOsPrintf("\n tc              = %d",tc);
        cpssOsPrintf("\n pfc counter     = %d\n",pfcCount);
     }

     /*Now get global TC*/

     regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Global_counters[tc];

     rc = prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,regAddr,PRV_QFC_REG_FIELD_MAC(devNum,GLOBAL_COUNTERS_TC_COUNTER_VAL),&pfcCount);

     if(rc == GT_OK)
     {
        cpssOsPrintf("\n Global TC count  = %d\n",pfcCount);
     }

    return rc;
}

/**
* @internal prvCpssFalconTxqQfcTcAvaileblePoolBuffersSet
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation for PFC for specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - physical device number
* @param[in] tileNum          - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum           - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] confMode         - Define what availeble buffer value should be updated (Global TC/Port-TC/Both)
* @param[in] tc               - Traffic class [0..7]
* @param[in] availableBuffers - amount of available buffers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcTcAvaileblePoolBuffersSet
(
     IN  GT_U8                                          devNum,
     IN  GT_U32                                         tileNum,
     IN  GT_U32                                         qfcNum,
     IN  CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT  confMode,
     IN  GT_U8                                          tc,
     IN  GT_U32                                         availeblePoolBuffers
)
{
      GT_U32  regValue=0,fieldSize,fieldOffset;
      GT_U32 regAddr;
      PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
      GT_STATUS rc;

      TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

      TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

      rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"Global_TC_Available_buffers_Conf");
      if (rc != GT_OK)
      {
        return rc;
      }
      log.regIndex = tc;

      if(confMode==CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_GLOBAL_TC_E)
      {
        fieldOffset = PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_TC_AVAILABLE_BUFFERS_CONF_GLOBAL_TC_AVAILABLE_BUFFERS_FIELD_OFFSET);
        fieldSize = PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_TC_AVAILABLE_BUFFERS_CONF_GLOBAL_TC_AVAILABLE_BUFFERS_FIELD_SIZE);

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Global_TC_Available_buffers_Conf[tc];
      }
      else
      {
        fieldOffset =PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,PORT_TC_AVAILABLE_BUFFERS_CONF_TC_AVAILABLE_BUFFERS_FIELD_OFFSET);
        fieldSize = PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,PORT_TC_AVAILABLE_BUFFERS_CONF_TC_AVAILABLE_BUFFERS_FIELD_SIZE);
        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Port_TC_Available_buffers_Conf[tc];
        log.regName = "Port_TC_Available_buffers_Conf";
      }

      if(availeblePoolBuffers>=(GT_U32)(1<<fieldSize))
      {
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "availeblePoolBuffers  is too big %d",availeblePoolBuffers);
      }

      U32_SET_FIELD_MASKED_MAC(regValue,fieldOffset,fieldSize,availeblePoolBuffers);

      return  prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);
}


/**
* @internal prvCpssFalconTxqQfcTcAvaileblePoolBuffersGet
* @endinternal
*
* @brief   Get amount of buffers available for dynamic allocation for PFC for specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               - physical device number
* @param[in] tileNum              - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum               - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] confMode             - Define what availeble buffer value should be updated (Global TC/Port-TC/Both)
* @param[in] tc                   - Traffic class [0..7]
* @param[out] availableBuffersPtr - (pointer to)amount of available buffers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcTcAvaileblePoolBuffersGet
(
     IN  GT_U8                                          devNum,
     IN  GT_U32                                         tileNum,
     IN  GT_U32                                         qfcNum,
     IN  CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT  confMode,
     IN  GT_U8                                          tc,
     IN  GT_U32                                         *availeblePoolBuffersPtr
)
{
      GT_U32  fieldSize,fieldOffset;
      GT_U32 regAddr,tileOffset;

      TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

      TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

      if(confMode==CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_GLOBAL_TC_E)
      {
        fieldOffset = PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_TC_AVAILABLE_BUFFERS_CONF_GLOBAL_TC_AVAILABLE_BUFFERS_FIELD_OFFSET);
        fieldSize = PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_TC_AVAILABLE_BUFFERS_CONF_GLOBAL_TC_AVAILABLE_BUFFERS_FIELD_SIZE);
        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Global_TC_Available_buffers_Conf[tc];
      }
      else
      {
        fieldOffset = PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,PORT_TC_AVAILABLE_BUFFERS_CONF_TC_AVAILABLE_BUFFERS_FIELD_OFFSET);
        fieldSize = PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,PORT_TC_AVAILABLE_BUFFERS_CONF_TC_AVAILABLE_BUFFERS_FIELD_SIZE);
        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Port_TC_Available_buffers_Conf[tc];
      }

      tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
      regAddr+=tileOffset;

      return  prvCpssHwPpGetRegField(devNum,regAddr,fieldOffset,fieldSize,availeblePoolBuffersPtr);
}

/**
* @internal prvCpssFalconTxqQfcUburstEnableSet
* @endinternal
*
* @brief   Enable/disable micro burst event generation per queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..399).
* @param[in] queueNum              - local  queue offset (APPLICABLE RANGES:0..7).
* @param[in] enable                - Enable/disable micro burst feature
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstEnableSet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tileNum,
     IN  GT_U32                             qfcNum,
     IN  GT_U32                             queueNum,
     IN  GT_BOOL                            enable
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    if(queueNum>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].uburst_map[queueNum];

    rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"uburst_map");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = queueNum;


    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,
        PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,UBURST_MAP_UBURST_ENABLE_FIELD_OFFSET),
        PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,UBURST_MAP_UBURST_ENABLE_FIELD_SIZE),
        enable?1:0);

   return rc;


}

/**
* @internal prvCpssFalconTxqQfcUburstEnableGet
* @endinternal
*
* @brief   Get enable/disable micro burst event generation per queue
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..399).
* @param[in] queueNum              - local queue offset (APPLICABLE RANGES:0..7).
* @param[in] enable                - (pointer to)Enable/disable micro burst feature
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstEnableGet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tileNum,
     IN  GT_U32                             qfcNum,
     IN  GT_U32                             queueNum,
     IN  GT_BOOL                            *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32    regAddr,tileOffset;
    GT_U32    value;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    if(queueNum>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].uburst_map[queueNum];
    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    rc = prvCpssHwPpGetRegField(devNum,regAddr,
        PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,UBURST_MAP_UBURST_ENABLE_FIELD_OFFSET),
        PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,UBURST_MAP_UBURST_ENABLE_FIELD_SIZE),
        &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal prvCpssFalconTxqQfcUburstProfileBindSet
* @endinternal
*
* @brief  Set queue micro burst profile binding
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] queueNum              - local  queue offset (APPLICABLE RANGES:0..399).
* @param[in] profileNum            - Profile number[0..31]
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstProfileBindSet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tileNum,
     IN  GT_U32                             qfcNum,
     IN  GT_U32                             queueNum,
     IN  GT_U32                             profileNum
)
{
    GT_U32    regAddr;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    if(profileNum>=CPSS_DXCH_SIP_6_MAX_UBURST_PROFILE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(queueNum>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].uburst_map[queueNum];
    rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"uburst_map");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = queueNum;

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,
        PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,UBURST_MAP_UBURST_PROFILE_FIELD_OFFSET),
        PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,UBURST_MAP_UBURST_PROFILE_FIELD_SIZE),
        profileNum);

    return rc;
}
/**
* @internal prvCpssFalconTxqQfcUburstProfileBindGet
* @endinternal
*
* @brief  Get queue micro burst profile binding
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] queueNum              - local  queue offset (APPLICABLE RANGES:0..399).
* @param[in] profileNumPtr         - (pointer to)Profile number[0..31]
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstProfileBindGet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tileNum,
     IN  GT_U32                             qfcNum,
     IN  GT_U32                             queueNum,
     IN  GT_U32                             *profileNumPtr
)
{
    GT_U32    regAddr,tileOffset;
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(profileNumPtr);
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    if(queueNum>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].uburst_map[queueNum];
    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    rc = prvCpssHwPpGetRegField(devNum,regAddr,
            PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,UBURST_MAP_UBURST_PROFILE_FIELD_OFFSET),
            PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,UBURST_MAP_UBURST_PROFILE_FIELD_SIZE),
            profileNumPtr);

    return rc;
}

/**
* @internal prvCpssFalconTxqQfcUburstProfileSet
* @endinternal
*
* @brief  Set  micro burst profile attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] profileNum            - Profile number[0..31]
* @param[in] treshold0Trigger      - Threshold 0 trigger (none/fill/drain/either)
* @param[in] treshold0Value        - Threshold 0 value [0..0xFFFFF]
* @param[in] treshold1Trigger      - Threshold 0 trigger (none/fill/drain/either)
* @param[in] treshold1Value        - Threshold 0 value [0..0xFFFFF]
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstProfileSet
(
     IN  GT_U8                                  devNum,
     IN  GT_U32                                 profileNum,
     IN  CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT treshold0Trigger,
     IN  GT_U32                                 treshold0Value,
     IN  CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT treshold1Trigger,
     IN  GT_U32                                 treshold1Value
)
{
    GT_U32    type0InHwFormat,type1InHwFormat;
    GT_U32    data[2] ={0},numberOfTiles;
    GT_STATUS rc=GT_OK;
    GT_U32 i,j,regAddr;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    if(profileNum>=CPSS_DXCH_SIP_6_MAX_UBURST_PROFILE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_NUM_OF_TILE_GET_MAC(devNum,numberOfTiles);

    TXQ_SIP_6_CHECK_FIELD_LENGTH_VALID_MAC(treshold0Value,
        PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,UBURST_PROFILE_THRESHOLD_PROFILE_THRESHOLD_VALUE_FIELD_SIZE));
    TXQ_SIP_6_CHECK_FIELD_LENGTH_VALID_MAC(treshold1Value,
        PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,UBURST_PROFILE_THRESHOLD_PROFILE_THRESHOLD_VALUE_FIELD_SIZE));

    if(treshold0Value>treshold1Value)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_QFC_UBURST_TRIGGER_SW_2_HW_FORMAT(treshold0Trigger,type0InHwFormat);
    PRV_QFC_UBURST_TRIGGER_SW_2_HW_FORMAT(treshold1Trigger,type1InHwFormat);

    PRV_QFC_FIELD_SET_MAC(devNum,UBURST_PROFILE_THRESHOLD_PROFILE_THRESHOLD_VALUE,data[0],treshold0Value);
    PRV_QFC_FIELD_SET_MAC(devNum,UBURST_PROFILE_THRESHOLD_PROFILE_THRESHOLD_TRIGGER_TYPE,data[0],type0InHwFormat);

    PRV_QFC_FIELD_SET_MAC(devNum,UBURST_PROFILE_THRESHOLD_PROFILE_THRESHOLD_VALUE,data[1],treshold1Value);
    PRV_QFC_FIELD_SET_MAC(devNum,UBURST_PROFILE_THRESHOLD_PROFILE_THRESHOLD_TRIGGER_TYPE,data[1],type1InHwFormat);



    /*duplicate to all tiles*/
    for(i=0;i<numberOfTiles;i++)
    {
       for(j=0;j<MAX_DP_IN_TILE(devNum);j++)
       {
         rc = prvCpssSip6TxqQfcLogInit(&log,j,"uburst_profile_threshold_0");
         if (rc != GT_OK)
         {
           return rc;
         }
         log.regIndex = profileNum;
         regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[j].uburst_profile_threshold_0[profileNum];


         rc = prvCpssSip6TxqRegisterFieldWrite(devNum,i,&log,regAddr,0,32,data[0]);
         if(rc!=GT_OK)
         {
            return rc;
         }

         log.regName = "uburst_profile_threshold_1";

         regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[j].uburst_profile_threshold_1[profileNum];

         rc = prvCpssSip6TxqRegisterFieldWrite(devNum,i,&log,regAddr,0,32,data[1]);
         if(rc!=GT_OK)
         {
            return rc;
         }
       }
    }

    return rc;
}

/**
* @internal prvCpssFalconTxqQfcUburstProfileGet
* @endinternal
*
* @brief  Get  micro burst profile attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] profileNum            - Profile number[0..31]
* @param[out] treshold0TriggerPtr  - (pointer to)Threshold 0 trigger (none/fill/drain/either)
* @param[out] treshold0ValuePtr    - (pointer to)Threshold 0 value [0..0xFFFFF]
* @param[out] treshold1TriggerPtr  - (pointer to)Threshold 0 trigger (none/fill/drain/either)
* @param[out] treshold1ValuePtr    - (pointer to)Threshold 0 value [0..0xFFFFF]
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstProfileGet
(
     IN  GT_U8                                    devNum,
     IN  GT_U32                                   profileNum,
     OUT  CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT  *treshold0TriggerPtr,
     OUT  GT_U32                                  *treshold0ValuePtr,
     OUT  CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT  *treshold1TriggerPtr,
     OUT  GT_U32                                  *treshold1ValuePtr
)
{
    GT_U32    type0InHwFormat,type1InHwFormat;
    GT_U32    data =0;
    GT_STATUS rc;
    GT_U32 regAddr;

    if(profileNum>=CPSS_DXCH_SIP_6_MAX_UBURST_PROFILE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[0].uburst_profile_threshold_0[profileNum];
    rc = prvCpssHwPpReadRegister(devNum,regAddr,&data);
    if(rc!=GT_OK)
    {
      return rc;
    }

    PRV_QFC_FIELD_GET_MAC(devNum,UBURST_PROFILE_THRESHOLD_PROFILE_THRESHOLD_TRIGGER_TYPE,data,type0InHwFormat);
    PRV_QFC_FIELD_GET_MAC(devNum,UBURST_PROFILE_THRESHOLD_PROFILE_THRESHOLD_VALUE,data,*treshold0ValuePtr);

    PRV_QFC_UBURST_TRIGGER_HW_2_SW_FORMAT(*treshold0TriggerPtr,type0InHwFormat);

    data=0;

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[0].uburst_profile_threshold_1[profileNum];
    rc = prvCpssHwPpReadRegister(devNum,regAddr,&data);
    if(rc!=GT_OK)
    {
      return rc;
    }

    PRV_QFC_FIELD_GET_MAC(devNum,UBURST_PROFILE_THRESHOLD_PROFILE_THRESHOLD_TRIGGER_TYPE,data,type1InHwFormat);
    PRV_QFC_FIELD_GET_MAC(devNum,UBURST_PROFILE_THRESHOLD_PROFILE_THRESHOLD_VALUE,data,*treshold1ValuePtr);

    PRV_QFC_UBURST_TRIGGER_HW_2_SW_FORMAT(*treshold1TriggerPtr,type1InHwFormat);

    return rc;
}

/**
* @internal prvCpssFalconTxqQfcUburstEventInfoGet function
* @endinternal
*
* @brief Get micro burst event from specific tile/dp
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                -  physical device number
* @param[in] tileNum               -  current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[out] queueNumPtr          - (pointer to)local  queue offset  that had a micro burst
* @param[out] thresholdIdPtr       - (pointer to)Threshold id (0/1)
* @param[out] timestampPtr         - (pointer to)Time stamp of the event
* @param[out] triggerPtr           - (pointer to)Trigger that caused the event(FILL/DRAIN)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_MORE - no more new events found
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstEventInfoGet
(
    IN   GT_U8                                    devNum,
    IN   GT_U32                                   tileNum,
    IN   GT_U32                                   qfcNum,
    OUT  GT_U32                                   *queueNumPtr,
    OUT  GT_U32                                   *thresholdIdPtr,
    OUT  CPSS_DXCH_UBURST_TIME_STAMP_STC          *timestampPtr,
    OUT  CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT   *triggerPtr
)
{
    GT_U32 regAddr,tileOffset,regValue[2],tmp;
    GT_STATUS rc;
    GT_BOOL     eventValid = GT_TRUE;

    CPSS_NULL_PTR_CHECK_MAC(queueNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(thresholdIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(timestampPtr);
    CPSS_NULL_PTR_CHECK_MAC(triggerPtr);
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Uburst_Event_FIFO;
    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,regValue);
    if(rc!=GT_OK)
    {
           return rc;
    }
    rc = prvCpssHwPpReadRegister(devNum,regAddr+4,regValue+1);
    if(rc!=GT_OK)
    {
           return rc;
    }

    if(GT_TRUE==PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        PRV_QFC_DEV_SPEC_FIELD_GET_WITH_TYPE_MAC(TXQ_HAWK_QFC_UBURST_EVENT_FIFO_0_VALID_BIT_OF_REGISTERS,
            regValue[0],eventValid,GT_BOOL);
    }

    if(GT_TRUE==eventValid)
    {

        PRV_QFC_FIELD_GET_MAC(devNum,UBURST_EVENT_FIFO_0_TIMESTAMP_NANO,regValue[0],timestampPtr->nanoseconds);
        PRV_QFC_FIELD_GET_MAC(devNum,UBURST_EVENT_FIFO_0_TIMESTAMP_SECONDS_BIT_0,regValue[0],tmp);

        PRV_QFC_FIELD_GET_MAC(devNum,UBURST_EVENT_FIFO_1_TIMESTAMP_SECONDS_BITS_15_1,regValue[1],timestampPtr->seconds);
        timestampPtr->seconds = ((timestampPtr->seconds)<<PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,UBURST_EVENT_FIFO_0_TIMESTAMP_SECONDS_BIT_0_FIELD_SIZE))|tmp;
        PRV_QFC_FIELD_GET_MAC(devNum,UBURST_EVENT_FIFO_1_Q_ID,regValue[1],*queueNumPtr);
        PRV_QFC_FIELD_GET_MAC(devNum,UBURST_EVENT_FIFO_1_THRESHOLD_CROSSED,regValue[1],*thresholdIdPtr);
        tmp=0;
        PRV_QFC_FIELD_GET_MAC(devNum,UBURST_EVENT_FIFO_1_TYPE_UBURST_EVENT,regValue[1],tmp);

        PRV_QFC_UBURST_INTERRUPT_TYPE_HW_2_SW_FORMAT(*triggerPtr,tmp);
    }
    else
    {
        *triggerPtr = CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_NOT_VALID_E;/*False alarm*/
    }

    return GT_OK;
}

/**
* @internal prvCpssFalconTxqQfcUburstEventPendingGet function
* @endinternal
*
* @brief Get the number of pending micro burst events from specific tile/dp
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[out] eventPendingNumPtr   - (pointer to)the number of  micro burst events
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_MORE - no more new events found
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstEventPendingGet
(
    IN   GT_U8                                    devNum,
    IN   GT_U32                                   tileNum,
    IN   GT_U32                                   qfcNum,
    IN   GT_BOOL                                  clearOnly,
    OUT  GT_U32                                   *eventPendingNumPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr,tileOffset,regValue,fifoDepth[2],i,iternum;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);
    /*

    This register is latching the maximum value of the uburst event FIFO from the last read. hence 2 consecutive reads will result in the FIFO fill level.
    assuming read value of first read is X0, and read value of second read is X1,CPSS method should read #EVENTS times in order to avoid reading from "empty" registers:
    X1>0    : #EVENTS = MIN{X1+1,64}
    X1 ==0: if X0==1 - #EVENTS =1, else #EVENTS=0
    */
    regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Uburst_Event_FIFO_Max_Peak;
    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    iternum = 1;

    if(clearOnly == GT_FALSE)
    {
        iternum = 2;
    }
    for(i=0;i<iternum;i++)
    {
        rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);
        if(rc!=GT_OK)
        {
               return rc;
        }
       PRV_QFC_FIELD_GET_MAC(devNum,UBURST_EVENT_FIFO_MAX_PEAK_UBURST_EVEN_FIFO_MAX_PEAK_STATUS,regValue,fifoDepth[i]);
    }

    if(clearOnly == GT_TRUE)
    {
        *eventPendingNumPtr = MIN(fifoDepth[0]+1,64);/*For debug,this value is never used in case of clear only*/
        return GT_OK;
    }

    /*Check max peak twice in order to discover fifo depth.
            The value is clear on read.
            If the second time max peak is non zero it mean that there is entries at fifo.
            If the second time max peak is non zero then need to check the first read .
            This is due to the fact that fifo HEAD is not counted in fifo depth ,so if there is only one entry then
            first querry will return 1 and the second 0.
     */

    if(fifoDepth[1]!=0)
    {
      *eventPendingNumPtr = MIN(fifoDepth[1]+1,64);
    }
    else
    {
        if(fifoDepth[0]!=0)
        {
          *eventPendingNumPtr = 1;
        }
        else
        {
          *eventPendingNumPtr = 0;
        }
    }

    return GT_OK;

}
/**
* @internal  prvCpssFalconTxqQfcPfcStatusGet  function
* @endinternal
*
* @brief   Read QFC status from HW
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - PP's device number.
* @param[in] tileNum               - tile number.
* @param[in] qfcNum                - QFC number
*
* @param[out] globalPfcStatusBmpPtr- (Pointer to)Global PFC status
* @param[out] portPfcStatusBmpPtr  - (Pointer to)Port  PFC status
* @param[out] portTcPfcStatusBmpPtr- (Pointer to)Port TC  PFC status
* @param[out] pfcMessageCounterPtr - (Pointer to)PFC message counter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqQfcPfcStatusGet
(
    IN   GT_U8                                    devNum,
    IN   GT_U32                                   tileNum,
    IN   GT_U32                                   qfcNum,
    OUT  GT_U32                                   *globalPfcStatusBmpPtr,
    OUT  GT_U32                                   *portPfcStatusBmpPtr,
    OUT  GT_U32                                   *portTcPfcStatusBmpPtr,
    OUT  GT_U32                                   *pfcMessageCounterPtr
)
{
    GT_U32 regAddr,tileOffset,i;
    GT_U32 numRegisters;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);

     if(globalPfcStatusBmpPtr)
    {
        regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].global_PFC_status;
        regAddr+=tileOffset;
        rc = prvCpssHwPpReadRegister(devNum,regAddr,globalPfcStatusBmpPtr);
        if(rc!=GT_OK)
        {
          return rc;
        }
    }

    if(pfcMessageCounterPtr)
    {
        regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].PFC_Messages_counter;
        regAddr+=tileOffset;

        rc = prvCpssHwPpReadRegister(devNum,regAddr,pfcMessageCounterPtr);
        if(rc!=GT_OK)
        {
          return rc;
        }
    }

    if(portTcPfcStatusBmpPtr||portPfcStatusBmpPtr)
    {
        numRegisters = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ?
            CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum):  /* register per port */
            (CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum)/PRV_QFC_GROUP_SIZE_CNS); /* register for 3 ports */

        for(i=0;i<numRegisters;i++)
        {
            if(portTcPfcStatusBmpPtr)
            {
                regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Group_Port_TC_PFC_Status[i];
                regAddr+=tileOffset;
                rc = prvCpssHwPpReadRegister(devNum,regAddr,portTcPfcStatusBmpPtr+i);
                if(rc!=GT_OK)
                {
                  return rc;
                }
            }

            if(portPfcStatusBmpPtr)
            {
                regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Group_Ports_PFC_Status[i];
                regAddr+=tileOffset;

                rc = prvCpssHwPpReadRegister(devNum,regAddr,portPfcStatusBmpPtr+i);
                if(rc!=GT_OK)
                {
                  return rc;
                }
             }
        }
     }

    return GT_OK;
}

/**
* @internal  prvCpssFalconTxqQfcPfcGlobalStatusParse  function
* @endinternal
*
* @brief   Parse global Pfc Status Bitmap
*
* @note  APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - PP's device number.
* @param[in] globalPfcStatusBmpPtr     - (Pointer to)Global PFC status
* @param[out] pbLimitCrossedPtr        - (Pointer to)PB limit status
* @param[out] pdxLimitCrossedPtr       - (Pointer to) PDX  limit status
* @param[out] iaLimitCrossedPtr        - (Pointer to)IA limit status
* @param[out] globalLimitCrossedBmpPtr - (Pointer to)Global  TC limit status bitmap

* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqQfcPfcGlobalStatusParse
(
    IN   GT_U8                                    devNum,
    IN   GT_U32                                   globalPfcStatusBmp,
    OUT  GT_U32                                   *pbLimitCrossedPtr,
    OUT  GT_U32                                   *pdxLimitCrossedPtr,
    OUT  GT_U32                                   *iaLimitCrossedPtr,
    OUT  GT_U32                                   *globalLimitCrossedBmpPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(pbLimitCrossedPtr);
    CPSS_NULL_PTR_CHECK_MAC(pdxLimitCrossedPtr);
    CPSS_NULL_PTR_CHECK_MAC(iaLimitCrossedPtr);
    CPSS_NULL_PTR_CHECK_MAC(globalLimitCrossedBmpPtr);

    PRV_QFC_FIELD_GET_MAC(devNum,GLOBAL_PFC_STATUS_PB_CROSSED_LIMIT_STATUS,globalPfcStatusBmp,*pbLimitCrossedPtr);
    PRV_QFC_FIELD_GET_MAC(devNum,GLOBAL_PFC_STATUS_PDX_CONGESTION_STATUS,globalPfcStatusBmp,*pdxLimitCrossedPtr);
    PRV_QFC_FIELD_GET_MAC(devNum,GLOBAL_PFC_STATUS_IA_TC_PFC_STATUS,globalPfcStatusBmp,*iaLimitCrossedPtr);
    PRV_QFC_FIELD_GET_MAC(devNum,GLOBAL_PFC_STATUS_GLOBAL_TC_PFC_STATUS,globalPfcStatusBmp,*globalLimitCrossedBmpPtr);

    return GT_OK;
}

/**
* @internal  prvCpssFalconTxqQfcPfcPortStatusParse  function
* @endinternal
*
* @brief  Parse  Pfc  Port Status Bitmap
*
* @note  APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - PP's device number.
* @param[in] portPfcStatusBmpPtr   - (Pointer to)Port PFC status
* @param[in] perTc                 - if equal GT_TRUE then parcing port/tc status,else parsing port status
* @param[out] portLimitCrossedPtr  - (Pointer to) port status

* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqQfcPfcPortStatusParse
(
    IN  GT_U8                                     devNum,
    IN   GT_U32                                   *portPfcStatusBmpPtr,
    IN   GT_BOOL                                   perTc,
    OUT  GT_U32                                   *portLimitCrossedPtr
)
{
    GT_U32 i,j;

    CPSS_NULL_PTR_CHECK_MAC(portPfcStatusBmpPtr);
    CPSS_NULL_PTR_CHECK_MAC(portLimitCrossedPtr);

    for(i=0,j=0;i<CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum);i++)
    {
        if(perTc == GT_FALSE)
        {
            portLimitCrossedPtr[i]= (*portPfcStatusBmpPtr)&0x1;
            (*portPfcStatusBmpPtr)>>=1;
        }
        else
        {

          portLimitCrossedPtr[i]= (*portPfcStatusBmpPtr)&0xFF;
          if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
          {
              (*portPfcStatusBmpPtr)>>=8;
          }
        }

        if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            if(j ==TXQ_QFC_GROUP_PORTS_STATUS_GROUP_PORT_PFC_STATUS_FIELD_SIZE-1)
            {
                portPfcStatusBmpPtr++;
                j=0;
            }
            else
            {
                j++;
            }
        }
        else
        {
            portPfcStatusBmpPtr++;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssFalconTxqQfcHeadroomCounterGet function
* @endinternal
*
* @brief  Gets the current number of headroom buffers allocated on specified local port/DP/TC
*         for specified Traffic Class queues.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum            - physical device number
* @param[in] tileNum           - tile number.
* @param[in] qfcNum            - QFC number
* @param[in]localPort          - local port (APPLICABLE RANGES:0..8).

* @param[in] tc                - trafiic class (0..7)
* @param[out] hrCountValPtr    - (pointer to)number of buffers
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqQfcHeadroomCounterGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      tileNum,
    IN  GT_U32      qfcNum,
    IN  GT_U32      localPort,
    IN  GT_U32      tc,
    OUT  GT_U32     *hrCountValPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr,regValue;
    GT_U32 counter = 0;
    GT_BOOL dataValid = GT_FALSE;
    GT_U32  data = 0;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;


    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

    rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"HR_Counters_Indirect_Read_Access");
    if (rc != GT_OK)
    {
      return rc;
    }

    regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].HR_Counters_Indirect_Read_Access ;

    PRV_QFC_FIELD_SET_MAC(devNum,HR_COUNTERS_INDIRECT_READ_ACCESS_HR_COUNTER_INDIRECT_ACCESS_LOCAL_PORT,data,localPort);
    PRV_QFC_FIELD_SET_MAC(devNum,HR_COUNTERS_INDIRECT_READ_ACCESS_HR_COUNTER_INDIRECT_ACCESS_TC,data,tc);
    PRV_QFC_FIELD_SET_MAC(devNum,HR_COUNTERS_INDIRECT_READ_ACCESS_HR_COUNTER_INDIRECT_ACCESS_VAL_VALID,data,0);

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,data);
    if(rc!=GT_OK)
    {
        return rc;
    }
    /*Busy wait until the value is latched*/
    while(counter <PRV_MAX_QFC_BUSY_WAIT_CYCLE_MAC)
    {
        counter++;
    }

     /*Now read*/

     counter = 0;

     while((dataValid == GT_FALSE)&&(counter<PRV_MAX_QFC_RETRY_MAC))
     {
         rc = prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,regAddr,0,32,&regValue);

         if(rc!=GT_OK)
         {
                return rc;
         }
#ifdef ASIC_SIMULATION
    dataValid = GT_TRUE;/* Yet not implemented in simulation */
#else
     PRV_QFC_FIELD_GET_MAC(devNum,HR_COUNTERS_INDIRECT_READ_ACCESS_HR_COUNTER_INDIRECT_ACCESS_VAL_VALID,regValue,dataValid);
#endif


         if(dataValid == GT_TRUE)
         {
#ifdef GM_USED
            *hrCountValPtr =0;/* Yet not implemented in golden model */
#else
            PRV_QFC_FIELD_GET_MAC(devNum,HR_COUNTERS_INDIRECT_READ_ACCESS_HR_COUNTER_INDIRECT_ACCESS_VAL,regValue,*hrCountValPtr);
#endif
         }
         else
         {
                counter++;
         }
      }

     if(counter ==PRV_MAX_QFC_RETRY_MAC)
      {
            rc = GT_FAIL;
      }

    return rc;
}

/**
* @internal prvCpssFalconTxqQfcHeadroomMonitorSet
* @endinternal
*
* @brief  Set tile/dp/local port /tc for monitoring headroom peak.
*  Results are captured at prvCpssFalconTxqQfcMaxMinHeadroomCounterGet
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - tile number.
* @param[in] qfcNum                - QFC number
* @param[in]localPort              - local port (APPLICABLE RANGES:0..8).
* @param[in] tc                    - Traffic class[0..7]
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcHeadroomMonitorSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      tileNum,
    IN  GT_U32      qfcNum,
    IN  GT_U32      localPort,
    IN  GT_U32      tc
)
{
    GT_U32    regAddr;
    GT_U32    data = 0;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

    rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"Port_TC_HR_Counter_Min_Max_Conf");
    if (rc != GT_OK)
    {
      return rc;
    }

    regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Port_TC_HR_Counter_Min_Max_Conf ;

    PRV_QFC_FIELD_SET_MAC(devNum,PORT_TC_HR_COUNTER_MIN_MAX_CONF_MIN_MAX_HR_COUNTER_LATCHING_TC,data,tc);
    PRV_QFC_FIELD_SET_MAC(devNum,PORT_TC_HR_COUNTER_MIN_MAX_CONF_MIN_MAX_HR_COUNTER_LATCHING_PORT,data,localPort);

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,data);
    if(rc!=GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssFalconTxqQfcMaxMinHeadroomCounterGet
* @endinternal
*
* @brief  Get maximal and minimal headroom  headroom peak size for  port/TC set in
*   cpssDxChPortPfcPortTcHeadroomPeakMonitorSet.
*  Note : Peak values are captured since last read.
*
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - tile number.
* @param[in] qfcNum                - QFC number
* @param[out] minPeakValPtr        - (pointer to)Minimal Headroom size
* @param[out] maxPeakValPtr        - (pointer to)Maximal Headroom size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcMaxMinHeadroomCounterGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      tileNum,
    IN  GT_U32      qfcNum,
    OUT  GT_U32     *minCountValPtr,
    OUT  GT_U32     *maxCountValPtr
)
{
    GT_U32    regAddr,tileOffset;
    GT_U32    data = 0;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].HR_Counter_Max_Peak;

    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&data);
    if(rc!=GT_OK)
    {
        return rc;
    }
    if(maxCountValPtr)
    {
        PRV_QFC_FIELD_GET_MAC(devNum,HR_COUNTER_MAX_PEAK_HR_COUNTER_MAX_VALUE,data,*maxCountValPtr);
    }

    regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].HR_Counter_Min_Peak;

    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&data);
    if(rc!=GT_OK)
    {
        return rc;
    }

    if(minCountValPtr)
    {
        PRV_QFC_FIELD_GET_MAC(devNum,HR_COUNTER_MIN_PEAK_HR_COUNTER_MIN_VALUE,data,*minCountValPtr);
    }

    return GT_OK;

}

/**
* @internal prvCpssFalconTxqQfcHeadroomThresholdSet
* @endinternal
*
* @brief  Set threshold for headroom size interrupt.Once one  or more of the Port. TC has
*   crossed the configured Threshold, an interrupt is asserted and the Status of the Port. TC is written
*   to the HR crossed Threshold Status.
*
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - tile number.
* @param[in] qfcNum                - QFC number
* @param[in]localPort              - local port (APPLICABLE RANGES:0..8).
* @param[in] threshold             - Headroom threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcHeadroomThresholdSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      tileNum,
    IN  GT_U32      qfcNum,
    IN  GT_U32      localPort,
    IN  GT_U32      threshold
)
{
    GT_U32    regAddr;
    GT_U32    data = 0;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    TXQ_SIP_6_CHECK_FIELD_LENGTH_VALID_MAC(threshold,PRV_QFC_DEV_DEP_FIELD_GET_MAC(devNum,PORT_HR_COUNTERS_THRESHOLD_PORT_HR_COUNTER_THRESHOLD_FIELD_SIZE));

    regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Port_HR_Counters_Threshold+4*localPort;


    PRV_QFC_FIELD_SET_MAC(devNum,PORT_HR_COUNTERS_THRESHOLD_PORT_HR_COUNTER_THRESHOLD,data,threshold);

    rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"Port_HR_Counters_Threshold");
    if (rc != GT_OK)
    {
       return rc;
    }
    log.regIndex = localPort;

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,data);
    if(rc!=GT_OK)
    {
        return rc;
    }

    return GT_OK;

}

/**
* @internal prvCpssFalconTxqQfcHeadroomThresholdGet
* @endinternal
*
* @brief  Get threshold for headroom size interrupt.Once one  or more of the Port. TC has
*   crossed the configured Threshold, an interrupt is asserted and the Status of the Port. TC is written
*   to the HR crossed Threshold Status.
*
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - tile number.
* @param[in] qfcNum                - QFC number
* @param[in]localPort              - local port (APPLICABLE RANGES:0..8).

* @param[out] thresholdPtr         - (pointer to)Headroom threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcHeadroomThresholdGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      tileNum,
    IN  GT_U32      qfcNum,
    IN  GT_U32      localPort,
    OUT GT_U32      *thresholdPtr
)
{
    GT_U32    regAddr,tileOffset;
    GT_U32    data = 0;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].Port_HR_Counters_Threshold+4*localPort;

    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&data);
    if(rc!=GT_OK)
    {
        return rc;
    }

    PRV_QFC_FIELD_GET_MAC(devNum,PORT_HR_COUNTERS_THRESHOLD_PORT_HR_COUNTER_THRESHOLD,data,*thresholdPtr);

    return GT_OK;

}

/**
* @internal prvCpssFalconTxqQfcHeadroomInterruptStatusGet
* @endinternal
*
* @brief  Read HR interrupt cause register from specific tile/qfc
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - tile number.
* @param[in] qfcNum                - QFC number
* @param[out]numOfEventsPtr        - (pointer to)number of headroom threshold crossed events.
* @param[out] eventsArr            - Array of headroom threshold crossed events.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcHeadroomInterruptStatusGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U32                               qfcNum,
    OUT GT_U32                               *numOfEventsPtr,
    OUT PRV_CPSS_DXCH_HR_EVENT_INFO_STC      *eventsArr
)
{
    GT_STATUS rc;
    GT_U32    regAddr,tileOffset,i,j,k,data[PRV_QFC_MAX_GROUP_NUM_CNS];
    GT_U32    portCause[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];


    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    cpssOsMemSet(data,0,sizeof(GT_U32)*PRV_QFC_MAX_GROUP_NUM_CNS);

    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);

    for(i=0;i<(CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum)/PRV_QFC_GROUP_SIZE_CNS);i++)
    {
        regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].HR_Crossed_Threshold_Reg_Cause[i];
        regAddr+=tileOffset;
        rc = prvCpssHwPpReadRegister(devNum,regAddr,data+i);
        if(rc!=GT_OK)
        {
          return rc;
        }
    }

    k=0;

    cpssOsMemSet(portCause,0,sizeof(GT_U32)*CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum));

    for(i=0;i<(CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum)/PRV_QFC_GROUP_SIZE_CNS);i++)
    {
        if(data[i]&0x1)/*First bit is interrupt summary*/
        {
            data[i]>>=1;

            for(j=0;j<PRV_FALCON_QFC_HR_PORT_PER_REGISTER_NUM_CNS(devNum);j++)
            {
                portCause[k++] = data[i]&0xFF;
                data[i]>>=8;
            }
        }
        else
        {
            k+=PRV_FALCON_QFC_HR_PORT_PER_REGISTER_NUM_CNS(devNum);
        }
    }

    *numOfEventsPtr=0;

    for(i=0;i<CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum);i++)
    {
        if(portCause[i]!=0)
        {
            for(j=0;j<CPSS_TC_RANGE_CNS;j++)
            {
                if(portCause[i]&0x1)
                {
                    eventsArr[*numOfEventsPtr].localPortNum = i;
                    eventsArr[*numOfEventsPtr].trafficClass = j;
                    (*numOfEventsPtr)++;
                }
                portCause[i]>>=1;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssFalconTxqQfcHeadroomInterruptEnableSet
* @endinternal
*
* @brief Enable HR threshold crossed interrupt on tile/qfc
*
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - tile number.
* @param[in] qfcNum                - QFC number
* @param[in] enable                - mask/unmask interrupts
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcHeadroomInterruptEnableSet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U32                               qfcNum,
    IN  GT_BOOL                              enable
)
{
    GT_STATUS rc;
    GT_U32    regAddr,i;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);


    rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"HR_Crossed_Threshold_Reg_Mask");
    if (rc != GT_OK)
    {
      return rc;
    }


    for(i=0;i<(CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum)/PRV_QFC_GROUP_SIZE_CNS);i++)
    {
        regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].HR_Crossed_Threshold_Reg_Mask[i];
        log.regIndex = i;

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,enable?0x1FFFFFE:0x0);
        if(rc!=GT_OK)
        {
          return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssFalconTxqQfcHeadroomIntRegisterGet
* @endinternal
*
* @brief  Read HR interrupt cause register from specific tile/qfc/register
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - tile number.
* @param[in] qfcNum                - QFC number
* @param[in] registerInd           - index of register.each register hold status for 3 ports.[0..2]
* @param[out]numOfEventsPtr        - (pointer to)number of headroom threshold crossed events.
* @param[out] eventsArr            - Array of headroom threshold crossed events.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcHeadroomIntRegisterGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U32                               qfcNum,
    IN  GT_U32                               registerInd,
    OUT GT_U32                               *numOfEventsPtr,
    OUT PRV_CPSS_DXCH_HR_EVENT_INFO_STC      *eventsArr
)
{
    GT_STATUS rc;
    GT_U32    regAddr,i,j,data;
    GT_U32    portCause[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    if(registerInd>=(CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum)/PRV_QFC_GROUP_SIZE_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "registerInd[%d] must be less than [%d]",registerInd,
            (CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum)/PRV_QFC_GROUP_SIZE_CNS));
    }

    /*Disable interrupt*/

    rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"HR_Crossed_Threshold_Reg_Mask");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = registerInd;

    regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].HR_Crossed_Threshold_Reg_Mask[registerInd];

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,0x0);
    if(rc!=GT_OK)
    {
      return rc;
    }

    regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].HR_Crossed_Threshold_Reg_Cause[registerInd];

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,regAddr,0,32,&data);
    if(rc!=GT_OK)
    {
      return rc;
    }


    /*Enable  interrupt*/

    regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].HR_Crossed_Threshold_Reg_Mask[registerInd];

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,0x1FFFFFE);
    if(rc!=GT_OK)
    {
      return rc;
    }

    cpssOsMemSet(portCause,0,sizeof(GT_U32)*PRV_FALCON_QFC_HR_PORT_PER_REGISTER_NUM_CNS(devNum));

    /*First bit is interrupt summary*/
    data>>=1;

    for(j=0;j<PRV_FALCON_QFC_HR_PORT_PER_REGISTER_NUM_CNS(devNum);j++)
    {
        portCause[j] = data&0xFF;
        data>>=8;
    }


    *numOfEventsPtr=0;

    for(i=0;i<PRV_FALCON_QFC_HR_PORT_PER_REGISTER_NUM_CNS(devNum);i++)
    {
        if(portCause[i]!=0)
        {
            for(j=0;j<CPSS_TC_RANGE_CNS;j++)
            {
                if(portCause[i]&0x1)
                {
                    eventsArr[*numOfEventsPtr].localPortNum = i+registerInd*PRV_FALCON_QFC_HR_PORT_PER_REGISTER_NUM_CNS(devNum);
                    eventsArr[*numOfEventsPtr].trafficClass = j;
                    (*numOfEventsPtr)++;
                }
                portCause[i]>>=1;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssFalconTxqQfcValidateDpSyncronization
* @endinternal
*
* @brief  This function check that all data paths configured similar in regarding
*   to global TC threshold.
*   The function compare configurations to Tile 0 dp 0 and expect all tiles and dps to
*   hold same values.
*
*
* @note   APPLICABLE DEVICES:       Falcon; Hawk;AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[out] tileOutOfSyncPtr     - (pointer to)tile number that is not configred correctly.
* @param[out] dpOutOfSync          - (pointer to)QFC number that is not configred correctly.
* @param[out] isSyncedPtr          - (pointer to)equal GT_TRUE if all tiles are synced ,GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcValidateDpSyncronization
(
    IN  GT_U8                                devNum,
    OUT GT_U32                               *tileOutOfSyncPtr,
    OUT GT_U32                               *dpOutOfSync,
    OUT GT_BOOL                              *isSyncedPtr
)
{
    GT_U32 numberOfTiles,i,j,k;
    GT_STATUS   rc;
    CPSS_DXCH_PFC_THRESHOLD_STC       thresholdCfg[2];
    CPSS_DXCH_PFC_HYSTERESIS_CONF_STC hysteresisCfg[2];
    GT_BOOL                           enable[2];

    CPSS_NULL_PTR_CHECK_MAC(tileOutOfSyncPtr);
    CPSS_NULL_PTR_CHECK_MAC(dpOutOfSync);
    CPSS_NULL_PTR_CHECK_MAC(isSyncedPtr);

    PRV_CPSS_NUM_OF_TILE_GET_MAC(devNum,numberOfTiles);

    *isSyncedPtr = GT_TRUE;
    /*Global PFC treshold*/
    for(k=0;k<CPSS_TC_RANGE_CNS;k++)
    {
      for(i=0;i<numberOfTiles;i++)
      {
         for(j=0;j<MAX_DP_IN_TILE(devNum);j++)
         {
           rc =prvCpssFalconTxqQfcGlobalTcPfcThresholdGet(devNum,k,i,j,enable,thresholdCfg,hysteresisCfg);
           if(rc!=GT_OK)
           {
             return rc;
           }
           /*remember the first one*/
           if(i==0&&j==0)
           {
             enable[1]=enable[0];
             cpssOsMemCpy(thresholdCfg+1,thresholdCfg,sizeof(CPSS_DXCH_PFC_THRESHOLD_STC));
             cpssOsMemCpy(hysteresisCfg+1,hysteresisCfg,sizeof(CPSS_DXCH_PFC_HYSTERESIS_CONF_STC));
           }
           else
           {
                if((enable[1]!=enable[0])||
                (cpssOsMemCmp(thresholdCfg+1,thresholdCfg,sizeof(CPSS_DXCH_PFC_THRESHOLD_STC)))||
                (cpssOsMemCmp(hysteresisCfg+1,hysteresisCfg,sizeof(CPSS_DXCH_PFC_HYSTERESIS_CONF_STC))))
                {
                    *tileOutOfSyncPtr=i;
                    *dpOutOfSync=j;
                    *isSyncedPtr=GT_FALSE;
                    return GT_OK;
                }

           }
         }
       }
     }

     return GT_OK;
}


/**
* @internal prvCpssSip6TxqQfcDebugInterruptDisableSet function
* @endinternal
*
* @brief   Debug function that disable iterrupt in QFC
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS prvCpssSip6TxqQfcDebugInterruptDisableSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 qfcNum
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);


    rc = prvCpssSip6TxqQfcLogInit(&log,qfcNum,"QFC_Interrupt_Summary_Mask");
    if (rc != GT_OK)
    {
        return rc;
    }


    return prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].QFC_Interrupt_Summary_Mask,
                 0,32,0);
}

/**
* @internal prvCpssSip6TxqQfcDebugInterruptGet function
* @endinternal
*
* @brief   Get interrupt cause for QFC
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
GT_STATUS prvCpssSip6TxqQfcDebugInterruptGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 qfcNum,
    OUT GT_U32 *functionalPtr,
    OUT GT_U32 *debugPtr
)
{
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(functionalPtr);
    CPSS_NULL_PTR_CHECK_MAC(debugPtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(qfcNum);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].QFC_Interrupt_functional_Cause,
                 0,32,functionalPtr);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcNum].qfc_interrupt_debug_cause,
                 0,32,debugPtr);

    return rc;
}



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
* @file cpssPxCfgInit.c
*
* @brief CPSS PX general config APIs.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/log/prvCpssPxGenDbLog.h>
#include <cpss/px/config/private/prvCpssPxConfigLog.h>
#include <cpss/px/config/cpssPxCfgInit.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpcDevCfg.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define DEVICE_NOT_EXISTS_CNS 0xFFFFFFFF

/* Object that holds calbback function to table HW access */
extern  CPSS_PX_CFG_HW_ACCESS_OBJ_STC prvPxTableCpssHwAccessObj;

/**
* @internal internal_cpssPxCfgDevRemove function
* @endinternal
*
* @brief   Remove the device from the CPSS.
*         This function we release the DB of CPSS that associated with the PP ,
*         and will "know" that the device is not longer valid.
*         This function will not do any HW actions , and therefore can be called
*         before or after the HW was actually removed.
*         After this function is called the devNum is no longer valid for any API
*         until this devNum is used in the initialization sequence for a "new" PP.
*         NOTE: 1. Application may want to disable the Traffic to the CPU prior to
*         this operation (if the device still exists).
*         2. Application may want to a reset the HW PP , and there for need
*         to call the "hw reset" function (before calling this function)
*         and after calling this function to do the initialization
*         sequence all over again for the device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to remove.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is "ISR safe".The interrupt are disabled on API entry and enabled on API exit
*
*/
static GT_STATUS internal_cpssPxCfgDevRemove
(
    IN GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;

    /* check that the device still exists */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    /* free allocation relate to register addresses DB */
    rc = prvCpssPxHwRegAddrVer1Remove(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* free allocations are 'PX' specific allocations (not generic) */
    /* need to free :
       1. pools
       2. dynamic allocations
       3. semaphores/mutexes
    */
    for(ii = 0;ii < devPtr->numOfPorts ;ii++)
    {
        FREE_PTR_MAC(PRV_CPSS_PX_PP_MAC(devNum)->port.portsMibShadowArr[ii]);
    }

    prvCpssGenericSrvCpuRemove(CAST_SW_DEVNUM(devNum), SRVCPU_IPC_CHANNEL_ALL);
    /* next part -- release the 'generic' DB */
    rc = prvCpssPpConfigDevDbRelease(CAST_SW_DEVNUM(devNum));
    if(rc != GT_OK)
    {
        return rc;
    }

    /*************************************************************/
    /* NOTE :  PRV_CPSS_PP_MAC(devNum) == NULL at this point !!! */
    /*************************************************************/

    /* reset the HWS */
    mvHwsDeviceClose(devNum);
    /*prvCpssGenericSrvCpuRemove(devNum, SRVCPU_IPC_CHANNEL_ALL);*/

    return GT_OK;

}

/**
* @internal cpssPxCfgDevRemove function
* @endinternal
*
* @brief   Remove the device from the CPSS.
*         This function we release the DB of CPSS that associated with the PP ,
*         and will "know" that the device is not longer valid.
*         This function will not do any HW actions , and therefore can be called
*         before or after the HW was actually removed.
*         After this function is called the devNum is no longer valid for any API
*         until this devNum is used in the initialization sequence for a "new" PP.
*         NOTE: 1. Application may want to disable the Traffic to the CPU prior to
*         this operation (if the device still exists).
*         2. Application may want to a reset the HW PP , and there for need
*         to call the "hw reset" function (before calling this function)
*         and after calling this function to do the initialization
*         sequence all over again for the device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to remove.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is "ISR safe".The interrupt are disabled on API entry and enabled on API exit
*
*/
GT_STATUS cpssPxCfgDevRemove
(
    IN GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCfgDevRemove);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    PRV_CPSS_INT_SCAN_LOCK();
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssPxCfgDevRemove(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    PRV_CPSS_INT_SCAN_UNLOCK();
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCfgDevEnable function
* @endinternal
*
* @brief   This routine sets the device state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to set.
* @param[in] enable                   - GT_TRUE device enable, GT_FALSE disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCfgDevEnable
(
    IN GT_SW_DEV_NUM   devNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;       /* return code */
    GT_U32  bit;        /* The bit to be written to the register    */
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    bit = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        CPSS_TBD_BOOKMARK_EARCH/* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    regAddr =
        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                          DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl;

    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
         CAST_SW_DEVNUM(devNum), regAddr, 0, 1, bit);

    return rc;

}
/**
* @internal cpssPxCfgDevEnable function
* @endinternal
*
* @brief   This routine sets the device state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to set.
* @param[in] enable                   - GT_TRUE device enable, GT_FALSE disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCfgDevEnable
(
    IN GT_SW_DEV_NUM   devNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCfgDevEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxCfgDevEnable(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCfgDevEnableGet function
* @endinternal
*
* @brief   This routine gets the device state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to set.
*
* @param[out] enablePtr                - GT_TRUE device enable, GT_FALSE disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static GT_STATUS internal_cpssPxCfgDevEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    GT_U32    bit;
    GT_U32    regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        CPSS_TBD_BOOKMARK_EARCH/* current GM doesn't support DFX unit so far */
        *enablePtr = GT_TRUE;
        return GT_OK;
    }

    regAddr =
        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                          DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl;

    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
         CAST_SW_DEVNUM(devNum), regAddr, 0, 1, &bit);
    if(GT_OK != rc)
    {
        return rc;
    }

    *enablePtr =  BIT2BOOL_MAC(bit);

    return rc;
}

/**
* @internal cpssPxCfgDevEnableGet function
* @endinternal
*
* @brief   This routine gets the device state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to set.
*
* @param[out] enablePtr                - GT_TRUE device enable, GT_FALSE disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssPxCfgDevEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCfgDevEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxCfgDevEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCfgNextDevGet function
* @endinternal
*
* @brief   Return the number of the next existing device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to start from. For the first one  should be 0xFF.
*
* @param[out] nextDevNumPtr            - number of next device after devNum.
*
* @retval GT_OK                    - on success.
* @retval GT_NO_MORE               - devNum is the last device. nextDevNumPtr will be set to 0xFF.
* @retval GT_BAD_PARAM             - devNum > max device number
* @retval GT_BAD_PTR               - nextDevNumPtr pointer is NULL.
*/
static GT_STATUS internal_cpssPxCfgNextDevGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT GT_SW_DEV_NUM *nextDevNumPtr
)
{
    GT_SW_DEV_NUM num;

    /* device should be in the allowed range but not necessary exist */
    if ((devNum > PRV_CPSS_MAX_PP_DEVICES_CNS) && (devNum != DEVICE_NOT_EXISTS_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "devNum[%d] not exists",(devNum));
    }

    CPSS_NULL_PTR_CHECK_MAC(nextDevNumPtr);

    if (devNum == DEVICE_NOT_EXISTS_CNS)
        num = 0;
    else
        num = (devNum+1);

    for (; num < PRV_CPSS_MAX_PP_DEVICES_CNS; num++)
    {
        if (PRV_CPSS_PP_MAC(num) &&
            (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(num)->devFamily)))
        {
            *nextDevNumPtr = num;
            return GT_OK;
        }
    }

    /* no more devices found */
    *nextDevNumPtr = DEVICE_NOT_EXISTS_CNS;
    return /* it's not error for log */ GT_NO_MORE;
}

/**
* @internal cpssPxCfgNextDevGet function
* @endinternal
*
* @brief   Return the number of the next existing device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to start from. For the first one  should be 0xFF.
*
* @param[out] nextDevNumPtr            - number of next device after devNum.
*
* @retval GT_OK                    - on success.
* @retval GT_NO_MORE               - devNum is the last device. nextDevNumPtr will be set to 0xFF.
* @retval GT_BAD_PARAM             - devNum > max device number
* @retval GT_BAD_PTR               - nextDevNumPtr pointer is NULL.
*/
GT_STATUS cpssPxCfgNextDevGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT GT_SW_DEV_NUM *nextDevNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCfgNextDevGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, nextDevNumPtr));

    rc = internal_cpssPxCfgNextDevGet(devNum, nextDevNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, nextDevNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCfgTableNumEntriesGet function
* @endinternal
*
* @brief   the function return the number of entries of each individual table in
*         the HW
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
* @param[in] table                    - type of the specific table
*
* @param[out] numEntriesPtr            - (pointer to) number of entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCfgTableNumEntriesGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  CPSS_PX_TABLE_ENT           table,
    OUT GT_U32                      *numEntriesPtr
)
{
    /* validate the device */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    /* validate the pointer */
    CPSS_NULL_PTR_CHECK_MAC(numEntriesPtr);

    switch(table)
    {
        case     CPSS_PX_TABLE_BMA_PORT_MAPPING_E                                        :
        case     CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E                                  :
        /*TXQ-DQ*/
        case     CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E     :
        case     CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E              :
        case     CPSS_PX_TABLE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E                        :
        /*TXQ-QUEUE*/
        case     CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_MAX_QUEUE_LIMITS_E                    :
        case     CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP0_E                          :
        case     CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP1_E                          :
        case     CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP2_E                          :
        case     CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_BUF_LIMITS_E                          :
        case     CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_DESC_LIMITS_E                         :
        case     CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E                :
        /*TXQ-QCN*/
        case     CPSS_PX_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E                             :
        /*PFC*/
        case     CPSS_PX_TABLE_PFC_LLFC_COUNTERS_E                                       :
        /*CNC*/
        case     CPSS_PX_TABLE_CNC_0_COUNTERS_E                                          :
        case     CPSS_PX_TABLE_CNC_1_COUNTERS_E                                          :
        /*PCP*/
        case     CPSS_PX_TABLE_PCP_DST_PORT_MAP_TABLE_E                                  :
        case     CPSS_PX_TABLE_PCP_PORT_FILTERING_TABLE_E                                :
        /*PHA*/
        case     CPSS_PX_TABLE_PHA_HA_TABLE_E                                            :
        case     CPSS_PX_TABLE_PHA_SRC_PORT_DATA_E                                       :
        case     CPSS_PX_TABLE_PHA_TARGET_PORT_DATA_E                                    :
        case     CPSS_PX_TABLE_PHA_SHARED_DMEM_E                                         :
        case     CPSS_PX_TABLE_PHA_FW_IMAGE_E                                            :
        /**************************/
        /* MULTI-instance support */
        /**************************/
        /*TXQ-DQ*/
        case     CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG____DQ_1___E:   /* DQ[1] */
        case     CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG____DQ_1___E         :   /* DQ[1] */
        case     CPSS_PX_MULTI_INSTANCE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E____DQ_1___E                 :   /* DQ[1] */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(table);
    }

    *numEntriesPtr = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,table)->directAccessInfo.maxNumOfEntries;

    return GT_OK;
}

/**
* @internal cpssPxCfgTableNumEntriesGet function
* @endinternal
*
* @brief   the function return the number of entries of each individual table in
*         the HW
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
* @param[in] table                    - type of the specific table
*
* @param[out] numEntriesPtr            - (pointer to) number of entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCfgTableNumEntriesGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  CPSS_PX_TABLE_ENT           table,
    OUT GT_U32                      *numEntriesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCfgTableNumEntriesGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, table, numEntriesPtr));

    rc = internal_cpssPxCfgTableNumEntriesGet(devNum, table, numEntriesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, table, numEntriesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxCfgReNumberDevNum function
* @endinternal
*
* @brief   function allow the caller to modify the DB of the cpss ,
*         so all info that was 'attached' to 'oldDevNum' will be moved to 'newDevNum'.
*         NOTE:
*         1. there are no HW operations involved in this API.
*         2. This API is "ISR safe".The interrupt are disabled on API entry and enabled on API exit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] oldDevNum                - old device number
* @param[in] newDevNum                - new device number (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - the device oldDevNum not exist
* @retval GT_OUT_OF_RANGE          - the device newDevNum out of range
* @retval GT_ALREADY_EXIST         - the new device number is already used
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*
* @note This API is "ISR safe".The interrupt are disabled on API entry and enabled on API exit
*
*/
static GT_STATUS internal_cpssPxCfgReNumberDevNum
(
    IN GT_SW_DEV_NUM   oldDevNum,
    IN GT_SW_DEV_NUM   newDevNum
)
{
    GT_STATUS   rc;
    /* validate the device */
    PRV_CPSS_PX_DEV_CHECK_MAC(oldDevNum);

    /*validate the new device number*/
    CPSS_DATA_CHECK_MAX_MAC(newDevNum              ,PRV_CPSS_MAX_PP_DEVICES_CNS     );

    if(oldDevNum == newDevNum)
    {
        /* nothing more to do */
        return GT_OK;
    }

    /* swap the special DB - for re-init support */
    rc = prvCpssPpConfigDevDbRenumber(CAST_SW_DEVNUM(oldDevNum),CAST_SW_DEVNUM(newDevNum));
    if(rc != GT_OK)
    {
        return rc;
    }

    /* swap the cpss DB */
    if(PRV_CPSS_PP_CONFIG_ARR_MAC[newDevNum] != NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, "newDevNum[%d] already exists",newDevNum);
    }

    PRV_CPSS_PP_CONFIG_ARR_MAC[newDevNum] = PRV_CPSS_PP_CONFIG_ARR_MAC[oldDevNum];
    PRV_CPSS_PP_CONFIG_ARR_MAC[oldDevNum] = NULL;

    /* let the cpssDriver also 'SWAP' pointers */
    rc = prvCpssDrvHwPpRenumber(CAST_SW_DEVNUM(oldDevNum),CAST_SW_DEVNUM(newDevNum));
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxCfgReNumberDevNum function
* @endinternal
*
* @brief   function allow the caller to modify the DB of the cpss ,
*         so all info that was 'attached' to 'oldDevNum' will be moved to 'newDevNum'.
*         NOTE:
*         1. there are no HW operations involved in this API.
*         2. This API is "ISR safe".The interrupt are disabled on API entry and enabled on API exit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] oldDevNum                - old device number
* @param[in] newDevNum                - new device number (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - the device oldDevNum not exist
* @retval GT_OUT_OF_RANGE          - the device newDevNum out of range
* @retval GT_ALREADY_EXIST         - the new device number is already used
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*
* @note This API is "ISR safe".The interrupt are disabled on API entry and enabled on API exit
*
*/
GT_STATUS cpssPxCfgReNumberDevNum
(
    IN GT_SW_DEV_NUM   oldDevNum,
    IN GT_SW_DEV_NUM   newDevNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCfgReNumberDevNum);

    CPSS_API_LOCK_MAC(oldDevNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_API_LOCK_MAC(newDevNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    PRV_CPSS_INT_SCAN_LOCK();

    CPSS_LOG_API_ENTER_MAC((funcId, oldDevNum, newDevNum));

    rc = internal_cpssPxCfgReNumberDevNum(oldDevNum, newDevNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, oldDevNum, newDevNum));
    PRV_CPSS_INT_SCAN_UNLOCK();
    CPSS_API_UNLOCK_MAC(newDevNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_API_UNLOCK_MAC(oldDevNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCfgDevInfoGet function
* @endinternal
*
* @brief   the function returns device static information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
*
* @param[out] devInfoPtr               - (pointer to) device information
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCfgDevInfoGet
(
    IN  GT_SW_DEV_NUM               devNum,
    OUT CPSS_PX_CFG_DEV_INFO_STC   *devInfoPtr
)
{
    GT_STATUS   rc;

    /* validate the device */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /* validate the pointer */
    CPSS_NULL_PTR_CHECK_MAC(devInfoPtr);

    /* fill in generic part of device info */
    rc = prvCpssPpConfigDevInfoGet(CAST_SW_DEVNUM(devNum),&(devInfoPtr->genDevInfo));
    if (rc != GT_OK)
    {
        return rc;
    }

    devInfoPtr->genDevInfo.numOfLedInfPerPortGroup = 0;

    return GT_OK;
}

/**
* @internal cpssPxCfgDevInfoGet function
* @endinternal
*
* @brief   the function returns device static information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
*
* @param[out] devInfoPtr               - (pointer to) device information
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCfgDevInfoGet
(
    IN  GT_SW_DEV_NUM               devNum,
    OUT CPSS_PX_CFG_DEV_INFO_STC   *devInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCfgDevInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, devInfoPtr));

    rc = internal_cpssPxCfgDevInfoGet(devNum, devInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, devInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCfgHwAccessObjectBind function
* @endinternal
*
* @brief   The function binds/unbinds a callback routines for HW access.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] cfgAccessObjPtr          - HW access object pointer.
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                      GT_FALSE - unbind callback routines.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS internal_cpssPxCfgHwAccessObjectBind
(
    IN CPSS_PX_CFG_HW_ACCESS_OBJ_STC *cfgAccessObjPtr,
    IN GT_BOOL                     bind
)
{
    if(bind)
        cpssOsMemCpy( &prvPxTableCpssHwAccessObj, cfgAccessObjPtr, sizeof(*cfgAccessObjPtr));
    else
        cpssOsMemSet( &prvPxTableCpssHwAccessObj, 0, sizeof(*cfgAccessObjPtr));

    return GT_OK;
}

/**
* @internal cpssPxCfgHwAccessObjectBind function
* @endinternal
*
* @brief   The function binds/unbinds a callback routines for HW access.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] cfgAccessObjPtr          - HW access object pointer.
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                      GT_FALSE - unbind callback routines.
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssPxCfgHwAccessObjectBind
(
    IN CPSS_PX_CFG_HW_ACCESS_OBJ_STC *cfgAccessObjPtr,
    IN GT_BOOL                     bind
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCfgHwAccessObjectBind);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, cfgAccessObjPtr, bind));

    rc = internal_cpssPxCfgHwAccessObjectBind(cfgAccessObjPtr, bind);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, cfgAccessObjPtr, bind));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCfgHitlessWriteMethodEnableSet function
* @endinternal
*
* @brief   Enable/disable hitless write method.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  hitless write method is enable. In this case write operation
*                                      would be performed only if writing data is not equal with
*                                      affected memory contents.
*                                      GT_FALSE: hitless write method is disable. In this case write operation
*                                      is done at any rate.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCfgHitlessWriteMethodEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_BOOL       enable
)
{
    /* check parameters */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PP_MAC(devNum)->hitlessWriteMethodEnable = enable;

    return GT_OK;
}
/**
* @internal cpssPxCfgHitlessWriteMethodEnableSet function
* @endinternal
*
* @brief   Enable/disable hitless write method.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  hitless write method is enable. In this case write operation
*                                      would be performed only if writing data is not equal with
*                                      affected memory contents.
*                                      GT_FALSE: hitless write method is disable. In this case write operation
*                                      is done at any rate.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCfgHitlessWriteMethodEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_BOOL       enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCfgHitlessWriteMethodEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxCfgHitlessWriteMethodEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCfgLastRegAccessTimeStampGet function
* @endinternal
*
* @brief   Gets the last read/write time stamp counter value that represent the exact
*         timestamp of the last read/write operation to the PP registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] timeStampValuePtr        - pointer to timestamp of the last read operation.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portGroupsBmp
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCfgLastRegAccessTimeStampGet
(
    IN  GT_SW_DEV_NUM           devNum,
    OUT GT_U32                  *timeStampValuePtr
)
{
    GT_U32    regAddr;       /* register address   */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(timeStampValuePtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.globalRegs.lastReadTimeStampReg;

    /* Gets the last-read-write-time-stamp counter value */
    return prvCpssHwPpReadRegister(
        CAST_SW_DEVNUM(devNum), regAddr, timeStampValuePtr);
}

/**
* @internal cpssPxCfgLastRegAccessTimeStampGet function
* @endinternal
*
* @brief   Gets the last read/write time stamp counter value that represent the exact
*         timestamp of the last read/write operation to the PP registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] timeStampValuePtr        - pointer to timestamp of the last read operation.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portGroupsBmp
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCfgLastRegAccessTimeStampGet
(
    IN  GT_SW_DEV_NUM           devNum,
    OUT GT_U32                  *timeStampValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCfgLastRegAccessTimeStampGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, timeStampValuePtr));

    rc = internal_cpssPxCfgLastRegAccessTimeStampGet(devNum, timeStampValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, timeStampValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxCfgCntrGet function
* @endinternal
*
* @brief   Gets the MCFC counters for specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] counterType              - the specific counter type
*
* @param[out] cntPtr                   - pointer to the number of packets that were counted by MCFC unit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPxCfgCntrGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  CPSS_PX_CFG_CNTR_ENT  counterType,
    OUT GT_U32  *cntPtr
)
{
    GT_U32    regAddr;       /* register address   */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(cntPtr);

    switch (counterType)
    {
        case CPSS_PX_CFG_CNTR_RECEIVED_PFC_E:
            regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).cntrs.PFCReceivedCntr;
            break;
        case CPSS_PX_CFG_CNTR_RECEIVED_MC_E:
            regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).cntrs.MCReceivedCntr;
            break;
        case CPSS_PX_CFG_CNTR_RECEIVED_UC_E:
            regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).cntrs.UCReceivedCntr;
            break;
        case CPSS_PX_CFG_CNTR_RECEIVED_QCN_E:
            regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).cntrs.QCNReceivedCntr;
            break;
        case CPSS_PX_CFG_CNTR_RECEIVED_MC_QCN_E:
            regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).cntrs.MCQCNReceivedCntr;
            break;
        case CPSS_PX_CFG_CNTR_INGRESS_DROP_E:
            regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).cntrs.ingrDropCntr;
            break;
        case CPSS_PX_CFG_CNTR_OUT_MC_PACKETS_E:
            regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).cntrs.outgoingMcPktsCntr;
            break;
        case CPSS_PX_CFG_CNTR_OUT_QCN_TO_PFC_MSG_E:
            regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).cntrs.outgoingQCN2PFCMsgsCntr;
            break;
        case CPSS_PX_CFG_CNTR_OUT_QCN_PACKETS_E:
            regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).cntrs.outgoingQCNPktsCntr;
            break;
        case CPSS_PX_CFG_CNTR_OUT_PFC_PACKETS_E:
            regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).cntrs.outgoingPFCPktsCntr;
            break;
        case CPSS_PX_CFG_CNTR_OUT_UC_PACKETS_E:
            regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).cntrs.outgoingUcPktsCntr;
            break;
        case CPSS_PX_CFG_CNTR_OUT_MC_QCN_PACKETS_E:
            regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).cntrs.outgoingMCQCNPktsCntr;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "counterType[%d] not exists", counterType);
    }

    /* Gets the counter value */
    return prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, cntPtr);
}

/**
* @internal cpssPxCfgCntrGet function
* @endinternal
*
* @brief   Gets the MCFC counters for specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] counterType              - the specific counter type
*
* @param[out] cntPtr                   - pointer to the number of packets that were counted by MCFC unit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCfgCntrGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  CPSS_PX_CFG_CNTR_ENT  counterType,
    OUT GT_U32  *cntPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCfgCntrGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, counterType, cntPtr));

    rc = internal_cpssPxCfgCntrGet(devNum, counterType, cntPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, counterType, cntPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



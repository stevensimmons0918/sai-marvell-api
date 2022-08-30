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
* @file cpssDxChPortDiag.c
*
* @brief CPSS implementation for Port Diagnostics functionality.
*
*
* @version   1
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortDiag.h>

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
* @internal internal_cpssDxChPortDiagVoltageGet function
* @endinternal
*
* @brief   Get the voltage (in mV) of given port.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] voltagePtr               - (pointer to) voltage in mV.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS internal_cpssDxChPortDiagVoltageGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *voltagePtr
)
{
    GT_STATUS   rc;
    GT_U32      portGroupId;
    GT_U32 portMacNum; /* MAC number */
    GT_U32 chipIndex = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(voltagePtr);


    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortVoltageGet(devNum[%d], portGroup[%d], *voltage)", devNum, portGroupId);

    if(CPSS_PP_FAMILY_DXCH_FALCON_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        chipIndex = devNum * MAX_TILE_CNS * FALCON_RAVENS_PER_TILE + portMacNum / (MAX_TILE_CNS * FALCON_RAVENS_PER_TILE);
    }

    rc = mvHwsPortVoltageGet(devNum, portGroupId, chipIndex, voltagePtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal cpssDxChPortDiagVoltageGet function
* @endinternal
*
* @brief   Get the voltage (in mV) of given port.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] voltagePtr               - (pointer to) voltage in mV.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChPortDiagVoltageGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *voltagePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDiagVoltageGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, voltagePtr));

    rc = internal_cpssDxChPortDiagVoltageGet(devNum, portNum, voltagePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, voltagePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortDiagTemperatureGet function
* @endinternal
*
* @brief   Get the Temperature (in C) of given port.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] temperaturePtr           - (pointer to) Temperature (in C).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS internal_cpssDxChPortDiagTemperatureGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_32                   *temperaturePtr
)
{
    GT_STATUS   rc;
    GT_U32      portGroupId;
    GT_U32 portMacNum; /* MAC number */
    GT_U32 chipIndex = 0;
    MV_HWS_PORT_STANDARD    portMode;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(temperaturePtr);

    if(cpssDeviceRunCheck_onEmulator())
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Emulator mode : The Temperature indication must not be accessed");
    }

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if(CPSS_PP_FAMILY_DXCH_FALCON_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        chipIndex = devNum * MAX_TILE_CNS * FALCON_RAVENS_PER_TILE + portMacNum / (MAX_TILE_CNS * FALCON_RAVENS_PER_TILE);
    }

    if ( HWS_DEV_SIP_6_10_CHECK_MAC(devNum) )
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsSerdesTemperatureGet(devNum[%d], portGroup[%d], *temperature)", devNum, portGroupId);
        
        /* extract serdesNum */
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum), PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum), &portMode);
        if (rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Calling: prvCpssCommonPortIfModeToHwsTranslate(devNum[%d], portGroup[%d], *temperature)", devNum, portGroupId);
            return rc;
        }
        if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroupId, portMacNum, portMode, &curPortParams))
        {
            CPSS_LOG_INFORMATION_MAC("Calling: hwsPortModeParamsGetToBuffer(devNum[%d], portGroup[%d], *temperature)", devNum, portGroupId);
            return rc;
        }

        rc = mvHwsSerdesTemperatureGet(devNum, portGroupId, /*serdesNum*/ curPortParams.firstLaneNum, temperaturePtr);
    }
    else
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortTemperatureGet(devNum[%d], portGroup[%d], *temperature)", devNum, portGroupId);
        rc = mvHwsPortTemperatureGet(devNum, portGroupId, chipIndex, temperaturePtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortTemperatureGet(devNum[%d], portGroup[%d], *temperature)", devNum, portGroupId);
            return rc;
        }
    }

    return rc;
}

/**
* @internal cpssDxChPortDiagTemperatureGet function
* @endinternal
*
* @brief   Get the Temperature (in C) of given port.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] temperaturePtr           - (pointer to) Temperature (in C).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChPortDiagTemperatureGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_32                   *temperaturePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortDiagTemperatureGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, temperaturePtr));

    rc = internal_cpssDxChPortDiagTemperatureGet(devNum, portNum, temperaturePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, temperaturePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



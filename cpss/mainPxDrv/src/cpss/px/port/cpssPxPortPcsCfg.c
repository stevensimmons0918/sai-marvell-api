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
* @file cpssPxPortPcsCfg.c
*
* @brief CPSS implementation for PCS configuration and control facility.
*
* @version   17
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/port/private/prvCpssPxPortLog.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>

/**
* @internal internal_cpssPxPortPcsLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPcsLoopbackModeSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT      mode
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  portGroupId;/* core number of port */
    MV_HWS_PORT_STANDARD    portMode;   /* port interface in HWS format */
    MV_HWS_PORT_LB_TYPE     lbType;     /* loopback type in HWS format */
    GT_U32                  portMacNum;      /* MAC number */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;/* current interface of port */
    CPSS_PORT_SPEED_ENT     speed;/* current speed of port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacNum);
    speed =  PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacNum);
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
        return rc;

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacNum);
    if(CPSS_PX_PORT_PCS_LOOPBACK_TX2RX_E == mode)
    {/* restore application configuration for force link down if loopback enabled */
        rc = prvCpssPxPortForceLinkDownEnableSet(devNum,portNum,
        CPSS_PORTS_BMP_IS_PORT_SET_MAC(&PRV_CPSS_PX_PP_MAC(devNum)->port.portForceLinkDownBmp,portMacNum));
        if(rc != GT_OK)
            return rc;
    }
    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackSet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], lbType[%d])",devNum, portGroupId, portMacNum, portMode, HWS_PCS, DISABLE_LB);
    /* clear any old loopback type - must because of mvHwsSerdesLoopback
        internal implementation */
    rc = mvHwsPortLoopbackSet(CAST_SW_DEVNUM(devNum), portGroupId, portMacNum, portMode,
                              HWS_PCS, DISABLE_LB);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    switch(mode)
    {
        case CPSS_PX_PORT_PCS_LOOPBACK_DISABLE_E:
            lbType = DISABLE_LB;
            break;
        case CPSS_PX_PORT_PCS_LOOPBACK_TX2RX_E:
            lbType = TX_2_RX_LB;
            break;
        case CPSS_PX_PORT_PCS_LOOPBACK_RX2TX_E:
            if(PRV_CPSS_PORT_GE_E == PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            lbType = RX_2_TX_LB;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(lbType != DISABLE_LB)
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackSet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], lbType[%d])",devNum, portGroupId, portMacNum, portMode, HWS_PCS, lbType);
        rc = mvHwsPortLoopbackSet(CAST_SW_DEVNUM(devNum), portGroupId, portMacNum, portMode,
                                  HWS_PCS, lbType);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal cpssPxPortPcsLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPcsLoopbackModeSet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT    mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPcsLoopbackModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mode));

    rc = internal_cpssPxPortPcsLoopbackModeSet(devNum, portNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPcsLoopbackModeGet function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - current loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPcsLoopbackModeGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT    *modePtr
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  portGroupId;/* core number of port */
    MV_HWS_PORT_STANDARD    portMode;   /* port interface in HWS format */
    MV_HWS_PORT_LB_TYPE     lbType;     /* loopback type in HWS format */
    GT_U32                  portMacNum;      /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacNum),
                                PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacNum),
                                &portMode);
    if(rc != GT_OK)
        return rc;

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacNum);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackStatusGet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], *lbType)",devNum, portGroupId, portMacNum, portMode, HWS_PCS, &lbType);
    rc = mvHwsPortLoopbackStatusGet(CAST_SW_DEVNUM(devNum), portGroupId, portMacNum, portMode,
                                    HWS_PCS, &lbType);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    switch(lbType)
    {
        case DISABLE_LB:
            *modePtr = CPSS_PX_PORT_PCS_LOOPBACK_DISABLE_E;
            break;
        case TX_2_RX_LB:
            *modePtr = CPSS_PX_PORT_PCS_LOOPBACK_TX2RX_E;
            break;
        case RX_2_TX_LB:
            *modePtr = CPSS_PX_PORT_PCS_LOOPBACK_RX2TX_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortPcsLoopbackModeGet function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - current loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPcsLoopbackModeGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT    *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPcsLoopbackModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, modePtr));

    rc = internal_cpssPxPortPcsLoopbackModeGet(devNum, portNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortXgPcsResetStateSet function
* @endinternal
*
* @brief   Set XPCS Reset state of XG unit including internal features.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port Xpcs is under Reset
*                                      GT_FALSE - Port Xpcs is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note XLG doesn't need such treatment, because it has just one bit for reset.
*       This API for internal usage purposed to run on start and finish of port
*       ifMode & speed reconfiguration.
*
*/
GT_STATUS prvCpssPxPortXgPcsResetStateSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
)
{

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    return cpssPxPortPcsResetSet(devNum, portNum,
                                   CPSS_PORT_PCS_RESET_MODE_ALL_E,
                                   state);
}

/**
* @internal prvCpssPxPortXgPcsResetStateGet function
* @endinternal
*
* @brief   Set XPCS Reset state of XG unit including internal features.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] statePtr                 - Current reset state
*                                      GT_TRUE   - Port Xpcs is under Reset
*                                      GT_FALSE - Port Xpcs is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - statePtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortXgPcsResetStateGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *statePtr
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      portGroupId;    /*the port group Id - support multi-port-groups device */
    GT_U32      portMacNum;              /* MAC number */
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32                  numOfLanes;
    MV_HWS_PORT_STANDARD    portMode;   /* port ifMode in HWS format */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portNum);
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacNum),
                                PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacNum),
                                &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroupId, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsActiveStatusGet(devNum[%d], portGroup[%d], pcsNum[%d], pcsType[%d], *numOfLanes)", devNum, portGroupId, curPortParams.portPcsNumber, curPortParams.portPcsType);
    rc = mvHwsPcsActiveStatusGet(CAST_SW_DEVNUM(devNum), portGroupId, portMacNum, portMode, &numOfLanes);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }
    *statePtr = (0 == numOfLanes) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal internal_cpssPxPortPcsResetSet function
* @endinternal
*
* @brief   Set/unset the PCS reset for given mode on port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - Tx/Rx/All
* @param[in] state                    - If GT_TRUE, enable reset
*                                      If GT_FALSE, disable reset
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Reset only PCS unit used by port for currently configured interface.
*
*/
static GT_STATUS internal_cpssPxPortPcsResetSet
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_PCS_RESET_MODE_ENT   mode,
    IN  GT_BOOL                        state
)
{
    GT_STATUS rc;           /* return code */
    GT_U32    portGroup;    /* local core number */
    MV_HWS_RESET action;    /* reset state in HWS format */
    MV_HWS_PORT_STANDARD    portMode;   /* port ifMode in HWS format */
    GT_U32                  portMacNum;      /* MAC number */
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacNum),
                                PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacNum),
                                &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacNum);


    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup, portNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    action = (GT_TRUE == state) ? RESET : UNRESET;

    switch(mode)
    {
        case CPSS_PORT_PCS_RESET_MODE_RX_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsRxReset(devNum[%d], portGroup[%d], pcsNum[%d], pcsType[%d], action[%d])", devNum, portGroup, curPortParams.portPcsNumber, curPortParams.portPcsType, action);
            rc = mvHwsPcsRxReset(CAST_SW_DEVNUM(devNum), portGroup, curPortParams.portPcsNumber, curPortParams.portPcsType, action);
            break;

        case CPSS_PORT_PCS_RESET_MODE_TX_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);

        case CPSS_PORT_PCS_RESET_MODE_ALL_E:
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], pcsNum[%d], pcsType[%d], action[%d])", devNum, portGroup, curPortParams.portPcsNumber, curPortParams.portPcsType, action);
            rc = mvHwsPcsReset(CAST_SW_DEVNUM(devNum), portGroup, curPortParams.portPcsNumber, portMode, curPortParams.portPcsType, action);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }
    return rc;
}

/**
* @internal cpssPxPortPcsResetSet function
* @endinternal
*
* @brief   Set/unset the PCS reset for given mode on port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - Tx/Rx/All
* @param[in] state                    - If GT_TRUE, enable reset
*                                      If GT_FALSE, disable reset
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Reset only PCS unit used by port for currently configured interface.
*
*/
GT_STATUS cpssPxPortPcsResetSet
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_PCS_RESET_MODE_ENT   mode,
    IN  GT_BOOL                        state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPcsResetSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mode, state));

    rc = internal_cpssPxPortPcsResetSet(devNum, portNum, mode, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mode, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPcsGearBoxStatusGet function
* @endinternal
*
* @brief   Return PCS Gear Box lock status (true - locked /false - not locked).
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] gbLockPtr                - Gear Box lock state on serdes:
*                                      GT_TRUE  - locked;
*                                      GT_FALSE - not locked.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - gbLockPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPcsGearBoxStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *gbLockPtr
)
{
    GT_STATUS               rc;   /* return code */
    GT_U32                  portGroup; /* local core number */
    MV_HWS_PORT_STANDARD    portMode;   /* port ifMode in HWS format */
    GT_U32                  portMacNum;              /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacNum),
                                PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacNum),
                                &portMode);
    if(rc != GT_OK)
        return rc;

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portNum);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortCheckGearBox(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], *laneLock)", devNum, portGroup, portMacNum, portMode);
    rc = mvHwsPortCheckGearBox(CAST_SW_DEVNUM(devNum), portGroup, portMacNum, portMode, gbLockPtr);

    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }
    return rc;
}

/**
* @internal cpssPxPortPcsGearBoxStatusGet function
* @endinternal
*
* @brief   Return PCS Gear Box lock status (true - locked /false - not locked).
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] gbLockPtr                - Gear Box lock state on serdes:
*                                      GT_TRUE  - locked;
*                                      GT_FALSE - not locked.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - gbLockPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPcsGearBoxStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *gbLockPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPcsGearBoxStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, gbLockPtr));

    rc = internal_cpssPxPortPcsGearBoxStatusGet(devNum, portNum, gbLockPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, gbLockPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortPcsSyncStatusGet function
* @endinternal
*
* @brief   Return PCS Sync status from XGKR sync block.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] syncPtr                  - Sync status from XGKR sync block :
*                                      GT_TRUE  - synced;
*                                      GT_FALSE - not synced.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - syncPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPxPortPcsSyncStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *syncPtr
)
{
    GT_STATUS               rc;     /* return code */
    GT_U32                  portGroup;  /* local core number */
    MV_HWS_PORT_STANDARD    portMode;   /* port ifMode in HWS format */
    MV_HWS_TEST_GEN_STATUS  status; /* structure for sync status get */
    GT_U32                  i;     /* iterator */
    GT_U32                  portMacMap; /*port Mac Map*/

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap),
                                PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap),
                                &portMode);
    if(rc != GT_OK)
        return rc;

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);
    i = 0;
    do
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortTestGeneratorStatus(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], portPattern[%d], *status)",devNum, portGroup, portMacMap, portMode, TEST_GEN_PRBS7);
        rc = mvHwsPortTestGeneratorStatus(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, portMode,
                                    TEST_GEN_PRBS7,/* not used by API, value doesn't really matter */
                                    &status);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
        if(0 == status.checkerLock)
        {/* no lock/sync */
            break;
        }

    }while(i++ < 1000);

    *syncPtr = BIT2BOOL_MAC(status.checkerLock);
    return GT_OK;
}

/**
* @internal cpssPxPortPcsSyncStatusGet function
* @endinternal
*
* @brief   Return PCS Sync status from XGKR sync block.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] syncPtr                  - Sync status from XGKR sync block :
*                                      GT_TRUE  - synced;
*                                      GT_FALSE - not synced.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - syncPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPcsSyncStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *syncPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPcsSyncStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, syncPtr));

    rc = internal_cpssPxPortPcsSyncStatusGet(devNum, portNum, syncPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, syncPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPcsSyncStableStatusGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes when it became be stable(true/false).
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - the stable signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on signal is not stable during 1000ms.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Using algorithm to detect Serdes Signal to be stable.
*
*/
GT_STATUS internal_cpssPxPortPcsSyncStableStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32 interval = 100;
    GT_U32 window = 5;          /* 5 * 10msec = 50msec continuous time for stable signal indication */
    GT_U32 window_nosignal = 2; /* 2 * 10msec = 20msec continuous time for stable no signal indication */
    GT_U32 delayMS = 10;
    GT_BOOL signalStateFirst;
    GT_BOOL signalStateNext;
    GT_U32 stableInterval;
    GT_U32 portMacMap;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
    if(NULL == signalStatePtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    *signalStatePtr = 0;

    rc = cpssPxPortPcsSyncStatusGet(devNum, portNum, &signalStateFirst);
    if(GT_OK != rc)
    {
        return rc;
    }
    stableInterval = 0;
    for(i = 0; i < interval; i++)
    {
        rc = cpssPxPortPcsSyncStatusGet(devNum, portNum, &signalStateNext);
        if(GT_OK != rc)
        {
            return rc;
        }
        if(signalStateNext == signalStateFirst)
        {
            stableInterval++;
            if((GT_TRUE == signalStateFirst) && (stableInterval >= window))
            {
                *signalStatePtr = signalStateFirst;
                return GT_OK;
            }
            else if((GT_FALSE == signalStateFirst) && (stableInterval >= window_nosignal))
            {
                *signalStatePtr = signalStateFirst;
                return GT_OK;
            }
        }
        else
        {
            signalStateFirst = signalStateNext;
            stableInterval = 0;
        }
        cpssOsTimerWkAfter(delayMS);
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssPxPortPcsSyncStableStatusGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes when it became be stable(true/false).
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - the stable signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on signal is not stable during 1000ms.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Using algorithm to detect Serdes Signal to be stable.
*
*/
GT_STATUS cpssPxPortPcsSyncStableStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPcsSyncStableStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, signalStatePtr));

    rc = internal_cpssPxPortPcsSyncStableStatusGet(devNum, portNum, signalStatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, signalStatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortXgLanesSwapEnableSet function
* @endinternal
*
* @brief   Enable/Disable swapping XAUI port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Swap Lanes by follows:
*                                      The lanes are swapped by follows:
*                                      Lane 0 PSC Tx data is connected to SERDES Lane 3
*                                      Lane 1 PSC Tx data is connected to SERDES Lane 2
*                                      Lane 2 PSC Tx data is connected to SERDES Lane 1
*                                      Lane 3 PSC Tx data is connected to SERDES Lane 0
*                                      - GT_FALSE - Normal operation (no swapping)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note Supported for XAUI ports only.
*
*/
static GT_STATUS internal_cpssPxPortXgLanesSwapEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_U32      regAddr;           /* register address */
    GT_U32      value;             /* register field value */
    GT_BOOL      resetState;        /* reset state */
    GT_STATUS   rc;                /* return status */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32      lane, portMacNum;        /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) < PRV_CPSS_PORT_XG_E)
    {
        /* supported only on XG ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacNum);
    /* Configure Reset PCS */
    rc = prvCpssPxPortXgPcsResetStateGet(devNum, portNum, &resetState);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(resetState != GT_TRUE)
    {
        rc = prvCpssPxPortXgPcsResetStateSet(devNum, portNum, GT_TRUE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    for(lane = 0; lane < 4; lane++)
    {
        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
              GOP.perPortRegs[portMacNum].laneConfig1[lane];
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        /* Swap Lanes */
        if(enable == GT_TRUE)
        {
            value = (3 - lane) | ((3 - lane) << 3);
        }
        else
        {
            value = lane | (lane << 3);
        }

        /* Configure RxSwpSel / TxSwpSel */
        rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId, regAddr, 8, 6, value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* restore PSC Reset state */
    if(resetState != GT_TRUE)
    {
        rc = prvCpssPxPortXgPcsResetStateSet(devNum, portNum, GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal cpssPxPortXgLanesSwapEnableSet function
* @endinternal
*
* @brief   Enable/Disable swapping XAUI port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Swap Lanes by follows:
*                                      The lanes are swapped by follows:
*                                      Lane 0 PSC Tx data is connected to SERDES Lane 3
*                                      Lane 1 PSC Tx data is connected to SERDES Lane 2
*                                      Lane 2 PSC Tx data is connected to SERDES Lane 1
*                                      Lane 3 PSC Tx data is connected to SERDES Lane 0
*                                      - GT_FALSE - Normal operation (no swapping)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note Supported for XAUI ports only.
*
*/
GT_STATUS cpssPxPortXgLanesSwapEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortXgLanesSwapEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortXgLanesSwapEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortXgLanesSwapEnableGet function
* @endinternal
*
* @brief   Gets status of swapping XAUI port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - pointer to status of swapping XAUI PHY SERDES Lanes.
*                                      - GT_TRUE  - Swap Lanes by follows:
*                                      The lanes are swapped by follows:
*                                      Lane 0 PSC Tx data is connected to SERDES Lane 3
*                                      Lane 1 PSC Tx data is connected to SERDES Lane 2
*                                      Lane 2 PSC Tx data is connected to SERDES Lane 1
*                                      Lane 3 PSC Tx data is connected to SERDES Lane 0
*                                      - GT_FALSE - Normal operation (no swapping)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Supported for XAUI ports only.
*
*/
static GT_STATUS internal_cpssPxPortXgLanesSwapEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_STATUS   rc;      /* return status */
    GT_U32      portMacNum;        /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) < PRV_CPSS_PORT_XG_E)
    {
        /* supported only on XG ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
          GOP.perPortRegs[portMacNum].laneConfig1[0];
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* status of swapping XAUI PHY SERDES Lanes */
    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                                            PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacNum),
                                            regAddr, 8, 6, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (value == 0) ? GT_FALSE : GT_TRUE;
    return GT_OK;
}

/**
* @internal cpssPxPortXgLanesSwapEnableGet function
* @endinternal
*
* @brief   Gets status of swapping XAUI port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - pointer to status of swapping XAUI PHY SERDES Lanes.
*                                      - GT_TRUE  - Swap Lanes by follows:
*                                      The lanes are swapped by follows:
*                                      Lane 0 PSC Tx data is connected to SERDES Lane 3
*                                      Lane 1 PSC Tx data is connected to SERDES Lane 2
*                                      Lane 2 PSC Tx data is connected to SERDES Lane 1
*                                      Lane 3 PSC Tx data is connected to SERDES Lane 0
*                                      - GT_FALSE - Normal operation (no swapping)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Supported for XAUI ports only.
*
*/
GT_STATUS cpssPxPortXgLanesSwapEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortXgLanesSwapEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortXgLanesSwapEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortXgPscLanesSwapSet function
* @endinternal
*
* @brief   Set swapping configuration of XAUI port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Rx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
* @param[in] txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Tx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - wrong SERDES lane
* @retval GT_BAD_VALUE             - multiple connection detected
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported only for 4 PCS lanes for XAUI port modes,
*       2 PCS lanes for RXAUI mode
*
*/
static GT_STATUS internal_cpssPxPortXgPscLanesSwapSet
(
    IN GT_SW_DEV_NUM         devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_U32 rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS],
    IN GT_U32 txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS]
)
{
    GT_U32      regAddr;                 /* register address */
    GT_U32      value;                   /* register field value */
    GT_BOOL     resetState;              /* original reset state */
    GT_U32      lane;                    /* iterator */
    GT_U32      rxLaneBitMap, txLaneBitMap;/* auxilary bit maps to check multiple conn. */
    GT_STATUS   rc;                      /* return status */
    GT_U32      portGroupId;             /*the port group Id - support multi-port-groups device */
    GT_U32      portMacNum;              /* MAC number */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;    /* current interface of port */
    GT_U32                          numOfPcsLanes;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(rxSerdesLaneArr);
    CPSS_NULL_PTR_CHECK_MAC(txSerdesLaneArr);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) < PRV_CPSS_PORT_XG_E)
    {
        /* supported only on XG ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacNum);

    rxLaneBitMap = 0x0;
    txLaneBitMap = 0x0;

    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacNum);
    numOfPcsLanes = (ifMode == CPSS_PORT_INTERFACE_MODE_RXAUI_E) ? 2 : 4;

    for( lane = 0 ; lane < numOfPcsLanes ; lane++ )
    {
        if(rxSerdesLaneArr[lane] >= numOfPcsLanes ||
           txSerdesLaneArr[lane] >= numOfPcsLanes)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        rxLaneBitMap |= ( 1 << rxSerdesLaneArr[lane]);
        txLaneBitMap |= ( 1 << txSerdesLaneArr[lane]);
    }

    if( (rxLaneBitMap != (GT_U32)(( 1 << numOfPcsLanes ) - 1)) ||
        (txLaneBitMap != (GT_U32)(( 1 << numOfPcsLanes ) - 1)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }

    /* check if XPCS is under Reset  */
   rc = prvCpssPxPortXgPcsResetStateGet(devNum, portNum, &resetState);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(resetState != GT_TRUE)
    {
        /* perform MAC RESET */
        rc = prvCpssPxPortMacResetStateSet(devNum, portNum, GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxPortMacResetStateSet: error for portNum = %d\n", portNum);
        }
        /* perform XPCS RESET */
        rc = prvCpssPxPortXgPcsResetStateSet(devNum, portNum, GT_TRUE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    for( lane = 0 ; lane < CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS ; lane++ )
    {
        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].laneConfig1[lane];
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        value = txSerdesLaneArr[lane] + (rxSerdesLaneArr[lane] << 3);

        /* Configure RxSwpSel / TxSwpSel */
        rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, 8, 6, value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* restore PSC Reset state */
    if(resetState != GT_TRUE)
    {
        /* perform XPCS UNRESET */
        rc = prvCpssPxPortXgPcsResetStateSet(devNum, portNum, GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* perform MAC UNRESET */
        rc = prvCpssPxPortMacResetStateSet(devNum, portNum, GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxPortMacResetStateSet: error for portNum = %d\n", portNum);
        }
    }

    return GT_OK;

}

/**
* @internal cpssPxPortXgPscLanesSwapSet function
* @endinternal
*
* @brief   Set swapping configuration of XAUI port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Rx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
* @param[in] txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Tx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - wrong SERDES lane
* @retval GT_BAD_VALUE             - multiple connection detected
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported only for 4 PCS lanes for XAUI port modes,
*       2 PCS lanes for RXAUI mode
*
*/
GT_STATUS cpssPxPortXgPscLanesSwapSet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS],
    IN GT_U32 txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortXgPscLanesSwapSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, rxSerdesLaneArr, txSerdesLaneArr));

    rc = internal_cpssPxPortXgPscLanesSwapSet(devNum, portNum, rxSerdesLaneArr, txSerdesLaneArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, rxSerdesLaneArr, txSerdesLaneArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortXgPscLanesSwapGet function
* @endinternal
*
* @brief   Get swapping configuration of XAUI port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Rx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
* @param[out] txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Tx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported for XAUI ports only.
*
*/
static GT_STATUS internal_cpssPxPortXgPscLanesSwapGet
(
    IN GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32 rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS],
    OUT GT_U32 txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS]
)
{
    GT_U32      regAddr;                 /* register address */
    GT_U32      value;                   /* register field value */
    GT_U32      lane;                    /* iterator */
    GT_STATUS   rc;                      /* return status */
    GT_U32      portMacNum;              /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(rxSerdesLaneArr);
    CPSS_NULL_PTR_CHECK_MAC(txSerdesLaneArr);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) < PRV_CPSS_PORT_XG_E)
    {
        /* supported only on XG ports */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for( lane = 0 ; lane < CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS ; lane++ )
    {
        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
              GOP.perPortRegs[portMacNum].laneConfig1[lane];
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        /* Get RxSwpSel / TxSwpSel */
        rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                                                PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacNum),
                                                regAddr, 8, 6, &value);
        if(rc != GT_OK)
        {
            return rc;
        }

        txSerdesLaneArr[lane] = value & 0x7;
        rxSerdesLaneArr[lane] = ( value >> 3 ) & 0x7;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortXgPscLanesSwapGet function
* @endinternal
*
* @brief   Get swapping configuration of XAUI port SERDES Lanes.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Rx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
* @param[out] txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS] - array for binding port Tx PSC and SERDES lanes:
*                                      array index is PSC lane, array value is SERDES lane.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported for XAUI ports only.
*
*/
GT_STATUS cpssPxPortXgPscLanesSwapGet
(
    IN GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32 rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS],
    OUT GT_U32 txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortXgPscLanesSwapGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, rxSerdesLaneArr, txSerdesLaneArr));

    rc = internal_cpssPxPortXgPscLanesSwapGet(devNum, portNum, rxSerdesLaneArr, txSerdesLaneArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, rxSerdesLaneArr, txSerdesLaneArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortPcsLoopbackModeSetWrapper function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - define loopback type or no loopback
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortPcsLoopbackModeSetWrapper
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_PHYSICAL_PORT_NUM               portNum,
    IN  CPSS_PORT_PCS_LOOPBACK_MODE_ENT    mode
)
{
    GT_STATUS rc;

    rc = cpssPxPortPcsLoopbackModeSet(devNum,portNum,(CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT)mode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling cpssPxPortPcsLoopbackModeSet from wrapper failed=%d", rc);
    }

    return GT_OK;
}


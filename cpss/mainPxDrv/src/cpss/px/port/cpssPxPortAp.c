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
* @file cpssPxPortAp.c
*
* @brief CPSS implementation for 802.3ap standard (defines the auto negotiation
* for backplane Ethernet) configuration and control facility for Px family
*
* @version   1
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/common/cpssTypes.h>
#include <cpss/px/port/cpssPxPortAp.h>

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/px/port/private/prvCpssPxPortLog.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/PizzaArbiter/prvCpssPxPortPizzaArbiter.h>
#include <cpss/px/port/private/prvCpssPxPortIfModeCfgPipeResource.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/port/cpssPortCtrl.h>

#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsLion2.h>

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define AP_DBG_EN
#ifdef AP_DBG_EN
    static GT_U32   allowPrint=0;/* option to disable the print in runtime*/
    #define AP_DBG_PRINT_MAC(x) if(allowPrint) cpssOsPrintSync x

extern GT_U32 apPxPrintControl(IN GT_U32  allowPrintNew)
{
    GT_U32  oldState = allowPrint;
    allowPrint = allowPrintNew;

    return oldState;
}

static GT_U32   apPxSemPrintEn = 0; /* option to disable the print in runtime*/
extern GT_U32 apPxSemPrint(IN GT_U32  allowPrintNew)
{
    GT_U32  oldState = apPxSemPrintEn;
    apPxSemPrintEn = allowPrintNew;

    return oldState;
}

#else
    #define AP_DBG_PRINT_MAC(x)
#endif

/* CPSS suggested defaults for AP configuration */
static CPSS_PX_PORT_AP_PARAMS_STC prvCpssPxPortApDefaultParams =
{
    /* fcPause */  GT_TRUE,
    /* fcAsmDir */ CPSS_PX_PORT_AP_FLOW_CONTROL_SYMMETRIC_E,
    /* fecSupported */ GT_TRUE,
    /* fecRequired */ GT_FALSE,
    /* noneceDisable */ GT_TRUE,
    /* laneNum */       0,
    /* modesAdvertiseArr */
    {
        {CPSS_PORT_INTERFACE_MODE_1000BASE_X_E, CPSS_PORT_SPEED_1000_E}
        ,{CPSS_PORT_INTERFACE_MODE_XGMII_E, CPSS_PORT_SPEED_10000_E}
        ,{CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E}
        ,{CPSS_PORT_INTERFACE_MODE_KR4_E, CPSS_PORT_SPEED_40000_E}
        ,{CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E, CPSS_PORT_SPEED_NA_E}
    },
    {
        CPSS_PORT_FEC_MODE_ENABLED_E,
        CPSS_PORT_FEC_MODE_ENABLED_E,
        CPSS_PORT_FEC_MODE_ENABLED_E,
        CPSS_PORT_FEC_MODE_ENABLED_E,
        CPSS_PORT_FEC_MODE_ENABLED_E
    },
    {
        CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_PORT_FEC_MODE_DISABLED_E
    }
};

/**
* @internal internal_cpssPxPortApEnableSet function
* @endinternal
*
* @brief   Enable/disable AP engine (loads AP code into shared memory and starts AP
*         engine).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE  -  AP on port group
*                                      GT_FALSE - disbale
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortApEnableSet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_BOOL             enable
)
{
    GT_STATUS   rc;         /* return code */

    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, prvCpssLogErrorMsgDeviceNotExist, devNum);

    /* engaged before info about devices configured */
    AP_DBG_PRINT_MAC(("cpssPxPortApEnableSet:devNum=%d,enable=%d\n",
                        devNum, enable));

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsServiceCpuEnable(devNum[%d], enable[%d])", devNum, enable);
    rc = mvHwsServiceCpuEnable(CAST_SW_DEVNUM(devNum), enable);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal cpssPxPortApEnableSet function
* @endinternal
*
* @brief   Enable/disable AP engine (loads AP code into shared memory and starts AP
*         engine).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE  -  AP on port group
*                                      GT_FALSE - disbale
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApEnableSet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_BOOL             enable
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApEnableSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPortApEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortApEnableGet function
* @endinternal
*
* @brief   Get AP engine enabled and functional status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - enabledPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortApEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enabledPtr
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enabledPtr);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsServiceCpuEnableGet(devNum[%d])", devNum);
    AP_DBG_PRINT_MAC(("mvHwsServiceCpuEnableGet(%d)\n", devNum));
    *enabledPtr = mvHwsServiceCpuEnableGet(CAST_SW_DEVNUM(devNum));
    CPSS_LOG_INFORMATION_MAC("service cpu %s", (*enabledPtr) ? "enabled" : "disabled");
    AP_DBG_PRINT_MAC(("service cpu %s\n", (*enabledPtr) ? "enabled" : "disabled"));

    return GT_OK;
}

/**
* @internal cpssPxPortApEnableGet function
* @endinternal
*
* @brief   Get AP engine enabled and functional on port group (local core) status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port group or device
* @retval GT_BAD_PTR               - enabledPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enabledPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enabledPtr));

    rc = internal_cpssPxPortApEnableGet(devNum, enabledPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enabledPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortApModesVectorBuild function
* @endinternal
*
* @brief   Get array of port modes in CPSS format and build advertisement array in
*         HWS format.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] modesAdvertiseArrPtr     - array of advertised modes in CPSS format
* @param[in] fecRequestedArrPtr       - array of requested FEC modes for every pair in
* @param[in] modesAdvertiseArrPtr
* @param[in] fecSupported             - FEC supported negotiation advertisement parameter for
*                                      interfaces supporting only single type of FEC (or FC or RS)
* @param[in] fecRequired              - FEC required negotiation advertisement parameter for
*                                      interfaces supporting only single type of FEC (or FC or RS)
*
* @param[out] sdVecSizeMaxPtr          - maximum number of Serdes required in current configuration
*                                      e.g. if advertised 10G KR only sdVecSizeMax = 1;
*                                      if advertised 10G KR and 40G KR or 40G KR only sdVecSizeMax = 4;
* @param[out] sdVectorMaxPtr           - list of serdes lanes used by most "fat" interface
* @param[out] apCfgPtr                 - advertisement structure in HWS format
* @param[out] singleModePtr            - true - single interface advertised
*                                      false - multiple interfaces advertised
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPortApModesVectorBuild
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MODE_SPEED_STC    *modesAdvertiseArrPtr,
    IN  CPSS_PORT_FEC_MODE_ENT      *fecAbilityArrPtr,
    IN  CPSS_PORT_FEC_MODE_ENT      *fecRequestedArrPtr,
    IN  GT_BOOL                     fecSupported,
    IN  GT_BOOL                     fecRequired,
    MV_HWS_PORT_INIT_PARAMS         *portParams,
    OUT MV_HWS_AP_CFG               *apCfgPtr,
    OUT GT_BOOL                     *singleModePtr
)
{
    GT_STATUS   rc = GT_OK;     /* return code */
    GT_U32      i;              /* iterator */
    GT_U32      localPort;      /* number of port in core */
    MV_HWS_PORT_STANDARD        portMode = NON_SUP_MODE;
    MV_HWS_PORT_INIT_PARAMS     tmpPortParams;
    GT_U32      *modesVectorPtr; /* pointer to bitmap of suggested interfaces */
    GT_U32      *fecAdvertiseArray; /* pointer to bitmap of suggested FEC modes */
    GT_U32      *fecRequestArray; /* pointer to bitmap of requested FEC modes */

    modesVectorPtr = &(apCfgPtr->modesVector);
    fecAdvertiseArray = &(apCfgPtr->fecAdvanceAbil);
    fecRequestArray = &(apCfgPtr->fecAdvanceReq);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, localPort);

    portParams->numOfActLanes = 0;
    for(i = 0, *modesVectorPtr = 0, *fecRequestArray = 0, *fecAdvertiseArray = 0;
         (modesAdvertiseArrPtr[i].ifMode != CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E)
             && (modesAdvertiseArrPtr[i].ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
             && (i < CPSS_PX_PORT_AP_IF_ARRAY_SIZE_CNS);
         i++)
    {
        switch(modesAdvertiseArrPtr[i].ifMode)
        {
            case CPSS_PORT_INTERFACE_MODE_SGMII_E:
            case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
                if(modesAdvertiseArrPtr[i].speed != CPSS_PORT_SPEED_1000_E)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                portMode = _1000Base_X;
                AP_CTRL_1000Base_KX_SET(*modesVectorPtr, 1);
                break;
            case CPSS_PORT_INTERFACE_MODE_XGMII_E:
                if(modesAdvertiseArrPtr[i].speed != CPSS_PORT_SPEED_10000_E)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                portMode = _10GBase_KX4;
                AP_CTRL_10GBase_KX4_SET(*modesVectorPtr, 1);
                break;
            case CPSS_PORT_INTERFACE_MODE_KR_E:
                if(CPSS_PORT_SPEED_10000_E == modesAdvertiseArrPtr[i].speed)
                {
                    apCfgPtr->fecReq = fecRequired;
                    apCfgPtr->fecSup = fecSupported;
                    portMode = _10GBase_KR;
                    AP_CTRL_10GBase_KR_SET(*modesVectorPtr, 1);
                }
                else if(CPSS_PORT_SPEED_25000_E == modesAdvertiseArrPtr[i].speed)
                {
                    switch(fecRequestedArrPtr[i])
                    {
                        case CPSS_PORT_FEC_MODE_ENABLED_E:
                            *fecRequestArray |= AP_ST_HCD_FEC_RES_FC << FEC_ADVANCE_BASE_R_SHIFT;
                            break;
                        case CPSS_PORT_RS_FEC_MODE_ENABLED_E:
                            *fecRequestArray |= AP_ST_HCD_FEC_RES_RS << FEC_ADVANCE_BASE_R_SHIFT;
                            break;
                        case CPSS_PORT_BOTH_FEC_MODE_ENABLED_E:
                            *fecRequestArray |= (AP_ST_HCD_FEC_RES_RS | AP_ST_HCD_FEC_RES_FC)
                                                                    << FEC_ADVANCE_BASE_R_SHIFT;
                            break;
                        default:
                            /* do nothing to not delete advertisement from other modes */
                            break;
                    }
                    portMode = _25GBase_KR;
                    AP_CTRL_25GBase_KR1_SET(*modesVectorPtr, 1);
                    AP_CTRL_25GBase_KR1S_SET(*modesVectorPtr, 1);
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;

            case CPSS_PORT_INTERFACE_MODE_CR_E:
                if(CPSS_PORT_SPEED_25000_E == modesAdvertiseArrPtr[i].speed)
                {
                    switch(fecRequestedArrPtr[i])
                    {
                        case CPSS_PORT_FEC_MODE_ENABLED_E:
                            *fecRequestArray |= AP_ST_HCD_FEC_RES_FC << FEC_ADVANCE_BASE_R_SHIFT;
                            break;
                        case CPSS_PORT_RS_FEC_MODE_ENABLED_E:
                            *fecRequestArray |= AP_ST_HCD_FEC_RES_RS << FEC_ADVANCE_BASE_R_SHIFT;
                            break;
                        case CPSS_PORT_BOTH_FEC_MODE_ENABLED_E:
                            *fecRequestArray |= (AP_ST_HCD_FEC_RES_RS | AP_ST_HCD_FEC_RES_FC)
                                                                    << FEC_ADVANCE_BASE_R_SHIFT;
                            break;
                        default:
                            /* do nothing to not delete advertisement from other modes */
                            break;
                    }
                    portMode = _25GBase_CR;
                        AP_CTRL_25GBase_CR1_SET(*modesVectorPtr, 1);
                        AP_CTRL_25GBase_CR1S_SET(*modesVectorPtr, 1);
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            case CPSS_PORT_INTERFACE_MODE_CR_S_E:
            case CPSS_PORT_INTERFACE_MODE_KR_S_E:
                 switch(fecRequestedArrPtr[i])
                 {
                    case CPSS_PORT_FEC_MODE_ENABLED_E:
                    case CPSS_PORT_RS_FEC_MODE_ENABLED_E:
                        *fecRequestArray |= AP_ST_HCD_FEC_RES_FC << FEC_ADVANCE_BASE_R_SHIFT;
                        break;
                    default:
                        /* do nothing to not delete advertisement from other modes */
                        break;
                }
                if(CPSS_PORT_SPEED_25000_E == modesAdvertiseArrPtr[i].speed)
                {
                    if(modesAdvertiseArrPtr[i].ifMode == CPSS_PORT_INTERFACE_MODE_KR_S_E)
                    {
                        portMode = _25GBase_KR;
                        AP_CTRL_25GBase_KR1S_SET(*modesVectorPtr, 1);
                    }
                    else
                    {
                        portMode = _25GBase_CR;
                        AP_CTRL_25GBase_CR1S_SET(*modesVectorPtr, 1);
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            case CPSS_PORT_INTERFACE_MODE_KR_C_E:
            case CPSS_PORT_INTERFACE_MODE_KR2_C_E:
            case CPSS_PORT_INTERFACE_MODE_CR_C_E:
            case CPSS_PORT_INTERFACE_MODE_CR2_C_E:
                switch (fecAbilityArrPtr[i])
                {
                case CPSS_PORT_FEC_MODE_ENABLED_E:
                    *fecAdvertiseArray |= AP_ST_HCD_FEC_RES_FC << FEC_ADVANCE_CONSORTIUM_SHIFT;
                    break;
                case CPSS_PORT_RS_FEC_MODE_ENABLED_E:
                    *fecAdvertiseArray |= AP_ST_HCD_FEC_RES_RS << FEC_ADVANCE_CONSORTIUM_SHIFT;
                    break;
                case CPSS_PORT_BOTH_FEC_MODE_ENABLED_E:
                    *fecAdvertiseArray |= AP_ST_HCD_FEC_RES_BOTH << FEC_ADVANCE_CONSORTIUM_SHIFT;
                    break;
                case CPSS_PORT_FEC_MODE_DISABLED_E:
                default:
                    break;
                }
                switch (fecRequestedArrPtr[i])
                {
                case CPSS_PORT_FEC_MODE_ENABLED_E:
                    *fecRequestArray |= AP_ST_HCD_FEC_RES_FC << FEC_ADVANCE_CONSORTIUM_SHIFT;
                    break;
                case CPSS_PORT_RS_FEC_MODE_ENABLED_E:
                    *fecRequestArray |= AP_ST_HCD_FEC_RES_RS << FEC_ADVANCE_CONSORTIUM_SHIFT;
                    break;
                case CPSS_PORT_BOTH_FEC_MODE_ENABLED_E:
                    *fecRequestArray |= AP_ST_HCD_FEC_RES_BOTH << FEC_ADVANCE_CONSORTIUM_SHIFT;
                case CPSS_PORT_FEC_MODE_DISABLED_E:
                default:
                    break;
                }
                if(CPSS_PORT_SPEED_25000_E == modesAdvertiseArrPtr[i].speed)
                {
                    if(modesAdvertiseArrPtr[i].ifMode == CPSS_PORT_INTERFACE_MODE_KR_C_E)
                    {
                        AP_DBG_PRINT_MAC(("cpssPxPortApPortConfigSet:KR1 CONSORTIUM \n"));
                        portMode = _25GBase_KR_C;
                        AP_CTRL_25GBase_KR_CONSORTIUM_SET(*modesVectorPtr, 1);
                        AP_DBG_PRINT_MAC(("cpssPxPortApPortConfigSet:KR1 CONSORTIUM rc %d\n",rc));
                    }
                    else
                    {
                        AP_DBG_PRINT_MAC(("cpssPxPortApPortConfigSet:CR1 CONSORTIUM \n"));
                        portMode = _25GBase_CR_C;
                        AP_CTRL_25GBase_CR_CONSORTIUM_SET(*modesVectorPtr, 1);
                        AP_DBG_PRINT_MAC(("cpssPxPortApPortConfigSet:CR1 CONSORTIUM rc %d\n",rc));
                    }
                }
                else if(CPSS_PORT_SPEED_50000_E == modesAdvertiseArrPtr[i].speed)
                {
                    if(modesAdvertiseArrPtr[i].ifMode == CPSS_PORT_INTERFACE_MODE_KR2_C_E)
                    {
                        AP_DBG_PRINT_MAC(("cpssPxPortApPortConfigSet:KR2 CONSORTIUM \n"));
                        portMode = _50GBase_KR2_C;
                        AP_CTRL_50GBase_KR2_CONSORTIUM_SET(*modesVectorPtr, 1);
                        AP_DBG_PRINT_MAC(("cpssPxPortApPortConfigSet:KR2 CONSORTIUM rc %d\n",rc));
                    }
                    else
                    {
                        AP_DBG_PRINT_MAC(("cpssPxPortApPortConfigSet:CR2 CONSORTIUM \n"));
                        portMode = _50GBase_CR2_C;
                        AP_CTRL_50GBase_CR2_CONSORTIUM_SET(*modesVectorPtr, 1);
                        AP_DBG_PRINT_MAC(("cpssPxPortApPortConfigSet:CR2 CONSORTIUM rc %d\n",rc));
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            case CPSS_PORT_INTERFACE_MODE_KR4_E:
                if(CPSS_PORT_SPEED_100G_E == modesAdvertiseArrPtr[i].speed)
                {
                    portMode = _100GBase_KR4;
                    AP_CTRL_100GBase_KR4_SET(*modesVectorPtr, 1);
                    /* AP_CTRL_100GBase_CR4_SET(*modesVectorPtr, 1); */
                    /* for 100G RS-FEC is mandatory - not subject for negotiation */
                }
                else if(CPSS_PORT_SPEED_40000_E == modesAdvertiseArrPtr[i].speed)
                {
                    apCfgPtr->fecReq = fecRequired;
                    apCfgPtr->fecSup = fecSupported;
                    portMode = _40GBase_KR4;
                    AP_CTRL_40GBase_KR4_SET(*modesVectorPtr, 1);
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;

            case CPSS_PORT_INTERFACE_MODE_CR4_E:
                if(CPSS_PORT_SPEED_100G_E == modesAdvertiseArrPtr[i].speed)
                {
                    portMode = _100GBase_CR4;
                    AP_CTRL_100GBase_CR4_SET(*modesVectorPtr, 1);
                }
                else if(CPSS_PORT_SPEED_40000_E == modesAdvertiseArrPtr[i].speed)
                {
                    apCfgPtr->fecReq = fecRequired;
                    apCfgPtr->fecSup = fecSupported;
                    portMode = _40GBase_CR4;
                    AP_CTRL_40GBase_CR4_SET(*modesVectorPtr, 1);
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Not allow CR and KR simultaneously */
        if((AP_CTRL_40GBase_KR4_GET(*modesVectorPtr) || AP_CTRL_10GBase_KR_GET(*modesVectorPtr) || AP_CTRL_10GBase_KX4_GET(*modesVectorPtr) || AP_CTRL_1000Base_KX_GET(*modesVectorPtr) || AP_CTRL_100GBase_KR4_GET(*modesVectorPtr) ||
            AP_CTRL_25GBase_KR1S_GET(*modesVectorPtr) || AP_CTRL_25GBase_KR1_GET(*modesVectorPtr) || AP_CTRL_25GBase_KR_CONSORTIUM_GET(*modesVectorPtr) || AP_CTRL_50GBase_KR2_CONSORTIUM_GET(*modesVectorPtr)) &&
           (AP_CTRL_100GBase_CR4_GET(*modesVectorPtr) || AP_CTRL_25GBase_CR_CONSORTIUM_GET(*modesVectorPtr) || AP_CTRL_50GBase_CR2_CONSORTIUM_GET(*modesVectorPtr) || AP_CTRL_40GBase_CR4_GET(*modesVectorPtr) ||
            AP_CTRL_25GBase_CR1S_GET(*modesVectorPtr) || AP_CTRL_25GBase_CR1_GET(*modesVectorPtr)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
        CPSS_LOG_INFORMATION_MAC("Calling: hwsPortModeParamsGetToBuffer(devNum[%d], portGroup[%d], portNum[%d], portMode[%d], portParams)", CAST_SW_DEVNUM(devNum), 0, localPort, portMode);
        rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum),0,localPort,portMode,&tmpPortParams);
        if(GT_OK != rc)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return GT_BAD_PARAM");
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if((portParams->numOfActLanes < tmpPortParams.numOfActLanes))
        {
            hwsOsMemCopyFuncPtr(portParams, &tmpPortParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
        }
    }

    if (0 == *modesVectorPtr)
    {
        CPSS_LOG_INFORMATION_MAC("Advertisement modes not defined");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *singleModePtr = (1 == i) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal internal_cpssPxPortApPortConfigSet function
* @endinternal
*
* @brief   Enable/disable AP process on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] apEnable                 - AP enable/disable on port
* @param[in] apParamsPtr              - (ptr to) AP parameters for port
*                                      (NULL - for CPSS defaults).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortApPortConfigSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     apEnable,
    IN  CPSS_PX_PORT_AP_PARAMS_STC  *apParamsPtr
)
{
    GT_STATUS                rc;        /* return code */
    GT_U32                   phyPortNum;/* port number in local core */
    MV_HWS_AP_CFG            apCfg;     /* AP parameters of port in HWS format */
    CPSS_PX_PORT_AP_PARAMS_STC  *localApParamsPtr;/* temporary pointer to
                                        AP configuration parameters structure */
    GT_BOOL                  singleMode = GT_FALSE; /* just one ifMode/speed pair adverised */
    GT_U8                    sdVecSizeMax;
    GT_U16                  *sdVectorMaxPtr;
    GT_U32                  i;
    PRV_CPSS_PX_PORT_SERDES_POLARITY_CONFIG_STC   polarityValues;
    GT_U32                  ctleBiasOverride;
    GT_U32                   ctleBiasValue;
    MV_HWS_PORT_INIT_PARAMS  portParamsBuffer;
    MV_HWS_PORT_STANDARD     portMode;
    MV_HWS_PORT_INIT_PARAMS  curPortParams;
    GT_U8                    sdVecSize;
    GT_U16                   *sdVectorPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, phyPortNum);

    AP_DBG_PRINT_MAC(("cpssPxPortApPortConfigSet:devNum=%d,portNum=%d,apEnable=%d mode %d speed %d fecAbilityArr %d fecRequesredArr %d\n",
                        devNum, portNum, apEnable, apParamsPtr->modesAdvertiseArr[0].ifMode, apParamsPtr->modesAdvertiseArr[0].speed,apParamsPtr->fecAbilityArr[0], apParamsPtr->fecRequestedArr[0]));

    if (PRV_CPSS_GE_PORT_GE_ONLY_E ==
                    PRV_CPSS_PX_PORT_TYPE_OPTIONS_MAC(devNum, phyPortNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(GT_FALSE == apEnable)
    {
        MV_HWS_AP_PORT_STATUS   apStatus;

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlStatusGet(devNum[%d], portGroup[%d], phyPortNum[%d] &apStatus)", devNum, 0, phyPortNum);
        cpssOsMemSet(&apStatus, 0, sizeof(MV_HWS_AP_PORT_STATUS));

        rc = mvHwsApPortCtrlStatusGet(CAST_SW_DEVNUM(devNum), 0, phyPortNum, &apStatus);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }

        if(apStatus.smState != 0)/* if AP enabled on port stop it as required */
        {
            PRV_CPSS_PX_PORT_STATE_STC  portStateStc; /* (dummy) current enable port state */
            rc = prvCpssPxPortStateDisableAndGet(CAST_SW_DEVNUM(devNum), portNum, &portStateStc);
            if(rc != GT_OK)
            {
                return rc;
            }

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlStop(devNum[%d], portGroup[%d], phyPortNum[%d])", devNum, 0, phyPortNum);
            rc = mvHwsApPortCtrlStop(CAST_SW_DEVNUM(devNum), 0, phyPortNum);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }
        }
        if(PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, phyPortNum) != CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            if(PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, phyPortNum) == CPSS_PORT_INTERFACE_MODE_NA_HCD_E)
            {
                PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, phyPortNum) = CPSS_PORT_INTERFACE_MODE_NA_E;
                PRV_CPSS_PX_PORT_SPEED_MAC(devNum, phyPortNum) = CPSS_PORT_SPEED_NA_E;

            } else {
                CPSS_PORTS_BMP_STC portsBmp;
                PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,portNum);

                rc = cpssPxPortModeSpeedSet(devNum, &portsBmp, GT_FALSE,
                                            PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, phyPortNum) + CPSS_PORT_INTERFACE_MODE_NA_E,
                                            PRV_CPSS_PX_PORT_SPEED_MAC(devNum,phyPortNum) + CPSS_PORT_SPEED_NA_E);
                if(rc != GT_OK)
                {
                    CPSS_LOG_INFORMATION_MAC("cpssPxPortModeSpeedSet:rc=%d,portNum=%d\n",rc, portNum);
                    return rc;
                }
            }
        }

        /* enable port MAC after it was disabled when AP enabled and
           could be left disabled if there was no link */
        rc = prvCpssPxPortEnableSet(devNum, portNum, GT_TRUE);
        if(rc!=GT_OK)
        {
            AP_DBG_PRINT_MAC(("prvCpssPxPortEnableSet:devNum=%d,portNum=%d,TRUE\n",
                                devNum, portNum));
        }
        return rc;
    }

    localApParamsPtr = (apParamsPtr != NULL) ? apParamsPtr :
                                                &prvCpssPxPortApDefaultParams;

    cpssOsMemSet(&apCfg, 0, sizeof(apCfg));
    sdVecSizeMax = 0;
    sdVectorMaxPtr = NULL;
    rc = prvCpssPxPortApModesVectorBuild(devNum, portNum,
                                              localApParamsPtr->modesAdvertiseArr,
                                              localApParamsPtr->fecAbilityArr,
                                              localApParamsPtr->fecRequestedArr,
                                              localApParamsPtr->fecSupported,
                                              localApParamsPtr->fecRequired,
                                              &portParamsBuffer,
                                              &apCfg,
                                              &singleMode);
    if(rc != GT_OK)
    {
        AP_DBG_PRINT_MAC(("ApModesVectorBuild:devNum=%d,portNum=%d failed rc=%d\n",
                            devNum, portNum, rc));
        return rc;
    }

    sdVecSizeMax    = portParamsBuffer.numOfActLanes;
    sdVectorMaxPtr  = portParamsBuffer.activeLanesList;

    if((localApParamsPtr->fcAsmDir != CPSS_PX_PORT_AP_FLOW_CONTROL_SYMMETRIC_E)
        && (localApParamsPtr->fcAsmDir != CPSS_PX_PORT_AP_FLOW_CONTROL_ASYMMETRIC_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    apCfg.fcAsmDir  = localApParamsPtr->fcAsmDir;
    apCfg.fcPause   = localApParamsPtr->fcPause;
    apCfg.nonceDis  = localApParamsPtr->noneceDisable;
    if(localApParamsPtr->laneNum > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (NULL == sdVectorMaxPtr)
    {/* added check here to prevent compiler warnings */
        AP_DBG_PRINT_MAC(("devNum=%d,portNum=%d:sdVectorMaxPtr==NULL\n",
                            devNum, portNum));
        return rc;
    }

    apCfg.apLaneNum = sdVectorMaxPtr[0] + localApParamsPtr->laneNum;

    /* set the Polarity values on Serdeses if SW DB values initialized - only for Caelum (Bobk) device and above */
    apCfg.polarityVector = 0;
    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        if(PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr != NULL)
        {
            for(i=0; i < sdVecSizeMax; i++)
            {
                polarityValues = PRV_CPSS_PX_PP_MAC(devNum)->serdesPolarityPtr[sdVectorMaxPtr[i]];

                if(polarityValues.txPolarity == GT_TRUE)
                {
                    apCfg.polarityVector |= (1 << (2*i+1));
                }

                if(polarityValues.rxPolarity == GT_TRUE)
                {
                    apCfg.polarityVector |= (1 << (2*i));
                }
            }
        }
    }

    apCfg.refClockCfg.refClockSource = PRIMARY_LINE_SRC;
    if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[phyPortNum].portRefClock.enableOverride == GT_TRUE)
    {
        switch (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[phyPortNum].portRefClock.portRefClockSource)
        {
            case CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E:
                apCfg.refClockCfg.refClockSource = PRIMARY_LINE_SRC;
                break;
            case CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E:
                apCfg.refClockCfg.refClockSource = SECONDARY_LINE_SRC;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum, apParamsPtr->modesAdvertiseArr[0].ifMode, apParamsPtr->modesAdvertiseArr[0].speed, &portMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, portMode, &curPortParams);
    if (GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    sdVecSize   = curPortParams.numOfActLanes;
    sdVectorPtr = curPortParams.activeLanesList;
    /* CPLL ref clock update */
    rc = prvCpssPxPortRefClockUpdate(devNum, portMode, sdVectorPtr, sdVecSize, apCfg.refClockCfg.refClockSource, &apCfg.refClockCfg.refClockFreq);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxSerdesRefClockTranslateCpss2Hws(devNum,
                                                 &apCfg.refClockCfg.refClockFreq);
    /*PIPE AP can not work with refClockFreq different than 156MHz - CPSS-11701*/
    apCfg.refClockCfg.refClockFreq = MHz_156;

    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssPxPortCtleBiasOverrideEnableGet(devNum, portNum, &ctleBiasOverride, (CPSS_PX_PORT_CTLE_BIAS_MODE_ENT*)&ctleBiasValue);
    {
        if (rc != GT_OK)
        {
            return rc;
        }
        else
        {
            /* if override enabled, update apCfg struct with the new value */
            if (ctleBiasOverride == 1)
            {
                apCfg.ctleBiasValue    = ctleBiasValue;
            }
            else
            {
                /* by default CTLE Value is zero */
                apCfg.ctleBiasValue    = 0;
            }
        }
    }

    if (singleMode)
    {/* if just single option (ifMode/speed pair) advertised resources can
        be allocated already */

        CPSS_PORTS_BMP_STC portsBmp;
        PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,portNum);

        rc = cpssPxPortModeSpeedSet(devNum, &portsBmp, GT_TRUE,
                                    localApParamsPtr->modesAdvertiseArr[0].ifMode + CPSS_PORT_INTERFACE_MODE_NA_E,
                                    localApParamsPtr->modesAdvertiseArr[0].speed + CPSS_PORT_SPEED_NA_E);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("cpssPxPortModeSpeedSet:rc=%d,portNum=%d\n",rc, portNum);
            return rc;
        }
    }
    else
    {
        PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, phyPortNum) = CPSS_PORT_INTERFACE_MODE_NA_HCD_E;
        PRV_CPSS_PX_PORT_SPEED_MAC(devNum, phyPortNum) = CPSS_PORT_SPEED_NA_HCD_E;
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlStart(devNum[%d], phyPortNum[%d], apCfg{apLaneNum[%d], modesVector[%d], fcPause[%d], fcAsmDir[%d], fecSup[%d], fecReq[%d], nonceDis[%d], refClockCfg{refClockFreq[%d], refClockSource[%d]}})",
                        devNum, phyPortNum, apCfg.apLaneNum,apCfg.modesVector,apCfg.fcPause,apCfg.fcAsmDir,apCfg.fecSup,apCfg.fecReq, apCfg.nonceDis, apCfg.refClockCfg.refClockFreq,apCfg.refClockCfg.refClockSource);
    AP_DBG_PRINT_MAC(("mvHwsApPortCtrlStart:devNum=%d,phyPortNum=%d,\
laneNum=%d,modesVector=0x%x,fcAsmDir=%d,fcPause=%d,fecReq=%d,fecSup=%d,nonceDis=%d,\
refClock=%d,refClockSource=%d,polarity=%d fecAdvanceReq=0x%x, fecAdvanceAbil=0x%x\n",
                      devNum, phyPortNum, apCfg.apLaneNum,
                      apCfg.modesVector,apCfg.fcAsmDir,apCfg.fcPause,apCfg.fecReq,
                      apCfg.fecSup, apCfg.nonceDis, apCfg.refClockCfg.refClockFreq,
                      apCfg.refClockCfg.refClockSource, apCfg.polarityVector, apCfg.fecAdvanceReq, apCfg.fecAdvanceAbil));
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        rc = mvHwsApPortCtrlStart(CAST_SW_DEVNUM(devNum), 0, phyPortNum, &apCfg);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
    }

    return rc;
}

/**
* @internal cpssPxPortApPortConfigSet function
* @endinternal
*
* @brief   Enable/disable AP process on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] apEnable                 - AP enable/disable on port
* @param[in] apParamsPtr              - (ptr to) AP parameters for port
*                                      (NULL - for CPSS defaults).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApPortConfigSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     apEnable,
    IN  CPSS_PX_PORT_AP_PARAMS_STC  *apParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApPortConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, apEnable, apParamsPtr));

    rc = internal_cpssPxPortApPortConfigSet(devNum, portNum, apEnable, apParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, apEnable, apParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortApPortConfigGet function
* @endinternal
*
* @brief   Get AP configuration of port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] apEnablePtr              - AP enable/disable on port
* @param[out] apParamsPtr              - (ptr to) AP parameters of port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortApPortConfigGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                     *apEnablePtr,
    OUT CPSS_PX_PORT_AP_PARAMS_STC  *apParamsPtr
)
{
    GT_STATUS       rc;         /* return code */
    MV_HWS_AP_CFG   apCfg;      /* AP configuration parameters */
    GT_U32          phyPortNum; /* port number in local core */
    GT_U32          i;          /* iterator */
    MV_HWS_AP_PORT_STATUS   apStatus; /* AP resolution on port */
    MV_HWS_PORT_INIT_PARAMS  curPortParams;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, phyPortNum);
    CPSS_NULL_PTR_CHECK_MAC(apEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(apParamsPtr);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsServiceCpuEnableGet(devNum[%d])", devNum);
    *apEnablePtr = mvHwsServiceCpuEnableGet(CAST_SW_DEVNUM(devNum));
    if(GT_FALSE == *apEnablePtr)
    {
        return GT_OK;
    }
    if (PRV_CPSS_GE_PORT_GE_ONLY_E ==
                    PRV_CPSS_PX_PORT_TYPE_OPTIONS_MAC(devNum, phyPortNum))
    {
        *apEnablePtr = GT_FALSE;
        return GT_OK;
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlStatusGet(devNum[%d], portGroup[%d], phyPortNum[%d] &apStatus)", devNum, 0, phyPortNum);
    cpssOsMemSet(&apStatus, 0, sizeof(MV_HWS_AP_PORT_STATUS));
    rc = mvHwsApPortCtrlStatusGet(CAST_SW_DEVNUM(devNum), 0, phyPortNum, &apStatus);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }
    *apEnablePtr = (0 == apStatus.smState) ? GT_FALSE : GT_TRUE;
    cpssOsMemSet(&apCfg, 0, sizeof(MV_HWS_AP_CFG));
    if(GT_TRUE == *apEnablePtr)/* if AP disabled on port no need to read garbage */
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlCfgGet(devNum[%d], portGroup[%d], phyPortNum[%d] *apCfg)", devNum, 0, phyPortNum);
        rc = mvHwsApPortCtrlCfgGet(CAST_SW_DEVNUM(devNum), 0, phyPortNum, &apCfg);
        AP_DBG_PRINT_MAC(("mvHwsApPortCtrlCfgGet:rc=%d,devNum=%d,phyPortNum=%d,\
apLaneNum=%d,modesVector=0x%x,fcAsmDir=%d,fcPause=%d,fecReq=%d,fecSup=%d,nonceDis=%d,\
refClockFreq=%d,refClockSource=%d, fecAdvanceReq=0x%x, fecAdvanceAbil=0x%x\n",
                          rc, devNum, phyPortNum, apCfg.apLaneNum,
                          apCfg.modesVector,apCfg.fcAsmDir,apCfg.fcPause,apCfg.fecReq,
                          apCfg.fecSup, apCfg.nonceDis, apCfg.refClockCfg.refClockFreq,
                          apCfg.refClockCfg.refClockSource, apCfg.fecAdvanceReq, apCfg.fecAdvanceAbil));
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
    }

    apParamsPtr->fcAsmDir = apCfg.fcAsmDir;
    apParamsPtr->fcPause = apCfg.fcPause;
    apParamsPtr->fecRequired = apCfg.fecReq;
    apParamsPtr->fecSupported = apCfg.fecSup;
    apParamsPtr->noneceDisable = apCfg.nonceDis;

    i = 0;
    if (apCfg.modesVector != 0)
    {
        MV_HWS_PORT_STANDARD    portMode;
        CPSS_PORT_FEC_MODE_ENT fecR = CPSS_PORT_FEC_MODE_LAST_E, fecA = CPSS_PORT_FEC_MODE_LAST_E;

        if(AP_CTRL_10GBase_KX4_GET(apCfg.modesVector))
        {
            apParamsPtr->fecRequestedArr[i] = (apParamsPtr->fecRequired)? CPSS_PORT_FEC_MODE_ENABLED_E: CPSS_PORT_FEC_MODE_DISABLED_E;
            apParamsPtr->fecAbilityArr[i] = (apParamsPtr->fecSupported)? CPSS_PORT_FEC_MODE_ENABLED_E: CPSS_PORT_FEC_MODE_DISABLED_E;
            apParamsPtr->modesAdvertiseArr[i].ifMode =
                CPSS_PORT_INTERFACE_MODE_XGMII_E;
            apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_10000_E;
        }

        if(AP_CTRL_1000Base_KX_GET(apCfg.modesVector))
        {
            apParamsPtr->fecRequestedArr[i] = (apParamsPtr->fecRequired)? CPSS_PORT_FEC_MODE_ENABLED_E: CPSS_PORT_FEC_MODE_DISABLED_E;
            apParamsPtr->fecAbilityArr[i] = (apParamsPtr->fecSupported)? CPSS_PORT_FEC_MODE_ENABLED_E: CPSS_PORT_FEC_MODE_DISABLED_E;
            apParamsPtr->modesAdvertiseArr[i].ifMode =
                CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
            apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_1000_E;
        }

        if(AP_CTRL_10GBase_KR_GET(apCfg.modesVector))
        {
            apParamsPtr->fecRequestedArr[i] = (apParamsPtr->fecRequired)? CPSS_PORT_FEC_MODE_ENABLED_E: CPSS_PORT_FEC_MODE_DISABLED_E;
            apParamsPtr->fecAbilityArr[i] = (apParamsPtr->fecSupported)? CPSS_PORT_FEC_MODE_ENABLED_E: CPSS_PORT_FEC_MODE_DISABLED_E;
            apParamsPtr->modesAdvertiseArr[i].ifMode =
                CPSS_PORT_INTERFACE_MODE_KR_E;
            apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_10000_E;
        }

        if(AP_CTRL_40GBase_KR4_GET(apCfg.modesVector))
        {
            apParamsPtr->fecRequestedArr[i] = (apParamsPtr->fecRequired)? CPSS_PORT_FEC_MODE_ENABLED_E: CPSS_PORT_FEC_MODE_DISABLED_E;
            apParamsPtr->fecAbilityArr[i] = (apParamsPtr->fecSupported)? CPSS_PORT_FEC_MODE_ENABLED_E: CPSS_PORT_FEC_MODE_DISABLED_E;
            apParamsPtr->modesAdvertiseArr[i].ifMode =
                CPSS_PORT_INTERFACE_MODE_KR4_E;
            apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_40000_E;
        }

        if(AP_CTRL_40GBase_CR4_GET(apCfg.modesVector))
        {
            apParamsPtr->fecRequestedArr[i] = (apParamsPtr->fecRequired)? CPSS_PORT_FEC_MODE_ENABLED_E: CPSS_PORT_FEC_MODE_DISABLED_E;
            apParamsPtr->fecAbilityArr[i] = (apParamsPtr->fecSupported)? CPSS_PORT_FEC_MODE_ENABLED_E: CPSS_PORT_FEC_MODE_DISABLED_E;
            apParamsPtr->modesAdvertiseArr[i].ifMode =
                CPSS_PORT_INTERFACE_MODE_CR4_E;
            apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_40000_E;
        }

        if(AP_CTRL_100GBase_KR4_GET(apCfg.modesVector))
        {
            apParamsPtr->fecRequestedArr[i] = CPSS_PORT_RS_FEC_MODE_ENABLED_E;
            apParamsPtr->fecAbilityArr[i] = CPSS_PORT_RS_FEC_MODE_ENABLED_E;
            apParamsPtr->modesAdvertiseArr[i].ifMode =
                CPSS_PORT_INTERFACE_MODE_KR4_E;
            apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_100G_E;
        }

        if(AP_CTRL_100GBase_CR4_GET(apCfg.modesVector))
        {
            apParamsPtr->fecRequestedArr[i] = CPSS_PORT_RS_FEC_MODE_ENABLED_E;
            apParamsPtr->fecAbilityArr[i] = CPSS_PORT_RS_FEC_MODE_ENABLED_E;
            apParamsPtr->modesAdvertiseArr[i].ifMode =
                CPSS_PORT_INTERFACE_MODE_CR4_E;
            apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_100G_E;
        }

        if(AP_CTRL_25GBase_KR1S_GET(apCfg.modesVector)|| AP_CTRL_25GBase_CR1S_GET(apCfg.modesVector))
        {
            GT_U32 fecAdv = apCfg.fecAdvanceReq >> FEC_ADVANCE_BASE_R_SHIFT;
            if (AP_ST_HCD_FEC_RES_FC == fecAdv)
            {
                apParamsPtr->fecRequestedArr[i] = CPSS_PORT_FEC_MODE_ENABLED_E;
            }
            else
            {
                apParamsPtr->fecRequestedArr[i] = CPSS_PORT_FEC_MODE_DISABLED_E;
            }
            if(AP_CTRL_25GBase_KR1S_GET(apCfg.modesVector))
            {
                apParamsPtr->modesAdvertiseArr[i].ifMode = CPSS_PORT_INTERFACE_MODE_KR_S_E;
            }
            else
            {
                apParamsPtr->modesAdvertiseArr[i].ifMode = CPSS_PORT_INTERFACE_MODE_CR_S_E;
            }
            apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_25000_E;
        }
        if(AP_CTRL_25GBase_KR1_GET(apCfg.modesVector) || AP_CTRL_25GBase_CR1_GET(apCfg.modesVector))
        {
            GT_U32 fecAdv = apCfg.fecAdvanceReq >> FEC_ADVANCE_BASE_R_SHIFT;
            if (AP_ST_HCD_FEC_RES_FC == fecAdv)
            {
                apParamsPtr->fecRequestedArr[i] = CPSS_PORT_FEC_MODE_ENABLED_E;
            }
            else if (AP_ST_HCD_FEC_RES_RS == fecAdv)
            {
                apParamsPtr->fecRequestedArr[i] = CPSS_PORT_RS_FEC_MODE_ENABLED_E;
            }
            else if ((AP_ST_HCD_FEC_RES_RS | AP_ST_HCD_FEC_RES_FC) == fecAdv)
            {
                apParamsPtr->fecRequestedArr[i] = CPSS_PORT_BOTH_FEC_MODE_ENABLED_E;
            }
            else
            {
                apParamsPtr->fecRequestedArr[i] = CPSS_PORT_FEC_MODE_DISABLED_E;
            }
            if(AP_CTRL_25GBase_KR1_GET(apCfg.modesVector))
            {
                apParamsPtr->modesAdvertiseArr[i].ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
            }
            else
            {
                apParamsPtr->modesAdvertiseArr[i].ifMode = CPSS_PORT_INTERFACE_MODE_CR_E;
            }
            apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_25000_E;
        }
        if(AP_CTRL_ADV_CONSORTIUM_GET(apCfg.modesVector))
        {
            switch ((apCfg.fecAdvanceAbil >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_MASK) {
            case AP_ST_HCD_FEC_RES_FC:
                fecA = CPSS_PORT_FEC_MODE_ENABLED_E;
                break;
            case AP_ST_HCD_FEC_RES_RS:
                fecA = CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                break;
            case AP_ST_HCD_FEC_RES_BOTH:
                fecA = CPSS_PORT_BOTH_FEC_MODE_ENABLED_E;
                break;
            case AP_ST_HCD_FEC_RES_NONE:
                fecA = CPSS_PORT_FEC_MODE_DISABLED_E;
                break;
            }
            switch ((apCfg.fecAdvanceReq >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_MASK) {
            case AP_ST_HCD_FEC_RES_FC:
                fecR = CPSS_PORT_FEC_MODE_ENABLED_E;
                break;
            case AP_ST_HCD_FEC_RES_RS:
                fecR = CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                break;
            case AP_ST_HCD_FEC_RES_BOTH:
                fecR = CPSS_PORT_BOTH_FEC_MODE_ENABLED_E;
                break;
            case AP_ST_HCD_FEC_RES_NONE:
                fecR = CPSS_PORT_FEC_MODE_DISABLED_E;
                break;
            }
            AP_DBG_PRINT_MAC(("cpssPxPortApPortConfigGet:CONSORTIUM, %d fecAbilityArr=%d fecRequestedArr=%d\n",i,
                            fecA, fecR));
            if(AP_CTRL_25GBase_KR_CONSORTIUM_GET(apCfg.modesVector))
            {
                apParamsPtr->modesAdvertiseArr[i].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR_C_E;
                apParamsPtr->fecAbilityArr[i] = fecA;
                apParamsPtr->fecRequestedArr[i] = fecR;
                apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_25000_E;
            }
            if(AP_CTRL_25GBase_CR_CONSORTIUM_GET(apCfg.modesVector))
            {
                apParamsPtr->modesAdvertiseArr[i].ifMode =
                    CPSS_PORT_INTERFACE_MODE_CR_C_E;
                apParamsPtr->fecAbilityArr[i] = fecA;
                apParamsPtr->fecRequestedArr[i] = fecR;
                apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_25000_E;
            }
            if(AP_CTRL_50GBase_KR2_CONSORTIUM_GET(apCfg.modesVector))
            {
                apParamsPtr->modesAdvertiseArr[i].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR2_C_E;
                apParamsPtr->fecAbilityArr[i] = fecA;
                apParamsPtr->fecRequestedArr[i] = fecR;
                apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_50000_E;
            }
            if(AP_CTRL_50GBase_CR2_CONSORTIUM_GET(apCfg.modesVector))
            {
                apParamsPtr->modesAdvertiseArr[i].ifMode =
                    CPSS_PORT_INTERFACE_MODE_CR2_C_E;
                apParamsPtr->fecAbilityArr[i] = fecA;
                apParamsPtr->fecRequestedArr[i] = fecR;
                apParamsPtr->modesAdvertiseArr[i++].speed = CPSS_PORT_SPEED_50000_E;
            }
        }
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,apParamsPtr->modesAdvertiseArr[i-1].ifMode,
                                                    apParamsPtr->modesAdvertiseArr[i-1].speed,
                                                    &portMode);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, phyPortNum, portMode, &curPortParams);
        if (GT_OK != rc)
        {
            cpssOsPrintf("error in hwsPortModeParamsGetToBuffer,rc = %d, portMacMap = %d\n", rc, phyPortNum);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        apParamsPtr->laneNum = apCfg.apLaneNum - curPortParams.activeLanesList[0];
    }
    else
    {/* HWS pass garbage in such case */
        cpssOsMemSet(apParamsPtr, 0, sizeof(CPSS_PX_PORT_AP_PARAMS_STC));
        apParamsPtr->fecRequestedArr[0] = CPSS_PORT_FEC_MODE_DISABLED_E;
        apParamsPtr->fecAbilityArr[0] = CPSS_PORT_FEC_MODE_DISABLED_E;
    }

    for(;i<CPSS_PX_PORT_AP_IF_ARRAY_SIZE_CNS;i++)
    {
        apParamsPtr->modesAdvertiseArr[i].ifMode =
            CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;
        apParamsPtr->modesAdvertiseArr[i].speed = CPSS_PORT_SPEED_NA_E;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortApPortConfigGet function
* @endinternal
*
* @brief   Get AP configuration of port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] apEnablePtr              - AP enable/disable on port
* @param[out] apParamsPtr              - (ptr to) AP parameters of port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApPortConfigGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                     *apEnablePtr,
    OUT CPSS_PX_PORT_AP_PARAMS_STC  *apParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApPortConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, apEnablePtr, apParamsPtr));

    rc = internal_cpssPxPortApPortConfigGet(devNum, portNum, apEnablePtr, apParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, apEnablePtr, apParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortApPortEnableGet function
* @endinternal
*
* @brief   Get if AP is enabled on a port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] apEnablePtr              - AP enable/disable on port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortApPortEnableGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                     *apEnablePtr
)
{
    GT_STATUS       rc;         /* return code */
    GT_U32          phyPortNum; /* port number in local core */
    MV_HWS_AP_PORT_STATUS   apStatus; /* AP resolution on port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, phyPortNum);
    CPSS_NULL_PTR_CHECK_MAC(apEnablePtr);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsServiceCpuEnableGet(devNum[%d])", devNum);
    *apEnablePtr = mvHwsServiceCpuEnableGet(CAST_SW_DEVNUM(devNum));
    if(GT_FALSE == *apEnablePtr)
    {
        return GT_OK;
    }
    if (PRV_CPSS_GE_PORT_GE_ONLY_E ==
                    PRV_CPSS_PX_PORT_TYPE_OPTIONS_MAC(devNum, phyPortNum))
    {
        *apEnablePtr = GT_FALSE;
        return GT_OK;
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlStatusGet(devNum[%d], portGroup[%d], phyPortNum[%d] &apStatus)", devNum, 0, phyPortNum);
    cpssOsMemSet(&apStatus, 0, sizeof(MV_HWS_AP_PORT_STATUS));
    rc = mvHwsApPortCtrlStatusGet(CAST_SW_DEVNUM(devNum), 0, phyPortNum, &apStatus);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }
    *apEnablePtr = (0 == apStatus.smState) ? GT_FALSE : GT_TRUE;

    return GT_OK;
}

/**
* @internal cpssPxPortApPortEnableGet function
* @endinternal
*
* @brief   Get if AP is enabled on a port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] apEnablePtr              - AP enable/disable on port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApPortEnableGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                     *apEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApPortEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, apEnablePtr));

    rc = internal_cpssPxPortApPortEnableGet(devNum, portNum, apEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, apEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortApPortStatusGet function
* @endinternal
*
* @brief   Get status of AP on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] apStatusPtr              - (ptr to) AP parameters for port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apStatusPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - AP engine not run
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortApPortStatusGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PORT_AP_STATUS_STC  *apStatusPtr
)
{
    GT_STATUS               rc; /* return status */
    GT_U32                  apPortNum;
    MV_HWS_AP_PORT_STATUS   apResult;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, apPortNum);
    CPSS_NULL_PTR_CHECK_MAC(apStatusPtr);

    if (PRV_CPSS_GE_PORT_GE_ONLY_E ==
                    PRV_CPSS_PX_PORT_TYPE_OPTIONS_MAC(devNum, apPortNum))
    {
        AP_DBG_PRINT_MAC(("GE_only port\n"));
        apStatusPtr->hcdFound = GT_FALSE;
        return GT_OK;
    }

    cpssOsMemSet(&apResult, 0, sizeof(MV_HWS_AP_PORT_STATUS));
    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlStatusGet(devNum[%d], portGroup[%d], apPortNum[%d], *apResult)", devNum, 0, apPortNum);
    rc = mvHwsApPortCtrlStatusGet(CAST_SW_DEVNUM(devNum), 0, apPortNum, &apResult);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    AP_DBG_PRINT_MAC(("cpssPxPortApPortStatusGet:mvHwsApPortStatusGet:apPortNum=%d,apResult:apLaneNum=%d,hcdFound=%d,hcdLinkStatus=%d,\
hcdFecEn=%d,hcdFcPause=%d,hcdFcAsmDir=%d,postApPortMode=%d,postApPortNum=%d,preApPortNum=%d\n",
                      apPortNum,
                      apResult.apLaneNum,
                      apResult.hcdResult.hcdFound,
                      apResult.hcdResult.hcdLinkStatus,
                      apResult.hcdResult.hcdFecEn,
                      apResult.hcdResult.hcdFcRxPauseEn,
                      apResult.hcdResult.hcdFcTxPauseEn,
                      apResult.postApPortMode,
                      apResult.postApPortNum,
                      apResult.preApPortNum));

    cpssOsMemSet(apStatusPtr, 0, sizeof(CPSS_PX_PORT_AP_STATUS_STC));
    if((apStatusPtr->hcdFound = apResult.hcdResult.hcdFound) != GT_TRUE)
    {/* nothing interesting any more */
        return GT_OK;
    }

    apStatusPtr->fecEnabled = apResult.hcdResult.hcdFecEn;

    switch(apResult.hcdResult.hcdFecType)
    {
        case AP_ST_HCD_FEC_RES_FC:
            apStatusPtr->fecType = CPSS_PORT_FEC_MODE_ENABLED_E;
            break;
        case AP_ST_HCD_FEC_RES_RS:
            apStatusPtr->fecType = CPSS_PORT_RS_FEC_MODE_ENABLED_E;
            break;
        case AP_ST_HCD_FEC_RES_NONE:
        default:
            apStatusPtr->fecType = CPSS_PORT_FEC_MODE_DISABLED_E;
            break;
    }

    switch(apResult.postApPortMode)
    {
        case _1000Base_X:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_1000_E;
            break;
        case _10GBase_KX4:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_XGMII_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_10000_E;
            break;
        case _10GBase_KR:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_10000_E;
            break;
        case _40GBase_KR4:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_KR4_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_40000_E;
            break;
        case _40GBase_CR4:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_CR4_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_40000_E;
            break;
        case _100GBase_KR4:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_KR4_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_100G_E;
            break;
        case _100GBase_CR4:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_CR4_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_100G_E;
            break;
        case _25GBase_KR:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_25000_E;
            break;
        case _25GBase_CR:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_CR_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_25000_E;
            break;
        case _25GBase_KR_S:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_KR_S_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_25000_E;
            break;
        case _25GBase_CR_S:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_CR_S_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_25000_E;
            break;
        case _25GBase_KR_C:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_KR_C_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_25000_E;
            break;
        case _25GBase_CR_C:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_CR_C_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_25000_E;
            break;
        case _50GBase_KR2_C:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_KR2_C_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_50000_E;
            break;
        case _50GBase_CR2_C:
            apStatusPtr->portMode.ifMode = CPSS_PORT_INTERFACE_MODE_CR2_C_E;
            apStatusPtr->portMode.speed = CPSS_PORT_SPEED_50000_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortPhysicalPortMapReverseMappingGet(devNum,
                            PRV_CPSS_PX_PORT_TYPE_MAC_E, apResult.postApPortNum,
                            &apStatusPtr->postApPortNum);
    if(rc != GT_OK)
    {
        AP_DBG_PRINT_MAC(("prvCpssPxPortPhysicalPortMapReverseMappingGet(devNum=%d,PRV_CPSS_PX_PORT_TYPE_MAC_E,apResult.postApPortNum=%d):rc=%d\n",
                  devNum, apResult.postApPortNum, rc));
        return rc;
    }

    apStatusPtr->fcTxPauseEn = apResult.hcdResult.hcdFcTxPauseEn;
    apStatusPtr->fcRxPauseEn = apResult.hcdResult.hcdFcRxPauseEn;

    AP_DBG_PRINT_MAC(("apStatusPtr:fcTxPauseEn=%d,fcRxPauseEn=%d,fecEnabled=%d,hcdFound=%d,ifMode=%d,speed=%d,postApPortNum=%d\n",
                      apStatusPtr->fcTxPauseEn,apStatusPtr->fcRxPauseEn,apStatusPtr->
                      fecEnabled,apStatusPtr->hcdFound,
                      apStatusPtr->portMode.ifMode,apStatusPtr->portMode.speed,
                      apStatusPtr->postApPortNum));

    return GT_OK;
}

/**
* @internal cpssPxPortApPortStatusGet function
* @endinternal
*
* @brief   Get status of AP on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] apStatusPtr              - (ptr to) AP parameters for port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apStatusPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - AP engine not run
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApPortStatusGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PORT_AP_STATUS_STC  *apStatusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApPortStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, apStatusPtr));

    rc = internal_cpssPxPortApPortStatusGet(devNum, portNum, apStatusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, apStatusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortApStatsGet function
* @endinternal
*
* @brief   Returns the AP port statistics information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] apStatsPtr               - (ptr to) AP statistics information
* @param[out] intropAbilityMaxIntervalPtr - (ptr to) Introp Ability Max Interval
*                                      parameter - will help to represent
*                                      number of failed HCD cycles
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortApStatsGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PORT_AP_STATS_STC   *apStatsPtr,
    OUT GT_U16                      *intropAbilityMaxIntervalPtr
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  phyPortNum; /* port number in local core */
    MV_HWS_AP_PORT_STATS    apStats;    /* AP statistics information in HWS format*/
    MV_HWS_AP_PORT_INTROP   apIntrop;  /* AP port introp information in HWS format */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,phyPortNum);
    CPSS_NULL_PTR_CHECK_MAC(apStatsPtr);
    CPSS_NULL_PTR_CHECK_MAC(intropAbilityMaxIntervalPtr);

    if (PRV_CPSS_GE_PORT_GE_ONLY_E == PRV_CPSS_PX_PORT_TYPE_OPTIONS_MAC(devNum, phyPortNum))
    {
        return GT_OK;
    }

    cpssOsMemSet(&apStats, 0, sizeof(MV_HWS_AP_PORT_STATS));
    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlStatsGet(devNum[%d], portGroup[%d], phyPortNum[%d])", devNum, 0, phyPortNum);
    rc = mvHwsApPortCtrlStatsGet(CAST_SW_DEVNUM(devNum), 0, phyPortNum, &apStats);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("mvHwsApPortCtrlStatsGet: Hws return code is %d", rc);
        return rc;
    }

    cpssOsMemSet(&apIntrop, 0, sizeof(MV_HWS_AP_PORT_INTROP));
    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlIntropGet(devNum[%d], portGroup[%d], phyPortNum[%d])", devNum, 0, phyPortNum);
    rc = mvHwsApPortCtrlIntropGet(CAST_SW_DEVNUM(devNum), 0, phyPortNum, &apIntrop);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("mvHwsApPortCtrlIntropGet:Hws return code is %d", rc);
        return rc;
    }

    *intropAbilityMaxIntervalPtr = apIntrop.abilityMaxInterval;

    apStatsPtr->txDisCnt = apStats.txDisCnt;
    apStatsPtr->abilityCnt = apStats.abilityCnt;
    apStatsPtr->abilitySuccessCnt = apStats.abilitySuccessCnt;
    apStatsPtr->linkFailCnt = apStats.linkFailCnt;
    apStatsPtr->linkSuccessCnt = apStats.linkSuccessCnt;
    apStatsPtr->hcdResoultionTime = apStats.hcdResoultionTime;
    apStatsPtr->linkUpTime = apStats.linkUpTime;

    return GT_OK;
}

/**
* @internal cpssPxPortApStatsGet function
* @endinternal
*
* @brief   Returns the AP port statistics information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] apStatsPtr               - (ptr to) AP statistics information
* @param[out] intropAbilityMaxIntervalPtr - (ptr to) Introp Ability Max Interval
*                                      parameter - will help to represent
*                                      number of failed HCD cycles
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApStatsGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PORT_AP_STATS_STC   *apStatsPtr,
    OUT GT_U16                      *intropAbilityMaxIntervalPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApStatsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, apStatsPtr));

    rc = internal_cpssPxPortApStatsGet(devNum, portNum, apStatsPtr, intropAbilityMaxIntervalPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, apStatsPtr, intropAbilityMaxIntervalPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortApStatsReset function
* @endinternal
*
* @brief   Reset AP port statistics information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success - port not in use by AP processor
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_INITIALIZED       - AP engine or library not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortApStatsReset
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS   rc;
    GT_U32      phyPortNum; /* number of port in local core */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, phyPortNum);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortStatsReset(devNum[%d], portGroup[%d], phyPortNum[%d])", devNum, 0, phyPortNum);
    rc = mvHwsApPortCtrlStatsReset(CAST_SW_DEVNUM(devNum), 0, phyPortNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal cpssPxPortApStatsReset function
* @endinternal
*
* @brief   Reset AP port statistics information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success - port not in use by AP processor
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_INITIALIZED       - AP engine or library not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortApStatsReset
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApStatsReset);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum));

    rc = internal_cpssPxPortApStatsReset(devNum, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortApIntropSet function
* @endinternal
*
* @brief   Set AP port introp information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] apIntropPtr              - (ptr to) AP introp parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - apIntropPtr is NULL
*/
static GT_STATUS internal_cpssPxPortApIntropSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_PORT_AP_INTROP_STC  *apIntropPtr
)
{
    GT_STATUS               rc;        /* return code */
    GT_U32                  phyPortNum;/* port number in local core */
    MV_HWS_AP_PORT_INTROP   apIntrop;  /* AP port introp information in HWS format */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,phyPortNum);
    CPSS_NULL_PTR_CHECK_MAC(apIntropPtr);

    apIntrop.attrBitMask = apIntropPtr->attrBitMask;
    apIntrop.txDisDuration = apIntropPtr->txDisDuration;
    apIntrop.abilityDuration = apIntropPtr->abilityDuration;
    apIntrop.abilityMaxInterval = apIntropPtr->abilityMaxInterval;
    apIntrop.abilityFailMaxInterval = apIntropPtr->abilityFailMaxInterval;
    apIntrop.apLinkDuration = apIntropPtr->apLinkDuration;
    apIntrop.apLinkMaxInterval = apIntropPtr->apLinkMaxInterval;
    apIntrop.pdLinkDuration = apIntropPtr->pdLinkDuration;
    apIntrop.pdLinkMaxInterval = apIntropPtr->pdLinkMaxInterval;

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlIntropSet(devNum[%d], portGroup[%d], phyPortNum[%d])", devNum, 0, phyPortNum);
    rc = mvHwsApPortCtrlIntropSet(CAST_SW_DEVNUM(devNum), 0, phyPortNum, &apIntrop);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal cpssPxPortApIntropSet function
* @endinternal
*
* @brief   Set AP port introp information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] apIntropPtr              - (ptr to) AP introp parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - apIntropPtr is NULL
*/
GT_STATUS cpssPxPortApIntropSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_PORT_AP_INTROP_STC  *apIntropPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApIntropSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, apIntropPtr));

    rc = internal_cpssPxPortApIntropSet(devNum, portNum, apIntropPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, apIntropPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortApIntropGet function
* @endinternal
*
* @brief   Returns AP port introp information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
*
* @param[out] apIntropPtr              - (ptr to) AP introp parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - apIntropPtr is NULL
*/
static GT_STATUS internal_cpssPxPortApIntropGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT  CPSS_PX_PORT_AP_INTROP_STC *apIntropPtr
)
{
    GT_STATUS               rc;        /* return code */
    GT_U32                  phyPortNum;/* port number in local core */
    MV_HWS_AP_PORT_INTROP   apIntrop;  /* AP port introp information in HWS format */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,phyPortNum);
    CPSS_NULL_PTR_CHECK_MAC(apIntropPtr);

    cpssOsMemSet(&apIntrop, 0, sizeof(MV_HWS_AP_PORT_INTROP));

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsApPortCtrlIntropGet(devNum[%d], portGroup[%d], phyPortNum[%d])", devNum, 0, phyPortNum);
    rc = mvHwsApPortCtrlIntropGet(CAST_SW_DEVNUM(devNum),0,phyPortNum,&apIntrop);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    apIntropPtr->attrBitMask = apIntrop.attrBitMask;
    apIntropPtr->txDisDuration = apIntrop.txDisDuration;
    apIntropPtr->abilityDuration = apIntrop.abilityDuration;
    apIntropPtr->abilityMaxInterval = apIntrop.abilityMaxInterval;
    apIntropPtr->abilityFailMaxInterval = apIntrop.abilityFailMaxInterval;
    apIntropPtr->apLinkDuration = apIntrop.apLinkDuration;
    apIntropPtr->apLinkMaxInterval = apIntrop.apLinkMaxInterval;
    apIntropPtr->pdLinkDuration = apIntrop.pdLinkDuration;
    apIntropPtr->pdLinkMaxInterval = apIntrop.pdLinkMaxInterval;

    return rc;
}

/**
* @internal cpssPxPortApIntropGet function
* @endinternal
*
* @brief   Returns AP port introp information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
*
* @param[out] apIntropPtr              - (ptr to) AP introp parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - apIntropPtr is NULL
*/
GT_STATUS cpssPxPortApIntropGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PORT_AP_INTROP_STC  *apIntropPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApIntropGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, apIntropPtr));

    rc = internal_cpssPxPortApIntropGet(devNum, portNum, apIntropPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, apIntropPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortApDebugInfoGet function
* @endinternal
*
* @brief   Print AP port real-time log information stored in system
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] output                   - enum indicate log output
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
static GT_STATUS internal_cpssPxPortApDebugInfoGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT              output
)
{

    GT_STATUS               rc;         /* return code */
    GT_U32                  phyPortNum; /* port number in local core */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if(portNum == AP_LOG_ALL_PORT_DUMP)
    {
        phyPortNum = portNum;
    }
    else
    {
        PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,phyPortNum);
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvApPortCtrlDebugInfoShow(apPortNum[%d], cpssLog[%d])", phyPortNum, output);
    rc = mvApPortCtrlDebugInfoShow(devNum, phyPortNum,output);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("mvApPortCtrlDebugInfoShow: Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal cpssPxPortApDebugInfoGet function
* @endinternal
*
* @brief   Print AP port real-time log information stored in system
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] output                   - enum indicate log output
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
GT_STATUS cpssPxPortApDebugInfoGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT              output
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApDebugInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, output));

    rc = internal_cpssPxPortApDebugInfoGet(devNum, portNum, output);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cpssLog));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortApPortEnableCtrlSet function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable after training
*         Host or Service CPU (default value service CPU).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] srvCpuEnable             - if TRUE Service CPU responsible to preform port enable after training,
*                                      if FALSE Host
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
static GT_STATUS internal_cpssPxPortApPortEnableCtrlSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         srvCpuEnable
)
{
    GT_STATUS               rc;         /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_LOG_INFORMATION_MAC("Calling: (devNum[%d])", devNum);
    rc = mvHwsApPortCtrlEnablePortCtrlSet(CAST_SW_DEVNUM(devNum),srvCpuEnable);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal cpssPxPortApPortEnableCtrlSet function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable after training
*         Host or Service CPU (default value service CPU)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] srvCpuEnable             - if TRUE Service CPU responsible to preform port enable after training,
*                                      if FALSE Host
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
GT_STATUS cpssPxPortApPortEnableCtrlSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         srvCpuEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApPortEnableCtrlSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srvCpuEnable));

    rc = internal_cpssPxPortApPortEnableCtrlSet(devNum, srvCpuEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srvCpuEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortApPortEnableCtrlGet function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable after training
*         Host or Service CPU (default value service CPU)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*                                      portNum       - number of physical port
*
* @param[out] srvCpuEnablePtr          - (ptr to) port enable parmeter - if TRUE Service CPU responsible to preform port
*                                      enable after training, if FALSE Host
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
static GT_STATUS internal_cpssPxPortApPortEnableCtrlGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *srvCpuEnablePtr
)
{
    GT_STATUS               rc;         /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_LOG_INFORMATION_MAC("Calling: (devNum[%d])", devNum);
    rc = mvHwsApPortCtrlEnablePortCtrlGet(CAST_SW_DEVNUM(devNum),srvCpuEnablePtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal cpssPxPortApPortEnableCtrlGet function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable after training
*         Host or Service CPU (default value service CPU)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] srvCpuEnablePtr          - (ptr to) port enable parmeter - if TRUE Service CPU responsible to preform port
*                                      enable after training, if FALSE Host
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
GT_STATUS cpssPxPortApPortEnableCtrlGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *srvCpuEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApPortEnableCtrlGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srvCpuEnablePtr));

    rc = internal_cpssPxPortApPortEnableCtrlGet(devNum, srvCpuEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srvCpuEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortApSerdesRxParametersManualSet function
* @endinternal
*
* @brief   Set serdes RX parameters . Those offsets
*         will take place after TRX training.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - serdes lane number to configure the offsets to
* @param[in] serdesLane               - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] rxOverrideParamsPtr      - (pointer to) parameters data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssPxPortApSerdesRxParametersManualSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   serdesLane,
    IN  CPSS_PORT_AP_SERDES_RX_CONFIG_STC *rxOverrideParamsPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      portGroup, phyPortNum;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE  rxCfg;

    /* sanity checks */
    PRV_CPSS_DEV_CHECK_MAC(devNum);

    CPSS_LOG_INFORMATION_MAC("Calling: (devNum[%d])", devNum);

    CPSS_NULL_PTR_CHECK_MAC(rxOverrideParamsPtr);

    if((rxOverrideParamsPtr->lowFrequency > 15)  ||
       (rxOverrideParamsPtr->highFrequency > 15) ||
       (rxOverrideParamsPtr->bandWidth > 15) ||
       (rxOverrideParamsPtr->squelch > 308)||
       ((rxOverrideParamsPtr->serdesSpeed != CPSS_PORT_SERDES_SPEED_25_78125_E) && (rxOverrideParamsPtr->serdesSpeed != CPSS_PORT_SERDES_SPEED_10_3125_E)) ||
       (rxOverrideParamsPtr->etlMinDelay > 31) ||
       (rxOverrideParamsPtr->etlMaxDelay > 31))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    /* lane index check */
    if (serdesLane >= PRV_CPSS_MAX_PORT_LANES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"lane index %d not belong to port ", serdesLane);

    }
    /* getting mac number */
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                 portNum,
                                                 phyPortNum);

    /* getting port group number */
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, (phyPortNum+serdesLane));

    rxCfg.ctleParams.dcGain         = rxOverrideParamsPtr->dcGain;
    rxCfg.ctleParams.lowFrequency   = rxOverrideParamsPtr->lowFrequency;
    rxCfg.ctleParams.highFrequency  = rxOverrideParamsPtr->highFrequency;
    rxCfg.ctleParams.bandWidth      = rxOverrideParamsPtr->bandWidth;
    rxCfg.ctleParams.squelch        = rxOverrideParamsPtr->squelch;
    rxCfg.fieldOverrideBmp          = rxOverrideParamsPtr->fieldOverrideBmp;
    rxCfg.serdesSpeed               = (rxOverrideParamsPtr->serdesSpeed == CPSS_PORT_SERDES_SPEED_10_3125_E)? _10_3125G: _25_78125G;
    rxCfg.etlParams.etlMinDelay     = rxOverrideParamsPtr->etlMinDelay;
    rxCfg.etlParams.etlMaxDelay     = rxOverrideParamsPtr->etlMaxDelay;
    rxCfg.etlParams.etlEnableOverride = (GT_U8)rxOverrideParamsPtr->etlEnable;
    /* passing rx params to AP */
    rc = mvHwsApSerdesRxParametersManualSet(devNum, portGroup, (phyPortNum+serdesLane), &rxCfg);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsApSerdesRxParametersManualSet- Hws return code is %d", rc);
    }

    return GT_OK;

}


/**
* @internal cpssPxPortApSerdesRxParametersManualSet function
* @endinternal
*
* @brief   Set serdes RX parameters . Those offsets
*         will take place after TRX training.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - serdes lane number to configure the offsets to
* @param[in] serdesLane               - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] rxOverrideParamsPtr      - (pointer to) parameters data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxPortApSerdesRxParametersManualSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   serdesLane,
    IN  CPSS_PORT_AP_SERDES_RX_CONFIG_STC *rxOverrideParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApSerdesRxParametersManualSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesLane, rxOverrideParamsPtr));

    rc = internal_cpssPxPortApSerdesRxParametersManualSet(devNum, portNum, serdesLane, rxOverrideParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesLane, rxOverrideParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortApPortConfigSetConvert function
* @endinternal
*
* @brief   Enable/disable AP process on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
* @param[in] apEnable                 - AP enable/disable on port
* @param[in] apParamsPtr              - (ptr to) AP parameters for port
*                                      (NULL - for CPSS defaults).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For now allowed negotiation on serdes lanes of port 0-3, because in Lion2
*       just these options are theoreticaly possible.
*
*/
GT_STATUS prvCpssPxPortApPortConfigSetConvert
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    IN  GT_BOOL                    apEnable,
    IN  CPSS_PORT_AP_PARAMS_STC    *apParamsPtr,
    IN  GT_U32                     portOperationBitmap,
    IN  GT_BOOL                    skipRes
)
{
    portOperationBitmap = portOperationBitmap;
    skipRes = skipRes;
    return cpssPxPortApPortConfigSet(devNum,portNum,apEnable,(CPSS_PX_PORT_AP_PARAMS_STC*)apParamsPtr);
}

/**
* @internal prvCpssPxPortApPortStatusGetConvert function
* @endinternal
*
* @brief   Get status of AP on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] apStatusPtr              - (ptr to) AP parameters for port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apStatusPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - AP engine not run
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortApPortStatusGetConvert
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    OUT CPSS_PORT_AP_STATUS_STC   *apStatusPtr
)
{
    return cpssPxPortApPortStatusGet(devNum,portNum, (CPSS_PX_PORT_AP_STATUS_STC*)apStatusPtr);
}

/**
* @internal internal_cpssPXPortApSerdesTxParametersOffsetSet
*           function
* @endinternal
*
* @brief   Set serdes TX parameters offsets (negative of positive). Those offsets
*         will take place after resolution and prior to running TRX training.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - serdes lane number to configure the offsets to
* @param[in] serdesLane               - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
* @param[in] serdesTxOffsetsPtr       - (pointer to) parameters data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Offsets are limited to the range of minimum -15 and maximum +15 step size(2)
*
*/
static GT_STATUS internal_cpssPxPortApSerdesTxParametersOffsetSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   serdesLane,
    IN  CPSS_PORT_AP_SERDES_TX_OFFSETS_STC *serdesTxOffsetsPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U16      offsetsPacked;
    GT_8        txAmpOffset, txEmph0Offset, txEmph1Offset, reqInit = 0;
    MV_HWS_PORT_STANDARD portMode;
    GT_U32      phyPortNum, portGroup;
    MV_HWS_PORT_INIT_PARAMS curPortParamsPtr;

    /* sanity checks */
    PRV_CPSS_DEV_CHECK_MAC(devNum);

    CPSS_LOG_INFORMATION_MAC("Calling: (devNum[%d])", devNum);

    CPSS_NULL_PTR_CHECK_MAC(serdesTxOffsetsPtr);

    /* getting mac number */
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, phyPortNum);

    /* getting port group number */
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, phyPortNum);

    /* fectching values from staructure */
    txAmpOffset = serdesTxOffsetsPtr->txAmpOffset;
    txEmph0Offset = serdesTxOffsetsPtr->txEmph0Offset;
    txEmph1Offset = serdesTxOffsetsPtr->txEmph1Offset;
    if(serdesTxOffsetsPtr->reqInit == GT_TRUE)
    {
        reqInit = 1;
    }

    /* range check */
    if (txAmpOffset>15 || txAmpOffset<-15 ||
        txEmph0Offset>15 || txEmph0Offset<-15 ||
        txEmph1Offset>15 || txEmph1Offset<-15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                "offsets range is -15 and +15. given ranges: txAmpOffset=%d, txEmph0Offset=%d, txEmph1Offset=%d",
                                      txAmpOffset, txEmph0Offset,txEmph1Offset);
    }

    /* translate cpss ifMode and speed to single Hws port mode */
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,serdesTxOffsetsPtr->ifMode,
                                            serdesTxOffsetsPtr->speed, &portMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssCommonPortIfModeToHwsTranslate returned rc %d",rc );
    }

    /* get hws parameters for a given Hws port mode */

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParamsPtr))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "hwsPortModeParamsGetToBuffer returned null " );
    }

    /* lane index check */
    if (serdesLane >= curPortParamsPtr.numOfActLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"lane index %d not belong to port ", serdesLane);
    }

    /* packing values */
    offsetsPacked =
         ( ( (txAmpOffset<0) ? (((txAmpOffset*-1) & 0xF) | (0x10)) : (txAmpOffset & 0xF))
       |
         ((txEmph0Offset<0) ? ((((txEmph0Offset*-1) & 0xF) <<5) | (0x10<<5)) : ((txEmph0Offset & 0xF)<<5))
       |
         ((txEmph1Offset<0) ? ((((txEmph1Offset*-1) & 0xF) <<10) | (0x10<<10)) : ((txEmph1Offset & 0xF)<<10))
       |
         ((reqInit & 0x1) << 15));

    CPSS_LOG_INFORMATION_MAC("given values: txAmpOffset=%d, txEmph0Offset=%d, txEmph1Offset=%d",
                                            txAmpOffset, txEmph0Offset,txEmph1Offset);
    CPSS_LOG_INFORMATION_MAC("offsetsPacked=0x%08x", offsetsPacked);

    /* passing packed ofssets with the appropriated hws port mode to AP */
    rc = mvHwsApSerdesTxParametersOffsetSet(devNum, portGroup, phyPortNum,
                                curPortParamsPtr.activeLanesList[serdesLane], offsetsPacked, curPortParamsPtr.serdesSpeed);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsApSerdesTxParametersOffsetSet- Hws return code is %d", rc);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortApSerdesTxParametersOffsetSet function
* @endinternal
*
* @brief   Set serdes TX parameters offsets (negative of positive). Those offsets
*         will take place after resolution and prior to running TRX training.
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - physical device number
*                                      serdesNumber  - serdes lane number to configure the offsets to
* @param[in] serdesTxOffsetsPtr       - (pointer to) parameters data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Offsets are limited to the range of minimum -15 and maximum +15 step size(2)
*
*/
GT_STATUS cpssPxPortApSerdesTxParametersOffsetSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   serdesLane,
    IN  CPSS_PORT_AP_SERDES_TX_OFFSETS_STC *serdesTxOffsetsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortApSerdesTxParametersOffsetSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, serdesLane, serdesTxOffsetsPtr));

    rc = internal_cpssPxPortApSerdesTxParametersOffsetSet(devNum, portNum, serdesLane, serdesTxOffsetsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, serdesLane, serdesTxOffsetsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

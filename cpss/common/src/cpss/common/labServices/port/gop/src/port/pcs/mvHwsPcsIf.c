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
* @file mvHwsPcsIf.c
*
* @brief PCS level APIs
*
* @version   22
********************************************************************************
*/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>

#ifdef __GNUC__
/* Test for GCC > 4.4 */
# if  (__GNUC__*100+__GNUC_MINOR__) >=404
#  define __NOOPT__ __attribute__((optimize("O0")))
# endif
#endif

#ifdef HWS_DEBUG
extern GT_FILEP fdHws;
#endif

#ifdef MV_HWS_REDUCED_BUILD
static MV_HWS_PCS_FUNC_PTRS *hwsPcsFuncsPtr[HWS_MAX_DEVICE_NUM][LAST_PCS] = {{NULL}};
#define PRV_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(_var) _var
#define PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(_var) _var


#else
#define PRV_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.pcsDir.mvHwsPcsIfSrc._var)

#define PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(_var)\
    PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.pcsDir.mvHwsPcsIfSrc._var)

#endif

#define MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(_devNum, _pcsType, _funcPtrName)                                    \
        if((NULL == PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[_devNum][_pcsType]) || (NULL == PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[_devNum][_pcsType]->_funcPtrName))        \
        {                                                                                                                   \
            CHECK_STATUS_EXT(GT_NOT_IMPLEMENTED, LOG_ARG_STRING_MAC("function is NULL"), LOG_ARG_GEN_PARAM_MAC(_pcsType));  \
        }
#ifndef RAVEN_DEV_SUPPORT

/**
* @internal mvHwsPcsSendFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start sending fault signals to partner, at both 10G and 40G.
*         On 10G the result will be local-fault on the sender and remote-fault on the receiver,
*         on 40G there will be local-fault on both sides, but there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - Pcs Number
* @param[in] pcsType                  - Pcs type
* @param[in] portMode                 - port mode
* @param[in] send                     - start/ stop send faults
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsSendFaultSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 send
)
{
    if ((pcsType == PCS_NA) || (pcsType >= LAST_PCS))
    {
      CHECK_STATUS_EXT(GT_BAD_PARAM, LOG_ARG_STRING_MAC("pcs type NA"), LOG_ARG_GEN_PARAM_MAC(pcsType));
    }

    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, pcsType, pcsSendFaultSetFunc)

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run PCS Send Fault\n",
      PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsTypeGetFunc());
    osFclose(fdHws);
#endif

    CHECK_STATUS_EXT(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsSendFaultSetFunc(devNum, portGroup, pcsNum, portMode, send),
                     LOG_ARG_STRING_MAC("pcs type"), LOG_ARG_GEN_PARAM_MAC(pcsType));

    return GT_OK;
}
#endif
/**
* @internal mvHwsPcsReset function
* @endinternal
*
* @brief   Set the selected PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] pcsType                  - PCS type
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsReset
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portMacNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    portPcsType = pcsType;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ARG_STRING_MAC("mvHwsPcsReset null hws param ptr"));
    }

    if ((pcsType == PCS_NA) || (pcsType >= LAST_PCS))
    {
      return GT_BAD_PARAM;
    }

    if ((PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType] == NULL) || (PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsResetFunc == NULL))
    {
        return GT_OK;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run PCS Reset: pcsType %s, action %d.(0 - for reset)\n",
      PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsTypeGetFunc(), action);
    osFclose(fdHws);
#endif
#ifndef ALDRIN_DEV_SUPPORT
    if(MMPCS == portPcsType)
    {
        /* default value in port elements DB is MMPCS (for NO-FEC and FC-FEC), for RS-FEC 25G/20GR1 should be updated */
        if ((HWS_25G_MODE_CHECK(portMode)) && (curPortParams.portFecMode == RS_FEC))
        {
            portPcsType = CGPCS;
        }
    }
#endif

    CHECK_STATUS_EXT(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][portPcsType]->pcsResetFunc(devNum, portGroup, portMacNum, portMode, action), LOG_ARG_STRING_MAC("serdes type"), LOG_ARG_GEN_PARAM_MAC(pcsType));
    return GT_OK;
}

/**
* @internal mvHwsPcsModeCfg function
* @endinternal
*
* @brief   Set the internal mux's to the required PCS in the PI.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] attributesPtr            - port attributes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsModeCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    portPcsType;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((curPortParams.portPcsType == PCS_NA) || (curPortParams.portPcsType >= LAST_PCS))
    {
      return GT_BAD_PARAM;
    }

    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, curPortParams.portPcsType, pcsModeCfgFunc)

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run PCS Mode Cfg: pcsType %s, number of lanes %d.\n",
      PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][curPortParams.portPcsType]->pcsTypeGetFunc(), curPortParams.numOfActLanes);
    osFclose(fdHws);
#endif

    portPcsType = curPortParams.portPcsType;
#ifndef ALDRIN_DEV_SUPPORT
    if(MMPCS == portPcsType)
    {
        /* default value in port elements DB is MMPCS (for NO-FEC and FC-FEC), for RS-FEC 25G/20GR1 should be updated */
        if((HWS_25G_MODE_CHECK(portMode)) && (curPortParams.portFecMode == RS_FEC))
        {
            portPcsType = CGPCS;
        }

    }
#endif
    CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][portPcsType]->pcsModeCfgFunc(devNum, portGroup, phyPortNum, portMode, attributesPtr));

    return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPcsLoopbackCfg function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] pcsType                  - PCS type
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsLoopbackCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_RESET pcsState = UNRESET;
    GT_U32       numOfLanes;
    GT_STATUS res;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    tmpPcsType;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((pcsType == PCS_NA) || (pcsType >= LAST_PCS))
    {
      return GT_BAD_PARAM;
    }
    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, pcsType, pcsLbCfgFunc)

    /* check if PCS us under reset or not */
    res = mvHwsPcsActiveStatusGet(devNum, portGroup, phyPortNum, portMode, &numOfLanes);
    if(res == GT_OK) /* check if this function is supported for this PCS */
    {
        pcsState = (numOfLanes == 0) ? RESET : UNRESET;
    }
    else if(res != GT_NOT_IMPLEMENTED)
    {
        return res;
    }

    /* rest PCS if it's not in reset*/
    if(pcsState == UNRESET)
    {
        CHECK_STATUS(mvHwsPcsReset (devNum,portGroup,phyPortNum,portMode,pcsType,RESET));
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run PCS Loopback Cfg: pcsType %s, type %d.\n",
      PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsTypeGetFunc(), lbType);
    osFclose(fdHws);
#endif
    tmpPcsType = pcsType;
    CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][tmpPcsType]->pcsLbCfgFunc(devNum, portGroup, phyPortNum, portMode, lbType));

    if(pcsState == UNRESET)
    {
        CHECK_STATUS(mvHwsPcsReset (devNum,portGroup,phyPortNum,portMode,pcsType,UNRESET));
    }

    return GT_OK;
}

/**
* @internal mvHwsPcsLoopbackCfgGet function
* @endinternal
*
* @brief   Return the PCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[out] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsLoopbackCfgGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_PORT_PCS_TYPE    portPcsType;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((curPortParams.portPcsType == PCS_NA) || (curPortParams.portPcsType >= LAST_PCS))
    {
      return GT_BAD_PARAM;
    }

    portPcsType = curPortParams.portPcsType;

    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, curPortParams.portPcsType, pcsLbCfgGetFunc)

    CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][portPcsType]->pcsLbCfgGetFunc(devNum, portGroup, phyPortNum, portMode, lbType));

    return GT_OK;
}

/**
 * @internal mvHwsPcsAutoNeg1GSgmii function
 * @endinternal
 *
 * @brief   Configure Auto-negotiation for SGMII/1000BaseX port
 *          modes.
 *
 *
 * @param devNum
 * @param phyPortNum
 * @param portMode
 * @param autoNegotiationPtr
 *
 * @return GT_STATUS
 */
GT_STATUS mvHwsPcsAutoNeg1GSgmii
(
    IN GT_U8                        devNum,
    IN GT_U32                       phyPortNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN MV_HWS_PCS_AUTONEG_1G_SGMII  *autoNegotiationPtr
)
{
    MV_HWS_PORT_PCS_TYPE    portPcsType;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((curPortParams.portPcsType == PCS_NA) || (curPortParams.portPcsType >= LAST_PCS))
    {
      return GT_BAD_PARAM;
    }

    portPcsType = curPortParams.portPcsType;

    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, curPortParams.portPcsType, pcsLbCfgGetFunc)

    CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][portPcsType]->pcsAutoNeg1GSgmiiFunc(devNum, phyPortNum, portMode, autoNegotiationPtr));

    return GT_OK;
}

/**
* @internal mvHwsPcsTestGenCfg function
* @endinternal
*
* @brief   Set PCS internal test generator mechanisms.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
*                                      pattern   - pattern to generate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsTestGenCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN MV_HWS_PORT_TEST_GEN_PATTERN     pattern
)
{
    if ((pcsType == PCS_NA) || (pcsType >= LAST_PCS))
    {
      return GT_BAD_PARAM;
    }
    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, pcsType, pcsTestGenFunc)

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run PCS Test Generator Cfg: pcsType %s, pattern is %d.\n",
      PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsTypeGetFunc(),pattern);
    osFclose(fdHws);
#endif
    CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsTestGenFunc(devNum, portGroup, pcsNum, pattern));

    return GT_OK;
}

/**
* @internal mvHwsPcsTestGenStatus function
* @endinternal
*
* @brief   Get PCS internal test generator mechanisms error counters and status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] pattern                  -  to generate
* @param[out] status                  -  status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsTestGenStatus
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN MV_HWS_PORT_TEST_GEN_PATTERN     pattern,
    OUT MV_HWS_TEST_GEN_STATUS        *status
)
{
    if ((pcsType == PCS_NA) || (pcsType >= LAST_PCS))
    {
      return GT_BAD_PARAM;
    }
    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, pcsType, pcsTestGenStatusFunc)

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run PCS Test Generator Status: pcsType %s, pattern is %d.\n",
      PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsTypeGetFunc(),pattern);
    osFclose(fdHws);
#endif
    CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsTestGenStatusFunc(devNum, portGroup, pcsNum, pattern, status));

    return GT_OK;
}

/**
* @internal mvHwsPcsRxReset function
* @endinternal
*
* @brief   Set the selected RX PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsRxReset
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN MV_HWS_RESET            action
)
{
    if ((pcsType == PCS_NA) || (pcsType >= LAST_PCS))
    {
      return GT_BAD_PARAM;
    }
    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, pcsType, pcsRxResetFunc)

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run RX PCS Reset: pcsType %s, action %d.(0 - for reset)\n",
      PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsTypeGetFunc(), action);
    osFclose(fdHws);
#endif

    CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsRxResetFunc(devNum, portGroup, pcsNum, action));
    return GT_OK;
}

/**
* @internal mvHwsPcsExtPllCfg function
* @endinternal
*
* @brief   Set the selected PCS type and number to external PLL mode.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsExtPllCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_PCS_TYPE    pcsType
)
{
    if (pcsType != MMPCS)
    {
        CHECK_STATUS_EXT(GT_BAD_PARAM, LOG_ARG_PCS_IDX_MAC(pcsNum));
    }
    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, pcsType, pcsExtPllCfgFunc)

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\nRun External PLL configuration: pcsType %s, pscNum %d\n",
      PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsTypeGetFunc(), pcsNum);
    osFclose(fdHws);
#endif

    CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsExtPllCfgFunc(devNum, portGroup, pcsNum));
    return GT_OK;
}

/**
* @internal mvHwsPcsSignalDetectMaskSet function
* @endinternal
*
* @brief   Set all related PCS with Signal Detect Mask value (1/0).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] maskEn                   - if true, enable signal detect mask bit
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsSignalDetectMaskSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN GT_BOOL                 maskEn
)
{
    if ((PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType] == NULL) || (PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsSignalDetectMaskEn == NULL))
    {
        return GT_OK;
    }

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\nRun Signal Detect Mask configuration: pcsType %s, pscNum %d\n",
      PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsTypeGetFunc(), pcsNum);
    osFclose(fdHws);
#endif

    CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsSignalDetectMaskEn(devNum, portGroup, pcsNum, maskEn));
    return GT_OK;
}

/**
* @internal mvHwsPcsFecConfig function
* @endinternal
*
* @brief   Configure FEC disable/enable on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pcsType                  - PCS type
* @param[in] portFecType              -
*       0-AP_ST_HCD_FEC_RES_NONE, 1-AP_ST_HCD_FEC_RES_FC,
*       2-AP_ST_HCD_FEC_RES_RS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsFecConfig
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  pcsNum,
    IN MV_HWS_PORT_PCS_TYPE    pcsType,
    IN MV_HWS_PORT_FEC_MODE    portFecType
)
{
    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, pcsType, pcsFecCfgFunc)

    CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsFecCfgFunc(devNum, portGroup,
                                                        pcsNum, portFecType));
    return GT_OK;
}

/**
* @internal mvHwsPcsFecConfigGet function
* @endinternal
*
* @brief   Return the FEC state on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] portFecTypePtr
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsFecConfigGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_FEC_MODE    *portFecTypePtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    pcsType;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsType = curPortParams.portPcsType;

    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, pcsType, pcsFecCfgGetFunc)

    CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsFecCfgGetFunc(devNum, portGroup,
                                                        phyPortNum, portMode, portFecTypePtr));
    return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsPcsActiveStatusGet function
* @endinternal
*
* @brief   Return number of PCS active lanes or 0, if current PCS isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsActiveStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_U32                  *numOfLanes
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    pcsType;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsType = curPortParams.portPcsType;

    /*  the macro generates 'errors' to the CPSS LOG ... but ignored by the CPSS/HWS internal CALLER !
        meaning that it should not generate the error to the LOG !
        MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, pcsType, pcsActiveStatusGetFunc)
    */

    if((PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]) && (PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsActiveStatusGetFunc))
        CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsActiveStatusGetFunc(devNum, portGroup, curPortParams.portPcsNumber, numOfLanes));

    return GT_OK;
}
#endif

/**
* @internal mvHwsPcsCheckGearBox function
* @endinternal
*
* @brief   Check Gear Box Status on related lanes (currently used only on MMPCS).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] numOfLanes               - number of lanes agregated in PCS
*
* @param[out] laneLock                 - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsCheckGearBox
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portMacNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *laneLock
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_PCS_TYPE    pcsType;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((curPortParams.portPcsType == PCS_NA) || (curPortParams.portPcsType >= LAST_PCS) || (curPortParams.numOfActLanes == 0))
    {
      return GT_BAD_PARAM;
    }

    pcsType = curPortParams.portPcsType;

#ifndef ALDRIN_DEV_SUPPORT
    if(MMPCS == pcsType)
    {
        /*the PCS active mode is MMPCS but PCS lock services for RS_FEC are supplied from CGPCS unit for 25G/20GR1 */
        if((HWS_25G_MODE_CHECK(portMode)) && (curPortParams.portFecMode == RS_FEC))
        {
            pcsType = CGPCS;
        }
        else
        /*the PCS active mode is MMPCS but PCS lock services are supplied from CGPCS unit for 50G/40GR2 */
        if(HWS_TWO_LANES_MODE_CHECK(portMode))
        {
            pcsType = CGPCS;
        }
    }
#endif

    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, pcsType, pcsCheckGearBoxFunc)

    CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsCheckGearBoxFunc(devNum, portGroup, portMacNum, portMode, laneLock));
    return GT_OK;
}
#endif

/**
* @internal mvHwsPcsAlignLockGet function
* @endinternal
*
* @brief   Read align status of PCS used by port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - MAC number
* @param[in] portMode                 - port mode
*
* @param[out] lock                     - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsAlignLockGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lock
)
{
    MV_HWS_PORT_PCS_TYPE    portPcsType;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((curPortParams.portPcsType == PCS_NA) || (curPortParams.portPcsType >= LAST_PCS))
    {
      return GT_BAD_PARAM;
    }

    portPcsType = curPortParams.portPcsType;

    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, portPcsType, pcsAlignLockGetFunc)

    CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][portPcsType]->pcsAlignLockGetFunc(devNum, portGroup, phyPortNum, portMode, lock));
   return GT_OK;
}

/**
* @internal mvHwsPcsPortLinkStatusGet function
* @endinternal
*
* @brief   Read link status of PCS used by port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - MAC number
* @param[in] portMode                 - port mode
*
* @param[out] linkStatus                - link Status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsPortLinkStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *linkStatus
)
{
    MV_HWS_PORT_PCS_TYPE    portPcsType;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((curPortParams.portPcsType == PCS_NA) || (curPortParams.portPcsType >= LAST_PCS))
    {
      return GT_BAD_PARAM;
    }

    portPcsType = curPortParams.portPcsType;

    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, portPcsType, pcsLinkStatusGetFunc)

    CHECK_STATUS(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][portPcsType]->pcsLinkStatusGetFunc(devNum, portGroup, curPortParams.portPcsNumber, linkStatus));

    return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPcsFastLinkDownSet function
* @endinternal
*
* @brief   Configure the PCS to Enable/Disable Fast link Down/Up.
*          This is done by sending fault signals to partner, the result will be local-fault on the
*          sender and remote-fault on the receiver.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - PCS number
* @param[in] pcsType                  - PCS type according to port configuration
* @param[in] portMode                 - Port Mode
* @param[in] enable                   - GT_TRUE for port enable, GT_FALSE otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPcsSendLocalFaultSet
(
    IN GT_U8                devNum,
    IN GT_U32               portGroup,
    IN GT_U32               pcsNum,
    IN MV_HWS_PORT_PCS_TYPE pcsType,
    IN MV_HWS_PORT_STANDARD portMode,
    IN GT_BOOL              enable
)
{
    /* Check for valid pcs type */
    if ((pcsType == PCS_NA) || (pcsType >= LAST_PCS))
    {
        return GT_BAD_PARAM;
    }
    MV_HWS_IS_PCS_FUNC_IMPLEMENTED_CHECK_AND_RETURN(devNum, pcsType, pcsSendLocalFaultSetFunc)

    CHECK_STATUS_EXT(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]->pcsSendLocalFaultSetFunc(devNum, portGroup, pcsNum, portMode, enable),
                     LOG_ARG_STRING_MAC("pcs type"), LOG_ARG_GEN_PARAM_MAC(pcsType));

    return GT_OK;
}
#endif

/**
* @internal hwsPcsGetFuncPtr function
* @endinternal
*
* @brief   Get function structure pointer.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPcsGetFuncPtr
(
    GT_U8                 devNum,
    MV_HWS_PCS_FUNC_PTRS  ***hwsFuncsPtr
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_NOT_SUPPORTED;
    }

    *hwsFuncsPtr = PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum];

    return GT_OK;
}

/**
* @internal hwsPcsIfClose function
* @endinternal
*
* @brief   Free all resources allocated by supported SerDes
*          types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPcsIfClose
(
    IN GT_U8                   devNum
)
{
    MV_HWS_PORT_PCS_TYPE pcsType;
    for (pcsType = (MV_HWS_PORT_PCS_TYPE)0 ; pcsType < LAST_PCS ; pcsType++)
    {
        if(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType])
        {
            hwsOsFreeFuncPtr(PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType]);
            PRV_NON_SHARED_PCS_DIR_PCS_IF_SRC_GLOBAL_VAR(hwsPcsFuncsPtr)[devNum][pcsType] = NULL;
        }
    }
    return GT_OK;
}

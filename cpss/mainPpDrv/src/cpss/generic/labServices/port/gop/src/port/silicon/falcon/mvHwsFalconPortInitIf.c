/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsFalconPortInitIf.c
*
* @brief
*
* @version   55
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/generic/labservices/port/gop/silicon/falcon/mvHwsFalconPortIf.h>
#include <cpss/generic/labservices/port/gop/silicon/falcon/mvHwsPortCalIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>
#include <cpss/common/labServices/port/gop/port/mac/d2dMac/mvHwsD2dMacIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
GT_BOOL hwsPpHwTraceFlag = GT_FALSE;
extern GT_BOOL hwsFalconZ2Mode;

#ifndef MICRO_INIT
GT_STATUS debugHwsPpHwTraceEnableSet(GT_BOOL enable)
{
    hwsPpHwTraceFlag = enable;
    return GT_OK;
}
#endif

#if (!defined MV_HWS_REDUCED_BUILD) || defined (MICRO_INIT)
extern GT_U32  hwsFalconD2dEagleChannelTxEnableBypass_calledFrom_portCreate;
extern GT_U32  hwsFalconD2dEagleChannelTxEnableBypass;


static GT_STATUS mvHwsFalconCm3SemOper
(
    GT_U8                       devNum,
    GT_U32                      portMacNum,
    GT_BOOL                     takeFree /*true - take,false - free*/
)
{
    GT_U8   chipIdx;

    if ( mvHwsMtipIsReducedPort(devNum, portMacNum) )
    {
         chipIdx = (GT_U8)(portMacNum - (hwsDeviceSpecInfo[devNum].portsNum - hwsDeviceSpecInfo[devNum].numOfTiles*4));
    }
    else
    {
         chipIdx = portMacNum/16;
    }
    if ( takeFree ) {
        mvHwsHWAccessLock(devNum, chipIdx, MV_SEMA_PM_CM3);

    }
    else
    {
        mvHwsHWAccessUnlock(devNum, chipIdx, MV_SEMA_PM_CM3);
    }
    return GT_OK;
}

/**
* @internal mvHwsFalconPortCommonInit function
* @endinternal
*
* @brief   Falcon port common part (regular and AP) init
* (without mac/psc and unreset - thet is done from firmware)
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] portMode           - Port Mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsFalconPortCommonInit
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  d2dIdx, d2dNumRaven, d2dNumEagle, channelEagle, channelRaven;

#ifndef MICRO_INIT
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
#endif

    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }
    if (HWS_PAM4_MODE_CHECK(portMode))
    {
        curPortParams.portFecMode = RS_FEC_544_514;
    }

    /* Init D2D */
    d2dIdx = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(devNum,phyPortNum);
    d2dNumRaven = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1);
    d2dNumEagle  = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0);
#if !defined (RAVEN_DEV_SUPPORT)
    channelEagle = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, phyPortNum);
#else
    channelEagle = portNum %8;
#endif

    /* CPU port channel update for Raven side */
    if(channelEagle == D2D_EAGLE_CPU_PORT_INDEX_CNS)
    {
        channelRaven = D2D_RAVEN_CPU_PORT_INDEX_CNS;
    }
    else
    {
        channelRaven = channelEagle;
    }
#ifndef MICRO_INIT
    CHECK_STATUS(cpssSystemRecoveryStateGet(&tempSystemRecovery_Info));
    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
#endif
    {
        if (hwsFalconNumOfRavens)
        {
            CHECK_STATUS(mvHwsD2dConfigChannel(devNum, d2dNumRaven, channelRaven, curPortParams.serdesSpeed, curPortParams.numOfActLanes));
            if (!hwsFalconZ2Mode)
            {
                CHECK_STATUS(mvHwsD2dConfigChannel(devNum, d2dNumEagle, channelEagle, curPortParams.serdesSpeed, curPortParams.numOfActLanes));
            }
        }
    }
    mvHwsFalconCm3SemOper(devNum,phyPortNum, GT_TRUE);
    CHECK_STATUS(mvHwsMtipExtSetLaneWidth(devNum, phyPortNum, portMode, curPortParams.serdes10BitStatus));
    CHECK_STATUS(mvHwsMtipExtLowJitterEnable(devNum, phyPortNum, portMode, GT_TRUE));
    CHECK_STATUS(mvHwsMtipExtPcsClockEnable(devNum, phyPortNum, portMode, GT_TRUE));
    CHECK_STATUS(mvHwsMtipExtMacClockEnable(devNum, phyPortNum, portMode, GT_TRUE));
    CHECK_STATUS(mvHwsMtipExtMacResetRelease(devNum, phyPortNum, portMode, GT_TRUE));
    CHECK_STATUS(mvHwsMtipExtSetChannelMode(devNum, phyPortNum, portMode, GT_TRUE));

    CHECK_STATUS(mvHwsMsdbConfigChannel(devNum, phyPortNum, portMode, GT_TRUE));
    CHECK_STATUS(mvHwsMpfSetPchMode(devNum, phyPortNum, portMode, GT_TRUE));
    mvHwsFalconCm3SemOper(devNum,phyPortNum, GT_FALSE);

    if(hwsFalconNumOfRavens)
    {
        CHECK_STATUS(hwsD2dMacChannelEnable(devNum, d2dNumRaven, channelRaven, GT_TRUE));
        if (!hwsFalconZ2Mode)
        {
            if (hwsFalconD2dEagleChannelTxEnableBypass)
            {
                /* debug mode to allow the 'test' to control this 'TxEnable' config */
                /* the function hwsD2dMacChannelEnable(...) will check
                   hwsFalconD2dEagleChannelTxEnableBypass_calledFrom_portCreate flag */
                hwsFalconD2dEagleChannelTxEnableBypass_calledFrom_portCreate = 1;
            }
            CHECK_STATUS(hwsD2dMacChannelEnable(devNum, d2dNumEagle, channelEagle, GT_TRUE));
            if (hwsFalconD2dEagleChannelTxEnableBypass)
            {
                /* debug mode to allow the 'test' to control this 'TxEnable' config */
                hwsFalconD2dEagleChannelTxEnableBypass_calledFrom_portCreate = 0;
            }
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsFalconPortInit function
* @endinternal
*
* @brief   Falcon port init
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] portMode           - Port Mode
* @param[in] portInitInParamPtr - Input parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsFalconPortInit
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_BOOL serdesInit = GT_TRUE;
    MV_HWS_REF_CLOCK_SUP_VAL  refClock;
    MV_HWS_REF_CLOCK_SOURCE   refClockSource;

#ifndef MICRO_INIT
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
#endif

    if (NULL == portInitInParamPtr)
    {
        return GT_BAD_PTR;
    }
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL)
    {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** port %d create mode %d ******\n", phyPortNum, portMode);
    }
#endif
    /* init all modules as in ap port (msdb,mpf,mtipExt,D2D) */
    CHECK_STATUS(mvHwsFalconPortCommonInit(devNum,portGroup,phyPortNum,portMode));
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }
    mvHwsFalconCm3SemOper(devNum,phyPortNum, GT_TRUE);
    CHECK_STATUS(mvHwsMtipExtFecClockEnable(devNum, phyPortNum, portMode, curPortParams.portFecMode, GT_TRUE));
    mvHwsFalconCm3SemOper(devNum,phyPortNum, GT_FALSE);

    CHECK_STATUS(mvHwsMtipExtFecTypeSet(devNum, portGroup, phyPortNum, portMode, curPortParams.portFecMode));

    refClock = portInitInParamPtr->refClock;
    refClockSource = portInitInParamPtr->refClockSource;
    if (serdesInit == GT_TRUE)
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasLanesNum = curPortParams.numOfActLanes;
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasMacNum  = curPortParams.portMacNumber;
        /* rebuild active lanes list according to current configuration (redundancy) */
        CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

#ifndef MICRO_INIT
        /* power up the serdes */
        CHECK_STATUS(cpssSystemRecoveryStateGet(&tempSystemRecovery_Info));
        if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
#endif
        {
            CHECK_STATUS(mvHwsPortSerdesPowerUp(devNum, portGroup, phyPortNum, portMode, refClock, refClockSource, &(curLanesList[0])));
        }

    }
    /* Configure MAC/PCS */
    CHECK_STATUS(mvHwsPortModeCfg(devNum, portGroup, phyPortNum, portMode, NULL));

    /* Un-Reset the port */
    CHECK_STATUS(mvHwsPortStartCfg(devNum, portGroup, phyPortNum, portMode));

    /* Un-Reset mtip */
    CHECK_STATUS(mvHwsMtipExtReset(devNum, phyPortNum, portMode, GT_FALSE));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL)
    {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("**********************\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsFalconPortApReset function
* @endinternal
*
* @brief   Clears the AP port mode and release all its resources
*         according to selected. Does not verify that the
*         selected mode/port number is valid at the core level
*         and actual terminated mode.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsFalconPortApReset
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                phyPortNum,
    IN MV_HWS_PORT_STANDARD  portMode
)
{
    GT_U32  d2dIdx, d2dNumRaven, d2dNumEagle, channelEagle, channelRaven;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** AP port %d delete mode %d ******\n", phyPortNum, portMode);
    }
#endif
    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

     /* Init D2D */
    d2dIdx = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(devNum,phyPortNum);
    d2dNumRaven = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1);
    d2dNumEagle  = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0);
#if !defined (RAVEN_DEV_SUPPORT)
    channelEagle = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, phyPortNum);
#else
    channelEagle = portNum %8;
#endif

    /* CPU port channel update for Raven side */
    if(channelEagle == D2D_EAGLE_CPU_PORT_INDEX_CNS)
    {
        channelRaven = D2D_RAVEN_CPU_PORT_INDEX_CNS;
    }
    else
    {
        channelRaven = channelEagle;
    }

    mvHwsFalconCm3SemOper(devNum,phyPortNum, GT_TRUE);
    CHECK_STATUS(mvHwsMtipExtFecTypeSet(devNum, portGroup,phyPortNum, portMode, FEC_NA));
    CHECK_STATUS(mvHwsMtipExtSetChannelMode(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMpfSetPchMode(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMsdbConfigChannel(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtMacResetRelease(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtMacClockEnable(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtSetLaneWidth(devNum, phyPortNum, portMode, _10BIT_OFF));
    CHECK_STATUS(mvHwsMtipExtLowJitterEnable(devNum, phyPortNum, portMode, GT_FALSE));
    mvHwsFalconCm3SemOper(devNum,phyPortNum, GT_FALSE);

    /* TODO - add support for power reduction
    CHECK_STATUS(mvHwsMtipExtFecClockEnable(devNum, phyPortNum, portMode, curPortParams.portFecMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtPcsClockEnable(devNum, phyPortNum, portMode, GT_FALSE));*/

    CHECK_STATUS(mvHwsD2dDisableChannel(devNum, d2dNumRaven, portMode, channelRaven));
    if (!hwsFalconZ2Mode)
    {
        CHECK_STATUS(mvHwsD2dDisableChannel(devNum, d2dNumEagle, portMode, channelEagle));
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("**********************\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsFalconPortReset function
* @endinternal
*
* @brief   Clears the port mode and release all its resources according to selected.
*         Does not verify that the selected mode/port number is valid at the core
*         level and actual terminated mode.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsFalconPortReset
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ACTION  action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32  d2dIdx, d2dNumRaven, d2dNumEagle, channelEagle, channelRaven;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** port %d delete mode %d ******\n", phyPortNum, portMode);
    }
#endif
    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
       return GT_BAD_PTR;
    }
     /* Init D2D */
    d2dIdx = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(devNum,phyPortNum);
    d2dNumRaven = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1);
    d2dNumEagle  = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0);
#if !defined (RAVEN_DEV_SUPPORT)
    channelEagle = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, phyPortNum);
#else
    channelEagle = portNum %8;
#endif

    /* CPU port channel update for Raven side */
    if(channelEagle == D2D_EAGLE_CPU_PORT_INDEX_CNS)
    {
        channelRaven = D2D_RAVEN_CPU_PORT_INDEX_CNS;
    }
    else
    {
        channelRaven = channelEagle;
    }
    mvHwsFalconCm3SemOper(devNum,phyPortNum, GT_TRUE);
    CHECK_STATUS(mvHwsMtipExtReset(devNum, phyPortNum, portMode, GT_TRUE));
    mvHwsFalconCm3SemOper(devNum,phyPortNum, GT_FALSE);

    CHECK_STATUS(mvHwsPortStopCfg(devNum, portGroup, phyPortNum, portMode, action, &(curLanesList[0]), POWER_DOWN, POWER_DOWN));
    mvHwsFalconCm3SemOper(devNum,phyPortNum, GT_TRUE);

    CHECK_STATUS(mvHwsMtipExtFecTypeSet(devNum, portGroup, phyPortNum, portMode, FEC_NA));
    CHECK_STATUS(mvHwsMtipExtSetChannelMode(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMpfSetPchMode(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMsdbConfigChannel(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtSetLaneWidth(devNum, phyPortNum, portMode, _10BIT_OFF));
    CHECK_STATUS(mvHwsMtipExtLowJitterEnable(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtMacResetRelease(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtMacClockEnable(devNum, phyPortNum, portMode, GT_FALSE));
    mvHwsFalconCm3SemOper(devNum,phyPortNum, GT_FALSE);

    /* TODO - add support for power reduction
    CHECK_STATUS(mvHwsMtipExtFecClockEnable(devNum, phyPortNum, portMode, curPortParams.portFecMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtPcsClockEnable(devNum, phyPortNum, portMode, GT_FALSE));*/

    if(hwsFalconNumOfRavens)
    {
        CHECK_STATUS(mvHwsD2dDisableChannel(devNum, d2dNumRaven, portMode, channelRaven));
        if (!hwsFalconZ2Mode)
        {
            CHECK_STATUS(mvHwsD2dDisableChannel(devNum, d2dNumEagle, portMode, channelEagle));
        }
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("**********************\n");
    }
#endif
    return GT_OK;
}
#endif


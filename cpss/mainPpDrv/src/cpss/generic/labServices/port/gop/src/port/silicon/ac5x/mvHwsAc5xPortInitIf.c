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
* @file mvHwsAc5xPortInitIf.c
*
* @brief
*
* @version   1
********************************************************************************
*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5x/mvHwsAc5xPortIf.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>

extern  GT_BOOL hwsPpHwTraceFlag;

#if (!defined MV_HWS_REDUCED_BUILD)

/**
* @internal mvHwsPhoenixPortCommonInit function
* @endinternal
*
* @brief   Falcon port common part (regular and AP) init
* (without mac/psc and unreset - thet is done from firmware)
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
GT_STATUS mvHwsPhoenixPortCommonInit
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroup,
    IN GT_U32                   phyPortNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    portInitInParamPtr = portInitInParamPtr;

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
#if 0
    CHECK_STATUS(mvHwsMifChannelEnable(devNum, phyPortNum, portMode, GT_TRUE, portInitInParamPtr->isPreemptionEnabled));
    CHECK_STATUS(mvHwsEthPortPowerUp(devNum, phyPortNum, portMode, portInitInParamPtr));
#endif
    return GT_OK;
}

/**
* @internal mvHwsPhoenixPortInit function
* @endinternal
*
* @brief   Phoenix port init
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
GT_STATUS mvHwsPhoenixPortInit
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

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    refClock = portInitInParamPtr->refClock;
    refClockSource = portInitInParamPtr->refClockSource;

    if (serdesInit == GT_TRUE)
    {
        /* rebuild active lanes list according to current configuration (redundancy) */
        CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
        /* power up the serdes */
        CHECK_STATUS(mvHwsPortSerdesPowerUp(devNum, portGroup, phyPortNum, portMode, refClock, refClockSource, &(curLanesList[0])));
    }

    /* init all modules as in ap port (msdb,mpf,mtipExt) */
    CHECK_STATUS(mvHwsPhoenixPortCommonInit(devNum,portGroup,phyPortNum,portMode,portInitInParamPtr));

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
* @internal mvHwsHawkPortApReset function
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
GT_STATUS mvHwsPhoenixPortApReset
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                phyPortNum,
    IN MV_HWS_PORT_STANDARD  portMode
)
{

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** AP port %d delete mode %d ******\n", phyPortNum, portMode);
    }
#endif
    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

#if 0
    CHECK_STATUS(mvHwsMtipExtFecTypeSet(devNum, portGroup,phyPortNum, portMode, FEC_NA));
    CHECK_STATUS(mvHwsMtipExtSetChannelMode(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMpfSetPchMode(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMsdbConfigChannel(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtMacResetRelease(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtMacClockEnable(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtSetLaneWidth(devNum, phyPortNum, portMode, _10BIT_OFF));
    CHECK_STATUS(mvHwsMtipExtLowJitterEnable(devNum, phyPortNum, portMode, GT_FALSE));

    /* TODO - add support for power reduction
    CHECK_STATUS(mvHwsMtipExtFecClockEnable(devNum, phyPortNum, portMode, curPortParams.portFecMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtPcsClockEnable(devNum, phyPortNum, portMode, GT_FALSE));*/
#endif

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("**********************\n");
    }
#endif
    return GT_OK;
}

#endif


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
* mvHwsD2dMacIf.c
*
* DESCRIPTION: Port extrenal interface
*
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 55 $
******************************************************************************/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#endif
#include <cpss/common/labServices/port/gop/port/mac/d2dMac/mvHwsD2dMacIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/d2dPcs/mvHwsD2dPcsIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/gw16/mvGw16If.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>

#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>

static char* mvHwsMacTypeGet(void)
{
  return "D2D_MAC";
}
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};

extern  GT_BOOL hwsPpHwTraceFlag;

/* debug mode to allow the 'test' to control the D2D of Eagle Side for 'TxEnable' config */
/* the function hwsD2dMacChannelEnable(...) will check
   hwsFalconD2dEagleChannelTxEnableBypass_calledFrom_portCreate flag */
GT_U32  hwsFalconD2dEagleChannelTxEnableBypass_calledFrom_portCreate = 0;
GT_U32  hwsFalconD2dEagleChannelTxEnableBypass                       = 0;
#ifndef MICRO_INIT
GT_STATUS   hwsFalconD2dEagleChannelTxEnableBypass_set(IN GT_U32    bypassEn)
{
    hwsFalconD2dEagleChannelTxEnableBypass = bypassEn;

    return GT_OK;
}
#endif

GT_STATUS hwsD2dMacChannelEnable
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  channel,
    GT_BOOL enable

)
{
    GT_U32 regAddr, regData;
    GT_U32 unitIndex, unitNum;
    GT_U32 regMask, baseAddr, chStart ,chEnd, cpuChannel = 0;
    GT_U32 d2dIndex;

#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of hwsD2dMacChannelEnable ******\n");
    }
#endif

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));

    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    regData = enable<<31;
    regMask = 0x80000000;
    if (channel != HWS_D2D_MAC_CHANNEL_ALL) {
        chStart = chEnd = channel;
    }
    else
    {
        chStart = 0;
        chEnd = 7;
        if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
            cpuChannel = 8;
        else
            cpuChannel = 16;
    }

    for (channel = chStart; channel <= chEnd; channel++) {
        /*enable TX */
        if(hwsFalconD2dEagleChannelTxEnableBypass_calledFrom_portCreate)
        {
            /* NOTE: hwsFalconD2dEagleChannelTxEnableBypass_calledFrom_portCreate
                is called from mvHwsFalconPortInit(...) and set by it only for
                D2D of eagle side.
            */

            /* do not change the 'Tx Enable' at the D2D of Eagle */
        }
        else
        {
            regAddr = baseAddr + channel * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_TX_CHANNEL;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
        }

        /*enable RX */
        regAddr = baseAddr + channel * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_CHANNEL;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    }

    if (cpuChannel) {
        /*enable TX */
        regAddr = baseAddr + cpuChannel * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_TX_CHANNEL;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

        /*enable RX */
        regAddr = baseAddr + cpuChannel * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_CHANNEL;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

    }

#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of hwsD2dMacChannelEnable ******\n");
    }
#endif
    return GT_OK;
}
#ifndef MICRO_INIT
GT_STATUS hwsD2dMacChannelTxEnableSet
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  channel,
    GT_BOOL enable
)
{
    GT_U32 regAddr, regData;
    GT_U32 unitIndex, unitNum;
    GT_U32 regMask, baseAddr;
    GT_U32 d2dIndex;

#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of hwsD2dMacChannelTxEnableSet ******\n");
    }
#endif

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));

    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    regMask = 0x80000000;
    regData =  enable == GT_TRUE ? regMask : 0;

    regAddr = baseAddr + channel * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_TX_CHANNEL;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of hwsD2dMacChannelTxEnableSet ******\n");
    }
#endif
    return GT_OK;
}

GT_STATUS hwsD2dMacChannelTxEnableGet
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  channel,
    GT_BOOL *enablePtr
)
{
    GT_U32 regAddr, regData;
    GT_U32 unitIndex, unitNum;
    GT_U32 regMask, baseAddr;
    GT_U32 d2dIndex;

#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of hwsD2dMacChannelEnableGet ******\n");
    }
#endif

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));

    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    regMask = 0x80000000;
    regAddr = baseAddr + channel * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_TX_CHANNEL;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    *enablePtr = regData ? GT_TRUE : GT_FALSE;

#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of hwsD2dMacChannelEnableGet ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal hwsD2dMacFrameTableConfigSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number 0..63
* @param[in] frameIndex               - frame table index 0..31
* @param[in] configPtr                - (pointer to) structure that defines frame generator table entry.
*/
GT_STATUS hwsD2dMacFrameTableConfigSet
(
    GT_U8                               devNum,
    GT_U32                              d2dNum,
    GT_U32                              frameIndex,
    MV_HWS_D2D_FRAME_TABLE_CONFIG_STC   *configPtr
)
{
    GT_U32 regAddr, regData;
    GT_U32 baseAddr = 0, unitNum, unitIndex;
    GT_U32 regMask;     /* register mask */
    GT_U32 d2dIndex;
    GT_U32 i;           /* loop iterator */

    if(configPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    /*
        FRM_REPEAT_CNT_0 - set number of times the frame is sent (set number + 1)
        IDLE_WRD_0 - set to 0
        FRM_CMD_0 -
                    0: Normal SOP / EOP marked frame
                    1: Missing SOP. Frame is transmitted without SOP.
                    2: Missing EOP. Frame is transmitted without EOP.
                    3: EOP_ERR. Frame is abort marked.
    */

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    regAddr = baseAddr + D2D_MAC_DBG_FRM_TBL_START_ADDR + 4*frameIndex;
    regData = ((configPtr->frameRepeatCnt - 1) << 16) | (configPtr->idleWord << 8) | configPtr->frameCommand;
    regMask = 0xFFFF07;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

    /*
        range [FrmLenMin; FrmLenMin + 2**FrmLenMax - 1].
    */
    regAddr = baseAddr + D2D_MAC_DBG_FRM_TBL_LEN_START_ADDR + 4*frameIndex;
    regData = (configPtr->frameLengthMin) | (configPtr->frameLengthMax << 16);
    regMask = 0xFFFFF;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

    /* frame payload configuration */
    regData = configPtr->framePayload;
    regMask = 0xFFFFFFFF;
    regAddr = baseAddr + D2D_MAC_DBG_FRM_TBL_PAYLOAD_START_ADDR + 4*frameIndex;

    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

    /* frame header configuration */
    regMask = 0xFFFFFFFF;
    for(i = 0; i < HWS_D2D_MAC_DBG_FRM_TBL_HEADER_LEN; i++)
    {
        regData = configPtr->frameHeader[i];
        regAddr = baseAddr + D2D_MAC_DBG_FRM_TBL_HEADER_START_ADDR + 4*HWS_D2D_MAC_DBG_FRM_TBL_HEADER_LEN*frameIndex + 4*i;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    }

    return GT_OK;
}


/**
* @internal hwsD2dMacFrameGenConfigSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number 0..63
* @param[in] enable                   - enable/disable frame generator
* @param[in] channelNum               - channel index
* @param[in] configPtr                - (pointer to) structure that defines frame generator
*/
GT_STATUS hwsD2dMacFrameGenConfigSet
(
    GT_U8                           devNum,
    GT_U32                          d2dNum,
    GT_BOOL                         enable,
    GT_U32                          channelNum,
    MV_HWS_D2D_FRAME_GEN_CONFIG_STC *configPtr
)
{
    GT_U32 regAddrCfg, regAddrEn, regData,txFrameRepeatCountRegValue;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 regMask;     /* register mask */
    GT_U32 d2dIndex;

    if(configPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    if(configPtr->txFrameRepeatCount == 0xFFFFFFFF)
    {
        /* continuous traffic */
        txFrameRepeatCountRegValue = 255;
    }
    else if(configPtr->txFrameRepeatCount <= 255)
    {
        txFrameRepeatCountRegValue = configPtr->txFrameRepeatCount - 1;
    }
    else
    {
        return GT_BAD_PARAM;
    }

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    regAddrCfg = baseAddr + D2D_MAC_DBG_FRM_GEN_CFG + HWS_D2D_MAC_CHANNEL_STEP * channelNum;
    regAddrEn = baseAddr + D2D_MAC_DBG_CFG_FRM_EN;

    if(enable == GT_FALSE)
    {
        /* disable TX_CH_ACTIVE */
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddrCfg, 0, 1, 0));

        /* disable the overall operation in DBG_CFG_FRM_EN */
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddrEn, 0, 1, 0));
        return GT_OK;
    }

    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddrEn, 0, 1, 1));

    /* DBG_FRM_GEN_CFG configuration */
    regData = (txFrameRepeatCountRegValue << 16) | (configPtr->txPayloadType << 13) | (configPtr->txFrameTableCount << 7) | (configPtr->txFrameTableStartPointer << 1) | 1;
    regMask = 0xFF7FFF;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrCfg, regData, regMask));

    return GT_OK;
}

/**
* @internal hwsD2dMacFrameCheckerConfigSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number 0..63
* @param[in] enable                   - enable/disable frame checker
* @param[in] channelNum               - channel index
* @param[in] configPtr                - (pointer to) structure that defines frame checker
*/
GT_STATUS hwsD2dMacFrameCheckerConfigSet
(
    GT_U8                           devNum,
    GT_U32                          d2dNum,
    GT_BOOL                         enable,
    GT_U32                          channelNum,
    MV_HWS_D2D_FRAME_CHECKER_CONFIG_STC *configPtr
)
{
    GT_U32 regAddrStart, regAddrEn, regAddrGood, regData;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 regMask;     /* register mask */
    GT_U32 d2dIndex;

    if(configPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    regAddrStart = baseAddr + D2D_MAC_DBG_FRM_CHECK_CFG_START_ADDR + HWS_D2D_MAC_CHANNEL_STEP * channelNum;
    regAddrEn = baseAddr + D2D_MAC_DBG_CFG_FRM_EN;
    regAddrGood = baseAddr + D2D_MAC_DBG_FRM_CNT_GOOD_START_ADDR + HWS_D2D_MAC_CHANNEL_STEP * channelNum;

    if(enable == GT_FALSE)
    {
        /* disable RX_CH_ACTIVE */
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddrStart, 0, 1, 0));

        /* disable the overall operation in DBG_CFG_FRM_EN */
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddrEn, 0, 1, 0));

        return GT_OK;
    }

    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddrEn, 0, 1, 1));

    /* DBG_FRM_CHECK_CFG configuration */
    regData = (configPtr->rxPayloadType << 5) | (configPtr->rxBadCntMask << 1) | 1;
    regMask = 0x7F;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrStart, regData, regMask));

    /* Read register to clear the counter */
    regMask = 0xFFFFFFFF;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddrGood, &regData, regMask));

    return GT_OK;
}

/**
* @internal hwsD2dMacFrameGenStatusGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number 0..63
* @param[in] channelNum               - channel index 0..16
* @param[out] frameGoodCnt            - (pointer to) good frames counter
* @param[out] frameBadCnt             - (pointer to) bad frames counter
*/
GT_STATUS hwsD2dMacFrameGenStatusGet
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  channelNum,
    GT_U32  *frameGoodCnt,
    GT_U32  *frameBadCnt
)
{
    GT_U32 regMask = 0xFFFFFFFF;    /* register mask */
    GT_U32 regAddrBad, regAddrGood;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dIndex;

    if((frameGoodCnt == NULL) || (frameBadCnt == NULL))
    {
        return GT_BAD_PTR;
    }

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    regAddrBad = baseAddr + D2D_MAC_DBG_FRM_CNT_BAD_START_ADDR + HWS_D2D_MAC_CHANNEL_STEP * channelNum;
    regAddrGood = baseAddr + D2D_MAC_DBG_FRM_CNT_GOOD_START_ADDR + HWS_D2D_MAC_CHANNEL_STEP * channelNum;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddrGood, frameGoodCnt, regMask));
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddrBad, frameBadCnt, regMask));

    return GT_OK;
}

/**
* @internal hwsD2dMacFrameGenSentStatusGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number 0..63
* @param[in] channelNum               - channel index
* @param[in] expectedPcktNum          - expected packet number
*/
GT_STATUS hwsD2dMacFrameGenSentStatusGet
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  channelNum,
    GT_U32  expectedPcktNum
)
{
    GT_U32 regMask = 0xFFFFFFFF;    /* register mask */
    GT_U32 regAddr, regData;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dIndex;

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    regAddr = baseAddr + D2D_MAC_DBG_FRM_GEN_CFG_STAT + 8 * channelNum;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    if(expectedPcktNum != regData)
    {
        hwsOsPrintf("transmitted packet num = %d\n", regData);
        return GT_BAD_STATE;
    }

    return GT_OK;
}
#endif

static GT_STATUS hwsD2dMacInit
(
    GT_U8 devNum
)
{
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regMask;
    GT_U32 d2dIndex;

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        for(d2dIndex = 0; d2dIndex < 2; d2dIndex++)
        {
            /* Write to: CFG_TX_FIFO_SEG_POINTER_0 */
            regAddr = PRV_HWS_D2D_MAC_REG_ADDR_CALC_MAC(d2dIndex, D2D_MAC_CFG_TX_FIFO_SEG_POINTER);
            regData = 0;
            regMask = 0x3F;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: CFG_TX_CHANNEL_0 - TX Enable */
            regAddr = PRV_HWS_D2D_MAC_REG_ADDR_CALC_MAC(d2dIndex, D2D_MAC_CFG_TX_CHANNEL);
            regData = 0x80000000;
            regMask = 0x80000000;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: CFG_RX_FIFO_SEG_POINTER_0 */
            regAddr = PRV_HWS_D2D_MAC_REG_ADDR_CALC_MAC(d2dIndex, D2D_MAC_CFG_RX_FIFO_SEG_POINTER);
            regData = 0;
            regMask = 0x3F;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: CFG_RX_CHANNEL_0 */
            regAddr = PRV_HWS_D2D_MAC_REG_ADDR_CALC_MAC(d2dIndex, D2D_MAC_CFG_RX_CHANNEL);
            regData = 0xA1020003;
            regMask = 0xFFFFFFFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: CFG_RX_TDM_SCHEDULE_0 - Enable Slot 0 */
            regAddr = PRV_HWS_D2D_MAC_REG_ADDR_CALC_MAC(d2dIndex, D2D_MAC_CFG_RX_TDM_SCHEDULE);
            regData = 0x80;
            regMask = 0x80;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
        }
    }
    return GT_OK;
}

/**
* @internal hwsD2dMacIfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS hwsD2dMacIfInit(GT_U8 devNum, MV_HWS_MAC_FUNC_PTRS **funcPtrArray)
{

    if ((hwsInitDone[devNum] == GT_FALSE) && (hwsDeviceSpecInfo[devNum].devType == Raven))
    {
        /* init sequences DB */
        CHECK_STATUS(hwsD2dMacInit(devNum));
    }
    if(!funcPtrArray[D2D_MAC])
    {
        funcPtrArray[D2D_MAC] = (MV_HWS_MAC_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_MAC_FUNC_PTRS));
        if(!funcPtrArray[D2D_MAC])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[D2D_MAC], 0, sizeof(MV_HWS_MAC_FUNC_PTRS));
    }

    funcPtrArray[D2D_MAC]->macTypeGetFunc = mvHwsMacTypeGet;

    hwsInitDone[devNum] = GT_TRUE;

    return GT_OK;
}

/**
* @internal hwsD2dMacIfClose function
* @endinternal
 *
* @param[in] devNum                   - system device number
*/
#ifndef MICRO_INIT
void hwsD2dMacIfClose
(
    GT_U8 devNum
)
{
    hwsInitDone[devNum] = GT_FALSE;
}
#endif

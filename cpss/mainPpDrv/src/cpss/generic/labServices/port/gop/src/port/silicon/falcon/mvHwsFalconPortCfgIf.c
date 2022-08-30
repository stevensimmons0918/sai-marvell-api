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
* mvHwsFalconPortCfgIf.c
*
* DESCRIPTION:
*           This file contains API for port configuartion and tuning parameters
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 48 $
******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/generic/labservices/port/gop/silicon/falcon/mvHwsFalconPortIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>
#ifndef MICRO_INIT
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#endif

extern GT_U32  hwsRavenDevBmp; /* used for falcon Z2 if we want to use only several ravens and not all */
extern GT_BOOL hwsFalconZ2Mode;
/**
* @internal mvHwsMsdbRegWaInit function
* @endinternal
*
* @brief  MSDB micsonfiguration fix
*
* @param[in] devNum                   - system device number
* @param[in] portNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMsdbRegWaInit
(
    GT_U8 devNum,
    GT_U32 portNum
)
{
    GT_U32 d2dIdx;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddrTx, regAddrRx;

    /*
        for 200G port on offset 4 it is needed to fix:
            MSDB_SEGMENTED_CHANNEL_TX_CONTROL
            MSDB_SEGMENTED_CHANNEL_RX_CONTROL

        On regular port create flow - D2D Eagle and D2D Raven are initialized first
        and start to send credits to MSDB. MSDB channel at this stage disabled, but
        can get data (and can not send), therefore default value should be fixed
        before port create.
    */
    if (portNum != 0xFFFFFFFF /*d2dNum != HWS_D2D_ALL*/)
    {
        return GT_NOT_IMPLEMENTED;
    }
    else
    {
        for (d2dIdx = 0; d2dIdx < hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS; d2dIdx++) {
            if (hwsRavenDevBmp & (1<<(d2dIdx/HWS_D2D_NUM_IN_RAVEN_CNS)))
            {
                CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MSDB_UNIT, (d2dIdx * 8), &baseAddr, &unitIndex, &unitNum ));
                regAddrTx = baseAddr + 1 * HWS_MSDB_CHANNEL_OFFSET + MSDB_SEGMENTED_CHANNEL_TX_CONTROL;
                regAddrRx = baseAddr + 1 * HWS_MSDB_CHANNEL_OFFSET + MSDB_SEGMENTED_CHANNEL_RX_CONTROL;
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrTx, 0x4, 0x3F));
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrRx, 0x4, 0x3F));
            }
        }
    }

    return GT_OK;
}


/**
* @internal mvHwsMsdbFcuEnable function
* @endinternal
*
* @brief  set fcu in D2D unit
*
* @param[in] devNum                   - system device number
* @param[in] portNum               - physical port number
* @param[in] enable                   -  / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMsdbFcuEnable
(
    GT_U8   devNum,
    GT_U32  portNum,
    GT_BOOL enable
)
{
    GT_U32 msdbUnit, d2dIdx;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask;

    regMask = 0x7001C;
    if (portNum != 0xFFFFFFFF /*d2dNum != HWS_D2D_ALL*/)
    {
        msdbUnit = (PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(devNum,portNum)) % 2;
        CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MSDB_UNIT, portNum, &baseAddr, &unitIndex, &unitNum ));
        regAddr = baseAddr + MSDB_FCU_CONTROL;
        if (enable)
            regData = ((msdbUnit+1) << 16) | ((msdbUnit+1) << 2);
        else
            regData = 0;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    }
    else
    {
        for (d2dIdx = 0; d2dIdx < hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS; d2dIdx++) {
            if (hwsRavenDevBmp & (1<<(d2dIdx/HWS_D2D_NUM_IN_RAVEN_CNS)))
            {
                msdbUnit = d2dIdx % 2;
                CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MSDB_UNIT, (d2dIdx * 8), &baseAddr, &unitIndex, &unitNum ));
                   /*FCU Control*/
                regAddr = baseAddr + MSDB_FCU_CONTROL;
                if (enable)
                    regData = ((msdbUnit+1) << 16) | ((msdbUnit+1) << 2);
                else
                    regData = 0;
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
            }
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsMpfTsuEnable function
* @endinternal
*
* @brief  set TSU in mpf unit
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical port number
* @param[in] portMode              - port mode
* @param[in] enable                -  / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*
* m_RAL.tsu_ip_units_reg_model[i].tsu_ip_units_RegFile.PTP_Tx_Pipe_Status_Delay.Tx_Pipe_Status_Delay.set(2);
* m_RAL.tsu_ip_units_reg_model[i].tsu_ip_units_RegFile.PTP_Tx_Pipe_Status_Delay.update(status);
* m_RAL.tsu_ip_units_reg_model[i].tsu_ip_units_RegFile.PTP_general_control.PTP_unit_enable.set(1);
* m_RAL.tsu_ip_units_reg_model[i].tsu_ip_units_RegFile.PTP_general_control.update(status);
*/

GT_STATUS mvHwsMpfTsuEnable
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    GT_U32 d2dIdx;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 controlRegAddr, regData, i, idx;

    if (enable)
        regData = 1;
    else
        regData = 0;

    if (portNum != 0xFFFFFFFF /*d2dNum != HWS_D2D_ALL*/)
    {

        if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_TSU_UNIT,( portNum & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
            if (portNum%8 == 0) { /*in seg mode port 0 is taken from mpf 8 */
                baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
            }
            else /*in seg mode port 4 is taken from mpf 9 */
            {
                baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
            }
        }
        else
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_TSU_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
        }

        controlRegAddr = baseAddr + TSU_CONTROL;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, controlRegAddr, (regData << 1), 0x2));
    }
    else
    {
        for (d2dIdx = 0; d2dIdx < hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS; d2dIdx++) {
            if (hwsRavenDevBmp & (1<<(d2dIdx/HWS_D2D_NUM_IN_RAVEN_CNS)))
            {
               /* msdbUnit = d2dIdx % 2;*/
               /* ravenIdx = d2dIdx / 2;*/
                idx = d2dIdx * 8;
                CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_TSU_UNIT, idx, &baseAddr, &unitIndex, &unitNum ));
                for (i = 0; i < 10; i++) {
                    controlRegAddr = baseAddr + i*HWS_MPF_CHANNEL_OFFSET + TSU_CONTROL;
                    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, controlRegAddr, (regData << 1), 0x2));
                }
            }
        }
    }
    return GT_OK;
}
/**
* @internal mvHwsMsdbConfigChannel function
* @endinternal
*
* @brief  configure MSDB unit with the port mode (single/multi)
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical port number
* @param[in] portMode              - port mode
* @param[in] enable                - enable / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMsdbConfigChannel
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    GT_U32 mpfChannel;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddrRx, regAddrTx, regData, regMask;
    GT_U32 regAddrFcuTx,regAddrFcuRx;
    GT_BOOL segmentedMode = GT_FALSE;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMsdbConfigChannel ******\n");
    }
#endif

    if (PRV_HWS_D2D_IS_REDUCE_PORT_MAC(devNum, portNum))
    {
        mpfChannel = 8;
    }
    else
    {
        mpfChannel = portNum % 8;
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MSDB_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode) == GT_FALSE)
    {
        regAddrTx = baseAddr + mpfChannel * HWS_MSDB_CHANNEL_OFFSET + MSDB_CHANNEL_TX_CONTROL;
        regAddrRx = baseAddr + mpfChannel * HWS_MSDB_CHANNEL_OFFSET + MSDB_CHANNEL_RX_CONTROL;
    }
    else
    {
        regAddrTx = baseAddr + (mpfChannel/4) * HWS_MSDB_CHANNEL_OFFSET + MSDB_SEGMENTED_CHANNEL_TX_CONTROL;
        regAddrRx = baseAddr + (mpfChannel/4) * HWS_MSDB_CHANNEL_OFFSET + MSDB_SEGMENTED_CHANNEL_RX_CONTROL;
        segmentedMode = GT_TRUE;
    }

    if (enable)
    {
        regMask = 0x3F;
        /* MPFS index of CPU port is 16 */
        regData = (PRV_HWS_D2D_IS_REDUCE_PORT_MAC(devNum, portNum) == GT_FALSE) ? (mpfChannel) : (mpfChannel << 1);

        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrTx, regData, regMask));
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrRx, regData, regMask));
    }

    regMask = 0x80000000;
    regData = enable<<31;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrTx, regData, regMask));
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrRx, regData, regMask));

    /*FCU Channel control*/
    regAddrFcuTx = baseAddr + mpfChannel * HWS_MSDB_CHANNEL_OFFSET + MSDB_FCU_CHANNEL_TX_CONTROL;
    regAddrFcuRx = baseAddr + mpfChannel * HWS_MSDB_CHANNEL_OFFSET + MSDB_FCU_CHANNEL_RX_CONTROL;

    if (enable)
    {
        regMask = 0x3F;
        if(GT_FALSE==segmentedMode)
        {
            regData = mpfChannel;
        }
        else
        {
            regData = (mpfChannel>>2);
        }
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrFcuTx, regData, regMask));
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrFcuRx, regData, regMask));
     }

    regMask = 0xC0000000;
    regData = enable<<31;
    regData|=(segmentedMode & enable)<<30;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrFcuTx, regData, regMask));
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddrFcuRx, regData, regMask));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMsdbConfigChannel ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsMpfSetPchMode function
* @endinternal
*
* @brief  Set mpf pch mode
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] pchMode         - pch mode
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMpfSetPchMode
(
    IN GT_U8                   devNum,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 pchMode
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMpfSetPchMode ******\n");
    }
#endif

    if (PRV_HWS_D2D_IS_REDUCE_PORT_MAC(devNum, portNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MTI_CPU_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
    }
    else
    {
        if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
        {

            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT,( portNum & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
            if (portNum%8 == 0)     /*in seg mode port 0 is taken from mpf 8 */
            {
                baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
            }
            else /*in seg mode port 4 is taken from mpf 9 */
            {
                baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
            }
        }
        else
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
        }
    }
    regAddr = baseAddr + MPFS_PAU_CONTROL;
    /* tx bits[10,9], rx bits[2,1] */
    regMask = (3 << 1)/*RX PCH Mode*/ | (3 << 9)/*TX PCH Mode*/;

    if ((pchMode == GT_FALSE) || (hwsFalconZ2Mode == GT_TRUE))
    {
        /* When Raven is used without Eagle - TX/RX PCH mode should be disabled */
        regData = 0;
    }
    else
    {
        regData = (2 << 1)/*RX PCH Mode*/ | (1  << 9)/*TX PCH Mode*/;
    }

    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

    /* EOP at link down - EOP is generate when a link failure is detected */
    regAddr = baseAddr + MPFS_SAU_CONTROL;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 10, 1, 1));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMpfSetPchMode ******\n");
    }
#endif
    return GT_OK;
}
#ifndef MICRO_INIT
/**
* @internal mvHwsMpfSetPauTxEnable function
* @endinternal
*
* @brief  Set PAU tx enable <pautx_enable>
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] pauTxEnable     - Enable/Disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMpfSetPauTxEnable
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 pauTxEnable
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMpfSetPauTxEnable ******\n");
    }
#endif

    if (PRV_HWS_D2D_IS_REDUCE_PORT_MAC(devNum, portNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MTI_CPU_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
    }
    else
    {
        if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
        {

            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT,( portNum & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
            if (portNum%8 == 0)     /*in seg mode port 0 is taken from mpf 8 */
            {
                baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
            }
            else /*in seg mode port 4 is taken from mpf 9 */
            {
                baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
            }
        }
        else
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
        }
    }
    regAddr = baseAddr + MPFS_PAU_CONTROL;
    regMask = (1 << 8);   /* <pautx_enable> */

    if (pauTxEnable == GT_FALSE) {
        regData = 0;
    }
    else
    {
        regData = regMask;
    }

    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMpfSetPauTxEnable ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsMpfSetPauTxEnableGet function
* @endinternal
*
* @brief  Get PAU tx enable <pautx_enable>
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] pauTxEnablePtr - (pointer to) Enable/Disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMpfSetPauTxEnableGet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *pauTxEnablePtr
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMpfSetPauTxEnableGet ******\n");
    }
#endif

    if (PRV_HWS_D2D_IS_REDUCE_PORT_MAC(devNum, portNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MTI_CPU_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
    }
    else
    {
        if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
        {

            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT,( portNum & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
            if (portNum%8 == 0)     /*in seg mode port 0 is taken from mpf 8 */
            {
                baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
            }
            else /*in seg mode port 4 is taken from mpf 9 */
            {
                baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
            }
        }
        else
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
        }
    }
    regAddr = baseAddr + MPFS_PAU_CONTROL;
    regMask = (1 << 8);   /* <pautx_enable> */

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    if(regData & regMask)
    {
        *pauTxEnablePtr = GT_TRUE;
    }
    else
    {
        *pauTxEnablePtr = GT_FALSE;
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMpfSetPauTxEnableGet ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsMpfSauStatusGet function
* @endinternal
*
* @brief  Get MPFS SAU counter get - for specific port in the MPFS (0..7)
*                   if needed .. 'port 200/400' uses index 8/9
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] dropCntValuePtr - (pointer to drop counter)
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMpfSauStatusPortGet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *dropCntValuePtr
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask = 0xFFFFFFFF;

    if (PRV_HWS_D2D_IS_REDUCE_PORT_MAC(devNum, portNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MTI_CPU_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
    }
    else
    {
        if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
        {

            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT,( portNum & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
            if (portNum%8 == 0)     /*in seg mode port 0 is taken from mpf 8 */
            {
                baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
            }
            else /*in seg mode port 4 is taken from mpf 9 */
            {
                baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
            }
        }
        else
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
        }
    }

    regAddr = baseAddr + MPFS_SAU_STATUS;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    *dropCntValuePtr = regData;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMpfSauStatusPortGet ******\n");
    }
#endif

    return GT_OK;
}
/**
* @internal mvHwsMpfSauOpenDrainSet function
* @endinternal
*
* @brief   Set 'open drain' in MPFS for specific port
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] openDrain       - GT_TRUE  -     open drain --> KILL  the traffic from going to the MAC of MTI
*                              GT_FALSE - NOT open drain --> allow the traffic going to the MAC of MTI
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMpfSauOpenDrainSet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 openDrain
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask = (GT_U32)(0x3 << 30);

    if(HWS_DEV_SILICON_TYPE(devNum) != Falcon)
    {
        return GT_NOT_SUPPORTED;
    }

    if (PRV_HWS_D2D_IS_REDUCE_PORT_MAC(devNum, portNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MTI_CPU_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
    }
    else
    {
        if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
        {

            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT,( portNum & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
            if (portNum%8 == 0)     /*in seg mode port 0 is taken from mpf 8 */
            {
                baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
            }
            else /*in seg mode port 4 is taken from mpf 9 */
            {
                baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
            }
        }
        else
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
        }
    }

    regAddr = baseAddr + MPFS_SAU_CONTROL;
    if(openDrain == GT_TRUE)
    {
        regData = 0x1 << 30;/* we KILL the traffic from passing through the MPFS ...
                       not going to the MAC of MTI */
    }
    else
    {
        regData = 0; /* allow normal traffic via the MPFS to the MAC of MTI */
    }

    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

    return GT_OK;
}
/**
* @internal mvHwsMpfSauOpenDrainGet function
* @endinternal
*
* @brief   Get 'open drain' in MPFS for specific port
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] openDrainPtr   - (pointer to)
*                              GT_TRUE  -     open drain --> KILL  the traffic from going to the MAC of MTI
*                              GT_FALSE - NOT open drain --> allow the traffic going to the MAC of MTI
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMpfSauOpenDrainGet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *openDrainPtr
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask = (GT_U32)(0x3 << 30);

    if (PRV_HWS_D2D_IS_REDUCE_PORT_MAC(devNum, portNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MTI_CPU_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
    }
    else
    {
        if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
        {

            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT,( portNum & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
            if (portNum%8 == 0)     /*in seg mode port 0 is taken from mpf 8 */
            {
                baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
            }
            else /*in seg mode port 4 is taken from mpf 9 */
            {
                baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
            }
        }
        else
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
        }
    }

    regAddr = baseAddr + MPFS_SAU_CONTROL;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    if(regData == (0x1<<30))
    {
        *openDrainPtr = GT_TRUE;
    }
    else
    {
        *openDrainPtr = GT_FALSE;
    }


    return GT_OK;
}

/**
* @internal mvHwsMpfSauStatusGet function
* @endinternal
*
* @brief  Set MPFS SAU status/counter get - this counter is incremented on
*         every packet discard when the port is in link down
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] dropCntArrSize  - port mode
* @param[out] dropCntValueArr - (pointer to drop counter)
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMpfSauStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portNum,
    IN GT_U32                  dropCntArrSize,
    OUT GT_U32                  *dropCntValueArr
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask = 0xFFFFFFFF;
    GT_U32 tempPort = portNum;
    GT_U32 i;

    if(dropCntValueArr == NULL)
    {
        return GT_BAD_PTR;
    }

    for(i = 0; i < dropCntArrSize; i++)
    {
        if((i == 8) || (i == 9))
        {
            tempPort = portNum & 0xfffffff8;
        }
        else
        {
            tempPort = portNum + i;
        }

        CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT, tempPort, &baseAddr, &unitIndex, &unitNum));

        if(i == 8)
        {
            baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
        }
        else if(i == 9)
        {
            baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
        }
        else
        {
            /*do nothing*/
        }

        regAddr = baseAddr + MPFS_SAU_STATUS;

        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

        dropCntValueArr[i] = regData;
    }

    return GT_OK;
}

#endif
/**
* @internal mvHwsMtipExtReset function
* @endinternal
*
* @brief  Reset/unreset MTIP unit
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] reset           - true = reset/ false = unreset
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtReset
(
    IN GT_U8                   devNum,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 reset
)
{
    GT_U32 baseAddr, unitIndex, unitNum, portGroup, i;
    GT_U32 regAddr, regData, regMask, localPortIndex;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtReset ******\n");
    }
#endif

    if(mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        /*
            mtip_cpu_ext_units_RegFile.Port_Clocks.sd_pcs_tx_clk_en.set(1'b1);
            mtip_cpu_ext_units_RegFile.Port_Clocks.sd_pcs_rx_clk_en.set(1'b1);

            mtip_cpu_ext_units_RegFile.Port_Clocks.app_clk_en.set(1'b1);
            mtip_cpu_ext_units_RegFile.Port_Clocks.mac_tx_clk_en.set(1'b1);
            mtip_cpu_ext_units_RegFile.Port_Clocks.mac_rx_clk_en.set(1'b1);
        */
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
        regAddr = baseAddr + MTIP_CPU_EXT_PORT_RESET;
        regData = (reset == GT_FALSE) ? 0x3 : 0;

        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, 0x3));
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsMtipExtReset ******\n");
        }
#endif

        return GT_OK;
    }

    /* bit [7:0] v = m_RAL.mtip_ext_units_RegFile.Global_Reset_Control.gc_sd_tx_reset_.get();*/
    /* for UNIT_MTI_EXT we check only port 0 or 8 */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_RESET_CONTROL;
    regData = 0;
#if !defined (RAVEN_DEV_SUPPORT)
    localPortIndex = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, portNum);
#else
    localPortIndex = portNum %8;
#endif
    #ifndef MICRO_INIT
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    #else
    portGroup=0;
    #endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    regMask = 0;
    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        /* set the rx[0-7] and the tx[8-15] bits */
        regMask |= ((1<<(localPortIndex+i) )| (1<<(8+localPortIndex+i)));
    }
    regData = (reset == GT_FALSE) ? regMask : 0;

    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtReset ******\n");
    }
#endif
    return GT_OK;
}



/**
* @internal mvHwsMtipExtSetLaneWidth function
* @endinternal
*
* @brief  Set port lane width (20/40/80)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] width           - lane width
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtSetLaneWidth
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN MV_HWS_SERDES_BUS_WIDTH_ENT width
)
{
    GT_U32 baseAddr, unitIndex, unitNum, portGroup, i;
    GT_U32 regAddr, regData, regMask, localPortIndex;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtSetLaneWidth ******\n");
    }
#endif

    if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
    {
        /*
            Value = 0 - 40 bit - 25G_BaseR1
            Value = 1 - 20 bit - 10G_BaseR1
            mtip_cpu_ext_units_RegFile.Port_Control.sd_n2.set(Value)
        */
        regData = (portMode == _10GBase_KR) ? 1 : 0;
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
        regAddr = baseAddr + MTIP_CPU_EXT_PORT_CONTROL;

        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, 0x1));
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsMtipExtSetLaneWidth ******\n");
        }
#endif
        return GT_OK;
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_PMA_CONTROL;
#if !defined (RAVEN_DEV_SUPPORT)
    localPortIndex = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, portNum);
#else
    localPortIndex = portNum %8;
#endif
#ifndef FALCON_DEV_SUPPORT
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
#else
    portGroup = 0;
#endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }
    regData = 0;
    regMask = 0;
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        switch (width) {
            case _80BIT_ON:
                regData |=  (0x100<<(localPortIndex+i));
                break;
            case _40BIT_ON:
                regData |=  (0x0<<(localPortIndex+i));
                break;
            case _20BIT_ON:
            case _10BIT_ON:
                regData |=  (0x1<<(localPortIndex+i));
                break;
            case _10BIT_OFF:
                regData =  0;
                break;
            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetLaneWidth hws bad width param"));
                break;
        }
        regMask |=  (0x101<<(localPortIndex+i));
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtSetLaneWidth ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsMtipExtSetChannelMode function
* @endinternal
*
* @brief  for speeds 100G/200G/400G Set port channel mode
*         (20/40/80)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/

GT_STATUS mvHwsMtipExtSetChannelMode
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN GT_BOOL                     enable
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask, localPortIndex, evenPort, quadPort;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtSetChannelMode ******\n");
    }
#endif
    if ((!HWS_40G_R4_MODE_CHECK(portMode)) &&
        (!HWS_100G_R4_MODE_CHECK(portMode)) &&
        (portMode != _100GBase_KR2) &&
        (portMode != _102GBase_KR2) &&
        (portMode != _100GBase_CR2) &&
        (portMode != _100GBase_SR_LR2) &&
        (portMode != _200GBase_KR4) &&
        (portMode != _200GBase_CR4) &&
        (portMode != _200GBase_SR_LR4) &&
        (portMode != _200GBase_KR8) &&
        (portMode != _200GBase_CR8) &&
        (portMode != _200GBase_SR_LR8) &&
        (portMode != _400GBase_CR8) &&
        (portMode != _400GBase_KR8) &&
        (portMode != _400GBase_SR_LR8) &&
        (portMode != _424GBase_KR8))
    {
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of   mvHwsMtipExtSetChannelMode ******\n");
        }
#endif
        return GT_OK;
    }
    else
    {
#if !defined (RAVEN_DEV_SUPPORT)
        localPortIndex = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, portNum);
#else
        localPortIndex = portNum %8;
#endif
        if (localPortIndex%2 != 0)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetChannelMode hws bad port param"));
        }
        else if (!((portMode == _100GBase_KR2) || (portMode == _102GBase_KR2) || (portMode == _100GBase_CR2) || (portMode == _100GBase_SR_LR2)) && (localPortIndex%4 != 0))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetChannelMode hws bad port param"));
        }
    }
    regMask = 0xFF;
    regData = 0;
    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_CHANNEL_CONTROL;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    evenPort = localPortIndex / 2;
    quadPort = localPortIndex / 4;

    switch (portMode){
        case _40GBase_KR4:
        case _40GBase_CR4:
        case _40GBase_SR_LR4:
        case _42GBase_KR4:
        case _48GBaseR4:
        case _48GBase_SR4:
            if (enable)
            {
                regData |= 1 << quadPort;
            }
            else
            {
                regData &= ~(1 << quadPort);
            }
            break;

        case _100GBase_CR2:
        case _100GBase_KR2:
        case _102GBase_KR2:
        case _100GBase_SR_LR2:
        case _106GBase_KR4:
        case _100GBase_KR4:
        case _100GBase_CR4:
        case _100GBase_SR4:
            if (enable)
            {
                regData |= 1 << (2 + evenPort);
            }
            else
            {
                regData &= ~(1 << (2 + evenPort));
            }
            break;
        case _200GBase_CR4:
        case _200GBase_KR4:
        case _200GBase_SR_LR4:
            if (enable)
            {
                regData |= 1 << (6 + quadPort);
            }
            else
            {
                regData &= ~(1 << (6 + quadPort));
            }
            break;
        case _200GBase_CR8:
        case _200GBase_KR8:
        case _200GBase_SR_LR8:
        case _400GBase_KR8:
        case _400GBase_CR8:
        case _400GBase_SR_LR8:
        case _424GBase_KR8:
            if (enable)
            {
                regData |= 3 << 6;
            }
            else
            {
                regData &= ~(3 << 6);
            }
            break;
        default:
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetChannelMode hws bad port mode"));
            break;
    }

    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtSetChannelMode ******\n");
    }
#endif
    return GT_OK;
}


/**
* @internal mvHwsMtipExtLowJitterEnable function
* @endinternal
*
* @brief  EXT unit set low jitter for 10G port
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMtipExtLowJitterEnable
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN GT_BOOL                     enable
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, regMask, localPortIndex;


    if (portMode != _10GBase_KR)
    {
        return GT_OK;
    }
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtipExtLowJitterEnable ******\n");
    }
#endif

    if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
    {
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsMtipExtLowJitterEnable ******\n");
        }
#endif
        return GT_OK;
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + MTIP_EXT_GLOBAL_CLOCK_CONTROL;
#if !defined (RAVEN_DEV_SUPPORT)
    localPortIndex = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(devNum, portNum);
#else
    localPortIndex = portNum %8;
#endif

    regMask =  (0x1<<(localPortIndex+8));
    regData = (enable == GT_TRUE) ? regMask : 0;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtipExtLowJitterEnable ******\n");
    }
#endif

    return GT_OK;
}

#ifndef MICRO_INIT
GT_STATUS hwsD2dMacDump
(
    GT_U8   devNum,
    GT_U32  d2dIdx,
    GT_BOOL isRaven
)
{
    GT_U32 regAddr;                 /* register address */
    GT_U32 regData;                 /* register value */
    GT_U32 regMask1 = 0xFFFFFFFF;   /* register mask */
    GT_U32 regMask2 = 0xFFFF;       /* register mask */
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 channelNum;
    GT_BOOL nonZeroCounterFound = GT_FALSE;

    if(d2dIdx >= hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS)
    {
        return GT_OUT_OF_RANGE;
    }

    if (isRaven == GT_FALSE)
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIdx, &baseAddr, &unitIndex, &unitNum ));
        cpssOsPrintf("EAGLE D2D\n");
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIdx, &baseAddr, &unitIndex, &unitNum ));
        cpssOsPrintf("RAVEN D2D\n");
    }

    /* MAC RX section */
    for(channelNum = 0; channelNum < 17; channelNum++)
    {
        regAddr = baseAddr + D2D_MAC_RX_STAT_GOOD_PACKETS_LOW + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
        if(regData != 0)
        {
            cpssOsPrintf("MAC RX[d2d=%d] Good Packets Low[channelNum=%2.2d] = 0x%8.8x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }
        regAddr = baseAddr + D2D_MAC_RX_STAT_GOOD_PACKETS_HIGH + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
        if(regData != 0)
        {
            cpssOsPrintf("MAC RX[d2d=%d] Good Packets High[channelNum=%2.2d] = 0x%4.4x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }

        regAddr = baseAddr + D2D_MAC_RX_STAT_BAD_PACKETS_LOW + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
        if(regData != 0)
        {
            cpssOsPrintf("ERROR: MAC RX[d2d=%d] Bad Packets Low[channelNum=%2.2d] = 0x%8.8x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }
        regAddr = baseAddr + D2D_MAC_RX_STAT_BAD_PACKETS_HIGH + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
        if(regData != 0)
        {
            cpssOsPrintf("ERROR: MAC RX[d2d=%d] Bad Packets High[channelNum=%2.2d] = 0x%4.4x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }
    }

    regAddr = baseAddr + 0x4F08;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX - ERROR CAUSE[d2d=%d] = 0x%4.4x\n", d2dIdx, regData);
    }
    regAddr = baseAddr + 0x4F0C;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX - TX First FIFO Overflow[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    }
    regAddr = baseAddr + 0x4F10;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX - MAC Error TX First Interface Sequencing[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    }
    regAddr = baseAddr + 0x4F14;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX - MAC Error TX First Credit Underflow[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    }
    regAddr = baseAddr + 0x4F18;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX - MAC Error RX First FIFO Overflow 0[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    }
    regAddr = baseAddr + 0x4F1C;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX -   MAC Error RX First Credit Underflow[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    }
    regAddr = baseAddr + 0x4F20;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX -   MAC Error RX First FIFO Overflow 1[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    }
    regAddr = baseAddr + 0x4F24;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    if(regData != 0)
    {
        cpssOsPrintf("MAC RX -   MAC Error RX Bad Address[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    }


    /* MAC TX section */
    for(channelNum = 0; channelNum < 17; channelNum++)
    {
        regAddr = baseAddr + D2D_MAC_TX_STAT_GOOD_PACKETS_LOW + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
        if(regData != 0)
        {
            cpssOsPrintf("MAC TX[d2d=%d] Good Packets Low[channelNum=%2.2d] = 0x%8.8x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }
        regAddr = baseAddr + D2D_MAC_TX_STAT_GOOD_PACKETS_HIGH + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
        if(regData != 0)
        {
            cpssOsPrintf("MAC TX[d2d=%d] Good Packets High[channelNum=%2.2d] = 0x%4.4x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }

        regAddr = baseAddr + D2D_MAC_TX_STAT_BAD_PACKETS_LOW + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
        if(regData != 0)
        {
            cpssOsPrintf("MAC TX[d2d=%d] Bad Packets Low[channelNum=%2.2d] = 0x%8.8x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }
        regAddr = baseAddr + D2D_MAC_TX_STAT_BAD_PACKETS_HIGH + 8 * channelNum;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
        if(regData != 0)
        {
            cpssOsPrintf("MAC TX[d2d=%d] Bad Packets High[channelNum=%2.2d] = 0x%4.4x\n", d2dIdx, channelNum, regData);
            nonZeroCounterFound = GT_TRUE;
        }
    }
    regAddr = baseAddr + 0x4400;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    cpssOsPrintf("MAC TX -  MAC Error TX Bad Address[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    regAddr = baseAddr + 0x4600;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask2));
    cpssOsPrintf("MAC TX - MAC TX Error Cause[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);

    if(nonZeroCounterFound == GT_FALSE)
    {
        cpssOsPrintf("\nMAC RX/TX[d2d=%d] ALL COUNTERS ARE ZERO\n", d2dIdx);
    }

    cpssOsPrintf("\nD2D PCS\n");
    regAddr = baseAddr + 0x7040;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    cpssOsPrintf("PCS -  PCS Event Cause[d2d=%d] = 0x%4.4x\n", d2dIdx, regData);
    regAddr = baseAddr + 0x7058;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    cpssOsPrintf("PCS - PCS Receive Status[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);

    cpssOsPrintf("\nD2D PMA\n");
    regAddr = baseAddr + 0x8030;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    cpssOsPrintf("PMA - PMA Sticky Status[d2d=%d] = 0x%4.4x\n", d2dIdx, regData);
    regAddr = baseAddr + 0x8034;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    cpssOsPrintf("PMA - PMA 66b Alignment Status[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    regAddr = baseAddr + 0x8038;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    cpssOsPrintf("PMA - PMA Debug Clock Domain Crossing Sticky Status[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);
    regAddr = baseAddr + 0x8060;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    cpssOsPrintf("PMA - PMA Event Cause[d2d=%d] cause = 0x%4.4x\n", d2dIdx, regData);

#if 0
    /* PCS section */
    nonZeroCounterFound = GT_FALSE;
    /* PCS Transmit Statistic Low part */
    regAddr = baseAddr + D2D_PCS_PCS_STAT_TX_BLOCK_LOW;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    if(regData != 0)
    {
        cpssOsPrintf("PCS TX[d2d=%d] Stat TX Low = 0x%8.8x\n", d2dIdx, regData);
        nonZeroCounterFound = GT_TRUE;
    }
    /* PCS Transmit Statistic High part */
    regAddr = baseAddr + D2D_PCS_PCS_STAT_TX_BLOCK_HIGH;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    if(regData != 0)
    {
        cpssOsPrintf("PCS TX[d2d=%d] Stat TX High = 0x%8.8x\n", d2dIdx, regData);
        nonZeroCounterFound = GT_TRUE;
    }
    /* PCS Receive Statistic Low part */
    regAddr = baseAddr + D2D_PCS_PCS_STAT_RX_BLOCK_LOW;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    if(regData != 0)
    {
        cpssOsPrintf("PCS RX[d2d=%d] Stat RX Low = 0x%8.8x\n", d2dIdx, regData);
        nonZeroCounterFound = GT_TRUE;
    }
    /* PCS Receive Statistic Low part */
    regAddr = baseAddr + D2D_PCS_PCS_STAT_RX_BLOCK_HIGH;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask1));
    if(regData != 0)
    {
        cpssOsPrintf("PCS RX[d2d=%d] Stat RX High = 0x%8.8x\n", d2dIdx, regData);
        nonZeroCounterFound = GT_TRUE;
    }

    if(nonZeroCounterFound == GT_FALSE)
    {
        cpssOsPrintf("PCS RX/TX[d2d=%d] ALL COUNTERS ARE ZERO\n", d2dIdx);
    }
#endif


    return GT_OK;
}

/**
* @internal mvHwsMpfLoobackSet function
* @endinternal
*
* @brief   Set MPF loopback.
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] lbType          - loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMpfLoobackSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_LB_TYPE     lbType
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMpfLoobackSet ******\n");
    }
#endif
    switch(lbType)
    {
        case DISABLE_LB:
            regData = 0x0;
            break;

        case TX_2_RX_LB:
            regData = 0x1;
            break;

        default:
            return GT_NOT_SUPPORTED;
    }

    if (PRV_HWS_D2D_IS_REDUCE_PORT_MAC(devNum, portNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MTI_CPU_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
    }
    else
    {
        if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
        {

            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT,( portNum & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
            if (portNum%8 == 0)     /*in seg mode port 0 is taken from mpf 8 */
            {
                baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
            }
            else /*in seg mode port 4 is taken from mpf 9 */
            {
                baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
            }
        }
        else
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
        }
    }
    regAddr = baseAddr + MPFS_CLOCK_CONTROL;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 30, 1, regData));

    regAddr = baseAddr + MPFS_LOOPBACK_CONTROL;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 1, regData));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMpfLoobackSet ******\n");
    }
#endif
    return GT_OK;
}


/**
* @internal mvHwsMpfLoobackGet function
* @endinternal
*
* @brief   Get MPF loopback status.
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] lbType         - loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMpfLoobackGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
)
{
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMpfLoobackGet ******\n");
    }
#endif

    if (PRV_HWS_D2D_IS_REDUCE_PORT_MAC(devNum, portNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MTI_CPU_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
    }
    else
    {
        if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
        {

            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT,( portNum & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
            if (portNum%8 == 0)     /*in seg mode port 0 is taken from mpf 8 */
            {
                baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
            }
            else /*in seg mode port 4 is taken from mpf 9 */
            {
                baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
            }
        }
        else
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
        }
    }

    regAddr = baseAddr + MPFS_LOOPBACK_CONTROL;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 1, &regData));

    *lbType = (regData == 0) ? DISABLE_LB : TX_2_RX_LB;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMpfLoobackGet ******\n");
    }
#endif
    return GT_OK;
}

#endif

/*******************************************************************************
*           Copyright 2001, Marvell International Ltd.
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
* @file mvHwsPortAnp.c
*
* @brief This file contains API for ANPp port configuartion
*
* @version   1
********************************************************************************
*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortAnp.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>


/**************************** Globals ****************************************************/
extern  GT_BOOL hwsPpHwTraceFlag;
extern GT_U32  cpssDeviceRunCheck_onEmulator(void);
/**************************** Definition *************************************************/

typedef enum
{
    MV_HWS_ANP_PROG_STATE_INIT        = 0x0,
    MV_HWS_ANP_PROG_STATE_PWRUP       = 0x1,
    MV_HWS_ANP_PROG_STATE_TXON        = 0x2,
    MV_HWS_ANP_PROG_STATE_RXSD        = 0x3,
    MV_HWS_ANP_PROG_STATE_RXON        = 0x4,
    MV_HWS_ANP_PROG_STATE_TXRXSD      = 0x5,
    MV_HWS_ANP_PROG_STATE_TXRXON      = 0x6,
    MV_HWS_ANP_PROG_STATE_NORM        = 0x7,
    MV_HWS_ANP_PROG_STATE_PWRDN       = 0x8,
    MV_HWS_ANP_PROG_STATE_TX_RX_INIT  = 0xa,
    MV_HWS_ANP_PROG_STATE_TX_TRAIN    = 0xb,
    MV_HWS_ANP_PROG_STATE_RX_TRAIN    = 0xc,
    MV_HWS_ANP_PROG_STATE_RX_INIT     = 0xd
}MV_HWS_ANP_PROG_STATE;




#define MV_HWS_ANP_TIMER_DISABLE_CNS 0xFFFF


GT_STATUS mvHwsAnpSetForStaticPort (GT_BOOL set)
{
    PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_SET(mvHwsAnpUsedForStaticPort, set);
    return GT_OK;
}
GT_STATUS mvHwsAnpUseCmdTableSet (GT_BOOL set)
{
    PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_SET(mvHwsAnpUseCmdTable, set);
    return GT_OK;
}

GT_STATUS mvHwsHawkLocalIndexToAnpIndexConvert
(
    GT_U8                       devNum,
    GT_U32                      portNum,
    IN GT_U32                   localIdx,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT GT_U32                  *anpIdxPtr
);

extern GT_STATUS mvHwsExtIfLastInPortGroupCheck
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT   *isLastPtr
);

extern GT_STATUS mvHwsExtIfFirstInSerdesGroupCheck
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT    *isFisrtPtr
);

extern GT_STATUS mvHwsExtIfLastInSerdesGroupCheck
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT    *isLastPtr
);

/**************************** Declaration ********************************************/
GT_U32 mvHwsAnpFieldSet
(
    IN GT_U8    devNum,
    IN GT_U32   hawkField,
    IN GT_U32   harrierField,
    IN GT_U32   phoenixField
)
{
    switch(HWS_DEV_SILICON_TYPE(devNum))
    {
        case AC5P:
            return hawkField;
        case Harrier:
            return harrierField;
        case AC5X:
        default:
            return phoenixField;
    }
}

/**
* @internal mvHwsAnpClose function
* @endinternal
*
* @brief  ANP unit close
*
* @param[in] devNum                - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpClose
(
    IN GT_U8              devNum
)
{
    if (NULL != PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum])
    {
        hwsOsFreeFuncPtr(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum]);
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum] = NULL;
    }
    if (NULL != PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum])
    {
        hwsOsFreeFuncPtr(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum]);
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum] = NULL;
    }
    if (NULL != PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum])
    {
        hwsOsFreeFuncPtr(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum]);
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum] = NULL;
    }
    if (NULL != PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSkipResetPtr[devNum])
    {
        hwsOsFreeFuncPtr(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSkipResetPtr[devNum]);
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSkipResetPtr[devNum] = NULL;
    }


    return GT_OK;
}

/**
* @internal mvHwsAnpInit function
* @endinternal
*
* @brief  ANP unit init
*
* @param[in] devNum                - system device number
* @param[in] skipWritingToHW       - skip writing to HW
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpInit
(
    IN GT_U8   devNum,
    IN GT_BOOL skipWritingToHW
)
{
    GT_U32 cluster, anpIdx, field;
    GT_U32 portNum;
    MV_HWS_UNITS_ID unitIdArr[2];
    GT_U32 i;
    MV_HWS_PORT_INIT_PARAMS curPortParams;  /* current port parameters */
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
#if 0
    GT_U32 hawkPort400IdxConvert[8] = {0,1,2,6,10,14,18,22};
    GT_U32 harrierPort400IdxConvert[8] = {0,2,4,6,8,10,12,14};
#endif

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAnpInit ******\n");
    }
#endif

    /********************/
    /* DBs allocations */
    /********************/
    if ( PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum] == NULL )
    {
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum] = (MV_HWS_AP_CFG *)cpssOsMalloc(sizeof(MV_HWS_AP_CFG)*(hwsDeviceSpecInfo[devNum].portsNum));
    }
    hwsOsMemSetFuncPtr(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum], 0, sizeof(MV_HWS_AP_CFG)*(hwsDeviceSpecInfo[devNum].portsNum));

    if ( PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum] == NULL )
    {
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum] = (MV_HWS_ANP_MISC_PARAM_STC *)cpssOsMalloc(sizeof(MV_HWS_ANP_MISC_PARAM_STC)*(hwsDeviceSpecInfo[devNum].portsNum));
    }
    hwsOsMemSetFuncPtr(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum], 0, sizeof(MV_HWS_ANP_MISC_PARAM_STC)*(hwsDeviceSpecInfo[devNum].portsNum));

    if ( PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum] == NULL )
    {
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum] = (GT_U32 *)cpssOsMalloc(sizeof(GT_U32)*(hwsDeviceSpecInfo[devNum].portsNum));
    }
    hwsOsMemSetFuncPtr(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum], MV_HWS_ANP_CMD_TABLE_EMPTY_CNS, sizeof(GT_U32)*(hwsDeviceSpecInfo[devNum].portsNum));

    if ( PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSkipResetPtr[devNum] == NULL )
    {
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSkipResetPtr[devNum] = (GT_BOOL *)cpssOsMalloc(sizeof(GT_BOOL)*(hwsDeviceSpecInfo[devNum].portsNum));
    }
    hwsOsMemSetFuncPtr(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSkipResetPtr[devNum], 0, sizeof(GT_BOOL)*(hwsDeviceSpecInfo[devNum].portsNum));

#if 0
    for(portNum = 0; portNum < hwsDeviceSpecInfo[devNum].portsNum; portNum++)
    {
        mvHwsAnpPortMiscParamDb[devNum][portNum].interopLinkTimer = 0xFF;
    }
#endif

    /* if skipWritingToHW == true, make only database allocations and don't write to FW */
    if (skipWritingToHW == GT_TRUE)
    {
        return GT_OK;
    }

    /*skip hw write in case of HA or HS */
    CHECK_STATUS(cpssSystemRecoveryStateGet(&tempSystemRecovery_Info));
    if (!(((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
          (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E)) &&
          (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)))
    {
        if(HWS_DEV_SILICON_TYPE(devNum) == Harrier)
        {
            /* fix multi lane tx train*/
            field = HARRIER_ANP_UNITS_GLOBAL_AN_TRAIN_TYPE_TX_TRAIN_COUPLE_ENABLE_E;
            for(portNum = 0; portNum < hwsDeviceSpecInfo[devNum].portsNum; portNum = portNum+8)
            {
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, ANP_400_UNIT, field, 0x1, NULL));
            }
        }
        for(portNum = 0; portNum < hwsDeviceSpecInfo[devNum].portsNum; portNum++)
        {
            unitIdArr[0] = unitIdArr[1] =  LAST_UNIT;

            if(mvHwsMtipIsReducedPort(devNum, portNum))
            {
                unitIdArr[0] = ANP_CPU_UNIT;
            }
            else
            {
                if (GT_OK == hwsPortModeParamsGetToBuffer(devNum, 0, portNum, _10GBase_KR, &curPortParams))
                {
                    if ((curPortParams.portMacType == MTI_MAC_100) ||
                        (curPortParams.portMacType == MTI_MAC_100_BR))
                    {
                        unitIdArr[0] = ANP_400_UNIT;
                        if(hwsIsIronmanAsPhoenix())
                        {
                            break;
                        }
                    }
                }
                if (GT_OK == hwsPortModeParamsGetToBuffer(devNum, 0, portNum, _20G_QXGMII, &curPortParams))
                {
                    if (curPortParams.portMacType == MTI_USX_MAC)
                    {
                        unitIdArr[1] = ANP_USX_UNIT;
                    }
                }
            }

            /*********************/
            /* ANP init sequence */
            /*********************/
            for(i = 0; i < 2; i++)
            {
                if (unitIdArr[i] != LAST_UNIT)
                {
                    if(hwsIsIronmanAsPhoenix())
                    {
                        break;
                    }
                    if(HWS_DEV_SILICON_TYPE(devNum) == Harrier)
                    {
                        /* disable in harrier the core_reset */
                        field = HARRIER_ANP_UNITS_PORT0_CONTROL10_P0_RESET_CORE_TX_ACK_ENABLE_E;
                        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, ANP_400_UNIT, field, 0x0, NULL));
                        field = HARRIER_ANP_UNITS_PORT0_CONTROL10_P0_RESET_CORE_RX_ACK_ENABLE_E;
                        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, ANP_400_UNIT, field, 0x0, NULL));
                        field = HARRIER_ANP_UNITS_PORT0_CONTROL10_P0_RESET_TX_CORE_ENABLE_E;
                        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, ANP_400_UNIT, field, 0x0, NULL));
                        field = HARRIER_ANP_UNITS_PORT0_CONTROL10_P0_RESET_RX_CORE_ENABLE_E;
                        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, ANP_400_UNIT, field, 0x0, NULL));
                    }
                    /* ANP SW reset enable */
                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_PWM_SOFT_RESET__E,
                                             HARRIER_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_PWM_SOFT_RESET__E,
                                             PHOENIX_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_PWM_SOFT_RESET__E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0, NULL));

                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_GLOBAL_CONTROL_REG_TX_READY_LOSS_LATCH_EN_E,
                                             HARRIER_ANP_UNITS_GLOBAL_CONTROL_REG_TX_READY_LOSS_LATCH_EN_E,
                                             PHOENIX_ANP_UNITS_GLOBAL_CONTROL_REG_TX_READY_LOSS_LATCH_EN_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0, NULL));

                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_GLOBAL_CONTROL_REG_DSP_SIGDET_LOSS_LATCH_EN_E,
                                             HARRIER_ANP_UNITS_GLOBAL_CONTROL_REG_DSP_SIGDET_LOSS_LATCH_EN_E,
                                             PHOENIX_ANP_UNITS_GLOBAL_CONTROL_REG_DSP_SIGDET_LOSS_LATCH_EN_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0, NULL));

                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_GLOBAL_CONTROL_REG_DSP_LOCK_LOSS_LATCH_EN_E,
                                             HARRIER_ANP_UNITS_GLOBAL_CONTROL_REG_DSP_LOCK_LOSS_LATCH_EN_E,
                                             PHOENIX_ANP_UNITS_GLOBAL_CONTROL_REG_DSP_LOCK_LOSS_LATCH_EN_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0, NULL));

                    /*from DataCom comparison*/
                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_GLOBAL_SD_TX_IDLE_MIN_WAIT_REG_TX_IDLE_MIN_WAIT_S_E,
                                             HARRIER_ANP_UNITS_GLOBAL_SD_TX_IDLE_MIN_WAIT_REG_TX_IDLE_MIN_WAIT_S_E,
                                             PHOENIX_ANP_UNITS_GLOBAL_SD_TX_IDLE_MIN_WAIT_REG_TX_IDLE_MIN_WAIT_S_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x3D09, NULL));

                   /*from DataCom comparison*/
                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_GLOBAL_SD_RX_IDLE_MIN_WAIT_REG_RX_IDLE_MIN_WAIT_S_E,
                                             HARRIER_ANP_UNITS_GLOBAL_SD_RX_IDLE_MIN_WAIT_REG_RX_IDLE_MIN_WAIT_S_E,
                                             PHOENIX_ANP_UNITS_GLOBAL_SD_RX_IDLE_MIN_WAIT_REG_RX_IDLE_MIN_WAIT_S_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x3D09, NULL));

                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_GLOBAL_PHY_GEN_PDN_TO_LOAD_TIMER_REG_PHY_GEN_PDN_TO_LOAD_TIMER_E,
                                             HARRIER_ANP_UNITS_GLOBAL_PHY_GEN_PDN_TO_LOAD_TIMER_REG_PHY_GEN_PDN_TO_LOAD_TIMER_E,
                                             PHOENIX_ANP_UNITS_GLOBAL_PHY_GEN_PDN_TO_LOAD_TIMER_REG_PHY_GEN_PDN_TO_LOAD_TIMER_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x1D5, NULL));

                    /*from DataCom comparison*/
                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_GLOBAL_CONTROL4_REG_RXSTR_REGRET_ENABLE_E,
                                             HARRIER_ANP_UNITS_GLOBAL_CONTROL4_REG_RXSTR_REGRET_ENABLE_E,
                                             PHOENIX_ANP_UNITS_GLOBAL_CONTROL4_REG_RXSTR_REGRET_ENABLE_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x0, NULL));
                    /*from DataCom comparison*/
                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_GLOBAL_CONTROL4_REG_TXSTR_REGRET_ENABLE_E,
                                             HARRIER_ANP_UNITS_GLOBAL_CONTROL4_REG_TXSTR_REGRET_ENABLE_E,
                                             PHOENIX_ANP_UNITS_GLOBAL_CONTROL4_REG_TXSTR_REGRET_ENABLE_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x0, NULL));

                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_GLOBAL_PHY_GEN_LOAD_TO_PUP_TIMER_REG_PHY_GEN_LOAD_TO_PUP_TIMER_E,
                                             HARRIER_ANP_UNITS_GLOBAL_PHY_GEN_LOAD_TO_PUP_TIMER_REG_PHY_GEN_LOAD_TO_PUP_TIMER_E,
                                             PHOENIX_ANP_UNITS_GLOBAL_PHY_GEN_LOAD_TO_PUP_TIMER_REG_PHY_GEN_LOAD_TO_PUP_TIMER_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x1D5, NULL));

                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_PORT0_AN_TIED_IN_P0_RING_OSC_A_E,
                                             HARRIER_ANP_UNITS_PORT0_AN_TIED_IN_P0_RING_OSC_A_E,
                                             PHOENIX_ANP_UNITS_PORT0_AN_TIED_IN_P0_RING_OSC_A_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x1, NULL));

                    /* must - remove Control10 writes */
                    /*field = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_ANP_UNITS_PORT0_CONTROL10_P0_REG_RESET_EN_TX_TRAIN_S_E : PHOENIX_ANP_UNITS_PORT0_CONTROL10_P0_REG_RESET_EN_TX_TRAIN_S_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x0, NULL));

                    field = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_ANP_UNITS_PORT0_CONTROL10_P0_REG_RESET_EN_RX_TRAIN_S_E : PHOENIX_ANP_UNITS_PORT0_CONTROL10_P0_REG_RESET_EN_RX_TRAIN_S_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x0, NULL));

                    field = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_ANP_UNITS_PORT0_CONTROL10_P0_REG_RESET_EN_RX_INIT_S_E : PHOENIX_ANP_UNITS_PORT0_CONTROL10_P0_REG_RESET_EN_RX_INIT_S_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x0, NULL));

                    field = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_ANP_UNITS_PORT0_CONTROL10_P0_REG_RESET_EN_PU_PLL_S_E : PHOENIX_ANP_UNITS_PORT0_CONTROL10_P0_REG_RESET_EN_PU_PLL_S_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x0, NULL));*/

                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_PORT0_CONTROL10_P0_REG_TX_TRAIN_DSP_SIGDET_SEL_S_E,
                                             HARRIER_ANP_UNITS_PORT0_CONTROL10_P0_REG_TX_TRAIN_DSP_SIGDET_SEL_S_E,
                                             PHOENIX_ANP_UNITS_PORT0_CONTROL10_P0_REG_TX_TRAIN_DSP_SIGDET_SEL_S_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x1, NULL));

                    /*must - HCD restart problem fix*/
                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_PORT0_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_NORM_INF_S_E,
                                             HARRIER_ANP_UNITS_PORT0_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_NORM_INF_S_E,
                                             PHOENIX_ANP_UNITS_PORT0_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_NORM_INF_S_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x1, NULL));

                    /*must - remove Control11 writes*/
#if 0
                    field = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_ANP_UNITS_PORT0_CONTROL11_P0_REG_PM_SD_PU_RESET_ON_SFTRST_S_E : PHOENIX_ANP_UNITS_PORT0_CONTROL11_P0_REG_PM_SD_PU_RESET_ON_SFTRST_S_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x0, NULL));
#endif

#if 0
                    /*must - align speed change to DataCom */
                    if(HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum))
                    {
                        field = HAWK_ANP_UNITS_GLOBAL_CONTROL4_REG_PHY_GEN_DN_FRC_E;
                        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x0, NULL));
                        field = HAWK_ANP_UNITS_GLOBAL_CONTROL4_PHY_GEN_RX_DONE_OW_E;
                        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x1, NULL));
                        field = HAWK_ANP_UNITS_GLOBAL_CONTROL4_PHY_GEN_RX_DONE_OW_VAL_E;
                        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x1, NULL));
                        field = HAWK_ANP_UNITS_GLOBAL_CONTROL4_PHY_GEN_TX_DONE_OW_E;
                        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x1, NULL));
                        field = HAWK_ANP_UNITS_GLOBAL_CONTROL4_PHY_GEN_TX_DONE_OW_VAL_E;
                        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x1, NULL));
                    }

                    /*must - alignment to DataCom*/
                    field = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_ANP_UNITS_PORT0_SD_PLL_UP_MAX_TIMER_P0_REG_PLL_UP_TIME_OUT_S_E : PHOENIX_ANP_UNITS_PORT0_SD_PLL_UP_MAX_TIMER_P0_REG_PLL_UP_TIME_OUT_S_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0xb2d05e, NULL));

                    /* CLAMP */
                    field = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?  HAWK_ANP_UNITS_PORT0_CONTROL5_P0_SD_RX_DTL_CLAMP_S_OW_E : PHOENIX_ANP_UNITS_PORT0_CONTROL5_P0_SD_RX_DTL_CLAMP_S_OW_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x1, NULL));
                    /*SQ_DETECT*/
                    field = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?  HAWK_ANP_UNITS_PORT0_CONTROL4_P0_SQ_DETECTED_LPF_OW_E : PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_SQ_DETECTED_LPF_OW_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x1, NULL));
#endif
                    /*from DataCom comparison*/
                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_PORT0_SD_RX_PLL_UP_MIN_TIMER_P0_REG_RX_PLL_UP_MIN_WAIT_S_E,
                                             HARRIER_ANP_UNITS_PORT0_SD_RX_PLL_UP_MIN_TIMER_P0_REG_RX_PLL_UP_MIN_WAIT_S_E,
                                             PHOENIX_ANP_UNITS_PORT0_SD_RX_PLL_UP_MIN_TIMER_P0_REG_RX_PLL_UP_MIN_WAIT_S_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x300000, NULL));
                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_PORT0_SD_TX_PLL_UP_MIN_TIMER_P0_REG_TX_PLL_UP_MIN_WAIT_S_E,
                                             HARRIER_ANP_UNITS_PORT0_SD_TX_PLL_UP_MIN_TIMER_P0_REG_TX_PLL_UP_MIN_WAIT_S_E,
                                             PHOENIX_ANP_UNITS_PORT0_SD_TX_PLL_UP_MIN_TIMER_P0_REG_TX_PLL_UP_MIN_WAIT_S_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x300000, NULL));


                    /* 13/04/2021 updates */
#if 0
                    field = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?  HAWK_ANP_UNITS_PORT0_SD_RX_INIT_MAX_TIMER_P0_REG_RX_INIT_TIME_OUT_S_E : PHOENIX_ANP_UNITS_PORT0_SD_RX_INIT_MAX_TIMER_P0_REG_RX_INIT_TIME_OUT_S_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x4800, NULL));
                    field = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?  HAWK_ANP_UNITS_PORT0_CH_TXRX_MIN_TIMER_P0_RG_TXRX_MIN_TIMER_E : PHOENIX_ANP_UNITS_PORT0_CH_TXRX_MIN_TIMER_P0_RG_TXRX_MIN_TIMER_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x2625a, NULL));

                    field = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?  HAWK_ANP_UNITS_PORT0_PCS_LINK_MAX_TIMER_NORM_P0_PCS_LINK_MAX_TIMER_NORM_E : PHOENIX_ANP_UNITS_PORT0_PCS_LINK_MAX_TIMER_NORM_P0_PCS_LINK_MAX_TIMER_NORM_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0xee6b28, NULL));

#endif
                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_PORT0_CONTROL10_P0_TXT_IGNORE_TX_READY_LOSS_E,
                                             HARRIER_ANP_UNITS_PORT0_CONTROL10_P0_TXT_IGNORE_TX_READY_LOSS_E,
                                             PHOENIX_ANP_UNITS_PORT0_CONTROL10_P0_TXT_IGNORE_TX_READY_LOSS_E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 0x1, NULL));

                    field = mvHwsAnpFieldSet(devNum,
                                             HAWK_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_PWM_SOFT_RESET__E,
                                             HARRIER_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_PWM_SOFT_RESET__E,
                                             PHOENIX_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_PWM_SOFT_RESET__E);
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdArr[i], field, 1, NULL));
                }
            }
        }
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAnpInit ******\n");
    }
#endif


    if((HWS_DEV_SILICON_TYPE(devNum) == AC5X) || (HWS_DEV_SILICON_TYPE(devNum) == Harrier))
    {
        return GT_OK;
    }

    /************************************************************************************************************************ */
    /* portNum      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |10 |11 |12 |13 |14 |15 |16 |17 |18 |19 |20 |21 |22 |23 |24 |25 | */
    /************************************************************************************************************************ */
    /* convert portNum ==> localIdx  according to portMode*/
    /************************************************************************************************************************ */
    /* localIdx 400 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | ----------------------------------------------------------------------- */
    /* localIdx USX | - | - | 0 | 2 | 1 | 3 | 4 | 6 | 5 | 7 | 8 |10 | 9 |11 |12 |14 |13 |15 |16 |18 |17 |19 |20 |22 |21 |23 | */
    /************************************************************************************************************************ */
    /* convert localIdx ==> anpIdx  according to portMode*/
    /************************************************************************************************************************ */
    /* anpIdx 400   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | ----------------------------------------------------------------------- */
    /* anpIdx USX   | - | - | 8 | 8 | 8 | 8 | 9 | 9 | 9 | 9 |10 |10 |10 |10 |11 |11 |11 |11 |12 |12 |12 |12 |13 |13 |13 |13 | */
    /* anpIdx USX_O | - | - | 8 | 8 | 8 | 8 | 8 | 8 | 8 | 8 |10 |10 |10 |10 |10 |10 |10 |10 |12 |12 |12 |12 |12 |12 |12 |12 | */
    /************************************************************************************************************************ */

    /* INIT anpDB */
    /********************************************************************* */
    /* anpIdx      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |10 |11 |12 |13 | */
    /********************************************************************* */
    /* serdesOutCfg| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |10 |11 |12 |13 | */
    /********************************************************************* */
    /* serdesOutCfg - change when port is configure according to port mode */


    /* for each channel init the ANP default parameters */
    for (cluster = 0; cluster < 2; cluster++)
    {
        for (anpIdx = 0; anpIdx < MV_HWS_HAWK_ANP_NUM_CNS; anpIdx++)
        {
            if (anpIdx < 2)
            {
                PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSdMuxDb[cluster][anpIdx].outSerdesIdx = anpIdx;
                PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSdMuxDb[cluster][anpIdx].hwSerdesIdx = anpIdx;
            }
            else
            if (anpIdx < 8)
            {
                PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSdMuxDb[cluster][anpIdx].outSerdesIdx = anpIdx+6;
                PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSdMuxDb[cluster][anpIdx].hwSerdesIdx = anpIdx;
            }
            else
            {
                PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSdMuxDb[cluster][anpIdx].outSerdesIdx = anpIdx;
                PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSdMuxDb[cluster][anpIdx].hwSerdesIdx = anpIdx - 6;
            }
        }
    }


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAnpInit ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsAnpSerdesSdwMuxSet function
* @endinternal
*
* @brief  ANP unit enable SerDes control
*
* @param[in] devNum    - system device number
* @param[in] serdesNum - system SerDes number
* @param[in] anpEnable - true  - SerDes is controlled by ANP
*                      - false - SerDes is controlled by regs
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpSerdesSdwMuxSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               serdesNum,
    IN  GT_BOOL              anpEnable
)
{
    GT_UREG_DATA laneMuxControl0, laneMuxControl0Mask;
    GT_UREG_DATA laneMuxControl1, laneMuxControl1Mask;
    GT_UREG_DATA generalMuxControl0 , generalMuxControl0Mask;

    if (anpEnable)
    {
        if (HWS_DEV_SILICON_TYPE(devNum) == Harrier)
        {
            laneMuxControl0     = 0x400000;
            laneMuxControl0Mask = 0xFFFFFFF;
            laneMuxControl1     = 0x0;
            laneMuxControl1Mask = 0xFFFFF;
        }
        else
        {
            laneMuxControl0     = 0xE41FF40;
            laneMuxControl0Mask = 0xFFFFFFF;
            laneMuxControl1     = 0x1FF;
            laneMuxControl1Mask = 0xFFF;
        }
    }
    else
    {
        if (HWS_DEV_SILICON_TYPE(devNum) == Harrier)
        {
            laneMuxControl0     = 0xFFFFFFF;
            laneMuxControl0Mask = 0xFFFFFFF;
            laneMuxControl1     = 0xFFFFF;
            laneMuxControl1Mask = 0xFFFFF;
        }
        else
        {
            laneMuxControl0     = 0xFFFFFFF;
            laneMuxControl0Mask = 0xFFFFFFF;
            laneMuxControl1     = 0xFFF;
            laneMuxControl1Mask = 0xFFF;
        }
    }
    generalMuxControl0      = 0x3FF;
    generalMuxControl0Mask  = 0x3FF;

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG,
                                        serdesNum,
                                        SDW_LANE_MUX_CONTROL_0,
                                        laneMuxControl0,
                                        laneMuxControl0Mask));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG,
                                        serdesNum,
                                        SDW_LANE_MUX_CONTROL_1,
                                        laneMuxControl1,
                                        laneMuxControl1Mask));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG,
                                        serdesNum,
                                        SDW_GENERAL_MUX_CONTROL,
                                        generalMuxControl0,
                                        generalMuxControl0Mask));

    return GT_OK;
}

/**
* @internal mvHwsAnpPortEnable function
* @endinternal
*
* @brief  ANP unit port enable
*
* @param[in] devNum                - system device number
* @param[in] portNum               - system port number
* @param[in] portMode              - configured port mode
* @param[in] apEnable              - apEnable flag
* @param[in] enable                - True = enable/False =
*       disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortEnable
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              apEnable,
    GT_BOOL              enable
)
{
    GT_U32 anpIdx, anpSwapIdx, serdesIdx, serdesSwapIdx=0;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_PORT_INIT_PARAMS curPortParams;  /* current port parameters */
    GT_U32          laneIdx, anpNum;
    MV_HWS_HAWK_ANP_UNITS anpRegName,anpSwapRegName;
    GT_STATUS       rc;
    MV_HWS_REG_ADDR_FIELD_STC regField;
    MV_HWS_UNITS_ID unitId,swapUnitId;
    GT_U32          unitAddr = 0,swapUnitAddr =0;
    GT_U32          localUnitNum = 0;
    GT_U32          unitIndex = 0,swapPortNum;
    MV_HWS_PORT_IN_GROUP_ENT portFullConfig;
    GT_U32 anp2portConvert[14] = {0,1,2,6,10,14,18,22,2,6,10,14,18,22};
    MV_HWS_PORT_STANDARD swapPortMode;
    MV_HWS_HAWK_ANP_UNITS field;
    MV_HWS_PORT_SERDES_TO_MAC_MUX   macToSerdesMuxStc;
    GT_U32 owValue = 0,val, sdIndex;
    GT_BOOL doRxTraining = GT_TRUE;
    GT_U32 hawkLocalIndexArr[8]    = {0,1,2,6,10,14,18,22};
    GT_U32 nextPortShift;
    MV_HWS_SERDES_CONFIG_STC serdesConfig;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAnpPortEnable ******\n");
    }
#endif


    if(((apEnable == GT_FALSE) && (enable == GT_TRUE) && (PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpUsedForStaticPort) == GT_FALSE)) || cpssDeviceRunCheck_onEmulator())
    {
        owValue = 1;
    }

    if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        unitId = ANP_CPU_UNIT;
    }
    else if(mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        if(hwsIsIronmanAsPhoenix())
        {
            return GT_OK;
        }
        unitId = ANP_400_UNIT;
    }

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    if (rc != GT_OK)
    {
        return GT_BAD_PARAM;
    }
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    for (laneIdx = 0; laneIdx < curPortParams.numOfActLanes; laneIdx++)
    {
        MV_HWS_PORT_IN_GROUP_ENT isFirst, isLast;

        if(mvHwsUsxModeCheck(devNum, portNum, portMode))
        {
            CHECK_STATUS(mvHwsExtIfFirstInSerdesGroupCheck(devNum,portNum,portMode, &isFirst));
            CHECK_STATUS(mvHwsExtIfLastInSerdesGroupCheck(devNum,portNum,portMode, &isLast));
            if((isFirst != MV_HWS_PORT_IN_GROUP_FIRST_E) && (isLast != MV_HWS_PORT_IN_GROUP_LAST_E))
            {
                break;
            }
            nextPortShift = 0;
        }
        else
        {
            if (HWS_DEV_SILICON_TYPE(devNum) == AC5P)
            {
                nextPortShift = hawkLocalIndexArr[convertIdx.ciderIndexInUnit + laneIdx] - hawkLocalIndexArr[convertIdx.ciderIndexInUnit];
            }
            else if (HWS_DEV_SILICON_TYPE(devNum) == Harrier)
            {
                nextPortShift = 2*laneIdx;
            }
            else /* AC5x */
            {
                nextPortShift = laneIdx;
            }
        }

        field = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_SD_RX_RESETN_OW_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_SD_RX_RESETN_OW_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_SD_RX_RESETN_OW_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum+nextPortShift, unitId, field,  owValue, NULL));
        field = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_SD_RX_RESETN_OW_VAL_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_SD_RX_RESETN_OW_VAL_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_SD_RX_RESETN_OW_VAL_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum+nextPortShift, unitId, field,  owValue, NULL));
        field = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_SD_TX_RESETN_OW_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_SD_TX_RESETN_OW_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_SD_TX_RESETN_OW_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum+nextPortShift, unitId, field,  owValue, NULL));
        field = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_SD_TX_RESETN_OW_VAL_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_SD_TX_RESETN_OW_VAL_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_SD_TX_RESETN_OW_VAL_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum+nextPortShift, unitId, field,  owValue, NULL));
        field = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum+nextPortShift, unitId, field,  owValue, NULL));
        field = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_VAL_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_VAL_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_VAL_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum+nextPortShift, unitId, field,  owValue, NULL));
        field = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_TX_CLK_ENA_OW_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_TX_CLK_ENA_OW_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_TX_CLK_ENA_OW_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum+nextPortShift, unitId, field,  owValue, NULL));
        field = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_TX_CLK_ENA_OW_VAL_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_TX_CLK_ENA_OW_VAL_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_TX_CLK_ENA_OW_VAL_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum+nextPortShift, unitId, field,  owValue, NULL));
        field = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL6_P0_REG_SIGDET_MODE_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL6_P0_REG_SIGDET_MODE_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_REG_SIGDET_MODE_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum+nextPortShift, unitId, field,  (1-owValue), NULL));
    }

    if (PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpUsedForStaticPort))
    {
        if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
        {
            return GT_BAD_PTR;
        }

        serdesConfig.serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[0])));

        /* CPSS_TBD_BOOKMARK_AC5P: add check first if SD group */
        for(sdIndex = 0; sdIndex < curPortParams.numOfActLanes; sdIndex++)
        {
            CHECK_STATUS(mvHwsSerdesApPowerCtrl(devNum, 0, curPortParams.activeLanesList[sdIndex], GT_TRUE, &serdesConfig));
        }

        /*  Set the desired port mode in bits[4:0]:
        /Cider/EBU-IP/GOP/EPI/ANP_IP/ANP_IP 1.0 (hawk1)/ANP_IP {RTLF 8.1 200301.0}/ANP/ANP Units/Port<%n> Control*/
        if((apEnable == GT_FALSE) )
        {
            if ( enable == GT_FALSE)
            {
                val = 0xe;
            }
            else
            {
                switch ( portMode)
                {
                case _1000Base_X:
                case SGMII:
                    val = 0;
                    doRxTraining = GT_FALSE;
                    break;
                case _2500Base_X:
                    val = 1;
                    doRxTraining = GT_FALSE;
                    break;
                case _5GBaseR:
                    val = 2;
                    break;
                case HWS_10G_MODE_CASE:
                    val = 3;
                    break;
                case HWS_25G_MODE_CASE:
                    val = 4;
                    break;
                case HWS_40G_R4_MODE_CASE:
                    val = 5;
                    break;
                case HWS_40G_R2_MODE_CASE:
                    val = 6;
                    break;
                case HWS_50G_R2_MODE_CASE:
                    val = 7;
                    break;
                case HWS_50G_PAM4_MODE_CASE:
                    val = 8;
                    break;
                case HWS_100G_R4_MODE_CASE:
                    val = 9;
                    break;
                case HWS_100G_PAM4_MODE_CASE:
                    val = 10;
                    break;
                case HWS_200G_PAM4_MODE_CASE:
                    val = 11;
                    break;
                case HWS_400G_PAM4_MODE_CASE:
                    val = 12;
                    break;
                case HWS_200G_R8_MODE_CASE:
                    val = 13;
                    break;

                /* TODO MODE_CUSTOM * /
                case _107GBase_KR4:
                case _106GBase_KR2:
                case _212GBase_KR4:
                    val = 15;
                    break;*/

                case QSGMII:
                case _5G_QUSGMII:
                    val = 16;
                    doRxTraining = GT_FALSE;
                    break;
                case _10G_OUSGMII:
                    val = 17;
                    doRxTraining = GT_FALSE;
                    break;

                case _2_5G_SXGMII:
                case _5G_DXGMII:
                case _10G_QXGMII:
                case _20G_OXGMII:
                    val = 18;
                    break;
                case _5G_SXGMII:
                case _10G_DXGMII:
                case _20G_QXGMII:
                    val = 19;
                    break;
                case _10G_SXGMII:
                case _20G_DXGMII:
                    val = 20;
                    break;
                default:
                    return GT_BAD_PARAM;
                }
            }
            field = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL_P0_PORT_MODE_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL_P0_PORT_MODE_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL_P0_PORT_MODE_E);
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, field, val, NULL));

            /*Set the desired train_type: /Cider/EBU-IP/GOP/EPI/ANP_IP/ANP_IP 1.0 (hawk1)/ANP_IP {RTLF 8.1 200301.0}/ANP/ANP Units/Port<%n> Control8
            This is done by OW. By default 10G+ train_type is “KR_TRAINING” while for other ports train_type is “NO_TRAINING”.
            There are 2 overwrites for the train_type, one is before the couple mux, and one is after, we wish to use the one before the mux,
            then need to set only for the relevant index, and not per lane.
            This overwrite is found in this register (mentioned above):
            Bit[0] – OW ; bit[2:1] – OW_VAL.
            0x0: KR_TRAINING 0x1: RX_TRAINING 0x2: NO_TRAINING 0x3: Rsvd.*/
            if ( enable == GT_FALSE)
            {
                field = mvHwsAnpFieldSet(devNum,
                                         HAWK_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_E,
                                         HARRIER_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_E,
                                         PHOENIX_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_E);
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, field, 0, NULL));
                field = mvHwsAnpFieldSet(devNum,
                                         HAWK_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_VAL_E,
                                         HARRIER_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_VAL_E,
                                         PHOENIX_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_VAL_E);
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, field, 0, NULL));
            }
            else if ( doRxTraining )
            {
                field = mvHwsAnpFieldSet(devNum,
                                         HAWK_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_E,
                                         HARRIER_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_E,
                                         PHOENIX_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_E);
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, field, 1, NULL));
                field = mvHwsAnpFieldSet(devNum,
                                         HAWK_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_VAL_E,
                                         HARRIER_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_VAL_E,
                                         PHOENIX_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_VAL_E);
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, field, 1, NULL));
            }


        }
    }


    if((hwsDeviceSpecInfo[devNum].devType == AC5X) || (hwsDeviceSpecInfo[devNum].devType == Harrier))
    {
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsAnpPortEnable ******\n");
        }
#endif
        return GT_OK;
    }

    if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
    {
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsAnpPortEnable ******\n");
        }
#endif
        return GT_OK;
    }

    /* INIT anpDB */
    /*********************************************************************** */
    /* anpIdx      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |10 |11 |12 |13 | */
    /*********************************************************************** */
    /* serdesOutCfg| 0 | 1 | 8 | 9 | 4 | 5 | 6 | 7 | 2 | 3 |10 |11 |12 |13 | */
    /*********************************************************************** */
    /* serdesOutCfg - change when port is configure according to port mode  active serdeses are 0-7*/
    if(convertIdx.ciderUnit >= 2)
    {
        /* EPI_2 and EPI_3 controlled only via SD_MUX_CFG API.
           No need to do swap bacause we have here 8-to-8 MUX and not 14-to-8 MUX */
        return GT_OK;
    }
    if (enable == GT_FALSE)
    {
        return GT_OK;
    }

    rc =  mvHwsHawkLocalIndexToAnpIndexConvert(devNum, portNum, convertIdx.ciderIndexInUnit, portMode, &anpIdx);
    if (rc != GT_OK)
    {
        return GT_BAD_PARAM;
    }

    if(HWS_DEV_SILICON_TYPE(devNum) == AC5P)
    {
        anpRegName = HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E;
    }
    else
    {
        anpRegName = HARRIER_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E;
    }
    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        if (enable == GT_TRUE)
        {
            CHECK_STATUS(mvHwsExtIfFirstInSerdesGroupCheck(devNum,portNum,portMode, &portFullConfig));
            if(portFullConfig != MV_HWS_PORT_IN_GROUP_FIRST_E)
            {
                return GT_OK;
            }
        }

        if (HWS_USX_O_MODE_CHECK(portMode))
        {
            unitId = ANP_USX_O_UNIT;
        }
        else
        {
            unitId = ANP_USX_UNIT;
        }
        swapUnitId = ANP_400_UNIT;
        swapPortMode = _25GBase_KR;
    }
    else
    {
        unitId = ANP_400_UNIT;
        swapUnitId = ANP_USX_UNIT;
        swapPortMode = _10G_QXGMII;
    }

    CHECK_STATUS(mvHwsPortLaneMacToSerdesMuxGet(devNum, 0, portNum, &macToSerdesMuxStc));

    for (laneIdx = 0; laneIdx < curPortParams.numOfActLanes; laneIdx++) {

        if (laneIdx == 0)
        {
            rc = mvUnitExtInfoGet(devNum, unitId, portNum, &unitAddr, &unitIndex, &localUnitNum);
            if((unitAddr == 0) || (rc != GT_OK) || (unitAddr == MV_HWS_SW_PTR_ENTRY_UNUSED))
            {
                return rc;
            }
        }

        anpNum = anpIdx + laneIdx;
        serdesIdx = PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSdMuxDb[convertIdx.ciderUnit][anpNum].hwSerdesIdx;
        if(macToSerdesMuxStc.enableSerdesMuxing)
        {
            serdesIdx = macToSerdesMuxStc.serdesLanes[serdesIdx];
        }
        anpSwapIdx = (anpNum < 8) ? (anpNum + 6) : (anpNum - 6);
        {
            if (unitId == ANP_400_UNIT)
            {
                switch (anpNum / 4) {
                    case 0:
                        if(HWS_DEV_SILICON_TYPE(devNum) == AC5P)
                        {
                            anpRegName = HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E + anpNum;
                        }
                        else
                        {
                            anpRegName = HARRIER_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E + anpNum;
                        }
                        break;
                    case 1:
                        if(HWS_DEV_SILICON_TYPE(devNum) == AC5P)
                        {
                            anpRegName = HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL4_E + (anpNum - 4);
                        }
                        else
                        {
                            anpRegName = HARRIER_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL4_E + (anpNum - 4);
                        }
                        break;

                    default:
                        return GT_BAD_PARAM;
                }
                anpSwapRegName = (HWS_DEV_SILICON_TYPE(devNum) == AC5P) ? HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E : HARRIER_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E;
            }else{
                switch (anpSwapIdx / 4) {
                    case 0:
                        if(HWS_DEV_SILICON_TYPE(devNum) == AC5P)
                        {
                            anpSwapRegName = HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E + anpSwapIdx;
                        }
                        else
                        {
                            anpSwapRegName = HARRIER_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E + anpSwapIdx;
                        }
                        break;
                    case 1:
                        if(HWS_DEV_SILICON_TYPE(devNum) == AC5P)
                        {
                            anpSwapRegName = HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL4_E + (anpSwapIdx - 4);
                        }
                        else
                        {
                            anpSwapRegName = HARRIER_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL4_E + (anpSwapIdx - 4);
                        }
                        break;

                    default:
                        return GT_BAD_PARAM;
                }
            }
            /*enable port*/
            rc = genUnitRegDbEntryGet(devNum, portNum, unitId, portMode, anpRegName, &regField, &convertIdx);
            if(GT_OK != rc) return rc;

            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, unitAddr + regField.regOffset, regField.fieldStart,
                                                 regField.fieldLen, serdesIdx));

            if (anpNum >= 2)
            {
                serdesSwapIdx = PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSdMuxDb[convertIdx.ciderUnit][anpSwapIdx].outSerdesIdx;
                swapPortNum = anp2portConvert[anpSwapIdx] + MV_HWS_AC5P_GOP_PORT_NUM_CNS * convertIdx.ciderUnit;
                rc = mvUnitExtInfoGet(devNum, swapUnitId, swapPortNum, &swapUnitAddr, &unitIndex, &localUnitNum);
                if((swapUnitAddr == 0) || (rc != GT_OK) || (swapUnitAddr == MV_HWS_SW_PTR_ENTRY_UNUSED))
                {
                    return rc;
                }
                /*disable port*/
                rc = genUnitRegDbEntryGet(devNum, swapPortNum, swapUnitId, swapPortMode, anpSwapRegName, &regField, &convertIdx);
                if(GT_OK != rc) return rc;

                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, swapUnitAddr + regField.regOffset, regField.fieldStart,
                                                     regField.fieldLen, serdesSwapIdx));
            }
        }
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAnpPortEnable ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsAnpPortReset function
* @endinternal
*
* @brief   reset anp machine
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] reset              - reset parameter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortReset
(
    GT_U8                devNum,
    GT_U32               portNum,
    GT_BOOL              reset
)
{
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32          fieldNameHw, fieldNameSw, localIdx, data;
    MV_HWS_UNITS_ID unitId;
    GT_STATUS rc;
    MV_HWS_PORT_STANDARD portMode = _10GBase_KR;

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    if (rc != GT_OK)
    {
        return GT_BAD_PARAM;
    }
    localIdx = convertIdx.ciderIndexInUnit %8;

    if (!HWS_AP_MODE_CHECK(portMode))
    {
         return GT_OK;
    }
    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, portNum, portMode)) {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
    }
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAnpPortReset port %d ******\n",portNum);
    }
#endif

    data = (reset == GT_FALSE)?1:0;
    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        MV_HWS_SET_FIELD_IDX0_3_MAC(fieldNameHw, localIdx, PHOENIX_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_P, _AN_HW_SOFT_RESET__E);
        MV_HWS_SET_FIELD_IDX0_3_MAC(fieldNameSw, localIdx, PHOENIX_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_P, _AN_SW_SOFT_RESET__E);
    }
    else if((HWS_DEV_SILICON_TYPE(devNum) == AC5P))
    {
        MV_HWS_SET_FIELD_IDX0_7_MAC(fieldNameHw, localIdx, HAWK_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_P, _AN_HW_SOFT_RESET__E);
        MV_HWS_SET_FIELD_IDX0_7_MAC(fieldNameSw, localIdx, HAWK_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_P, _AN_SW_SOFT_RESET__E);
    }
    else if((HWS_DEV_SILICON_TYPE(devNum) == Harrier))
    {
        MV_HWS_SET_FIELD_IDX0_7_MAC(fieldNameHw, localIdx, HARRIER_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_P, _AN_HW_SOFT_RESET__E);
        MV_HWS_SET_FIELD_IDX0_7_MAC(fieldNameSw, localIdx, HARRIER_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_P, _AN_SW_SOFT_RESET__E);
    }
    else
    {
        return GT_OK;
    }
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldNameHw,  0, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldNameSw,  0, NULL));

    if ( reset == GT_FALSE) {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldNameHw,  data, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldNameSw,  data, NULL));
    }
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAnpPortReset port %d ******\n",portNum);
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsAnpPortCapabilitiesConsortiumSet function
* @endinternal
*
* @brief   set anp capability first register
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] unitId             - unit id
* @param[in] localApCfgPtr      - port ap parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS mvHwsAnpPortCapabilitiesConsortiumSet
(
    GT_U8               devNum,
    GT_U32              portNum,
    MV_HWS_UNITS_ID     unitId,
    MV_HWS_AP_CFG       *localApCfgPtr
)
{
    GT_U32          fieldName;
    GT_U32          fecEnable;

    if (AP_CTRL_25GBase_KR_CONSORTIUM_GET(localApCfgPtr->modesVector))
    {
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_20_CONSORTIUM_ET_REGISTER_1_REG20_CONSORTIUM_25G_KR1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }

    if (AP_CTRL_25GBase_CR_CONSORTIUM_GET(localApCfgPtr->modesVector))
    {
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_20_CONSORTIUM_ET_REGISTER_1_REG20_CONSORTIUM_25G_CR1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }

    if (AP_CTRL_50GBase_KR2_CONSORTIUM_GET(localApCfgPtr->modesVector))
    {
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_20_CONSORTIUM_ET_REGISTER_1_REG20_CONSORTIUM_50G_KR2_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }

    if (AP_CTRL_50GBase_CR2_CONSORTIUM_GET(localApCfgPtr->modesVector))
    {
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_20_CONSORTIUM_ET_REGISTER_1_REG20_CONSORTIUM_50G_CR2_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }

    if (AP_CTRL_40GBase_KR2_GET(localApCfgPtr->modesVector))
    {
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_MARVELL_CONSORTIUM_40GR2_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }

    if (AP_CTRL_400GBase_KR8_GET(localApCfgPtr->modesVector) || AP_CTRL_400GBase_CR8_GET(localApCfgPtr->modesVector))
    {
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_MARVELL_CONSORTIUM_400GR8_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }

    /*  F1 advertises Clause 91 FEC ability (RS-FEC)
        F2 advertises Clause 74 FEC ability (BASE-R FEC)
        F3 requests Clause 91 FEC (RS-FEC)
        F4 requests Clause 74 FEC (BASE-R FEC) */
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_CONSORTIUM_F1_E;
    fecEnable =  ((localApCfgPtr->fecAdvanceAbil >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_RS)? 1:0;
    if ( fecEnable )
    {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, fecEnable, NULL));
    }

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_CONSORTIUM_F2_E;
    fecEnable =  ((localApCfgPtr->fecAdvanceAbil >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_FC)? 1:0;
    if ( fecEnable )
    {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, fecEnable, NULL));
    }

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_CONSORTIUM_F3_E;
    fecEnable =  ((localApCfgPtr->fecAdvanceReq >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_RS)? 1:0;
    if ( fecEnable )
    {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, fecEnable, NULL));
    }

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_CONSORTIUM_F4_E;
    fecEnable =  ((localApCfgPtr->fecAdvanceReq >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_FC)? 1:0;
    if ( fecEnable )
    {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, fecEnable, NULL));
    }
    return GT_OK;
}

/**
* @internal mvHwsAnpPortCapabilitiesReg1Set function
* @endinternal
*
* @brief   set anp capability first register
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] unitId             - unit id
* @param[in] localApCfgPtr      - port ap parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS mvHwsAnpPortCapabilitiesReg1Set
(
    GT_U8               devNum,
    GT_U32              portNum,
    MV_HWS_UNITS_ID     unitId,
    MV_HWS_AP_CFG       *localApCfgPtr
)
{
    GT_U32          fieldName;
    if (AP_CTRL_ADV_CONSORTIUM_GET(localApCfgPtr->modesVector))
    {
        /* this bit is used only for SW support.
           for HW support we only set consortium registers
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1_LD_NEXT_PAGE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));*/

        /*TODO add consortium support*/
    }

    if ( localApCfgPtr->fcPause& 0x1 ) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1_LD_PAUSE_CAPABLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }

    if ( localApCfgPtr->fcAsmDir& 0x1 ) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1_LD_ASYMMETRIC_PAUSE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    return GT_OK;
}

/**
* @internal mvHwsAnpPortCapabilitiesReg2Set function
* @endinternal
*
* @brief   set anp capability second register
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] unitId             - unit id
* @param[in] localApCfgPtr      - port ap parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS mvHwsAnpPortCapabilitiesReg2Set
(
    GT_U8               devNum,
    GT_U32              portNum,
    MV_HWS_UNITS_ID     unitId,
    MV_HWS_AP_CFG       *localApCfgPtr
)
{
    GT_U32          fieldName;
    GT_U32          enable;
    /*abilityReg2 =
        (nonceUserPattern&0x1f)                               |
        (AP_CTRL_1000Base_KX_GET(localApCfg->modesVector)  << 5 ) |
        (AP_CTRL_10GBase_KX4_GET(localApCfg->modesVector)  << 6 ) |
        (AP_CTRL_10GBase_KR_GET(localApCfg->modesVector)   << 7 ) |
        (AP_CTRL_40GBase_KR4_GET(localApCfg->modesVector)  << 8 ) |
        (AP_CTRL_40GBase_CR4_GET(localApCfg->modesVector)  << 9 ) |
        (AP_CTRL_100GBase_KR4_GET(localApCfg->modesVector) << 12) |
        (AP_CTRL_100GBase_CR4_GET(localApCfg->modesVector) << 13) |
        (AP_CTRL_25GBase_KR1S_GET(localApCfg->modesVector) << 14) |
        (AP_CTRL_25GBase_CR1S_GET(localApCfg->modesVector) << 14) |
        (AP_CTRL_25GBase_KR1_GET(localApCfg->modesVector)  << 15) |
        (AP_CTRL_25GBase_CR1_GET(localApCfg->modesVector)  << 15);*/

    /* in case of 1000BaseX the default wrong so we need to unset*/
    fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_1000BASE_KX__E;
    enable = (AP_CTRL_1000Base_KX_GET(localApCfgPtr->modesVector))? 1:0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, enable, NULL));


    if ( AP_CTRL_10GBase_KX4_GET(localApCfgPtr->modesVector) ) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_10GBASE_KX4_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }

    /* in case of 10GBaseKR the default wrong so we need to unset*/
    fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_10GBASE_KR_E;
    enable = (AP_CTRL_10GBase_KR_GET(localApCfgPtr->modesVector))? 1:0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, enable, NULL));

    if ( AP_CTRL_40GBase_KR4_GET(localApCfgPtr->modesVector) ) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_40GBASE_KR4_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    if ( AP_CTRL_40GBase_CR4_GET(localApCfgPtr->modesVector) ) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_40GBASE_CR4_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    if ( AP_CTRL_100GBase_KR4_GET(localApCfgPtr->modesVector) ) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_100GBASE_KR4_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    if ( AP_CTRL_100GBase_CR4_GET(localApCfgPtr->modesVector) ) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_100GBASE_CR4_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    if ( (AP_CTRL_25GBase_KR1S_GET(localApCfgPtr->modesVector)) || (AP_CTRL_25GBase_CR1S_GET(localApCfgPtr->modesVector)) ) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_25GBASE_KR_S_OR_25GBASE_CR_S_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    if ( (AP_CTRL_25GBase_KR1_GET(localApCfgPtr->modesVector)) || (AP_CTRL_25GBase_CR1_GET(localApCfgPtr->modesVector)) ) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_25GBASE_KR_OR_25GBASE_CR_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    return GT_OK;
}

/**
* @internal mvHwsAnpPortCapabilitiesReg3Set function
* @endinternal
*
* @brief   set anp capability third register
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] unitId             - unit id
* @param[in] localApCfgPtr      - port ap parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS mvHwsAnpPortCapabilitiesReg3Set
(
    GT_U8               devNum,
    GT_U32              portNum,
    MV_HWS_UNITS_ID     unitId,
    MV_HWS_AP_CFG       *localApCfgPtr
)
{
    GT_U32          fieldName;
   /* f0 = localApCfg->fecSup&0x1;             / * fecAbility bit 46*/
   /* f1 = localApCfg->fecReq&0x1;             / * 10G FEC bit 47*/
   /* f2 = (localApCfg->fecAdvanceReq>>1)&0x1; / * 25G and above RS fec bit 44*/
   /* f3 = localApCfg->fecAdvanceReq&0x1;      / * 25G and above FC fec bit 45*/

   /* abilityReg3 =
        (AP_CTRL_50GBase_KR1_GET(localApCfg->modesVector)  << 2) |
        (AP_CTRL_50GBase_CR1_GET(localApCfg->modesVector)  << 2) |
        (AP_CTRL_100GBase_KR2_GET(localApCfg->modesVector) << 3) |
        (AP_CTRL_100GBase_CR2_GET(localApCfg->modesVector) << 3) |
        (AP_CTRL_200GBase_KR4_GET(localApCfg->modesVector) << 4) |
        (AP_CTRL_200GBase_CR4_GET(localApCfg->modesVector) << 4) |
        (f2<<12)                                                 |
        (f3<<13)                                                 |
        (f0<<14)                                                 |
        (f1<<15);*/

    if ( (AP_CTRL_50GBase_KR1_GET(localApCfgPtr->modesVector)) || (AP_CTRL_50GBase_CR1_GET(localApCfgPtr->modesVector)) ) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_50G_KRCR_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    if ( (AP_CTRL_100GBase_KR2_GET(localApCfgPtr->modesVector)) ||  (AP_CTRL_100GBase_CR2_GET(localApCfgPtr->modesVector))) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_100G_KRCR2_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    if ( (AP_CTRL_200GBase_KR4_GET(localApCfgPtr->modesVector)) ||  (AP_CTRL_200GBase_CR4_GET(localApCfgPtr->modesVector))) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_200G_KRCR4_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    if ( (AP_CTRL_200GBase_KR8_GET(localApCfgPtr->modesVector)) ||  (AP_CTRL_200GBase_CR8_GET(localApCfgPtr->modesVector))) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_200G_KRCR8_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }

    if ( localApCfgPtr->fecSup&0x1 ) {

        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_LINK_PARTNER_FEC_ABILITY_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    if ( localApCfgPtr->fecReq&0x1 ) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_LINK_PARTNER_REQUESTING_FEC_ENABLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    if ( localApCfgPtr->fecAdvanceReq&0x2 ) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_25G_RS_FEC_REQUESTEDF2_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    if ( localApCfgPtr->fecAdvanceReq&0x1 ) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_25G_BASE_R_REQUESTEDF3_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    return GT_OK;
}

/**
* @internal mvHwsAnpPortCapabilities function
* @endinternal
*
* @brief   set anp capabilities
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] localApCfgPtr      - port ap parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS mvHwsAnpPortCapabilities
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_AP_CFG       *localApCfg
)
{
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32          fieldName, data;
    MV_HWS_UNITS_ID unitId, unitIdAn;
    GT_STATUS rc;
    MV_HWS_PORT_STANDARD portMode = _10GBase_KR;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }
    HWS_NULL_PTR_CHECK_MAC(localApCfg);
    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    if (rc != GT_OK)
    {
        return GT_BAD_PARAM;
    }
    if (!HWS_AP_MODE_CHECK(portMode))
    {
         return GT_OK;
    }
    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
        unitIdAn = AN_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, portNum, portMode)) {
        unitId = ANP_USX_UNIT;
        unitIdAn = AN_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
        unitIdAn = AN_400_UNIT;
    }
    PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum][portNum] = *localApCfg;
    /* Setting the 40GBASE-R2 priority to be higher than the 40GBASE-R4.
    m_RAL.m_RAL_AN[n].an_units_RegFile.ANEG_LANE_0_CONTROL_REGISTER_14.cfg_40gr2_prio_higher_than_40gr4_s.set(1);*/
   /* if(hwsDeviceSpecInfo[devNum].devType == AC5X)*/
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_14_CFG_40GR2_PRIO_HIGHER_THAN_40GR4_S_E;
    data = 1;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdAn, fieldName,  data, NULL));

    if ( localApCfg->nonceDis )
    {
        /*m_RAL.m_RAL_AN[n].an_units_RegFile.ANEG_LANE_0_CONTROL_REGISTER_1.ow_as_nonce_match_s.set(1);
          m_RAL.m_RAL_AN[n].an_units_RegFile.ANEG_LANE_0_CONTROL_REGISTER_1.rg_as_nonce_match_s.set(0); */
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_AS_NONCE_MATCH_S_E;
        data = 0;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdAn, fieldName,  data, NULL));
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_OW_AS_NONCE_MATCH_S_E;
        data = 1;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdAn, fieldName,  data, NULL));
    }

    /* set consortum abilities*/
    if ( AP_CTRL_ADV_CONSORTIUM_GET(localApCfg->modesVector) )
    {
        mvHwsAnpPortCapabilitiesConsortiumSet(devNum, portNum, unitIdAn, localApCfg);
    }
    /* Writing to the Base page registers = 3 registers of 16 bit each.
    m_RAL.m_RAL_AN[n].an_units_RegFile.REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3.write(status, bp[47:32]);
    m_RAL.m_RAL_AN[n].an_units_RegFile.REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2.write(status, bp[31:16]);
    m_RAL.m_RAL_AN[n].an_units_RegFile.REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1.write(status, bp[15:0]); */
    mvHwsAnpPortCapabilitiesReg3Set(devNum, portNum, unitIdAn, localApCfg);
    mvHwsAnpPortCapabilitiesReg2Set(devNum, portNum, unitIdAn, localApCfg);
    mvHwsAnpPortCapabilitiesReg1Set(devNum, portNum, unitIdAn, localApCfg);


    /* Write ENABLE AN
    m_RAL.m_RAL_AN[n].an_units_RegFile.REG_802_3AP_AUTO_NEGOTIATION_CONTROL.field_802_3ap_auto_negotiation_enable.set(1);*/
    fieldName = AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_CONTROL_FIELD_802_3AP_AUTO_NEGOTIATION_ENABLE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdAn, fieldName, 1, NULL));

    /* change cfg_done to clear upon restart */
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL6_P0_PCS_CFG_DONE_HW_CLR_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL6_P0_PCS_CFG_DONE_HW_CLR_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_PCS_CFG_DONE_HW_CLR_E);
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));

    return GT_OK;
}

/**
* @internal mvHwsAnpEmulatorForceHCDComplete function
* @endinternal
*
* @brief   debug function that run only in emulator in order to
*          find resolution
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpEmulatorForceHCDComplete
(
    GT_U8                devNum,
    GT_U32               portNum
)
{
    MV_HWS_UNITS_ID unitId;
    MV_HWS_PORT_STANDARD portMode = _10GBase_KR;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
{
    return GT_BAD_PARAM;
}

    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, portNum, portMode)) {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
    }

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_AN_CONTROL_P0_PM_ENCLK_AP_FR_OW_E,        1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_AN_CONTROL_P0_PM_ENCLK_AP_FR_OW_VAL_E,    1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_AN_CONTROL_P0_PM_ENCLK_AP_FT_OW_E,        1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_AN_CONTROL_P0_PM_ENCLK_AP_FT_OW_VAL_E,    1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_AN_CONTROL_P0_RG_BREAK_LINK_TIMER_FAST_E, 1, NULL));

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL7_P0_PLL_READY_RX_CLEAN_OW_E,      1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL7_P0_PLL_READY_RX_CLEAN_OW_VAL_E,  1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL7_P0_PLL_READY_TX_CLEAN_OW_E,      1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL7_P0_PLL_READY_TX_CLEAN_OW_VAL_E,  1, NULL));

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_SQ_DETECTED_LPF_OW_E,         1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_SQ_DETECTED_LPF_OW_VAL_E,     0, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_RX_INIT_DONE_OW_E,            1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_RX_INIT_DONE_OW_VAL_E,        1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_RX_TRAIN_COMPLETE_OW_E,       1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_RX_TRAIN_COMPLETE_OW_VAL_E,   1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_RX_TRAIN_FAILED_OW_E,         1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_RX_TRAIN_FAILED_OW_VAL_E,     0, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_TX_TRAIN_COMPLETE_OW_E,       1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_TX_TRAIN_COMPLETE_OW_VAL_E,   1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_TX_TRAIN_FAILED_OW_E,         1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_TX_TRAIN_FAILED_OW_VAL_E,     0, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_PM_DSP_TXDN_ACK_OW_E,         1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_PM_DSP_TXDN_ACK_OW_VAL_E,     1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_PM_DSP_RXDN_ACK_OW_E,         1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_PM_DSP_RXDN_ACK_OW_VAL_E,     1, NULL));
    return GT_OK;
}

/**
* @internal mvHwsAnpEmulatorOwHCDResolution function
* @endinternal
*
* @brief   debug function that used to force resolution
*
* @param[in] devNum          - system device number
* @param[in] portNum         - Physical Port Number
* @param[in] portMode        - required port mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpEmulatorOwHCDResolution
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_PORT_STANDARD portMode
)
{
    MV_HWS_UNITS_ID unitId, unitIdAnp;
    GT_U32          baseAddr,unitIndex, unitNum,regAddr;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32          fieldName,mode;
    GT_U32 regData;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }
    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE )
    {
        unitId = AN_CPU_UNIT;
        unitIdAnp = ANP_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, portNum, portMode)) {
        unitId = AN_USX_UNIT;
        unitIdAnp = ANP_USX_UNIT;
    }
    else
    {
        unitId = AN_400_UNIT;
        unitIdAnp = ANP_400_UNIT;
    }
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAnpEmulatorOwHCDResulotion port %d ******\n",portNum);
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
    }
#endif

        /*dbg register write device 0 offset 0xA7E06800 data 0x1005*/
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0_OVERRIDE_CTRL_S_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0_AP_ANEG_AMDISAM_S_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0_AP_ANEG_STATE_S10_0_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 0x100, NULL));

    /*SW final_ag_mode; 00000 : 1GKX;  00001 : 10GKR; 00010 : 2P5GKX;  00100 : 5GKR;
    00101 : 40GCR4; 00110 : 40GKR4;00111 : IEEE 25GKRCRS; 01000 : CONS 25GKR;
    01001 : CONS 25GCR; 01010 : 100GCR4;01011 : 100GKR4; 01101 : CON 50GKR;
    01110 : CON 50GCR; 01111 : IEEE25GKRCR; 10010 : 50GKRCR; 10011 : 100GKRCR2;
    10100 : 200GKRCR4; 10101 : Marvell IEEE 25GR4;10110 : Marvell IEEE 50GR4;10111 : Marvell IEEE 25GR2;11000 : Marvell Consortium 40GRw;others : reserved*/
    switch ( portMode )
    {
        case _1000Base_X:
            mode = 0;break;
        case _10GBase_KR:
            mode = 1;break;
        /*case 2P5GKX:
            mode = 2;break;*/
        case _5GBaseR:
            mode = 4;break;
        case _40GBase_CR4:
            mode = 5;break;
        case _40GBase_KR4:
            mode = 6;break;
        case _25GBase_KR_S:
        case _25GBase_CR_S:
            mode = 7;break;
        case _25GBase_KR_C:
            mode = 8;break;
        case _25GBase_CR_C:
            mode = 9;break;
        case _100GBase_CR4:
            mode = 10;break;
        case _100GBase_KR4:
            mode = 11;break;
        case _50GBase_KR2_C:
            mode = 13;break;
        case _50GBase_CR2_C:
            mode = 14;break;
        case _25GBase_KR:
        case _25GBase_CR:
            mode = 15;break;
        case _50GBase_KR:
        case _50GBase_CR:
            mode = 18;break;
        case _100GBase_KR2:
        case _100GBase_CR2:
            mode = 19;break;
        case _200GBase_KR4:
        case _200GBase_CR4:
            mode = 20;break;
        default:
            return GT_BAD_PARAM;
    }

    /* register write device 0 offset 0xA7E04824 data 0xa08f*/
    mode |=0xa080;
    CHECK_STATUS(mvUnitExtInfoGet(devNum, unitId, (portNum), &baseAddr, &unitIndex, &unitNum ));
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_9_SW_AG_MODE_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, portNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT))
    {
        convertIdx.ciderIndexInUnit = 0;
    }
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 29, mode));

    if(CPSS_DEV_IS_WM_NATIVE_OR_ASIM_MAC(devNum))
    {
        switch (portMode)
        {
            /*case (1>>0(): / * 10KX4* /
                break;*/
            case _1000Base_X:
                regData = (1<<1);
                break;
            case _2500Base_X:
                regData = (1<<2);
                break;
            case _5GBaseR:
                regData = (1<<3);
                break;
            case _10GBase_KR:
                regData = (1<<4);
                break;
            case _25GBase_KR_S:
                regData = (1<<5);
                break;
            case _25GBase_CR_S:
                regData = (1<<5);
                break ;
            case _25GBase_KR:
                regData = (1<<6);
                break;
            case _25GBase_CR:
                regData = (1<<6);
                break ;
            case _25GBase_KR_C:
                regData = (1<<7);
                break;
            case _25GBase_CR_C:
                regData = (1<<8);
                break;
            case _40GBase_KR4:
                regData = (1<<9);
                break;
            case _40GBase_CR4:
                regData = (1<<10);
                break;
            case _50GBase_KR2_C:
                regData = (1<<11);
                break;
            case _50GBase_CR2_C:
                regData = (1<<12);
                break;
            case _100GBase_CR4:
                regData = (1<<13);
                break;
            /*case _100GBase_CR10:
                regData = (1<<14);
                break;*/
            case _100GBase_KR4:
                regData = (1<<15);
                break;
           /* case _100GBase_KP4:
                regData = (1<<16);
                break;*/
            case _200GBase_KR8:
                regData = (1<<17);
                break;
            case _200GBase_CR8:
                regData = (1<<17);
                break ;
            case _25GBase_KR2:
                regData = (1<<18);
                break;
            case _40GBase_KR2:
                /*if (AP_CTRL_40GBase_KR2_GET(mvHwsAnpPortParam[devNum][phyPortNum]))*/
                    regData = (1<<19);
                /*else
                    regData = _40GBase_CR2;*/
                break ;
            case _50GBase_KR4:
                regData = (1<<20);
                break;
            case _50GBase_KR:
                regData = (1<<21);
                break;
            case _50GBase_CR:
                regData = (1<<21);
                break;
            case _100GBase_KR2:
                regData = (1<<22);
                break;
            case _100GBase_CR2:
                regData = (1<<22);
                break;
            case _200GBase_KR4:
                regData = (1<<23);
                break;
            case _200GBase_CR4:
                regData = (1<<23);
                break;
            /*case (1<<24):
                regData = _100GBase_KR/CR;
                break;
            case (1<<25):
                regData = _200GBase_KR2/CR2;
                break;
            case (1<<26):
                regData = _400GBase_KR4/CR4;
                break;
            case (1<<27):
                regData = _800GBase_KR8/CR8;
                break;*/
            case _400GBase_KR8:
                regData = (1<<28);
                break;
            case _400GBase_CR8:
                regData = (1<<28);
                break;
            default:
                return GT_FAIL;
        }

        CHECK_STATUS(mvUnitExtInfoGet(devNum, unitIdAnp, portNum, &baseAddr, &unitIndex, &unitNum ));
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_AN_STATUS1_P0_AP_PWRUP_4X_S_E,
                                     HARRIER_ANP_UNITS_PORT0_AN_STATUS1_P0_AP_PWRUP_4X_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_AN_STATUS1_P0_AP_PWRUP_4X_S_E);
        CHECK_STATUS(genUnitRegDbEntryGet(devNum, portNum, unitIdAnp, portMode, fieldName, &fieldReg, &convertIdx));
        if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT))
        {
            convertIdx.ciderIndexInUnit = 0;
        }
        regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 29, regData));


        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_HCD_FOUND_E,
                                     HARRIER_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_HCD_FOUND_E,
                                     PHOENIX_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_HCD_FOUND_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdAnp, fieldName, 0x1, NULL));
    }


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAnpEmulatorOwHCDResulotion port %d ******\n",portNum);
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
    }
#endif

    return GT_OK;
}


/**
* @internal mvHwsAnpPortInteropGet function
* @endinternal
*
* @brief   Return ANP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] apInteropPtr             - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortInteropGet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_AP_PORT_INTROP  *apInteropPtr
)
{
    GT_U32 timeInMsecPam4,timeInMsecNrz,timeInMsecSlow;
    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }
    hwsOsMemSetFuncPtr(apInteropPtr, 0, sizeof(MV_HWS_AP_PORT_INTROP));
    CHECK_STATUS(mvHwsAnpPortLinkTimerGet(devNum, phyPortNum, _1000Base_X, &timeInMsecSlow));
    CHECK_STATUS(mvHwsAnpPortLinkTimerGet(devNum, phyPortNum, _10GBase_KR, &timeInMsecNrz));
    CHECK_STATUS(mvHwsAnpPortLinkTimerGet(devNum, phyPortNum, _50GBase_KR, &timeInMsecPam4));
    apInteropPtr->apLinkDuration = 1;
    apInteropPtr->apLinkMaxInterval = timeInMsecNrz;
    apInteropPtr->pdLinkDuration = 1;
    apInteropPtr->pdLinkMaxInterval = timeInMsecSlow;
    apInteropPtr->anPam4LinkMaxInterval = timeInMsecPam4;
    /*TODO add support*/
    /*txDisDuration;
      abilityDuration;
      abilityMaxInterval;
      abilityFailMaxInterval;*/

    return GT_OK;
}

/**
* @internal mvHwsAnpPortLinkTimerSet function
* @endinternal
*
* @brief   set link up timer.
*
* @param[in] devNum          - system device number
* @param[in] portNum         - Physical Port Number
* @param[in] portMode        - required port mode
* @param[in] timeInMsec      - link up timeout.
*   supported timeout: slow speed - 50ms,100ms,200ms,500ms.
*                      nrz- 500ms, 1s, 2s, 5s.
*                      pam4- 3150ms, 6s, 9s, 12s.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortLinkTimerSet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  timeInMsec
)
{
    GT_U32  value;
#if 0
    MV_HWS_UNITS_ID unitIdAn;

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitIdAn = AN_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, portMode)) {
        unitIdAn = AN_USX_UNIT;
    }
    else
    {
        unitIdAn = AN_400_UNIT;
    }
#endif

    /* disable ap timer*/
    if (timeInMsec == MV_HWS_ANP_TIMER_DISABLE_CNS)
    {
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_SET(mvHwsAnpTimerDisable, 1);
        return GT_OK;
    }
    else if (PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpTimerDisable) == 1)
    {
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_SET(mvHwsAnpTimerDisable, 2);
    }
    else
    {
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_SET(mvHwsAnpTimerDisable, 0);
    }

    if ( HWS_PAM4_MODE_CHECK(portMode) )
    {
        /*  Def_3150ms is set according to the configuration "rg_link_fail_timer_sel_s[1:0]":
            2'b00: 3150ms. 2'b01: 6s. 2'b10: 9s. 2'b11: 12s.
            Default is 2'b00.
            In order to change, set desired value in bits[10:9] of following AN register:
            /Cider/External IP/Soft IP/ANP/AN/AN 1.0 (hawk1)/AN {RTLF 8.1 200120.0}/AN/AN Units/ANEG_LANE_0_CONTROL_1 */
        if ( timeInMsec <= 3200) {
            value = 0;
        }
        else if ( timeInMsec <= 6000 ) {
            value = 1;
        }
        else if ( timeInMsec <= 9000 ) {
            value = 2;
        }
        else /*if ( timeInMsec < 6000 ) */{
            value = 3;
        }
        /*fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL1500_S_E;*/

        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum][phyPortNum].interopLinkTimer &= ~0x3;
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum][phyPortNum].interopLinkTimer |= value;
    }
    else if ( portMode ==_1000Base_X)
    {
        /*Def_50ms is set according to the configuration "rg_link_fail_timer_sel_s[5:4]":
            2'b00: 50ms. 2'b01: 100ms. 2'b10: 200ms. 2'b11: 500ms. Default is 2'b00.
            In order to change, set desired value in bits[14:13] of following AN register:
            /Cider/External IP/Soft IP/ANP/AN/AN 1.0 (hawk1)/AN {RTLF 8.1 200120.0}/AN/AN Units/ANEG_LANE_0_CONTROL_1*/
        if ( timeInMsec <= 50) {
            value = 0;
        }
        else if ( timeInMsec <= 100 ) {
            value = 1;
        }
        else if ( timeInMsec <= 200 ) {
            value = 2;
        }
        else /*if ( timeInMsec <= 500 ) */{
            value = 3;
        }
        /*fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL50_S_E;*/

        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum][phyPortNum].interopLinkTimer &= ~(0x3 << 4);
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum][phyPortNum].interopLinkTimer |= (value << 4);
    }
    else
    {
        /*Def_500ms is set according to the configuration "rg_link_fail_timer_sel_s[3:2]":
            2'b00: 500ms. 2'b01: 1s. 2'b10: 2s. 2'b11: 5s. Default is 2'b00.
            In order to change, set desired value in bits[12:11] of following AN register:
            /Cider/External IP/Soft IP/ANP/AN/AN 1.0 (hawk1)/AN {RTLF 8.1 200120.0}/AN/AN Units/ANEG_LANE_0_CONTROL_1*/
        if ( timeInMsec <= 500) {
            value = 0;
        }
        else if ( timeInMsec <= 1000 ) {
            value = 1;
        }
        else if ( timeInMsec <= 2000 ) {
            value = 2;
        }
        else /*if ( timeInMsec <= 5000 ) */{
            value = 3;
        }
        /*fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL500_S_E;*/

        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum][phyPortNum].interopLinkTimer &= ~(0x3 << 2);
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum][phyPortNum].interopLinkTimer |= (value << 2);
    }

#if 0
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitIdAn, fieldName, value, NULL));
#endif
    return GT_OK;
}

/**
* @internal mvHwsAnpPortLinkTimerGet function
* @endinternal
*
* @brief   set link up timer.
*
* @param[in] devNum          - system device number
* @param[in] portNum         - Physical Port Number
* @param[in] portMode        - required port mode
* @param[in] timeInMsec      - link up timeout.
*   supported timeout: slow speed - 50ms,100ms,200ms,500ms.
*                      nrz- 500ms, 1s, 2s, 5s.
*                      pam4- 3150ms, 6s, 9s, 12s.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortLinkTimerGet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *timeInMsecPtr
)
{
    GT_U32          value;

    if ( HWS_PAM4_MODE_CHECK(portMode))
    {
        /*  Def_3150ms is set according to the configuration "rg_link_fail_timer_sel_s[1:0]":
            2'b00: 3150ms. 2'b01: 6s. 2'b10: 9s. 2'b11: 12s.
            Default is 2'b00.
            In order to change, set desired value in bits[10:9] of following AN register:
            /Cider/External IP/Soft IP/ANP/AN/AN 1.0 (hawk1)/AN {RTLF 8.1 200120.0}/AN/AN Units/ANEG_LANE_0_CONTROL_1 */

        /*fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL1500_S_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitIdAn, fieldName, &value, NULL));*/

        value = PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum][phyPortNum].interopLinkTimer & 0x3;

        switch ( value) {
        case 1:
            *timeInMsecPtr = 6000; break;
        case 2:
            *timeInMsecPtr = 9000; break;
        case 3:
            *timeInMsecPtr = 12000; break;
        case 0:
        default:
            *timeInMsecPtr = 3200; break;
        }
    }
    else if ( portMode ==_1000Base_X)
    {
        /*Def_50ms is set according to the configuration "rg_link_fail_timer_sel_s[5:4]":
            2'b00: 50ms. 2'b01: 100ms. 2'b10: 200ms. 2'b11: 500ms. Default is 2'b00.
            In order to change, set desired value in bits[14:13] of following AN register:
            /Cider/External IP/Soft IP/ANP/AN/AN 1.0 (hawk1)/AN {RTLF 8.1 200120.0}/AN/AN Units/ANEG_LANE_0_CONTROL_1*/

        /*fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL50_S_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitIdAn, fieldName, &value, NULL));*/

        value = (PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum][phyPortNum].interopLinkTimer >> 4) & 0x3;

        switch ( value) {
        case 1:
            *timeInMsecPtr = 100; break;
        case 2:
            *timeInMsecPtr = 200; break;
        case 3:
            *timeInMsecPtr = 500; break;
        case 0:
        default:
            *timeInMsecPtr = 50; break;
        }
    }
    else
    {
        /*Def_500ms is set according to the configuration "rg_link_fail_timer_sel_s[3:2]":
            2'b00: 500ms. 2'b01: 1s. 2'b10: 2s. 2'b11: 5s. Default is 2'b00.
            In order to change, set desired value in bits[12:11] of following AN register:
            /Cider/External IP/Soft IP/ANP/AN/AN 1.0 (hawk1)/AN {RTLF 8.1 200120.0}/AN/AN Units/ANEG_LANE_0_CONTROL_1*/

        /*fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL500_S_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitIdAn, fieldName, &value, NULL));*/

        value = (PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum][phyPortNum].interopLinkTimer >> 2) & 0x3;

        switch ( value) {
        case 1:
            *timeInMsecPtr = 1000; break;
        case 2:
            *timeInMsecPtr = 2000; break;
        case 3:
            *timeInMsecPtr = 5000; break;
        case 0:
        default:
            *timeInMsecPtr = 500; break;
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsAnpPortInteropSet function
* @endinternal
*
* @brief   Set ANP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] apInteropPtr             - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortInteropSet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_AP_PORT_INTROP  *apInteropPtr
)
{
    GT_U32 timeInMsec;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (apInteropPtr->attrBitMask & (AP_PORT_INTROP_AP_LINK_DUR | PD_PORT_INTROP_AP_LINK_DUR | AN_PAM4_PORT_INTROP_AP_LINK_DUR))
    {
        return GT_NOT_SUPPORTED;
    }

    if (apInteropPtr->attrBitMask & AP_PORT_INTROP_AP_LINK_MAX_INT)
    {
        timeInMsec = apInteropPtr->apLinkMaxInterval;
        CHECK_STATUS(mvHwsAnpPortLinkTimerSet(devNum, phyPortNum, _10GBase_KR, timeInMsec));
    }
    if (apInteropPtr->attrBitMask & PD_PORT_INTROP_AP_LINK_MAX_INT)
    {
        timeInMsec = apInteropPtr->pdLinkMaxInterval;
        CHECK_STATUS(mvHwsAnpPortLinkTimerSet(devNum, phyPortNum, _1000Base_X, timeInMsec));
    }
    if (apInteropPtr->attrBitMask & AN_PAM4_PORT_INTROP_AP_LINK_MAX_INT)
    {
        timeInMsec = apInteropPtr->anPam4LinkMaxInterval;
        CHECK_STATUS(mvHwsAnpPortLinkTimerSet(devNum, phyPortNum, _50GBase_KR, timeInMsec));
    }
    /*TODO - add support*/
    /*if (apInteropPtr->attrBitMask & AP_PORT_INTROP_TX_DIS)
    {}
    if (apInteropPtr->attrBitMask & AP_PORT_INTROP_ABILITY_DUR)
    {}
    if (apInteropPtr->attrBitMask & AP_PORT_INTROP_ABILITY_MAX_INT)
    {}
    if (apInteropPtr->attrBitMask & AP_PORT_INTROP_ABILITY_MAX_FAIL_INT)
    {}*/
    return GT_OK;
}

/**
* @internal mvHwsPortAnpConfigGet function
* @endinternal
*
* @brief   Returns the ANP port configuration.
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
*
* @param[out] apCfgPtr             - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAnpConfigGet
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    MV_HWS_AP_CFG       *apCfgPtr
)
{
    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }
    HWS_NULL_PTR_CHECK_MAC(apCfgPtr);
    *apCfgPtr = PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum][phyPortNum];
    return GT_OK;
}

/**
* @internal mvHwsAnpPortCountersGet function
* @endinternal
*
* @brief   Returns the ANP port counters information
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[out] apCounters           - AP counters parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortCountersGet
(
    GT_U8                    devNum,
    GT_U32                   phyPortNum,
    MV_HWS_ANP_PORT_COUNTERS *anpCounters
)
{
    GT_U32          fieldName, value, clkTime;
    MV_HWS_UNITS_ID unitId;
    MV_HWS_PORT_STANDARD portMode = _10GBase_KR;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    HWS_NULL_PTR_CHECK_MAC(anpCounters);

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, portMode)) {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
    }
    /* AN restart counter - /ANP Units/Port<%n> AN restart counter
       Counts the number of times AN had restart, after got to resolution.
       That is, link_fail_inhibit timer expired without link, or AN completed, and then link failed.*/
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_AN_RESTART_COUNTER_P0_AN_RESTART_COUNTER_E,
                                 HARRIER_ANP_UNITS_PORT0_AN_RESTART_COUNTER_P0_AN_RESTART_COUNTER_E,
                                 PHOENIX_ANP_UNITS_PORT0_AN_RESTART_COUNTER_P0_AN_RESTART_COUNTER_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->anRestartCounter = value;

    /* DSP lock fail counter: ANP Units/Port<%n> DSP lock fail counter
       Counts the number of times Channel SM waits for dsp_lock and gets timeout.*/
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_DSP_LOCK_FAIL_COUNTER_P0_DSP_LOCK_FAIL_COUTNER_E,
                                 HARRIER_ANP_UNITS_PORT0_DSP_LOCK_FAIL_COUNTER_P0_DSP_LOCK_FAIL_COUTNER_E,
                                 PHOENIX_ANP_UNITS_PORT0_DSP_LOCK_FAIL_COUNTER_P0_DSP_LOCK_FAIL_COUTNER_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->dspLockFailCounter = value;

    /* Link fail counter: ANP Units/Port<%n> Link fail counter
       Counts the number of times Channel SM waits for link and gets timeout.*/
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_LINK_FAIL_COUNTER_P0_LINK_FAIL_COUNTER_E,
                                 HARRIER_ANP_UNITS_PORT0_LINK_FAIL_COUNTER_P0_LINK_FAIL_COUNTER_E,
                                 PHOENIX_ANP_UNITS_PORT0_LINK_FAIL_COUNTER_P0_LINK_FAIL_COUNTER_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->linkFailCounter = value;

    /* TX train duration: ANP Units/Port<%n> Counter2 (32 Low bits)
       ANP Units/Port<%n> Counter4[31:30](2 High bits)
       Controls: clear on sd soft reset. ANP Units/Port<%n> Control7 Bit[14].
       Holds the duration (clock cycles) of the last tx_train.*/
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_COUNTER2_P0_STAT_TX_TRAIN_DURATION_L_E,
                                 HARRIER_ANP_UNITS_PORT0_COUNTER2_P0_STAT_TX_TRAIN_DURATION_L_E,
                                 PHOENIX_ANP_UNITS_PORT0_COUNTER2_P0_STAT_TX_TRAIN_DURATION_L_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->txTrainDuration = value;

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_COUNTER4_P0_STAT_TX_TRAIN_DURATION_L_HI_E,
                                 HARRIER_ANP_UNITS_PORT0_COUNTER4_P0_STAT_TX_TRAIN_DURATION_L_HI_E,
                                 PHOENIX_ANP_UNITS_PORT0_COUNTER4_P0_STAT_TX_TRAIN_DURATION_L_HI_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->txTrainDuration |= ((value&3)<<30);
    /* change from clock cycle to msec*/
    clkTime =  HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? 156250 /*156.25M*/: 160000 /*160M*/;
    anpCounters->txTrainDuration /= clkTime;
    /* RX train duration: ANP Units/Port<%n> Counter1 (32 Low bits)
       ANP Units/Port<%n> Counter3[31:30](2 High bits)
       Controls: clear on sd soft reset. ANP Units/Port<%n> Control7 Bit[14].
       Holds the duration (clock cycles) of the last rx_train.*/
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_COUNTER1_P0_STAT_RX_TRAIN_DURATION_L_E,
                                 HARRIER_ANP_UNITS_PORT0_COUNTER1_P0_STAT_RX_TRAIN_DURATION_L_E,
                                 PHOENIX_ANP_UNITS_PORT0_COUNTER1_P0_STAT_RX_TRAIN_DURATION_L_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->rxTrainDuration = value;

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_COUNTER3_P0_STAT_RX_TRAIN_DURATION_L_HI_E,
                                 HARRIER_ANP_UNITS_PORT0_COUNTER3_P0_STAT_RX_TRAIN_DURATION_L_HI_E,
                                 PHOENIX_ANP_UNITS_PORT0_COUNTER3_P0_STAT_RX_TRAIN_DURATION_L_HI_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->rxTrainDuration |= ((value&3)<<30);
    /* change from clock cycle to msec*/
    clkTime =  HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? 156250 /*156.25M*/: 160000 /*160M*/;
    anpCounters->rxTrainDuration /= clkTime;
    /* TX train failed: ANP Units/Port<%n> Counter4 [29:20]
       Controls: ANP Units/Port<%n> Control7 Bit[13] - Clear on sd soft reset. Bit[20] - Clear.
       Counts the number of times tx_train performed and completed with fail status*/

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_COUNTER4_P0_STAT_TX_TRAIN_FAILED_CNT_E,
                                 HARRIER_ANP_UNITS_PORT0_COUNTER4_P0_STAT_TX_TRAIN_FAILED_CNT_E,
                                 PHOENIX_ANP_UNITS_PORT0_COUNTER4_P0_STAT_TX_TRAIN_FAILED_CNT_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->txTrainFailCounter = value;

    /* TX train ok: ANP Units/Port<%n> Counter5 [9:0]
       Controls: ANP Units/Port<%n> Control7 Bit[13] - Clear on sd soft reset.Bit[21] - Clear.
       Counts the number of times tx_train performed and completed without fail.*/
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_COUNTER5_P0_STAT_TX_TRAIN_OK_CNT_E,
                                 HARRIER_ANP_UNITS_PORT0_COUNTER5_P0_STAT_TX_TRAIN_OK_CNT_E,
                                 PHOENIX_ANP_UNITS_PORT0_COUNTER5_P0_STAT_TX_TRAIN_OK_CNT_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->txTrainOkCounter = value;

    /* TX train timeout: ANP Units/Port<%n> Counter5 [19:10]
       Controls: ANP Units/Port<%n> Control7 Bit[13] - Clear on sd soft reset.Bit[22] - Clear.
       Counts the number of times tx_train performed and got timeout.*/
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_COUNTER5_P0_STAT_TX_TRAIN_TIMEOUT_CNT_E,
                                 HARRIER_ANP_UNITS_PORT0_COUNTER5_P0_STAT_TX_TRAIN_TIMEOUT_CNT_E,
                                 PHOENIX_ANP_UNITS_PORT0_COUNTER5_P0_STAT_TX_TRAIN_TIMEOUT_CNT_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->txTrainTimeoutCounter = value;

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_COUNTER3_P0_STAT_RX_INIT_OK_CNT_E,
                                 HARRIER_ANP_UNITS_PORT0_COUNTER3_P0_STAT_RX_INIT_OK_CNT_E,
                                 PHOENIX_ANP_UNITS_PORT0_COUNTER3_P0_STAT_RX_INIT_OK_CNT_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->rxInitOk = value;

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_COUNTER3_P0_STAT_RX_INIT_TIMEOUT_CNT_E,
                                 HARRIER_ANP_UNITS_PORT0_COUNTER3_P0_STAT_RX_INIT_TIMEOUT_CNT_E,
                                 PHOENIX_ANP_UNITS_PORT0_COUNTER3_P0_STAT_RX_INIT_TIMEOUT_CNT_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->rxInitTimeOut = value;

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_COUNTER3_P0_STAT_RX_TRAIN_FAILED_CNT_E,
                                 HARRIER_ANP_UNITS_PORT0_COUNTER3_P0_STAT_RX_TRAIN_FAILED_CNT_E,
                                 PHOENIX_ANP_UNITS_PORT0_COUNTER3_P0_STAT_RX_TRAIN_FAILED_CNT_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->rxTrainFailed = value;

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_COUNTER4_P0_STAT_RX_TRAIN_OK_CNT_E,
                                 HARRIER_ANP_UNITS_PORT0_COUNTER4_P0_STAT_RX_TRAIN_OK_CNT_E,
                                 PHOENIX_ANP_UNITS_PORT0_COUNTER4_P0_STAT_RX_TRAIN_OK_CNT_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->rxTrainOk = value;

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_COUNTER4_P0_STAT_RX_TRAIN_TIMEOUT_CNT_E,
                                 HARRIER_ANP_UNITS_PORT0_COUNTER4_P0_STAT_RX_TRAIN_TIMEOUT_CNT_E,
                                 PHOENIX_ANP_UNITS_PORT0_COUNTER4_P0_STAT_RX_TRAIN_TIMEOUT_CNT_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpCounters->rxTrainTimeOut = value;
    return GT_OK;
}
/**
* @internal mvHwsAnpPortStatsGet function
* @endinternal
*
* @brief   Returns the ANP port statistics information
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[out] apStats              - AP statistics parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortStatsGet
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    MV_HWS_AP_PORT_STATS *apStats
)
{
    MV_HWS_ANP_PORT_COUNTERS anpCounters;
    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }
    HWS_NULL_PTR_CHECK_MAC(apStats);
    hwsOsMemSetFuncPtr(apStats, 0, sizeof(MV_HWS_AP_PORT_STATS));

    CHECK_STATUS(mvHwsAnpPortCountersGet(devNum, phyPortNum, &anpCounters));
    apStats->linkFailCnt = anpCounters.linkFailCounter;
    apStats->rxTrainDuration = anpCounters.rxTrainDuration;
    /*TODO GET STATS*/
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(phyPortNum);
    GT_UNUSED_PARAM(apStats);
    /*CPSS_TBD_BOOKMARK_AC5P
        GT_U16 txDisCnt;
        GT_U16 abilityCnt;
        GT_U16 abilitySuccessCnt;
        GT_U16 linkFailCnt;
        GT_U16 linkSuccessCnt;
        GT_U32 hcdResoultionTime;
        GT_U32 linkUpTime;*/

    return GT_OK;
}

/**
* @internal mvHwsAnpPortStatsReset function
* @endinternal
*
* @brief   Reset ANP port statistics information
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortStatsReset
(
    GT_U8                devNum,
    GT_U32               phyPortNum
)
{
    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return GT_BAD_PARAM;
    }
    /*TODO STATS RESET*/
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(phyPortNum);

    return GT_OK;
}

/**
* @internal mvHwsAnpPortStatusGet function
* @endinternal
*
* @brief   Returns the AP port resolution information
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] apStatusPtr                 - AP status parameters
*
* @param[out] apStatus                 - AP/HCD results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortStatusGet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_AP_PORT_STATUS  *apStatusPtr
)
{
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;
    MV_HWS_UNITS_ID unitId, unitIdAn;
    MV_HWS_PORT_STANDARD portMode = _10GBase_KR;
    GT_U32          baseAddr,unitIndex, unitNum;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32 regAddr, regData = 0, val, portFec;
    MV_HWS_PORT_STANDARD resolution = NON_SUP_MODE;
    GT_U32          fieldName;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
        unitIdAn = AN_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, portMode)) {
        unitId = ANP_USX_UNIT;
        unitIdAn = AN_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
        unitIdAn = AN_400_UNIT;
    }
    hwsOsMemSetFuncPtr(apStatusPtr, 0, sizeof(MV_HWS_AP_PORT_STATUS));

    apStatusPtr->preApPortNum  = phyPortNum;
    apStatusPtr->postApPortNum = phyPortNum;
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAnpPortStatusGet port %d ******\n",phyPortNum);
        hwsPpHwTraceEnablePtr(devNum, 2, GT_TRUE);
    }
#endif

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0_AP_ANEG_STATE_S10_0_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitIdAn, fieldName, &val, NULL));
    apStatusPtr->smState = val;
    /* check if sm is in reset state*/
    if (val == 0)
    {
        return GT_OK;
    }
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_AG_HCD_RESOLVED_S_E,
                                 HARRIER_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_AG_HCD_RESOLVED_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_AG_HCD_RESOLVED_S_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &val, NULL));

    if(CPSS_DEV_IS_WM_NATIVE_OR_ASIM_MAC(devNum))
    {
        val = 1;
    }

    apStatusPtr->hcdResult.hcdFound = val;
    if ( val == 0 ) {
        return GT_OK;
    }
    /* read ral_reg_anp_units_anp_units_RegFile_Port_AN_Status1 status1;
          ral_reg_anp_units_anp_units_RegFile_Port_AN_Status2 status2;*/
    CHECK_STATUS(mvUnitExtInfoGet(devNum, unitId, (phyPortNum), &baseAddr, &unitIndex, &unitNum ));
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_AN_STATUS1_P0_AP_PWRUP_4X_S_E,
                                 HARRIER_ANP_UNITS_PORT0_AN_STATUS1_P0_AP_PWRUP_4X_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_AN_STATUS1_P0_AP_PWRUP_4X_S_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT))
    {
        convertIdx.ciderIndexInUnit = 0;
    }
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 29, &regData));

    switch ( regData) {
    /*case (1>>0(): / * 10KX4* /
        break;*/
    case (1<<1):
        resolution = _1000Base_X;
        break;
    case (1<<2):
        resolution = _2500Base_X;
        break;
    case (1<<3):
        resolution = _5GBaseR;
        break;
    case (1<<4):
        resolution = _10GBase_KR;
        break;
    case (1<<5):
        if ( AP_CTRL_25GBase_KR1S_GET(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum][phyPortNum].modesVector) )
            resolution = _25GBase_KR_S;
        else
            resolution = _25GBase_CR_S;
        break ;
    case (1<<6):
        if ( AP_CTRL_25GBase_KR1_GET(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum][phyPortNum].modesVector) )
            resolution = _25GBase_KR;
        else
            resolution = _25GBase_CR;
        break ;
    case (1<<7):
        resolution = _25GBase_KR_C;
        break;
    case (1<<8):
        resolution = _25GBase_CR_C;
        break;
    case (1<<9):
        resolution = _40GBase_KR4;
        break;
    case (1<<10):
        resolution = _40GBase_CR4;
        break;
    case (1<<11):
        resolution = _50GBase_KR2_C;
        break;
    case (1<<12):
        resolution = _50GBase_CR2_C;
        break;
    case (1<<13):
        resolution = _100GBase_CR4;
        break;
    /*case (1<<14):
        resolution = _100GBase_CR10;
        break;*/
    case (1<<15):
        resolution = _100GBase_KR4;
        break;
   /* case (1<<16):
        resolution = _100GBase_KP4;
        break;*/
    case (1<<17):
        if ( AP_CTRL_200GBase_KR8_GET(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum][phyPortNum].modesVector) )
            resolution = _200GBase_KR8;
        else
            resolution = _200GBase_CR8;
        break ;
    case (1<<18):
        resolution = _25GBase_KR2;
        break;
    case (1<<19):
        /*if (AP_CTRL_40GBase_KR2_GET(mvHwsAnpPortParam[devNum][phyPortNum]))*/
            resolution = _40GBase_KR2;
        /*else
            resolution = _40GBase_CR2;*/
        break ;
    case (1<<20):
        resolution = _50GBase_KR4;
        break;
    case (1<<21):
        if ( AP_CTRL_50GBase_KR1_GET(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum][phyPortNum].modesVector) )
            resolution = _50GBase_KR;
        else
            resolution = _50GBase_CR;
        break ;
    case (1<<22):
        if ( AP_CTRL_100GBase_KR2_GET(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum][phyPortNum].modesVector) )
            resolution = _100GBase_KR2;
        else
            resolution = _100GBase_CR2;
        break ;
    case (1<<23):
        if ( AP_CTRL_200GBase_KR4_GET(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum][phyPortNum].modesVector) )
            resolution = _200GBase_KR4;
        else
            resolution = _200GBase_CR4;
        break ;
    /*case (1<<24):
        resolution = _100GBase_KR/CR;
        break;
    case (1<<25):
        resolution = _200GBase_KR2/CR2;
        break;
    case (1<<26):
        resolution = _400GBase_KR4/CR4;
        break;
    case (1<<27):
        resolution = _800GBase_KR8/CR8;
        break;*/
    case (1<<28):
        if ( AP_CTRL_400GBase_KR8_GET(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum][phyPortNum].modesVector) )
            resolution = _400GBase_KR8;
        else
            resolution = _400GBase_CR8;
        break ;
    default:
        return GT_FAIL;
    }
    apStatusPtr->postApPortMode = resolution;


    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_AG_RX_PAUSE_ENABLE_S_E,
                                 HARRIER_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_AG_RX_PAUSE_ENABLE_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_AG_RX_PAUSE_ENABLE_S_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &val, NULL));
    apStatusPtr->hcdResult.hcdFcRxPauseEn = val;
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_AG_TX_PAUSE_ENABLE_S_E,
                                 HARRIER_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_AG_TX_PAUSE_ENABLE_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_AG_TX_PAUSE_ENABLE_S_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &val, NULL));
    apStatusPtr->hcdResult.hcdFcTxPauseEn = val;

    apStatusPtr->hcdResult.hcdFecType = AP_ST_HCD_FEC_RES_NONE;
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_RSFEC_ENABLE_S_E,
                                 HARRIER_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_RSFEC_ENABLE_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_RSFEC_ENABLE_S_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &portFec, NULL));
    if ( portFec ) {
        apStatusPtr->hcdResult.hcdFecType = AP_ST_HCD_FEC_RES_RS;
        apStatusPtr->hcdResult.hcdFecEn = 1;
    }
    else
    {
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_FEC_ENABLE_S_E,
                                     HARRIER_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_FEC_ENABLE_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_FEC_ENABLE_S_E);
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &portFec, NULL));
        if ( portFec ) {
            apStatusPtr->hcdResult.hcdFecType = AP_ST_HCD_FEC_RES_FC;
            apStatusPtr->hcdResult.hcdFecEn = 1;
        }
    }

    /*TODO add all port status*/
    if(CPSS_DEV_IS_WM_NATIVE_OR_ASIM_MAC(devNum))
    {
        apStatusPtr->hcdResult.hcdFecType = AP_ST_HCD_FEC_RES_NONE;
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAnpPortStatusGet port %d ******\n",phyPortNum);
        hwsPpHwTraceEnablePtr(devNum, 2, GT_FALSE);
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsAnpPortResolutionBitSet function
* @endinternal
*
* @brief   set resolution bit per port mode
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] anResBit                 - resolution bit shift
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortResolutionBitSet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  anResBit
)
{
    MV_HWS_UNITS_ID unitId;
    GT_U32          fieldName, value;

    switch ( portMode )
    {
    case HWS_400G_PAM4_MODE_CASE:
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_10_ADVERTISED_CON400GRCR8_LOCATION_E;
        value = anResBit + 2; /* default value is 2 */
        break;

    case _50GBase_KR2_C:
    case _50GBase_CR2_C:
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_10_ADVERTISED_CON50GKRCR_LOCATION_E;
        value = anResBit + 8;  /* default value is 8 */
        break;

    case _25GBase_KR_C:
    case _25GBase_CR_C:
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_10_ADVERTISED_CON25GKRCR_LOCATION_E;
        value = anResBit + 4; /* default value is 4 */
        break;

    case HWS_40G_R2_MODE_CASE:
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_14_ADVERTISED_40G_R2_MODE_A15_TO_A22_BIT_LOCATION_SELECT_E;
        value = anResBit; /* default value is 0 */
        break;

    default:
        return GT_NOT_SUPPORTED;
    }

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = AN_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, portMode)) {
        unitId = AN_USX_UNIT;
    }
    else
    {
        unitId = AN_400_UNIT;
    }

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, value, NULL));
    return GT_OK;
}

/**
* @internal mvHwsAnpPortBreakpointCheck function
* @endinternal
*
* @brief   check if  ANP breakpoint reached
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[in] portMode              - port mode
* @param[in] progState             - program state
* @param[out] progStateReachedPtr  - program state reached
*
* @retval 0                        - on success
* @retval 1                        - on error
* */
GT_STATUS mvHwsAnpPortBreakpointCheck
(
    GT_U8                 devNum,
    GT_U32                phyPortNum,
    MV_HWS_PORT_STANDARD  portMode,
    MV_HWS_ANP_PROG_STATE progState,
    GT_BOOL               *progStateReachedPtr
)
{
    GT_U32          fieldName, value;
    MV_HWS_UNITS_ID unitId;


    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, portMode)) {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
    }
    switch ( progState )
    {
        /*CH SM: PWRUP, TXON, RXSD, RXON, TXRXSD, TXRXON, NORM.*/
    case MV_HWS_ANP_PROG_STATE_NORM:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_NORM_EN_MX_S_E,
                                     HARRIER_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_NORM_EN_MX_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_NORM_EN_MX_S_E);
        break;
    case MV_HWS_ANP_PROG_STATE_PWRUP:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_PWRUP_EN_MX_S_E,
                                     HARRIER_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_PWRUP_EN_MX_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_PWRUP_EN_MX_S_E);
        break;
    case MV_HWS_ANP_PROG_STATE_TXON:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_TXON_EN_MX_S_E,
                                     HARRIER_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_TXON_EN_MX_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_TXON_EN_MX_S_E);
        break;
    case MV_HWS_ANP_PROG_STATE_RXSD:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_RXSD_EN_MX_S_E,
                                     HARRIER_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_RXSD_EN_MX_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_RXSD_EN_MX_S_E);
        break;
    case MV_HWS_ANP_PROG_STATE_RXON:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_RXON_EN_MX_S_E,
                                     HARRIER_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_RXON_EN_MX_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_RXON_EN_MX_S_E);
        break;
    case MV_HWS_ANP_PROG_STATE_TXRXSD:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_TXRXSD_EN_MX_S_E,
                                     HARRIER_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_TXRXSD_EN_MX_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_TXRXSD_EN_MX_S_E);
        break;
    case MV_HWS_ANP_PROG_STATE_TXRXON:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_TXRXON_EN_MX_S_E,
                                     HARRIER_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_TXRXON_EN_MX_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PROG_PWM_TXRXON_EN_MX_S_E);
        break;


        /*SD TX SM: TX_TRAIN.*/
    case MV_HWS_ANP_PROG_STATE_TX_TRAIN:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PM_PROG_TX_TRAIN_S_E,
                                     HARRIER_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PM_PROG_TX_TRAIN_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PM_PROG_TX_TRAIN_S_E);
        break;


        /*SD RX SM: RX_INIT, RX_TRAIN.*/
    case MV_HWS_ANP_PROG_STATE_RX_TRAIN:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PM_PROG_RX_TRAIN_S_E,
                                     HARRIER_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PM_PROG_RX_TRAIN_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PM_PROG_RX_TRAIN_S_E);
        break;

    case MV_HWS_ANP_PROG_STATE_RX_INIT:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PM_PROG_RX_INIT_S_E,
                                     HARRIER_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PM_PROG_RX_INIT_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_INT_PM_PROG_RX_INIT_S_E);
        break;

    default:
        return GT_NOT_SUPPORTED;

    }
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    if (progStateReachedPtr != NULL)
        *progStateReachedPtr = (GT_BOOL)value;

    hwsOsPrintf("Port %d  prog state reached %d:\n", phyPortNum, value);

    return GT_OK;
}

/**
* @internal mvHwsAnpPortBreakpointSet function
* @endinternal
*
* @brief   set ANP breakpoint
*
* @param[in] devNum        - system device number
* @param[in] phyPortNum    - physical port number
* @param[in] portMode      - port mode
 *@param[in] progState     - program state to stop before
*
* @retval 0                        - on success
* @retval 1                        - on error
            INIT        = 0
            PWRUP       = 1
            TXON        = 2
            RXSD        = 3
            RXON        = 4
            TXRXSD      = 5
            TXRXON      = 6
            NORM        = 7
            PWRDN       = 8
            TX_RX_INIT  = a
            TX_TRAIN    = b
            RX_TRAIN    = c
            RX_INIT     = d
*/
GT_STATUS mvHwsAnpPortBreakpointSet
(
    GT_U8                 devNum,
    GT_U32                phyPortNum,
    MV_HWS_PORT_STANDARD  portMode,
    MV_HWS_ANP_PROG_STATE progState,
    GT_BOOL               enable
)
{
    GT_U32          fieldName, fieldNameMask;
    MV_HWS_UNITS_ID unitId;
    GT_BOOL reached;

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, portMode)) {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
    }
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAnpPortBreakpointSet port %d ******\n",phyPortNum);
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
    }
#endif

    /* clear ststus before starting */
    mvHwsAnpPortBreakpointCheck(devNum,phyPortNum,portMode,progState,&reached);

    switch ( progState )
    {
        /*CH SM: PWRUP, TXON, RXSD, RXON, TXRXSD, TXRXON, NORM.*/
        /*CH SM: /Cider/EBU-IP/GOP/EPI/ANP_IP/ANP_IP 1.0 (hawk1)/ANP_IP {RTLF 8.1 200301.0}/ANP/ANP Units/Port<%n> Control7 Bits[12:7, 5].*/
    case MV_HWS_ANP_PROG_STATE_NORM:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_NORM_EN_S_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_NORM_EN_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_NORM_EN_S_E);
        fieldNameMask = mvHwsAnpFieldSet(devNum,
                                         HAWK_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_NORM_S_E,
                                         HARRIER_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_NORM_S_E,
                                         PHOENIX_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_NORM_S_E);
        break;
    case MV_HWS_ANP_PROG_STATE_PWRUP:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_PWRUP_EN_S_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_PWRUP_EN_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_PWRUP_EN_S_E);
        fieldNameMask = mvHwsAnpFieldSet(devNum,
                                         HAWK_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_PWRUP_S_E,
                                         HARRIER_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_PWRUP_S_E,
                                         PHOENIX_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_PWRUP_S_E);
        break;
    case MV_HWS_ANP_PROG_STATE_TXON:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_TXON_EN_S_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_TXON_EN_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_TXON_EN_S_E);
        fieldNameMask = mvHwsAnpFieldSet(devNum,
                                         HAWK_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_TXON_S_E,
                                         HARRIER_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_TXON_S_E,
                                         PHOENIX_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_TXON_S_E);
        break;
    case MV_HWS_ANP_PROG_STATE_RXSD:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_RXSD_EN_S_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_RXSD_EN_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_RXSD_EN_S_E);
        fieldNameMask = mvHwsAnpFieldSet(devNum,
                                         HAWK_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_RXSD_S_E,
                                         HARRIER_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_RXSD_S_E,
                                         PHOENIX_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_RXSD_S_E);
        break;
    case MV_HWS_ANP_PROG_STATE_RXON:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_RXON_EN_S_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_RXON_EN_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_RXON_EN_S_E);
        fieldNameMask = mvHwsAnpFieldSet(devNum,
                                         HAWK_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_RXON_S_E,
                                         HARRIER_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_RXON_S_E,
                                         PHOENIX_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_RXON_S_E);
        break;
    case MV_HWS_ANP_PROG_STATE_TXRXSD:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_TXRXSD_EN_S_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_TXRXSD_EN_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_TXRXSD_EN_S_E);
        fieldNameMask = mvHwsAnpFieldSet(devNum,
                                         HAWK_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_TXRXSD_S_E,
                                         HARRIER_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_TXRXSD_S_E,
                                         PHOENIX_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_TXRXSD_S_E);
        break;
    case MV_HWS_ANP_PROG_STATE_TXRXON:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_TXRXON_EN_S_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_TXRXON_EN_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_TXRXON_EN_S_E);
        fieldNameMask = mvHwsAnpFieldSet(devNum,
                                         HAWK_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_TXRXON_S_E,
                                         HARRIER_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_TXRXON_S_E,
                                         PHOENIX_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_TXRXON_S_E);
        break;

        /*SD TX SM: TX_TRAIN.*/
    case MV_HWS_ANP_PROG_STATE_TX_TRAIN:
        /* /Cider/EBU-IP/GOP/EPI/ANP_IP/ANP_IP 1.0 (hawk1)/ANP_IP {RTLF 8.1 200301.0}/ANP/ANP Units/Port<%n> Control8 Bit[8]*/
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL8_P0_REG_PROG_EN_TX_TRAIN_S_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL8_P0_REG_PROG_EN_TX_TRAIN_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL8_P0_REG_PROG_EN_TX_TRAIN_S_E);
        fieldNameMask = mvHwsAnpFieldSet(devNum,
                                         HAWK_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_TX_TRAIN_S_E,
                                         HARRIER_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_TX_TRAIN_S_E,
                                         PHOENIX_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_TX_TRAIN_S_E);
        break;

        /*SD RX SM: RX_INIT, RX_TRAIN.*/
        /* /Cider/EBU-IP/GOP/EPI/ANP_IP/ANP_IP 1.0 (hawk1)/ANP_IP {RTLF 8.1 200301.0}/ANP/ANP Units/Port<%n> Control8 Bits[6:5] */
    case MV_HWS_ANP_PROG_STATE_RX_TRAIN:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL8_P0_REG_PROG_EN_RX_TRAIN_S_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL8_P0_REG_PROG_EN_RX_TRAIN_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL8_P0_REG_PROG_EN_RX_TRAIN_S_E);
        fieldNameMask = mvHwsAnpFieldSet(devNum,
                                         HAWK_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_RX_TRAIN_S_E,
                                         HARRIER_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_RX_TRAIN_S_E,
                                         PHOENIX_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_RX_TRAIN_S_E);
        break;

    case MV_HWS_ANP_PROG_STATE_RX_INIT:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL8_P0_REG_PROG_EN_RX_INIT_S_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL8_P0_REG_PROG_EN_RX_INIT_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL8_P0_REG_PROG_EN_RX_INIT_S_E);
        fieldNameMask = mvHwsAnpFieldSet(devNum,
                                         HAWK_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_RX_INIT_S_E,
                                         HARRIER_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_RX_INIT_S_E,
                                         PHOENIX_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_RX_INIT_S_E);
        break;

    default:
        return GT_NOT_SUPPORTED;

    }
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, enable, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldNameMask, enable, NULL));
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAnpPortBreakpointSet port %d ******\n",phyPortNum);
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
    }
#endif

    return GT_OK;
}


/**
* @internal mvHwsAnpPortBreakpointClear function
* @endinternal
*
* @brief   Clear ANP after breakpoint reached
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[in] portMode              - port mode
* @param[in] progState             - program state
*
* @retval 0                        - on success
* @retval 1                        - on error
* */
GT_STATUS mvHwsAnpPortBreakpointClear
(
    GT_U8                 devNum,
    GT_U32                phyPortNum,
    MV_HWS_PORT_STANDARD  portMode,
    MV_HWS_ANP_PROG_STATE progState
)
{
    GT_U32          fieldName;
    MV_HWS_UNITS_ID unitId;
    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, portMode)) {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
    }

    switch ( progState )
    {
        /*CH SM: PWRUP, TXON, RXSD, RXON, TXRXSD, TXRXON, NORM.*/
    case MV_HWS_ANP_PROG_STATE_NORM:
    case MV_HWS_ANP_PROG_STATE_PWRUP:
    case MV_HWS_ANP_PROG_STATE_TXON:
    case MV_HWS_ANP_PROG_STATE_RXSD:
    case MV_HWS_ANP_PROG_STATE_RXON:
    case MV_HWS_ANP_PROG_STATE_TXRXSD:
    case MV_HWS_ANP_PROG_STATE_TXRXON:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_STATUS_P0_REG_PROG_PWM_DONE_S_E,
                                     HARRIER_ANP_UNITS_PORT0_STATUS_P0_REG_PROG_PWM_DONE_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_STATUS_P0_REG_PROG_PWM_DONE_S_E);
        break;


        /*SD TX SM: TX_TRAIN.*/
    case MV_HWS_ANP_PROG_STATE_TX_TRAIN:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_STATUS_P0_REG_PROG_TX_DONE_S_E,
                                     HARRIER_ANP_UNITS_PORT0_STATUS_P0_REG_PROG_TX_DONE_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_STATUS_P0_REG_PROG_TX_DONE_S_E);
        break;


        /*SD RX SM: RX_INIT, RX_TRAIN.*/
    case MV_HWS_ANP_PROG_STATE_RX_TRAIN:
    case MV_HWS_ANP_PROG_STATE_RX_INIT:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_STATUS_P0_REG_PROG_RX_DONE_S_E,
                                     HARRIER_ANP_UNITS_PORT0_STATUS_P0_REG_PROG_RX_DONE_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_STATUS_P0_REG_PROG_RX_DONE_S_E);
        break;

    default:
        return GT_NOT_SUPPORTED;

    }
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 1, NULL));

    CHECK_STATUS(mvHwsAnpPortBreakpointSet(devNum, phyPortNum, portMode, progState, GT_FALSE));
    return GT_OK;
}

/**
* @internal mvHwsAnpPortInfoGet function
* @endinternal
*
* @brief   Get ANP port information
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[out] anpInfoPtr           - anp info
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortInfoGet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum,
    OUT MV_HWS_ANP_INFO_STC      *anpInfoPtr
)
{
    GT_U32          fieldName, value;
    MV_HWS_UNITS_ID unitId;
    MV_HWS_PORT_STANDARD portMode = _10GBase_KR;

    hwsOsMemSetFuncPtr(anpInfoPtr, 0, sizeof(MV_HWS_ANP_INFO_STC));
    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, portMode)) {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
    }

    CHECK_STATUS(mvHwsAnpPortCountersGet(devNum, phyPortNum, &anpInfoPtr->portConters));

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_STATE_E,
                                 HARRIER_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_STATE_E,
                                 PHOENIX_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_STATE_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpInfoPtr->portSm.chSmState = value;

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_SERDES_TX_SM_CONTROL_P0_SD_TX_SM_STATE_E,
                                 HARRIER_ANP_UNITS_PORT0_SERDES_TX_SM_CONTROL_P0_SD_TX_SM_STATE_E,
                                 PHOENIX_ANP_UNITS_PORT0_SERDES_TX_SM_CONTROL_P0_SD_TX_SM_STATE_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpInfoPtr->portSm.txSmState = value;

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_SERDES_RX_SM_CONTROL_P0_SD_RX_SM_STATE_E,
                                 HARRIER_ANP_UNITS_PORT0_SERDES_RX_SM_CONTROL_P0_SD_RX_SM_STATE_E,
                                 PHOENIX_ANP_UNITS_PORT0_SERDES_RX_SM_CONTROL_P0_SD_RX_SM_STATE_E);
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    anpInfoPtr->portSm.rxSmState = value;

    return GT_OK;
}

/**
* @internal mvHwsAnpPortDump function
* @endinternal
*
* @brief   Print ANP port information
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortDump
(
    GT_U8                    devNum,
    GT_U32                   phyPortNum
)
{
    GT_U32          fieldName, value;
    MV_HWS_UNITS_ID unitId;
    MV_HWS_PORT_STANDARD portMode = _10GBase_KR;
    GT_U32  numOfLanes =1, idx;
    MV_HWS_PORT_INIT_PARAMS curPortParams;  /* current port parameters */
    const GT_U32 hawkLocalIndexArr[8]    = {0,1,2,6,10,14,18,22};
    GT_U32 nextPortShift = 0;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_ANP_INFO_STC      anpInfoDb[8];
    const char *tx_fsm_states[] =
    {
        "TX_IDLE",
        "TX_PWR_ON",
        "TX_PLL_UP",
        "ERR - 0x3",
        "ERR - 0x4",
        "TX_IDLE_REMOVE_2KR",
        "TX_TRAIN",
        "TX_IDLE_REMOVE_NOKR",
        "TX_DONE_OK",
        "TX_DONE_FAIL",
        "ERR - 0xa",
        "ERR - 0xb",
        "TX_PROG_TX_TRAIN",
        "TX_TRAIN_OK_WAIT",
        "TX_RESET",
        "TX_RESET_WAIT"
    };
    const char *rx_fsm_states[] =
    {
        "RX_IDLE",
        "RX_PWR_ON",
        "RX_PLL_UP",
        "RX_WAIT_PLUG",
        "RX_WAIT_SQ_DET",
        "RX_INIT",
        "RX_TRAIN",
        "RX_DONE_OK",
        "RX_DONE_FAIL",
        "RX_TRAIN_OK_WAIT",
        "RX_PROG_RX_INIT",
        "RX_PROG_RX_TRAIN",
        "ERR - 0xc",
        "ERR - 0xd",
        "RX_RESET",
        "RX_RESET_WAIT"
    };
    const char *ch_fsm_states[] =
    {
        "ST_RESET",
        "ST_PWR_DN",
        "ST_PWR_UP",
        "ST_TX_ON",
        "ST_RX_SD",
        "ST_RX_ON",
        "ST_NORM",
        "ST_WAIT_PWR_DN",
        "ST_TXRX_SD",
        "ST_TXRX_ON",
        "ERR - 0xa",
        "ST_TXSTR_RXSD",
        "ST_TXSTR_RXON",
        "ST_RXSTR_TXON",
        "ST_RXSTR_PWR_UP",
        "ERR - 0xf",
        "ERR - 0x10",
        "ST_PROG_PWR_UP",
        "ST_PROG_TXON",
        "ST_PROG_RXSD",
        "ST_PROG_RXON",
        "ST_PROG_TXRXSD",
        "ST_PROG_TXRXON",
        "ST_PROG_NORM"
    };
    MV_HWS_AP_PORT_STATUS    apStatus;
    MV_HWS_AP_PORT_INTROP    apInterop;

    hwsOsMemSetFuncPtr(&apInterop, 0, sizeof(MV_HWS_AP_PORT_INTROP));
    hwsOsMemSetFuncPtr(&apStatus, 0, sizeof(MV_HWS_AP_PORT_STATUS));
    mvHwsAnpPortStatusGet(devNum, phyPortNum, &apStatus);

    hwsOsPrintf("------------------------------------------\n");
    hwsOsPrintf("Port %d status:\n", phyPortNum);
    hwsOsPrintf("    Resolution found:%d \n", apStatus.hcdResult.hcdFound);
    if ( apStatus.hcdResult.hcdFound ) {
        hwsOsPrintf("    Mode:%d fecEn:%d fecType:%d fcRxPause:%d, fcTxPause:%d\n", apStatus.postApPortMode,apStatus.hcdResult.hcdFecEn,
               apStatus.hcdResult.hcdFecType, apStatus.hcdResult.hcdFcRxPauseEn, apStatus.hcdResult.hcdFcTxPauseEn);
        if (GT_OK == hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, apStatus.postApPortMode, &curPortParams))
        {
            numOfLanes = curPortParams.numOfActLanes;
        }
    }

    mvHwsAnpPortInteropGet(devNum, phyPortNum, &apInterop);
    hwsOsPrintf("Port %d interop:\n", phyPortNum);
    hwsOsPrintf("    Slow:%d Nrz:%d Pam4:%d\n", apInterop.pdLinkMaxInterval, apInterop.apLinkMaxInterval, apInterop.anPam4LinkMaxInterval);

    if (mvHwsGlobalMacToLocalIndexConvert(devNum, phyPortNum, portMode, &convertIdx) != GT_OK)
    {
        convertIdx.ciderIndexInUnit = 0;
    }

    for (idx = 0; idx < numOfLanes; idx++)
    {
        if (HWS_DEV_SILICON_TYPE(devNum) == AC5P)
        {
            nextPortShift = hawkLocalIndexArr[convertIdx.ciderIndexInUnit + idx] - hawkLocalIndexArr[convertIdx.ciderIndexInUnit];
        }
        else if (HWS_DEV_SILICON_TYPE(devNum) == Harrier)
        {
            nextPortShift = 2*idx;
        }
        else
        {
            nextPortShift = idx;
        }

        mvHwsAnpPortInfoGet(devNum, phyPortNum + nextPortShift, &anpInfoDb[idx]);
    }

    hwsOsPrintf("Port %d counters \n", phyPortNum);
    hwsOsPrintf("  anRstCnt ");
    hwsOsPrintf("  dspLockFailCnt ");
    hwsOsPrintf("  linkFailCnt ");
    hwsOsPrintf("  txTrainFailCnt ");
    hwsOsPrintf("  txTrainOkCnt ");
    hwsOsPrintf("  txTrainTimeoutCnt ");
    hwsOsPrintf("  txTrainDuration ");
    hwsOsPrintf("\n");

    for (idx = 0; idx < numOfLanes; idx++)
    {
        hwsOsPrintf("L%d: ", idx);
        hwsOsPrintf("%3d", anpInfoDb[idx].portConters.anRestartCounter);
        hwsOsPrintf("%18d", anpInfoDb[idx].portConters.dspLockFailCounter);
        hwsOsPrintf("%12d", anpInfoDb[idx].portConters.linkFailCounter);
        hwsOsPrintf("%16d", anpInfoDb[idx].portConters.txTrainFailCounter);
        hwsOsPrintf("%14d", anpInfoDb[idx].portConters.txTrainOkCounter);
        hwsOsPrintf("%22d", anpInfoDb[idx].portConters.txTrainTimeoutCounter);
        hwsOsPrintf("%20d", anpInfoDb[idx].portConters.txTrainDuration);
        hwsOsPrintf("\n");
    }

    hwsOsPrintf("  rxInitOk ");
    hwsOsPrintf("  rxInitTimeOut ");
    hwsOsPrintf("  rxTrainFailed ");
    hwsOsPrintf("  rxTrainOk ");
    hwsOsPrintf("  rxTrainTimeOut ");
    hwsOsPrintf("\n");
    for (idx = 0; idx < numOfLanes; idx++)
    {
        hwsOsPrintf("L%d: ", idx);
        hwsOsPrintf("%3d", anpInfoDb[idx].portConters.rxInitOk);
        hwsOsPrintf("%18d", anpInfoDb[idx].portConters.rxInitTimeOut);
        hwsOsPrintf("%12d", anpInfoDb[idx].portConters.rxTrainFailed);
        hwsOsPrintf("%14d", anpInfoDb[idx].portConters.rxTrainOk);
        hwsOsPrintf("%14d", anpInfoDb[idx].portConters.rxTrainTimeOut);
        hwsOsPrintf("\n");
    }

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, portMode)) {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
    }

    hwsOsPrintf("\nPort %d StateMachine status:\n", phyPortNum);

    hwsOsPrintf("  CH FSM State     :  %s \n", ch_fsm_states[anpInfoDb[0].portSm.chSmState]);
    /*fieldName = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_OVERRIDE_CTRL_E:
                PHOENIX_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_OVERRIDE_CTRL_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    hwsOsPrintf("oberrideCtrl %d  ", value);

    fieldName = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_BP_REACHED_E:
                PHOENIX_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_BP_REACHED_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &value, NULL));
    hwsOsPrintf("bpReached %d\n", value);*/

    for (idx = 0; idx < numOfLanes; idx++)
    {
        if (HWS_DEV_SILICON_TYPE(devNum) == AC5P)
        {
            nextPortShift = hawkLocalIndexArr[convertIdx.ciderIndexInUnit + idx] - hawkLocalIndexArr[convertIdx.ciderIndexInUnit];
        }
        else if (HWS_DEV_SILICON_TYPE(devNum) == Harrier)
        {
            nextPortShift = 2*idx;
        }
        else
        {
            nextPortShift = idx;
        }

        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_SERDES_TX_SM_CONTROL_P0_SD_TX_SM_STATE_E,
                                     HARRIER_ANP_UNITS_PORT0_SERDES_TX_SM_CONTROL_P0_SD_TX_SM_STATE_E,
                                     PHOENIX_ANP_UNITS_PORT0_SERDES_TX_SM_CONTROL_P0_SD_TX_SM_STATE_E);
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum+nextPortShift, unitId, fieldName, &value, NULL));
        hwsOsPrintf("  TX FSM idx %d State: %s\n", idx, tx_fsm_states[value]);

        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_SERDES_RX_SM_CONTROL_P0_SD_RX_SM_STATE_E,
                                     HARRIER_ANP_UNITS_PORT0_SERDES_RX_SM_CONTROL_P0_SD_RX_SM_STATE_E,
                                     PHOENIX_ANP_UNITS_PORT0_SERDES_RX_SM_CONTROL_P0_SD_RX_SM_STATE_E);
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum+nextPortShift, unitId, fieldName, &value, NULL));

        hwsOsPrintf("  RX FSM idx %d State: %s\n", idx, rx_fsm_states[value]);
    }

    return GT_OK;
}


/**
* @internal mvHwsAnpCmdGetOpmode function
* @endinternal
*
* @brief  convert from serdesSpeed to opMode
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - port number
* @param[in] serdesSpeed           - serdes speed
* @param[in] unitId                - unit id
* @param[out] opmodePtr            - op mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAnpCmdGetOpmode
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    MV_HWS_SERDES_SPEED  serdesSpeed,
    MV_HWS_UNITS_ID      unitId,
    GT_U32               *opmodePtr
)
{
    GT_U32 fieldName;
    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }
    switch ( serdesSpeed ) {
    case SPEED_NA:
        *opmodePtr = 0x1F;
        return GT_OK;
    case _1_25G:
    case _1_25G_SR_LR:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_1P25G_E,
                                     HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_1P25G_E,
                                     PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_1P25G_E);
        break;
    case _2_578125:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_2P578125G_E,
                                     HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_2P578125G_E,
                                     PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_2P578125G_E);
        break;
    case _3_125G:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_3P125G_E,
                                     HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_3P125G_E,
                                     PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_3P125G_E);
        break;
    case _5G:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_5G_E,
                                     HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_5G_E,
                                     PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_5G_E);
        break;
    case _5_15625G:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_5P15625G_E,
                                     HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_5P15625G_E,
                                     PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_5P15625G_E);
        break;
    case _10G:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_10G_E,
                                     HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_10G_E,
                                     PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_10G_E);
        break;
    case _10_3125G:
    case _10_3125G_SR_LR:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_10P3125G_E,
                                     HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_10P3125G_E,
                                     PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_10P3125G_E);
        break;
    case _20_625G:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_20P625G_E,
                                     HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_20P625G_E,
                                     PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_20P625G_E);
        break;
    case _25_78125G:
    case _25_78125G_SR_LR:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_25P78125G_E,
                                     HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_25P78125G_E,
                                     PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_25P78125G_E);
        break;
    case _26_5625G:
    /*case _26_5625G_PAM4:
    case _26_5625G_PAM4_SR_LR:*/
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_26P5625G_E,
                                     HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_26P5625G_E,
                                     PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_26P5625G_E);
        break;
    case _27_5G:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_27P5G_E,
                                     HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE2_PM_SPEED_TABLE_27P5G_E,
                                     PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_27P5G_E);
        break;
    case _26_5625G_PAM4:
    case _26_5625G_PAM4_SR_LR:
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_53P125G_E,
                                     HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE2_PM_SPEED_TABLE_53P125G_E,
                                     PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_53P125G_E);
        break;
    default:
        return GT_NOT_SUPPORTED;
    }

    return genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, fieldName, opmodePtr, NULL);
}

/**
* @internal mvHwsAnpCmdInterface function
* @endinternal
*
* @brief  add line to anr command table
*
* @param[in] devNum        - system device number
* @param[in] cmdLineIdx    - line index
* @param[in] enable        - enable ile
* @param[in] phyPortNum    - port number
* @param[in] serdesSpeed   - serdes speed
* @param[in] numOfLanes    - num of lanes  1/2/4/8/FF
*       - R1/R2/R4/R8/ALL
* @param[in] progState     - program state
* @param[in] apMode        - ap mode 0-non ap, 1- ap, ff dont
*       care
* @param[in] code          - code
* @param[in] data          - data
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
/*  0xA7E00224 + t*0x4+n*0x2a4 : where n (0-3) represents port_num, where t (0-63) represents cmd_line
    32LSB of command interface cmem in line <<%t>>
    [0] = pg_en_s
    [8:4] = pg_opmode_s
    [11:9] = pg_couple_s
    [15:12] = pg_fsm_field_s
            INIT        = 0
            PWRUP       = 1
            TXON        = 2
            RXSD        = 3
            RXON        = 4
            TXRXSD      = 5
            TXRXON      = 6
            NORM        = 7
            PWRDN       = 8
            TX_RX_INIT  = a
            TX_TRAIN    = b
            RX_TRAIN    = c
            RX_INIT     = d
    [17:16] = pg_ap_s
    [25:18] = interrupt code
    [31:26] = interrupt data[5:0] */

GT_STATUS mvHwsAnpCmdInterface
(
    GT_U8                devNum,
    GT_U32               cmdLineIdx,
    GT_BOOL              enable,
    GT_U32               phyPortNum,
    MV_HWS_SERDES_SPEED  serdesSpeed,
    GT_U32               numOfLanes, /*1/2/4/8/FF - R1/R2/R4/R8/ALL*/
    GT_U32               progState,
    GT_U32               apMode, /* 0-non ap, 1- ap, ff - dont care */
    GT_U32               code,   /* 8 bits*/
    GT_U32               data    /* 22 bits */
)
{
    GT_U32 cmdLineLow = 0, cmdLineHigh = 0, val = 0;
    MV_HWS_UNITS_ID unitId;
    GT_U32 unitAddr, fieldName,lineAddress;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || (cmdLineIdx > 63))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else if ((HWS_DEV_SILICON_TYPE(devNum) == AC5X) && (phyPortNum < MV_HWS_AC5X_GOP_PORT_CPU_0_CNS))
    {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
    }
    hwsOsPrintf("mvHwsAnpCmdInterface line:%d port:%d enable:%d serdesSpeed:%d numOfLanes:%d progState:%d apMode:%d code:%d data:0x%x\n",cmdLineIdx,phyPortNum,enable, serdesSpeed, numOfLanes,progState,apMode,code,data);

    /**** write enable [0:0]****/
    if ( enable == GT_TRUE) {
        cmdLineLow |= (1<<0);
        /**** write serdesSpeed - opMode [4:8]****/
        CHECK_STATUS(mvHwsAnpCmdGetOpmode(devNum, phyPortNum, serdesSpeed, unitId, &val));
        hwsOsPrintf("mvHwsAnpCmdInterface opMode %d \n",val);
        cmdLineLow |= ((val & 0x1F)<<4);

        /**** write numOfLanes - couple(R1/R2/R4/R8) [9:11]****/
        switch ( numOfLanes) {
        case 1:
            val = 0;
            break;
        case 2:
            val = 1;
            break;
        case 4:
            val = 2;
            break;
        case 8:
            val = 3;
            break;
        case 0xFF:
            val = 4;
            break;
        default:
            hwsOsPrintf("mvHwsAnpCmdInterface fail numOfLanes %d \n",numOfLanes);
            return GT_NOT_SUPPORTED;
        }
        cmdLineLow |= ((val & 0x7)<<9);


        /**** write progState [12:15]****/
        cmdLineLow |= ((progState & 0xF)<<12);

        /**** write apMode (NON-AP/AP/ALL) [16:17]****/
        val = 0;
        switch ( apMode) {
        case 0:
            val = 0;
            break;
        case 1:
            val = 1;
            break;
        case 0xFF:
            val = 2;
            break;
        default:
            hwsOsPrintf("mvHwsAnpCmdInterface fail apMode %d \n",apMode);
            return GT_NOT_SUPPORTED;
        }
        cmdLineLow |= ((val & 0x3)<<16);

        /**** write code [18:25]****/
        cmdLineLow |= ((code & 0xFF)<<18);

        /**** write data [26:31]****/
        cmdLineLow |= ((data & 0x3F)<<26);

        /**** write data high bits [6:21]****/
        val = ((data & 0x3FFFC0)>>6);
       /* if ((cmdLineIdx%2) == 0) */
        {
            cmdLineHigh = val;
        }
       /* else
        {
            cmdLineHigh = (val<<16);
        }*/
    }


    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CMD_LINE0_LO_P0_CMD_LINE0_LO_E,
                                 HARRIER_ANP_UNITS_PORT0_CMD_LINE0_LO_P0_CMD_LINE0_LO_E,
                                 PHOENIX_ANP_UNITS_PORT0_CMD_LINE0_LO_P0_CMD_LINE0_LO_E);

    unitAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, _10GBase_KR, fieldName, &fieldReg);
    if(unitAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        hwsOsPrintf("mvHwsAnpCmdInterface fail unitId %d fieldName %d\n",unitId, fieldName);
        return GT_BAD_PARAM;
    }


    lineAddress = unitAddr + cmdLineIdx*4;
    hwsOsPrintf("mvHwsAnpCmdInterface low addr:0x%x data:0x%x\n",lineAddress, cmdLineLow);
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, lineAddress, 0, 32, cmdLineLow));

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CMD_LINE0_HI_P0_CMD_LINE_0_HI_E,
                                 HARRIER_ANP_UNITS_PORT0_CMD_LINE0_HI_P0_CMD_LINE_0_HI_E,
                                 PHOENIX_ANP_UNITS_PORT0_CMD_LINE0_HI_P0_CMD_LINE_0_HI_E);

    unitAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, _10GBase_KR, fieldName, &fieldReg);
    if(unitAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        return GT_BAD_PARAM;
    }


    lineAddress = unitAddr + (cmdLineIdx/2)*4;
    hwsOsPrintf("mvHwsAnpCmdInterface high addr:0x%x data:0x%x\n",lineAddress, cmdLineHigh);
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, lineAddress, 16 * (cmdLineIdx%2), 16, cmdLineHigh));


    return GT_OK;
}

/**
* @internal mvHwsAnpCmdTableGetIdx function
* @endinternal
*
* @brief  get next line in command table
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - port number
* @param[out] indexPtr             - index
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpCmdTableGetIdx
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    GT_U32               *indexPtr
)
{
    if (PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum][phyPortNum] == MV_HWS_ANP_CMD_TABLE_EMPTY_CNS)
    {
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum][phyPortNum] = 0;
        *indexPtr = PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum][phyPortNum];
    }
    else if ( PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum][phyPortNum] >= MV_HWS_ANP_CMD_TABLE_SIZE_CNS-1 )
    {
        *indexPtr = PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum][phyPortNum];
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ARG_STRING_MAC("Anp cmd table is full"))
    }
    else
    {
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum][phyPortNum]++;
        *indexPtr = PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum][phyPortNum];
    }
    return GT_OK;
}

/**
* @internal mvHwsAnpCmdTableClear function
* @endinternal
*
* @brief  clear anp command table
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpCmdTableClear
(
    GT_U8                devNum,
    GT_U32               phyPortNum
)
{
    GT_U32  i;
    if (PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpUseCmdTable))
    {
        for (i = 0; i < PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum][phyPortNum]; i++)
        {
            mvHwsAnpCmdInterface(devNum,i,GT_FALSE, phyPortNum, 0, 0, 0, 0, 0, 0);
        }
        PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpCmdIntLineIdxPtr[devNum][phyPortNum] = MV_HWS_ANP_CMD_TABLE_EMPTY_CNS;
    }
    return GT_OK;
}

/**
* @internal mvHwsAnpCmdTxOverride function
* @endinternal
*
* @brief  write all tx parametrs to anp command table
*
* @param[in] devNum        - system device number
* @param[in] phyPortNum    - port number
* @param[in] serdesSpeed   - serdes speed
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpCmdTxOverride
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    MV_HWS_SERDES_SPEED  serdesSpeed
)
{
    GT_STATUS  rc;
    GT_UOPT    portGroup = 0;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT configParams;
    GT_U32               numOfLanes = 0xff; /*1/2/4/8/FF - R1/R2/R4/R8/ALL*/
    GT_U32               progState =0xb/* 1*/; /*pwrUp*/
    GT_U32               apMode = 1; /* 0-non ap, 1- ap, ff - dont care */
    GT_U32               code = 0x17;    /* 8 bits - txEqualizer*/
    GT_U32               idx;    /* 22 bits */
    GT_32                data;

    rc = mvHwsSerdesDbTxConfigGet(devNum, portGroup,  phyPortNum,  HWS_DEV_SERDES_TYPE(devNum,phyPortNum), serdesSpeed, &configParams);
    if ( rc != GT_OK)
    {
        return rc;
    }

    switch ( HWS_DEV_SERDES_TYPE(devNum,phyPortNum) )
    {
    case COM_PHY_C28GP4X1:
    case COM_PHY_C28GP4X2:
    case COM_PHY_C28GP4X4:
#if 0
        /*GT_U8 pre;
        GT_U8 peak;
        GT_U8 post;*/

        if ( configParams.txComphyC28GP4.peak != NA_8BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = (1 << 16) /*| (1<<12)*/ | configParams.txComphyC28GP4.peak;
            mvHwsAnpCmdInterface(devNum, idx, GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if (configParams.txComphyC28GP4.post != NA_8BIT)
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = (1 << 16) /*| (1<<12)*/ | configParams.txComphyC28GP4.post;
            mvHwsAnpCmdInterface(devNum, idx, GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }

        if (configParams.txComphyC28GP4.pre != NA_8BIT)
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = (1 << 16) /*| (1<<12)*/ | configParams.txComphyC28GP4.pre;
            mvHwsAnpCmdInterface(devNum, idx, GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
#endif
        break;

    case COM_PHY_C112GX4:
       /* GT_U8 pre2;
        GT_U8 pre;
        GT_U8 main;
        GT_U8 post;*/

        if ( configParams.txComphyC112G.main != NA_8BIT_SIGNED )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = (1 << 16) | (2<<12) | configParams.txComphyC112G.main;
            mvHwsAnpCmdInterface(devNum, idx, GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if (configParams.txComphyC112G.post != NA_8BIT_SIGNED)
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = (1 << 16) | (3<<12) | configParams.txComphyC112G.post;
            mvHwsAnpCmdInterface(devNum, idx, GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }

        if (configParams.txComphyC112G.pre != NA_8BIT_SIGNED)
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = (1 << 16) | (1<<12) | configParams.txComphyC112G.pre;
            mvHwsAnpCmdInterface(devNum, idx, GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.txComphyC112G.pre2 != NA_8BIT_SIGNED )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = (1 << 16) | (0<<12) | configParams.txComphyC112G.pre2;
            mvHwsAnpCmdInterface(devNum, idx, GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        break;

    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal mvHwsAnpCmdRxOverride function
* @endinternal
*
* @brief  write all rx parametrs to anp command table
*
* @param[in] devNum        - system device number
* @param[in] phyPortNum    - port number
* @param[in] serdesSpeed   - serdes speed
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpCmdRxOverride
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    MV_HWS_SERDES_SPEED  serdesSpeed
)
{
    GT_STATUS  rc;
    GT_UOPT    portGroup = 0;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT configParams;
    GT_U32               numOfLanes = 0xff; /*1/2/4/8/FF - R1/R2/R4/R8/ALL*/
    GT_U32               progState = 0xb/*1*/; /*pwrUp*/
    GT_U32               apMode = 1; /* 0-non ap, 1- ap, ff - dont care */
    GT_U32               code = 0x1;    /* 8 bits - txEqualizer*/
    GT_U32               idx, data;    /* 22 bits */

     rc = mvHwsSerdesDbRxConfigGet(devNum, portGroup,  phyPortNum,  HWS_DEV_SERDES_TYPE(devNum,phyPortNum), serdesSpeed, &configParams);
    if ( rc != GT_OK)
    {
        return rc;
    }

    switch ( HWS_DEV_SERDES_TYPE(devNum,phyPortNum) )
    {
    case COM_PHY_C28GP4X1:
    case COM_PHY_C28GP4X2:
    case COM_PHY_C28GP4X4:
#if 0
        /* Basic (CTLE) * /
        GT_U32 dataRate;
        GT_U32 res1Sel;
        GT_U32 res2Sel;
        GT_U32 cap1Sel;
        GT_U32 cap2Sel;*/
        if ( configParams.rxComphyC28GP4.dataRate != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.dataRate;
            mvHwsAnpCmdInterface(devNum,idx , GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC28GP4.res1Sel != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.res1Sel;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC28GP4.res2Sel != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.res2Sel;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC28GP4.cap1Sel != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.cap1Sel;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC28GP4.cap2Sel != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.cap2Sel;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }

        /* Advanced (CDR) * /
        GT_U32 selmufi;
        GT_U32 selmuff;
        GT_U32 selmupi;
        GT_U32 selmupf;*/
        if ( configParams.rxComphyC28GP4.selmufi != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.selmufi;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC28GP4.selmuff != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.selmuff;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC28GP4.selmupi != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.selmupi;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC28GP4.selmupf != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.selmupf;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }

        /* Advanced (Thresholds) * /
        GT_U32 midpointLargeThresKLane;
        GT_U32 midpointSmallThresKLane;
        GT_U32 midpointLargeThresCLane;
        GT_U32 midpointSmallThresCLane;*/
        if ( configParams.rxComphyC28GP4.midpointLargeThresKLane != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.midpointLargeThresKLane;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC28GP4.midpointSmallThresKLane != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.midpointSmallThresKLane;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC28GP4.midpointLargeThresCLane != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.midpointLargeThresCLane;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC28GP4.midpointSmallThresCLane != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.midpointSmallThresCLane;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }

        /* Advanced (DFE) * /
        GT_U32 dfeResF0aHighThresInitLane;
        GT_U32 dfeResF0aHighThresEndLane;

        GT_16  squelch;*/

        if ( configParams.rxComphyC28GP4.dfeResF0aHighThresInitLane != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.dfeResF0aHighThresInitLane;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC28GP4.dfeResF0aHighThresEndLane != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.dfeResF0aHighThresEndLane;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC28GP4.squelch != NA_16BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data = /*| (1<<14)*/ | configParams.rxComphyC28GP4.squelch;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
#endif
        break;

    case COM_PHY_C112GX4:
        /* Basic (CTLE) * /
        GT_U32 current1Sel;
        GT_U32 rl1Sel;
        GT_U32 rl1Extra;
        GT_U32 res1Sel;
        GT_U32 cap1Sel;
        GT_U32 cl1Ctrl;
        GT_U32 enMidFreq;
        GT_U32 cs1Mid;
        GT_U32 rs1Mid;*/
        if ( configParams.rxComphyC112G.current1Sel != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (1<<14) | configParams.rxComphyC112G.current1Sel;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.rl1Sel != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (2<<14) | configParams.rxComphyC112G.rl1Sel;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.rl1Extra != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (3<<14) | configParams.rxComphyC112G.rl1Extra;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.res1Sel != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (4<<14) | configParams.rxComphyC112G.res1Sel;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.cap1Sel != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (5<<14) | configParams.rxComphyC112G.cap1Sel;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.cl1Ctrl != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (6<<14) | configParams.rxComphyC112G.cl1Ctrl;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.enMidFreq != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (7<<14) | configParams.rxComphyC112G.enMidFreq;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.cs1Mid != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (8<<14) | configParams.rxComphyC112G.cs1Mid;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.rs1Mid != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (9<<14) | configParams.rxComphyC112G.rs1Mid;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        /* 1st stage TIA * /
        GT_U32 rfCtrl;
        GT_U32 rl1TiaSel;
        GT_U32 rl1TiaExtra;
        GT_U32 hpfRSel1st;
        GT_U32 current1TiaSel;*/
        if ( configParams.rxComphyC112G.rfCtrl != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (0xA<<14) | configParams.rxComphyC112G.rfCtrl;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.rl1TiaSel != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (0xB<<14) | configParams.rxComphyC112G.rl1TiaSel;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.rl1TiaExtra != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (0xC<<14) | configParams.rxComphyC112G.rl1TiaExtra;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.hpfRSel1st != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (0xD<<14) | configParams.rxComphyC112G.hpfRSel1st;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.current1TiaSel != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (0xE<<14) | configParams.rxComphyC112G.current1TiaSel;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }

        /* 2nd Stage * /
        GT_U32 rl2Tune;
        GT_U32 rl2Sel;
        GT_U32 rs2Sel;
        GT_U32 current2Sel;
        GT_U32 cap2Sel;
        GT_U32 hpfRsel2nd;*/
        if ( configParams.rxComphyC112G.rl2Tune != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (0xF<<14) | configParams.rxComphyC112G.rl2Tune;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.rl2Sel != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (0x13<<14) | configParams.rxComphyC112G.rl2Sel;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.rs2Sel != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (0x17<<14) | configParams.rxComphyC112G.rs2Sel;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.current2Sel != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (0x1B<<14) | configParams.rxComphyC112G.current2Sel;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.cap2Sel != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (0x1F<14) | configParams.rxComphyC112G.cap2Sel;
            mvHwsAnpCmdInterface(devNum,idx,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }
        if ( configParams.rxComphyC112G.hpfRsel2nd != NA_32BIT )
        {
            CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
            data =  (0x20<<14) | configParams.rxComphyC112G.hpfRsel2nd;
            mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, data);
        }

        /* Advanced (CDR) * /
        GT_U32 selmufi;
        GT_U32 selmuff;
        GT_U32 selmupi;
        GT_U32 selmupf;
        GT_U8  squelch; */
        break;

    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

GT_STATUS mvHwsAnpCmdRegisterWrite
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    MV_HWS_SERDES_SPEED  serdesSpeed,
    GT_U32               numOfLanes, /*1/2/4/8/FF - R1/R2/R4/R8/ALL*/
    GT_U32               progState,
    GT_U32               apMode, /* 0-non ap, 1- ap, ff - dont care */
    GT_U32               addr,
    GT_U32               data
)
{
    GT_U32               code, idx,val;

    code = 0x11;
    CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
    val =  data & 0xFFFF;
    mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, val);


    code = 0x10;
    CHECK_STATUS(mvHwsAnpCmdTableGetIdx(devNum, phyPortNum, &idx));
    val =  addr & 0xFFFF;
    mvHwsAnpCmdInterface(devNum,idx ,GT_TRUE, phyPortNum, serdesSpeed, numOfLanes, progState, apMode, code, val);

    return GT_OK;
}

/**
* @internal mvHwsHawkLocalIndexToAnpIndexConvert function
* @endinternal
*
* @brief  ANP convert fuction
*
*
************************************************************************************************************************
* portNum      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |10 |11 |12 |13 |14 |15 |16 |17 |18 |19 |20 |21 |22 |23 |24 |25 |
************************************************************************************************************************
* convert portNum ==> localIdx  according to portMode
************************************************************************************************************************
* localIdx 400 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | -----------------------------------------------------------------------
* localIdx USX | - | - | 0 | 2 | 1 | 3 | 4 | 6 | 5 | 7 | 8 |10 | 9 |11 |12 |14 |13 |15 |16 |18 |17 |19 |20 |22 |21 |23 |
************************************************************************************************************************
* convert localIdx ==> anpIdx  according to portMode
************************************************************************************************************************
* anpIdx 400   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | -----------------------------------------------------------------------
* anpIdx USX   | - | - | 8 | 8 | 8 | 8 | 9 | 9 | 9 | 9 |10 |10 |10 |10 |11 |11 |11 |11 |12 |12 |12 |12 |13 |13 |13 |13 |
* anpIdx USX_O | - | - | 8 | 8 | 8 | 8 | 8 | 8 | 8 | 8 |10 |10 |10 |10 |10 |10 |10 |10 |12 |12 |12 |12 |12 |12 |12 |12 |
************************************************************************************************************************
*/

GT_STATUS mvHwsHawkLocalIndexToAnpIndexConvert
(
    GT_U8                       devNum,
    GT_U32                      portNum,
    IN GT_U32                   localIdx,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT GT_U32                  *anpIdxPtr
)
{
    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        if (localIdx >=24)
        {
            return GT_NO_SUCH;
        }
        if (HWS_USX_O_MODE_CHECK(portMode))
        {
            *anpIdxPtr = (8 + (localIdx/8)*2);
        }
        else
        {
            *anpIdxPtr = (8 + localIdx/4);
        }
    }
    else
    {
        if (localIdx >=8)
        {
            return GT_NO_SUCH;
        }
        *anpIdxPtr = localIdx;
    }
    return GT_OK;
}

GT_STATUS mvHwsAnpSetDefaultLinkTimer
(
    GT_U8  devNum,
    GT_U32 port,
    GT_U32 capability
)
{
    GT_U32 timer = 0;

/*TODO remove when training is fixed (time)*/
    /*if (AP_CTRL_PAM4_GET(capability) && ((mvHwsAnpPortMiscParamDb[devNum][port].interopLinkTimer & 0x3) == 0))
    {
        timer = 12000;
        mvHwsAnpPortLinkTimerSet(devNum,port,_50GBase_KR, timer);
    }*/
    if (((AP_CTRL_ADV_25G_ALL_GET(capability)) || (AP_CTRL_ADV_100G_R4_ALL_GET(capability))|| (AP_CTRL_ADV_40G_R4_ALL_GET(capability))) &&
        ((PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum][port].interopLinkTimer & (0x3 << 2)) == 0))
    {
        timer = 1000;
        mvHwsAnpPortLinkTimerSet(devNum,port,_25GBase_KR, timer);
    }
    if ((AP_CTRL_1000Base_KX_GET(capability)) && ((PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum][port].interopLinkTimer & (0x3 << 4)) == 0))
    {
        timer = 500;
        mvHwsAnpPortLinkTimerSet(devNum,port,_1000Base_X, timer);
    }

    return GT_OK;
}

/**
* @internal mvHwsAnpCfgPcsDone function
* @endinternal
*
* @brief   config psc done after pcs configuration.
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] portMode           - port mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpCfgPcsDone
(
    IN GT_U8                    devNum,
    IN GT_U32                   phyPortNum,
    IN MV_HWS_PORT_STANDARD     portMode
)
{
    MV_HWS_UNITS_ID unitId;
    GT_U32 fieldName;

    if(mvHwsMtipIsReducedPort(devNum, phyPortNum))
    {
        unitId = ANP_CPU_UNIT;
    }
    else if(mvHwsUsxModeCheck(devNum, phyPortNum, portMode))
    {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
    }
    /* Set pcs_cfg_done (ANP_RF)
       m_RAL_ANP.anp_units_RegFile.Port_Status[port_num].p_pcs_cfg_done.set(1'b1); */
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_STATUS_P0_PCS_CFG_DONE_E,
                                 HARRIER_ANP_UNITS_PORT0_STATUS_P0_PCS_CFG_DONE_E,
                                 PHOENIX_ANP_UNITS_PORT0_STATUS_P0_PCS_CFG_DONE_E);

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 1, NULL));
    return GT_OK;
}

/**
* @internal mvHwsAnpPortStopAn function
* @endinternal
*
* @brief   Port anp stop autoneg
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortStopAn
(
    GT_U8                devNum,
    GT_U32               phyPortNum
)
{
    MV_HWS_PORT_STANDARD        maxPortMode;
    GT_U32                      sdIndex;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;

    if ( PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSkipResetPtr[devNum][phyPortNum] )
    {
        return GT_OK;
    }

    CHECK_STATUS(mvHwsAnpPortReset(devNum, phyPortNum, GT_TRUE));

    CHECK_STATUS(mvHwsPortApGetMaxLanesPortModeGet(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum][phyPortNum].modesVector, &maxPortMode));

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, maxPortMode, &curPortParams))
    {
        return GT_BAD_PARAM;
    }
    for(sdIndex = 0; sdIndex < curPortParams.numOfActLanes; sdIndex++)
    {
        CHECK_STATUS(mvHwsAnpCmdTableClear(devNum, curPortParams.activeLanesList[sdIndex]));
    }
    hwsOsMemSetFuncPtr(&(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum][phyPortNum]), 0, sizeof(MV_HWS_AP_CFG));
    return GT_OK;
}

/**
* @internal mvHwsAnpPortRxConfig function
* @endinternal
*
* @brief   "fix" squelch value
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port
* @param[in] portMode              - configured port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortRxConfig
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_PORT_STANDARD portMode
)
{
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT   rxDefaultParameters;
    GT_U32 sdIndex;
    GT_U16 retries = 50;
    GT_BOOL allStable;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PARAM;
    }

    /* Wait for stable PLLs */
    do
    {
        allStable = GT_TRUE;
        for(sdIndex = 0; sdIndex < curPortParams.numOfActLanes; sdIndex++)
        {
            CHECK_STATUS(mvHwsSerdesRxPllLockGet(devNum, curPortParams.activeLanesList[sdIndex], (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[0]))), &allStable));
            if (!allStable)
            {
                break;
            }
        }
        cpssOsTimerWkAfter(5);
        retries--;
    } while ((retries != 0) && (allStable == GT_FALSE));
    if(allStable == GT_FALSE)
    {
        return GT_TIMEOUT;
    }

    for(sdIndex = 0; sdIndex < curPortParams.numOfActLanes; sdIndex++)
    {
        rxDefaultParameters.rxComphyC28GP4.dataRate = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.res1Sel = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.res2Sel = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.cap1Sel = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.cap2Sel = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.selmufi = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.selmuff = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.selmupi = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.selmupf = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.midpointLargeThresKLane = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.midpointSmallThresKLane = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.midpointLargeThresCLane = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.midpointSmallThresCLane = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.dfeResF0aHighThresInitLane = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.dfeResF0aHighThresEndLane = NA_16BIT;
        rxDefaultParameters.rxComphyC28GP4.squelch = 0xa;

        CHECK_STATUS(mvHwsSerdesManualRxConfig(devNum, 0, curPortParams.activeLanesList[sdIndex], (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[0]))), &rxDefaultParameters));
    }

    return GT_OK;
}


/**
* @internal mvHwsAnpPortStart function
* @endinternal
*
* @brief   Port anp start (set capabilities and start resolution)
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] apCfgPtr           - Ap parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortStart
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_AP_CFG       *apCfgPtr
)
{
    GT_U32 sdIndex, regData, fieldName;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */
    MV_HWS_PORT_STANDARD     portMode;
    MV_HWS_UNITS_ID unitId,unitIdAnp;
    MV_HWS_SERDES_SPEED  serdesSpeed;
    GT_U32 hawkLocalIndexArr[8]    = {0,1,2,6,10,14,18,22};
    GT_U32 nextPortShift = 0, laneIdx, val;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_STATUS rc;
    MV_HWS_SERDES_CONFIG_STC serdesConfig;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAnpPortStart port %d ******\n",portNum);
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
    }
#endif
    HWS_NULL_PTR_CHECK_MAC(apCfgPtr);

    CHECK_STATUS(mvHwsPortApGetMaxLanesPortModeGet(apCfgPtr->modesVector, &portMode));

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PARAM;
    }
    CHECK_STATUS(mvHwsAnpSetDefaultLinkTimer(devNum, portNum, apCfgPtr->modesVector));

    CHECK_STATUS(mvHwsAnpPortReset(devNum, portNum, GT_FALSE));

    if((cpssDeviceRunCheck_onEmulator()) && (hwsDeviceSpecInfo[devNum].devType == AC5X))
    {
        CHECK_STATUS(mvHwsAnpEmulatorForceHCDComplete(devNum, portNum));

    }

    serdesConfig.serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[0])));
    serdesConfig.refClock = apCfgPtr->refClockCfg.refClockFreq;
    serdesConfig.refClockSource = apCfgPtr->refClockCfg.refClockSource;

    for(sdIndex = 0; sdIndex < curPortParams.numOfActLanes; sdIndex++)
    {
        CHECK_STATUS(mvHwsSerdesApPowerCtrl(devNum, 0, curPortParams.activeLanesList[sdIndex], GT_TRUE, &serdesConfig));
    }

    if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        unitId = AN_CPU_UNIT;
        unitIdAnp = ANP_CPU_UNIT;
    }
    else if(mvHwsUsxModeCheck(devNum,portNum, /*for AC5X it should catch only ports 0..47*/_10GBase_KR))
    {
        unitId = AN_USX_UNIT;
        unitIdAnp = ANP_USX_UNIT;

        if(portNum % 8 != 0)
        {
            return GT_OK;
        }
    }
    else
    {
        if(hwsIsIronmanAsPhoenix())
        {
            return GT_OK;
        }
        unitId = AN_400_UNIT;
        unitIdAnp = ANP_400_UNIT;
    }

    regData = PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum][portNum].interopLinkTimer & 0x3;
    if (regData)
    {
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL1500_S_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, regData, NULL));
    }

    regData = (PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum][portNum].interopLinkTimer >> 4 ) & 0x3;
    if (regData)
    {
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL50_S_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, regData, NULL));
    }

    regData = (PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortMiscParamDbPtr[devNum][portNum].interopLinkTimer >> 2 ) & 0x3;
    if (regData)
    {
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL500_S_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, regData, NULL));
    }

    if (PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpTimerDisable)>0)
    {
        val = ((PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpTimerDisable))==1)?1:0;
        hwsOsPrintf("start timer disable:  %d \n",val);
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_OFF_S_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, val, NULL));
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_NORM_INF_S_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_NORM_INF_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_NORM_INF_S_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdAnp, fieldName, val, NULL));
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_AN_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_AP_INF_S_E,
                                     HARRIER_ANP_UNITS_PORT0_AN_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_AP_INF_S_E,
                                     PHOENIX_ANP_UNITS_PORT0_AN_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_AP_INF_S_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdAnp, fieldName, val, NULL));
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_SD_PLL_UP_MAX_TIMER_P0_REG_PLL_UP_TIME_OUT_S_INF_E,
                                     HARRIER_ANP_UNITS_PORT0_SD_PLL_UP_MAX_TIMER_P0_REG_PLL_UP_TIME_OUT_S_INF_E,
                                     PHOENIX_ANP_UNITS_PORT0_SD_PLL_UP_MAX_TIMER_P0_REG_PLL_UP_TIME_OUT_S_INF_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdAnp, fieldName, val, NULL));
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CH_DSP_RXUP_MAX_TIMER_P0_RG_DSP_RXUP_MAX_TIMER_INF_E,
                                     HARRIER_ANP_UNITS_PORT0_CH_DSP_RXUP_MAX_TIMER_P0_RG_DSP_RXUP_MAX_TIMER_INF_E,
                                     PHOENIX_ANP_UNITS_PORT0_CH_DSP_RXUP_MAX_TIMER_P0_RG_DSP_RXUP_MAX_TIMER_INF_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdAnp, fieldName, val, NULL));
    }

    /* update cmd table */
    if (PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpUseCmdTable))
    {
        rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
        if (rc != GT_OK)
        {
            return GT_BAD_PARAM;
        }
        for (laneIdx = 0; laneIdx < curPortParams.numOfActLanes; laneIdx++)
        {
            if (HWS_DEV_SILICON_TYPE(devNum) == AC5P)
            {
                nextPortShift = hawkLocalIndexArr[convertIdx.ciderIndexInUnit + laneIdx] - hawkLocalIndexArr[convertIdx.ciderIndexInUnit];
            }
            else if (HWS_DEV_SILICON_TYPE(devNum) == Harrier)
            {
                nextPortShift = 2*laneIdx;
            }
            else
            {
                nextPortShift = laneIdx;
            }

            /* serdes speed 10G */
            if ( (AP_CTRL_ADV_40G_R4_ALL_GET(apCfgPtr->modesVector)) || (AP_CTRL_10GBase_KR_GET(apCfgPtr->modesVector)) ||
                 (AP_CTRL_20GBase_KR2_GET(apCfgPtr->modesVector)) ) {
                serdesSpeed = _10_3125G;
                CHECK_STATUS(mvHwsAnpCmdRxOverride(devNum, portNum + nextPortShift, serdesSpeed));
                CHECK_STATUS(mvHwsAnpCmdTxOverride(devNum, portNum + nextPortShift, serdesSpeed));
            }
            /* serdes speed 25G */
            if ( (AP_CTRL_ADV_25G_ALL_GET(apCfgPtr->modesVector)) || (AP_CTRL_ADV_50G_CONSORTIUM_GET(apCfgPtr->modesVector)) ||
                 (AP_CTRL_ADV_100G_R4_ALL_GET(apCfgPtr->modesVector)) ) {
                serdesSpeed = _25_78125G;
                CHECK_STATUS(mvHwsAnpCmdRxOverride(devNum, portNum + nextPortShift, serdesSpeed));
                CHECK_STATUS(mvHwsAnpCmdTxOverride(devNum, portNum + nextPortShift, serdesSpeed));
            }
            /* serdes speed PAM4 */
            if ( (AP_CTRL_PAM4_GET(apCfgPtr->modesVector)) ) {
                serdesSpeed = _26_5625G_PAM4;
                CHECK_STATUS(mvHwsAnpCmdRxOverride(devNum, portNum + nextPortShift, serdesSpeed));
                CHECK_STATUS(mvHwsAnpCmdTxOverride(devNum, portNum + nextPortShift, serdesSpeed));
            }
        }
    }

    /*fieldName = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_ANP_UNITS_PORT0_CONTROL6_P0_REG_SIGDET_MODE_E : PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_REG_SIGDET_MODE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, ANP_400_UNIT, fieldName,  1, NULL));*/

    CHECK_STATUS(mvHwsAnpPortCapabilities(devNum, portNum, apCfgPtr));

    /* simulate HCD found interrupt */
    if(CPSS_DEV_IS_WM_NATIVE_OR_ASIM_MAC(devNum))
    {
        CHECK_STATUS(mvHwsAnpEmulatorOwHCDResolution(devNum, portNum, portMode));
    }

    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        mvHwsAnpPortRxConfig(devNum, portNum, portMode);
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAnpPortStart port %d ******\n",portNum);
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
    }
#endif

    return GT_OK;
}


/**
* @internal mvHwsAnpPortSkipResetSet function
* @endinternal
*
* @brief   set skip reset value (prevent from delete port to
*          reset ANP)
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortSkipResetSet (
    GT_U8                       devNum,
    GT_U32                      portNum,
    GT_BOOL                     skip
)
{
    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }
    PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSkipResetPtr[devNum][portNum] = skip;
    return GT_OK;
}

GT_STATUS mvHwsHawkAnpMuxRegDump
(
    void
)
{
    GT_UREG_DATA   fieldData;
    MV_HWS_REG_ADDR_FIELD_STC fieldRegOut;
    GT_U32  unitAddr = 0, unitIndex, unitNum, cluster, anpIdx;

    hwsOsPrintf("*************** ANP SW DB *****************\n");

   /* for each channel init the ANP default parameters */
    for (cluster = 0; cluster < MV_HWS_HAWK_CLUSTER_NUM_CNS; cluster++)
    {
        for (anpIdx = 0; anpIdx < MV_HWS_HAWK_ANP_NUM_CNS; anpIdx++)
        {
            hwsOsPrintf("cluster:%d  anpIdx:%2d  hwSerdes:%2d  outSerdesIdx:%2d\n",cluster, anpIdx, PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSdMuxDb[cluster][anpIdx].hwSerdesIdx, PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpSdMuxDb[cluster][anpIdx].outSerdesIdx);
        }
    }


    cpssOsPrintf("*************** ANP HW *****************\n");
    /* for each channel init the ANP parameters */
    for (cluster = 0; cluster < MV_HWS_HAWK_CLUSTER_NUM_CNS; cluster++)
    {
        mvUnitExtInfoGet(0, ANP_400_UNIT, cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, &unitAddr, &unitIndex,&unitNum);
        /* Init ANP 400 */
        CHECK_STATUS(genUnitRegisterFieldGet(0, 0,  0 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, ANP_400_UNIT, HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E,  &fieldData, &fieldRegOut));
        hwsOsPrintf("field: HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", unitAddr, fieldRegOut.regOffset , fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        CHECK_STATUS(genUnitRegisterFieldGet(0, 0,  1 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, ANP_400_UNIT, HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL1_E,  &fieldData, &fieldRegOut));
        hwsOsPrintf("field: HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL1_E base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", unitAddr, fieldRegOut.regOffset , fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        CHECK_STATUS(genUnitRegisterFieldGet(0, 0,  2 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, ANP_400_UNIT, HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL2_E,  &fieldData, &fieldRegOut));
        hwsOsPrintf("field: HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL2_E base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", unitAddr, fieldRegOut.regOffset , fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        CHECK_STATUS(genUnitRegisterFieldGet(0, 0,  6 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, ANP_400_UNIT, HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL3_E,  &fieldData, &fieldRegOut));
        hwsOsPrintf("field: HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL3_E base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", unitAddr, fieldRegOut.regOffset , fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        CHECK_STATUS(genUnitRegisterFieldGet(0, 0, 10 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, ANP_400_UNIT, HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL4_E, &fieldData, &fieldRegOut));
        hwsOsPrintf("field: HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL4_E base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", unitAddr, fieldRegOut.regOffset , fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        CHECK_STATUS(genUnitRegisterFieldGet(0, 0, 14 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, ANP_400_UNIT, HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL5_E, &fieldData, &fieldRegOut));
        hwsOsPrintf("field: HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL5_E base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", unitAddr, fieldRegOut.regOffset , fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        CHECK_STATUS(genUnitRegisterFieldGet(0, 0, 18 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, ANP_400_UNIT, HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL6_E, &fieldData, &fieldRegOut));
        hwsOsPrintf("field: HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL6_E base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", unitAddr, fieldRegOut.regOffset , fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        CHECK_STATUS(genUnitRegisterFieldGet(0, 0, 22 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, ANP_400_UNIT, HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL7_E, &fieldData, &fieldRegOut));
        hwsOsPrintf("field: HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL7_E base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", unitAddr, fieldRegOut.regOffset , fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);


        /* Init ANP USX */
        if (cluster < 2)
        {
            mvUnitExtInfoGet(0, ANP_USX_UNIT, 2 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, &unitAddr, &unitIndex,&unitNum);
            CHECK_STATUS(genUnitRegisterFieldGet(0, 0, 2 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, ANP_USX_UNIT, HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E, &fieldData, &fieldRegOut));
            hwsOsPrintf("field: HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", unitAddr, fieldRegOut.regOffset , fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            mvUnitExtInfoGet(0, ANP_USX_UNIT, 6 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, &unitAddr, &unitIndex,&unitNum);
            CHECK_STATUS(genUnitRegisterFieldGet(0, 0, 6 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, ANP_USX_UNIT, HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E, &fieldData, &fieldRegOut));
            hwsOsPrintf("field: HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", unitAddr, fieldRegOut.regOffset , fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            mvUnitExtInfoGet(0, ANP_USX_UNIT, 10 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, &unitAddr, &unitIndex,&unitNum);
            CHECK_STATUS(genUnitRegisterFieldGet(0, 0, 10 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, ANP_USX_UNIT, HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E, &fieldData, &fieldRegOut));
            hwsOsPrintf("field: HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", unitAddr, fieldRegOut.regOffset , fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            mvUnitExtInfoGet(0, ANP_USX_UNIT, 14 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, &unitAddr, &unitIndex,&unitNum);
            CHECK_STATUS(genUnitRegisterFieldGet(0, 0, 14 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, ANP_USX_UNIT, HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E, &fieldData, &fieldRegOut));
            hwsOsPrintf("field: HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", unitAddr, fieldRegOut.regOffset , fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            mvUnitExtInfoGet(0, ANP_USX_UNIT, 18 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, &unitAddr, &unitIndex,&unitNum);
            CHECK_STATUS(genUnitRegisterFieldGet(0, 0, 18 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, ANP_USX_UNIT, HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E, &fieldData, &fieldRegOut));
            hwsOsPrintf("field: HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", unitAddr, fieldRegOut.regOffset , fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            mvUnitExtInfoGet(0, ANP_USX_UNIT, 22 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, &unitAddr, &unitIndex,&unitNum);
            CHECK_STATUS(genUnitRegisterFieldGet(0, 0, 22 + cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS, ANP_USX_UNIT, HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E, &fieldData, &fieldRegOut));
            hwsOsPrintf("field: HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", unitAddr, fieldRegOut.regOffset , fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsAnpPortRegDump function
* @endinternal
*
* @brief   Print ANP registers information
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortRegDump
(
    GT_U8                    devNum,
    GT_U32                   phyPortNum
)
{
    GT_U32          fieldName;
    MV_HWS_UNITS_ID unitId,unitIdAn;
    MV_HWS_PORT_STANDARD portMode = _10GBase_KR;
    GT_U32  regAddr, regData;
    GT_U32          baseAddr,unitIndex, unitNum;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
        unitIdAn = AN_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, portMode)) {
        unitId = ANP_USX_UNIT;
        unitIdAn = AN_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
        unitIdAn = AN_400_UNIT;
    }
    CHECK_STATUS(mvUnitExtInfoGet(devNum, unitId, (phyPortNum), &baseAddr, &unitIndex, &unitNum ));
    hwsOsPrintf("------ ANP registers ------\n");

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_GLOBAL_CONTROL_AN_AP_TRAIN_TYPE_E,
                                 HARRIER_ANP_UNITS_GLOBAL_CONTROL_AN_AP_TRAIN_TYPE_E,
                                 PHOENIX_ANP_UNITS_GLOBAL_CONTROL_AN_AP_TRAIN_TYPE_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_GLOBAL_CONTROL(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_PWM_SOFT_RESET__E,
                                 HARRIER_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_PWM_SOFT_RESET__E,
                                 PHOENIX_ANP_UNITS_GLOBAL_CLOCK_AND_RESET_PWM_SOFT_RESET__E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_GLOBAL_CLOCK_AND_RESET(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E,
                                 HARRIER_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E,
                                 PHOENIX_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_GLOBAL_SD_MUX_CONTROL(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL4_E,
                                 HARRIER_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL4_E,
                                 PHOENIX_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL4_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_GLOBAL_SD_MUX_CONTROL1(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_GLOBAL_AN_TRAIN_TYPE_AN_TRAIN_TYPE_MODE_1G_E,
                                 HARRIER_ANP_UNITS_GLOBAL_AN_TRAIN_TYPE_AN_TRAIN_TYPE_MODE_1G_E,
                                 PHOENIX_ANP_UNITS_GLOBAL_AN_TRAIN_TYPE_AN_TRAIN_TYPE_MODE_1G_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_GLOBAL_AN_TRAIN(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_1P25G_E,
                                 HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_1P25G_E,
                                 PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE_PM_SPEED_TABLE_1P25G_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_GLOBAL_SPEED(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_10P3125G_E,
                                 HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_10P3125G_E,
                                 PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE1_PM_SPEED_TABLE_10P3125G_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_GLOBAL_SPEED_TABLE1(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_GLOBAL_SPEED_TABLE2_PM_SPEED_TABLE_56G_E,
                                 HARRIER_ANP_UNITS_GLOBAL_SPEED_TABLE2_PM_SPEED_TABLE_56G_E,
                                 PHOENIX_ANP_UNITS_GLOBAL_SPEED_TABLE2_PM_SPEED_TABLE_56G_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_GLOBAL_SPEED_TABLE2(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_GLOBAL_INTERRUPT_CAUSE_GLOBAL_INT_SUM_E,
                                 HARRIER_ANP_UNITS_GLOBAL_INTERRUPT_CAUSE_GLOBAL_INT_SUM_E,
                                 PHOENIX_ANP_UNITS_GLOBAL_INTERRUPT_CAUSE_GLOBAL_INT_SUM_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_GLOBAL_INTERRUPT(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_INTERRUPT_SUMMARY_CAUSE_INTERRUPT_CAUSE_INT_SUM_E,
                                 HARRIER_ANP_UNITS_INTERRUPT_SUMMARY_CAUSE_INTERRUPT_CAUSE_INT_SUM_E,
                                 PHOENIX_ANP_UNITS_INTERRUPT_SUMMARY_CAUSE_INTERRUPT_CAUSE_INT_SUM_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_INTERRUPT(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_GLOBAL_CONTROL2_REG_PU_TX_CONF_DELAY_E,
                                 HARRIER_ANP_UNITS_GLOBAL_CONTROL2_REG_PU_TX_CONF_DELAY_E,
                                 PHOENIX_ANP_UNITS_GLOBAL_CONTROL2_REG_PU_TX_CONF_DELAY_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_GLOBAL_CONTROL2(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_GLOBAL_CONTROL3_REG_NO_PRE_SELECTOR_E,
                                 HARRIER_ANP_UNITS_GLOBAL_CONTROL3_REG_NO_PRE_SELECTOR_E,
                                 PHOENIX_ANP_UNITS_GLOBAL_CONTROL3_REG_NO_PRE_SELECTOR_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_GLOBAL_CONTROL3(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_GLOBAL_CONTROL4_REG_SD_DFE_UPDATE_DIS_SAMP_E,
                                 HARRIER_ANP_UNITS_GLOBAL_CONTROL4_REG_SD_DFE_UPDATE_DIS_SAMP_E,
                                 PHOENIX_ANP_UNITS_GLOBAL_CONTROL4_REG_SD_DFE_UPDATE_DIS_SAMP_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_GLOBAL_CONTROL4(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_PORT_INT_SUM_E,
                                 HARRIER_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_PORT_INT_SUM_E,
                                 PHOENIX_ANP_UNITS_PORT0_INTERRUPT_CAUSE_P0_PORT_INT_SUM_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_INTERRUPT(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_SERDES_TX_SM_CONTROL_P0_SD_TX_SM_OVERRIDE_CTRL_E,
                                 HARRIER_ANP_UNITS_PORT0_SERDES_TX_SM_CONTROL_P0_SD_TX_SM_OVERRIDE_CTRL_E,
                                 PHOENIX_ANP_UNITS_PORT0_SERDES_TX_SM_CONTROL_P0_SD_TX_SM_OVERRIDE_CTRL_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_SERDES_TX_SM(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_AN_CONTROL_P0_PHY_GEN_AP_OW_E,
                                 HARRIER_ANP_UNITS_PORT0_AN_CONTROL_P0_PHY_GEN_AP_OW_E,
                                 PHOENIX_ANP_UNITS_PORT0_AN_CONTROL_P0_PHY_GEN_AP_OW_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_AN_CONTROL(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_AN_TIED_IN_P0_PM_AP_ANEG_REMOTE_READY_S_E,
                                 HARRIER_ANP_UNITS_PORT0_AN_TIED_IN_P0_PM_AP_ANEG_REMOTE_READY_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_AN_TIED_IN_P0_PM_AP_ANEG_REMOTE_READY_S_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_AN_TIED_IN(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_AN_HANG_OUT_P0_AP_IRQ_S_E,
                                 HARRIER_ANP_UNITS_PORT0_AN_HANG_OUT_P0_AP_IRQ_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_AN_HANG_OUT_P0_AP_IRQ_S_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_AN_HANG_OUT(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_AN_STATUS1_P0_AP_PWRUP_4X_S_E,
                                 HARRIER_ANP_UNITS_PORT0_AN_STATUS1_P0_AP_PWRUP_4X_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_AN_STATUS1_P0_AP_PWRUP_4X_S_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_AN_STATUS1(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_AA_CLEAR_HCD_S_E,
                                 HARRIER_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_AA_CLEAR_HCD_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_AN_STATUS2_P0_AP_AA_CLEAR_HCD_S_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_AN_STATUS2(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL1_P0_RG_MODE_50GR_OW_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL1_P0_RG_MODE_50GR_OW_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL1_P0_RG_MODE_50GR_OW_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_CONTROL1(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL2_P0_PM_SD_PHY_GEN_TX_OW_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL2_P0_PM_SD_PHY_GEN_TX_OW_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL2_P0_PM_SD_PHY_GEN_TX_OW_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_CONTROL2(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL3_P0_PM_PCS_COUPLE_OW_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL3_P0_PM_PCS_COUPLE_OW_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL3_P0_PM_PCS_COUPLE_OW_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_CONTROL3(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL4_P0_PM_AP_EN_S_OW_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL4_P0_PM_AP_EN_S_OW_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL4_P0_PM_AP_EN_S_OW_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_CONTROL4(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL5_P0_PM_RX_TRAIN_ENABLE_OW_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL5_P0_PM_RX_TRAIN_ENABLE_OW_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL5_P0_PM_RX_TRAIN_ENABLE_OW_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_CONTROL5(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_CONTROL6(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL7_P0_PLL_READY_RX_CLEAN_OW_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL7_P0_PLL_READY_RX_CLEAN_OW_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL7_P0_PLL_READY_RX_CLEAN_OW_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_CONTROL7(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_CONTROL8(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL9_P0_INT_ENABLED_OW_VAL_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL9_P0_INT_ENABLED_OW_VAL_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL9_P0_INT_ENABLED_OW_VAL_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_CONTROL9(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL10_P0_PM_PU_RX_REQ_S_OW_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL10_P0_PM_PU_RX_REQ_S_OW_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL10_P0_PM_PU_RX_REQ_S_OW_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_CONTROL10(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL11_P0_PU_RX_BOTH_IN_IDLE_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL11_P0_PU_RX_BOTH_IN_IDLE_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL11_P0_PU_RX_BOTH_IN_IDLE_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_CONTROL11(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_STATUS_P0_REG_TX_TRAIN_COMPLETE_E,
                                 HARRIER_ANP_UNITS_PORT0_STATUS_P0_REG_TX_TRAIN_COMPLETE_E,
                                 PHOENIX_ANP_UNITS_PORT0_STATUS_P0_REG_TX_TRAIN_COMPLETE_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_STATUS(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_STATUS1_P0_STAT_DSP_RXSTR_REQ_MX_S_E,
                                 HARRIER_ANP_UNITS_PORT0_STATUS1_P0_STAT_DSP_RXSTR_REQ_MX_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_STATUS1_P0_STAT_DSP_RXSTR_REQ_MX_S_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_STATUS1(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_STATUS2_P0_STAT_PM_MODE_50GR2_E,
                                 HARRIER_ANP_UNITS_PORT0_STATUS2_P0_STAT_PM_MODE_50GR2_E,
                                 PHOENIX_ANP_UNITS_PORT0_STATUS2_P0_STAT_PM_MODE_50GR2_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_STATUS2(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_STATUS3_P0_STAT_PM_SD_PU_TX_S_E,
                                 HARRIER_ANP_UNITS_PORT0_STATUS3_P0_STAT_PM_SD_PU_TX_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_STATUS3_P0_STAT_PM_SD_PU_TX_S_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_STATUS3(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_STATUS4_P0_STAT_PM_SD_TX_IDLE_S_E,
                                 HARRIER_ANP_UNITS_PORT0_STATUS4_P0_STAT_PM_SD_TX_IDLE_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_STATUS4_P0_STAT_PM_SD_TX_IDLE_S_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_STATUS4(0x%x) :0x%x\n",regAddr,regData);

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_STATUS5_P0_STAT_PM_CMEM_ADDR_S_E,
                                 HARRIER_ANP_UNITS_PORT0_STATUS5_P0_STAT_PM_CMEM_ADDR_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_STATUS5_P0_STAT_PM_CMEM_ADDR_S_E);
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg, &convertIdx));
    if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("ANP_UNITS_PORT0_STATUS5(0x%x) :0x%x\n",regAddr,regData);

    /***********AN**************/
    hwsOsPrintf("------ AN registers ------\n");
    CHECK_STATUS(mvUnitExtInfoGet(devNum, unitIdAn, (phyPortNum), &baseAddr, &unitIndex, &unitNum ));

    fieldName = AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_CONTROL_CONTROL_RESERVED_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_CONTROL(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_STATUS_LINK_PARTNER_AUTO_NEGOTIATION_ABILITY_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_STATUS(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_1_LP_SELECTOR_FIELD_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_1(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_REG_802_3AP_NEXT_PAGE_TRANSMIT_REGISTER_EXTENDED_NEXT_PAGE_TRANSMIT_REGISTER_LD_NP_MESSAGEUNFORMATTED_FIELD_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_REG_802_3AP_NEXT_PAGE_TRANSMIT_REGISTER_EXTENDED_NEXT_PAGE_TRANSMIT(0x%x) :0x%x\n",regAddr,regData);


    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0_OVERRIDE_CTRL_S_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_AG_LGTH_MATCH_CNT_S3_0_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_CLEAR_HCD_OW_S_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_REG3_RESERVED_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_G_S_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_5_AA_PWRUP_200GRCR4_S_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_5(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_6_OW_PM_NORM_X_STATE_S_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_6(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_7_RG_LOSTLOCK_VALUE_S_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_7(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_8_DET_DATA_TIMER_WINDOW_SELECT__E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_8(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_9_SW_AG_MODE_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_9(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_10_ADVERTISED_CON25GKRCR_LOCATION_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_10(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_11_REG11_RESERVED_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_11(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_12_REG12_RESERVED_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_12(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_13_AP_INTERRUPT_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_13(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_14_ADVERTISED_200G_R8_MARVELL_MODE_A15_TO_A22_BIT_LOCATION_SELECT_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_14(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_15_PM_NORM_X_STATE_S_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_15(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_16_CONSORTIUM_MP5_REGISTER_0_REG16_MESSAGEUNFORMATTED_FIELD_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_16(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_17_CONSORTIUM_MP5_REGISTER_1_REG17_OUI_23_13_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_17(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_18_CONSORTIUM_MP5_REGISTER_2_REG18_OUI_12_2_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_18(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_19_CONSORTIUM_ET_REGISTER_0_REG19_MESSAGEUNFORMATTED_FIELD_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_19(0x%x) :0x%x\n",regAddr,regData);

    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_20_CONSORTIUM_ET_REGISTER_1_REG20_RESERVED_E;
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
    regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
    hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_20(0x%x) :0x%x\n",regAddr,regData);

   fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_MARVELL_CONSORTIUM_40GR2_E;
   CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
   regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
   CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
   hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21(0x%x) :0x%x\n",regAddr,regData);

   fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_22_LINK_PARTNER_CONSORTIUM_MP5_REGISTER_0_REG22_MESSAGEUNFORMATTED_FIELD_E;
   CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
   regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
   CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
   hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_22(0x%x) :0x%x\n",regAddr,regData);

   fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_23_LINK_PARTNER_CONSORTIUM_MP5_REGISTER_1_REG23_OUI_23_13_E;
   CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
   regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
   CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
   hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_23(0x%x) :0x%x\n",regAddr,regData);

   fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_24_LINK_PARTNER_CONSORTIUM_MP5_REGISTER_2_REG24_OUI_12_2_E;
   CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
   regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
   CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
   hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_24(0x%x) :0x%x\n",regAddr,regData);

   fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_25_LINK_PARTNER_CONSORTIUM_ET_REGISTER_0_REG25_MESSAGEUNFORMATTED_FIELD_E;
   CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
   regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
   CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
   hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_25(0x%x) :0x%x\n",regAddr,regData);

   fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_26_LINK_PARTERN_CONSORTIUM_ET_REGISTER_1_REG26_RESERVED_E;
   CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
   regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
   CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
   hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_26(0x%x) :0x%x\n",regAddr,regData);

   fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27_LINK_PARTNER_CONSORTIUM_ET_REGISTER_2_REG27_RESERVED1_E;
   CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitIdAn, portMode, fieldName, &fieldReg, &convertIdx));
   regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
   CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 32, &regData));
   hwsOsPrintf("AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27(0x%x) :0x%x\n",regAddr,regData);


    return GT_OK;
}

/**
* @internal mvHwsAnpPortParallelDetectInit function
* @endinternal
*
* @brief   Init sequence for parallel detect
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectInit
(
    GT_U8                    devNum,
    GT_U32                   phyPortNum
)
{
    GT_U32          fieldName;
    MV_HWS_UNITS_ID unitId,unitIdAn;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
        unitIdAn = AN_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, /*for AC5X it should catch only ports 0..47*/_1000Base_X)) {
        unitId = ANP_USX_UNIT;
        unitIdAn = AN_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
        unitIdAn = AN_400_UNIT;
    }

    /*1.Overwrite the HCD to value of �0�*/
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_HCD_RESOLVED_OW_S_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitIdAn, fieldName, 1, NULL));
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_RG_HCD_RESOLVED_S_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitIdAn, fieldName, 0, NULL));
    /*2.Overwrite 1000BASE-X resolution to value of �1�*/
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_5_SELECTED_MODES_TO_OVERWRITE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitIdAn, fieldName, 0, NULL));
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_5_PWRUP_MODES_OW_S_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitIdAn, fieldName, 1, NULL));
    /*3.Set enable parallel detect timer*/
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_NORM_INF_S_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_NORM_INF_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_NORM_INF_S_E);
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 0, NULL));
    /*4.Set PWRUP prog mask*/
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_PWRUP_S_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_PWRUP_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_PWRUP_S_E);
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 1, NULL));
    /*5.Set PWRUP prog*/
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_PWRUP_EN_S_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_PWRUP_EN_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_PWRUP_EN_S_E);
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 1, NULL));
    /*6.Overwrite the HCD to value of �1�*/
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_HCD_RESOLVED_OW_S_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitIdAn, fieldName, 1, NULL));
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_RG_HCD_RESOLVED_S_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitIdAn, fieldName, 1, NULL));

    return GT_OK;
}

/**
* @internal mvHwsAnpPortParallelDetectReset function
* @endinternal
*
* @brief   Reset sequence for parallel detect
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectReset
(
    GT_U8                    devNum,
    GT_U32                   phyPortNum
)
{
    GT_U32          fieldName;
    MV_HWS_UNITS_ID unitId,unitIdAn;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
        unitIdAn = AN_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, /*for AC5X it should catch only ports 0..47*/_1000Base_X)) {
        unitId = ANP_USX_UNIT;
        unitIdAn = AN_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
        unitIdAn = AN_400_UNIT;
    }

    /*1.remove resolution overwrite�*/
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_5_SELECTED_MODES_TO_OVERWRITE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitIdAn, fieldName, 0, NULL));
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_5_PWRUP_MODES_OW_S_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitIdAn, fieldName, 0, NULL));
    /*2.disable parallel detect timer*/
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_NORM_INF_S_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_NORM_INF_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL_P0_RG_ST_PCSLINK_MAX_TIME_NORM_INF_S_E);
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 1, NULL));
    /*3.disable HCD overwrite*/
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_HCD_RESOLVED_OW_S_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitIdAn, fieldName, 0, NULL));
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_RG_HCD_RESOLVED_S_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitIdAn, fieldName, 0, NULL));
    /*4.disable PWRUP prog mask*/
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_PWRUP_S_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_PWRUP_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL11_P0_REG_PROG_MASK_PWRUP_S_E);
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 0, NULL));
    /*5.disable PWRUP prog*/
    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_PWRUP_EN_S_E,
                                 HARRIER_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_PWRUP_EN_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_CONTROL7_P0_REG_PROG_PWRUP_EN_S_E);
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 0, NULL));

    /*Reset AN unit and re-configure AN capabilities*/
    mvHwsAnpPortStart(devNum, phyPortNum,&(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum][phyPortNum]));

    return GT_OK;
}

/**
* @internal mvHwsAnpParallelDetectWaitForPWRUPprog function
* @endinternal
*
* @brief   Wait for CH FSM to reach PWRUP prog state
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectWaitForPWRUPprog
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum
)
{
    GT_U32          fieldName, val, i;
    MV_HWS_UNITS_ID unitId;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, /*for AC5X it should catch only ports 0..47*/_1000Base_X)) {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
    }

    fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_STATE_E,
                                 HARRIER_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_STATE_E,
                                 PHOENIX_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_STATE_E);

    for(i=0;i<5;i++)
    {
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, fieldName, &val, NULL));
        if(val == 0x11)
        {
            /*Exit PWRUP prog*/
            fieldName = mvHwsAnpFieldSet(devNum,
                                 HAWK_ANP_UNITS_PORT0_STATUS_P0_REG_PROG_PWM_DONE_S_E,
                                 HARRIER_ANP_UNITS_PORT0_STATUS_P0_REG_PROG_PWM_DONE_S_E,
                                 PHOENIX_ANP_UNITS_PORT0_STATUS_P0_REG_PROG_PWM_DONE_S_E);
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 1, NULL));
            return GT_OK;
        }
        else
        {
          cpssOsTimerWkAfter(5);
        }
    }
    return GT_FAIL;
}

/**
* @internal mvHwsAnpPortParallelDetectAutoNegSet function
* @endinternal
*
* @brief   Set AN37
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[in] autoNegEnabled        - enable/disable AN37
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectAutoNegSet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum,
    IN  GT_BOOL                  autoNegEnabled
)
{
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */
    GT_UREG_DATA data = 0;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, _1000Base_X, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    if ((curPortParams.portPcsType == PCS_NA) || (curPortParams.portPcsType >= LAST_PCS))
    {
        return GT_BAD_PARAM;
    }

    if(autoNegEnabled) data = 1;

    switch (curPortParams.portPcsType)
    {
    case MTI_CPU_SGPCS:
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_CPU_SGPCS_UNIT, CPU_SGPCS_UNIT_PORT_CONTROL_PORT_AN_ENABLE_E, data, NULL));
        break;
    case MTI_USX_PCS_LOW_SPEED:
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_CONTROL_P0_ANENABLE_E, data, NULL));
        break;
    case MTI_PCS_LOW_SPEED:
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_CONTROL_P0_ANENABLE_E, data, NULL));
        break;
    default:
        return GT_BAD_PARAM;

    }

    return GT_OK;
}

/**
* @internal mvHwsAnpPortParallelDetectOWLinkStatus function
* @endinternal
*
* @brief enable to override link_status from PCS value
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[in] owLinkStatus          - true/false if override is
*       enabled
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectOWLinkStatus
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum,
    IN  GT_BOOL                  owLinkStatus
)
{
    GT_U32          fieldName;
    MV_HWS_UNITS_ID unitId;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, /*for AC5X it should catch only ports 0..47*/_1000Base_X)) {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
    }

    if (owLinkStatus)/*link_status from PCS value is overriden with ow_val.*/
    {
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL6_P0_LINK_STATUS_OW_VAL_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL6_P0_LINK_STATUS_OW_VAL_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_LINK_STATUS_OW_VAL_E);

        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 1, NULL));

        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL6_P0_LINK_STATUS_OW_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL6_P0_LINK_STATUS_OW_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_LINK_STATUS_OW_E);

        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 1, NULL));
    }
    else
    {
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CONTROL6_P0_LINK_STATUS_OW_E,
                                     HARRIER_ANP_UNITS_PORT0_CONTROL6_P0_LINK_STATUS_OW_E,
                                     PHOENIX_ANP_UNITS_PORT0_CONTROL6_P0_LINK_STATUS_OW_E);

        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 0, NULL));
    }

    return GT_OK;
}

/**
* @internal mvHwsAnpPortParallelDetectAN37Seq function
* @endinternal
*
* @brief   Check peer side AN37 status seq.
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port
* @param[in] startSeq              - start or end seq
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectAN37Seq
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum,
    IN  GT_BOOL                  startSeq
)
{
    GT_U32 fieldName;
    MV_HWS_UNITS_ID unitId;
    GT_U32  regAddr;
    GT_U32  baseAddr, unitIndex, unitNum;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;
    GT_STATUS rc;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else if (mvHwsUsxModeCheck(devNum, phyPortNum, /*for AC5X it should catch only ports 0..47*/_1000Base_X)) {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
    }

    if(startSeq)
    {/*AN37 seq "start" steps*/
        /*Overwrite link_status towards ANP to ensure switching to AN37 enabled will not cause a restart*/
        rc = mvHwsAnpPortParallelDetectOWLinkStatus(devNum, phyPortNum, GT_TRUE);
        if(rc != GT_OK)
            return rc;

        /*Set Timer to 50msec */
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_PCS_LINK_MAX_TIMER_NORM_P0_PCS_LINK_MAX_TIMER_NORM_E,
                                     HARRIER_ANP_UNITS_PORT0_PCS_LINK_MAX_TIMER_NORM_P0_PCS_LINK_MAX_TIMER_NORM_E,
                                     PHOENIX_ANP_UNITS_PORT0_PCS_LINK_MAX_TIMER_NORM_P0_PCS_LINK_MAX_TIMER_NORM_E);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 0x773594, NULL));


        /*Enable AN37*/
        rc = mvHwsAnpPortParallelDetectAutoNegSet(devNum,phyPortNum,GT_TRUE);
        if(rc != GT_OK)
            return rc;

        /*Additional Timer Setting - Force CH SM to ST_TXRX_ON*/
        CHECK_STATUS(mvUnitExtInfoGet(devNum, unitId, phyPortNum, &baseAddr, &unitIndex, &unitNum ));
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_OVERRIDE_CTRL_E,
                                     HARRIER_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_OVERRIDE_CTRL_E,
                                     PHOENIX_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_OVERRIDE_CTRL_E);
        CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, _1000Base_X, fieldName, &fieldReg, &convertIdx));
        if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
        regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0x95));

         /*remove the link_status overwrite*/
        rc = mvHwsAnpPortParallelDetectOWLinkStatus(devNum, phyPortNum, GT_FALSE);
        if(rc != GT_OK)
            return rc;

    }
    else
    {/*AN37 seq "end" steps*/
        /*Force the CH SM to ST_NORM state*/
        CHECK_STATUS(mvUnitExtInfoGet(devNum, unitId, phyPortNum, &baseAddr, &unitIndex, &unitNum ));
        fieldName = mvHwsAnpFieldSet(devNum,
                                     HAWK_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_OVERRIDE_CTRL_E,
                                     HARRIER_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_OVERRIDE_CTRL_E,
                                     PHOENIX_ANP_UNITS_PORT0_CANNEL_SM_CONTROL_P0_CH_SM_OVERRIDE_CTRL_E);
        CHECK_STATUS(genUnitRegDbEntryGet(devNum, phyPortNum, unitId, _1000Base_X, fieldName, &fieldReg, &convertIdx));
        if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT)) convertIdx.ciderIndexInUnit = 0;
        regAddr = baseAddr + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0x175));

        /*Release CH SM force*/
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0x4));


    }

    return GT_OK;
}


/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssPxHwInit.c
*
* @brief Includes CPSS PX level basic Hw initialization functions.
*
* @version   1
********************************************************************************
*/
/* use px_prvCpssPpConfig[dev]  : for debugging MACRO PRV_CPSS_PX_PP_MAC(dev)*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/cpssHwInit/private/prvCpssPxCpssHwInitLog.h>
#include <cpss/px/cpssHwInit/cpssPxHwInit.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxHwInitLedCtrl.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/PizzaArbiter/prvCpssPxPortPizzaArbiter.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/port/private/prvCpssPortManager.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterruptsInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/px/port/private/prvCpssPxPortIfModeCfgPipeResource.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiterWS.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/cpssPxPortDynamicPizzaArbiterWorkConserving.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAUnitDrv.h>
#include <cpss/px/phy/private/prvCpssPxPhySmi.h>
#include <cpss/px/systemRecovery/catchUp/private/prvCpssPxCatchUp.h>
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpcDevCfg.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* NEXT are for 'check compilation'*/
#include <cpss/px/networkIf/cpssPxNetIf.h>

#ifdef ASIC_SIMULATION
    #define DEBUG_OPENED
#endif /*ASIC_SIMULATION*/

#ifdef DEBUG_OPENED
    #ifdef STR
        #undef STR
    #endif /*STR*/

    #define STR(strname)    \
        #strname

    #define ERRATA_NAMES                                                    \
      STR(PRV_CPSS_PX_TOKEN_BUCKET_RATE_SHAPING_DISABLE_WA_E               )\
     ,STR(PRV_CPSS_PX_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E              )\
     ,STR(PRV_CPSS_PX_RM_MG_TERMINATOR_BYPASS_ENABLE_WA_E                  )\
     ,STR(PRV_CPSS_PX_RM_TXQ_QCN_GLOBAL_MISS_CONFIG_WA_E                   )\
     ,STR(PRV_CPSS_PX_RM_AVS_VDD_LIMITS_WA_E                               )\
     ,STR(PRV_CPSS_PX_TAI_MINIMAL_TOD_DRIFT_CONSTRAIN_WA_E                 )\
     ,STR(PRV_CPSS_PX_TAI_FREQUENCY_ADJUST_TIME_WINDOW_INIT_VAL_WA_E       )\
     ,STR(PRV_CPSS_PX_RM_GOP_FCA_MISS_CONFIG_WA_E                          )\
     ,STR(PRV_CPSS_PX_SLOW_PORT_TXQ_CRDITS_WA_E                            )\
     ,"--last one--"

    static const char * errataNames[PRV_CPSS_PX_ERRATA_MAX_NUM_E+1] = {ERRATA_NAMES};
#endif /*DEBUG_OPENED*/

/* used print routine   */
#define DUMP_PRINT_MAC(x) cpssOsPrintf x
/* get the name (string) and value of field */
#define NAME_AND_VALUE_MAC(field)   #field , field

/* pointers of PX devices into prvCpssPpConfig[] --> for debugging only */
static PRV_CPSS_PX_PP_CONFIG_STC* px_prvCpssPpConfig[PRV_CPSS_MAX_PP_DEVICES_CNS];

/* definitions for interrupt coalescing */
#define MIN_INTERRUPT_COALESCING_PERIOD_CNS 320
#define MAX_INTERRUPT_COALESCING_PERIOD_CNS 5242560

/* the HW devNum is not applicable for the PX device */
#define PRV_CPSS_PX_HW_DEV_NUM_NOT_VALID_CNS    0xFFFFFFFF

/* auto calc field */
#define AUTO_CALC_FIELD_CNS 0xFFFFFFFF

/* indication that the number of CNC blocks and size of block should be
    'auto calculated' from the 'total number of cnc counters ...*/
#define CNC_AUTO_CALC                       AUTO_CALC_FIELD_CNS

/* indication for the last device in devices lists */
#define LAST_DEV_IN_LIST_CNS   0xFFFFFFFF

/**
* @struct PRV_PX_FINE_TUNING_STC
 *
 * @brief structure for the devices flavors, about table sizes
*/
typedef struct{

    CPSS_PP_DEVICE_TYPE *devTypePtr;

    /** @brief device parameters being object of fine tuning
     *  Comment:
     */
    PRV_CPSS_PX_PP_CONFIG_FINE_TUNING_STC config;

} PRV_PX_FINE_TUNING_STC;

/* list of the PIPE : Data center switch fabric High scale */
static CPSS_PP_DEVICE_TYPE devs_pipe[]=
        {
            CPSS_98PX1008_CNS,
            CPSS_98PX1012_CNS,
            CPSS_98PX1022_CNS,
            CPSS_98PX1024_CNS,
            CPSS_98PX1016_CNS,
            CPSS_98PX1015_CNS,

            LAST_DEV_IN_LIST_CNS /* must be last one */
         };

/* Pipe - fine tuning table  sizes according to devices flavors */
static PRV_PX_FINE_TUNING_STC pipeTables[]=
{
    { /* Pipe */
        devs_pipe,
        {
            {
                /* CNC section                               */
                 CNC_AUTO_CALC  /* GT_U32 cncBlocks;         */
                ,_2K           /* GT_U32 cncBlockNumEntries; */

                /* Transmit Descriptors                      */
                ,8*_1K             /*GT_U32 transmitDescr;   */

                /* Buffer Memory                             */
   /*16 MBits*/ ,16*_1M              /*GT_U32 bufferMemory;   */

                ,250                /* pipeBWCapacityInGbps - auto calculated
                    from pipe_A0_coreClockBw[] according to 'coreClock' */
            }
        }
    }   /* end : Pipe */
};
static GT_U32 pipeTables_size = sizeof(pipeTables)/sizeof(pipeTables[0]);

/* array of FEr/RM WA for the Pipe */
static GT_U32 pipe_ErrAndRmArray[]=
{
    PRV_CPSS_PX_TOKEN_BUCKET_RATE_SHAPING_DISABLE_WA_E,
    PRV_CPSS_PX_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E,
    PRV_CPSS_PX_RM_MG_TERMINATOR_BYPASS_ENABLE_WA_E,
    PRV_CPSS_PX_RM_TXQ_QCN_GLOBAL_MISS_CONFIG_WA_E,
    PRV_CPSS_PX_RM_AVS_VDD_LIMITS_WA_E,
    PRV_CPSS_PX_TAI_MINIMAL_TOD_DRIFT_CONSTRAIN_WA_E,
    PRV_CPSS_PX_TAI_FREQUENCY_ADJUST_TIME_WINDOW_INIT_VAL_WA_E,
    PRV_CPSS_PX_RM_GOP_FCA_MISS_CONFIG_WA_E,
    PRV_CPSS_PX_SLOW_PORT_TXQ_CRDITS_WA_E,

    LAST_DEV_IN_LIST_CNS/* must be last */
};

/* Array of units that not generated interrupts */
static PRV_CPSS_DXCH_UNIT_ENT prvCpssPxNoInterruptsGenerationUnits[]={
        PRV_CPSS_DXCH_UNIT_GOP_E,
        PRV_CPSS_DXCH_UNIT_MIB_E,
        PRV_CPSS_DXCH_UNIT_SERDES_E,
        PRV_CPSS_DXCH_UNIT_CNC_0_E,
        PRV_CPSS_DXCH_UNIT_SMI_0_E,
        PRV_CPSS_DXCH_UNIT_LED_0_E
};

GT_STATUS prvCpssPxEgressInit
(
    IN GT_SW_DEV_NUM    devNum,
    IN CPSS_PX_INIT_INFO_STC   *initParamsPtr
);
GT_STATUS prvCpssPxPortIfCfgInit
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_BOOL          allowHwAccessOnly
);
GT_STATUS prvCpssPxHwPpPhase1ShadowInit
(
    IN GT_SW_DEV_NUM           devNum,
    IN CPSS_PX_INIT_INFO_STC   *initParamsPtr
);
GT_STATUS prvCpssPxPortDynamicPATxQHighSpeedPortInit
(
    IN  GT_SW_DEV_NUM  devNum
);
GT_STATUS cpssPxPortTxGlobalDescLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32  limit
);

extern GT_STATUS TxQPort2TxDMAMapSet
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_U32 txqNum,
    IN GT_U32 txDmaNum
);

static GT_STATUS prvCpssPxUnitInterruptGenerationCheck
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_U32 causeReg
);

GT_STATUS prvCpssPxPFCResponsePhysPort2TxQMapSet
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_PHYSICAL_PORT_NUM physPort,
    IN GT_U32 txqNum
);

/**
* @internal hwPpPxRegsDefault function
* @endinternal
*
* @brief   Set mandatory default values for the device.
*         handle init stage 'Errata and RMs'
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] initParamsPtr            - PX device init parameters.
* @param[in] microInitEn              - microInit configuration set
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
*/
static GT_STATUS hwPpPxRegsDefault
(
    IN GT_SW_DEV_NUM                                    devNum,
    IN CPSS_PX_INIT_INFO_STC                            *initParamsPtr,
    IN GT_BOOL                                          microInitEn
)
{
    GT_STATUS rc = GT_OK; /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regData;    /* register data */
    GT_U32 ii;            /* iterator */
    GT_U32 profile;       /* scheduler profile */
    PRV_CPSS_PX_PP_CONFIG_STC *pDev;
    GT_U32  portMacNum;   /* MAC number */
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    pDev = PRV_CPSS_PX_PP_MAC(devNum);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* set <Tsen Reset> , otherwise can not get temperature from the sensors */
        regAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                      DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
                                                 CAST_SW_DEVNUM(devNum), regAddr, 8, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    pDev->hwInfo.smi_support.activeSMIList[0] = (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E;

    /* TAIs UNRESET*/
    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).TAICtrlReg0;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, 1);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Disable global legacy exact shaping and scheduling */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.schedulerConfig.schedulerConfig;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 22, 1, 0);
    if(rc != GT_OK)
    {
        return rc;
    }


    for(profile = 0; profile < 16; profile++)
    {
        /* bit  9 :Configure Shaper byte count mode to transmitted packet size */
        /* bit 10 :Configure Scheduler byte count mode to transmitted packet size */
        /* bit 11 :Configure Scheduler byte count mode to enable High speed */
        regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.profileByteCountModification[profile];
        rc = prvCpssHwPpWriteRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, 0x0E00, 0x0E00);

       if(rc != GT_OK)
        {
            return rc;
        }

    }

    /* Enable shaper token update, with 64 clock granularity */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).shaper.tokenBucketUpdateRate;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 29, 3, 1);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* set comon mode voltage range of the clock input. bit 4 of
           Reset and Init Controller/DFX Server Units - specific registers/Device Control 0
           should be '0' */
        regAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl0;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
                                                 CAST_SW_DEVNUM(devNum), regAddr, 4, 1, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum) == GT_TRUE)
    {
        GT_U32 numOfRxDMAs;
        GT_U32 numOfCPURxDMAs;
        /* Enable SERDES delay write */
        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.globalRegs.genxsRateConfig;
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 23, 1, 1);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Enable the Flow Control indication pause toward the port to allow
            FCA functionality. In A0 this bit by default was 1. */
        numOfRxDMAs    = PRV_CPSS_PX_PP_MAC(devNum)->genInfo.numOfPorts;
        numOfCPURxDMAs = 1;
        for (ii = 0; ii < numOfRxDMAs+numOfCPURxDMAs; ii++)
        {
            if(ii < numOfRxDMAs)
            {
                PRV_CPSS_SKIP_NOT_EXIST_PORT_MAC(devNum,ii);
            }

            regAddr = PRV_PX_REG1_UNIT_RXDMA_MAC(devNum).
                                    singleChannelDMAConfigs.SCDMAConfig0[ii];
            rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
                                                    regAddr,
                                                    28, /* Flow Control En<<%n>> filed offset */
                                                    1,  /* field length */
                                                    1); /* Enable Flow Control */
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        /* now take care of CPU */
    }

    /* Write PRNG seed */
    /* First seed word */
    regAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).PCPLFSRSeed[0];
    rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, 0x00000000);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Second seed word */
    regAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).PCPLFSRSeed[1];
    rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, 0xFFF00000);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Assert load signal */
    regAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).PCPPrngCtrl;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, 1);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Deassert load signal */
    regAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).PCPPrngCtrl;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (GT_TRUE != microInitEn)
    {
        rc = prvCpssPxEgressInit(devNum, initParamsPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* Set and enable CPU port to be 16 */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum, 1).global.
        globalDQConfig.globalDequeueConfig;
    regData = 0x1 /* enable CPU port */ |
        (0x6 /* 10(ports per DQ) * 1(DQ index)  + 6 (relative portnum) */<< 1);
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 8, regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(GT_TRUE == PRV_CPSS_PX_ERRATA_GET_MAC(devNum, PRV_CPSS_PX_RM_MG_TERMINATOR_BYPASS_ENABLE_WA_E))
    {
        /* SoC Control Register */
        regAddr = 0x00018204;
        rc = cpssDrvPpHwInternalPciRegRead(CAST_SW_DEVNUM(devNum),
                                           CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                           regAddr, &regData);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Bit [19] conf_mg_term_bypass */
        U32_SET_FIELD_MAC(regData, 19, 1, 1);

        rc = cpssDrvPpHwInternalPciRegWrite(CAST_SW_DEVNUM(devNum),
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            regAddr, regData);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(GT_TRUE == PRV_CPSS_PX_ERRATA_GET_MAC(devNum, PRV_CPSS_PX_RM_TXQ_QCN_GLOBAL_MISS_CONFIG_WA_E))
    {
        /* CN Global Configuration */
        regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).CNGlobalConfig;
        rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regData);
        if(rc != GT_OK)
        {
            return rc;
        }
        /* QcnDropEn - Enables dropping of CN packets */
        U32_SET_FIELD_MAC(regData, 1, 1, 1);
        /* CnEnCnm4CnUntagged - CNM is generated regardless of the CN tag state of the sampled frame */
        U32_SET_FIELD_MAC(regData, 6, 1, 1);

        rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, regData);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(GT_TRUE == PRV_CPSS_PX_ERRATA_GET_MAC(devNum, PRV_CPSS_PX_RM_AVS_VDD_LIMITS_WA_E) &&
       (initParamsPtr->powerSuppliesNumbers == CPSS_PX_POWER_SUPPLIES_DUAL_E))
    {
        if (PRV_CPSS_PP_MAC(devNum)->coreClock <= 450)
        {
            /* AVS Enabled Control */
            regAddr =
                PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnits.DFXServerRegs.AVSEnabledCtrl[0];
            rc = prvCpssDrvHwPpResetAndInitControllerReadReg(CAST_SW_DEVNUM(devNum), regAddr, &regData);
            if( GT_OK != rc )
            {
                return rc;
            }

            /* avs_low_vdd_limit */
            U32_SET_FIELD_MAC(regData, 3, 10, 0x22);
            /* avs_high_vdd_limit */
            U32_SET_FIELD_MAC(regData, 13, 10, 0x22);

            rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(CAST_SW_DEVNUM(devNum), regAddr, regData);
            if( GT_OK != rc )
            {
                return rc;
            }
        }
    }

    if(GT_TRUE == PRV_CPSS_PX_ERRATA_GET_MAC(devNum, PRV_CPSS_PX_RM_GOP_FCA_MISS_CONFIG_WA_E))
    {
        for (portMacNum = 0; portMacNum < PRV_CPSS_PP_MAC(devNum)->numOfPorts; portMacNum++)
        {
            PRV_CPSS_SKIP_NOT_EXIST_PORT_MAC(devNum, portMacNum);

            /* Always activate the Send On Change Only feature by configuring the <Send On Change Only> field in the FCA Control
               register to 0x1 */
            regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
            rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 8, 1, 1);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if(GT_TRUE == PRV_CPSS_PX_ERRATA_GET_MAC(
        devNum, PRV_CPSS_PX_TAI_FREQUENCY_ADJUST_TIME_WINDOW_INIT_VAL_WA_E))
    {
        /* RM-7969777 WA implemented for for BobK and above. */
        /*  Frequency adjust time window should be internalized and configured to 0x1 only (default is 0x0!)*/
        regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).frequencyAdjustTimeWindow;
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 3, 1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* CG port 12 configuration - for bring up !!! */
    PRV_CPSS_PX_REG1_CG_CONVERTERS_RESETS_REG_MAC(devNum, 12, &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
         rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),regAddr, 26, 1, 1);
         if (rc != GT_OK)
         {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "CG regfile unreset operation failed");
         }
    }

    return GT_OK;
}


/**
* @internal hwPpPhase1NeededWaSettings function
* @endinternal
*
* @brief   Part 1 of "phase 1"
*         initialize all the WA that needed for the device
*         function called from the "phase 1" sequence.
* @param[in] devNum                   - The Pp's device number to set the errata Wa needed.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on failing memory allocation
*/
static GT_STATUS hwPpPhase1NeededWaSettings
(
    IN GT_SW_DEV_NUM    devNum
)
{
    GT_U32  ii;/*iterator*/

    /* set PIPE device Errata */
    for(ii = 0 ; pipe_ErrAndRmArray[ii] != LAST_DEV_IN_LIST_CNS ; ii++)
    {
        PRV_CPSS_PX_ERRATA_SET_MAC(devNum, pipe_ErrAndRmArray[ii]);
    }

    return GT_OK;
}

/**
* @struct PLL0_FLD_DEF_STC
 *
 * @brief A structure defines PPL0 register and field
*/
typedef struct{

    /** register address */
    GT_U32 addr;

    /** field start bit */
    GT_U32 startBit;

    /** field length */
    GT_U32 length;

} PLL0_FLD_DEF_STC;
/**
* @struct REV_x_CORECLOCK_LIST_CONV_STC
 *
 * @brief A structure to Core Clock PLL config tables pointers
*/
typedef struct{

    CPSS_PP_FAMILY_TYPE_ENT family;

    CPSS_PP_SUB_FAMILY_TYPE_ENT subFamily;

    /** @brief The device's revision number.
     *  tablePtr   - pointer to PLL config to core clock table
     */
    GT_U32 revision;

    PLL0_FLD_DEF_STC sar1_pll0_def;

    GT_U16 *tablePtr;

    /** size of table */
    GT_U16 tableSize;

} REV_x_CORECLOCK_LIST_CONV_STC;

extern GT_U32 simCoreClockOverwrittenGet(GT_VOID);
/**
* @internal hwPpAutoDetectCoreClock function
* @endinternal
*
* @brief   Retrieves Core Clock value
*
* @param[in] devNum                   - The Pp's device numer.
*
* @param[out] coreClkPtr               - Pp's core clock in MHz
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_STATE             - can't map HW value to core clock value.
* @retval GT_FAIL                  - otherwise.
*
* @note Function is using hard coded registers address since registers DB is not
*       yet initialized.
*
*/
static GT_STATUS hwPpAutoDetectCoreClock
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32  *coreClkPtr
)
{
    GT_STATUS   rc;
    GT_U32 data = 0;
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;/* pointer to common device info */
    GT_BOOL valueReadFromHw = GT_TRUE;

    devPtr = PRV_CPSS_PP_MAC(devNum);

    switch(devPtr->devFamily)
    {
        case CPSS_PX_FAMILY_PIPE_E:
        {
            const REV_x_CORECLOCK_LIST_CONV_STC *clockTbl = NULL;
            CPSS_TBD_BOOKMARK_PIPE
            static const GT_U16 clockMHzByPLL0_pipe_Status_x_revision_0[] =
            {
                    /* 0 */         500,
                    /* 1 */         450,
                    /* 2 */         288,/*287.5MHZ*/
                    /* 3 */         350,
                    /* 4 */         531,/*531.25*/
                    /* 5 */         550,
                    /* 6 */         575,
                    /* 7 */         0/*PLL bypass (Reserved) */
            };

#define CLK_TBL(T) (GT_U16*)&T[0], (GT_U16)(sizeof(T)/sizeof(T[0]))

            static const REV_x_CORECLOCK_LIST_CONV_STC fam_subFam_rev_x_coreClockList[] =
            {     /*                       family,                  subfamily,      rev, fldAddr, startBit, len, table    */
                 { CPSS_PX_FAMILY_PIPE_E    , CPSS_PP_SUB_FAMILY_NONE_E,         0, { 0x000F8204,  21,     3} , CLK_TBL(clockMHzByPLL0_pipe_Status_x_revision_0)        }
            };

            GT_U32 i;

            /*-------------------------------------------------*/
            /* find greatest revision smaller or equal to given */
            /*-------------------------------------------------*/

            /* find first suitable revision */
            for (i = 0 ; i < sizeof(fam_subFam_rev_x_coreClockList)/sizeof(fam_subFam_rev_x_coreClockList[0]); i++)
            {
                if (fam_subFam_rev_x_coreClockList[i].family    == PRV_CPSS_PX_PP_MAC(devNum)->genInfo.devFamily &&
                    fam_subFam_rev_x_coreClockList[i].subFamily == PRV_CPSS_PX_PP_MAC(devNum)->genInfo.devSubFamily)
                {
                    if (fam_subFam_rev_x_coreClockList[i].revision <= PRV_CPSS_PX_PP_MAC(devNum)->genInfo.revision)
                    {
                        if (clockTbl != NULL)
                        {
                            if (fam_subFam_rev_x_coreClockList[i].revision <= clockTbl->revision)
                            {
                                continue;
                            }
                        }
                        clockTbl = fam_subFam_rev_x_coreClockList+i;
                    }
                }
            }
            if (clockTbl == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "devNum[%d] : no information for 'coreClock' register",devNum);
            }

            /* read from HW */
            if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                /* retrieve PP Core Clock from "Device SAR2 PLL0 " */
                /* /Reset and Init Controller/DFX Server Units - BC2 specific registers/Device SAR2                */
                /* Reset And Init Controller/DFX Units/Units/DFX Server Registers/Device Sample at Reset (SAR) Status<1> */
                rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
                             CAST_SW_DEVNUM(devNum),
                             clockTbl->sar1_pll0_def.addr,
                             clockTbl->sar1_pll0_def.startBit,
                             clockTbl->sar1_pll0_def.length, &data);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            else
            {
                valueReadFromHw = GT_FALSE;
            }
            if (data >= clockTbl->tableSize)  /*Bypass; PLL0 is bypassed.*/
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "devNum[%d] : expected value less than [%d] got [%d]",
                    devNum,
                    clockTbl->tableSize,
                    data);
            }
            *coreClkPtr = (GT_U32)clockTbl->tablePtr[data];
        }
        break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "devNum[%d] : devFamily[%d] was not implemented",
                devNum,
                devPtr->devFamily);
    }

    if( valueReadFromHw == GT_FALSE)
    {
        #ifdef ASIC_SIMULATION
        if(simCoreClockOverwrittenGet() != 0)
        {
            *coreClkPtr = simCoreClockOverwrittenGet();
        }
        #endif
    }

    return GT_OK;
}

/**
* @internal devDbBusBaseAddrAttachKeyGet function
* @endinternal
*
* @brief   Calculate unique BusBaseAddr key for prvCpssPpConfigDevDbBusBaseAddrAttach call.
*
* @param[in] initParamsPtr            - Packet processor hardware specific parameters.
*                                       bus Base Addr Key value
*/
static GT_UINTPTR devDbBusBaseAddrAttachKeyGet
(
    IN      CPSS_PX_INIT_INFO_STC               *initParamsPtr
)
{
    CPSS_HW_INFO_STC *hwInfoPtr;

    hwInfoPtr = &(initParamsPtr->hwInfo);

    if (initParamsPtr->mngInterfaceType == CPSS_CHANNEL_PEX_KERNEL_E)
    {
        return (3 |
            (hwInfoPtr->hwAddr.busNo << 2) |
            (hwInfoPtr->hwAddr.devSel << 12) |
            (hwInfoPtr->hwAddr.funcNo << 22));
    }
    else
    {
        return hwInfoPtr->resource.switching.start;
    }
}
/**
* @internal setSipVersion function
* @endinternal
*
* @brief   set the sip version (legacy / 5 / 5_10 / 5_15 /...).
*
* @param[in] devNum                   -  the device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS setSipVersion
(
    IN GT_SW_DEV_NUM    devNum
)
{
    /* supports sip 5 to 5_20 */
    PRV_CPSS_SIP_SET_MAC(devNum,CPSS_GEN_SIP_5_20_E);

    /* set additional flags needed by 'higher' SIP version */
    PRV_CPSS_SIP_SET_MAC(devNum,CPSS_GEN_SIP_5_16_E);
    PRV_CPSS_SIP_SET_MAC(devNum,CPSS_GEN_SIP_5_15_E);
    PRV_CPSS_SIP_SET_MAC(devNum,CPSS_GEN_SIP_5_10_E);
    PRV_CPSS_SIP_SET_MAC(devNum,CPSS_GEN_SIP_5_E);

    return GT_OK;
}

/**
* @internal setSip5FlexInfo function
* @endinternal
*
* @brief   set sip5 flex parameters.
*
* @param[in] devNum                   -  the device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS setSip5FlexInfo
(
    IN GT_SW_DEV_NUM    devNum
)
{
    PRV_CPSS_PX_PP_CONFIG_STC *pxDevPtr = PRV_CPSS_PX_PP_MAC(devNum);

    pxDevPtr->genInfo.hwInfo.dfxServer.supported = GT_TRUE;
    pxDevPtr->genInfo.hwInfo.dfxServer.dfxMultiInstance = GT_FALSE;
#ifdef  GM_USED
    /* the GM device not supports the DFX server unit */
    pxDevPtr->genInfo.hwInfo.dfxServer.supported = GT_FALSE;
#endif  /*GM_USED*/


    pxDevPtr->hwInfo.multiDataPath.numTxqDq = PRV_CPSS_PX_NUM_DQ_CNS;/* there is TXQ,dq per 'data path' */
    pxDevPtr->hwInfo.multiDataPath.txqDqNumPortsPerDp = PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS;

    return GT_OK;
}
/**
* @internal maxIterationsOfBusyWaitSet function
* @endinternal
*
* @brief   set busy wait parameters.
*
* @param[in] devNum                   -  the device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS maxIterationsOfBusyWaitSet
(
    IN GT_SW_DEV_NUM    devNum,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT          mngInterfaceType
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = PRV_CPSS_PP_MAC(devNum);

    /* value taken from : prvCpssPxPhyPortSmiRegisterWrite(...) and from
        smiStatusCheck(...) */
    devPtr->maxIterationsOfSmiWait = PRV_CPSS_SMI_RETRY_COUNTER_CNS;

    /* must be done before doing any busy wait on a device  */
    if (CPSS_CHANNEL_IS_PCI_COMPATIBLE_MAC(mngInterfaceType))
    {
        devPtr->maxIterationsOfBusyWait =
            66000000;/* 66 million for PCI bus with rate of 66MHz ,
                        and every read operation is about 2 clocks
                        give us about 2 second.
                        For PEX x1 it give us about 1 second */
    }
    else
    {
        switch(mngInterfaceType)
        {
            case CPSS_CHANNEL_SMI_E:
            case CPSS_CHANNEL_TWSI_E:
                devPtr->maxIterationsOfBusyWait =
                    78125; /* For SMI MDC 20Mhz the read/write register longs
                             then near 12.8 micro seconds.
                             78125 read registers give us about 1 second */
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(mngInterfaceType);
        }
    }
#ifdef GM_USED
    /* the GM is heavy and query into the GM takes it's own time.
       no need more the 30 iterations , to understand 'not done' */
    devPtr->maxIterationsOfBusyWait = 30;
#elif defined ASIC_SIMULATION
    /* the simulation devices also need to do wait with sleep between iterations
        so the other task that should finish the triggered action can finish */
    devPtr->maxIterationsOfBusyWait = 500; /* with the 10 mili sleep between iterations --> 5 seconds */
#endif /*ASIC_SIMULATION*/

    return GT_OK;
}

/**
* @internal initPortTypeArray function
* @endinternal
*
* @brief   initialize phyPortInfoArray[]
*
* @param[in] devNum                   -  the device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS initPortTypeArray
(
    IN GT_SW_DEV_NUM    devNum
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = PRV_CPSS_PP_MAC(devNum);
    GT_U32  port;

    /* allocation of physical ports information - phyPortInfoArray */
    devPtr->phyPortInfoArray = (PRV_CPSS_PORT_INFO_ARRAY_STC*)
        cpssOsMalloc(sizeof(PRV_CPSS_PORT_INFO_ARRAY_STC) * devPtr->numOfPorts);

    if (devPtr->phyPortInfoArray == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(devPtr->phyPortInfoArray, 0,
                 (sizeof(PRV_CPSS_PORT_INFO_ARRAY_STC) * devPtr->numOfPorts));

    /* init The port info struct to all ports */
    for (port = 0; port < devPtr->numOfPorts; port++)
    {
        if(!CPSS_PORTS_BMP_IS_PORT_SET_MAC(&devPtr->existingPorts , port))
        {
            /* set the non-exists ports*/
            devPtr->phyPortInfoArray[port].portType = PRV_CPSS_PORT_NOT_EXISTS_E;
                continue;
        }

        devPtr->phyPortInfoArray[port].isFlexLink = GT_TRUE;
        devPtr->phyPortInfoArray[port].portType = PRV_CPSS_PORT_XLG_E;
        devPtr->phyPortInfoArray[port].portTypeOptions = PRV_CPSS_XG_PORT_CG_SGMII_E;
        devPtr->phyPortInfoArray[port].vctLengthOffset = 0;
        devPtr->phyPortInfoArray[port].portIfMode = CPSS_PORT_INTERFACE_MODE_NA_E;
        devPtr->phyPortInfoArray[port].portSpeed = CPSS_PORT_SPEED_NA_E;

        devPtr->phyPortInfoArray[port].portRefClock.enableOverride = GT_FALSE;
        devPtr->phyPortInfoArray[port].portRefClock.portRefClockSource = CPSS_PORT_REF_CLOCK_SOURCE_LAST_E;

        /* initializing port SMI and PHY address DB to indicate that these parameters have */
        /* not been set yet. */
        PRV_CPSS_PHY_SMI_PORT_ADDR_MAC(devNum,port)     = PRV_CPSS_PHY_SMI_NO_PORT_ADDR_CNS;
        PRV_CPSS_PHY_SMI_INSTANCE_MAC(devNum,port)      = PRV_CPSS_PHY_INVALID_SMI_INSTANCE_CNS;

        /* initializing the SMI interfaces to relevant port group */
        /* Each port's PHY is access by its associated port group's SMI*/
        PRV_CPSS_PHY_SMI_GROUP_PORT_MAC(devNum,port)=
            PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), port);

    }

    return GT_OK;
}
/**
* @internal initPortMibShadow function
* @endinternal
*
* @brief   initialize portsMibShadowArr[]
*
* @param[in] devNum                   -  the device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS initPortMibShadow
(
    IN GT_SW_DEV_NUM    devNum
)
{
    PRV_CPSS_PX_PP_CONFIG_STC *pxDevPtr = PRV_CPSS_PX_PP_MAC(devNum);
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = &pxDevPtr->genInfo;
    GT_U32  port;

    for (port = 0; port < devPtr->numOfPorts; port++)
    {
        if(!CPSS_PORTS_BMP_IS_PORT_SET_MAC(&devPtr->existingPorts , port))
        {
            /* skip the non-exists ports*/
            continue;
        }

        /* allocate the memory for the MIB shadow */
        PRV_CPSS_PX_PP_MAC(devNum)->port.portsMibShadowArr[port] =
            cpssOsMalloc(sizeof(PRV_CPSS_PX_PORT_XLG_MIB_SHADOW_STC));

        if(PRV_CPSS_PX_PP_MAC(devNum)->port.portsMibShadowArr[port] == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        cpssOsMemSet(PRV_CPSS_PX_PP_MAC(devNum)->port.portsMibShadowArr[port],
                     0,sizeof(PRV_CPSS_PX_PORT_XLG_MIB_SHADOW_STC));

        /* set "clear on read" for XGL MAC counters default to true */
        PRV_CPSS_PX_PP_MAC(devNum)->port.portsMibShadowArr[port]->clearOnReadEnable = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal setCncParameters function
* @endinternal
*
* @brief   initialize cnc parameters
*
* @param[in] devNum                   -  the device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS setCncParameters
(
    IN GT_SW_DEV_NUM    devNum
)
{
    /* CNC parameter initialization */
    PRV_CPSS_PX_PP_HW_INFO_CNC_MAC(devNum).cncUnits = 1;
    PRV_CPSS_PX_PP_HW_INFO_CNC_MAC(devNum).cncBlocks = 2;
    PRV_CPSS_PX_PP_HW_INFO_CNC_MAC(devNum).cncBlockNumEntries = _1K;
    PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.block = 0;
    PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.blockSize = 0;
    PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.curIdx = 0;
    PRV_CPSS_PX_PP_MAC(devNum)->cncDmaDesc.unreadCount = 0;

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* The GM not support the DFX */
        PRV_CPSS_PX_PP_HW_INFO_CNC_MAC(devNum).cncUploadUseDfxUnit = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal hwPpPhase1Part1 function
* @endinternal
*
* @brief   Part 1 of "phase 1"
*         Performs configures that should be done at the start of "Phase 1" :
*         initialize the cpssDriver
*         build the DB needed for the device
*         function called from the "phase 1" sequence.
*
* @param[out] deviceTypePtr            - (pointer to) The Pp's device type.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_BAD_VALUE             - the driver found unknown device type
* @retval GT_NOT_IMPLEMENTED       - the CPSS was not compiled properly
* @retval GT_OUT_OF_RANGE          - the value is out of range
*
* @note was called "pre start init"
*
*/
static GT_STATUS hwPpPhase1Part1
(
    IN      GT_SW_DEV_NUM                       devNum,
    IN      CPSS_PX_INIT_INFO_STC               *initParamsPtr,
    OUT     CPSS_PP_DEVICE_TYPE                 *deviceTypePtr
)
{
    GT_STATUS           rc;
    GT_U32              port;   /* iterator */
    GT_UINTPTR busBaseAddrAttachKey; /* key for prvCpssPpConfigDevDbBusBaseAddrAttach call */

    /* info needed by the cpssDriver */
    PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC drvPpPhase1InInfo;
    /* info returned from the cpssDriver */
    PRV_CPSS_DRV_PP_PHASE_1_OUTPUT_INFO_STC drvPpPhase1OutInfo;
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;/* pointer to common device info */

    /* prepare INPUT info for the cpssDriver */
    drvPpPhase1InInfo.mngInterfaceType = initParamsPtr->mngInterfaceType;
    drvPpPhase1InInfo.hwInfo[0] = initParamsPtr->hwInfo;
    drvPpPhase1InInfo.numOfPortGroups = 1; /* non multi-port-group device */
    drvPpPhase1InInfo.ppHAState = CPSS_SYS_HA_MODE_ACTIVE_E;
    drvPpPhase1InInfo.devIdLocationType = PRV_CPSS_DRV_DEV_ID_LOCATION_DEVICE_ADDR_0x0000004C_E;
    drvPpPhase1InInfo.isrAddrCompletionRegionsBmp = initParamsPtr->isrAddrCompletionRegionsBmp;
    drvPpPhase1InInfo.appAddrCompletionRegionsBmp = initParamsPtr->appAddrCompletionRegionsBmp;

    /* call cpssDriver to initialize itself - phase 1 */
    rc = prvCpssDrvHwPpPhase1Init(CAST_SW_DEVNUM(devNum),
                                  &drvPpPhase1InInfo,
                                  &drvPpPhase1OutInfo);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"hwPpPhase1Part1(..) : prvCpssDrvHwPpPhase1Init(..)");
    }

    /* set the out put parameter */
    *deviceTypePtr = drvPpPhase1OutInfo.devType;


    if(! CPSS_IS_PX_FAMILY_MAC(drvPpPhase1OutInfo.devFamily))
    {
        /* we got bad value from the cpssDriver !
            that means that there is problem in the cpssDriver or that the
            PP , is not PX supported device.
        */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "PP is not PX supported device");
    }

    /* allocate the memory for the device DB */
    PRV_CPSS_PP_CONFIG_ARR_MAC[devNum] = cpssOsMalloc(sizeof(PRV_CPSS_PX_PP_CONFIG_STC));

    if (PRV_CPSS_PP_MAC(devNum) == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "hwPpPhase1Part1(..) : PRV_CPSS_PP_MAC(devNum) == NULL");
    }

    px_prvCpssPpConfig[devNum] = PRV_CPSS_PP_CONFIG_ARR_MAC[devNum];

    /* reset the device DB */
    cpssOsMemSet(PRV_CPSS_PP_MAC(devNum) , 0 ,
                                sizeof(PRV_CPSS_PX_PP_CONFIG_STC));

    /* allocate the memory for the non-shared device DB */
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]) = cpssOsMalloc(sizeof(PRV_CPSS_NON_SHARED_DEVICE_SPECIFIC_DATA));
    if (PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]) == NULL)
    {
        CPSS_LOG_INFORMATION_MAC("hwPpPhase1Part1(..) : PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]) == NULL : GT_OUT_OF_CPU_MEM");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

       /* reset the device DB */
    cpssOsMemSet(PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]) , 0 ,
                                sizeof(PRV_CPSS_NON_SHARED_DEVICE_SPECIFIC_DATA));

    devPtr = PRV_CPSS_PP_MAC(devNum);
    /*******************************************/
    /* start filling the info to the device DB */
    /*******************************************/
    /* set the device number */
    devPtr->devNum = devNum;

    /* fill info retrieved from the cpssDriver */
    devPtr->devType = drvPpPhase1OutInfo.devType;
    devPtr->revision = drvPpPhase1OutInfo.revision;
    devPtr->devFamily = drvPpPhase1OutInfo.devFamily;
    devPtr->numOfPorts = drvPpPhase1OutInfo.numOfPorts;
    devPtr->existingPorts = drvPpPhase1OutInfo.existingPorts;
    devPtr->numOfVirtPorts = devPtr->numOfPorts;

    /* cpuPortPortGroupId for such boards configured in hwPpPhase1Part1 */
    devPtr->portGroupsInfo.cpuPortPortGroupId = devPtr->netifSdmaPortGroupId = 0;
    devPtr->noTraffic2CPU = GT_FALSE;/*TXQ PIZZA code should get it directly ... not from 'init' stage*/
    devPtr->baseCoreClock = 0;/* this value has no meaning for PX device */

    /* set default CPU port mode to none */
    devPtr->cpuPortMode = CPSS_NET_CPU_PORT_MODE_NONE_E;

    devPtr->devSubFamily = drvPpPhase1OutInfo.devSubFamily;
    /*set the sip version (5 / 5_10 / 5_15 / 5_20 /...)*/
    rc = setSipVersion(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* set the device as 'non multi-port-groups' device '*/
    rc = prvCpssNonMultiPortGroupsDeviceSet(CAST_SW_DEVNUM(devNum));
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpPhase1Part1(..) : prvCpssNonMultiPortGroupsDeviceSet ");
    }
    /* from this point on we can start use the macros of:
       PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC,
       PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC
    */

    devPtr->useIsr = (initParamsPtr->hwInfo.irq.switching == CPSS_PP_INTERRUPT_VECTOR_NOT_USED_CNS) ?
                        GT_FALSE : GT_TRUE;

    /* attach the device with it base address , needed to support re-init after
       'shutdown' */
    busBaseAddrAttachKey = devDbBusBaseAddrAttachKeyGet(
            initParamsPtr);
    rc = prvCpssPpConfigDevDbBusBaseAddrAttach(CAST_SW_DEVNUM(devNum), busBaseAddrAttachKey);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpPhase1Part1(..) : prvCpssPpConfigDevDbBusBaseAddrAttach ");
    }

    /* set this device to support the "PX" functions */
    devPtr->functionsSupportedBmp = PRV_CPSS_PX_FUNCTIONS_SUPPORT_CNS;
    devPtr->appDevFamily = CPSS_PIPE_E;

    rc = setSip5FlexInfo(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"hwPpPhase1Part1(..) : setSip5FlexInfo(..)");
    }

    /* sets HW interface parameters , only after this point you can use the
        PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC , to know that the device is PCICompatible*/
    rc = prvCpssHwIfSet(CAST_SW_DEVNUM(devNum),initParamsPtr->mngInterfaceType);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"hwPpPhase1Part1(..) : prvCpssHwIfSet(..)");
    }

    /* no meaning for HW devNum ... invalidate it in the DB */
    PRV_CPSS_HW_DEV_NUM_MAC(devNum) = PRV_CPSS_PX_HW_DEV_NUM_NOT_VALID_CNS;

    /*set busy wait parameters.*/
    rc = maxIterationsOfBusyWaitSet(devNum,initParamsPtr->mngInterfaceType);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"hwPpPhase1Part1(..) : maxIterationsOfBusyWaitSet(..)");
    }

    /* check if core clock needed to be read from hardware */
    rc = hwPpAutoDetectCoreClock(devNum,&(devPtr->coreClock));
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"hwPpPhase1Part1(..) : hwPpAutoDetectCoreClock(..)");
    }

    /* initialize all the WA that needed for the device */
    rc = hwPpPhase1NeededWaSettings(devNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"hwPpPhase1Part1(..) : hwPpPhase1NeededWaSettings(..)");
    }


    /* initialize phyPortInfoArray[] */
    rc = initPortTypeArray(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"hwPpPhase1Part1(..) : initPortTypeArray(..)");
    }

    /* initialize portsMibShadowArr[] */
    rc = initPortMibShadow(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"hwPpPhase1Part1(..) : initPortMibShadow(..)");
    }

    rc = setCncParameters(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"hwPpPhase1Part1(..) : setCncParameters(..)");
    }

    /*initialize the DB of 'register address'*/
    rc = prvCpssPxHwRegAddrVer1Init(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"hwPpPhase1Part1(..) : prvCpssPxHwRegAddrVer1Init(..)");
    }

    /* initialize the DB of 'Reset And Init Controller address' (DFX) */
    rc = prvCpssPxHwResetAndInitControllerRegAddrInit(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"hwPpPhase1Part1(..) : prvCpssPxHwResetAndInitControllerRegAddrInit(..)");
    }


    /* bind SMI related functions */
    prvCpssPxPhySmiObjBind(devNum);
    rc = prvCpssSmiInit(devNum, 0);
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("hwPpPhase1Part1(..) : prvCpssSmiInit(..) : return code is %d", rc);
        return rc;
    }

    /* initialize port ifMode configuration method */
    rc = prvCpssPxPortIfCfgInit(devNum,initParamsPtr->allowHwAccessOnly);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"hwPpPhase1Part1(..) : prvCpssPxPortIfCfgInit(..)");
    }

    /* initialize only cpssDriver for HW access, skip the rest */
    if (initParamsPtr->allowHwAccessOnly == GT_TRUE)
    {
        CPSS_LOG_INFORMATION_MAC("Finished to initialize only cpssDriver for HW access, skip the rest");
        return GT_OK;
    }

#ifdef DEBUG_OPENED
    /* check that all enum value also appear in the 'strings' for debug */
    if(errataNames[PRV_CPSS_PX_ERRATA_MAX_NUM_E] == NULL ||
       cpssOsStrCmp(errataNames[PRV_CPSS_PX_ERRATA_MAX_NUM_E] , "--last one--"))
    {
        /* don't fail the initialization , only prompt about the problem */
        DUMP_PRINT_MAC(("hwPpPhase1Part1: errataNames[] not filled with all names properly \n"));
    }
#endif /*DEBUG_OPENED*/

#if 0
    /* bind the system recovery handling functions */
    prvCpssSystemRecoveryMsgDisableModeHandleFuncPtr = prvCpssPxSystemRecoveryDisableModeHandle;
#endif
    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryCatchUpHandleFuncPtr = prvCpssPxSystemRecoveryCatchUpHandle;
    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryCompletionHandleFuncPtr = prvCpssPxSystemRecoveryCompletionHandle;

    for (port = 0; port < devPtr->numOfPorts; port++)
    {
        /* use XSMI#0 controller to access port's PHY*/
        PRV_CPSS_PHY_XSMI_PORT_INTERFACE_MAC(devNum, port) =
                CPSS_PHY_XSMI_INTERFACE_0_E;
    }

    return GT_OK;
}


/**
* @internal hwPpPhase1Part2Config function
* @endinternal
*
* @brief   CPU enable.
*
* @param[in] devNum                   - The PP's device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS hwPpPhase1Part2Config
(
    IN      GT_SW_DEV_NUM   devNum
)
{
    /* CPU enable           */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
                                 PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.globalRegs.
                                 globalControl,1,1,1);
}


/**
* @internal hwPpPhase1Part2Enhanced_common function
* @endinternal
*
* @brief   set fine tuning parameters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] tablesSize               - number of elements in the 'fine tunning' table array(parameter tablePtr)
* @param[in] tablePtr                 - (array of) the table with the 'fine tuning' parameters
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - the device not found in the DB
*/
static GT_STATUS hwPpPhase1Part2Enhanced_common
(
    IN GT_SW_DEV_NUM   devNum,
    IN GT_U32  tablesSize,
    IN PRV_PX_FINE_TUNING_STC *tablePtr
)
{
    PRV_CPSS_PX_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr = &PRV_CPSS_PX_PP_MAC(devNum)->fineTuning;
    GT_U32  ii,jj;/*iterators*/
    GT_BOOL found = GT_FALSE;
    GT_U32  totalCncCountersNeeded;

    /* first set the 'Fine tuning' parameters , from the DB */
    for(ii = 0 ; ii < tablesSize ; ii++)
    {
        for(jj = 0 ; tablePtr[ii].devTypePtr[jj] != LAST_DEV_IN_LIST_CNS ; jj++)
        {
            if(tablePtr[ii].devTypePtr[jj] == PRV_CPSS_PP_MAC(devNum)->devType)
            {
                /* found the device type */

                /* copy it's info */
                fineTuningPtr->tableSize = tablePtr[ii].config.tableSize;

                found = GT_TRUE;
                break;
            }
        }

        if(found == GT_TRUE)
        {
            break;
        }
    }

    if(found == GT_FALSE)
    {
        /* the device type not supported in the DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "devNum[%d] : was not found in the 'fine tuning' table",devNum);
    }

    found = GT_FALSE;

    /* now set the 'Buffer memory' parameters of the 'Fine tuning' parameters , from the DB */

    if(fineTuningPtr->tableSize.transmitDescr == AUTO_CALC_FIELD_CNS)
    {
        fineTuningPtr->tableSize.transmitDescr =
            fineTuningPtr->tableSize.bufferMemory / _2K;/* 2K = 8bits * 256Bytes per buffer */
    }

    if(fineTuningPtr->tableSize.cncBlocks == CNC_AUTO_CALC)
    {
        /* support auto calculation */
        totalCncCountersNeeded = fineTuningPtr->tableSize.cncBlockNumEntries;
    }
    else
    {
        /* support explicit value definitions */
        totalCncCountersNeeded = fineTuningPtr->tableSize.cncBlockNumEntries *
                                 fineTuningPtr->tableSize.cncBlocks;
    }

    if(totalCncCountersNeeded != 0)
    {
        /* update value - to support actual device silicon values */
        fineTuningPtr->tableSize.cncBlockNumEntries = PRV_CPSS_PX_PP_HW_INFO_CNC_MAC(devNum).cncBlockNumEntries;
        /* calculate the number of CNC blocks needed to support the total number of CNC counters */
        fineTuningPtr->tableSize.cncBlocks =
            (totalCncCountersNeeded + (PRV_CPSS_PX_PP_HW_INFO_CNC_MAC(devNum).cncBlockNumEntries-1)) /
            PRV_CPSS_PX_PP_HW_INFO_CNC_MAC(devNum).cncBlockNumEntries;

        /* check that the fine tuning value not overrun the HW max support */
        if(fineTuningPtr->tableSize.cncBlocks >
           (PRV_CPSS_PX_PP_HW_INFO_CNC_MAC(devNum).cncBlocks
            * PRV_CPSS_PX_PP_HW_INFO_CNC_MAC(devNum).cncUnits))
        {
            /* Error in fine tuning or in calculation */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "devNum[%d] : cncBlocks : Error in fine tuning or in calculation");
        }
    }

    return GT_OK;
}



typedef struct
{
    GT_U32 coreClockMHz;
    GT_U32 gopBWGpbs;
}PRV_CPSS_PX_CORECLOCK_BW_STC;


typedef struct
{
    CPSS_PP_FAMILY_TYPE_ENT         family;
    GT_U32                          revision;
    PRV_CPSS_PX_CORECLOCK_BW_STC *coreClockBWListPtr;
}PRV_CPSS_PX_DEV_REV_x_CC_BW_STC;

CPSS_TBD_BOOKMARK_PIPE
static PRV_CPSS_PX_CORECLOCK_BW_STC pipe_A0_coreClockBw [] =
{
  /* ccMHz,  gopBWGpbs */
     {  500 ,  260}
    ,{  450 ,  260}
    ,{  350 ,  200}
    ,{  288 ,  100}
    ,{    0 ,    0}
};

static PRV_CPSS_PX_DEV_REV_x_CC_BW_STC devRev_x_CcBwList[] =
{
      /* dev                            revision,    core clock x BW table */
     { CPSS_PX_FAMILY_PIPE_E        ,            0,  &pipe_A0_coreClockBw[0]              }
    ,{ CPSS_MAX_FAMILY              , (GT_U32)(~0),  (PRV_CPSS_PX_CORECLOCK_BW_STC*)NULL  }
};

/**
* @internal internal_hwPpPhase1Part2Enhanced_BandWidthByDevRevCoreClockGet function
* @endinternal
*
* @brief   This function returns supported bandwidth by dev family/revision/corecolock
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] pipeBWCapacityInGbpsPtr  - pointer to bw corresponding to dev family/revision/corecolock
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on error
* @retval GT_NOT_SUPPORTED         - on device family/revision/coreclock not found
*/
static GT_U32 internal_hwPpPhase1Part2Enhanced_BandWidthByDevRevCoreClockGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32  *pipeBWCapacityInGbpsPtr
)
{
    PRV_CPSS_PX_PP_CONFIG_STC *pDev;
    PRV_CPSS_PX_DEV_REV_x_CC_BW_STC * devRevPtr;
    PRV_CPSS_PX_CORECLOCK_BW_STC    * coreClockBwPtr;
    GT_U32                              min_distance;
    GT_U32                              cur_distance;
    PRV_CPSS_PX_CORECLOCK_BW_STC     *suitableRevCoreClockBWListPtr;
    CPSS_PP_FAMILY_TYPE_ENT             family;
    GT_U32                              revision;
    GT_U32                              coreClockMHz;

    CPSS_NULL_PTR_CHECK_MAC(pipeBWCapacityInGbpsPtr);

    pDev         = PRV_CPSS_PX_PP_MAC(devNum);
    family       = pDev->genInfo.devFamily;
    revision     = pDev->genInfo.revision;
    coreClockMHz = pDev->genInfo.coreClock;
    /*-------------------------------------------------------------*/
    /* search greatest smaller or equal to than requested revision */
    /*-------------------------------------------------------------*/
    suitableRevCoreClockBWListPtr = NULL;
    min_distance                  = (GT_U32)(~0);
    for (devRevPtr = &devRev_x_CcBwList[0]; devRevPtr->family != CPSS_MAX_FAMILY; devRevPtr++)
    {
        if (devRevPtr->family == family && devRevPtr->revision <= revision)
        {
            cur_distance = revision - devRevPtr->revision;
            if (cur_distance < min_distance)
            {
                min_distance = cur_distance;
                suitableRevCoreClockBWListPtr = devRevPtr->coreClockBWListPtr;
            }
        }
    }

    if (suitableRevCoreClockBWListPtr != NULL) /* suitable coreclock list is found */
    {
        for (coreClockBwPtr = suitableRevCoreClockBWListPtr; coreClockBwPtr->coreClockMHz != 0; coreClockBwPtr++)
        {
            if (coreClockBwPtr->coreClockMHz == coreClockMHz)
            {
                *pipeBWCapacityInGbpsPtr = coreClockBwPtr->gopBWGpbs;
                return GT_OK;
            }
        }
    }
    *pipeBWCapacityInGbpsPtr = 0;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "devNum[%d] : coreClockMHz[%d] is not supported by 'Band Width' DB",
        devNum,coreClockMHz); /* not supported */
}

/**
* @internal hwPpPhase1Part2Enhanced function
* @endinternal
*
* @brief   set fine tuning parameters.
*         bandwidth by dev family/revision/corecolock
* @param[in] devNum                   - The PP's device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS hwPpPhase1Part2Enhanced
(
    IN      GT_SW_DEV_NUM  devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_PX_PP_CONFIG_STC *pDev;

    cpssOsMemSet(&PRV_CPSS_PX_PP_MAC(devNum)->fineTuning,0,
                sizeof(PRV_CPSS_PX_PP_MAC(devNum)->fineTuning));

    pDev = PRV_CPSS_PX_PP_MAC(devNum);

    /*set fine tuning parameters*/
    rc = hwPpPhase1Part2Enhanced_common(devNum,pipeTables_size, pipeTables);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*get bandwidth by dev family/revision/corecolock*/
    rc = internal_hwPpPhase1Part2Enhanced_BandWidthByDevRevCoreClockGet(devNum, /*OUT*/&pDev->fineTuning.tableSize.pipeBWCapacityInGbps);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (GT_TRUE == PRV_CPSS_PX_ERRATA_GET_MAC(devNum,
        PRV_CPSS_PX_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
    {
        rc = prvCpssPxLedErrataPortLedInit(devNum);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal hwPpPhase1Part2 function
* @endinternal
*
* @brief   Part 2 of "phase 1"
*         CPU enable
*         set fine tuning parameters.
*         bandwidth by dev family/revision/corecolock
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The Pp's device number
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS hwPpPhase1Part2
(
    IN  GT_SW_DEV_NUM                               devNum
)
{
    GT_STATUS           rc;

    /* CPU enable.     */
    rc = hwPpPhase1Part2Config(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpPhase1Part2Config");
    }

    /* set fine tuning parameters.
        bandwidth by dev family/revision/corecolock */
    rc = hwPpPhase1Part2Enhanced(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpPhase1Part2Enhanced");
    }

    return rc;
}


/**
* @internal hwPpPhase1Part3 function
* @endinternal
*
* @brief   Part 3 of "phase 1"
*         Set mandatory default values for the device.
*         handle init stage 'Errata and RMs'.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The Pp's device number to run the errata Wa on.
* @param[in] initParamsPtr            - PX device init parameters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS hwPpPhase1Part3
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_INIT_INFO_STC                           *initParamsPtr

)
{
    /*Set mandatory default values for the device*/
    /*handle init stage 'Errata and RMs'*/
    return hwPpPxRegsDefault(devNum, initParamsPtr, GT_FALSE);
}

/**
* @internal prvCpssPxPipePortMappingConfigSet function
* @endinternal
*
* @brief   Pipe Port mapping initial configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The Pp's device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssPxPipePortMappingConfigSet
(
    IN  GT_SW_DEV_NUM  devNum
)
{
    GT_STATUS rc;       /* return code */
    GT_U32 regAddr;     /* register address */
    GT_U32 globalDmaNum; /* local DMA number 0..16 */
    GT_U32 globalTxqPort;/* global txq port 0..19 */
    GT_U32  ii, regData;
    CPSS_PX_DETAILED_PORT_MAP_STC  *detailedShadowPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC  tempSystemRecovery_Info;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* INIT mapping */
    detailedShadowPtr = &PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[0];
    for(ii = 0 ; ii < PRV_CPSS_PX_PORTS_NUM_CNS; ii++,detailedShadowPtr++)
    {
        detailedShadowPtr->valid = GT_FALSE;
        detailedShadowPtr->portMap.mappingType = GT_PX_NA;
        detailedShadowPtr->portMap.macNum      = GT_PX_NA;
        detailedShadowPtr->portMap.dmaNum      = GT_PX_NA;
        detailedShadowPtr->portMap.txqNum      = GT_PX_NA;
    }

    for(ii = 0 ; ii < PRV_CPSS_PX_GOP_PORTS_NUM_CNS; ii++)
    {
        PRV_CPSS_PX_PP_MAC(devNum)->port.portsMac2PhyscalConvArr[ii] = GT_PX_NA;
    }

    /*skip in case of HA */
    if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
        (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) )
    {
        return GT_OK;
    }

    /***************************************/
    /* PIPW -- Port Mapping Configurations */
    /***************************************/

    /* Enable TXQ to DMA port mapping */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.globalDequeueConfig;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 10, 1, 1);
    if(GT_OK != rc)
    {
        return rc;
    }

    /*
        MAP the DQ ports to TXDMA ports :
        DQ[0] ports 0..9 mapped to TXDMA ports 0..9
        DQ[1] ports 0..6 mapped to TXDMA ports 10..16
    */

    for(globalTxqPort = 0 ; globalTxqPort < PRV_CPSS_PX_MAX_DQ_PORTS_CNS ; globalTxqPort++)
    {
        if(globalTxqPort == PRV_CPSS_PX_DMA_PORTS_NUM_CNS)
        {
            /* not need more than than the 17 first ports  */
            break;
        }

        globalDmaNum = globalTxqPort;

        /* bind the DQ port to the DMA number */
        rc = TxQPort2TxDMAMapSet(devNum,
            globalTxqPort,
            globalDmaNum);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /*Mapping PFC Source Port to TxQ Port*/
    /* set port range for unit0 0.. 9*/
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,0).flowCtrlResponse.flowCtrlResponseConfig;
    /*last port in range - PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp , first - 0 */
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 13, 10, (PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp - 1));
    if(GT_OK != rc)
    {
        return rc;
    }

    /* set port range for unit1 */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,1).flowCtrlResponse.flowCtrlResponseConfig;
    if(GT_OK != rc)
    {
        return rc;
    }
    /* set last port  0xf and first - (PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp +1) */
    regData = ((0xF <<10) | PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp);
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 3, 20, regData);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* According to design DQ0 and DQ1 should have identical data:
       for ports that are hanled by DQ0:
            queue number  = port number
       for ports that are handled by DQ1:
            queue number  = (port number - txqDqNumPortsPerDp)
             0  - 0
             1  - 1
             2  - 2
             3  - 3
             4  - 4
             5  - 5
             6  - 6
             7  - 7
             8  - 8
             9  - 9
             10 - 0
             11 - 1
             12 - 2
             13 - 3
             14 - 4
             15 - 5
            */
    for(globalTxqPort = 0 ; globalTxqPort < PRV_CPSS_PX_GOP_PORTS_NUM_CNS ; globalTxqPort++)
    {
        if (globalTxqPort < PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp)
        {
            rc = prvCpssPxPFCResponsePhysPort2TxQMapSet(devNum, globalTxqPort, globalTxqPort);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
        else
        {
             rc = prvCpssPxPFCResponsePhysPort2TxQMapSet(devNum, globalTxqPort, (globalTxqPort - PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp));
             if(GT_OK != rc)
             {
                 return rc;
             }

        }
    }

    return GT_OK;
}

/**
* @internal hwPpPhase1RegisterMisconfigurationAfterTablesInit function
* @endinternal
*
* @brief   RM (Register Miss-configuration) after calling to
*         prvCpssPxTablesAccessInit(...)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The Pp's device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS hwPpPhase1RegisterMisconfigurationAfterTablesInit
(
    IN  GT_SW_DEV_NUM               devNum
)
{
    GT_STATUS   rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    /*******************************/
    /* Port Mapping Configurations */
    /*******************************/
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
    {
        rc = prvCpssPxPipePortMappingConfigSet(devNum);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal hwPpSip5StartInitSet function
* @endinternal
*
* @brief   After DEV_INIT_DONE signal is asserted, software can read fully
*         functional state from the chip.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The Pp's device number.
* @param[in] ignoreInitDone           - Whether to return OK though initDone bits are still 'init_onGoing'.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - the device
*/
static GT_STATUS hwPpSip5StartInitSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_BOOL             ignoreInitDone
)
{
    GT_U32              regAddr;        /* register address */
    GT_U32              tmpData;
    GT_U32              tmpData1;
    GT_U32              mask;
    GT_U32              timeout;        /* Timeout for waiting for the      */
                                        /* startinit operation to be done.  */
    GT_STATUS           rc;             /* Return code */
    GT_U32              timeToSleepMilisec;

#ifdef _FreeBSD
    /* under FreeBSD required longer timeout untill
     * ASIC reset finished
     */
    timeToSleepMilisec = 1000;
#else
    timeToSleepMilisec = 10;
#endif
    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    if(PRV_CPSS_PP_MAC(devNum)->ppHAState == CPSS_SYS_HA_MODE_STANDBY_E)
    {
        return GT_OK;
    }


    {
        timeout = 100;
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                          DFXServerUnitsDeviceSpecificRegs.initializationStatusDone;

        do
        {

            /* Check init stage status */
            rc = prvCpssDrvHwPpResetAndInitControllerReadReg(CAST_SW_DEVNUM(devNum), regAddr, &tmpData);
            if( GT_OK != rc )
            {
                return rc;
            }


            if(timeout == 0)
            {
                /* timeout, no initDone indication its System with TM or some other issue */
                if (ignoreInitDone)
                {
                    CPSS_LOG_INFORMATION_MAC("hwPpSip5StartInitSet-initDone: ignore Uncompleted HW init value: %0x\n", tmpData);
                    tmpData = 0;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, "devNum[%d] : did not reach <initDone> ",
                        devNum);
                }
            }

            rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(CAST_SW_DEVNUM(devNum), 0xF8F20, 0, 1, &tmpData1);
            if( GT_OK != rc )
            {
                return rc;
            }

            /************************************************************************
                In the system there might be IDEfuse burned devices with
                security bit enabled or disabled.

                When the security bit is disabled, in case of burning some
                effuse bits, we might run into an issue of wrong the
                <dfx_server_Init_Done>[1]  status indication (MSB bit of field {2:1})

                Thus, we need to check on the <dfx_server_Init_Done>[1] only when
                ID Efuse[256] - (bit [0] of 0x000F8F20) is burned to 0x1.
                If it's not burned (0x1) SW needs to check only on <dfx_server_Init_Done>[0] .

            ************************************************************************/
            if(tmpData1 == 1)
            {
                mask = 0xFFFFFDDE;
            }
            else
            {
                mask = 0xFFFFFDDA;
            }

            #ifdef ASIC_SIMULATION
                /* single iteration is enough */
                tmpData = 0;
            #endif /*ASIC_SIMULATION*/

            if((tmpData & mask) == 0)
            {/* if all needed units passed init, clear "all init done" */
                rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(CAST_SW_DEVNUM(devNum), regAddr,
                                                                     0, 1, 0);
                if( GT_OK != rc )
                {
                    return rc;
                }
                break;
            }
            cpssOsTimerWkAfter(timeToSleepMilisec);
            timeout--;
        }
        while (1);
    }

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                      DFXServerUnits.DFXServerRegs.serverStatus;

    timeout = 100;

    do
    {
        if(timeout == 0)
        {
            /* time over, the device is not finished init. This means HW problem. */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, "devNum[%d] : <init stage> was not finished",
                devNum);
        }

        /* Check init stage status */
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
                                               CAST_SW_DEVNUM(devNum), regAddr, 4, 2, &tmpData);
        if( GT_OK != rc )
        {
            return rc;
        }

        cpssOsTimerWkAfter(timeToSleepMilisec);
        timeout--;
    }
    while (tmpData != 0x3);

    return GT_OK;
}


/**
* @internal hwPpPhase2 function
* @endinternal
*
* @brief   update the cpssDriver
*         Assume that we are under "locked interrupts"
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*
* @note function called when PP's interrupts are disabled !
*
*/
static GT_STATUS hwPpPhase2
(
    IN  GT_SW_DEV_NUM               devNum
)
{
    GT_STATUS rc ;

    rc = prvCpssDrvInterruptsAfterPhase2(CAST_SW_DEVNUM(devNum),CAST_SW_DEVNUM(devNum));
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpPhase2 : prvCpssDrvInterruptsAfterPhase2");
    }

    return GT_OK;
}

/**
* @internal portPizzaArbiterIfInit function
* @endinternal
*
* @brief   init Pizza arbiter
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS portPizzaArbiterIfInit
(
    IN  GT_SW_DEV_NUM               devNum
)
{
    GT_STATUS rc;

    rc = prvCpssPxPortPizzaArbiterIfInit(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxPortPizzaArbiterIfInit");
    }

    return GT_OK;
}

/**
* @internal prv_internal_cpssPxHwInit function
* @endinternal
*
* @brief   This function performs basic hardware configurations on the given PX, in
*         Hw registration phase.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] initParamsPtr            - PX device init parameters.
*
* @param[out] deviceTypePtr            - The Pp's device Id.
* @param[out] clearDbOnErrorPtr        - indication that caller need to clear allocated DB in case of error.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_ALREADY_EXIST         - the devNum already in use
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_BAD_VALUE             - the driver found unknown device type
* @retval GT_NOT_IMPLEMENTED       - the CPSS was not compiled properly
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prv_internal_cpssPxHwInit
(
    IN      GT_SW_DEV_NUM                       devNum,
    IN      CPSS_PX_INIT_INFO_STC               *initParamsPtr,
    OUT     CPSS_PP_DEVICE_TYPE                 *deviceTypePtr,
    OUT     GT_BOOL                             *clearDbOnErrorPtr
)
{
    GT_STATUS rc ;
    PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_ENT prePhase1InitMode;
    GT_BOOL   ignoreInitDone = GT_FALSE;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    CPSS_NULL_PTR_CHECK_MAC(initParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(deviceTypePtr);

    *clearDbOnErrorPtr = GT_FALSE;

    rc = prvCpssPrePhase1PpInitModeGet(&prePhase1InitMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPrePhase1PpInitModeGet");
    }

    /* check input parameters */
    CPSS_PARAM_CHECK_MAX_MAC(devNum,PRV_CPSS_MAX_PP_DEVICES_CNS);

    if(PRV_CPSS_PP_MAC(devNum))
    {
        /* the device occupied by some device ... */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST,
            "A device with SW devNum[%d] already exists",devNum);
    }

    *clearDbOnErrorPtr = GT_TRUE;

    /* initialize the cpssDriver , build the DB needed for the device */
    rc = hwPpPhase1Part1(devNum,initParamsPtr,deviceTypePtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpPhase1Part1");
    }

    /* initialize only cpssDriver for HW access, skip the rest */
    if (initParamsPtr->allowHwAccessOnly == GT_TRUE)
    {
        CPSS_LOG_INFORMATION_MAC("Finished to initialize only cpssDriver for HW access, skip the rest");
        return GT_OK;
    }

    /*  ONLY After DEV_INIT_DONE signal is asserted,
        software can read fully functional state from the chip.
    */
    rc = hwPpSip5StartInitSet(devNum, ignoreInitDone);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpSip5StartInitSet");
    }

    /*
    *       Part 2 of "phase 1"
    *       CPU enable
    *       set fine tuning parameters.
    *       bandwidth by dev family/revision/corecolock
    */
    rc = hwPpPhase1Part2(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpPhase1Part2");
    }

    /* Initialize the interrupts mechanism for a given device. */
    rc = prvCpssDrvInterruptsInit(CAST_SW_DEVNUM(devNum),
            CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,
            initParamsPtr->hwInfo.irq.switching,
            initParamsPtr->hwInfo.irq.switching);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvInterruptsInit");
    }

    /* bind the cpssDriver with the 'Interrupt generation' function */
    PRV_INTERRUPT_FUNC_GET(devNum,unitIntGenerationCheckFunc) = prvCpssPxUnitInterruptGenerationCheck;
    /* bind the cpssDriver with the port mapping */
    PRV_INTERRUPT_FUNC_GET(devNum,eventPortMapConvertFunc) = prvCpssPxPortEventPortMapConvert;
    PRV_INTERRUPT_FUNC_GET(devNum,macEventPortMapConvertFunc) = prvCpssPortMacEventConvert;

    /* init Cheetah tables database .
       called after : prvCpssPxPpConfigEnhancedInitFuncPtr
       but should be called even if prvCpssPxPpConfigEnhancedInitFuncPtr
       is NULL
    */
    rc = prvCpssPxTablesAccessInit(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxTablesAccessInit");
    }
    rc = prvCpssPxHwPpPhase1ShadowInit(devNum,initParamsPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxHwPpPhase1ShadowInit");
    }

    if (prePhase1InitMode == PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_NO_PP_INIT_E)
    {
        /* no more to do */
        return GT_OK;
    }

    /*
    *       Part 3 of "phase 1"
    *       Set mandatory default values for the device.
    *       handle init stage 'Errata and RMs'.
    */

    if ( system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E )
    {
        rc = hwPpPhase1Part3(devNum, initParamsPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpPhase1Part3");
        }
    }
#ifdef ASIC_SIMULATION
    else
    {
        rc = prvCpssPxEgressInit(devNum, initParamsPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxEgressInit_fastBoot");
        }
    }
#endif
    /* set RM (Register Miss-configuration) after tables initialization */
    rc = hwPpPhase1RegisterMisconfigurationAfterTablesInit(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpPhase1RegisterMisconfigurationAfterTablesInit");
    }

    /* Lock the interrupt scan, this phase changes the interrupts nodes   */
    /* pool data. And relocates the port group / driverPpDevs elements.   */
    PRV_CPSS_INT_SCAN_LOCK();
    /*
    *       update the cpssDriver
    *       initialize the AU descriptors
    *       initialize the network interface
    */
    rc = hwPpPhase2(devNum);
    /* UnLock the interrupt scan */
    PRV_CPSS_INT_SCAN_UNLOCK();
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpPhase2");
    }


    rc = portPizzaArbiterIfInit(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "portPizzaArbiterIfInit");
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxHwInit function
* @endinternal
*
* @brief   This function performs basic hardware configurations on the given PX, in
*         Hw registration phase.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] initParamsPtr            - PX device init parameters.
*
* @param[out] deviceTypePtr            - The Pp's device Id.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_ALREADY_EXIST         - the devNum already in use
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_BAD_VALUE             - the driver found unknown device type
* @retval GT_NOT_IMPLEMENTED       - the CPSS was not compiled properly
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxHwInit
(
    IN      GT_SW_DEV_NUM                       devNum,
    IN      CPSS_PX_INIT_INFO_STC               *initParamsPtr,
    OUT     CPSS_PP_DEVICE_TYPE                 *deviceTypePtr
)
{
    GT_STATUS rc;
    GT_BOOL   clearDbOnError = GT_FALSE;

    rc = prv_internal_cpssPxHwInit(devNum,initParamsPtr,deviceTypePtr,&clearDbOnError);

    if(rc != GT_OK)
    {
        prvCpssGenericSrvCpuRemove(CAST_SW_DEVNUM(devNum), SRVCPU_IPC_CHANNEL_ALL);
        CPSS_LOG_INFORMATION_MAC("error during operation , so clear dynamic allocated memory of device [%d] DB", devNum);
        prvCpssDrvHwPpDevRemove(devNum);

        if((clearDbOnError == GT_TRUE) && (PRV_CPSS_PX_PP_MAC(devNum)))
        {
            /* restore all the DB state to start point */
            (void)prvCpssPxHwRegAddrVer1Remove(devNum);

            /* do not modify 'rc' that hold ERROR at this point */

            /* free the memory of this device */
            cpssOsFree(PRV_CPSS_PP_MAC(devNum));

            /* set this index as "not occupied" */
            PRV_CPSS_PP_CONFIG_ARR_MAC[devNum] = NULL;
            px_prvCpssPpConfig[devNum] = PRV_CPSS_PP_CONFIG_ARR_MAC[devNum];
        }

        /* Cleanup all resources allocated for HW Serdes functions */
        mvHwsDeviceClose(devNum);
        /*prvCpssGenericSrvCpuRemove(devNum, SRVCPU_IPC_CHANNEL_ALL);*/
    }

    return rc;
}

/**
* @internal cpssPxHwInit function
* @endinternal
*
* @brief   This function performs basic hardware configurations on the given PX, in
*         Hw registration phase.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to assign in CPSS DB for the initialized PX device.
* @param[in] initParamsPtr            - PX device init parameters.
*
* @param[out] deviceTypePtr            - The Pp's device Id.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_ALREADY_EXIST         - the devNum already in use
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_BAD_VALUE             - the driver found unknown device type
* @retval GT_NOT_IMPLEMENTED       - the CPSS was not compiled properly
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHwInit
(
    IN      GT_SW_DEV_NUM                       devNum,
    IN      CPSS_PX_INIT_INFO_STC            *initParamsPtr,
    OUT     CPSS_PP_DEVICE_TYPE                 *deviceTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxHwInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, initParamsPtr, deviceTypePtr));

    rc = internal_cpssPxHwInit(devNum, initParamsPtr, deviceTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, initParamsPtr, deviceTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxHwDescSizeGet function
* @endinternal
*
* @brief   This function returns the size in bytes of a several descriptor sizes,
*         for a given device.
*         the size needed for rx descriptors for packet to the CPU.
*         the alignment needed for rx buffer for packet to the CPU.
*         the size needed for tx descriptors for packets from the CPU.
*         the size needed for CNC descriptors (pair of counters each) for CNC fast dump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devType                  - The PP's device type to return the descriptor's size for.
*
* @param[out] rxDescSizePtr            - (pointer to) The Rx  descrptor's size (in bytes).
*                                      ignored if NULL
* @param[out] rxBufAlignmentPtr        - (pointer to) The Rx buffer alignment size (in bytes).
*                                      ignored if NULL
* @param[out] txDescSizePtr            - (pointer to) The Tx  descrptor's size (in bytes).
*                                      ignored if NULL
* @param[out] cncDescSizePtr           - (pointer to) The Cnc descrptor's size (in bytes).
*                                      ignored if NULL
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxHwDescSizeGet
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *rxDescSizePtr,
    OUT GT_U32                  *rxBufAlignmentPtr,
    OUT GT_U32                  *txDescSizePtr,
    OUT GT_U32                  *cncDescSizePtr
)
{
    GT_U32 txShortBufSize;

    /* the devType is not used yet */
    (void)devType;

    if(rxDescSizePtr)
    {
        *rxDescSizePtr = RX_DESC_SIZE;
    }

    if(txDescSizePtr)
    {
        txShortBufSize = TX_SHORT_BUFF_SIZE_16_CNS;

        *txDescSizePtr = TX_DESC_SIZE + txShortBufSize + (TX_HEADER_SIZE / 2);
    }

    if(rxBufAlignmentPtr)
    {
        *rxBufAlignmentPtr = 128;
    }

    if(cncDescSizePtr)
    {
        *cncDescSizePtr = 16;/* 4 words : 2 per counter */
    }

    return GT_OK;
}

/**
* @internal cpssPxHwDescSizeGet function
* @endinternal
*
* @brief   This function returns the size in bytes of a several descriptor sizes,
*         for a given device.
*         the size needed for rx descriptors for packet to the CPU.
*         the alignment needed for rx buffer for packet to the CPU.
*         the size needed for tx descriptors for packets from the CPU.
*         the size needed for CNC descriptors (pair of counters each) for CNC fast dump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devType                  - The PP's device type to return the descriptor's size for.
*
* @param[out] rxDescSizePtr            - (pointer to) The Rx  descrptor's size (in bytes).
*                                      ignored if NULL
* @param[out] rxBufAlignmentPtr        - (pointer to) The Rx buffer alignment size (in bytes).
*                                      ignored if NULL
* @param[out] txDescSizePtr            - (pointer to) The Tx  descrptor's size (in bytes).
*                                      ignored if NULL
* @param[out] cncDescSizePtr           - (pointer to) The Cnc descrptor's size (in bytes).
*                                      ignored if NULL
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHwDescSizeGet
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *rxDescSizePtr,
    OUT GT_U32                  *rxBufAlignmentPtr,
    OUT GT_U32                  *txDescSizePtr,
    OUT GT_U32                  *cncDescSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxHwDescSizeGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devType, rxDescSizePtr, rxBufAlignmentPtr, txDescSizePtr, cncDescSizePtr));

    rc = internal_cpssPxHwDescSizeGet(devType, rxDescSizePtr, rxBufAlignmentPtr, txDescSizePtr, cncDescSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devType, rxDescSizePtr, rxBufAlignmentPtr, txDescSizePtr, cncDescSizePtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxHwPpSoftResetTrigger function
* @endinternal
*
* @brief   This routine issue soft reset for a specific pp.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to reset.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Soft reset is the process in which selected entities (as configured by
*       using the cpssPxHwPpSoftResetSkipParamSet API) are reset to their
*       default values.
*
*/
static GT_STATUS internal_cpssPxHwPpSoftResetTrigger
(
    IN  GT_SW_DEV_NUM devNum
)
{
    GT_STATUS rc;             /* return code */
    GT_U32    dfxDeviceResetControlRegAddr;   /* register address                */
    GT_BOOL   regsSkipInit;
    GT_BOOL   tablesSkipInit;
    GT_BOOL   pexSkipInit;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    rc = cpssPxHwPpSoftResetSkipParamGet(devNum, CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E, &regsSkipInit);
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = cpssPxHwPpSoftResetSkipParamGet(devNum, CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E, &tablesSkipInit);
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = cpssPxHwPpSoftResetSkipParamGet(devNum, CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E, &pexSkipInit);
    if(GT_OK != rc)
    {
        return rc;
    }

    dfxDeviceResetControlRegAddr =
        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                          DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl;

    /* state to the special DB that the device did HW reset */
    prvCpssPpConfigDevDbHwResetSet(CAST_SW_DEVNUM(devNum),GT_TRUE);

    /*  NOTE about <Table Start Init> : if at some point in the future the CPSS
        will not force '0' , then be aware of implications of
        JIRA:  BOBCAT2-506 chip requires "manual" activation of table_start_init_ & tm_start_init_ after soft reset
    */


    /* set bits of : <MG Soft Reset Trigger> and <Table Start Init> */
    /* the <Table Start Init> = 0 is relevant only when <skip table> == false
        and means that 'tables' will be re-initialized */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
                  CAST_SW_DEVNUM(devNum), dfxDeviceResetControlRegAddr, 1, 2, 0);

    return rc;
}
/**
* @internal cpssPxHwPpSoftResetTrigger function
* @endinternal
*
* @brief   This routine issue soft reset for a specific pp.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to reset.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Soft reset is the process in which selected entities (as configured by
*       using the cpssPxHwPpSoftResetSkipParamSet API) are reset to their
*       default values.
*
*/
GT_STATUS cpssPxHwPpSoftResetTrigger
(
    IN  GT_SW_DEV_NUM devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxHwPpSoftResetTrigger);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssPxHwPpSoftResetTrigger(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal softResetSkipParamSetSip5 function
* @endinternal
*
* @brief   This routine configure skip parameters related to soft reset.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] skipType                 - the skip parameter to set
*                                      see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
* @param[in] skipEnable               - GT_FALSE: Do Not Skip
*                                      GT_TRUE:  Skip
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS softResetSkipParamSetSip5
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    IN  GT_BOOL                         skipEnable
)
{
    GT_U32    resetRegAddr;     /* register address */
    GT_U32    addConfigRegAddr; /* register address for additional configuration */
    GT_U32    bitOffset;        /* bit offset or value in register */
    GT_U32    hwValue;          /* HW value            */
    GT_STATUS rc;               /* return code */

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    bitOffset = 8;

    /*
        0x0 = Skip init  ON; SKIP_INIT_ON
        0x1 = Skip init OFF; SKIP_INIT_OFF
    */
    hwValue = 1 - BOOL2BIT_MAC(skipEnable);


    switch(skipType)
    {
        case CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E:
            resetRegAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
               DFXServerUnitsDeviceSpecificRegs.configSkipInitializationMatrix;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E:
            resetRegAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
               DFXServerUnitsDeviceSpecificRegs.tableSkipInitializationMatrix;

            addConfigRegAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
               DFXServerUnitsDeviceSpecificRegs.RAMInitSkipInitializationMatrix;

            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
                CAST_SW_DEVNUM(devNum), addConfigRegAddr, bitOffset, 1, hwValue);
            if(GT_OK != rc)
            {
                return rc;
            }

            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E:
            resetRegAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
               DFXServerUnitsDeviceSpecificRegs.EEPROMSkipInitializationMatrix;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E:
            resetRegAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
               DFXServerUnitsDeviceSpecificRegs.SERDESSkipInitializationMatrix;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E:
            /*
                Behavior is un-predictable (probably the device will hang)
                if CPU try to read/write registers/tables
                of the device during the time of soft reset
                (Soft reset is active for 2000 core clock cycles (6uS). Waiting 20uS should be enough)
                Event when '<PEX Skip Init if MG Soft Reset> = SKIP INIT ON'
                (no pex reset).

                *******************************
                meaning that even when skip pex reset there is still interval of
                time that the CPU must not approach the device.
            */
            resetRegAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
               DFXServerUnitsDeviceSpecificRegs.PCIeSkipInitializationMatrix;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(skipType);
    }


    return prvCpssDrvHwPpResetAndInitControllerSetRegField(
        CAST_SW_DEVNUM(devNum), resetRegAddr, bitOffset, 1, hwValue);

}

/**
* @internal softResetSkipParamSetAll function
* @endinternal
*
* @brief   This routine configure skip parameters related to soft reset. - ALL /ALL_EXCLUDE_PEX
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] skipType                 - the skip parameter to set , one of:
*                                      CPSS_HW_PP_RESET_SKIP_TYPE_ALL_E,
*                                      CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E
* @param[in] skipEnable               - GT_FALSE: Do Not Skip
*                                      GT_TRUE:  Skip
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS softResetSkipParamSetAll
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    IN  GT_BOOL                         skipEnable
)
{
    GT_STATUS   rc;         /* return code */
    CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipIterator; /* loop iterator */
    GT_U32    num_GT_OK = 0;/* number of GT_OK that we got */

    for(skipIterator = CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E;
        skipIterator < CPSS_HW_PP_RESET_SKIP_TYPE_ALL_E;
        skipIterator++)
    {
        if(skipType == CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E &&
            skipIterator == CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E)
        {
            /* the pex always will set 'skip' */
            rc = cpssPxHwPpSoftResetSkipParamSet(devNum, skipIterator, GT_TRUE);
        }
        else
        {
            rc = cpssPxHwPpSoftResetSkipParamSet(devNum, skipIterator, skipEnable);
        }
        if(rc != GT_OK)
        {
            if(rc == GT_BAD_PARAM || rc == GT_NOT_SUPPORTED)
            {
                /* assume that this specific case is not supported */
            }
            else
            {
                return rc;
            }
        }
        else
        {
            num_GT_OK++;
        }
    }

    if(num_GT_OK == 0)
    {
        /* no flag is supported so declare 'Not supported' */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "devNum[%d] : skipType[%d] is NOT supported",
            devNum,skipType);
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxHwPpSoftResetSkipParamSet function
* @endinternal
*
* @brief   This routine configure skip parameters related to soft reset.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] skipType                 - the skip parameter to set
*                                      see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
* @param[in] skipEnable               - GT_FALSE: Do Not Skip
*                                      GT_TRUE:  Skip
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxHwPpSoftResetSkipParamSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    IN  GT_BOOL                         skipEnable
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if(skipType == CPSS_HW_PP_RESET_SKIP_TYPE_ALL_E ||
       skipType == CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E)
    {
        /* set 'ALL' or 'ALL_EXCLUDE_PEX' */
        return softResetSkipParamSetAll(devNum,skipType,skipEnable);
    }

    return softResetSkipParamSetSip5(devNum,skipType,skipEnable);
}

/**
* @internal cpssPxHwPpSoftResetSkipParamSet function
* @endinternal
*
* @brief   This routine configure skip parameters related to soft reset.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] skipType                 - the skip parameter to set
*                                      see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
* @param[in] skipEnable               - GT_FALSE: Do Not Skip
*                                      GT_TRUE:  Skip
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHwPpSoftResetSkipParamSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    IN  GT_BOOL                         skipEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxHwPpSoftResetSkipParamSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, skipType, skipEnable));

    rc = internal_cpssPxHwPpSoftResetSkipParamSet(devNum, skipType, skipEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, skipType, skipEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxHwPpSoftResetSkipParamGet function
* @endinternal
*
* @brief   This routine return configuration of skip parameters related to soft reset.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] skipType                 - the skip parameter to set
*                                      see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
*
* @param[out] skipEnablePtr            - GT_FALSE: Do Not Skip
*                                      GT_TRUE:  Skip
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS internal_cpssPxHwPpSoftResetSkipParamGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    OUT GT_BOOL                         *skipEnablePtr

)
{
    GT_STATUS rc;        /* return code */
    GT_U32    bitOffset; /* bit offset in register */
    GT_U32    value;     /* register data */
    GT_U32    resetRegAddr;   /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(skipEnablePtr);

    switch(skipType)
    {
        case CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E:
            resetRegAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
               DFXServerUnitsDeviceSpecificRegs.configSkipInitializationMatrix;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E:
            resetRegAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
               DFXServerUnitsDeviceSpecificRegs.tableSkipInitializationMatrix;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E:
            resetRegAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
               DFXServerUnitsDeviceSpecificRegs.EEPROMSkipInitializationMatrix;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E:
            resetRegAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
               DFXServerUnitsDeviceSpecificRegs.SERDESSkipInitializationMatrix;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E:
            resetRegAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
               DFXServerUnitsDeviceSpecificRegs.PCIeSkipInitializationMatrix;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(skipType);
    }

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        *skipEnablePtr = GT_TRUE;
        return GT_OK;
    }

    bitOffset = 8;

    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
                                CAST_SW_DEVNUM(devNum), resetRegAddr, bitOffset, 1, &value);

    if (rc != GT_OK)
    {
        return rc;
    }

    /* invert the retrieved bit */
    value = ((~ value) & 1);
    *skipEnablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssPxHwPpSoftResetSkipParamGet function
* @endinternal
*
* @brief   This routine return configuration of skip parameters related to soft reset.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] skipType                 - the skip parameter to set
*                                      see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
*
* @param[out] skipEnablePtr            - GT_FALSE: Do Not Skip
*                                      GT_TRUE:  Skip
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssPxHwPpSoftResetSkipParamGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    OUT GT_BOOL                         *skipEnablePtr

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxHwPpSoftResetSkipParamGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, skipType, skipEnablePtr));

    rc = internal_cpssPxHwPpSoftResetSkipParamGet(devNum, skipType, skipEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, skipType, skipEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxHwPpInitStageGet function
* @endinternal
*
* @brief   Indicates the initialization stage of the device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to reset.
*
* @param[out] initStagePtr             - pointer to the stage of the device
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS internal_cpssPxHwPpInitStageGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    OUT CPSS_HW_PP_INIT_STAGE_ENT  *initStagePtr
)
{
    GT_STATUS rc;
    GT_U32    value;        /* value of field */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(initStagePtr);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* we are not allowed to access the DFX registers */
        *initStagePtr =  CPSS_HW_PP_INIT_STAGE_FULLY_FUNC_E;

        return GT_OK;
    }

    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(CAST_SW_DEVNUM(devNum),
                    PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnits.DFXServerRegs.serverStatus,
                    4, 2, &value);
    if(rc != GT_OK)
        return rc;

    switch(value)
    {
        default:/* only 2 bits ... so no real 'default' */
        case 0:
            *initStagePtr =  CPSS_HW_PP_INIT_STAGE_INIT_DURING_RESET_E;
            break;
        case 1:
            *initStagePtr =  CPSS_HW_PP_INIT_STAGE_EEPROM_DONE_INT_MEM_DONE_E;
            break;
        case 2:
            *initStagePtr =  CPSS_HW_PP_INIT_STAGE_EEPROM_NOT_DONE_INT_MEM_DONE_E;
            break;
        case 3:
            *initStagePtr =  CPSS_HW_PP_INIT_STAGE_FULLY_FUNC_E;
            break;
    }

    return rc;

}

/**
* @internal cpssPxHwPpInitStageGet function
* @endinternal
*
* @brief   Indicates the initialization stage of the device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to reset.
*
* @param[out] initStagePtr             - pointer to the stage of the device
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssPxHwPpInitStageGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    OUT CPSS_HW_PP_INIT_STAGE_ENT  *initStagePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxHwPpInitStageGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, initStagePtr));

    rc = internal_cpssPxHwPpInitStageGet(devNum, initStagePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, initStagePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssPxHwInterruptCoalescingSet function
* @endinternal
*
* @brief   Configures the interrupt coalescing parameters and enable\disable the
*         functionality.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: interrupt coalescing is enabled
*                                      GT_FALSE: interrupt coalescing is disabled
* @param[in] period                   - Minimal IDLE  between two consecutive interrupts.
*                                      The units of this input parameter is in nSec, the
*                                      resolution is in 320 nSec (the HW resolution for the
*                                      interrupt coalescing period is 0.32uSec). In case interrupt
*                                      coalescing is enabled (enable == GT_TRUE)
*                                      (APPLICABLE RANGES: 320..5242560).
*                                      Otherwise (enable == GT_FALSE), ignored.
* @param[in] linkChangeOverride       - GT_TRUE: A link change in one of the ports
*                                      results interrupt regardless of the (coalescing)
* @param[in] period
*                                      GT_FALSE: A link change in one of the ports
*                                      does not results interrupt immediately but
*                                      according to the (coalescing) period.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxHwInterruptCoalescingSet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN GT_BOOL  enable,
    IN GT_U32   period,
    IN GT_BOOL  linkChangeOverride
)
{
    GT_U32 regAddr;   /* register's address */
    GT_U32 regData;   /* register's data */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if( enable == GT_TRUE )
    {
        if( period < MIN_INTERRUPT_COALESCING_PERIOD_CNS ||
            period > MAX_INTERRUPT_COALESCING_PERIOD_CNS )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, " [period] must be in [%d..%d] but got[%d]",
                MIN_INTERRUPT_COALESCING_PERIOD_CNS,
                MAX_INTERRUPT_COALESCING_PERIOD_CNS,
                period);
        }
        regData = period/MIN_INTERRUPT_COALESCING_PERIOD_CNS;
    }
    else /* enable == GT_FALSE */
    {
        regData = 0;
    }

    U32_SET_FIELD_MAC(regData, 16, 1, ((linkChangeOverride == GT_TRUE) ? 0 : 1));

    regAddr = PRV_PX_REG1_UNIT_MG_MAC(devNum).globalRegs.interruptCoalescing;

    return prvCpssHwPpWriteRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, 0x13FFF, regData);
}

/**
* @internal cpssPxHwInterruptCoalescingSet function
* @endinternal
*
* @brief   Configures the interrupt coalescing parameters and enable\disable the
*         functionality.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: interrupt coalescing is enabled
*                                      GT_FALSE: interrupt coalescing is disabled
* @param[in] period                   - Minimal IDLE  between two consecutive interrupts.
*                                      The units of this input parameter is in nSec, the
*                                      resolution is in 320 nSec (the HW resolution for the
*                                      interrupt coalescing period is 0.32uSec). In case interrupt
*                                      coalescing is enabled (enable == GT_TRUE)
*                                      (APPLICABLE RANGES: 320..5242560).
*                                      Otherwise (enable == GT_FALSE), ignored.
* @param[in] linkChangeOverride       - GT_TRUE: A link change in one of the ports
*                                      results interrupt regardless of the (coalescing)
* @param[in] period
*                                      GT_FALSE: A link change in one of the ports
*                                      does not results interrupt immediately but
*                                      according to the (coalescing) period.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHwInterruptCoalescingSet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN GT_BOOL  enable,
    IN GT_U32   period,
    IN GT_BOOL  linkChangeOverride
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxHwInterruptCoalescingSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable, period, linkChangeOverride));

    rc = internal_cpssPxHwInterruptCoalescingSet(devNum, enable, period, linkChangeOverride);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable, period, linkChangeOverride));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxHwInterruptCoalescingGet function
* @endinternal
*
* @brief   Gets the interrupt coalescing configuration parameters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE: interrupt coalescing is enabled
*                                      GT_FALSE: interrupt coalescing is disabled
* @param[out] periodPtr                - (pointer to) Minimal IDLE period between two consecutive
*                                      interrupts. The units are in nSec with resolution of
*                                      320nSec (due to HW resolution) and is relevant only in
*                                      case interrupt coalescing is enabled (enablePtr == GT_TRUE).
*                                      (APPLICABLE RANGES: 320..5242560).
* @param[out] linkChangeOverridePtr    - (pointer to)
*                                      GT_TRUE: A link change in one of the ports
*                                      results interrupt regardless of the (coalescing)
*                                      period.
*                                      GT_FALSE: A link change in one of the ports
*                                      does not results interrupt immediately but
*                                      according to the (coalescing) period.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The HW resolution for the interrupt coalescing period is 0.32uSec.
*
*/
static GT_STATUS internal_cpssPxHwInterruptCoalescingGet
(
    IN  GT_SW_DEV_NUM  devNum,
    OUT GT_BOOL     *enablePtr,
    OUT GT_U32      *periodPtr,
    OUT GT_BOOL     *linkChangeOverridePtr
)
{
    GT_U32 regAddr;   /* register's address */
    GT_U32 regData;   /* register's data */
    GT_STATUS rc;     /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(periodPtr);
    CPSS_NULL_PTR_CHECK_MAC(linkChangeOverridePtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.globalRegs.interruptCoalescing;

    rc = prvCpssHwPpReadRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, 0x13FFF, &regData);
    if( rc != GT_OK )
        return rc;

    *periodPtr = (GT_U32)(U32_GET_FIELD_MAC(regData, 0, 14) * MIN_INTERRUPT_COALESCING_PERIOD_CNS);

    *enablePtr = (*periodPtr == 0 ) ? GT_FALSE : GT_TRUE ;

    *linkChangeOverridePtr =
        (U32_GET_FIELD_MAC(regData, 16, 1) == 0) ? GT_TRUE : GT_FALSE ;

    return GT_OK;
}

/**
* @internal cpssPxHwInterruptCoalescingGet function
* @endinternal
*
* @brief   Gets the interrupt coalescing configuration parameters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE: interrupt coalescing is enabled
*                                      GT_FALSE: interrupt coalescing is disabled
* @param[out] periodPtr                - (pointer to) Minimal IDLE period between two consecutive
*                                      interrupts. The units are in nSec with resolution of
*                                      320nSec (due to HW resolution) and is relevant only in
*                                      case interrupt coalescing is enabled (enablePtr == GT_TRUE).
*                                      (APPLICABLE RANGES: 320..5242560).
* @param[out] linkChangeOverridePtr    - (pointer to)
*                                      GT_TRUE: A link change in one of the ports
*                                      results interrupt regardless of the (coalescing)
*                                      period.
*                                      GT_FALSE: A link change in one of the ports
*                                      does not results interrupt immediately but
*                                      according to the (coalescing) period.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The HW resolution for the interrupt coalescing period is 0.32uSec.
*
*/
GT_STATUS cpssPxHwInterruptCoalescingGet
(
    IN  GT_SW_DEV_NUM  devNum,
    OUT GT_BOOL     *enablePtr,
    OUT GT_U32      *periodPtr,
    OUT GT_BOOL     *linkChangeOverridePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxHwInterruptCoalescingGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr, periodPtr, linkChangeOverridePtr));

    rc = internal_cpssPxHwInterruptCoalescingGet(devNum, enablePtr, periodPtr, linkChangeOverridePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr, periodPtr, linkChangeOverridePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

#ifdef DEBUG_OPENED
/**
* @internal prvCpssPxPrintFineTuningDebugInfo function
* @endinternal
*
* @brief   PX prints information about Packet processor , fine tuning values
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - invalid devNum
*/
GT_STATUS prvCpssPxPrintFineTuningDebugInfo
(
    IN  GT_SW_DEV_NUM  devNum
)
{
    PRV_CPSS_PX_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    PRV_CPSS_PX_PP_HW_INFO_STC            *ppHwInfoPtr;
    PRV_CPSS_GEN_PP_CONFIG_STC *devInfoPtr = PRV_CPSS_PP_MAC(devNum);
    PRV_CPSS_PX_PP_CONFIG_STC *pxDevInfoPtr = PRV_CPSS_PX_PP_MAC(devNum);

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    fineTuningPtr = &pxDevInfoPtr->fineTuning;
    ppHwInfoPtr = &pxDevInfoPtr->hwInfo;

    DUMP_PRINT_MAC((" devNum = [%d] \n",devInfoPtr->devNum));
    DUMP_PRINT_MAC((" devType = [0x%8.8x] \n",devInfoPtr->devType));
    DUMP_PRINT_MAC((" revision = [%d] \n",devInfoPtr->revision));
    DUMP_PRINT_MAC((" devFamily = [%s] \n",
        devInfoPtr->devFamily == CPSS_PX_FAMILY_PIPE_E   ? STR(CPSS_PX_FAMILY_PIPE_E):
        " unknown "
    ));

    /************************/
    /* fine tuning - tables */
    /************************/

        /* CNC section */
    DUMP_PRINT_MAC((" [%s] = [%d] \n",
        NAME_AND_VALUE_MAC(fineTuningPtr->tableSize.cncBlocks               )
        ));
    DUMP_PRINT_MAC((" [%s] = [%d] \n",
        NAME_AND_VALUE_MAC(fineTuningPtr->tableSize.cncBlockNumEntries      )
        ));
        /* Transmit Descriptors */
    DUMP_PRINT_MAC((" [%s] = [%d] \n",
        NAME_AND_VALUE_MAC(fineTuningPtr->tableSize.transmitDescr           )
        ));
        /* Buffer Memory */
    DUMP_PRINT_MAC((" [%s] = [%d] \n",
        NAME_AND_VALUE_MAC(fineTuningPtr->tableSize.bufferMemory            )
        ));

    /***************/
    /* other info  */
    /***************/

    /************/
    /* HW info  */
    /************/
    DUMP_PRINT_MAC((" [%s] = [%d] \n",
        NAME_AND_VALUE_MAC(ppHwInfoPtr->cnc.cncBlocks)
        ));
    DUMP_PRINT_MAC((" [%s] = [%d] \n",
        NAME_AND_VALUE_MAC(ppHwInfoPtr->cnc.cncBlockNumEntries)
        ));
    return GT_OK;
}

/**
* @internal prvCpssPxPrintErrataNamesDebugInfo function
* @endinternal
*
* @brief   PX prints information about which errata WA implemented for the device
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - invalid devNum
*/
GT_STATUS prvCpssPxPrintErrataNamesDebugInfo
(
    IN  GT_SW_DEV_NUM  devNum
)
{
    GT_U32  ii;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    DUMP_PRINT_MAC(("cpssPxPrintErrataNamesInfo : the errata,RM WA for the device \n"));

    for(ii = 0 ; ii < PRV_CPSS_PX_ERRATA_MAX_NUM_E; ii++)
    {
        if(GT_TRUE == PRV_CPSS_PX_ERRATA_GET_MAC(devNum,ii))
        {
            if(errataNames[ii] == NULL)
            {
                DUMP_PRINT_MAC((" index %d is unknown \n" ,ii));
            }
            else
            {
                DUMP_PRINT_MAC((" %s \n" , errataNames[ii]));
            }
        }
    }

    return GT_OK;
}

#endif /*DEBUG_OPENED*/


/**
* @internal prvCpssPxHwInitNumOfSerdesGet function
* @endinternal
*
* @brief   Get number of SERDES lanes in device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval 0                        - for not applicable device
*                                       Number of SERDES lanes in device
*/
GT_U32 prvCpssPxHwInitNumOfSerdesGet
(
    IN GT_SW_DEV_NUM devNum
)
{
    GT_U32      lanesNumInDev; /* number of SERDES lanes */

    devNum = devNum;

    lanesNumInDev = PRV_CPSS_PX_SERDES_NUM_CNS;

    return lanesNumInDev;
}

/**
* @internal internal_cpssPxHwCoreClockGet function
* @endinternal
*
* @brief   This function returns the core clock value from HW.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The Pp's device number.
*
* @param[out] coreClkPtr               - Pp's core clock from HW and aligned with CPSS DB (MHz)
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxHwCoreClockGet
(
    IN  GT_SW_DEV_NUM  devNum,
    OUT GT_U32  *coreClkPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(coreClkPtr);

    rc = hwPpAutoDetectCoreClock(devNum,coreClkPtr);
    if (rc != GT_OK)
    {
        if(rc == GT_BAD_STATE)
        {
            *coreClkPtr = PRV_CPSS_PP_MAC(devNum)->coreClock;
        }
    }

    if((*coreClkPtr) != PRV_CPSS_PP_MAC(devNum)->coreClock)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "devNum[%d] : mismatch : DB hold coreClock[%d] but HW [%d]",
            devNum,
            PRV_CPSS_PP_MAC(devNum)->coreClock,
            (*coreClkPtr));
    }

    return GT_OK;
}

/**
* @internal cpssPxHwCoreClockGet function
* @endinternal
*
* @brief   This function returns the core clock value from HW.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The Pp's device number.
*
* @param[out] coreClkPtr               - Pp's core clock from HW and aligned with CPSS DB (MHz)
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHwCoreClockGet
(
    IN  GT_SW_DEV_NUM  devNum,
    OUT GT_U32  *coreClkPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxHwCoreClockGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, coreClkPtr));

    rc = internal_cpssPxHwCoreClockGet(devNum, coreClkPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, coreClkPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxUnitInterruptGenerationCheck function
* @endinternal
*
* @brief   This function checks if unit can generates interrupt.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] causeReg                 - Address of the interrupt cause register.
*
* @retval GT_OK                    - interrupt cause register is in supported unit
* @retval GT_NOT_SUPPORTED         - interrupt cause register is in unsupported unit
*/
static GT_STATUS prvCpssPxUnitInterruptGenerationCheck
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_U32        causeReg
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_UNIT_ENT unit;
    GT_U32                 unitItr; /* unit iterator */
    GT_U32                 unsupportedUnitsArraySize; /* size of prvCpssPxNoInterruptsGenerationUnits array */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    unsupportedUnitsArraySize=sizeof(prvCpssPxNoInterruptsGenerationUnits)/
                              sizeof(prvCpssPxNoInterruptsGenerationUnits[0]);

    unit = prvCpssPxHwRegAddrToUnitIdConvert (devNum, causeReg);
    if(unit == PRV_CPSS_DXCH_UNIT_LAST_E)
    {
        rc = GT_BAD_PARAM;
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "The causeReg[0x%8.8x] not part of known unit in the device",
            causeReg);
    }

    for (unitItr = 0; unitItr < unsupportedUnitsArraySize; unitItr++)
    {
        if (unit == prvCpssPxNoInterruptsGenerationUnits[unitItr])
        {
            /* Return GT_NOT_SUPPORTED if unit is in list of unsupported units */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}


/************************************************/
/* DUMMY implementation ... wait for proper one */
/************************************************/
GT_STATUS prvCpssPxPortDynamicPATxQHighSpeedPortInit
(
    IN  GT_SW_DEV_NUM  devNum
)
{
    devNum = devNum;

    /* dummy implementation */
    CPSS_TBD_BOOKMARK_PIPE
    return GT_OK;
}

/************************************************/
/* DUMMY implementation ... wait for proper one */
/************************************************/
GT_STATUS cpssPxPortTxGlobalDescLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32  limit
)
{
    devNum = devNum;
    limit = limit;

    /* dummy implementation */
    CPSS_TBD_BOOKMARK_PIPE
    return GT_OK;
}

/**
* @internal cpssPxMicroInitBasicCodeGenerate function
* @endinternal
*
* @brief   This function performs basic and per feature code generation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] initParamsPtr            - PX device init parameters.
* @param[in] featuresBitmap           - bitmap of cpss features participated in code generation.
* @param[in] deviceTypePtr            - (pointer to) The Pp's device type.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_ALREADY_EXIST         - the devNum already in use
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_BAD_VALUE             - the driver found unknown device type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxMicroInitBasicCodeGenerate
(
    IN GT_SW_DEV_NUM              devNum,
    IN CPSS_PX_INIT_INFO_STC      *initParamsPtr,
    IN GT_U32                     featuresBitmap,
    OUT CPSS_PP_DEVICE_TYPE       *deviceTypePtr
)
{
    GT_STATUS rc ;
    GT_U32      i;
    static GT_BOOL cpllFirstTime = GT_TRUE;
    GT_U32      unitIdx;
    PRV_CPSS_PX_PA_WORKSPACE_STC *paWsPtr;

    CPSS_NULL_PTR_CHECK_MAC(initParamsPtr);

    /* check input parameters */
    CPSS_PARAM_CHECK_MAX_MAC(devNum,PRV_CPSS_MAX_PP_DEVICES_CNS);

    if (cpllFirstTime == GT_TRUE)
    {
        /* close hw write prints */
        rc = cpssDrvPpHwTraceEnable(CAST_SW_DEVNUM(devNum),
                CPSS_DRV_HW_TRACE_TYPE_WRITE_DELAY_E, GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* initialize the cpssDriver , build the DB needed for the device */
        rc = hwPpPhase1Part1(devNum,initParamsPtr,deviceTypePtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpPhase1Part1");
        }

        /* open hw write prints */
        rc = cpssDrvPpHwTraceEnable(CAST_SW_DEVNUM(devNum),
                CPSS_DRV_HW_TRACE_TYPE_WRITE_DELAY_E, GT_TRUE);
        if(rc != GT_OK)
        {
            return rc;
        }

        for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
        {
            hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
            hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
        }
        rc = prvCpssPxPortCpllConfig(devNum);
        if(rc != GT_OK)
        {
            return rc;
        }

    }
    if ( (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PX_FAMILY_PIPE_E) &&
         (cpllFirstTime == GT_TRUE) )
    {
        cpllFirstTime = GT_FALSE;
        return GT_OK;
    }
    cpllFirstTime = GT_FALSE;

    /*  ONLY After DEV_INIT_DONE signal is asserted,
        software can read fully functional state from the chip.
    */
    rc = hwPpSip5StartInitSet(devNum, GT_FALSE);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpSip5StartInitSet");
    }

    /* check if LED POSITION JUST ON WA is needed */
    if ((featuresBitmap & 0x10) == GT_FALSE)
    {
        PRV_CPSS_PX_ERRATA_CLEAR_MAC(devNum, PRV_CPSS_PX_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E);
    }

    /*
    *       Part 2 of "phase 1"
    *       CPU enable
    *       set fine tuning parameters.
    *       bandwidth by dev family/revision/corecolock
    */
    rc = hwPpPhase1Part2(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpPhase1Part2");
    }

    /* init Cheetah tables database .
       called after : prvCpssPxPpConfigEnhancedInitFuncPtr
       but should be called even if prvCpssPxPpConfigEnhancedInitFuncPtr
       is NULL
    */

    /* close hw write prints */
    rc = cpssDrvPpHwTraceEnable(CAST_SW_DEVNUM(devNum),
            CPSS_DRV_HW_TRACE_TYPE_WRITE_DELAY_E, GT_FALSE);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssPxTablesAccessInit(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxTablesAccessInit");
    }

    /* open hw write prints */
    rc = cpssDrvPpHwTraceEnable(CAST_SW_DEVNUM(devNum),
            CPSS_DRV_HW_TRACE_TYPE_WRITE_DELAY_E, GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*
    *       Set mandatory default values for the device.
    *       handle init stage 'Errata and RMs'.
    */

    /* check if RM MG TERMINATOR BYPASS ENABLE WA is needed */
    if ((featuresBitmap & 0x20) == GT_FALSE)
    {
        PRV_CPSS_PX_ERRATA_CLEAR_MAC(devNum, PRV_CPSS_PX_RM_MG_TERMINATOR_BYPASS_ENABLE_WA_E);
    }

    /* check if RM TXQ QCN GLOBAL MISS CONFIG WA is needed */
    if ((featuresBitmap & 0x40) == GT_FALSE)
    {
        PRV_CPSS_PX_ERRATA_CLEAR_MAC(devNum, PRV_CPSS_PX_RM_TXQ_QCN_GLOBAL_MISS_CONFIG_WA_E);
    }

    /* check if RM AVS VDD LIMITS WA is needed */
    if ((featuresBitmap & 0x80) == GT_FALSE)
    {
        PRV_CPSS_PX_ERRATA_CLEAR_MAC(devNum, PRV_CPSS_PX_RM_AVS_VDD_LIMITS_WA_E);
    }

    rc = hwPpPxRegsDefault(devNum, initParamsPtr, GT_TRUE);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpPhase1Part3");
    }

    /* set RM (Register Miss-configuration) after tables initialization */
    rc = hwPpPhase1RegisterMisconfigurationAfterTablesInit(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwPpPhase1RegisterMisconfigurationAfterTablesInit");
    }

    /* close hw write prints */
    rc = cpssDrvPpHwTraceEnable(CAST_SW_DEVNUM(devNum),
            CPSS_DRV_HW_TRACE_TYPE_WRITE_DELAY_E, GT_FALSE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*
    *       update the cpssDriver
    *       initialize the AU descriptors
    *       initialize the network interface
    */
    rc = hwPpPhase2(devNum);

    rc = portPizzaArbiterIfInit(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* open hw write prints */
    rc = cpssDrvPpHwTraceEnable(CAST_SW_DEVNUM(devNum),
            CPSS_DRV_HW_TRACE_TYPE_WRITE_DELAY_E, GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssPxPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*--------------------------------------------------------------*
     * set work conserving mode for TxQ unit                        *
     *     cpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet() *
     *--------------------------------------------------------------*/
    if (paWsPtr->workConservingModeOnUnitListPtr != NULL)
    {
        for ( unitIdx = 0 ; paWsPtr->workConservingModeOnUnitListPtr[unitIdx] !=  CPSS_PX_PA_UNIT_UNDEFINED_E ; unitIdx++)
        {
            rc = cpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet(devNum,paWsPtr->workConservingModeOnUnitListPtr[unitIdx],GT_TRUE);
            if (GT_OK != rc)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxCfgHwDevNumGet function
* @endinternal
*
* @brief   Read HW device number
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] hwDevNumPtr              - Pointer to HW device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxCfgHwDevNumGet
(
    IN GT_U8    devNum,
    OUT GT_HW_DEV_NUM   *hwDevNumPtr
)
{
    GT_U32    regVal; /* register field value */
    GT_STATUS rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(hwDevNumPtr);

    /* Get 5 or 10 bits "ownDevNum" from Global Control register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.globalRegs.globalControl, 4, 10, &regVal);
    if(rc != GT_OK)
    {
        return rc;
    }

    *hwDevNumPtr = regVal;

    return GT_OK;
}

/**
* @internal cpssPxCfgHwDevNumGet function
* @endinternal
*
* @brief   Reads HW device number
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] hwDevNumPtr              - Pointer to HW device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCfgHwDevNumGet
(
    IN GT_U8    devNum,
    OUT GT_HW_DEV_NUM   *hwDevNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCfgHwDevNumGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hwDevNumPtr));

    rc = internal_cpssPxCfgHwDevNumGet(devNum, hwDevNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hwDevNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


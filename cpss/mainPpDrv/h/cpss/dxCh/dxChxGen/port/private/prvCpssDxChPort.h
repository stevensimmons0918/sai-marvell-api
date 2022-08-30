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
* @file prvCpssDxChPort.h
*
* @brief Includes structures definition for the use of CPSS DxCh Port lib .
*
*
* @version   58
********************************************************************************
*/
#ifndef __prvCpssDxChPorth
#define __prvCpssDxChPorth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/generic/port/cpssPortTx.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>

/* max number ports in cheetah */
#define PRV_CPSS_CHEETAH_MAX_PORTS_NUM_CNS      27

/* max number ports in cheetah 2 */
#define PRV_CPSS_CHEETAH2_MAX_PORTS_NUM_CNS     28

/* version 0 : The size of XLG ports MAC MIB counters entry in words */
#define PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_VER_0_CNS 32
/* version 1 : The size of XLG ports MAC MIB counters entry in words */
/* BC3 hold 'all 30 counters are 64 bits' */
#define PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_VER_1_CNS (30*2)

/* The size of XLG ports MAC MIB counters entry in words */
#define PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_VER_1_CNS

/* get the actual counters entry in words , for the device */
#define PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_GET(devNum)    \
    ((PRV_CPSS_DXCH_PP_MAC(devNum)->port.portMibCounters64Bits == GT_FALSE) ? \
        PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_VER_0_CNS:                      \
        PRV_CPSS_XLG_MIB_COUNTERS_ENTRY_SIZE_VER_1_CNS)


/* The size of RX ILKN ports MAC MIB counters entry in words */
#define PRV_CPSS_ILKN_RX_MIB_COUNTERS_ENTRY_SIZE_CNS 16


/* defines the maximal number of Port Drop Profiles according
   to Cheetah device type */
#define PRV_CPSS_DXCH_PORT_TX_DROP_PROFILE_MAX_MAC(_devNum)                     \
   ((!PRV_CPSS_SIP_5_CHECK_MAC(devNum)) ? CPSS_PORT_TX_DROP_PROFILE_8_E : CPSS_PORT_TX_DROP_PROFILE_16_E)

/* check the Drop Profile set range */
#define PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(profileSet,devNum) \
        if ((GT_U32)(profileSet) >                                      \
            (GT_U32)PRV_CPSS_DXCH_PORT_TX_DROP_PROFILE_MAX_MAC(devNum)) \
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* get the port mac register address - for register with name passed as parameter */
#define PRV_CPSS_DXCH_PORT_MAC_VAR_REG_MAC(devNum,portNum,portMacType,regName,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.cpuPortRegs.\
                        macRegsPerType[PRV_CPSS_PORT_GE_E].regName;\
    else                                                                        \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].\
                            macRegsPerType[portMacType].regName

/* get the register address - port mac control */
#define PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portNum,portMacType,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.cpuPortRegs.\
                        macRegsPerType[PRV_CPSS_PORT_GE_E].macCtrl;\
    else                                                                        \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].\
                            macRegsPerType[portMacType].macCtrl

/* get the register address - port mac control register1 */
#define PRV_CPSS_DXCH_PORT_MAC_CTRL1_REG_MAC(devNum,portNum,portMacType,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.cpuPortRegs. \
            macRegsPerType[PRV_CPSS_PORT_GE_E].macCtrl1;\
    else                                                                        \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].\
                            macRegsPerType[portMacType].macCtrl1

/* get the register address - port mac control register2 */
#define PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portNum,portMacType,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.cpuPortRegs.  \
            macRegsPerType[PRV_CPSS_PORT_GE_E].macCtrl2; \
    else                                                                        \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum]. \
        macRegsPerType[portMacType].macCtrl2

/* get the register address - port mac control register3 */
#define PRV_CPSS_DXCH_PORT_MAC_CTRL3_REG_MAC(devNum,portNum,portMacType,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.cpuPortRegs.  \
             macRegsPerType[PRV_CPSS_PORT_GE_E].macCtrl3; \
    else                                                                        \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum]. \
        macRegsPerType[portMacType].macCtrl3

/* get the register address - port mac control register4 */
#define PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portNum,portMacType,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.cpuPortRegs.  \
             macRegsPerType[PRV_CPSS_PORT_GE_E].macCtrl4; \
    else                                                                        \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum]. \
        macRegsPerType[portMacType].macCtrl4

/* get the register address - port interrupt mask register */
#define PRV_CPSS_DXCH_PORT_MAC_INT_MASK_REG_MAC(devNum,portNum,portMacType,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.cpuPortRegs.  \
             macRegsPerType[PRV_CPSS_PORT_GE_E].macIntMask; \
    else                                                                        \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum]. \
        macRegsPerType[portMacType].macIntMask


#define PRV_CPSS_DXCH_PORT_MAC_CTRL5_REG_MAC(devNum,portNum,portMacType,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED;\
    else                                                                        \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum]. \
        macRegsPerType[portMacType].macCtrl5;\

/* get the register address - Auto-Negotiation Configuration Register */
#define PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.cpuPortRegs.autoNegCtrl;\
    else                                                                        \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].autoNegCtrl

/* get the register address - Port<n> Status Register0 */
#define PRV_CPSS_DXCH_PORT_STATUS_CTRL_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.cpuPortRegs.macStatus;\
    else                                                                        \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].macStatus

/* get the register address - Port<n> Serial Parameters Configuration Register */
#define PRV_CPSS_DXCH_PORT_SERIAL_PARAM_CTRL_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.cpuPortRegs.serialParameters;\
    else                                                                        \
        *(regAddrPtr) = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].serialParameters

/* get the register address - port TXQ configuration */
#define PRV_CPSS_DXCH_PORT_TXQ_REG_MAC(devNum,portNum,regAddrPtr)\
    if(GT_OK != prvCpssDxChPortTxRegisterAddrGet(devNum,portNum,0,PRV_CPSS_DXCH_PORT_TX_REG_TYPE_TX_CONFIG_E,regAddrPtr))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG)

/* get the register address - port Token bucket configuration */
#define PRV_CPSS_DXCH_PORT_TOKEN_BUCKET_REG_MAC(devNum,portNum,regAddrPtr)\
    if(GT_OK != prvCpssDxChPortTxRegisterAddrGet(devNum,portNum,0,PRV_CPSS_DXCH_PORT_TX_REG_TYPE_TOKEN_BUCK_E,regAddrPtr))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG)

/* get the register address - port , Queue -- Token bucket configuration */
#define PRV_CPSS_DXCH_PORT_Q_TOKEN_BUCKET_REG_MAC(devNum,portNum,tcQueue,regAddrPtr)\
    if(GT_OK != prvCpssDxChPortTxRegisterAddrGet(devNum,portNum,tcQueue,PRV_CPSS_DXCH_PORT_TX_REG_TYPE_PRIO_TOKEN_BUCK_E,regAddrPtr))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG)

/* get the register address - port Descriptors Counter Register */
#define PRV_CPSS_DXCH_PORT_DESCR_COUNT_REG_MAC(devNum,portNum,regAddrPtr)\
    if(GT_OK != prvCpssDxChPortTxRegisterAddrGet(devNum,portNum,0,PRV_CPSS_DXCH_PORT_TX_REG_TYPE_DESCR_COUNTER_E,regAddrPtr))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG)

/* get the register address - port Buffers Counter Register */
#define PRV_CPSS_DXCH_PORT_BUFFER_COUNT_REG_MAC(devNum,portNum,regAddrPtr)\
    if(GT_OK != prvCpssDxChPortTxRegisterAddrGet(devNum,portNum,0,PRV_CPSS_DXCH_PORT_TX_REG_TYPE_BUFFER_COUNTER_E,regAddrPtr))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG)

/* get the register address - port TC Descriptors Counter Register */
#define PRV_CPSS_DXCH_PORT_TC_DESCR_COUNT_REG_MAC(devNum,portNum,tcQueue,regAddrPtr)\
    if(GT_OK != prvCpssDxChPortTxRegisterAddrGet(devNum,portNum,tcQueue,PRV_CPSS_DXCH_PORT_TX_REG_TYPE_TC_DESCR_COUNTER_E,regAddrPtr))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG)

/* get the register address - port TC Buffers Counter Register */
#define PRV_CPSS_DXCH_PORT_TC_BUFFER_COUNT_REG_MAC(devNum,portNum,tcQueue,regAddrPtr)\
    if(GT_OK != prvCpssDxChPortTxRegisterAddrGet(devNum,portNum,tcQueue,PRV_CPSS_DXCH_PORT_TX_REG_TYPE_TC_BUFFER_COUNTER_E,regAddrPtr))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG)

/* get the register address - WRR_WEIGHTS_0 Register */
#define PRV_CPSS_DXCH_PORT_WRR_WEIGHTS_0_REG_MAC(devNum,profile,regAddrPtr)\
    if(GT_OK != prvCpssDxChPortTxRegisterAddrGet(devNum,profile,0,PRV_CPSS_DXCH_PROFILE_TX_REG_TYPE_WRR_WEIGHTS_0_E,regAddrPtr))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG)

/* get the register address - WRR_WEIGHTS_1 Register */
#define PRV_CPSS_DXCH_PORT_WRR_WEIGHTS_1_REG_MAC(devNum,profile,regAddrPtr)\
    if(GT_OK != prvCpssDxChPortTxRegisterAddrGet(devNum,profile,0,PRV_CPSS_DXCH_PROFILE_TX_REG_TYPE_WRR_WEIGHTS_1_E,regAddrPtr))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG)

/* get the register address - DESCR_LIMIT Register */
#define PRV_CPSS_DXCH_PORT_WRR_STRICT_PRIO_REG_MAC(devNum,profile,regAddrPtr)\
    if(GT_OK != prvCpssDxChPortTxRegisterAddrGet(devNum,profile,0,PRV_CPSS_DXCH_PROFILE_TX_REG_TYPE_WRR_STRICT_PRIO_E,regAddrPtr))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG)

/* get the register address - DESCR_LIMIT_EN Register */
#define PRV_CPSS_DXCH_PORT_WRR_STRICT_PRIO_EN_REG_MAC(devNum,profile,regAddrPtr)\
    if(GT_OK != prvCpssDxChPortTxRegisterAddrGet(devNum,profile,0,PRV_CPSS_DXCH_PROFILE_TX_REG_TYPE_WRR_STRICT_PRIO_EN_E,regAddrPtr))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG)

/* get the register address - DESCR_LIMIT Register */
#define PRV_CPSS_DXCH_PORT_DESCR_LIMIT_REG_MAC(devNum,profile,regAddrPtr)\
    if(GT_OK != prvCpssDxChPortTxRegisterAddrGet(devNum,profile,0,PRV_CPSS_DXCH_PROFILE_TX_REG_TYPE_DESCR_LIMIT_E,regAddrPtr))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG)

/* defines the maximal number of Scheduler Profiles according
   to Cheetah device type */
#define PRV_CPSS_DXCH_PORT_TX_SCHEDULER_PROFILE_MAX_MAC(_devNum)                     \
    ((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(_devNum)) ? CPSS_PORT_TX_SCHEDULER_PROFILE_16_E : CPSS_PORT_TX_SCHEDULER_PROFILE_8_E)

/* check the Tx Queue scheduler profile set range */
#define PRV_CPSS_DXCH_SCHEDULER_PROFILE_CHECK_MAC(profileSet,devNum) \
        if ((GT_U32)(profileSet) >                                           \
            (GT_U32)PRV_CPSS_DXCH_PORT_TX_SCHEDULER_PROFILE_MAX_MAC(devNum)) \
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* check the WRR weight set range */
#define PRV_CPSS_DXCH_SCHEDULER_WRR_CHECK_MAC(weight) \
        if (weight == 0) \
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG)

/* defines the number of bits used in PFC threshold fields for sip5 device type */
#define PRV_CPSS_SIP5_PFC_BITS_NUM_MAC(_devNum)                     \
      ((PRV_CPSS_SIP_5_20_CHECK_MAC(_devNum)) ? 21 : 29 )

/* defines the max value of PFC thresholds for sip5 device type */
#define PRV_CPSS_SIP5_PFC_MAX_VALUE_MAC(_devNum)                     \
            (BIT_MASK_MAC(PRV_CPSS_SIP5_PFC_BITS_NUM_MAC(_devNum)))

typedef enum{
    PRV_CPSS_DXCH_PORT_TX_REG_TYPE_TX_CONFIG_E,
    PRV_CPSS_DXCH_PORT_TX_REG_TYPE_TOKEN_BUCK_E,
    PRV_CPSS_DXCH_PORT_TX_REG_TYPE_TOKEN_BUCK_LEN_E,
    PRV_CPSS_DXCH_PORT_TX_REG_TYPE_PRIO_TOKEN_BUCK_E,
    PRV_CPSS_DXCH_PORT_TX_REG_TYPE_PRIO_TOKEN_BUCK_LEN_E,
    PRV_CPSS_DXCH_PORT_TX_REG_TYPE_DESCR_COUNTER_E,
    PRV_CPSS_DXCH_PORT_TX_REG_TYPE_BUFFER_COUNTER_E,
    PRV_CPSS_DXCH_PORT_TX_REG_TYPE_TC_DESCR_COUNTER_E,
    PRV_CPSS_DXCH_PORT_TX_REG_TYPE_TC_BUFFER_COUNTER_E,

    PRV_CPSS_DXCH_PROFILE_TX_REG_TYPE_WRR_WEIGHTS_0_E,
    PRV_CPSS_DXCH_PROFILE_TX_REG_TYPE_WRR_WEIGHTS_1_E,
    PRV_CPSS_DXCH_PROFILE_TX_REG_TYPE_WRR_STRICT_PRIO_E,
    PRV_CPSS_DXCH_PROFILE_TX_REG_TYPE_WRR_STRICT_PRIO_EN_E,
    PRV_CPSS_DXCH_PROFILE_TX_REG_TYPE_DESCR_LIMIT_E
}PRV_CPSS_DXCH_PORT_TX_REG_TYPE_ENT;

/**
* @enum PRV_CPSS_DXCH_PORT_SERDES_REG_OFFSET_ENT
 *
 * @brief Corresponds to place of serdes configuration register address
 * in _lpSerdesConfig and must reflect changes there
*/
typedef enum{

    /** PLL/INTP Register 1 */
    PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG1_OFFSET_E,

    /** PLL/INTP Register 2 */
    PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG2_OFFSET_E,

    /** PLL/INTP Register 3 */
    PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG3_OFFSET_E,

    /** PLL/INTP Register 4 */
    PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG4_OFFSET_E,

    /** PLL/INTP Register 5 */
    PRV_CPSS_DXCH_PORT_SERDES_PLL_INTP_REG5_OFFSET_E,

    /** Callibration Register 0 */
    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG0_OFFSET_E,

    /** Calibration Register 1 */
    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG1_OFFSET_E,

    /** Calibration Register 2 */
    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG2_OFFSET_E,

    /** Calibration Register 3 */
    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG3_OFFSET_E,

    /** Calibration Register 5 */
    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG5_OFFSET_E,

    /** Calibration Register 7 */
    PRV_CPSS_DXCH_PORT_SERDES_CALIBRATION_REG7_OFFSET_E,

    /** Transmit Register 0 */
    PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG0_OFFSET_E,

    /** Transmit Register 1 */
    PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG1_OFFSET_E,

    /** Transmit Register 2 */
    PRV_CPSS_DXCH_PORT_SERDES_TRANSMIT_REG2_OFFSET_E,

    /** Reciever Register 0 */
    PRV_CPSS_DXCH_PORT_SERDES_RECEIVER_REG0_OFFSET_E,

    /** Reciever Register 1 */
    PRV_CPSS_DXCH_PORT_SERDES_RECEIVER_REG1_OFFSET_E,

    /** Reciever Register 2 */
    PRV_CPSS_DXCH_PORT_SERDES_RECEIVER_REG2_OFFSET_E,

    /** FFE Register 0 */
    PRV_CPSS_DXCH_PORT_SERDES_FFE_REG0_OFFSET_E,

    /** Analog Register 0 */
    PRV_CPSS_DXCH_PORT_SERDES_ANALOG_REG0_OFFSET_E,

    /** SLC Register 1 */
    PRV_CPSS_DXCH_PORT_SERDES_SLC_REG_OFFSET_E,

    /** Reference Register 1 */
    PRV_CPSS_DXCH_PORT_SERDES_REFERENCE_REG1_OFFSET_E,

    /** Power Register 0 */
    PRV_CPSS_DXCH_PORT_SERDES_POWER_REG0_OFFSET_E,

    /** Reset Register 0 */
    PRV_CPSS_DXCH_PORT_SERDES_RESET_REG0_OFFSET_E,

    /** Digital interface Reg. 0 */
    PRV_CPSS_DXCH_PORT_SERDES_DIGITAL_IF_REG0_OFFSET_E,

    /** Standalone Control Register 0 */
    PRV_CPSS_DXCH_PORT_SERDES_SA_CTRL_REG0_OFFSET_E,

    /** Serdes External Configuration 1(1 for xCat\0 for Lion) */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG1_OFFSET_E,

    /** Serdes External Configuration 2(2 for xCat\1 For Lion) */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_OFFSET_E,

    /** Serdes External Configuration 3(2 For Lion) */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG3_OFFSET_E,

    /** PHY Test Register 0 */
    PRV_CPSS_DXCH_PORT_SERDES_PHY_TEST_REG0_OFFSET_E,

    /** PHY Test Register 1 */
    PRV_CPSS_DXCH_PORT_SERDES_PHY_TEST_REG1_OFFSET_E,

    /** PHY Test Data Register 5 */
    PRV_CPSS_DXCH_PORT_SERDES_PHY_TEST_DATA_REG5_OFFSET_E,

    /** PHY Test PRBS Counter Register 2 */
    PRV_CPSS_DXCH_PORT_SERDES_PHY_TEST_PRBS_CNTR_REG2_OFFSET_E,

    /** @brief PHY Test
     *  PRBS Error Counter Register 0
     */
    PRV_CPSS_DXCH_PORT_SERDES_PHY_TEST_PRBS_ERROR_CNTR_REG0_OFFSET_E,

    /** @brief PHY Test
     *  PRBS Error Counter Register 1
     */
    PRV_CPSS_DXCH_PORT_SERDES_PHY_TEST_PRBS_ERROR_CNTR_REG1_OFFSET_E,

    /** DFE F0-F1 Coefficient Control Reg. */
    PRV_CPSS_DXCH_PORT_SERDES_DFE_F0F1_COEFFICIENT_CTRL_OFFSET_E,

    /** @brief Number of serdes registers
     *  in structure
     */
    PRV_CPSS_DXCH_PORT_SERDES_REG_OFFSET_NUM_E

} PRV_CPSS_DXCH_PORT_SERDES_REG_OFFSET_ENT;

/**
* @enum PRV_CPSS_DXCH_PORT_SERDES_EXT_REG_ENT
 *
 * @brief Defines SERDES external configuration:
 * Pre-init and post-init.
*/
typedef enum{

    /** Serdes External Configuration 2/0 Register */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_0_E,

    /** Serdes External Configuration 1 Register */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG1_E,

    /** Serdes External Configuration 2/1 Register */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_1_E,

    /** Serdes External Configuration 2/2 Register */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_2_E,

    /** Serdes External Configuration 2/3 Register */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_3_E,

    /** Serdes External Configuration 2/4 Register */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_REG2_4_E,

    /** Number of Serdes External Registers */
    PRV_CPSS_DXCH_PORT_SERDES_EXT_REG_NUM_E

} PRV_CPSS_DXCH_PORT_SERDES_EXT_REG_ENT;

/* type for pointer to ...SerdesPowerUpSequence matrix */
typedef GT_U32 PRV_CPSS_DXCH_PORT_SERDES_POWER_UP_ARRAY[CPSS_DXCH_PORT_SERDES_SPEED_NA_E+2];

/* type for pointer to lpSerdesExtConfig... matrix */
typedef GT_U32 PRV_CPSS_DXCH_PORT_SERDES_EXT_CFG_ARRAY[2];

/* Values for xCat A2 and above Short Reach 125MHz SerDes RefClk */
extern GT_U32 xcatSerdesPowerUpSequence[][CPSS_DXCH_PORT_SERDES_SPEED_NA_E+2];

/* Values for xCat A1 Short Reach 125MHz SerDes RefClk */
extern GT_U32 xcatA1SerdesPowerUpSequence[][CPSS_DXCH_PORT_SERDES_SPEED_NA_E+2];

/* Values for xCat2 Short Reach 125MHz SerDes RefClk */
extern GT_U32 xcat2SerdesPowerUpSequence[][CPSS_DXCH_PORT_SERDES_SPEED_NA_E+2];
/* Values for Lion B0 Long Reach 156.25MHz SerDes RefClk */
extern GT_U32 lionSerdesPowerUpSequence[][CPSS_DXCH_PORT_SERDES_SPEED_NA_E+2];

/* Values for xCat External registers configuration */
extern GT_U32 lpSerdesExtConfig[PRV_CPSS_DXCH_PORT_SERDES_EXT_REG_NUM_E][2];
/* Values for Lion B0 External registers configuration */
extern GT_U32 lpSerdesExtConfig_Lion_B0_156Clk_LR[PRV_CPSS_DXCH_PORT_SERDES_EXT_REG_NUM_E][2];
/* Values for xCat2 External registers configuration */
extern GT_U32 lpSerdesExtConfig_xCat2_125Clk[PRV_CPSS_DXCH_PORT_SERDES_EXT_REG_NUM_E][2];

/*******************************************************************************
* PRV_CPSS_DXCH_PORT_INTERFACE_MODE_SET_FUN
*
* DESCRIPTION:
*       Sets Interface mode on a specified port.
*
* INPUTS:
*       dev   - physical device number
*       port  - physical port number
*       ifMode - interface mode
*
* OUTPUTS:
*       none
*
* RETURNS:
*       GT_OK             - on success
*       GT_BAD_PARAM      - on wrong port number or device
*       GT_HW_ERROR       - on hardware error
*       GT_NOT_SUPPORTED  - on not supported interface for given port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DXCH_PORT_INTERFACE_MODE_SET_FUN)
(
    IN  GT_U8                           dev,
    IN  GT_PHYSICAL_PORT_NUM            port,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode
);

/*******************************************************************************
* PRV_CPSS_DXCH_PORT_INTERFACE_MODE_GET_FUN
*
* DESCRIPTION:
*       Gets Interface mode on a specified port.
*
* INPUTS:
*       dev   - physical device number
*       port  - physical port number
*
* OUTPUTS:
*       ifModePtr - (ptr to) current interface mode
*
* RETURNS:
*       GT_OK             - on success
*       GT_BAD_PARAM      - on wrong port number or device
*       GT_BAD_PTR        - on null pointer
*       GT_HW_ERROR       - on hardware error
*       GT_NOT_SUPPORTED  - on not supported interface for given port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DXCH_PORT_INTERFACE_MODE_GET_FUN)
(
    IN  GT_U8                           dev,
    IN  GT_PHYSICAL_PORT_NUM            port,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    *ifModePtr
);

/*******************************************************************************
* PRV_CPSS_DXCH_PORT_SPEED_SET_FUN
*
* DESCRIPTION:
*       Sets speed for specified port on specified device.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number or CPU port
*       speed    - port speed
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device or speed
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported speed
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_STATE             - Speed of the port group member is different
*                                  from speed for setting.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DXCH_PORT_SPEED_SET_FUN)
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
);

/*******************************************************************************
* PRV_CPSS_DXCH_PORT_SPEED_GET_FUN
*
* DESCRIPTION:
*       Gets speed for specified port on specified device.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number or CPU port
*
* OUTPUTS:
*       speedPtr - port speed
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device or speed
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported speed
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_STATE             - Speed of the port group member is different
*                                  from speed for setting.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DXCH_PORT_SPEED_GET_FUN)
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_SPEED_ENT  *speedPtr
);

/******************************************************************************
* PRV_CPSS_DXCH_PORT_SERDES_POWER_STATUS_SET_FUN
*
* DESCRIPTION:
*       Sets power state of SERDES port lanes according to port capabilities.
*       XG / XAUI ports: All 4 (TX or RX) lanes are set simultanuously.
*       HX / QX ports: lanes 0,1 or TX and RX may be set separately.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number
*       direction - may be either CPSS_PORT_DIRECTION_RX_E,
*                                 CPSS_PORT_DIRECTION_TX_E, or
*                                 CPSS_PORT_DIRECTION_BOTH_E.
*       lanesBmp  - bitmap of SERDES lanes (bit 0-> lane 0, etc.)
*       powerUp   - GT_TRUE  = power up, GT_FALSE = power down
*
* OUTPUTS:
*       None.
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on bad parameters
*     GT_FAIL          - on error
*     GT_NOT_SUPPORTED - HW does not support the requested operation
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*     DxCh3 and above devices supports only CPSS_PORT_DIRECTION_BOTH_E.
*
******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DXCH_PORT_SERDES_POWER_STATUS_SET_FUN)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
);

/*******************************************************************************
 * Typedef: struct PRV_CPSS_DXCH_PORT_OBJ_STC
 *
 * Description: A structure to hold PORT methods entry points
 *
 *
 * Fields:
 *      setPortInterfaceMode    - set port interface mode
 *      getPortInterfaceMode    - get current interface mode of port
 *      setPortSpeed            - set port speed
 *      getPortSpeed            - get port speed
 *      setSerdesPowerStatus    - set serdes power status
 *                                  (power down or run power up sequence)
 *
 */
#include <cpss/common/port/private/prvCpssPortTypes.h>
typedef struct
{
    PRV_CPSS_DXCH_PORT_INTERFACE_MODE_SET_FUN   setPortInterfaceMode[CPSS_PORT_INTERFACE_MODE_NA_E];
    PRV_CPSS_DXCH_PORT_INTERFACE_MODE_GET_FUN   getPortInterfaceMode;
    PRV_CPSS_DXCH_PORT_SPEED_SET_FUN            setPortSpeed[PRV_CPSS_XG_PORT_OPTIONS_MAX_E];
    PRV_CPSS_DXCH_PORT_SPEED_GET_FUN            getPortSpeed;
    PRV_CPSS_DXCH_PORT_SERDES_POWER_STATUS_SET_FUN setSerdesPowerStatus;

} PRV_CPSS_DXCH_PORT_OBJ_STC, *PRV_CPSS_DXCH_PORT_OBJ_PTR;

/**
* @internal prvCpssDxChLpPreInitSerdesSequence function
* @endinternal
*
* @brief   Make pre-init sequence for given serdes.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - ports group number
* @param[in] serdesNum                - given SERDES number
* @param[in] serdesExtCnfg1Value      - value for Serdes External Cnfg 1 register .
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on not initialized serdes
*/
GT_STATUS prvCpssDxChLpPreInitSerdesSequence
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               serdesNum,
    IN  GT_U32               serdesExtCnfg1Value
);

/**
* @internal prvCpssDxChPortMacResetStateSet function
* @endinternal
*
* @brief   Set MAC Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port MAC is under Reset
*                                      GT_FALSE - Port MAC is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMacResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);

/**
* @internal prvCpssDxChPortMacResetStateGet function
* @endinternal
*
* @brief   Get MAC Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU)
*
* @param[out] statePtr                 - Reset state
*                                      GT_TRUE   - Port MAC is under Reset
*                                      GT_FALSE - Port MAC is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - statePtr == NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMacResetStateGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL   *statePtr
);

/**
* @internal prvCpssDxChPortXgPcsResetStateSet function
* @endinternal
*
* @brief   Set XPCS Reset state of XG unit including internal features.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
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
GT_STATUS prvCpssDxChPortXgPcsResetStateSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
);

/**
* @internal prvCpssDxChPortXgPcsResetStateGet function
* @endinternal
*
* @brief   Set XPCS Reset state of XG unit including internal features.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
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
GT_STATUS prvCpssDxChPortXgPcsResetStateGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *statePtr
);

/**
* @internal prvCpssDxChLpCheckAndInitNeededSerdes function
* @endinternal
*
* @brief   Make serdes initialization and enable access to serdes registers
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - ports group number
* @param[in] startSerdes              - start SERDES number
* @param[in] serdesesNum              - number of  SERDES per port
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on not initialized serdes
*/
GT_STATUS prvCpssDxChLpCheckAndInitNeededSerdes
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               startSerdes,
    IN  GT_U32               serdesesNum
);

/**
* @internal prvCpssDxCh3PortSerdesPowerUpDownGet function
* @endinternal
*
* @brief   Get power up or down state to port and serdes.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] powerUpPtr               - (pointer to) power up state.
*                                      GT_TRUE  - power up
*                                      GT_FALSE - power down
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxCh3PortSerdesPowerUpDownGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL   *powerUpPtr
);

/**
* @internal prvCpssDxChPortTxRegisterAddrGet function
* @endinternal
*
* @brief   get register address for 'txq per port' .
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] primaryIndex             - primary index , can be used as :
*                                      port number (also CPU port)
*                                      or as profile number
* @param[in] secondaryIndex           - secondary index , can be used as TC
* @param[in] registerType             - register type
*
* @param[out] regAddrPtr               - (pointer to) register address
*                                       GT_OK
*/
GT_STATUS prvCpssDxChPortTxRegisterAddrGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   primaryIndex,
    IN  GT_U32   secondaryIndex,
    IN  PRV_CPSS_DXCH_PORT_TX_REG_TYPE_ENT  registerType,
    OUT GT_U32   *regAddrPtr
);

/**
* @internal prvCpssDxChPortPfcCascadeEnableSet function
* @endinternal
*
* @brief   Enable/Disable insertion of DSA tag for PFC frames
*         transmitted on given port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
* @param[in] enable                   - GT_TRUE - PFC packets are transmitted with a DSA tag.
*                                      GT_FALSE - PFC packets are transmitted without DSA tag.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPfcCascadeEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL enable
);

/**
* @internal prvCpssDxChPortIfModeCheckAndSerdesNumGet function
* @endinternal
*
* @brief   Get number of first serdes and quantity of serdeses occupied by given
*         port in given interface mode
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port media interface mode
*
* @param[out] startSerdesPtr           - first used serdes number
* @param[out] numOfSerdesLanesPtr      - quantity of serdeses occupied
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if port doesn't support given interface mode
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS prvCpssDxChPortIfModeCheckAndSerdesNumGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    OUT GT_U32                          *startSerdesPtr,
    OUT GT_U32                          *numOfSerdesLanesPtr
);

/**
* @internal prvCpssDxChPortIfModeSerdesNumGet function
* @endinternal
*
* @brief   Get number of first serdes and quantity of serdeses occupied by given
*         port in given interface mode. Extended function used directly only in
*         special cases when naturally error would be returned.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port media interface mode
*
* @param[out] startSerdesPtr           - first used serdes number
* @param[out] numOfSerdesLanesPtr      - quantity of serdeses occupied
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if port doesn't support given interface mode
* @retval GT_NOT_SUPPORTED         - wrong device family
*
* @note While working on Lion2/3 prior to call this function, port speed must
*       be defined.
*
*/
GT_STATUS prvCpssDxChPortIfModeSerdesNumGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    OUT GT_U32                          *startSerdesPtr,
    OUT GT_U32                          *numOfSerdesLanesPtr
);

/**
* @internal prvCpssDxChPortParamsGet function
* @endinternal
*
* @brief   get the active serdes/lanes list and quantity of
*          serdeses occupied by given port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[out] activeLanePtr          - number of active serdeses
*                                        and number of active
*                                        serdeses.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortParamsGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_ACTIVE_LANES_STC *activeLanePtr
);

/**
* @internal prvCpssDxChPortNumberOfSerdesLanesGet function
* @endinternal
*
* @brief   Get number of first SERDES and quantity of SERDESes occupied by given
*         port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] firstSerdesPtr           - (pointer to) first used SERDES number
* @param[out] numOfSerdesLanesPtr      - (pointer to) quantity of SERDESes occupied
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - wrong device family
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortNumberOfSerdesLanesGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *firstSerdesPtr,
    OUT GT_U32                          *numOfSerdesLanesPtr
);

/**
* @internal prvCpssDxChPortTxQueueingEnableSet function
* @endinternal
*
* @brief   Enable/Disable enqueuing to a Traffic Class queue
*         on the specified port of specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] tcQueue                  - traffic class queue on this device (0..7)
* @param[in] enable                   - GT_TRUE,  enqueuing to the queue
*                                      GT_FALSE, disable enqueuing to the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortTxQueueingEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8     tcQueue,
    IN  GT_BOOL   enable
);

/**
* @internal prvCpssDxChPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   enable
);

/**
* @internal prvCpssDxChPortPcsLoopbackEnableSet function
* @endinternal
*
* @brief   Set the PCS Loopback state in the packet processor MAC port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPcsLoopbackEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL     enable
);

/**
* @internal prvCpssDxChPortEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable status of a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - Pointer to the Get Enable/disable state of the port.
*                                      GT_TRUE for enabled port, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL   *statePtr
);

/**
* @internal prvCpssDxChPortTxBindPortToDpSet function
* @endinternal
*
* @brief   Bind a Physical Port to a specific Drop Profile Set.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChPortTxBindPortToDpSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet
);


/**
* @internal prvCpssDxChIsCgUnitInUse function
* @endinternal
*
* @brief   Check if given pair portNum and portMode use CG MAC
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port MAC number (not CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - interface speed
*
* @param[out] isCgUnitInUsePtr         - GT_TRUE ? GC MAC in use
*                                      GT_FALSE ? GC MAC not in use
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if one of input parameters wrong
* @retval GT_BAD_PTR               - if supportedPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIsCgUnitInUse
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *isCgUnitInUsePtr
);

/**
* @internal prvCpssDxChPortSerdesFunctionsObjInit function
* @endinternal
*
* @brief   Init and bind common function pointers to DXCH port serdes functions.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChPortSerdesFunctionsObjInit
(
    IN GT_U8 devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChPorth */



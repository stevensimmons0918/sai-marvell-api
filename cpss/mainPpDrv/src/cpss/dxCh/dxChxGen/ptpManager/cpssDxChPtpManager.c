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
* @file cpssDxChPtpManager.c
*
* @brief Precision Time Protocol Manager Support - function implementations.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/ptpManager/cpssDxChPtpManager.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/ptp/private/prvCpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceFalcon.h>
#include <cpss/dxCh/dxChxGen/ptp/private/prvCpssDxChPtpLog.h>
#include <cpss/dxCh/dxChxGen/ptpManager/private/prvCpssDxChPtpManagerLog.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceHawk.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* TAI Iterator LOOP Header */
#define PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum,taiNumber,taiIdPtr)    \
    while (numIterations > 0)                                                               \
    {                                                                                       \
        if (taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E)                                    \
        {                                                                                   \
            numIterations = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.maxNumOfTais;      \
            (taiIdPtr)->taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;                           \
            taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;                                       \
        }                                                                                   \
        else                                                                                \
        {                                                                                   \
            (taiIdPtr)->taiNumber = taiNumber;                                              \
        }                                                                                   \

/* TAI Iterator LOOP footer */
#define PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, taiIdPtr)            \
        numIterations--;                                                                    \
        taiNumber++;                                                                        \
    }

/* check tai number */
#define PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(_dev, _taiNumber)                               \
    if (_taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E)                                           \
    {                                                                                           \
        /* do nothing */                                                                        \
    }                                                                                           \
    else if (_taiNumber >= PRV_CPSS_DXCH_PP_MAC(_dev)->hwInfo.ptpInfo.maxNumOfTais)             \
    {                                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                          \
    }                                                                                           \

/* wether to skip this port */
static GT_BOOL prvCpssDxChPortSkipCheck
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
)
{
    GT_BOOL isCpu, isValid;
    GT_STATUS rc,rc1;

    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if ((portNum) >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum))
            return GT_TRUE; /* no more */
        rc = cpssDxChPortPhysicalPortMapIsCpuGet(devNum, portNum, &isCpu);
        rc1 = cpssDxChPortPhysicalPortMapIsValidGet(devNum, portNum, &isValid);
        if((rc != GT_OK) || (isCpu == GT_TRUE) ||(rc1 != GT_OK) || (isValid != GT_TRUE))
            return GT_TRUE;
    }
    else
    {
        if(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType == PRV_CPSS_PORT_NOT_EXISTS_E)
            return GT_TRUE;
    }
    return GT_FALSE;
}

/**
* @internal internal_cpssDxChPtpManagerPtpInit function
* @endinternal
*
* @brief  Set one-time global PTP-related settings.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] outputInterfaceConfPtr   - (pointer to) clock output interface configuration.
* @param[in] ptpOverEthernetConfPtr   - (pointer to) PTP over Ethernet configuration.
* @param[in] ptpOverIpUdpConfPtr      - (pointer to) PTP over IP/UDP configuration.
* @param[in] ptpTsTagGlobalConfPtr    - (pointer to) PTP timestamp tag global configuration.
* @param[in] cpuCode                  - The base of CPU code assigned to PTP packets.
*                                       The command is configured in cpssDxChPtpManagerPortCommandAssignment
*                                       per{source port, domain index, message type}.
*                                       Applicable if command == CPSS_PACKET_CMD_MIRROR_TO_CPU_E or
*                                       CPSS_PACKET_CMD_TRAP_TO_CPU_E.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
static GT_STATUS internal_cpssDxChPtpManagerPtpInit
(
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC   *outputInterfaceConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC      *ptpOverEthernetConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC        *ptpOverIpUdpConfPtr,
    IN CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC              *ptpTsTagGlobalConfPtr,
    IN CPSS_NET_RX_CPU_CODE_ENT                         cpuCode
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT taiNumber;
    GT_U32 ptpClkInKhz;
    CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT pClkSelMode;
    CPSS_DXCH_PTP_TAI_TOD_STEP_STC todStep;
    GT_U32 numIterations=1,i,j,k,m,offset;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(outputInterfaceConfPtr);
    CPSS_NULL_PTR_CHECK_MAC(ptpOverEthernetConfPtr);
    CPSS_NULL_PTR_CHECK_MAC(ptpOverIpUdpConfPtr);
    CPSS_NULL_PTR_CHECK_MAC(ptpTsTagGlobalConfPtr);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,outputInterfaceConfPtr->taiNumber);

    /* set default values in DB */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->ptpMgrInfo.initDone == GT_FALSE)
    {
        /*avoid memory leak ,release memory if already allocated*/
        FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr);

        /* allocation of physical ports information - phyPortInfoArray */
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr = (CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC*)
            cpssOsMalloc(PRV_CPSS_PP_MAC(devNum)->numOfPorts*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E*sizeof(CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC));

        if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        /* Shadow of PTP Delay Values per port speed/interface/fec */
        for (i = 0; i < PRV_CPSS_PP_MAC(devNum)->numOfPorts; i++)
        {
            for (j = 0; j < CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E; j++)
            {
                for (k = 0; k < CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E; k++)
                {
                    for (m = 0; m < CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E; m++)
                    {
                        offset = i*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                                 j*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                                 k*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E + m;

                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].egressPipeDelay = 0;
                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].egressPipeDelayEncrypted = 0;
                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].egressPipeDelayFsuOffset = 0;
                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ingressPortDelay = 0;
                    }
                }
            }
        }

        PRV_CPSS_DXCH_PP_MAC(devNum)->ptpMgrInfo.initDone = GT_TRUE;
    }

    taiNumber = outputInterfaceConfPtr->taiNumber;

    switch (outputInterfaceConfPtr->outputInterfaceMode)
    {
    case CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_DISABLED_E:
        pClkSelMode = CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_DISABLED_E;
        break;

    case CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PCLK_E:
        pClkSelMode = CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_PCLK_E;
        break;

    case CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PPS_MASTER_E:
        pClkSelMode = CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_CLOCK_GEN_E;
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    ptpClkInKhz = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.ptpClkInKhz;

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* select the PCLK source */
        rc = cpssDxChPtpTaiPClkSelectionModeSet(devNum,&taiId,pClkSelMode);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
            if (outputInterfaceConfPtr->outputInterfaceMode == CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PCLK_E ||
                outputInterfaceConfPtr->outputInterfaceMode == CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PPS_MASTER_E)
            {
                /* set master device driving the PTP_PULSE */
                rc = cpssDxChPtpTaiPtpPulseInterfaceSet(devNum,CPSS_DIRECTION_EGRESS_E,taiId.taiNumber);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }

        if (outputInterfaceConfPtr->outputInterfaceMode == CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PCLK_E)
        {
            if (outputInterfaceConfPtr->nanoSeconds != 0)
            {
                /* set PCLK cycle */
                rc = cpssDxChPtpTaiPClkCycleSet(devNum,&taiId,outputInterfaceConfPtr->nanoSeconds);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }

        /* Configure TAIs nanosec step values */
        todStep.nanoSeconds = 1000000/ptpClkInKhz;
        todStep.fracNanoSeconds = (GT_U32)(0xFFFFFFFF *
                  (1000000.0/ptpClkInKhz - todStep.nanoSeconds) +
                  (1000000.0/ptpClkInKhz - todStep.nanoSeconds));

        rc = cpssDxChPtpTaiTodStepSet(devNum, &taiId, &todStep);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Configures input TAI clock's selection to PTP PLL; IronMan & AC5X use Core Clock for TAIs */
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) && (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5X_E) && (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E))
        {
            rc = cpssDxChPtpTaiInputClockSelectSet(devNum,taiId.taiNumber,CPSS_DXCH_PTP_TAI_CLOCK_SELECT_PTP_PLL_E,CPSS_DXCH_PTP_25_FREQ_E);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    /* configure the base of CPU code assigned to PTP packets */
    rc = cpssDxChPtpCpuCodeBaseSet(devNum,cpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* enable PTP over Ethernet frames identification */
    rc = cpssDxChPtpOverEthernetEnableSet(devNum,ptpOverEthernetConfPtr->ptpOverEthernetEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (ptpOverEthernetConfPtr->ptpOverEthernetEnable == GT_TRUE)
    {
        /* set the PTP frame Ethertype0 */
        rc = cpssDxChPtpEtherTypeSet(devNum,0,ptpOverEthernetConfPtr->etherType0value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set the PTP frame Ethertype1 */
        rc = cpssDxChPtpEtherTypeSet(devNum,1,ptpOverEthernetConfPtr->etherType1value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* enable PTP over UDP rames identification */
    rc = cpssDxChPtpOverUdpEnableSet(devNum,ptpOverIpUdpConfPtr->ptpOverIpUdpEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (ptpOverIpUdpConfPtr->ptpOverIpUdpEnable == GT_TRUE)
    {
        /* set UDP destination port0 */
        rc = cpssDxChPtpUdpDestPortsSet(devNum,0,ptpOverIpUdpConfPtr->udpPort0value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set UDP destination port1 */
        rc = cpssDxChPtpUdpDestPortsSet(devNum,1,ptpOverIpUdpConfPtr->udpPort1value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* select mode for handling the UDP checksum per packet type */
        rc = cpssDxChPtpTsUdpChecksumUpdateModeSet(devNum,&(ptpOverIpUdpConfPtr->udpCsUpdMode));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Avoid bypass all CTSUs */
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = cpssDxChPtpTsuBypassEnableSet(devNum,GT_FALSE);
    }

    /* set Timestamp Global Configuration */
    rc = cpssDxChPtpTsTagGlobalCfgSet(devNum,ptpTsTagGlobalConfPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerPtpInit function
* @endinternal
*
* @brief  Set one-time global PTP-related settings.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] outputInterfaceConfPtr   - (pointer to) clock output interface configuration.
* @param[in] ptpOverEthernetConfPtr   - (pointer to) PTP over Ethernet configuration.
* @param[in] ptpOverIpUdpConfPtr      - (pointer to) PTP over IP/UDP configuration.
* @param[in] ptpTsTagGlobalConfPtr    - (pointer to) PTP timestamp tag global configuration.
* @param[in] cpuCode                  - The base of CPU code assigned to PTP packets.
*                                       The command is configured in cpssDxChPtpManagerPortCommandAssignment
*                                       per{source port, domain index, message type}.
*                                       Applicable if command == CPSS_PACKET_CMD_MIRROR_TO_CPU_E or
*                                       CPSS_PACKET_CMD_TRAP_TO_CPU_E.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
GT_STATUS cpssDxChPtpManagerPtpInit
(
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC   *outputInterfaceConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC      *ptpOverEthernetConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC        *ptpOverIpUdpConfPtr,
    IN CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC              *ptpTsTagGlobalConfPtr,
    IN CPSS_NET_RX_CPU_CODE_ENT                         cpuCode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPtpInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, outputInterfaceConfPtr, ptpOverEthernetConfPtr, ptpOverIpUdpConfPtr, ptpTsTagGlobalConfPtr, cpuCode));

    rc = internal_cpssDxChPtpManagerPtpInit(devNum, outputInterfaceConfPtr, ptpOverEthernetConfPtr, ptpOverIpUdpConfPtr,  ptpTsTagGlobalConfPtr, cpuCode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, outputInterfaceConfPtr, ptpOverEthernetConfPtr, ptpOverIpUdpConfPtr, ptpTsTagGlobalConfPtr, cpuCode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerInputPulseSet function
* @endinternal
*
* @brief   Set input pulse configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] inputMode                - Whether an incoming pulse comes from the PULSE pin (slave PPS) or the CLK pin.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E,
*                                           CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerInputPulseSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT   inputMode
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    GT_U32 numIterations=1;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E);
    if (inputMode == CPSS_DXCH_PTP_TAI_PULSE_IN_DISABLED_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* select the triggering mode */
        rc = cpssDxChPtpTaiPulseInModeSet(devNum,taiId.taiNumber,inputMode);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set slave device reacting to an incoming PTP_PULSE */
        rc = cpssDxChPtpTaiPtpPulseInterfaceSet(devNum,CPSS_DIRECTION_INGRESS_E,taiId.taiNumber);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerInputPulseSet function
* @endinternal
*
* @brief   Set input pulse configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] inputMode                - Whether an incoming pulse comes from the PULSE pin (slave PPS) or the CLK pin.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E,
*                                           CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerInputPulseSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT   inputMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerInputPulseSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, inputMode));

    rc = internal_cpssDxChPtpManagerInputPulseSet(devNum, taiNumber, inputMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, inputMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerMasterPpsActivate function
* @endinternal
*
* @brief   Activating the TAI as Master PPS.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] phaseValue               - rising phase value time of generation clock, in nanoseconds.
* @param[in] seconds                  - seconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] nanoSeconds              - nanoseconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..2^28-1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
static GT_STATUS internal_cpssDxChPtpManagerMasterPpsActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  GT_U32                                phaseValue,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    CPSS_DXCH_PTP_TOD_COUNT_STC todValue;
    GT_U32 numIterations=1;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    cpssOsMemSet(&todValue, 0, sizeof(todValue));

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* enable PPS output */
        rc = cpssDxChPtpTaiClockGenerationModeSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_CLOCK_GEN_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* trigger a signal at a specific nanosecond following a whole second */
        rc = cpssDxChPtpTaiClockGenerationEdgeSet(devNum, &taiId, phaseValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set PPS frequency */
        rc = cpssDxChPtpTaiClockCycleSet(devNum,&taiId,seconds,nanoSeconds);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* master PPS triggering every round second + t ns, where t is todValue.nanoSeconds */
        todValue.nanoSeconds = nanoSeconds;
        rc = cpssDxChPtpTaiTodSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,&todValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* prepare the trigger update information */
        rc = cpssDxChPtpTodCounterFunctionSet(devNum,0 /* not relevant */,&taiId,CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* trigger update*/
        rc = cpssDxChPtpTodCounterFunctionTriggerSet(devNum,0 /* not relevant */,&taiId);
        if (rc != GT_OK)
        {
            return rc;
        }

    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerMasterPpsActivate function
* @endinternal
*
* @brief   Activating the TAI as Master PPS.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] phaseValue               - rising phase value time of generation clock, in nanoseconds.
* @param[in] seconds                  - seconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] nanoSeconds              - nanoseconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..2^28-1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
GT_STATUS cpssDxChPtpManagerMasterPpsActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  GT_U32                                phaseValue,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerMasterPpsActivate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, phaseValue, seconds, nanoSeconds));

    rc = internal_cpssDxChPtpManagerMasterPpsActivate(devNum, taiNumber, phaseValue, seconds, nanoSeconds);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, phaseValue, seconds, nanoSeconds));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerSlavePpsActivate function
* @endinternal
*
* @brief   Activating the TAI as Slave PPS.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] phaseValue               - rising phase value time of generation clock, in nanoseconds.
* @param[in] seconds                  - seconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] nanoSeconds              - nanoseconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..2^28-1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
static GT_STATUS internal_cpssDxChPtpManagerSlavePpsActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  GT_U32                                phaseValue,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    CPSS_DXCH_PTP_TOD_COUNT_STC todValue;
    GT_U32 numIterations=1;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    cpssOsMemSet(&todValue, 0, sizeof(todValue));

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* enable PPS input */
        rc = cpssDxChPtpTaiClockReceptionModeSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_CLOCK_RECEPTION_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* trigger a signal at a specific nanosecond following a whole second */
        rc = cpssDxChPtpTaiClockReceptionEdgeSet(devNum, &taiId, phaseValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set PPS frequency */
        rc = cpssDxChPtpTaiClockCycleSet(devNum,&taiId,seconds,nanoSeconds);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* master-to-slave delay d, and master PPS triggers at phase t, the slave PPS
           is triggered at phase t+d, where t+d is todValuePtr->nanoSeconds */
        todValue.nanoSeconds = nanoSeconds; /* phase adjustment value */
        rc = cpssDxChPtpTaiTodSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,&todValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* prepare the trigger update information */
        rc = cpssDxChPtpTodCounterFunctionSet(devNum,0 /* not relevant */,&taiId,CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* trigger update*/
        rc = cpssDxChPtpTodCounterFunctionTriggerSet(devNum,0 /* not relevant */,&taiId);
        if (rc != GT_OK)
        {
            return rc;
        }

    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerSlavePpsActivate function
* @endinternal
*
* @brief   Activating the TAI as Slave PPS.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] phaseValue               - rising phase value time of generation clock, in nanoseconds.
* @param[in] seconds                  - seconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] nanoSeconds              - nanoseconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..2^28-1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
GT_STATUS cpssDxChPtpManagerSlavePpsActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  GT_U32                                phaseValue,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerSlavePpsActivate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, phaseValue, seconds, nanoSeconds));

    rc = internal_cpssDxChPtpManagerSlavePpsActivate(devNum, taiNumber, phaseValue, seconds, nanoSeconds);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, phaseValue, seconds, nanoSeconds));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortAllocateTai function
* @endinternal
*
* @brief   Allocating Port Interface to TAI.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
* @param[in] egressPortBindMode       - determines the mode in which egress port is bound to TAI,
*                                       either static or dynamic per packet.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E, (dynamic bind, per ingress port of a specific packet)
*                                           CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E)  (static bind per taiNumber param)
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
*                                       NOTE: for high accuracy use CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPortAllocateTai
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT               taiNumber,
    IN  CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT    egressPortBindMode
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    CPSS_DXCH_PTP_TSU_CONTROL_STC control;
    GT_U32 numIterations=1;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        /* dummy; not relevant for Ironman */
        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        control.unitEnable = GT_TRUE;
        control.taiNumber = taiId.taiNumber;

        if (!prvCpssDxChPortSkipCheck(devNum,portNum) && !prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            /* Timestamping unit enable */
            rc = cpssDxChPtpTsuControlSet(devNum, portNum, &control);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E)
            {
                /* Set Egress Timestamp TAI Select */
                rc = cpssDxChPtpTsDelayEgressTimestampTaiSelectSet(devNum,portNum,taiId.taiNumber);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

     if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        return GT_OK;

    /* set the mode in which egress port is bound to TAI */
    rc = cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet(devNum,portNum,egressPortBindMode);

    return rc;
}

/**
* @internal cpssDxChPtpManagerPortAllocateTai function
* @endinternal
*
* @brief   Allocating Port Interface to TAI.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
* @param[in] egressPortBindMode       - determines the mode in which egress port is bound to TAI,
*                                       either static or dynamic per packet.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E, (dynamic bind, per ingress port of a specific packet)
*                                           CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E)  (static bind per taiNumber param)
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
*                                       NOTE: for high accuracy use CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortAllocateTai
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT               taiNumber,
    IN  CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT    egressPortBindMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortAllocateTai);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, taiNumber, egressPortBindMode));

    rc = internal_cpssDxChPtpManagerPortAllocateTai(devNum, portNum, taiNumber, egressPortBindMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, taiNumber, egressPortBindMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerCaptureTodValueSet function
* @endinternal
*
* @brief   capture the current TOD values.
*
* @note    NOTE: Call API cpssDxChPtpManagerTodValueGet to retreive captured TOD values.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerCaptureTodValueSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT                 taiNumber
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    GT_U32 numIterations=1;
    GT_U32 taiSelectBmp=0;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
            /* save tai triggering selection */
            rc = cpssDxChPtpTaiTodCounterFunctionGroupTriggerGet(devNum,&taiSelectBmp);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* select taiNumber for triggering */
            rc = cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet(devNum,(1 << taiNumber));
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* prepare the trigger caputre TOD counter value */
        rc = cpssDxChPtpTodCounterFunctionSet(devNum,0 /* not relevant */,&taiId,CPSS_DXCH_PTP_TOD_COUNTER_FUNC_CAPTURE_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* trigger capture */
        rc = cpssDxChPtpTodCounterFunctionTriggerSet(devNum,0 /* not relevant */,&taiId);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
            /* restore tai triggering selection */
            rc = cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet(devNum,taiSelectBmp);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)


    return GT_OK;
}



/**
* @internal cpssDxChPtpManagerCaptureTodValueSet function
* @endinternal
*
* @brief   capture the current TOD values.
*
* @note    NOTE: Call API cpssDxChPtpManagerTodValueGet to retreive captured TOD values.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerCaptureTodValueSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT                 taiNumber
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerCaptureTodValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber));

    rc = internal_cpssDxChPtpManagerCaptureTodValueSet(devNum, taiNumber);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerTodValueSet function
* @endinternal
*
* @brief   Set a new TOD Value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] taiNumber                 - TAI number.
* @param[in] todValuePtr               - (pointer to) TOD value.
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerTodValueSet
(
    IN GT_U8                                  devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT           taiNumber,
    IN CPSS_DXCH_PTP_TOD_COUNT_STC            *todValuePtr,
    IN GT_BOOL                                capturePrevTodValueEnable
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    GT_U32 numIterations=1;
    CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT clockGenerationMode = CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(todValuePtr);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* disable PTP CLOCK OUT (a.k.a. 1pps) before update TOD second field */
        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_AC5P_TOD_UPDATE_SECONDS_WA_E))
        {
            rc = cpssDxChPtpTaiClockGenerationModeGet(devNum,&taiId,&clockGenerationMode);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (clockGenerationMode != CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E)
            {
                rc = cpssDxChPtpTaiClockGenerationModeSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }

        /* assigns the TOD a new value */
        rc = cpssDxChPtpTaiTodSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,todValuePtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* prepare the trigger update information */
        rc = cpssDxChPtpTodCounterFunctionSet(devNum,0 /* not relevant */,&taiId,CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* enable/disable capturing the TOD value before the update */
        rc = cpssDxChPtpTaiCapturePreviousModeEnableSet(devNum,&taiId,capturePrevTodValueEnable);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* allow the new capture to override entry 0 */
        rc = cpssDxChPtpTaiCaptureOverrideEnableSet(devNum,&taiId,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* trigger update*/
        rc = cpssDxChPtpTodCounterFunctionTriggerSet(devNum,0 /* not relevant */,&taiId);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_AC5P_TOD_UPDATE_SECONDS_WA_E))
        {
            if (clockGenerationMode != CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E)
            {
                rc = cpssDxChPtpTaiClockGenerationModeSet(devNum,&taiId,clockGenerationMode);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerTodValueSet function
* @endinternal
*
* @brief   Set a new TOD Value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] taiNumber                 - TAI number.
* @param[in] todValuePtr               - (pointer to) TOD value.
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerTodValueSet
(
    IN GT_U8                                  devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT           taiNumber,
    IN CPSS_DXCH_PTP_TOD_COUNT_STC            *todValuePtr,
    IN GT_BOOL                                capturePrevTodValueEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerTodValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, todValuePtr, capturePrevTodValueEnable));

    rc = internal_cpssDxChPtpManagerTodValueSet(devNum, taiNumber, todValuePtr, capturePrevTodValueEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, todValuePtr, capturePrevTodValueEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerTodValueFrequencyUpdate function
* @endinternal
*
* @brief   Adjusts the fractional nanosecond drift value which is added to the TOD stepSet logic.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] taiNumber                 - TAI number.
* @param[in] updateValue               - fractional nanosecond drift value.
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerTodValueFrequencyUpdate
(
    IN GT_U8                                  devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT           taiNumber,
    IN GT_U32                                 updateValue,
    IN GT_BOOL                                capturePrevTodValueEnable
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    CPSS_DXCH_PTP_TOD_COUNT_STC todValue;
    GT_U32 numIterations=1;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    cpssOsMemSet(&todValue, 0, sizeof(todValue));

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* adjusts the "fractional nanosecond drift" value which is added to the TOD step logic */
        todValue.fracNanoSeconds = updateValue;
        rc = cpssDxChPtpTaiTodSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,&todValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* prepare the trigger frequency update information */
        rc = cpssDxChPtpTodCounterFunctionSet(devNum,0 /* not relevant */,&taiId,CPSS_DXCH_PTP_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* enable/disable capturing the TOD value before the update */
        rc = cpssDxChPtpTaiCapturePreviousModeEnableSet(devNum,&taiId,capturePrevTodValueEnable);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* allow the new capture to override entry 0 */
        rc = cpssDxChPtpTaiCaptureOverrideEnableSet(devNum,&taiId,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerTodValueFrequencyUpdate function
* @endinternal
*
* @brief   Adjusts the fractional nanosecond drift value which is added to the TOD stepSet logic.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] taiNumber                 - TAI number.
* @param[in] updateValue               - fractional nanosecond drift value.
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerTodValueFrequencyUpdate
(
    IN GT_U8                                  devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT           taiNumber,
    IN GT_U32                                 updateValue,
    IN GT_BOOL                                capturePrevTodValueEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerTodValueFrequencyUpdate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, updateValue, capturePrevTodValueEnable));

    rc = internal_cpssDxChPtpManagerTodValueFrequencyUpdate(devNum, taiNumber, updateValue, capturePrevTodValueEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, updateValue, capturePrevTodValueEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerTodValueUpdate function
* @endinternal
*
* @brief  update the current TOD value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] updateValuePtr           - (pointer to) increment/decrement value.
* @param[in] function                 - TOD counter functions.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E,
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_DECREMENT_E,
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E,
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E)
* @param[in] gracefulStep             - Graceful Step in nanoseconds. Applicable if "function" ==
*                                       CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E or CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerTodValueUpdate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC           *updateValuePtr,
    IN  CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT    function,
    IN  GT_U32                                gracefulStep,
    IN  GT_BOOL                               capturePrevTodValueEnable
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    GT_U32 numIterations=1;
    CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT clockGenerationMode = CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(updateValuePtr);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    switch (function)
    {
    case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E:
    case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_DECREMENT_E:
    case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E:
    case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E:
        /* do nothing */
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* disable PTP CLOCK OUT (a.k.a. 1pps) before update TOD second field */
        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_AC5P_TOD_UPDATE_SECONDS_WA_E))
        {
            rc = cpssDxChPtpTaiClockGenerationModeGet(devNum,&taiId,&clockGenerationMode);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (clockGenerationMode != CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E)
            {
                rc = cpssDxChPtpTaiClockGenerationModeSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }

        /* increment/decrement the TOD Counter by a specified value */
        rc = cpssDxChPtpTaiTodSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,updateValuePtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* prepare the trigger function information */
        rc = cpssDxChPtpTodCounterFunctionSet(devNum,0 /* not relevant */,&taiId,function);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* enable/disable capturing the TOD value before the update */
        rc = cpssDxChPtpTaiCapturePreviousModeEnableSet(devNum,&taiId,capturePrevTodValueEnable);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* allow the new capture to override entry 0 */
        rc = cpssDxChPtpTaiCaptureOverrideEnableSet(devNum,&taiId,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        if ((function == CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E) || (function == CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E))
        {
            /* set the graceful step */
            rc = cpssDxChPtpTaiGracefulStepSet(devNum,&taiId,gracefulStep);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_AC5P_TOD_UPDATE_SECONDS_WA_E))
        {
            if (clockGenerationMode != CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E)
            {
                rc = cpssDxChPtpTaiClockGenerationModeSet(devNum,&taiId,clockGenerationMode);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerTodValueUpdate function
* @endinternal
*
* @brief  update the current TOD value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] updateValuePtr           - (pointer to) increment/decrement value.
* @param[in] function                 - TOD counter functions.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E,
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_DECREMENT_E,
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E,
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E)
* @param[in] gracefulStep             - Graceful Step in nanoseconds. Applicable if "function" ==
*                                       CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E or CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerTodValueUpdate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC           *updateValuePtr,
    IN  CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT    function,
    IN  GT_U32                                gracefulStep,
    IN  GT_BOOL                               capturePrevTodValueEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerTodValueUpdate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, updateValuePtr, function, gracefulStep, capturePrevTodValueEnable));

    rc = internal_cpssDxChPtpManagerTodValueUpdate(devNum, taiNumber, updateValuePtr, function, gracefulStep, capturePrevTodValueEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, updateValuePtr, function, gracefulStep, capturePrevTodValueEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerTodValueGet function
* @endinternal
*
* @brief  Get all valid TOD values that were captured.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_NUMBER_0_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_1_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_2_E, (APPLICABLE DEVICES: Ironman)
*                                           CPSS_DXCH_PTP_TAI_NUMBER_3_E, (APPLICABLE DEVICES: Ironman)
*                                           CPSS_DXCH_PTP_TAI_NUMBER_4_E  (APPLICABLE DEVICES: Ironman))
* @param[out] todValueEntry0Ptr       - (pointer to) captured TOD value of first entry.
* @param[out] todValueEntry1Ptr       - (pointer to) captured TOD value of second entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerTodValueGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry0Ptr,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry1Ptr
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(todValueEntry0Ptr);
    CPSS_NULL_PTR_CHECK_MAC(todValueEntry1Ptr);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    if (taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    taiId.taiNumber = taiNumber;

    /* ensure TOD0 value is captured */
    rc = cpssDxChPtpTaiTodCaptureStatusGet(devNum,&taiId,0,&(todValueEntry0Ptr->todValueIsValid));
    if (rc != GT_OK)
    {
        return rc;
    }

    if (todValueEntry0Ptr->todValueIsValid == GT_TRUE)
    {
        /* get captured TOD0 value */
        rc = cpssDxChPtpTaiTodGet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E,&(todValueEntry0Ptr->todValue));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* ensure TOD1 value is captured */
    rc = cpssDxChPtpTaiTodCaptureStatusGet(devNum,&taiId,1,&(todValueEntry1Ptr->todValueIsValid));
    if (rc != GT_OK)
    {
        return rc;
    }

    if (todValueEntry1Ptr->todValueIsValid == GT_TRUE)
    {
        /* get captured TOD1 value */
        rc = cpssDxChPtpTaiTodGet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE1_E,&(todValueEntry1Ptr->todValue));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerTodValueGet function
* @endinternal
*
* @brief  Get all valid TOD values that were captured.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_NUMBER_0_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_1_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_2_E, (APPLICABLE DEVICES: Ironman)
*                                           CPSS_DXCH_PTP_TAI_NUMBER_3_E, (APPLICABLE DEVICES: Ironman)
*                                           CPSS_DXCH_PTP_TAI_NUMBER_4_E  (APPLICABLE DEVICES: Ironman))
* @param[out] todValueEntry0Ptr       - (pointer to) captured TOD value of first entry.
* @param[out] todValueEntry1Ptr       - (pointer to) captured TOD value of second entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerTodValueGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry0Ptr,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry1Ptr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerTodValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, todValueEntry0Ptr, todValueEntry1Ptr));

    rc = internal_cpssDxChPtpManagerTodValueGet(devNum, taiNumber, todValueEntry0Ptr, todValueEntry1Ptr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, todValueEntry0Ptr, todValueEntry1Ptr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerTriggerGenerate function
* @endinternal
*
* @brief   Generates a trigger on the PTP_PULSE_OUT interface at a specified time.
*
* @note    NOTE: This functionality should only be triggered on the master device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] extPulseWidth            - external pulse width in nanoseconds of generated output trigger.
*                                       (APPLICABLE RANGES: 0..2^28-1.)
* @param[in] triggerTimePtr           - (pointer to) trigger generation time.
* @param[in] maskEnable               - enable/disable masking of trigger generation time.
* @param[in] maskValuePtr             - (pointer to) trigger generation time mask.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerTriggerGenerate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  GT_U32                                  extPulseWidth,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC             *triggerTimePtr,
    IN  GT_BOOL                                 maskEnable,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC             *maskValuePtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    GT_U32 numIterations=1;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(triggerTimePtr);
    CPSS_NULL_PTR_CHECK_MAC(maskValuePtr);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* define the pulse width */
        rc = cpssDxChPtpTaiExternalPulseWidthSet(devNum,&taiId,extPulseWidth);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* setting trigger generation time */
        rc = cpssDxChPtpTaiTodSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,triggerTimePtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (maskEnable == GT_TRUE)
        {
            /* setting trigger generation time mask */
            rc = cpssDxChPtpTaiTodSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,maskValuePtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* enable the trigger */
        rc = cpssDxChPtpTaiOutputTriggerEnableSet(devNum,&taiId,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* trigger update*/
        rc = cpssDxChPtpTodCounterFunctionTriggerSet(devNum,0 /* not relevant */,&taiId);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerTriggerGenerate function
* @endinternal
*
* @brief   Generates a trigger on the PTP_PULSE_OUT interface at a specified time.
*
* @note    NOTE: This functionality should only be triggered on the master device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] extPulseWidth            - external pulse width in nanoseconds of generated output trigger.
*                                       (APPLICABLE RANGES: 0..2^28-1.)
* @param[in] triggerTimePtr           - (pointer to) trigger generation time.
* @param[in] maskEnable               - enable/disable masking of trigger generation time.
* @param[in] maskValuePtr             - (pointer to) trigger generation time mask.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerTriggerGenerate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  GT_U32                                  extPulseWidth,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC             *triggerTimePtr,
    IN  GT_BOOL                                 maskEnable,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC             *maskValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerTriggerGenerate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, extPulseWidth, triggerTimePtr, maskEnable, maskValuePtr));

    rc = internal_cpssDxChPtpManagerTriggerGenerate(devNum, taiNumber, extPulseWidth, triggerTimePtr, maskEnable, maskValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, extPulseWidth, triggerTimePtr, maskEnable, maskValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerGlobalPtpDomainSet function
* @endinternal
*
* @brief   Configures the global settings for PTP Domain.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] domainConfPtr            - (pointer to) PTP domain configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerGlobalPtpDomainSet
(
    IN GT_U8                                           devNum,
    IN CPSS_DXCH_PTP_MANAGER_DOMAIN_STC                *domainConfPtr
)
{
    GT_STATUS rc;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(domainConfPtr);
    switch (domainConfPtr->domainMode)
    {
    case CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E:
    case CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E:
        /* do nothing */
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* define the PTP protocol (v1 or v2) per domain index */
    rc = cpssDxChPtpDomainModeSet(devNum,domainConfPtr->domainIndex,domainConfPtr->domainMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (domainConfPtr->domainMode == CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E)
    {
        /* Set PTP V1 domain Id */
        rc = cpssDxChPtpDomainV1IdSet(devNum,domainConfPtr->domainIndex,domainConfPtr->v1DomainIdArr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (domainConfPtr->domainMode == CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E)
    {
        /* Set PTP V2 domain Id */
        rc = cpssDxChPtpDomainV2IdSet(devNum,domainConfPtr->domainIndex,domainConfPtr->v2DomainId);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerGlobalPtpDomainSet function
* @endinternal
*
* @brief   Configures the global settings for PTP Domain.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] domainConfPtr            - (pointer to) PTP domain configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerGlobalPtpDomainSet
(
    IN GT_U8                                           devNum,
    IN CPSS_DXCH_PTP_MANAGER_DOMAIN_STC                *domainConfPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerGlobalPtpDomainSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, domainConfPtr));

    rc = internal_cpssDxChPtpManagerGlobalPtpDomainSet(devNum, domainConfPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, domainConfPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortCommandAssignment function
* @endinternal
*
* @brief   Assign packet command per{ingress port, domain index, message type}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - ingress port number
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - PTP Message Type to trigger the selected Command.
* @param[in] command                  - command assigned  to the packet.
*                                      (APPLICABLE VALUES:
*                                         CPSS_PACKET_CMD_FORWARD_E,
*                                         CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                         CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                         CPSS_PACKET_CMD_DROP_HARD_E,
*                                         CPSS_PACKET_CMD_DROP_SOFT_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPortCommandAssignment
(
    IN GT_U8                                    devNum,
    IN GT_PHYSICAL_PORT_NUM                     portNum,
    IN GT_U32                                   domainIndex,
    IN CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT   messageType,
    IN CPSS_PACKET_CMD_ENT                      command
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);


    return cpssDxChPtpMessageTypeCmdSet(devNum,portNum,domainIndex,messageType,command);
}

/**
* @internal cpssDxChPtpManagerPortCommandAssignment function
* @endinternal
*
* @brief   Assign packet command per{ingress port, domain index, message type}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - ingress port number
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - PTP Message Type to trigger the selected Command.
* @param[in] command                  - command assigned  to the packet.
*                                      (APPLICABLE VALUES:
*                                         CPSS_PACKET_CMD_FORWARD_E,
*                                         CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                         CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                         CPSS_PACKET_CMD_DROP_HARD_E,
*                                         CPSS_PACKET_CMD_DROP_SOFT_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortCommandAssignment
(
    IN GT_U8                                    devNum,
    IN GT_PHYSICAL_PORT_NUM                     portNum,
    IN GT_U32                                   domainIndex,
    IN CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT   messageType,
    IN CPSS_PACKET_CMD_ENT                      command
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortCommandAssignment);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, domainIndex, messageType, command));

    rc = internal_cpssDxChPtpManagerPortCommandAssignment(devNum, portNum, domainIndex, messageType, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, domainIndex, messageType, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortPacketActionsConfig function
* @endinternal
*
* @brief  Configure action to be performed on PTP packets per{egress port, domain index, message type}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
* @param[in] domainIndex              - domain index.
*                                       (APPLICABLE RANGES: 0..4.)
* @param[in] domainEntryPtr           - (pointer to) domain table entry.
* @param[in] messageType              - PTP message type
* @param[in] action                   - PTP action on the packet
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TS_ACTION_NONE_E
*                                           CPSS_DXCH_PTP_TS_ACTION_DROP_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E
*                                           CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E,
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPortPacketActionsConfig
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  GT_U32                                  domainIndex,
    IN  CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   *domainEntryPtr,
    IN  CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT  messageType,
    IN  CPSS_DXCH_PTP_TS_ACTION_ENT             action
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    CPSS_DXCH_PTP_TSU_CONTROL_STC control;
    CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC entry;
    GT_U32 numIterations=1;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        /* dummy; not relevant for Ironman */
        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        control.unitEnable = GT_TRUE;
        control.taiNumber = taiId.taiNumber;

        if (!prvCpssDxChPortSkipCheck(devNum,portNum) && !prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            /* Timestamping unit enable */
            rc = cpssDxChPtpTsuControlSet(devNum, portNum, &control);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* set the PTP domain table per egress physical port and per domain index */
        rc = cpssDxChPtpEgressDomainTableSet(devNum,portNum,domainIndex,domainEntryPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPtpTsLocalActionTableGet(devNum,portNum,domainIndex,messageType,&entry);
        if (rc != GT_OK)
        {
            return rc;
        }

        entry.tsAction = action;
        if (entry.tsAction != CPSS_DXCH_PTP_TS_ACTION_NONE_E)
        {
            entry.ingrLinkDelayEnable = GT_TRUE;
        }
        else
        {
            entry.ingrLinkDelayEnable = GT_FALSE;
        }

        /* set action per egress port, domain index and message type */
        rc = cpssDxChPtpTsLocalActionTableSet(devNum,portNum,domainIndex,messageType,&entry);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerPortPacketActionsConfig function
* @endinternal
*
* @brief  Configure action to be performed on PTP packets per{egress port, domain index, message type}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
* @param[in] domainIndex              - domain index.
*                                       (APPLICABLE RANGES: 0..4.)
* @param[in] domainEntryPtr           - (pointer to) domain table entry.
* @param[in] messageType              - PTP message type
* @param[in] action                   - PTP action on the packet
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TS_ACTION_NONE_E
*                                           CPSS_DXCH_PTP_TS_ACTION_DROP_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E
*                                           CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E,
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortPacketActionsConfig
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  GT_U32                                  domainIndex,
    IN  CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   *domainEntryPtr,
    IN  CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT  messageType,
    IN  CPSS_DXCH_PTP_TS_ACTION_ENT             action
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortPacketActionsConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, taiNumber, domainIndex, domainEntryPtr, messageType, action));

    rc = internal_cpssDxChPtpManagerPortPacketActionsConfig(devNum, portNum, taiNumber, domainIndex, domainEntryPtr, messageType, action);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, taiNumber, domainIndex, domainEntryPtr, messageType, action));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortTsTagConfig function
* @endinternal
*
* @brief    Controlling Timestamp tag for Ingress identification and Egress Addition.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsTagPortCfgPtr          - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPortTsTagConfig
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC   *tsTagPortCfgPtr
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(tsTagPortCfgPtr);

    /* Set Timestamp Port Configuration */
    return cpssDxChPtpTsTagPortCfgSet(devNum,portNum,tsTagPortCfgPtr);
}

/**
* @internal cpssDxChPtpManagerPortTsTagConfig function
* @endinternal
*
* @brief    Controlling Timestamp tag for Ingress identification and Egress Addition.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsTagPortCfgPtr          - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortTsTagConfig
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC   *tsTagPortCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortTsTagConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tsTagPortCfgPtr));

    rc = internal_cpssDxChPtpManagerPortTsTagConfig(devNum, portNum, tsTagPortCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tsTagPortCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerTsQueueConfig function
* @endinternal
*
* @brief  Configure queue number per message type.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number
* @param[in] messageType            - PTP Message Type
* @param[in] queueNum               - Queue number; 0 - queue0, 1 - queue1.
*                                     (APPLICABLE RANGES: 0..1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
static GT_STATUS internal_cpssDxChPtpManagerTsQueueConfig
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT      messageType,
    IN  GT_U32                                      queueNum
)
{
    GT_U32 regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    if (messageType >= CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if (queueNum >= BIT_1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).timestampQueueMsgType;

    /* Configure queue number per message type */
    return prvCpssHwPpSetRegField(devNum, regAddr, messageType /*offset*/, 1, queueNum);

}

/**
* @internal cpssDxChPtpManagerTsQueueConfig function
* @endinternal
*
* @brief  Configure queue number per message type.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number
* @param[in] messageType            - PTP Message Type
* @param[in] queueNum               - Queue number; 0 - queue0, 1 - queue1.
*                                     (APPLICABLE RANGES: 0..1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
GT_STATUS cpssDxChPtpManagerTsQueueConfig
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT      messageType,
    IN  GT_U32                                      queueNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerTsQueueConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, messageType, queueNum));

    rc = internal_cpssDxChPtpManagerTsQueueConfig(devNum, messageType, queueNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, messageType, queueNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Global Queue Entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - queue Number
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
static GT_STATUS internal_cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC    *entryPtr
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    if (queueNum >= BIT_1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Read Egress Timestamp Global Queue Entry */
    return cpssDxChPtpTsEgressTimestampQueueEntryRead(devNum,queueNum,entryPtr);

}

/**
* @internal cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Global Queue Entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - queue Number
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
GT_STATUS cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC    *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueNum, entryPtr));

    rc = internal_cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead(devNum, queueNum, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueNum, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerEgressTimestampPortQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Queue Entry per port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] queueNum                 - queue number.
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
static GT_STATUS internal_cpssDxChPtpManagerEgressTimestampPortQueueEntryRead
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    IN  GT_U32                                                      queueNum,
    OUT CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC      *entryPtr
)
{
    CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC  entry;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    if (queueNum >= BIT_1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Read Egress Timestamp Queue Entry per port */
    rc = cpssDxChPtpTsuTxTimestampQueueRead(devNum,portNum,queueNum,&entry);
    if (rc != GT_OK)
    {
        return rc;
    }

    entryPtr->entryValid = entry.entryValid;
    entryPtr->queueEntryId = entry.queueEntryId;
    entryPtr->taiSelect = entry.taiSelect;
    entryPtr->todUpdateFlag = entry.todUpdateFlag;
    entryPtr->seconds = (entry.timestamp >> 30);
    entryPtr->nanoSeconds = (entry.timestamp & 0x3FFFFFFF);

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerEgressTimestampPortQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Queue Entry per port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] queueNum                 - queue number.
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
GT_STATUS cpssDxChPtpManagerEgressTimestampPortQueueEntryRead
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    IN  GT_U32                                                      queueNum,
    OUT CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC      *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerEgressTimestampPortQueueEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, queueNum, entryPtr));

    rc = internal_cpssDxChPtpManagerEgressTimestampPortQueueEntryRead(devNum, portNum, queueNum, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, queueNum, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead function
* @endinternal
*
* @brief   Read Ingress Timestamp Global Queue Entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - queue Number
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Ingress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
static GT_STATUS internal_cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   *entryPtr
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    if (queueNum >= BIT_1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Read Ingress Timestamp Global Queue Entry */
    return cpssDxChPtpTsIngressTimestampQueueEntryRead(devNum,queueNum,entryPtr);

}

/**
* @internal cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead function
* @endinternal
*
* @brief   Read Ingress Timestamp Global Queue Entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - queue Number
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Ingress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
GT_STATUS cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueNum, entryPtr));

    rc = internal_cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead(devNum, queueNum, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueNum, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortTimestampingModeConfig function
* @endinternal
*
* @brief   Configure Enhanced Timestamping Mode per Egress port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsMode                   - enhance timestamping mode.
* @param[in] operationMode            - operation mode.
*                                       (APPLICABLE VALUES:
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_IN_SWITCH_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_IN_MAC_E
*
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_PHY_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_MARVELL_FORMAT_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_STANDARD_FORMAT_E
*
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_IN_SWITCH_E
*                                           (APPLICABLE DEVICES: AC5P; Harrier)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPortTimestampingModeConfig
(
    IN  GT_U8                                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                            portNum,
    IN  CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT    tsMode,
    IN  CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT        operationMode
)
{
    GT_STATUS rc;
    GT_U32  regAddr,regAddr1;
    GT_U32  tsuIndex = 0;
    GT_U32  tsuLocalChannel = 0;
    GT_U32  domainProfile;
    GT_U32  messageType;
    CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC entry;
    GT_U32  portMacNum;
    GT_U32  amdOffset;
    GT_BOOL egressTsCorrectionEnable;
    GT_BOOL dispatchEnable;
    GT_BOOL fsuEnable;
    GT_BOOL amdEnable;
    GT_BOOL tsPchInsertionEnable;
    GT_BOOL egressTsEncryptedEnable;
    GT_U32  value,data;
    CPSS_PORT_SPEED_ENT speed;
    GT_U32     unitIndex;
    GT_U32     channel;
    GT_U32     isPca;
    GT_U32  amdCntTypeSel,amdCntTypeSelOffset;
    GT_BOOL egressTsAmResilienceEnable;

    /* CPSS_TBD_BOOKMARK_IRONMAN */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    operationMode = CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_E; /* avoid compilation warnings CPSS_TBD_BOOKMARK_IRONMAN */
    cpssOsPrintf("operationMode %d\n",operationMode);

    rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(devNum, portNum, &tsuIndex, &tsuLocalChannel);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    switch (tsMode)
    {
    case  CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E:
        /* This mode should be used for high accuracy timestamping on single lane ports, without RSFEC and
           PTP on clear (not encrypted).*/

        egressTsCorrectionEnable = GT_TRUE;
        dispatchEnable = GT_FALSE;
        fsuEnable = GT_FALSE;
        amdEnable = GT_FALSE;
        tsPchInsertionEnable = GT_FALSE;
        egressTsEncryptedEnable = GT_FALSE;
        break;

    case CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E:
        /* When working in this mode, an external copper PHY is connected between the switch port (Host)
           and the media/copper side.
           For increased timestamping accuracy, timestamping should be done in the PHY itself (avoiding the
           PDV in the PHY itself) both on the ingress path and on the egress path. The communication
           between the switch and the PHY for passing the timestamp information is done via a dedicated 8B
           header. This header is not a part of the payload, but rather it replaces the preamble of the frame. */

        egressTsCorrectionEnable = GT_FALSE;
        dispatchEnable = GT_FALSE;
        fsuEnable = GT_FALSE;
        amdEnable = GT_FALSE;
        tsPchInsertionEnable = GT_TRUE;
        egressTsEncryptedEnable = GT_FALSE;
        break;

    case CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E:
        /* This mode should be used when MACSEC encryption is enabled for the PTP flow on the egress port.
           When encryption is incorporated, the frame cannot be modified post the MACSEC block.
           The timestamping is thus done at an earlier stage by utilizing a dedicated high accuracy engine. */

        egressTsCorrectionEnable = GT_FALSE;
        dispatchEnable = GT_TRUE;
        /* Timestamp to the Future is not supported in AC5X */
        fsuEnable = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ? GT_FALSE : GT_TRUE;
        amdEnable = GT_FALSE;
        tsPchInsertionEnable = GT_FALSE;
        egressTsEncryptedEnable = GT_TRUE;

        /* NOTE: fsu_offset and Encrypted Egress pipe delay are set by port manager and are relevant for this mode only (prvCpssDxChPortPtpDelayParamsSetWrapper) */
        break;

    case CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_AM_E:
        /* This mode should be used for high accuracy timestamping on port modes in which "Alignment Markers" are present:
           - Multi-lane ports, e.g. 100G-R4, 50G-R2, 40G-R4.
           - RSFEC - Any port which runs with RSEFEC enabled.
           AM timestamping mode should be used only for non-encrypted PTP traffic. */

        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        egressTsCorrectionEnable = GT_TRUE;
        dispatchEnable = GT_TRUE;
        fsuEnable = GT_FALSE;
        amdEnable = GT_TRUE;
        tsPchInsertionEnable = GT_FALSE;
        egressTsEncryptedEnable = GT_FALSE;

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* enable/disable  Egress TS encrypted */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].channelTsPrecision[tsuLocalChannel];
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 1, 1, egressTsEncryptedEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* enable/disable egress TS Correction */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].channelTsPrecision[tsuLocalChannel];
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, egressTsCorrectionEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* enable/disable DISPATCH in TsLocalActionTable */
    for (domainProfile=0; domainProfile<=4; domainProfile++)
    {
        for (messageType=0; messageType<CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_LAST_E; messageType++)
        {
            rc = cpssDxChPtpTsLocalActionTableGet(devNum,portNum,domainProfile,messageType,&entry);
            if (rc != GT_OK)
            {
                return rc;
            }

            entry.packetDispatchingEnable = dispatchEnable;

            if (tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E)
            {
                entry.tsAction = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E;
            }

            rc = cpssDxChPtpTsLocalActionTableSet(devNum,portNum,domainProfile,messageType,&entry);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portTsdControl1;
        amdOffset = 19;
    }
    else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).portTsdControl0;
        amdOffset = 1;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portTsdControl0;
        amdOffset = 1;
    }

    /* enable/disable Future Timestamp feature (bit 0) */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, fsuEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* configure registers for 200G/400G ports */
    regAddr1 = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).segPortTsdControl0;

    if (regAddr1 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr1, 0, 1, fsuEnable);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* enable/disable Alignment Marker Dispatcher feature (bit 1/19) */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, amdOffset, 1, amdEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (regAddr1 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr1, 1, 1, amdEnable);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* enable/disable Egress TS PCH Insertion  */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].channelTsPrecision[tsuLocalChannel];
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 3, 1, tsPchInsertionEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
    {
        if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portTsdControl1;
            amdCntTypeSelOffset = 20;
        }
        else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).portTsdControl0;
            amdCntTypeSelOffset = 24;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portTsdControl0;
            amdCntTypeSelOffset = 24;
        }

        if (tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_AM_E)
        {

            rc = cpssDxChPortSpeedGet(devNum,portNum,&speed);
            if (rc != GT_OK)
            {
                return rc;
            }

            amdCntTypeSel = (speed == CPSS_PORT_SPEED_100G_E) ? 1 : 0;
            egressTsAmResilienceEnable = GT_TRUE;
        }
        else
        {
            amdCntTypeSel = 0;
            egressTsAmResilienceEnable = GT_FALSE;
        }

        /* set AM Counter Type selector, according to the Port's speed: 100G port should be configured to 1, other speeds to 0. */
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, amdCntTypeSelOffset, 1, amdCntTypeSel);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (regAddr1 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr1, 24, 1, amdCntTypeSel);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* enable/disable Egress TS AM Resilience; When this bit is enabled, channel %n egress port MAC layer is resilient to alignment markers insertion,
           thus higher precision is achieved. */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].channelTsPrecision[tsuLocalChannel];
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 2, 1, egressTsAmResilienceEnable);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
    {
        if ((tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E) && (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) != PRV_CPSS_PORT_MTI_USX_E))
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesTxChannelAndIsPcaFromPortNum(devNum, portNum, &unitIndex, &channel, &isPca);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChPortSpeedGet(devNum,portNum,&speed);
            if (rc != GT_OK)
            {
                return rc;
            }

            switch (speed)
            {

            case CPSS_PORT_SPEED_10000_E:
                value = 16;
                break;
            case CPSS_PORT_SPEED_25000_E:
                value = 32;
                break;
            case CPSS_PORT_SPEED_50000_E:
                value = 64;
                break;
            case CPSS_PORT_SPEED_100G_E:
            case CPSS_PORT_SPEED_102G_E:
            case CPSS_PORT_SPEED_106G_E:
                value = 128;
                break;
            default:
                value = 0; /* default */
            }

            /* Ch %n Wait Slots: This field specifies a time (measured in time slots of the given channel) necessary to ensure that the entire Tx pipe
               (starting from SFF in PCA Cluster and until TXF in EPI Cluster) becomes empty after traffic (of the given channel) was stopped in the SFF. */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_SFF[unitIndex].channelControlConfig[channel];
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 8, value);
            if (rc != GT_OK)
            {
                return rc;
            }

            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portTsdControl0;

            /* minimal_tx_stop_toggle: De-bouncing protection to the TX_STOP signal; small range to reduce jitter */
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 25, 5, 10);
            if (rc != GT_OK)
            {
                return rc;
            }

            switch (speed)
            {

            case CPSS_PORT_SPEED_10000_E:
                value = 136;
                break;
            case CPSS_PORT_SPEED_25000_E:
                value = 54;
                break;
            case CPSS_PORT_SPEED_100G_E:
            case CPSS_PORT_SPEED_102G_E:
            case CPSS_PORT_SPEED_106G_E:
                value = 10;
                break;
            default:
                value = 16; /* default */
            }

            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portTsdControl2;

            /* bits 0..15 - minimal_empty_for_stop_tx: Minimal TXFIFO empty cycles before assertion of stop tx*/
            data = value;
            /* bit 16 - fsu_preemption_hold_en: Ability to En/Dis impact over p.MAC HOLD signal; stop transmission in mii while PTP is in */
            data |= 0x10000;
            /* bits 17..20 - hold_kept_active: Number of CCs hold signal is kept asserted after exp_stop_tx has de-asserted. Must be above 'd2 */
            data |= 0x1E0000;

            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 21, data);
            if (rc != GT_OK)
            {
                return rc;
            }

            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portPmacControl;

            /* tx_hold_req_from_hw: tx_hold_req controlled by HW; otherwise preemption_hold is not asserted */
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 29, 1, 1);
            if (rc != GT_OK)
            {
                return rc;
            }

            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).tsuControl2;

            /* bit 0 - c_rx_mode_ow: When 1, c_rx_mode_ow value is taken from RF and not from internal logic; bypass TSX for RX to avoid jumps in TOD when matching in TSD */
            /* bit 1 - c_tx_mode_ow: When 1, c_tx_mode_ow value is taken from RF and not from internal logic; bypass TSX for TX to avoid jumps in TOD when matching in TSD */
            /* bit 7 - c_modulo_tx_ow: When 1, c_modulo_tx value is taken from RF and not from internal logic; TSX bypass mode */
            rc = prvCpssDrvHwPpWriteRegBitMask(devNum,regAddr,0x83,0x83);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) != PRV_CPSS_PORT_MTI_USX_E)
            {
                rc = prvCpssDxChTxPortSpeedPizzaResourcesTxChannelAndIsPcaFromPortNum(devNum, portNum, &unitIndex, &channel, &isPca);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /* Ch %n Wait Slots: This field specifies a time (measured in time slots of the given channel) necessary to ensure that the entire Tx pipe
                   (starting from SFF in PCA Cluster and until TXF in EPI Cluster) becomes empty after traffic (of the given channel) was stopped in the SFF. */
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_SFF[unitIndex].channelControlConfig[channel];
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 8, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }

                regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portTsdControl0;

                /* minimal_tx_stop_toggle: De-bouncing protection to the TX_STOP signal; small range to reduce jitter */
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 25, 5, 16);
                if (rc != GT_OK)
                {
                    return rc;
                }

                regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portTsdControl2;

                /* bits 17..20 - hold_kept_active: Number of CCs hold signal is kept asserted after exp_stop_tx has de-asserted. Must be above 'd2 */
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 21, 0x60000);
                if (rc != GT_OK)
                {
                    return rc;
                }

                regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portPmacControl;

                /* tx_hold_req_from_hw: tx_hold_req controlled by HW; otherwise preemption_hold is not asserted */
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 29, 1, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }

                regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).tsuControl2;

                /* bit 0 - c_rx_mode_ow: When 1, c_rx_mode_ow value is taken from RF and not from internal logic; bypass TSX for RX to avoid jumps in TOD when matching in TSD */
                /* bit 1 - c_tx_mode_ow: When 1, c_tx_mode_ow value is taken from RF and not from internal logic; bypass TSX for TX to avoid jumps in TOD when matching in TSD */
                /* bit 7 - c_modulo_tx_ow: When 1, c_modulo_tx value is taken from RF and not from internal logic; TSX bypass mode */
                rc = prvCpssDrvHwPpWriteRegBitMask(devNum,regAddr,0x83,0);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerPortTimestampingModeConfig function
* @endinternal
*
* @brief   Configure Enhanced
*          cpssDxChPtpTsDelayEgressPipeDelaySetTimestamping Mode
*          per Egress port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsMode                   - enhance timestamping mode.
* @param[in] operationMode            - operation mode.
*                                       (APPLICABLE VALUES:
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_IN_SWITCH_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_IN_MAC_E
*
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_PHY_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_MARVELL_FORMAT_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_STANDARD_FORMAT_E
*
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_IN_SWITCH_E
*                                           (APPLICABLE DEVICES: AC5P; Harrier)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortTimestampingModeConfig
(
    IN  GT_U8                                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                            portNum,
    IN  CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT    tsMode,
    IN  CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT        operationMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortTimestampingModeConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tsMode, operationMode));

    rc = internal_cpssDxChPtpManagerPortTimestampingModeConfig(devNum, portNum, tsMode, operationMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tsMode, operationMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortDelayValuesSet
*           function
* @endinternal
*
* @brief   Set delay values per port {speed,interface,fec}
*          mode.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speed                    - speed
* @param[in] interfaceMode            - interface mode
* @param[in] fecMode                  - forward error correction mode
* @param[in] delayValPtr              - (pointer to) delay values
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPortDelayValuesSet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT                speed,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT       interfaceMode,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT             fecMode,
    IN  CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC          *delayValPtr
)
{

    GT_U32 offset;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(delayValPtr);
    if (speed >= CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (interfaceMode >= CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (fecMode >= CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->ptpMgrInfo.initDone == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    /* configure values into DB */
    offset = portNum*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
             speed*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
             interfaceMode*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E + fecMode;

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].egressPipeDelay = delayValPtr->egressPipeDelay;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].egressPipeDelayEncrypted = delayValPtr->egressPipeDelayEncrypted;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].egressPipeDelayFsuOffset = delayValPtr->egressPipeDelayFsuOffset;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ingressPortDelay = delayValPtr->ingressPortDelay;

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerPortDelayValuesSet function
* @endinternal
*
* @brief   Set delay values per port {speed,interface,fec}
*          mode.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speed                    - speed
* @param[in] interfaceMode            - interface mode
* @param[in] fecMode                  - forward error correction mode
* @param[in] delayValPtr              - (pointer to) delay values
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortDelayValuesSet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT                speed,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT       interfaceMode,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT             fecMode,
    IN  CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC          *delayValPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortDelayValuesSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, speed, interfaceMode, fecMode, delayValPtr));

    rc = internal_cpssDxChPtpManagerPortDelayValuesSet(devNum, portNum, speed, interfaceMode, fecMode, delayValPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, speed, interfaceMode, fecMode, delayValPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortDelayValuesGet
*           function
* @endinternal
*
* @brief   Get delay values per port {speed,interface,fec}
*          mode.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speed                    - speed
* @param[in] interfaceMode            - interface mode
* @param[in] fecMode                  - forward error correction mode
* @param[out] delayValPtr             - (pointer to) delay values
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPortDelayValuesGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT                speed,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT       interfaceMode,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT             fecMode,
    OUT CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC          *delayValPtr
)
{
    GT_U32 offset;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(delayValPtr);
    if (speed >= CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (interfaceMode >= CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (fecMode >= CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->ptpMgrInfo.initDone == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    /* get values from DB */
    offset = portNum*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
             speed*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
             interfaceMode*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E + fecMode;

    delayValPtr->egressPipeDelay = PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].egressPipeDelay;
    delayValPtr->egressPipeDelayEncrypted = PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].egressPipeDelayEncrypted;
    delayValPtr->egressPipeDelayFsuOffset = PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].egressPipeDelayFsuOffset;
    delayValPtr->ingressPortDelay = PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ingressPortDelay;

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerPortDelayValuesGet function
* @endinternal
*
* @brief   Get delay values per port {speed,interface,fec}
*          mode.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speed                    - speed
* @param[in] interfaceMode            - interface mode
* @param[in] fecMode                  - forward error correction mode
* @param[out] delayValPtr             - (pointer to) delay values
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortDelayValuesGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT                speed,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT       interfaceMode,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT             fecMode,
    OUT CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC          *delayValPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortDelayValuesGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, speed, interfaceMode, fecMode, delayValPtr));

    rc = internal_cpssDxChPtpManagerPortDelayValuesGet(devNum, portNum, speed, interfaceMode, fecMode, delayValPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, speed, interfaceMode, fecMode, delayValPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead function
* @endinternal
*
* @brief   Read MAC Egress Timestamp Queue Entry per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[out] entryPtr                - (pointer to) MAC Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
static GT_STATUS internal_cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    OUT CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC      *entryPtr
)
{
    cpssOsPrintf("devNum %d portNum %d entryPtr %d\n",devNum,portNum,*entryPtr); /* avoid compilation warnings CPSS_TBD_BOOKMARK_IRONMAN */

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead function
* @endinternal
*
* @brief   Read MAC Egress Timestamp Queue Entry per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[out] entryPtr                - (pointer to) MAC Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
GT_STATUS cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    OUT CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC      *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, entryPtr));

    rc = internal_cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead(devNum, portNum, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManager8021AsMsgSelectionEnableSet function
* @endinternal
*
* @brief   Set PTP Cumulative Scaled Rate Offset enable/disable per message Type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[in] FollowUpEn                           - Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a FollowUp message "FollowUp information TLV".
* @param[in] SyncEn                               - Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a Sync message "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManager8021AsMsgSelectionEnableSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_BOOL                                     followUpEnable,
    IN  GT_BOOL                                     syncEnable
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    if (taiNumber > CPSS_DXCH_PTP_TAI_NUMBER_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).cumulativeScaleRateTaiReg0[taiNumber];
    value = (BOOL2BIT_MAC(followUpEnable) << 1) | BOOL2BIT_MAC(syncEnable);
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 2, value);

    return rc;
}

/**
* @internal cpssDxChPtpManager8021AsMsgSelectionEnableSet function
* @endinternal
*
* @brief   Set PTP Cumulative Scaled Rate Offset enable/disable per message Type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[in] FollowUpEn                           - Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a FollowUp message "FollowUp information TLV".
* @param[in] SyncEn                               - Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a Sync message "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManager8021AsMsgSelectionEnableSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_BOOL                                     followUpEnable,
    IN  GT_BOOL                                     syncEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManager8021AsMsgSelectionEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, followUpEnable, syncEnable));

    rc = internal_cpssDxChPtpManager8021AsMsgSelectionEnableSet(devNum, taiNumber, followUpEnable, syncEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, followUpEnable, syncEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManager8021AsMsgSelectionEnableGet function
* @endinternal
*
* @brief   Get PTP Cumulative Scaled Rate Offset enable/disable per message Type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[out] followUpEnablePtr                   - (pointer to) Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a FollowUp message "FollowUp information TLV".
* @param[out] syncEnablePtr                       - (pointer to) Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a Sync message "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManager8021AsMsgSelectionEnableGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    OUT GT_BOOL                                     *followUpEnablePtr,
    OUT GT_BOOL                                     *syncEnablePtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(followUpEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(syncEnablePtr);
    if (taiNumber > CPSS_DXCH_PTP_TAI_NUMBER_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).cumulativeScaleRateTaiReg0[taiNumber];
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 2, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *followUpEnablePtr = BIT2BOOL_MAC((value >> 1) & 1);
    *syncEnablePtr = BIT2BOOL_MAC(value & 1);

    return GT_OK;
}

/**
* @internal cpssDxChPtpManager8021AsMsgSelectionEnableGet function
* @endinternal
*
* @brief   Get PTP Cumulative Scaled Rate Offset enable/disable per message Type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[out] followUpEnablePtr                   - (pointer to) Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a FollowUp message "FollowUp information TLV".
* @param[out] syncEnablePtr                       - (pointer to) Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a Sync message "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManager8021AsMsgSelectionEnableGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    OUT GT_BOOL                                     *followUpEnablePtr,
    OUT GT_BOOL                                     *syncEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManager8021AsMsgSelectionEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, followUpEnablePtr, syncEnablePtr));

    rc = internal_cpssDxChPtpManager8021AsMsgSelectionEnableGet(devNum, taiNumber, followUpEnablePtr, syncEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, followUpEnablePtr, syncEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet function
* @endinternal
*
* @brief   Set PTP Cumulative Scaled Rate Offset in the "FollowUp information TLV"
*          either in a Sync or a FollowUp message.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[in] offset                               - value of cumulativeScaledRateOffset to be set in
*                                                   the "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_U32                                      offset
)
{
    GT_STATUS rc;
    GT_U32  regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    if (taiNumber > CPSS_DXCH_PTP_TAI_NUMBER_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).cumulativeScaleRateTaiReg1[taiNumber];
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 32, offset);

    return rc;
}

/**
* @internal cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet function
* @endinternal
*
* @brief   Set PTP Cumulative Scaled Rate Offset in the "FollowUp information TLV"
*          either in a Sync or a FollowUp message.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[in] offset                               - value of cumulativeScaledRateOffset to be set in
*                                                   the "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_U32                                      offset
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, offset));

    rc = internal_cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet(devNum, taiNumber, offset);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, offset));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet function
* @endinternal
*
* @brief   Get PTP Cumulative Scaled Rate Offset in the "FollowUp information TLV"
*          either in a Sync or a FollowUp message.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[in] offsetPtr                            - (pointer to) value of cumulativeScaledRateOffset to be set in
*                                                   the "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_U32                                      *offsetPtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(offsetPtr);
    if (taiNumber > CPSS_DXCH_PTP_TAI_NUMBER_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).cumulativeScaleRateTaiReg1[taiNumber];
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 32, offsetPtr);

    return rc;
}

/**
* @internal cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet function
* @endinternal
*
* @brief   Get PTP Cumulative Scaled Rate Offset in the "FollowUp information TLV"
*          either in a Sync or a FollowUp message.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[in] offsetPtr                            - (pointer to) value of cumulativeScaledRateOffset to be set in
*                                                   the "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_U32                                      *offsetPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, offsetPtr));

    rc = internal_cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet(devNum, taiNumber, offsetPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, offsetPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortMacTsQueueSigatureSet function
* @endinternal
*
* @brief   Set MCH signature configuration per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[in] signatureMode            - MCH signature mode.
* @param[in] signatureValue           - MCH initial signature value.
*                                       valid when signatureMode == CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E.
*                                       (APPLICABLE RANGE: (0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
*/
static GT_STATUS internal_cpssDxChPtpManagerPortMacTsQueueSigatureSet
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    IN  CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT                    signatureMode,
    IN  GT_U32                                                      signatureValue
)
{
    cpssOsPrintf("devNum %d portNum %d signatureMode %d signatureValue %d\n",devNum,portNum,signatureMode,signatureValue); /* avoid compilation warnings CPSS_TBD_BOOKMARK_IRONMAN */

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
}


/**
* @internal cpssDxChPtpManagerPortMacTsQueueSigatureSet function
* @endinternal
*
* @brief   Set MCH signature configuration per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[in] signatureMode            - MCH signature mode.
* @param[in] signatureValue           - MCH initial signature value.
*                                       valid when signatureMode == CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E.
*                                       (APPLICABLE RANGE: (0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
*/
GT_STATUS cpssDxChPtpManagerPortMacTsQueueSigatureSet
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    IN  CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT                    signatureMode,
    IN  GT_U32                                                      signatureValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortMacTsQueueSigatureSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, signatureMode, signatureValue));

    rc = internal_cpssDxChPtpManagerPortMacTsQueueSigatureSet(devNum, portNum, signatureMode, signatureValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, signatureMode, signatureValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortMacTsQueueSigatureGet function
* @endinternal
*
* @brief   Get MCH signature configuration per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[out] signatureModePtr        - (pointer to) MCH signature mode.
* @param[out] signatureValuePtr       - (pointer to) MCH initial signature value.
*                                       valid when signatureMode == CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E.
*                                       (APPLICABLE RANGE: (0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
*/
static GT_STATUS internal_cpssDxChPtpManagerPortMacTsQueueSigatureGet
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    OUT CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT                    *signatureModePtr,
    OUT GT_U32                                                      *signatureValuePtr
)
{
    cpssOsPrintf("devNum %d portNum %d signatureMode %d signatureValue %d\n",devNum,portNum,*signatureModePtr,*signatureValuePtr); /* avoid compilation warnings CPSS_TBD_BOOKMARK_IRONMAN */

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChPtpManagerPortMacTsQueueSigatureGet function
* @endinternal
*
* @brief   Get MCH signature configuration per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[out] signatureModePtr        - (pointer to) MCH signature mode.
* @param[out] signatureValuePtr       - (pointer to) MCH initial signature value.
*                                       valid when signatureMode == CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E.
*                                       (APPLICABLE RANGE: (0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
*/
GT_STATUS cpssDxChPtpManagerPortMacTsQueueSigatureGet
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    OUT CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT                    *signatureModePtr,
    OUT GT_U32                                                      *signatureValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortMacTsQueueSigatureGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, signatureModePtr, signatureValuePtr));

    rc = internal_cpssDxChPtpManagerPortMacTsQueueSigatureGet(devNum, portNum, signatureModePtr, signatureValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, signatureModePtr, signatureValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file cpssPxPhySmi.c
*
* @brief API implementation for port Serial Management Interface facility.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/phy/private/prvCpssPxPhyLog.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/phy/private/prvCpssPxPhySmi.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* Validate SMI interfce */
#define PRV_CPSS_PX_SMI_INTERFACE_CHECK_MAC(interface)  \
    if (interface != CPSS_PHY_SMI_INTERFACE_0_E)        \
    {                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(interface));  \
    }
/* Validate XSMI interfce */
#define PRV_CPSS_PX_XSMI_INTERFACE_CHECK_MAC(interface) \
    if (interface != CPSS_PHY_XSMI_INTERFACE_0_E)       \
    {                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(interface));  \
    }

/**
* @internal internal_cpssPxPhySmiMdcDivisionFactorSet function
* @endinternal
*
* @brief   Sets fast MDC Division Selector
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] divisionFactor           - division factor of the core clock
*                                      to get the MDC (Serial Management Interface Clock)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPhySmiMdcDivisionFactorSet
(
    IN GT_SW_DEV_NUM devNum,
    IN CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  fastMdcDivSel;
    GT_U32  smiAccelerate;
    GT_U32  smiFastMdc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.SMI.SMIMiscConfiguration;

    switch (divisionFactor)
    {
        case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E:
            /* Configure <SMI accelerate> */
            return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, 1);
        case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E:
            smiAccelerate = 0;
            smiFastMdc = 1;
            fastMdcDivSel = 0;
            break;
        case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E:
            smiAccelerate = 0;
            smiFastMdc = 1;
            fastMdcDivSel = 1;
            break;
        case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E:
            smiAccelerate = 0;
            smiFastMdc = 1;
            fastMdcDivSel = 2;
            break;
        case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E:
            smiAccelerate = 0;
            smiFastMdc = 1;
            fastMdcDivSel = 3;
            break;
        case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E:
            smiAccelerate = 0;
            smiFastMdc = 1;
            fastMdcDivSel = 4;
            break;
        case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_512_E:
            smiAccelerate = 0;
            smiFastMdc = 0;
            fastMdcDivSel = 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(divisionFactor));
    }

    /* Configure <SMI accelerate> */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, smiAccelerate);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Configure <SMI FastMDC> */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 1, 1, smiFastMdc);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Configure <Fast MDC Division Selector> */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 2, 3, fastMdcDivSel);
}

/**
* @internal cpssPxPhySmiMdcDivisionFactorSet function
* @endinternal
*
* @brief   Sets fast MDC Division Selector
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] divisionFactor           - division factor of the core clock
*                                      to get the MDC (Serial Management Interface Clock)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPhySmiMdcDivisionFactorSet
(
    IN GT_SW_DEV_NUM devNum,
    IN CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPhySmiMdcDivisionFactorSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, divisionFactor));

    rc = internal_cpssPxPhySmiMdcDivisionFactorSet(devNum, divisionFactor);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, divisionFactor));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPhySmiMdcDivisionFactorGet function
* @endinternal
*
* @brief   Gets fast MDC Division Selector
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
*
* @param[out] divisionFactorPtr        - (pointer to) division factor of the core clock
*                                      to get the MDC (Serial Management Interface Clock)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPhySmiMdcDivisionFactorGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT *divisionFactorPtr
)
{
    GT_U32  regAddr;
    GT_U32  regData;
    GT_STATUS rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(divisionFactorPtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.SMI.SMIMiscConfiguration;

    /* <SMI accelerate> */
    rc =  prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }
    if (regData == 1)
    {
        *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E;
    }
    else
    {
        /* <SMI FastMDC>  */
        rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 1, 1, &regData);
        if(rc != GT_OK)
        {
            return rc;
        }
        if (regData == 0)
        {
            *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_512_E;
        }
        else
        {
            /* <Fast MDC Division Selector> */
            rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 2, 3, &regData);
            if(rc != GT_OK)
            {
                return rc;
            }

            switch (regData)
            {
                case 0:
                    *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E;
                    break;
                case 1:
                    *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E;
                    break;
                case 2:
                    *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E;
                    break;
                case 3:
                    *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E;
                    break;
                case 4:
                    *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
    }
    return GT_OK;
}

/**
* @internal cpssPxPhySmiMdcDivisionFactorGet function
* @endinternal
*
* @brief   Gets fast MDC Division Selector
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
*
* @param[out] divisionFactorPtr        - (pointer to) division factor of the core clock
*                                      to get the MDC (Serial Management Interface Clock)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPhySmiMdcDivisionFactorGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT *divisionFactorPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPhySmiMdcDivisionFactorGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, divisionFactorPtr));

    rc = internal_cpssPxPhySmiMdcDivisionFactorGet(devNum, divisionFactorPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, divisionFactorPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPhyXsmiMdcDivisionFactorSet function
* @endinternal
*
* @brief   Sets Division factor of the core clock to get the MDC
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] divisionFactor           - division factor of the core clock
*                                      to get the MDC (Serial Management Interface Clock)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPhyXsmiMdcDivisionFactorSet
(
    IN GT_SW_DEV_NUM devNum,
    IN CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
)
{
    GT_U32  regAddr;
    GT_U32  regData;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    switch(divisionFactor)
    {
        case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E:
            regData = 0;
            break;
        case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E:
            regData = 1;
            break;
        case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E:
            regData = 2;
            break;
        case CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E:
            regData = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_MG_MAC(devNum).XSMI.XSMIConfiguration;

    /* <MDC Division Mode> */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 2, regData);
}

/**
* @internal cpssPxPhyXsmiMdcDivisionFactorSet function
* @endinternal
*
* @brief   Sets MDC frequency for Master XSMI Interface
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] divisionFactor           - division factor of the core clock
*                                      to get the MDC (Serial Management Interface Clock)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPhyXsmiMdcDivisionFactorSet
(
    IN GT_SW_DEV_NUM devNum,
    IN CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPhyXsmiMdcDivisionFactorSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, divisionFactor));

    rc = internal_cpssPxPhyXsmiMdcDivisionFactorSet(devNum, divisionFactor);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, divisionFactor));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPhyXsmiMdcDivisionFactorGet function
* @endinternal
*
* @brief   Gets MDC frequency for Master XSMI Interface
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
*
* @param[out] divisionFactorPtr        - (pointer to) division factor of the core clock
*                                      to get the MDC (Serial Management Interface Clock)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPhyXsmiMdcDivisionFactorGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT *divisionFactorPtr
)
{
    GT_U32  regAddr;
    GT_U32  regData;
    GT_STATUS rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(divisionFactorPtr);

    regAddr = PRV_PX_REG1_UNIT_MG_MAC(devNum).XSMI.XSMIConfiguration;

    /* Get <MDC Division Mode */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 2, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch (regData)
    {
        case 0:
            *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E;
            break;
        case 1:
            *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E;
            break;
        case 2:
            *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E;
            break;
        case 3:
            *divisionFactorPtr = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPhyXsmiMdcDivisionFactorGet function
* @endinternal
*
* @brief   Gets MDC frequency for Master XSMI Interface
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
*
* @param[out] divisionFactorPtr        - (pointer to) division factor of the core clock
*                                      to get the MDC (Serial Management Interface Clock)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPhyXsmiMdcDivisionFactorGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT *divisionFactorPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPhyXsmiMdcDivisionFactorGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, divisionFactorPtr));

    rc = internal_cpssPxPhyXsmiMdcDivisionFactorGet(devNum, divisionFactorPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, divisionFactorPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPhySmiMngRegsAddrGet function
* @endinternal
*
* @brief   Gets addresses of control SMI register
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] smiInterface             - SMI instance.
*                                      Supports only one SMI interface
*
* @param[out] ctrlRegAddrPtr          - address of SMI control register
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvCpssPxPhySmiMngRegsAddrGet
(
    IN GT_U8    devNum,
    IN  CPSS_PHY_SMI_INTERFACE_ENT smiInterface,
    OUT GT_U32  *ctrlRegAddrPtr
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_SMI_INTERFACE_CHECK_MAC(smiInterface);
    *ctrlRegAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.SMI.SMIManagement;
    return GT_OK;
}

/**
* @internal prvCpssPxPhyXSmiMngRegsAddrGet function
* @endinternal
*
* @brief   Gets addresses of control and address XSMI registers
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] xsmiInterface            - XSMI instance.
*                                      Supports only one XSMI interface
*
* @param[out] ctrlRegAddrPtr           - address of XSMI control register
* @param[out] addrRegAddrPtr           - address of XSMI address register
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvCpssPxPhyXSmiMngRegsAddrGet
(
    IN  GT_U8           devNum,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    OUT GT_U32          *ctrlRegAddrPtr,
    OUT GT_U32          *addrRegAddrPtr
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_XSMI_INTERFACE_CHECK_MAC(xsmiInterface);

    CPSS_NULL_PTR_CHECK_MAC(ctrlRegAddrPtr);
    CPSS_NULL_PTR_CHECK_MAC(addrRegAddrPtr);

    xsmiInterface = xsmiInterface;

    *ctrlRegAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.XSMI.XSMIManagement;
    *addrRegAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.XSMI.XSMIAddress;

    return GT_OK;
}

/**
* @internal prvCpssPxPhySmiObjBind function
* @endinternal
*
* @brief   Binds SMI service function pointers:
*         SMI Control Register Read/Write.
*         The generic SMI functions - cpssSmiRegisterReadShort/cpssSmiRegisterWriteShort
*         use these pointers.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - the device number
*                                       none.
*/
GT_VOID prvCpssPxPhySmiObjBind
(
    IN  GT_SW_DEV_NUM     devNum
)
{
    PRV_CPSS_PP_MAC(devNum)->phyInfo.genSmiBindFunc.
                      cpssPhySmiRegsAddrGet = prvCpssPxPhySmiMngRegsAddrGet;
    PRV_CPSS_PP_MAC(devNum)->phyInfo.genXSmiBindFunc.
                      cpssPhyXSmiRegsAddrGet = prvCpssPxPhyXSmiMngRegsAddrGet;

}



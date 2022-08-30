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
* @file cpssDxChPortFastInit.c
*
* @brief CPSS implementation for Port initialization.
*
* @version   3
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

extern CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency[CPSS_PORT_INTERFACE_MODE_NA_E][CPSS_PORT_SPEED_NA_E];

extern GT_STATUS prvCpssDxChPortTraceDelay
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_U32      millisec
);

/**
* @internal prvCpssDxChPortFastXgPcsResetStateSet function
* @endinternal
*
* @brief   Just set XG and above PCS Reset state.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port pcs is under Reset
*                                      GT_FALSE - Port pcs is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortFastXgPcsResetStateSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      regAddr;        /* register address */
    GT_U32      value;          /* value to write into the register */
    GT_U32      portGroupId;    /* local core number */

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) < PRV_CPSS_PORT_XG_E)
    {
        return GT_OK;
    }

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);

    if((GT_TRUE == state) || (PRV_CPSS_PORT_XG_E ==
                              PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum)))
    {
        value = BOOL2BIT_MAC(!state);
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                      macRegs.perPortRegs[portNum].xgGlobalConfReg0;
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                        0, 1, value);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if((GT_TRUE == state) || (PRV_CPSS_PORT_XLG_E ==
                              PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum)))
    {
        value = BOOL2BIT_MAC(state);
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->xlgRegs.pcs40GRegs.commonCtrl;
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                    9, 1, value);
            if (rc != GT_OK)
                return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortFastSerdesResetStateSet function
* @endinternal
*
* @brief   Set SERDES Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not CPU port)
* @param[in] startSerdes              - first SERDES number
* @param[in] serdesesNum              - number of SERDESes
* @param[in] state                    - Reset state
*                                      GT_TRUE  - Port SERDES is under Reset
*                                      GT_FALSE - Port SERDES is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortFastSerdesResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32    startSerdes,
    IN  GT_U32    serdesesNum,
    IN  GT_BOOL   state
)
{
    GT_U32    regValue;     /* register field value */
    GT_U32    regAddr;      /* register address */
    GT_U32    fieldOffset;  /* register field offset */
    GT_U32    i;            /* iterator */
    GT_U32    portGroupId;  /* local core number */
    GT_STATUS rc;           /* return code */

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    regValue = BOOL2BIT_MAC(!state);
    fieldOffset = 3; /* sd_reset_in */
    for (i = startSerdes; i < startSerdes + serdesesNum; i++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[i].serdesExternalReg2;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId , regAddr,
                                                fieldOffset, 1, regValue);
        if(rc !=GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortFastMacResetStateSet function
* @endinternal
*
* @brief   Set MAC Reset state on specified port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
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
GT_STATUS prvCpssDxChPortFastMacResetStateSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* value to write into the register */
    GT_U32      offset;     /* bit number inside register       */
    GT_U32      portGroupId;/* local core number */

    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                                devNum, portNum);
    offset = 1;
    value = BOOL2BIT_MAC(!state);
    if((GT_TRUE == state) || (PRV_CPSS_PORT_XG_E ==
                              PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portNum)))
    {/* set MACResetn bit */
        PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portNum,PRV_CPSS_PORT_XG_E,&regAddr);
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            if((rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                        offset, 1, value)) != GT_OK)
            {
                return rc;
            }
        }
    }

    if((GT_TRUE == state) || (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) <
                              PRV_CPSS_PORT_XG_E))
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portNum,PRV_CPSS_PORT_GE_E,&regAddr);
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            offset = 6;
            value = BOOL2BIT_MAC(state);
            if((rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                        offset, 1, value)) != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

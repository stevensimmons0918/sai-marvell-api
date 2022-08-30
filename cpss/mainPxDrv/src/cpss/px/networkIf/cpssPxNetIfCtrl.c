/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssPxNetIf.c
*
* DESCRIPTION:
*       'control' functions for PX network interface API functions
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/networkIf/private/prvCpssPxNetworkIfLog.h>
#include <cpss/px/networkIf/cpssPxNetIf.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/cos/private/prvCpssPxCoS.h>


/**
* @internal internal_cpssPxNetIfSdmaRxResourceErrorModeSet function
* @endinternal
*
* @brief   Set a bit per TC queue which defines the behavior in case of
*         RX resource error
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] queue                    - traffic class  (APPLICABLE RANGES: 0..7)
* @param[in] mode                     - current packet mode: retry to send or abort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, queue, mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the request is not supported
*/
static GT_STATUS internal_cpssPxNetIfSdmaRxResourceErrorModeSet
(
    IN  GT_SW_DEV_NUM                devNum,
    IN  GT_U32                       queue,
    IN  CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT   mode
)
{
    GT_U32      regAddr;      /* The register address to read from.           */
    GT_U32      regData;      /* Data read/write from/to register.            */
    GT_U32         portGroupId;/* port group Id for multi-port-groups device support */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(queue);
    PRV_CPSS_PX_NETIF_INITIALIZED_CHECK_MAC(devNum);

    portGroupId = 0;

    /* get the register address of queue 0..7 */
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.rxSdmaResourceErrorCountAndMode[queue];

    switch (mode)
    {
        case CPSS_PX_NET_RESOURCE_ERROR_MODE_RETRY_E:
            regData = 2; /* set bit 9 reset bit 8 */
            break;
        case CPSS_PX_NET_RESOURCE_ERROR_MODE_ABORT_E:
            regData = 1; /* reset bit 9 set bit 8 */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(mode);
    }

    return prvCpssHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId, regAddr, 8, 2, regData);
}


/**
* @internal cpssPxNetIfSdmaRxResourceErrorModeSet function
* @endinternal
*
* @brief   Set a bit per TC queue which defines the behavior in case of
*         RX resource error
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] queue                    - traffic class  (APPLICABLE RANGES: 0..7)
* @param[in] mode                     - current packet mode: retry to send or abort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, queue, mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the request is not supported
*/
GT_STATUS cpssPxNetIfSdmaRxResourceErrorModeSet
(
    IN  GT_SW_DEV_NUM                devNum,
    IN  GT_U32                       queue,
    IN  CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT   mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxNetIfSdmaRxResourceErrorModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queue, mode));

    rc = internal_cpssPxNetIfSdmaRxResourceErrorModeSet(devNum, queue, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queue, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssPxNetIfSdmaRxResourceErrorModeGet function
* @endinternal
*
* @brief   Get a bit per TC queue which defines the behavior in case of
*         RX resource error
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] queue                    - traffic class  (APPLICABLE RANGES: 0..7)
*
* @param[out] modePtr                  - current packet mode: retry to send or abort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, queue, mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the request is not supported
* @retval GT_BAD_PTR               - on NULL ptr
*/
static GT_STATUS internal_cpssPxNetIfSdmaRxResourceErrorModeGet
(
    IN  GT_SW_DEV_NUM                devNum,
    IN  GT_U32                       queue,
    OUT CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT   *modePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;      /* The register address to read from.           */
    GT_U32      regData;      /* Data read/write from/to register.            */
    GT_U32      portGroupId;/* port group Id for multi-port-groups device support */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(queue);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);
    PRV_CPSS_PX_NETIF_INITIALIZED_CHECK_MAC(devNum);

    portGroupId = 0;

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.rxSdmaResourceErrorCountAndMode[queue];
    rc = prvCpssHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum), portGroupId,
                                             regAddr, &regData);
    if(rc != GT_OK)
        return rc;
    *modePtr = (U32_GET_FIELD_MAC(regData,9,1)) ? CPSS_PX_NET_RESOURCE_ERROR_MODE_RETRY_E : CPSS_PX_NET_RESOURCE_ERROR_MODE_ABORT_E;

    return rc;
}

/**
* @internal cpssPxNetIfSdmaRxResourceErrorModeGet function
* @endinternal
*
* @brief   Get a bit per TC queue which defines the behavior in case of
*         RX resource error
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] queue                    - traffic class  (APPLICABLE RANGES: 0..7)
*
* @param[out] modePtr                  - current packet mode: retry to send or abort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, queue, mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the request is not supported
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssPxNetIfSdmaRxResourceErrorModeGet
(
    IN  GT_SW_DEV_NUM                devNum,
    IN  GT_U32                       queue,
    OUT CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT   *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxNetIfSdmaRxResourceErrorModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queue, modePtr));

    rc = internal_cpssPxNetIfSdmaRxResourceErrorModeGet(devNum, queue, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queue, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}



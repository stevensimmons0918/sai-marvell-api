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
* @file cpssDxChBrgPe.c
*
* @brief Bridge Port Extender (BPE) DxCh cpss implementation.
*
* @version   9
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgPe.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
* @internal internal_cpssDxChBrgPeEnableSet function
* @endinternal
*
* @brief   Enables globally Bridge Port Extender (BPE) feature.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  BPE feature enabled.
*                                      GT_FALSE: BPE feature disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChBrgPeEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_U32  hwValue;    /* HW Value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    hwValue = (enable == GT_TRUE) ? 1 : 0;

    return prvCpssHwPpSetRegField(devNum,
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                     egrTxQConf.br802_1ConfigRegs.globalConfig,
                0, 1, hwValue);
}

/**
* @internal cpssDxChBrgPeEnableSet function
* @endinternal
*
* @brief   Enables globally Bridge Port Extender (BPE) feature.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  BPE feature enabled.
*                                      GT_FALSE: BPE feature disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPeEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgPeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgPeEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgPeEnableGet function
* @endinternal
*
* @brief   Gets Bridge Port Extender (BPE) global enabling status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE:  BPE feature enabled.
*                                      GT_FALSE: BPE feature disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChBrgPeEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;       /* return code  */
    GT_U32    hwValue;  /* HW Value     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    rc = prvCpssHwPpGetRegField(devNum,
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                     egrTxQConf.br802_1ConfigRegs.globalConfig,
                0, 1, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (hwValue == 0) ? GT_FALSE : GT_TRUE ;

    return GT_OK;
}

/**
* @internal cpssDxChBrgPeEnableGet function
* @endinternal
*
* @brief   Gets Bridge Port Extender (BPE) global enabling status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE:  BPE feature enabled.
*                                      GT_FALSE: BPE feature disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPeEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgPeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgPeEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgPePortEnableSet function
* @endinternal
*
* @brief   Enables per port Bridge Port Extender (BPE) feature.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE:  port enabled for BPE feature.
*                                      GT_FALSE: port disabled for BPE feature.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChBrgPePortEnableSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL                enable
)
{
    GT_STATUS rc;       /* return code  */
    GT_U32    hwValue;  /* HW Value     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    hwValue = (enable == GT_TRUE) ? 1 : 0;

    rc = prvCpssHwPpSetRegField(devNum,
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.srcPortBpeEnable,
                portNum, 1, hwValue);

    if (rc != GT_OK)
    {
        return rc;
    }

    return prvCpssHwPpSetRegField(devNum,
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.perPortBpeEnable,
                portNum, 1, hwValue);
}

/**
* @internal cpssDxChBrgPePortEnableSet function
* @endinternal
*
* @brief   Enables per port Bridge Port Extender (BPE) feature.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE:  port enabled for BPE feature.
*                                      GT_FALSE: port disabled for BPE feature.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPePortEnableSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL                enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgPePortEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgPePortEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgPePortEnableGet function
* @endinternal
*
* @brief   Gets Bridge Port Extender (BPE) enabling status per port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - (pointer to) GT_TRUE:  port enabled for BPE feature.
*                                      GT_FALSE: port disabled for BPE feature.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device or port number
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChBrgPePortEnableGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL                *enablePtr
)
{
    GT_STATUS rc;       /* return code  */
    GT_U32    hwValue;  /* HW Value     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    rc = prvCpssHwPpGetRegField(devNum,
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.srcPortBpeEnable,
                portNum, 1, &hwValue);

    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (hwValue == 0) ? GT_FALSE : GT_TRUE ;

    return GT_OK;
}

/**
* @internal cpssDxChBrgPePortEnableGet function
* @endinternal
*
* @brief   Gets Bridge Port Extender (BPE) enabling status per port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - (pointer to) GT_TRUE:  port enabled for BPE feature.
*                                      GT_FALSE: port disabled for BPE feature.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device or port number
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPePortEnableGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL                *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgPePortEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgPePortEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgPePortPcidMcFilterEnableSet function
* @endinternal
*
* @brief   Configures <port,PCID> pair for downsteam multicast source filtering.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] pcid                     - Port E-Channel Identifier (APPLICABLE RANGES: 0..4095)
* @param[in] mcFilterEnable           - GT_TRUE:  Multicast source filtering is enabled for the
*                                      <port,PCID> pair.
*                                      - GT_FALSE: Multicast source filtering is disabled for the
*                                      <port,PCID> pair.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device, port number or PCID value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChBrgPePortPcidMcFilterEnableSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32                 pcid,
    IN GT_BOOL                mcFilterEnable
)
{
    GT_U32    hwValue;  /* HW Value     */
    GT_U32    bitIndex;/* bit index in the register for the portNum*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.bpePointToPointEChannels <= pcid)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    hwValue = (mcFilterEnable == GT_TRUE) ? 1 : 0;
    bitIndex = (portNum == CPSS_CPU_PORT_NUM_CNS) ? 28 : portNum;

    return prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_XCAT_TABLE_INGRESS_ECID_E,
                                        pcid, 0, bitIndex, 1, hwValue);
}

/**
* @internal cpssDxChBrgPePortPcidMcFilterEnableSet function
* @endinternal
*
* @brief   Configures <port,PCID> pair for downsteam multicast source filtering.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] pcid                     - Port E-Channel Identifier (APPLICABLE RANGES: 0..4095)
* @param[in] mcFilterEnable           - GT_TRUE:  Multicast source filtering is enabled for the
*                                      <port,PCID> pair.
*                                      - GT_FALSE: Multicast source filtering is disabled for the
*                                      <port,PCID> pair.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device, port number or PCID value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPePortPcidMcFilterEnableSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32                 pcid,
    IN GT_BOOL                mcFilterEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgPePortPcidMcFilterEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, pcid, mcFilterEnable));

    rc = internal_cpssDxChBrgPePortPcidMcFilterEnableSet(devNum, portNum, pcid, mcFilterEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, pcid, mcFilterEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgPePortPcidMcFilterEnableGet function
* @endinternal
*
* @brief   Gets downsteam multicast source filtering enabling status for <port,PCID> pair.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] pcid                     - Port E-Channel Identifier (APPLICABLE RANGES: 0..4095)
*
* @param[out] mcFilterEnablePtr        - (pointer to)
*                                      GT_TRUE:  Multicast source filtering is enabled for the
*                                      <port,PCID> pair.
*                                      GT_FALSE: Multicast source filtering is disabled for the
*                                      <port,PCID> pair.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device, port number or PCID value
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChBrgPePortPcidMcFilterEnableGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32                 pcid,
    OUT GT_BOOL                *mcFilterEnablePtr
)
{
    GT_STATUS rc;   /* return code  */
    GT_U32    hwValue;  /* HW Value     */
    GT_U32    bitIndex;/* bit index in the register for the portNum*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(mcFilterEnablePtr);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.bpePointToPointEChannels <= pcid)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    bitIndex = (portNum == CPSS_CPU_PORT_NUM_CNS) ? 28 : portNum;

    rc =  prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_XCAT_TABLE_INGRESS_ECID_E,
                                        pcid, 0, bitIndex, 1, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *mcFilterEnablePtr = (hwValue == 0) ? GT_FALSE : GT_TRUE ;

    return rc;
}

/**
* @internal cpssDxChBrgPePortPcidMcFilterEnableGet function
* @endinternal
*
* @brief   Gets downsteam multicast source filtering enabling status for <port,PCID> pair.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] pcid                     - Port E-Channel Identifier (APPLICABLE RANGES: 0..4095)
*
* @param[out] mcFilterEnablePtr        - (pointer to)
*                                      GT_TRUE:  Multicast source filtering is enabled for the
*                                      <port,PCID> pair.
*                                      GT_FALSE: Multicast source filtering is disabled for the
*                                      <port,PCID> pair.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device, port number or PCID value
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPePortPcidMcFilterEnableGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32                 pcid,
    OUT GT_BOOL                *mcFilterEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgPePortPcidMcFilterEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, pcid, mcFilterEnablePtr));

    rc = internal_cpssDxChBrgPePortPcidMcFilterEnableGet(devNum, portNum, pcid, mcFilterEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, pcid, mcFilterEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet function
* @endinternal
*
* @brief   Associate the cascade port (not DSA cascade port) with a trunkId.
*         this trunk id should represent the 'uplink trunk'
*         The purpose as described in the Functional specification is for "Trunk-ID for E-Tag<Ingress_E-CID_base>
*         assignment for multi-destination traffic"
*         NOTE: this function MUST not be called on 'Control Bridge'
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         NOT APPLICABLE DEVICES:
*         Lion2; xCat3; AC5.
*         INPUTS:
*         devNum     - device number
*         cascadePortNum - physical port number of the BPE 802.1br cascade port (not DSA cascade port)
*         enable     - GT_TRUE: associate the trunkId to cascadePortNum.
*         - GT_FALSE: disassociate the trunkId from cascadePortNum.
*         trunkId    - the trunkId (of the uplink trunk)
*         note: relevant only when enable == GT_TRUE
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] cascadePortNum           - physical port number of the BPE 802.1br cascade port (not DSA cascade port)
* @param[in] enable                   - GT_TRUE:  associate the trunkId to cascadePortNum.
*                                      - GT_FALSE: disassociate the trunkId from cascadePortNum.
* @param[in] trunkId                  - the  (of the uplink trunk)
*                                      note: relevant only when enable == GT_TRUE
*                                      APPLICABLE RANGE: 1..(4K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad devNum, cascadePortNum
* @retval GT_OUT_OF_RANGE          - trunkId out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     cascadePortNum,
    IN GT_BOOL                  enable,
    IN GT_TRUNK_ID              trunkId
)
{
    GT_STATUS rc;   /* return code  */
    CPSS_TRUNK_MEMBER_STC   trunkMember;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,cascadePortNum);
    if (enable == GT_FALSE)
    {
        /* disassociate the port from the trunk of the 'uplink' */
        trunkMember.port = cascadePortNum;
        trunkMember.hwDevice = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

        /* check if this cascade port is part of trunk by itself ! */
        rc = cpssDxChTrunkDbIsMemberOfTrunk(devNum,&trunkMember,&trunkId);
        if (rc == GT_OK)
        {
            /* trunkId was set */
        }
        else
        {
            trunkId = 0;
        }
    }
    else
    {
        /* trunkId from the caller */
        if (trunkId == 0)
        {
            /* non valid value for trunk . return GT_OUT_OF_RANGE to be the same as if trunkId >= 4k */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    /* set the trunkId in the port physical table*/
    return prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                        cascadePortNum,/*global port*/
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRUNK_ID_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        trunkId);
}

/**
* @internal cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet function
* @endinternal
*
* @brief   Associate the cascade port (not DSA cascade port) with a trunkId.
*         this trunk id should represent the 'uplink trunk'
*         The purpose as described in the Functional specification is for "Trunk-ID for E-Tag<Ingress_E-CID_base>
*         assignment for multi-destination traffic"
*         NOTE: this function MUST not be called on 'Control Bridge'
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         NOT APPLICABLE DEVICES:
*         Lion2; xCat3; AC5.
*         INPUTS:
*         devNum     - device number
*         cascadePortNum - physical port number of the BPE 802.1br cascade port (not DSA cascade port)
*         enable     - GT_TRUE: associate the trunkId to cascadePortNum.
*         - GT_FALSE: disassociate the trunkId from cascadePortNum.
*         trunkId    - the trunkId (of the uplink trunk)
*         note: relevant only when enable == GT_TRUE
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] cascadePortNum           - physical port number of the BPE 802.1br cascade port (not DSA cascade port)
* @param[in] enable                   - GT_TRUE:  associate the trunkId to cascadePortNum.
*                                      - GT_FALSE: disassociate the trunkId from cascadePortNum.
* @param[in] trunkId                  - the  (of the uplink trunk)
*                                      note: relevant only when enable == GT_TRUE
*                                      APPLICABLE RANGE: 1..(4K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad devNum, cascadePortNum
* @retval GT_OUT_OF_RANGE          - trunkId out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     cascadePortNum,
    IN GT_BOOL                  enable,
    IN GT_TRUNK_ID              trunkId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet);

    /* This API calls cpssDxChTrunkDbIsMemberOfTrunk from High Level trunk lib.
       Use first Lock of cpssDxChTrunkDbIsMemberOfTrunk to avoid deadlocks. */
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRUNK_MANAGER_CNS);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cascadePortNum, enable, trunkId));

    rc = internal_cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet(devNum, cascadePortNum, enable, trunkId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cascadePortNum, enable, trunkId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRUNK_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet function
* @endinternal
*
* @brief   Get the associate trunkId with the cascade port (not DSA cascade port) .
*         this trunk id should represent the 'uplink trunk'
*         The purpose as described in the Functional specification is for "Trunk-ID for E-Tag<Ingress_E-CID_base>
*         assignment for multi-destination traffic"
*         NOTE: this function MUST not be called on 'Control Bridge'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] cascadePortNum           - physical port number of the BPE 802.1br cascade port (not DSA cascade port)
*
* @param[out] enablePtr                - (pointer to) indication that trunkId associate to the cascadePortNum.
*                                      - GT_TRUE:  trunkId associate to the cascadePortNum.
*                                      - GT_FALSE: trunkId is not associate to the cascadePortNum.
* @param[out] trunkIdPtr               - the trunkId (of the uplink trunk)
*                                      note: relevant only when (enablePtr) == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad devNum, cascadePortNum
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     cascadePortNum,
    OUT GT_BOOL                  *enablePtr,
    OUT GT_TRUNK_ID              *trunkIdPtr
)
{
    GT_STATUS rc;   /* return code  */
    CPSS_TRUNK_MEMBER_STC   trunkMember;
    GT_TRUNK_ID trunkId,haTrunkId;
    GT_U32  value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,cascadePortNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(trunkIdPtr);

    /* disassociate the port from the trunk of the 'uplink' */
    trunkMember.port = cascadePortNum;
    trunkMember.hwDevice = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

    /* check if this cascade port is part of trunk by itself ! */
    rc = cpssDxChTrunkDbIsMemberOfTrunk(devNum,&trunkMember,&trunkId);
    if (rc == GT_OK)
    {
        /* trunkId was set */
    }
    else
    {
        trunkId = 0;
    }

    /* set the trunkId in the port physical table*/
    rc =  prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                        cascadePortNum,/*global port*/
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRUNK_ID_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    haTrunkId = (GT_TRUNK_ID)value;

    if (haTrunkId != trunkId)
    {
        *enablePtr = GT_TRUE;
        *trunkIdPtr = haTrunkId;

        if (haTrunkId == 0)
        {
            /* the port is member in 'orig trunk'
               (because trunkId != haTrunkId --> so trunkId != 0) ...
               but the 'HA' state 'no in trunk' ...
               how can this happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        *enablePtr = GT_FALSE;
        *trunkIdPtr = 0;/*don't care*/
    }

    return GT_OK;

}

/**
* @internal cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet function
* @endinternal
*
* @brief   Get the associate trunkId with the cascade port (not DSA cascade port) .
*         this trunk id should represent the 'uplink trunk'
*         The purpose as described in the Functional specification is for "Trunk-ID for E-Tag<Ingress_E-CID_base>
*         assignment for multi-destination traffic"
*         NOTE: this function MUST not be called on 'Control Bridge'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] cascadePortNum           - physical port number of the BPE 802.1br cascade port (not DSA cascade port)
*
* @param[out] enablePtr                - (pointer to) indication that trunkId associate to the cascadePortNum.
*                                      - GT_TRUE:  trunkId associate to the cascadePortNum.
*                                      - GT_FALSE: trunkId is not associate to the cascadePortNum.
* @param[out] trunkIdPtr               - the trunkId (of the uplink trunk)
*                                      note: relevant only when (enablePtr) == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad devNum, cascadePortNum
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     cascadePortNum,
    OUT GT_BOOL                  *enablePtr,
    OUT GT_TRUNK_ID              *trunkIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet);

    /* This API calls cpssDxChTrunkDbIsMemberOfTrunk from High Level trunk lib.
       Use first Lock of cpssDxChTrunkDbIsMemberOfTrunk to avoid deadlocks. */
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRUNK_MANAGER_CNS);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cascadePortNum, enablePtr, trunkIdPtr));

    rc = internal_cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet(devNum, cascadePortNum, enablePtr, trunkIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cascadePortNum, enablePtr, trunkIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRUNK_MANAGER_CNS);

    return rc;
}



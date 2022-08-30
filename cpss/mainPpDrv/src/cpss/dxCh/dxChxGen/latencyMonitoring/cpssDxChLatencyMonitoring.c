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
* @file cpssDxChLatencyMonitoring.c
*
* @brief CPSS DxCh Latency monitoring API implementation.
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/latencyMonitoring/cpssDxChLatencyMonitoring.h>
#include <cpss/dxCh/dxChxGen/latencyMonitoring/private/prvCpssDxChLatencyMonitoringLog.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* access DB of registers by the global LMU index */
#define PRV_DXCH_REG1_UNIT_GLOBAL_LMU_MAC(_devNum,_globalLmuIndex) \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->LMU[_globalLmuIndex]


#define LMU_UNIT_EXISTS_CHECK(_devNum)                                     \
    if(PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.lmuInfo.numLmuUnits == 0)     \
    {                                                                      \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "The device hold no LMU support"); \
    }



/**
* @internal regDbGlobalLmuIndexGet function
* @endinternal
*
* @brief  Get index to 'regDb' of the LMU and for the 'tables' of LMU .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] globalLmuByApplication - index by the application to the LMU (unaware to mirrored tiles!)
*
* @retval db_globalLmuIndex - the index to the 'regDb'
*/
static GT_U32   regDbGlobalLmuIndexGet(
    IN GT_U8    devNum,
    IN GT_U32   globalLmuByApplication
)
{
    GT_U32 db_globalLmuIndex;/* DB index for 'reg DB' to the global LMU index : 0..31 - considering 'Mirrored tiles' */
    GT_U32 lmuNum;
    GT_U32 ravenNum;
    GT_U32 tileNum;

    lmuNum   =  U32_GET_FIELD_MAC(globalLmuByApplication, 0, 1);
    ravenNum =  U32_GET_FIELD_MAC(globalLmuByApplication, 1, 2);
    tileNum  =  U32_GET_FIELD_MAC(globalLmuByApplication, 3, 2);

    if((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
    {
        /* mirrored tile - Raven 'sit' on mirrored Addresses */
        ravenNum   = 3 - ravenNum;
        /* rebuild index using mirrored Raven index */
        db_globalLmuIndex = lmuNum | ravenNum << 1 | tileNum << 3;
    }
    else
    {
        /* global LMU index */
        db_globalLmuIndex = globalLmuByApplication;
    }

    return db_globalLmuIndex;
}


/**
* @internal internal_cpssDxChLatencyMonitoringPortCfgSet function
* @endinternal
*
* @brief  Set index and profile for physical port for port latency monitoring.
*         Combination {source index, target index} used for indexing.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] portType              - determine type of port: ingress, egress or both.
* @param[in] index                 - index (APPLICABLE RANGES: 0..255)
* @param[in] profile               - latency profile (APPLICABLE RANGES: 0..511)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - index or profile is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringPortCfgSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN CPSS_DIRECTION_ENT                   portType,
    IN GT_U32                               index,
    IN GT_U32                               profile
)
{
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    if(index > 255)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(profile > 511)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }


    if((CPSS_DIRECTION_INGRESS_E == portType) || (CPSS_DIRECTION_BOTH_E == portType))
    {
        rc = prvCpssDxChWriteTableEntryField(
            devNum,
            CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_SOURCE_PHYSICAL_PORT_MAPPING_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            0, 17, (index | (profile << 8))
        );
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    if((CPSS_DIRECTION_EGRESS_E == portType) || (CPSS_DIRECTION_BOTH_E == portType))
    {
        rc = prvCpssDxChWriteTableEntryField(
            devNum,
            CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_TARGET_PHYSICAL_PORT_MAPPING_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            0, 17, (index | (profile << 8))
        );
    }
    return rc;
}

/**
* @internal cpssDxChLatencyMonitoringPortCfgSet function
* @endinternal
*
* @brief  Set index and profile for physical port for port latency monitoring.
*         Combination {source index, target index} used for indexing.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] portType              - determine type of port: ingress, egress or both.
* @param[in] index                 - index (APPLICABLE RANGES: 0..255)
* @param[in] profile               - latency profile (APPLICABLE RANGES: 0..511)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - index or profile is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringPortCfgSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN CPSS_DIRECTION_ENT                   portType,
    IN GT_U32                               index,
    IN GT_U32                               profile
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringPortCfgSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portType, index, profile));

    rc = internal_cpssDxChLatencyMonitoringPortCfgSet(devNum, portNum, portType, index, profile);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portType, index, profile));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringPortCfgGet function
* @endinternal
*
* @brief  Get index and profile for physical port for port latency monitoring.
*         Combination {source index, target index} used for indexing.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] portType              - determine type of port: ingress or egress
*
* @param[out] indexPtr             - (pointer to) index
* @param[out] profilePtr           - (pointer to) latency profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringPortCfgGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DIRECTION_ENT                   portType,
    OUT GT_U32                               *indexPtr,
    OUT GT_U32                               *profilePtr
)
{
    GT_STATUS rc;
    GT_U32    value;
    CPSS_DXCH_TABLE_ENT table;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(indexPtr);
    CPSS_NULL_PTR_CHECK_MAC(profilePtr);

    table = (CPSS_DIRECTION_INGRESS_E == portType)?
                CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_SOURCE_PHYSICAL_PORT_MAPPING_E:
            (CPSS_DIRECTION_EGRESS_E == portType)?
                CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_TARGET_PHYSICAL_PORT_MAPPING_E:
                0;
    if(0 == table)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc =  prvCpssDxChReadTableEntryField(devNum, table, portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        0, 17, &value
    );

    if(rc != GT_OK)
    {
        return rc;
    }

    *indexPtr   = U32_GET_FIELD_MAC(value, 0, 8);
    *profilePtr = U32_GET_FIELD_MAC(value, 8, 9);

    return GT_OK;
}

/**
* @internal cpssDxChLatencyMonitoringPortCfgGet function
* @endinternal
*
* @brief  Get index and profile for physical port for port latency monitoring.
*         Combination {source index, target index} used for indexing.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] portType              - determine type of port: ingress or egress
*
* @param[out] indexPtr             - (pointer to) index
* @param[out] profilePtr           - (pointer to) latency profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringPortCfgGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DIRECTION_ENT                   portType,
    OUT GT_U32                               *indexPtr,
    OUT GT_U32                               *profilePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringPortCfgGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portType, indexPtr, profilePtr));

    rc = internal_cpssDxChLatencyMonitoringPortCfgGet(devNum, portNum, portType, indexPtr, profilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portType, indexPtr, profilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringEnableSet function
* @endinternal
*
* @brief  Set latency monitoring enabled state.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] srcPortNum            - source port number
* @param[in] trgPortNum            - target port number
* @param[in] enabled               - latency monitoring enabled state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM srcPortNum,
    IN GT_PHYSICAL_PORT_NUM trgPortNum,
    IN GT_BOOL              enabled
)
{
    GT_U32    index;
    GT_U32    offset;
    GT_U32    srcIdx;
    GT_U32    dstIdx;
    GT_U32    profile;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    rc = cpssDxChLatencyMonitoringPortCfgGet(devNum, srcPortNum,
        CPSS_DIRECTION_INGRESS_E, &srcIdx, &profile);
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = cpssDxChLatencyMonitoringPortCfgGet(devNum, trgPortNum,
        CPSS_DIRECTION_EGRESS_E, &dstIdx, &profile);
    if(GT_OK != rc)
    {
        return rc;
    }

    index = dstIdx * 256 + srcIdx;
    offset = index & 0x1F;
    index >>= 5;

    return prvCpssDxChWriteTableEntryField(
            devNum,
            CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_PORT_LATENCY_MONITORING_E,
            index,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            offset, 1, BOOL2BIT_MAC(enabled));
}

/**
* @internal cpssDxChLatencyMonitoringEnableSet function
* @endinternal
*
* @brief  Set latency monitoring enabled state.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] srcPortNum            - source port number
* @param[in] trgPortNum            - target port number
* @param[in] enabled               - latency monitoring enabled state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM srcPortNum,
    IN GT_PHYSICAL_PORT_NUM trgPortNum,
    IN GT_BOOL              enabled
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringEnableSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srcPortNum, trgPortNum, enabled));

    rc = internal_cpssDxChLatencyMonitoringEnableSet(devNum, srcPortNum, trgPortNum, enabled);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srcPortNum, trgPortNum, enabled));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringEnableGet function
* @endinternal
*
* @brief  Get latency monitoring enabled state.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] srcPortNum            - source port number
* @param[in] trgPortNum            - target port number
*
* @param[out] enabledPtr           - (pointer to) latency monitoring enabled state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringEnableGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM srcPortNum,
    IN GT_PHYSICAL_PORT_NUM trgPortNum,
    OUT GT_BOOL             *enabledPtr
)
{
    GT_U32    index;
    GT_U32    offset;
    GT_U32    value;
    GT_U32    srcIdx;
    GT_U32    dstIdx;
    GT_U32    profile;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(enabledPtr);

    rc = cpssDxChLatencyMonitoringPortCfgGet(devNum, srcPortNum,
        CPSS_DIRECTION_INGRESS_E, &srcIdx, &profile);
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = cpssDxChLatencyMonitoringPortCfgGet(devNum, trgPortNum,
        CPSS_DIRECTION_EGRESS_E, &dstIdx, &profile);
    if(GT_OK != rc)
    {
        return rc;
    }

    index = dstIdx * 256 + srcIdx;
    offset = index & 0x1F;
    index >>= 5;

    rc = prvCpssDxChReadTableEntryField(
        devNum,
        CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_PORT_LATENCY_MONITORING_E,
        index,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        offset, 1, &value);
    if(GT_OK != rc)
    {
        return rc;
    }

    *enabledPtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChLatencyMonitoringEnableGet function
* @endinternal
*
* @brief  Get latency monitoring enabled state.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] srcPortNum            - source port number
* @param[in] trgPortNum            - target port number
*
* @param[out] enabledPtr           - (pointer to) latency monitoring enabled state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringEnableGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM srcPortNum,
    IN GT_PHYSICAL_PORT_NUM trgPortNum,
    OUT GT_BOOL             *enabledPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringEnableGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srcPortNum, trgPortNum, enabledPtr));

    rc = internal_cpssDxChLatencyMonitoringEnableGet(devNum, srcPortNum, trgPortNum, enabledPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srcPortNum, trgPortNum, enabledPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringSamplingProfileSet function
* @endinternal
*
* @brief  Set latency monitoring sampling profile.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringSamplingProfileSet
(
    IN GT_U8        devNum,
    IN GT_U32       latencyProfile,
    IN GT_U32       samplingProfile
)
{
    GT_U32 regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    if(latencyProfile > 511)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(samplingProfile > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).
        latencyMonitoringProfile2SamplingProfile[latencyProfile >> 3];

    return prvCpssHwPpSetRegField(devNum, regAddr, (latencyProfile & 7) * 3,
        3, samplingProfile);
}

/**
* @internal cpssDxChLatencyMonitoringSamplingProfileSet function
* @endinternal
*
* @brief  Set latency monitoring sampling profile.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringSamplingProfileSet
(
    IN GT_U8        devNum,
    IN GT_U32       latencyProfile,
    IN GT_U32       samplingProfile
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringSamplingProfileSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, latencyProfile, samplingProfile));

    rc = internal_cpssDxChLatencyMonitoringSamplingProfileSet(devNum, latencyProfile, samplingProfile);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, latencyProfile, samplingProfile));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringSamplingProfileGet function
* @endinternal
*
* @brief  Get latency monitoring sampling profile.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
*
* @param[out] samplingProfilePtr   - (pointer to) sampling profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringSamplingProfileGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       latencyProfile,
    OUT GT_U32       *samplingProfilePtr
)
{
    GT_U32 regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(samplingProfilePtr);

    if(latencyProfile > 511)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).
        latencyMonitoringProfile2SamplingProfile[latencyProfile >> 3];

    return prvCpssHwPpGetRegField(devNum, regAddr, (latencyProfile & 7) * 3,
        3, samplingProfilePtr);
}

/**
* @internal cpssDxChLatencyMonitoringSamplingProfileGet function
* @endinternal
*
* @brief  Get latency monitoring sampling profile.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
*
* @param[out] samplingProfilePtr   - (pointer to) sampling profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringSamplingProfileGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       latencyProfile,
    OUT GT_U32       *samplingProfilePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringSamplingProfileGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, latencyProfile, samplingProfilePtr));

    rc = internal_cpssDxChLatencyMonitoringSamplingProfileGet(devNum, latencyProfile, samplingProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, latencyProfile, samplingProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringSamplingConfigurationSet function
* @endinternal
*
* @brief  Set latency monitoring sampling configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
* @param[in] samplingMode          - sampling mode.
* @param[in] samplingThreshold     - sampling threshold. Value 0 - every packet sampled.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringSamplingConfigurationSet
(
    IN GT_U8                                          devNum,
    IN GT_U32                                         samplingProfile,
    IN CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT samplingMode,
    IN GT_U32                                         samplingThreshold
)
{
    GT_U32 regAddr;
    GT_U32 rc;
    GT_U32 value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    if(samplingProfile > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(samplingMode)
    {
        case CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_DETERMINISTIC_E:
           value = 0;
           break;
        case CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_RANDOM_E:
           value = 1;
           break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).
        latencyMonitoringSamplingConfig[samplingProfile];
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, samplingThreshold);
    if(GT_OK != rc)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).latencyMonitoringControl;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, samplingProfile, 1, value);
    if(GT_OK != rc)
    {
        return rc;
    }

    if(samplingMode == CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_RANDOM_E)
    {
        /* enable PRNG and set Every_Usage mode if random mode was chosen */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 8, 2, 3);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal cpssDxChLatencyMonitoringSamplingConfigurationSet function
* @endinternal
*
* @brief  Set latency monitoring sampling configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
* @param[in] samplingMode          - sampling mode.
* @param[in] samplingThreshold     - sampling threshold. Value 0 - every packet sampled.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringSamplingConfigurationSet
(
    IN GT_U8                                          devNum,
    IN GT_U32                                         samplingProfile,
    IN CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT samplingMode,
    IN GT_U32                                         samplingThreshold
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringSamplingConfigurationSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, samplingProfile, samplingMode, samplingThreshold));

    rc = internal_cpssDxChLatencyMonitoringSamplingConfigurationSet(devNum, samplingProfile, samplingMode, samplingThreshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, samplingProfile, samplingMode, samplingThreshold));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringSamplingConfigurationGet function
* @endinternal
*
* @brief  Get latency monitoring sampling configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
*
* @param[out] samplingModePtr      - (pointer to)sampling mode.
* @param[out] samplingThresholdPtr - (pointer to)sampling threshold
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringSamplingConfigurationGet
(
    IN  GT_U8                                          devNum,
    IN  GT_U32                                         samplingProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT *samplingModePtr,
    OUT GT_U32                                         *samplingThresholdPtr
)
{
    GT_U32 regAddr;
    GT_U32 rc;
    GT_U32 value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(samplingThresholdPtr);
    CPSS_NULL_PTR_CHECK_MAC(samplingModePtr);

    if(samplingProfile > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).
        latencyMonitoringSamplingConfig[samplingProfile];

    rc = prvCpssHwPpReadRegister(devNum, regAddr, samplingThresholdPtr);
    if(GT_OK != rc)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).
        latencyMonitoringControl;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, samplingProfile, 1, &value);
    if(GT_OK != rc)
    {
        return rc;
    }

    switch(value)
    {
        case 0:
           *samplingModePtr = CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_DETERMINISTIC_E;
           break;
        case 1:
           *samplingModePtr = CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_RANDOM_E;
           break;
        default:
            /* NOT REACHED */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal cpssDxChLatencyMonitoringSamplingConfigurationGet function
* @endinternal
*
* @brief  Get latency monitoring sampling configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] samplingProfile       - sampling profile (APPLICABLE RANGES: 0..7)
*
* @param[out] samplingModePtr      - (pointer to)sampling mode.
* @param[out] samplingThresholdPtr - (pointer to)sampling threshold
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringSamplingConfigurationGet
(
    IN  GT_U8                                          devNum,
    IN  GT_U32                                         samplingProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT *samplingModePtr,
    OUT GT_U32                                         *samplingThresholdPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringSamplingConfigurationGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, samplingProfile, samplingModePtr, samplingThresholdPtr));

    rc = internal_cpssDxChLatencyMonitoringSamplingConfigurationGet(devNum, samplingProfile, samplingModePtr, samplingThresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, samplingProfile, samplingModePtr, samplingThresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* check that the BMP is valid for the device */
static GT_STATUS latencyMonitoringBmpCheck
(
    IN  GT_U8                                  devNum,
    IN  GT_U32                                 latencyProfile,
    INOUT  GT_LATENCY_MONITORING_UNIT_BMP      *lmuBmpPtr,
    OUT  GT_U32                                *numLmuUnitsPtr
)
{
    GT_U32  lmuMaxUnits = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lmuInfo.numLmuUnits;
    GT_LATENCY_MONITORING_UNIT_BMP  lmuBmp = *lmuBmpPtr;
    GT_LATENCY_MONITORING_UNIT_BMP  activeLmuBmp; /* bitmap of active LMUs */
    GT_U32                          chiplet; /* chiplet ID in Falcon */

    if(latencyProfile >= 512)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "latencyProfile must be less than [%d] but got [%d]",
            512 ,
            latencyProfile);
    }

    *numLmuUnitsPtr = lmuMaxUnits;

    if(lmuMaxUnits == 1/* the device ignores the BMP */)
    {
        /* update the output */
        *lmuBmpPtr = CPSS_LMU_UNAWARE_MODE_CNS;
        return GT_OK;
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* Falcon part numbers have variable number of chiplets including holes
           between them. Calculate LMU active bitmap based on available chiplets. */
        activeLmuBmp = 0;
        for (chiplet = 0; chiplet < 16; chiplet++)
        {
            if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->activeRavensBitmap & (1 << chiplet))
            {
                /* each chiplet has two LMUs */
                activeLmuBmp |= (0x3 << (chiplet * 2));
            }
        }
    }
    else
    {
        /* all possible LMUs are active */
        activeLmuBmp = 0xFFFFFFFF;
    }

    if(lmuBmp == CPSS_LMU_UNAWARE_MODE_CNS)
    {
        *lmuBmpPtr &= activeLmuBmp;
        return GT_OK;
    }

    if(0 == lmuBmp)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Empty BMP is not supported");
    }

    if (lmuBmp & (~activeLmuBmp))
    {
        /* input bitmap hold not active units */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "The device supports active BMP of[0x%8.8x] but got BMP of[0x%8.8x]",
            activeLmuBmp,
            lmuBmp);
    }

    if(lmuMaxUnits < 32)
    {
        /* check that the BMP is valid */
        if(lmuBmp > (GT_U32)((1<<lmuMaxUnits)-1))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "The device supports BMP of[0x%8.8x] but got BMP of[0x%8.8x]",
                (1<<lmuMaxUnits)-1,
                lmuBmp);
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChLatencyMonitoringStatGet function
* @endinternal
*
* @brief  Get latency monitoring statistics.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[out] statisticsPtr        - (pointer to) latency monitoring statistics
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note In case if multiple LMUs selected, packet counts are summed, average
* latency calculated as average of average latency values for all selected LMUs.
* Minimal latency is the lowest value of minimal latency values for all
* selected LMUs. Maximal latency is the highest value of maximal latency values
* for all selected LMUs.
*
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringStatGet
(
    IN  GT_U8                                  devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP         lmuBmp,
    IN  GT_U32                                 latencyProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_STAT_STC  *statisticsPtr
)
{
    GT_U32 regAddr;
    GT_U32 ii;

    GT_U32 db_globalLmuIndex;/* DB index for 'reg DB' to the global LMU index : 0..31 - considering 'Mirrored tiles' */
    GT_U32 value[8];
    GT_STATUS rc;
    GT_U64 totalAvgLatency;   /* summary of all average values */
    GT_U32 averageNumber = 0; /* number of values for average latency calculation */
    GT_U64 tmp;
    GT_U64 tmp2;
    GT_STATUS st;
    GT_U32 lmuMaxUnits;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    LMU_UNIT_EXISTS_CHECK(devNum);

    CPSS_NULL_PTR_CHECK_MAC(statisticsPtr);

    /* check that the BMP is valid for the device */
    rc = latencyMonitoringBmpCheck(devNum,latencyProfile,&lmuBmp,&lmuMaxUnits);
    if(rc != GT_OK)
    {
        return rc;
    }

    statisticsPtr->packetsOutOfRange.l[0] = 0;
    statisticsPtr->packetsOutOfRange.l[1] = 0;
    statisticsPtr->packetsInRange.l[0] = 0;
    statisticsPtr->packetsInRange.l[1] = 0;
    totalAvgLatency.l[0] = 0;
    totalAvgLatency.l[1] = 0;
    statisticsPtr->maxLatency = 0;
    statisticsPtr->minLatency = 0x3FFFFFFF;

    for(ii = 0; ii < lmuMaxUnits; ii++)
    {
        if((lmuBmp & (1 << ii)) == 0)
        {
            continue;
        }

        /* get index to the LMU 'regDb' (and tables DB) */
        db_globalLmuIndex = regDbGlobalLmuIndexGet(devNum,ii);

        rc = prvCpssDxChReadTableEntry(
            devNum,
            CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + db_globalLmuIndex,
            latencyProfile, &value[0]);
        if(GT_OK != rc)
        {
            return rc;
        }

        tmp.l[1] = 0;
        regAddr = PRV_DXCH_REG1_UNIT_GLOBAL_LMU_MAC(devNum,db_globalLmuIndex).
            profileStatisticsReadData[0];
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &tmp.l[0]);
        if(GT_OK != rc)
        {
            return rc;
        }
        statisticsPtr->packetsOutOfRange = prvCpssMathAdd64(
            statisticsPtr->packetsOutOfRange, tmp);


        regAddr = PRV_DXCH_REG1_UNIT_GLOBAL_LMU_MAC(devNum,db_globalLmuIndex).
            profileStatisticsReadData[1];
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &tmp.l[0]);
        if(GT_OK != rc)
        {
            return rc;
        }

        regAddr = PRV_DXCH_REG1_UNIT_GLOBAL_LMU_MAC(devNum,db_globalLmuIndex).
            profileStatisticsReadData[2];
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &tmp.l[1]);
        if(GT_OK != rc)
        {
            return rc;
        }
        statisticsPtr->packetsInRange = prvCpssMathAdd64(
            statisticsPtr->packetsInRange, tmp);

        tmp.l[1] = 0;
        regAddr = PRV_DXCH_REG1_UNIT_GLOBAL_LMU_MAC(devNum,db_globalLmuIndex).
            profileStatisticsReadData[3];
        rc = prvCpssHwPpReadRegister(devNum, regAddr,&tmp.l[0]);
        if(GT_OK != rc)
        {
            return rc;
        }

        if(tmp.l[0])
        {
            averageNumber++;
            totalAvgLatency = prvCpssMathAdd64(totalAvgLatency, tmp);
        }

        regAddr = PRV_DXCH_REG1_UNIT_GLOBAL_LMU_MAC(devNum,db_globalLmuIndex).
            profileStatisticsReadData[4];
        rc = prvCpssHwPpReadRegister(devNum, regAddr,
            &tmp.l[0]);
        if(GT_OK != rc)
        {
            return rc;
        }

        if(tmp.l[0] > statisticsPtr->maxLatency)
        {
            statisticsPtr->maxLatency = tmp.l[0];
        }

        regAddr = PRV_DXCH_REG1_UNIT_GLOBAL_LMU_MAC(devNum,db_globalLmuIndex).
            profileStatisticsReadData[5];
        rc = prvCpssHwPpReadRegister(devNum, regAddr,
            &tmp.l[0]);
        if(GT_OK != rc)
        {
            return rc;
        }

        if(tmp.l[0] < statisticsPtr->minLatency)
        {
            statisticsPtr->minLatency = tmp.l[0];
        }
    }

    if(averageNumber)
    {
        tmp.l[0] = averageNumber;
        tmp.l[1] = 0;
        st = prvCpssMathDiv64(totalAvgLatency, tmp, &tmp2, NULL);
        if(GT_OK != st)
        {
            return st;
        }
        statisticsPtr->avgLatency = tmp2.l[0];
    }
    else
    {
        statisticsPtr->avgLatency = 0;
    }

    return GT_OK;
}

/**
* @internal cpssDxChLatencyMonitoringStatGet function
* @endinternal
*
* @brief  Get latency monitoring statistics.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[out] statisticsPtr        - (pointer to) latency monitoring statistics
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note In case if multiple LMUs selected, packet counts are summed, average
* latency calculated as average of average latency values for all selected LMUs.
* Minimal latency is the lowest value of minimal latency values for all
* selected LMUs. Maximal latency is the highest value of maximal latency values
* for all selected LMUs.
*
*/
GT_STATUS cpssDxChLatencyMonitoringStatGet
(
    IN  GT_U8                                  devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP         lmuBmp,
    IN  GT_U32                                 latencyProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_STAT_STC  *statisticsPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringStatGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lmuBmp, latencyProfile, statisticsPtr));

    rc = internal_cpssDxChLatencyMonitoringStatGet(devNum, lmuBmp, latencyProfile, statisticsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lmuBmp, latencyProfile, statisticsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringStatReset function
* @endinternal
*
* @brief    Reset latency monitoring statistics.
*           Function is for debug purpose only and should be used when specific
*           LMU units statistic is not updated by traffic.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringStatReset
(
    IN  GT_U8                                  devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP         lmuBmp,
    IN  GT_U32                                 latencyProfile
)
{
    GT_U32 ii;
    GT_U32 db_globalLmuIndex;/* DB index for 'reg DB' to the global LMU index : 0..31 - considering 'Mirrored tiles' */
    GT_STATUS rc;
    GT_U32 lmuMaxUnits;
    /* inital values of lmu_stat_ds table shifted once*/
    GT_U32 val[]={0/*outOfRange*/,0/*inRange[0]*/,0/*inRange[1]*/,0/*avg*/,0xF0000000/*maximal*/,0x03FFFFFF/*minimal*/};
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    LMU_UNIT_EXISTS_CHECK(devNum);

    /* check that the BMP is valid for the device */
    rc = latencyMonitoringBmpCheck(devNum,latencyProfile,&lmuBmp,&lmuMaxUnits);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(ii = 0; ii < lmuMaxUnits; ii++)
    {
        if((lmuBmp & (1 << ii)) == 0)
        {
            continue;
        }

        /* get index to the LMU 'regDb' (and tables DB) */
        db_globalLmuIndex = regDbGlobalLmuIndexGet(devNum,ii);
        rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + db_globalLmuIndex,
                                        latencyProfile, val);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal cpssDxChLatencyMonitoringStatReset function
* @endinternal
*
* @brief    Reset latency monitoring statistics.
*           Function is for debug purpose only and should be used when specific
*           LMU units statistic is not updated by traffic.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS cpssDxChLatencyMonitoringStatReset
(
    IN  GT_U8                                  devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP         lmuBmp,
    IN  GT_U32                                 latencyProfile
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringStatReset);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lmuBmp, latencyProfile));

    rc = internal_cpssDxChLatencyMonitoringStatReset(devNum, lmuBmp, latencyProfile);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lmuBmp, latencyProfile));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringCfgSet function
* @endinternal
*
* @brief  Set latency monitoring configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[in] lmuConfigPtr          - (pointer to)latency monitoring configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringCfgSet
(
    IN  GT_U8                                  devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP         lmuBmp,
    IN  GT_U32                                 latencyProfile,
    IN  CPSS_DXCH_LATENCY_MONITORING_CFG_STC  *lmuConfigPtr
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    value[4];
    GT_U32 db_globalLmuIndex;/* DB index for 'reg DB' to the global LMU index : 0..31 - considering 'Mirrored tiles' */
    GT_U32 lmuMaxUnits;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    LMU_UNIT_EXISTS_CHECK(devNum);

    /* check that the BMP is valid for the device */
    rc = latencyMonitoringBmpCheck(devNum,latencyProfile,&lmuBmp,&lmuMaxUnits);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(lmuConfigPtr->rangeMax > 0x3FFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(lmuConfigPtr->rangeMin > 0x3FFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(lmuConfigPtr->notificationThresh > 0x3FFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_IN_ENTRY_MAC(&value[0],  0, 30, lmuConfigPtr->notificationThresh);
    U32_SET_FIELD_IN_ENTRY_MAC(&value[0], 30, 30, lmuConfigPtr->rangeMax);
    U32_SET_FIELD_IN_ENTRY_MAC(&value[0], 60, 30, lmuConfigPtr->rangeMin);

    for(ii = 0; ii < lmuMaxUnits; ii++)
    {
        if(0 == (lmuBmp & (1 << ii)))
        {
            continue;
        }

        /* get index to the LMU 'regDb' (and tables DB) */
        db_globalLmuIndex = regDbGlobalLmuIndexGet(devNum,ii);

        rc = prvCpssDxChWriteTableEntry(
            devNum, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + db_globalLmuIndex,
            latencyProfile, &value[0]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChLatencyMonitoringCfgSet function
* @endinternal
*
* @brief  Set latency monitoring configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
* @param[in] lmuConfigPtr          - (pointer to)latency monitoring configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringCfgSet
(
    IN  GT_U8                                  devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP         lmuBmp,
    IN  GT_U32                                 latencyProfile,
    IN  CPSS_DXCH_LATENCY_MONITORING_CFG_STC  *lmuConfigPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringCfgSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lmuBmp, latencyProfile, lmuConfigPtr));

    rc = internal_cpssDxChLatencyMonitoringCfgSet(devNum, lmuBmp, latencyProfile, lmuConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lmuBmp, latencyProfile, lmuConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringCfgGet function
* @endinternal
*
* @brief  Get latency monitoring configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3. Only one LMU
*                                    can be selected.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
*
* @param[out] lmuConfigPtr         - (pointer to) latency monitoring configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringCfgGet
(
    IN  GT_U8                                 devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP        lmuBmp,
    IN  GT_U32                                latencyProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_CFG_STC  *lmuConfigPtr
)
{
    GT_U32 ii;
    GT_U32 value[4];
    GT_STATUS rc;
    GT_U32 db_globalLmuIndex;/* DB index for 'reg DB' to the global LMU index : 0..31 - considering 'Mirrored tiles' */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    LMU_UNIT_EXISTS_CHECK(devNum);

    CPSS_NULL_PTR_CHECK_MAC(lmuConfigPtr);

    if(latencyProfile > 511)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == lmuBmp)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for(ii = 0; (lmuBmp & 1) == 0; ii++) lmuBmp >>= 1;

    if (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        TXQ_SIP_6_CHECK_TILE_NUM_MAC(U32_GET_FIELD_MAC(ii, 3, 2));
    }

    /* get index to the LMU 'regDb' (and tables DB) */
    db_globalLmuIndex = regDbGlobalLmuIndexGet(devNum,ii);

    rc = prvCpssDxChReadTableEntry(
        devNum, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + db_globalLmuIndex,
        latencyProfile, &value[0]);
    if(GT_OK != rc)
    {
        return rc;
    }

    U32_GET_FIELD_IN_ENTRY_MAC(&value[0],  0, 30, lmuConfigPtr->notificationThresh);
    U32_GET_FIELD_IN_ENTRY_MAC(&value[0], 30, 30, lmuConfigPtr->rangeMax);
    U32_GET_FIELD_IN_ENTRY_MAC(&value[0], 60, 30, lmuConfigPtr->rangeMin);

    return rc;
}

/**
* @internal cpssDxChLatencyMonitoringCfgGet function
* @endinternal
*
* @brief  Get latency monitoring configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] lmuBmp                - LMU bitmap. LMU# selected depending on
*                                    egress MAC range of 8 MACs it corresponds. Bit
*                                    offset in the LMU bitmap equals LMU#. LMU#
*                                    calculated as egress MAC# >> 3. Only one LMU
*                                    can be selected.
* @param[in] latencyProfile        - latency profile (APPLICABLE RANGES: 0..511)
*
* @param[out] lmuConfigPtr         - (pointer to) latency monitoring configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLatencyMonitoringCfgGet
(
    IN  GT_U8                                 devNum,
    IN  GT_LATENCY_MONITORING_UNIT_BMP        lmuBmp,
    IN  GT_U32                                latencyProfile,
    OUT CPSS_DXCH_LATENCY_MONITORING_CFG_STC  *lmuConfigPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringCfgGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lmuBmp, latencyProfile, lmuConfigPtr));

    rc = internal_cpssDxChLatencyMonitoringCfgGet(devNum, lmuBmp, latencyProfile, lmuConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lmuBmp, latencyProfile, lmuConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringPortEnableSet function
* @endinternal
*
* @brief  Enable/Disable egress port for latency monitoring.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] enable                - latency monitoring enabled state
*                                    GT_TRUE - enable
*                                    GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringPortEnableSet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  GT_BOOL                                  enable
)
{
    GT_STATUS                       rc;
    GT_U32                          lmuNum;
    GT_U32                          channel;
    GT_U32                          regAddr;
    GT_U32                          regVal, regMask;
    GT_U32                          portMacNum;
    PRV_CPSS_REG_DB_INFO_STC        regDbInfo;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    LMU_UNIT_EXISTS_CHECK(devNum);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    rc = prvCpssSip6RegDbInfoGet(devNum,portMacNum,PRV_CPSS_REG_DB_TYPE_MTI_LMU_E,&regDbInfo);
    if(rc != GT_OK)
    {
        return rc;
    }
    lmuNum  = regDbInfo.regDbIndex;
    channel = regDbInfo.ciderIndexInUnit;

    regAddr = PRV_DXCH_REG1_UNIT_LMU_MAC(devNum, lmuNum).
        channelEnable;

    /* 200G and 400G ports use exclusive channels 8,9. Other type of ports use channels 0..7.
       LMU related data came either from channels 8,9 or from 0..7.
       CPSS calculate above channel = 0,4 for 200G ports and channel 0 for 400G.
       Configure channels 8,9 together with 0,4 to avoid reconfiguration during
       speed change procedures. For channels 0,4 and should set bits 8,9 respectively. */
    regMask = 1 << channel;
    regMask = (channel == 0)? (regMask | (1<<8)) :
              (channel == 4)? (regMask | (1<<9)) :
              regMask;
    regVal = (enable == GT_TRUE) ? regMask : 0;

    return prvCpssHwPpWriteRegBitMask(devNum, regAddr, regMask, regVal);
}

/**
* @internal cpssDxChLatencyMonitoringPortEnableSet function
* @endinternal
*
* @brief  Enable/Disable egress port for latency monitoring.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] enable                - latency monitoring enabled state
*                                    GT_TRUE - enable
*                                    GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS cpssDxChLatencyMonitoringPortEnableSet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  GT_BOOL                                  enable
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringPortEnableSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChLatencyMonitoringPortEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringPortEnableGet function
* @endinternal
*
* @brief  Get enabled state of latency monitoring for port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @param[out] enablePtr            - (pointer to)latency monitoring enabled state
*                                    GT_TRUE  - enabled
*                                    GT_FALSE - disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringPortEnableGet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    OUT GT_BOOL                                  *enablePtr
)
{
    GT_STATUS                       rc;
    GT_U32                          lmuNum;
    GT_U32                          channel;
    GT_U32                          value;
    GT_U32                          regAddr;
    GT_U32                          portMacNum;
    PRV_CPSS_REG_DB_INFO_STC        regDbInfo;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    LMU_UNIT_EXISTS_CHECK(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    rc = prvCpssSip6RegDbInfoGet(devNum,portMacNum,PRV_CPSS_REG_DB_TYPE_MTI_LMU_E,&regDbInfo);
    if(rc != GT_OK)
    {
        return rc;
    }
    lmuNum  = regDbInfo.regDbIndex;
    channel = regDbInfo.ciderIndexInUnit;

    regAddr = PRV_DXCH_REG1_UNIT_LMU_MAC(devNum, lmuNum).
        channelEnable;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, channel, 1, &value);
    *enablePtr = BIT2BOOL_MAC(value);
    return rc;
}


/**
* @internal cpssDxChLatencyMonitoringPortEnableGet function
* @endinternal
*
* @brief  Get enabled state of latency monitoring for port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @param[out] enablePtr            - (pointer to)latency monitoring enabled state
*                                    GT_TRUE  - enabled
*                                    GT_FALSE - disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS cpssDxChLatencyMonitoringPortEnableGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringPortEnableGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChLatencyMonitoringPortEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChLatencyMonitoringRegDbPrint function
* @endinternal
*
* @brief  debug function to print 'regDb' of the LMU and 'tables' of LMU .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChLatencyMonitoringRegDbPrint(IN GT_U8 devNum)
{
    PRV_CPSS_DXCH_TABLES_INFO_STC           *tableInfoPtr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC    *tablePtr;
    GT_U32  db_globalLmuIndex;
    GT_U32  regAddr;
    GT_U32  numOfTiles;
    GT_U32  maxLmu;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    LMU_UNIT_EXISTS_CHECK(devNum);

    numOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ?
                 PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles :
                 1;
    maxLmu = 8 * numOfTiles;/* 2 LMUs per Raven , 4 Ravens in Tile */

    cpssOsPrintf("LMU  regAddr \n");
    cpssOsPrintf("============ \n");
    for(db_globalLmuIndex = 0 ; db_globalLmuIndex < maxLmu; db_globalLmuIndex++)
    {
        /* representative register in the DB , just to have the 'tile' and 'Raven' and 'lmu' prefixes */
        regAddr = PRV_DXCH_REG1_UNIT_GLOBAL_LMU_MAC(devNum,db_globalLmuIndex).
            profileStatisticsReadData[0];
        cpssOsPrintf("%d      0x%8.8x \n" , db_globalLmuIndex , regAddr);
    }

    cpssOsPrintf("\n");
    cpssOsPrintf("LMU  Table Addr STATISTICS \n");
    cpssOsPrintf("========================== \n");

    for(db_globalLmuIndex = 0 ; db_globalLmuIndex < maxLmu; db_globalLmuIndex++)
    {
        tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,
            (CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+ db_globalLmuIndex));
        tablePtr = (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->writeTablePtr);
        cpssOsPrintf("%d      0x%8.8x \n" , db_globalLmuIndex , tablePtr->baseAddress);
    }

    cpssOsPrintf("\n");
    cpssOsPrintf("LMU  Table Addr CONFIGURATION \n");
    cpssOsPrintf("============================= \n");
    for(db_globalLmuIndex = 0 ; db_globalLmuIndex < maxLmu; db_globalLmuIndex++)
    {
        tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,
            (CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+ db_globalLmuIndex));
        tablePtr = (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->writeTablePtr);
        cpssOsPrintf("%d      0x%8.8x \n" , db_globalLmuIndex , tablePtr->baseAddress);
    }

    cpssOsPrintf("\n The End \n\n");
    return GT_OK;
}

/**
* @internal internal_cpssDxChLatencyMonitoringProfileModeSet function
* @endinternal
*
* @brief  Set the latency profile mode (port mode / queue mode)
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] profileMode           - latency profile mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringProfileModeSet
(
    IN   GT_U8                                 devNum,
    IN   CPSS_DXCH_LATENCY_PROFILE_MODE_ENT    profileMode
)
{
    GT_U32                  value;
    GT_U32                  regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    switch(profileMode)
    {
        case CPSS_DXCH_LATENCY_PROFILE_MODE_PORT_E: value = 0;
            break;
        case CPSS_DXCH_LATENCY_PROFILE_MODE_QUEUE_E: value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).egrPolicyGlobalConfig;

    return prvCpssHwPpSetRegField(devNum, regAddr, 17 /*offset*/, 1 /*length*/, value);
}

/**
* @internal cpssDxChLatencyMonitoringProfileModeSet function
* @endinternal
*
* @brief  Set the latency profile mode (port mode / queue mode)
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] profileMode           - latency profile mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChLatencyMonitoringProfileModeSet
(
    IN   GT_U8                                 devNum,
    IN   CPSS_DXCH_LATENCY_PROFILE_MODE_ENT    profileMode
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringProfileModeSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileMode));

    rc = internal_cpssDxChLatencyMonitoringProfileModeSet(devNum, profileMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChLatencyMonitoringProfileModeGet function
* @endinternal
*
* @brief  Get the latency profile mode (port mode/queue mode)
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - device number
* @param[out] profileModePtr        - (pointer to) latency profile mode
*
* @retval GT_OK                     - on success.
* @retval GT_BAD_PARAM              - one of the input parameters is not valid.
* @retval GT_BAD_PTR                - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_HW_ERROR               - on hardware error
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringProfileModeGet
(
    IN  GT_U8                                      devNum,
    OUT CPSS_DXCH_LATENCY_PROFILE_MODE_ENT         *profileModePtr
)
{

    GT_STATUS               rc;
    GT_U32                  regAddr;
    GT_U32                  value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(profileModePtr);
    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).egrPolicyGlobalConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 17 /*offset*/, 1 /*length*/, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *profileModePtr = (value == 1)? CPSS_DXCH_LATENCY_PROFILE_MODE_QUEUE_E : CPSS_DXCH_LATENCY_PROFILE_MODE_PORT_E;

    return rc;
}

/**
* @internal cpssDxChLatencyMonitoringProfileModeGet function
* @endinternal
*
* @brief  Get the latency profile mode (port mode/queue mode)
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - device number
* @param[out] profileModePtr        - (pointer to) latency profile mode
*
* @retval GT_OK                     - on success.
* @retval GT_BAD_PARAM              - one of the input parameters is not valid.
* @retval GT_BAD_PTR                - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_HW_ERROR               - on hardware error
*/
GT_STATUS cpssDxChLatencyMonitoringProfileModeGet
(
    IN  GT_U8                                     devNum,
    OUT CPSS_DXCH_LATENCY_PROFILE_MODE_ENT        *profileModePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringProfileModeGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileModePtr));

    rc = internal_cpssDxChLatencyMonitoringProfileModeGet(devNum, profileModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringPortProfileSet function
* @endinternal
*
* @brief  Set port latency profile in queue profile mode for the given physical port.
*         Here physical port represent TxQ port(queue group).
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] portNum               - the physical port number
* @param[in] portProfile           - port latency profile
*                                    (APPLICABLE RANGES: 0..511)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - profile is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringPortProfileSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   portProfile
)
{
    GT_STATUS   rc;
    GT_U32      groupOfQueuesIndex; /* Group of Queues Index for a port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    if(portProfile > 511)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /*convert physical port number to group of queues index ,let's read it from HW*/
    rc = prvCpssDxChTxqSip6GoQIndexGet(devNum,portNum,GT_TRUE,&groupOfQueuesIndex);
    if(rc!=GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP6_10_TABLE_EGRESS_PCL_QUEUE_GROUP_LATENCY_PROFILE_CONFIG_E,
            groupOfQueuesIndex,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            0, 9, portProfile);

    return rc;
}

/**
* @internal cpssDxChLatencyMonitoringPortProfileSet function
* @endinternal
*
* @brief  Set port latency profile in queue profile mode for the given physical port.
*         Here physical port represent TxQ port(queue group).
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] portNum               - the physical port number
* @param[in] portProfile           - port latency profile
*                                    (APPLICABLE RANGES: 0..511)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - profile is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChLatencyMonitoringPortProfileSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   portProfile
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringPortProfileSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portProfile));

    rc = internal_cpssDxChLatencyMonitoringPortProfileSet(devNum, portNum, portProfile);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portProfile));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringPortProfileGet function
* @endinternal
*
* @brief  Get port latency profile in queue profile mode for the given physical port.
*         Here physical port represent TxQ port(queue group).
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] portNum               - the physical port number
*
* @param[out] portProfilePtr       - (pointer to) port latency profile
*
* @retval GT_OK                       - on success.
* @retval GT_BAD_PARAM                - one of the input parameters is not valid.
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_HW_ERROR                 - on hardware error
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringPortProfileGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_U32                   *portProfilePtr
)
{
    GT_STATUS   rc;
    GT_U32      groupOfQueuesIndex; /* Group of Queues Index for a port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(portProfilePtr);

    /*convert physical port number to group of queues index ,let's read it from HW*/
    rc = prvCpssDxChTxqSip6GoQIndexGet(devNum,portNum,GT_TRUE,&groupOfQueuesIndex);
    if(rc!=GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_SIP6_10_TABLE_EGRESS_PCL_QUEUE_GROUP_LATENCY_PROFILE_CONFIG_E,
            groupOfQueuesIndex,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            0, 9, portProfilePtr);

    return rc;
}

/**
* @internal cpssDxChLatencyMonitoringPortProfileGet function
* @endinternal
*
* @brief  Get port latency profile in queue profile mode for the given physical port.
*         Here physical port represent TxQ port(queue group).
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] portNum               - the physical port number
*
* @param[out] portProfilePtr       - (pointer to) port latency profile
*
* @retval GT_OK                       -  on success.
* @retval GT_BAD_PARAM                - one of the input parameters is not valid.
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_HW_ERROR                 - on hardware error
*/
GT_STATUS cpssDxChLatencyMonitoringPortProfileGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_U32                   *portProfilePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringPortProfileGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portProfilePtr));

    rc = internal_cpssDxChLatencyMonitoringPortProfileGet(devNum, portNum, portProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringQueueProfileSet function
* @endinternal
*
* @brief  Set queue latency profile.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum            - device number
* @param[in] tcQueue           - traffic class queue
*                                (APPLICABLE RANGE: 0..15)
* @param[in] queueProfile      - queue latency profile
*                                (APPLICABLE RANGE: 0..511)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - profile is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringQueueProfileSet
(
    IN GT_U8                devNum,
    IN GT_U8                tcQueue,
    IN GT_U32               queueProfile
)
{

    GT_U32                  regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_COS_CHECK_16_TC_MAC(tcQueue);
    if(queueProfile > 511)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).queueOffsetLatencyProfileConfig[tcQueue >> 1];

    return prvCpssHwPpSetRegField(devNum, regAddr, (tcQueue % 2) * 9/*offset*/, 9/*Length*/, queueProfile);
}

/**
* @internal cpssDxChLatencyMonitoringQueueProfileSet function
* @endinternal
*
* @brief  Set queue latency profile.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum            - device number
* @param[in] tcQueue           - traffic class queue
*                                (APPLICABLE RANGE: 0..15)
* @param[in] queueProfile      - queue latency profile
*                                (APPLICABLE RANGE: 0..511)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - profile is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChLatencyMonitoringQueueProfileSet
(
    IN GT_U8                devNum,
    IN GT_U8                tcQueue,
    IN GT_U32               queueProfile
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringQueueProfileSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, queueProfile));

    rc = internal_cpssDxChLatencyMonitoringQueueProfileSet(devNum, tcQueue, queueProfile);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, queueProfile));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLatencyMonitoringQueueProfileGet function
* @endinternal
*
* @brief  Get queue offset latency profile.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum             - device number
* @param[in] tcQueue            - traffic class queue
*                                 (APPLICABLE RANGE: 0..15)
* @param[OUT] queueProfilePtr   - (pointer to) queue latency profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssDxChLatencyMonitoringQueueProfileGet
(
    IN  GT_U8          devNum,
    IN  GT_U8          tcQueue,
    OUT GT_U32         *queueProfilePtr
)
{
    GT_U32                  regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E |CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(queueProfilePtr);
    PRV_CPSS_DXCH_COS_CHECK_16_TC_MAC(tcQueue);

    regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).queueOffsetLatencyProfileConfig[tcQueue >> 1];

    return prvCpssHwPpGetRegField(devNum, regAddr, (tcQueue % 2) * 9/*offset*/, 9/*length*/, queueProfilePtr);
}

/**
* @internal cpssDxChLatencyMonitoringQueueProfileGet function
* @endinternal
*
* @brief  Get queue offset latency profile.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum             - device number
* @param[in] tcQueue            - traffic class queue
*                                 (APPLICABLE RANGE: 0..15)
* @param[OUT] queueProfilePtr   - (pointer to) queue latency profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChLatencyMonitoringQueueProfileGet
(
    IN  GT_U8          devNum,
    IN  GT_U8          tcQueue,
    OUT GT_U32         *queueProfilePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLatencyMonitoringQueueProfileGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, queueProfilePtr));

    rc = internal_cpssDxChLatencyMonitoringQueueProfileGet(devNum, tcQueue, queueProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, queueProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


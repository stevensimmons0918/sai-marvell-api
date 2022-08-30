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
* @file cpssPxEgress.c
*
* @brief CPSS PX implementation for egress processing.
*
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/cpssPxTypes.h>
#include <cpss/generic/log/prvCpssPxGenLog.h>
#include <cpss/px/egress/private/prvCpssPxEgressLog.h>
#include <cpss/px/egress/cpssPxEgress.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
/* include the definition of PHA Firmware */
#include <cpss/px/egress/private/ppa_fw_imem_addr_data.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

/* Used in THR25_EVB_QCN thread. Each entry in this table should hold 16 bits of VLAN tag {UP[2:0],CFI,VID[11:0]}
   The table is 17 entires (ports 0-16), thus total 17x16b = 272b  */
#define SHARED_MEM_QCN_VLAN_TBL_ENTRY_OFFSET_CNS  (0x100 / 4)

/* check packet type */
#define PACKET_TYPE_CHECK_MAC(packetType)   \
    CPSS_PARAM_CHECK_MAX_MAC(packetType,32)

/* check VLAN tag */
#define QCN_VLAN_TAG_CHECK_MAC(vlanTagPtr)   \
    CPSS_PARAM_CHECK_MAX_MAC(vlanTagPtr->vid, 4096); \
    CPSS_PARAM_CHECK_MAX_MAC(vlanTagPtr->dei, 2); \
    CPSS_PARAM_CHECK_MAX_MAC(vlanTagPtr->pcp, 8)

/**
* @internal internal_cpssPxEgressBypassModeSet function
* @endinternal
*
* @brief   Enable/disable bypass of the egress processing and option to disable
*         PPA clock for power saving when 'bypass' the egress processing.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phaBypassEnable          GT_TRUE  -     bypass the programmable header alteration (PHA).
*                                      GT_FALSE - not bypass the programmable header alteration (PHA).
* @param[in] ppaClockEnable           - The PPA clock can be disabled for power saving.
*                                      The PPA clock can be disabled only when the phaBypassEnable == GT_TRUE.
*                                      GT_TRUE  - the PPA clock is enabled.
*                                      GT_FALSE - the PPA clock is disabled.(for power saving).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxEgressBypassModeSet
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_BOOL           phaBypassEnable,
    IN GT_BOOL           ppaClockEnable
)
{
    GT_STATUS rc;
    GT_U32 regAddr;    /* register address to set */
    GT_U32 value;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    regAddr = PRV_PX_REG1_UNIT_PHA_MAC(devNum).pha_regs.PHACtrl;

    /*<PHA Bypass>*/
    value = BOOL2BIT_MAC(phaBypassEnable);
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),regAddr,1,1,value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*<Disable PPA Clock>*/
    value = 1 - BOOL2BIT_MAC(ppaClockEnable);
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),regAddr,2,1,value);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxEgressBypassModeSet function
* @endinternal
*
* @brief   Enable/disable bypass of the egress processing and option to disable
*         PPA clock for power saving when 'bypass' the egress processing.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phaBypassEnable          GT_TRUE  -     bypass the programmable header alteration (PHA).
*                                      GT_FALSE - not bypass the programmable header alteration (PHA).
* @param[in] ppaClockEnable           - The PPA clock can be disabled for power saving.
*                                      The PPA clock can be disabled only when the phaBypassEnable == GT_TRUE.
*                                      GT_TRUE  - the PPA clock is enabled.
*                                      GT_FALSE - the PPA clock is disabled.(for power saving).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressBypassModeSet
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_BOOL           phaBypassEnable,
    IN GT_BOOL           ppaClockEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxEgressBypassModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, phaBypassEnable, ppaClockEnable));

    rc = internal_cpssPxEgressBypassModeSet(devNum, phaBypassEnable, ppaClockEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, phaBypassEnable, ppaClockEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxEgressBypassModeGet function
* @endinternal
*
* @brief   Get bypass enable/disable of the egress processing and is the PPA clock
*         disable for power saving when 'bypass' the egress processing.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] phaBypassEnablePtr       - (pointer to)
*                                      GT_TRUE  -     bypass the programmable header alteration (PHA).
*                                      GT_FALSE - not bypass the programmable header alteration (PHA).
* @param[out] ppaClockEnablePtr        - (pointer to)
*                                      GT_TRUE  - the PPA clock is enabled.
*                                      GT_FALSE - the PPA clock is disabled.(for power saving).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxEgressBypassModeGet
(
    IN  GT_SW_DEV_NUM     devNum,
    OUT GT_BOOL          *phaBypassEnablePtr,
    OUT GT_BOOL          *ppaClockEnablePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;    /* register address to set */
    GT_U32 value;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(phaBypassEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(ppaClockEnablePtr);

    regAddr = PRV_PX_REG1_UNIT_PHA_MAC(devNum).pha_regs.PHACtrl;

    /*<PHA Bypass>*/
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),regAddr,1,1,&value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *phaBypassEnablePtr = BIT2BOOL_MAC(value);

    /*<Disable PPA Clock>*/
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),regAddr,2,1,&value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *ppaClockEnablePtr = BIT2BOOL_MAC(1-value);


    return GT_OK;
}

/**
* @internal cpssPxEgressBypassModeGet function
* @endinternal
*
* @brief   Get bypass enable/disable of the egress processing and is the PPA clock
*         disable for power saving when 'bypass' the egress processing.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] phaBypassEnablePtr       - (pointer to)
*                                      GT_TRUE  -     bypass the programmable header alteration (PHA).
*                                      GT_FALSE - not bypass the programmable header alteration (PHA).
* @param[out] ppaClockEnablePtr        - (pointer to)
*                                      GT_TRUE  - the PPA clock is enabled.
*                                      GT_FALSE - the PPA clock is disabled.(for power saving).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressBypassModeGet
(
    IN  GT_SW_DEV_NUM     devNum,
    OUT GT_BOOL          *phaBypassEnablePtr,
    OUT GT_BOOL          *ppaClockEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxEgressBypassModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, phaBypassEnablePtr, ppaClockEnablePtr));

    rc = internal_cpssPxEgressBypassModeGet(devNum, phaBypassEnablePtr, ppaClockEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, phaBypassEnablePtr, ppaClockEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxEgressTimestampModeSet function
* @endinternal
*
* @brief   Set if the time of day (TOD) information used instead of source and target port
*         configuration for specific packet type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] packetType               - the packet type (that was assigned by ingress processing).
*                                      (APPLICABLE RANGE: 0..31)
* @param[in] useTod                   - GT_TRUE  - the time of day (TOD) information instead of .
*                                      GT_FALSE - the source and target port information instead of the time of day (TOD).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxEgressTimestampModeSet
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_PACKET_TYPE  packetType,
    IN GT_BOOL              useTod
)
{
    GT_STATUS rc;
    GT_U32 regAddr;    /* register address to set */
    GT_U32 value;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PACKET_TYPE_CHECK_MAC(packetType);

    regAddr = PRV_PX_REG1_UNIT_PHA_MAC(devNum).pha_regs.egrTimestampConfig;

    /*<Packet Type i Egress Timestamp Enable >*/
    value = BOOL2BIT_MAC(useTod);
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),regAddr,packetType,1,value);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxEgressTimestampModeSet function
* @endinternal
*
* @brief   Set if the time of day (TOD) information used instead of source and target port
*         configuration for specific packet type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] packetType               - the packet type (that was assigned by ingress processing).
*                                      (APPLICABLE RANGE: 0..31)
* @param[in] useTod                   - GT_TRUE  - the time of day (TOD) information instead of .
*                                      GT_FALSE - the source and target port information instead of the time of day (TOD).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressTimestampModeSet
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_PACKET_TYPE  packetType,
    IN GT_BOOL              useTod
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxEgressTimestampModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetType, useTod));

    rc = internal_cpssPxEgressTimestampModeSet(devNum, packetType, useTod);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetType, useTod));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxEgressTimestampModeGet function
* @endinternal
*
* @brief   Get if the time of day (TOD) information used instead of source and target port
*         configuration for specific packet type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] packetType               - the packet type (that was assigned by ingress processing).
*                                      (APPLICABLE RANGE: 0..31)
*
* @param[out] useTodPtr                - (pointer to)
*                                      GT_TRUE  - the time of day (TOD) information instead of .
*                                      GT_FALSE - the source and target port information instead of the time of day (TOD).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxEgressTimestampModeGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_PACKET_TYPE  packetType,
    OUT GT_BOOL             *useTodPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;    /* register address to set */
    GT_U32 value;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PACKET_TYPE_CHECK_MAC(packetType);
    CPSS_NULL_PTR_CHECK_MAC(useTodPtr);

    regAddr = PRV_PX_REG1_UNIT_PHA_MAC(devNum).pha_regs.egrTimestampConfig;

    /*<Packet Type i Egress Timestamp Enable >*/
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),regAddr,packetType,1,&value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *useTodPtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssPxEgressTimestampModeGet function
* @endinternal
*
* @brief   Get if the time of day (TOD) information used instead of source and target port
*         configuration for specific packet type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] packetType               - the packet type (that was assigned by ingress processing).
*                                      (APPLICABLE RANGE: 0..31)
*
* @param[out] useTodPtr                - (pointer to)
*                                      GT_TRUE  - the time of day (TOD) information instead of .
*                                      GT_FALSE - the source and target port information instead of the time of day (TOD).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressTimestampModeGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_PACKET_TYPE  packetType,
    OUT GT_BOOL             *useTodPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxEgressTimestampModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetType, useTodPtr));

    rc = internal_cpssPxEgressTimestampModeGet(devNum, packetType, useTodPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetType, useTodPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal haEntry_build2wordsFrom802_1brETagParams function
* @endinternal
*
* @brief   check that ETag parameters are valid.
*         haEntry : build 2 words of ETAG from the info
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] etagPtr                  - (pointer to) the ETag info.
*
* @param[out] dataPtr[/2/]             - the 2 words that hold the ETag format
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS haEntry_build2wordsFrom802_1brETagParams
(
    IN  CPSS_802_1BR_ETAG_STC *etagPtr,
    OUT GT_U32                 dataPtr[/*2*/]
)
{
    CPSS_NULL_PTR_CHECK_MAC(etagPtr);

    CPSS_DATA_CHECK_MAX_MAC(etagPtr->TPID              ,BIT_16     );
    CPSS_DATA_CHECK_MAX_MAC(etagPtr->E_PCP             ,BIT_3      );
    CPSS_DATA_CHECK_MAX_MAC(etagPtr->E_DEI             ,BIT_1      );
    CPSS_DATA_CHECK_MAX_MAC(etagPtr->Ingress_E_CID_base,BIT_12     );
    CPSS_DATA_CHECK_MAX_MAC(etagPtr->Direction         ,BIT_1      );
    CPSS_DATA_CHECK_MAX_MAC(etagPtr->Upstream_Specific ,BIT_1      );
    CPSS_DATA_CHECK_MAX_MAC(etagPtr->GRP               ,BIT_2      );
    CPSS_DATA_CHECK_MAX_MAC(etagPtr->E_CID_base        ,BIT_12     );
    CPSS_DATA_CHECK_MAX_MAC(etagPtr->Ingress_E_CID_ext ,BIT_8      );
    CPSS_DATA_CHECK_MAX_MAC(etagPtr->E_CID_ext         ,BIT_8      );


    dataPtr[0] =  etagPtr->E_PCP  << 29 |
                  etagPtr->E_DEI  << 28 |
                  etagPtr->Ingress_E_CID_base << 16 |
                  etagPtr->Direction << 15 |
                  etagPtr->Upstream_Specific << 14 |
                  etagPtr->GRP << 12 |
                  etagPtr->E_CID_base;

    dataPtr[1] =  etagPtr->Ingress_E_CID_ext << 24 |
                  etagPtr->E_CID_ext << 16 |
                  etagPtr->TPID;                /* The field is NOT ETAG field - only for regression purpose */

    return GT_OK;
}

/**
* @internal haEntry_build2wordsFrom802_1brPcidParams function
* @endinternal
 *
* @brief   check that PCID values are valid.
*   haEntry : build 2 words table info
 *
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
 *
* @param[in] pcidPtr               - (pointer to) PCIDs info.
* @param[out] dataPtr[/2/]         - the 2 words that hold PCIDs
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS haEntry_build2wordsFromPcidParams
(
    IN  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC *pcidPtr,
    OUT GT_U32                                                                               dataPtr[/*2*/]
)
{
    GT_U32 i;

    CPSS_NULL_PTR_CHECK_MAC(pcidPtr);

    for (i=0; i < 3; i++)
    {
        CPSS_DATA_CHECK_MAX_MAC(pcidPtr->pcid[i], BIT_12);
    }
    dataPtr[0] = (pcidPtr->pcid[0] << 20)  | (pcidPtr->pcid[1] << 8);
    dataPtr[1] =  pcidPtr->pcid[2] << 20;
    return GT_OK;
}
/**
* @internal haEntry_build4wordsFrom802_1brPcidParams function
* @endinternal
 *
* @brief   check that PCID values are valid.
*   haEntry : build 4 words table info
 *
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pcidPtr               - (pointer to) PCIDs info.
* @param[out] dataPtr[/4/]         - the 4 words that hold PCIDs
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS haEntry_build4wordsFromPcidParams
(
    IN  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC *pcidPtr,
    OUT GT_U32                                                                               dataPtr[/*4*/]
)
{
    GT_U32 i;

    CPSS_NULL_PTR_CHECK_MAC(pcidPtr);

    for (i=0; i < 7; i++)
    {
        CPSS_DATA_CHECK_MAX_MAC(pcidPtr->pcid[i], BIT_12);
    }
    dataPtr[0] = (pcidPtr->pcid[0] << 20)  | (pcidPtr->pcid[1] << 8);
    dataPtr[1] = (pcidPtr->pcid[2] << 20)  | (pcidPtr->pcid[3] << 8);
    dataPtr[2] = (pcidPtr->pcid[4] << 20)  | (pcidPtr->pcid[5] << 8);
    dataPtr[3] = pcidPtr->pcid[6] << 20;

    return GT_OK;
}

/**
* @internal haEntry_buildSingleWordFromDsaFrwParams function
* @endinternal
*
* @brief   check that DSA-forward parameters are valid.
*         haEntry : build 1 word of DSA-forward from the info
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dsaFrwPtr                - (pointer to) the DSA FRW info.
*
* @param[out] dataPtr[/1/]             - the 1 word that hold the DSA format
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS haEntry_buildSingleWordFromDsaFrwParams
(
    IN  CPSS_PX_REGULAR_DSA_FORWARD_STC *dsaFrwPtr,
    OUT GT_U32                 dataPtr[/*1*/]
)
{
    GT_U32  TagCommand = 3;

    CPSS_DATA_CHECK_MAX_MAC(dsaFrwPtr->hwSrcDev         ,BIT_5      );
    CPSS_DATA_CHECK_MAX_MAC(dsaFrwPtr->srcPortOrTrunk   ,BIT_5      );
    CPSS_DATA_CHECK_MAX_MAC(dsaFrwPtr->cfi              ,BIT_1      );
    CPSS_DATA_CHECK_MAX_MAC(dsaFrwPtr->up               ,BIT_3      );
    CPSS_DATA_CHECK_MAX_MAC(dsaFrwPtr->vid              ,BIT_12     );

    dataPtr[0] =  TagCommand << 30 |
                  BOOL2BIT_MAC(dsaFrwPtr->srcTagged) << 29 |
                  dsaFrwPtr->hwSrcDev << 24 |
                  dsaFrwPtr->srcPortOrTrunk << 19 |
                  BOOL2BIT_MAC(dsaFrwPtr->srcIsTrunk) << 18 |
                  dsaFrwPtr->cfi << 16 |
                  dsaFrwPtr->up << 13 |
                  dsaFrwPtr->vid;

    return GT_OK;
}

/**
* @internal haEntry_buildDataFromEDsaFrwParams function
* @endinternal
*
* @brief  Check that eDSA-forward parameters are valid.
*         haEntry : build words of eDSA-forward from the info
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] eDsaFrwPtr            - (pointer to) the eDSA FRW info.
* @param[in] operationType         - operation type
*
* @param[out] dataPtr[/4/]         - the 4 words that hold the DSA format
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on bad template fields
*/
static GT_STATUS haEntry_buildDataFromEDsaFrwParams
(
    IN  CPSS_PX_EDSA_FORWARD_STC    *eDsaFrwPtr,
    IN  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operationType,
    OUT GT_U32                       dataPtr[/*4*/]
)
{
    GT_U32 TagCommand = 3;
    GT_U32 extended = 1;
    GT_U32 srcTag0IsOuterTag = 0;
    GT_U32 phySrcMcFilterEn;
    GT_U32 useVidx;

    CPSS_NULL_PTR_CHECK_MAC(eDsaFrwPtr);
    CPSS_DATA_CHECK_MAX_MAC(eDsaFrwPtr->hwSrcDev         ,BIT_10     );
    CPSS_DATA_CHECK_MAX_MAC(eDsaFrwPtr->tpIdIndex        ,BIT_3      );

    switch (operationType)
    {
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E       :
            if (eDsaFrwPtr->srcTagged       != GT_FALSE ||
                eDsaFrwPtr->tag1SrcTagged   != GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "eDsa untagged packet expected to be "
                                                            "srcTagged[0] and tag1SrcTagged[0] "
                                                            "but got srcTagged[%d] tag1SrcTagged[%d]",
                                              eDsaFrwPtr->srcTagged, eDsaFrwPtr->tag1SrcTagged);
            }
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E    :
            if (eDsaFrwPtr->srcTagged       != GT_FALSE ||
                eDsaFrwPtr->tag1SrcTagged   != GT_TRUE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "eDsa Tag1 source tagged packet expected to be "
                                                            "srcTagged[0] and tag1SrcTagged[1] "
                                                            "but got srcTagged[%d] tag1SrcTagged[%d]",
                                              eDsaFrwPtr->srcTagged, eDsaFrwPtr->tag1SrcTagged);
            }
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E         :
            if (eDsaFrwPtr->srcTagged       != GT_TRUE ||
                eDsaFrwPtr->tag1SrcTagged   != GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "eDsa tagged packet expected to be "
                                                            "srcTagged[1] and tag1SrcTagged[0] "
                                                            "but got srcTagged[%d] tag1SrcTagged[%d]",
                                              eDsaFrwPtr->srcTagged, eDsaFrwPtr->tag1SrcTagged);
            }
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E:
            if (eDsaFrwPtr->srcTagged       != GT_TRUE ||
                eDsaFrwPtr->tag1SrcTagged   != GT_TRUE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "eDsa tagged packet expected to be "
                                                            "srcTagged[1] and tag1SrcTagged[1] "
                                                            "but got srcTagged[%d] tag1SrcTagged[%d]",
                                              eDsaFrwPtr->srcTagged, eDsaFrwPtr->tag1SrcTagged);
            }
            /* <srcTag0IsOuterTag> : JIRA : CPSS-8783 : PIPE : <srcTag0IsOuterTag> in eDSA template
                                should always be set to 1 internally by the CPSS

               but actually should be set to 1 only when 'tag0+tag1'
            */
            srcTag0IsOuterTag = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(operationType);
    }

    phySrcMcFilterEn = 1;/* JIRA : CPSS-8814  : PIPE : <phySrcMcFilterEn> in the eDSA tag should be set to 1 , to allow src port/trunk filtering */
    useVidx = 1;/* as part of CPSS-8814 <useVidx> must be 1 , otherwise <phySrcMcFilterEn> ignored . */

    dataPtr[0] =  TagCommand << 30 |
                  BOOL2BIT_MAC(eDsaFrwPtr->srcTagged) << 29 |
                  U32_GET_FIELD_MAC(eDsaFrwPtr->hwSrcDev, 0, 5) << 24 |
                  extended << 12;
    dataPtr[1] =  extended << 31 |
                  useVidx  << 12 ;
    dataPtr[2] =  extended << 31 | U32_GET_FIELD_MAC(eDsaFrwPtr->hwSrcDev, 5, 5) << 14 | eDsaFrwPtr->tpIdIndex;
    dataPtr[3] =  BOOL2BIT_MAC(eDsaFrwPtr->tag1SrcTagged) << 26 |
                  srcTag0IsOuterTag << 25 |
                  phySrcMcFilterEn  << 6  ;

    return GT_OK;
}

/**
* @internal haEntry_build4WordFromQcnParams function
* @endinternal
*
* @brief   check that QCN parameters are valid.
*         haEntry : build 4 words of QNC from the info
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] dataPtr[/4/]             - the 4 words that hold the QCN format
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS haEntry_build4WordFromQcnParams
(
    IN  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC *qcnPtr,
    OUT GT_U32                 dataPtr[/*4*/]
)
{
    GT_U32  TagCommand = 3;
    GT_U32  dsaWords[2];
    CPSS_PX_EXTENDED_DSA_FORWARD_STC *dsaExtFrwPtr = &qcnPtr->dsaExtForward;
    GT_U32  vidxOrDevPortInfo = 0;

    CPSS_DATA_CHECK_MAX_MAC(dsaExtFrwPtr->hwSrcDev         ,BIT_5      );
    CPSS_DATA_CHECK_MAX_MAC(dsaExtFrwPtr->srcPortOrTrunk   ,BIT_7      );
    CPSS_DATA_CHECK_MAX_MAC(dsaExtFrwPtr->cfi              ,BIT_1      );
    CPSS_DATA_CHECK_MAX_MAC(dsaExtFrwPtr->up               ,BIT_3      );
    CPSS_DATA_CHECK_MAX_MAC(dsaExtFrwPtr->vid              ,BIT_12     );
    CPSS_DATA_CHECK_MAX_MAC(dsaExtFrwPtr->srcId            ,BIT_5      );
    CPSS_DATA_CHECK_MAX_MAC(dsaExtFrwPtr->qosProfileIndex  ,BIT_7      );

    if(dsaExtFrwPtr->useVidx == GT_FALSE)
    {
        CPSS_DATA_CHECK_MAX_MAC(dsaExtFrwPtr->trgPort      ,BIT_6      );
        CPSS_DATA_CHECK_MAX_MAC(dsaExtFrwPtr->hwTrgDev     ,BIT_5      );

        vidxOrDevPortInfo = dsaExtFrwPtr->trgPort << 5 |
                            dsaExtFrwPtr->hwTrgDev ;
    }
    else
    {
        CPSS_DATA_CHECK_MAX_MAC(dsaExtFrwPtr->trgVidx     ,BIT_12      );

        vidxOrDevPortInfo = dsaExtFrwPtr->trgVidx;
    }


    dsaWords[0] =  TagCommand << 30 |
                  BOOL2BIT_MAC(dsaExtFrwPtr->srcTagged) << 29 |
                  dsaExtFrwPtr->hwSrcDev << 24 |
                  (dsaExtFrwPtr->srcPortOrTrunk & 0x1f) << 19 |
                  BOOL2BIT_MAC(dsaExtFrwPtr->srcIsTrunk) << 18 |
                  dsaExtFrwPtr->cfi << 16 |
                  dsaExtFrwPtr->up << 13 |
                  1 << 12 | /*extended*/
                  dsaExtFrwPtr->vid;
    dsaWords[1] = (dsaExtFrwPtr->srcPortOrTrunk >> 5) << 29 |
                  BOOL2BIT_MAC(dsaExtFrwPtr->egrFilterRegistered) << 28 |
                  BOOL2BIT_MAC(dsaExtFrwPtr->dropOnSource) << 27 |
                  BOOL2BIT_MAC(dsaExtFrwPtr->packetIsLooped) << 26 |
                  BOOL2BIT_MAC(dsaExtFrwPtr->wasRouted) << 25 |
                  dsaExtFrwPtr->srcId << 20 |
                  dsaExtFrwPtr->qosProfileIndex << 13 |
                  BOOL2BIT_MAC(dsaExtFrwPtr->useVidx) << 12 |
                  vidxOrDevPortInfo;


    dataPtr[0] =
        qcnPtr->macSa.arEther[0] << 24 |
        qcnPtr->macSa.arEther[1] << 16 |
        qcnPtr->macSa.arEther[2] <<  8 |
        qcnPtr->macSa.arEther[3] <<  0 ;

    dataPtr[1] =
        qcnPtr->macSa.arEther[4] << 24 |
        qcnPtr->macSa.arEther[5] << 16 |
        dsaWords[0] >> 16;

    dataPtr[2] =
        dsaWords[0] << 16 |
        dsaWords[1] >> 16;

    dataPtr[3] =
        dsaWords[1] << 16 |
        qcnPtr->cnmTpid;

    return GT_OK;
}

/**
* @internal haEntry_build2WordFromEvbQcnParams function
* @endinternal
*
* @brief   Build 2 words of EVB QCN from the QCN EVB info
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] dataPtr[/2/]         - the 2 upper words of HA entry that hold the EVB QCN format
*
*/
static GT_VOID haEntry_build2WordFromEvbQcnParams
(
    IN  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC *qcnEvbPtr,
    OUT GT_U32                 dataPtr[/*2*/]
)
{
    dataPtr[0] =
        qcnEvbPtr->macSa.arEther[0] << 24 |
        qcnEvbPtr->macSa.arEther[1] << 16 |
        qcnEvbPtr->macSa.arEther[2] <<  8 |
        qcnEvbPtr->macSa.arEther[3] <<  0 ;

    dataPtr[1] =
        qcnEvbPtr->macSa.arEther[4] << 24 |
        qcnEvbPtr->macSa.arEther[5] << 16 |
        qcnEvbPtr->qcnTpid;
}

/**
* @internal haEntry_build2wordsFromPreDaParams function
* @endinternal
 *
* @brief   check Pre MAC DA values validity.
*          Build 2 words of pre MAC DA data from pre MAC DA info.
 *
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
 *
* @param[in] preDaPtr              - (pointer to) pre MAC DA info.
* @param[out] dataPtr[/2/]         - the 2 words hold pre MAC DA data
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS haEntry_build2wordsFromPreDaParams
(
    IN  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC *preDaPtr,
    OUT GT_U32                                                     dataPtr[/*2*/]
)
{
    CPSS_NULL_PTR_CHECK_MAC(preDaPtr);

    CPSS_DATA_CHECK_MAX_MAC(preDaPtr->messageType                   ,BIT_4 );
    CPSS_DATA_CHECK_MAX_MAC(preDaPtr->ptpVersion                    ,BIT_4 );
    CPSS_DATA_CHECK_MAX_MAC(preDaPtr->domainNumber                  ,BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(preDaPtr->udpDestPort                   ,BIT_16);
    CPSS_DATA_CHECK_MAX_MAC(preDaPtr->ipv4ProtocolIpv6NextHeader    ,BIT_8 );

    dataPtr[0] =    preDaPtr->udpDestPort << 16 |
                    preDaPtr->domainNumber << 8 |
                    preDaPtr->ptpVersion << 4 |
                    preDaPtr->messageType;

    dataPtr[1] =    preDaPtr->ipv4ProtocolIpv6NextHeader;

    return GT_OK;
}

/**
* @internal haEntry_convertDataToPreDaParams function
* @endinternal
*
* @brief  Converts data of pre MAC DA thread from two words of haEntry to pre MAC DA structure
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] data                  - the 2 words that hold the pre MAC DA format (from the haEntry)
*
* @param[out] preDaPtr             - (pointer to) the pre MAC DA structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS haEntry_convertDataToPreDaParams
(
    IN  GT_U32                                                     data[/*2*/],
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC  *preDaPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(preDaPtr);

    preDaPtr->messageType   =  data[0] & 0xF;
    preDaPtr->ptpVersion    = (data[0] >>  4) & 0xF;
    preDaPtr->domainNumber  = (data[0] >>  8) & 0xFF;
    preDaPtr->udpDestPort   = (data[0] >> 16) & 0xFFFF;

    preDaPtr->ipv4ProtocolIpv6NextHeader
                            = data[1] & 0xFF;

    return GT_OK;
}

/**
* @internal haEntry_convert2wordsTo802_1brETagParams function
* @endinternal
*
* @brief   convert 2 words of ETAG from haEntry to ETAG parameters
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dataPtr[/2/]             - the 2 words that hold the ETAG format (from the haEntry)
*
* @param[out] etagPtr                  - (pointer to) the ETAG info.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS haEntry_convert2wordsTo802_1brETagParams(
    IN GT_U32   dataPtr[/*2*/],
    OUT CPSS_802_1BR_ETAG_STC *etagPtr
)
{
   etagPtr->E_PCP = (dataPtr[0] >> 29) & 0x7;
   etagPtr->E_DEI = (dataPtr[0] >> 28) & 0x1;
   etagPtr->Ingress_E_CID_base = (dataPtr[0] >> 16) & 0xFFF;
   etagPtr->Direction         = (dataPtr[0] >> 15) & 0x1;
   etagPtr->Upstream_Specific = (dataPtr[0] >> 14) & 0x1;
   etagPtr->GRP        = (dataPtr[0] >> 12) & 0x3;
   etagPtr->E_CID_base = dataPtr[0] & 0xFFF;
   etagPtr->Ingress_E_CID_ext = (dataPtr[1] >> 24) & 0xFF;
   etagPtr->E_CID_ext  = (dataPtr[1] >> 16) & 0xFF;
   etagPtr->TPID  = dataPtr[1] & 0xFFFF;        /* The field is NOT ETAG field - only for regression purpouse */

   return GT_OK;
}

/**
* @internal haEntry_convert2wordsTo802_1brPcidParams function
* @endinternal
 *
* @brief   convert 2 words from haEntry to PCID array
 *
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dataPtr[/2/]          - the 2 words that hold the
*       PCIDs(from the haEntry)
* @param[out] pcidPtr              - (pointer to) PCID array.
* @retval GT_OK                    - on success
*/
static GT_STATUS haEntry_convert2wordsTo802_1brPcidParams(
    IN GT_U32                                                                                dataPtr[/*2*/],
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC *pcidPtr
)
{
    pcidPtr->pcid[0] = (dataPtr[0] & 0xFFF00000) >> 20;
    pcidPtr->pcid[1] = (dataPtr[0] & 0xFFF00) >> 8;
    pcidPtr->pcid[2] = (dataPtr[1] & 0xFFF00000) >> 20;


   return GT_OK;
}

/**
* @internal haEntry_convert4wordsTo802_1brPcidParams function
* @endinternal
 *
* @brief   convert 4 words from haEntry to PCID array
 *
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dataPtr[/4/]          - the 4 words that hold the
*       PCIDs(from the haEntry)
* @param[out] pcidPtr              - (pointer to) PCID array.
* @retval GT_OK                    - on success
*/
static GT_STATUS haEntry_convert4wordsTo802_1brPcidParams(
    IN GT_U32                                                                                dataPtr[/*4*/],
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC *pcidPtr
)
{
    pcidPtr->pcid[0] = (dataPtr[0] & 0xFFF00000) >> 20;
    pcidPtr->pcid[1] = (dataPtr[0] & 0xFFF00) >> 8;
    pcidPtr->pcid[2] = (dataPtr[1] & 0xFFF00000) >> 20;
    pcidPtr->pcid[3] = (dataPtr[1] & 0xFFF00) >> 8;
    pcidPtr->pcid[4] = (dataPtr[2] & 0xFFF00000) >> 20;
    pcidPtr->pcid[5] = (dataPtr[2] & 0xFFF00) >> 8;
    pcidPtr->pcid[6] = (dataPtr[3] & 0xFFF00000) >> 20;

   return GT_OK;
}

/**
* @internal haEntry_convertSingleWordToDsaFrwParams function
* @endinternal
*
* @brief   convert word of DSA-forward from haEntry to DSA-forward parameters
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] data                     - the 1 word that hold the DSA format (from the haEntry)
*
* @param[out] dsaFrwPtr                - (pointer to) the DSA FRW info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
static GT_STATUS haEntry_convertSingleWordToDsaFrwParams(
    IN GT_U32   data,
    OUT CPSS_PX_REGULAR_DSA_FORWARD_STC *dsaFrwPtr
)
{
    GT_U32  TagCommand = data >> 30;
    GT_U32  extended = (data >> 12) & 0x1;

    if(TagCommand != 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "TagCommand expected to be 'FORWARD'(3) but got [%d]",
            TagCommand);
    }

    if(extended)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "extended expected to be 0 but got 1");
    }


    dsaFrwPtr->srcTagged        = BOOL2BIT_MAC(((data >> 29) & 1));
    dsaFrwPtr->hwSrcDev         = (data >> 24) & 0x1f;
    dsaFrwPtr->srcPortOrTrunk   = (data >> 19) & 0x1f;
    dsaFrwPtr->srcIsTrunk       = BOOL2BIT_MAC(((data >> 18) & 1));
    dsaFrwPtr->cfi              = BOOL2BIT_MAC(((data >> 16) & 1));
    dsaFrwPtr->up               = (data >> 13) & 0x7;
    dsaFrwPtr->vid              = (data >>  0) & 0xFFF;

    return GT_OK;
}

/**
* @internal haEntry_convertDataToEDsaFrwParams function
* @endinternal
*
* @brief  Convert data of eDSA-forward from haEntry to eDSA-forward parameters
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] data                  - the 4 words that hold the eDSA format (from the haEntry)
*
* @param[out] eDsaFrwPtr           - (pointer to) the eDSA FRW info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on wrong eDsa tag fields
*/
static GT_STATUS haEntry_convertDataToEDsaFrwParams(
    IN GT_U32   data[],
    OUT CPSS_PX_EDSA_FORWARD_STC *eDsaFrwPtr
)
{
    GT_U32  tagCommand = data[3] >> 30;
    GT_BOOL extended = (U32_GET_FIELD_MAC(data[3], 12, 1) == 1 &&
                        U32_GET_FIELD_MAC(data[2], 31, 1) == 1 &&
                        U32_GET_FIELD_MAC(data[1], 31, 1) == 1 &&
                        U32_GET_FIELD_MAC(data[0], 31, 1) == 0);

    CPSS_NULL_PTR_CHECK_MAC(eDsaFrwPtr);

    if(tagCommand != 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "TagCommand expected to be 'FORWARD'(3) but got [%d]",
            tagCommand);
    }

    if(extended == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "eDSA expected to be 'extended', but got NOT 'extended'");
    }


    eDsaFrwPtr->srcTagged        = BOOL2BIT_MAC(((data[3] >> 29) & 1));
    eDsaFrwPtr->hwSrcDev         = U32_GET_FIELD_MAC(data[3], 24, 5) | U32_GET_FIELD_MAC(data[1], 14, 5) << 5;
    eDsaFrwPtr->tpIdIndex        = data[1] & 0x7;
    eDsaFrwPtr->tag1SrcTagged    = BOOL2BIT_MAC(((data[0] >> 26) & 1));

    return GT_OK;
}

/**
* @internal haEntry_convert4WordToQcnParams function
* @endinternal
*
* @brief   convert 4 words of QCN from haEntry to QCN parameters
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dataPtr[/4/]             - the 4 words that hold the QCN format (from the haEntry)
*
* @param[out] qcnPtr                   - (pointer to) the QCN info.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS haEntry_convert4WordToQcnParams(
    IN  GT_U32                 dataPtr[/*4*/],
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC *qcnPtr
)
{
    GT_U32 dsaWords[2];
    GT_U32  TagCommand;
    GT_U32  extended;
    CPSS_PX_EXTENDED_DSA_FORWARD_STC *dsaExtFrwPtr = &qcnPtr->dsaExtForward;

    qcnPtr->macSa.arEther[0] = (GT_U8)(dataPtr[0] >> 24);
    qcnPtr->macSa.arEther[1] = (GT_U8)(dataPtr[0] >> 16);
    qcnPtr->macSa.arEther[2] = (GT_U8)(dataPtr[0] >>  8);
    qcnPtr->macSa.arEther[3] = (GT_U8)(dataPtr[0] >>  0);
    qcnPtr->macSa.arEther[4] = (GT_U8)(dataPtr[1] >> 24);
    qcnPtr->macSa.arEther[5] = (GT_U8)(dataPtr[1] >> 16);

    dsaWords[0] = (dataPtr[1] & 0xFFFF) << 16 |
                   dataPtr[2] >> 16;
    dsaWords[1] = (dataPtr[2] & 0xFFFF) << 16 |
                   dataPtr[3] >> 16;

    TagCommand = dsaWords[0] >> 30;
    if(TagCommand != 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "TagCommand expected to be 'FORWARD'(3) but got [%d]",
            TagCommand);
    }

    extended = (dsaWords[0] >> 12) & 0x1;
    if(extended == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "extended word 0 expected to be 1 but got 0");
    }

    extended = (dsaWords[1] >> 31) & 0x1;
    if(extended == 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "extended word 1 expected to be 0 but got 1");
    }



    dsaExtFrwPtr->srcTagged      = BIT2BOOL_MAC((dsaWords[0] >> 29) & 1);
    dsaExtFrwPtr->hwSrcDev       = (dsaWords[0] >> 24) & 0x1f;
    dsaExtFrwPtr->srcPortOrTrunk = (dsaWords[0] >> 19) & 0x1f;
    dsaExtFrwPtr->srcIsTrunk     = BIT2BOOL_MAC((dsaWords[0] >> 18) & 1);
    dsaExtFrwPtr->cfi            = (dsaWords[0] >> 16) & 0x1;
    dsaExtFrwPtr->up             = (dsaWords[0] >> 13) & 0x7;
    dsaExtFrwPtr->vid            = dsaWords[0] & 0xfff;


    dsaExtFrwPtr->srcPortOrTrunk     |= ((dsaWords[1] >> 29) & 0x3) << 5;
    dsaExtFrwPtr->egrFilterRegistered = BIT2BOOL_MAC((dsaWords[1] >> 28) & 1);
    dsaExtFrwPtr->dropOnSource        = BIT2BOOL_MAC((dsaWords[1] >> 27) & 1);
    dsaExtFrwPtr->packetIsLooped      = BIT2BOOL_MAC((dsaWords[1] >> 26) & 1);
    dsaExtFrwPtr->wasRouted           = BIT2BOOL_MAC((dsaWords[1] >> 25) & 1);
    dsaExtFrwPtr->srcId               = (dsaWords[1] >> 20) & 0x1f;
    dsaExtFrwPtr->qosProfileIndex     = (dsaWords[1] >> 13) & 0x7f;
    dsaExtFrwPtr->useVidx             = BIT2BOOL_MAC((dsaWords[1] >> 12) & 1);

    if(dsaExtFrwPtr->useVidx)
    {
        dsaExtFrwPtr->trgVidx         = dsaWords[1] & 0xfff;
    }
    else
    {
        dsaExtFrwPtr->trgPort         = (dsaWords[1] >> 5) & 0x3f;
        dsaExtFrwPtr->hwTrgDev        = dsaWords[1] & 0x1f;
    }


    qcnPtr->cnmTpid = (GT_U16)(dataPtr[3] & 0xFFFF);

    return GT_OK;
}

/**
* @internal haEntry_convert2WordToEvbQcnParams function
* @endinternal
*
* @brief   convert 2 upper words of EVB QCN from haEntry to QCN parameters
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dataPtr[/2/]          - the 2 words that hold the EVB QCN format (from the haEntry)
*
* @param[out] qcnEvbPtr            - (pointer to) the EVB QCN info.
*
*/
static GT_VOID haEntry_convert2WordToEvbQcnParams(
    IN  GT_U32                 dataPtr[/*2*/],
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC *qcnEvbPtr
)
{
    qcnEvbPtr->macSa.arEther[0] = (GT_U8)(dataPtr[0] >> 24);
    qcnEvbPtr->macSa.arEther[1] = (GT_U8)(dataPtr[0] >> 16);
    qcnEvbPtr->macSa.arEther[2] = (GT_U8)(dataPtr[0] >>  8);
    qcnEvbPtr->macSa.arEther[3] = (GT_U8)(dataPtr[0] >>  0);
    qcnEvbPtr->macSa.arEther[4] = (GT_U8)(dataPtr[1] >> 24);
    qcnEvbPtr->macSa.arEther[5] = (GT_U8)(dataPtr[1] >> 16);
    qcnEvbPtr->qcnTpid = (GT_U16)(dataPtr[1] & 0xFFFF);
}

typedef    GT_U32   FW_THREAD_ID;
#define    THR0_DoNothing_support_slow_path   0
#define    THR1_E2U                           1
#define    THR2_U2E                           2
#define    THR3_U2C                           3
#define    THR4_ET2U                          4
#define    THR5_EU2U                          5
#define    THR6_U2E                           6
#define    THR7_Mrr2E                         7
#define    THR8_E_V2U                         8
#define    THR9_E2U                           9
#define    THR15_QCN                          15
#define    THR17_U2IPL                        17
#define    THR18_IPL2IPL                      18
#define    THR19_E2U_Untagged                 19
#define    THR20_U2E_M4                       20
#define    THR21_U2E_M8                       21
#define    THR22_Discard                      22
#define    THR23_EVB_E2U                      23
#define    THR24_EVB_U2E                      24
#define    THR25_EVB_QCN                      25
#define    THR26_PRE_DA_U2E                   26
#define    THR27_PRE_DA_E2U                   27
#define    THR46_DoNothing                    46

/* indication that thread is not valid/not implemented */
#define FW_INSTRUCTION_POINTER_NOT_VALID_CNS   0xFFFFFFFF

/* ALL addresses of instruction pointers need to be with prefix 0x00400000 */
/* the PHA table need to hold only lower 16 bits (the prefix is added internally by the HW) */
#define FW_INSTRUCTION_DOMAIN_ADDR_CNS      0x00400000
#define FW_INSTRUCTION_DOMAIN_ADDR_MAX_CNS  0x0040FFFF

/* max number of threads that the firmware supports */
#define MAX_THREADS_CNS 50

/*
    NOTE: the  value in pipeFirmwareThreadIdToInstructionPointer are coming from the
    FIRMWARE release in file : PC_addr/Threads_PC.txt
    MUST be aligned with     : FW_Vers/ppa_fw_imem_addr_data.h

    current FIRMWARE         : PIPE_FW_20_07_00
*/
static const GT_U32  pipeFirmwareThreadIdToInstructionPointer[MAX_THREADS_CNS] =
{
    /* 0*/    0x004007b0      /*THR0_DoNothing_support_slow_path*/
    /* 1*/   ,0x004008a0      /*THR1_E2U                        */
    /* 2*/   ,0x004009b0      /*THR2_U2E                        */
    /* 3*/   ,0x00400b10      /*THR3_U2C                        */
    /* 4*/   ,0x00400c20      /*THR4_ET2U                       */
    /* 5*/   ,0x00400d30      /*THR5_EU2U                       */
    /* 6*/   ,0x00400e30      /*THR6_U2E                        */
    /* 7*/   ,0x00400fa0      /*THR7_Mrr2E                      */
    /* 8*/   ,0x004010b0      /*THR8_E_V2U                      */
    /* 9*/   ,0x004011c0      /*THR9_E2U                        */
    /*10*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*11*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*12*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*13*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*14*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*15*/   ,0x00401930      /*THR15_QCN                       */
    /*16*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*17*/   ,0x00401ae0      /*THR17_U2IPL                     */
    /*18*/   ,0x00401be0      /*THR18_IPL2IPL                   */
    /*19*/   ,0x00401d00      /*THR19_E2U_Untagged              */
    /*20*/   ,0x00401e10      /*THR20_U2E_M4                    */
    /*21*/   ,0x00401f80      /*THR21_U2E_M8                    */
    /*22*/   ,0x00402120      /*THR22_Discard                   */
    /*23*/   ,0x00402150      /*THR23_EVB_E2U                   */
    /*24*/   ,0x00402250      /*THR24_EVB_U2E                   */
    /*25*/   ,0x00402350      /*THR25_EVB_QCN                   */
    /*26*/   ,0x004023a0      /*THR26_PRE_DA_U2E                */
    /*27*/   ,0x00402660      /*THR27_PRE_DA_E2U                */
    /*28*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*29*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*30*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*31*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*32*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*33*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*34*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*35*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*36*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*37*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*38*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*39*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*40*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*41*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*42*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*43*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*44*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*45*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*46*/   ,0x00402950      /*THR46_DoNothing                 */
    /*47*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*48*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
    /*49*/   ,FW_INSTRUCTION_POINTER_NOT_VALID_CNS
};

#ifdef ASIC_SIMULATION


extern GT_STATUS cpssSimPipeFirmwareThreadIdToInstructionPointerSet(
    IN GT_U32 cpssDevNum,
    IN GT_U32 threadId,
    IN GT_U32 firmwareInstructionPointer
);

/**
* @internal ASIC_SIMULATION_firmwareInit function
* @endinternal
*
* @brief   ASIC_SIMULATION : init the ASIC_SIMULATION for FW addresses.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on unknown threadId
*/
static GT_STATUS ASIC_SIMULATION_firmwareInit(
    IN GT_SW_DEV_NUM    devNum
)
{
    GT_U32  threadId;
    GT_STATUS   rc;

    for(threadId = 0 ; threadId < MAX_THREADS_CNS; threadId++)
    {
        if(pipeFirmwareThreadIdToInstructionPointer[threadId] == FW_INSTRUCTION_POINTER_NOT_VALID_CNS)
        {
            continue;
        }

        rc = cpssSimPipeFirmwareThreadIdToInstructionPointerSet(devNum,threadId,
            pipeFirmwareThreadIdToInstructionPointer[threadId]);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ASIC_SIMULATION failed to bind threadId[%d] with InstructionPointer[0x%x]",
                threadId,
                pipeFirmwareThreadIdToInstructionPointer[threadId]);
        }

    }

    return GT_OK;
}

#endif /*ASIC_SIMULATION*/
/**
* @internal firmwareAddrValidityCheck function
* @endinternal
*
* @brief   Check Firmware addresses.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on threadId that hold bad address.
*/
static GT_STATUS firmwareAddrValidityCheck(void)
{
    GT_U32  threadId;
    GT_U32  addr;

    for(threadId = 0 ; threadId < MAX_THREADS_CNS; threadId++)
    {
        addr = pipeFirmwareThreadIdToInstructionPointer[threadId];
        if(addr == FW_INSTRUCTION_POINTER_NOT_VALID_CNS)
        {
            continue;
        }

        if((addr & 0xFFFF0000) != FW_INSTRUCTION_DOMAIN_ADDR_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "firmware address for thread [%d] must be in [0x%x..0x%x]\n"
                "but got[0x%x] --> need to fix pipeFirmwareThreadIdToInstructionPointer[]",
                threadId ,
                FW_INSTRUCTION_DOMAIN_ADDR_CNS,
                FW_INSTRUCTION_DOMAIN_ADDR_MAX_CNS,
                addr
                );
        }
    }

    return GT_OK;
}


/**
* @internal firmwareDownload function
* @endinternal
*
* @brief   Download the PHA firmware to the device.
*         Need to download to IMEM of ALL PPGs (4 PPGs)
*         (each PPG hold 8 PPUs)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  the device number
* @param[in] firmwareDataArr[]        - array of words of firmware data.
* @param[in] firmwareDataNumWords     - number of words in firmwareDataArr[].
*                                      must be multiple of 4.
*                                      meaning (firmwareDataNumWords % 4) must be 0
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad firmwareDataNumWords
*/
static GT_STATUS firmwareDownload
(
    IN GT_SW_DEV_NUM    devNum,
    IN const GT_U32     firmwareDataArr[],
    IN GT_U32           firmwareDataNumWords
)
{
    GT_STATUS   rc;
    GT_U32  ppg;
    GT_U32  regAddr;
    GT_U32  ii;
    GT_U32  entryWidthInWords = 4;
    GT_U32  entryWidthInBytes = 4*entryWidthInWords;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        /*skip in case of sysyem recovery HA */
        return GT_OK;
    }

    if(firmwareDataNumWords % 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "firmware data number of words must me multiple of 4 but got [%d] words \n",
            firmwareDataNumWords);
    }

    if(firmwareDataNumWords > _4K)/* size of IMEM in words */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "firmware data number of words is [%d] but must NOT be more than [%d] words \n",
            firmwareDataNumWords,
            _4K);
    }

    for(ppg = 0 ; ppg < 4; ppg++)
    {
        regAddr = PRV_PX_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPG_IMEM_base_addr;

        for(ii = 0 ; ii < firmwareDataNumWords; ii += entryWidthInWords ,
            regAddr += entryWidthInBytes)
        {
            rc = prvCpssHwPpWriteRam(CAST_SW_DEVNUM(devNum),
                regAddr,entryWidthInWords,
                (GT_U32*)(&firmwareDataArr[ii]));/* casting that removes the 'const' */
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* set field <ppa_imem_hold_off> to :
        0x0 = No HoldOff; IMEM responds with data
    */
    regAddr = PRV_PX_REG1_UNIT_PHA_MAC(devNum).PPA.ppa_regs.PPACtrl;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),regAddr,0,1,
        0);/*0x0 = No HoldOff; IMEM responds with data*/
    if (rc != GT_OK)
    {
        return rc;
    }



    return GT_OK;
}

/**
* @internal prvCpssPxEgressInit function
* @endinternal
*
* @brief   init the device for Egress processing.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  the device number
* @param[in] initParamsPtr            - PX device init parameters.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on unknown threadId
*/
GT_STATUS prvCpssPxEgressInit
(
    IN GT_SW_DEV_NUM    devNum,
    IN CPSS_PX_INIT_INFO_STC  *initParamsPtr
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PX_PACKET_TYPE     packetType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operationType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  haEntry;
    GT_U32 regAddr;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (initParamsPtr->phaFirmwareType == CPSS_PX_PHA_FIRMWARE_TYPE_DEFAULT_E)
    {
        /****************************/
        /* check firmware addresses */
        /****************************/
        rc = firmwareAddrValidityCheck();
        if(rc != GT_OK)
        {
            return rc;
        }

    #ifdef ASIC_SIMULATION
        {
            /*****************************************/
            /* bind firmware addresses to simulation */
            /*****************************************/
            rc = ASIC_SIMULATION_firmwareInit(devNum);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    #endif /*ASIC_SIMULATION*/

        /***********************************/
        /* download firmware to the device */
        /***********************************/
        rc = firmwareDownload(devNum,
            ppa_fw_imem_data,           /*comes from FW_Vers/ppa_fw_imem_addr_data.h */
            sizeof(ppa_fw_imem_data)/
            sizeof(ppa_fw_imem_data[0]));
        if(rc != GT_OK)
        {
            return rc;
        }

        /****************************************************************/
        /* initialize all the PHA table entries to bind to 'do nothing' */
        /****************************************************************/
        {
            operationType = CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E;
            haEntry.notNeeded = 0;/* dummy ... not used ... don't care */

            for(portNum = 0 ; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
            {
                for(packetType = 0 ; packetType < CPSS_PX_PACKET_TYPE_MAX_CNS; packetType++)
                {
                    rc = cpssPxEgressHeaderAlterationEntrySet(
                        devNum,portNum,packetType,operationType,&haEntry);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }

        if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
        {
            /* allow PHA processing */
            rc = cpssPxEgressBypassModeSet(devNum,
                GT_FALSE,/* set to not bypass the PHA */
                GT_TRUE);/* set to use the PPA clock */
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* init needed DB */
    for(portNum = 0 ; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
    {
        PRV_CPSS_PX_PP_MAC(devNum)->port.egressSourcePortEntryTypeArr[portNum] =
            CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE___MUST_BE_LAST___E;
        PRV_CPSS_PX_PP_MAC(devNum)->port.egressTargetPortEntryTypeArr[portNum] =
            CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE___MUST_BE_LAST___E;
    }

    if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        /* Set <Generate_CNC_Default> bit. */
        regAddr = PRV_PX_REG1_UNIT_PHA_MAC(devNum).pha_regs.PHAGeneralConfig;
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, 1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/* the 4 words of the HA entry are not used by all threads beside THR15_QCN
   so we use one of those words ... index 0 .. to save the operationType
*/
#define     OPERATION_TYPE_WORD_CNS           0

/**
* @internal firmwareInstructionPointerGet function
* @endinternal
*
* @brief   convert operationType to firmwareInstructionPointer and firmwareThreadId
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] operationType            -  the operation type
*
* @param[out] firmwareInstructionPointerPtr - (pointer to) the firmware Instruction Pointer.
* @param[out] firmwareThreadIdPtr      - (pointer to) the firmware Thread Id (0..49)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on wrong parameter
*/
static GT_STATUS firmwareInstructionPointerGet
(
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT operationType,
    OUT GT_U32   *firmwareInstructionPointerPtr,
    OUT FW_THREAD_ID   *firmwareThreadIdPtr
)
{
    FW_THREAD_ID      firmwareThreadId = THR0_DoNothing_support_slow_path;

    switch(operationType)
    {
        /* 802.1br types */
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E          :
            firmwareThreadId = THR1_E2U;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E          :
            firmwareThreadId = THR2_U2E;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E           :
            firmwareThreadId = THR3_U2C;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E           :
            firmwareThreadId = THR0_DoNothing_support_slow_path;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E               :
            firmwareThreadId = THR1_E2U;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E               :
            firmwareThreadId = THR3_U2C;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E              :
            firmwareThreadId = THR17_U2IPL;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E                   :
            firmwareThreadId = THR18_IPL2IPL;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E:
            firmwareThreadId = THR19_E2U_Untagged;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E          :
            firmwareThreadId = THR20_U2E_M4;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E          :
            firmwareThreadId = THR21_U2E_M8;
            break;

        /* DSA types */
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E       :
            firmwareThreadId = THR4_ET2U;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E     :
            firmwareThreadId = THR5_EU2U;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E              :
            firmwareThreadId = THR6_U2E;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E    :
            firmwareThreadId = THR7_Mrr2E;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E      :
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E     :
            firmwareThreadId = THR8_E_V2U;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E    :
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E :
            firmwareThreadId = THR9_E2U;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E                   :
            firmwareThreadId = THR0_DoNothing_support_slow_path;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E                                         :
            firmwareThreadId = THR15_QCN;
            break;

        /* General types */
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E:
            firmwareThreadId = THR22_Discard;
            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E:
            firmwareThreadId = THR23_EVB_E2U;
            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_U2E_PACKET_E:
            firmwareThreadId = THR24_EVB_U2E;
            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E:
            firmwareThreadId = THR25_EVB_QCN;
            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_UPSTREAM_PORT_TO_EXTENDED_PORT_E:
            firmwareThreadId = THR26_PRE_DA_U2E;
            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E:
            firmwareThreadId = THR27_PRE_DA_E2U;
            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E                                   :
            firmwareThreadId = THR46_DoNothing;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, "operationType[%d] was not implemented for 'firmwareThreadId' ",
                operationType);
    }

    if(firmwareThreadId >= MAX_THREADS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "firmwareThreadId[%d] expected to be less than[%d] ",
            firmwareThreadId , MAX_THREADS_CNS);
    }

    *firmwareThreadIdPtr = firmwareThreadId;

    /* convert the 'firmwareThreadId' to the actual firmware address in memory */
    *firmwareInstructionPointerPtr =
        (pipeFirmwareThreadIdToInstructionPointer[firmwareThreadId]) &
        (~FW_INSTRUCTION_DOMAIN_ADDR_CNS);

    return GT_OK;
}

/**
* @internal firmwareInstructionPointerConvert function
* @endinternal
*
* @brief   convert firmwareInstructionPointer to firmwareThreadId
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] firmwareInstructionPointer - the firmware Instruction Pointer.
*
* @param[out] firmwareThreadIdPtr      - (pointer to) the firmware Thread Id (0..49)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on wrong parameter
*/
static GT_STATUS firmwareInstructionPointerConvert(
    IN  GT_U32   firmwareInstructionPointer,
    OUT FW_THREAD_ID   *firmwareThreadIdPtr
)
{
    FW_THREAD_ID  firmwareThreadId;

    firmwareInstructionPointer |= FW_INSTRUCTION_DOMAIN_ADDR_CNS;

    for(firmwareThreadId = 0 ; firmwareThreadId < MAX_THREADS_CNS; firmwareThreadId++)
    {
        if(firmwareInstructionPointer == pipeFirmwareThreadIdToInstructionPointer[firmwareThreadId])
        {
            *firmwareThreadIdPtr = firmwareThreadId;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
        "firmwareInstructionPointer[0x%x] was not found as valid address for known firmware thread",
        firmwareInstructionPointer);
}

/**
* @internal internal_cpssPxEgressHeaderAlterationEntrySet function
* @endinternal
*
* @brief   Set per target port and per packet type the header alteration entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - target physical port number.
* @param[in] packetType               - the packet type (that was assigned by ingress processing).
*                                      (APPLICABLE RANGE: 0..31)
* @param[in] operationType            - the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[in] operationInfoPtr         - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxEgressHeaderAlterationEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_PACKET_TYPE  packetType,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operationType,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  *operationInfoPtr
)
{
    GT_STATUS rc;
    GT_U32  entryIndex;/* index into PHA_HA_TABLE */
    GT_U32  firmwareInstructionPointer;/* the firmware Instruction Pointer */
    FW_THREAD_ID  firmwareThreadId;/* the firmware thread Id */
    GT_U32  qcnWords[4];
    GT_U32  eTagWords[2];
    GT_U32  pcidArr[7]={0};
    GT_U32  singleWordDsa;
    GT_U32  eDsaWords[4]={0,0,0,0};
    GT_U32  haEntryWords[5];/* needed 144 bits , supporting 32*5=160 bits */
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    PACKET_TYPE_CHECK_MAC(packetType);
    CPSS_NULL_PTR_CHECK_MAC(operationInfoPtr);

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        return GT_OK;
    }

     entryIndex = portNum << 5 |
                 packetType;

    cpssOsMemSet(haEntryWords,0,sizeof(haEntryWords));

    switch(operationType)
    {
        /* 802.1br types */
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E            :
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E                 :
            /* check and build specific info */
            rc = haEntry_build2wordsFrom802_1brETagParams(&operationInfoPtr->info_802_1br_E2U.eTag, &eTagWords[0]);
            if(rc != GT_OK)
            {
                return rc;
            }

            haEntryWords[3] = eTagWords[0];
            haEntryWords[2] = eTagWords[1];

            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E            :
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E             :
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E             :
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E                 :
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E         :
            rc = haEntry_build2wordsFromPcidParams(&operationInfoPtr->info_802_1br_U2E_MC,
                                                    &pcidArr[0]);
            haEntryWords[3] = pcidArr[0];
            haEntryWords[2] = pcidArr[1];
            haEntryWords[1] = 0;
            haEntryWords[0] = 0;
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E         :
            rc = haEntry_build4wordsFromPcidParams(&operationInfoPtr->info_802_1br_U2E_MC,
                                                    &pcidArr[0]);
            haEntryWords[3] = pcidArr[0];
            haEntryWords[2] = pcidArr[1];
            haEntryWords[1] = pcidArr[2];
            haEntryWords[0] = pcidArr[3];

            break;

            /* 802.1BR IPL types*/
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E                 :
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E                      :
            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E   :
            /* Check VLAN tag TPID */
            CPSS_DATA_CHECK_MAX_MAC(operationInfoPtr->info_802_1br_E2U.vlanTagTpid    ,BIT_16);

            /* Set template[63:48] - VLAN Tag TPID - override unused 'etag TPID' by VLAN Tag TPID used by the new thread */
            operationInfoPtr->info_802_1br_E2U.eTag.TPID = operationInfoPtr->info_802_1br_E2U.vlanTagTpid;
            /* check and build specific ETAG and VLAN Tag TPID info*/
            rc = haEntry_build2wordsFrom802_1brETagParams(&operationInfoPtr->info_802_1br_E2U.eTag, &eTagWords[0]);
            if(rc != GT_OK)
            {
                return rc;
            }

            haEntryWords[3] = eTagWords[0];
            haEntryWords[2] = eTagWords[1];

            break;
        /* DSA types */
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E         :
            /* check and build specific info */
            rc = haEntry_buildSingleWordFromDsaFrwParams(&operationInfoPtr->info_dsa_ET2U.dsaForward, &singleWordDsa);
            if(rc != GT_OK)
            {
                return rc;
            }

            haEntryWords[3] = singleWordDsa;

            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E       :
            /* check and build specific info */
            rc = haEntry_buildSingleWordFromDsaFrwParams(&operationInfoPtr->info_dsa_EU2U.dsaForward, &singleWordDsa);
            if(rc != GT_OK)
            {
                return rc;
            }

            haEntryWords[3] = singleWordDsa;

            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E       :
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E    :
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E         :
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E        :
            rc = haEntry_buildDataFromEDsaFrwParams(&operationInfoPtr->info_edsa_E2U.eDsaForward, operationType, &eDsaWords[0]);
            if(rc != GT_OK)
            {
                return rc;
            }

            haEntryWords[3] = eDsaWords[0];
            haEntryWords[2] = eDsaWords[1];
            haEntryWords[1] = eDsaWords[2];
            haEntryWords[0] = eDsaWords[3];

            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E                :
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E      :
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E                     :
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E                                           :
            /* check and build specific info */
            rc = haEntry_build4WordFromQcnParams(&operationInfoPtr->info_dsa_QCN, &qcnWords[0]);
            if(rc != GT_OK)
            {
                return rc;
            }

            haEntryWords[3] = qcnWords[0];
            haEntryWords[2] = qcnWords[1];
            haEntryWords[1] = qcnWords[2];
            haEntryWords[0] = qcnWords[3];

            break;

        /* General types */
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E:
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E                                     :
            break;

        /* EVB types */
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E:
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_U2E_PACKET_E:
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E:
            /* build specific info */
            haEntry_build2WordFromEvbQcnParams(&operationInfoPtr->info_evb_QCN, &qcnWords[0]);

            haEntryWords[3] = qcnWords[0];
            haEntryWords[2] = qcnWords[1];

            break;
        /* Pre-DA type */
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E:
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_UPSTREAM_PORT_TO_EXTENDED_PORT_E:
            rc = haEntry_build2wordsFromPreDaParams(&operationInfoPtr->info_pre_da_PTP, &qcnWords[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* No need words reordering for new threads */
            haEntryWords[0] = qcnWords[0];
            haEntryWords[1] = qcnWords[1];

            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(operationType);
    }

    rc = firmwareInstructionPointerGet(
        operationType,
        &firmwareInstructionPointer,
        &firmwareThreadId);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(firmwareInstructionPointer >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "firmwareInstructionPointer [0x%x] , must be limited to 16 bits",
            firmwareInstructionPointer);
    }

    if((firmwareThreadId != THR15_QCN) && (firmwareThreadId != THR25_EVB_QCN) && (firmwareThreadId != THR21_U2E_M8) && (firmwareThreadId != THR20_U2E_M4) &&
       (firmwareThreadId != THR8_E_V2U) && (firmwareThreadId != THR9_E2U) &&
       (firmwareThreadId != THR26_PRE_DA_U2E) && (firmwareThreadId != THR27_PRE_DA_E2U))
    {
        /* we have several operations that need this firmware thread .
           so we need to write it's type to HW so on the 'GET' API we can reverse
           the logic of the operation.
        */

        haEntryWords[OPERATION_TYPE_WORD_CNS] = operationType;
    }

    haEntryWords[4] = firmwareInstructionPointer;
    rc = prvCpssPxWriteTableEntry(devNum,CPSS_PX_TABLE_PHA_HA_TABLE_E,
                                  entryIndex,&haEntryWords[0]);
    return rc;
}

/**
* @internal cpssPxEgressHeaderAlterationEntrySet function
* @endinternal
*
* @brief   Set per target port and per packet type the header alteration entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - target physical port number.
* @param[in] packetType               - the packet type (that was assigned by ingress processing).
*                                      (APPLICABLE RANGE: 0..31)
* @param[in] operationType            - the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[in] operationInfoPtr         - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressHeaderAlterationEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_PACKET_TYPE  packetType,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operationType,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  *operationInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxEgressHeaderAlterationEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, packetType, operationType, operationInfoPtr));

    rc = internal_cpssPxEgressHeaderAlterationEntrySet(devNum, portNum, packetType, operationType, operationInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, packetType, operationType, operationInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxEgressHeaderAlterationEntryGet function
* @endinternal
*
* @brief   Get per target port and per packet type the header alteration entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - target physical port number.
* @param[in] packetType               - the packet type (that was assigned by ingress processing).
*                                      (APPLICABLE RANGE: 0..31)
*
* @param[out] operationTypePtr         - (pointer to) the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[out] operationInfoPtr         - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxEgressHeaderAlterationEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_PACKET_TYPE  packetType,
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   *operationTypePtr,
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  *operationInfoPtr
)
{
    GT_STATUS rc;
    GT_U32  entryIndex;/* index into PHA_HA_TABLE */
    GT_U32  firmwareInstructionPointer;/* the firmware Instruction Pointer */
    FW_THREAD_ID  firmwareThreadId;/* the firmware thread Id */
    GT_U32  qcnWords[4];
    GT_U32  eTagWords[2];
    GT_U32  pcidArr[7];
    GT_U32  preDaWords[2];
    GT_U32  singleWordDsa;
    GT_U32  haEntryWords[5];/* needed 144 bits , supporting 32*5=160 bits */
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT operationType;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    PACKET_TYPE_CHECK_MAC(packetType);
    CPSS_NULL_PTR_CHECK_MAC(operationTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(operationInfoPtr);

    entryIndex = portNum << 5 |
                 packetType;

    rc = prvCpssPxReadTableEntry(devNum,CPSS_PX_TABLE_PHA_HA_TABLE_E,
        entryIndex,&haEntryWords[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    firmwareInstructionPointer = haEntryWords[4];

    rc = firmwareInstructionPointerConvert(
        firmwareInstructionPointer,
        &firmwareThreadId);
    if(rc != GT_OK)
    {
        return rc;
    }
    if(firmwareThreadId > MAX_THREADS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "firmwareThreadId must be in [0..49] but got[%d]",
            firmwareThreadId);
    }

    operationInfoPtr->notNeeded = 0;

    if ((firmwareThreadId != THR15_QCN) &&
        (firmwareThreadId != THR8_E_V2U) &&
        (firmwareThreadId != THR9_E2U) &&
        (firmwareThreadId != THR26_PRE_DA_U2E) &&
        (firmwareThreadId != THR27_PRE_DA_E2U))
    {
        operationType = haEntryWords[OPERATION_TYPE_WORD_CNS];
    }
    else
    {
        operationType = 0;/* just to remove compiler warning */
    }

    switch (firmwareThreadId)
    {
        case THR0_DoNothing_support_slow_path:

            switch(operationType)
            {
                case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E:
                case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E        :
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "failed to retrieve operationType from the HW got unknown value[%d]",
                        operationType);
            }

            break;
        case THR1_E2U  :

            eTagWords[0] =  haEntryWords[3];
            eTagWords[1] =  haEntryWords[2];

            /* convert HW info to specific info */
            rc = haEntry_convert2wordsTo802_1brETagParams(&eTagWords[0],&operationInfoPtr->info_802_1br_E2U.eTag);
            if(rc != GT_OK)
            {
                return rc;
            }


            break;

        case THR19_E2U_Untagged  :

            eTagWords[0] =  haEntryWords[3];
            eTagWords[1] =  haEntryWords[2];

            /* convert HW info to specific info */
            rc = haEntry_convert2wordsTo802_1brETagParams(&eTagWords[0],&operationInfoPtr->info_802_1br_E2U.eTag);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* Get template[63:48] VLAN Tag TPID */
            operationInfoPtr->info_802_1br_E2U.vlanTagTpid = haEntryWords[2] & 0xFFFF;


            break;
        case THR2_U2E  :
            break;
        case THR3_U2C  :
            break;
        case THR4_ET2U :
            singleWordDsa = haEntryWords[3];

            /* convert HW info to specific info */
            rc = haEntry_convertSingleWordToDsaFrwParams(singleWordDsa,&operationInfoPtr->info_dsa_ET2U.dsaForward);
            if(rc != GT_OK)
            {
                return rc;
            }


            break;
        case THR5_EU2U :
            singleWordDsa = haEntryWords[3];

            /* convert HW info to specific info */
            rc = haEntry_convertSingleWordToDsaFrwParams(singleWordDsa,&operationInfoPtr->info_dsa_EU2U.dsaForward);
            if(rc != GT_OK)
            {
                return rc;
            }

            break;
        case THR6_U2E  :
            break;
        case THR7_Mrr2E:
            break;
        case THR8_E_V2U:
            rc = haEntry_convertDataToEDsaFrwParams(haEntryWords, &operationInfoPtr->info_edsa_E2U.eDsaForward);
            if(rc != GT_OK)
            {
                return rc;
            }
            if (operationInfoPtr->info_edsa_E2U.eDsaForward.tag1SrcTagged)
            {
                operationType = CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E;
            }
            else
            {
                operationType = CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E;
            }
            break;
        case THR9_E2U  :
            rc = haEntry_convertDataToEDsaFrwParams(haEntryWords, &operationInfoPtr->info_edsa_E2U.eDsaForward);
            if(rc != GT_OK)
            {
                return rc;
            }
            if (operationInfoPtr->info_edsa_E2U.eDsaForward.tag1SrcTagged)
            {
                operationType = CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E;
            }
            else
            {
                operationType = CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E;
            }
            break;
        case THR15_QCN :
            operationType = CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E;

            qcnWords[0] = haEntryWords[3];
            qcnWords[1] = haEntryWords[2];
            qcnWords[2] = haEntryWords[1];
            qcnWords[3] = haEntryWords[0];

            /* convert HW info to specific info */
            rc = haEntry_convert4WordToQcnParams(&qcnWords[0],&operationInfoPtr->info_dsa_QCN);
            if(rc != GT_OK)
            {
                return rc;
            }

            break;
        case THR17_U2IPL :
            break;
        case THR18_IPL2IPL  :
            break;

        case THR20_U2E_M4 :
            operationType = CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E;

            pcidArr[0] = haEntryWords[3];
            pcidArr[1] = haEntryWords[2];

            /* convert HW info to specific info */
            rc = haEntry_convert2wordsTo802_1brPcidParams(&pcidArr[0],&operationInfoPtr->info_802_1br_U2E_MC);
            if(rc != GT_OK)
            {
                return rc;
            }
            break;

        case THR21_U2E_M8 :
            operationType = CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E;

            pcidArr[0] = haEntryWords[3];
            pcidArr[1] = haEntryWords[2];
            pcidArr[2] = haEntryWords[1];
            pcidArr[3] = haEntryWords[0];

            /* convert HW info to specific info */
            rc = haEntry_convert4wordsTo802_1brPcidParams(&pcidArr[0],&operationInfoPtr->info_802_1br_U2E_MC);
            if(rc != GT_OK)
            {
                return rc;
            }
            break;

        case THR25_EVB_QCN :
            operationType = CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E;

            qcnWords[0] = haEntryWords[3];
            qcnWords[1] = haEntryWords[2];

            /* convert HW info to specific info */
            haEntry_convert2WordToEvbQcnParams(&qcnWords[0], &operationInfoPtr->info_evb_QCN);

            break;

        case THR26_PRE_DA_U2E:
        case THR27_PRE_DA_E2U:
            operationType = (firmwareThreadId == THR26_PRE_DA_U2E) ?
                CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_UPSTREAM_PORT_TO_EXTENDED_PORT_E :
                CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E;
            /* No need words reordering for new threads */
            preDaWords[0] =  haEntryWords[0];
            preDaWords[1] =  haEntryWords[1];

            rc = haEntry_convertDataToPreDaParams(preDaWords, &operationInfoPtr->info_pre_da_PTP);
            if(rc != GT_OK)
            {
                return rc;
            }

            break;
        case THR22_Discard   :
        case THR23_EVB_E2U   :
        case THR24_EVB_U2E   :
        case THR46_DoNothing :
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(firmwareThreadId);
    }
    *operationTypePtr = operationType;


    return GT_OK;
}

/**
* @internal cpssPxEgressHeaderAlterationEntryGet function
* @endinternal
*
* @brief   Get per target port and per packet type the header alteration entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - target physical port number.
* @param[in] packetType               - the packet type (that was assigned by ingress processing).
*                                      (APPLICABLE RANGE: 0..31)
*
* @param[out] operationTypePtr         - (pointer to) the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[out] operationInfoPtr         - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressHeaderAlterationEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_PACKET_TYPE  packetType,
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   *operationTypePtr,
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  *operationInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxEgressHeaderAlterationEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, packetType, operationTypePtr, operationInfoPtr));

    rc = internal_cpssPxEgressHeaderAlterationEntryGet(devNum, portNum, packetType, operationTypePtr, operationInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, packetType, operationTypePtr, operationInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxEgressSourcePortEntrySet function
* @endinternal
*
* @brief   Set per source port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - source physical port number.
* @param[in] infoType                 - the type of source port info.
* @param[in] portInfoPtr              - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxEgressSourcePortEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT infoType,
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT      *portInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32      entryData;/*the entry is single word*/
    /* 802.1br */
    CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC  *info_802_1brPtr;
    /* DSA */
    CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC      *info_dsaPtr;
    /* VLAN tag */
    CPSS_PX_VLAN_TAG_STC *vlanTagPtr;
    /* EVB */
    CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC *info_evbPtr;
    /* Pre-DA */
    CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC *info_pre_daPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC         tempSystemRecovery_Info;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(portInfoPtr);

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch(infoType)
    {
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E    :
            info_802_1brPtr =  &portInfoPtr->info_802_1br;

            CPSS_DATA_CHECK_MAX_MAC(info_802_1brPtr->pcid                ,BIT_12      );
            CPSS_DATA_CHECK_MAX_MAC(info_802_1brPtr->srcPortInfo.srcFilteringVector  ,BIT_16      );
            CPSS_DATA_CHECK_MAX_MAC(info_802_1brPtr->ptpPortInfo.ptpPortMode, CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE___MUST_BE_LAST___E);

            entryData = info_802_1brPtr->pcid  << 20 |
                        info_802_1brPtr->srcPortInfo.srcFilteringVector << 4 |
                        (BOOL2BIT_MAC (info_802_1brPtr->upstreamPort) << 3) |
                        info_802_1brPtr->ptpPortInfo.ptpPortMode;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E        :
            info_dsaPtr = &portInfoPtr->info_dsa;

            CPSS_DATA_CHECK_MAX_MAC(info_dsaPtr->srcPortNum       ,BIT_4       );
            CPSS_DATA_CHECK_MAX_MAC(info_dsaPtr->ptpPortInfo.ptpPortMode, CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE___MUST_BE_LAST___E);

            entryData = info_dsaPtr->srcPortNum << 16 |
                        info_dsaPtr->ptpPortInfo.ptpPortMode;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EDSA_E        :
            info_dsaPtr = &portInfoPtr->info_dsa;

            CPSS_DATA_CHECK_MAX_MAC(info_dsaPtr->srcPortNum       ,BIT_14       );
            CPSS_DATA_CHECK_MAX_MAC(info_dsaPtr->ptpPortInfo.ptpPortMode, CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE___MUST_BE_LAST___E);

            entryData = info_dsaPtr->srcPortNum << 18 |
                        info_dsaPtr->ptpPortInfo.ptpPortMode;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E   :
            /* Only set information on TC/OC/BC type of processing for this source port */
            CPSS_DATA_CHECK_MAX_MAC(portInfoPtr->ptpPortInfo.ptpPortMode, CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE___MUST_BE_LAST___E);
            entryData = portInfoPtr->ptpPortInfo.ptpPortMode;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E    :
            info_802_1brPtr =  &portInfoPtr->info_802_1br;
            vlanTagPtr =  &portInfoPtr->info_802_1br.srcPortInfo.vlanTag;

            CPSS_DATA_CHECK_MAX_MAC(info_802_1brPtr->pcid                ,BIT_12      );
            CPSS_DATA_CHECK_MAX_MAC(vlanTagPtr->vid                      ,BIT_12      );
            CPSS_DATA_CHECK_MAX_MAC(vlanTagPtr->dei                      ,BIT_1       );
            CPSS_DATA_CHECK_MAX_MAC(vlanTagPtr->pcp                      ,BIT_3       );
            CPSS_DATA_CHECK_MAX_MAC(info_802_1brPtr->ptpPortInfo.ptpPortMode, CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE___MUST_BE_LAST___E);

            entryData = info_802_1brPtr->pcid  << 20 |
                        vlanTagPtr->pcp << 17 | vlanTagPtr->dei << 16 | vlanTagPtr->vid << 4 |
                        (BOOL2BIT_MAC (info_802_1brPtr->upstreamPort) << 3) |
                        info_802_1brPtr->ptpPortInfo.ptpPortMode;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EVB_E:
            info_evbPtr = &portInfoPtr->info_evb;
            CPSS_DATA_CHECK_MAX_MAC(info_evbPtr->vid                      ,BIT_12      );
            CPSS_DATA_CHECK_MAX_MAC(info_evbPtr->ptpPortInfo.ptpPortMode, CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE___MUST_BE_LAST___E);

            entryData = info_evbPtr->vid << 20 |
                        info_evbPtr->ptpPortInfo.ptpPortMode;

            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_PRE_DA_E:
            info_pre_daPtr = &portInfoPtr->info_pre_da;
            CPSS_DATA_CHECK_MAX_MAC(info_pre_daPtr->port                  ,BIT_8       );
            CPSS_DATA_CHECK_MAX_MAC(info_pre_daPtr->ptpPortInfo.ptpPortMode, CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE___MUST_BE_LAST___E);

            entryData = info_pre_daPtr->port << 16 |
                        BOOL2BIT_MAC(info_pre_daPtr->ptpPortInfo.ptpOverMplsEn) << 2 |
                        info_pre_daPtr->ptpPortInfo.ptpPortMode;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(infoType);
    }

    if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        rc = prvCpssPxWriteTableEntry(devNum,CPSS_PX_TABLE_PHA_SRC_PORT_DATA_E,
                    portNum,&entryData);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /*************************************/
    /* save value for the 'Get' function */
    /*************************************/
    PRV_CPSS_PX_PP_MAC(devNum)->port.egressSourcePortEntryTypeArr[portNum] = infoType;

    return GT_OK;
}

/**
* @internal cpssPxEgressSourcePortEntrySet function
* @endinternal
*
* @brief   Set per source port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - source physical port number.
* @param[in] infoType                 - the type of source port info.
* @param[in] portInfoPtr              - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressSourcePortEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT infoType,
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT      *portInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxEgressSourcePortEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, infoType, portInfoPtr));

    rc = internal_cpssPxEgressSourcePortEntrySet(devNum, portNum, infoType, portInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, infoType, portInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxEgressSourcePortEntryGet function
* @endinternal
*
* @brief   Get per source port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - source physical port number.
*
* @param[out] infoTypePtr              - (pointer to) the type of source port info.
* @param[out] portInfoPtr              - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the port was not initialized with valid info
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxEgressSourcePortEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT *infoTypePtr,
    OUT CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT      *portInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32      entryData;/*the entry is single word*/
    /* 802.1br */
    CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC  *info_802_1brPtr;
    /* DSA */
    CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC      *info_dsaPtr;

    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT  infoType;
    /* VLAN tag */
    CPSS_PX_VLAN_TAG_STC *vlanTagPtr;
    /* EVB */
    CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC *info_evbPtr;
    /* Pre-DA */
    CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC *info_pre_daPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(infoTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(portInfoPtr);

    rc = prvCpssPxReadTableEntry(devNum,CPSS_PX_TABLE_PHA_SRC_PORT_DATA_E,
                portNum,&entryData);
    if(rc != GT_OK)
    {
        return rc;
    }

    infoType = PRV_CPSS_PX_PP_MAC(devNum)->port.egressSourcePortEntryTypeArr[portNum];

    switch(infoType)
    {
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E    :
            info_802_1brPtr =  &portInfoPtr->info_802_1br;
            info_802_1brPtr->pcid = entryData >> 20;
            info_802_1brPtr->srcPortInfo.srcFilteringVector = (entryData >> 4) & 0xFFFF;
            info_802_1brPtr->upstreamPort = BIT2BOOL_MAC ((entryData >> 3 ) & 1);
            info_802_1brPtr->ptpPortInfo.ptpPortMode = entryData & 0x03;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E        :
            info_dsaPtr = &portInfoPtr->info_dsa;

            info_dsaPtr->srcPortNum = (entryData >> 16) & 0xf;
            info_dsaPtr->ptpPortInfo.ptpPortMode = entryData & 0x03;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EDSA_E        :
            info_dsaPtr = &portInfoPtr->info_dsa;

            info_dsaPtr->srcPortNum = (entryData >> 18) & 0x3fff;
            info_dsaPtr->ptpPortInfo.ptpPortMode = entryData & 0x03;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E   :
            portInfoPtr->ptpPortInfo.ptpPortMode = entryData & 0x3;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E  :
            info_802_1brPtr =  &portInfoPtr->info_802_1br;
            vlanTagPtr = &portInfoPtr->info_802_1br.srcPortInfo.vlanTag;

            info_802_1brPtr->pcid = entryData >> 20;
            vlanTagPtr->pcp = (entryData >> 17) & 0x7;
            vlanTagPtr->dei = (entryData >> 16) & 1;
            vlanTagPtr->vid = (entryData >> 4) & 0xFFF;
            info_802_1brPtr->upstreamPort = BIT2BOOL_MAC ((entryData >> 3 ) & 1);
            info_802_1brPtr->ptpPortInfo.ptpPortMode = entryData & 0x03;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EVB_E:
            info_evbPtr = &portInfoPtr->info_evb;
            info_evbPtr->vid =  entryData >> 20;
            info_evbPtr->ptpPortInfo.ptpPortMode = entryData & 0x03;
            break;

        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_PRE_DA_E:
            info_pre_daPtr = &portInfoPtr->info_pre_da;
            info_pre_daPtr->port = (entryData >> 16) & 0xFF;
            info_pre_daPtr->ptpPortInfo.ptpPortMode = entryData & 0x3;
            info_pre_daPtr->ptpPortInfo.ptpOverMplsEn = BIT2BOOL_MAC ((entryData >> 2) & 1);

            break;

        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE___MUST_BE_LAST___E :
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "infoType [%d] indicate that port was not initialized",
                infoType);
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(infoType);
    }

    *infoTypePtr = infoType;

    return GT_OK;
}

/**
* @internal cpssPxEgressSourcePortEntryGet function
* @endinternal
*
* @brief   Get per source port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - source physical port number.
*
* @param[out] infoTypePtr              - (pointer to) the type of source port info.
* @param[out] portInfoPtr              - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the port was not initialized with valid info
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressSourcePortEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT *infoTypePtr,
    OUT CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT      *portInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxEgressSourcePortEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, infoTypePtr, portInfoPtr));

    rc = internal_cpssPxEgressSourcePortEntryGet(devNum, portNum, infoTypePtr, portInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, infoTypePtr, portInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxEgressTargetPortEntrySet function
* @endinternal
*
* @brief   Set per target port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - target physical port number.
* @param[in] infoType                 - the type of target port info.
* @param[in] portInfoPtr              - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxEgressTargetPortEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT infoType,
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT     *portInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  entryData[2];/* the entry is 2 words*/
    CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC  *info_commonPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC           tempSystemRecovery_Info;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(portInfoPtr);

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch(infoType)
    {
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E         :
            info_commonPtr =   &portInfoPtr->info_common;

            CPSS_DATA_CHECK_MAX_MAC(info_commonPtr->pcid           ,BIT_12      );
            CPSS_DATA_CHECK_MAX_MAC(info_commonPtr->egressDelay    ,BIT_20      );

            entryData[1] = info_commonPtr->tpid << 16 | (portNum & 0xF) << 12;

            entryData[0] = info_commonPtr->pcid << 20 |
                           info_commonPtr->egressDelay;
            break;

        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DSA_QCN_E        :
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E  :
            entryData[1] = 0;
            entryData[0] = 0;
            break;

        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_EVB_E:
            info_commonPtr =   &portInfoPtr->info_common;

            entryData[1] = info_commonPtr->tpid << 16;
            entryData[0] = 0;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(infoType);
    }

    if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        rc = prvCpssPxWriteTableEntry(devNum,CPSS_PX_TABLE_PHA_TARGET_PORT_DATA_E,
                    portNum,&entryData[0]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /*************************************/
    /* save value for the 'Get' function */
    /*************************************/
    PRV_CPSS_PX_PP_MAC(devNum)->port.egressTargetPortEntryTypeArr[portNum] = infoType;

    return GT_OK;
}

/**
* @internal cpssPxEgressTargetPortEntrySet function
* @endinternal
*
* @brief   Set per target port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - target physical port number.
* @param[in] infoType                 - the type of target port info.
* @param[in] portInfoPtr              - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressTargetPortEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT infoType,
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT     *portInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxEgressTargetPortEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, infoType, portInfoPtr));

    rc = internal_cpssPxEgressTargetPortEntrySet(devNum, portNum, infoType, portInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, infoType, portInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxEgressTargetPortEntryGet function
* @endinternal
*
* @brief   Get per target port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - target physical port number.
*
* @param[out] infoTypePtr              - (pointer to) the type of target port info.
* @param[out] portInfoPtr              - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the port was not initialized with valid info
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxEgressTargetPortEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT *infoTypePtr,
    OUT CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT      *portInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  entryData[2];/* the entry is 2 words*/
    CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC  *info_commonPtr;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT  infoType;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(infoTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(portInfoPtr);

    rc = prvCpssPxReadTableEntry(devNum,CPSS_PX_TABLE_PHA_TARGET_PORT_DATA_E,
                portNum,&entryData[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    infoType = PRV_CPSS_PX_PP_MAC(devNum)->port.egressTargetPortEntryTypeArr[portNum];

    switch(infoType)
    {
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E         :
            info_commonPtr =   &portInfoPtr->info_common;

            info_commonPtr->tpid            = entryData[1] >> 16;

            info_commonPtr->pcid            = entryData[0] >> 20;
            info_commonPtr->egressDelay     = entryData[0] & 0xFFFFF;
            break;

        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DSA_QCN_E        :
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E  :
            portInfoPtr->notNeeded = 0;
            break;

        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_EVB_E:
            info_commonPtr =   &portInfoPtr->info_common;
            info_commonPtr->tpid = entryData[1] >> 16;
            break;

        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE___MUST_BE_LAST___E :
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "infoType [%d] indicate that port was not initialized",
                infoType);
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(infoType);
    }

    *infoTypePtr = infoType;

    return GT_OK;
}

/**
* @internal cpssPxEgressTargetPortEntryGet function
* @endinternal
*
* @brief   Get per target port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - target physical port number.
*
* @param[out] infoTypePtr              - (pointer to) the type of target port info.
* @param[out] portInfoPtr              - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the port was not initialized with valid info
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressTargetPortEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT *infoTypePtr,
    OUT CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT      *portInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxEgressTargetPortEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, infoTypePtr, portInfoPtr));

    rc = internal_cpssPxEgressTargetPortEntryGet(devNum, portNum, infoTypePtr, portInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, infoTypePtr, portInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxEgressVlanTagStateEntrySet function
* @endinternal
*
* @brief   Sets VLAN tag state entry for target ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - VLAN Id
*                                      (APPLICABLE RANGE: 0..4095)
* @param[in] portsTagging             - bitmap of physical ports tagged in the VLAN.
*                                      Every bit in bitmap represents VLAN tag state per target port:
*                                      0 - Egress tag state is untagged. Packet's VLAN tag is removed
*                                      1 - Egress tag state is tagged. Packet's VLAN tag is not removed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxEgressVlanTagStateEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               vlanId,
    IN CPSS_PX_PORTS_BMP    portsTagging
)
{
    GT_STATUS   rc;
    GT_U32  entryIndex; /* index into SHARED DMEM */
    GT_U32  entryData;  /* data entry SHARED DMEM */
    GT_U32  offset;     /* offset in SHARED DMEM */
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(vlanId, BIT_12);
    PRV_CPSS_PX_PORT_BMP_CHECK_MAC(devNum, portsTagging);

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        /*skip in case of sysyem recovery HA */
        return GT_OK;
    }

    entryIndex = vlanId / 2;
    offset = 16 - (16 * (vlanId % 2));

    rc = prvCpssPxReadTableEntry(devNum, CPSS_PX_TABLE_PHA_SHARED_DMEM_E,
                entryIndex, &entryData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Set table entry */
    U32_SET_FIELD_MAC(entryData, offset, 16, portsTagging);

    return prvCpssPxWriteTableEntry(devNum, CPSS_PX_TABLE_PHA_SHARED_DMEM_E,
                entryIndex, &entryData);
}

/**
* @internal cpssPxEgressVlanTagStateEntrySet function
* @endinternal
*
* @brief   Sets VLAN tag state entry for target ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - VLAN Id
*                                      (APPLICABLE RANGE: 0..4095)
* @param[in] portsTagging             - bitmap of physical ports tagged in the VLAN.
*                                      Every bit in bitmap represents VLAN tag state per target port:
*                                      0 - Egress tag state is untagged. Packet's VLAN tag is removed
*                                      1 - Egress tag state is tagged. Packet's VLAN tag is not removed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressVlanTagStateEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               vlanId,
    IN CPSS_PX_PORTS_BMP    portsTagging
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxEgressVlanTagStateEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, portsTagging));

    rc = internal_cpssPxEgressVlanTagStateEntrySet(devNum, vlanId, portsTagging);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, portsTagging));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxEgressVlanTagStateEntryGet function
* @endinternal
*
* @brief   Gets VLAN tag state entry for target ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - VLAN Id
*                                      (APPLICABLE RANGE: 0..4095)
*
* @param[out] portsTaggingPtr          - (pointer to) bitmap of physical ports tagged in the VLAN.
*                                      Every bit in bitmap represents VLAN tag state per target port:
*                                      0 - Egress tag state is untagged. Packet's VLAN tag is removed
*                                      1 - Egress tag state is tagged. Packet's VLAN tag is not removed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPxEgressVlanTagStateEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               vlanId,
    OUT CPSS_PX_PORTS_BMP   *portsTaggingPtr
)
{
    GT_STATUS   rc;
    GT_U32  entryIndex; /* index into SHARED DMEM */
    GT_U32  entryData;  /* data entry SHARED DMEM */
    GT_U32  offset;     /* offset in SHARED DMEM */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(vlanId, BIT_12);
    CPSS_NULL_PTR_CHECK_MAC(portsTaggingPtr);

    entryIndex = vlanId / 2;
    offset = 16 - (16 * (vlanId % 2));

    rc = prvCpssPxReadTableEntry(devNum, CPSS_PX_TABLE_PHA_SHARED_DMEM_E,
                                 entryIndex, &entryData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get table entry */
    *portsTaggingPtr = U32_GET_FIELD_MAC(entryData, offset, 16);

    return GT_OK;
}

/**
* @internal cpssPxEgressVlanTagStateEntryGet function
* @endinternal
*
* @brief   Gets VLAN tag state entry for target ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - VLAN Id
*                                      (APPLICABLE RANGE: 0..4095)
*
* @param[out] portsTaggingPtr          - (pointer to) bitmap of physical ports tagged in the VLAN.
*                                      Every bit in bitmap represents VLAN tag state per target port:
*                                      0 - Egress tag state is untagged. Packet's VLAN tag is removed
*                                      1 - Egress tag state is tagged. Packet's VLAN tag is not removed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressVlanTagStateEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               vlanId,
    OUT CPSS_PX_PORTS_BMP   *portsTaggingPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxEgressVlanTagStateEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, portsTaggingPtr));

    rc = internal_cpssPxEgressVlanTagStateEntryGet(devNum, vlanId, portsTaggingPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, portsTaggingPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxEgressQcnVlanTagEntrySet function
* @endinternal
*
* @brief   Sets QCN VLAN tag entry for congested ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number.
* @param[in] port                  - physical port.
*                                   (APPLICABLE RANGE: 0..16)
* @param[in] vlanTagPtr            - (pointer to) VLAN tag added to QCN packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxEgressQcnVlanTagEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_VLAN_TAG_STC *vlanTagPtr
)
{
    GT_STATUS   rc;
    GT_U32  entryIndex; /* index into SHARED DMEM */
    GT_U32  entryData;  /* data entry SHARED DMEM */
    GT_U32  offset;     /* offset in SHARED DMEM */
    GT_U32  hwVlanTag;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(vlanTagPtr);
    QCN_VLAN_TAG_CHECK_MAC(vlanTagPtr);

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        /*skip in case of sysyem recovery HA */
        return GT_OK;
    }

    entryIndex = SHARED_MEM_QCN_VLAN_TBL_ENTRY_OFFSET_CNS + (portNum / 2);
    offset = 16 - (16 * (portNum % 2));

    rc = prvCpssPxReadTableEntry(devNum, CPSS_PX_TABLE_PHA_SHARED_DMEM_E,
                entryIndex, &entryData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Build VLAN tag in HW format */
    hwVlanTag = vlanTagPtr->pcp << 13 | vlanTagPtr->dei << 12 | vlanTagPtr->vid;

    /* Set table entry */
    U32_SET_FIELD_MAC(entryData, offset, 16, hwVlanTag);

    return prvCpssPxWriteTableEntry(devNum, CPSS_PX_TABLE_PHA_SHARED_DMEM_E,
                entryIndex, &entryData);
}

/**
* @internal cpssPxEgressQcnVlanTagEntrySet function
* @endinternal
*
* @brief   Sets QCN VLAN tag entry for congested ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number.
* @param[in] port                  - physical port.
*                                   (APPLICABLE RANGE: 0..16)
* @param[in] vlanTagPtr            - (pointer to) VLAN tag added to QCN packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressQcnVlanTagEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_VLAN_TAG_STC *vlanTagPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxEgressQcnVlanTagEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, vlanTagPtr));

    rc = internal_cpssPxEgressQcnVlanTagEntrySet(devNum, portNum, vlanTagPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, vlanTagPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxEgressQcnVlanTagEntryGet function
* @endinternal
*
* @brief   Gets QCN VLAN tag state entry for target ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number.
* @param[in] port                  - physical port.
*                                   (APPLICABLE RANGE: 0..16)
* @param[out] vlanTagPtr            - (pointer to) VLAN tag added to QCN packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxEgressQcnVlanTagEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PX_VLAN_TAG_STC *vlanTagPtr
)
{
    GT_STATUS   rc;
    GT_U32  entryIndex; /* index into SHARED DMEM */
    GT_U32  entryData;  /* data entry SHARED DMEM */
    GT_U32  offset;     /* offset in SHARED DMEM */
    GT_U32  hwVlanTag;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(vlanTagPtr);

    entryIndex = SHARED_MEM_QCN_VLAN_TBL_ENTRY_OFFSET_CNS + (portNum / 2);
    offset = 16 - (16 * (portNum % 2));

    rc = prvCpssPxReadTableEntry(devNum, CPSS_PX_TABLE_PHA_SHARED_DMEM_E,
                entryIndex, &entryData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get table entry */
    hwVlanTag = U32_GET_FIELD_MAC(entryData, offset, 16);

    /* Build VLAN tag in SW format */
    vlanTagPtr->pcp = (hwVlanTag >> 13) & 0x7;
    vlanTagPtr->dei = (hwVlanTag >> 12) & 0x1;
    vlanTagPtr->vid = hwVlanTag & 0xFFF;

    return GT_OK;
}

/**
* @internal cpssPxEgressQcnVlanTagEntryGet function
* @endinternal
*
* @brief   Gets QCN VLAN tag state entry for target ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number.
* @param[in] port                  - physical port.
*                                   (APPLICABLE RANGE: 0..16)
* @param[out] vlanTagPtr            - (pointer to) VLAN tag added to QCN packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressQcnVlanTagEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PX_VLAN_TAG_STC *vlanTagPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxEgressQcnVlanTagEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, vlanTagPtr));

    rc = internal_cpssPxEgressQcnVlanTagEntryGet(devNum, portNum, vlanTagPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, vlanTagPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

